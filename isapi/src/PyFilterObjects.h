/*
 ======================================================================
 Copyright 2002-2003 by Blackdog Software Pty Ltd.

                         All Rights Reserved

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that copyright notice and this permission
 notice appear in supporting documentation, and that the name of
 Blackdog Software not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.

 BLACKDOG SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 NO EVENT SHALL BLACKDOG SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ======================================================================
 */
#ifndef __PyHFC_H
#define __PyHFC_H

#include "FilterContext.h"
#include "tupleobject.h"

class PyFILTER_VERSION : public PyObject {
    HTTP_FILTER_VERSION *m_pfv;

   public:
    PyFILTER_VERSION(HTTP_FILTER_VERSION *pfv);
    ~PyFILTER_VERSION();

   public:
    void Reset() { m_pfv = NULL; }
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *ob);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
};

class PyHFC : public PyObject {
    CFilterContext *m_pfc;

   public:
    PyHFC(CFilterContext *pfc = NULL);
    ~PyHFC();
    void Reset() { m_pfc = NULL; }
    CFilterContext *GetFilterContext() { return m_pfc; }

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);

    // class methods
    static PyObject *GetData(PyObject *self, PyObject *args);
    static PyObject *WriteClient(PyObject *self, PyObject *args);
    static PyObject *AddResponseHeaders(PyObject *self, PyObject *args);
    static PyObject *GetServerVariable(PyObject *self, PyObject *args);
    // ServerSupportFunction implemented functions.
    static PyObject *SendResponseHeader(PyObject *self, PyObject *args);
    static PyObject *DisableNotifications(PyObject *self, PyObject *args);
    static struct PyMemberDef members[];

   protected:
    DWORD m_notificationType;
    DWORD m_revision;
    BOOL m_isSecurePort;
};

class PyURL_MAP : public PyObject {
   public:
    PyHFC *m_parent;

   public:
    PyURL_MAP(PyHFC *);
    ~PyURL_MAP();
    HTTP_FILTER_URL_MAP *GetURLMap();

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
};

class PyPREPROC_HEADERS : public PyObject {
   public:
    PyHFC *m_parent;

   public:
    PyPREPROC_HEADERS(PyHFC *);
    ~PyPREPROC_HEADERS();
    HTTP_FILTER_CONTEXT *GetFILTER_CONTEXT();
    HTTP_FILTER_PREPROC_HEADERS *GetPREPROC_HEADERS();

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
};

class PyRAW_DATA : public PyObject {
   public:
    PyHFC *m_parent;

   public:
    PyRAW_DATA(PyHFC *);
    ~PyRAW_DATA();
    HTTP_FILTER_CONTEXT *GetFILTER_CONTEXT();
    HTTP_FILTER_RAW_DATA *GetRAW_DATA();

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
};

class PyAUTHENT : public PyObject {
   public:
    PyHFC *m_parent;

   public:
    PyAUTHENT(PyHFC *);
    ~PyAUTHENT();
    HTTP_FILTER_CONTEXT *GetFILTER_CONTEXT();
    HTTP_FILTER_AUTHENT *GetAUTHENT();

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
};

class PyFILTER_LOG : public PyObject {
   public:
    PyHFC *m_parent;

   public:
    PyFILTER_LOG(PyHFC *);
    ~PyFILTER_LOG();
    HTTP_FILTER_LOG *GetFilterLog();

   public:
    // Python support
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
};

// error handling
static PyObject *PyHFC_Error = NULL;
PyObject *SetPyHFCError(char *fnName, long err = 0);

#endif  // __PyHFC_H
