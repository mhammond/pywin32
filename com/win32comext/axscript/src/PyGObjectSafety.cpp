// PyGObjectSafety.cpp

#include "stdafx.h"
#include "PyIObjectSafety.h"

STDMETHODIMP PyGObjectSafety::GetInterfaceSafetyOptions(
    /* [in] */ REFIID riid,
    /* [out] */ DWORD __RPC_FAR *pdwSupportedOptions,
    /* [out] */ DWORD __RPC_FAR *pdwEnabledOptions)
{
    PY_GATEWAY_METHOD;
    PyObject *obIID = PyWinObject_FromIID(riid);
    PyObject *result = NULL;
    HRESULT hr = S_OK;
    if (obIID) {
        hr = InvokeViaPolicy("GetInterfaceSafetyOptions", &result, "O", obIID);
        if (FAILED(hr))
            return hr;
    }

    if (result)
        PyArg_ParseTuple(result, "ii", pdwSupportedOptions, pdwEnabledOptions);
    if (PyErr_Occurred())
        hr = PyCom_HandlePythonFailureToCOM();
    Py_XDECREF(obIID);
    Py_XDECREF(result);
    return hr;
}

STDMETHODIMP PyGObjectSafety::SetInterfaceSafetyOptions(
    /* [in] */ REFIID riid,
    /* [in] */ DWORD dwOptionSetMask,
    /* [in] */ DWORD dwEnabledOptions)
{
    PY_GATEWAY_METHOD;
    PyObject *obIID = PyWinObject_FromIID(riid);
    PyObject *result = NULL;
    if (obIID) {
        HRESULT hr =
            InvokeViaPolicy("SetInterfaceSafetyOptions", &result, "Oii", obIID, dwOptionSetMask, dwEnabledOptions);
        if (FAILED(hr))
            return hr;
    }
    HRESULT hr = result == NULL ? PyCom_HandlePythonFailureToCOM() : S_OK;
    Py_XDECREF(obIID);
    Py_XDECREF(result);
    return hr;
}
