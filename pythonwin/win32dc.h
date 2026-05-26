// device context class
class PYW_EXPORT ui_dc_object : public ui_assoc_CObject {
   protected:
    ui_dc_object() : m_deleteDC(FALSE) {}
    ~ui_dc_object();
    virtual void SetAssocInvalid();

   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(ui_dc_object)
    static CDC *GetDC(PyObject *self);

    // couple of python methods
    static PyObject *create_dc(PyObject *self, PyObject *args);
    static PyObject *create_compatible_dc(PyObject *self, PyObject *args);
    static PyObject *create_printer_dc(PyObject *self, PyObject *args);
    BOOL m_deleteDC;
};
