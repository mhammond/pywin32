//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSBCAPS>|pywintypes|DSBCAPS|Creates a new DSBCAPS object
PyObject *PyWinMethod_NewDSBCAPS(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSBCAPS"))
		return NULL;
	return new PyDSBCAPS();
}

PyObject *PyWinObject_FromDSBCAPS(const DSBCAPS &caps)
{
	return new PyDSBCAPS(caps);
}

BOOL PyWinObject_AsDSBCAPS(PyObject *ob, DSBCAPS **ppDSBCAPS, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSBCAPS = NULL;
	} else if (!PyDSBCAPS_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSBCAPS object");
		return FALSE;
	} else {
		PyDSBCAPS *pycaps= (PyDSBCAPS *)ob;
		*ppDSBCAPS = pycaps->GetCAPS();
	}
	return TRUE;
}


// @object PyDSBCAPS|A Python object, representing a DSBCAPS structure
static struct PyMethodDef PyDSBCAPS_methods[] = {
	{NULL}
};


PyTypeObject PyDSBCAPSType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyDSBCAPS",
	sizeof(PyDSBCAPS),
	0,
	PyDSBCAPS::deallocFunc,
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
	PyDSBCAPS::members,
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

#define OFF(e) offsetof(PyDSBCAPS, e)

/*static*/ struct PyMemberDef PyDSBCAPS::members[] = {
	{"dwFlags",  T_INT,  OFF(m_caps.dwFlags), 0, "Flags that specify buffer-object capabilities."}, // @prop integer|dwFlags|Flags that specify buffer-object capabilities.
	{"dwBufferBytes",  T_INT,  OFF(m_caps.dwBufferBytes), 0, "Size of the buffer, in bytes"}, // @prop integer|nChannels|Size of the buffer, in bytes.
	{"dwUnlockTransferRate",  T_INT,  OFF(m_caps.dwUnlockTransferRate), 0, 
		"Specifies the rate, in kilobytes per second, at which data is transferred to the buffer memory when IDirectSoundBuffer::Update is called. High-performance applications can use this value to determine the time required for IDirectSoundBuffer::Update to execute. For software buffers located in system memory, the rate will be very high because no processing is required. For hardware buffers, the rate might be slower because the buffer might have to be downloaded to the sound card, which might have a limited transfer rate."}, 
		// @prop integer|dwUnlockTransferRate|Specifies the rate, in kilobytes per second, at which data is transferred to the buffer memory when IDirectSoundBuffer::Unlock is called. High-performance applications can use this value to determine the time required for IDirectSoundBuffer::Unlock to execute. For software buffers located in system memory, the rate will be very high because no processing is required. For hardware buffers, the rate might be slower because the buffer might have to be downloaded to the sound card, which might have a limited transfer rate. 
	{"dwPlayCpuOverhead",  T_INT,  OFF(m_caps.dwPlayCpuOverhead)}, // @prop integer|nAvgBytesPerSec|Specifies whether the returned handle is inherited when a new process is created. If this member is TRUE, the new process inherits the handle.
	{NULL}	/* Sentinel */
};

PyDSBCAPS::PyDSBCAPS(void)
{
	ob_type = &PyDSBCAPSType;
	_Py_NewReference(this);
	memset(&m_caps, 0, sizeof(m_caps));
	m_caps.dwSize = sizeof(DSBCAPS);
}

PyDSBCAPS::PyDSBCAPS(const DSBCAPS &caps)
{
	ob_type = &PyDSBCAPSType;
	_Py_NewReference(this);
	m_caps = caps;
	m_caps.dwSize = sizeof(DSBCAPS);
}

PyDSBCAPS::~PyDSBCAPS()
{
}

/*static*/ void PyDSBCAPS::deallocFunc(PyObject *ob)
{
	delete (PyDSBCAPS *)ob;
}

