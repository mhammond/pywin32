// *sob* - a simple loader for pythoncomxx.dll - but this DLL has a
// manifest referencing the CRT whereas pythoncomxx.dll does not.
#include "windows.h"
#include "tchar.h"

// activation context stuff stolen from python...
typedef BOOL(WINAPI *PFN_GETCURRENTACTCTX)(HANDLE *);
typedef BOOL(WINAPI *PFN_ACTIVATEACTCTX)(HANDLE, ULONG_PTR *);
typedef BOOL(WINAPI *PFN_DEACTIVATEACTCTX)(DWORD, ULONG_PTR);
typedef BOOL(WINAPI *PFN_ADDREFACTCTX)(HANDLE);
typedef BOOL(WINAPI *PFN_RELEASEACTCTX)(HANDLE);

// locals and function pointers for this activation context magic.
static HANDLE PyWin_DLLhActivationContext = NULL;
static PFN_GETCURRENTACTCTX pfnGetCurrentActCtx = NULL;
static PFN_ACTIVATEACTCTX pfnActivateActCtx = NULL;
static PFN_DEACTIVATEACTCTX pfnDeactivateActCtx = NULL;
static PFN_ADDREFACTCTX pfnAddRefActCtx = NULL;
static PFN_RELEASEACTCTX pfnReleaseActCtx = NULL;
static HINSTANCE hinstThisModule = NULL;

void _LoadActCtxPointers()
{
    HINSTANCE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32)
        pfnGetCurrentActCtx = (PFN_GETCURRENTACTCTX)GetProcAddress(hKernel32, "GetCurrentActCtx");
    // If we can't load GetCurrentActCtx (ie, pre XP) , don't bother with the rest.
    if (pfnGetCurrentActCtx) {
        pfnActivateActCtx = (PFN_ACTIVATEACTCTX)GetProcAddress(hKernel32, "ActivateActCtx");
        pfnDeactivateActCtx = (PFN_DEACTIVATEACTCTX)GetProcAddress(hKernel32, "DeactivateActCtx");
        pfnAddRefActCtx = (PFN_ADDREFACTCTX)GetProcAddress(hKernel32, "AddRefActCtx");
        pfnReleaseActCtx = (PFN_RELEASEACTCTX)GetProcAddress(hKernel32, "ReleaseActCtx");
    }
}

ULONG_PTR _Py_ActivateActCtx()
{
    ULONG_PTR ret = 0;
    if (PyWin_DLLhActivationContext && pfnActivateActCtx)
        if (!(*pfnActivateActCtx)(PyWin_DLLhActivationContext, &ret)) {
            OutputDebugString("Pythoncomloader failed to activate the activation context before loading a DLL\n");
            ret = 0;  // no promise the failing function didn't change it!
        }
    return ret;
}

void _Py_DeactivateActCtx(ULONG_PTR cookie)
{
    if (cookie && pfnDeactivateActCtx)
        if (!(*pfnDeactivateActCtx)(0, cookie))
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
// Loads pythoncomxx.dll after activating our context and delegates the call to it.
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
            _LoadActCtxPointers();
            if (pfnGetCurrentActCtx && pfnAddRefActCtx)
                if ((*pfnGetCurrentActCtx)(&PyWin_DLLhActivationContext))
                    if (!(*pfnAddRefActCtx)(PyWin_DLLhActivationContext))
                        OutputDebugString("pythoncomloader failed to load the default activation context\n");
            hinstThisModule = hInst;
            break;

        case DLL_PROCESS_DETACH:
            if (pfnReleaseActCtx)
                (*pfnReleaseActCtx)(PyWin_DLLhActivationContext);
            break;
    }
    return TRUE;
}
