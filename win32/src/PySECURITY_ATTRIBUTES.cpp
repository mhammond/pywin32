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
		*ppSECURITY_ATTRIBUTES = ((PySECURITY_ATTRIBUTES *)ob)->GetSA();
	}
	return TRUE;
}

// @pymethod |PySECURITY_ATTRIBUTES|Initialize|Initialize the ACL.
// @comm It should not be necessary to call this, as the ACL object
// is initialised by Python.  This method gives you a chance to trap
// any errors that may occur.
PyObject *PySECURITY_ATTRIBUTES::Initialize(PyObject *self, PyObject *args)
{
	PySECURITY_ATTRIBUTES *This = (PySECURITY_ATTRIBUTES *)self;
	if (!PyArg_ParseTuple(args, ":Initialize"))
		return NULL;
	if (!::InitializeSecurityDescriptor(&This->m_sd, SECURITY_DESCRIPTOR_REVISION))
		return PyWin_SetAPIError("InitializeSecurityDescriptor");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PySECURITY_ATTRIBUTES|SetSecurityDescriptorDacl|
PyObject *PySECURITY_ATTRIBUTES::SetSecurityDescriptorDacl(PyObject *self, PyObject *args)
{
	PySECURITY_ATTRIBUTES *This = (PySECURITY_ATTRIBUTES *)self;
	BOOL bPresent, bDefaulted;
	PyObject *obACL;
	if (!PyArg_ParseTuple(args, "iOi:SetSecurityDescriptorDacl", &bPresent, &obACL, &bDefaulted))
		return NULL;
	PACL pacl;
	if (!PyWinObject_AsACL(obACL, &pacl, TRUE))
		return NULL;
	if (!::SetSecurityDescriptorDacl(&This->m_sd, bPresent, pacl, bDefaulted))
		return PyWin_SetAPIError("SetSecurityDescriptorDacl");
	Py_INCREF(Py_None);
	return Py_None;
}


// @object PySECURITY_ATTRIBUTES|A Python object, representing a SECURITY_ATTRIBUTES structure
static struct PyMethodDef PySECURITY_ATTRIBUTES_methods[] = {
	{"Initialize",     PySECURITY_ATTRIBUTES::Initialize, 1}, 	// @pymeth Initialize|Initializes the object.
	{"SetSecurityDescriptorDacl",     PySECURITY_ATTRIBUTES::SetSecurityDescriptorDacl, 1}, 	// @pymeth SetSecurityDescriptorDacl|
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
	{NULL}	/* Sentinel */
};

PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(void)
{
	ob_type = &PySECURITY_ATTRIBUTESType;
	_Py_NewReference(this);
	m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	m_sa.lpSecurityDescriptor = &m_sd;
	m_sa.bInheritHandle = TRUE;
	::InitializeSecurityDescriptor(&m_sd, SECURITY_DESCRIPTOR_REVISION);
}

PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa)
{
	ob_type = &PySECURITY_ATTRIBUTESType;
	_Py_NewReference(this);
	m_sa = sa;
	::InitializeSecurityDescriptor(&m_sd, SECURITY_DESCRIPTOR_REVISION);
}


PyObject *PySECURITY_ATTRIBUTES::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PySECURITY_ATTRIBUTES_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	return PyMember_Get((char *)self, memberlist, name);
}

int PySECURITY_ATTRIBUTES::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete SECURITY_ATTRIBUTES attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PySECURITY_ATTRIBUTES::deallocFunc(PyObject *ob)
{
	delete (PySECURITY_ATTRIBUTES *)ob;
}

#endif /* MS_WINCE */
