/***********************************************************

win32apimodule.cpp -- module for interface into Win32' API


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "win32api_display.h"
#include "malloc.h"

#include "math.h" // for some of the date stuff...

#define SECURITY_WIN32 // required by below
#include "security.h"  // for GetUserNameEx

// Identical to PyW32_BEGIN_ALLOW_THREADS except no script "{" !!!
// means variables can be declared between the blocks
#define PyW32_BEGIN_ALLOW_THREADS PyThreadState *_save = PyEval_SaveThread();
#define PyW32_END_ALLOW_THREADS PyEval_RestoreThread(_save);
#define PyW32_BLOCK_THREADS Py_BLOCK_THREADS

#if (_WIN32_WINNT < 0x0500)
// We don't get COMPUTER_NAME_FORMAT unless we bump this.
// As we use it dynamically, we don't *need* to bump it.
typedef int COMPUTER_NAME_FORMAT;
#endif

// from kernel32.dll
typedef BOOL (WINAPI *GetComputerNameExfunc)(COMPUTER_NAME_FORMAT,LPWSTR,PULONG);
static GetComputerNameExfunc pfnGetComputerNameEx=NULL;
typedef DWORD (WINAPI *GetLongPathNameAfunc)(LPCSTR, LPSTR, DWORD);
static GetLongPathNameAfunc pfnGetLongPathNameA =NULL;
typedef DWORD (WINAPI *GetLongPathNameWfunc)(LPCWSTR, LPWSTR, DWORD);
static GetLongPathNameWfunc pfnGetLongPathNameW=NULL;
typedef BOOL (WINAPI *GetHandleInformationfunc)(HANDLE, LPDWORD);
static GetHandleInformationfunc pfnGetHandleInformation=NULL;
typedef BOOL (WINAPI *SetHandleInformationfunc)(HANDLE, DWORD, DWORD);
static SetHandleInformationfunc pfnSetHandleInformation=NULL;

// from secur32.dll
typedef BOOLEAN (WINAPI *GetUserNameExfunc)(EXTENDED_NAME_FORMAT,LPWSTR,PULONG);
static GetUserNameExfunc pfnGetUserNameEx=NULL;
static GetUserNameExfunc pfnGetComputerObjectName=NULL;


/* error helper */
PyObject *ReturnError(char *msg, char *fnName = NULL)
{
	PyObject *v = Py_BuildValue("(izs)", 0, fnName, msg);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
	return NULL;
}
/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *ReturnAPIError(char *fnName, long err = 0)
{
	return PyWin_SetAPIError(fnName, err);
}
// @pymethod |win32api|Beep|Generates simple tones on the speaker.
static PyObject *
PyBeep( PyObject *self, PyObject *args )
{
	DWORD freq;
	DWORD dur;

	if (!PyArg_ParseTuple(args, "ii:Beep", 
	          &freq,  // @pyparm int|freq||Specifies the frequency, in hertz, of the sound. This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
	          &dur)) // @pyparm int|dur||Specifies the duration, in milliseconds, of the sound.~
	                // One value has a special meaning: If dwDuration is  - 1, the function 
	                // operates asynchronously and produces sound until called again. 
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::Beep(freq,dur);
	PyW32_END_ALLOW_THREADS
	if (!ok) // @pyseeapi Beep
		return ReturnAPIError("Beep");		
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|GetStdHandle|Returns a handle for the standard input, standard output, or standard error device
static PyObject* PyGetStdHandle (PyObject *self, PyObject *args)
{
  DWORD nStdHandle;

  if (!PyArg_ParseTuple(args, "i:GetStdHandle",
			&nStdHandle)) // @pyparm int|handle||input, output, or error device
    return NULL;
  return Py_BuildValue("i", ::GetStdHandle (nStdHandle));
}

// @pymethod |win32api|SetStdHandle|Set the handle for the standard input, standard output, or standard error device
static PyObject* PySetStdHandle (PyObject *self, PyObject *args)
{
  DWORD nStdHandle;
  HANDLE hHandle;
  PyObject *obHandle;

  if (!PyArg_ParseTuple(args, "iO:SetStdHandle",
			&nStdHandle, // @pyparm int|handle||input, output, or error device
			&obHandle)) // @pyparm <o PyHANDLE>/int|handle||A previously opened handle to be a standard handle
    return NULL;

  if (!PyWinObject_AsHANDLE(obHandle, &hHandle))
    return NULL;

  if (!::SetStdHandle(nStdHandle, hHandle))
    return ReturnAPIError ("SetStdHandle");

  Py_INCREF(Py_None);
  return Py_None;
}

// @pymethod |win32api|CloseHandle|Closes an open handle.
static PyObject *PyCloseHandle(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	if (!PyArg_ParseTuple(args, "O:CloseHandle",
			&obHandle)) // @pyparm <o PyHANDLE>/int|handle||A previously opened handle.
		return NULL;
	if (!PyWinObject_CloseHANDLE(obHandle))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyHANDLE>|win32api|DuplicateHandle|Duplicates a handle.
static PyObject *PyDuplicateHandle(PyObject *self, PyObject *args)
{
	HANDLE hSourceProcess, hSource, hTarget, hResult;
	PyObject *obSourceProcess, *obSource, *obTarget;
	BOOL bInherit;
	DWORD options, access;
	if (!PyArg_ParseTuple(args, "OOOiii:DuplicateHandle",
			&obSourceProcess, // @pyparm <o PyHANDLE>|hSourceProcess||Identifies the process containing the handle to duplicate.
			&obSource, // @pyparm <o PyHANDLE>|hSource||Identifies the handle to duplicate. This is an open object handle that is valid in the context of the source process.
			&obTarget, // @pyparm <o PyHANDLE>|hTargetProcessHandle||Identifies the process that is to receive the duplicated handle. The handle must have PROCESS_DUP_HANDLE access. 
			&access, // @pyparm int|desiredAccess||Specifies the access requested for the new handle. This parameter is ignored if the dwOptions parameter specifies the DUPLICATE_SAME_ACCESS flag. Otherwise, the flags that can be specified depend on the type of object whose handle is being duplicated. For the flags that can be specified for each object type, see the following Remarks section. Note that the new handle can have more access than the original handle. 
			&bInherit, // @pyparm int|bInheritHandle||Indicates whether the handle is inheritable. If TRUE, the duplicate handle can be inherited by new processes created by the target process. If FALSE, the new handle cannot be inherited. 
			&options)) // @pyparm int|options||Specifies optional actions. This parameter can be zero, or any combination of the following flags
			// @flag DUPLICATE_CLOSE_SOURCE|loses the source handle. This occurs regardless of any error status returned.
			// @flag DUPLICATE_SAME_ACCESS|Ignores the dwDesiredAccess parameter. The duplicate handle has the same access as the source handle.
		return NULL;
	if (!PyWinObject_AsHANDLE(obSourceProcess, &hSourceProcess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obSource, &hSource))
		return NULL;
	if (!PyWinObject_AsHANDLE(obTarget, &hTarget))
		return NULL;
	if (!DuplicateHandle(hSourceProcess, hSource, hTarget, &hResult, access, bInherit, options))
		return ReturnAPIError("DuplicateHandle");
	return PyWinObject_FromHANDLE(hResult);
}

// @pymethod int|win32api|GetHandleInformation|Retrieves a handle's flags.
// @comm Not available on Win98/Me
// @rdesc Returns a combination of HANDLE_FLAG_INHERIT, HANDLE_FLAG_PROTECT_FROM_CLOSE
static PyObject *PyGetHandleInformation(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetHandleInformation);
	PyObject *obObject;
	HANDLE h;
	DWORD Flags;
	if (!PyArg_ParseTuple(args, "O:GetHandleInformation", 
		&obObject))  // @pyparm <o PyHANDLE>|Object||Handle to an object
		return NULL;
	if (!PyWinObject_AsHANDLE(obObject, &h))
		return NULL;
	if (!(*pfnGetHandleInformation)(h, &Flags))
		return PyWin_SetAPIError("GetHandleInformation");
	return PyLong_FromUnsignedLong(Flags);

}

// @pymethod |win32api|SetHandleInformation|Sets a handles's flags
// @comm Not available on Win98/Me
static PyObject *PySetHandleInformation(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetHandleInformation);
	PyObject *obObject;
	HANDLE h;
	DWORD Mask, Flags;
	if (!PyArg_ParseTuple(args, "Okk:SetHandleInformation", 
		&obObject,	// @pyparm <o PyHANDLE>|Object||Handle to an object
		&Mask,		// @pyparm int|Mask||Bitmask specifying which flags should be set
		&Flags))	// @pyparm int|Flags||Bitmask of flag values to be set. Valid Flags are HANDLE_FLAG_INHERIT, HANDLE_FLAG_PROTECT_FROM_CLOSE
		return NULL;
	if (!PyWinObject_AsHANDLE(obObject, &h))
		return NULL;
	if (!(*pfnSetHandleInformation)(h, Mask, Flags))
		return PyWin_SetAPIError("SetHandleInformation");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|CopyFile|Copies an existing file to a new file
static PyObject *
PyCopyFile( PyObject *self, PyObject *args )
{
	BOOL failOnExist = FALSE;
	PyObject *obSrc, *obDest;
	if (!PyArg_ParseTuple(args, "OO|i:CopyFile", 
	          &obSrc,  // @pyparm string<o PyUnicode>|src||Name of an existing file.
	          &obDest, // @pyparm string/<o PyUnicode>|dest||Name of file to copy to.
	          &failOnExist)) // @pyparm int|bFailOnExist|0|Indicates if the operation should fail if the file exists.
		return NULL;
	char *src, *dest;
	if (!PyWinObject_AsTCHAR(obSrc, &src, FALSE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obDest, &dest, FALSE)) {
		PyWinObject_FreeTCHAR(src);
		return NULL;
	}
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::CopyFile(src, dest, failOnExist);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(src);
	PyWinObject_FreeTCHAR(dest);
	if (!ok) // @pyseeapi CopyFile
		return ReturnAPIError("CopyFile");		
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|DebugBreak|Breaks into the C debugger
static PyObject *
PyDebugBreak(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":DebugBreak"))
		return NULL;
	// @pyseeapi DebugBreak
	PyW32_BEGIN_ALLOW_THREADS
	DebugBreak();
	PyW32_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|DeleteFile|Deletes the specified file.
static PyObject *
PyDeleteFile(PyObject * self, PyObject * args)
{
	PyObject *obPath;
	// @pyparm string/<o PyUnicode>|fileName||File to delete.
	if (!PyArg_ParseTuple(args, "O:DeleteFile", &obPath))
		return NULL;
	TCHAR *szPath;
	if (!PyWinObject_AsTCHAR(obPath, &szPath, FALSE))
		return NULL;
	// @pyseeapi DeleteFile
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = DeleteFile(szPath);
	PyW32_END_ALLOW_THREADS
		PyWinObject_FreeTCHAR(szPath);
	if (!ok)
		return ReturnAPIError("DeleteFile");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod string/int|win32api|DragQueryFile|Retrieves the file names of dropped files.
static PyObject *
PyDragQueryFile( PyObject *self, PyObject *args )
{
	char buf[MAX_PATH];
	HDROP hDrop;
	int iFileNum = 0xFFFFFFFF;
	if (!PyArg_ParseTuple(args, "i|i:DragQueryFile", 
	           &hDrop, // @pyparm int|hDrop||Handle identifying the structure containing the file names.
	           &iFileNum)) // @pyparm int|fileNum|0xFFFFFFFF|Specifies the index of the file to query.
		return NULL;
	if (iFileNum<0)
		return Py_BuildValue("i", ::DragQueryFile( hDrop, (UINT)-1, NULL, 0));
	else { // @pyseeapi DragQueryFile
		PyW32_BEGIN_ALLOW_THREADS
		int ret = ::DragQueryFile( hDrop, iFileNum, buf, sizeof(buf));
		PyW32_END_ALLOW_THREADS
		if (ret <=0)
			return ReturnAPIError("DragQueryFile");
		else
			return Py_BuildValue("s", buf);
	}
// @rdesc If the fileNum parameter is 0xFFFFFFFF (the default) then the return value
// is an integer with the count of files dropped.  If fileNum is between 0 and Count, 
// the return value is a string containing the filename.<nl>
// If an error occurs, and exception is raised.
}
// @pymethod |win32api|DragFinish|Releases the memory stored by Windows for the filenames.
static PyObject *
PyDragFinish( PyObject *self, PyObject *args )
{
	HDROP hDrop;
	// @pyparm int|hDrop||Handle identifying the structure containing the file names.
	if (!PyArg_ParseTuple(args, "i:DragFinish", &hDrop))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	::DragFinish( hDrop); // @pyseeapi DragFinish
	PyW32_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod string|win32api|GetEnvironmentVariable|Retrieves the value of an environment variable.
static PyObject *
PyGetEnvironmentVariable( PyObject *self, PyObject *args )
{
	char *szVar;
	if (!PyArg_ParseTuple(args, "s:GetEnvironmentVariable", 
	           &szVar)) // @pyparm string|variable||The variable to get
		return NULL;
	// @pyseeapi GetEnvironmentVariable
	PyW32_BEGIN_ALLOW_THREADS
	DWORD size = GetEnvironmentVariable(szVar, NULL, 0);
	char *pResult = NULL;
	if (size) {
		pResult = (char *)malloc(sizeof(char) * size);
		GetEnvironmentVariable(szVar, pResult, size);
	}
	PyW32_END_ALLOW_THREADS
	PyObject *ret;
	if (pResult==NULL) {
		Py_INCREF(Py_None);
		ret = Py_None;
	} else
		ret = PyString_FromString(pResult);
	if (pResult)
		free(pResult);
	return ret;
}

// @pymethod string|win32api|ExpandEnvironmentStrings|Expands environment-variable strings and replaces them with their defined values. 
static PyObject *
PyExpandEnvironmentStrings( PyObject *self, PyObject *args )
{
	char *in;
	if (!PyArg_ParseTuple(args, "s:ExpandEnvironmentStrings", 
	           &in)) // @pyparm string|in||String to expand
		return NULL;
	// @pyseeapi ExpandEnvironmentStrings
	DWORD size;
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	// @comm On Windows 95, the string is limited to 1024 bytes.
	// On other platforms, there is no (practical) limit
	if (osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && osvi.dwMinorVersion==0)
		 /* Default to 1024 for Windows 95, as the size request fails! */
		size = 1024;
	else
		size = ExpandEnvironmentStrings(in, NULL, 0);
	char *result = (char *)malloc(size);
	PyObject *rc;
	PyW32_BEGIN_ALLOW_THREADS
	long lrc = ExpandEnvironmentStrings(in, result, size);
	PyW32_END_ALLOW_THREADS
	if (lrc==0) {
		rc = ReturnAPIError("ExpandEnvironmentStrings");
	} else {
		rc = PyString_FromString(result);
	}
	free(result);
	return rc;
}

// @pymethod (int, string)|win32api|FindExecutable|Retrieves the name and handle of the executable (.EXE) file associated with the specified filename.
static PyObject *
PyFindExecutable( PyObject *self, PyObject *args )
{
	char *file, *dir="";
	char res[MAX_PATH];

	if (!PyArg_ParseTuple(args, "s|s:FindExecutable", 
	           &file, // @pyparm string|filename||A file name.  This can be either a document or executable file.
	           &dir)) // @pyparm string|dir||The default directory.
		return NULL;
	HINSTANCE rc;
	// @pyseeapi FixedExecutable
	PyW32_BEGIN_ALLOW_THREADS
	rc=::FindExecutable(file, dir, res);
	PyW32_END_ALLOW_THREADS
	if (rc<=(HINSTANCE)32) {
		if ((int)rc==31) 
			return ReturnError("There is no association for the file");
		return ReturnAPIError("FindExecutable", (int)rc );
	}
	return Py_BuildValue("(is)", rc, res );
	// @rdesc The return value is a tuple of (integer, string)<nl>
	// The integer is the instance handle of the executable file associated
	// with the specified filename. (This handle could also be the handle of
	// a dynamic data exchange [DDE] server application.)<nl>
	// The may contain the path to the DDE server started if no server responds to a request to initiate a DDE conversation.
	// @comm The function will raise an exception if it fails.
}

// @pymethod list|win32api|FindFiles|Retrieves a list of matching filenames.  An interface to the API FindFirstFile/FindNextFile/Find close functions.
// @rdesc Returns a sequence of <o WIN32_FIND_DATA> tuples
static PyObject *
PyFindFiles(PyObject *self, PyObject *args)
{
	char *fileSpec;
	// @pyparm string|fileSpec||A string that specifies a valid directory or path and filename, which can contain wildcard characters (* and ?).
	if (!PyArg_ParseTuple (args, "s:FindFiles", &fileSpec))
		return NULL;
	WIN32_FIND_DATA findData;
	// @pyseeapi FindFirstFile
	HANDLE hFind;

	hFind =  ::FindFirstFile(fileSpec, &findData);
	if (hFind==INVALID_HANDLE_VALUE) {
		if (::GetLastError()==ERROR_FILE_NOT_FOUND) {	// this is OK
			return PyList_New(0);
		}
		return ReturnAPIError("FindFirstFile");
	}
	PyObject *retList = PyList_New(0);
	if (!retList) {
		::FindClose(hFind);
		return NULL;
	}
	BOOL ok = TRUE;
	while (1) {
		PyObject *newItem = PyObject_FromWIN32_FIND_DATAA(&findData);
		if (newItem==NULL || PyList_Append(retList, newItem)==-1)
			ok=FALSE;
		Py_XDECREF(newItem);
		if (!ok)
			break;
		// @pyseeapi FindNextFile
		if (!FindNextFile(hFind, &findData)){
			ok=(GetLastError()==ERROR_NO_MORE_FILES);
			if (!ok)
				PyWin_SetAPIError("FindNextFile");
			break;
			}
		}

	// @pyseeapi FindClose
	::FindClose(hFind);
	if (!ok) {
		Py_DECREF(retList);
		retList=NULL;
	}
	return retList;
}

// @pymethod int|win32api|FindFirstChangeNotification|Creates a change notification handle and sets up initial change notification filter conditions.
// @rdesc Although the result is a handle, the handle can not be closed via CloseHandle() - therefore a PyHandle object is not used.
static PyObject *
PyFindFirstChangeNotification(PyObject *self, PyObject *args)
{
	DWORD dwFilter;
	BOOL subDirs;
	PyObject *obPathName;
	// @pyparm string|pathName||Specifies the path of the directory to watch. 
	// @pyparm int|bSubDirs||Specifies whether the function will monitor the directory or the directory tree. If this parameter is TRUE, the function monitors the directory tree rooted at the specified directory; if it is FALSE, it monitors only the specified directory
	// @pyparm int|filter||Specifies the filter conditions that satisfy a change notification wait. This parameter can be one or more of the following values:
	// @flagh Value|Meaning
	// @flag FILE_NOTIFY_CHANGE_FILE_NAME|Any file name change in the watched directory or subtree causes a change notification wait operation to return. Changes include renaming, creating, or deleting a file name. 
	// @flag FILE_NOTIFY_CHANGE_DIR_NAME|Any directory-name change in the watched directory or subtree causes a change notification wait operation to return. Changes include creating or deleting a directory. 
	// @flag FILE_NOTIFY_CHANGE_ATTRIBUTES|Any attribute change in the watched directory or subtree causes a change notification wait operation to return. 
	// @flag FILE_NOTIFY_CHANGE_SIZE|Any file-size change in the watched directory or subtree causes a change notification wait operation to return. The operating system detects a change in file size only when the file is written to the disk. For operating systems that use extensive caching, detection occurs only when the cache is sufficiently flushed. 
	// @flag FILE_NOTIFY_CHANGE_LAST_WRITE|Any change to the last write-time of files in the watched directory or subtree causes a change notification wait operation to return. The operating system detects a change to the last write-time only when the file is written to the disk. For operating systems that use extensive caching, detection occurs only when the cache is sufficiently flushed. 
	// @flag FILE_NOTIFY_CHANGE_SECURITY|Any security-descriptor change in the watched directory or subtree causes a change notification wait operation to return 
	if (!PyArg_ParseTuple(args, "Oil", &obPathName, &subDirs, &dwFilter))
		return NULL;
	TCHAR *pathName;
	if (!PyWinObject_AsTCHAR(obPathName, &pathName, FALSE))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	HANDLE h = FindFirstChangeNotification(pathName, subDirs, dwFilter);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(pathName);
	if (h==NULL || h==INVALID_HANDLE_VALUE)
		return ReturnAPIError("FindFirstChangeNotification");
	return PyInt_FromLong((long)h);
}

// @pymethod |win32api|FindNextChangeNotification|Requests that the operating system signal a change notification handle the next time it detects an appropriate change.
static PyObject *
PyFindNextChangeNotification(PyObject *self, PyObject *args)
{
	HANDLE h;
	// @pyparm int|handle||The handle returned from <om win32api.FindFirstChangeNotification>
	if (!PyArg_ParseTuple(args, "l", &h))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = FindNextChangeNotification(h);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("FindNextChangeNotification");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|FindCloseChangeNotification|Closes the change notification handle.
static PyObject *
PyFindCloseChangeNotification(PyObject *self, PyObject *args)
{
	HANDLE h;
	// @pyparm int|handle||The handle returned from <om win32api.FindFirstChangeNotification>
	if (!PyArg_ParseTuple(args, "l", &h))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = FindCloseChangeNotification(h);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("FindCloseChangeNotification");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod string|win32api|FormatMessage|Returns an error message from the system error file.
static PyObject *
PyFormatMessage (PyObject *self, PyObject *args)
{
	int errCode=0;
	// @pyparm int|errCode|0|The error code to return the message for,  If this value is 0, then GetLastError() is called to determine the error code.
	if (PyArg_ParseTuple (args, "|k:FormatMessage", &errCode)) {
		if (errCode==0)
			// @pyseeapi GetLastError
			errCode = GetLastError();
		const int bufSize = 512;
		char buf[bufSize];
		DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		HMODULE hmodule = PyWin_GetErrorMessageModule(errCode);
		if (hmodule)
			flags |= FORMAT_MESSAGE_FROM_HMODULE;
		// @pyseeapi FormatMessage
		if (::FormatMessage(flags, hmodule, errCode, 0, buf, bufSize, NULL )<=0)
			return ReturnAPIError("FormatMessage");
		return Py_BuildValue("s", buf);
	}
	PyErr_Clear();
	// Support for "full" argument list
	//
	// @pyparmalt1 int|flags||Flags for the call.  Note that FORMAT_MESSAGE_ALLOCATE_BUFFER and FORMAT_MESSAGE_ARGUMENT_ARRAY will always be added.
	// @pyparmalt1 int/string|source||The source object.  If flags contain FORMAT_MESSAGE_FROM_HMODULE it should be an int, if flags contain FORMAT_MESSAGE_FROM_STRING, otherwise it is ignored.
	// @pyparmalt1 int|messageId||The message ID.
	// @pyparmalt1 int|languageID||The language ID.
	// @pyparmalt1 [string,...]/None|inserts||The string inserts to insert.
	// @pyparmalt1 int|bufSize|1024|
	DWORD  flags, msgId, langId;
	PyObject *obSource;
	PyObject *obInserts;
	HANDLE hSource;
	char *szSource;
	char **pInserts;
	void *pSource;
	if (!PyArg_ParseTuple (args, "iOiiO:FormatMessage", &flags, &obSource, &msgId, &langId, &obInserts))
		return NULL;
	if (flags & FORMAT_MESSAGE_FROM_HMODULE) {
		if (!PyInt_Check(obSource)) {
			PyErr_SetString(PyExc_TypeError, "Flags has FORMAT_MESSAGE_FROM_HMODULE, but object not an integer");
			return NULL;
		}
		hSource = (HANDLE)PyInt_AsLong(obSource);
		pSource = (void *)hSource;
	}
	else if (flags & FORMAT_MESSAGE_FROM_STRING) {
		if (!PyString_Check(obSource)) {
			PyErr_SetString(PyExc_TypeError, "Flags has FORMAT_MESSAGE_FROM_STRING, but object not a string");
			return NULL;
		}
		szSource = PyString_AsString(obSource);
		pSource = (void *)szSource;
	} else
		pSource = NULL;
	if (obInserts==NULL || obInserts==Py_None) {
		pInserts = NULL;
	} else if (PySequence_Check(obInserts)) {
		int len = PySequence_Length(obInserts);
		pInserts = (char **)malloc(sizeof(char *) * (len+1));
		if (pInserts==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating buffer for inserts");
			return NULL;
		}
		int i;
		for (i=0;i<len;i++) {
			PyObject *subObject = PySequence_GetItem(obInserts, i);
			if (subObject==NULL) {
				free(pInserts);
				return NULL;
			}
			if (!PyString_Check(subObject)) {
				free(pInserts);
				PyErr_SetString(PyExc_TypeError, "Inserts must be sequence of strings");
				return NULL;
			}
			if ((pInserts[i] = PyString_AsString(subObject))==NULL) {
				free(pInserts);
				return NULL;
			}
			Py_DECREF(subObject);
		}
		pInserts[i] = NULL;
	} else {
			PyErr_SetString(PyExc_TypeError, "Inserts must be sequence or None");
			return NULL;
	}
	char *buf;
	flags |= (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY);
	PyW32_BEGIN_ALLOW_THREADS
	long lrc = ::FormatMessage(flags, pSource, msgId, langId, (LPTSTR)&buf, 0, pInserts );
	PyW32_END_ALLOW_THREADS
	if (lrc<=0)
			return ReturnAPIError("FormatMessage");
	PyObject *rc = PyString_FromString(buf);
	LocalFree(buf);
	if (pInserts)
		free(pInserts);
	return rc;
}

// @pymethod <o PyUnicode>|win32api|FormatMessageW|Returns an error message from the system error file.
static PyObject *
PyFormatMessageW(PyObject *self, PyObject *args)
{
	// Only support for "full" argument list
	//
	// @pyparmalt1 int|flags||Flags for the call.  Note that FORMAT_MESSAGE_ALLOCATE_BUFFER and FORMAT_MESSAGE_ARGUMENT_ARRAY will always be added.
	// @pyparmalt1 int/<o PyUnicode>|source||The source object.  If flags contain FORMAT_MESSAGE_FROM_HMODULE it should be an int, if flags contain FORMAT_MESSAGE_FROM_STRING, otherwise it is ignored.
	// @pyparmalt1 int|messageId||The message ID.
	// @pyparmalt1 int|languageID||The language ID.
	// @pyparmalt1 [<o PyUnicode>,...]/None|inserts||The string inserts to insert.
	// @pyparmalt1 int|bufSize|1024|
	DWORD  flags, msgId, langId;
	PyObject *obSource;
	PyObject *obInserts;
	HANDLE hSource;
	WCHAR *szSource = NULL;
	WCHAR **pInserts = NULL;
	int numInserts = 0;
	void *pSource;
	PyObject *rc = NULL;
	WCHAR *resultBuf;
	int i;
	long lrc;
	if (!PyArg_ParseTuple (args, "iOiiO:FormatMessageW", &flags, &obSource, &msgId, &langId, &obInserts))
		goto cleanup;
	if (flags & FORMAT_MESSAGE_FROM_HMODULE) {
		if (!PyInt_Check(obSource)) {
			PyErr_SetString(PyExc_TypeError, "Flags has FORMAT_MESSAGE_FROM_HMODULE, but object not an integer");
			goto cleanup;
		}
		hSource = (HANDLE)PyInt_AsLong(obSource);
		pSource = (void *)hSource;
	}
	else if (flags & FORMAT_MESSAGE_FROM_STRING) {
		if (!PyWinObject_AsWCHAR(obSource, &szSource))
			goto cleanup;
		pSource = (void *)szSource;
	} else
		pSource = NULL;
	if (obInserts==NULL || obInserts==Py_None) {
		; // do nothing - already NULL
	} else if (PySequence_Check(obInserts)) {
		numInserts = PySequence_Length(obInserts);
		pInserts = (WCHAR **)malloc(sizeof(WCHAR *) * (numInserts+1));
		if (pInserts==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating buffer for inserts");
			goto cleanup;
		}
		for (i=0;i<numInserts;i++)	// Make sure clean for cleanup
			pInserts[i] = NULL;
		for (i=0;i<numInserts;i++) {
			PyObject *subObject = PySequence_GetItem(obInserts, i);
			if (subObject==NULL) {
				goto cleanup;
			}
			if (!PyUnicode_Check(subObject)) {
				PyErr_SetString(PyExc_TypeError, "Inserts must be sequence of UnicodeObjects");
				goto cleanup;
			}
			if (!PyWinObject_AsWCHAR(subObject, pInserts+i)) {
				goto cleanup;
			}
			Py_DECREF(subObject);
		}
		pInserts[i] = NULL;	// One beyond end - seems necessary if inserts dont match. 
	} else {
			PyErr_SetString(PyExc_TypeError, "Inserts must be sequence or None");
			goto cleanup;
	}
	flags |= (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY);
	{
	PyW32_BEGIN_ALLOW_THREADS
	lrc = ::FormatMessageW(flags, pSource, msgId, langId, (LPWSTR)&resultBuf, 0, (va_list *)pInserts );
	PyW32_END_ALLOW_THREADS
	}
	if (lrc<=0) {
			ReturnAPIError("FormatMessage");
			goto cleanup;
	}
	rc = PyWinObject_FromWCHAR(resultBuf);
cleanup:
	if (pInserts) {
		for (i=0;i<numInserts;i++)
			PyWinObject_FreeWCHAR(pInserts[i]);
		free(pInserts);
	}
	PyWinObject_FreeWCHAR(szSource);
	if (resultBuf)
		LocalFree(resultBuf);
	return rc;
}

#ifndef DONT_HAVE_GENERATE_CONSOLE_CTRL_EVENT
// @pymethod int|win32api|GenerateConsoleCtrlEvent|Send a specified signal to a console process group that shares the console associated with the calling process.
static PyObject *
PyGenerateConsoleCtrlEvent (PyObject *self, PyObject *args)
{
	DWORD dwControlEvent, dwProcessGroupId;
	if (!PyArg_ParseTuple(args,"ll:GenerateConsoleCtrlEvent",
		&dwControlEvent, 	// @pyparm int|controlEvent||Signal to generate.
		&dwProcessGroupId)) 	// @pyparm int|processGroupId||Process group to get signal.
		return NULL;
	// @pyseeapi GenerateConsoleCtrlEvent
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = GenerateConsoleCtrlEvent(dwControlEvent, dwProcessGroupId);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GenerateConsoleCtrlEvent");
	Py_INCREF(Py_None);
	return Py_None;
}
#endif // DONT_HAVE_GENERATE_CONSOLE_CTRL_EVENT

// @pymethod int|win32api|GetLogicalDrives|Returns a bitmask representing the currently available disk drives.
static PyObject *
PyGetLogicalDrives (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetLogicalDrives"))
		return NULL;
	// @pyseeapi GetLogicalDrives
	DWORD rc = GetLogicalDrives();
	if (rc==0)
		return ReturnAPIError("GetLogicalDrives");
	return PyInt_FromLong(rc);
}

// @pymethod string|win32api|GetConsoleTitle|Returns the title for the current console.
static PyObject *
PyGetConsoleTitle (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetConsoleTitle"))
		return NULL;
	char title[128] = "";
	// @pyseeapi GetConsoleTitle
	::SetLastError(0); // sigh - stale errors can hang around.
	if (GetConsoleTitle(title, sizeof(title))==0 && ::GetLastError() != 0)
		return ReturnAPIError("GetConsoleTitle");
	return Py_BuildValue("s", title);
	// @rdesc The title for the current console window.  This function will
	// raise an exception if the current application does not have a console.
}


// @pymethod string|win32api|GetComputerName|Returns the local computer name
static PyObject *
PyGetComputerName (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetComputerName"))
		return NULL;
	// @pyseeapi GetComputerName
	char buf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(buf);
	if (GetComputerName(buf, &size)==0)
		return ReturnAPIError("GetComputerName");
	return Py_BuildValue("s", buf);
}

// @pymethod string|win32api|GetComputerNameEx|Retrieves a NetBIOS or DNS name associated with the local computer
static PyObject *
PyGetComputerNameEx(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetComputerNameEx);
	WCHAR *formattedname=NULL;
	COMPUTER_NAME_FORMAT fmt;
	PyObject *ret = NULL;
	ULONG nSize=0;
	BOOL ok;
	// @pyseeapi GetComputerNameEx
	if (!PyArg_ParseTuple (args, "i:GetComputerNameEx",
		&fmt))	// @pyparm int|NameType||Value from COMPUTER_NAME_FORMAT enum, win32con.ComputerName*
		return NULL;

	// We always get into trouble with WinXP vs 2k error codes.
	// Simply assume that if we have a size, the function gave us the correct one.
	(*pfnGetComputerNameEx)(fmt,formattedname,&nSize);
	if (!nSize)
		return PyWin_SetAPIError("GetComputerNameExW");
	formattedname=(WCHAR *)malloc(nSize*sizeof(WCHAR));
	if (!formattedname)
		return PyErr_NoMemory();
	PyW32_BEGIN_ALLOW_THREADS
	ok = (*pfnGetComputerNameEx)(fmt,formattedname,&nSize);
	PyW32_END_ALLOW_THREADS
	if (!ok){
		PyWin_SetAPIError("GetComputerNameEx");
		goto done;
	}
	ret=PyWinObject_FromWCHAR(formattedname);
	done:
	if (formattedname!=NULL)
		free(formattedname);
	return ret;
}

// @pymethod string|win32api|GetComputerObjectName|Retrieves the local computer's name in a specified format.
static PyObject *
PyGetComputerObjectName(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetComputerObjectName);
	WCHAR *formattedname=NULL;
	EXTENDED_NAME_FORMAT fmt;
	PyObject *ret = NULL;
	ULONG nSize=0;
	BOOL ok;
	// @pyseeapi GetComputerObjectName
	if (!PyArg_ParseTuple (args, "i:GetComputerObjectName", 
		&fmt))	// @pyparm int|NameFormat||EXTENDED_NAME_FORMAT value, win32con.Name*
		return NULL;

	// We always get into trouble with WinXP vs 2k error codes.
	// Simply assume that if we have a size, the function gave us the correct one.
	(*pfnGetComputerObjectName)(fmt,formattedname,&nSize);
	if (!nSize)
		return PyWin_SetAPIError("GetComputerObjectName");
	formattedname=(WCHAR *)malloc(nSize*sizeof(WCHAR));
	if (!formattedname)
		return PyErr_NoMemory();
	PyW32_BEGIN_ALLOW_THREADS
	ok = (*pfnGetComputerObjectName)(fmt,formattedname,&nSize);
	PyW32_END_ALLOW_THREADS
	
	if (!ok){
		PyWin_SetAPIError("GetComputerObjectName");
		goto done;
	}
	ret=PyWinObject_FromWCHAR(formattedname);
	done:
	if (formattedname!=NULL)
		free(formattedname);
	return ret;
}

// @pymethod string|win32api|GetUserName|Returns the current user name
static PyObject *
PyGetUserName (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetUserName"))
		return NULL;
	// @pyseeapi GetUserName
	char buf[MAX_PATH + 1];
	DWORD size = sizeof(buf);
	if (GetUserName(buf, &size)==0)
		return ReturnAPIError("GetUserName");
	return Py_BuildValue("s", buf);
}
 
// @pymethod string|win32api|GetUserNameEx|Returns the current user name in format from EXTENDED_NAME_FORMAT enum
static PyObject *
PyGetUserNameEx (PyObject *self, PyObject *args)
{
	CHECK_PFN(GetUserNameEx);
	WCHAR *formattedname=NULL;
	EXTENDED_NAME_FORMAT fmt;
	PyObject *ret = NULL;
	ULONG nSize=0;
	BOOL ok;
	// @pyseeapi GetUserNameEx
	if (!PyArg_ParseTuple (args, "i:GetUserNameEx", 
		&fmt))	// @pyparm int|NameFormat||EXTENDED_NAME_FORMAT value, win32con.Name*
		return NULL;

	// We always get into trouble with WinXP vs 2k error codes.
	// Simply assume that if we have a size, the function gave us the correct one.
	(*pfnGetUserNameEx)(fmt,formattedname,&nSize);
	if (!nSize)
		return PyWin_SetAPIError("GetUserNameExW");
	formattedname=(WCHAR *)malloc(nSize*sizeof(WCHAR));
	if (!formattedname)
		return PyErr_NoMemory();
	PyW32_BEGIN_ALLOW_THREADS
	ok = (*pfnGetUserNameEx)(fmt,formattedname,&nSize);
	PyW32_END_ALLOW_THREADS
	if (!ok){
		PyWin_SetAPIError("GetUserNameEx");
		goto done;
	}
	ret=PyWinObject_FromWCHAR(formattedname);
	done:
	if (formattedname!=NULL)
		free(formattedname);
	return ret;
}

// @pymethod string|win32api|GetDomainName|Returns the current domain name
// @comm This is a convenience wrapper of the Win32 function LookupAccountSid()
static PyObject *
PyGetDomainName (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetUserName"))
		return NULL;
    HANDLE hToken;
    #define MY_BUFSIZE 512  // highly unlikely to exceed 512 bytes
    UCHAR InfoBuffer[ MY_BUFSIZE ];
    DWORD cbInfoBuffer = MY_BUFSIZE;
    SID_NAME_USE snu;

    BOOL bSuccess;

    if(!OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,TRUE,&hToken)) {
        if(GetLastError() == ERROR_NO_TOKEN) {
            // attempt to open the process token, since no thread token
            // exists
            if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
                return ReturnAPIError("OpenProcessToken");
        } else {
        	return ReturnAPIError("OpenThreadToken");
        }
    }
    bSuccess = GetTokenInformation(hToken, TokenUser,
        InfoBuffer,
        cbInfoBuffer,
        &cbInfoBuffer
        );

    CloseHandle(hToken);

    if(!bSuccess)
       	return ReturnAPIError("GetTokenInformation");

	char UserName[256];
	DWORD cchUserName = sizeof(UserName);
	char DomainName[256];
	DWORD cchDomainName = sizeof(DomainName);
    if (!LookupAccountSid(
        NULL,
        ((PTOKEN_USER)InfoBuffer)->User.Sid,
        UserName,
        &cchUserName,
        DomainName,
        &cchDomainName,
        &snu
        ))
       	return ReturnAPIError("LookupAccountSid");
    return PyString_FromString(DomainName);
}

// @pymethod int|win32api|GetCurrentThread|Returns a pseudohandle for the current thread.
static PyObject *
PyGetCurrentThread (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCurrentThread"))
		return NULL;
	// @pyseeapi GetCurrentThread
	return PyInt_FromLong((long)::GetCurrentThread());
	// @comm A pseudohandle is a special constant that is interpreted as the current thread handle. The calling thread can use this handle to specify itself whenever a thread handle is required. Pseudohandles are not inherited by child processes.
	// The method <om win32api.DuplicateHandle> can be used to create a handle that other threads and processes can use.
	// As this handle can not be closed, and integer is returned rather than
	// a <o PyHANDLE> object, which would attempt to automatically close the handle.

}

// @pymethod int|win32api|GetCurrentThreadId|Returns the thread ID for the current thread.
static PyObject *
PyGetCurrentThreadId (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCurrentThreadId"))
		return NULL;
	// @pyseeapi GetCurrentThreadId
	return Py_BuildValue("i", ::GetCurrentThreadId());
}

// @pymethod int|win32api|GetCurrentProcess|Returns a pseudohandle for the current process.
static PyObject *
PyGetCurrentProcess (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCurrentProcess"))
		return NULL;
	// @pyseeapi GetCurrentProcess
	return PyInt_FromLong((long)::GetCurrentProcess());
	// @comm A pseudohandle is a special constant that is interpreted as the current thread handle. The calling thread can use this handle to specify itself whenever a thread handle is required. Pseudohandles are not inherited by child processes.
	// The method <om win32api.DuplicateHandle> can be used to create a handle that other threads and processes can use.
	// As this handle can not be closed, and integer is returned rather than
	// a <o PyHANDLE> object, which would attempt to automatically close the handle.
}

// @pymethod int|win32api|GetCurrentProcessId|Returns the thread ID for the current process.
static PyObject *
PyGetCurrentProcessId (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCurrentProcessId"))
		return NULL;
	// @pyseeapi GetCurrentProcessId
	return Py_BuildValue("i", ::GetCurrentProcessId());
}

// @pymethod int|win32api|GetFocus|Retrieves the handle of the keyboard focus window associated with the thread that called the method. 
static PyObject *
PyGetFocus (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetFocus"))
		return NULL;
	// @pyseeapi GetFocus
	PyW32_BEGIN_ALLOW_THREADS
	HWND rc = GetFocus();
	PyW32_END_ALLOW_THREADS
	if (rc==NULL)
		return ReturnError("No window has the focus");
	return Py_BuildValue("i", rc);
	// @rdesc The method raises an exception if no window with the focus exists.
}

// @pymethod |win32api|ClipCursor|Confines the cursor to a rectangular area on the screen.
static PyObject *
PyClipCursor( PyObject *self, PyObject *args )
{
	RECT r;
	RECT *pRect;
	// @pyparm (int, int, int, int)|left, top, right, bottom||contains the screen coordinates of the upper-left and lower-right corners of the confining rectangle. If this parameter is omitted or (0,0,0,0), the cursor is free to move anywhere on the screen. 
	if (!PyArg_ParseTuple(args, "|(iiii):ClipCursor", &r.left, &r.top, &r.right, &r.bottom))
		return NULL;
	if (r.left == 0 && r.top == 0 && r.right == 0 && r.bottom == 0)
		pRect = NULL;
	else
		pRect = &r;
	// @pyseeapi ClipCursor
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::ClipCursor(pRect);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("ClipCursor");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod int, int|win32api|GetCursorPos|Returns the position of the cursor, in screen co-ordinates.
static PyObject *
PyGetCursorPos (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCursorPos"))
		return NULL;
	POINT pt;
	// @pyseeapi GetCursorPos
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = GetCursorPos(&pt);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GetCursorPos");
	return Py_BuildValue("ii", pt.x, pt.y);
}

// @pymethod int|win32api|SetCursor|Set the cursor to the HCURSOR object.
static PyObject *
PySetCursor( PyObject *self, PyObject *args )
{
	long hCursor;
	if (!PyArg_ParseTuple(args,"l:SetCursor",
		&hCursor)) // @pyparm long|hCursor||The new cursor.
		return NULL;
	// @pyseeapi SetCursor
	PyW32_BEGIN_ALLOW_THREADS
	HCURSOR ret = ::SetCursor((HCURSOR)hCursor); 
	PyW32_END_ALLOW_THREADS
	return PyLong_FromLong((long)ret);
	// @rdesc The result is the previous cursor if there was one.
}

// @pymethod int|win32api|LoadCursor|Loads a cursor.
static PyObject *
PyLoadCursor( PyObject *self, PyObject *args )
{
	long hInstance;
	long id;
	if (!PyArg_ParseTuple(args,"ll:LoadCursor",
		&hInstance, // @pyparm int|hInstance||Handle to the instance to load the resource from.
		&id)) // @pyparm int|cursorid||The ID of the cursor.
		return NULL;
	// @pyseeapi LoadCursor
	PyW32_BEGIN_ALLOW_THREADS
	HCURSOR ret = ::LoadCursor((HINSTANCE)hInstance, MAKEINTRESOURCE(id));
	PyW32_END_ALLOW_THREADS
	if (ret==NULL) ReturnAPIError("LoadCursor");
	return PyLong_FromLong((long)ret);
}

// @pymethod string|win32api|GetCommandLine|Retrieves the current application's command line.
static PyObject *
PyGetCommandLine (PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple (args, ":GetCommandLine"))
		return NULL;
	return Py_BuildValue("s", ::GetCommandLine());
	// @pyseeapi GetCommandLine
}

// @pymethod tuple|win32api|GetDiskFreeSpace|Retrieves information about the specified disk, including the amount of free space available.
PyObject *PyGetDiskFreeSpace (PyObject *self, PyObject *args)
{
  char *path = NULL;
  // @pyparm string|rootPath||Specifies the root directory of the disk to return information about. If rootPath is None, the method uses the root of the current directory. 
  if (!PyArg_ParseTuple (args, "|z:GetDiskFreeSpace", &path))
	return NULL;
  DWORD spc, bps, fc, c;
  // @pyseeapi GetDiskFreeSpace
  PyW32_BEGIN_ALLOW_THREADS
  BOOL ok = ::GetDiskFreeSpace(path, &spc, &bps, &fc, &c);
  PyW32_END_ALLOW_THREADS
  if (!ok)
	return ReturnAPIError("GetDiskSpaceFree");
  return Py_BuildValue ("(iiii)",  spc, bps, fc, c);
  // @rdesc The return value is a tuple of 4 integers, containing
  // the number of sectors per cluster, the number of bytes per sector,
  // the total number of free clusters on the disk and the total number of clusters on the disk.
  // <nl>If the function fails, an error is returned.
}

// @pymethod tuple|win32api|GetDiskFreeSpaceEx|Retrieves information about the specified disk, including the amount of free space available.
PyObject *PyGetDiskFreeSpaceEx (PyObject *self, PyObject *args)
{
  char *path = NULL;
  // @pyparm string|rootPath||Specifies the root directory of the disk to return information about. If rootPath is None, the method uses the root of the current directory. 
  if (!PyArg_ParseTuple (args, "|z:GetDiskFreeSpaceEx", &path))
	return NULL;
  ULARGE_INTEGER freeBytes, totalBytes, totalFree;
  // @pyseeapi GetDiskFreeSpaceEx
  PyW32_BEGIN_ALLOW_THREADS
  BOOL ok = ::GetDiskFreeSpaceEx(path, &freeBytes, &totalBytes, &totalFree);
  PyW32_END_ALLOW_THREADS
  if (!ok)
	return ReturnAPIError("GetDiskSpaceFreeEx");
  return Py_BuildValue ("LLL",  freeBytes, totalBytes, totalFree);
  // @rdesc The return value is a tuple of 3 integers, containing
  // the number of free bytes available
  // the total number of bytes available on disk
  // the total number of free bytes on disk
  // the above values may be less, if user-quotas are in effect
  // <nl>If the function fails, an error is returned.
}
// @pymethod int|win32api|GetAsyncKeyState|Retrieves the status of the specified key.
static PyObject *
PyGetAsyncKeyState(PyObject * self, PyObject * args)
{
	int key;
	// @pyparm int|key||Specifies one of 256 possible virtual-key codes.
	if (!PyArg_ParseTuple(args, "i:GetAsyncKeyState", &key))
		return (NULL);
	int ret;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi GetAsyncKeyState
	ret = GetAsyncKeyState(key);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("i",ret);
	// @rdesc The return value specifies whether the key was pressed since the last
	// call to GetAsyncKeyState, and whether the key is currently up or down. If
	// the most significant bit is set, the key is down, and if the least significant
	// bit is set, the key was pressed after the previous call to GetAsyncKeyState.
	// <nl>The return value is zero if a window in another thread or process currently has the
	// keyboard focus.
	// @comm An application can use the virtual-key code constants win32con.VK_SHIFT,
	// win32con.VK_CONTROL, and win32con.VK_MENU as values for the key parameter.
	// This gives the state of the SHIFT, CTRL, or ALT keys without distinguishing
	// between left and right. An application can also use the following virtual-key
	// code constants as values for key to distinguish between the left and
	// right instances of those keys:
	// <nl>win32con.VK_LSHIFT
	// <nl>win32con.VK_RSHIFT
	// <nl>win32con.VK_LCONTROL
	// <nl>win32con.VK_RCONTROL
	// <nl>win32con.VK_LMENU
	// <nl>win32con.VK_RMENU
	// <nl>The GetAsyncKeyState method works with mouse buttons. However, it checks on
	// the state of the physical mouse buttons, not on the logical mouse buttons that
	// the physical buttons are mapped to.
}

// @pymethod int|win32api|GetFileAttributes|Retrieves the attributes for the named file.
static PyObject *
PyGetFileAttributes (PyObject *self, PyObject *args)
{
	PyObject *obPathName;
	// @pyparm string|pathName||The name of the file whose attributes are to be returned.
	// If this param is a unicode object, GetFileAttributesW is called.
	if (!PyArg_ParseTuple (args, "O:GetFileAttributes", &obPathName))
		return NULL;
	DWORD rc;
	if (PyString_Check(obPathName)) {
		PyW32_BEGIN_ALLOW_THREADS
		rc = ::GetFileAttributes(PyString_AS_STRING(obPathName));
		PyW32_END_ALLOW_THREADS
	} else if (PyUnicode_Check(obPathName)) {
		PyW32_BEGIN_ALLOW_THREADS
		rc = ::GetFileAttributesW(PyUnicode_AS_UNICODE(obPathName));
		PyW32_END_ALLOW_THREADS
	} else
		return PyErr_Format(PyExc_TypeError, "pathName arg must be string or unicode");

	if (rc==(DWORD)0xFFFFFFFF)
		return ReturnAPIError("GetFileAttributes");
	return Py_BuildValue("i", rc);
	// @pyseeapi GetFileAttributes
	// @pyseeapi GetFileAttributesW
	// @rdesc The return value is a combination of the win32con.FILE_ATTRIBUTE_* constants.
	// <nl>An exception is raised on failure.
}

// @pymethod int|win32api|GetKeyState|Retrieves the status of the specified key.
static PyObject *
PyGetKeyState(PyObject * self, PyObject * args)
{
	int key;
	// @pyparm int|key||Specifies a virtual key. If the desired virtual key is a letter or digit (A through Z, a through z, or 0 through 9), key must be set to the ASCII value of that character. For other keys, it must be a virtual-key code.
	if (!PyArg_ParseTuple(args, "i:GetKeyState", &key))
		return (NULL);
	int ret;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi GetKeyState
	ret = GetKeyState(key);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("i",ret);
	// @rdesc The return value specifies the status of
	// the given virtual key. If the high-order bit is 1, the key is down;
	// otherwise, it is up. If the low-order bit is 1, the key is toggled. 
	// A key, such as the CAPS LOCK key, is toggled if it is turned on.
	// The key is off and untoggled if the low-order bit is 0. A toggle key's
	// indicator light (if any) on the keyboard will be on when the key is
	// toggled, and off when the key is untoggled.
	// @comm The key status returned from this function changes as a given thread
	// reads key messages from its message queue. The status does not reflect the
	// interrupt-level state associated with the hardware. Use the <om win32api.GetAsyncKeyState> method to retrieve that information.
}

// @pymethod string|win32api|GetKeyboardState|Retrieves the status of the 256 virtual keys on the keyboard.
static PyObject *
PyGetKeyboardState(PyObject * self, PyObject * args)
{
	BYTE buf[256];
	if (!PyArg_ParseTuple(args, ":GetKeyboardState"))
		return (NULL);
	BOOL ok;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi GetKeyboardState
	ok = GetKeyboardState(buf);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("GetKeyboardState");

	return PyString_FromStringAndSize((char *)buf, 256);
	// @rdesc The return value is a string of exactly 256 characters.
	// Each character represents the bitmask for a key - see the Win32
	// documentation for more details.
}

// @pymethod int|win32api|VkKeyScan|Translates a character to the corresponding virtual-key code and shift state. 
static PyObject *
PyVkKeyScan(PyObject * self, PyObject * args)
{
	char *key;
	int len;
	// @pyparm chr|char||Specifies a character
	if (!PyArg_ParseTuple(args, "s#:VkKeyScan", &key, &len))
		return (NULL);
	if (len != 1)
		return PyErr_Format(PyExc_ValueError, "arg must be a string of length 1");
	int ret;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi VkKeyScan
	ret = VkKeyScan(key[0]);
	PyW32_END_ALLOW_THREADS
	return PyInt_FromLong(ret);
}

// @pymethod int|win32api|VkKeyScanEx|Translates a character to the corresponding virtual-key code and shift state. 
static PyObject *
PyVkKeyScanEx(PyObject * self, PyObject * args)
{
	char *key;
	int len;
	long kl;
	// @pyparm chr|char||Specifies a character
	if (!PyArg_ParseTuple(args, "s#l:VkKeyScanEx", &key, &len, &kl))
		return (NULL);
	if (len != 1)
		return PyErr_Format(PyExc_ValueError, "arg must be a string of length 1");
	int ret;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi VkKeyScanEx
	ret = VkKeyScanEx(key[0], (HKL)kl);
	PyW32_END_ALLOW_THREADS
	return PyInt_FromLong(ret);
}

// @pymethod int|win32api|GetLastError|Retrieves the calling thread's last error code value.
static PyObject *
PyGetLastError(PyObject * self, PyObject * args)
{
	// @pyseeapi GetLastError
	return Py_BuildValue("i",::GetLastError());
}

// @pymethod int|win32api|SetLastError|Sets the calling thread's last error code value.
static PyObject *
PySetLastError(PyObject * self, PyObject * args)
{
	long errVal;
	if (!PyArg_ParseTuple(args, "k", &errVal))
		return NULL;
	// @pyseeapi SetLastError
	::SetLastError(errVal);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod string|win32api|GetLogicalDriveStrings|Returns a string with all logical drives currently mapped.
static PyObject * PyGetLogicalDriveStrings (PyObject * self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetLogicalDriveStrings"))
		return (NULL);
	// @pyseeapi GetLogicalDriveStrings
	int result;
	// first, find out how big our string needs to be.
	result = ::GetLogicalDriveStrings(0, NULL);
	if (!result) {
		return ReturnAPIError("GetLogicalDriveStrings");
	} else {
		char * buffer = new char[result];
		result = ::GetLogicalDriveStrings (result, buffer);
		if (!result) {
			return ReturnAPIError("GetLogicalDriveStrings");
		} else {
			PyObject * retval = Py_BuildValue ("s#", buffer, result);
			delete [] buffer;
			return (retval);
		}
	}
	// @rdesc The return value is a single string, with each drive
	// letter NULL terminated.
	// <nl>Use "string.splitfields (s, '\\000')" to split into components.
}

// @pymethod string|win32api|GetModuleFileName|Retrieves the filename of the specified module.
static PyObject *
PyGetModuleFileName(PyObject * self, PyObject * args)
{
	int iMod;
	char buf[_MAX_PATH];
	// @pyparm int|hModule||Specifies the handle to the module.
	if (!PyArg_ParseTuple(args, "i:GetModuleFileName", &iMod))
		return (NULL);
	// @pyseeapi GetModuleFileName
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::GetModuleFileName( (HMODULE)iMod, buf, sizeof(buf));
	PyW32_END_ALLOW_THREADS
	if (rc==0)
		return ReturnAPIError("GetModuleFileName");
	return Py_BuildValue("s",buf);
}

// @pymethod int|win32api|GetModuleHandle|Returns the handle of an already loaded DLL.
static PyObject *
PyGetModuleHandle(PyObject * self, PyObject * args)
{
	char *fname = NULL;
	// @pyparm string|fileName|None|Specifies the file name of the module to load.
	if (!PyArg_ParseTuple(args, "|z:GetModuleHandle", &fname))
		return (NULL);
	// @pyseeapi GetModuleHandle
	HINSTANCE hInst = ::GetModuleHandle(fname);
	if (hInst==NULL)
		return ReturnAPIError("GetModuleHandle");
	return Py_BuildValue("i",hInst);
}

// @pymethod int|win32api|GetUserDefaultLCID|Retrieves the user default locale identifier.
static PyObject *
PyGetUserDefaultLCID(PyObject * self, PyObject * args)
{
	// @pyseeapi GetUserDefaultLCID
	return Py_BuildValue("i",::GetUserDefaultLCID());
}

// @pymethod int|win32api|GetUserDefaultLangID|Retrieves the user default language identifier. 
static PyObject *
PyGetUserDefaultLangID(PyObject * self, PyObject * args)
{
	// @pyseeapi GetUserDefaultLangID
	return Py_BuildValue("i",::GetUserDefaultLangID());
}

// @pymethod int|win32api|GetSystemDefaultLCID|Retrieves the system default locale identifier.
static PyObject *
PyGetSystemDefaultLCID(PyObject * self, PyObject * args)
{
	// @pyseeapi GetSystemDefaultLCID
	return Py_BuildValue("i",::GetSystemDefaultLCID());
}

// @pymethod int|win32api|GetSystemDefaultLangID|Retrieves the system default language identifier. 
static PyObject *
PyGetSystemDefaultLangID(PyObject * self, PyObject * args)
{
	// @pyseeapi GetSystemDefaultLangID
	return Py_BuildValue("i",::GetSystemDefaultLangID());
}

#ifndef DONT_HAVE_SYSTEM_SHUTDOWN
// @pymethod |win32api|AbortSystemShutdown|Aborts a system shutdown
static PyObject *
PyAbortSystemShutdown(PyObject * self, PyObject * args)
{
	// @pyparm string/<o PyUnicode>|computerName||Specifies the name of the computer where the shutdown is to be stopped.
	PyObject *obName;
	if (!PyArg_ParseTuple(args, "O:AbortSystemShutdown", &obName))
		return NULL;
	TCHAR *cname;
	if (!PyWinObject_AsTCHAR(obName, &cname, TRUE))
		return NULL;
	// @pyseeapi AbortSystemShutdown
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = AbortSystemShutdown(cname);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(cname);
	if (!ok)
		return ReturnAPIError("AbortSystemShutdown");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|InitiateSystemShutdown|Initiates a shutdown and optional restart of the specified computer. 
static PyObject *
PyInitiateSystemShutdown(PyObject * self, PyObject * args)
{
	PyObject *obName;
	PyObject *obMessage;
	DWORD dwTimeOut;
	BOOL bForceClosed;
	BOOL bRebootAfter;
	// @pyparm string/<o PyUnicode>|computerName||Specifies the name of the computer to shut-down, or None
	// @pyparm string/<o PyUnicode>|message||Message to display in a dialog box
	// @pyparm int|timeOut||Specifies the time (in seconds) that the dialog box should be displayed. While this dialog box is displayed, the shutdown can be stopped by the AbortSystemShutdown function. 
	// If dwTimeout is zero, the computer shuts down without displaying the dialog box, and the shutdown cannot be stopped by AbortSystemShutdown.
	// @pyparm int|bForceClose||Specifies whether applications with unsaved changes are to be forcibly closed. If this parameter is TRUE, such applications are closed. If this parameter is FALSE, a dialog box is displayed prompting the user to close the applications.
	// @pyparm int|bRebootAfterShutdown||Specifies whether the computer is to restart immediately after shutting down. If this parameter is TRUE, the computer is to restart. If this parameter is FALSE, the system flushes all caches to disk, clears the screen, and displays a message indicating that it is safe to power down.
	if (!PyArg_ParseTuple(args, "OOlll:InitiateSystemShutdown", &obName, &obMessage, &dwTimeOut, &bForceClosed, &bRebootAfter))
		return (NULL);
	char *cname;
	char *message;
	if (!PyWinObject_AsTCHAR(obName, &cname, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obMessage, &message, TRUE)) {
		PyWinObject_FreeTCHAR(cname);
		return NULL;
	}
	// @pyseeapi InitiateSystemShutdown
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = InitiateSystemShutdown(cname, message, dwTimeOut, bForceClosed, bRebootAfter);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(cname);
	PyWinObject_FreeTCHAR(message);
	if (!ok)
		return ReturnAPIError("InitiateSystemShutdown");
	Py_INCREF(Py_None);
	return Py_None;
}
#endif // DONT_HAVE_SYSTEM_SHUTDOWN

// @pymethod |win32api|ExitWindows|Logs off the current user
static PyObject *
PyExitWindows(PyObject * self, PyObject * args)
{
	// @pyparm int|reserved1|0|
	// @pyparm int|reserved2|0|
	DWORD dwReserved = 0;
	ULONG ulReserved = 0;
	if (!PyArg_ParseTuple(args, "|ll:ExitWindows", &dwReserved, &ulReserved))
		return NULL;
	// @pyseeapi AbortSystemShutdown
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ExitWindows(dwReserved, ulReserved);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("ExitWindows");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|ExitWindowsEx|either logs off the current user, shuts down the system, or shuts down and restarts the system.
static PyObject *
PyExitWindowsEx(PyObject * self, PyObject * args)
{
	// @comm It sends the WM_QUERYENDSESSION message to all applications to determine if they can be terminated. 

	// @pyparm int|flags||The shutdown operation
	// @pyparm int|reserved|0|
	UINT flags;
	DWORD reserved = 0;
	if (!PyArg_ParseTuple(args, "l|l:ExitWindowsEx", &flags, &reserved))
		return NULL;
	// @pyseeapi AbortSystemShutdown
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ExitWindowsEx(flags, reserved);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("ExitWindowsEx");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|LoadLibrary|Loads the specified DLL, and returns the handle.
static PyObject *
PyLoadLibrary(PyObject * self, PyObject * args)
{
	char *fname;
	// @pyparm string|fileName||Specifies the file name of the module to load.
	if (!PyArg_ParseTuple(args, "s:LoadLibrary", &fname))
		return (NULL);
	// @pyseeapi LoadLibrary
	PyW32_BEGIN_ALLOW_THREADS
	HINSTANCE hInst = ::LoadLibrary(fname);
	PyW32_END_ALLOW_THREADS
	if (hInst==NULL)
		return ReturnAPIError("LoadLibrary");
	return Py_BuildValue("i",hInst);
}

// @pymethod int|win32api|LoadLibraryEx|Loads the specified DLL, and returns the handle.
static PyObject *
PyLoadLibraryEx(PyObject * self, PyObject * args)
{
	char *fname;
	HANDLE handle;
	DWORD flags;
	// @pyparm string|fileName||Specifies the file name of the module to load.
	// @pyparm int|handle||Reserved - must be zero
	// @pyparm flags|handle||Specifies the action to take when loading the module.
	if (!PyArg_ParseTuple(args, "sll:LoadLibraryEx", &fname, &handle, &flags))
		return (NULL);
	// @pyseeapi LoadLibraryEx
	PyW32_BEGIN_ALLOW_THREADS
	HINSTANCE hInst = ::LoadLibraryEx(fname, handle, flags);
	PyW32_END_ALLOW_THREADS
	if (hInst==NULL)
		return ReturnAPIError("LoadLibraryEx");
	return Py_BuildValue("i",hInst);
}

// @pymethod |win32api|FreeLibrary|Decrements the reference count of the loaded dynamic-link library (DLL) module.
static PyObject *
PyFreeLibrary(PyObject * self, PyObject * args)
{
	HINSTANCE handle;
	// @pyparm int|hModule||Specifies the handle to the module.
	if (!PyArg_ParseTuple(args, "i:FreeLibrary", &handle))
		return (NULL);
	// @pyseeapi FreeLibrary
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::FreeLibrary(handle);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("FreeLibrary");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|GetProcAddress|Returns the address of the specified exported dynamic-link library (DLL) function. 
static PyObject *
PyGetProcAddress(PyObject * self, PyObject * args)
{
	HINSTANCE handle;
	char *fnName;
	// @pyparm int|hModule||Specifies the handle to the module.
	// @pyparm string|functionName||Specifies the name of the procedure.
	if (!PyArg_ParseTuple(args, "is:GetProcAddress", &handle, &fnName))
		return (NULL);
	FARPROC proc = ::GetProcAddress(handle, fnName);
	if (proc==NULL)
		return ReturnAPIError("GetProcAddress");
	// @pyseeapi GetProcAddress
	return PyLong_FromVoidPtr(proc);
}

// @pymethod int/string|win32api|GetProfileVal|Retrieves entries from a windows INI file.  This method encapsulates GetProfileString, GetProfileInt, GetPrivateProfileString and GetPrivateProfileInt.
static PyObject *
PyGetProfileVal(PyObject *self, PyObject *args)
{
	char *sect, *entry, *strDef, *iniName=NULL;
	int intDef;
	BOOL bHaveInt = TRUE;
	if (!PyArg_ParseTuple(args, "ssi|s", 
	          &sect,  // @pyparm string|section||The section in the INI file to retrieve a value for.
	          &entry, // @pyparm string|entry||The entry within the section in the INI file to retrieve a value for.
	          &intDef, // @pyparm int/string|defValue||The default value.  The type of this parameter determines the methods return type.
	          &iniName)) { // @pyparm string|iniName|None|The name of the INI file.  If None, the system INI file is used.
		bHaveInt = FALSE;
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "sss|z", &sect, &entry, &strDef, &iniName)) {
			// set my own error
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "GetProfileVal must have format (ssi|s) or (sss|s)");
			return NULL;
		}
	}

	if (iniName) {	
		if (bHaveInt)
			return Py_BuildValue("i",::GetPrivateProfileInt(sect, entry, intDef, iniName));
		else {
			char resBuf[2046];
			PyW32_BEGIN_ALLOW_THREADS
			::GetPrivateProfileString(sect, entry, strDef, resBuf, sizeof(resBuf), iniName);
			PyW32_END_ALLOW_THREADS
			return Py_BuildValue("s",resBuf);
		}
	}
	else {
		if (bHaveInt)
			return Py_BuildValue("i",::GetProfileInt(sect, entry, intDef));
		else {
			char resBuf[2046];
			PyW32_BEGIN_ALLOW_THREADS
			::GetProfileString(sect, entry, strDef, resBuf, sizeof(resBuf));
			PyW32_END_ALLOW_THREADS
			return Py_BuildValue("s",resBuf);
		}
	}
	// @pyseeapi GetProfileString
	// @pyseeapi GetProfileInt
	// @pyseeapi GetPrivateProfileString
	// @pyseeapi GetPrivateProfileInt
	// @rdesc The return value is the same type as the default parameter.
}
// @pymethod list|win32api|GetProfileSection|Retrieves all entries from a section in an INI file.
static PyObject *
PyGetProfileSection(PyObject * self, PyObject * args)
{
	char *szSection;
	char *iniName = NULL;
	// @pyparm string|section||The section in the INI file to retrieve a entries for.
	// @pyparm string|iniName|None|The name of the INI file.  If None, the system INI file is used.
	if (!PyArg_ParseTuple(args, "s|z:GetProfileSection", &szSection, &iniName))
		return (NULL);
	int size=0;
	int retVal = 0;
	char *szRetBuf = NULL;
	while (retVal >= size-2) {
		if (szRetBuf)
			delete szRetBuf;
		size=size?size*2:256;
		szRetBuf = new char[size]; /* cant fail - may raise exception */
		if (szRetBuf==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Error allocating space for return buffer");
			return NULL;
		}
		PyW32_BEGIN_ALLOW_THREADS
		if (iniName)
			retVal = GetPrivateProfileSection(szSection, szRetBuf, size, iniName);
		else
			retVal = GetProfileSection(szSection, szRetBuf, size);
		PyW32_END_ALLOW_THREADS
	}
	PyObject *retList = PyList_New(0);
	char *sz = szRetBuf;
	char *szLast = szRetBuf;
	while (*szLast!='\0') {
		for (;*sz;sz++)
			;
		PyObject *newItem = Py_BuildValue("s", szLast);
		PyList_Append(retList, newItem );
		sz++;
		szLast = sz;
	}
	// @pyseeapi GetProfileString
	// @pyseeapi GetProfileInt
	// @pyseeapi GetPrivateProfileString
	// @pyseeapi GetPrivateProfileInt
	delete szRetBuf;
	return retList;
	// @rdesc The return value is a list of strings.
}

// @pymethod list|win32api|WriteProfileSection|Writes a complete section to an INI file or registry.
static PyObject *
PyWriteProfileSection(PyObject * self, PyObject * args)
{
	char *szSection;
	char *data;
	int dataSize;
	// @pyparm string|section||The section in the INI file to retrieve a entries for.
	// @pyparm string|data||The data to write.  This must be string, with each entry terminated with '\0', followed by another terminating '\0'
	if (!PyArg_ParseTuple(args, "ss#:WriteProfileSection", &szSection, &data, &dataSize))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = WriteProfileSection(szSection, data);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GetTempPath");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod tuple|win32api|GetSystemInfo|Retrieves information about the current system.
static PyObject *
PyGetSystemInfo(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":GetSystemInfo"))
		return NULL;
	// @pyseeapi GetSystemInfo
	SYSTEM_INFO info;
	GetSystemInfo( &info );
	return Py_BuildValue("iiiiiiii(ii)",
#if !defined(MAINWIN)
						 info.dwOemId,
#else
						 0,
#endif // MAINWIN
						 info.dwPageSize, 
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

// @pymethod int|win32api|GetSystemMetrics|Retrieves various system metrics and system configuration settings. 
static PyObject *
PyGetSystemMetrics(PyObject * self, PyObject * args)
{
	int which;
	// @pyparm int|index||Which metric is being requested.  See the API documentation for a full list.
	if (!PyArg_ParseTuple(args, "i:GetSystemMetrics", &which))
		return NULL;
	// @pyseeapi GetSystemMetrics
	int rc = ::GetSystemMetrics(which);
	return Py_BuildValue("i",rc);
}

// @pymethod string|win32api|GetShortPathName|Obtains the short path form of the specified path.
static PyObject *
PyGetShortPathName(PyObject * self, PyObject * args)
{
	PyObject *obPath;
	// @pyparm string/unicode|path||If a unicode object is passed,
	// GetShortPathNameW will be called and a unicode object returned.
	if (!PyArg_ParseTuple(args, "O:GetShortPathName", &obPath))
		return NULL;
	if (PyString_Check(obPath)) {
		char *path;
		if (!PyWinObject_AsString(obPath, &path))
			return NULL;

		char szOutPath[_MAX_PATH];
		// @pyseeapi GetShortPathName
		PyW32_BEGIN_ALLOW_THREADS
		DWORD rc = GetShortPathName(path, szOutPath, sizeof(szOutPath));
		PyW32_END_ALLOW_THREADS
		if (rc==0)
			return ReturnAPIError("GetShortPathName");
		if (rc>=sizeof(szOutPath))
			return ReturnError("The pathname would be too big!!!");
		return Py_BuildValue("s", szOutPath);
	} else {
		WCHAR *path;
		if (!PyWinObject_AsWCHAR(obPath, &path))
			return NULL;
		WCHAR szOutPath[_MAX_PATH];
		// @pyseeapi GetShortPathName
		PyW32_BEGIN_ALLOW_THREADS
		DWORD rc = GetShortPathNameW(path, szOutPath, sizeof(szOutPath));
		PyW32_END_ALLOW_THREADS
		if (rc==0)
			return ReturnAPIError("GetShortPathNameW");
		if (rc>=sizeof(szOutPath))
			return ReturnError("The (unicode) pathname would be too big!!!");
		return Py_BuildValue("u", szOutPath);
	}
	// @comm The short path name is an 8.3 compatible file name.  As the input path does
	// not need to be absolute, the returned name may be longer than the input path.
	return PyErr_Format(PyExc_RuntimeError, "not reached!?");
}

// @pymethod string|win32api|GetLongPathName|Converts the specified path to its long form.
static PyObject *
PyGetLongPathNameA (PyObject *self, PyObject *args)
{
	// @comm This function may raise a NotImplementedError exception if the version
	// of Windows does not support this function.
	CHECK_PFN(GetLongPathNameA);

	char *fileName, *pathBuf=NULL;
	DWORD bufsize=MAX_PATH, reqd_bufsize;
	PyObject *ret=NULL;

	if (!PyArg_ParseTuple (args, "s:GetLongPathName", 
		&fileName))	// @pyparm string|fileName||The file name.
		return NULL;

	while (1){
		if (pathBuf)
			free(pathBuf);
		pathBuf=(char *)malloc(bufsize);
		if (pathBuf==NULL)
			return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		PyW32_BEGIN_ALLOW_THREADS
		reqd_bufsize = (*pfnGetLongPathNameA)(fileName, pathBuf, bufsize);
		PyW32_END_ALLOW_THREADS
		if (reqd_bufsize==0){
			PyWin_SetAPIError("GetLongPathName");
			break;
			}
		if (reqd_bufsize<=bufsize){
			ret=PyString_FromStringAndSize(pathBuf, reqd_bufsize);
			break;
			}
		bufsize=reqd_bufsize+1;
		}
	free(pathBuf);
	return ret;
}

// @pymethod <o PyUnicode>|win32api|GetLongPathNameW|Converts the specified path to its long form.
static PyObject *
PyGetLongPathNameW (PyObject *self, PyObject *args)
{
	// @comm This function may raise a NotImplementedError exception if the version
	// of Windows does not support this function.
	CHECK_PFN(GetLongPathNameW);

	WCHAR pathBuf[MAX_PATH];
	WCHAR *fileName;
	PyObject *obLongPathNameW = NULL;

	// @pyparm <o PyUnicode>|fileName||The file name.
	PyObject *obFileName;
	if (!PyArg_ParseTuple (args, "O:GetLongPathNameW", &obFileName))
		return NULL;
	if (!PyWinObject_AsWCHAR(obFileName, &fileName))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	DWORD length = (*pfnGetLongPathNameW)(fileName, pathBuf, sizeof(pathBuf)/sizeof(pathBuf[0]));
	PyW32_END_ALLOW_THREADS
	if (length)
	{
		if (length < sizeof(pathBuf)/sizeof(pathBuf[0]))
			obLongPathNameW = PyWinObject_FromWCHAR(pathBuf);
		else
		{
			// retry with a buffer that is big enough.  Now we know the
			// size and that it is big, avoid double-handling.
			Py_UNICODE *buf;
			// The length is the buffer needed, which includes the NULL.
			// PyUnicode_FromUnicode adds one.
			obLongPathNameW = PyUnicode_FromUnicode(NULL, length-1);
			if (!obLongPathNameW) {
				PyWinObject_FreeWCHAR(fileName);
				return NULL;
			}
			buf = PyUnicode_AS_UNICODE(obLongPathNameW);
			PyW32_BEGIN_ALLOW_THREADS
			DWORD length2 = (*pfnGetLongPathNameW)(fileName, buf, length);
			PyW32_END_ALLOW_THREADS
			if (length2==0) {
				Py_DECREF(obLongPathNameW);
				obLongPathNameW = NULL;
			}
			// On success, it is the number of chars copied *not* including
			// the NULL.  Check this is true.
			assert(length2+1==length);
		}
	}
	PyWinObject_FreeWCHAR(fileName);
	if(!obLongPathNameW)
		return ReturnAPIError("GetLongPathNameW");
	return obLongPathNameW;
}

// @pymethod string|win32api|GetTickCount|Returns the number of milliseconds since windows started.
static PyObject *
PyGetTickCount(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple (args, ":PyGetTickCount"))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	DWORD count = GetTickCount();
	PyW32_END_ALLOW_THREADS

	return Py_BuildValue("l",(long)count);
}

// @pymethod string|win32api|GetTempPath|Retrieves the path of the directory designated for temporary files.
static PyObject *
PyGetTempPath(PyObject * self, PyObject * args)
{
	char buf[MAX_PATH];
	if (!PyArg_ParseTuple (args, ":GetTempPath"))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = GetTempPath(sizeof(buf), buf);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GetTempPath");
	return Py_BuildValue("s",buf);
}
// @pymethod tuple|win32api|GetTempFileName|Returns creates a temporary filename of the following form: path\\preuuuu.tmp.
static PyObject *
PyGetTempFileName(PyObject * self, PyObject * args)
{
	char *path, *prefix;
	int n = 0;
	if (!PyArg_ParseTuple(args,"ss|i:GetTempFileName", 
	          &path,  // @pyparm string|path||Specifies the path where the method creates the temporary filename.
	                  // Applications typically specify a period (.) or the result of the GetTempPath function for this parameter.
	          &prefix,// @pyparm string|prefix||Specifies the temporary filename prefix.
	          &n))    // @pyparm int|nUnique||Specifies an nteger used in creating the temporary filename.
	                  // If this parameter is nonzero, it is appended to the temporary filename.
	                  // If this parameter is zero, Windows uses the current system time to create a number to append to the filename.
		return NULL;

    char buf[MAX_PATH];
	PyW32_BEGIN_ALLOW_THREADS
	UINT rc=GetTempFileName(path, prefix, n, buf);
	PyW32_END_ALLOW_THREADS
    if (!rc) // @pyseeapi GetTempFileName
		return ReturnAPIError("GetTempFileName");
	return Py_BuildValue("(si)",buf,rc);
	// @rdesc The return value is a tuple of (string, int), where string is the
	// filename, and rc is the unique number used to generate the filename.
}
// @pymethod tuple|win32api|GetTimeZoneInformation|Retrieves the system time-zone information.
static PyObject *
PyGetTimeZoneInformation(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple (args, ":GetTimeZoneInformation"))
		return NULL;
	TIME_ZONE_INFORMATION tzinfo;
	DWORD rc;
	
	rc = ::GetTimeZoneInformation(&tzinfo);
	if( rc == TIME_ZONE_ID_INVALID ) {
		return ReturnAPIError("GetTimeZoneInformation");
	}
	// else ok...
	// According to the ::GetTimezoneInformation() documentation, 
	// it will return:
	//    TIME_ZONE_ID_STANDARD if in standard time,
	//    TIME_ZONE_ID_DAYLIGHT if in daylight savings time, and
	//    TIME_ZONE_ID_UNKNOWN if the timezone in question doesn't 
	//               use daylight savings time, (eg. indiana time).  
	// We therefore need to add this code to the return tuple, so 
	// the calling code can decide which part of the tzinfo data to 
	// use.  This is now returned as the 1st element of an outer 
	// 2-tuple; the 2nd element is the (corrected) tuple representing 
	// the tzinfo structure.
	return Py_BuildValue("i,(lNNlNNl)",
				  rc,
				  tzinfo.Bias,
				  PyWinObject_FromWCHAR(tzinfo.StandardName),
				  PyWinObject_FromSYSTEMTIME(tzinfo.StandardDate),
				  tzinfo.StandardBias,
				  PyWinObject_FromWCHAR(tzinfo.DaylightName),
				  PyWinObject_FromSYSTEMTIME(tzinfo.DaylightDate),
				  tzinfo.DaylightBias );
	
	// @rdesc The return value is a tuple of (rc, tzinfo), where rc is
	// the integer return code from ::GetTimezoneInformation(), which may be
	// @flagh value|description
	// @flag TIME_ZONE_ID_STANDARD|if in standard time
	// @flag TIME_ZONE_ID_DAYLIGHT|if in daylight savings time
	// @flag TIME_ZONE_ID_UNKNOWN|if the timezone in question doesn't use daylight savings time, (eg. indiana time).  
	// @rdesc tzinfo is a tuple of:
	// @tupleitem 0|int|bias|Specifies the current bias, in minutes, for local time translation on this computer. The bias is the difference, in minutes, between Coordinated Universal Time (UTC) and local time. All translations between UTC and local time are based on the following formula:<nl><nl>UTC = local time + bias <nl><nl>
	// @tupleitem 1|unicode|standardName|Specifies a string associated with standard time on this operating system. For example, this member could contain "EST" to indicate Eastern Standard Time. This string is not used by the operating system, so anything stored there using the SetTimeZoneInformation function is returned unchanged by the GetTimeZoneInformation function. This string can be empty.
	// @tupleitem 2|<o PyTime>|standardTime|Specifies a SYSTEMTIME object that contains a date and local time when the transition from daylight saving time to standard time occurs on this operating system. If this date is not specified, the wMonth member in the SYSTEMTIME structure must be zero. If this date is specified, the DaylightDate value in the TIME_ZONE_INFORMATION structure must also be specified. 
	// <nl>To select the correct day in the month, set the wYear member to zero, the wDayOfWeek member to an appropriate weekday, and the wDay member to a value in the range 1 through 5. Using this notation, the first Sunday in April can be specified, as can the last Thursday in October (5 is equal to "the last"). 
	// @tupleitem 3|int|standardBias|Specifies a bias value to be used during local time translations that occur during standard time. This member is ignored if a value for the StandardDate member is not supplied. <nl>This value is added to the value of the Bias member to form the bias used during standard time. In most time zones, the value of this member is zero. 
	// @tupleitem 4|unicode|daylightName|
	// @tupleitem 5|<o PyTime>|daylightTime|
	// @tupleitem 6|int|daylightBias|Specifies a bias value to be used during local time translations that occur during daylight saving time. This member is ignored if a value for the DaylightDate member is not supplied. 
	// <nl>This value is added to the value of the Bias member to form the bias used during daylight saving time. In most time zones, the value of this member is – 60. 
	
}

// @pymethod string|win32api|GetDateFormat|Formats a date as a date string for a specified locale. The function formats either a specified date or the local system date.
static PyObject *PyGetDateFormat(PyObject *self, PyObject *args)
{
	int locale, flags;
	PyObject *obTime;
	char *szFormat = NULL;
	if (!PyArg_ParseTuple(args, "iiO|z:GetDateFormat",
						  &locale, // @pyparm int|locale||
						  &flags, // @pyparm int|flags||
						  &obTime, // @pyparm <o PyTime>|time||The time to use, or None to use the current time.
						  &szFormat)) // @pyparm string|format||May be None
		return NULL;
	SYSTEMTIME st, *pst = NULL;
	if (obTime != Py_None) {
		if (!PyWinObject_AsSYSTEMTIME(obTime, &st))
			return NULL;
		pst = &st;
	}
	char buf[512];
	int nchars = ::GetDateFormat(locale, flags, pst, szFormat, buf, sizeof(buf)/sizeof(buf)[0]);
	if (nchars==0)
		return PyWin_SetAPIError("GetDateFormat");
	return PyString_FromStringAndSize(buf, nchars-1);
}

// @pymethod string|win32api|GetTimeFormat|Formats a time as a time string for a specified locale. The function formats either a specified time or the local system time.
static PyObject *PyGetTimeFormat(PyObject *self, PyObject *args)
{
	int locale, flags;
	PyObject *obTime;
	char *szFormat = NULL;
	if (!PyArg_ParseTuple(args, "iiO|z:GetTimeFormat",
						  &locale, // @pyparm int|locale||
						  &flags, // @pyparm int|flags||
						  &obTime, // @pyparm <o PyTime>|time||The time to use, or None to use the current time.
						  &szFormat)) // @pyparm string|format||May be None
		return NULL;
	SYSTEMTIME st, *pst = NULL;
	if (obTime != Py_None) {
		if (!PyWinObject_AsSYSTEMTIME(obTime, &st))
			return NULL;
		pst = &st;
	}
	char buf[512];
	int nchars = ::GetTimeFormat(locale, flags, pst, szFormat, buf, sizeof(buf)/sizeof(buf)[0]);
	if (nchars==0)
		return PyWin_SetAPIError("GetTimeFormat");
	return PyString_FromStringAndSize(buf, nchars-1);
}

// @pymethod int|win32api|GetSysColor|Returns the current system color for the specified element.
static PyObject *
PyGetSysColor (PyObject *self, PyObject *args)
{
  int color_id, color_rgb;
  // @pyparm int|index||The Id of the element to return.  See the API for full details.
  if (!PyArg_ParseTuple (args, "i:GetSysColor", &color_id))
	return NULL;
  color_rgb = GetSysColor (color_id);
  // @pyseeapi GetSysColor
  return Py_BuildValue ("i", color_rgb);
  // @rdesc The return value is a windows RGB color representation.
}

// @pymethod |win32api|SetSysColors|Changes color of various window elements
static PyObject *PySetSysColors(PyObject *self, PyObject *args)
{
	int element_cnt=NULL, element_ind;
	int *elements=NULL, *element;
	COLORREF *rgbs=NULL, *rgb;
	PyObject *obelements, *obelement, *obrgbs, *ret=NULL;
	
	// @pyparm tuple|Elements||A tuple of ints, COLOR_* constants indicating which window element to change
	// @pyparm tuple|RgbValues||An equal length tuple of ints representing RGB values (see <om win32api.RGB>)
	if (!PyArg_ParseTuple(args, "OO:SetSysColors", &obelements, &obrgbs))
		return NULL;
	if (!PyTuple_Check(obelements)||!PyTuple_Check(obrgbs)
		||((element_cnt=PyTuple_Size(obelements))!=PyTuple_Size(obrgbs))){
		PyErr_SetString(PyExc_TypeError,"SetSysColors: Arguments must be equal length tuples of ints");
		return NULL;
		}
		
	elements=(int *)malloc(element_cnt*sizeof(int));
	if (elements==NULL){
		PyErr_Format(PyExc_MemoryError,"SetSysColors: Unable to allocate array of %d ints",element_cnt);
		goto done;
		}
	rgbs=(COLORREF *)malloc(element_cnt*sizeof(COLORREF));
	if (rgbs==NULL){
		PyErr_Format(PyExc_MemoryError,"SetSysColors: Unable to allocate array of %d COLORREF's",element_cnt);
		goto done;
		}

	rgb=rgbs;
	element=elements;
	for (element_ind=0;element_ind<element_cnt;element_ind++){
		obelement=PyTuple_GetItem(obelements,element_ind);
		if (obelement==NULL)
			goto done;
		*element=PyLong_AsLong(obelement);
		if ((*element==-1) && PyErr_Occurred()){
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError,"Color element must be an int");
			goto done;
			}
		obelement=PyTuple_GetItem(obrgbs,element_ind);
		if (obelement==NULL)
			goto done;
		*rgb=PyLong_AsLong(obelement);
		if ((*rgb==-1) && PyErr_Occurred()){
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError,"RGB value must be an int");
			goto done;
			}
		element++;
		rgb++;
		}
	if (!SetSysColors(element_cnt, elements, rgbs))
		PyWin_SetAPIError("SetSysColors");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}

	done:
	if (elements!=NULL)
		free(elements);
	if (rgbs!=NULL)
		free(rgbs);
	return ret;
}

// @pymethod string|win32api|GetSystemDirectory|Returns the path of the Windows system directory.
static PyObject *
PyGetSystemDirectory (PyObject *self, PyObject *args)
{
	char buf[MAX_PATH];
	if (!PyArg_ParseTuple (args, ":GetSystemDirectory"))
	// @pyseeapi GetSystemDirectory
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	::GetSystemDirectory(buf, sizeof(buf));
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("s", buf);
}

// @pymethod int|win32api|GetVersion|Returns the current version of Windows, and information about the environment.
static PyObject *
PyGetVersion(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple(args, ":GetVersion"))
		return NULL;
	return Py_BuildValue("i",::GetVersion());
	// @rdesc The return value's low word is the major/minor version of Windows.  The high
	// word is 0 if the platform is Windows NT, or 1 if Win32s on Windows 3.1
}

// @pymethod tuple|win32api|GetVersionEx|Returns the current version of Windows, and information about the environment.
static PyObject *
PyGetVersionEx(PyObject * self, PyObject * args)
{
	// @pyparm int|format|0|The format of the version info to return.
	// May be 0 (for OSVERSIONINFO) or 1 (for OSVERSIONINFOEX)
	int format = 0;
	if (!PyArg_ParseTuple(args, "|i:GetVersionEx", &format))
		return NULL;
	if (format == 0) {
		OSVERSIONINFO ver;
		ver.dwOSVersionInfoSize = sizeof(ver);
		if (!::GetVersionEx(&ver))
			return ReturnAPIError("GetVersionEx");
		return Py_BuildValue("iiiis",
		// @rdesc The return value is a tuple with the following information.<nl>
				 ver.dwMajorVersion, // @tupleitem 0|int|majorVersion|Identifies the major version number of the operating system.<nl>
					 ver.dwMinorVersion, //	@tupleitem 1|int|minorVersion|Identifies the minor version number of the operating system.<nl>
					 ver.dwBuildNumber,  //	@tupleitem 2|int|buildNumber|Identifies the build number of the operating system in the low-order word. (The high-order word contains the major and minor version numbers.)<nl>
					 ver.dwPlatformId, // @tupleitem 3|int|platformId|Identifies the platform supported by the operating system.  May be one of VER_PLATFORM_WIN32s, VER_PLATFORM_WIN32_WINDOWS or VER_PLATFORM_WIN32_NT<nl>
					 ver.szCSDVersion); // @tupleitem 4|string|version|Contains arbitrary additional information about the operating system.
	} else if (format == 1) {
		OSVERSIONINFOEX ver;
		ver.dwOSVersionInfoSize = sizeof(ver);
		if (!::GetVersionEx((LPOSVERSIONINFO)&ver))
			return ReturnAPIError("GetVersionEx");
		return Py_BuildValue("iiiisiiiii",
		// @rdesc or if the format param is 1, the return value is a tuple with:<nl>
				 ver.dwMajorVersion, // @tupleitem 0|int|majorVersion|Identifies the major version number of the operating system.<nl>
					 ver.dwMinorVersion, //	@tupleitem 1|int|minorVersion|Identifies the minor version number of the operating system.<nl>
					 ver.dwBuildNumber,  //	@tupleitem 2|int|buildNumber|Identifies the build number of the operating system in the low-order word. (The high-order word contains the major and minor version numbers.)<nl>
					 ver.dwPlatformId, // @tupleitem 3|int|platformId|Identifies the platform supported by the operating system.  May be one of VER_PLATFORM_WIN32s, VER_PLATFORM_WIN32_WINDOWS or VER_PLATFORM_WIN32_NT<nl>
					 ver.szCSDVersion, // @tupleitem 4|string|version|Contains arbitrary additional information about the operating system.
					 ver.wServicePackMajor, // @tupleitem 5|int|wServicePackMajor|Major version number of the latest Service Pack installed on the system. For example, for Service Pack 3, the major version number is 3. If no Service Pack has been installed, the value is zero. 
					 ver.wServicePackMinor, // @tupleitem 6|int|wServicePackMinor|Minor version number of the latest Service Pack installed on the system. For example, for Service Pack 3, the minor version number is 0.
					 ver.wSuiteMask, // @tupleitem 7|int|wSuiteMask|Bit flags that identify the product suites available on the system. This member can be a combination of the VER_SUITE_* values. 
					 ver.wProductType, // @tupleitem 8|int|wProductType|Additional information about the system. This member can be one of the VER_NT_* values.
					 ver.wReserved); // @tupleitem 9|int|wReserved|

	}
	return PyErr_Format(PyExc_ValueError, "format must be 0 or 1 (got %d)", format);
}

// @pymethod tuple|win32api|GetVolumeInformation|Returns information about a file system and colume whose root directory is specified.
static PyObject *
PyGetVolumeInformation(PyObject * self, PyObject * args)
{
	char *pathName;
	// @pyparm string|path||The root path of the volume on which information is being requested.
	if (!PyArg_ParseTuple(args, "s:GetVolumeInformation", &pathName))
		return NULL;
	char szVolName[_MAX_PATH];
	DWORD serialNo;
	DWORD maxCompLength;
	DWORD sysFlags;
	char szSysName[50];
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::GetVolumeInformation( pathName, szVolName, sizeof(szVolName), &serialNo, &maxCompLength, &sysFlags, szSysName, sizeof(szSysName));
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GetVolumeInformation");
	return Py_BuildValue("sllls", szVolName, (long)serialNo, (long)maxCompLength, (long)sysFlags, szSysName );
	// @rdesc The return is a tuple of:
	// <nl>string - Volume Name
	// <nl>long - Volume serial number.
	// <nl>long - Maximum Component Length of a file name.
	// <nl>long - Sys Flags - other flags specific to the file system.  See the api for details.
	// <nl>string - File System Name
}

// @pymethod string|win32api|GetFullPathName|Returns the full path of a (possibly relative) path
static PyObject *
PyGetFullPathName (PyObject *self, PyObject *args)
{
	char pathBuf[MAX_PATH];
	char *fileName;
	// @pyparm string|fileName||The file name.
	if (!PyArg_ParseTuple (args, "s:GetFullPathName", &fileName))
		return NULL;
	char *temp;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = GetFullPathName(fileName, sizeof(pathBuf), pathBuf, &temp);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("GetFullPathName");
	return Py_BuildValue("s", pathBuf);
}

// @pymethod string|win32api|GetWindowsDirectory|Returns the path of the Windows directory.
static PyObject *
PyGetWindowsDirectory (PyObject *self, PyObject *args)
{
	char buf[MAX_PATH];
	if (!PyArg_ParseTuple (args, ":GetWindowsDirectory"))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	::GetWindowsDirectory(buf, sizeof(buf));
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("s", buf);
}
// @pymethod |win32api|MoveFile|Renames a file, or a directory (including its children).
static PyObject *
PyMoveFile( PyObject *self, PyObject *args )
{
	char *src, *dest;
	// @pyparm string|srcName||The name of the source file.
	// @pyparm string|destName||The name of the destination file.
	// @comm This method can not move files across volumes.
	if (!PyArg_ParseTuple(args, "ss:MoveFile", &src, &dest))
		return NULL;
	// @pyseeapi MoveFile.
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::MoveFile(src, dest);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("MoveFile");
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |win32api|MoveFileEx|Renames a file.
static PyObject *
PyMoveFileEx( PyObject *self, PyObject *args )
{
	int flags;
	char *src, *dest;
	// @pyparm string|srcName||The name of the source file.
	// @pyparm string|destName||The name of the destination file.  May be None.
	// @pyparm int|flag||Flags indicating how the move is to be performed.  See the API for full details.
	// @comm This method can move files across volumes.<nl>
	// If destName is None, and flags contains win32con.MOVEFILE_DELAY_UNTIL_REBOOT, the
	// file will be deleted next reboot.
	if (!PyArg_ParseTuple(args, "szi:MoveFileEx", &src, &dest, &flags))
		return NULL;
	// @pyseeapi MoveFileEx
	PyW32_BEGIN_ALLOW_THREADS
		BOOL ok = ::MoveFileEx(src, dest, flags);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("MoveFileEx");
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |win32api|PostMessage|Post a message to a window.
PyObject *PyPostMessage(PyObject *self, PyObject *args)
{
	HWND hwnd;
	UINT message;
	WPARAM wParam=0;
	LPARAM lParam=0;
	if (!PyArg_ParseTuple(args, "ii|ii:PostMessage", 
	          &hwnd,    // @pyparm int|hwnd||The hWnd of the window to receive the message.
	          &message, // @pyparm int|idMessage||The ID of the message to post.
	          &wParam,  // @pyparm int|wParam||The wParam for the message
	          &lParam)) // @pyparm int|lParam||The lParam for the message
		return NULL;
	// @pyseeapi PostMessage
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::PostMessage(hwnd, message, wParam, lParam);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("PostMessage");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|PostThreadMessage|Post a message to the specified thread.
PyObject *PyPostThreadMessage(PyObject *self, PyObject *args)
{
	DWORD threadId;
	UINT message;
	WPARAM wParam=0;
	LPARAM lParam=0;
	if (!PyArg_ParseTuple(args, "ii|ii:PostThreadMessage", 
	          &threadId,    // @pyparm int|tid||Identifier of the thread to which the message will be posted.
	          &message, // @pyparm int|idMessage||The ID of the message to post.
	          &wParam,  // @pyparm int|wParam||The wParam for the message
	          &lParam)) // @pyparm int|lParam||The lParam for the message
		return NULL;
	// @pyseeapi PostThreadMessage
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::PostThreadMessage(threadId, message, wParam, lParam);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("PostThreadMessage");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|PostQuitMessage|Post a quit message to an app.
PyObject *PyPostQuitMessage(PyObject *self, PyObject *args)
{
	DWORD exitCode = 0;
	if (!PyArg_ParseTuple(args, "|i:PostQuitMessage", 
	          &exitCode))    // @pyparm int|exitCode|0|The exit code
		return NULL;
	// @pyseeapi PostQuitMessage
	PyW32_BEGIN_ALLOW_THREADS
	::PostQuitMessage(exitCode);
	PyW32_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|RegCloseKey|Closes a previously opened registry key.
static PyObject *
PyRegCloseKey( PyObject *self, PyObject *args )
{
	PyObject *obKey;
	// @pyparm <o PyHKEY>/int|key||The key to be closed.
	if (!PyArg_ParseTuple(args, "O:RegCloseKey", &obKey))
		return NULL;
	// @pyseeapi RegCloseKey
	if (!PyWinObject_CloseHKEY(obKey))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod int|win32api|RegConnectRegistry|Establishes a connection to a predefined registry handle on another computer.
static PyObject *
PyRegConnectRegistry( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *szCompName = NULL;
	HKEY retKey;
	long rc;
	// @pyparm string|computerName||The name of the remote computer, of the form \\\\computername.  If None, the local computer is used.
	// @pyparm int|key||The predefined handle.  May be win32con.HKEY_LOCAL_MACHINE or win32con.HKEY_USERS.
	if (!PyArg_ParseTuple(args, "zO:RegConnectRegistry", &szCompName, &obKey))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegConnectRegistry
	rc=RegConnectRegistry(szCompName, hKey, &retKey);
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegConnectRegistry", rc);
	return PyWinObject_FromHKEY(retKey);
	// @rdesc The return value is the handle of the opened key. 
	// If the function fails, an exception is raised.
}
// @pymethod <o PyHKEY>|win32api|RegCreateKey|Creates the specified key, or opens the key if it already exists.
static PyObject *
PyRegCreateKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;
	HKEY retKey;
	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of a key that this method opens or creates.
	// This key must be a subkey of the key identified by the key parameter.
	// If key is one of the predefined keys, subKey may be None. In that case,
	// the handle returned is the same hkey handle passed in to the function.
	if (!PyArg_ParseTuple(args, "Oz:RegCreateKey", &obKey, &subKey ))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegCreateKey
	rc=RegCreateKey(hKey, subKey, &retKey);
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegCreateKey", rc);
	return PyWinObject_FromHKEY(retKey);
	// @rdesc The return value is the handle of the opened key.
	// If the function fails, an exception is raised.
}
// @pymethod |win32api|RegDeleteKey|Deletes the specified key.  This method can not delete keys with subkeys.
static PyObject *
PyRegDeleteKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;
	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of the key to delete.
	// This key must be a subkey of the key identified by the key parameter.
	// This value must not be None, and the key may not have subkeys.
	if (!PyArg_ParseTuple(args, "Os:RegDeleteKey", &obKey, &subKey))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegDeleteKey
	rc=RegDeleteKey(hKey, subKey );
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegDeleteKey", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm If the method succeeds, the entire key, including all of its values, is removed.
	// If the method fails, and exception is raised.
}
// @pymethod |win32api|RegDeleteValue|Removes a named value from the specified registry key.
static PyObject *
PyRegDeleteValue( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;
	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|value||The name of the value to remove.
	if (!PyArg_ParseTuple(args, "Oz:RegDeleteValue", &obKey, &subKey))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegDeleteValue
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegDeleteValue(hKey, subKey);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegDeleteValue", rc);
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod string|win32api|RegEnumKey|Enumerates subkeys of the specified open registry key. The function retrieves the name of one subkey each time it is called.
static PyObject *
PyRegEnumKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	int index;
	long rc;
	char *retBuf;
    DWORD len;
    
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm int|index||The index of the key to retrieve.
	if (!PyArg_ParseTuple(args, "Oi:RegEnumKey", &obKey, &index))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;

	if ((rc=RegQueryInfoKey( hKey, NULL, NULL, NULL, NULL, &len, 
		                           NULL, NULL, NULL, NULL, NULL, NULL))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryInfoKey", rc);
	++len;	// include null terminator
	retBuf=(char *)alloca(len);

	// @pyseeapi RegEnumKey
	if ((rc=RegEnumKey(hKey, index, retBuf, len))!=ERROR_SUCCESS)
		return ReturnAPIError("RegEnumKey", rc);
	return Py_BuildValue("s", retBuf);
}

// @pymethod <o PyTuple>|win32api|RegEnumKeyEx|Returns list of subkeys, info is (name, reserved, class, last write time) - class currently not defined, will always be None, reserved always 0
static PyObject *
PyRegEnumKeyEx( PyObject *self, PyObject *args )
{
	PyObject *obreghandle=NULL, *obretitem=NULL, *obtimestamp=NULL;
	HKEY reghandle;
	FILETIME timestamp;
	long err;
	char *key_name;
    DWORD key_len=0, max_len=0, key_ind=0, nbr_keys=0;
    PyObject *ret=NULL;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS.
	if (!PyArg_ParseTuple(args, "O:RegEnumKeyEx", &obreghandle))
		return NULL;
	if (!PyWinObject_AsHKEY(obreghandle, &reghandle))
		return NULL;

	err=RegQueryInfoKey(reghandle,NULL,NULL,NULL, &nbr_keys, &max_len, NULL,NULL,NULL,NULL,NULL,NULL);
	if (err!=ERROR_SUCCESS)
		return ReturnAPIError("RegEnumKeyEx:RegQueryInfoKey",err);
	max_len++;						 // trailing NULL not included
	key_name=(char *)malloc(max_len);
	if (key_name==NULL){
		PyErr_SetString(PyExc_MemoryError, "RegEnumKeyEx: SOM");
		return NULL;
		}

	ret=PyTuple_New(nbr_keys);
	for (key_ind=0;key_ind<nbr_keys;key_ind++){
		key_len=max_len;
		err=RegEnumKeyEx(reghandle, key_ind, key_name, &key_len, NULL, NULL, NULL, &timestamp);
		if (err!=ERROR_SUCCESS){
			Py_DECREF(ret);
			ret=NULL;
			PyWin_SetAPIError("RegEnumKeyEx",err);
			break;
			}
		obtimestamp=PyWinObject_FromFILETIME(timestamp);
		obretitem=Py_BuildValue("s#iOO", key_name, key_len, 0, Py_None, obtimestamp);
		Py_DECREF(obtimestamp);
		PyTuple_SET_ITEM(ret, key_ind, obretitem);
		}
	free(key_name);
	return ret;
}

// @pymethod |win32api|RegNotifyChangeKeyValue|Receive notification of registry changes
static PyObject *
PyRegNotifyChangeKeyValue( PyObject *self, PyObject *args )
{
	PyObject *obreghandle=NULL, *obevent=NULL, *ret=NULL;
	HKEY reghandle;
	BOOL subtree=FALSE, asynch=FALSE;
	DWORD filter=0;
	HANDLE hevent;
	long err=0;
	if (!PyArg_ParseTuple(args,"OiiOi", 
		&obreghandle,			//@pyparm <o PyHKEY>/int|key||Handle to an open registry key
		&subtree,				//@pyparm int|bWatchSubTree||Boolean, notify of changes to subkeys if True
		&filter,				//@pyparm int|dwNotifyFilter||Combination of REG_NOTIFY_CHANGE_* constants
		&obevent,				//@pyparm <o PyHANDLE>|hKey||Event handle to be signalled, use None if fAsynchronous is False
		&asynch))				//@pyparm int|fAsynchronous||Boolean, function returns immediately if True, waits for change if False
		return NULL;
	if (!PyWinObject_AsHKEY(obreghandle, &reghandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(obevent, &hevent, TRUE)) // handle should be NULL if asynch is False
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	err=RegNotifyChangeKeyValue(reghandle, subtree, filter, hevent, asynch);
	PyW32_END_ALLOW_THREADS
	if (err==ERROR_SUCCESS)
		ret=Py_None;
	else
		PyWin_SetAPIError("RegNotifyChangeKeyValue",err);
	Py_XINCREF(ret);
	return ret;
}


// Note that fixupMultiSZ and countString have both had changes
// made to support "incorrect strings".  The registry specification
// calls for strings to be terminated with 2 null bytes.  It seems
// some commercial packages install strings whcich dont conform,
// causing this code to fail - however, "regedit" etc still work 
// with these strings (ie only we dont!).
static void
fixupMultiSZ(char **str, char *data, int len)
{
	char *P;
	int i;
	char    *Q;


	Q = data + len;
	for(P=data, i=0; P < Q && *P!='\0'; P++, i++)
	{
		str[i]=P;
		for(; *P!='\0'; P++)
			;
	}
}

static int
countStrings(char *data, int len)
{
	int strings;
	char *P;
	char *Q = data + len;

	for (P=data, strings=0; P < Q && *P!='\0'; P++, strings++)
		for(; P < Q && *P!='\0'; P++)
			;

	return strings;
}

/* Convert PyObject into Registry data. 
   Allocates space as needed. */
static bool
Py2Reg(PyObject *value, DWORD typ, BYTE **retDataBuf, DWORD *retDataSize)
{
	int i,j;
	switch (typ) {
		case REG_DWORD:
			if (value!=Py_None && !PyInt_Check(value))
				return false;
			*retDataBuf = (BYTE *)PyMem_NEW(DWORD, sizeof(DWORD));
			if (*retDataBuf==NULL){
				PyErr_NoMemory();
				return false;
			}
			*retDataSize=sizeof(DWORD);
			if (value==Py_None) {
				DWORD zero = 0;
				memcpy(*retDataBuf, &zero, sizeof(DWORD));
			} 
			else
				memcpy(*retDataBuf, &PyInt_AS_LONG((PyIntObject *)value), sizeof(DWORD));
			break;
		case REG_SZ:
		case REG_EXPAND_SZ:
			if (value==Py_None)
				*retDataSize=1;
			else {
				if (!PyString_Check(value))
					return false;
				*retDataSize=strlen(PyString_AS_STRING((PyStringObject *)value))+1;
			}
			*retDataBuf=(BYTE *)PyMem_NEW(DWORD, *retDataSize);
			if (*retDataBuf==NULL){
				PyErr_NoMemory();
				return false;
			}
			if (value==Py_None)
				strcpy((char *)*retDataBuf, "");
			else
				strcpy((char *)*retDataBuf, PyString_AS_STRING((PyStringObject *)value));
			break;
		case REG_MULTI_SZ:
			{
				DWORD size=0;
				PyObject *t;

				if (value==Py_None)
					i = 0;
				else {
					if (!PyList_Check(value))
						return false;
					i=PyList_Size(value);
				}
				for(j=0; j<i; j++)
				{
					t=PyList_GET_ITEM((PyListObject *)value,j);
					if (!PyString_Check(t))
						return 0;
					size=size+strlen(PyString_AS_STRING((PyStringObject *)t))+1;
				}
			
				*retDataSize=size+1;
				*retDataBuf=(BYTE *)PyMem_NEW(char, *retDataSize);
				if (*retDataBuf==NULL){
					PyErr_NoMemory();
					return false;
				}
				char *P=(char *)*retDataBuf;

				for(j=0; j<i; j++)
				{
					t=PyList_GET_ITEM((PyListObject *)value,j);
					strcpy(P,PyString_AS_STRING((PyStringObject *)t));
					P=P+strlen(PyString_AS_STRING((PyStringObject *)t))+1;
				}
				// And doubly-terminate the list...
				*P = '\0';
				break;
			}
		case REG_BINARY:
		// ALSO handle ALL unknown data types here.  Even if we cant support
		// it natively, we should handle the bits.
		default: 
			if (value==Py_None)
				*retDataSize = 0;
			else {
				if (!PyString_Check(value))
					return false;
				*retDataSize=PyString_Size(value);
				*retDataBuf=(BYTE *)PyMem_NEW(char, *retDataSize);
				if (*retDataBuf==NULL){
					PyErr_NoMemory();
					return false;
				}
				memcpy(*retDataBuf,PyString_AS_STRING((PyStringObject *)value),*retDataSize);
			}
			break;
	}

	return 1;

}

/* Convert Registry data into PyObject*/
static PyObject *
Reg2Py(char *retDataBuf, DWORD retDataSize, DWORD typ)
{
	PyObject *obData;

	switch (typ) {
		case REG_DWORD:
			if (retDataSize==0)
				obData = Py_BuildValue("i", 0);
			else
				obData = Py_BuildValue("i", *(int *)retDataBuf);
			break;
		case REG_SZ:
		case REG_EXPAND_SZ:
			// retDataBuf may or may not have a trailing NULL in
			// the buffer.
			if (retDataSize && retDataBuf[retDataSize-1]=='\0')
				--retDataSize;
			if (retDataSize==0)
				retDataBuf = "";
			obData = PyString_FromStringAndSize(retDataBuf, retDataSize);
			break;
		case REG_MULTI_SZ:
			if (retDataSize==0)
				obData = PyList_New(0);
			else
			{
				int index=0;
				int s=countStrings(retDataBuf, retDataSize);
				char **str=(char **)alloca(sizeof(char *)*s);

				fixupMultiSZ(str, retDataBuf, retDataSize);
				obData = PyList_New(s);
				for(index=0; index < s; index++)
				{
					PyList_SetItem(obData, index, Py_BuildValue("s", (char *)str[index]));
				}
		
				break;
			}
		case REG_BINARY:
		// ALSO handle ALL unknown data types here.  Even if we cant support
		// it natively, we should handle the bits.
		default:
			if (retDataSize==0) {
				Py_INCREF(Py_None);
				obData = Py_None;
			} else
				obData = Py_BuildValue("s#", (char *)retDataBuf, retDataSize);
			break;
	}
	if (obData==NULL)
		return NULL;
	else
		return obData;
}

// @pymethod (string,object,type)|win32api|RegEnumValue|Enumerates values of the specified open registry key. The function retrieves the name of one subkey each time it is called.
static PyObject *
PyRegEnumValue( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	int index;
	long rc;
	char *retValueBuf;
	char *retDataBuf;
	DWORD retValueSize;
	DWORD retDataSize;
	DWORD typ;

	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm int|index||The index of the key to retrieve.

	if (!PyArg_ParseTuple(args, "Oi:PyRegEnumValue", &obKey, &index))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;

	if ((rc=RegQueryInfoKey( hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		             &retValueSize, &retDataSize, NULL, NULL))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryInfoKey", rc);
	++retValueSize;	// include null terminators
	++retDataSize;
	retValueBuf=(char *)alloca(retValueSize);
	retDataBuf=(char *)alloca(retDataSize);

	// @pyseeapi PyRegEnumValue
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegEnumValue(hKey, index, retValueBuf, &retValueSize, NULL, &typ, (BYTE *)retDataBuf, &retDataSize);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("PyRegEnumValue", rc);
	PyObject *obData=Reg2Py(retDataBuf, retDataSize, typ);
	if (obData==NULL)
		return NULL;
	PyObject *retVal = Py_BuildValue("sOi", retValueBuf, obData, typ);
	Py_DECREF(obData);
	return retVal;
	// @comm This function is typically called repeatedly, until an exception is raised, indicating no more values.
}

// @pymethod |win32api|RegFlushKey|Writes all the attributes of the specified key to the registry.
static PyObject *
PyRegFlushKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	if (!PyArg_ParseTuple(args, "O:RegFlushKey", &obKey))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegFlushKey
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegFlushKey(hKey);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegFlushKey", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm It is not necessary to call RegFlushKey to change a key.
	// Registry changes are flushed to disk by the registry using its lazy flusher.
	// Registry changes are also flushed to disk at system shutdown.
	// <nl>Unlike <om win32api.RegCloseKey>, the RegFlushKey method returns only when all the data has been written to the registry.
	// <nl>An application should only call RegFlushKey if it requires absolute certainty that registry changes are on disk. If you don't know whether a RegFlushKey call is required, it probably isn't.
}
// @pymethod |win32api|RegLoadKey|The RegLoadKey method creates a subkey under HKEY_USER or HKEY_LOCAL_MACHINE
// and stores registration information from a specified file into that subkey.
static PyObject *
PyRegLoadKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;
	char *fileName;

	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of the key to delete.
	// This key must be a subkey of the key identified by the key parameter.
	// This value must not be None, and the key may not have subkeys.
	// @pyparm string|filename||The name of the file to load registry data from.
	//  This file must have been created with the <om win32api.RegSaveKey> function.
	// Under the file allocation table (FAT) file system, the filename may not have an extension.
	if (!PyArg_ParseTuple(args, "Oss:RegLoadKey", &obKey, &subKey, &fileName))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegLoadKey
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegLoadKey(hKey, subKey, fileName );
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegLoadKey", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm A call to RegLoadKey fails if the calling process does not have the SE_RESTORE_PRIVILEGE privilege.
	// <nl>If hkey is a handle returned by <om win32api.RegConnectRegistry>, then the path specified in fileName is relative to the remote computer. 
}
// @pymethod |win32api|RegUnLoadKey|The RegUnLoadKey function unloads the specified registry key and its subkeys from the registry.
// The key should have been created by a previous call to <om win32api.RegLoadKey>.
static PyObject *
PyRegUnLoadKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;

	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_USERS<nl>HKEY_LOCAL_MACHINE
	// @pyparm string|subKey||The name of the key to unload.
	// This key must be a subkey of the key identified by the key parameter.
	// This value must not be None.
	if (!PyArg_ParseTuple(args, "Os:RegUnLoadKey", &obKey, &subKey))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegUnLoadKey
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegUnLoadKey(hKey, subKey);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegUnLoadKey", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm A call to RegUnLoadKey fails if the calling process does not have the SE_RESTORE_PRIVILEGE privilege.
	// <nl>If hkey is a handle returned by <om win32api.RegConnectRegistry>, then the path specified in fileName is relative to the remote computer.
}

// @pymethod <o PyHKEY>|win32api|RegOpenKey|Opens the specified key.
// @comm This funcion is implemented using <om win32api.RegOpenKeyEx>, by taking advantage
// of default parameters.  See <om win32api.RegOpenKeyEx> for more details.
// @pymethod <o PyHKEY>|win32api|RegOpenKeyEx|Opens the specified key.
static PyObject *
PyRegOpenKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;

	char *subKey;
	int res = 0;
	HKEY retKey;
	long rc;
	REGSAM sam = KEY_READ;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of a key that this method opens.
	// This key must be a subkey of the key identified by the key parameter.
	// If key is one of the predefined keys, subKey may be None. In that case,
	// the handle returned is the same key handle passed in to the function.
	// @pyparm int|reserved|0|Reserved.  Must be zero.
	// @pyparm int|sam|KEY_READ|Specifies an access mask that describes the desired security access for the new key. This parameter can be a combination of the following win32con constants:
	// <nl>KEY_ALL_ACCESS<nl>KEY_CREATE_LINK<nl>KEY_CREATE_SUB_KEY<nl>KEY_ENUMERATE_SUB_KEYS<nl>KEY_EXECUTE<nl>KEY_NOTIFY<nl>KEY_QUERY_VALUE<nl>KEY_READ<nl>KEY_SET_VALUE<nl>KEY_WRITE<nl>
	if (!PyArg_ParseTuple(args, "Oz|ii:RegOpenKey", &obKey, &subKey, &res, &sam ))
		return NULL;
	// @pyseeapi RegOpenKeyEx
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;

	PyW32_BEGIN_ALLOW_THREADS
	rc=RegOpenKeyEx(hKey, subKey, res, sam, &retKey);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegOpenKeyEx", rc);
	return PyWinObject_FromHKEY(retKey);

	// @rdesc The return value is the handle of the opened key.
	// If the function fails, an exception is raised.
}

// @pymethod (int, int, long)|win32api|RegQueryInfoKey|Returns the number of 
// subkeys, the number of values a key has, 
// and if available the last time the key was modified as
// 100's of nanoseconds since Jan 1, 1600.
static PyObject *
PyRegQueryInfoKey( PyObject *self, PyObject *args)
{
  HKEY hKey;
  PyObject *obKey;
  long rc;
  DWORD nSubKeys, nValues;
  FILETIME ft;
  PyObject *l;

  // @pyparm <o PyHKEY>/int|key||An already open key, or or any one of the following win32con
  // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
  if (!PyArg_ParseTuple(args, "O:RegQueryInfoKey", &obKey))
    return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
  // @pyseeapi RegQueryInfoKey
  if ((rc=RegQueryInfoKey(hKey, NULL, NULL, 0, &nSubKeys, NULL, NULL, &nValues,
    NULL,
    NULL,
    NULL,
    &ft)
       )!=ERROR_SUCCESS)
    return ReturnAPIError("RegQueryInfoKey", rc);
  if (!(l=PyLong_FromTwoInts(ft.dwHighDateTime, ft.dwLowDateTime)))
      return NULL;
  PyObject *ret = Py_BuildValue("iiO",nSubKeys,nValues,l);
  Py_DECREF(l);
  return ret;
}

// @pymethod string|win32api|RegQueryValue|The RegQueryValue method retrieves the value associated with
// the unnamed value for a specified key in the registry.
static PyObject *
PyRegQueryValue( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;

	long rc;
	char *retBuf;
	long bufSize = 0;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of the subkey with which the value is associated.
	// If this parameter is None or empty, the function retrieves the value set by the <om win32api.RegSetValue> method for the key identified by key. 
	if (!PyArg_ParseTuple(args, "Oz:RegQueryValue", &obKey, &subKey))
		return NULL;
	// @pyseeapi RegQueryValue
	
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	if ((rc=RegQueryValue(hKey, subKey, NULL, &bufSize))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryValue", rc);
	retBuf=(char *)alloca(bufSize);
	if ((rc=RegQueryValue(hKey, subKey, retBuf, &bufSize))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryValue", rc);
	return Py_BuildValue("s", retBuf);
	// @comm Values in the registry have name, type, and data components. This method
	// retrieves the data for a key's first value that has a NULL name.
	// But the underlying API call doesn't return the type, Lame Lame Lame, DONT USE THIS!!!
}

// @pymethod (object,type)|win32api|RegQueryValueEx|Retrieves the type and data for a specified value name associated with an open registry key. 
static PyObject *
PyRegQueryValueEx( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *valueName;

	long rc;
	char *retBuf;
	DWORD bufSize = 0;
	DWORD typ;

	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|valueName||The name of the value to query.
	if (!PyArg_ParseTuple(args, "Oz:RegQueryValueEx", &obKey, &valueName))
		return NULL;
	// @pyseeapi RegQueryValueEx
	
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	if ((rc=RegQueryValueEx(hKey, valueName, NULL, NULL, NULL, &bufSize))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryValueEx", rc);
	retBuf=(char *)alloca(bufSize);
	if ((rc=RegQueryValueEx(hKey, valueName, NULL, &typ, (BYTE *)retBuf, &bufSize))!=ERROR_SUCCESS)
		return ReturnAPIError("RegQueryValueEx", rc);
	PyObject *obData=Reg2Py(retBuf, bufSize, typ);
	if (obData==NULL)
		return NULL;
	PyObject *result = Py_BuildValue("Oi", obData, typ);
	Py_DECREF(obData);
	return result;
	// @comm Values in the registry have name, type, and data components. This method
	// retrieves the data for the given value.
}



// @pymethod |win32api|RegSaveKey|The RegSaveKey method saves the specified key, and all its subkeys to the specified file.
static PyObject *
PyRegSaveKey( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	PyObject *obSA = Py_None;
	char *fileName;

	long rc;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|filename||The name of the file to save registry data to.
	// This file cannot already exist. If this filename includes an extension, it cannot be used on file allocation table (FAT) file systems by the <om win32api.RegLoadKey>, <om win32api.RegReplaceKey>, or <om win32api.RegRestoreKey> methods. 
	// @pyparm <o PySECURITY_ATTRIBUTES>|sa|None|The security attributes of the created file.
	if (!PyArg_ParseTuple(args, "Os|O:RegSaveKey", &obKey, &fileName, &obSA))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	LPSECURITY_ATTRIBUTES pSA;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obSA, &pSA, TRUE))
		return NULL;
	// @pyseeapi RegSaveKey
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegSaveKey(hKey, fileName, pSA );
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegSaveKey", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm If key represents a key on a remote computer, the path described by fileName is relative to the remote computer.
	// <nl>The caller of this method must possess the SeBackupPrivilege security privilege.
}
// @pymethod |win32api|RegSetValue|Associates a value with a specified key.  Currently, only strings are supported.
static PyObject *
PyRegSetValue( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *subKey;
	char *str;
	DWORD typ;
	DWORD len;
	long rc;
	PyObject *obStrVal;
	PyObject *obSubKey;
	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|subKey||The name of the subkey with which the value is associated.
	// This parameter can be None or empty, in which case the value will be added to the key identified by the key parameter. 
	// @pyparm int|type||Type of data. Must be win32con.REG_SZ
	// @pyparm string|value||The value to associate with the key.
	if (!PyArg_ParseTuple(args, "OOiO:RegSetValue", &obKey, &obSubKey, &typ, &obStrVal))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	if (typ!=REG_SZ) {
		PyErr_SetString(PyExc_TypeError, "Type must be win32con.REG_SZ");
		return NULL;
	}
	if (!PyWinObject_AsString(obStrVal, &str, FALSE, &len))
		return NULL;
	if (!PyWinObject_AsString(obSubKey, &subKey, TRUE)) {
		PyWinObject_FreeString(str);
		return NULL;
	}

	// @pyseeapi RegSetValue
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegSetValue(hKey, subKey, REG_SZ, str, len+1 );
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeString(str);
	PyWinObject_FreeString(subKey);
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegSetValue", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm If the key specified by the lpszSubKey parameter does not exist, the RegSetValue function creates it.
	// <nl>Value lengths are limited by available memory. Long values (more than 2048 bytes) should be stored as files with the filenames stored in the configuration registry. This helps the registry perform efficiently.
	// <nl>The key identified by the key parameter must have been opened with KEY_SET_VALUE access.
}

// @pymethod |win32api|RegSetValueEx|Stores data in the value field of an open registry key.
static PyObject *
PyRegSetValueEx( PyObject *self, PyObject *args )
{
	HKEY hKey;
	PyObject *obKey;
	char *valueName;
	PyObject *obRes;
	PyObject *value;
	BYTE *data;
	DWORD len;
	DWORD typ;

	DWORD rc;

	// @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
	// @pyparm string|valueName||The name of the value to set.
	// If a value with this name is not already present in the key, the method adds it to the key.
	// <nl>If this parameter is None or an empty string and the type parameter is the win32api.REG_SZ type, this function sets the same value the <om win32api.RegSetValue> method would set.
	// @pyparm any|reserved||Place holder for reserved argument.  Zero will always be passed to the API function.
	// @pyparm int|type||Type of data. 
	// @flagh Value|Meaning 
	// @flag REG_BINARY|Binary data in any form. 
	// @flag REG_DWORD|A 32-bit number. 
	// @flag REG_DWORD_LITTLE_ENDIAN|A 32-bit number in little-endian format. This is equivalent to REG_DWORD.<nl>In little-endian format, a multi-byte value is stored in memory from the lowest byte (the little end) to the highest byte. For example, the value 0x12345678 is stored as (0x78 0x56 0x34 0x12) in little-endian format. 
	// Windows NT and Windows 95 are designed to run on little-endian computer architectures. A user may connect to computers that have big-endian architectures, such as some UNIX systems. 
	// @flag REG_DWORD_BIG_ENDIAN|A 32-bit number in big-endian format.
	// In big-endian format, a multi-byte value is stored in memory from the highest byte (the big end) to the lowest byte. For example, the value 0x12345678 is stored as (0x12 0x34 0x56 0x78) in big-endian format. 
	// @flag REG_EXPAND_SZ|A null-terminated string that contains unexpanded references to environment variables (for example, %PATH%). It will be a Unicode or ANSI string depending on whether you use the Unicode or ANSI functions. 
	// @flag REG_LINK|A Unicode symbolic link. 
	// @flag REG_MULTI_SZ|An array of null-terminated strings, terminated by two null characters. 
	// @flag REG_NONE|No defined value type.
	// @flag REG_RESOURCE_LIST|A device-driver resource list. 
	// @flag REG_SZ|A null-terminated string. It will be a Unicode or ANSI string depending on whether you use the Unicode or ANSI functions
 

	// @pyparm registry data|value||The value to be stored with the specified value name.
	if (!PyArg_ParseTuple(args, "OzOiO:RegSetValueEx", &obKey, &valueName, &obRes, &typ, &value))
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi RegSetValueEx
	if (!Py2Reg(value, typ, &data, &len))
	{
		if (!PyErr_Occurred())
			PyErr_SetString(PyExc_ValueError, 
		                	"Could not convert the data to the specified type.");
		return NULL;
	}
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegSetValueEx(hKey, valueName, NULL, typ, data, len );
	PyW32_END_ALLOW_THREADS
	PyMem_Free((char *)data);
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegSetValueEx", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm  This method can also set additional value and type information for the specified key.
	// <nl>The key identified by the key parameter must have been opened with KEY_SET_VALUE access.
	// To open the key, use the <om win32api.RegCreateKeyEx> or <om win32api.RegOpenKeyEx> methods.
	// <nl>Value lengths are limited by available memory. 
	// Long values (more than 2048 bytes) should be stored as files with the filenames stored in the configuration registry.
	// This helps the registry perform efficiently.
	// <nl>The key identified by the key parameter must have been opened with KEY_SET_VALUE access.
}

// @pymethod |win32api|RegSetKeySecurity|Sets the security on the specified registry key.
static PyObject *PyRegSetKeySecurity(PyObject *self, PyObject *args)
{
	long si;
	HKEY hKey;
	PyObject *obKey, *obSD;
	DWORD rc;
	PSECURITY_DESCRIPTOR psd;
	if (!PyArg_ParseTuple(args, "OlO:RegSetKeySecurity", 
		&obKey, // @pyparm <o PyHKEY>/int|key||Handle to an open key for which the security descriptor is set.
		&si, //@pyparm int|security_info||] Specifies the components of the security descriptor to set. The value can be a combination of the *_SECURITY_INFORMATION constants.
		&obSD)) // @pyparm <o PySECURITY_DESCRIPTOR>|sd||The new security descriptor for the key
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obSD, &psd, FALSE))
		return NULL;
	// @pyseeapi PyRegSetKeySecurity
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegSetKeySecurity(hKey, si, psd);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS)
		return ReturnAPIError("RegSetKeySecurity", rc);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm If key is one of the predefined keys, the predefined key should be closed with <om win32api.RegCloseKey>. That ensures that the new security information is in effect the next time the predefined key is referenced.
}

// @pymethod <o PySECURITY_DESCRIPTOR>|win32api|RegGetKeySecurity|Retrieves the security on the specified registry key.
static PyObject *PyRegGetKeySecurity(PyObject *self, PyObject *args)
{
	long si;
	HKEY hKey;
	PyObject *obKey;
	if (!PyArg_ParseTuple(args, "Ol:RegGetKeySecurity", 
		&obKey, // @pyparm <o PyHKEY>/int|key||Handle to an open key for which the security descriptor is set.
		&si)) //@pyparm int|security_info||Specifies the components of the security descriptor to retrieve. The value can be a combination of the *_SECURITY_INFORMATION constants.
		return NULL;
	if (!PyWinObject_AsHKEY(obKey, &hKey))
		return NULL;
	// @pyseeapi PyRegGetKeySecurity
	DWORD cb = 0;
	DWORD rc;
	PyW32_BEGIN_ALLOW_THREADS
	rc=RegGetKeySecurity(hKey, si, NULL, &cb);
	PyW32_END_ALLOW_THREADS
	if (rc!=ERROR_INSUFFICIENT_BUFFER)
		return ReturnAPIError("RegGetKeySecurity", rc);
	PSECURITY_DESCRIPTOR psd = (SECURITY_DESCRIPTOR *)malloc(cb);
	if (psd==NULL)
		return PyErr_NoMemory();
	Py_BEGIN_ALLOW_THREADS
	rc=RegGetKeySecurity(hKey, si, psd, &cb);
	Py_END_ALLOW_THREADS
	if (rc!=ERROR_SUCCESS) {
		free(psd);
		return ReturnAPIError("RegGetKeySecurity", rc);
	}
	PyObject *ret = PyWinObject_FromSECURITY_DESCRIPTOR(psd);
	free(psd);
	return ret;
}

// @pymethod |win32api|RegisterWindowMessage|The RegisterWindowMessage method, given a string, returns a system wide unique
// message ID, suitable for sending messages between applications who both register the same string.
static PyObject *
PyRegisterWindowMessage( PyObject *self, PyObject *args )
{
	char *msgString;
	UINT msgID;

	// @pyparm string|msgString||The name of the message to register.
	// All applications that register this message string will get the same message.
	// ID back.  It will be unique in the system and suitable for applications to 
	// use to exchange messages.
	if (!PyArg_ParseTuple(args, "s:RegisterWindowMessage", &msgString))
		return NULL;
	// @pyseeapi RegisterWindowMessage
	PyW32_BEGIN_ALLOW_THREADS
	msgID=RegisterWindowMessage(msgString);
	PyW32_END_ALLOW_THREADS
	if (msgID==0)
		return ReturnAPIError("RegisterWindowMessage", msgID);
	return Py_BuildValue("i",msgID);
	// @comm Only use RegisterWindowMessage when more than one application must process the 
	// <nl> same message. For sending private messages within a window class, an application
	// <nl> can use any integer in the range WM_USER through 0x7FFF. (Messages in this range
	// <nl> are private to a window class, not to an application. For example, predefined 
	// <nl> control classes such as BUTTON, EDIT, LISTBOX, and COMBOBOX may use values in
	// <nl> this range.) 
}

// @pymethod int|win32api|SearchPath|Searches a path for the specified file.
static PyObject *
PySearchPath (PyObject *self, PyObject *args)
{
	char *szPath, *szFileName, *szExt = NULL;
	char retBuf[512], *szBase;
	// @pyparm string|path||The path to search.  If None, searches the standard paths.
	// @pyparm string|fileName||The name of the file to search for.
	// @pyparm string|fileExt|None|specifies an extension to be added to the filename when searching for the file.
	// The first character of the filename extension must be a period (.).
	// The extension is added only if the specified filename does not end with an extension.
	// If a filename extension is not required or if the filename contains an extension, this parameter can be None.
	if (!PyArg_ParseTuple (args, "zs|z:SearchPath", &szPath, &szFileName, &szExt))
		return NULL;
	DWORD rc;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi SearchPath
	rc = ::SearchPath(szPath, szFileName, szExt, sizeof(retBuf), retBuf, &szBase );
	PyW32_END_ALLOW_THREADS
	if (rc==0)
		return ReturnAPIError("SearchPath");
	return Py_BuildValue("si", retBuf, (szBase-retBuf) );
	// @rdesc The return value is a tuple of (string, int).  string is the full
	// path name located.  int is the offset in the string of the base name
	// of the file.
}
// @pymethod |win32api|SendMessage|Send a message to a window.
PyObject *PySendMessage(PyObject *self, PyObject *args)
{
	HWND hwnd;
	int message;
	int wParam=0;
	int lParam=0;
	if (!PyArg_ParseTuple(args, "ii|ii:SendMessage",
	          &hwnd,    // @pyparm int|hwnd||The hWnd of the window to receive the message.
		      &message, // @pyparm int|idMessage||The ID of the message to send.
	          &wParam,  // @pyparm int|wParam||The wParam for the message
	          &lParam)) // @pyparm int|lParam||The lParam for the message

		return NULL;
	int rc;
	// @pyseeapi SendMessage
	PyW32_BEGIN_ALLOW_THREADS
	rc = ::SendMessage(hwnd, message, wParam, lParam);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("i",rc);
}

// @pymethod |win32api|SetConsoleTitle|Sets the title for the current console.
static PyObject *
PySetConsoleTitle( PyObject *self, PyObject *args )
{
	char *title;
	// @pyparm string|title||The new title
	if (!PyArg_ParseTuple(args, "s:SetConsoleTitle", &title))
		return NULL;
	// @pyseeapi SetConsoleTitle
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::SetConsoleTitle(title);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("SetConsoleTitle");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|SetCursorPos|The SetCursorPos function moves the cursor to the specified screen coordinates.
static PyObject *
PySetCursorPos( PyObject *self, PyObject *args )
{
	int x,y;
	// @pyparm (int, int)|x,y||The new position.
	if (!PyArg_ParseTuple(args, "(ii):SetCursorPos", &x, &y))
		return NULL;
	// @pyseeapi SetCursorPos
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::SetCursorPos(x,y);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("SetCursorPos");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|SetErrorMode|Controls whether the system will handle the specified types of serious errors, or whether the process will handle them.
static PyObject *
PySetErrorMode( PyObject *self, PyObject *args )
{
	int mode;
	// @pyparm int|errorMode||A set of bit flags that specify the process error mode
	if (!PyArg_ParseTuple(args, "i:SetErrorMode", &mode))
		return NULL;
	// @pyseeapi SetErrorMode
	PyW32_BEGIN_ALLOW_THREADS
	UINT ret = ::SetErrorMode(mode);
	PyW32_END_ALLOW_THREADS
	// @rdesc The result is an integer containing the old error flags.
	return PyInt_FromLong(ret);
}

// @pymethod int|win32api|ShowCursor|The ShowCursor method displays or hides the cursor. 
static PyObject *
PyShowCursor( PyObject *self, PyObject *args )
{
	BOOL bShow;
	// @pyparm int|show||Visiblilty flag
	if (!PyArg_ParseTuple(args, "i:ShowCursor", &bShow))
		return NULL;
	// @pyseeapi ShowCursor
	PyW32_BEGIN_ALLOW_THREADS
	int rc = ::ShowCursor(bShow);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("i", rc);
	// @rdesc The return value specifies the new display counter
	// @comm This function sets an internal display counter that 
	// determines whether the cursor should be displayed. The 
	// cursor is displayed only if the display count is greater 
	// than or equal to 0. If a mouse is installed, the initial display 
	// count is 0. If no mouse is installed, the display count is -1. 
}


// @pymethod int|win32api|ShellExecute|Opens or prints a file.
static PyObject *
PyShellExecute( PyObject *self, PyObject *args )
{
	HWND hwnd;
	char *op, *file, *params, *dir;
	int show;
	if (!PyArg_ParseTuple(args, "izszzi:ShellExecute", 
		      &hwnd, // @pyparm int|hwnd||The handle of the parent window, or 0 for no parent.  This window receives any message boxes an application produces (for example, for error reporting).
		      &op,   // @pyparm string|op||The operation to perform.  May be "open", "print", or None, which defaults to "open".
		      &file, // @pyparm string|file||The name of the file to open.
		      &params,// @pyparm string|params||The parameters to pass, if the file name contains an executable.  Should be None for a document file.
		      &dir,  // @pyparm string|dir||The initial directory for the application.
		      &show))// @pyparm int|bShow||Specifies whether the application is shown when it is opened. If the lpszFile parameter specifies a document file, this parameter is zero.
		return NULL;
	if (dir==NULL)
		dir="";
	PyW32_BEGIN_ALLOW_THREADS
	HINSTANCE rc=::ShellExecute(hwnd, op, file, params, dir, show);
	PyW32_END_ALLOW_THREADS
	// @pyseeapi ShellExecute
	if ((rc) <= (HINSTANCE)32) {
		return ReturnAPIError("ShellExecute", (int)rc );
	}
	return Py_BuildValue("i", rc );
	// @rdesc The instance handle of the application that was run. (This handle could also be the handle of a dynamic data exchange [DDE] server application.)
	// If there is an error, the method raises an exception.
}
// @pymethod int|win32api|Sleep|Suspends execution of the current thread for the specified time.
static PyObject *
PySleep (PyObject *self, PyObject *args)
{
	BOOL bAlertable = FALSE;
	int time;
	// @pyparm int|time||The number of milli-seconds to sleep for,
	// @pyparm int|bAlterable|0|Specifies whether the function may terminate early due to an I/O completion callback function.
	if (!PyArg_ParseTuple (args, "i|i:Sleep", &time, &bAlertable))
		return NULL;
	DWORD rc;
	PyW32_BEGIN_ALLOW_THREADS
	// @pyseeapi Sleep
	// @pyseeapi SleepEx
	rc = ::SleepEx(time, bAlertable);
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("i", rc );
	// @rdesc The return value is zero if the specified time interval expired. 
}
// @pymethod |win32api|WinExec|Runs the specified application.
static PyObject *
PyWinExec( PyObject *self, PyObject *args )
{
	char *cmd;
	int style = SW_SHOWNORMAL;
	// @pyparm string|cmdLine||The command line to execute.
	// @pyparm int|show|win32con.SW_SHOWNORMAL|The initial state of the applications window.
	if (!PyArg_ParseTuple(args, "s|i:WinExec", &cmd, &style))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	int rc=::WinExec(cmd, style);
	PyW32_END_ALLOW_THREADS
	if ((rc)<=32) // @pyseeapi WinExec
		return ReturnAPIError("WinExec", rc );
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|WinHelp|Invokes the Windows Help system.
static PyObject *
PyWinHelp( PyObject *self, PyObject *args )
{
	HWND hwnd;
	char *hlpFile;
	UINT cmd;
	PyObject *dataOb = Py_None;
	DWORD data;
	if (!PyArg_ParseTuple(args, "isi|O:WinHelp",
		      &hwnd,   // @pyparm int|hwnd||The handle of the window requesting help.
			  &hlpFile,// @pyparm string|hlpFile||The name of the help file.
			  &cmd,    // @pyparm int|cmd||The type of help.  See the api for full details.
			  &dataOb))   // @pyparm int/string|data|0|Additional data specific to the help call.
		return NULL;
	if (dataOb==Py_None)
		data = 0;
	else if (PyString_Check(dataOb))
		data = (DWORD)PyString_AsString(dataOb);
	else if (PyInt_Check(dataOb))
		data = (DWORD)PyInt_AsLong(dataOb);
	else {
		PyErr_SetString(PyExc_TypeError, "4th argument must be a None, string or an integer.");
		return NULL;
	}
		
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = ::WinHelp(hwnd, hlpFile, cmd, data);
	PyW32_END_ALLOW_THREADS
	if (!ok) // @pyseeapi WinHelp
		return ReturnAPIError("WinHelp");
	Py_INCREF(Py_None);
	return Py_None;
	// @rdesc The method raises an exception if an error occurs.
}


// @pymethod |win32api|WriteProfileVal|Writes a value to a Windows INI file.
static PyObject *
PyWriteProfileVal(PyObject *self, PyObject *args)
{
	char *sect, *entry, *strVal, *iniFile=NULL;
	int intVal;
	BOOL bHaveInt = TRUE;

	if (!PyArg_ParseTuple(args, "ssi|s:WriteProfileVal", 
	          &sect,  // @pyparm string|section||The section in the INI file to write to.
	          &entry, // @pyparm string|entry||The entry within the section in the INI file to write to.
	          &intVal, // @pyparm int/string|value||The value to write.
	          &iniFile)) { // @pyparm string|iniName|None|The name of the INI file.  If None, the system INI file is used.
		bHaveInt = FALSE;
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "ssz|s", &sect, &entry, &strVal, &iniFile)) {
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "WriteProfileVal must have format (ssi|s) or (ssz|s)");
			return NULL;
		}
	}
	BOOL rc;
	char intBuf[20];
	if (bHaveInt) {
		itoa( intVal, intBuf, 10 );
		strVal = intBuf;
	}

	// @pyseeapi WritePrivateProfileString
	// @pyseeapi WriteProfileString
	PyW32_BEGIN_ALLOW_THREADS
	if (iniFile)
		rc = ::WritePrivateProfileString( sect, entry, strVal, iniFile );
	else
		rc = ::WriteProfileString( sect, entry, strVal );
	PyW32_END_ALLOW_THREADS

	if (!rc)
		return ReturnAPIError("Write[Private]ProfileString");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|MessageBeep|Plays a predefined waveform sound.
static PyObject *
PyMessageBeep(PyObject *self, PyObject *args)
{
	// @comm The waveform sound for each sound type is identified by an entry in the [sounds] section of the registry.
	int val = MB_OK;

	if (!PyArg_ParseTuple(args, "|i:MessageBeep", 
	          &val)) // @pyparm int|type|win32con.MB_OK|Specifies the sound type, as
	          // identified by an entry in the [sounds] section of the
	          // registry. This parameter can be one of MB_ICONASTERISK,
	          // MB_ICONEXCLAMATION, MB_ICONHAND, MB_ICONQUESTION or MB_OK.
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = MessageBeep(val);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("MessageBeep");
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod int|win32api|MessageBox|Display a message box.
static PyObject *
PyMessageBox(PyObject * self, PyObject * args)
{
  char *message;
  long style = MB_OK;
  const char *title = NULL;
  HWND hwnd = NULL;
  WORD langId = MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT);
  // @pyparm int|hwnd||The handle of the parent window.  See the comments section.
  // @pyparm string|message||The message to be displayed in the message box.
  // @pyparm string/None|title||The title for the message box.  If None, the applications title will be used.
  // @pyparm int|style|win32con.MB_OK|The style of the message box.
  // @pyparm int|language|win32api.MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)|The language ID to use.

  // @comm Normally, a program in a GUI environment will use one of the MessageBox
  // methods supplied by the GUI (eg, <om win32ui.MessageBox> or <om PyCWnd.MessageBox>)
  if (!PyArg_ParseTuple(args, "is|zli:MessageBox(Ex)", &hwnd, &message, &title, &style, &langId))
    return NULL;
  PyW32_BEGIN_ALLOW_THREADS
  int rc = ::MessageBoxEx(hwnd, message, title, style, langId);
  PyW32_END_ALLOW_THREADS
  return Py_BuildValue("i",rc);
  // @rdesc An integer identifying the button pressed to dismiss the dialog.
}


// @pymethod int|win32api|SetFileAttributes|Sets the named file's attributes.
static PyObject *
PySetFileAttributes(PyObject * self, PyObject * args)
{
	char *pathName;
	int attrs;
	// @pyparm string|pathName||The name of the file.
	// @pyparm int|attrs||The attributes to set.  Must be a combination of the win32con.FILE_ATTRIBUTE_* constants.
	if (!PyArg_ParseTuple(args, "si:SetFileAttributes", &pathName, &attrs))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = SetFileAttributes(pathName, attrs);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("SetFileAttributes");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|GetWindowLong|Retrieves a long value at the specified offset into the extra window memory of the given window.
static PyObject *
PyGetWindowLong(PyObject * self, PyObject * args)
{
	int hwnd;
	int offset;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|offset||Specifies the zero-based byte offset of the value to change. Valid values are in the range zero through the number of bytes of extra window memory, minus four (for example, if 12 or more bytes of extra memory were specified, a value of 8 would be an index to the third long integer), or one of the GWL_ constants.
	if (!PyArg_ParseTuple(args, "ii:GetWindowLong", &hwnd, &offset))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::GetWindowLong( (HWND)hwnd, offset );
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("l", rc);
}

// @pymethod int|win32api|SetWindowLong|Places a long value at the specified offset into the extra window memory of the given window.
static PyObject *
PySetWindowLong(PyObject * self, PyObject * args)
{
	int hwnd;
	int offset;
	long newVal;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|offset||Specifies the zero-based byte offset of the value to change. Valid values are in the range zero through the number of bytes of extra window memory, minus four (for example, if 12 or more bytes of extra memory were specified, a value of 8 would be an index to the third long integer), or one of the GWL_ constants.
	// @pyparm int|val||Specifies the long value to place in the window's reserved memory.
	if (!PyArg_ParseTuple(args, "iil:SetWindowLong", &hwnd, &offset, &newVal))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::SetWindowLong( (HWND)hwnd, offset, newVal ) ;
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("l", rc);
}
// @pymethod int|win32api|SetWindowWord|
static PyObject *
PySetWindowWord(PyObject * self, PyObject * args)
{
	int hwnd;
	int offset;
	WORD newVal;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|offset||Specifies the zero-based byte offset of the value to change. Valid values are in the range zero through the number of bytes of extra window memory, minus four (for example, if 12 or more bytes of extra memory were specified, a value of 8 would be an index to the third long integer), or one of the GWL_ constants.
	// @pyparm int|val||Specifies the long value to place in the window's reserved memory.
	if (!PyArg_ParseTuple(args, "iii:SetWindowWord", &hwnd, &offset, (int *)(&newVal)))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::SetWindowWord( (HWND)hwnd, offset, newVal );
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("l", rc);
}

// @pymethod int|win32api|SetClassLong|Replaces the specified 32-bit (long) value at the specified offset into the extra class memory for the window.
static PyObject *
PySetClassLong(PyObject * self, PyObject * args)
{
	int hwnd;
	int offset;
	long newVal;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|offset||Specifies the zero-based byte offset of the value to change. Valid values are in the range zero through the number of bytes of extra window memory, minus four (for example, if 12 or more bytes of extra memory were specified, a value of 8 would be an index to the third long integer), or one of the GWL_ constants.
	// @pyparm int|val||Specifies the long value to place in the window's reserved memory.
	if (!PyArg_ParseTuple(args, "iil:SetClassLong", &hwnd, &offset, &newVal))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::SetClassLong( (HWND)hwnd, offset, newVal );
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("l", rc);
}
// @pymethod int|win32api|SetClassWord|
static PyObject *
PySetClassWord(PyObject * self, PyObject * args)
{
	int hwnd;
	int offset;
	WORD newVal;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|offset||Specifies the zero-based byte offset of the value to change. Valid values are in the range zero through the number of bytes of extra window memory, minus four (for example, if 12 or more bytes of extra memory were specified, a value of 8 would be an index to the third long integer), or one of the GWL_ constants.
	// @pyparm int|val||Specifies the long value to place in the window's reserved memory.
	if (!PyArg_ParseTuple(args, "iii:SetClassWord", &hwnd, &offset, (int *)(&newVal)))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	long rc = ::SetClassWord( (HWND)hwnd, offset, newVal );
	PyW32_END_ALLOW_THREADS
	return Py_BuildValue("l", rc);
}

/***** SOME "MACROS" ******/
// @pymethod int|win32api|MAKELANGID|Creates a language identifier from a primary language identifier and a sublanguage identifier.
static PyObject *
PyMAKELANGID(PyObject *self, PyObject *args)
{
	int lang, sublang;
	if (!PyArg_ParseTuple(args, "ii:MAKELANGID", 
	          &lang,  // @pyparm int|PrimaryLanguage||Primary language identifier

	          &sublang)) // @pyparm int|SubLanguage||The sublanguage identifier
		return NULL;
	return Py_BuildValue("i", (int)MAKELANGID(lang, sublang));
	// @comm This is simply a wrapper to a C++ macro.
}

// @pymethod int|win32api|HIWORD|An interface to the win32api HIWORD macro.
static PyObject *
PyHIWORD(PyObject *self, PyObject *args)
{
	int val;

	if (!PyArg_ParseTuple(args, "i:HIWORD", 
	          &val)) // @pyparm int|val||The value to retrieve the HIWORD from.
		return NULL;
	return Py_BuildValue("i", (int)HIWORD(val));
	// @comm This is simply a wrapper to a C++ macro.
}
// @pymethod int|win32api|LOWORD|An interface to the win32api LOWORD macro.
static PyObject *
PyLOWORD(PyObject *self, PyObject *args)
{
	int val;

	if (!PyArg_ParseTuple(args, "i:LOWORD", 
	          &val)) // @pyparm int|val||The value to retrieve the LOWORD from.
		return NULL;
	return Py_BuildValue("i", (int)LOWORD(val));
	// @comm This is simply a wrapper to a C++ macro.
}
// @pymethod int|win32api|HIBYTE|An interface to the win32api HIBYTE macro.
static PyObject *
PyHIBYTE(PyObject *self, PyObject *args)
{
	int val;

	if (!PyArg_ParseTuple(args, "i:HIBYTE", 
	          &val)) // @pyparm int|val||The value to retrieve the HIBYTE from.
		return NULL;
	return Py_BuildValue("i", (int)HIBYTE(val));
	// @comm This is simply a wrapper to a C++ macro.
}
// @pymethod int|win32api|LOBYTE|An interface to the win32api LOBYTE macro.
static PyObject *
PyLOBYTE(PyObject *self, PyObject *args)
{
	int val;

	if (!PyArg_ParseTuple(args, "i:LOBYTE", 
	          &val)) // @pyparm int|val||The value to retrieve the LOBYTE from.
		return NULL;
	return Py_BuildValue("i", (int)LOBYTE(val));
	// @comm This is simply a wrapper to a C++ macro.
}

// @pymethod int|win32api|MAKEWORD|creates a WORD value by concatenating the specified values.
static PyObject *
PyMAKEWORD(PyObject *self, PyObject *args)
{
	int hi, lo;
	if (!PyArg_ParseTuple(args, "ii:MAKEWORD", 
	          &lo,  // @pyparm int|low||Specifies the low-order byte of the new value. 
	          &hi)) // @pyparm int|high||Specifies the high-order byte of the new value. 
		return NULL;
	return Py_BuildValue("i", (int)MAKEWORD(lo, hi));
	// @comm This is simply a wrapper to a C++ macro.
}

// @pymethod int|win32api|MAKELONG|creates a LONG value by concatenating the specified values.
static PyObject *
PyMAKELONG(PyObject *self, PyObject *args)
{
	int hi, lo;
	if (!PyArg_ParseTuple(args, "ii:MAKELONG", 
	          &lo,  // @pyparm int|low||Specifies the low-order byte of the new value. 
	          &hi)) // @pyparm int|high||Specifies the high-order byte of the new value. 
		return NULL;
	return Py_BuildValue("i", (long)MAKELONG(lo, hi));
	// @comm This is simply a wrapper to a C++ macro.
}

// @pymethod int|win32api|RGB|An interface to the win32api RGB macro.
static PyObject *
PyRGB(PyObject *self, PyObject *args)
{
	int r,g,b;
	// @pyparm int|red||The red value
	// @pyparm int|green||The green value
	// @pyparm int|blue||The blue value
	if (!PyArg_ParseTuple(args, "iii:RGB", 
	          &r, &g, &b)) 
		return NULL;
	return Py_BuildValue("i", (int)RGB(r,g,b));
	// @comm This is simply a wrapper to a C++ macro.
}

// @pymethod tuple|win32api|GetSystemTime|Returns the current system time
static PyObject *
PyGetSystemTime (PyObject * self, PyObject * args)
{
  SYSTEMTIME t;
  if (!PyArg_ParseTuple (args, "")) {
 return NULL;
  } else {
 // GetSystemTime is a void function
 PyW32_BEGIN_ALLOW_THREADS
 GetSystemTime(&t);
 PyW32_END_ALLOW_THREADS;
 return Py_BuildValue ("(iiiiiiii)",
        t.wYear,
        t.wMonth,
        t.wDayOfWeek,
        t.wDay,
        t.wHour,
        t.wMinute,
        t.wSecond,
        t.wMilliseconds
        );
  }
}						  

// @pymethod tuple|win32api|GetLocalTime|Returns the current local time
static PyObject *
PyGetLocalTime (PyObject * self, PyObject * args)
{
  SYSTEMTIME t;
  if (!PyArg_ParseTuple (args, "")) {
 return NULL;
  } else {
 // GetLocalTime is a void function
 GetLocalTime(&t);
 return Py_BuildValue ("(iiiiiiii)",
        t.wYear,
        t.wMonth,
        t.wDayOfWeek,
        t.wDay,
        t.wHour,
        t.wMinute,
        t.wSecond,
        t.wMilliseconds
        );
  }
}


// @pymethod int|win32api|SetSystemTime|Returns the current system time
static PyObject *
PySetSystemTime (PyObject * self, PyObject * args)
{
  SYSTEMTIME t;
  int result;

  if (!PyArg_ParseTuple (args,
       "hhhhhhhh",
       &t.wYear,   	// @pyparm int|year||
       &t.wMonth, // @pyparm int|month||
       &t.wDayOfWeek, // @pyparm int|dayOfWeek||
       &t.wDay, // @pyparm int|day||
       &t.wHour,// @pyparm int|hour||
       &t.wMinute,// @pyparm int|minute||
       &t.wSecond,// @pyparm int|second||
       &t.wMilliseconds // @pyparm int|millseconds||
       ))
 return NULL;
 PyW32_BEGIN_ALLOW_THREADS
 result = ::SetSystemTime (&t);
 PyW32_END_ALLOW_THREADS

 if (! result ) {
 return ReturnAPIError ("SetSystemTime");
  } else {
 return Py_BuildValue ("i", result);
  }
}

// @pymethod |win32api|SetThreadLocale|Sets the current thread's locale.
static PyObject *
PySetThreadLocale(PyObject *self, PyObject *args)
{
	int lcid;
	// @pyparm int|lcid||The new LCID
	if (!PyArg_ParseTuple(args, "i:SetThreadLocale", &lcid))
		return NULL;
	SetThreadLocale(lcid);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32api|GetThreadLocale|Returns the current thread's locale.
static PyObject *
PyGetThreadLocale(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetThreadLocale"))
		return NULL;
	return PyInt_FromLong(GetThreadLocale());
}

// @pymethod |win32api|OutputDebugString|Sends a string to the Windows debugging device.
static PyObject *
PyOutputDebugString(PyObject *self, PyObject *args)
{
	char *msg;
	// @pyparm string|msg||The string to write.
	if (!PyArg_ParseTuple(args, "s:OutputDebugString", &msg))
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	OutputDebugString(msg);
	PyW32_END_ALLOW_THREADS;
	Py_INCREF(Py_None);
	return Py_None;
}

// Process stuff

// @pymethod <o PyHANDLE>|win32api|OpenProcess|Retrieves a handle to an existing process
static PyObject *PyOpenProcess(PyObject *self, PyObject *args)
{
	DWORD pid, reqdAccess;
	BOOL inherit;
	if (!PyArg_ParseTuple(args, "iil:OpenProcess",
			&reqdAccess, // @pyparm int|reqdAccess||The required access.
			&inherit,    // @pyparm int|bInherit||Specifies whether the returned handle can be inherited by a new process created by the current process. If TRUE, the handle is inheritable.
			&pid)) // @pyparm int|pid||The process ID
		return NULL;
	PyW32_BEGIN_ALLOW_THREADS
	HANDLE handle = OpenProcess(reqdAccess, inherit, pid);
	PyW32_END_ALLOW_THREADS;
	if (handle==NULL)
		return ReturnAPIError("OpenProcess");
	return PyWinObject_FromHANDLE(handle);
}

// @pymethod |win32api|TerminateProcess|Kills a process
static PyObject *PyTerminateProcess(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	HANDLE handle;
	UINT exitCode;
	if (!PyArg_ParseTuple(args, "Oi:TerminateProcess",
			&obHandle, // @pyparm <o PyHANDLE>|handle||The handle of the process to terminate.
	        &exitCode)) // @pyparm int|exitCode||The exit code for the process.
		return NULL;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;
	// @comm See also <om win32api.OpenProcess>
	PyW32_BEGIN_ALLOW_THREADS
	BOOL ok = TerminateProcess(handle, exitCode);
	PyW32_END_ALLOW_THREADS
	if (!ok)
		return ReturnAPIError("TerminateProcess");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyUnicode>|win32api|LoadString|Loads a string from a resource file.
static PyObject * PyLoadString(PyObject *self, PyObject *args)
{
	HMODULE hModule;
	int numChars = 1024, gotChars=0;
	UINT stringId;
	if ( !PyArg_ParseTuple(args, "ii|i",
						   &hModule, // @pyparm int|handle||The handle of the module containing the resource.
						   &stringId, // @pyparm int|stringId||The ID of the string to load.
						   &numChars)) // @pyparm int|numChars|1024|Number of characters to allocate for the return buffer.
		return NULL;
	int numBytes = sizeof(WCHAR) * numChars;
	WCHAR *buffer = (WCHAR *)malloc(numBytes);
	if (buffer==NULL)
		return PyErr_Format(PyExc_MemoryError, "Allocating buffer of %d bytes for LoadString", numBytes);
	gotChars = LoadStringW(hModule, stringId, buffer, numChars);
	PyObject *rc;
	if (gotChars==0)
		rc = ReturnAPIError("LoadString");
	else
		rc = PyWinObject_FromWCHAR(buffer, gotChars);
	free(buffer);
	return rc;
}


// @pymethod string|win32api|LoadResource|Finds and loads a resource from a PE file.
static PyObject * PyLoadResource(PyObject *self, PyObject *args)
{
	HMODULE hModule;
	PyObject *obType;
	PyObject *obName;
	WORD wLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	if ( !PyArg_ParseTuple(args, "iOO|i",
						   &hModule, // @pyparm int|handle||The handle of the module containing the resource.
						   &obType, // @pyparm object|type||The type of resource to load.
						   &obName, // @pyparm object|name||The name of the resource to load.
						   &wLanguage // @pyparm int|language|NEUTRAL|Language to use, defaults to LANG_NEUTRAL.
		) )
		return NULL;


	BOOL bFreeType = FALSE, bFreeName = FALSE;
	LPTSTR lpType;
	if ( PyInt_Check(obType) )
		lpType = MAKEINTRESOURCE(PyInt_AS_LONG((PyIntObject *)obType));
	else if (PyWinObject_AsTCHAR(obType, &lpType))
		bFreeType = TRUE;
	else
		return ReturnError("Bad type for resource type.", "LoadResource");

	LPTSTR lpName;
	if ( PyInt_Check(obName) )
		lpName = MAKEINTRESOURCE(PyInt_AS_LONG((PyIntObject *)obName));
	else if (PyWinObject_AsTCHAR(obName, &lpName))
		bFreeName = TRUE;
	else {
		if (bFreeType) PyWinObject_FreeTCHAR(lpType);
		return ReturnError("Bad type for resource name.", "LoadResource");
	}

	HRSRC hrsrc = FindResourceEx(hModule, lpType, lpName, wLanguage);
	if (bFreeType) PyWinObject_FreeTCHAR(lpType);
	if (bFreeName) PyWinObject_FreeTCHAR(lpName);
	if ( hrsrc == NULL )
		return ReturnAPIError("LoadResource");

	DWORD size = SizeofResource(hModule, hrsrc);
	if ( size == 0 )
		return ReturnAPIError("LoadResource");

	HGLOBAL hglob = LoadResource(hModule, hrsrc);
	if ( hglob == NULL )
		return ReturnAPIError("LoadResource");

	LPVOID p = LockResource(hglob);
	if ( p == NULL )
		return ReturnAPIError("LoadResource");

	return PyString_FromStringAndSize((char *)p, size);
}

// @pymethod |win32api|BeginUpdateResource|Begins an update cycle for a PE file.
static PyObject * PyBeginUpdateResource(PyObject *self, PyObject *args)
{
	const char *szFileName;
	int bDeleteExistingResources;

	if ( !PyArg_ParseTuple(args, "si",
						   &szFileName, // @pyparm string|filename||File in which to update resources.
						   &bDeleteExistingResources // @pyparm int|delete||Flag to indicate that all existing resources should be deleted.
		) )
		return NULL;

	HANDLE h = BeginUpdateResource(szFileName, bDeleteExistingResources);
	if ( h == NULL )
		return ReturnAPIError("BeginUpdateResource");

	return PyInt_FromLong((int)h);
}

// @pymethod |win32api|UpdateResource|Updates a resource in a PE file.
static PyObject * PyUpdateResource(PyObject *self, PyObject *args)
{
	HMODULE hUpdate;
	PyObject *obType;
	PyObject *obName;
	LPVOID lpData;
	DWORD cbData;
	WORD wLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	if ( !PyArg_ParseTuple(args, "iOOs#|i",
						   &hUpdate, // @pyparm int|handle||The update-file handle.
						   &obType, // @pyparm object|type||The type of resource to load.
						   &obName, // @pyparm object|name||The name of the resource to load.
						   &lpData, // @pyparm string|data||The data to place into the resource.
						   &cbData,
						   &wLanguage // @pyparm int|language|NEUTRAL|Language to use, defaults to LANG_NEUTRAL.
		) )
		return NULL;

	BOOL bFreeType = FALSE, bFreeName = FALSE;
	LPWSTR lpType;
	if ( PyInt_Check(obType) )
		lpType = MAKEINTRESOURCEW(PyInt_AS_LONG((PyIntObject *)obType));
	else if (PyWinObject_AsWCHAR(obType, &lpType) )
		bFreeType = TRUE;
	else
		return ReturnError("Bad type for resource type.", "UpdateResource");

	LPWSTR lpName;
	if ( PyInt_Check(obName) )
		lpName = MAKEINTRESOURCEW(PyInt_AS_LONG((PyIntObject *)obName));
	else if ( PyWinObject_AsWCHAR(obName, &lpName) )
		bFreeName = TRUE;
	else {
		if (bFreeType) PyWinObject_FreeBstr(lpType);
		return ReturnError("Bad type for resource name.", "UpdateResource");
	}

	BOOL ok = UpdateResourceW(hUpdate, lpType, lpName, wLanguage, lpData, cbData);
	if (bFreeType) PyWinObject_FreeWCHAR(lpType);
	if (bFreeName) PyWinObject_FreeWCHAR(lpName);
	if ( !ok )
		return ReturnAPIError("UpdateResource");

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32api|EndUpdateResource|Ends a resource update cycle of a PE file.
static PyObject * PyEndUpdateResource(PyObject *self, PyObject *args)
{
	HMODULE hUpdate;
	int fDiscard;

	if ( !PyArg_ParseTuple(args, "ii",
						   &hUpdate, // @pyparm int|handle||The update-file handle.
						   &fDiscard // @pyparm int|discard||Flag to discard all writes.
		) )
		return NULL;

	if ( !EndUpdateResource(hUpdate, fDiscard) )
		return ReturnAPIError("EndUpdateResource");

	Py_INCREF(Py_None);
	return Py_None;
}

BOOL PyWinObject_AsResourceID(PyObject *ob, long *resource_id)
{
	// resource names and types can be either string pointers or long ints
	if (PyWinObject_AsWCHAR(ob, (WCHAR **)resource_id))
		return TRUE;
	PyErr_Clear();
	if (PyInt_Check(ob)){
		*resource_id=PyInt_AsLong(ob);
		return TRUE;
		}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Resource name/type must be integer or string");
	return FALSE;
}

BOOL CALLBACK EnumResProc(HMODULE module, LPCSTR type, LPSTR name, PyObject
*param)
{
	PyObject *pyname;
	if (HIWORD(name) == 0)
	{
		pyname = PyInt_FromLong(reinterpret_cast<long>(name));
	}
	else if (name[0] == '#')
	{
		pyname = PyInt_FromLong(_ttoi(name + 1));
	}
	else
	{
		pyname = PyString_FromString(name);
	}
	PyList_Append(param, pyname);
	Py_DECREF(pyname);
	return TRUE;
}

// @pymethod [string, ...]|win32api|EnumResourceNames|Enumerates all the resources of the specified type from the nominated file.
PyObject *PyEnumResourceNames(PyObject *, PyObject *args)
{
	HMODULE hmodule;
	LPCSTR restype;
	char buf[20];
	// NOTE:  MH can't make the string version of the param
	// return anything useful, so I undocumented its use!
	// pyparm int|hmodule||The handle to the module to enumerate.
	// pyparm string|resType||The type of resource to enumerate as a string (eg, 'RT_DIALOG')
	if (!PyArg_ParseTuple(args, "is", &hmodule, &restype))
	{
		PyErr_Clear();
		int restypeint;
		// @pyparm int|hmodule||The handle to the module to enumerate.
		// @pyparm int|resType||The type of resource to enumerate as an integer (eg, win32con.RT_DIALOG)
		if (!PyArg_ParseTuple(args, "ii", &hmodule, &restypeint))
		{
			return NULL;
		}
		sprintf(buf, "#%d", restypeint);
		restype = buf;
	}
	// @rdesc The result is a list of string or integers, one for each resource enumerated.
	PyObject *result = PyList_New(0);
	EnumResourceNames(
		hmodule,
		restype,
		reinterpret_cast<ENUMRESNAMEPROC>(EnumResProc),
		reinterpret_cast<LONG>(result));

	return result;
}

BOOL CALLBACK EnumResourceTypesProc(HMODULE hmodule, WCHAR* typname, PyObject *ret)
{
	PyObject *obname=NULL;
	if (IS_INTRESOURCE(typname))
		obname=PyInt_FromLong((LONG)typname);
	else
		obname=PyWinObject_FromWCHAR(typname);
	if (obname==NULL)
		return FALSE;
	PyList_Append(ret, obname);
	Py_DECREF(obname);
	return TRUE;
}

// @pymethod [<o PyUnicode>,...]|win32api|EnumResourceTypes|Return name or integer id of all resource types contained in module
PyObject *PyEnumResourceTypes(PyObject *, PyObject *args)
{
	PyObject *ret=NULL, *pyhandle=NULL;
	HMODULE hmodule;

	// @pyparm <o PyHandle>|hmodule||The handle to the module to enumerate.
	if (!PyArg_ParseTuple(args, "O:EnumResourceTypes", &pyhandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(pyhandle, (void **)&hmodule))
		return NULL;
	ret=PyList_New(0);
	if(!EnumResourceTypesW(hmodule, 
			reinterpret_cast<ENUMRESTYPEPROCW>(EnumResourceTypesProc),
			reinterpret_cast<LONG>(ret))){
		Py_DECREF(ret);
		ret=NULL;
		PyWin_SetAPIError("EnumResourceTypes",GetLastError());
		}
	return ret;
}

BOOL CALLBACK EnumResourceLanguagesProc(HMODULE hmodule, WCHAR* typname, WCHAR *resname, WORD wIDLanguage, PyObject *ret)
{
	long resid;
	resid=wIDLanguage;
	PyObject *oblangid = PyInt_FromLong(resid);
	PyList_Append(ret, oblangid);
	Py_DECREF(oblangid);
	return TRUE;
}

// @pymethod [<o PyUnicode>,...]|win32api|EnumResourceLanguages|List languages for a resource
PyObject *PyEnumResourceLanguages(PyObject *, PyObject *args)
{
	PyObject *ret=NULL, *pyhandle=NULL;
	HMODULE hmodule;
	WCHAR *resname=NULL, *typname=NULL;
	PyObject *obresname=NULL, *obtypname=NULL;
		// @pyparm <o PyHandle>|hmodule||Handle to the module that contains resource
		// @pyparm string/unicode/int|lpType||Resource type, can be string or integer
		// @pyparm string/unicode/int|lpName||Resource name, can be string or integer
	if (!PyArg_ParseTuple(args, "OOO:EnumResourceLanguages", &pyhandle, &obtypname, &obresname))
		return NULL;
	if (!PyWinObject_AsHANDLE(pyhandle, (void **)&hmodule))
		return NULL;
	if(!PyWinObject_AsResourceID(obtypname,(long *)&typname))
		goto done;
	if(!PyWinObject_AsResourceID(obresname,(long *)&resname))
		goto done;
	ret=PyList_New(0);
	if(!EnumResourceLanguagesW(hmodule,
			typname,
			resname,
			reinterpret_cast<ENUMRESLANGPROCW>(EnumResourceLanguagesProc),
			reinterpret_cast<LONG>(ret))){
		Py_DECREF(ret);
		ret=NULL;
		PyWin_SetAPIError("EnumResourceLanguages",GetLastError());
		}
done:
	if ((typname!=NULL)&&(!IS_INTRESOURCE(typname)))
		PyWinObject_FreeWCHAR(typname);
	if ((resname!=NULL)&&(!IS_INTRESOURCE(resname)))
		PyWinObject_FreeWCHAR(resname);
	return ret;
}

// @pymethod <o PyUnicode>|win32api|Unicode|Creates a new Unicode object
PYWINTYPES_EXPORT PyObject *PyWin_NewUnicode(PyObject *self, PyObject *args);

/****** Now uses pywintypes version!
static PyObject *Py_Unicode(PyObject *self, PyObject *args)
{
	PyObject *obString;
	// @pyparm string|str||The string to convert.
	if (!PyArg_ParseTuple(args, "O", &obString))
		return NULL;

	PyUnicode *result = new PyUnicode(obString);
	if ( result->m_bstrValue )
		return result;
	Py_DECREF(result);
	// an error should have been raised 
	return NULL;
}
*****/
///////////////////
//
// Win32 Exception Handler.
//
// A recursive routine called by the exception handler!
// (I hope this doesnt wind too far on a stack overflow :-)
// Limited testing indicates it doesnt, and this can handle
// a stack overflow fine.
PyObject *MakeExceptionRecord( PEXCEPTION_RECORD pExceptionRecord )
{
	if (pExceptionRecord==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	} else {
		PyObject *obExcRecord = MakeExceptionRecord(pExceptionRecord->ExceptionRecord);
		PyObject *ret = Py_BuildValue("llOlO", 
			pExceptionRecord->ExceptionCode,
			pExceptionRecord->ExceptionFlags,
			obExcRecord,
			pExceptionRecord->ExceptionAddress,
			Py_None);
		Py_XDECREF(obExcRecord);
		return ret;
	}
}
int PyApplyExceptionFilter( 
	DWORD ExceptionCode,
	PEXCEPTION_POINTERS ExceptionInfo,
	PyObject *obHandler,
	PyObject **ppExcType,
	PyObject **ppExcValue)
{

	PyThreadState *stateSave = PyThreadState_Swap(NULL);
	PyThreadState_Swap(stateSave);
	BOOL createdThreadState = FALSE;
	BOOL acquiredThreadLock = FALSE;
	if (stateSave==NULL) { // Need to create one!
		// Copied from CEnterLeavePython!
		createdThreadState = PyWinThreadState_Ensure();
#ifdef PYCOM_USE_FREE_THREAD
#error Please revisit the thread requirements here!
#endif
		acquiredThreadLock = TRUE;
		PyWinInterpreterLock_Acquire();
	}

	PyObject *obRecord = MakeExceptionRecord(ExceptionInfo->ExceptionRecord);
	PyObject *args = Py_BuildValue("i(OO)", ExceptionCode, obRecord, Py_None);
	Py_XDECREF(obRecord);
	PyObject *obRet = PyObject_CallObject(obHandler, args);
	Py_DECREF(args);
	int ret = EXCEPTION_CONTINUE_SEARCH;
	if (obRet) {
		// Simple integer return code
		if (PyInt_Check(obRet)) {
			ret = PyInt_AsLong(obRet);
		// Exception instance to be raised.
		} else if (PyInstance_Check(obRet)) {
			*ppExcType = obRet;
			Py_INCREF(obRet);
			*ppExcValue = NULL;
			ret = EXCEPTION_EXECUTE_HANDLER;
		}
		// (exc_type, exc_value) to be raised.
		// Sequence checking MUST COME LAST!
		else if (PySequence_Check(obRet)) {
			*ppExcType = PySequence_GetItem(obRet, 0);
			*ppExcValue = PySequence_GetItem(obRet, 1);
			ret = EXCEPTION_EXECUTE_HANDLER;
		// else default == not handled.
		}
	}
	Py_XDECREF(obRet);

	if (acquiredThreadLock)
		PyWinInterpreterLock_Release();

	if ( createdThreadState )
		PyWinThreadState_Free();

	return ret;
}

#ifndef MAINWIN
// @pymethod object|win32api|Apply|Calls a Python function, but traps Win32 exceptions.
static PyObject *PyApply(PyObject *self, PyObject *args)
{
	PyObject *ret, *obHandler, *obFunc, *obArgs;
	PyObject *exc_type = NULL, *exc_value = NULL;
	if (!PyArg_ParseTuple(args, "OOO", 
		&obHandler, // @pyparm object|exceptionHandler||An object which will be called when a win32 exception occurs.
		&obFunc, // @pyparm object|func||The function call call under the protection of the Win32 exception handler.
		&obArgs)) // @pyparm tuple|args||Args for the function.
		return NULL;

	if (!PyCallable_Check(obHandler)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be an exception handler which accepts 2 arguments.");
		return NULL;
	}
	PyThreadState *stateSave = PyThreadState_Swap(NULL);
	PyThreadState_Swap(stateSave);
	_try {
		ret = PyObject_CallObject(obFunc, obArgs);
	}
	_except( PyApplyExceptionFilter( GetExceptionCode(),
	                                 GetExceptionInformation(),
	                                 obHandler,
									 &exc_type,
									 &exc_value) ) {
		// Do my best to restore the thread state to a sane spot.
		PyThreadState *stateCur = PyThreadState_Swap(NULL);
		if (stateCur == NULL) stateCur = stateSave;
		PyThreadState_Swap(stateCur);
		if (PyInstance_Check(exc_type)) {
			if (exc_value != NULL)
				PyErr_SetString(PyExc_TypeError, "instance exception returned from exception handler may not have a separate value");
			else {
				// Normalize to class, instance
				exc_value = exc_type;
				exc_type = (PyObject*) ((PyInstanceObject*)exc_type)->in_class;
				Py_INCREF(exc_type);
				PyErr_SetObject(exc_type, exc_value);
			}
		} else if (exc_type==NULL || exc_value==NULL)
			PyErr_SetString(PyExc_TypeError, "exception handler must return a valid object which can be raised as an exception (eg (exc_type, exc_value) or exc_class_instance)");
		else
			PyErr_SetObject(exc_type, exc_value);
		Py_XDECREF(exc_type);
		Py_XDECREF(exc_value);
		ret = NULL;
	}
	return ret;
// @comm Calls the specified function in a manner similar to 
// the built-in function apply(), but allows Win32 exceptions
// to be handled by Python.  If a Win32 exception occurs calling
// the function, the specified exceptionHandler is called, and its
// return value determines the action taken.
// @flagh Return value|Description
// @flag Tuple of (exc_type, exc_value)|This exception is raised to the 
// Python caller of Apply() - This is conceptually similar to 
// "raise exc_type, exc_value", although exception handlers must not
// themselves raise exceptions (see below).
// @flag Integer|Must be one of the win32 exception constants, and this
// value is returned to Win32.  See the Win32 documentation for details.
// @flag None|The exception is considered not handled (ie, it is as if no 
// exception handler exists).  If a Python exception occurs in the Win32 
// exception handler, it is as if None were returned (ie, no tracebacks 
// or other diagnostics are printed)
}
#endif // MAINWIN

// @pymethod |win32api|GetFileVersionInfo|Retrieve version info for specified file
PyObject *PyGetFileVersionInfo(PyObject *self, PyObject *args)
{
	int wcharcmp=0, nbr_langs=0, lang_ind=0;
	WORD lang=0, codepage=0;
	int langret=0, codepageret=0;
	DWORD *lang_codepage;
	PyObject *obfile_name=NULL, *obinfo=NULL;
	PyObject *ret=NULL, *ret_item=NULL, *obft=NULL;
	WCHAR *file_name=NULL, *info=NULL;
	DWORD dwHandle=0, buf_len=0;
	UINT value_len;
	VOID *buf=NULL, *value=NULL;
	VS_FIXEDFILEINFO *fixed_info;
	FILETIME ft;
	BOOL success;
	if (!PyArg_ParseTuple(args,"OO", 
		&obfile_name, // @pyparm string/unicode|Filename||File to query for version info
		&obinfo))	  // @pyparm string/unicode|SubBlock||Information to return: \ for VS_FIXEDFILEINFO, \VarFileInfo\Translation for languages/codepages available
		return NULL;
	if (!PyWinObject_AsWCHAR(obfile_name, &file_name, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obinfo, &info, FALSE))
		goto done;
	buf_len=GetFileVersionInfoSizeW(file_name, &dwHandle); //handle is ignored
	if (buf_len==0){
		PyWin_SetAPIError("GetFileVersionInfo:GetFileVersionInfoSize",GetLastError());
		goto done;
		}
	buf=malloc(buf_len);
	if (buf==NULL){
		PyErr_SetString(PyExc_MemoryError, "GetFileVersionInfo");
		goto done;
		}
	if (!GetFileVersionInfoW(file_name, dwHandle, buf_len, buf)){
		PyWin_SetAPIError("GetFileVersionInfo",GetLastError());
		goto done;
		}
	success=VerQueryValueW(buf, info, &value, &value_len);

	wcharcmp = CompareStringW(LOCALE_USER_DEFAULT,0,info,-1,L"\\",-1);
	if (wcharcmp==CSTR_EQUAL){
		if (!success){
			PyWin_SetAPIError("GetFileVersionInfo:VerQueryValue",GetLastError());
			goto done;
			}

		fixed_info=(VS_FIXEDFILEINFO *)value;
		ft.dwHighDateTime=fixed_info->dwFileDateMS;
		ft.dwLowDateTime=fixed_info->dwFileDateLS;
		// ?????? can't find any files where these are non-zero - conversion has not been tested ??????
		if ((ft.dwHighDateTime==0)&&(ft.dwLowDateTime==0)){
			obft=Py_None;
			Py_INCREF(Py_None);
			}
		else
			obft=PyWinObject_FromFILETIME(ft);

		ret=Py_BuildValue(
			"{u:l,u:l,u:l,u:l,u:l,u:l,u:l,u:l,u:l,u:l,u:l,u:N}",
			L"Signature",			fixed_info->dwSignature,
			L"StrucVersion",		fixed_info->dwStrucVersion,
			L"FileVersionMS",		fixed_info->dwFileVersionMS,
			L"FileVersionLS",		fixed_info->dwFileVersionLS,
			L"ProductVersionMS",	fixed_info->dwProductVersionMS,
			L"ProductVersionLS",	fixed_info->dwProductVersionLS,
			L"FileFlagsMask",		fixed_info->dwFileFlagsMask,
			L"FileFlags",			fixed_info->dwFileFlags,
			L"FileOS",				fixed_info->dwFileOS,
			L"FileType",			fixed_info->dwFileType,
			L"FileSubtype",			fixed_info->dwFileSubtype,
			L"FileDate",			obft);
		goto done;
		}

//   win32api.GetFileVersionInfo('c:/win2k/system32/cmd.exe',"\\VarFileInfo\\Translation")
	wcharcmp = CompareStringW(LOCALE_USER_DEFAULT,0,info,-1,L"\\VarFileInfo\\Translation",-1);
	if (wcharcmp==CSTR_EQUAL){
		if (!success){
			PyWin_SetAPIError("GetFileVersionInfo:VerQueryValue",GetLastError());
			goto done;
			}
		//return value consists of lang id/code page pairs as DWORDs
		nbr_langs=value_len/(sizeof(DWORD));
		ret=PyTuple_New(nbr_langs);
		lang_codepage=(DWORD *)value;
		for (lang_ind=0;lang_ind<nbr_langs;lang_ind++){
			langret=(lang=LOWORD(*lang_codepage));
			codepageret=(codepage=HIWORD(*lang_codepage));
			ret_item=Py_BuildValue("ii",langret,codepageret);
			PyTuple_SetItem(ret,lang_ind,ret_item);
			lang_codepage++;
			}
		goto done;
		}
	// VerQueryValue returns false and value pointer is null if specified string doesn't exist
	// This includes cases where the language and codepage are wrong, and simple misspellings of the
	//    standard string parms.  Maybe should throw error all the time ?  GetLastError returns no
	//    useful info, though.
	if (success)
		ret=PyWinObject_FromWCHAR((WCHAR *)value);
	else{
		if (value==NULL){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("GetFileVersionInfo:VerQueryValue",GetLastError());
		}

done:
	if (file_name)
		PyWinObject_FreeWCHAR(file_name);
	if (info)
		PyWinObject_FreeWCHAR(info);
	if (buf)
		free(buf);
	return ret;
}

// @pymethod |win32api|keybd_event|Simulate a keyboard event
PyObject *Pykeybd_event(PyObject *self, PyObject *args)
{
  BYTE bVk;
  BYTE bScan;
  DWORD dwFlags = 0;
  DWORD dwExtraInfo = 0;

  if (!PyArg_ParseTuple(args, "ii|ii:keybd_event",
           &bVk,    // @pyparm BYTE|bVk||Virtual-key code
           &bScan, // @pyparm BYTE|bScan||Hardware scan code
           &dwFlags,  // @pyparm DWORD|dwFlags|0|Flags specifying various function options
           &dwExtraInfo)) // @pyparm DWORD|dwExtraInfo|0|Additional data associated with keystroke
    return NULL;
  // @pyseeapi keybd_event
  PyW32_BEGIN_ALLOW_THREADS
  ::keybd_event(bVk,bScan,dwFlags,dwExtraInfo);
  PyW32_END_ALLOW_THREADS
  Py_INCREF(Py_None);
  return Py_None;
}
//
// @pymethod |win32api|mouse_event|Simulate a mouse event
PyObject *Pymouse_event(PyObject *self, PyObject *args)
{
  DWORD dwFlags;
  DWORD dx;
  DWORD dy;
  DWORD dwData = 0;
  DWORD dwExtraInfo = 0;

  if (!PyArg_ParseTuple(args, "iii|ii:mouse_event",
           &dwFlags,  // @pyparm DWORD|dwFlags|0|Flags specifying various function options
           &dx,     // @pyparm DWORD|dx||Horizontal position of mouse
           &dy,     // @pyparm DWORD|dy||Vertical position of mouse
           &dwData,    // @pyparm DWORD|dwData||Flag specific parameter
           &dwExtraInfo)) // @pyparm DWORD|dwExtraInfo|0|Additional data associated with mouse event

    return NULL;
  // @pyseeapi mouse_event
  PyW32_BEGIN_ALLOW_THREADS
  ::mouse_event(dwFlags,dx,dy,dwData,dwExtraInfo);
  PyW32_END_ALLOW_THREADS
  Py_INCREF(Py_None);
  return Py_None;
}


static BOOL addedCtrlHandler = FALSE;
static PyObject *consoleControlHandlers = NULL;
static BOOL WINAPI PyCtrlHandler(DWORD dwCtrlType)
{
	CEnterLeavePython _celp;
	// try and keep similar semantics to windows itself - last first, and
	// first to return TRUE stops the search.
	// Thread-safety provided by GIL
	PyObject *args = Py_BuildValue("(i)", dwCtrlType);
	if (!args) return FALSE;
	BOOL rc = FALSE;
	for (int i=PyList_GET_SIZE(consoleControlHandlers);i>0 && !rc;i--) {
		// The list may shift underneath us during the call - check index
		// is still valid.
		if (i > PyList_GET_SIZE(consoleControlHandlers))
			continue;

		PyObject *ob = PyList_GET_ITEM(consoleControlHandlers, i-1);
		PyObject *ret = PyObject_Call(ob, args, NULL);
		if (ret == NULL) {
			// EEK - this is printed in the case of SystemExit - but SystemExit
			// *is* honoured by virtue of PyErr_Print() doing the termination.
			// This will not be the main thread.  I don't think we want this.
			PySys_WriteStderr("ConsoleCtrlHandler function failed");
			PyErr_Print();
			PyErr_Clear();
			continue;
		}
		rc = PyObject_IsTrue(ret);
		Py_DECREF(ret);
	}
	Py_DECREF(args);
	return rc;
}

// @pymethod |win32api|SetConsoleCtrlHandler|Adds or removes an application-defined HandlerRoutine function from the list of handler functions for the calling process.
PyObject *PySetConsoleCtrlHandler(PyObject *self, PyObject *args)
{
	// @comm Note that the implementation is a single CtrlHandler in C, which
	// keeps a list of the handlers added by this function.  So although this
	// function uses the same semantics as the Win32 function (ie, last
	// registered first called, and first to return True stops the calls) the
	// true order of all Python and C implemented CtrlHandlers may not match
	// what would happen if all were implemented in C.
	// <nl>This handler must acquire the Python lock before it can call any
	// of the registered handlers.  This means the handler may not be called
	// until the current Python thread yields the lock.
	// <nl>
	// A console process can use the <om win32api.GenerateConsoleCtrlEvent>
	// function to send a CTRL+C or CTRL+BREAK signal to a console process
	// group.
	// <nl>The system generates CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, and
	// CTRL_SHUTDOWN_EVENT signals when the user closes the console, logs off,
	// or shuts down the system so that the process has an opportunity to
	// clean up before termination.
	// @pyseeapi SetConsoleCtrlHandler
	PyObject *func;
	int bAdd = TRUE;
	// @pyparm callable|ctrlHandler||The function to call.  This function
	// should accept one param - the type of signal.
	// @pyparm int|bAdd||True if the handler is being added, false if removed.
	if (!PyArg_ParseTuple(args, "O|i:SetConsoleCtrlHandler", &func, &bAdd))
		return NULL;
	// Handle special case of None first
	if (func == Py_None) {
		if (!SetConsoleCtrlHandler(NULL, bAdd))
			return ReturnAPIError("SetConsoleCtrlHandler");
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (!PyCallable_Check(func))
		return PyErr_Format(PyExc_TypeError,
							"First argument must be callable (got %s)",
							func->ob_type->tp_name);
	// thread-safety provided by GIL
	if (consoleControlHandlers==NULL)
		consoleControlHandlers = PyList_New(0);
	if (consoleControlHandlers==NULL)
		return NULL;

	BOOL ok = TRUE; // we may not actually make the call!
	if (bAdd) {
		if (0 != PyList_Append(consoleControlHandlers, func))
			return NULL;
		if (!addedCtrlHandler) {
			ok = SetConsoleCtrlHandler(PyCtrlHandler, TRUE);
			addedCtrlHandler = ok;
		}
	} else {
		int i;
		BOOL found = FALSE;
		for (i=0;i<PyList_Size(consoleControlHandlers);i++) {
			if (PyList_GET_ITEM(consoleControlHandlers, i)==func) {
				if (0 != PyList_SetSlice(consoleControlHandlers, i, i+1, NULL))
					return NULL;
				found = TRUE;
			}
		}
		if (!found)
			return PyErr_Format(PyExc_ValueError, "The object has not been registered");
		if (addedCtrlHandler && PyList_Size(consoleControlHandlers)==0) {
			ok = SetConsoleCtrlHandler(PyCtrlHandler, FALSE);
			addedCtrlHandler = FALSE;
		}
	}
	if (!ok)
		return ReturnAPIError("SetConsoleCtrlHandler");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod (int,..)|win32api|GetKeyboardLayoutList|Returns a sequence of all locale ids currently loaded
PyObject *PyGetKeyboardLayoutList(PyObject *self, PyObject *args)
{
	int buflen;
	HKL *buf;
	PyObject *ret=NULL;
	if (!PyArg_ParseTuple(args,":GetKeyboardLayoutList"))
		return NULL;
	buflen=GetKeyboardLayoutList(0,NULL);
	buf=(HKL *)malloc(buflen*sizeof(HKL));
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen*sizeof(HKL));
	buflen=GetKeyboardLayoutList(buflen, buf);
	if (buflen==0)
		PyWin_SetAPIError("GetKeyboardLayout");
	else{
		ret=PyTuple_New(buflen);
		if (ret!=NULL){
			for (int tuple_ind=0;tuple_ind<buflen;tuple_ind++){
				PyObject *tuple_item=PyLong_FromLong((long)buf[tuple_ind]);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, tuple_ind, tuple_item);
				}
			}
		}
	free(buf);
	return ret;
}

// @pymethod int|win32api|LoadKeyboardLayout|Loads a new locale id
// @rdesc Returns the numeric locale id that was loaded
PyObject *PyLoadKeyboardLayout(PyObject *self, PyObject *args)
{	
	char *lcid_str;
	HKL lcid;
	UINT flags=0;
	if (!PyArg_ParseTuple(args, "s|k:LoadKeyboardLayout",
		&lcid_str,		// @pyparm string|KLID||Hex string containing a locale id, eg "00000409"
		&flags))		// @pyparm int|Flags|0|Combination of win32con.KLF_* constants
		return NULL;
	lcid=LoadKeyboardLayout(lcid_str, flags);
	if (lcid==NULL)
		return PyWin_SetAPIError("LoadKeyboardLayout");
	return PyLong_FromLong((long)lcid);
}


/* List of functions exported by this module */
// @module win32api|A module, encapsulating the Windows Win32 API.
static struct PyMethodDef win32api_functions[] = {
#ifndef DONT_HAVE_SYSTEM_SHUTDOWN
	{"AbortSystemShutdown",	PyAbortSystemShutdown,1},     // @pymeth AbortSystemShutdown|Aborts a system shutdown
	{"InitiateSystemShutdown",  PyInitiateSystemShutdown,1}, // @pymeth InitiateSystemShutdown|Initiates a shutdown and optional restart of the specified computer.
#endif
#ifndef MAINWIN	
	{"Apply",               PyApply, 1}, // @pymeth Apply|Calls a Python function, but traps Win32 exceptions.
#endif
	{"Beep",				PyBeep,         1},     // @pymeth Beep|Generates a simple tone on the speaker.
	{"BeginUpdateResource", PyBeginUpdateResource, 1 }, // @pymeth BeginUpdateResource|Begins an update cycle for a PE file.
	{"ChangeDisplaySettings", PyChangeDisplaySettings, 1}, // @pymeth ChangeDisplaySettings|Changes video mode for default display
	{"ChangeDisplaySettingsEx", (PyCFunction)PyChangeDisplaySettingsEx, METH_VARARGS|METH_KEYWORDS}, // @pymeth ChangeDisplaySettingsEx|Changes video mode for specified display
	{"ClipCursor",			PyClipCursor,       1}, // @pymeth ClipCursor|Confines the cursor to a rectangular area on the screen.
	{"CloseHandle",		    PyCloseHandle,     1},  // @pymeth CloseHandle|Closes an open handle.
	{"CopyFile",			PyCopyFile,         1}, // @pymeth CopyFile|Copy a file.
	{"DebugBreak",			PyDebugBreak,       1}, // @pymeth DebugBreak|Breaks into the C debugger.
	{"DeleteFile",			PyDeleteFile,       1}, // @pymeth DeleteFile|Deletes the specified file.
	{"DragQueryFile",		PyDragQueryFile,    1}, // @pymeth DragQueryFile|Retrieve the file names for dropped files.
	{"DragFinish",			PyDragFinish,       1}, // @pymeth DragFinish|Free memory associated with dropped files.
	{"DuplicateHandle",     PyDuplicateHandle,  1}, // @pymeth DuplicateHandle|Duplicates a handle.
	{"EndUpdateResource",   PyEndUpdateResource, 1 }, // @pymeth EndUpdateResource|Ends a resource update cycle of a PE file.
	{"EnumDisplayDevices",	(PyCFunction)PyEnumDisplayDevices,	METH_VARARGS|METH_KEYWORDS}, //@pymeth EnumDisplayDevices|Obtain information about the display devices in a system 
	{"EnumDisplayMonitors", (PyCFunction)PyEnumDisplayMonitors, METH_VARARGS|METH_KEYWORDS}, // @pymeth EnumDisplayMonitors|Lists monitors for a device context
	{"EnumDisplaySettings", (PyCFunction)PyEnumDisplaySettings,	METH_VARARGS|METH_KEYWORDS}, //@pymeth EnumDisplaySettings|Lists available modes for specified device 
	{"EnumDisplaySettingsEx", (PyCFunction)PyEnumDisplaySettingsEx,METH_VARARGS|METH_KEYWORDS}, //@pymeth EnumDisplaySettingsEx|Lists available modes for a display device, with optional flags
	{"EnumResourceLanguages",   PyEnumResourceLanguages, 1 }, // @pymeth EnumResourceLanguages|List languages for specified resource
	{"EnumResourceNames",   PyEnumResourceNames, 1 }, // @pymeth EnumResourceNames|Enumerates all the resources of the specified type from the nominated file.
	{"EnumResourceTypes",   PyEnumResourceTypes, 1 }, // @pymeth EnumResourceTypes|Return list of all resource types contained in module
	{"ExpandEnvironmentStrings",PyExpandEnvironmentStrings, 1}, // @pymeth ExpandEnvironmentStrings|Expands environment-variable strings and replaces them with their defined values. 
	{"ExitWindows",         PyExitWindows,      1}, // @pymeth ExitWindows|Logs off the current user
	{"ExitWindowsEx",       PyExitWindowsEx,      1}, // @pymeth ExitWindowsEx|either logs off the current user, shuts down the system, or shuts down and restarts the system.
	{"FindFiles",			PyFindFiles,        1}, // @pymeth FindFiles|Find files matching a file spec.
	{"FindFirstChangeNotification", PyFindFirstChangeNotification, 1}, // @pymeth FindFirstChangeNotification|Creates a change notification handle and sets up initial change notification filter conditions.
	{"FindNextChangeNotification", PyFindNextChangeNotification, 1}, // @pymeth FindNextChangeNotification|Requests that the operating system signal a change notification handle the next time it detects an appropriate change.
	{"FindCloseChangeNotification", PyFindCloseChangeNotification, 1}, // @pymeth FindCloseChangeNotification|Closes the change notification handle.
	{"FindExecutable",		PyFindExecutable,   1}, // @pymeth FindExecutable|Find an executable associated with a document.
	{"FormatMessage",		PyFormatMessage,    1}, // @pymeth FormatMessage|Return an error message string.
	{"FormatMessageW",		PyFormatMessageW,    1}, // @pymeth FormatMessageW|Return an error message string (as a Unicode object).
	{"FreeLibrary",			PyFreeLibrary,1},       // @pymeth FreeLibrary|Decrements the reference count of the loaded dynamic-link library (DLL) module.
#ifndef DONT_HAVE_GENERATE_CONSOLE_CTRL_EVENT
	{"GenerateConsoleCtrlEvent",	PyGenerateConsoleCtrlEvent,  1}, // @pymeth GenerateConsoleCtrlEvent|Send a specified signal to a console process group that shares the console associated with the calling process.
#endif
	
	{"GetAsyncKeyState",	PyGetAsyncKeyState,1}, // @pymeth GetAsyncKeyState|Retrieves the asynch state of a virtual key code.
	{"GetCommandLine",		PyGetCommandLine,   1}, // @pymeth GetCommandLine|Return the application's command line.
	{"GetComputerName",     PyGetComputerName,  1}, // @pymeth GetComputerName|Returns the local computer name
	{"GetComputerNameEx",   PyGetComputerNameEx,  1}, // @pymeth GetComputerNameEx|Retrieves a NetBIOS or DNS name associated with the local computer
	{"GetComputerObjectName",PyGetComputerObjectName,  1}, // @pymeth GetComputerObjectName|Retrieves the local computer's name in a specified format
	{"GetMonitorInfo",		(PyCFunction)PyGetMonitorInfo, METH_VARARGS|METH_KEYWORDS},	// @pymeth GetMonitorInfo|Retrieves information for a monitor by handle
	{"GetUserName",         PyGetUserName,  1},     // @pymeth GetUserName|Returns the current user name.
	{"GetUserNameEx",       PyGetUserNameEx,  1},     // @pymeth GetUserNameEx|Returns the current user name in format specified by Name* constants
	{"GetCursorPos",		PyGetCursorPos,   1},   // @pymeth GetCursorPos|Returns the position of the cursor, in screen co-ordinates.
	{"GetCurrentThread",    PyGetCurrentThread,   1}, // @pymeth GetCurrentThread|Returns a pseudohandle for the current thread.
	{"GetCurrentThreadId",  PyGetCurrentThreadId,   1}, // @pymeth GetCurrentThreadId|Returns the thread ID for the current thread.
	{"GetCurrentProcessId", PyGetCurrentProcessId,   1}, // @pymeth GetCurrentProcessId|Returns the thread ID for the current thread.
	{"GetCurrentProcess",   PyGetCurrentProcess,   1}, // @pymeth GetCurrentProcess|Returns a pseudohandle for the current process.
	{"GetConsoleTitle",		PyGetConsoleTitle,  1}, // @pymeth GetConsoleTitle|Return the application's console title.
	{"GetDateFormat",       PyGetDateFormat, 1}, // @pymeth GetDateFormat|Formats a date as a date string for a specified locale.
	{"GetDiskFreeSpace",	PyGetDiskFreeSpace, 1}, // @pymeth GetDiskFreeSpace|Retrieves information about a disk.
	{"GetDiskFreeSpaceEx",	PyGetDiskFreeSpaceEx, 1}, // @pymeth GetDiskFreeSpaceEx|Retrieves information about a disk.
	{"GetDomainName",		PyGetDomainName, 1}, 	// @pymeth GetDomainName|Returns the current domain name
	{"GetEnvironmentVariable", PyGetEnvironmentVariable, 1}, // @pymeth GetEnvironmentVariable|Retrieves the value of an environment variable.
	{"GetFileAttributes",   PyGetFileAttributes,1}, // @pymeth GetFileAttributes|Retrieves the attributes for the named file.
	{"GetFileVersionInfo",	PyGetFileVersionInfo, 1}, //@pymeth GetFileVersionInfo|Retrieves string version info
	{"GetFocus",            PyGetFocus,         1}, // @pymeth GetFocus|Retrieves the handle of the keyboard focus window associated with the thread that called the method. 
	{"GetFullPathName",     PyGetFullPathName,1},   // @pymeth GetFullPathName|Returns the full path of a (possibly relative) path
	{"GetHandleInformation",     PyGetHandleInformation,1},   // @pymeth GetHandleInformation|Retrieves a handle's flags.
	{"GetKeyboardLayoutList", PyGetKeyboardLayoutList, 1}, // @pymeth GetKeyboardLayoutList|Returns a sequence of all locale ids in the system
	{"GetKeyboardState", PyGetKeyboardState, 1}, // @pymeth GetKeyboardState|Retrieves the status of the 256 virtual keys on the keyboard.
	{"GetKeyState",			PyGetKeyState,      1}, // @pymeth GetKeyState|Retrives the last known key state for a key.
	{"GetLastError",		PyGetLastError,     1}, // @pymeth GetLastError|Retrieves the last error code known by the system.
	{"GetLocalTime",         PyGetLocalTime,      1},  // @pymeth GetLocalTime|Returns the current local time.
	{"GetLongPathName",     PyGetLongPathNameA, 1}, // @pymeth GetLongPathName|Converts the specified path to its long form.
	{"GetLongPathNameW",    PyGetLongPathNameW, 1}, // @pymeth GetLongPathNameW|Converts the specified path to its long form.
	{"GetLogicalDrives",	PyGetLogicalDrives,     1}, // @pymeth GetLogicalDrives|Returns a bitmask representing the currently available disk drives.
	{"GetLogicalDriveStrings",	PyGetLogicalDriveStrings,     1}, // @pymeth GetLogicalDriveStrings|Returns a list of strings for all the drives.
	{"GetModuleFileName",	PyGetModuleFileName,1}, // @pymeth GetModuleFileName|Retrieves the filename of the specified module.
	{"GetModuleHandle",     PyGetModuleHandle,1},   // @pymeth GetModuleHandle|Returns the handle of an already loaded DLL.
	{"GetProfileSection",	PyGetProfileSection,1}, // @pymeth GetProfileSection|Returns a list of entries in an INI file.
	{"GetProcAddress",      PyGetProcAddress,1},    // @pymeth GetProcAddress|Returns the address of the specified exported dynamic-link library (DLL) function.
	{"GetProfileVal",		PyGetProfileVal,    1}, // @pymeth GetProfileVal|Returns a value from an INI file.
	{"GetShortPathName",	PyGetShortPathName, 1}, // @pymeth GetShortPathName|Returns the 8.3 version of a pathname.
	{"GetStdHandle",	PyGetStdHandle,	1}, // @pymeth GetStdHandle|Returns a handle for the standard input, standard output, or standard error device
	{"GetSysColor",			PyGetSysColor,      1}, // @pymeth GetSysColor|Returns the system colors.
	{"GetSystemDefaultLangID",PyGetSystemDefaultLangID,1}, // @pymeth GetSystemDefaultLangID|Retrieves the system default language identifier. 
	{"GetSystemDefaultLCID",PyGetSystemDefaultLCID,1}, // @pymeth GetSystemDefaultLCID|Retrieves the system default locale identifier. 
	{"GetSystemDirectory",	PyGetSystemDirectory,1}, // @pymeth GetSystemDirectory|Returns the Windows system directory.
	{"GetSystemInfo",		PyGetSystemInfo, 1},    // @pymeth GetSystemInfo|Retrieves information about the current system.
	{"GetSystemMetrics",	PyGetSystemMetrics, 1}, // @pymeth GetSystemMetrics|Returns the specified system metrics.
	{"GetSystemTime",		PyGetSystemTime,	1},	// @pymeth GetSystemTime|Returns the current system time.
	{"GetTempFileName",		PyGetTempFileName,  1}, // @pymeth GetTempFileName|Creates a temporary file.
	{"GetTempPath",			PyGetTempPath,      1}, // @pymeth GetTempPath|Returns the path designated as holding temporary files.
	{"GetThreadLocale",     PyGetThreadLocale, 1}, // @pymeth GetThreadLocale|Returns the current thread's locale.
	{"GetTickCount",	    PyGetTickCount,      1}, // @pymeth GetTickCount|Returns the milliseconds since windows started.
	{"GetTimeFormat",       PyGetTimeFormat, 1}, // @pymeth GetTimeFormat|Formats a time as a time string for a specified locale.
	{"GetTimeZoneInformation",	PyGetTimeZoneInformation,1}, // @pymeth GetTimeZoneInformation|Returns the system time-zone information.
	{"GetVersion",			PyGetVersion,       1}, // @pymeth GetVersion|Returns Windows version information.
	{"GetVersionEx",		PyGetVersionEx,       1}, // @pymeth GetVersionEx|Returns Windows version information as a tuple.
	{"GetVolumeInformation",PyGetVolumeInformation,1}, // @pymeth GetVolumeInformation|Returns information about a volume and file system attached to the system.
	{"GetWindowsDirectory", PyGetWindowsDirectory,1}, // @pymeth GetWindowsDirectory|Returns the windows directory.
	{"GetWindowLong",       PyGetWindowLong,1}, // @pymeth GetWindowLong|Retrieves a long value at the specified offset into the extra window memory of the given window.
	{"GetUserDefaultLangID",PyGetUserDefaultLangID,1}, // @pymeth GetUserDefaultLangID|Retrieves the user default language identifier. 
	{"GetUserDefaultLCID",  PyGetUserDefaultLCID,1}, // @pymeth GetUserDefaultLCID|Retrieves the user default locale identifier.
	{"keybd_event",         Pykeybd_event, 1}, // @pymeth keybd_event|Simulate a keyboard event
	{"mouse_event",         Pymouse_event, 1}, // @pymeth mouse_event|Simulate a mouse event
	{"LoadCursor",          PyLoadCursor, 1}, // @pymeth LoadCursor|Loads a cursor.
	{"LoadKeyboardLayout",  PyLoadKeyboardLayout, 1}, // @pymeth LoadKeyboardLayout|Loads a new locale id
	{"LoadLibrary",	        PyLoadLibrary,1}, // @pymeth LoadLibrary|Loads the specified DLL, and returns the handle.
	{"LoadLibraryEx",	    PyLoadLibraryEx,1}, // @pymeth LoadLibraryEx|Loads the specified DLL, and returns the handle.
	{"LoadResource",	    PyLoadResource,1}, // @pymeth LoadResource|Finds and loads a resource from a PE file.
	{"LoadString",	        PyLoadString,1}, // @pymeth LoadString|Loads a string from a resource file.
	{"MessageBeep",         PyMessageBeep,1}, // @pymeth MessageBeep|Plays a predefined waveform sound.
	{"MessageBoxEx",        PyMessageBox, 1},
	{"MessageBox",          PyMessageBox, 1}, // @pymeth MessageBox|Display a message box.
	{"MonitorFromPoint",	(PyCFunction)PyMonitorFromPoint, METH_VARARGS|METH_KEYWORDS},// @pymeth MonitorFromPoint|Finds monitor that contains a point
	{"MonitorFromRect",		(PyCFunction)PyMonitorFromRect, METH_VARARGS|METH_KEYWORDS},// @pymeth MonitorFromRect|Finds monitor that has largest intersection with a rectangle
	{"MonitorFromWindow",	(PyCFunction)PyMonitorFromWindow, METH_VARARGS|METH_KEYWORDS},// @pymeth MonitorFromWindow|Finds monitor that contains a window
	{"MoveFile",			PyMoveFile,			1}, // @pymeth MoveFile|Moves or renames a file.
	{"MoveFileEx",			PyMoveFileEx,		1}, // @pymeth MoveFileEx|Moves or renames a file.
	{"OpenProcess",         PyOpenProcess, 1}, // @pymeth OpenProcess|Retrieves a handle to an existing process.
	{"OutputDebugString",	PyOutputDebugString, 1 }, // @pymeth OutputDebugString|Writes output to the Windows debugger.
	{"PostMessage",         PyPostMessage, 1}, // @pymeth PostMessage|Post a message to a window.
	{"PostQuitMessage",     PyPostQuitMessage, 1}, // @pymeth PostQuitMessage|Posts a quit message.
	{"PostThreadMessage",   PyPostThreadMessage, 1}, // @pymeth PostThreadMessage|Post a message to a thread.
	{"RegCloseKey",			PyRegCloseKey, 1}, // @pymeth RegCloseKey|Closes a registry key.
	{"RegConnectRegistry",	PyRegConnectRegistry, 1}, // @pymeth RegConnectRegistry|Establishes a connection to a predefined registry handle on another computer.
	{"RegCreateKey",        PyRegCreateKey, 1}, // @pymeth RegCreateKey|Creates the specified key, or opens the key if it already exists.
	{"RegDeleteKey",        PyRegDeleteKey, 1}, // @pymeth RegDeleteKey|Deletes the specified key.
	{"RegDeleteValue",      PyRegDeleteValue, 1}, // @pymeth RegDeleteValue|Removes a named value from the specified registry key.
	{"RegEnumKey",          PyRegEnumKey, 1}, // @pymeth RegEnumKey|Enumerates subkeys of the specified open registry key.
	{"RegEnumKeyEx",        PyRegEnumKeyEx, 1}, // @pymeth RegEnumKey|Enumerates subkeys of the specified open registry key.
	{"RegEnumValue",        PyRegEnumValue, 1}, // @pymeth RegEnumValue|Enumerates values of the specified open registry key.
	{"RegFlushKey",	        PyRegFlushKey, 1}, // @pymeth RegFlushKey|Writes all the attributes of the specified key to the registry.
	{"RegGetKeySecurity",   PyRegGetKeySecurity, 1}, // @pymeth RegGetKeySecurity|Retrieves the security on the specified registry key.
	{"RegLoadKey",          PyRegLoadKey, 1}, // @pymeth RegLoadKey|Creates a subkey under HKEY_USER or HKEY_LOCAL_MACHINE and stores registration information from a specified file into that subkey.
	{"RegOpenKey",          PyRegOpenKey, 1}, // @pymeth RegOpenKey|Alias for <om win32api.RegOpenKeyEx>
	{"RegOpenKeyEx",        PyRegOpenKey, 1}, // @pymeth RegOpenKeyEx|Opens the specified key.
	{"RegQueryValue",       PyRegQueryValue, 1}, // @pymeth RegQueryValue|Retrieves the value associated with the unnamed value for a specified key in the registry.
	{"RegQueryValueEx",	PyRegQueryValueEx, 1}, // @pymeth RegQueryValueEx|Retrieves the type and data for a specified value name associated with an open registry key. 
	{"RegQueryInfoKey",	PyRegQueryInfoKey, 1}, // @pymeth RegQueryInfoKey|Returns information about the specified key.
	{"RegSaveKey",          PyRegSaveKey, 1}, // @pymeth RegSaveKey|Saves the specified key, and all its subkeys to the specified file.
	{"RegSetKeySecurity",   PyRegSetKeySecurity, 1}, // @pymeth RegSetKeySecurity|Sets the security on the specified registry key.
	{"RegSetValue",         PyRegSetValue, 1}, // @pymeth RegSetValue|Associates a value with a specified key.  Currently, only strings are supported.
	{"RegSetValueEx",       PyRegSetValueEx, 1}, // @pymeth RegSetValueEx|Stores data in the value field of an open registry key.
	{"RegUnLoadKey",        PyRegUnLoadKey, 1}, // @pymeth RegUnLoadKey|Unloads the specified registry key and its subkeys from the registry.  The keys must have been loaded previously by a call to RegLoadKey.
	{"RegisterWindowMessage",PyRegisterWindowMessage, 1}, // @pymeth RegisterWindowMessage|Given a string, return a system wide unique message ID.
	{"RegNotifyChangeKeyValue", PyRegNotifyChangeKeyValue, 1}, //@pymeth RegNotifyChangeKeyValue|Watch for registry changes
	{"SearchPath",          PySearchPath, 1}, // @pymeth SearchPath|Searches a path for a file.
	{"SendMessage",         PySendMessage, 1}, // @pymeth SendMessage|Send a message to a window.
	{"SetConsoleCtrlHandler",PySetConsoleCtrlHandler, 1}, // @pymeth SetConsoleCtrlHandler|Adds or removes an application-defined HandlerRoutine function from the list of handler functions for the calling process.
	{"SetConsoleTitle",     PySetConsoleTitle, 1}, // @pymeth SetConsoleTitle|Sets the title for the current console.
	{"SetCursorPos",		PySetCursorPos,1}, // @pymeth SetCursorPos|The SetCursorPos function moves the cursor to the specified screen coordinates.
	{"SetErrorMode",        PySetErrorMode, 1}, // @pymeth SetErrorMode|Controls whether the system will handle the specified types of serious errors, or whether the process will handle them.
	{"SetFileAttributes",   PySetFileAttributes,1}, // @pymeth SetFileAttributes|Sets the named file's attributes.
	{"SetLastError",        PySetLastError,     1}, // @pymeth SetLastError|Sets the last error code known for the current thread.
	{"SetSysColors",		PySetSysColors,      1}, // @pymeth SetSysColors|Changes color of various window elements
	{"SetSystemTime",		PySetSystemTime,	1},	// @pymeth SetSystemTime|Sets the system time.	
	{"SetClassLong",       PySetClassLong,1}, // @pymeth SetClassLong|Replaces the specified 32-bit (long) value at the specified offset into the extra class memory for the window.
	{"SetClassWord",       PySetClassWord,1}, // @pymeth SetClassWord|Replaces the specified 32-bit (long) value at the specified offset into the extra class memory for the window.
	{"SetClassWord",       PySetWindowWord,1}, // @pymeth SetWindowWord|
	{"SetCursor",           PySetCursor,1}, // @pymeth SetCursor|Set the cursor to the HCURSOR object.
	{"SetHandleInformation",	PySetHandleInformation,1}, // @pymeth SetHandleInformation|Sets a handles's flags
	{"SetStdHandle",	PySetStdHandle,	1}, // @pymeth SetStdHandle|Sets a handle for the standard input, standard output, or standard error device
	{"SetThreadLocale",     PySetThreadLocale, 1}, // @pymeth SetThreadLocale|Sets the current thread's locale.
	{"SetWindowLong",       PySetWindowLong,1}, // @pymeth SetWindowLong|Places a long value at the specified offset into the extra window memory of the given window.
	{"ShellExecute",		PyShellExecute,		1}, // @pymeth ShellExecute|Executes an application.
	{"ShowCursor",			PyShowCursor,		1}, // @pymeth ShowCursor|The ShowCursor method displays or hides the cursor. 
	{"Sleep",				PySleep,			1}, 
	{"SleepEx",				PySleep,			1}, // @pymeth Sleep|Suspends current application execution
	{"TerminateProcess",	PyTerminateProcess,	1}, // @pymeth TerminateProcess|Terminates a process.
	{"Unicode",             PyWin_NewUnicode,         1},	// @pymeth Unicode|Creates a new <o PyUnicode> object
	{"UpdateResource",     PyUpdateResource, 1 },  // @pymeth UpdateResource|Updates a resource in a PE file.
	{"VkKeyScan",           PyVkKeyScan,     1}, // @pymeth VkKeyScan|Translates a character to the corresponding virtual-key code and shift state. 
	{"VkKeyScanEx",         PyVkKeyScanEx,   1}, // @pymeth VkKeyScan|Translates a character to the corresponding virtual-key code and shift state. 
	{"WinExec",             PyWinExec,  		1}, // @pymeth WinExec|Execute a program.
	{"WinHelp",             PyWinHelp,  		1}, // @pymeth WinHelp|Invokes the Windows Help engine.
	{"WriteProfileSection",	PyWriteProfileSection,  1}, // @pymeth WriteProfileSection|Writes a complete section to an INI file or registry.
	{"WriteProfileVal",		PyWriteProfileVal,  1}, // @pymeth WriteProfileVal|Write a value to a Windows INI file.
	{"HIBYTE",              PyHIBYTE,           1}, // @pymeth HIBYTE|An interface to the win32api HIBYTE macro.
	{"LOBYTE",              PyLOBYTE,           1}, // @pymeth LOBYTE|An interface to the win32api LOBYTE macro.
	{"HIWORD",              PyHIWORD,           1}, // @pymeth HIWORD|An interface to the win32api HIWORD macro.
	{"LOWORD",              PyLOWORD,           1}, // @pymeth LOWORD|An interface to the win32api LOWORD macro.
	{"RGB",                 PyRGB,              1}, // @pymeth RGB|An interface to the win32api RGB macro.
	{"MAKELANGID",          PyMAKELANGID,       1}, // @pymeth MAKELANGID|Creates a language identifier from a primary language identifier and a sublanguage identifier.
	{"MAKEWORD",            PyMAKEWORD,         1}, // @pymeth MAKEWORD|creates a WORD value by concatenating the specified values.
	{"MAKELONG",            PyMAKELONG,         1}, // @pymeth MAKELONG|creates a LONG value by concatenating the specified values.
	{NULL,			NULL}
};

extern "C" __declspec(dllexport) void
initwin32api(void)
{
  PyObject *dict, *module;
  PyWinGlobals_Ensure();
  module = Py_InitModule("win32api", win32api_functions);
  if (!module) /* Eeek - some serious error! */
    return;
  dict = PyModule_GetDict(module);
  if (!dict) return; /* Another serious error!*/
  Py_INCREF(PyWinExc_ApiError);
  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
  PyDict_SetItemString(dict,"STD_INPUT_HANDLE",
		       PyInt_FromLong(STD_INPUT_HANDLE));
  PyDict_SetItemString(dict,"STD_OUTPUT_HANDLE",
		       PyInt_FromLong(STD_OUTPUT_HANDLE));
  PyDict_SetItemString(dict,"STD_ERROR_HANDLE",
		       PyInt_FromLong(STD_ERROR_HANDLE));
  PyDict_SetItemString(dict, "PyDISPLAY_DEVICEType", (PyObject *)&PyDISPLAY_DEVICEType);
  PyModule_AddIntConstant(module, "NameUnknown", NameUnknown);
  PyModule_AddIntConstant(module, "NameFullyQualifiedDN", NameFullyQualifiedDN);
  PyModule_AddIntConstant(module, "NameSamCompatible", NameSamCompatible);
  PyModule_AddIntConstant(module, "NameDisplay", NameDisplay);
  PyModule_AddIntConstant(module, "NameUniqueId", NameUniqueId);
  PyModule_AddIntConstant(module, "NameCanonical", NameCanonical);
  PyModule_AddIntConstant(module, "NameUserPrincipal", NameUserPrincipal);
  PyModule_AddIntConstant(module, "NameCanonicalEx", NameCanonicalEx);
  PyModule_AddIntConstant(module, "NameServicePrincipal", NameServicePrincipal);

  PyModule_AddIntConstant(module, "REG_NOTIFY_CHANGE_NAME", REG_NOTIFY_CHANGE_NAME);
  PyModule_AddIntConstant(module, "REG_NOTIFY_CHANGE_ATTRIBUTES", REG_NOTIFY_CHANGE_ATTRIBUTES);
  PyModule_AddIntConstant(module, "REG_NOTIFY_CHANGE_LAST_SET", REG_NOTIFY_CHANGE_LAST_SET);
  PyModule_AddIntConstant(module, "REG_NOTIFY_CHANGE_SECURITY", REG_NOTIFY_CHANGE_SECURITY);

    // FileOS values
  PyModule_AddIntConstant(module, "VOS_DOS",VOS_DOS);
  PyModule_AddIntConstant(module, "VOS_NT",VOS_NT);
  PyModule_AddIntConstant(module, "VOS__WINDOWS16",VOS__WINDOWS16);
  PyModule_AddIntConstant(module, "VOS__WINDOWS32",VOS__WINDOWS32);
  PyModule_AddIntConstant(module, "VOS_OS216",VOS_OS216);
  PyModule_AddIntConstant(module, "VOS_OS232",VOS_OS232);
  PyModule_AddIntConstant(module, "VOS__PM16",VOS__PM16);
  PyModule_AddIntConstant(module, "VOS__PM32",VOS__PM32);
  PyModule_AddIntConstant(module, "VOS_UNKNOWN",VOS_UNKNOWN);
  PyModule_AddIntConstant(module, "VOS_DOS_WINDOWS16",VOS_DOS_WINDOWS16);
  PyModule_AddIntConstant(module, "VOS_DOS_WINDOWS32",VOS_DOS_WINDOWS32);
  PyModule_AddIntConstant(module, "VOS_NT_WINDOWS32",VOS_NT_WINDOWS32);
  PyModule_AddIntConstant(module, "VOS_OS216_PM16",VOS_OS216_PM16);
  PyModule_AddIntConstant(module, "VOS_OS232_PM32",VOS_OS232_PM32);

  //FileType values
  PyModule_AddIntConstant(module, "VFT_UNKNOWN",VFT_UNKNOWN);
  PyModule_AddIntConstant(module, "VFT_APP",VFT_APP);
  PyModule_AddIntConstant(module, "VFT_DLL",VFT_DLL);
  PyModule_AddIntConstant(module, "VFT_DRV",VFT_DRV);
  PyModule_AddIntConstant(module, "VFT_FONT",VFT_FONT);
  PyModule_AddIntConstant(module, "VFT_VXD",VFT_VXD);
  PyModule_AddIntConstant(module, "VFT_STATIC_LIB",VFT_STATIC_LIB);

  //FileFlags
  PyModule_AddIntConstant(module, "VS_FF_DEBUG",VS_FF_DEBUG);
  PyModule_AddIntConstant(module, "VS_FF_INFOINFERRED",VS_FF_INFOINFERRED);
  PyModule_AddIntConstant(module, "VS_FF_PATCHED",VS_FF_PATCHED);
  PyModule_AddIntConstant(module, "VS_FF_PRERELEASE",VS_FF_PRERELEASE);
  PyModule_AddIntConstant(module, "VS_FF_PRIVATEBUILD",VS_FF_PRIVATEBUILD);
  PyModule_AddIntConstant(module, "VS_FF_SPECIALBUILD",VS_FF_SPECIALBUILD);

  HMODULE hmodule = GetModuleHandle("secur32.dll");
  if (hmodule==NULL)
    hmodule=LoadLibrary("secur32.dll");
  if (hmodule!=NULL){
    pfnGetUserNameEx=(GetUserNameExfunc)GetProcAddress(hmodule,"GetUserNameExW");
    pfnGetComputerObjectName=(GetUserNameExfunc)GetProcAddress(hmodule,"GetComputerObjectNameW");
  }

  hmodule = GetModuleHandle("kernel32.dll");
  if (hmodule==NULL)
	  hmodule=LoadLibrary("kernel32.dll");
  if (hmodule!=NULL){
    pfnGetComputerNameEx=(GetComputerNameExfunc)GetProcAddress(hmodule,"GetComputerNameExW");
    pfnGetLongPathNameA=(GetLongPathNameAfunc)GetProcAddress(hmodule,"GetLongPathNameA");
    pfnGetLongPathNameW=(GetLongPathNameWfunc)GetProcAddress(hmodule,"GetLongPathNameW");
    pfnGetHandleInformation=(GetHandleInformationfunc)GetProcAddress(hmodule,"GetHandleInformation");
    pfnSetHandleInformation=(SetHandleInformationfunc)GetProcAddress(hmodule,"SetHandleInformation");
  }

  hmodule = GetModuleHandle("user32.dll");
  if (hmodule==NULL)
    hmodule=LoadLibrary("user32.dll");
  if (hmodule!=NULL){
	pfnEnumDisplayMonitors=(EnumDisplayMonitorsfunc)GetProcAddress(hmodule, "EnumDisplayMonitors");
	pfnEnumDisplayDevices=(EnumDisplayDevicesfunc)GetProcAddress(hmodule, "EnumDisplayDevicesA");
	pfnChangeDisplaySettingsEx=(ChangeDisplaySettingsExfunc)GetProcAddress(hmodule,"ChangeDisplaySettingsExA");
	pfnMonitorFromWindow=(MonitorFromWindowfunc)GetProcAddress(hmodule,"MonitorFromWindow");
	pfnMonitorFromRect=(MonitorFromRectfunc)GetProcAddress(hmodule,"MonitorFromRect");
	pfnMonitorFromPoint=(MonitorFromPointfunc)GetProcAddress(hmodule,"MonitorFromPoint");
	pfnGetMonitorInfo=(GetMonitorInfofunc)GetProcAddress(hmodule,"GetMonitorInfoA");
	pfnEnumDisplaySettingsEx=(EnumDisplaySettingsExfunc)GetProcAddress(hmodule,"EnumDisplaySettingsExA");
  }
}  
