//
// @doc

#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#ifdef MS_WINCE 

BOOL PyWinObject_AsSECURITY_ATTRIBUTES(PyObject *ob, SECURITY_ATTRIBUTES **ppSECURITY_ATTRIBUTES, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSECURITY_ATTRIBUTES = NULL;
	} else {
		if (bNoneOK)
			PyErr_SetString(PyExc_TypeError, "Windows CE only supports None as a SECURITY_ATTRIBUTE");
		else
			PyErr_SetString(PyExc_TypeError, "This function can not work under Windows CE, as only None may be used as a SECURITY_ATTRIBUTE");
		return FALSE;
	}
	return TRUE;
}

#else /* This code is not available on Windows CE */

// @pymethod <o PySECURITY_ATTRIBUTES>|pywintypes|SECURITY_ATTRIBUTES|Creates a new SECURITY_ATTRIBUTES object
PyObject *PyWinMethod_NewSECURITY_ATTRIBUTES(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SECURITY_ATTRIBUTES"))
		return NULL;
	return new PySECURITY_ATTRIBUTES();
}

PyObject *PyWinObject_FromSECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa)
{
	return new PySECURITY_ATTRIBUTES(sa);
}

BOOL PyWinObject_AsSECURITY_ATTRIBUTES(PyObject *ob, SECURITY_ATTRIBUTES **ppSECURITY_ATTRIBUTES, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSECURITY_ATTRIBUTES = NULL;
	} else if (!PySECURITY_ATTRIBUTES_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PySECURITY_ATTRIBUTES object");
		return FALSE;
	} else {
		PySECURITY_ATTRIBUTES *pysa= (PySECURITY_ATTRIBUTES *)ob;
		*ppSECURITY_ATTRIBUTES = pysa->GetSA();
		// in case the PySECURITY_DESCRIPTOR has been manipulated and points to a different address now
		((SECURITY_ATTRIBUTES *)*ppSECURITY_ATTRIBUTES)->lpSecurityDescriptor =
			((PySECURITY_DESCRIPTOR *)pysa->m_obSD)->GetSD();
	}
	return TRUE;
}


// @object PySECURITY_ATTRIBUTES|A Python object, representing a SECURITY_ATTRIBUTES structure
static struct PyMethodDef PySECURITY_ATTRIBUTES_methods[] = {
	{NULL}
};


PYWINTYPES_EXPORT PyTypeObject PySECURITY_ATTRIBUTESType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySECURITY_ATTRIBUTES",
	sizeof(PySECURITY_ATTRIBUTES),
	0,
	PySECURITY_ATTRIBUTES::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySECURITY_ATTRIBUTES::getattr,				/* tp_getattr */
	PySECURITY_ATTRIBUTES::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PySECURITY_ATTRIBUTES, e)

/*static*/ struct memberlist PySECURITY_ATTRIBUTES::memberlist[] = {
	{"bInheritHandle",  T_INT,  OFF(m_sa.bInheritHandle)}, // @prop integer|bInheritHandle|Specifies whether the returned handle is inherited when a new process is created. If this member is TRUE, the new process inherits the handle.
	{"SECURITY_DESCRIPTOR", T_OBJECT, OFF(m_obSD)},
	{NULL}	/* Sentinel */
};

PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(void)
{
	ob_type = &PySECURITY_ATTRIBUTESType;
	_Py_NewReference(this);
	m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	m_obSD = new PySECURITY_DESCRIPTOR();
	m_sa.lpSecurityDescriptor = ((PySECURITY_DESCRIPTOR *)m_obSD)->GetSD();
	m_sa.bInheritHandle = TRUE;
}

PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa)
{
	ob_type = &PySECURITY_ATTRIBUTESType;
	_Py_NewReference(this);
	m_sa = sa;
	m_obSD = new PySECURITY_DESCRIPTOR(sa.lpSecurityDescriptor);
	// above creates a copy, put pointer to copy back in SECURITY_ATTRIBUTES structure
	m_sa.lpSecurityDescriptor=((PySECURITY_DESCRIPTOR *)m_obSD)->GetSD();
}

PySECURITY_ATTRIBUTES::~PySECURITY_ATTRIBUTES()
{
	Py_XDECREF( m_obSD );
}

PyObject *PySECURITY_ATTRIBUTES::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PySECURITY_ATTRIBUTES_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();

	res = PyMember_Get((char *)self, memberlist, name);
	if (res != NULL)
		return res;
	// let it inherit methods from PySECURITY_DESCRIPTOR for backward compatibility
	PyErr_Clear();
	PySECURITY_ATTRIBUTES *This = (PySECURITY_ATTRIBUTES *)self;
	return ((PySECURITY_DESCRIPTOR *)(This->m_obSD))->getattr(This->m_obSD, name);
}

int PySECURITY_ATTRIBUTES::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete SECURITY_ATTRIBUTES attributes");
		return -1;
	}
	if (strcmp(name, "SECURITY_DESCRIPTOR")==0){
		if (!PySECURITY_DESCRIPTOR_Check(v)){
			PyErr_SetString(PyExc_TypeError, "The object is not a PySECURITY_DESCRIPTOR object");
			return -1;
			}
		PySECURITY_ATTRIBUTES *This=(PySECURITY_ATTRIBUTES *)self;
		// Py_DECREF(This->m_obSD);
		This->m_sa.lpSecurityDescriptor=((PySECURITY_DESCRIPTOR *)This->m_obSD)->GetSD();
		}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PySECURITY_ATTRIBUTES::deallocFunc(PyObject *ob)
{
	delete (PySECURITY_ATTRIBUTES *)ob;
}

#endif /* MS_WINCE */

