#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIEnumVARIANT.h"

PyIEnumVARIANT::PyIEnumVARIANT(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIEnumVARIANT::~PyIEnumVARIANT()
{
}

/* static */ IEnumVARIANT *PyIEnumVARIANT::GetI(PyObject *self)
{
	return (IEnumVARIANT *)PyIUnknown::GetI(self);
}

// @pymethod (object, ...)|PyIEnumVARIANT|Next|Retrieves a specified number of items in the enumeration sequence.
PyObject *PyIEnumVARIANT::Next(PyObject *self, PyObject *args)
{
	long celt = 1;
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "|l:Next", &celt) )
		return NULL;

	IEnumVARIANT *pIEVARIANT = GetI(self);
	if ( pIEVARIANT == NULL )
		return NULL;

	VARIANT *rgVar = new VARIANT[celt];
	if ( rgVar == NULL ) {
		PyErr_SetString(PyExc_MemoryError, "allocating result VARIANTs");
		return NULL;
	}

	int i;
	for ( i = celt; i--; )
		VariantInit(&rgVar[i]);

	ULONG celtFetched;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIEVARIANT->Next(celt, rgVar, &celtFetched);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
	{
		delete [] rgVar;
		return PyCom_BuildPyException(hr);
	}

	PyObject *result = PyTuple_New(celtFetched);
	if ( result != NULL )
	{
		for ( i = celtFetched; i--; )
		{
			PyObject *ob = PyCom_PyObjectFromVariant(&rgVar[i]);
			if ( ob == NULL )
			{
				Py_DECREF(result);
				result = NULL;
				break;
			}
			PyTuple_SET_ITEM(result, i, ob);
		}
	}

	for ( i = celtFetched; i--; )
		VariantClear(&rgVar[i]);
	delete [] rgVar;

	return result;
	// @rdesc The result is a tuple of Python objects converted from Variants,
	// one for each element returned.  Note that if zero elements are returned, it is not considered
	// an error condition - an empty tuple is simply returned.
}

// @pymethod |PyIEnumVARIANT|Skip|Skips over the next specified elementes.
PyObject *PyIEnumVARIANT::Skip(PyObject *self, PyObject *args)
{
	long celt;
	if ( !PyArg_ParseTuple(args, "l:Skip", &celt) )
		return NULL;

	IEnumVARIANT *pIEVARIANT = GetI(self);
	if ( pIEVARIANT == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIEVARIANT->Skip(celt);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIEnumVARIANT|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumVARIANT::Reset(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Reset") )
		return NULL;

	IEnumVARIANT *pIEVARIANT = GetI(self);
	if ( pIEVARIANT == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIEVARIANT->Reset();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumVARIANT>|PyIEnumVARIANT|Clone|Creates another enumerator that contains the same enumeration state as the current one
PyObject *PyIEnumVARIANT::Clone(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Clone") )
		return NULL;

	IEnumVARIANT *pIEVARIANT = GetI(self);
	if ( pIEVARIANT == NULL )
		return NULL;

	IEnumVARIANT *pClone;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIEVARIANT->Clone(&pClone);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	return PyCom_PyObjectFromIUnknown(pClone, IID_IEnumVARIANT, FALSE);
}

// @object PyIEnumVARIANT|A Python interface to IEnumVARIANT
static struct PyMethodDef PyIEnumVARIANT_methods[] =
{
	{ "Next", PyIEnumVARIANT::Next, 1 },    // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
	{ "Skip", PyIEnumVARIANT::Skip, 1 },	// @pymeth Skip|Skips over the next specified elementes.
	{ "Reset", PyIEnumVARIANT::Reset, 1 },	// @pymeth Reset|Resets the enumeration sequence to the beginning.
	{ "Clone", PyIEnumVARIANT::Clone, 1 },	// @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
	{ NULL }
};

PyComEnumTypeObject PyIEnumVARIANT::type("PyIEnumVARIANT",
		&PyIUnknown::type, // @base PyIEnumVariant|PyIUnknown
		sizeof(PyIEnumVARIANT),
		PyIEnumVARIANT_methods,
		GET_PYCOM_CTOR(PyIEnumVARIANT));
