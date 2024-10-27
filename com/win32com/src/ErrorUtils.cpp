// oleerr.cpp : Defines error codes
//
#include "stdafx.h"
#include "PythonCOM.h"
#include "oaidl.h"
#include "olectl.h"  // For connection point constants.

static const WCHAR *szBadStringObject = L"<Bad String Object>";
extern PyObject *PyCom_InternalError;

void GetScodeString(SCODE sc, TCHAR *buf, int bufSize);
LPCTSTR GetScodeRangeString(SCODE sc);
LPCTSTR GetSeverityString(SCODE sc);
LPCTSTR GetFacilityString(SCODE sc);

static PyObject *PyCom_PyObjectFromIErrorInfo(IErrorInfo *, HRESULT errorhr);

static const WCHAR *traceback_prefix = L"Traceback (most recent call last):\n";

////////////////////////////////////////////////////////////////////////
//
// Server Side Errors - translate a Python exception to COM error information
//
////////////////////////////////////////////////////////////////////////

// Generically fills an EXCEP_INFO.  The scode in the EXCEPINFO
// is the HRESULT as nominated by the user.
void PyCom_ExcepInfoFromPyException(EXCEPINFO *pExcepInfo)
{
    // If the caller did not provide a valid exception info, get out now!
    if (pExcepInfo == NULL) {
        PyErr_Clear();  // must leave Python in a clean state.
        return;
    }
    PyObject *exception, *v, *tb;
    PyErr_Fetch(&exception, &v, &tb);
    if (PyCom_ExcepInfoFromPyObject(v, pExcepInfo, NULL)) {
        // done.
    }
    else {
        memset(pExcepInfo, 0, sizeof(EXCEPINFO));
        // Clear the exception raised by PyCom_ExcepInfoFromPyObject,
        // not the one we are interested in!
        PyErr_Clear();
        // Not a special exception object - do the best we can.
        WCHAR *szBaseMessage = L"Unexpected Python Error: ";
        WCHAR *szException = GetPythonTraceback(exception, v, tb);
        size_t len = wcslen(szBaseMessage) + wcslen(szException) + 1;
        WCHAR *tempBuf = new WCHAR[len];
        if (tempBuf) {
            _snwprintf(tempBuf, len, L"%s%s", szBaseMessage, szException);
            pExcepInfo->bstrDescription = SysAllocString(tempBuf);
            delete[] tempBuf;
        }
        else
            pExcepInfo->bstrDescription = SysAllocString(L"memory error allocating exception buffer!");
        pExcepInfo->bstrSource = SysAllocString(L"Python COM Server Internal Error");

        // Map some well known exceptions to specific HRESULTs
        // Note: v can be NULL. This can happen via PyErr_SetNone().
        //       e.g.: KeyboardInterrupt
        if (PyErr_GivenExceptionMatches(exception, PyExc_MemoryError))
            pExcepInfo->scode = E_OUTOFMEMORY;
        else
            // Any other common Python exceptions we should map?
            pExcepInfo->scode = E_FAIL;
    }
    Py_XDECREF(exception);
    Py_XDECREF(v);
    Py_XDECREF(tb);
    PyErr_Clear();
}

static BOOL PyCom_ExcepInfoFromServerExceptionInstance(PyObject *v, EXCEPINFO *pExcepInfo)
{
    BSTR temp;

    assert(v != NULL);
    assert(pExcepInfo != NULL);
    memset(pExcepInfo, 0, sizeof(EXCEPINFO));

    PyObject *ob = PyObject_GetAttrString(v, "description");
    if (ob && ob != Py_None) {
        if (!PyWinObject_AsBstr(ob, &temp))
            pExcepInfo->bstrDescription = SysAllocString(szBadStringObject);
        else
            pExcepInfo->bstrDescription = temp;
    }
    else {
        // No description - leave it empty.
        PyErr_Clear();
    }
    Py_XDECREF(ob);

    ob = PyObject_GetAttrString(v, "source");
    if (ob && ob != Py_None) {
        if (!PyWinObject_AsBstr(ob, &temp))
            pExcepInfo->bstrSource = SysAllocString(szBadStringObject);
        else
            pExcepInfo->bstrSource = temp;
    }
    else
        PyErr_Clear();
    Py_XDECREF(ob);

    ob = PyObject_GetAttrString(v, "helpfile");
    if (ob && ob != Py_None) {
        if (!PyWinObject_AsBstr(ob, &temp))
            pExcepInfo->bstrHelpFile = SysAllocString(szBadStringObject);
        else
            pExcepInfo->bstrHelpFile = temp;
    }
    else
        PyErr_Clear();
    Py_XDECREF(ob);

    ob = PyObject_GetAttrString(v, "code");
    if (ob && ob != Py_None) {
        PyObject *temp = PyNumber_Long(ob);
        if (temp) {
            pExcepInfo->wCode = (unsigned short)PyLong_AsLong(temp);
            Py_DECREF(temp);
        }  // XXX - else - what to do here, apart from call the user a moron :-)
    }
    else
        PyErr_Clear();
    Py_XDECREF(ob);

    ob = PyObject_GetAttrString(v, "scode");
    if (ob && ob != Py_None) {
        PyObject *temp = PyNumber_Long(ob);
        if (temp) {
            pExcepInfo->scode = PyLong_AsLong(temp);
            Py_DECREF(temp);
        }
        else
            // XXX - again, should we call the user a moron?
            pExcepInfo->scode = E_FAIL;
    }
    else
        PyErr_Clear();
    Py_XDECREF(ob);

    ob = PyObject_GetAttrString(v, "helpcontext");
    if (ob && ob != Py_None) {
        PyObject *temp = PyNumber_Long(ob);
        if (temp) {
            pExcepInfo->dwHelpContext = (unsigned short)PyLong_AsLong(temp);
            Py_DECREF(temp);
        }
    }
    else
        PyErr_Clear();
    Py_XDECREF(ob);
    return TRUE;
}

BSTR BstrFromOb(PyObject *value)
{
    BSTR result = NULL;
    if (!PyWinObject_AsBstr(value, &result, TRUE, NULL)) {
        PyCom_LoggerNonServerException(NULL, L"Failed to convert exception element to a string");
        PyErr_Clear();
    }
    return result;
}

// Fill an exception info from a specific COM error raised by the
// Python code.  If the Python exception is not a specific COM error
// (ie, pythoncom.com_error, or a COM server exception instance)
// then return FALSE.
BOOL PyCom_ExcepInfoFromPyObject(PyObject *v, EXCEPINFO *pExcepInfo, HRESULT *phresult)
{
    assert(pExcepInfo != NULL);
    if (v == NULL || pExcepInfo == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "invalid arg to PyCom_ExcepInfoFromPyObject");
        return FALSE;
    }

    // New handling for 1.5 exceptions.
    if (!PyErr_GivenExceptionMatches(v, PyWinExc_COMError)) {
        PyErr_Format(PyExc_TypeError, "Must be a COM exception object (not '%s')", v->ob_type->tp_name);
        return FALSE;
    }

    // It is a COM exception, but may be a server or client instance.
    // Explicit check for client.
    // Note that with class based exceptions, a simple pointer check fails.
    // Any class sub-classed from the client is considered a server error,
    // so we need to check the class explicitly.
    if ((PyObject *)v->ob_type == PyWinExc_COMError) {
        // Client side error
        // Clear the state of the excep info.
        // use abstract API to get at details.
        memset(pExcepInfo, 0, sizeof(EXCEPINFO));
        PyObject *ob;
        if (phresult) {
            ob = PyObject_GetAttrString(v, "hresult");
            if (ob) {
                *phresult = PyLong_AsLong(ob);
                Py_DECREF(ob);
            }
        }
        // we ignore `strerror` (item[1] of the args tuple)
        ob = PyObject_GetAttrString(v, "excepinfo");
        if (ob) {
            int code, helpContext, scode;
            PyObject *source, *description, *helpFile;
            if (!PyArg_ParseTuple(ob, "iOOOii:ExceptionInfo", &code, &source, &description, &helpFile, &helpContext,
                                  &scode)) {
                Py_DECREF(ob);
                PyErr_Clear();
                PyErr_SetString(PyExc_TypeError, "The inner excepinfo tuple must be of format 'izzzii'");
                return FALSE;
            }
            pExcepInfo->wCode = code;
            pExcepInfo->wReserved = 0;
            pExcepInfo->bstrSource = BstrFromOb(source);
            pExcepInfo->bstrDescription = BstrFromOb(description);
            pExcepInfo->bstrHelpFile = BstrFromOb(helpFile);
            pExcepInfo->dwHelpContext = helpContext;
            pExcepInfo->pvReserved = 0;
            pExcepInfo->pfnDeferredFillIn = NULL;
            pExcepInfo->scode = scode;
            Py_DECREF(ob);
        }
        return TRUE;
    }
    else {
        // Server side error
        BOOL ok = PyCom_ExcepInfoFromServerExceptionInstance(v, pExcepInfo);
        if (ok && phresult)
            *phresult = pExcepInfo->scode;
        return ok;
    }
}

// Given an EXCEPINFO, register the error information with the
// IErrorInfo interface.
BOOL PyCom_SetCOMErrorFromExcepInfo(const EXCEPINFO *pexcepinfo, REFIID riid)
{
    ICreateErrorInfo *pICEI;
    HRESULT hr = CreateErrorInfo(&pICEI);
    if (SUCCEEDED(hr)) {
        pICEI->SetGUID(riid);
        pICEI->SetHelpContext(pexcepinfo->dwHelpContext);
        if (pexcepinfo->bstrDescription)
            pICEI->SetDescription(pexcepinfo->bstrDescription);
        if (pexcepinfo->bstrHelpFile)
            pICEI->SetHelpFile(pexcepinfo->bstrHelpFile);
        if (pexcepinfo->bstrSource)
            pICEI->SetSource(pexcepinfo->bstrSource);

        IErrorInfo *pIEI;
        Py_BEGIN_ALLOW_THREADS hr = pICEI->QueryInterface(IID_IErrorInfo, (LPVOID *)&pIEI);
        Py_END_ALLOW_THREADS if (SUCCEEDED(hr))
        {
            SetErrorInfo(0, pIEI);
            pIEI->Release();
        }
        pICEI->Release();
    }
    return SUCCEEDED(hr);
}

void PyCom_CleanupExcepInfo(EXCEPINFO *pexcepinfo)
{
    if (pexcepinfo->bstrDescription) {
        SysFreeString(pexcepinfo->bstrDescription);
        pexcepinfo->bstrDescription = NULL;
    }
    if (pexcepinfo->bstrHelpFile) {
        SysFreeString(pexcepinfo->bstrHelpFile);
        pexcepinfo->bstrHelpFile = NULL;
    }
    if (pexcepinfo->bstrSource) {
        SysFreeString(pexcepinfo->bstrSource);
        pexcepinfo->bstrSource = NULL;
    }
}

HRESULT PyCom_CheckIEnumNextResult(HRESULT hr, REFIID riid)
{
    return PyCom_SetCOMErrorFromSimple(
        hr, riid, L"Could not convert the result from Next()/Clone() into the required COM interface");
}

HRESULT PyCom_HandleIEnumNoSequence(REFIID riid)
{
    return PyCom_SetCOMErrorFromSimple(E_FAIL, riid, L"Next() did not return a sequence of objects");
}

HRESULT PyCom_SetCOMErrorFromSimple(HRESULT hr, REFIID riid /* = IID_NULL */, const WCHAR *description /* = NULL*/)
{
    // fast path...
    if (hr == S_OK)
        return S_OK;

    // If you specify a description you should also specify the IID
    assert(riid != IID_NULL || description == NULL);
    BSTR bstrDesc = description ? SysAllocString(description) : NULL;

    EXCEPINFO einfo = {
        0,         // wCode
        0,         // wReserved
        NULL,      // bstrSource
        bstrDesc,  // bstrDescription
        NULL,      // bstrHelpFile
        0,         // dwHelpContext
        NULL,      // pvReserved
        NULL,      // pfnDeferredFillIn
        hr         // scode
    };
    HRESULT ret = PyCom_SetCOMErrorFromExcepInfo(&einfo, riid);
    PyCom_CleanupExcepInfo(&einfo);
    return ret;
}

PYCOM_EXPORT HRESULT PyCom_SetCOMErrorFromPyException(REFIID riid /* = IID_NULL */)
{
    if (!PyErr_Occurred())
        // No error occurred
        return S_OK;

    EXCEPINFO einfo;
    PyCom_ExcepInfoFromPyException(&einfo);

    // force this to a failure just in case we couldn't extract a proper
    // error value
    if (einfo.scode == S_OK)
        einfo.scode = E_FAIL;

    PyCom_SetCOMErrorFromExcepInfo(&einfo, riid);
    PyCom_CleanupExcepInfo(&einfo);
    return einfo.scode;
}

PYCOM_EXPORT HRESULT PyCom_SetAndLogCOMErrorFromPyException(const char *methodName, REFIID riid /* = IID_NULL */)
{
    if (!PyErr_Occurred())
        // No error occurred
        return S_OK;
    PyCom_LoggerNonServerException(NULL, L"Unexpected exception in gateway method '%hs'", methodName);
    return PyCom_SetCOMErrorFromPyException(riid);
}

PYCOM_EXPORT HRESULT PyCom_SetAndLogCOMErrorFromPyExceptionEx(PyObject *provider, const char *methodName,
                                                              REFIID riid /* = IID_NULL */)
{
    if (!PyErr_Occurred())
        // No error occurred
        return S_OK;
    PyCom_LoggerNonServerException(NULL, L"Unexpected exception in gateway method '%hs'", methodName);
    return PyCom_SetCOMErrorFromPyException(riid);
}

void PyCom_StreamMessage(const WCHAR *pszMessageText)
{
    OutputDebugString(pszMessageText);
    // PySys_WriteStderr has an internal 1024 limit due to varargs.
    // we've already resolved them, so we gotta do it the hard way
    // We can't afford to screw with the Python exception state
    PyObject *typ, *val, *tb;
    PyErr_Fetch(&typ, &val, &tb);
    PyObject *pyfile = PySys_GetObject("stderr");
    if (pyfile) {
        PyObject *obUnicode = PyWinObject_FromWCHAR(pszMessageText);
        if (obUnicode) {
            if (PyFile_WriteObject(obUnicode, pyfile, Py_PRINT_RAW) != 0) {
                // eeek - Python error writing this error - write it to stdout.
                fwprintf(stdout, L"%s", pszMessageText);
            }
            Py_DECREF(obUnicode);
        }
    }
    PyErr_Restore(typ, val, tb);
}

BOOL VLogF_Logger(PyObject *logger, const char *log_method, const WCHAR *prefix, const WCHAR *fmt, va_list argptr)
{
    // Protected by Python lock
    static WCHAR buff[8196];
    size_t buf_len = sizeof(buff) / sizeof(buff[0]);
    size_t prefix_len = wcslen(prefix);
    wcsncpy(buff, prefix, buf_len);
    _vsnwprintf(buff + prefix_len, buf_len - prefix_len, fmt, argptr);

    PyObject *exc_typ = NULL, *exc_val = NULL, *exc_tb = NULL;
    PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);

    PyObject *kw = PyDict_New();
    if (kw && exc_typ) {
        PyObject *exc_info = Py_BuildValue("OOO", exc_typ, exc_val ? exc_val : Py_None, exc_tb ? exc_tb : Py_None);
        PyDict_SetItemString(kw, "exc_info", exc_info);
        Py_XDECREF(exc_info);
    }
    PyObject *args = Py_BuildValue("(u)", buff);
    PyObject *method = PyObject_GetAttrString(logger, log_method);
    PyObject *result = NULL;
    if (method && kw && args)
        result = PyObject_Call(method, args, kw);
    Py_XDECREF(method);
    Py_XDECREF(kw);
    Py_XDECREF(args);
    if (!result)
        PyErr_Print();
    BOOL rc = result != NULL;
    Py_XDECREF(result);
    PyErr_Restore(exc_typ, exc_val, exc_tb);
    return rc;
}

void VLogF(const WCHAR *fmt, va_list argptr)
{
    static WCHAR buff[8196];  // protected by Python lock
    _vsnwprintf(buff, 8196, fmt, argptr);
    PyCom_StreamMessage(buff);
}

void PyCom_LogF(const WCHAR *fmt, ...)
{
    va_list marker;

    va_start(marker, fmt);
    VLogF(fmt, marker);
    PyCom_StreamMessage(L"\n");
}

void _LogException(PyObject *exc_typ, PyObject *exc_val, PyObject *exc_tb)
{
    WCHAR *szTraceback = GetPythonTraceback(exc_typ, exc_val, exc_tb);
    PyCom_StreamMessage(szTraceback);
    free(szTraceback);
}

// XXX - _DoLogError() was a really bad name in retrospect, given
// the "logger" module and my dumb choice of _DoLogger() for logger errors :)
// Thankfully both are private.
static void _DoLogError(const WCHAR *prefix, const WCHAR *fmt, va_list argptr)
{
    PyCom_StreamMessage(prefix);
    VLogF(fmt, argptr);
    PyCom_StreamMessage(L"\n");
    // If we have a Python exception, also log that:
    PyObject *exc_typ = NULL, *exc_val = NULL, *exc_tb = NULL;
    PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);
    if (exc_typ) {
        PyErr_NormalizeException(&exc_typ, &exc_val, &exc_tb);
        PyCom_StreamMessage(L"\n");
        _LogException(exc_typ, exc_val, exc_tb);
    }
    PyErr_Restore(exc_typ, exc_val, exc_tb);
}

static void _DoLogger(PyObject *logProvider, char *log_method, const WCHAR *fmt, va_list argptr)
{
    CEnterLeavePython _celp;
    PyObject *exc_typ = NULL, *exc_val = NULL, *exc_tb = NULL;
    PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);
    PyObject *logger = NULL;
    WCHAR prefix[128];
    _snwprintf(prefix, 128, L"pythoncom %hs: ", log_method);

    if (logProvider) {
        logger = PyObject_CallMethod(logProvider, "_GetLogger_", NULL);
        if (!logger) {
            PyErr_Clear();
        }
    }
    if (logger == NULL) {
        PyObject *mod = PyImport_ImportModule("win32com");
        if (mod) {
            logger = PyObject_GetAttrString(mod, "logger");
            Py_DECREF(mod);
        }
    }
    // Returning a logger of None means "no logger"
    if (logger == Py_None) {
        Py_DECREF(logger);
        logger = NULL;
    }
    PyErr_Restore(exc_typ, exc_val, exc_tb);
    if (!logger || !VLogF_Logger(logger, log_method, prefix, fmt, argptr))
        // No logger, or logger error - normal stdout stream.
        _DoLogError(prefix, fmt, argptr);
    Py_XDECREF(logger);
}

// Is the current exception a "server" exception? - ie, one explicitly
// thrown by Python code to indicate an error.  This is defined as
// any exception whose type is a subclass of com_error (a plain
// com_error probably means an unhandled exception from someone
// calling an interface)
BOOL IsServerErrorCurrent()
{
    BOOL rc = FALSE;
    PyObject *exc_typ = NULL, *exc_val = NULL, *exc_tb = NULL;
    PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);
    assert(exc_typ);  // we should only be called with an exception current.
    if (exc_typ) {
        PyErr_NormalizeException(&exc_typ, &exc_val, &exc_tb);
        // so it must "match" a com_error, but not be *exactly* a COM error.
        rc = PyErr_GivenExceptionMatches(exc_val, PyWinExc_COMError) && exc_typ != PyWinExc_COMError;
    }
    PyErr_Restore(exc_typ, exc_val, exc_tb);
    return rc;
}

PYCOM_EXPORT void PyCom_LoggerException(PyObject *logProvider, const WCHAR *fmt, ...)
{
    va_list marker;
    va_start(marker, fmt);
    _DoLogger(logProvider, "error", fmt, marker);
}

PYCOM_EXPORT void PyCom_LoggerWarning(PyObject *logProvider, const WCHAR *fmt, ...)
{
    va_list marker;
    va_start(marker, fmt);
    _DoLogger(logProvider, "warning", fmt, marker);
}

PYCOM_EXPORT
void PyCom_LoggerNonServerException(PyObject *logProvider, const WCHAR *fmt, ...)
{
    if (IsServerErrorCurrent())
        return;
    va_list marker;
    va_start(marker, fmt);
    _DoLogger(logProvider, "error", fmt, marker);
}

////////////////////////////////////////////////////////////////////////
//
// Client Side Errors - translate a COM failure to a Python exception
//
////////////////////////////////////////////////////////////////////////
PyObject *PyCom_BuildPyException(HRESULT errorhr, IUnknown *pUnk /* = NULL */, REFIID iid /* = IID_NULL */)
{
    PyObject *obEI = NULL;
    TCHAR scodeStringBuf[512];
    GetScodeString(errorhr, scodeStringBuf, sizeof(scodeStringBuf) / sizeof(scodeStringBuf[0]));

    if (pUnk != NULL) {
        assert(iid != IID_NULL);  // If you pass an IUnknown, you should pass the specific IID.
        // See if it supports error info.
        ISupportErrorInfo *pSEI;
        HRESULT hr;
        Py_BEGIN_ALLOW_THREADS hr = pUnk->QueryInterface(IID_ISupportErrorInfo, (void **)&pSEI);
        if (SUCCEEDED(hr)) {
            hr = pSEI->InterfaceSupportsErrorInfo(iid);
            pSEI->Release();  // Finished with this object
        }
        Py_END_ALLOW_THREADS if (SUCCEEDED(hr))
        {
            IErrorInfo *pEI;
            Py_BEGIN_ALLOW_THREADS hr = GetErrorInfo(0, &pEI);
            Py_END_ALLOW_THREADS if (hr == S_OK)
            {
                obEI = PyCom_PyObjectFromIErrorInfo(pEI, errorhr);
                PYCOM_RELEASE(pEI);
            }
        }
    }
    if (obEI == NULL) {
        obEI = Py_None;
        Py_INCREF(Py_None);
    }
    PyObject *evalue = Py_BuildValue("iNOO", errorhr, PyWinObject_FromTCHAR(scodeStringBuf), obEI, Py_None);
    Py_DECREF(obEI);

    PyErr_SetObject(PyWinExc_COMError, evalue);
    Py_XDECREF(evalue);
    return NULL;
}

// Uses the HRESULT and an EXCEPINFO structure to create and
// set a pythoncom.com_error.
// Used rarely - currently by IDispatch and IActiveScriptParse* interfaces.
PyObject *PyCom_BuildPyExceptionFromEXCEPINFO(HRESULT hr, EXCEPINFO *pexcepInfo /* = NULL */, UINT nArgErr /* = -1 */)
{
    TCHAR buf[512];
    GetScodeString(hr, buf, sizeof(buf) / sizeof(TCHAR));
    PyObject *obScodeString = PyWinObject_FromTCHAR(buf);
    PyObject *evalue;
    PyObject *obArg;

    if (nArgErr != -1) {
        obArg = PyLong_FromLong(nArgErr);
    }
    else {
        obArg = Py_None;
        Py_INCREF(obArg);
    }
    if (pexcepInfo == NULL) {
        evalue = Py_BuildValue("iOzO", hr, obScodeString, NULL, obArg);
    }
    else {
        PyObject *obExcepInfo = PyCom_PyObjectFromExcepInfo(pexcepInfo);
        if (obExcepInfo) {
            evalue = Py_BuildValue("iOOO", hr, obScodeString, obExcepInfo, obArg);
            Py_DECREF(obExcepInfo);
        }
        else
            evalue = NULL;

        /* done with the exception, free it */
        PyCom_CleanupExcepInfo(pexcepInfo);
    }
    Py_DECREF(obArg);
    PyErr_SetObject(PyWinExc_COMError, evalue);
    Py_XDECREF(evalue);
    Py_XDECREF(obScodeString);
    return NULL;
}

PyObject *PyCom_BuildInternalPyException(char *msg)
{
    PyErr_SetString(PyCom_InternalError, msg);
    return NULL;
}

PyObject *PyCom_PyObjectFromExcepInfo(const EXCEPINFO *pexcepInfo)
{
    EXCEPINFO filledIn;

    // Do a deferred fill-in if necessary
    if (pexcepInfo->pfnDeferredFillIn) {
        filledIn = *pexcepInfo;
        (*pexcepInfo->pfnDeferredFillIn)(&filledIn);
        pexcepInfo = &filledIn;
    }

    // ### should these by PyUnicode values?  Still strings for compatibility.
    PyObject *obSource = PyWinObject_FromBstr(pexcepInfo->bstrSource);
    PyObject *obDescription = PyWinObject_FromBstr(pexcepInfo->bstrDescription);
    PyObject *obHelpFile = PyWinObject_FromBstr(pexcepInfo->bstrHelpFile);
    PyObject *rc = Py_BuildValue("iOOOii", (int)pexcepInfo->wCode, obSource, obDescription, obHelpFile,
                                 (int)pexcepInfo->dwHelpContext, (int)pexcepInfo->scode);
    Py_XDECREF(obSource);
    Py_XDECREF(obDescription);
    Py_XDECREF(obHelpFile);
    return rc;
}

// NOTE - This MUST return the same object format as the above function
static PyObject *PyCom_PyObjectFromIErrorInfo(IErrorInfo *pEI, HRESULT errorhr)
{
    BSTR desc;
    BSTR source;
    BSTR helpfile;
    PyObject *obDesc;
    PyObject *obSource;
    PyObject *obHelpFile;

    HRESULT hr;

    Py_BEGIN_ALLOW_THREADS hr = pEI->GetDescription(&desc);
    Py_END_ALLOW_THREADS if (hr != S_OK)
    {
        obDesc = Py_None;
        Py_INCREF(obDesc);
    }
    else
    {
        obDesc = MakeBstrToObj(desc);
        SysFreeString(desc);
    }

    Py_BEGIN_ALLOW_THREADS hr = pEI->GetSource(&source);
    Py_END_ALLOW_THREADS if (hr != S_OK)
    {
        obSource = Py_None;
        Py_INCREF(obSource);
    }
    else
    {
        obSource = MakeBstrToObj(source);
        SysFreeString(source);
    }
    Py_BEGIN_ALLOW_THREADS hr = pEI->GetHelpFile(&helpfile);
    Py_END_ALLOW_THREADS if (hr != S_OK)
    {
        obHelpFile = Py_None;
        Py_INCREF(obHelpFile);
    }
    else
    {
        obHelpFile = MakeBstrToObj(helpfile);
        SysFreeString(helpfile);
    }
    DWORD helpContext = 0;
    pEI->GetHelpContext(&helpContext);
    PyObject *ret = Py_BuildValue("iOOOii",
                                  0,  // wCode remains zero, as scode holds our data.
                                  // ### should these by PyUnicode values?
                                  obSource, obDesc, obHelpFile, (int)helpContext, errorhr);
    Py_XDECREF(obSource);
    Py_XDECREF(obDesc);
    Py_XDECREF(obHelpFile);
    return ret;
}

////////////////////////////////////////////////////////////////////////
//
// Error string helpers - get SCODE, FACILITY etc strings
//
////////////////////////////////////////////////////////////////////////
#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

void GetScodeString(HRESULT hr, LPTSTR buf, int bufSize)
{
    struct HRESULT_ENTRY {
        HRESULT hr;
        LPCTSTR lpszName;
    };
#define MAKE_HRESULT_ENTRY(hr) \
    {                          \
        hr, _T(#hr)            \
    }
    static const HRESULT_ENTRY hrNameTable[] = {
        MAKE_HRESULT_ENTRY(S_OK),
        MAKE_HRESULT_ENTRY(S_FALSE),

        MAKE_HRESULT_ENTRY(CACHE_S_FORMATETC_NOTSUPPORTED),
        MAKE_HRESULT_ENTRY(CACHE_S_SAMECACHE),
        MAKE_HRESULT_ENTRY(CACHE_S_SOMECACHES_NOTUPDATED),
        MAKE_HRESULT_ENTRY(CONVERT10_S_NO_PRESENTATION),
        MAKE_HRESULT_ENTRY(DATA_S_SAMEFORMATETC),
        MAKE_HRESULT_ENTRY(DRAGDROP_S_CANCEL),
        MAKE_HRESULT_ENTRY(DRAGDROP_S_DROP),
        MAKE_HRESULT_ENTRY(DRAGDROP_S_USEDEFAULTCURSORS),
        MAKE_HRESULT_ENTRY(INPLACE_S_TRUNCATED),
        MAKE_HRESULT_ENTRY(MK_S_HIM),
        MAKE_HRESULT_ENTRY(MK_S_ME),
        MAKE_HRESULT_ENTRY(MK_S_MONIKERALREADYREGISTERED),
        MAKE_HRESULT_ENTRY(MK_S_REDUCED_TO_SELF),
        MAKE_HRESULT_ENTRY(MK_S_US),
        MAKE_HRESULT_ENTRY(OLE_S_MAC_CLIPFORMAT),
        MAKE_HRESULT_ENTRY(OLE_S_STATIC),
        MAKE_HRESULT_ENTRY(OLE_S_USEREG),
        MAKE_HRESULT_ENTRY(OLEOBJ_S_CANNOT_DOVERB_NOW),
        MAKE_HRESULT_ENTRY(OLEOBJ_S_INVALIDHWND),
        MAKE_HRESULT_ENTRY(OLEOBJ_S_INVALIDVERB),
        MAKE_HRESULT_ENTRY(OLEOBJ_S_LAST),
        MAKE_HRESULT_ENTRY(STG_S_CONVERTED),
        MAKE_HRESULT_ENTRY(VIEW_S_ALREADY_FROZEN),

        MAKE_HRESULT_ENTRY(E_UNEXPECTED),
        MAKE_HRESULT_ENTRY(E_NOTIMPL),
        MAKE_HRESULT_ENTRY(E_OUTOFMEMORY),
        MAKE_HRESULT_ENTRY(E_INVALIDARG),
        MAKE_HRESULT_ENTRY(E_NOINTERFACE),
        MAKE_HRESULT_ENTRY(E_POINTER),
        MAKE_HRESULT_ENTRY(E_HANDLE),
        MAKE_HRESULT_ENTRY(E_ABORT),
        MAKE_HRESULT_ENTRY(E_FAIL),
        MAKE_HRESULT_ENTRY(E_ACCESSDENIED),

        MAKE_HRESULT_ENTRY(CACHE_E_NOCACHE_UPDATED),
        MAKE_HRESULT_ENTRY(CLASS_E_CLASSNOTAVAILABLE),
        MAKE_HRESULT_ENTRY(CLASS_E_NOAGGREGATION),
        MAKE_HRESULT_ENTRY(CLIPBRD_E_BAD_DATA),
        MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_CLOSE),
        MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_EMPTY),
        MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_OPEN),
        MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_SET),
        MAKE_HRESULT_ENTRY(CO_E_ALREADYINITIALIZED),
        MAKE_HRESULT_ENTRY(CO_E_APPDIDNTREG),
        MAKE_HRESULT_ENTRY(CO_E_APPNOTFOUND),
        MAKE_HRESULT_ENTRY(CO_E_APPSINGLEUSE),
        MAKE_HRESULT_ENTRY(CO_E_BAD_PATH),
        MAKE_HRESULT_ENTRY(CO_E_CANTDETERMINECLASS),
        MAKE_HRESULT_ENTRY(CO_E_CLASS_CREATE_FAILED),
        MAKE_HRESULT_ENTRY(CO_E_CLASSSTRING),
        MAKE_HRESULT_ENTRY(CO_E_DLLNOTFOUND),
        MAKE_HRESULT_ENTRY(CO_E_ERRORINAPP),
        MAKE_HRESULT_ENTRY(CO_E_ERRORINDLL),
        MAKE_HRESULT_ENTRY(CO_E_IIDSTRING),
        MAKE_HRESULT_ENTRY(CO_E_NOTINITIALIZED),
        MAKE_HRESULT_ENTRY(CO_E_OBJISREG),
        MAKE_HRESULT_ENTRY(CO_E_OBJNOTCONNECTED),
        MAKE_HRESULT_ENTRY(CO_E_OBJNOTREG),
        MAKE_HRESULT_ENTRY(CO_E_OBJSRV_RPC_FAILURE),
        MAKE_HRESULT_ENTRY(CO_E_SCM_ERROR),
        MAKE_HRESULT_ENTRY(CO_E_SCM_RPC_FAILURE),
        MAKE_HRESULT_ENTRY(CO_E_SERVER_EXEC_FAILURE),
        MAKE_HRESULT_ENTRY(CO_E_SERVER_STOPPING),
        MAKE_HRESULT_ENTRY(CO_E_WRONGOSFORAPP),
        MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB),
        MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_FMT),
        MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_GET),
        MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_PUT),
        MAKE_HRESULT_ENTRY(CONVERT10_E_STG_DIB_TO_BITMAP),
        MAKE_HRESULT_ENTRY(CONVERT10_E_STG_FMT),
        MAKE_HRESULT_ENTRY(CONVERT10_E_STG_NO_STD_STREAM),
        MAKE_HRESULT_ENTRY(DISP_E_ARRAYISLOCKED),
        MAKE_HRESULT_ENTRY(DISP_E_BADCALLEE),
        MAKE_HRESULT_ENTRY(DISP_E_BADINDEX),
        MAKE_HRESULT_ENTRY(DISP_E_BADPARAMCOUNT),
        MAKE_HRESULT_ENTRY(DISP_E_BADVARTYPE),
        MAKE_HRESULT_ENTRY(DISP_E_EXCEPTION),
        MAKE_HRESULT_ENTRY(DISP_E_MEMBERNOTFOUND),
        MAKE_HRESULT_ENTRY(DISP_E_NONAMEDARGS),
        MAKE_HRESULT_ENTRY(DISP_E_NOTACOLLECTION),
        MAKE_HRESULT_ENTRY(DISP_E_OVERFLOW),
        MAKE_HRESULT_ENTRY(DISP_E_PARAMNOTFOUND),
        MAKE_HRESULT_ENTRY(DISP_E_PARAMNOTOPTIONAL),
        MAKE_HRESULT_ENTRY(DISP_E_TYPEMISMATCH),
        MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNINTERFACE),
        MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNLCID),
        MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNNAME),
        MAKE_HRESULT_ENTRY(DRAGDROP_E_ALREADYREGISTERED),
        MAKE_HRESULT_ENTRY(DRAGDROP_E_INVALIDHWND),
        MAKE_HRESULT_ENTRY(DRAGDROP_E_NOTREGISTERED),
        MAKE_HRESULT_ENTRY(DV_E_CLIPFORMAT),
        MAKE_HRESULT_ENTRY(DV_E_DVASPECT),
        MAKE_HRESULT_ENTRY(DV_E_DVTARGETDEVICE),
        MAKE_HRESULT_ENTRY(DV_E_DVTARGETDEVICE_SIZE),
        MAKE_HRESULT_ENTRY(DV_E_FORMATETC),
        MAKE_HRESULT_ENTRY(DV_E_LINDEX),
        MAKE_HRESULT_ENTRY(DV_E_NOIVIEWOBJECT),
        MAKE_HRESULT_ENTRY(DV_E_STATDATA),
        MAKE_HRESULT_ENTRY(DV_E_STGMEDIUM),
        MAKE_HRESULT_ENTRY(DV_E_TYMED),
        MAKE_HRESULT_ENTRY(INPLACE_E_NOTOOLSPACE),
        MAKE_HRESULT_ENTRY(INPLACE_E_NOTUNDOABLE),
        MAKE_HRESULT_ENTRY(MEM_E_INVALID_LINK),
        MAKE_HRESULT_ENTRY(MEM_E_INVALID_ROOT),
        MAKE_HRESULT_ENTRY(MEM_E_INVALID_SIZE),
        MAKE_HRESULT_ENTRY(MK_E_CANTOPENFILE),
        MAKE_HRESULT_ENTRY(MK_E_CONNECTMANUALLY),
        MAKE_HRESULT_ENTRY(MK_E_ENUMERATION_FAILED),
        MAKE_HRESULT_ENTRY(MK_E_EXCEEDEDDEADLINE),
        MAKE_HRESULT_ENTRY(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED),
        MAKE_HRESULT_ENTRY(MK_E_INVALIDEXTENSION),
        MAKE_HRESULT_ENTRY(MK_E_MUSTBOTHERUSER),
        MAKE_HRESULT_ENTRY(MK_E_NEEDGENERIC),
        MAKE_HRESULT_ENTRY(MK_E_NO_NORMALIZED),
        MAKE_HRESULT_ENTRY(MK_E_NOINVERSE),
        MAKE_HRESULT_ENTRY(MK_E_NOOBJECT),
        MAKE_HRESULT_ENTRY(MK_E_NOPREFIX),
        MAKE_HRESULT_ENTRY(MK_E_NOSTORAGE),
        MAKE_HRESULT_ENTRY(MK_E_NOTBINDABLE),
        MAKE_HRESULT_ENTRY(MK_E_NOTBOUND),
        MAKE_HRESULT_ENTRY(MK_E_SYNTAX),
        MAKE_HRESULT_ENTRY(MK_E_UNAVAILABLE),
        MAKE_HRESULT_ENTRY(OLE_E_ADVF),
        MAKE_HRESULT_ENTRY(OLE_E_ADVISENOTSUPPORTED),
        MAKE_HRESULT_ENTRY(OLE_E_BLANK),
        MAKE_HRESULT_ENTRY(OLE_E_CANT_BINDTOSOURCE),
        MAKE_HRESULT_ENTRY(OLE_E_CANT_GETMONIKER),
        MAKE_HRESULT_ENTRY(OLE_E_CANTCONVERT),
        MAKE_HRESULT_ENTRY(OLE_E_CLASSDIFF),
        MAKE_HRESULT_ENTRY(OLE_E_ENUM_NOMORE),
        MAKE_HRESULT_ENTRY(OLE_E_INVALIDHWND),
        MAKE_HRESULT_ENTRY(OLE_E_INVALIDRECT),
        MAKE_HRESULT_ENTRY(OLE_E_NOCACHE),
        MAKE_HRESULT_ENTRY(OLE_E_NOCONNECTION),
        MAKE_HRESULT_ENTRY(OLE_E_NOSTORAGE),
        MAKE_HRESULT_ENTRY(OLE_E_NOT_INPLACEACTIVE),
        MAKE_HRESULT_ENTRY(OLE_E_NOTRUNNING),
        MAKE_HRESULT_ENTRY(OLE_E_OLEVERB),
        MAKE_HRESULT_ENTRY(OLE_E_PROMPTSAVECANCELLED),
        MAKE_HRESULT_ENTRY(OLE_E_STATIC),
        MAKE_HRESULT_ENTRY(OLE_E_WRONGCOMPOBJ),
        MAKE_HRESULT_ENTRY(OLEOBJ_E_INVALIDVERB),
        MAKE_HRESULT_ENTRY(OLEOBJ_E_NOVERBS),
        MAKE_HRESULT_ENTRY(REGDB_E_CLASSNOTREG),
        MAKE_HRESULT_ENTRY(REGDB_E_IIDNOTREG),
        MAKE_HRESULT_ENTRY(REGDB_E_INVALIDVALUE),
        MAKE_HRESULT_ENTRY(REGDB_E_KEYMISSING),
        MAKE_HRESULT_ENTRY(REGDB_E_READREGDB),
        MAKE_HRESULT_ENTRY(REGDB_E_WRITEREGDB),
        MAKE_HRESULT_ENTRY(RPC_E_ATTEMPTED_MULTITHREAD),
        MAKE_HRESULT_ENTRY(RPC_E_CALL_CANCELED),
        MAKE_HRESULT_ENTRY(RPC_E_CALL_REJECTED),
        MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_AGAIN),
        MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_INASYNCCALL),
        MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_INEXTERNALCALL),
        MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_ININPUTSYNCCALL),
        MAKE_HRESULT_ENTRY(RPC_E_CANTPOST_INSENDCALL),
        MAKE_HRESULT_ENTRY(RPC_E_CANTTRANSMIT_CALL),
        MAKE_HRESULT_ENTRY(RPC_E_CHANGED_MODE),
        MAKE_HRESULT_ENTRY(RPC_E_CLIENT_CANTMARSHAL_DATA),
        MAKE_HRESULT_ENTRY(RPC_E_CLIENT_CANTUNMARSHAL_DATA),
        MAKE_HRESULT_ENTRY(RPC_E_CLIENT_DIED),
        MAKE_HRESULT_ENTRY(RPC_E_CONNECTION_TERMINATED),
        MAKE_HRESULT_ENTRY(RPC_E_DISCONNECTED),
        MAKE_HRESULT_ENTRY(RPC_E_FAULT),
        MAKE_HRESULT_ENTRY(RPC_E_INVALID_CALLDATA),
        MAKE_HRESULT_ENTRY(RPC_E_INVALID_DATA),
        MAKE_HRESULT_ENTRY(RPC_E_INVALID_DATAPACKET),
        MAKE_HRESULT_ENTRY(RPC_E_INVALID_PARAMETER),
        MAKE_HRESULT_ENTRY(RPC_E_INVALIDMETHOD),
        MAKE_HRESULT_ENTRY(RPC_E_NOT_REGISTERED),
        MAKE_HRESULT_ENTRY(RPC_E_OUT_OF_RESOURCES),
        MAKE_HRESULT_ENTRY(RPC_E_RETRY),
        MAKE_HRESULT_ENTRY(RPC_E_SERVER_CANTMARSHAL_DATA),
        MAKE_HRESULT_ENTRY(RPC_E_SERVER_CANTUNMARSHAL_DATA),
        MAKE_HRESULT_ENTRY(RPC_E_SERVER_DIED),
        MAKE_HRESULT_ENTRY(RPC_E_SERVER_DIED_DNE),
        MAKE_HRESULT_ENTRY(RPC_E_SERVERCALL_REJECTED),
        MAKE_HRESULT_ENTRY(RPC_E_SERVERCALL_RETRYLATER),
        MAKE_HRESULT_ENTRY(RPC_E_SERVERFAULT),
        MAKE_HRESULT_ENTRY(RPC_E_SYS_CALL_FAILED),
        MAKE_HRESULT_ENTRY(RPC_E_THREAD_NOT_INIT),
        MAKE_HRESULT_ENTRY(RPC_E_UNEXPECTED),
        MAKE_HRESULT_ENTRY(RPC_E_WRONG_THREAD),
        MAKE_HRESULT_ENTRY(STG_E_ABNORMALAPIEXIT),
        MAKE_HRESULT_ENTRY(STG_E_ACCESSDENIED),
        MAKE_HRESULT_ENTRY(STG_E_CANTSAVE),
        MAKE_HRESULT_ENTRY(STG_E_DISKISWRITEPROTECTED),
        MAKE_HRESULT_ENTRY(STG_E_EXTANTMARSHALLINGS),
        MAKE_HRESULT_ENTRY(STG_E_FILEALREADYEXISTS),
        MAKE_HRESULT_ENTRY(STG_E_FILENOTFOUND),
        MAKE_HRESULT_ENTRY(STG_E_INSUFFICIENTMEMORY),
        MAKE_HRESULT_ENTRY(STG_E_INUSE),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDFLAG),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDFUNCTION),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDHANDLE),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDHEADER),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDNAME),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDPARAMETER),
        MAKE_HRESULT_ENTRY(STG_E_INVALIDPOINTER),
        MAKE_HRESULT_ENTRY(STG_E_LOCKVIOLATION),
        MAKE_HRESULT_ENTRY(STG_E_MEDIUMFULL),
        MAKE_HRESULT_ENTRY(STG_E_NOMOREFILES),
        MAKE_HRESULT_ENTRY(STG_E_NOTCURRENT),
        MAKE_HRESULT_ENTRY(STG_E_NOTFILEBASEDSTORAGE),
        MAKE_HRESULT_ENTRY(STG_E_OLDDLL),
        MAKE_HRESULT_ENTRY(STG_E_OLDFORMAT),
        MAKE_HRESULT_ENTRY(STG_E_PATHNOTFOUND),
        MAKE_HRESULT_ENTRY(STG_E_READFAULT),
        MAKE_HRESULT_ENTRY(STG_E_REVERTED),
        MAKE_HRESULT_ENTRY(STG_E_SEEKERROR),
        MAKE_HRESULT_ENTRY(STG_E_SHAREREQUIRED),
        MAKE_HRESULT_ENTRY(STG_E_SHAREVIOLATION),
        MAKE_HRESULT_ENTRY(STG_E_TOOMANYOPENFILES),
        MAKE_HRESULT_ENTRY(STG_E_UNIMPLEMENTEDFUNCTION),
        MAKE_HRESULT_ENTRY(STG_E_UNKNOWN),
        MAKE_HRESULT_ENTRY(STG_E_WRITEFAULT),
        MAKE_HRESULT_ENTRY(TYPE_E_AMBIGUOUSNAME),
        MAKE_HRESULT_ENTRY(TYPE_E_BADMODULEKIND),
        MAKE_HRESULT_ENTRY(TYPE_E_BUFFERTOOSMALL),
        MAKE_HRESULT_ENTRY(TYPE_E_CANTCREATETMPFILE),
        MAKE_HRESULT_ENTRY(TYPE_E_CANTLOADLIBRARY),
        MAKE_HRESULT_ENTRY(TYPE_E_CIRCULARTYPE),
        MAKE_HRESULT_ENTRY(TYPE_E_DLLFUNCTIONNOTFOUND),
        MAKE_HRESULT_ENTRY(TYPE_E_DUPLICATEID),
        MAKE_HRESULT_ENTRY(TYPE_E_ELEMENTNOTFOUND),
        MAKE_HRESULT_ENTRY(TYPE_E_INCONSISTENTPROPFUNCS),
        MAKE_HRESULT_ENTRY(TYPE_E_INVALIDSTATE),
        MAKE_HRESULT_ENTRY(TYPE_E_INVDATAREAD),
        MAKE_HRESULT_ENTRY(TYPE_E_IOERROR),
        MAKE_HRESULT_ENTRY(TYPE_E_LIBNOTREGISTERED),
        MAKE_HRESULT_ENTRY(TYPE_E_NAMECONFLICT),
        MAKE_HRESULT_ENTRY(TYPE_E_OUTOFBOUNDS),
        MAKE_HRESULT_ENTRY(TYPE_E_QUALIFIEDNAMEDISALLOWED),
        MAKE_HRESULT_ENTRY(TYPE_E_REGISTRYACCESS),
        MAKE_HRESULT_ENTRY(TYPE_E_SIZETOOBIG),
        MAKE_HRESULT_ENTRY(TYPE_E_TYPEMISMATCH),
        MAKE_HRESULT_ENTRY(TYPE_E_UNDEFINEDTYPE),
        MAKE_HRESULT_ENTRY(TYPE_E_UNKNOWNLCID),
        MAKE_HRESULT_ENTRY(TYPE_E_UNSUPFORMAT),
        MAKE_HRESULT_ENTRY(TYPE_E_WRONGTYPEKIND),
        MAKE_HRESULT_ENTRY(VIEW_E_DRAW),

        MAKE_HRESULT_ENTRY(CONNECT_E_NOCONNECTION),
        MAKE_HRESULT_ENTRY(CONNECT_E_ADVISELIMIT),
        MAKE_HRESULT_ENTRY(CONNECT_E_CANNOTCONNECT),
        MAKE_HRESULT_ENTRY(CONNECT_E_OVERRIDDEN),

        MAKE_HRESULT_ENTRY(CLASS_E_NOTLICENSED),
        MAKE_HRESULT_ENTRY(CLASS_E_NOAGGREGATION),
        MAKE_HRESULT_ENTRY(CLASS_E_CLASSNOTAVAILABLE),

        MAKE_HRESULT_ENTRY(CTL_E_ILLEGALFUNCTIONCALL),
        MAKE_HRESULT_ENTRY(CTL_E_OVERFLOW),
        MAKE_HRESULT_ENTRY(CTL_E_OUTOFMEMORY),
        MAKE_HRESULT_ENTRY(CTL_E_DIVISIONBYZERO),
        MAKE_HRESULT_ENTRY(CTL_E_OUTOFSTRINGSPACE),
        MAKE_HRESULT_ENTRY(CTL_E_OUTOFSTACKSPACE),
        MAKE_HRESULT_ENTRY(CTL_E_BADFILENAMEORNUMBER),
        MAKE_HRESULT_ENTRY(CTL_E_FILENOTFOUND),
        MAKE_HRESULT_ENTRY(CTL_E_BADFILEMODE),
        MAKE_HRESULT_ENTRY(CTL_E_FILEALREADYOPEN),
        MAKE_HRESULT_ENTRY(CTL_E_DEVICEIOERROR),
        MAKE_HRESULT_ENTRY(CTL_E_FILEALREADYEXISTS),
        MAKE_HRESULT_ENTRY(CTL_E_BADRECORDLENGTH),
        MAKE_HRESULT_ENTRY(CTL_E_DISKFULL),
        MAKE_HRESULT_ENTRY(CTL_E_BADRECORDNUMBER),
        MAKE_HRESULT_ENTRY(CTL_E_BADFILENAME),
        MAKE_HRESULT_ENTRY(CTL_E_TOOMANYFILES),
        MAKE_HRESULT_ENTRY(CTL_E_DEVICEUNAVAILABLE),
        MAKE_HRESULT_ENTRY(CTL_E_PERMISSIONDENIED),
        MAKE_HRESULT_ENTRY(CTL_E_DISKNOTREADY),
        MAKE_HRESULT_ENTRY(CTL_E_PATHFILEACCESSERROR),
        MAKE_HRESULT_ENTRY(CTL_E_PATHNOTFOUND),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDPATTERNSTRING),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDUSEOFNULL),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDFILEFORMAT),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDPROPERTYVALUE),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDPROPERTYARRAYINDEX),
        MAKE_HRESULT_ENTRY(CTL_E_SETNOTSUPPORTEDATRUNTIME),
        MAKE_HRESULT_ENTRY(CTL_E_SETNOTSUPPORTED),
        MAKE_HRESULT_ENTRY(CTL_E_NEEDPROPERTYARRAYINDEX),
        MAKE_HRESULT_ENTRY(CTL_E_SETNOTPERMITTED),
        MAKE_HRESULT_ENTRY(CTL_E_GETNOTSUPPORTEDATRUNTIME),
        MAKE_HRESULT_ENTRY(CTL_E_GETNOTSUPPORTED),
        MAKE_HRESULT_ENTRY(CTL_E_PROPERTYNOTFOUND),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDCLIPBOARDFORMAT),
        MAKE_HRESULT_ENTRY(CTL_E_INVALIDPICTURE),
        MAKE_HRESULT_ENTRY(CTL_E_PRINTERERROR),
        MAKE_HRESULT_ENTRY(CTL_E_CANTSAVEFILETOTEMP),
        MAKE_HRESULT_ENTRY(CTL_E_SEARCHTEXTNOTFOUND),
        MAKE_HRESULT_ENTRY(CTL_E_REPLACEMENTSTOOLONG),
    };
#undef MAKE_HRESULT_ENTRY

    // first ask the OS to give it to us..
    // ### should we get the Unicode version instead?
    int numCopied = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, buf, bufSize, NULL);
    if (numCopied > 0) {
        if (numCopied < bufSize) {
            // trim trailing crap
            if (numCopied > 2 && (buf[numCopied - 2] == '\n' || buf[numCopied - 2] == '\r'))
                buf[numCopied - 2] = '\0';
        }
        return;
    }
    // Next see if this particular error code is registered as being supplied
    // by a specific DLL.
    HINSTANCE hi = PyWin_GetErrorMessageModule(hr);
    if (hi) {
        numCopied = ::FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, hi, hr, 0, buf, bufSize, NULL);
        if (numCopied > 0) {
            if (numCopied < bufSize) {
                // trim trailing crap
                if (numCopied > 2 && (buf[numCopied - 2] == '\n' || buf[numCopied - 2] == '\r'))
                    buf[numCopied - 2] = '\0';
            }
            return;
        }
    }

    // else look for it in the table
    for (int i = 0; i < _countof(hrNameTable); i++) {
        if (hr == hrNameTable[i].hr) {
            _tcsncpy(buf, hrNameTable[i].lpszName, bufSize);
            return;
        }
    }
    // not found - make one up
    wsprintf(buf, _T("OLE error 0x%08x"), hr);
}

LPCTSTR GetScodeRangeString(HRESULT hr)
{
    struct RANGE_ENTRY {
        HRESULT hrFirst;
        HRESULT hrLast;
        LPCTSTR lpszName;
    };
#define MAKE_RANGE_ENTRY(hrRange)                                                              \
    {                                                                                          \
        hrRange##_FIRST, hrRange##_LAST, _T(#hrRange) _T("_FIRST...") _T(#hrRange) _T("_LAST") \
    }

    static const RANGE_ENTRY hrRangeTable[] = {
        MAKE_RANGE_ENTRY(CACHE_E),        MAKE_RANGE_ENTRY(CACHE_S),      MAKE_RANGE_ENTRY(CLASSFACTORY_E),
        MAKE_RANGE_ENTRY(CLASSFACTORY_S), MAKE_RANGE_ENTRY(CLIENTSITE_E), MAKE_RANGE_ENTRY(CLIENTSITE_S),
        MAKE_RANGE_ENTRY(CLIPBRD_E),      MAKE_RANGE_ENTRY(CLIPBRD_S),    MAKE_RANGE_ENTRY(CONVERT10_E),
        MAKE_RANGE_ENTRY(CONVERT10_S),    MAKE_RANGE_ENTRY(CO_E),         MAKE_RANGE_ENTRY(CO_S),
        MAKE_RANGE_ENTRY(DATA_E),         MAKE_RANGE_ENTRY(DATA_S),       MAKE_RANGE_ENTRY(DRAGDROP_E),
        MAKE_RANGE_ENTRY(DRAGDROP_S),     MAKE_RANGE_ENTRY(ENUM_E),       MAKE_RANGE_ENTRY(ENUM_S),
        MAKE_RANGE_ENTRY(INPLACE_E),      MAKE_RANGE_ENTRY(INPLACE_S),    MAKE_RANGE_ENTRY(MARSHAL_E),
        MAKE_RANGE_ENTRY(MARSHAL_S),      MAKE_RANGE_ENTRY(MK_E),         MAKE_RANGE_ENTRY(MK_S),
        MAKE_RANGE_ENTRY(OLEOBJ_E),       MAKE_RANGE_ENTRY(OLEOBJ_S),     MAKE_RANGE_ENTRY(OLE_E),
        MAKE_RANGE_ENTRY(OLE_S),          MAKE_RANGE_ENTRY(REGDB_E),      MAKE_RANGE_ENTRY(REGDB_S),
        MAKE_RANGE_ENTRY(VIEW_E),         MAKE_RANGE_ENTRY(VIEW_S),       MAKE_RANGE_ENTRY(CONNECT_E),
        MAKE_RANGE_ENTRY(CONNECT_S),

    };
#undef MAKE_RANGE_ENTRY

    // look for it in the table
    for (int i = 0; i < _countof(hrRangeTable); i++) {
        if (hr >= hrRangeTable[i].hrFirst && hr <= hrRangeTable[i].hrLast)
            return hrRangeTable[i].lpszName;
    }
    return NULL;  // not found
}

LPCTSTR GetSeverityString(HRESULT hr)
{
    static LPCTSTR rgszSEVERITY[] = {
        _T("SEVERITY_SUCCESS"),
        _T("SEVERITY_ERROR"),
    };
    return rgszSEVERITY[HRESULT_SEVERITY(hr)];
}

LPCTSTR GetFacilityString(HRESULT hr)
{
    static LPCTSTR rgszFACILITY[] = {
        _T("FACILITY_NULL"),
        _T("FACILITY_RPC"),
        _T("FACILITY_DISPATCH"),
        _T("FACILITY_STORAGE"),
        _T("FACILITY_ITF"),
        _T("FACILITY_ADSI"),
        _T("FACILITY_0x06"),
        _T("FACILITY_WIN32"),
        _T("FACILITY_WINDOWS"),
        _T("FACILITY_SSPI/FACILITY_MQ"),  // SSPI from AdsErr.h, MQ from mq.h
        _T("FACILITY_CONTROL"),
        _T("FACILITY_EDK"),
        _T("FACILITY_INTERNET"),
        _T("FACILITY_MEDIASERVER"),
        _T("FACILITY_MSMQ"),
        _T("FACILITY_SETUPAPI"),
    };
    if (HRESULT_FACILITY(hr) >= _countof(rgszFACILITY))
        switch (HRESULT_FACILITY(hr)) {
            case 0x7FF:
                return _T("FACILITY_BACKUP");
            case 0x800:
                return _T("FACILITY_EDB");
            case 0x900:
                return _T("FACILITY_MDSI");
            default:
                return _T("<Unknown Facility>");
        }
    return rgszFACILITY[HRESULT_FACILITY(hr)];
}
