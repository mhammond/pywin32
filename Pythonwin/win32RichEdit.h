// win32RichText.h : header file
//
//
/////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
// PyCRichEditDoc object
class PYW_EXPORT PyCRichEditDoc : public PyCDocument {
   protected:
   public:
    MAKE_PY_CTOR(PyCRichEditDoc);
    static ui_type_CObject type;
};

//	PyCRichEditView
class PYW_EXPORT PyCRichEditView : public PyCCtrlView {
   public:
    MAKE_PY_CTOR(PyCRichEditView)
    static PyCCtrlView_Type type;
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
    PyCRichEditView() { return; }
};
