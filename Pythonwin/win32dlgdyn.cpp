/*
 * win32dlgdyn.cpp - Dynamic dialog creation
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

// @doc - Autoduck!

#include "stdafx.h"

#include "win32dlgdyn.h"

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

#ifdef WIN32
static void DwordAlign(PCHAR *ptr)
{
	int offset = ((int)*ptr) & 0x03;
	if (offset > 0)
	{
		*ptr += (4 - offset);
	}
}

RESSTR MakeResStr(LPCSTR x)
{
	if (!x)
	{
		return NULL;
	}

	int result = ::MultiByteToWideChar(CP_ACP, 0, x, -1, NULL, 0);
	if (result <= 0)
	{
		return NULL;
	}
	WCHAR *ws = new WCHAR[result+1];
	result = ::MultiByteToWideChar(CP_ACP, 0, x, -1, ws, result+1);
	if (result <= 0)
	{
		delete ws;
		return NULL;
	}
	ws[result] = 0;
	return ws;
}

void FreeResStr(RESCSTR x)
{
	if (x)
	{
		delete (LPWSTR)x;
	}
}

LPSTR UnmakeResStr(RESCSTR x)
{
	if (x == NULL)
	{
		return NULL;
	}

	int result = ::WideCharToMultiByte(CP_ACP, 0, x, -1, NULL, 0, NULL, NULL);
	if (result <= 0)
	{
		return NULL;
	}
	char *s = new char[result+1];
	result = ::WideCharToMultiByte(CP_ACP, 0, x, -1, s, result+1, NULL, NULL);
	if (result <= 0)
	{
		delete s;
		return NULL;
	}
	s[result] = 0;

	return s;
}

#endif

/*
 * CPythonDialogTemplate
 */

CPythonDialogTemplate::CPythonDialogTemplate(LPCSTR cpin, DLGTEMPLATE *tmpl, WORD fontsize,
	LPCSTR font, LPCSTR mnu, LPCSTR wc)
{
	static const RESCHAR BASED_CODE _font[] = _RES("MS Sans Serif");

	RESCSTR thefont = font ? MAKERESSTR(font) : _font;
	RESCSTR capt = MAKERESSTR(cpin);
	RESCSTR menu = MAKERESSTR(mnu);
	RESCSTR wclass = MAKERESSTR(wc);

	m_alloc = sizeof(DLGTEMPLATE) + alloclenR(capt) + alloclenR(thefont) + 20;
	if (HIWORD(menu) != 0)
	{
		m_alloc += alloclenR(menu);
	}
	if (HIWORD(wclass) != 0)
	{
		m_alloc += alloclenR(wclass);
	}

	m_h = GlobalAlloc(GHND, m_alloc);
	DLGTEMPLATE *hdr = (DLGTEMPLATE*)GlobalLock(m_h);
	memcpy(hdr, tmpl, sizeof(DLGTEMPLATE));
	hdr->cdit = 0;
	RESSTR ptr = (RESSTR)((char*)hdr + sizeof(DLGTEMPLATE));
	if (HIWORD(menu) != 0)
	{
		strcpyR(ptr, menu);
		ptr += strlenR(ptr) + 1;
	}
	else if (LOWORD(menu) != 0)
	{
		*ptr++ = (RESCHAR)-1;
		*(WORD*)ptr = LOWORD(menu);
		ptr += (sizeof(WORD) / sizeof(RESCHAR));
	}
	else
	{
		*ptr++ = 0;
	}
	if (HIWORD(wclass) != 0)
	{
		strcpyR(ptr, wclass);
		ptr += strlenR(ptr) + 1;
	}
	else if (LOWORD(wclass) != 0)
	{
		*ptr++ = (RESCHAR)-1;
		*(WORD*)ptr = LOWORD(wclass);
		ptr += (sizeof(WORD) / sizeof(RESCHAR));
	}
	else
	{
		*ptr++ = 0;
	}
	strcpyR(ptr, capt);
	ptr += strlenR(capt) + 1;
	if (hdr->style & DS_SETFONT)
	{
		*(WORD*)ptr = fontsize;
		ptr += (sizeof(WORD) / sizeof(RESCHAR));
		strcpyR(ptr, thefont);
		ptr += strlenR(ptr) + 1;
	}
#ifdef WIN32
	DwordAlign((PCHAR*)&ptr);
#endif
	int len = (BYTE*)ptr - (BYTE*)hdr;
	ASSERT(len <= m_alloc);
	m_len = len;
#ifdef WIN32
	m_ptr = hdr;
#else
	GlobalUnlock(m_h);
#endif
	
	if (font)
	{
		FREERESSTR(thefont);
	}
	FREERESSTR(capt);
	FREERESSTR(menu);
	FREERESSTR(wclass);
}

BOOL CPythonDialogTemplate::Add(LPCSTR wc, DLGITEMTEMPLATE *tmpl, LPCSTR text,
	int datalen, BYTE *data)
{
#ifdef WIN32
	GlobalUnlock(m_h);
#endif
	RESCSTR wclass = MAKERESSTR(wc);
	RESCSTR txt = MAKERESSTR(text);
	int len = sizeof(DLGITEMTEMPLATE) + alloclenR(wclass) + datalen + 20;
	if (txt)
	{
		len += alloclenR(txt);
	}
	if (m_len + len > m_alloc)
	{
		m_alloc += len;
		m_h = GlobalReAlloc(m_h, m_alloc, 0);
		ASSERT(m_h);
	}
	
	DLGTEMPLATE *hdr = (DLGTEMPLATE*)GlobalLock(m_h);
	hdr->cdit++;
	DLGITEMTEMPLATE *ctrl = (DLGITEMTEMPLATE*)((char*)hdr + m_len);
	memcpy(ctrl, tmpl, sizeof(DLGITEMTEMPLATE));
	RESSTR ptr = (RESSTR)((char*)ctrl + sizeof(DLGITEMTEMPLATE));
	strcpyR(ptr, wclass);
	ptr += strlenR(ptr) + 1;
	if (txt)
	{
		strcpyR(ptr, txt);
		ptr += strlenR(ptr) + 1;
	}
	else
	{
		*ptr++ = 0;
	}
#ifdef WIN32
	*ptr++ = (WORD)datalen;
#else
	*ptr++ = (char)(BYTE)datalen;
#endif
	if (datalen)
	{
		ASSERT(data);
		memcpy(ptr, data, datalen);
		ptr = (RESSTR)(((BYTE*)ptr) + datalen);
	}
#ifdef WIN32
	DwordAlign((PCHAR*)&ptr);
#endif
	len = (BYTE*)ptr - (BYTE*)hdr;
	ASSERT(len <= m_alloc);
	m_len = len;
#ifdef WIN32
	m_ptr = hdr;
#else
	GlobalUnlock(m_h);
#endif
	FREERESSTR(txt);
	FREERESSTR(wclass);

	return TRUE;
}

BOOL CPythonDialogTemplate::Add(BYTE wclass, DLGITEMTEMPLATE *tmpl, LPCSTR text)
{
#ifdef WIN32
	GlobalUnlock(m_h);
#endif
	RESCSTR txt = MAKERESSTR(text);
	int len = sizeof(DLGITEMTEMPLATE) + 20;
	if (txt)
	{
		len += alloclenR(txt);
	}
	if (m_len + len > m_alloc)
	{
		m_alloc += len;
		m_h = GlobalReAlloc(m_h, m_alloc, 0);
		ASSERT(m_h);
	}
	
	DLGTEMPLATE *hdr = (DLGTEMPLATE*)GlobalLock(m_h);
	hdr->cdit++;
	DLGITEMTEMPLATE *ctrl = (DLGITEMTEMPLATE*)((char*)hdr + m_len);
	memcpy(ctrl, tmpl, sizeof(DLGITEMTEMPLATE));
	RESSTR ptr = (RESSTR)((char*)ctrl + sizeof(DLGITEMTEMPLATE));
#ifdef WIN32
	*ptr++ = (WORD)-1;
	*ptr++ = (WORD)wclass;
#else
	*ptr++ = (BYTE)wclass;
#endif
	if (txt)
	{
		strcpyR(ptr, txt);
		ptr += strlenR(ptr) + 1;
	}
	else
	{
		*ptr++ = 0;
	}
	*ptr++ = 0;
#ifdef WIN32
	DwordAlign((PCHAR*)&ptr);
#endif
	len = (BYTE*)ptr - (BYTE*)hdr;
	ASSERT(len <= m_alloc);
	m_len = len;
#ifdef WIN32
	m_ptr = hdr;
#else
	GlobalUnlock(m_h);
#endif
	FREERESSTR(txt);

	return TRUE;
}

void CPythonDialogTemplate::Get(DLGTEMPLATE *tmpl)
{
	DLGTEMPLATE *hdr = (DLGTEMPLATE*)GlobalLock(m_h);
	memcpy(tmpl, hdr, sizeof(DLGTEMPLATE));
	GlobalUnlock(m_h);
}

void CPythonDialogTemplate::Set(DLGTEMPLATE *tmpl)
{
	DLGTEMPLATE *hdr = (DLGTEMPLATE*)GlobalLock(m_h);
	memcpy(hdr, tmpl, sizeof(DLGTEMPLATE));
	GlobalUnlock(m_h);
}

HGLOBAL CPythonDialogTemplate::ClaimTemplate()
{
	register HGLOBAL h = m_h;
	m_h = NULL;
	m_alloc = 0;
	m_len = 0;
#ifdef WIN32
	m_ptr = 0;
	GlobalUnlock(h);
#endif
	return h;
}

CPythonDialogTemplate::~CPythonDialogTemplate()
{
	if (m_h)
	{
#ifdef WIN32
		GlobalUnlock(m_h);
#endif
		GlobalFree(m_h);
	}
}


#if defined(WIN32) && !defined(NO_PYTHON)

///////////////////////////////////////////////////////////////////////////
// Python-specific dialog code

static BOOL Py_GetAsDWORD(PyObject *obj, DWORD *ptr)
{
	int i;
	if (PyArg_GetInt(obj, &i))
	{
		*ptr = (DWORD)i;
		return TRUE;
	}
	if (!PyLong_Check(obj))
		return FALSE;
	double dval = PyLong_AsDouble(obj);
	if (dval < 0 || dval > (double)ULONG_MAX)
		return FALSE;
	*(long*)ptr = (long)dval;
	return TRUE;
}

static void FillList(PyObject *list, int n)
{
	int size = PyList_Size(list);
	while (n > size)
	{
		Py_INCREF(Py_None);
		PyList_Append(list, Py_None);
		size++;
	}
}

static PyObject *MakeResName(RESSTR *val)
{
	RESSTR ptr = *val;
	PyObject *obj = NULL;
	if (*ptr == (WORD)-1)
	{
		ptr++;
		obj = PyInt_FromLong((WORD)*ptr++);
	}
	else if (*ptr != (WORD)0)
	{
		LPSTR wc = UnmakeResStr(ptr);
		obj = PyString_FromString(wc);
		delete wc;
		ptr += strlenR(ptr) + 1;
	}
	else
		ptr++;
	*val = ptr;
	return obj;
}

// Given a pointer to a dialog hdr template, return a Python list to match it
static PyObject *MakeListFromDlgHdr(LPVOID *tplin, int &items)
{
	PyObject *list = PyList_New(0);
	DLGTEMPLATE *tpl = (LPDLGTEMPLATE)*tplin;

	RESSTR ptr = (RESSTR)((char*)tpl + sizeof(DLGTEMPLATE));
	// DwordAlign((PCHAR*)&ptr);
	PyObject *menuObj = MakeResName(&ptr);
	PyObject *classObj = MakeResName(&ptr);

	// Parameter 0 - Caption
	LPSTR txt = UnmakeResStr(ptr);
	PyList_Append(list, PyString_FromString(txt));
	delete txt;
	ptr += strlenR(ptr) + 1;

	// Parameter 1 - Bounds
	PyObject *rect = PyTuple_New(4);
	PyTuple_SetItem(rect, 0, PyInt_FromLong(tpl->x));
	PyTuple_SetItem(rect, 1, PyInt_FromLong(tpl->y));
	PyTuple_SetItem(rect, 2, PyInt_FromLong(tpl->cx));
	PyTuple_SetItem(rect, 3, PyInt_FromLong(tpl->cy));
	PyList_Append(list, rect);

	// Parameter 2 - Style
	PyList_Append(list, PyLong_FromDouble(tpl->style));
	
	// Parameter 3 - Extended Style
	if (tpl->dwExtendedStyle)
		PyList_Append(list, PyLong_FromDouble(tpl->dwExtendedStyle));

	// Parameter 4 - Font tuple
	if (tpl->style & DS_SETFONT)
	{
		FillList(list, 4);
		WORD fontsize = (WORD)*ptr++;
		LPSTR txt = UnmakeResStr(ptr);
		PyObject *tuple = PyTuple_New(2);
		PyTuple_SetItem(tuple, 0, PyInt_FromLong(fontsize));
		PyTuple_SetItem(tuple, 1, PyString_FromString(txt));
		PyList_Append(list, tuple);
		delete txt;
		ptr += strlenR(ptr) + 1;
	}

	// Parameter 5 - Menu
	if (menuObj)
	{
		FillList(list, 5);
		PyList_Append(list, menuObj);
	}

	// Parameter 6 - Window Class
	if (classObj)
	{
		FillList(list, 6);
		PyList_Append(list, classObj);
	}
	
	DwordAlign((PCHAR*)&ptr);
	*tplin = ptr;
	items = tpl->cdit;

	return list;
}

// Given a pointer to a dialog item template, return a Python list to match it
static PyObject *MakeListFromDlgItem(LPVOID *tplin)
{
	PyObject *list = PyList_New(0);
	DLGITEMTEMPLATE *tpl = (LPDLGITEMTEMPLATE)*tplin;

	// Parameter 0 - Window class
	RESSTR ptr = (RESSTR)((char*)tpl + sizeof(DLGITEMTEMPLATE));
	if (*ptr == (WORD)-1)
	{
		ptr++;
		PyList_Append(list, PyInt_FromLong((WORD)*ptr++));
	}
	else
	{
		LPSTR wc = UnmakeResStr(ptr);
		PyList_Append(list, PyString_FromString(wc));
		delete wc;
		ptr += strlenR(ptr) + 1;
	}

	// Parameter 1 - Text
	LPSTR txt = UnmakeResStr(ptr);
	PyList_Append(list, PyString_FromString(txt));
	delete txt;
	ptr += strlenR(ptr) + 1;

	// Parameter 2 - ID
	PyList_Append(list, PyInt_FromLong(tpl->id));

	// Parameter 3 - Bounds
	PyObject *rect = PyTuple_New(4);
	PyTuple_SetItem(rect, 0, PyInt_FromLong(tpl->x));
	PyTuple_SetItem(rect, 1, PyInt_FromLong(tpl->y));
	PyTuple_SetItem(rect, 2, PyInt_FromLong(tpl->cx));
	PyTuple_SetItem(rect, 3, PyInt_FromLong(tpl->cy));
	PyList_Append(list, rect);

	// Parameter 4 - Style
	PyList_Append(list, PyLong_FromDouble(tpl->style));
	
	WORD datalen = *ptr++;
	// Parameter 5 - Extended Style
	if (tpl->dwExtendedStyle)
		PyList_Append(list, PyLong_FromDouble(tpl->dwExtendedStyle));
	else if (datalen > 0)
	{
		Py_INCREF(Py_None);
		PyList_Append(list, Py_None);
	}

	// Parameter 6 - Extra data
	if (datalen > 0)
	{
		PyList_Append(list, PyString_FromStringAndSize((char*)ptr, datalen));
		ptr = (RESSTR)(((char*)ptr) + datalen);
	}
	
	DwordAlign((PCHAR*)&ptr);
	*tplin = ptr;

	return list;
}

// Given a dialog resource, return a Python object to match it
PYW_EXPORT PyObject *MakeDlgListFromResource(HGLOBAL res)
{
	void *t = (void*)res;

	PyObject *list = PyList_New(0);
	int n = 0;
	PyList_Append(list, MakeListFromDlgHdr(&t, n));
	for (int i = 0; i < n; i++)
	{
		PyList_Append(list, MakeListFromDlgItem(&t));
	}

	return list;
}

// Given a Python dialog header list, parse out a matching CPythonDialogTemplate
// @object Dialog Header Tuple|A tuple describing a dialog box, that can be used to create the dialog.
// @comm For further information, see the win32 SDK for documentation on the DLGTEMPLATE structure.
static CPythonDialogTemplate *ParseDlgHdrList(PyObject *tmpl)
{
	if (!PyList_Check(tmpl))
		return NULL;
	int size = PyList_Size(tmpl);
	if (size < 3 || size > 7)
		return NULL;

	// @tupleitem 0|string|caption|The caption for the window
	PyObject *o = PyList_GetItem(tmpl, 0);
	if (!PyString_Check(o))
		RETURN_TYPE_ERR("Window caption must be a string");

	char *capt = PyString_AsString(o);

	// @tupleitem 1|(int,int,int,int)|(x,y,cx,cy)|The bounding rectange for the dialog.
	o = PyList_GetItem(tmpl, 1);
	DLGTEMPLATE tpl;
	tpl.cdit = 0;
	int x, y, cx, cy;
	if (!PyArg_ParseTuple(o, "iiii", &x, &y, &cx, &cy))
		return NULL;
	tpl.x = (WORD)x;
	tpl.y = (WORD)y;
	tpl.cx = (WORD)cx;
	tpl.cy = (WORD)cy;

	// @tupleitem 2|int|style|The style bits for the dialog.  Combination of WS_* and DS_* constants.
	// Note that the DS_SETFONT style need never be specified - it is determined by the font item (below)
	// <nl>The following dialog style flags can be used.
	// @flag DS_3DLOOK|Gives the dialog box a nonbold font and draws three-dimensional borders around control windows in the dialog box.<nl>The DS_3DLOOK style is required only by Win32-based applications compiled for versions of Windows earlier than Windows 95 or Windows NT 4.0. The system automatically applies the three-dimensional look to dialog boxes created by applications compiled for current versions of Windows. 
	// @flag DS_ABSALIGN|Indicates that the coordinates of the dialog box are screen coordinates. If this style is not specified, Windows assumes they are client coordinates. 
	// @flag DS_CENTER|Centers the dialog box in the working area; that is, the area not obscured by the tray. 
	// @flag DS_CENTERMOUSE|Centers the mouse cursor in the dialog box. 
	// @flag DS_CONTEXTHELP|Includes a question mark in the title bar of the dialog box. When the user clicks the question mark, the cursor changes to a question mark with a pointer. If the user then clicks a control in the dialog box, the control receives a WM_HELP message. The control should pass the message to the dialog box procedure, which should call the WinHelp function using the HELP_WM_HELP command. The Help application displays a pop-up window that typically contains help for the control.<nl>Note that DS_CONTEXTHELP is only a placeholder. When the dialog box is created, the system checks for DS_CONTEXTHELP and, if it is there, adds WS_EX_CONTEXTHELP to the extended style of the dialog box. WS_EX_CONTEXTHELP cannot be used with the WS_MAXIMIZEBOX or WS_MINIMIZEBOX styles. 
	// @flag DS_CONTROL|Creates a dialog box that works well as a child window of another dialog box, much like a page in a property sheet. This style allows the user to tab among the control windows of a child dialog box, use its accelerator keys, and so on. 
	// @flag DS_FIXEDSYS|Causes the dialog box to use the SYSTEM_FIXED_FONT instead of the default SYSTEM_FONT. SYSTEM_FIXED_FONT is a monospace font compatible with the System font in Windows versions earlier than 3.0. 
	// @flag DS_LOCALEDIT|Applies to 16-bit applications only. This style directs edit controls in the dialog box to allocate memory from the application's data segment. Otherwise, edit controls allocate storage from a global memory object. 
	// @flag DS_MODALFRAME|Creates a dialog box with a modal dialog-box frame that can be combined with a title bar and System menu by specifying the WS_CAPTION and WS_SYSMENU styles. 
	// @flag DS_NOFAILCREATE|Windows 95 only: Creates the dialog box even if errors occur - for example, if a child window cannot be created or if the system cannot create a special data segment for an edit control. 
	// @flag DS_NOIDLEMSG|Suppresses WM_ENTERIDLE messages that Windows would otherwise send to the owner of the dialog box while the dialog box is displayed. 
	// @flag DS_SETFOREGROUND|Causes the system to use the SetForegroundWindow function to bring the dialog box to the foreground. 
	// @flag DS_SYSMODAL|Creates a system-modal dialog box. This style causes the dialog box to have the WS_EX_TOPMOST style, but otherwise has no effect on the dialog box or the behavior of other windows in the system when the dialog box is displayed. 
	o = PyList_GetItem(tmpl, 2);
	if (!Py_GetAsDWORD(o, &tpl.style))
		return NULL;

	// @tupleitem 3|int|extStyle|The extended style bits for the dialog.
	tpl.dwExtendedStyle = 0;
	if (size > 3)
	{
		o = PyList_GetItem(tmpl, 3);
		if (o != Py_None && !Py_GetAsDWORD(o, &tpl.dwExtendedStyle))
			return NULL;
	}

	// @tupleitem 4|(int, string)|(fontSize, fontName)|A tuple describing the font, or None if the system default font is to be used.
	LPCSTR font = NULL;
	WORD fontsize = 8;
	tpl.style &= ~DS_SETFONT;
	if (size > 4)
	{
		o = PyList_GetItem(tmpl, 4);
		if (o != Py_None)
		{
			int tmp;
			if (!PyArg_ParseTuple(o, "is", &tmp, &font))
				return NULL;
			fontsize = (WORD)tmp;
			tpl.style |= DS_SETFONT;
		}
	}

	// @tupleitem 5|string/int|menuResource|The resource ID of the menu to be used for the dialog, or None for no menu.
	LPCSTR menu = NULL;
	if (size > 5)
	{
		o = PyList_GetItem(tmpl, 5);
		if (o != Py_None)
		{
			if (PyString_Check(o))
				menu = PyString_AsString(o);
			else if (PyInt_Check(o))
				menu = (LPCSTR)MAKELONG((WORD)(PyInt_AsLong(o)), 0);
			else
				return NULL;
		}
	}

	// @tupleitem 6|string/int|windowClass|The window class for the dialog, or None.  If an int, the value must be an atom returned from RegisterWindowClass.
	LPCSTR wclass = NULL;
	if (size > 6)
	{
		o = PyList_GetItem(tmpl, 6);
		if (o != Py_None)
		{
			if (PyString_Check(o))
				wclass = PyString_AsString(o);
			else if (PyInt_Check(o))
				wclass = (LPCSTR)MAKELONG((WORD)(PyInt_AsLong(o)), 0);
			else
				return NULL;
		}
	}

	return new CPythonDialogTemplate(capt, &tpl, fontsize, font, menu, wclass);
}

// Given a Python dialog item list, parse out a dialog item
// @object Dialog Item Tuple|A tuple describing a control in a dialog box.
// @comm For further information, see the win32 SDK for documentation on the DLGITEMTEMPLATE structure.
static BOOL ParseDlgItemList(CPythonDialogTemplate *dlg, PyObject *tmpl)
{
	if (!PyList_Check(tmpl))
		return FALSE;
	int size = PyList_Size(tmpl);
	if (size < 5 || size > 7)
		return FALSE;

	// @tupleitem 0|string/int|windowClass|The window class.  If not a string, it must be in integer defining one of the built-in Windows controls.
	// If a string, it must be a pre-registered windows class name, a built-in class, or the CLSID of an OLE controls.
	// Built-in classes include:
	// @flagh Control Type|String Class Name
	// @flag Check Box|Button
	// @flag Combo Box|ComboBox
	// @flag Command Button|Button
	// @flag Header|SysHeader32
	// @flag Label|Static
	// @flag List Box|ListBox<nl>SysListView32
	// @flag Option Button|Button
	// @flag Tab|SysTabControl32
	// @flag Text Box|Edit<nl>RICHEDIT
	// @flag Tool Bar|ToolbarWindow32
	// @flag Tool Tips|tooltips_class32<nl>tooltips_class
	// @flag Tree View|SysTreeView32
		// The built-in windows controls are:
	// @flagh Integer Value|Window Type
	// @flag 0x0080|Button 
	// @flag 0x0081|Edit 
	// @flag 0x0082|Static 
	// @flag 0x0083|List box 
	// @flag 0x0084|Scroll bar 
	// @flag 0x0085|Combo box 
	PyObject *o = PyList_GetItem(tmpl, 0);
	BOOL isBuiltin;
	LPCSTR wclass;
	int bclass;
	if (PyString_Check(o))
	{
		wclass = (LPCSTR)PyString_AsString(o);
		isBuiltin = FALSE;
	}
	else if (PyInt_Check(o))
	{
		bclass = PyInt_AsLong(o);
		isBuiltin = TRUE;
	}
	else
		return FALSE;

	if (size == 7 && isBuiltin)
		return FALSE;

	// @tupleitem 1|text|caption|Caption for the control, or None
	LPCSTR text = NULL;
	o = PyList_GetItem(tmpl, 1);
	if (PyString_Check(o))
		text = (LPCSTR)PyString_AsString(o);
	else if (o != Py_None)
		return FALSE;

	// @tupleitem 2|int|ID|The child ID of this control.  All children should have unique
	// IDs.  This ID can be used by <om PyCDialog.GetDlgItem> to retrieve the actual control
	// object at runtime.
	o = PyList_GetItem(tmpl, 2);
	if (!PyInt_Check(o))
		return FALSE;
	int id = PyInt_AsLong(o);
	DLGITEMTEMPLATE tpl;
	tpl.id = (WORD)id;

	// @tupleitem 3|(int,int,int,int)|(x,y,cx,cy)|The bounding rectange for the control, relative to the upper left of the dialog, in dialog units..
	o = PyList_GetItem(tmpl, 3);
	int x, y, cx, cy;
	if (!PyArg_ParseTuple(o, "iiii", &x, &y, &cx, &cy))
		return FALSE;
	tpl.x = (WORD)x;
	tpl.y = (WORD)y;
	tpl.cx = (WORD)cx;
	tpl.cy = (WORD)cy;

	// @tupleitem 4|int|style|The window style of the control (WS_* constants). Depending on the type of control,
	// other constants may also be valid (eg, BS_* for Button, ES_* for Edit controls, etc).
	o = PyList_GetItem(tmpl, 4);
	if (!Py_GetAsDWORD(o, &tpl.style))
		return FALSE;

	// @tupleitem 5|int|extStyle|The extended style of the control.
	tpl.dwExtendedStyle = 0;
	if (size > 5)
	{
		o = PyList_GetItem(tmpl, 5);
		if (!Py_GetAsDWORD(o, &tpl.dwExtendedStyle))
			return FALSE;
	}

	// @tupleitem 6|string|extraData|A string of bytes used as extra data for the control.  The value depends on the control.
	BYTE *data = NULL;
	int datalen = 0;
	if (size > 6)
	{
		o = PyList_GetItem(tmpl, 6);
		if (o != Py_None)
		{
			if (PyString_Check(o))
				return FALSE;
			data = (BYTE*)PyString_AsString(o);
			datalen = PyString_Size(o);
		}
	}

	if (isBuiltin)
		dlg->Add(bclass, &tpl, text);
	else
		dlg->Add(wclass, &tpl, text, datalen, data);

	return TRUE;
}

// Given a Python dialog template object, parse out a dialog resource
PYW_EXPORT HGLOBAL MakeResourceFromDlgList(PyObject *tmpl)
{
	if (!PyList_Check(tmpl))
	{
		RETURN_ERR("Passed object must be a dialog template list");
	}
	int size = PyList_Size(tmpl);
	if (size < 1)
	{
		RETURN_ERR("Passed object must be a dialog template list");
	}
	PyObject *o = PyList_GetItem(tmpl, 0);
	CPythonDialogTemplate *dlg = ParseDlgHdrList(o);
	if (dlg == NULL)
	{
		RETURN_ERR("Unable to parse dialog header");
	}
	for (register int i = 1; i < size; i++)
	{
		if (!ParseDlgItemList(dlg, PyList_GetItem(tmpl, i)))
		{
			delete dlg;
			RETURN_ERR("Unable to parse a dialog item");
		}
	}
	HGLOBAL h = dlg->ClaimTemplate();
	delete dlg;
	return h;
}

#endif
