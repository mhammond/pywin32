
//
// Python Thread object
//
class PyCWinThread : public PyCCmdTarget {
protected:
	PyCWinThread();
	~PyCWinThread();
public:
	static PyObject *create(PyObject *self, PyObject *args);
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCWinThread)
};

//
// Application Object.
//

class PyCWinApp : public PyCWinThread {
protected:
	PyCWinApp();
	~PyCWinApp();
public:
	static PyObject *pExistingAppObject;

	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCWinApp)
	static void cleanup();
};


/////////////////////////////////////////////////////////////////////
//
// Hack Application objects
//
// These objects are purely to get access to protected members.
// It is never instantiated.  Therefore, it must not have virtual
// functions or data items.
// It is used purely so C++ casts can override protection.
class CProtectedDocManager : public CDocManager 
{
public:
	CPtrList &GetTemplateList() {return m_templateList;}
};

class PYW_EXPORT CProtectedWinApp : public CWinApp {
public:
	// how do I change from protected to public?
	CString GetRecentFileName(int index);
	void RemoveRecentFile(int index);
	// Get main window - usually (but not always!) a CMDIFrameWnd
	CWnd *GetMainFrame () {return m_pMainWnd;}
	void SetMainFrame (CWnd *pWnd) {m_pMainWnd = pWnd;}
	CDocument *FindOpenDocument (const char *lpszFileName);
	BOOL Enable3dControls() {return CWinApp::Enable3dControls();}
	void SetDialogBkColor(COLORREF clrCtlBk, COLORREF clrCtlText) { CWinApp::SetDialogBkColor(clrCtlBk, clrCtlText);}
	BOOL HaveLoadStdProfileSettings() {return m_pRecentFileList!=NULL;}
	void LoadStdProfileSettings(UINT max) {CWinApp::LoadStdProfileSettings(max);}
	void SetRegistryKey(LPCTSTR key) {CWinApp::SetRegistryKey(key);}
	void OnFileNew(void) {CWinApp::OnFileNew();}
	void OnFileOpen(void) {CWinApp::OnFileOpen();}
	CProtectedDocManager *GetDocManager();
	PyObject *MakePyDocTemplateList(void);
};

class CProtectedWinThread : public CWinThread {
public:
	void PumpIdle();
	void PumpWaitingMessages(UINT firstMsg, UINT lastMsg);
	void PumpMessages();
};

inline CWinApp *GetApp() {CWinApp *ret = AfxGetApp(); if (ret==NULL) RETURN_ERR("There is no application object"); return ret;}
inline CProtectedWinApp *GetProtectedApp() {return (CProtectedWinApp *)GetApp();}
inline CProtectedWinThread *GetProtectedThread() {return (CProtectedWinThread *)GetApp();}
//////////////////////////////////////////////////////////////////////
