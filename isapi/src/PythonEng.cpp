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

// NOTE: This code used to host the thread-pool used by dispatch to Python
// Some of the methods etc made alot more sense then.

#include "stdafx.h"
#include "Utils.h"
#include "PythonEng.h"
#include "pyExtensionObjects.h"
#include "pyFilterObjects.h"
#include "pyISAPI_messages.h"

extern HINSTANCE g_hInstance;
extern bool g_IsFrozen;
extern char g_CallbackModuleName[_MAX_PATH + _MAX_FNAME];

extern void InitExtensionTypes();
extern void InitFilterTypes();

/////////////////////////////////////////////////////////////////////
// Python  Engine
/////////////////////////////////////////////////////////////////////

CRITICAL_SECTION CPythonEngine::m_initLock;
bool CPythonEngine::m_haveInit = false;
PyObject *CPythonEngine::m_reload_exception = NULL;

CPythonEngine::CPythonEngine() { InitializeCriticalSection(&m_initLock); }

CPythonEngine::~CPythonEngine() { DeleteCriticalSection(&m_initLock); }

bool CPythonEngine::InitMainInterp(void)
{
    // ensure only 1 engine/thread initialises this only
    CSLock l(m_initLock);
    if (!m_haveInit) {
        PyGILState_STATE old_state;
        if (Py_IsInitialized())
            old_state = PyGILState_Ensure();
        else {
            Py_Initialize();
            old_state = PyGILState_UNLOCKED;
        }
        PyEval_InitThreads();

        if (!g_IsFrozen) {
            TCHAR *dll_path = GetModulePath();
            AddToPythonPath(dll_path);
            free(dll_path);
            PyErr_Clear();
        }

        // isapidllhandle to match dllhandle, frozendllhandle, etc :)  Also a
        // nice way for a program to know they are in an ISAPI context.
        PyObject *obh = PyLong_FromVoidPtr(g_hInstance);
        PySys_SetObject("isapidllhandle", obh);
        Py_XDECREF(obh);
        // Locate the special exception we use to trigger a reload.
        PyObject *isapi_package = PyImport_ImportModule("isapi");
        if (isapi_package)
            m_reload_exception = PyObject_GetAttrString(isapi_package, "InternalReloadException");
        Py_XDECREF(isapi_package);

        // ready our types.
        InitExtensionTypes();
        InitFilterTypes();

        PyGILState_Release(old_state);
        FindModuleName();
        m_haveInit = true;
    }
    return true;
}

void CPythonEngine::FindModuleName()
{
    char szFilePath[_MAX_PATH];
    char szBase[_MAX_FNAME];
    char *module_name;

    // If a name for the module has been magically setup (eg, via a frozen
    // app), then use it.  Otherwise, assume it is the DLL name without the
    // first character (ie, without the leading _)
    if (g_CallbackModuleName && *g_CallbackModuleName)
        module_name = g_CallbackModuleName;
    else {
        // find out where our DLL/EXE module lives
        // NOTE: the long file name does not get returned (don't know why)
        ::GetModuleFileNameA(g_hInstance, szFilePath, sizeof(szFilePath));
        ::_splitpath(szFilePath, NULL, NULL, szBase, NULL);
        module_name = szBase + 1;  // skip first char.
    }
    strncpy(m_module_name, module_name, sizeof(m_module_name) / sizeof(m_module_name[0]));
}

bool CPythonEngine::AddToPythonPath(LPCTSTR pPathName)
{
    PyObject *obPathList = PySys_GetObject("path");
    if (obPathList == NULL) {
        return false;
    }

    // Some pathnames have a leading '\\?\', which tells allows Unicode
    // win32 functions to avoid MAX_PATH limitations.  Notably,
    // GetModulePath for our extension DLL may - presumably as such a
    // path was specified by IIS when loading the DLL.
    // Current Python versions handle neither this, nor Unicode on
    // sys.path, so correct this here.
    size_t len = _tcslen(pPathName);
    if (len > 4 && _tcsncmp(pPathName, _T("\\\\?\\"), 4) == 0) {
        pPathName += 4;
        len -= 4;
    }
#if (PY_VERSION_HEX < 0x03000000)
    PyObject *obNew = PyString_FromStringAndSize(pPathName, len);
#else
    PyObject *obNew = PyUnicode_FromWideChar(pPathName, len);
#endif
    if (obNew == NULL) {
        return false;
    }

    bool bFnd = false;
    for (int i = 0; i < PyList_Size(obPathList); i++) {
        PyObject *obItem = PyList_GetItem(obPathList, i);
        if (PyObject_RichCompare(obNew, obItem, Py_EQ) == Py_True) {
            bFnd = true;
            break;
        }
    }

    if (!bFnd)
        PyList_Insert(obPathList, 0, obNew);

    Py_XDECREF(obNew);
    return true;
}

///////////////////////////////////////////////////////////////////////
//
// The callback manager
//
CPythonHandler::CPythonHandler()
    : m_namefactory(0),
      m_nameinit(0),
      m_namedo(0),
      m_nameterm(0),
      m_callback_init(0),
      m_callback_do(0),
      m_callback_term(0),
      m_handler(0)
{
    return;
}

bool CPythonHandler::Init(CPythonEngine *engine, const char *factory, const char *nameinit, const char *namedo,
                          const char *nameterm)
{
    if (!engine->InitMainInterp())
        return false;
    m_nameinit = nameinit;
    m_namedo = namedo;
    m_nameterm = nameterm;
    m_namefactory = factory;
    m_engine = engine;
    return LoadHandler(false);
}

bool CPythonHandler::LoadHandler(bool reload)
{
    char szErrBuf[1024];
    PyObject *m;
    CEnterLeavePython celp;
    m = PyImport_ImportModule(m_engine->m_module_name);
    if (m && reload) {
        PyObject *m_orig = m;
        m = PyImport_ReloadModule(m);
        Py_DECREF(m_orig);
    }
    if (!m) {
        _snprintf(szErrBuf, sizeof(szErrBuf) / sizeof(szErrBuf[0]), "Failed to import callback module '%s'",
                  m_engine->m_module_name);
        ExtensionError(NULL, szErrBuf);
    }
    if (m) {
        Py_XDECREF(m_handler);
        if (!((m_handler = PyObject_CallMethod(m, (char *)m_namefactory, NULL)))) {
            _snprintf(szErrBuf, sizeof(szErrBuf) / sizeof(szErrBuf[0]), "Factory function '%s' failed", m_namefactory);
            ExtensionError(NULL, szErrBuf);
        }
        Py_DECREF(m);
    }
    return m_handler != NULL;
}

bool CPythonHandler::CheckCallback(const char *cbname, PyObject **cb)
{
    if (*cb != NULL)
        return true;  // already have the callback.

    CEnterLeavePython celp;
    if (!m_handler) {
        PyErr_SetString(PyExc_RuntimeError, "The handler failed to load");
        return false;
    }
    *cb = PyObject_GetAttrString(m_handler, (char *)cbname);
    if (!*cb)
        ExtensionError(NULL, "Failed to locate the callback");
    return (*cb) != NULL;
}

// NOTE: Caller must setup and release thread-state - as we return a PyObject,
// the caller must at least Py_DECREF it, so must hold the lock.
PyObject *CPythonHandler::DoCallback(HANDLER_TYPE typ, PyObject *args)
{
    PyObject **ppcb;
    const char *cb_name;
    switch (typ) {
        case HANDLER_INIT:
            ppcb = &m_callback_init;
            cb_name = m_nameinit;
            break;
        case HANDLER_TERM:
            ppcb = &m_callback_term;
            cb_name = m_nameterm;
            break;
        default:
            ppcb = &m_callback_do;
            cb_name = m_namedo;
            break;
    }
    if (!CheckCallback(cb_name, ppcb))
        return NULL;

    return PyObject_Call(*ppcb, args, NULL);
}

PyObject *CPythonHandler::Callback(HANDLER_TYPE typ, const char *format /* = NULL */, ...)
{
    va_list va;
    PyObject *args;
    PyObject *ret = NULL;

    if (format && *format) {
        va_start(va, format);
        args = Py_VaBuildValue((char *)format, va);
        va_end(va);
    }
    else
        args = PyTuple_New(0);

    if (args == NULL)
        return NULL;

    if (!PyTuple_Check(args)) {
        PyObject *a;

        a = PyTuple_New(1);
        if (a == NULL)
            goto done;
        if (PyTuple_SET_ITEM(a, 0, args) < 0) {
            Py_DECREF(a);
            goto done;
        }
        // 'args' ref consumed by _SET_ITEM.
        args = a;
    }

    ret = DoCallback(typ, args);
    if (!ret) {
        if (m_engine->m_reload_exception && PyErr_ExceptionMatches(m_engine->m_reload_exception)) {
            PyErr_Clear();
            // Need to call term first
            PyObject *temp_args = Py_BuildValue("(i)", 0);
            ret = DoCallback(HANDLER_TERM, temp_args);
            Py_XDECREF(temp_args);
            if (!ret) {
                ExtensionError(NULL, "Terminating for reload failed");
                PyErr_Clear();
            }
            Py_XDECREF(ret);
            // Now force the reload and refresh of all callbacks.
            if (!LoadHandler(true))
                goto done;
            Py_XDECREF(m_callback_init);
            m_callback_init = NULL;
            Py_XDECREF(m_callback_do);
            m_callback_do = NULL;
            Py_XDECREF(m_callback_term);
            m_callback_term = NULL;
            // call init again
            temp_args = Py_BuildValue("(z)", NULL);
            ret = DoCallback(HANDLER_INIT, temp_args);
            Py_XDECREF(temp_args);
            if (!ret) {
                ExtensionError(NULL, "Reinitializing after import failed");
                PyErr_Clear();
            }
            Py_XDECREF(ret);
            // And make the original call again.
            ret = DoCallback(typ, args);
        }
    }
done:
    Py_DECREF(args);
    return ret;
}

void CPythonHandler::Term(void)
{
    // never shut down - Python leaks badly and has other
    // side effects if you repeatedly Init then Term
    Py_XDECREF(m_callback_init);
    Py_XDECREF(m_callback_do);
    Py_XDECREF(m_callback_term);
}

//////////////////////////////////////////////////////////////////////////////
// general error handler

void ExtensionError(CControlBlock *pcb, const char *errmsg)
{
    char *windows_error = ::GetLastError() ? ::FormatSysError(::GetLastError()) : NULL;
    {  // temp scope to release python lock
        CEnterLeavePython celp;
        PySys_WriteStderr("Internal Extension Error: %s\n", errmsg);
        if (windows_error)
            PySys_WriteStderr("Last Windows error: %s\n", windows_error);
        if (PyErr_Occurred()) {
            PyErr_Print();
            PyErr_Clear();
        }
    }  // end temp scope
    if (pcb) {
        char *htmlStream = HTMLErrorResp(errmsg);

        pcb->SetStatus(HSE_STATUS_ERROR);
        pcb->SetLogMessage(errmsg);
        HSE_SEND_HEADER_EX_INFO SendHeaderExInfo;
        SendHeaderExInfo.pszStatus = "200 OK";
        SendHeaderExInfo.cchStatus = strlen(SendHeaderExInfo.pszStatus);
        SendHeaderExInfo.pszHeader = "Content-type: text/html\r\n\r\n";
        SendHeaderExInfo.cchHeader = strlen(SendHeaderExInfo.pszHeader);
        SendHeaderExInfo.fKeepConn = FALSE;
        EXTENSION_CONTROL_BLOCK *ecb = pcb->GetECB();
        ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &SendHeaderExInfo, NULL, NULL);
        pcb->WriteStream(htmlStream, strlen(htmlStream));
        if (windows_error) {
            static char *chunk = "<br>Last Windows error:";
            pcb->WriteStream(chunk, strlen(chunk));
            pcb->WriteStream(windows_error, strlen(windows_error));
        }
    }
    const char *inserts[] = {errmsg, windows_error ? windows_error : "n/a"};
    WriteEventLogMessage(EVENTLOG_ERROR_TYPE, E_PYISAPI_EXTENSION_FAILED, 2, inserts);
    if (windows_error)
        free(windows_error);
}

void FilterError(CFilterContext *pfc, const char *errmsg)
{
    char *windows_error = ::GetLastError() ? ::FormatSysError(::GetLastError()) : NULL;

    CEnterLeavePython celp;
    PySys_WriteStderr("Internal Filter Error: %s\n", errmsg);
    if (PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
    }
    const char *inserts[] = {errmsg, windows_error ? windows_error : "n/a"};
    WriteEventLogMessage(EVENTLOG_ERROR_TYPE, E_PYISAPI_FILTER_FAILED, 2, inserts);
    if (windows_error)
        free(windows_error);
    // what else to do here? AddResponseHeaders->WriteClient?
}
