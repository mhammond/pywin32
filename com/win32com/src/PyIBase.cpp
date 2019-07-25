#include "stdafx.h"
#include "PythonCOM.h"

PyIBase::PyIBase() { _Py_NewReference(this); }
PyIBase::~PyIBase() {}

/*static*/ BOOL PyIBase::is_object(PyObject *ob, PyComTypeObject *which)
{
    return PyObject_IsInstance(ob, (PyObject *)which);
}

BOOL PyIBase::is_object(PyComTypeObject *which) { return is_object(this, which); }

/*static*/ PyObject *PyIBase::getattro(PyObject *self, PyObject *name)
{
    // Using PyObject_GenericGetAttr allows some special type magic
    // (ie,
    return PyObject_GenericGetAttr(self, name);
}

PyObject *PyIBase::getattr(char *name) { return PyObject_GetAttrString(this, name); }

/*static*/ int PyIBase::setattro(PyObject *op, PyObject *obname, PyObject *v)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    PyIBase *bc = (PyIBase *)op;
    return bc->setattr(name, v);
}

int PyIBase::setattr(char *name, PyObject *v)
{
    char buf[128];
    sprintf(buf, "%s has read-only attributes", ob_type->tp_name);
    PyErr_SetString(PyExc_TypeError, buf);
    return -1;
}

/*static*/ PyObject *PyIBase::repr(PyObject *ob) { return ((PyIBase *)ob)->repr(); }
PyObject *PyIBase::repr()
{
    TCHAR buf[80];
    wsprintf(buf, _T("<%hs at %p>"), ob_type->tp_name, (PyObject *)this);
    return PyWinCoreString_FromString(buf);
}

/*static*/ void PyIBase::dealloc(PyObject *ob) { delete (PyIBase *)ob; }

/*static*/ int PyIBase::cmp(PyObject *ob1, PyObject *ob2) { return ((PyIBase *)ob1)->compare(ob2); }

/*static*/ PyObject *PyIBase::richcmp(PyObject *ob1, PyObject *ob2, int op)
{
    // our 'compare' implementations don't assume ob2 is our type, so
    // no additional checks are needed.
    int c = cmp(ob1, ob2);
    // BUT - it doesn't propogate exceptions correctly.
    if (c == -1 && PyErr_Occurred()) {
        // if the error related to the type of the object,
        // rich-compare wants Py_NotImplemented returned.
        if (PyErr_ExceptionMatches(PyExc_TypeError)) {
            PyErr_Clear();
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        return NULL;
    }
    assert(!PyErr_Occurred());  // should always have returned -1 on error.
    BOOL ret;
    if (op == Py_EQ)
        ret = c == 0;
    else if (op == Py_NE)
        ret = c != 0;
    else {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    return PyBool_FromLong(ret);
}
