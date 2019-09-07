//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSBCAPS>|directsound|DSBCAPS|Creates a new PyDSBCAPS object
PyObject *PyWinMethod_NewDSBCAPS(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":DSBCAPS"))
        return NULL;
    return new PyDSBCAPS();
}

PyObject *PyWinObject_FromDSBCAPS(const DSBCAPS &caps) { return new PyDSBCAPS(caps); }

BOOL PyWinObject_AsDSBCAPS(PyObject *ob, DSBCAPS **ppDSBCAPS, BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppDSBCAPS = NULL;
    }
    else if (!PyDSBCAPS_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyDSBCAPS object");
        return FALSE;
    }
    else {
        PyDSBCAPS *pycaps = (PyDSBCAPS *)ob;
        *ppDSBCAPS = pycaps->GetCAPS();
    }
    return TRUE;
}

// @object PyDSBCAPS|A Python object, representing a DSBCAPS structure
static struct PyMethodDef PyDSBCAPS_methods[] = {{NULL}};

PyTypeObject PyDSBCAPSType = {
    PYWIN_OBJECT_HEAD "PyDSBCAPS",
    sizeof(PyDSBCAPS),
    0,
    PyDSBCAPS::deallocFunc,
    0,  // tp_print;
    0,  // tp_getattr
    0,  // tp_setattr
    0,  // tp_compare
    0,  // tp_repr
    0,  // tp_as_number
    0,  // tp_as_sequence
    0,  // tp_as_mapping
    0,
    0, /* tp_call */
    0, /* tp_str */
    PyObject_GenericGetAttr,
    PyObject_GenericSetAttr,
    0,                                         // tp_as_buffer;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags;
    0,                                         // tp_doc; /* Documentation string */
    0,                                         // traverseproc tp_traverse;
    0,                                         // tp_clear;
    0,                                         // tp_richcompare;
    0,                                         // tp_weaklistoffset;
    0,                                         // tp_iter
    0,                                         // iternextfunc tp_iternext
    0,                                         // methods
    PyDSBCAPS::members,
    0,  // tp_getset;
    0,  // tp_base;
    0,  // tp_dict;
    0,  // tp_descr_get;
    0,  // tp_descr_set;
    0,  // tp_dictoffset;
    0,  // tp_init;
    0,  // tp_alloc;
    0   // newfunc tp_new;
};

#define OFF(e) offsetof(PyDSBCAPS, e)

/*static*/ struct PyMemberDef PyDSBCAPS::members[] = {
    {"dwFlags", T_INT, OFF(m_caps.dwFlags), 0,
     "Flags that specify buffer-object capabilities."},  // @prop integer|dwFlags|Flags that specify buffer-object
                                                         // capabilities.
                                                         // @flagh Flag|Description
                                                         // @flag DSBCAPS_PRIMARYBUFFER|Indicates that the buffer is a
                                                         // primary sound buffer. If this value is not specified, a
                                                         // secondary sound buffer will be created.
                                                         // @flag DSBCAPS_STATIC|Indicates that the buffer will be used
                                                         // for static sound data. Typically, these buffers are loaded
                                                         // once and played many times. These buffers are candidates for
                                                         // hardware memory.
                                                         // @flag DSBCAPS_LOCHARDWARE|The buffer is in hardware memory
                                                         // and uses hardware mixing.
                                                         // @flag DSBCAPS_LOCSOFTWARE|The buffer is in software memory
                                                         // and uses software mixing.
                                                         // @flag DSBCAPS_CTRL3D|The buffer is either a primary buffer
                                                         // or a secondary buffer that uses 3-D control. To create a
                                                         // primary buffer, the dwFlags member of the DSBUFFERDESC
                                                         // structure should include the DSBCAPS_PRIMARYBUFFER flag.
                                                         // @flag DSBCAPS_CTRLFREQUENCY|The buffer must have frequency
                                                         // control capability.
                                                         // @flag DSBCAPS_CTRLPAN|The buffer must have pan control
                                                         // capability.
                                                         // @flag DSBCAPS_CTRLVOLUME|The buffer must have volume control
                                                         // capability.
                                                         // @flag DSBCAPS_CTRLPOSITIONNOTIFY|The buffer must have
                                                         // control position notify capability.
                                                         // @flag DSBCAPS_STICKYFOCUS|Changes the focus behavior of the
                                                         // sound buffer. This flag can be specified in an
                                                         // IDirectSound::CreateSoundBuffer call. With this flag set, an
                                                         // application using DirectSound can continue to play its
                                                         // sticky focus buffers if the user switches to another
                                                         // application not using DirectSound. In this situation, the
                                                         // application's normal buffers are muted, but the sticky focus
                                                         // buffers are still audible. This is useful for nongame
                                                         // applications, such as movie playback (DirectShow™), when the
                                                         // user wants to hear the soundtrack while typing in Microsoft
                                                         // Word or Microsoft® Excel, for example. However, if the user
                                                         // switches to another DirectSound application, all sound
                                                         // buffers, both normal and sticky focus, in the previous
                                                         // application are muted.
                                                         // @flag DSBCAPS_GLOBALFOCUS|The buffer is a global sound
                                                         // buffer. With this flag set, an application using DirectSound
                                                         // can continue to play its buffers if the user switches focus
                                                         // to another application, even if the new application uses
                                                         // DirectSound. The one exception is if you switch focus to a
                                                         // DirectSound application that uses the DSSCL_EXCLUSIVE or
                                                         // DSSCL_WRITEPRIMARY flag for its cooperative level. In this
                                                         // case, the global sounds from other applications will not be
                                                         // audible.
                                                         // @flag DSBCAPS_GETCURRENTPOSITION2|Indicates that
                                                         // IDirectSoundBuffer::GetCurrentPosition should use the new
                                                         // behavior of the play cursor. In DirectSound in DirectX 1,
                                                         // the play cursor was significantly ahead of the actual
                                                         // playing sound on emulated sound cards; it was directly
                                                         // behind the write cursor. Now, if the
                                                         // DSBCAPS_GETCURRENTPOSITION2 flag is specified, the
                                                         // application can get a more accurate play position. If this
                                                         // flag is not specified, the old behavior is preserved for
                                                         // compatibility. Note that this flag affects only emulated
                                                         // sound cards; if a DirectSound driver is present, the play
                                                         // cursor is accurate for DirectSound in all versions of
                                                         // DirectX.
                                                         // @flag DSBCAPS_MUTE3DATMAXDISTANCE|The sound is reduced to
                                                         // silence at the maximum distance. The buffer will stop
                                                         // playing when the maximum distance is exceeded, so that
                                                         // processor time is not wasted.
    {"dwBufferBytes", T_INT, OFF(m_caps.dwBufferBytes), 0,
     "Size of the buffer, in bytes"},  // @prop integer|nChannels|Size of the buffer, in bytes.
    {"dwUnlockTransferRate", T_INT, OFF(m_caps.dwUnlockTransferRate), 0,
     "Specifies the rate, in kilobytes per second, at which data is transferred to the buffer memory when "
     "IDirectSoundBuffer::Update is called. High-performance applications can use this value to determine the time "
     "required for IDirectSoundBuffer::Update to execute. For software buffers located in system memory, the rate will "
     "be very high because no processing is required. For hardware buffers, the rate might be slower because the "
     "buffer might have to be downloaded to the sound card, which might have a limited transfer rate."},
    // @prop integer|dwUnlockTransferRate|Specifies the rate, in kilobytes per second, at which data is transferred to
    // the buffer memory when IDirectSoundBuffer::Unlock is called. High-performance applications can use this value to
    // determine the time required for IDirectSoundBuffer::Unlock to execute. For software buffers located in system
    // memory, the rate will be very high because no processing is required. For hardware buffers, the rate might be
    // slower because the buffer might have to be downloaded to the sound card, which might have a limited transfer
    // rate.
    {"dwPlayCpuOverhead", T_INT,
     OFF(m_caps.dwPlayCpuOverhead)},  // @prop integer|nAvgBytesPerSec|Specifies whether the returned handle is
                                      // inherited when a new process is created. If this member is TRUE, the new
                                      // process inherits the handle.
    {NULL}                            /* Sentinel */
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

PyDSBCAPS::~PyDSBCAPS() {}

/*static*/ void PyDSBCAPS::deallocFunc(PyObject *ob) { delete (PyDSBCAPS *)ob; }
