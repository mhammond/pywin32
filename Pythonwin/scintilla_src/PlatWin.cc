// Scintilla source code edit control
// PlatfGDK.cc - implementation of platform facilities on GTK+/Linux
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#include <windows.h>

#include "Platform.h"

Surface::Surface() {
	hdc = 0;
	pen = 0;
	penOld = 0;
	brush = 0;
	brush;
	brushOld = 0;
	font = 0;
	fontOld = 0;
	hwnd = 0;
}

Surface::~Surface() {
	if (pen) {
		SelectObject(hdc, penOld);
		DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	if (brush) {
		SelectObject(hdc, brushOld);
		DeleteObject(brush);
		brush = 0;
		brushOld = 0;
	}
	if (fontOld) {
		SelectObject(hdc, fontOld);
		fontOld = 0;
	}
	if (hwnd) {
		// Surface allocated DC on window so must release it
		ReleaseDC(hwnd,hdc);
		hdc = 0;
		hwnd = 0;
	}
}

void Surface::Init(HDC hdc_) {
	hdc = hdc_;
	pen = 0;
	penOld = 0;
	brush = 0;
	brushOld = 0;
	font = 0;
	fontOld = 0;
	hwnd = 0;
}

void Surface::InitOnWindow(HWND hwnd_) {
	pen = 0;
	penOld = 0;
	brush = 0;
	brushOld = 0;
	font = 0;
	fontOld = 0;
	hwnd = hwnd_;
	hdc = GetDC(hwnd);
}

void Surface::PenColor(COLORREF fore) {
	if (pen) {
		SelectObject(hdc, penOld);
		DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	pen = CreatePen(0,1,fore);
	penOld = static_cast<HPEN>(SelectObject(hdc, pen));
}

void Surface::BrushColor(COLORREF back) {
	if (brush) {
		SelectObject(hdc, brushOld);
		DeleteObject(brush);
		brush = 0;
		brushOld = 0;
	}
	// Only ever want pure, non-dithered brushes
	COLORREF colourNearest = GetNearestColor(hdc, back);
	brush = CreateSolidBrush(colourNearest);
	brushOld = static_cast<HBRUSH>(SelectObject(hdc, brush));
}

void Surface::SetFont(HFONT font_) {
	if (font_ != font) {
		if (fontOld) {
			SelectObject(hdc, fontOld);
			fontOld = 0;
		}
		fontOld = static_cast<HFONT>(SelectObject(hdc, font_));
		font = font_;
	}
}

void Surface::MoveTo(int x_, int y_) {
	MoveToEx(hdc, x_, y_, 0);
}

void Surface::LineTo(int x_, int y_) {
	::LineTo(hdc, x_, y_);
}

void Surface::Polygon(POINT *pts, int npts, COLORREF fore,
                      COLORREF back) {
	PenColor(fore);
	BrushColor(back);
	::Polygon(hdc, pts, npts);
}

void Surface::Rectangle(RECT rc, COLORREF fore, COLORREF back) {
	PenColor(fore);
	BrushColor(back);
	::Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void Surface::FillRectangle(RECT rc, COLORREF back) {
	// Using ExtTextOut rather than a FillRect ensures that no dithering occurs.
	// There is no need to allocate a brush either.
	SetBkColor(hdc, back);
	ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE, &rc, "", 0, NULL);
}

void Surface::RoundedRectangle(RECT rc, COLORREF fore, COLORREF back) {
	PenColor(fore);
	BrushColor(back);
	RoundRect(hdc,
          	rc.left + 1, rc.top,
          	rc.right - 1, rc.bottom,
          	8, 8 );
}

void Surface::Ellipse(RECT rc, COLORREF fore, COLORREF back) {
	PenColor(fore);
	BrushColor(back);
	::Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

void Surface::DrawText(RECT rc, HFONT font_, int ybase, char *s, int len, COLORREF fore, COLORREF back) {
	SetFont(font_);
	SetTextColor(hdc, fore);
	SetBkColor(hdc, back);
	ExtTextOut(hdc, rc.left, ybase, ETO_OPAQUE, &rc, s, len, NULL);
}

int Surface::WidthText(HFONT font_, char *s, int len) {
	SetFont(font_);
	SIZE sz;
	GetTextExtentPoint32(hdc, s, len, &sz);
	return sz.cx;
}

int Surface::WidthChar(HFONT font_, char ch) {
	SetFont(font_);
	SIZE sz;
	GetTextExtentPoint32(hdc, &ch, 1, &sz);
	return sz.cx;
}

