// This file implements the IDirectSound Interface for Python.

#include "directsound_pch.h"
#include "PySoundObjects.h"
#include "PyIDirectSoundBuffer.h"
#include "PyIDirectSoundNotify.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDirectSoundBuffer::PyIDirectSoundBuffer(IUnknown *pdisp) : PyIUnknown(pdisp), m_DS(NULL) { ob_type = &type; }

PyIDirectSoundBuffer::~PyIDirectSoundBuffer()
{
    // Release should be called before IDirectSound::Release, which may be
    // triggered below
    SafeRelease(this);

    // This may trigger IDirectSound::Release
    if (m_DS)
        Py_DECREF(m_DS);
}

/* static */ IDirectSoundBuffer *PyIDirectSoundBuffer::GetI(PyObject *self)
{
    return (IDirectSoundBuffer *)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIDirectSoundBuffer::QueryInterface(PyObject *self, PyObject *args)
{
    PyObject *obiid;
    PyObject *obUseIID = NULL;
    if (!PyArg_ParseTuple(args, "O|O:QueryInterface", &obiid, &obUseIID))
        return NULL;

    PyObject *rc = PyIUnknown::QueryInterface(self, args);

    // Special treatment for PyIDirectSoundNotify

    // This is a workaround for a reference counting bug in IDirectSound:
    // If IDirectSound::Release() is called before IDirectSoundBuffer::Release()
    // or IDirectSoundNotify::Release(), we will get an Access Violation

    // We work around this by manipulating the reference count on the Python objects
    // that encapsulate them
    if (PyIBase::is_object(rc, &PyIDirectSoundNotify::type)) {
        PyIDirectSoundNotify *notify = (PyIDirectSoundNotify *)rc;
        PyIDirectSoundBuffer *me = (PyIDirectSoundBuffer *)self;

        Py_INCREF(me->m_DS);
        notify->m_DS = me->m_DS;
    }

    return rc;
}

// @pymethod |PyIDirectSoundBuffer|GetCaps|Retrieves the capabilities of the DirectSoundBuffer object as a DSBCAPS
// object.
PyObject *PyIDirectSoundBuffer::GetCaps(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetCaps"))
        return NULL;

    HRESULT hr;
    PyDSBCAPS *caps = new PyDSBCAPS();
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetCaps(caps->GetCAPS());
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetCaps", hr);
        return NULL;
    }

    Py_INCREF(caps);
    return caps;
}

// @pymethod |PyIDirectSoundBuffer|GetFormat|Description of GetFormat.
PyObject *PyIDirectSoundBuffer::GetFormat(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetFormat"))
        return NULL;

    HRESULT hr;
    PyWAVEFORMATEX *wfx = new PyWAVEFORMATEX();
    PY_INTERFACE_PRECALL;
    // We don't support getting more than standard wave headers
    hr = pIDSB->GetFormat(&wfx->m_wfx, sizeof(WAVEFORMATEX), NULL);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetFormat", hr);
        return NULL;
    }

    Py_INCREF(wfx);
    return wfx;
}

// @pymethod |PyIDirectSoundBuffer|GetStatus|Retrieves the current status of the sound buffer.
PyObject *PyIDirectSoundBuffer::GetStatus(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetStatus"))
        return NULL;

    HRESULT hr;
    DWORD dwStatus;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetStatus(&dwStatus);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetStatus", hr);
        return NULL;
    }

    return PyInt_FromLong(dwStatus);
}

// @pymethod |PyIDirectSoundBuffer|SetFormat|Sets the format of the primary sound buffer for the application. Whenever
// this application has the input focus, DirectSound will set the primary buffer to the specified format.
PyObject *PyIDirectSoundBuffer::SetFormat(PyObject *self, PyObject *args)
{
    // @pyparm WAVEFORMATEX|format||A WAVEFORMATEX object that describes the new format for the primary sound buffer.

    PyObject *obWfx;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "O:SetFormat", &obWfx))
        return NULL;

    if (!PyWAVEFORMATEX_Check(obWfx)) {
        PyErr_SetString(PyExc_TypeError, "Argument 1 must be of type WAVEFORMATEX");
        return NULL;
    }

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->SetFormat(&((PyWAVEFORMATEX *)obWfx)->m_wfx);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("SetFormat", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|Initialize|Description of Initialize.
PyObject *PyIDirectSoundBuffer::Initialize(PyObject *self, PyObject *args)
{
    PyObject *obDSBD = NULL;
    PyObject *obDS = NULL;
    IDirectSound *pIDS = NULL;

    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "OO:Initialize", &obDS, &obDSBD))
        return NULL;

    // Todo - check and initialize pIDS

    if (!PyDSBUFFERDESC_Check(obDSBD)) {
        PyErr_SetString(PyExc_TypeError, "Argument 2 must be of type DSBUFFERDESC");
        return NULL;
    }

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->Initialize(pIDS, &((PyDSBUFFERDESC *)obDSBD)->m_dsbd);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Initialize", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|Restore|Restores the memory allocation for a lost sound buffer for the specified
// DirectSoundBuffer object.
PyObject *PyIDirectSoundBuffer::Restore(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":Restore"))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->Restore();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Restore", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|GetCurrentPosition|Description of GetCurrentPosition.
PyObject *PyIDirectSoundBuffer::GetCurrentPosition(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetCurrentPosition"))
        return NULL;

    HRESULT hr;
    DWORD dwPlay = 0, dwWrite = 0;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetCurrentPosition(&dwPlay, &dwWrite);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetCurrentPosition", hr);
        return NULL;
    }

    PyObject *result = PyTuple_New(2);
    if (!result)
        return NULL;

    PyTuple_SetItem(result, 0, PyInt_FromLong(dwPlay));
    PyTuple_SetItem(result, 1, PyInt_FromLong(dwWrite));

    return result;
}

// @pymethod |PyIDirectSoundBuffer|Play|Description of Play.
PyObject *PyIDirectSoundBuffer::Play(PyObject *self, PyObject *args)
{
    DWORD dwFlags;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:Play", &dwFlags))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->Play(0, 0, dwFlags);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Play", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|SetCurrentPosition|Description of SetCurrentPosition.
PyObject *PyIDirectSoundBuffer::SetCurrentPosition(PyObject *self, PyObject *args)
{
    DWORD dwNewPosition;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:SetCurrentPosition", &dwNewPosition))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->SetCurrentPosition(dwNewPosition);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("SetCurrentPosition", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|Stop|Description of Stop.
PyObject *PyIDirectSoundBuffer::Stop(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":Stop"))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->Stop();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Stop", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|Update|Description of Update.
PyObject *PyIDirectSoundBuffer::Update(PyObject *self, PyObject *args)
{
    DWORD dwWriteCursor = 0;
    DWORD dwFlags = 0;
    PyObject *obData = NULL;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "iS|i:Update", &dwWriteCursor, &obData, &dwFlags))
        return NULL;

    HRESULT hr;
    LPVOID lpAudioPtr1 = NULL;
    DWORD dwAudioBytes1 = 0;
    LPVOID lpAudioPtr2 = NULL;
    DWORD dwAudioBytes2 = 0;

    PY_INTERFACE_PRECALL;
    hr = pIDSB->Lock(dwWriteCursor, PyString_Size(obData), &lpAudioPtr1, &dwAudioBytes1, &lpAudioPtr2, &dwAudioBytes2,
                     dwFlags);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("Update(Lock)", hr);
        return NULL;
    }

    // The play buffer is circular, so we may get two pointers and have to
    // do the wrap-around ourselves.

    // Raise error if assumption isn't met

    if (dwAudioBytes1 + dwAudioBytes2 != (DWORD)PyString_Size(obData)) {
        PY_INTERFACE_PRECALL;
        hr = pIDSB->Unlock(lpAudioPtr1, dwAudioBytes1, lpAudioPtr2, dwAudioBytes2);
        PY_INTERFACE_POSTCALL;

        PyErr_SetString(PyExc_RuntimeError, "Size mismatch from Unlock");

        return NULL;
    }

    memcpy(lpAudioPtr1, PyString_AsString(obData), dwAudioBytes1);
    if (dwAudioBytes2) {
        memcpy(lpAudioPtr2, PyString_AsString(obData) + dwAudioBytes1, dwAudioBytes2);
    }

    {
        // need extra block for local variables from PY_INTERFACE_UPCALL macro
        PY_INTERFACE_PRECALL;
        hr = pIDSB->Unlock(lpAudioPtr1, dwAudioBytes1, lpAudioPtr2, dwAudioBytes2);
        PY_INTERFACE_POSTCALL;
    }

    if (FAILED(hr)) {
        PyWin_SetAPIError("Update(Unlock)", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|GetFrequency|Description of GetFrequency.
PyObject *PyIDirectSoundBuffer::GetFrequency(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetFrequency"))
        return NULL;

    HRESULT hr;
    DWORD dwFrequency;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetFrequency(&dwFrequency);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetFrequency", hr);
        return NULL;
    }

    return PyInt_FromLong(dwFrequency);
}

// @pymethod |PyIDirectSoundBuffer|GetPan|Description of GetPan.
PyObject *PyIDirectSoundBuffer::GetPan(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetPan"))
        return NULL;

    HRESULT hr;
    LONG pan;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetPan(&pan);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetPan", hr);
        return NULL;
    }

    return PyInt_FromLong(pan);
}

// @pymethod |PyIDirectSoundBuffer|GetVolume|Description of GetVolume.
PyObject *PyIDirectSoundBuffer::GetVolume(PyObject *self, PyObject *args)
{
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetVolume"))
        return NULL;

    HRESULT hr;
    LONG pan;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->GetVolume(&pan);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("GetVolume", hr);
        return NULL;
    }

    return PyInt_FromLong(pan);
}

// @pymethod |PyIDirectSoundBuffer|SetFrequency|Description of SetFrequency.
PyObject *PyIDirectSoundBuffer::SetFrequency(PyObject *self, PyObject *args)
{
    DWORD dwNewFrequency;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:SetFrequency", &dwNewFrequency))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->SetFrequency(dwNewFrequency);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("SetFrequency", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|SetPan|Description of SetPan.
PyObject *PyIDirectSoundBuffer::SetPan(PyObject *self, PyObject *args)
{
    LONG dwNewPan;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:SetPan", &dwNewPan))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->SetPan(dwNewPan);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("SetPan", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIDirectSoundBuffer|SetVolume|Description of SetVolume.
PyObject *PyIDirectSoundBuffer::SetVolume(PyObject *self, PyObject *args)
{
    LONG dwNewVolume;
    IDirectSoundBuffer *pIDSB = GetI(self);
    if (pIDSB == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:SetVolume", &dwNewVolume))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIDSB->SetVolume(dwNewVolume);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr)) {
        PyWin_SetAPIError("SetVolume", hr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIDirectSoundBuffer|Description of the interface
static struct PyMethodDef PyIDirectSoundBuffer_methods[] = {
    {"QueryInterface", PyIDirectSoundBuffer::QueryInterface, 1},
    {"GetCaps", PyIDirectSoundBuffer::GetCaps, 1},  // @pymeth Initialize|Description of Initialize.
    {"GetFormat", PyIDirectSoundBuffer::GetFormat,
     1},  // @pymeth SetCooperativeLevel|Description of SetCooperativeLevel.
    {"GetStatus", PyIDirectSoundBuffer::GetStatus, 1},    // @pymeth GetStatus|Description of GetStatus.
    {"SetFormat", PyIDirectSoundBuffer::SetFormat, 1},    // @pymeth GetCaps|Description of GetCaps.
    {"Initialize", PyIDirectSoundBuffer::Initialize, 1},  // @pymeth Initialize|Description of GetCaps.
    {"Restore", PyIDirectSoundBuffer::Restore, 1},        // @pymeth Restore|Description of Restore.
    {"GetCurrentPosition", PyIDirectSoundBuffer::GetCurrentPosition,
     1},                                      // @pymeth GetCurrentPosition|Description of GetCaps.
    {"Play", PyIDirectSoundBuffer::Play, 1},  // @pymeth Play|Description of GetCaps.
    {"SetCurrentPosition", PyIDirectSoundBuffer::SetCurrentPosition,
     1},                                                      // @pymeth SetCurrentPosition|Description of GetCaps.
    {"Stop", PyIDirectSoundBuffer::Stop, 1},                  // @pymeth Stop|Description of GetCaps.
    {"Update", PyIDirectSoundBuffer::Update, 1},              // @pymeth Unlock|Description of Unlock.
    {"GetFrequency", PyIDirectSoundBuffer::GetFrequency, 1},  // @pymeth GetFrequency|Description of GetCaps.
    {"GetPan", PyIDirectSoundBuffer::GetPan, 1},              // @pymeth GetPan|Description of GetCaps.
    {"GetVolume", PyIDirectSoundBuffer::GetVolume, 1},        // @pymeth GetVolume|Description of GetCaps.
    {"SetFrequency", PyIDirectSoundBuffer::SetFrequency, 1},  // @pymeth SetFrequency|Description of GetCaps.
    {"SetPan", PyIDirectSoundBuffer::SetPan, 1},              // @pymeth SetPan|Description of GetCaps.
    {"SetVolume", PyIDirectSoundBuffer::SetVolume, 1},        // @pymeth SetVolume|Description of GetCaps.
    {NULL}};

PyComTypeObject PyIDirectSoundBuffer::type("PyIDirectSoundBuffer", &PyIUnknown::type, sizeof(PyIDirectSoundBuffer),
                                           PyIDirectSoundBuffer_methods, GET_PYCOM_CTOR(PyIDirectSoundBuffer));
