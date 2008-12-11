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
	pResult->QuadPart=PyLong_AsLongLong(ob);
	return !(pResult->QuadPart == -1 && PyErr_Occurred());
}

BOOL PyWinObject_AsULARGE_INTEGER(PyObject *ob, ULARGE_INTEGER *pResult)
{
	pResult->QuadPart=PyLong_AsUnsignedLongLong(ob);
	return !(pResult->QuadPart == (ULONGLONG) -1 && PyErr_Occurred());
}

PyObject *PyWinObject_FromLARGE_INTEGER(LARGE_INTEGER &val)
{
	return PyLong_FromLongLong(val.QuadPart);
}

PyObject *PyWinObject_FromULARGE_INTEGER(ULARGE_INTEGER &val)
{
	return PyLong_FromUnsignedLongLong(val.QuadPart);
}
