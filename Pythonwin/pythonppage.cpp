// pythonppage.cpp : implementation file
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"
#include "pythonwin.h"
#include "win32ui.h"

#include "pythonppage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonPropertyPage property page

IMPLEMENT_DYNAMIC(CPythonPropertyPage, CPropertyPage)


CPythonPropertyPage::CPythonPropertyPage(UINT id, UINT caption) : 
	CPythonPropertyPageFramework<CPropertyPage>(id, caption)
{
	CommonConstruct();
}

CPythonPropertyPage::CPythonPropertyPage(LPCTSTR id, UINT caption) : 
	CPythonPropertyPageFramework<CPropertyPage>(id, caption)
{
	CommonConstruct();
}

void CPythonPropertyPage::CommonConstruct()
{
//	hTemplate = 0;
	hSaved = 0;
	//{{AFX_DATA_INIT(CPythonPropertyPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPythonPropertyPage::~CPythonPropertyPage()
{
	if (m_psp.pResource)
	{
		GlobalUnlock(hSaved);
		GlobalFree(hSaved);
	}
}

void CPythonPropertyPage::PostNcDestroy()
{
}

BOOL CPythonPropertyPage::SetTemplate(HGLOBAL tpl)
{
	hSaved = tpl;
//	if (m_psp.pResource!=NULL) {
//		PyErr_SetString(ui_module_error, "The template can only be assigned once");
//		return FALSE;
//	}
	m_psp.dwFlags |= PSP_DLGINDIRECT;

	m_psp.pResource = (const DLGTEMPLATE *)GlobalLock(tpl);

	// Set the caption if not already set
	if (m_strCaption.GetLength() == 0)
	{
		// use a LPWSTR because all resource are UNICODE
		LPCWSTR p = (LPCWSTR)((BYTE*)m_psp.pResource + sizeof(DLGTEMPLATE));
		// skip menu stuff
		p+= (*p == 0xffff) ? 2 : wcslen(p)+1;
		// skip window class stuff
		p+= (*p == 0xffff) ? 2 : wcslen(p)+1;
		// we're now at the caption
		m_strCaption = p;
	}
	return TRUE; //CreatePage();
}

#ifdef _DEBUG
void CPythonPropertyPage::Dump( CDumpContext &dc ) const
{
	CPropertyPage::Dump(dc);
	DumpAssocPyObject(dc, (void *)this);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonPropertyPage message handlers
