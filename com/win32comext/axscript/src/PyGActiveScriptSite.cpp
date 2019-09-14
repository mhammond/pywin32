#include "stdafx.h"

STDMETHODIMP PyGActiveScriptSite::GetLCID(
    /* [out] */ LCID FAR *plcid)
{
    PY_GATEWAY_METHOD;
    if (plcid == NULL)
        return E_POINTER;

    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetLCID", &result, NULL);
    if (FAILED(hr))
        return hr;

    *plcid = PyInt_AsLong(result);
    Py_DECREF(result);
    return PyCom_HandlePythonFailureToCOM();
}

STDMETHODIMP PyGActiveScriptSite::GetItemInfo(
    /* [in] */ LPCOLESTR pstrName,
    /* [in] */ DWORD dwReturnMask,
    /* [out] */ IUnknown FAR *FAR *ppiunkItem,
    /* [out] */ ITypeInfo FAR *FAR *ppti)
{
    PY_GATEWAY_METHOD;
    if (((dwReturnMask & SCRIPTINFO_IUNKNOWN) && ppiunkItem == NULL) ||
        ((dwReturnMask & SCRIPTINFO_ITYPEINFO) && ppti == NULL))
        return E_POINTER;

    PyObject *result;
    PyObject *obName = PyWinObject_FromOLECHAR(pstrName);
    HRESULT hr = InvokeViaPolicy("GetItemInfo", &result, "Ni", obName, (int)dwReturnMask);
    if (FAILED(hr))
        return hr;
    if (!PySequence_Check(result) || PyObject_Length(result) != 2) {
        Py_DECREF(result);
        return E_FAIL;
    }
    PyObject *obIUnknown = PySequence_GetItem(result, 0);
    if (!obIUnknown) {
        Py_DECREF(result);
        return E_FAIL;
    }
    PyObject *obITypeInfo = PySequence_GetItem(result, 1);
    if (!obITypeInfo) {
        Py_DECREF(obIUnknown);
        Py_DECREF(result);
        return E_FAIL;
    }
    Py_DECREF(result);

    hr = E_FAIL;
    if (obIUnknown != Py_None && (dwReturnMask & SCRIPTINFO_IUNKNOWN) != 0) {
        if (!PyIBase::is_object(obIUnknown, &PyIUnknown::type))
            goto error;

        *ppiunkItem = PyIUnknown::GetI(obIUnknown);
        if (!*ppiunkItem)
            goto error;
        (*ppiunkItem)->AddRef();
    }
    else if (ppiunkItem)
        *ppiunkItem = NULL;

    if (obITypeInfo != Py_None && (dwReturnMask & SCRIPTINFO_ITYPEINFO) != 0) {
        if (!PyIBase::is_object(obITypeInfo, &PyITypeInfo::type))
            goto error;

        *ppti = PyITypeInfo::GetI(obITypeInfo);
        if (!*ppti)
            goto error;
        (*ppti)->AddRef();
    }
    else if (ppti)
        *ppti = NULL;

    hr = S_OK;

error:
    Py_DECREF(obIUnknown);
    Py_DECREF(obITypeInfo);

    return hr;
}

STDMETHODIMP PyGActiveScriptSite::GetDocVersionString(
    /* [out] */ BSTR FAR *pbstrVersion)
{
    if (pbstrVersion == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetDocVersionString", &result, NULL);
    if (FAILED(hr))
        return hr;
    PyWinObject_AsBstr(result, pbstrVersion, FALSE);
    Py_XDECREF(result);
    return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
}

STDMETHODIMP PyGActiveScriptSite::OnScriptTerminate(
    /* [in] */ const VARIANT FAR *pvarResult,
    /* [in] */ const EXCEPINFO FAR *pexcepinfo)
{
    PY_GATEWAY_METHOD;
    PyObject *obResult;
    if (pvarResult) {
        obResult = PyCom_PyObjectFromVariant(pvarResult);
        if (!obResult) {
            PyErr_Clear();
            return E_FAIL;
        }
    }
    else {
        Py_INCREF(Py_None);
        obResult = Py_None;
    }

    PyObject *obExcepInfo;
    if (pexcepinfo) {
        obExcepInfo = PyCom_PyObjectFromExcepInfo(pexcepinfo);
        if (!obExcepInfo) {
            Py_DECREF(obResult);
            PyErr_Clear();
            return E_FAIL;
        }
    }
    else {
        Py_INCREF(Py_None);
        obExcepInfo = Py_None;
    }

    HRESULT hr = InvokeViaPolicy("OnScriptTerminate", NULL, "OO", obResult, obExcepInfo);
    Py_DECREF(obResult);
    Py_DECREF(obExcepInfo);
    return hr;
}

STDMETHODIMP PyGActiveScriptSite::OnStateChange(
    /* [in] */ SCRIPTSTATE ssScriptState)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("OnStateChange", NULL, "i", (int)ssScriptState);
}

STDMETHODIMP PyGActiveScriptSite::OnScriptError(
    /* [in] */ IActiveScriptError FAR *pscripterror)
{
    PY_GATEWAY_METHOD;
    PyObject *obGateway = PyCom_PyObjectFromIUnknown(pscripterror, IID_IActiveScriptError, TRUE);
    if (obGateway == NULL)
        return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("OnScriptError", &result, "O", obGateway);
    Py_DECREF(obGateway);
    if (FAILED(hr))
        return hr;
    if (result == Py_None)
        hr = S_OK;
    else {
        hr = PyInt_AsLong(result);
        if (hr == -1 && PyErr_Occurred())
            hr = PyCom_HandlePythonFailureToCOM();
    }
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScriptSite::OnEnterScript(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("OnEnterScript", NULL, NULL);
}

STDMETHODIMP PyGActiveScriptSite::OnLeaveScript(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("OnLeaveScript", NULL, NULL);
}
