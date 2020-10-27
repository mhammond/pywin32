/*
** Storage helpers.
*/

#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
static HMODULE ole32 = NULL;
// @doc

// @pymethod <o PyIID>|pythoncom|ReadClassStg|Reads a CLSID from a storage object.
PyObject *pythoncom_ReadClassStg(PyObject *self, PyObject *args)
{
    PyObject *obStg;
    if (!PyArg_ParseTuple(args, "O:ReadClassStg",
                          &obStg))  // @pyparm <o PyIStorage>|storage||The storage to read the CLSID from.
        return NULL;
    IStorage *pStorage;
    if (!PyCom_InterfaceFromPyObject(obStg, IID_IStorage, (void **)&pStorage, FALSE))
        return NULL;
    CLSID clsidRet;
    PY_INTERFACE_PRECALL;
    HRESULT hr = ReadClassStg(pStorage, &clsidRet);
    pStorage->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromIID(clsidRet);
}

// @pymethod |pythoncom|WriteClassStg|Writes a CLSID to a storage object
PyObject *pythoncom_WriteClassStg(PyObject *self, PyObject *args)
{
    PyObject *obStg;
    PyObject *obCLSID;
    if (!PyArg_ParseTuple(args, "OO:WriteClassStg",
                          &obStg,  // @pyparm <o PyIStorage>|storage||Storage object into which CLSID will be written.
                          &obCLSID))  // @pyparm <o PyIID>|iid||The IID to write
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
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIID>|pythoncom|ReadClassStm|Retrieves the CLSID from a stream
PyObject *pythoncom_ReadClassStm(PyObject *self, PyObject *args)
{
    PyObject *obStm;
    if (!PyArg_ParseTuple(args, "O:ReadClassStm",
                          &obStm))  // @pyparm <o PyIStream>|Stm||An IStream interface
        return NULL;
    IStream *pStm;
    if (!PyCom_InterfaceFromPyObject(obStm, IID_IStream, (void **)&pStm, FALSE))
        return NULL;
    CLSID clsidRet;
    PY_INTERFACE_PRECALL;
    HRESULT hr = ReadClassStm(pStm, &clsidRet);
    pStm->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromIID(clsidRet);
}

// @pymethod |pythoncom|WriteClassStm|Writes a CLSID to a stream.
PyObject *pythoncom_WriteClassStm(PyObject *self, PyObject *args)
{
    PyObject *obStm;
    PyObject *obCLSID;
    if (!PyArg_ParseTuple(args, "OO:WriteClassStm",
                          &obStm,     // @pyparm <o PyIStream>|Stm||An IStream interface
                          &obCLSID))  // @pyparm <o PyIID>|clsid||The IID to write
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        return NULL;

    IStream *pStm;
    if (!PyCom_InterfaceFromPyObject(obStm, IID_IStream, (void **)&pStm, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = WriteClassStm(pStm, clsid);
    pStm->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIStream>|pythoncom|CreateStreamOnHGlobal|Creates an in-memory stream storage object
PyObject *pythoncom_CreateStreamOnHGlobal(PyObject *self, PyObject *args)
{
    PyObject *obhglobal = Py_None;
    HGLOBAL hglobal = NULL;
    BOOL bdelete = TRUE;
    IStream *pIStream = NULL;
    if (!PyArg_ParseTuple(args, "|Ol:CreateStreamOnHGlobal",
                          &obhglobal,  // @pyparm <o PyHANDLE>|hGlobal|None|Global memory handle.  If None, a new global
                                       // memory object is allocated.
                          &bdelete))   // @pyparm bool|DeleteOnRelease|True|Indicates if global memory should be freed
                                       // when IStream object is destroyed.
        return NULL;
    if (!PyWinObject_AsHANDLE(obhglobal, &hglobal))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateStreamOnHGlobal(hglobal, bdelete, &pIStream);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pIStream, IID_IStream, FALSE);
}

// @pymethod <o PyILockBytes>|pythoncom|CreateILockBytesOnHGlobal|Creates an ILockBytes interface based on global memory
PyObject *pythoncom_CreateILockBytesOnHGlobal(PyObject *self, PyObject *args)
{
    PyObject *obhglobal = Py_None;
    HGLOBAL hglobal = NULL;
    BOOL bdelete = TRUE;
    ILockBytes *pILockBytes = NULL;
    if (!PyArg_ParseTuple(args, "|Ol:CreateILockBytesOnHGlobal",
                          &obhglobal,  // @pyparm <o PyHANDLE>|hGlobal|None|Global memory handle.  If None, a new global
                                       // memory object is allocated.
                          &bdelete))   // @pyparm bool|DeleteOnRelease|True|Indicates if global memory should be freed
                                       // when interface is released.
        return NULL;
    if (!PyWinObject_AsHANDLE(obhglobal, &hglobal))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateILockBytesOnHGlobal(hglobal, bdelete, &pILockBytes);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pILockBytes, IID_ILockBytes, FALSE);
}

// @pymethod <o PyIStorage>|pythoncom|StgCreateDocfile|Creates a new compound file storage object using the OLE-provided
// compound file implementation for the <o PyIStorage> interface.
PyObject *pythoncom_StgCreateDocfile(PyObject *self, PyObject *args)
{
    DWORD reserved = 0;
    PyObject *obName;
    DWORD mode;
    IStorage *pResult;

    if (!PyArg_ParseTuple(
            args, "Oi|i:StgCreateDocfile",
            &obName,  // @pyparm string|name||the path of the compound file to create. It is passed uninterpreted to the
                      // file system. This can be a relative name or None.  If None, a temporary stream is created.
            &mode,    // @pyparm int|mode||Specifies the access mode used to open the storage.
            &reserved))  // @pyparm int|reserved|0|A reserved value
        return NULL;
    PyWin_AutoFreeBstr bstrName;
    if (!PyWinObject_AsAutoFreeBstr(obName, &bstrName, TRUE))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = StgCreateDocfile(bstrName, mode, reserved, &pResult);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

// @pymethod <o PyIStorage>|pythoncom|StgCreateDocfileOnILockBytes|Creates a new compound file storage object using the
// OLE-provided compound file implementation for the <o PyIStorage> interface.
PyObject *pythoncom_StgCreateDocfileOnILockBytes(PyObject *self, PyObject *args)
{
    DWORD reserved = 0;
    DWORD mode;
    IStorage *pResult;
    PyObject *obLockBytes;

    if (!PyArg_ParseTuple(args, "Oi|i:StgCreateDocfileOnILockBytes",
                          &obLockBytes,  // @pyparm <o PyILockBytes>|lockBytes||The <o PyILockBytes> interface on the
                                         // underlying byte array object on which to create a compound file.
                          &mode,         // @pyparm int|mode||Specifies the access mode used to open the storage.
                          &reserved))    // @pyparm int|reserved|0|A reserved value
        return NULL;
    ILockBytes *plb;
    if (!PyCom_InterfaceFromPyObject(obLockBytes, IID_ILockBytes, (void **)&plb, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = StgCreateDocfileOnILockBytes(plb, mode, reserved, &pResult);
    plb->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

// @pymethod <o PyIStorage>|pythoncom|StgOpenStorageOnILockBytes|Open an existing storage object that does not reside in
// a disk file, but instead has an underlying <o PyILockBytes> byte array provided by the caller.
PyObject *pythoncom_StgOpenStorageOnILockBytes(PyObject *self, PyObject *args)
{
    PyObject *obLockBytes;
    PyObject *obStgPriority = NULL;
    DWORD mode;
    PyObject *obSnbExclude = NULL;
    DWORD reserved = 0;
    IStorage *pResult;

    if (!PyArg_ParseTuple(
            args, "OOk|Ok:StgOpenStorageOnILockBytes",
            &obLockBytes,  // @pyparm <o PyILockBytes>|lockBytes||The <o PyILockBytes> interface on the underlying byte
                           // array object on which to open an existing storage object.
            &obStgPriority,  // @pyparm <o PyIStorage>|stgPriority||Usually None, or another parent storage.
            &mode,
            &obSnbExclude,  // @pyparm object|snbExclude|None|Not yet supported - must be None
            &reserved))     // @pyparm int|reserved|0|A reserved value
        return NULL;
    ILockBytes *plb;
    if (!PyCom_InterfaceFromPyObject(obLockBytes, IID_ILockBytes, (void **)&plb, FALSE))
        return NULL;
    IStorage *pStgPriority;
    if (!PyCom_InterfaceFromPyObject(obStgPriority, IID_IStorage, (void **)&pStgPriority, TRUE)) {
        plb->Release();
        return NULL;
    }
    PY_INTERFACE_PRECALL;
    HRESULT hr = StgOpenStorageOnILockBytes(plb, pStgPriority, mode, NULL, reserved, &pResult);
    plb->Release();
    if (pStgPriority)
        pStgPriority->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

#ifndef MS_WINCE
// @pymethod int|pythoncom|StgIsStorageFile|Indicates whether a particular disk file contains a storage object.
PyObject *pythoncom_StgIsStorageFile(PyObject *self, PyObject *args)
{
    PyObject *obName;
    if (!PyArg_ParseTuple(args, "O:StgIsStorageFile",
                          &obName))  // @pyparm string|name||The path to the file to check.
        return NULL;
    PyWin_AutoFreeBstr bstrName;
    if (!PyWinObject_AsAutoFreeBstr(obName, &bstrName))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = StgIsStorageFile(bstrName);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    // @rdesc The return value is 1 if a storage file, else 0.  This
    // method will also raise com_error if the StgIsStorageFile function
    // returns a failure HRESULT.
    return PyInt_FromLong(hr == 0);
}
#endif  // MS_WINCE

// @pymethod <o PyIStorage>|pythoncom|StgOpenStorage|Opens an existing root storage object in the file system.
PyObject *pythoncom_StgOpenStorage(PyObject *self, PyObject *args)
{
    PyObject *temp = NULL;
    DWORD reserved = 0;
    PyObject *obName;
    DWORD mode;
    IStorage *pResult;
    PyObject *obOther;

    if (!PyArg_ParseTuple(
            args, "OOi|Oi:StgOpenStorage",
            &obName,     // @pyparm string|name||Name of the stream, or possibly None if storageOther is non None.
            &obOther,    // @pyparm <o PyIStorage>|other||Usually None, or another parent storage.
            &mode,       // @pyparm int|mode||Specifies the access mode used to open the storage.  A combination of the
                         // storagecon.STGM_* constants.
            &temp,       // @pyparm object|snbExclude|None|Not yet supported - must be None
            &reserved))  // @pyparm int|reserved|0|A reserved value
        return NULL;
    PyWin_AutoFreeBstr bstrName;
    if (!PyWinObject_AsAutoFreeBstr(obName, &bstrName, TRUE))
        return NULL;
    IStorage *pOther;
    if (!PyCom_InterfaceFromPyObject(obOther, IID_IStorage, (void **)&pOther, TRUE))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = StgOpenStorage(bstrName, pOther, mode, NULL, reserved, &pResult);
    if (pOther)
        pOther->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pResult, IID_IStorage, FALSE);
}

// @pymethod <o PyIStorage>|pythoncom|StgOpenStorageEx|Advanced version of StgOpenStorage, win2k or better
// @comm Requires Win2k or later
// @comm Accepts keyword args
PyObject *pythoncom_StgOpenStorageEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
#ifndef NO_PYCOM_STGOPENSTORAGEEX
    typedef HRESULT(WINAPI * PFNStgOpenStorageEx)(WCHAR *, DWORD, DWORD, DWORD, STGOPTIONS *, void *, REFIID, void **);
    ;
    static PFNStgOpenStorageEx myStgOpenStorageEx = NULL;
    if (myStgOpenStorageEx == NULL) {  // Haven't tried to fetch it yet.
        myStgOpenStorageEx = (PFNStgOpenStorageEx)-1;
        if (ole32 == NULL)
            ole32 = GetModuleHandle(_T("Ole32.dll"));
        if (ole32 != NULL) {
            FARPROC fp = GetProcAddress(ole32, "StgOpenStorageEx");
            if (fp != NULL)
                myStgOpenStorageEx = (PFNStgOpenStorageEx)fp;
        }
    }
    if (myStgOpenStorageEx == (PFNStgOpenStorageEx)-1)
        return PyErr_Format(PyExc_NotImplementedError, "StgOpenStorageEx not supported by this version of Windows");
    PyObject *obfname, *obstgoptions = Py_None;
    TmpWCHAR fname;
    DWORD mode, attrs, stgfmt;
    VOID *reserved = NULL;
    IID riid;
    STGOPTIONS *pstgoptions = NULL;
    HRESULT hr;
    void *ret;

    static char *keywords[] = {"Name", "Mode", "stgfmt", "Attrs", "riid", "StgOptions", NULL};
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OiiiO&|O:StgOpenStorageEx", keywords,
            &obfname,  //@pyparm string|Name||Name of the stream or file to open
            &mode,     // @pyparm int|Mode||Access mode, combination of storagecon.STGM_* flags
            &stgfmt,   // @pyparm int|stgfmt||Storage format (STGFMT_STORAGE,STGFMT_FILE,STGFMT_ANY, or STGFMT_DOCFILE)
            &attrs,    // @pyparm int|Attrs||File flags and attributes, only used with STGFMT_DOCFILE
            PyWinObject_AsIID,
            &riid,           // @pyparm <o PyIID>|riid||Interface id to return, IStorage or IPropertySetStorage
            &obstgoptions))  //@pyparm dict|StgOptions|None|Dictionary representing STGOPTIONS struct (only used with
                             // STGFMT_DOCFILE)
        return NULL;
    if (!PyWinObject_AsWCHAR(obfname, &fname))
        return NULL;
    if (!PyCom_PyObjectAsSTGOPTIONS(obstgoptions, &pstgoptions))
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = (*myStgOpenStorageEx)(fname, mode, stgfmt, attrs, pstgoptions, reserved, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (pstgoptions)
        delete (pstgoptions);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid, FALSE);
#else
    return PyErr_Format(PyExc_NotImplementedError, "StgOpenStorageEx not supported by this version of Windows");
#endif  // NO_PYCOM_STGOPENSTORAGEEX
}

// @pymethod <o PyIStorage>|pythoncom|StgCreateStorageEx|Creates a new structured storage file or property set
// @comm Requires Win2k or later
// @comm Accepts keyword args
PyObject *pythoncom_StgCreateStorageEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    // MSDN mistakenly shows the security descriptor as PSECURITY_DESCRIPTOR *
    typedef HRESULT(WINAPI * PFNStgCreateStorageEx)(WCHAR *, DWORD, DWORD, DWORD, STGOPTIONS *, PSECURITY_DESCRIPTOR,
                                                    REFIID, void **);
    static PFNStgCreateStorageEx myStgCreateStorageEx = NULL;
    if (myStgCreateStorageEx == NULL) {  // Haven't tried to fetch it yet.
        myStgCreateStorageEx = (PFNStgCreateStorageEx)-1;
        if (ole32 == NULL)
            ole32 = GetModuleHandle(_T("Ole32.dll"));
        if (ole32 != NULL) {
            FARPROC fp = GetProcAddress(ole32, "StgCreateStorageEx");
            if (fp != NULL)
                myStgCreateStorageEx = (PFNStgCreateStorageEx)fp;
        }
    }
    if (myStgCreateStorageEx == (PFNStgCreateStorageEx)-1)
        return PyErr_Format(PyExc_NotImplementedError, "StgCreateStorageEx not supported by this version of Windows");

    PyObject *obfname, *obstgoptions = Py_None, *obsd = Py_None;
    TmpWCHAR fname;
    DWORD mode, attrs, stgfmt;
    IID riid;
    STGOPTIONS *pstgoptions = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    HRESULT hr;
    void *ret;
    static char *keywords[] = {"Name", "Mode", "stgfmt", "Attrs", "riid", "StgOptions", "SecurityDescriptor", NULL};
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OiiiO&|OO:StgCreateStorageEx", keywords,
            &obfname,  //@pyparm string|Name||Name of the stream or file to open
            &mode,     // @pyparm int|Mode||Access mode, combination of storagecon.STGM_* flags
            &stgfmt,   // @pyparm int|stgfmt||Storage format, storagecon.STGFMT_*
            &attrs,    // @pyparm int|Attrs||File flags and attributes, only used with STGFMT_DOCFILE
            PyWinObject_AsIID,
            &riid,          // @pyparm <o PyIID>|riid||Interface id to return, IStorage or IPropertySetStorage
            &obstgoptions,  //@pyparm dict|StgOptions|None|Dictionary representing STGOPTIONS struct (only used with
                            // STGFMT_DOCFILE)
            &obsd))  // @pyparm <o PySECURITY_DESCRIPTOR>|SecurityDescriptor|None|Specifies security for the new file.
                     // Must be None on Windows XP.
        return NULL;

    if (!PyWinObject_AsSECURITY_DESCRIPTOR(obsd, &psd, TRUE))
        return NULL;
    if (!PyWinObject_AsWCHAR(obfname, &fname, TRUE))
        return NULL;
    if (!PyCom_PyObjectAsSTGOPTIONS(obstgoptions, &pstgoptions))
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = (*myStgCreateStorageEx)(fname, mode, stgfmt, attrs, pstgoptions, psd, riid, &ret);
    // hr = StgCreateStorageEx(fname, mode, stgfmt, attrs, pstgoptions, psd, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (pstgoptions)
        delete (pstgoptions);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid, FALSE);
}

// @pymethod <o PyUNICODE>|pythoncom|FmtIdToPropStgName|Converts a FMTID to its stream name
PyObject *pythoncom_FmtIdToPropStgName(PyObject *self, PyObject *args)
{
    // @pyparm <o PyIID>|fmtid||Format id - a property storage GUID (FMTID_* IIDs)
    HRESULT err;
    WCHAR oszName[CCH_MAX_PROPSTG_NAME];
    FMTID fmtid;
    PyObject *obfmtid = NULL;

    typedef HRESULT(WINAPI * PFNFmtIdToPropStgName)(const FMTID *, LPOLESTR);
    static PFNFmtIdToPropStgName pfnFmtIdToPropStgName = NULL;
    static BOOL pfnchecked = FALSE;
    if (!pfnchecked) {
        if (ole32 == NULL)
            ole32 = GetModuleHandle(_T("Ole32.dll"));
        if (ole32 != NULL)
            pfnFmtIdToPropStgName = (PFNFmtIdToPropStgName)GetProcAddress(ole32, "FmtIdToPropStgName");
        pfnchecked = TRUE;
    }
    if (pfnFmtIdToPropStgName == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "FmtIdToPropStgName is not available on this platform");

    if (!PyArg_ParseTuple(args, "O:FmtIdToPropStgName", &obfmtid))
        return NULL;
    if (!PyWinObject_AsIID(obfmtid, &fmtid))
        return NULL;

    PY_INTERFACE_PRECALL;
    err = (*pfnFmtIdToPropStgName)(&fmtid, oszName);
    PY_INTERFACE_POSTCALL;

    if (err != S_OK)
        return PyCom_BuildPyException(err);
    return PyWinObject_FromWCHAR(oszName);
}

// @pymethod <o PyIID>|pythoncom|PropStgNameToFmtId|Converts a property set name to its format id (GUID)
PyObject *pythoncom_PropStgNameToFmtId(PyObject *self, PyObject *args)
{
    // @pyparm string/unicode|Name||Storage stream name
    FMTID fmtid;
    WCHAR *oszName = NULL;
    HRESULT err;
    PyObject *obName = NULL;

    typedef HRESULT(WINAPI * PFNPropStgNameToFmtId)(const LPOLESTR, FMTID *);
    static PFNPropStgNameToFmtId pfnPropStgNameToFmtId = NULL;
    static BOOL pfnchecked = FALSE;
    if (!pfnchecked) {
        if (ole32 == NULL)
            ole32 = GetModuleHandle(_T("Ole32.dll"));
        if (ole32 != NULL)
            pfnPropStgNameToFmtId = (PFNPropStgNameToFmtId)GetProcAddress(ole32, "PropStgNameToFmtId");
        pfnchecked = TRUE;
    }
    if (pfnPropStgNameToFmtId == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "PropStgNameToFmtId is not available on this platform");

    if (!PyArg_ParseTuple(args, "O:PropStgNameToFmtId", &obName))
        return NULL;
    if (!PyWinObject_AsWCHAR(obName, &oszName))
        return NULL;

    PY_INTERFACE_PRECALL;
    err = (*pfnPropStgNameToFmtId)(oszName, &fmtid);
    PY_INTERFACE_POSTCALL;

    PyWinObject_FreeWCHAR(oszName);
    if (err != S_OK)
        return PyCom_BuildPyException(err);
    return PyWinObject_FromIID(fmtid);
}
