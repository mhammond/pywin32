// Scintilla source code edit control
/** @file PlatWin.cxx
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "Platform.h"
#include "PlatformRes.h"
#include "UniConversion.h"

Point Point::FromLong(long lpoint) {
	return Point(static_cast<short>(LOWORD(lpoint)), static_cast<short>(HIWORD(lpoint)));
}

static RECT RectFromPRectangle(PRectangle prc) {
	RECT rc = {prc.left, prc.top, prc.right, prc.bottom};
	return rc;
}

Colour::Colour(long lcol) {
	co = lcol;
}

Colour::Colour(unsigned int red, unsigned int green, unsigned int blue) {
	co = RGB(red, green, blue);
}

bool Colour::operator==(const Colour &other) const {
	return co == other.co;
}

long Colour::AsLong() const {
	return co;
}

unsigned int Colour::GetRed() {
	return co & 0xff;
}

unsigned int Colour::GetGreen() {
	return (co >> 8) & 0xff;
}

unsigned int Colour::GetBlue() {
	return (co >> 16) & 0xff;
}

Palette::Palette() {
	used = 0;
	allowRealization = false;
	hpal = 0;
}

Palette::~Palette() {
	Release();
}

void Palette::Release() {
	used = 0;
	if (hpal)
		::DeleteObject(hpal);
	hpal = 0;
}

/**
 * This method either adds a colour to the list of wanted colours (want==true)
 * or retrieves the allocated colour back to the ColourPair.
 * This is one method to make it easier to keep the code for wanting and retrieving in sync.
 */
void Palette::WantFind(ColourPair &cp, bool want) {
	if (want) {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired)
				return;
		}

		if (used < numEntries) {
			entries[used].desired = cp.desired;
			entries[used].allocated = cp.desired;
			used++;
		}
	} else {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return;
			}
		}
		cp.allocated = cp.desired;
	}
}

void Palette::Allocate(Window &) {
	if (hpal)
		::DeleteObject(hpal);
	hpal = 0;

	if (allowRealization) {
		char *pal = new char[sizeof(LOGPALETTE) + (used-1) * sizeof(PALETTEENTRY)];
		LOGPALETTE *logpal = reinterpret_cast<LOGPALETTE *>(pal);
		logpal->palVersion = 0x300;
		logpal->palNumEntries = static_cast<WORD>(used);
		for (int iPal=0;iPal<used;iPal++) {
			Colour desired = entries[iPal].desired;
			logpal->palPalEntry[iPal].peRed   = static_cast<BYTE>(desired.GetRed());
			logpal->palPalEntry[iPal].peGreen = static_cast<BYTE>(desired.GetGreen());
			logpal->palPalEntry[iPal].peBlue  = static_cast<BYTE>(desired.GetBlue());
			entries[iPal].allocated =
				PALETTERGB(desired.GetRed(), desired.GetGreen(), desired.GetBlue());
			// PC_NOCOLLAPSE means exact colours allocated even when in background this means other windows
			// are less likely to get their colours and also flashes more when switching windows
			logpal->palPalEntry[iPal].peFlags = PC_NOCOLLAPSE;
			// 0 allows approximate colours when in background, yielding moe colours to other windows
			//logpal->palPalEntry[iPal].peFlags = 0;
		}
		hpal = ::CreatePalette(logpal);
		delete []pal;
	}
}

void SetLogFont(LOGFONT &lf, const char *faceName, int characterSet, int size, bool bold, bool italic) {
	memset(&lf, 0, sizeof(lf));
	// The negative is to allow for leading
	lf.lfHeight = -(abs(size));
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = static_cast<BYTE>(italic ? 1 : 0);
	lf.lfCharSet = static_cast<BYTE>(characterSet);
	strcpy(lf.lfFaceName, faceName);
}

/**
 * Create a hash from the parameters for a font to allow easy checking for identity.
 * If one font is the same as another, its hash will be the same, but if the hash is the
 * same then they may still be different.
 */
int HashFont(const char *faceName, int characterSet, int size, bool bold, bool italic) {
    return
        size ^
        (characterSet << 10) ^
        (bold ? 0x10000000 : 0) ^
        (italic ? 0x20000000 : 0) ^
        faceName[0];
}

class FontCached : Font {
	FontCached *next;
	int usage;
	LOGFONT lf;
	int hash;
	FontCached(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_);
	~FontCached() {}
	bool SameAs(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_);
	virtual void Release();

	static FontCached *first;
public:
	static FontID FindOrCreate(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_);
	static void ReleaseId(FontID id_);
};

FontCached *FontCached::first = 0;

FontCached::FontCached(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_) :
    next(0), usage(0), hash(0) {
    SetLogFont(lf, faceName_, characterSet_, size_, bold_, italic_);
    hash = HashFont(faceName_, characterSet_, size_, bold_, italic_);
	id = ::CreateFontIndirect(&lf);
	usage = 1;
}

bool FontCached::SameAs(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_) {
	return
        (lf.lfHeight == -(abs(size_))) &&
		(lf.lfWeight == (bold_ ? FW_BOLD : FW_NORMAL)) &&
		(lf.lfItalic == static_cast<BYTE>(italic_ ? 1 : 0)) &&
		(lf.lfCharSet == characterSet_) &&
		0 == strcmp(lf.lfFaceName,faceName_);
}

void FontCached::Release() {
	if (id)
		::DeleteObject(id);
    id = 0;
}

FontID FontCached::FindOrCreate(const char *faceName_, int characterSet_, int size_, bool bold_, bool italic_) {
    int hashFind = HashFont(faceName_, characterSet_, size_, bold_, italic_);
	for (FontCached *cur=first; cur; cur=cur->next) {
        if ((cur->hash == hashFind) &&
            cur->SameAs(faceName_, characterSet_, size_, bold_, italic_)) {
			cur->usage++;
			return cur->id;
		}
	}
	FontCached *fc = new FontCached(faceName_, characterSet_, size_, bold_, italic_);
	if (fc) {
		fc->next = first;
		first = fc;
		return fc->id;
	} else {
		return 0;
	}
}

void FontCached::ReleaseId(FontID id_) {
	FontCached **pcur=&first;
	for (FontCached *cur=first; cur; cur=cur->next) {
		if (cur->id == id_) {
			cur->usage--;
			if (cur->usage == 0) {
				*pcur = cur->next;
                cur->Release();
				cur->next = 0;
				delete cur;
			}
			return;
		}
		pcur=&cur->next;
	}
}

Font::Font() {
	id = 0;
}

Font::~Font() {
}

#define FONTS_CACHED

void Font::Create(const char *faceName, int characterSet, int size, bool bold, bool italic) {
#ifndef FONTS_CACHED
	Release();

	LOGFONT lf;
    SetLogFont(lf, faceName, characterSet, size, bold, italic);
    id = ::CreateFontIndirect(&lf);
#else
	id = FontCached::FindOrCreate(faceName, characterSet, size, bold, italic);
#endif
}

void Font::Release() {
#ifndef FONTS_CACHED
	if (id)
		::DeleteObject(id);
#else
	if (id)
		FontCached::ReleaseId(id);
#endif
	id = 0;
}

Surface::Surface() :
	unicodeMode(false),
	hdc(0), 	hdcOwned(false),
	pen(0), 	penOld(0),
	brush(0), brushOld(0),
	font(0), 	fontOld(0),
	bitmap(0), bitmapOld(0),
	paletteOld(0) {
}

Surface::~Surface() {
	Release();
}

void Surface::Release() {
	if (penOld) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		penOld = 0;
	}
	pen = 0;
	if (brushOld) {
		::SelectObject(hdc, brushOld);
		::DeleteObject(brush);
		brushOld = 0;
	}
	brush = 0;
	if (fontOld) {
		// Fonts are not deleted as they are owned by a Font object
		::SelectObject(hdc, fontOld);
		fontOld = 0;
	}
	font =0;
	if (bitmapOld) {
		::SelectObject(hdc, bitmapOld);
		::DeleteObject(bitmap);
		bitmapOld = 0;
	}
	bitmap = 0;
	if (paletteOld) {
		// Fonts are not deleted as they are owned by a Palette object
		::SelectPalette(hdc, paletteOld, TRUE);
		paletteOld = 0;
	}
	if (hdcOwned) {
		::DeleteDC(hdc);
		hdc = 0;
		hdcOwned = false;
	}
}

bool Surface::Initialised() {
	return hdc;
}

void Surface::Init() {
	Release();
	hdc = ::CreateCompatibleDC(NULL);
	hdcOwned = true;
	::SetTextAlign(hdc, TA_BASELINE);
}

void Surface::Init(SurfaceID sid) {
	Release();
	hdc = sid;
	::SetTextAlign(hdc, TA_BASELINE);
}

void Surface::InitPixMap(int width, int height, Surface *surface_) {
	Release();
	hdc = ::CreateCompatibleDC(surface_->hdc);
	hdcOwned = true;
	bitmap = ::CreateCompatibleBitmap(surface_->hdc, width, height);
	bitmapOld = static_cast<HBITMAP>(::SelectObject(hdc, bitmap));
	::SetTextAlign(hdc, TA_BASELINE);
}

void Surface::PenColour(Colour fore) {
	if (pen) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	pen = ::CreatePen(0,1,fore.AsLong());
	penOld = static_cast<HPEN>(::SelectObject(hdc, pen));
}

void Surface::BrushColor(Colour back) {
	if (brush) {
		::SelectObject(hdc, brushOld);
		::DeleteObject(brush);
		brush = 0;
		brushOld = 0;
	}
	// Only ever want pure, non-dithered brushes
	Colour colourNearest = ::GetNearestColor(hdc, back.AsLong());
	brush = ::CreateSolidBrush(colourNearest.AsLong());
	brushOld = static_cast<HBRUSH>(::SelectObject(hdc, brush));
}

void Surface::SetFont(Font &font_) {
	if (font_.GetID() != font) {
		if (fontOld) {
			::SelectObject(hdc, font_.GetID());
		} else {
			fontOld = static_cast<HFONT>(::SelectObject(hdc, font_.GetID()));
		}
		font = font_.GetID();
	}
}

int Surface::LogPixelsY() {
	return ::GetDeviceCaps(hdc, LOGPIXELSY);
}

int Surface::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), 72);
}

void Surface::MoveTo(int x_, int y_) {
	::MoveToEx(hdc, x_, y_, 0);
}

void Surface::LineTo(int x_, int y_) {
	::LineTo(hdc, x_, y_);
}

void Surface::Polygon(Point *pts, int npts, Colour fore,
                      Colour back) {
	PenColour(fore);
	BrushColor(back);
	::Polygon(hdc, reinterpret_cast<POINT *>(pts), npts);
}

void Surface::RectangleDraw(PRectangle rc, Colour fore, Colour back) {
	PenColour(fore);
	BrushColor(back);
	::Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void Surface::FillRectangle(PRectangle rc, Colour back) {
	// Using ExtTextOut rather than a FillRect ensures that no dithering occurs.
	// There is no need to allocate a brush either.
	RECT rcw = RectFromPRectangle(rc);
	::SetBkColor(hdc, back.AsLong());
	::ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE, &rcw, "", 0, NULL);
}

void Surface::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	HBRUSH br;
	if (surfacePattern.bitmap)
		br = ::CreatePatternBrush(surfacePattern.bitmap);
	else	// Something is wrong so display in red
		br = ::CreateSolidBrush(RGB(0xff, 0, 0));
	RECT rcw = RectFromPRectangle(rc);
	::FillRect(hdc, &rcw, br);
	::DeleteObject(br);
}

void Surface::RoundedRectangle(PRectangle rc, Colour fore, Colour back) {
	PenColour(fore);
	BrushColor(back);
	::RoundRect(hdc,
          	rc.left + 1, rc.top,
          	rc.right - 1, rc.bottom,
          	8, 8 );
}

void Surface::Ellipse(PRectangle rc, Colour fore, Colour back) {
	PenColour(fore);
	BrushColor(back);
	::Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void Surface::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	::BitBlt(hdc, rc.left, rc.top, rc.Width(), rc.Height(),
		surfaceSource.hdc, from.x, from.y, SRCCOPY);
}

#define MAX_US_LEN 5000

void Surface::DrawText(PRectangle rc, Font &font_, int ybase, const char *s, int len, Colour fore, Colour back) {
	SetFont(font_);
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	RECT rcw = RectFromPRectangle(rc);
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t));
		tbuf[tlen] = L'\0';
		::ExtTextOutW(hdc, rc.left, ybase, ETO_OPAQUE, &rcw, tbuf, tlen, NULL);
	} else {
		::ExtTextOut(hdc, rc.left, ybase, ETO_OPAQUE, &rcw, s, len, NULL);
	}
}

void Surface::DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len, Colour fore, Colour back) {
	SetFont(font_);
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	RECT rcw = RectFromPRectangle(rc);
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t));
		tbuf[tlen] = L'\0';
		::ExtTextOutW(hdc, rc.left, ybase, ETO_OPAQUE | ETO_CLIPPED, &rcw, tbuf, tlen, NULL);
	} else {
		::ExtTextOut(hdc, rc.left, ybase, ETO_OPAQUE | ETO_CLIPPED, &rcw, s, len, NULL);
	}
}

int Surface::WidthText(Font &font_, const char *s, int len) {
	SetFont(font_);
	SIZE sz={0,0};
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t));
		tbuf[tlen] = L'\0';
		::GetTextExtentPoint32W(hdc, tbuf, tlen, &sz);
	} else {
		::GetTextExtentPoint32(hdc, s, len, &sz);
	}
	return sz.cx;
}

void Surface::MeasureWidths(Font &font_, const char *s, int len, int *positions) {
	SetFont(font_);
	SIZE sz={0,0};
	int fit = 0;
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t));
		tbuf[tlen] = L'\0';
		int poses[MAX_US_LEN];
		fit = tlen;
		if (!::GetTextExtentExPointW(hdc, tbuf, tlen, 30000, &fit, poses, &sz)) {
			// Likely to have failed because on Windows 9x where function not available
			// So measure the character widths by measuring each initial substring
			// Turns a linear operation into a qudratic but seems fast enough on test files
			for (int widthSS=0; widthSS < tlen; widthSS++) {
				::GetTextExtentPoint32W(hdc, tbuf, widthSS+1, &sz);
				poses[widthSS] = sz.cx;
			}
		}
		// Map the widths given for UCS-2 characters back onto the UTF-8 input string
		int ui=0;
		const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
		int i=0;
		while (i<len) {
			unsigned char uch = us[i];
			positions[i++] = poses[ui];
			if (uch >= 0x80) {
				if (uch < (0x80 + 0x40 + 0x20)) {
					positions[i++] = poses[ui];
				} else {
					positions[i++] = poses[ui];
					positions[i++] = poses[ui];
				}
			}
			ui++;
		}
		positions[i] = sz.cx;
	} else {
		if (!::GetTextExtentExPoint(hdc, s, len, 30000, &fit, positions, &sz)) {
			// Eeek - a NULL DC or other foolishness could cause this.
			// The least we can do is set the positions to zero!
			memset(positions, 0, len * sizeof(*positions));
		}
	}
}

int Surface::WidthChar(Font &font_, char ch) {
	SetFont(font_);
	SIZE sz;
	::GetTextExtentPoint32(hdc, &ch, 1, &sz);
	return sz.cx;
}

int Surface::Ascent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmAscent;
}

int Surface::Descent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmDescent;
}

int Surface::InternalLeading(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmInternalLeading;
}

int Surface::ExternalLeading(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmExternalLeading;
}

int Surface::Height(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmHeight;
}

int Surface::AverageCharWidth(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmAveCharWidth;
}

int Surface::SetPalette(Palette *pal, bool inBackGround) {
	if (paletteOld) {
		::SelectPalette(hdc,paletteOld,TRUE);
	}
	paletteOld = 0;
	int changes = 0;
	if (pal->allowRealization) {
		paletteOld = ::SelectPalette(hdc, pal->hpal, inBackGround);
		changes = ::RealizePalette(hdc);
	}
	return changes;
}

void Surface::SetClip(PRectangle rc) {
	::IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void Surface::FlushCachedState() {
	pen = 0;
	brush = 0;
	font = 0;
}

Window::~Window() {
}

void Window::Destroy() {
	if (id)
		::DestroyWindow(id);
	id = 0;
}

bool Window::HasFocus() {
	return ::GetFocus() == id;
}

PRectangle Window::GetPosition() {
	RECT rc;
	::GetWindowRect(id, &rc);
	return PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::SetPosition(PRectangle rc) {
	::SetWindowPos(id, 0, rc.left, rc.top, rc.Width(), rc.Height(), 0);
}

void Window::SetPositionRelative(PRectangle rc, Window) {
	SetPosition(rc);
}

PRectangle Window::GetClientPosition() {
	RECT rc;
	::GetClientRect(id, &rc);
	return  PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::Show(bool show) {
	if (show)
		::ShowWindow(id, SW_SHOWNORMAL);
	else
		::ShowWindow(id, SW_HIDE);
}

void Window::InvalidateAll() {
	::InvalidateRect(id, NULL, FALSE);
}

void Window::InvalidateRectangle(PRectangle rc) {
	RECT rcw = RectFromPRectangle(rc);
	::InvalidateRect(id, &rcw, FALSE);
}

void Window::SetFont(Font &font) {
	SendMessage(WM_SETFONT,
		reinterpret_cast<WPARAM>(font.GetID()), 0);
}

static HINSTANCE hinstPlatformRes = 0;

void Window::SetCursor(Cursor curs) {
	switch (curs) {
	case cursorText:
		::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
		break;
	case cursorArrow:
		::SetCursor(::LoadCursor(NULL,IDC_ARROW));
		break;
	case cursorUp:
		::SetCursor(::LoadCursor(NULL,IDC_UPARROW));
		break;
	case cursorWait:
		::SetCursor(::LoadCursor(NULL,IDC_WAIT));
		break;
	case cursorHoriz:
		::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
		break;
	case cursorVert:
		::SetCursor(::LoadCursor(NULL,IDC_SIZENS));
		break;
	case cursorReverseArrow: {
			if (!hinstPlatformRes)
				hinstPlatformRes = GetModuleHandle("Scintilla");
			if (!hinstPlatformRes)
				hinstPlatformRes = GetModuleHandle("SciLexer");
			if (!hinstPlatformRes)
				hinstPlatformRes = GetModuleHandle(NULL);
			::SetCursor(::LoadCursor(hinstPlatformRes, MAKEINTRESOURCE(IDC_MARGIN)));
		}
		break;
	}
}

void Window::SetTitle(const char *s) {
	::SetWindowText(id, s);
}

LRESULT Window::SendMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	if (id)
		return ::SendMessage(id, msg, wParam, lParam);
	else
		return 0;
}

int Window::GetDlgCtrlID() {
	return ::GetDlgCtrlID(id);
}

HINSTANCE Window::GetInstance() {
	return reinterpret_cast<HINSTANCE>(
		::GetWindowLong(id,GWL_HINSTANCE));
}

ListBox::ListBox() : desiredVisibleRows(5), maxItemCharacters(0), aveCharWidth(8) {
}

ListBox::~ListBox() {
}

void ListBox::Create(Window &parent, int ctrlID) {
	id = ::CreateWindowEx(
                WS_EX_WINDOWEDGE, "listbox", "",
       		WS_CHILD | WS_THICKFRAME | WS_VSCROLL | LBS_SORT | LBS_NOTIFY,
       		100,100, 150,80, parent.GetID(), reinterpret_cast<HMENU>(ctrlID),
		parent.GetInstance(), 0);
}

void ListBox::SetFont(Font &font) {
	Window::SetFont(font);
}

void ListBox::SetAverageCharWidth(int width) {
    aveCharWidth = width;
}

void ListBox::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

PRectangle ListBox::GetDesiredRect() {
	PRectangle rcDesired = GetPosition();
	int itemHeight = SendMessage(LB_GETITEMHEIGHT, 0);
	int rows = Length();
	if ((rows == 0) || (rows > desiredVisibleRows))
		rows = desiredVisibleRows;
	// The +6 allows for borders
	rcDesired.bottom = rcDesired.top + 6 + itemHeight * rows;
    int width = maxItemCharacters;
    if (width < 12)
        width = 12;
	rcDesired.right = rcDesired.left + width * (aveCharWidth+aveCharWidth/3);
    if (Length() > rows)
        rcDesired.right = rcDesired.right + GetSystemMetrics(SM_CXVSCROLL);
	return rcDesired;
}

void ListBox::Clear() {
	SendMessage(LB_RESETCONTENT);
    maxItemCharacters = 0;
}

void ListBox::Append(char *s) {
	SendMessage(LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
    size_t len = strlen(s);
    if (maxItemCharacters < len)
        maxItemCharacters = len;
}

int ListBox::Length() {
	return SendMessage(LB_GETCOUNT);
}

void ListBox::Select(int n) {
	SendMessage(LB_SETCURSEL, n);
}

int ListBox::GetSelection() {
	return SendMessage(LB_GETCURSEL);
}

int ListBox::Find(const char *prefix) {
	return SendMessage(LB_FINDSTRING, static_cast<WPARAM>(-1),
        reinterpret_cast<LPARAM>(prefix));
}

void ListBox::GetValue(int n, char *value, int len) {
	int lenText = SendMessage(LB_GETTEXTLEN, n);
	if ((len > 0) && (lenText > 0)){
		char *text = new char[len+1];
		if (text) {
			SendMessage(LB_GETTEXT, n, reinterpret_cast<LPARAM>(text));
			strncpy(value, text, len);
			value[len-1] = '\0';
			delete []text;
		} else {
			value[0] = '\0';
		}
	} else {
		value[0] = '\0';
	}
}

void ListBox::Sort() {
	// Windows keeps sorted so no need to sort
}

Menu::Menu() : id(0) {
}

void Menu::CreatePopUp() {
	Destroy();
	id = ::CreatePopupMenu();
}

void Menu::Destroy() {
	if (id)
		::DestroyMenu(id);
	id = 0;
}

void Menu::Show(Point pt, Window &w) {
	::TrackPopupMenu(id, 0, pt.x - 4, pt.y, 0, w.GetID(), NULL);
	Destroy();
}

Colour Platform::Chrome() {
	return ::GetSysColor(COLOR_3DFACE);
}

Colour Platform::ChromeHighlight() {
	return ::GetSysColor(COLOR_3DHIGHLIGHT);
}

const char *Platform::DefaultFont() {
	return "Verdana";
}

int Platform::DefaultFontSize() {
	return 8;
}

unsigned int Platform::DoubleClickTime() {
	return ::GetDoubleClickTime();
}

void Platform::DebugDisplay(const char *s) {
	::OutputDebugString(s);
}

bool Platform::IsKeyDown(int key) {
	return ::GetKeyState(key) & 0x80000000;
}

long Platform::SendScintilla(WindowID w, unsigned int msg, unsigned long wParam, long lParam) {
	return ::SendMessage(w, msg, wParam, lParam);
}

// These are utility functions not really tied to a platform

int Platform::Minimum(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

int Platform::Maximum(int a, int b) {
	if (a > b)
		return a;
	else
		return b;
}

//#define TRACE

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...) {
	char buffer[2000];
	va_list pArguments;
	va_start(pArguments, format);
	vsprintf(buffer,format,pArguments);
	va_end(pArguments);
	Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {
}
#endif

static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
	bool ret = assertionPopUps;
	assertionPopUps = assertionPopUps_;
	return ret;
}

void Platform::Assert(const char *c, const char *file, int line) {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d", c, file, line);
	if (assertionPopUps) {
		int idButton = ::MessageBox(0, buffer, "Assertion failure",
			MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
		if (idButton == IDRETRY) {
			::DebugBreak();
		} else if (idButton == IDIGNORE) {
			// all OK
		} else {
			abort();
		}
	} else {
		strcat(buffer, "\r\n");
		Platform::DebugDisplay(buffer);
		abort();
	}
}

int Platform::Clamp(int val, int minVal, int maxVal) {
	if (val > maxVal)
		val = maxVal;
	if (val < minVal)
		val = minVal;
	return val;
}
