//  MODULE:   PythonService.exe
//
//  PURPOSE:  An executable that hosts Python services.
//            This source file is used to compile 2 discrete targets:
//            * servicemanager.pyd - A Python extension that contains
//              all the functionality.
//            * PythonService.exe - This simply loads servicemanager.pyd, and
//              calls a public function.  Note that PythonService.exe may one
//              day die - it is now possible for python.exe to directly host
//              services.
//
// @doc

// We use DCOM constants and possible CoInitializeEx.
#define _WIN32_DCOM

#include "windows.h"
#include "objbase.h"
#include "Python.h"
#include "tchar.h"
#include "PyWinTypes.h"

#ifdef PYSERVICE_BUILD_DLL
#define PYSERVICE_EXPORT extern "C" __declspec(dllexport)
#else
#define PYSERVICE_EXPORT extern "C" __declspec(dllimport)
#endif

PYSERVICE_EXPORT BOOL PythonService_Initialize(const TCHAR *evtsrc_name, const TCHAR *evtsrc_file);
PYSERVICE_EXPORT void PythonService_Finalize();
PYSERVICE_EXPORT BOOL PythonService_PrepareToHostSingle(PyObject *);
PYSERVICE_EXPORT BOOL PythonService_PrepareToHostMultiple(const TCHAR *service_name, PyObject *klass);
PYSERVICE_EXPORT BOOL PythonService_StartServiceCtrlDispatcher();
PYSERVICE_EXPORT int PythonService_main(int argc, char **argv);

TCHAR g_szEventSourceName[MAX_PATH] = _T("Python Service");
BOOL bServiceDebug = FALSE;

// Globals
HINSTANCE g_hdll = 0; // remains zero in the exe stub.

static void ReportAPIError(DWORD msgCode, DWORD errCode = 0);
static void ReportPythonError(DWORD);
static BOOL ReportError(DWORD, LPCTSTR *inserts = NULL, WORD errorType = EVENTLOG_ERROR_TYPE);

#include "PythonServiceMessages.h"

// Useful for debugging problems that only show themselves when run under the SCM
#define LOG_TRACE_MESSAGE(msg) {\
		LPTSTR  lpszStrings[] = {_T(msg), NULL}; \
		ReportError(MSG_IR1, (LPCTSTR *)lpszStrings, EVENTLOG_INFORMATION_TYPE); \
		}

#ifdef PYSERVICE_BUILD_DLL // The bulk of this file is only used when building the core DLL.

#define MAX_SERVICES 10

typedef struct {
	PyObject *klass; // The Python class we instantiate as the service.
	SERVICE_STATUS_HANDLE   sshStatusHandle; // the handle for this service.
	PyObject *obServiceCtrlHandler; // The Python control handler for the service.
} PY_SERVICE_TABLE_ENTRY;

// Globals
// Will be set to one of SERVICE_WIN32_OWN_PROCESS etc flags.
DWORD g_serviceProcessFlags = 0; 

// The global SCM dispatch table.  A trailing NULL indicates to the SCM
// how many are used - as we add new entries, we null the next.
static SERVICE_TABLE_ENTRY   DispatchTable[] = 
{ 
    { NULL,              NULL         } 
}; 
// A parallel array of Python information for the service.
static PY_SERVICE_TABLE_ENTRY PythonServiceTable[MAX_SERVICES];

#define RESOURCE_SERVICE_NAME 1016 // resource ID in the EXE of the service name

// internal function prototypes
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
BOOL WINAPI DebugControlHandler ( DWORD dwCtrlType );
DWORD WINAPI service_ctrl_ex(DWORD, DWORD, LPVOID, LPVOID);

BOOL RegisterPythonServiceExe(void);

static PY_SERVICE_TABLE_ENTRY *FindPythonServiceEntry(LPCTSTR svcName);

static PyObject *LoadPythonServiceClass(char *svcInitString);
static PyObject *LoadPythonServiceInstance(PyObject *,
										DWORD dwArgc,
										LPTSTR *lpszArgv );
static BOOL LocatePythonServiceClassString( TCHAR *svcName, char *buf, int cchBuf);


// Some handy service statuses we can use without filling at runtime.
SERVICE_STATUS neverStartedStatus = {
	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_STOPPED,
    0, // dwControlsAccepted,
    ERROR_SERVICE_SPECIFIC_ERROR, // dwWin32ExitCode; 
    1, // dwServiceSpecificExitCode; 
    0, // dwCheckPoint; 
    0 };

SERVICE_STATUS errorStatus = {
	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_STOP_PENDING,
    0, // dwControlsAccepted,
    ERROR_SERVICE_SPECIFIC_ERROR, // dwWin32ExitCode; 
    1, // dwServiceSpecificExitCode; 
    0, // dwCheckPoint; 
    5000 };

SERVICE_STATUS startingStatus = {
	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_START_PENDING,
    0, // dwControlsAccepted,
    0, // dwWin32ExitCode; 
    0, // dwServiceSpecificExitCode; 
    0, // dwCheckPoint; 
    5000 };

SERVICE_STATUS stoppedStatus = {
	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_STOPPED,
    0, // dwControlsAccepted,
    0, // dwWin32ExitCode; 
    0, // dwServiceSpecificExitCode; 
    0, // dwCheckPoint; 
    0 };

///////////////////////////////////////////////////////////////////////
//
//
// ** The builtin Python module - referenced as 'servicemanager' by
// ** Python code
//
//
///////////////////////////////////////////////////////////////////////
static PyObject *servicemanager_startup_error;

static PyObject *DoLogMessage(WORD errorType, PyObject *obMsg)
{
	WCHAR *msg;
	if (!PyWinObject_AsWCHAR(obMsg, &msg))
		return NULL;
	DWORD errorCode = errorType==EVENTLOG_ERROR_TYPE ? PYS_E_GENERIC_ERROR : PYS_E_GENERIC_WARNING;
	LPCTSTR inserts[] = {msg, NULL};
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReportError(errorCode, inserts, errorType);
	PyWinObject_FreeWCHAR(msg);
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("RegisterEventSource/ReportEvent");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |servicemanager|LogMsg|Logs a specific message
static PyObject *PyLogMsg(PyObject *self, PyObject *args)
{
	PyObject *obStrings;
	WORD errorType;
	DWORD code;
	LPCTSTR *pStrings = NULL;
	PyObject *rc = NULL;
	int numStrings = 0;
	BOOL ok = FALSE;

	// @pyparm int|errorType||
	// @pyparm int|eventId||
	// @pyparm (string, )|inserts|None|
	if (!PyArg_ParseTuple(args, "il|O:LogMsg", &errorType, &code, &obStrings))
		return NULL;

	if (obStrings==Py_None) {
		pStrings = NULL;
	} else if (PySequence_Check(obStrings)) {
		numStrings = PySequence_Length(obStrings);
		pStrings = new LPCTSTR [numStrings+1];
		if (pStrings==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating string arrays");
			goto cleanup;
		}
		memset(pStrings, 0, sizeof(TCHAR *)*(numStrings+1)); // this also terminates array!
		for (int i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(obStrings, i);
			if (obString==NULL) {
				goto cleanup;
			}
			BOOL ok = PyWinObject_AsTCHAR(obString, (LPTSTR *)(pStrings+i));
			Py_XDECREF(obString);
			if (!ok)
				goto cleanup;
		}
	} else {
		PyErr_SetString(PyExc_TypeError, "strings must be None or a sequence");
		goto cleanup;
	}
	Py_BEGIN_ALLOW_THREADS
	ok = ReportError(code, pStrings, errorType);
	Py_END_ALLOW_THREADS
	if (ok) {
		Py_INCREF(Py_None);
		rc = Py_None;
	} else
		PyWin_SetAPIError("RegisterEventSource/ReportEvent");

cleanup:
	if (pStrings) {
		for (int i=0;i<numStrings;i++)
			PyWinObject_FreeTCHAR((LPTSTR)pStrings[i]);
		delete [] pStrings;
	}
	return rc;

}

// @pymethod |servicemanager|LogInfoMsg|Logs a generic informational message to the event log
static PyObject *PyLogInfoMsg(PyObject *self, PyObject *args)
{
	PyObject *obMsg;
	// @pyparm <o PyUnicode>|msg||The message to write.
	if (!PyArg_ParseTuple(args, "O:LogInfoMsg", &obMsg))
		return NULL;
	return DoLogMessage(EVENTLOG_INFORMATION_TYPE, obMsg);
}

// @pymethod |servicemanager|LogWarningMsg|Logs a generic warning message to the event log
static PyObject *PyLogWarningMsg(PyObject *self, PyObject *args)
{
	PyObject *obMsg;
	// @pyparm <o PyUnicode>|msg||The message to write.
	if (!PyArg_ParseTuple(args, "O:LogWarningMsg", &obMsg))
		return NULL;
	return DoLogMessage(EVENTLOG_WARNING_TYPE, obMsg);
}

// @pymethod |servicemanager|LogErrorMsg|Logs a generic error message to the event log
static PyObject *PyLogErrorMsg(PyObject *self, PyObject *args)
{
	// @pyparm <o PyUnicode>|msg||The message to write.
	PyObject *obMsg;
	if (!PyArg_ParseTuple(args, "O:LogErrorMsg", &obMsg))
		return NULL;
	return DoLogMessage(EVENTLOG_ERROR_TYPE, obMsg);
}

// @pymethod int/None|servicemanager|RegisterServiceCtrlHandler|Registers the Python service control handler function.
static PyObject *PyRegisterServiceCtrlHandler(PyObject *self, PyObject *args)
{
	PyObject *nameOb, *obCallback;
	// @pyparm <o PyUnicode>|serviceName||The name of the service.  This is provided in args[0] of the service class __init__ method.
	// @pyparm object|callback||The Python function that performs as the control function.  This will be called with an integer status argument.
	if (!PyArg_ParseTuple(args, "OO", &nameOb, &obCallback))
		return NULL;
	if (!PyCallable_Check(obCallback)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be a callable object");
		return NULL;
	}
	WCHAR *szName;
	if (!PyWinObject_AsWCHAR(nameOb, &szName))
		return NULL;
	PY_SERVICE_TABLE_ENTRY *pe = FindPythonServiceEntry(szName);
	if (pe==NULL) {
		PyErr_SetString(PyExc_ValueError, "The service name is not hosted by this process");
		PyWinObject_FreeWCHAR(szName);
		return NULL;
	}
	Py_XDECREF(pe->obServiceCtrlHandler);
	pe->obServiceCtrlHandler = obCallback;
	Py_INCREF(obCallback);
	if (bServiceDebug) { // If debugging, get out now, and give None back.
		Py_INCREF(Py_None);
		return Py_None;
	}
	pe->sshStatusHandle = RegisterServiceCtrlHandlerEx(szName, service_ctrl_ex, pe);
	PyWinObject_FreeWCHAR(szName);
	PyObject *rc;
	if (pe->sshStatusHandle==0) {
		Py_DECREF(obCallback);
		obCallback = NULL;
		rc = PyWin_SetAPIError("RegisterServiceCtrlHandlerEx");
	} else {
		rc = PyInt_FromLong((long)pe->sshStatusHandle);
	}
	return rc;
	// @rdesc If the service manager is in debug mode, this returns None, indicating
	// there is no service control manager handle, otherwise the handle to the Win32 service manager.
}

// @pymethod |servicemanager|CoInitializeEx|Initialize OLE with additional options.
static PyObject *PyCoInitializeEx(PyObject *self, PyObject *args)
{
	DWORD flags;
	if (!PyArg_ParseTuple(args, "l:CoInitializeEx", &flags))
		return NULL;
	HRESULT hr = CoInitializeEx(NULL, flags);
	return PyInt_FromLong(hr);
}

// @pymethod |servicemanager|CoUninitialize|Unitialize OLE
static PyObject *PyCoUninitialize(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":CoUninitialize"))
		return NULL;
	CoUninitialize();
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod True/False|servicemanager|Debugging|Indicates if the service is running in debug mode.
static PyObject *PyDebugging(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Debugging"))
		return NULL;
	PyObject *rc = bServiceDebug ? Py_True : Py_False;
	Py_INCREF(rc);
	return rc;
}

// @pymethod int|servicemanager|PumpWaitingMessages|Pumps all waiting messages.
// @rdesc Returns 1 if a WM_QUIT message was received, else 0
static PyObject *PyPumpWaitingMessages(PyObject *self, PyObject *args)
{
    MSG msg;
	long result = 0;
	// Read all of the messages in this next loop, 
	// removing each message as we read it.
	Py_BEGIN_ALLOW_THREADS
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		// If it's a quit message, we're out of here.
		if (msg.message == WM_QUIT) {
			result = 1;
			break;
		}
		// Otherwise, dispatch the message.
		DispatchMessage(&msg); 
	} // End of PeekMessage while loop
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(result);
}

static PyObject *PyStartServiceCtrlDispatcher(PyObject *self)
{
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = PythonService_StartServiceCtrlDispatcher();
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("StartServiceCtrlDispatcher");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |servicemanager|Initialize|Initialize the module for hosting a service.  This is generally called automatically
static PyObject *PyServiceInitialize(PyObject *self, PyObject *args)
{
	PyObject *nameOb = Py_None, *fileOb = Py_None;
	// @pyparm <o PyUnicode>|eventSourceName|None|The event source name
	if (!PyArg_ParseTuple(args, "|OO", &nameOb, &fileOb))
		return NULL;
	TCHAR *name, *file;
	if (!PyWinObject_AsTCHAR(nameOb, &name, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(fileOb, &file, TRUE)) {
		PyWinObject_FreeTCHAR(name);
		return NULL;
	}
	PythonService_Initialize(name, file);
	PyWinObject_FreeTCHAR(name);
	PyWinObject_FreeTCHAR(file);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |servicemanager|Finalize|
static PyObject *PyServiceFinalize(PyObject *self)
{
	PythonService_Finalize();
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |servicemanager|PrepareToHostSingle|Prepare for hosting a single service in this EXE
static PyObject *PyPrepareToHostSingle(PyObject *self, PyObject *args)
{
	PyObject *klass = Py_None;
	// @pyparm object|klass|None|The Python class to host.  If not specified, the
	// service name is looked up in the registry and the specified class instantiated.
	if (!PyArg_ParseTuple(args, "|O", &klass))
		return NULL;
	BOOL ok = PythonService_PrepareToHostSingle(klass);
	if (!ok) {
		PyErr_SetString(servicemanager_startup_error, "PrepareToHostSingle failed!");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |servicemanager|PrepareToHostMultiple|Prepare for hosting a multiple services in this EXE
static PyObject *PyPrepareToHostMultiple(PyObject *self, PyObject *args)
{
	PyObject *klass, *obSvcName;
	// @pyparm string/unicode|service_name||The name of the service hosted by the class
	// @pyparm object|klass||The Python class to host.
	if (!PyArg_ParseTuple(args, "OO", &obSvcName, &klass))
		return NULL;
	TCHAR *name;
	if (!PyWinObject_AsTCHAR(obSvcName, &name, FALSE))
		return NULL;
	BOOL ok = PythonService_PrepareToHostMultiple(name, klass);
	if (!ok) {
		PyErr_SetString(servicemanager_startup_error, "PrepareToHostMultiple failed!");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

// @module servicemanager|A module that interfaces with the Windows Service Control Manager.  While this
// module can be imported by regular Python programs, it is only useful when used by a Python program
// hosting a service - and even then is generally used automatically by the Python Service framework.
// See the pipeTestService sample for an example of using this module.
// <nl>The module <o win32service> and <o win32serviceutil> provide other facilities for controlling
// and managing services.
static struct PyMethodDef servicemanager_functions[] = {
	{"CoInitializeEx", PyCoInitializeEx, 1}, // @pymeth CoInitializeEx|
	{"CoUninitialize", PyCoUninitialize, 1}, // @pymeth CoUninitialize|
	{"RegisterServiceCtrlHandler", PyRegisterServiceCtrlHandler, 1}, // @pymeth RegisterServiceCtrlHandler|Registers a function to retrieve service control notification messages.
	{"LogMsg",                     PyLogMsg, 1},	    // @pymeth LogMsg|Write an specific message to the log.
	{"LogInfoMsg",                 PyLogInfoMsg, 1},	// @pymeth LogInfoMsg|Write an informational message to the log.
	{"LogErrorMsg",                PyLogErrorMsg, 1},	// @pymeth LogErrorMsg|Write an error message to the log.
	{"LogWarningMsg",              PyLogWarningMsg, 1}, // @pymeth LogWarningMsg|Logs a generic warning message to the event log
	{"PumpWaitingMessages",        PyPumpWaitingMessages, 1},  // @pymeth PumpWaitingMessages|Pumps waiting window messages for the service.
	{"Debugging",                  PyDebugging, 1},  // @pymeth Debugging|Indicates if the service is running in debug mode.
	{"StartServiceCtrlDispatcher", (PyCFunction)PyStartServiceCtrlDispatcher, METH_NOARGS}, // @pymeth StartServiceCtrlDispatcher|Starts the service by calling the win32 StartServiceCtrlDispatcher function.
	{"Initialize",                 PyServiceInitialize, 1}, // @pymeth Initialize|
	{"Finalize",                   (PyCFunction)PyServiceFinalize, METH_NOARGS}, // @pymeth Finalize|
	{"PrepareToHostSingle",        PyPrepareToHostSingle, 1}, // @pymeth  PrepareToHostSingle|
	{"PrepareToHostMultiple",      PyPrepareToHostMultiple, 1}, // @pymeth  PrepareToHostMultiple|
	{NULL}
};

static int AddConstant(PyObject *dict, const char *key, long value)
{
	PyObject *oval = PyInt_FromLong(value);
	if (!oval)
	{
		return 1;
	}
	int rc = PyDict_SetItemString(dict, (char*)key, oval);
	Py_DECREF(oval);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)

extern "C" __declspec(dllexport) void
initservicemanager(void)
{
  PyObject *dict, *module;
  module = Py_InitModule("servicemanager", servicemanager_functions);
  if (!module) /* Eeek - some serious error! */
    return;
  dict = PyModule_GetDict(module);
  if (!dict) return; /* Another serious error!*/

  servicemanager_startup_error = PyErr_NewException("servicemanager.startup_error", NULL, NULL);
  if (servicemanager_startup_error == NULL) return;

  PyDict_SetItemString(dict, "startup_error", servicemanager_startup_error);

  ADD_CONSTANT(COINIT_MULTITHREADED);
  ADD_CONSTANT(COINIT_APARTMENTTHREADED);
  ADD_CONSTANT(COINIT_DISABLE_OLE1DDE);
  ADD_CONSTANT(COINIT_SPEED_OVER_MEMORY);

  ADD_CONSTANT(PYS_SERVICE_STARTING);
  ADD_CONSTANT(PYS_SERVICE_STARTED);
  ADD_CONSTANT(PYS_SERVICE_STOPPING);
  ADD_CONSTANT(PYS_SERVICE_STOPPED);

  ADD_CONSTANT(EVENTLOG_ERROR_TYPE);
  ADD_CONSTANT(EVENTLOG_INFORMATION_TYPE);
  ADD_CONSTANT(EVENTLOG_WARNING_TYPE);
  ADD_CONSTANT(EVENTLOG_AUDIT_SUCCESS);
  ADD_CONSTANT(EVENTLOG_AUDIT_FAILURE);
  PyWinGlobals_Ensure();
}

// Couple of helpers for the service manager
static void PyService_InitPython()
{
	// XXX - this assumes GIL held, so no races possible
	static BOOL have_init = FALSE;
	if (have_init)
		return;
	have_init = TRUE;
	// Often for a service, __argv[0] will be just "ExeName", rather
	// than "c:\path\to\ExeName.exe"
	// This, however, shouldnt be a problem, as Python itself
	// knows how to get the .EXE name when it needs.
	Py_SetProgramName(__argv[0]);
#ifdef BUILD_FREEZE
	PyInitFrozenExtensions();
#endif
	Py_Initialize();
#ifdef BUILD_FREEZE
	PyWinFreeze_ExeInit();
#endif
	// Ensure we are set for threading.
	PyEval_InitThreads();
	PySys_SetArgv(__argc, __argv);

	initservicemanager();
}

/*************************************************************************
 *
 *
 * Our Python Service "public API" - allows clients to use our DLL
 * in almost any possible way
 *
 *
 *************************************************************************/

//  FUNCTION: PythonService_Initialize
//
//  PURPOSE: Initialize the DLL
//
//  PARAMETERS:
//    evtsrc_name - The event source name, as it appears in the event log (and
//                  as used to obtain the eventsource handle.
//    evtsrc_file - The name of the file registered with the event viewer for this
//                  source.
//   Both params can be NULL, meaning a default source name is used, and this
//   DLL as the file.  If either param is non-NULL, both must be non-NULL.
//
//  RETURN VALUE:
//    TRUE if we registered OK.
BOOL PythonService_Initialize( const TCHAR *evtsrc_name, const TCHAR *evtsrc_file)
{
	TCHAR evtsrc_file_buf[MAX_PATH+_MAX_FNAME];
	if (!evtsrc_name && !evtsrc_file) {
		// g_szEventSourceName keeps default of "Python Service"
		GetModuleFileName(g_hdll, evtsrc_file_buf,
		                  sizeof evtsrc_file_buf/sizeof TCHAR);
		evtsrc_file = evtsrc_file_buf;
	} else {
		if (!evtsrc_name || !evtsrc_file)
			return FALSE;
		_tcsncpy(g_szEventSourceName, evtsrc_name,
				 sizeof g_szEventSourceName/sizeof TCHAR);
	}
	// And register the event source with the event log.
	HKEY hkey;
	TCHAR keyName[MAX_PATH];
	_tcscpy(keyName, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"));
	_tcscat(keyName, g_szEventSourceName );
	// ignore all failures when setting up - for whatever reason it fails,
	// we are probably still better off calling ReportEvent than
	// not calling due to some other failure here.
	BOOL rc = FALSE;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
		               keyName, 
		               0, 
		               NULL, 
		               REG_OPTION_NON_VOLATILE, 
		               KEY_WRITE, NULL, 
		               &hkey, 
		               NULL) == ERROR_SUCCESS) {
		RegSetValueEx(hkey, TEXT("EventMessageFile"), 0, REG_SZ, 
			          (const BYTE *)evtsrc_file, (_tcslen(evtsrc_file)+1)*sizeof(TCHAR));
		DWORD types = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
		RegSetValueEx(hkey, TEXT("TypesSupported"), 0, REG_DWORD, 
			          (const BYTE *)&types, sizeof(types));
		RegCloseKey(hkey);
		rc = TRUE;
	}
	return rc;
}

//  FUNCTION: PythonService_Finalize
//
//  PURPOSE: Finalize our service hosting framework
void PythonService_Finalize()
{
	UINT i;
	for (i=0;i<MAX_SERVICES;i++) {
		if (DispatchTable[i].lpServiceName==NULL)
			break;
		Py_XDECREF(PythonServiceTable[i].klass);
		PythonServiceTable[i].klass = NULL;
	}
}

//  FUNCTION: PythonService_PrepareToHostSingle
//
//  PURPOSE: Prepare this EXE for hosting a single service.  The service name
//           need not be given - the service named passed by Windows as the
//           service starts is used.
//
//  PARAMETERS:
//    klass - The Python class which implements this service.  Note this may be NULL,
//            which means we will lookup and instantiate the class using the service
//            name that Windows starts us with
//
//  RETURN VALUE:
//    FALSE if we have exceeded the maximum number of services in this executable, 
//    or if this service name has already been prepared.
//
//  COMMENTS:
//    Theoretically could be called multiple times, once for each service hosted
//    by this process - however, some code will need tweaking to get it working
//    correctly for more than one service.
BOOL PythonService_PrepareToHostSingle(PyObject *klass)
{
	if (g_serviceProcessFlags==0)
		g_serviceProcessFlags = SERVICE_WIN32_OWN_PROCESS;
	else if (g_serviceProcessFlags != SERVICE_WIN32_OWN_PROCESS)
		return FALSE;
	DispatchTable[0].lpServiceName = _tcsdup(_T(""));
	DispatchTable[0].lpServiceProc = service_main;
	PythonServiceTable[0].klass = klass;
	Py_XINCREF(klass);
	PythonServiceTable[0].sshStatusHandle = 0;
	PythonServiceTable[0].obServiceCtrlHandler = NULL;
	return TRUE;
}


//  FUNCTION: PythonService_PrepareToHostMultiple
//
//  PURPOSE: Prepare this EXE for hosting the nominated Python service.
//
//  PARAMETERS:
//    service_name - name of the service.
//    klass - The Python class which implements this service.
//
//  RETURN VALUE:
//    FALSE if we have exceeded the maximum number of services in this executable, 
//    if the exe is already setup to host an "own process" service, or if this 
//    service name has already been prepared.
//
//  COMMENTS:
//    Should be called multiple times, once for each service hosted
//    by this process - however, some code will need tweaking to get it working
//    correctly for more than one service.
BOOL PythonService_PrepareToHostMultiple(const TCHAR *service_name, PyObject *klass)
{
	if (g_serviceProcessFlags==0)
		g_serviceProcessFlags = SERVICE_WIN32_SHARE_PROCESS;
	else if (g_serviceProcessFlags != SERVICE_WIN32_SHARE_PROCESS)
		return FALSE;
	UINT i;
	for (i=0;i<MAX_SERVICES;i++) {
		if (DispatchTable[i].lpServiceName==NULL)
			break;
		if (_tcscmp(service_name, DispatchTable[i].lpServiceName)==0)
			return FALSE;
	}
	if (i>=MAX_SERVICES)
		return FALSE;

	DispatchTable[i].lpServiceName = _tcsdup(service_name);
	DispatchTable[i].lpServiceProc = service_main;

	PythonServiceTable[i].klass = klass;
	Py_INCREF(klass);
	PythonServiceTable[i].sshStatusHandle = 0;
	PythonServiceTable[i].obServiceCtrlHandler = NULL;
	return TRUE;
}

//  FUNCTION: PythonService_StartServiceCtrlDispatcher
//
//  PURPOSE: Calls the Windows StartServiceCtrlDispatcher with
//           the DispatchTable setup by previous calls to PrepareToHost 
//           functions.
//
//  RETURN VALUE:
//    As per the API.  Call GetLastError() to work out why.
BOOL PythonService_StartServiceCtrlDispatcher()
{
    return StartServiceCtrlDispatcher( DispatchTable);
}

/*************************************************************************
 *
 *
 * Service Implementation - the main service entry point, and our magic
 * of delegating to Python.
 *
 *
 *************************************************************************/
// Find a previously registered SERVICE_TABLE_ENTRY for the named service,
// or NULL if not found.
PY_SERVICE_TABLE_ENTRY *FindPythonServiceEntry(LPCTSTR svcName)
{
	PY_SERVICE_TABLE_ENTRY *ppy = PythonServiceTable;
	if (g_serviceProcessFlags==SERVICE_WIN32_OWN_PROCESS)
		return ppy;
	SERVICE_TABLE_ENTRY *ps = DispatchTable;
	while (ps->lpServiceName) {
		if (_tcscmp(ps->lpServiceName, svcName)==0)
			break;
		ppy++;
		ps++;
	}
	if (ps->lpServiceName)
		return ppy;
	return NULL;
}

//
//  FUNCTION: service_main
//
//  PURPOSE: To perform actual initialization and execution of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine is called by the Service Control Manager.  It loads 
//    the "SvcRun" function from the class instance and calls it.
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	PyObject *instance = NULL;
	PyObject *start = NULL;

	if (bServiceDebug)
		SetConsoleCtrlHandler( DebugControlHandler, TRUE );

	CEnterLeavePython _celp;
	PY_SERVICE_TABLE_ENTRY *pe;
	if (g_serviceProcessFlags == SERVICE_WIN32_OWN_PROCESS) {
		pe = PythonServiceTable;
		if (!pe->klass) {
			char svcInitBuf[256];
			LocatePythonServiceClassString(lpszArgv[0], svcInitBuf, sizeof(svcInitBuf));
			pe->klass = LoadPythonServiceClass(svcInitBuf);
		}
	} else
		PY_SERVICE_TABLE_ENTRY *pe = FindPythonServiceEntry(lpszArgv[0]);
	if (!pe) {
		LPTSTR  lpszStrings[] = {lpszArgv[0], NULL};
		ReportError(E_PYS_NO_SERVICE, (LPCTSTR *)lpszStrings);
		goto cleanup;
	}
	assert(pe->sshStatusHandle==0); // should have no scm handle yet.
	instance = LoadPythonServiceInstance(pe->klass, dwArgc, lpszArgv);
	// If Python has not yet registered the service control handler, then
	// we are in serious trouble - it is likely the service will enter a 
	// zombie state, where it wont do anything, but you can not start 
	// another.  Therefore, we still create register the handler, thereby 
	// getting a handle, so we can immediately tell Windows the service 
	// is rooted (that is a technical term!)
	if (!bServiceDebug && pe->sshStatusHandle==0) {
		// If Python seemed to init OK, but hasnt done what we expect,
		// report an error, as we are gunna fail!
		if (instance) 
			ReportPythonError(E_PYS_NOT_CONTROL_HANDLER);
		if (!bServiceDebug)
			// Note - we upgraded this to the win2k only "Ex" function.
			// If this was a problem for someone, you could do a LoadLibrary
			// RegisterServiceCtrlHandlerEx(), and if that fails, fallback to
			// RegisterServiceCtrlHandler(), manually passing the first entry
			// of our PY_SERVICE_TABLE_ENTRY table.  You would also need to
			// insist that only 1 service is being hosted.
			pe->sshStatusHandle = RegisterServiceCtrlHandlerEx( lpszArgv[0], service_ctrl_ex, pe);
	}
	// No instance - we can't start.
	if (!instance) {
		if (pe->sshStatusHandle) {
			SetServiceStatus( pe->sshStatusHandle, &neverStartedStatus );
			pe->sshStatusHandle = 0; // reset so we don't attempt to set 'stopped'
		}
		goto cleanup;
	}
	if (!bServiceDebug)
		if (!SetServiceStatus( pe->sshStatusHandle, &startingStatus ))
			ReportAPIError(PYS_E_API_CANT_SET_PENDING);
	start = PyObject_GetAttrString(instance, "SvcRun");
	if (start==NULL)
		ReportPythonError(E_PYS_NO_RUN_METHOD);
	else {
		// Call the Python service entry point - when this returns, the
		// service has stopped!
		PyObject *result = PyObject_CallObject(start, NULL);
		if (result==NULL)
			ReportPythonError(E_PYS_START_FAILED);
		else
			Py_DECREF(result);
	}
	// We are all done.
cleanup:
	// try to report the stopped status to the service control manager.
	Py_XDECREF(start);
	Py_XDECREF(instance);
	if (pe && pe->sshStatusHandle) { // Wont be true if debugging.
		if (!SetServiceStatus( pe->sshStatusHandle, &stoppedStatus ))
			ReportAPIError(PYS_E_API_CANT_SET_STOPPED);
	}
	return;
}

// The service control handler - receives async notifications from the
// SCM, and delegates to the Python instance.
DWORD WINAPI service_ctrl_ex(
	  DWORD dwCtrlCode,     // requested control code
	  DWORD dwEventType,   // event type
	  LPVOID lpEventData,  // event data
	  LPVOID lpContext     // user-defined context data
	  )
{
	PY_SERVICE_TABLE_ENTRY *pse = (PY_SERVICE_TABLE_ENTRY *)lpContext;
	if (pse->obServiceCtrlHandler==NULL) { // Python is in error.
		if (!bServiceDebug)
			SetServiceStatus( pse->sshStatusHandle, &errorStatus );
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
	// Ensure we have a context for our thread.
	DWORD dwResult;
	CEnterLeavePython celp;
	PyObject *args = Py_BuildValue("(l)", dwCtrlCode);
	PyObject *result = PyObject_CallObject(pse->obServiceCtrlHandler, args);
	Py_XDECREF(args);
	if (result==NULL) {
		ReportPythonError(PYS_E_SERVICE_CONTROL_FAILED);
		dwResult = ERROR_CALL_NOT_IMPLEMENTED; // correct code?
	} else if (PyInt_Check(result)||PyLong_Check(result))
		dwResult = PyInt_AsLong(result);
	else
		dwResult = NOERROR;

	Py_XDECREF(result);
	return dwResult;
}


// When debugging, a console event handler that simulates a service 
// stop control.
BOOL WINAPI DebugControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			{
            _tprintf(TEXT("Stopping debug service.\n"));
			// simulate a stop even to each service
			PY_SERVICE_TABLE_ENTRY *ppy = PythonServiceTable;
			SERVICE_TABLE_ENTRY *ps = DispatchTable;
			while (ps->lpServiceName) {
	            service_ctrl_ex(SERVICE_CONTROL_STOP, 0, NULL, ppy);
				ppy++;
				ps++;
			}
            return TRUE;
            break;
			}

    }
    return FALSE;
}

/*************************************************************************
 *
 *
 * Generic Service Host implementation - handles command-line args,
 * uses the registry to work out what Python classes to load, etc.
 * This section could be split into the EXE.
 *
 *
 *************************************************************************/
//
//  FUNCTION: PythonService_main
//
//  PURPOSE: entrypoint for our generic PythonService host 
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    main() either performs the command line task, or
//    call StartServiceCtrlDispatcher to register the
//    main service thread.  When the this call returns,
//    the service has stopped, so exit.
//
int PythonService_main(int argc, char **argv)
{
	// Note that we don't know the service name we are hosting yet!
	// If only one service is hosted, then it is no real problem - our dispatch
	// table has a single entry, and this must be our service (and indeed
	// Windows doesn't care - it ignores the service name in the dispatch table
	// if SERVICE_WIN32_OWN_PROCESS is set.
	// However, if we want to host multiple services, we have a problem.
	// For now, the solution is to not support multiple services via this
	// generic PythonService.exe.  However, py2exe etc is free to register
	// multiple services (and we don't care how it find out the service names)
	// Later, we could add support to the service registration code so that
	// the service names are always passed on the command-line (-services=)
	// (This is also the reason "-debug" requires the service name on the
	// command-line.)
	int temp;
	LPTSTR *targv;

#ifdef UNICODE
	targv = CommandLineToArgvW(GetCommandLineW(), &temp);
#else
	targv = argv;
#endif
	
    if ( (argc > 1) &&
         ((*argv[1] == '-') || (*argv[1] == '/')) )
    {
#ifndef BUILD_FREEZE
        if ( _stricmp( "register", argv[1]+1 ) == 0 ||
             _stricmp( "install", argv[1]+1 ) == 0 )
        {
	        // Get out of here.
			return RegisterPythonServiceExe() ? 0 : 1;
        }
#endif
        if ( _stricmp( "debug", argv[1]+1 ) == 0 ) {
			/* Debugging the service.  If this EXE has a service name
			   embedded in it, use it, otherwise insist one is passed on the
			   command line
			   (NOTE: Embedding a resource to specify the service name is
			   deprecated)
			*/
			TCHAR svcNameBuf[256];
			TCHAR *svcName;
			int argOffset = 1;
			if (LoadString(GetModuleHandle(NULL), RESOURCE_SERVICE_NAME, svcNameBuf, sizeof(svcNameBuf)/sizeof(TCHAR))>1) {
				svcName = svcNameBuf;
			} else {
				if (argc<3) {
		    		printf("-debug requires a service name");
					return 1;
				}
				svcName = targv[2];
				argOffset = 2;
			}
			bServiceDebug = TRUE;
			_tprintf(_T("Debugging service %s - press Ctrl+C to stop.\n"), svcName);
			PythonService_Initialize(NULL, NULL);
			PythonService_PrepareToHostSingle(NULL);
			service_main(argc-argOffset, targv+argOffset);
		return 0; // gotta assume OK...
		}
	}
	PythonService_Initialize(NULL, NULL);
	PythonService_PrepareToHostSingle(NULL);

	if (!PythonService_StartServiceCtrlDispatcher()) {
		DWORD errCode = GetLastError();
		if (errCode==ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
			// We are not being run by the SCM - print a debug message.
			printf("%s - Python Service Manager\n", argv[0]);
			printf("Options:\n");
#ifndef BUILD_FREEZE
			printf(" -register - register the EXE - this should generally not be necessary.\n");
#endif
		    printf(" -debug servicename [parms] - debug the Python service.\n");
		    printf("\nNOTE: You do not start the service using this program - start the\n");
		    printf("service using Control Panel, or 'net start service_name'\n");
		} else {
			// Some other nasty error - log it.
			ReportAPIError(PYS_E_API_CANT_START_SERVICE, errCode);
			printf("Could not start the service - error %d\n", errCode);
			// Just incase the error was caused by this EXE not being registered
#ifndef BUILD_FREEZE
			RegisterPythonServiceExe();
#endif
		}
		return 2;
	}
	// life is good!
	return 0;
}


// Given the string in form [path\]module.ClassName, return
// an instance of the class
PyObject *LoadPythonServiceClass(char *svcInitString)
{
	char valueBuf[512];
	// Initialize Python
	PyService_InitPython();
	strncpy(valueBuf, svcInitString, sizeof(valueBuf));
	// Find the last "\\"
	char *sep = strrchr(valueBuf, '\\');
	char *fname;
	if (sep) {
		*sep = '\0';
		fname = sep+1;
		// Stick the Path on the Python sys.path.
		PyObject *obPath = PySys_GetObject("path");
		if (obPath==NULL) {
				ReportPythonError(PYS_E_NO_SYS_PATH);
				return FALSE;
		}
		PyObject *obNew = PyString_FromString(valueBuf);
		if (obNew==NULL) {
			ReportPythonError(PYS_E_NO_MEMORY_FOR_SYS_PATH);
			return FALSE;
		}
		PyList_Append(obPath, obNew);
		Py_DECREF(obNew);
	} else {
		fname = valueBuf;
	}
	// Find the last "." in the name, and assume it is a module name.
	char *classNamePos = strrchr(fname, '.');
	if (classNamePos==NULL) {
		ReportError(PYS_E_CANT_LOCATE_MODULE_NAME);
		return NULL;
	}
	PyObject *module;
	// XXX - does this work for packages?  I fear that like Python,
	// PyImport_ImportModule("foo.bar") will return 'foo', not bar.
	*classNamePos++ = '\0';
	module = PyImport_ImportModule(fname);
	if (module==NULL) {
		ReportPythonError(E_PYS_NO_MODULE);
		return NULL;
	}
	PyObject *pyclass = PyObject_GetAttrString(module, classNamePos);
	Py_DECREF(module);
	if (pyclass==NULL) {
		ReportPythonError(E_PYS_NO_CLASS);
		return NULL;
	}
	return pyclass;
}

// Given a Python class and an "argv" array, instantiate our
// instance.
PyObject *LoadPythonServiceInstance(	PyObject *pyclass,
										DWORD dwArgc,
										LPTSTR *lpszArgv )
{
	if (pyclass==NULL) {
		ReportPythonError(PYS_E_BAD_CLASS);
		return NULL;
	}
	PyObject *args = PyTuple_New(dwArgc);
	if (args==NULL) {
		Py_DECREF(pyclass);
		ReportPythonError(PYS_E_NO_MEMORY_FOR_ARGS);
		return NULL;
	}
	for (DWORD i=0;i<dwArgc;i++) {
		PyObject *arg = PyWinObject_FromWCHAR(lpszArgv[i]);
		if (arg==NULL) {
			Py_DECREF(args);
			Py_DECREF(pyclass);
			ReportPythonError(PYS_E_BAD_ARGS);
			return NULL;
		}
		PyTuple_SET_ITEM(args, i, arg);
	}
	PyObject *realArgs = PyTuple_New(1);
	PyTuple_SET_ITEM(realArgs, 0, args);
	PyObject *result = PyObject_CallObject(pyclass, realArgs);
	if (result==NULL) {
		BOOL bHandledError = PyErr_ExceptionMatches(servicemanager_startup_error);
		if (bHandledError)
			PyErr_Clear();
		else
			ReportPythonError(PYS_E_BAD_CLASS);
	}
	return result;
}

BOOL LocatePythonServiceClassString( TCHAR *svcName, char *buf, int cchBuf)
{
	char keyName[1024];

	// If not error loading, and not an empty string
	// (NOTE: Embedding a resource to specify the service name is
	// deprecated)
	if (LoadStringA(GetModuleHandle(NULL), RESOURCE_SERVICE_NAME, buf, cchBuf)>1)
		// Get out of here now!
		return TRUE;

	HKEY key = NULL;
	BOOL ok = TRUE;
	wsprintfA(keyName, "System\\CurrentControlSet\\Services\\%S\\PythonClass", svcName);
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, keyName, &key) != ERROR_SUCCESS) {
		ReportAPIError(PYS_E_API_CANT_LOCATE_PYTHON_CLASS);
		return FALSE;
	}
	DWORD dataType;
	DWORD valueBufSize = cchBuf;
	if ((RegQueryValueExA(key, "", 0, &dataType, (LPBYTE)buf, &valueBufSize)!=ERROR_SUCCESS) ||
		(dataType != REG_SZ)) {
		ReportAPIError(PYS_E_API_CANT_LOCATE_PYTHON_CLASS);
		ok = FALSE;
	}
/***
	// See if the optimized flag is turned on.
	BOOL bOptimize;
	DWORD readSize = sizeof(bOptimize);
	if (key) {
		if ((RegQueryValueExA(key, "Optimize", 0, &dataType, (LPBYTE)&bOptimize, &readSize)!=ERROR_SUCCESS) || 
			(dataType != REG_DWORD)) {
			bOptimize = FALSE;
		}
	}

***/
	if (key)
		RegCloseKey(key);
	return ok;
}

// Register the EXE.
// This writes an entry to the Python registry and also
// to the EventLog so I can stick in messages.
static BOOL RegisterPythonServiceExe(void)
{
	printf("Registering the Python Service Manager...\n");
	const int fnameBufSize = MAX_PATH + 1;
	TCHAR fnameBuf[fnameBufSize];
	if (GetModuleFileName( NULL, fnameBuf, fnameBufSize)==0) {
		printf("Registration failed due to GetModuleFileName() failing (error %d)\n", GetLastError());
		return FALSE;
	}
	assert (Py_IsInitialized());
	CEnterLeavePython _celp;
	// Register this specific EXE against this specific DLL version
	PyObject *obVerString = PySys_GetObject("winver");
	if (obVerString==NULL || !PyString_Check(obVerString)) {
		Py_XDECREF(obVerString);
		printf("Registration failed as sys.winver is not available or not a string\n");
		return FALSE;
	}
	char *szVerString = PyString_AsString(obVerString);
	Py_DECREF(obVerString);
	// note wsprintf allows %hs to be "char *" even when UNICODE!
	TCHAR keyBuf[256];
	wsprintf(keyBuf, _T("Software\\Python\\PythonService\\%hs"), szVerString);
	DWORD rc;
	if ((rc=RegSetValue(HKEY_LOCAL_MACHINE,
	                keyBuf, REG_SZ, 
					fnameBuf, _tcslen(fnameBuf)))!=ERROR_SUCCESS) {
		printf("Registration failed due to RegSetValue() of service EXE - error %d\n", rc);
		return FALSE;
	}
	// don't bother registering in the event log - do it when we write a log entry.
	return TRUE;
}

#endif // PYSERVICE_BUILD_DLL

// Code that exists in both EXE and DLL - mainly error handling code.
/*************************************************************************
 *
 *
 * Error and Event Log related functions.
 *
 *
 *************************************************************************/
static void ReportAPIError(DWORD msgCode, DWORD errCode /*= 0*/)
{
	if (errCode==0)
		errCode = GetLastError();

	const int bufSize = 512;
	TCHAR buf[bufSize];
	BOOL bHaveMessage = (::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, 0, buf, bufSize, NULL )>0);
	if (!bHaveMessage)
		_tcscpy(buf,TEXT("No error message is available"));
	/* strip trailing cr/lf */
	int end = _tcslen(buf)-1;
	if (end>1 && (buf[end-1]==L'\n' || buf[end-1]==L'\r'))
		buf[end-1] = L'\0';
	else
		if (end>0 && (buf[end]==L'\n' || buf[end]==L'\r'))
			buf[end]=L'\0';

	TCHAR cvtBuf[20];
	_stprintf(cvtBuf, L"%d", errCode);
    LPTSTR  lpszStrings[] = {cvtBuf, buf, L'\0'};
    ReportError(msgCode, (LPCTSTR *)lpszStrings);
}

#define GPEM_ERROR(what) {errorMsg = "<Error getting traceback - " ## what ## ">";goto done;}
static char *GetPythonTraceback(PyObject *exc_tb)
{
	char *result = NULL;
	char *errorMsg = NULL;
	PyObject *modStringIO = NULL;
	PyObject *modTB = NULL;
	PyObject *obFuncStringIO = NULL;
	PyObject *obStringIO = NULL;
	PyObject *obFuncTB = NULL;
	PyObject *argsTB = NULL;
	PyObject *obResult = NULL;

	/* Import the modules we need - cStringIO and traceback */
	modStringIO = PyImport_ImportModule("cStringIO");
	if (modStringIO==NULL) GPEM_ERROR("cant import cStringIO");
	modTB = PyImport_ImportModule("traceback");
	if (modTB==NULL) GPEM_ERROR("cant import traceback");

	/* Construct a cStringIO object */
	obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find cStringIO.StringIO");
	obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
	if (obStringIO==NULL) GPEM_ERROR("cStringIO.StringIO() failed");

	/* Get the traceback.print_exception function, and call it. */
	obFuncTB = PyObject_GetAttrString(modTB, "print_tb");
	if (obFuncTB==NULL) GPEM_ERROR("cant find traceback.print_tb");
	argsTB = Py_BuildValue("OOO", 
			exc_tb  ? exc_tb  : Py_None,
			Py_None, 
			obStringIO);
	if (argsTB==NULL) GPEM_ERROR("cant make print_tb arguments");

	obResult = PyObject_CallObject(obFuncTB, argsTB);
	if (obResult==NULL) GPEM_ERROR("traceback.print_tb() failed");

	/* Now call the getvalue() method in the StringIO instance */
	Py_DECREF(obFuncStringIO);
	obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find getvalue function");
	Py_DECREF(obResult);
	obResult = PyObject_CallObject(obFuncStringIO, NULL);
	if (obResult==NULL) GPEM_ERROR("getvalue() failed.");

	/* And it should be a string all ready to go - duplicate it. */
	if (!PyString_Check(obResult))
		GPEM_ERROR("getvalue() did not return a string");
	result = strdup(PyString_AsString(obResult));
done:
	if (result==NULL && errorMsg != NULL)
		result = strdup(errorMsg);
	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obFuncStringIO);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obFuncTB);
	Py_XDECREF(argsTB);
	Py_XDECREF(obResult);
	return result;
}

static void ReportPythonError(DWORD code)
{
	if (PyErr_Occurred()) {
		LPTSTR inserts[4];
		inserts[3] = NULL; // terminate array
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);
		WCHAR *szTracebackUse = L"<No memory!>"; // default.
		WCHAR *szTraceback = NULL; // to be freed.
		char *szmbTraceback = GetPythonTraceback(traceback);
		if (szmbTraceback) {
			int tb_len = strlen(szmbTraceback) + 1;
			szTraceback = (TCHAR *)malloc(sizeof(WCHAR) * tb_len);
			if (szTraceback) {
				szTracebackUse = szTraceback;
				MultiByteToWideChar(CP_ACP, 0, szmbTraceback, tb_len, szTraceback, tb_len);
				// trim crud from the end.
				if (tb_len>2) szTracebackUse[tb_len-2] = L'\0';
			}
			free(szmbTraceback);
		}
		inserts[0] = szTracebackUse;
		PyObject *obStr = PyObject_Str(type);
		PyWinObject_AsWCHAR(obStr, inserts+1);
		Py_XDECREF(obStr);
		obStr = PyObject_Str(value);
		PyWinObject_AsWCHAR(PyObject_Str(obStr), inserts+2);
		Py_XDECREF(obStr);
	    ReportError(code, (LPCTSTR *)inserts);
		if (szTraceback) free(szTraceback);
		PyWinObject_FreeWCHAR(inserts[1]);
		PyWinObject_FreeWCHAR(inserts[2]);
		if (bServiceDebug) { // If debugging, restore for traceback print,
			PyErr_Restore(type, value, traceback);
		} else {	// free em up.
			Py_XDECREF(type);
			Py_XDECREF(value);
			Py_XDECREF(traceback);
		}
	} else {
		LPCTSTR inserts[] = {L"<No Python Error!>", L"", L"", NULL};
		ReportError(code, inserts);
	}
	PyErr_Clear();
}

static BOOL ReportError(DWORD code, LPCTSTR *inserts, WORD errorType /* = EVENTLOG_ERROR_TYPE*/)
{
	WORD numInserts = 0;
	while (inserts && inserts[numInserts]!=NULL)
		numInserts++;
		
    HANDLE  hEventSource;
    // Use event logging to log the error.
	//

    if (bServiceDebug) {
		TCHAR *szType;
		switch (errorType) {
			case EVENTLOG_WARNING_TYPE:
				szType = _T("Warning");
				break;
			case EVENTLOG_INFORMATION_TYPE:
				szType = _T("Info");
				break;
			case EVENTLOG_ERROR_TYPE:
				szType = _T("Error");
				break;
			default:
				szType = _T("Message");
				break;
		}
    	LPTSTR buffer;
    	// Get the message text, and just print it.
    	if (FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    			g_hdll, code, 0, (LPTSTR)&buffer, 0, (va_list *)inserts)==0) {
    		_tprintf(_T("%s 0x%X - No message available\nMessage inserts were"), szType, code);
    		for (int i=0;i<numInserts;i++)
    			_tprintf(_T("'%s',"), inserts[i]);
    	} else {
    		_tprintf(_T("%s 0x%X - %s"), szType, code, buffer);
    		LocalFree(buffer);
    	}
		return TRUE;
	} else {
		hEventSource = RegisterEventSource(NULL, g_szEventSourceName);
		if (hEventSource==NULL)
			return FALSE;

        BOOL rc = ReportEvent(hEventSource, // handle of event source
            errorType,  // event type
            0,                    // event category
            code,                 // event ID
            NULL,                 // current user's SID
            numInserts,           // strings in lpszStrings
            0,                    // no bytes of raw data
            inserts,          // array of error strings
            NULL);                // no raw data

        (VOID) DeregisterEventSource(hEventSource);
		return rc;
    }
}


/*************************************************************************
 *
 *
 * Entry points
 *
 *
 *************************************************************************/
#ifdef PYSERVICE_BUILD_DLL
extern "C" __declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if ( dwReason == DLL_PROCESS_ATTACH )
		g_hdll = hInstance;
	return TRUE;
}

#else // PYSERVICE_BUILD_DLL
// Our EXE entry point.

int main(int argc, char **argv)
{
	PyObject *module, *f;
	PyThreadState *threadState;
	HMODULE hmod;
	FARPROC proc;
	Py_Initialize();
	PyEval_InitThreads();
	module = PyImport_ImportModule("servicemanager");
	if (!module) goto failed;
	f = PyObject_GetAttrString(module, "__file__");
	Py_DECREF(module);
	if (!f) goto failed;
	if (!PyString_Check(f)) {
		PyErr_SetString(PyExc_TypeError, "servicemanager.__file__ is not a string!");
		goto failed;
	}
	// now get the handle to the DLL, and call the main function.
	hmod = GetModuleHandleA(PyString_AsString(f));
	Py_DECREF(f);
	if (!hmod) {
		PyErr_Format(PyExc_RuntimeError, "servicemanager.__file__ could not be loaded - win32 error code is %d", GetLastError());
		goto failed;
	}
	proc = GetProcAddress(hmod, "PythonService_main");
	if (!proc) {
		PyErr_Format(PyExc_RuntimeError, "servicemanager.__file__ does not contain PythonService_main - win32 error code is %d", GetLastError());
		goto failed;
	}
	// A little thread-state dance, as our module will attempt to acquire it.
	threadState = PyThreadState_Swap(NULL);
	PyThreadState_Swap(threadState);
	PyEval_ReleaseThread(threadState);

	typedef int (* FNPythonService_main)(int argc, char **argv);
	return ((FNPythonService_main)proc)(argc, argv);
failed:
	fprintf(stderr, "PythonService was unable to locate the service manager. "
	                "Please see the event log for details\n");
	ReportPythonError(PYS_E_NO_SERVICEMANAGER);
	return 1;
}
#endif

