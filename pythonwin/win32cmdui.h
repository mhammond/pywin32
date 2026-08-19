// CCmdUI definition.
//
class PyCCmdUI : public ui_assoc_object {
   public:
    MAKE_PY_CTOR(PyCCmdUI);
    static ui_type type;
    static CCmdUI *GetCCmdUIPtr(PyObject *self);

   protected:
    PyCCmdUI();
    virtual ~PyCCmdUI();
    virtual PyObject *getattro(PyObject *obname);
};
