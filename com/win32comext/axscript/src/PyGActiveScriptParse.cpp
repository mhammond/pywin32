// PyGActiveScriptParse.cpp

#include "stdafx.h"

extern HRESULT InvokeGatewayViaPolicy(
    PyGatewayBase *pGateway,
	const char *szMethodName,
	EXCEPINFO *pei,
	PyObject **ppResult /* = NULL */,
	const char *szFormat /* = NULL */,
	...);

	// IActiveScriptParse
STDMETHODIMP PyGActiveScriptParse::InitNew(void)
{
	PY_GATEWAY_METHOD;
	return InvokeViaPolicy(	"InitNew", NULL, NULL);
}
        
STDMETHODIMP PyGActiveScriptParse::AddScriptlet( 
            /* [in] */ LPCOLESTR pstrDefaultName,
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ LPCOLESTR pstrItemName,
            /* [in] */ LPCOLESTR pstrSubItemName,
            /* [in] */ LPCOLESTR pstrEventName,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwSourceContextCookie,
            /* [in] */ ULONG ulStartingLineNumber,
            /* [in] */ DWORD dwFlags,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo)
{
	PY_GATEWAY_METHOD;
	USES_CONVERSION;
	PyObject *result;
	HRESULT hr = InvokeGatewayViaPolicy(this, "AddScriptlet", pexcepinfo, &result,
                                                        "ssssssii",
                                                        OLE2CT(pstrDefaultName),
                                                        OLE2CT(pstrCode),
                                                        OLE2CT(pstrItemName),
                                                        OLE2CT(pstrSubItemName),
                                                        OLE2CT(pstrEventName),
                                                        OLE2CT(pstrDelimiter),
                                                        dwSourceContextCookie,
                                                        ulStartingLineNumber);
	if (FAILED(hr)) return hr;
	if (result && PyString_Check(result)) {
		*pbstrName = A2BSTR(PyString_AS_STRING((PyStringObject*)result));
	}
	Py_DECREF(result);
	return S_OK;
}
        
STDMETHODIMP PyGActiveScriptParse::ParseScriptText( 
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ LPCOLESTR pstrItemName,
            /* [in] */ IUnknown __RPC_FAR *punkContext,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwSourceContextCookie,
            /* [in] */ ULONG ulStartingLineNumber,
            /* [in] */ DWORD dwFlags,
            /* [out] */ VARIANT __RPC_FAR *pvarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo)
{
	PY_GATEWAY_METHOD;
	USES_CONVERSION;
	PyObject *context = PyCom_PyObjectFromIUnknown(punkContext, IID_IUnknown, TRUE);
	if (context==NULL) {
		PyCom_ExcepInfoFromPyException(pexcepinfo);
		return DISP_E_EXCEPTION;
	}
	PyObject *result = NULL;
	BOOL bWantResult = pvarResult!=NULL;
	
	HRESULT hr = InvokeGatewayViaPolicy(this, "ParseScriptText", pexcepinfo, &result,
											"ssOsiiii",
											OLE2CT(pstrCode),
											OLE2CT(pstrItemName),
											context,
											OLE2CT(pstrDelimiter),
											dwSourceContextCookie,
											ulStartingLineNumber,
											dwFlags,
											bWantResult);
	Py_DECREF(context);
	if (FAILED(hr)) return hr;
	if (pvarResult) {
		if (!PyCom_VariantFromPyObject(result, pvarResult)) {
			PyCom_ExcepInfoFromPyException(pexcepinfo);
			hr = DISP_E_EXCEPTION;
		}
	}
	Py_DECREF(result);
	return hr;
}
