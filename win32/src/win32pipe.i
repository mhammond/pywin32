/* File : win32pipe.i */
// @doc

%module win32pipe // An interface to the win32 pipe API's

%{
  /* Used to validate access modes in win32pipe.win32pipe() */
#include "fcntl.h"

%}

%include "typemaps.i"
%include "pywin32.i"

%{

// Global used to determine if Win9x win32pipe hack is necessary.
bool g_fUsingWin9x;
CHAR g_szModulePath[_MAX_PATH + 2];
HINSTANCE g_hInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

#ifndef BUILD_FREEZE
// Doesnt work for freeze - but in that case, g_hInstance
// remaims NULL, so GetModuleFileName(NULL) will be used
// and still give the answer we are after.
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		g_hInstance = hInstance;
	return TRUE;
}
#endif // BUILD_FREEZE

static BOOL LoadModulePath(void)
{
	DWORD cbModuleFilename;
	CHAR *psz = NULL;

	// Note: GetModuleFileName will write nSize + 1 characters
	// to get the null terminator.
	cbModuleFilename = GetModuleFileName(
		g_hInstance,
		g_szModulePath,
		sizeof(g_szModulePath) - 1);
	if (0 == cbModuleFilename)
	{
//	    hr = HRESULT_FROM_WIN32(GetLastError());
//	    ErrorTrace(hr);
		return FALSE;
	}

	if ((sizeof(g_szModulePath) - 1) == cbModuleFilename)
	{
		// Note: This should never happen
//	    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
//	    ErrorTrace(hr);
		return FALSE;
	}

	// Start from the end of the string and insert a '\0' after the first '\\' you find.
	psz = g_szModulePath + strlen(g_szModulePath) - 1;
	while (psz > g_szModulePath && *psz != '\\')
	{
		psz--;
	}

	if (*psz == '\\')
	{
		psz++;
		*psz = '\0';
	}
	else
	{
		// Something wierd happened. :(
		return FALSE;
	}
	return TRUE;
}


%}

%init %{
	// All errors raised by this module are of this type.
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);

	// Setup g_fUsingWin9x and module path correctly...
	{
		if (!LoadModulePath()) {
			PyErr_SetString(PyExc_RuntimeError, "Could not determine the module base path!?");
		}

		OSVERSIONINFO osvi;

		memset(&osvi, 0, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		
		if (!GetVersionEx(&osvi))
		{
			PyWin_SetAPIError("GetVersionEx");
		}

		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			g_fUsingWin9x = TRUE;
		}
		else
		{
			g_fUsingWin9x = FALSE;
		}
	}
%}

%{
extern PyObject *PyPopen(PyObject *self, PyObject  *args);
extern PyObject *PyPopen2(PyObject *self, PyObject  *args);
extern PyObject *PyPopen3(PyObject *self, PyObject  *args);
extern PyObject *PyPopen4(PyObject *self, PyObject  *args);

%}
// @pymeth popen|Version of popen that works in a GUI
%native(popen) PyPopen;

// @pymeth popen2|Variation on popen - returns 2 pipes
%native(popen2) PyPopen2;

// @pymeth popen3|Variation on popen - returns 3 pipes
%native(popen3) PyPopen3;

// @pymeth popen4|Like popen2, but stdout/err are combined.
%native(popen4) PyPopen4;

// @pymeth GetNamedPipeHandleState|Returns the state of a named pipe.
%native(GetNamedPipeHandleState) MyGetNamedPipeHandleState;

// @pymeth SetNamedPipeHandleState|Sets the state of a named pipe.
%native(SetNamedPipeHandleState) MySetNamedPipeHandleState;

// @pymeth ConnectNamedPipe|Connects to a named pipe
%native(ConnectNamedPipe) MyConnectNamedPipe;

// @pymeth CallNamedPipe|Calls a named pipe
%native(CallNamedPipe) MyCallNamedPipe;

%name(CreatePipe)
PyObject *MyCreatePipe(SECURITY_ATTRIBUTES *INPUT, DWORD nSize);
PyObject *FdCreatePipe(SECURITY_ATTRIBUTES *INPUT, DWORD nSize, int mode);


#define PIPE_ACCESS_DUPLEX PIPE_ACCESS_DUPLEX
#define PIPE_ACCESS_INBOUND PIPE_ACCESS_INBOUND
#define PIPE_ACCESS_OUTBOUND PIPE_ACCESS_OUTBOUND
#define PIPE_TYPE_BYTE PIPE_TYPE_BYTE
#define PIPE_TYPE_MESSAGE PIPE_TYPE_MESSAGE
#define PIPE_READMODE_BYTE PIPE_READMODE_BYTE
#define PIPE_READMODE_MESSAGE PIPE_READMODE_MESSAGE
#define PIPE_WAIT PIPE_WAIT
#define PIPE_NOWAIT PIPE_NOWAIT
#define NMPWAIT_NOWAIT NMPWAIT_NOWAIT
#define NMPWAIT_WAIT_FOREVER NMPWAIT_WAIT_FOREVER
#define NMPWAIT_USE_DEFAULT_WAIT NMPWAIT_USE_DEFAULT_WAIT
#define PIPE_UNLIMITED_INSTANCES PIPE_UNLIMITED_INSTANCES

%{
// @pyswig (int, int, int/None, int/None, <o PyUnicode>|GetNamedPipeHandleState|Determines the state of the named pipe.
PyObject *MyGetNamedPipeHandleState(PyObject *self, PyObject *args)
{
	HANDLE hNamedPipe;
	PyObject *obhNamedPipe;
	unsigned long State;
	unsigned long CurInstances;
	unsigned long MaxCollectionCount;
	unsigned long CollectDataTimeout;
	unsigned long *pMaxCollectionCount;
	unsigned long *pCollectDataTimeout;
	PyObject *obMaxCollectionCount;
	PyObject *obCollectDataTimeout;

	BOOL getCollectData = FALSE;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm int|bGetCollectionData|0|Determines of the collection data should be returned.  If not, None is returned in their place.

	if (!PyArg_ParseTuple(args, "O|i:GetNamedPipeHandleState", &obhNamedPipe, &getCollectData))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	TCHAR buf[512];
	if (getCollectData) {
		pMaxCollectionCount = &MaxCollectionCount;
		pCollectDataTimeout = &CollectDataTimeout;
	} else
		pMaxCollectionCount = pCollectDataTimeout = NULL;

	if (!GetNamedPipeHandleState(hNamedPipe, &State, &CurInstances, pMaxCollectionCount, pCollectDataTimeout, buf, 512))
		return PyWin_SetAPIError("GetNamedPipeHandleState");
	PyObject *obName = PyWinObject_FromTCHAR(buf);
	if (getCollectData) {
		obMaxCollectionCount = PyInt_FromLong(MaxCollectionCount);
		obCollectDataTimeout = PyInt_FromLong(CollectDataTimeout);
	} else {
		obMaxCollectionCount = Py_None; Py_INCREF(Py_None);
		obCollectDataTimeout = Py_None; Py_INCREF(Py_None);
	}
	PyObject *rc = Py_BuildValue("iiOOO", State, CurInstances, obMaxCollectionCount, obCollectDataTimeout, obName);
	Py_DECREF(obMaxCollectionCount);
	Py_DECREF(obCollectDataTimeout);
	Py_DECREF(obName);
	return rc;
}

// @pyswig |SetNamedPipeHandleState|Sets the state of the named pipe.
PyObject *MySetNamedPipeHandleState(PyObject *self, PyObject *args)
{
	HANDLE hNamedPipe;
	unsigned long Mode;
	unsigned long MaxCollectionCount;
	unsigned long CollectDataTimeout;
	PyObject *obhNamedPipe;

	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm int|Mode||The pipe read mode.
	// @pyparm int|MaxCollectionCount||Maximum bytes collected before transmission to the server.
	// @pyparm int|CollectDataTimeout||Maximum time to wait, in milliseconds, before transmission to server.

	if (!PyArg_ParseTuple(args, "Oiii:SetNamedPipeHandleState", 
			      &obhNamedPipe, &Mode, 
			      &MaxCollectionCount, &CollectDataTimeout))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;

	if (!SetNamedPipeHandleState(hNamedPipe, &Mode, &MaxCollectionCount,
				     &CollectDataTimeout)) 
		return PyWin_SetAPIError("SetNamedPipeHandleState");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig int|ConnectNamedPipe|Connects to a named pipe
// @comm The result is the HRESULT from the underlying function.
// If an overlapped object is passed, the result may be ERROR_IO_PENDING or ERROR_PIPE_CONNECTED.
PyObject *MyConnectNamedPipe(PyObject *self, PyObject *args)
{
	HANDLE hNamedPipe;
	PyObject *obhNamedPipe;
	OVERLAPPED *pOverlapped;
	PyObject *obOverlapped = NULL;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm <o PyOVERLAPPED>|overlapped|None|An overlapped object to use, else None
	if (!PyArg_ParseTuple(args, "O|O:ConnectNamedPipe", &obhNamedPipe, &obOverlapped))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	if (obOverlapped==NULL)
		pOverlapped = NULL;
	else {
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
	}
	BOOL ok;
    Py_BEGIN_ALLOW_THREADS
	ok = ConnectNamedPipe(hNamedPipe, pOverlapped);
    Py_END_ALLOW_THREADS
	DWORD rc = GetLastError();
	// These error conditions are documented as "acceptable" - ie,
	// the function has still worked.
	if (!ok && rc!= 0 && rc != ERROR_IO_PENDING && rc != ERROR_PIPE_CONNECTED)
		return PyWin_SetAPIError("ConnectNamedPipe");
	return PyInt_FromLong(rc);
}

// @pyswig string|CallNamedPipe|Opens and performs a transaction on a named pipe.
PyObject *MyCallNamedPipe(PyObject *self, PyObject *args)
{
	PyObject *obPipeName;
	char *data;
	int dataSize;
	DWORD timeOut;
	int readBufSize;
	TCHAR *szPipeName;
	if (!PyArg_ParseTuple(args, "Os#il:CallNamedPipe", 
		&obPipeName, // @pyparm <o PyUNICODE>|pipeName||The name of the pipe.
		&data, &dataSize, // @pyparm string|data||The data to write.
		&readBufSize, // @pyparm int|bufSize||The size of the result buffer to allocate for the read.
		&timeOut)) // @pyparm int|timeOut||Specifies the number of milliseconds to wait for the named pipe to be available. In addition to numeric values, the following special values can be specified.
		// @flagh Value|Meaning 
		// @flag win32pipe.NMPWAIT_NOWAIT|Does not wait for the named pipe. If the named pipe is not available, the function returns an error. 
		// @flag win32pipe.NMPWAIT_WAIT_FOREVER|Waits indefinitely. 
		// @flag win32pipe.NMPWAIT_USE_DEFAULT_WAIT|Uses the default time-out specified in a call to the CreateNamedPipe function. 
		return NULL;

	if (!PyWinObject_AsTCHAR(obPipeName, &szPipeName))
		return NULL;

	void *readBuf = malloc(readBufSize);

	DWORD numRead = 0;
	BOOL ok;
    Py_BEGIN_ALLOW_THREADS
	ok = CallNamedPipe(szPipeName, (void *)data, dataSize, readBuf, readBufSize, &numRead, timeOut);
    Py_END_ALLOW_THREADS
	if (!ok) {
		PyWinObject_FreeTCHAR(szPipeName);
		free(readBuf);
		return PyWin_SetAPIError("CallNamedPipe");
	}
	PyObject *rc = PyString_FromStringAndSize( (char *)readBuf, numRead);
	PyWinObject_FreeTCHAR(szPipeName);
	free(readBuf);
	return rc;
}

// @pyswig (<o PyHANDLE>, <o PyHANDLE>)|CreatePipe|Creates an anonymous pipe, and returns handles to the read and write ends of the pipe
PyObject *MyCreatePipe(
		       SECURITY_ATTRIBUTES *INPUT, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||
		       DWORD nSize // @pyparm int|nSize||
		       )
{
  HANDLE hReadPipe;		// variable for read handle 
  HANDLE hWritePipe;		// variable for write handle 
  BOOL   ok;			// did CreatePipe work?

  ok = CreatePipe(&hReadPipe, &hWritePipe, INPUT, nSize);
  if (!ok)
    return PyWin_SetAPIError("CreatePipe");

  PyObject *read_obj = PyWinObject_FromHANDLE(hReadPipe);
  PyObject *write_obj = PyWinObject_FromHANDLE(hWritePipe);
  PyObject *result = Py_BuildValue("OO", read_obj, write_obj);
  Py_DECREF(read_obj);
  Py_DECREF(write_obj);
  return result;
}

// @pyswig (int, int)|FdCreatePipe|As CreatePipe but returns file descriptors
PyObject *FdCreatePipe(
		    SECURITY_ATTRIBUTES *INPUT, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||
		    DWORD nSize, // @pyparm int|nSize||
		    int mode // @pyparm int|mode||
		    )
{
  HANDLE hReadPipe;		// variable for read handle 
  HANDLE hWritePipe;		// variable for write handle 
  BOOL   ok;			// did CreatePipe work?
  if (mode != _O_TEXT && mode != _O_BINARY)
    {
      PyErr_SetString(PyExc_ValueError, "mode must be O_TEXT or O_BINARY");
      return NULL;
    }

  ok = CreatePipe(&hReadPipe, &hWritePipe, INPUT, nSize);
  if (!ok)
    return PyWin_SetAPIError("CreatePipe");

  int read_fd = _open_osfhandle ((long) hReadPipe, mode);
  int write_fd = _open_osfhandle ((long) hWritePipe, mode);
  PyObject *result = Py_BuildValue("ii", read_fd, write_fd);
  return result;
}

%}

// @pyswig <o PyHANDLE>|CreateNamedPipe|Creates an instance of a named pipe and returns a handle for subsequent pipe operations
PyHANDLE CreateNamedPipe( 
	TCHAR *lpName,	// @pyparm <o PyUnicode>|pipeName||The name of the pipe
	unsigned long dwOpenMode, // @pyparm int|openMode||OpenMode of the pipe
	unsigned long dwPipeMode, // @pyparm int|pipeMode||
	unsigned long nMaxInstances, // @pyparm int|nMaxInstances||
	unsigned long nOutBufferSize,// @pyparm int|nOutBufferSize||
	unsigned long nInBufferSize, // @pyparm int|nInBufferSize||
	unsigned long nDefaultTimeOut, // @pyparm int|nDefaultTimeOut||
	SECURITY_ATTRIBUTES *INPUT // @pyparm <o PySECURITY_ATTRIBUTES>|sa||
);
// @pyswig |DisconnectNamedPipe|Disconnects the server end of a named pipe instance from a client process. 
BOOLAPI DisconnectNamedPipe(
	PyHANDLE hFile // @pyparm <o PyHANDLE>|hFile||The handle to the pipe to disconnect.
);

// @pyswig int|GetOverlappedResult|Determines the result of the most recent call with an OVERLAPPED object.
// @comm The result is the number of bytes transferred.  The overlapped object's attributes will be changed during this call.
BOOLAPI GetOverlappedResult(
	PyHANDLE hFile, 	// @pyparm <o PyHANDLE>|hFile||The handle to the pipe or file
	OVERLAPPED *lpOverlapped, // @pyparm <o PyOVERLAPPED>|overlapped||The overlapped object to check.
	unsigned long *OUTPUT, // lpNumberOfBytesTransferred
	BOOL bWait	// @pyparm int|bWait||Indicates if the function should wait for data to become available.
);

// @pyswig |WaitNamedPipe|Waits until either a time-out interval elapses or an instance of the specified named pipe is available to be connected to (that is, the pipe's server process has a pending <om win32pipe.ConnectNamedPipe> operation on the pipe). 
BOOLAPI WaitNamedPipe( 
	TCHAR *pipeName, // @pyparm <o PyUnicode>|pipeName||The name of the pipe
	unsigned long timeout); // @pyparm int|timeout||The number of milliseconds the function will wait.
	// instead of a literal value, you can specify one of the following values for the timeout:
	// @flagh Value|Meaning 
	// @flag NMPWAIT_USE_DEFAULT_WAIT|The time-out interval is the default value specified by the server process in the CreateNamedPipe function. 
	// @flag NMPWAIT_WAIT_FOREVER|The function does not return until an instance of the named pipe is available 
	
%{

// @pyswig (string, int, int)|PeekNamedPipe|Copies data from a named or anonymous pipe into a buffer without removing it from the pipe. It also returns information about data in the pipe.
PyObject *MyPeekNamedPipe(PyObject *self, PyObject *args)
{
	HANDLE hNamedPipe;
	PyObject *obhNamedPipe;
	unsigned long bytesRead, totalAvail, bytesLeft;
	int size;

	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm int|size||The size of the buffer.

	if (!PyArg_ParseTuple(args, "Oi:PeekNamedPipe", &obhNamedPipe, &size))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	void *buf = malloc(size);
	if (buf==NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	PyObject *rc = NULL;
	if (PeekNamedPipe(hNamedPipe, buf, size, &bytesRead, &totalAvail, &bytesLeft)) {
		rc = Py_BuildValue("s#ii", (char *)buf, bytesRead, totalAvail, bytesLeft);
	} else
		PyWin_SetAPIError("GetNamedPipeHandleState");
	free(buf);
	return rc;
}
%}

%native(PeekNamedPipe) MyPeekNamedPipe;
