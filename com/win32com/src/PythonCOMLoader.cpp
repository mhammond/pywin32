// *sob* - a simple loader for pythoncomXX.dll - but this DLL has a
// manifest referencing the CRT whereas pythoncomXX.dll does not.
#include "windows.h"
#include "tchar.h"

// locals and function pointers for this activation context magic.
static HANDLE PyWin_DLLhActivationContext = NULL;
static HINSTANCE hinstThisModule = NULL;

ULONG_PTR _Py_ActivateActCtx()
{
    ULONG_PTR ret = 0;
    if (PyWin_DLLhActivationContext && !ActivateActCtx(PyWin_DLLhActivationContext, &ret)) {
        OutputDebugString("Pythoncomloader failed to activate the activation context before loading a DLL\n");
        ret = 0;  // no promise the failing function didn't change it!
    }
    return ret;
}

void _Py_DeactivateActCtx(ULONG_PTR cookie)
{
    if (cookie && !DeactivateActCtx(0, cookie))
        OutputDebugString("Pythoncomloader failed to de-activate the activation context\n");
}

STDAPI DllCanUnloadNow(void)
{
    // pythoncom just unconditionally returns S_FALSE...
    return S_FALSE;
}

typedef HRESULT(STDAPICALLTYPE *PFNDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
PFNDllGetClassObject pfnDllGetClassObject = NULL;

/////////////////////////////////////////////////////////////////////////////
// Loads pythoncomXX.dll after activating our context and delegates the call to it.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    if (pfnDllGetClassObject == 0) {
        // before loading pythoncom we must activate our context so
        // the CRT loads correctly.
        HMODULE hpycom = NULL;
        ULONG_PTR cookie = _Py_ActivateActCtx();
        TCHAR loader_path[MAX_PATH];
        if (GetModuleFileName(hinstThisModule, loader_path, MAX_PATH) != 0) {
            TCHAR fullpath[MAX_PATH];
            TCHAR *filepart;
            if (GetFullPathName(loader_path, MAX_PATH, fullpath, &filepart) != 0 && filepart != NULL) {
                if (_tcslen(DLL_DELEGATE) + _tcslen(loader_path) < sizeof(fullpath) / sizeof(fullpath[0])) {
                    _tcscpy(filepart, DLL_DELEGATE);
                    hpycom = LoadLibraryEx(fullpath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
                }
            }
        }
        if (hpycom == NULL)
            hpycom = LoadLibrary(DLL_DELEGATE);
        _Py_DeactivateActCtx(cookie);
        if (hpycom) {
            pfnDllGetClassObject = (PFNDllGetClassObject)GetProcAddress(hpycom, _T("DllGetClassObject"));
        }
    }
    if (pfnDllGetClassObject == 0) {
        return E_UNEXPECTED;
    }
    return (*pfnDllGetClassObject)(rclsid, riid, ppv);
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // capture our activation context for use when loading pythoncom
            if (GetCurrentActCtx(&PyWin_DLLhActivationContext) && !AddRefActCtx(PyWin_DLLhActivationContext))
                OutputDebugString("pythoncomloader failed to load the default activation context\n");
            hinstThisModule = hInst;
            break;

        case DLL_PROCESS_DETACH:
            ReleaseActCtx(PyWin_DLLhActivationContext);
            break;
    }
    return TRUE;
}
