#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"

// @doc

// mem manager helpers.
extern void *AllocateMoreBuffer(size_t size);
extern void *AllocMore( void *pRoot, size_t size, BOOL bForVariant = FALSE );
extern void FreeMoreBuffer(void *);

// @pymethod <o VARDESC>|pythoncom|VARDESC|Creates a new VARDESC object
PyObject *Py_NewVARDESC(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return new PyVARDESC();
}

PyObject *PyObject_FromVARDESC(VARDESC *desc)
{
	return new PyVARDESC(desc);
}

// @object VARDESC|A VARDESC object represents a COM VARDESC structure.
static struct PyMethodDef PyVARDESC_methods[] = {
	{NULL}
};


BOOL PyObject_AsVARDESC(PyObject *ob, VARDESC *v, void *pMore)
{
	if (ob->ob_type != &PyVARDESC::Type) {
		PyErr_SetString(PyExc_TypeError, "Object is not a VARDESC.");
		return FALSE;
	}
	PyVARDESC *pyv = (PyVARDESC *)ob;
	v->memid = pyv->memid;
	v->wVarFlags = pyv->wVarFlags;
	v->varkind = (VARKIND)pyv->varkind;
	if (!PyObject_AsELEMDESC(pyv->elemdescVar, &v->elemdescVar, pMore))
		return FALSE;

	if (v->varkind == VAR_PERINSTANCE) {
		if (!PyInt_Check(pyv->value)) {
			PyErr_SetString(PyExc_TypeError, "If varkind==VAR_PERINSTANCE, value attribute must be an integer");
			return FALSE;
		}
		v->oInst = PyInt_AsLong(pyv->value);
	}  else if (v->varkind == VAR_CONST) {
		VARIANT *pVar = (VARIANT *)AllocMore(pMore, sizeof(VARIANT), TRUE);
		if (pVar==NULL) return NULL;
		VariantInit(pVar);
		if (!PyCom_VariantFromPyObject(pyv->value, pVar))
			return NULL;
		v->lpvarValue = pVar;
	}
	// else ignore value.
	return TRUE;
}

BOOL PyObject_AsVARDESC(PyObject *ob, VARDESC **pp)
{
	*pp = (VARDESC *)AllocateMoreBuffer(sizeof(VARDESC));
	if (*pp==NULL) return FALSE;
	BOOL rc = PyObject_AsVARDESC(ob, *pp, *pp);
	if (!rc)
		FreeMoreBuffer(*pp);
	return rc;
}

void PyObject_FreeVARDESC(VARDESC *p)
{
	FreeMoreBuffer(p);
}

// Sequence stuff to provide compatibility with tuples.
static PySequenceMethods PyVARDESC_Sequence = 
{
	PyVARDESC::getlength, // sq_length;
	NULL, // sq_concat;
	NULL, // sq_repeat;
	PyVARDESC::getitem, // sq_item;
	NULL, // sq_slice;
	NULL, // sq_ass_item;
	NULL, // sq_ass_slice;
};


PyTypeObject PyVARDESC::Type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyVARDESC",
	sizeof(PyVARDESC),
	0,
	PyVARDESC::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyVARDESC::getattr,				/* tp_getattr */
	PyVARDESC::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	&PyVARDESC_Sequence,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyVARDESC, e)

/*static*/ struct memberlist PyVARDESC::memberlist[] = {
	{"memid",          T_INT,   OFF(memid)}, // @prop int|memid|The dispid of the member
	{"value",          T_OBJECT,OFF(value)}, // @prop int/object|value|A value for the variant.  If PERINSTANCE then an offset into the instance, otherwise a variant converted to a Python object.
	{"elemdescVar",    T_OBJECT,OFF(elemdescVar)}, // @prop <o ELEMDESC>|elemdescVar|Object describing the member.
	{"wVarFlags",      T_INT,   OFF(wVarFlags)}, // @prop int|varFlags|Variable flags
	{"varkind",        T_INT,   OFF(varkind)}, // @prop int|varkind|Kind flags.
	{NULL}
};

PyVARDESC::PyVARDESC()
{
	ob_type = &PyVARDESC::Type;
	_Py_NewReference(this);

	memid = 0;
	value = NULL;
	elemdescVar = NULL;
	wVarFlags = 0;
	varkind = 0;
}

PyVARDESC::PyVARDESC(const VARDESC *pVD)
{
	ob_type = &PyVARDESC::Type;
	_Py_NewReference(this);

	memid = pVD->memid;
	wVarFlags = pVD->wVarFlags;
	varkind = pVD->varkind;

	if (pVD->varkind == VAR_PERINSTANCE)
		value = PyInt_FromLong(pVD->oInst);
	else if (pVD->varkind == VAR_CONST)
		value  = PyCom_PyObjectFromVariant(pVD->lpvarValue);
	else {
		value = Py_None;
		Py_INCREF(Py_None);
	}
	elemdescVar = PyObject_FromELEMDESC(&pVD->elemdescVar);
}

PyVARDESC::~PyVARDESC()
{
	Py_XDECREF(elemdescVar);
	Py_XDECREF(value);
}

PyObject *PyVARDESC::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PyVARDESC_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	return PyMember_Get((char *)self, memberlist, name);
}

int PyVARDESC::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete PyVARDESC attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyVARDESC::deallocFunc(PyObject *ob)
{
	delete (PyVARDESC *)ob;
}

// Sequence stuff to provide compatibility with tuples.
/* static */ int PyVARDESC::getlength(PyObject *self)
{
	// NEVER CHANGE THIS - you will break all the old
	// code written when these object were tuples!
	return 5;
}

/* static */ PyObject *PyVARDESC::getitem(PyObject *self, int index)
{
	PyVARDESC *p = (PyVARDESC *)self;
	PyObject *rc;
	switch (index) {
		case 0: // @tupleitem 0|int|memid|The id of the member
			return PyInt_FromLong(p->memid);
		case 1: // @tupleitem 1|int/object|value|A value for the variant.  If PERINSTANCE then an offset into the instance, otherwise a variant converted to a Python object.
			rc = p->value ? p->value : Py_None;
			Py_INCREF(rc);
			return rc;
		case 2:	// @tupleitem 2|<o ELEMDESC>|elemdescVar|Object describing the member.
			rc = p->elemdescVar ? p->elemdescVar : Py_None;
			Py_INCREF(rc);
			return rc;
		case 3: // @tupleitem 3|int|varFlags|Variable flags
			return PyInt_FromLong(p->wVarFlags);
		case 4: // @tupleitem 4|int|varKind|Kind flags.
			return PyInt_FromLong(p->varkind); 
	}
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
}
