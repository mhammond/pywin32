//  MODULE:   PythonService.exe
//
//  PURPOSE:  An executable that hosts Python services.
//
// @doc

// We use DCOM constants and possible CoInitializeEx.
#define _WIN32_DCOM

#include "windows.h"
#include "objbase.h"
#include "Python.h"
#undef main
#include "tchar.h"

#include "PythonServiceMessages.h"

#include "PyWinTypes.h"


#ifdef BUILD_FREEZE
extern "C" void PyWinFreeze_ExeInit();
extern "C" void PyWinFreeze_ExeTerm();
extern "C" int PyInitFrozenExtensions();
#endif

SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle = 0;
DWORD                   dwErr = 0;
BOOL                    bServiceDebug = FALSE;
TCHAR                   szErr[256];

#define RESOURCE_SERVICE_NAME 1016 // resource ID in the EXE of the service name

TCHAR g_szEventSourceName[MAX_PATH] = _T("PythonServiceManager");

// internal function prototypes
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
static void ReportAPIError(DWORD msgCode, DWORD errCode = 0);
static void ReportPythonError(DWORD);
static BOOL ReportError(DWORD, LPCTSTR *inserts = NULL, WORD errorType = EVENTLOG_ERROR_TYPE);

BOOL RegisterPythonServiceExe(void);
BOOL WINAPI DebugControlHandler ( DWORD dwCtrlType );

BOOL LocatePythonServiceClass( DWORD dwArgc, LPTSTR *lpszArgv, PyObject **result );

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

// The built-in Python module.
static PyObject *servicemanager_startup_error;
static PyObject *g_obServiceCtrlHandler = NULL;

VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
	if (g_obServiceCtrlHandler==NULL) { // Python is in error.
		if (!bServiceDebug)
			SetServiceStatus( sshStatusHandle, &errorStatus );
		return;
	}
	// Ensure we have a context for our thread.
	CEnterLeavePython celp;
	PyObject *args = Py_BuildValue("(l)", dwCtrlCode);
	PyObject *result = PyObject_CallObject(g_obServiceCtrlHandler, args);
	Py_XDECREF(args);
	if (result==NULL)
		ReportPythonError(PYS_E_SERVICE_CONTROL_FAILED);
	else
		Py_DECREF(result);
}

static PyObject *DoLogMessage(WORD errorType, PyObject *obMsg)
{
	BSTR msg;
	if (!PyWinObject_AsBstr(obMsg, &msg))
		return NULL;
	DWORD errorCode = errorType==EVENTLOG_ERROR_TYPE ? PYS_E_GENERIC_ERROR : PYS_E_GENERIC_WARNING;
	LPCTSTR inserts[] = {msg, NULL};
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReportError(errorCode, inserts, errorType);
	SysFreeString(msg);
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
		memset(pStrings, 0, sizeof(BSTR *)*(numStrings+1)); // this also terminates array!
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
	BSTR bstrName;
	if (!PyWinObject_AsBstr(nameOb, &bstrName))
		return NULL;
	Py_XDECREF(g_obServiceCtrlHandler);
	g_obServiceCtrlHandler = obCallback;
	Py_INCREF(obCallback);
	if (bServiceDebug) { // If debugging, get out now, and give None back.
		Py_INCREF(Py_None);
		return Py_None;
	}
	sshStatusHandle = RegisterServiceCtrlHandler(bstrName, service_ctrl);
	SysFreeString(bstrName);
	PyObject *rc;
	if (sshStatusHandle==0) {
		Py_DECREF(obCallback);
		obCallback = NULL;
		rc = PyWin_SetAPIError("RegisterServiceCtrlHandler");
	} else {
		rc = PyInt_FromLong((long)sshStatusHandle);
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

// @pymethod |servicemanager|Debugging|Indicates if the service is running in debug mode.
static PyObject *PyDebugging(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Debugging"))
		return NULL;
	return PyInt_FromLong(bServiceDebug);
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

// @module servicemanager|A module built in to PythonService.exe (and therefore only available to Python service programs).
// <nl>The module <o win32service> provides other service facilities.
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
}

// Couple of helpers for the service manager
static void PyService_InitPython()
{
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
	PyWinGlobals_Ensure();
	PySys_SetArgv(__argc, __argv);

	initservicemanager();
}

// The Service Manager
//

//
//  FUNCTION: PythonService_main
//
//  PURPOSE: entrypoint for service
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

extern "C" int PythonService_main(int argc, char **argv)
{
	SERVICE_TABLE_ENTRY   DispatchTable[] = 
    { 
        { TEXT(""), 	service_main      }, 
        { NULL,              NULL         } 
    }; 

	// Get the name of the EXE, and store it in the global variable
	// for the Event Source name
	TCHAR tempFileNameBuf[MAX_PATH];
	GetModuleFileName(0, tempFileNameBuf, sizeof(tempFileNameBuf)/sizeof(TCHAR));
	TCHAR *posSlash = _tcsrchr(tempFileNameBuf, _T('\\'));
	if (posSlash)
		_tcscpy( g_szEventSourceName, posSlash+1 );
	TCHAR *posDot = _tcsrchr(g_szEventSourceName, _T('.'));
	if (posDot)
		*posDot = _T('\0');


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
			*/
			char svcNameBuf[256];
			char *svcName;
			int argOffset = 1;
			if (LoadStringA(GetModuleHandle(NULL), RESOURCE_SERVICE_NAME, svcNameBuf, sizeof(svcNameBuf))>1) {
				svcName = svcNameBuf;
			} else {
	        	if (argc<3) {
		    		printf("-debug requires a service name");
					return 1;
        		}
				svcName = argv[2];
				argOffset = 2;
			}
			bServiceDebug = TRUE;
			printf("Debugging service %s\n", svcName);
		    int dwArgc;
		    LPTSTR *lpszArgv;

#ifdef UNICODE
		    lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc) );
#else
		    dwArgc   = argc;
		    lpszArgv = argv;
#endif
	        SetConsoleCtrlHandler( DebugControlHandler, TRUE );
			service_main(dwArgc-argOffset, lpszArgv+argOffset);
        	return 0; // gotta assume OK...
        }
    }

	// To be friendly, say what we are doing
    printf("%s - Python Service Manager\n", argv[0]);
    printf("Options:\n");
#ifndef BUILD_FREEZE
	printf(" -register - register the EXE - this must be done at least once.\n");
#endif
    printf(" -debug servicename [parms] - debug the Python service.\n");
    printf("\nNOTE: You do not start the service using this program - start the\n");
    printf("service using Control Panel, or 'net start %s'\n", svcName);
    printf("\nConnecting to the service control manager....\n");

    if (!StartServiceCtrlDispatcher( DispatchTable)) {
    	ReportAPIError(PYS_E_API_CANT_START_SERVICE);
    	printf("Could not start the service - error %d\n", GetLastError());
#ifndef BUILD_FREEZE
		RegisterPythonServiceExe();
#endif
    }
	return 2;
}

#ifndef BUILD_FREEZE
int main(int argc, char **argv)
{
	return PythonService_main(argc, argv);
}
#endif


BOOL WINAPI DebugControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
            _tprintf(TEXT("Stopping debug service.\n"));
            service_ctrl(SERVICE_CONTROL_STOP);
            return TRUE;
            break;

    }
    return FALSE;
}

//
//  FUNCTION: service_main
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine performs the service initialization and then calls
//    the user defined ServiceStart() routine to perform majority
//    of the work.
//
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	PyObject *instance = NULL;
	PyObject *start = NULL;

	BOOL bPythonInitedOK = LocatePythonServiceClass(dwArgc, lpszArgv, &instance);

	// If Python has not yet registered the service control handler, then
	// we are in serious trouble - it is likely the service will enter a zombie
	// state, where it wont do anything, but you can not start another.
	if (g_obServiceCtrlHandler==NULL) {
		// If Python seemed to init OK, but hasnt done what we expect,
		// report an error, as we are gunna fail!
		if (bPythonInitedOK) 
			ReportPythonError(E_PYS_NOT_CONTROL_HANDLER);
		if (!bServiceDebug)
			sshStatusHandle = RegisterServiceCtrlHandler( lpszArgv[0], service_ctrl);
	}
	// Not much we can do here.
    if ( !bPythonInitedOK || (!sshStatusHandle && !bServiceDebug) || !instance) {
		if (sshStatusHandle)
			SetServiceStatus( sshStatusHandle, &neverStartedStatus );
        goto cleanup;
	}

	if (!bServiceDebug)
		if (!SetServiceStatus( sshStatusHandle, &startingStatus ))
			ReportAPIError(PYS_E_API_CANT_SET_PENDING);

	start = PyObject_GetAttrString(instance, "SvcRun");
	if (start==NULL)
		ReportPythonError(E_PYS_NO_RUN_METHOD);
	else {
		PyObject *result = PyObject_CallObject(start, NULL);
		if (result==NULL)
			ReportPythonError(E_PYS_START_FAILED);
		else
			Py_DECREF(result);
	}
	// We are all done.
cleanup:

    // try to report the stopped status to the service control manager.
    //
	Py_XDECREF(start);
	Py_XDECREF(instance);
		
    if (sshStatusHandle) { // Wont be true if debugging.
		if (!SetServiceStatus( sshStatusHandle, &stoppedStatus ))
			ReportAPIError(PYS_E_API_CANT_SET_STOPPED);
    }
    return;
}

static BOOL LocatePythonServiceClassString( DWORD dwArgc, LPTSTR *lpszArgv, char *buf, int cchBuf)
{
	char keyName[1024];

	// If not error loading, and not an empty string
	if (LoadStringA(GetModuleHandle(NULL), RESOURCE_SERVICE_NAME, buf, cchBuf)>1)
		// Get out of here now!
		return TRUE;


	HKEY key = NULL;
	BOOL ok = TRUE;
	wsprintfA(keyName, "System\\CurrentControlSet\\Services\\%S\\PythonClass", lpszArgv[0]);
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

static BOOL LocatePythonServiceClass( DWORD dwArgc, LPTSTR *lpszArgv, PyObject **result )
{
	char valueBuf[512];

	BOOL ok = LocatePythonServiceClassString( dwArgc, lpszArgv, valueBuf, sizeof(valueBuf));
	if (!ok)
		return FALSE;
	
	// Initialize Python
	PyService_InitPython();
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
		return FALSE;
	}
	PyObject *module;
	*classNamePos++ = '\0';
	// If we have another '.', then likely a 'ni' package.
/* And we hate ni now :-)
	if (strrchr(keyName, '.')!=NULL) {
		module = PyImport_ImportModule("ni");
		Py_XDECREF(module);
	}
*/
	module = PyImport_ImportModule(fname);
	if (module==NULL) {
		ReportPythonError(E_PYS_NO_MODULE);
		return FALSE;
	}
	PyObject *pyclass = PyObject_GetAttrString(module, classNamePos);
	Py_DECREF(module);
	if (pyclass==NULL) {
		ReportPythonError(E_PYS_NO_CLASS);
		return FALSE;
	}
	PyObject *args = PyTuple_New(dwArgc);
	if (args==NULL) {
		Py_DECREF(pyclass);
		ReportPythonError(PYS_E_NO_MEMORY_FOR_ARGS);
		return FALSE;
	}
	for (DWORD i=0;i<dwArgc;i++) {
		PyObject *arg = PyWinObject_FromWCHAR(lpszArgv[i]);
		if (arg==NULL) {
			Py_DECREF(args);
			Py_DECREF(pyclass);
			ReportPythonError(PYS_E_BAD_ARGS);
			return FALSE;
		}
		PyTuple_SET_ITEM(args, i, arg);
	}
	PyObject *realArgs = PyTuple_New(1);
	PyTuple_SET_ITEM(realArgs, 0, args);
	*result = PyObject_CallObject(pyclass, realArgs);
	if (*result==NULL) {
		ok = FALSE;
		BOOL bHandledError = PyErr_ExceptionMatches(servicemanager_startup_error);
		if (bHandledError)
			PyErr_Clear();
		else
			ReportPythonError(PYS_E_BAD_CLASS);
	}
	Py_DECREF(pyclass);
	return ok;
}

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
		PyWinObject_AsBstr(obStr, inserts+1);
		Py_XDECREF(obStr);
		obStr = PyObject_Str(value);
		PyWinObject_AsBstr(PyObject_Str(obStr), inserts+2);
		Py_XDECREF(obStr);
	    ReportError(code, (LPCTSTR *)inserts);
		if (szTraceback) free(szTraceback);
	    SysFreeString(inserts[1]);
	    SysFreeString(inserts[2]);
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
//    if (bServiceDebug) {
//	    if (PyErr_Occurred())
//		    PyErr_Print();
//    }
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
    			GetModuleHandle(NULL), code, 0, (LPTSTR)&buffer, 0, (va_list *)inserts)==0) {
    		_tprintf(_T("%s 0x%X - No message available\nMessage inserts were"), szType, code);
    		for (int i=0;i<numInserts;i++)
    			_tprintf(_T("'%s',"), inserts[i]);
    	} else {
    		_tprintf(_T("%s 0x%X - %s"), szType, code, buffer);
    		LocalFree(buffer);
    	}
		return TRUE;
	} else {
		// Ensure we are setup in the eventlog
		HKEY hkey;
		TCHAR keyName[MAX_PATH];
		_tcscpy(keyName, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"));
		_tcscat(keyName, g_szEventSourceName );
		// ignore all failures when settingup - for whatever reason it fails,
		// we are probably still better off calling ReportEvent than
		// not calling due to some other failure here.
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
		                   keyName, 
		                   0, 
		                   NULL, 
		                   REG_OPTION_NON_VOLATILE, 
		                   KEY_WRITE, NULL, 
		                   &hkey, 
		                   NULL) == ERROR_SUCCESS) {
			TCHAR fnameBuf[MAX_PATH+MAX_PATH];
			const DWORD fnameBufSize = sizeof(fnameBuf)/sizeof(fnameBuf[0]);
			GetModuleFileName( NULL, fnameBuf, fnameBufSize);
			RegSetValueEx(hkey, TEXT("EventMessageFile"), 0, REG_SZ, 
			              (const BYTE *)fnameBuf, (_tcslen(fnameBuf)+1)*sizeof(TCHAR));
			DWORD types = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
			RegSetValueEx(hkey, TEXT("TypesSupported"), 0, REG_DWORD, 
			              (const BYTE *)&types, sizeof(types));
			RegCloseKey(hkey);
		}
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
	if (!Py_IsInitialized())
		Py_Initialize();
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

