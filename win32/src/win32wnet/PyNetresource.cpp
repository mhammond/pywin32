/******************************************************************
 * Copyright (c) 1998-1999 Cisco Systems, Inc. All Rights Reserved
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 *
 * CISCO SYSTEMS, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL CISCO SYSTEMS BE LIABLE FOR ANY LOST REVENUE,
 * PROFIT OR DATA, OR FOR SPECIAL, INDIRECT, CONSEQUENTIAL OR INCIDENTAL
 * DAMAGES OR ANY OTHER DAMAGES WHATSOEVER, HOWEVER CAUSED AND REGARDLESS
 * OF THE THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************/
// @doc

#if defined(_WIN32_WCE_)  // defined by the Windows CE compiler environment

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include "PyWinTypes.h"
#include "netres.h"  // C++ header file for NETRESOURCE object

static PyObject *NETRESOURCE_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {0};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, ":NETRESOURCE", kwlist))  // no arguments
        return NULL;
    return new PyNETRESOURCE();  // call the C++ constructor
}

BOOL PyWinObject_AsNETRESOURCE(PyObject *ob, NETRESOURCE **ppNetresource, BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None)  // Py_None has a direct value in C ? (from M. Hammond's code)
    {
        *ppNetresource = NULL;
    }
    else if (!PyNETRESOURCE_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyNETRESOURCE object");
        return FALSE;
    }
    else {
        *ppNetresource = ((PyNETRESOURCE *)ob)->GetNetresource();
    }
    return TRUE;
}

PyObject *PyWinObject_FromNETRESOURCE(const NETRESOURCE *pNetresource)
{
    if (pNetresource == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyNETRESOURCE(pNetresource);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "Allocating pNetresource");
    return ret;
}

// @object PyNETRESOURCE|A Python object that encapsulates a Win32 NETRESOURCE structure.
__declspec(dllexport) PyTypeObject PyNETRESOURCEType = {
    PYWIN_OBJECT_HEAD "PyNETRESOURCE",
    sizeof(PyNETRESOURCE),
    0,
    PyNETRESOURCE::deallocFunc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
#if (PY_VERSION_HEX >= 0x03000000)
    0, /* tp_as_async */
#else
    PyNETRESOURCE::compareFunc, /* tp_compare */
#endif
    0,                       /* tp_repr */
    0,                       /* tp_as_number */
    0,                       /* tp_as_sequence */
    0,                       /* tp_as_mapping */
    0,                       /* hash? */
    0,                       /* tp_call */
    0,                       /* tp_str */
    PyNETRESOURCE::getattro, /* tp_getattro */
    PyNETRESOURCE::setattro, /* tp_setattro */
    0,                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,      /* tp_flags */
    0,                       /* tp_doc */
    0,                       /* tp_traverse */
    0,                       /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    0,                       /* tp_methods */
    PyNETRESOURCE::members,  /* tp_members */
    0,                       /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    0,                       /* tp_init */
    0,                       /* tp_alloc */
    NETRESOURCE_new,         /* tp_new */
};

#define OFF(e) offsetof(PyNETRESOURCE, e)

struct PyMemberDef PyNETRESOURCE::members[] = {
    // Note we avoid the use of 'U'nsigned types as they always force
    // a long to be returned.
    {"dwScope", T_INT, OFF(m_nr.dwScope), 0},              // @prop integer|dwScope|
    {"dwType", T_INT, OFF(m_nr.dwType), 0},                // @prop integer|dwType|
    {"dwDisplayType", T_INT, OFF(m_nr.dwDisplayType), 0},  // @prop integer|dwDisplayType|
    {"dwUsage", T_INT, OFF(m_nr.dwUsage), 0},              // @prop integer|dwUsage|

    // These are handled by getattro/setattro
    {"lpLocalName", T_STRING, OFF(m_nr.lpLocalName), 0},    // @prop string|localName|
    {"lpRemoteName", T_STRING, OFF(m_nr.lpRemoteName), 0},  // @prop string|remoteName|
    {"lpComment", T_STRING, OFF(m_nr.lpComment), 0},        // @prop string|comment|
    {"lpProvider", T_STRING, OFF(m_nr.lpProvider), 0},      // @prop string|provider|
    {NULL}
    // @comm Note that in pywin32-212 and earlier, the string attributes
    // were always strings, but empty strings when the underlying Windows
    // structure had NULL.  On later pywin32 builds, these string attributes will
    // return None in such cases.
};

PyNETRESOURCE::PyNETRESOURCE(void)
{
    ob_type = &PyNETRESOURCEType;
    _Py_NewReference(this);
    memset(&m_nr, 0, sizeof(m_nr));
}

PyNETRESOURCE::PyNETRESOURCE(const NETRESOURCE *p_nr)
{
    ob_type = &PyNETRESOURCEType;
    _Py_NewReference(this);
    m_nr = *p_nr;

    // Copy strings so they can be freed in same way as when set via setattro
    // No error checking here, no way to return an error from a constructor anyway
    if (p_nr->lpProvider)
        m_nr.lpProvider = PyWin_CopyString(p_nr->lpProvider);
    if (p_nr->lpRemoteName)
        m_nr.lpRemoteName = PyWin_CopyString(p_nr->lpRemoteName);
    if (p_nr->lpLocalName)
        m_nr.lpLocalName = PyWin_CopyString(p_nr->lpLocalName);
    if (p_nr->lpComment)
        m_nr.lpComment = PyWin_CopyString(p_nr->lpComment);
}

PyNETRESOURCE::~PyNETRESOURCE(void)
{
    PyWinObject_FreeTCHAR(m_nr.lpProvider);
    PyWinObject_FreeTCHAR(m_nr.lpRemoteName);
    PyWinObject_FreeTCHAR(m_nr.lpLocalName);
    PyWinObject_FreeTCHAR(m_nr.lpComment);
}

PyObject *PyNETRESOURCE::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyNETRESOURCE *This = (PyNETRESOURCE *)self;

    if (strcmp(name, "lpProvider") == 0)
        return PyWinObject_FromTCHAR(This->m_nr.lpProvider);
    if (strcmp(name, "lpRemoteName") == 0)
        return PyWinObject_FromTCHAR(This->m_nr.lpRemoteName);
    if (strcmp(name, "lpLocalName") == 0)
        return PyWinObject_FromTCHAR(This->m_nr.lpLocalName);
    if (strcmp(name, "lpComment") == 0)
        return PyWinObject_FromTCHAR(This->m_nr.lpComment);
    return PyObject_GenericGetAttr(self, obname);
}

int PyNETRESOURCE::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete NETRESOURCE attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyNETRESOURCE *This = (PyNETRESOURCE *)self;

    TCHAR *value;
    if (strcmp(name, "lpProvider") == 0) {
        if (!PyWinObject_AsTCHAR(v, &value, TRUE))
            return -1;
        PyWinObject_FreeTCHAR(This->m_nr.lpProvider);
        This->m_nr.lpProvider = value;
        return 0;
    }
    if (strcmp(name, "lpRemoteName") == 0) {
        if (!PyWinObject_AsTCHAR(v, &value, TRUE))
            return -1;
        PyWinObject_FreeTCHAR(This->m_nr.lpRemoteName);
        This->m_nr.lpRemoteName = value;
        return 0;
    }
    if (strcmp(name, "lpLocalName") == 0) {
        if (!PyWinObject_AsTCHAR(v, &value, TRUE))
            return -1;
        PyWinObject_FreeTCHAR(This->m_nr.lpLocalName);
        This->m_nr.lpLocalName = value;
        return 0;
    }
    if (strcmp(name, "lpComment") == 0) {
        if (!PyWinObject_AsTCHAR(v, &value, TRUE))
            return -1;
        PyWinObject_FreeTCHAR(This->m_nr.lpComment);
        This->m_nr.lpComment = value;
        return 0;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

void PyNETRESOURCE::deallocFunc(PyObject *ob) { delete (PyNETRESOURCE *)ob; }

int PyNETRESOURCE::compare(PyObject *ob)  // only returns 0 or 1  (1 means equal)
{
    NETRESOURCE *p_nr;

    if (!PyWinObject_AsNETRESOURCE(ob, &p_nr, FALSE))  // sets error exception
        return -1;
    // do integer tests first
    if (m_nr.dwType != p_nr->dwType || m_nr.dwScope != p_nr->dwScope || m_nr.dwUsage != p_nr->dwUsage ||
        m_nr.dwDisplayType != p_nr->dwDisplayType)
        return 1;

    if (m_nr.lpComment && p_nr->lpComment) {
        if (_tcscmp(m_nr.lpComment, p_nr->lpComment) != 0)
            return 1;
    }
    else if (m_nr.lpComment || p_nr->lpComment)
        return 1;

    if (m_nr.lpLocalName && p_nr->lpLocalName) {
        if (_tcscmp(m_nr.lpLocalName, p_nr->lpLocalName) != 0)
            return 1;
    }
    else if (m_nr.lpLocalName || p_nr->lpLocalName)
        return 1;

    if (m_nr.lpProvider && p_nr->lpProvider) {
        if (_tcscmp(m_nr.lpProvider, p_nr->lpProvider) != 0)
            return 1;
    }
    else if (m_nr.lpProvider || p_nr->lpProvider)
        return 1;

    if (m_nr.lpRemoteName && p_nr->lpRemoteName) {
        if (_tcscmp(m_nr.lpRemoteName, p_nr->lpRemoteName) != 0)
            return 1;
    }
    else if (m_nr.lpRemoteName || p_nr->lpRemoteName)
        return 1;

    return 0;
};

int PyNETRESOURCE::compareFunc(PyObject *ob1, PyObject *ob2) { return ((PyNETRESOURCE *)ob1)->compare(ob2); }
