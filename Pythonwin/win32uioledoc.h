// Delcaration of the Python support for a PyCOleDocument object
#pragma once
#include "win32doc.h"

class PyCOleDocument : public PyCDocument {
   protected:
   public:
    static COleDocument *GetDoc(PyObject *self);
    static PyObject *Create(PyObject *self, PyObject *args);

    MAKE_PY_CTOR(PyCOleDocument);
    static ui_type_CObject type;
};

class PyCOleClientItem : public PyCCmdTarget {
   protected:
    PyCOleClientItem() { ; }
    ~PyCOleClientItem() { ; }

   public:
    static COleClientItem *GetOleClientItem(PyObject *self);

    MAKE_PY_CTOR(PyCOleClientItem);
    static ui_type_CObject type;
};