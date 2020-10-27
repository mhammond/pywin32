// PyGActiveScript.cpp

#include "stdafx.h"

STDMETHODIMP PyGActiveScript::SetScriptSite(
    /* [in]  */ IActiveScriptSite *pioss)
{
    PY_GATEWAY_METHOD;
    PyObject *obSite = PyCom_PyObjectFromIUnknown(pioss, IID_IActiveScriptSite, TRUE);
    HRESULT hr = InvokeViaPolicy("SetScriptSite", NULL, "O", obSite);
    Py_DECREF(obSite);
    return hr;
}

STDMETHODIMP PyGActiveScript::GetScriptSite(
    /* [in]  */ REFIID iid,
    /* [out] */ VOID **ppvSiteObject)
{
    PY_GATEWAY_METHOD;
    *ppvSiteObject = NULL;
    PyObject *obIID = PyWinObject_FromIID(iid);
    if (!obIID)
        return PyCom_SetCOMErrorFromPyException(GetIID());

    PyObject *result = NULL;
    HRESULT hr = InvokeViaPolicy("GetScriptSite", &result, "O", obIID);
    Py_DECREF(obIID);
    if (SUCCEEDED(hr)) {
        if (!PyCom_InterfaceFromPyInstanceOrObject(result, iid, ppvSiteObject, TRUE))
            hr = PyCom_HandlePythonFailureToCOM();
    }
    Py_XDECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScript::SetScriptState(
    /* [in]  */ SCRIPTSTATE ss)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("SetScriptState", NULL, "i", (int)ss);
}

STDMETHODIMP PyGActiveScript::GetScriptState(
    /* [out] */ SCRIPTSTATE *pssState)
{
    if (pssState == NULL)
        return E_INVALIDARG;
    PY_GATEWAY_METHOD;
    PyObject *result = NULL;
    HRESULT hr = InvokeViaPolicy("GetScriptState", &result, NULL);
    if (FAILED(hr))
        return hr;
    if (result && PyInt_Check(result))
        *pssState = (SCRIPTSTATE)PyInt_AsLong(result);
    Py_XDECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScript::Close(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("Close", NULL, NULL);
}

STDMETHODIMP PyGActiveScript::AddNamedItem(
    /* [in]  */ LPCOLESTR pstrName,
    /* [in]  */ DWORD dwFlags)
{
    PY_GATEWAY_METHOD;
    PyObject *obName = PyWinObject_FromOLECHAR(pstrName);
    return InvokeViaPolicy("AddNamedItem", NULL, "Ni", obName, dwFlags);
}

STDMETHODIMP PyGActiveScript::AddTypeLib(
    /* [in]  */ REFGUID rguidTypeLib,
    /* [in]  */ DWORD dwMajor,
    /* [in]  */ DWORD dwMinor,
    /* [in]  */ DWORD dwFlags)
{
    PY_GATEWAY_METHOD;
    PyObject *obIID = PyWinObject_FromIID(rguidTypeLib);
    HRESULT hr = InvokeViaPolicy("AddTypeLib", NULL, "Olll", obIID, dwMajor, dwMinor, dwFlags);
    Py_XDECREF(obIID);
    return hr;
}

STDMETHODIMP PyGActiveScript::GetScriptDispatch(
    /* [in]  */ LPCOLESTR pstrItemName,
    /* [out] */ IDispatch **ppdisp)
{
    if (ppdisp == NULL)
        return E_POINTER;
    PY_GATEWAY_METHOD;
    *ppdisp = NULL;
    PyObject *result = NULL;
    PyObject *obItemName = PyWinObject_FromOLECHAR(pstrItemName);
    HRESULT hr = InvokeViaPolicy("GetScriptDispatch", &result, "O", obItemName);
    Py_XDECREF(obItemName);
    if (FAILED(hr))
        return hr;

    if (result)
        PyCom_InterfaceFromPyObject(result, IID_IDispatch, (void **)ppdisp, FALSE);
    if (PyErr_Occurred())
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    Py_XDECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScript::GetCurrentScriptThreadID(
    /* [out] */ SCRIPTTHREADID *pstidThread)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetCurrentScriptThreadID", &result, NULL);
    if (FAILED(hr))
        return hr;

    if (PyInt_Check(result)) {
        *pstidThread = PyInt_AsLong(result);
        if (PyErr_Occurred())
            hr = PyCom_SetCOMErrorFromPyException(GetIID());
    }
    else
        hr = PyCom_SetCOMErrorFromSimple(E_FAIL, GetIID(), "Python did not return an integer");

    Py_XDECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScript::GetScriptThreadID(
    /* [in]  */ DWORD dwWin32ThreadId,
    /* [out] */ SCRIPTTHREADID *pstidThread)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetScriptThreadID", &result, "i", dwWin32ThreadId);
    if (FAILED(hr))
        return hr;
    if (PyInt_Check(result)) {
        *pstidThread = PyInt_AsLong(result);
        if (PyErr_Occurred())
            hr = PyCom_SetCOMErrorFromPyException(GetIID());
    }
    else
        hr = PyCom_SetCOMErrorFromSimple(E_FAIL, GetIID(), "Python didnt return an integer");
    return hr;
}

STDMETHODIMP PyGActiveScript::GetScriptThreadState(
    /* [in]  */ SCRIPTTHREADID stidThread,
    /* [out] */ SCRIPTTHREADSTATE *pstsState)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetScriptThreadState", &result, "i", stidThread);

    if (FAILED(hr))
        return hr;
    if (PyInt_Check(result)) {
        *pstsState = (SCRIPTTHREADSTATE)PyInt_AsLong(result);
        if (PyErr_Occurred())
            hr = PyCom_HandlePythonFailureToCOM();
    }
    else
        hr = PyCom_SetCOMErrorFromSimple(E_FAIL, GetIID(), "Python did not return an integer");
    Py_XDECREF(result);
    return hr;
}

// extern "C" __declspec( dllimport) void PyErr_SetInterrupt();

STDMETHODIMP PyGActiveScript::InterruptScriptThread(
    /* [in]  */ SCRIPTTHREADID stidThread,
    /* [in]  */ const EXCEPINFO *pexcepinfo,
    /* [in]  */ DWORD dwFlags)
{
    PyErr_SetInterrupt();
    return S_OK;
}

STDMETHODIMP PyGActiveScript::Clone(
    /* [out] */ IActiveScript **ppscript)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("Clone", &result, NULL);
    if (FAILED(hr))
        return hr;
    if (!PyCom_InterfaceFromPyObject(result, IID_IActiveScript, (void **)ppscript, FALSE))
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    Py_XDECREF(result);
    return hr;
}
