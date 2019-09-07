#include "intshcut.h"
class PyIUniformResourceLocator : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIUniformResourceLocator);
    static IUniformResourceLocator *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *GetURL(PyObject *self, PyObject *args);
    static PyObject *SetURL(PyObject *self, PyObject *args);
    static PyObject *InvokeCommand(PyObject *self, PyObject *args);

   protected:
    PyIUniformResourceLocator(IUnknown *pdisp);
    ~PyIUniformResourceLocator();
};
