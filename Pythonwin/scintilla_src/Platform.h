// Scintilla source code edit control
// Platform.h - interface to platform facilities
// Implemented in PlatGTK.cc for GTK+/Linux and PlatWin.cc for Windows
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#ifdef GTK
#define PLAT_GTK 1
#else
#define PLAT_WIN 1
#endif

#ifdef GTK

#define COLORREF GdkColor
#define POINT GdkPoint
#define HFONT GdkFont*
#define HWND GtkWidget*

struct RECT {
	int left;
	int top;
	int right;
	int bottom;
};

#endif

// A surface abstracts a place to draw
class Surface {
public:
#ifdef GTK
	GtkWidget *hwnd;
	GdkDrawable *drawable;
	GdkGC *gc;
	int x;
	int y;
#else
	HDC hdc;
	HPEN pen;
	HPEN penOld;
	HBRUSH brush;
	HBRUSH brushOld;
	HFONT font;
	HFONT fontOld;
	HWND hwnd;
#endif

public:
#ifdef GTK
	void Init(GtkWidget *hwnd_=0, GdkDrawable *drawable_=0, GdkGC *gc_=0);
#else
	void Init(HDC hdc_);
	void InitOnWindow(HWND hwnd_);
#endif

	Surface();
	~Surface();
	void PenColor(COLORREF fore);
#ifndef GTK
	void BrushColor(COLORREF back);
	void SetFont(HFONT font_);
#endif
	void MoveTo(int x_, int y_);
	void LineTo(int x_, int y_);
	void Polygon(POINT *pts, int npts, COLORREF fore, COLORREF back);
	void Rectangle(RECT rc, COLORREF fore, COLORREF back);
	void FillRectangle(RECT rc, COLORREF back);
	void RoundedRectangle(RECT rc, COLORREF fore, COLORREF back);
	void Ellipse(RECT rc, COLORREF fore, COLORREF back);

	void DrawText(RECT rc, HFONT font_, int ybase, char *s, int len, COLORREF fore, COLORREF back);
	int WidthText(HFONT font_, char *s, int len);
	int WidthChar(HFONT font_, char ch);
};
