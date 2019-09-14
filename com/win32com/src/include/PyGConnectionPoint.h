class PyGConnectionPoint : public PyGatewayBase, public IConnectionPoint {
   protected:
    PyGConnectionPoint(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGConnectionPoint, IConnectionPoint, IID_IConnectionPoint)

    // IConnectionPoint
    STDMETHOD(GetConnectionInterface)(IID *pIID);

    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer **ppCPC);

    STDMETHOD(Advise)(IUnknown *pUnk, DWORD *pdwCookie);

    STDMETHOD(Unadvise)(DWORD cookie);

    STDMETHOD(EnumConnections)(IEnumConnections **ppEnum);
};
