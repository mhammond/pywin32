// dllmain.

#include "stdafx.h"
#include "afxdllx.h"

#include "win32uiHostGlue.h"

static HWND GetConsoleHwnd(void);

HINSTANCE hWin32uiDll;  // Handle to this DLL.
static AFX_EXTENSION_MODULE extensionDLL;
static CDynLinkLibrary *pDLL = NULL;

BOOL PyWin_bHaveMFCHost = TRUE;  // indicates if the CWinApp was locally created.

extern BOOL bInFatalShutdown;
extern void Win32uiFinalize();

class CInProcApp : public CWinApp {
   public:
    CInProcApp(LPCTSTR lpszAppName);
    Win32uiHostGlue glue;
    void CleanupMainWindow();

   public:
    virtual BOOL InitInstance();
    virtual int ExitInstance()
    {
        glue.ExitInstance();  // ignore errors
        return CWinApp::ExitInstance();
    }
    DECLARE_MESSAGE_MAP()
   private:
    virtual BOOL PreTranslateMessage(MSG *pMsg)
    {
        if (glue.PreTranslateMessage(pMsg))
            return TRUE;
        else
            return CWinApp::PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle(LONG lCount)
    {
        // call base class idle first
        if (CWinApp::OnIdle(lCount))
            return TRUE;
        return glue.OnIdle(lCount);
    }
    BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
    {
        // yield to Python first.
        if (glue.OnCmdMsg(this, nID, nCode, pExtra, pHandlerInfo))
            return TRUE;
        else
            return CWinApp::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }
    // special idle handling to ignore WM_TIMER messages
    // (mainly for Scintilla until it uses WM_SYSTIMER messages)
    virtual BOOL IsIdleMessage(MSG *pmsg)
    {
        BOOL is = CWinApp::IsIdleMessage(pmsg);
        if (is)
            is = pmsg->message != WM_TIMER;
        return is;
    }

    BOOL m_bIsConsoleWindow;
};

static CInProcApp *pCreatedApp = NULL;

/////////////////////////////////////////////////////////////////////////////
// CInProcApp

BEGIN_MESSAGE_MAP(CInProcApp, CWinApp)
//{{AFX_MSG_MAP(CInProcApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInProcApp construction
CInProcApp::CInProcApp(LPCTSTR lpszAppName) : CWinApp(lpszAppName)
{
    PyWin_bHaveMFCHost = FALSE;
    m_bIsConsoleWindow = FALSE;  // Assume not until we find otherwise.
    glue.bShouldAbandonThreadState = FALSE;
    // Place all significant initialization in InitInstance
    // if I have a console window, make it the main window.
    HWND main = GetConsoleHwnd();
    if (main) {
        CWnd *pWndMain = new CWnd();
        pWndMain->Attach(main);
        m_pMainWnd = pWndMain;
        m_bIsConsoleWindow = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CInProcApp initialization

extern "C" PYW_EXPORT BOOL Win32uiApplicationInit(Win32uiHostGlue *pGlue, const TCHAR *cmd, const TCHAR *addnPaths);

BOOL CInProcApp::InitInstance()
{
    // Avoid dynamic search for Win32uiApplicationInit from inside DLL
    // if (!glue.DynamicApplicationInit())
    if (!Win32uiApplicationInit(&glue, NULL, NULL))
        return FALSE;
    return glue.InitInstance();
}

// Check that we have a valid CWinApp object to use.
bool CheckGoodWinApp()
{
    // shouldn't need special symbols now that we delay the creation.
    // If the host exports a special symbol, then
    // don't create a host app.
    //	HMODULE hModule = GetModuleHandle(NULL);
    //	BOOL hasSymbol = (GetProcAddress(hModule, "NoCreateWinApp") != NULL);
    if (AfxGetApp() == NULL) {  // && !hasSymbol) {
        // shared initialization
        pCreatedApp = new CInProcApp(_T("win32ui module"));

        // As we are looking for a WinApp, we are likely to be creating the
        // application object itself.  Trick MFC into thinking we are not
        // a DLL extension, but the app itself.
        AfxGetModuleState()->m_bDLL = 0;  // XXX - todo - expose this to Python???

        // Do the WinMain thang...
        // AFX internal initialization
        if (!AfxWinInit(hWin32uiDll, NULL, _T(""), SW_NORMAL))
            return 0;

        // App global initializations (rare)
        ASSERT_VALID(pCreatedApp);
        if (!pCreatedApp->InitApplication())
            return 0;

        // Perform specific initializations
        if (!pCreatedApp->InitInstance()) {
            pCreatedApp->CleanupMainWindow();
            pCreatedApp->ExitInstance();
            return 0;
        }
        ASSERT_VALID(pCreatedApp);
        if (AfxGetApp() == NULL)
            OutputDebugString(_T("Warning - still no CWinApp I can use!"));
    }
    return TRUE;
}

void CInProcApp::CleanupMainWindow()
{
    if (m_pMainWnd == NULL)
        return;

    if (m_bIsConsoleWindow) {
        Python_delete_assoc(m_pMainWnd);
        m_pMainWnd->Detach();
        delete m_pMainWnd;
        m_pMainWnd = NULL;
        m_bIsConsoleWindow = FALSE;
    }
    else {
        TRACE0("Warning: Destroying non-NULL m_pMainWnd\n");
        m_pMainWnd->DestroyWindow();
    }
}

#ifndef FREEZE_WIN32UI
extern "C" int __stdcall DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
#else
extern "C" __declspec(dllexport) int __stdcall DllMainwin32ui(HINSTANCE hInstance, DWORD dwReason, LPVOID)
#endif

{
    if (dwReason == DLL_PROCESS_ATTACH) {
        hWin32uiDll = hInstance;
        TCHAR path[_MAX_PATH];
        GetModuleFileName(hInstance, path, sizeof(path) / sizeof(TCHAR));
#ifndef FREEZE_WIN32UI
        // Normal win32ui.pyd initialization
#ifdef _DEBUG
        TRACE("Extension module %s initialising.\n", path);
#endif
        // Extension DLL one-time initialization
        if (!AfxInitExtensionModule(extensionDLL, hInstance))
            return 0;
        // insert into resource chain.
        pDLL = new CDynLinkLibrary(extensionDLL);

#else  // Frozen .EXE that embeds win32ui is initializing
        TRACE("win32ui in frozen %s initializing.\n", path);
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        //		Py_Cleanup();
        // NOT safe to cleanup here - other DLLs may have already been unloaded

        // From this point on, trying to do anything would be pretty serious!
        // (believe it or not, CoUninitialize() called after this point will
        // still manage to call into this DLL!!
        bInFatalShutdown = TRUE;

        if (pCreatedApp) {
            pCreatedApp->CleanupMainWindow();
            // We don't call ExitInstance, as the InitInstance we called could
            // not have possibly called back to Python, as the Python app object
            // could not have been created.  Let the Python code manage if it wants!
            Win32uiFinalize();
            AfxWinTerm();
            afxCurrentWinApp = NULL;  // So AfxGetApp fails from here.
            delete pCreatedApp;
            pCreatedApp = NULL;
        }
        // Only delete the Library if not already autodeleted by resource chain.
        AFX_MODULE_STATE *pafxmostState = AfxGetModuleState();
        if (NULL != pafxmostState && !pafxmostState->m_libraryList.IsEmpty()) {
            CDynLinkLibrary *pdynDll = pafxmostState->m_libraryList.GetHead();
            while (NULL != pdynDll && NULL != pDLL) {
                if (pdynDll == pDLL) {
                    delete pDLL;
                    pDLL = NULL;
                    break;
                }
                pdynDll = pafxmostState->m_libraryList.GetNext(pdynDll);
            }
        }
    }
    return 1;  // ok
}

// straight from the SDK.
HWND GetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024                   // buffer size for console window titles
    HWND hwndFound;                       // this is what is returned to the caller
    TCHAR pszNewWindowTitle[MY_BUFSIZE];  // contains fabricated WindowTitle
    TCHAR pszOldWindowTitle[MY_BUFSIZE];  // contains original WindowTitle

    // fetch current window title
    if (GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE) == 0)
        return NULL;

    // format a "unique" NewWindowTitle
    wsprintf(pszNewWindowTitle, _T("%d/%d"), GetTickCount(), GetCurrentProcessId());

    // change current window title
    SetConsoleTitle(pszNewWindowTitle);

    // ensure window title has been updated
    Sleep(40);

    // look for NewWindowTitle
    hwndFound = FindWindow(NULL, pszNewWindowTitle);

    // restore original window title
    SetConsoleTitle(pszOldWindowTitle);

    return (hwndFound);
}
