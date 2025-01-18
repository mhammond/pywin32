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
#include "PyIEnumSTATPROPSETSTG.h"
#include "PyIEnumFORMATETC.h"
#include "PyIDataObject.h"
#include "PyIDropSource.h"
#include "PyIDropTarget.h"
#include "PyIOleWindow.h"
#include "PyIGlobalInterfaceTable.h"
#include "PyIEnumString.h"
#include "PyIServerSecurity.h"
#include "PyIClientSecurity.h"
#include "PyIContext.h"
#include "PyIEnumContextProps.h"
#include "PyICancelMethodCalls.h"

// PyObject *CLSIDMapping;  // Maps CLSIDs onto PyClassObjects
PyObject *g_obPyCom_MapIIDToType = NULL;                // map of IID's to client types.
PyObject *g_obPyCom_MapGatewayIIDToName = NULL;         // map of IID's to names
PyObject *g_obPyCom_MapInterfaceNameToIID = NULL;       // map of names to IID
PyObject *g_obPyCom_MapServerIIDToGateway = NULL;       // map of IID's to gateways.
PyObject *g_obPyCom_MapRecordGUIDToRecordClass = NULL;  // map of COM Record GUIDs to subclasses of com_record.

// Register a Python on both the UID and Name maps.
int PyCom_RegisterClientType(PyTypeObject *typeOb, const GUID *guid)
{
    if (guid == NULL || g_obPyCom_MapIIDToType == NULL)
        return 0;

    PyObject *obiid = PyWinObject_FromIID(*guid);
    if (!obiid)
        return 1;
    int rc = PyDict_SetItem(g_obPyCom_MapIIDToType, obiid, (PyObject *)typeOb);
    Py_DECREF(obiid);
    return rc;
}

// COM Server helpers.
HRESULT PyCom_RegisterGatewayObject(REFIID iid, pfnPyGatewayConstructor ctor, const char *interfaceName)
{
    if (ctor == NULL)
        return E_INVALIDARG;
    if (g_obPyCom_MapServerIIDToGateway == NULL) {
        g_obPyCom_MapServerIIDToGateway = PyDict_New();
    }
    if (g_obPyCom_MapServerIIDToGateway == NULL)
        return E_OUTOFMEMORY;
    PyObject *keyObject = PyWinObject_FromIID(iid);
    if (!keyObject)
        return E_FAIL;

    PyObject *valueObject = PyLong_FromVoidPtr((void *)ctor);

    if (!valueObject) {
        Py_DECREF(keyObject);
        return E_FAIL;
    }
    if (PyDict_SetItem(g_obPyCom_MapServerIIDToGateway, keyObject, valueObject) != 0) {
        Py_DECREF(keyObject);
        return E_FAIL;
    }
    Py_DECREF(valueObject);
    // Now in the other server map.
    if (g_obPyCom_MapGatewayIIDToName) {
        valueObject = PyBytes_FromString((char *)interfaceName);
        if (!valueObject) {
            Py_DECREF(keyObject);
            return E_FAIL;
        }
        if (PyDict_SetItem(g_obPyCom_MapGatewayIIDToName, keyObject, valueObject) != 0) {
            Py_DECREF(valueObject);
            Py_DECREF(keyObject);
            return E_FAIL;
        }
        Py_DECREF(valueObject);
    }
    // And finally in the map of names to gateway IIDs.
    if (g_obPyCom_MapInterfaceNameToIID) {
        valueObject = PyWinCoreString_FromString(interfaceName);
        if (!valueObject) {
            Py_DECREF(keyObject);
            return E_FAIL;
        }
        // Note we reuse the key as the value, and value as the key!
        if (PyDict_SetItem(g_obPyCom_MapInterfaceNameToIID, valueObject, keyObject) != 0) {
            Py_DECREF(valueObject);
            Py_DECREF(keyObject);
            return E_FAIL;
        }
        Py_DECREF(valueObject);
    }
    Py_DECREF(keyObject);

    return S_OK;
}

/* PyType_Ready assures that the type's tp_base is ready, but it does *not* call
    itself for entries in tp_bases, leading to a crash or indecipherable errors
    if one of multiple bases is not itself ready.
    https://github.com/python/cpython/issues/47703
    This code is also in win32uimodule.cpp, should move into pywintypes.
*/
int PyWinType_Ready(PyTypeObject *pT)
{
    if (pT->tp_flags & Py_TPFLAGS_READY)
        return 0;
    if (pT->tp_bases) {
        for (Py_ssize_t b = 0; b < PyTuple_GET_SIZE(pT->tp_bases); b++) {
            PyTypeObject *base_type = (PyTypeObject *)PyTuple_GET_ITEM(pT->tp_bases, b);
            if (PyWinType_Ready(base_type) == -1)
                return -1;
        }
    }
    return PyType_Ready(pT);
}

int PyCom_RegisterSupportedInterfaces(const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
    // Register all interfaces, IID's, etc
    int i;
    for (i = numEntries; i--;) {
        if (pInterfaces[i].pTypeOb)
            if (PyWinType_Ready(pInterfaces[i].pTypeOb) == -1)
                return -1;
        if (pInterfaces[i].pTypeOb && PyCom_RegisterClientType(pInterfaces[i].pTypeOb, pInterfaces[i].pGUID) != 0)
            return -1;
        if (pInterfaces[i].ctor != NULL) {
            HRESULT hr =
                PyCom_RegisterGatewayObject(*pInterfaces[i].pGUID, pInterfaces[i].ctor, pInterfaces[i].interfaceName);
            if (FAILED(hr))
                return -1;
        }
    }
    return 0;
}

int PyCom_RegisterIIDs(PyObject *dict, const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
    int i;
    for (i = numEntries; i--;) {
        PyObject *newIID = PyWinObject_FromIID(*pInterfaces[i].pGUID);
        if (!newIID)
            return -1;
        int rc = PyDict_SetItemString(dict, (char *)pInterfaces[i].iidName, newIID);
        Py_DECREF(newIID);
        if (rc != 0)
            return -1;
    }
    return 0;
}

// Register both client/server gateways, and the IIDs.
int PyCom_RegisterExtensionSupport(PyObject *dict, const PyCom_InterfaceSupportInfo *pInterfaces, int numEntries)
{
    if (PyCom_RegisterSupportedInterfaces(pInterfaces, numEntries) != 0)
        return -1;
    return PyCom_RegisterIIDs(dict, pInterfaces, numEntries);
}

// Determine if a gateway has been registered.
int PyCom_IsGatewayRegistered(REFIID iid)
{
    PyObject *keyObject = PyWinObject_FromIID(iid);
    if (!keyObject) {
        return 0;
    }
    int rc = PyMapping_HasKey(g_obPyCom_MapServerIIDToGateway, keyObject);
    Py_DECREF(keyObject);
    return rc;
}

// @pymethod int|pythoncom|IsGatewayRegistered|Returns true if a gateway has been registered for the given IID
PyObject *pythoncom_IsGatewayRegistered(PyObject *self, PyObject *args)
{
    PyObject *obIID;
    PyObject *v;

    // @pyparm <o PyIID>|iid||IID of the interface.
    if (!PyArg_ParseTuple(args, "O:IsGatewayRegistered", &obIID))
        return NULL;
    v = PyDict_GetItem(g_obPyCom_MapServerIIDToGateway, obIID);
    if (!v) {
        PyErr_Clear();
        v = PyLong_FromLong(0);
    }
    else {
        Py_DECREF(v);
        v = PyLong_FromLong(1);
    }
    return v;
}

/////////////////////////////////////////////////////////////////////////////////
//
// Registration of the core PythonCOM module
//
static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] = {
    PYCOM_INTERFACE_CLSID_ONLY(StdComponentCategoriesMgr),
    PYCOM_INTERFACE_CLSID_ONLY(StdGlobalInterfaceTable),
    // Sort alphabetically just for us poor humans!
    PYCOM_INTERFACE_CLIENT_ONLY(BindCtx),
    PYCOM_INTERFACE_CLIENT_ONLY(CatInformation),
    PYCOM_INTERFACE_CLIENT_ONLY(CatRegister),
    PYCOM_INTERFACE_CLIENT_ONLY(ClassFactory),
    PYCOM_INTERFACE_FULL(ConnectionPoint),
    PYCOM_INTERFACE_FULL(ConnectionPointContainer),
    PYCOM_INTERFACE_CLIENT_ONLY(CreateTypeInfo),
    PYCOM_INTERFACE_CLIENT_ONLY(CreateTypeLib),
    PYCOM_INTERFACE_CLIENT_ONLY(CreateTypeLib2),
    PYCOM_INTERFACE_FULL(DataObject),
    PYCOM_INTERFACE_FULL(DropSource),
    PYCOM_INTERFACE_FULL(DropTarget),
    PYCOM_INTERFACE_CLIENT_ONLY(EnumCATEGORYINFO),
    PYCOM_INTERFACE_FULL(EnumConnectionPoints),
    PYCOM_INTERFACE_FULL(EnumConnections),
    PYCOM_INTERFACE_FULL(EnumFORMATETC),
    PYCOM_INTERFACE_FULL(EnumGUID),
    PYCOM_INTERFACE_CLIENT_ONLY(EnumMoniker),
#ifndef NO_PYCOM_ENUMSTATPROPSTG
    PYCOM_INTERFACE_FULL(EnumSTATPROPSTG),
    PYCOM_INTERFACE_FULL(EnumSTATPROPSETSTG),
#endif  // NO_PYCOM_ENUMSTATPROPSTG
    PYCOM_INTERFACE_FULL(EnumSTATSTG),
    PYCOM_INTERFACE_FULL(EnumString),
    PYCOM_INTERFACE_FULL(EnumVARIANT),
    PYCOM_INTERFACE_FULL(ErrorLog),
    PYCOM_INTERFACE_FULL(ExternalConnection),
    PYCOM_INTERFACE_CLIENT_ONLY(GlobalInterfaceTable),
    PYCOM_INTERFACE_FULL(LockBytes),
    PYCOM_INTERFACE_IID_ONLY(Marshal),
    PYCOM_INTERFACE_CLIENT_ONLY(Moniker),
    PYCOM_INTERFACE_FULL(OleWindow),
    PYCOM_INTERFACE_FULL(Persist),
    PYCOM_INTERFACE_FULL(PersistFile),
    PYCOM_INTERFACE_FULL(PersistPropertyBag),
    PYCOM_INTERFACE_FULL(PersistStorage),
    PYCOM_INTERFACE_FULL(PersistStream),
    PYCOM_INTERFACE_FULL(PersistStreamInit),
    PYCOM_INTERFACE_FULL(PropertyBag),
    PYCOM_INTERFACE_FULL(PropertySetStorage),
    PYCOM_INTERFACE_FULL(PropertyStorage),

    PYCOM_INTERFACE_CLIENT_ONLY(ProvideClassInfo),
    PYCOM_INTERFACE_CLIENT_ONLY(ProvideClassInfo2),

    PYCOM_INTERFACE_CLIENT_ONLY(RunningObjectTable),
    PYCOM_INTERFACE_CLIENT_ONLY(TypeComp),
    PYCOM_INTERFACE_CLIENT_ONLY(TypeInfo),
    PYCOM_INTERFACE_CLIENT_ONLY(TypeLib),
    PYCOM_INTERFACE_FULL(ServiceProvider),
    PYCOM_INTERFACE_IID_ONLY(StdMarshalInfo),
    PYCOM_INTERFACE_FULL(Storage),
    PYCOM_INTERFACE_FULL(Stream),
    PYCOM_INTERFACE_FULL(ServerSecurity),
    PYCOM_INTERFACE_FULL(ClientSecurity),
    PYCOM_INTERFACE_CLIENT_ONLY(Context),
    PYCOM_INTERFACE_CLIENT_ONLY(EnumContextProps),
    PYCOM_INTERFACE_FULL(CancelMethodCalls),
    // No wrapper for IAccessControl yet, but you can still get the system implementation
    //  by calling pythoncom.CoCreateInstance with IID_IUnknown as the returned interface
    PYCOM_INTERFACE_CLSID_ONLY(DCOMAccessControl),

    // NULL, Unknown and dispatch special cases.
    {&IID_NULL, "Null", "IID_NULL", NULL, NULL},
    {&IID_IUnknown, "IUnknown", "IID_IUnknown", &PyIUnknown::type, GET_PYGATEWAY_CTOR(PyGatewayBase)},
    {&IID_IDispatch, "IDispatch", "IID_IDispatch", &PyIDispatch::type, GET_PYGATEWAY_CTOR(PyGatewayBase)},
    {&IID_IDispatchEx, "IDispatchEx", "IID_IDispatchEx", &PyIDispatchEx::type, GET_PYGATEWAY_CTOR(PyGatewayBase)},
    {&IID_StdOle, "IID_StdOle", "IID_StdOle", NULL, NULL},
};

int PyCom_RegisterCoreSupport(void)
{
    if (g_obPyCom_MapIIDToType)
        return 0;  // already done!
    // Create the name and type mappings.
    g_obPyCom_MapIIDToType = PyDict_New();  // map of IID's to types.
    if (g_obPyCom_MapIIDToType == NULL)
        return -1;
    g_obPyCom_MapGatewayIIDToName = PyDict_New();
    if (g_obPyCom_MapGatewayIIDToName == NULL)
        return -1;
    g_obPyCom_MapInterfaceNameToIID = PyDict_New();
    if (g_obPyCom_MapInterfaceNameToIID == NULL)
        return -1;

    return PyCom_RegisterSupportedInterfaces(g_interfaceSupportData,
                                             sizeof(g_interfaceSupportData) / sizeof(PyCom_InterfaceSupportInfo));
}

// Add the IIDs we know about to the core module dictionary.
// Currently adds only core IIDs - not IID from extensions.
int PyCom_RegisterCoreIIDs(PyObject *dict)
{
    return PyCom_RegisterIIDs(dict, g_interfaceSupportData,
                              sizeof(g_interfaceSupportData) / sizeof(PyCom_InterfaceSupportInfo));
}

int PyCom_UnregisterCoreSupport(void)
{
    Py_XDECREF(g_obPyCom_MapIIDToType);
    g_obPyCom_MapIIDToType = NULL;
    Py_XDECREF(g_obPyCom_MapGatewayIIDToName);
    g_obPyCom_MapGatewayIIDToName = NULL;
    Py_XDECREF(g_obPyCom_MapInterfaceNameToIID);
    g_obPyCom_MapInterfaceNameToIID = NULL;
    return 0;
}
