// -*- Mode: C++; tab-width: 4 -*-
// font class

class PyCBrush : public PyCGdiObject {
public:
  static ui_type_CObject type;
  MAKE_PY_CTOR(PyCBrush)
  CBrush *GetBrush() {return GetBrush(this);}
  static CBrush *GetBrush(PyObject *self);

  static PyObject *create (PyObject *self, PyObject *args);
protected:
private:
};
