// pythonwin.h : main header file for the PYTHONWIN application
//
#ifndef __PYTHONWIN_H__
#define __PYTHONWIN_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonWinApp:
// See pythonwin.cpp for the implementation of this class
//

class CPythonWinApp : public CWinApp
{
public:
	CPythonWinApp();
	void SetStatusText(const char *szStatus, BOOL bForce=FALSE);

protected:
	BOOL OnCmdMsg (UINT nID, int nCode,
		       void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo);

private:
// Overrides
	virtual CDocument *CPythonWinApp::OpenDocumentFile(LPCTSTR lpszFileName);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL InitInstance();
	virtual BOOL InitApplication();
	virtual int ExitInstance();
	virtual BOOL OnIdle( LONG );
	virtual int Run(void);
	virtual BOOL IsIdleMessage(MSG *pmsg);

// Implementation
//	CPtrList idleHookList;
//	int myIdleCtr;
//	CString lastFile;
public:	// give access to message map.
	//{{AFX_MSG(CPythonWinApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif // __filename_h__
