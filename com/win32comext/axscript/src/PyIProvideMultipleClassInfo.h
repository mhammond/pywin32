class PYAXSCRIPT_EXPORT PyIProvideMultipleClassInfo : public PyIProvideClassInfo2 {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIProvideMultipleClassInfo, IID_IProvideMultipleClassInfo);
    static IProvideMultipleClassInfo *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *GetMultiTypeInfoCount(PyObject *self, PyObject *args);
    static PyObject *GetInfoOfIndex(PyObject *self, PyObject *args);

   protected:
    PyIProvideMultipleClassInfo(IUnknown *pdisp);
    ~PyIProvideMultipleClassInfo();
};
