// PyIPersistStream

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIStream.h"
#include "PyIPersist.h"
#include "PyIPersistStream.h"


/////////////////////////////////////////////////////////////
PyIPersistStream::PyIPersistStream(IUnknown *pDisp) :
	PyIPersist(pDisp)
{
	ob_type = &type;
}

PyIPersistStream::~PyIPersistStream()
{
}

/*static*/ IPersistStream *PyIPersistStream::GetI(PyObject *self)
{
	return (IPersistStream *)PyIPersist::GetI(self);
}


// @pymethod int|PyIPersistStream|IsDirty|Checks the object for changes since it was last saved.
PyObject *PyIPersistStream::IsDirty(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsDirty"))
		return NULL;

	IPersistStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->IsDirty();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pMy, IID_IPersistStream);

	// anything but S_FALSE means dirty.
	return PyInt_FromLong(hr != S_FALSE);
	// @rvalue S_OK (ie, 0)|The object has changed since it was last saved. 
	// @rvalue S_FALSE (ie, 1)|The object has not changed since the last save. 
}

// @pymethod |PyIPersistStream|Load|Initializes an object from the stream where it was previously saved.
PyObject *PyIPersistStream::Load(PyObject *self, PyObject *args)
{
		IPersistStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PyObject *obStream;
	// @pyparm <o PyIStream>|stream||Stream object to load from.
	if (!PyArg_ParseTuple(args, "O:Load", &obStream))
		return NULL;

	IStream *pStream;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obStream, IID_IStream, (void **)&pStream, FALSE /*bNoneOK*/))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Load(pStream);
	pStream->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IPersistStream);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm This method loads an object from its associated stream. The seek pointer is set as it was in the most recent <om PyIPersistStream.Save> method. This method can seek and read from the stream, but cannot write to it.
	// @comm On exit, the seek pointer must be in the same position it was in on entry, immediately past the end of the data.
}

// @pymethod |PyIPersistStream|Save|Saves an object to the specified stream.
PyObject *PyIPersistStream::Save(PyObject *self, PyObject *args)
{
	IPersistStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PyObject *obStream;
	int bClearDirty;
	// @pyparm <o PyIStream>|stream||The stream to save to.
	// @pyparm int|bClearDirty||Indicates whether to clear the dirty flag after the save is complete
	if (!PyArg_ParseTuple(args, "Oi:Save", &obStream, &bClearDirty))
		return NULL;
	IStream *pStream;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obStream, IID_IStream, (void **)&pStream, FALSE /*bNoneOK*/))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Save(pStream, bClearDirty);
	pStream->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IPersistStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod ULARGE_INTEGER|PyIPersistStream|GetSizeMax|Returns the size in bytes of the stream needed to save the object.
PyObject *PyIPersistStream::GetSizeMax(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetSizeMax"))
		return NULL;

	IPersistStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	ULARGE_INTEGER result;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->GetSizeMax(&result);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IPersistStream);
	return PyWinObject_FromULARGE_INTEGER(result);
}

// @object PyIPersistStream|A Python interface to IPersistStream
static struct PyMethodDef PyIPersistStream_methods[] =
{
	{"IsDirty",         PyIPersistStream::IsDirty,  1}, // @pymeth IsDirty|Checks the object for changes since it was last saved.
	{"Load",            PyIPersistStream::Load, 1}, // @pymeth Load|Initializes an object from the stream where it was previously saved.
	{"Save",            PyIPersistStream::Save, 1}, // @pymeth Save|Saves an object to the specified stream.
	{"GetSizeMax",      PyIPersistStream::GetSizeMax, 1}, // @pymeth GetSizeMax|Returns the size in bytes of the stream needed to save the object.
	{NULL,  NULL}        
};

PyComTypeObject PyIPersistStream::type("PyIPersistStream",
                 &PyIPersist::type,	// @base PyIPersistStream|PyIPersist
                 sizeof(PyIPersistStream),
                 PyIPersistStream_methods,
				 GET_PYCOM_CTOR(PyIPersistStream));
