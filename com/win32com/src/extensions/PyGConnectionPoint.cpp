#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include <PyGConnectionPoint.h>

STDMETHODIMP PyGConnectionPoint::GetConnectionInterface(IID *pIID)
{
	PY_GATEWAY_METHOD;
	if (pIID==NULL) return E_POINTER;
	PyObject *result;
	HRESULT hr = InvokeViaPolicy("GetConnectionInterface", &result, NULL);
	if (FAILED(hr)) return hr;
	if (!PyWinObject_AsIID(result, pIID))
		hr = PyCom_SetCOMErrorFromPyException(IID_IConnectionPoint);
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::GetConnectionPointContainer( IConnectionPointContainer **ppCPC)
{
	PY_GATEWAY_METHOD;
	if (ppCPC==NULL) return E_POINTER;
	PyObject *result;
	HRESULT hr = InvokeViaPolicy("GetConnectionPointContainer", &result, NULL);
	if (FAILED(hr)) return hr;
	if (!PyCom_InterfaceFromPyObject(result, IID_IConnectionPointContainer, (void **)ppCPC))
		hr = PyCom_SetCOMErrorFromPyException(GetIID());
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::Advise(IUnknown *pUnk, DWORD *pdwCookie)
{
	PY_GATEWAY_METHOD;
	if (pUnk==NULL || pdwCookie==NULL) return E_POINTER;
	PyObject *obUnknown = PyCom_PyObjectFromIUnknown(pUnk, IID_IUnknown, TRUE); // Take a reference to this object.
	if (obUnknown==NULL)
		return PyCom_SetCOMErrorFromPyException(GetIID());

	PyObject *result = NULL;
	*pdwCookie = 0;
	HRESULT hr = InvokeViaPolicy("Advise", &result, "O", obUnknown);
	Py_DECREF(obUnknown);
	if (FAILED(hr)) return hr;
	*pdwCookie = PyInt_AsLong(result);
	if (PyErr_Occurred()) {
		hr = PyCom_SetCOMErrorFromPyException(GetIID());
		*pdwCookie = 0;
	}
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGConnectionPoint::Unadvise(DWORD cookie)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("Unadvise", NULL, "i", cookie);
}

STDMETHODIMP PyGConnectionPoint::EnumConnections(IEnumConnections **ppEnum)
{
	return E_NOTIMPL;
/*
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy("EnumConnections", NULL, NULL);
*/
}

