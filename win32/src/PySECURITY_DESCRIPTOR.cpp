//
// @doc

#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#ifndef MS_WINCE /* This code is not available on Windows CE */

// @pymethod <o PySECURITY_DESCRIPTOR>|pywintypes|SECURITY_DESCRIPTOR|Creates a new SECURITY_DESCRIPTOR object
PyObject *PyWinMethod_NewSECURITY_DESCRIPTOR(PyObject *self, PyObject *args)
{
	unsigned cb = 0;
	// @pyparm int|cb|0|The number of bytes to allocate, or 0 for the default.
	if (PyArg_ParseTuple(args, "|i:SECURITY_DESCRIPTOR", &cb))
		return new PySECURITY_DESCRIPTOR(cb);

	PyErr_Clear();
	char *szRawData;
	// @pyparmalt1 buffer|data||A buffer (eg, a string) with the raw bytes for the security descriptor.
	if (!PyArg_ParseTuple(args, "s#:SECURITY_DESCRIPTOR", &szRawData, &cb))
		return NULL;
	return new PySECURITY_DESCRIPTOR((SECURITY_DESCRIPTOR *)szRawData, cb);
}

BOOL PyWinObject_AsSECURITY_DESCRIPTOR(PyObject *ob, SECURITY_DESCRIPTOR **ppSECURITY_DESCRIPTOR, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSECURITY_DESCRIPTOR = NULL;
	} else if (!PySECURITY_DESCRIPTOR_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PySECURITY_DESCRIPTOR object");
		return FALSE;
	} else {
		*ppSECURITY_DESCRIPTOR = ((PySECURITY_DESCRIPTOR *)ob)->GetSD();
	}
	return TRUE;
}

PyObject *PyWinObject_FromSECURITY_DESCRIPTOR(SECURITY_DESCRIPTOR *psd, unsigned cb /*=0*/)
{
	return new PySECURITY_DESCRIPTOR(psd, cb);
}

// @pymethod |PySECURITY_DESCRIPTOR|Initialize|Initialize the SD.
PyObject *PySECURITY_DESCRIPTOR::Initialize(PyObject *self, PyObject *args)
{
	PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
	if (!PyArg_ParseTuple(args, ":Initialize"))
		return NULL;
	if (!::InitializeSecurityDescriptor(This->m_psd, SECURITY_DESCRIPTOR_REVISION))
		return PyWin_SetAPIError("InitializeSecurityDescriptor");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PySECURITY_DESCRIPTOR|SetDacl|Sets information in a discretionary access-control list.
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl(PyObject *self, PyObject *args)
{
	PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
	BOOL bPresent, bDefaulted;
	PyObject *obACL;
	// @pyparm int|bPresent||A flag indicating if the SE_DACL_PRESENT flag should be set.
	// @pyparm <o PyACL>|acl||The ACL to set.  If None, a NULL ACL will be created allowing world access.
	// @pyparm int|bDaclDefaulted||A flag indicating if the SE_DACL_DEFAULTED flag should be set.
	if (!PyArg_ParseTuple(args, "iOi:SetDacl", &bPresent, &obACL, &bDefaulted))
		return NULL;
	PACL pacl;
	if (!PyWinObject_AsACL(obACL, &pacl))
		return NULL;
	if (!::SetSecurityDescriptorDacl(This->m_psd, bPresent, pacl, bDefaulted))
		return PyWin_SetAPIError("SetSecurityDescriptorDacl");
	Py_INCREF(Py_None);
	return Py_None;
	// @comm This method is also known by the alias SetSecurityDescriptorDacl
}


// @object PySECURITY_DESCRIPTOR|A Python object, representing a SECURITY_DESCRIPTOR structure
static struct PyMethodDef PySECURITY_DESCRIPTOR_methods[] = {
	{"Initialize",     PySECURITY_DESCRIPTOR::Initialize, 1}, 	// @pymeth Initialize|Initializes the object.
	{"SetSecurityDescriptorDacl",     PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl, 1},
	{"SetDacl",     PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl, 1}, 	// @pymeth SetDacl|Sets information in a discretionary access-control list.
	{NULL}
};

static PyBufferProcs PySECURITY_DESCRIPTOR_as_buffer = {
	(getreadbufferproc)PySECURITY_DESCRIPTOR::getreadbuf,
	(getwritebufferproc)0,
	(getsegcountproc)PySECURITY_DESCRIPTOR::getsegcount,
	(getcharbufferproc)0,
};

PYWINTYPES_EXPORT PyTypeObject PySECURITY_DESCRIPTORType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySECURITY_DESCRIPTOR",
	sizeof(PySECURITY_DESCRIPTOR),
	0,
	PySECURITY_DESCRIPTOR::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySECURITY_DESCRIPTOR::getattr,				/* tp_getattr */
	PySECURITY_DESCRIPTOR::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
	0,		/*tp_getattro*/
	0,		/*tp_setattro*/
	// @comm Note the PySECURITY_DESCRIPTOR object supports the buffer interface.  Thus buffer(sd) can be used to obtain the raw bytes.
	&PySECURITY_DESCRIPTOR_as_buffer,	/*tp_as_buffer*/
};

#define OFF(e) offsetof(PySECURITY_DESCRIPTOR, e)

/*static*/ struct memberlist PySECURITY_DESCRIPTOR::memberlist[] = {
	{NULL}	/* Sentinel */
};

PySECURITY_DESCRIPTOR::PySECURITY_DESCRIPTOR(unsigned cb /*= 0*/)
{
	ob_type = &PySECURITY_DESCRIPTORType;
	_Py_NewReference(this);
	cb = max(cb, SECURITY_DESCRIPTOR_MIN_LENGTH);
	m_psd = (SECURITY_DESCRIPTOR *)malloc(cb);
	::InitializeSecurityDescriptor(m_psd, SECURITY_DESCRIPTOR_REVISION);
}

PySECURITY_DESCRIPTOR::PySECURITY_DESCRIPTOR(const SECURITY_DESCRIPTOR *psd, unsigned cb /*= 0*/)
{
	ob_type = &PySECURITY_DESCRIPTORType;
	_Py_NewReference(this);
	if (cb==0) cb = GetSecurityDescriptorLength((void *)psd);
	m_psd = (SECURITY_DESCRIPTOR *)malloc(cb);
	memcpy(m_psd, psd, cb);
}

PySECURITY_DESCRIPTOR::~PySECURITY_DESCRIPTOR(void)
{
	if (m_psd) free(m_psd);
}


PyObject *PySECURITY_DESCRIPTOR::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PySECURITY_DESCRIPTOR_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	return PyMember_Get((char *)self, memberlist, name);
}

int PySECURITY_DESCRIPTOR::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete SECURITY_DESCRIPTOR attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PySECURITY_DESCRIPTOR::deallocFunc(PyObject *ob)
{
	delete (PySECURITY_DESCRIPTOR *)ob;
}

/*static*/ int PySECURITY_DESCRIPTOR::getreadbuf(PyObject *self, int index, const void **ptr)
{
	if ( index != 0 ) {
		PyErr_SetString(PyExc_SystemError,
				"accessing non-existent SID segment");
		return -1;
	}
	PySECURITY_DESCRIPTOR *pysd = (PySECURITY_DESCRIPTOR *)self;
	*ptr = pysd->m_psd;
	return GetSecurityDescriptorLength(pysd->m_psd);
}

/*static*/ int PySECURITY_DESCRIPTOR::getsegcount(PyObject *self, int *lenp)
{
	if ( lenp )
		*lenp = GetSecurityDescriptorLength(((PySECURITY_DESCRIPTOR *)self)->m_psd);
	return 1;
}

#endif /* MS_WINCE */