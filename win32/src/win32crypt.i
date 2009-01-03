/* File : win32crypt.i */

%module win32crypt // An interface to the win32 Cryptography API

%{
#include "PyWinTypes.h"
#include "wincrypt.h"
#include "malloc.h"
#include "abstract.h" // for PyObject_AsReadBuffer
%}

%include "typemaps.i"
%include "pywin32.i"

%{
BOOL PyWinObject_AsDATA_BLOB(PyObject *ob, DATA_BLOB *b)
{
    Py_ssize_t cb;
    if (PyObject_AsReadBuffer(ob, (const void **)(&b->pbData), &cb)!=0)
        return FALSE;
    b->cbData = PyWin_SAFE_DOWNCAST(cb, Py_ssize_t, int);
    return TRUE;
}

PyObject *PyWinObject_FromDATA_BLOB(DATA_BLOB *b)
{
    return PyString_FromStringAndSize((char *)b->pbData, b->cbData);
}
%}

%typemap(python,argout) WCHAR **CUD_OUTPUT {
    PyObject *o;
    o = PyWinObject_FromWCHAR(*$source);
    // Output string must be LocalFree'd
    if (*$source)
        LocalFree(*$source);
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
%typemap(python,ignore) WCHAR **CUD_OUTPUT(WCHAR *temp)
{
  temp = NULL;
  $target = &temp;
}


%typemap(python,in) DATA_BLOB *
{
    $target = (DATA_BLOB *)_alloca(sizeof(DATA_BLOB));
	if (!PyWinObject_AsDATA_BLOB($source, $target))
		return NULL;
}

%typemap(python,in) DATA_BLOB *INPUT_NULLOK
{
    if ($source==Py_None)
        $target = NULL;
    else {
        $target = (DATA_BLOB *)_alloca(sizeof(DATA_BLOB));
        if (!PyWinObject_AsDATA_BLOB($source, $target))
            return NULL;
    }
}

%typemap(python,argout) DATA_BLOB *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromDATA_BLOB($source);
    // Output DATA_BLOB data pointers must be LocalFree'd
    if ($source->pbData)
        LocalFree($source->pbData);
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
%typemap(python,ignore) DATA_BLOB *OUTPUT(DATA_BLOB temp)
{
  temp.cbData = 0;
  temp.pbData = NULL;
  $target = &temp;
}

// @object PyCRYPTPROTECT_PROMPTSTRUCT|A tuple representing a CRYPTPROTECT_PROMPTSTRUCT structure
// @tupleitem 0|int|flags|
// @tupleitem 1|int|hwndApp|parent hwnd (default is 0)
// @tupleitem 2|<o PyUnicode>|prompt|A prompt string (default is None)
%typemap(python,in) CRYPTPROTECT_PROMPTSTRUCT *
{
    if ($source == Py_None)
        $target = NULL;
    else if (PyTuple_Check($source)) {
        $target = (CRYPTPROTECT_PROMPTSTRUCT *)_alloca(sizeof(CRYPTPROTECT_PROMPTSTRUCT));
        PyObject *obPrompt = Py_None, *obhwndApp = Py_None;
        memset($target, 0, sizeof(*$target));
        $target->cbSize = sizeof(*$target);
        if (!PyArg_ParseTuple($source, "l|OO", &$target->dwPromptFlags,
                                              &obhwndApp,
                                              &obPrompt))
            return NULL;
        if (!PyWinObject_AsWCHAR(obPrompt, (WCHAR **)(&$target->szPrompt), TRUE))
            return NULL;
		if (!PyWinObject_AsHANDLE(obhwndApp, (HANDLE *)&$target->hwndApp))
			return NULL;
    } else {
        PyErr_Format(PyExc_TypeError, "CRYPTPROTECT_PROMPTSTRUCT must be None or a tuple (got %s)",
                    $source->ob_type->tp_name);
        return NULL;
    }
}

%typemap(python,freearg) CRYPTPROTECT_PROMPTSTRUCT {
	PyWinObject_FreeTCHAR($target->szPrompt);
}

%typedef void *NULL_ONLY

%typemap(python,in) NULL_ONLY {
	if ($source != Py_None) {
		PyErr_SetString(PyExc_TypeError, "This param must be None");
		return NULL;
	}
	$target = NULL;
}

// @pyswig string|CryptProtectData|
BOOLAPI CryptProtectData(
  DATA_BLOB* pDataIn, // @pyparm buffer|data||Data to be encrypted.
  WCHAR *szDataDescr, // @pyparm <o PyUnicode>|description||Description to add to the data
  DATA_BLOB* INPUT_NULLOK, // @pyparm buffer|optionalEntropy||
  NULL_ONLY pvReserved, // @pyparm None|reserved||Must be None
  CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct, //@pyparm <o PyCRYPTPROTECT_PROMPTSTRUCT>/None|promptStruct||
  DWORD dwFlags, //@pyparm int|flags||
  DATA_BLOB* OUTPUT
);

// @pyswig (string, string)|CryptUnprotectData|
// @rdesc The result is a tuple of (description, data) where description
// is the description that was passed to <om win32crypt.CryptProtectData>, and
// data is the unencrypted data.
BOOLAPI CryptUnprotectData(
  DATA_BLOB* pDataIn, // buffer|data||The data to unprotect
  WCHAR **CUD_OUTPUT,
  DATA_BLOB* INPUT_NULLOK, // @pyparm buffer|optionalEntropy||The entropy passed to CryptProtectData
  NULL_ONLY pvReserved, // @pyparm None|reserved||Must be None
  CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,//@pyparm <o PyCRYPTPROTECT_PROMPTSTRUCT>/None|promptStruct||
  DWORD dwFlags, //@pyparm int|flags||
  DATA_BLOB* OUTPUT
);

%init %{
    PyEval_InitThreads(); /* Start the interpreter's thread-awareness */
    PyDict_SetItemString(d, "error", PyWinExc_ApiError);
%}
