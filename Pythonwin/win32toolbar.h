// long awaited toolbar class

class PyCControlBar : public PyCWnd
{
public:
  MAKE_PY_CTOR(PyCControlBar)
  static ui_type_CObject type;
  static CControlBar *GetControlBar (PyObject *self);
  virtual PyObject *getattr(char *name);
  virtual int setattr(char *name, PyObject *v);
  static PyObject *create (PyObject *self, PyObject *args);

protected:
  // virtual CString repr();  maybe add later to show id?
private:
};

class PyCToolBar : public PyCControlBar
{
public:
  MAKE_PY_CTOR(PyCToolBar)
  static ui_type_CObject type;
  static PyObject *create (PyObject *self, PyObject *args);
  static CToolBar *GetToolBar (PyObject *self);
};

class PyCStatusBar : public PyCControlBar
{
public:
  MAKE_PY_CTOR(PyCStatusBar)
  static ui_type_CObject type;
  static PyObject *create (PyObject *self, PyObject *args);
  static CStatusBar *GetStatusBar (PyObject *self);
};

class PyCToolBarCtrl : public PyCWnd
{
public:
  MAKE_PY_CTOR(PyCToolBarCtrl)
  static ui_type_CObject type;
  CPtrArray *bmplist;
  CPtrArray *strlist;
protected:
  PyCToolBarCtrl();
  ~PyCToolBarCtrl();
private:
};