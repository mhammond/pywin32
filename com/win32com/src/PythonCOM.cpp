// pythoncom.cpp :

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "stdafx.h"
#include <objbase.h>
#include <ComSvcs.h>
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyFactory.h"
#include "PyRecord.h"
#include "PyComTypeObjects.h"
#include "OleAcc.h"    // for ObjectFromLresult proto...
#include "IAccess.h"   // for IAccessControl
#include "pyerrors.h"  // for PyErr_Warn in 2.5 and earlier...

extern int PyCom_RegisterCoreIIDs(PyObject *dict);

extern int PyCom_RegisterCoreSupport(void);

extern PyObject *pythoncom_IsGatewayRegistered(PyObject *self, PyObject *args);

extern PyObject *g_obPyCom_MapIIDToType;
extern PyObject *g_obPyCom_MapGatewayIIDToName;
extern PyObject *g_obPyCom_MapInterfaceNameToIID;
extern PyObject *g_obPyCom_MapRecordGUIDToRecordClass;

PyObject *g_obEmpty = NULL;
PyObject *g_obMissing = NULL;
PyObject *g_obArgNotFound = NULL;
PyObject *g_obNothing = NULL;
PyObject *PyCom_InternalError = NULL;

// Storage related functions.
extern PyObject *pythoncom_StgOpenStorage(PyObject *self, PyObject *args);
extern PyObject *pythoncom_StgOpenStorageEx(PyObject *self, PyObject *args, PyObject *kwargs);
extern PyObject *pythoncom_StgCreateStorageEx(PyObject *self, PyObject *args, PyObject *kwargs);
extern PyObject *pythoncom_FmtIdToPropStgName(PyObject *self, PyObject *args);
extern PyObject *pythoncom_PropStgNameToFmtId(PyObject *self, PyObject *args);

extern PyObject *pythoncom_StgIsStorageFile(PyObject *self, PyObject *args);
extern PyObject *pythoncom_StgCreateDocfile(PyObject *self, PyObject *args);
extern PyObject *pythoncom_StgCreateDocfileOnILockBytes(PyObject *self, PyObject *args);
extern PyObject *pythoncom_StgOpenStorageOnILockBytes(PyObject *self, PyObject *args);
extern PyObject *pythoncom_WriteClassStg(PyObject *self, PyObject *args);
extern PyObject *pythoncom_ReadClassStg(PyObject *self, PyObject *args);
extern PyObject *pythoncom_WriteClassStm(PyObject *self, PyObject *args);
extern PyObject *pythoncom_ReadClassStm(PyObject *self, PyObject *args);
extern PyObject *pythoncom_CreateStreamOnHGlobal(PyObject *self, PyObject *args);
extern PyObject *pythoncom_CreateILockBytesOnHGlobal(PyObject *self, PyObject *args);

extern PyObject *pythoncom_GetRecordFromGuids(PyObject *self, PyObject *args);
extern PyObject *pythoncom_GetRecordFromTypeInfo(PyObject *self, PyObject *args);

extern PyObject *Py_NewSTGMEDIUM(PyObject *self, PyObject *args);

// Typelib related functions
extern PyObject *pythoncom_loadtypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_loadregtypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_registertypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_unregistertypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_querypathofregtypelib(PyObject *self, PyObject *args);

// Type object helpers
PyObject *Py_NewFUNCDESC(PyObject *self, PyObject *args);
PyObject *Py_NewTYPEATTR(PyObject *self, PyObject *args);
PyObject *Py_NewVARDESC(PyObject *self, PyObject *args);

// Error related functions
void GetScodeString(SCODE sc, TCHAR *buf, int bufSize);
LPCTSTR GetScodeRangeString(SCODE sc);
LPCTSTR GetSeverityString(SCODE sc);
LPCTSTR GetFacilityString(SCODE sc);

/* Debug/Test helpers */
extern LONG _PyCom_GetInterfaceCount(void);
extern LONG _PyCom_GetGatewayCount(void);

// Function pointers we load at runtime.
#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyCom_BuildPyException(E_NOTIMPL);

// Requires IE 5.5 or later
typedef HRESULT(STDAPICALLTYPE *CreateURLMonikerExfunc)(LPMONIKER, LPCWSTR, LPMONIKER *, DWORD);
static CreateURLMonikerExfunc pfnCreateURLMonikerEx = NULL;

typedef HRESULT(STDAPICALLTYPE *CoWaitForMultipleHandlesfunc)(DWORD dwFlags, DWORD dwTimeout, ULONG cHandles,
                                                              LPHANDLE pHandles, LPDWORD lpdwindex);
static CoWaitForMultipleHandlesfunc pfnCoWaitForMultipleHandles = NULL;
typedef HRESULT(STDAPICALLTYPE *CoGetObjectContextfunc)(REFIID, void **);
static CoGetObjectContextfunc pfnCoGetObjectContext = NULL;
typedef HRESULT(STDAPICALLTYPE *CoGetCancelObjectfunc)(DWORD, REFIID, void **);
static CoGetCancelObjectfunc pfnCoGetCancelObject = NULL;
typedef HRESULT(STDAPICALLTYPE *CoSetCancelObjectfunc)(IUnknown *);
static CoSetCancelObjectfunc pfnCoSetCancelObject = NULL;

// typedefs for the function pointers are in OleAcc.h
LPFNOBJECTFROMLRESULT pfnObjectFromLresult = NULL;

typedef HRESULT(STDAPICALLTYPE *CoCreateInstanceExfunc)(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, ULONG, MULTI_QI *);
static CoCreateInstanceExfunc pfnCoCreateInstanceEx = NULL;
typedef HRESULT(STDAPICALLTYPE *CoInitializeSecurityfunc)(PSECURITY_DESCRIPTOR, LONG, SOLE_AUTHENTICATION_SERVICE *,
                                                          void *, DWORD, DWORD, void *, DWORD, void *);
static CoInitializeSecurityfunc pfnCoInitializeSecurity = NULL;

BOOL PyCom_HasDCom()
{
    static BOOL bHaveDCOM = -1;
    if (bHaveDCOM == -1) {
        HMODULE hMod = GetModuleHandle(_T("ole32.dll"));
        if (hMod) {
            FARPROC fp = GetProcAddress(hMod, "CoInitializeEx");
            bHaveDCOM = (fp != NULL);
        }
        else
            bHaveDCOM = FALSE;  // not much we can do!
    }
    return bHaveDCOM;
}

#ifdef _MSC_VER
#pragma optimize("y", off)
#endif  // _MSC_VER
// This optimisation seems to screw things in release builds...

/* MODULE FUNCTIONS: pythoncom */
// @pymethod <o PyIUnknown>|pythoncom|CoCreateInstance|Create a new instance of an OLE automation server.
static PyObject *pythoncom_CoCreateInstance(PyObject *self, PyObject *args)
{
    PyObject *obCLSID;
    PyObject *obUnk;
    DWORD dwClsContext;
    PyObject *obiid;
    CLSID clsid;
    IUnknown *punk;
    CLSID iid;
    if (!PyArg_ParseTuple(args, "OOiO:CoCreateInstance",
                          &obCLSID,       // @pyparm <o PyIID>|clsid||Class identifier (CLSID) of the object
                          &obUnk,         // @pyparm <o PyIUnknown>|unkOuter||The outer unknown, or None
                          &dwClsContext,  // @pyparm int|context||The create context for the object, combination of
                                          // pythoncom.CLSCTX_* flags
                          &obiid))        // @pyparm <o PyIID>|iid||The IID required from the object
        return NULL;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        return NULL;
    if (!PyWinObject_AsIID(obiid, &iid))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&punk, TRUE))
        return NULL;
    // Make the call.
    IUnknown *result = NULL;
    PY_INTERFACE_PRECALL;
    SCODE sc = CoCreateInstance(clsid, punk, dwClsContext, iid, (void **)&result);
    if (punk)
        punk->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(sc))
        return PyCom_BuildPyException(sc);
    return PyCom_PyObjectFromIUnknown(result, iid);
}
#ifdef _MSC_VER
#pragma optimize("", on)
#endif  // _MSC_VER

#ifdef _MSC_VER
#pragma optimize("", off)
#endif  // _MSC_VER
// @pymethod <o PyIUnknown>|pythoncom|CoCreateInstanceEx|Create a new instance of an OLE automation server possibly on a
// remote machine.
static PyObject *pythoncom_CoCreateInstanceEx(PyObject *self, PyObject *args)
{
    CHECK_PFN(CoCreateInstanceEx);
    PyObject *obCLSID;
    PyObject *obUnk;
    PyObject *obCoServer;
    DWORD dwClsContext;
    PyObject *obrgiids;
    CLSID clsid;
    COSERVERINFO serverInfo = {0, NULL, NULL, 0};
    COSERVERINFO *pServerInfo = NULL;
    IID *iids = NULL;
    MULTI_QI *mqi = NULL;
    IUnknown *punk = NULL;
    PyObject *result = NULL;
    ULONG numIIDs = 0;
    ULONG i;
    if (!PyArg_ParseTuple(args, "OOiOO:CoCreateInstanceEx",
                          &obCLSID,       // @pyparm <o PyIID>|clsid||Class identifier (CLSID) of the object
                          &obUnk,         // @pyparm <o PyIUnknown>|unkOuter||The outer unknown, or None
                          &dwClsContext,  // @pyparm int|context||The create context for the object, combination of
                                          // pythoncom.CLSCTX_* flags
                          &obCoServer,    // @pyparm (server, authino=None, reserved1=0,reserved2=0)|serverInfo||May be
                                          // None, or describes the remote server to execute on.
                          &obrgiids))     // @pyparm [<o PyIID>, ...]|iids||A list of IIDs required from the object
        return NULL;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        goto done;

    if (obCoServer == Py_None)
        pServerInfo = NULL;
    else {
        pServerInfo = &serverInfo;
        PyObject *obName, *obAuth = Py_None;
        if (!PyArg_ParseTuple(obCoServer, "O|Oii", &obName, &obAuth, &serverInfo.dwReserved1,
                              &serverInfo.dwReserved2)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError, "The SERVERINFO is not in the correct format");
            goto done;
        }
        if (obAuth != Py_None) {
            PyErr_SetString(PyExc_TypeError, "authinfo in the SERVERINFO must be None");
            goto done;
        }
        if (!PyWinObject_AsWCHAR(obName, &serverInfo.pwszName, FALSE))
            goto done;
    }
    if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&punk, TRUE))
        goto done;

    if (!SeqToVector(obrgiids, &iids, &numIIDs, PyWinObject_AsIID))
        goto done;
    mqi = new MULTI_QI[numIIDs];
    if (mqi == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating MULTIQI array");
        goto done;
    }

    for (i = 0; i < numIIDs; i++) {
        mqi[i].pIID = iids + i;
        mqi[i].pItf = NULL;
        mqi[i].hr = 0;
    }

    {  // scoping
        PY_INTERFACE_PRECALL;
        HRESULT hr = (*pfnCoCreateInstanceEx)(clsid, punk, dwClsContext, pServerInfo, numIIDs, mqi);
        PY_INTERFACE_POSTCALL;
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr);
            goto done;
        }
    }  // end scoping

    result = PyTuple_New(numIIDs);
    if (result == NULL)
        goto done;
    for (i = 0; i < numIIDs; i++) {
        PyObject *obNew;
        if (mqi[i].hr == 0) {
            obNew = PyCom_PyObjectFromIUnknown(mqi[i].pItf, *mqi[i].pIID, FALSE);
            mqi[i].pItf = NULL;
            if (!obNew) {
                Py_DECREF(result);
                result = NULL;
                goto done;
            }
        }
        else {
            obNew = Py_None;
            Py_INCREF(Py_None);
        }
        PyTuple_SET_ITEM(result, i, obNew);
    }
done:
    PYCOM_RELEASE(punk);
    if (serverInfo.pwszName)
        PyWinObject_FreeWCHAR(serverInfo.pwszName);

    for (i = 0; i < numIIDs; i++) PYCOM_RELEASE(mqi[i].pItf)
    CoTaskMemFree(iids);
    delete[] mqi;
    return result;
}
#ifdef _MSC_VER
#pragma optimize("", on)
#endif  // _MSC_VER

// @pymethod |pythoncom|CoInitializeSecurity|Registers security and sets the default security values.
static PyObject *pythoncom_CoInitializeSecurity(PyObject *self, PyObject *args)
{
    CHECK_PFN(CoInitializeSecurity);
    DWORD cAuthSvc;
    SOLE_AUTHENTICATION_SERVICE *pAS = NULL;
    DWORD dwAuthnLevel;
    DWORD dwImpLevel;
    DWORD dwCapabilities;
    PSECURITY_DESCRIPTOR pSD = NULL, pSD_absolute = NULL;
    IID appid;
    IAccessControl *pIAC = NULL;
    PyObject *obSD, *obAuthSvc, *obReserved1, *obReserved2, *obAuthInfo;
    if (!PyArg_ParseTuple(
            args, "OOOiiOiO:CoInitializeSecurity",
            &obSD,  // @pyparm <o PySECURITY_DESCRIPTOR>|sd||Security descriptor containing access permissions for
                    // process' objects, can be None. <nl>If Capabilities contains EOAC_APPID, sd should be an AppId
                    // (guid), or None to use server executable. <nl>If Capabilities contains EOAC_ACCESS_CONTROL, sd
                    // parameter should be an IAccessControl interface.
            &obAuthSvc,  // @pyparm object|authSvc||A value of None tells COM to choose which authentication services to
                         // use.  An empty list means use no services.
            &obReserved1,   // @pyparm object|reserved1||Must be None
            &dwAuthnLevel,  // @pyparm int|authnLevel||One of pythoncom.RPC_C_AUTHN_LEVEL_* values. The default
                            // authentication level for proxies. On the server side, COM will fail calls that arrive at
                            // a lower level. All calls to AddRef and Release are made at this level.
            &dwImpLevel,  // @pyparm int|impLevel||One of pythoncom.RPC_C_IMP_LEVEL_* values.  The default impersonation
                          // level for proxies. This value is not checked on the server side. AddRef and Release calls
                          // are made with this impersonation level so even security aware apps should set this
                          // carefully. Setting IUnknown security only affects calls to QueryInterface, not AddRef or
                          // Release.
            &obAuthInfo,  // @pyparm object|authInfo||Must be None
            &dwCapabilities,  // @pyparm int|capabilities||Authentication capabilities, combination of pythoncom.EOAC_*
                              // flags.
            &obReserved2))    // @pyparm object|reserved2||Must be None
        return NULL;
    if (obReserved1 != Py_None || obReserved2 != Py_None || obAuthInfo != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Not all of the 'None' arguments are None!");
        return NULL;
    }

    if (obAuthSvc == Py_None)
        cAuthSvc = (DWORD)-1;
    else if (PySequence_Check(obAuthSvc)) {
        cAuthSvc = 0;
    }
    else {
        PyErr_SetString(PyExc_TypeError, "obAuthSvc must be None or an empty sequence.");
        return NULL;
    }

    // Depending on capabilities flags, first arg can be one of:
    //		AppId (or NULL to lookup server executable in APPID registry key)
    //		IAccessControl interface (cannot be NULL)
    //		Absolute security descriptor (or NULL to use a default SD)
    if (dwCapabilities & EOAC_APPID) {
        if (obSD != Py_None) {
            if (!PyWinObject_AsIID(obSD, &appid))
                return NULL;
            pSD = (PSECURITY_DESCRIPTOR)&appid;
        }
    }
    else if (dwCapabilities & EOAC_ACCESS_CONTROL) {
        if (!PyCom_InterfaceFromPyObject(obSD, IID_IAccessControl, (void **)&pIAC, FALSE))
            return NULL;
        pSD = (PSECURITY_DESCRIPTOR)pIAC;
    }
    else {
        if (!PyWinObject_AsSECURITY_DESCRIPTOR(obSD, &pSD, /*BOOL bNoneOK = */ TRUE))
            return NULL;
        // Security descriptor must be in absolute form
        if (pSD) {
            if (!_MakeAbsoluteSD(pSD, &pSD_absolute))
                return NULL;
            pSD = pSD_absolute;
        }
    }

    PY_INTERFACE_PRECALL;
    HRESULT hr =
        (*pfnCoInitializeSecurity)(pSD, cAuthSvc, pAS, NULL, dwAuthnLevel, dwImpLevel, NULL, dwCapabilities, NULL);
    if (pIAC)
        pIAC->Release();
    PY_INTERFACE_POSTCALL;
    if (pSD_absolute != NULL)
        FreeAbsoluteSD(pSD_absolute);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef _MSC_VER
#pragma optimize("y", off)
#endif  // _MSC_VER
// @pymethod int|pythoncom|CoRegisterClassObject|Registers an EXE class object with OLE so other applications can
// connect to it.
static PyObject *pythoncom_CoRegisterClassObject(PyObject *self, PyObject *args)
{
    DWORD reg;
    DWORD context;
    DWORD flags;
    PyObject *obIID, *obFactory;
    IID iid;

    if (!PyArg_ParseTuple(
            args, "OOii:CoRegisterClassObject",
            &obIID,      // @pyparm <o PyIID>|iid||The IID of the object to register
            &obFactory,  // @pyparm <o PyIUnknown>|factory||The class factory object.  It is the Python programmers
                         // responsibility to ensure this object remains alive until the class is unregistered.
            &context,  // @pyparm int|context||The create context for the server.  Must be a combination of the CLSCTX_*
                       // flags.
            &flags))   // @pyparm int|flags||Create flags.
        return NULL;
    // @comm The class factory object should be <o PyIClassFactory> object, but as per the COM documentation, only <o
    // PyIUnknown> is checked.
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IUnknown *pFactory;
    if (!PyCom_InterfaceFromPyObject(obFactory, IID_IUnknown, (void **)&pFactory, /*BOOL bNoneOK=*/FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = CoRegisterClassObject(iid, pFactory, context, flags, &reg);
    pFactory->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    // @rdesc The result is a handle which should be revoked using <om pythoncom.CoRevokeClassObject>
    return PyLong_FromLong(reg);
}
// @pymethod |pythoncom|CoRevokeClassObject|Informs OLE that a class object, previously registered with the <om
// pythoncom.CoRegisterClassObject> method, is no longer available for use.
static PyObject *pythoncom_CoRevokeClassObject(PyObject *self, PyObject *args)
{
    DWORD reg;

    if (!PyArg_ParseTuple(args, "i:CoRevokeClassObject",
                          &reg))  // @pyparm int|reg||The value returned from <om pythoncom.CoRegisterClassObject>
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = CoRevokeClassObject(reg);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr)) {
        return PyCom_BuildPyException(hr);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|CoResumeClassObjects|Called by a server that can register multiple class objects to inform the
// OLE SCM about all registered classes, and permits activation requests for those class objects.
static PyObject *pythoncom_CoResumeClassObjects(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":CoResumeClassObjects"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoResumeClassObjects();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|CoTreatAsClass|Establishes or removes an emulation, in which objects of one class are treated as
// objects of a different class.
static PyObject *pythoncom_CoTreatAsClass(PyObject *self, PyObject *args)
{
    PyObject *obguid1, *obguid2 = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &obguid1, &obguid2))
        return NULL;
    CLSID clsid1, clsid2 = GUID_NULL;
    // @pyparm <o PyIID>|clsidold||CLSID of the object to be emulated.
    // @pyparm <o PyIID>|clsidnew|CLSID_NULL|CLSID of the object that should emulate the original object. This replaces
    // any existing emulation for clsidOld. Can be ommitted or CLSID_NULL, in which case any existing emulation for
    // clsidOld is removed.
    if (!PyWinObject_AsIID(obguid1, &clsid1))
        return NULL;
    if (obguid2 != NULL && !PyWinObject_AsIID(obguid2, &clsid2))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoTreatAsClass(clsid1, clsid2);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIClassFactory>|pythoncom|MakePyFactory|Creates a new <o PyIClassFactory> object wrapping a PythonCOM
// Class Factory object.
static PyObject *pythoncom_MakePyFactory(PyObject *self, PyObject *args)
{
    PyObject *obIID;
    if (!PyArg_ParseTuple(args, "O:MakePyFactory",
                          &obIID))  // @pyparm <o PyIID>|iid||The IID of the object the class factory provides.
        return NULL;
    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    PY_INTERFACE_PRECALL;
    CPyFactory *pFact = new CPyFactory(iid);
    PY_INTERFACE_POSTCALL;
    if (pFact == NULL)
        return PyCom_BuildPyException(E_OUTOFMEMORY);
    return PyCom_PyObjectFromIUnknown(pFact, IID_IClassFactory, /*bAddRef =*/FALSE);
}

// @pymethod int|pythoncom|_GetInterfaceCount|Retrieves the number of interface objects currently in existance
static PyObject *pythoncom_GetInterfaceCount(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":_GetInterfaceCount"))
        return NULL;
    return PyLong_FromLong(_PyCom_GetInterfaceCount());
    // @comm If is occasionally a good idea to call this function before your Python program
    // terminates.  If this function returns non-zero, then you still have PythonCOM objects
    // alive in your program (possibly in global variables).
}

// @pymethod int|pythoncom|_GetGatewayCount|Retrieves the number of gateway objects currently in existance
static PyObject *pythoncom_GetGatewayCount(PyObject *self, PyObject *args)
{
    // @comm This is the number of Python object that implement COM servers which
    // are still alive (ie, serving a client).  The only way to reduce this count
    // is to have the process which uses these PythonCOM servers release its references.
    if (!PyArg_ParseTuple(args, ":_GetGatewayCount"))
        return NULL;
    return PyLong_FromLong(_PyCom_GetGatewayCount());
}

// @pymethod <o PyIUnknown>|pythoncom|GetActiveObject|Retrieves an object representing a running object registered with
// OLE
static PyObject *pythoncom_GetActiveObject(PyObject *self, PyObject *args)
{
    PyObject *obCLSID;
    // @pyparm CLSID|cls||The IID for the program.  As for all CLSID's in Python, a "program.name" or IID format string
    // may be used, or a real <o PyIID> object.
    if (!PyArg_ParseTuple(args, "O:GetActiveObject", &obCLSID))
        return NULL;
    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        return NULL;
    // Make the call.
    IUnknown *result = NULL;
    PY_INTERFACE_PRECALL;
    SCODE sc = GetActiveObject(clsid, NULL, &result);
    PY_INTERFACE_POSTCALL;
    if (FAILED(sc))
        return PyCom_BuildPyException(sc);
    return PyCom_PyObjectFromIUnknown(result, IID_IUnknown);
}

// @pymethod  <o PyIDispatch>|pythoncom|Connect|Connect to an already running OLE automation server.
static PyObject *pythoncom_connect(PyObject *self, PyObject *args)
{
    PyObject *obCLSID;
    // @pyparm CLSID|cls||An identifier for the program.  Usually "program.item"
    if (!PyArg_ParseTuple(args, "O:Connect", &obCLSID))
        return NULL;
    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        return NULL;

    IUnknown *unk = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = GetActiveObject(clsid, NULL, &unk);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr) || unk == NULL)
        return PyCom_BuildPyException(hr);
    IDispatch *disp = NULL;
    SCODE sc;
    // local scope for macro PY_INTERFACE_PRECALL local variables
    {
        PY_INTERFACE_PRECALL;
        sc = unk->QueryInterface(IID_IDispatch, (void **)&disp);
        unk->Release();
        PY_INTERFACE_POSTCALL;
    }
    if (FAILED(sc) || disp == NULL)
        return PyCom_BuildPyException(sc);
    return PyCom_PyObjectFromIUnknown(disp, IID_IDispatch);
    // @comm This function is equivalent to <om pythoncom.GetActiveObject>(clsid).<om
    // pythoncom.QueryInterace>(pythoncom.IID_IDispatch)
}

// @pymethod <o PyIDispatch>|pythoncom|new|Create a new instance of an OLE automation server.
static PyObject *pythoncom_new(PyObject *self, PyObject *args)
{
    PyErr_Clear();
    PyObject *progid;
    // @pyparm CLSID|cls||An identifier for the program.  Usually "program.item"
    if (!PyArg_ParseTuple(args, "O", &progid))
        return NULL;

    // @comm This is just a wrapper for the CoCreateInstance method.
    // Specifically, this call is identical to:
    // <nl>pythoncom.CoCreateInstance(cls, None, pythoncom.CLSCTX_SERVER, pythoncom.IID_IDispatch)
    int clsctx = PyCom_HasDCom() ? CLSCTX_SERVER : CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER;
    PyObject *obIID = PyWinObject_FromIID(IID_IDispatch);
    PyObject *newArgs = Py_BuildValue("OOiO", progid, Py_None, clsctx, obIID);
    Py_DECREF(obIID);
    PyObject *rc = pythoncom_CoCreateInstance(self, newArgs);
    Py_DECREF(newArgs);
    return rc;
}

// @pymethod <o PyIID>|pythoncom|CreateGuid|Creates a new, unique GUIID.
static PyObject *pythoncom_createguid(PyObject *self, PyObject *args)
{
    PyErr_Clear();
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(PyExc_TypeError, "function requires no arguments");
        return NULL;
    }
    GUID guid;
    PY_INTERFACE_PRECALL;
    CoCreateGuid(&guid);
    PY_INTERFACE_POSTCALL;
    // @comm Use the CreateGuid function when you need an absolutely unique number that you will use as a persistent
    // identifier in a distributed environment.To a very high degree of certainty, this function returns a unique value
    // - no other invocation, on the same or any other system (networked or not), should return the same value.
    return PyWinObject_FromIID(guid);
}

// @pymethod string|pythoncom|ProgIDFromCLSID|Converts a CLSID to a progID.
static PyObject *pythoncom_progidfromclsid(PyObject *self, PyObject *args)
{
    PyObject *obCLSID;
    // @pyparm IID|clsid||A CLSID (either in a string, or in an <o PyIID> object)
    if (!PyArg_ParseTuple(args, "O", &obCLSID))
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid))
        return NULL;
    LPOLESTR progid = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT sc = ProgIDFromCLSID(clsid, &progid);
    PY_INTERFACE_POSTCALL;
    if (FAILED(sc))
        return PyCom_BuildPyException(sc);

    PyObject *ob = MakeOLECHARToObj(progid);
    CoTaskMemFree(progid);
    return ob;
}

// @pymethod string|pythoncom|GetScodeString|Returns the string for an OLE scode (HRESULT)
static PyObject *pythoncom_GetScodeString(PyObject *self, PyObject *args)
{
    SCODE scode;
    TCHAR buf[512];
    // @pyparm int|scode||The OLE error code for the scode string requested.
    if (!PyArg_ParseTuple(args, "k", &scode))
        return NULL;
    GetScodeString(scode, buf, sizeof(buf) / sizeof(buf[0]));
    return PyWinObject_FromTCHAR(buf);
    // @comm This will obtain the COM Error message for a given HRESULT.
    // Internally, PythonCOM uses this function to obtain the description
    // when a <o com_error> COM Exception is raised.
}

// @pymethod string|pythoncom|GetScodeRangeString|Returns the scode range string, given an OLE scode.
static PyObject *pythoncom_GetScodeRangeString(PyObject *self, PyObject *args)
{
    SCODE scode;
    // @pyparm int|scode||An OLE error code to return the scode range string for.
    if (!PyArg_ParseTuple(args, "k", &scode))
        return NULL;
    return PyWinObject_FromTCHAR(GetScodeRangeString(scode));
}

// @pymethod string|pythoncom|GetSeverityString|Returns the severity string, given an OLE scode.
static PyObject *pythoncom_GetSeverityString(PyObject *self, PyObject *args)
{
    SCODE scode;
    // @pyparm int|scode||The OLE error code for the severity string requested.
    if (!PyArg_ParseTuple(args, "k", &scode))
        return NULL;
    return PyWinObject_FromTCHAR(GetSeverityString(scode));
}

// @pymethod string|pythoncom|GetFacilityString|Returns the facility string, given an OLE scode.
static PyObject *pythoncom_GetFacilityString(PyObject *self, PyObject *args)
{
    SCODE scode;
    // @pyparm int|scode||The OLE error code for the facility string requested.
    if (!PyArg_ParseTuple(args, "k", &scode))
        return NULL;
    return PyWinObject_FromTCHAR(GetFacilityString(scode));
}

// @pymethod <o PyIDispatch>|pythoncom|UnwrapObject|Unwraps a Python instance in a gateway object.
static PyObject *pythoncom_UnwrapObject(PyObject *self, PyObject *args)
{
    PyObject *ob;
    // @pyparm <o PyIUnknown>|ob||The object to unwrap.
    if (!PyArg_ParseTuple(args, "O", &ob))
        return NULL;
    // @comm If the object is not a PythonCOM object, then ValueError is raised.
    if (!PyIBase::is_object(ob, &PyIUnknown::type)) {
        PyErr_SetString(PyExc_ValueError, "argument is not a COM object");
        return NULL;
    }

    // Unwrapper does not need thread state management
    // Ie PY_INTERFACE_PRE/POSTCALL;
    HRESULT hr;
    IInternalUnwrapPythonObject *pUnwrapper;
    if (S_OK !=
        (hr = ((PyIUnknown *)ob)->m_obj->QueryInterface(IID_IInternalUnwrapPythonObject, (void **)&pUnwrapper))) {
        PyErr_Format(PyExc_ValueError, "argument is not a Python gateway (0x%x)", hr);
        return NULL;
    }
    PyObject *retval;
    pUnwrapper->Unwrap(&retval);
    pUnwrapper->Release();
    if (S_OK != hr)
        return PyCom_BuildPyException(hr);
    return retval;
    // Use this function to obtain the inverse of the <om WrapObject> method.
    // Eg, if you pass to this function the value you received from <om WrapObject>, it
    // will return the object you originally passed as the parameter to <om WrapObject>
}

// @pymethod <o PyIUnknown>|pythoncom|WrapObject|Wraps a Python instance in a gateway object.
static PyObject *pythoncom_WrapObject(PyObject *self, PyObject *args)
{
    PyObject *ob;
    PyObject *obIID = NULL;
    IID iid = IID_IDispatch;
    PyObject *obIIDInterface = NULL;
    IID iidInterface = IID_IDispatch;

    // @pyparm object|ob||The object to wrap.
    // @pyparm <o PyIID>|gatewayIID|IID_IDispatch|The IID of the gateway object to create (ie, the interface of the
    // server object wrapped by the return value)
    // @pyparm <o PyIID>|interfaceIID|IID_IDispatch|The IID of the interface object to create (ie, the interface of the
    // returned object)
    if (!PyArg_ParseTuple(args, "O|OO", &ob, &obIID, &obIIDInterface))
        return NULL;

    // @rdesc Note that there are 2 objects created by this call - a gateway (server) object, suitable for
    // use by other external COM clients/hosts, as well as the returned Python interface (client) object, which
    // maps to the new gateway.
    // <nl>There are some unusual cases where the 2 IID parameters will not be identical.
    // If you need to do this, you should know exactly what you are doing, and why!
    if (obIID && obIID != Py_None) {
        if (!PyWinObject_AsIID(obIID, &iid))
            return NULL;
    }
    if (obIIDInterface && obIIDInterface != Py_None) {
        if (!PyWinObject_AsIID(obIIDInterface, &iidInterface))
            return NULL;
    }
    // Make a gateway of the specific IID we ask for.
    // The gateway must exist (ie, we _must_ support PyGIXXX

    // XXX - do we need an optional arg for "base object"?
    // XXX - If we did, we would unwrap it like this:
    /****
    IUnknown *pLook = (IUnknown *)(*ppv);
    IInternalUnwrapPythonObject *pTemp;
    if (pLook->QueryInterface(IID_IInternalUnwrapPythonObject, (void **)&pTemp)==S_OK) {
        // One of our objects, so set the base object if it doesn't already have one
        PyGatewayBase *pG = (PyGatewayBase *)pTemp;
        // Eeek - just these few next lines need to be thread-safe :-(
        PyWin_AcquireGlobalLock();
        if (pG->m_pBaseObject==NULL && pG != (PyGatewayBase *)this) {
            pG->m_pBaseObject = this;
            pG->m_pBaseObject->AddRef();
        }
        PyWin_ReleaseGlobalLock();
        pTemp->Release();
    }
    ******/
    IUnknown *pDispatch;
    PY_INTERFACE_PRECALL;
    HRESULT hr = PyCom_MakeRegisteredGatewayObject(iid, ob, NULL, (void **)&pDispatch);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    /* return a PyObject wrapping it */
    return PyCom_PyObjectFromIUnknown(pDispatch, iidInterface, FALSE);
}

static PyObject *pythoncom_MakeIID(PyObject *self, PyObject *args)
{
    PyErr_Warn(PyExc_DeprecationWarning, "MakeIID is deprecated - please use pywintypes.IID() instead.");
    return PyWinMethod_NewIID(self, args);
}

// no autoduck - this is deprecated.
static PyObject *pythoncom_MakeTime(PyObject *self, PyObject *args)
{
    PyErr_Warn(PyExc_DeprecationWarning, "MakeTime is deprecated - please use pywintypes.Time() instead.");
    return PyWinMethod_NewTime(self, args);
}

// @pymethod <o PyIMoniker>,int,<o PyIBindCtx>|pythoncom|MkParseDisplayName|Parses a moniker display name into a moniker
// object. The inverse of <om PyIMoniker.GetDisplayName>
static PyObject *pythoncom_MkParseDisplayName(PyObject *self, PyObject *args)
{
    WCHAR *displayName = NULL;
    PyObject *obDisplayName;
    PyObject *obBindCtx = NULL;

    // @pyparm string|displayName||The display name to parse
    // @pyparm <o PyIBindCtx>|bindCtx|None|The bind context object to use.
    // @comm If a binding context is not provided, then one will be created.
    // Any binding context created or passed in will be returned to the
    // caller.
    if (!PyArg_ParseTuple(args, "O|O:MkParseDisplayName", &obDisplayName, &obBindCtx))
        return NULL;

    if (!PyWinObject_AsWCHAR(obDisplayName, &displayName, FALSE))
        return NULL;

    HRESULT hr;
    IBindCtx *pBC;
    if (obBindCtx == NULL || obBindCtx == Py_None) {
        hr = CreateBindCtx(0, &pBC);
        if (FAILED(hr)) {
            PyWinObject_FreeWCHAR(displayName);
            return PyCom_BuildPyException(hr);
        }

        /* pass the pBC ref into obBindCtx */
        if (!(obBindCtx = PyCom_PyObjectFromIUnknown(pBC, IID_IBindCtx, FALSE))) {
            PyWinObject_FreeWCHAR(displayName);
            return NULL;
        }
    }
    else {
        if (!PyCom_InterfaceFromPyObject(obBindCtx, IID_IBindCtx, (LPVOID *)&pBC, FALSE)) {
            PyWinObject_FreeWCHAR(displayName);
            return NULL;
        }

        /* we want our own ref to obBindCtx, but not pBC */
        Py_INCREF(obBindCtx);
        pBC->Release();
    }
    /* at this point: we own a ref to obBindCtx, but not a direct one on pBC
       (obBindCtx itself has an indirect reference to pBC though, so it is still
       safe to use ...)
    */
    ULONG chEaten;
    IMoniker *pmk;
    PY_INTERFACE_PRECALL;
    hr = MkParseDisplayName(pBC, displayName, &chEaten, &pmk);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(displayName);
    if (FAILED(hr)) {
        Py_DECREF(obBindCtx);
        return PyCom_BuildPyException(hr);
    }
    /* build the result */
    return Py_BuildValue("NiN", PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE), chEaten, obBindCtx);
}

// @pymethod <o PyIMoniker>|pythoncom|CreatePointerMoniker|Creates a new <o PyIMoniker> object.
static PyObject *pythoncom_CreatePointerMoniker(PyObject *self, PyObject *args)
{
    PyObject *obUnk;
    // @pyparm <o PyIUnknown>|IUnknown||The interface for the moniker.
    if (!PyArg_ParseTuple(args, "O:CreatePointerMoniker", &obUnk))
        return NULL;

    IUnknown *punk;
    if (!PyCom_InterfaceFromPyObject(obUnk, IID_IUnknown, (LPVOID *)&punk, FALSE))
        return NULL;

    IMoniker *pmk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreatePointerMoniker(punk, &pmk);
    punk->Release();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);

    return PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);
}

// @pymethod <o PyIMoniker>|pythoncom|CreateFileMoniker|Creates a new <o PyIMoniker> object.
static PyObject *pythoncom_CreateFileMoniker(PyObject *self, PyObject *args)
{
    PyObject *obName;
    // @pyparm string|filename||The name of the file.
    if (!PyArg_ParseTuple(args, "O:CreateFileMoniker", &obName))
        return NULL;

    TmpWCHAR Name;
    if (!PyWinObject_AsWCHAR(obName, &Name))
        return NULL;

    IMoniker *pmk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateFileMoniker(Name, &pmk);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr);

    return PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);
}

// @pymethod <o PyIMoniker>|pythoncom|CreateItemMoniker|Creates an item moniker
// that identifies an object within a containing object (typically a compound document).
static PyObject *pythoncom_CreateItemMoniker(PyObject *self, PyObject *args)
{
    PyObject *obDelim, *obItem;
    // @pyparm string|delim||String containing the delimiter (typically "!") used to separate this item's display name
    // from the display name of its containing object.
    // @pyparm string|item||String indicating the containing object's name for the object being identified.
    if (!PyArg_ParseTuple(args, "OO:CreateItemMoniker", &obDelim, &obItem))
        return NULL;

    TmpWCHAR Delim, Item;
    if (!PyWinObject_AsWCHAR(obDelim, &Delim, TRUE))
        return NULL;
    if (!PyWinObject_AsWCHAR(obItem, &Item, FALSE))
        return NULL;

    IMoniker *pmk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateItemMoniker(Delim, Item, &pmk);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);
}

// @pymethod <o PyIMoniker>|pythoncom|CreateURLMonikerEx|Create a URL moniker from a full url or partial url and base
// moniker
// @pyseeapi CreateURLMonikerEx
static PyObject *pythoncom_CreateURLMonikerEx(PyObject *self, PyObject *args)
{
    WCHAR *url = NULL;
    PyObject *obbase, *oburl, *ret = NULL;
    IMoniker *base_moniker = NULL, *output_moniker = NULL;
    HRESULT hr;
    DWORD flags = URL_MK_UNIFORM;
    CHECK_PFN(CreateURLMonikerEx);

    if (!PyArg_ParseTuple(args, "OO|k:CreateURLMonikerEx",
                          &obbase,  // @pyparm <o PyIMoniker>|Context||An IMoniker interface to be used as a base with a
                                    // partial URL, can be None
                          &oburl,   // @pyparm <o PyUNICODE>|URL||Full or partial url for which to create a moniker
                          &flags))  // @pyparm int|Flags|URL_MK_UNIFORM|URL_MK_UNIFORM or URL_MK_LEGACY
        return NULL;

    if (!PyWinObject_AsWCHAR(oburl, &url, FALSE))
        return NULL;
    if (PyCom_InterfaceFromPyObject(obbase, IID_IMoniker, (LPVOID *)&base_moniker, TRUE)) {
        PY_INTERFACE_PRECALL;
        hr = (*pfnCreateURLMonikerEx)(base_moniker, url, &output_moniker, flags);
        if (base_moniker)
            base_moniker->Release();
        PY_INTERFACE_POSTCALL;
        if (FAILED(hr))
            PyCom_BuildPyException(hr);
        else
            ret = PyCom_PyObjectFromIUnknown(output_moniker, IID_IMoniker, FALSE);
    }
    PyWinObject_FreeWCHAR(url);
    return ret;
}

// @pymethod <o PyIID>|pythoncom|GetClassFile|Supplies the CLSID associated with the given filename.
static PyObject *pythoncom_GetClassFile(PyObject *self, PyObject *args)
{
    CLSID clsid;
    PyObject *obFileName;
    TmpWCHAR fname;
    // @pyparm str|fileName||The filename for which you are requesting the associated CLSID.
    if (!PyArg_ParseTuple(args, "O", &obFileName))
        return NULL;
    if (!PyWinObject_AsWCHAR(obFileName, &fname, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = GetClassFile(fname, &clsid);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyWinObject_FromIID(clsid);
}

// @pymethod |pythoncom|CoInitialize|Initialize the COM libraries for the calling thread.
static PyObject *pythoncom_CoInitialize(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":CoInitialize"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = PyCom_CoInitialize(NULL);
    PY_INTERFACE_POSTCALL;
    // @rdesc This function will ignore the RPC_E_CHANGED_MODE error, as
    // that error indicates someone else beat you to the initialization, and
    // did so with a different threading model.  This error is ignored as it
    // still means COM is ready for use on this thread, and as this function
    // does not explicitly specify a threading model the caller probably
    // doesn't care what model it is.
    // <nl>All other COM errors will raise pythoncom.error as usual.  Use
    // <om pythoncom.CoInitializeEx> if you also want to handle the RPC_E_CHANGED_MODE
    // error.
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
    // @comm Apart from the error handling semantics, this is equivalent
    // to <om pythoncom.CoInitializeEx>(pythoncom.COINIT_APARTMENTTHREADED).
    // See <om pythoncom.CoInitializeEx> for a description.
}

// @pymethod |pythoncom|CoInitializeEx|Initialize the COM libraries for the calling thread.
static PyObject *pythoncom_CoInitializeEx(PyObject *self, PyObject *args)
{
    // @rdesc This function will raise pythoncom.error for all error
    // return values, including RPC_E_CHANGED_MODE error.  This is
    // in contrast to <om pythoncom.CoInitialize> which will hide that
    // specific error.  If your code is happy to work in a threading model
    // other than the one you specified, you must explicitly handle
    // (and presumably ignore) this exception.
    DWORD val;
    if (!PyArg_ParseTuple(args, "l:CoInitializeEx", &val))
        // @pyparm int|flags||Flags for the initialization.
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = PyCom_CoInitializeEx(NULL, val);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
    // @comm There is no need to call this for the main Python thread, as it is called
    // automatically by pythoncom (using sys.coinit_flags as the param, or COINIT_APARTMENTTHREADED
    // if sys.coinit_flags does not exist).
    // <nl>You must call this manually if you create a thread which wishes to use COM.
}

// @pymethod |pythoncom|CoUninitialize|Uninitialize the COM libraries for the calling thread.
static PyObject *pythoncom_CoUninitialize(PyObject *self, PyObject *args)
{
    // comm This function is never called automatically by COM (as this seems the better of
    // 2 evils if COM objects are still alive).  If your Python program hangs on termination,
    // add a call to this function before terminating.
    PY_INTERFACE_PRECALL;
    PyCom_CoUninitialize();
    PY_INTERFACE_POSTCALL;
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|CoFreeUnusedLibraries|Unloads any DLLs that are no longer in use and that, when loaded, were
// specified to be freed automatically.
static PyObject *pythoncom_CoFreeUnusedLibraries(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":CoFreeUnusedLibraries"))
        return NULL;
    PY_INTERFACE_PRECALL;
    CoFreeUnusedLibraries();
    PY_INTERFACE_POSTCALL;
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIRunningObjectTable>|pythoncom|GetRunningObjectTable|Creates a new <o PyIRunningObjectTable> object.
static PyObject *pythoncom_GetRunningObjectTable(PyObject *self, PyObject *args)
{
    DWORD reserved = 0;
    // @pyparm int|reserved|0|A reserved parameter.  Should be zero unless you have inside information that I don't!
    if (!PyArg_ParseTuple(args, "|l:GetRunningObjectTable", &reserved))
        return NULL;
    IRunningObjectTable *pROT = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = GetRunningObjectTable(reserved, &pROT);
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pROT, IID_IRunningObjectTable, FALSE);
}

// @pymethod <o PyIBindCtx>|pythoncom|CreateBindCtx|Creates a new <o PyIBindCtx> object.
static PyObject *pythoncom_CreateBindCtx(PyObject *self, PyObject *args)
{
    DWORD reserved = 0;
    if (!PyArg_ParseTuple(args, "|l:CreateBindCtx", &reserved))
        return NULL;
    IBindCtx *pBC = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateBindCtx(reserved, &pBC);
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pBC, IID_IBindCtx, FALSE);
}

// @pymethod int|pythoncom|RegisterActiveObject|Register an object as the active object for its class
static PyObject *pythoncom_RegisterActiveObject(PyObject *self, PyObject *args)
{
    DWORD dwflags = 0, dwkey = 0;
    HRESULT hr;
    CLSID clsid;
    PyObject *obclsid;
    PyObject *obunk;
    IUnknown *punk;

    // @pyparm <o PyIUnknown>|obUnknown||The object to register.
    // @pyparm <o PyIID>|clsid||The CLSID for the object
    // @pyparm int|flags||Flags to use.
    if (!PyArg_ParseTuple(args, "OOi:RegisterActiveObject", &obunk, &obclsid, &dwflags))
        return NULL;

    if (!PyWinObject_AsIID(obclsid, &clsid))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obunk, IID_IUnknown, (void **)&punk), FALSE)
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = RegisterActiveObject(punk, clsid, dwflags, &dwkey);
    punk->Release();
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr);
    return PyLong_FromLong(dwkey);
    // @rdesc The result is a handle which should be pass to <om pythoncom.RevokeActiveObject>
}

// @pymethod |pythoncom|RevokeActiveObject|Ends an object's status as active.
static PyObject *pythoncom_RevokeActiveObject(PyObject *self, PyObject *args)
{
    DWORD dw_x = 0;
    HRESULT hr;

    // @pyparm int|handle||A handle obtained from <om pythoncom.RegisterActiveObject>
    if (!PyArg_ParseTuple(args, "l:RevokeActiveObject", &dw_x))
        return NULL;
    PY_INTERFACE_PRECALL;
    hr = RevokeActiveObject(dw_x, NULL);
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// Some basic marshalling support
// @pymethod <o PyIStream>|pythoncom|CoMarshalInterThreadInterfaceInStream|Marshals an interface pointer from one thread
// to another thread in the same process.
static PyObject *pythoncom_CoMarshalInterThreadInterfaceInStream(PyObject *self, PyObject *args)
{
    PyObject *obIID, *obUnk;
    IID iid;
    if (!PyArg_ParseTuple(args, "OO:CoMarshalInterThreadInterfaceInStream",
                          &obIID,   // @pyparm <o PyIID>|iid||The IID of the interface to marshal.
                          &obUnk))  // @pyparm <o PyIUnknown>|unk||The interface to marshal.
        return NULL;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;
    IUnknown *pUnk;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnk, FALSE))
        return NULL;
    IStream *pStream = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoMarshalInterThreadInterfaceInStream(iid, pUnk, &pStream);
    pUnk->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pStream, IID_IStream, /*BOOL bAddRef*/ FALSE);
}

// @pymethod <o PyIUnknown>|pythoncom|CoGetInterfaceAndReleaseStream|Unmarshals a buffer containing an interface pointer
// and releases the stream when an interface pointer has been marshaled from another thread to the calling thread.
static PyObject *pythoncom_CoGetInterfaceAndReleaseStream(PyObject *self, PyObject *args)
{
    PyObject *obStream, *obIID;
    if (!PyArg_ParseTuple(args, "OO:CoGetInterfaceAndReleaseStream",
                          &obStream,  // @pyparm <o PyIStream>|stream||The stream to unmarshal the object from.
                          &obIID))    // @pyparm <o PyIID>|iid||The IID if the interface to unmarshal.
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IStream *pStream;
    if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
        return NULL;

    IUnknown *pUnk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoGetInterfaceAndReleaseStream(pStream, iid, (void **)&pUnk);
    // pStream is released by this call - no need for me to do it!
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pUnk, iid, /*BOOL bAddRef*/ FALSE);
}

// @pymethod <o PyIUnknown>|pythoncom|CoCreateFreeThreadedMarshaler|Creates an aggregatable object capable of
// context-dependent marshaling.
static PyObject *pythoncom_CoCreateFreeThreadedMarshaler(PyObject *self, PyObject *args)
{
    PyObject *obUnk;
    if (!PyArg_ParseTuple(args, "O:CoCreateFreeThreadedMarshaler",
                          &obUnk))  // @pyparm <o PyIUnknown>|unk||The unknown object to marshal.
        return NULL;

    IUnknown *pUnk;
    if (!PyCom_InterfaceFromPyObject(obUnk, IID_IUnknown, (void **)&pUnk, FALSE))
        return NULL;

    IUnknown *pUnkRet;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoCreateFreeThreadedMarshaler(pUnk, &pUnkRet);
    pUnk->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pUnkRet, IID_IUnknown, FALSE);
}

// @pymethod |pythoncom|CoMarshalInterface|Marshals an interface into a stream
static PyObject *pythoncom_CoMarshalInterface(PyObject *self, PyObject *args)
{
    PyObject *obriid, *obIStream, *obUnk, *ret = NULL;
    IID riid;
    IStream *pIStream = NULL;
    IUnknown *pIUnknown = NULL;
    void *pvdestctxt = NULL;  // reserved
    DWORD destctxt, flags = MSHLFLAGS_NORMAL;
    if (!PyArg_ParseTuple(
            args, "OOOk|k:CoMarshalInterface",
            &obIStream,  // @pyparm <o PyIStream>|Stm||An IStream interface into which marshalled interface will be
                         // written
            &obriid,     // @pyparm <o PyIID>|riid||IID of interface to be marshalled
            &obUnk,      // @pyparm <o PyIUnknown>|Unk||Base IUnknown of the object to be marshalled
            &destctxt,   // @pyparm int|DestContext||MSHCTX_* flag indicating where object will be unmarshalled
            &flags))     // @pyparm int|flags|MSHLFLAGS_NORMAL|MSHLFLAGS_* flag indicating marshalling options
        return NULL;
    if (PyWinObject_AsIID(obriid, &riid) &&
        PyCom_InterfaceFromPyObject(obIStream, IID_IStream, (void **)&pIStream, FALSE) &&
        PyCom_InterfaceFromPyObject(obUnk, IID_IUnknown, (void **)&pIUnknown, FALSE)) {
        PY_INTERFACE_PRECALL;
        HRESULT hr = CoMarshalInterface(pIStream, riid, pIUnknown, destctxt, pvdestctxt, flags);
        PY_INTERFACE_POSTCALL;
        if (FAILED(hr))
            PyCom_BuildPyException(hr);
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    PY_INTERFACE_PRECALL;
    if (pIUnknown)
        pIUnknown->Release();
    if (pIStream)
        pIStream->Release();
    PY_INTERFACE_POSTCALL;
    return ret;
}

// @pymethod interface|pythoncom|CoUnmarshalInterface|Unmarshals an interface
static PyObject *pythoncom_CoUnmarshalInterface(PyObject *self, PyObject *args)
{
    PyObject *obriid, *obIStream, *ret = NULL;
    IID riid;
    IStream *pIStream = NULL;
    IUnknown *pIUnknown = NULL;
    if (!PyArg_ParseTuple(args, "OO:CoUnmarshalInterface",
                          &obIStream,  // @pyparm <o PyIStream>|Stm||Stream containing marshalled interface
                          &obriid))    // @pyparm <o PyIID>|riid||IID of interface to be unmarshalled
        return NULL;
    if (PyWinObject_AsIID(obriid, &riid) &&
        PyCom_InterfaceFromPyObject(obIStream, IID_IStream, (void **)&pIStream, FALSE)) {
        PY_INTERFACE_PRECALL;
        HRESULT hr = CoUnmarshalInterface(pIStream, riid, (void **)&pIUnknown);
        pIStream->Release();
        PY_INTERFACE_POSTCALL;
        if (FAILED(hr))
            PyCom_BuildPyException(hr);
        else
            ret = PyCom_PyObjectFromIUnknown(pIUnknown, riid, FALSE);
    }
    return ret;
}

// @pymethod |pythoncom|CoReleaseMarshalData|Frees resources used by a marshalled interface
// @comm This is usually only needed when the interface could not be successfully unmarshalled
static PyObject *pythoncom_CoReleaseMarshalData(PyObject *self, PyObject *args)
{
    PyObject *obIStream, *ret = NULL;
    IStream *pIStream = NULL;
    if (!PyArg_ParseTuple(args, "O:CoReleaseMarshalData",
                          &obIStream))  // @pyparm <o PyIStream>|Stm||Stream containing marshalled interface
        return NULL;
    if (!PyCom_InterfaceFromPyObject(obIStream, IID_IStream, (void **)&pIStream, FALSE))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoReleaseMarshalData(pIStream);
    pIStream->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIUnknown>|pythoncom|CoGetObject|Converts a display name into a moniker that identifies the object
// named, and then binds to the object identified by the moniker.
static PyObject *pythoncom_CoGetObject(PyObject *self, PyObject *args)
{
    PyObject *obName;
    PyObject *obBindOpts = Py_None;
    IID iid = IID_IUnknown;
    if (!PyArg_ParseTuple(args, "O|OO&:CoGetObject",
                          &obName,      // @pyparm string|name||
                          &obBindOpts,  // @pyparm None|bindOpts|None|Must be None
                          PyWinObject_AsIID,
                          &iid))  // @pyparm <o PyIID>|iid|IID_IUnknown|The IID of the interface to return.
        return NULL;

    if (obBindOpts != Py_None)
        return PyErr_Format(PyExc_ValueError, "BindOptions must be None");
    TmpWCHAR name;
    if (!PyWinObject_AsWCHAR(obName, &name))
        return NULL;

    IUnknown *pUnk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CoGetObject(name, NULL, iid, (void **)&pUnk);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pUnk, iid, /*BOOL bAddRef*/ FALSE);
}

// @pymethod |pythoncom|OleLoad|Loads into memory an object nested within a specified storage object.
static PyObject *pythoncom_OleLoad(PyObject *self, PyObject *args)
{
    PyObject *obStorage, *obIID, *obSite;
    if (!PyArg_ParseTuple(args, "OOO:OleLoad",
                          &obStorage,  // @pyparm <o PyIStorage>|storage||The storage object from which to load
                          &obIID,      // @pyparm <o PyIID>|iid||The IID if the interface to load.
                          &obSite))    // @pyparm <o PyIOleClientSite>|site||The client site for the object.
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IStorage *pStorage;
    if (!PyCom_InterfaceFromPyObject(obStorage, IID_IStorage, (void **)&pStorage, FALSE))
        return NULL;
    IOleClientSite *pSite;
    if (!PyCom_InterfaceFromPyObject(obSite, IID_IOleClientSite, (void **)&pSite, TRUE)) {
        pStorage->Release();
        return NULL;
    }

    IUnknown *pUnk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = OleLoad(pStorage, iid, pSite, (void **)&pUnk);
    pStorage->Release();
    pSite->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pUnk, iid, /*BOOL bAddRef*/ FALSE);
}

// @pymethod |pythoncom|OleLoadFromStream|Load an object from an IStream.
static PyObject *pythoncom_OleLoadFromStream(PyObject *self, PyObject *args)
{
    PyObject *obStream, *obIID;
    if (!PyArg_ParseTuple(args, "OO:OleLoadFromStream",
                          &obStream,  // @pyparm <o PyIStream>|stream||The stream to load the object from.
                          &obIID))    // @pyparm <o PyIID>|iid||The IID if the interface to load.
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IStream *pStream;
    if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
        return NULL;

    IUnknown *pUnk;
    PY_INTERFACE_PRECALL;
    HRESULT hr = OleLoadFromStream(pStream, iid, (void **)&pUnk);
    pStream->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pUnk, iid, /*BOOL bAddRef*/ FALSE);
}

// @pymethod |pythoncom|OleSaveToStream|Save an object to an IStream.
static PyObject *pythoncom_OleSaveToStream(PyObject *self, PyObject *args)
{
    PyObject *obPersist, *obStream;
    if (!PyArg_ParseTuple(args, "OO:OleSaveToStream",
                          &obPersist,  // @pyparm <o PyIPersistStream>|persist||The object to save
                          &obStream))  // @pyparm <o PyIStream>|stream||The stream to save the object to.
        return NULL;

    // This parameter is allowed to be None. This follows the COM documentation rather
    // than the COM implementation, which is likely to return an error if you do pass
    // it a NULL IPersistStream
    IPersistStream *pPersist;
    if (!PyCom_InterfaceFromPyObject(obPersist, IID_IPersistStream, (void **)&pPersist, FALSE))
        return NULL;

    IStream *pStream;
    if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE)) {
        PYCOM_RELEASE(pPersist);
        return NULL;
    }

    PY_INTERFACE_PRECALL;
    HRESULT hr = OleSaveToStream(pPersist, pStream);
    pStream->Release();
    if (pPersist)
        pPersist->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o ICreateTypeLib>|pythoncom|CreateTypeLib|Provides access to a new object instance that supports the
// ICreateTypeLib interface.
static PyObject *pythoncom_CreateTypeLib(PyObject *self, PyObject *args)
{
    long syskind;
    PyObject *obfname;
    if (!PyArg_ParseTuple(args, "lO", &syskind, &obfname))
        return NULL;
    TmpWCHAR fname;
    if (!PyWinObject_AsWCHAR(obfname, &fname))
        return NULL;
    ICreateTypeLib *pcti = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateTypeLib((SYSKIND)syskind, fname, &pcti);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pcti, IID_ICreateTypeLib, FALSE);
}

// @pymethod <o ICreateTypeLib2>|pythoncom|CreateTypeLib2|Provides access to a new object instance that supports the
// ICreateTypeLib2 interface.
static PyObject *pythoncom_CreateTypeLib2(PyObject *self, PyObject *args)
{
    long syskind;
    PyObject *obfname;
    if (!PyArg_ParseTuple(args, "lO", &syskind, &obfname))
        return NULL;
    TmpWCHAR fname;
    if (!PyWinObject_AsWCHAR(obfname, &fname))
        return NULL;
    ICreateTypeLib2 *pcti = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = CreateTypeLib2((SYSKIND)syskind, fname, &pcti);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pcti, IID_ICreateTypeLib2, FALSE);
}

// @pymethod int|pythoncom|PumpWaitingMessages|Pumps all waiting messages for the current thread.
// @comm It is sometimes necessary for a COM thread to have a message loop.  This function
// can be used with <om win32event.MsgWaitForMultipleObjects> to pump all messages
// when necessary.  Please see the COM documentation for more details.
// @rdesc Returns 1 if a WM_QUIT message was received, else 0
static PyObject *pythoncom_PumpWaitingMessages(PyObject *self, PyObject *args)
{
    UINT firstMsg = 0, lastMsg = 0;
    if (!PyArg_ParseTuple(args, "|ii:PumpWaitingMessages", &firstMsg, &lastMsg))
        return NULL;
    // @pyseeapi PeekMessage and DispatchMessage

    MSG msg;
    long result = 0;
    // Read all of the messages in this next loop,
    // removing each message as we read it.
    Py_BEGIN_ALLOW_THREADS while (PeekMessage(&msg, NULL, firstMsg, lastMsg, PM_REMOVE))
    {
        // If it's a quit message, we're out of here.
        if (msg.message == WM_QUIT) {
            result = 1;
            break;
        }
        // Otherwise, dispatch the message.
        DispatchMessage(&msg);
    }  // End of PeekMessage while loop
    Py_END_ALLOW_THREADS return PyLong_FromLong(result);
}

// @pymethod |pythoncom|PumpMessages|Pumps all messages for the current thread until a WM_QUIT message.
static PyObject *pythoncom_PumpMessages(PyObject *self, PyObject *args)
{
    MSG msg;
    int rc;
    Py_BEGIN_ALLOW_THREADS while ((rc = GetMessage(&msg, 0, 0, 0)) == 1)
    {
        TranslateMessage(&msg);  // needed?
        DispatchMessage(&msg);
    }
    Py_END_ALLOW_THREADS if (rc == -1) return PyWin_SetAPIError("GetMessage");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|EnableQuitMessage|Indicates the thread PythonCOM should post a WM_QUIT message to.
static PyObject *pythoncom_EnableQuitMessage(PyObject *self, PyObject *args)
{
    extern void PyCom_EnableQuitMessage(DWORD dwThreadId);

    DWORD id;
    // @pyparm int|threadId||The thread ID.
    if (!PyArg_ParseTuple(args, "l:EnableQuitMessage", &id))
        return NULL;
    PyCom_EnableQuitMessage(id);
    Py_INCREF(Py_None);
    return Py_None;
}

static BOOL MakeHandleList(PyObject *handleList, HANDLE **ppBuf, DWORD *pNumEntries)
{
    if (!PySequence_Check(handleList)) {
        PyErr_SetString(PyExc_TypeError, "Handles must be a list of integers");
        return FALSE;
    }
    DWORD numItems = (DWORD)PySequence_Length(handleList);
    HANDLE *pItems = (HANDLE *)malloc(sizeof(HANDLE) * numItems);
    if (pItems == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating array of handles");
        return FALSE;
    }
    for (DWORD i = 0; i < numItems; i++) {
        PyObject *obItem = PySequence_GetItem(handleList, i);
        if (obItem == NULL) {
            free(pItems);
            return FALSE;
        }
        if (!PyWinObject_AsHANDLE(obItem, pItems + i)) {
            Py_DECREF(obItem);
            free(pItems);
            PyErr_SetString(PyExc_TypeError, "Handles must be a list of integers");
            return FALSE;
        }
        Py_DECREF(obItem);
    }
    *ppBuf = pItems;
    *pNumEntries = numItems;
    return TRUE;
}

// @pymethod int|pythoncom|CoWaitForMultipleHandles|Waits for specified handles to be signaled or for a specified
// timeout period to elapse.
static PyObject *pythoncom_CoWaitForMultipleHandles(PyObject *self, PyObject *args)
{
    DWORD flags, timeout;
    PyObject *obHandles;
    DWORD numItems;
    HANDLE *pItems = NULL;
    CHECK_PFN(CoWaitForMultipleHandles);

    if (!PyArg_ParseTuple(args, "iiO:CoWaitForMultipleHandles",
                          &flags,       // @pyparm int|Flags||Combination of pythoncom.COWAIT_* values
                          &timeout,     // @pyparm int|Timeout||Timeout in milliseconds
                          &obHandles))  // @pyparm [<o PyHANDLE>, ...]|Handles||Sequence of handles
        return NULL;
    if (!MakeHandleList(obHandles, &pItems, &numItems))
        return NULL;
    DWORD index;
    PyObject *rc = NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = (*pfnCoWaitForMultipleHandles)(flags, timeout, numItems, pItems, &index);
    Py_END_ALLOW_THREADS if (FAILED(hr)) { PyCom_BuildPyException(hr); }
    else rc = PyLong_FromLong(index);
    free(pItems);
    return rc;
}

// @pymethod <o PyIDataObject>|pythoncom|OleGetClipboard|Retrieves a data object that you can use to access the contents
// of the clipboard.
static PyObject *pythoncom_OleGetClipboard(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OleGetClipboard"))
        return NULL;
    IDataObject *pd = NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::OleGetClipboard(&pd);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    return PyCom_PyObjectFromIUnknown(pd, IID_IDataObject, FALSE);
}

// @pymethod |pythoncom|OleSetClipboard|Places a pointer to a specific data object onto the clipboard. This makes the
// data object accessible to the OleGetClipboard function.
static PyObject *pythoncom_OleSetClipboard(PyObject *, PyObject *args)
{
    PyObject *obd;
    // @pyparm <o PyIDataObject>|dataObj||The data object to place on the clipboard.
    // This parameter can be None in which case the clipboard is emptied.
    if (!PyArg_ParseTuple(args, "O:OleSetClipboard", &obd))
        return NULL;
    IDataObject *pd;
    if (!PyCom_InterfaceFromPyObject(obd, IID_IDataObject, (void **)&pd, TRUE))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::OleSetClipboard(pd);
    Py_END_ALLOW_THREADS if (pd) pd->Release();
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod true/false|pythoncom|OleIsCurrentClipboard|Determines whether the data object pointer previously placed on
// the clipboard by the OleSetClipboard function is still on the clipboard.
static PyObject *pythoncom_OleIsCurrentClipboard(PyObject *, PyObject *args)
{
    PyObject *obd;
    if (!PyArg_ParseTuple(args, "O:OleIsCurrentClipboard", &obd))
        return NULL;
    // @pyparm <o PyIDataObject>|dataObj||The data object to check
    IDataObject *pd;
    if (!PyCom_InterfaceFromPyObject(obd, IID_IDataObject, (void **)&pd, FALSE))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::OleIsCurrentClipboard(pd);
    Py_END_ALLOW_THREADS pd->Release();
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    PyObject *ret = hr == S_OK ? Py_True : Py_False;
    Py_INCREF(ret);
    return ret;
}

// @pymethod |pythoncom|OleFlushClipboard|Carries out the clipboard shutdown sequence. It also releases the IDataObject
// pointer that was placed on the clipboard by the <om pythoncom.OleSetClipboard> function.
static PyObject *pythoncom_OleFlushClipboard(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OleFlushClipboard"))
        return NULL;

    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::OleFlushClipboard();
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|RegisterDragDrop|Registers the specified window as
// one that can be the target of an OLE drag-and-drop operation and
// specifies the <o PyIDropTarget> instance to use for drop operations.
static PyObject *pythoncom_RegisterDragDrop(PyObject *, PyObject *args)
{
    PyObject *obd;
    HWND hwnd;
    PyObject *obhwnd;
    if (!PyArg_ParseTuple(args, "OO:RegisterDragDrop", &obhwnd, &obd))
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    // @pyparm <o PyHANDLE>|hwnd||Handle to a window
    // @pyparm <o PyIDropTarget>|dropTarget||Object that implements the IDropTarget interface
    IDropTarget *dt;
    if (!PyCom_InterfaceFromPyObject(obd, IID_IDropTarget, (void **)&dt, FALSE))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::RegisterDragDrop(hwnd, dt);
    Py_END_ALLOW_THREADS dt->Release();
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|RevokeDragDrop|Revokes the registration of the
// specified application window as a potential target for OLE drag-and-drop
// operations.
static PyObject *pythoncom_RevokeDragDrop(PyObject *, PyObject *args)
{
    HWND hwnd;
    PyObject *obhwnd;
    // @pyparm <o PyHANDLE>|hwnd||Handle to a window registered as an OLE drop target.
    if (!PyArg_ParseTuple(args, "O:RevokeDragDrop", &obhwnd))
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::RevokeDragDrop(hwnd);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|DoDragDrop|Carries out an OLE drag and drop operation.
static PyObject *pythoncom_DoDragDrop(PyObject *, PyObject *args)
{
    PyObject *obdo, *obds;
    DWORD effects;
    if (!PyArg_ParseTuple(args, "OOl:DoDragDrop", &obdo, &obds, &effects))
        return NULL;
    IDropSource *ds;
    if (!PyCom_InterfaceFromPyObject(obds, IID_IDropSource, (void **)&ds, FALSE))
        return NULL;
    IDataObject *dob;
    if (!PyCom_InterfaceFromPyObject(obdo, IID_IDataObject, (void **)&dob, FALSE)) {
        ds->Release();
        return NULL;
    }
    HRESULT hr;
    DWORD retEffect = 0;
    Py_BEGIN_ALLOW_THREADS hr = ::DoDragDrop(dob, ds, effects, &retEffect);
    Py_END_ALLOW_THREADS ds->Release();
    dob->Release();
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    return PyLong_FromLong(retEffect);
}

// @pymethod |pythoncom|OleInitialize|Calls OleInitialized - this should rarely
// be needed, although some clipboard operations insist this is called rather
// than <om pythoncom.CoInitialize>
static PyObject *pythoncom_OleInitialize(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OleInitialize"))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::OleInitialize(NULL);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIUnknown>|pythoncom|ObjectFromLresult|Retrieves a requested
// interface pointer for an object based on a previously generated object reference.
static PyObject *pythoncom_ObjectFromLresult(PyObject *self, PyObject *args)
{
    PyObject *oblresult;
    PyObject *obIID = NULL;
    PyObject *obwparam;
    // @pyparm int|lresult||
    // @pyparm <o PyIID>|iid||The IID to query
    // @pyparm int|wparm||
    LRESULT lresult;
    WPARAM wparam;
    IID iid;
    if (!PyArg_ParseTuple(args, "OOO", &oblresult, &obIID, &obwparam))
        return NULL;
    if (!PyWinLong_AsULONG_PTR(oblresult, (ULONG_PTR *)&lresult))
        return NULL;
    if (!PyWinLong_AsULONG_PTR(obwparam, (ULONG_PTR *)&wparam))
        return NULL;
    if (obIID && !PyWinObject_AsIID(obIID, &iid))
        return NULL;

    // GIL protects us from races here.
    if (pfnObjectFromLresult == NULL) {
        HMODULE hmod = LoadLibrary(_T("oleacc.dll"));
        if (hmod)
            pfnObjectFromLresult = (LPFNOBJECTFROMLRESULT)GetProcAddress(hmod, "ObjectFromLresult");
    }
    if (pfnObjectFromLresult == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "Not available on this platform");

    HRESULT hr;
    void *ret = 0;
    Py_BEGIN_ALLOW_THREADS hr = (*pfnObjectFromLresult)(lresult, iid, wparam, &ret);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    // docs for ObjectFromLresult don't mention reference counting, but
    // it does say that you can't call this twice on the same object, and
    // it has a signature that implies normal reference counting.  So
    // we assume this call has already added a reference to the result.
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, iid, FALSE);
}

// @pymethod <o PyIUnknown>|pythoncom|ObjectFromAddress|Returns a COM object given its address in memory.
// @rdesc This method is useful for applications which return objects via non-standard
// mechanisms - eg, Windows Explorer allows you to send a specific message to the
// explorer window and the result will be the address of an object Explorer implements.
// This method allows you to recover the object from that address.
static PyObject *pythoncom_ObjectFromAddress(PyObject *self, PyObject *args)
{
    IID iid = IID_IUnknown;
    void *addr;
    PyObject *obAddr;
    PyObject *obIID = NULL;
    // @pyparm int|address||The address which holds a COM object
    // @pyparm <o PyIID>|iid|IUnknown|The IID to query
    if (!PyArg_ParseTuple(args, "O|O", &obAddr, &obIID))
        return NULL;
    if (!PyWinLong_AsVoidPtr(obAddr, &addr))
        return NULL;
    if (obIID && !PyWinObject_AsIID(obIID, &iid))
        return NULL;

    HRESULT hr;
    IUnknown *ret = 0;
    PyThreadState *_save = PyEval_SaveThread();
    PYWINTYPES_TRY
    {
        hr = ((IUnknown *)addr)->QueryInterface(iid, (void **)&ret);
        PyEval_RestoreThread(_save);
    }
    PYWINTYPES_EXCEPT
    {
        PyEval_RestoreThread(_save);
        return PyErr_Format(PyExc_ValueError,
                            "Address is not a valid COM object (win32 exception attempting to retrieve it!)");
    }
    if (FAILED(hr) || ret == 0) {
        PyCom_BuildPyException(hr);
        return NULL;
    }
    // We've added a reference via the QI above.
    return PyCom_PyObjectFromIUnknown(ret, iid, FALSE);
}

// @pymethod <o PyIServerSecurity>|pythoncom|CoGetCallContext|Creates interfaces used to access client security
// requirements and perform impersonation
// @comm ISecurityCallContext will only be available for a server that uses role-based security
static PyObject *pythoncom_CoGetCallContext(PyObject *self, PyObject *args)
{
    // @pyparm <o PyIID>|riid|IID_IServerSecurity|The interface to create,
    //	IID_IServerSecurity or IID_ISecurityCallContext
    IID riid = IID_IServerSecurity;
    void *ret;
    if (!PyArg_ParseTuple(args, "|O&:CoGetCallContext", PyWinObject_AsIID, &riid))
        return NULL;

    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = CoGetCallContext(riid, &ret);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid, FALSE);
}

// @pymethod <o PyIContext>|pythoncom|CoGetObjectContext|Creates an interface to interact with the context of the
// current object
// @comm Requires Win2k or later
// @comm COM applications can use this function to create IComThreadingInfo, IContext, or IContextCallback
//	COM+ applications may also create IObjectContext, IObjectContextInfo, IObjectContextActivity, or IContextState
static PyObject *pythoncom_CoGetObjectContext(PyObject *self, PyObject *args)
{
    CHECK_PFN(CoGetObjectContext);
    // @pyparm <o PyIID>|riid|IID_IContext|The interface to return
    IID riid = IID_IContext;
    void *ret;
    if (!PyArg_ParseTuple(args, "|O&:CoGetObjectContext", PyWinObject_AsIID, &riid))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = (*pfnCoGetObjectContext)(riid, &ret);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid, FALSE);
}

// @pymethod <o PyICancelMethodCalls>|pythoncom|CoGetCancelObject|Retrieves an interface used to cancel a pending call
// @comm Requires Win2k or later
static PyObject *pythoncom_CoGetCancelObject(PyObject *self, PyObject *args)
{
    CHECK_PFN(CoGetCancelObject);
    // @pyparm int|ThreadID|0|Id of thread with pending call, or 0 for current thread
    // @pyparm <o PyIID>|riid|IID_ICancelMethodCalls|The interface to return
    DWORD tid = 0;
    IID riid = IID_ICancelMethodCalls;
    void *ret;
    if (!PyArg_ParseTuple(args, "|kO&:CoGetCancelObject", &tid, PyWinObject_AsIID, &riid))
        return NULL;
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = (*pfnCoGetCancelObject)(tid, riid, &ret);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown((IUnknown *)ret, riid, FALSE);
}

// @pymethod |pythoncom|CoSetCancelObject|Sets or removes a <o PyICancelMethodCalls> interface to be used on the current
// thread
// @comm Requires Win2k or later
static PyObject *pythoncom_CoSetCancelObject(PyObject *self, PyObject *args)
{
    CHECK_PFN(CoSetCancelObject);
    // @pyparm <o PyIUnknown>|Unk||An interface that support ICancelMethodCalls, can be None to unregister current
    // cancel object
    IUnknown *pUnk;
    PyObject *obUnk;
    if (!PyArg_ParseTuple(args, "O:CoSetCancelObject", &obUnk))
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnk, TRUE))
        return NULL;

    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = (*pfnCoSetCancelObject)(pUnk);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|CoEnableCallCancellation|Enables call cancellation for synchronous calls on the current thread
static PyObject *pythoncom_CoEnableCallCancellation(PyObject *self, PyObject *args)
{
    // Only param is reserved
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = CoEnableCallCancellation(NULL);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |pythoncom|CoDisableCallCancellation|Disables call cancellation for synchronous calls on the current thread
static PyObject *pythoncom_CoDisableCallCancellation(PyObject *self, PyObject *args)
{
    // Only param is reserved
    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = CoDisableCallCancellation(NULL);
    Py_END_ALLOW_THREADS if (FAILED(hr)) return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

/* List of module functions */
// @module pythoncom|A module, encapsulating the OLE automation API
static struct PyMethodDef pythoncom_methods[] = {
    {"_GetInterfaceCount", pythoncom_GetInterfaceCount,
     1},  // @pymeth _GetInterfaceCount|Retrieves the number of interface objects currently in existance
    {"_GetGatewayCount", pythoncom_GetGatewayCount,
     1},  // @pymeth _GetInterfaceCount|Retrieves the number of gateway objects currently in existance
    {"CoCreateFreeThreadedMarshaler", pythoncom_CoCreateFreeThreadedMarshaler,
     1},  // @pymeth CoCreateFreeThreadedMarshaler|Creates an aggregatable object capable of context-dependent
          // marshaling.
    {"CoCreateInstanceEx", pythoncom_CoCreateInstanceEx,
     1},  // @pymeth CoCreateInstanceEx|Create a new instance of an OLE automation server possibly on a remote machine.
    {"CoCreateInstance", pythoncom_CoCreateInstance,
     1},  // @pymeth CoCreateInstance|Create a new instance of an OLE automation server.
    {"CoFreeUnusedLibraries", pythoncom_CoFreeUnusedLibraries,
     1},  // @pymeth CoFreeUnusedLibraries|Unloads any DLLs that are no longer in use and that, when loaded, were
          // specified to be freed automatically.
    {"CoInitialize", pythoncom_CoInitialize,
     1},  // @pymeth CoInitialize|Initialize the COM libraries for the calling thread.
    {"CoInitializeEx", pythoncom_CoInitializeEx,
     1},  // @pymeth CoInitializeEx|Initialize the COM libraries for the calling thread.
    {"CoInitializeSecurity", pythoncom_CoInitializeSecurity,
     1},  // @pymeth CoInitializeSecurity|Registers security and sets the default security values.
    {"CoGetInterfaceAndReleaseStream", pythoncom_CoGetInterfaceAndReleaseStream,
     1},  // @pymeth CoGetInterfaceAndReleaseStream|Unmarshals a buffer containing an interface pointer and releases the
          // stream when an interface pointer has been marshaled from another thread to the calling thread.
    {"CoMarshalInterThreadInterfaceInStream", pythoncom_CoMarshalInterThreadInterfaceInStream,
     1},  // @pymeth CoMarshalInterThreadInterfaceInStream|Marshals an interface pointer from one thread to another
          // thread in the same process.
    {"CoMarshalInterface", pythoncom_CoMarshalInterface,
     1},  // @pymeth CoMarshalInterface|Marshals an interface into a stream
    {"CoUnmarshalInterface", pythoncom_CoUnmarshalInterface,
     1},  // @pymeth CoUnmarshalInterface|Unmarshals an interface
    {"CoReleaseMarshalData", pythoncom_CoReleaseMarshalData,
     1},  // @pymeth CoReleaseMarshalData|Frees resources used by a marshalled interface
    {"CoGetObject", pythoncom_CoGetObject,
     1},  // @pymeth CoGetObject|Converts a display name into a moniker that identifies the object named, and then binds
          // to the object identified by the moniker.
    {"CoUninitialize", pythoncom_CoUninitialize, 1},  // @pymeth CoUninitialize|Uninitialize the COM libraries.
    {"CoRegisterClassObject", pythoncom_CoRegisterClassObject,
     1},  // @pymeth CoRegisterClassObject|Registers an EXE class object with OLE so other applications can connect to
          // it.
    {"CoResumeClassObjects", pythoncom_CoResumeClassObjects,
     1},  // @pymeth CoResumeClassObjects|Called by a server that can register multiple class objects to inform the OLE
          // SCM about all registered classes, and permits activation requests for those class objects.
    {"CoRevokeClassObject", pythoncom_CoRevokeClassObject,
     1},  // @pymeth CoRevokeClassObject|Informs OLE that a class object, previously registered with the <om
          // pythoncom.CoRegisterClassObject> method, is no longer available for use.
    {"CoTreatAsClass", pythoncom_CoTreatAsClass,
     1},  // @pymeth CoTreatAsClass|Establishes or removes an emulation, in which objects of one class are treated as
          // objects of a different class.
    {"CoWaitForMultipleHandles", pythoncom_CoWaitForMultipleHandles,
     1},  // @pymeth CoWaitForMultipleHandles|Waits for specified handles to be signaled or for a specified timeout
          // period to elapse.
    {"Connect", pythoncom_connect, 1},  // @pymeth Connect|Connects to a running instance of an OLE automation server.
    {"connect", pythoncom_connect, 1},
    {"CreateGuid", pythoncom_createguid, 1},        // @pymeth CreateGuid|Creates a new, unique GUIID.
    {"CreateBindCtx", pythoncom_CreateBindCtx, 1},  // @pymeth CreateBindCtx|Obtains a <o PyIBindCtx> object.
    {"CreateFileMoniker", pythoncom_CreateFileMoniker,
     1},  // @pymeth CreateFileMoniker|Creates a file moniker given a file name.
    {"CreateItemMoniker", pythoncom_CreateItemMoniker,
     1},  // @pymeth CreateItemMoniker|Creates an item moniker that identifies an object within a containing object
          // (typically a compound document).
    {"CreatePointerMoniker", pythoncom_CreatePointerMoniker,
     1},  // @pymeth CreatePointerMoniker|Creates a pointer moniker based on a pointer to an object.

    {"CreateURLMonikerEx", pythoncom_CreateURLMonikerEx,
     1},  // @pymeth CreateURLMoniker|Create a URL moniker from a full url or partial url and base moniker

    {"CreateTypeLib", pythoncom_CreateTypeLib,
     1},  // @pymeth CreateTypeLib|Provides access to a new object instance that supports the ICreateTypeLib interface.
    {"CreateTypeLib2", pythoncom_CreateTypeLib2, 1},  // @pymeth CreateTypeLib2|Provides access to a new object instance
                                                      // that supports the ICreateTypeLib2 interface.
    {"CreateStreamOnHGlobal", pythoncom_CreateStreamOnHGlobal,
     1},  // @pymeth CreateStreamOnHGlobal|Creates an in-memory stream storage object
    {"CreateILockBytesOnHGlobal", pythoncom_CreateILockBytesOnHGlobal,
     1},  // @pymeth CreateILockBytesOnHGlobal|Creates an ILockBytes interface based on global memory

    {"EnableQuitMessage", pythoncom_EnableQuitMessage,
     1},  // @pymeth EnableQuitMessage|Indicates the thread PythonCOM should post a WM_QUIT message to.
    {"FUNCDESC", Py_NewFUNCDESC, 1},  // @pymeth FUNCDESC|Returns a new <o FUNCDESC> object.
    {"GetActiveObject", pythoncom_GetActiveObject,
     1},  // @pymeth GetActiveObject|Retrieves an object representing a running object registered with OLE
    {"GetClassFile", pythoncom_GetClassFile,
     1},  // @pymeth GetClassFile|Supplies the CLSID associated with the given filename.
    {"GetFacilityString", pythoncom_GetFacilityString,
     1},  // @pymeth GetFacilityString|Returns the facility string, given an OLE scode.
    {"GetRecordFromGuids", pythoncom_GetRecordFromGuids,
     1},  // @pymeth GetRecordFromGuids|Creates a new record object from the given GUIDs
    {"GetRecordFromTypeInfo", pythoncom_GetRecordFromTypeInfo,
     1},  // @pymeth GetRecordFromTypeInfo|Creates a <o PyRecord> object from a <o PyITypeInfo> interface
    {"GetRunningObjectTable", pythoncom_GetRunningObjectTable,
     1},  // @pymeth GetRunningObjectTable|Obtains a <o PyIRunningObjectTable> object.
    {"GetScodeString", pythoncom_GetScodeString, 1},  // @pymeth GetScodeString|Returns the string for an OLE scode.
    {"GetScodeRangeString", pythoncom_GetScodeRangeString,
     1},  // @pymeth GetScodeRangeString|Returns the scode range string, given an OLE scode.
    {"GetSeverityString", pythoncom_GetSeverityString,
     1},  // @pymeth GetSeverityString|Returns the severity string, given an OLE scode.
    {"IsGatewayRegistered", pythoncom_IsGatewayRegistered,
     1},  // @pymeth IsGatewayRegistered|Returns 1 if the given IID has a registered gateway object.
    {"LoadRegTypeLib", pythoncom_loadregtypelib, 1},  // @pymeth LoadRegTypeLib|Loads a registered type library by CLSID
    {"LoadTypeLib", pythoncom_loadtypelib, 1},        // @pymeth LoadTypeLib|Loads a type library by name
    {"MakeIID", pythoncom_MakeIID, 1},
    {"MakeTime", pythoncom_MakeTime, 1},
    {"MakePyFactory", pythoncom_MakePyFactory,
     1},  // @pymeth MakePyFactory|Creates a new <o PyIClassFactory> object wrapping a PythonCOM Class Factory object.
    {"MkParseDisplayName", pythoncom_MkParseDisplayName,
     1},  // @pymeth MkParseDisplayName|Parses a moniker display name into a moniker object. The inverse of
          // IMoniker::GetDisplayName.
    {"new", pythoncom_new, 1},
    {"New", pythoncom_new, 1},  // @pymeth New|Create a new instance of an OLE automation server.
    {"ObjectFromAddress", pythoncom_ObjectFromAddress,
     1},  // @pymeth ObjectFromAddress|Returns a COM object given its address in memory.
    {"ObjectFromLresult", pythoncom_ObjectFromLresult,
     1},  // @pymeth ObjectFromLresult|Retrieves a requested interface pointer for an object based on a previously
          // generated object reference.
    {"OleInitialize", pythoncom_OleInitialize, 1},  // @pymeth OleInitialize|
    {"OleGetClipboard", pythoncom_OleGetClipboard,
     1},  // @pymeth OleGetClipboard|Retrieves a data object that you can use to access the contents of the clipboard.
    {"OleFlushClipboard", pythoncom_OleFlushClipboard,
     1},  // @pymeth OleFlushClipboard|Carries out the clipboard shutdown sequence. It also releases the IDataObject
          // pointer that was placed on the clipboard by the <om pythoncom.OleSetClipboard> function.
    {"OleIsCurrentClipboard", pythoncom_OleIsCurrentClipboard,
     1},  // @pymeth OleIsCurrentClipboard|Determines whether the data object pointer previously placed on the clipboard
          // by the OleSetClipboard function is still on the clipboard.
    {"OleSetClipboard", pythoncom_OleSetClipboard,
     1},  // @pymeth OleSetClipboard|Places a pointer to a specific data object onto the clipboard. This makes the data
          // object accessible to the OleGetClipboard function.
    {"OleLoadFromStream", pythoncom_OleLoadFromStream, 1},  // @pymeth OleLoadFromStream|Load an object from an IStream.
    {"OleSaveToStream", pythoncom_OleSaveToStream, 1},      // @pymeth OleSaveToStream|Save an object to an IStream.
    {"OleLoad", pythoncom_OleLoad,
     1},  // @pymeth OleLoad|Loads into memory an object nested within a specified storage object.
    {"ProgIDFromCLSID", pythoncom_progidfromclsid, 1},  // @pymeth ProgIDFromCLSID|Converts a CLSID string to a progID.
    {"PumpWaitingMessages", pythoncom_PumpWaitingMessages,
     1},  // @pymeth PumpWaitingMessages|Pumps all waiting messages for the current thread.
    {"PumpMessages", pythoncom_PumpMessages,
     1},  // @pymeth PumpMessages|Pumps all messages for the current thread until a WM_QUIT message.
    {"QueryPathOfRegTypeLib", pythoncom_querypathofregtypelib,
     1},  // @pymeth QueryPathOfRegTypeLib|Retrieves the path of a registered type library
    {"ReadClassStg", pythoncom_ReadClassStg, 1},  // @pymeth ReadClassStg|Reads a CLSID from a storage object
    {"ReadClassStm", pythoncom_ReadClassStm, 1},  // @pymeth ReadClassStm|Reads a CLSID from a <o PyIStream> object
    {"RegisterTypeLib", pythoncom_registertypelib,
     1},  // @pymeth RegisterTypeLib|Adds information about a type library to the system registry.
    {"UnRegisterTypeLib", pythoncom_unregistertypelib,
     1},  // @pymeth UnRegisterTypeLib|Removes a type library from the system registry.
    {"RegisterActiveObject", pythoncom_RegisterActiveObject,
     1},  // @pymeth RegisterActiveObject|Register an object as the active object for its class
    {"RevokeActiveObject", pythoncom_RevokeActiveObject,
     1},  // @pymeth RevokeActiveObject|Ends an object's status as active.
    {"RegisterDragDrop", pythoncom_RegisterDragDrop,
     1},  // @pymeth RegisterDragDrop|Registers the specified window as one that can be the target of an OLE
          // drag-and-drop operation.
    {"RevokeDragDrop", pythoncom_RevokeDragDrop,
     1},  // @pymeth RevokeDragDrop|Revokes the specified window as the target of an OLE drag-and-drop operation.
    {"DoDragDrop", pythoncom_DoDragDrop, 1},  // @pymeth DoDragDrop|Carries out an OLE drag and drop operation.
    {"StgCreateDocfile", pythoncom_StgCreateDocfile,
     1},  // @pymeth StgCreateDocfile|Creates a new compound file storage object using the OLE-provided compound file
          // implementation for the <o PyIStorage> interface.
    {"StgCreateDocfileOnILockBytes", pythoncom_StgCreateDocfileOnILockBytes,
     1},  // @pymeth StgCreateDocfileOnILockBytes|Creates a new compound file storage object using the OLE-provided
          // compound file implementation for the <o PyIStorage> interface.
    {"StgOpenStorageOnILockBytes", pythoncom_StgOpenStorageOnILockBytes,
     1},  // @pymeth StgOpenStorageOnILockBytes|Open an existing storage object that does not reside in a disk file, but
          // instead has an underlying <o PyILockBytes> byte array provided by the caller.
    {"StgIsStorageFile", pythoncom_StgIsStorageFile,
     1},  // @pymeth StgIsStorageFile|Indicates whether a particular disk file contains a storage object.
    {"STGMEDIUM", Py_NewSTGMEDIUM,
     1},  // @pymeth STGMEDIUM|Creates a new <o PySTGMEDIUM> object suitable for the <o PyIDataObject> interface.
    {"StgOpenStorage", pythoncom_StgOpenStorage,
     1},  // @pymeth StgOpenStorage|Opens an existing root storage object in the file system.
    {"StgOpenStorageEx", (PyCFunction)pythoncom_StgOpenStorageEx,
     METH_KEYWORDS |
         METH_VARARGS},  // @pymeth StgOpenStorageEx|Access IStorage and IPropertySetStorage interfaces for normal files
    {"StgCreateStorageEx", (PyCFunction)pythoncom_StgCreateStorageEx,
     METH_KEYWORDS | METH_VARARGS},  // @pymeth StgCreateStorageEx|Creates a new structured storage file or property set
    {"TYPEATTR", Py_NewTYPEATTR, 1},                // @pymeth TYPEATTR|Returns a new <o TYPEATTR> object.
    {"VARDESC", Py_NewVARDESC, 1},                  // @pymeth VARDESC|Returns a new <o VARDESC> object.
    {"WrapObject", pythoncom_WrapObject, 1},        // @pymeth WrapObject|Wraps an object in a gateway.
    {"WriteClassStg", pythoncom_WriteClassStg, 1},  // @pymeth WriteClassStg|Stores a CLSID from a storage object
    {"WriteClassStm", pythoncom_WriteClassStm, 1},  // @pymeth WriteClassStm|Sets the CLSID of a stream
    {"UnwrapObject", pythoncom_UnwrapObject, 1},  // @pymeth UnwrapObject|Unwraps a Python instance in a gateway object.
    {"FmtIdToPropStgName", pythoncom_FmtIdToPropStgName,
     1},  //@pymeth FmtIdToPropStgName|Convert a FMTID to its stream name
    {"PropStgNameToFmtId", pythoncom_PropStgNameToFmtId,
     1},  //@pymeth PropStgNameToFmtId|Convert property set name to FMTID
    {"CoGetCallContext", pythoncom_CoGetCallContext, 1},  // @pymeth CoGetCallContext|Creates interfaces used to access
                                                          // client security settings and perform impersonation
    {"CoGetObjectContext", pythoncom_CoGetObjectContext,
     1},  // @pymeth CoGetObjectContext|Creates an interface to interact with the context of the current object
    {"CoGetCancelObject", pythoncom_CoGetCancelObject,
     1},  // @pymeth CoGetCancelObject|Retrieves an interface used to cancel a pending call
    {"CoSetCancelObject", pythoncom_CoSetCancelObject,
     1},  // @pymeth CoSetCancelObject|Sets or removes a <o PyICancelMethodCalls> interface to be used on the current
          // thread
    {"CoEnableCallCancellation", pythoncom_CoEnableCallCancellation,
     METH_NOARGS},  // @pymeth CoEnableCallCancellation|Enables call cancellation for synchronous calls on the current
                    // thread
    {"CoDisableCallCancellation", pythoncom_CoDisableCallCancellation,
     METH_NOARGS},  // @pymeth CoDisableCallCancellation|Disables call cancellation for synchronous calls on the current
                    // thread
    {NULL, NULL}};

int AddConstant(PyObject *dict, const char *key, long value)
{
    PyObject *oval = PyLong_FromLong(value);
    if (!oval) {
        return 1;
    }
    int rc = PyDict_SetItemString(dict, (char *)key, oval);
    Py_DECREF(oval);
    return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)

static char *modName = "pythoncom";

extern BOOL initunivgw(PyObject *parentDict);

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(pythoncom)
{
    // The DLL Load inited the module.
    // All we do here is init COM itself.  Done here
    // so other clients get a chance to beat us to it!

    // Support a special sys.coinit_flags attribute to control us.
    DWORD coinit_flags = COINIT_APARTMENTTHREADED;

    PyObject *obFlags = PySys_GetObject("coinit_flags");
    // No reference added to obFlags.
    if (obFlags) {
        coinit_flags = PyLong_AsUnsignedLongMask(obFlags);
        if (coinit_flags == -1 && PyErr_Occurred())
            PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    else
        PyErr_Clear();  // Error raised by no coinit_flags attribute.

    HRESULT hr = PyCom_CoInitializeEx(NULL, coinit_flags);
    if (hr == E_NOTIMPL)  // Special return val from PyCom_Co.. indicates not DCOM.
        hr = PyCom_CoInitialize(NULL);
    // If HR fails, we really don't care - the import should work.  User can
    // manually CoInit() to see!

    PYWIN_MODULE_INIT_PREPARE(pythoncom, pythoncom_methods, "A module, encapsulating the OLE automation API");

    // ensure the framework has valid state to work with.
    if (PyCom_RegisterCoreSupport() != 0) {
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }

    // Initialize the dictionary for registering com_record subclasses.
    g_obPyCom_MapRecordGUIDToRecordClass = PyDict_New();
    if (g_obPyCom_MapRecordGUIDToRecordClass == NULL) {
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    PyDict_SetItemString(dict, "RecordClasses", g_obPyCom_MapRecordGUIDToRecordClass);

    // XXX - more error checking?
    PyDict_SetItemString(dict, "TypeIIDs", g_obPyCom_MapIIDToType);
    PyDict_SetItemString(dict, "ServerInterfaces", g_obPyCom_MapGatewayIIDToName);
    PyDict_SetItemString(dict, "InterfaceNames", g_obPyCom_MapInterfaceNameToIID);

    if (PyType_Ready(&PyOleEmptyType) == -1 || PyType_Ready(&PyOleMissingType) == -1 ||
        PyType_Ready(&PyOleArgNotFoundType) == -1 || PyType_Ready(&PyOleNothingType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    g_obEmpty = new PyOleEmpty;
    PyDict_SetItemString(dict, "Empty", g_obEmpty);

    g_obMissing = new PyOleMissing;
    PyDict_SetItemString(dict, "Missing", g_obMissing);

    g_obArgNotFound = new PyOleArgNotFound;
    PyDict_SetItemString(dict, "ArgNotFound", g_obArgNotFound);

    // code changed by ssc
    g_obNothing = new PyOleNothing;
    PyDict_SetItemString(dict, "Nothing", g_obNothing);
    // end code changed by ssc

    // Add some symbolic constants to the module
    // pycom_Error = PyBytes_FromString("pythoncom.error");
    if (PyWinExc_COMError == NULL) {
        // This is created by PyWin_Globals_Ensure
        PyErr_SetString(PyExc_MemoryError, "can't define ole_error");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    PyObject *pycom_Error = PyWinExc_COMError;
    if (PyDict_SetItemString(dict, "ole_error", PyWinExc_COMError) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "error", pycom_Error) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Add the same constant, but with a "new name"
    if (PyDict_SetItemString(dict, "com_error", PyWinExc_COMError) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PyCom_InternalError = PyErr_NewException("pythoncom.internal_error", NULL, NULL);
    if (PyDict_SetItemString(dict, "internal_error", PyCom_InternalError) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Add the IIDs
    if (PyCom_RegisterCoreIIDs(dict) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Initialize various non-interface types
    if (PyType_Ready(&PyFUNCDESC::Type) == -1 || PyType_Ready(&PySTGMEDIUM::Type) == -1 ||
        PyType_Ready(&PyTYPEATTR::Type) == -1 || PyType_Ready(&PyVARDESC::Type) == -1 ||
        PyType_Ready(&PyRecord::Type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Add the PyRecord type as a module attribute
    if (PyModule_AddObject(module, "com_record", (PyObject *)&PyRecord::Type) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Setup our sub-modules
    if (!initunivgw(dict))
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Load function pointers.
    HMODULE hModOle32 = GetModuleHandle(_T("ole32.dll"));
    if (hModOle32) {
        pfnCoWaitForMultipleHandles =
            (CoWaitForMultipleHandlesfunc)GetProcAddress(hModOle32, "CoWaitForMultipleHandles");
        pfnCoGetObjectContext = (CoGetObjectContextfunc)GetProcAddress(hModOle32, "CoGetObjectContext");
        pfnCoGetCancelObject = (CoGetCancelObjectfunc)GetProcAddress(hModOle32, "CoGetCancelObject");
        pfnCoCreateInstanceEx = (CoCreateInstanceExfunc)GetProcAddress(hModOle32, "CoCreateInstanceEx");
        pfnCoInitializeSecurity = (CoInitializeSecurityfunc)GetProcAddress(hModOle32, "CoInitializeSecurity");
    }

    HMODULE hModurlmon = GetModuleHandle(_T("urlmon.dll"));
    if (hModurlmon == NULL)
        hModurlmon = LoadLibrary(_T("urlmon.dll"));
    if (hModurlmon)
        pfnCreateURLMonikerEx = (CreateURLMonikerExfunc)GetProcAddress(hModurlmon, "CreateURLMonikerEx");

    // Symbolic constants.
    ADD_CONSTANT(ACTIVEOBJECT_STRONG);
    ADD_CONSTANT(ACTIVEOBJECT_WEAK);

    ADD_CONSTANT(CLSCTX_ALL);
    ADD_CONSTANT(CLSCTX_INPROC);
    ADD_CONSTANT(CLSCTX_SERVER);

    ADD_CONSTANT(CLSCTX_INPROC_SERVER);
    ADD_CONSTANT(CLSCTX_INPROC_HANDLER);
    ADD_CONSTANT(CLSCTX_LOCAL_SERVER);
    ADD_CONSTANT(CLSCTX_REMOTE_SERVER);

    // COINIT values
    ADD_CONSTANT(COINIT_APARTMENTTHREADED);
#ifdef _WIN32_DCOM
    ADD_CONSTANT(COINIT_MULTITHREADED);
    ADD_CONSTANT(COINIT_DISABLE_OLE1DDE);
    ADD_CONSTANT(COINIT_SPEED_OVER_MEMORY);
#endif
    // CLIPBOARD
    ADD_CONSTANT(DATADIR_GET);
    ADD_CONSTANT(DATADIR_SET);
    ADD_CONSTANT(TYMED_HGLOBAL);
    ADD_CONSTANT(TYMED_FILE);
    ADD_CONSTANT(TYMED_ISTREAM);
    ADD_CONSTANT(TYMED_ISTORAGE);
    ADD_CONSTANT(TYMED_GDI);
    ADD_CONSTANT(TYMED_MFPICT);
    ADD_CONSTANT(TYMED_ENHMF);
    ADD_CONSTANT(TYMED_NULL);
    ADD_CONSTANT(DVASPECT_CONTENT);
    ADD_CONSTANT(DVASPECT_THUMBNAIL);
    ADD_CONSTANT(DVASPECT_ICON);
    ADD_CONSTANT(DVASPECT_DOCPRINT);

    // DISPATCH
    ADD_CONSTANT(DISPATCH_PROPERTYGET);
    ADD_CONSTANT(DISPATCH_PROPERTYPUT);
    ADD_CONSTANT(DISPATCH_PROPERTYPUTREF);
    ADD_CONSTANT(DISPATCH_METHOD);

    // DISPID
    ADD_CONSTANT(DISPID_CONSTRUCTOR);
    ADD_CONSTANT(DISPID_DESTRUCTOR);
    ADD_CONSTANT(DISPID_COLLECT);
    ADD_CONSTANT(DISPID_VALUE);
    ADD_CONSTANT(DISPID_UNKNOWN);
    ADD_CONSTANT(DISPID_PROPERTYPUT);
    ADD_CONSTANT(DISPID_NEWENUM);
    ADD_CONSTANT(DISPID_EVALUATE);
    ADD_CONSTANT(DISPID_STARTENUM);
    ADD_CONSTANT(DISPID_UNKNOWN);
#ifdef DISPID_THIS
    ADD_CONSTANT(DISPID_THIS);
#endif

    // EXTCON
    ADD_CONSTANT(EXTCONN_STRONG);
    ADD_CONSTANT(EXTCONN_WEAK);
    ADD_CONSTANT(EXTCONN_CALLABLE);

    // FUNCFLAGS
    ADD_CONSTANT(FUNCFLAG_FRESTRICTED);
    ADD_CONSTANT(FUNCFLAG_FSOURCE);
    ADD_CONSTANT(FUNCFLAG_FBINDABLE);
    ADD_CONSTANT(FUNCFLAG_FREQUESTEDIT);
    ADD_CONSTANT(FUNCFLAG_FDISPLAYBIND);
    ADD_CONSTANT(FUNCFLAG_FDEFAULTBIND);
    ADD_CONSTANT(FUNCFLAG_FHIDDEN);
    ADD_CONSTANT(FUNCFLAG_FUSESGETLASTERROR);

    // FUNCKIND
    ADD_CONSTANT(FUNC_VIRTUAL);
    ADD_CONSTANT(FUNC_PUREVIRTUAL);
    ADD_CONSTANT(FUNC_NONVIRTUAL);
    ADD_CONSTANT(FUNC_STATIC);
    ADD_CONSTANT(FUNC_DISPATCH);

    // IMPLTYPEFLAGS
    ADD_CONSTANT(IMPLTYPEFLAG_FDEFAULT);
    ADD_CONSTANT(IMPLTYPEFLAG_FSOURCE);
    ADD_CONSTANT(IMPLTYPEFLAG_FRESTRICTED);

    // IDLFLAGS
    ADD_CONSTANT(IDLFLAG_NONE);
    ADD_CONSTANT(IDLFLAG_FIN);
    ADD_CONSTANT(IDLFLAG_FOUT);
    ADD_CONSTANT(IDLFLAG_FLCID);
    ADD_CONSTANT(IDLFLAG_FRETVAL);

    // Moniker types.
    ADD_CONSTANT(MKSYS_NONE);
    ADD_CONSTANT(MKSYS_GENERICCOMPOSITE);
    ADD_CONSTANT(MKSYS_FILEMONIKER);
    ADD_CONSTANT(MKSYS_ANTIMONIKER);
    ADD_CONSTANT(MKSYS_ITEMMONIKER);
    ADD_CONSTANT(MKSYS_POINTERMONIKER);
    ADD_CONSTANT(MKSYS_CLASSMONIKER);

    // PARAMFLAGS
    ADD_CONSTANT(PARAMFLAG_NONE);
    ADD_CONSTANT(PARAMFLAG_FIN);
    ADD_CONSTANT(PARAMFLAG_FOUT);
    ADD_CONSTANT(PARAMFLAG_FLCID);
    ADD_CONSTANT(PARAMFLAG_FRETVAL);
    ADD_CONSTANT(PARAMFLAG_FOPT);
    ADD_CONSTANT(PARAMFLAG_FHASDEFAULT);

    // STREAMSEEK
    ADD_CONSTANT(STREAM_SEEK_SET);
    ADD_CONSTANT(STREAM_SEEK_CUR);
    ADD_CONSTANT(STREAM_SEEK_END);

    // INVOKEKIND
    ADD_CONSTANT(INVOKE_FUNC);
    ADD_CONSTANT(INVOKE_PROPERTYGET);
    ADD_CONSTANT(INVOKE_PROPERTYPUT);
    ADD_CONSTANT(INVOKE_PROPERTYPUTREF);

    ADD_CONSTANT(REGCLS_SINGLEUSE);
    ADD_CONSTANT(REGCLS_MULTIPLEUSE);
    ADD_CONSTANT(REGCLS_MULTI_SEPARATE);
    ADD_CONSTANT(REGCLS_SUSPENDED);

    // ROT
    ADD_CONSTANT(ROTFLAGS_REGISTRATIONKEEPSALIVE);
    ADD_CONSTANT(ROTFLAGS_ALLOWANYCLIENT);

    // RPC
    // Authentication Level used with CoInitializeSecurity
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_DEFAULT);  // RPC_C_AUTHN_LEVEL_DEFAULT|Lets DCOM negotiate the authentication level
                                              // automatically. (Win2k or later)
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_NONE);     // RPC_C_AUTHN_LEVEL_NONE|Performs no authentication.
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_CONNECT);  // RPC_C_AUTHN_LEVEL_CONNECT|Authenticates only when the client
                                              // establishes a relationship with the server. Datagram transports always
                                              // use RPC_AUTHN_LEVEL_PKT instead.
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_CALL);  // RPC_C_AUTHN_LEVEL_CALL|Authenticates only at the beginning of each remote
                                           // procedure call when the server receives the request. Datagram transports
                                           // use RPC_C_AUTHN_LEVEL_PKT instead.
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_PKT);   // RPC_C_AUTHN_LEVEL_PKT|Authenticates that all data received is from the
                                           // expected client.
    ADD_CONSTANT(
        RPC_C_AUTHN_LEVEL_PKT_INTEGRITY);  // RPC_C_AUTHN_LEVEL_PKT_INTEGRITY|Authenticates and verifies that none of
                                           // the data transferred between client and server has been modified.
    ADD_CONSTANT(RPC_C_AUTHN_LEVEL_PKT_PRIVACY);  // RPC_C_AUTHN_LEVEL_PKT_PRIVACY|Authenticates all previous levels and
                                                  // encrypts the argument value of each remote procedure call.

    // Impersonation level used with CoInitializeSecurity
    ADD_CONSTANT(RPC_C_IMP_LEVEL_DEFAULT);  // RPC_C_IMP_LEVEL_DEFAULT|Use default impersonation level (Win2k or later)
    ADD_CONSTANT(
        RPC_C_IMP_LEVEL_ANONYMOUS);  // RPC_C_IMP_LEVEL_ANONYMOUS|(Not supported in this release.) The client is
                                     // anonymous to the server. The server process cannot obtain identification
                                     // information about the client and it cannot impersonate the client.
    ADD_CONSTANT(
        RPC_C_IMP_LEVEL_IDENTIFY);  // RPC_C_IMP_LEVEL_IDENTIFY|The server can obtain the client's identity. The server
                                    // can impersonate the client for ACL checking, but cannot access system objects as
                                    // the client. This information is obtained when the connection is established, not
                                    // on every call.<nl>Note: GetUserName will fail while impersonating at identify
                                    // level. The workaround is to impersonate, OpenThreadToken, revert, call
                                    // GetTokenInformation, and finally, call LookupAccountSid.
    ADD_CONSTANT(
        RPC_C_IMP_LEVEL_IMPERSONATE);  // RPC_C_IMP_LEVEL_IMPERSONATE|The server process can impersonate the client's
                                       // security context while acting on behalf of the client. This information is
                                       // obtained when the connection is established, not on every call.
    ADD_CONSTANT(RPC_C_IMP_LEVEL_DELEGATE);  // RPC_C_IMP_LEVEL_DELEGATE|(Not supported in this release.) The server
                                             // process can impersonate the client's security context while acting on
                                             // behalf of the client. The server process can also make outgoing calls to
                                             // other servers while acting on behalf of the client. This information is
                                             // obtained when the connection is established, not on every call.

    // Authentication service identifiers
    ADD_CONSTANT(RPC_C_AUTHN_NONE);
    ADD_CONSTANT(RPC_C_AUTHN_DCE_PRIVATE);
    ADD_CONSTANT(RPC_C_AUTHN_DCE_PUBLIC);
    ADD_CONSTANT(RPC_C_AUTHN_DEC_PUBLIC);
    ADD_CONSTANT(RPC_C_AUTHN_GSS_NEGOTIATE);
    ADD_CONSTANT(RPC_C_AUTHN_WINNT);
    ADD_CONSTANT(RPC_C_AUTHN_GSS_SCHANNEL);
    ADD_CONSTANT(RPC_C_AUTHN_GSS_KERBEROS);
    ADD_CONSTANT(RPC_C_AUTHN_MSN);
    ADD_CONSTANT(RPC_C_AUTHN_DPA);
    ADD_CONSTANT(RPC_C_AUTHN_MQ);
    ADD_CONSTANT(RPC_C_AUTHN_DEFAULT);

    // Authorization service identifiers
    ADD_CONSTANT(RPC_C_AUTHZ_NONE);
    ADD_CONSTANT(RPC_C_AUTHZ_NAME);
    ADD_CONSTANT(RPC_C_AUTHZ_DCE);
    ADD_CONSTANT(RPC_C_AUTHZ_DEFAULT);

    // Authentication capabilities used with CoInitializeSecurity (EOLE_AUTHENTICATION_CAPABILITIES enum)
    ADD_CONSTANT(EOAC_NONE);
    ADD_CONSTANT(EOAC_MUTUAL_AUTH);
    ADD_CONSTANT(EOAC_SECURE_REFS);
    ADD_CONSTANT(EOAC_ACCESS_CONTROL);
    ADD_CONSTANT(EOAC_APPID);
    ADD_CONSTANT(EOAC_DYNAMIC);
    ADD_CONSTANT(EOAC_STATIC_CLOAKING);
    ADD_CONSTANT(EOAC_DYNAMIC_CLOAKING);
    ADD_CONSTANT(EOAC_ANY_AUTHORITY);
    ADD_CONSTANT(EOAC_MAKE_FULLSIC);
    ADD_CONSTANT(EOAC_REQUIRE_FULLSIC);
    ADD_CONSTANT(EOAC_AUTO_IMPERSONATE);
    ADD_CONSTANT(EOAC_DEFAULT);
    ADD_CONSTANT(EOAC_DISABLE_AAA);
    ADD_CONSTANT(EOAC_NO_CUSTOM_MARSHAL);

    // STDOLE
    ADD_CONSTANT(STDOLE_MAJORVERNUM);
    ADD_CONSTANT(STDOLE_MINORVERNUM);
    ADD_CONSTANT(STDOLE_LCID);
    ADD_CONSTANT(STDOLE2_MAJORVERNUM);
    ADD_CONSTANT(STDOLE2_MINORVERNUM);
    ADD_CONSTANT(STDOLE2_LCID);

    // SYSKIND
    ADD_CONSTANT(SYS_WIN16);
    ADD_CONSTANT(SYS_WIN32);
    ADD_CONSTANT(SYS_MAC);

    // TYPEFLAGS
    ADD_CONSTANT(TYPEFLAG_FAPPOBJECT);
    ADD_CONSTANT(TYPEFLAG_FCANCREATE);
    ADD_CONSTANT(TYPEFLAG_FLICENSED);
    ADD_CONSTANT(TYPEFLAG_FPREDECLID);
    ADD_CONSTANT(TYPEFLAG_FHIDDEN);
    ADD_CONSTANT(TYPEFLAG_FCONTROL);
    ADD_CONSTANT(TYPEFLAG_FDUAL);
    ADD_CONSTANT(TYPEFLAG_FNONEXTENSIBLE);
    ADD_CONSTANT(TYPEFLAG_FOLEAUTOMATION);
    ADD_CONSTANT(TYPEFLAG_FRESTRICTED);
    ADD_CONSTANT(TYPEFLAG_FAGGREGATABLE);
    ADD_CONSTANT(TYPEFLAG_FREPLACEABLE);
    ADD_CONSTANT(TYPEFLAG_FDISPATCHABLE);
    ADD_CONSTANT(TYPEFLAG_FREVERSEBIND);

    // TYPEKIND
    ADD_CONSTANT(TKIND_ENUM);
    ADD_CONSTANT(TKIND_RECORD);
    ADD_CONSTANT(TKIND_MODULE);
    ADD_CONSTANT(TKIND_INTERFACE);
    ADD_CONSTANT(TKIND_DISPATCH);
    ADD_CONSTANT(TKIND_COCLASS);
    ADD_CONSTANT(TKIND_ALIAS);
    ADD_CONSTANT(TKIND_UNION);

    // VARFLAGS
    ADD_CONSTANT(VARFLAG_FREADONLY);

    // VARKIND
    ADD_CONSTANT(VAR_PERINSTANCE);
    ADD_CONSTANT(VAR_STATIC);
    ADD_CONSTANT(VAR_CONST);
    ADD_CONSTANT(VAR_DISPATCH);

    // VARTYPE
    ADD_CONSTANT(VT_EMPTY);
    ADD_CONSTANT(VT_NULL);
    ADD_CONSTANT(VT_I2);
    ADD_CONSTANT(VT_I4);
    ADD_CONSTANT(VT_R4);
    ADD_CONSTANT(VT_R8);
    ADD_CONSTANT(VT_CY);
    ADD_CONSTANT(VT_DATE);
    ADD_CONSTANT(VT_BSTR);
    ADD_CONSTANT(VT_DISPATCH);
    ADD_CONSTANT(VT_ERROR);
    ADD_CONSTANT(VT_BOOL);
    ADD_CONSTANT(VT_VARIANT);
    ADD_CONSTANT(VT_UNKNOWN);
    ADD_CONSTANT(VT_DECIMAL);
    ADD_CONSTANT(VT_I1);
    ADD_CONSTANT(VT_UI1);
    ADD_CONSTANT(VT_UI2);
    ADD_CONSTANT(VT_UI4);
    ADD_CONSTANT(VT_I8);
    ADD_CONSTANT(VT_UI8);
    ADD_CONSTANT(VT_INT);
    ADD_CONSTANT(VT_UINT);
    ADD_CONSTANT(VT_VOID);
    ADD_CONSTANT(VT_HRESULT);
    ADD_CONSTANT(VT_PTR);
    ADD_CONSTANT(VT_SAFEARRAY);
    ADD_CONSTANT(VT_CARRAY);
    ADD_CONSTANT(VT_USERDEFINED);
    ADD_CONSTANT(VT_LPSTR);
    ADD_CONSTANT(VT_LPWSTR);
    ADD_CONSTANT(VT_RECORD);
    ADD_CONSTANT(VT_FILETIME);
    ADD_CONSTANT(VT_BLOB);
    ADD_CONSTANT(VT_STREAM);
    ADD_CONSTANT(VT_STORAGE);
    ADD_CONSTANT(VT_STREAMED_OBJECT);
    ADD_CONSTANT(VT_STORED_OBJECT);
    ADD_CONSTANT(VT_BLOB_OBJECT);
    ADD_CONSTANT(VT_CF);
    ADD_CONSTANT(VT_CLSID);
    ADD_CONSTANT(VT_BSTR_BLOB);

    ADD_CONSTANT(VT_VECTOR);
    ADD_CONSTANT(VT_ARRAY);
    ADD_CONSTANT(VT_BYREF);

    ADD_CONSTANT(VT_RESERVED);
    ADD_CONSTANT(VT_ILLEGAL);
    ADD_CONSTANT(VT_ILLEGALMASKED);
    ADD_CONSTANT(VT_TYPEMASK);

    // DestContext for CoMarshalInterface (MSHCTX enum)
    ADD_CONSTANT(MSHCTX_LOCAL);
    ADD_CONSTANT(MSHCTX_NOSHAREDMEM);
    ADD_CONSTANT(MSHCTX_DIFFERENTMACHINE);
    ADD_CONSTANT(MSHCTX_INPROC);

    // Marshalling flags used with CoMarshalInterface (MSHLFLAGS enum)
    ADD_CONSTANT(MSHLFLAGS_NORMAL);
    ADD_CONSTANT(MSHLFLAGS_TABLESTRONG);
    ADD_CONSTANT(MSHLFLAGS_TABLEWEAK);
    ADD_CONSTANT(MSHLFLAGS_NOPING);

    // Flags for CreateUrlMoniker
    ADD_CONSTANT(URL_MK_UNIFORM);
    ADD_CONSTANT(URL_MK_LEGACY);

    // Flags for CoWaitForMultipleHandles
    ADD_CONSTANT(COWAIT_WAITALL);
    ADD_CONSTANT(COWAIT_ALERTABLE);

    ADD_CONSTANT(fdexNameCaseSensitive);  // Request that the name lookup be done in a case-sensitive manner. May be
                                          // ignored by object that does not support case-sensitive lookup.
    ADD_CONSTANT(fdexNameEnsure);    // Request that the member be created if it does not already exist. The new member
                                     // should be created with the value VT_EMPTY.
    ADD_CONSTANT(fdexNameImplicit);  // Indicates that the caller is searching object(s) for a member of a particular
                                     // name, when the base object is not explicitly specified.
    ADD_CONSTANT(fdexNameCaseInsensitive);  // Request that the name lookup be done in a case-insensitive manner. May be
                                            // ignored by object that does not support case-insensitive lookup.
    ADD_CONSTANT(fdexPropCanGet);           // The member can be obtained using DISPATCH_PROPERTYGET.
    ADD_CONSTANT(fdexPropCannotGet);        // The member cannot be obtained using DISPATCH_PROPERTYGET.
    ADD_CONSTANT(fdexPropCanPut);           // The member can be set using DISPATCH_PROPERTYPUT.
    ADD_CONSTANT(fdexPropCannotPut);        // The member cannot be set using DISPATCH_PROPERTYPUT.
    ADD_CONSTANT(fdexPropCanPutRef);        // The member can be set using DISPATCH_PROPERTYPUTREF.
    ADD_CONSTANT(fdexPropCannotPutRef);     // The member cannot be set using DISPATCH_PROPERTYPUTREF.
    ADD_CONSTANT(
        fdexPropNoSideEffects);  // The member does not have any side effects. For example, a debugger could safely
                                 // get/set/call this member without changing the state of the script being debugged.
    ADD_CONSTANT(fdexPropDynamicType);      // The member is dynamic and can change during the lifetime of the object.
    ADD_CONSTANT(fdexPropCanCall);          // The member can be called as a method using DISPATCH_METHOD.
    ADD_CONSTANT(fdexPropCannotCall);       // The member cannot be called as a method using DISPATCH_METHOD.
    ADD_CONSTANT(fdexPropCanConstruct);     // The member can be called as a constructor using DISPATCH_CONSTRUCT.
    ADD_CONSTANT(fdexPropCannotConstruct);  // The member cannot be called as a constructor using DISPATCH_CONSTRUCT.
    ADD_CONSTANT(fdexPropCanSourceEvents);  // The member can fire events.
    ADD_CONSTANT(fdexPropCannotSourceEvents);  // The member cannot fire events.

    ADD_CONSTANT(DESCKIND_FUNCDESC);
    ADD_CONSTANT(DESCKIND_VARDESC);
    // Expose the frozen flag, as Python itself doesn't!!
    // @prop int|frozen|1 if the host is a frozen program, else 0
    AddConstant(dict, "frozen", Py_FrozenFlag);

    // And finally some gross hacks relating to DCOM
    // I'm really not sure what a better option is!
    //
    // If these #error pragma's fire it means this needs revisiting for
    // an upgrade to the MSVC header files!
    if (PyCom_HasDCom()) {
#if ((CLSCTX_ALL != (CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER)) || \
     (CLSCTX_SERVER != (CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER)))
#error DCOM constants are not in synch.
#endif
        ADD_CONSTANT(CLSCTX_ALL);
        ADD_CONSTANT(CLSCTX_SERVER);
        AddConstant(dict, "dcom", 1);
    }
    else {
        AddConstant(dict, "CLSCTX_ALL", CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
        AddConstant(dict, "CLSCTX_SERVER", CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER);
        AddConstant(dict, "dcom", 0);
    }

    PyObject *obfmtid = NULL;
    obfmtid = PyWinObject_FromIID(FMTID_DocSummaryInformation);
    PyDict_SetItemString(dict, "FMTID_DocSummaryInformation", obfmtid);
    Py_DECREF(obfmtid);
    obfmtid = PyWinObject_FromIID(FMTID_SummaryInformation);
    PyDict_SetItemString(dict, "FMTID_SummaryInformation", obfmtid);
    Py_DECREF(obfmtid);
    obfmtid = PyWinObject_FromIID(FMTID_UserDefinedProperties);
    PyDict_SetItemString(dict, "FMTID_UserDefinedProperties", obfmtid);
    Py_DECREF(obfmtid);
    // obfmtid=PyWinObject_FromIID(FMTID_MediaFileSummaryInfo);
    // PyDict_SetItemString(dict,"FMTID_MediaFileSummaryInfo",obfmtid);
    // Py_DECREF(obfmtid);

    // @prop int|dcom|1 if the system is DCOM aware, else 0.  Only Win95 without DCOM extensions should return 0

    // ### ALL THE @PROPERTY TAGS MUST COME AFTER THE LAST @PROP TAG!!
    // @property int|pythoncom|frozen|1 if the host is a frozen program, else 0
    // @property int|pythoncom|dcom|1 if the system is DCOM aware, else 0.  Only Win95 without DCOM extensions should
    // return 0

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
