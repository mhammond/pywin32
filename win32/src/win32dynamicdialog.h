/*
 * win32dlgdyn.h - Dynamic dialog creation
 *
 * Copyright (C) 1995 by Motek Information Systems, Beverly Hills, CA, USA
 *
 *                       All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice and the two paragraphs following
 * it appear in all copies, and that the name of Motek Information Systems
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * 
 * MOTEK INFORMATION SYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL MOTEK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Curt Hagenlocher <curt@hagenlocher.org>
 */

#ifndef WIN32DLGDYN_H
#define WIN32DLGDYN_H

#ifndef WIN32

#pragma pack(1)
struct DLGTEMPLATE
{
	DWORD style;
	BYTE cdit;
	WORD x, y;
	WORD cx, cy;
	// char menu[]
	// char class[]
	// char caption[]
	// WORD points; (only if DS_SETFONT)
	// char fontname[]; (only if DS_SETFONT)
};

struct DLGITEMTEMPLATE
{
	WORD x, y;
	WORD cx, cy;
	WORD id;
	DWORD style;
	// union
	// {
	//	BYTE idClass;
	//	char szClass[2];
	// };
	// char text[]
	// BYTE datalen;
	// char data[datalen]
};
#pragma pack()

#endif

typedef DLGTEMPLATE *LPDLGTEMPLATE;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATE;

inline void SetDlgTemplate(DLGTEMPLATE *t, DWORD s, WORD x, WORD y, WORD w, WORD h)
{
	t->style = s;
#ifdef WIN32
	t->dwExtendedStyle = 0;
#endif
	t->cdit = 0;
	t->x = x;
	t->y = y;
	t->cx = w;
	t->cy = h;
}

inline void SetDlgItemTemplate(DLGITEMTEMPLATE *t, DWORD s, WORD x, WORD y,
	WORD w, WORD h, WORD id)
{
	t->style = s;
#ifdef WIN32
	t->dwExtendedStyle = 0;
#endif
	t->x = x;
	t->y = y;
	t->cx = w;
	t->cy = h;
	t->id = id;
}

const BYTE dlgButton = 0x80;
const BYTE dlgEdit = 0x81;
const BYTE dlgStatic = 0x82;
const BYTE dlgListbox = 0x83;
const BYTE dlgScrollbar = 0x84;
const BYTE dlgCombobox = 0x85;

class CPythonDialogTemplate
{
protected:
	HGLOBAL m_h;
	int m_alloc, m_len;
#ifdef WIN32
	DLGTEMPLATE *m_ptr;
#endif

public:
	CPythonDialogTemplate(LPCSTR capt, DLGTEMPLATE *tmpl, WORD fontsize = 8,
		LPCSTR font = NULL, LPCSTR menu = NULL, LPCSTR wclass = NULL);
	~CPythonDialogTemplate();
	BOOL Add(LPCSTR wclass, DLGITEMTEMPLATE *tmpl, LPCSTR txt = NULL,
		int datalen = 0, BYTE *data = NULL);
	BOOL Add(BYTE wclass, DLGITEMTEMPLATE *tmpl, LPCSTR txt = NULL);
	void Get(DLGTEMPLATE *tmpl);
	void Set(DLGTEMPLATE *tmpl);
#ifndef WIN32
	HGLOBAL GetTemplate() { return m_h; }
#else
	HGLOBAL GetTemplate() { return (HGLOBAL)m_ptr; }
#endif
	HGLOBAL ClaimTemplate();
};

#ifdef WIN32

#define _RES(x) L ## x
#define RESCHAR WCHAR
#define RESSTR LPWSTR
#define RESCSTR LPCWSTR
#define MAKERESSTR(x) MakeResStr(x)
#define FREERESSTR(x) FreeResStr(x)
#define strcpyR wcscpy
#define strlenR wcslen
#define alloclenR 2*wcslen

void DwordAlign(PCHAR *ptr);
RESSTR MakeResStr(LPCSTR x);
void FreeResStr(RESCSTR x);

#else

#define _RES(x) x
#define RESCHAR char
#define RESSTR LPSTR
#define RESCSTR LPCSTR
#define MAKERESSTR(x) x
#define FREERESSTR(x) NULL
#define strcpyR strcpy
#define strlenR strlen
#define alloclenR strlen

#endif

#endif
