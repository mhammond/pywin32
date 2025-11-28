/* File : pywin32.i

The start of an interface file for SWIG and the Win32 Python extensions.

Much of the support in here requires the extension linking with
"PyWinTypes.lib", as core Win32 Python object support lives in PyWinTypes.dll

Not complete, but a pretty good start!

Maintained by MHammond@skippinet.com.au, and my version is almost guaranteed later
than the one you are looking at - please forward any changes on, and I'll send you
a new integrated one!
*/

typedef char * LPCSTR;
%apply char * {LPCSTR};

typedef const char * LPCTSTR;
%apply const char * {LPCTSTR};

typedef unsigned short WORD;
%apply unsigned short {WORD};

typedef unsigned long DWORD;
%apply unsigned long {DWORD};

typedef int BOOL;
%apply int {BOOL};

typedef long LONG;
%apply long {LONG};

typedef unsigned long ULONG;
%apply unsigned long {ULONG};


%{
#define PyInt_FromLong PyLong_FromLong // py3k pain.

#include "PyWinTypes.h"
#ifdef NEED_PYWINOBJECTS_H
#include "PyWinObjects.h"
#endif
#include "tchar.h"
%}

// DWORDs can use longs so long as they fit in 32 unsigned bits
%typemap(in) DWORD {
	// PyLong_AsUnsignedLongMask isn't ideal - no overflow checking - but
	// this is what the 'k' format specifier in PyArg_ParseTuple uses, and
	// that is what much of pywin32 uses for DWORDS, so we use it here too
	$1 = PyLong_AsUnsignedLongMask($input);
	if ($1==(DWORD)-1 && PyErr_Occurred())
		return NULL;
}

// Override the SWIG default for this.
%typemap(out) PyObject *{
	if ($1==NULL) return NULL; // get out now!
	$result = $1;
}

//
// Map API functions that return BOOL to
// functions that return None, but raise exceptions.
// These functions must set the win32 LastError.
%{
typedef BOOL BOOLAPI;
%}

%typemap(out) BOOLAPI {
	$result = Py_None;
	Py_INCREF(Py_None);
}

%exception BOOLAPI {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (!$1)  {
           $cleanup
           return PyWin_SetAPIError("$name");
      }
}

%{
typedef DWORD DWORDAPI;
%}

%typemap(out) DWORDAPI {
	$result = Py_None;
	Py_INCREF(Py_None);
}

%exception DWORDAPI {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($1!=0)  {
           $cleanup
           return PyWin_SetAPIError("$name", $1);
      }
}

// String support
%typemap(in) char *inNullString {
	if ($input==Py_None) {
		$1 = NULL;
	} else if (PyBytes_Check($input)) {
		$1 = PyBytes_AsString($input);
	} else {
		PyErr_SetString(PyExc_TypeError, "Argument must be None or a string");
		return NULL;
	}
}

%typemap(in) TCHAR * {
	if (!PyWinObject_AsTCHAR($input, &$1, FALSE))
		return NULL;
}

%typemap(arginit) TCHAR *,OLECHAR *, WCHAR *
{
	$1 = NULL;
}

%typemap(in) TCHAR *inNullString{
	if (!PyWinObject_AsTCHAR($input, &$1, TRUE))
		return NULL;
}
%typemap(in) TCHAR *INPUT_NULLOK = TCHAR *inNullString;

%typemap(freearg) TCHAR *{
	PyWinObject_FreeTCHAR($1);
}

// Delete this!
%typemap(freearg) TCHAR *inNullWideString {
	PyWinObject_FreeTCHAR($1);
}

%typemap(freearg) TCHAR *inNullString {
	PyWinObject_FreeTCHAR($1);
}


%typemap(in) OLECHAR *, WCHAR *{
	// Wide string code!
	if (!PyWinObject_AsWCHAR($input, &$1, FALSE))
		return NULL;
}

%typemap(in) OLECHAR *inNullWideString,
                    WCHAR *inNullWideString {
	// Wide string code!
	if (!PyWinObject_AsWCHAR($input, &$1, TRUE))
		return NULL;
}

%typemap(in) WCHAR *inNullWideString = OLECHAR *inNullWideString;
%typemap(in) WCHAR *INPUT_NULLOK = WCHAR *inNullWideString;

%typemap(freearg) OLECHAR *, WCHAR *{
	// Wide string cleanup
	PyWinObject_FreeWCHAR($1);
}

%typemap(in,numinputs=0) BSTR *OUTPUT (BSTR temp) {
	$1 = &temp;
}

%typemap(argout) BSTR *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromBstr(*$1, TRUE);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}


// An object that can be used in place of a BSTR, but guarantees
// cleanup of the string.
%typemap(in) PyWin_AutoFreeBstr inWideString {
	// Auto-free Wide string code!
	if (!PyWinObject_AsAutoFreeBstr($input, &$1, FALSE))
		return NULL;
}

%typemap(in) PyWin_AutoFreeBstr inNullWideString {
	// Auto-free Wide string code!
	if (!PyWinObject_AsAutoFreeBstr($input, &$1, TRUE))
		return NULL;
}

%typemap(in) OVERLAPPED *
{
	if (!PyWinObject_AsOVERLAPPED($input, &$1, TRUE))
		return NULL;
}
%typemap(argout) OVERLAPPED *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromOVERLAPPED(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}
%typemap(in,numinputs=0) OVERLAPPED *OUTPUT(OVERLAPPED temp)
{
  $1 = &temp;
}

%typemap(argout) OVERLAPPED **OUTPUT {
    PyObject *o;
    o = PyWinObject_FromOVERLAPPED(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}
%typemap(in,numinputs=0) OVERLAPPED **OUTPUT(OVERLAPPED *temp)
{
  $1 = &temp;
}



%typemap(in) SECURITY_ATTRIBUTES *{
	if (!PyWinObject_AsSECURITY_ATTRIBUTES($input, &$1))
		return NULL;
}
//---------------------------------------------------------------------------
//
// HANDLE support
//
// PyHANDLE will use a PyHANDLE object.
// PyHKEY will use a PyHKEY object
// HANDLE, HWND will use an integer.
//---------------------------------------------------------------------------
//typedef void *HANDLE;

%typemap(in,numinputs=0) HANDLE *OUTPUT(HANDLE temp)
{
  $1 = &temp;
}

%exception PyHANDLE {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($1==0 || $1==INVALID_HANDLE_VALUE)  {
           $cleanup
           return PyWin_SetAPIError("$name");
      }
}

%exception PyHKEY {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($1==0 || $1==INVALID_HANDLE_VALUE)  {
           $cleanup
           return PyWin_SetAPIError("$name");
      }
}

%exception HANDLE {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($1==0 || $1==INVALID_HANDLE_VALUE)  {
           $cleanup
           return PyWin_SetAPIError("$name");
      }
}

typedef long HANDLE;
typedef HANDLE PyHANDLE;
%{
#define PyHANDLE HANDLE // Use a #define so we can undef it later if we need the true defn.
//typedef HANDLE PyHKEY;
%}

%typemap(in) HANDLE {
	if (!PyWinObject_AsHANDLE($input, &$1))
		return NULL;
}

%typemap(in) PyHANDLE {
	if (!PyWinObject_AsHANDLE($input, &$1))
		return NULL;
}
%typemap(in) PyHKEY {
	if (!PyWinObject_AsHKEY($input, &$1))
		return NULL;
}

%typemap(in) PyHANDLE INPUT_NULLOK {
	if (!PyWinObject_AsHANDLE($input, &$1))
		return NULL;
}
%typemap(in) PyHKEY INPUT_NULLOK {
	if (!PyWinObject_AsHKEY($input, &$1))
		return NULL;
}

%typemap(in,numinputs=0) PyHANDLE *OUTPUT(HANDLE handle_output)
{
  $1 = &handle_output;
}
%typemap(in,numinputs=0) PyHKEY *OUTPUT(HKEY hkey_output)
{
  $1 = &hkey_output;
}

%typemap(out) PyHANDLE {
  $result = PyWinObject_FromHANDLE($1);
}
%typemap(out) PyHKEY {
  $result = PyWinObject_FromHKEY($1);
}
%typemap(out) HANDLE {
  $result = PyWinLong_FromHANDLE($1);
}

%typemap(argout) PyHANDLE *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromHANDLE(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}
%typemap(argout) PyHKEY *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromHKEY(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}

// HWND (used in win32process, adsi, win32inet, win32crypt)
// Has to be typedef'ed to a non-pointer type or the typemaps are ignored
typedef float HWND;
%typemap(in) HWND{
	if (!PyWinObject_AsHANDLE($input, (HANDLE *)&$1))
		return NULL;
}
%typemap(out) HWND{
	$result=PyWinLong_FromHANDLE($1);
}

%typemap(out) HDESK {
    $result = PyWinLong_FromHANDLE($1);
}

//---------------------------------------------------------------------------
//
// LARGE_INTEGER support
//
//---------------------------------------------------------------------------
%typemap(in) LARGE_INTEGER {
	if (!PyWinObject_AsLARGE_INTEGER($input, &$1))
		return NULL;
}
%typemap(in) LARGE_INTEGER * (LARGE_INTEGER temp) {
	$1 = &temp;
	if (!PyWinObject_AsLARGE_INTEGER($input, $1))
		return NULL;
}
%typemap(in) ULARGE_INTEGER {
	if (!PyWinObject_AsULARGE_INTEGER($input, &$1))
		return NULL;
}
%typemap(in) ULARGE_INTEGER * (ULARGE_INTEGER temp) {
	$1 = &temp;
	if (!PyWinObject_AsULARGE_INTEGER($input, $1))
		return NULL;
}

%typemap(in,numinputs=0) LARGE_INTEGER *OUTPUT(LARGE_INTEGER temp)
{
  $1 = &temp;
}
%typemap(in,numinputs=0) ULARGE_INTEGER *OUTPUT(ULARGE_INTEGER temp)
{
  $1 = &temp;
}

%typemap(out) LARGE_INTEGER {
  $result = PyWinObject_FromLARGE_INTEGER($1);
}
%typemap(out) ULARGE_INTEGER {
  $result = PyWinObject_FromULARGE_INTEGER($1);
}

%typemap(argout) LARGE_INTEGER *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromLARGE_INTEGER(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}
%typemap(argout) ULARGE_INTEGER *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromULARGE_INTEGER(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}

//--------------------------------------------------------------------------
//
// ULONG_PTR
//
//--------------------------------------------------------------------------
%typemap(in) ULONG_PTR
{
	if (!PyWinLong_AsULONG_PTR($input, &$1))
		return NULL;
}
%typemap(in) ULONG_PTR * (ULONG_PTR temp)
{
	$1 = &temp;
	if (!PyWinLong_AsULONG_PTR($input, $1))
		return NULL;
}
%typemap(in,numinputs=0) ULONG_PTR *OUTPUT(ULONG_PTR temp)
{
	$1 = &temp;
}
%typemap(out) ULONG_PTR
{
	$result = PyWinObject_FromULONG_PTR($1)
}
%typemap(argout) ULONG_PTR *OUTPUT {
	PyObject *o;
	o = PyWinObject_FromULONG_PTR(*$1);
	if (!$result) {
		$result = o;
	} else if ($result == Py_None) {
		Py_DECREF(Py_None);
		$result = o;
	} else {
		if (!PyList_Check($result)) {
			PyObject *o2 = $result;
			$result = PyList_New(0);
			PyList_Append($result,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($result,o);
		Py_XDECREF(o);
	}
}

//---------------------------------------------------------------------------
//
// TIME
//
//---------------------------------------------------------------------------
%typemap(in) FILETIME * {
	if (!PyWinObject_AsFILETIME($input, $1, FALSE))
		return NULL;
}
%typemap(in,numinputs=0) FILETIME *(FILETIME temp)
{
  $1 = &temp;
}

%typemap(out) FILETIME * {
  $result = PyWinObject_FromFILETIME($1);
}

%typemap(argout) FILETIME *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromFILETIME(*$1);
    if (!$result) {
      $result = o;
    } else if ($result == Py_None) {
      Py_DECREF(Py_None);
      $result = o;
    } else {
      if (!PyList_Check($result)) {
	PyObject *o2 = $result;
	$result = PyList_New(0);
	PyList_Append($result,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($result,o);
      Py_XDECREF(o);
    }
}

//---------------------------------------------------------------------------
//
// SOCKET support.
//
//---------------------------------------------------------------------------
%typemap(in) SOCKET *(SOCKET sockettemp)
{
	$1 = &sockettemp;
	if (!PySocket_AsSOCKET($input, $1))
	{
		return NULL;
	}
}


//---------------------------------------------------------------------------
//
// Module initialization
//
//---------------------------------------------------------------------------
%init %{
#ifndef SWIG_PYTHONCOM
/* This code only valid if non COM SWIG builds */
#ifndef PYCOM_EXPORT
	 PyDict_SetItemString(d,"UNICODE", PyLong_FromLong(1));
#endif
  PyWinGlobals_Ensure();
  PyDict_SetItemString(d, "error", PyWinExc_ApiError);
#endif SWIG_PYTHONCOM
%}
