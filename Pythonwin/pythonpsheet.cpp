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

BEGIN_MESSAGE_MAP(CPythonPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPythonPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef _DEBUG
void CPythonPropertySheet::Dump( CDumpContext &dc ) const
{
	CPropertySheet::Dump(dc);
	DumpAssocPyObject(dc, (void *)this);
}
#endif

