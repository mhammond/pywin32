// SciTE - Scintilla based Text Editor
// PropSet.h - a java style properties file module
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PROPSET_H
#define PROPSET_H

bool EqualCaseInsensitive(const char *a, const char *b);

// Define another string class.
// While it would be 'better' to use std::string, that doubles the executable size.

inline char *StringDup(const char *s, int len=-1) {
	if (!s)
		return 0;
    if (len == -1)
        len = strlen(s);
	char *sNew = new char[len + 1];
    if (sNew) {
		strncpy(sNew, s, len);
        sNew[len] = '\0';
    }
	return sNew;
}

class SString {
	char *s;
public:
	SString() {
		s = 0;
	}
	SString(const SString &source) {
		s = StringDup(source.s);
	}
	SString(const char *s_) {
		s = StringDup(s_);
	}
	SString(int i) {
		char number[100];
		sprintf(number, "%0d", i);
		//itoa(i, number, 10);
		s = StringDup(number);
	}
	~SString() {
		delete []s;
		s = 0;
	}
	SString &operator=(const SString &source) {
		if (this != &source) {
			delete []s;
			s = StringDup(source.s);
		}
		return *this;
	}
	bool operator==(const SString &other) const {
		if ((s == 0) && (other.s == 0))
			return true;
		if ((s == 0) || (other.s == 0))
			return false;
		return strcmp(s, other.s) == 0;
	}
	bool operator==(const char *sother) const {
		if ((s == 0) && (sother == 0))
			return true;
		if ((s == 0) || (sother == 0))
			return false;
		return strcmp(s, sother) == 0;
	}
	const char *c_str() const {
		if (s)
			return s;
		else
			return "";
	}
	int length() const {
		if (s)
			return strlen(s);
		else
			return 0;
	}
	char operator[](int i) const {
		if (s)
			return s[i];
		else
			return '\0';
	}
	SString &operator +=(const char *sother) {
		int len = length();
		int lenOther = strlen(sother);
		char *sNew = new char[len + lenOther + 1];
		if (sNew) {
			if (s)
				memcpy(sNew, s, len);
			memcpy(sNew + len, sother, lenOther);
			sNew[len + lenOther] = '\0';
			delete []s;
			s = sNew;
		}
		return *this;
	}
	int value() const {
		if (s)
			return atoi(s);
		else 
			return 0;
	}
};

struct Property {
    unsigned int hash;
	char *key;
    char *val;
    Property *next;
    Property() : hash(0), key(0), val(0), next(0) {}
};

class PropSet {
private:
    enum { hashRoots=31 };
    Property *props[hashRoots];
public:
	PropSet *superPS;
	PropSet();
	~PropSet();
	void Set(const char *key, const char *val);
	void Set(char *keyval);
	SString Get(const char *key);
    SString GetExpanded(const char *key);
    SString Expand(const char *withvars);
	int GetInt(const char *key, int defaultValue=0);
	SString GetWild(const char *keybase, const char *filename);
	SString GetNewExpand(const char *keybase, const char *filename);
	void Clear();
	void ReadFromMemory(const char *data, int len, const char *directoryForImports=0);
	void Read(const char *filename, const char *directoryForImports);
};

class WordList {
public:
	// Each word contains at least one character - a empty word acts as sentinal at the end.
	char **words;
	char *list;
	int len;
	bool onlyLineEnds;	// Delimited by any white space or only line ends
	int starts[256];
	WordList(bool onlyLineEnds_ = false) : 
		words(0), list(0), len(0), onlyLineEnds(onlyLineEnds_) {}
	~WordList() { Clear(); }
	operator bool() { return (list && list[0]) ? true : false; }
	const char *operator[](int ind) { return words[ind]; }
	void Clear();
	void Set(const char *s);
	char *Allocate(int size);
	void SetFromAllocated();
	bool InList(const char *s);
};

#endif
