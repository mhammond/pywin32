class PYAXSCRIPT_EXPORT PyGActiveScriptError : public PyGatewayBase, public IActiveScriptError
{
protected:
	PyGActiveScriptError(PyObject *instance) : PyGatewayBase(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT(PyGActiveScriptError, IActiveScriptError, IID_IActiveScriptError)

	// IActiveScriptError
	STDMETHOD(GetExceptionInfo)(EXCEPINFO __RPC_FAR *pexcepinfo);

	STDMETHOD(GetSourcePosition)(DWORD __RPC_FAR *pdwSourceContext, ULONG __RPC_FAR *pulLineNumber, LONG __RPC_FAR *plCharacterPosition);
		
	STDMETHOD(GetSourceLineText)(BSTR __RPC_FAR *pbstrSourceLine);
};
