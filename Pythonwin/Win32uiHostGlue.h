// Win32uiHostGlue.h : Defines a connection between win32ui and its
// application object.

// Sometimes I break this at the binary level - ie, all components must
// be in synch!  Use a version number to check this.
#define WIN32UIHOSTGLUE_VERSION 3

class Win32uiHostGlue : public CObject {
public:
	Win32uiHostGlue();
	~Win32uiHostGlue();

#ifndef LINK_WITH_WIN32UI
	// This will dynamically attach to win32ui.pyd.
	BOOL DynamicApplicationInit(const char *cmd = NULL, const char *additionalPaths = NULL);
#else
	BOOL ApplicationInit(const char *cmd = NULL, const char *additionalPaths = NULL);
#endif
	// placeholder in case application want to provide custom status text.
	virtual void SetStatusText(const char * /*cmd*/, int /*bForce*/) {return;}
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
	BOOL InitInstance() 
		{return pfnInitInstance ? (*pfnInitInstance)() : FALSE;}
	int ExitInstance(void) \
		{return pfnExitInstance ? (*pfnExitInstance)() : -1;}
	BOOL OnCmdMsg(CCmdTarget *pT, UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo ) \
		{return pfnOnCmdMsg ? (*pfnOnCmdMsg)(pT, nID, nCode, pExtra, pHandlerInfo) : FALSE;}	
	BOOL PreTranslateMessage(MSG *pMsg) \
		{return pfnPreTranslateMessage ? (*pfnPreTranslateMessage)(pMsg) : FALSE;}
	BOOL OnIdle( LONG lCount ) \
		{return pfnOnIdle ? (*pfnOnIdle)(lCount) : FALSE;}

	// This can be used as the main application "Run" method
	// if you want Python to have this level of control.
	int Run()
		{ return pfnRun ? (*pfnRun)() : -1;}

	// Must be the last thing called, ever!
	void ApplicationFinalize()
		{if (pfnFinalize) (*pfnFinalize)();}


	// some helpers for this class.
	HKEY GetRegistryRootKey();

	// function pointers.
	BOOL (*pfnInitInstance)();
	int (*pfnExitInstance)(void);
	BOOL (*pfnOnCmdMsg)(CCmdTarget *, UINT, int, void*, AFX_CMDHANDLERINFO* );
	BOOL (*pfnPreTranslateMessage)(MSG *pMsg);
	BOOL (*pfnOnIdle)( LONG lCount );
	int (*pfnRun)();
	void (*pfnFinalize)();
	bool bShouldFinalizePython; // Should win32ui shut down Python?
	bool bShouldAbandonThreadState; // Should win32ui abandon the thread state as it initializes?
	int versionNo; // version ID of the creator of the structure.
	bool bDebugBuild; // If the creator of the structure in a debug build?
	bool bWantStatusBarText; // The app should want this if it wants to override the status bar.

};

inline 	Win32uiHostGlue::Win32uiHostGlue()
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
	bShouldAbandonThreadState = true; // Depends on how embedded.
	bWantStatusBarText = false; // We can handle it by default.
	bDebugBuild = 
#ifdef _DEBUG
		true;
#else
		false;
#endif
}
inline 	Win32uiHostGlue::~Win32uiHostGlue()
{
}

inline HKEY Win32uiHostGlue::GetRegistryRootKey()
{
	// different for win32s.
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx(&ver);
	return ver.dwPlatformId == VER_PLATFORM_WIN32s ? HKEY_CLASSES_ROOT : HKEY_LOCAL_MACHINE;
}


#ifndef LINK_WITH_WIN32UI
inline BOOL Win32uiHostGlue::DynamicApplicationInit(const char *cmd, const char *additionalPaths)
{
#ifdef _DEBUG
	char *szWinui_Name = "win32ui_d.pyd";
#else
	char *szWinui_Name = "win32ui.pyd";
#endif
	HMODULE hModWin32ui = LoadLibrary(szWinui_Name);
	if (hModWin32ui==NULL) {
		char buf[256];
		sprintf(buf,"The application can not locate %s (%d)\n", szWinui_Name, GetLastError()); 
		int len = strlen(buf);
		int bufLeft = sizeof(buf) - len;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
			MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), buf+len, bufLeft, NULL);
		AfxMessageBox(buf);
		return FALSE;
	}
	HMODULE hModCore = NULL;
	for (int i=15;i<40;i++) {
		char fname[20];
#ifdef _DEBUG
		wsprintf(fname, "Python%d_d.dll", i);
#else
		wsprintf(fname, "Python%d.dll", i);
#endif
		hModCore = GetModuleHandle(fname);
		if (hModCore)
			break;
		// No point searching for 1.6->2.0!
		if (i==15)
			i = 19;
	}
	if (hModCore==NULL) {
		AfxMessageBox("Can not locate the Python DLL");
		return FALSE;
	}

	// Now the modules are loaded, call the Python init functions.
	int (__cdecl *pfnIsInit)(void);
	pfnIsInit = (int (__cdecl *)(void))GetProcAddress(hModCore, "Py_IsInitialized");
	BOOL bShouldInitPython;
	if (pfnIsInit)
		bShouldFinalizePython = bShouldInitPython = !(*pfnIsInit)();
	else {
		bShouldFinalizePython = FALSE; // Dont cleanup if we cant tell (this wont happen - Im paranoid :-)
		bShouldInitPython = TRUE;
	}

	void (__cdecl *pfnPyInit)(void);
	pfnPyInit = (void (__cdecl *)(void))GetProcAddress(hModCore, "Py_Initialize");
	if (pfnPyInit && bShouldInitPython) {
		(*pfnPyInit)();
	}

	BOOL (__cdecl *pfnWin32uiInit)(Win32uiHostGlue *, char *, const char *);

	pfnWin32uiInit = (BOOL (__cdecl *)(Win32uiHostGlue *, char *, const char *))GetProcAddress(hModWin32ui, "Win32uiApplicationInit");
	BOOL rc;
	if (pfnWin32uiInit)
		rc = (*pfnWin32uiInit)(this, (char *)cmd, additionalPaths);
	else {
		OutputDebugString("WARNING - win32uiHostGlue could not load the entry point for ApplicationInit\n");
		rc = FALSE;
	}
	// We must not free the win32ui module, as we
	// still hold function pointers to it!
	return rc;
}
#else

extern "C" __declspec(dllimport) BOOL Win32uiApplicationInit(Win32uiHostGlue *pGlue, char *cmd, const char *addnPaths);
extern "C" void initwin32ui();

inline BOOL Win32uiHostGlue::ApplicationInit(const char *cmd, const char *additionalPaths)
{
	if (!Py_IsInitialized()) {
		bShouldFinalizePython = TRUE;
		Py_Initialize();
	}
	// Make sure the statically linked win32ui is the one Python sees
	// (and doesnt go searching for a new one)
	initwin32ui();
	return Win32uiApplicationInit(this, (char *)cmd,additionalPaths);
}

#endif