// This file contains some hacks for supporting LARGE_INTEGERS.

// The basic strategy is thus:

// @doc

// @object LARGE_INTEGER|A Python object used wherever a COM LARGE_INTEGER is used.
// @comm Please see <om pywintypes.ULARGE_INTEGER> for a description.

// @object ULARGE_INTEGER|A Python object used wherever a COM ULARGE_INTEGER is used.

// @comm When passed into a Python function, this will always be a single object.
// It will either be an integer, or a long integer, depending on the size.

#include "PyWinTypes.h"

#ifdef __MINGW32__
#define __int64 long long
#endif

BOOL PyWinObject_AsLARGE_INTEGER(PyObject *ob, LARGE_INTEGER *pResult)
{
    if (PyLong_Check(ob)) {
        pResult->QuadPart = PyLong_AsLongLong(ob);
        return !(pResult->QuadPart == -1 && PyErr_Occurred());
    }
    else {
        PyErr_Warn(PyExc_DeprecationWarning,
                   "Support for passing 2 integers to create a 64bit value is deprecated - pass a long instead");
        long hiVal, loVal;
        if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
            PyErr_SetString(PyExc_TypeError, "LARGE_INTEGER must be 'int', or '(int, int)'");
            return FALSE;
        }
        // ### what to do about a "negative" loVal?!
        pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
        return TRUE;
    }
    assert(0);  // not reached.
}

BOOL PyWinObject_AsULARGE_INTEGER(PyObject *ob, ULARGE_INTEGER *pResult)
{
    if (PyLong_Check(ob)) {
        pResult->QuadPart = PyLong_AsUnsignedLongLong(ob);
        return !(pResult->QuadPart == (ULONGLONG)-1 && PyErr_Occurred());
    }
    long hiVal, loVal;
    if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
        PyErr_SetString(PyExc_TypeError, "ULARGE_INTEGER must be 'int', or '(int, int)'");
        return FALSE;
    }
    PyErr_Warn(PyExc_DeprecationWarning,
               "Support for passing 2 integers to create a 64bit value is deprecated - pass a long instead");
    pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
    return TRUE;
}

PYWINTYPES_EXPORT PyObject *PyWinObject_FromLARGE_INTEGER(const LARGE_INTEGER &val)
{
    return PyLong_FromLongLong(val.QuadPart);
}

PYWINTYPES_EXPORT PyObject *PyWinObject_FromULARGE_INTEGER(const ULARGE_INTEGER &val)
{
    return PyLong_FromUnsignedLongLong(val.QuadPart);
}
