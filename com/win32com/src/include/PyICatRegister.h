#ifndef NO_PYCOM_ICATREGISTER
class PyICatRegister : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyICatRegister);
    static ICatRegister *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *RegisterCategories(PyObject *self, PyObject *args);
    static PyObject *UnRegisterCategories(PyObject *self, PyObject *args);
    static PyObject *RegisterClassImplCategories(PyObject *self, PyObject *args);
    static PyObject *UnRegisterClassImplCategories(PyObject *self, PyObject *args);
    static PyObject *RegisterClassReqCategories(PyObject *self, PyObject *args);
    static PyObject *UnRegisterClassReqCategories(PyObject *self, PyObject *args);

   protected:
    PyICatRegister(IUnknown *pdisp);
    ~PyICatRegister();
};
#endif  // NO_PYCOM_ICATREGISTER
