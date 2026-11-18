// menu class
class PyCMenu : public ui_assoc_object {
   protected:
    PyCMenu::PyCMenu() { return; }
    virtual void SetAssocInvalid();
    virtual bool CheckCppObject(ui_type *ui_type_check) const;

   public:
    static ui_type type;
    MAKE_PY_CTOR(PyCMenu)
    static HMENU GetMenu(PyObject *self);

    static PyObject *create_popup(PyObject *self, PyObject *args);
    static PyObject *create_menu(PyObject *self, PyObject *args);
    static PyObject *load_menu(PyObject *self, PyObject *args);

    static PyObject *AppendMenu(PyObject *self, PyObject *args);
    static PyObject *DeleteMenu(PyObject *self, PyObject *args);
    static PyObject *InsertMenu(PyObject *self, PyObject *args);
    static PyObject *ModifyMenu(PyObject *self, PyObject *args);
    static PyObject *EnableMenuItem(PyObject *self, PyObject *args);
    static PyObject *GetHandle(PyObject *self, PyObject *args);
    static PyObject *GetMenuItemCount(PyObject *self, PyObject *args);
    static PyObject *GetMenuItemID(PyObject *self, PyObject *args);
    static PyObject *GetMenuString(PyObject *self, PyObject *args);
    static PyObject *GetSubMenu(PyObject *self, PyObject *args);
    static PyObject *TrackPopupMenu(PyObject *self, PyObject *args);
};
