#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGErrorLog::AddError(
            /* [in] */ LPCOLESTR pszPropName,
            /* [in] */ EXCEPINFO __RPC_FAR *pExcepInfo)
{
	if ( pszPropName == NULL || pExcepInfo == NULL )
		return E_POINTER;

	PY_GATEWAY_METHOD;
	PyObject *obExcepInfo = PyCom_PyObjectFromExcepInfo(pExcepInfo);
	if ( !obExcepInfo )
		return PyCom_HandlePythonFailureToCOM();

	// We use a string object for B/W compatibility.
	PyObject *obName = PyString_FromUnicode(pszPropName);
	PyObject *result = DispatchViaPolicy("AddError",
										 "OO",
										 obName,
										 obExcepInfo);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_DECREF(obExcepInfo);
	Py_XDECREF(obName);
	Py_XDECREF(result);
	return hr;
}

