// Win32uiHostGlue.h : Defines a connection between win32ui and its
// application object.

// Sometimes I break this at the binary level - ie, all components must
// be in synch!  Use a version number to check this.
#define WIN32UIHOSTGLUE_VERSION 3

#include "pywintypes.h"

class Win32uiHostGlue : public CObject {
   public:
    Win32uiHostGlue();
    ~Win32uiHostGlue();

#ifndef LINK_WITH_WIN32UI
    // This will dynamically attach to win32ui.pyd.
    BOOL DynamicApplicationInit(const TCHAR *cmd = NULL, const TCHAR *additionalPaths = NULL);
#else
    BOOL ApplicationInit(const TCHAR *cmd = NULL, const TCHAR *additionalPaths = NULL);
#endif
    // placeholder in case application want to provide custom status text.
    virtual void SetStatusText(const TCHAR * /*cmd*/, int /*bForce*/) { return; }
    // Helper class, to register _any_ HMODULE as a module name.
    // This allows modules built into .EXE's, or in differently
    // named DLL's etc.  This requires admin priveliges on some machines, so
    // a program should not refuse to start if this fails, but calling it
    // each time means the app is guaranteed to work when moved.
    // REMOVED - See below!!!
    //	BOOL RegisterModule(HMODULE hModule, const char *moduleName);
    // or if you know the file name
    //	BOOL RegisterModule(const char *fileName, const char *moduleName);

    // These must be called by Host Application at the relevant time.
    BOOL InitInstance() { return pfnInitInstance ? (*pfnInitInstance)() : FALSE; }
    int ExitInstance(void) { return pfnExitInstance ? (*pfnExitInstance)() : -1; }
    BOOL OnCmdMsg(CCmdTarget *pT, UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
    {
        return pfnOnCmdMsg ? (*pfnOnCmdMsg)(pT, nID, nCode, pExtra, pHandlerInfo) : FALSE;
    }
    BOOL PreTranslateMessage(MSG *pMsg) { return pfnPreTranslateMessage ? (*pfnPreTranslateMessage)(pMsg) : FALSE; }
    BOOL OnIdle(LONG lCount) { return pfnOnIdle ? (*pfnOnIdle)(lCount) : FALSE; }

    // This can be used as the main application "Run" method
    // if you want Python to have this level of control.
    int Run() { return pfnRun ? (*pfnRun)() : -1; }

    // Must be the last thing called, ever!
    void ApplicationFinalize()
    {
        if (pfnFinalize)
            (*pfnFinalize)();
    }

    // some helpers for this class.
    HKEY GetRegistryRootKey();

    // function pointers.
    BOOL (*pfnInitInstance)();
    int (*pfnExitInstance)(void);
    BOOL (*pfnOnCmdMsg)(CCmdTarget *, UINT, int, void *, AFX_CMDHANDLERINFO *);
    BOOL (*pfnPreTranslateMessage)(MSG *pMsg);
    BOOL (*pfnOnIdle)(LONG lCount);
    int (*pfnRun)();
    void (*pfnFinalize)();
    bool bShouldFinalizePython;      // Should win32ui shut down Python?
    bool bShouldAbandonThreadState;  // Should win32ui abandon the thread state as it initializes?
    int versionNo;                   // version ID of the creator of the structure.
    bool bDebugBuild;                // If the creator of the structure in a debug build?
    bool bWantStatusBarText;         // The app should want this if it wants to override the status bar.
};

inline Win32uiHostGlue::Win32uiHostGlue()
{
    versionNo = WIN32UIHOSTGLUE_VERSION;
    pfnInitInstance = NULL;
    pfnExitInstance = NULL;
    pfnOnCmdMsg = NULL;
    pfnPreTranslateMessage = NULL;
    pfnOnIdle = NULL;
    pfnRun = NULL;
    pfnFinalize = NULL;
    bShouldFinalizePython = false;
    bShouldAbandonThreadState = true;  // Depends on how embedded.
    bWantStatusBarText = false;        // We can handle it by default.
    bDebugBuild =
#ifdef _DEBUG
        true;
#else
        false;
#endif
}
inline Win32uiHostGlue::~Win32uiHostGlue() {}

inline HKEY Win32uiHostGlue::GetRegistryRootKey()
{
    // different for win32s.
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);
    return ver.dwPlatformId == VER_PLATFORM_WIN32s ? HKEY_CLASSES_ROOT : HKEY_LOCAL_MACHINE;
}

#ifndef LINK_WITH_WIN32UI
inline BOOL Win32uiHostGlue::DynamicApplicationInit(const TCHAR *cmd, const TCHAR *additionalPaths)
{
#ifdef _DEBUG
    TCHAR *szWinui_Name = _T("win32ui_d.pyd");
#else
    TCHAR *szWinui_Name = _T("win32ui.pyd");
#endif
    // god damn - this all should die.
    // The problem is finding the correct win32ui.pyd and the correct
    // Python.
    // If we can get win32ui loaded, we can get Python from that.
    // Otherwise, we can try and find a Python.dll in various directories,
    // then try again.
    // Otherwise we give up in disgust.
    TCHAR app_dir[MAX_PATH];
    _tcscpy(app_dir, _T("\0"));
    GetModuleFileName(NULL, app_dir, sizeof(app_dir) / sizeof(TCHAR));
    TCHAR *p = app_dir + _tcslen(app_dir);
    while (p > app_dir && *p != '\\') p--;
    *p = '\0';

    TCHAR fname[MAX_PATH * 2];

    HMODULE hModCore = NULL;
    // There are 2 cases we care about:
    // * pythonwin.exe next to win32ui, in lib\site-packages\pythonwin
    // * pythonwin.exe next to python.exe, in sys.home - this is for
    //   older style installs and for custom layouts.
    // * a kind-of sub-case - handle the PCBuild directory
    TCHAR *py_dll_candidates[] = {
        _T("..\\..\\.."),  // lib\site-packages\pythonwin
#ifdef _M_X64
        _T("..\\..\\..\\PCBuild\\amd64"),
#else
        _T("..\\..\\..\\PCBuild"),
#endif
        // and relative to the root of the py dir.
        _T(""),
#ifdef _M_X64
        _T("PCBuild\\amd64"),
#else
        _T("PCBuild"),
#endif
    };
    TCHAR py_dll[20];
#ifdef _DEBUG
    wsprintf(py_dll, _T("Python%d%d_d.dll"), PY_MAJOR_VERSION, PY_MINOR_VERSION);
#else
    wsprintf(py_dll, _T("Python%d%d.dll"), PY_MAJOR_VERSION, PY_MINOR_VERSION);
#endif
    // try it simple - if we can load the module we are done.
    HMODULE hModWin32ui = LoadLibrary(szWinui_Name);
    if (hModWin32ui == NULL) {
        // try an installed version (old versions installed pythonwin.exe next
        // to python.exe - but we shouldn't get here if pythonwin.exe is next
        // to win32ui)
        wsprintf(fname, _T("%s\\%s\\%s"), app_dir, _T("lib\\site-packages\\pythonwin"), szWinui_Name);
        hModWin32ui = LoadLibrary(fname);
    }
    if (hModWin32ui == NULL) {
        // 2 main reasons we get here: can't load MFC, or can't load
        // Python itself.  We try and handle the latter now...
        int i;
        const int ncandidates = sizeof(py_dll_candidates) / sizeof(py_dll_candidates[0]);
        for (i = 0; i < ncandidates && hModCore == 0; i++) {
            wsprintf(fname, _T("%s\\%s\\%s"), app_dir, py_dll_candidates[i], py_dll);
            hModCore = LoadLibrary(fname);
        }
        if (hModCore) {
            hModWin32ui = LoadLibrary(szWinui_Name);
        }
    }
    else {
        hModCore = GetModuleHandle(py_dll);
        ASSERT(hModCore);  // loaded win32ui, how can I not have a handle to python?
    }
    if (!hModCore) {
        // No Python, no win32ui :(
        TCHAR buf[256];
        wsprintf(buf, _T("The application can not locate %s (or Python) (%d)\n"), szWinui_Name, GetLastError());
        Py_ssize_t len = _tcslen(buf);
        Py_ssize_t bufLeft = sizeof(buf) / sizeof(TCHAR) - len;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                      buf + len, PyWin_SAFE_DOWNCAST(bufLeft, Py_ssize_t, DWORD), NULL);
        AfxMessageBox(buf);
        return FALSE;
    }
    // Now the modules are loaded, call the Python init functions.
    int(__cdecl * pfnIsInit)(void);
    pfnIsInit = (int(__cdecl *)(void))GetProcAddress(hModCore, "Py_IsInitialized");
    BOOL bShouldInitPython;
    if (pfnIsInit)
        bShouldFinalizePython = bShouldInitPython = !(*pfnIsInit)();
    else {
        bShouldFinalizePython = FALSE;  // Dont cleanup if we cant tell (this wont happen - Im paranoid :-)
        bShouldInitPython = TRUE;
    }

    void(__cdecl * pfnPyInit)(void);
    pfnPyInit = (void(__cdecl *)(void))GetProcAddress(hModCore, "Py_Initialize");
    if (pfnPyInit && bShouldInitPython) {
        (*pfnPyInit)();
        void(__cdecl * pfnPyEval_InitThreads)(void);
        pfnPyEval_InitThreads = (void(__cdecl *)(void))GetProcAddress(hModCore, "PyEval_InitThreads");
        ASSERT(pfnPyEval_InitThreads);
        if (pfnPyEval_InitThreads)
            pfnPyEval_InitThreads();
    }

    if (!hModWin32ui) {  // sigh - try and import it
        int(__cdecl * pfnPyRun_SimpleString)(const char *);
        pfnPyRun_SimpleString = (int(__cdecl *)(const char *))GetProcAddress(hModCore, "PyRun_SimpleString");
        if (pfnPyRun_SimpleString)
            pfnPyRun_SimpleString("import win32ui");
        hModWin32ui = GetModuleHandle(szWinui_Name);
        if (!hModWin32ui)
            AfxMessageBox(_T("Still can't get my hands on win32ui"));
    }

    BOOL(__cdecl * pfnWin32uiInit)(Win32uiHostGlue *, TCHAR *, const TCHAR *);

    pfnWin32uiInit = (BOOL(__cdecl *)(Win32uiHostGlue *, TCHAR *, const TCHAR *))GetProcAddress(
        hModWin32ui, "Win32uiApplicationInit");
    BOOL rc;
    if (pfnWin32uiInit)
        rc = (*pfnWin32uiInit)(this, (TCHAR *)cmd, (TCHAR *)additionalPaths);
    else {
        OutputDebugString(_T("WARNING - win32uiHostGlue could not load the entry point for ApplicationInit\n"));
        rc = FALSE;
    }
    // We must not free the win32ui module, as we
    // still hold function pointers to it!
    return rc;
}
#else  // LINK_WITH_WIN32UI defined

extern "C" __declspec(dllimport) BOOL
    Win32uiApplicationInit(Win32uiHostGlue *pGlue, TCHAR *cmd, const TCHAR *addnPaths);
extern "C" void initwin32ui();

inline BOOL Win32uiHostGlue::ApplicationInit(const TCHAR *cmd, const TCHAR *additionalPaths)
{
    if (!Py_IsInitialized()) {
        bShouldFinalizePython = TRUE;
        Py_Initialize();
    }
    // Make sure the statically linked win32ui is the one Python sees
    // (and doesnt go searching for a new one)

    initwin32ui();
    return Win32uiApplicationInit(this, cmd, additionalPaths);
}

#endif