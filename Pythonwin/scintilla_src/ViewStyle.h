// Scintilla source code edit control
// ViewStyle.h - store information on how the document is to be viewed
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef VIEWSTYLE_H
#define VIEWSTYLE_H

class ViewStyle {
public:
	Style styles[STYLE_MAX + 1];
	LineMarker markers[MARKER_MAX + 1];
	Indicator indicators[INDIC_MAX + 1];
	int lineHeight;
	unsigned int maxAscent;
	unsigned int maxDescent;
	unsigned int aveCharWidth;
	unsigned int spaceWidth;
	//ColourPair foreground;
	//ColourPair background;
	//int size;
	//char fontName[100];
	//bool bold;
	//bool italic;
	bool selforeset;
	ColourPair selforeground;
	bool selbackset;
	ColourPair selbackground;
	ColourPair selbar;
	ColourPair selbarlight;
	int selMarginWidth;
	int lineNumberWidth;
	int fixedColumnWidth;
	int zoomLevel;
	bool viewWhitespace;
        bool viewEOL;
	bool showMarkedLines;
	ColourPair caretcolour;
	
	ViewStyle();
	ViewStyle(const ViewStyle &source);
	~ViewStyle();
	void Init();
	void RefreshColourPalette(Palette &pal, bool want);
	void Refresh(Surface &surface);
	void ResetDefaultStyle();
	void ClearStyles();
};

#endif
