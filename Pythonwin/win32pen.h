// -*- Mode: C++; tab-width: 4 -*-
// font class

class ui_pen_object : public PyCGdiObject {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(ui_pen_object)
    static PyObject *create(PyObject *self, PyObject *args);

   protected:
   private:
};
