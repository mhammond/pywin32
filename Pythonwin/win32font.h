// font class

class PyCFont : public PyCGdiObject {

public:
  MAKE_PY_CTOR(PyCFont)
  static ui_type_CObject type;
  static PyObject *create (PyObject *self, PyObject *args);

protected:
private:
};
