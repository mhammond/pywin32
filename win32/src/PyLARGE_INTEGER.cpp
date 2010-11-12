// This file contains some hacks for supporting LARGE_INTEGERS.

// The basic strategy is thus:

// @doc

// @object LARGE_INTEGER|A Python object used wherever a COM LARGE_INTEGER is used.
// @comm Please see <om pywintypes.ULARGE_INTEGER> for a description.

// @object ULARGE_INTEGER|A Python object used wherever a COM ULARGE_INTEGER is used.

// @comm When passed into a Python function, this will always be a single object.
// It will either be an integer, or a long integer, depending on the size.

#include "PyWinTypes.h"
#include "longintrepr.h"

#ifdef __MINGW32__
#define __int64 long long
#endif

BOOL PyWinObject_AsLARGE_INTEGER(PyObject *ob, LARGE_INTEGER *pResult)
{
#if (PY_VERSION_HEX < 0x03000000)
	if (PyInt_Check(ob)) {
		// 32 bit integer value.
		int x = PyInt_AS_LONG(ob);
		if (x==(int)-1 && PyErr_Occurred())
			return FALSE;
		LISet32(*pResult, x);
		return TRUE;
	} else 
#endif
	if (PyLong_Check(ob)) {
		pResult->QuadPart=PyLong_AsLongLong(ob);
		return !(pResult->QuadPart == -1 && PyErr_Occurred());
	} else {
		PyErr_Warn(PyExc_PendingDeprecationWarning, "Support for passing 2 integers to create a 64bit value is deprecated - pass a long instead");
		long hiVal, loVal;
		if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
			PyErr_SetString(PyExc_TypeError, "LARGE_INTEGER must be 'int', or '(int, int)'");
			return FALSE;
		}
		// ### what to do about a "negative" loVal?!
		pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
		return TRUE;
	}
	assert(0); // not reached.
}

BOOL PyWinObject_AsULARGE_INTEGER(PyObject *ob, ULARGE_INTEGER *pResult)
{
#if (PY_VERSION_HEX < 0x03000000)
	// py2k - ints and longs are different, and we assume 'int' is 32bits.
	if (PyInt_Check(ob)) {
		// 32 bit integer value.
		int x = PyInt_AS_LONG(ob);
		if (x==(int)-1 && PyErr_Occurred())
			return FALSE;
		// ### what to do with "negative" integers?  Nothing - they
		// get treated as unsigned!
		ULISet32(*pResult, x);
		return TRUE;
	}
#endif // py2k
	if (PyLong_Check(ob)) {
		pResult->QuadPart=PyLong_AsUnsignedLongLong(ob);
		return !(pResult->QuadPart == (ULONGLONG) -1 && PyErr_Occurred());
	}
	long hiVal, loVal;
	if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
		PyErr_SetString(PyExc_TypeError, "ULARGE_INTEGER must be 'int', or '(int, int)'");
		return FALSE;
	}
	PyErr_Warn(PyExc_PendingDeprecationWarning, "Support for passing 2 integers to create a 64bit value is deprecated - pass a long instead");
	pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
	return TRUE;
}

PyObject *PyWinObject_FromLARGE_INTEGER(LARGE_INTEGER &val)
{
	return PyLong_FromLongLong(val.QuadPart);
}

PyObject *PyWinObject_FromULARGE_INTEGER(ULARGE_INTEGER &val)
{
	return PyLong_FromUnsignedLongLong(val.QuadPart);
}
