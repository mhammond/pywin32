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
#if !defined(_WIN32_WCE)  // so far, none of this is supported by Windows CE
#if defined(_WIN32_WCE_)  // defined by Windows CE compiler environment

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include "Pywintypes.h"
#include <windows.h>
#include "python.h"
#include "PyNCB.h"

#include <crtdbg.h>

/***************************************************************************
** Create a new NCB Object
***************************************************************************/
static PyObject *NCB_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {0};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, ":NCB", kwlist))  // no arguments
        return NULL;
    return new PyNCB();  // call the C++ constructor
}

__declspec(dllexport) PyTypeObject PyNCBType = {
    PYWIN_OBJECT_HEAD "PyNCB",
    sizeof(PyNCB),
    0,
    PyNCB::deallocFunc, /* tp_dealloc */
    0,                  /* tp_print */
    0,                  /* tp_getattr */
    0,                  /* tp_setattr */
    0,                  /* tp_compare */
    0,                  /* tp_repr */
    0,                  /* tp_as_number */
    0,                  /* tp_as_sequence */
    0,                  /* tp_as_mapping */
    0,                  /* tp_hash */
    0,                  /* tp_call */
    0,                  /* tp_str */
    PyNCB::getattro,    /* tp_getattro */
    PyNCB::setattro,    /* tp_setattro */
    0,                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    0,                  /* tp_doc */
    0,                  /* tp_traverse */
    0,                  /* tp_clear */
    0,                  /* tp_richcompare */
    0,                  /* tp_weaklistoffset */
    0,                  /* tp_iter */
    0,                  /* tp_iternext */
    PyNCB::methods,     /* tp_methods */
    PyNCB::members,     /* tp_members */
    0,                  /* tp_getset */
    0,                  /* tp_base */
    0,                  /* tp_dict */
    0,                  /* tp_descr_get */
    0,                  /* tp_descr_set */
    0,                  /* tp_dictoffset */
    0,                  /* tp_init */
    0,                  /* tp_alloc */
    NCB_new,            /* tp_new */
};

static PyObject *PyNCB_Reset(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Reset"))
        return NULL;
    PyNCB *pyncb = (PyNCB *)self;
    pyncb->Reset();
    Py_INCREF(Py_None);
    return Py_None;
}

struct PyMethodDef PyNCB::methods[] = {
    {"Reset", PyNCB_Reset, METH_VARARGS},
    {NULL},
};

// @object NCB|A Python object that encapsulates a Win32 NCB structure.
#define OFF(e) offsetof(PyNCB, e)
struct PyMemberDef PyNCB::members[] = {
    // Note we avoid the use of 'U'nsigned types as they always force
    // a long to be returned.
    {"Command", T_BYTE, OFF(m_ncb.ncb_command), 0},  // @prop int|Command|
    {"Retcode", T_BYTE, OFF(m_ncb.ncb_retcode), 0},  // @prop int|Retcode|
    {"Lsn", T_BYTE, OFF(m_ncb.ncb_lsn), 0},          // @prop int|Lsn|
    {"Num", T_BYTE, OFF(m_ncb.ncb_num), 0},          // @prop int|Num|
    {"Bufflen", T_SHORT, OFF(m_ncb.ncb_length), 1},  // @prop int|Bufflen|read-only
    {"Callname", T_STRING, OFF(m_ncb.ncb_callname),
     0},  // @prop string|Callname| - The strings need to be space padded to 16 chars exactly
    {"Name", T_STRING, OFF(m_ncb.ncb_name),
     0},  // @prop string|Name| - The strings need to be space padded to 16 chars exactly
    {"Rto", T_BYTE, OFF(m_ncb.ncb_rto),
     0},  // @prop string|Rto| - The strings need to be space padded to 16 chars exactly
    {"Sto", T_BYTE, OFF(m_ncb.ncb_sto),
     0},  // @prop string|Sto| - The strings need to be space padded to 16 chars exactly
    {"Lana_num", T_BYTE, OFF(m_ncb.ncb_lana_num), 0},  // @prop int|Lana_num|
    {"Cmd_cplt", T_BYTE, OFF(m_ncb.ncb_cmd_cplt), 0},  // @prop int|Cmd_cplt|
    {"Event", T_LONG, OFF(m_ncb.ncb_event), 0},        // @prop int|Event|
    {"Post", T_LONG, OFF(m_ncb.ncb_post), 0},          // @prop int|Post|
    {NULL}};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PyNCB::PyNCB()
{
    ob_type = &PyNCBType;
    _Py_NewReference(this);
    memset(&m_ncb, 0, sizeof(m_ncb));
    dwStatus = 0;
    m_obbuffer = NULL;
    m_obuserbuffer = NULL;
};
/*************************************************************
 * Creates a new NCB structure from the passed in version.
 * Note: at this time it copies the Post processing
 * function pointer.  It is unclear whether support of this
 * feature could lead to reference problems.  Simlar issues with
 * the event handle.  For Future Work
 *************************************************************/
PyNCB::PyNCB(const NCB *pNCB)  // place holder

{
    ob_type = &PyNCBType;
    _Py_NewReference(this);
    memset(&m_ncb, 0, sizeof(m_ncb));
    dwStatus = 0;

    m_ncb.ncb_command = pNCB->ncb_command;
    m_ncb.ncb_retcode = pNCB->ncb_command;
    m_ncb.ncb_lsn = pNCB->ncb_lsn;
    m_ncb.ncb_num = pNCB->ncb_num;
    m_ncb.ncb_buffer = pNCB->ncb_buffer;
    m_ncb.ncb_length = pNCB->ncb_length;
    m_ncb.ncb_rto = pNCB->ncb_rto;
    m_ncb.ncb_sto = pNCB->ncb_sto;
    m_ncb.ncb_lana_num = pNCB->ncb_lana_num;
    m_ncb.ncb_cmd_cplt = pNCB->ncb_cmd_cplt;

    // should this be duplicated or just copied???

    //    if(!DuplicateHandle(GetCurrentProcess(),pNCB->ncb_event,GetCurrentProcess(),
    //									&m_ncb.ncb_event,NULL,TRUE,DUPLICATE_SAME_ACCESS))
    //	{
    //		dwStatus = 1;
    //		return;
    //	}
    //
    m_ncb.ncb_event = pNCB->ncb_event;
    //
    //	m_ncb.ncb_reserve[] is a string of 10 NULLs by definition

    strncpy((char *)m_ncb.ncb_name, (char *)pNCB->ncb_name, NCBNAMSZ);
    strncpy((char *)m_ncb.ncb_callname, (char *)pNCB->ncb_callname, NCBNAMSZ);

    // not sure about supporting this parameter!
    m_ncb.ncb_post = pNCB->ncb_post;
    m_obbuffer = NULL;
    m_obuserbuffer = NULL;
};

PyNCB::~PyNCB() { Reset(); };

void PyNCB::Reset()
{
    memset(&m_ncb, 0, sizeof(m_ncb));
    Py_XDECREF(m_obbuffer);
    Py_XDECREF(m_obuserbuffer);
    m_obbuffer = NULL;
    m_obuserbuffer = NULL;
}
void PyNCB::deallocFunc(PyObject *ob) { delete (PyNCB *)ob; };

/*********************************************************************/
PyObject *PyNCB::getattro(PyObject *self, PyObject *obname)
{
    PyNCB *This = (PyNCB *)self;
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    // Our string attributes still need special handling as the NCB isn't
    // unicode aware.
    // These 2 string attributes are logically "strings" rather than "bytes"
    if (strcmp(name, "Callname") == 0)  // these "strings" are not null terminated so build
                                        // a local representation of them and return
                                        // the Pythonized version
    {
        char TempString[17];
        TempString[16] = '\0';
        return (PyWinCoreString_FromString(strncpy((char *)TempString, (char *)This->m_ncb.ncb_callname, NCBNAMSZ)));
    }
    else if (strcmp(name, "Name") == 0) {
        char TempString[17];
        TempString[16] = '\0';
        return (PyWinCoreString_FromString(strncpy((char *)TempString, (char *)This->m_ncb.ncb_name, NCBNAMSZ)));
    }
    else if (strcmp(name, "Buffer") == 0) {
        // This is logically bytes
        if (This->m_obuserbuffer != NULL) {
            Py_INCREF(This->m_obuserbuffer);
            return This->m_obuserbuffer;
        }
        if (This->m_ncb.ncb_buffer == NULL) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return PyBuffer_FromMemory(This->m_ncb.ncb_buffer, This->m_ncb.ncb_length);
    }
    return PyObject_GenericGetAttr(self, obname);
};

/********************************************************************/
int PyNCB::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete NCB attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyNCB *This = (PyNCB *)self;

    if (strcmp(name, "Callname") == 0) {
        char *value;
        DWORD valuelen;

        if (!PyWinObject_AsString(v, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > NCBNAMSZ)  // cap string length at NCBNAMSZ(16)
            valuelen = NCBNAMSZ;

        memset(This->m_ncb.ncb_callname, ' ', NCBNAMSZ);  // make sure that the name is space padded
        strncpy((char *)This->m_ncb.ncb_callname, value, valuelen);
        if (valuelen == 0)  // source was null string
            This->m_ncb.ncb_callname[0] = '\0';
        PyWinObject_FreeString(value);
        return 0;
    }

    if (strcmp(name, "Name") == 0) {
        char *value;
        DWORD valuelen;
        if (!PyWinObject_AsString(v, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > NCBNAMSZ)  // cap string length at NCBNAMSZ(16)
            valuelen = NCBNAMSZ;

        memset(This->m_ncb.ncb_name, ' ', NCBNAMSZ);
        strncpy((char *)This->m_ncb.ncb_name, value, valuelen);
        if (valuelen == 0)  // source was null string
            This->m_ncb.ncb_callname[0] = '\0';
        PyWinObject_FreeString(value);
        return 0;
    }

    if (strcmp(name, "Buffer") == 0) {
        PyNCB *This = (PyNCB *)self;
        PyObject *ob_buf = PyObject_GetAttrString(v, "_buffer_");
        if (ob_buf == NULL) {
            PyErr_Clear();
            ob_buf = v;
            Py_INCREF(ob_buf);
        }

        void *buf;
        DWORD buflen;
        if (!PyWinObject_AsWriteBuffer(ob_buf, &buf, &buflen)) {
            Py_DECREF(ob_buf);
            return -1;
        }
        if (buflen > USHRT_MAX) {
            Py_DECREF(ob_buf);
            PyErr_Format(PyExc_ValueError, "Buffer can be at most %d bytes", USHRT_MAX);
            return -1;
        }
        Py_XDECREF(This->m_obbuffer);
        This->m_obbuffer = ob_buf;
        Py_XDECREF(This->m_obuserbuffer);
        Py_INCREF(v);
        This->m_obuserbuffer = v;
        This->m_ncb.ncb_length = (WORD)buflen;
        This->m_ncb.ncb_buffer = (PUCHAR)buf;
        return 0;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

#endif
