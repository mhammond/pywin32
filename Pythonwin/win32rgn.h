#ifndef INC_PYCRGN
#define INC_PYCRGN

#include "win32gdi.h"

class PyCRgn : public PyCGdiObject 
	{
	public:
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCRgn)
	CRgn *GetRgn() {return GetRgn(this);}
	static CRgn *GetRgn(PyObject *self);

	static PyObject *create(PyObject *self, PyObject *args);
	static PyObject *create_elliptic_rgn(PyObject *self, PyObject *args);

	static PyObject *create_rect_rgn(PyObject *self, PyObject *args);
	static PyObject *combine_rgn(PyObject *self, PyObject *args);
	static PyObject *copy_rgn(PyObject *self, PyObject *args);
	static PyObject *get_rgn_box(PyObject *self, PyObject *args);
	static PyObject *delete_object(PyObject *self, PyObject *args);
	static PyObject *get_safe_handle(PyObject *self, PyObject *args);

	};

#endif
