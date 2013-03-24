// This file implements the IEnumSTATPROPSETSTG Interface and Gateway for Python.
// Cloned from PyIEnumSTATPROPSTG.cpp

#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"

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
			if (ob == NULL){
				Py_DECREF(result);
				result = NULL;
				break;
				}
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

// ---------------------------------------------------
//
// Gateway Implementation
STDMETHODIMP PyGEnumSTATPROPSETSTG::Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgVar,
            /* [out] */ ULONG __RPC_FAR *pCeltFetched)
{
	PY_GATEWAY_METHOD;
	PyObject *result;
	HRESULT hr = InvokeViaPolicy("Next", &result, "i", celt);
	if ( FAILED(hr) )
		return hr;

	if ( !PySequence_Check(result) )
		goto error;
	int len;
	len = PyObject_Length(result);
	if ( len == -1 )
		goto error;
	if ( len > (int)celt)
		len = celt;

	if ( pCeltFetched )
		*pCeltFetched = len;

	int i;
	for ( i = 0; i < len; ++i )
	{
		TmpPyObject ob = PySequence_GetItem(result, i);
		if ( ob == NULL )
			goto error;

		if ( !PyCom_PyObjectAsSTATPROPSETSTG(ob, &rgVar[i]) )
		{
			Py_DECREF(result);
			return PyCom_SetCOMErrorFromPyException(IID_IEnumSTATPROPSETSTG);
		}
	}

	Py_DECREF(result);

	return len < (int)celt ? S_FALSE : S_OK;

  error:
	PyErr_Clear();	// just in case
	Py_DECREF(result);
	return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumSTATPROPSETSTG, "Next() did not return a sequence of objects");
}

STDMETHODIMP PyGEnumSTATPROPSETSTG::Skip( 
            /* [in] */ ULONG celt)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("Skip", NULL, "i", celt);
}

STDMETHODIMP PyGEnumSTATPROPSETSTG::Reset(void)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("Reset");
}

STDMETHODIMP PyGEnumSTATPROPSETSTG::Clone( 
            /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppEnum)
{
	PY_GATEWAY_METHOD;
	PyObject * result;
	HRESULT hr = InvokeViaPolicy("Clone", &result);
	if ( FAILED(hr) )
		return hr;

	/*
	** Make sure we have the right kind of object: we should have some kind
	** of IUnknown subclass wrapped into a PyIUnknown instance.
	*/
	if ( !PyIBase::is_object(result, &PyIUnknown::type) )
	{
		/* the wrong kind of object was returned to us */
		Py_DECREF(result);
		return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumSTATPROPSETSTG);
	}

	/*
	** Get the IUnknown out of the thing. note that the Python ob maintains
	** a reference, so we don't have to explicitly AddRef() here.
	*/
	IUnknown *punk = ((PyIUnknown *)result)->m_obj;
	if ( !punk )
	{
		/* damn. the object was released. */
		Py_DECREF(result);
		return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumSTATPROPSETSTG);
	}

	/*
	** Get the interface we want. note it is returned with a refcount.
	** This QI is actually going to instantiate a PyGEnumSTATPROPSETSTG.
	*/
	hr = punk->QueryInterface(IID_IEnumSTATPROPSETSTG, (LPVOID *)ppEnum);

	/* done with the result; this DECREF is also for <punk> */
	Py_DECREF(result);

	return PyCom_SetCOMErrorFromSimple(hr, IID_IEnumSTATPROPSETSTG, "Python could not convert the result from Next() into the required COM interface");
}

#endif // NO_PYCOM_ENUMSTATPROPSETSTG
