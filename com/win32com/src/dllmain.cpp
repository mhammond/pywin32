// PythonCOM.cpp : Implementation of DLL Exports.

#include "stdafx.h"

#include <windows.h>

#include <Python.h>
#include <PythonRun.h>	/* for Py_Initialize() */
#include <Import.h>		/* for PyImport_GetModuleDict() */

#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyFactory.h"

extern void FreeGatewayModule(void);


/*
** This value counts the number of references to objects that contain code
** within this DLL.  The DLL cannot be unloaded until this reaches 0.
**
** Additional locks (such as via the LockServer method on a CPyFactory) can
** add a "pseudo-reference" to ensure the DLL is not tossed.
*/
static LONG g_cLockCount = 0;
static BOOL bDidInitPython = FALSE;
static PyThreadState *ptsGlobal = NULL;

/*
** To support servers in .EXE's, PythonCOM allows you to register a threadID.
** A WM_QUIT message will be posted this thread when the external locks on the 
** objects hits zero.
*/
static DWORD dwQuitThreadId = 0;
PYCOM_EXPORT void PyCom_EnableQuitMessage( DWORD threadId )
{
	dwQuitThreadId = threadId;
}

void PyCom_DLLAddRef(void)
{
	// Must be thread-safe, although cant have the Python lock!
	CEnterLeaveFramework _celf;
	LONG cnt = InterlockedIncrement(&g_cLockCount);
	if (cnt==1) { // First call 
		if (!Py_IsInitialized()) {
			Py_Initialize();
			// Make sure our Windows framework is all setup.
			PyWinGlobals_Ensure();
			// Make sure we have _something_ as sys.argv.
			if (PySys_GetObject("argv")==NULL) {
				PyObject *path = PyList_New(0);
				PyObject *str = PyString_FromString("");
				PyList_Append(path, str);
				PySys_SetObject("argv", path);
				Py_XDECREF(path);
				Py_XDECREF(str);
			}

			// Must force Python to start using thread locks, as
			// we are free-threaded (maybe, I think, sometimes :-)
			PyEval_InitThreads();
//			PyWinThreadState_Ensure();
			// Release Python lock, as first thing we do is re-get it.
			ptsGlobal = PyEval_SaveThread();
			bDidInitPython=TRUE;
			// NOTE: We no longer ever finalize Python!!
		}
	}
}
void PyCom_DLLReleaseRef(void)
{
	/*** NOTE: We no longer finalize Python EVER in the COM world
	     see pycom-dev mailing list archives from April 2000 for why
	***/
	// Must be thread-safe, although cant have the Python lock!
// only needed when we finalize.
//	CEnterLeaveFramework _celf;
	LONG cnt = InterlockedDecrement(&g_cLockCount);
	// Not optimal, but anything better is hard - g_cLockCount
	// could always transition 1->0->1 at some stage, screwing this
	// up.  Oh well...
	if (cnt==0) {
		// Send a quit message to the registered thread (if we have one)
		if (dwQuitThreadId)
			PostThreadMessage(dwQuitThreadId, WM_QUIT, 0, 0);
/*** Old finalize code
		if (bDidInitPython) {
			PyEval_RestoreThread(ptsGlobal);
			PyWinGlobals_Free();
			FreeGatewayModule();
			Py_Finalize();

			bDidInitPython=FALSE;
		}
***/
	}
}

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

static DWORD g_dwCoInitThread = 0;
static BOOL g_bCoInitThreadHasInit = FALSE;

/* declare this outside of DllMain which has "C" scoping */
extern int PyCom_RegisterCoreSupport(void);
extern int PyCom_UnregisterCoreSupport(void);

#ifndef BUILD_FREEZE
#define DLLMAIN DllMain
#define DLLMAIN_DECL
#else
#define DLLMAIN DllMainpythoncom
#define DLLMAIN_DECL __declspec(dllexport)
#endif

#ifndef MS_WINCE
extern "C" DLLMAIN_DECL
BOOL WINAPI DLLMAIN(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
#else
DLLMAIN_DECL
BOOL WINAPI DLLMAIN(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
#endif
{
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
//		LogEvent("Loaded pythoncom.dll");

		/*
		** NOTE: we assume that our globals are not shared among processes,
		**       so for all intents and purposes, we can only hit this body
		**       of code "once" (from the standpoint of what our variables
		**       tell us).
		*/

		/*
		** NOTE: PythonCOM.DLL is now linked against pywintypes.dll
		**
		** pywintypes.dll's DLLMain() will be executed before us, and it
		** needs to initialise and call Python.  Thus is is never necessary for
		** us to initialise Python.
		*/
		PyCom_RegisterCoreSupport();
		/*
		** we don't need to be notified about threads
		*/
#ifndef MS_WINCE /* but CE doesnt seem to support it ?! */
		DisableThreadLibraryCalls(hInstance);
#endif
	}
	else if ( dwReason == DLL_PROCESS_DETACH )
	{
//		LogEvent("Terminated pythoncom.dll");

		{
//			CEnterLeavePython celp;
			/* free the gateway module if loaded (see PythonCOMObj.cpp) */

//			(void)PyCom_UnregisterCoreSupport();
		}
		// Call our helper to do smart Uninit of OLE.
// XXX - this seems to regularly hang - probably because it is being
// called from DllMain, and therefore threading issues get in the way!
//		PyCom_CoUninitialize();
	}

	return TRUE;    // ok
}

// Wierd problems with optimizer.  When anyone
// calls this, things go very strange.  Debugger indicates
// all params are wierd etc..  
// Only release mode, of course :-(  Dunno what optimization!
// Compiler version 11.00.7022
#pragma optimize ("", off)


// Some clients or COM extensions (notably MAPI) are _very_
// particular about the order of shutdown - in MAPI's case, you MUST
// do the CoUninit _before_ the MAPIUninit.
// These functions have logic so the Python programmer can call either
// the Init for Term function explicitely, and the framework will detect
// it no longer needs doing.
// XXX - Needs more thought about threading implications.
HRESULT PyCom_CoInitializeEx(LPVOID reserved, DWORD dwInit)
{
	// Must be thread-safe, although doesnt need the Python lock.
	CEnterLeaveFramework _celf;
	if (g_bCoInitThreadHasInit && g_dwCoInitThread == GetCurrentThreadId())
		return S_OK;
#ifndef MS_WINCE
	// Do a LoadLibrary, as the Ex version may not always exist
	// on Win95.
	HMODULE hMod = GetModuleHandle("ole32.dll");
	if (hMod==0) return E_HANDLE;
	FARPROC fp = GetProcAddress(hMod, "CoInitializeEx");
	if (fp==NULL) return E_NOTIMPL;

	HRESULT (*mypfn)(void *pvReserved, DWORD dwCoInit);
	mypfn = (HRESULT (*)(void *pvReserved, DWORD dwCoInit))fp;

	HRESULT hr = (*mypfn)(reserved, dwInit);
#else // Windows CE _only_ has the Ex version!
	HRESULT hr = CoInitializeEx(reserved, dwInit);
#endif // MS_WINCE

	if ( (hr != RPC_E_CHANGED_MODE) && FAILED(hr) )
	{
#ifdef _DEBUG
		PyCom_LogError(_T("OLE initialization failed! (0x%08lx)"), hr);
#endif
		return hr;
	}
	// If we have never been initialized before, then consider this
	// thread our "main initializer" thread.
	if (g_dwCoInitThread==0 && hr == S_OK) {
		g_dwCoInitThread = GetCurrentThreadId();
		g_bCoInitThreadHasInit = TRUE;
	}
	return hr;
}
#pragma optimize ("", on)


HRESULT PyCom_CoInitialize(LPVOID reserved)
{
	// Must be thread-safe, although doesnt need the Python lock.
	CEnterLeaveFramework _celf;
	// If our "main" thread has ever called this before, just
	// ignore it.  If it is another thread, then that thread
	// must manage itself.
	if (g_bCoInitThreadHasInit && g_dwCoInitThread == GetCurrentThreadId())
		return S_OK;
#ifndef MS_WINCE
	HRESULT hr = CoInitialize(reserved);
#else // Windows CE _only_ has the Ex version, and only multi-threaded!
	HRESULT hr = CoInitializeEx(reserved, COINIT_MULTITHREADED);
#endif // MS_WINCE
	if ( (hr != RPC_E_CHANGED_MODE) && FAILED(hr) )
	{
#ifdef _DEBUG
		PyCom_LogError(_T("OLE initialization failed! (0x%08lx)"), hr);
#endif
		return hr;
	}
	// If we have never been initialized before, then consider this
	// thread our "main initializer" thread.
	if (g_dwCoInitThread==0 && hr == S_OK) {
		g_dwCoInitThread = GetCurrentThreadId();
		g_bCoInitThreadHasInit = TRUE;
	}
	return hr;
}

void PyCom_CoUninitialize()
{
	// Must be thread-safe, although doesnt need the Python lock.
	CEnterLeaveFramework _celf;
	if (g_dwCoInitThread == GetCurrentThreadId()) {
		// being asked to terminate on our "main" thread
		// Check our flag, but always consider it success.
		if (g_bCoInitThreadHasInit) {
			CoUninitialize();
			g_bCoInitThreadHasInit = FALSE;
		}
	} else {
		// Not our thread - assume caller knows what they are doing
		CoUninitialize();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	// If we dont finalize Python, we should never unload!
	return S_FALSE;
//	return g_cLockCount ? S_FALSE : S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	//PyCom_StreamMessage("in DllGetClassObject\n");

	if ( ppv == NULL )
		return E_INVALIDARG;
	if ( !IsEqualIID(riid, IID_IUnknown) &&
		 !IsEqualIID(riid, IID_IClassFactory) )
		return E_INVALIDARG;

	// ### validate that we support rclsid?

	/* Put the factory right into *ppv; we know it supports <riid> */
	*ppv = (LPVOID*) new CPyFactory(rclsid);
	if ( *ppv == NULL )
		return E_OUTOFMEMORY;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Auto Registration Stuff
//   fileName is as passed to regsvr32
//   argc and argv are what Python should see as sys.argv
HRESULT DoRegisterUnregister(LPCSTR fileName, int argc, char **argv)
{
#ifdef MS_WINCE
	FILE *fp = Py_fopen(fileName, "r");
#else
	FILE *fp = fopen(fileName, "r");
#endif
	if (fp==NULL)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	HRESULT hr = S_OK;
	// Let the existing COM framework manage the Python state for us!
	PyCom_DLLAddRef();
	{ // A scope for _celp
		CEnterLeavePython _celp;

		PySys_SetArgv(argc, argv);
	
		if (PyRun_SimpleFile(fp, (char *)fileName) != 0) {
			// Convert the Python error to a HRESULT.
			hr = PyCom_SetCOMErrorFromPyException();
		}
	} // End scope.
#ifdef MS_WINCE
	Py_fclose(fp);
#else
	fclose(fp);
#endif
	PyCom_DLLReleaseRef();

	return hr;
}

extern "C" __declspec(dllexport) HRESULT DllRegisterServerEx(LPCSTR fileName)
{
	char *argv[] = { "regsvr32.exe" };
	return DoRegisterUnregister(fileName, 1, argv);
}

extern "C" __declspec(dllexport) HRESULT DllUnregisterServerEx(LPCSTR fileName)
{
	char *argv[] = { "regsvr32.exe", "--unregister" };
	return DoRegisterUnregister(fileName, 2, argv);
}