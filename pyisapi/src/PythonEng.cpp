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

extern HINSTANCE g_hInstance;
extern bool g_IsFrozen;
extern char g_CallbackModuleName[_MAX_PATH + _MAX_FNAME];

/////////////////////////////////////////////////////////////////////
// Python  Engine
/////////////////////////////////////////////////////////////////////

CRITICAL_SECTION CPythonEngine::m_initLock;
bool CPythonEngine::m_haveInit = false;

CPythonEngine::CPythonEngine() : m_handler(0), m_callback(0)
{
	InitializeCriticalSection(&m_initLock);
}

CPythonEngine::~CPythonEngine()
{
	DeleteCriticalSection(&m_initLock);
}

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
			char *dll_path = GetModulePath();
			AddToPythonPath(dll_path);
			free(dll_path);
			PyErr_Clear();
			}
	
		// isapidllhandle to match dllhandle, frozendllhandle, etc :)  Also a 
		// nice way for a program to know they are in an ISAPI context.
		PyObject *obh = PyLong_FromVoidPtr(g_hInstance);
		PySys_SetObject("isapidllhandle", obh);
		Py_XDECREF(obh);
		
		PyGILState_Release(old_state);
		m_haveInit = true;
	}
	return true;
}

bool CPythonEngine::LoadHandler(char *factory_name)
{
	PyObject *m;
	TCHAR szFilePath[_MAX_PATH];
	TCHAR szBase[_MAX_FNAME];
	TCHAR szErrBuf[1024];
	TCHAR *module_name;
	
	assert(m_handler==NULL); // should only be called once.

	// If a name for the module has been magically setup (eg, via a frozen
	// app), then use it.  Otherwise, assume it is the DLL name without the
	// first character (ie, without the leading _)
	if (g_CallbackModuleName && *g_CallbackModuleName)
		module_name = g_CallbackModuleName;
	else {
		// find out where our DLL/EXE module lives
		// NOTE: the long file name does not get returned (don't know why)
		if (!g_IsFrozen) {
			::GetModuleFileName(g_hInstance, szFilePath, sizeof(szFilePath));
			::_splitpath( szFilePath, NULL, NULL, szBase, NULL);
			module_name = szBase + 1; // skip first char.
		} else {
			// When frozen, the module is always called 'PyISAPI_config'
			strcpy(szBase, "pyISAPI_config.py");
			module_name = szBase;
		}
	}
	PyGILState_STATE old_state = PyGILState_Ensure();
	if (!(m = PyImport_ImportModule(module_name))) {
		_snprintf(szErrBuf, sizeof(szErrBuf)/sizeof(szErrBuf[0]), 
				  "Failed to import callback module '%s'", module_name);
		ExtensionError(NULL, szErrBuf);
	}
	if (m) {
		if (!((m_handler = PyObject_CallMethod(m, factory_name, NULL)))) {
			_snprintf(szErrBuf, sizeof(szErrBuf)/sizeof(szErrBuf[0]), 
			          "Factory function '%s' failed", factory_name);
			ExtensionError(NULL, szErrBuf);
		}
		Py_DECREF(m);
    }
	PyGILState_Release(old_state);
	return m_handler != NULL;
}

// Set the current 'callback' - all future callbacks will be made to
// the fetched method.
bool CPythonEngine::SetCallback(const char *cbname)
{
	assert(m_handler);
	if (!m_handler)
		return NULL;

	PyGILState_STATE old_state = PyGILState_Ensure();
	
	Py_XDECREF(m_callback);
	m_callback = PyObject_GetAttrString(m_handler, (char *)cbname);
	if (!m_callback)
		ExtensionError(NULL, "Failed to locate the callback");
	PyGILState_Release(old_state);
	return m_callback != NULL;
}

// NOTE: Caller must setup and release thread-state - as we return a PyObject,
// the caller must at least Py_DECREF it, so must hold the lock.
PyObject *CPythonEngine::Callback(
	const char *format /* = NULL */,
	...
	)
{
	assert(m_callback);
	if (!m_callback)
		return NULL;

	va_list va;
	PyObject *args, *retval;

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
			return NULL;
		if (PyTuple_SetItem(a, 0, args) < 0)
			return NULL;
		args = a;
	}
	retval = PyObject_Call(m_callback, args, NULL);
	Py_DECREF(args);
	return retval;

}

void CPythonEngine::ShutdownInterp(void)
{
	// never shut down - Python leaks badly and has other
	// side effects if you repeatedly Init then Term
}

bool CPythonEngine::AddToPythonPath(LPCTSTR pPathName)
{
	PyObject *obPathList = PySys_GetObject(_T("path"));
	if (obPathList==NULL) {
		return false;
	}

	PyObject *obNew = PyString_FromString(pPathName);
	if (obNew==NULL) {
		return false;
	}

	bool bFnd=false;
	for (int i=0; i<PyList_Size(obPathList); i++){
		PyObject * obItem = PyList_GetItem(obPathList, i);
		if(PyObject_Compare(obNew, obItem) == 0){
			bFnd = true;
			break;
		}
	}

	if (!bFnd)
		PyList_Insert(obPathList, 0, obNew);

	Py_XDECREF(obNew);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// general error handler

void ExtensionError(CControlBlock *pcb, LPCTSTR errmsg)
{
	char *windows_error = ::GetLastError() ?
	                          ::FormatSysError(::GetLastError()) : NULL;
	PyGILState_STATE s = PyGILState_Ensure();
	PySys_WriteStderr("Internal Extension Error: %s\n", errmsg);
	if (windows_error)
		PySys_WriteStderr("Last Windows error: %s\n", windows_error);
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	PyGILState_Release(s);
	if (pcb) {
		char *htmlStream = HTMLErrorResp(errmsg);
	
		pcb->SetStatus(HSE_STATUS_ERROR);
		pcb->SetLogMessage(errmsg);
		pcb->WriteHeaders(_T("200 OK"), _T("Content-type: text/html\r\n\r\n"), false);
		pcb->WriteStream(htmlStream, strlen(htmlStream));
		if (windows_error) {
			static char *chunk = "<br>Last Windows error:";
			pcb->WriteStream(chunk, strlen(chunk));
			pcb->WriteStream(windows_error, strlen(windows_error));
		}
	}
	if (windows_error)
		free(windows_error);
}

void FilterError(CFilterContext *pfc,  LPCTSTR errmsg)
{
	PyGILState_STATE s = PyGILState_Ensure();
	PySys_WriteStderr("Internal Filter Error: %s\n", errmsg);
	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
	PyGILState_Release(s);
	// what else to do here? AddResponseHeaders->WriteClient?
}
