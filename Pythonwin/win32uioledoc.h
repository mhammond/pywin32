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
