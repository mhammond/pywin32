#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include <PyGConnectionPointContainer.h>

#ifdef MS_WINCE
#include "olectl.h" // For the error codes!?
#endif

STDMETHODIMP PyGConnectionPointContainer::EnumConnectionPoints(IEnumConnectionPoints **)
{
/*	
	PY_GATEWAY_METHOD;
	PyObject *result = DispatchViaPolicy("EnumConnectionPoints", NULL);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
*/
	return E_NOTIMPL;
}

STDMETHODIMP PyGConnectionPointContainer::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
	PY_GATEWAY_METHOD;
	if (ppCP==NULL) return E_POINTER;
	*ppCP = NULL;
	PyObject *obIID = PyWinObject_FromIID(riid);
	PyObject *result = DispatchViaPolicy("FindConnectionPoint", "O", obIID);
	Py_XDECREF(obIID);
	if (result)
		PyCom_InterfaceFromPyObject(result, IID_IConnectionPoint, (void **)ppCP);
	Py_XDECREF(result);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	return (hr==S_OK && *ppCP==NULL) ? CONNECT_E_NOCONNECTION : hr;
}

