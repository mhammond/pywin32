// SciTE - Scintilla based Text Editor
// PropSet.cc - a java style properties file module
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

// Maintain a dictionary of properties

#ifndef GTK
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "PropSet.h"
#ifdef GTK
#include <glib.h>
#define stricmp strcasecmp
#endif

// Get a line of input. If end of line escaped with '\\' then continue reading.
static bool GetFullLine(FILE *fp, char *s, int len) {
	while (len > 0) {
		char *cp = fgets(s, len, fp);
		if (!cp)
			return false;
		int last = strlen(s);
		// Remove probable trailing line terminator characters
		if ((last > 0) && ((s[last-1] == '\n') || (s[last-1] == '\r'))) {
			s[last-1] = '\0';
			last--;
		}
		if ((last > 0) && ((s[last-1] == '\n') || (s[last-1] == '\r'))) {
			s[last-1] = '\0';
			last--;
		}
		if (last == 0)	// Empty line so do not need to check for trailing '\\'
			return true;
		if (s[last-1] != '\\')
			return true;
		// Trailing '\\' so read another line
		s[last-1] = '\0';
		last--;
		s += last;
		len -= last;
	}
	return false;
}

PropSet::PropSet() {
	superPS = 0;
	size = 10;
	used = 0;
	vals = new char*[size];
}

PropSet::~PropSet() {
	superPS = 0;
	Clear();
	delete vals;
}

void PropSet::EnsureCanAddEntry() {
	if (used >= size - 2) {
		int newsize = size + 10;
		char **newvals = new char*[newsize];

		for (int i=0;i<used;i++) {
			newvals[i] = vals[i];
		}
		delete [] vals;
		vals = newvals;
		size = newsize;
	}
}

void PropSet::Set(const char *key, const char *val) {
	EnsureCanAddEntry();
	for (int i=0;i<used;i+=2) {
		if (0 == stricmp(vals[i], key)) {
			// Replace current value
			free(vals[i+1]);
			vals[i+1] = strdup(val);
			return;
		}
	}
	// Not found
	vals[used++] = strdup(key);
	vals[used++] = strdup(val);
}

void PropSet::Set(char *keyval) {
	char *eqat = strchr(keyval, '=');
	if (eqat) {
		*eqat = '\0';
		Set(keyval, eqat + 1);
		*eqat = '=';
	}
}

char *PropSet::Get(const char *key) {
	for (int i=0;i<used;i+=2) {
		if (0 == stricmp(vals[i], key)) {
			return vals[i+1];
		}
	}
	if (superPS) {
		// Failed here, so try in base property set
		return superPS->Get(key);
	} else {
		return "";
	}
}

int PropSet::GetInt(const char *key) {
	char *val = Get(key);
	if (*val)
		return atoi(val);
	else
		return 0;
}

bool isprefix(const char *target, const char *prefix) {
	while (*target && *prefix) {
		if (toupper(*target) != toupper(*prefix))
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
	for (int i=lensuffix-1; i >= 0; i--) {
		if (toupper(target[i + lentarget - lensuffix]) != toupper(suffix[i]))
			return false;
	}
	return true;
}

char *PropSet::GetWild(const char *keybase, const char *filename) {
	int lenbase = strlen(keybase);
	int lenfile = strlen(filename);
	for (int i=0;i<used;i+=2) {
		if (isprefix(vals[i], keybase)) {
			const char *keyfile = vals[i] + strlen(keybase);
			if (*keyfile == '*') {
				if (issuffix(filename, keyfile + 1)) {
					return vals[i+1];
				}
			} else if (0 == stricmp(keyfile, filename)) {
				return vals[i+1];
			} else if (0 == stricmp(vals[i], keybase)) {
				return vals[i+1];
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

char *PropSet::GetNewExpand(const char *keybase, const char *filename) {
	char *base = strdup(GetWild(keybase, filename));
	char *cpvar = strstr(base, "$(");
	while (cpvar) {
		char *cpendvar = strchr(cpvar, ')');
		if (cpendvar) {
			int lenvar = cpendvar - cpvar - 2;	// Subtract the $()
			char *var = static_cast<char *>(malloc(lenvar + 1));
			strncpy(var, cpvar+2, lenvar);
			var[lenvar] = '\0';
			char *val = GetWild(var, filename);
			int newlenbase = strlen(base) + strlen(val) - lenvar;
			char *newbase = static_cast<char *>(malloc(newlenbase));
			strncpy(newbase, base, cpvar - base);
			strcpy(newbase + (cpvar - base), val);
			strcpy(newbase + (cpvar - base) + strlen(val), cpendvar + 1);
			free(var);
			free(base);
			base = newbase;
		}
		cpvar = strstr(base, "$(");
	}
	return base;
}

void PropSet::Clear() {
	for (int i=0;i<used;i++) {
		free(vals[i]);
		vals[i] = 0;
	}
	used = 0;
}

void PropSet::Read(const char *filename) {
	//printf("Opening properties <%s>\n", filename);
	Clear();
	FILE *rcfile = fopen(filename, "rt");
	if (rcfile) {
		char linebuf[4000];
		while (GetFullLine(rcfile, linebuf, sizeof(linebuf))) {
			if (isalpha(linebuf[0]))
				Set(linebuf);
		}
		fclose(rcfile);
	} else {
		//printf("Could not open <%s>\n", filename);
	}
}

