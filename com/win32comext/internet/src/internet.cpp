// AXControl.cpp :
// $Id$

// Interfaces that support the Internet COM interfaces

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "internet_pch.h"
#include "MsHtmHst.h"
#include "stddef.h"             // for offsetof
#include "PythonCOMRegister.h"  // For simpler registration of IIDs etc.

#include "PyIDocHostUIHandler.h"
#include "PyIHTMLOMWindowServices.h"
#include "PyIInternetProtocolRoot.h"
#include "PyIInternetProtocol.h"
#include "PyIInternetProtocolInfo.h"
#include "PyIInternetProtocolSink.h"
#include "PyIInternetPriority.h"
#include "PyIInternetBindInfo.h"
#include "PyIInternetSecurityManager.h"

// Check a function pointer that is supplied by a specific IE version (ie,
// we require an IE version later than what is installed)
#define CHECK_IE_PFN(fname)                                                                                       \
    if (pfn##fname == NULL)                                                                                       \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available with this Internet Explorer version", \
                            #fname);

typedef HRESULT(WINAPI *CoInternetSetFeatureEnabled_func)(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags,
                                                          BOOL fEnable);
static CoInternetSetFeatureEnabled_func pfnCoInternetSetFeatureEnabled = NULL;

typedef HRESULT(WINAPI *CoInternetIsFeatureEnabled_func)(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags);
static CoInternetIsFeatureEnabled_func pfnCoInternetIsFeatureEnabled = NULL;

typedef HRESULT(WINAPI *CoInternetCreateSecurityManager_func)(IServiceProvider *pSP, IInternetSecurityManager **ppSM,
                                                              DWORD dwReserved);
static CoInternetCreateSecurityManager_func pfnCoInternetCreateSecurityManager = NULL;

// STDAPI CoInternetCreateZoneManager(IServiceProvider *pSP, IInternetZoneManager **ppZM, DWORD dwReserved);

HMODULE loadmodule(TCHAR *dllname)
{
    HMODULE hmodule = GetModuleHandle(dllname);
    if (hmodule == NULL)
        hmodule = LoadLibrary(dllname);
    return hmodule;
}

FARPROC loadapifunc(char *funcname, HMODULE hmodule)
{
    if (hmodule == NULL)
        return NULL;
    return GetProcAddress(hmodule, funcname);
}

//////////////////////////////////////////////////////////////
//
// PROTOCOLDATA support
//
BOOL PyObject_AsPROTOCOLDATA(PyObject *ob, PROTOCOLDATA *pPD)
{
    if (ob != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Only None is support for PROTOCOLDATA objects");
        return FALSE;
    }
    pPD->grfFlags = 0;
    pPD->dwState = 0;
    pPD->pData = NULL;
    pPD->cbData = 0;
    return TRUE;
}

PyObject *PyObject_FromPROTOCOLDATA(PROTOCOLDATA *pPD)
{
    return Py_BuildValue("iiz#", pPD->grfFlags, pPD->dwState, pPD->pData, pPD->cbData);
}
//////////////////////////////////////////////////////////////
//
// BINDINFO support
//
BOOL PyObject_AsBINDINFO(PyObject *ob, BINDINFO *pPD)
{
    BOOL ok = FALSE;
    memset(pPD, 0, sizeof(BINDINFO));
    pPD->cbSize = sizeof(BINDINFO);
    PyObject *obExtra = Py_None;
    PyObject *obSTGM = Py_None;
    PyObject *obCustomVerb = Py_None;
    PyObject *obSA = Py_None;
    PyObject *obIID = Py_None;
    PyObject *obUnk = Py_None;
    if (!PyArg_ParseTuple(ob, "|OOllOlllOOOl", &obExtra, &obSTGM, &pPD->grfBindInfoF, &pPD->dwBindVerb, &obCustomVerb,
                          &pPD->dwOptions, &pPD->dwOptionsFlags, &pPD->dwCodePage, &obSA, &obIID, &obUnk,
                          &pPD->dwReserved))
        goto done;
    if (!PyWinObject_AsTaskAllocatedWCHAR(obExtra, &pPD->szExtraInfo, /*bNoneOK=*/TRUE, NULL))
        goto done;
    if (obSTGM != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Sorry - dont support STGMEDIUM yet - must be None");
        goto done;
    }
    if (!PyWinObject_AsTaskAllocatedWCHAR(obCustomVerb, &pPD->szCustomVerb, /*bNoneOK=*/TRUE, NULL))
        goto done;
    SECURITY_ATTRIBUTES *pSA;
    if (!PyWinObject_AsSECURITY_ATTRIBUTES(obSA, &pSA, TRUE))
        goto done;
    pPD->securityAttributes = *pSA;
    if (obIID != Py_None && !PyWinObject_AsIID(obIID, &pPD->iid))
        goto done;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, pPD->iid, (void **)&pPD->pUnk, TRUE))
        goto done;

    ok = TRUE;
done:
    // todo: cleanup if !ok
    return ok;
}

PyObject *PyObject_FromBINDINFO(BINDINFO *pPD)
{
    BOOL bNewFormat = pPD->cbSize >= offsetof(BINDINFO, dwOptions);
    int tupleSize = bNewFormat ? 12 : 5;
    PyObject *obRet = PyTuple_New(tupleSize);
    PyTuple_SET_ITEM(obRet, 0, PyWinObject_FromWCHAR(pPD->szExtraInfo));
    Py_INCREF(Py_None);
    PyTuple_SET_ITEM(obRet, 1, Py_None);  // STGMEDUIM not yet supported.
    PyTuple_SET_ITEM(obRet, 2, PyInt_FromLong(pPD->grfBindInfoF));
    PyTuple_SET_ITEM(obRet, 3, PyInt_FromLong(pPD->dwBindVerb));
    PyTuple_SET_ITEM(obRet, 4, PyWinObject_FromWCHAR(pPD->szCustomVerb));
    if (bNewFormat) {
        PyTuple_SET_ITEM(obRet, 5, PyInt_FromLong(pPD->dwOptions));
        PyTuple_SET_ITEM(obRet, 6, PyInt_FromLong(pPD->dwOptionsFlags));
        PyTuple_SET_ITEM(obRet, 7, PyInt_FromLong(pPD->dwCodePage));
        PyTuple_SET_ITEM(obRet, 8, PyWinObject_FromSECURITY_ATTRIBUTES(pPD->securityAttributes));
        PyTuple_SET_ITEM(obRet, 9, PyWinObject_FromIID(pPD->iid));
        PyTuple_SET_ITEM(obRet, 10, PyCom_PyObjectFromIUnknown(pPD->pUnk, pPD->iid, /*bAddRef = */ TRUE));
        PyTuple_SET_ITEM(obRet, 11, PyInt_FromLong(pPD->dwReserved));
    }
    return obRet;
}

//////////////////////////////////////////////////////////////
//
// The methods
//
// @pymethod bool|internet|CoInternetIsFeatureEnabled|
// @rdesc Returns true for S_OK, False for other non-error hresults, or
// raises a com_error.
static PyObject *PyCoInternetIsFeatureEnabled(PyObject *self, PyObject *args)
{
    CHECK_IE_PFN(CoInternetIsFeatureEnabled);
    int featureEntry, flags;
    if (!PyArg_ParseTuple(args, "ii",
                          &featureEntry,  // &pyparm int|featureEntry||
                          &flags))        // @pyparm int|flags||
        return NULL;
    HRESULT hr = (*pfnCoInternetIsFeatureEnabled)((INTERNETFEATURELIST)featureEntry, flags);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    PyObject *rc = (hr == S_OK) ? Py_True : Py_False;
    Py_INCREF(rc);
    return rc;
}

// @pymethod int|internet|CoInternetSetFeatureEnabled|
static PyObject *PyCoInternetSetFeatureEnabled(PyObject *self, PyObject *args)
{
    CHECK_IE_PFN(CoInternetSetFeatureEnabled);
    int featureEntry, flags, enable;
    if (!PyArg_ParseTuple(args, "iii",
                          &featureEntry,  // &pyparm int|featureEntry||
                          &flags,         // @pyparm int|flags||
                          &enable))       // @pyparm bool|enable||
        return NULL;
    HRESULT hr = (*pfnCoInternetSetFeatureEnabled)((INTERNETFEATURELIST)featureEntry, flags, enable);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyInt_FromLong(hr);
}

// @pymethod <o PyIInternetSecurityManager>|internet|CoInternetCreateSecurityManager|
static PyObject *PyCoInternetCreateSecurityManager(PyObject *self, PyObject *args)
{
    CHECK_IE_PFN(CoInternetCreateSecurityManager);
    PyObject *obprov;
    DWORD reserved;
    if (!PyArg_ParseTuple(args, "Oi",
                          &obprov,     // &pyparm <o PyIServiceProvider>|serviceProvider||
                          &reserved))  // @pyparm int|reserved||
        return NULL;
    IServiceProvider *prov;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obprov, IID_IServiceProvider, (void **)&prov, TRUE /* bNoneOK */))
        return NULL;
    HRESULT hr;
    IInternetSecurityManager *sm = 0;
    PY_INTERFACE_PRECALL;
    hr = (*pfnCoInternetCreateSecurityManager)(prov, &sm, reserved);
    prov->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(sm, IID_IInternetSecurityManager, FALSE);
}

/* List of module functions */
// @module internet|A module, encapsulating the ActiveX Internet interfaces
static struct PyMethodDef internet_functions[] = {
    {"CoInternetCreateSecurityManager", PyCoInternetCreateSecurityManager},  // @pymeth CoInternetCreateSecurityManager|
    {"CoInternetIsFeatureEnabled", PyCoInternetIsFeatureEnabled},            // @pymeth CoInternetIsFeatureEnabled|
    {"CoInternetSetFeatureEnabled", PyCoInternetSetFeatureEnabled},          // @pymeth CoInternetSetFeatureEnabled|
    {NULL, NULL},
};

static int AddConstant(PyObject *dict, const char *key, long value)
{
    PyObject *oval = PyInt_FromLong(value);
    if (!oval) {
        return 1;
    }
    int rc = PyDict_SetItemString(dict, (char *)key, oval);
    Py_DECREF(oval);
    return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)

static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] = {
    PYCOM_INTERFACE_FULL(DocHostUIHandler),        PYCOM_INTERFACE_SERVER_ONLY(HTMLOMWindowServices),
    PYCOM_INTERFACE_FULL(InternetProtocolRoot),    PYCOM_INTERFACE_FULL(InternetProtocol),
    PYCOM_INTERFACE_FULL(InternetProtocolInfo),    PYCOM_INTERFACE_FULL(InternetProtocolSink),
    PYCOM_INTERFACE_FULL(InternetPriority),        PYCOM_INTERFACE_FULL(InternetBindInfo),
    PYCOM_INTERFACE_FULL(InternetSecurityManager),
};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(internet)
{
    PYWIN_MODULE_INIT_PREPARE(internet, internet_functions, "A module, encapsulating the ActiveX Internet interfaces");

    // Register all of our interfaces, gateways and IIDs.
    PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData,
                                   sizeof(g_interfaceSupportData) / sizeof(PyCom_InterfaceSupportInfo));

    // load up our function pointers for stuff we can't rely on being
    // there at runtime
    HMODULE urlmon_dll = loadmodule(_T("urlmon.dll"));
    pfnCoInternetSetFeatureEnabled =
        (CoInternetSetFeatureEnabled_func)loadapifunc("CoInternetSetFeatureEnabled", urlmon_dll);
    pfnCoInternetIsFeatureEnabled =
        (CoInternetIsFeatureEnabled_func)loadapifunc("CoInternetIsFeatureEnabled", urlmon_dll);
    pfnCoInternetCreateSecurityManager =
        (CoInternetCreateSecurityManager_func)loadapifunc("CoInternetCreateSecurityManager", urlmon_dll);

    ADD_CONSTANT(FEATURE_OBJECT_CACHING);                  // @const internet|FEATURE_OBJECT_CACHING|
    ADD_CONSTANT(FEATURE_ZONE_ELEVATION);                  // @const internet|FEATURE_ZONE_ELEVATION|
    ADD_CONSTANT(FEATURE_MIME_HANDLING);                   // @const internet|FEATURE_MIME_HANDLING|
    ADD_CONSTANT(FEATURE_MIME_SNIFFING);                   // @const internet|FEATURE_MIME_SNIFFING|
    ADD_CONSTANT(FEATURE_WINDOW_RESTRICTIONS);             // @const internet|FEATURE_WINDOW_RESTRICTIONS|
    ADD_CONSTANT(FEATURE_WEBOC_POPUPMANAGEMENT);           // @const internet|FEATURE_WEBOC_POPUPMANAGEMENT|
    ADD_CONSTANT(FEATURE_BEHAVIORS);                       // @const internet|FEATURE_BEHAVIORS|
    ADD_CONSTANT(FEATURE_DISABLE_MK_PROTOCOL);             // @const internet|FEATURE_DISABLE_MK_PROTOCOL|
    ADD_CONSTANT(FEATURE_LOCALMACHINE_LOCKDOWN);           // @const internet|FEATURE_LOCALMACHINE_LOCKDOWN|
    ADD_CONSTANT(FEATURE_SECURITYBAND);                    // @const internet|FEATURE_SECURITYBAND|
    ADD_CONSTANT(FEATURE_RESTRICT_ACTIVEXINSTALL);         // @const internet|FEATURE_RESTRICT_ACTIVEXINSTALL|
    ADD_CONSTANT(FEATURE_VALIDATE_NAVIGATE_URL);           // @const internet|FEATURE_VALIDATE_NAVIGATE_URL|
    ADD_CONSTANT(FEATURE_RESTRICT_FILEDOWNLOAD);           // @const internet|FEATURE_RESTRICT_FILEDOWNLOAD|
    ADD_CONSTANT(FEATURE_ADDON_MANAGEMENT);                // @const internet|FEATURE_ADDON_MANAGEMENT|
    ADD_CONSTANT(FEATURE_PROTOCOL_LOCKDOWN);               // @const internet|FEATURE_PROTOCOL_LOCKDOWN|
    ADD_CONSTANT(FEATURE_HTTP_USERNAME_PASSWORD_DISABLE);  // @const internet|FEATURE_HTTP_USERNAME_PASSWORD_DISABLE|
    ADD_CONSTANT(FEATURE_SAFE_BINDTOOBJECT);               // @const internet|FEATURE_SAFE_BINDTOOBJECT|
    ADD_CONSTANT(FEATURE_UNC_SAVEDFILECHECK);              // @const internet|FEATURE_UNC_SAVEDFILECHECK|
    ADD_CONSTANT(FEATURE_GET_URL_DOM_FILEPATH_UNENCODED);  // @const internet|FEATURE_GET_URL_DOM_FILEPATH_UNENCODED|
    ADD_CONSTANT(FEATURE_ENTRY_COUNT);                     // @const internet|FEATURE_ENTRY_COUNT|

    ADD_CONSTANT(SET_FEATURE_ON_THREAD);                 // @const internet|SET_FEATURE_ON_THREAD|
    ADD_CONSTANT(SET_FEATURE_ON_PROCESS);                // @const internet|SET_FEATURE_ON_PROCESS|
    ADD_CONSTANT(SET_FEATURE_IN_REGISTRY);               // @const internet|SET_FEATURE_IN_REGISTRY|
    ADD_CONSTANT(SET_FEATURE_ON_THREAD_LOCALMACHINE);    // @const internet|SET_FEATURE_ON_THREAD_LOCALMACHINE|
    ADD_CONSTANT(SET_FEATURE_ON_THREAD_INTRANET);        // @const internet|SET_FEATURE_ON_THREAD_INTRANET|
    ADD_CONSTANT(SET_FEATURE_ON_THREAD_TRUSTED);         // @const internet|SET_FEATURE_ON_THREAD_TRUSTED|
    ADD_CONSTANT(SET_FEATURE_ON_THREAD_INTERNET);        // @const internet|SET_FEATURE_ON_THREAD_INTERNET|
    ADD_CONSTANT(SET_FEATURE_ON_THREAD_RESTRICTED);      // @const internet|SET_FEATURE_ON_THREAD_RESTRICTED|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD);               // @const internet|GET_FEATURE_FROM_THREAD|
    ADD_CONSTANT(GET_FEATURE_FROM_PROCESS);              // @const internet|GET_FEATURE_FROM_PROCESS|
    ADD_CONSTANT(GET_FEATURE_FROM_REGISTRY);             // @const internet|GET_FEATURE_FROM_REGISTRY|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD_LOCALMACHINE);  // @const internet|GET_FEATURE_FROM_THREAD_LOCALMACHINE|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD_INTRANET);      // @const internet|GET_FEATURE_FROM_THREAD_INTRANET|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD_TRUSTED);       // @const internet|GET_FEATURE_FROM_THREAD_TRUSTED|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD_INTERNET);      // @const internet|GET_FEATURE_FROM_THREAD_INTERNET|
    ADD_CONSTANT(GET_FEATURE_FROM_THREAD_RESTRICTED);    // @const internet|GET_FEATURE_FROM_THREAD_RESTRICTED|

    //	ADD_CONSTANT(); // @const internet||

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
