/*
** Storage helpers.
*/

#include "stdafx.h"

#include "PythonCOM.h"
#include "PythonCOMServer.h"
// @doc

// @pymethod <o PyIID>|pythoncom|ReadClassStg|Reads a CLSID from a storage object.
PyObject *pythoncom_ReadClassStg(PyObject *self, PyObject *args)
{
	PyObject *obStg;
	if (!PyArg_ParseTuple(args, "O:ReadClassStg",
					   &obStg)) // @pyparm <o PyIStorage>|storage||The storage to read the CLSID from.
		return NULL;
	IStorage *pStorage;
	if (!PyCom_InterfaceFromPyObject(obStg, IID_IStorage, (void **)&pStorage, FALSE))
		return NULL;
	CLSID clsidRet;
	PY_INTERFACE_PRECALL;
	HRESULT hr = ReadClassStg(pStorage, &clsidRet);
	pStorage->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	return PyWinObject_FromIID(clsidRet);
}
// @pymethod |pythoncom|WriteClassStg|Writes a CLSID to a storage.
PyObject *pythoncom_WriteClassStg(PyObject *self, PyObject *args)
{
	PyObject *obStg;
	PyObject *obCLSID;
	if (!PyArg_ParseTuple(args, "OO:WriteClassStg",
					   &obStg, // @pyparm <o PyIStorage>|storage||The storage to read the CLSID from.
					   &obCLSID)) // @pyparm <o PyIID>|iid||The IID to write
		return NULL;

	CLSID clsid;
	if (!PyWinObject_AsIID(obCLSID, &clsid))
		return NULL;

	IStorage *pStorage;
	if (!PyCom_InterfaceFromPyObject(obStg, IID_IStorage, (void **)&pStorage, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = WriteClassStg(pStorage, clsid);
	pStorage->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod <o PyIStorage>|pythoncom|StgCreateDocfile|Creates a new compound file storage object using the OLE-provided compound file implementation for the <o PyIStorage> interface.
PyObject *pythoncom_StgCreateDocfile(PyObject *self, PyObject *args)
{
	DWORD reserved = 0;
	PyObject *obName;
	DWORD mode;
	IStorage *pResult;

	if (!PyArg_ParseTuple(args, "Oi|i:StgCreateDocfile",
		               &obName, // @pyparm string|name||the path of the compound file to create. It is passed uninterpreted to the file system. This can be a relative name or None.  If None, a temporary stream is created.
					   &mode, // @pyparm int|mode||Specifies the access mode used to open the storage.
					   &reserved)) // @pyparm int|reserved|0|A reserved value
		return NULL;
	PyWin_AutoFreeBstr bstrName;
	if ( !PyWinObject_AsAutoFreeBstr(obName, &bstrName, TRUE) )
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = StgCreateDocfile(bstrName, mode, reserved, &pResult);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

// @pymethod <o PyIStorage>|pythoncom|StgCreateDocfileOnILockBytes|Creates a new compound file storage object using the OLE-provided compound file implementation for the <o PyIStorage> interface.
PyObject *pythoncom_StgCreateDocfileOnILockBytes(PyObject *self, PyObject *args)
{
	DWORD reserved = 0;
	DWORD mode;
	IStorage *pResult;
	PyObject *obLockBytes;

	if (!PyArg_ParseTuple(args, "Oi|i:StgCreateDocfileOnILockBytes",
		               &obLockBytes, // @pyparm <o PyILockBytes>|lockBytes||The <o PyILockBytes> interface on the underlying byte array object on which to create a compound file.
					   &mode, // @pyparm int|mode||Specifies the access mode used to open the storage.
					   &reserved)) // @pyparm int|reserved|0|A reserved value
		return NULL;
	ILockBytes *plb;
	if (!PyCom_InterfaceFromPyObject(obLockBytes, IID_ILockBytes, (void **)&plb, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = StgCreateDocfileOnILockBytes(plb, mode, reserved, &pResult);
	plb->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

#ifndef MS_WINCE
// @pymethod int|pythoncom|StgIsStorageFile|Indicates whether a particular disk file contains a storage object.
PyObject *pythoncom_StgIsStorageFile(PyObject *self, PyObject *args)
{
	PyObject *obName;
	if (!PyArg_ParseTuple(args, "O:StgIsStorageFile",
		               &obName)) // @pyparm string|name||The path to the file to check.
		return NULL;
	PyWin_AutoFreeBstr bstrName;
	if ( !PyWinObject_AsAutoFreeBstr(obName, &bstrName) )
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = StgIsStorageFile(bstrName);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	// @rdesc The return value is 1 if a storage file, else 0
	return PyInt_FromLong(hr==0);
}
#endif // MS_WINCE

// @pymethod <o PyIStorage>|pythoncom|StgOpenStorage|Opens an existing root storage object in the file system.
PyObject *pythoncom_StgOpenStorage(PyObject *self, PyObject *args)
{
	PyObject *temp = NULL;
	DWORD reserved = 0;
	PyObject *obName;
	DWORD mode;
	IStorage *pResult;
	PyObject *obOther;

	if (!PyArg_ParseTuple(args, "OOi|Oi:StgOpenStorage",
		               &obName, // @pyparm string|name||Name of the stream, or possibly None if storageOther is non None.
					   &obOther, // @pyparm <o PyIStorage>|other||Usually None, or another parent storage.
					   &mode, // @pyparm int|mode||Specifies the access mode used to open the storage.  A combination of the storagecon.STGM_* constants.
					   &temp, // @pyparm object|snbExclude|None|Not yet supported - must be None
					   &reserved)) // @pyparm int|reserved|0|A reserved value
		return NULL;
	PyWin_AutoFreeBstr bstrName;
	if ( !PyWinObject_AsAutoFreeBstr(obName, &bstrName, TRUE) )
		return NULL;
	IStorage *pOther;
	if (!PyCom_InterfaceFromPyObject(obOther, IID_IStorage, (void **)&pOther, TRUE))
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = StgOpenStorage(bstrName, pOther, mode, NULL, reserved, &pResult);
	if (pOther) pOther->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

