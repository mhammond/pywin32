// SciTE - Scintilla based Text Editor
// PropSet.h - a java style properties file module
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

class PropSet {
private:
	char **vals;
	int size;
	int used;
public:
	PropSet *superPS;
	PropSet();
	~PropSet();
	void EnsureCanAddEntry();
	void Set(const char *key, const char *val);
	void Set(char *keyval);
	char *Get(const char *key);
	int GetInt(const char *key);
	char *GetWild(const char *keybase, const char *filename);
	char *GetNewExpand(const char *keybase, const char *filename);
	void Clear();
	void Read(const char *filename);
};

