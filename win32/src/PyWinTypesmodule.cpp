/***********************************************************

PyWinTypes.cpp -- implementation of standard win32 types


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#define PY_SSIZE_T_CLEAN
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "malloc.h"

PyObject *PyWinExc_ApiError = NULL;
PyObject *PyWinExc_COMError = NULL;

extern PyObject *PyWinMethod_NewHKEY(PyObject *self, PyObject *args);

extern BOOL _PyWinDateTime_Init();
extern BOOL _PyWinDateTime_PrepareModuleDict(PyObject *dict);

HMODULE PyWin_GetOrLoadLibraryHandle(const char *name)
{
    DWORD lastErr = GetLastError();
    HMODULE hmodule = GetModuleHandleA(name);
    if (hmodule == NULL)
        hmodule = LoadLibraryA(name);
    if (hmodule != NULL)
        SetLastError(lastErr);
    return hmodule;
}

// XXX - Needs py3k modernization!
// For py3k, a function that returns new memoryview object instead of buffer.
// ??? Byte array object is mutable, maybe just use that directly as a substitute ???
// Docs do not specify that you can pass NULL buffer to PyByteArray_FromStringAndSize, but it works
PyObject *PyBuffer_New(Py_ssize_t size)
{
    PyObject *bah = PyByteArray_FromStringAndSize(NULL, size);
    if (bah == NULL)
        return NULL;
    PyObject *ret = PyMemoryView_FromObject(bah);
    Py_DECREF(bah);  // Memory view keeps its own ref to base object
    return ret;
}

PyObject *PyBuffer_FromMemory(void *buf, Py_ssize_t size)
{
    // buf is not freed by returned object !!!!!!!
    Py_ssize_t shape0 = size;
    Py_buffer info = {
        buf,
        NULL,     // obj added in 3.0b3
        size,     // len
        1,        // itemsize
        TRUE,     // readonly
        1,        // ndim
        NULL,     // format
        &shape0,  // shape
        NULL,     // strides
        NULL,     // suboffsets
        NULL,     // internal
    };
    return PyMemoryView_FromBuffer(&info);
}

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
    (PyObject *obSocket,
     // [in] Python object being converted into a SOCKET handle.
     SOCKET *ps
     // [out] Returned socket handle
    )
{
    PyObject *o = NULL;
    PyObject *out = NULL;

    // Most common case, a python socket object (which apparently has no public C API)
    o = PyObject_GetAttrString(obSocket, "fileno");
    if (o == NULL) {
        // Not a socket object, attempt direct conversion to integer handle
        PyErr_Clear();
        out = obSocket;
        Py_INCREF(out);
    }
    else if (PyCallable_Check(o)) {
        // Normal socket object, whose fileno() method returns the integer handle
        out = PyObject_CallObject(o, NULL);
        Py_DECREF(o);
        if (out == NULL)
            return FALSE;
    }
    else  // ??? fileno may be a number, rather than a method that returns a number ???
        out = o;

    BOOL bsuccess = PyWinLong_AsVoidPtr(out, (void **)ps);
    Py_DECREF(out);
    if (!bsuccess) {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "Expected a socket object or numeric socket handle");
    }
    return bsuccess;
}

#ifndef PYCOM_USE_FREE_THREAD
DWORD dwTlsIndex = 0;
// PyThreadState *_tlsThisThreadState = NULL; // thread local storage.

// This structure is stored in the TLS slot.
struct ThreadData {
    PyThreadState *ts;
    BOOL owned;  // do we free the state when we die?
};

PyInterpreterState *PyWin_InterpreterState;

// This function must be called at some time when the interpreter lock and state is valid.
// Called by init{module} functions and also COM factory entry point.
void PyWinInterpreterState_Ensure()
{
    if (PyWin_InterpreterState == NULL) {
        PyThreadState *threadStateSave = PyThreadState_Swap(NULL);
        if (threadStateSave == NULL)
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
    PyWin_InterpreterState = NULL;  // Eek - should I be freeing something?
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
    if (pData == NULL) { /* First request on this thread */
        /* Check we have an interpreter state */
        if (PyWin_InterpreterState == NULL) {
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
#endif                /* TRACE_THREADSTATE */
        return TRUE;  // Did create a thread state state
    }
    return FALSE;  // Thread state was previously created
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
    if (!pData || !pData->owned)
        return;
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
    if (num_message_modules >= MAX_MESSAGE_MODULES) {
        assert(0);  // need a either bump the limit, or make a real implementation!
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
    for (i = 0; i < num_message_modules; i++) {
        if ((DWORD)err >= error_message_modules[i].firstError && (DWORD)err <= error_message_modules[i].lastError) {
            return error_message_modules[i].hmodule;
        }
    }
    return NULL;
}

/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *PyWin_SetAPIError(char *fnName, long err /*= ERROR_SUCCESS*/)
{
    DWORD errorCode = err == ERROR_SUCCESS ? GetLastError() : err;
    DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;
    // try and find the hmodule providing this error.
    HMODULE hmodule = PyWin_GetErrorMessageModule(errorCode);
    if (hmodule)
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    TCHAR *buf = NULL;
    BOOL free_buf = TRUE;
    if (errorCode)
        ::FormatMessage(flags, hmodule, errorCode, 0, (LPTSTR)&buf, 0, NULL);
    if (!buf) {
        buf = _T("No error message is available");
        free_buf = FALSE;
    }
    /* strip trailing cr/lf */
    size_t end = _tcslen(buf) - 1;
    if (end > 1 && (buf[end - 1] == _T('\n') || buf[end - 1] == _T('\r')))
        buf[end - 1] = _T('\0');
    else if (end > 0 && (buf[end] == _T('\n') || buf[end] == _T('\r')))
        buf[end] = _T('\0');

    PyObject *v = Py_BuildValue("(iNN)", errorCode, PyWinCoreString_FromString(fnName), PyWinObject_FromTCHAR(buf));
    if (free_buf && buf)
        LocalFree(buf);
    if (v != NULL) {
        PyErr_SetObject(PyWinExc_ApiError, v);
        Py_DECREF(v);
    }
    return NULL;
}

/* error helper - like PyWin_SetAPIError, but returns None on success */
PyObject *PyWin_SetAPIErrorOrReturnNone(char *fnName, long err /*= ERROR_SUCCESS*/)
{
    DWORD errorCode = err == ERROR_SUCCESS ? GetLastError() : err;
    if (errorCode == ERROR_SUCCESS)
        Py_RETURN_NONE;
    return PyWin_SetAPIError(fnName, errorCode);
}

// This function sets a basic COM error - it is a valid COM
// error, but may not contain rich error text about the error.
// Designed to be used before pythoncom has been loaded.
// If a COM extension wants to raise a COM error, it should use
// the "real" functions exposed via pythoncom.
PyObject *PyWin_SetBasicCOMError(HRESULT hr)
{
    TCHAR buf[255];
    int bufSize = sizeof(buf) / sizeof(TCHAR);
    int numCopied = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, buf, bufSize, NULL);
    if (numCopied > 0) {
        if (numCopied < bufSize) {
            // trim trailing crap
            if (numCopied > 2 && (buf[numCopied - 2] == _T('\n') || buf[numCopied - 2] == _T('\r')))
                buf[numCopied - 2] = _T('\0');
        }
    }
    else {
        wsprintf(buf, _T("COM Error 0x%x"), hr);
    }
    PyObject *evalue = Py_BuildValue("iNzz", hr, PyWinObject_FromTCHAR(buf), NULL, NULL);
    PyErr_SetObject(PyWinExc_COMError, evalue);
    Py_XDECREF(evalue);
    return NULL;
}

// @pymethod string|pywintypes|UnicodeFromRaw|Creates a new Unicode object from raw binary data
static PyObject *PyWin_NewUnicodeFromRaw(PyObject *self, PyObject *args)
{
    PyObject *ob;

    // @pyparm string/buffer|str||The string containing the binary data.
    if (!PyArg_ParseTuple(args, "O", &ob))
        return NULL;
    PyWinBufferView pybuf(ob);
    if (!pybuf.ok())
        return NULL;
    return PyWinObject_FromWCHAR((WCHAR *)pybuf.ptr(), pybuf.len() / sizeof(OLECHAR));
}

// @pymethod int, int|pywintypes|IsTextUnicode|Determines whether a buffer probably contains a form of Unicode text.
static PyObject *PyWin_IsTextUnicode(PyObject *self, PyObject *args)
{
    const char *value;
    Py_ssize_t numBytes;
    int flags;

    // @pyparm string|str||The string containing the binary data.
    // @pyparm int|flags||Determines the specific tests to make
    if (!PyArg_ParseTuple(args, "s#i", &value, &numBytes, &flags))
        return NULL;
    if (numBytes > INT_MAX) {
        PyErr_SetString(PyExc_ValueError, "string size beyond INT_MAX");
        return NULL;
    }

    DWORD rc = IsTextUnicode((LPVOID)value, (int)numBytes, &flags);
    return Py_BuildValue("ii", rc, flags);
    // @rdesc The function returns (result, flags), both integers.
    // <nl>result is nonzero if the data in the buffer passes the specified tests.
    // <nl>result is zero if the data in the buffer does not pass the specified tests.
    // <nl>In either case, flags contains the results of the specific tests the function applied to make its
    // determination.
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
// @pymethod <o PyDateTime>|pywintypes|DosDateTimeToTime|Converts an MS-DOS Date/Time to a standard Time object.
static PyObject *PyWin_DosDateTimeToTime(PyObject *self, PyObject *args)
{
    WORD wFatDate, wFatTime;
    if (!PyArg_ParseTuple(args, "hh", (WORD *)&wFatDate, (WORD *)&wFatTime))
        return NULL;
    FILETIME fd;
    if (!DosDateTimeToFileTime(wFatDate, wFatTime, &fd))
        return PyWin_SetAPIError("DosDateTimeToFileTime");
    return PyWinObject_FromFILETIME(fd);
}

PyObject *PyObject_FromWIN32_FIND_DATAW(WIN32_FIND_DATAW *pData)
{
    return Py_BuildValue("lNNNNNNNuu", pData->dwFileAttributes, PyWinObject_FromFILETIME(pData->ftCreationTime),
                         PyWinObject_FromFILETIME(pData->ftLastAccessTime),
                         PyWinObject_FromFILETIME(pData->ftLastWriteTime),
                         PyLong_FromUnsignedLong(pData->nFileSizeHigh), PyLong_FromUnsignedLong(pData->nFileSizeLow),
                         PyLong_FromUnsignedLong(pData->dwReserved0), PyLong_FromUnsignedLong(pData->dwReserved1),
                         pData->cFileName, pData->cAlternateFileName);
}

// @object PyPOINT|Tuple of two ints (x,y) representing a POINT struct
BOOL PyWinObject_AsPOINT(PyObject *obpoint, LPPOINT ppoint)
{
    if (!PyTuple_Check(obpoint)) {
        PyErr_SetString(PyExc_TypeError, "POINT must be a tuple of 2 ints (x,y)");
        return FALSE;
    }
    return PyArg_ParseTuple(obpoint, "ll;POINT must be a tuple of 2 ints (x,y)", &ppoint->x, &ppoint->y);
}

// Return an IO_COUNTERS structure, used in win32process,i and win32job.i
PyObject *PyWinObject_FromIO_COUNTERS(PIO_COUNTERS pioc)
{
    return Py_BuildValue("{s:N,s:N,s:N,s:N,s:N,s:N}", "ReadOperationCount",
                         PyLong_FromUnsignedLongLong(pioc->ReadOperationCount), "WriteOperationCount",
                         PyLong_FromUnsignedLongLong(pioc->WriteOperationCount), "OtherOperationCount",
                         PyLong_FromUnsignedLongLong(pioc->OtherOperationCount), "ReadTransferCount",
                         PyLong_FromUnsignedLongLong(pioc->ReadTransferCount), "WriteTransferCount",
                         PyLong_FromUnsignedLongLong(pioc->WriteTransferCount), "OtherTransferCount",
                         PyLong_FromUnsignedLongLong(pioc->OtherTransferCount));
}

// Alocates and populates an array of DWORDS from a sequence of Python ints
BOOL PyWinObject_AsDWORDArray(PyObject *obdwords, DWORD **pdwords, DWORD *item_cnt, BOOL bNoneOk)
{
    BOOL ret = TRUE;
    DWORD bufsize, tuple_index;
    PyObject *dwords_tuple = NULL, *tuple_item;
    *pdwords = NULL;
    *item_cnt = 0;
    if (obdwords == Py_None) {
        if (bNoneOk)
            return TRUE;
        PyErr_SetString(PyExc_ValueError, "Sequence of dwords cannot be None");
        return FALSE;
    }
    if ((dwords_tuple = PyWinSequence_Tuple(obdwords, item_cnt)) == NULL)
        return FALSE;  // last exit without cleaning up
    bufsize = *item_cnt * sizeof(DWORD);
    *pdwords = (DWORD *)malloc(bufsize);
    if (*pdwords == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
        ret = FALSE;
    }
    else
        for (tuple_index = 0; tuple_index < *item_cnt; tuple_index++) {
            tuple_item = PyTuple_GET_ITEM(dwords_tuple, tuple_index);
            // Doesn't check for overflow, but will accept a python long
            //  greater than INT_MAX (even on Python 2.3).  Also accepts
            //  negatives and converts to the correct hex representation
            (*pdwords)[tuple_index] = PyLong_AsUnsignedLongMask(tuple_item);
            if (((*pdwords)[tuple_index] == -1) && PyErr_Occurred()) {
                ret = FALSE;
                break;
            }
        }
    if (!ret)
        if (*pdwords != NULL) {
            free(*pdwords);
            *pdwords = NULL;
            *item_cnt = 0;
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
    (except starting in 3.10, it no longer does!)

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
https://github.com/python/cpython/issues/44430
apparently if you want any reasonable or consistent behaviour from this function
you're expected to perform the type checking yourself first.
And if you have to do all that, why use the damn function at all ?
Accordingly, here is our own version.
*/
BOOL PyWinLong_AsVoidPtr(PyObject *ob, void **pptr)
{
    assert(!PyErr_Occurred());  // lingering exception?
                                // PyLong_AsLong (and PyLong_AsLongLong on x64) handle objects
                                // with tp_number slots, and longs that fit in 32bits - but *not*
                                // longs that fit in 32bits if they are treated as unsigned - eg,
                                // eg, the result of:
                                // struct.unpack("P", struct.pack("P", -1)) -> (4294967295L,)
                                // So, we do the PyLong_AsLong thing first, then fall back to the
                                // *_AsUnsignedLong[Long] versions.
#ifdef _WIN64
#define SIGNED_CONVERTER PyLong_AsLongLong
#define UNSIGNED_CONVERTER PyLong_AsUnsignedLongLong
#else
#define SIGNED_CONVERTER PyLong_AsLong
#define UNSIGNED_CONVERTER PyLong_AsUnsignedLong
#endif
    // Since Python 3.10, calling __int__ is no longer done, so we convert to an int explicitly.
    PyObject *longob = PyNumber_Long(ob);
    if (!longob && PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError, "Unable to convert %s to pointer-sized value", Py_TYPE(ob)->tp_name);
        return FALSE;
    }
    *pptr = (void *)SIGNED_CONVERTER(longob);
    if (*pptr == (void *)-1 && PyErr_Occurred()) {
        PyErr_Clear();
        *pptr = (void *)UNSIGNED_CONVERTER(longob);
        if (*pptr == (void *)-1 && PyErr_Occurred()) {
            Py_DECREF(longob);
            PyErr_Format(PyExc_TypeError, "Unable to convert %s to pointer-sized value", Py_TYPE(ob)->tp_name);
            return FALSE;
        }
    }
    Py_DECREF(longob);
    return TRUE;
}

PyObject *PyWinLong_FromVoidPtr(const void *ptr)
{
#ifdef _WIN64
    return PyLong_FromLongLong((LONG_PTR)ptr);
#else
    return PyLong_FromLong((LONG_PTR)ptr);
#endif
}

// @object PyResourceId|Identifies a resource or function in a module.
//    This can be a WORD-sized integer value (0-65536), or bytes.
BOOL PyWinObject_AsResourceIdA(PyObject *ob, char **presource_id, BOOL bNoneOK)
{
    // Plain character conversion
    if (PyWinObject_AsChars(ob, presource_id, bNoneOK))
        return TRUE;
    PyErr_Clear();
    if (PyWinLong_AsVoidPtr(ob, (void **)presource_id) && IS_INTRESOURCE(*presource_id))
        return TRUE;
    *presource_id = NULL;
    PyErr_SetString(PyExc_TypeError, "Resource id/name must be string or int in the range 0-65536");
    return FALSE;
}

void PyWinObject_FreeResourceIdA(char *resource_id)
{
    if ((resource_id != NULL) && !IS_INTRESOURCE(resource_id))
        PyWinObject_FreeChars(resource_id);
}

// @object PyResourceId|Identifies a resource or function in a module.
//    This can be a WORD-sized integer value (0-65536), or unicode.
//    Class atoms as used with <om win32gui.CreateWindow> are also treated
//    as resource ids since they can also be represented by a name or WORD id.
//    When passing resource names and types as strings, they are usually formatted
//    as a pound sign followed by decimal form of the id.  ('#42' for example)
BOOL PyWinObject_AsResourceId(PyObject *ob, WCHAR **presource_id, BOOL bNoneOK)
{
    if (PyWinObject_AsWCHAR(ob, presource_id, bNoneOK))
        return TRUE;
    PyErr_Clear();
    if (PyWinLong_AsVoidPtr(ob, (void **)presource_id) && IS_INTRESOURCE(*presource_id))
        return TRUE;
    *presource_id = NULL;
    PyErr_SetString(PyExc_TypeError, "Resource id/name must be unicode or int in the range 0-65536");
    return FALSE;
}

void PyWinObject_FreeResourceId(WCHAR *resource_id)
{
    if ((resource_id != NULL) && !IS_INTRESOURCE(resource_id))
        PyWinObject_FreeWCHAR(resource_id);
}

// Conversion for WPARAM and LPARAM from a simple integer value. Used when we
// can't guarantee memory pointed at will remain valid as long as necessary.
// In that scenario, the caller is responsible for arranging memory safety.
BOOL PyWinObject_AsSimplePARAM(PyObject *ob, WPARAM *wparam)
{
    // convert simple integers directly
    void *simple = PyLong_AsVoidPtr(ob);
    if (simple || !PyErr_Occurred()) {
        *wparam = (WPARAM)simple;
        return TRUE;
    }
    PyErr_Clear();

    // unlikely - convert any object providing .__int__() for backward compatibility
    if (PyWinLong_AsVoidPtr(ob, (void **)wparam)) {
        return TRUE;
    }

    PyErr_Format(PyExc_TypeError, "WPARAM is simple, so must be an int object (got %s)", Py_TYPE(ob)->tp_name);
    return FALSE;
}

// Converts for WPARAM and LPARAM: int or str (WCHAR*) or buffer (pointer to its locked memory)
// (WPARAM is defined as UINT_PTR, and LPARAM is defined as LONG_PTR - see
// pywintypes.h for inline functions to resolve this)
BOOL PyWinObject_AsPARAM(PyObject *ob, PyWin_PARAMHolder *holder)
{
    assert(!PyErr_Occurred());  // lingering exception?
    if (ob == NULL || ob == Py_None) {
        *holder = (WPARAM)0;
        return TRUE;
    }

    // fast-track - most frequent by far are simple integers
    void *simple = PyLong_AsVoidPtr(ob);
    if (simple || !PyErr_Occurred()) {
        *holder = (WPARAM)simple;
        return TRUE;
    }
    PyErr_Clear();

    if (PyUnicode_Check(ob)) {
        return holder->set_allocated(PyUnicode_AsWideCharString(ob, NULL)) != NULL;
    }

    if (holder->init_buffer(ob)) {
        return TRUE;
    }
    PyErr_Clear();

    // Finally try to convert any object providing .__int__() . That's undocumented
    // and probably not used from inside pywin32. But existing for long time and won't impact
    // speed here at the end of the game.
    if (PyWinLong_AsVoidPtr(ob, &simple)) {
        *holder = (WPARAM)simple;
        return TRUE;
    }

    PyErr_Format(PyExc_TypeError, "WPARAM must be a unicode string, int, or buffer object (got %s)",
                 Py_TYPE(ob)->tp_name);
    return FALSE;
}

// @object PyRECT|Tuple of 4 ints defining a rectangle: (left, top, right, bottom)
BOOL PyWinObject_AsRECT(PyObject *obrect, LPRECT prect)
{
    if (!PyTuple_Check(obrect)) {
        PyErr_SetString(PyExc_TypeError, "RECT must be a tuple of 4 ints (left, top, right, bottom)");
        return FALSE;
    }
    return PyArg_ParseTuple(obrect, "llll;RECT must be a tuple of 4 ints (left, top, right, bottom)", &prect->left,
                            &prect->top, &prect->right, &prect->bottom);
}

PyObject *PyWinObject_FromRECT(LPRECT prect)
{
    if (prect == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return Py_BuildValue("llll", prect->left, prect->top, prect->right, prect->bottom);
}

// When init() fails, an appropriate Python error has been set too
bool PyWinBufferView::init(PyObject *ob, bool bWrite, bool bNoneOk)
{
    release();
    if (ob == Py_None) {
        if (bNoneOk) {
            // using Py_None as sentinel, for handling a "valid" NULL buffer pointer
            m_view.obj = Py_None;
            m_view.buf = NULL;
            m_view.len = 0;
        }
        else
            PyErr_SetString(PyExc_TypeError, "Buffer cannot be None");
    }
    else if (ob != NULL) {
        PyObject_GetBuffer(ob, &m_view, bWrite ? PyBUF_WRITABLE : PyBUF_SIMPLE);

#ifdef _WIN64
        if (m_view.obj && m_view.len > MAXDWORD) {
            PyBuffer_Release(&m_view);  // already sets view->obj = NULL
            PyErr_Format(PyExc_ValueError, "Buffer length can be at most %d characters", MAXDWORD);
        }
#endif
    }
    else  // ob == NULL handled as not ok
        m_view.obj = NULL;
    return ok();
}

// Converts sequence into a tuple and verifies that length fits in length variable
PyObject *PyWinSequence_Tuple(PyObject *obseq, DWORD *len)
{
    PyObject *obtuple = PySequence_Tuple(obseq);
    if (obtuple == NULL)
        return NULL;
    Py_ssize_t py_len = PyTuple_GET_SIZE(obtuple);
    if (py_len > MAXDWORD) {
        Py_DECREF(obtuple);
        return PyErr_Format(PyExc_ValueError, "Sequence can contain at most %d items", MAXDWORD);
    }
    *len = (DWORD)py_len;
    return obtuple;
}

// @object PyMSG|A tuple representing a win32 MSG structure.
BOOL PyWinObject_AsMSG(PyObject *ob, MSG *pMsg)
{
    PyObject *obhwnd, *obwParam, *oblParam;
    if (!PyArg_ParseTuple(ob, "OiOOi(ii):MSG param",
                          &obhwnd,         // @tupleitem 0|<o PyHANDLE>|hwnd|Handle to the window whose window procedure
                                           // receives the message.
                          &pMsg->message,  // @tupleitem 1|int|message|Specifies the message identifier.
                          &obwParam,    // @tupleitem 2|int|wParam|Specifies additional information about the message.
                          &oblParam,    // @tupleitem 3|int|lParam|Specifies additional information about the message.
                          &pMsg->time,  // @tupleitem 4|int|time|Specifies the time at which the message was posted
                                        // (retrieved via GetTickCount()).
                          &pMsg->pt.x,  // @tupleitem 5|(int, int)|point|Specifies the cursor position, in screen
                                        // coordinates, when the message was posted.
                          &pMsg->pt.y))
        return FALSE;
    return PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&pMsg->hwnd) && PyWinObject_AsSimplePARAM(obwParam, &pMsg->wParam) &&
           PyWinObject_AsSimplePARAM(oblParam, &pMsg->lParam);
}

PyObject *PyWinObject_FromMSG(const MSG *pMsg)
{
    return Py_BuildValue("NiNNi(ii)", PyWinLong_FromHANDLE(pMsg->hwnd), pMsg->message,
                         PyWinObject_FromPARAM(pMsg->wParam), PyWinObject_FromPARAM(pMsg->lParam), pMsg->time,
                         pMsg->pt.x, pMsg->pt.y);
}

/* List of functions exported by this module */
// @module pywintypes|A module which supports common Windows types.
static struct PyMethodDef pywintypes_functions[] = {
    {"DosDateTimeToTime", PyWin_DosDateTimeToTime,
     1},  // @pymeth DosDateTimeToTime|Converts an MS-DOS Date/Time to a standard Time object
    {"UnicodeFromRaw", PyWin_NewUnicodeFromRaw,
     1},  // @pymeth UnicodeFromRaw|Creates a new string object from raw binary data
    {"IsTextUnicode", PyWin_IsTextUnicode,
     1},  // @pymeth IsTextUnicode|Determines whether a buffer probably contains a form of Unicode text.
    {"OVERLAPPED", PyWinMethod_NewOVERLAPPED, 1},  // @pymeth OVERLAPPED|Creates a new <o PyOVERLAPPED> object
#ifndef NO_PYWINTYPES_IID
    {"IID", PyWinMethod_NewIID, 1},  // @pymeth IID|Makes an <o PyIID> object from a string.
#endif
    {"Time", PyWinMethod_NewTime, 1},            // @pymeth Time|Makes a <o PyDateTime> object from the argument.
    {"TimeStamp", PyWinMethod_NewTimeStamp, 1},  // @pymeth Time|Makes a <o PyDateTime> object from the argument.
    {"CreateGuid", PyWin_CreateGuid, 1},         // @pymeth CreateGuid|Creates a new, unique GUIID.
    {"ACL", PyWinMethod_NewACL, 1},              // @pymeth ACL|Creates a new <o PyACL> object.
    {"SID", PyWinMethod_NewSID, 1},              // @pymeth SID|Creates a new <o PySID> object.
    {"SECURITY_ATTRIBUTES", PyWinMethod_NewSECURITY_ATTRIBUTES,
     1},  // @pymeth SECURITY_ATTRIBUTES|Creates a new <o PySECURITY_ATTRIBUTES> object.
    {"SECURITY_DESCRIPTOR", PyWinMethod_NewSECURITY_DESCRIPTOR,
     1},  // @pymeth SECURITY_DESCRIPTOR|Creates a new <o PySECURITY_DESCRIPTOR> object.
    {"HANDLE", PyWinMethod_NewHANDLE, 1},  // @pymeth HANDLE|Creates a new <o PyHANDLE> object.
    {"HKEY", PyWinMethod_NewHKEY, 1},      // @pymeth HKEY|Creates a new <o PyHKEY> object.
#ifdef TRACE_THREADSTATE
    {"_GetLockStats", _GetLockStats, 1},
#endif                                                 /* TRACE_THREADSTATE */
    {"WAVEFORMATEX", PyWinMethod_NewWAVEFORMATEX, 1},  // @pymeth WAVEFORMATEX|Creates a new <o PyWAVEFORMATEX> object.
    {NULL, NULL}};

int PyWinGlobals_Ensure()
{
    PyWinInterpreterState_Ensure();
    if (PyWinExc_ApiError == NULL) {
        // Setup our exception objects so they have attributes.
        // do things the easy way - but also the (hopefully!) smart way
        // as our exception objects always behave exactly like they were
        // defined in regular .py code - because the are!
        PyObject *d = PyDict_New();
        if (!d)
            return -1;
        PyObject *name = PyWinCoreString_FromString("pywintypes");
        if (!name) {
            Py_DECREF(d);
            return -1;
        }
        PyDict_SetItemString(d, "Exception", PyExc_Exception);
        PyDict_SetItemString(d, "__name__", name);
        Py_DECREF(name);
        PyObject *bimod = PyImport_ImportModule("builtins");
        if ((bimod == NULL) || PyDict_SetItemString(d, "__builtins__", bimod) == -1) {
            Py_XDECREF(bimod);
            return -1;
        }
        Py_DECREF(bimod);

        // Note using 'super()' doesn't work as expected on py23...
        // Need to be careful to support "insane" args...
        PyObject *res = PyRun_String(
            "class error(Exception):\n"
            "  def __init__(self, *args, **kw):\n"
            "    nargs = len(args)\n"
            "    if nargs > 0: self.winerror = args[0]\n"
            "    else: self.winerror = None\n"
            "    if nargs > 1: self.funcname = args[1]\n"
            "    else: self.funcname = None\n"
            "    if nargs > 2: self.strerror = args[2]\n"
            "    else: self.strerror = None\n"
            "    Exception.__init__(self, *args, **kw)\n"
            "class com_error(Exception):\n"
            "  def __init__(self, *args, **kw):\n"
            "    nargs = len(args)\n"
            "    if nargs > 0: self.hresult = args[0]\n"
            "    else: self.hresult = None\n"
            "    if nargs > 1: self.strerror = args[1]\n"
            "    else: self.strerror = None\n"
            "    if nargs > 2: self.excepinfo = args[2]\n"
            "    else: self.excepinfo = None\n"
            "    if nargs > 3: self.argerror = args[3]\n"
            "    else: self.argerror = None\n"
            "    Exception.__init__(self, *args, **kw)\n",
            Py_file_input, d, d);
        if (res == NULL)
            return -1;
        Py_DECREF(res);

        PyWinExc_ApiError = PyDict_GetItemString(d, "error");
        Py_XINCREF(PyWinExc_ApiError);
        PyWinExc_COMError = PyDict_GetItemString(d, "com_error");
        Py_XINCREF(PyWinExc_COMError);
        Py_DECREF(d);
        // @object error|An exception raised when a win32 error occurs
        // @comm This error is defined in the pywintypes module, but most
        // of the win32 modules expose this error object via their own
        // error attribute - eg, win32api.error is pywintypes.error is
        // win32gui.error.
        // @comm This exception is derived from the standard Python Exception object.
        // @comm Instances of these exception can be accessed via indexing
        // or via attribute access.  Attribute access is more forwards
        // compatible with Python 3, so is recommended.
        // @comm See also <o com_error>
        // @tupleitem 0|int|winerror|The windows error code.
        // @tupleitem 1|string|funcname|The name of the windows function that failed.
        // @tupleitem 2|string|strerror|The error message.

        // @object com_error|An exception raised when a COM exception occurs.
        // @comm This error is defined in the pywintypes module, but is
        // also available via pythoncom.com_error.
        // @comm This exception is derived from the standard Python Exception object.
        // @comm Instances of these exception can be accessed via indexing
        // or via attribute access.  Attribute access is more forwards
        // compatible with Python 3, so is recommended.
        // @comm See also <o error>
        // @tupleitem 0|int|hresult|The COM hresult
        // @tupleitem 1|string|strerror|The error message
        // @tupleitem 2|None/tuple|excepinfo|An optional EXCEPINFO tuple.
        // @tupleitem 3|None/int|argerror|The index of the argument in error, or (usually) None or -1
    }

    /* PyType_Ready *needs* to be called anytime pywintypesXX.dll is loaded, since
        other extension modules can use types defined here without pywintypes itself
        having been imported.
        ??? All extension modules that call this need to be changed to check the exit code ???
    */
    if (PyType_Ready(&PyHANDLEType) == -1 || PyType_Ready(&PyOVERLAPPEDType) == -1 ||
        PyType_Ready(&PyDEVMODEWType) == -1 || PyType_Ready(&PyWAVEFORMATEXType) == -1
#ifndef NO_PYWINTYPES_IID
        || PyType_Ready(&PyIIDType) == -1
#endif  // NO_PYWINTYPES_IID
        || PyType_Ready(&PySECURITY_DESCRIPTORType) == -1 || PyType_Ready(&PySECURITY_ATTRIBUTESType) == -1 ||
        PyType_Ready(&PySIDType) == -1 || PyType_Ready(&PyACLType) == -1)
        return -1;

    if (!_PyWinDateTime_Init())
        return -1;
    return 0;
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

#define ADD_CONSTANT(tok)                                 \
    if (PyModule_AddIntConstant(module, #tok, tok) == -1) \
        PYWIN_MODULE_INIT_RETURN_ERROR;

#define ADD_TYPE(type_name)                                                       \
    if (PyType_Ready(&Py##type_name) == -1 ||                                     \
        PyDict_SetItemString(dict, #type_name, (PyObject *)&Py##type_name) == -1) \
        PYWIN_MODULE_INIT_RETURN_ERROR;

PYWIN_MODULE_INIT_FUNC(pywintypes)
{
    PYWIN_MODULE_INIT_PREPARE(pywintypes, pywintypes_functions,
                              "Module containing common objects and functions used by various Pywin32 modules");

    if (PyWinExc_ApiError == NULL || PyWinExc_COMError == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not initialise the error objects");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }

    if (PyDict_SetItemString(dict, "error", PyWinExc_ApiError) == -1 ||
        PyDict_SetItemString(dict, "com_error", PyWinExc_COMError) == -1 ||
        PyDict_SetItemString(dict, "TRUE", Py_True) == -1 || PyDict_SetItemString(dict, "FALSE", Py_False) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    ADD_CONSTANT(WAVE_FORMAT_PCM);

    if (!_PyWinDateTime_PrepareModuleDict(dict))
        PYWIN_MODULE_INIT_RETURN_ERROR;
#ifndef NO_PYWINTYPES_IID
    ADD_TYPE(IIDType);
#endif  // NO_PYWINTYPES_IID
    ADD_TYPE(SECURITY_DESCRIPTORType);
    ADD_TYPE(SECURITY_ATTRIBUTESType);
    ADD_TYPE(SIDType);
    ADD_TYPE(ACLType);
    ADD_TYPE(HANDLEType);
    ADD_TYPE(OVERLAPPEDType);
    ADD_TYPE(DEVMODEWType);
    if (PyDict_SetItemString(dict, "DEVMODEType", (PyObject *)&PyDEVMODEWType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    ADD_TYPE(WAVEFORMATEXType);

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}

extern "C" __declspec(dllexport) BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    FARPROC fp;
    // dll usually will already be loaded
    HMODULE hmodule = PyWin_GetOrLoadLibraryHandle("advapi32.dll");
    if (hmodule != NULL) {
        fp = GetProcAddress(hmodule, "AddAccessAllowedAce");
        if (fp)
            addaccessallowedace = (addacefunc)(fp);
        fp = GetProcAddress(hmodule, "AddAccessDeniedAce");
        if (fp)
            addaccessdeniedace = (addacefunc)(fp);
        fp = GetProcAddress(hmodule, "AddAccessAllowedAceEx");
        if (fp)
            addaccessallowedaceex = (addaceexfunc)(fp);
        fp = GetProcAddress(hmodule, "AddMandatoryAce");
        if (fp)
            addmandatoryace = (addaceexfunc)(fp);
        fp = GetProcAddress(hmodule, "AddAccessAllowedObjectAce");
        if (fp)
            addaccessallowedobjectace = (addobjectacefunc)(fp);
        fp = GetProcAddress(hmodule, "AddAccessDeniedAceEx");
        if (fp)
            addaccessdeniedaceex = (addaceexfunc)(fp);
        fp = GetProcAddress(hmodule, "AddAccessDeniedObjectAce");
        if (fp)
            addaccessdeniedobjectace = (addobjectacefunc)(fp);
        fp = GetProcAddress(hmodule, "AddAuditAccessAceEx");
        if (fp)
            addauditaccessaceex = (BOOL(WINAPI *)(PACL, DWORD, DWORD, DWORD, PSID, BOOL, BOOL))(fp);
        fp = GetProcAddress(hmodule, "AddAuditAccessObjectAce");
        if (fp)
            addauditaccessobjectace = (BOOL(WINAPI *)(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID, BOOL, BOOL))(fp);
        fp = GetProcAddress(hmodule, "SetSecurityDescriptorControl");
        if (fp)
            setsecuritydescriptorcontrol =
                (BOOL(WINAPI *)(PSECURITY_DESCRIPTOR, SECURITY_DESCRIPTOR_CONTROL, SECURITY_DESCRIPTOR_CONTROL))(fp);
    }

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
        case DLL_PROCESS_DETACH: {
            DeleteCriticalSection(&g_csMain);
            TlsFree(dwTlsIndex);
#ifdef _DEBUG
            if (g_cGlobalLocks) {
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
    return TRUE;  // ok
}

// Function to format a python traceback into a character string.
#define GPEM_ERROR(what)                                      \
    {                                                         \
        errorMsg = L"<Error getting traceback - "##what##">"; \
        goto done;                                            \
    }
PYWINTYPES_EXPORT WCHAR *GetPythonTraceback(PyObject *exc_type, PyObject *exc_value, PyObject *exc_tb)
{
    WCHAR *result = NULL;
    WCHAR *errorMsg = NULL;
    PyObject *modStringIO = NULL;
    PyObject *modTB = NULL;
    PyObject *obFuncStringIO = NULL;
    PyObject *obStringIO = NULL;
    PyObject *obFuncTB = NULL;
    PyObject *argsTB = NULL;
    PyObject *obResult = NULL;
    TmpWCHAR resultPtr;

    // cStringIO is in "io"
    modStringIO = PyImport_ImportModule("io");

    if (modStringIO == NULL)
        GPEM_ERROR("can't import cStringIO");
    modTB = PyImport_ImportModule("traceback");
    if (modTB == NULL)
        GPEM_ERROR("can't import traceback");

    /* Construct a cStringIO object */
    obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
    if (obFuncStringIO == NULL)
        GPEM_ERROR("can't find cStringIO.StringIO");
    obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
    if (obStringIO == NULL)
        GPEM_ERROR("cStringIO.StringIO() failed");

    /* Get the traceback.print_exception function, and call it. */
    obFuncTB = PyObject_GetAttrString(modTB, "print_exception");
    if (obFuncTB == NULL)
        GPEM_ERROR("can't find traceback.print_exception");
    // Py3k has added an undocumented 'chain' argument which defaults to True
    // and causes all kinds of exceptions while trying to print a traceback!
    // This *could* be useful thought if we can tame it - later!
    int chain = 0;

    argsTB = Py_BuildValue("OOOOOi", exc_type ? exc_type : Py_None, exc_value ? exc_value : Py_None,
                           exc_tb ? exc_tb : Py_None,
                           Py_None,  // limit
                           obStringIO, chain);
    if (argsTB == NULL)
        GPEM_ERROR("can't make print_exception arguments");

    obResult = PyObject_CallObject(obFuncTB, argsTB);
    if (obResult == NULL) {
        // Chain parameter when True causes traceback.print_exception to fail, leaving no
        //	way to see what the original problem is, or even what error print_exc raises
        // PyObject *t, *v, *tb;
        // PyErr_Fetch(&t, &v, &tb);
        // PyUnicodeObject *uo=(PyUnicodeObject *)v;
        // DebugBreak();
        GPEM_ERROR("traceback.print_exception() failed");
    }
    /* Now call the getvalue() method in the StringIO instance */
    Py_DECREF(obFuncStringIO);
    obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
    if (obFuncStringIO == NULL)
        GPEM_ERROR("can't find getvalue function");
    Py_DECREF(obResult);
    obResult = PyObject_CallObject(obFuncStringIO, NULL);
    if (obResult == NULL)
        GPEM_ERROR("getvalue() failed.");

    /* And it should be a string all ready to go - duplicate it. */
    if (PyWinObject_AsWCHAR(obResult, &resultPtr, FALSE))
        result = wcsdup(resultPtr);
    else
        GPEM_ERROR("getvalue() did not return a string");

done:
    if (result == NULL && errorMsg != NULL)
        result = wcsdup(errorMsg);
    Py_XDECREF(modStringIO);
    Py_XDECREF(modTB);
    Py_XDECREF(obFuncStringIO);
    Py_XDECREF(obStringIO);
    Py_XDECREF(obFuncTB);
    Py_XDECREF(argsTB);
    Py_XDECREF(obResult);
    return result;
}
