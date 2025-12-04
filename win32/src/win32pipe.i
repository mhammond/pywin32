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
#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);
typedef	BOOL (WINAPI *GetNamedPipeClientProcessIdfunc)(HANDLE, PULONG);
static GetNamedPipeClientProcessIdfunc pfnGetNamedPipeClientProcessId = NULL;
static GetNamedPipeClientProcessIdfunc pfnGetNamedPipeServerProcessId = NULL;
static GetNamedPipeClientProcessIdfunc pfnGetNamedPipeClientSessionId = NULL;
static GetNamedPipeClientProcessIdfunc pfnGetNamedPipeServerSessionId = NULL;
%}

%init %{
	// All errors raised by this module are of this type.
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);

	HMODULE hmod = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
	if (hmod != NULL) {
		pfnGetNamedPipeClientProcessId = (GetNamedPipeClientProcessIdfunc)GetProcAddress(hmod, "GetNamedPipeClientProcessId");
		pfnGetNamedPipeServerProcessId = (GetNamedPipeClientProcessIdfunc)GetProcAddress(hmod, "GetNamedPipeServerProcessId");
		pfnGetNamedPipeClientSessionId = (GetNamedPipeClientProcessIdfunc)GetProcAddress(hmod, "GetNamedPipeClientSessionId");
		pfnGetNamedPipeServerSessionId = (GetNamedPipeClientProcessIdfunc)GetProcAddress(hmod, "GetNamedPipeServerSessionId");
	}
%}

%{
PyObject *PyPopen(PyObject *self, PyObject  *args) {PyErr_SetString(PyExc_NotImplementedError, "not available in py3k"); return NULL;};
PyObject *PyPopen2(PyObject *self, PyObject  *args) {PyErr_SetString(PyExc_NotImplementedError, "not available in py3k"); return NULL;};
PyObject *PyPopen3(PyObject *self, PyObject  *args) {PyErr_SetString(PyExc_NotImplementedError, "not available in py3k"); return NULL;};
PyObject *PyPopen4(PyObject *self, PyObject  *args) {PyErr_SetString(PyExc_NotImplementedError, "not available in py3k"); return NULL;};

%}
// @pymeth popen|Version of popen that works in a GUI
// @comm Not implemented in py3k.
%native(popen) PyPopen;

// @pymeth popen2|Variation on popen - returns 2 pipes
// @comm Not implemented in py3k.
%native(popen2) PyPopen2;

// @pymeth popen3|Variation on popen - returns 3 pipes
// @comm Not implemented in py3k.
%native(popen3) PyPopen3;

// @pymeth popen4|Like popen2, but stdout/err are combined.
// @comm Not implemented in py3k.
%native(popen4) PyPopen4;

%native(GetNamedPipeHandleState) MyGetNamedPipeHandleState;

%native(SetNamedPipeHandleState) MySetNamedPipeHandleState;

%native(ConnectNamedPipe) MyConnectNamedPipe;

%native(TransactNamedPipe) MyTransactNamedPipe;

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
#define PIPE_ACCEPT_REMOTE_CLIENTS PIPE_ACCEPT_REMOTE_CLIENTS
#define PIPE_REJECT_REMOTE_CLIENTS PIPE_REJECT_REMOTE_CLIENTS
#define FILE_FLAG_FIRST_PIPE_INSTANCE FILE_FLAG_FIRST_PIPE_INSTANCE

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
	BOOL getUserName = FALSE;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm int|bGetCollectionData|0|Determines if the collection data should be retrieved.  If not, None is returned in their place.
	// @pyparm int|bGetUserName|0|Determines if the username should be retrieved. Works only for a server handle and if the client opened the pipe with SECURITY_IMPERSONATION access.

	if (!PyArg_ParseTuple(args, "O|ii:GetNamedPipeHandleState", &obhNamedPipe, &getCollectData, &getUserName))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	TCHAR buf[512] = L"";
	if (getCollectData) {
		pMaxCollectionCount = &MaxCollectionCount;
		pCollectDataTimeout = &CollectDataTimeout;
	} else
		pMaxCollectionCount = pCollectDataTimeout = NULL;

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = GetNamedPipeHandleState(hNamedPipe, &State, &CurInstances, pMaxCollectionCount, pCollectDataTimeout,
								  getUserName ? buf : NULL, 512);
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("GetNamedPipeHandleState");
	PyObject *obName = PyWinObject_FromTCHAR(buf);
	if (getCollectData) {
		obMaxCollectionCount = PyLong_FromLong(MaxCollectionCount);
		obCollectDataTimeout = PyLong_FromLong(CollectDataTimeout);
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
    unsigned long *pMaxCollectionCount = NULL;
    unsigned long *pCollectDataTimeout = NULL;
    unsigned long *pMode = NULL;
	PyObject *obhNamedPipe;
    PyObject *obMode, *obMaxCollectionCount, *obCollectDataTimeout;

	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	// @pyparm int/None|Mode||The pipe read mode.
	// @pyparm int/None|MaxCollectionCount||Maximum bytes collected before transmission to the server.
	// @pyparm int/None|CollectDataTimeout||Maximum time to wait, in milliseconds, before transmission to server.

	if (!PyArg_ParseTuple(args, "OOOO:SetNamedPipeHandleState",
			      &obhNamedPipe, &obMode,
			      &obMaxCollectionCount, &obCollectDataTimeout))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
    if (obMode!=Py_None) {
        if (!PyLong_Check(obMode))
            return PyErr_Format(PyExc_TypeError, "mode param must be None or an integer (got %s)", obMode->ob_type->tp_name);
        Mode = PyLong_AsLong(obMode);
        pMode = &Mode;
    }
    if (obMaxCollectionCount!=Py_None) {
        if (!PyLong_Check(obMaxCollectionCount))
            return PyErr_Format(PyExc_TypeError, "maxCollectionCount param must be None or an integer (got %s)", obMaxCollectionCount->ob_type->tp_name);
        MaxCollectionCount = PyLong_AsLong(obMaxCollectionCount);
        pMaxCollectionCount = &MaxCollectionCount;
    }
    if (obCollectDataTimeout!=Py_None) {
        if (!PyLong_Check(obCollectDataTimeout))
            return PyErr_Format(PyExc_TypeError, "collectDataTimeout param must be None or an integer (got %s)", obCollectDataTimeout->ob_type->tp_name);
        CollectDataTimeout = PyLong_AsLong(obCollectDataTimeout);
        pCollectDataTimeout = &CollectDataTimeout;
    }

	if (!SetNamedPipeHandleState(hNamedPipe, pMode, pMaxCollectionCount,
				     pCollectDataTimeout))
		return PyWin_SetAPIError("SetNamedPipeHandleState");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig int|ConnectNamedPipe|Connects to a named pipe
// @comm The result is zero if the function succeeds.  If the function fails,
// GetLastError() is called, and if the result is ERROR_IO_PENDING or ERROR_PIPE_CONNECTED
// (common when passing an overlapped object), this value is returned.  All
// other error values raise a win32 exception (from which the error code
// can be extracted)
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
	DWORD rc = ok ? 0 : GetLastError();
	// These error conditions are documented as "acceptable" - ie,
	// the function has still worked.
	if (!ok && rc != ERROR_IO_PENDING && rc != ERROR_PIPE_CONNECTED)
		return PyWin_SetAPIError("ConnectNamedPipe");
	return PyLong_FromLong(rc);
}

// @pyswig string/buffer|TransactNamedPipe|Combines the functions that write a
// message to and read a message from the specified named pipe into a single
// network operation.
// @comm This function is modelled on <om win32file.ReadFile> - for overlapped
// operations you are expected to provide a buffer which will be filled
// asynchronously.
PyObject *MyTransactNamedPipe(PyObject *self, PyObject *args)
{
	PyObject *obHandle, *obWriteData, *obReadData, *obOverlapped = Py_None;
	void *readData;
	DWORD cbReadData;
	HANDLE handle;
	OVERLAPPED *pOverlapped = NULL;
	if (!PyArg_ParseTuple(args, "OOO|O:TransactNamedPipe",
		&obHandle,	// @pyparm <o PyUNICODE>|pipeName||The name of the pipe.
		&obWriteData,   // @pyparm string/buffer|writeData||The data to write to the pipe.
		// @pyparm <o PyOVERLAPPEDReadBuffer>/int|buffer/bufSize||Size of the buffer to create for the result,
		// or a buffer to fill with the result. If a buffer object and overlapped is passed, the result is
		// the buffer itself.  If a buffer but no overlapped is passed, the result is a new string object,
		// built from the buffer, but with a length that reflects the data actually read.
		&obReadData,
		&obOverlapped))		// @pyparm <o PyOVERLAPPED>|overlapped|None|An overlapped structure or None
		return NULL;

	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;

	PyWinBufferView read_buf, write_buf(obWriteData);
	if (!write_buf.ok())
		return NULL;

	if (obOverlapped!=Py_None && !PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
		return NULL;

	BOOL bIsNewString=FALSE;
	PyObject *obRet = NULL;
	// process the tricky read buffer.
	cbReadData = PyLong_AsLong(obReadData);
	if ((cbReadData!=(DWORD)-1) || !PyErr_Occurred()){
		if (pOverlapped){
			obRet = PyBuffer_New(cbReadData);
			if (obRet==NULL)
				return NULL;
			// This shouldn't fail for buffer just created, but new buffer interface is screwy ...
			// maybe also check new buffer size matched requested size ?
			if (!read_buf.init(obRet, true)) {
			    Py_DECREF(obRet);
			    return NULL;
			    }
			readData = read_buf.ptr();
		} else {
			obRet=PyBytes_FromStringAndSize(NULL, cbReadData);
			if (obRet==NULL)
				return NULL;
			readData=PyBytes_AS_STRING(obRet);
			bIsNewString=TRUE;
		}
	} else {
		PyErr_Clear();
		if (!read_buf.init(obReadData, true))
		    return NULL;
		readData = read_buf.ptr();
		cbReadData = read_buf.len();
		// If they didn't pass an overlapped, then we can't return the
		// original buffer as they have no way to know how many bytes
		// were read - so leave obRet NULL and the ret will be a new
		// string object, built from buffer, but the correct length.
		if (pOverlapped){
			obRet = obReadData;
			Py_INCREF(obRet);
		}
	}
	BOOL ok;
	DWORD numRead = 0;
	Py_BEGIN_ALLOW_THREADS
	ok = TransactNamedPipe(handle, write_buf.ptr(), write_buf.len(), readData, cbReadData, &numRead, pOverlapped);
	Py_END_ALLOW_THREADS
	DWORD err = 0;
	if (!ok) {
		err = GetLastError();
		if (err!=ERROR_MORE_DATA && err != ERROR_IO_PENDING) {
			Py_XDECREF(obRet);
			return PyWin_SetAPIError("TransactNamedPipe", err);
		}
	}
	if (obRet==NULL)
		obRet=PyBytes_FromStringAndSize((char *)readData, numRead);
	else if (bIsNewString && (numRead < cbReadData))
		_PyBytes_Resize(&obRet, numRead);
	if (obRet==NULL)
		return NULL;
	return Py_BuildValue("iN", err, obRet);
}

// @pyswig string|CallNamedPipe|Opens and performs a transaction on a named pipe.
PyObject *MyCallNamedPipe(PyObject *self, PyObject *args)
{
	PyObject *obPipeName, *obdata;
	DWORD timeOut;
	DWORD readBufSize;
	TCHAR *szPipeName;
	if (!PyArg_ParseTuple(args, "OOil:CallNamedPipe",
		&obPipeName,	// @pyparm <o PyUNICODE>|pipeName||The name of the pipe.
		&obdata,		// @pyparm string|data||The data to write.
		&readBufSize,	// @pyparm int|bufSize||The size of the result buffer to allocate for the read.
		&timeOut))		// @pyparm int|timeOut||Specifies the number of milliseconds to wait for the named pipe to be available. In addition to numeric values, the following special values can be specified.
		// @flagh Value|Meaning
		// @flag win32pipe.NMPWAIT_NOWAIT|Does not wait for the named pipe. If the named pipe is not available, the function returns an error.
		// @flag win32pipe.NMPWAIT_WAIT_FOREVER|Waits indefinitely.
		// @flag win32pipe.NMPWAIT_USE_DEFAULT_WAIT|Uses the default time-out specified in a call to the CreateNamedPipe function.
		return NULL;
	PyWinBufferView pybuf(obdata);
	if (!pybuf.ok())
		return NULL;
	if (!PyWinObject_AsTCHAR(obPipeName, &szPipeName))
		return NULL;
	void *readBuf = malloc(readBufSize);
	if (!readBuf){
		PyWinObject_FreeTCHAR(szPipeName);
		return PyErr_NoMemory();
		}
	DWORD numRead = 0;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = CallNamedPipe(szPipeName, pybuf.ptr(), pybuf.len(), readBuf, readBufSize, &numRead, timeOut);
	Py_END_ALLOW_THREADS
	if (!ok) {
		PyWinObject_FreeTCHAR(szPipeName);
		free(readBuf);
		return PyWin_SetAPIError("CallNamedPipe");
	}
	PyObject *rc = PyBytes_FromStringAndSize( (char *)readBuf, numRead);
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
	SECURITY_ATTRIBUTES *INPUT, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||Specifies security and inheritance for the pipe
	DWORD nSize,				// @pyparm int|nSize||Buffer size for pipe.  Use 0 for default size.
	int mode)					// @pyparm int|mode||O_TEXT or O_BINARY
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

  int read_fd = _open_osfhandle ((INT_PTR)hReadPipe, mode);
  int write_fd = _open_osfhandle ((INT_PTR)hWritePipe, mode);
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

// @pyswig (int, int, int, int)|GetNamedPipeInfo|Returns pipe's flags, buffer sizes, and max instances
BOOLAPI GetNamedPipeInfo(
	HANDLE hNamedPipe,	// @pyparm <o PyHANDLE>|hNamedPipe||Handle to a named pipe
	DWORD *OUTPUT,
	DWORD *OUTPUT,
	DWORD *OUTPUT,
	DWORD *OUTPUT);

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
		rc = Py_BuildValue("Nii",
			PyBytes_FromStringAndSize((char *)buf, bytesRead),
			totalAvail, bytesLeft);
	} else
		PyWin_SetAPIError("PeekNamedPipe");
	free(buf);
	return rc;
}
%}

%native(PeekNamedPipe) MyPeekNamedPipe;

%{
// @pyswig int|GetNamedPipeClientProcessId|Returns the process id of client that is connected to a named pipe
PyObject *MyGetNamedPipeClientProcessId(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetNamedPipeClientProcessId);
	HANDLE hNamedPipe;
	DWORD pid;
	PyObject *obhNamedPipe;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	if (!PyArg_ParseTuple(args, "O:GetNamedPipeClientProcessId", &obhNamedPipe))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	if (!(*pfnGetNamedPipeClientProcessId)(hNamedPipe, &pid))
		return PyWin_SetAPIError("GetNamedPipeClientProcessId");
	return PyLong_FromUnsignedLong(pid);
}

// @pyswig int|GetNamedPipeServerProcessId|Returns pid of server process that created a named pipe
PyObject *MyGetNamedPipeServerProcessId(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetNamedPipeServerProcessId);
	HANDLE hNamedPipe;
	DWORD pid;
	PyObject *obhNamedPipe;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	if (!PyArg_ParseTuple(args, "O:GetNamedPipeServerProcessId", &obhNamedPipe))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	if (!(*pfnGetNamedPipeServerProcessId)(hNamedPipe, &pid))
		return PyWin_SetAPIError("GetNamedPipeServerProcessId");
	return PyLong_FromUnsignedLong(pid);
}

// @pyswig int|GetNamedPipeClientSessionId|Returns the session id of client that is connected to a named pipe
PyObject *MyGetNamedPipeClientSessionId(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetNamedPipeClientSessionId);
	HANDLE hNamedPipe;
	DWORD pid;
	PyObject *obhNamedPipe;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	if (!PyArg_ParseTuple(args, "O:GetNamedPipeClientSessionId", &obhNamedPipe))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	if (!(*pfnGetNamedPipeClientSessionId)(hNamedPipe, &pid))
		return PyWin_SetAPIError("GetNamedPipeClientSessionId");
	return PyLong_FromUnsignedLong(pid);
}

// @pyswig int|GetNamedPipeServerSessionId|Returns session id of server process that created a named pipe
PyObject *MyGetNamedPipeServerSessionId(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetNamedPipeServerSessionId);
	HANDLE hNamedPipe;
	DWORD pid;
	PyObject *obhNamedPipe;
	// @pyparm <o PyHANDLE>|hPipe||The handle to the pipe.
	if (!PyArg_ParseTuple(args, "O:GetNamedPipeServerSessionId", &obhNamedPipe))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhNamedPipe, &hNamedPipe))
		return NULL;
	if (!(*pfnGetNamedPipeServerSessionId)(hNamedPipe, &pid))
		return PyWin_SetAPIError("GetNamedPipeServerSessionId");
	return PyLong_FromUnsignedLong(pid);
}
%}
%native(GetNamedPipeClientProcessId) MyGetNamedPipeClientProcessId;
%native(GetNamedPipeServerProcessId) MyGetNamedPipeServerProcessId;
%native(GetNamedPipeClientSessionId) MyGetNamedPipeClientSessionId;
%native(GetNamedPipeServerSessionId) MyGetNamedPipeServerSessionId;
