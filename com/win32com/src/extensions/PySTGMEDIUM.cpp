// A Python object representing a windows STGMEDIUM structure.
#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"
// @doc This file contains autoduck documentation.
// @pymethod <o PySTGMEDIUM>|pythoncom|STGMEDIUM|Creates a new STGMEDIUM object
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

// @pymethod |PySTGMEDIUM|set|Sets the type and data of the object.
PyObject *PySet(PyObject *self, PyObject *args)
{
	int tymed;
	PyObject *ob;
	// @pyparm int|tymed||The type of the data
	// @pyparm object|data||
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
			// todo: support buffer (but see byte-count discussion below)
			if (!PyString_Check(ob))
				return PyErr_Format(PyExc_TypeError, "tymed value of %d requires a string", tymed);
			// We need to include the NULL, as the Windows clipboard functions
			// will assume it is there for text related formats (eg, CF_TEXT).
			// I can't see one extra byte could cause any problems - but if
			// in the future it does, we can take the win32clipboard route,
			// and only include the extra \0 for strings, allowing buffers to
			// use the exact cb.
			int cb = PyString_Size(ob)+1;
			ps->medium.hGlobal = GlobalAlloc(GMEM_FIXED, cb);
			if (!ps->medium.hGlobal)
				return PyErr_NoMemory();
			memcpy( (void *)ps->medium.hGlobal, PyString_AsString(ob), cb);
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

// @object PySTGMEDIUM|A STGMEDIUM object represents a COM STGMEDIUM structure.
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
	// @prop int|tymed|An integer indicating the type of data in the stgmedium
	if (strcmp(name, "tymed")==0)
		return PyInt_FromLong(ps->medium.tymed);
	// @prop object|data|The data in the stgmedium.  
	// The result depends on the value of the 'tymed' property of the <o PySTGMEDIUM> object.
	// @flagh tymed|Result Type
	if (strcmp(name, "data")==0) {
		switch (ps->medium.tymed) {
			// @flag TYMED_GDI|An integer GDI handle
			case TYMED_GDI:
				return PyLong_FromVoidPtr(ps->medium.hBitmap);
			// @flag TYMED_MFPICT|An integer METAFILE handle
			case TYMED_MFPICT:
				return PyLong_FromVoidPtr(ps->medium.hMetaFilePict);
			// @flag TYMED_ENHMF|An integer ENHMETAFILE handle
			case TYMED_ENHMF:
				return PyLong_FromVoidPtr(ps->medium.hEnhMetaFile);
			// @flag TYMED_HGLOBAL|A string with the bytes of the global memory object.
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
			// @flag TYMED_FILE|A string/unicode filename
			case TYMED_FILE: 
				return PyWinObject_FromWCHAR(ps->medium.lpszFileName);
			// @flag TYMED_ISTREAM|A <o PyIStream> object
			case TYMED_ISTREAM:
			// @flag TYMED_ISTORAGE|A <o PyIStorage> object
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
	// @prop int|data_handle|The raw 'integer' representation of the data.  
	// For TYMED_HGLOBAL, this is the handle rather than the string data.
	// For the string and interface types, this is an integer holding the pointer.
	if (strcmp(name, "data_handle")==0) {
		switch (ps->medium.tymed) {
			case TYMED_GDI:
				return PyLong_FromVoidPtr(ps->medium.hBitmap);
			case TYMED_MFPICT:
				return PyLong_FromVoidPtr(ps->medium.hMetaFilePict);
			case TYMED_ENHMF:
				return PyLong_FromVoidPtr(ps->medium.hEnhMetaFile);
			case TYMED_HGLOBAL:
				return PyLong_FromVoidPtr(ps->medium.hGlobal);
			// and may as well hand out the pointers for these.  
			// We are all consenting adults :)
			case TYMED_FILE: 
				return PyLong_FromVoidPtr(ps->medium.lpszFileName);
			case TYMED_ISTREAM:
				return PyLong_FromVoidPtr(ps->medium.pstm);
			case TYMED_ISTORAGE:
				return PyLong_FromVoidPtr(ps->medium.pstg);
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

