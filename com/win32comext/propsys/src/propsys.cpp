// propsys.cpp :
// $Id$

// Implements wrappers for the Property System functions and interfaces.
// These interfaces are present on Windows Vista and later, but can also
// be installed on XP with Desktop Search 3.
// However, this module doeen't dynamically load any libraries or functions,
// so it will fail to import if the components are not installed.

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

#include "delayimp.h"
#include "propvarutil.h"
#include "Shobjidl.h"

// @object PyPROPERTYKEY|A tuple of a fmtid and property id (IID, int) that uniquely identifies a property
BOOL PyWinObject_AsPROPERTYKEY(PyObject *obkey, PROPERTYKEY *pkey)
{
	return PyArg_ParseTuple(obkey, "O&k:PROPERTYKEY",
		PyWinObject_AsIID, &pkey->fmtid, &pkey->pid);
}

PyObject *PyWinObject_FromPROPERTYKEY(REFPROPERTYKEY key)
{
	return Py_BuildValue("Nk", PyWinObject_FromIID(key.fmtid), key.pid);
}


// @pymethod <o PyIPropertyDescription>|propsys|PSGetPropertyDescription|Gets a description interface for a property
// @comm Possible interfaces include IPropertyDescription, IPropertyDescriptionAliasInfo, and IPropertyDescriptionSearchInfo
PyObject *PyPSGetPropertyDescription(PyObject *self, PyObject *args)
{
	PROPERTYKEY key;
	void *ret;
	IID riid = IID_IPropertyDescription;
	// @pyparm <o PyPROPERTYKEY>|Key||A property key identifier
	// @pyparm <o PyIID>|riid|IID_IPropertyDescription|The interface to return
	if (!PyArg_ParseTuple(args, "O&|O&:PSGetPropertyDescription",
		PyWinObject_AsPROPERTYKEY, &key,
		PyWinObject_AsIID, &riid))
		return NULL;

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = PSGetPropertyDescription(key, riid, &ret);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid);
}

// @pymethod string|propsys|PSGetNameFromPropertyKey|Retrieves the canonical name of a property
PyObject *PyPSGetNameFromPropertyKey(PyObject *self, PyObject *args)
{
	PROPERTYKEY key;
	WCHAR *name=NULL;
	// @pyparm <o PyPROPERTYKEY>|Key||A property key
	if (!PyArg_ParseTuple(args, "O&:PSGetNameFromPropertyKey", PyWinObject_AsPROPERTYKEY, &key))
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = PSGetNameFromPropertyKey(key, &name);
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
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

	if ( FAILED(hr) )
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
	if ( FAILED(hr) )
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
// @pymethod <o PyIPropertyStore>|propsys|SHGetPropertyStoreFromParsingName|Retrieves the property store for an item by path
static PyObject *PySHGetPropertyStoreFromParsingName(PyObject *self, PyObject *args)
{
	// @comm This function does not exist on XP, even with Desktop Search installed
	PyObject *obpath, *obbindctx=Py_None;
	TmpWCHAR path;
	GETPROPERTYSTOREFLAGS flags=GPS_DEFAULT;
	IID riid=IID_IPropertyStore;
	IBindCtx *bindctx=NULL;
	void *ret=NULL;
	// @pyparm string|Path||Path to file
	// @pyparm <o PyIBindCtx>|BindCtx|None|Bind context, or None
	// @pyparm int|Flags|GPS_DEFAULT|Combination of GETPROPERTYSTOREFLAGS values (shellcon.GPS_*)
	// @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to return
	if (!PyArg_ParseTuple(args, "O|OkO&:SHGetPropertyStoreFromParsingName",
		&obpath, &obbindctx, &flags,
		PyWinObject_AsIID, &riid))
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return FALSE;
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
// @pymethod <o PyIPropertyStore>|propsys|SHGetPropertyStoreFromIDList|Retrieves the property store from an absolute ID list
static PyObject *PySHGetPropertyStoreFromIDList(PyObject *self, PyObject *args)
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
	IShellItem *item=NULL;
	IPropertyStore *propertystore;
	PyObject *obitem;
	BOOL writeable=FALSE;
	IID riid = IID_IPropertyStore;
	
	// @pyparm <o PyIShellItem>|Item||A shell item
	// @pyparm bool|ReadWrite|False|Pass True for a writeable property store
	// @pyparm <o PyIID>|riid|IID_IPropertyStore|Interface to return
	if(!PyArg_ParseTuple(args, "O|iO&:PSGetItemPropertyHandler",
		&obitem,
		&writeable,
		PyWinObject_AsIID, &riid))
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
	int tmp = sizeof(PROPDESC_DISPLAYTYPE);
	PROPVARIANT *pv;
	SERIALIZEDPROPERTYVALUE *pspv=NULL;
	ULONG bufsize;
	HRESULT hr;
	// @pyparm <o PyPROPVARIANT>|propvar||The value to serialize
	if (!PyArg_ParseTuple(args, "O&:StgSerializePropVariant", PyWinObject_AsPROPVARIANT, &pv))
		return NULL;
	hr = StgSerializePropVariant(pv, &pspv, &bufsize);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	PyObject *ret = PyString_FromStringAndSize((char *)pspv, bufsize);
	CoTaskMemFree(pspv);
	return ret;
};

// @pymethod <o PyPROPVARIANT>|propsys|StgDeserializePropVariant|Creates a <o PyPROPVARIANT> from a serialized buffer
static PyObject *PyStgDeserializePropVariant(PyObject *self, PyObject *args)
{
	PROPVARIANT pv;
	SERIALIZEDPROPERTYVALUE *pspv;
	ULONG bufsize;
	PyObject *ob;
	HRESULT hr;
	if (!PyArg_ParseTuple(args, "O:StgDeserializePropVariant", &ob))
		return NULL;
	// @pyparm bytes|prop||Buffer or bytes object (or str in Python 2) containing a serialized value
	if (!PyWinObject_AsReadBuffer(ob, (void **)&pspv, &bufsize))
		return NULL;
	hr = StgDeserializePropVariant(pspv, bufsize, &pv);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyWinObject_FromPROPVARIANT(&pv);
};

// @pymethod <o PyIPropertyStore>|propsys|PSCreateMemoryPropertyStore|Creates a temporary property store that is not connected to any backing storage
// @comm May also be used to create <o PyINamedPropertyStore>, <o PyIPropertyStoreCache>, <o PyIPersistStream>, or <o PyIPropertyBag>
static PyObject *PyPSCreateMemoryPropertyStore(PyObject *self, PyObject *args)
{
	void *ret;
	IID riid = IID_IPropertyStore;
	// @pyparm <o PyIID>|riid|IID_IPropertyStore|The interface to create
	if (!PyArg_ParseTuple(args, "|O&:PSCreateMemoryPropertyStore", PyWinObject_AsIID, &riid))
		return NULL;
	HRESULT hr = PSCreateMemoryPropertyStore(riid, &ret);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown((IUnknown *) ret, riid);
};


/* List of module functions */
// @module propsys|A module, encapsulating the Vista Property System interfaces
static struct PyMethodDef propsys_methods[]=
{
//	{ "SHGetPropertyStoreFromIDList", PySHGetPropertyStoreFromIDList, 1 }, // @pymeth SHGetPropertyStoreFromParsingName|Retrieves the property store from an absolute ID list
	{ "PSGetItemPropertyHandler", PyPSGetItemPropertyHandler, 1 }, // @pymeth PSGetItemPropertyHandler|Retrieves the property store for a shell item
	{ "PSGetPropertyDescription", PyPSGetPropertyDescription, 1 }, // @pymeth PSGetPropertyDescription|Gets a description interface for a property
	{ "PSGetPropertySystem", PyPSGetPropertySystem, 1 }, // @pymeth PSGetPropertySystem|Creates an IPropertySystem interface
	{ "PSGetNameFromPropertyKey", PyPSGetNameFromPropertyKey, 1 }, // @pymeth PSGetNameFromPropertyKey|Retrieves the canonical name for a property key
	{ "PSGetPropertyKeyFromName", PyPSGetPropertyKeyFromName, 1 }, // @pymeth PSGetPropertyKeyFromName|Retrieves the property key by canonical name
	{ "PSRegisterPropertySchema", PyPSRegisterPropertySchema, 1 }, // @pymeth PSRegisterPropertySchema|Registers a group of properties described in a schema file
	{ "PSUnregisterPropertySchema", PyPSUnregisterPropertySchema, 1 }, // @pymeth PSUnregisterPropertySchema|Removes a property schema definition
	{ "SHGetPropertyStoreFromParsingName", PySHGetPropertyStoreFromParsingName, 1 }, // @pymeth SHGetPropertyStoreFromParsingName|Retrieves the property store for an item by path
	{ "StgSerializePropVariant", PyStgSerializePropVariant, 1 }, // @pymeth StgSerializePropVariant|Serializes a <o PyPROPVARIANT>
	{ "StgDeserializePropVariant", PyStgDeserializePropVariant, 1 }, // @pymeth StgDeserializePropVariant|Creates a <o PyPROPVARIANT> from a serialized buffer
	{ "PSCreateMemoryPropertyStore", PyPSCreateMemoryPropertyStore, 1 }, // @pymeth PSCreateMemoryPropertyStore|Creates a temporary property store that is not connected to any backing storage
	{ NULL, NULL },
};


static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] =
{
	PYCOM_INTERFACE_FULL (InitializeWithFile),
	PYCOM_INTERFACE_FULL (InitializeWithStream),
	PYCOM_INTERFACE_FULL (NamedPropertyStore),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyDescription),
	PYCOM_INTERFACE_FULL (PropertyDescriptionList),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyDescriptionSearchInfo),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyDescriptionAliasInfo),
	PYCOM_INTERFACE_FULL (PropertyStore),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyStoreCache),
	PYCOM_INTERFACE_FULL (PropertyStoreCapabilities),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertySystem),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyEnumType),
	PYCOM_INTERFACE_CLIENT_ONLY (PropertyEnumTypeList),
};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(propsys)
{
	PYWIN_MODULE_INIT_PREPARE(propsys, propsys_methods,
	                          "A module, encapsulating the Property System interfaces."
							  "Available on Windows Vista and later, but can also be used"
							  "on XP if Desktop Search 3 is installed.");

	if (PyDict_SetItemString(dict, "error", PyWinExc_COMError) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;

	if (PyType_Ready(&PyPROPVARIANTType) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;
	if (PyDict_SetItemString(dict, "PROPVARIANTType", (PyObject *)&PyPROPVARIANTType) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;

	// Register all of our interfaces, gateways and IIDs.
	if (PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData,
		sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo)) != 0)
		PYWIN_MODULE_INIT_RETURN_ERROR;
	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
