//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "assert.h"
#include "structmember.h"

// @pymethod <o PyOVERLAPPED>|pywintypes|OVERLAPPED|Creates a new OVERLAPPED object
PyObject *PyWinMethod_NewOVERLAPPED(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":OVERLAPPED"))
		return NULL;
	return new PyOVERLAPPED();
}

// @object PyOVERLAPPED|A Python object, representing an overlapped structure
// @comm Typically you create a PyOVERLAPPED object, and set its hEvent property.
// The object can then be passed to any function which takes an OVERLAPPED object, and
// the object attributes will be automatically updated.
PYWINTYPES_EXPORT BOOL PyWinObject_AsOVERLAPPED(PyObject *ob, OVERLAPPED **ppOverlapped, BOOL bNoneOK /*= TRUE*/)
{
	PyOVERLAPPED *po = NULL;
	if (!PyWinObject_AsPyOVERLAPPED(ob, &po, bNoneOK))
		return FALSE;
	if (bNoneOK && po==NULL) {
		*ppOverlapped = NULL;
		return TRUE;
	}
	assert(po);
	if (!po)
		return FALSE;
	*ppOverlapped = po->GetOverlapped();
	return TRUE;
}

PYWINTYPES_EXPORT BOOL PyWinObject_AsPyOVERLAPPED(PyObject *ob, PyOVERLAPPED **ppOverlapped, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppOverlapped = NULL;
	} else if (!PyOVERLAPPED_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyOVERLAPPED object");
		return FALSE;
	} else {
		*ppOverlapped = ((PyOVERLAPPED *)ob);
	}
	return TRUE;
}

PYWINTYPES_EXPORT PyObject *PyWinObject_FromOVERLAPPED(const OVERLAPPED *pOverlapped)
{
	if (pOverlapped==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyOVERLAPPED::sMyOverlapped myo(*pOverlapped);
	PyObject *ret = new PyOVERLAPPED(&myo);
	if(ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "Allocating pOverlapped");
	return ret;
}

PYWINTYPES_EXPORT PyTypeObject PyOVERLAPPEDType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyOVERLAPPED",
	sizeof(PyOVERLAPPED),
	0,
	PyOVERLAPPED::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyOVERLAPPED::getattr,				/* tp_getattr */
	PyOVERLAPPED::setattr,				/* tp_setattr */
	// @pymeth __cmp__|Used when OVERLAPPED objects are compared.
	PyOVERLAPPED::compareFunc,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyOVERLAPPED, e)

/*static*/ struct memberlist PyOVERLAPPED::memberlist[] = {
	{"Internal",    T_INT,      OFF(m_overlapped.Internal)}, // @prop integer|Internal|Reserved for operating system use.
	{"InternalHigh",T_INT,      OFF(m_overlapped.InternalHigh)}, // @prop integer|InternalHigh|Reserved for operating system use.
	{"Offset",      T_INT,      OFF(m_overlapped.Offset)}, // @prop integer|Offset|Specifies a file position at which to start the transfer. The file position is a byte offset from the start of the file. The calling process sets this member before calling the ReadFile or WriteFile function. This member is ignored when reading from or writing to named pipes and communications devices.
	{"OffsetHigh",  T_INT,      OFF(m_overlapped.OffsetHigh)}, // @prop integer|OffsetHigh|Specifies the high word of the byte offset at which to start the transfer.
	{NULL}
};
// @prop integer/<o PyHANDLE>|hEvent|Identifies an event set to the signaled state when the transfer has been completed. The calling process sets this member before calling the <om win32file.ReadFile>, <om win32file.WriteFile>, <om win32pipe.ConnectNamedPipe>, or <om win32pipe.TransactNamedPipe> function.
// @prop object|object|Any python object that you want to attach to your overlapped I/O request.

PyOVERLAPPED::PyOVERLAPPED(void)
{
	ob_type = &PyOVERLAPPEDType;
	_Py_NewReference(this);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_obHandle = NULL;
}

PyOVERLAPPED::PyOVERLAPPED(const sMyOverlapped *pO)
{
	ob_type = &PyOVERLAPPEDType;
	_Py_NewReference(this);
	m_overlapped = *pO;
	Py_XINCREF(m_overlapped.obState);
	m_obHandle = NULL;
}

PyOVERLAPPED::~PyOVERLAPPED(void)
{
	Py_XDECREF(m_obHandle);
	Py_XDECREF(m_overlapped.obState);
}

int PyOVERLAPPED::compare(PyObject *ob)
{
	return memcmp(&m_overlapped, &((PyOVERLAPPED *)ob)->m_overlapped, sizeof(m_overlapped));
}

// @pymethod int|PyOVERLAPPED|__cmp__|Used when objects are compared.
int PyOVERLAPPED::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyOVERLAPPED *)ob1)->compare(ob2);
}

PyObject *PyOVERLAPPED::getattr(PyObject *self, char *name)
{
	// @prop integer/<o PyHANDLE>|hEvent|Identifies an event set to the signaled state when the transfer has been completed. The calling process sets this member before calling the <om win32file.ReadFile>, <om win32file.WriteFile>, <om win32pipe.ConnectNamedPipe>, or <om win32pipe.TransactNamedPipe> function.
	if (strcmp("hEvent", name)==0) {
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		if (pO->m_obHandle) {
			Py_INCREF(pO->m_obHandle);
			return pO->m_obHandle;
		}
		return PyInt_FromLong((long)pO->m_overlapped.hEvent);
	}
// @prop object|object|Any python object that you want to attach to your overlapped I/O request.
	else if (strcmp("object", name) == 0)
	{
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
			
		if (pO->m_overlapped.obState)
		{
			Py_INCREF(pO->m_overlapped.obState);
			return pO->m_overlapped.obState;
		}
		Py_INCREF(Py_None);
		return Py_None;
	}
// @prop int|dword|An integer buffer that may be used by overlapped functions (eg, <om win32file.WaitCommEvent>)
	else if (strcmp("dword", name) == 0)
	{
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		return PyInt_FromLong(pO->m_overlapped.dwValue);
	}
	return PyMember_Get((char *)self, memberlist, name);
}

int PyOVERLAPPED::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete OVERLAPPED attributes");
		return -1;
	}
	if (strcmp("hEvent", name)==0) {
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		Py_XDECREF(pO->m_obHandle);
		pO->m_obHandle = NULL;
		if (PyHANDLE_Check(v)) {
			pO->m_obHandle = v;
			PyWinObject_AsHANDLE(v, &pO->m_overlapped.hEvent, FALSE);
			Py_INCREF(v);
		} else if (PyInt_Check(v)) {
			pO->m_overlapped.hEvent = (HANDLE)PyInt_AsLong(v);
		}
		return 0;
	}
	else if (strcmp("object", name) == 0)
	{
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		Py_XDECREF(pO->m_overlapped.obState);
		Py_INCREF(v);
		pO->m_overlapped.obState = v;
		return 0;
	}
	else if (strcmp("dword", name) == 0)
	{
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		PyErr_Clear();
		pO->m_overlapped.dwValue = PyInt_AsLong(v);
		if (PyErr_Occurred())
			PyErr_SetString(PyExc_TypeError, "The 'dword' value must be an integer");
		return 0;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyOVERLAPPED::deallocFunc(PyObject *ob)
{
	delete (PyOVERLAPPED *)ob;
}
