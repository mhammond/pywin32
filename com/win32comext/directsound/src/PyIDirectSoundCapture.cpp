// This file implements the IDirectSoundCapture Interface for Python.

#include "directsound_pch.h"
#include "PySoundObjects.h"
#include "PyIDirectSoundCapture.h"
#include "PyIDirectSoundCaptureBuffer.h"
#include "PyIDirectSoundNotify.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDirectSoundCapture::PyIDirectSoundCapture(IUnknown *pdisp) : PyIUnknown(pdisp), m_DS(NULL) { ob_type = &type; }

PyIDirectSoundCapture::~PyIDirectSoundCapture()
{
    // Release should be called before IDirectSound::Release, which may be
    // triggered below
    SafeRelease(this);

    // This may trigger IDirectSound::Release
    if (m_DS)
        Py_DECREF(m_DS);
}

/* static */ IDirectSoundCapture *PyIDirectSoundCapture::GetI(PyObject *self)
{
    return (IDirectSoundCapture *)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIDirectSoundCapture::QueryInterface(PyObject *self, PyObject *args)
{
    PyObject *obiid;
    PyObject *obUseIID = NULL;
    if (!PyArg_ParseTuple(args, "O|O:QueryInterface", &obiid, &obUseIID))
        return NULL;

    PyObject *rc = PyIUnknown::QueryInterface(self, args);

    // Special treatment for PyIDirectSoundNotify

    // This is a workaround for a reference counting bug in IDirectSound:
    // If IDirectSound::Release() is called before IDirectSoundCapture::Release()
    // or IDirectSoundNotify::Release(), we will get an Access Violation

    // We work around this by manipulating the reference count on the Python objects
    // that encapsulate them
    if (PyIBase::is_object(rc, &PyIDirectSoundNotify::type)) {
        PyIDirectSoundNotify *notify = (PyIDirectSoundNotify *)rc;
        PyIDirectSoundCapture *me = (PyIDirectSoundCapture *)self;

        Py_INCREF(me->m_DS);
        notify->m_DS = me->m_DS;
    }

    return rc;
}

// @pymethod |PyIDirectSoundCapture|Initialize|Not normally called directly. Use DirectSoundCaptureCreate instead.
PyObject *PyIDirectSoundCapture::Initialize(PyObject *self, PyObject *args)
{
    PyObject *obGUID;

    IDirectSoundCapture *pIDSC = GetI(self);
    if (pIDSC == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "|O:Initialize", &obGUID))
        return NULL;

    GUID guid;
    LPGUID pguid = NULL;
    if (!obGUID && obGUID != Py_None) {
        if (!PyWinObject_AsIID(obGUID, &guid))
            return NULL;

        pguid = &guid;
    }

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSC->Initialize(pguid);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Initialize", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundCapture|CreateCaptureBuffer|The IDirectSoundCapture::CreateSoundBuffer method creates a
// DirectSoundBuffer object to hold a sequence of audio samples.
PyObject *PyIDirectSoundCapture::CreateCaptureBuffer(PyObject *self, PyObject *args)
{
    PyObject *obDSCBD = NULL;
    PyObject *obUnk = NULL;
    IUnknown *pUnkIn = NULL;

    IDirectSoundCapture *pIDSC = GetI(self);
    if (pIDSC == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "O|O:CreateCaptureBuffer",
                          &obDSCBD,  // @pyparm <o PyDSCBUFFERDESC>|lpDSCBufferDesc||a DSCBUFFERDESC structure
                                     // containing values for the capture buffer being created.
                          &obUnk))   // @pyparm <o PyIUknown>|unk|None|The IUnknown for COM aggregation.
        return NULL;

    if (!PyDSCBUFFERDESC_Check(obDSCBD)) {
        PyErr_SetString(PyExc_TypeError, "Argument 1 must be of type DSCBUFFERDESC");
        return NULL;
    }

    if (obUnk && !PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE)) {
        return NULL;
    }

    DSCBUFFERDESC *pdscbd = &((PyDSCBUFFERDESC *)obDSCBD)->m_dscbd;
    HRESULT hr;
    IDirectSoundCaptureBuffer *buffer;

    PY_INTERFACE_PRECALL;
    hr = pIDSC->CreateCaptureBuffer(pdscbd, &buffer, pUnkIn);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("CreateCaptureBuffer", hr);
        return NULL;
    }

    PyIDirectSoundCaptureBuffer *rc = new PyIDirectSoundCaptureBuffer(buffer);

    Py_INCREF(self);
    rc->m_DS = self;

    return rc;
}

// @pymethod |PyIDirectSoundCapture|GetCaps|The GetCaps method retrieves the capabilities of the hardware device that is
// represented by the DirectSound object. See <l DSCAPS contants>.
PyObject *PyIDirectSoundCapture::GetCaps(PyObject *self, PyObject *args)
{
    IDirectSoundCapture *pIDSC = GetI(self);
    if (pIDSC == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetCaps"))
        return NULL;

    HRESULT hr;
    PyDSCCAPS *caps = new PyDSCCAPS();
    PY_INTERFACE_PRECALL;
    hr = pIDSC->GetCaps(caps->GetCAPS());
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetCaps", hr);
        return NULL;
    }

    Py_INCREF(caps);
    return caps;
}

// @object PyIDirectSoundCapture|The methods of the IDirectSoundCapture interface are used to create sound capture
// buffers.
static struct PyMethodDef PyIDirectSoundCapture_methods[] = {
    {"Initialize", PyIDirectSoundCapture::Initialize, 1},  // @pymeth Initialize|Description of Initialize.
    {"CreateCaptureBuffer", PyIDirectSoundCapture::CreateCaptureBuffer,
     1},                                             // @pymeth CreateSoundBuffer|Description of CreateSoundBuffer.
    {"GetCaps", PyIDirectSoundCapture::GetCaps, 1},  // @pymeth GetCaps|Description of GetCaps.
    {NULL}};

PyComTypeObject PyIDirectSoundCapture::type("PyIDirectSoundCapture", &PyIUnknown::type, sizeof(PyIDirectSoundCapture),
                                            PyIDirectSoundCapture_methods, GET_PYCOM_CTOR(PyIDirectSoundCapture));
