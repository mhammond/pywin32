#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"

// @doc

// @object IDLDESC|An IDLDESC is respresented as
static PyObject *MakeIDLDesc(const IDLDESC *id)
{
	// @tupleitem 0|int|reserved|A reserved value!
	// @tupleitem 1|int|flags|IDL flags.
	return Py_BuildValue("li", id->dwReserved, id->wIDLFlags);
}


// @pymethod <o TYPEATTR>|pythoncom|TYPEATTR|Creates a new TYPEATTR object
PyObject *Py_NewTYPEATTR(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return new PyTYPEATTR();
}

PyObject *PyObject_FromTYPEATTR(TYPEATTR *desc)
{
	return new PyTYPEATTR(desc);
}

// @object TYPEATTR|A TYPEATTR object represents a COM TYPEATTR structure.
static struct PyMethodDef PyTYPEATTR_methods[] = {
	{NULL}
};


// Sequence stuff to provide compatibility with tuples.
static PySequenceMethods PyTYPEATTR_Sequence = 
{
	PyTYPEATTR::getlength, // sq_length;
	NULL, // sq_concat;
	NULL, // sq_repeat;
	PyTYPEATTR::getitem, // sq_item;
	NULL, // sq_slice;
	NULL, // sq_ass_item;
	NULL, // sq_ass_slice;
};


PyTypeObject PyTYPEATTR::Type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyTYPEATTR",
	sizeof(PyTYPEATTR),
	0,
	PyTYPEATTR::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyTYPEATTR::getattr,				/* tp_getattr */
	PyTYPEATTR::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	&PyTYPEATTR_Sequence,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyTYPEATTR, e)

/*static*/ struct memberlist PyTYPEATTR::memberlist[] = {
	{"iid",            T_OBJECT,OFF(iid)},// @prop <o PyIID>|IID|The IID
	{"lcid",           T_INT,   OFF(lcid)}, // @prop int|lcid|The lcid
	{"memidConstructor",T_INT,  OFF(memidConstructor)}, // @prop int|memidConstructor|ID of constructor
	{"memidDestructor",T_INT,   OFF(memidDestructor)}, // @prop int|memidDestructor|ID of destructor
	{"cbSizeInstance", T_INT,   OFF(cbSizeInstance)}, // @prop int|cbSizeInstance|The size of an instance of this type
	{"typekind",       T_INT,   OFF(typekind)}, // @prop int|typekind|The kind of type this information describes.  One of the win32con.TKIND_* constants.
	{"cFuncs",         T_INT,   OFF(cFuncs)}, // @prop int|cFuncs|Number of functions.
	{"cVars",          T_INT,   OFF(cVars)}, // @prop int|cVars|Number of variables/data members.
	{"cImplTypes",     T_INT,   OFF(cImplTypes)}, // @prop int|cImplTypes|Number of implemented interfaces.
	{"cbSizeVft",      T_INT,   OFF(cbSizeVft)}, // @prop int|cbSizeVft|The size of this type's VTBL
	{"cbAlignment",    T_INT,   OFF(cbAlignment)}, // @prop int|cbAlignment|Byte alignment for an instance of this type.
	{"wTypeFlags",     T_INT,   OFF(wTypeFlags)}, // @prop int|wTypeFlags|One of the pythoncom TYPEFLAG_
	{"wMajorVerNum",   T_INT,   OFF(wMajorVerNum)}, // @prop int|wMajorVerNum|Major version number.
	{"wMinorVerNum",   T_INT,   OFF(wMinorVerNum)}, // @prop int|wMinorVerNum|Minor version number.
	{"tdescAlias",     T_OBJECT,OFF(obDescAlias)}, // @prop <o TYPEDESC>|obDescAlias|If TypeKind == pythoncom.TKIND_ALIAS, specifies the type for which this type is an alias.
	{"idldescType",    T_OBJECT,OFF(obIDLDesc)}, // @prop <o IDLDESC>|obIDLDesc|IDL attributes of the described type.
	{NULL}
};

PyTYPEATTR::PyTYPEATTR()
{
	ob_type = &PyTYPEATTR::Type;
	_Py_NewReference(this);

	iid = NULL;
	lcid = 0;
	memidConstructor = 0;
	memidDestructor = 0;
	cbSizeInstance = 0;
	typekind = 0;
	cFuncs = 0;
	cVars = 0;
	cImplTypes = 0;
	cbSizeVft = 0;
	cbAlignment = 0;
	wTypeFlags = 0;
	wMajorVerNum = 0;
	wMinorVerNum = 0;
	obDescAlias = NULL;
	obIDLDesc = NULL;
}

PyTYPEATTR::PyTYPEATTR(const TYPEATTR *attr)
{
	ob_type = &PyTYPEATTR::Type;
	_Py_NewReference(this);

	iid = PyWinObject_FromIID(attr->guid);
	lcid = attr->lcid;
	memidConstructor = attr->memidConstructor;
	memidDestructor = attr->memidDestructor;
	cbSizeInstance = attr->cbSizeInstance;
	typekind = attr->typekind;
	cFuncs = attr->cFuncs;
	cVars = attr->cVars;
	cImplTypes = attr->cImplTypes;
	cbSizeVft = attr->cbSizeVft;
	cbAlignment = attr->cbAlignment;
	wTypeFlags = attr->wTypeFlags;
	wMajorVerNum = attr->wMajorVerNum;
	wMinorVerNum = attr->wMinorVerNum;

	// Some (only a few 16 bit MSOffice only one so far, and even then only occasionally!)
	// servers seem to send invalid tdescAlias when its not actually an alias.
	if (attr->typekind == TKIND_ALIAS)
		obDescAlias = PyObject_FromTYPEDESC(&attr->tdescAlias);
	else {
		Py_INCREF(Py_None);
		obDescAlias=Py_None;
	}

	obIDLDesc = MakeIDLDesc(&attr->idldescType);
}

PyTYPEATTR::~PyTYPEATTR()
{
	Py_XDECREF(iid);
	Py_XDECREF(obDescAlias);
	Py_XDECREF(obIDLDesc);
}

PyObject *PyTYPEATTR::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PyTYPEATTR_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	return PyMember_Get((char *)self, memberlist, name);
}

int PyTYPEATTR::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete PyTYPEATTR attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyTYPEATTR::deallocFunc(PyObject *ob)
{
	delete (PyTYPEATTR *)ob;
}

// Sequence stuff to provide compatibility with tuples.
/* static */ int PyTYPEATTR::getlength(PyObject *self)
{
	// NEVER CHANGE THIS - you will break all the old
	// code written when these object were tuples!
	return 16;
}

/* static */ PyObject *PyTYPEATTR::getitem(PyObject *self, int index)
{
	PyTYPEATTR *p = (PyTYPEATTR *)self;
	PyObject *rc;
	switch (index) {
		case 0: // @tupleitem 0|<o PyIID>|IID|The IID
			rc = p->iid ? p->iid : Py_None;
			Py_INCREF(rc);
			return rc;
		case 1: // @tupleitem 1|int|lcid|The lcid
			return PyInt_FromLong(p->lcid);
		case 2: // @tupleitem 2|int|memidConstructor|ID of constructor
			return PyInt_FromLong(p->memidConstructor);
		case 3: // @tupleitem 3|int|memidDestructor|ID of destructor,
			return PyInt_FromLong(p->memidDestructor);
		case 4: // @tupleitem 4|int|cbSizeInstance|The size of an instance of this type
			return PyInt_FromLong(p->cbSizeInstance);
		case 5: // @tupleitem 5|int|typekind|The kind of type this information describes.  One of the win32con.TKIND_* constants.
			return PyInt_FromLong(p->typekind);
		case 6: // @tupleitem 6|int|cFuncs|Number of functions.
			return PyInt_FromLong(p->cFuncs);
		case 7: // @tupleitem 7|int|cVars|Number of variables/data members.
			return PyInt_FromLong(p->cVars);
		case 8: // @tupleitem 8|int|cImplTypes|Number of implemented interfaces.
			return PyInt_FromLong(p->cImplTypes);
		case 9: // @tupleitem 9|int|cbSizeVft|The size of this type's VTBL
			return PyInt_FromLong(p->cbSizeVft);
		case 10: // @tupleitem 10|int|cbAlignment|Byte alignment for an instance of this type.
			return PyInt_FromLong(p->cbAlignment);
		case 11: // @tupleitem 11|int|wTypeFlags|One of the pythoncom TYPEFLAG_* constants
			return PyInt_FromLong(p->wTypeFlags);
		case 12: // @tupleitem 12|int|wMajorVerNum|Major version number.
			return PyInt_FromLong(p->wMajorVerNum);
		case 13: // @tupleitem 13|int|wMinorVerNum|Minor version number.
			return PyInt_FromLong(p->wMinorVerNum);
		case 14: // @tupleitem 14|<o TYPEDESC>|obDescAlias|If TypeKind == pythoncom.TKIND_ALIAS, specifies the type for which this type is an alias.
			rc = p->obDescAlias ? p->obDescAlias : Py_None;
			Py_INCREF(rc);
			return rc;
		case 15: // @tupleitem 15|<o IDLDESC>|obIDLDesc|IDL attributes of the described type.
			rc = p->obIDLDesc ? p->obIDLDesc : Py_None;
			Py_INCREF(rc);
			return rc;
	}
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
}
