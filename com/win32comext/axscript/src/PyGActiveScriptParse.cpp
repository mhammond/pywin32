// PyGActiveScriptParse.cpp

#include "stdafx.h"

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
	HRESULT hr = InvokeViaPolicy("AddScriptlet", &result,
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
	return PyCom_HandlePythonFailureToCOM(pexcepinfo);
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
	PyObject *result = NULL;
	BOOL bWantResult = pvarResult!=NULL;
	
	HRESULT hr = PyCom_HandlePythonFailureToCOM(pexcepinfo);
	if (FAILED(hr)) return hr;
	hr = InvokeViaPolicy(			"ParseScriptText", &result,
											"ssOsiiii",
											OLE2CT(pstrCode),
											OLE2CT(pstrItemName),
											context,
											OLE2CT(pstrDelimiter),
											dwSourceContextCookie,
											ulStartingLineNumber,
											dwFlags,
											bWantResult);
	if (FAILED(hr)) return hr;
	if (pvarResult) PyCom_VariantFromPyObject(result, pvarResult);
	return PyCom_HandlePythonFailureToCOM(pexcepinfo);
}
