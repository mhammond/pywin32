// Scintilla source code edit control
// Editor.cxx - main code for the edit control
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Platform.h"

#include "Scintilla.h"
#include "CellBuffer.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "Document.h"
#include "Editor.h"

Caret::Caret() :
active(true), on(true), period(500) {}

Timer::Timer() :
ticking(false), ticksToWait(0), tickerID(0) {}

Editor::Editor() {
	ctrlID = 0;

	stylesValid = false;

	hideSelection = false;
	inOverstrike = false;

	bufferedDraw = true;

	lastClickTime = 0;
	ptMouseLast.x = 0;
	ptMouseLast.y = 0;
	firstExpose = true;
	inDragDrop = false;
	dropWentOutside = false;
	posDrag = invalidPosition;
	posDrop = invalidPosition;
	selectionType = selChar;

	lastXChosen = 0;
	lineAnchor = 0;
	originalAnchorPos = 0;

	dragChars = 0;
	lenDrag = 0;

	ucWheelScrollLines = 0;
	cWheelDelta = 0;   //wheel delta from roll

	xOffset = 0;
	currentPos = 0;
	anchor = 0;

	topLine = 0;

        braces[0]=invalidPosition;
        braces[1]=invalidPosition;
	bracesMatchStyle = STYLE_BRACEBAD;
	paintState = notPainting;
	
	doc.AddWatcher(this, 0);
}

Editor::~Editor() {
	DropGraphics();

	delete []dragChars;
	dragChars = 0;
	lenDrag = 0;
}

void Editor::Finalise() {}


void Editor::DropGraphics() {
	pixmapLine.Release();
	pixmapSelMargin.Release();
	pixmapSelPattern.Release();
}

void Editor::InvalidateStyleData() {
	stylesValid = false;
	palette.Release();
	DropGraphics();
}

void Editor::InvalidateStyleRedraw() {
	InvalidateStyleData();
	Redraw();
}

void Editor::RefreshColourPalette(Palette &pal, bool want) {
	vs.RefreshColourPalette(pal, want);
}

void Editor::RefreshStyleData() {
	if (!stylesValid) {
		stylesValid = true;
		Surface surface;
		surface.Init();
		vs.Refresh(surface);
		RefreshColourPalette(palette, true);
		palette.Allocate(wMain);
		RefreshColourPalette(palette, false);
		SetScrollBars();
	}
}

PRectangle Editor::GetClientRectangle() {
	return wDraw.GetClientPosition();
}

PRectangle Editor::GetTextRectangle() {
	PRectangle rc = GetClientRectangle();
	rc.left += vs.fixedColumnWidth;
	return rc;
}

int Editor::LinesOnScreen() {
	PRectangle rcClient = GetClientRectangle();
	int htClient = rcClient.bottom - rcClient.top;
	//Platform::DebugPrintf("lines on screen = %d\n", htClient / lineHeight + 1);
	return htClient / vs.lineHeight;
}

int Editor::LinesToScroll() {
	int retVal = LinesOnScreen() - 1;
	if (retVal < 1)
		return 1;
	else
		return retVal;
}

int Editor::MaxScrollPos() {
	//Platform::DebugPrintf("Lines %d screen = %d maxScroll = %d\n",
	//LinesTotal(), LinesOnScreen(), LinesTotal() - LinesOnScreen() + 1);
	//int retVal = LinesTotal() - LinesOnScreen() + 1;
	int retVal = doc.LinesTotal() - LinesOnScreen();
	if (retVal < 0)
		return 0;
	else
		return retVal;
}

bool IsControlCharacter(char ch) {
	// iscntrl returns true for lots of chars > 127 which are displayable
	return ch >= 0 && ch < ' ';
}

const char *ControlCharacterString(char ch) {
	const char *reps[] = {
	    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
	    "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
	    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
	    "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US"
	};
	if (ch < (sizeof(reps) / sizeof(reps[0]))) {
		return reps[ch];
	} else {
		return "BAD";
	}
}

Point Editor::LocationFromPosition(unsigned int pos) {
	RefreshStyleData();
	Point pt;
	int line = doc.LineFromPosition(pos);
	//Platform::DebugPrintf("line=%d\n", line);
	Surface surface;
	surface.Init();
	pt.y = (line - topLine) * vs.lineHeight;  	// + half a lineheight?
	unsigned int posLineStart = doc.LineStart(line);
	if ((pos - posLineStart) > LineLayout::maxLineLength) {
		// very long line so put x at arbitrary large position
		pt.x = 30000 + vs.fixedColumnWidth - xOffset;
	} else {
		LineLayout ll;
		LayoutLine(line, &surface, vs, ll);
		pt.x = ll.positions[pos - posLineStart] + vs.fixedColumnWidth - xOffset;
	}
	return pt;
}

int Editor::LineFromLocation(Point pt) {
	return pt.y / vs.lineHeight + topLine;
}

int Editor::PositionFromLocation(Point pt) {
	pt.x = pt.x - vs.fixedColumnWidth + xOffset;
	int line = pt.y / vs.lineHeight + topLine;
	if (pt.y < 0)	// Division rounds towards 0
		line = (pt.y - (vs.lineHeight - 1)) / vs.lineHeight + topLine;
	if (line < 0)
		return 0;
	if (line >= doc.LinesTotal())
		return doc.Length();
	//Platform::DebugPrintf("Position of (%d,%d) line = %d top=%d\n", pt.x, pt.y, line, topLine);
	Surface surface;
	surface.Init();
	unsigned int posLineStart = doc.LineStart(line);

	LineLayout ll;
	LayoutLine(line, &surface, vs, ll);
	for (int i = 0; i < ll.numCharsInLine; i++) {
		if (pt.x < ((ll.positions[i] + ll.positions[i + 1]) / 2) || 
			ll.chars[i] == '\r' || ll.chars[i] == '\n') {
			return i + posLineStart;
		}
	}

	return ll.numCharsInLine + posLineStart;
}

void Editor::RedrawRect(PRectangle rc) {
	//Platform::DebugPrintf("Redraw %d %d - %d %d\n", rc.left, rc.top, rc.right, rc.bottom);
	wDraw.InvalidateRectangle(rc);
}

void Editor::Redraw() {
	//Platform::DebugPrintf("Redraw all\n");
	wDraw.InvalidateAll();
}

void Editor::RedrawSelMargin() {
	if (vs.fixedColumnWidth > 0) {
		PRectangle rcSelMargin = GetClientRectangle();
		rcSelMargin.right = vs.fixedColumnWidth;
		wDraw.InvalidateRectangle(rcSelMargin);
	} else {
		Redraw();
	}
}

PRectangle Editor::RectangleFromRange(int start, int end) {
	int minPos = start;
	if (minPos > end)
		minPos = end;
	int maxPos = start;
	if (maxPos < end)
		maxPos = end;
	int minLine = doc.LineFromPosition(minPos);
	int maxLine = doc.LineFromPosition(maxPos);
	PRectangle rcClient = GetTextRectangle();
	PRectangle rc;
	rc.left = vs.fixedColumnWidth;
	rc.top = (minLine - topLine) * vs.lineHeight;
	rc.right = rcClient.right;
	rc.bottom = (maxLine - topLine + 1) * vs.lineHeight;
	// Ensure PRectangle is within 16 bit space
	rc.top = Platform::Clamp(rc.top, -32000, 32000);
	rc.bottom = Platform::Clamp(rc.bottom, -32000, 32000);

	return rc;
}

void Editor::InvalidateRange(int start, int end) {
	RedrawRect(RectangleFromRange(start, end));
}

int Editor::CurrentPosition() {
	return currentPos;
}

bool Editor::SelectionEmpty() {
	return anchor == currentPos;
}

int Editor::SelectionStart() {
	return Platform::Minimum(currentPos, anchor);
}

int Editor::SelectionEnd() {
	return Platform::Maximum(currentPos, anchor);
}

void Editor::SetSelection(int currentPos_, int anchor_) {
	currentPos_ = doc.ClampPositionIntoDocument(currentPos_);
	anchor_ = doc.ClampPositionIntoDocument(anchor_);
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
		if (lastAffected < (currentPos_ + 1))	// +1 ensures caret repainted
			lastAffected = (currentPos_ + 1);
		currentPos = currentPos_;
		anchor = anchor_;
		InvalidateRange(firstAffected, lastAffected);
	}
	ClaimSelection();
}

void Editor::SetSelection(int currentPos_) {
	currentPos_ = doc.ClampPositionIntoDocument(currentPos_);
	if (currentPos != currentPos_) {
		int firstAffected = anchor;
		if (firstAffected > currentPos)
			firstAffected = currentPos;
		if (firstAffected > currentPos_)
			firstAffected = currentPos_;
		int lastAffected = anchor;
		if (lastAffected < currentPos)
			lastAffected = currentPos;
		if (lastAffected < (currentPos_ + 1))	// +1 ensures caret repainted
			lastAffected = (currentPos_ + 1);
		currentPos = currentPos_;
		InvalidateRange(firstAffected, lastAffected);
	}
	ClaimSelection();
}

void Editor::SetEmptySelection(int currentPos_) {
	SetSelection(currentPos_, currentPos_);
}

void Editor::SetPosition(int pos, bool shift) {
	int oldPos = currentPos;
	currentPos = doc.ClampPositionIntoDocument(pos);
	currentPos = doc.MovePositionOutsideChar(currentPos, oldPos - currentPos);
	if (!shift)
		anchor = currentPos;
	EnsureCaretVisible();
	ClaimSelection();
}

int Editor::MovePositionTo(int newPos, bool extend) {
	int delta = newPos - currentPos;
	newPos = doc.ClampPositionIntoDocument(newPos);
	newPos = doc.MovePositionOutsideChar(newPos, delta);
	if (extend) {
		SetSelection(newPos);
	} else {
		SetEmptySelection(newPos);
	}
	EnsureCaretVisible();
	ShowCaretAtCurrentPosition();
	return 0;
}

// Choose the x position that the caret will try to stick to as it is moves up and down
void Editor::SetLastXChosen() {
	Point pt = LocationFromPosition(currentPos);
	lastXChosen = pt.x;
}

void Editor::ScrollTo(int line) {
	int topLineNew = Platform::Clamp(line, 0, MaxScrollPos());
	if (topLineNew != topLine) {
		// Try to optimise small scrolls
		int linesToMove = topLine - topLineNew;
		topLine = topLineNew;
		ShowCaretAtCurrentPosition();
		if (abs(linesToMove) <= 10) {
			ScrollText(linesToMove);
		} else {
			Redraw();
		}
		SetVerticalScrollPos();
	}
}

void Editor::ScrollText(int linesToMove) {
	//Platform::DebugPrintf("Editor::ScrollText %d\n", linesToMove);
	Redraw();
}

void Editor::HorizontalScrollTo(int xPos) {
	//Platform::DebugPrintf("HorizontalScroll %d\n", xPos);
	xOffset = xPos;
	if (xOffset < 0)
		xOffset = 0;
	SetHorizontalScrollPos();
	Redraw();
}

void Editor::EnsureCaretVisible() {
	//Platform::DebugPrintf("EnsureCaretVisible %d\n", xOffset);
	PRectangle rcClient = GetTextRectangle();
	int posCaret = currentPos;
	if (posDrag >= 0)
		posCaret = posDrag;
	Point pt = LocationFromPosition(posCaret);
	Point ptBottomCaret = pt;
	int lineCaret = doc.LineFromPosition(posCaret);
	ptBottomCaret.y += vs.lineHeight - 1;
	if (!rcClient.Contains(pt) || !rcClient.Contains(ptBottomCaret)) {
		//Platform::DebugPrintf("EnsureCaretVisible move, (%d,%d) (%d,%d)\n", pt.x, pt.y, rcClient.left, rcClient.right);
		// It should be possible to scroll the window to show the caret,
		// but this fails to remove the caret on GTK+
		if (topLine > lineCaret) {
			//ScrollTo(lineCaret);
			topLine = Platform::Clamp(lineCaret, 0, MaxScrollPos());
			SetVerticalScrollPos();
			Redraw();
		} else if (lineCaret > topLine + LinesOnScreen() - 1) {	// Caret is below the last displayed line
			//ScrollTo(lineCaret - LinesOnScreen() + 1);	// Scroll so the caret is in the last displayed line
			topLine = Platform::Clamp(lineCaret - LinesOnScreen() + 1, 0, MaxScrollPos());
			SetVerticalScrollPos();
			Redraw();
		}
		int xOffsetNew = xOffset;
		// The 2s here are to ensure the caret is really visible
		if (pt.x < rcClient.left) {
			xOffsetNew = xOffset - (rcClient.left - pt.x) - 2;
		} else if (pt.x > rcClient.right) {
			xOffsetNew = xOffset + (pt.x - rcClient.right) + 2;
		}
		if (xOffsetNew < 0)
			xOffsetNew = 0;
		if (xOffset != xOffsetNew) {
			xOffset = xOffsetNew;
			SetHorizontalScrollPos();
			Redraw();
		}
	}
}

void Editor::ShowCaretAtCurrentPosition() {
	if (!wMain.HasFocus()) {
		caret.active = false;
		caret.on = false;
		return;
	}
	caret.active = true;
	caret.on = true;
	SetTicking(true);
}

void Editor::DropCaret() {
	caret.active = false;
	InvalidateCaret();
}

void Editor::InvalidateCaret() {
	if (posDrag >= 0)
		InvalidateRange(posDrag, posDrag + 1);
	else
		InvalidateRange(currentPos, currentPos + 1);
}

void Editor::PaintSelMargin(Surface *surfWindow, PRectangle &rc) {
	if (vs.fixedColumnWidth == 0)
		return;

	PRectangle rcMargin = GetClientRectangle();
	rcMargin.right = vs.fixedColumnWidth;

	PRectangle rcSelMargin = rcMargin;
	rcSelMargin.left = vs.lineNumberWidth;

	if (!rc.Intersects(rcMargin))
		return;

	Surface *surface;
	if (bufferedDraw) {
		surface = &pixmapSelMargin;
	} else {
		surface = surfWindow;
	}

	// Required because of special way brush is created for selection margin
	surface->FillRectangle(rcSelMargin, pixmapSelPattern); {	// Scope the line and yposScreen variables
		int line = topLine;
		int yposScreen = 0;

		while (line < doc.LinesTotal() && yposScreen < rcMargin.bottom) {
			int marks = doc.GetMark(line);
			if (marks) {
				PRectangle rcMarker;
				rcMarker.left = 1 + vs.lineNumberWidth;
				rcMarker.top = yposScreen + 1;
				rcMarker.right = vs.lineNumberWidth + vs.selMarginWidth - 1;
				rcMarker.bottom = yposScreen + vs.lineHeight - 1;
				for (int markBit = 0; (markBit < 32) && marks; markBit++) {
					if (marks & 1) {
						vs.markers[markBit].Draw(surface, rcMarker);
					}
					marks >>= 1;
				}
			}
			line++;
			yposScreen += vs.lineHeight;
		}
	}

	if (vs.lineNumberWidth > 0) {
		int line = topLine;
		int ypos = 0;

		while (ypos < rcMargin.bottom) {
			char number[100];
			number[0] = '\0';
			if (line < doc.LinesTotal())
				sprintf(number, "%d", line + 1);
			int xpos = 0;
			PRectangle rcNumber;
			rcNumber.left = xpos;
			rcNumber.right = xpos + vs.lineNumberWidth;
			rcNumber.top = ypos;
			rcNumber.bottom = ypos + vs.lineHeight;
			surface->FillRectangle(rcNumber, vs.styles[STYLE_LINENUMBER].back.allocated);
			// Right justify
			int width = surface->WidthText(vs.styles[STYLE_LINENUMBER].font, number, strlen(number));
			xpos += vs.lineNumberWidth - width - 3;
			rcNumber.left = xpos;
			surface->DrawText(rcNumber, vs.styles[STYLE_LINENUMBER].font,
			                  ypos + vs.maxAscent, number, strlen(number),
			                  vs.styles[STYLE_LINENUMBER].fore.allocated, 
					  vs.styles[STYLE_LINENUMBER].back.allocated);
			line++;
			ypos += vs.lineHeight;
		}
	}

	if (bufferedDraw) {
		surfWindow->Copy(rcMargin, Point(), pixmapSelMargin);
	}
}

void DrawTabArrow(Surface *surface, PRectangle rcTab, int ymid) {
	int ydiff = (rcTab.bottom - rcTab.top) / 2;
	int xhead = rcTab.right - 1 - ydiff;
	if ((rcTab.left + 2) < (rcTab.right - 1))
		surface->MoveTo(rcTab.left + 2, ymid);
	else
		surface->MoveTo(rcTab.right - 1, ymid);
	surface->LineTo(rcTab.right - 1, ymid);
	surface->LineTo(xhead, ymid - ydiff);
	surface->MoveTo(rcTab.right - 1, ymid);
	surface->LineTo(xhead, ymid + ydiff);
}

void Editor::LayoutLine(int line, Surface *surface, ViewStyle &vstyle, LineLayout &ll) {
	int numCharsInLine = 0;
	int posLineStart = doc.LineStart(line);
	int posLineEnd = doc.LineStart(line + 1);
	Font &ctrlCharsFont = vstyle.styles[STYLE_CONTROLCHAR].font;
	char styleByte = 0;
	for (int charInDoc = posLineStart; 
		charInDoc < posLineEnd && numCharsInLine < LineLayout::maxLineLength - 1; 
		charInDoc++) {
		char chDoc = doc.CharAt(charInDoc);
		styleByte = doc.StyleAt(charInDoc);
		if (vstyle.viewEOL || ((chDoc != '\r') && (chDoc != '\n'))) {
			ll.chars[numCharsInLine] = chDoc;
			ll.styles[numCharsInLine] = styleByte & STYLE_MASK;
			ll.indicators[numCharsInLine] = styleByte & ~STYLE_MASK;
			numCharsInLine++;
		}
	}
	ll.chars[numCharsInLine] = 0;
	ll.styles[numCharsInLine] = styleByte;	// For eolFilled
	ll.indicators[numCharsInLine] = 0;

	// Layout the line, determining the position of each character
	int startseg = 0;
	int startsegx = 0;
	ll.positions[0] = 0;
	unsigned int tabWidth = vstyle.spaceWidth * doc.tabInChars;
	
	for (int charInLine = 0; charInLine < numCharsInLine; charInLine++) {
		if ((ll.styles[charInLine] != ll.styles[charInLine + 1]) ||
		        IsControlCharacter(ll.chars[charInLine]) || IsControlCharacter(ll.chars[charInLine + 1])) {
			ll.positions[startseg] = 0;
			if (IsControlCharacter(ll.chars[charInLine])) {
				if (ll.chars[charInLine] == '\t') {
					ll.positions[charInLine + 1] = ((((startsegx + 2) /
					                                   tabWidth) + 1) * tabWidth) - startsegx;
				} else {
					const char *ctrlChar = ControlCharacterString(ll.chars[charInLine]);
					// +3 For a blank on front and rounded edge each side:
					ll.positions[charInLine + 1] = surface->WidthText(ctrlCharsFont, ctrlChar, strlen(ctrlChar)) + 3;
				}
			} else {
				surface->MeasureWidths(vstyle.styles[ll.styles[charInLine]].font, ll.chars + startseg, 
					charInLine - startseg + 1, ll.positions + startseg + 1);
			}
			for (int posToIncrease = startseg; posToIncrease <= (charInLine + 1); posToIncrease++) {
				ll.positions[posToIncrease] += startsegx;
			}
			startsegx = ll.positions[charInLine + 1];
			startseg = charInLine + 1;
		}
	}
	ll.numCharsInLine = numCharsInLine;
}

void Editor::DrawLine(Surface *surface, ViewStyle &vsDraw, int line, int xStart, PRectangle rcLine, LineLayout &ll) {
	
	PRectangle rcSegment = rcLine;
	
	// Using one font for all control characters so it can be controlled independently to ensure
	// the box goes around the characters tightly. Seems to be no way to work out what height
	// is taken by an individual character - internal leading gives varying results.
	Font &ctrlCharsFont = vsDraw.styles[STYLE_CONTROLCHAR].font;

	int marks = 0;
	Colour markBack = Colour(0, 0, 0);
	if ((vsDraw.selMarginWidth == 0) && (vsDraw.showMarkedLines)) {
		marks = doc.GetMark(line);
		if (marks) {
			for (int markBit = 0; (markBit < 32) && marks; markBit++) {
				if (marks & 1) {
					markBack = vsDraw.markers[markBit].back.allocated;
				}
				marks >>= 1;
			}
		}
		marks = doc.GetMark(line);
	}

	int posLineStart = doc.LineStart(line);
	int posLineEnd = doc.LineStart(line + 1);

	int selStart = SelectionStart();
	int selEnd = SelectionEnd();

	int startseg = 0;
	for (int i = 0; i < ll.numCharsInLine; i++) {

		int iDoc = i + posLineStart;
		// If there is the end of a style run for any reason
		if ((ll.styles[i] != ll.styles[i + 1]) ||
		        IsControlCharacter(ll.chars[i]) || IsControlCharacter(ll.chars[i + 1]) ||
		        ((selStart != selEnd) && ((iDoc + 1 == selStart) || (iDoc + 1 == selEnd)))) {
			int styleMain = ll.styles[i];
			// text appears not to have a background color, so draw backing rect first
			Colour textBack = vsDraw.styles[styleMain].back.allocated;
			Colour textFore = vsDraw.styles[styleMain].fore.allocated;
			Font &textFont = vsDraw.styles[styleMain].font;
			bool inSelection = (iDoc >= selStart) && (iDoc < selEnd) && (selStart != selEnd);
			if (inSelection && !hideSelection) {
				if (vsDraw.selbackset)
					textBack = vsDraw.selbackground.allocated;
				if (vsDraw.selforeset)
					textFore = vsDraw.selforeground.allocated;
			} else {
				if (marks)
					textBack = markBack;
			}
			// Manage tab display
			if (ll.chars[i] == '\t') {
				rcSegment.left = ll.positions[i] + xStart;
				rcSegment.right = ll.positions[i + 1] + xStart;
				surface->FillRectangle(rcSegment, textBack);
				if (vsDraw.viewWhitespace) {
					surface->PenColour(textFore);
					PRectangle rcTab(rcSegment.left + 1, rcSegment.top + 4,
					                 rcSegment.right - 1, rcSegment.bottom - vsDraw.maxDescent);
					DrawTabArrow(surface, rcTab, rcSegment.top + vsDraw.lineHeight / 2);
				}
			// Manage control character display
			} else if (IsControlCharacter(ll.chars[i])) {
				const char *ctrlChar = ControlCharacterString(ll.chars[i]);
				rcSegment.left = ll.positions[i] + xStart;
				rcSegment.right = ll.positions[i + 1] + xStart;
				surface->FillRectangle(rcSegment, textBack);
				int normalCharHeight = surface->Ascent(ctrlCharsFont) -
				                       surface->InternalLeading(ctrlCharsFont);
				PRectangle rcCChar = rcSegment;
				rcCChar.left = rcCChar.left + 1;
				rcCChar.top = rcSegment.top + vsDraw.maxAscent - normalCharHeight;
				rcCChar.bottom = rcSegment.top + vsDraw.maxAscent + 1;
				PRectangle rcCentral = rcCChar;
				rcCentral.top++;
				rcCentral.bottom--;
				surface->FillRectangle(rcCentral, textFore);
				PRectangle rcChar = rcCChar;
				rcChar.left++;
				rcChar.right--;
				surface->DrawTextClipped(rcChar, ctrlCharsFont,
				                        rcSegment.top + vsDraw.maxAscent, ctrlChar, strlen(ctrlChar), 
							textBack, textFore);
			// Manage normal display
			} else {
				rcSegment.left = ll.positions[startseg] + xStart;
				rcSegment.right = ll.positions[i + 1] + xStart;
				surface->DrawText(rcSegment, textFont,
				                  rcSegment.top + vsDraw.maxAscent, ll.chars + startseg,
				                  i - startseg + 1, textFore, textBack);
				if (vsDraw.viewWhitespace) {
					for (int cpos = 0; cpos <= i - startseg; cpos++) {
						if (ll.chars[cpos + startseg] == ' ') {
							int xmid = (ll.positions[cpos + startseg] + ll.positions[cpos + startseg + 1]) / 2;
							PRectangle rcDot(xmid + xStart, rcSegment.top + vsDraw.lineHeight / 2, 0, 0);
							rcDot.right = rcDot.left + 1;
							rcDot.bottom = rcDot.top + 1;
							surface->FillRectangle(rcDot, textFore);
						}
					}
				}
			}
			startseg = i + 1;
		}
	}

	// Draw indicators
	int indStart[INDIC_MAX + 1] = {0};
	for (int indica = 0; indica <= INDIC_MAX; indica++)
		indStart[indica] = 0;

	for (int indicPos = 0; indicPos <= ll.numCharsInLine; indicPos++) {
		if (ll.indicators[indicPos] != ll.indicators[indicPos + 1]) {
			int mask = INDIC0_MASK;
			for (int indicnum = 0; indicnum <= INDIC_MAX; indicnum++) {
				if ((ll.indicators[indicPos + 1] & mask) && !(ll.indicators[indicPos] & mask)) {
					indStart[indicnum] = ll.positions[indicPos + 1];
				}
				if (!(ll.indicators[indicPos + 1] & mask) && (ll.indicators[indicPos] & mask)) {
					PRectangle rcIndic(
					    indStart[indicnum] + xStart,
					    rcLine.top + vsDraw.maxAscent,
					    ll.positions[indicPos + 1] + xStart,
					    rcLine.top + vsDraw.maxAscent + 3);
					vsDraw.indicators[indicnum].Draw(surface, rcIndic);
				}
				mask = mask << 1;
			}
		}
	}
	// End of the drawing of the current line

	// Fill in a PRectangle representing the end of line characters
	int xEol = ll.positions[ll.numCharsInLine];
	rcSegment.left = xEol + xStart;
	rcSegment.right = xEol + vsDraw.aveCharWidth + xStart;
	bool eolInSelection = (posLineEnd > selStart) && (posLineEnd <= selEnd) && (selStart != selEnd);
	if (eolInSelection && !hideSelection && vsDraw.selbackset && (line < doc.LinesTotal()-1)) {
		surface->FillRectangle(rcSegment, vsDraw.selbackground.allocated);
	} else if (marks) {
		surface->FillRectangle(rcSegment, markBack);
	} else {
		surface->FillRectangle(rcSegment, vsDraw.styles[ll.styles[ll.numCharsInLine] & STYLE_MASK].back.allocated);
	}

	rcSegment.left = xEol + vsDraw.aveCharWidth + xStart;
	rcSegment.right = rcLine.right;
	if (marks) {
		surface->FillRectangle(rcSegment, markBack);
	} else if (vsDraw.styles[ll.styles[ll.numCharsInLine] & STYLE_MASK].eolFilled) {
		surface->FillRectangle(rcSegment, vsDraw.styles[ll.styles[ll.numCharsInLine] & STYLE_MASK].back.allocated);
	} else {
		surface->FillRectangle(rcSegment, vsDraw.styles[STYLE_DEFAULT].back.allocated);
	}
}

void Editor::Paint(Surface *surfaceWindow, PRectangle rcArea) {
	//Platform::DebugPrintf("Paint %d %d - %d %d\n", rcArea.left, rcArea.top, rcArea.right, rcArea.bottom);
	RefreshStyleData();

	PRectangle rcClient = GetClientRectangle();
	//Platform::DebugPrintf("Client: (%3d,%3d) ... (%3d,%3d)   %d\n",
	//	rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

	if (!pixmapSelPattern.Initialised()) {
		pixmapSelPattern.InitPixMap(8, 8, surfaceWindow);
		// This complex procedure is to reproduce the checker board dithered pattern used by windows
		// for scroll bars and Visual Studio for its selection margin. The colour of this pattern is half
		// way between the chrome colour and the chrome highlight colour making a nice transition
		// between the window chrome and the content area. And it works in low colour depths.
		PRectangle rcPattern(0, 0, 8, 8);
		if (vs.selbarlight.desired == Colour(0xff, 0xff, 0xff)) {
			pixmapSelPattern.FillRectangle(rcPattern, vs.selbar.allocated);
			pixmapSelPattern.PenColour(vs.selbarlight.allocated);
			for (int stripe = 0; stripe < 8; stripe++) {
				pixmapSelPattern.MoveTo(0, stripe * 2);
				pixmapSelPattern.LineTo(8, stripe * 2 - 8);
			}
		} else {
			// User has chosen an unusual chrome colour scheme so just use the highlight edge colour.
			pixmapSelPattern.FillRectangle(rcPattern, vs.selbarlight.allocated);
		}
	}

	if (bufferedDraw) {
		if (!pixmapLine.Initialised()) {
			pixmapLine.InitPixMap(rcClient.Width(), rcClient.Height(),
			                      surfaceWindow);
			pixmapSelMargin.InitPixMap(vs.fixedColumnWidth,
			                           rcClient.Height(), surfaceWindow);
		}
	}

	surfaceWindow->SetPalette(&palette, true);
	pixmapLine.SetPalette(&palette, !wMain.HasFocus());

	//Platform::DebugPrintf("Paint: (%3d,%3d) ... (%3d,%3d)   %d\n",
	//	rcArea.left, rcArea.top, rcArea.right, rcArea.bottom);

	int screenLinePaintFirst = rcArea.top / vs.lineHeight;
	int linePaintLast = topLine + rcArea.bottom / vs.lineHeight + 1;
	int endPosPaint = doc.Length();
	if (linePaintLast < doc.LinesTotal())
		endPosPaint = doc.LineStart(linePaintLast + 1);

	int xStart = vs.fixedColumnWidth - xOffset;
	int ypos = 0;
	if (!bufferedDraw)
		ypos += screenLinePaintFirst * vs.lineHeight;
	int yposScreen = screenLinePaintFirst * vs.lineHeight;

	PaintSelMargin(surfaceWindow, rcArea);

	doc.StartStyleSequence();
	if (endPosPaint > doc.GetEndStyled()) {
		// Notify container to do some more styling
		NotifyStyleNeeded(endPosPaint);
		CheckForChangeOutsidePaint(doc.StyleChanged());
	}
	NotifyCheckBrace();
	
	if (paintState == paintAbandoned) {
		// Either NotifyStyleNeeded or NotifyCheckBrace noticed that painting is needed
		// outside the current painting rectangle
		//Platform::DebugPrintf("Abandoning paint\n");
		return;
	}
	//Platform::DebugPrintf("start display %d, margin = %d offset = %d\n", doc.Length(), selMarginWidth, xOffset);

	Surface *surface = 0;
	if (rcArea.right > vs.selMarginWidth) {

		if (bufferedDraw) {
			surface = &pixmapLine;
		} else {
			surface = surfaceWindow;
		}

		int line = topLine + screenLinePaintFirst;

		int posCaret = currentPos;
		if (posDrag >= 0)
			posCaret = posDrag;
		int lineCaret = doc.LineFromPosition(posCaret);

		// Remove selection margin from drawing area so text will not be drawn
		// on it in unbuffered mode.
		PRectangle rcTextArea = rcClient;
		rcTextArea.left = vs.fixedColumnWidth;
		surfaceWindow->SetClip(rcTextArea);
		//GTimer *tim=g_timer_new();
		while (line < doc.LinesTotal() && yposScreen < rcArea.bottom) {
			//g_timer_start(tim);
			//Platform::DebugPrintf("Painting line %d\n", line);

			int posLineStart = doc.LineStart(line);
			int posLineEnd = doc.LineStart(line + 1);
			//Platform::DebugPrintf("line %d %d - %d\n", line, posLineStart, posLineEnd);

			PRectangle rcLine = rcClient;
			rcLine.top = ypos;
			rcLine.bottom = ypos + vs.lineHeight;

			// Copy this line and its styles from the document into local arrays
			// and determine the x position at which each character starts.
			LineLayout ll;
			LayoutLine(line, surface, vs, ll);
			                                
			// Highlight the current braces if any
			if ((braces[0] >= posLineStart) && (braces[0] < posLineEnd))
				ll.styles[braces[0] - posLineStart] = bracesMatchStyle;
			if ((braces[1] >= posLineStart) && (braces[1] < posLineEnd))
				ll.styles[braces[1] - posLineStart] = bracesMatchStyle;
				
			// Draw the line
			DrawLine(surface, vs, line, xStart, rcLine, ll);
			
			// Draw the Caret
			if (line == lineCaret) {
				int xposCaret = ll.positions[posCaret - posLineStart] + xStart;
				int widthOverstrikeCaret =
				    ll.positions[posCaret - posLineStart + 1] - ll.positions[posCaret - posLineStart];
				if (posCaret == doc.Length())
					widthOverstrikeCaret = vs.aveCharWidth;
				if (widthOverstrikeCaret < 3)
					widthOverstrikeCaret = 3;
				if (((caret.active && caret.on) || (posDrag >= 0)) && xposCaret >= 0) {
					PRectangle rcCaret = rcLine;
					if (posDrag >= 0) {
						rcCaret.left = xposCaret;
						rcCaret.right = xposCaret + 1;
					} else {
						if (inOverstrike) {
							rcCaret.top = rcCaret.bottom - 2;
							rcCaret.left = xposCaret + 1;
							rcCaret.right = rcCaret.left + widthOverstrikeCaret - 1;
						} else {
							rcCaret.left = xposCaret;
							rcCaret.right = xposCaret + 1;
						}
					}
					surface->FillRectangle(rcCaret, vs.caretcolour.allocated);
				}
			}
			
			if (bufferedDraw) {
				Point from(vs.fixedColumnWidth, 0);
				PRectangle rcCopyArea(vs.fixedColumnWidth, yposScreen,
				                  rcClient.right, yposScreen + vs.lineHeight);
				surfaceWindow->Copy(rcCopyArea, from, pixmapLine);
			}

			if (!bufferedDraw) {
				ypos += vs.lineHeight;
			}

			yposScreen += vs.lineHeight;
			line++;
			//gdk_flush();
			//g_timer_stop(tim);
			//Platform::DebugPrintf("Paint [%0d] took %g\n", line, g_timer_elapsed(tim, 0));
		}
		//g_timer_destroy(tim);
		PRectangle rcBeyondEOF = rcClient;
		rcBeyondEOF.left = vs.fixedColumnWidth;
		rcBeyondEOF.right = rcBeyondEOF.right;
		rcBeyondEOF.top = (doc.LinesTotal() - topLine) * vs.lineHeight;
		if (rcBeyondEOF.top < rcBeyondEOF.bottom) {
			surfaceWindow->FillRectangle(rcBeyondEOF, vs.styles[STYLE_DEFAULT].back.allocated);
		}
	}
}

// Space (3 space characters) between line numbers and text when printing.
#define lineNumberPrintSpace "   "

// This is mostly copied from the Paint method but with some things omitted
// such as the margin markers, line numbers, selection and caret
// Should be merged back into a combined Draw method.
long Editor::FormatRange(bool draw, FORMATRANGE *pfr) {
	if (!pfr)
		return 0;

	Surface *surface = new Surface();
	surface->Init(pfr->hdc);
	Surface *surfaceMeasure = new Surface();
	surfaceMeasure->Init(pfr->hdcTarget);
	
	ViewStyle vsPrint(vs);
	
	// Modify the view style for printing as do not normally want any of the transient features to be printed
	// Printing supports only the line number margin.
	vsPrint.selMarginWidth = 0;
	vsPrint.showMarkedLines = false;
	vsPrint.fixedColumnWidth = 0;
	vsPrint.zoomLevel = 0;
	// Don't show the selection when printing
	vsPrint.selbackset = false;
	vsPrint.selforeset = false;
	// White background for the line numbers
	vsPrint.styles[STYLE_LINENUMBER].back.desired = Colour(0xff,0xff,0xff); 
	
	vsPrint.Refresh(*surfaceMeasure);
	// Ensure colours are set up
	vsPrint.RefreshColourPalette(palette, true);
	vsPrint.RefreshColourPalette(palette, false);
	// Determining width must hapen after fonts have been realised in Refresh
	if (vsPrint.lineNumberWidth) 
		vsPrint.lineNumberWidth = surface->WidthText(vsPrint.styles[STYLE_LINENUMBER].font, 
			"9999" lineNumberPrintSpace, 4 + strlen(lineNumberPrintSpace));

	int linePrintStart = doc.LineFromPosition(pfr->chrg.cpMin);
	int linePrintLast = linePrintStart + (pfr->rc.bottom - pfr->rc.top) / vsPrint.lineHeight - 1;
	if (linePrintLast < linePrintStart)
		linePrintLast = linePrintStart;
	int linePrintMax = doc.LineFromPosition(pfr->chrg.cpMax - 1);
	if (linePrintLast > linePrintMax)
		linePrintLast = linePrintMax;
	//Platform::DebugPrintf("Formatting lines=[%0d,%0d,%0d] top=%0d bottom=%0d line=%0d %0d\n",
	//	linePrintStart, linePrintLast, linePrintMax, pfr->rc.top, pfr->rc.bottom, vsPrint.lineHeight,
	//	surfaceMeasure->Height(vsPrint.styles[STYLE_LINENUMBER].font));
	int endPosPrint = doc.Length();
	if (linePrintLast < doc.LinesTotal())
		endPosPrint = doc.LineStart(linePrintLast + 1);

	if (endPosPrint > doc.GetEndStyled()) {
		// Notify container to do some more styling
		NotifyStyleNeeded(endPosPrint);
	}
	int xStart = vsPrint.fixedColumnWidth + pfr->rc.left + vsPrint.lineNumberWidth;
	int ypos = pfr->rc.top;
	int line = linePrintStart;

	if (draw) {	// Otherwise just measuring

		while (line <= linePrintLast && ypos < pfr->rc.bottom) {

			PRectangle rcLine;
			rcLine.left = pfr->rc.left + vsPrint.lineNumberWidth;
			rcLine.top = ypos;
			rcLine.right = pfr->rc.right;
			rcLine.bottom = ypos + vsPrint.lineHeight;
			
			if (vsPrint.lineNumberWidth) {
				char number[100];
				sprintf(number, "%d" lineNumberPrintSpace, line + 1);
				PRectangle rcNumber = rcLine;
				rcNumber.right = rcNumber.left + vs.lineNumberWidth;
				// Right justify
				rcNumber.left += vs.lineNumberWidth - 
					surface->WidthText(vsPrint.styles[STYLE_LINENUMBER].font, number, strlen(number));
				surface->DrawText(rcNumber, vsPrint.styles[STYLE_LINENUMBER].font,
				                  ypos + vsPrint.maxAscent, number, strlen(number),
				                  vsPrint.styles[STYLE_LINENUMBER].fore.allocated, 
						  vsPrint.styles[STYLE_LINENUMBER].back.allocated);
			}
			
			// Copy this line and its styles from the document into local arrays
			// and determine the x position at which each character starts.
			LineLayout ll;
			LayoutLine(line, surfaceMeasure, vsPrint, ll);
			                                
			// Draw the line
			DrawLine(surface, vsPrint, line, xStart, rcLine, ll);

			ypos += vsPrint.lineHeight;
			line++;
		}
	}

	delete surface;
	delete surfaceMeasure;

	return endPosPrint;
}

void Editor::SetScrollBarsTo(PRectangle) {
	RefreshStyleData();

	int nMax = doc.LinesTotal();
	int nPage = doc.LinesTotal() - MaxScrollPos() + 1;
	bool modified = ModifyScrollBars(nMax, nPage);

	// TODO: ensure always showing as many lines as possible
	// May not be, if, for example, window made larger
	if (topLine > MaxScrollPos()) {
		topLine = Platform::Clamp(topLine, 0, MaxScrollPos());
		SetVerticalScrollPos();
		Redraw();
	}
	if (modified)
		Redraw();
	//Platform::DebugPrintf("end max = %d page = %d\n", nMax, nPage);
}

void Editor::SetScrollBars() {
	PRectangle rsClient = GetClientRectangle();
	SetScrollBarsTo(rsClient);
}

void Editor::AddChar(char ch) {
	bool wasSelection = currentPos != anchor;
	ClearSelection();
	if (inOverstrike && !wasSelection) {
		if (currentPos < (doc.Length() - 1)) {
			if ((doc.CharAt(currentPos) != '\r') && (doc.CharAt(currentPos) != '\n')) {
				doc.DelChar(currentPos);
			}
		}
	}
	doc.InsertChar(currentPos, ch);
	SetEmptySelection(currentPos + 1);
	EnsureCaretVisible();
	if (!wasSelection && isprint(ch)) {
		// Try to optimise redraw to just the current line and following line
		// Colourise line and see if line end style changes
		int line = doc.LineFromPosition(currentPos);
		int lineEnd = doc.Length();
		if (line < doc.LinesTotal()-1)
			lineEnd = doc.LineStart(line + 2) - 1;
		// Try to style to end of line including line end characters
		Position endCheck = lineEnd;
		if (lineEnd < doc.Length() - 1)
			endCheck = lineEnd + 1;
		doc.StartStyleSequence();
		NotifyStyleNeeded(endCheck);
		//Platform::DebugPrintf("Addchar trying line = %d lineEnd = %d colour = %d new colour = %d\n",
		//		line, lineEnd, colour,
		//		doc.StyleAt(lineEnd));
		Range rangeStyleChanged = doc.StyleChanged();
		if (rangeStyleChanged.end >= endCheck - 1) {
			//Platform::DebugPrintf("Fallback addchar %x\n", ch);
			Redraw();
		} else {
			InvalidateRange(doc.LineStart(line), doc.LineEndPosition(currentPos));
			//Platform::DebugPrintf("Optimal %d %d-%d\n", line, doc.LineStart(line), doc.LineEndPosition(currentPos));
		}



	} else {
		//Platform::DebugPrintf("Standard addchar %x\n", ch);
		Redraw();
	}
	SetLastXChosen();
	NotifyChar(ch);
}

void Editor::ClearSelection() {
	int startPos = SelectionStart();
	unsigned int chars = SelectionEnd() - startPos;
	SetEmptySelection(startPos);
	if (0 != chars) {
		doc.DeleteChars(startPos, chars);
	}
}

void Editor::ClearAll() {
	if (0 != doc.Length()) {
		doc.DeleteChars(0, doc.Length());
	}
	anchor = 0;
	currentPos = 0;
	topLine = 0;
	SetVerticalScrollPos();
	Redraw();
}

void Editor::Cut() {
	Copy();
	ClearSelection();
	Redraw();
}

void Editor::Clear() {
	if (currentPos == anchor) {
		DelChar();
	} else {
		ClearSelection();
	}
	SetEmptySelection(currentPos);
	Redraw();
}

void Editor::SelectAll() {
	SetSelection(0, doc.Length());
	Redraw();
}

void Editor::Undo() {
	if (doc.CanUndo()) {
		int newPos = doc.Undo();
		SetEmptySelection(newPos);
		EnsureCaretVisible();
		Redraw();
		SetScrollBars();
	}
}

void Editor::Redo() {
	if (doc.CanRedo()) {
		int newPos = doc.Redo();
		SetEmptySelection(newPos);
		EnsureCaretVisible();
		Redraw();
		SetScrollBars();
	}
}

void Editor::DelChar() {
	doc.DelChar(currentPos);
	Redraw();
}

void Editor::DelCharBack() {
	if (currentPos == anchor) {
		int newPos = doc.DelCharBack(currentPos);
		SetEmptySelection(newPos);
	} else {
		ClearSelection();
		SetEmptySelection(currentPos);
	}
	Redraw();
}

void Editor::NotifyStyleNeeded(int endStyleNeeded) {
	SCNotification scn;
	scn.nmhdr.code = SCN_STYLENEEDED;
	scn.position = endStyleNeeded;
	NotifyParent(scn);
}

void Editor::NotifyChar(char ch) {
	SCNotification scn;
	scn.nmhdr.code = SCN_CHARADDED;
	scn.ch = ch;
	NotifyParent(scn);
}

void Editor::NotifySavePoint(bool isSavePoint) {
	SCNotification scn;
	if (isSavePoint) {
		scn.nmhdr.code = SCN_SAVEPOINTREACHED;
	} else {
		scn.nmhdr.code = SCN_SAVEPOINTLEFT;
	}
	NotifyParent(scn);
}

void Editor::NotifyModifyAttempt() {
	SCNotification scn;
	scn.nmhdr.code = SCN_MODIFYATTEMPTRO;
	NotifyParent(scn);
}

void Editor::NotifyDoubleClick(Point, bool) {
	SCNotification scn;
	scn.nmhdr.code = SCN_DOUBLECLICK;
	NotifyParent(scn);
}

void Editor::NotifyCheckBrace() {
	SCNotification scn;
	scn.nmhdr.code = SCN_CHECKBRACE;
	NotifyParent(scn);
}

// Notifications from document
void Editor::NotifyModifyAttempt(Document*, void *) {
	//Platform::DebugPrintf("** Modify Attempt\n");
	NotifyModifyAttempt();
}

void Editor::NotifySavePoint(Document*, void *, bool atSavePoint) {
	//Platform::DebugPrintf("** Save Point %s\n", atSavePoint ? "On" : "Off");
	NotifySavePoint(atSavePoint);
}

void Editor::NotifyModified(Document*, void *) {
	//Platform::DebugPrintf("** Changed\n");
	NotifyChange();
	SetScrollBars();
}

// Force scroll and keep position relative to top of window
void Editor::PageMove(int direction, bool extend) {
	Point pt = LocationFromPosition(currentPos);
	int newTop = Platform::Clamp(
	                 topLine + direction * LinesToScroll(), 0, MaxScrollPos());
	int newPos = PositionFromLocation(
	                 Point(lastXChosen, pt.y + direction * (vs.lineHeight * LinesToScroll())));
	if (newTop != topLine) {
		topLine = newTop;
		MovePositionTo(newPos, extend);
		Redraw();
		SetVerticalScrollPos();
	} else {
		MovePositionTo(newPos, extend);
	}
}

int Editor::KeyCommand(UINT iMessage) {
	Point pt = LocationFromPosition(currentPos);

	switch (iMessage) {
	case SCI_LINEDOWN:
		MovePositionTo(PositionFromLocation(
		                   Point(lastXChosen, pt.y + vs.lineHeight)));
		break;
	case SCI_LINEDOWNEXTEND:
		MovePositionTo(PositionFromLocation(
		                   Point(lastXChosen, pt.y + vs.lineHeight)), true);
		break;
	case SCI_LINEUP:
		MovePositionTo(PositionFromLocation(
		                   Point(lastXChosen, pt.y - vs.lineHeight)));
		break;
	case SCI_LINEUPEXTEND:
		MovePositionTo(PositionFromLocation(
		                   Point(lastXChosen, pt.y - vs.lineHeight)), true);
		break;
	case SCI_CHARLEFT:
		if (SelectionEmpty()) {
			MovePositionTo(currentPos - 1);
		} else {
			MovePositionTo(SelectionStart());
		}
		SetLastXChosen();
		break;
	case SCI_CHARLEFTEXTEND:
		MovePositionTo(currentPos - 1, true);
		SetLastXChosen();
		break;
	case SCI_CHARRIGHT:
		if (SelectionEmpty()) {
			MovePositionTo(currentPos + 1);
		} else {
			MovePositionTo(SelectionEnd());
		}
		SetLastXChosen();
		break;
	case SCI_CHARRIGHTEXTEND:
		MovePositionTo(currentPos + 1, true);
		SetLastXChosen();
		break;
	case SCI_WORDLEFT:
		MovePositionTo(doc.NextWordStart(currentPos, -1));
		SetLastXChosen();
		break;
	case SCI_WORDLEFTEXTEND:
		MovePositionTo(doc.NextWordStart(currentPos, -1), true);
		SetLastXChosen();
		break;
	case SCI_WORDRIGHT:
		MovePositionTo(doc.NextWordStart(currentPos, 1));
		SetLastXChosen();
		break;
	case SCI_WORDRIGHTEXTEND:
		MovePositionTo(doc.NextWordStart(currentPos, 1), true);
		SetLastXChosen();
		break;
	case SCI_HOME:
		MovePositionTo(doc.LineStart(doc.LineFromPosition(currentPos)));
		SetLastXChosen();
		break;
	case SCI_HOMEEXTEND:
		MovePositionTo(doc.LineStart(doc.LineFromPosition(currentPos)), true);
		SetLastXChosen();
		break;
	case SCI_LINEEND:
		MovePositionTo(doc.LineEndPosition(currentPos));
		SetLastXChosen();
		break;
	case SCI_LINEENDEXTEND:
		MovePositionTo(doc.LineEndPosition(currentPos), true);
		SetLastXChosen();
		break;
	case SCI_DOCUMENTSTART:
		MovePositionTo(0);
		SetLastXChosen();
		break;
	case SCI_DOCUMENTSTARTEXTEND:
		MovePositionTo(0, true);
		SetLastXChosen();
		break;
	case SCI_DOCUMENTEND:
		MovePositionTo(doc.Length());
		SetLastXChosen();
		break;
	case SCI_DOCUMENTENDEXTEND:
		MovePositionTo(doc.Length(), true);
		SetLastXChosen();
		break;
	case SCI_PAGEUP:
		PageMove( -1);
		break;
	case SCI_PAGEUPEXTEND:
		PageMove( -1, true);
		break;
	case SCI_PAGEDOWN:
		PageMove(1);
		break;
	case SCI_PAGEDOWNEXTEND:
		PageMove(1, true);
		break;
	case SCI_EDITTOGGLEOVERTYPE:
		inOverstrike = !inOverstrike;
		DropCaret();
		ShowCaretAtCurrentPosition();
		break;
	case SCI_CANCEL:  	// Cancel any modes - handled in subclass
		// Also unselect text
		SetEmptySelection(currentPos);
		break;
	case SCI_DELETEBACK:
		DelCharBack();
		EnsureCaretVisible();
		break;
	case SCI_TAB:
		Indent(true);
		break;
	case SCI_BACKTAB:
		Indent(false);
		break;
	case SCI_NEWLINE:
		ClearSelection();
		if (doc.eolMode == SC_EOL_CRLF) {
			doc.InsertString(currentPos, "\r\n");
			SetEmptySelection(currentPos + 2);
			NotifyChar('\r');
			NotifyChar('\n');
		} else if (doc.eolMode == SC_EOL_CR) {
			doc.InsertChar(currentPos, '\r');
			SetEmptySelection(currentPos + 1);
			NotifyChar('\r');
		} else if (doc.eolMode == SC_EOL_LF) {
			doc.InsertChar(currentPos, '\n');
			SetEmptySelection(currentPos + 1);
			NotifyChar('\n');
		}
		EnsureCaretVisible();
		break;
	case SCI_FORMFEED:
		AddChar('\f');
		break;
	case SCI_VCHOME:
		MovePositionTo(doc.VCHomePosition(currentPos));
		SetLastXChosen();
		break;
	case SCI_VCHOMEEXTEND:
		MovePositionTo(doc.VCHomePosition(currentPos), true);
		SetLastXChosen();
		break;
	case SCI_ZOOMIN:
		if (vs.zoomLevel < 20)
			vs.zoomLevel++;
		InvalidateStyleRedraw();
		break;
	case SCI_ZOOMOUT:
		if (vs.zoomLevel > -10)
			vs.zoomLevel--;
		InvalidateStyleRedraw();
		break;
	case SCI_DELWORDLEFT: {
			int startWord = doc.NextWordStart(currentPos, -1);
			doc.DeleteChars(startWord, currentPos - startWord);
			MovePositionTo(startWord);
			Redraw();
		}
		break;
	case SCI_DELWORDRIGHT: {
			int endWord = doc.NextWordStart(currentPos, 1);
			doc.DeleteChars(currentPos, endWord - currentPos);
			Redraw();
		}
		break;
	}
	return 0;
}

int Editor::KeyDefault(int, int) {
	return 0;
}

int Editor::KeyDown(int key, bool shift, bool ctrl, bool alt) {
	int modifiers = (shift ? SCI_SHIFT : 0) | (ctrl ? SCI_CTRL : 0) |
	                (alt ? SCI_ALT : 0);
	int msg = kmap.Find(key, modifiers);
	if (msg)
		return WndProc(msg, 0, 0);
	else
		return KeyDefault(key, modifiers);
}

void Editor::SetWhitespaceVisible(bool view) {
	vs.viewWhitespace = view;
}

bool Editor::GetWhitespaceVisible() {
	return vs.viewWhitespace;
}

void Editor::Indent(bool forwards) {
	//Platform::DebugPrintf("INdent %d\n", forwards);
	int lineOfAnchor = doc.LineFromPosition(anchor);
	int lineCurrentPos = doc.LineFromPosition(currentPos);
	if (lineOfAnchor == lineCurrentPos) {
		ClearSelection();
		doc.InsertChar(currentPos++, '\t');
		SetEmptySelection(currentPos);
	} else {
		int anchorPosOnLine = anchor - doc.LineStart(lineOfAnchor);
		int currentPosPosOnLine = currentPos - doc.LineStart(lineCurrentPos);
		// Multiple lines selected so indent / dedent
		int lineTopSel = Platform::Minimum(lineOfAnchor, lineCurrentPos);
		int lineBottomSel = Platform::Maximum(lineOfAnchor, lineCurrentPos);
		if (doc.LineStart(lineBottomSel) == anchor || doc.LineStart(lineBottomSel) == currentPos)
			lineBottomSel--;  	// If not selecting any characters on a line, do not indent
		doc.BeginUndoAction();
		doc.Indent(forwards, lineBottomSel, lineTopSel);
		doc.EndUndoAction();
		if (lineOfAnchor < lineCurrentPos) {
			if (currentPosPosOnLine == 0)
				SetSelection(doc.LineStart(lineCurrentPos), doc.LineStart(lineOfAnchor));
			else
				SetSelection(doc.LineStart(lineCurrentPos + 1), doc.LineStart(lineOfAnchor));
		} else {
			if (anchorPosOnLine == 0)
				SetSelection(doc.LineStart(lineCurrentPos), doc.LineStart(lineOfAnchor));
			else
				SetSelection(doc.LineStart(lineCurrentPos), doc.LineStart(lineOfAnchor + 1));
		}
	}
}

long Editor::FindText(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	FINDTEXTEX *ft = reinterpret_cast<FINDTEXTEX *>(lParam);
	int pos = doc.FindText(ft->chrg.cpMin, ft->chrg.cpMax, ft->lpstrText,
	                       wParam & FR_MATCHCASE, wParam & FR_WHOLEWORD);
	if (pos != -1) {
		if (iMessage == EM_FINDTEXTEX) {
			ft->chrgText.cpMin = pos;
			ft->chrgText.cpMax = pos + strlen(ft->lpstrText);
		}
	}
	return pos;
}

void Editor::GoToLine(int lineNo) {
	if (lineNo > doc.LinesTotal())
		lineNo = doc.LinesTotal();
	if (lineNo < 0)
		lineNo = 0;
	SetEmptySelection(doc.LineStart(lineNo));
	ShowCaretAtCurrentPosition();
	EnsureCaretVisible();
}

static bool Close(Point pt1, Point pt2) {
	if (abs(pt1.x - pt2.x) > 3)
		return false;
	if (abs(pt1.y - pt2.y) > 3)
		return false;
	return true;
}

char *Editor::CopyRange(int start, int end) {
	char *text = 0;
	if (start < end) {
		int len = end - start;
		text = new char[len + 1];
		if (text) {
			for (int i = 0; i < len; i++) {
				text[i] = doc.CharAt(start + i);
			}
			text[len] = '\0';
		}
	}
	return text;
}

char *Editor::CopySelectionRange() {
	return CopyRange(SelectionStart(), SelectionEnd());
}

void Editor::CopySelectionIntoDrag() {
	delete []dragChars;
	dragChars = 0;
	lenDrag = SelectionEnd() - SelectionStart();
	dragChars = CopyRange(SelectionStart(), SelectionEnd());
	if (!dragChars) {
		lenDrag = 0;
	}
}

void Editor::SetDragPosition(int newPos) {
	if (newPos >= 0) {
		newPos = doc.MovePositionOutsideChar(newPos, 1);
		posDrop = newPos;
	}
	if (posDrag != newPos) {
		caret.on = true;
		SetTicking(true);
		InvalidateCaret();
		posDrag = newPos;
		InvalidateCaret();
	}
}

void Editor::StartDrag() {
	// Always handled by subclasses
	//SetMouseCapture(true);
	//wDraw.SetCursor(Window::cursorArrow);
}

void Editor::DropAt(int position, const char *value, bool moving) {
	//Platform::DebugPrintf("DropAt %d\n", inDragDrop);
	int selStart = SelectionStart();
	int selEnd = SelectionEnd();

	if (inDragDrop)
		dropWentOutside = false;

	if ((!inDragDrop) || (position <= selStart) || (position >= selEnd)) {

		doc.BeginUndoAction();

		if (inDragDrop && moving) {
			// Remove dragged out text
			ClearSelection();
			if (position > selStart) {
				position -= selEnd - selStart;
			}
		}

		position = doc.MovePositionOutsideChar(position, currentPos - position);
		doc.InsertString(position, value);

		doc.EndUndoAction();

		SetSelection(position + strlen(value), position);

	} else if (inDragDrop) {
		SetSelection(position, position);
	}
}

bool Editor::PositionInSelection(int pos) {
	pos = doc.MovePositionOutsideChar(pos, currentPos - pos);
	if (currentPos > anchor) {
		return (pos >= anchor) && (pos <= currentPos);
	} else if (currentPos < anchor) {
		return (pos <= anchor) && (pos >= currentPos);
	}
	return false;
}

bool Editor::PointInSelection(Point pt) {
	int pos = PositionFromLocation(pt);
	if (PositionInSelection(pos)) {
		if (pos == SelectionStart()) {
			// see if just before selection
			Point locStart = LocationFromPosition(pos);
			if (pt.x < locStart.x)
				return false;
		}
		if (pos == SelectionEnd()) {
			// see if just after selection
			Point locEnd = LocationFromPosition(pos);
			if (pt.x > locEnd.x)
				return false;
		}
		return true;
	}
	return false;
}

bool Editor::PointInSelMargin(Point pt) {
	if (vs.fixedColumnWidth > 0) {	// There is a margin
		PRectangle rcSelMargin = GetClientRectangle();
		rcSelMargin.right = vs.fixedColumnWidth;
		//		rcSelMargin.left = vs.lineNumberWidth;
		return rcSelMargin.Contains(pt);
	} else {
		return false;
	}
}

void Editor::ButtonDown(Point pt, unsigned int curTime, bool shift, bool ctrl) {
	//Platform::DebugPrintf("Scintilla:ButtonDown %d %d = %d\n", curTime, lastClickTime, curTime - lastClickTime);
	ptMouseLast = pt;
	int newPos = PositionFromLocation(pt);
	newPos = doc.MovePositionOutsideChar(newPos, currentPos - newPos);
	inDragDrop = false;
	if (shift) {
		SetSelection(newPos);
	}
	if (((curTime - lastClickTime) < Platform::DoubleClickTime()) && Close(pt, lastClick)) {
		//Platform::DebugPrintf("Double click %d %d = %d\n", curTime, lastClickTime, curTime - lastClickTime);
		SetMouseCapture(true);
		SetEmptySelection(newPos);
		bool doubleClick = false;
		// Stop mouse button bounce changing selection type
		if (curTime != lastClickTime) {
			if (selectionType == selChar) {
				selectionType = selWord;
				doubleClick = true;
			} else if (selectionType == selWord) {
				selectionType = selLine;
			} else {
				selectionType = selChar;
				originalAnchorPos = currentPos;
			}
		}

		if (selectionType == selWord) {
			if (currentPos >= originalAnchorPos) {	// Moved forward
				SetSelection(doc.ExtendWordSelect(currentPos, 1),
				             doc.ExtendWordSelect(originalAnchorPos, -1));
			} else {	// Moved backward
				SetSelection(doc.ExtendWordSelect(currentPos, -1),
				             doc.ExtendWordSelect(originalAnchorPos, 1));
			}
		} else if (selectionType == selLine) {
			lineAnchor = LineFromLocation(pt);
			SetSelection(doc.LineStart(lineAnchor + 1), doc.LineStart(lineAnchor));
			//Platform::DebugPrintf("Triple click: %d - %d\n", anchor, currentPos);
		}
		else {
			SetEmptySelection(currentPos);
		}
		//Platform::DebugPrintf("Double click: %d - %d\n", anchor, currentPos);
		if (doubleClick)
			NotifyDoubleClick(pt, shift);
	} else {	// Single click
		if (PointInSelMargin(pt)) {
			if (ctrl) {
				SelectAll();
				lastClickTime = curTime;
				return;
			}
			lineAnchor = LineFromLocation(pt);
			if (!shift) {
				// Single click in margin: select whole line
				SetSelection(doc.LineStart(lineAnchor + 1), doc.LineStart(lineAnchor));
			} else {
				// Single shift+click in margin: select from anchor to beginning of clicked line
				SetSelection(doc.LineStart(lineAnchor), anchor);
			}
			SetDragPosition(invalidPosition);
			SetMouseCapture(true);
			selectionType = selLine;
		} else {
			if (!shift) {
				inDragDrop = PointInSelection(pt);
			}
			if (inDragDrop) {
				SetMouseCapture(false);
				SetDragPosition(newPos);
				CopySelectionIntoDrag();
				StartDrag();
			} else {
				SetDragPosition(invalidPosition);
				SetMouseCapture(true);
				if (!shift)
					SetEmptySelection(newPos);
				selectionType = selChar;
				originalAnchorPos = currentPos;
			}
		}
	}
	lastClickTime = curTime;
	Redraw();
	lastXChosen = pt.x;
	ShowCaretAtCurrentPosition();
}

void Editor::ButtonMove(Point pt) {
	//Platform::DebugPrintf("Move %d %d\n", pt.x, pt.y);
	if (HaveMouseCapture()) {
		ptMouseLast = pt;
		int movePos = PositionFromLocation(pt);
		movePos = doc.MovePositionOutsideChar(movePos, currentPos - movePos);
		if (posDrag >= 0) {
			SetDragPosition(movePos);
		} else {
			if (selectionType == selChar) {
				SetSelection(movePos);
				//Platform::DebugPrintf("Move: %d - %d\n", anchor, currentPos);
			} else if (selectionType == selWord) {
				// Continue selecting by word
				if (currentPos > originalAnchorPos) {	// Moved forward
					SetSelection(doc.ExtendWordSelect(movePos, 1),
					             doc.ExtendWordSelect(originalAnchorPos, -1));
				} else {	// Moved backward
					SetSelection(doc.ExtendWordSelect(movePos, -1),
					             doc.ExtendWordSelect(originalAnchorPos, 1));
				}
			} else {
				// Continue selecting by line
				int lineMove = LineFromLocation(pt);
				if (lineAnchor < lineMove) {
					SetSelection(doc.LineStart(lineMove + 1),
					             doc.LineStart(lineAnchor));
				} else {
					SetSelection(doc.LineStart(lineAnchor + 1),
					             doc.LineStart(lineMove));
				}
			}
		}
		EnsureCaretVisible();
	} else {
		if (vs.fixedColumnWidth > 0) {	// There is a margin
			if (PointInSelMargin(pt)) {
				wDraw.SetCursor(Window::cursorReverseArrow);
				return; 	// No need to test for selection
			}
		}
		// Display regular (drag) cursor over selection
		if (PointInSelection(pt))
			wDraw.SetCursor(Window::cursorArrow);
		else
			wDraw.SetCursor(Window::cursorText);
	}

}

void Editor::ButtonUp(Point pt, unsigned int curTime, bool ctrl) {
	//Platform::DebugPrintf("ButtonUp %d\n", HaveMouseCapture());
	if (HaveMouseCapture()) {
		wDraw.SetCursor(Window::cursorText);
		ptMouseLast = pt;
		SetMouseCapture(false);
		int newPos = PositionFromLocation(pt);
		newPos = doc.MovePositionOutsideChar(newPos, currentPos - newPos);
		if (inDragDrop) {
			int selStart = SelectionStart();
			int selEnd = SelectionEnd();
			if (selStart < selEnd) {
				if (dragChars && lenDrag) {
					if (ctrl) {
						doc.InsertString(newPos, dragChars, lenDrag);
						SetSelection(newPos, newPos + lenDrag);
					} else if (newPos < selStart) {
						doc.DeleteChars(selStart, lenDrag);
						doc.InsertString(newPos, dragChars, lenDrag);
						SetSelection(newPos, newPos + lenDrag);
					} else if (newPos > selEnd) {
						doc.DeleteChars(selStart, lenDrag);
						newPos -= lenDrag;
						doc.InsertString(newPos, dragChars, lenDrag);
						SetSelection(newPos, newPos + lenDrag);
					} else {
						SetEmptySelection(newPos);
					}
					delete []dragChars;
					dragChars = 0;
					lenDrag = 0;
				}
				selectionType = selChar;
			}
		} else {
			if (selectionType == selChar) {
				SetSelection(newPos);
			}
		}
		lastClickTime = curTime;
		lastClick = pt;
		lastXChosen = pt.x;
		inDragDrop = false;
	}
}

// Called frequently to perform background UI including
// caret blinking and automatic scrolling.
void Editor::Tick() {
	if (HaveMouseCapture()) {
		// Auto scroll
		ButtonMove(ptMouseLast);
	}
	if (caret.period > 0) {
		timer.ticksToWait -= timer.tickSize;
		if (timer.ticksToWait <= 0) {
			caret.on = !caret.on;
			timer.ticksToWait = caret.period;
			InvalidateCaret();
		}
	}
}

Range Editor::RangeFromRectangle(PRectangle rc) {
	PRectangle rcText = GetTextRectangle();
	// TODO: deal with partially contained lines
	int topLineRect = (rc.top - rcText.top) / vs.lineHeight + topLine;
	topLineRect = Platform::Clamp(topLineRect, 0, doc.LinesTotal());
	Position posStart = doc.LineStart(topLineRect);
	int bottomLineRect = (rc.bottom - rcText.top - 1) / vs.lineHeight + topLine;
	bottomLineRect = Platform::Clamp(bottomLineRect, 0, doc.LinesTotal());
	Position posEnd = doc.Length();
	if (bottomLineRect < doc.LinesTotal()) {
		posEnd = doc.LineStart(bottomLineRect+1) - 1;
	}
//Platform::DebugPrintf("RangeFromRectangle %d-%d\n", topLineRect, bottomLineRect);
	return Range(posStart, posEnd);
}

void Editor::CheckForChangeOutsidePaint(Range r) {
	if (paintState == painting) {
		//Platform::DebugPrintf("Checking range in paint %d-%d\n", r.start, r.end);
		if (!r.Valid())
			return;
		PRectangle rcText = GetTextRectangle();
		if (rcPaint.Contains(rcText)) {
			// All of text is within paint rectangle
			return;
		}
		Range rangeVisible = RangeFromRectangle(rcText);
		Range rangePaint = RangeFromRectangle(rcPaint);
		if (rangeVisible.Overlaps(r) && !rangePaint.Contains(r)) {
			// Changed visible text that is not going to be painted so abandon this paint
			paintState = paintAbandoned;
		}
	}
}

char BraceOpposite(char ch) {
	switch (ch) {
                case '(': return ')';
                case ')': return '(';
                case '[': return ']';
                case ']': return '[';
                case '{': return '}';
                case '}': return '{';
                case '<': return '>';
                case '>': return '<';
                default: return '\0';
	}
}

// TODO: should be able to extend styled region to find matching brace
// TODO: may need to make DBCS safe
// so should be moved into Document
int Editor::BraceMatch(int position, int maxReStyle) {
	char chBrace = doc.CharAt(position);
	char chSeek = BraceOpposite(chBrace);
	if (!chSeek)
		return - 1;
	char styBrace = doc.StyleAt(position) & STYLE_MASK;
	int direction = -1;
	if (chBrace == '(' || chBrace == '[' || chBrace == '{' || chBrace == '<')
		direction = 1;
	int depth = 1;
	position = position + direction;
	while ((position >= 0) && (position < doc.Length())) {
		char chAtPos = doc.CharAt(position);
		char styAtPos = doc.StyleAt(position) & STYLE_MASK;
		if ((position > doc.GetEndStyled()) || (styAtPos == styBrace)) {
			if (chAtPos == chBrace)
				depth++;
			if (chAtPos == chSeek)
				depth--;
			if (depth == 0)
				return position;
		}
		position = position + direction;
	}
	return - 1;
}

void Editor::SetBraceHighlight(Position pos0, Position pos1, int matchStyle) {
	if ((pos0 != braces[0]) || (pos1 != braces[1]) || (matchStyle != bracesMatchStyle)) {
		if ((braces[0] != pos0)  || (matchStyle != bracesMatchStyle)) {
			CheckForChangeOutsidePaint(Range(braces[0]));
			CheckForChangeOutsidePaint(Range(pos0));
			braces[0] = pos0;
		}
		if ((braces[1] != pos1)  || (matchStyle != bracesMatchStyle)) {
			CheckForChangeOutsidePaint(Range(braces[1]));
			CheckForChangeOutsidePaint(Range(pos1));
			braces[1] = pos1;
		}
		bracesMatchStyle = matchStyle;
		if (paintState == notPainting) {
			Redraw();
		}
	}
}

LRESULT Editor::WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//Platform::DebugPrintf("S start wnd proc %d %d %d\n",iMessage, wParam, lParam);
	switch (iMessage) {

	case WM_GETTEXT:
		{
			if (lParam == 0)
				return 0;
			char *ptr = reinterpret_cast<char *>(lParam);
			unsigned int iChar = 0;
			for (; iChar < wParam; iChar++)
				ptr[iChar] = doc.CharAt(iChar);
			ptr[iChar] = '\0';
			return iChar;
		}

	case WM_SETTEXT:
		{
			if (lParam == 0)
				return FALSE;
			doc.DeleteChars(0, doc.Length());
			SetEmptySelection(0);
			doc.InsertString(0, reinterpret_cast<char *>(lParam));
			Redraw();
			return TRUE;
		}

	case WM_GETTEXTLENGTH:
		return doc.Length();

	case WM_NOTIFY:
		//Platform::DebugPrintf("S notify %d %d\n", wParam, lParam);
		break;

	case WM_CUT:
		Cut();
		SetLastXChosen();
		break;

	case WM_COPY:
		Copy();
		break;

	case WM_PASTE:
		Paste();
		SetLastXChosen();
		SetScrollBars();
		break;

	case WM_CLEAR:
		//Platform::DebugPrintf("S Clear %d %x %x\n",iMessage, wParam, lParam);
		Clear();
		SetLastXChosen();
		SetScrollBars();
		break;

	case WM_UNDO:
		Undo();
		SetLastXChosen();
		break;

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
		//			EM_SETTARGETDEVICE

	case EM_CANUNDO:
		return doc.CanUndo() ? TRUE : FALSE;

	case EM_UNDO:
		Undo();
		SetScrollBars();
		break;

	case EM_EMPTYUNDOBUFFER:
		doc.DeleteUndoHistory();
		return 0;

	case EM_GETFIRSTVISIBLELINE:
		return topLine;

	case EM_GETLINE: {
			if (lParam == 0)
				return 0;
			int lineStart = doc.LineStart(wParam);
			int lineEnd = doc.LineStart(wParam + 1);
			char *ptr = reinterpret_cast<char *>(lParam);
			ptr[0] = '\0'; 	// If no characters copied have to put a NUL into buffer
			WORD *pBufSize = reinterpret_cast<WORD *>(lParam);
			if (*pBufSize < lineEnd - lineStart)
				return 0;
			int iPlace = 0;
			for (int iChar = lineStart; iChar < lineEnd; iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			return iPlace;
		}

	case EM_GETLINECOUNT:
		if (doc.LinesTotal() == 0)
			return 1;
		else
			return doc.LinesTotal();

	case EM_GETMODIFY:
		return !doc.IsSavePoint();

	case EM_SETMODIFY:
		// Not really supported now that there is the save point stuff
		//doc.isModified = wParam;
		//return doc.isModified;
		return false;

	case EM_GETRECT:
		if (lParam == 0)
			return 0;
		*(reinterpret_cast<PRectangle *>(lParam)) = GetClientRectangle();
		break;

	case EM_GETSEL:
		if (wParam)
			*reinterpret_cast<int *>(wParam) = SelectionStart();
		if (lParam)
			*reinterpret_cast<int *>(lParam) = SelectionEnd();
		return MAKELONG(SelectionStart(), SelectionEnd());

	case EM_EXGETSEL: {
			if (lParam == 0)
				return 0;
			CHARRANGE *pCR = reinterpret_cast<CHARRANGE *>(lParam);
			pCR->cpMin = SelectionStart();
			pCR->cpMax = SelectionEnd();
		}
		break;

	case EM_SETSEL: {
			int nStart = static_cast<int>(wParam);
			int nEnd = static_cast<int>(lParam);
			if (nEnd < 0)
				nEnd = doc.Length();
			if (nStart < 0)
				nStart = nEnd; 	// Remove selection
			SetSelection(nEnd, nStart);
			EnsureCaretVisible();
			Redraw();
		}
		break;

	case EM_EXSETSEL: {
			if (lParam == 0)
				return 0;
			CHARRANGE *pCR = reinterpret_cast<CHARRANGE *>(lParam);
			if (pCR->cpMax == -1) {
				SetSelection(pCR->cpMin, doc.Length());
			} else {
				SetSelection(pCR->cpMin, pCR->cpMax);
			}
			EnsureCaretVisible();
			Redraw();
			return doc.LineFromPosition(SelectionStart());
		}

	case EM_GETSELTEXT: {
			if (lParam == 0)
				return 0;
			char *ptr = reinterpret_cast<char *>(lParam);
			int iPlace = 0;
			int maxChar = SelectionEnd();
			for (int iChar = SelectionStart(); iChar < maxChar; iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			ptr[iPlace] = '\0';
			return iPlace;
		}

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
		if (static_cast<int>(wParam) < 0)
			wParam = SelectionStart();
		return doc.LineFromPosition(wParam);

	case EM_EXLINEFROMCHAR:
		if (static_cast<int>(lParam) < 0)
			lParam = SelectionStart(); 	// Not specified, but probably OK
		return doc.LineFromPosition(lParam);

	case EM_LINEINDEX:
		if (static_cast<int>(wParam) < 0)
			wParam = SelectionStart();
		if (wParam == 0)
			return 0; 	// Even if there is no text, there is a first line that starts at 0
		if (static_cast<int>(wParam) > doc.LinesTotal())
			return - 1;
		//if (wParam > doc.LineFromPosition(doc.Length()))	// Useful test, anyway...
		//	return -1;
		return doc.LineStart(wParam);

	case EM_LINELENGTH:
		{
			if (static_cast<int>(wParam) < 0)	// Who use this anyway?
				return 0; 	// Should be... Too complex to describe here, see MS specs!
			if (static_cast<int>(wParam) > doc.Length())	// Useful test, anyway...
				return 0;
			int line = doc.LineFromPosition(wParam);
			int charsOnLine = 0;
			for (int pos = doc.LineStart(line); pos < doc.LineStart(line + 1); pos++) {
				if ((doc.CharAt(pos) != '\r') && (doc.CharAt(pos) != '\n'))
					charsOnLine++;
			}
			return charsOnLine;
		}

		// Replacement of the old Scintilla interpretation of EM_LINELENGTH
	case SCI_LINELENGTH:
		if ((static_cast<int>(wParam) < 0) ||
		        (static_cast<int>(wParam) > doc.LineFromPosition(doc.Length())))
			return 0;
		return doc.LineStart(wParam + 1) - doc.LineStart(wParam);

	case EM_REPLACESEL: {
			if (lParam == 0)
				return 0;
			doc.BeginUndoAction();
			ClearSelection();
			char *replacement = reinterpret_cast<char *>(lParam);
			doc.InsertString(currentPos, replacement);
			doc.EndUndoAction();
			SetEmptySelection(currentPos + strlen(replacement));
			EnsureCaretVisible();
			Redraw();
		}
		break;

	case EM_LINESCROLL:
		ScrollTo(topLine + lParam);
		HorizontalScrollTo(xOffset + wParam * vs.spaceWidth);
		return TRUE;

	case EM_SCROLLCARET:
		EnsureCaretVisible();
		break;

	case EM_SETREADONLY:
		doc.SetReadOnly(wParam);
		return TRUE;

	case EM_SETRECT:
		break;

	case EM_CANPASTE:
		return 1;

	case EM_CHARFROMPOS: {
			if (lParam == 0)
				return 0;
			Point *ppt = reinterpret_cast<Point *>(lParam);
			int pos = PositionFromLocation(*ppt);
			int line = doc.LineFromPosition(pos);
			return MAKELONG(pos, line);
		}

	case EM_POSFROMCHAR: {
			// The MS specs for this have changed 3 times: using the RichEdit 3 version
			if (wParam == 0)
				return 0;
			Point *ppt = reinterpret_cast<Point *>(wParam);
			if (lParam < 0) {
				*ppt = Point(0, 0);
			} else {
				*ppt = LocationFromPosition(lParam);
			}
			return 0;
		}

	case EM_FINDTEXT:
		return FindText(iMessage, wParam, lParam);

	case EM_FINDTEXTEX:
		return FindText(iMessage, wParam, lParam);

	case EM_GETTEXTRANGE: {
			if (lParam == 0)
				return 0;
			TEXTRANGE *tr = reinterpret_cast<TEXTRANGE *>(lParam);
			int cpMax = tr->chrg.cpMax;
			if (cpMax == -1)
				cpMax = doc.Length();
			int len = cpMax - tr->chrg.cpMin; 	// No -1 as cpMin and cpMax are referring to inter character positions
			doc.GetCharRange(tr->lpstrText, tr->chrg.cpMin, len);
			// Spec says copied text is terminated with a NUL
			tr->lpstrText[len] = '\0';
			return len; 	// Not including NUL
		}

	case EM_SELECTIONTYPE:
		if (currentPos == anchor)
			return SEL_EMPTY;
		else
			return SEL_TEXT;

	case EM_HIDESELECTION:
		hideSelection = wParam;
		Redraw();
		break;

	case EM_FORMATRANGE:
		return FormatRange(wParam, reinterpret_cast<FORMATRANGE *>(lParam));

		// Control specific mesages

	case SCI_ADDTEXT: {
			if (lParam == 0)
				return 0;
			doc.InsertString(CurrentPosition(), reinterpret_cast<char *>(lParam), wParam);
			SetEmptySelection(currentPos + wParam);
			Redraw();
			return 0;
		}

	case SCI_ADDSTYLEDTEXT: {
			if (lParam == 0)
				return 0;
			doc.InsertStyledString(CurrentPosition() * 2, reinterpret_cast<char *>(lParam), wParam);
			SetEmptySelection(currentPos + wParam / 2);
			Redraw();
			return 0;
		}

	case SCI_INSERTTEXT: {
			if (lParam == 0)
				return 0;
			int insertPos = wParam;
			if (static_cast<short>(wParam) == -1)
				insertPos = CurrentPosition();
			int newCurrent = CurrentPosition();
			int newAnchor = anchor;
			char *sz = reinterpret_cast<char *>(lParam);
			doc.InsertString(insertPos, sz);
			if (newCurrent > insertPos)
				newCurrent += strlen(sz);
			if (newAnchor > insertPos)
				newAnchor += strlen(sz);
			SetEmptySelection(newCurrent);
			Redraw();
			return 0;
		}

	case SCI_CLEARALL:
		ClearAll();
		return 0;

	case SCI_SETUNDOCOLLECTION:
		doc.SetUndoCollection(static_cast<enum undoCollectionType>(wParam));
		return 0;

	case SCI_APPENDUNDOSTARTACTION:
		doc.AppendUndoStartAction();
		return 0;

	case SCI_BEGINUNDOACTION:
		doc.BeginUndoAction();
		return 0;

	case SCI_ENDUNDOACTION:
		doc.EndUndoAction();
		return 0;

	case SCI_GETCARETPERIOD:
		return caret.period;

	case SCI_SETCARETPERIOD:
		caret.period = wParam;
		break;

	case SCI_SETWORDCHARS: {
			if (lParam == 0)
				return 0;
			doc.SetWordChars(reinterpret_cast<unsigned char *>(lParam));
		}
		break;

	case SCI_GETLENGTH:
		return doc.Length();

	case SCI_GETCHARAT:
		return doc.CharAt(wParam);

	case SCI_GETCURRENTPOS:
		return currentPos;

	case SCI_GETANCHOR:
		return anchor;

	case SCI_GETSTYLEAT:
		if (static_cast<short>(wParam) >= doc.Length())
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
			if (lParam == 0)
				return 0;
			TEXTRANGE *tr = reinterpret_cast<TEXTRANGE *>(lParam);
			int iPlace = 0;
			for (int iChar = tr->chrg.cpMin; iChar < tr->chrg.cpMax; iChar++) {
				tr->lpstrText[iPlace++] = doc.CharAt(iChar);
				tr->lpstrText[iPlace++] = doc.StyleAt(iChar);
			}
			tr->lpstrText[iPlace] = '\0';
			tr->lpstrText[iPlace + 1] = '\0';
			return iPlace;
		}

	case SCI_CANREDO:
		return doc.CanRedo() ? TRUE : FALSE;

	case SCI_MARKERLINEFROMHANDLE:
		return doc.LineFromHandle(wParam);

	case SCI_MARKERDELETEHANDLE:
		doc.DeleteMarkFromHandle(wParam);
		break;

	case SCI_GETVIEWWS:
		return vs.viewWhitespace;

	case SCI_SETVIEWWS:
		vs.viewWhitespace = wParam;
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
		//Platform::DebugPrintf("SetAnchor %d %d\n", currentPos, anchor);
		break;

	case SCI_GETCURLINE: {
			if (lParam == 0)
				return 0;
			int lineCurrentPos = doc.LineFromPosition(currentPos);
			int lineStart = doc.LineStart(lineCurrentPos);
			unsigned int lineEnd = doc.LineStart(lineCurrentPos + 1);
			char *ptr = reinterpret_cast<char *>(lParam);
			unsigned int iPlace = 0;
			for (unsigned int iChar = lineStart; iChar < lineEnd && iPlace < wParam; iChar++)
				ptr[iPlace++] = doc.CharAt(iChar);
			ptr[iPlace++] = '\0';
			return currentPos - lineStart;
		}

	case SCI_GETENDSTYLED:
		return doc.GetEndStyled();

	case SCI_GETEOLMODE:
		return doc.eolMode;

	case SCI_SETEOLMODE:
		doc.eolMode = wParam;
		break;

	case SCI_STARTSTYLING:
		doc.StartStyling(wParam, lParam);
		break;

	case SCI_SETSTYLING:
		doc.SetStyleFor(wParam, lParam);
		break;

	case SCI_SETSTYLINGEX:   // Specify a complete styling buffer
		if (lParam == 0)
			return 0;
		doc.SetStyles(wParam, reinterpret_cast<char *>(lParam));
		break;

	case SCI_SETMARGINWIDTH:
		if (wParam < 100) {
			vs.selMarginWidth = wParam;
		}
		InvalidateStyleRedraw();
		break;

	case SCI_SETBUFFEREDDRAW:
		bufferedDraw = wParam;
		break;

	case SCI_SETTABWIDTH:
		if (wParam > 0)
			doc.tabInChars = wParam;
		InvalidateStyleRedraw();
		break;

	case SCI_SETCODEPAGE:
		doc.dbcsCodePage = wParam;
		break;

	case SCI_SETLINENUMBERWIDTH:
		if (wParam < 200) {
			vs.lineNumberWidth = wParam;
		}
		InvalidateStyleRedraw();
		break;

	case SCI_SETUSEPALETTE:
		palette.allowRealization = wParam;
		InvalidateStyleRedraw();
		break;

		// Marker definition and setting
	case SCI_MARKERDEFINE:
		if (wParam <= MARKER_MAX)
			vs.markers[wParam].markType = lParam;
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERSETFORE:
		if (wParam <= MARKER_MAX)
			vs.markers[wParam].fore.desired = Colour(lParam);
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERSETBACK:
		if (wParam <= MARKER_MAX)
			vs.markers[wParam].back.desired = Colour(lParam);
		InvalidateStyleData();
		RedrawSelMargin();
		break;
	case SCI_MARKERADD: {
			int markerID = doc.AddMark(wParam, lParam);
			RedrawSelMargin();
			return markerID;
		}

	case SCI_MARKERDELETE:
		doc.DeleteMark(wParam, lParam);
		RedrawSelMargin();
		break;

	case SCI_MARKERDELETEALL:
		doc.DeleteAllMarks(static_cast<int>(wParam));
		RedrawSelMargin();
		break;

	case SCI_MARKERGET:
		return doc.GetMark(wParam);

	case SCI_MARKERNEXT: {
			int lt = doc.LinesTotal();
			for (int iLine = wParam; iLine < lt; iLine++) {
				if ((doc.GetMark(iLine) & lParam) != 0)
					return iLine;
			}
		}
		return - 1;

	case SCI_STYLECLEARALL:
		vs.ClearStyles();
		InvalidateStyleRedraw();
		break;

	case SCI_STYLESETFORE:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].fore.desired = Colour(lParam);
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETBACK:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].back.desired = Colour(lParam);
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETBOLD:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].bold = lParam;
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETITALIC:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].italic = lParam;
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETEOLFILLED:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].eolFilled = lParam;
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETSIZE:
		if (wParam <= STYLE_MAX) {
			vs.styles[wParam].size = lParam;
			InvalidateStyleRedraw();
		}
		break;
	case SCI_STYLESETFONT:
		if (lParam == 0)
			return 0;
		if (wParam <= STYLE_MAX) {
			strcpy(vs.styles[wParam].fontName, reinterpret_cast<char *>(lParam));
			InvalidateStyleRedraw();
		}
		break;
		
	case SCI_STYLERESETDEFAULT:
		vs.ResetDefaultStyle();
		InvalidateStyleRedraw();
		break;
		
	case SCI_SETFORE:
		vs.styles[STYLE_DEFAULT].fore.desired = Colour(wParam);
		InvalidateStyleRedraw();
		break;

	case SCI_SETBACK:
		vs.styles[STYLE_DEFAULT].back.desired = Colour(wParam);
		InvalidateStyleRedraw();
		break;

	case SCI_SETBOLD:
		vs.styles[STYLE_DEFAULT].bold = wParam;
		InvalidateStyleRedraw();
		break;

	case SCI_SETITALIC:
		vs.styles[STYLE_DEFAULT].italic = wParam;
		InvalidateStyleRedraw();
		break;

	case SCI_SETSIZE:
		vs.styles[STYLE_DEFAULT].size = wParam;
		InvalidateStyleRedraw();
		break;

	case SCI_SETFONT:
		if (wParam == 0)
			return 0;
		strcpy(vs.styles[STYLE_DEFAULT].fontName, reinterpret_cast<char *>(wParam));
		InvalidateStyleRedraw();
		break;

	case SCI_SETSELFORE:
		vs.selforeset = wParam;
		vs.selforeground.desired = Colour(lParam);
		InvalidateStyleRedraw();
		break;

	case SCI_SETSELBACK:
		vs.selbackset = wParam;
		vs.selbackground.desired = Colour(lParam);
		InvalidateStyleRedraw();
		break;

	case SCI_SETCARETFORE:
		vs.caretcolour.desired = Colour(wParam);
		InvalidateStyleRedraw();
		break;

	case SCI_ASSIGNCMDKEY:
		kmap.AssignCmdKey(LOWORD(wParam), HIWORD(wParam), lParam);
		break;

	case SCI_CLEARCMDKEY:
		kmap.AssignCmdKey(LOWORD(wParam), HIWORD(wParam), WM_NULL);
		break;

	case SCI_CLEARALLCMDKEYS:
		kmap.Clear();
		break;

	case SCI_INDICSETSTYLE:
		if (wParam <= INDIC_MAX) {
			vs.indicators[wParam].style = lParam;
			InvalidateStyleRedraw();
		}
		break;

	case SCI_INDICGETSTYLE:
		return (wParam <= INDIC_MAX) ? vs.indicators[wParam].style : 0;

	case SCI_INDICSETFORE:
		if (wParam <= INDIC_MAX) {
			vs.indicators[wParam].fore.desired = Colour(lParam);
			InvalidateStyleRedraw();
		}
		break;

	case SCI_INDICGETFORE:
		return (wParam <= INDIC_MAX) ? vs.indicators[wParam].fore.desired.AsLong() : 0;

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
	case SCI_ZOOMIN:
	case SCI_ZOOMOUT:
	case SCI_DELWORDLEFT:
	case SCI_DELWORDRIGHT:
		return KeyCommand(iMessage);

	case SCI_BRACEHIGHLIGHT:
		SetBraceHighlight(static_cast<int>(wParam), lParam, STYLE_BRACELIGHT);
		break;
		
	case SCI_BRACEBADLIGHT:
		SetBraceHighlight(static_cast<int>(wParam), -1, STYLE_BRACEBAD);
		break;

	case SCI_BRACEMATCH:
		// wParam is position of char to find brace for,
		// lParam is maximum amount of text to restyle to find it
		return BraceMatch(wParam, lParam);

	case SCI_GETVIEWEOL:
		return vs.viewEOL;

	case SCI_SETVIEWEOL:
		vs.viewEOL = wParam;
		Redraw();
		break;

	default:
		return DefWndProc(iMessage, wParam, lParam);
	}
	//Platform::DebugPrintf("end wnd proc\n");
	return 0l;
}
