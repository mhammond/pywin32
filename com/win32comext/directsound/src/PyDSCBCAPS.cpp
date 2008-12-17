//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSCBCAPS>|directsound|DSCBCAPS|Creates a new PyDSCBCAPS object
PyObject *PyWinMethod_NewDSCBCAPS(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSCBCAPS"))
		return NULL;
	return new PyDSCBCAPS();
}

PyObject *PyWinObject_FromDSCBCAPS(const DSCBCAPS &caps)
{
	return new PyDSCBCAPS(caps);
}

BOOL PyWinObject_AsDSCBCAPS(PyObject *ob, DSCBCAPS **ppDSCBCAPS, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSCBCAPS = NULL;
	} else if (!PyDSCBCAPS_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSCBCAPS object");
		return FALSE;
	} else {
		PyDSCBCAPS *pycaps= (PyDSCBCAPS *)ob;
		*ppDSCBCAPS = pycaps->GetCAPS();
	}
	return TRUE;
}


// @object PyDSCBCAPS|A Python object, representing a DSCBCAPS structure
static struct PyMethodDef PyDSCBCAPS_methods[] = {
	{NULL}
};

PyTypeObject PyDSCBCAPSType =
{
	PYWIN_OBJECT_HEAD
	"PyDSCBCAPSType",
	sizeof(PyDSCBCAPSType),
	0,
	PyDSCBCAPS::deallocFunc,
	0,			// tp_print;
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// tp_as_number
	0,			// tp_as_sequence
	0,			// tp_as_mapping
	0,
	0,						/* tp_call */
	0,		/* tp_str */
	PyObject_GenericGetAttr,
	PyObject_GenericSetAttr,
	0,			// tp_as_buffer;
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags;
	0,			// tp_doc; /* Documentation string */
	0,			// traverseproc tp_traverse;
	0,			// tp_clear;
	0,			// tp_richcompare;
	0,			// tp_weaklistoffset;
	0,			// tp_iter
	0,			// iternextfunc tp_iternext
	0,			// methods
	PyDSCBCAPS::members,
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get;
	0,			// tp_descr_set;
	0,			// tp_dictoffset;
	0,			// tp_init;
	0,			// tp_alloc;
	0			// newfunc tp_new;
};


#define OFF(e) offsetof(PyDSCBCAPS, e)

/*static*/ struct PyMemberDef PyDSCBCAPS::members[] = {
	{"dwFlags",  T_INT,  OFF(m_caps.dwFlags), 0, "Specifies device capabilities. Can be 0 or DSCBCAPS_EMULDRIVER (indicates that no DirectSoundCapture device is available and standard wave audio functions are being used)"}, 
	// @prop integer|dwFlags|Specifies device capabilities. Can be 0 or DSCBCAPS_EMULDRIVER (indicates that no DirectSound Device is available and standard wave audio functions are being used).
	{"dwBufferBytes",  T_INT,  OFF(m_caps.dwBufferBytes), 0, "The size, in bytes, of the capture buffer."}, 
	// @prop integer|dwBufferBytes|The size, in bytes, of the capture buffer.
	{NULL}
};

PyDSCBCAPS::PyDSCBCAPS(void)
{
	ob_type = &PyDSCBCAPSType;
	_Py_NewReference(this);
	memset(&m_caps, 0, sizeof(m_caps));
}

PyDSCBCAPS::PyDSCBCAPS(const DSCBCAPS &caps)
{
	ob_type = &PyDSCBCAPSType;
	_Py_NewReference(this);
	m_caps = caps;
	m_caps.dwSize = sizeof(DSCBCAPS);
}

PyDSCBCAPS::~PyDSCBCAPS()
{
}

/*static*/ void PyDSCBCAPS::deallocFunc(PyObject *ob)
{
	delete (PyDSCBCAPS *)ob;
}

