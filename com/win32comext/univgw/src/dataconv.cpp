//
// Data conversion
//

#include "stdafx.h"

PyObject * dataconv_L64(PyObject *self, PyObject *args)
{
	void *pSrc;
	int size;

	if ( !PyArg_ParseTuple(args, "s#:L64", &pSrc, &size) )
		return NULL;
	if ( size != 8 )
	{
		PyErr_SetString(PyExc_ValueError, "argument must be 8 characters");
		return NULL;
	}

	return PyLong_FromLongLong(*(__int64 *)pSrc);
}

PyObject * dataconv_UL64(PyObject *self, PyObject *args)
{
	void *pSrc;
	int size;

	if ( !PyArg_ParseTuple(args, "s#:UL64", &pSrc, &size) )
		return NULL;
	if ( size != 8 )
	{
		PyErr_SetString(PyExc_ValueError, "argument must be 8 characters");
		return NULL;
	}

	return PyLong_FromUnsignedLongLong(*(unsigned __int64 *)pSrc);
}

PyObject * dataconv_strL64(PyObject *self, PyObject *args)
{
	__int64 val;

	if ( !PyArg_ParseTuple(args, "L:strL64", &val) )
		return NULL;

	return PyString_FromStringAndSize((char *)&val, sizeof(val));
}

PyObject * dataconv_strUL64(PyObject *self, PyObject *args)
{
	PyObject *ob;

	if ( !PyArg_ParseTuple(args, "O!:strUL64", &PyLong_Type, &ob) )
		return NULL;

	unsigned __int64 val = PyLong_AsUnsignedLongLong(ob);

	return PyString_FromStringAndSize((char *)&val, sizeof(val));
}

PyObject * dataconv_interface(PyObject *self, PyObject *args)
{
	PyObject *obPtr;
	PyObject *obIID;

	if ( !PyArg_ParseTuple(args, "OO:interface", &obPtr, &obIID) )
		return NULL;

	// determine the Py2COM thunk to wrap around the punk
	IID iid;
	if ( !PyWinObject_AsIID(obIID, &iid) )
		return NULL;

	IUnknown *punk = (IUnknown *)PyLong_AsVoidPtr(obPtr);
	if ( punk == NULL && PyErr_Occurred() )
		return NULL;

	// make sure to add a ref, which will be released with this object
	return PyCom_PyObjectFromIUnknown(punk, iid, TRUE);
}

#if 0
PyObject * dataconv_Read(char *fmt, void *p)
{
	switch ( *fmt )
	{
	case 'p':	// pointer
		return PyInt_FromLong((long)*(void **)p, NULL);

	case 'u':	// unsigned int/long
		// note: ULONG == DWORD
		return PyLong_FromDouble((double)*(ULONG *)p);

	case 'i':	// int/long
		return PyLong_FromLong(*(int *)p);

	case 'f':	// float
		return PyFloat_FromDouble(*(float *)p);

	case 'd':	// double
		return PyFloat_FromDouble(*(double *)p);

	case 'L':	// LARGE_INTEGER
		return PyWinObject_FromLARGE_INTEGER(*(LARGE_INTEGER *)p);

	case 'U':	// ULARGE_INTEGER
		return PyWinObject_FromULARGE_INTEGER(*(ULARGE_INTEGER *)p);

	case 's':	// OLECHAR *
		return PyWinObject_FromOLECHAR(*(OLECHAR **)p);

	case 'v':	// VARIANT
		return PyCom_PyObjectFromVariant(*(VARIANT **)p);

	case 'C':	// CLSID
		IID iid = *(IID **)p;
		return PyWinObject_FromIID(iid);

	case 'B':	// BSTR
		return PyWinObject_FromBsr(*(BSTR *)p);
	}

	PyErr_SetString(PyExc_RuntimeError, "bad char in fmt");
	return NULL;
}
#endif /* 0 */
