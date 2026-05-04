/////////////////////////////////////////////////////////////////////////////
// class PyICatInformation
class PyICatInformation : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyICatInformation);
    static PyComTypeObject type;
    static ICatInformation *GetI(PyObject *self);

    static PyObject *EnumCategories(PyObject *self, PyObject *args);
    static PyObject *GetCategoryDesc(PyObject *self, PyObject *args);
    static PyObject *EnumClassesOfCategories(PyObject *self, PyObject *args);

   protected:
    PyICatInformation(IUnknown *);
    ~PyICatInformation();
};
