// Connect.cpp : Implementation of DLL Exports.

// You will need the NT SUR Beta 2 SDK or VC 4.2 in order to build this
// project.  This is because you will need MIDL 3.00.15 or higher and new
// headers and libs.  If you have VC 4.2 installed, then everything should
// already be configured correctly.

#include "preconn.h"
#include "connres.h"
#include "initguid.h"
#include "PyCOMTest.h"
#include "PyCOMImpl.h"
#include "SimpleCounter.h"
#include "DSCArrayTest.h"

#define IID_DEFINED
#include "PyCOMTest_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CoPyCOMTest, CPyCOMTest)
OBJECT_ENTRY(CLSID_CoPyCOMTest2, CPyCOMTest2)
OBJECT_ENTRY(CLSID_CoSimpleCounter, CSimpleCounter)
OBJECT_ENTRY(CLSID_ArrayTest, DSCArrayTest)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;  // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void) { return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE; }

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}
