/***********************************************************

PyWinTypes.cpp -- implementation of standard win32 types


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "malloc.h"

PyObject * PyWinExc_ApiError = NULL;
PyObject * PyWinExc_COMError = NULL;

extern PyObject *PyWinMethod_NewHKEY(PyObject *self, PyObject *args);

#ifdef MS_WINCE
// Where is this supposed to come from on CE???
const GUID GUID_NULL \
                = { 0, 0, 0, { 0, 0,  0,  0,  0,  0,  0,  0 } };
#endif

// See comments in pywintypes.h for why we need this!
void PyWin_MakePendingCalls()
{
	while (1) {
		int rc = Py_MakePendingCalls();
		if (rc == 0)
			break;
		// An exception - just report it as normal.
		// Note that a traceback is very unlikely!
		// XXX - need somewhere reasonable for these to go!!
		fprintf(stderr, "Unhandled exception detected before entering Python.\n");
		PyErr_Clear();
		// And loop around again until we are told everything is done!
	}
}


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
	PyObject *out = NULL;

	if (PyInt_Check(obSocket))
	{
		*ps = (SOCKET)PyInt_AS_LONG(obSocket);
	}
	else
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
// PyThreadState *_tlsThisThreadState = NULL; // thread local storage.

// This structure is stored in the TLS slot.
struct ThreadData{
	PyThreadState *ts;
	BOOL owned; // do we free the state when we die?
};

PyInterpreterState *PyWin_InterpreterState;

// This function must be called at some time when the interpreter lock and state is valid.
// Called by init{module} functions and also COM factory entry point.
void PyWinInterpreterState_Ensure()
{
	if (PyWin_InterpreterState==NULL) {
		PyThreadState *threadStateSave = PyThreadState_Swap(NULL);
		if (threadStateSave==NULL)
			Py_FatalError("pywintypes: can not setup interpreter state, as current state is invalid");

		PyWin_InterpreterState = threadStateSave->interp;
		PyThreadState_Swap(threadStateSave);
	}
	// Save the main thread's state in the TLS map, but not owned.
	ThreadData *pData = (ThreadData *)LocalAlloc(LMEM_ZEROINIT, sizeof(ThreadData));
	if (!pData)
		Py_FatalError("Out of memory allocating thread state.");
	TlsSetValue(dwTlsIndex, pData);
	pData->ts = PyThreadState_Swap(NULL);
	PyThreadState_Swap(pData->ts);
	pData->owned = FALSE;
}

void PyWinInterpreterState_Free()
{
	PyWinThreadState_Free();
	PyWin_InterpreterState = NULL; // Eek - should I be freeing something?
}

// Ensure that we have a Python thread state available to use.
// If this is called for the first time on a thread, it will allocate
// the thread state.  This does NOT change the state of the Python lock.
// Returns TRUE if a new thread state was created, or FALSE if a
// thread state already existed.
#ifdef TRACE_THREADSTATE
static LONG numThreadStatesCreated = 0;
static LONG numAcquires = 0;
#endif /* TRACE_THREADSTATE */

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
		pData->owned = TRUE;
#ifdef TRACE_THREADSTATE
		InterlockedIncrement(&numThreadStatesCreated);
#endif /* TRACE_THREADSTATE */
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
#ifdef TRACE_THREADSTATE
	InterlockedIncrement(&numAcquires);
#endif /* TRACE_THREADSTATE */
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
	if (!pData || !pData->owned) return;
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

#ifdef TRACE_THREADSTATE
static PyObject *_GetLockStats(PyObject *, PyObject *)
{
	return Py_BuildValue("ll", numThreadStatesCreated, numAcquires);
}
#endif /* TRACE_THREADSTATE */

#endif

// Support the fact that error messages can come from any number of DLLs.
// wininet certainly does.  The win32net and MAPI modules could probably 
// also take advantage of this,
struct error_message_module {
    DWORD firstError;
    DWORD lastError;
    HMODULE hmodule;
};

// just allow a limit of 10 for now!
#define MAX_MESSAGE_MODULES 10
static error_message_module error_message_modules[MAX_MESSAGE_MODULES];
static int num_message_modules = 0;

BOOL PyWin_RegisterErrorMessageModule(DWORD first, DWORD last, HINSTANCE hmod)
{
	if (num_message_modules>=MAX_MESSAGE_MODULES) {
		assert(0); // need a either bump the limit, or make a real implementation!
		return FALSE;
	}
	error_message_modules[num_message_modules].firstError = first;
	error_message_modules[num_message_modules].lastError = last;
	error_message_modules[num_message_modules].hmodule = hmod;
	num_message_modules += 1;
	return TRUE;
}

HINSTANCE PyWin_GetErrorMessageModule(DWORD err)
{
	int i;
	for (i=0;i<num_message_modules;i++) {
		if ((DWORD)err >= error_message_modules[i].firstError && 
			(DWORD)err <= error_message_modules[i].lastError) {
			return error_message_modules[i].hmodule;
		}
	}
	return NULL;
}

/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *PyWin_SetAPIError(char *fnName, long err /*= 0*/)
{
	DWORD errorCode = err == 0 ? GetLastError() : err;
	DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | \
	              FORMAT_MESSAGE_IGNORE_INSERTS;
	// try and find the hmodule providing this error.
	HMODULE hmodule = PyWin_GetErrorMessageModule(errorCode);
	if (hmodule)
		flags |= FORMAT_MESSAGE_FROM_HMODULE;
	TCHAR *buf = NULL;
	BOOL free_buf = TRUE;
	if (errorCode)
		::FormatMessage(flags, hmodule, errorCode, 0, (LPTSTR)&buf, 0, NULL );
	if (!buf) {
		buf = _T("No error message is available");
		free_buf = FALSE;
	}
	/* strip trailing cr/lf */
	size_t end = _tcslen(buf)-1;
	if (end>1 && (buf[end-1]==_T('\n') || buf[end-1]==_T('\r')))
		buf[end-1] = _T('\0');
	else
		if (end>0 && (buf[end]==_T('\n') || buf[end]==_T('\r')))
			buf[end]=_T('\0');
	PyObject *obBuf = PyString_FromTCHAR(buf);
	if (free_buf && buf)
		LocalFree(buf);
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

PyObject *PyObject_FromWIN32_FIND_DATAA(WIN32_FIND_DATAA *pData)
{
	// @object WIN32_FIND_DATA|A tuple representing a WIN32_FIND_DATA structure.
	return Py_BuildValue("lNNNNNNNss",
		pData->dwFileAttributes, // @tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
		PyWinObject_FromFILETIME(pData->ftCreationTime), // @tupleitem 1|<o PyTime>|createTime|File creation time.
		PyWinObject_FromFILETIME(pData->ftLastAccessTime), // @tupleitem 2|<o PyTime>|accessTime|File access time.
		PyWinObject_FromFILETIME(pData->ftLastWriteTime), // @tupleitem 3|<o PyTime>|writeTime|Time of last file write
		PyLong_FromUnsignedLong(pData->nFileSizeHigh), // @tupleitem 4|int|nFileSizeHigh|high order DWORD of file size.
		PyLong_FromUnsignedLong(pData->nFileSizeLow),	// @tupleitem 5|int|nFileSizeLow|low order DWORD of file size.
		PyLong_FromUnsignedLong(pData->dwReserved0),	// @tupleitem 6|int|reserved0|Contains reparse tag if path is a reparse point
		PyLong_FromUnsignedLong(pData->dwReserved1),   // @tupleitem 7|int|reserved1|Reserved.
		pData->cFileName,     // @tupleitem 8|str/unicode|fileName|The name of the file.
		pData->cAlternateFileName); // @tupleitem 9|str/unicode|alternateFilename|Alternative name of the file, expressed in 8.3 format.
}

PyObject *PyObject_FromWIN32_FIND_DATAW(WIN32_FIND_DATAW *pData)
{
	return Py_BuildValue("lNNNNNNNuu",
		pData->dwFileAttributes,
		PyWinObject_FromFILETIME(pData->ftCreationTime),
		PyWinObject_FromFILETIME(pData->ftLastAccessTime),
		PyWinObject_FromFILETIME(pData->ftLastWriteTime),
		PyLong_FromUnsignedLong(pData->nFileSizeHigh),
		PyLong_FromUnsignedLong(pData->nFileSizeLow),
		PyLong_FromUnsignedLong(pData->dwReserved0),
		PyLong_FromUnsignedLong(pData->dwReserved1),
		pData->cFileName,
		pData->cAlternateFileName);
}

// @object PyPOINT|Tuple of two ints (x,y) representing a POINT struct
BOOL PyWinObject_AsPOINT(PyObject *obpoint, LPPOINT ppoint)
{
	if (!PyTuple_Check(obpoint)){
		PyErr_SetString(PyExc_TypeError, "POINT must be a tuple of 2 ints (x,y)");
		return FALSE;
		}
	return PyArg_ParseTuple(obpoint, "ll;POINT must be a tuple of 2 ints (x,y)", 
			&ppoint->x, &ppoint->y);
}

// Return an IO_COUNTERS structure, used in win32process,i and win32job.i
PyObject *PyWinObject_FromIO_COUNTERS(PIO_COUNTERS pioc)
{
	return Py_BuildValue("{s:N,s:N,s:N,s:N,s:N,s:N}",
		"ReadOperationCount",  PyLong_FromUnsignedLongLong(pioc->ReadOperationCount),
		"WriteOperationCount", PyLong_FromUnsignedLongLong(pioc->WriteOperationCount),
		"OtherOperationCount", PyLong_FromUnsignedLongLong(pioc->OtherOperationCount),
		"ReadTransferCount",   PyLong_FromUnsignedLongLong(pioc->ReadTransferCount),
		"WriteTransferCount",  PyLong_FromUnsignedLongLong(pioc->WriteTransferCount),
		"OtherTransferCount",  PyLong_FromUnsignedLongLong(pioc->OtherTransferCount));
}

// Alocates and populates an array of DWORDS from a sequence of Python ints
BOOL PyWinObject_AsDWORDArray(PyObject *obdwords, DWORD **pdwords, DWORD *item_cnt, BOOL bNoneOk)
{
	BOOL ret=TRUE;
	DWORD bufsize, tuple_index;
	PyObject *dwords_tuple=NULL, *tuple_item;
	*pdwords=NULL;
	*item_cnt=0;
	if (obdwords==Py_None){
		if (bNoneOk)
			return TRUE;
		PyErr_SetString(PyExc_ValueError,"Sequence of dwords cannot be None");
		return FALSE;
		}
	if ((dwords_tuple=PySequence_Tuple(obdwords))==NULL)
		return FALSE;	// last exit without cleaning up
	*item_cnt=PyTuple_Size(dwords_tuple);
	bufsize=*item_cnt * sizeof(DWORD);
	*pdwords=(DWORD *)malloc(bufsize);
	if (*pdwords==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		ret=FALSE;
		}
	else
		for (tuple_index=0; tuple_index<*item_cnt; tuple_index++){
			tuple_item=PyTuple_GET_ITEM(dwords_tuple,tuple_index);
			(*pdwords)[tuple_index]=PyInt_AsLong(tuple_item);
			if (((*pdwords)[tuple_index]==-1) && PyErr_Occurred()){
				ret=FALSE;
				break;
				}
			}
	if (!ret)
		if (*pdwords!=NULL){
			free(*pdwords);
			*pdwords=NULL;
			*item_cnt=0;
			}
	Py_XDECREF(dwords_tuple);
	return ret;
}

/*
PyLong_AsVoidPtr is unsuitable for use in many places due to the following issues:

1. It fails to convert some types.  On 32-bit, it calls PyLong_AsLong
	which doesn't check if the type has number methods defined (tp_as_number).
	This causes it to fail for PyHANDLE's.
	However, it doesn't even fail consistently since on 64-bit it uses
	PyLong_AsLongLong which does check tp_as_number.

2. When it fails to convert an object (even one for which it should succeed!)
	it uses PyErr_BadInternalCall which returns a vague and misleading error.

3. The documentation says it's only guaranteed to work for objects created using
	PyLong_FromVoidPtr.  However, there's no way to call this from the
	interpreter which means that places which can also accept a plain number
	as well as an address have no way to ensure that both will be converted
	consistently.  Additionally, PyLong_FromVoidPtr just returns a python int or
	long so there is actually no way to verify that an object was created using
	that function and can be converted back to a usable address.

From the response to this bug report:
http://sourceforge.net/tracker/?func=detail&atid=105470&aid=1630863&group_id=5470
apparently if you want any reasonable or consistent behaviour from this function
you're expected to perform the type checking yourself first.
And if you have to do all that, why use the damn function at all ?
Accordingly, here is our own version.
*/
BOOL PyWinLong_AsVoidPtr(PyObject *ob, void **pptr)
{
#ifdef _WIN64
	*pptr=(void *)PyLong_AsLongLong(ob);
#else
	*pptr=(void *)PyInt_AsLong(ob);
#endif
	if (*pptr==(void *)-1 && PyErr_Occurred()){
		PyErr_Format(PyExc_TypeError,"Unable to convert %s to pointer-sized value", ob->ob_type->tp_name);
		return FALSE;
		}
	return TRUE;
}

PyObject *PyWinLong_FromVoidPtr(const void *ptr)
{
#ifdef _WIN64
	return PyLong_FromLongLong((LONG_PTR)ptr);
#else
	return PyInt_FromLong((LONG_PTR)ptr);
#endif
}


// @object PyResourceId|Identifies a resource or function in a module.
//	This can be a WORD-sized integer value (0-65536), or string/unicode
//	depending on whether the *A or *W API function is to be called.
//	Class atoms as used with <om win32gui.CreateWindow> are also treated
//	as resource ids since they can also be represented by a name or WORD id.
//	When passing resource names and types as strings, they are usually formatted
//	as a pound sign followed by decimal form of the id.  ('#42' for example)
BOOL PyWinObject_AsResourceIdA(PyObject *ob, char **presource_id, BOOL bNoneOK)
{
	// Plain character conversion
	if (PyWinObject_AsString(ob, presource_id, bNoneOK))
		return TRUE;
	PyErr_Clear();
	if (PyWinLong_AsVoidPtr(ob, (void **)presource_id) && IS_INTRESOURCE(*presource_id))
		return TRUE;
	*presource_id=NULL;
	PyErr_SetString(PyExc_TypeError, "Resource id/name must be string or int in the range 0-65536");
	return FALSE;
}

BOOL PyWinObject_AsResourceIdW(PyObject *ob, WCHAR **presource_id, BOOL bNoneOK)
{
	// Unicode version of above
	if (PyWinObject_AsWCHAR(ob, presource_id, bNoneOK))
		return TRUE;
	PyErr_Clear();
	if (PyWinLong_AsVoidPtr(ob, (void **)presource_id) && IS_INTRESOURCE(*presource_id))
		return TRUE;
	*presource_id=NULL;
	PyErr_SetString(PyExc_TypeError, "Resource id/name must be unicode or int in the range 0-65536");
	return FALSE;
}

// PyWinObject_FreeString is overloaded to accept either char * or WCHAR *
void PyWinObject_FreeResourceId(char *resource_id)
{
	if ((resource_id!=NULL) && !IS_INTRESOURCE(resource_id))
		PyWinObject_FreeString(resource_id);
}

void PyWinObject_FreeResourceId(WCHAR *resource_id)
{
	if ((resource_id!=NULL) && !IS_INTRESOURCE(resource_id))
		PyWinObject_FreeString(resource_id);
}


// Conversion for WPARAM and LPARAM
/* ??? WPARAM is defined as UINT_PTR, and LPARAM is defined as LONG_PTR.
	Make separate functions to avoid cast ??? */
BOOL PyWinObject_AsPARAM(PyObject *ob, WPARAM *pparam)
{
	if (ob==NULL || ob==Py_None){
		*pparam=NULL;
		return TRUE;
		}
#ifdef UNICODE
#define TCHAR_DESC "Unicode"
	if (PyUnicode_Check(ob)){
		*pparam = (WPARAM)PyUnicode_AS_UNICODE(ob);
		return TRUE;
		}
#else
#define TCHAR_DESC "String"	
	if (PyString_Check(ob)){
		*pparam = (WPARAM)PyString_AS_STRING(ob);
		return TRUE;
		}
#endif
	if (PyWinLong_AsVoidPtr(ob, (void **)pparam))
		return TRUE;

	PyErr_Clear();
	PyBufferProcs *pb = ob->ob_type->tp_as_buffer;
	if (pb != NULL && pb->bf_getreadbuffer)
		return pb->bf_getreadbuffer(ob,0,(VOID **)pparam)!=-1;
	PyErr_SetString(PyExc_TypeError, "WPARAM must be a " TCHAR_DESC ", int, or buffer object");
	return FALSE;
}

// @object PyRECT|Tuple of 4 ints defining a rectangle: (left, top, right, bottom)
BOOL PyWinObject_AsRECT(PyObject *obrect, LPRECT prect)
{
	if (!PyTuple_Check(obrect)){
		PyErr_SetString(PyExc_TypeError, "RECT must be a tuple of 4 ints (left, top, right, bottom)");
		return FALSE;
		}
	return PyArg_ParseTuple(obrect, "llll;RECT must be a tuple of 4 ints (left, top, right, bottom)", 
			&prect->left, &prect->top, &prect->right, &prect->bottom);
}

PyObject *PyWinObject_FromRECT(LPRECT prect)
{
	if (prect==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return Py_BuildValue("llll",
		prect->left, prect->top,
		prect->right, prect->bottom);
}

// Buffer conversion functions that use DWORD for length
BOOL PyWinObject_AsReadBuffer(PyObject *ob, void **buf, DWORD *buf_len, BOOL bNoneOk)
{
	if (ob==Py_None){
		if (bNoneOk){
			*buf_len=0;
			*buf=NULL;
			return TRUE;
			}
		PyErr_SetString(PyExc_TypeError, "Buffer cannot be None");
		return FALSE;
		}
	Py_ssize_t py_len;
	if (PyObject_AsReadBuffer(ob, (const void **)buf, &py_len)==-1)
		return FALSE;

#ifdef _WIN64
	if (py_len>MAXDWORD){
		PyErr_Format(PyExc_ValueError,"Buffer length can be at most %d characters", MAXDWORD);
		return FALSE;
		}
#endif

	*buf_len=(DWORD)py_len;
	return TRUE;
}

BOOL PyWinObject_AsWriteBuffer(PyObject *ob, void **buf, DWORD *buf_len, BOOL bNoneOk)
{
	if (ob==Py_None){
		if (bNoneOk){
			*buf_len=0;
			*buf=NULL;
			return TRUE;
			}
		PyErr_SetString(PyExc_TypeError, "Buffer cannot be None");
		return FALSE;
		}
	Py_ssize_t py_len;
	if (PyObject_AsWriteBuffer(ob, buf, &py_len)==-1)
		return FALSE;

#ifdef _WIN64
	if (py_len>MAXDWORD){
		PyErr_Format(PyExc_ValueError,"Buffer length can be at most %d characters", MAXDWORD);
		return FALSE;
		}
#endif

	*buf_len=(DWORD)py_len;
	return TRUE;
}


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
#endif // MS_WINCE
#ifndef NO_PYWINTYPES_SECURITY
	{"ACL",         PyWinMethod_NewACL, 1 },      // @pymeth ACL|Creates a new <o PyACL> object.
	{"SID",         PyWinMethod_NewSID, 1 },      // @pymeth SID|Creates a new <o PySID> object.
	{"SECURITY_ATTRIBUTES",         PyWinMethod_NewSECURITY_ATTRIBUTES, 1 },      // @pymeth SECURITY_ATTRIBUTES|Creates a new <o PySECURITY_ATTRIBUTES> object.
	{"SECURITY_DESCRIPTOR",         PyWinMethod_NewSECURITY_DESCRIPTOR, 1 },      // @pymeth SECURITY_DESCRIPTOR|Creates a new <o PySECURITY_DESCRIPTOR> object.
#endif // NO_PYWINTYPES_SECURITY
	{"HANDLE",      PyWinMethod_NewHANDLE, 1 },      // @pymeth HANDLE|Creates a new <o PyHANDLE> object.
	{"HKEY",        PyWinMethod_NewHKEY, 1 },      // @pymeth HKEY|Creates a new <o PyHKEY> object.
#ifdef TRACE_THREADSTATE
	{"_GetLockStats", _GetLockStats, 1},
#endif /* TRACE_THREADSTATE */
	{"WAVEFORMATEX",         PyWinMethod_NewWAVEFORMATEX, 1 },      // @pymeth WAVEFORMATEX|Creates a new <o PyWAVEFORMATEX> object.
	{NULL,			NULL}
};

void PyWinGlobals_Ensure()
{
	PyEval_InitThreads();
	PyWinInterpreterState_Ensure();
	if (PyWinExc_ApiError==NULL) {
		PyWinExc_ApiError = PyErr_NewException("pywintypes.error", NULL, NULL);
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
  ADD_CONSTANT(WAVE_FORMAT_PCM);

  // Add a few types.
#ifndef NO_PYWINTYPES_TIME
  PyDict_SetItemString(dict, "TimeType", (PyObject *)&PyTimeType);
#endif // NO_PYWINTYPES_TIME
#ifndef NO_PYWINTYPES_IID
  PyDict_SetItemString(dict, "IIDType", (PyObject *)&PyIIDType);
#endif // NO_PYWINTYPES_IID
  PyDict_SetItemString(dict, "UnicodeType", (PyObject *)&PyUnicodeType);
#ifndef NO_PYWINTYPES_SECURITY
  PyDict_SetItemString(dict, "SECURITY_ATTRIBUTESType", (PyObject *)&PySECURITY_ATTRIBUTESType);
  PyDict_SetItemString(dict, "SIDType", (PyObject *)&PySIDType);
  PyDict_SetItemString(dict, "ACLType", (PyObject *)&PyACLType);
#endif
  PyDict_SetItemString(dict, "HANDLEType", (PyObject *)&PyHANDLEType);
  PyDict_SetItemString(dict, "OVERLAPPEDType", (PyObject *)&PyOVERLAPPEDType);
  PyDict_SetItemString(dict, "DEVMODEType", (PyObject *)&PyDEVMODEType);
  PyDict_SetItemString(dict, "WAVEFORMATEXType", (PyObject *)&PyWAVEFORMATEXType);

}

#ifndef MS_WINCE
extern "C" __declspec(dllexport)
#endif
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifndef NO_PYWINTYPES_SECURITY
	FARPROC fp;
	// dll usually will already be loaded
	HMODULE hmodule=GetModuleHandle("AdvAPI32.dll");
	if (hmodule==NULL)
		hmodule=LoadLibrary("AdvAPI32.dll");
	if (hmodule){
		fp=GetProcAddress(hmodule,"AddAccessAllowedAce");
		if (fp)
			addaccessallowedace=(addacefunc)(fp);
		fp=GetProcAddress(hmodule,"AddAccessDeniedAce");
		if (fp)
			addaccessdeniedace=(addacefunc)(fp);
		fp=GetProcAddress(hmodule,"AddAccessAllowedAceEx");
		if (fp)
			addaccessallowedaceex=(addaceexfunc)(fp);
		fp=GetProcAddress(hmodule,"AddAccessAllowedObjectAce");
		if (fp)
			addaccessallowedobjectace=(addobjectacefunc)(fp);
		fp=GetProcAddress(hmodule,"AddAccessDeniedAceEx");
		if (fp)
			addaccessdeniedaceex=(addaceexfunc)(fp);
		fp=GetProcAddress(hmodule,"AddAccessDeniedObjectAce");
		if (fp)
			addaccessdeniedobjectace= (addobjectacefunc)(fp);
		fp=GetProcAddress(hmodule,"AddAuditAccessAceEx");
		if (fp)
			addauditaccessaceex=(BOOL (WINAPI *)(PACL, DWORD, DWORD, DWORD, PSID, BOOL, BOOL))(fp);
		fp=GetProcAddress(hmodule,"AddAuditAccessObjectAce");
		if (fp)
			addauditaccessobjectace=(BOOL (WINAPI *)(PACL,DWORD,DWORD,DWORD,GUID*,GUID*,PSID,BOOL,BOOL))(fp);
		fp=GetProcAddress(hmodule,"SetSecurityDescriptorControl");
		if (fp)
			setsecuritydescriptorcontrol=(BOOL (WINAPI *)(PSECURITY_DESCRIPTOR, SECURITY_DESCRIPTOR_CONTROL, SECURITY_DESCRIPTOR_CONTROL))(fp);
	}

#endif // NO_PYWINTYPES_SECURITY
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

			**
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

