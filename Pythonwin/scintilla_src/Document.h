// Scintilla source code edit control
// Document.h - manages the text of the document
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

struct LineData {
	int startPosition;
	int marker;
	LineData() : startPosition(0), marker(0) {
	}
};

class LineCache {
public:
	enum { growSize = 4000 };
	int lines;
	LineData *linesData;
	int size;

	LineCache();
	~LineCache();
	void Init();

	void Expand(int sizeNew);
	void InsertValue(int pos, int value);
	void SetValue(int pos, int value);
	void Remove(int pos);
	int LineFromPosition(int pos);
};

enum actionType { insertAction, removeAction, startAction };

class Action {
public:
	actionType at;
	int position;
	char *data;
	int lenData;

	Action();
	~Action();
	void Create(actionType at_, int position_=0, char *data_=0, int lenData_=0);
	void Destroy();
};

enum undoCollectionType { undoCollectNone, undoCollectAutoStart, undoCollectManualStart };

// Holder for an expandable array of characters
// Based on article "Data Structures in a Bit-Mapped Text Editor"
// by Wilfred J. Hansen, Byte January 1987, page 183
class Document {
private:
	char *body;
	int size;
	int length;
	int part1len;
	int gaplen;
	char *part2body;
	bool readOnly;

	void GapTo(int position);
	void RoomFor(int insertionLength);

	Action *actions;
	int lenActions;
	int maxAction;
	int currentAction;
	undoCollectionType collectingUndo;
	int savePoint;

	void AppendAction(actionType at, int position, char *data, int length);

	char ByteAt(int position);
	void SetByteAt(int position, char ch);

public:

	Document(int initialLength = 4000);
	~Document();
	char CharAt(int position);
	char StyleAt(int position);
	int ByteLength();
	int Length();
	int Lines();
	int LineStart(int line);
	void InsertString(int position, char *s, int insertLength);
	void InsertCharStyle(int position, char ch, char style);
	void SetStyleAt(int position, char style, char mask=0xff);
	void DeleteChars(int position, int deleteLength);

	bool IsReadOnly();
	void SetReadOnly(bool set);

	void SetSavePoint();
	bool IsSavePoint();

	void SetMark(int line, int marker);
	int GetMark(int line);
	void DeleteAllMarks(int markerNum);

	// Without undo
	void BasicInsertString(int position, char *s, int insertLength);
	void BasicDeleteChars(int position, int deleteLength);

	undoCollectionType SetUndoCollection(undoCollectionType collectUndo);
	bool IsCollectingUndo();
	void AppendUndoStartAction();
	void DeleteUndoHistory();
	int Undo(int *posEarliestChanged=0);
	int Redo(int *posEarliestChanged=0);
	bool CanUndo();
	bool CanRedo();

	LineCache lc;
};

