// Interface Declaration

class PyIDebugDocumentInfo : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIDebugDocumentInfo, IID_IDebugDocumentInfo);
    static IDebugDocumentInfo *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *GetName(PyObject *self, PyObject *args);
    static PyObject *GetDocumentClassId(PyObject *self, PyObject *args);

   protected:
    PyIDebugDocumentInfo(IUnknown *pdisp);
    ~PyIDebugDocumentInfo();
};
// ---------------------------------------------------
//
// Gateway Declaration

class PyGDebugDocumentInfo : public PyGatewayBase, public IDebugDocumentInfo {
   protected:
    PyGDebugDocumentInfo(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGDebugDocumentInfo, IDebugDocumentInfo, IID_IDebugDocumentInfo)

    // IDebugDocumentInfo
    STDMETHOD(GetName)(DOCUMENTNAMETYPE dnt, BSTR *pbstrName);
    STDMETHOD(GetDocumentClassId)(CLSID *pclsidDocument);
};
