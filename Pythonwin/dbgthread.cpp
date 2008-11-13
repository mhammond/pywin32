/* Debugger code

	This creates a window with debugging options in a secondary thread.

*/
#include "stdafx.h"
#include "reswin32ui.h"

HWND hwndDebug = NULL; // The HWND of the main window for the thread.

void ProcessShellMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT cmdId = 0;
	switch (lParam) {
        case WM_LBUTTONUP:
			cmdId = ID_SHELL_ACTIVATE;
			break;
        case WM_RBUTTONUP: 
		{

			CMenu menu;
			menu.LoadMenu(MAKEINTRESOURCE(IDR_SHELLTRAY));
//			HMENU hMenu = AfxGetApp()->LoadMenu(MAKEINTRESOURCE(IDR_SHELLTRAY));
//			HWND hWndMain = AfxGetMainWnd()->GetSafeHwnd();
			HMENU hMenuTrackPopup = *menu.GetSubMenu (0); // convert to a HMENU
			POINT pt;
			GetCursorPos(&pt);
			// This is required when using a notify icon -- see KB article 
			// PRB: Menus for Notification Icons Don't Work Correctly 
			SetForegroundWindow (hWnd);
			SetMenuDefaultItem(hMenuTrackPopup, 0, MF_BYPOSITION);
			cmdId = TrackPopupMenu(hMenuTrackPopup, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

			// This is required when using a notify icon -- see KB article 
			// PRB: Menus for Notification Icons Don't Work Correctly 
			::PostMessage (hWnd, WM_USER, 0, 0);
			break;
		}
		break;
	}

	switch (cmdId) {
		case ID_SHELL_ACTIVATE: 
		{
			HWND hwndMain = AfxGetMainWnd()->GetSafeHwnd();
			BOOL ok = (hwndMain != NULL);
			if (ok)
				::SetForegroundWindow(hwndMain);
			if (ok) {
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				ok = ::GetWindowPlacement(hwndMain, &wp);
				if (ok && wp.showCmd==SW_SHOWMINIMIZED)
					::ShowWindow(hwndMain, SW_RESTORE);
			}
			break;
		}
		case ID_SHELL_BREAK:
			// set a flag to tell the process to break;
			PyErr_SetInterrupt();
			break;
		default:
			break;
	}
}

static void AddIcons(HWND hwndDebug)
{
	HICON hIcon = AfxGetApp()->LoadIcon( MAKEINTRESOURCE(IDR_MAINFRAME) );
	DWORD flags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA), hwndDebug, 0, flags, WM_USER+20, hIcon };
	_tcscpy(nid.szTip, _T("Pythonwin"));
	Shell_NotifyIcon(NIM_ADD, &nid);
}

LRESULT CALLBACK DebuggerWndProc( HWND hWnd, UINT msg, WPARAM wParam,
   LPARAM lParam )
{
    static UINT s_uTaskbarRestart;
    switch( msg ) {
      case WM_CREATE:
         s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
         break;
      case WM_COMMAND:
         break;
      case WM_DESTROY:
         PostQuitMessage( 0 );
         break;
      case WM_USER+20:
		  ProcessShellMessage(hWnd, msg, wParam, lParam);
		  break;
      case WM_USER+21:
		  DestroyWindow(hWnd);
		  break;
/**************************************************************\
*     Let the default window proc handle all other messages    *
\**************************************************************/
      default:
          if(msg==s_uTaskbarRestart)
              AddIcons(hWnd);
         return( DefWindowProc( hWnd, msg, wParam, lParam ));
   }
   return 0;
}


void StopDebuggerThread()
{
	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA), hwndDebug, 0 };
	Shell_NotifyIcon(NIM_DELETE, &nid);
	::PostMessage(hwndDebug, WM_USER+21, 0, 0);
}

DWORD DebuggerThreadFunc( LPDWORD lpdwWhatever )
{
	MSG msg;

	LPCTSTR cls = AfxRegisterWndClass( 0 );

	WNDCLASS wc;
	const TCHAR *className = _T("PythonDebugThreadClass");
	wc.lpszClassName = className;
	wc.lpfnWndProc = DebuggerWndProc;
	wc.style = /*CS_OWNDC |*/ CS_VREDRAW | CS_HREDRAW;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)( COLOR_WINDOW );
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClass( &wc );
	hwndDebug = ::CreateWindowEx( 0, className, _T("Python"), WS_OVERLAPPEDWINDOW, 
				14, 8, 70, 60, 
				NULL, NULL, AfxGetInstanceHandle(),   NULL );

	AddIcons(hwndDebug);

    while (GetMessage(&msg, 0, 0, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	return 0;
}