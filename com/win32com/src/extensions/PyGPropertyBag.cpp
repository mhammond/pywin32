#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGPropertyBag::Read(
            /* [in] */ LPCOLESTR pszPropName,
            /* [out][in] */ VARIANT __RPC_FAR *pVar,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog)
{
	if ( pszPropName == NULL || pVar == NULL )
		return E_POINTER;

	PY_GATEWAY_METHOD;
	PyObject *obLog;
	if ( pErrorLog )
	{
		obLog = PyCom_PyObjectFromIUnknown(pErrorLog, IID_IErrorLog, TRUE);
		if ( !obLog )
			return PyCom_HandlePythonFailureToCOM();
	}
	else
	{
		Py_INCREF(Py_None);
		obLog = Py_None;
	}

	PyObject *obName = PyString_FromUnicode(pszPropName); // keep with string for b/w compat.
	PyObject *result = DispatchViaPolicy("Read",
										 "OiO",
										 obName,
										 (int)V_VT(pVar),
										 obLog);

	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_DECREF(obLog);
	Py_XDECREF(obName);
	if ( !result )
		return hr;

	BOOL ok = PyCom_VariantFromPyObject(result, pVar);
	Py_DECREF(result);

	return ok ? S_OK : E_FAIL;
}

STDMETHODIMP PyGPropertyBag::Write(
            /* [in] */ LPCOLESTR pszPropName,
            /* [in] */ VARIANT __RPC_FAR *pVar)
{
	if ( pszPropName == NULL || pVar == NULL )
		return E_POINTER;

	PY_GATEWAY_METHOD;
	PyObject *value = PyCom_PyObjectFromVariant(pVar);
	if ( !value )
		return PyCom_HandlePythonFailureToCOM();

	PyObject *obName = PyString_FromUnicode(pszPropName); // keep with string for b/w compat.
	PyObject *result = DispatchViaPolicy("Write",
										 "OO",
										 obName,
										 value);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_DECREF(value);
	Py_XDECREF(result);
	Py_XDECREF(obName);
	return hr;
}

