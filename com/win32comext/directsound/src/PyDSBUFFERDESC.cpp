//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSBUFFERDESC>|directsound|DSBUFFERDESC|Creates a new PyDSBUFFERDESC object
PyObject *PyWinMethod_NewDSBUFFERDESC(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSBUFFERDESC"))
		return NULL;
	return new PyDSBUFFERDESC();
}

PyObject *PyWinObject_FromDSBUFFERDESC(const DSBUFFERDESC &dsbd)
{
	return new PyDSBUFFERDESC(dsbd);
}

BOOL PyWinObject_AsDSBUFFERDESC(PyObject *ob, DSBUFFERDESC **ppDSBUFFERDESC, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSBUFFERDESC = NULL;
	} else if (!PyDSBUFFERDESC_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSBUFFERDESC object");
		return FALSE;
	} else {
		PyDSBUFFERDESC *pydsbd= (PyDSBUFFERDESC *)ob;
		*ppDSBUFFERDESC = &pydsbd->m_dsbd;

		// in case the PyWAVEFORMATEX has been manipulated and points to a different address now
		((DSBUFFERDESC *)*ppDSBUFFERDESC)->lpwfxFormat =
			&((PyWAVEFORMATEX *)pydsbd->m_obWFX)->m_wfx;

	}
	return TRUE;
}


// @object PyDSBUFFERDESC|A Python object, representing a DSBUFFERDESC structure
static struct PyMethodDef PyDSBUFFERDESC_methods[] = {
	{NULL}
};

PyTypeObject PyDSBUFFERDESCType =
{
	PYWIN_OBJECT_HEAD
	"PyDSBUFFERDESC",
	sizeof(PyDSBUFFERDESC),
	0,
	PyDSBUFFERDESC::deallocFunc,
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
	PyDSBUFFERDESC::setattro,
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
	PyDSBUFFERDESC::members,
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

#define OFF(e) offsetof(PyDSBUFFERDESC, e)

/*static*/ struct PyMemberDef PyDSBUFFERDESC::members[] = {
	{"dwFlags",  T_INT,  OFF(m_dsbd.dwFlags), 0, "Identifies the capabilities to include when creating a new DirectSoundBuffer object"}, 
		// @prop integer|dwFlags|Identifies the capabilities to include when creating a new DirectSoundBuffer object. Specify one or more of the following:
		// @flagh Flag|Description
		// @flag DSBCAPS_PRIMARYBUFFER|Indicates that the buffer is a primary sound buffer. If this value is not specified, a secondary sound buffer will be created. 
		// @flag DSBCAPS_STATIC|Indicates that the buffer will be used for static sound data. Typically, these buffers are loaded once and played many times. These buffers are candidates for hardware memory. 
		// @flag DSBCAPS_LOCHARDWARE|The buffer is in hardware memory and uses hardware mixing. 
		// @flag DSBCAPS_LOCSOFTWARE|The buffer is in software memory and uses software mixing. 
		// @flag DSBCAPS_CTRL3D|The buffer is either a primary buffer or a secondary buffer that uses 3-D control. To create a primary buffer, the dwFlags member of the DSBUFFERDESC structure should include the DSBCAPS_PRIMARYBUFFER flag. 
		// @flag DSBCAPS_CTRLFREQUENCY|The buffer must have frequency control capability. 
		// @flag DSBCAPS_CTRLPAN|The buffer must have pan control capability. 
		// @flag DSBCAPS_CTRLVOLUME|The buffer must have volume control capability. 
		// @flag DSBCAPS_CTRLPOSITIONNOTIFY|The buffer must have control position notify capability. 
		// @flag DSBCAPS_STICKYFOCUS|Changes the focus behavior of the sound buffer. This flag can be specified in an IDirectSound::CreateSoundBuffer call. With this flag set, an application using DirectSound can continue to play its sticky focus buffers if the user switches to another application not using DirectSound. In this situation, the application's normal buffers are muted, but the sticky focus buffers are still audible. This is useful for nongame applications, such as movie playback (DirectShow™), when the user wants to hear the soundtrack while typing in Microsoft Word or Microsoft® Excel, for example. However, if the user switches to another DirectSound application, all sound buffers, both normal and sticky focus, in the previous application are muted. 
		// @flag DSBCAPS_GLOBALFOCUS|The buffer is a global sound buffer. With this flag set, an application using DirectSound can continue to play its buffers if the user switches focus to another application, even if the new application uses DirectSound. The one exception is if you switch focus to a DirectSound application that uses the DSSCL_EXCLUSIVE or DSSCL_WRITEPRIMARY flag for its cooperative level. In this case, the global sounds from other applications will not be audible. 
		// @flag DSBCAPS_GETCURRENTPOSITION2|Indicates that IDirectSoundBuffer::GetCurrentPosition should use the new behavior of the play cursor. In DirectSound in DirectX 1, the play cursor was significantly ahead of the actual playing sound on emulated sound cards; it was directly behind the write cursor. Now, if the DSBCAPS_GETCURRENTPOSITION2 flag is specified, the application can get a more accurate play position. If this flag is not specified, the old behavior is preserved for compatibility. Note that this flag affects only emulated sound cards; if a DirectSound driver is present, the play cursor is accurate for DirectSound in all versions of DirectX. 
		// @flag DSBCAPS_MUTE3DATMAXDISTANCE|The sound is reduced to silence at the maximum distance. The buffer will stop playing when the maximum distance is exceeded, so that processor time is not wasted. 
	{"dwBufferBytes",  T_INT,  OFF(m_dsbd.dwBufferBytes), 0, "Size of the new buffer, in bytes. This value must be 0 when creating primary buffers. For secondary buffers, the minimum and maximum sizes allowed are specified by DSBSIZE_MIN and DSBSIZE_MAX"}, 
		// @prop integer|dwBufferBytes|Size of the new buffer, in bytes. This value must be 0 when creating primary buffers. For secondary buffers, the minimum and maximum sizes allowed are specified by DSBSIZE_MIN and DSBSIZE_MAX.
	{"lpwfxFormat", T_OBJECT, OFF(m_obWFX), 0, "Structure specifying the waveform format for the buffer. This value must be None for primary buffers. The application can use IDirectSoundBuffer::SetFormat to set the format of the primary buffer."},
		// @prop WAVEFORMATEX|lpwfxFormat|Structure specifying the waveform format for the buffer. This value must be None for primary buffers. The application can use IDirectSoundBuffer::SetFormat to set the format of the primary buffer.
	{NULL}	/* Sentinel */
};

PyDSBUFFERDESC::PyDSBUFFERDESC(void)
{
	ob_type = &PyDSBUFFERDESCType;
	_Py_NewReference(this);
	memset(&m_dsbd, 0, sizeof(m_dsbd));
	m_dsbd.dwSize = sizeof(DSBUFFERDESC);
	Py_INCREF(Py_None);
	m_obWFX = Py_None;
}

PyDSBUFFERDESC::PyDSBUFFERDESC(const DSBUFFERDESC &dsbd)
{
	m_dsbd.dwSize = sizeof(DSBUFFERDESC);
	ob_type = &PyDSBUFFERDESCType;
	_Py_NewReference(this);
	m_dsbd = dsbd;
	if (dsbd.lpwfxFormat) {
		m_obWFX = new PyWAVEFORMATEX(*dsbd.lpwfxFormat);
		m_dsbd.lpwfxFormat = &((PyWAVEFORMATEX*)m_obWFX)->m_wfx;
	}
	else {
		Py_INCREF(Py_None);
		m_obWFX = Py_None;
	}
}

PyDSBUFFERDESC::~PyDSBUFFERDESC()
{
	Py_XDECREF( m_obWFX );
}

/*static*/ void PyDSBUFFERDESC::deallocFunc(PyObject *ob)
{
	delete (PyDSBUFFERDESC *)ob;
}

int PyDSBUFFERDESC::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
	PyDSBUFFERDESC *obself = (PyDSBUFFERDESC*)self;
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return -1;

	if (strcmp(name,"lpwfxFormat") == 0) {
		if (obvalue == Py_None)
		{
			obself->m_dsbd.lpwfxFormat = NULL;
		}
		else if (!PyWAVEFORMATEX_Check(obvalue)) {
			PyErr_SetString(PyExc_ValueError,"lpwfxFormat must be a WAVEFORMATEX instance");
			return -1;
		}
		else {
			obself->m_dsbd.lpwfxFormat = &((PyWAVEFORMATEX*)obvalue)->m_wfx;
		}
	}

	return PyObject_GenericSetAttr(self, obname, obvalue);
}

