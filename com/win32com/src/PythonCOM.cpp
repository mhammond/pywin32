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
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyFactory.h"

extern int PyCom_RegisterCoreIIDs(PyObject *dict);
extern PyObject  *pythoncom_IsGatewayRegistered(PyObject *self, PyObject *args);

extern PyObject *g_obPyCom_MapIIDToType;
extern PyObject *g_obPyCom_MapGatewayIIDToName;
extern PyObject *g_obPyCom_MapInterfaceNameToIID;

static PyObject *g_obEmpty = NULL;
static PyObject *g_obMissing = NULL;

PyObject *PyCom_InternalError = NULL;

// Storage related functions.
extern PyObject *pythoncom_StgOpenStorage(PyObject *self, PyObject *args);
#ifndef MS_WINCE
extern PyObject *pythoncom_StgIsStorageFile(PyObject *self, PyObject *args);
#endif // MS_WINCE
extern PyObject *pythoncom_StgCreateDocfile(PyObject *self, PyObject *args);
extern PyObject *pythoncom_StgCreateDocfileOnILockBytes(PyObject *self, PyObject *args);
extern PyObject *pythoncom_WriteClassStg(PyObject *self, PyObject *args);
extern PyObject *pythoncom_ReadClassStg(PyObject *self, PyObject *args);
extern PyObject *pythoncom_GetRecordFromGuids(PyObject *self, PyObject *args);


// Typelib related functions
extern PyObject *pythoncom_loadtypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_loadregtypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_registertypelib(PyObject *self, PyObject *args);
extern PyObject *pythoncom_unregistertypelib(PyObject *self, PyObject *args);
#ifndef MS_WINCE
extern PyObject *pythoncom_querypathofregtypelib(PyObject *self, PyObject *args);
#endif // MS_WINCE

// Type object helpers
PyObject *Py_NewFUNCDESC(PyObject *self, PyObject *args);
PyObject *Py_NewTYPEATTR(PyObject *self, PyObject *args);
PyObject *Py_NewVARDESC(PyObject *self, PyObject *args);

// Error related functions
void GetScodeString(SCODE sc, TCHAR *buf, int bufSize);
LPCSTR GetScodeRangeString(SCODE sc);
LPCSTR GetSeverityString(SCODE sc);
LPCSTR GetFacilityString(SCODE sc);

/* Debug/Test helpers */
extern LONG _PyCom_GetInterfaceCount(void);
extern LONG _PyCom_GetGatewayCount(void);


BOOL PyCom_HasDCom()
{
#ifndef MS_WINCE
	static BOOL bHaveDCOM = -1;
	if (bHaveDCOM==-1) {
		HMODULE hMod = GetModuleHandle("ole32.dll");
		if (hMod) {
			FARPROC fp = GetProcAddress(hMod, "CoInitializeEx");
			bHaveDCOM = (fp!=NULL);
		} else
			bHaveDCOM = FALSE; // not much we can do!
	}
	return bHaveDCOM;
#else // no DCOM on WinCE.
	return FALSE;
#endif
}

#pragma optimize ("y", off)
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
		&obCLSID,	// @pyparm <o PyIID>|clsid||Class identifier (CLSID) of the object
		&obUnk,	// @pyparm <o PyIUnknown>|unkOuter||The outer unknown, or None
		&dwClsContext,// @pyparm int|context||The create context for the object
		&obiid)) // @pyparm <o PyIID>|iid||The IID required from the object
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
	if ( punk )
		punk->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc);
	return PyCom_PyObjectFromIUnknown(result, iid);
}
#pragma optimize ("", on)

#ifndef MS_WINCE
#pragma optimize ("", off)
// @pymethod <o PyIUnknown>|pythoncom|CoCreateInstanceEx|Create a new instance of an OLE automation server possibly on a remote machine.
static PyObject *pythoncom_CoCreateInstanceEx(PyObject *self, PyObject *args)
{
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
	int numIIDs = 0;
	int i;
	if (!PyArg_ParseTuple(args, "OOiOO:CoCreateInstanceEx",
		&obCLSID, // @pyparm <o PyIID>|clsid||Class identifier (CLSID) of the object
		&obUnk, // @pyparm <o PyIUnknown>|unkOuter||The outer unknown, or None
		&dwClsContext, // @pyparm int|context||The create context for the object
		&obCoServer, // @pyparm (server, authino=None, reserved1=0,reserved2=0)|serverInfo||May be None, or describes the remote server to execute on.
		&obrgiids)) // @pyparm [<o PyIID>, ...]|iids||A list of IIDs required from the object
		return NULL;
	if (!PyWinObject_AsIID(obCLSID, &clsid))
		goto done;

	if (obCoServer==Py_None)
		pServerInfo = NULL;
	else {
		pServerInfo = &serverInfo;
		PyObject *obName, *obAuth = Py_None;
		if (!PyArg_ParseTuple(obCoServer, "O|Oii", &obName, &obAuth, &serverInfo.dwReserved1, &serverInfo.dwReserved2)) {
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "The SERVERINFO is not in the correct format");
			goto done;
		}
		if (obAuth!=Py_None) {
			PyErr_SetString(PyExc_TypeError, "authinfo in the SERVERINFO must be None");
			goto done;
		}
		if (!PyWinObject_AsWCHAR(obName, &serverInfo.pwszName, FALSE))
			goto done;
	}
	if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&punk, TRUE))
		goto done;

	if (!PySequence_Check(obrgiids)) {
		PyErr_SetString(PyExc_TypeError, "IID's must be sequence of IID objects");
		goto done;
	}
	numIIDs = PySequence_Length(obrgiids);

	iids = new IID[numIIDs];
	mqi = new MULTI_QI[numIIDs];
	if (iids==NULL || mqi==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating MULTIQI array");
		goto done;
	}

	for (i=0;i<numIIDs;i++) {
		PyObject *me = PySequence_GetItem(obrgiids, i);
		if (me==NULL) goto done;
		BOOL ok = PyWinObject_AsIID(me, iids+i);
		Py_DECREF(me);
		if (!ok) goto done;
		mqi[i].pIID = iids+i;
		mqi[i].pItf = NULL;
		mqi[i].hr = 0;
	}
	// Jump hoops in case the platform doesnt have it.
	{ // scoping
	HRESULT (*mypfn)(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, ULONG, MULTI_QI *);
	HMODULE hMod = GetModuleHandle("ole32.dll");
	if (hMod==0) {
		PyCom_BuildInternalPyException("Can not load ole32.dll");
		goto done;
	}
	FARPROC fp = GetProcAddress(hMod, "CoCreateInstanceEx");
	if (fp==NULL) {
		PyCom_BuildPyException(E_NOTIMPL);
		goto done;
	}
	mypfn = (HRESULT (*)(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, ULONG, MULTI_QI *))fp;
	PY_INTERFACE_PRECALL;
	HRESULT hr = (*mypfn)(clsid, punk, dwClsContext, pServerInfo, numIIDs, mqi);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	} // end scoping.
	result = PyTuple_New(numIIDs);
	if (result==NULL) goto done;
	for (i=0;i<numIIDs;i++) {
		PyObject *obNew;
		if (mqi[i].hr==0)
			obNew = PyCom_PyObjectFromIUnknown(mqi[i].pItf, *mqi[i].pIID, FALSE);
		else {
			obNew = Py_None;
			Py_INCREF(Py_None);
		}
		PyTuple_SET_ITEM(result, i, obNew);
	}
done:
	if (punk) {
		PY_INTERFACE_PRECALL;
		punk->Release();
		PY_INTERFACE_POSTCALL;
	}
	if (serverInfo.pwszName)
		PyWinObject_FreeWCHAR(serverInfo.pwszName);

	delete [] iids;
	delete [] mqi;
	return result;
}
#pragma optimize ("", on)

// @pymethod |pythoncom|CoInitializeSecurity|Registers security and sets the default security values. 
static PyObject *pythoncom_CoInitializeSecurity(PyObject *self, PyObject *args)
{
	DWORD cAuthSvc;
	SOLE_AUTHENTICATION_SERVICE *pAS = NULL;
	DWORD dwAuthnLevel;
	DWORD dwImpLevel;
	DWORD dwCapabilities;
	SECURITY_DESCRIPTOR *pSD;
	PyObject *obSD, *obAuthSvc, *obReserved1, *obReserved2, *obAuthInfo;
	if (!PyArg_ParseTuple(args, "OOOiiOiO:CoInitializeSecurity",
		&obSD, // @pyparm <o PySECURITY_DESCRIPTOR>|sd||
		&obAuthSvc, // @pyparm object|authInfo||A value of None tells COM to choose which authentication services to use.  An empty list means use no services.
		&obReserved1,// @pyparm object|reserved1||Must be None
		&dwAuthnLevel, // @pyparm int|authnLevel||The default authentication level for proxies. On the server side, COM will fail calls that arrive at a lower level. All calls to AddRef and Release are made at this level.
		&dwImpLevel, // @pyparm int|impLevel||The default impersonation level for proxies. This value is not checked on the server side. AddRef and Release calls are made with this impersonation level so even security aware apps should set this carefully. Setting IUnknown security only affects calls to QueryInterface, not AddRef or Release. 
		&obAuthInfo, // @pyparm object|authInfo||Must be None
		&dwCapabilities, // @pyparm int|capabilities||Additional client and/or server-side capabilities. Any set of EOAC flags may be passed. Currently only EOAC_MUTUAL_AUTH, EOAC_SECURE_REFS, and EOAC_NONE are defined
		&obReserved2)) // @pyparm object|reserved2||Must be None
		return NULL;
	if (obReserved1 != Py_None || obReserved2 != Py_None || obAuthInfo != Py_None) {
		PyErr_SetString(PyExc_TypeError, "Not all of the 'None' arguments are None!");
		return NULL;
	}
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obSD, &pSD, /*BOOL bNoneOK = */TRUE))
		return NULL;

	if (obAuthSvc==NULL)
		cAuthSvc = -1;
	else if (PySequence_Check(obAuthSvc)) {
		cAuthSvc = 0;
	} else {
		PyErr_SetString(PyExc_TypeError, "obAuthSvc must be None or an empty sequence.");
		return NULL;
	}
	HMODULE hMod = GetModuleHandle("ole32.dll");
	if (hMod==0) return PyCom_BuildInternalPyException("Can not load ole32.dll");
	FARPROC fp = GetProcAddress(hMod, "CoInitializeSecurity");
	if (fp==NULL) return PyCom_BuildPyException(E_NOTIMPL);

	HRESULT (*mypfn)(SECURITY_DESCRIPTOR*, LONG, SOLE_AUTHENTICATION_SERVICE*, void *, DWORD, DWORD, void *, DWORD, void *);
	mypfn = (HRESULT (*)(SECURITY_DESCRIPTOR*, LONG, SOLE_AUTHENTICATION_SERVICE*, void *, DWORD, DWORD, void *, DWORD, void *))fp;

	PY_INTERFACE_PRECALL;
	HRESULT hr = (*mypfn)(pSD, cAuthSvc, pAS, NULL, dwAuthnLevel, dwImpLevel, NULL, dwCapabilities, NULL);
//	HRESULT hr = CoInitializeSecurity(pSD, cAuthSvc, pAS, NULL, dwAuthnLevel, dwImpLevel, NULL, dwCapabilities, NULL);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

#pragma optimize ("y", off)
// @pymethod int|pythoncom|CoRegisterClassObject|Registers an EXE class object with OLE so other applications can connect to it.
static PyObject *pythoncom_CoRegisterClassObject(PyObject *self, PyObject *args)
{
	DWORD reg;
	DWORD context;
	DWORD flags;
	PyObject *obIID, *obFactory;
	IID iid;

	if (!PyArg_ParseTuple(args, "OOii:CoRegisterClassObject",
		&obIID,	// @pyparm <o PyIID>|iid||The IID of the object to register
		&obFactory,	// @pyparm <o PyIUnknown>|factory||The class factory object.  It is the Python programmers responsibility to ensure this object remains alive until the class is unregistered.
		&context, // @pyparm int|context||The create context for the server.  Must be a combination of the CLSCTX_* flags.
		&flags))  // @pyparm int|flags||Create flags.
		return NULL;
	// @comm The class factory object should be <o PyIClassFactory> object, but as per the COM documentation, only <o PyIUnknown> is checked.
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
	return PyInt_FromLong(reg);
}
// @pymethod |pythoncom|CoRevokeClassObject|Informs OLE that a class object, previously registered with the <om pythoncom.CoRegisterClassObject> method, is no longer available for use. 
static PyObject *pythoncom_CoRevokeClassObject(PyObject *self, PyObject *args)
{
	DWORD reg;

	if (!PyArg_ParseTuple(args, "i:CoRevokeClassObject",
		&reg))	// @pyparm int|reg||The value returned from <om pythoncom.CoRegisterClassObject>
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

#pragma optimize ("", on)

// I cont understand this compiler :-(
#pragma optimize ("", off)
// @pymethod |pythoncom|CoResumeClassObjects|Called by a server that can register multiple class objects to inform the OLE SCM about all registered classes, and permits activation requests for those class objects.
static PyObject *pythoncom_CoResumeClassObjects(PyObject *self, PyObject *args)
{
	// @comm This is not available on Window95.  If it is called on that platform,
	// a E_NOTIMPLEMENTED <o com_error> is raised.
	if (!PyArg_ParseTuple(args, ":CoResumeClassObjects"))
		return NULL;
	// *** AARG - NOT ON 95!
	HMODULE hMod = GetModuleHandle("ole32.dll");
	if (hMod==0) return PyWin_SetAPIError("GetModuleHandle(\"ole32.dll\")");
	FARPROC fp = GetProcAddress(hMod, "CoResumeClassObjects");
	if (fp==NULL)
		return PyCom_BuildPyException(E_NOTIMPL);
	PY_INTERFACE_PRECALL;
	HRESULT hr = (*fp)();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}
#pragma optimize ("", on)

// @pymethod |pythoncom|CoTreatAsClass|Establishes or removes an emulation, in which objects of one class are treated as objects of a different class.
static PyObject *pythoncom_CoTreatAsClass(PyObject *self, PyObject *args)
{
	PyObject *obguid1, *obguid2 = NULL;
	if (!PyArg_ParseTuple(args, "O|O", &obguid1, &obguid2))
		return NULL;
	CLSID clsid1, clsid2 = GUID_NULL;
	// @pyparm <o PyIID>|clsidold||CLSID of the object to be emulated. 
	// @pyparm <o PyIID>|clsidnew|CLSID_NULL|CLSID of the object that should emulate the original object. This replaces any existing emulation for clsidOld. Can be ommitted or CLSID_NULL, in which case any existing emulation for clsidOld is removed. 
	if (!PyWinObject_AsIID(obguid1, &clsid1))
		return NULL;
	if (obguid2!=NULL && !PyWinObject_AsIID(obguid2, &clsid2))
			return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = CoTreatAsClass(clsid1, clsid2);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}
#endif // MS_WINCE

// @pymethod <o PyIClassFactory>|pythoncom|MakePyFactory|Creates a new <o PyIClassFactory> object wrapping a PythonCOM Class Factory object.
static PyObject *pythoncom_MakePyFactory(PyObject *self, PyObject *args)
{
	PyObject *obIID;
	if (!PyArg_ParseTuple(args, "O:MakePyFactory", 
		&obIID))	// @pyparm <o PyIID>|iid||The IID of the object the class factory provides.
		return NULL;
	IID iid;
	if (!PyWinObject_AsIID(obIID, &iid))
		return NULL;

	PY_INTERFACE_PRECALL;
	CPyFactory *pFact = new CPyFactory(iid);
	PY_INTERFACE_POSTCALL;
	if (pFact==NULL)
		return PyCom_BuildPyException(E_OUTOFMEMORY);
	return PyCom_PyObjectFromIUnknown(pFact, IID_IClassFactory, /*bAddRef =*/FALSE);
}

// @pymethod int|pythoncom|_GetInterfaceCount|Retrieves the number of interface objects currently in existance
static PyObject *pythoncom_GetInterfaceCount(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":_GetInterfaceCount"))
		return NULL;
	return PyInt_FromLong(_PyCom_GetInterfaceCount());
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
	return PyInt_FromLong(_PyCom_GetGatewayCount());
}

#ifndef MS_WINCE
// @pymethod <o PyIUnknown>|pythoncom|GetActiveObject|Retrieves an object representing a running object registered with OLE
static PyObject *pythoncom_GetActiveObject(PyObject *self, PyObject *args)
{
	PyObject *obCLSID;
	// @pyparm CLSID|cls||The IID for the program.  As for all CLSID's in Python, a "program.name" or IID format string may be used, or a real <o PyIID> object.
	if (!PyArg_ParseTuple(args, "O:GetActiveObject",
		&obCLSID))
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
	if (!PyArg_ParseTuple(args, "O:Connect",
		&obCLSID))
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
	Py_BEGIN_ALLOW_THREADS; // Cant use the INTERFACE macros twice :-(
	sc = unk->QueryInterface(IID_IDispatch, (void**)&disp);
	unk->Release();
	Py_END_ALLOW_THREADS;
	if (FAILED(sc) || disp == NULL)
		return PyCom_BuildPyException(sc);
	return PyCom_PyObjectFromIUnknown(disp, IID_IDispatch);
	// @comm This function is equivilent to <om pythoncom.GetActiveObject>(clsid).<om pythoncom.QueryInterace>(pythoncom.IID_IDispatch)
}
#endif // MS_WINCE

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
	int clsctx = PyCom_HasDCom() ? CLSCTX_SERVER : CLSCTX_INPROC_SERVER| CLSCTX_LOCAL_SERVER;
	PyObject *obIID = PyWinObject_FromIID(IID_IDispatch);
	PyObject *newArgs = Py_BuildValue("OOiO", progid, Py_None, clsctx, obIID);
	Py_DECREF(obIID);
	PyObject *rc = pythoncom_CoCreateInstance(self, newArgs);
	Py_DECREF(newArgs);
	return rc;
}

#ifndef MS_WINCE
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
	// @comm Use the CreateGuid function when you need an absolutely unique number that you will use as a persistent identifier in a distributed environment.To a very high degree of certainty, this function returns a unique value – no other invocation, on the same or any other system (networked or not), should return the same value.
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
#endif // MS_WINCE

// @pymethod string|pythoncom|GetScodeString|Returns the string for an OLE scode (HRESULT)
static PyObject *pythoncom_GetScodeString(PyObject *self, PyObject *args)
{
	SCODE scode;
	TCHAR buf[512];
	// @pyparm int|scode||The OLE error code for the scode string requested.
	if (!PyArg_ParseTuple(args, "i", &scode))
		return NULL;
	GetScodeString(scode, buf, sizeof(buf));
	return PyString_FromTCHAR(buf);
	// @comm This will obtain the COM Error message for a given HRESULT.
	// Internally, PythonCOM uses this function to obtain the description
	// when a <o com_error> COM Exception is raised.
}

// @pymethod string|pythoncom|GetScodeRangeString|Returns the scode range string, given an OLE scode.
static PyObject *pythoncom_GetScodeRangeString(PyObject *self, PyObject *args)
{
	SCODE scode;
	// @pyparm int|scode||An OLE error code to return the scode range string for.
	if (!PyArg_ParseTuple(args, "i", &scode))
		return NULL;
	return Py_BuildValue("z", GetScodeRangeString(scode) );
}

// @pymethod string|pythoncom|GetSeverityString|Returns the severity string, given an OLE scode.
static PyObject *pythoncom_GetSeverityString(PyObject *self, PyObject *args)
{
	SCODE scode;
	// @pyparm int|scode||The OLE error code for the severity string requested.
	if (!PyArg_ParseTuple(args, "i", &scode))
		return NULL;
	return Py_BuildValue("z", GetSeverityString(scode) );
}

// @pymethod string|pythoncom|GetFacilityString|Returns the facility string, given an OLE scode.
static PyObject *pythoncom_GetFacilityString(PyObject *self, PyObject *args)
{
	SCODE scode;
	// @pyparm int|scode||The OLE error code for the facility string requested.
	if (!PyArg_ParseTuple(args, "i", &scode))
		return NULL;
	return Py_BuildValue("z", GetFacilityString(scode) );
}

static PyObject *pythoncom_RecoverFromID(PyObject *self, PyObject *args)
{
	int p = 0;

	if ( !PyArg_ParseTuple(args, "i", &p) )
		return NULL;

	if ( !p )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	return (PyObject *)p;
}

// @pymethod <o PyIDispatch>|pythoncom|UnwrapObject|Unwraps a Python instance in a gateway object.
static PyObject *pythoncom_UnwrapObject(PyObject *self, PyObject *args)
{
	PyObject *ob;
	// @pyparm <o PyIUnknown>|ob||The object to unwrap.
	if ( !PyArg_ParseTuple(args, "O", &ob ) )
		return NULL;
	// @comm If the object is not a PythonCOM object, then ValueError is raised.
	if ( !PyIBase::is_object(ob, &PyIUnknown::type) ) {
		PyErr_SetString(PyExc_ValueError, "argument is not a COM object");
		return NULL;
	}

	// Unwrapper does not need thread state management
	// Ie PY_INTERFACE_PRE/POSTCALL;
	HRESULT hr;
	IInternalUnwrapPythonObject *pUnwrapper;
	if (S_OK!=(hr=((PyIUnknown *)ob)->m_obj->QueryInterface(IID_IInternalUnwrapPythonObject, (void **)&pUnwrapper))) {
		PyErr_Format(PyExc_ValueError, "argument is not a Python gateway (0x%x)", hr);
		return NULL;
	}
	PyObject *retval;
	pUnwrapper->Unwrap(&retval);
	pUnwrapper->Release();
	if (S_OK!=hr)
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
	// @pyparm <o PyIID>|gatewayIID|IID_IDispatch|The IID of the gateway object to create (ie, the interface of the server object wrapped by the return value)
	// @pyparm <o PyIID>|interfaceIID|IID_IDispatch|The IID of the interface object to create (ie, the interface of the returned object)
	if ( !PyArg_ParseTuple(args, "O|OO", &ob, &obIID, &obIIDInterface) )
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
	// XXX - If we did, we would unwrap it like thus:
	/****
	IUnknown *pLook = (IUnknown *)(*ppv);
	IInternalUnwrapPythonObject *pTemp;
	if (pLook->QueryInterface(IID_IInternalUnwrapPythonObject, (void **)&pTemp)==S_OK) {
		// One of our objects, so set the base object if it doesnt already have one
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
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	/* pass the pDispatch reference into this thing */
	/* ### this guy should always AddRef() ... */
	PyObject *result = PyCom_PyObjectFromIUnknown(pDispatch, iidInterface, FALSE);
	if ( !result )
	{
		PY_INTERFACE_PRECALL;
		pDispatch->Release();
		PY_INTERFACE_POSTCALL;
		return NULL;
	}

	return result;
}

// @pymethod <o PyIID>|pythoncom|MakeIID|Creates a new IID object.
static PyObject *pythoncom_MakeIID(PyObject *self, PyObject *args)
{
	// @comm This is simply an alias for <om pywintypes.IID>.  Please see that method
	// for details.
	return PyWinMethod_NewIID(self, args);
}

// @pymethod <o PyTime>|pythoncom|MakeTime|Creates a new time object.
static PyObject *pythoncom_MakeTime(PyObject *self, PyObject *args)
{
	// @comm This is simply an alias for <om pywintypes.Time>.  Please see that method
	// for details.
	return PyWinMethod_NewTime(self, args);
}

// @pymethod <o PyUnicode>|pythoncom|Unicode|Converts a string into a <o PyUnicode> object.
static PyObject *pythoncom_Unicode(PyObject *self, PyObject *args)
{
	const char *s;

	// @pyparm string|s||The string to convert into a Unicode object
    if ( !PyArg_ParseTuple(args, "s:Unicode", &s))
       return NULL;
	return PyUnicodeObject_FromString(s);
	// @comm As Python itself becomes Unicode aware, this function will not be necessary.
}

#ifndef MS_WINCE
// @pymethod <o PyIMoniker>,int,<o PyIBindCtx>|pythoncom|MkParseDisplayName|Parses a moniker display name into a moniker object. The inverse of <om PyIMoniker.GetDisplayName>
static PyObject *pythoncom_MkParseDisplayName(PyObject *self, PyObject *args)
{
	const char *displayName;
	PyObject *obBindCtx = NULL;

	// @pyparm string|displayName||The display name to parse
	// @pyparm <o PyIBindCtx>|bindCtx|None|The bind context object to use.
	// @comm If a binding context is not provided, then one will be created.
	// Any binding context created or passed in will be returned to the
	// caller.
	if ( !PyArg_ParseTuple(args, "s|O:MkParseDisplayName", &displayName, &obBindCtx) )
		return NULL;

	HRESULT hr;
	IBindCtx *pBC;
	if ( obBindCtx == NULL || obBindCtx==Py_None)
	{
		hr = CreateBindCtx(0, &pBC);
		if ( FAILED(hr) )
			return PyCom_BuildPyException(hr);

		/* pass the pBC ref into obBindCtx */
		obBindCtx = PyCom_PyObjectFromIUnknown(pBC, IID_IBindCtx, FALSE);
	}
	else
	{
		if ( !PyCom_InterfaceFromPyObject(obBindCtx, IID_IBindCtx, (LPVOID*)&pBC, FALSE) )
			return NULL;

		/* we want our own ref to obBindCtx, but not pBC */
		Py_INCREF(obBindCtx);
		pBC->Release();
	}
	/* at this point: we own a ref to obBindCtx, but not pBC */

	USES_CONVERSION;
	ULONG chEaten;
	IMoniker *pmk;
	PY_INTERFACE_PRECALL;
	hr = MkParseDisplayName(pBC, A2W(displayName), &chEaten, &pmk);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
	{
		Py_DECREF(obBindCtx);
		return PyCom_BuildPyException(hr);
	}

	/* pass ownership of the moniker into the result */
	PyObject *obMoniker = PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);

	/* build the result */
	PyObject *result = Py_BuildValue("OiO", obMoniker, chEaten, obBindCtx);

	/* done with these obs */
	Py_XDECREF(obMoniker);
	Py_DECREF(obBindCtx);

	return result;
}

// @pymethod <o PyIMoniker>|pythoncom|CreatePointerMoniker|Creates a new <o PyIMoniker> object.
static PyObject *pythoncom_CreatePointerMoniker(PyObject *self, PyObject *args)
{
	PyObject *obUnk;
	// @pyparm <o PyIUnknown>|IUnknown||The interface for the moniker.
	if ( !PyArg_ParseTuple(args, "O:CreatePointerMoniker", &obUnk) )
		return NULL;

	IUnknown *punk;
	if ( !PyCom_InterfaceFromPyObject(obUnk, IID_IUnknown, (LPVOID*)&punk, FALSE) )
		return NULL;

	IMoniker *pmk;
	PY_INTERFACE_PRECALL;
	HRESULT hr = CreatePointerMoniker(punk, &pmk);
	punk->Release();
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	return PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);
}

// @pymethod <o PyIMoniker>|pythoncom|CreateFileMoniker|Creates a new <o PyIMoniker> object.
static PyObject *pythoncom_CreateFileMoniker(PyObject *self, PyObject *args)
{
	PyObject *obName;
	// @pyparm string|filename||The name of the file.
	if ( !PyArg_ParseTuple(args, "O:CreateFileMoniker", &obName) )
		return NULL;

	BSTR bstrName;
	if (!PyWinObject_AsBstr(obName, &bstrName))
		return NULL;

	IMoniker *pmk;
	PY_INTERFACE_PRECALL;
	HRESULT hr = CreateFileMoniker(bstrName, &pmk);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeBstr(bstrName);

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	return PyCom_PyObjectFromIUnknown(pmk, IID_IMoniker, FALSE);
}

// @pymethod <o PyIID>|pythoncom|GetClassFile|Supplies the CLSID associated with the given filename.
static PyObject *pythoncom_GetClassFile(PyObject *self, PyObject *args)
{
	CLSID clsid;
	PyObject *obFileName;
	BSTR fname;
	// @pyparm string|fileName||The filename for which you are requesting the associated CLSID.
	if (!PyArg_ParseTuple(args, "O", &obFileName))
		return NULL;

	if (!PyCom_BstrFromPyObject(obFileName, &fname, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = GetClassFile(fname, &clsid);
	PY_INTERFACE_POSTCALL;
	SysFreeString(fname);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyWinObject_FromIID(clsid);
}
#endif // MS_WINCE

// @pymethod |pythoncom|CoInitialize|Initialize the COM libraries for the calling thread.
static PyObject *pythoncom_CoInitialize(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":CoInitialize"))
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = PyCom_CoInitialize(NULL);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm Equivilent to <om pythoncom.CoInitializeEx>(pythoncom.COINIT_APARTMENTTHREADED).
	// See <om pythoncom.CoInitializeEx> for a description.
}

// @pymethod |pythoncom|CoInitializeEx|Initialize the COM libraries for the calling thread.
static PyObject *pythoncom_CoInitializeEx(PyObject *self, PyObject *args)
{
	DWORD val;
	if (!PyArg_ParseTuple(args, "l:CoInitializeEx", &val))
	// @pyparm int|flags||Flags for the initialization.
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = PyCom_CoInitializeEx(NULL, val);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		PyCom_BuildPyException(hr);
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

// @pymethod |pythoncom|CoFreeUnusedLibraries|Unloads any DLLs that are no longer in use and that, when loaded, were specified to be freed automatically.
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

#ifndef MS_WINCE
// @pymethod <o PyIRunningObjectTable>|pythoncom|GetRunningObjectTable|Creates a new <o PyIRunningObjectTable> object.
static PyObject *pythoncom_GetRunningObjectTable(PyObject *self, PyObject *args)
{
	DWORD reserved = 0;
	// @pyparm int|reserved|0|A reserved parameter.  Should be zero unless you have inside information that I don't!
	if ( !PyArg_ParseTuple(args, "|l:GetRunningObjectTable", &reserved) )
		return NULL;
	IRunningObjectTable *pROT = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = GetRunningObjectTable(reserved,&pROT);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr)
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pROT, IID_IRunningObjectTable, FALSE);
}

// @pymethod <o PyIBindCtx>|pythoncom|CreateBindCtx|Creates a new <o PyIBindCtx> object.
static PyObject *pythoncom_CreateBindCtx(PyObject *self, PyObject *args)
{
	DWORD reserved = 0;
	if ( !PyArg_ParseTuple(args, "|l:CreateBindCtx", &reserved) )
		return NULL;
	IBindCtx *pBC = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = CreateBindCtx(reserved,&pBC);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr)
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pBC, IID_IBindCtx, FALSE);
}

// @pymethod int|pythoncom|RegisterActiveObject|Register an object as the active object for its class
static PyObject *pythoncom_RegisterActiveObject(PyObject *self, PyObject *args)
{    
	DWORD    dwflags=0, dwkey=0;
     HRESULT  hr;
     CLSID    clsid;
     PyObject *obclsid;
     PyObject *obunk;
     IUnknown *punk;

	// @pyparm <o PyIUnknown>|obUnknown||The object to register.
	// @pyparm <o PyIID>|clsid||The CLSID for the object
	// @pyparm int|flags||Flags to use.
    if (!PyArg_ParseTuple(args, "OOi:RegisterActiveObject",
                                &obunk,
                                &obclsid,
                                &dwflags)) return NULL;

    if (!PyWinObject_AsIID(obclsid, &clsid)) return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obunk, IID_IUnknown, (void **)&punk), FALSE) return NULL;

	PY_INTERFACE_PRECALL;
    hr = RegisterActiveObject(punk, clsid, dwflags, &dwkey);
	punk->Release();
	PY_INTERFACE_POSTCALL;
    if (S_OK!=hr) return PyCom_BuildPyException(hr);
    return PyInt_FromLong(dwkey);
	// @rdesc The result is a handle which should be pass to <om pythoncom.RevokeActiveObject>
}

// @pymethod |pythoncom|RevokeActiveObject|Ends an object’s status as active.
static PyObject *pythoncom_RevokeActiveObject(PyObject *self, PyObject *args)
{
	DWORD    dw_x=0;
    HRESULT  hr;
     
	// @pyparm int|handle||A handle obtained from <om pythoncom.RegisterActiveObject>
    if(!PyArg_ParseTuple(args,"l:RevokeActiveObject", &dw_x))
       return NULL;
	PY_INTERFACE_PRECALL;
    hr = RevokeActiveObject(dw_x, NULL);
	PY_INTERFACE_POSTCALL;
    if (S_OK!=hr) return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// Some basic marshalling support
// @pymethod <o PyIStream>|pythoncom|CoMarshalInterThreadInterfaceInStream|Marshals an interface pointer from one thread to another thread in the same process.
static PyObject *pythoncom_CoMarshalInterThreadInterfaceInStream(PyObject *self, PyObject*args)
{
	PyObject *obIID, *obUnk;
	IID iid;
    if ( !PyArg_ParseTuple(args, "OO:CoMarshalInterThreadInterfaceInStream", 
			&obIID, // @pyparm <o PyIID>|iid||The IID of the interface to marshal.
			&obUnk)) // @pyparm <o PyIUnknown>|unk||The interface to marshal.
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

// @pymethod <o PyIUnknown>|pythoncom|CoGetInterfaceAndReleaseStream|Unmarshals a buffer containing an interface pointer and releases the stream when an interface pointer has been marshaled from another thread to the calling thread.
static PyObject *pythoncom_CoGetInterfaceAndReleaseStream(PyObject *self, PyObject*args)
{
	PyObject *obStream, *obIID;
    if ( !PyArg_ParseTuple(args, "OO:CoGetInterfaceAndReleaseStream", 
			&obStream, // @pyparm <o PyIStream>|stream||The stream to unmarshal the object from.
			&obIID )) // @pyparm <o PyIID>|iid||The IID if the interface to unmarshal.
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
// @pymethod <o PyIUnknown>|pythoncom|CoCreateFreeThreadedMarshaler|Creates an aggregatable object capable of context-dependent marshaling. 
static PyObject *pythoncom_CoCreateFreeThreadedMarshaler(PyObject *self, PyObject*args)
{
	PyObject *obUnk;
    if ( !PyArg_ParseTuple(args, "O:CoCreateFreeThreadedMarshaler", 
			&obUnk )) // @pyparm <o PyIUnknown>|unk||The unknown object to marshal.
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

#endif // MS_WINCE

// @pymethod |pythoncom|OleLoadFromStream|Load an object from an IStream.
static PyObject *pythoncom_OleLoadFromStream(PyObject *self, PyObject* args)
{
	PyObject *obStream, *obIID;
	if ( !PyArg_ParseTuple(args, "OO:OleLoadFromStream",
		&obStream, // @pyparm <o PyIStream>|stream||The stream to load the object from.
		&obIID )) // @pyparm <o PyIID>|iid||The IID if the interface to load.
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
static PyObject *pythoncom_OleSaveToStream(PyObject *self, PyObject*args)
{
	PyObject *obPersist,*obStream;
	if ( !PyArg_ParseTuple(args, "OO:OleSaveToStream",
		&obPersist,  // @pyparm <o PyIPersistStream>|persist||The object to save
		&obStream )) // @pyparm <o PyIStream>|stream||The stream to save the object to.
		return NULL;

	// This parameter is allowed to be None. This follows the COM documentation rather
	// than the COM implementation, which is likely to return an error if you do pass
	// it a NULL IPersistStream
	IPersistStream *pPersist;
	if (!PyCom_InterfaceFromPyObject(obPersist, IID_IPersistStream, (void**)&pPersist, FALSE))
		return NULL;

	IStream *pStream;
	if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
	{
		PY_INTERFACE_PRECALL;
		if(pPersist) pPersist->Release();
		PY_INTERFACE_POSTCALL;
		return NULL;
	}

	PY_INTERFACE_PRECALL;
	HRESULT hr = OleSaveToStream(pPersist, pStream);
	pStream->Release();
	if(pPersist) pPersist->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod <o ICreateTypeLib>|pythoncom|CreateTypeLib|Provides access to a new object instance that supports the ICreateTypeLib interface.
static PyObject *pythoncom_CreateTypeLib(PyObject *self, PyObject *args)
{
	long syskind;
	PyObject *obfname;
	if (!PyArg_ParseTuple(args, "lO", &syskind, &obfname))
		return NULL;
	BSTR fname;
	if (!PyWinObject_AsBstr(obfname, &fname))
		return NULL;
	ICreateTypeLib *pcti = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = CreateTypeLib((SYSKIND)syskind, fname, &pcti);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeBstr(fname);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pcti, IID_ICreateTypeLib, FALSE);
}

// @pymethod int|pythoncom|PumpWaitingMessages|Pumps all waiting messages for the current thread.
// @comm It is sometimes necessary for a COM thread to have a message loop.  This function
// can be used with <om win32event.MsgWaitForMultipleObjects> to pump all messages
// when necessary.  Please see the COM documentation for more details.
// @rdesc Returns 1 if a WM_QUIT message was received, else 0
static PyObject *pythoncom_PumpWaitingMessages(PyObject *self, PyObject *args)
{
	UINT firstMsg = 0, lastMsg = 0;
	if (!PyArg_ParseTuple (args, "|ii:PumpWaitingMessages", &firstMsg, &lastMsg))
		return NULL;
	// @pyseeapi PeekMessage and DispatchMessage

    MSG msg;
	long result = 0;
	// Read all of the messages in this next loop, 
	// removing each message as we read it.
	Py_BEGIN_ALLOW_THREADS
	while (PeekMessage(&msg, NULL, firstMsg, lastMsg, PM_REMOVE)) {
		// If it's a quit message, we're out of here.
		if (msg.message == WM_QUIT) {
			result = 1;
			break;
		}
		// Otherwise, dispatch the message.
		DispatchMessage(&msg); 
	} // End of PeekMessage while loop
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(result);
}

// @pymethod |pythoncom|PumpMessages|Pumps all messages for the current thread until a WM_QUIT message.
static PyObject *pythoncom_PumpMessages(PyObject *self, PyObject *args)
{
	MSG msg;
	int rc;
	Py_BEGIN_ALLOW_THREADS
	while ((rc=GetMessage(&msg, 0, 0, 0))==1) {
		TranslateMessage(&msg); // needed?
		DispatchMessage(&msg);
	}
	Py_END_ALLOW_THREADS
	if (rc==-1)
		return PyWin_SetAPIError("GetMessage");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |pythoncom|EnableQuitMessage|Indicates the thread PythonCOM should post a WM_QUIT message to.
static PyObject *pythoncom_EnableQuitMessage(PyObject *self, PyObject *args)
{
	extern void PyCom_EnableQuitMessage( DWORD dwThreadId );

	DWORD id;
	// @pyparm int|threadId||The thread ID.
	if (!PyArg_ParseTuple (args, "l:EnableQuitMessage", &id))
		return NULL;
	PyCom_EnableQuitMessage(id);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *pythoncom_OleLoadPicture(PyObject *, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *obStream, *obIIDAPI, *obIIDRet = NULL;
	LONG size;
	BOOL runMode;
	if (!PyArg_ParseTuple(args, "OiiO|O", &obStream, &size, &runMode, &obIIDAPI, &obIIDRet)) {
		return NULL;
	}

	IUnknown *pUnk = NULL;
	IStream* pStream = NULL;
	IID iidAPI, iidRet;
	HRESULT hr;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obStream, IID_IStream, (void **)&pStream, FALSE))
		goto done;

	if (!PyWinObject_AsIID(obIIDAPI, &iidAPI))
		goto done;
	if (obIIDRet == NULL)
		iidRet = iidAPI;
	else {
		if (!PyWinObject_AsIID(obIIDRet, &iidRet))
			goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	hr = ::OleLoadPicture(pStream, size, runMode, iidAPI, (LPVOID*)&pUnk);
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	ret = PyCom_PyObjectFromIUnknown(pUnk, iidRet, FALSE);
done:
	if (pStream)
		pStream->Release();
	return ret;
}



/* List of module functions */
// @module pythoncom|A module, encapsulating the OLE automation API
static struct PyMethodDef pythoncom_methods[]=
{
	{ "_GetInterfaceCount",  pythoncom_GetInterfaceCount, 1},    // @pymeth _GetInterfaceCount|Retrieves the number of interface objects currently in existance
	{ "_GetGatewayCount",    pythoncom_GetGatewayCount, 1},    // @pymeth _GetInterfaceCount|Retrieves the number of gateway objects currently in existance
#ifndef MS_WINCE
	{ "CoCreateFreeThreadedMarshaler", pythoncom_CoCreateFreeThreadedMarshaler, 1},// @pymeth CoCreateFreeThreadedMarshaler|Creates an aggregatable object capable of context-dependent marshaling.
	{ "CoCreateInstanceEx",  pythoncom_CoCreateInstanceEx, 1 },    // @pymeth CoCreateInstanceEx|Create a new instance of an OLE automation server possibly on a remote machine.
#endif // MS_WINCE
	{ "CoCreateInstance",    pythoncom_CoCreateInstance, 1 },    // @pymeth CoCreateInstance|Create a new instance of an OLE automation server.
	{ "CoFreeUnusedLibraries", pythoncom_CoFreeUnusedLibraries, 1}, // @pymeth CoFreeUnusedLibraries|Unloads any DLLs that are no longer in use and that, when loaded, were specified to be freed automatically.
	{ "CoInitialize",        pythoncom_CoInitialize, 1 },		   // @pymeth CoInitialize|Initialize the COM libraries for the calling thread.
	{ "CoInitializeEx",      pythoncom_CoInitializeEx, 1 },        // @pymeth CoInitializeEx|Initialize the COM libraries for the calling thread.
#ifndef MS_WINCE
	{ "CoInitializeSecurity",pythoncom_CoInitializeSecurity, 1}, // @pymeth CoInitializeSecurity|Registers security and sets the default security values. 
	{ "CoGetInterfaceAndReleaseStream", pythoncom_CoGetInterfaceAndReleaseStream, 1}, // @pymeth CoGetInterfaceAndReleaseStream|Unmarshals a buffer containing an interface pointer and releases the stream when an interface pointer has been marshaled from another thread to the calling thread.
	{ "CoMarshalInterThreadInterfaceInStream", pythoncom_CoMarshalInterThreadInterfaceInStream, 1}, // @pymeth CoMarshalInterThreadInterfaceInStream|Marshals an interface pointer from one thread to another thread in the same process.
#endif // MS_WINCE
	{ "CoUninitialize",      pythoncom_CoUninitialize, 1 },		   // @pymeth CoUninitialize|Uninitialize the COM libraries.
#ifndef MS_WINCE
	{ "CoRegisterClassObject",pythoncom_CoRegisterClassObject, 1 },// @pymeth CoRegisterClassObject|Registers an EXE class object with OLE so other applications can connect to it.
	{ "CoResumeClassObjects", pythoncom_CoResumeClassObjects, 1},  // @pymeth CoResumeClassObjects|Called by a server that can register multiple class objects to inform the OLE SCM about all registered classes, and permits activation requests for those class objects.
	{ "CoRevokeClassObject",pythoncom_CoRevokeClassObject, 1 },// @pymeth CoRevokeClassObject|Informs OLE that a class object, previously registered with the <om pythoncom.CoRegisterClassObject> method, is no longer available for use. 
	{ "CoTreatAsClass",      pythoncom_CoTreatAsClass, 1}, // @pymeth CoTreatAsClass|Establishes or removes an emulation, in which objects of one class are treated as objects of a different class.
	{ "Connect",             pythoncom_connect, 1 },			 // @pymeth Connect|Connects to a running instance of an OLE automation server.
	{ "connect",             pythoncom_connect, 1 },
	{ "CreateGuid",          pythoncom_createguid, 1 },          // @pymeth CreateGuid|Creates a new, unique GUIID.
	{ "CreateBindCtx",       pythoncom_CreateBindCtx, 1 },       // @pymeth CreateBindCtx|Obtains a <o PyIBindCtx> object.
	{ "CreateFileMoniker",   pythoncom_CreateFileMoniker, 1 }, // @pymeth CreateFileMoniker|Creates a file moniker given a file name.
	{ "CreatePointerMoniker", pythoncom_CreatePointerMoniker, 1 }, // @pymeth CreatePointerMoniker|Creates a pointer moniker based on a pointer to an object.
	{ "CreateTypeLib",       pythoncom_CreateTypeLib, 1}, // @pymeth CreateTypeLib|Provides access to a new object instance that supports the ICreateTypeLib interface.
#endif // MS_WINCE
	{ "EnableQuitMessage",   pythoncom_EnableQuitMessage, 1 }, // @pymeth EnableQuitMessage|Indicates the thread PythonCOM should post a WM_QUIT message to.
	{ "FUNCDESC",            Py_NewFUNCDESC, 1}, // @pymeth FUNCDESC|Returns a new <o FUNCDESC> object.
#ifndef MS_WINCE
	{ "GetActiveObject",     pythoncom_GetActiveObject, 1 },     // @pymeth GetActiveObject|Retrieves an object representing a running object registered with OLE
	{ "GetClassFile",        pythoncom_GetClassFile, 1 },        // @pymeth GetClassFile|Supplies the CLSID associated with the given filename.
#endif // MS_WINCE
	{ "GetFacilityString",   pythoncom_GetFacilityString, 1 },   // @pymeth GetFacilityString|Returns the facility string, given an OLE scode.
	{ "GetRecordFromGuids",  pythoncom_GetRecordFromGuids, 1},   // @pymeth GetRecordFromGuids|Creates a new record object from the given GUIDs
#ifndef MS_WINCE
	{ "GetRunningObjectTable", pythoncom_GetRunningObjectTable, 1 }, // @pymeth GetRunningObjectTable|Obtains a <o PyIRunningObjectTable> object.
#endif // MS_WINCE
	{ "GetScodeString",      pythoncom_GetScodeString, 1 },      // @pymeth GetScodeString|Returns the string for an OLE scode.
	{ "GetScodeRangeString", pythoncom_GetScodeRangeString, 1 }, // @pymeth GetScodeRangeString|Returns the scode range string, given an OLE scode.
	{ "GetSeverityString",   pythoncom_GetSeverityString, 1 },   // @pymeth GetSeverityString|Returns the severity string, given an OLE scode.
	{ "IsGatewayRegistered", pythoncom_IsGatewayRegistered, 1}, // @pymeth IsGatewayRegistered|Returns 1 if the given IID has a registered gateway object.
	{ "LoadRegTypeLib",      pythoncom_loadregtypelib, 1 },		 // @pymeth LoadRegTypeLib|Loads a registered type library by CLSID
	{ "LoadTypeLib",         pythoncom_loadtypelib, 1 },		 // @pymeth LoadTypeLib|Loads a type library by name
	{ "MakeIID",             pythoncom_MakeIID, 1 },             // @pymeth MakeIID|Makes an IID object from a string.
	{ "MakeTime",			pythoncom_MakeTime, 1 },			// @pymeth MakeTime|Makes a time object from the argument.  Argument can be an integer/float or a tuple (as returned by time module functions).
	{ "MakePyFactory",       pythoncom_MakePyFactory, 1 },      // @pymeth MakePyFactory|Creates a new <o PyIClassFactory> object wrapping a PythonCOM Class Factory object.
#ifndef MS_WINCE
	{ "MkParseDisplayName",	pythoncom_MkParseDisplayName, 1 },	// @pymeth MkParseDisplayName|Parses a moniker display name into a moniker object. The inverse of IMoniker::GetDisplayName.
#endif // MS_WINCE
	{ "new",                 pythoncom_new, 1 },
	{ "New",                 pythoncom_new, 1 },                 // @pymeth New|Create a new instance of an OLE automation server.
	{ "OleLoadPicture",      pythoncom_OleLoadPicture, 1 },      // @pymeth OleLoadPicture|Creates a new picture object and initializes it from the contents of a stream.
#ifndef MS_WINCE
	{ "ProgIDFromCLSID",     pythoncom_progidfromclsid, 1 },     // @pymeth ProgIDFromCLSID|Converts a CLSID string to a progID.
#endif // MS_WINCE
	{ "PumpWaitingMessages", pythoncom_PumpWaitingMessages, 1},  // @pymeth PumpWaitingMessages|Pumps all waiting messages for the current thread.
	{ "PumpMessages",	     pythoncom_PumpMessages, 1},  // @pymeth PumpMessages|Pumps all messages for the current thread until a WM_QUIT message.
#ifndef MS_WINCE
	{ "QueryPathOfRegTypeLib",pythoncom_querypathofregtypelib, 1}, // @pymeth QueryPathOfRegTypeLib|Retrieves the path of a registered type library
#endif // MS_WINCE
	{ "ReadClassStg",        pythoncom_ReadClassStg, 1}, // @pymeth ReadClassStg|Reads a CLSID from a storage object
	{ "RecoverFromID",       pythoncom_RecoverFromID, 1 },
	{ "RegisterTypeLib",     pythoncom_registertypelib, 1}, // @pymeth RegisterTypeLib|Adds information about a type library to the system registry.
	{ "UnRegisterTypeLib",     pythoncom_unregistertypelib, 1}, // @pymeth UnRegisterTypeLib|Removes a type library from the system registry.
#ifndef MS_WINCE
	{ "RegisterActiveObject",pythoncom_RegisterActiveObject, 1 }, // @pymeth RegisterActiveObject|Register an object as the active object for its class
	{ "RevokeActiveObject",  pythoncom_RevokeActiveObject, 1 },   // @pymeth RevokeActiveObject|Ends an object’s status as active.
#endif // MS_WINCE
	{ "StgCreateDocfile",      pythoncom_StgCreateDocfile, 1 },       // @pymeth StgCreateDocfile|Creates a new compound file storage object using the OLE-provided compound file implementation for the <o PyIStorage> interface.
	{ "StgCreateDocfileOnILockBytes",      pythoncom_StgCreateDocfileOnILockBytes, 1 }, // @pymeth StgCreateDocfileOnILockBytes|Creates a new compound file storage object using the OLE-provided compound file implementation for the <o PyIStorage> interface.
#ifndef MS_WINCE
	{ "StgIsStorageFile",    pythoncom_StgIsStorageFile, 1 },       // @pymeth StgIsStorageFile|Indicates whether a particular disk file contains a storage object.
#endif // MS_WINCE
	{ "StgOpenStorage",      pythoncom_StgOpenStorage, 1 },       // @pymeth StgOpenStorage|Opens an existing root storage object in the file system.
	{ "TYPEATTR",            Py_NewTYPEATTR, 1}, // @pymeth TYPEATTR|Returns a new <o TYPEATTR> object.
	{ "VARDESC",             Py_NewVARDESC, 1}, // @pymeth VARDESC|Returns a new <o VARDESC> object.
	{ "WrapObject",          pythoncom_WrapObject, 1 }, // @pymeth WrapObject|Wraps an object in a gateway.
	{ "WriteClassStg",       pythoncom_WriteClassStg, 1}, // @pymeth WriteClassStg|Stores a CLSID from a storage object
	{ "UnwrapObject",        pythoncom_UnwrapObject, 1 }, // @pymeth UnwrapObject|Unwraps a Python instance in a gateway object.
	{ "Unicode",			pythoncom_Unicode, 1 }, // @pymeth Unicode|Converts a string into a <o PyUnicode> object.
	{ NULL, NULL }
};

int AddConstant(PyObject *dict, const char *key, long value)
{
	PyObject *oval = PyInt_FromLong(value);
	if (!oval)
	{
		return 1;
	}
	int rc = PyDict_SetItemString(dict, (char*)key, oval);
	Py_DECREF(oval);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)

static char *modName = "pythoncom";

extern BOOL initunivgw(PyObject *parentDict);

/* Module initialisation */
extern "C" __declspec(dllexport) void initpythoncom()
{
	PyObject *oModule;

	// The DLL Load inited the module.
	// All we do here is init COM itself.  Done here
	// so other clients get a chance to beat us to it!

	// Support a special sys.coinit_flags attribute to control us.
	DWORD coinit_flags = COINIT_APARTMENTTHREADED;

	PyObject *obFlags = PySys_GetObject("coinit_flags");
	// No reference added to obFlags.
	if (obFlags) {
		if (PyInt_Check(obFlags))
			coinit_flags = PyInt_AsLong(obFlags);
	} else
		PyErr_Clear(); // Error raised by no coinit_flags attribute.

	HRESULT hr = PyCom_CoInitializeEx(NULL, coinit_flags);
	if (hr==E_NOTIMPL) // Special return val from PyCom_Co.. indicates not DCOM.
		hr = PyCom_CoInitialize(NULL);
	// If HR fails, we really dont care - the import should work.  User can
	// manually CoInit() to see!

	// ensure the framework has valid state to work with.
	PyWinGlobals_Ensure();

	// Create the module and add the functions
	oModule = Py_InitModule(modName, pythoncom_methods);
	if (!oModule) /* Eeek - some serious error! */
		return;

	PyObject *dict = PyModule_GetDict(oModule);
	if (!dict) return; /* Another serious error!*/
	PyDict_SetItemString(dict, "TypeIIDs", g_obPyCom_MapIIDToType);
	PyDict_SetItemString(dict, "ServerInterfaces", g_obPyCom_MapGatewayIIDToName);
	PyDict_SetItemString(dict, "InterfaceNames", g_obPyCom_MapInterfaceNameToIID);

	g_obEmpty = new PyOleEmpty;
	PyDict_SetItemString(dict, "Empty", g_obEmpty);

	g_obMissing = new PyOleMissing;
	PyDict_SetItemString(dict, "Missing", g_obMissing);

	// Add some symbolic constants to the module   
	// pycom_Error = PyString_FromString("pythoncom.error");
	PyObject *pycom_Error = PyWinExc_COMError;
	if (pycom_Error == NULL || PyDict_SetItemString(dict, "error", pycom_Error) != 0)
	{
		PyErr_SetString(PyExc_MemoryError, "can't define error");
		return;
	}
	if (PyWinExc_COMError==NULL || PyDict_SetItemString(dict, "ole_error", PyWinExc_COMError) != 0)
	{
		PyErr_SetString(PyExc_MemoryError, "can't define ole_error");
		return;
	}
	// Add the same constant, but with a "new name"
	if (PyDict_SetItemString(dict, "com_error", PyWinExc_COMError) != 0)
	{
		PyErr_SetString(PyExc_MemoryError, "can't define com_error");
		return;
	}
	PyCom_InternalError = PyErr_NewException("pythoncom.internal_error", NULL, NULL);
	if (PyDict_SetItemString(dict, "internal_error", PyCom_InternalError) != 0)
	{
		PyErr_SetString(PyExc_MemoryError, "can't define internal_error");
		return;
	}

	// Add the IIDs
	if (PyCom_RegisterCoreIIDs(dict) != 0)
		return;

	// Setup our sub-modules
	if (!initunivgw(dict))
		return;

	// Add a few types.
	// NOTE - We do not autoduck these types, as they are for b/w compat only
	// New code should use the functions in pywintypes.
	PyDict_SetItemString(dict, "PyTimeType", (PyObject *)&PyTimeType);
	PyDict_SetItemString(dict, "PyIIDType", (PyObject *)&PyIIDType);
	PyDict_SetItemString(dict, "PyUnicodeType", (PyObject *)&PyUnicodeType);

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
#ifndef NO_PYCOM_IDISPATCHEX
	ADD_CONSTANT(DISPID_STARTENUM);
	ADD_CONSTANT(DISPID_UNKNOWN);
#endif
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

	// RPC
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_NONE); // RPC_C_AUTHN_LEVEL_NONE|Performs no authentication. 
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_CONNECT); // RPC_C_AUTHN_LEVEL_CONNECT|Authenticates only when the client establishes a relationship with the server. Datagram transports always use RPC_AUTHN_LEVEL_PKT instead. 
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_CALL); // RPC_C_AUTHN_LEVEL_CALL|Authenticates only at the beginning of each remote procedure call when the server receives the request. Datagram transports use RPC_C_AUTHN_LEVEL_PKT instead. 
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_PKT); // RPC_C_AUTHN_LEVEL_PKT|Authenticates that all data received is from the expected client. 
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_PKT_INTEGRITY); // RPC_C_AUTHN_LEVEL_PKT_INTEGRITY|Authenticates and verifies that none of the data transferred between client and server has been modified. 
	ADD_CONSTANT(RPC_C_AUTHN_LEVEL_PKT_PRIVACY); // RPC_C_AUTHN_LEVEL_PKT_PRIVACY|Authenticates all previous levels and encrypts the argument value of each remote procedure call. 

	ADD_CONSTANT(RPC_C_IMP_LEVEL_ANONYMOUS); // RPC_C_IMP_LEVEL_ANONYMOUS|(Not supported in this release.) The client is anonymous to the server. The server process cannot obtain identification information about the client and it cannot impersonate the client. 
	ADD_CONSTANT(RPC_C_IMP_LEVEL_IDENTIFY); // RPC_C_IMP_LEVEL_IDENTIFY|The server can obtain the client’s identity. The server can impersonate the client for ACL checking, but cannot access system objects as the client. This information is obtained when the connection is established, not on every call.<nl>Note  GetUserName will fail while impersonating at identify level. The workaround is to impersonate, OpenThreadToken, revert, call GetTokenInformation, and finally, call LookupAccountSid. 
	ADD_CONSTANT(RPC_C_IMP_LEVEL_IMPERSONATE); // RPC_C_IMP_LEVEL_IMPERSONATE|The server process can impersonate the client's security context while acting on behalf of the client. This information is obtained when the connection is established, not on every call. 
	ADD_CONSTANT(RPC_C_IMP_LEVEL_DELEGATE); // RPC_C_IMP_LEVEL_DELEGATE|(Not supported in this release.) The server process can impersonate the client's security context while acting on behalf of the client. The server process can also make outgoing calls to other servers while acting on behalf of the client. This information is obtained when the connection is established, not on every call. 

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

#ifndef NO_PYCOM_IDISPATCHEX
	ADD_CONSTANT(fdexNameCaseSensitive);  // Request that the name lookup be done in a case-sensitive manner. May be ignored by object that does not support case-sensitive lookup.  
	ADD_CONSTANT(fdexNameEnsure);  // Request that the member be created if it does not already exist. The new member should be created with the value VT_EMPTY.  
	ADD_CONSTANT(fdexNameImplicit);  // Indicates that the caller is searching object(s) for a member of a particular name, when the base object is not explicitly specified.  
	ADD_CONSTANT(fdexNameCaseInsensitive);  // Request that the name lookup be done in a case-insensitive manner. May be ignored by object that does not support case-insensitive lookup.  
	ADD_CONSTANT(fdexPropCanGet);  // The member can be obtained using DISPATCH_PROPERTYGET.  
	ADD_CONSTANT(fdexPropCannotGet);  // The member cannot be obtained using DISPATCH_PROPERTYGET.  
	ADD_CONSTANT(fdexPropCanPut);  // The member can be set using DISPATCH_PROPERTYPUT.  
	ADD_CONSTANT(fdexPropCannotPut);  // The member cannot be set using DISPATCH_PROPERTYPUT.  
	ADD_CONSTANT(fdexPropCanPutRef);  // The member can be set using DISPATCH_PROPERTYPUTREF.  
	ADD_CONSTANT(fdexPropCannotPutRef);  // The member cannot be set using DISPATCH_PROPERTYPUTREF.  
	ADD_CONSTANT(fdexPropNoSideEffects);  // The member does not have any side effects. For example, a debugger could safely get/set/call this member without changing the state of the script being debugged.  
	ADD_CONSTANT(fdexPropDynamicType);  // The member is dynamic and can change during the lifetime of the object.  
	ADD_CONSTANT(fdexPropCanCall);  // The member can be called as a method using DISPATCH_METHOD.  
	ADD_CONSTANT(fdexPropCannotCall);  // The member cannot be called as a method using DISPATCH_METHOD.  
	ADD_CONSTANT(fdexPropCanConstruct);  // The member can be called as a constructor using DISPATCH_CONSTRUCT.  
	ADD_CONSTANT(fdexPropCannotConstruct);  // The member cannot be called as a constructor using DISPATCH_CONSTRUCT.  
	ADD_CONSTANT(fdexPropCanSourceEvents);  // The member can fire events.  
	ADD_CONSTANT(fdexPropCannotSourceEvents);  // The member cannot fire events.  
#endif // NO_PYCOM_IDISPATCHEX

	// Expose the frozen flag, as Python itself doesnt!!
	// @prop int|frozen|1 if the host is a frozen program, else 0
	AddConstant(dict, "frozen", Py_FrozenFlag );

	// And finally some gross hacks relating to DCOM
	// Im really not sure what a better option is!
	//
	// If these #error pragma's fire it means this needs revisiting for
	// an upgrade to the MSVC header files!
	if (PyCom_HasDCom()) {
#		if  ((CLSCTX_ALL != (CLSCTX_INPROC_SERVER| CLSCTX_INPROC_HANDLER|CLSCTX_LOCAL_SERVER| CLSCTX_REMOTE_SERVER)) || \
		     (CLSCTX_SERVER != (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)))
#			error DCOM constants are not in synch.
#		endif
		ADD_CONSTANT(CLSCTX_ALL);
		ADD_CONSTANT(CLSCTX_SERVER);
		AddConstant(dict, "dcom", 1 );
	} else {
		AddConstant(dict, "CLSCTX_ALL", CLSCTX_INPROC_SERVER| CLSCTX_INPROC_HANDLER| CLSCTX_LOCAL_SERVER );
		AddConstant(dict, "CLSCTX_SERVER", CLSCTX_INPROC_SERVER| CLSCTX_LOCAL_SERVER );
		AddConstant(dict, "dcom", 0 );
	}
	// @prop int|dcom|1 if the system is DCOM aware, else 0.  Only Win95 without DCOM extensions should return 0

	// ### ALL THE @PROPERTY TAGS MUST COME AFTER THE LAST @PROP TAG!!
	// @property int|pythoncom|frozen|1 if the host is a frozen program, else 0
	// @property int|pythoncom|dcom|1 if the system is DCOM aware, else 0.  Only Win95 without DCOM extensions should return 0
}
