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

inline HKEY Win32uiHostGlue::GetRegistryRootKey() { return HKEY_LOCAL_MACHINE; }

#ifndef LINK_WITH_WIN32UI

#define CHECK_PFN(p)                                                        \
    if (!p) {                                                               \
        wsprintf(err_buf, _T("Failed to load ##p - %d\n"), GetLastError()); \
        goto fail_with_error_dlg;                                           \
    }

inline BOOL Win32uiHostGlue::DynamicApplicationInit(const TCHAR *cmd, const TCHAR *additionalPaths)
{
    // We need to load the `win32ui` module and get a symbol from it.
    // This is quite complex, but it used to be worse! The strategy we take is
    // to just have Python import win32ui - which is easier said than done as
    // we need to load and initialize Python first.
    TCHAR app_dir[MAX_PATH];
    _tcscpy(app_dir, _T("\0"));
    GetModuleFileName(NULL, app_dir, sizeof(app_dir) / sizeof(TCHAR));
    TCHAR *p = app_dir + _tcslen(app_dir);
    while (p > app_dir && *p != '\\') p--;
    *p = '\0';

    TCHAR fname[MAX_PATH * 2];

    HMODULE hModCore = NULL;
    HMODULE hModWin32ui = NULL;
    // There are 2 cases we care about:
    // * pythonwin.exe next to win32ui, in lib\site-packages\pythonwin
    // * pythonwin.exe next to python.exe, in sys.home - this is for
    //   older style installs and for custom layouts.
    // * a kind-of sub-case - handle the PCBuild directory
    TCHAR *py_dll_candidates[] = {
        _T("..\\..\\.."),  // lib\site-packages\pythonwin
#ifdef _M_X64
        _T("..\\..\\..\\PCBuild\\amd64"),
#elif defined(_M_ARM64)
        _T("..\\..\\..\\PCBuild\\arm64"),
#else
        _T("..\\..\\..\\PCBuild"),
#endif
        // and relative to the root of the py dir.
        _T(""),
#ifdef _M_X64
        _T("PCBuild\\amd64"),
#elif defined(_M_ARM64)
        _T("PCBuild\\arm64"),
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
    TCHAR err_buf[256];
    // It's critical Python is loaded *and initialized* before we load win32ui
    // as just loading win32ui will cause it to call into Python.
    hModCore = GetModuleHandle(py_dll);  // Check if Python is already loaded
    if (hModCore == NULL) {
        const int ncandidates = sizeof(py_dll_candidates) / sizeof(py_dll_candidates[0]);
        for (int i = 0; i < ncandidates && hModCore == NULL; i++) {
            wsprintf(fname, _T("%s\\%s\\%s"), app_dir, py_dll_candidates[i], py_dll);
            hModCore = LoadLibrary(fname);
        }
    }
    if (hModCore == NULL) {
        wsprintf(err_buf, _T("The application can not locate %s (%d)\n"), py_dll, GetLastError());
        goto fail_with_error_dlg;
    }

    // Now Python is loaded we can initialize it.
    int(__cdecl * pfnIsInit)(void);
    pfnIsInit = (int(__cdecl *)(void))GetProcAddress(hModCore, "Py_IsInitialized");
    BOOL bShouldInitPython;

    CHECK_PFN(pfnIsInit);

    bShouldFinalizePython = bShouldInitPython = !(*pfnIsInit)();

    if (bShouldInitPython) {
        void(__cdecl * pfnPyInit)(void);
        pfnPyInit = (void(__cdecl *)(void))GetProcAddress(hModCore, "Py_Initialize");
        CHECK_PFN(pfnPyInit);
        (*pfnPyInit)();
    }

    PyObject *(*pPyImport_ImportModule)(const char *name) =
        (PyObject * (*)(const char *name)) GetProcAddress(hModCore, "PyImport_ImportModule");
    CHECK_PFN(pPyImport_ImportModule);
    PyObject *(*pPyObject_GetAttrString)(PyObject *o, const char *attr_name) =
        (PyObject * (*)(PyObject * o, const char *attr_name)) GetProcAddress(hModCore, "PyObject_GetAttrString");
    CHECK_PFN(pPyObject_GetAttrString);
    Py_ssize_t (*pPyUnicode_AsWideChar)(PyObject *unicode, wchar_t *w, Py_ssize_t size) =
        (Py_ssize_t(*)(PyObject * unicode, wchar_t * w, Py_ssize_t size))
            GetProcAddress(hModCore, "PyUnicode_AsWideChar");
    CHECK_PFN(pPyUnicode_AsWideChar);

    PyObject *win32ui = pPyImport_ImportModule("win32ui");
    if (!win32ui) {
        wsprintf(err_buf, _T("Failed to import win32ui\n"));
        goto fail_with_error_dlg;
    }
    PyObject *pyfn = pPyObject_GetAttrString(win32ui, "__file__");
    if (!pyfn) {
        wsprintf(err_buf, _T("Failed to get __file__ from win32ui\n"));
        goto fail_with_error_dlg;
    }

    Py_ssize_t nchars = pPyUnicode_AsWideChar(pyfn, fname, MAX_PATH * 2);
    if (nchars == -1) {
        wsprintf(err_buf, _T("Failed to get unicode from __file__\n"));
        goto fail_with_error_dlg;
    }
    fname[nchars] = _T('\0');

    hModWin32ui = GetModuleHandleW(fname);

    BOOL(__cdecl * pfnWin32uiInit)(Win32uiHostGlue *, TCHAR *, const TCHAR *);

    pfnWin32uiInit = (BOOL(__cdecl *)(Win32uiHostGlue *, TCHAR *, const TCHAR *))GetProcAddress(
        hModWin32ui, "Win32uiApplicationInit");
    if (!pfnWin32uiInit) {
        wsprintf(err_buf, _T("Failed to load 'Win32uiApplicationInit' - %d\n"), GetLastError());
        goto fail_with_error_dlg;
    }
    // We must not free the win32ui module, as we
    // still hold function pointers to it!
    return (*pfnWin32uiInit)(this, (TCHAR *)cmd, (TCHAR *)additionalPaths);

fail_with_error_dlg:
    // Assumes err_buf has already had the "core" message, will then add
    // detailed error info from windows.
    Py_ssize_t len = _tcslen(err_buf);
    Py_ssize_t bufLeft = sizeof(err_buf) / sizeof(TCHAR) - len;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                  err_buf + len, PyWin_SAFE_DOWNCAST(bufLeft, Py_ssize_t, DWORD), NULL);
    AfxMessageBox(err_buf);
    return FALSE;
}
#else  // LINK_WITH_WIN32UI defined

extern "C" __declspec(dllimport) BOOL Win32uiApplicationInit(Win32uiHostGlue *pGlue, const TCHAR *cmd,
                                                             const TCHAR *addnPaths);
extern "C" void initwin32ui();

inline BOOL Win32uiHostGlue::ApplicationInit(const TCHAR *cmd, const TCHAR *additionalPaths)
{
    if (!Py_IsInitialized()) {
        bShouldFinalizePython = TRUE;
        Py_Initialize();
    }
    // Make sure the statically linked win32ui is the one Python sees
    // (and doesn't go searching for a new one)

    PyInit_win32ui();
    return Win32uiApplicationInit(this, cmd, additionalPaths);
}

#endif
