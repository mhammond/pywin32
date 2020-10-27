class PyIConnectionPointContainer : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIConnectionPointContainer);
    static IConnectionPointContainer *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *EnumConnectionPoints(PyObject *self, PyObject *args);
    static PyObject *FindConnectionPoint(PyObject *self, PyObject *args);

   protected:
    PyIConnectionPointContainer(IUnknown *pdisp);
    ~PyIConnectionPointContainer();
};
