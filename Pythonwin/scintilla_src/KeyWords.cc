// SciTE - Scintilla based Text Editor
// KeyWords.cc
// Colourise for particular languages

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef GTK
#include <gtk/gtk.h>
#define LRESULT long
#define UINT unsigned int
#define WPARAM long
#define LPARAM long
#include "WinDefs.h"
#define stricmp strcasecmp
LRESULT SendMessage(HWND w,UINT msg, WPARAM wParam=0, LPARAM lParam=0);
#else
#include <windows.h>
#endif

#include "KeyWords.h"
#include "Scintilla.h"

static void dprintf(char *szFormat, ...) {
#ifdef TRACE
	char szBuffer[1000];
	char *pArguments = (char *) & szFormat + sizeof(szFormat);
	vsprintf(szBuffer,szFormat,pArguments);
	printf("%s",szBuffer);
#endif
}

bool wordInList(char *word, char **list) {
	if (0 == list)
		return false;
	for (int i=0; list[i][0]; i++) {
		// Initial test is to mostly avoid slow function call
		if ((list[i][0] == word[0]) && (0 == strcmp(list[i], word)))
			return true;
	}
	return false;
}

bool wordInListInsensitive(char *word, char **list) {
	if (0 == list)
		return false;
	for (int i=0; list[i][0]; i++) {
		// Initial test is to mostly avoid slow function call
		if ((toupper(list[i][0]) == toupper(word[0])) && (0 == stricmp(list[i], word))) 
			return true;
	}
	return false;
}

void colourSegHwnd(HWND hwnd, unsigned int start, unsigned int end,char chAttr) {
	// Only perform styling if non empty range
	if (end != start - 1) {
		if (end < start) {
			dprintf("Bad colour positions %d - %d\n", start, end);
		}
		//dprintf("Colour %d %0d-%0d\n", chAttr, start, end);
		SendMessage(hwnd, SCI_SETSTYLING, end - start + 1, chAttr);
	}
}

void classifyWord(char *cdoc, unsigned int start, unsigned int end, const char *language, char **keywords, HWND hwnd) {
	char s[100];
	bool wordIsNumber = isdigit(cdoc[start]) || (cdoc[start] == '.');
	for (int i=0; i<end-start+1 && i<30; i++) {
		s[i] = cdoc[start+i];
		s[i+1] = '\0';
	}
	char chAttr = 0;
	if (wordIsNumber)
		chAttr = 4;
	else {
		if (wordInList(s, keywords))
			chAttr = 5;
	}
	colourSegHwnd(hwnd, start, end, chAttr);
}

inline bool iswordchar(char ch) {
	return isalnum(ch) || ch == '.' || ch == '_';
}

inline bool iswordstart(char ch) {
	return isalnum(ch) || ch == '_';
}

inline bool isoperator(char ch) {
	if (isalnum(ch))
		return false;
	// '.' left out as it is used to make up numbers
	if (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||
    		ch == '(' || ch == ')' || ch == '-' || ch == '+' ||
    		ch == '=' || ch == '|' || ch == '{' || ch == '}' ||
    		ch == '[' || ch == ']' || ch == ':' || ch == ';' ||
    		ch == '<' || ch == '>' || ch == ',' || ch == '/' ||
    		ch == '?' || ch == '!' || ch == '.' || ch == '~')
		return true;
	return false;
}

enum eState {
    eDefault = 0,
    eComment = 1,
    eLineComment = 2,
    eDocComment = 3,
    eNumber = 4,
    eWord = 5,
    eString = 6,
    eChar = 7,
    ePunct = 8,
    ePreProc = 9,
    eOperator = 10,
    eIdentifier = 11,
};

void classifyWordCpp(char *cdoc, unsigned int start, unsigned int end, char **keywords, HWND hwnd) {
	char s[100];
	bool wordIsNumber = isdigit(cdoc[start]) || (cdoc[start] == '.');
	for (int i=0; i<end-start+1 && i<30; i++) {
		s[i] = cdoc[start+i];
		s[i+1] = '\0';
	}
	char chAttr = eIdentifier;
	if (wordIsNumber)
		chAttr = eNumber;
	else {
		if (wordInList(s, keywords))
			chAttr = eWord;
	}
	colourSegHwnd(hwnd, start, end, chAttr);
}

static void ColouriseCppDoc(char *cdoc, int lengthDoc, int initStyle, char **keywords, HWND hwnd) {
	eState state = static_cast<eState>(initStyle);
	char chPrev = ' ';
	char chNext = cdoc[0];
	int startSeg = 0;
	for (int i=0;i<=lengthDoc;i++) {
		eState statePrev = state;
		char ch = chNext;
		chNext = ' ';
		if (i+1 < lengthDoc)
			chNext = cdoc[i+1];

		if (state == eDefault) {
			if (iswordstart(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eWord;
				startSeg = i;
			} else if (ch == '/' && chNext == '*') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eComment;
				startSeg = i;
			} else if (ch == '/' && chNext == '/') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eLineComment;
				startSeg = i;
			} else if (ch == '\"') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eString;
				startSeg = i;
			} else if (ch == '\'') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eChar;
				startSeg = i;
			} else if (ch == '#') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = ePreProc;
				startSeg = i;
			} else if (isoperator(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				colourSegHwnd(hwnd, i, i, eOperator);
				startSeg = i+1;
			}
		} else if (state == eWord) {
			if (!iswordchar(ch)) {
				classifyWordCpp(cdoc, startSeg, i - 1, keywords, hwnd);
				state = eDefault;
				startSeg = i;
				if (ch == '/' && chNext == '*') {
					state = eComment;
				} else if (ch == '/' && chNext == '/') {
					state = eLineComment;
				} else if (ch == '\"') {
					state = eString;
				} else if (ch == '\'') {
					state = eChar;
				} else if (ch == '#') {
					state = ePreProc;
				} else if (isoperator(ch)) {
					colourSegHwnd(hwnd, startSeg, i, eOperator);
					state = eDefault;
					startSeg = i + 1;
				}
			}
		} else {
			if (state == ePreProc) {
				if ((ch == '\r' || ch == '\n') && (chPrev != '\\')) {
					state = eDefault;
					colourSegHwnd(hwnd, startSeg, i-1, ePreProc);
					startSeg = i;
				}
			} else if (state == eComment) {
				if (ch == '/' && chPrev == '*' && ((i > startSeg + 2) || ((initStyle == eComment) && (startSeg == 0)))) {
					state = eDefault;
					colourSegHwnd(hwnd, startSeg, i, eComment);
					startSeg = i + 1;
				}
			} else if (state == eLineComment) {
				if (ch == '\r' || ch == '\n') {
					colourSegHwnd(hwnd, startSeg, i-1, eLineComment);
					state = eDefault;
					startSeg = i;
				}
			} else if (state == eString) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, eString);
					state = eDefault;
					i++;
					ch = chNext;
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
					startSeg = i;
				}
			} else if (state == eChar) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\'') {
					colourSegHwnd(hwnd, startSeg, i, eChar);
					state = eDefault;
					i++;
					ch = chNext;
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
					startSeg = i;
				}
			}
			if (state == eDefault) {    // One of the above succeeded
				if (ch == '/' && chNext == '*') {
					state = eComment;
				} else if (ch == '/' && chNext == '/') {
					state = eLineComment;
				} else if (ch == '\"') {
					state = eString;
				} else if (ch == '\'') {
					state = eChar;
				} else if (ch == '#') {
					state = ePreProc;
				} else if (iswordstart(ch)) {
					state = eWord;
				} else if (isoperator(ch)) {
					colourSegHwnd(hwnd, startSeg, i, eOperator);
					startSeg = i + 1;
				}
			}
		}
		chPrev = ch;
	}
	if (startSeg < lengthDoc)
		colourSegHwnd(hwnd, startSeg, lengthDoc-1, state);
}

static void ColouriseJavaDoc(char *cdoc, int lengthDoc, int initStyle, const char *language, char **keywords, HWND hwnd) {
	eState state = static_cast<eState>(initStyle);
	char chPrev = ' ';
	char chNext = cdoc[0];
	char chNext2 = cdoc[0];
	int startSeg = 0;
	for (int i=0;i<=lengthDoc;i++) {
		eState statePrev = state;
		char ch = chNext;
		chNext = ' ';
		if (i+1 < lengthDoc)
			chNext = cdoc[i+1];
		if (i+2 < lengthDoc)
			chNext2 = cdoc[i+2];

		if (state == eDefault) {
			if (iswordstart(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eWord;
				startSeg = i;
			} else if (ch == '/' && chNext == '*') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				if (chNext2 == '*')
					state = eDocComment;
				else
					state = eComment;
				startSeg = i;
			} else if (ch == '/' && chNext == '/') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eLineComment;
				startSeg = i;
			} else if (ch == '\"') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eString;
				startSeg = i;
			} else if (ch == '\'') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eChar;
				startSeg = i;
			} else if (isoperator(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				colourSegHwnd(hwnd, i, i, eOperator);
				startSeg = i+1;
			}
		} else if (state == eWord) {
			if (!iswordchar(ch)) {
				classifyWord(cdoc, startSeg, i - 1, language, keywords, hwnd);
				state = eDefault;
				startSeg = i;
				if (ch == '/' && chNext == '*') {
					if (chNext2 == '*')
						state = eDocComment;
					else
						state = eComment;
				} else if (ch == '/' && chNext == '/') {
					state = eLineComment;
				} else if (ch == '\"') {
					state = eString;
				} else if (ch == '\'') {
					state = eChar;
				} else if (isoperator(ch)) {
					colourSegHwnd(hwnd, startSeg, i, eOperator);
					state = eDefault;
					startSeg = i + 1;
				}
			}
		} else {
			if (state == eComment) {
				if (ch == '/' && chPrev == '*' && ((i > startSeg + 2) || ((initStyle == eComment) && (startSeg == 0)))) {
					state = eDefault;
					colourSegHwnd(hwnd, startSeg, i, eComment);
					startSeg = i + 1;
				}
			} else if (state == eDocComment) {
				if (ch == '/' && chPrev == '*' && ((i > startSeg + 2) || ((initStyle == eComment) && (startSeg == 0)))) {
					state = eDefault;
					colourSegHwnd(hwnd, startSeg, i, eDocComment);
					startSeg = i + 1;
				}
			} else if (state == eLineComment) {
				if (ch == '\r' || ch == '\n') {
					colourSegHwnd(hwnd, startSeg, i-1, eLineComment);
					state = eDefault;
					startSeg = i;
				}
			} else if (state == eString) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, 7);
					state = eDefault;
					i++;
					ch = chNext;
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
					startSeg = i;
				}
			} else if (state == eChar) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\'') {
					colourSegHwnd(hwnd, startSeg, i, 7);
					state = eDefault;
					i++;
					ch = chNext;
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
					startSeg = i;
				}
			}
			if (state == eDefault) {    // One of the above succeeded
				if (ch == '/' && chNext == '*') {
					if (chNext2 == '*')
						state = eDocComment;
					else
						state = eComment;
				} else if (ch == '/' && chNext == '/') {
					state = eLineComment;
				} else if (ch == '\"') {
					state = eString;
				} else if (ch == '\'') {
					state = eChar;
				} else if (iswordstart(ch)) {
					state = eWord;
				} else if (isoperator(ch)) {
					colourSegHwnd(hwnd, startSeg, i, eOperator);
					startSeg = i + 1;
				}
			}
		}
		chPrev = ch;
	}
	if (startSeg < lengthDoc)
		colourSegHwnd(hwnd, startSeg, lengthDoc, state);
}

void classifyWordVB(char *cdoc, unsigned int start, unsigned int end, char **keywords, HWND hwnd) {
	char s[100];
	bool wordIsNumber = isdigit(cdoc[start]) || (cdoc[start] == '.');
	for (int i=0; i<end-start+1 && i<30; i++) {
		s[i] = cdoc[start+i];
		s[i+1] = '\0';
	}
	char chAttr = 0;
	if (wordIsNumber)
		chAttr = 4;
	else {
		if (wordInListInsensitive(s, keywords))
			chAttr = 5;
	}
    colourSegHwnd(hwnd, start, end, chAttr);
}

static void ColouriseVBDoc(char *cdoc, int lengthDoc, int initStyle, const char *language, char **keywords, HWND hwnd) {
    eState state = static_cast<eState>(initStyle);
    char chPrev = ' ';
    char chNext = cdoc[0];
    char chNext2 = cdoc[0];
    int startSeg = 0;
    for (int i=0;i<lengthDoc;i++) {
	    eState statePrev = state;
	    char ch = chNext;
	    chNext = ' ';
	    if (i+1 < lengthDoc) 
			chNext = cdoc[i+1];
	    if (i+2 < lengthDoc) 
			chNext2 = cdoc[i+2];

	    if (state == eDefault) {
			if (iswordstart(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eWord;
				startSeg = i;
			} else if (ch == '\'') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eLineComment;
				startSeg = i;
			} else if (ch == '\"') {
				colourSegHwnd(hwnd, startSeg, i - 1, eDefault);
				state = eString;
				startSeg = i;
			}
	    } else if (state == eWord) {
			if (!iswordchar(ch)) {
				if ((i - startSeg == 3) && 
					toupper(cdoc[startSeg]) == 'R' && 
					toupper(cdoc[startSeg+1]) == 'E' && 
					toupper(cdoc[startSeg+2]) == 'M' ) {
				    colourSegHwnd(hwnd, startSeg, i - 1, eLineComment);
					state = eLineComment;
					startSeg = i;
				} else {
					classifyWordVB(cdoc, startSeg, i - 1, keywords, hwnd);
	   				state = eDefault;
					startSeg = i;
					if (ch == '\'') {
						state = eLineComment;
					} else if (ch == '\"') {
						state = eString;
					}
				}
			}
	    } else {
			if (state == eLineComment) {
				if (ch == '\r' || ch == '\n') {
					colourSegHwnd(hwnd, startSeg, i-1, eLineComment);
					state = eDefault;
    				startSeg = i;
				}
			} else if (state == eString) {
				// VB doubles quotes to preserve them
				if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, 7);
					state = eDefault;
					i++;
					ch = chNext;
					chNext = ' ';
					if (i+1 < lengthDoc) 
						chNext = cdoc[i+1];
					startSeg = i;
				}
			}
			if (state == eDefault) {    // One of the above succeeded
				if (ch == '\'') {
					state = eLineComment;
				} else if (ch == '\"') {
					state = eString;
				} else if (iswordstart(ch)) {
					state = eWord;
				}
			}
	    }
	    chPrev = ch;
    }
	if (startSeg < lengthDoc)
		colourSegHwnd(hwnd, startSeg, lengthDoc, state);
}

enum ePyState {
    ePyDefault = 0,
    ePyComment = 1,
    ePyNumber = 2,
    ePyString = 3,
    ePyChar = 4,
    ePyWord = 5,
    eTriple = 6,
    eTripleDouble = 7,
    eClassName = 8,
    eDefName = 9,
    ePyOperator = 10,
    ePyIdentifier = 11,
};

void classifyWordPy(char *cdoc, unsigned int start, unsigned int end, char **keywords, HWND hwnd, char *prevWord) {
	char s[100];
	bool wordIsNumber = isdigit(cdoc[start]);
	for (int i=0; i<end-start+1 && i<30; i++) {
		s[i] = cdoc[start+i];
		s[i+1] = '\0';
	}
	char chAttr = ePyIdentifier;
	if (0 == strcmp(prevWord, "class"))
		chAttr = eClassName;
	else if (0 == strcmp(prevWord, "def"))
		chAttr = eDefName;
	else if (wordIsNumber)
		chAttr = ePyNumber;
	else if (wordInList(s, keywords))
		chAttr = ePyWord;
	colourSegHwnd(hwnd, start, end, chAttr);
	strcpy(prevWord, s);
}

static void ColourisePyDoc(char *cdoc, int lengthDoc, int initStyle, char **keywords, HWND hwnd) {
	//dprintf("Python coloured\n");
	char prevWord[200];
	prevWord[0] = '\0';
	if (lengthDoc == 0)
		return;
	ePyState state = static_cast<ePyState>(initStyle);
	char chPrev = ' ';
	char chPrev2 = ' ';
	char chNext = cdoc[0];
	char chNext2 = cdoc[0];
	int startSeg = 0;
	for (int i=0;i<=lengthDoc;i++) {
		char ch = chNext;
		chNext = ' ';
		if (i+1 < lengthDoc)
			chNext = cdoc[i+1];
		chNext2 = ' ';
		if (i+2 < lengthDoc)
			chNext2 = cdoc[i+2];

		if (state == ePyDefault) {
			if (iswordstart(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				state = ePyWord;
				startSeg = i;
			} else if (ch == '#') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				state = ePyComment;
				startSeg = i;
			} else if (ch == '\"') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				startSeg = i;
				if (chNext == '\"' && chNext2 == '\"') {
					i += 2;
					state = eTripleDouble;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyString;
				}
			} else if (ch == '\'') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				startSeg = i;
				if (chNext == '\'' && chNext2 == '\'') {
					i += 2;
					state = eTriple;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyChar;
				}
			} else if (isoperator(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				colourSegHwnd(hwnd, i, i, ePyOperator);
				startSeg = i+1;
			}
		} else if (state == ePyWord) {
			if (!iswordchar(ch)) {
				classifyWordPy(cdoc, startSeg, i - 1, keywords, hwnd, prevWord);
				state = ePyDefault;
				startSeg = i;
				if (ch == '#') {
					state = ePyComment;
				} else if (ch == '\"') {
					if (chNext == '\"' && chNext2 == '\"') {
						i += 2;
						state = eTripleDouble;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyString;
					}
				} else if (ch == '\'') {
					if (chNext == '\'' && chNext2 == '\'') {
						i += 2;
						state = eTriple;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyChar;
					}
				} else if (isoperator(ch)) {
					colourSegHwnd(hwnd, startSeg, i, ePyOperator);
					state = ePyDefault;
					startSeg = i + 1;
				}
			}
		} else {
			if (state == ePyComment) {
				if (ch == '\r' || ch == '\n') {
					colourSegHwnd(hwnd, startSeg, i-1, ePyComment);
					state = ePyDefault;
					startSeg = i;
				}
			} else if (state == ePyString) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, ePyString);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == ePyChar) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\'') {
					colourSegHwnd(hwnd, startSeg, i, ePyChar);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTriple) {
				if (ch == '\'' && chPrev == '\'' && chPrev2 == '\'') {
					colourSegHwnd(hwnd, startSeg, i, eTriple);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTripleDouble) {
				if (ch == '\"' && chPrev == '\"' && chPrev2 == '\"') {
					colourSegHwnd(hwnd, startSeg, i, eTripleDouble);
					state = ePyDefault;
					startSeg = i+1;
				}
			}
		}
		chPrev2 = chPrev;
		chPrev = ch;
	}
	if (startSeg < lengthDoc) {
		if (state == ePyDefault) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyDefault);
		} else if (state == ePyWord) {
			classifyWordPy(cdoc, startSeg, lengthDoc, keywords, hwnd, prevWord);
		} else if (state == ePyComment) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyComment);
		} else if (state == ePyString) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyString);
		} else if (state == ePyChar) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyChar);
		} else if (state == eTriple) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, eTriple);
		} else if (state == eTripleDouble) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, eTripleDouble);
		}
	}
}

void classifyWordPyro(char *cdoc, unsigned int start, unsigned int end, char **keywords, HWND hwnd, char *prevWord) {
	char s[100];
	bool wordIsNumber = isdigit(cdoc[start]);
	for (int i=0; i<end-start+1 && i<30; i++) {
		s[i] = cdoc[start+i];
		s[i+1] = '\0';
	}
	char chAttr = 0;
	if (0 == strcmp(prevWord, "class"))
		chAttr = eClassName;
	else if (0 == strcmp(prevWord, "def"))
		chAttr = eDefName;
	else if (wordIsNumber)
		chAttr = ePyNumber;
	else if (wordInList(s, keywords))
		chAttr = ePyWord;
	if (0 == strcmp(s, "deprecated"))
		chAttr |= 32;
	colourSegHwnd(hwnd, start, end, chAttr);
	strcpy(prevWord, s);
}

static void ColourisePyroDoc(char *cdoc, int lengthDoc, int initStyle, char **keywords, HWND hwnd) {
	char prevWord[200];
	prevWord[0] = '\0';
	if (lengthDoc == 0)
		return;
	ePyState state = static_cast<ePyState>(initStyle & 31);
	char chPrev = ' ';
	char chPrev2 = ' ';
	char chNext = cdoc[0];
	char chNext2 = cdoc[0];
	int startSeg = 0;
	for (int i=0;i<=lengthDoc;i++) {
		char ch = chNext;
		chNext = ' ';
		if (i+1 < lengthDoc)
			chNext = cdoc[i+1];
		chNext2 = ' ';
		if (i+2 < lengthDoc)
			chNext2 = cdoc[i+2];

		if (state == ePyDefault) {
			if (iswordstart(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				state = ePyWord;
				startSeg = i;
			} else if (ch == '#') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				state = ePyComment;
				startSeg = i;
			} else if (ch == '\"') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				startSeg = i;
				if (chNext == '\"' && chNext2 == '\"') {
					i += 2;
					state = eTripleDouble;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyString;
				}
			} else if (ch == '\'') {
				colourSegHwnd(hwnd, startSeg, i - 1, ePyDefault);
				startSeg = i;
				if (chNext == '\'' && chNext2 == '\'') {
					i += 2;
					state = eTriple;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyChar;
				}
			}
		} else if (state == ePyWord) {
			if (!iswordchar(ch)) {
				classifyWordPyro(cdoc, startSeg, i - 1, keywords, hwnd, prevWord);
				state = ePyDefault;
				startSeg = i;
				if (ch == '#') {
					state = ePyComment;
				} else if (ch == '\"') {
					if (chNext == '\"' && chNext2 == '\"') {
						i += 2;
						state = eTripleDouble;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyString;
					}
				} else if (ch == '\'') {
					if (chNext == '\'' && chNext2 == '\'') {
						i += 2;
						state = eTriple;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyChar;
					}
				}
			}
		} else {
			if (state == ePyComment) {
				if (ch == '\r' || ch == '\n') {
					colourSegHwnd(hwnd, startSeg, i-1, ePyComment);
					state = ePyDefault;
					startSeg = i;
				}
			} else if (state == ePyString) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, ePyString);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == ePyChar) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\'') {
					colourSegHwnd(hwnd, startSeg, i, ePyChar);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTriple) {
				if (ch == '\'' && chPrev == '\'' && chPrev2 == '\'') {
					colourSegHwnd(hwnd, startSeg, i, eTriple);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTripleDouble) {
				if (ch == '\"' && chPrev == '\"' && chPrev2 == '\"') {
					colourSegHwnd(hwnd, startSeg, i, eTripleDouble);
					state = ePyDefault;
					startSeg = i+1;
				}
			}
		}
		chPrev2 = chPrev;
		chPrev = ch;
	}
	if (startSeg < lengthDoc) {
		if (state == ePyDefault) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyDefault);
		} else if (state == ePyWord) {
			classifyWordPyro(cdoc, startSeg, lengthDoc, keywords, hwnd, prevWord);
		} else if (state == ePyComment) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyComment);
		} else if (state == ePyString) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyString);
		} else if (state == ePyChar) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, ePyChar);
		} else if (state == eTriple) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, eTriple);
		} else if (state == eTripleDouble) {
			colourSegHwnd(hwnd, startSeg, lengthDoc, eTripleDouble);
		}
	}
}

static void ColouriseBatchLine(char *lineBuffer, int lengthLine, HWND hwnd) {
	if (0 == strncmp(lineBuffer, "REM", 3)) {
		colourSegHwnd(hwnd, 0, lengthLine-1, 1);
	} else if (0 == strncmp(lineBuffer, "rem", 3)) {
		colourSegHwnd(hwnd, 0, lengthLine-1, 1);
	} else if (0 == strncmp(lineBuffer, "SET", 3)) {
		colourSegHwnd(hwnd, 0, lengthLine-1, 2);
	} else if (0 == strncmp(lineBuffer, "set", 3)) {
		colourSegHwnd(hwnd, 0, lengthLine-1, 2);
	} else if (lineBuffer[0] == ':') {
		colourSegHwnd(hwnd, 0, lengthLine-1, 3);
	} else {
		colourSegHwnd(hwnd, 0, lengthLine-1, 0);
	}
}

static void ColouriseBatchDoc(char *cdoc, int lengthDoc, int initStyle, HWND hwnd) {
	char lineBuffer[1024];
	int linePos = 0;
	for (int i=0;i<lengthDoc;i++) {
		lineBuffer[linePos++] = cdoc[i];
		if (cdoc[i] == '\r' || cdoc[i] == '\n' || (linePos >= sizeof(lineBuffer) - 1)) {
			ColouriseBatchLine(lineBuffer, linePos, hwnd);
			linePos = 0;
		}
	}
	if (linePos > 0)
		ColouriseBatchLine(lineBuffer, linePos, hwnd);
}

static void ColourisePropsLine(char *lineBuffer, int lengthLine, HWND hwnd) {
	int i=0;
	while (isspace(lineBuffer[i]) && (i < lengthLine))
		i++;
	if (lineBuffer[i] == '#' || lineBuffer[i] == '!') {
		colourSegHwnd(hwnd, 0, lengthLine-1, 1);
	} else {
		colourSegHwnd(hwnd, 0, lengthLine-1, 0);
	}
}

static void ColourisePropsDoc(char *cdoc, int lengthDoc, int initStyle, HWND hwnd) {
	char lineBuffer[1024];
	int linePos = 0;
	for (int i=0;i<=lengthDoc;i++) {
		lineBuffer[linePos++] = cdoc[i];
		if (cdoc[i] == '\r' || cdoc[i] == '\n' || (linePos >= sizeof(lineBuffer) - 1)) {
			ColourisePropsLine(lineBuffer, linePos, hwnd);
			linePos = 0;
		}
	}
	if (linePos > 0)
		ColourisePropsLine(lineBuffer, linePos, hwnd);
}

static void ColouriseErrorListLine(char *lineBuffer, int lengthLine, HWND hwnd) {
	if (lineBuffer[0] == '>') {
		// Command or return status
		colourSegHwnd(hwnd, 0, lengthLine-1, 4);
	} else if (strstr(lineBuffer, "File \"") && strstr(lineBuffer, ", line ")) {
		colourSegHwnd(hwnd, 0, lengthLine-1, 1);
	} else {
		// Look for <filename>:<line>:message
		int state = 0;
		for (int i=0;i<lengthLine;i++) {
			if (state == 0 && lineBuffer[i] == ':' && isdigit(lineBuffer[i+1])) {
				state = 1;
			} else if (state == 0 && lineBuffer[i] == '(') {
				state = 10;
			} else if (state == 1 && isdigit(lineBuffer[i])) {
				state = 2;
			} else if (state == 2 && lineBuffer[i] == ':') {
				state = 3;
			} else if (state == 2 && !isdigit(lineBuffer[i])) {
				state = 99;
			} else if (state == 10 && isdigit(lineBuffer[i])) {
				state = 11;
			} else if (state == 11 && lineBuffer[i] == ')') {
				state = 12;
			} else if (state == 12 && lineBuffer[i] == ':') {
				state = 13;
			} else if (state == 11 && !isdigit(lineBuffer[i])) {
				state = 99;
			}
		}
		if (state == 3) {
			colourSegHwnd(hwnd, 0, lengthLine-1, 2);
		} else if (state == 13) {
			colourSegHwnd(hwnd, 0, lengthLine-1, 3);
		} else {
			colourSegHwnd(hwnd, 0, lengthLine-1, 0);
		}
	}
}

static void ColouriseErrorListDoc(char *cdoc, int lengthDoc, int initStyle, HWND hwnd) {
	char lineBuffer[1024];
	int linePos = 0;
	for (int i=0;i<=lengthDoc;i++) {
		lineBuffer[linePos++] = cdoc[i];
		if (cdoc[i] == '\r' || cdoc[i] == '\n' || (linePos >= sizeof(lineBuffer) - 1)) {
			ColouriseErrorListLine(lineBuffer, linePos, hwnd);
			linePos = 0;
		}
	}
	if (linePos > 0)
		ColouriseErrorListLine(lineBuffer, linePos, hwnd);
}

void ColouriseDoc(char *cdoc, int startPos, int lengthDoc, int initStyle, const char *language, char **keywords, HWND hwnd) {
	//dprintf("ColouriseDoc <%s>\n", language);
	SendMessage(hwnd, SCI_STARTSTYLING, startPos, 31);
	if (0 == strcmp(language, "python")) {
		ColourisePyDoc(cdoc, lengthDoc, initStyle, keywords, hwnd);
	} else if (0 == strcmp(language, "pyro")) {
		ColourisePyroDoc(cdoc, lengthDoc, initStyle, keywords, hwnd);
	} else if (0 == strcmp(language, "batch")) {
		ColouriseBatchDoc(cdoc, lengthDoc, initStyle, hwnd);
	} else if (0 == strcmp(language, "java")) {
		ColouriseJavaDoc(cdoc, lengthDoc, initStyle, language, keywords, hwnd);
	} else if (0 == strcmp(language, "javascript")) {
		ColouriseJavaDoc(cdoc, lengthDoc, initStyle, language, keywords, hwnd);
	} else if (0 == strcmp(language, "vb")) {
		ColouriseVBDoc(cdoc, lengthDoc, initStyle, language, keywords, hwnd);
	} else if (0 == strcmp(language, "cpp")) {
		ColouriseCppDoc(cdoc, lengthDoc, initStyle, keywords, hwnd);
	} else if (0 == strcmp(language, "props")) {
		ColourisePropsDoc(cdoc, lengthDoc, initStyle, hwnd);
	} else if (0 == strcmp(language, "errorlist")) {
		ColouriseErrorListDoc(cdoc, lengthDoc, initStyle, hwnd);
	} else {
		colourSegHwnd(hwnd, 0, lengthDoc, 0);
	}
}

static bool tabsCheck(int line, int tabsPrev[], int tabsThis[]) {
	dprintf("%d %0d:%0d %0d:%0d %0d:%0d %0d:%0d\n", line,
        	tabsPrev[0], tabsThis[0],
        	tabsPrev[1], tabsThis[1],
        	tabsPrev[3], tabsThis[3],
        	tabsPrev[7], tabsThis[7]);
	int tabDir = 0;
	if (tabsPrev[0] < tabsThis[0])
		tabDir = 1;
	if (tabsPrev[0] > tabsThis[0])
		tabDir = -1;
	tabsPrev[0] = tabsThis[0];
	tabsThis[0] = 1;
	bool tabsSame = true;
	for (int it=1;it<8;it++) {
		int tabDirIt = 0;
		if (tabsPrev[it] < tabsThis[it])
			tabDirIt = 1;
		if (tabsPrev[it] > tabsThis[it])
			tabDirIt = -1;
		if (tabDir != tabDirIt)
			tabsSame = false;
		tabsPrev[it] = tabsThis[it];
		tabsThis[it] = 1;
	}
	return tabsSame;
}

void TabTimmy(char *cdoc, int lengthDoc, HWND hwnd) {
	int tabsPrev[] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	int tabsThis[] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	SendMessage(hwnd, SCI_STARTSTYLING, 0, 64);
	bool inIndent = true;
	int line = 0;
	if (lengthDoc == 0)
		return;
	ePyState state = static_cast<ePyState>(0);
	char chPrev = ' ';
	char chPrev2 = ' ';
	char chNext = cdoc[0];
	char chNext2 = cdoc[0];
	int startSeg = 0;
	for (int i=0;i<lengthDoc;i++) {
		char ch = chNext;
		chNext = ' ';
		if (i+1 < lengthDoc)
			chNext = cdoc[i+1];
		chNext2 = ' ';
		if (i+2 < lengthDoc)
			chNext2 = cdoc[i+2];

		if (state == ePyDefault) {
			if (inIndent) {
				if (ch == ' ') {
					for (int it=0;it<8;it++)
						tabsThis[it]++;
				} else if (ch == '\t') {
					for (int it=0;it<8;it++)
						tabsThis[it] = ((tabsThis[it]) / (it+1) + 1) * (it+1);
				}
			}
			if (iswordstart(ch)) {
				if (inIndent && !tabsCheck(line, tabsPrev, tabsThis))
					colourSegHwnd(hwnd, startSeg, i - 1, 64);
				else
					colourSegHwnd(hwnd, startSeg, i - 1, 0);
				inIndent = false;
				state = ePyWord;
				startSeg = i;
			} else if (ch == '#') {
				if (inIndent && !tabsCheck(line, tabsPrev, tabsThis))
					colourSegHwnd(hwnd, startSeg, i - 1, 64);
				else
					colourSegHwnd(hwnd, startSeg, i - 1, 0);
				inIndent = false;
				state = ePyComment;
				startSeg = i;
			} else if (ch == '\"') {
				if (inIndent && !tabsCheck(line, tabsPrev, tabsThis))
					colourSegHwnd(hwnd, startSeg, i - 1, 64);
				else
					colourSegHwnd(hwnd, startSeg, i - 1, 0);
				inIndent = false;
				startSeg = i;
				if (chNext == '\"' && chNext2 == '\"') {
					i += 2;
					state = eTripleDouble;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyString;
				}
			} else if (ch == '\'') {
				if (inIndent && !tabsCheck(line, tabsPrev, tabsThis))
					colourSegHwnd(hwnd, startSeg, i - 1, 64);
				else
					colourSegHwnd(hwnd, startSeg, i - 1, 0);
				inIndent = false;
				startSeg = i;
				if (chNext == '\'' && chNext2 == '\'') {
					i += 2;
					state = eTriple;
					ch = ' ';
					chPrev = ' ';
					chNext = ' ';
					if (i+1 < lengthDoc)
						chNext = cdoc[i+1];
				} else {
					state = ePyChar;
				}
			} else if (ch == '\r' || ch == '\n') {
				inIndent = true;
				if (ch == '\n')
					line++;
			}
		} else if (state == ePyWord) {
			if (!iswordchar(ch)) {
				colourSegHwnd(hwnd, startSeg, i - 1, 0);
				state = ePyDefault;
				if (ch == '\r' || ch == '\n')
					inIndent = true;
				if (ch == '\n')
					line++;
				startSeg = i;
				if (ch == '#') {
					state = ePyComment;
				} else if (ch == '\"') {
					if (chNext == '\"' && chNext2 == '\"') {
						i += 2;
						state = eTripleDouble;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyString;
					}
				} else if (ch == '\'') {
					if (chNext == '\'' && chNext2 == '\'') {
						i += 2;
						state = eTriple;
						ch = ' ';
						chPrev = ' ';
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					} else {
						state = ePyChar;
					}
				}
			}
		} else {
			if (state == ePyComment) {
				if (ch == '\r' || ch == '\n') {
					if (ch == '\n')
						line++;
					colourSegHwnd(hwnd, startSeg, i-1, 0);
					state = ePyDefault;
					startSeg = i;
					inIndent = true;
				}
			} else if (state == ePyString) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\"') {
					colourSegHwnd(hwnd, startSeg, i, 0);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == ePyChar) {
				if (ch == '\\') {
					if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
						i++;
						ch = chNext;
						chNext = ' ';
						if (i+1 < lengthDoc)
							chNext = cdoc[i+1];
					}
				} else if (ch == '\'') {
					colourSegHwnd(hwnd, startSeg, i, 0);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTriple) {
				if (ch == '\'' && chPrev == '\'' && chPrev2 == '\'') {
					colourSegHwnd(hwnd, startSeg, i, 0);
					state = ePyDefault;
					startSeg = i+1;
				}
			} else if (state == eTripleDouble) {
				if (ch == '\"' && chPrev == '\"' && chPrev2 == '\"') {
					colourSegHwnd(hwnd, startSeg, i, 0);
					state = ePyDefault;
					startSeg = i+1;
				}
			}
		}
		chPrev2 = chPrev;
		chPrev = ch;
	}
}


