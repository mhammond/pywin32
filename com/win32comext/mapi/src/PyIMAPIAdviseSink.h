// This file declares the IMAPIAdviseSink Gateway for Python.
// ----------------------------------------------------------
//
// Interface Declaration

class PyIMAPIAdviseSink : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIMAPIAdviseSink);
    static IMAPIAdviseSink *GetI(PyObject *self);
    static PyComTypeObject type;

   protected:
    PyIMAPIAdviseSink(IUnknown *pdisp);
    ~PyIMAPIAdviseSink();
};

// Gateway Declaration

class PyGMAPIAdviseSink : public PyGatewayBase, public IMAPIAdviseSink {
   protected:
    PyGMAPIAdviseSink(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT2(PyGMAPIAdviseSink, IMAPIAdviseSink, IID_IMAPIAdviseSink, PyGatewayBase)

    // IMAPIAdviseSink
    MAPIMETHOD_(ULONG, OnNotify)(ULONG cNotif, LPNOTIFICATION lpNotifications);
};

// Used by HrAllocAdviseSink
class PyCMAPIAdviseSink : public IMAPIAdviseSink {
   public:
    PyCMAPIAdviseSink(PyObject *callback, PyObject *context);
    virtual ~PyCMAPIAdviseSink();

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP_(ULONG) OnNotify(ULONG cNotify, LPNOTIFICATION lpNotifications);

   private:
    LONG m_cRef;
    PyObject *m_callback;
    PyObject *m_context;
};
