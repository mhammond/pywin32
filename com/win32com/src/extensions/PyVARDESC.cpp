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
struct PyMethodDef PyVARDESC::methods[] = {
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
	}  else if (v->varkind == VAR_DISPATCH) {
		// nothing to do - memid is all that is needed by the caller.
		;
	} else {
		PyCom_LoggerWarning(NULL, "PyObject_AsVARDESC has unknown varkind (%d) - None will be used", v->varkind);
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
	PYWIN_OBJECT_HEAD
	"PyVARDESC",
	sizeof(PyVARDESC),
	0,
	PyVARDESC::deallocFunc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	&PyVARDESC_Sequence,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	PyVARDESC::methods,		/* tp_methods */
	PyVARDESC::members,		/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

#define OFF(e) offsetof(PyVARDESC, e)

/*static*/ struct PyMemberDef PyVARDESC::members[] = {
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

	if (varkind == VAR_PERINSTANCE)
		value = PyInt_FromLong(pVD->oInst);
	else if (varkind == VAR_CONST) {
		VARIANT varValue;

		// Cast the variant type here to the correct value for this constant
		// so that the correct Python type will be created below.
		// The problem seems to exist for unsigned types (the variant has
		// a signed type, but the typelib has an unsigned one).  However,
		// doing this unconditionally has side-effects, as the typelib
		// has VT_LPWSTR for the type of strings - and VariantChangeType
		// returns a VT_EMPTY variant in that case.
		// So we only perform this conversion for types known to be a problem:
		switch (pVD->elemdescVar.tdesc.vt) {
		case VT_UI1:
		case VT_UI2:
		case VT_UI4:
		case VT_UI8:
		case VT_UINT:
		case VT_UINT_PTR:
			VariantInit(&varValue);
			VariantChangeType(&varValue, pVD->lpvarValue, 0, pVD->elemdescVar.tdesc.vt);

			value  = PyCom_PyObjectFromVariant(&varValue);

			VariantClear(&varValue);
			break;
		default:
			value  = PyCom_PyObjectFromVariant(pVD->lpvarValue);
			break;
		}
	} else if (varkind == VAR_DISPATCH) {
		// all caller needs is memid, which is already setup.
		value = Py_None;
		Py_INCREF(Py_None);
	} else {
		PyCom_LoggerWarning(NULL, "PyVARDESC ctor has unknown varkind (%d) - returning None", varkind);
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

/*static*/ void PyVARDESC::deallocFunc(PyObject *ob)
{
	delete (PyVARDESC *)ob;
}

// Sequence stuff to provide compatibility with tuples.
/* static */ Py_ssize_t PyVARDESC::getlength(PyObject *self)
{
	// NEVER CHANGE THIS - you will break all the old
	// code written when these object were tuples!
	return 5;
}

/* static */ PyObject *PyVARDESC::getitem(PyObject *self, Py_ssize_t index)
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
