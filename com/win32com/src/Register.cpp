/*
** Registration type stuff
*/

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "stdafx.h"
#include "comcat.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"
#include "PyIStream.h"
#include "PyIPersist.h"
#include "PyIStorage.h"
#include "PyIPersistFile.h"
#include "PyIPersistStorage.h"
#include "PyGPersistStorage.h"
#include "PyIPersistStream.h"
#include "PyIPersistStreamInit.h"
#include "PyIMoniker.h"
#include "PyIRunningObjectTable.h"
#include "PyIBindCtx.h"
#include "PyIEnumGUID.h"
#include "PyIEnumVARIANT.h"
#include "PyICatInformation.h"
#include "PyICatRegister.h"
#include "propbag.h"
#include "PyGConnectionPoint.h"
#include "PyGConnectionPointContainer.h"
#include "PyILockBytes.h"
#include "PyIEnumSTATSTG.h"
#include "PyIExternalConnection.h"
#include "PyIServiceProvider.h"
#include "PyIEnumConnectionPoints.h"
#include "PyIEnumConnections.h"
#include "PyICreateTypeLib.h"
#include "PyICreateTypeInfo.h"
#include "PyIPropertyStorage.h"
#include "PyIPropertySetStorage.h"
#include "PyIEnumSTATPROPSTG.h"

//PyObject *CLSIDMapping;  // Maps CLSIDs onto PyClassObjects
PyObject *g_obPyCom_MapIIDToType = NULL; // map of IID's to client types.
PyObject *g_obPyCom_MapGatewayIIDToName = NULL; // map of IID's to names
PyObject *g_obPyCom_MapInterfaceNameToIID = NULL; // map of names to IID
PyObject *g_obPyCom_MapServerIIDToGateway = NULL; // map of IID's to gateways.

// Register a Python on both the UID and Name maps.
int PyCom_RegisterClientType(PyTypeObject *typeOb, const GUID *guid)
{
	if ( guid == NULL )
		return 0;

	PyObject *obiid = PyWinObject_FromIID(*guid);
	if ( !obiid )
		return 1;
	int rc = PyDict_SetItem(g_obPyCom_MapIIDToType, obiid, (PyObject *)typeOb);
	Py_DECREF(obiid);
	return rc;
}

// COM Server helpers.
HRESULT PyCom_RegisterGatewayObject(REFIID iid, pfnPyGatewayConstructor ctor, const char *interfaceName)
{
	if (ctor==NULL) return E_INVALIDARG;
	if (g_obPyCom_MapServerIIDToGateway==NULL) {
		g_obPyCom_MapServerIIDToGateway = PyDict_New();
	}
	if (g_obPyCom_MapServerIIDToGateway==NULL) return E_OUTOFMEMORY;
	PyObject *keyObject = PyWinObject_FromIID(iid);
	if (!keyObject) return E_FAIL;
#ifdef PYWIN_NO_PYTHON_LONG_LONG
	PyObject *valueObject = PyInt_FromLong((long)ctor);
#else
	PyObject *valueObject = PyLong_FromVoidPtr((void *)ctor);
#endif
	if (!valueObject) {
		Py_DECREF(keyObject);
		return E_FAIL;
	}
	if (PyDict_SetItem(g_obPyCom_MapServerIIDToGateway, keyObject, valueObject)!=0) {
		Py_DECREF(keyObject);
		return E_FAIL;
	}
	Py_DECREF(valueObject);
	// Now in the other server map.
	if (g_obPyCom_MapGatewayIIDToName) {
		valueObject = PyString_FromString((char *)interfaceName);
		if (!valueObject) {
			Py_DECREF(keyObject);
			return E_FAIL;
		}
		if (PyDict_SetItem(g_obPyCom_MapGatewayIIDToName, keyObject, valueObject)!=0) {
			Py_DECREF(keyObject);
			return E_FAIL;
		}
		Py_DECREF(valueObject);
	}
	// And finally in the map of names to gateway IIDs.
	if (g_obPyCom_MapInterfaceNameToIID) {
		valueObject = PyString_FromString((char *)interfaceName);
		if (!valueObject) {
			Py_DECREF(keyObject);
			return E_FAIL;
		}
		// Note we reuse the key as the value, and value as the key!
		if (PyDict_SetItem(g_obPyCom_MapInterfaceNameToIID, valueObject, keyObject)!=0) {
			Py_DECREF(keyObject);
			return E_FAIL;
		}
		Py_DECREF(valueObject);
	}
	Py_DECREF(keyObject);

	return S_OK;
}

int PyCom_RegisterSupportedInterfaces( const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
	// Register all interfaces, IID's, etc
	int i;
	for ( i = numEntries; i--; ) {
		if ( pInterfaces[i].pTypeOb && PyCom_RegisterClientType(pInterfaces[i].pTypeOb, pInterfaces[i].pGUID) != 0 )
			return -1;
		if ( pInterfaces[i].ctor != NULL ) {
			HRESULT hr = PyCom_RegisterGatewayObject(*pInterfaces[i].pGUID,
													pInterfaces[i].ctor,
													pInterfaces[i].interfaceName);
			if ( FAILED(hr) )
				return -1;
		}
	}
	return 0;
}

int PyCom_RegisterIIDs( PyObject *dict, const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
	int i;
	for ( i = numEntries; i--; ) {
		PyObject *newIID = PyWinObject_FromIID(*pInterfaces[i].pGUID);
		if (!newIID) return -1;
		int rc = PyDict_SetItemString(dict, (char *)pInterfaces[i].iidName, newIID);
		Py_DECREF(newIID);
		if (rc!=0) return -1;
	}
	return 0;
}

// Register both client/server gateways, and the IIDs.
int PyCom_RegisterExtensionSupport( PyObject *dict, const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
	if (PyCom_RegisterSupportedInterfaces(pInterfaces, numEntries) != 0)
		return -1;
	return PyCom_RegisterIIDs(dict, pInterfaces, numEntries);
}

// Determine if a gateway has been registered.
int PyCom_IsGatewayRegistered(REFIID iid)
{
	PyObject *keyObject = PyWinObject_FromIID(iid);
	if (!keyObject) 
	{
		return 0;
	}
	return PyMapping_HasKey(
		g_obPyCom_MapServerIIDToGateway,
		keyObject);
}

// @pymethod int|pythoncom|IsGatewayRegistered|Returns true if a gateway has been registered for the given IID 
PyObject  *pythoncom_IsGatewayRegistered(PyObject *self, PyObject *args)
{
	PyObject *obIID;
	PyObject *v;

    // @pyparm <o PyIID>|iid||IID of the interface.
	if ( !PyArg_ParseTuple(args, "O:IsGatewayRegistered", &obIID) )
		return NULL;
	v = PyDict_GetItem(
		g_obPyCom_MapServerIIDToGateway,
		obIID);
	if (!v)
	{
		PyErr_Clear();
		v = PyInt_FromLong(0);
	}
	else
	{
		Py_DECREF(v);
		v = PyInt_FromLong(1);
	}
	return v;
}

/////////////////////////////////////////////////////////////////////////////////
//
// Registration of the core PythonCOM module
//
static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] =
{
#ifndef NO_PYCOM_ICATINFORMATION
	PYCOM_INTERFACE_CLSID_ONLY ( StdComponentCategoriesMgr ),
#endif // NO_PYCOM_ICATINFORMATION
	// Sort alphabetically just for us poor humans!
	PYCOM_INTERFACE_CLIENT_ONLY( BindCtx),
#ifndef NO_PYCOM_ICATINFORMATION
	PYCOM_INTERFACE_CLIENT_ONLY( CatInformation),
#endif // NO_PYCOM_ICATINFORMATION
#ifndef NO_PYCOM_ICATREGISTER
	PYCOM_INTERFACE_CLIENT_ONLY( CatRegister),
#endif // NO_PYCOM_ICATREGISTER
	PYCOM_INTERFACE_CLIENT_ONLY( ClassFactory),
	PYCOM_INTERFACE_FULL       ( ConnectionPoint),
	PYCOM_INTERFACE_FULL       ( ConnectionPointContainer),
	PYCOM_INTERFACE_CLIENT_ONLY( CreateTypeInfo),
	PYCOM_INTERFACE_CLIENT_ONLY( CreateTypeLib),
#ifndef NO_PYCOM_IENUMCATEGORYINFO
	PYCOM_INTERFACE_CLIENT_ONLY( EnumCATEGORYINFO),
#endif // NO_PYCOM_IENUMCATEGORYINFO
	PYCOM_INTERFACE_FULL       ( EnumConnectionPoints),
	PYCOM_INTERFACE_FULL       ( EnumConnections),
#ifndef NO_PYCOM_IENUMGUID
	PYCOM_INTERFACE_CLIENT_ONLY( EnumGUID),
#endif // NO_PYCOM_IENUMGUID
	PYCOM_INTERFACE_CLIENT_ONLY( EnumMoniker),
#ifndef NO_PYCOM_ENUMSTATPROPSTG
	PYCOM_INTERFACE_CLIENT_ONLY( EnumSTATPROPSTG),
#endif // NO_PYCOM_ENUMSTATPROPSTG
	PYCOM_INTERFACE_FULL       ( EnumSTATSTG),
	PYCOM_INTERFACE_FULL       ( EnumVARIANT),
	PYCOM_INTERFACE_FULL       ( ErrorLog),
	PYCOM_INTERFACE_FULL       ( ExternalConnection),
	PYCOM_INTERFACE_FULL       ( LockBytes),
	PYCOM_INTERFACE_IID_ONLY   ( Marshal ),
	PYCOM_INTERFACE_CLIENT_ONLY( Moniker),
	PYCOM_INTERFACE_FULL       ( Persist),
	PYCOM_INTERFACE_FULL       ( PersistFile),
	PYCOM_INTERFACE_FULL       ( PersistPropertyBag),
	PYCOM_INTERFACE_FULL       ( PersistStorage),
	PYCOM_INTERFACE_FULL       ( PersistStream),
	PYCOM_INTERFACE_FULL       ( PersistStreamInit),
	PYCOM_INTERFACE_FULL       ( PropertyBag),
#ifndef NO_PYCOM_IPROPERTYSETSTORAGE
	PYCOM_INTERFACE_CLIENT_ONLY( PropertySetStorage),
#endif // NO_PYCOM_IPROPERTYSETSTORAGE
#ifndef NO_PYCOM_IPROPERTYSTORAGE
	PYCOM_INTERFACE_CLIENT_ONLY( PropertyStorage),
#endif // NO_PYCOM_IPROPERTYSTORAGE

#ifndef NO_PYCOM_IPROVIDECLASSINFO
	PYCOM_INTERFACE_CLIENT_ONLY( ProvideClassInfo),
	PYCOM_INTERFACE_CLIENT_ONLY( ProvideClassInfo2),
#endif // NO_PYCOM_IPROVIDECLASSINFO
	
	PYCOM_INTERFACE_CLIENT_ONLY( RunningObjectTable),
	PYCOM_INTERFACE_CLIENT_ONLY( TypeComp),
	PYCOM_INTERFACE_CLIENT_ONLY( TypeInfo),
	PYCOM_INTERFACE_CLIENT_ONLY( TypeLib),
#ifndef NO_PYCOM_ISERVICEPROVIDER
	PYCOM_INTERFACE_FULL       ( ServiceProvider),
#endif // NO_PYCOM_ISERVICEPROVIDER
	PYCOM_INTERFACE_IID_ONLY   ( StdMarshalInfo ),
	PYCOM_INTERFACE_FULL       ( Storage),
	PYCOM_INTERFACE_FULL       ( Stream),

	// NULL, Unknown and dispatch special cases.
	{ &IID_NULL, "Null", "IID_NULL", NULL, NULL},
	{ &IID_IUnknown, "IUnknown", "IID_IUnknown", &PyIUnknown::type, GET_PYGATEWAY_CTOR(PyGatewayBase)},
	{ &IID_IDispatch, "IDispatch", "IID_IDispatch", &PyIDispatch::type, GET_PYGATEWAY_CTOR(PyGatewayBase) },
#ifndef NO_PYCOM_IDISPATCHEX
	{ &IID_IDispatchEx, "IDispatchEx", "IID_IDispatchEx", &PyIDispatchEx::type, GET_PYGATEWAY_CTOR(PyGatewayBase) },
#endif // NO_PYCOM_IDISPATCHEX
	{ &IID_StdOle, "IID_StdOle", "IID_StdOle", NULL, NULL},
};

int PyCom_RegisterCoreSupport(void)
{
	// Create the name and type mappings.
	g_obPyCom_MapIIDToType = PyDict_New(); // map of IID's to types.
	if (g_obPyCom_MapIIDToType==NULL) return -1;
	g_obPyCom_MapGatewayIIDToName = PyDict_New();
	if (g_obPyCom_MapGatewayIIDToName==NULL) return -1;
	g_obPyCom_MapInterfaceNameToIID = PyDict_New();
	if (g_obPyCom_MapInterfaceNameToIID==NULL) return -1;

	return PyCom_RegisterSupportedInterfaces(g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo) );
}

// Add the IIDs we know about to the core module dictionary.
// Currently adds only core IIDs - not IID from extensions.
int PyCom_RegisterCoreIIDs(PyObject *dict)
{
	return PyCom_RegisterIIDs( dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo) );
}

int PyCom_UnregisterCoreSupport(void)
{
	Py_DECREF(g_obPyCom_MapIIDToType);
	g_obPyCom_MapIIDToType = NULL;
	Py_DECREF(g_obPyCom_MapGatewayIIDToName);
	g_obPyCom_MapGatewayIIDToName = NULL;
	Py_DECREF(g_obPyCom_MapInterfaceNameToIID);
	g_obPyCom_MapInterfaceNameToIID = NULL;
	return 0;
}
