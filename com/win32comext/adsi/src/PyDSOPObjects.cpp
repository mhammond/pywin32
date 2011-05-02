// Directory Service Object Picker objects.
// @doc
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#include "PyWinTypes.h"
#include "PythonCOM.h"
#include "Objsel.h"

// DS_SELECTION_LIST helpers
PyObject *PyStringAsDS_SELECTION_LIST(PyObject *self, PyObject *args)
{
	char *sz;
	unsigned int cb;
	if (!PyArg_ParseTuple(args, "s#:PyStringAsDS_SELECTION_LIST", &sz, &cb))
		return NULL;
	if (cb < sizeof(DS_SELECTION_LIST))
		return PyErr_Format(PyExc_ValueError,
							"String must be at least %d bytes (got %d)",
							sizeof(DS_SELECTION_LIST), cb);
	DS_SELECTION_LIST *pSL = (DS_SELECTION_LIST *)sz;
	PyObject *ret = PyList_New(pSL->cItems);
	if (!ret)
		return NULL;
	for (unsigned int i=0;i<pSL->cItems;i++) {
		// get attrs for this item
		DS_SELECTION *pItem = pSL->aDsSelection+i;
		PyObject *obAttr;
		if (pItem->pvarFetchedAttributes) {
			obAttr = PyList_New(pSL->cFetchedAttributes);
			if (!obAttr) {
				Py_DECREF(ret);
				return NULL;
			}
			for (unsigned int ia=0;ia<pSL->cFetchedAttributes;ia++)
				PyList_SET_ITEM(obAttr, ia,
								 PyCom_PyObjectFromVariant(pItem->pvarFetchedAttributes+ia));
		}
		else {
			obAttr = Py_None;
			Py_INCREF(Py_None);
		}
		PyObject *sub = Py_BuildValue("NNNNNl",
									  PyWinObject_FromWCHAR(pItem->pwzName),
									  PyWinObject_FromWCHAR(pItem->pwzADsPath),
									  PyWinObject_FromWCHAR(pItem->pwzClass),
									  PyWinObject_FromWCHAR(pItem->pwzUPN),
									  obAttr,
									  pItem->flScopeType);
		if (!sub) {
			Py_DECREF(ret);
			return NULL;
		}
		PyList_SET_ITEM(ret, i, sub);
	}
	return ret;
}

class PyDSOP_SCOPE_INIT_INFOs : public PyObject
{
public:
	PyDSOP_SCOPE_INIT_INFOs(DSOP_SCOPE_INIT_INFO *_scopes, int _count);
	PyDSOP_SCOPE_INIT_INFOs();
	~PyDSOP_SCOPE_INIT_INFOs();
	static void deallocFunc(PyObject *ob);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	static PySequenceMethods sequencemethods;

	DSOP_SCOPE_INIT_INFO *pScopes;
	int count;
};

class PyDSOP_SCOPE_INIT_INFO : public PyObject
{
public:
	PyDSOP_SCOPE_INIT_INFO(PyDSOP_SCOPE_INIT_INFOs *owner, int index);
	~PyDSOP_SCOPE_INIT_INFO();
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
public:
	PyDSOP_SCOPE_INIT_INFOs *owner;
	int index; // my pos in the owner's array
};

class PyDSOP_FILTER_FLAGS : public PyObject
{
public:
	PyDSOP_FILTER_FLAGS(PyDSOP_SCOPE_INIT_INFO *owner);
	~PyDSOP_FILTER_FLAGS();
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
protected:
	PyDSOP_SCOPE_INIT_INFO *owner;
};

class PyDSOP_UPLEVEL_FILTER_FLAGS : public PyObject
{
public:
	PyDSOP_UPLEVEL_FILTER_FLAGS( PyDSOP_SCOPE_INIT_INFO *owner);
	~PyDSOP_UPLEVEL_FILTER_FLAGS();
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
protected:
	PyDSOP_SCOPE_INIT_INFO *owner;
};

////////////////////////////////////////////////////////////////////////////
// PyDSOP_SCOPE_INIT_INFOs
////////////////////////////////////////////////////////////////////////////

// @object PyDSOP_SCOPE_INIT_INFOs|An object representing an array of <o PyDSOP_SCOPE_INIT_INFO> objects
// @comm You must pass the number of items in the array to the constructor.
// Once set, this can not be changed.  You can index the index (eg, ob[2]).  The
// object has no other (interesting) methods or attributes.
// <nl>These objects are created via <om adsi.DSOP_SCOPE_INIT_INFOs>(size)
Py_ssize_t PyDSOP_SCOPE_INIT_INFOs_sq_length(PyObject *self)
{
	PyDSOP_SCOPE_INIT_INFOs *p = (PyDSOP_SCOPE_INIT_INFOs *)self;
	return p->count;
}

PyObject *PyDSOP_SCOPE_INIT_INFOs_sq_item(PyObject *self, Py_ssize_t i)
{
	PyDSOP_SCOPE_INIT_INFOs *p =(PyDSOP_SCOPE_INIT_INFOs *)self;
	if (i>=p->count){
		PyErr_SetString(PyExc_IndexError,"Index specified larger than number of allocated buffers");
		return NULL;
		}
	return new PyDSOP_SCOPE_INIT_INFO(p, i);
}


PySequenceMethods PyDSOP_SCOPE_INIT_INFOs_sequencemethods=
{
	PyDSOP_SCOPE_INIT_INFOs_sq_length,		// inquiry sq_length;
	NULL,							// binaryfunc sq_concat;
	NULL,							// intargfunc sq_repeat;
	PyDSOP_SCOPE_INIT_INFOs_sq_item,		// intargfunc sq_item;
	NULL,							// intintargfunc sq_slice;
	NULL,							// intobjargproc sq_ass_item;;
	NULL,							// intintobjargproc sq_ass_slice;
	NULL,							// objobjproc sq_contains;
	NULL,							// binaryfunc sq_inplace_concat;
	NULL							// intargfunc sq_inplace_repeat;
};

PyTypeObject PyDSOP_SCOPE_INIT_INFOsType =
{
	PYWIN_OBJECT_HEAD
	"PyDSOP_SCOPE_INIT_INFOs",
	sizeof(PyDSOP_SCOPE_INIT_INFOs),
	0,
	PyDSOP_SCOPE_INIT_INFOs::deallocFunc,  // tp_dealloc
	0,			// tp_print
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// PyNumberMethods *tp_as_number
	&PyDSOP_SCOPE_INIT_INFOs_sequencemethods, // PySequenceMethods *tp_as_sequence
	0,			// PyMappingMethods *tp_as_mapping
	0,			// hashfunc tp_hash
	0,			// tp_call
	0,			// tp_str
	PyObject_GenericGetAttr, // tp_getattro
	PyObject_GenericSetAttr, // tp_setattro
	0,			// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,			// tp_doc
	0,			// traverseproc tp_traverse
	0,			// tp_clear
	0,			// richcmpfunc tp_richcompare
	0,			// tp_weaklistoffset
	0,			// getiterfunc tp_iter
	0,			// iternextfunc tp_iternext
	0,          // methods
	0,          // members
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get
	0,			// tp_descr_set
	0,			// tp_dictoffset
	0,			// tp_init
	0,			// tp_alloc
	PyDSOP_SCOPE_INIT_INFOs::tp_new,          // newfunc tp_new;
};

PyDSOP_SCOPE_INIT_INFOs::PyDSOP_SCOPE_INIT_INFOs(DSOP_SCOPE_INIT_INFO *_scopes, int _count)
{
	ob_type = &PyDSOP_SCOPE_INIT_INFOsType;
	pScopes = _scopes;
	count = _count;
	memset(pScopes, 0, sizeof(DSOP_SCOPE_INIT_INFO) * count);
	for (int i=0;i<count;i++)
		pScopes[i].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	_Py_NewReference(this);
}

PyDSOP_SCOPE_INIT_INFOs::~PyDSOP_SCOPE_INIT_INFOs()
{
	for (int i=0;i<count;i++)
		// Need to cast away the const of pwzDcName
		PyWinObject_FreeWCHAR((WCHAR *)pScopes[i].pwzDcName);
	free(pScopes);
}

void PyDSOP_SCOPE_INIT_INFOs::deallocFunc(PyObject *ob)
{
	delete (PyDSOP_SCOPE_INIT_INFOs *)ob;
}

PyObject *PyDSOP_SCOPE_INIT_INFOs::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
	int count;
	if (!PyArg_ParseTuple(args, "i", &count))
		return NULL;
	DSOP_SCOPE_INIT_INFO *p = (DSOP_SCOPE_INIT_INFO *)malloc(sizeof(DSOP_SCOPE_INIT_INFO) * count);
	if (!p)
		return PyErr_NoMemory();
	return new PyDSOP_SCOPE_INIT_INFOs(p, count);
}

BOOL PyObject_AsDSOP_SCOPE_INIT_INFOs(PyObject *ob, DSOP_SCOPE_INIT_INFO**p, ULONG *n)
{
	if (ob->ob_type != &PyDSOP_SCOPE_INIT_INFOsType) {
		PyErr_Format(PyExc_TypeError, "Expected DSOP_SCOPE_INIT_INFOs (got %s)",
					 ob->ob_type->tp_name);
		return FALSE;
	}
	PyDSOP_SCOPE_INIT_INFOs *psii = (PyDSOP_SCOPE_INIT_INFOs *)ob;
	*p = psii->pScopes;
	*n = psii->count;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// PyDSOP_SCOPE_INIT_INFO
////////////////////////////////////////////////////////////////////////////
// @object PyDSOP_SCOPE_INIT_INFO|An object representing an ActiveDirectory
// DSOP_SCOPE_INIT_INFO structure.
// <nl>These objects can only be accessed by indexing a <o PyDSOP_SCOPE_INIT_INFOs> object.
PyTypeObject PyDSOP_SCOPE_INIT_INFOType =
{
	PYWIN_OBJECT_HEAD
	"PyDSOP_SCOPE_INIT_INFO",
	sizeof(PyDSOP_SCOPE_INIT_INFO),
	0,
	PyDSOP_SCOPE_INIT_INFO::deallocFunc,  // tp_dealloc
	0,			// tp_print
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// PyNumberMethods *tp_as_number
	0,          // PySequenceMethods *tp_as_sequence
	0,			// PyMappingMethods *tp_as_mapping
	0,			// hashfunc tp_hash
	0,			// tp_call
	0,			// tp_str
	PyDSOP_SCOPE_INIT_INFO::getattro, // tp_getattro
	PyDSOP_SCOPE_INIT_INFO::setattro, // tp_setattro
	0,			// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,			// tp_doc
	0,			// traverseproc tp_traverse
	0,			// tp_clear
	0,			// richcmpfunc tp_richcompare
	0,			// tp_weaklistoffset
	0,			// getiterfunc tp_iter
	0,			// iternextfunc tp_iternext
	0,          // methods
	0,          // members
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get
	0,			// tp_descr_set
	0,			// tp_dictoffset
	0,			// tp_init
	0,			// tp_alloc
	0,          // newfunc tp_new;
};

PyDSOP_SCOPE_INIT_INFO::PyDSOP_SCOPE_INIT_INFO(PyDSOP_SCOPE_INIT_INFOs *_owner, int _index)
{
	ob_type = &PyDSOP_SCOPE_INIT_INFOType;
	owner = _owner;
	index = _index;

	Py_INCREF(owner);
	_Py_NewReference(this);
}

PyDSOP_SCOPE_INIT_INFO::~PyDSOP_SCOPE_INIT_INFO()
{
	Py_DECREF(owner);
}

PyObject *
PyDSOP_SCOPE_INIT_INFO::getattro(PyObject *self, PyObject *obname)
{
	PyDSOP_SCOPE_INIT_INFO *p = (PyDSOP_SCOPE_INIT_INFO *)self;
	DSOP_SCOPE_INIT_INFO *pssi = p->owner->pScopes + p->index;
	char *name=PyString_AsString(obname);
	if (!name) return NULL;
	// @prop int|type|
	if (strcmp(name, "type")==0) return PyInt_FromLong(pssi->flType);
	// @prop int|scope|
	if (strcmp(name, "scope")==0) return PyInt_FromLong(pssi->flScope);
	// @prop int|hr|
	if (strcmp(name, "hr")==0) return PyInt_FromLong(pssi->hr);
	// @prop <o PyUnicode>|dcName|
	if (strcmp(name, "dcName")==0) return PyWinObject_FromWCHAR(pssi->pwzDcName);
	// @prop <o PyDSOP_FILTER_FLAGS>|filterFlags|
	if (strcmp(name, "filterFlags")==0) return new PyDSOP_FILTER_FLAGS(p);
	return PyObject_GenericGetAttr(self,obname);
}

int PyDSOP_SCOPE_INIT_INFO::setattro(PyObject *self, PyObject *obname, PyObject *val)
{
	PyDSOP_SCOPE_INIT_INFO *p = (PyDSOP_SCOPE_INIT_INFO *)self;
	DSOP_SCOPE_INIT_INFO *pssi = p->owner->pScopes + p->index;
	char *name=PyString_AsString(obname);
	PyErr_Clear();
	if (strcmp(name, "type")==0) {
		pssi->flType = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "scope")==0) {
		pssi->flScope = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "scope")==0) {
		pssi->hr = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "dcName")==0) {
		WCHAR *buf;
		if (!PyWinObject_AsWCHAR(val, &buf, TRUE))
			return -1;
		PyWinObject_FreeWCHAR((WCHAR *)pssi->pwzDcName);
		pssi->pwzDcName = buf;
	}
	else if (strcmp(name, "filterFlags")==0) {
		PyErr_SetString(PyExc_AttributeError, "filterFlags attribute can not be set (try setting attributes on the object itself)");
		return -1;
	}
	else {
		return PyObject_GenericSetAttr(self, obname, val);
	}
	return 0;
}

void PyDSOP_SCOPE_INIT_INFO::deallocFunc(PyObject *ob)
{
	delete (PyDSOP_SCOPE_INIT_INFO *)ob;
}

////////////////////////////////////////////////////////////////////////////
// PyDSOP_FILTER_FLAGS
////////////////////////////////////////////////////////////////////////////
// @object PyDSOP_FILTER_FLAGS|An object representing an ActiveDirectory DSOP_FILTER_FLAGS structure
// <nl>These objects can only be accessed via a <o PyDSOP_SCOPE_INIT_INFO> object.
PyTypeObject PyDSOP_FILTER_FLAGSType =
{
	PYWIN_OBJECT_HEAD
	"PyDSOP_FILTER_FLAGS",
	sizeof(PyDSOP_FILTER_FLAGS),
	0,
	PyDSOP_FILTER_FLAGS::deallocFunc,  // tp_dealloc
	0,			// tp_print
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// PyNumberMethods *tp_as_number
	0,          // PySequenceMethods *tp_as_sequence
	0,			// PyMappingMethods *tp_as_mapping
	0,			// hashfunc tp_hash
	0,			// tp_call
	0,			// tp_str
	PyDSOP_FILTER_FLAGS::getattro, // tp_getattro
	PyDSOP_FILTER_FLAGS::setattro, // tp_setattro
	0,			// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,			// tp_doc
	0,			// traverseproc tp_traverse
	0,			// tp_clear
	0,			// richcmpfunc tp_richcompare
	0,			// tp_weaklistoffset
	0,			// getiterfunc tp_iter
	0,			// iternextfunc tp_iternext
	0,          // methods
	0,          // members
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get
	0,			// tp_descr_set
	0,			// tp_dictoffset
	0,			// tp_init
	0,			// tp_alloc
	0,          // newfunc tp_new;
};

PyDSOP_FILTER_FLAGS::PyDSOP_FILTER_FLAGS(PyDSOP_SCOPE_INIT_INFO *_owner)
{
	ob_type = &PyDSOP_FILTER_FLAGSType;
	owner = _owner;
	Py_INCREF(owner);
	_Py_NewReference(this);
}

PyDSOP_FILTER_FLAGS::~PyDSOP_FILTER_FLAGS()
{
	Py_DECREF(owner);
}

PyObject *
PyDSOP_FILTER_FLAGS::getattro(PyObject *self, PyObject *obname)
{
	PyDSOP_FILTER_FLAGS *p = (PyDSOP_FILTER_FLAGS *)self;
	char *name=PyString_AsString(obname);
	if (!name) return NULL;
	DSOP_SCOPE_INIT_INFO *psii = p->owner->owner->pScopes + p->owner->index;
	// @prop <o PyDSOP_UPLEVEL_FILTER_FLAGS>|uplevel|
	if (strcmp(name, "uplevel")==0) return new PyDSOP_UPLEVEL_FILTER_FLAGS(p->owner);
	// @prop int|downlevel|
	if (strcmp(name, "downlevel")==0) return PyInt_FromLong(psii->FilterFlags.flDownlevel);
	return PyObject_GenericGetAttr(self,obname);
}

int PyDSOP_FILTER_FLAGS::setattro(PyObject *self, PyObject *obname, PyObject *val)
{
	PyDSOP_FILTER_FLAGS *p = (PyDSOP_FILTER_FLAGS *)self;
	char *name=PyString_AsString(obname);
	if (!name) return NULL;
	DSOP_SCOPE_INIT_INFO *psii = p->owner->owner->pScopes + p->owner->index;
	PyErr_Clear();
	if (strcmp(name, "downlevel")==0) {
		psii->FilterFlags.flDownlevel = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "uplevel")==0) {
		PyErr_SetString(PyExc_AttributeError, "uplevel attribute can not be set (try setting attributes on the object itself)");
		return -1;
	}
	else {
		return PyObject_GenericSetAttr(self, obname, val);
	}
	return 0;
}

void PyDSOP_FILTER_FLAGS::deallocFunc(PyObject *ob)
{
	delete (PyDSOP_FILTER_FLAGS *)ob;
}
////////////////////////////////////////////////////////////////////////////
// PyDSOP_UPLEVEL_FILTER_FLAGS
////////////////////////////////////////////////////////////////////////////
// @object PyDSOP_UPLEVEL_FILTER_FLAGS|An object representing an ActiveDirectory
// DSOP_UPLEVEL_FILTER_FLAGS structure.
// <nl>These objects can only be accessed via a <o PyDSOP_FILTER_FLAGS> object.
PyTypeObject PyDSOP_UPLEVEL_FILTER_FLAGSType =
{
	PYWIN_OBJECT_HEAD
	"PyDSOP_UPLEVEL_FILTER_FLAGS",
	sizeof(PyDSOP_UPLEVEL_FILTER_FLAGS),
	0,
	PyDSOP_UPLEVEL_FILTER_FLAGS::deallocFunc,  // tp_dealloc
	0,			// tp_print
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// PyNumberMethods *tp_as_number
	0,			// PySequenceMethods *tp_as_sequence
	0,			// PyMappingMethods *tp_as_mapping
	0,			// hashfunc tp_hash
	0,			// tp_call
	0,			// tp_str
	PyDSOP_UPLEVEL_FILTER_FLAGS::getattro, // tp_getattro
	PyDSOP_UPLEVEL_FILTER_FLAGS::setattro, // tp_setattro
	0,			// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,			// tp_doc
	0,			// traverseproc tp_traverse
	0,			// tp_clear
	0,			// richcmpfunc tp_richcompare
	0,			// tp_weaklistoffset
	0,			// getiterfunc tp_iter
	0,			// iternextfunc tp_iternext
	0,			// methods
	0,			// members
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get
	0,			// tp_descr_set
	0,			// tp_dictoffset
	0,			// tp_init
	0,			// tp_alloc
	0,			// newfunc tp_new;
};

PyDSOP_UPLEVEL_FILTER_FLAGS::PyDSOP_UPLEVEL_FILTER_FLAGS(PyDSOP_SCOPE_INIT_INFO *_owner)
{
	ob_type = &PyDSOP_UPLEVEL_FILTER_FLAGSType;
	owner = _owner;
	Py_INCREF(owner);
	_Py_NewReference(this);
}

PyDSOP_UPLEVEL_FILTER_FLAGS::~PyDSOP_UPLEVEL_FILTER_FLAGS()
{
	Py_DECREF(owner);
}

PyObject *
PyDSOP_UPLEVEL_FILTER_FLAGS::getattro(PyObject *self, PyObject *obname)
{
	PyDSOP_UPLEVEL_FILTER_FLAGS *p = (PyDSOP_UPLEVEL_FILTER_FLAGS *)self;
	DSOP_SCOPE_INIT_INFO *psii = p->owner->owner->pScopes + p->owner->index;
	char *name=PyString_AsString(obname);
	if (!name) return NULL;
	// @prop int|bothModes|
	if (strcmp(name, "bothModes")==0) return PyInt_FromLong(psii->FilterFlags.Uplevel.flBothModes);
	// @prop int|mixedModeOnly|
	if (strcmp(name, "mixedModeOnly")==0) return PyInt_FromLong(psii->FilterFlags.Uplevel.flMixedModeOnly);
	// @prop int|nativeModeOnly|
	if (strcmp(name, "nativeModeOnly")==0) return PyInt_FromLong(psii->FilterFlags.Uplevel.flNativeModeOnly);
	return PyObject_GenericGetAttr(self,obname);
}

int PyDSOP_UPLEVEL_FILTER_FLAGS::setattro(PyObject *self, PyObject *obname, PyObject *val)
{
	PyDSOP_UPLEVEL_FILTER_FLAGS *p = (PyDSOP_UPLEVEL_FILTER_FLAGS *)self;
	DSOP_SCOPE_INIT_INFO *psii = p->owner->owner->pScopes + p->owner->index;
	char *name=PyString_AsString(obname);
	if (!name) return NULL;
	PyErr_Clear();
	if (strcmp(name, "bothModes")==0) {
		psii->FilterFlags.Uplevel.flBothModes = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "mixedModeOnly")==0) {
		psii->FilterFlags.Uplevel.flMixedModeOnly = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else if (strcmp(name, "nativeModeOnly")==0) {
		psii->FilterFlags.Uplevel.flNativeModeOnly = PyInt_AsLong(val);
		if (PyErr_Occurred()) return -1;
	}
	else {
		return PyObject_GenericSetAttr(self, obname, val);
	}
	return 0;
}

void PyDSOP_UPLEVEL_FILTER_FLAGS::deallocFunc(PyObject *ob)
{
	delete (PyDSOP_UPLEVEL_FILTER_FLAGS *)ob;
}
