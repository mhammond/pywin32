#include "stdafx.h"
#include "PythonCOM.h"

PyIBase::PyIBase()
{
	_Py_NewReference(this);
}
PyIBase::~PyIBase()
{
}

/*static*/BOOL PyIBase::is_object(const PyObject *ob, PyComTypeObject *which)
{
	// First, is the object an instance of an interface type?
	if ( !PyComTypeObject::is_interface_type((PyObject *)ob->ob_type) )
		return FALSE;

	// now check for inheritance.
	PyComTypeObject *thisType = (PyComTypeObject *)ob->ob_type;
	while (thisType) {
		if (which==thisType)
			return TRUE;
		thisType = thisType->baseType;
	}
	return FALSE;
}
BOOL PyIBase::is_object(PyComTypeObject *which)
{
	return is_object(this,which);
}

/*static*/PyObject *
PyIBase::getattro(PyObject *self, PyObject *name)
{
	if (PyString_Check(name)) {
		PyObject *rc = ((PyIBase *)self)->getattr(PyString_AsString(name));
		if (rc)
			return rc;
		PyErr_Clear();
	}
	// Using PyObject_GenericGetAttr allows some special type magic
	// (ie, 
#ifdef OLD_PYTHON_TYPES
	PyErr_SetObject(PyExc_AttributeError, name);
	return NULL;
#else
	return PyObject_GenericGetAttr(self, name);
#endif
}

PyObject *
PyIBase::getattr(char *name)
{
	return Py_FindMethodInChain(&((PyComTypeObject *)ob_type)->chain, this, name);
}
PyObject *
PyIBase::iternext()
{
	return PyErr_Format(PyExc_RuntimeError,
			"iternext must be overridden by objects supporting enumeration (type '%s').", ob_type->tp_name);
}

/*static*/int PyIBase::setattr(PyObject *op, char *name, PyObject *v)
{
	PyIBase* bc = (PyIBase *)op;
	return bc->setattr(name,v);
}
int PyIBase::setattr(char *name, PyObject *v)
{
	char buf[128];
	sprintf(buf, "%s has read-only attributes", ob_type->tp_name );
	PyErr_SetString(PyExc_TypeError, buf);
	return -1;
}

/*static*/ PyObject *
PyIBase::repr( PyObject *ob )
{
	return ((PyIBase *)ob)->repr();
}
PyObject * PyIBase::repr()
{
	TCHAR buf[80];
	wsprintf(buf, _T("<%hs at %p>"),ob_type->tp_name, (PyObject *)this);
	return PyString_FromTCHAR(buf);
}

/*static*/ void PyIBase::dealloc(PyObject *ob)
{
	delete (PyIBase *)ob;
}

/*static*/ int PyIBase::cmp(PyObject *ob1, PyObject *ob2)
{
	return ((PyIBase *)ob1)->compare(ob2);
}

/*static*/ PyObject *PyIBase::iter(PyObject *self)
{
	return ((PyIBase *)self)->iter();
}

/*static*/ PyObject *PyIBase::iternext(PyObject *self)
{
	return ((PyIBase *)self)->iternext();
}
