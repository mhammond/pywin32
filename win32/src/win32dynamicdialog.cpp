/*
 * win32dynamicdialog.cpp - Dynamic dialog creation
 *
 * Originally part of Pythonwin as win32dlgdyn.cpp, then moved into
 * win32\src\win32dynamicdialog.cpp, and shared between Pythonwin
 * and the lighter-weight win32gui module.
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

#ifdef WIN32GUI  // being compiled from WIN32GUI
#define PYW_EXPORT
#include "python.h"
#undef PyHANDLE
#include <windows.h>
#include "commctrl.h"
#include "windowsx.h"  // For edit control hacks.

#ifdef MS_WINCE
#include "winbase.h"
#endif
#include "pywintypes.h"
#include "pywinobjects.h"
#include "tchar.h"

#define BASED_CODE
#ifndef ASSERT
#define ASSERT(a)
#endif

#else  // else being compiled from WIN32UI
#include "..\..\pythonwin\stdafx.h"
#endif

#include "win32dynamicdialog.h"
static void DwordAlign(PCHAR *ptr)
{
    size_t offset = ((ULONG_PTR)*ptr) & 0x03;
    if (offset > 0) {
        *ptr += (4 - offset);
    }
}

/*
 * CPythonDialogTemplate
 */
CPythonDialogTemplate::CPythonDialogTemplate(LPCWSTR caption, DLGTEMPLATE *tmpl, WORD fontsize, LPCWSTR font,
                                             LPCWSTR menu, LPCWSTR wclass)
{
    LPCWSTR thefont = font ? font : L"MS Sans Serif";

    m_alloc =
        sizeof(DLGTEMPLATE) + (wcslen(caption) * sizeof(caption[0])) + (wcslen(thefont) * sizeof(thefont[0])) + 20;
    if (!IS_INTRESOURCE(menu))
        m_alloc += wcslen(menu) * sizeof(menu[0]);
    if (!IS_INTRESOURCE(wclass))
        m_alloc += wcslen(wclass) * sizeof(wclass[0]);

    m_h = GlobalAlloc(GHND, m_alloc);
    DLGTEMPLATE *hdr = (DLGTEMPLATE *)GlobalLock(m_h);
    memcpy(hdr, tmpl, sizeof(DLGTEMPLATE));
    hdr->cdit = 0;
    WCHAR *ptr = (WCHAR *)((BYTE *)hdr + sizeof(DLGTEMPLATE));

    if (!IS_INTRESOURCE(menu)) {
        wcscpy(ptr, menu);
        ptr += wcslen(menu) + 1;
    }
    else if (menu) {
        *ptr++ = (WCHAR)-1;
        *ptr++ = LOWORD(menu);
    }
    else
        *ptr++ = 0;

    if (!IS_INTRESOURCE(wclass)) {
        wcscpy(ptr, wclass);
        ptr += wcslen(wclass) + 1;
    }
    else if (wclass) {
        *ptr++ = (WCHAR)-1;
        *ptr++ = LOWORD(wclass);
    }
    else
        *ptr++ = 0;

    wcscpy(ptr, caption);
    ptr += wcslen(caption) + 1;
    if (hdr->style & DS_SETFONT) {
        *ptr++ = fontsize;
        wcscpy(ptr, thefont);
        ptr += wcslen(thefont) + 1;
    }
    DwordAlign((PCHAR *)&ptr);
    size_t len = (BYTE *)ptr - (BYTE *)hdr;
    ASSERT(len <= m_alloc);
    m_len = len;
    m_ptr = hdr;
}

BOOL CPythonDialogTemplate::Add(LPCWSTR wclass, DLGITEMTEMPLATE *tmpl, LPCWSTR txt, int datalen, BYTE *data)
{
    GlobalUnlock(m_h);
    size_t len = sizeof(DLGITEMTEMPLATE) + wcslen(wclass) * sizeof(wclass[0]) + datalen + 20;
    if (txt) {
        len += wcslen(txt) * sizeof(txt[0]);
    }
    if (m_len + len > m_alloc) {
        m_alloc += len;
        m_h = GlobalReAlloc(m_h, m_alloc, 0);
        ASSERT(m_h);
    }

    DLGTEMPLATE *hdr = (DLGTEMPLATE *)GlobalLock(m_h);
    hdr->cdit++;
    DLGITEMTEMPLATE *ctrl = (DLGITEMTEMPLATE *)((char *)hdr + m_len);
    memcpy(ctrl, tmpl, sizeof(DLGITEMTEMPLATE));
    WCHAR *ptr = (WCHAR *)((char *)ctrl + sizeof(DLGITEMTEMPLATE));
    wcscpy(ptr, wclass);
    ptr += wcslen(ptr) + 1;
    if (txt) {
        wcscpy(ptr, txt);
        ptr += wcslen(ptr) + 1;
    }
    else {
        *ptr++ = 0;
    }

    *ptr++ = (WORD)datalen;
    if (datalen) {
        ASSERT(data);
        memcpy(ptr, data, datalen);
        ptr = (WCHAR *)(((BYTE *)ptr) + datalen);
    }
    DwordAlign((PCHAR *)&ptr);
    len = (BYTE *)ptr - (BYTE *)hdr;
    ASSERT(len <= m_alloc);
    m_len = len;
    m_ptr = hdr;
    return TRUE;
}

BOOL CPythonDialogTemplate::Add(WORD wclass, DLGITEMTEMPLATE *tmpl, LPCWSTR txt)
{
    GlobalUnlock(m_h);
    size_t len = sizeof(DLGITEMTEMPLATE) + 20;
    if (txt) {
        len += wcslen(txt) * sizeof(txt[0]);
    }
    if (m_len + len > m_alloc) {
        m_alloc += len;
        m_h = GlobalReAlloc(m_h, m_alloc, 0);
        ASSERT(m_h);
    }

    DLGTEMPLATE *hdr = (DLGTEMPLATE *)GlobalLock(m_h);
    hdr->cdit++;
    DLGITEMTEMPLATE *ctrl = (DLGITEMTEMPLATE *)((char *)hdr + m_len);
    memcpy(ctrl, tmpl, sizeof(DLGITEMTEMPLATE));
    WCHAR *ptr = (WCHAR *)((char *)ctrl + sizeof(DLGITEMTEMPLATE));
    *ptr++ = (WORD)-1;
    *ptr++ = wclass;

    if (txt) {
        wcscpy(ptr, txt);
        ptr += wcslen(txt) + 1;
    }
    else {
        *ptr++ = 0;
    }
    *ptr++ = 0;
    DwordAlign((PCHAR *)&ptr);
    len = (BYTE *)ptr - (BYTE *)hdr;
    ASSERT(len <= m_alloc);
    m_len = len;
    m_ptr = hdr;
    return TRUE;
}

void CPythonDialogTemplate::Get(DLGTEMPLATE *tmpl)
{
    DLGTEMPLATE *hdr = (DLGTEMPLATE *)GlobalLock(m_h);
    memcpy(tmpl, hdr, sizeof(DLGTEMPLATE));
    GlobalUnlock(m_h);
}

void CPythonDialogTemplate::Set(DLGTEMPLATE *tmpl)
{
    DLGTEMPLATE *hdr = (DLGTEMPLATE *)GlobalLock(m_h);
    memcpy(hdr, tmpl, sizeof(DLGTEMPLATE));
    GlobalUnlock(m_h);
}

HGLOBAL CPythonDialogTemplate::ClaimTemplate()
{
    register HGLOBAL h = m_h;
    m_h = NULL;
    m_alloc = 0;
    m_len = 0;
    m_ptr = 0;
    GlobalUnlock(h);
    return h;
}

CPythonDialogTemplate::~CPythonDialogTemplate()
{
    if (m_h) {
        GlobalUnlock(m_h);
        GlobalFree(m_h);
    }
}

///////////////////////////////////////////////////////////////////////////
// Python-specific dialog code

// Extracts a string or resource id/atom from a dialog template buffer
// and updates the pointer param to reflect how many bytes were consumed.
static PyObject *MakeResName(WCHAR **val)
{
    WCHAR *ptr = *val;
    PyObject *obj = NULL;
    if (*ptr == (WORD)-1) {
        ptr++;
        obj = PyInt_FromLong((WORD)*ptr++);
    }
    else if (*ptr != (WORD)0) {
        obj = PyWinObject_FromWCHAR(ptr);
        ptr += wcslen(ptr) + 1;
    }
    else {
        // This used to return None instead of 0 for reasons unknown
        obj = PyLong_FromLong(0);
        ptr++;
    }
    *val = ptr;
    return obj;
}

// Given a pointer to a dialog hdr template, return a Python list to match it
/* Also need to handle extended templates (DLGTEMPLATEEX), quoting from MSDN:
    To distinguish between a standard template and an extended template,
    check the first 16-bits of a dialog box template.
    In an extended template, the first WORD is 0xFFFF;
    any other value indicates a standard template.
*/
static PyObject *MakeListFromDlgHdr(LPVOID *tplin, int &items)
{
    PyObject *obitem;
    PyObject *ret = PyList_New(7);
    if (ret == NULL)
        return NULL;
    DLGTEMPLATE *tpl = (LPDLGTEMPLATE)*tplin;
    WCHAR *ptr = (WCHAR *)((char *)tpl + sizeof(DLGTEMPLATE));

    // Parse menu and window class out first, since they have to be consumed to get to the
    //	caption, which is used as first item
    // Parameter 5 - Menu
    obitem = MakeResName(&ptr);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 5, obitem);

    // Parameter 6 - Window Class
    obitem = MakeResName(&ptr);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 6, obitem);

    // Parameter 0 - Caption
    obitem = PyWinObject_FromWCHAR(ptr);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 0, obitem);
    ptr += wcslen(ptr) + 1;

    // Parameter 1 - Bounds
    obitem = Py_BuildValue("(hhhh)", tpl->x, tpl->y, tpl->cx, tpl->cy);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 1, obitem);

    // Parameter 2 - Style
    obitem = PyLong_FromUnsignedLong(tpl->style);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 2, obitem);

    // Parameter 3 - Extended Style
    obitem = PyLong_FromUnsignedLong(tpl->dwExtendedStyle);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 3, obitem);

    // Parameter 4 - Font tuple
    if (tpl->style & DS_SETFONT) {
        WORD fontsize = (WORD)*ptr++;
        obitem = Py_BuildValue("HN", fontsize, PyWinObject_FromWCHAR(ptr));
        ptr += wcslen(ptr) + 1;
    }
    else {
        obitem = Py_None;
        Py_INCREF(Py_None);
    }
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 4, obitem);

    DwordAlign((PCHAR *)&ptr);
    *tplin = ptr;
    items = tpl->cdit;
    return ret;
}

// Given a pointer to a dialog item template, return a Python list to match it
static PyObject *MakeListFromDlgItem(LPVOID *tplin)
{
    PyObject *obitem;
    PyObject *ret = PyList_New(7);
    if (ret == NULL)
        return NULL;
    DLGITEMTEMPLATE *tpl = (LPDLGITEMTEMPLATE)*tplin;

    // Parameter 0 - Window class
    WCHAR *ptr = (WCHAR *)((char *)tpl + sizeof(DLGITEMTEMPLATE));
    if (*ptr == (WCHAR)-1) {
        ptr++;
        obitem = PyInt_FromLong((WORD)*ptr++);
    }
    else {
        LPWSTR wc = LPWSTR(ptr);
        obitem = PyWinObject_FromWCHAR(wc);
        ptr += wcslen(wc) + 1;
    }
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 0, obitem);

    // Parameter 1 - Text
    LPWSTR txt = LPWSTR(ptr);
    obitem = PyWinObject_FromWCHAR(txt);
    ptr += wcslen(txt) + 1;
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 1, obitem);

    // Parameter 2 - ID
    obitem = PyInt_FromLong(tpl->id);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 2, obitem);

    // Parameter 3 - Bounds
    obitem = Py_BuildValue("(hhhh)", tpl->x, tpl->y, tpl->cx, tpl->cy);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 3, obitem);

    // Parameter 4 - Style
    obitem = PyLong_FromUnsignedLong(tpl->style);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 4, obitem);

    // Parameter 5 - Extended Style
    obitem = PyLong_FromUnsignedLong(tpl->dwExtendedStyle);
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 5, obitem);

    // Parameter 6 - Extra data
    WORD datalen = *ptr++;
    if (datalen > 0) {
        obitem = PyString_FromStringAndSize((char *)ptr, datalen);
        ptr = (WCHAR *)(((char *)ptr) + datalen);
    }
    else {
        obitem = Py_None;
        Py_INCREF(Py_None);
    }
    if (obitem == NULL) {
        Py_DECREF(ret);
        return NULL;
    }
    PyList_SET_ITEM(ret, 6, obitem);

    DwordAlign((PCHAR *)&ptr);
    *tplin = ptr;

    return ret;
}

// Given a dialog resource, return a Python object to match it
PYW_EXPORT PyObject *MakeDlgListFromResource(HGLOBAL res)
{
    void *t = (void *)res;
    // Extended dialog templates have a different format which is not handled yet.
    //	They can be identified by 0xFFFF in the second WORD of data.
    DWORD tmp = *(DWORD *)t;
    if (HIWORD(tmp) == 0xFFFF) {
        PyErr_SetString(PyExc_NotImplementedError, "Extended dialog templates are not yet supported");
        return NULL;
    }

    PyObject *list = PyList_New(0);
    if (list == NULL)
        return NULL;
    int n = 0;
    PyObject *obitem = MakeListFromDlgHdr(&t, n);
    if (obitem == NULL || PyList_Append(list, obitem) == -1) {
        Py_DECREF(list);
        Py_XDECREF(obitem);
        return NULL;
    }
    Py_DECREF(obitem);
    for (int i = 0; i < n; i++) {
        obitem = MakeListFromDlgItem(&t);
        if (obitem == NULL || PyList_Append(list, obitem) == -1) {
            Py_DECREF(list);
            Py_XDECREF(obitem);
            return NULL;
        }
        Py_DECREF(obitem);
    }
    return list;
}

// Given a Python dialog header list, parse out a matching CPythonDialogTemplate
// @object PyDLGTEMPLATE|A tuple of items describing a dialog box, that can be used to create the dialog.
// @pyseeapi DLGTEMPLATE
static CPythonDialogTemplate *ParseDlgHdrList(PyObject *tmpl)
{
    PyObject *obhdr = PySequence_Tuple(tmpl);
    if (obhdr == NULL)
        return NULL;
    WCHAR *caption = NULL, *menu = NULL, *wclass = NULL, *fontname = NULL;
    PyObject *obcaption, *obfontname, *obfont = Py_None, *obmenu = Py_None, *obwclass = Py_None;
    PyObject *obexstyle = Py_None;
    DLGTEMPLATE tpl = {0, 0, 0, 0, 0, 0, 0};
    WORD fontsize;

    CPythonDialogTemplate *ret = NULL;

    if (!PyArg_ParseTuple(obhdr, "O(hhhh)k|OOOO:DLGTEMPLATE", &obcaption, &tpl.x, &tpl.y, &tpl.cx, &tpl.cy, &tpl.style,
                          &obexstyle, &obfont, &obmenu, &obwclass))
        goto cleanup;

    // @tupleitem 0|string|caption|The caption for the window
    // @tupleitem 1|(int,int,int,int)|(x,y,cx,cy)|The bounding rectange for the dialog.
    // @tupleitem 2|int|style|The style bits for the dialog.  Combination of WS_* and DS_* constants.
    // Note that the DS_SETFONT style need never be specified - it is determined by the font item (below)
    // <nl>See MSDN documentation on Dialog Boxes for allowable values.
    // @tupleitem 3|int|extStyle|The extended style bits for the dialog. Defaults to 0 if not passed and None is
    // supported for backwards compatibility.
    // @tupleitem 4|(int, string)|(fontSize, fontName)|A tuple describing the font, or None if the system default font
    // is to be used.
    // @tupleitem 5|<o PyResourceId>|menuResource|The resource ID of the menu to be used for the dialog, or None for no
    // menu.
    // @tupleitem 6|<o PyResourceId>|windowClass|Window class name or atom as returned from RegisterWindowClass.
    // Defaults to None.

    if (!PyWinObject_AsWCHAR(obcaption, &caption, FALSE))
        goto cleanup;
    if (!PyWinObject_AsResourceIdW(obmenu, &menu, TRUE))
        goto cleanup;
    if (!PyWinObject_AsResourceIdW(obwclass, &wclass, TRUE))
        goto cleanup;
    if (obexstyle != Py_None) {
        tpl.dwExtendedStyle = PyLong_AsUnsignedLong(obexstyle);
        if (tpl.dwExtendedStyle == -1 && PyErr_Occurred())
            goto cleanup;
    }

    tpl.style &= ~DS_SETFONT;
    if (obfont != Py_None) {
        if (!PyArg_ParseTuple(obfont, "HO", &fontsize, &obfontname))
            goto cleanup;
        if (!PyWinObject_AsWCHAR(obfontname, &fontname, TRUE))
            goto cleanup;
        tpl.style |= DS_SETFONT;
    }
    ret = new CPythonDialogTemplate(caption, &tpl, fontsize, fontname, menu, wclass);

cleanup:
    Py_DECREF(obhdr);
    PyWinObject_FreeWCHAR(caption);
    PyWinObject_FreeWCHAR(fontname);
    PyWinObject_FreeResourceId(menu);
    PyWinObject_FreeResourceId(wclass);
    return ret;
}

// Given a Python dialog item list, parse out a dialog item
// @object PyDLGITEMTEMPLATE|A tuple describing a control in a dialog box.
// @pyseeapi DLGITEMTEMPLATE
static BOOL ParseDlgItemList(CPythonDialogTemplate *dlg, PyObject *tmpl)
{
    // @tupleitem 0|string/int|windowClass|The window class.  If not a string, it must be in integer defining one of the
    // built-in Windows controls. If a string, it must be a pre-registered windows class name, a built-in class, or the
    // CLSID of an OLE controls. Built-in classes include:
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

    // @tupleitem 1|<o PyUnicode>|caption|Caption for the control, or None
    // @tupleitem 2|int|ID|The child ID of this control.  All children should have unique
    // IDs.  This ID can be used by <om PyCDialog.GetDlgItem> to retrieve the actual control
    // object at runtime.
    // @tupleitem 3|(int,int,int,int)|(x,y,cx,cy)|The bounding rectange for the control, relative to the upper left of
    // the dialog, in dialog units.
    // @tupleitem 4|int|style|The window style of the control (WS_* constants). Depending on the type of control,
    // other constants may also be valid (eg, BS_* for Button, ES_* for Edit controls, etc).
    // @tupleitem 5|int|extStyle|The extended style of the control.
    // @tupleitem 6|buffer|extraData|A byte string or buffer used as extra data for the control.  The value depends on
    // the control.

    BOOL ret = FALSE;
    PyObject *obwclass, *obcaption, *obdata = Py_None;
    LPWSTR wclass = NULL, caption = NULL;
    DLGITEMTEMPLATE tpl = {0, 0, 0, 0, 0, 0, 0};
    BYTE *data = NULL;
    DWORD datalen = 0;

    PyObject *obitem = PySequence_Tuple(tmpl);
    if (obitem == NULL)
        return FALSE;
    if (!PyArg_ParseTuple(obitem, "OOH|(hhhh)kkO:DLGITEMTEMPLATE", &obwclass, &obcaption, &tpl.id, &tpl.x, &tpl.y,
                          &tpl.cx, &tpl.cy, &tpl.style, &tpl.dwExtendedStyle, &obdata))
        goto cleanup;
    if (!PyWinObject_AsResourceIdW(obwclass, &wclass, FALSE))
        goto cleanup;
    if (!PyWinObject_AsWCHAR(obcaption, &caption, TRUE))
        goto cleanup;
    if (!PyWinObject_AsReadBuffer(obdata, (void **)&data, &datalen, TRUE))
        goto cleanup;

    if (IS_INTRESOURCE(wclass))
        ret = dlg->Add((WORD)wclass, &tpl, caption);
    else
        ret = dlg->Add(wclass, &tpl, caption, datalen, data);

cleanup:
    PyWinObject_FreeResourceId(wclass);
    PyWinObject_FreeWCHAR(caption);
    Py_DECREF(obitem);
    return ret;
}

// Given a Python dialog template object, parse out a dialog resource
// @object PyDialogTemplate|Sequence of items defining a dialog.
//	The first item is a <o PyDLGTEMPLATE> describing the dialog, followed by
//	zero or more <o PyDLGITEMTEMPLATE>s describing controls within the dialog.
PYW_EXPORT HGLOBAL MakeResourceFromDlgList(PyObject *tmpl)
{
    PyObject *obdlg = PySequence_Tuple(tmpl);
    if (obdlg == NULL)
        return NULL;
    Py_ssize_t size = PyTuple_GET_SIZE(obdlg);
    if (size == 0) {
        PyErr_SetString(PyExc_ValueError, "Dialog template must contain at least a PyDLGTEMPLATE");
        return NULL;
    }

    PyObject *o = PyTuple_GET_ITEM(obdlg, 0);
    CPythonDialogTemplate *dlg = ParseDlgHdrList(o);
    if (dlg == NULL)
        return NULL;

    for (register Py_ssize_t i = 1; i < size; i++) {
        if (!ParseDlgItemList(dlg, PyTuple_GET_ITEM(obdlg, i))) {
            delete dlg;
            Py_DECREF(obdlg);
            return NULL;
        }
    }
    HGLOBAL h = dlg->ClaimTemplate();
    delete dlg;
    Py_DECREF(obdlg);
    return h;
}
