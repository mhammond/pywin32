/* File : wincerapi.i */
// @doc

%module wincerapi // A module which provides an interface to the win32 CE Remote API

%include "typemaps.i"
%include "pywintypes.i"

%{
#include "Rapi.h"
#include "assert.h"
#include "SHLOBJ.H"

PyObject *PyWinObject_FromCEHANDLE(HANDLE);
BOOL PyWinObject_AsCEHANDLE(PyObject *ob, HANDLE *pRes, BOOL bNoneOK);

DWORD GetLastCEError()
{
	DWORD rc = CeRapiGetError();
	return rc==0 ? CeGetLastError() : rc;
}

// Identical to PyW32_BEGIN_ALLOW_THREADS except no script "{" !!!
// means variables can be declared between the blocks
#define PyW32_BEGIN_ALLOW_THREADS PyThreadState *_save = PyEval_SaveThread();
#define PyW32_END_ALLOW_THREADS PyEval_RestoreThread(_save);
#define PyW32_BLOCK_THREADS Py_BLOCK_THREADS

%}

%typemap(python,except) PyCEHANDLE {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($source==0 || $source==INVALID_HANDLE_VALUE)  {
           $cleanup
           return PyWin_SetAPIError("$name",GetLastCEError());
      }
}

%typemap(python,in) PyCEHANDLE {
	if (!PyWinObject_AsCEHANDLE($source, &$target, FALSE))
		return NULL;
}

%typemap(python,in) PyCEHANDLE INPUT_NULLOK {
	if (!PyWinObject_AsCEHANDLE($source, &$target, TRUE))
		return NULL;
}

%typemap(python,ignore) PyCEHANDLE *OUTPUT(HANDLE temp)
{
  $target = &temp;
}

%typemap(python,out) PyCEHANDLE {
  $target = PyWinObject_FromCEHANDLE($source);
}

%typemap(python,argout) PyCEHANDLE *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromCEHANDLE(*$source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}


typedef HANDLE PyCEHANDLE;
%{
#define PyCEHANDLE HANDLE;
%}

%typedef int HRESULTAPI

%typemap(python,out) HRESULTAPI {
	$target = Py_None;
	Py_INCREF(Py_None);
}

%typemap(python,except) HRESULTAPI {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($source))  {
           $cleanup
           return PyWin_SetAPIError("$name", $source);
      }
}

%typedef BOOL BOOLCEAPI

%typemap(python,out) BOOLCEAPI {
	$target = Py_None;
	Py_INCREF(Py_None);
}

%typemap(python,except) BOOLCEAPI {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (!$source)  {
           $cleanup
           return PyWin_SetAPIError("$name", GetLastCEError());
      }
}

%typemap(python,in) STARTUPINFO *
{
	if ($source!=Py_None) {
		PyErr_SetString(PyExc_TypeError, "STARTUPINFO must be None on Windows CE");
		return NULL;
	}
	$target = NULL;
}

// @pyswig |CeRapiInit|Initializes the remote API.
HRESULTAPI CeRapiInit();

// @pyswig |CeRapiUninit|UnInitializes the remote API.
HRESULTAPI CeRapiUninit();


%{

static BOOL CreateEnvironmentString(PyObject *env, LPVOID *ppRet, BOOL *pRetIsUnicode)
{
	*pRetIsUnicode = FALSE;
	if (env==Py_None) {
		*ppRet = NULL;
		return TRUE;
	}
	// First loop counting the size of the environment.
	if (!PyMapping_Check(env)) {
		PyErr_SetString(PyExc_TypeError, "environment parameter must be a dictionary object of strings or unicode objects.");
		return FALSE;
	}
	int envLength = PyMapping_Length(env);
	PyObject *keys = PyMapping_Keys(env);
	PyObject *vals = PyMapping_Values(env);
	if (!keys || !vals)
		return FALSE;

	int i;
	BOOL bIsUnicode;
	unsigned bufLen = 0;
	for (i=0;i<envLength;i++) {
		PyObject *key = PyList_GetItem(keys, i);
		PyObject *val = PyList_GetItem(vals, i);
		if (i==0) {
			if (PyString_Check(key)) {
				bIsUnicode = FALSE;
				bufLen += PyString_Size(key) + 1;
			} else if (PyUnicode_Check(key)) {
				bIsUnicode = TRUE;
				bufLen += PyUnicode_Size(key) + 1;
			} else {
				PyErr_SetString(PyExc_TypeError, "dictionary must have keys and values as strings or unicode objects.");
				Py_DECREF(keys);
				Py_DECREF(vals);
				return FALSE;
			}
		} else {
			if (bIsUnicode) {
				if (!PyUnicode_Check(key)) {
					PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
					Py_DECREF(keys);
					Py_DECREF(vals);
					return FALSE;
				}
				bufLen += PyUnicode_Size(key) + 1;
			}
			else {
				if (!PyString_Check(key)) {
					PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
					Py_DECREF(keys);
					Py_DECREF(vals);
					return FALSE;
				}
				bufLen += PyString_Size(key) + 1;
			}
		}
		if (bIsUnicode) {
			if (!PyUnicode_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				Py_DECREF(keys);
				Py_DECREF(vals);
				return FALSE;
			}
			bufLen += PyUnicode_Size(val) + 2; // For the '=' and '\0'
		}
		else {
			if (!PyString_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				Py_DECREF(keys);
				Py_DECREF(vals);
				return FALSE;
			}
			bufLen += PyString_Size(val) + 2; // For the '=' and '\0'
		}
	}
	LPVOID result = (LPVOID)malloc( (bIsUnicode ? sizeof(WCHAR) : sizeof(char)) * (bufLen + 1) );
	WCHAR *pUCur = (WCHAR *)result;
	char *pACur = (char *)result;
	// Now loop filling it!
	for (i=0;i<envLength;i++) {
		PyObject *key = PyList_GetItem(keys, i);
		PyObject *val = PyList_GetItem(vals, i);
		if (bIsUnicode) {
			BSTR pTemp;
			PyWinObject_AsBstr(key, &pTemp);
			wcscpy(pUCur, pTemp);
			pUCur += wcslen(pTemp);
			PyWinObject_FreeBstr(pTemp);
		} else {
			char *pTemp = PyString_AsString(key);
			strcpy(pACur, pTemp);
			pACur += strlen(pTemp);
		}
		if (bIsUnicode)
			*pUCur++ = L'=';
		else
			*pACur++ = '=';
		if (bIsUnicode) {
			BSTR pTemp;
			PyWinObject_AsBstr(val, &pTemp);
			wcscpy(pUCur, pTemp);
			pUCur += wcslen(pTemp);
			PyWinObject_FreeBstr(pTemp);
		} else {
			char *pTemp = PyString_AsString(val);
			strcpy(pACur, pTemp);
			pACur += strlen(pTemp);
		}
		if (bIsUnicode)
			*pUCur++ = L'\0';
		else
			*pACur++ = '\0';
	}
	Py_DECREF(keys);
	Py_DECREF(vals);
	if (bIsUnicode) {
		*pUCur++ = L'\0';
		assert(((unsigned)(pUCur - (WCHAR *)result))==bufLen);
	} else {
		*pACur++ = '\0';
		assert(((unsigned)(pACur - (char *)result))==bufLen);
	}
	*pRetIsUnicode = bIsUnicode;
	*ppRet = result;

	return TRUE;
}

PyObject *MyCreateProcess(
	TCHAR *appName, 
	TCHAR *cmdLine, 
	SECURITY_ATTRIBUTES *psaP,
	SECURITY_ATTRIBUTES *psaT,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	PyObject *environment,
	TCHAR *directory,
	STARTUPINFO *si)
{
	PROCESS_INFORMATION pi;
	// Convert the environment.
	LPVOID pEnv;
	BOOL bEnvIsUnicode;
	if (!CreateEnvironmentString(environment, &pEnv, &bEnvIsUnicode))
		return NULL;

	if (bEnvIsUnicode)
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = CeCreateProcess(appName, cmdLine, psaP, psaT, bInheritHandles, dwCreationFlags, pEnv, directory, si, &pi);
	PyW32_END_ALLOW_THREADS

	free(pEnv);

	if (!ok)
		return PyWin_SetAPIError("CeCreateProcess", GetLastCEError());

	PyObject *ret = PyTuple_New(4);
	PyTuple_SET_ITEM(ret, 0, PyWinObject_FromCEHANDLE(pi.hProcess));
	PyTuple_SET_ITEM(ret, 1, PyWinObject_FromCEHANDLE(pi.hThread));
	PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(pi.dwProcessId));
	PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(pi.dwThreadId));
	return ret;
}
%}
// @pyswig <o PyHANDLE>, <o PyHANDLE>, int, int|CreateProcess|Creates a new process and its primary thread. The new process executes the specified executable file.
// @comm The result is a tuple of (hProcess, hThread, dwProcessId, dwThreadId)
%name(CeCreateProcess)
PyObject *MyCreateProcess(
	TCHAR *INPUT_NULLOK,  // @pyparm string|appName||name of executable module, or None
	TCHAR *INPUT_NULLOK,  // @pyparm string|commandLine||command line string, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|processAttributes||process security attributes, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|threadAttributes||thread security attributes, or None
	BOOL bInheritHandles, // @pyparm int|bInheritHandles||handle inheritance flag
	DWORD dwCreationFlags, // @pyparm int|dwCreationFlags||creation flags
	PyObject *env, // @pyparm None|newEnvironment||A dictionary of stringor Unicode pairs to define the environment for the process, or None to inherit the current environment.
	TCHAR *INPUT_NULLOK, // @pyparm string|currentDirectory||current directory name, or None
	STARTUPINFO *lpStartupInfo // @pyparm <o PySTARTUPINFO>|startupinfo||a STARTUPINFO object that specifies how the main window for the new process should appear.

);

%{
PyObject *PyCeRapiInitEx(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	RAPIINIT ri;
	ri.cbSize = sizeof(ri);
	PyW32_BEGIN_ALLOW_THREADS
	HRESULT hr = CeRapiInitEx(&ri);
	PyW32_END_ALLOW_THREADS
	if (hr==S_OK) { // || hr==CERAPI_E_ALREADYINITIALIZED)
		PyObject *obHandle = PyWinObject_FromHANDLE(ri.heRapiInit);
		PyObject *ret = Py_BuildValue("Ol", obHandle, ri.hrRapiInit);
		Py_DECREF(obHandle);
		return ret;
	} else
		return PyWin_SetAPIError("CeCreateProcess", hr);
}
%}
// @pyswig int|CeRapiInitEx|Initializes the remote API asynchronously.
%native(CeRapiInitEx) PyCeRapiInitEx;


// @pyswig |CeCopyFile|Copies a file
BOOLCEAPI CeCopyFile(
    TCHAR *from, // @pyparm <o PyUnicode>|from||The name of the file to copy from
    TCHAR *to, // @pyparm <o PyUnicode>|to||The name of the file to copy to
    BOOL bFailIfExists); // @pyparm int|bFailIfExists||Indicates if the operation should fail if the file exists.

// @pyswig |CeCheckPassword|This function compares a specified string to the system password.
BOOLCEAPI CeCheckPassword(
	TCHAR *password // @pyparm <o PyUnicode>|password||The password to compare.
);

// @pyswig <o PyCEHANDLE>|CeCreateFile|Creates or opens the a file or other object and returns a handle that can be used to access the object.
// @comm The following objects can be opened:<nl>files<nl>pipes<nl>mailslots<nl>communications resources<nl>disk devices (Windows NT only)<nl>consoles<nl>directories (open only)
PyCEHANDLE CeCreateFile(
    TCHAR *lpFileName,	// @pyparm <o PyUnicode>|fileName||The name of the file
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||access (read-write) mode
			// Specifies the type of access to the object. An application can obtain read access, write access, read-write access, or device query access. This parameter can be any combination of the following values. 
			// @flagh Value|Meaning 
			// @flag 0|Specifies device query access to the object. An application can query device attributes without accessing the device.
			// @flag GENERIC_READ|Specifies read access to the object. Data can be read from the file and the file pointer can be moved. Combine with GENERIC_WRITE for read-write access.  
			// @flag GENERIC_WRITE|Specifies write access to the object. Data can be written to the file and the file pointer can be moved. Combine with GENERIC_READ for read-write access.
    DWORD dwShareMode,	// @pyparm int|shareMode||Set of bit flags that specifies how the object can be shared. If dwShareMode is 0, the object cannot be shared. Subsequent open operations on the object will fail, until the handle is closed. 
			// To share the object, use a combination of one or more of the following values:
			// @flagh Value|Meaning 
			// @flag FILE_SHARE_DELETE|Windows NT: Subsequent open operations on the object will succeed only if delete access is requested.  
			// @flag FILE_SHARE_READ|Subsequent open operations on the object will succeed only if read access is requested.
			// @flag FILE_SHARE_WRITE|Subsequent open operations on the object will succeed only if write access is requested.
    SECURITY_ATTRIBUTES *lpSecurityAttributes,	// @pyparm <o PySECURITY_ATTRIBUTES>|attributes||The security attributes, or None
    DWORD dwCreationDistribution,	// @pyparm int|creationDisposition||Specifies which action to take on files that exist, and which action to take when files do not exist. For more information about this parameter, see the Remarks section. This parameter must be one of the following values:
			// @flagh Value|Meaning
			// @flag CREATE_NEW|Creates a new file. The function fails if the specified file already exists. 
			// @flag CREATE_ALWAYS|Creates a new file. If the file exists, the function overwrites the file and clears the existing attributes. 
			// @flag OPEN_EXISTING|Opens the file. The function fails if the file does not exist. 
			//       See the Remarks section for a discussion of why you should use the OPEN_EXISTING flag if you are using the CreateFile function for devices, including the console. 
			// @flag OPEN_ALWAYS|Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDisposition were CREATE_NEW. 
			// @flag TRUNCATE_EXISTING|Opens the file. Once opened, the file is truncated so that its size is zero bytes. The calling process must open the file with at least GENERIC_WRITE access. The function fails if the file does not exist. 
    DWORD dwFlagsAndAttributes,	// @pyparm int|flagsAndAttributes||file attributes
    PyCEHANDLE INPUT_NULLOK // @pyparm <o PyHANDLE>|hTemplateFile||Specifies a handle with GENERIC_READ access to a template file. The template file supplies file attributes and extended attributes for the file being created.   Under Win95, this must be 0, else an exception will be raised.
);

// @pyswig |CeDeleteFile|Deletes a file.
BOOLCEAPI CeDeleteFile(TCHAR *fileName);
// @pyparm <o PyUnicode>|fileName||The filename to delete

// @pyswig |CeMoveFile|Renames an existing file or a directory (including all its children). 
BOOLCEAPI CeMoveFile(
    TCHAR *lpExistingFileName,	// @pyparm <o PyUnicode>|existingFileName||Name of the existing file  
    TCHAR *lpNewFileName 	// @pyparm <o PyUnicode>|newFileName||New name for the file 
);

// @pyswig |CeCreateDirectory|Creates a directory
BOOLCEAPI CeCreateDirectory(
    TCHAR *name, // @pyparm <o PyUnicode>|name||The name of the directory to create
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig |CeRemoveDirectory|Removes an existing directory
BOOLCEAPI CeRemoveDirectory(
    TCHAR *lpPathName	// @pyparm <o PyUnicode>|lpPathName||Name of the path to remove.
);

%{
// @pyswig <o PyUnicode>|CeGetTempPath|Obtains the temp path on the device.
static PyObject *PyCeGetTempPath(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	TCHAR buf[_MAX_PATH];
	PyW32_BEGIN_ALLOW_THREADS
	DWORD numChars = CeGetTempPath(_MAX_PATH, buf);
	PyW32_END_ALLOW_THREADS
	if (numChars==0)
		return PyWin_SetAPIError("CeGetTempPath", GetLastCEError());
	return PyWinObject_FromTCHAR(buf, numChars);
}
%}
%native (CeGetTempPath) PyCeGetTempPath;

%{
// @pyswig tuple|CeGetSystemInfo|Retrieves information about the CE device.
static PyObject *
PyCeGetSystemInfo(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":GetSystemInfo"))
		return NULL;
	// @pyseeapi GetSystemInfo
	SYSTEM_INFO info;
	PyW32_BEGIN_ALLOW_THREADS
	CeGetSystemInfo( &info );
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("iiiiiiii(ii)", info.dwOemId, info.dwPageSize, 
                         info.lpMinimumApplicationAddress, info.lpMaximumApplicationAddress,
                         info.dwActiveProcessorMask, info.dwNumberOfProcessors,
                         info.dwProcessorType, info.dwAllocationGranularity,
						 info.wProcessorLevel, info.wProcessorRevision);
	// @rdesc The return value is a tuple of 9 values, which corresponds
	// to the Win32 SYSTEM_INFO structure.  The element names are:
	// <nl>dwOemId<nl>dwPageSize<nl>lpMinimumApplicationAddress<nl>lpMaximumApplicationAddress<nl>,
    // dwActiveProcessorMask<nl>dwNumberOfProcessors<nl>
	// dwProcessorType<nl>dwAllocationGranularity<nl>(wProcessorLevel,wProcessorRevision)
}
%}
%native (CeGetSystemInfo) PyCeGetSystemInfo;

// @pyswig int|CeGetDesktopDeviceCaps|Retrieves information about the CE desktop.
int CeGetDesktopDeviceCaps(int nIndex); 

// @pyswig int|CeGetSystemMetrics|Retrieves information about the CE system.
int CeGetSystemMetrics(int nIndex); 

// @pyswig <o PyUnicode>|CeGetSpecialFolderPath|Retrieves the location of special folders on the CE device.
%{
static PyObject *PyCeGetSpecialFolderPath(PyObject *self, PyObject *args)
{
	int typ;
	if (!PyArg_ParseTuple(args, "i", &typ))
		return NULL;
	TCHAR buf[_MAX_PATH];
	PyW32_BEGIN_ALLOW_THREADS
	DWORD numChars = CeGetSpecialFolderPath(typ, _MAX_PATH, buf);
	PyW32_END_ALLOW_THREADS
	if (numChars==0)
		return PyWin_SetAPIError("CeGetSpecialFolderPath", GetLastCEError());
	return PyWinObject_FromTCHAR(buf, numChars);
}
%}
%native (CeGetSpecialFolderPath) PyCeGetSpecialFolderPath;

// @pyswig int, int|CeGetStoreInformation|Retrieves information about store on the CE system.
%{
static PyObject *PyCeGetStoreInformation(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	STORE_INFORMATION si;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = CeGetStoreInformation(&si);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("CeGetStoreInformation", GetLastCEError());
	// @rdesc The result is a tuple of (storeSize, freeSize)
	return Py_BuildValue("ii", si.dwStoreSize, si.dwFreeSize);
}
%}
%native (CeGetStoreInformation) PyCeGetStoreInformation;

// @pyswig tuple|CeGetSystemPowerStatusEx|Retrieves the power status of the CE device.
%{
static PyObject *
PyCeGetSystemPowerStatusEx(PyObject *self, PyObject *args)
{	SYSTEM_POWER_STATUS_EX *lpInfo=NULL;
	PyObject *obInfo=NULL;

	BOOL bUpdate = TRUE;
	if(!PyArg_ParseTuple(args, "|i:CeGetSystemPowerStatusEx", &bUpdate))
		return NULL;
	SYSTEM_POWER_STATUS_EX info;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = CeGetSystemPowerStatusEx(&info, TRUE);
	PyW32_END_ALLOW_THREADS
	if (!ok) {
		return PyWin_SetAPIError("CeGetSystemPowerStatusEx", GetLastCEError());
	}
	// @rdesc The result is a tuple of (ACLineStatus, BatteryFlag, BatteryLifePercent, BatteryLifeTime, BatteryFullLifeTime, BackupBatteryFlag, BackupBatteryLifePercent, BackupBatteryLifeTime, BackupBatteryLifeTime);
	return Py_BuildValue("iiiiiiiii", info.ACLineStatus, info.BatteryFlag, info.BatteryLifePercent, info.BatteryLifeTime, info.BatteryFullLifeTime, info.BackupBatteryFlag, info.BackupBatteryLifePercent, info.BackupBatteryLifeTime, info.BackupBatteryLifeTime);
}
%}
%native (CeGetSystemPowerStatusEx) PyCeGetSystemPowerStatusEx;


// @pyswig |CeSHCreateShortcut|Creates a shortcut on the remote device.
DWORDAPI CeSHCreateShortcut(TCHAR *lpszShortcut, TCHAR *lpszTarget); 

// @pyswig tuple|CeSHGetShortcutTarget|Retrieves the target of a shortcut.
%{
static PyObject *
PyCeSHGetShortcutTarget(PyObject *self, PyObject *args)
{	
	PyObject *obSC;
	if(!PyArg_ParseTuple(args, "|O:CeGetSystemPowerStatusEx", &obSC))
		return NULL;
	TCHAR *sc;
	if (!PyWinObject_AsTCHAR(obSC, &sc, FALSE))
		return NULL;
	TCHAR target[_MAX_PATH];
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = CeSHGetShortcutTarget(sc, target, _MAX_PATH);
	PyW32_END_ALLOW_THREADS
	PyObject *result;
	if (ok)
		result = PyWinObject_FromTCHAR(target);
	else 
		result = PyWin_SetAPIError("CeSHGetShortcutTarget", GetLastCEError());
	PyWinObject_FreeTCHAR(sc);
	return result;
}
%}
%native (CeSHGetShortcutTarget) PyCeSHGetShortcutTarget;

%{
// @pyswig (int,int,int,int,string)|CeGetVersionEx|Returns the current version of Windows, and information about the environment for the CE device.
static PyObject *
PyCeGetVersionEx(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":GetVersionEx"))
		return NULL;
	CEOSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(ver);
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::CeGetVersionEx(&ver);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("CeGetVersionEx", GetLastCEError());
	PyObject *textVersion = PyWinObject_FromTCHAR(ver.szCSDVersion);
	PyObject *rc = Py_BuildValue("iiiiO",
	// @rdesc The return value is a tuple with the following information.<nl>
		         ver.dwMajorVersion, // majorVersion - Identifies the major version number of the operating system.<nl>
				 ver.dwMinorVersion, //	minorVersion - Identifies the minor version number of the operating system.<nl>
				 ver.dwBuildNumber,  //	buildNumber - Identifies the build number of the operating system in the low-order word. (The high-order word contains the major and minor version numbers.)<nl>
				 ver.dwPlatformId, // platformId - Identifies the platform supported by the operating system.  May be one of VER_PLATFORM_WIN32s, VER_PLATFORM_WIN32_WINDOWS or VER_PLATFORM_WIN32_NT<nl>
				 textVersion); // version - Contains a string that provides arbitrary additional information about the operating system.
	Py_XDECREF(textVersion);
	return rc;
}
%}
%native (CeGetVersionEx) PyCeGetVersionEx;

%{
// @pyswig tuple|CeGlobalMemoryStatus|Returns information about current memory availability.
static PyObject *
PyCeGlobalMemoryStatus(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":PyCeGetGlobalMemoryStatus"))
		return NULL;
	MEMORYSTATUS ms;
	// No return code from fn, so zero memory!
	memset(&ms, 0, sizeof(ms));
	ms.dwLength = sizeof(ms);
	PyW32_BEGIN_ALLOW_THREADS
	CeGlobalMemoryStatus(&ms);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("lllllll",
	// @rdesc The return value is a tuple with the following information.<nl>
				ms.dwMemoryLoad, // MemoryLoad - Specifies a number between 0 and 100 that gives a general idea of current memory utilization, in which 0 indicates no memory use and 100 indicates full memory use. 
				ms.dwTotalPhys, // TotalPhys - Indicates the total number of bytes of physical memory. 
				ms.dwAvailPhys, // AvailPhys - Indicates the number of bytes of physical memory available. 
				ms.dwTotalPageFile, // TotalPageFile - Indicates the total number of bytes that can be stored in the paging file. Note that this number does not represent the actual physical size of the paging file on disk. 
				ms.dwAvailPageFile, // AvailPageFile - Indicates the number of bytes available in the paging file. 
				ms.dwTotalVirtual, // TotalVirtual - Indicates the total number of bytes that can be described in the user mode portion of the virtual address space of the calling process. 
				ms.dwAvailVirtual); // AvailVirtual - Indicates the number of bytes of unreserved and uncommitted memory in the user mode portion of the virtual address space of the calling process. 
}
%}
%native (CeGlobalMemoryStatus) PyCeGlobalMemoryStatus;

%{
// @pyswig list|FindFiles|Retrieves a list of matching filenames on the CE device.  An interface to the API CeFindFirstFile/CeFindNextFile functions.
static PyObject *
PyCeFindFiles(PyObject *self, PyObject *args)
{
	PyObject *obFileSpec;
	// @pyparm <o PyUnicode>|fileSpec||A string that specifies a valid directory or path and filename, which can contain wildcard characters (* and ?).

	if (!PyArg_ParseTuple (args, "O:FindFiles", &obFileSpec))
		return NULL;
	TCHAR *fileSpec;
	if (!PyWinObject_AsTCHAR(obFileSpec, &fileSpec, FALSE))
		return NULL;

	CE_FIND_DATA findData;
	// @pyseeapi CeFindFirstFile
	HANDLE hFind;
	PyW32_BEGIN_ALLOW_THREADS
	hFind =  ::CeFindFirstFile(fileSpec, &findData);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(fileSpec);
	if (hFind==INVALID_HANDLE_VALUE) {
		if (::GetLastError()==ERROR_FILE_NOT_FOUND) {	// this is OK
			return PyList_New(0);
		}
		return PyWin_SetAPIError("CeFindFirstFile", GetLastCEError());
	}
	PyObject *retList = PyList_New(0);
	if (!retList) {
		::CeFindClose(hFind);
		return NULL;
	}
	BOOL ok = TRUE;
	while (ok) {
		PyObject *obCreateTime = PyWinObject_FromFILETIME(findData.ftCreationTime);
		PyObject *obAccessTime = PyWinObject_FromFILETIME(findData.ftLastAccessTime);
		PyObject *obWriteTime = PyWinObject_FromFILETIME(findData.ftLastWriteTime);
		PyObject *obFileName = PyWinObject_FromTCHAR(findData.cFileName);
		if (obCreateTime==NULL || obAccessTime==NULL || obWriteTime==NULL || obFileName==NULL) {
			Py_XDECREF(obCreateTime);
			Py_XDECREF(obAccessTime);
			Py_XDECREF(obWriteTime);
			Py_XDECREF(obFileName);
			Py_DECREF(retList);
			::CeFindClose(hFind);
			return NULL;
		}
		PyObject *newItem = Py_BuildValue("lOOOllllOz",
		// @rdesc The return value is a list of tuples, in the same format as the WIN32_FIND_DATA structure:
			findData.dwFileAttributes, // @tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
			obCreateTime, // @tupleitem 1|<o PyTime>|createTime|File creation time.
    		obAccessTime, // @tupleitem 2|<o PyTime>|accessTime|File access time.
    		obWriteTime, // @tupleitem 3|<o PyTime>|writeTime|Time of last file write
    		findData.nFileSizeHigh, // @tupleitem 4|int|nFileSizeHigh|high order word of file size.
    		findData.nFileSizeLow,	// @tupleitem 5|int|nFileSizeLow|low order word of file size.
    		findData.dwOID,			// @tupleitem 6|int|OID|The object identifier for the file
			0,                      // @tupleitem 7|int|zero|Filler
    		obFileName,		// @tupleitem 8|string|fileName|The name of the file.
    		NULL);		// @tupleitem 9|None|altName|Always None
		if (newItem!=NULL) {
			PyList_Append(retList, newItem); 
			Py_DECREF(newItem);
		}
		// @pyseeapi FindNextFile
		Py_XDECREF(obFileName);
		Py_DECREF(obCreateTime);
		Py_DECREF(obAccessTime);
		Py_DECREF(obWriteTime);
		PyW32_BEGIN_ALLOW_THREADS
		ok=::CeFindNextFile(hFind, &findData);
		PyW32_END_ALLOW_THREADS
	}
	ok = (GetLastCEError()==ERROR_NO_MORE_FILES);
	// @pyseeapi CloseHandle
	::CeFindClose(hFind);
	if (!ok) {
		Py_DECREF(retList);
		return PyWin_SetAPIError("CeFindNextFile", GetLastCEError());
	}
	return retList;
}
%}
%native (CeFindFiles) PyCeFindFiles;

%{
// @pyswig int|CeGetFileAttributes|Determines a files attributes.
static PyObject *
PyCeGetFileAttributes(PyObject * self, PyObject * args)
{
	PyObject *obfname;
	// @pyparm <o PyUnicode>|fileName||Name of the file to retrieve attributes for.
	if (!PyArg_ParseTuple(args, "O:CeGetFileAttributes", &obfname))
		return NULL;
	TCHAR *fname;
	if (!PyWinObject_AsTCHAR(obfname, &fname, FALSE))
		return NULL;

	PyW32_BEGIN_ALLOW_THREADS
	DWORD rc = CeGetFileAttributes(fname);
	PyW32_END_ALLOW_THREADS

	PyWinObject_FreeTCHAR(fname);
	if (rc==(DWORD)-1)
		return PyWin_SetAPIError("CeGetFileAttributes", GetLastCEError());

	return PyInt_FromLong(rc);
}
%}
%native (CeGetFileAttributes) PyCeGetFileAttributes;

// @pyswig |CeSetFileAttributes|Changes a file's attributes.
BOOLCEAPI CeSetFileAttributes(
    TCHAR *lpFileName,	// @pyparm <o PyUnicode>|filename||filename 
    DWORD dwFileAttributes 	// @pyparm int|newAttributes||attributes to set 
);	

%{
static PyObject *PyCeGetFileSize(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	if (!PyArg_ParseTuple(args, "O", &obHandle))
		return NULL;
	HANDLE hFile;
	if (!PyWinObject_AsCEHANDLE(obHandle, &hFile, FALSE))
		return NULL;
	DWORD dwSizeLow=0, dwSizeHigh=0;
    Py_BEGIN_ALLOW_THREADS
	dwSizeLow = CeGetFileSize (hFile, &dwSizeHigh);
    Py_END_ALLOW_THREADS
	// If we failed ... 
	if (dwSizeLow == 0xFFFFFFFF && 
	    GetLastCEError() != NO_ERROR )
		return PyWin_SetAPIError("GetFileSize");
	return PyLong_FromTwoInts(dwSizeHigh, dwSizeLow);
}

%}
// @pyswig <o PyLARGE_INTEGER>|CeGetFileSize|Determines the size of a file.
%native(CeGetFileSize) PyCeGetFileSize;

// @pyswig string|CeReadFile|Reads a file from the CE device.
%{
PyObject *PyCeReadFile(PyObject *self, PyObject *args)
{
	PyObject *obhFile;
	PyObject *obOverlapped;
	HANDLE hFile;
	DWORD bufSize;

	if (!PyArg_ParseTuple(args, "Ol|O:CeReadFile", 
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		&bufSize, // @pyparm int|bufSize||Size of the buffer to create for the read.
		&obOverlapped))
		return NULL;
	if (!PyWinObject_AsCEHANDLE(obhFile, &hFile, FALSE))
		return NULL;

	void *buf = malloc(bufSize);
	if (buf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating read buffer");
		return NULL;
	}
	DWORD numRead;
	BOOL ok;
    Py_BEGIN_ALLOW_THREADS
	ok = CeReadFile(hFile, buf, bufSize, &numRead, NULL);
    Py_END_ALLOW_THREADS
	if (!ok) {
		free(buf);
		return PyWin_SetAPIError("CeReadFile", GetLastCEError());
	}
	return PyString_FromStringAndSize((char *)buf, numRead);
}
%}
%native (CeReadFile) PyCeReadFile;


%{
// @pyswig int, int|WriteFile|Writes a string to a file
// @rdesc The result is a tuple of (errCode, nBytesWritten).
// errCode will always be zero (until overlapped IO is supported!)
PyObject *PyCeWriteFile(PyObject *self, PyObject *args)
{
	char *writeData;
	int dataSize;
	PyObject *obOverlapped;
	PyObject *obhFile;
	if (!PyArg_ParseTuple(args, "Os#|O:CeWriteFile",
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		&writeData, // @pyparm string|data||The data to write.
		&dataSize,
		&obOverlapped))	
		return NULL;
	HANDLE hFile;
	if (!PyWinObject_AsCEHANDLE(obhFile, &hFile, FALSE))
		return NULL;
	DWORD numWritten;
	BOOL ok;
    Py_BEGIN_ALLOW_THREADS
	ok = CeWriteFile(hFile, writeData, dataSize, &numWritten, NULL);
    Py_END_ALLOW_THREADS
	DWORD err = GetLastError();
	if (!ok) {
		return PyWin_SetAPIError("CeWriteFile", GetLastCEError());
	}
	return Py_BuildValue("ll", err, numWritten);
}
%}
%native (CeWriteFile) PyCeWriteFile;

/////////////////////////////////////////////////////////////////////////
//
// Support for a remote handle.
//
%{
#undef PyHANDLE
#undef PyCEHANDLE
#include "PyWinObjects.h"

class PyCEHANDLE : public PyHANDLE
{
public:
	PyCEHANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void);
	virtual const char *GetTypeName() {return "PyCEHANDLE";}
};

// A CE Remote handle.
// @object PyCEHANDLE|A Python object, representing a remote Windows CE handle
BOOL PyWinObject_AsCEHANDLE(PyObject *ob, HANDLE *pRes, BOOL bNoneOK)
{
	return PyWinObject_AsHANDLE(ob, pRes, bNoneOK);
}
PyObject *PyWinObject_FromCEHANDLE(HANDLE h)
{
	return new PyCEHANDLE(h);
}
// @pymethod <o PyCEHANDLE>|wincerapi|CEHANDLE|Creates a new CEHANDLE object
PyObject *PyWinMethod_NewCEHANDLE(PyObject *self, PyObject *args)
{
	HANDLE hInit;
	if (!PyArg_ParseTuple(args, "|i:CEHANDLE", &hInit))
		return NULL;
	return new PyCEHANDLE(hInit);
}

BOOL PyWinObject_CloseCEHANDLE(PyObject *obHandle)
{
	BOOL ok;
	if (PyHANDLE_Check(obHandle))
		ok = ((PyCEHANDLE *)obHandle)->Close();
	else if PyInt_Check(obHandle) {
		PyW32_BEGIN_ALLOW_THREADS
		long rc = ::CeCloseHandle((HANDLE)PyInt_AsLong(obHandle));
		PyW32_END_ALLOW_THREADS
		ok = (rc==ERROR_SUCCESS);
		if (!ok)
			PyWin_SetAPIError("CeCloseHandle", rc);
	} else {
		PyErr_SetString(PyExc_TypeError, "A handle must be a CEHANDLE object or an integer");
		return FALSE;
	}
	return ok;
}

// The non-static member functions
BOOL PyCEHANDLE::Close(void)
{
	BOOL ok = m_handle ? CeCloseHandle((HANDLE)m_handle) : TRUE;
	m_handle = 0;
	if (!ok)
		PyWin_SetAPIError("CeCloseHandle", GetLastCEError());
	return ok;
}

// Redefine them
#define PyHANDLE HANDLE
#define PyCEHANDLE HANDLE

%}

//  End of CEHANDLE support.
//

// Registry stuff externally implemented.
%{
extern PyObject *PyCeRegCreateKeyEx(PyObject *self, PyObject *args);
extern PyObject *PyCeRegDeleteKey(PyObject *self, PyObject *args);
extern PyObject *PyCeRegDeleteValue(PyObject *self, PyObject *args);
extern PyObject *PyCeRegEnumKeyEx(PyObject *self, PyObject *args);
extern PyObject *PyCeRegEnumValue(PyObject *self, PyObject *args);
extern PyObject *PyCeRegOpenKeyEx(PyObject *self, PyObject *args);
extern PyObject *PyCeRegQueryInfoKey(PyObject *self, PyObject *args);
extern PyObject *PyCeRegQueryValueEx(PyObject *self, PyObject *args);
extern PyObject *PyCeRegSetValueEx(PyObject *self, PyObject *args);
extern PyObject *PyWinMethod_NewCEHKEY(PyObject *self, PyObject *args);
%}

%native(CeRegCreateKeyEx) PyCeRegCreateKeyEx;
%native (CeRegDeleteKey) PyCeRegDeleteKey;
%native (CeRegDeleteValue) PyCeRegDeleteValue;
%native (CeRegEnumKeyEx) PyCeRegEnumKeyEx;
%native (CeRegEnumValue) PyCeRegEnumValue;
%native (CeRegOpenKeyEx) PyCeRegOpenKeyEx;
%native (CeRegQueryInfoKey) PyCeRegQueryInfoKey;
%native (CeRegQueryValueEx) PyCeRegQueryValueEx;
%native (CeRegSetValueEx) PyCeRegSetValueEx;
%native (CEHKEY) PyWinMethod_NewCEHKEY;
////////////////////////////////////////////////////////////////////////
#define CSIDL_BITBUCKET CSIDL_BITBUCKET 
// Recycle bin-file system directory containing file objects in the user's recycle bin. The location of this directory is not in the registry; it is marked with the hidden and system attributes to prevent the user from moving or deleting it. 
#define CSIDL_COMMON_DESKTOPDIRECTORY CSIDL_COMMON_DESKTOPDIRECTORY
// File system directory that contains files and folders that appear on the desktop for all users. 
#define CSIDL_COMMON_PROGRAMS CSIDL_COMMON_PROGRAMS 
// File system directory that contains the directories for the common program groups that appear on the Start menu for all users. 
#define CSIDL_COMMON_STARTMENU CSIDL_COMMON_STARTMENU 
// File system directory that contains the programs and folders that appear on the Start menu for all users. 
#define CSIDL_COMMON_STARTUP CSIDL_COMMON_STARTUP 
// File system directory that contains the programs that appear in the Startup folder for all users. The system starts these programs whenever any user logs on to a Windows desktop platform. 
#define CSIDL_CONTROLS CSIDL_CONTROLS 
// Control Panel-virtual folder containing icons for the control panel applications. 
#define CSIDL_DESKTOP CSIDL_DESKTOP 
// Windows desktop-virtual folder at the root of the name space. 
#define CSIDL_DESKTOPDIRECTORY CSIDL_DESKTOPDIRECTORY 
// File system directory used to physically store file objects on the desktop - not to be confused with the desktop folder itself. 
#define CSIDL_DRIVES CSIDL_DRIVES 
// My Computer-virtual folder containing everything on the local computer: storage devices, printers, and Control Panel. The folder can also contain mapped network drives. 
#define CSIDL_FONTS CSIDL_FONTS 
// Virtual folder containing fonts. 
#define CSIDL_NETHOOD CSIDL_NETHOOD 
// File system directory containing objects that appear in the network neighborhood. 
#define CSIDL_NETWORK CSIDL_NETWORK 
// Network Neighborhood-virtual folder representing the top level of the network hierarchy. 
#define CSIDL_PERSONAL CSIDL_PERSONAL 
// File system directory that serves as a common repository for documents. 
#define CSIDL_PRINTERS CSIDL_PRINTERS 
// Printers folder-virtual folder containing installed printers. 
#define CSIDL_PROGRAMS CSIDL_PROGRAMS 
// File system directory that contains the user's program groups which are also file system directories. 
#define CSIDL_RECENT CSIDL_RECENT 
// File system directory containing the user's most recently used documents. 
#define CSIDL_SENDTO CSIDL_SENDTO 
// File system directory containing Send To menu items. 
#define CSIDL_STARTMENU CSIDL_STARTMENU 
// File system directory containing Start menu items. 
#define CSIDL_STARTUP CSIDL_STARTUP 
// File system directory that corresponds to the user's Startup program group. 
#define CSIDL_TEMPLATES CSIDL_TEMPLATES 
// File system directory that serves as a common repository for document templates. 
