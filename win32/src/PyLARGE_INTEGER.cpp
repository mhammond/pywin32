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

#include "windows.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "longintrepr.h"

#ifdef PYWIN_NO_PYTHON_LONG_LONG
/* If we either dont have, or dont want to use, Python's native
   64bit integer support.
*/
PyLongObject *PyLong_Normalize(register PyLongObject *v);
PyLongObject *alloclongobject(int size);
static __int64 PyWLong_AsLongLong(PyObject *vv);
#define PyLong_AsLongLong PyWLong_AsLongLong

static unsigned __int64 PyWLong_AsUnsignedLongLong(PyObject *vv);
#define PyLong_AsUnsignedLongLong PyWLong_AsUnsignedLongLong


PyObject *PyLong_FromI64(__int64 ival)
{
 int i;

 /* A 64-bit value should fit in 5 'digits' */
 int n = 5;
 PyLongObject *v = alloclongobject(n);
 if (v == NULL)
  return NULL;

 if (ival < 0)
 {
  ival = -ival;
  v->ob_size = -(v->ob_size);
 }

 unsigned __int64 uval = (unsigned __int64)ival;
 for (i = 0; i < n; i++)
 {
  v->ob_digit[i] = (unsigned short)(uval & MASK);
  uval = (uval >> SHIFT);
 }
 v = PyLong_Normalize(v);

 return (PyObject*)v;
}

// ### the hack already does unsigned(!)
#define PyLong_FromUI64(x)	PyLong_FromI64(x)

#else // PYWIN_NO_PYTHON_LONG_LONG
// We have native support - use it.
PyObject *PyLong_FromI64(__int64 ival)
{
	return PyLong_FromLongLong(ival);
}
PyObject *PyLong_FromUI64(unsigned __int64 ival)
{
	return PyLong_FromUnsignedLongLong(ival);
}

#endif // PYWIN_NO_PYTHON_LONG_LONG

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
	if (hidword==0)
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
		*hiint = 0;
		*loint = PyInt_AsLong(ob);
		return !PyErr_Occurred();
	}
	// Otherwize a long integer.
	__int64 newval = PyLong_AsLongLong(ob);
	if (newval==(_int64)-1)
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
		pResult->HighPart = hiVal;
		pResult->LowPart = loVal;
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
		pResult->HighPart = hiVal;
		pResult->LowPart = loVal;
	}
	return TRUE;
}
PyObject *PyWinObject_FromLARGE_INTEGER(LARGE_INTEGER &val)
{
	if (val.HighPart==0)
		return PyInt_FromLong(val.LowPart);
	else
		return PyLong_FromI64(val.QuadPart);
}
PyObject *PyWinObject_FromULARGE_INTEGER(ULARGE_INTEGER &val)
{
	if (val.HighPart==0)
		// ### this should check for overflow!! (don't want a negative int)
		return PyInt_FromLong(val.LowPart);
	else
		return PyLong_FromUI64(val.QuadPart);
}

#ifdef PYWIN_NO_PYTHON_LONG_LONG
// Our own hacks for 64bit support.
#include <longintrepr.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

PyLongObject *PyLong_Normalize(register PyLongObject *v)
{
 int j = ABS(v->ob_size);
 register int i = j;

 while (i > 0 && v->ob_digit[i-1] == 0)
  --i;
 if (i != j)
  v->ob_size = (v->ob_size < 0) ? -(i) : i;
 return v;
}

PyLongObject *alloclongobject(int size)
{
 return PyObject_NEW_VAR(PyLongObject, &PyLong_Type, size);
}


__int64
PyWLong_AsLongLong(PyObject *vv)
{
	register PyLongObject *v;
	__int64 x, prev;
	int i, sign;
	
	if (vv == NULL || !PyLong_Check(vv)) {
		PyErr_BadInternalCall();
		return -1;
	}

	v = (PyLongObject *)vv;
	i = v->ob_size;
	sign = 1;
	x = 0;

	if (i < 0) {
		sign = -1;
		i = -(i);
	}

	while (--i >= 0) {
		prev = x;
		x = (x << SHIFT) + v->ob_digit[i];
		if ((x >> SHIFT) != prev) {
			PyErr_SetString(PyExc_OverflowError,
				"long int too long to convert");
			return -1;
		}
	}

	return x * sign;
}

// ### hack this dumb thing for now
unsigned __int64
PyWLong_AsUnsignedLongLong(PyObject *vv)
{
	return (unsigned __int64)PyLong_AsLongLong(vv);
}
#endif
