// Scintilla source code edit control
// PlatfGDK.cc - implementation of platform facilities on GTK+/Linux
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#include <gtk/gtk.h>

#include "Platform.h"

Surface::Surface() {
	hwnd = 0;
	drawable = 0;
	gc = 0;
	x = 0;
	y = 0;
}

Surface::~Surface() {
	//gdk_gc_unref(gc);
	gc = 0;
} 

void Surface::Init(GtkWidget *hwnd_, GdkDrawable *drawable_, GdkGC *gc_) {
	hwnd = hwnd_;
	drawable = drawable_;
	//if (gc_)
		gc = gc_;
	//else 
	//	gc = gdk_gc_new(hwnd->window);
}

void Surface::PenColor(COLORREF fore) {
	gdk_gc_set_foreground(gc, &fore);
}

void Surface::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

void Surface::LineTo(int x_, int y_) {
	gdk_draw_line(drawable, gc,
              	x, y,
              	x_, y_);
	x = x_;
	y = y_;
}

void Surface::Polygon(POINT *pts, int npts, COLORREF fore,
                      COLORREF back) {
	PenColor(back);
	gdk_draw_polygon(drawable, gc, 1, pts, npts);
	PenColor(fore);
	gdk_draw_polygon(drawable, gc, 0, pts, npts);
}

void Surface::Rectangle(RECT rc, COLORREF fore, COLORREF back) {
	PenColor(back);
	gdk_draw_rectangle(drawable, gc, 1,
                   	rc.left, rc.top,
                   	rc.right - rc.left + 1, rc.bottom - rc.top + 1);
	PenColor(fore);
	gdk_draw_rectangle(drawable, gc, 0,
                   	rc.left, rc.top,
                   	rc.right - rc.left + 1, rc.bottom - rc.top + 1);
}

void Surface::FillRectangle(RECT rc, COLORREF back) {
	// GTK+ rectangles include their lower and right edges
	rc.bottom--;
	rc.right--;
	PenColor(back);
	gdk_draw_rectangle(drawable, gc, 1,
                   	rc.left, rc.top,
                   	rc.right - rc.left + 1, rc.bottom - rc.top + 1);
}

void Surface::RoundedRectangle(RECT rc, COLORREF fore, COLORREF back) {
	if (((rc.right - rc.left) > 4) && ((rc.bottom - rc.top) > 4)) {
		// Approximate a round rect with some cut off corners
		POINT pts[] = {
    		{rc.left + 2, rc.top},
    		{rc.right - 2, rc.top},
    		{rc.right, rc.top + 2},
    		{rc.right, rc.bottom - 2},
    		{rc.right - 2, rc.bottom},
    		{rc.left + 2, rc.bottom},
    		{rc.left, rc.bottom - 2},
    		{rc.left, rc.top + 2},
		};
		Polygon(pts, sizeof(pts) / sizeof(pts[0]), fore, back);
	} else {
		Rectangle(rc, fore, back);
	}
}

void Surface::Ellipse(RECT rc, COLORREF fore, COLORREF back) {
	PenColor(back);
	gdk_draw_arc(drawable, gc, 1,
             	rc.left, rc.top,
             	rc.right - rc.left, rc.bottom - rc.top,
             	0, 32767);
	PenColor(fore);
	gdk_draw_arc(drawable, gc, 0,
             	rc.left, rc.top,
             	rc.right - rc.left, rc.bottom - rc.top,
             	0, 32767);
}

void Surface::DrawText(RECT rc, HFONT font, int ybase, char *s, int len, COLORREF fore, COLORREF back) {
	FillRectangle(rc, back);
	PenColor(fore);
	gdk_draw_text(drawable, font, gc, rc.left, ybase, s, len);
}

int Surface::WidthText(HFONT font, char *s, int len) {
	return gdk_text_width(font, s, len);
}

int Surface::WidthChar(HFONT font, char ch) {
	return gdk_char_width(font, ch);
}

