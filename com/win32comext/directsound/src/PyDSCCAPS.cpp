//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSCCAPS>|directsound|DSCCAPS|Creates a new PyDSCCAPS object
PyObject *PyWinMethod_NewDSCCAPS(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSCCAPS"))
		return NULL;
	return new PyDSCCAPS();
}

PyObject *PyWinObject_FromDSCCAPS(const DSCCAPS &caps)
{
	return new PyDSCCAPS(caps);
}

BOOL PyWinObject_AsDSCCAPS(PyObject *ob, DSCCAPS **ppDSCCAPS, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSCCAPS = NULL;
	} else if (!PyDSCCAPS_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSCCAPS object");
		return FALSE;
	} else {
		PyDSCCAPS *pycaps= (PyDSCCAPS *)ob;
		*ppDSCCAPS = pycaps->GetCAPS();
	}
	return TRUE;
}


// @object PyDSCCAPS|A Python object, representing a DSCCAPS structure
static struct PyMethodDef PyDSCCAPS_methods[] = {
	{NULL}
};

PyTypeObject PyDSCCAPSType =
{
	PYWIN_OBJECT_HEAD
	"PyDSCCAPSType",
	sizeof(PyDSCCAPSType),
	0,
	PyDSCCAPS::deallocFunc,
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
	PyDSCCAPS::members,
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


#define OFF(e) offsetof(PyDSCCAPS, e)

/*static*/ struct PyMemberDef PyDSCCAPS::members[] = {
	{"dwFlags",  T_INT,  OFF(m_caps.dwFlags), 0, "Specifies device capabilities. Can be 0 or DSCCAPS_EMULDRIVER (indicates that no DirectSoundCapture device is available and standard wave audio functions are being used)"}, 
	// @prop integer|dwFlags|Specifies device capabilities. Can be zero or the following flag:
		// @flagh Flag|Description
		// @flag DSCCAPS_EMULDRIVER|Indicates that no DirectSound Device is available and standard wave audio functions are being used.
	{"dwFormats",  T_INT,  OFF(m_caps.dwFormats), 0, "Supported WAVE_FORMAT formats."}, 
	// @prop integer|dwFormats|Bitset of supported WAVE_FORMAT formats.
	{"dwChannels",  T_INT,  OFF(m_caps.dwChannels), 0, "Number of channels supported by the device."}, 
	// @prop integer|dwChannels|Number of channels supported by the device.
	{NULL}
};

PyDSCCAPS::PyDSCCAPS(void)
{
	ob_type = &PyDSCCAPSType;
	_Py_NewReference(this);
	memset(&m_caps, 0, sizeof(m_caps));
}

PyDSCCAPS::PyDSCCAPS(const DSCCAPS &caps)
{
	ob_type = &PyDSCCAPSType;
	_Py_NewReference(this);
	m_caps = caps;
	m_caps.dwSize = sizeof(DSCCAPS);
}

PyDSCCAPS::~PyDSCCAPS()
{
}

/*static*/ void PyDSCCAPS::deallocFunc(PyObject *ob)
{
	delete (PyDSCCAPS *)ob;
}

