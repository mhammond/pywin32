// SciTE - Scintilla based Text Editor
// PropSet.cxx - a java style properties file module
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// Maintain a dictionary of properties

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "Platform.h"

#include "PropSet.h"

bool EqualCaseInsensitive(const char *a, const char *b) {
#if PLAT_GTK
	return 0 == strcasecmp(a, b);
#elif PLAT_WIN
	return 0 == stricmp(a, b);
#elif PLAT_WX
	return 0 == wxStricmp(a, b);
#endif
}

inline unsigned int HashString(const char *s) {
    unsigned int ret = 0;
    while (*s) {
        ret <<= 4;
        ret ^= *s;
        s++;
    }
    return ret;
}

// Get a line of input. If end of line escaped with '\\' then continue reading.
static bool GetFullLine(const char *&fpc, int &lenData, char *s, int len) {
	bool continuation = true;
	s[0] = '\0';
	while ((len > 1) && lenData > 0) {
		char ch = *fpc;
		fpc++;
		lenData--;
		if ((ch == '\r') || (ch == '\n')) {
			if (!continuation) {
				if ((lenData > 0) && (ch == '\r') && ((*fpc) == '\n')) {
					// munch the second half of a crlf
					fpc++;
					lenData--;
				}
				*s = '\0';
				return true;
			}
		} else if ((ch == '\\') && (lenData > 0) && ((*fpc == '\r') || (*fpc == '\n'))) {
			continuation = true;
		} else {
			continuation = false;
			*s++ = ch;
			*s = '\0';
			len--;
		}
	}
	return false;
}

PropSet::PropSet() {
	superPS = 0;
    for (int root=0; root < hashRoots; root++)
        props[root] = 0;
}

PropSet::~PropSet() {
	superPS = 0;
	Clear();
}

void PropSet::Set(const char *key, const char *val) {
    unsigned int hash = HashString(key);
	for (Property *p=props[hash % hashRoots]; p; p=p->next) {
		if ((hash == p->hash) && (0 == strcmp(p->key, key))) {
			// Replace current value
			delete [](p->val);
			p->val = StringDup(val);
			return;
		}
	}
	// Not found
    Property *pNew = new Property;
    if (pNew) {
        pNew->hash = HashString(key);
	    pNew->key = StringDup(key);
	    pNew->val = StringDup(val);
	    pNew->next = props[hash % hashRoots];
        props[hash % hashRoots] = pNew;
    }
}

void PropSet::Set(char *keyval) {
    while (isspace(*keyval))
        keyval++;
	char *eqat = strchr(keyval, '=');
	if (eqat) {
		*eqat = '\0';
		Set(keyval, eqat + 1);
		*eqat = '=';
	}
}

SString PropSet::Get(const char *key) {
    unsigned int hash = HashString(key);
	for (Property *p=props[hash % hashRoots]; p; p=p->next) {
		if ((hash == p->hash) && (0 == strcmp(p->key, key))) {
            return p->val;
        }
    }
	if (superPS) {
		// Failed here, so try in base property set
		return superPS->Get(key);
	} else {
		return "";
	}
}

SString PropSet::GetExpanded(const char *key) {
    SString val = Get(key);
    return Expand(val.c_str());
}

SString PropSet::Expand(const char *withvars) {
	char *base = StringDup(withvars);
	char *cpvar = strstr(base, "$(");
	while (cpvar) {
		char *cpendvar = strchr(cpvar, ')');
		if (cpendvar) {
			int lenvar = cpendvar - cpvar - 2;  	// Subtract the $()
			char *var = StringDup(cpvar+2, lenvar);
			SString val = GetExpanded(var);
			int newlenbase = strlen(base) + val.length() - lenvar;
			char *newbase = new char[newlenbase];
			strncpy(newbase, base, cpvar - base);
			strcpy(newbase + (cpvar - base), val.c_str());
			strcpy(newbase + (cpvar - base) + val.length(), cpendvar + 1);
			delete []var;
			delete []base;
			base = newbase;
		}
		cpvar = strstr(base, "$(");
	}
	SString sret = base;
	delete []base;
	return sret;
}

int PropSet::GetInt(const char *key, int defaultValue) {
	SString val = Get(key);
	if (val.length())
		return val.value();
	else
		return defaultValue;
}

inline bool isprefix(const char *target, const char *prefix) {
	while (*target && *prefix) {
		if (*target != *prefix)
			return false;
		target++;
		prefix++;
	}
	if (*prefix)
		return false;
	else
		return true;
}

bool issuffix(const char *target, const char *suffix) {
	int lentarget = strlen(target);
	int lensuffix = strlen(suffix);
	if (lensuffix > lentarget)
		return false;
	for (int i = lensuffix - 1; i >= 0; i--) {
		if (target[i + lentarget - lensuffix] != suffix[i])
			return false;
	}
	return true;
}

SString PropSet::GetWild(const char *keybase, const char *filename) {
    for (int root=0; root < hashRoots; root++) {
	    for (Property *p=props[root]; p; p=p->next) {
		    if (isprefix(p->key, keybase)) {
			    char *orgkeyfile = p->key + strlen(keybase);
			    char *keyfile = NULL;

			    if (strstr(orgkeyfile, "$(") == orgkeyfile) {
				    char *cpendvar = strchr(orgkeyfile, ')');
				    if (cpendvar) {
					    *cpendvar = '\0';
					    SString s = Get(orgkeyfile + 2);
					    *cpendvar= ')';
					    keyfile = strdup(s.c_str());
				    }
			    }
			    char *keyptr = keyfile;

			    if (keyfile == NULL)
				    keyfile = orgkeyfile;

			    for (; ; ) {
				    char *del = strchr(keyfile, ';');
				    if (del == NULL)
					    del = keyfile + strlen(keyfile);
				    char delchr = *del;
				    *del = '\0';
				    if (*keyfile == '*') {
					    if (issuffix(filename, keyfile + 1)) {
						    *del = delchr;
						    free(keyptr);
						    return p->val;
					    }
				    } else if (0 == strcmp(keyfile, filename)) {
					    *del = delchr;
					    free(keyptr);
					    return p->val;
				    }
				    if (delchr == '\0')
					    break;
				    *del = delchr;
				    keyfile = del + 1;
			    }
			    free(keyptr);

			    if (0 == strcmp(p->key, keybase)) {
				    return p->val;
			    }
		    }
	    }
    }
	if (superPS) {
		// Failed here, so try in base property set
		return superPS->GetWild(keybase, filename);
	} else {
		return "";
	}
}

SString PropSet::GetNewExpand(const char *keybase, const char *filename) {
	char *base = StringDup(GetWild(keybase, filename).c_str());
	char *cpvar = strstr(base, "$(");
	while (cpvar) {
		char *cpendvar = strchr(cpvar, ')');
		if (cpendvar) {
			int lenvar = cpendvar - cpvar - 2;  	// Subtract the $()
			char *var = StringDup(cpvar+2, lenvar);
			SString val = GetWild(var, filename);
			int newlenbase = strlen(base) + val.length() - lenvar;
			char *newbase = new char[newlenbase];
			strncpy(newbase, base, cpvar - base);
			strcpy(newbase + (cpvar - base), val.c_str());
			strcpy(newbase + (cpvar - base) + val.length(), cpendvar + 1);
			delete []var;
			delete []base;
			base = newbase;
		}
		cpvar = strstr(base, "$(");
	}
	SString sret = base;
	delete []base;
	return sret;
}

void PropSet::Clear() {
    for (int root=0; root < hashRoots; root++) {
        Property *p=props[root];
	    while (p) {
            Property *pNext=p->next;
		    p->hash = 0;
		    delete p->key;
		    p->key = 0;
		    delete p->val;
		    p->val = 0;
            delete p;
            p = pNext;
        }
        props[root] = 0;
    }
}

void PropSet::ReadFromMemory(const char *data, int len, const char *directoryForImports) {
	const char *pd = data;
	char linebuf[60000];
	bool ifIsTrue = true;
	while (len > 0) {
		GetFullLine(pd, len, linebuf, sizeof(linebuf));
		if (isalpha(linebuf[0]))    // If clause ends with first non-indented line
			ifIsTrue = true;
		if (isprefix(linebuf, "if ")) {
			const char *expr = linebuf + strlen("if") + 1;
			ifIsTrue = GetInt(expr);
		} else if (isprefix(linebuf, "import ") && directoryForImports) {
			char importPath[1024];
			strcpy(importPath, directoryForImports);
			strcat(importPath, linebuf + strlen("import") + 1);
			strcat(importPath, ".properties");
            		Read(importPath,directoryForImports);
		} else if (isalpha(linebuf[0])) {
			Set(linebuf);
		} else if (isspace(linebuf[0]) && ifIsTrue) {
			Set(linebuf);
		}
	}
}

void PropSet::Read(const char *filename, const char *directoryForImports) {
	char propsData[60000];
	FILE *rcfile = fopen(filename, "rb");
	if (rcfile) {
		int lenFile = fread(propsData, 1, sizeof(propsData), rcfile);
		fclose(rcfile);
		ReadFromMemory(propsData, lenFile, directoryForImports);
	} else {
		//printf("Could not open <%s>\n", filename);
	}
}

static bool iswordsep(char ch, bool onlyLineEnds) {
	if (!isspace(ch))
		return false;
	if (!onlyLineEnds)
		return true;
	return ch == '\r' || ch == '\n';
}

// Creates an array that points into each word in the string and puts \0 terminators
// after each word.
static char **ArrayFromWordList(char *wordlist, bool onlyLineEnds = false) {
	char prev = '\n';
	int words = 0;
	for (int j = 0; wordlist[j]; j++) {
		if (!iswordsep(wordlist[j], onlyLineEnds) && iswordsep(prev, onlyLineEnds))
			words++;
		prev = wordlist[j];
	}
	char **keywords = new char * [words + 1];
	if (keywords) {
		words = 0;
		prev = '\0';
		int len = strlen(wordlist);
		for (int k = 0; k < len; k++) {
			if (!iswordsep(wordlist[k], onlyLineEnds)) {
				if (!prev) {
					keywords[words] = &wordlist[k];
					words++;
				}
			} else {
				wordlist[k] = '\0';
			}
			prev = wordlist[k];
		}
		keywords[words] = &wordlist[len];
	}
	return keywords;
}

void WordList::Clear() {
	if (words) {
		delete []words;
		delete []list;
	}
	words = 0;
	list = 0;
	len = 0;
}

void WordList::Set(const char *s) {
	len = 0;
	list = StringDup(s);
	words = ArrayFromWordList(list, onlyLineEnds);
}

char *WordList::Allocate(int size) {
	list = new char[size + 1];
	list[size] = '\0';
	return list;
}

void WordList::SetFromAllocated() {
	len = 0;
	words = ArrayFromWordList(list, onlyLineEnds);
}

// Shell sort based upon public domain C implementation by Raymond Gardner 1991
// Used here because of problems with mingw qsort.
static void SortWordList(char **words, unsigned int len) {
	unsigned int gap = len / 2;

	while (gap > 0) {
		unsigned int i = gap;
		while (i < len) {
			unsigned int j = i;
			char **a = words + j;
			do {
				j -= gap;
				char **b = a;
				a -= gap;
				if (strcmp(*a, *b) > 0) {
					char *tmp = *a;
					*a = *b;
					*b = tmp;
				} else {
					break;
				}
			} while (j >= gap);
			i++;
		}
		gap = gap / 2;
	}
}

bool WordList::InList(const char *s) {
	if (0 == words)
		return false;
	if (len == 0) {
		for (int i = 0; words[i][0]; i++)
			len++;
		SortWordList(words, len);
		for (unsigned int k = 0; k < (sizeof(starts) / sizeof(starts[0])); k++)
			starts[k] = -1;
		for (int l = len - 1; l >= 0; l--) {
			unsigned char indexChar = words[l][0];
			starts[indexChar] = l;
		}
	}
	unsigned char firstChar = s[0];
	int j = starts[firstChar];
	if (j >= 0) {
		while (words[j][0] == firstChar) {
			if (s[1] == words[j][1]) {
				const char *a = words[j] + 1;
				const char *b = s + 1;
				while (*a && *a == *b) {
					a++;
					b++;
				}
				if (!*a && !*b)
					return true;
			}
			j++;
		}
	}
	return false;
}
