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

typedef DLGTEMPLATE *LPDLGTEMPLATE;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATE;

inline void SetDlgTemplate(DLGTEMPLATE *t, DWORD s, WORD x, WORD y, WORD w, WORD h)
{
	t->style = s;
	t->dwExtendedStyle = 0;
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
	t->dwExtendedStyle = 0;
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
	size_t m_alloc, m_len;
	DLGTEMPLATE *m_ptr;

public:
	CPythonDialogTemplate(LPCWSTR capt, DLGTEMPLATE *tmpl, WORD fontsize = 8,
		LPCWSTR font = NULL, LPCWSTR menu = NULL, LPCWSTR wclass = NULL);
	~CPythonDialogTemplate();
	BOOL Add(LPCWSTR wclass, DLGITEMTEMPLATE *tmpl, LPCWSTR txt = NULL,
		int datalen = 0, BYTE *data = NULL);
	BOOL Add(WORD wclass, DLGITEMTEMPLATE *tmpl, LPCWSTR txt = NULL);
	void Get(DLGTEMPLATE *tmpl);
	void Set(DLGTEMPLATE *tmpl);
	HGLOBAL GetTemplate() { return (HGLOBAL)m_ptr; }
	HGLOBAL ClaimTemplate();
};

void DwordAlign(PCHAR *ptr);

#endif
