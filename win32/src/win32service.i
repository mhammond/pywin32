/* File : win32service.i */

%module win32service // An interface to the Windows NT Service API

%include "typemaps.i"
%include "pywin32.i"

%init %{
	// All errors raised by this module are of this type.
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);
%}

%{

BOOL BuildDeps(PyObject *obDeps, TCHAR **ppDeps)
{
	TCHAR *lpDeps = NULL;
	BOOL rc = FALSE;
	if (obDeps!=Py_None) {
		if (!PySequence_Check(obDeps)) {
			PyErr_SetString(PyExc_ValueError, "Dependencies must be None or a list of strings");
			goto cleanup;
		}
 		int numStrings = PySequence_Length(obDeps);
		// Need to loop twice - once to get the buffer length
		int len = 0;
		for (int i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(obDeps, i);
			if (obString==NULL)
				goto cleanup;
			if (!PyString_Check(obString)) {
				Py_DECREF(obString);
				PyErr_SetString(PyExc_ValueError, "The list items for Dependencies must all be strings");
				goto cleanup;
			}
			len += PyString_Size(obString) + 1;
			Py_DECREF(obString);
		}
		// Allocate the buffer
		lpDeps = new TCHAR[len+2]; // Double '\0' terminated
		TCHAR *p = lpDeps;
		for (i=0;i<numStrings;i++) {
			// We know the sequence is valid.
			PyObject *obString = PySequence_GetItem(obDeps, i);
			BSTR pStr;
			if (!PyWinObject_AsTCHAR(obString, &pStr)) {
				Py_DECREF(obString);
				goto cleanup;
			}
			int len = _tcslen(pStr);
			_tcsncpy(p, pStr, len);
			p += len;
			*p++ = L'\0';
			PyWinObject_FreeTCHAR(pStr);
			Py_DECREF(obString);
		}
		*p = L'\0'; // Add second terminator.
	}
	*ppDeps = lpDeps;
	rc = TRUE;
cleanup:
	if (!rc) {
		delete [] lpDeps;
	}
	return rc;
}

PyObject *MyCreateService(
    SC_HANDLE hSCManager,	// handle to service control manager database  
    TCHAR *lpServiceName,	// pointer to name of service to start 
    TCHAR *lpDisplayName,	// pointer to display name 
    DWORD dwDesiredAccess,	// type of access to service 
    DWORD dwServiceType,	// type of service 
    DWORD dwStartType,		// when to start service 
    DWORD dwErrorControl,	// severity if service fails to start 
    TCHAR * lpBinaryPathName,	// pointer to name of binary file 
    TCHAR * lpLoadOrderGroup,	// pointer to name of load ordering group 
    BOOL  bFetchTag,
    PyObject *obDeps,		// array of dependency names 
    TCHAR *lpServiceStartName,	// pointer to account name of service 
    TCHAR *lpPassword 	// pointer to password for service account 
   )
{
	PyObject *rc = NULL;
	TCHAR *lpDeps = NULL;
	DWORD tagID;
	DWORD *pTagID = bFetchTag ? &tagID : NULL;
	SC_HANDLE sh = 0;
	if (!BuildDeps(obDeps, &lpDeps))
		goto cleanup;

	sh = CreateService(hSCManager,lpServiceName,lpDisplayName,dwDesiredAccess,
	                             dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
	                             lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword);
	if (sh==0) {
		PyWin_SetAPIError("CreateService");
		rc = NULL;
	} else {
		if (bFetchTag)
			rc = Py_BuildValue("ll", sh, tagID);
		else
			rc = PyInt_FromLong((long)sh);
	}
cleanup:
	delete [] lpDeps;
	return rc;
		
}

PyObject *MyChangeServiceConfig(
    SC_HANDLE hSCManager,	// handle to service control manager database  
    DWORD dwServiceType,	// type of service 
    DWORD dwStartType,		// when to start service 
    DWORD dwErrorControl,	// severity if service fails to start 
    TCHAR * lpBinaryPathName,	// pointer to name of binary file 
    TCHAR * lpLoadOrderGroup,	// pointer to name of load ordering group 
    BOOL  bFetchTag,
    PyObject *obDeps,		// array of dependency names 
    TCHAR *lpServiceStartName,	// pointer to account name of service 
    TCHAR *lpPassword, 	// pointer to password for service account 
    TCHAR *lpDisplayName	// pointer to display name 
   )
{
	PyObject *rc = NULL;
	TCHAR *lpDeps = NULL;
	DWORD tagID;
	DWORD *pTagID = bFetchTag ? &tagID : NULL;
	SC_HANDLE sh = 0;
	if (!BuildDeps(obDeps, &lpDeps))
		goto cleanup;

	if (!ChangeServiceConfig(hSCManager,
                         dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
                         lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword,
						 lpDisplayName))
		rc = PyWin_SetAPIError("ChangeServiceConfig");
	else if (bFetchTag)
		rc = PyInt_FromLong(tagID);
	else {
		rc = Py_None;
		Py_INCREF(rc);
	}
cleanup:
	delete [] lpDeps;
	return rc;
		
}

PyObject *MyStartService( SC_HANDLE scHandle, PyObject *serviceArgs )
{
	LPTSTR *pArgs;
	DWORD numStrings = 0;
	if (serviceArgs==Py_None)
		pArgs = NULL;
	else if (!PySequence_Check(serviceArgs)) {
		PyErr_SetString(PyExc_ValueError, "Service arguments must be list of strings.");
		return NULL;
	} else {
		numStrings = PySequence_Length(serviceArgs);
		pArgs = new LPTSTR [numStrings];
		if (pArgs==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating argument arrays");
			return NULL;
		}
		for (DWORD i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(serviceArgs, (int)i);
			if (obString==NULL) {
				delete [] pArgs;
				return NULL;
			}
			pArgs[i] = NULL;
			PyWinObject_AsTCHAR(obString, pArgs+i);
			Py_DECREF(obString);
		}
	}
	PyObject *rc;
	if (StartService(scHandle, numStrings, (LPCTSTR *)pArgs)) {
		rc = Py_None;
		Py_INCREF(Py_None);
	} else
		rc = PyWin_SetAPIError("StartService");
	for (DWORD i=0;i<numStrings;i++)
		PyWinObject_FreeTCHAR(pArgs[i]);
	delete [] pArgs;
	return rc;
}
%}

// These 3 function contributed by Curt Hagenlocher

%native (EnumServicesStatus) MyEnumServicesStatus;

%{
static PyObject *MyEnumServicesStatus(PyObject *self, PyObject *args)
{
	SC_HANDLE hscm;
	DWORD serviceType = SERVICE_WIN32;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "l|ll:EnumServicesStatus", &hscm, &serviceType, &serviceState))
	{
		return NULL;
	}

	long tmp;
	LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)&tmp;
	DWORD bytesNeeded, servicesReturned, resumeHandle = 0;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	EnumServicesStatus(hscm, serviceType, serviceState, services, sizeof(tmp), &bytesNeeded,
		&servicesReturned, &resumeHandle);
	
	if (GetLastError() == ERROR_MORE_DATA)
	{
		buffer = new char[bytesNeeded + 1];
		services = (LPENUM_SERVICE_STATUS)buffer;
		result = EnumServicesStatus(hscm, serviceType, serviceState, services, bytesNeeded + 1,
			&bytesNeeded, &servicesReturned, &resumeHandle);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete buffer;
		return PyWin_SetAPIError("EnumServicesStatus");
	}

	PyObject *retval = PyTuple_New(servicesReturned);
	for (DWORD i = 0; i < servicesReturned; i++)
	{
		PyObject *obServiceName = PyWinObject_FromTCHAR(services[i].lpServiceName);
		PyObject *obDisplayName = PyWinObject_FromTCHAR(services[i].lpDisplayName);
		PyTuple_SetItem(retval, i, Py_BuildValue("OO(lllllll)",
			obServiceName,
			obDisplayName,
			services[i].ServiceStatus.dwServiceType,
			services[i].ServiceStatus.dwCurrentState,
			services[i].ServiceStatus.dwControlsAccepted,
			services[i].ServiceStatus.dwWin32ExitCode,
			services[i].ServiceStatus.dwServiceSpecificExitCode,
			services[i].ServiceStatus.dwCheckPoint,
			services[i].ServiceStatus.dwWaitHint));
		Py_XDECREF(obServiceName);
		Py_XDECREF(obDisplayName);
	}

	delete buffer;
	return retval;
}
%}

%native (EnumDependentServices) MyEnumDependentServices;
%{
static PyObject *MyEnumDependentServices(PyObject *self, PyObject *args)
{
	SC_HANDLE hsc;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "l|l:EnumDependentServices", &hsc, &serviceState))
	{
		return NULL;
	}

	long tmp;
	LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)&tmp;
	DWORD bytesNeeded, servicesReturned, resumeHandle = 0;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	result = EnumDependentServices(hsc, serviceState, services, sizeof(tmp), &bytesNeeded,
		&servicesReturned);
	
	if (!result && GetLastError() == ERROR_MORE_DATA)
	{
		buffer = new char[bytesNeeded + 1];
		services = (LPENUM_SERVICE_STATUS)buffer;
		result = EnumDependentServices(hsc, serviceState, services, bytesNeeded + 1,
			&bytesNeeded, &servicesReturned);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete buffer;
		return PyWin_SetAPIError("EnumDependentServices");
	}

	PyObject *retval = PyTuple_New(servicesReturned);
	for (DWORD i = 0; i < servicesReturned; i++)
	{
		PyObject *obServiceName = PyWinObject_FromTCHAR(services[i].lpServiceName);
		PyObject *obDisplayName = PyWinObject_FromTCHAR(services[i].lpDisplayName);
		PyTuple_SetItem(retval, i, Py_BuildValue("OO(lllllll)",
			obServiceName,
			obDisplayName,
			services[i].ServiceStatus.dwServiceType,
			services[i].ServiceStatus.dwCurrentState,
			services[i].ServiceStatus.dwControlsAccepted,
			services[i].ServiceStatus.dwWin32ExitCode,
			services[i].ServiceStatus.dwServiceSpecificExitCode,
			services[i].ServiceStatus.dwCheckPoint,
			services[i].ServiceStatus.dwWaitHint));
		Py_XDECREF(obServiceName);
		Py_XDECREF(obDisplayName);
	}

	delete buffer;
	return retval;
}
%}

%native (QueryServiceConfig) MyQueryServiceConfig;

%{
static PyObject *MyQueryServiceConfig(PyObject *self, PyObject *args)
{
	SC_HANDLE hsc;
	if (!PyArg_ParseTuple(args, "l:QueryServiceConfig", &hsc))
	{
		return NULL;
	}

	long tmp;
	LPQUERY_SERVICE_CONFIG config = (LPQUERY_SERVICE_CONFIG)&tmp;
	DWORD bytesNeeded;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	result = QueryServiceConfig(hsc, config, sizeof(tmp), &bytesNeeded);
	
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		buffer = new char[bytesNeeded + 1];
		config = (LPQUERY_SERVICE_CONFIG)buffer;
		result = QueryServiceConfig(hsc, config, bytesNeeded, &bytesNeeded);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete buffer;
		return PyWin_SetAPIError("QueryServiceConfig");
	}

	PyObject *obBinaryPathName = PyWinObject_FromTCHAR(config->lpBinaryPathName);
	PyObject *obLoadOrderGroup = PyWinObject_FromTCHAR(config->lpLoadOrderGroup);
	PyObject *obDependencies = PyWinObject_FromTCHAR(config->lpDependencies);
	PyObject *obServiceStartName = PyWinObject_FromTCHAR(config->lpServiceStartName);
	PyObject *obDisplayName = PyWinObject_FromTCHAR(config->lpDisplayName);
	PyObject *retval = Py_BuildValue("lllOOlOOO",
			config->dwServiceType,
			config->dwStartType,
			config->dwErrorControl,
			obBinaryPathName,
			obLoadOrderGroup,
			config->dwTagId,
			obDependencies,
			obServiceStartName,
			obDisplayName);
	Py_XDECREF(obBinaryPathName);
	Py_XDECREF(obLoadOrderGroup);
	Py_XDECREF(obDependencies);
	Py_XDECREF(obServiceStartName);
	Py_XDECREF(obDisplayName);

	delete buffer;
	return retval;
}
%}

typedef long SC_HANDLE; // 32 bit?
typedef long SC_LOCK;
typedef long SERVICE_STATUS_HANDLE
//typedef unsigned int TCHAR;

%typemap(python,except) SC_HANDLE {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($source==0)  {
           $cleanup;
           return PyWin_SetAPIError("$name");
      }
}

// SERVICE_STATUS support
%typemap(python,ignore) SERVICE_STATUS *outServiceStatus (SERVICE_STATUS temp) {
	$target = &temp;
}

// @object SERVICE_STATUS|A Win32 service status object is represented by a tuple:
%typemap(python,argout) SERVICE_STATUS *outServiceStatus {
	Py_DECREF($target);
	$target = Py_BuildValue("lllllll", 
		$source->dwServiceType, // @tupleitem 0|int|serviceType|The type of service.
		$source->dwCurrentState, // @tupleitem 1|int|serviceState|The current state of the service.
		$source->dwControlsAccepted, // @tupleitem 2|int|controlsAccepted|The controls the service accepts.
		$source->dwWin32ExitCode, // @tupleitem 3|int|win32ExitCode|The win32 error code for the service.
		$source->dwServiceSpecificExitCode, // @tupleitem 4|int|serviceSpecificErrorCode|The service specific error code.
		$source->dwCheckPoint, // @tupleitem 5|int|checkPoint|The checkpoint reported by the service.
		$source->dwWaitHint); // @tupleitem 6|int|waitHint|The wait hint reported by the service.
}

%typemap(python,in) SERVICE_STATUS *inServiceStatus (SERVICE_STATUS junk) {
	$target = &junk;
	if (!PyArg_ParseTuple($source, "lllllll", 
		&$target->dwServiceType,
		&$target->dwCurrentState,
		&$target->dwControlsAccepted,
		&$target->dwWin32ExitCode,
		&$target->dwServiceSpecificExitCode,
		&$target->dwCheckPoint,
		&$target->dwWaitHint))
		return NULL;
}

// @pyswig |StartService|Starts the specified service
%name (StartService) PyObject *MyStartService (
     SC_HANDLE  scHandle, // @pyparm int|scHandle||Handle to the Service Control Mananger
     PyObject *pyobject /* serviceArgs */); // @pyparm [string, ...]|args||Arguments to the service.

// @pyswig int|OpenService|Returns a handle to the specified service.
SC_HANDLE OpenService(
	SC_HANDLE hSCManager, // @pyparm int|scHandle||Handle to the Service Control Mananger
	TCHAR *name, // @pyparm <o PyUnicode>|name||The name of the service to open.
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired.

// @pyswig int|OpenSCManager|Returns a handle to the service control manager
SC_HANDLE OpenSCManager(
	TCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|machineName||The name of the computer, or None
	TCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|dbName||The name of the service database, or None
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired.

// @pyswig |CloseServiceHandle|Closes a service handle
BOOLAPI CloseServiceHandle(SC_HANDLE handle); // @pyparm int|scHandle||Handle to close

// @pyswig <o SERVICE_STATUS>|QueryServiceStatus|Queries a service status
BOOLAPI QueryServiceStatus(SC_HANDLE handle, SERVICE_STATUS *outServiceStatus);
// @pyparm int|scHandle||Handle to query

// @pyswig <o SERVICE_STATUS>|SetServiceStatus|Sets a service status
BOOLAPI SetServiceStatus(
	SERVICE_STATUS_HANDLE hSCManager, // @pyparm int|scHandle||Handle to set
	SERVICE_STATUS *inServiceStatus); // @pyparm object|serviceStatus||The new status

// @pyswig <o SERVICE_STATUS>|ControlService|Sends a control message to a service.
// @rdesc The result is the new service status.
BOOLAPI ControlService(
    SC_HANDLE handle, // @pyparm int|scHandle||Handle to control
    DWORD status, // @pyparm int|code||The service control code.
    SERVICE_STATUS *outServiceStatus);

// @pyswig |DeleteService|Deletes the specified service
BOOLAPI DeleteService(SC_HANDLE);
// @pyparm int|scHandle||Handle to delete

// @pyswig int/(int, int)|CreateService|Creates a new service.
%name (CreateService) PyObject * MyCreateService(
    SC_HANDLE hSCManager,	// @pyparm int|scHandle||handle to service control manager database  
    TCHAR *name,	// @pyparm <o PyUnicode>|name||Name of service
    TCHAR *displayName,	// @pyparm <o PyUnicode>|displayName||Display name 
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||type of access to service 
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service 
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service 
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start
    TCHAR *binaryFile,	// @pyparm <o PyUnicode>|binaryFile||name of binary file 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,            // @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is a tuple of (handle, tag), otherwise just handle.
    PyObject *pyobject,		// @pyparm [<o PyUnicode>,...]|serviceDeps||sequence of dependency names 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK 	// @pyparm <o PyUnicode>|password||password for service account , or None
   );

// @pyswig int/None|ChangeServiceConfig|Changes the configuration of an existing service.
%name (ChangeServiceConfig) PyObject * MyChangeServiceConfig(
    SC_HANDLE hSCManager,	// @pyparm int|scHandle||handle to service control manager database  
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service, or SERVICE_NO_CHANGE
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service, or SERVICE_NO_CHANGE
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start, or SERVICE_NO_CHANGE
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|binaryFile||name of binary file, or None
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,            // @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is the tag, else None.
    PyObject *pyobject,		// @pyparm [<o PyUnicode>,...]|serviceDeps||sequence of dependency names 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK, 	// @pyparm <o PyUnicode>|password||password for service account , or None
    TCHAR *INPUT_NULLOK	// @pyparm <o PyUnicode>|displayName||Display name 
   );

// @pyswig int|LockServiceDatabase|Locks the service database.
SC_LOCK LockServiceDatabase(
	SC_HANDLE handle // @pyparm int|sc_handle||A handle to the SCM.
);

// @pyswig int|UnlockServiceDatabase|Unlocks the service database.
BOOLAPI UnlockServiceDatabase(
	SC_LOCK lock // @pyparm int|lock||A lock provided by <om win32service.LockServiceDatabase>
);

%{
// @pyswig (int, <o PyUnicode>, int)|QueryServiceLockStatus|Retrieves the lock status of the specified service control manager database. 
static PyObject *PyQueryServiceLockStatus(PyObject *self, PyObject *args)
{
	long handle;
	// @pyparm int|handle||Handle to the SCM.
	if (!PyArg_ParseTuple(args, "l:QueryServiceLockStatus", &handle))
		return NULL;

	DWORD bufSize;
	QueryServiceLockStatus((SC_HANDLE)handle, NULL, 0, &bufSize);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("QueryServiceLockStatus");
	QUERY_SERVICE_LOCK_STATUS *buf;
	buf = (QUERY_SERVICE_LOCK_STATUS *)malloc(bufSize);
	if (buf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "No memory for status buffer");
		return NULL;
	}
	BOOL ok = QueryServiceLockStatus((SC_HANDLE)handle, buf, bufSize, &bufSize);
	PyObject *ret;
	if (ok) {
		// @rdesc The result is a tuple of (bIsLocked, userName, lockDuration)
		PyObject *str = PyWinObject_FromTCHAR(buf->lpLockOwner);
		ret = Py_BuildValue("lOl", buf->fIsLocked, str, buf->dwLockDuration);
		Py_XDECREF(str);
	} else
		ret = PyWin_SetAPIError("QueryServiceLockStatus");
	free(buf);
	return ret;
}
%}
%native (QueryServiceLockStatus) PyQueryServiceLockStatus;

#define SERVICE_WIN32 SERVICE_WIN32

#define SERVICE_DRIVER SERVICE_DRIVER

#define SERVICE_ACTIVE SERVICE_ACTIVE

#define SERVICE_INACTIVE SERVICE_INACTIVE

#define SERVICE_STATE_ALL SERVICE_STATE_ALL

#define SERVICE_CONTROL_STOP SERVICE_CONTROL_STOP 
// Requests the service to stop. The hService handle must have SERVICE_STOP access.
#define SERVICE_CONTROL_PAUSE SERVICE_CONTROL_PAUSE 
// Requests the service to pause. The hService handle must have SERVICE_PAUSE_CONTINUE access.
#define SERVICE_CONTROL_CONTINUE SERVICE_CONTROL_CONTINUE 
// Requests the paused service to resume. The hService handle must have SERVICE_PAUSE_CONTINUE access.
#define SERVICE_CONTROL_INTERROGATE SERVICE_CONTROL_INTERROGATE 
// Requests the service to update immediately its current status information to the service control manager. The hService handle must have SERVICE_INTERROGATE access.
#define SERVICE_CONTROL_SHUTDOWN SERVICE_CONTROL_SHUTDOWN 
// The ControlService function fails if this control code is specified.

#define SC_MANAGER_ALL_ACCESS SC_MANAGER_ALL_ACCESS 
// Includes STANDARD_RIGHTS_REQUIRED, in addition to all of the access types listed in this table.
#define SC_MANAGER_CONNECT SC_MANAGER_CONNECT 
// Enables connecting to the service control manager.
#define SC_MANAGER_CREATE_SERVICE SC_MANAGER_CREATE_SERVICE 
// Enables calling of the CreateService function to create a service object and add it to the database.
#define SC_MANAGER_ENUMERATE_SERVICE SC_MANAGER_ENUMERATE_SERVICE 
// Enables calling of the EnumServicesStatus function to list the services that are in the database.
#define SC_MANAGER_LOCK SC_MANAGER_LOCK 
// Enables calling of the LockServiceDatabase function to acquire a lock on the database.
#define SC_MANAGER_QUERY_LOCK_STATUS SC_MANAGER_QUERY_LOCK_STATUS 
// Enables calling of the QueryServiceLockStatus function to retrieve the lock status information for the database.
	
#define SC_MANAGER_MODIFY_BOOT_CONFIG SC_MANAGER_MODIFY_BOOT_CONFIG

#define SC_GROUP_IDENTIFIER  SC_GROUP_IDENTIFIER 

#define SERVICE_WIN32_OWN_PROCESS SERVICE_WIN32_OWN_PROCESS 
// A service type flag that indicates a Win32 service that runs in its own process.
#define SERVICE_WIN32_SHARE_PROCESS SERVICE_WIN32_SHARE_PROCESS 
// A service type flag that indicates a Win32 service that shares a process with other services.
#define SERVICE_KERNEL_DRIVER SERVICE_KERNEL_DRIVER 
// A service type flag that indicates a Windows NT device driver.
#define SERVICE_FILE_SYSTEM_DRIVER SERVICE_FILE_SYSTEM_DRIVER 
// A service type flag that indicates a Windows NT file system driver.
#define SERVICE_INTERACTIVE_PROCESS  SERVICE_INTERACTIVE_PROCESS  
// A flag that indicates a Win32 service process that can interact with the desktop.
 
#define SERVICE_STOPPED	SERVICE_STOPPED 
// The service is not running.
#define SERVICE_START_PENDING SERVICE_START_PENDING 
// The service is starting.
#define SERVICE_STOP_PENDING SERVICE_STOP_PENDING 
// The service is stopping.
#define SERVICE_RUNNING SERVICE_RUNNING 
// The service is running.
#define SERVICE_CONTINUE_PENDING SERVICE_CONTINUE_PENDING 
// The service continue is pending.
#define SERVICE_PAUSE_PENDING SERVICE_PAUSE_PENDING 
// The service pause is pending.
#define SERVICE_PAUSED SERVICE_PAUSED 
// The service is paused.
 
#define SERVICE_ACCEPT_STOP SERVICE_ACCEPT_STOP 
// The service can be stopped. This enables the SERVICE_CONTROL_STOP value.
#define SERVICE_ACCEPT_PAUSE_CONTINUE SERVICE_ACCEPT_PAUSE_CONTINUE 
// The service can be paused and continued. This enables the SERVICE_CONTROL_PAUSE and SERVICE_CONTROL_CONTINUE values.
#define SERVICE_ACCEPT_SHUTDOWN SERVICE_ACCEPT_SHUTDOWN 
// The service is notified when system shutdown occurs. This enables the system to send a SERVICE_CONTROL_SHUTDOWN value to the service. The ControlService function cannot send this control 


//#define SERVICE_ERROR_IGNORER_IGNORE SERVICE_ERROR_IGNORER_IGNORE
#define SERVICE_BOOT_START SERVICE_BOOT_START 
// Specifies a device driver started by the operating system loader. This value is valid only if the service type is SERVICE_KERNEL_DRIVER or SERVICE_FILE_SYSTEM_DRIVER.
#define SERVICE_SYSTEM_START SERVICE_SYSTEM_START 
// Specifies a device driver started by the IoInitSystem function. This value is valid only if the service type is SERVICE_KERNEL_DRIVER or SERVICE_FILE_SYSTEM_DRIVER.
#define SERVICE_AUTO_START SERVICE_AUTO_START 
// Specifies a device driver or Win32 service started by the service control manager automatically during system startup.
#define SERVICE_DEMAND_START SERVICE_DEMAND_START 
// Specifies a device driver or Win32 service started by the service control manager when a process calls the StartService function.
#define SERVICE_DISABLED SERVICE_DISABLED 
// Specifies a device driver or Win32 service that can no longer be started.
 
#define SERVICE_ERROR_IGNORE SERVICE_ERROR_IGNORE 
// The startup (boot) program logs the error but continues the startup operation.
#define SERVICE_ERROR_NORMAL SERVICE_ERROR_NORMAL 
// The startup program logs the error and displays a message box pop-up but continues the startup operation.
#define SERVICE_ERROR_SEVERE SERVICE_ERROR_SEVERE 
// The startup program logs the error. If the last-known good configuration is being started, 
	// the startup operation continues. Otherwise, the system is restarted with the last-known-good configuration.
#define SERVICE_ERROR_CRITICAL SERVICE_ERROR_CRITICAL 
// The startup program logs the error, if possible. If the last-known good configuration is being started, 
	// the startup operation fails. Otherwise, the system is restarted with the last-known good configuration.

#define SERVICE_ALL_ACCESS SERVICE_ALL_ACCESS
// Includes STANDARD_RIGHTS_REQUIRED in addition to all of the access types listed in this table. 

#define SERVICE_CHANGE_CONFIG SERVICE_CHANGE_CONFIG
// Enables calling of the ChangeServiceConfig function to change the service configuration. 

#define SERVICE_ENUMERATE_DEPENDENTS SERVICE_ENUMERATE_DEPENDENTS
//Enables calling of the EnumDependentServices function to enumerate all the services dependent on the service. 

#define SERVICE_INTERROGATE SERVICE_INTERROGATE
// Enables calling of the ControlService function to ask the service to report its status immediately. 

#define SERVICE_PAUSE_CONTINUE SERVICE_PAUSE_CONTINUE
// Enables calling of the ControlService function to pause or continue the service. 

#define SERVICE_QUERY_CONFIG SERVICE_QUERY_CONFIG
// Enables calling of the QueryServiceConfig function to query the service configuration. 

#define SERVICE_QUERY_STATUS SERVICE_QUERY_STATUS
// Enables calling of the QueryServiceStatus function to ask the service control manager about the status of the service. 

#define SERVICE_START SERVICE_START
// Enables calling of the StartService function to start the service. 

#define SERVICE_STOP SERVICE_STOP
// Enables calling of the ControlService function to stop the service. 

#define SERVICE_USER_DEFINED_CONTROL SERVICE_USER_DEFINED_CONTROL
// Enables calling of the ControlService function to specify a user-defined control code. 

#define SERVICE_NO_CHANGE SERVICE_NO_CHANGE // Indicates the parameter should not be changed.

#define SERVICE_SPECIFIC_ERROR ERROR_SERVICE_SPECIFIC_ERROR  // A service specific error has occurred.
