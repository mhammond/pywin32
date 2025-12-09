// Interface Declaration

class PyIDebugStackFrameSnifferEx : public PyIDebugStackFrameSniffer {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIDebugStackFrameSnifferEx, IID_IDebugStackFrameSnifferEx);
    static IDebugStackFrameSnifferEx *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *EnumStackFramesEx(PyObject *self, PyObject *args);

   protected:
    PyIDebugStackFrameSnifferEx(IUnknown *pdisp);
    ~PyIDebugStackFrameSnifferEx();
};
// ---------------------------------------------------
//
// Gateway Declaration

class PyGDebugStackFrameSnifferEx : public PyGDebugStackFrameSniffer, public IDebugStackFrameSnifferEx {
   protected:
    PyGDebugStackFrameSnifferEx(PyObject *instance) : PyGDebugStackFrameSniffer(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT2(PyGDebugStackFrameSnifferEx, IDebugStackFrameSnifferEx, IID_IDebugStackFrameSnifferEx,
                            PyGDebugStackFrameSniffer)

    // IDebugStackFrameSniffer
    STDMETHOD(EnumStackFrames)(IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf)
    {
        return PyGDebugStackFrameSniffer::EnumStackFrames(ppedsf);
    }

    // IDebugStackFrameSnifferEx
#ifdef _WIN64
    STDMETHOD(EnumStackFramesEx64)(DWORDLONG dwSpMin, IEnumDebugStackFrames64 __RPC_FAR *__RPC_FAR *ppedsf);
#else
    STDMETHOD(EnumStackFramesEx)(DWORD dwSpMin, IEnumDebugStackFrames __RPC_FAR *__RPC_FAR *ppedsf);
#endif
};
