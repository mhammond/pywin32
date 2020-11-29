class PyGConnectionPointContainer : public PyGatewayBase, public IConnectionPointContainer {
   protected:
    PyGConnectionPointContainer(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGConnectionPointContainer, IConnectionPointContainer, IID_IConnectionPointContainer)

    // IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **);
    STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint **ppCP);
};
