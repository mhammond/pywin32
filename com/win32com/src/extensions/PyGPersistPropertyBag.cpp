#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGPersistPropertyBag::InitNew(void)
{
	PY_GATEWAY_METHOD;
	PyObject *result = DispatchViaPolicy("InitNew", NULL);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGPersistPropertyBag::Load(
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog)
{
	HRESULT hr;

	if ( pPropBag == NULL )
		return E_POINTER;

	PY_GATEWAY_METHOD;
	PyObject *obLog;
	if ( pErrorLog != NULL )
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

	PyObject *obBag = PyCom_PyObjectFromIUnknown(pPropBag, IID_IPropertyBag, TRUE);
	if ( !obBag )
	{
		hr = PyCom_HandlePythonFailureToCOM();
		Py_DECREF(obLog);
		return hr;
	}

	PyObject *result = DispatchViaPolicy("Load", "OO", obBag, obLog);
	hr = PyCom_HandlePythonFailureToCOM();
	Py_DECREF(obBag);
	Py_DECREF(obLog);
	Py_XDECREF(result);
	return hr;
}

STDMETHODIMP PyGPersistPropertyBag::Save(
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ BOOL fClearDirty,
            /* [in] */ BOOL fSaveAllProperties)
{
	if ( pPropBag == NULL )
		return E_POINTER;

	PY_GATEWAY_METHOD;
	PyObject *obBag = PyCom_PyObjectFromIUnknown(pPropBag, IID_IPropertyBag, TRUE);
	if ( !obBag )
		return PyCom_HandlePythonFailureToCOM();

	PyObject *result = DispatchViaPolicy("Save",
										 "Oii",
										 obBag,
										 (int)fClearDirty,
										 (int)fSaveAllProperties);
	HRESULT hr = PyCom_HandlePythonFailureToCOM();
	Py_DECREF(obBag);
	Py_XDECREF(result);
	return hr;
}

