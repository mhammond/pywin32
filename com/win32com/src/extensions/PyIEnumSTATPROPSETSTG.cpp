// This file implements the IEnumSTATPROPSETSTG Interface and Gateway for Python.
// Cloned from PyIEnumSTATPROPSTG.cpp

#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_ENUMSTATPROPSETSTG

#include "PyIEnumSTATPROPSETSTG.h"

// @doc - This file contains autoduck documentation

// ---------------------------------------------------
//
// Interface Implementation

PyIEnumSTATPROPSETSTG::PyIEnumSTATPROPSETSTG(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIEnumSTATPROPSETSTG::~PyIEnumSTATPROPSETSTG()
{
}

/* static */ IEnumSTATPROPSETSTG *PyIEnumSTATPROPSETSTG::GetI(PyObject *self)
{
	return (IEnumSTATPROPSETSTG *)PyIUnknown::GetI(self);
}

// @pymethod object|PyIEnumSTATPROPSETSTG|Next|Retrieves a specified number of items in the enumeration sequence.
PyObject *PyIEnumSTATPROPSETSTG::Next(PyObject *self, PyObject *args)
{
	long celt = 1;
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "|l:Next", &celt) )
		return NULL;

	IEnumSTATPROPSETSTG *pIESTATPROPSETSTG = GetI(self);
	if ( pIESTATPROPSETSTG == NULL )
		return NULL;

	STATPROPSETSTG *rgVar = new STATPROPSETSTG[celt];
	if ( rgVar == NULL ) {
		PyErr_SetString(PyExc_MemoryError, "allocating result STATPROPSETSTGs");
		return NULL;
	}

	int i;
/*	for ( i = celt; i--; )
		// *** possibly init each structure element???
*/

	ULONG celtFetched = 0;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIESTATPROPSETSTG->Next(celt, rgVar, &celtFetched);
	PY_INTERFACE_POSTCALL;
	if (  HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS && FAILED(hr) )
	{
		delete [] rgVar;
		return PyCom_BuildPyException(hr);
	}

	PyObject *result = PyTuple_New(celtFetched);
	if ( result != NULL )
	{
		for ( i = celtFetched; i--; )
		{
			PyObject *ob=PyCom_PyObjectFromSTATPROPSETSTG(&(rgVar[i]));
			PyTuple_SET_ITEM(result, i, ob);
		}
	}

	delete [] rgVar;
	return result;
}

// @pymethod |PyIEnumSTATPROPSETSTG|Skip|Skips over the next specified elementes.
PyObject *PyIEnumSTATPROPSETSTG::Skip(PyObject *self, PyObject *args)
{
	long celt;
	if ( !PyArg_ParseTuple(args, "l:Skip", &celt) )
		return NULL;

	IEnumSTATPROPSETSTG *pIESTATPROPSETSTG = GetI(self);
	if ( pIESTATPROPSETSTG == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIESTATPROPSETSTG->Skip(celt);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIEnumSTATPROPSETSTG|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumSTATPROPSETSTG::Reset(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Reset") )
		return NULL;

	IEnumSTATPROPSETSTG *pIESTATPROPSETSTG = GetI(self);
	if ( pIESTATPROPSETSTG == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIESTATPROPSETSTG->Reset();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumSTATPROPSETSTG>|PyIEnumSTATPROPSETSTG|Clone|Creates another enumerator that contains the same enumeration state as the current one
PyObject *PyIEnumSTATPROPSETSTG::Clone(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Clone") )
		return NULL;

	IEnumSTATPROPSETSTG *pIESTATPROPSETSTG = GetI(self);
	if ( pIESTATPROPSETSTG == NULL )
		return NULL;

	IEnumSTATPROPSETSTG *pClone;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIESTATPROPSETSTG->Clone(&pClone);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	return PyCom_PyObjectFromIUnknown(pClone, IID_IEnumSTATPROPSETSTG, FALSE);
}

// @object PyIEnumSTATPROPSETSTG|A Python interface to IEnumSTATPROPSETSTG
static struct PyMethodDef PyIEnumSTATPROPSETSTG_methods[] =
{
	{ "Next", PyIEnumSTATPROPSETSTG::Next, 1 },    // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
	{ "Skip", PyIEnumSTATPROPSETSTG::Skip, 1 },	// @pymeth Skip|Skips over the next specified elementes.
	{ "Reset", PyIEnumSTATPROPSETSTG::Reset, 1 },	// @pymeth Reset|Resets the enumeration sequence to the beginning.
	{ "Clone", PyIEnumSTATPROPSETSTG::Clone, 1 },	// @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
	{ NULL }
};

PyComEnumTypeObject PyIEnumSTATPROPSETSTG::type("PyIEnumSTATPROPSETSTG",
		&PyIUnknown::type,
		sizeof(PyIEnumSTATPROPSETSTG),
		PyIEnumSTATPROPSETSTG_methods,
		GET_PYCOM_CTOR(PyIEnumSTATPROPSETSTG));

#endif // NO_PYCOM_ENUMSTATPROPSETSTG
