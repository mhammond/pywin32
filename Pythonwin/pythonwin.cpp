// pythonwin.cpp : Defines the class behaviors for the application.
//

#include "stdafxpw.h"
#include "pythonwin.h"
#include "win32uiHostGlue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////
// CPythonWinApp

BEGIN_MESSAGE_MAP(CPythonWinApp, CWinApp)
//{{AFX_MSG_MAP(CPythonWinApp)
//}}AFX_MSG_MAP
// Standard file based document commands
// Standard print setup command
ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPythonWinApp construction

CPythonWinApp::CPythonWinApp()
{
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPythonWinApp object

CPythonWinApp NEAR theApp;

// The one and only Glue object.
Win32uiHostGlue NEAR glue;

/////////////////////////////////////////////////////////////////////////////
// CPythonWinApp initialization
BOOL CPythonWinApp::InitApplication()
{
    // create mem mapped file for switching instances.
    m_pDocManager = new CDocManager();
    if (!CWinApp::InitApplication())
        return FALSE;
    CString startup;
    startup.LoadString(57346);  // Grrr....
    if (startup.GetLength() == 0)
        startup = "import pywin.framework.startup";

    if (!glue.DynamicApplicationInit(startup))
        return FALSE;
    return TRUE;
}

BOOL CPythonWinApp::InitInstance()
{
    if (!glue.InitInstance())
        return FALSE;
    // dialog based apps dont have a message pump.
    return m_pMainWnd && !m_pMainWnd->IsKindOf(RUNTIME_CLASS(CDialog));
}
int CPythonWinApp::ExitInstance()
{
    int rc = glue.ExitInstance();
    CWinApp::ExitInstance();
    return rc;
}

// special idle handling to ignore WM_TIMER messages
// (mainly for Scintilla until it uses WM_SYSTIMER messages)
BOOL CPythonWinApp::IsIdleMessage(MSG *pmsg)
{
    BOOL is = CWinApp::IsIdleMessage(pmsg);
    if (is)
        is = pmsg->message != WM_TIMER;
    return is;
}

BOOL CPythonWinApp::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
    // yield to Python first
    if (glue.OnCmdMsg(this, nID, nCode, pExtra, pHandlerInfo))
        return TRUE;
    else
        return CWinApp::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CPythonWinApp::PreTranslateMessage(MSG *pMsg)
{
    if (glue.PreTranslateMessage(pMsg))
        return TRUE;
    else
        return CWinApp::PreTranslateMessage(pMsg);

    /*	BOOL ret=CWinApp::PreTranslateMessage(pMsg);
        BOOL ret2 = glue.PreTranslateMessage(pMsg);
        return ret||ret2;
    */
}

BOOL CPythonWinApp::OnIdle(LONG lCount)
{
    // call base class idle first
    if (CWinApp::OnIdle(lCount))
        return TRUE;
    return glue.OnIdle(lCount);
}

CDocument *CPythonWinApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
#if 0  // win32s no longer supported
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx(&ver);
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx(&ver);
	if (ver.dwPlatformId == VER_PLATFORM_WIN32s) {
		OutputDebugString("Win32s - Searching templates!\n");
		POSITION posTempl = m_pDocManager->GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(posTempl);
		if (pTemplate)
			return pTemplate->OpenDocumentFile(lpszFileName);
		else {
			AfxMessageBox("win32s error - There is no template to use");
			return NULL;
		}
	} else
#endif
    return CWinApp::OpenDocumentFile(lpszFileName);
}

int CPythonWinApp::Run()
{
    // Allow our Python app to override the run!
    int rc = glue.Run();
    glue.ApplicationFinalize();
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
// CPythonWinApp commands
