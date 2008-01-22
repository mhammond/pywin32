// PyIEnumGUID

// @doc
#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_IENUMGUID

#include <comcat.h>
#include "PyIEnumGUID.h"

PyIEnumGUID::PyIEnumGUID(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIEnumGUID::~PyIEnumGUID()
{
}

/*static*/ IEnumGUID *PyIEnumGUID::GetI(PyObject *self)
{
	return (IEnumGUID *)PyIUnknown::GetI(self);
}


// @pymethod (<o PyIID>, ...)|PyIEnumGUID|Next|Retrieves a specified number of items in the enumeration sequence.
PyObject *PyIEnumGUID::Next(PyObject *self, PyObject *args)
{
	long celt = 1;
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "|l:Next", &celt) )
		return NULL;

	IEnumGUID *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	GUID *rgVar = new GUID[celt];
	if ( rgVar == NULL ) {
		PyErr_SetString(PyExc_MemoryError, "allocating result GUIDs");
		return NULL;
	}

	int i;
	ULONG celtFetched;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Next(celt, rgVar, &celtFetched);
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
			PyObject *ob = PyWinObject_FromIID(rgVar[i]);
			if ( ob == NULL )
			{
				Py_DECREF(result);
				result = NULL;
				break;
			}
			PyTuple_SET_ITEM(result, i, ob);
		}
	}
	delete [] rgVar;
	return result;
	// @rdesc The result is a tuple of <o PyIID> objects, 
	// one for each element returned.  Note that if zero elements are returned, it is not considered
	// an error condition - an empty tuple is simply returned.
}
// @pymethod |PyIEnumGUID|Skip|Skips over the next specified elementes.
PyObject *PyIEnumGUID::Skip(PyObject *self, PyObject *args)
{
	ULONG num;
	// @pyparm int|num||The number of elements being requested.
	if (!PyArg_ParseTuple(args, "l:Skip", &num))
		return NULL;

	IEnumGUID *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Skip(num);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |PyIEnumGUID|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumGUID::Reset(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Reset"))
		return NULL;

	IEnumGUID *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Reset();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumGUID>|PyIEnumGUID|Clone|Creates another enumerator that contains the same enumeration state as the current one
PyObject *PyIEnumGUID::Clone(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Clone"))
		return NULL;

	IEnumGUID *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IEnumGUID *pNew = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Clone(&pNew);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pNew, IID_IEnumGUID, FALSE);
}

// @object PyIEnumGUID|A Python interface to IEnumGUID
static struct PyMethodDef PyIEnumGUID_methods[] =
{
	{"Next",          PyIEnumGUID::Next,  1}, // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
	{"Skip",          PyIEnumGUID::Skip,  1}, // @pymeth Skip|Skips over the next specified elementes.
	{"Reset",          PyIEnumGUID::Reset,  1}, // @pymeth Reset|Resets the enumeration sequence to the beginning.
	{"Clone",          PyIEnumGUID::Clone,  1}, // @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
	{NULL,  NULL}        
};

PyComEnumTypeObject PyIEnumGUID::type("PyIEnumGUID",
                 &PyIUnknown::type, // @base PyIEnumGUID|PyIUnknown
                 sizeof(PyIEnumGUID),
                 PyIEnumGUID_methods,
				 GET_PYCOM_CTOR(PyIEnumGUID));

// ---------------------------------------------------
//
// Gateway Implementation


STDMETHODIMP PyGEnumGUID::Next(ULONG celt, GUID __RPC_FAR *rgVar,ULONG __RPC_FAR *pCeltFetched)
{
	PY_GATEWAY_METHOD;
	PyObject *result, *result_tuple, *result_item;
	ULONG item_index;
	HRESULT hr = InvokeViaPolicy("Next", &result, "i", celt);
	if ( FAILED(hr) )
		return hr;

	// Caller is expected to allocate array of GUIDs
	ZeroMemory(rgVar, celt*sizeof(LPOLESTR));
	result_tuple=PySequence_Tuple(result);
	if (result_tuple==NULL)
		return PyCom_SetCOMErrorFromPyException(IID_IEnumGUID);
	hr=S_OK;
	*pCeltFetched = PyWin_SAFE_DOWNCAST(PyTuple_GET_SIZE(result_tuple), Py_ssize_t, ULONG);
	if (*pCeltFetched > celt){
		PyErr_Format(PyExc_ValueError, "Received %d items , but only %d items requested", *pCeltFetched, celt);
		hr=PyCom_SetCOMErrorFromPyException(IID_IEnumGUID);
		}
	else
		for (item_index = 0; item_index < *pCeltFetched; item_index++){
			result_item = PyTuple_GET_ITEM(result_tuple, item_index);
			if (!PyWinObject_AsIID(result_item, &rgVar[item_index])){
				hr=PyCom_SetCOMErrorFromPyException(IID_IEnumGUID);
				break;
				}
			}

	Py_DECREF(result_tuple);
	return hr;
}

STDMETHODIMP PyGEnumGUID::Skip(ULONG celt)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("Skip", NULL, "i", celt);
}

STDMETHODIMP PyGEnumGUID::Reset(void)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("Reset");
}

STDMETHODIMP PyGEnumGUID::Clone(IEnumGUID __RPC_FAR *__RPC_FAR *ppEnum)
{
	PY_GATEWAY_METHOD;
	PyObject * result;
	HRESULT hr = InvokeViaPolicy("Clone", &result);
	if ( FAILED(hr) )
		return hr;

	if ( !PyIBase::is_object(result, &PyIUnknown::type) )
	{
		// the wrong kind of object was returned to us
		Py_DECREF(result);
		return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumGUID);
	}

	IUnknown *punk = ((PyIUnknown *)result)->m_obj;
	if ( !punk )
	{
		Py_DECREF(result);
		return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumGUID);
	}

	Py_BEGIN_ALLOW_THREADS
	hr = punk->QueryInterface(IID_IEnumGUID, (LPVOID *)ppEnum);
	Py_END_ALLOW_THREADS

	// done with the result; this DECREF is also for <punk>
	Py_DECREF(result);

	return PyCom_SetCOMErrorFromSimple(hr, IID_IEnumGUID);
}

#endif // NO_PYCOM_IENUMGUID
