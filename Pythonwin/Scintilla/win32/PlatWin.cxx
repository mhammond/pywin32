// Scintilla source code edit control
/** @file PlatWin.cxx
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define _WIN32_WINNT  0x0400
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <windowsx.h>

#include "Platform.h"
#include "PlatformRes.h"
#include "UniConversion.h"
#include "XPM.h"

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

// Take care of 32/64 bit pointers
#ifdef GetWindowLongPtr
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(ptr));
}
#else
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLong(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(ptr));
}
#endif

static CRITICAL_SECTION crPlatformLock;
static HINSTANCE hinstPlatformRes = 0;
static bool onNT = false;

bool IsNT() {
	return onNT;
}

Point Point::FromLong(long lpoint) {
	return Point(static_cast<short>(LOWORD(lpoint)), static_cast<short>(HIWORD(lpoint)));
}

static RECT RectFromPRectangle(PRectangle prc) {
	RECT rc = {prc.left, prc.top, prc.right, prc.bottom};
	return rc;
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
			entries[used].allocated.Set(cp.desired.AsLong());
			used++;
		}
	} else {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return;
			}
		}
		cp.allocated.Set(cp.desired.AsLong());
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
			ColourDesired desired = entries[iPal].desired;
			logpal->palPalEntry[iPal].peRed   = static_cast<BYTE>(desired.GetRed());
			logpal->palPalEntry[iPal].peGreen = static_cast<BYTE>(desired.GetGreen());
			logpal->palPalEntry[iPal].peBlue  = static_cast<BYTE>(desired.GetBlue());
			entries[iPal].allocated.Set(
				PALETTERGB(desired.GetRed(), desired.GetGreen(), desired.GetBlue()));
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

static void SetLogFont(LOGFONT &lf, const char *faceName, int characterSet, int size, bool bold, bool italic) {
	memset(&lf, 0, sizeof(lf));
	// The negative is to allow for leading
	lf.lfHeight = -(abs(size));
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = static_cast<BYTE>(italic ? 1 : 0);
	lf.lfCharSet = static_cast<BYTE>(characterSet);
	strncpy(lf.lfFaceName, faceName, sizeof(lf.lfFaceName));
}

/**
 * Create a hash from the parameters for a font to allow easy checking for identity.
 * If one font is the same as another, its hash will be the same, but if the hash is the
 * same then they may still be different.
 */
static int HashFont(const char *faceName, int characterSet, int size, bool bold, bool italic) {
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
	::SetLogFont(lf, faceName_, characterSet_, size_, bold_, italic_);
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
	FontID ret = 0;
	::EnterCriticalSection(&crPlatformLock);
	int hashFind = HashFont(faceName_, characterSet_, size_, bold_, italic_);
	for (FontCached *cur=first; cur; cur=cur->next) {
		if ((cur->hash == hashFind) &&
			cur->SameAs(faceName_, characterSet_, size_, bold_, italic_)) {
			cur->usage++;
			ret = cur->id;
		}
	}
	if (ret == 0) {
		FontCached *fc = new FontCached(faceName_, characterSet_, size_, bold_, italic_);
		if (fc) {
			fc->next = first;
			first = fc;
			ret = fc->id;
		}
	}
	::LeaveCriticalSection(&crPlatformLock);
	return ret;
}

void FontCached::ReleaseId(FontID id_) {
	::EnterCriticalSection(&crPlatformLock);
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
			break;
		}
		pcur=&cur->next;
	}
	::LeaveCriticalSection(&crPlatformLock);
}

Font::Font() {
	id = 0;
}

Font::~Font() {
}

#define FONTS_CACHED

void Font::Create(const char *faceName, int characterSet, int size, bool bold, bool italic) {
	Release();
#ifndef FONTS_CACHED
	LOGFONT lf;
	::SetLogFont(lf, faceName, characterSet, size, bold, italic);
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

class SurfaceImpl : public Surface {
	bool unicodeMode;
	HDC hdc;
	bool hdcOwned;
	HPEN pen;
	HPEN penOld;
	HBRUSH brush;
	HBRUSH brushOld;
	HFONT font;
	HFONT fontOld;
	HBITMAP bitmap;
	HBITMAP bitmapOld;
	HPALETTE paletteOld;
	int maxWidthMeasure;
	int maxLenText;

	void BrushColor(ColourAllocated back);
	void SetFont(Font &font_);

	// Private so SurfaceImpl objects can not be copied
	SurfaceImpl(const SurfaceImpl &) : Surface() {}
	SurfaceImpl &operator=(const SurfaceImpl &) { return *this; }
public:
	SurfaceImpl();
	virtual ~SurfaceImpl();

	void Init(WindowID wid);
	void Init(SurfaceID sid, WindowID wid);
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid);

	void Release();
	bool Initialised();
	void PenColour(ColourAllocated fore);
	int LogPixelsY();
	int DeviceHeightFont(int points);
	void MoveTo(int x_, int y_);
	void LineTo(int x_, int y_);
	void Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back);
	void RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void FillRectangle(PRectangle rc, ColourAllocated back);
	void FillRectangle(PRectangle rc, Surface &surfacePattern);
	void RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Copy(PRectangle rc, Point from, Surface &surfaceSource);

	void DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextTransparent(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore);
	void MeasureWidths(Font &font_, const char *s, int len, int *positions);
	int WidthText(Font &font_, const char *s, int len);
	int WidthChar(Font &font_, char ch);
	int Ascent(Font &font_);
	int Descent(Font &font_);
	int InternalLeading(Font &font_);
	int ExternalLeading(Font &font_);
	int Height(Font &font_);
	int AverageCharWidth(Font &font_);

	int SetPalette(Palette *pal, bool inBackGround);
	void SetClip(PRectangle rc);
	void FlushCachedState();

	void SetUnicodeMode(bool unicodeMode_);
	void SetDBCSMode(int codePage);
};

SurfaceImpl::SurfaceImpl() :
	unicodeMode(false),
	hdc(0), 	hdcOwned(false),
	pen(0), 	penOld(0),
	brush(0), brushOld(0),
	font(0), 	fontOld(0),
	bitmap(0), bitmapOld(0),
	paletteOld(0) {
	// Windows 9x has only a 16 bit coordinate system so break after 30000 pixels
	maxWidthMeasure = IsNT() ? 1000000 : 30000;
	// There appears to be a 16 bit string length limit in GDI on NT and a limit of
	// 8192 characters on Windows 95.
	maxLenText = IsNT() ? 65535 : 8192;
}

SurfaceImpl::~SurfaceImpl() {
	Release();
}

void SurfaceImpl::Release() {
	if (penOld) {
		::SelectObject(reinterpret_cast<HDC>(hdc), penOld);
		::DeleteObject(pen);
		penOld = 0;
	}
	pen = 0;
	if (brushOld) {
		::SelectObject(reinterpret_cast<HDC>(hdc), brushOld);
		::DeleteObject(brush);
		brushOld = 0;
	}
	brush = 0;
	if (fontOld) {
		// Fonts are not deleted as they are owned by a Font object
		::SelectObject(reinterpret_cast<HDC>(hdc), fontOld);
		fontOld = 0;
	}
	font =0;
	if (bitmapOld) {
		::SelectObject(reinterpret_cast<HDC>(hdc), bitmapOld);
		::DeleteObject(bitmap);
		bitmapOld = 0;
	}
	bitmap = 0;
	if (paletteOld) {
		// Fonts are not deleted as they are owned by a Palette object
		::SelectPalette(reinterpret_cast<HDC>(hdc),
			reinterpret_cast<HPALETTE>(paletteOld), TRUE);
		paletteOld = 0;
	}
	if (hdcOwned) {
		::DeleteDC(reinterpret_cast<HDC>(hdc));
		hdc = 0;
		hdcOwned = false;
	}
}

bool SurfaceImpl::Initialised() {
	return hdc != 0;
}

void SurfaceImpl::Init(WindowID) {
	Release();
	hdc = ::CreateCompatibleDC(NULL);
	hdcOwned = true;
	::SetTextAlign(reinterpret_cast<HDC>(hdc), TA_BASELINE);
}

void SurfaceImpl::Init(SurfaceID sid, WindowID) {
	Release();
	hdc = reinterpret_cast<HDC>(sid);
	::SetTextAlign(reinterpret_cast<HDC>(hdc), TA_BASELINE);
}

void SurfaceImpl::InitPixMap(int width, int height, Surface *surface_, WindowID) {
	Release();
	hdc = ::CreateCompatibleDC(static_cast<SurfaceImpl *>(surface_)->hdc);
	hdcOwned = true;
	bitmap = ::CreateCompatibleBitmap(static_cast<SurfaceImpl *>(surface_)->hdc, width, height);
	bitmapOld = static_cast<HBITMAP>(::SelectObject(hdc, bitmap));
	::SetTextAlign(reinterpret_cast<HDC>(hdc), TA_BASELINE);
}

void SurfaceImpl::PenColour(ColourAllocated fore) {
	if (pen) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	pen = ::CreatePen(0,1,fore.AsLong());
	penOld = static_cast<HPEN>(::SelectObject(reinterpret_cast<HDC>(hdc), pen));
}

void SurfaceImpl::BrushColor(ColourAllocated back) {
	if (brush) {
		::SelectObject(hdc, brushOld);
		::DeleteObject(brush);
		brush = 0;
		brushOld = 0;
	}
	// Only ever want pure, non-dithered brushes
	ColourAllocated colourNearest = ::GetNearestColor(hdc, back.AsLong());
	brush = ::CreateSolidBrush(colourNearest.AsLong());
	brushOld = static_cast<HBRUSH>(::SelectObject(hdc, brush));
}

void SurfaceImpl::SetFont(Font &font_) {
	if (font_.GetID() != font) {
		if (fontOld) {
			::SelectObject(hdc, font_.GetID());
		} else {
			fontOld = static_cast<HFONT>(::SelectObject(hdc, font_.GetID()));
		}
		font = reinterpret_cast<HFONT>(font_.GetID());
	}
}

int SurfaceImpl::LogPixelsY() {
	return ::GetDeviceCaps(hdc, LOGPIXELSY);
}

int SurfaceImpl::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), 72);
}

void SurfaceImpl::MoveTo(int x_, int y_) {
	::MoveToEx(hdc, x_, y_, 0);
}

void SurfaceImpl::LineTo(int x_, int y_) {
	::LineTo(hdc, x_, y_);
}

void SurfaceImpl::Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back) {
	PenColour(fore);
	BrushColor(back);
	::Polygon(hdc, reinterpret_cast<POINT *>(pts), npts);
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	PenColour(fore);
	BrushColor(back);
	::Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourAllocated back) {
	// Using ExtTextOut rather than a FillRect ensures that no dithering occurs.
	// There is no need to allocate a brush either.
	RECT rcw = RectFromPRectangle(rc);
	::SetBkColor(hdc, back.AsLong());
	::ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE, &rcw, "", 0, NULL);
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	HBRUSH br;
	if (static_cast<SurfaceImpl &>(surfacePattern).bitmap)
		br = ::CreatePatternBrush(static_cast<SurfaceImpl &>(surfacePattern).bitmap);
	else	// Something is wrong so display in red
		br = ::CreateSolidBrush(RGB(0xff, 0, 0));
	RECT rcw = RectFromPRectangle(rc);
	::FillRect(hdc, &rcw, br);
	::DeleteObject(br);
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	PenColour(fore);
	BrushColor(back);
	::RoundRect(hdc,
		rc.left + 1, rc.top,
		rc.right - 1, rc.bottom,
		8, 8 );
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	PenColour(fore);
	BrushColor(back);
	::Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	::BitBlt(hdc,
		rc.left, rc.top, rc.Width(), rc.Height(),
		static_cast<SurfaceImpl &>(surfaceSource).hdc, from.x, from.y, SRCCOPY);
}

#define MAX_US_LEN 10000

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len,
	ColourAllocated fore, ColourAllocated back) {
	SetFont(font_);
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	RECT rcw = RectFromPRectangle(rc);
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
		tbuf[tlen] = L'\0';
		::ExtTextOutW(hdc, rc.left, ybase, ETO_OPAQUE, &rcw, tbuf, tlen, NULL);
	} else {
		::ExtTextOut(hdc, rc.left, ybase, ETO_OPAQUE, &rcw, s,
			Platform::Minimum(len, maxLenText), NULL);
	}
}

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len,
	ColourAllocated fore, ColourAllocated back) {
	SetFont(font_);
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	RECT rcw = RectFromPRectangle(rc);
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
		tbuf[tlen] = L'\0';
		::ExtTextOutW(hdc, rc.left, ybase, ETO_OPAQUE | ETO_CLIPPED, &rcw, tbuf, tlen, NULL);
	} else {
		::ExtTextOut(hdc, rc.left, ybase, ETO_OPAQUE | ETO_CLIPPED, &rcw, s,
			Platform::Minimum(len, maxLenText), NULL);
	}
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, int ybase, const char *s, int len,
	ColourAllocated fore) {
	// Avoid drawing spaces in transparent mode
	for (int i=0;i<len;i++) {
		if (s[i] != ' ') {
			SetFont(font_);
			::SetTextColor(hdc, fore.AsLong());
			::SetBkMode(hdc, TRANSPARENT);
			RECT rcw = RectFromPRectangle(rc);
			if (unicodeMode) {
				wchar_t tbuf[MAX_US_LEN];
				int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
				tbuf[tlen] = L'\0';
				::ExtTextOutW(hdc, rc.left, ybase, 0, &rcw, tbuf, tlen, NULL);
			} else {
				::ExtTextOut(hdc, rc.left, ybase, 0, &rcw, s,
					Platform::Minimum(len,maxLenText), NULL);
			}
			::SetBkMode(hdc, OPAQUE);
			return;
		}
	}
}

int SurfaceImpl::WidthText(Font &font_, const char *s, int len) {
	SetFont(font_);
	SIZE sz={0,0};
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
		tbuf[tlen] = L'\0';
		::GetTextExtentPoint32W(hdc, tbuf, tlen, &sz);
	} else {
		::GetTextExtentPoint32(hdc, s, Platform::Minimum(len, maxLenText), &sz);
	}
	return sz.cx;
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len, int *positions) {
	SetFont(font_);
	SIZE sz={0,0};
	int fit = 0;
	if (unicodeMode) {
		wchar_t tbuf[MAX_US_LEN];
		int tlen = UCS2FromUTF8(s, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
		tbuf[tlen] = L'\0';
		int poses[MAX_US_LEN];
		fit = tlen;
		if (!::GetTextExtentExPointW(hdc, tbuf, tlen, maxWidthMeasure, &fit, poses, &sz)) {
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
		while (ui<fit) {
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
		int lastPos = 0;
		if (i > 0)
			lastPos = positions[i-1];
		while (i<len) {
			positions[i++] = lastPos;
		}
	} else {
		if (!::GetTextExtentExPoint(hdc, s, Platform::Minimum(len, maxLenText),
			maxWidthMeasure, &fit, positions, &sz)) {
			// Eeek - a NULL DC or other foolishness could cause this.
			// The least we can do is set the positions to zero!
			memset(positions, 0, len * sizeof(*positions));
		} else if (fit < len) {
			// For some reason, such as an incomplete DBCS character
			// Not all the positions are filled in so make them equal to end.
			for (int i=fit;i<len;i++)
				positions[i] = positions[fit-1];
		}
	}
}

int SurfaceImpl::WidthChar(Font &font_, char ch) {
	SetFont(font_);
	SIZE sz;
	::GetTextExtentPoint32(hdc, &ch, 1, &sz);
	return sz.cx;
}

int SurfaceImpl::Ascent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmAscent;
}

int SurfaceImpl::Descent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmDescent;
}

int SurfaceImpl::InternalLeading(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmInternalLeading;
}

int SurfaceImpl::ExternalLeading(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmExternalLeading;
}

int SurfaceImpl::Height(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmHeight;
}

int SurfaceImpl::AverageCharWidth(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return tm.tmAveCharWidth;
}

int SurfaceImpl::SetPalette(Palette *pal, bool inBackGround) {
	if (paletteOld) {
		::SelectPalette(hdc, paletteOld, TRUE);
	}
	paletteOld = 0;
	int changes = 0;
	if (pal->allowRealization) {
		paletteOld = ::SelectPalette(hdc,
			reinterpret_cast<HPALETTE>(pal->hpal), inBackGround);
		changes = ::RealizePalette(hdc);
	}
	return changes;
}

void SurfaceImpl::SetClip(PRectangle rc) {
	::IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void SurfaceImpl::FlushCachedState() {
	pen = 0;
	brush = 0;
	font = 0;
}

void SurfaceImpl::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

void SurfaceImpl::SetDBCSMode(int) {
	// No action on window as automatically handled by system.
}

Surface *Surface::Allocate() {
	return new SurfaceImpl;
}

Window::~Window() {
}

void Window::Destroy() {
	if (id)
		::DestroyWindow(reinterpret_cast<HWND>(id));
	id = 0;
}

bool Window::HasFocus() {
	return ::GetFocus() == id;
}

PRectangle Window::GetPosition() {
	RECT rc;
	::GetWindowRect(reinterpret_cast<HWND>(id), &rc);
	return PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::SetPosition(PRectangle rc) {
	::SetWindowPos(reinterpret_cast<HWND>(id),
		0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
}

void Window::SetPositionRelative(PRectangle rc, Window w) {
	LONG style = ::GetWindowLong(reinterpret_cast<HWND>(id), GWL_STYLE);
	if (style & WS_POPUP) {
		RECT rcOther;
		::GetWindowRect(reinterpret_cast<HWND>(w.GetID()), &rcOther);
		rc.Move(rcOther.left, rcOther.top);
		if (rc.left < 0) {
			rc.Move(-rc.left,0);
		}
		if (rc.top < 0) {
			rc.Move(0,-rc.top);
		}
	}
	SetPosition(rc);
}

PRectangle Window::GetClientPosition() {
	RECT rc={0,0,0,0};
	if (id)
		::GetClientRect(reinterpret_cast<HWND>(id), &rc);
	return  PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::Show(bool show) {
	if (show)
		::ShowWindow(reinterpret_cast<HWND>(id), SW_SHOWNOACTIVATE);
	else
		::ShowWindow(reinterpret_cast<HWND>(id), SW_HIDE);
}

void Window::InvalidateAll() {
	::InvalidateRect(reinterpret_cast<HWND>(id), NULL, FALSE);
}

void Window::InvalidateRectangle(PRectangle rc) {
	RECT rcw = RectFromPRectangle(rc);
	::InvalidateRect(reinterpret_cast<HWND>(id), &rcw, FALSE);
}

static LRESULT Window_SendMessage(Window *w, UINT msg, WPARAM wParam=0, LPARAM lParam=0) {
	return ::SendMessage(reinterpret_cast<HWND>(w->GetID()), msg, wParam, lParam);
}

void Window::SetFont(Font &font) {
	Window_SendMessage(this, WM_SETFONT,
		reinterpret_cast<WPARAM>(font.GetID()), 0);
}

void Window::SetCursor(Cursor curs) {
	switch (curs) {
	case cursorText:
		::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
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
	case cursorHand:
		::SetCursor(::LoadCursor(NULL,IDC_HAND));
		break;
	case cursorReverseArrow: {
			if (!hinstPlatformRes)
				hinstPlatformRes = ::GetModuleHandle("Scintilla");
			if (!hinstPlatformRes)
				hinstPlatformRes = ::GetModuleHandle("SciLexer");
			if (!hinstPlatformRes)
				hinstPlatformRes = ::GetModuleHandle(NULL);
			HCURSOR hcursor = ::LoadCursor(hinstPlatformRes, MAKEINTRESOURCE(IDC_MARGIN));
			if (hcursor)
				::SetCursor(hcursor);
			else
				::SetCursor(::LoadCursor(NULL,IDC_ARROW));
		}
		break;
	case cursorArrow:
	case cursorInvalid:	// Should not occur, but just in case.
		::SetCursor(::LoadCursor(NULL,IDC_ARROW));
		break;
	}
}

void Window::SetTitle(const char *s) {
	::SetWindowText(reinterpret_cast<HWND>(id), s);
}

class LineToType {
	int *data;
	int len;
	int maximum;
public:
	LineToType() :data(0), len(0), maximum(0) {
	}
	~LineToType() {
		Clear();
	}
	void Clear() {
		delete []data;
		data = 0;
		len = 0;
		maximum = 0;
	}
	void Add(int index, int value) {
		if (index >= maximum) {
			if (index >= len) {
				int lenNew = (index+1) * 2;
				int *dataNew = new int[lenNew];
				for (int i=0; i<maximum; i++) {
					dataNew[i] = data[i];
				}
				delete []data;
				data = dataNew;
				len = lenNew;
			}
			while (maximum < index) {
				data[maximum] = 0;
				maximum++;
			}
		}
		data[index] = value;
		if (index == maximum) {
			maximum++;
		}
	}
	int Get(int index) {
		if (index < maximum) {
			return data[index];
		} else {
			return 0;
		}
	}
};

const char ListBoxX_ClassName[] = "ListBoxX";

ListBox::ListBox() {
}

ListBox::~ListBox() {
}

class ListBoxX : public ListBox {
	int lineHeight;
	FontID fontCopy;
	XPMSet xset;
	LineToType ltt;
	HWND lb;
	bool unicodeMode;
	int desiredVisibleRows;
	unsigned int maxItemCharacters;
	unsigned int aveCharWidth;
public:
	ListBoxX() : lineHeight(10), fontCopy(0), lb(0), unicodeMode(false),
		desiredVisibleRows(5), maxItemCharacters(0), aveCharWidth(8){
	}
	virtual ~ListBoxX() {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
	}
	virtual void SetFont(Font &font);
	virtual void Create(Window &parent, int ctrlID, int lineHeight_, bool unicodeMode_);
	virtual void SetAverageCharWidth(int width);
	virtual void SetVisibleRows(int rows);
	virtual PRectangle GetDesiredRect();
	int IconWidth();
	virtual int CaretFromEdge();
	virtual void Clear();
	virtual void Append(char *s, int type = -1);
	virtual int Length();
	virtual void Select(int n);
	virtual int GetSelection();
	virtual int Find(const char *prefix);
	virtual void GetValue(int n, char *value, int len);
	virtual void RegisterImage(int type, const char *xpm_data);
	virtual void ClearRegisteredImages();
	virtual void SetDoubleClickAction(CallBackAction, void *) {
	}
	void Draw(DRAWITEMSTRUCT *pDrawItem);
	long WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static long PASCAL StaticWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

ListBox *ListBox::Allocate() {
	ListBoxX *lb = new ListBoxX();
	return lb;
}

void ListBoxX::Create(Window &parent, int ctrlID, int lineHeight_, bool unicodeMode_) {
	lineHeight = lineHeight_;
	unicodeMode = unicodeMode_;
	HINSTANCE hinstanceParent = GetWindowInstance(reinterpret_cast<HWND>(parent.GetID()));
	id = ::CreateWindowEx(
		WS_EX_WINDOWEDGE, ListBoxX_ClassName, "",
		WS_CHILD | WS_THICKFRAME,
		100,100, 150,80, reinterpret_cast<HWND>(parent.GetID()),
		reinterpret_cast<HMENU>(ctrlID),
		hinstanceParent,
		this);
	lb = ::GetWindow(reinterpret_cast<HWND>(id), GW_CHILD);
}

void ListBoxX::SetFont(Font &font) {
	LOGFONT lf;
	if (0 != ::GetObject(font.GetID(), sizeof(lf), &lf)) {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
		fontCopy = ::CreateFontIndirect(&lf);
		::SendMessage(lb, WM_SETFONT, reinterpret_cast<WPARAM>(fontCopy), 0);
	}
}

void ListBoxX::SetAverageCharWidth(int width) {
	aveCharWidth = width;
}

void ListBoxX::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

PRectangle ListBoxX::GetDesiredRect() {
	PRectangle rcDesired = GetPosition();
	int itemHeight = ::SendMessage(lb, LB_GETITEMHEIGHT, 0, 0);
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
		rcDesired.right += ::GetSystemMetrics(SM_CXVSCROLL);
	rcDesired.right += IconWidth();
	return rcDesired;
}

int ListBoxX::IconWidth() {
	return xset.GetWidth() + 2;
}

int ListBoxX::CaretFromEdge() {
	return 4 + IconWidth();
}

void ListBoxX::Clear() {
	::SendMessage(lb, LB_RESETCONTENT, 0, 0);
	maxItemCharacters = 0;
	ltt.Clear();
}

void ListBoxX::Append(char *s, int type) {
	::SendMessage(lb, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	unsigned int len = static_cast<unsigned int>(strlen(s));
	if (maxItemCharacters < len)
		maxItemCharacters = len;
	int count = ::SendMessage(lb, LB_GETCOUNT, 0, 0);
	ltt.Add(count-1, type);
}

int ListBoxX::Length() {
	return ::SendMessage(lb, LB_GETCOUNT, 0, 0);
}

void ListBoxX::Select(int n) {
	::SendMessage(lb, LB_SETCURSEL, n, 0);
}

int ListBoxX::GetSelection() {
	return ::SendMessage(lb, LB_GETCURSEL, 0, 0);
}

int ListBoxX::Find(const char *prefix) {
	return ::SendMessage(lb, LB_FINDSTRING, static_cast<WPARAM>(-1),
		reinterpret_cast<LPARAM>(prefix));
}

void ListBoxX::GetValue(int n, char *value, int len) {
	int lenText = ::SendMessage(lb, LB_GETTEXTLEN, n, 0);
	if ((len > 0) && (lenText > 0)){
		char *text = new char[len+1];
		if (text) {
			::SendMessage(lb, LB_GETTEXT, n, reinterpret_cast<LPARAM>(text));
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

void ListBoxX::RegisterImage(int type, const char *xpm_data) {
	xset.Add(type, xpm_data);
}

void ListBoxX::ClearRegisteredImages() {
	xset.Clear();
}

void ListBoxX::Draw(DRAWITEMSTRUCT *pDrawItem) {
	if ((pDrawItem->itemAction == ODA_SELECT) || (pDrawItem->itemAction == ODA_DRAWENTIRE)) {
		char text[1000];
		int len = ::SendMessage(pDrawItem->hwndItem, LB_GETTEXTLEN, pDrawItem->itemID, 0);
		if (len < static_cast<int>(sizeof(text))) {
			::SendMessage(pDrawItem->hwndItem, LB_GETTEXT, pDrawItem->itemID, reinterpret_cast<LPARAM>(text));
		} else {
			len = 0;
			text[0] = '\0';
		}
		int pixId = ltt.Get(pDrawItem->itemID);
		XPM *pxpm = xset.Get(pixId);
		if (pDrawItem->itemState & ODS_SELECTED) {
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		} else {
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOW));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
		}
		int widthPix = xset.GetWidth() + 2;
		if (unicodeMode) {
			wchar_t tbuf[MAX_US_LEN];
			int tlen = UCS2FromUTF8(text, len, tbuf, sizeof(tbuf)/sizeof(wchar_t)-1);
			tbuf[tlen] = L'\0';
			::ExtTextOutW(pDrawItem->hDC, pDrawItem->rcItem.left+widthPix+1, pDrawItem->rcItem.top,
				ETO_OPAQUE, &(pDrawItem->rcItem), tbuf, tlen, NULL);
		} else {
			::ExtTextOut(pDrawItem->hDC, pDrawItem->rcItem.left+widthPix+1, pDrawItem->rcItem.top,
				ETO_OPAQUE, &(pDrawItem->rcItem), text,
				len, NULL);
		}
		if (pxpm) {
			Surface *surfaceItem = Surface::Allocate();
			if (surfaceItem) {
				surfaceItem->Init(pDrawItem->hDC, pDrawItem->hwndItem);
				//surfaceItem->SetUnicodeMode(unicodeMode);
				//surfaceItem->SetDBCSMode(codePage);
				int left = pDrawItem->rcItem.left;
				PRectangle rc(left + 1, pDrawItem->rcItem.top,
					left + 1 + widthPix, pDrawItem->rcItem.bottom);
				pxpm->Draw(surfaceItem, rc);
				delete surfaceItem;
				::SetTextAlign(pDrawItem->hDC, TA_TOP);
			}
		}
        if (pDrawItem->itemState & ODS_SELECTED) {
			::DrawFocusRect(pDrawItem->hDC, &(pDrawItem->rcItem));
        }
	}
}

long ListBoxX::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE: {
		HWND parent = ::GetParent(hWnd);
		HINSTANCE hinstanceParent = GetWindowInstance(parent);
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
		::CreateWindowEx(
			WS_EX_WINDOWEDGE, "listbox", "",
			WS_CHILD | WS_VSCROLL | WS_VISIBLE |
			LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY,
			0, 0, 150,80, hWnd,
			pCreate->hMenu,
			hinstanceParent,
			0);
		}
		break;
	case WM_SIZE: {
			HWND lb = ::GetWindow(hWnd, GW_CHILD);
			::SetWindowPos(lb, 0, 0,0, LOWORD(lParam), HIWORD(lParam), 0);
		}
		break;
	case WM_PAINT: {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
		}
		break;
	case WM_COMMAND: {
			HWND parent = ::GetParent(hWnd);
			::SendMessage(parent, iMessage, wParam, lParam);
		}
		break;

	case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *pMeasureItem = reinterpret_cast<MEASUREITEMSTRUCT *>(lParam);
			pMeasureItem->itemHeight = lineHeight;
			if (pMeasureItem->itemHeight < static_cast<unsigned int>(xset.GetHeight())) {
				pMeasureItem->itemHeight = xset.GetHeight();
			}
		}
		break;
	case WM_DRAWITEM:
		Draw(reinterpret_cast<DRAWITEMSTRUCT *>(lParam));
		break;
	case WM_DESTROY:
		::SetWindowLong(hWnd, 0, 0);
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	default:
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}

	return 0;
}

long PASCAL ListBoxX::StaticWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	if (iMessage == WM_CREATE) {
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
		SetWindowPointer(hWnd, pCreate->lpCreateParams);
	}
	// Find C++ object associated with window.
	ListBoxX *lbx = reinterpret_cast<ListBoxX *>(PointerFromWindow(hWnd));
	if (lbx) {
		return lbx->WndProc(hWnd, iMessage, wParam, lParam);
	} else {
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
}

static bool ListBoxX_Register() {
	WNDCLASSEX wndclassc;
	wndclassc.cbSize = sizeof(wndclassc);
	wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclassc.cbClsExtra = 0;
	wndclassc.cbWndExtra = sizeof(ListBoxX *);
	wndclassc.hInstance = hinstPlatformRes;
	wndclassc.hIcon = NULL;
	wndclassc.hbrBackground = NULL;
	wndclassc.lpszMenuName = NULL;
	wndclassc.lpfnWndProc = ListBoxX::StaticWndProc;
	wndclassc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wndclassc.lpszClassName = ListBoxX_ClassName;
	wndclassc.hIconSm = 0;

	return ::RegisterClassEx(&wndclassc) != 0;
}

bool ListBoxX_Unregister() {
	return ::UnregisterClass(ListBoxX_ClassName, hinstPlatformRes) != 0;
}

Menu::Menu() : id(0) {
}

void Menu::CreatePopUp() {
	Destroy();
	id = ::CreatePopupMenu();
}

void Menu::Destroy() {
	if (id)
		::DestroyMenu(reinterpret_cast<HMENU>(id));
	id = 0;
}

void Menu::Show(Point pt, Window &w) {
	::TrackPopupMenu(reinterpret_cast<HMENU>(id),
		0, pt.x - 4, pt.y, 0,
		reinterpret_cast<HWND>(w.GetID()), NULL);
	Destroy();
}

static bool initialisedET = false;
static bool usePerformanceCounter = false;
static LARGE_INTEGER frequency;

ElapsedTime::ElapsedTime() {
	if (!initialisedET) {
		usePerformanceCounter = ::QueryPerformanceFrequency(&frequency) != 0;
		initialisedET = true;
	}
	if (usePerformanceCounter) {
		LARGE_INTEGER timeVal;
		::QueryPerformanceCounter(&timeVal);
		bigBit = timeVal.HighPart;
		littleBit = timeVal.LowPart;
	} else {
		bigBit = clock();
	}
}

double ElapsedTime::Duration(bool reset) {
	double result;
	long endBigBit;
	long endLittleBit;

	if (usePerformanceCounter) {
		LARGE_INTEGER lEnd;
		::QueryPerformanceCounter(&lEnd);
		endBigBit = lEnd.HighPart;
		endLittleBit = lEnd.LowPart;
		LARGE_INTEGER lBegin;
		lBegin.HighPart = bigBit;
		lBegin.LowPart = littleBit;
		double elapsed = lEnd.QuadPart - lBegin.QuadPart;
		result = elapsed / static_cast<double>(frequency.QuadPart);
	} else {
		endBigBit = clock();
		endLittleBit = 0;
		double elapsed = endBigBit - bigBit;
		result = elapsed / CLOCKS_PER_SEC;
	}
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}

class DynamicLibraryImpl : public DynamicLibrary {
protected:
	HMODULE h;
public:
	DynamicLibraryImpl(const char *modulePath) {
		h = ::LoadLibrary(modulePath);
	}

	virtual ~DynamicLibraryImpl() {
		if (h != NULL)
			::FreeLibrary(h);
	}

	// Use GetProcAddress to get a pointer to the relevant function.
	virtual Function FindFunction(const char *name) {
		if (h != NULL) {
			return static_cast<Function>(
				(void *)(::GetProcAddress(h, name)));
		} else
			return NULL;
	}

	virtual bool IsValid() {
		return h != NULL;
	}
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
	return static_cast<DynamicLibrary *>( new DynamicLibraryImpl(modulePath) );
}

ColourDesired Platform::Chrome() {
	return ::GetSysColor(COLOR_3DFACE);
}

ColourDesired Platform::ChromeHighlight() {
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

bool Platform::MouseButtonBounce() {
	return false;
}

void Platform::DebugDisplay(const char *s) {
	::OutputDebugString(s);
}

bool Platform::IsKeyDown(int key) {
	return (::GetKeyState(key) & 0x80000000) != 0;
}

long Platform::SendScintilla(WindowID w, unsigned int msg, unsigned long wParam, long lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(w), msg, wParam, lParam);
}

long Platform::SendScintillaPointer(WindowID w, unsigned int msg, unsigned long wParam, void *lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(w), msg, wParam,
		reinterpret_cast<LPARAM>(lParam));
}

bool Platform::IsDBCSLeadByte(int codePage, char ch) {
	return ::IsDBCSLeadByteEx(codePage, ch) != 0;
}

int Platform::DBCSCharLength(int codePage, const char *s) {
	return (::IsDBCSLeadByteEx(codePage, s[0]) != 0) ? 2 : 1;
}

int Platform::DBCSCharMaxLength() {
	return 2;
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

void Platform_Initialise(void *hInstance) {
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO),0,0,0,0,""};
	::GetVersionEx(&osv);
	onNT = osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
	::InitializeCriticalSection(&crPlatformLock);
	hinstPlatformRes = reinterpret_cast<HINSTANCE>(hInstance);
	ListBoxX_Register();
}

void Platform_Finalise() {
	ListBoxX_Unregister();
	::DeleteCriticalSection(&crPlatformLock);
}
