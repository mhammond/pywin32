// This file implements the IDirectSound Interface for Python.

#include "directsound_pch.h"
#include "PySoundObjects.h"
#include "PyIDirectSound.h"
#include "PyIDirectSoundBuffer.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDirectSound::PyIDirectSound(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIDirectSound::~PyIDirectSound()
{
}

/* static */ IDirectSound *PyIDirectSound::GetI(PyObject *self)
{
	return (IDirectSound *)PyIUnknown::GetI(self);
}

// @pymethod |PyIDirectSound|Initialize|Description of Initialize.
PyObject *PyIDirectSound::Initialize(PyObject *self, PyObject *args)
{
	PyObject *obGUID;

	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "|O:Initialize", 
		&obGUID) )  // @pyparm <o PyIID>|guid||Globally unique identifier (GUID) specifying the sound driver to which this DirectSound object binds. Pass None to select the primary sound driver. 

		return NULL;

	GUID guid;
	LPGUID pguid = NULL;
	if (!obGUID && obGUID != Py_None)
	{
		if (!PyWinObject_AsIID(obGUID, &guid))
			return NULL;

		pguid = &guid;
	}

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDS->Initialize(pguid);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("Initialize", hr);
		return NULL;
	}
	
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIDirectSound|SetCooperativeLevel|The IDirectSound::SetCooperativeLevel method sets the cooperative level of the application for this sound device.
PyObject *PyIDirectSound::SetCooperativeLevel(PyObject *self, PyObject *args)
{
	int level;
	PyObject *obHWND = NULL;
	HWND hwnd;

	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "Oi:SetCooperativeLevel", 
		&obHWND, // @pyparm int|hwnd||Window handle to the application or None.
		&level) ) // @pyparm int|level||Requested priority level. Specify one of the following values:
		// @flagh Level|Description
		// @flag DSSCL_NORMAL|Sets the application to a fully cooperative status. Most applications should use this level, because it has the smoothest multitasking and resource-sharing behavior.
		// @flag DSSCL_PRIORITY|Sets the application to the priority level. Applications with this cooperative level can call the DirectSoundBuffer.setFormat and DirectSound.compact methods. 
		// @flag DSSCL_EXCLUSIVE|Sets the application to the exclusive level. When it has the input focus, the application will be the only one audible (sounds from applications with the DSBCAPS_GLOBALFOCUS flag set will be muted). With this level, it also has all the privileges of the DSSCL_PRIORITY level. DirectSound will restore the hardware format, as specified by the most recent call to the DirectSoundBuffer.setFormat method, once the application gains the input focus. (Note that DirectSound will always restore the wave format, no matter what priority level is set.) 
		// @flag DSSCL_WRITEPRIMARY|This is the highest priority level. The application has write access to the primary sound buffers. No secondary sound buffers in any application can be played. 

		return NULL;
	if (!PyWinObject_AsHANDLE(obHWND, (HANDLE *)&hwnd))
		return NULL;
	if (hwnd == NULL)
	{
		hwnd = GetForegroundWindow();
		if (hwnd == NULL)
		{
	        hwnd = GetDesktopWindow();
	    }
	}

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDS->SetCooperativeLevel(hwnd, level);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("SetCooperativeLevel", hr);
		return NULL;
	}
	
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIDirectSound|CreateSoundBuffer|The IDirectSound::CreateSoundBuffer method creates a DirectSoundBuffer object to hold a sequence of audio samples.
PyObject *PyIDirectSound::CreateSoundBuffer(PyObject *self, PyObject *args)
{
	PyObject *obDSBD = NULL;
	PyObject *obUnk = NULL;
	IUnknown *pUnkIn = NULL;

	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "O|O:CreateSoundBuffer", 
		&obDSBD,  // @pyparm <o PyDSCBUFFERDESC>|lpDSCBufferDesc||a DSBUFFERDESC structure containing values for the sound buffer being created.
		&obUnk) ) // @pyparm <o PyIUknown>|unk|None|The IUnknown for COM aggregation.
		return NULL;

	if (!PyDSBUFFERDESC_Check(obDSBD)) {
		PyErr_SetString(PyExc_TypeError, "Argument 1 must be of type DSBUFFERDESC");
		return NULL;
	}

	if (obUnk && !PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE)) {
		return NULL;
	}
	

	DSBUFFERDESC *pdsbd = &((PyDSBUFFERDESC*)obDSBD)->m_dsbd;
	HRESULT hr;
	IDirectSoundBuffer *buffer;

	PY_INTERFACE_PRECALL;
	hr = pIDS->CreateSoundBuffer(pdsbd, &buffer, pUnkIn);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("CreateSoundBuffer", hr);
		return NULL;
	}

	PyIDirectSoundBuffer *rc = new PyIDirectSoundBuffer(buffer);

	Py_INCREF(self);
	rc->m_DS = self;

	return rc;
}

// @pymethod |PyIDirectSound|GetCaps|The GetCaps method retrieves the capabilities of the hardware device that is represented by the DirectSound object. See <l DSCAPS contants>.
PyObject *PyIDirectSound::GetCaps(PyObject *self, PyObject *args)
{
	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetCaps") )
		return NULL;


	HRESULT hr;
	PyDSCAPS *caps = new PyDSCAPS();
	PY_INTERFACE_PRECALL;
	hr = pIDS->GetCaps(caps->GetCAPS());
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetCaps", hr);
		return NULL;
	}

	Py_INCREF(caps);
	return caps;
}

// @pymethod |PyIDirectSound|Compact|The Compact method moves the unused portions of on-board sound memory, if any, to a contiguous block so that the largest portion of free memory will be available.
PyObject *PyIDirectSound::Compact(PyObject *self, PyObject *args)
{
	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":Compact") )
		return NULL;


	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDS->Compact();
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetCaps", hr);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIDirectSound|GetSpeakerConfig|The GetSpeakerConfig method retrieves the speaker configuration.
PyObject *PyIDirectSound::GetSpeakerConfig(PyObject *self, PyObject *args)
{
	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":GetSpeakerConfig") )
		return NULL;


	HRESULT hr;
	DWORD config;
	PY_INTERFACE_PRECALL;
	hr = pIDS->GetSpeakerConfig(&config);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("GetSpeakerConfig", hr);
		return NULL;
	}

	return PyInt_FromLong(config);
}

// @pymethod |PyIDirectSound|SetSpeakerConfig|The SetSpeakerConfig method specifies the speaker configuration of the DirectSound object.
PyObject *PyIDirectSound::SetSpeakerConfig(PyObject *self, PyObject *args)
{
	DWORD config;
	IDirectSound *pIDS = GetI(self);
	if ( pIDS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "i:SetSpeakerConfig", 
		&config) ) // @pyparm int|dwSpeakerConfig||Speaker configuration of the specified DirectSound object. See the DSSPEAKER constants.
		return NULL;


	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIDS->SetSpeakerConfig(config);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		PyWin_SetAPIError("SetSpeakerConfig", hr);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @object PyIDirectSound|Description of the interface
static struct PyMethodDef PyIDirectSound_methods[] =
{
	{ "Initialize", PyIDirectSound::Initialize, 1 }, // @pymeth Initialize|Description of Initialize.
	{ "SetCooperativeLevel", PyIDirectSound::SetCooperativeLevel, 1 }, // @pymeth SetCooperativeLevel|Description of SetCooperativeLevel.
	{ "CreateSoundBuffer", PyIDirectSound::CreateSoundBuffer, 1 }, // @pymeth CreateSoundBuffer|Description of CreateSoundBuffer.
	{ "GetCaps", PyIDirectSound::GetCaps, 1 }, // @pymeth GetCaps|Description of GetCaps.
	{ "Compact", PyIDirectSound::Compact, 1 }, // @pymeth Compact|Description of Compact.
	{ NULL }
};

PyComTypeObject PyIDirectSound::type("PyIDirectSound",
		&PyIUnknown::type,
		sizeof(PyIDirectSound),
		PyIDirectSound_methods,
		GET_PYCOM_CTOR(PyIDirectSound));
