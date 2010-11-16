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
	PYWIN_OBJECT_HEAD
	"PyOVERLAPPED",
	sizeof(PyOVERLAPPED),
	0,
	PyOVERLAPPED::deallocFunc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	PyOVERLAPPED::hashFunc,	/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyOVERLAPPED::getattro,	/* tp_getattro */
	PyOVERLAPPED::setattro,	/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	PyOVERLAPPED::richcompareFunc,			/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	0,						/* tp_methods */
	PyOVERLAPPED::members,	/* tp_members and tp_getset are apparently mutually exclusive, but this isn't documented anywhere */
	0,	//PyOVERLAPPED::getset,	/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

#define OFF(e) offsetof(PyOVERLAPPED, e)

/*static*/ struct PYWINTYPES_EXPORT PyMemberDef PyOVERLAPPED::members[] = {
	{"Offset",		T_ULONG,	OFF(m_overlapped.Offset)},		// @prop integer|Offset|Specifies a file position at which to start the transfer. The file position is a byte offset from the start of the file. The calling process sets this member before calling the ReadFile or WriteFile function. This member is ignored when reading from or writing to named pipes and communications devices.
	{"OffsetHigh",	T_ULONG,	OFF(m_overlapped.OffsetHigh)},	// @prop integer|OffsetHigh|Specifies the high word of the byte offset at which to start the transfer.
	{"object",		T_OBJECT,	OFF(m_overlapped.obState)},		// @prop object|object|Any python object that you want to attach to your overlapped I/O request.
	{"dword",		T_ULONG,	OFF(m_overlapped.dwValue)},		// @prop int|dword|An integer buffer that may be used by overlapped functions (eg, <om win32file.WaitCommEvent>)

	// These are handled by PyOVERLAPPED::getattro, included here so they show up as attributes
	{"hEvent",		T_OBJECT,	OFF(obDummy)},					// @prop <o PyHANDLE>|hEvent|Identifies an event set to the signaled state when the transfer has been completed. The calling process sets this member before calling the <om win32file.ReadFile>, <om win32file.WriteFile>, <om win32pipe.ConnectNamedPipe>, or <om win32pipe.TransactNamedPipe> function.
	{"Internal",	T_OBJECT,	OFF(obDummy)},					// @prop integer|Internal|Reserved for operating system use. (pointer-sized value)
	{"InternalHigh",T_OBJECT,	OFF(obDummy)},					// @prop integer|InternalHigh|Reserved for operating system use. (pointer-sized value)
	{NULL}
};

PyOVERLAPPED::PyOVERLAPPED(void)
{
	ob_type = &PyOVERLAPPEDType;
	_Py_NewReference(this);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	obDummy = NULL;
	m_obhEvent = NULL;
}

PyOVERLAPPED::PyOVERLAPPED(const sMyOverlapped *pO)
{
	ob_type = &PyOVERLAPPEDType;
	_Py_NewReference(this);
	m_overlapped = *pO;
	Py_XINCREF(m_overlapped.obState);
	m_obhEvent = NULL;
}

PyOVERLAPPED::~PyOVERLAPPED(void)
{
	Py_XDECREF(m_obhEvent);
	Py_XDECREF(m_overlapped.obState);
	// set our memory to zero, so our clunky check for an invalid
	// object in win32file has more chance of success.
	memset(this, 0, sizeof(PyOVERLAPPED));
}

PyObject *PyOVERLAPPED::richcompareFunc(PyObject *ob, PyObject *other, int op)
{
	PyOVERLAPPED::sMyOverlapped *mine = &((PyOVERLAPPED *)ob)->m_overlapped;
	PyOVERLAPPED::sMyOverlapped *oother;
	if (PyOVERLAPPED_Check(other)) {
		oother = &((PyOVERLAPPED *)other)->m_overlapped;
	} else {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	BOOL e = memcmp(mine, oother, sizeof(*mine))==0;
	PyObject *ret;
	if (op==Py_EQ)
		ret = e ? Py_True : Py_False;
	else if (op==Py_NE)
		ret = !e ? Py_True : Py_False;
	else
		ret = Py_NotImplemented;
	Py_INCREF(ret);
	return ret;
}

PyObject *PyOVERLAPPED::getattro(PyObject *self, PyObject *obname)
{
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	if (strcmp("hEvent", name)==0) {
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		if (pO->m_obhEvent) {
			Py_INCREF(pO->m_obhEvent);
			return pO->m_obhEvent;
		}
		return PyWinLong_FromHANDLE(pO->m_overlapped.hEvent);
	}
	if (strcmp("Internal", name) == 0){
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		return PyWinObject_FromULONG_PTR(pO->m_overlapped.Internal);
	}
	if (strcmp("InternalHigh", name) == 0){
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		return PyWinObject_FromULONG_PTR(pO->m_overlapped.InternalHigh);
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyOVERLAPPED::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete OVERLAPPED attributes");
		return -1;
	}
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	if (strcmp("hEvent", name)==0) {
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		// Use an intermediate so the original isn't lost if conversion fails
		HANDLE htmp;	
		if (!PyWinObject_AsHANDLE(v, &htmp))
			return -1;
		pO->m_overlapped.hEvent=htmp;
		Py_XDECREF(pO->m_obhEvent);
		if (PyHANDLE_Check(v)) {
			pO->m_obhEvent = v;
			Py_INCREF(v);
			}
		else
			pO->m_obhEvent = NULL;
		return 0;
	}
	if (strcmp("Internal", name)==0){
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		ULONG_PTR ul_tmp;
		if (!PyWinLong_AsULONG_PTR(v, &ul_tmp))
			return -1;
		pO->m_overlapped.Internal=ul_tmp;
		return 0;
	}
	if (strcmp("InternalHigh", name)==0){
		PyOVERLAPPED *pO = (PyOVERLAPPED *)self;
		ULONG_PTR ul_tmp;
		if (!PyWinLong_AsULONG_PTR(v, &ul_tmp))
			return -1;
		pO->m_overlapped.InternalHigh=ul_tmp;
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ Py_hash_t PyOVERLAPPED::hashFunc(PyObject *ob)
{
	// Just use the address.
	return _Py_HashPointer(ob);
}

/*static*/ void PyOVERLAPPED::deallocFunc(PyObject *ob)
{
	delete (PyOVERLAPPED *)ob;
}
