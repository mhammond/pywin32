#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include <PyGConnectionPoint.h>

STDMETHODIMP PyGConnectionPoint::GetConnectionInterface(IID *pIID)
{
	PY_GATEWAY_METHOD;
	if (pIID==NULL) return E_POINTER;
	PyObject *result = DispatchViaPolicy("GetConnectionInterface", NULL);
	if (result)
		PyWinObject_AsIID(result, pIID);
	Py_XDECREF(result);
	return PyCom_HandlePythonFailureToCOM();
}

STDMETHODIMP PyGConnectionPoint::GetConnectionPointContainer( IConnectionPointContainer **ppCPC)
{
	PY_GATEWAY_METHOD;
	if (ppCPC==NULL) return E_POINTER;
	PyObject *result = DispatchViaPolicy("GetConnectionPointContainer", NULL);
	if (result)
		PyCom_InterfaceFromPyObject(result, IID_IConnectionPointContainer, (void **)ppCPC);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::Advise(IUnknown *pUnk, DWORD *pdwCookie)
{
	PY_GATEWAY_METHOD;
	if (pUnk==NULL || pdwCookie==NULL) return E_POINTER;
	PyObject *obUnknown = PyCom_PyObjectFromIUnknown(pUnk, IID_IUnknown, TRUE); // Take a reference to this object.
	PyObject *result = NULL;
	*pdwCookie = 0;
	if (obUnknown) {
		result = DispatchViaPolicy("Advise", "O", obUnknown);
		Py_DECREF(obUnknown);
		if (result) {
			PyErr_Clear();
			*pdwCookie = PyInt_AsLong(result);
			if (PyErr_Occurred())
				*pdwCookie = 0;
		}
	}
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::Unadvise(DWORD cookie)
{
	PY_GATEWAY_METHOD;
	PyObject *result = DispatchViaPolicy("Unadvise", "i", cookie);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::EnumConnections(IEnumConnections **ppEnum)
{
	return E_NOTIMPL;
/*
	PY_GATEWAY_METHOD;
	PyObject *result = DispatchViaPolicy("EnumConnections", NULL);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
*/
}

