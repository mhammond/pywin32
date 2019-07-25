class PYAXSCRIPT_EXPORT PyIObjectSafety : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIObjectSafety, IID_IObjectSafety);
    static IObjectSafety *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *GetInterfaceSafetyOptions(PyObject *self, PyObject *args);
    static PyObject *SetInterfaceSafetyOptions(PyObject *self, PyObject *args);

   protected:
    PyIObjectSafety(IUnknown *pdisp);
    ~PyIObjectSafety();
};

class PYAXSCRIPT_EXPORT PyGObjectSafety : public PyGatewayBase, public IObjectSafety {
    PyGObjectSafety(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGObjectSafety, IObjectSafety, IID_IObjectSafety);
    // IObjectSafety
    STDMETHOD(GetInterfaceSafetyOptions)
    (
        /* [in] */ REFIID riid,
        /* [out] */ DWORD __RPC_FAR *pdwSupportedOptions,
        /* [out] */ DWORD __RPC_FAR *pdwEnabledOptions);

    STDMETHOD(SetInterfaceSafetyOptions)
    (
        /* [in] */ REFIID riid,
        /* [in] */ DWORD dwOptionSetMask,
        /* [in] */ DWORD dwEnabledOptions);
};
