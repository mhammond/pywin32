//****************************************************************************
// File: progress.c
//
// Purpose: example DLL file to display a custom progress bar
//
// Functions: LibMain, Update
//
//
// Programmer:  John McMillan
//
// Taken pretty much directly from Wise samples 

//
//****************************************************************************

#include <windows.h>
#include "resource.h"
#include "wisedll.h"
#include "process.h"
#include "commctrl.h"

extern HINSTANCE hDllInst;

HWND hProgressDlg;   // The Progress dialog window handle
HWND hProgressBar;	// The progress bar
HFONT hLightFont;    // A non-bold font for the dialog text
int iWndPosition;    // A number from 0 to 8 indicating the postition of the progress dialog
int iTotalPercent;   // The percentage (times 10) of the total installation
BOOL bCanceled;      // True if the user pressed the cancel button
BOOL bHidden;        // The progress bar dialog is hidden
BOOL bCentered;      // Indicates if the dialog has been centered
POINT ptTotalStart,ptTotalEnd; // The upper left and lower right points of total progress bar
BOOL bCreated;

BOOL CALLBACK ProgressDlg(HWND, UINT, WPARAM, LPARAM);
void CenterWindow(HWND hWnd);
void GetEndPoints(HWND hDlg,UINT uID,POINT* pStart,POINT* pEnd);
void DisplayBar(HDC hdc,POINT* ptStart,POINT* ptEnd,int iPercent);
void SetLightFont(HWND hWnd);
BOOL CALLBACK EnumChildCallback(HWND hWnd,LPARAM lParam);

#define USE_MSG_PUMP

#ifdef USE_MSG_PUMP
HANDLE hCreateEvent;
#endif

void ProgressSetStep(int step);

//***********************************************************************
// Function: 
//
// Purpose: Called to update the progress bar
//
// Parameters: 
//
//
// Comments:
//
// History:  Date       Author        Reason
//
//****************************************************************************
#ifdef USE_MSG_PUMP
void _ProgressThread(void *parent)
{
	parent = NULL; // Hmmmmmm.....
	bHidden = TRUE;
	bCanceled = FALSE;
	bCreated = FALSE;
	bCentered = FALSE;

	hProgressDlg = CreateDialogParam(hDllInst,MAKEINTRESOURCE(IDD_PROGRESS_DLG),(HWND)parent,(DLGPROC)ProgressDlg, 0);
	hProgressBar = GetDlgItem(hProgressDlg, IDC_PROGRESS);
	LONG styleex = GetWindowLong(hProgressDlg, GWL_EXSTYLE);
	styleex |= WS_EX_TOPMOST;
	SetWindowLong(hProgressDlg, GWL_EXSTYLE, styleex);
	ProgressSetStep(1);

	::SetEvent(hCreateEvent);
//	hProgressDlg = CreateDialog(hDllInst,MAKEINTRESOURCE(IDD_PROGRESS_DLG),(HWND)parent,(DLGPROC)ProgressDlg);

    MSG msg;
	while (::GetMessage(&msg, NULL, NULL, NULL)) {
		if (hProgressDlg == (HWND) NULL ||
			!IsDialogMessage(hProgressDlg, &msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	bCreated = FALSE;
	hProgressDlg = NULL;
	hProgressBar = NULL;
	::SetEvent(hCreateEvent);
}
#endif
HWND ProgressInit(HWND parent)
{
#ifdef USE_MSG_PUMP
	hCreateEvent = ::CreateEvent(NULL, 0, 0, NULL);
	_beginthread( _ProgressThread, 0, parent);
	::WaitForSingleObject(hCreateEvent, INFINITE);
#else
	hProgressDlg = CreateDialog(hDllInst,MAKEINTRESOURCE(IDD_PROGRESS_DLG),(HWND)parent,(DLGPROC)ProgressDlg);
	hProgressBar = GetDlgItem(hProgressDlg, IDC_PROGRESS);
#endif
	return 0;
}

void ProgressDone()
{
	if (hLightFont != NULL) DeleteObject(hLightFont);
#ifdef USE_MSG_PUMP
	if (hProgressDlg != NULL) {
		::PostMessage(hProgressDlg, WM_USER+1024, 0, 0);
		::WaitForSingleObject(hCreateEvent, 10000);
	}
	::CloseHandle(hCreateEvent);
#else // not USE_MSG_PUMP
	if (hProgressDlg != NULL)
		::DestroyWindow(hProgressDlg);
#endif
}

void ProgressHide()
{
      EnableWindow(hProgressDlg,FALSE);
      ShowWindow(hProgressDlg,SW_HIDE);
      bHidden = TRUE;
}

void ProgressSetStep(int step)
{
	if (hProgressBar)
		::SendMessage(hProgressBar, PBM_SETSTEP, step, 0); 	
}

void ProgressSetRange(int min, int max)
{
	if (hProgressBar)
		::SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(min, max)); 	
}

BOOL _ProgressEnsure()
{
	if (hProgressDlg==NULL) return FALSE;
	if (bCanceled)
		return FALSE;
	if (bCreated && bHidden) return TRUE;
	bCreated = TRUE;
	iWndPosition = 0; // wtf?
	if (!bCentered) {
		CenterWindow(hProgressDlg);
		bCentered = TRUE;
	}
	if (bHidden) {
		EnableWindow(hProgressDlg,TRUE);
		ShowWindow(hProgressDlg,SW_SHOW);
		bHidden = FALSE;
	}
	return TRUE;
}

BOOL ProgressSetText(char *text)
{
	if (!_ProgressEnsure())
		return FALSE;

	if (text && hProgressDlg) 
		SetDlgItemText(hProgressDlg,IDC_STATIC_TEXT,text);
	return TRUE;
}

BOOL ProgressSetTitle(char *text)
{
	if (!_ProgressEnsure())
		return FALSE;

	if (text && hProgressDlg) 
		SetWindowText(hProgressDlg,text);
	return TRUE;
}

BOOL ProgressStepIt(char *szDescr /* = NULL */)
{
	if (!_ProgressEnsure())
		return FALSE;
	if (szDescr != NULL) SetDlgItemText(hProgressDlg,IDC_STATIC_TEXT,szDescr);

	if (hProgressBar) {
		::PostMessage(hProgressBar, PBM_STEPIT, 0, 0); 	
		return TRUE;
	}
	else
		return FALSE;
}

/***
BOOL ProgressUpdate(int iPosition,char const* szDescr,int iPerInstall)
{
      if (hProgressDlg==NULL) return FALSE;
      RECT rc;
      if (bCanceled) {
//         bCanceled = FALSE; // Turn this off in case user chooses to continue - what??
         return FALSE;
      }
      if ((szDescr == NULL) && bHidden) return TRUE;
      iWndPosition = iPosition;
      if (!bCentered) {
         CenterWindow(hProgressDlg);
         bCentered = TRUE;
      }
      if (bHidden) {
         EnableWindow(hProgressDlg,TRUE);
         ShowWindow(hProgressDlg,SW_SHOW);
         bHidden = FALSE;
      }
      if (szDescr != NULL) SetDlgItemText(hProgressDlg,IDC_STATIC_TEXT,szDescr);
      iTotalPercent = iPerInstall;
      rc.top = ptTotalStart.y - 1;
      rc.bottom = ptTotalEnd.y + 1;
      rc.left = ptTotalStart.x - 1;
      rc.right = ptTotalEnd.x + 1;
      InvalidateRect(hProgressDlg,&rc,FALSE);
	  return TRUE;
}
***/
BOOL CALLBACK ProgressDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
   switch (message) {
    case WM_INITDIALOG:
      SetLightFont(hDlg);
      GetEndPoints(hDlg,IDC_STATIC_TOTAL,&ptTotalStart,&ptTotalEnd);
      return TRUE;
    case WM_COMMAND:
      bCanceled = TRUE;
      break;
	case WM_DESTROY:
	  PostQuitMessage(0);
	  break;
#ifdef USE_MSG_PUMP
	case WM_USER+1024:
	  ::DestroyWindow(hProgressDlg);
	  break;
#endif
   }
   return FALSE;
}

void CenterWindow(HWND hWnd)
{
   int i,j,x,y,iScreenX,iScreenY;
   RECT rc;
   HDC hDC;

   GetWindowRect(hWnd,&rc);
   i = rc.right - rc.left;
   j = rc.bottom - rc.top;
   hDC = GetDC(NULL);
   iScreenX = GetDeviceCaps(hDC,HORZRES);
   iScreenY = GetDeviceCaps(hDC,VERTRES);
/* WTF?
   x = (((((iWndPosition & 0xf) % 3) + 1) * iScreenX) / 4) - (i / 2);
   y = (((((iWndPosition & 0xf) / 3) + 1) * iScreenY) / 4) - (j / 2);
*/
   x = (iScreenX / 2) - (i / 2);
   y = (iScreenY / 2) - (j / 2);
   if ((x + i) > iScreenX) x = iScreenX - i;
   if ((y + j) > iScreenY) y = iScreenY - j;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   ReleaseDC(NULL,hDC);
   MoveWindow(hWnd,x,y,i,j,FALSE);
}


void GetEndPoints(HWND hDlg,UINT uID,POINT* ptStart,POINT* ptEnd)
{
   HWND hBoxWnd;
   RECT rc;

   hBoxWnd = GetDlgItem(hDlg,uID);
   if (hBoxWnd != NULL) {
      GetWindowRect(hBoxWnd,&rc);
      ptStart->x = rc.left + 1;
      ptStart->y = rc.top + 1;
      ScreenToClient(hDlg,ptStart);
      ptEnd->x = rc.right - 1;
      ptEnd->y = rc.bottom - 1;
      ScreenToClient(hDlg,ptEnd);
   }
}

void DisplayBar(HDC hdc,POINT* ptStart,POINT* ptEnd,int iPercent)
{
   SIZE sSize;
   char ach[8];
   RECT rc,rc1;
   DWORD wGomerX,wWidth,wHeight;
   int iCurrPercent;

   wWidth = ptEnd->x - ptStart->x;
   wHeight = ptEnd->y - ptStart->y;
   rc.left = ptStart->x;
   rc.right = ptStart->x + (WORD)(((DWORD)iPercent * (DWORD)wWidth) / (DWORD)1000);
   rc.top = ptStart->y;
   rc.bottom = ptEnd->y;
   iCurrPercent = (iPercent + 5) / 10;
   wsprintf(ach,"%3d%%",iCurrPercent);
   GetTextExtentPoint(hdc,ach,wGomerX = lstrlen(ach),&sSize);
   SetBkColor(hdc,RGB(0,0,127));
   SetTextColor(hdc,RGB(192,192,192));
   ExtTextOut(hdc, (wWidth - sSize.cx) / 2 + ptStart->x,
              (wHeight - sSize.cy) / 2 + ptStart->y,
              ETO_OPAQUE | ETO_CLIPPED, &rc, ach, wGomerX, NULL);
   SetBkColor(hdc,RGB(192,192,192));
   SetTextColor(hdc,RGB(0,0,127));
   rc1 = rc;
   rc1.left = rc.right + 1;
   rc1.right = ptEnd->x;
   ExtTextOut(hdc, (wWidth - sSize.cx) / 2 + ptStart->x,
              (wHeight - sSize.cy) / 2 + ptStart->y,
              ETO_OPAQUE | ETO_CLIPPED, &rc1, ach, wGomerX, NULL);
}

void SetLightFont(HWND hWnd)
{
   HFONT hDialogFont;
   LOGFONT dlgFont;

   if (hLightFont == NULL) {
      hLightFont = NULL;
      if ((hDialogFont = (HFONT) SendMessage(hWnd,WM_GETFONT,0,0)) != NULL) {
         if (GetObject(hDialogFont,sizeof(dlgFont),&dlgFont)) {
            dlgFont.lfWeight = FW_LIGHT;
            hLightFont = CreateFontIndirect(&dlgFont);
         }
      }
   }
   if (hLightFont != NULL) EnumChildWindows(hWnd,(WNDENUMPROC)EnumChildCallback,(LPARAM)hLightFont);
}

BOOL CALLBACK EnumChildCallback(HWND hWnd,LPARAM lParam)
{
   SendMessage(hWnd,WM_SETFONT,(WPARAM)lParam,0L);
   return TRUE;
}
