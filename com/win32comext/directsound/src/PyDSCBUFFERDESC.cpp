//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSCBUFFERDESC>|directsound|DSCBUFFERDESC|Creates a new PyDSCBUFFERDESC object
PyObject *PyWinMethod_NewDSCBUFFERDESC(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSCBUFFERDESC"))
		return NULL;
	return new PyDSCBUFFERDESC();
}

PyObject *PyWinObject_FromDSCBUFFERDESC(const DSCBUFFERDESC &dscbd)
{
	return new PyDSCBUFFERDESC(dscbd);
}

BOOL PyWinObject_AsDSCBUFFERDESC(PyObject *ob, DSCBUFFERDESC **ppDSCBUFFERDESC, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSCBUFFERDESC = NULL;
	} else if (!PyDSCBUFFERDESC_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSCBUFFERDESC object");
		return FALSE;
	} else {
		PyDSCBUFFERDESC *pydscbd= (PyDSCBUFFERDESC *)ob;
		*ppDSCBUFFERDESC = &pydscbd->m_dscbd;

		// in case the PyWAVEFORMATEX has been manipulated and points to a different address now
		((DSCBUFFERDESC *)*ppDSCBUFFERDESC)->lpwfxFormat =
			&((PyWAVEFORMATEX *)pydscbd->m_obWFX)->m_wfx;

	}
	return TRUE;
}


// @object PyDSCBUFFERDESC|A Python object, representing a DSCBUFFERDESC structure
static struct PyMethodDef PyDSCBUFFERDESC_methods[] = {
	{NULL}
};

PyTypeObject PyDSCBUFFERDESCType =
{
	PYWIN_OBJECT_HEAD
	"PyDSCBUFFERDESC",
	sizeof(PyDSCBUFFERDESC),
	0,
	PyDSCBUFFERDESC::deallocFunc,
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
	PyDSCBUFFERDESC::setattro,
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
	PyDSCBUFFERDESC::members,
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

#define OFF(e) offsetof(PyDSCBUFFERDESC, e)

/*static*/ struct PyMemberDef PyDSCBUFFERDESC::members[] = {
	{"dwFlags",  T_INT,  OFF(m_dscbd.dwFlags), 0, "Identifies the capabilities to include when creating a new DirectSoundBuffer object"}, 
		// @prop integer|dwFlags|Identifies the capabilities to include when creating a new DirectSoundBuffer object. Can be zero or the following flag:
		// @flagh Flag|Description
		// @flag DSCBCAPS_WAVEMAPPED|The Win32 wave mapper will be used for formats not supported by the device.
	{"dwBufferBytes",  T_INT,  OFF(m_dscbd.dwBufferBytes), 0, "Size of the new buffer, in bytes. This value must be 0 when creating primary buffers. For secondary buffers, the minimum and maximum sizes allowed are specified by DSBSIZE_MIN and DSBSIZE_MAX"}, 
		// @prop integer|dwBufferBytes|Size of the new buffer, in bytes. This value must be 0 when creating primary buffers. For secondary buffers, the minimum and maximum sizes allowed are specified by DSBSIZE_MIN and DSBSIZE_MAX.
	{"lpwfxFormat", T_OBJECT, OFF(m_obWFX), 0, "Structure specifying the waveform format for the buffer. This value must be None for primary buffers. The application can use IDirectSoundCaptureBuffer::SetFormat to set the format of the primary buffer."},
		// @prop WAVEFORMATEX|lpwfxFormat|Structure specifying the waveform format for the buffer. This value must be None for primary buffers. The application can use IDirectSoundBuffer::SetFormat to set the format of the primary buffer.
	{NULL}	/* Sentinel */
};

PyDSCBUFFERDESC::PyDSCBUFFERDESC(void)
{
	ob_type = &PyDSCBUFFERDESCType;
	_Py_NewReference(this);
	memset(&m_dscbd, 0, sizeof(m_dscbd));
	m_dscbd.dwSize = sizeof(DSCBUFFERDESC);
	Py_INCREF(Py_None);
	m_obWFX = Py_None;
}

PyDSCBUFFERDESC::PyDSCBUFFERDESC(const DSCBUFFERDESC &dscbd)
{
	m_dscbd.dwSize = sizeof(DSCBUFFERDESC);
	ob_type = &PyDSCBUFFERDESCType;
	_Py_NewReference(this);
	m_dscbd = dscbd;
	if (dscbd.lpwfxFormat) {
		m_obWFX = new PyWAVEFORMATEX(*dscbd.lpwfxFormat);
		m_dscbd.lpwfxFormat = &((PyWAVEFORMATEX*)m_obWFX)->m_wfx;
	}
	else {
		Py_INCREF(Py_None);
		m_obWFX = Py_None;
	}
}

PyDSCBUFFERDESC::~PyDSCBUFFERDESC()
{
	Py_XDECREF( m_obWFX );
}

/*static*/ void PyDSCBUFFERDESC::deallocFunc(PyObject *ob)
{
	delete (PyDSCBUFFERDESC *)ob;
}

int PyDSCBUFFERDESC::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
	PyDSCBUFFERDESC *obself = (PyDSCBUFFERDESC*)self;
	char *name=PYWIN_ATTR_CONVERT(obname);

	if (name==NULL)
		return -1;

	if (strcmp(name,"lpwfxFormat") == 0) {
		if (obvalue == Py_None)
		{
			obself->m_dscbd.lpwfxFormat = NULL;
		}
		else if (!PyWAVEFORMATEX_Check(obvalue)) {
			PyErr_SetString(PyExc_ValueError,"lpwfxFormat must be a WAVEFORMATEX instance");
			return -1;
		}
		else {
			obself->m_dscbd.lpwfxFormat = &((PyWAVEFORMATEX*)obvalue)->m_wfx;
		}
	}

	return PyObject_GenericSetAttr(self, obname, obvalue);
}

