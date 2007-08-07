// PyIStream

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIStream.h"

PyIStream::PyIStream(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIStream::~PyIStream()
{
}

/*static*/ IStream *PyIStream::GetI(PyObject *self)
{
	return (IStream *)PyIUnknown::GetI(self);
}


// @pymethod string|PyIStream|Read|Read the specified number of bytes from the string.
PyObject *PyIStream::Read(PyObject *self, PyObject *args)
{
	ULONG numBytes;
	// @pyparm int|numBytes||The number of bytes to read from the stream.  Must not be zero.
	if (!PyArg_ParseTuple(args, "l:Read", &numBytes))
		return NULL;

	if (numBytes==0) {
		PyErr_SetString(PyExc_TypeError, "The numBytes param must be greater than zero");
		return NULL;
	}
	char *buffer = new char[numBytes];

	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	ULONG read;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Read(buffer, numBytes, &read );
	PY_INTERFACE_POSTCALL;
	PyObject *result;
	if (FAILED(hr))
		result = PyCom_BuildPyException(hr, pMy, IID_IStream);
	else
		result = PyString_FromStringAndSize(buffer, read);
	delete buffer;
	// @rdesc The result is a string containing binary data.
	return result;
}

// @pymethod |PyIStream|Write|Write data to a stream
PyObject *PyIStream::Write(PyObject *self, PyObject *args)
{
	void *strValue;
	PyObject *obstrValue;
	DWORD strSize;
	ULONG cbWritten;
	// @pyparm string|data||The binary data to write.
	if (!PyArg_ParseTuple(args, "O:Write", &obstrValue))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obstrValue, &strValue, &strSize, FALSE))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Write(strValue, strSize, &cbWritten);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	return PyLong_FromUnsignedLong(cbWritten);
}

// @pymethod ULARGE_INTEGER|PyIStream|Seek|Changes the seek pointer to a new location.
PyObject *PyIStream::Seek(PyObject *self, PyObject *args)
{
	PyObject *obLI;
	LARGE_INTEGER offset;
	DWORD origin;
	ULARGE_INTEGER newPos;
	// @pyparm int|offset||The new location
	// @pyparm int|origin||Relative to where?
	if (!PyArg_ParseTuple(args, "Ol", &obLI, (long *)&origin))
		return NULL;
	if (!PyWinObject_AsLARGE_INTEGER(obLI, &offset))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Seek(offset, origin, &newPos);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	return PyWinObject_FromULARGE_INTEGER(newPos);
}

// @pymethod |PyIStream|SetSize|Changes the size of the stream object.
PyObject *PyIStream::SetSize(PyObject *self, PyObject *args)
{
	PyObject *obNewSize;
	// @pyparm ULARGE_INTEGER|newSize||The new size
	if (!PyArg_ParseTuple(args, "O", &obNewSize))
		return NULL;
	ULARGE_INTEGER newSize;
	if (!PyWinObject_AsULARGE_INTEGER(obNewSize, &newSize))
		return NULL;

	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->SetSize(newSize);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod ULARGE_INTEGER|PyIStream|CopyTo|Copies a specified number of bytes from the current seek pointer in the stream to the current seek pointer in another stream.
PyObject *PyIStream::CopyTo(PyObject *self, PyObject *args)
{
	PyObject *obStream;
	PyObject *obCB;
	// @pyparm <o PyIStream>|stream||The stream to write to.
	// @pyparm ULARGE_INTEGER|cb||The number of bytes to write.
	if (!PyArg_ParseTuple(args, "OO:CopyTo", &obStream, &obCB))
		return NULL;
	ULARGE_INTEGER cb;
	if (!PyWinObject_AsULARGE_INTEGER(obCB, &cb))
		return NULL;
	IStream *pStream = GetI(obStream);
	if (pStream==NULL) return NULL;

	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	ULARGE_INTEGER written;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->CopyTo(pStream, cb, NULL, &written);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	// @rdesc The return value is the number of bytes actually written.
	return PyWinObject_FromULARGE_INTEGER(written);
}

// @pymethod |PyIStream|Commit|Ensures that any changes made to a stream object open in transacted mode are reflected in the parent storage. 
PyObject *PyIStream::Commit(PyObject *self, PyObject *args)
{
	DWORD flags = STGC_DEFAULT;
	// @pyparm int|flags|STGC_DEFAULT|Controls how changes are performed.
	if (!PyArg_ParseTuple(args, "|l:Commit", &flags))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Commit(flags);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIStream|Revert|Discards all changes that have been made to a transacted stream since the last <om PyIStream::Commit> call.
PyObject *PyIStream::Revert(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Revert"))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Revert();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIStream|LockRegion|Restricts access to a specified range of bytes in the stream.
PyObject *PyIStream::LockRegion(PyObject *self, PyObject *args)
{
	PyObject *obOffset;
	PyObject *obCB;
	DWORD lockType;
	// @pyparm ULARGE_INTEGER|offset||Integer that specifies the byte offset for the beginning of the range.
	// @pyparm ULARGE_INTEGER|cb||The number of bytes to restrict.
	// @pyparm int|lockType||Restrictions requested.
	if (!PyArg_ParseTuple(args, "OOl:LockRegion", &obOffset, &obCB, &lockType))
		return NULL;
	ULARGE_INTEGER offset;
	if (!PyWinObject_AsULARGE_INTEGER(obOffset, &offset))
		return NULL;
	ULARGE_INTEGER cb;
	if (!PyWinObject_AsULARGE_INTEGER(obCB, &cb))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->LockRegion(offset, cb, lockType);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIStream|UnlockRegion|Removes the access restriction on a range of bytes previously restricted with <om PyIStream::LockRegion>.
PyObject *PyIStream::UnlockRegion(PyObject *self, PyObject *args)
{
	PyObject *obOffset;
	PyObject *obCB;
	DWORD lockType;
	// @pyparm ULARGE_INTEGER|offset||Integer that specifies the byte offset for the beginning of the range.
	// @pyparm ULARGE_INTEGER|cb||The number of bytes to restrict.
	// @pyparm int|lockType||Restrictions requested.
	if (!PyArg_ParseTuple(args, "OOl:UnlockRegion", &obOffset, &obCB, &lockType))
		return NULL;
	ULARGE_INTEGER offset;
	if (!PyWinObject_AsULARGE_INTEGER(obOffset, &offset))
		return NULL;
	ULARGE_INTEGER cb;
	if (!PyWinObject_AsULARGE_INTEGER(obCB, &cb))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->UnlockRegion(offset, cb, lockType);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIStream>|PyIStream|Clone|Creates a new stream object with its own seek pointer that references the same bytes as the original stream. 
PyObject *PyIStream::Clone(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Clone"))
		return NULL;
	IStream *pMy = GetI(self);
	if (pMy==NULL) return NULL;
	IStream *pNewStream;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Clone(&pNewStream);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IStream);
	return PyCom_PyObjectFromIUnknown(pNewStream, IID_IStream, FALSE);
}

// @pymethod <o STATSTG>|PyIStream|Stat|Returns information about the stream
PyObject *PyIStream::Stat(PyObject *self, PyObject *args)
{
	IStream *pIS = GetI(self);
	if ( pIS == NULL )
		return NULL;
	// @pyparm int|grfStatFlag|0|Flags.
	DWORD grfStatFlag = 0;
	if ( !PyArg_ParseTuple(args, "|i:Stat", &grfStatFlag) )
		return NULL;
	STATSTG pstatstg;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIS->Stat( &pstatstg, grfStatFlag );
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIS, IID_IStream);

	PyObject *obpstatstg = PyCom_PyObjectFromSTATSTG(&pstatstg);
	// STATSTG doco says our responsibility to free
	if ((pstatstg).pwcsName) CoTaskMemFree((pstatstg).pwcsName);
	PyObject *pyretval = Py_BuildValue("O", obpstatstg);
	Py_XDECREF(obpstatstg);
	return pyretval;
}


// @object PyIStream|A Python interface to IStream
static struct PyMethodDef PyIStream_methods[] =
{
	{"Read",          PyIStream::Read,  1}, // @pymeth Read|Read the specified number of bytes from the string.
	{"read",          PyIStream::Read,  1}, // @pymeth read|Alias for <om PyIStream.Read>
	{"Write",         PyIStream::Write,  1}, // @pymeth Write|Write data from a stream.
	{"write",         PyIStream::Write,  1}, // @pymeth write|Alias for <om PyIStream.Write>
	{"Seek",          PyIStream::Seek,  1}, // @pymeth Seek|Changes the seek pointer to a new location.
	{"SetSize",       PyIStream::SetSize,  1}, // @pymeth SetSize|Changes the size of the stream object.
	{"CopyTo",        PyIStream::CopyTo,  1}, // @pymeth CopyTo|Copies a specified number of bytes from the current seek pointer in the stream to the current seek pointer in another stream.
	{"Commit",        PyIStream::Commit,  1}, // @pymeth Commit|Ensures that any changes made to a stream object open in transacted mode are reflected in the parent storage. 
	{"Revert",        PyIStream::Revert,  1}, // @pymeth Revert|Discards all changes that have been made to a transacted stream since the last <om PyIStream::Commit> call.
	{"LockRegion",    PyIStream::LockRegion,  1}, // @pymeth LockRegion|Restricts access to a specified range of bytes in the stream.
	{"UnlockRegion",  PyIStream::UnlockRegion,  1}, // @pymeth UnLockRegion|Removes the access restriction on a range of bytes previously restricted with <om PyIStream::LockRegion>.
	{"Clone",         PyIStream::Clone,  1}, // @pymeth Clone|Creates a new stream object with its own seek pointer that references the same bytes as the original stream. 
	{"Stat",	      PyIStream::Stat, 1 }, // @pymeth Stat|Returns information about a stream
	{NULL,  NULL}        
};

PyComTypeObject PyIStream::type("PyIStream",
                 &PyIUnknown::type, // @base PyIStream|PyIUnknown
                 sizeof(PyIStream),
                 PyIStream_methods,
				 GET_PYCOM_CTOR(PyIStream));

