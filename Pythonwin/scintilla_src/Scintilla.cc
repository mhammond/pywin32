// Scintilla source code edit control
// Scintilla.cc - main code for the edit control
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef GTK
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef GTK
#include <gtk/gtk.h>
#include "gtk/gtksignal.h"
#include "gtk/gtktable.h"
#include "gtk/gtktogglebutton.h"
#include "gdk/gdkkeysyms.h"
#else
#ifdef _MSC_VER
#include <richedit.h>
#include <commctrl.h>
#else
extern "C" LONG STDCALL timeGetTime();
#endif
#endif

#include "Scintilla.h"
#include "Document.h"
#include "Platform.h"
#ifndef GTK
#include "CallTip.h"
#endif

enum {
    COMMAND_SIGNAL,
    NOTIFY_SIGNAL,
    LAST_SIGNAL
};

#ifdef GTK
char defaultFont[] = "lucidatypewriter";
const int defaultSize = 12;
#else
char defaultFont[] = "Verdana";
const int defaultSize = 8;
#endif

#ifdef GTK

static GdkAtom clipboard_atom = GDK_NONE;

#define LOWORD(x) (x & 0xffff)
#define HIWORD(x) (x >> 16)

enum {
    TARGET_STRING,
    TARGET_TEXT,
    TARGET_COMPOUND_TEXT
};

static COLORREF RGB(unsigned int red, unsigned int green, unsigned int blue) {
	GdkColor ret;
	ret.red = red * (65535/255);
	ret.green = green * (65535/255);
	ret.blue = blue * (65535/255);
	// the pixel value indicates the index in the colourmap of the colour.
	// it is simply a combination of the RGB values we set earlier
	ret.pixel = (gulong)(red*65536 + green*256 + blue);
	return ret;
}

static bool operator==(GdkColor a, GdkColor b) {
	return 
		a.red == b.red &&
		a.green == b.green &&
		a.blue == b.blue &&
		a.pixel == b.pixel;
}

static bool PtInRect(RECT *prc, POINT pt) {
	if (pt.x < prc->left || pt.x > prc->right)
		return false;
	if (pt.y < prc->top || pt.y > prc->bottom)
		return false;
	return true;
}

static unsigned int min(unsigned int a, unsigned int b) {
	if (a < b)
		return a;
	else
		return b;
}

static unsigned int max(unsigned int a, unsigned int b) {
	if (a > b)
		return a;
	else
		return b;
}

unsigned int GetDoubleClickTime() {
	return 500;	// Half a second
}

#else

bool IsKeyDown(int nVirtKey) {
	return GetKeyState(nVirtKey) & 0x80000000;
}

#endif

//#define TRACE

static void dprintf(char *format, ...) {
#ifdef TRACE
	char buffer[2000];
	char *pArguments = (char *) & format + sizeof(format);
	vsprintf(buffer,format,pArguments);
#ifdef GTK
	printf("%s",buffer);
#else
	OutputDebugString(buffer);
#endif
#endif
}

static int clamp(int val, int minVal, int maxVal) {
	if (val > maxVal)
		val = maxVal;
	if (val < minVal)
		val = minVal;
	return val;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Scintilla Zone

#ifdef GTK
#define SHIFT_PRESSED 1
#define LEFT_CTRL_PRESSED 2
#define LEFT_ALT_PRESSED 4
#endif

#define SCI_NORM 0
#define SCI_SHIFT SHIFT_PRESSED
#define SCI_CTRL LEFT_CTRL_PRESSED
#define SCI_ALT LEFT_ALT_PRESSED
#define SCI_CSHIFT (SCI_CTRL | SCI_SHIFT)

class KeyToCommand {
public:
	int key;
	int modifiers;
	int msg;
};

KeyToCommand keymapDefault[] = {
    VK_DOWN,	SCI_NORM,	SCI_LINEDOWN,
    VK_DOWN,	SCI_SHIFT,	SCI_LINEDOWNEXTEND,
    VK_UP,		SCI_NORM,	SCI_LINEUP,
    VK_UP,		SCI_SHIFT,	SCI_LINEUPEXTEND,
    VK_LEFT,		SCI_NORM,	SCI_CHARLEFT,
    VK_LEFT,		SCI_SHIFT,	SCI_CHARLEFTEXTEND,
    VK_LEFT,		SCI_CTRL,	SCI_WORDLEFT,
    VK_LEFT,		SCI_CSHIFT,	SCI_WORDLEFTEXTEND,
    VK_RIGHT,	SCI_NORM,	SCI_CHARRIGHT,
    VK_RIGHT,	SCI_SHIFT,	SCI_CHARRIGHTEXTEND,
    VK_RIGHT,	SCI_CTRL,	SCI_WORDRIGHT,
    VK_RIGHT,	SCI_CSHIFT,	SCI_WORDRIGHTEXTEND,
    VK_HOME, 	SCI_NORM, 	SCI_VCHOME,
    VK_HOME, 	SCI_SHIFT, 	SCI_VCHOMEEXTEND,
    VK_HOME, 	SCI_CTRL, 	SCI_DOCUMENTSTART,
    VK_HOME, 	SCI_CSHIFT, 	SCI_DOCUMENTSTARTEXTEND,
    VK_END,	 	SCI_NORM, 	SCI_LINEEND,
    VK_END,	 	SCI_SHIFT, 	SCI_LINEENDEXTEND,
    VK_END, 		SCI_CTRL, 	SCI_DOCUMENTEND,
    VK_END, 		SCI_CSHIFT, 	SCI_DOCUMENTENDEXTEND,
    VK_PRIOR,	SCI_NORM, 	SCI_PAGEUP,
    VK_PRIOR,	SCI_SHIFT, 	SCI_PAGEUPEXTEND,
    VK_NEXT, 	SCI_NORM, 	SCI_PAGEDOWN,
    VK_NEXT, 	SCI_SHIFT, 	SCI_PAGEDOWNEXTEND,
    VK_DELETE, 	SCI_NORM,	WM_CLEAR,
    VK_DELETE, 	SCI_SHIFT,	WM_CUT,
    VK_INSERT, 	SCI_NORM,	SCI_EDITTOGGLEOVERTYPE,
    VK_INSERT, 	SCI_SHIFT,	WM_PASTE,
    VK_INSERT, 	SCI_CTRL,	WM_COPY,
    VK_ESCAPE,  	SCI_NORM,	SCI_CANCEL,
    VK_BACK,		SCI_NORM, 	SCI_DELETEBACK,
    'Z', 			SCI_CTRL,	WM_UNDO,
    'Y', 			SCI_CTRL,	SCI_REDO,
    'X', 			SCI_CTRL,	WM_CUT,
    'C', 			SCI_CTRL,	WM_COPY,
    'V', 			SCI_CTRL,	WM_PASTE,
    'A', 			SCI_CTRL,	SCI_SELECTALL,
    VK_TAB,		SCI_NORM,	SCI_TAB,
    VK_TAB,		SCI_SHIFT,	SCI_BACKTAB,
    VK_RETURN, 	SCI_NORM,	SCI_NEWLINE,
    'L', 			SCI_CTRL,	SCI_FORMFEED,
    0,0,0,
};

// Colour pairs hold a desired colour and the colour that the graphics engine
// allocates to approximate the desired colour.
// To make palette management more automatic, ColourPairs could register at 
// construction time with a palette management object.
struct ColourPair {
	COLORREF desired;
	COLORREF allocated;

	ColourPair(COLORREF desired_=RGB(0,0,0)) {
		desired = desired_;
		allocated = desired;
	}
};

class Indicator {
public:
	int style;
	ColourPair fore;
	Indicator() : style(INDIC_PLAIN), fore(RGB(0,0,0)) {
	}
	void Draw(Surface *surface, RECT &rc);
};

void Indicator::Draw(Surface *surface, RECT &rc) {
	surface->PenColor(fore.allocated);
	int ymid = (rc.bottom + rc.top) / 2;
	if (style == INDIC_SQUIGGLE) {
		surface->MoveTo(rc.left, rc.top);
		int x = rc.left + 2;
		int y = 2;
		while (x < rc.right) {
			surface->LineTo(x, rc.top + y);
			x += 2;
			y = 2 - y;
		}
	} else if (style == INDIC_TT) {
		surface->MoveTo(rc.left, ymid);
		int x = rc.left + 5;
		while (x < rc.right) {
			surface->LineTo(x, ymid);
			surface->MoveTo(x-3, ymid);
			surface->LineTo(x-3, ymid+2);

			x++;
			surface->MoveTo(x, ymid);
			x += 5;
		}
	} else {	// Either INDIC_PLAIN or unknown
		surface->MoveTo(rc.left, ymid);
		surface->LineTo(rc.right, ymid);
	}
}

class LineMarker {
public:
	int markType;
	ColourPair fore;
	ColourPair back;
	LineMarker() {
		markType = SC_MARK_CIRCLE;
		fore = RGB(0,0,0);
		back = RGB(0xff,0xff,0xff);
	}
	void Draw(Surface *surface, RECT &rc);

};

void LineMarker::Draw(Surface *surface, RECT &rc) {
	int minDim = min ( rc.right - rc.left, rc.bottom - rc.top );
	int centreX = (rc.right + rc.left) / 2;
	int centreY = (rc.bottom + rc.top) / 2;
	int dimOn2 = minDim / 2;
	int dimOn4 = minDim / 4;
	if (markType == SC_MARK_ROUNDRECT) {
		RECT rcRounded = rc;
		rcRounded.left = rc.left + 1;
		rcRounded.right = rc.right - 1;
		surface->RoundedRectangle(rcRounded, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_CIRCLE) {
		RECT rcCircle;
		rcCircle.left = centreX - minDim / 2;
		rcCircle.top = centreY - minDim / 2;
		rcCircle.right = centreX + minDim / 2;
		rcCircle.bottom = centreY + minDim / 2;
		surface->Ellipse(rcCircle, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_ARROW) {
		POINT pts[] = {
    		{centreX, centreY - minDim / 2},
    		{centreX, centreY + minDim / 2},
    		{centreX + minDim / 2, centreY},
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_SMALLRECT) {
		RECT rcSmall;
		rcSmall.left = rc.left + 1;
		rcSmall.top = rc.top + 2;
		rcSmall.right = rc.right - 1;
		rcSmall.bottom = rc.bottom - 2;
		surface->Rectangle(rcSmall, fore.allocated, back.allocated);
	} else { // SC_MARK_SHORTARROW
		POINT pts[] = {
			{centreX, centreY + dimOn2},
			{centreX + dimOn2, centreY},
			{centreX, centreY - dimOn2},
			{centreX, centreY - dimOn4},
			{centreX - dimOn4, centreY - dimOn4},
			{centreX - dimOn4, centreY + dimOn4},
			{centreX, centreY + dimOn4},
			{centreX, centreY + dimOn2},
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
				fore.allocated, back.allocated);
	}
}

class Palette {
public:
	Palette();
	~Palette();

	void Clear();
	void Want(ColourPair &cp);
	void Allocate(HWND hwnd=0);
	void Find(ColourPair &cp);

	int used;
	bool allowRealization;
	enum {numEntries = 100};
	ColourPair entries[numEntries];

#ifdef GTK
	GdkColor *allocatedPalette;
	int allocatedLen;
#else
	HPALETTE hpal;
	HPALETTE SelectInto(HDC hdc, bool inBackGround);
#endif
};

Palette::Palette() {
	used = 0;
	allowRealization = false;
#ifdef GTK
	allocatedPalette = 0;
	allocatedLen = 0;
#else
	hpal = 0;
#endif
}

Palette::~Palette() {
	Clear();
}

void Palette::Clear() {
	used = 0;
#ifdef GTK
	delete []allocatedPalette;
	allocatedPalette = 0;
	allocatedLen = 0;
#else
	if (hpal)
		DeleteObject(hpal);
	hpal = 0;
#endif
}

void Palette::Want(ColourPair &cp) {
	for (int i=0; i < used; i++) {
		if (entries[i].desired == cp.desired)
			return;
	}

	if (used < numEntries) {
		entries[used].desired = cp.desired;
		entries[used].allocated = cp.desired;
		used++;
	}
}

void Palette::Allocate(HWND hwnd) {
#ifdef GTK
	if (allocatedPalette) {
		gdk_colormap_free_colors(gtk_widget_get_colormap(hwnd),
			allocatedPalette, allocatedLen);
		delete []allocatedPalette;
		allocatedPalette = 0;
		allocatedLen = 0;
	}
	allocatedPalette = new GdkColor[used];
	gboolean *successPalette = new gboolean[used];
	if (allocatedPalette) {
		allocatedLen = used;
		int iPal = 0;
		for (iPal=0;iPal<used;iPal++) {
			allocatedPalette[iPal] = entries[iPal].desired;
		}
		gdk_colormap_alloc_colors(gtk_widget_get_colormap(hwnd),
			allocatedPalette, allocatedLen, FALSE, TRUE, 
			successPalette);
		for (iPal=0;iPal<used;iPal++) {
			entries[iPal].allocated = allocatedPalette[iPal];
		}
	} 
	delete []successPalette;
#else
	if (hpal)
		DeleteObject(hpal);
	hpal = 0;

	if (allowRealization) {
		char *pal = new char[sizeof(LOGPALETTE) + (used-1) * sizeof(PALETTEENTRY)];
		LOGPALETTE *logpal = reinterpret_cast<LOGPALETTE *>(pal);
		logpal->palVersion = 0x300;
		logpal->palNumEntries = used;
		for (int iPal=0;iPal<used;iPal++) {
			COLORREF desired = entries[iPal].desired;
			//dprintf("Palette[%d] = %x\n", iPal, desired);
			logpal->palPalEntry[iPal].peRed   = GetRValue(desired);
			logpal->palPalEntry[iPal].peGreen = GetGValue(desired);
			logpal->palPalEntry[iPal].peBlue  = GetBValue(desired);
			entries[iPal].allocated = 
				PALETTERGB(GetRValue(desired), GetGValue(desired), GetBValue(desired));
			// PC_NOCOLLAPSE means exact colours allocated even when in background this means other windows 
			// are less likely to get their colours and also flashes more when switching windows
			logpal->palPalEntry[iPal].peFlags = PC_NOCOLLAPSE;
			// 0 allows approximate colours when in background, yielding moe colours to other windows
			//logpal->palPalEntry[iPal].peFlags = 0;
		}
		hpal = CreatePalette(logpal);
		delete []pal;
		//dprintf("Palette created %x\n", hpal);
	}
#endif
}

void Palette::Find(ColourPair &cp) {
	for (int i=0; i < used; i++) {
		if (entries[i].desired == cp.desired) {
			cp.allocated = entries[i].allocated;
			return;
		}
	}
	cp.allocated = cp.desired;
}

#ifndef GTK
HPALETTE Palette::SelectInto(HDC hdc, bool inBackGround) {
	if (allowRealization)
		return SelectPalette(hdc, hpal, inBackGround);
	return 0;
}
#endif

class Style {
public:
	ColourPair fore;
	ColourPair back;
	bool bold;
	bool italic;
	int size;
	char fontName[100];

	HFONT font;
	unsigned int lineHeight;
	unsigned int ascent;
	unsigned int descent;
	unsigned int externalLeading;
	unsigned int aveCharWidth;
	unsigned int spaceWidth;

	Style();
	~Style();
	void Clear(COLORREF fore_=RGB(0,0,0), COLORREF back_=RGB(0xff,0xff,0xff),
           	int size_=defaultSize, const char *fontName_=defaultFont, bool bold_=false, bool italic_=false);
	void Realise();
};

Style::Style() {
	font = 0;
	Clear();
}

Style::~Style() {
#ifdef GTK
	if (font)
		gdk_font_unref(font);
#else
	if (font)
		DeleteObject(font);
#endif
	font = 0;
}

void Style::Clear(COLORREF fore_, COLORREF back_, int size_, const char *fontName_, bool bold_, bool italic_) {
	fore.desired = fore_;
	back.desired = back_;
	bold = bold_;
	italic = italic_;
	size = size_;
	strcpy(fontName, fontName_);
#ifdef GTK
	if (font)
		gdk_font_unref(font);
#else
	if (font)
		DeleteObject(font);
#endif
	font = 0;
}

void Style::Realise() {
#ifdef GTK
	char fontspec[300];
	fontspec[0] = '\0';
	strcat(fontspec, "-*-");
	strcat(fontspec, fontName);
	if (bold)
		strcat(fontspec, "-bold");
	else
		strcat(fontspec, "-medium");
	if (italic)
		strcat(fontspec, "-i");
	else
		strcat(fontspec, "-r");
	strcat(fontspec, "-*-*-*");
	char sizePts[100];
	sprintf(sizePts, "-%0d", size * 10);
	strcat(fontspec, sizePts);
	strcat(fontspec, "-*-*-*-*-*-*");
	font = gdk_font_load(fontspec);
	if (NULL == font) {
		// Font not available so substitute a reasonable code font
		// iso8859 appears to only allow western characters.
		//dprintf("Null font %s\n", fontspec);
		font = gdk_font_load("*-*-*-*-*-*-*-*-*-*-*-*-iso8859-*");
	}
	gint lbearing;
	gint rbearing;
	gint width;
	gint ascent_;
	gint descent_;

	gdk_string_extents(font,
                   	" ",
                   	&lbearing,
                   	&rbearing,
                   	&width,
                   	&ascent_,
                   	&descent_);
	spaceWidth = width;

	gdk_string_extents(font,
                   	" `~!@#$%^&*()-_=+\\|[]{};:\"\'<,>.?/1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                   	&lbearing,
                   	&rbearing,
                   	&width,
                   	&ascent_,
                   	&descent_);
	ascent = ascent_;
	descent = descent_;
	//dprintf("Font l:%s: r:%d %d w:%d a:%d d:%d\n", fontspec, lbearing, rbearing, aveCharWidth, ascent, descent);
	lineHeight = ascent + descent;
	externalLeading = 0;
	aveCharWidth = gdk_char_width(font, 'n');
#else
	if (font)
		DeleteObject(font);
	font = 0;

	HDC hdc = CreateCompatibleDC(NULL);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	// The negative is to allow for leading
	lf.lfHeight = -(abs(size) * GetDeviceCaps(hdc, LOGPIXELSY)) / 72;
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = italic ? 1 : 0;
	lf.lfCharSet = DEFAULT_CHARSET;
	strcpy(lf.lfFaceName, fontName);

	font = CreateFontIndirect(&lf);
	HFONT fontOld = static_cast<HFONT>(SelectObject(hdc, font));
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	ascent = tm.tmAscent;
	descent = tm.tmDescent;
	/*
	dprintf("Font: ht:%d as:%d de:%d led:%d\n", 
	tm.tmHeight,
	tm.tmAscent,
	tm.tmDescent,
	tm.tmExternalLeading);
	*/
	// Probably more typographically correct to include leading
	// but that means more complex drawing as leading must be erased
	//lineHeight = tm.tmExternalLeading + tm.tmHeight;
	externalLeading = tm.tmExternalLeading;
	lineHeight = tm.tmHeight;
	aveCharWidth = tm.tmAveCharWidth;

	SIZE sz;
	char chSpace = ' ';
	GetTextExtentPoint32(hdc, &chSpace, 1, &sz);
	spaceWidth = sz.cx;

	SelectObject(hdc, fontOld);
	DeleteDC(hdc);
#endif
}

class Scintilla {

	// Enumeration of commands and child windows
	enum {
		idCallTip=1,
		idAutoComplete=2,
		
		idcmdUndo=10,
		idcmdRedo=11,
		idcmdCut=12,
		idcmdCopy=13,
		idcmdPaste=14,
		idcmdDelete=15,
		idcmdSelectAll=16,
	};

	HWND hwndCallTip;
	bool inCallTipMode;
	int posStartCallTip;
	HWND hwndAutoComplete;
	bool inAutoCompleteMode;
	int posStartAutoComplete;
	char autoCompleteStops[256];
	int endStyled;

#ifdef GTK
	_ScintillaObject *sci;
	GtkWidget *hwnd;
	GtkObject *adjustmentv;
	GtkWidget *scrollbarv;
	GtkObject *adjustmenth;
	GtkWidget *scrollbarh;
	GtkWidget *draw;
	GtkWidget *popup;
	GtkWidget *listAutoComplete;
	int currentAutoComplete;
	char *valCT;
	int startHighlightCT;
	int endHighlightCT;
#endif

	bool isModified;
	bool hideSelection;
	bool inOverstrike;

	int selMarginWidth;
	int lineNumberWidth;
	int fixedColumnWidth;
	bool bufferedDraw;

	Style styles[STYLE_MAX + 1];
	bool stylesValid;

	LineMarker markers[MARKER_MAX + 1];

	Indicator indicators[INDIC_MAX + 1];

	Palette palette;

	bool caret;

	unsigned int tabInChars;

	enum {maxLineLength = 4000};
	int lineHeight;
	unsigned int maxAscent;
	unsigned int maxDescent;
	unsigned int tabWidth;
	unsigned int aveCharWidth;
	unsigned int spaceWidth;
	int xOffset;

	int eolMode;
	int dbcsCodePage;

	ColourPair foreground;
	ColourPair background;
	int size;
	char fontName[100];
	bool bold;
	bool italic;
	bool selforeset;
	ColourPair selforeground;
	bool selbackset;
	ColourPair selbackground;

	ColourPair selbar;
	ColourPair linenumfore;
	ColourPair caretcolour;

#ifdef GTK
	GdkPixmap *pixmapLine;
	GdkPixmap *pixmapSelMargin;
#else
	HBRUSH selmarginbrush;
	HBITMAP bitmapSelMargin;
	HBITMAP bitmapLineBuffer;
	HBITMAP oldBitmap;
	HDC hdcBitmap;
#endif

	KeyToCommand *keymap;
	int keymapLen;
	int keymapAlloc;

#ifdef GTK
	unsigned char *pasteBuffer;
#endif

	void InvalidateStyleData();
	void RefreshStyleData();

	int LineFromPosition(int pos);

	int SelectionStart();
	int SelectionEnd();
#ifndef GTK
	HGLOBAL GetSelText();
#endif
	void DelCharBack();
	int ClampPositionIntoDocument(int pos);
	bool IsCrLf(int pos);
	int MovePositionOutsideChar(int pos, int moveDir);

	void GetTextRect(RECT *prc);
	void ScrollTo(int line);
	void HorizontalScrollTo(int xPos);
	void EnsureCaretVisible();
	void MoveCaret(int x, int y);
	void ShowCaretAtCurrentPosition();
	void DropCaret();

	void GetClientRectangle(RECT *rc);
	int LinesOnScreen();
	int LinesToScroll();
	int MaxScrollPos();
	POINT lastClick;
	unsigned int lastClickTime;
	enum { selChar, selWord, selLine } selType;
	bool capturedMouse;
	int lastXChosen;
	int lineAnchor;
	int originalAnchorPos;
	int currentPos;
	int anchor;
	int topLine;
	bool viewWhitespace;
	int stylingPos;
	int stylingMask;

#ifndef GTK
	static HINSTANCE hInstance;
	HWND hwnd;
#endif

	Document doc;

	void Paint(RECT rcPaint);
#ifdef GTK
	gint PaintCT(GtkWidget *widget_, GdkEventExpose ose);
#endif

	void DropGraphics();
	void SetVertScrollFromTopLine();
	void SetScrollBars(LPARAM *plParam=NULL,WPARAM wParam=0);
	void Redraw();
	void RedrawSelMargin();
	void ModifiedAt(int pos);

	// Gateways to modifying document
	void DeleteChars(int pos, int len);
	void InsertStyledString(int position, char *s, int insertLength);
	void Undo();
	void Redo();

	void InsertChar(int pos, char ch);
	void InsertString(int position, char *s);
	void InsertString(int position, char *s, int insertLength);

	void ClearAll();
	void ClearSelection();
	void DelChar();
	int LinesTotal();
	void InvalidateRange(int start, int end);
	void SetSelection(int currentPos_, int anchor_);
	void SetSelection(int currentPos_);
	void SetPosition(int pos, bool shift=false);
	int LineStart(int line);
	int LineEndPosition(int position);
	int VCHomePosition(int position);
	int MovePositionTo(int newPos, bool extend = false);
	void SetLastXChosen();
	void ChangePosition(int delta, bool fExtend=false);
	void NotifyChange();
	void NotifyStyleNeeded(int endStyleNeeded);
	void NotifyChar(char ch);
	void NotifySavePoint(bool isSavePoint);
	void NotifyModifyAttempt();
	void NotifyKey(int key, int modifiers);

	void Indent(bool forwards);
	void AddChar(char ch);
	int ExtendWordSelect(int pos, int delta);
	int NextWordStart(int delta);
	int KeyCommand(WORD iMessage);
	int KeyDown(int key, bool shift, bool ctrl, bool alt);

	void AssignCmdKey(int key, int modifiers, int msg);
	bool GetWhitespaceVisible();
	void SetWhitespaceVisible(bool view);

	void DeleteUndoHistory();
	void Cut();
	void Copy();
	void Paste();
	void Clear();
	void SelectAll();

	void GoToLine(int lineNo);

	POINT LocationFromPosition(int pos);
	int PositionFromLocation(POINT pt);
	int LineFromLocation(POINT pt);
	int Length();
	char CharAt(int pos);
	int CurrentPosition();

	void PaintSelMargin(Surface *surface, RECT &rc);
#ifndef GTK
	void CreateGraphicObjects(HDC hdc);
#endif
	void RealizeWindowPalette(bool inBackGround);
#ifdef GTK
	void Scroll(int topLineNew);
#else
	void Scroll(WPARAM wParam);
#endif
	void HorizontalScroll(WPARAM wParam);

	void AutoCompleteCancel();
	void AutoCompleteMove(int delta);
	void AutoCompleteChanged(char ch=0);
	void AutoCompleteCompleted();
	void AutoCompleteStart(char *list);

	void CallTipStart(int pos, char *defn);
	void CallTipCancel();
#ifdef GTK
	void AddToPopUp(const char *label, bool enabled=true);
#endif
	void ContextMenu(POINT pt);
	void Command(WPARAM wParam);
	bool IsWordAt(int start, int end);
	long FindText(WORD iMessage,WPARAM wParam,LPARAM lParam);

	void Capture();
	void Release();

	void ButtonDown(POINT pt, unsigned int curTime, bool shift);
	void ButtonMove(POINT pt);
	void ButtonUp(POINT pt, unsigned int curTime);

#ifdef GTK
	// GTK methods
	void ReceivedSelection(GtkSelectionData *selection_data, guint time);
	void GetSelection(GtkSelectionData *selection_data);
	void Resize(int width, int height);

	// Callback functions
	static gint Expose(GtkWidget *widget, GdkEventExpose *ose, Scintilla *sci);
	static gint ExposeCT(GtkWidget *widget, GdkEventExpose *ose, gpointer p);
	static void ScrollSignal(GtkAdjustment *adj, Scintilla *sci);
	static void ScrollHSignal(GtkAdjustment *adj, Scintilla *sci);
	static void PopUpCB(GtkWidget *widget, gpointer cbdata);
	static void SelectionAC(GtkWidget *clist, gint row, gint column,
		     GdkEventButton *event, gpointer p);
	static gint MoveResize(GtkWidget *widget, GtkAllocation *allocation, Scintilla *sci);
	static gint Press(GtkWidget *widget, GdkEventButton *event, Scintilla *sci);
	static gint MouseRelease(GtkWidget *widget, GdkEventButton *event, Scintilla *sci);
	static gint Motion(GtkWidget *widget, GdkEventMotion *event, Scintilla *sci);
	static gint KeyPress(GtkWidget *widget, GdkEventKey *event, Scintilla *sci);
	static gint KeyRelease(GtkWidget *widget, GdkEventKey *event, Scintilla *sci);
	static void SelectionReceived(GtkWidget *widget, GtkSelectionData *selection_data,
		guint time, Scintilla *sci);
	static void SelectionGet(GtkWidget *widget, GtkSelectionData *selection_data,
		guint info, guint time, Scintilla *sci);
#endif

public:
#ifdef GTK
	Scintilla(_ScintillaObject *sci_);
#else
	Scintilla();
#endif
	~Scintilla();

	long WndProc(WORD iMessage,WPARAM wParam,LPARAM lParam);
#ifndef GTK
	static void Register(HINSTANCE hInstance_);
	static LRESULT PASCAL SWndProc(
		HWND hWnd,UINT iMessage,WPARAM wParam, LPARAM lParam);
#endif
	int ctrlID;	// Public so scintilla_set_id can use it
};

#ifndef GTK
HINSTANCE Scintilla::hInstance = 0;
#endif

#ifdef GTK

gint Scintilla::MoveResize(GtkWidget *widget, GtkAllocation *allocation, Scintilla *sci) {
	//dprintf("sci move resize %d %d\n", allocation->width, allocation->height);
	sci->Resize(allocation->width, allocation->height);
	return TRUE;
}

gint Scintilla::Press(GtkWidget *widget, GdkEventButton *event, Scintilla *sci) {
	//	dprintf("Press %x time=%d state = %x button = %x\n",p,event->time, event->state, event->button);
	POINT pt;
	pt.x = int(event->x);
	pt.y = int(event->y);
	if (event->button == 1) {
		sci->ButtonDown(pt, event->time, event->state & 1);
	} else if (event->button == 3) {
		// PopUp menu
		sci->ContextMenu(pt);
	}
	return TRUE;
}

gint Scintilla::MouseRelease(GtkWidget *widget, GdkEventButton *event, Scintilla *sci) {
	//	dprintf("Release %x %d\n",p,event->time);
	if (event->button == 1) {
		POINT pt;
		pt.x = int(event->x);
		pt.y = int(event->y);
		sci->ButtonUp(pt, event->time);
	}
	return TRUE;
}

gint Scintilla::Motion(GtkWidget *widget, GdkEventMotion *event, Scintilla *sci) {
	//dprintf("Move %x %d\n",p,event->time);
	int x = 0;
	int y = 0;
	GdkModifierType state;
	if (event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = static_cast<int>(event->x);
		y = static_cast<int>(event->y);
		state = static_cast<GdkModifierType>(event->state);
	}
	if (state & GDK_BUTTON1_MASK) {
		POINT pt;
		pt.x = x;
		pt.y = y;
		sci->ButtonMove(pt);
	}
	return TRUE;
}

gint Scintilla::KeyPress(GtkWidget *widget, GdkEventKey *event, Scintilla *sci) {
	//dprintf("SC-key: %d %x %x\n",event->keyval, event->state, GTK_WIDGET_FLAGS(widget));
	bool shift = event->state & GDK_SHIFT_MASK;
	bool ctrl = event->state & GDK_CONTROL_MASK;
	bool alt = event->state & GDK_MOD1_MASK;
	int key = event->keyval;
	if (ctrl && (key < 128))
		key = toupper(key);
	if (key == GDK_ISO_Left_Tab)
		key = VK_TAB;
	sci->KeyDown(key, shift, ctrl, alt);
	//dprintf("SK-key: %d %x %x\n",event->keyval, event->state, GTK_WIDGET_FLAGS(widget));
	return 1;
}

gint Scintilla::KeyRelease(GtkWidget *widget, GdkEventKey *event, Scintilla *sci) {
	//dprintf("SC-keyrel: %d %x %3s\n",event->keyval, event->state, event->string);
	return TRUE;
}

gint Scintilla::Expose(GtkWidget *widget, GdkEventExpose *ose, Scintilla *sci) {
	RECT rcPaint;
	rcPaint.left = ose->area.x;
	rcPaint.top = ose->area.y;
	rcPaint.right = ose->area.x + ose->area.width;
	rcPaint.bottom = ose->area.y + ose->area.height;
	sci->Paint(rcPaint);
	return TRUE;
}

void Scintilla::ScrollSignal(GtkAdjustment *adj, Scintilla *sci) {
	//dprintf("Scrolly %g %x\n",adj->value,p);
	sci->Scroll((int)adj->value);
}

void Scintilla::ScrollHSignal(GtkAdjustment *adj, Scintilla *sci) {
	//dprintf("Scrollyh %g %x\n",adj->value,p);
	sci->HorizontalScrollTo((int)adj->value * 2);
}

static gint sci_focus_in (GtkWidget     *widget,
                      	GdkEventFocus *event) {
	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
	return FALSE;
}

static gint sci_focus_out (GtkWidget     *widget,
                       	GdkEventFocus *event) {
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);
	return FALSE;
}

void Scintilla::SelectionReceived(GtkWidget *widget,
                                 	GtkSelectionData *selection_data,
                                 	guint time,
                                 	Scintilla *sci) {
	//dprintf("Selection received\n");
	sci->ReceivedSelection(selection_data,time);
}

void Scintilla::SelectionGet(GtkWidget *widget,
                            	GtkSelectionData *selection_data,
                            	guint info,
                            	guint time,
                            	Scintilla *sci) {
	//dprintf("Selection get\n");
	sci->GetSelection(selection_data);
}

#endif

#ifdef GTK
Scintilla::Scintilla(_ScintillaObject *sci_)
#else
Scintilla::Scintilla()
#endif
{	// Because of the two signatures, the indenter would get confused if the brace were in the normal place
#ifdef GTK
	sci = sci_;
	hwnd = GTK_WIDGET(sci);
	adjustmentv = 0;
	adjustmenth = 0;
	ctrlID = 0;
#endif

	hwndCallTip = 0;
	hwndAutoComplete = 0;
	inAutoCompleteMode = false;
	inCallTipMode = false;
	posStartAutoComplete = 0;
	strcpy(autoCompleteStops, "");
	endStyled = 0;

	isModified = false;
	hideSelection = false;
	inOverstrike = false;

	selMarginWidth = 20;
	lineNumberWidth = 0;
	fixedColumnWidth = selMarginWidth + lineNumberWidth;
	bufferedDraw = true;

	stylesValid = false;

	lastClick.x = 0;
	lastClick.y = 0;
	lastClickTime = 0;
	capturedMouse = false;
	selType = selChar;

	lastXChosen = 0;
	lineAnchor = 0;
	originalAnchorPos = 0;

	lineHeight = 24;	// bad value to start

#ifdef GTK
	pixmapLine = 0;
	pixmapSelMargin = 0;
#else
	bitmapLineBuffer = NULL;
	bitmapSelMargin = NULL;
	oldBitmap = NULL;
	hdcBitmap = NULL;
#endif

	viewWhitespace = false;

	caret = false;

	tabInChars = 8;

	lineHeight = 1;
	maxAscent = 1;
	maxDescent = 1;

	xOffset = 0;
	currentPos = 0;
	anchor = 0;
	stylingPos = 0;
	stylingMask = 0;

#ifdef GTK
	eolMode = SC_EOL_LF;
#else
	eolMode = SC_EOL_CRLF;
#endif

	dbcsCodePage = 0;

	foreground.desired = RGB(0,0,0);
	background.desired = RGB(0xff,0xff,0xff);
	size = defaultSize;
	strcpy(fontName, defaultFont);
	bold = false;
	italic = false;
	selforeset = false;
	selforeground.desired = RGB(0xff,0,0);
	selbackset = true;
	selbackground.desired = RGB(0xC0,0xC0,0xC0);
#ifdef GTK
	selbar.desired = RGB(0xe0, 0xe0, 0xe0);
#else
	selbar.desired = GetSysColor(COLOR_3DFACE);
#endif
	// TODO: the line numbers should have their own style
	linenumfore.desired = RGB(0,0,0);
	caretcolour.desired = RGB(0xff,0,0);

	indicators[0].style = INDIC_SQUIGGLE;
	indicators[0].fore = RGB(0,0x7f,0);
	indicators[1].style = INDIC_TT;
	indicators[1].fore = RGB(0,0,0xff);
	indicators[2].style = INDIC_PLAIN;
	indicators[2].fore = RGB(0xff,0,0);

	keymap = 0;
	keymapLen = 0;
	keymapAlloc = 0;
	for (int keyIndex = 0; keymapDefault[keyIndex].key; keyIndex++) {
		AssignCmdKey(keymapDefault[keyIndex].key, keymapDefault[keyIndex].modifiers,
             		keymapDefault[keyIndex].msg);
	}

	topLine = 0;

#ifdef GTK
	GTK_WIDGET_SET_FLAGS(hwnd, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(hwnd), GTK_SENSITIVE);
	gtk_signal_connect(GTK_OBJECT(hwnd), "size_allocate",
                   	GTK_SIGNAL_FUNC(MoveResize), this);
	gtk_widget_set_events (hwnd,
                       	GDK_KEY_PRESS_MASK
                       	| GDK_KEY_RELEASE_MASK
                       	| GDK_FOCUS_CHANGE_MASK);
	// Using "after" connect to avoid main window using cursor keys
	// to move focus.
	//gtk_signal_connect(GTK_OBJECT(hwnd), "key_press_event",
	//	GtkSignalFunc(key_event), this);
	gtk_signal_connect_after(GTK_OBJECT(hwnd), "key_press_event",
                         	GtkSignalFunc(KeyPress), this);

	gtk_signal_connect(GTK_OBJECT(hwnd), "key_release_event",
                   	GtkSignalFunc(KeyRelease), this);
	gtk_signal_connect(GTK_OBJECT(hwnd), "focus_in_event",
                   	GtkSignalFunc(sci_focus_in), this);
	gtk_signal_connect(GTK_OBJECT(hwnd), "focus_out_event",
                   	GtkSignalFunc(sci_focus_out), this);

	draw = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
                   	GtkSignalFunc(Expose), this);
	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
                   	GtkSignalFunc(Motion), this);
	gtk_signal_connect(GTK_OBJECT(draw), "button_press_event",
                   	GtkSignalFunc(Press), this);
	gtk_signal_connect(GTK_OBJECT(draw), "button_release_event",
                   	GtkSignalFunc(MouseRelease), this);
	gtk_signal_connect(GTK_OBJECT(draw), "selection_received",
                   	GtkSignalFunc(SelectionReceived), this);
	gtk_signal_connect(GTK_OBJECT(draw), "selection_get",
                   	GtkSignalFunc(SelectionGet), this);

	gtk_widget_set_events(draw,
                       	GDK_EXPOSURE_MASK
                       	| GDK_LEAVE_NOTIFY_MASK
                       	| GDK_BUTTON_PRESS_MASK
                       	| GDK_BUTTON_RELEASE_MASK
                       	| GDK_POINTER_MOTION_MASK
                       	| GDK_POINTER_MOTION_HINT_MASK
                      	);

	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), 400, 400);
	gtk_fixed_put(GTK_FIXED(sci), draw, 0, 0);

	adjustmentv = gtk_adjustment_new(0.0, 0.0, 201.0, 1.0, 20.0, 20.0);
	scrollbarv = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustmentv));
	GTK_WIDGET_UNSET_FLAGS(scrollbarv, GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmentv), "value_changed",
                   	GTK_SIGNAL_FUNC(ScrollSignal), this);
	gtk_fixed_put(GTK_FIXED(sci), scrollbarv, 0, 0);

	adjustmenth = gtk_adjustment_new(0.0, 0.0, 101.0, 1.0, 20.0, 20.0);
	scrollbarh = gtk_hscrollbar_new(GTK_ADJUSTMENT(adjustmenth));
	GTK_WIDGET_UNSET_FLAGS(scrollbarh, GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmenth), "value_changed",
                   	GTK_SIGNAL_FUNC(ScrollHSignal), this);
	gtk_fixed_put(GTK_FIXED(sci), scrollbarh, 0, 0);

	gtk_widget_grab_focus(hwnd);

	static const GtkTargetEntry targets[] = {
                                    		{ "STRING", 0, TARGET_STRING
                                    		},
                                    		{ "TEXT",   0, TARGET_TEXT },
                                    		{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT }
                                    	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);

	if (!clipboard_atom)
		clipboard_atom = gdk_atom_intern ("CLIPBOARD", FALSE);

	gtk_selection_add_targets (GTK_WIDGET(draw), clipboard_atom,
                           	targets, n_targets);

	popup = 0;
	listAutoComplete = 0;
	currentAutoComplete = 0;
	valCT = 0;
	startHighlightCT = 0;
	endHighlightCT = 0;
#endif
}

Scintilla::~Scintilla() {
	DropCaret();

	DropGraphics();

#ifdef GTK
	if (hwndAutoComplete)
		gtk_widget_destroy(GTK_WIDGET(hwndAutoComplete));
#else
	if (hwndAutoComplete)
		DestroyWindow(hwndAutoComplete);
#endif
	hwndAutoComplete = 0;
#ifdef GTK
	if (hwndCallTip)
		gtk_widget_destroy(GTK_WIDGET(hwndCallTip));
#else
	if (hwndCallTip)
		DestroyWindow(hwndCallTip);
#endif
	hwndCallTip = 0;

	delete []keymap;
	keymap = 0;
	keymapLen = 0;
	keymapAlloc = 0;
}

void Scintilla::DropGraphics() {
#ifdef GTK
	if (pixmapLine) {
		gdk_pixmap_unref(pixmapLine);
		pixmapLine = 0;
	}
	if (pixmapSelMargin) {
		gdk_pixmap_unref(pixmapSelMargin);
		pixmapSelMargin = 0;
	}
#else
	if (selmarginbrush)
		DeleteObject(selmarginbrush);
	selmarginbrush = 0;
	if (hdcBitmap && oldBitmap)
		SelectObject(hdcBitmap,oldBitmap);
	oldBitmap = NULL;
	if (bitmapLineBuffer)
		DeleteObject(bitmapLineBuffer);
	bitmapLineBuffer = NULL;
	if (bitmapSelMargin)
		DeleteObject(bitmapSelMargin);
	bitmapSelMargin = 0;
	if (hdcBitmap)
		DeleteDC(hdcBitmap);
	hdcBitmap = NULL;
#endif
}

void Scintilla::InvalidateStyleData() {
	stylesValid = false;
	palette.Clear();
	DropGraphics();
	DropCaret();
}

void Scintilla::RefreshStyleData() {
	if (!stylesValid) {
		stylesValid = true;
		maxAscent = 1;
		maxDescent = 1;
		int i;	// A common iterator to avoid thinking about old vs new scope rules
		for (i=0;i<(sizeof(styles)/sizeof(styles[0]));i++) {
			styles[i].Realise();
			if (maxAscent < styles[i].ascent)
				maxAscent = styles[i].ascent;
			if (maxDescent < styles[i].descent)
				maxDescent = styles[i].descent;
			palette.Want(styles[i].fore);
			palette.Want(styles[i].back);
		}
		for (i=0;i<(sizeof(indicators)/sizeof(indicators[0]));i++) {
			palette.Want(indicators[i].fore);
		}
		for (i=0;i<(sizeof(markers)/sizeof(markers[0]));i++) {
			palette.Want(markers[i].fore);
			palette.Want(markers[i].back);
		}
		palette.Want(foreground);
		palette.Want(background);
		palette.Want(selforeground);
		palette.Want(selbackground);
		palette.Want(selbar);
		palette.Want(linenumfore);
		palette.Want(caretcolour);

		lineHeight = maxAscent + maxDescent;
		aveCharWidth = styles[0].aveCharWidth;
		spaceWidth = styles[0].spaceWidth;
		tabWidth = spaceWidth * tabInChars;

		palette.Allocate(hwnd);
		for (i=0;i<(sizeof(styles)/sizeof(styles[0]));i++) {
			palette.Find(styles[i].fore);
			palette.Find(styles[i].back);
		}
		for (i=0;i<(sizeof(indicators)/sizeof(indicators[0]));i++) {
			palette.Find(indicators[i].fore);
		}
		for (i=0;i<(sizeof(markers)/sizeof(markers[0]));i++) {
			palette.Find(markers[i].fore);
			palette.Find(markers[i].back);
		}
		palette.Find(foreground);
		palette.Find(background);
		palette.Find(selforeground);
		palette.Find(selbackground);
		palette.Find(selbar);
		palette.Find(linenumfore);
		palette.Find(caretcolour);

		SetScrollBars();
	}
}

void Scintilla::GetClientRectangle(RECT *rc) {
#ifdef GTK
	rc->left = 0;
	rc->top = 0;
	// Before any size allocated pretend its 100 wide so not scrolled
	rc->right = 100;
	rc->bottom = 100;
	if (draw && draw->allocation.width > 20) {
		rc->right = draw->allocation.width;
		rc->bottom = draw->allocation.height;
	}
#else
	GetClientRect(hwnd, rc);
#endif
}

int Scintilla::LinesTotal() {
	return doc.Lines();
}

int Scintilla::LinesOnScreen() {
	RECT rcClient;
	GetClientRectangle(&rcClient);
	int htClient = rcClient.bottom - rcClient.top;
#ifdef GTK
	htClient -= 16;
#else
	htClient -= GetSystemMetrics(SM_CYHSCROLL);
#endif
	//dprintf("lines on screen = %d\n", htClient / lineHeight + 1);
	return htClient / lineHeight + 1;
}

int Scintilla::LinesToScroll() {
	int retVal = LinesOnScreen() - 1;
	if (retVal < 1)
		return 1;
	else
		return retVal;
}

int Scintilla::MaxScrollPos() {
	//dprintf("Lines %d screen = %d maxScroll = %d\n",
	//LinesTotal(), LinesOnScreen(), LinesTotal() - LinesOnScreen() + 1);
	//int retVal = LinesTotal() - LinesOnScreen() + 1;
	int retVal = LinesTotal() - LinesOnScreen();
	if (retVal < 0)
		return 0;
	else
		return retVal;
}

int Scintilla::ClampPositionIntoDocument(int pos) {
	return clamp(pos, 0, Length());
}

bool Scintilla::IsCrLf(int pos) {
	if (pos < 0)
		return false;
	if (pos >= (Length()-1))
		return false;
	return (doc.CharAt(pos) == '\r') && (doc.CharAt(pos+1) == '\n');
}

// Normalise a position so that it is not halfway through a two byte character.
// This can occur in two situations -
// When lines are terminated with \r\n pairs which should be treated as one character.
// When displaying DBCS text such as Japanese.
// If moving, move the position in the indicated direction.
int Scintilla::MovePositionOutsideChar(int pos, int moveDir) {
	//dprintf("NoCRLF %d %d\n", pos, moveDir);
	// If out of range, just return value - should be fixed up after
	if (pos < 0)
		return pos;
	if (pos > Length())
		return pos;

	// Position 0 and Length() can not be between any two characters
	if (pos == 0)
		return pos;
	if (pos == Length())
		return pos;

	// assert pos > 0 && pos < Length()
	if (IsCrLf(pos-1)) {
		if (moveDir > 0)
			return pos + 1;
		else
			return pos - 1;
	}

	// Not between CR and LF

#ifndef GTK
	// DBCS support
	if (dbcsCodePage) {
		// Anchor DBCS calculations at start of line because start of line can
		// not be a DBCS trail byte.
		int startLine = pos;
		while (startLine > 0 && doc.CharAt(startLine) != '\r' && doc.CharAt(startLine) != '\n')
			startLine--;
		bool atLeadByte = false;
		while (startLine < pos) {
			if (atLeadByte)
				atLeadByte = false;
			else if (IsDBCSLeadByteEx(dbcsCodePage, doc.CharAt(startLine)))
				atLeadByte = true;
			else
				atLeadByte = false;
			startLine++;
			//dprintf("DBCS %s\n", atlead ? "D" : "-");
		}
		if (atLeadByte) {
			// Position is between a lead byte and a trail byte
			if (moveDir > 0)
				return pos + 1;
			else
				return pos - 1;
		}
	}
#endif

	return pos;
}

void Scintilla::InvalidateRange(int start, int end) {
	int minPos = start;
	if (minPos > end)
		minPos = end;
	int maxPos = start;
	if (maxPos < end)
		maxPos = end;
	int minLine = LineFromPosition(minPos);
	int maxLine = LineFromPosition(maxPos);
	RECT rcRedraw;
	rcRedraw.left = fixedColumnWidth;
	rcRedraw.top = (minLine - topLine) * lineHeight;
	rcRedraw.right = 32000;
	rcRedraw.bottom = (maxLine - topLine + 1) * lineHeight;
	// Ensure rectangle is within 16 bit space
	rcRedraw.top = clamp(rcRedraw.top, -32000, 32000);
	rcRedraw.bottom = clamp(rcRedraw.bottom, -32000, 32000);

#ifdef GTK
	if (hwnd) {
		GdkRectangle update_rect;
		update_rect.x = rcRedraw.left;
		update_rect.y =  rcRedraw.top;
		update_rect.width = 32000;
		update_rect.height = rcRedraw.bottom - rcRedraw.top + 1;
		gtk_widget_queue_draw_area(draw,
			update_rect.x,
			update_rect.y,
			update_rect.width,
			update_rect.height);
	}
#else
	InvalidateRect(hwnd,&rcRedraw,FALSE);
#endif
}

void Scintilla::SetSelection(int currentPos_, int anchor_) {
	currentPos_ = ClampPositionIntoDocument(currentPos_);
	anchor_ = ClampPositionIntoDocument(anchor_);
	if ((currentPos != currentPos_) || (anchor != anchor_)) {
		int firstAffected = anchor;
		if (firstAffected > currentPos)
			firstAffected = currentPos;
		if (firstAffected > anchor_)
			firstAffected = anchor_;
		if (firstAffected > currentPos_)
			firstAffected = currentPos_;
		int lastAffected = anchor;
		if (lastAffected < currentPos)
			lastAffected = currentPos;
		if (lastAffected < anchor_)
			lastAffected = anchor_;
		if (lastAffected < (currentPos_+1))	// +1 ensures caret repainted
			lastAffected = (currentPos_+1);
		currentPos = currentPos_;
		anchor = anchor_;
		InvalidateRange(firstAffected, lastAffected);
	}
}

void Scintilla::SetSelection(int currentPos_) {
	currentPos_ = ClampPositionIntoDocument(currentPos_);
	if (currentPos != currentPos_) {
		int firstAffected = anchor;
		if (firstAffected > currentPos)
			firstAffected = currentPos;
		if (firstAffected > currentPos_)
			firstAffected = currentPos_;
		int lastAffected = anchor;
		if (lastAffected < currentPos)
			lastAffected = currentPos;
		if (lastAffected < (currentPos_+1))	// +1 ensures caret repainted
			lastAffected = (currentPos_+1);
		currentPos = currentPos_;
		InvalidateRange(firstAffected, lastAffected);
	}
}

void Scintilla::SetPosition(int pos, bool shift) {
	int oldPos = currentPos;
	currentPos = ClampPositionIntoDocument(pos);
	currentPos = MovePositionOutsideChar(currentPos, oldPos - currentPos);
	if (!shift)
		anchor = currentPos;
	EnsureCaretVisible();
}

int Scintilla::LineStart(int line) {
	return doc.LineStart(line);
}

int Scintilla::LineEndPosition(int position) {
	int line = LineFromPosition(position);
	if (line == LinesTotal() - 1)
		position = LineStart(line+1);
	else
		position = LineStart(line+1) - 1;
	if (position > 0 && (doc.CharAt(position-1) == '\r' || doc.CharAt(position-1) == '\n')) {
		position--;
	}
	return position;
}

int Scintilla::VCHomePosition(int position) {
	int line = LineFromPosition(position);
	int startPosition = LineStart(line);
	int endLine = LineStart(line+1) - 1;
	int startText = startPosition;
	while (startText < endLine && (doc.CharAt(startText) == ' ' || doc.CharAt(startText) == '\t' ) )
		startText++;
	if (position == startText)
		return startPosition;
	else
		return startText;
}

int Scintilla::MovePositionTo(int newPos, bool extend) {
	int delta = newPos - currentPos;
	newPos = ClampPositionIntoDocument(newPos);
	newPos = MovePositionOutsideChar(newPos, delta);
	if (extend) {
		SetSelection(newPos);
	} else {
		SetSelection(newPos, newPos);
	}
	ShowCaretAtCurrentPosition();
	EnsureCaretVisible();
	return 0;
}

// Choose the x position that the caret will try to stick to as it is moves up and down
void Scintilla::SetLastXChosen() {
	POINT pt = LocationFromPosition(currentPos);
	lastXChosen = pt.x;
}

void Scintilla::GetTextRect(RECT *prc) {
	GetClientRectangle(prc);
	prc->left += fixedColumnWidth;
}

void Scintilla::ScrollTo(int line) {
	if (line < 0)
		line = 0;
	if (line > MaxScrollPos())
		line = MaxScrollPos();
	topLine = clamp(line, 0, MaxScrollPos());
	SetVertScrollFromTopLine();
	Redraw();
}

void Scintilla::HorizontalScrollTo(int xPos) {
	//dprintf("HorizontalScroll %d\n", xPos);
	xOffset = xPos;
	if (xOffset < 0)
		xOffset = 0;
#ifdef GTK
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), xOffset / 2);
#else
	SetScrollPos(hwnd,SB_HORZ,xOffset,TRUE);
#endif
	Redraw();
}

void Scintilla::EnsureCaretVisible() {
	//dprintf("EnsureCaretVisible %d\n", xOffset);
	RECT rcClient = {0,0,0,0};
	GetTextRect(&rcClient);
	POINT pt = LocationFromPosition(currentPos);
	POINT ptBottomCaret = pt;
	int lineCaret = LineFromPosition(currentPos);
	ptBottomCaret.y += lineHeight;
	if (!PtInRect(&rcClient,pt) || !PtInRect(&rcClient,ptBottomCaret)) {
		//dprintf("EnsureCaretVisible move, (%d,%d) (%d,%d)\n", pt.x, pt.y, rcClient.left, rcClient.right);
		if (topLine > lineCaret) {
			ScrollTo(lineCaret);
		} else if (topLine < (lineCaret - LinesToScroll())) {
			ScrollTo(lineCaret - LinesToScroll());
		}
		// The 2s here are to ensure the caret is really visible
		if (pt.x < rcClient.left) {
			xOffset = xOffset - (rcClient.left - pt.x) - 2;
		} else if (pt.x > rcClient.right) {
			xOffset = xOffset + (pt.x - rcClient.right) + 2;
		}
		if (xOffset < 0)
			xOffset = 0;
#ifdef GTK
		gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), xOffset / 2);
#else
		SetScrollPos(hwnd,SB_HORZ,xOffset,TRUE);
#endif
		Redraw();
	}
}

int Scintilla::SelectionStart() {
	return min(currentPos, anchor);
}

int Scintilla::SelectionEnd() {
	return max(currentPos, anchor);
}

#ifndef GTK
HGLOBAL Scintilla::GetSelText() {
	int bytes = 0;
	int startPos = SelectionStart();
	bytes = SelectionEnd() - startPos;
	HGLOBAL hand = 0;
	LPSTR ptr;
	hand = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,bytes+1);
	if (hand) {
		ptr = (LPSTR)GlobalLock(hand);
		for (int i=0;i<bytes;i++) {
			ptr[i] = doc.CharAt(startPos + i);
		}
		ptr[bytes] = '\0';
		GlobalUnlock(hand);
	}
	return hand;
}
#endif

void Scintilla::Redraw() {
#ifdef GTK
	if (draw) {
		gtk_widget_queue_draw(draw);
	}
#else
	InvalidateRect(hwnd,(LPRECT)NULL,FALSE);
#endif
}

void Scintilla::RedrawSelMargin() {
	if (fixedColumnWidth > 0) {
		RECT rcClient;
		GetClientRectangle(&rcClient);
		rcClient.right = fixedColumnWidth;
#ifdef GTK
		GdkRectangle update_rect;
		update_rect.x = rcClient.left;
		update_rect.y = rcClient.top;
		update_rect.width = fixedColumnWidth;
		update_rect.height = rcClient.bottom - rcClient.top + 1;
		gtk_widget_queue_draw_area(draw,
                           		update_rect.x,
                           		update_rect.y,
                           		update_rect.width,
                           		update_rect.height);
#else
		InvalidateRect(hwnd,&rcClient,FALSE);
#endif
	} else {
		Redraw();
	}
}

void Scintilla::MoveCaret(int x, int y) {
#ifdef GTK
	// Under GTK caret is displayed by expose method so this does nothing
#else
	RECT rcClient;
	GetClientRect(hwnd,&rcClient);
	POINT ptTop = {x,y};
	POINT ptBottom = {x,y+lineHeight};
	bool caretVisible = PtInRect(&rcClient, ptTop) || PtInRect(&rcClient, ptBottom);
	if (GetFocus() != hwnd)
		caretVisible = false;
	if (caret) {
		if (caretVisible) {
			POINT pt;
			GetCaretPos(&pt);
			// Only moving caret when it is really moving reduces flickering
			if (pt.x != x || pt.y != y) {
				if (inOverstrike) {
					SetCaretPos(x,y + lineHeight - 2);
				} else {
					SetCaretPos(x,y);
				}
			}
		} else {
			DropCaret();
		}
	} else {
		if (caretVisible) {
			if (inOverstrike) {
				CreateCaret(hwnd, 0, aveCharWidth-1, 2);
				SetCaretPos(x,y + lineHeight - 2);
			} else {
				CreateCaret(hwnd, 0, 1, lineHeight);
				SetCaretPos(x,y);
			}
			ShowCaret(hwnd);
			caret = true;
		}
	}
#endif
}

void Scintilla::ShowCaretAtCurrentPosition() {
#ifndef GTK
	POINT ptCaret = LocationFromPosition(currentPos);
	MoveCaret(ptCaret.x, ptCaret.y);
#endif
}

void Scintilla::DropCaret() {
#ifndef GTK
	if (caret) {
		HideCaret(hwnd);
		DestroyCaret();
		caret = false;
	}
#endif
}

void Scintilla::PaintSelMargin(Surface *surfWindow, RECT &rc) {
	if (fixedColumnWidth == 0)
		return;

	RECT rcMargin = {0,0,0,0};
	GetClientRectangle(&rcMargin);
	rcMargin.right = fixedColumnWidth;

	RECT rcSelMargin = rcMargin;
	rcSelMargin.left = lineNumberWidth;

	RECT rcis;
	Surface surface;
#ifdef GTK
	if (bufferedDraw) {
		if (!pixmapSelMargin) {
			pixmapSelMargin = gdk_pixmap_new(hwnd->window,
                                 			fixedColumnWidth,
                                 			rcMargin.bottom - rcMargin.top,
                                 			-1);
		}
		surface.Init(surfWindow->hwnd, pixmapSelMargin, surfWindow->gc);
	} else {
		surface.Init(surfWindow->hwnd, surfWindow->hwnd->window, surfWindow->gc);
	}
#else
	BOOL intersects = IntersectRect(&rcis, &rc, &rcMargin);
	if (!intersects)
		return;

	HBITMAP oldBM = 0;
	if (bufferedDraw) {
		if (NULL == bitmapSelMargin) {
			bitmapSelMargin = CreateCompatibleBitmap(
                      			surfWindow->hdc,
                      			fixedColumnWidth,
                      			rcMargin.bottom - rcMargin.top);
		}
		oldBM = static_cast<HBITMAP>(SelectObject(hdcBitmap, bitmapSelMargin));
		surface.Init(hdcBitmap);
	} else {
		surface.Init(surfWindow->hdc);
	}
	SetTextAlign(surface.hdc, TA_BASELINE);
#endif

	//dprintf("Scintilla cleared sel %d %d %d %d\n", rcSelMargin.left, rcSelMargin.top, rcSelMargin.right, rcSelMargin.bottom);
#ifdef GTK
	surface.FillRectangle(rcSelMargin, selbar.allocated);
#else
	// Required because of special way brush is created for selection margin
	FillRect(surface.hdc, &rcSelMargin, selmarginbrush);
#endif

	{	// Scope the line and yposScreen variables
		int line = topLine;
		int yposScreen = 0;
	
		while (line < LinesTotal() && yposScreen < rcMargin.bottom) {
			int marks = doc.GetMark(line);
			if (marks) {
				RECT rcMarker;
				rcMarker.left = 1 + lineNumberWidth;
				rcMarker.top = yposScreen + 1;
				rcMarker.right = lineNumberWidth + selMarginWidth - 1;
				rcMarker.bottom = yposScreen + lineHeight - 1;
				for (int markBit=0;(markBit < 32) && marks; markBit++) {
					if (marks & 1) {
						markers[markBit].Draw(&surface, rcMarker);
					}
					marks >>= 1;
				}
			}
			line++;
			yposScreen += lineHeight;
		}
	}

	if (lineNumberWidth > 0) {
		int line = topLine;
		int ypos = 0;

		while (ypos < rcMargin.bottom) {
			char number[100];
			number[0] = '\0';
			if (line < LinesTotal())
				sprintf(number, "%d", line+1);
			int xpos = 0;
			RECT rcNumber;
			rcNumber.left = xpos;
			rcNumber.right = xpos + lineNumberWidth;
			rcNumber.top = ypos;
			rcNumber.bottom = ypos + lineHeight;
			surface.FillRectangle(rcNumber,selbar.allocated);
			// Right justify
			int width = surface.WidthText(styles[0].font, number, strlen(number));
			xpos += lineNumberWidth - width - 3;
			rcNumber.left = xpos;
#ifdef GTK
			// Different y parameters because Windows is drawing text at base line, GTK+ from bottom
			surface.DrawText(rcNumber, styles[0].font,
				ypos + lineHeight - styles[0].descent, 
				number, strlen(number),
				linenumfore.allocated, selbar.allocated);
#else
			surface.DrawText(rcNumber, styles[0].font,
				ypos + maxAscent, 
				number, strlen(number),
				linenumfore.allocated, selbar.allocated);
#endif
			line++;
			ypos += lineHeight;
		}
	}

	if (bufferedDraw) {
#ifdef GTK
		gdk_draw_pixmap(surfWindow->hwnd->window,
                		surfWindow->hwnd->style->fg_gc[GTK_WIDGET_STATE (hwnd)],
                		pixmapSelMargin,
                		0, 0,
                		0, 0,
                		fixedColumnWidth, rcMargin.bottom - rcMargin.top);
#else
		BitBlt(surfWindow->hdc,rcMargin.left, rcMargin.top, rcMargin.right, rcMargin.bottom, 
			hdcBitmap, 0, 0, SRCCOPY);
		SelectObject(hdcBitmap, oldBM);
#endif
	}
}

#ifndef GTK
void Scintilla::CreateGraphicObjects(HDC hdc) {
	hdcBitmap = CreateCompatibleDC(hdc);

	HBITMAP selMap = CreateCompatibleBitmap(hdc,8,8);
	oldBitmap = static_cast<HBITMAP>(SelectObject(hdcBitmap, selMap));

	// This complex procedure is to reproduce the checker board dithered pattern used by windows 
	// for scroll bars and Visual Studio for its selection margin. The colour of this pattern is half
	// way between the chrome colour and the chrome highlight colour making a nice transition
	// between the window chrome and the content area. And it works in low colour depths.
	COLORREF highlight = GetSysColor(COLOR_3DHILIGHT);
	if (highlight == RGB(0xff,0xff,0xff)) {
		RECT rcPattern = {0,0,8,8};
		FillRect(hdcBitmap,&rcPattern,(HBRUSH)GetStockObject(WHITE_BRUSH));
		HPEN penSel = CreatePen(0,1,GetSysColor(COLOR_3DFACE));
		HPEN penOld = static_cast<HPEN>(SelectObject(hdcBitmap, penSel));
		for (int stripe=0;stripe<8;stripe++) {
			MoveToEx(hdcBitmap, 0, stripe * 2, 0);
			LineTo(hdcBitmap, 8, stripe * 2 - 8);
		}
		selmarginbrush = CreatePatternBrush(selMap);

		SelectObject(hdcBitmap, penOld);
		DeleteObject(penSel);
	} else {
		// User has choen an unusual chrome colour scheme so just use the highlight edge colour.
		selmarginbrush = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
	}

	RECT rcClient = {0,0,0,0};
	GetClientRect(hwnd,&rcClient);
	bitmapLineBuffer = CreateCompatibleBitmap(hdc, rcClient.right - rcClient.left, lineHeight);
	SelectObject(hdcBitmap, bitmapLineBuffer);
	DeleteObject(selMap);
}
#endif

void Scintilla::Paint(RECT rcPaint) {

	RefreshStyleData();

#ifdef GTK
	if (bufferedDraw) {
		if (!pixmapLine) {
			pixmapLine = gdk_pixmap_new(draw->window,
                            			draw->allocation.width,
                            			lineHeight,
                            			-1);
		}
	}
#else
	PAINTSTRUCT ps;
	BeginPaint(hwnd,&ps);
	if (NULL == hdcBitmap) {
		CreateGraphicObjects(ps.hdc);
	}

	HPALETTE hpalSaveBM = palette.SelectInto(hdcBitmap,GetFocus() != hwnd);
	HPALETTE hpalSave = palette.SelectInto(ps.hdc,TRUE);
	if (palette.allowRealization) {
		RealizePalette(ps.hdc);
		RealizePalette(hdcBitmap);
	}
	rcPaint = ps.rcPaint;
#endif
	//dprintf("Paint: (%3d,%3d) ... (%3d,%3d)   %d\n",
	//	rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);

	RECT rcClient = {0,0,0,0};
	GetClientRectangle(&rcClient);
	//dprintf("Client: (%3d,%3d) ... (%3d,%3d)   %d\n",
	//	rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

	int screenLinePaintFirst = rcPaint.top / lineHeight;
	int linePaintLast = topLine + rcPaint.bottom / lineHeight + 1;
	int endPosPaint = Length();
	if (linePaintLast < LinesTotal())
		endPosPaint = LineStart(linePaintLast+1)-1;

	if (endPosPaint > endStyled) {
		// Notify container to do some more styling
		NotifyStyleNeeded(endPosPaint);
	}

#ifdef GTK
	GdkGC *gc = gdk_gc_new(hwnd->window);
#endif

	int sty = 0;
	char segment[maxLineLength];
	int xpos = fixedColumnWidth;
	int ypos = 0;
	if (!bufferedDraw)
		ypos += screenLinePaintFirst * lineHeight;
	int yposScreen = screenLinePaintFirst * lineHeight;

	//dprintf("start display %d tabWidth = %d, margin = %d offset = %d\n", doc.Length(), tabWidth, selMarginWidth, xOffset);
	int selStart = SelectionStart();
	int selEnd = SelectionEnd(); {
		Surface marginSurface;
#ifdef GTK
		marginSurface.Init(draw, draw->window, gc);
#else
		marginSurface.Init(ps.hdc);
#endif
		PaintSelMargin(&marginSurface,rcPaint);
	}

	Surface *surface = new Surface();
	if (rcPaint.right > selMarginWidth) {

#ifdef GTK
		GdkDrawable *hdcShow = draw->window;
		if (bufferedDraw) {
			hdcShow = pixmapLine;
		}
		surface->Init(draw, hdcShow, gc);
#else
		HDC hdcShow = ps.hdc;
		if (bufferedDraw) {
			SelectObject(hdcBitmap, bitmapSelMargin);
			hdcShow = hdcBitmap;
		}
		surface->Init(hdcShow);
		SelectObject(hdcBitmap, bitmapLineBuffer);
#endif

		int line = topLine + screenLinePaintFirst;

		// Remove selection margin from drawing area so text will not be drawn
		// on it in unbuffered mode.
#ifndef GTK
		IntersectClipRect(ps.hdc, fixedColumnWidth, 0, 32000, 32000);
#endif
		while (line < LinesTotal() && yposScreen < rcPaint.bottom) {

			int marks = 0;
			COLORREF markBack = RGB(0,0,0);
			if (selMarginWidth == 0) {
				marks = doc.GetMark(line);
				if (marks) {
					for (int markBit=0;(markBit < 32) && marks; markBit++) {
						if (marks & 1) {
							markBack = markers[markBit].back.allocated;
						}
						marks >>= 1;
					}
				}
				marks = doc.GetMark(line);
			}

			segment[0] = '\0';
			int segPos = 0;

			int posLineStart = LineStart(line);
			int posLineEnd = LineStart(line+1);
			//dprintf("line %d %d - %d\n", line, posLineStart, posLineEnd);

			RECT rcBlank;
			rcBlank.top = ypos;
			rcBlank.bottom = ypos + lineHeight;

			bool inSelection = posLineStart > selStart && posLineStart < selEnd;

#ifndef GTK
			SetTextAlign(hdcShow, TA_BASELINE);
#endif

			int indicatorsSet = 0;
			if (posLineStart < posLineEnd)
				indicatorsSet = doc.StyleAt(posLineStart) & INDICS_MASK;

			int prevIndic = doc.StyleAt(posLineStart) & INDICS_MASK;


			int indStart[INDIC_MAX+1] = {0};
			for (int indica=0; indica <= INDIC_MAX; indica++) 
				indStart[indica] = fixedColumnWidth;

			char chPrev = '\0';
			bool visibleInSelection = false;
			int ch = ' ';
			int colour = -1;

			int xposCaret = -1;

			for (int i=posLineStart;i<=posLineEnd;i++) {
				//dprintf("pos %d\n", line, posLineStart, posLineEnd);
				if (i < posLineEnd) {	// Do not index onto next line
					ch = doc.CharAt(i);
					colour = doc.StyleAt(i);
				}

				// If there is the end of a style run for any reason
				if (colour != sty || (ch == '\t') || (chPrev == '\t') || (i == selStart) || (i == selEnd) || (i == posLineEnd)) {
					int styleMain = sty & 31;
					// text appears not to have a background color, so draw backing rect first
					COLORREF textBack = styles[styleMain].back.allocated;
					COLORREF textFore = styles[styleMain].fore.allocated;
					if (inSelection && !hideSelection) {
						if (selbackset)
							textBack = selbackground.allocated;
						if (selforeset)
							textFore = selforeground.allocated;
						visibleInSelection = true;
					} else {
						if (marks)
							textBack = markBack;
					}
					unsigned int width = 0;
					if (segment[0] == '\t') {
						int nextTab = (((xpos + 2 - fixedColumnWidth) / tabWidth) + 1) * tabWidth +
              						fixedColumnWidth;
						rcBlank.left = xpos - xOffset;
						rcBlank.right = nextTab - xOffset;
						surface->FillRectangle(rcBlank, textBack);
						if (viewWhitespace) {
							RECT rcTab;
							rcTab.left = rcBlank.left + 1 - xOffset;
							rcTab.top = rcBlank.top + 4;
							rcTab.right = rcBlank.right - 1 - xOffset;
							rcTab.bottom = rcBlank.bottom - maxDescent;
							int ymid = ypos + lineHeight/2;
							int ydiff = (rcTab.bottom - rcTab.top) / 2;
							int xhead = rcTab.right - 1 - ydiff;
							surface->PenColor(textFore);
							if ((rcTab.left + 2) < (rcTab.right - 1))
								surface->MoveTo(rcTab.left + 2, ymid);
							else
								surface->MoveTo(rcTab.right - 1, ymid);
							surface->LineTo(rcTab.right - 1, ymid);
							surface->LineTo(xhead, ymid - ydiff);
							surface->MoveTo(rcTab.right - 1, ymid);
							surface->LineTo(xhead, ymid + ydiff);
						}
						width = nextTab - xpos;
					} else {
						//dprintf("pos %d %d %s\n", line, i, segment);
						width = surface->WidthText(styles[styleMain].font, segment, segPos);
						rcBlank.left = xpos - xOffset;
						rcBlank.right = xpos + width - xOffset;
						rcBlank.top = ypos;
						rcBlank.bottom = ypos + lineHeight;
#ifdef GTK
						// Different y parameters because Windows is drawing text at base line, GTK+ from bottom
						surface->DrawText(rcBlank, styles[styleMain].font,
                  						ypos + lineHeight - styles[styleMain].descent, segment, segPos,
                  						textFore, textBack);
#else
						surface->DrawText(rcBlank, styles[styleMain].font,
                  						ypos + maxAscent, segment, segPos,
                  						textFore, textBack);
#endif
						if (viewWhitespace) {
							surface->PenColor(textFore);
							int xx = xpos;
							for (int cpos=0;cpos < segPos;cpos++) {
								int szc = surface->WidthChar(styles[styleMain].font, segment[cpos]);
								if (segment[cpos] == ' ') {
									surface->MoveTo(xx + szc / 2 - xOffset, ypos + lineHeight/2);
									surface->LineTo(xx + szc / 2 - xOffset + 1, ypos + lineHeight/2);
								}
								xx += szc;
							}
						}
					}
					segPos = 0;
					segment[segPos] = '\0';
					xpos += width;
					sty = colour;

					if (i == selStart)
						inSelection = true;

					if (i == selEnd)
						inSelection = false;

					if (i == currentPos && (i<posLineEnd || (line == LinesTotal()-1))) {
						xposCaret = xpos;
					}

					if (i < posLineEnd) {	// Do not index onto next line
						indicatorsSet = sty & INDICS_MASK;
					}
					if (indicatorsSet != prevIndic) {
						int mask = INDIC0_MASK;
						int indicnum = 0;
						for (indicnum=0; indicnum <= INDIC_MAX; indicnum++) {
							if ((indicatorsSet & mask) && !(prevIndic & mask)) {
								indStart[indicnum] = xpos;
							}
							mask = mask << 1;
						}

						mask = INDIC0_MASK;
						for (indicnum=0; indicnum <= INDIC_MAX; indicnum++) {
							if (!(indicatorsSet & mask) && (prevIndic & mask)) {
								RECT rcIndic = {
    								indStart[indicnum] - xOffset,
    								ypos + maxAscent,
    								xpos - xOffset,
    								ypos + maxAscent + 3};
								indicators[indicnum].Draw(surface, rcIndic);
							}
							mask = mask << 1;
						}
						prevIndic = indicatorsSet;
					}
				}

				if (ch != '\r' && ch != '\n' && ((segPos+1) < sizeof(segment)))
					segment[segPos++] = ch;
				chPrev = ch;
			}

			rcBlank.left = xpos - xOffset;
			rcBlank.right = xpos + aveCharWidth - xOffset;
			if (inSelection && !hideSelection && visibleInSelection && selbackset) {
				surface->FillRectangle(rcBlank, selbackground.allocated);
			} else if (marks) {
				surface->FillRectangle(rcBlank, markBack);
			} else {
				surface->FillRectangle(rcBlank, background.allocated);
			}

			rcBlank.left = xpos + aveCharWidth - xOffset;
			rcBlank.right = rcClient.right;
			if (marks) {
				surface->FillRectangle(rcBlank, markBack);
			} else {
				surface->FillRectangle(rcBlank, background.allocated);
			}

#ifdef GTK
			// TODO: Add optional blinking here, enable for windows and make this the only
			// caret drawing code.
			if (xposCaret >= 0) {
				rcBlank.left = xposCaret - xOffset;
				rcBlank.right = xposCaret - xOffset + 1;
				surface->FillRectangle(rcBlank, caretcolour.allocated);
			}
#endif
			if (bufferedDraw) {
#ifdef GTK
				gdk_draw_pixmap(draw->window,
                				draw->style->fg_gc[GTK_WIDGET_STATE (hwnd)],
                				pixmapLine,
                				fixedColumnWidth, 0,
                				fixedColumnWidth, yposScreen,
                				draw->allocation.width, lineHeight);
#else
				BitBlt(ps.hdc,fixedColumnWidth,yposScreen,rcClient.right - rcClient.left,
       					lineHeight+1,hdcBitmap,fixedColumnWidth,0,SRCCOPY);
#endif
			}

			if (!bufferedDraw) {
				ypos += lineHeight;
			}

			yposScreen += lineHeight;
			xpos = fixedColumnWidth;
			line++;
		}
		RECT rcBeyondEOF = rcClient;
		rcBeyondEOF.left = fixedColumnWidth;
		rcBeyondEOF.right = rcBeyondEOF.right;
		rcBeyondEOF.top = (LinesTotal() - topLine) * lineHeight;
		if (rcBeyondEOF.top < rcBeyondEOF.bottom) {
			Surface endSurface;
#ifdef GTK
			endSurface.Init(draw, draw->window, gc);
#else
			endSurface.Init(ps.hdc);
#endif
			endSurface.FillRectangle(rcBeyondEOF, background.allocated);
		}
	}

	delete surface;
#ifdef GTK
	gdk_gc_unref(gc);
#else
	if (hpalSaveBM)
		SelectPalette(hdcBitmap,hpalSaveBM,TRUE);
	if (hpalSave)
		SelectPalette(ps.hdc,hpalSave,TRUE);
	EndPaint(hwnd,&ps);

	ShowCaretAtCurrentPosition();
#endif
}

void Scintilla::RealizeWindowPalette(bool inBackGround) {
#ifdef GTK
#else
	RefreshStyleData();
	HDC hdc = GetDC(hwnd);
	HPALETTE hpalSave = palette.SelectInto(hdc, inBackGround);
	if (hpalSave) {
		int changes = RealizePalette(hdc);
		SelectPalette(hdc, hpalSave, TRUE);
		if (changes > 0)
			Redraw();
	}
	ReleaseDC(hwnd,hdc);
#endif
}

void Scintilla::ModifiedAt(int pos) {
	if (endStyled > pos)
		endStyled = pos;
}

// Document only modified by gateways DeleteChars, InsertStyledString, Undo, Redo, and SetStyleAt.
// SetStyleAt does not change the persistent state of a document

// Unlike Undo, Redo, and InsertStyledString, the pos argument is a cell number not a char number
void Scintilla::DeleteChars(int pos, int len) {
	if (doc.IsReadOnly())
		NotifyModifyAttempt();
	if (!doc.IsReadOnly()) {
		bool startSavePoint = doc.IsSavePoint();
		doc.DeleteChars(pos*2, len * 2);
		if (startSavePoint && doc.IsCollectingUndo())
			NotifySavePoint(!startSavePoint);
		ModifiedAt(pos);
		NotifyChange();
		SetScrollBars();
	}
}

void Scintilla::InsertStyledString(int position, char *s, int insertLength) {
	if (doc.IsReadOnly())
		NotifyModifyAttempt();
	if (!doc.IsReadOnly()) {
		bool startSavePoint = doc.IsSavePoint();
		doc.InsertString(position, s, insertLength);
		if (startSavePoint && doc.IsCollectingUndo())
			NotifySavePoint(!startSavePoint);
		ModifiedAt(position / 2);
		NotifyChange();
		SetScrollBars();
	}
}

void Scintilla::Undo() {
	if (doc.CanUndo()) {
		bool startSavePoint = doc.IsSavePoint();
		int earliestMod = Length();
		int newPos = doc.Undo(&earliestMod) / 2;
		SetSelection(newPos, newPos);
		EnsureCaretVisible();
		ModifiedAt(earliestMod / 2);
		NotifyChange();
		Redraw();
		bool endSavePoint = doc.IsSavePoint();
		if (startSavePoint != endSavePoint)
			NotifySavePoint(endSavePoint);
		SetScrollBars();
	}
}

void Scintilla::Redo() {
	if (doc.CanRedo()) {
		bool startSavePoint = doc.IsSavePoint();
		int earliestMod = Length();
		int newPos = doc.Redo(&earliestMod) / 2;
		SetSelection(newPos, newPos);
		EnsureCaretVisible();
		ModifiedAt(earliestMod / 2);
		NotifyChange();
		Redraw();
		bool endSavePoint = doc.IsSavePoint();
		if (startSavePoint != endSavePoint)
			NotifySavePoint(endSavePoint);
		SetScrollBars();
	}
}

void Scintilla::InsertChar(int pos, char ch) {
	char chs[2];
	chs[0] = ch;
	chs[1] = 0;
	InsertStyledString(pos*2, chs, 2);
}

// Insert a null terminated string
void Scintilla::InsertString(int position, char *s) {
	InsertString(position, s, strlen(s));
}

// Insert a string with a length
void Scintilla::InsertString(int position, char *s, int insertLength) {
	char *sWithStyle = new char[insertLength*2];
	if (sWithStyle) {
		for (int i=0; i<insertLength; i++) {
			sWithStyle[i*2] = s[i];
			sWithStyle[i*2+1] = 0;
		}
		InsertStyledString(position*2, sWithStyle, insertLength*2);
		delete []sWithStyle;
	}
}


void Scintilla::ClearAll() {
	if (0 != Length()) {
		DeleteChars(0, Length());
	}
	anchor = 0;
	currentPos = 0;
	topLine = 0;
	SetVertScrollFromTopLine();
	Redraw();
}

void Scintilla::ClearSelection() {
	int startPos = SelectionStart();
	unsigned int chars = SelectionEnd() - startPos;
	SetSelection(startPos, startPos);
	if (0 != chars) {
		DeleteChars(startPos, chars);
		NotifyChange();
	}
}

#ifdef GTK
static gint scintilla_signals[LAST_SIGNAL] = { 0 };
#endif

void Scintilla::NotifyChange() {
	isModified = true;
#ifdef GTK
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
                	MAKELONG(ctrlID, EN_CHANGE), hwnd);
#else
	SendMessage(GetParent(hwnd), WM_COMMAND,
            	MAKELONG(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
#endif
}

void Scintilla::NotifyStyleNeeded(int endStyleNeeded) {
	//dprintf("Notify style need %d\n", endStyleNeeded);
	SCNotification scn;
	scn.nmhdr.hwndFrom = hwnd;
	scn.nmhdr.idFrom = ctrlID;
	scn.nmhdr.code = SCN_STYLENEEDED;
	scn.position = endStyleNeeded;
#ifdef GTK
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
#else
	SendMessage(GetParent(hwnd), WM_NOTIFY,
            	GetDlgCtrlID(hwnd), reinterpret_cast<LPARAM>(&scn));
#endif
}

void Scintilla::NotifyChar(char ch) {
	SCNotification scn;
	scn.nmhdr.hwndFrom = hwnd;
	scn.nmhdr.idFrom = ctrlID;
	scn.nmhdr.code = SCN_CHARADDED;
	scn.ch = ch;
#ifdef GTK
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
#else
	SendMessage(GetParent(hwnd), WM_NOTIFY,
            	GetDlgCtrlID(hwnd), reinterpret_cast<LPARAM>(&scn));
#endif
}

void Scintilla::NotifySavePoint(bool isSavePoint) {
	SCNotification scn;
	scn.nmhdr.hwndFrom = hwnd;
	scn.nmhdr.idFrom = ctrlID;
	if (isSavePoint) {
		scn.nmhdr.code = SCN_SAVEPOINTREACHED;
	} else {
		scn.nmhdr.code = SCN_SAVEPOINTLEFT;
	}
#ifdef GTK
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
#else
	SendMessage(GetParent(hwnd), WM_NOTIFY,
            	GetDlgCtrlID(hwnd), reinterpret_cast<LPARAM>(&scn));
#endif
}

void Scintilla::NotifyModifyAttempt() {
	SCNotification scn;
	scn.nmhdr.hwndFrom = hwnd;
	scn.nmhdr.idFrom = ctrlID;
	scn.nmhdr.code = SCN_MODIFYATTEMPTRO;
#ifdef GTK
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
#else
	SendMessage(GetParent(hwnd), WM_NOTIFY,
            	GetDlgCtrlID(hwnd), reinterpret_cast<LPARAM>(&scn));
#endif
}

void Scintilla::NotifyKey(int key, int modifiers) {
#ifdef GTK
	SCNotification scn;
	scn.nmhdr.hwndFrom = hwnd;
	scn.nmhdr.idFrom = ctrlID;
	scn.nmhdr.code = SCN_KEY;
	scn.ch = key;
	scn.modifiers = modifiers;

	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
#endif
}

void Scintilla::Indent(bool forwards) {
	int lineAnchor = LineFromPosition(anchor);
	int lineCurrentPos = LineFromPosition(currentPos);
	if (lineAnchor == lineCurrentPos) {
		ClearSelection();
		InsertChar(currentPos++, '\t');
		SetSelection(currentPos, currentPos);
	} else {
		int anchorPosOnLine = anchor - LineStart(lineAnchor);
		int currentPosPosOnLine = currentPos - LineStart(lineCurrentPos);
		// Multiple lines selected so indent / dedent
		int lineTopSel = min(lineAnchor, lineCurrentPos);
		int lineBottomSel = max(lineAnchor, lineCurrentPos);
		if (LineStart(lineBottomSel) == anchor || LineStart(lineBottomSel) == currentPos)
			lineBottomSel--;	// If not selecting any characters on a line, do not indent
		if (!forwards) {
			// Dedent - suck white space off the front of the line to dedent by equivalent of a tab
			for (int line=lineBottomSel; line >= lineTopSel; line--) {
				int ispc = 0;
				while (ispc < tabInChars && doc.CharAt(LineStart(line) + ispc) == ' ')
					ispc++;
				int posStartLine = LineStart(line);
				if (ispc == tabInChars) {
					DeleteChars(posStartLine,ispc);
				} else if (doc.CharAt(posStartLine + ispc) == '\t') {
					DeleteChars(posStartLine,ispc+1);
				} else {	// Hit a non-white
					DeleteChars(posStartLine,ispc);
				}
			}
		} else {
			// Indent by a tab
			for (int line=lineBottomSel; line >= lineTopSel; line--) {
				InsertChar(LineStart(line), '\t');
			}
		}
		if (lineAnchor < lineCurrentPos) {
			if (currentPosPosOnLine == 0)
				SetSelection(LineStart(lineCurrentPos), LineStart(lineAnchor));
			else
				SetSelection(LineStart(lineCurrentPos+1), LineStart(lineAnchor));
		} else {
			if (anchorPosOnLine == 0)
				SetSelection(LineStart(lineCurrentPos), LineStart(lineAnchor));
			else
				SetSelection(LineStart(lineCurrentPos), LineStart(lineAnchor+1));
		}
	}
}

static bool iswordchar(char ch) {
	return isalnum(ch) || ch == '_';
}

int Scintilla::ExtendWordSelect(int pos, int delta) {
	int newPos = pos;
	if (delta < 0) {
		while (newPos > 0 && iswordchar(doc.CharAt(newPos-1)))
			newPos--;
	} else {
		while (newPos < (Length()-1) && iswordchar(doc.CharAt(newPos)))
			newPos++;
	}
	return newPos;
}

int Scintilla::NextWordStart(int delta) {
	int newPos = currentPos;
	if (delta < 0) {
		while (newPos > 0 && (doc.CharAt(newPos-1) == ' ' || doc.CharAt(newPos-1) == '\t'))
			newPos--;
		if (isspace(doc.CharAt(newPos-1))) {	// Back up to previous line
			while (newPos > 0 && isspace(doc.CharAt(newPos-1)))
				newPos--;
		} else {
			bool startAtWordChar = iswordchar(doc.CharAt(newPos-1));
			while (newPos > 0 && !isspace(doc.CharAt(newPos-1)) && (startAtWordChar == iswordchar(doc.CharAt(newPos-1))))
				newPos--;
		}
	} else {
		bool startAtWordChar = iswordchar(doc.CharAt(newPos));
		while (newPos < (Length()-1) && isspace(doc.CharAt(newPos)))
			newPos++;
		while (newPos < (Length()-1) && !isspace(doc.CharAt(newPos)) && (startAtWordChar == iswordchar(doc.CharAt(newPos))))
			newPos++;
		while (newPos < (Length()-1) && (doc.CharAt(newPos) == ' ' || doc.CharAt(newPos) == '\t'))
			newPos++;
	}
	return newPos;
}

POINT MakePoint(int x, int y) {
	POINT pt = {x, y};
	return pt;
}

int Scintilla::KeyCommand(WORD iMessage) {
	POINT pt = LocationFromPosition(currentPos);

	// Most key commands cancel autocompletion mode
	if (inAutoCompleteMode) {
		switch (iMessage) {
			// Except for these
			case SCI_LINEDOWN:
			case SCI_LINEUP:
			case SCI_DELETEBACK:
			case SCI_TAB:
				break;

			default:
				AutoCompleteCancel();
		}
	}

	switch (iMessage) {
	case SCI_LINEDOWN:
		if (inAutoCompleteMode) {
			AutoCompleteMove(1);
		} else {
			return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y + lineHeight)));
		}
		return 0;
	case SCI_LINEDOWNEXTEND:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y + lineHeight)), true);
	case SCI_LINEUP:
		if (inAutoCompleteMode) {
			AutoCompleteMove(-1);
		} else {
			MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y - lineHeight)));
		}
		return 0;
	case SCI_LINEUPEXTEND:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y - lineHeight)), true);
	case SCI_CHARLEFT:
		MovePositionTo(currentPos - 1);
		SetLastXChosen();
		return 0;
	case SCI_CHARLEFTEXTEND:
		MovePositionTo(currentPos - 1, true);
		SetLastXChosen();
		return 0;
	case SCI_CHARRIGHT:
		MovePositionTo(currentPos + 1);
		SetLastXChosen();
		return 0;
	case SCI_CHARRIGHTEXTEND:
		MovePositionTo(currentPos + 1, true);
		SetLastXChosen();
		return 0;
	case SCI_WORDLEFT:
		return MovePositionTo(NextWordStart(-1));
	case SCI_WORDLEFTEXTEND:
		return MovePositionTo(NextWordStart(-1), true);
	case SCI_WORDRIGHT:
		return MovePositionTo(NextWordStart(1));
	case SCI_WORDRIGHTEXTEND:
		return MovePositionTo(NextWordStart(1), true);
	case SCI_HOME:
		MovePositionTo(LineStart(LineFromPosition(currentPos)));
		SetLastXChosen();
		return 0;
	case SCI_HOMEEXTEND:
		MovePositionTo(LineStart(LineFromPosition(currentPos)), true);
		SetLastXChosen();
		return 0;
	case SCI_LINEEND:
		MovePositionTo(LineEndPosition(currentPos));
		SetLastXChosen();
		return 0;
	case SCI_LINEENDEXTEND:
		MovePositionTo(LineEndPosition(currentPos), true);
		SetLastXChosen();
		return 0;
	case SCI_DOCUMENTSTART:
		MovePositionTo(0);
		SetLastXChosen();
		return 0;
	case SCI_DOCUMENTSTARTEXTEND:
		MovePositionTo(0, true);
		SetLastXChosen();
		return 0;
	case SCI_DOCUMENTEND:
		MovePositionTo(Length());
		SetLastXChosen();
		return 0;
	case SCI_DOCUMENTENDEXTEND:
		MovePositionTo(Length(), true);
		SetLastXChosen();
		return 0;
	case SCI_PAGEUP:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y - lineHeight * LinesToScroll())));
	case SCI_PAGEUPEXTEND:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y - lineHeight * LinesToScroll())), true);
	case SCI_PAGEDOWN:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y + lineHeight * LinesToScroll())));
	case SCI_PAGEDOWNEXTEND:
		return MovePositionTo(PositionFromLocation(MakePoint(lastXChosen, pt.y + lineHeight * LinesToScroll())), true);
	case SCI_EDITTOGGLEOVERTYPE:
		inOverstrike = !inOverstrike;
		DropCaret();
		ShowCaretAtCurrentPosition();
		return 0;
	case SCI_CANCEL:
		CallTipCancel();
		return 0;
	case SCI_DELETEBACK:
		DelCharBack();
		if (inAutoCompleteMode)
			AutoCompleteChanged();
		if (inCallTipMode && (posStartCallTip > currentPos))
			CallTipCancel();
		NotifyChange();
		EnsureCaretVisible();
		break;
	case SCI_TAB:
		if (inAutoCompleteMode) {
			AutoCompleteCompleted();
		} else {
			Indent(true);
		}
		NotifyChange();
		break;
	case SCI_BACKTAB:
		Indent(false);
		break;
	case SCI_NEWLINE:
		AutoCompleteCancel();
		CallTipCancel();
		ClearSelection();
		if (eolMode == SC_EOL_CRLF) {
			InsertString(currentPos, "\r\n");
			SetSelection(currentPos+2, currentPos+2);
		} else if (eolMode == SC_EOL_CR) {
			InsertChar(currentPos, '\r');
			SetSelection(currentPos+1, currentPos+1);
		} else if (eolMode == SC_EOL_LF) {
			InsertChar(currentPos, '\n');
			SetSelection(currentPos+1, currentPos+1);
		}
		EnsureCaretVisible();
		break;
	case SCI_FORMFEED:
		AddChar('\f');
		break;
	case SCI_VCHOME:
		MovePositionTo(VCHomePosition(currentPos));
		SetLastXChosen();
		return 0;
	case SCI_VCHOMEEXTEND:
		MovePositionTo(VCHomePosition(currentPos), true);
		SetLastXChosen();
		break;

	}
	return 0;
}

void Scintilla::AssignCmdKey(int key, int modifiers, int msg) {
	if ((keymapLen+1) >= keymapAlloc) {
		KeyToCommand *ktcNew = new KeyToCommand[keymapAlloc + 5];
		if (!ktcNew)
			return;
		for (int k=0;k<keymapLen;k++)
			ktcNew[k] = keymap[k];
		keymapAlloc += 5;
		delete []keymap;
		keymap = ktcNew;
	}
	for (int keyIndex = 0; keyIndex < keymapLen; keyIndex++) {
		if ((key == keymap[keyIndex].key) && (modifiers == keymap[keyIndex].modifiers)) {
			keymap[keyIndex].msg = msg;
			return;
		}
	}
	keymap[keymapLen].key = key;
	keymap[keymapLen].modifiers = modifiers;
	keymap[keymapLen].msg = msg;
	keymapLen++;
}

void Scintilla::SetWhitespaceVisible(bool view) {
	viewWhitespace = view;
}

bool Scintilla::GetWhitespaceVisible() {
	return viewWhitespace;
}

void Scintilla::DelChar() {
	if (IsCrLf(currentPos)) {
		DeleteChars(currentPos,2);
	} else if (currentPos < Length()) {
		DeleteChars(currentPos,1);
	}
	NotifyChange();
	Redraw();
}

void Scintilla::DelCharBack() {
	if (currentPos > 0) {
		if (IsCrLf(currentPos-2)) {
			DeleteChars(currentPos-2,2);
			SetSelection(currentPos-2, currentPos-2);
		} else {
			DeleteChars(currentPos-1,1);
			SetSelection(currentPos-1, currentPos-1);
		}
	}
	NotifyChange();
}

void Scintilla::AddChar(char ch) {
	bool wasSelection = currentPos != anchor;
	ClearSelection();
	if (inOverstrike && !wasSelection) {
		if (currentPos < (Length() - 1)) {
			if ((doc.CharAt(currentPos) != '\r') && (doc.CharAt(currentPos) != '\n')) {
				DeleteChars(currentPos,1);
			}
		}
	}
	InsertChar(currentPos, ch);
	SetSelection(currentPos+1, currentPos+1);
	if (inAutoCompleteMode)
		AutoCompleteChanged(ch);
	EnsureCaretVisible();
	NotifyChange();
	Redraw();
	NotifyChar(ch);
}

static bool Close(POINT pt1, POINT pt2) {
	if (abs(pt1.x - pt2.x) > 3)
		return false;
	if (abs(pt1.y - pt2.y) > 3)
		return false;
	return true;
}

static POINT PointFromLparam(LPARAM lParam) {
	POINT pt;
	pt.x = (int)(short)LOWORD(lParam);
	pt.y = (int)(short)HIWORD(lParam);
	return pt;
}

COLORREF ColourFromLparam(LPARAM lParam) {
#ifdef GTK
	return RGB(lParam >> 16, (lParam >> 8) & 0xff, lParam & 0xff);
#else
	return lParam;
#endif
}

LPARAM LparamFromColour(COLORREF col) {
#ifdef GTK
	return col.pixel;
#else
	return col;
#endif
}

void Scintilla::ButtonDown(POINT pt, unsigned int curTime, bool shift) {
	AutoCompleteCancel();
	CallTipCancel();
	Capture();
#ifdef GTK
	gtk_widget_grab_focus(GTK_WIDGET(sci));
#endif
	//dprintf("Scintilla:ButtonDown %d %d = %d\n", curTime, lastClickTime, curTime - lastClickTime);
	int newPos = PositionFromLocation(pt);
	newPos = MovePositionOutsideChar(newPos, currentPos - newPos);
	if (shift)
		SetSelection(newPos);
	else
		SetSelection(newPos, newPos);
	if ((curTime - lastClickTime) < GetDoubleClickTime() && Close(pt,lastClick)) {
		// Stop mouse button bounce changing selection type
		if (curTime != lastClickTime) {
			if (selType == selChar) {
#ifndef GTK
				// Send myself a WM_LBUTTONDBLCLK, so the container can handle it too.
				SendMessage(hwnd, WM_LBUTTONDBLCLK, 
					shift ? MK_SHIFT : 0, 
					MAKELPARAM(pt.x, pt.y));
#endif
				selType = selWord;
			} else if (selType == selWord) {
				selType = selLine;
			} else {
				selType = selChar;
			}
		}
	
		if (selType == selWord) {
			if (currentPos >= originalAnchorPos) {	// Moved forward
				SetSelection(ExtendWordSelect(currentPos, 1),
             				ExtendWordSelect(originalAnchorPos, -1));
			} else {	// Moved backward
				SetSelection(ExtendWordSelect(currentPos, -1),
             				ExtendWordSelect(originalAnchorPos, 1));
			}
		} else if (selType == selLine) {
			lineAnchor = LineFromLocation(pt);
			SetSelection(LineStart(lineAnchor+1), LineStart(lineAnchor));
			//dprintf("Triple click: %d - %d\n", anchor, currentPos);
		} else {
			SetSelection(currentPos, currentPos);
		}
		//dprintf("Double click: %d - %d\n", anchor, currentPos);
	} else {
		selType = selChar;
		originalAnchorPos = currentPos;
	}
	lastClickTime = curTime;
#ifdef GTK
	Redraw();
#endif
	lastXChosen = pt.x;
	ShowCaretAtCurrentPosition();
}

void Scintilla::ButtonMove(POINT pt) {
	//dprintf("Move %x\n", lParam);
	if (capturedMouse) {
		int movePos = PositionFromLocation(pt);
		movePos = MovePositionOutsideChar(movePos, currentPos - movePos);
		if (selType == selChar) {
			SetSelection(movePos);
			//dprintf("Move: %d - %d\n", anchor, currentPos);
		} else if (selType == selWord) {
			// continue selecting by word
			if (currentPos > originalAnchorPos) {	// Moved forward
				SetSelection(ExtendWordSelect(movePos, 1),
             				ExtendWordSelect(originalAnchorPos, -1));
			} else {	// Moved backward
				SetSelection(ExtendWordSelect(movePos, -1),
             				ExtendWordSelect(originalAnchorPos, 1));
			}
		} else {
			// continue selecting by line
			//int lineMove = LineFromPosition(movePos);
			int lineMove = LineFromLocation(pt);
			if (lineAnchor < lineMove) {
				SetSelection(LineStart(lineMove+1),
             				LineStart(lineAnchor));
			} else {
				SetSelection(LineStart(lineAnchor+1),
             				LineStart(lineMove));
			}
		}
		EnsureCaretVisible();
	}
}

void Scintilla::ButtonUp(POINT pt, unsigned int curTime) {
	if (capturedMouse) {
		Release();
		if (selType == selChar) {
			int newPos = PositionFromLocation(pt);
			newPos = MovePositionOutsideChar(newPos, currentPos - newPos);
			SetSelection(newPos);
			//dprintf("Up: %d - %d\n", anchor, currentPos);
		}
		lastClickTime = curTime;
		lastClick = pt;
		lastXChosen = pt.x;
	}
}

void Scintilla::DeleteUndoHistory() {
	doc.DeleteUndoHistory();
}

void Scintilla::Cut() {
	Copy();
	ClearSelection();
	Redraw();
}

void Scintilla::Copy() {
	//dprintf("Copy\n");
	if (currentPos != anchor) {
#ifdef GTK
		free(pasteBuffer);
		pasteBuffer = 0;
		unsigned int bytes = 0;
		int startPos = SelectionStart();
		bytes = SelectionEnd() - startPos;
		pasteBuffer = reinterpret_cast<unsigned char *>(malloc(bytes+1));
		if (pasteBuffer) {
			for (int i=0;i<bytes;i++) {
				pasteBuffer[i] = doc.CharAt(startPos + i);
			}
			pasteBuffer[bytes] = '\0';
		}
		gtk_selection_owner_set(GTK_WIDGET(draw),
                        		clipboard_atom,
                        		GDK_CURRENT_TIME);
#else
		HGLOBAL hmemSelection = GetSelText();
		OpenClipboard(hwnd);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hmemSelection);
		CloseClipboard();
#endif
	}
}

void Scintilla::Paste() {
#ifdef GTK
	//dprintf("Paste\n");
	gtk_selection_convert (GTK_WIDGET(draw),
                       	clipboard_atom,
                       	gdk_atom_intern ("STRING", FALSE),  GDK_CURRENT_TIME);
#else
	ClearSelection();
	OpenClipboard(hwnd);
	HGLOBAL hmemSelection = GetClipboardData(CF_TEXT);
	if (hmemSelection) {
		LPSTR ptr = (LPSTR)GlobalLock(hmemSelection);
		if (ptr) {
			unsigned int bytes = GlobalSize(hmemSelection);
			int len = bytes;
			for (int i=0; i<bytes;i++) {
				if ((len == bytes) && (0 == ptr[i]))
					len = i;
			}
			InsertString(currentPos, ptr, len);
			int newPos = currentPos + len;
			SetSelection(newPos,newPos);
		}
		GlobalUnlock(hmemSelection);
	}
	CloseClipboard();
	NotifyChange();
	Redraw();
#endif
}

#ifdef GTK
void Scintilla::ReceivedSelection(GtkSelectionData *selection_data, guint time) {
	if (selection_data->type == GDK_TARGET_STRING) {
		if ((selection_data->selection == clipboard_atom) &&
    			(selection_data->length > 0)) {
			ClearSelection();
			char *ptr = reinterpret_cast<char *>(selection_data->data);
			unsigned int bytes = selection_data->length;
			//dprintf("Size = %d\n", bytes);
			int len = bytes;
			for (int i=0; i<bytes;i++) {
				if ((len == bytes) && (0 == ptr[i]))
					len = i;
			}
			InsertString(currentPos, ptr, len);
			int newPos = currentPos + len;
			SetSelection(newPos,newPos);
		}
	}
	Redraw();
}

void Scintilla::GetSelection(GtkSelectionData *selection_data) {
	gtk_selection_data_set(selection_data, GDK_SELECTION_TYPE_STRING,
                       	8, pasteBuffer,
                       	strlen(reinterpret_cast<char *>(pasteBuffer)));
}
#endif

void Scintilla::Clear() {
	if (currentPos == anchor) {
		DelChar();
	} else {
		ClearSelection();
	}
	SetSelection(currentPos, currentPos);
	Redraw();
}

void Scintilla::SelectAll() {
	SetSelection(0, doc.Length());
	Redraw();
}

int Scintilla::KeyDown(int key, bool shift, bool ctrl, bool alt) {
	int modifiers = (shift ? SCI_SHIFT : 0) | (ctrl ? SCI_CTRL : 0) |
		(alt ? SCI_ALT : 0);
	for (int keyIndex = 0; keyIndex < keymapLen; keyIndex++) {
		if ((key == keymap[keyIndex].key) && (modifiers == keymap[keyIndex].modifiers)) {
			return WndProc(keymap[keyIndex].msg, 0, 0);
		}
	}
#ifdef GTK
	if (!ctrl && !alt && (key < 128)) {
		AddChar(key);
	} else {
		// Pass up to container in case it is an accelerator
		NotifyKey(key, modifiers);
	}
#endif
	return TRUE;
}

void Scintilla::GoToLine(int lineNo) {
	if (lineNo > LinesTotal())
		lineNo = LinesTotal();
	if (lineNo < 0)
		lineNo = 0;
	SetSelection(LineStart(lineNo), LineStart(lineNo));
	ShowCaretAtCurrentPosition();
	EnsureCaretVisible();
}

#ifdef GTK
void Scintilla::Resize(int width, int height) {
	//dprintf("Resize %d %d\n", width, height);
	DropGraphics();
	GtkAllocation alloc;

	alloc.x = 0;
	alloc.y = 0;
	alloc.width = width - 16;
	alloc.height = height - 16;
	gtk_widget_size_allocate(GTK_WIDGET(draw), &alloc);

	alloc.x = 0;
	alloc.y = height - 16;
	alloc.width = width - 16;
	alloc.height = 16;
	gtk_widget_size_allocate(GTK_WIDGET(scrollbarh), &alloc);

	alloc.x = width - 16;
	alloc.y = 0;
	alloc.width = 16;
	alloc.height = height - 16;
	gtk_widget_size_allocate(GTK_WIDGET(scrollbarv), &alloc);

	SetScrollBars(0,0);
}
#endif

void Scintilla::SetScrollBars(LPARAM *plParam,WPARAM wParam) {
	RefreshStyleData();
	RECT rsClient = {0,0, 0,0};

#ifndef GTK
	if (plParam) {
		rsClient.right = LOWORD(*plParam);
		rsClient.bottom = HIWORD(*plParam);
	} else
#endif
		GetClientRectangle(&rsClient);

	int nMax = LinesTotal();
	int nPage = LinesTotal() - MaxScrollPos() + 1;
#ifdef GTK
	int pageScroll = LinesToScroll();

	if (GTK_ADJUSTMENT(adjustmentv)->upper != nMax ||
    		GTK_ADJUSTMENT(adjustmentv)->page_size != nPage ||
    		GTK_ADJUSTMENT(adjustmentv)->page_increment != pageScroll) {
		GTK_ADJUSTMENT(adjustmentv)->upper = nMax;
		GTK_ADJUSTMENT(adjustmentv)->page_size = nPage;
		GTK_ADJUSTMENT(adjustmentv)->page_increment = pageScroll;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmentv));
	}
#else
	SCROLLINFO sci = {
    	sizeof(sci)
	};
	sci.fMask = SIF_PAGE|SIF_RANGE;
	sci.nMin = 0;
	sci.nMax = LinesTotal();
	sci.nPage = LinesTotal() - MaxScrollPos() + 1;
	sci.nPos = 0;
	sci.nTrackPos = 1;
	int b = SetScrollInfo(hwnd, SB_VERT, &sci,TRUE);
#endif

#ifdef GTK
	if (GTK_ADJUSTMENT(adjustmenth)->upper != 2000 ||
    		GTK_ADJUSTMENT(adjustmenth)->page_size != 20) {
		GTK_ADJUSTMENT(adjustmenth)->upper = 2000;
		GTK_ADJUSTMENT(adjustmenth)->page_size = 20;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmenth));
	}
#else
	SetScrollRange(hwnd,SB_HORZ,0,2000,TRUE);
#endif

	// TODO: ensure always showing as many lines as possible
	// May not be, if, for example, window made larger
	if (topLine > MaxScrollPos()) {
		topLine = clamp(topLine, 0, MaxScrollPos());
		SetVertScrollFromTopLine();
		Redraw();
	}
	if (!plParam)
		Redraw();
	//dprintf("end max = %d page = %d\n", nMax, nPage);
}

void Scintilla::Capture() {
	capturedMouse = true;
#ifdef GTK
	//gtk_grab_add(GTK_WIDGET(sci));
#else
	SetCapture(hwnd);
#endif
}

void Scintilla::Release() {
	capturedMouse = false;
#ifdef GTK
	//gtk_grab_remove(GTK_WIDGET(sci));
#else
	ReleaseCapture();
#endif
}

int Scintilla::LineFromPosition(int pos) {
	return doc.lc.LineFromPosition(pos);
}

POINT Scintilla::LocationFromPosition(int pos) {
	RefreshStyleData();
	POINT pt = {0,0};
	int line = LineFromPosition(pos);
	//dprintf("line=%d\n", line);
	int xpos = fixedColumnWidth;
	Surface surfaceMeasure;
#ifdef GTK
	surfaceMeasure.Init();
#else
	surfaceMeasure.InitOnWindow(hwnd);
#endif
	pt.y = (line - topLine) * lineHeight;	// + half a lineheight?
	unsigned int posLineStart = LineStart(line);
	unsigned int posLineEnd = LineStart(line+1);
	int selStart = SelectionStart();
	int selEnd = SelectionEnd();
	int retPos = posLineStart;
	int sty = 0;
	char segment[maxLineLength];
	segment[0] = '\0';
	int segPos = 0;
	char chPrev = '\0';
	bool complete = false;
	for (int i=posLineStart;i<=posLineEnd && !complete && (posLineEnd > posLineStart);i++) {
		char ch = doc.CharAt(i);
		int colour = doc.StyleAt(i);
		unsigned int width = 0;
		if (colour != sty || (ch == '\t') || (chPrev == '\t') || (i == selStart) || (i == selEnd) || (i == posLineEnd)) {
			if (segment[0] == '\t') {
				width = (((xpos + 2 - fixedColumnWidth) / tabWidth) + 1) * tabWidth + fixedColumnWidth - xpos;
				if (i >= pos) {
					xpos += width;
					complete = true;
				} 
			} else {
				int styleMain = sty & 31;
				if (i >= pos) {
					int charsToMeasure = segPos - (i - pos);
					width = surfaceMeasure.WidthText(styles[styleMain].font, segment, charsToMeasure);
					xpos += width;
					complete = true;
				} else {
					width = surfaceMeasure.WidthText(styles[styleMain].font, segment, segPos);
				}
			}
			if (!complete) {
				retPos += segPos;
				segPos = 0;
				segment[segPos] = '\0';
				xpos += width;
				sty = colour;
			}
		}
		if (ch != '\r' && ch != '\n' && ((segPos+1) < sizeof(segment)))
			segment[segPos++] = ch;
		chPrev = ch;
	}
	pt.x = xpos - xOffset;
	return pt;
}

int Scintilla::LineFromLocation(POINT pt) {
	return pt.y / lineHeight + topLine;
}

int Scintilla::Length() {
	return doc.Length();
}

char Scintilla::CharAt(int pos) {
	return doc.CharAt(pos);
}

int Scintilla::CurrentPosition() {
	return currentPos;
}

int Scintilla::PositionFromLocation(POINT pt) {
	pt.x += xOffset;
	int line = pt.y / lineHeight + topLine;
	if (line < 0)
		return 0;
	if (line >= doc.Lines())
		return Length();
	int xpos = fixedColumnWidth;
	Surface surfaceMeasure;
#ifdef GTK
	surfaceMeasure.Init();
#else
	surfaceMeasure.InitOnWindow(hwnd);
#endif
	unsigned int posLineStart = LineStart(line);
	unsigned int posLineEnd = LineStart(line+1);
	int selStart = SelectionStart();
	int selEnd = SelectionEnd();
	int retPos = posLineStart;
	int sty = 0;
	char segment[maxLineLength];
	segment[0] = '\0';
	int segPos = 0;
	char chPrev = '\0';
	bool complete = false;
	for (int i=posLineStart;i<=posLineEnd && xpos < pt.x && !complete && (posLineEnd > posLineStart);i++) {
		char ch = doc.CharAt(i);
		int colour = doc.StyleAt(i);
		unsigned int width = 0;
		if (colour != sty || (ch == '\t') || (chPrev == '\t') || (i == selStart) || (i == selEnd) || (i == posLineEnd)) {
			if (segment[0] == '\t') {
				width = (((xpos + 2 - fixedColumnWidth) / tabWidth) + 1) * tabWidth + fixedColumnWidth - xpos;
				if ((xpos + width) >= pt.x) {
					complete = true;
					if ((xpos + width / 2) <= pt.x)	// If in first half of char
						retPos = retPos + 1;
				}
			} else {
				int styleMain = sty & 31;
				width = surfaceMeasure.WidthText(styles[styleMain].font, segment, segPos);
				if ((xpos + width) >= pt.x) {
					// For non -tabs must measure positions
					int prevWidth = 0;
					for (int j=1; j<=segPos && !complete; j++) {
						width = surfaceMeasure.WidthText(styles[styleMain].font, segment, j);
						if ((xpos + width) >= pt.x) {
							complete = true;
							retPos += j - 1;
							if ((xpos + prevWidth + (width-prevWidth) / 2) < pt.x)
								retPos = retPos + 1;
						}
						prevWidth = width;
					}
				}
			}
			if (!complete) {
				retPos += segPos;
				segPos = 0;
				segment[segPos] = '\0';
				xpos += width;
				sty = colour;
			}
		}
		if (ch != '\r' && ch != '\n' && ((segPos+1) < sizeof(segment)))
			segment[segPos++] = ch;
		chPrev = ch;
	}

	if (retPos > posLineEnd)
		retPos = posLineEnd;
	if (retPos < posLineStart)
		retPos = posLineStart;
	return retPos;
}

void Scintilla::SetVertScrollFromTopLine() {
#ifdef GTK
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmentv), topLine);
#else
	SetScrollPos(hwnd,SB_VERT,topLine,TRUE);
#endif
}

#ifdef GTK
void Scintilla::Scroll(int topLineNew) {
	//dprintf("Scrolling\n");
	topLine = topLineNew;
	Redraw();
}
#else
void Scintilla::Scroll(WPARAM wParam) {
	//DWORD dwStart = timeGetTime();
	//dprintf("Scroll %x %d\n", wParam, lParam);

	SCROLLINFO sci;
	memset(&sci, 0, sizeof(sci));
	sci.cbSize = sizeof(sci);
	sci.fMask = SIF_ALL;

	BOOL b = GetScrollInfo(hwnd, SB_VERT, &sci);

	//dprintf("ScrollInfo %d mask=%x min=%d max=%d page=%d pos=%d track=%d\n", b,sci.fMask,
	//sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);

	switch (LOWORD(wParam)) {
	case SB_LINEUP:			topLine -= 1;				break;
	case SB_LINEDOWN:		topLine += 1;				break;
	case SB_PAGEUP:			topLine -= LinesToScroll();	break;
	case SB_PAGEDOWN:		topLine += LinesToScroll();	break;
	case SB_TOP:				topLine = 0;				break;
	case SB_BOTTOM:			topLine = MaxScrollPos();	break;
	case SB_THUMBPOSITION:	topLine = sci.nTrackPos;		break;
	case SB_THUMBTRACK:		topLine = sci.nTrackPos;		break;
	}
	topLine = clamp(topLine, 0, MaxScrollPos());
	SetVertScrollFromTopLine();
	ShowCaretAtCurrentPosition();
	Redraw();
	//DWORD dwEnd = timeGetTime();
	//dprintf("end scroll %d\n", dwEnd - dwStart);
}

void Scintilla::HorizontalScroll(WPARAM wParam) {
	int xPos = xOffset;
	switch (LOWORD(wParam)) {
	case SB_LINEUP:			xPos -= 20;	break;
	case SB_LINEDOWN:		xPos += 20;	break;
	case SB_PAGEUP:			xPos -= 200;	break;
	case SB_PAGEDOWN:		xPos += 200;	break;
	case SB_TOP:				xPos = 0;		break;
	case SB_BOTTOM:			xPos = 2000;	break;
	case SB_THUMBPOSITION:	xPos = HIWORD(wParam);	break;
	case SB_THUMBTRACK:		xPos = HIWORD(wParam);	break;
	}
	HorizontalScrollTo(xPos);
}
#endif

void Scintilla::AutoCompleteStart(char *list) {
	//dprintf("AutoCOmplete %s\n", list);
#ifdef GTK
	listAutoComplete = 0;
	currentAutoComplete = 0;
	GtkWidget *list_item = 0;
	GtkWidget *scrolled_window=0;
#endif
	CallTipCancel();

	RECT rcClient;
	GetClientRectangle(&rcClient);
	if (!hwndAutoComplete) {
#ifdef GTK
		hwndAutoComplete = gtk_window_new(GTK_WINDOW_POPUP);
		scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_widget_set_usize(scrolled_window, 120, 100);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(hwndAutoComplete), scrolled_window);

		listAutoComplete = gtk_clist_new(1);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), listAutoComplete);
		gtk_widget_show(scrolled_window);

		gtk_widget_show(listAutoComplete);
		gtk_signal_connect(GTK_OBJECT(listAutoComplete), "select_row",
			GTK_SIGNAL_FUNC(SelectionAC), this);
		//gtk_clist_set_shadow_type(GTK_CLIST(listAutoComplete), GTK_SHADOW_NONE);
		gtk_clist_set_sort_column(GTK_CLIST(listAutoComplete), 0);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(GTK_BIN(scrolled_window)->child), GTK_SHADOW_OUT);
#else
		hwndAutoComplete = CreateWindowEx(
                       		WS_EX_CLIENTEDGE, "listbox", "",
                       		WS_CHILD|WS_BORDER|WS_VSCROLL|LBS_SORT|LBS_NOTIFY,
                       		100,100, 150,80, hwnd, (HMENU)idAutoComplete, hInstance, 0);
#endif
	}
	inAutoCompleteMode = true;
	posStartAutoComplete = currentPos;
	//dprintf("Auto complete %x\n", hwndAutoComplete);
	POINT pt = LocationFromPosition(currentPos);
	int heightLB = 100;
	int widthLB = 100;
	int maxStrLen = 12;
	if (pt.x >= rcClient.right - widthLB) {
		HorizontalScrollTo(xOffset + pt.x - rcClient.right + widthLB);
		Redraw();
		pt = LocationFromPosition(currentPos);
	}
#ifdef GTK
	int ox = 0;
	int oy = 0;
	gdk_window_get_origin(draw->window, &ox, &oy);
	GtkAllocation alloc;
	if (pt.y >= rcClient.bottom - heightLB) {
		alloc.x = ox + pt.x-3;
		alloc.y = oy + pt.y - heightLB;
	} else {
		alloc.x = ox + pt.x-3;
		alloc.y = oy + pt.y + lineHeight;
	}
	alloc.width = 100;
	alloc.height = 100;
	gtk_widget_set_uposition(hwndAutoComplete, alloc.x, alloc.y);
	gtk_widget_size_allocate(GTK_WIDGET(listAutoComplete), &alloc);
#else
	if (pt.y >= rcClient.bottom - heightLB) {
		SetWindowPos(hwndAutoComplete, 0, pt.x-3, pt.y - heightLB, widthLB, heightLB, 0);
	} else {
		SetWindowPos(hwndAutoComplete, 0, pt.x-3, pt.y + lineHeight, widthLB, heightLB, 0);
	}
	SendMessage(hwndAutoComplete, WM_SETFONT, (WPARAM)styles[0].font, 0);
	SendMessage(hwndAutoComplete, LB_RESETCONTENT, 0, 0);
#endif

	char *words = strdup(list);
	char *startword = words;
	int i = 0;
	for (; words && words[i]; i++) {
		if (words[i] == ' ') {
			words[i] = '\0';
#ifdef GTK
			char *szs[] = { startword, 0};
			gtk_clist_append(GTK_CLIST(listAutoComplete), szs);
#else
			SendMessage(hwndAutoComplete, LB_ADDSTRING, 0, (LPARAM)startword);
#endif
			maxStrLen = max(maxStrLen, strlen(startword));
			startword = words + i + 1;
		}
	}
	if (startword) {
#ifdef GTK
		char *szs[] = { startword, 0};
		gtk_clist_append(GTK_CLIST(listAutoComplete), szs);
#else
		SendMessage(hwndAutoComplete, LB_ADDSTRING, 0, (LPARAM)startword);
#endif
		maxStrLen = max(maxStrLen, strlen(startword));
	}
	free(words);
#ifdef GTK
	//gtk_fixed_put(GTK_FIXED(sci), hwndAutoComplete, alloc.x, alloc.y);
	gtk_clist_sort(GTK_CLIST(listAutoComplete));
	gtk_widget_show(listAutoComplete);
	gtk_widget_realize(hwndAutoComplete);
	gtk_widget_show(hwndAutoComplete);
#else
	// Fiddle the position of the list so it is right next to the target and wide enough for all its strings
	RECT rcList;
	GetWindowRect(hwndAutoComplete, &rcList);
	int heightAlloced = rcList.bottom - rcList.top;
	// Make an allowance for large strings in list
	widthLB = max(widthLB, maxStrLen * 8 + 16);
	if (pt.y >= rcClient.bottom - heightLB) {
		SetWindowPos(hwndAutoComplete, 0, pt.x-3, pt.y - heightAlloced, widthLB, heightAlloced, 0);
	} else {
		SetWindowPos(hwndAutoComplete, 0, pt.x-3, pt.y + lineHeight, widthLB, heightAlloced, 0);
	}
	ShowWindow(hwndAutoComplete, SW_SHOWNORMAL);
	SendMessage(hwndAutoComplete, LB_SETCURSEL, 0, 0);
#endif
}

void Scintilla::AutoCompleteCancel() {
	if (hwndAutoComplete) {
#ifdef GTK
		gtk_widget_destroy(GTK_WIDGET(hwndAutoComplete));
#else
		ShowWindow(hwndAutoComplete, SW_HIDE);
		DestroyWindow(hwndAutoComplete);
#endif
		inAutoCompleteMode = false;
		hwndAutoComplete = 0;
	}
}

#ifdef GTK
void Scintilla::SelectionAC(GtkWidget      *clist,
                     gint            row,
                     gint            column,
		     GdkEventButton *event,
                     gpointer        p) {
	Scintilla *psci = reinterpret_cast<Scintilla *>(p);
	psci->currentAutoComplete = row;
}
#endif

void Scintilla::AutoCompleteMove(int delta) {
#ifdef GTK
	int count = GTK_CLIST(listAutoComplete)->rows;
	int current = currentAutoComplete;
#else
	int count = SendMessage(hwndAutoComplete, LB_GETCOUNT, 0, 0);
	int current = SendMessage(hwndAutoComplete, LB_GETCURSEL, 0, 0);
#endif
	current += delta;
	if (current >= count)
		current = count - 1;
	if (current < 0)
		current = 0;
#ifdef GTK
	gtk_clist_select_row(GTK_CLIST(listAutoComplete),
			   current, 0);
#else
	SendMessage(hwndAutoComplete, LB_SETCURSEL, current, 0);
#endif
}

void Scintilla::AutoCompleteChanged(char ch) {
	if (posStartAutoComplete > currentPos) {
		AutoCompleteCancel();
	} else if (ch && strchr(autoCompleteStops, ch)) {
		AutoCompleteCancel();
	} else {
		char wordCurrent[1000];
		int i;
		for (i=posStartAutoComplete;i<currentPos;i++)
			wordCurrent[i - posStartAutoComplete] = doc.CharAt(i);
		wordCurrent[i - posStartAutoComplete] = '\0';
#ifdef GTK
		int count = GTK_CLIST(listAutoComplete)->rows;
		for (int j=0;j<count;j++) {
			char *s=0;
			gtk_clist_get_text(GTK_CLIST(listAutoComplete),
					 j, 0, &s);
			if (s && (0 == strncmp(wordCurrent, s, strlen(wordCurrent)))) {
				gtk_clist_select_row(GTK_CLIST(listAutoComplete), j, 0);
				count = 0;
			}
		}
#else
		int pos = SendMessage(hwndAutoComplete, LB_FINDSTRING, 0, (LPARAM) wordCurrent);
		//dprintf("Autocompleting at <%s> %d\n", wordCurrent, pos);
		if (pos != -1)
			SendMessage(hwndAutoComplete, LB_SETCURSEL, pos, 0);
#endif
	}
}

void Scintilla::AutoCompleteCompleted() {
	inAutoCompleteMode = false;
	if (currentPos != posStartAutoComplete) {
		DeleteChars(posStartAutoComplete, currentPos - posStartAutoComplete);
	}
	SetSelection(posStartAutoComplete, posStartAutoComplete);
#ifdef GTK
	int item = currentAutoComplete;
	if (item != -1) {
		char *selected=0;
		gtk_clist_get_text(GTK_CLIST(listAutoComplete), item, 0, &selected);
		if (selected) {
			InsertString(currentPos, selected);
			SetSelection(currentPos+strlen(selected), currentPos+strlen(selected));
		}
	}
	gtk_widget_destroy(GTK_WIDGET(hwndAutoComplete));
#else
	ShowWindow(hwndAutoComplete, SW_HIDE);
	int item = SendMessage(hwndAutoComplete, LB_GETCURSEL, 0, 0);
	if (item != -1) {
		char selected[200];
		SendMessage(hwndAutoComplete, LB_GETTEXT, item, (LPARAM) selected);
		//dprintf("Selecting %d <%s>\n", item, selected);
		//dprintf("Selecting <%s>\n", selected);
		InsertString(currentPos, selected);
		SetSelection(currentPos+strlen(selected), currentPos+strlen(selected));
	}
	DestroyWindow(hwndAutoComplete);
#endif
	hwndAutoComplete = 0;
	NotifyChange();
}

#ifdef GTK
gint Scintilla::PaintCT(GtkWidget *widget_, GdkEventExpose ose) {
	GdkGC *gc = gdk_gc_new(widget_->window);
	Surface surfaceCT;
	surfaceCT.Init(widget_, widget_->window, gc);

	RECT rcClient;
	rcClient.left = 1;
	rcClient.right = widget_->allocation.width-2;
	rcClient.top = 1;
	rcClient.bottom = widget_->allocation.height-2;

	COLORREF bg = RGB(0xff, 0xff, 0xff);

	surfaceCT.FillRectangle(rcClient, bg);

	int x = 5;
	int xEnd = x + surfaceCT.WidthText(styles[0].font, valCT, startHighlightCT);
	rcClient.left = x;
	rcClient.right = xEnd;
	surfaceCT.DrawText(rcClient, styles[0].font, lineHeight - styles[0].descent, 
		valCT, startHighlightCT, 
		RGB(0x80,0x80,0x80), bg);
	x = xEnd;

	xEnd = x + surfaceCT.WidthText(styles[0].font, valCT + startHighlightCT, endHighlightCT - startHighlightCT);
	rcClient.left = x;
	rcClient.right = xEnd;
	surfaceCT.DrawText(rcClient, styles[0].font, lineHeight - styles[0].descent, 
		valCT + startHighlightCT, endHighlightCT - startHighlightCT, 
		RGB(0,0,0x80), bg);
	x = xEnd;

	xEnd = x + surfaceCT.WidthText(styles[0].font, valCT + endHighlightCT, strlen(valCT) - endHighlightCT);
	rcClient.left = x;
	rcClient.right = xEnd;
	surfaceCT.DrawText(rcClient, styles[0].font, lineHeight - styles[0].descent, 
		valCT + endHighlightCT, strlen(valCT) - endHighlightCT, 
		RGB(0x80,0x80,0x80), bg);
	x = xEnd;

	surfaceCT.MoveTo(0, widget_->allocation.height-1);
	surfaceCT.PenColor(RGB(0,0,0));
	surfaceCT.LineTo(widget_->allocation.width-1, widget_->allocation.height-1);
	surfaceCT.LineTo(widget_->allocation.width-1, 0);
	surfaceCT.PenColor(RGB(0xc0,0xc0,0xc0));
	surfaceCT.LineTo(0, 0);
	surfaceCT.LineTo(0, widget_->allocation.height-1);

	gdk_gc_unref(gc);
}

gint Scintilla::ExposeCT(GtkWidget *widget, GdkEventExpose *ose, gpointer p) {
	Scintilla *psci = reinterpret_cast<Scintilla *>(p);
	return psci->PaintCT(widget, *ose);
}
#endif

void Scintilla::CallTipStart(int pos, char *defn) {
	AutoCompleteCancel();
#ifdef GTK
	if (valCT)
		free(valCT);
	valCT = strdup(defn);
	startHighlightCT = 0;
	endHighlightCT = 0;
	if (!hwndCallTip) {
		Surface surfaceCT;
		surfaceCT.Init();
		int width = surfaceCT.WidthText(styles[0].font, valCT, strlen(valCT)) + 10;
		int height = lineHeight + 2;
		hwndCallTip = gtk_window_new(GTK_WINDOW_POPUP);
		GtkWidget *drawCT = gtk_drawing_area_new();
		gtk_container_add(GTK_CONTAINER(hwndCallTip), drawCT);
		gtk_signal_connect(GTK_OBJECT(drawCT), "expose_event",
                   	GtkSignalFunc(ExposeCT), this);
		gtk_widget_set_events(drawCT, GDK_EXPOSURE_MASK);
		gtk_drawing_area_size(GTK_DRAWING_AREA(drawCT), width, height);

		POINT pt = LocationFromPosition(pos);
		int ox = 0;
		int oy = 0;
		gdk_window_get_origin(draw->window, &ox, &oy);
		GtkAllocation alloc;
		alloc.x = ox + pt.x-5;
		alloc.y = oy + pt.y + lineHeight;
		alloc.width = width;
		alloc.height = height;
		gtk_widget_set_uposition(hwndCallTip, alloc.x, alloc.y);
		gtk_widget_size_allocate(GTK_WIDGET(drawCT), &alloc);

		gtk_widget_show(drawCT);
		gtk_widget_realize(hwndCallTip);
		gtk_widget_show(hwndCallTip);
	}
#else
	if (!hwndCallTip) {
		hwndCallTip = CreateWindow(callClassName, "ACallTip", WS_VISIBLE|WS_CHILD, 100,100, 150,20, hwnd, (HMENU)idAutoComplete, hInstance, 0);
	}
	//dprintf("Made tool tip %x\n", hwndCallTip);
	POINT pt = LocationFromPosition(pos);
	SetWindowPos(hwndCallTip, 0, pt.x-5, pt.y + lineHeight, 10, lineHeight + 2, 0);
	SendMessage(hwndCallTip, WM_SETFONT, (WPARAM)styles[0].font, 0);
	SetWindowText(hwndCallTip,defn);
	ShowWindow(hwndCallTip, SW_SHOWNORMAL);
#endif
	inCallTipMode = true;
	posStartCallTip = currentPos;
}

void Scintilla::CallTipCancel() {
	inCallTipMode = false;
#ifdef GTK
	if (hwndCallTip) {
		gtk_widget_destroy(GTK_WIDGET(hwndCallTip));
		hwndCallTip = 0;
	}
#else
	ShowWindow(hwndCallTip, SW_HIDE);
#endif
}

#ifdef GTK
void Scintilla::PopUpCB(GtkWidget *widget, gpointer cbdata) {
	Scintilla *sci = reinterpret_cast<Scintilla *>(cbdata);
	GList *children = gtk_container_children(GTK_CONTAINER(widget));
	GtkWidget *child = reinterpret_cast<GtkWidget *>(
                       	g_list_nth(children, 0)->data);
	GtkLabel *label = GTK_LABEL(child);
	//dprintf("Pop up %x %x %s\n", children, child, label->label);
	int cmd = 0;
	if (0 == strcmp(label->label, "Undo")) {
		cmd = idcmdUndo;
	} else if (0 == strcmp(label->label, "Redo")) {
		cmd = idcmdRedo;
	} else if (0 == strcmp(label->label, "Cut")) {
		cmd = idcmdCut;
	} else if (0 == strcmp(label->label, "Copy")) {
		cmd = idcmdCopy;
	} else if (0 == strcmp(label->label, "Paste")) {
		cmd = idcmdPaste;
	} else if (0 == strcmp(label->label, "Delete")) {
		cmd = idcmdDelete;
	} else if (0 == strcmp(label->label, "Select All")) {
		cmd = idcmdSelectAll;
	}
	if (cmd) {
		sci->Command(cmd);
	}
}

void Scintilla::AddToPopUp(const char *label, bool enabled) {
	GtkWidget *it = 0;
	if (0 == strcmp(label, "<separator>"))
		it = gtk_menu_item_new();
	else
		it = gtk_menu_item_new_with_label(label);
	gtk_widget_set_sensitive(it, enabled);
	gtk_menu_append(GTK_MENU(popup), it);
	gtk_widget_show(it);
	gtk_signal_connect(GTK_OBJECT(it), "activate",
                   	GTK_SIGNAL_FUNC(Scintilla::PopUpCB), this);
}
#endif

void Scintilla::ContextMenu(POINT pt) {
#ifndef GTK
	HMENU popMenu = CreatePopupMenu();
	if (popMenu == NULL)
		return;

	if (doc.CanUndo())
		AppendMenu(popMenu, MF_STRING, idcmdUndo, "Undo" );
	else
		AppendMenu(popMenu, MF_STRING | MF_DISABLED | MF_GRAYED, idcmdUndo, "Undo" );
	if (doc.CanRedo())
		AppendMenu(popMenu, MF_STRING, idcmdRedo,   "Redo" );
	else
		AppendMenu(popMenu, MF_STRING | MF_DISABLED | MF_GRAYED, idcmdRedo, "Redo" );
	AppendMenu(popMenu, MF_SEPARATOR, 0, "");
	if (currentPos != anchor) {
		AppendMenu(popMenu, MF_STRING, idcmdCut, "Cut" );
		AppendMenu(popMenu, MF_STRING, idcmdCopy, "Copy");
	} else {	// Empty selection
		AppendMenu(popMenu, MF_STRING | MF_DISABLED | MF_GRAYED, idcmdCut, "Cut" );
		AppendMenu(popMenu, MF_STRING | MF_DISABLED | MF_GRAYED, idcmdCopy, "Copy");
	}
	if (SendMessage(hwnd, EM_CANPASTE, 0, 0))
		AppendMenu(popMenu, MF_STRING, idcmdPaste, "Paste");
	else
		AppendMenu(popMenu, MF_STRING | MF_DISABLED | MF_GRAYED, idcmdPaste, "Paste");
	AppendMenu(popMenu, MF_STRING, idcmdDelete, "Delete");
	AppendMenu(popMenu, MF_SEPARATOR, 0, "");
	AppendMenu(popMenu, MF_STRING, idcmdSelectAll, "Select All");

	if (!TrackPopupMenu(popMenu, 0, pt.x - 4, pt.y, 0, hwnd, NULL))
		return;

	DestroyMenu(popMenu);
#else
	if (popup != 0) {
		gtk_widget_unref(popup);
		popup = 0;
	}
	popup = gtk_menu_new();
	AddToPopUp("Undo", doc.CanUndo());
	AddToPopUp("Redo", doc.CanRedo());
	AddToPopUp("<separator>");
	AddToPopUp("Cut", currentPos != anchor);
	AddToPopUp("Copy", currentPos != anchor);
	AddToPopUp("Paste");
	AddToPopUp("Delete");
	AddToPopUp("<separator>");
	AddToPopUp("Select All");
	gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
               	3, 0);
	//gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
	//	3, event->time);
#endif
}

void Scintilla::Command(WPARAM wParam) {
	int cmd = HIWORD(wParam);
	switch (LOWORD(wParam)) {
	case idAutoComplete:
		//dprintf("S command %x %x\n", wParam);
#ifndef GTK
		if (cmd == LBN_DBLCLK) {
			AutoCompleteCompleted();
		} else {
			if (cmd != LBN_SETFOCUS)
				SetFocus(hwnd);
		}
#endif
		break;

	case idCallTip:	// Nothing to do
		break;

	case idcmdUndo:
		Undo();
		break;

	case idcmdRedo:
		Redo();
		break;

	case idcmdCut:
		Cut();
		break;

	case idcmdCopy:
		Copy();
		break;

	case idcmdPaste:
		Paste();
		break;

	case idcmdDelete:
		Clear();
		break;

	case idcmdSelectAll:
		SelectAll();
		break;
	}
}

bool Scintilla::IsWordAt(int start, int end) {
	int lengthDoc = Length();
	if (start > 0) {
		char ch = CharAt(start-1);
		//dprintf("start = %c\n", ch);
		if (isalnum(ch))
			return false;
	}
	if (end < lengthDoc - 1) {
		char ch = CharAt(end + 1);
		//dprintf("end = %c\n", ch);
		if (isalnum(ch))
			return false;
	}
	return true;
}

long Scintilla::FindText(WORD iMessage,WPARAM wParam,LPARAM lParam) {
	FINDTEXTEX *ft = reinterpret_cast<FINDTEXTEX *>(lParam);
	int startPos = ClampPositionIntoDocument(ft->chrg.cpMin);
	int endPos = ClampPositionIntoDocument(ft->chrg.cpMax);
	int lengthFind = strlen(ft->lpstrText);
	//dprintf("Find %d %d %s %d\n", startPos, endPos, ft->lpstrText, lengthFind);
	for (int pos=startPos;pos<endPos-lengthFind+1;pos++) {
		char ch = CharAt(pos);
		if (wParam & FR_MATCHCASE) {
			if (ch == ft->lpstrText[0]) {
				bool found = true;
				for (int posMatch = 0; posMatch < lengthFind && found; posMatch++) {
					ch = CharAt(pos + posMatch);
					if (ch != ft->lpstrText[posMatch])
						found = false;
				}
				if (found) {
					if (wParam & FR_WHOLEWORD)
						found = IsWordAt(pos,pos + lengthFind);
					if (found) {
						if (iMessage == EM_FINDTEXTEX) {
							ft->chrgText.cpMin = pos;
							ft->chrgText.cpMax = pos + lengthFind;
						}
						return pos;
					}
				}
			}
		} else {
			if (toupper(ch) == toupper(ft->lpstrText[0])) {
				bool found = true;
				for (int posMatch = 0; posMatch < lengthFind && found; posMatch++) {
					ch = CharAt(pos + posMatch);
					if (toupper(ch) != toupper(ft->lpstrText[posMatch]))
						found = false;
				}
				if (found) {
					if (wParam & FR_WHOLEWORD)
						found = IsWordAt(pos,pos + lengthFind);
					if (found) {
						if (iMessage == EM_FINDTEXTEX) {
							ft->chrgText.cpMin = pos;
							ft->chrgText.cpMax = pos + lengthFind;
						}
						return pos;
					}
				}
			}
		}
	}
	//dprintf("Not found\n");
	return -1;
}

long Scintilla::WndProc(WORD iMessage,WPARAM wParam,LPARAM lParam) {
	//dprintf("S start wnd proc %d %d %d\n",iMessage, wParam, lParam);
	switch (iMessage) {

#ifndef GTK
	case WM_CREATE:
		ctrlID = GetDlgCtrlID(hwnd);
		break;

	case WM_PAINT: {
			RECT rcPaint = {0,0,0,0};
			Paint(rcPaint);
		}
		break;

	case WM_VSCROLL:
		Scroll(wParam);
		break;

	case WM_HSCROLL:
		HorizontalScroll(wParam);
		break;

	case WM_SIZE:
		//dprintf("S start wnd proc %d %d %d\n",iMessage, wParam, lParam);
		SetScrollBars(&lParam,wParam);
		DropGraphics();
		break;
#endif

	case WM_GETTEXT: {
			char *ptr = reinterpret_cast<char *>(lParam);
			for (int iChar=0;iChar<wParam;iChar++)
				ptr[iChar] = doc.CharAt(iChar);
		}
		break;

	case WM_SETTEXT: {
			DeleteChars(0, Length());
			SetSelection(0,0);
			InsertString(0, reinterpret_cast<char *>(lParam));
			NotifyChange();
			Redraw();
		}
		break;

	case WM_GETTEXTLENGTH:
		return Length();

#ifndef GTK
	case WM_GETMINMAXINFO:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
		break;

	case WM_LBUTTONDOWN:
		ButtonDown(PointFromLparam(lParam), GetTickCount(), wParam & MK_SHIFT);
		break;

	case WM_MOUSEMOVE:
		ButtonMove(PointFromLparam(lParam));
		break;

	case WM_LBUTTONUP:
		ButtonUp(PointFromLparam(lParam), GetTickCount());
		break;

	case WM_CHAR:
		//dprintf("S char proc %d %x %x\n",iMessage, wParam, lParam);
		if (!iscntrl(wParam&0xff))
			AddChar(wParam&0xff);
		return 1;

	case WM_KEYDOWN:
		//dprintf("S keydown %d %x %x %x %x\n",iMessage, wParam, lParam, ::IsKeyDown(VK_SHIFT), ::IsKeyDown(VK_CONTROL));
		return KeyDown(wParam, ::IsKeyDown(VK_SHIFT), ::IsKeyDown(VK_CONTROL), false);
		break;

	case WM_KEYUP:
		//dprintf("S keyup %d %x %x\n",iMessage, wParam, lParam);
		break;

	case WM_SETTINGCHANGE:
		//dprintf("Setting Changed\n");
		DropGraphics();
		break;

	case WM_GETDLGCODE:
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS;

	case WM_KILLFOCUS:
		DropCaret();
		//RealizeWindowPalette(true);
		break;

	case WM_SETFOCUS:
		ShowCaretAtCurrentPosition();
		RealizeWindowPalette(false);
		break;

	case WM_SYSCOLORCHANGE:
		//dprintf("Setting Changed\n");
		DropGraphics();
		break;

	case WM_PALETTECHANGED:
		if ((int)wParam != (int)hwnd) {
			dprintf("** Palette Changed\n");
			RealizeWindowPalette(true);
		}
		break;

	case WM_QUERYNEWPALETTE:
		dprintf("** Query palette\n");
		RealizeWindowPalette(false);
		break;
#endif

	case WM_COMMAND:
		Command(wParam);
		break;

	case WM_NOTIFY:
		//dprintf("S notify %d %d\n", wParam, lParam);
		break;

	case WM_CUT:
		Cut();
		break;

	case WM_COPY:
		Copy();
		break;

	case WM_PASTE:
		Paste();
		SetScrollBars();
		break;

	case WM_CLEAR:
		//dprintf("S Clear %d %x %x\n",iMessage, wParam, lParam);
		Clear();
		SetScrollBars();
		break;

	case WM_UNDO:
		Undo();
		break;

#ifndef GTK
	case WM_CONTEXTMENU:
		ContextMenu(PointFromLparam(lParam));
		break;
#endif

		// Edit control mesages

		// Not supported (no-ops):
		//		EM_GETWORDBREAKPROC
		//		EM_GETWORDBREAKPROCEX
		//		EM_SETWORDBREAKPROC
		//		EM_SETWORDBREAKPROCEX
		//		EM_GETWORDWRAPMODE
		//		EM_SETWORDWRAPMODE
		//		EM_LIMITTEXT
		//		EM_EXLIMITTEXT
		//		EM_SETRECT
		//		EM_SETRECTNP
		//		EM_FMTLINES
		//		EM_GETHANDLE
		//		EM_SETHANDLE
		//		EM_GETPASSWORDCHAR
		//		EM_SETPASSWORDCHAR
		//		EM_SETTABSTOPS
		//		EM_FINDWORDBREAK
		//		EM_GETCHARFORMAT
		//		EM_SETCHARFORMAT
		//		EM_GETOLEINTERFACE
		//		EM_SETOLEINTERFACE
		//		EM_SETOLECALLBACK
		//		EM_GETPARAFORMAT
		//		EM_SETPARAFORMAT
		//		EM_PASTESPECIAL
		//		EM_REQUESTRESIZE
		//		EM_GETBKGNDCOLOR
		//		EM_SETBKGNDCOLOR
		//		EM_STREAMIN
		//		EM_STREAMOUT
		//		EM_GETIMECOLOR
		//		EM_SETIMECOLOR
		//		EM_GETIMEOPTIONS
		//		EM_SETIMEOPTIONS
		//		EM_GETMARGINS
		//		EM_SETMARGINS
		//		EM_GETOPTIONS
		//		EM_SETOPTIONS
		//		EM_GETPUNCTUATION
		//		EM_SETPUNCTUATION
		//		EM_GETTHUMB

		// Not supported but should be:
		//		EM_GETEVENTMASK
		//		EM_SETEVENTMASK
		//		For printing:
		//			EM_DISPLAYBAND
		//			EM_FORMATRANGE
		//			EM_SETTARGETDEVICE

	case EM_CANUNDO:
		return doc.CanUndo() ? TRUE : FALSE;

	case EM_UNDO:
		Undo();
		SetScrollBars();
		break;

	case EM_EMPTYUNDOBUFFER:
		DeleteUndoHistory();
		return 0;

	case EM_GETFIRSTVISIBLELINE:
		return topLine;

	case EM_GETLINE: {
			int lineStart = LineStart(wParam);
			int lineEnd = LineStart(wParam+1);
			char *ptr = reinterpret_cast<char *>(lParam);
			int iPlace = 0;
			for (int iChar=lineStart;iChar < lineEnd;iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			ptr[iPlace] = '\0';
			return iPlace;
		}
		break;

	case EM_GETLINECOUNT:
		return LinesTotal();

	case EM_GETMODIFY:
		return isModified;

	case EM_SETMODIFY:
		isModified = wParam;
		return isModified;

	case EM_GETRECT:
		GetClientRectangle(reinterpret_cast<RECT *>(lParam));
		break;

	case EM_GETSEL:
	case EM_EXGETSEL:
		if (wParam)
			*reinterpret_cast<int *>(wParam) = SelectionStart();
		if (lParam)
			*reinterpret_cast<int *>(lParam) = SelectionEnd();
		return MAKELONG(SelectionStart(), SelectionEnd());
		break;

	case EM_SETSEL:
	case EM_EXSETSEL: {
			int nStart = static_cast<int>(wParam);
			int nEnd = static_cast<int>(lParam);
			if (nEnd < 0) 
				nEnd = Length();
			if (nStart < 0)
				nStart = nEnd;
			SetSelection(nEnd, nStart);
			EnsureCaretVisible();
			Redraw();
		}
		break;

	case EM_GETSELTEXT: {
			char *ptr = reinterpret_cast<char *>(lParam);
			int iPlace = 0;
			for (int iChar=SelectionStart();iChar < SelectionEnd();iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			ptr[iPlace] = '\0';
			return iPlace;
		}
		break;

	case EM_GETWORDBREAKPROC:
		return 0;

	case EM_SETWORDBREAKPROC:
		break;

	case EM_LIMITTEXT:
		// wParam holds the number of characters control should be limited to
		break;

	case EM_GETLIMITTEXT:
		return 0xffffffff;

	case EM_GETOLEINTERFACE:
		return 0;

	case EM_LINEFROMCHAR:
	case EM_EXLINEFROMCHAR:
		return LineFromPosition(wParam);

	case EM_LINEINDEX:
		return LineStart(wParam);

	case EM_LINELENGTH:
		return LineStart(wParam+1) - LineStart(wParam);

	case EM_REPLACESEL: {
			ClearSelection();
			char *replacement = reinterpret_cast<char *>(lParam);
			InsertString(currentPos, replacement);
			SetSelection(currentPos + strlen(replacement), currentPos + strlen(replacement));
			NotifyChange();
			SetScrollBars();
			EnsureCaretVisible();
			Redraw();
		}
		break;

	case EM_SCROLL: {
			int topStart = topLine;
			Scroll(wParam);
			return MAKELONG(topLine - topStart, TRUE);
		}
		break;

	case EM_LINESCROLL:
		ScrollTo(topLine + lParam);
		HorizontalScrollTo(xOffset + wParam * spaceWidth);
		return TRUE;

	case EM_SCROLLCARET:
		EnsureCaretVisible();
		break;

	case EM_SETREADONLY:
		doc.SetReadOnly(wParam);
		return TRUE;

	case EM_SETRECT:
		break;

	case EM_CANPASTE: {
#ifdef GTK
			return 1;
#else
			OpenClipboard(hwnd);
			HGLOBAL hmemSelection = GetClipboardData(CF_TEXT);
			if (hmemSelection)
				GlobalUnlock(hmemSelection);
			CloseClipboard();
			return hmemSelection != 0;
#endif
		}
		break;

	case EM_CHARFROMPOS: {
			POINT *ppt=reinterpret_cast<POINT *>(lParam);
			return PositionFromLocation(*ppt);
		}
		break;

	case EM_POSFROMCHAR: {
			POINT *ppt=reinterpret_cast<POINT *>(lParam);
			*ppt = LocationFromPosition(wParam);
			return 0;
		}
		break;

	case EM_FINDTEXT:
		return FindText(iMessage, wParam, lParam);
		break;

	case EM_FINDTEXTEX:
		return FindText(iMessage, wParam, lParam);
		break;

	case EM_GETTEXTRANGE: {
			TEXTRANGE *tr = reinterpret_cast<TEXTRANGE *>(lParam);
			int iPlace = 0;
			for (int iChar=tr->chrg.cpMin;iChar < tr->chrg.cpMax;iChar++)
				tr->lpstrText[iPlace++] = doc.CharAt(iChar);
			tr->lpstrText[iPlace] = '\0';
			return iPlace;
		}
		break;

	case EM_SELECTIONTYPE:
		if (currentPos == anchor)
			return SEL_EMPTY;
		else
			return SEL_TEXT;

	case EM_HIDESELECTION:
		hideSelection = wParam;
		Redraw();
		break;

		// Control specific mesages

	case SCI_ADDTEXT: {
			InsertString(CurrentPosition(), reinterpret_cast<char *>(lParam), wParam);
			SetSelection(currentPos + wParam, currentPos + wParam);
			SetScrollBars();
			NotifyChange();
			Redraw();
			return 0;
		}
		break;

	case SCI_ADDSTYLEDTEXT: {
			InsertStyledString(CurrentPosition() * 2, reinterpret_cast<char *>(lParam), wParam);
			SetSelection(currentPos + wParam/2, currentPos + wParam/2);
			SetScrollBars();
			Redraw();
			return 0;
		}
		break;

	case SCI_INSERTTEXT:{
			int insertPos = wParam;
			if (wParam == -1)
				insertPos = CurrentPosition();
			int newCurrent = CurrentPosition();
			int newAnchor = anchor;
			char *sz = reinterpret_cast<char *>(lParam);
			InsertString(insertPos, sz);
			if (newCurrent > insertPos)
				newCurrent += strlen(sz);
			if (newAnchor > insertPos)
				newAnchor += strlen(sz);
			SetSelection(newCurrent, newAnchor);
			SetScrollBars();
			NotifyChange();
			Redraw();
			return 0;
		}
		break;

	case SCI_CLEARALL:
		ClearAll();
		return 0;

	case SCI_SETUNDOCOLLECTION:
		doc.SetUndoCollection(static_cast<enum undoCollectionType>(wParam));
		return 0;

	case SCI_APPENDUNDOSTARTACTION:
		doc.AppendUndoStartAction();
		return 0;

	case SCI_GETLENGTH:
		return Length();

	case SCI_GETCHARAT:
		return CharAt(wParam);

	case SCI_GETCURRENTPOS:
		return currentPos;

	case SCI_GETANCHOR:
		return anchor;

	case SCI_GETSTYLEAT:
		if (wParam >= Length())
			return 0;
		else
			return doc.StyleAt(wParam);

	case SCI_REDO:
		Redo();
		break;

	case SCI_SELECTALL:
		SelectAll();
		break;

	case SCI_SETSAVEPOINT:
		doc.SetSavePoint();
		NotifySavePoint(true);
		break;

	case SCI_GETSTYLEDTEXT: {
			TEXTRANGE *tr = reinterpret_cast<TEXTRANGE *>(lParam);
			int iPlace = 0;
			for (int iChar=tr->chrg.cpMin;iChar < tr->chrg.cpMax;iChar++) {
				tr->lpstrText[iPlace++] = doc.CharAt(iChar);
				tr->lpstrText[iPlace++] = doc.StyleAt(iChar);
			}
			tr->lpstrText[iPlace] = '\0';
			tr->lpstrText[iPlace+1] = '\0';
			return iPlace;
		}
		break;

	case SCI_GETVIEWWS:
		return viewWhitespace;

	case SCI_SETVIEWWS:
		viewWhitespace = wParam;
		Redraw();
		break;

	case SCI_GOTOLINE:
		GoToLine(wParam);
		break;

	case SCI_GOTOPOS:
		SetPosition(wParam);
		Redraw();
		break;

	case SCI_SETANCHOR:
		SetSelection(currentPos, wParam);
		//dprintf("SetAnchor %d %d\n", currentPos, anchor);
		break;

	case SCI_GETCURLINE: {
			int lineCurrentPos = LineFromPosition(currentPos);
			int lineStart = LineStart(lineCurrentPos);
			int lineEnd = LineStart(lineCurrentPos+1);
			char *ptr = reinterpret_cast<char *>(lParam);
			int iPlace = 0;
			for (int iChar=lineStart;iChar < lineEnd && iPlace < wParam;iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			ptr[iPlace++] = '\0';
			return currentPos - lineStart;
		}
		break;

	case SCI_GETENDSTYLED:
		return endStyled;

	case SCI_GETEOLMODE:
		return eolMode;

	case SCI_SETEOLMODE:
		eolMode = wParam;
		break;

	case SCI_STARTSTYLING:
		stylingPos = wParam;
		stylingMask = lParam;
		break;

	case SCI_SETSTYLING: {
			for (int iPos=0; iPos<wParam; iPos++, stylingPos++) {
				doc.SetStyleAt(stylingPos, lParam, stylingMask);
			}
			endStyled = stylingPos;
		}
		break;

	case SCI_SETSTYLINGEX: { // Specify a complete styling buffer
			for (int iPos=0; iPos<wParam; iPos++, stylingPos++) {
				doc.SetStyleAt(stylingPos, reinterpret_cast<char *>(lParam)[iPos], stylingMask);
			}
			endStyled = stylingPos;
		}
		break;

	case SCI_SETMARGINWIDTH:
		if (wParam < 100) {
			selMarginWidth = wParam;
			fixedColumnWidth = selMarginWidth + lineNumberWidth;
		}
		Redraw();
		break;

	case SCI_SETBUFFEREDDRAW:
		bufferedDraw = wParam;
		break;

	case SCI_SETTABWIDTH:
		if (wParam > 0)
			tabInChars = wParam;
		InvalidateStyleData();
		break;

	case SCI_SETCODEPAGE:
		dbcsCodePage = wParam;
		break;

	case SCI_SETLINENUMBERWIDTH:
		if (wParam < 200) {
			lineNumberWidth = wParam;
			fixedColumnWidth = selMarginWidth + lineNumberWidth;
			InvalidateStyleData();
		}
		Redraw();
		break;

	case SCI_SETUSEPALETTE:
		palette.allowRealization = wParam;
		InvalidateStyleData();
		Redraw();
		break;

		// Marker definition and setting
	case SCI_MARKERDEFINE:
		if (wParam <= MARKER_MAX)
			markers[wParam].markType = lParam;
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERSETFORE:
		if (wParam <= MARKER_MAX)
			markers[wParam].fore.desired = ColourFromLparam(lParam);
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERSETBACK:
		if (wParam <= MARKER_MAX)
			markers[wParam].back.desired = ColourFromLparam(lParam);
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERADD:
		doc.SetMark(wParam, doc.GetMark(wParam) | (1 << lParam));
		RedrawSelMargin();
		break;

	case SCI_MARKERDELETE:
		doc.SetMark(wParam, doc.GetMark(wParam) & ~(1 << lParam));
		RedrawSelMargin();
		break;

	case SCI_MARKERDELETEALL:
		doc.DeleteAllMarks(static_cast<int>(wParam));
		RedrawSelMargin();
		break;

	case SCI_MARKERGET:
		return doc.GetMark(wParam);

	case SCI_MARKERNEXT: {
			for (int iLine=wParam; iLine<LinesTotal(); iLine++) {
				if ((doc.GetMark(iLine) & lParam) != 0)
					return iLine;
			}
		}
		return -1;

	case SCI_STYLECLEARALL: {
			for (int i=0; i<=STYLE_MAX; i++) {
				styles[i].Clear(foreground.desired, background.desired, 
					size, fontName, bold, italic);
			}
			InvalidateStyleData();
		}
		break;

	case SCI_STYLESETFORE:
		if (wParam <= STYLE_MAX) {
			styles[wParam].fore.desired = ColourFromLparam(lParam);
			InvalidateStyleData();
		}
		break;
	case SCI_STYLESETBACK:
		if (wParam <= STYLE_MAX) {
			styles[wParam].back.desired = ColourFromLparam(lParam);
			InvalidateStyleData();
		}
		break;
	case SCI_STYLESETBOLD:
		if (wParam <= STYLE_MAX) {
			styles[wParam].bold = lParam;
			InvalidateStyleData();
		}
		break;
	case SCI_STYLESETITALIC:
		if (wParam <= STYLE_MAX) {
			styles[wParam].italic = lParam;
			InvalidateStyleData();
		}
		break;
	case SCI_STYLESETSIZE:
		if (wParam <= STYLE_MAX) {
			styles[wParam].size = lParam;
			InvalidateStyleData();
		}
		break;
	case SCI_STYLESETFONT:
		if (wParam <= STYLE_MAX) {
			strcpy(styles[wParam].fontName, reinterpret_cast<char *>(lParam));
			InvalidateStyleData();
		}
		break;

	case SCI_SETFORE:
		foreground.desired = ColourFromLparam(wParam);
		InvalidateStyleData();
		break;

	case SCI_SETBACK:
		background.desired = ColourFromLparam(wParam);
		InvalidateStyleData();
		break;

	case SCI_SETBOLD:
		bold = wParam;
		InvalidateStyleData();
		break;

	case SCI_SETITALIC:
		italic = wParam;
		InvalidateStyleData();
		break;

	case SCI_SETSIZE:
		size = wParam;
		InvalidateStyleData();
		break;

	case SCI_SETFONT:
		strcpy(fontName, reinterpret_cast<char *>(wParam));
		InvalidateStyleData();
		break;

	case SCI_SETSELFORE:
		selforeset = wParam;
		selforeground.desired = ColourFromLparam(lParam);
		InvalidateStyleData();
		break;

	case SCI_SETSELBACK:
		selbackset = wParam;
		selbackground.desired = ColourFromLparam(lParam);
		InvalidateStyleData();
		break;

	case SCI_SETCARETFORE:
		caretcolour.desired = ColourFromLparam(wParam);
		InvalidateStyleData();
		break;

	case SCI_ASSIGNCMDKEY:
		//dprintf("Assign key %d %d\n", wParam, lParam);
		AssignCmdKey(LOWORD(wParam), HIWORD(wParam), lParam);
		break;

	case SCI_CLEARCMDKEY:
		//dprintf("Clear key %d\n", wParam);
		AssignCmdKey(LOWORD(wParam), HIWORD(wParam), WM_NULL);
		break;

	case SCI_CLEARALLCMDKEYS:
		delete []keymap;
		keymap = 0;
		keymapLen = 0;
		keymapAlloc = 0;
		break;

	case SCI_INDICSETSTYLE:
		if (wParam <= INDIC_MAX) {
			indicators[wParam].style = lParam;
			InvalidateStyleData();
		}
		break;

	case SCI_INDICGETSTYLE:
		return (wParam <= INDIC_MAX) ? indicators[wParam].style : 0;

	case SCI_INDICSETFORE:
		if (wParam <= INDIC_MAX) {
			indicators[wParam].fore.desired = ColourFromLparam(lParam);
			InvalidateStyleData();
		}
		break;

	case SCI_INDICGETFORE:
		return (wParam <= INDIC_MAX) ? LparamFromColour(indicators[wParam].fore.desired) : 0;

	case SCI_AUTOCSHOW:
		AutoCompleteStart(reinterpret_cast<char *>(lParam));
		break;

	case SCI_AUTOCCANCEL:
		AutoCompleteCancel();
		break;

	case SCI_AUTOCACTIVE:
		return inAutoCompleteMode;
		break;

	case SCI_AUTOCPOSSTART:
		return posStartAutoComplete;

	case SCI_AUTOCCOMPLETE:
		AutoCompleteCompleted();
		break;

	case SCI_AUTOCSTOPS:
		strcpy(autoCompleteStops, reinterpret_cast<char *>(lParam));
		break;

	case SCI_CALLTIPSHOW:
		CallTipStart(wParam, reinterpret_cast<char *>(lParam));
		break;

	case SCI_CALLTIPCANCEL:
		CallTipCancel();
		break;

	case SCI_CALLTIPACTIVE:
		return inCallTipMode;

	case SCI_CALLTIPPOSSTART:
		return posStartCallTip;

	case SCI_CALLTIPSETHLT:
#ifdef GTK
		startHighlightCT = wParam;
		endHighlightCT = lParam;
		if (hwndCallTip) {
			gtk_widget_queue_draw(hwndCallTip);
		}
#else
		SendMessage(hwndCallTip, SCI_CALLTIPSETHLT, wParam, lParam);
#endif
		break;

	case SCI_GRABFOCUS:
#ifdef GTK
		gtk_widget_grab_focus(hwnd);
#endif
		break;

	case SCI_LINEDOWN:
	case SCI_LINEDOWNEXTEND:
	case SCI_LINEUP:
	case SCI_LINEUPEXTEND:
	case SCI_CHARLEFT:
	case SCI_CHARLEFTEXTEND:
	case SCI_CHARRIGHT:
	case SCI_CHARRIGHTEXTEND:
	case SCI_WORDLEFT:
	case SCI_WORDLEFTEXTEND:
	case SCI_WORDRIGHT:
	case SCI_WORDRIGHTEXTEND:
	case SCI_HOME:
	case SCI_HOMEEXTEND:
	case SCI_LINEEND:
	case SCI_LINEENDEXTEND:
	case SCI_DOCUMENTSTART:
	case SCI_DOCUMENTSTARTEXTEND:
	case SCI_DOCUMENTEND:
	case SCI_DOCUMENTENDEXTEND:
	case SCI_PAGEUP:
	case SCI_PAGEUPEXTEND:
	case SCI_PAGEDOWN:
	case SCI_PAGEDOWNEXTEND:
	case SCI_EDITTOGGLEOVERTYPE:
	case SCI_CANCEL:
	case SCI_DELETEBACK:
	case SCI_TAB:
	case SCI_BACKTAB:
	case SCI_NEWLINE:
	case SCI_FORMFEED:
	case SCI_VCHOME:
	case SCI_VCHOMEEXTEND:
		return KeyCommand(iMessage);

	default:
#ifndef GTK
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
#endif
		break;
	}
	//dprintf("end wnd proc\n");
	return 0l;
}

#ifdef GTK
long scintilla_send_message(ScintillaObject *sci,int iMessage,int wParam,int lParam) {
	Scintilla *psci = reinterpret_cast<Scintilla *>(sci->pscin);
	return psci->WndProc(iMessage, wParam, lParam);
}

static void scintilla_class_init          (ScintillaClass *klass);
static void scintilla_init                (ScintillaObject *sci);

guint scintilla_get_type() {
	static guint scintilla_type = 0;

	if (!scintilla_type) {
		GtkTypeInfo scintilla_info = {
    		"Scintilla",
    		sizeof (ScintillaObject),
    		sizeof (ScintillaClass),
    		(GtkClassInitFunc) scintilla_class_init,
    		(GtkObjectInitFunc) scintilla_init,
    		(GtkArgSetFunc) NULL,
    		(GtkArgGetFunc) NULL
		};

		scintilla_type = gtk_type_unique(gtk_fixed_get_type(), &scintilla_info);
	}

	return scintilla_type;
}

static void scintilla_class_init(ScintillaClass *klass) {
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) klass;

	scintilla_signals[COMMAND_SIGNAL] = gtk_signal_new(
                                        	"command",
                                        	GTK_RUN_LAST,
                                        	object_class->type,
                                        	GTK_SIGNAL_OFFSET(ScintillaClass, command),
                                        	gtk_marshal_NONE__INT_POINTER,
                                        	GTK_TYPE_NONE,
                                        	2, GTK_TYPE_INT, GTK_TYPE_POINTER);

	scintilla_signals[NOTIFY_SIGNAL] = gtk_signal_new(
                                       	"notify",
                                       	GTK_RUN_LAST,
                                       	object_class->type,
                                       	GTK_SIGNAL_OFFSET(ScintillaClass, notify),
                                       	gtk_marshal_NONE__INT_POINTER,
                                       	GTK_TYPE_NONE,
                                       	2, GTK_TYPE_INT, GTK_TYPE_POINTER);

	gtk_object_class_add_signals(object_class,
                             	reinterpret_cast<unsigned int *>(scintilla_signals), LAST_SIGNAL);

	klass->command = NULL;
	klass->notify = NULL;
}

static void scintilla_init(ScintillaObject *sci) {
	GTK_WIDGET_SET_FLAGS(sci, GTK_CAN_FOCUS);
	sci->pscin = new Scintilla(sci);
}

GtkWidget* scintilla_new() {
	return GTK_WIDGET(gtk_type_new(scintilla_get_type()));
}

void scintilla_set_id(ScintillaObject *sci,int id) {
	Scintilla *psci = reinterpret_cast<Scintilla *>(sci->pscin);
	psci->ctrlID = id;
}
#else

const char *scintillaClassName = "Scintilla";

void Scintilla::Register(HINSTANCE hInstance_) {

	hInstance = hInstance_;

	InitCommonControls();

	WNDCLASS wndclass;	// Structure used to register Windows class.

	wndclass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)::Scintilla::SWndProc;
	wndclass.cbClsExtra = 0;
	// Reserve extra bytes for each instance of the window;
	// we will use these bytes to store a pointer to the C++
	// (Scintilla) object corresponding to the window.
	wndclass.cbWndExtra = sizeof(Scintilla*);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL,IDC_IBEAM);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = scintillaClassName;

	if (!RegisterClass(&wndclass)) {
		//dprintf("Could not register class\n");
		exit(FALSE);
	}
}

LRESULT PASCAL Scintilla::SWndProc(
    HWND hWnd,UINT iMessage,WPARAM wParam, LPARAM lParam) {
	//dprintf("S W:%x M:%d WP:%x L:%x\n", hWnd, iMessage, wParam, lParam);

	// Find C++ object associated with window.
	Scintilla *sci = reinterpret_cast<Scintilla *>(GetWindowLong(hWnd,0));
	// sci will be zero if WM_CREATE not seen yet
	if (sci == 0) {
		if (iMessage == WM_CREATE) {
			// Create C++ object associated with window
			sci = new Scintilla();
			sci->hwnd = hWnd;
			SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(sci));
			return sci->WndProc(iMessage, wParam, lParam);
		} else {
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			delete sci;
			SetWindowLong(hWnd, 0, 0);
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else {
			return sci->WndProc(iMessage, wParam, lParam);
		}
	}
}

// This function is externally visible so it can be called from container when building statically
void Scintilla_RegisterClasses(HINSTANCE hInstance) {
	CallTip_Register(hInstance);
	Scintilla::Register(hInstance);
}

#ifndef STATIC_BUILD
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
	//dprintf("Scintilla::DllMain %d %d\n", hInstance, dwReason);
	if (dwReason == DLL_PROCESS_ATTACH) {
		Scintilla_RegisterClasses(hInstance);
	}
	return TRUE;
}
#endif

#endif

