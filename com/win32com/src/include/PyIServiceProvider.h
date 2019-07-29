
/////////////////////////////////////////////////////////////////////////////
// class PyIServiceProvider
#ifndef NO_PYCOM_ISERVICEPROVIDER
class PYCOM_EXPORT PyIServiceProvider : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIServiceProvider);
    static PyComTypeObject type;
    static IServiceProvider *GetI(PyObject *self);

    static PyObject *QueryService(PyObject *self, PyObject *args);

   protected:
    PyIServiceProvider(IUnknown *);
    ~PyIServiceProvider();
};

class PyGServiceProvider : public PyGatewayBase, public IServiceProvider {
   protected:
    PyGServiceProvider(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGServiceProvider, IServiceProvider, IID_IServiceProvider)

    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv);
};
#endif  // NO_PYCOM_ISERVICEPROVIDER
