// Scintilla source code edit control
// AutoComplete.cxx - defines the auto completion list box
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>

#include "Platform.h"

#include "AutoComplete.h"

AutoComplete::AutoComplete() {
	active = false;
	posStart = 0;
	strcpy(stopChars, "");
	separator = ' ';
	cancelAtStartPos = true;
}

AutoComplete::~AutoComplete() {
	lb.Destroy();
}

bool AutoComplete::Active() {
	return active;
}

void AutoComplete::Start(Window &parent, int ctrlID, int position, int startLen_) {
	if (!lb.Created()) {
		lb.Create(parent, ctrlID);
	}
	lb.Clear();
	active = true;
	startLen = startLen_;
	posStart = position;
}

void AutoComplete::SetStopChars(const char *stopChars_) {
	strncpy(stopChars, stopChars_, sizeof(stopChars));
	stopChars[sizeof(stopChars) - 1] = '\0';
}

bool AutoComplete::IsStopChar(char ch) {
	return ch && strchr(stopChars, ch);
}

void AutoComplete::SetSeparator(char separator_) {
	separator = separator_;
}

char AutoComplete::GetSeparator() {
	return separator;
}

void AutoComplete::SetList(const char *list) {
	lb.Clear();
	char *words = new char[strlen(list) + 1];
	if (words) {
		strcpy(words, list);
		char *startword = words;
		int i = 0;
		for (; words && words[i]; i++) {
			if (words[i] == separator) {
				words[i] = '\0';
				lb.Append(startword);
				startword = words + i + 1;
			}
		}
		if (startword) {
			lb.Append(startword);
		}
		delete []words;
	}
	lb.Sort();
}

void AutoComplete::Show() {
	lb.Show();
	lb.Select(0);
}

void AutoComplete::Cancel() {
	if (lb.Created()) {
		lb.Destroy();
		active = false;
	}
}


void AutoComplete::Move(int delta) {
	int count = lb.Length();
	int current = lb.GetSelection();
	current += delta;
	if (current >= count)
		current = count - 1;
	if (current < 0)
		current = 0;
	lb.Select(current);
}

void AutoComplete::Select(const char *word) {
	int pos = lb.Find(word);
	//Platform::DebugPrintf("Autocompleting at <%s> %d\n", wordCurrent, pos);
	if (pos != -1)
		lb.Select(pos);
}

