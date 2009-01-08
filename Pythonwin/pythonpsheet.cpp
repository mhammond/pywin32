// pythonpsheet.cpp : implementation file
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"
#include "pythonpsheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#if _MFC_VER < 0x0600
// MSVC V 5.1 and certain version of the IE4 SDK cant agree on object sizes!

// God damn - inlines and DLL dont agree on object sizes!!!
#	if defined(PROPSHEETHEADERA_V1_SIZE)
#		if !defined(_WIN32_IE)
#			error "Please update the IE4 SDK to a newer version"
#		endif
#		if _WIN32_IE > 0x0300
#			error "Please recompile with _WIN32_IE set to 0x0300"
#		endif
#	endif // PROPSHEETHEADERA_V1_SIZE

#endif // _MFC_VER


#define WM_RESIZEPAGE WM_APP+1

// helper function which sets the font for a window and all its children
// and also resizes everything according to the new font
void ChangeDialogFont(CWnd* pWnd, CFont* pFont)
{
	CRect windowRect;

	// grab old and new text metrics
	TEXTMETRIC tmOld, tmNew;
	CDC * pDC = pWnd->GetDC();
	CFont * pSavedFont = pDC->SelectObject(pWnd->GetFont());
	pDC->GetTextMetrics(&tmOld);
	pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tmNew);
	pDC->SelectObject(pSavedFont);
	pWnd->ReleaseDC(pDC);

	long oldHeight = tmOld.tmHeight+tmOld.tmExternalLeading;
	long newHeight = tmNew.tmHeight+tmNew.tmExternalLeading;

        // calculate new dialog window rectangle
        CRect clientRect, newClientRect, newWindowRect;

        pWnd->GetWindowRect(windowRect);
        pWnd->GetClientRect(clientRect);
        long xDiff = windowRect.Width() - clientRect.Width();
        long yDiff = windowRect.Height() - clientRect.Height();
	
        newClientRect.left = newClientRect.top = 0;
        newClientRect.right = clientRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
        newClientRect.bottom = clientRect.bottom * newHeight / oldHeight;

        newWindowRect.left = windowRect.left;
        newWindowRect.top = windowRect.top;
        newWindowRect.right = windowRect.left + newClientRect.right + xDiff;
        newWindowRect.bottom = windowRect.top + newClientRect.bottom + yDiff;
        pWnd->MoveWindow(newWindowRect);

	pWnd->SetFont(pFont);

	// iterate through and move all child windows and change their font.
	CWnd* pChildWnd = pWnd->GetWindow(GW_CHILD);

	while (pChildWnd)
	{
		pChildWnd->SetFont(pFont);
		pChildWnd->GetWindowRect(windowRect);

		CString strClass;
		::GetClassName(pChildWnd->m_hWnd, strClass.GetBufferSetLength(32), 31);
		strClass.MakeUpper();
		if(strClass==_T("COMBOBOX"))
		{
			CRect rect;
			pChildWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM) &rect);
			windowRect.right = rect.right;
			windowRect.bottom = rect.bottom;
		}

		pWnd->ScreenToClient(windowRect);
		windowRect.left = windowRect.left * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
		windowRect.right = windowRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
		windowRect.top = windowRect.top * newHeight / oldHeight;
		windowRect.bottom = windowRect.bottom * newHeight / oldHeight;
		pChildWnd->MoveWindow(windowRect);
		
		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CPythonPropertySheet

IMPLEMENT_DYNAMIC(CPythonPropertySheet, CPropertySheet)

/*CPythonPropertySheet::CPythonPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CPythonPropertySheet::CPythonPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}
*/
CPythonPropertySheet::~CPythonPropertySheet()
{
}

void CPythonPropertySheet::PostNcDestroy(void)
{
	// Loop over all pages, ensuring no Python association exists.
	// This is because some pages may never have had windows, and
	// therefore will not have the WM_DESTROY handling done.
	int numPages = GetPageCount();
	for (int i = 0; i < numPages; i++)
	{
		CPropertyPage* pPage = GetPage(i);
		delete pPage;
	}

	Python_delete_assoc(this);
	delete this;
}

BOOL
CPythonPropertySheet::OnCmdMsg (UINT nID, int nCode,
			   void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo)
{
  // yield to Python first
  if (Python_OnCmdMsg (this, nID, nCode, pExtra, pHandlerInfo))
    return TRUE;
  else {
    if (!IsWindow( this->m_hWnd ))
      return TRUE;
    return CPropertySheet::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
  }
}

BOOL CPythonPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// the sheet resizes the page whenever the Apply button is clicked so we need to size it correctly
	if (m_customizeFont &&
            (ID_APPLY_NOW == wParam ||
             ID_WIZNEXT == wParam ||
             ID_WIZBACK == wParam))
            PostMessage (WM_RESIZEPAGE);
	
	return CPropertySheet::OnCommand(wParam, lParam);
}

BOOL 
CPythonPropertySheet::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
  if (m_customizeFont) {
    NMHDR* pnmh = (LPNMHDR) lParam;
    if (TCN_SELCHANGE == pnmh->code)
      PostMessage (WM_RESIZEPAGE);
  }
  // yield to Python first
  if (Python_OnNotify (this, wParam, lParam, pResult))
    return TRUE;
  else {
    if (!IsWindow( this->m_hWnd ))
      return TRUE;
    return CPropertySheet::OnNotify (wParam, lParam, pResult);
  }
}

#ifdef PYWIN_WITH_WINDOWPROC
LRESULT CPythonPropertySheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// @pyvirtual int|PyCPropertySheet|WindowProc|Default message handler.
	LRESULT res;
	CVirtualHelper helper( "WindowProc", this );
	if (!helper.HaveHandler() || !helper.call(message, wParam, lParam) || !helper.retval(res))
		return CPropertySheet::WindowProc(message, wParam, lParam);
	return res;
}
#endif PYWIN_WITH_WINDOWPROC


extern BOOL bInFatalShutdown;

void CPythonPropertySheet::BuildPropPageArray()
{
	CPropertySheet::BuildPropPageArray();

        m_customizeFont = FALSE;
	if (bInFatalShutdown)
		return;
	CEnterLeavePython _celp;
	ui_assoc_object *py_bob = ui_assoc_object::handleMgr.GetAssocObject(this);
	if (py_bob==NULL)
		return;
	if (!py_bob->is_uiobject(&ui_assoc_object::type)) {
		TRACE("CVirtualHelper::CVirtualHelper Error: Call object is not of required type\n");
		Py_DECREF(py_bob);
		return;
	}
	if (py_bob->virtualInst) {
		PyObject *t, *v, *tb;
		PyErr_Fetch(&t,&v,&tb);
		PyObject *attr = PyObject_GetAttrString(py_bob->virtualInst, "customizeFont");
		if (attr) {
                  if (PyInt_Check(attr)) {
                    m_customizeFont = (BOOL)PyInt_AsLong(PyNumber_Int(attr));
                  }
		}
		PyErr_Restore(t,v,tb);
	}
	Py_DECREF(py_bob);

        if (!m_customizeFont) {
          return;
        }

	// get first page
	CPropertyPage* pPage = GetPage (0);
	ASSERT (pPage);
	
	// dialog template class in afxpriv.h
	CDialogTemplate dlgtemp;
	// load the dialog template
	VERIFY (dlgtemp.Load (pPage->m_psp.pszTemplate));
	// get the font information
	CString strFace;
	WORD	wSize;
	VERIFY (dlgtemp.GetFont (strFace, wSize));
	if (m_fntPage.m_hObject)
		VERIFY (m_fntPage.DeleteObject ());
	// create a font using the info from first page
	VERIFY (m_fntPage.CreatePointFont (wSize*10, strFace));
}

LRESULT CPythonPropertySheet::OnResizePage (WPARAM, LPARAM)
{
	// resize the page
	if (m_customizeFont) {
          CPropertyPage* pPage = GetActivePage ();
          ASSERT (pPage);
          pPage->MoveWindow (&m_rctPage);
        }
	return 0;
}

BOOL CPythonPropertySheet::OnInitDialog()
{
	// @pyvirtual int|PyCPropertySheet|OnInitDialog|Override to augment dialog-box initialization.
	// @comm The base implementation is not called if a handler exists.  This can be
	// done via <om PyCDialog.OnInitDialog>.
	// @xref <om PyCDialog.OnInitDialog>
	BOOL result = FALSE;
	CVirtualHelper helper( "OnInitDialog", this );
	if (!helper.HaveHandler()) {
		result = CPropertySheet::OnInitDialog();
	} else {
		if (helper.call())
			helper.retval(result);
	}

        if (!m_customizeFont) {
          return result;
        }

	// get the font for the first active page
	CPropertyPage* pPage = GetActivePage ();
	ASSERT (pPage);

	// change the font for the sheet
	ChangeDialogFont (this, &m_fntPage);
	// change the font for each page
	for (int iCntr = 0; iCntr < GetPageCount (); iCntr++)
	{
		VERIFY (SetActivePage (iCntr));
		CPropertyPage* pPage = GetActivePage ();
		ASSERT (pPage);
		ChangeDialogFont (pPage, &m_fntPage);
	}

	VERIFY (SetActivePage (pPage));

	// set and save the size of the page
	CTabCtrl* pTab = GetTabControl ();
	ASSERT (pTab);

	if (m_psh.dwFlags & PSH_WIZARD)
	{
		pTab->ShowWindow (SW_HIDE);
		GetClientRect (&m_rctPage);

		CWnd* pButton = GetDlgItem (ID_WIZBACK);
		ASSERT (pButton);
		CRect rc;
		pButton->GetWindowRect (&rc);
		ScreenToClient (&rc);
		m_rctPage.bottom = rc.top-2;
	}
	else
	{
		pTab->GetWindowRect (&m_rctPage);
		ScreenToClient (&m_rctPage);
		pTab->AdjustRect (FALSE, &m_rctPage);
	}

	// resize the page	
	pPage->MoveWindow (&m_rctPage);

	return result;
	// @rdesc Specifies whether the application has set the input focus to 
	// one of the controls in the dialog box. If OnInitDialog returns 
	// nonzero, Windows sets the input focus to the first control 
	// in the dialog box. The application can return 0/None only if 
	// it has explicitly set the input focus to one of the controls in the 
	// dialog box.
}


BEGIN_MESSAGE_MAP(CPythonPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPythonPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_WM_CLOSE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE (WM_RESIZEPAGE, OnResizePage)	
END_MESSAGE_MAP()

#ifdef _DEBUG
void CPythonPropertySheet::Dump( CDumpContext &dc ) const
{
	CPropertySheet::Dump(dc);
	DumpAssocPyObject(dc, (void *)this);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonPropertySheet message handlers

void CPythonPropertySheet::OnClose()
{
  CVirtualHelper helper( "OnClose", this );
  int ret = 1;
  if (helper.call())
    helper.retval(ret);
  if (ret)
    CPropertySheet::OnClose();
}

int CPythonPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  int result = 0;
  CPropertySheet::OnCreate(lpCreateStruct);
  CVirtualHelper helper( "OnCreate", this );
  if (helper.HaveHandler()) {
    if (helper.call(lpCreateStruct))
      helper.retval(result);
  }
  return result;
}
