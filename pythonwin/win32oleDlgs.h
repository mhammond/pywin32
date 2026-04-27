
class PyCOleDialog : public PyCCommonDialog {
   public:
    static ui_type_CObject type;
};

class PyCOleInsertDialog : public PyCOleDialog {
   public:
    static PyObject *create(PyObject *self, PyObject *args);
    MAKE_PY_CTOR(PyCOleInsertDialog);
    static ui_type_CObject type;
};
