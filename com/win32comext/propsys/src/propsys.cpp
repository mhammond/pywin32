// propsys.cpp :
// $Id$

// Implements wrappers for the Property System functions and interfaces.

// This source file contains autoduck documentation.
// @doc

// Any python API functions that use 's#' format must use Py_ssize_t for length
#define PY_SSIZE_T_CLEAN

#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"

#include "PyIInitializeWithFile.h"
#include "PyIInitializeWithStream.h"
#include "PyIPropertyStoreCache.h"
#include "PyIPropertyStoreCapabilities.h"
#include "PyIPropertySystem.h"
#include "PyIPropertyDescriptionList.h"
#include "PyINamedPropertyStore.h"
#include "PyIPropertyDescription.h"
#include "PyIPropertyDescriptionSearchInfo.h"
#include "PyIPropertyDescriptionAliasInfo.h"
#include "PyIPropertyEnumType.h"
#include "PyIPropertyEnumTypeList.h"
#include "PyIPersistSerializedPropStorage.h"
#include "PyIObjectWithPropertyKey.h"
#include "PyIPropertyChange.h"
#include "PyIPropertyChangeArray.h"

#include "delayimp.h"
#include "propvarutil.h"
#include "Shobjidl.h"

#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);
typedef HRESULT(WINAPI *PFNSHGetPropertyStoreForWindow)(HWND, REFIID, void **);
static PFNSHGetPropertyStoreForWindow pfnSHGetPropertyStoreForWindow = NULL;

// @object PyPROPERTYKEY|A tuple of a fmtid and property id (IID, int) that uniquely identifies a property
BOOL PyWinObject_AsPROPERTYKEY(PyObject *obkey, PROPERTYKEY *pkey)
{
    return PyArg_ParseTuple(obkey, "O&k:PROPERTYKEY", PyWinObject_AsIID, &pkey->fmtid, &pkey->pid);
}

PyObject *PyWinObject_FromPROPERTYKEY(REFPROPERTYKEY key)
{
    return Py_BuildValue("Nk", PyWinObject_FromIID(key.fmtid), key.pid);
}

// @pymethod <o PyIPropertyDescription>|propsys|PSGetPropertyDescription|Gets a description interface for a property
// @comm Possible interfaces include IPropertyDescription, IPropertyDescriptionAliasInfo, and
// IPropertyDescriptionSearchInfo
PyObject *PyPSGetPropertyDescription(PyObject *self, PyObject *args)
{
    PROPERTYKEY key;
    void *ret;
    IID riid = IID_IPropertyDescription;
    // @pyparm <o PyPROPERTYKEY>|Key||A property key identifier
    // @pyparm <o PyIID>|riid|IID_IPropertyDescription|The interface to return
    if (!PyArg_ParseTuple(args, "O&|O&:PSGetPropertyDescription", PyWinObject_AsPROPERTYKEY, &key, PyWinObject_AsIID,
                          &riid))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetPropertyDescription(key, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// @pymethod string|propsys|PSGetNameFromPropertyKey|Retrieves the canonical name of a property
PyObject *PyPSGetNameFromPropertyKey(PyObject *self, PyObject *args)
{
    PROPERTYKEY key;
    WCHAR *name = NULL;
    // @pyparm <o PyPROPERTYKEY>|Key||A property key
    if (!PyArg_ParseTuple(args, "O&:PSGetNameFromPropertyKey", PyWinObject_AsPROPERTYKEY, &key))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetNameFromPropertyKey(key, &name);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    PyObject *ret = PyWinObject_FromWCHAR(name);
    CoTaskMemFree(name);
    return ret;
}

// @pymethod <o PyPROPERTYKEY>|propsys|PSGetPropertyKeyFromName|Retrieves the property key by canonical name
PyObject *PyPSGetPropertyKeyFromName(PyObject *self, PyObject *args)
{
    PROPERTYKEY key;
    TmpWCHAR name;
    PyObject *obname;
    // @pyparm str|Name||The canonical name of a property (eg System.Author)
    if (!PyArg_ParseTuple(args, "O:PSGetPropertyKeyFromName", &obname))
        return NULL;
    if (!PyWinObject_AsWCHAR(obname, &name, FALSE))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetPropertyKeyFromName(name, &key);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromPROPERTYKEY(key);
}

// @pymethod <o PyIPropertySystem>|propsys|PSGetPropertySystem|Creates an IPropertySystem interface
PyObject *PyPSGetPropertySystem(PyObject *self, PyObject *args)
{
    void *ret;
    IID riid = IID_IPropertySystem;
    // @pyparm <o PyIID>|riid|IID_IPropertySystem|The interface to return
    if (!PyArg_ParseTuple(args, "|O&:PSGetPropertySystem", PyWinObject_AsIID, &riid))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetPropertySystem(riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// @pymethod |propsys|PSRegisterPropertySchema|Registers a group of properties described in a schema file
static PyObject *PyPSRegisterPropertySchema(PyObject *self, PyObject *args)
{
    PyObject *obfname;
    // @pyparm unicode|filename||An XML file that defines a property schema (*.propdesc)
    if (!PyArg_ParseTuple(args, "O:PSRegisterPropertySchema", &obfname))
        return NULL;
    WCHAR *sz;
    if (!PyWinObject_AsWCHAR(obfname, &sz, FALSE))
        return FALSE;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSRegisterPropertySchema(sz);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(sz);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |propsys|PSUnregisterPropertySchema|Removes a property schema definition
static PyObject *PyPSUnregisterPropertySchema(PyObject *self, PyObject *args)
{
    PyObject *obfname;
    // @pyparm unicode|filename||A previously registered schema definition file
    if (!PyArg_ParseTuple(args, "O:PSUnregisterPropertySchema", &obfname))
        return NULL;
    WCHAR *sz;
    if (!PyWinObject_AsWCHAR(obfname, &sz, FALSE))
        return FALSE;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSUnregisterPropertySchema(sz);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(sz);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// Does not exist on XP, shell32 is /delayload'ed
// @pymethod <o PyIPropertyStore>|propsys|SHGetPropertyStoreFromParsingName|Retrieves the property store for an item by
// path
static PyObject *PySHGetPropertyStoreFromParsingName(PyObject *self, PyObject *args)
{
    // @comm This function does not exist on XP, even with Desktop Search installed
    PyObject *obpath, *obbindctx = Py_None;
    TmpWCHAR path;
    GETPROPERTYSTOREFLAGS flags = GPS_DEFAULT;
    IID riid = IID_IPropertyStore;
    IBindCtx *bindctx = NULL;
    void *ret = NULL;
    // @pyparm string|Path||Path to file
    // @pyparm <o PyIBindCtx>|BindCtx|None|Bind context, or None
    // @pyparm int|Flags|GPS_DEFAULT|Combination of GETPROPERTYSTOREFLAGS values (shellcon.GPS_*)
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to return
    if (!PyArg_ParseTuple(args, "O|OkO&:SHGetPropertyStoreFromParsingName", &obpath, &obbindctx, &flags,
                          PyWinObject_AsIID, &riid))
        return NULL;
    if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obbindctx, IID_IBindCtx, (void **)&bindctx))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = SHGetPropertyStoreFromParsingName(path, bindctx, flags, riid, &ret);
    if (bindctx)
        bindctx->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// ??? needs PyObject_AsPIDL from shell module, or maybe move this function into shell itself ???
/*
#include "..//..//shell//src//shell_pch.h"
// Does not exist on XP, shell32 is /delayload'ed
// @pymethod <o PyIPropertyStore>|propsys|SHGetPropertyStoreFromIDList|Retrieves the property store from an absolute ID
list static PyObject *PySHGetPropertyStoreFromIDList(PyObject *self, PyObject *args)
{
    // @comm This function does not exist on XP, even with Desktop Search installed
    PyObject *obidl, *obriid=Py_None;
    GETPROPERTYSTOREFLAGS flags=GPS_DEFAULT;
    IID riid=IID_IPropertyStore;
    LPITEMIDLIST pidl;
    void *ret=NULL;
    // @pyparm <o PyIDL>|pidl||An absolute item identifier list
    // @pyparm int|Flags|GPS_DEFAULT|Combination of GETPROPERTYSTOREFLAGS values (shellcon.GPS_*)
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to return
    if (!PyArg_ParseTuple(args, "O|kO&:SHGetPropertyStoreFromIDList",
        &obidl, &flags,
        PyWinObject_AsIID, &riid))
        return NULL;
    if (!PyObject_AsPIDL(obidl, &pidl, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = SHGetPropertyStoreFromIDList(pidl, flags, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}
*/

// @pymethod <o PyIPropertyStore>|propsys|PSGetItemPropertyHandler|Retrieves the property store for a shell item
static PyObject *PyPSGetItemPropertyHandler(PyObject *self, PyObject *args)
{
    IShellItem *item = NULL;
    IPropertyStore *propertystore;
    PyObject *obitem;
    BOOL writeable = FALSE;
    IID riid = IID_IPropertyStore;

    // @pyparm <o PyIShellItem>|Item||A shell item
    // @pyparm bool|ReadWrite|False|Pass True for a writeable property store
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|Interface to return
    if (!PyArg_ParseTuple(args, "O|iO&:PSGetItemPropertyHandler", &obitem, &writeable, PyWinObject_AsIID, &riid))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obitem, IID_IShellItem, (void **)&item, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetItemPropertyHandler(item, writeable, riid, (void **)&propertystore);
    item->Release();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)propertystore, riid, FALSE);
}

// @pymethod bytes|propsys|StgSerializePropVariant|Serializes a <o PyPROPVARIANT>
static PyObject *PyStgSerializePropVariant(PyObject *self, PyObject *args)
{
    PROPVARIANT *pv;
    SERIALIZEDPROPERTYVALUE *pspv = NULL;
    ULONG bufsize;
    HRESULT hr;
    // @pyparm <o PyPROPVARIANT>|propvar||The value to serialize
    if (!PyArg_ParseTuple(args, "O&:StgSerializePropVariant", PyWinObject_AsPROPVARIANT, &pv))
        return NULL;
    PY_INTERFACE_PRECALL;
    hr = StgSerializePropVariant(pv, &pspv, &bufsize);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    PyObject *ret = PyBytes_FromStringAndSize((char *)pspv, bufsize);
    CoTaskMemFree(pspv);
    return ret;
};

// @pymethod <o PyPROPVARIANT>|propsys|StgDeserializePropVariant|Creates a <o PyPROPVARIANT> from a serialized buffer
static PyObject *PyStgDeserializePropVariant(PyObject *self, PyObject *args)
{
    PROPVARIANT pv;
    PyObject *ob;
    HRESULT hr;
    if (!PyArg_ParseTuple(args, "O:StgDeserializePropVariant", &ob))
        return NULL;
    // @pyparm bytes|prop||Buffer or bytes object (or str in Python 2) containing a serialized value
    PyWinBufferView pybuf(ob);
    if (!pybuf.ok())
        return NULL;
    PY_INTERFACE_PRECALL;
    hr = StgDeserializePropVariant((SERIALIZEDPROPERTYVALUE *)pybuf.ptr(), pybuf.len(), &pv);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromPROPVARIANT(&pv);
};

// @pymethod <o PyIPropertyStore>|propsys|PSCreateMemoryPropertyStore|Creates a temporary property store that is not
// connected to any backing storage
// @comm May also be used to create <o PyINamedPropertyStore>, <o PyIPropertyStoreCache>, <o PyIPersistStream>, or <o
// PyIPropertyBag>
static PyObject *PyPSCreateMemoryPropertyStore(PyObject *self, PyObject *args)
{
    void *ret;
    IID riid = IID_IPropertyStore;
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to create
    if (!PyArg_ParseTuple(args, "|O&:PSCreateMemoryPropertyStore", PyWinObject_AsIID, &riid))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSCreateMemoryPropertyStore(riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
};

// @pymethod <o PyIPropertyStore>|propsys|PSCreatePropertyStoreFromPropertySetStorage|Wraps a <o PyIPropertySetStorage>
// interface in a <o PyIPropertyStore> object
// @comm This function does not work for the NTFS property storage implementation based on
//  alternate data streams.
static PyObject *PyPSCreatePropertyStoreFromPropertySetStorage(PyObject *self, PyObject *args)
{
    PyObject *obpss;
    IPropertySetStorage *pipss;
    DWORD mode;
    IID riid = IID_IPropertyStore;
    void *ret;
    // @pyparm <o PyIPropertySetStorage>|pss||Property container to be adapted
    // @pyparm int|Mode||Read or write mode, shellcon.STGM_*.  Must match mode used to open input interface.
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to create
    if (!PyArg_ParseTuple(args, "Ok|O&:PSCreatePropertyStoreFromPropertySetStorage", &obpss, &mode, PyWinObject_AsIID,
                          &riid))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obpss, IID_IPropertySetStorage, (void **)&pipss, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSCreateMemoryPropertyStore(riid, &ret);
    pipss->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
};

// @pymethod <o PyIID>|propsys|PSLookupPropertyHandlerCLSID|Returns the GUID of the property handler for a file
// @comm If no handler is found, the returned error code can be deceptive as it seems to indicate
//   that the file itself was not found
static PyObject *PyPSLookupPropertyHandlerCLSID(PyObject *self, PyObject *args)
{
    PyObject *obfname;
    TmpWCHAR fname;
    // @pyparm str|FilePath||Name of file
    IID iid;
    if (!PyArg_ParseTuple(args, "O:PSLookupPropertyHandlerCLSID", &obfname))
        return NULL;
    if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSLookupPropertyHandlerCLSID(fname, &iid);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromIID(iid);
};

// @pymethod <o PyIPropertyStore>|propsys|SHGetPropertyStoreForWindow|Retrieves a collection of a window's properties
// @rdesc The returned store can be used to set the System.AppUserModel.ID property that determines how windows
//	are grouped on the taskbar
static PyObject *PySHGetPropertyStoreForWindow(PyObject *self, PyObject *args)
{
    CHECK_PFN(SHGetPropertyStoreForWindow);
    HWND hwnd;
    IID riid = IID_IPropertyStore;
    void *ret;
    // @pyparm <o PyHANDLE>|hwnd||Handle to a window
    // @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to create
    if (!PyArg_ParseTuple(args, "O&|O&:SHGetPropertyStoreForWindow", PyWinObject_AsHANDLE, &hwnd, PyWinObject_AsIID,
                          &riid))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = (*pfnSHGetPropertyStoreForWindow)(hwnd, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
};

// @pymethod <o PyPROPVARIANT>|propsys|PSGetPropertyFromPropertyStorage|Extracts a property value from a serialized
// buffer by key
static PyObject *PyPSGetPropertyFromPropertyStorage(PyObject *self, PyObject *args)
{
    PROPERTYKEY key;
    PROPVARIANT val;
    PyObject *obbuf;
    // @pyparm buffer|ps||Bytes or buffer (or str in Python 2) containing a serialized property set (see <om
    // PyIPersistSerializedPropStorage.GetPropertyStorage>)
    // @pyparm <o PyPROPERTYKEY>|key||Property to return
    if (!PyArg_ParseTuple(args, "OO&:PSGetPropertyFromPropertyStorage", &obbuf, PyWinObject_AsPROPERTYKEY, &key))
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetPropertyFromPropertyStorage((PCUSERIALIZEDPROPSTORAGE)pybuf.ptr(), pybuf.len(), key, &val);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromPROPVARIANT(val);
}

// @pymethod <o PyPROPVARIANT>|propsys|PSGetNamedPropertyFromPropertyStorage|Extracts a property value from a serialized
// buffer by name
static PyObject *PyPSGetNamedPropertyFromPropertyStorage(PyObject *self, PyObject *args)
{
    TmpWCHAR name;
    PROPVARIANT val;
    PyObject *obname, *obbuf;
    // @pyparm buffer|ps||Bytes or buffer (or str in Python 2) containing a serialized property set (see <om
    // PyIPersistSerializedPropStorage.GetPropertyStorage>)
    // @pyparm str|name||Property to return
    if (!PyArg_ParseTuple(args, "OO:PSGetNamedPropertyFromPropertyStorage", &obbuf, &obname))
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsWCHAR(obname, &name, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSGetNamedPropertyFromPropertyStorage((PCUSERIALIZEDPROPSTORAGE)pybuf.ptr(), pybuf.len(), name, &val);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromPROPVARIANT(val);
}

// @pymethod <o PyIPropertyChange>|propsys|PSCreateSimplePropertyChange|Creates an IPropertyChange interface used to
// apply changes to a <o PyPROPVARIANT>
static PyObject *PyPSCreateSimplePropertyChange(PyObject *self, PyObject *args)
{
    // @pyparm int|flags||The change operation, pscon.PKA_*
    // @pyparm <o PyPROPERTYKEY>|key||The property key
    // @pyparm <o PyPROPVARIANT>|val||The value that the change operation will apply
    // @pyparm <o PyIID>|riid|IID_IPropertyChange|The interface to return.
    PKA_FLAGS flags;
    PROPERTYKEY key;
    PROPVARIANT *val;
    IID riid = IID_IPropertyChange;
    void *ret;
    if (!PyArg_ParseTuple(args, "lO&O&|O&:PSCreateSimplePropertyChange", &flags, PyWinObject_AsPROPERTYKEY, &key,
                          PyWinObject_AsPROPVARIANT, &val, PyWinObject_AsIID, &riid))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSCreateSimplePropertyChange(flags, key, *val, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// @pymethod <o PyIPropertyChangeArray>|propsys|PSCreatePropertyChangeArray|Creates an IPropertyChangeArray interface to
// be used with <o PyIFileOperation>
// @comm Currently only creates an empty array to be filled in later
static PyObject *PyPSCreatePropertyChangeArray(PyObject *self, PyObject *args)
{
    IID riid = IID_IPropertyChangeArray;
    void *ret;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = PSCreatePropertyChangeArray(NULL, NULL, NULL, 0, riid, &ret);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// @pymethod |propsys|SHSetDefaultProperties|Sets the default properties for a file.
// @comm Default properties are registered by filetype under SetDefaultsFor value.
static PyObject *PySHSetDefaultProperties(PyObject *self, PyObject *args)
{
    HWND hwnd;
    PyObject *obItem, *obSink = Py_None;
    DWORD flags = 0;
    IShellItem *pItem;
    IFileOperationProgressSink *pSink;
    // @pyparm <o PyHANDLE>|hwnd||Parent window for any notifications, can be None
    // @pyparm <o PyIShellItem>|Item||Shell item whose defaults are to be set
    // @pyparm int|FileOpFlags|0|File operation flags, as used with <om PyIFileOperation.SetOperationFlags>
    // @pyparm <o PyGFileOperationProgressSink>|Sink|None|Event sink to receive notifications
    if (!PyArg_ParseTuple(args, "O&O|kO:SHSetDefaultProperties", PyWinObject_AsHANDLE, &hwnd, &obItem, &flags, &obSink))
        return NULL;

    if (!PyCom_InterfaceFromPyInstanceOrObject(obItem, IID_IShellItem, (void **)&pItem, FALSE))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obSink, IID_IFileOperationProgressSink, (void **)&pSink, TRUE)) {
        PYCOM_RELEASE(pItem);
        return NULL;
    }

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = SHSetDefaultProperties(hwnd, pItem, flags, pSink);
    pItem->Release();
    if (pSink)
        pSink->Release();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

/* List of module functions */
// @module propsys|A module, encapsulating the Property System interfaces
static struct PyMethodDef propsys_methods[] = {
    //	{ "SHGetPropertyStoreFromIDList", PySHGetPropertyStoreFromIDList, 1 }, // @pymeth
    // SHGetPropertyStoreFromIDList|Retrieves the property store from an absolute ID list
    {"PSGetItemPropertyHandler", PyPSGetItemPropertyHandler,
     1},  // @pymeth PSGetItemPropertyHandler|Retrieves the property store for a shell item
    {"PSGetPropertyDescription", PyPSGetPropertyDescription,
     1},  // @pymeth PSGetPropertyDescription|Gets a description interface for a property
    {"PSGetPropertySystem", PyPSGetPropertySystem,
     1},  // @pymeth PSGetPropertySystem|Creates an IPropertySystem interface
    {"PSGetNameFromPropertyKey", PyPSGetNameFromPropertyKey,
     1},  // @pymeth PSGetNameFromPropertyKey|Retrieves the canonical name for a property key
    {"PSGetPropertyKeyFromName", PyPSGetPropertyKeyFromName,
     1},  // @pymeth PSGetPropertyKeyFromName|Retrieves the property key by canonical name
    {"PSRegisterPropertySchema", PyPSRegisterPropertySchema,
     1},  // @pymeth PSRegisterPropertySchema|Registers a group of properties described in a schema file
    {"PSUnregisterPropertySchema", PyPSUnregisterPropertySchema,
     1},  // @pymeth PSUnregisterPropertySchema|Removes a property schema definition
    {"SHGetPropertyStoreFromParsingName", PySHGetPropertyStoreFromParsingName,
     1},  // @pymeth SHGetPropertyStoreFromParsingName|Retrieves the property store for an item by path
    {"StgSerializePropVariant", PyStgSerializePropVariant,
     1},  // @pymeth StgSerializePropVariant|Serializes a <o PyPROPVARIANT>
    {"StgDeserializePropVariant", PyStgDeserializePropVariant,
     1},  // @pymeth StgDeserializePropVariant|Creates a <o PyPROPVARIANT> from a serialized buffer
    {"PSCreateMemoryPropertyStore", PyPSCreateMemoryPropertyStore,
     1},  // @pymeth PSCreateMemoryPropertyStore|Creates a temporary property store that is not connected to any backing
          // storage
    {"PSCreatePropertyStoreFromPropertySetStorage", PyPSCreatePropertyStoreFromPropertySetStorage,
     1},  // @pymeth PSCreatePropertyStoreFromPropertySetStorage|Wraps a <o PyIPropertySetStorage> interface in a <o
          // PyIPropertyStore> object
    {"PSLookupPropertyHandlerCLSID", PyPSLookupPropertyHandlerCLSID,
     1},  // @pymeth PSLookupPropertyHandlerCLSID|Returns the GUID of the property handler for a file
    {"SHGetPropertyStoreForWindow", PySHGetPropertyStoreForWindow,
     1},  // @pymeth SHGetPropertyStoreForWindow|Retrieves a collection of a window's properties
    {"PSGetPropertyFromPropertyStorage", PyPSGetPropertyFromPropertyStorage,
     1},  // @pymeth PSGetPropertyFromPropertyStorage|Extracts a property from a serialized buffer by key
    {"PSGetNamedPropertyFromPropertyStorage", PyPSGetNamedPropertyFromPropertyStorage,
     1},  // @pymeth PSGetNamedPropertyFromPropertyStorage|Extracts a property from a serialized buffer by name
    {"PSCreateSimplePropertyChange", PyPSCreateSimplePropertyChange,
     1},  // @pymeth PSCreateSimplePropertyChange|Creates a <o PyIPropertyChange> interface used to apply changes to a
          // <o PyPROPVARIANT>
    {"PSCreatePropertyChangeArray", PyPSCreatePropertyChangeArray,
     METH_NOARGS},  // @pymeth PSCreatePropertyChangeArray|Creates a <o PyIPropertyChangeArray> interface to be used
                    // with <o PyIFileOperation>
    {"SHSetDefaultProperties", PySHSetDefaultProperties,
     METH_VARARGS},  // @pymeth SHSetDefaultProperties|Sets the default properties for a file.
    {NULL, NULL},
};

static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] = {
    PYCOM_INTERFACE_FULL(InitializeWithFile),
    PYCOM_INTERFACE_FULL(InitializeWithStream),
    PYCOM_INTERFACE_FULL(NamedPropertyStore),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyDescription),
    PYCOM_INTERFACE_FULL(PropertyDescriptionList),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyDescriptionSearchInfo),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyDescriptionAliasInfo),
    PYCOM_INTERFACE_FULL(PropertyStore),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyStoreCache),
    PYCOM_INTERFACE_FULL(PropertyStoreCapabilities),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertySystem),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyEnumType),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyEnumTypeList),
    PYCOM_INTERFACE_CLIENT_ONLY(PersistSerializedPropStorage),
    PYCOM_INTERFACE_CLIENT_ONLY(ObjectWithPropertyKey),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyChange),
    PYCOM_INTERFACE_CLIENT_ONLY(PropertyChangeArray),
};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(propsys)
{
    PYWIN_MODULE_INIT_PREPARE(propsys, propsys_methods, "A module, encapsulating the Property System interfaces.");

    if (PyDict_SetItemString(dict, "error", PyWinExc_COMError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PyPROPVARIANTType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "PROPVARIANTType", (PyObject *)&PyPROPVARIANTType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Register all of our interfaces, gateways and IIDs.
    if (PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData,
                                       sizeof(g_interfaceSupportData) / sizeof(PyCom_InterfaceSupportInfo)) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    HMODULE hmod = GetModuleHandle(L"shell32.dll");
    if (hmod)
        pfnSHGetPropertyStoreForWindow =
            (PFNSHGetPropertyStoreForWindow)GetProcAddress(hmod, "SHGetPropertyStoreForWindow");

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
