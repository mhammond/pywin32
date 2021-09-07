
#ifndef __PYWINTYPES_H__
#define __PYWINTYPES_H__

// If building under a GCC, tweak what we need.
#if defined(__GNUC__) && defined(_POSIX_C_SOURCE)
// python.h complains if _POSIX_C_SOURCE is already defined
#undef _POSIX_C_SOURCE
#endif

// windows rpc.h defines "small" as "char" which breaks Python's accu.h,
// so we undefine it before including python.
#ifdef small
#undef small
#endif

#include "Python.h"
// many many files need python's structmember.h, and its possible people
// #included windows.h before including us...
#ifdef WRITE_RESTRICTED
#undef WRITE_RESTRICTED
#endif
#include "structmember.h"
// and python's structmember.h #defines this, conflicting with windows.h
#ifdef WRITE_RESTRICTED
#undef WRITE_RESTRICTED
#endif
#include "windows.h"
#undef WRITE_RESTRICTED  // stop anyone using the wrong one accidently...

// Helpers for our modules.
// Some macros to help the pywin32 modules co-exist in py2x and py3k.
// Creates and initializes local variables called 'module' and 'dict'.

// Maybe these should all be removed - they existed to help in the py2->3
// transition.
// On one hand: the code would be cleaner if they were all just re-inlined?
// On the other: high confidence everything uses the exact same patterns?
// (Regardless, *some*, eg, PYWIN_MODULE_INIT_RETURN_* should be re-inlined!)

// Use to define the function itself (ie, its name, linkage, params)
#define PYWIN_MODULE_INIT_FUNC(module_name) extern "C" __declspec(dllexport) PyObject *PyInit_##module_name(void)

// If the module needs to early-exit on an error condition.
#define PYWIN_MODULE_INIT_RETURN_ERROR return NULL;

// When the module has successfully initialized.
#define PYWIN_MODULE_INIT_RETURN_SUCCESS return module;

// To setup the module object itself and the module's dictionary.
#define PYWIN_MODULE_INIT_PREPARE(module_name, functions, docstring)                                        \
    PyObject *dict, *module;                                                                                \
    static PyModuleDef module_name##_def = {PyModuleDef_HEAD_INIT, #module_name, docstring, -1, functions}; \
    if (PyWinGlobals_Ensure() == -1)                                                                        \
        return NULL;                                                                                        \
    if (!(module = PyModule_Create(&module_name##_def)))                                                    \
        return NULL;                                                                                        \
    if (!(dict = PyModule_GetDict(module)))                                                                 \
        return NULL;

// Helpers for our types.
// Macro to handle PyObject layout changes in Py3k
#define PYWIN_OBJECT_HEAD PyVarObject_HEAD_INIT(NULL, 0)

/* Attribute names are passed as Unicode in Py3k, so use a macro to
    switch between string and unicode conversion.  This function is not
    documented, but is used extensively in the Python codebase itself,
    so it's reasonable to assume it won't disappear anytime soon.
*/
#define PYWIN_ATTR_CONVERT (char *)_PyUnicode_AsString

typedef Py_ssize_t Py_hash_t;

// This only enables runtime checks in debug builds - so we use
// our own so we can enable it always should we desire...
#define PyWin_SAFE_DOWNCAST Py_SAFE_DOWNCAST

// Lars: for WAVEFORMATEX
#include "mmsystem.h"

#ifdef BUILD_PYWINTYPES
/* We are building pywintypesxx.dll */
#define PYWINTYPES_EXPORT __declspec(dllexport)
#else
/* This module uses pywintypesxx.dll */
#define PYWINTYPES_EXPORT __declspec(dllimport)
#if defined(_MSC_VER)
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "pywintypes_d.lib")
#else
#pragma comment(lib, "pywintypes.lib")
#endif  // DEBUG/_DEBUG
#endif  // _MSC_VER
#endif  // BUILD_PYWINTYPES

// Py3k uses memoryview object in place of buffer, and we don't yet.
extern PYWINTYPES_EXPORT PyObject *PyBuffer_New(Py_ssize_t size);
extern PYWINTYPES_EXPORT PyObject *PyBuffer_FromMemory(void *buf, Py_ssize_t size);

// Formats a python traceback into a character string - result must be free()ed
PYWINTYPES_EXPORT char *GetPythonTraceback(PyObject *exc_type, PyObject *exc_value, PyObject *exc_tb);

#include <tchar.h>
/*
** Error/Exception handling
*/
extern PYWINTYPES_EXPORT PyObject *PyWinExc_ApiError;
// Register a Windows DLL that contains the messages in the specified range.
extern PYWINTYPES_EXPORT BOOL PyWin_RegisterErrorMessageModule(DWORD first, DWORD last, HINSTANCE hmod);
// Get the previously registered hmodule for an error code.
extern PYWINTYPES_EXPORT HINSTANCE PyWin_GetErrorMessageModule(DWORD err);

/* A global function that sets an API style error (ie, (code, fn, errTest)) */
PYWINTYPES_EXPORT PyObject *PyWin_SetAPIError(char *fnName, long err = 0);

/* Basic COM Exception handling.  The main COM exception object
   is actually defined here.  However, the most useful functions
   for raising the exception are still in the COM package.  Therefore,
   you can use the fn below to raise a basic COM exception - no fancy error
   messages available, just the HRESULT.  It will, however, _be_ a COM
   exception, and therefore trappable like any other COM exception
*/
extern PYWINTYPES_EXPORT PyObject *PyWinExc_COMError;
PYWINTYPES_EXPORT PyObject *PyWin_SetBasicCOMError(HRESULT hr);

// Given a PyObject (string, Unicode, etc) create a "BSTR" with the value
PYWINTYPES_EXPORT BOOL PyWinObject_AsBstr(PyObject *stringObject, BSTR *pResult, BOOL bNoneOK = FALSE,
                                          DWORD *pResultLen = NULL);
// And free it when finished.
PYWINTYPES_EXPORT void PyWinObject_FreeBstr(BSTR pResult);

PYWINTYPES_EXPORT PyObject *PyWinObject_FromBstr(const BSTR bstr, BOOL takeOwnership = FALSE);

// Given a string or Unicode object, get WCHAR characters.
PYWINTYPES_EXPORT BOOL PyWinObject_AsWCHAR(PyObject *stringObject, WCHAR **pResult, BOOL bNoneOK = FALSE,
                                           DWORD *pResultLen = NULL);
// And free it when finished.
PYWINTYPES_EXPORT void PyWinObject_FreeWCHAR(WCHAR *pResult);

inline BOOL PyWinObject_AsWCHAR(PyObject *stringObject, unsigned short **pResult, BOOL bNoneOK = FALSE,
                                DWORD *pResultLen = NULL)
{
    return PyWinObject_AsWCHAR(stringObject, (WCHAR **)pResult, bNoneOK, pResultLen);
}
inline void PyWinObject_FreeWCHAR(unsigned short *pResult) { PyWinObject_FreeWCHAR((WCHAR *)pResult); }

// Given a PyObject (string, Unicode, etc) create a "char *" with the value
// if pResultLen != NULL, it will be set to the result size NOT INCLUDING
// TERMINATOR (to be in line with SysStringLen, PyString_*, etc)
PYWINTYPES_EXPORT BOOL PyWinObject_AsString(PyObject *stringObject, char **pResult, BOOL bNoneOK = FALSE,
                                            DWORD *pResultLen = NULL);
// And free it when finished.
PYWINTYPES_EXPORT void PyWinObject_FreeString(char *pResult);
PYWINTYPES_EXPORT void PyWinObject_FreeString(WCHAR *pResult);

// Automatically freed WCHAR that can be used anywhere WCHAR * is required
class TmpWCHAR {
   public:
    WCHAR *tmp;
    TmpWCHAR() { tmp = NULL; }
    TmpWCHAR(WCHAR *t) { tmp = t; }
    WCHAR *operator=(WCHAR *t)
    {
        PyWinObject_FreeWCHAR(tmp);
        tmp = t;
        return t;
    }
    WCHAR **operator&() { return &tmp; }
    boolean operator==(WCHAR *t) { return tmp == t; }
    operator WCHAR *() { return tmp; }
    ~TmpWCHAR() { PyWinObject_FreeWCHAR(tmp); }
};

// For 64-bit python compatibility, convert sequence to tuple and check length fits in a DWORD
PYWINTYPES_EXPORT PyObject *PyWinSequence_Tuple(PyObject *obseq, DWORD *len);

// replacement for PyWinObject_AsReadBuffer and PyWinObject_AsWriteBuffer
class PYWINTYPES_EXPORT PyWinBufferView
{
public:
    PyWinBufferView();
    PyWinBufferView(PyObject *ob, bool bWrite = false, bool bNoneOk = false);
    ~PyWinBufferView();
    bool init(PyObject *ob, bool bWrite = false, bool bNoneOk = false);
    void release();
    bool ok();
    void* ptr();
    DWORD len();
private:
    Py_buffer m_view;

    // don't copy objects and don't use C++ >= 11 -> not implemented private
    // copy ctor and assignment operator
    PyWinBufferView(const PyWinBufferView& src);
    PyWinBufferView& operator=(PyWinBufferView const &);
};

/* ANSI/Unicode Support */
/* If UNICODE defined, will be a BSTR - otherwise a char *
   Either way - PyWinObject_FreeTCHAR() must be called
*/

// Helpers with py3k in mind: the result object is always a "core string"
// object; ie, a string in py2k and unicode in py3k.  Mainly to be used for
// objects that *must* be that type - tp_str slots, __dict__ items, etc. If
// Python doesn't *insist* the result be this type, consider using a function
// that always returns a unicode object (ie, most of the "PyWinObject_From*CHAR"
// functions)
PYWINTYPES_EXPORT PyObject *PyWinCoreString_FromString(const char *str, Py_ssize_t len = (Py_ssize_t)-1);
PYWINTYPES_EXPORT PyObject *PyWinCoreString_FromString(const WCHAR *str, Py_ssize_t len = (Py_ssize_t)-1);

#define PyWinObject_FromWCHAR PyWinObject_FromOLECHAR

// Converts a series of consecutive null terminated strings into a list
PYWINTYPES_EXPORT PyObject *PyWinObject_FromMultipleString(WCHAR *multistring);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromMultipleString(char *multistring);
// Converts a sequence of str/unicode objects into a series of consecutive null-terminated
//	wide character strings with extra terminating null
PYWINTYPES_EXPORT BOOL PyWinObject_AsMultipleString(PyObject *ob, WCHAR **pmultistring, BOOL bNoneOK = TRUE,
                                                    DWORD *chars_returned = NULL);
PYWINTYPES_EXPORT void PyWinObject_FreeMultipleString(WCHAR *pmultistring);

// Converts a sequence of str/unicode objects into a series of consecutive character strings
//	terminated by double null
PYWINTYPES_EXPORT BOOL PyWinObject_AsMultipleString(PyObject *ob, char **pmultistring, BOOL bNoneOK = TRUE,
                                                    DWORD *chars_returned = NULL);
PYWINTYPES_EXPORT void PyWinObject_FreeMultipleString(char *pmultistring);

// Convert a sequence of strings to an array of WCHAR pointers
PYWINTYPES_EXPORT void PyWinObject_FreeWCHARArray(LPWSTR *wchars, DWORD str_cnt);
PYWINTYPES_EXPORT BOOL PyWinObject_AsWCHARArray(PyObject *str_seq, LPWSTR **wchars, DWORD *str_cnt,
                                                BOOL bNoneOK = FALSE);

// Convert a sequence of string or unicode objects to an array of char *
PYWINTYPES_EXPORT void PyWinObject_FreeCharArray(char **pchars, DWORD str_cnt);
PYWINTYPES_EXPORT BOOL PyWinObject_AsCharArray(PyObject *str_seq, char ***pchars, DWORD *str_cnt, BOOL bNoneOK = FALSE);

PYWINTYPES_EXPORT PyObject *PyWinObject_FromOLECHAR(const OLECHAR *str);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromOLECHAR(const OLECHAR *str, int numChars);

// String support for buffers allocated via a function of your choice.
PYWINTYPES_EXPORT BOOL PyWinObject_AsPfnAllocatedWCHAR(PyObject *stringObject, void *(*pfnAllocator)(ULONG),
                                                       WCHAR **ppResult, BOOL bNoneOK = FALSE,
                                                       DWORD *pResultLen = NULL);

#ifdef UNICODE
// XXX - "AsTCHAR" functions should all die - the type of the Python object
// being returned should not depend on UNICODE or not.
#define PyWinObject_AsTCHAR PyWinObject_AsWCHAR
#define PyWinObject_FreeTCHAR PyWinObject_FreeWCHAR
#define PyWinObject_FromTCHAR PyWinObject_FromOLECHAR
#else /* not UNICODE */
#define PyWinObject_AsTCHAR PyWinObject_AsString
#define PyWinObject_FreeTCHAR PyWinObject_FreeString

// PyWinObject_FromTCHAR in a non-unicode build still depends on py3k or not:
// py2x a string object is returned (no conversions).  py3x a unicode object
// is returned (ie, the string is decoded)
PYWINTYPES_EXPORT PyObject *PyWinObject_FromTCHAR(const char *str, Py_ssize_t len = (Py_ssize_t)-1);

#endif  // UNICODE

// String support for buffers allocated via CoTaskMemAlloc and CoTaskMemFree
PYWINTYPES_EXPORT BOOL PyWinObject_AsTaskAllocatedWCHAR(PyObject *stringObject, WCHAR **ppResult, BOOL bNoneOK = FALSE,
                                                        DWORD *pResultLen = NULL);
PYWINTYPES_EXPORT void PyWinObject_FreeTaskAllocatedWCHAR(WCHAR *str);

PYWINTYPES_EXPORT void PyWinObject_FreeString(char *str);
PYWINTYPES_EXPORT void PyWinObject_FreeString(WCHAR *str);

// Copy null terminated string with same allocator as PyWinObject_AsWCHAR, etc
PYWINTYPES_EXPORT WCHAR *PyWin_CopyString(const WCHAR *input);
PYWINTYPES_EXPORT char *PyWin_CopyString(const char *input);

// Pointers.
// Substitute for Python's inconsistent PyLong_AsVoidPtr
PYWINTYPES_EXPORT BOOL PyWinLong_AsVoidPtr(PyObject *ob, void **pptr);
PYWINTYPES_EXPORT PyObject *PyWinLong_FromVoidPtr(const void *ptr);

/*
** LARGE_INTEGER objects
*/
// AsLARGE_INTEGER takes either int or long
PYWINTYPES_EXPORT BOOL PyWinObject_AsLARGE_INTEGER(PyObject *ob, LARGE_INTEGER *pResult);
PYWINTYPES_EXPORT BOOL PyWinObject_AsULARGE_INTEGER(PyObject *ob, ULARGE_INTEGER *pResult);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromLARGE_INTEGER(const LARGE_INTEGER &val);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromULARGE_INTEGER(const ULARGE_INTEGER &val);
// Helpers that take a Py_LONG_LONG, but (a) have pywin32 consistent signatures
// and (b) handle int *and* long (where Python only starts doing that in the
// PyLong_* APIs post 2.4)
// We also happen to know a LARGE_INTEGER is an __int64, so do it the easy way
#define PyWinObject_AsPY_LONG_LONG(ob, pResult) PyWinObject_AsLARGE_INTEGER((ob), (LARGE_INTEGER *)(pResult))
#define PyWinObject_AsUPY_LONG_LONG(ob, pResult) PyWinObject_AsULARGE_INTEGER((ob), (ULARGE_INTEGER *)(pResult))
#define PyWinObject_FromPY_LONG_LONG(val) PyWinObject_FromLARGE_INTEGER((LARGE_INTEGER)val)
#define PyWinObject_FromUPY_LONG_LONG(val) PyWinObject_FromULARGE_INTEGER((ULARGE_INTEGER)val)

// A DWORD_PTR and ULONG_PTR appear to mean "integer long enough to hold a pointer"
// It is *not* actually a pointer (but is the same size as a pointer)
inline PyObject *PyWinObject_FromULONG_PTR(ULONG_PTR v) { return PyWinLong_FromVoidPtr((void *)v); }
inline BOOL PyWinLong_AsULONG_PTR(PyObject *ob, ULONG_PTR *r) { return PyWinLong_AsVoidPtr(ob, (void **)r); }

inline PyObject *PyWinObject_FromDWORD_PTR(DWORD_PTR v) { return PyLong_FromVoidPtr((void *)v); }
inline BOOL PyWinLong_AsDWORD_PTR(PyObject *ob, DWORD_PTR *r) { return PyWinLong_AsVoidPtr(ob, (void **)r); }

/*
** OVERLAPPED Object and API
*/
class PyOVERLAPPED;                                      // forward declare
extern PYWINTYPES_EXPORT PyTypeObject PyOVERLAPPEDType;  // the Type for PyOVERLAPPED
#define PyOVERLAPPED_Check(ob) ((ob)->ob_type == &PyOVERLAPPEDType)
PYWINTYPES_EXPORT BOOL PyWinObject_AsOVERLAPPED(PyObject *ob, OVERLAPPED **ppOverlapped, BOOL bNoneOK = TRUE);
PYWINTYPES_EXPORT BOOL PyWinObject_AsPyOVERLAPPED(PyObject *ob, PyOVERLAPPED **ppOverlapped, BOOL bNoneOK = TRUE);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromOVERLAPPED(const OVERLAPPED *pOverlapped);

// A global function that can work as a module method for making an OVERLAPPED object.
PYWINTYPES_EXPORT PyObject *PyWinMethod_NewOVERLAPPED(PyObject *self, PyObject *args);

#ifndef NO_PYWINTYPES_IID
/*
** IID/GUID support
*/

extern PYWINTYPES_EXPORT PyTypeObject PyIIDType;  // the Type for PyIID
#define PyIID_Check(ob) ((ob)->ob_type == &PyIIDType)

// Given an object repring a CLSID (either PyIID or string), fill the CLSID.
PYWINTYPES_EXPORT BOOL PyWinObject_AsIID(PyObject *obCLSID, CLSID *clsid);

// return a native PyIID object representing an IID
PYWINTYPES_EXPORT PyObject *PyWinObject_FromIID(const IID &riid);

// return a string/Unicode object representing an IID
PYWINTYPES_EXPORT PyObject *PyWinCoreString_FromIID(const IID &riid);

// A global function that can work as a module method for making an IID object.
PYWINTYPES_EXPORT PyObject *PyWinMethod_NewIID(PyObject *self, PyObject *args);
#endif /*NO_PYWINTYPES_IID */

/*
** TIME support
**
** We use a subclass of the builtin datetime.
*/

PYWINTYPES_EXPORT PyObject *PyWinObject_FromSYSTEMTIME(const SYSTEMTIME &t);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromFILETIME(const FILETIME &t);

// Converts a TimeStamp, which is in 100 nanosecond units like a FILETIME
// TimeStamp is actually defined as a LARGE_INTEGER, so this function will also
// accept Windows security "TimeStamp" objects directly - however, we use a
// LARGE_INTEGER prototype to avoid pulling in the windows security headers.
PYWINTYPES_EXPORT PyObject *PyWinObject_FromTimeStamp(const LARGE_INTEGER &t);
PYWINTYPES_EXPORT PyObject *PyWinTimeObject_Fromtime_t(time_t t);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromDATE(DATE t);

PYWINTYPES_EXPORT BOOL PyWinObject_AsDATE(PyObject *ob, DATE *pDate);
PYWINTYPES_EXPORT BOOL PyWinObject_AsFILETIME(PyObject *ob, FILETIME *pDate);
PYWINTYPES_EXPORT BOOL PyWinObject_AsSYSTEMTIME(PyObject *ob, SYSTEMTIME *pDate);

// A global function that can work as a module method for making a time object.
PYWINTYPES_EXPORT PyObject *PyWinMethod_NewTime(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT PyObject *PyWinMethod_NewTimeStamp(PyObject *self, PyObject *args);

PYWINTYPES_EXPORT BOOL PyWinTime_Check(PyObject *ob);

// functions to return WIN32_FIND_DATA tuples, used in shell, win32api, and win32file
PYWINTYPES_EXPORT PyObject *PyObject_FromWIN32_FIND_DATAA(WIN32_FIND_DATAA *pData);
PYWINTYPES_EXPORT PyObject *PyObject_FromWIN32_FIND_DATAW(WIN32_FIND_DATAW *pData);
#ifdef UNICODE
#define PyObject_FromWIN32_FIND_DATA PyObject_FromWIN32_FIND_DATAW
#else
#define PyObject_FromWIN32_FIND_DATA PyObject_FromWIN32_FIND_DATAA
#endif

// POINT tuple, used in win32api_display.cpp and win32gui.i
PYWINTYPES_EXPORT BOOL PyWinObject_AsPOINT(PyObject *obpoint, LPPOINT ppoint);

// IO_COUNTERS dict, used in win32process and win32job
PYWINTYPES_EXPORT PyObject *PyWinObject_FromIO_COUNTERS(PIO_COUNTERS pioc);

// Make an array of DWORD's from a sequence of Python ints
PYWINTYPES_EXPORT BOOL PyWinObject_AsDWORDArray(PyObject *obdwords, DWORD **pdwords, DWORD *item_cnt,
                                                BOOL bNoneOk = TRUE);

// Conversion for resource id/name and class atom
PYWINTYPES_EXPORT BOOL PyWinObject_AsResourceIdA(PyObject *ob, char **presource_id, BOOL bNoneOK = FALSE);
PYWINTYPES_EXPORT BOOL PyWinObject_AsResourceIdW(PyObject *ob, WCHAR **presource_id, BOOL bNoneOK = FALSE);
PYWINTYPES_EXPORT void PyWinObject_FreeResourceId(char *resource_id);
PYWINTYPES_EXPORT void PyWinObject_FreeResourceId(WCHAR *resource_id);
#ifdef UNICODE
#define PyWinObject_AsResourceId PyWinObject_AsResourceIdW
#else
#define PyWinObject_AsResourceId PyWinObject_AsResourceIdA
#endif

// WPARAM and LPARAM conversion
PYWINTYPES_EXPORT BOOL PyWinObject_AsPARAM(PyObject *ob, WPARAM *pparam);
inline PyObject *PyWinObject_FromPARAM(WPARAM param) { return PyWinObject_FromULONG_PTR(param); }
inline BOOL PyWinObject_AsPARAM(PyObject *ob, LPARAM *pparam) { return PyWinObject_AsPARAM(ob, (WPARAM *)pparam); }
inline PyObject *PyWinObject_FromPARAM(LPARAM param) { return PyWinObject_FromULONG_PTR(param); }

// RECT conversions
// @object PyRECT|Tuple of 4 ints defining a rectangle: (left, top, right, bottom)
PYWINTYPES_EXPORT BOOL PyWinObject_AsRECT(PyObject *obrect, LPRECT prect);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromRECT(LPRECT prect);

/*
** SECURITY_ATTRIBUTES support
*/
extern PYWINTYPES_EXPORT PyTypeObject PySECURITY_ATTRIBUTESType;
#define PySECURITY_ATTRIBUTES_Check(ob) ((ob)->ob_type == &PySECURITY_ATTRIBUTESType)
extern PYWINTYPES_EXPORT PyTypeObject PyDEVMODEAType;
extern PYWINTYPES_EXPORT PyTypeObject PyDEVMODEWType;

PYWINTYPES_EXPORT PyObject *PyWinMethod_NewSECURITY_ATTRIBUTES(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT BOOL PyWinObject_AsSECURITY_ATTRIBUTES(PyObject *ob, SECURITY_ATTRIBUTES **ppSECURITY_ATTRIBUTES,
                                                         BOOL bNoneOK = TRUE);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromSECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa);
PYWINTYPES_EXPORT BOOL PyWinObject_AsDEVMODE(PyObject *ob, PDEVMODEA *ppDEVMODE, BOOL bNoneOK = TRUE);
PYWINTYPES_EXPORT BOOL PyWinObject_AsDEVMODE(PyObject *ob, PDEVMODEW *ppDEVMODE, BOOL bNoneOK);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromDEVMODE(PDEVMODEA);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromDEVMODE(PDEVMODEW);

/*
** WAVEFORMATEX support
*/

PYWINTYPES_EXPORT PyObject *PyWinMethod_NewWAVEFORMATEX(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromWAVEFROMATEX(const WAVEFORMATEX &wfx);
PYWINTYPES_EXPORT BOOL PyWinObject_AsWAVEFORMATEX(PyObject *ob, WAVEFORMATEX **ppWAVEFORMATEX, BOOL bNoneOK = TRUE);
extern PYWINTYPES_EXPORT PyTypeObject PyWAVEFORMATEXType;
#define PyWAVEFORMATEX_Check(ob) ((ob)->ob_type == &PyWAVEFORMATEXType)

/*
** SECURITY_DESCRIPTOR support
*/
extern PYWINTYPES_EXPORT PyTypeObject PySECURITY_DESCRIPTORType;
#define PySECURITY_DESCRIPTOR_Check(ob) ((ob)->ob_type == &PySECURITY_DESCRIPTORType)

PYWINTYPES_EXPORT PyObject *PyWinMethod_NewSECURITY_DESCRIPTOR(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT BOOL PyWinObject_AsSECURITY_DESCRIPTOR(PyObject *ob, PSECURITY_DESCRIPTOR *ppSECURITY_DESCRIPTOR,
                                                         BOOL bNoneOK = TRUE);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromSECURITY_DESCRIPTOR(PSECURITY_DESCRIPTOR psd);

PYWINTYPES_EXPORT BOOL _MakeAbsoluteSD(PSECURITY_DESCRIPTOR psd_relative, PSECURITY_DESCRIPTOR *ppsd_absolute);
PYWINTYPES_EXPORT void FreeAbsoluteSD(PSECURITY_DESCRIPTOR psd);

/*
** SID support
*/
extern PYWINTYPES_EXPORT PyTypeObject PySIDType;
#define PySID_Check(ob) ((ob)->ob_type == &PySIDType)

PYWINTYPES_EXPORT PyObject *PyWinMethod_NewSID(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT BOOL PyWinObject_AsSID(PyObject *ob, PSID *ppSID, BOOL bNoneOK = FALSE);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromSID(PSID pSID);

/*
** ACL support
*/
extern PYWINTYPES_EXPORT PyTypeObject PyACLType;
#define PyACL_Check(ob) ((ob)->ob_type == &PyACLType)

PYWINTYPES_EXPORT PyObject *PyWinMethod_NewACL(PyObject *self, PyObject *args);
PYWINTYPES_EXPORT BOOL PyWinObject_AsACL(PyObject *ob, PACL *ppACL, BOOL bNoneOK = FALSE);

/*
** Win32 HANDLE wrapper - any handle closable by "CloseHandle()"
*/
extern PYWINTYPES_EXPORT PyTypeObject PyHANDLEType;  // the Type for PyHANDLE
#define PyHANDLE_Check(ob) ((ob)->ob_type == &PyHANDLEType)

// Convert an object to a HANDLE - None is always OK, as are ints, etc.
PYWINTYPES_EXPORT BOOL PyWinObject_AsHANDLE(PyObject *ob, HANDLE *pRes);
// For handles that use PyHANDLE.
PYWINTYPES_EXPORT PyObject *PyWinObject_FromHANDLE(HANDLE h);
// For handles that aren't returned as PyHANDLE or a subclass thereof (HDC, HWND, etc).
// Return as python ints or longs
PYWINTYPES_EXPORT PyObject *PyWinLong_FromHANDLE(HANDLE h);

// A global function that can work as a module method for making a HANDLE object.
PYWINTYPES_EXPORT PyObject *PyWinMethod_NewHANDLE(PyObject *self, PyObject *args);

// A global function that does the right thing wrt closing a "handle".
// The object can be either a PyHANDLE or an integer.
// If result is FALSE, a Python error is all setup (cf PyHANDLE::Close(), which doesnt set the Python error)
PYWINTYPES_EXPORT BOOL PyWinObject_CloseHANDLE(PyObject *obHandle);

PYWINTYPES_EXPORT BOOL PyWinObject_AsHKEY(PyObject *ob, HKEY *pRes);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromHKEY(HKEY h);
PYWINTYPES_EXPORT BOOL PyWinObject_CloseHKEY(PyObject *obHandle);

// MSG structure keeps coming up...
PYWINTYPES_EXPORT BOOL PyWinObject_AsMSG(PyObject *ob, MSG *pMsg);
PYWINTYPES_EXPORT PyObject *PyWinObject_FromMSG(const MSG *pMsg);

#include "winsock.h"
/*
** SOCKET support.
*/
PYWINTYPES_EXPORT
BOOL PySocket_AsSOCKET
    //-------------------------------------------------------------------------
    // Helper function for dealing with socket arguments.
    (PyObject *obSocket,
     // [in] Python object being converted into a SOCKET handle.
     SOCKET *ps
     // [out] Returned socket handle
    );

/*
** Other Utilities
*/
// ----------------------------------------------------------------------
// WARNING - NEVER EVER USE new() ON THIS CLASS
// This class can be used as a local variable, typically in a Python/C
// function, and can be passed whereever a TCHAR/WCHAR is expected.
// Typical Usage:
// PyWin_AutoFreeBstr arg;
// PyArg_ParseTuple("O", &obStr);
// PyWinObject_AsAutoFreeBstr(obStr, &arg);
// CallTheFunction(arg); // Will correctly pass BSTR/OLECHAR
// -- when the function goes out of scope, the string owned by "arg" will
// -- automatically be freed.
// ----------------------------------------------------------------------
class PYWINTYPES_EXPORT PyWin_AutoFreeBstr {
   public:
    PyWin_AutoFreeBstr(BSTR bstr = NULL);
    ~PyWin_AutoFreeBstr();
    void SetBstr(BSTR bstr);
    operator BSTR() { return m_bstr; }

   private:
    BSTR m_bstr;
};

inline BOOL PyWinObject_AsAutoFreeBstr(PyObject *stringObject, PyWin_AutoFreeBstr *pResult, BOOL bNoneOK = FALSE)
{
    if (bNoneOK && stringObject == Py_None) {
        pResult->SetBstr(NULL);
        return TRUE;
    }
    BSTR bs;
    if (!PyWinObject_AsBstr(stringObject, &bs, bNoneOK))
        return FALSE;
    pResult->SetBstr(bs);
    return TRUE;
}

// ----------------------------------------------------------------------
//
// THREAD MANAGEMENT
//

// ### need to rename the PYCOM_ stuff soon...

// We have 2 discrete locks in use (when no free-threaded is used, anyway).
// The first type of lock is the global Python lock.  This is the standard lock
// in use by Python, and must be used as documented by Python.  Specifically, no
// 2 threads may _ever_ call _any_ Python code (including INCREF/DECREF) without
// first having this thread lock.
//
// The second type of lock is a "global framework lock".  This lock is simply a
// critical section, and used whenever 2 threads of C code need access to global
// data.  This is different than the Python lock - this lock is used when no Python
// code can ever be called by the threads, but the C code still needs thread-safety.

// We also supply helper classes which make the usage of these locks a one-liner.

// The "framework" lock, implemented as a critical section.
PYWINTYPES_EXPORT void PyWin_AcquireGlobalLock(void);
PYWINTYPES_EXPORT void PyWin_ReleaseGlobalLock(void);

// Helper class for the DLL global lock.
//
// This class magically waits for the Win32/COM framework global lock, and releases it
// when finished.
// NEVER new one of these objects - only use on the stack!
class CEnterLeaveFramework {
   public:
    CEnterLeaveFramework() { PyWin_AcquireGlobalLock(); }
    ~CEnterLeaveFramework() { PyWin_ReleaseGlobalLock(); }
};

// Python thread-lock stuff.  Free-threading patches use different semantics, but
// these are abstracted away here...
#ifndef FORCE_NO_FREE_THREAD
#ifdef WITH_FREE_THREAD
#define PYCOM_USE_FREE_THREAD
#endif
#endif
#ifdef PYCOM_USE_FREE_THREAD
#include <threadstate.h>
#else
#include <pystate.h>
#endif

// Helper class for Enter/Leave Python
//
// This class magically waits for the Python global lock, and releases it
// when finished.

// Nested invocations will deadlock, so be careful.

// NEVER new one of these objects - only use on the stack!
#ifndef PYCOM_USE_FREE_THREAD
extern PYWINTYPES_EXPORT PyInterpreterState *PyWin_InterpreterState;
extern PYWINTYPES_EXPORT BOOL PyWinThreadState_Ensure();
extern PYWINTYPES_EXPORT void PyWinThreadState_Free();
extern PYWINTYPES_EXPORT void PyWinThreadState_Clear();
extern PYWINTYPES_EXPORT void PyWinInterpreterLock_Acquire();
extern PYWINTYPES_EXPORT void PyWinInterpreterLock_Release();

extern PYWINTYPES_EXPORT int PyWinGlobals_Ensure();
extern PYWINTYPES_EXPORT void PyWinGlobals_Free();
#else
#define PyWinThreadState_Ensure PyThreadState_Ensure
#define PyWinThreadState_Free PyThreadState_Free
#define PyWinThreadState_Clear PyThreadState_ClearExc

#endif

extern PYWINTYPES_EXPORT void PyWin_MakePendingCalls();

class CEnterLeavePython {
   public:
    CEnterLeavePython() : released(TRUE) { acquire(); }
    void acquire(void)
    {
        if (!released)
            return;
        state = PyGILState_Ensure();
        released = FALSE;
    }
    ~CEnterLeavePython() { release(); }
    void release(void)
    {
        if (!released) {
            PyGILState_Release(state);
            released = TRUE;
        }
    }

   private:
    PyGILState_STATE state;
    BOOL released;
};

// A helper for simple exception handling.
// try/__try
#if defined(__MINGW32__) || defined(MAINWIN)
#define PYWINTYPES_TRY try
#else
#define PYWINTYPES_TRY __try
#endif /* MAINWIN */

// catch/__except
#if defined(__MINGW32__) || defined(MAINWIN)
#define PYWINTYPES_EXCEPT catch (...)
#else
#define PYWINTYPES_EXCEPT __except (EXCEPTION_EXECUTE_HANDLER)
#endif
// End of exception helper macros.

// Class to hold a temporary reference that decrements itself
class TmpPyObject {
   public:
    PyObject *tmp;
    TmpPyObject() { tmp = NULL; }
    TmpPyObject(PyObject *ob) { tmp = ob; }
    PyObject *operator=(PyObject *ob)
    {
        Py_XDECREF(tmp);
        tmp = ob;
        return tmp;
    }

    boolean operator==(PyObject *ob) { return tmp == ob; }
    operator PyObject *() { return tmp; }
    ~TmpPyObject() { Py_XDECREF(tmp); }
};

#endif  // __PYWINTYPES_H__
