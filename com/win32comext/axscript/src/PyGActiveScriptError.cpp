#include "stdafx.h"
#include "PyGActiveScriptError.h"

STDMETHODIMP PyGActiveScriptError::GetExceptionInfo(EXCEPINFO FAR *pexcepinfo)
{
    if (pexcepinfo == NULL)
        return E_POINTER;
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetExceptionInfo", &result, NULL);
    if (FAILED(hr))
        return hr;

    if (!PyCom_ExcepInfoFromPyObject(result, pexcepinfo))
        hr = E_FAIL;

    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScriptError::GetSourcePosition(DWORD *pdwSourceContext, ULONG *pulLineNumber,
                                                     LONG *plCharacterPosition)
{
    if (pdwSourceContext == NULL || pulLineNumber == NULL || plCharacterPosition == NULL)
        return E_POINTER;
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetSourcePosition", &result, NULL);
    if (FAILED(hr))
        return hr;

    if (!PyArg_ParseTuple(result, "iii", pdwSourceContext, pulLineNumber, plCharacterPosition))
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGActiveScriptError::GetSourceLineText(BSTR *pbstrSourceLine)
{
    if (pbstrSourceLine == NULL)
        return E_POINTER;
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetSourceLineText", &result, NULL);
    if (FAILED(hr))
        return hr;

    if (!PyWinObject_AsBstr(result, pbstrSourceLine, TRUE))
        hr = PyCom_SetCOMErrorFromPyException(GetIID());

    Py_DECREF(result);
    return hr;
}
