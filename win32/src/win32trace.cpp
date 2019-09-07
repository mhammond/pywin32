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

However, the most we will write at a time is "buffer_size/2" bytes, then we
will have a short, optimized sleep between chunks.

See - I told you the implementation was simple :-)

*/

#include "PyWinTypes.h"
#include "PyWinObjects.h"

const unsigned long BUFFER_SIZE = 0x20000;  // Includes size integer.
const TCHAR *MAP_OBJECT_NAME = _T("Global\\PythonTraceOutputMapping");
const TCHAR *MUTEX_OBJECT_NAME = _T("Global\\PythonTraceOutputMutex");
const TCHAR *EVENT_OBJECT_NAME = _T("Global\\PythonTraceOutputEvent");
const TCHAR *EVENT_EMPTY_OBJECT_NAME = _T("Global\\PythonTraceOutputEmptyEvent");

// Global\\ etc goodness:
// On NT4/9x, 'Global\\' is not understood and will fail.
// On 2k/XP, anyone can create 'global' objects.
// On Vista, you need elavated perms to create global objects - however, once
// it has been created and permissions adjusted, a user with normal
// permissions can open these global objects.
// As a service generally will be able to create global objects, we want a
// non-elevated Python to be capable of automatically using the global space
// if it exists, but coping when it can't create such an object (a local
// one is probably fine in such cases).
// [Why bother?: without the Global namespace, a 'win32traceutil' running in
// a 'Remote Desktop' session would not be able to see output from a
// service - they have different local namespaces]

// This means:
// * We first check to see if the mutex exists in the local namespace.  If it
//   does, it we use that namespace for all objects.
// * We then try and create a mutex in the global namespace - if this works, we also
//   use the global namespace.
// * We then create the mutex in our local namespace and use that for everything.
// (Ack - the above is only true for CreateFileMapping - creating mutexes etc
// works fine)

// This behavior is controlled by a global variable set at mutex creation time.
BOOL use_global_namespace = FALSE;

static const TCHAR *FixupObjectName(const TCHAR *global_name)
{
    if (!use_global_namespace)
        return _tcschr(global_name, '\\') + 1;
    // global prefix is ok.
    return global_name;
}

// no const because of python api, this is the name of the entry
// in the sys module that we store our PyTraceObject pointer
char *TRACEOBJECT_NAME = "__win32traceObject__";

HANDLE hMutex = NULL;
// An auto-reset event so a reader knows when data is avail without polling.
HANDLE hEvent = NULL;
// An auto-reset event so writing large data can know when the buffer has
// been read.
HANDLE hEventEmpty = NULL;

SECURITY_ATTRIBUTES sa;           // Security attributes.
PSECURITY_DESCRIPTOR pSD = NULL;  // Pointer to SD.

class PyTraceObject : public PyObject {
    // do not put virtual
    // methods in this class or we'll break the binary layout
    HANDLE hMapFileRead;   // The handle to the read side of the mem-mapped file
    HANDLE hMapFileWrite;  // The handle to the write side of the mem-mapped file
    void *pMapBaseRead;
    void *pMapBaseWrite;

   public:
    void Initialize();
    BOOL OpenReadMap();
    BOOL CloseReadMap();
    BOOL OpenWriteMap();
    BOOL CloseWriteMap();
    BOOL WriteData(const char *data, unsigned len);
    BOOL ReadData(char **ppResult, int *retSize, int waitMilliseconds);
    int fSoftSpace;
};  // PyTraceObject

static void PyTraceObject_dealloc(PyObject *self) { PyObject_Del(self); }

// In an attempt to allow py2k and py3k readers and writers to work together,
// we assume a 'latin1' encoding for the bytes on the wire.  When pulling the
// bytes off the wire, in py2k we return a string while in py3k we return
// a latin-1 decoded unicode object.
static PyObject *PyTraceObject_write(PyObject *self, PyObject *args)
{
    int len;
    char *data = NULL;
    if (!PyArg_ParseTuple(args, "et#:write", "latin-1", &data, &len))
        return NULL;
    BOOL ok = static_cast<PyTraceObject *>(self)->WriteData(data, len);
    PyMem_Free(data);
    if (!ok)
        return NULL;
    Py_RETURN_NONE;
}

static PyObject *PyTraceObject_read(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":read"))
        return NULL;
    int len;
    char *data;
    BOOL ok = static_cast<PyTraceObject *>(self)->ReadData(&data, &len, 0);
    if (!ok)
        return NULL;
#if (PY_VERSION_HEX < 0x03000000)
    PyObject *result = PyString_FromStringAndSize(data, len);
#else
    PyObject *result = PyUnicode_DecodeLatin1(data, len, "replace");
#endif
    free(data);
    return result;
}

static PyObject *PyTraceObject_blockingread(PyObject *self, PyObject *args)
{
    int milliSeconds = INFINITE;
    if (!PyArg_ParseTuple(args, "|i:blockingread", &milliSeconds))
        return NULL;
    int len;
    char *data;
    BOOL ok = static_cast<PyTraceObject *>(self)->ReadData(&data, &len, milliSeconds);
    if (!ok)
        return NULL;
#if (PY_VERSION_HEX < 0x03000000)
    PyObject *result = PyString_FromStringAndSize(data, len);
#else
    PyObject *result = PyUnicode_DecodeLatin1(data, len, "replace");
#endif
    free(data);
    return result;
}

static PyObject *PyTraceObject_flush(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":flush"))
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *PyTraceObject_isatty(PyObject *self, PyObject *args) { return Py_BuildValue("i", 0); }

static PyMethodDef PyTraceObject_methods[] = {
    {"blockingread", PyTraceObject_blockingread, METH_VARARGS},  // @pymeth blockingread
    {"read", PyTraceObject_read, METH_VARARGS},                  // @pymeth read|
    {"write", PyTraceObject_write, METH_VARARGS},                // @pymeth write|
    {"flush", PyTraceObject_flush,
     METH_VARARGS},  // @pymeth flush|Does nothing, but included to better emulate file semantics.
    {"isatty", PyTraceObject_isatty, METH_VARARGS},  // @pymeth isatty | returns false
    {0, 0},
};  // PyTraceObject_methods

#define OFF(x) offsetof(PyTraceObject, x)

static PyMemberDef PyTraceObject_members[] = {
    {"softspace", T_INT, OFF(fSoftSpace), 0, "flag indicating that a space needs to be printed; used by print"},
    {NULL} /* Sentinel */
};

static PyTypeObject PyTraceObjectType = {
    PYWIN_OBJECT_HEAD "PyTraceObject", sizeof(PyTraceObject), 0,
    // standard methods
    PyTraceObject_dealloc, (printfunc)0,
    0,  // getattr
    0,  // setattr
    0,  // cmp
    0,  // repr
    // type categories
    0, 0, 0,
    // more methods
    (hashfunc)0, 0, 0, PyObject_GenericGetAttr, 0, 0, Py_TPFLAGS_DEFAULT,
    0,  // doc
    0,  // tp_traverse
    0,  // tp_clear
    0,  // tp_richcompare
    0,
    0,  // tp_iter
    0,  // iternext
    PyTraceObject_methods,
    PyTraceObject_members,  // tp_members
    0,                      // tp_getsetlist

};  // PyTraceObjectType

static PyObject *newPyTraceObject()
{
    PyTraceObject *pThis = PyObject_New(PyTraceObject, &PyTraceObjectType);
    pThis->Initialize();
    return pThis;
}

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

BOOL DoOpenMap(HANDLE *pHandle, VOID **ppPtr)
{
    if (*pHandle || *ppPtr) {
        ReturnError("DoOpenMap, already open");
        return FALSE;
    }
    Py_BEGIN_ALLOW_THREADS *pHandle =
        CreateFileMapping((HANDLE)-1, &sa, PAGE_READWRITE, 0, BUFFER_SIZE, FixupObjectName(MAP_OBJECT_NAME));
    Py_END_ALLOW_THREADS if (*pHandle == NULL)
    {
        PyWin_SetAPIError("CreateFileMapping");
        return FALSE;
    }
    Py_BEGIN_ALLOW_THREADS *ppPtr = MapViewOfFile(*pHandle, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    Py_END_ALLOW_THREADS if (*ppPtr == NULL)
    {
        // not allowed to access the interpreter inside
        // Py_BEGIN_ALLOW_THREADS block
        PyWin_SetAPIError("MapViewOfFile");
        CloseHandle(*pHandle);
        return FALSE;
    }
    return TRUE;
}

BOOL DoCloseMap(HANDLE *pHandle, VOID **ppPtr)
{
    if (*ppPtr) {
        UnmapViewOfFile(*ppPtr);
        *ppPtr = NULL;
    }
    if (*pHandle) {
        CloseHandle(*pHandle);
        *pHandle = NULL;
    }
    // I don't think we ever want to close the Mutex or the event
    // they are global so one thread can't decide that.
    //
    // Explanation, there was code that closed the mutex and event
    // here before

    return TRUE;
}

BOOL GetMyMutex()
{
    // Give the mutex 10 seconds before timing out
    if (WaitForSingleObject(hMutex, 10 * 1000) == WAIT_FAILED) {
        // Danger this is currently called without holding the GIL
        PyWin_SetAPIError("WaitForSingleObject", GetLastError());
        return FALSE;
    }
    return TRUE;
}

BOOL ReleaseMyMutex()
{
    if (!ReleaseMutex(hMutex)) {
        // Danger this is currently called without holding the GIL
        PyWin_SetAPIError("ReleaseMutex", GetLastError());
        return FALSE;
    }
    return TRUE;
}

void PyTraceObject::Initialize()
{
    hMapFileRead = NULL;
    hMapFileWrite = NULL;
    pMapBaseRead = NULL;
    pMapBaseWrite = NULL;
    fSoftSpace = 0;
}

BOOL PyTraceObject::WriteData(const char *data, unsigned len)
{
    if (pMapBaseWrite == NULL) {
        ReturnError("The module has not been setup for writing");
        return FALSE;
    }
    BOOL rc = TRUE;
    Py_BEGIN_ALLOW_THREADS const char *data_this = data;
    while (len) {
        unsigned len_this = min(len, BUFFER_SIZE / 2);
        BOOL ok = GetMyMutex();
        if (ok) {
            // must use types with identical size on win32 and win64
            unsigned long *pLen = (unsigned long *)pMapBaseWrite;
            unsigned long sizeLeft = (BUFFER_SIZE - sizeof(unsigned long)) - *pLen;
            // If less than double we need left, wait for it to empty, or .1 sec.
            if (sizeLeft < len_this * 2) {
                ReleaseMyMutex();
                SetEvent(hEvent);
                WaitForSingleObject(hEventEmpty, 100);
                ok = GetMyMutex();
            }
        }
        if (ok) {
            unsigned long *pLen = (unsigned long *)pMapBaseWrite;
            char *buffer = (char *)(((unsigned long *)pMapBaseWrite) + 1);

            unsigned long sizeLeft = (BUFFER_SIZE - sizeof(unsigned long)) - *pLen;
            if (sizeLeft < len_this)
                *pLen = 0;
            memcpy(buffer + (*pLen), data_this, len_this);
            *pLen += len_this;
            rc = ReleaseMyMutex();
            SetEvent(hEvent);
            data_this += len_this;
            len -= len_this;
        }
    }
    Py_END_ALLOW_THREADS return rc;
}

BOOL PyTraceObject::ReadData(char **ppResult, int *retSize, int waitMilliseconds)
{
    if (pMapBaseRead == NULL) {
        ReturnError("The module has not been setup for reading");
        return FALSE;
    }
    if (waitMilliseconds != 0) {
        DWORD rc;
        Py_BEGIN_ALLOW_THREADS rc = WaitForSingleObject(hEvent, waitMilliseconds);
        Py_END_ALLOW_THREADS if (rc == WAIT_FAILED)
        {
            PyWin_SetAPIError("WaitForSingleObject", GetLastError());
            return FALSE;
        }
    }
    BOOL rc = FALSE;
    char *result = NULL;
    Py_BEGIN_ALLOW_THREADS if (GetMyMutex())
    {
        // must use sizes that are identical on win32 and win64
        unsigned long *pLen = (unsigned long *)pMapBaseRead;
        char *buffer = (char *)(((unsigned long *)pMapBaseRead) + 1);

        result = (char *)malloc(*pLen + 1);
        if (result) {
            memcpy(result, buffer, *pLen);
            result[*pLen] = '\0';
            *retSize = *pLen;
            *pLen = 0;
        }
        rc = ReleaseMyMutex();
        SetEvent(hEventEmpty);  // in case anyone wants to optimize waiting.
    }
    Py_END_ALLOW_THREADS if (!rc && result) { free(result); }
    if (rc && result == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating buffer for trace data");
        rc = FALSE;
    }
    if (rc)
        *ppResult = result;
    return rc;
}

BOOL PyTraceObject::OpenReadMap() { return DoOpenMap(&hMapFileRead, &pMapBaseRead); }

BOOL PyTraceObject::OpenWriteMap()
{
    return DoOpenMap(&hMapFileWrite, &pMapBaseWrite);
    ;
}

BOOL PyTraceObject::CloseReadMap() { return DoCloseMap(&hMapFileRead, &pMapBaseRead); }

BOOL PyTraceObject::CloseWriteMap() { return DoCloseMap(&hMapFileWrite, &pMapBaseWrite); }

static PyObject *win32trace_GetTracer(PyObject *, PyObject *)
{
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    Py_XINCREF(traceObject);
    if (traceObject == NULL) {
        traceObject = newPyTraceObject();
        int result = PySys_SetObject(TRACEOBJECT_NAME, traceObject);
        // To do: find out what result means
    }
    return traceObject;
}

static PyObject *win32trace_InitRead(PyObject *self, PyObject *args)
{
    BOOL ok;
    PyObject *traceObject = win32trace_GetTracer(NULL, NULL);
    ok = static_cast<PyTraceObject *>(traceObject)->OpenReadMap();
    Py_DECREF(traceObject);
    if (!ok)
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *win32trace_InitWrite(PyObject *self, PyObject *args)
{
    BOOL ok;
    PyObject *traceObject = win32trace_GetTracer(NULL, NULL);
    ok = static_cast<PyTraceObject *>(traceObject)->OpenWriteMap();
    Py_DECREF(traceObject);
    if (!ok)
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *win32trace_TermRead(PyObject *self, PyObject *args)
{
    BOOL ok;
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        // can't terminate something that you haven't started
        return ReturnError("The module has not been setup for reading");
    }
    Py_BEGIN_ALLOW_THREADS ok = static_cast<PyTraceObject *>(traceObject)->CloseReadMap();
    Py_END_ALLOW_THREADS if (!ok) return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *win32trace_TermWrite(PyObject *self, PyObject *args)
{
    BOOL ok;
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        // can't terminate something that you haven't started
        return ReturnError("The module has not been setup for writing");
    }
    Py_BEGIN_ALLOW_THREADS ok = static_cast<PyTraceObject *>(traceObject)->CloseWriteMap();
    Py_END_ALLOW_THREADS if (!ok) return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *win32trace_write(PyObject *, PyObject *args)
{
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        return ReturnError("The module has not been setup for writing");
    }
    PyObject *method = PyObject_GetAttrString(traceObject, "write");
    if (method == NULL) {
        return NULL;
    }
    PyObject *result = PyObject_CallObject(method, args);
    Py_DECREF(method);
    return result;
}

static PyObject *win32trace_read(PyObject *, PyObject *args)
{
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        return ReturnError("The module has not been setup for reading");
    }
    PyObject *method = PyObject_GetAttrString(traceObject, "read");
    if (method == NULL) {
        return NULL;
    }
    PyObject *result = PyObject_CallObject(method, args);
    Py_DECREF(method);
    return result;
}

static PyObject *win32trace_blockingread(PyObject *, PyObject *args)
{
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        return ReturnError("The module has not been setup for reading");
    }
    PyObject *method = PyObject_GetAttrString(traceObject, "blockingread");
    if (method == NULL) {
        return NULL;
    }
    PyObject *result = PyObject_CallObject(method, args);
    Py_DECREF(method);
    return result;
}

static PyObject *win32trace_setprint(PyObject *self, PyObject *args)
{
    PyObject *traceObject = PySys_GetObject(TRACEOBJECT_NAME);
    if (traceObject == NULL) {
        return ReturnError("The module has not been setup for writing");
    }
    PySys_SetObject("stdout", traceObject);
    PySys_SetObject("stderr", traceObject);
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
    if (hEvent == NULL) {
        // this is a fatal error for this module
        // hEvent is setup at module init time.
        // If the module init doesn't work, then nothing works
        PyErr_SetString(PyExc_ValueError, "There is not handle setup for this module.");
        return NULL;
    }
    // I'd rather return an object
    // but I don't want it to be Closed by the object
    return Py_BuildValue("i", hEvent);
}

/* List of functions exported by this module */
// @object win32trace|A module providing out-of-process tracing capabilities for Python.
static struct PyMethodDef win32trace_functions[] = {
    {"GetTracer", win32trace_GetTracer, METH_NOARGS},  // @pymeth GetTracer
    {"GetHandle", win32trace_GetHandle, 1},            // @pymeth GetHandle|
    {"InitRead", win32trace_InitRead, 1},              // @pymeth InitRead|
    {"InitWrite", win32trace_InitWrite, 1},            // @pymeth InitWrite|
    {"TermRead", win32trace_TermRead, 1},              // @pymeth TermRead|
    {"TermWrite", win32trace_TermWrite, 1},            // @pymeth TermWrite|
    {"write", win32trace_write, 1},                    // @pymeth write|
    {"blockingread", win32trace_blockingread, 1},      // @pymeth blockingread|
    {"read", win32trace_read, 1},                      // @pymeth read|
    {"setprint", win32trace_setprint, 1},              // @pymeth setprint|
    {"flush", win32trace_flush, 1},  // @pymeth flush|Does nothing, but included to better emulate file semantics.
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(win32trace)
{
    PYWIN_MODULE_INIT_PREPARE(
        win32trace, win32trace_functions,
        "Interface to the Windows Console functions for dealing with character-mode applications.");

    if (PyType_Ready(&PyTraceObjectType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "error", PyWinExc_ApiError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Allocate memory for the security descriptor.

    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

    // Initialize the new security descriptor.

    InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

    // Add a NULL descriptor ACL to the security descriptor.
    SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE);

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = TRUE;

    assert(hMutex == NULL);

    // See comments re global namespace above - the problem child is
    // CreateFileMapping - so we temporarily use that just to work out what
    // namespace to use for our objects.

    // is the "Global\" namespace even possible?
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    BOOL global_ok = info.dwMajorVersion > 4;
    if (global_ok) {
        // see comments at top of file - if it exists locally, stick with
        // local - use_global_namespace is still FALSE now, so that is the
        // name we get.
        HANDLE h = CreateFileMapping((HANDLE)-1, &sa, PAGE_READWRITE, 0, BUFFER_SIZE, FixupObjectName(MAP_OBJECT_NAME));
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            // no local one exists - see if we can create it globally - if
            // we can, we go global, else we stick with local.
            use_global_namespace = TRUE;
            HANDLE h2 =
                CreateFileMapping((HANDLE)-1, &sa, PAGE_READWRITE, 0, BUFFER_SIZE, FixupObjectName(MAP_OBJECT_NAME));
            use_global_namespace = h2 != NULL;
            if (h2)
                CloseHandle(h2);
        }
        if (h)
            CloseHandle(h);
    }
    // use_global_namespace is now set and will not change - all objects
    // we use are in the same namespace.

    hMutex = CreateMutex(&sa, FALSE, FixupObjectName(MUTEX_OBJECT_NAME));
    if (hMutex == NULL) {
        PyWin_SetAPIError("CreateMutex");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    assert(hEvent == NULL);
    hEvent = CreateEvent(&sa, FALSE, FALSE, FixupObjectName(EVENT_OBJECT_NAME));
    if (hEvent == NULL) {
        PyWin_SetAPIError("CreateEvent");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    assert(hEventEmpty == NULL);
    hEventEmpty = CreateEvent(&sa, FALSE, FALSE, FixupObjectName(EVENT_EMPTY_OBJECT_NAME));
    if (hEventEmpty == NULL) {
        PyWin_SetAPIError("CreateEvent");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
