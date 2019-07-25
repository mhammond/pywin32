// win32control.h : header file
//
//
/////////////////////////////////////////////////////////
//
// A derived type object. for the CCtrlView based  objects.
//
class PYW_EXPORT PyCCtrlView_Type : public ui_type_CObject {
   public:
    PyCCtrlView_Type(const char *name, ui_type *pBaseType, ui_type_CObject *pControlType, CRuntimeClass *rtClass,
                     int typeSize, int pyobjOffset, struct PyMethodDef *methodList, ui_base_class *(*thector)());

   public:
    ui_type_CObject *control;
};

////////////////////////////////////////////////////////////////////////
// View Classes
//
inline PyCCtrlView_Type::PyCCtrlView_Type(const char *name, ui_type *pBaseType, ui_type_CObject *pControlType,
                                          CRuntimeClass *pRT, int typeSize, int pyobjOffset,
                                          struct PyMethodDef *methodList, ui_base_class *(*thector)())
    : ui_type_CObject(name, pBaseType, pRT, typeSize, pyobjOffset, methodList, thector)
{
    control = pControlType;
    /* Some types also inherit from the control type (if different from itself).
        tp_base will already have been set in ui_type_CObject constructor.
    */
    if (pControlType != this)
        tp_bases = Py_BuildValue("OO", pBaseType, pControlType);
}

class PYW_EXPORT PyCCtrlView : public PyCView {
   public:
    MAKE_PY_CTOR(PyCCtrlView)
    static PyCCtrlView_Type type;
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
    PyCCtrlView() { return; }
};

class PYW_EXPORT PyCEditView : public PyCCtrlView {
   public:
    MAKE_PY_CTOR(PyCEditView)
    static PyCCtrlView_Type type;
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
    PyCEditView() { return; }
};

class PYW_EXPORT PyCListView : public PyCCtrlView {
   public:
    MAKE_PY_CTOR(PyCListView)
    static PyCCtrlView_Type type;
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
    PyCListView() { return; }
};

class PYW_EXPORT PyCTreeView : public PyCCtrlView {
   public:
    MAKE_PY_CTOR(PyCTreeView)
    static PyCCtrlView_Type type;
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
    PyCTreeView() { return; }
};

///////////////////////////////////////////////////////////////////////
// Control objects.
//
// ui_control_object
//
class ui_control_object : public PyCWnd {
   public:
    static ui_type_CObject type;

   protected:
    ui_control_object();
    virtual ~ui_control_object();
};

/////////////////////////////////////////////////////////
//
//	PyCButton
class PyCButton : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCButton)
   protected:
    PyCButton();
    virtual ~PyCButton();
};

/////////////////////////////////////////////////////////
//
//	PyCRichEditCtrl
class PyCRichEditCtrl : public ui_control_object {
   public:
    static PyCCtrlView_Type type;
    MAKE_PY_CTOR(PyCRichEditCtrl)
   protected:
    PyCRichEditCtrl();
    virtual ~PyCRichEditCtrl();
};

/////////////////////////////////////////////////////////
//
//	PyCListBox
class PyCListBox : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCListBox)
   protected:
    PyCListBox();
    virtual ~PyCListBox();
};

/////////////////////////////////////////////////////////
//
//	PyCComboBox
class PyCComboBox : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCComboBox)
   protected:
    PyCComboBox();
    virtual ~PyCComboBox();
};

/////////////////////////////////////////////////////////
//
//	PyCEdit
class PyCEdit : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCEdit)
   protected:
    PyCEdit();
    virtual ~PyCEdit();
};

/////////////////////////////////////////////////////////
//
//	PyCProgressCtrl
class PyCProgressCtrl : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCProgressCtrl)
   protected:
    PyCProgressCtrl();
    virtual ~PyCProgressCtrl();
};

/////////////////////////////////////////////////////////
//
//	PyCStatusBarCtrl
class PyCStatusBarCtrl : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCStatusBarCtrl)
   protected:
    PyCStatusBarCtrl();
    virtual ~PyCStatusBarCtrl();
};

/////////////////////////////////////////////////////////
//
//	PyCSliderCtrl
class PyCSliderCtrl : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCSliderCtrl)
   protected:
    PyCSliderCtrl();
    virtual ~PyCSliderCtrl();
};

/////////////////////////////////////////////////////////
//
//	PyCSpinButtonCtrl
class PyCSpinButtonCtrl : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCSpinButtonCtrl)
   protected:
};

/////////////////////////////////////////////////////////
//
//	PyCToolTipCtrl
class PyCToolTipCtrl : public ui_control_object {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCToolTipCtrl)
   protected:
    PyCToolTipCtrl();
    virtual ~PyCToolTipCtrl();
};

/////////////////////////////////////////////////////////////////////////////
