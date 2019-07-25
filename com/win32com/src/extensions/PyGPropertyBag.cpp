#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGPropertyBag::Read(
    /* [in] */ LPCOLESTR pszPropName,
    /* [out][in] */ VARIANT __RPC_FAR *pVar,
    /* [in] */ IErrorLog __RPC_FAR *pErrorLog)
{
    if (pszPropName == NULL || pVar == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *obLog;
    if (pErrorLog) {
        obLog = PyCom_PyObjectFromIUnknown(pErrorLog, IID_IErrorLog, TRUE);
        if (!obLog)
            return PyCom_SetCOMErrorFromPyException(GetIID());
    }
    else {
        Py_INCREF(Py_None);
        obLog = Py_None;
    }

    PyObject *obName = PyWinObject_FromWCHAR(pszPropName);
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("Read", &result, "OiO", obName, (int)V_VT(pVar), obLog);

    Py_DECREF(obLog);
    Py_XDECREF(obName);
    if (FAILED(hr))
        return hr;
    if (!PyCom_VariantFromPyObject(result, pVar))
        hr = PyCom_SetCOMErrorFromPyException(GetIID());

    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGPropertyBag::Write(
    /* [in] */ LPCOLESTR pszPropName,
    /* [in] */ VARIANT __RPC_FAR *pVar)
{
    if (pszPropName == NULL || pVar == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *value = PyCom_PyObjectFromVariant(pVar);
    if (!value)
        return PyCom_SetCOMErrorFromPyException(GetIID());

    PyObject *obName = PyWinObject_FromWCHAR(pszPropName);
    HRESULT hr = InvokeViaPolicy("Write", NULL, "OO", obName, value);
    Py_DECREF(value);
    Py_XDECREF(obName);
    return hr;
}
