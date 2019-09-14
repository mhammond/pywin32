// PyGActiveScriptParse.cpp

#include "stdafx.h"

extern HRESULT InvokeGatewayViaPolicy(PyGatewayBase *pGateway, const char *szMethodName, EXCEPINFO *pei,
                                      PyObject **ppResult /* = NULL */, const char *szFormat /* = NULL */, ...);

// IActiveScriptParse
STDMETHODIMP PyGActiveScriptParse::InitNew(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("InitNew", NULL, NULL);
}

STDMETHODIMP PyGActiveScriptParse::AddScriptlet(
    /* [in] */ LPCOLESTR pstrDefaultName,
    /* [in] */ LPCOLESTR pstrCode,
    /* [in] */ LPCOLESTR pstrItemName,
    /* [in] */ LPCOLESTR pstrSubItemName,
    /* [in] */ LPCOLESTR pstrEventName,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD_PTR dwSourceContextCookie,
    /* [in] */ ULONG ulStartingLineNumber,
    /* [in] */ DWORD dwFlags,
    /* [out] */ BSTR __RPC_FAR *pbstrName,
    /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo)
{
    PY_GATEWAY_METHOD;
    PyObject *obDefaultName = PyWinObject_FromOLECHAR(pstrDefaultName);
    PyObject *obCode = PyWinObject_FromOLECHAR(pstrCode);
    PyObject *obItemName = PyWinObject_FromOLECHAR(pstrItemName);
    PyObject *obSubItemName = PyWinObject_FromOLECHAR(pstrSubItemName);
    PyObject *obEventName = PyWinObject_FromOLECHAR(pstrEventName);
    PyObject *obDelimiter = PyWinObject_FromOLECHAR(pstrDelimiter);
    PyObject *obContext = PyWinObject_FromDWORD_PTR(dwSourceContextCookie);
    PyObject *result;
    HRESULT hr =
        InvokeGatewayViaPolicy(this, "AddScriptlet", pexcepinfo, &result, "NNNNNNNi", obDefaultName, obCode, obItemName,
                               obSubItemName, obEventName, obDelimiter, obContext, ulStartingLineNumber);
    if (FAILED(hr))
        return hr;
    PyWinObject_AsBstr(result, pbstrName, FALSE);
    Py_XDECREF(result);
    return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
}

STDMETHODIMP PyGActiveScriptParse::ParseScriptText(
    /* [in] */ LPCOLESTR pstrCode,
    /* [in] */ LPCOLESTR pstrItemName,
    /* [in] */ IUnknown __RPC_FAR *punkContext,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD_PTR dwSourceContextCookie,
    /* [in] */ ULONG ulStartingLineNumber,
    /* [in] */ DWORD dwFlags,
    /* [out] */ VARIANT __RPC_FAR *pvarResult,
    /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo)
{
    PY_GATEWAY_METHOD;
    PyObject *context = PyCom_PyObjectFromIUnknown(punkContext, IID_IUnknown, TRUE);
    if (context == NULL) {
        PyCom_ExcepInfoFromPyException(pexcepinfo);
        return DISP_E_EXCEPTION;
    }
    PyObject *result = NULL;
    BOOL bWantResult = pvarResult != NULL;
    PyObject *obCode = PyWinObject_FromOLECHAR(pstrCode);
    PyObject *obItemName = PyWinObject_FromOLECHAR(pstrItemName);
    PyObject *obDelimiter = PyWinObject_FromOLECHAR(pstrDelimiter);
    PyObject *obContext = PyWinObject_FromDWORD_PTR(dwSourceContextCookie);
    HRESULT hr = InvokeGatewayViaPolicy(this, "ParseScriptText", pexcepinfo, &result, "NNONNiii", obCode, obItemName,
                                        context, obDelimiter, obContext, ulStartingLineNumber, dwFlags, bWantResult);
    Py_DECREF(context);
    if (FAILED(hr))
        return hr;
    if (pvarResult) {
        if (!PyCom_VariantFromPyObject(result, pvarResult)) {
            PyCom_ExcepInfoFromPyException(pexcepinfo);
            hr = DISP_E_EXCEPTION;
        }
    }
    Py_DECREF(result);
    return hr;
}
