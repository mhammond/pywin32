//
// Document Object
//

class PYW_EXPORT PyCDocument : public PyCCmdTarget {
   protected:
    PyCDocument();
    ~PyCDocument();

   public:
    static PyObject *create_edit(PyObject *self, PyObject *args);
    static PyObject *create(PyObject *self, PyObject *args);

    static CDocument *GetDoc(PyObject *self);
    static PyObject *ui_doc_create(PyObject *self, PyObject *args);

    MAKE_PY_CTOR(PyCDocument);
    static ui_type_CObject type;
};
