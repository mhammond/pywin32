// Scintilla source code edit control
// Document.cc - manages the text of the document
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef GTK
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Document.h"

//#define TRACE

static void dprintf(char *format, ...) {
#ifdef TRACE
	char buffer[2000];
	char *pArguments = (char *) & format + sizeof(format);
	vsprintf(buffer,format,pArguments);
#ifdef GTK
	printf("%s",buffer);
#else
	OutputDebugString(buffer);
#endif
#endif
}

LineCache::LineCache() {
	linesData = 0;
	Init();
}

LineCache::~LineCache() {
	delete []linesData;
	linesData = 0;
}

void LineCache::Init() {
	delete []linesData;
	linesData = new LineData[growSize];
	size = growSize;
	lines = 1;
}

void LineCache::Expand(int sizeNew) {
	LineData *linesDataNew = new LineData[sizeNew];
	if (linesDataNew) {
		for (int i=0;i<size;i++)
			linesDataNew[i] = linesData[i];
		delete []linesData;
		linesData = linesDataNew;
		size = sizeNew;
	} else {
		dprintf("No memory available\n");
		// TODO: Blow up
	}
}

void LineCache::InsertValue(int pos, int value) {
	dprintf("InsertValue[%d] = %d\n", pos, value);
	if ((lines+2) >= size) {
		Expand(size + growSize);
	}
	lines++;
	for (int i=lines+1;i>pos;i--) {
		linesData[i] = linesData[i-1];
	}
	linesData[pos].startPosition = value;
	linesData[pos].marker = 0;
}

void LineCache::SetValue(int pos, int value) {
	dprintf("SetValue[%d] = %d\n", pos, value);
	if ((pos+2) >= size) {
		//dprintf("Resize %d %d\n", size,pos);
		Expand(pos + growSize);
		//dprintf("end Resize %d %d\n", size,pos);
		lines = pos;
	}
	linesData[pos].startPosition = value;
}

void LineCache::Remove(int pos) {
	dprintf("Remove %d\n", pos);
	// Retain the markers from the deleted line by oring them into the previous line
	if (pos > 0) {
		linesData[pos-1].marker |= linesData[pos].marker;
	}
	for (int i=pos;i<lines;i++) {
		linesData[i] = linesData[i+1];
	}
	lines--;
}

int LineCache::LineFromPosition(int pos) {
//dprintf("LineFromPostion %d lines=%d end = %d\n", pos, lines, linesData[lines].startPosition);
	if (lines == 0)
		return 0;
//dprintf("LineFromPosition %d\n", pos);
	if (pos >= linesData[lines].startPosition)
		return lines - 1;
	int lower = 0;
	int upper = lines;
	int middle = 0;
	do {
		middle = (upper + lower + 1) / 2;	// Round high
		if (pos < linesData[middle].startPosition) {
			upper = middle - 1;
		} else {
			lower = middle;
		}
	} while (lower < upper);
//dprintf("LineFromPostion %d %d %d\n", pos, lower, linesData[lower].startPosition, linesData[lower > 1 ? lower - 1 : 0].startPosition);
	return lower;
}

Action::Action() {
	at = startAction;
	position = 0;
	data = 0;
	lenData = 0;
}

Action::~Action() {
}

void Action::Destroy() {
	delete []data;
	data = 0;
}

void Action::Create(actionType at_, int position_, char *data_, int lenData_) {
	delete []data;
	position = position_;
	at = at_;
	data = data_;
	lenData = lenData_;
}

Document::Document(int initialLength) {
	body = new char[initialLength];
	size = initialLength;
	length = 0;
	part1len = 0;
	gaplen = initialLength;
	part2body = body + gaplen;

	actions = new Action[30000];
	lenActions = 30000;
	maxAction = 0;
	currentAction = 0;
	actions[currentAction].Create(startAction);

	readOnly = false;
}

Document::~Document() {
	delete []body;
	body = 0;

#ifdef NEED_UNDO_LOG
	FILE *fp = fopen("Log.log", "wt");
	fprintf(fp,"Max = %3d\n", maxAction);
	fprintf(fp,"Current = %3d\n", currentAction);
	for (int i=0;i<maxAction;i++) {
		fprintf(fp,"%3d ", i);
		if (actions[i].at == startAction) {
			fprintf(fp,"=");
		} else if (actions[i].at == removeAction) {
			fprintf(fp,"- %4d %4d ", actions[i].position, actions[i].lenData);
			for (int j=0; j<20&&j<actions[i].lenData;j++)
				fprintf(fp,"%c", actions[i].data[j]);
		} else {
			fprintf(fp,"+ %4d %4d ", actions[i].position, actions[i].lenData);
			for (int j=0; j<20&&j<actions[i].lenData;j++)
				fprintf(fp,"%c", actions[i].data[j]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
#endif
}

void Document::GapTo(int position) {
	if (position == part1len)
		return;
	if (position < part1len) {
		int diff = part1len - position;
		//dprintf("Move gap backwards to %d diff = %d part1len=%d length=%d \n", position,diff, part1len, length);
		for (int i=0;i<diff;i++)
			body[part1len + gaplen - i - 1] = body[part1len - i - 1];
	} else {	// position > part1len
		int diff = position - part1len;
		//dprintf("Move gap forwards to %d diff =%d\n", position,diff);
		for (int i=0;i<diff;i++)
			body[part1len + i] = body[part1len + gaplen + i];
	}
	part1len = position;
	part2body = body + gaplen;
}

void Document::RoomFor(int insertionLength) {
	//dprintf("need room %d %d\n", gaplen, insertionLength);
	if (gaplen <= insertionLength) {
		//dprintf("need room %d %d\n", gaplen, insertionLength);
		GapTo(length);
		int newSize = size + insertionLength + 4000;
		//dprintf("moved gap %d\n", newSize);
		char *newBody = new char[newSize];
		memcpy(newBody, body, size);
		delete []body;
		body = newBody;
		gaplen += newSize - size;
		part2body = body + gaplen;
		size = newSize;
		//dprintf("end need room %d %d - size=%d length=%d\n", gaplen, insertionLength,size,length);
	}
}

// To make it easier to write code that uses ByteAt, a position outside the range of the buffer
// can be retrieved. All characters outside the range have the value '\0'.
char Document::ByteAt(int position) {

	if (position < 0) {
		//dprintf("Bad position %d\n",position);
		return '\0';
	}
	//if (position >= length + 10) {
	//	dprintf("Very Bad position %d of %d\n",position,length);
		//char sz[30];
		//gets(sz);
	//	exit(3);
	//}
	if (position >= length) {
		//dprintf("Bad position %d of %d\n",position,length);
		//char sz[30];
		//gets(sz);
		return '\0';
	}

	if (position < part1len) {
		return body[position];
	} else {
		return part2body[position];
	}
}

void Document::SetByteAt(int position, char ch) {

	if (position < 0) {
		dprintf("Bad position %d\n",position);
		return;
	}
	if (position >= length + 11) {
		dprintf("Very Bad position %d of %d\n",position,length);
		//exit(2);
		return;
	}
	if (position >= length) {
		dprintf("Bad position %d of %d\n",position,length);
		return;
	}

	if (position < part1len) {
		body[position] = ch;
	} else {
		part2body[position] = ch;
	}
}

char Document::CharAt(int position) {
	return ByteAt(position*2);
}

char Document::StyleAt(int position) {
	return ByteAt(position*2 + 1);
}

void Document::InsertString(int position, char *s, int insertLength) {
	// InsertString and DeleteChars are the bottleneck though which all changes occur
	if (!readOnly) {
		if (collectingUndo) {
			// Save into the undo/redo stack, but only the characters - not the formatting
			// This takes up about half load time
			char *data = new char[insertLength/2];
			for (int i=0;i<insertLength/2;i++) {
				data[i] = s[i * 2];
			}
			AppendAction(insertAction, position, data, insertLength/2);
		}

		BasicInsertString(position, s, insertLength);
	}
}

void Document::InsertCharStyle(int position, char ch, char style) {
	char s[2];
	s[0] = ch;
	s[1] = style;
	InsertString(position*2, s,2);
}

void Document::SetStyleAt(int position, char style, char mask) {
	SetByteAt(position*2 + 1, (ByteAt(position*2 + 1) & ~mask) | style);
}

void Document::AppendAction(actionType at, int position, char *data, int length) {
	//dprintf("%% %d action %d %d %d\n", at, position, length, currentAction);
	if (currentAction >= 2) {
		// See if current action can be coalesced into previous action
		// Will work if both are inserts or deletes and position is same or two different
		if ((at != actions[currentAction-1].at) || (abs(position-actions[currentAction-1].position) > 2)) {
			currentAction++;
		} else if (currentAction == savePoint) {
			currentAction++;
		}
	} else {
		currentAction++;
	}
	actions[currentAction].Create(at, position, data, length);
	if (collectingUndo==undoCollectAutoStart) {
		currentAction++;
		actions[currentAction].Create(startAction);
	}
	maxAction = currentAction;
}

void Document::DeleteChars(int position, int deleteLength) {
	// InsertString and DeleteChars are the bottleneck though which all changes occur
	if (!readOnly) {
		if (collectingUndo) {
			// Save into the undo/redo stack, but only the characters - not the formatting
			char *data = new char[deleteLength/2];
			for (int i=0;i<deleteLength/2;i++) {
				data[i] = ByteAt(position + i * 2);
			}
			AppendAction(removeAction, position, data, deleteLength/2);
		}

		BasicDeleteChars(position, deleteLength);
	}
}

int Document::ByteLength() {
	return length;
}

int Document::Length() {
	return ByteLength() / 2;
}

int Document::Lines() {
//dprintf("Lines = %d\n", lc.lines);
	return lc.lines;
}

int Document::LineStart(int line) {
	return lc.linesData[line].startPosition;
}

bool Document::IsReadOnly() {
	return readOnly;
}

void Document::SetReadOnly(bool set) {
	readOnly = set;
}

void Document::SetSavePoint() {
	savePoint = currentAction;
}

bool Document::IsSavePoint() {
	return savePoint == currentAction;
}

void Document::SetMark(int line, int marker) {
	if ((line >= 0) && (line < lc.lines))
		lc.linesData[line].marker = marker;
}

int Document::GetMark(int line) {
	if ((line >= 0) && (line < lc.lines))
		return lc.linesData[line].marker;
	return 0;
}

void Document::DeleteAllMarks(int markerNum) {
	for (int line=0; line<lc.lines; line++) {
		if (markerNum == -1) {
			lc.linesData[line].marker = 0;
		} else {
			lc.linesData[line].marker &= ~(1 << markerNum);
		}
	}
}

// Without undo

void Document::BasicInsertString(int position, char *s, int insertLength) {
//dprintf("Inserting at %d for %d\n", position, insertLength);
	if (insertLength == 0)
		return;
	RoomFor(insertLength);
	GapTo(position);

	memcpy(body + part1len, s, insertLength);
	length += insertLength;
	part1len += insertLength;
	gaplen -= insertLength;
	part2body = body + gaplen;

	int lineInsert = lc.LineFromPosition(position/2) + 1;
	// Point all the lines after the insertion point further along in the buffer
	for (int lineAfter = lineInsert; lineAfter <= lc.lines; lineAfter++) {
		lc.linesData[lineAfter].startPosition += insertLength / 2;
	}
	char chPrev = ' ';
	if ((position-2) >= 0)
		chPrev = ByteAt(position - 2);
	char chAfter = ' ';
	if ((position + insertLength) < length)
		chAfter = ByteAt(position + insertLength);
	if (chPrev == '\r' && chAfter == '\n') {
//dprintf("Splitting a crlf pair at %d\n", lineInsert);
		// Splitting up a crlf pair at position
		lc.InsertValue(lineInsert,position/2);
		lineInsert++;
	}
	char ch = ' ';
	for (int i=0;i<insertLength;i+=2) {
		ch = s[i];
		if (ch == '\r') {
//dprintf("Inserting cr at %d\n", lineInsert);
			lc.InsertValue(lineInsert,(position + i)/2+1);
			lineInsert++;
		} else if (ch == '\n') {
			if (chPrev == '\r') {
//dprintf("Patching cr before lf at %d\n", lineInsert-1);
				// Patch up what was end of line
				lc.SetValue(lineInsert-1,(position + i)/2+1);
			} else {
//dprintf("Inserting lf at %d\n", lineInsert);
				lc.InsertValue(lineInsert,(position + i)/2+1);
				lineInsert++;
			}
		}
		chPrev = ch;
	}
	// Joining two lines where last insertion is cr and following text starts with lf
	if (chAfter == '\n') {
		if (ch == '\r') {
//dprintf("Joining cr before lf at %d\n", lineInsert-1);
			// End of line already in buffer so drop the newly created one
			lc.Remove(lineInsert-1);
		}
	}
}

void Document::BasicDeleteChars(int position, int deleteLength) {
//dprintf("Deleting at %d for %d\n", position, deleteLength);
	if (deleteLength == 0)
		return;

	if ((position == 0) && (deleteLength == length)) {
		// If whole buffer is being deleted, faster to reinitialise lines data
		// than to delete each line.
		//printf("Whole buffer being deleted\n");
		lc.Init();
	} else {
		// Have to fix up line positions before doing deletion as looking at text in buffer 
		// to work out which lines have been removed
	
		int lineRemove = lc.LineFromPosition(position/2) + 1;
		// Point all the lines after the insertion point further along in the buffer
		for (int lineAfter = lineRemove; lineAfter <= lc.lines; lineAfter++) {
			lc.linesData[lineAfter].startPosition -= deleteLength / 2;
		}
		char chPrev = ' ';
		if (position >= 2)
			chPrev = ByteAt(position - 2);
		char chBefore = chPrev;
		char chNext = ' ';
		if (position < length)
			chNext = ByteAt(position);
		bool ignoreNL = false;
		if (chPrev == '\r' && chNext == '\n') {
	//dprintf("Deleting lf after cr, move line end to cr at %d\n", lineRemove);
			// Move back one
			lc.SetValue(lineRemove,position/2);
			lineRemove++;
			ignoreNL = true;	// First \n is not real deletion
		}
		char ch = chNext;
		for (int i=0;i<deleteLength;i+=2) {
			chNext = ' ';
			if ((position + i + 2) < length)
				chNext = ByteAt(position + i +2);
	//dprintf("Deleting %d %x\n", i, ch);
			if (ch == '\r') {
				if (chNext != '\n') {
	//dprintf("Removing cr end of line\n");
					lc.Remove(lineRemove);
				}
			} else if ((ch == '\n') && !ignoreNL) {
	//dprintf("Removing lf end of line\n");
				lc.Remove(lineRemove);
				ignoreNL = false;	// Further \n are not real deletions
			}
			chPrev = ch;
			ch = chNext;
		}
		// May have to fix up end if last deletion causes cr to be next to lf
		// or removes one of a crlf pair
		char chAfter = ' ';
		if ((position + deleteLength) < length)
			chAfter = ByteAt(position + deleteLength);
		if (chBefore == '\r' && chAfter == '\n') {
	//d printf("Joining cr before lf at %d\n", lineRemove);
			// Using lineRemove-1 as cr ended line before start of deletion
			lc.Remove(lineRemove-1);
			lc.SetValue(lineRemove-1,position/2+1);
		}
	}
	GapTo(position);
	length -= deleteLength;
	gaplen += deleteLength;
	part2body = body + gaplen;
}

undoCollectionType Document::SetUndoCollection(undoCollectionType collectUndo) {
	collectingUndo = collectUndo;
	return collectingUndo;
}

bool Document::IsCollectingUndo() {
	return collectingUndo;
}

void Document::AppendUndoStartAction() {
	if (actions[currentAction].at != startAction) {
		currentAction++;
		actions[currentAction].Create(startAction);
		maxAction = currentAction;
	}
}

void Document::DeleteUndoHistory() {
	for (int i=1;i<maxAction;i++)
		actions[i].Destroy();
	maxAction = 0;
	currentAction = 0;
}

int Document::Undo(int *posEarliestChanged) {
	dprintf("Undoing from %d\n", currentAction);
	int retPosition = 0;	// Where the cursor should be after return
	int changedPosition = 0;	// Earliest byte modified
	if (posEarliestChanged)
		*posEarliestChanged = length;
	if (actions[currentAction].at == startAction && currentAction > 0)
		currentAction--;
	while (actions[currentAction].at != startAction && currentAction > 0) {
		if (actions[currentAction].at == insertAction) {
			BasicDeleteChars(actions[currentAction].position, actions[currentAction].lenData*2);
			retPosition = actions[currentAction].position;
		} else if (actions[currentAction].at == removeAction) {
			char *styledData = new char[actions[currentAction].lenData*2];
			memset(styledData, 0, actions[currentAction].lenData*2);
			for (int i=0;i<actions[currentAction].lenData;i++)
				styledData[i*2] = actions[currentAction].data[i];
			BasicInsertString(actions[currentAction].position, styledData, actions[currentAction].lenData*2);
			delete []styledData;
			retPosition = actions[currentAction].position + actions[currentAction].lenData*2;
		}
		changedPosition = actions[currentAction].position;
		if (posEarliestChanged && (*posEarliestChanged > changedPosition))
			*posEarliestChanged = changedPosition;
		currentAction--;
	}
	return retPosition;
}

int Document::Redo(int *posEarliestChanged) {
	int retPosition = 0;	// Where the cursor should be after return
	int changedPosition = 0;	// Earliest byte modified
	if (posEarliestChanged)
		*posEarliestChanged = length;
	if (actions[currentAction].at == startAction && currentAction < maxAction)
		currentAction++;
	while (actions[currentAction].at != startAction && currentAction < maxAction) {
		if (actions[currentAction].at == insertAction) {
			char *styledData = new char[actions[currentAction].lenData*2];
			memset(styledData, 0, actions[currentAction].lenData*2);
			for (int i=0;i<actions[currentAction].lenData;i++)
				styledData[i*2] = actions[currentAction].data[i];
			BasicInsertString(actions[currentAction].position, styledData, actions[currentAction].lenData*2);
			delete []styledData;
			retPosition = actions[currentAction].position + actions[currentAction].lenData*2;
		} else if (actions[currentAction].at == removeAction) {
			BasicDeleteChars(actions[currentAction].position, actions[currentAction].lenData*2);
			retPosition = actions[currentAction].position;
		}
		changedPosition = actions[currentAction].position;
		if (posEarliestChanged && (*posEarliestChanged > changedPosition))
			*posEarliestChanged = changedPosition;
		currentAction++;
	}
	return retPosition;
}

bool Document::CanUndo() {
	//dprintf("Can Undo?\n");
	return (!readOnly) && ((currentAction > 0) && (maxAction > 0));
}

bool Document::CanRedo() {
	return (!readOnly) && (maxAction > currentAction);
}
