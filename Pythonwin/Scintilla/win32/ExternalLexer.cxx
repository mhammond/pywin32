// Scintilla source code edit control
/** @file ExternalLexer.cxx
 ** Support external lexers in DLLs.
 **/
// Copyright 2001 Simon Steele <ss@pnotepad.org>, portions copyright Neil Hodgson.
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h> 
#include <stdio.h> 
#include <ctype.h> 

#define _WIN32_WINNT  0x0400
#include <windows.h>

#include "Platform.h"
#include "SciLexer.h"
#include "PropSet.h"
#include "Accessor.h"
#include "DocumentAccessor.h"
#include "KeyWords.h"
#include "ExternalLexer.h"

// Initialise the static vars...
int LexerManager::UseCount = 0;
LexerLibrary *LexerManager::first = NULL;
LexerLibrary *LexerManager::last = NULL;
LexerManager *LexerManager::firstlm = NULL;

//------------------------------------------
//
// ExternalLexerModule
//
//------------------------------------------

char **WordListsToStrings(WordList *val[]) {
	int dim = 0;
	while (val[dim])
		dim++;
	char **wls = new char * [dim + 1];
	for (int i = 0;i < dim;i++) {
		SString words;
		words = "";
		for (int n = 0; n < val[i]->len; n++) {
			words += val[i]->words[n];
			if (n != val[i]->len - 1)
				words += " ";
		}
		wls[i] = new char[words.length() + 1];
		strcpy(wls[i], words.c_str());
	}
	wls[dim] = 0;
	return wls;
}

void DeleteWLStrings(char *strs[]) {
	int dim = 0;
	while (strs[dim]) {
		delete strs[dim];
		dim++;
	}
	delete [] strs;
}

void ExternalLexerModule::Lex(unsigned int startPos, int lengthDoc, int initStyle,
                              WordList *keywordlists[], Accessor &styler) const {
	if (!fneLexer)
		return ;

	char **kwds = WordListsToStrings(keywordlists);
	char *ps = styler.GetProperties();
	
	// The accessor passed in is always a DocumentAccessor so this cast and the subsequent 
	// access will work. Can not use the stricter dynamic_cast as that requires RTTI.
	DocumentAccessor &da = static_cast<DocumentAccessor &>(styler);
	WindowID wID = da.GetWindow();

	fneLexer(externalLanguage, startPos, lengthDoc, initStyle, kwds, wID, ps);

	delete ps;
	DeleteWLStrings(kwds);
}

void ExternalLexerModule::Fold(unsigned int startPos, int lengthDoc, int initStyle,
                               WordList *keywordlists[], Accessor &styler) const {
	if (!fneFolder)
		return ;

	char **kwds = WordListsToStrings(keywordlists);
	char *ps = styler.GetProperties();
	
	// The accessor passed in is always a DocumentAccessor so this cast and the subsequent 
	// access will work. Can not use the stricter dynamic_cast as that requires RTTI.
	DocumentAccessor &da = static_cast<DocumentAccessor &>(styler);
	WindowID wID = da.GetWindow();

	fneFolder(externalLanguage, startPos, lengthDoc, initStyle, kwds, wID, ps);

	delete ps;
	DeleteWLStrings(kwds);
}

void ExternalLexerModule::SetExternal(ExtLexerFunction fLexer, ExtFoldFunction fFolder, int index) {
	fneLexer = fLexer;
	fneFolder = fFolder;
	externalLanguage = index;
}

//------------------------------------------
//
// LexerLibrary
//
//------------------------------------------

LexerLibrary::LexerLibrary(LPCTSTR ModuleName) {
	// Initialise some members...
	first = NULL;
	last = NULL;

	// Load the DLL
	m_hModule = LoadLibrary(ModuleName);
	if (m_hModule) {
		m_sModuleName = ModuleName;
		GetLexerCountFn GetLexerCount = (GetLexerCountFn)GetProcAddress(m_hModule, "GetLexerCount");

		if (GetLexerCount) {
			ExternalLexerModule *lex;
			LexerMinder *lm;

			// Find functions in the DLL
			GetLexerNameFn GetLexerName = (GetLexerNameFn)GetProcAddress(m_hModule, "GetLexerName");
			ExtLexerFunction Lexer = (ExtLexerFunction)GetProcAddress(m_hModule, "Lex");
			ExtFoldFunction Folder = (ExtFoldFunction)GetProcAddress(m_hModule, "Fold");

			// Assign a buffer for the lexer name.
			char lexname[100];
			strcpy(lexname, "");

			int nl = GetLexerCount();

			for (int i = 0; i < nl; i++) {
				GetLexerName(i, lexname, 100);
				lex = new ExternalLexerModule(SCLEX_AUTOMATIC, NULL, lexname, NULL);

				// Create a LexerMinder so we don't leak the ExternalLexerModule...
				lm = new LexerMinder;
				lm->self = lex;
				lm->next = NULL;
				if (first != NULL) {
					last->next = lm;
					last = lm;
				} else {
					first = lm;
					last = lm;
				}

				// The external lexer needs to know how to call into its DLL to
				// do its lexing and folding, we tell it here. Folder may be null.
				lex->SetExternal(Lexer, Folder, i);

			}
		}
	}
	next = NULL;
}

LexerLibrary::~LexerLibrary() {
	Release();
}

void LexerLibrary::Release() {
	//TODO maintain a list of lexers created, and delete them!
	LexerMinder *lm;
	LexerMinder *next;
	lm = first;
	while (NULL != lm) {
		next = lm->next;
		delete lm->self;
		delete lm;
		lm = next;
	}

	first = NULL;
	last = NULL;

	// Release the DLL
	if (NULL != m_hModule) {
		FreeLibrary(m_hModule);
	}
}

//------------------------------------------
//
// LexerManager
//
//------------------------------------------

int FindLastSlash(char *inp) {
	int i;
	int ret = -1;
	for (i = strlen(inp) - 1; i >= 0; i--) {
		if (inp[i] == '\\' || inp[i] == '/') {
			// if you don't like break:
			/*
			if (ret == -1)
			*/
			ret = i;
			break;
		}
	}
	return ret;
}

LexerManager::LexerManager() {
	
	UseCount++;
	if (1 == UseCount) {
		firstlm = this;
		m_bLoaded = false;
	}
}

void LexerManager::EnumerateLexers() {
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;

	char path[MAX_PATH + 1];

	GetModuleFileName(GetModuleHandle(NULL), path, MAX_PATH);

	int i = FindLastSlash(path);

	if (i == -1)
		i = strlen(path);

	SString sPath(path, 0, i);

	// Ensure a trailing slash...
	if ( sPath[sPath.size() - 1] != '/' && sPath[sPath.size() - 1] != '\\' ) {
		sPath += '\\';
	}

	SString sPattern(sPath);
	sPattern += "*.lexer";

	hFind = FindFirstFile(sPattern.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		//Found the first file...
		BOOL found = TRUE;
		SString to_open;

		while (found) {
			to_open = sPath;
			to_open += FindFileData.cFileName;
			LexerLibrary *lib = new LexerLibrary(to_open.c_str());
			if (NULL != first) {
				last->next = lib;
				last = lib;
			} else {
				first = lib;
				last = lib;
			}
			found = FindNextFile(hFind, &FindFileData);
		}

		FindClose(hFind);

	}
}

LexerManager::~LexerManager() {
	// If this is the last LexerManager to be freed then
	// we delete all of our LexerLibrarys.
	UseCount--;
	if (0 == UseCount) {
		if (NULL != first) {
			LexerLibrary *cur = first;
			LexerLibrary *next = first->next;
			while (cur) {
				delete cur;
				cur = next;
			}
			first = NULL;
			last = NULL;
		}
	}
	if (this == firstlm)
		firstlm = NULL;
}

void LexerManager::Load()
{
	if(!m_bLoaded)
	{
		m_bLoaded = true;
		EnumerateLexers();
	}
}

// Return a LexerManager, or create one and then return it.
LexerManager *LexerManager::GetInstance() {
	if(!firstlm)
		firstlm = new LexerManager;
	return firstlm;
}

LMMinder::~LMMinder()
{
	LexerManager *rem = LexerManager::firstlm;
	if(rem)
		delete rem;
}

LMMinder minder;
