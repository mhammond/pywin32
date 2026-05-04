/////////////////////////////////////////////////////////
//
//	hierlist

// fwd declare
class CPythonHierControl;
class CPythonHierListItem;
/////////////////////////

class ui_hierlist_object : public PyCWnd {
   public:
    static PyObject *create(PyObject *self, PyObject *args);
    CPythonHierControl *GetListObject();
    static CPythonHierControl *GetListObject(PyObject *self);

    static ui_type_CObject type;
    MAKE_PY_CTOR(ui_hierlist_object)
   protected:
    ui_hierlist_object();
    virtual ~ui_hierlist_object();
};
class ui_hierlist_item : public ui_assoc_object {
   public:
    static CPythonHierListItem *GetHLI(PyObject *self);

   protected:
    ui_hierlist_item() { return; }
    virtual ~ui_hierlist_item() { return; }

   public:
    static ui_type type;
    MAKE_PY_CTOR(ui_hierlist_item)
};
