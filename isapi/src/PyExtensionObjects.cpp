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

#include "stdafx.h"
#include "Utils.h"
#include "PyExtensionObjects.h"
#include "PythonEng.h"

// Asynch IO callbacks are a little tricky, as we never know how many
// callbacks a single connection might make (often each callback will trigger
// another IO request.) So we keep the Python objects used by the callback
// mechanism in a map, keyed by connection ID, of asynch callback handlers.
// Each element is a tuple of (callback, user_args). This is done rather than
// using the 'pContext' param of the callback to ensure lifetimes of the
// callback and the 'user arg' are maintained correctly. The item is removed
// as the HSE_REQ_DONE_WITH_SESSION callback is made, or of the callback
// raises an exception.
static PyObject *g_callbackMap = NULL;

BOOL SetupIOCallback(EXTENSION_CONTROL_BLOCK *ecb, PyObject *ob)
{
    if (!g_callbackMap) {
        if (!(g_callbackMap = PyDict_New()))
            return FALSE;
    }
    PyObject *key = PyLong_FromVoidPtr(ecb->ConnID);
    if (!key)
        return FALSE;
    if (0 != PyDict_SetItem(g_callbackMap, key, ob)) {
        Py_DECREF(key);
        return FALSE;
    }
    Py_DECREF(key);
    return TRUE;
}

void CleanupIOCallback(EXTENSION_CONTROL_BLOCK *ecb)
{
    if (!g_callbackMap)
        return;
    PyObject *key = PyLong_FromVoidPtr(ecb->ConnID);
    if (!key)
        return;  // ack - not much more we can do.
    if (0 != PyDict_DelItem(g_callbackMap, key))
        PyErr_Clear();
    Py_DECREF(key);
    return;
}

#define CALLBACK_ERROR(msg)        \
    {                              \
        ExtensionError(NULL, msg); \
        goto done;                 \
    }

extern "C" void WINAPI DoIOCallback(EXTENSION_CONTROL_BLOCK *ecb, PVOID pContext, DWORD cbIO, DWORD dwError)
{
    CEnterLeavePython _celp;
    CControlBlock *pcb = NULL;
    PyECB *pyECB = NULL;
    BOOL worked = FALSE;
    PyObject *callback = NULL;
    PyObject *user_arg = NULL;
    PyObject *args = NULL;
    PyObject *key = NULL;
    PyObject *ob = NULL;
    PyObject *result = NULL;

    if (!g_callbackMap)
        CALLBACK_ERROR("Callback when no callback map exists");

    key = PyLong_FromVoidPtr(ecb->ConnID);
    if (!key)
        CALLBACK_ERROR("Failed to create map key from connection ID");
    ob = PyDict_GetItem(g_callbackMap, key);
    if (!ob)
        CALLBACK_ERROR("Failed to locate map entry for this commID");
    // get the Python ECB object...
    pcb = new CControlBlock(ecb);
    pyECB = new PyECB(pcb);
    if (!pyECB || !pcb)
        CALLBACK_ERROR("Failed to create Python oject for ECB");

    // this should be impossible...
    if (!PyTuple_Check(ob) || (PyTuple_Size(ob) != 1 && PyTuple_Size(ob) != 2))
        CALLBACK_ERROR("Object in callback map not a tuple of correct size?");

    callback = PyTuple_GET_ITEM(ob, 0);
    user_arg = PyTuple_Size(ob) == 2 ? PyTuple_GET_ITEM(ob, 1) : Py_None;
    args = Py_BuildValue("(OOkk)", pyECB, user_arg, cbIO, dwError);
    if (!args)
        CALLBACK_ERROR("Failed to build callback args");
    result = PyObject_Call(callback, args, NULL);
    Py_DECREF(args);
    if (!result)
        CALLBACK_ERROR("Callback failed");
    Py_DECREF(result);
    worked = TRUE;
done:
    // If the callback failed, then its likely this request will end
    // up hanging.  So on error we nuke ourselves from the map then
    // call DoneWithSession.  We still hold the GIL, so we should be
    // safe from races...
    Py_XDECREF(pyECB);
    if (!worked) {
        // free the item from the map.
        CleanupIOCallback(ecb);
        // clobber the callback.
        ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_IO_COMPLETION, NULL, NULL, NULL);
        // and tell IIS there we are done with an error.
        DWORD status = HSE_STATUS_ERROR;
        ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_DONE_WITH_SESSION, &status, NULL, 0);
    }
}

// @doc
// @object HSE_VERSION_INFO|An object used by ISAPI GetExtensionVersion
PyTypeObject PyVERSION_INFOType = {
    PYISAPI_OBJECT_HEAD "HSE_VERSION_INFO",
    sizeof(PyVERSION_INFO),
    0,
    PyVERSION_INFO::deallocFunc, /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0,
    0,                        /* tp_call */
    0,                        /* tp_str */
    PyVERSION_INFO::getattro, /* tp_getattro */
    PyVERSION_INFO::setattro, /* tp_setattro */
    0,                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,       /* tp_flags */
};

PyVERSION_INFO::PyVERSION_INFO(HSE_VERSION_INFO *pvi)
{
    ob_type = &PyVERSION_INFOType;
    _Py_NewReference(this);
    m_pvi = pvi;
}

PyVERSION_INFO::~PyVERSION_INFO() {}

PyObject *PyVERSION_INFO::getattro(PyObject *self, PyObject *obname)
{
    PyVERSION_INFO *me = (PyVERSION_INFO *)self;
    if (!me->m_pvi)
        return PyErr_Format(PyExc_RuntimeError, "VERSION_INFO structure no longer exists");
    TCHAR *name = PYISAPI_ATTR_CONVERT(obname);
    if (_tcscmp(name, _T("ExtensionDesc")) == 0) {
        return PyString_FromString(me->m_pvi->lpszExtensionDesc);
    }
    return PyObject_GenericGetAttr(self, obname);
}

int PyVERSION_INFO::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    PyVERSION_INFO *me = (PyVERSION_INFO *)self;
    TCHAR *name = PYISAPI_ATTR_CONVERT(obname);
    if (!me->m_pvi) {
        PyErr_Format(PyExc_RuntimeError, "VERSION_INFO structure no longer exists");
        return -1;
    }
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete VERSION_INFO attributes");
        return -1;
    }
    // @prop string|ExtensionDesc|The description of the extension.
    else if (_tcscmp(name, _T("ExtensionDesc")) == 0) {
        DWORD size;
        const char *bytes = PyISAPIString_AsBytes(v, &size);
        if (!bytes)
            return -1;
        if (size > HSE_MAX_EXT_DLL_NAME_LEN) {
            PyErr_Format(PyExc_ValueError, "String is too long - max of %d chars", HSE_MAX_EXT_DLL_NAME_LEN);
            return -1;
        }
        strcpy(me->m_pvi->lpszExtensionDesc, bytes);
        return 0;
    }
    else {
        return PyObject_GenericSetAttr(self, obname, v);
    }
    return 0;
}

void PyVERSION_INFO::deallocFunc(PyObject *ob) { delete (PyVERSION_INFO *)ob; }

/////////////////////////////////////////////////////////////////////
// Extension block wrapper
/////////////////////////////////////////////////////////////////////

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#define ECBOFF(e) offsetof(PyECB, e)

// @object EXTENSION_CONTROL_BLOCK|A python representation of an ISAPI
// EXTENSION_CONTROL_BLOCK.
struct PyMemberDef PyECB::members[] = {{"Version", T_INT, ECBOFF(m_version), READONLY},
                                       {"TotalBytes", T_INT, ECBOFF(m_totalBytes), READONLY},
                                       {"AvailableBytes", T_INT, ECBOFF(m_available), READONLY},
                                       {"HttpStatusCode", T_INT, ECBOFF(m_HttpStatusCode)},
                                       {NULL}};

static struct PyMethodDef PyECB_methods[] = {
    {"write", PyECB::WriteClient, 1},  // @pymeth write|A synonym for WriteClient, this allows you to 'print >> ecb'
    {"WriteClient", PyECB::WriteClient, 1},                  // @pymeth WriteClient|
    {"GetServerVariable", PyECB::GetServerVariable, 1},      // @pymeth GetServerVariable|
    {"ReadClient", PyECB::ReadClient, 1},                    // @pymeth ReadClient|
    {"SendResponseHeaders", PyECB::SendResponseHeaders, 1},  // @pymeth SendResponseHeaders|
    {"SetFlushFlag", PyECB::SetFlushFlag, 1},                // @pymeth SetFlushFlag|
    {"TransmitFile", PyECB::TransmitFile, 1},                // @pymeth TransmitFile|
    {"MapURLToPath", PyECB::MapURLToPath, 1},                // @pymeth MapURLToPath|

    {"DoneWithSession", PyECB::DoneWithSession, 1},  // @pymeth DoneWithSession|
    {"close", PyECB::DoneWithSession, 1},            // @pymeth close|A synonym for DoneWithSession.
    {"Redirect", PyECB::Redirect, 1},                // @pymeth Redirect|
    {"IsKeepAlive", PyECB::IsKeepAlive, 1},          // @pymeth IsKeepAlive|
    {"GetAnonymousToken", PyECB::GetAnonymousToken,
     1},  // @pymeth GetAnonymousToken|Calls ServerSupportFunction with HSE_REQ_GET_ANONYMOUS_TOKEN or
          // HSE_REQ_GET_UNICODE_ANONYMOUS_TOKEN
    {"GetImpersonationToken", PyECB::GetImpersonationToken, 1},  // @pymeth GetImpersonationToken|
    {"IsKeepConn", PyECB::IsKeepConn, 1},  // @pymeth IsKeepConn|Calls ServerSupportFunction with HSE_REQ_IS_KEEP_CONN
    {"ExecURL", PyECB::ExecURL, 1},        // @pymeth ExecURL|Calls ServerSupportFunction with HSE_REQ_EXEC_URL
    {"GetExecURLStatus", PyECB::GetExecURLStatus,
     1},  // @pymeth GetExecURLStatus|Calls ServerSupportFunction with HSE_REQ_GET_EXEC_URL_STATUS
    {"IOCompletion", PyECB::IOCompletion,
     1},  // @pymeth IOCompletion|Calls ServerSupportFunction with HSE_REQ_IO_COMPLETION
    {"ReportUnhealthy", PyECB::ReportUnhealthy,
     1},  // @pymeth ReportUnhealthy|Calls ServerSupportFunction with HSE_REQ_REPORT_UNHEALTHY
    {NULL}};
// @pymeth IOCallback|A placeholder for a user-supplied callback function.

PyTypeObject PyECBType = {
    PYISAPI_OBJECT_HEAD "EXTENSION_CONTROL_BLOCK",
    sizeof(PyECB),
    0,
    PyECB::deallocFunc, /* tp_dealloc */
    0,                  /* tp_print */
    0,                  /* tp_getattr */
    0,                  /* tp_setattr */
    0,
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0,
    0,                  /* tp_call */
    0,                  /* tp_str */
    PyECB::getattro,    /* tp_getattro */
    PyECB::setattro,    /* tp_setattro */
    0,                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    0,                  /* tp_doc */
    0,                  /* tp_traverse */
    0,                  /* tp_clear */
    0,                  /* tp_richcompare */
    0,                  /* tp_weaklistoffset */
    0,                  /* tp_iter */
    0,                  /* tp_iternext */
    PyECB_methods,      /* tp_methods */
    PyECB::members,     /* tp_members */
    0,                  /* tp_getset */
    0,                  /* tp_base */
    0,                  /* tp_dict */
    0,                  /* tp_descr_get */
    0,                  /* tp_descr_set */
    0,                  /* tp_dictoffset */
    0,                  /* tp_init */
    0,                  /* tp_alloc */
    0,                  /* tp_new */
};

PyECB::PyECB(CControlBlock *pcb)
    :

      m_version(0),     // @prop integer|Version|Version info of this spec (read-only)
      m_totalBytes(0),  // @prop int|TotalBytes|Total bytes indicated from client
      m_available(0),   // @prop int|AvailableBytes|Available number of bytes
      m_HttpStatusCode(
          0)  // @prop int|HttpStatusCode|The status of the current transaction when the request is completed.

// <keep a blank line above this for autoduck!> these props are managed manually...
// @prop bytes|Method|REQUEST_METHOD
// @prop long|ConnID|Context number (read-only)
// @prop bytes|QueryString|QUERY_STRING
// @prop bytes|PathInfo|PATH_INFO
// @prop bytes|PathTranslated|PATH_TRANSLATED
// @prop bytes|AvailableData|Pointer to cbAvailable bytes
// @prop bytes|ContentType|Content type of client data
// @prop bytes|LogData|log data string
{
    ob_type = &PyECBType;
    _Py_NewReference(this);

    m_pcb = pcb;

    EXTENSION_CONTROL_BLOCK *pecb = pcb->GetECB();

    m_version = pecb->dwVersion;
    // load up the simple integers etc into members so we can use normal
    // python structmember T_ macros.
    m_HttpStatusCode = pecb->dwHttpStatusCode;
    m_totalBytes = pecb->cbTotalBytes;
    m_available = pecb->cbAvailable;
}

PyECB::~PyECB()
{
    if (m_pcb)
        delete m_pcb;
}

PyObject *PyECB::getattro(PyObject *self, PyObject *obname)
{
    TCHAR *name = PYISAPI_ATTR_CONVERT(obname);

    if (_tcscmp(name, _T("softspace")) == 0)  // help 'print' semantics.
        return PyInt_FromLong(1);

    EXTENSION_CONTROL_BLOCK *pecb = ((PyECB *)self)->m_pcb->GetECB();

    if (_tcscmp(name, _T("Method")) == 0)
        return PyString_FromString(pecb->lpszMethod);

    if (_tcscmp(name, _T("QueryString")) == 0)
        return PyString_FromString(pecb->lpszQueryString);

    if (_tcscmp(name, _T("PathInfo")) == 0)
        return PyString_FromString(pecb->lpszPathInfo);

    if (_tcscmp(name, _T("PathTranslated")) == 0)
        return PyString_FromString(pecb->lpszPathTranslated);

    if (_tcscmp(name, _T("AvailableData")) == 0)
        return PyString_FromStringAndSize((const char *)pecb->lpbData, pecb->cbAvailable);

    if (_tcscmp(name, _T("ContentType")) == 0)
        return PyString_FromString(pecb->lpszContentType);

    if (_tcscmp(name, _T("LogData")) == 0)
        return PyErr_Format(PyExc_AttributeError, "LogData attribute can only be set");

    if (_tcscmp(name, _T("ConnID")) == 0)
        return PyLong_FromVoidPtr(pecb->ConnID);

    return PyObject_GenericGetAttr(self, obname);
}

int PyECB::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete ECB attributes");
        return -1;
    }
    TCHAR *name = PYISAPI_ATTR_CONVERT(obname);

    if (_tcscmp(name, _T("HttpStatusCode")) == 0) {
        PyECB *pecb = (PyECB *)self;
        DWORD status = PyInt_AsLong(v);
        pecb->m_HttpStatusCode = status;
        if (pecb->m_pcb)
            pecb->m_pcb->SetStatus(status);
        return 0;
    }

    if (_tcscmp(name, _T("LogData")) == 0) {
        const char *logMsg = PyISAPIString_AsBytes(v);
        if (!logMsg)
            return -1;
        PyECB *pecb = (PyECB *)self;
        if (pecb->m_pcb)
            pecb->m_pcb->SetLogMessage(logMsg);
        return 0;
    }

    PyErr_SetString(PyExc_AttributeError,
                    "can't modify read only ECB attributes only HTTPStatusCode and LogData can be changed.");
    return -1;
}

void PyECB::deallocFunc(PyObject *ob) { delete (PyECB *)ob; }

// @pymethod int|EXTENSION_CONTROL_BLOCK|WriteClient|
PyObject *PyECB::WriteClient(PyObject *self, PyObject *args)
{
    BOOL bRes = FALSE;
    char *buffer = NULL;
    Py_ssize_t buffLenIn = 0;
    DWORD buffLenOut = 0;
    int reserved = 0;

    PyECB *pecb = (PyECB *)self;
    // @pyparm string/buffer|data||The data to write
    // @pyparm int|reserved|0|
    if (!PyArg_ParseTuple(args, "s#|l:WriteClient", &buffer, &buffLenIn, &reserved))
        return NULL;

    DWORD bytesWritten = 0;
    buffLenOut = Py_SAFE_DOWNCAST(buffLenIn, Py_ssize_t, DWORD);
    if (pecb->m_pcb) {
        Py_BEGIN_ALLOW_THREADS bRes = pecb->m_pcb->WriteClient(buffer, &buffLenOut, reserved);
        Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("WriteClient");
    }
    return PyInt_FromLong(buffLenOut);
    // @rdesc the result is the number of bytes written.
}

// @pymethod string|EXTENSION_CONTROL_BLOCK|GetServerVariable|
// @rdesc The result is a string object, unless the server variable name
// begins with 'UNICODE_', in which case it is a unicode object - see the
// ISAPI docs for more details.
PyObject *PyECB::GetServerVariable(PyObject *self, PyObject *args)
{
    BOOL bRes = FALSE;
    char *variable = NULL;
    PyObject *def = NULL;

    PyECB *pecb = (PyECB *)self;
    // @pyparm string|variable||
    // @pyparm object|default||If specified, the function will return this
    // value instead of raising an error if the variable could not be fetched.
    if (!PyArg_ParseTuple(args, "s|O:GetServerVariable", &variable, &def))
        return NULL;

    char buf[8192] = "";
    DWORD bufsize = sizeof(buf);
    char *bufUse = buf;

    if (pecb->m_pcb) {
        bRes = pecb->m_pcb->GetServerVariable(variable, buf, &bufsize);
        if (!bRes && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            // Although the IIS docs say it should be good, IIS5
            // returns -1 for 'bufsize' and MS samples show not
            // to trust it too.  Like the MS sample, we max out
            // at some value - we choose 64k.  We double each
            // time, meaning we get 3 goes around the loop
            bufUse = NULL;
            bufsize = sizeof(buf);
            for (int i = 0; i < 3; i++) {
                bufsize *= 2;
                bufUse = (char *)realloc(bufUse, bufsize);
                if (!bufUse)
                    break;
                bRes = pecb->m_pcb->GetServerVariable(variable, bufUse, &bufsize);
                if (bRes || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    break;
            }
        }
        if (!bufUse)
            return PyErr_NoMemory();
        if (!bRes) {
            if (bufUse != buf)
                free(bufUse);
            if (def) {
                Py_INCREF(def);
                return def;
            }
            return SetPyECBError("GetServerVariable");
        }
    }
    PyObject *ret = strncmp("UNICODE_", variable, 8) == 0
                        ? PyUnicode_FromWideChar((WCHAR *)bufUse, bufsize / sizeof(WCHAR))
                        : PyString_FromStringAndSize(bufUse, bufsize);
    if (bufUse != buf)
        free(bufUse);
    return ret;
}

// @pymethod string|EXTENSION_CONTROL_BLOCK|ReadClient|
PyObject *PyECB::ReadClient(PyObject *self, PyObject *args)
{
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    BOOL bRes = FALSE;
    BYTE *pBuff = NULL;
    DWORD nSize = pecb->m_totalBytes - pecb->m_available;
    // @pyparm int|nbytes||Default is to read all available data.
    if (!PyArg_ParseTuple(args, "|l:ReadClient", &nSize))
        return NULL;
    Py_BEGIN_ALLOW_THREADS assert(nSize >= 0);  // DWORD == unsigned == >= 0

    DWORD orgSize = nSize;
    DWORD bytesGot = nSize;

    pBuff = new BYTE[nSize];
    bRes = pecb->m_pcb->ReadClient(pBuff, &bytesGot);
    if (bytesGot < orgSize) {
        DWORD extraBytes = orgSize - bytesGot;
        DWORD offset = bytesGot;
        while (extraBytes > 0) {
            bytesGot = extraBytes;
            bRes = pecb->m_pcb->ReadClient(&pBuff[offset], &bytesGot);
            if (bytesGot < 1)
                break;

            extraBytes -= bytesGot;
            offset += bytesGot;
        }
        if (extraBytes > 0)
            nSize -= extraBytes;
    }

    Py_END_ALLOW_THREADS if (!bRes)
    {
        delete[] pBuff;
        return SetPyECBError("ReadClient");
    }

    PyObject *pyRes = NULL;
    if (nSize > 0)
        pyRes = PyString_FromStringAndSize((const char *)pBuff, nSize);
    else
        pyRes = PyString_FromStringAndSize("", 0);

    delete[] pBuff;

    return pyRes;
}

// The following are wrappers for the various ServerSupportFunction
// @pymethod |EXTENSION_CONTROL_BLOCK|SendResponseHeaders|Calls ServerSupportFunction with
// HSE_REQ_SEND_RESPONSE_HEADER_EX
PyObject *PyECB::SendResponseHeaders(PyObject *self, PyObject *args)
{
    BOOL bRes = FALSE;
    char *reply = NULL;
    char *headers = NULL;
    int bKeepAlive = 0;
    Py_ssize_t cchStatus, cchHeader;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    HSE_SEND_HEADER_EX_INFO SendHeaderExInfo;
    // @pyparm string|reply||
    // @pyparm string|headers||
    // @pyparm bool|keepAlive|False|
    if (!PyArg_ParseTuple(args, "s#s#|i:SendResponseHeaders", &SendHeaderExInfo.pszStatus, &cchStatus,
                          &SendHeaderExInfo.pszHeader, &cchHeader, &bKeepAlive))
        return NULL;

    SendHeaderExInfo.cchStatus = Py_SAFE_DOWNCAST(cchStatus, Py_ssize_t, DWORD);
    SendHeaderExInfo.cchHeader = Py_SAFE_DOWNCAST(cchHeader, Py_ssize_t, DWORD);
    SendHeaderExInfo.fKeepConn = (bKeepAlive) ? TRUE : FALSE;
    if (pecb->m_pcb) {
        Py_BEGIN_ALLOW_THREADS
            //  NOTE we must send Content-Length header with correct byte count
            //  in order for keep-alive to work, the bKeepAlive flag is not enough
            //  by itself..
            //  Send header
            EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();
        bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &SendHeaderExInfo, NULL, NULL);
        Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_SEND_RESPONSE_HEADER_EX)");
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifndef HSE_REQ_SET_FLUSH_FLAG
// *sigh* - need a better strategy here - maybe just insist on later SDK and
// #error with a helpful message?
#pragma message("You don't appear to have a late platform SDK that supports IIS7")
#define HSE_REQ_SET_FLUSH_FLAG (HSE_REQ_END_RESERVED + 43)
#endif

// @pymethod |EXTENSION_CONTROL_BLOCK|SetFlushFlag|Calls ServerSupportFunction with HSE_REQ_SET_FLUSH_FLAG.
PyObject *PyECB::SetFlushFlag(PyObject *self, PyObject *args)
{
    int f;
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    // @pyparm bool|flag||
    if (!PyArg_ParseTuple(args, "i:SetFlushFlag", &f))
        return NULL;

    if (pecb->m_pcb) {
        BOOL bRes;
        Py_BEGIN_ALLOW_THREADS EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();
        bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_SET_FLUSH_FLAG, (LPVOID)f, NULL, NULL);
        Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_SET_FLUSH_FLAG)");
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |EXTENSION_CONTROL_BLOCK|Redirect|Calls ServerSupportFunction with HSE_REQ_SEND_URL_REDIRECT_RESP
PyObject *PyECB::Redirect(PyObject *self, PyObject *args)
{
    BOOL bRes = FALSE;
    char *url = NULL;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    // @pyparm string|url||The URL to redirect to
    if (!PyArg_ParseTuple(args, "s:Redirect", &url))
        return NULL;

    if (pecb->m_pcb) {
        Py_BEGIN_ALLOW_THREADS bRes = pecb->m_pcb->Redirect(url);
        Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_SEND_URL_REDIRECT_RESP)");
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|GetImpersonationToken|Calls ServerSupportFunction with
// HSE_REQ_GET_IMPERSONATION_TOKEN
PyObject *PyECB::GetImpersonationToken(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetImpersonationToken"))
        return NULL;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;
    HANDLE handle;
    BOOL bRes;
    Py_BEGIN_ALLOW_THREADS bRes = pecb->m_pcb->GetImpersonationToken(&handle);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_GET_IMPERSONATION_TOKEN)");
    return PyLong_FromVoidPtr(handle);
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|GetAnonymousToken|Calls ServerSupportFunction with HSE_REQ_GET_ANONYMOUS_TOKEN
// or HSE_REQ_GET_UNICODE_ANONYMOUS_TOKEN
PyObject *PyECB::GetAnonymousToken(PyObject *self, PyObject *args)
{
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;
    EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();

    PyObject *obStr;
    // @pyparm string/unicode|metabase_path||
    if (!PyArg_ParseTuple(args, "O:GetImpersonationToken", &obStr))
        return NULL;
    HANDLE handle;
    BOOL bRes;
    if (PyString_Check(obStr)) {
        Py_BEGIN_ALLOW_THREADS bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_GET_ANONYMOUS_TOKEN,
                                                                 PyString_AS_STRING(obStr), (DWORD *)&handle, NULL);
        Py_END_ALLOW_THREADS
    }
    else if (PyUnicode_Check(obStr)) {
        Py_BEGIN_ALLOW_THREADS bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_GET_UNICODE_ANONYMOUS_TOKEN,
                                                                 PyUnicode_AS_UNICODE(obStr), (DWORD *)&handle, NULL);
        Py_END_ALLOW_THREADS
    }
    else
        return PyErr_Format(PyExc_TypeError, "must pass a string or unicode object (got %s)", obStr->ob_type->tp_name);
    if (!bRes)
        return SetPyECBError("ServerSupportFunction(HSE_REQ_GET_IMPERSONATION_TOKEN)");
    return PyLong_FromVoidPtr(handle);
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|IsKeepConn|Calls ServerSupportFunction with HSE_REQ_IS_KEEP_CONN
PyObject *PyECB::IsKeepConn(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":IsKeepConn"))
        return NULL;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;
    BOOL bRes, bIs;
    Py_BEGIN_ALLOW_THREADS bRes = pecb->m_pcb->IsKeepConn(&bIs);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_IS_KEEP_CONN)");
    return PyBool_FromLong(bIs);
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|ExecURL|Calls ServerSupportFunction with HSE_REQ_EXEC_URL
// @comm This function is only available in IIS6 and later.
PyObject *PyECB::ExecURL(PyObject *self, PyObject *args)
{
    PyObject *obInfo, *obEntity;
    HSE_EXEC_URL_INFO i;
    memset(&i, 0, sizeof(i));  // to be sure, to be sure...
    if (!PyArg_ParseTuple(args, "zzzOOi:ExecURL",
                          &i.pszUrl,           // @pyparm string|url||
                          &i.pszMethod,        // @pyparm string|method||
                          &i.pszChildHeaders,  // @pyparm string|clientHeaders||
                          &obInfo,             // @pyparm object|info||Must be None
                          &obEntity,           // @pyparm object|entity||Must be None
                          &i.dwExecUrlFlags))  // @pyparm int|flags||
        return NULL;

    if (obInfo != Py_None || obEntity != Py_None)
        return PyErr_Format(PyExc_ValueError, "info and entity params must be None");

    i.pUserInfo = NULL;
    i.pEntity = NULL;
    PyECB *pecb = (PyECB *)self;
    EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();
    if (!pecb || !pecb->Check())
        return NULL;
    BOOL bRes;
    Py_BEGIN_ALLOW_THREADS bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_EXEC_URL, &i, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_EXEC_URL)");
    Py_RETURN_NONE;
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|GetExecURLStatus|Calls ServerSupportFunction with HSE_REQ_GET_EXEC_URL_STATUS
PyObject *PyECB::GetExecURLStatus(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetExecURLStatus"))
        return NULL;

    PyECB *pecb = (PyECB *)self;
    EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();
    if (!pecb || !pecb->Check())
        return NULL;
    BOOL bRes;
    HSE_EXEC_URL_STATUS status;
    Py_BEGIN_ALLOW_THREADS bRes =
        ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_GET_EXEC_URL_STATUS, &status, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_GET_EXEC_URL_STATUS)");
    // @rdesc The result of a tuple of 3 integers - (uHttpStatusCode, uHttpSubStatus, dwWin32Error)
    // @pyseeapi HSE_EXEC_URL_STATUS
    return Py_BuildValue("HHk", status.uHttpStatusCode, status.uHttpSubStatus, status.dwWin32Error);
}

// *sob* - these autoduck comments should be closer to the actual callback impl,
// but autoduck makes life harder than it should be...
// @pymethod None|EXTENSION_CONTROL_BLOCK|IOCallback|A placeholder for a user-supplied callback function.
// @comm This is not a function you can call, it describes the signature of
// the callback function supplied to the <om EXTENSION_CONTROL_BLOCK.IOCompletion>
// function.
// @pyparm <o EXTENSION_CONTROL_BLOCK>|ecb||The extension control block that is associated with the current, active
// request.
// @pyparm object|arg||The user-supplied argument supplied to the <om EXTENSION_CONTROL_BLOCK.IOCompletion> function.
// @pyparm int|cbIO||An integer that contains the number of bytes of I/O in the last call.
// @pyparm int|dwError||The error code returned.
// @rdesc The result of this function is ignored.

// @pymethod int|EXTENSION_CONTROL_BLOCK|IOCompletion|Set a callback that will be used for handling asynchronous I/O
// operations.
// @comm If you call this multiple times, the previous callback will be discarded.
// @comm A reference to the callback and args are held until <om
// EXTENSION_CONTROL_BLOCK.DoneWithSession> is called. If the callback
// function fails, DoneWithSession(HSE_STATUS_ERROR) will automatically be
// called and no further callbacks for the ECB will be made.
PyObject *PyECB::IOCompletion(PyObject *self, PyObject *args)
{
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;
    EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();

    PyObject *obCallback;
    PyObject *obArg = NULL;
    if (!PyArg_ParseTuple(args, "O|O:IOCompletion",
                          &obCallback,  // @pyparm callable|func||The function to call, as described by the <om
                                        // EXTENSION_CONTROL_BLOCK.IOCallback> method.
                          &obArg))  // @pyparm object|arg|None|Any object which will be supplied as an argument to the
                                    // callback function.
        return NULL;

    if (!PyCallable_Check(obCallback))
        return PyErr_Format(PyExc_TypeError, "first param must be callable");
    // now we have checked the params just ignore them!  Stick args itself
    // in our map.
    if (!SetupIOCallback(ecb, args))
        return NULL;

    BOOL bRes;
    Py_BEGIN_ALLOW_THREADS bRes =
        ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_IO_COMPLETION, DoIOCallback, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_IO_COMPLETION)");
    Py_RETURN_NONE;
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|ReportUnhealthy|Calls ServerSupportFunction with HSE_REQ_REPORT_UNHEALTHY
PyObject *PyECB::ReportUnhealthy(PyObject *self, PyObject *args)
{
    char *reason = NULL;
    if (!PyArg_ParseTuple(args, "|z:ReportUnhealthy",
                          &reason))  // @pyparm string|reason|None|An optional reason to be written to the log.
        return NULL;

    PyECB *pecb = (PyECB *)self;
    EXTENSION_CONTROL_BLOCK *ecb = pecb->m_pcb->GetECB();
    if (!pecb || !pecb->Check())
        return NULL;
    BOOL bRes;
    Py_BEGIN_ALLOW_THREADS bRes = ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_REPORT_UNHEALTHY, reason, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bRes) return SetPyECBError("ServerSupportFunction(HSE_REQ_REPORT_UNHEALTHY)");
    Py_RETURN_NONE;
}

class PyTFD {
   public:
    PyTFD(PyObject *aCallable, PyObject *aArg)
    {
        callable = aCallable;
        Py_INCREF(callable);
        arg = aArg;
        Py_INCREF(arg);
    }
    void Cleanup()
    {  // NOTE: This not valid after Cleanup!
        Py_DECREF(callable);
        Py_DECREF(arg);
        delete this;  // unusual, but ok :)
    }
    PyObject *callable;
    PyObject *arg;
};

VOID WINAPI transmitFileCompletion(EXTENSION_CONTROL_BLOCK *pECB, PVOID pContext, DWORD cbIO, DWORD dwError)
{
    PyTFD *context = (PyTFD *)pContext;
    CEnterLeavePython celp;

    CControlBlock *pcb = new CControlBlock(pECB);
    // PyECB takes ownership of pcb - so when it dies, so does pcb.
    PyECB *pyECB = new PyECB(pcb);
    if (pyECB && pcb) {
        Py_INCREF(pyECB);
        PyObject *realArgs = Py_BuildValue("NOii", pyECB, context->arg, cbIO, dwError);
        if (realArgs) {
            PyObject *rc = PyObject_Call(context->callable, realArgs, NULL);
            if (rc)
                Py_DECREF(rc);
            else
                ExtensionError(pcb, "TransmitFile callback failed");
            Py_DECREF(realArgs);
            Py_DECREF(pyECB);
        }
    }
    context->Cleanup();  // let's hope we never get called again ;)
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|TransmitFile|Calls ServerSupportFunction with HSE_REQ_TRANSMIT_FILE
PyObject *PyECB::TransmitFile(PyObject *self, PyObject *args)
{
    PY_LONG_LONG hFile;  // int no good for 64bit - but can find no "pointer" format!
    HSE_TF_INFO info;
    memset(&info, 0, sizeof(info));
    PyObject *obCallback, *obCallbackParam;
    if (!PyArg_ParseTuple(args, "OOKsiiz#z#i:TransmitFile",
                          &obCallback,          // @pyparm callable|callback||
                          &obCallbackParam,     // @pyparm object|param||Any object - passed as 2nd arg to callback.
                          &hFile,               // @pyparm int|hFile||
                          &info.pszStatusCode,  // @pyparm string|statusCode||
                          &info.BytesToWrite,   // @pyparm int|BytesToWrite||
                          &info.Offset,         // @pyparm int|Offset||
                          &info.pHead,          // @pyparm string|head||
                          &info.HeadLength,
                          &info.pTail,  // @pyparm string|tail||
                          &info.TailLength,
                          &info.dwFlags  // @pyparm int|flags||
                          ))
        return NULL;
    info.hFile = (HANDLE)hFile;
    // @comm The callback is called with 4 args - (<o PyECB>, param, cbIO, dwErrCode)

    if (!PyCallable_Check(obCallback))
        return PyErr_Format(PyExc_TypeError, "Callback is not callable");
    // The 'pContext' is a pointer to a PyTFD structure.  The callback
    // also free's the memory.
    PyTFD *context = new PyTFD(obCallback, obCallbackParam);
    if (!context)
        return PyErr_NoMemory();
    info.pfnHseIO = transmitFileCompletion;
    info.pContext = context;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    BOOL bRes;
    Py_BEGIN_ALLOW_THREADS bRes = pecb->m_pcb->TransmitFile(&info);
    Py_END_ALLOW_THREADS if (!bRes)
    {
        // ack - the completion routine will not be called - clean up!
        context->Cleanup();
        return SetPyECBError("ServerSupportFunction(HSE_REQ_TRANSMIT_FILE)");
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |EXTENSION_CONTROL_BLOCK|IsKeepAlive|
// @comm This method simply checks a HTTP_CONNECTION header for 'keep-alive',
// making it fairly useless.  See <om EXTENSION_CONTROL_BLOCK.IsKeepCon>
PyObject *PyECB::IsKeepAlive(PyObject *self, PyObject *args)
{
    bool bKeepAlive = false;

    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    if (!PyArg_ParseTuple(args, ":IsKeepAlive"))
        return NULL;

    if (pecb->m_pcb) {
        Py_BEGIN_ALLOW_THREADS bKeepAlive = pecb->m_pcb->IsKeepAlive();
        Py_END_ALLOW_THREADS
    }

    return PyInt_FromLong((bKeepAlive) ? 1 : 0);
}

// @pymethod |EXTENSION_CONTROL_BLOCK|MapURLToPath|Calls ServerSupportFunction with HSE_REQ_MAP_URL_TO_PATH
PyObject *PyECB::MapURLToPath(PyObject *self, PyObject *args)
{
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;
    // todo - handle ERROR_INSUFFICIENT_BUFFER - but 4k will do for now.
    char buffer[1024 * 4];

    char *url;
    if (!PyArg_ParseTuple(args, "s:MapURLToPath", &url))
        return NULL;
    strncpy(buffer, url, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    DWORD bufSize = sizeof(buffer);
    BOOL ok;

    Py_BEGIN_ALLOW_THREADS ok = pecb->m_pcb->MapURLToPath(buffer, &bufSize);
    Py_END_ALLOW_THREADS if (!ok) return SetPyECBError("ServerSupportFunction(HSE_REQ_MAP_URL_TO_PATH)");
    return PyString_FromString(buffer);
}

// @pymethod |EXTENSION_CONTROL_BLOCK|DoneWithSession|Calls ServerSupportFunction with HSE_REQ_DONE_WITH_SESSION
PyObject *PyECB::DoneWithSession(PyObject *self, PyObject *args)
{
    DWORD status = HSE_STATUS_SUCCESS;
    PyECB *pecb = (PyECB *)self;
    if (!pecb || !pecb->Check())
        return NULL;

    // @pyparm int|status|HSE_STATUS_SUCCESS|An optional status.
    // HSE_STATUS_SUCCESS_AND_KEEP_CONN is supported by IIS to keep the connection alive.
    if (!PyArg_ParseTuple(args, "|i:DoneWithSession", &status))
        return NULL;

    // Free any resources we've allocated on behalf of this ECB - this
    // currently means just the io-completion callback.
    CleanupIOCallback(pecb->m_pcb->GetECB());

    Py_BEGIN_ALLOW_THREADS pecb->m_pcb->DoneWithSession(status);
    Py_END_ALLOW_THREADS pecb->m_pcb->Done();
    pecb->m_pcb = NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

// Setup an exception

PyObject *SetPyECBError(char *fnName, long err /*= 0*/)
{
    DWORD errorCode = err == 0 ? GetLastError() : err;
    if (PyECB_Error == NULL) {
        PyObject *mod = PyImport_ImportModule("isapi");
        if (mod)
            PyECB_Error = PyObject_GetAttrString(mod, "ExtensionError");
        else
            PyECB_Error = PyExc_RuntimeError;  // what's the alternative?
        Py_XDECREF(mod);
    }
    PyObject *v = Py_BuildValue("(izs)", errorCode, NULL, fnName);
    if (v != NULL) {
        PyErr_SetObject(PyECB_Error, v);
        Py_DECREF(v);
    }
    return NULL;
}

void InitExtensionTypes()
{
    PyType_Ready(&PyVERSION_INFOType);
    PyType_Ready(&PyECBType);
}
