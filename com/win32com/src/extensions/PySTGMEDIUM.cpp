
#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"

// @pymethod <o STGMEDIUM>|pythoncom|STGMEDIUM|Creates a new STGMEDIUM object
PyObject *Py_NewSTGMEDIUM(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return new PySTGMEDIUM();
}

PySTGMEDIUM *PyObject_FromSTGMEDIUM(STGMEDIUM *desc /* = NULL*/)
{
	return new PySTGMEDIUM(desc);
}

PyObject *PySet(PyObject *self, PyObject *args)
{
	int tymed;
	PyObject *ob;
	if (!PyArg_ParseTuple(args, "iO:set", &tymed, &ob))
		return NULL;

	PySTGMEDIUM *ps = (PySTGMEDIUM *)self;
	ps->Close(); // ensure any old data clean
	switch (tymed) {
		case TYMED_GDI:
			if (!PyInt_Check(ob) || !PyLong_Check(ob))
				return PyErr_Format(PyExc_TypeError, "tymed value of %d requires an integer handle", tymed);
			ps->medium.hBitmap = (HBITMAP)PyInt_AsLong(ob);
			break;
		case TYMED_MFPICT:
			if (!PyInt_Check(ob) || !PyLong_Check(ob))
				return PyErr_Format(PyExc_TypeError, "tymed value of %d requires an integer handle", tymed);
			ps->medium.hMetaFilePict = (HMETAFILEPICT)PyInt_AsLong(ob);
			break;
		case TYMED_ENHMF:
			if (!PyInt_Check(ob) || !PyLong_Check(ob))
				return PyErr_Format(PyExc_TypeError, "tymed value of %d requires an integer handle", tymed);
			ps->medium.hEnhMetaFile = (HENHMETAFILE)PyInt_AsLong(ob);
			break;
		case TYMED_HGLOBAL: {
			// todo: support buffer
			if (!PyString_Check(ob))
				return PyErr_Format(PyExc_TypeError, "tymed value of %d requires a string", tymed);
			ps->medium.hGlobal = GlobalAlloc(GMEM_FIXED, PyString_Size(ob));
			if (!ps->medium.hGlobal)
				return PyErr_NoMemory();
			memcpy( (void *)ps->medium.hGlobal, PyString_AsString(ob), PyString_Size(ob));
			break;
		}
		case TYMED_FILE: 
			if (!PyWinObject_AsTaskAllocatedWCHAR(ob, &ps->medium.lpszFileName, FALSE, NULL))
				return FALSE;
			break;
		case TYMED_ISTREAM:
			if (!PyCom_InterfaceFromPyInstanceOrObject(ob, IID_IStream, (void **)&ps->medium.pstm, FALSE/* bNoneOK */))
				return FALSE;
			break;
		case TYMED_ISTORAGE:
			if (!PyCom_InterfaceFromPyInstanceOrObject(ob, IID_IStorage, (void **)&ps->medium.pstg, FALSE/* bNoneOK */))
				return FALSE;
			break;
		default:
			PyErr_Format(PyExc_ValueError, "Unknown tymed value '%d'", tymed);
			return NULL;
	}
	ps->medium.tymed = tymed;
	Py_INCREF(Py_None);
	return Py_None;
}

// @object STGMEDIUM|A STGMEDIUM object represents a COM STGMEDIUM structure.
static struct PyMethodDef PySTGMEDIUM_methods[] = {
	{"set", PySet, 1}, // @pymeth set|Sets the type and data of the object
	{NULL}
};

PyTypeObject PySTGMEDIUM::Type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySTGMEDIUM",
	sizeof(PySTGMEDIUM),
	0,
	PySTGMEDIUM::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySTGMEDIUM::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PySTGMEDIUM, e)

PySTGMEDIUM::PySTGMEDIUM(STGMEDIUM *pm)
{
	ob_type = &PySTGMEDIUM::Type;
	_Py_NewReference(this);
	if (pm)
		memcpy(&medium, pm, sizeof(medium));
	else
		memset(&medium, 0, sizeof(medium));
}

PySTGMEDIUM::~PySTGMEDIUM()
{
	Close();
}

void PySTGMEDIUM::DropOwnership()
{
	memset(&medium, 0, sizeof(medium));
}

void PySTGMEDIUM::Close()
{
	if (medium.tymed) {
		ReleaseStgMedium(&medium);
		memset(&medium, 0, sizeof(medium));
		assert(!medium.tymed);
	}
}

PyObject *PySTGMEDIUM::getattr(PyObject *self, char *name)
{
	PyObject *res;
	PySTGMEDIUM *ps = (PySTGMEDIUM *)self;
	res = Py_FindMethod(PySTGMEDIUM_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	if (strcmp(name, "tymed")==0)
		return PyInt_FromLong(ps->medium.tymed);
	if (strcmp(name, "data")==0) {
		switch (ps->medium.tymed) {
			case TYMED_GDI:
				return PyLong_FromVoidPtr(ps->medium.hBitmap);
			case TYMED_MFPICT:
				return PyLong_FromVoidPtr(ps->medium.hMetaFilePict);
			case TYMED_ENHMF:
				return PyLong_FromVoidPtr(ps->medium.hEnhMetaFile);
			case TYMED_HGLOBAL: {
				PyObject *ret;
				void *p = GlobalLock(ps->medium.hGlobal);
				if (p) {
					ret = PyString_FromStringAndSize( (char *)p, GlobalSize(ps->medium.hGlobal));
					GlobalUnlock(ps->medium.hGlobal);
				} else {
					ret = Py_None;
					Py_INCREF(Py_None);
				}
				return ret;
			}
			case TYMED_FILE: 
				return PyWinObject_FromWCHAR(ps->medium.lpszFileName);
			case TYMED_ISTREAM:
				return PyCom_PyObjectFromIUnknown(ps->medium.pstm, IID_IStream, TRUE);
			case TYMED_ISTORAGE:
				return PyCom_PyObjectFromIUnknown(ps->medium.pstg, IID_IStorage, TRUE);
			case TYMED_NULL:
				PyErr_SetString(PyExc_ValueError, "This STGMEDIUM has no data");
				return NULL;
			default:
				PyErr_SetString(PyExc_RuntimeError, "Unknown tymed");
				return NULL;
		}
	}
	return PyErr_Format(PyExc_AttributeError, "STGMEDIUM objects have no attribute '%s'", name);
}

/*static*/ void PySTGMEDIUM::deallocFunc(PyObject *ob)
{
	delete (PySTGMEDIUM *)ob;
}

