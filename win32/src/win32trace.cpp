/* A debugging module for Python.

The design is for a set of functions that can be "printed" to from
one Python process, and the output read by another process.  Using different
processes is attractive for a number of reasons - debugging services, or GUI apps
where no output is available (eg ActiveX scripts in MSIE) etc etc etc.

It is assumed there may be many current clients sending output to the
tracer, but only one process reading it.  [Violating this will not cause a
crash, just cause only one of the processes to see a given piece of text.]

The implementation is very simple, because of the above assumptions.

* There is a mem-mapped file, with the first word being an integer, and the
  rest being string data.  The integer is the current length of the string.
* A write operation appends data to the buffer, and updates the length.
* A read operation reads the entire buffer, and resets the length to zero.
  (Thus, there is no way to read only chunks of the data)
* A single mutex protects the entire structure.  While the mutex is held, there
  can at worst be a strcpy, malloc, and integer change, so this should be reasonable.

Currently, the memmapped file is allocated in the system swap space, and only 64k of
data is allocated.  If this buffer fills before a server gets to read it, the _entire_
output is discarded, and the text written to the new, empty buffer.

See - I told you the implementation was simple :-)

*/

#include "windows.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"

PyObject *pModMe = NULL;

const size_t BUFFER_SIZE = 0x10000; // Includes size integer.
const char *MAP_OBJECT_NAME = "PythonTraceOutputMapping";
const char *MUTEX_OBJECT_NAME = "PythonTraceOutputMutex";
const char *EVENT_OBJECT_NAME = "PythonTraceOutputEvent";

HANDLE hMapFileRead = NULL; // The handle to the read side of the mem-mapped file
HANDLE hMapFileWrite = NULL; // The handle to the write side of the mem-mapped file
HANDLE hMutex = NULL;
// A Python wrapper around an auto-reset event so a reader knows when data is avail without polling.
PyHANDLE *obEvent = NULL; // a PyHANDLE object

void *pMapBaseRead = NULL;
void *pMapBaseWrite = NULL;

/* error helper */
static PyObject *ReturnError(char *msg, char *fnName = NULL)
{
	PyObject *v = Py_BuildValue("(izs)", 0, fnName, msg);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
	return NULL;
}

BOOL DoOpenMap( HANDLE *pHandle, VOID **ppPtr)
{
	SECURITY_ATTRIBUTES  sa;       // Security attributes.
	PSECURITY_DESCRIPTOR pSD;      // Pointer to SD.

	// Allocate memory for the security descriptor.

	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
	                        SECURITY_DESCRIPTOR_MIN_LENGTH);

	// Initialize the new security descriptor.

	InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

	// Add a NULL descriptor ACL to the security descriptor.
	SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE);

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = TRUE;

	*pHandle = CreateFileMapping((HANDLE)-1, &sa, PAGE_READWRITE, 0, BUFFER_SIZE, MAP_OBJECT_NAME);
	if (*pHandle==NULL) {
		PyWin_SetAPIError("CreateFileMapping");
		return FALSE;
	}
	if (hMutex==NULL) {
		hMutex = CreateMutex(&sa, FALSE, MUTEX_OBJECT_NAME);
		if (hMutex==NULL) {
			PyWin_SetAPIError("CreateMutex");
			CloseHandle(*pHandle);
			return FALSE;
		}
	}
	if (obEvent==NULL) {
		HANDLE hEvent = CreateEvent(&sa, FALSE, FALSE, EVENT_OBJECT_NAME);
		if (hEvent==NULL) {
			PyWin_SetAPIError("CreateEvent");
			CloseHandle(*pHandle);
			return FALSE;
		}
		obEvent = (PyHANDLE *)PyWinObject_FromHANDLE(hEvent);
		// All ref'd up!
	}
	*ppPtr = MapViewOfFile(*pHandle, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
	if (*ppPtr==NULL) {
		PyWin_SetAPIError("MapViewOfFile");
		CloseHandle(*pHandle);
	}
	return (*ppPtr!=NULL);
}

BOOL DoCloseMap( HANDLE *pHandle, VOID **ppPtr)
{
	if (*ppPtr) {
		UnmapViewOfFile(*ppPtr);
		*ppPtr=NULL;
	}
	if (*pHandle) {
		CloseHandle(*pHandle);
		*pHandle = NULL;
	}
	if (hMapFileRead==NULL && hMapFileWrite==NULL && hMutex != NULL) {
		CloseHandle(hMutex);
		hMutex = NULL;
		if (obEvent != NULL) {
			HANDLE hEvent = (HANDLE)obEvent->asLong();
			CloseHandle(hEvent);
			Py_XDECREF(obEvent);
			obEvent = NULL;
		}
	}
	return TRUE;
}

BOOL GetMyMutex()
{
	// Give the mutex 10 seconds before timing out
	if (WaitForSingleObject(hMutex, 10*1000)==WAIT_FAILED) {
		PyWin_SetAPIError("WaitForSingleObject", GetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL ReleaseMyMutex()
{
	if (!ReleaseMutex(hMutex)) {
		PyWin_SetAPIError("ReleaseMutex", GetLastError());
		return FALSE;
	}
	return TRUE;
}


BOOL OpenReadMap()
{
	return DoOpenMap( &hMapFileRead, &pMapBaseRead);
}

BOOL CloseReadMap()
{
	return DoCloseMap( &hMapFileRead, &pMapBaseRead);
}

BOOL OpenWriteMap()
{
	return DoOpenMap( &hMapFileWrite, &pMapBaseWrite);
}

BOOL CloseWriteMap()
{
	return DoCloseMap( &hMapFileWrite, &pMapBaseWrite);
}

BOOL WriteData(const char *data, unsigned len)
{
	if (pMapBaseWrite == NULL) {
		ReturnError("The module has not been setup for writing");
		return FALSE;
	}
	if (len>BUFFER_SIZE-sizeof(size_t)-1) {
		ReturnError("The data is too large.");
		return FALSE;
	}
	if (!GetMyMutex())
		return FALSE;

	size_t *pLen = (size_t *)pMapBaseWrite;
	char *buffer = (char *)(((size_t *)pMapBaseWrite)+1);

	size_t sizeLeft = (BUFFER_SIZE-sizeof(size_t)) - *pLen;
	if (sizeLeft<len)
		*pLen = 0;
	
	memcpy(buffer+(*pLen), data, len);
	*pLen += len;
	BOOL rc = ReleaseMyMutex();
	HANDLE hEvent = (HANDLE)obEvent->asLong();

	SetEvent(hEvent);
	return rc;
}

BOOL ReadData(char **ppResult, int *retSize, int waitMilliseconds)
{
	if (pMapBaseRead == NULL) {
		ReturnError("The module has not been setup for reading");
		return FALSE;
	}
	if (waitMilliseconds!=0) {
		HANDLE hEvent = (HANDLE)obEvent->asLong();
		if (WaitForSingleObject(hEvent, waitMilliseconds)==WAIT_FAILED) {
			PyWin_SetAPIError("WaitForSingleObject", GetLastError());
			return FALSE;
		}
	}

	if (!GetMyMutex())
		return FALSE;

	size_t *pLen = (size_t *)pMapBaseRead;
	char *buffer = (char *)(((size_t *)pMapBaseRead)+1);

	char *result = (char *)malloc(*pLen + 1);
	if (result==NULL) {
		ReleaseMyMutex();
		PyErr_SetString(PyExc_MemoryError, "Allocating buffer for trace data");
		return FALSE;
	}
	memcpy(result, buffer, *pLen);
	result[*pLen] = '\0';
	*retSize = *pLen;
	*pLen = 0;

	if (!ReleaseMyMutex()) {
		free(result);
		return FALSE;
	}
	*ppResult = result;
	return TRUE;
}

static PyObject *win32trace_InitRead(PyObject *self, PyObject *args)
{
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = OpenReadMap();
	Py_END_ALLOW_THREADS

	if (!ok)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_InitWrite(PyObject *self, PyObject *args)
{
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = OpenWriteMap();
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_TermRead(PyObject *self, PyObject *args)
{
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = CloseReadMap();
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_TermWrite(PyObject *self, PyObject *args)
{
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = CloseWriteMap();
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_write(PyObject *self, PyObject *args)
{
	int len;
	char *data;
	if (!PyArg_ParseTuple(args, "s#:write", &data, &len))
		return NULL;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = WriteData(data, len);
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}
static PyObject *win32trace_read(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":read"))
		return NULL;
	int len;
	char *data;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReadData(&data, &len, 0);
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	PyObject *result = PyString_FromStringAndSize(data, len);
	free(data);
	return result;
}

static PyObject *win32trace_blockingread(PyObject *self, PyObject *args)
{
	int milliSeconds = INFINITE;
	if (!PyArg_ParseTuple(args, "|i:blockingread", &milliSeconds))
		return NULL;
	int len;
	char *data;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReadData(&data, &len, milliSeconds);
	Py_END_ALLOW_THREADS
	if (!ok)
		return NULL;
	PyObject *result = PyString_FromStringAndSize(data, len);
	free(data);
	return result;
}

static PyObject *win32trace_setprint(PyObject *self, PyObject *args)
{
	PySys_SetObject("stdout", pModMe);
	PySys_SetObject("stderr", pModMe);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_flush(PyObject *self, PyObject *args)
{
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win32trace_GetHandle(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetHandle"))
		return NULL;
	if (obEvent==NULL) {
		PyErr_SetString(PyExc_ValueError, "There is not handle setup for this module.");
		return NULL;
	}
	Py_INCREF(obEvent);
	return obEvent;
}
/* List of functions exported by this module */
// @object win32trace|A module providing out-of-process tracing capabilities for Python.
static struct PyMethodDef win32trace_functions[] = {
	{"GetHandle",         win32trace_GetHandle, 1}, // @pymeth GetHandle|
	{"InitRead",          win32trace_InitRead, 1 }, // @pymeth InitRead|
	{"InitWrite",         win32trace_InitWrite, 1 }, // @pymeth InitWrite|
	{"TermRead",          win32trace_TermRead, 1 }, // @pymeth TermRead|
	{"TermWrite",         win32trace_TermWrite, 1 }, // @pymeth TermWrite|
	{"write",             win32trace_write, 1 }, // @pymeth write|
	{"blockingread",      win32trace_blockingread, 1 }, // @pymeth blockingread|
	{"read",              win32trace_read, 1 }, // @pymeth read|
	{"setprint",          win32trace_setprint, 1 }, // @pymeth setprint|
	{"flush",             win32trace_flush, 1 }, // @pymeth flush|Does nothing, but included to better emulate file semantics.
	{NULL,			NULL}
};


extern "C" __declspec(dllexport) void
initwin32trace(void)
{
  PyWinGlobals_Ensure();
  PyObject *dict;
  pModMe = Py_InitModule("win32trace", win32trace_functions);
  dict = PyModule_GetDict(pModMe);
  Py_INCREF(PyWinExc_ApiError);
  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
}
