// Scintilla source code edit control
// Document.h - text document that handles notifications, DBCS, styling, words and end of line
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef DOCUMENT_H
#define DOCUMENT_H

// A Position is a position within a document between two characters or at the beginning or end.
// Sometimes used as a character index where it identifies the character after the position.
typedef int Position;
const Position invalidPosition = -1;

// The range class represents a range of text in a document.
// The two values are not sorted as one end may be more significant than the other
// as is the case for the selection where the end position is the position of the caret.
// If either position is invalidPosition then the range is invalid and most operations will fail.
class Range {
public:
	Position start;
	Position end;
	
        Range(Position pos=0) : 
		start(pos), end(pos) {
        };
        Range(Position start_, Position end_) : 
		start(start_), end(end_) {
        };
        
        bool Valid() const {
        	return (start != invalidPosition) && (end != invalidPosition);
        }
        
        // Return a range extended to include posStart and posEnd.
        // posStart <= posEnd.
        Range Extend(Position posStart, Position posEnd) const {
        	if (!Valid())
			return Range(posStart, posEnd);
        	if (start < end) {
        		if (posStart < start) 
        			return Range(posStart, end);
        		else if (posEnd > end)
        			return Range(start, posEnd);
        		else
        			return *this;
        	} else {
        		if (posEnd > start) 
        			return Range(end, posEnd);
        		else if (posStart < end)
        			return Range(posStart, end);
        		else
        			return *this;
        	}
        }
        
        bool Contains(Position pos) const {
        	if (start < end) {
        		return (pos >= start && pos <= end);
        	} else {
        		return (pos <= start && pos >= end);
        	}
        }
        
        bool Contains(Range other) const {
        	return Contains(other.start) && Contains(other.end);
        }
        
        bool Overlaps(Range other) const {
        	return 
			Contains(other.start) ||
			Contains(other.end) ||
			other.Contains(start) ||
			other.Contains(end);
        }
};

class DocWatcher;

// Used internally by Document
class WatcherWithUserData {
public:
	DocWatcher *watcher;
	void *userData;
	WatcherWithUserData() {
		watcher = 0;
		userData = 0;
	}
};

class Document {

	CellBuffer cb;
	bool wordchars[256];
	bool modified;
	int stylingPos;
	int stylingMask;
	int endStyled;
	Range rangeStyleChanged;
	
	WatcherWithUserData *watchers;
	int lenWatchers;
	
public:
	int eolMode;
	int dbcsCodePage;
	int tabInChars;
	
	Document();
	virtual ~Document();
	
	int LineFromPosition(int pos);
	int ClampPositionIntoDocument(int pos);
	bool IsCrLf(int pos);
	int MovePositionOutsideChar(int pos, int moveDir);

	// Gateways to modifying document
	void DeleteChars(int pos, int len);
	void InsertStyledString(int position, char *s, int insertLength);
	int Undo();
	int Redo();
	bool CanUndo() { return cb.CanUndo(); }
	bool CanRedo() { return cb.CanRedo(); }
	void DeleteUndoHistory() { cb.DeleteUndoHistory(); }
	undoCollectionType SetUndoCollection(undoCollectionType collectUndo) {
		return cb.SetUndoCollection(collectUndo);
	}
	void AppendUndoStartAction() { cb.AppendUndoStartAction(); }
	void BeginUndoAction() { cb.BeginUndoAction(); }
	void EndUndoAction() { cb.EndUndoAction(); }
	void SetSavePoint() { cb.SetSavePoint(); }
	bool IsSavePoint() { return cb.IsSavePoint(); }
	void Indent(bool forwards, int lineBottom, int lineTop);
	void SetReadOnly(bool set) { cb.SetReadOnly(set); }

	void InsertChar(int pos, char ch);
	void InsertString(int position, const char *s);
	void InsertString(int position, const char *s, int insertLength);
	void DelChar(int pos);
	int DelCharBack(int pos);

	char CharAt(int position) { return cb.CharAt(position); }
	void GetCharRange(char *buffer, int position, int lengthRetrieve) {
		cb.GetCharRange(buffer, position, lengthRetrieve);
	}
	char StyleAt(int position) { return cb.StyleAt(position); }
	int GetMark(int line) { return cb.GetMark(line); }
	int AddMark(int line, int markerNum) { return cb.AddMark(line, markerNum); }
	void DeleteMark(int line, int markerNum) { cb.DeleteMark(line, markerNum); }
	void DeleteMarkFromHandle(int markerHandle) { cb.DeleteMarkFromHandle(markerHandle); }
	void DeleteAllMarks(int markerNum) { cb.DeleteAllMarks(markerNum); }
	int LineFromHandle(int markerHandle) { return cb.LineFromHandle(markerHandle); }
	int LineStart(int line);
	int LineEndPosition(int position);
	int VCHomePosition(int position);

	void Indent(bool forwards);
	int ExtendWordSelect(int pos, int delta);
	int NextWordStart(int pos, int delta);
	int Length() { return cb.Length(); }
	long FindText(int minPos, int maxPos, const char *s, bool caseSensitive, bool word);
	long FindText(WORD iMessage,WPARAM wParam,LPARAM lParam);
	int LinesTotal();
	
	void SetWordChars(unsigned char *chars);
	void StartStyleSequence();
	void StartStyling(int position, char mask);
	void SetStyleFor(int length, char style);
	void SetStyles(int length, char *styles);
	int GetEndStyled() { return endStyled; }
	Range StyleChanged() { return rangeStyleChanged; };
	
	bool AddWatcher(DocWatcher *watcher, void *userData);
	bool RemoveWatcher(DocWatcher *watcher, void *userData);
	
private:
	bool IsDBCS(int pos);
	bool IsWordChar(unsigned char ch);
	bool IsWordAt(int start, int end);
	void ModifiedAt(int pos);
		
	void NotifyModifyAttempt();
	void NotifySavePoint(bool atSavePoint);
	void NotifyModified();
};

// A class that wants to receive notifications from a Document must be derived from DocWatcher 
// and implement the notification methods. It can then be added to the watcher list with AddWatcher.
class DocWatcher {
public:
	virtual void NotifyModifyAttempt(Document *doc, void *userData) = 0;
	virtual void NotifySavePoint(Document *doc, void *userData, bool atSavePoint) = 0;
	virtual void NotifyModified(Document *doc, void *userData) = 0;
};

#endif
