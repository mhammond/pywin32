/***********************************************************

PyWinTypes.cpp -- implementation of standard win32 types


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "windows.h"
#include "malloc.h"

#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

PyObject * PyWinExc_ApiError = NULL;
PyObject * PyWinExc_COMError = NULL;

extern PyObject *PyWinMethod_NewHKEY(PyObject *self, PyObject *args);

#ifdef MS_WINCE
// Where is this supposed to come from on CE???
const GUID GUID_NULL \
                = { 0, 0, 0, { 0, 0,  0,  0,  0,  0,  0,  0 } };
#endif

BOOL PySocket_AsSOCKET
//-------------------------------------------------------------------------
// Helper function for dealing with socket arguments.
(
	PyObject *obSocket,
	// [in] Python object being converted into a SOCKET handle.
	SOCKET *ps
	// [out] Returned socket handle
)
{
	PyObject *o = NULL;
	PyObject *t = NULL;
	PyObject *out = NULL;

	if (PyInt_Check(obSocket))
	{
		*ps = (SOCKET)PyInt_AS_LONG(obSocket);
	}
	else if (PyInstance_Check(obSocket))
	{
		o = PyObject_GetAttrString(obSocket, "fileno");
		if (o == NULL)
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "socket instance does not have the socket in the 'fileno' attribute");
			return FALSE;
		}
		if (PyInt_Check(o))
		{
			*ps = (SOCKET)PyInt_AS_LONG(o);
		}
		else if (PyCallable_Check(o))
		{
			out = PyObject_CallObject(o, NULL);
			if (out == NULL)
			{
				Py_DECREF(o);
				return FALSE;
			}
			if (PyInt_Check(out))
			{
				*ps = (SOCKET)PyInt_AS_LONG(out);
			}
			else
			{
				Py_DECREF(o);
				PyErr_SetString(PyExc_TypeError, "socket instance's 'fileno' attribute is not a socket");
				return FALSE;
			}
		}
		else
		{
			Py_DECREF(o);
			PyErr_SetString(PyExc_TypeError, "socket instance's 'fileno' attribute is not a socket");
			return FALSE;
		}
		Py_DECREF(o);
	}

	return TRUE;
}


#ifndef PYCOM_USE_FREE_THREAD
DWORD dwTlsIndex = 0;

PyInterpreterState *PyWin_InterpreterState;

// This function must be called at some time when the interpreter lock and state is valid.
// Called by init{module} functions and also COM factory entry point.
void PyWinInterpreterState_Ensure()
{
	if (PyWin_InterpreterState==NULL) {
		PyThreadState *threadStateSave = PyThreadState_Swap(NULL);
		if (threadStateSave==NULL)
			Py_FatalError("Can not setup interpreter state, as current state is invalid");

		PyWin_InterpreterState = threadStateSave->interp;
		PyThreadState_Swap(threadStateSave);
	}
}

void PyWinInterpreterState_Free()
{
	PyWinThreadState_Free();
	PyWin_InterpreterState = NULL; // Eek - should I be freeing something?
}
// PyThreadState *_tlsThisThreadState = NULL; // thread local storage.

// This structure is stored in the TLS slot.  At this stage only a Python thread state
// is kept, but this may change in the future...
struct ThreadData{
	PyThreadState *ts;
};

// Ensure that we have a Python thread state available to use.
// If this is called for the first time on a thread, it will allocate
// the thread state.  This does NOT change the state of the Python lock.
// Returns TRUE if a new thread state was created, or FALSE if a
// thread state already existed.
BOOL PyWinThreadState_Ensure()
{
	ThreadData *pData = (ThreadData *)TlsGetValue(dwTlsIndex);
	if (pData==NULL) { /* First request on this thread */
		/* Check we have an interpreter state */
		if (PyWin_InterpreterState==NULL) {
				Py_FatalError("Can not setup thread state, as have no interpreter state");
		}
		pData = (ThreadData *)LocalAlloc(LMEM_ZEROINIT, sizeof(ThreadData));
		if (!pData)
			Py_FatalError("Out of memory allocating thread state.");
		TlsSetValue(dwTlsIndex, pData);
		pData->ts = PyThreadState_New(PyWin_InterpreterState);
		return TRUE; // Did create a thread state state
	}
	return FALSE; // Thread state was previously created
}

// Asuming we have a valid thread state, acquire the Python lock.
void PyWinInterpreterLock_Acquire()
{
	ThreadData *pData = (ThreadData *)TlsGetValue(dwTlsIndex);
	PyThreadState *thisThreadState = pData->ts;
	PyEval_AcquireThread(thisThreadState);
}

// Asuming we have a valid thread state, release the Python lock.
void PyWinInterpreterLock_Release()
{
	ThreadData *pData = (ThreadData *)TlsGetValue(dwTlsIndex);
	PyThreadState *thisThreadState = pData->ts;
	PyEval_ReleaseThread(thisThreadState);
}

// Free the thread state for the current thread
// (Presumably previously create with a call to
// PyWinThreadState_Ensure)
void PyWinThreadState_Free()
{
	ThreadData *pData = (ThreadData *)TlsGetValue(dwTlsIndex);
	if (!pData) return;
	PyThreadState *thisThreadState = pData->ts;
	PyThreadState_Delete(thisThreadState);
	TlsSetValue(dwTlsIndex, NULL);
	LocalFree(pData);
}

void PyWinThreadState_Clear()
{
	ThreadData *pData = (ThreadData *)TlsGetValue(dwTlsIndex);
	PyThreadState *thisThreadState = pData->ts;
	PyThreadState_Clear(thisThreadState);
}
#endif

/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *PyWin_SetAPIError(char *fnName, long err /*= 0*/)
{
	const int bufSize = 512;
	TCHAR buf[bufSize];
	DWORD errorCode = err == 0 ? GetLastError() : err;
	BOOL bHaveMessage = FALSE;
	if (errorCode)
		bHaveMessage = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buf, bufSize, NULL )>0;
	if (!bHaveMessage)
		_tcscpy(buf,_T("No error message is available"));
	/* strip trailing cr/lf */
	int end = _tcslen(buf)-1;
	if (end>1 && (buf[end-1]==_T('\n') || buf[end-1]==_T('\r')))
		buf[end-1] = _T('\0');
	else
		if (end>0 && (buf[end]==_T('\n') || buf[end]==_T('\r')))
			buf[end]=_T('\0');
	PyObject *obBuf = PyString_FromTCHAR(buf);
	PyObject *v = Py_BuildValue("(isO)", errorCode, fnName, obBuf);
	Py_XDECREF(obBuf);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
	return NULL;
}

// This function sets a basic COM error - it is a valid COM
// error, but may not contain rich error text about the error.
// Designed to be used before pythoncom has been loaded.
// If a COM extension wants to raise a COM error, it should use
// the "real" functions exposed via pythoncom.
PyObject *PyWin_SetBasicCOMError(HRESULT hr)
{

	TCHAR buf[255];
	int bufSize = sizeof(buf);
	int numCopied = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, buf, bufSize, NULL );
	if (numCopied>0) {
		if (numCopied<bufSize) {
			// trim trailing crap
			if (numCopied>2 && (buf[numCopied-2]==_T('\n')||buf[numCopied-2]==_T('\r')))
				buf[numCopied-2] = _T('\0');
		}
	} else {
		wsprintf(buf, _T("COM Error 0x%x"), hr);
	}
	PyObject *obBuf = PyString_FromTCHAR(buf);
	PyObject *evalue = Py_BuildValue("iOzz", hr, obBuf, NULL, NULL);
	Py_XDECREF(obBuf);
	PyErr_SetObject(PyWinExc_COMError, evalue);
	Py_XDECREF(evalue);
	return NULL;
}

// @pymethod <o PyUnicode>|pywintypes|Unicode|Creates a new Unicode object
PYWINTYPES_EXPORT PyObject *PyWin_NewUnicode(PyObject *self, PyObject *args)
{
#ifdef PYWIN_USE_PYUNICODE
	char *string;
	int slen;
	if (!PyArg_ParseTuple(args, "t#", &string, &slen))
		return NULL;
    return PyUnicode_DecodeMBCS(string, slen, NULL);
#else
	PyObject *obString;
	// @pyparm string|str||The string to convert.
	if (!PyArg_ParseTuple(args, "O", &obString))
		return NULL;
	PyUnicode *result = new PyUnicode(obString);
	if ( result->m_bstrValue )
		return result;
	Py_DECREF(result);
	/* an error should have been raised */
	return NULL;
#endif
}

// @pymethod <o PyUnicode>|pywintypes|UnicodeFromRaw|Creates a new Unicode object from raw binary data
static PyObject *PyWin_NewUnicodeFromRaw(PyObject *self, PyObject *args)
{
	const char * value;
	unsigned int numBytes;

	// @pyparm string|str||The string containing the binary data.
	if (!PyArg_ParseTuple(args, "s#", &value, &numBytes))
		return NULL;

#ifdef PYWIN_USE_PYUNICODE
	return PyWinObject_FromOLECHAR( (OLECHAR *)value, numBytes/sizeof(OLECHAR) );
#else
	PyUnicode *result = new PyUnicode(value, numBytes);
	if ( result->m_bstrValue )
		return result;
	Py_DECREF(result);
	/* an error should have been raised */
	return NULL;
#endif
}

#ifndef MS_WINCE /* This code is not available on Windows CE */

// @pymethod int, int|pywintypes|IsTextUnicode|Determines whether a buffer probably contains a form of Unicode text.
static PyObject *PyWin_IsTextUnicode(PyObject *self, PyObject *args)
{
	const char * value;
	unsigned int numBytes;
	int flags;

	// @pyparm string|str||The string containing the binary data.
	// @pyparm int|flags||Determines the specific tests to make
	if (!PyArg_ParseTuple(args, "s#i", &value, &numBytes, &flags))
		return NULL;

	DWORD rc = IsTextUnicode((LPVOID)value, numBytes, &flags);
	return Py_BuildValue("ii", rc, flags);
	// @rdesc The function returns (result, flags), both integers.
	// <nl>result is nonzero if the data in the buffer passes the specified tests.
	// <nl>result is zero if the data in the buffer does not pass the specified tests.
	// <nl>In either case, flags contains the results of the specific tests the function applied to make its determination.
}

// @pymethod <o PyIID>|pywintypes|CreateGuid|Creates a new, unique GUIID.
static PyObject *PyWin_CreateGuid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":CreateGuid"))
		return NULL;
	GUID guid;
	CoCreateGuid(&guid);
	return PyWinObject_FromIID(guid);
}
// @pymethod <o PyTime>|pywintypes|DosDateTimeToTime|Converts an MS-DOS Date/Time to a standard Time object.
static PyObject *PyWin_DosDateTimeToTime(PyObject *self, PyObject *args)
{
	WORD wFatDate, wFatTime;
	if (!PyArg_ParseTuple(args, "hh", (WORD *)&wFatDate, (WORD *)&wFatTime))
		return NULL;
	FILETIME fd;
	if (!DosDateTimeToFileTime(wFatDate, wFatTime, &fd))
		return PyWin_SetAPIError("DosDateTimeToFileTime");
	return new PyTime(fd);
}
#endif /* MS_WINCE */

/* List of functions exported by this module */
// @module pywintypes|A module which supports common Windows types.
static struct PyMethodDef pywintypes_functions[] = {
#ifndef MS_WINCE
	{"DosDateTimeToTime", PyWin_DosDateTimeToTime, 1}, // @pymeth DosDateTimeToTime|Converts an MS-DOS Date/Time to a standard Time object
#endif
	{"Unicode",     PyWin_NewUnicode, 1}, 	// @pymeth Unicode|Creates a new <o PyUnicode> object
	{"UnicodeFromRaw",     PyWin_NewUnicodeFromRaw, 1},	// @pymeth UnicodeFromRaw|Creates a new <o PyUnicode> object from raw binary data
#ifndef MS_WINCE
	{"IsTextUnicode",      PyWin_IsTextUnicode, 1}, // @pymeth IsTextUnicode|Determines whether a buffer probably contains a form of Unicode text.
#endif
	{"OVERLAPPED",  PyWinMethod_NewOVERLAPPED, 1}, 	// @pymeth OVERLAPPED|Creates a new <o PyOVERLAPPED> object
#ifndef NO_PYWINTYPES_IID
	{"IID",			PyWinMethod_NewIID, 1 },         // @pymeth IID|Makes an <o PyIID> object from a string.
#endif
#ifndef NO_PYWINTYPES_TIME
	{"Time",		PyWinMethod_NewTime, 1 },		// @pymeth Time|Makes a <o PyTime> object from the argument.  Argument can be an integer/float or a tuple (as returned by time module functions).
#endif
#ifndef MS_WINCE
	{"CreateGuid",  PyWin_CreateGuid, 1 },      // @pymeth CreateGuid|Creates a new, unique GUIID.
	{"ACL",         PyWinMethod_NewACL, 1 },      // @pymeth ACL|Creates a new <o PyACL> object.
	{"SID",         PyWinMethod_NewSID, 1 },      // @pymeth SID|Creates a new <o PySID> object.
	{"SECURITY_ATTRIBUTES",         PyWinMethod_NewSECURITY_ATTRIBUTES, 1 },      // @pymeth SECURITY_ATTRIBUTES|Creates a new <o PySECURITY_ATTRIBUTES> object.
	{"SECURITY_DESCRIPTOR",         PyWinMethod_NewSECURITY_DESCRIPTOR, 1 },      // @pymeth SECURITY_DESCRIPTOR|Creates a new <o PySECURITY_DESCRIPTOR> object.
#endif
	{"HANDLE",      PyWinMethod_NewHANDLE, 1 },      // @pymeth HANDLE|Creates a new <o PyHANDLE> object.
	{"HKEY",        PyWinMethod_NewHKEY, 1 },      // @pymeth HKEY|Creates a new <o PyHKEY> object.
	{NULL,			NULL}
};

void PyWinGlobals_Ensure()
{
	PyWinInterpreterState_Ensure();
	if (PyWinExc_ApiError==NULL) {
		PyWinExc_ApiError = PyErr_NewException("pywintypes.api_error", NULL, NULL);
		PyWinExc_COMError = PyErr_NewException("pywintypes.com_error", NULL, NULL);
	}
}

void PyWinGlobals_Free()
{
	PyWinInterpreterState_Free();
	Py_XDECREF(PyWinExc_ApiError);
	PyWinExc_ApiError = NULL;
	Py_XDECREF(PyWinExc_COMError);
	PyWinExc_COMError = NULL;
}


static CRITICAL_SECTION g_csMain;
#ifdef _DEBUG
static DWORD g_cGlobalLocks = 0;
#endif

void PyWin_AcquireGlobalLock(void)
{
	EnterCriticalSection(&g_csMain);
#ifdef _DEBUG
	++g_cGlobalLocks;
#endif
//	LogEvent("add a lock");
}
void PyWin_ReleaseGlobalLock(void)
{
//	LogEvent("remove a lock");
#ifdef _DEBUG
	--g_cGlobalLocks;
#endif
	LeaveCriticalSection(&g_csMain);
}

extern "C" __declspec(dllexport)
void initpywintypes(void)
{
  // ensure the framework has a valid thread state to work with.
  PyWinGlobals_Ensure();

  // Note we assume the Python global lock has been acquired for us already.
  PyObject *dict, *module;
  module = Py_InitModule("pywintypes", pywintypes_functions);
  if (!module) /* Eeek - some serious error! */
    return;
  dict = PyModule_GetDict(module);
  if (!dict) return; /* Another serious error!*/
  if (PyWinExc_ApiError == NULL || PyWinExc_COMError == NULL) {
	  PyErr_SetString(PyExc_MemoryError, "Could not initialise the error objects");
	  return;
  }

  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
  PyDict_SetItemString(dict, "com_error", PyWinExc_COMError);

  PyDict_SetItemString(dict, "TRUE", Py_True);
  PyDict_SetItemString(dict, "FALSE", Py_False);

  // Add a few types.
#ifndef NO_PYWINTYPES_TIME
  PyDict_SetItemString(dict, "TimeType", (PyObject *)&PyTimeType);
#endif // NO_PYWINTYPES_TIME
#ifndef NO_PYWINTYPES_IID
  PyDict_SetItemString(dict, "IIDType", (PyObject *)&PyIIDType);
#endif // NO_PYWINTYPES_IID
  PyDict_SetItemString(dict, "UnicodeType", (PyObject *)&PyUnicodeType);
#ifndef MS_WINCE
  PyDict_SetItemString(dict, "SECURITY_ATTRIBUTESType", (PyObject *)&PySECURITY_ATTRIBUTESType);
  PyDict_SetItemString(dict, "SIDType", (PyObject *)&PySIDType);
  PyDict_SetItemString(dict, "ACLType", (PyObject *)&PyACLType);
#endif
  PyDict_SetItemString(dict, "HANDLEType", (PyObject *)&PyHANDLEType);
  PyDict_SetItemString(dict, "OVERLAPPEDType", (PyObject *)&PyHANDLEType);
}

#ifndef BUILD_FREEZE
#define DLLMAIN DllMain
#define DLLMAIN_DECL
#else
#define DLLMAIN DllMainpywintypes
#define DLLMAIN_DECL __declspec(dllexport)
#endif

#ifndef MS_WINCE
extern "C" 
#endif
DLLMAIN_DECL
BOOL WINAPI DLLMAIN(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH: {
			/*
			** One of threee situations is now occurring:
			**
			**   1) Python is loading this DLL as part of its standard import
			**      mechanism.  Python has been initialized already and will
			**      eventually call initpywintypes().
			**
			**   2) Python is importing another DLL as part of its standard
			**      import mechanism, and that DLL is linked against pywintypes.dll
			**      Python has been initialized already, but may _never_ call
			**      initpywintypes(). (until the user does an explicit 
			**      "import pywintypes"
			**
			**   3) The OLE system is loading this DLL to serve out a particular
			**      object via PythonCOM.DLL (this is always loaded first, as PythonCOM
			**      links against us).  Python may or may not be initialized (it will be
			**      initialized if the client of the object is in this process
			**      and the Python interpreter is running in that process).
			**      initpywintypes and initpythoncom() may or may not be called.
			**
			        This code actually makes mo assumptions about it, but the
					comment remains true!

			/*
			** For various process-level global locks.  Strictly speaking, this
			** could throw a C++ exception, but we don't care to trap that.
			*/
			InitializeCriticalSection(&g_csMain);
			dwTlsIndex = TlsAlloc();
			break;
		}
		case DLL_PROCESS_DETACH: 
		{
			DeleteCriticalSection(&g_csMain);
			TlsFree(dwTlsIndex);
#ifdef _DEBUG
			if ( g_cGlobalLocks )
			{
// ### need to fix up with some "correct" code ...
#if 0
				char buf[100];
				wsprintf(buf, "non-zero global lock count: %ld", g_cGlobalLocks);
				LogEvent(buf);
#endif
			}
#endif
			break;
		}
		default:
			break;
	}
	return TRUE;    // ok
}

