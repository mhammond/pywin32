// This file implements the IDirectSoundCaptureBuffer Interface for Python.

#include "directsound_pch.h"
#include "PySoundObjects.h"
#include "PyIDirectSoundCaptureBuffer.h"
#include "PyIDirectSoundNotify.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDirectSoundCaptureBuffer::PyIDirectSoundCaptureBuffer(IUnknown *pdisp):
	PyIUnknown(pdisp), m_DS(NULL)
{
	ob_type = &type;
}

PyIDirectSoundCaptureBuffer::~PyIDirectSoundCaptureBuffer()
{
	// Release should be called before IDirectSound::Release, which may be
	// triggered below
	SafeRelease(this);

	// This may trigger IDirectSound::Release
	if (m_DS)
		Py_DECREF(m_DS);
}

/* static */ IDirectSoundCaptureBuffer *PyIDirectSoundCaptureBuffer::GetI(PyObject *self)
{
	return (IDirectSoundCaptureBuffer*)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIDirectSoundCaptureBuffer::QueryInterface(PyObject *self, PyObject *args)
{
	PyObject *obiid;
	PyObject *obUseIID = NULL;
	if (!PyArg_ParseTuple(args, "O|O:QueryInterface", &obiid, &obUseIID ))
		return NULL;

	PyObject *rc = PyIUnknown::QueryInterface(self, args);

	// Special treatment for PyIDirectSoundNotify

	// This is a workaround for a reference counting bug in IDirectSound:
	// If IDirectSound::Release() is called before IDirectSoundCaptureBuffer::Release() 
	// or IDirectSoundNotify::Release(), we will get an Access Violation

	// We work around this by manipulating the reference count on the Python objects 
	// that encapsulate them
	if (PyIBase::is_object(rc, &PyIDirectSoundNotify::type))
	{
		PyIDirectSoundNotify *notify = (PyIDirectSoundNotify*)rc;
		PyIDirectSoundCaptureBuffer *me = (PyIDirectSoundCaptureBuffer*)self;

		Py_INCREF(me->m_DS);
		notify->m_DS = me->m_DS;
	}

	return rc;
}

// @pymethod |PyIDirectSoundCaptureBuffer|GetCaps|Returns the capabilities of the DirectSound Capture Buffer.
PyObject *PyIDirectSoundCaptureBuffer::GetCaps(PyObject *self, PyObject *args)
{
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetCaps") )
		return NULL;


	HRESULT hr;
	PyDSCBCAPS *caps = new PyDSCBCAPS();
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->GetCaps(caps->GetCAPS());
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetCaps", hr);
		return NULL;
	}

	Py_INCREF(caps);
	return caps;
}

// @pymethod |PyIDirectSoundCaptureBuffer|GetFormat|Retrieves the current format of the sound capture buffer as a WAVEFORMATEX object.
PyObject *PyIDirectSoundCaptureBuffer::GetFormat(PyObject *self, PyObject *args)
{
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetFormat") )
		return NULL;


	HRESULT hr;
	PyWAVEFORMATEX *wfx = new PyWAVEFORMATEX();
	PY_INTERFACE_PRECALL;
	// We don't support getting more than standard wave headers
	hr = pIDSCB->GetFormat(&wfx->m_wfx, sizeof(WAVEFORMATEX), NULL);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetFormat", hr);
		return NULL;
	}
	
	Py_INCREF(wfx);
	return wfx;
}

// @pymethod |PyIDirectSoundCaptureBuffer|GetStatus|Retrieves the current status of the sound capture buffer.
PyObject *PyIDirectSoundCaptureBuffer::GetStatus(PyObject *self, PyObject *args)
{
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetStatus") )
		return NULL;


	HRESULT hr;
	DWORD dwStatus;
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->GetStatus(&dwStatus);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetStatus", hr);
		return NULL;
	}

	return PyInt_FromLong(dwStatus);
}

// @pymethod |PyIDirectSoundCaptureBuffer|Initialize|Not normally used. Used IDirectSoundCapture.CreateCaptureBuffer instead.
PyObject *PyIDirectSoundCaptureBuffer::Initialize(PyObject *self, PyObject *args)
{
	PyObject *obDSCBD = NULL;
	PyObject *obDSC = NULL;
	IDirectSoundCapture *pIDSC = NULL;

	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "OO:Initialize", &obDSC, &obDSCBD) )
		return NULL;

	// Todo - check and initialize pIDS

	if (!PyDSCBUFFERDESC_Check(obDSCBD)) {
		PyErr_SetString(PyExc_TypeError, "Argument 2 must be of type DSCBUFFERDESC");
		return NULL;
	}


	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->Initialize(pIDSC, &((PyDSCBUFFERDESC*)obDSCBD)->m_dscbd);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("Initialize", hr);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIDirectSoundCaptureBuffer|GetCurrentPosition|Returns a tuple of the current capture and read position in the buffer. The capture position is ahead of the read position. These positions are not always identical due to possible buffering of captured data either on the physical device or in the host. The data after the read position up to and including the capture position is not necessarily valid data.
PyObject *PyIDirectSoundCaptureBuffer::GetCurrentPosition(PyObject *self, PyObject *args)
{
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetCurrentPosition") )
		return NULL;

	HRESULT hr;
	DWORD dwCapture = 0, dwRead = 0;
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->GetCurrentPosition(&dwCapture, &dwRead);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetCurrentPosition", hr);
		return NULL;
	}

	PyObject *result = PyTuple_New(2);
	if (!result)
		return NULL;

	PyTuple_SetItem(result, 0, PyInt_FromLong(dwCapture));
	PyTuple_SetItem(result, 1, PyInt_FromLong(dwRead));

	return result;
}

// @pymethod |PyIDirectSoundCaptureBuffer|Start|The PyIDirectSoundCaptureBuffer::Start method puts the capture buffer into the capture state and begins capturing data into the buffer. If the capture buffer is already in the capture state then the method has no effect.
PyObject *PyIDirectSoundCaptureBuffer::Start(PyObject *self, PyObject *args)
{
	// @pyparm int|dwFlags|0|Flags that specify the behavior for the capture buffer when capturing sound data. Possible values for dwFlags can be one of the following: 
	// DSCBSTART_LOOPING 

	DWORD dwFlags;
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "i:Start", &dwFlags) )
		return NULL;

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->Start(dwFlags);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("Start", hr);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIDirectSoundCaptureBuffer|Stop|The IDirectSoundCaptureBuffer::Stop method puts the capture buffer into the "stop" state and stops capturing data. If the capture buffer is already in the stop state then the method has no effect.
PyObject *PyIDirectSoundCaptureBuffer::Stop(PyObject *self, PyObject *args)
{
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":Stop") )
		return NULL;

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDSCB->Stop();
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("Stop", hr);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIDirectSoundCaptureBuffer|Update|Retrieve data from the capture buffer.
PyObject *PyIDirectSoundCaptureBuffer::Update(PyObject *self, PyObject *args)
{
  // @pyparm int|dwReadCursor||Offset, in bytes, from the start of the buffer to where the update begins.

  // @pyparm int|dwReadBytes||Size, in bytes, of the portion of the buffer to update. 

  // @pyparm int|dwFlags|0|Flags modifying the update event. This value can be 0 or the following flag: DSCBLOCK_ENTIREBUFFER  
  // The dwReadBytes parameter is to be ignored and the entire capture buffer is to be locked. 

	DWORD dwReadCursor = 0;
	DWORD dwReadBytes = 0;
	DWORD dwFlags = 0;
	IDirectSoundCaptureBuffer *pIDSCB = GetI(self);
	if ( pIDSCB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "ii|i:Update", &dwReadCursor, &dwReadBytes, &dwFlags) )
		return NULL;

	HRESULT hr;
	LPVOID lpAudioPtr1 = NULL;
	DWORD dwAudioBytes1 = 0;
	LPVOID lpAudioPtr2 = NULL;
	DWORD dwAudioBytes2 = 0;

	PY_INTERFACE_PRECALL;
	hr = pIDSCB->Lock(dwReadCursor, dwReadBytes, &lpAudioPtr1, &dwAudioBytes1,
		&lpAudioPtr2, &dwAudioBytes2, dwFlags);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("Update(Lock)", hr);
		return NULL;
	}

	// The capture buffer is circular, so we may get two pointers and have to 
	// do the wrap-around ourselves.

	PyObject *obData = PyString_FromStringAndSize((char*)lpAudioPtr1, dwAudioBytes1);
	if (!obData)
	{
		PyErr_SetString(PyExc_MemoryError, "Update: could not allocate result string");
		goto error;
	}
	if (lpAudioPtr2)
	{
		PyObject *obData2 = PyString_FromStringAndSize((char*)lpAudioPtr2, dwAudioBytes2);
		
		PyString_Concat(&obData, obData2);
	
		if (!obData)
		{
			PyErr_SetString(PyExc_MemoryError, "Update: could not append to result string");
			goto error;
		}
	}

	{
		// need extra block for local variables from PY_INTERFACE_UPCALL macro
		PY_INTERFACE_PRECALL;
		hr = pIDSCB->Unlock(lpAudioPtr1, dwAudioBytes1, lpAudioPtr2, dwAudioBytes2);
		PY_INTERFACE_POSTCALL;
	}

	if (FAILED(hr)) {
		Py_DECREF(obData);
		PyWin_SetAPIError("Update(Unlock)", hr);
		return NULL;
	}

	return obData;

error:
	{
		// need extra block for local variables from PY_INTERFACE_UPCALL macro
		PY_INTERFACE_PRECALL;
		hr = pIDSCB->Unlock(lpAudioPtr1, dwAudioBytes1, lpAudioPtr2, dwAudioBytes2);
		PY_INTERFACE_POSTCALL;
	}

	return NULL;
}

// @object PyIDirectSoundCaptureBuffer|The methods of the IDirectSoundCaptureBuffer interface are used to manipulate sound capture buffers.
static struct PyMethodDef PyIDirectSoundCaptureBuffer_methods[] =
{
	{ "QueryInterface", PyIDirectSoundCaptureBuffer::QueryInterface, 1 },
	{ "GetCaps", PyIDirectSoundCaptureBuffer::GetCaps, 1 }, // @pymeth Initialize|Description of GetCaps.
	{ "GetFormat", PyIDirectSoundCaptureBuffer::GetFormat, 1 }, // @pymeth SetCooperativeLevel|Description of GetFormat.
	{ "GetStatus", PyIDirectSoundCaptureBuffer::GetStatus, 1 }, // @pymeth GetStatus|Description of GetStatus.
	{ "Initialize", PyIDirectSoundCaptureBuffer::Initialize, 1 }, // @pymeth Initialize|Description of Initialize.
	{ "GetCurrentPosition", PyIDirectSoundCaptureBuffer::GetCurrentPosition, 1 }, // @pymeth GetCurrentPosition|Description of GetCaps.
	{ "Start", PyIDirectSoundCaptureBuffer::Start, 1 }, // @pymeth Play|Description of Start.
	{ "Stop", PyIDirectSoundCaptureBuffer::Stop, 1 }, // @pymeth Stop|Description of Stop.
	{ "Update", PyIDirectSoundCaptureBuffer::Update, 1 }, // @pymeth Unlock|Description of Update.
	{ NULL }
};

PyComTypeObject PyIDirectSoundCaptureBuffer::type("PyIDirectSoundCaptureBuffer",
		&PyIUnknown::type,
		sizeof(PyIDirectSoundCaptureBuffer),
		PyIDirectSoundCaptureBuffer_methods,
		GET_PYCOM_CTOR(PyIDirectSoundCaptureBuffer));
