// This file implements the IDirectSoundNotify Interface for Python.

#include "directsound_pch.h"
#include "PySoundObjects.h"
#include "PyIDirectSoundNotify.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDirectSoundNotify::PyIDirectSoundNotify(IUnknown *pdisp):
	PyIUnknown(pdisp), m_DS(NULL)
{
	ob_type = &type;
}

PyIDirectSoundNotify::~PyIDirectSoundNotify()
{
	// Release should be called before IDirectSound::Release, which may be
	// triggered below
	SafeRelease(this);

	// This may trigger IDirectSound::Release
	if (m_DS)
		Py_DECREF(m_DS);
}

/* static */ IDirectSoundNotify *PyIDirectSoundNotify::GetI(PyObject *self)
{
	return (IDirectSoundNotify*)PyIUnknown::GetI(self);
}

static BOOL unpack(PyObject *tuple, DSBPOSITIONNOTIFY *&notify, int pos)
{
	if (!PyTuple_Check(tuple) || PyTuple_Size(tuple) < 2) 
		return FALSE;

	PyObject *o0 = PyTuple_GET_ITEM(tuple, 0);
	PyObject *o1 = PyTuple_GET_ITEM(tuple, 1);

	if (!o0 || !PyInt_Check(o0) || !o1 || !PyHANDLE_Check(o1))
		return FALSE;

	notify[pos].dwOffset = PyInt_AS_LONG(o0);
	if (!PyWinObject_AsHANDLE(o1, &notify[pos].hEventNotify))
		return FALSE;

	return TRUE;
}

// @pymethod |PyIDirectSoundNotify|SetNotificationPositions|Description of GetCaps.
PyObject *PyIDirectSoundNotify::SetNotificationPositions(PyObject *self, PyObject *args)
{
	int i;
	PyObject *obPos = NULL;
	IDirectSoundNotify *pIDSB = GetI(self);
	if ( pIDSB == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "O:SetNotificationPositions", &obPos) )
		return NULL;

	int size = 0;
	DSBPOSITIONNOTIFY *notify = NULL;

	if (PyTuple_Check(obPos)){
		size = PyTuple_Size(obPos);
		if (size < 1) 
			goto argerror;

		if (PyTuple_Check(PyTuple_GET_ITEM(obPos, 0))) {
			// nested tuples
			notify = new DSBPOSITIONNOTIFY[size];
			for (i = 0; i < size; ++i) {
				if (!unpack(PyTuple_GET_ITEM(obPos, i), notify, i))
					goto argerror;
			}
		}
		else {
			notify = new DSBPOSITIONNOTIFY[1];
			size = 1;
			if (!unpack(obPos, notify, 0))
				goto argerror;
		}
	}
	else if (PyList_Check(obPos)) {
		size = PyList_Size(obPos);
		if (size < 1) 
			goto argerror;

		notify = new DSBPOSITIONNOTIFY[size];
		for (i = 0; i < size; ++i) {
			if (!unpack(PyList_GET_ITEM(obPos, i), notify, i))
				goto argerror;
		}
	}
	else
		goto argerror;

	HRESULT hr;
	{
		PY_INTERFACE_PRECALL;
		hr = pIDSB->SetNotificationPositions(size, notify);
		PY_INTERFACE_POSTCALL;
	}

	delete[] notify;

	if (FAILED(hr)) {
		PyWin_SetAPIError("SetNotificationPositions", hr);
		return NULL;
	}


	Py_INCREF(Py_None);
	return Py_None;

argerror:
	delete[] notify;

	PyErr_SetString(PyExc_TypeError, "Argument must be a tuple (or a list of tuples) with two items: position and win32 Event handle");
	return NULL;
}

// @object PyIDirectSoundNotify|Description of the interface
static struct PyMethodDef PyIDirectSoundNotify_methods[] =
{
	{ "SetNotificationPositions", PyIDirectSoundNotify::SetNotificationPositions, 1 }, // @pymeth Initialize|Description of SetNotificationPositions.
	{ NULL }
};

PyComTypeObject PyIDirectSoundNotify::type("PyIDirectSoundNotify",
		&PyIUnknown::type,
		sizeof(PyIDirectSoundNotify),
		PyIDirectSoundNotify_methods,
		GET_PYCOM_CTOR(PyIDirectSoundNotify));
