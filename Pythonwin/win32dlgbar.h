// CDialogBar support for Pythonwin

class PyCDialogBar : public PyCControlBar
{
public:
  MAKE_PY_CTOR(PyCDialogBar)
  static ui_type_CObject type;
  static PyObject *create (PyObject *self, PyObject *args);
  static CDialogBar *GetDialogBar (PyObject *self);

protected:
  // virtual CString repr();  maybe add later to show id?

private:
};
