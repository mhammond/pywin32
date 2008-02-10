// This file contains some hacks for supporting LARGE_INTEGERS.

// The basic strategy is thus:

// @doc

// @object LARGE_INTEGER|A Python object used wherever a COM LARGE_INTEGER is used.
// @comm Please see <om pywintypes.ULARGE_INTEGER> for a description.

// @object ULARGE_INTEGER|A Python object used wherever a COM ULARGE_INTEGER is used.

// @comm When passed into a Python function, this will always be a single object.
// It will either be an integer, or a long integer, depending on the size.
// When passed into a Pythoncom function, either a single integer object or a tuple of
// integers is supported.
// @todo Note that you can not pass a long integer into Pythoncom.  This needs
// to be fixed.  A work around is to pass a (int, int) tuple.

// This code was given to me by Curt Hagenlocher.  It relies on
// internal knowledge of a Python long.

// Hopefully Python will formalise an API for this?

#include "PyWinTypes.h"
#include "longintrepr.h"

#ifdef __MINGW32__
#define __int64 long long
#endif

PyObject *PyLong_FromI64(__int64 ival)
{
	return PyLong_FromLongLong(ival);
}
PyObject *PyLong_FromUI64(unsigned __int64 ival)
{
	return PyLong_FromUnsignedLongLong(ival);
}

BOOL PyLong_AsI64(PyObject *val, __int64 *lval)
{
	*lval = PyLong_AsLongLong(val);
	return *lval != -1 || !PyErr_Occurred();
}

BOOL PyLong_AsUI64(PyObject *val, unsigned __int64 *lval)
{
	*lval = PyLong_AsUnsignedLongLong(val);
	return *lval != (unsigned __int64)-1 || !PyErr_Occurred();
}

// ### should be obsolete since win32 has __int64 to work with
PyObject *PyLong_FromTwoInts(int hidword, unsigned lodword)
{
    // If it fits in a normal Python int, we return one of them.
	if (hidword==0 && ((lodword & 0x80000000)==0))
		return PyInt_FromLong(lodword);
	else {
		__int64 ival = hidword;
		ival = (ival << 32) + lodword;
		return PyLong_FromI64(ival);
	}
}


// ### should be obsolete since win32 has __int64 to work with
BOOL PyLong_AsTwoInts(PyObject *ob, int *hiint, unsigned *loint)
{
	if (PyInt_Check(ob)) {
		PyErr_Clear();
		*loint = PyInt_AsLong(ob);
		*hiint = (*(int *)loint) < 0 ? -1 : 0;
		return !PyErr_Occurred();
	}
	// Otherwize a long integer.
	__int64 newval = PyLong_AsLongLong(ob);
	if (newval==(__int64)-1 && PyErr_Occurred())
		return FALSE;
	if (hiint) *hiint=(int)(newval>>32);
	if (loint) *loint=(unsigned)newval; // just lob the top 32 bits off!
	return TRUE;
}


BOOL PyWinObject_AsLARGE_INTEGER(PyObject *ob, LARGE_INTEGER *pResult)
{
	if (PyInt_Check(ob)) {
		// 32 bit integer value.
		LISet32(*pResult, PyInt_AS_LONG(ob));
	} else if (PyLong_Check(ob)) {
		return PyLong_AsI64(ob, &pResult->QuadPart);
	} else {
		long hiVal, loVal;
		if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
			PyErr_SetString(PyExc_TypeError, "LARGE_INTEGER must be 'int', or '(int, int)'");
			return FALSE;
		}
		// ### what to do about a "negative" loVal?!
		pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
	}
	return TRUE;
}

BOOL PyWinObject_AsULARGE_INTEGER(PyObject *ob, ULARGE_INTEGER *pResult)
{
	if (PyInt_Check(ob)) {
		// 32 bit integer value.
		int x = PyInt_AS_LONG(ob);
		// ### what to do with "negative" integers?
#if 0
		if ( x < 0 ) {
			PyErr_SetString(PyExc_ValueError, "integer argument must be positive");
			return FALSE;
		}
#endif
		ULISet32(*pResult, x);
	} else if (PyLong_Check(ob)) {
		return PyLong_AsUI64(ob, &pResult->QuadPart);
	} else {
		long hiVal, loVal;
		if (!PyArg_ParseTuple(ob, "ll", &hiVal, &loVal)) {
			PyErr_SetString(PyExc_TypeError, "ULARGE_INTEGER must be 'int', or '(int, int)'");
			return FALSE;
		}
		// ### what to do about "negative" integers?!
		pResult->QuadPart = (((__int64)hiVal) << 32) | loVal;
	}
	return TRUE;
}
PyObject *PyWinObject_FromLARGE_INTEGER(LARGE_INTEGER &val)
{
	// NOTE: The max _signed_ positive integer is the largest we 
	// can return as a simple Python integer.
	if (val.QuadPart < (__int64)0x80000000)
		return PyInt_FromLong((long)val.QuadPart);
	else
		return PyLong_FromI64(val.QuadPart);
}
PyObject *PyWinObject_FromULARGE_INTEGER(ULARGE_INTEGER &val)
{
	if (val.QuadPart < (__int64)0x80000000)
		return PyInt_FromLong((long)val.QuadPart);
	else
		return PyLong_FromUI64(val.QuadPart);
}
