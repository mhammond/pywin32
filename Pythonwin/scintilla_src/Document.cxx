// Scintilla source code edit control
// Document.cxx - text document that handles notifications, DBCS, styling, words and end of line
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Platform.h"

#include "Scintilla.h"
#include "CellBuffer.h"
#include "Document.h"

Document::Document() {
#ifdef unix
	eolMode = SC_EOL_LF;
#else
	eolMode = SC_EOL_CRLF;
#endif
	dbcsCodePage = 0;
	stylingPos = 0;
	stylingMask = 0;
	for (int ch = 0; ch < 256; ch++) {
		wordchars[ch] = isalnum(ch) || ch == '_';
	}
	endStyled = 0;
	tabInChars = 8;
	watchers = 0;
	lenWatchers = 0;
}

Document::~Document() {
	delete []watchers;
	watchers = 0;
	lenWatchers = 0;
}

int Document::LineStart(int line) {
	return cb.LineStart(line);
}

int Document::LineFromPosition(int pos) {
	return cb.LineFromPosition(pos);
}

int Document::LineEndPosition(int position) {
	int line = LineFromPosition(position);
	if (line == LinesTotal() - 1)
		position = LineStart(line + 1);
	else
		position = LineStart(line + 1) - 1;
	if (position > 0 && (cb.CharAt(position - 1) == '\r' || cb.CharAt(position - 1) == '\n')) {
		position--;
	}
	return position;
}

int Document::VCHomePosition(int position) {
	int line = LineFromPosition(position);
	int startPosition = LineStart(line);
	int endLine = LineStart(line + 1) - 1;
	int startText = startPosition;
	while (startText < endLine && (cb.CharAt(startText) == ' ' || cb.CharAt(startText) == '\t' ) )
		startText++;
	if (position == startText)
		return startPosition;
	else
		return startText;
}

int Document::ClampPositionIntoDocument(int pos) {
	return Platform::Clamp(pos, 0, Length());
}

bool Document::IsCrLf(int pos) {
	if (pos < 0)
		return false;
	if (pos >= (Length() - 1))
		return false;
	return (cb.CharAt(pos) == '\r') && (cb.CharAt(pos + 1) == '\n');
}

bool Document::IsDBCS(int pos) {
#ifdef PLAT_WIN
	if (dbcsCodePage) {
		// Anchor DBCS calculations at start of line because start of line can
		// not be a DBCS trail byte.
		int startLine = pos;
		while (startLine > 0 && cb.CharAt(startLine) != '\r' && cb.CharAt(startLine) != '\n')
			startLine--;
		while (startLine <= pos) {
			if (IsDBCSLeadByteEx(dbcsCodePage, cb.CharAt(startLine))) {
				startLine++;
				if (startLine >= pos)
					return true;
			}
			startLine++;
		}
	}
	return false;
#else
	return false;
#endif
}

// Normalise a position so that it is not halfway through a two byte character.
// This can occur in two situations -
// When lines are terminated with \r\n pairs which should be treated as one character.
// When displaying DBCS text such as Japanese.
// If moving, move the position in the indicated direction.
int Document::MovePositionOutsideChar(int pos, int moveDir) {
	//Platform::DebugPrintf("NoCRLF %d %d\n", pos, moveDir);
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
	if (IsCrLf(pos - 1)) {
		if (moveDir > 0)
			return pos + 1;
		else
			return pos - 1;
	}

	// Not between CR and LF

#ifdef PLAT_WIN
	if (dbcsCodePage) {
		// Anchor DBCS calculations at start of line because start of line can
		// not be a DBCS trail byte.
		int startLine = pos;
		while (startLine > 0 && cb.CharAt(startLine) != '\r' && cb.CharAt(startLine) != '\n')
			startLine--;
		bool atLeadByte = false;
		while (startLine < pos) {
			if (atLeadByte)
				atLeadByte = false;
			else if (IsDBCSLeadByteEx(dbcsCodePage, cb.CharAt(startLine)))
				atLeadByte = true;
			else
				atLeadByte = false;
			startLine++;
			//Platform::DebugPrintf("DBCS %s\n", atlead ? "D" : "-");
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

void Document::ModifiedAt(int pos) {
	if (endStyled > pos)
		endStyled = pos;
}

// Document only modified by gateways DeleteChars, InsertStyledString, Undo, Redo, and SetStyleAt.
// SetStyleAt does not change the persistent state of a document

// Unlike Undo, Redo, and InsertStyledString, the pos argument is a cell number not a char number
void Document::DeleteChars(int pos, int len) {
	if (cb.IsReadOnly())
		NotifyModifyAttempt();
	if (!cb.IsReadOnly()) {
		bool startSavePoint = cb.IsSavePoint();
		cb.DeleteChars(pos*2, len * 2);
		if (startSavePoint && cb.IsCollectingUndo())
			NotifySavePoint(!startSavePoint);
		ModifiedAt(pos);
		NotifyModified();
	}
}

void Document::InsertStyledString(int position, char *s, int insertLength) {
	if (cb.IsReadOnly())
		NotifyModifyAttempt();
	if (!cb.IsReadOnly()) {
		bool startSavePoint = cb.IsSavePoint();
		cb.InsertString(position, s, insertLength);
		if (startSavePoint && cb.IsCollectingUndo())
			NotifySavePoint(!startSavePoint);
		ModifiedAt(position / 2);
		NotifyModified();
	}
}

int Document::Undo() {
	bool startSavePoint = cb.IsSavePoint();
	int earliestMod = Length();
	int newPos = cb.Undo(&earliestMod) / 2;
	ModifiedAt(earliestMod / 2);
	NotifyModified();
	bool endSavePoint = cb.IsSavePoint();
	if (startSavePoint != endSavePoint)
		NotifySavePoint(endSavePoint);
	return newPos;
}

int Document::Redo() {
	bool startSavePoint = cb.IsSavePoint();
	int earliestMod = Length();
	int newPos = cb.Redo(&earliestMod) / 2;
	ModifiedAt(earliestMod / 2);
	NotifyModified();
	bool endSavePoint = cb.IsSavePoint();
	if (startSavePoint != endSavePoint)
		NotifySavePoint(endSavePoint);
	return newPos;
}

void Document::InsertChar(int pos, char ch) {
	char chs[2];
	chs[0] = ch;
	chs[1] = 0;
	InsertStyledString(pos*2, chs, 2);
}

// Insert a null terminated string
void Document::InsertString(int position, const char *s) {
	InsertString(position, s, strlen(s));
}

// Insert a string with a length
void Document::InsertString(int position, const char *s, int insertLength) {
	char *sWithStyle = new char[insertLength * 2];
	if (sWithStyle) {
		for (int i = 0; i < insertLength; i++) {
			sWithStyle[i*2] = s[i];
			sWithStyle[i*2 + 1] = 0;
		}
		InsertStyledString(position*2, sWithStyle, insertLength*2);
		delete []sWithStyle;
	}
}

void Document::DelChar(int pos) {
	if (IsCrLf(pos)) {
		DeleteChars(pos, 2);
	} else if (IsDBCS(pos)) {
		DeleteChars(pos, 2);
	} else if (pos < Length()) {
		DeleteChars(pos, 1);
	}
}

int Document::DelCharBack(int pos) {
	if (pos <= 0) {
		return pos;
	} else if (IsCrLf(pos - 2)) {
		DeleteChars(pos - 2, 2);
		return pos - 2;
	} else if (IsDBCS(pos - 1)) {
		DeleteChars(pos - 2, 2);
		return pos - 2;
	} else {
		DeleteChars(pos - 1, 1);
		return pos - 1;
	}
}

void Document::Indent(bool forwards, int lineBottom, int lineTop) {
	if (forwards) {
		// Indent by a tab
		for (int line = lineBottom; line >= lineTop; line--) {
			InsertChar(LineStart(line), '\t');
		}
	} else {
		// Dedent - suck white space off the front of the line to dedent by equivalent of a tab
		for (int line = lineBottom; line >= lineTop; line--) {
			int ispc = 0;
			while (ispc < tabInChars && cb.CharAt(LineStart(line) + ispc) == ' ')
				ispc++;
			int posStartLine = LineStart(line);
			if (ispc == tabInChars) {
				DeleteChars(posStartLine, ispc);
			} else if (cb.CharAt(posStartLine + ispc) == '\t') {
				DeleteChars(posStartLine, ispc + 1);
			} else {	// Hit a non-white
				DeleteChars(posStartLine, ispc);
			}
		}
	}
}

bool Document::IsWordChar(unsigned char ch) {
	return wordchars[ch];
}

int Document::ExtendWordSelect(int pos, int delta) {
	if (delta < 0) {
		while (pos > 0 && IsWordChar(cb.CharAt(pos - 1)))
			pos--;
	} else {
		while (pos < (Length()) && IsWordChar(cb.CharAt(pos)))
			pos++;
	}
	return pos;
}

int Document::NextWordStart(int pos, int delta) {
	if (delta < 0) {
		while (pos > 0 && (cb.CharAt(pos - 1) == ' ' || cb.CharAt(pos - 1) == '\t'))
			pos--;
		if (isspace(cb.CharAt(pos - 1))) {	// Back up to previous line
			while (pos > 0 && isspace(cb.CharAt(pos - 1)))
				pos--;
		} else {
			bool startAtWordChar = IsWordChar(cb.CharAt(pos - 1));
			while (pos > 0 && !isspace(cb.CharAt(pos - 1)) && (startAtWordChar == IsWordChar(cb.CharAt(pos - 1))))
				pos--;
		}
	} else {
		bool startAtWordChar = IsWordChar(cb.CharAt(pos));
		while (pos < (Length()) && isspace(cb.CharAt(pos)))
			pos++;
		while (pos < (Length()) && !isspace(cb.CharAt(pos)) && (startAtWordChar == IsWordChar(cb.CharAt(pos))))
			pos++;
		while (pos < (Length()) && (cb.CharAt(pos) == ' ' || cb.CharAt(pos) == '\t'))
			pos++;
	}
	return pos;
}

bool Document::IsWordAt(int start, int end) {
	int lengthDoc = Length();
	if (start > 0) {
		char ch = CharAt(start - 1);
		if (IsWordChar(ch))
			return false;
	}
	if (end < lengthDoc - 1) {
		char ch = CharAt(end);
		if (IsWordChar(ch))
			return false;
	}
	return true;
}

long Document::FindText(int minPos, int maxPos, const char *s, bool caseSensitive, bool word) {
	int startPos = ClampPositionIntoDocument(minPos);
	if (IsDBCS(startPos))
		startPos++;
	int endPos = ClampPositionIntoDocument(maxPos);
	if (IsDBCS(endPos))
		endPos++;
	int lengthFind = strlen(s);
	//Platform::DebugPrintf("Find %d %d %s %d\n", startPos, endPos, ft->lpstrText, lengthFind);
	char firstChar = s[0];
	if (!caseSensitive)
		firstChar = toupper(firstChar);
	int maxSearch = endPos - lengthFind + 1;
	for (int pos = startPos; pos < maxSearch; pos++) {
		char ch = CharAt(pos);
		if (caseSensitive) {
			if (ch == firstChar) {
				bool found = true;
				for (int posMatch = 1; posMatch < lengthFind && found; posMatch++) {
					ch = CharAt(pos + posMatch);
					if (ch != s[posMatch])
						found = false;
				}
				if (found) {
					if ((!word) || IsWordAt(pos, pos + lengthFind))
						return pos;
				}
			}
		} else {
			if (toupper(ch) == firstChar) {
				bool found = true;
				for (int posMatch = 1; posMatch < lengthFind && found; posMatch++) {
					ch = CharAt(pos + posMatch);
					if (toupper(ch) != toupper(s[posMatch]))
						found = false;
				}
				if (found) {
					if ((!word) || IsWordAt(pos, pos + lengthFind))
						return pos;
				}
			}
		}
		if (IsDBCS(pos)) {
			pos++;
		}
	}
	//Platform::DebugPrintf("Not found\n");
	return - 1;
}

int Document::LinesTotal() {
	return cb.Lines();
}

void Document::SetWordChars(unsigned char *chars) {
	int ch;
	for (ch = 0; ch < 256; ch++) {
		wordchars[ch] = false;
	}
	if (chars) {
		while (*chars) {
			wordchars[*chars] = true;
			chars++;
		}
	} else {
		for (ch = 0; ch < 256; ch++) {
			wordchars[ch] = isalnum(ch) || ch == '_';
		}
	}
}

void Document::StartStyleSequence() {
	rangeStyleChanged = Range(invalidPosition);
}

void Document::StartStyling(int position, char mask) {
	stylingPos = position;
	stylingMask = mask;
}

void Document::SetStyleFor(int length, char style) {
	if (cb.SetStyleFor(stylingPos, length, style, stylingMask)) {
		rangeStyleChanged = rangeStyleChanged.Extend(stylingPos, stylingPos+length);
	}
	stylingPos += length;
	endStyled = stylingPos;
}

void Document::SetStyles(int length, char *styles) {
	for (int iPos = 0; iPos < length; iPos++, stylingPos++) {
		if (cb.SetStyleAt(stylingPos, styles[iPos], stylingMask)) {
			rangeStyleChanged = rangeStyleChanged.Extend(stylingPos, stylingPos+1);
		}
	}
	endStyled = stylingPos;
}

bool Document::AddWatcher(DocWatcher *watcher, void *userData) {
	for (int i = 0; i < lenWatchers; i++) {
		if ((watchers[i].watcher == watcher) &&
		        (watchers[i].userData == userData))
			return false;
	}
	WatcherWithUserData *pwNew = new WatcherWithUserData[lenWatchers + 1];
	if (!pwNew)
		return false;
	for (int j = 0; j < lenWatchers; j++)
		pwNew[j] = watchers[j];
	pwNew[lenWatchers].watcher = watcher;
	pwNew[lenWatchers].userData = userData;
	delete []watchers;
	watchers = pwNew;
	lenWatchers++;
	return true;
}

bool Document::RemoveWatcher(DocWatcher *watcher, void *userData) {
	for (int i = 0; i < lenWatchers; i++) {
		if ((watchers[i].watcher == watcher) &&
		        (watchers[i].userData == userData)) {
			if (lenWatchers == 1) {
				delete []watchers;
				watchers = 0;
				lenWatchers = 0;
			} else {
				WatcherWithUserData *pwNew = new WatcherWithUserData[lenWatchers];
				if (!pwNew)
					return false;
				for (int j = 0; j < lenWatchers - 1; j++) {
					pwNew[j] = (j < i) ? watchers[j] : watchers[j + 1];
				}
				delete []watchers;
				watchers = pwNew;
				lenWatchers--;
			}
			return true;
		}
	}
	return false;
}

void Document::NotifyModifyAttempt() {
	for (int i = 0; i < lenWatchers; i++) {
		watchers[i].watcher->NotifyModifyAttempt(this, watchers[i].userData);
	}
}

void Document::NotifySavePoint(bool atSavePoint) {
	for (int i = 0; i < lenWatchers; i++) {
		watchers[i].watcher->NotifySavePoint(this, watchers[i].userData, atSavePoint);
	}
}

void Document::NotifyModified() {
	for (int i = 0; i < lenWatchers; i++) {
		watchers[i].watcher->NotifyModified(this, watchers[i].userData);
	}
}
