// Scintilla source code edit control
// ViewStyle.cxx - store information on how the document is to be viewed
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <string.h>

#include "Platform.h"

#include "Scintilla.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"

ViewStyle::ViewStyle() {
	Init();
}

ViewStyle::ViewStyle(const ViewStyle &source) {
	Init();
	for (int sty=0;sty<=STYLE_MAX;sty++) {
		styles[sty] = source.styles[sty];
	}
	for (int mrk=0;mrk<=MARKER_MAX;mrk++) {
		markers[mrk] = source.markers[mrk];
	}
	for (int ind=0;ind<=INDIC_MAX;ind++) {
		indicators[ind] = source.indicators[ind];
	}
	
	selforeset = source.selforeset;
	selforeground.desired = source.selforeground.desired;
	selbackset = source.selbackset;
	selbackground.desired = source.selbackground.desired;
	selbar.desired = source.selbar.desired;
	selbarlight.desired = source.selbarlight.desired;
	caretcolour.desired = source.caretcolour.desired;
	
	selMarginWidth = source.selMarginWidth;
	lineNumberWidth = source.lineNumberWidth;
	fixedColumnWidth = selMarginWidth + lineNumberWidth;
	zoomLevel = source.zoomLevel;
	viewWhitespace = source.viewWhitespace;
	viewEOL = source.viewEOL;
	showMarkedLines = source.showMarkedLines;		
}

ViewStyle::~ViewStyle() {
}

void ViewStyle::Init() {
	indicators[0].style = INDIC_SQUIGGLE;
	indicators[0].fore = Colour(0, 0x7f, 0);
	indicators[1].style = INDIC_TT;
	indicators[1].fore = Colour(0, 0, 0xff);
	indicators[2].style = INDIC_PLAIN;
	indicators[2].fore = Colour(0xff, 0, 0);

	lineHeight = 1;
	maxAscent = 1;
	maxDescent = 1;
	aveCharWidth = 8;
	spaceWidth = 8;

	selforeset = false;
	selforeground.desired = Colour(0xff, 0, 0);
	selbackset = true;
	selbackground.desired = Colour(0xc0, 0xc0, 0xc0);
	selbar.desired = Platform::Chrome();
	selbarlight.desired = Platform::ChromeHighlight();
	styles[STYLE_LINENUMBER].fore.desired = Colour(0, 0, 0);
	styles[STYLE_LINENUMBER].back.desired = Platform::Chrome();
	//caretcolour.desired = Colour(0xff, 0, 0);
	caretcolour.desired = Colour(0, 0, 0);
	
	selMarginWidth = 20;
	lineNumberWidth = 0;
	fixedColumnWidth = selMarginWidth + lineNumberWidth;
	zoomLevel = 0;
	viewWhitespace = false;
	viewEOL = false;
	showMarkedLines = true;
}

void ViewStyle::RefreshColourPalette(Palette &pal, bool want) {
	unsigned int i;
	for (i=0;i<(sizeof(styles)/sizeof(styles[0]));i++) {
		pal.WantFind(styles[i].fore, want);
		pal.WantFind(styles[i].back, want);
	}
	for (i=0;i<(sizeof(indicators)/sizeof(indicators[0]));i++) {
		pal.WantFind(indicators[i].fore, want);
	}
	for (i=0;i<(sizeof(markers)/sizeof(markers[0]));i++) {
		pal.WantFind(markers[i].fore, want);
		pal.WantFind(markers[i].back, want);
	}
	pal.WantFind(selforeground, want);
	pal.WantFind(selbackground, want);
	pal.WantFind(selbar, want);
	pal.WantFind(selbarlight, want);
	pal.WantFind(caretcolour, want);
}

void ViewStyle::Refresh(Surface &surface) {
	selbar.desired = Platform::Chrome();
	selbarlight.desired = Platform::ChromeHighlight();
	maxAscent = 1;
	maxDescent = 1;
	for (unsigned int i=0;i<(sizeof(styles)/sizeof(styles[0]));i++) {
		styles[i].Realise(surface, zoomLevel);
		if (maxAscent < styles[i].ascent)
			maxAscent = styles[i].ascent;
		if (maxDescent < styles[i].descent)
			maxDescent = styles[i].descent;
	}
	
	lineHeight = maxAscent + maxDescent;
	aveCharWidth = styles[0].aveCharWidth;
	spaceWidth = styles[0].spaceWidth;

	fixedColumnWidth = selMarginWidth + lineNumberWidth;
}

void ViewStyle::ResetDefaultStyle() {
	styles[STYLE_DEFAULT].Clear();
}

void ViewStyle::ClearStyles() {
	// Reset all styles to be like the default style
	for (int i=0; i<=STYLE_MAX; i++) {
		if (i != STYLE_DEFAULT) {
			styles[i].Clear(
				styles[STYLE_DEFAULT].fore.desired, 
				styles[STYLE_DEFAULT].back.desired, 
				styles[STYLE_DEFAULT].size, 
				styles[STYLE_DEFAULT].fontName, 
				styles[STYLE_DEFAULT].bold, 
				styles[STYLE_DEFAULT].italic);
		}
	}
	styles[STYLE_LINENUMBER].back.desired = Platform::Chrome();
}

