// pythonpsheet.h : header file
//
#ifndef __PYTHONPSHEET_H__
#define __PYTHONPSHEET_H__

/////////////////////////////////////////////////////////////////////////////
// CPythonPropertySheet

class CPythonPropertySheet : public CPythonPropertySheetFramework<CPropertySheet>
{
	DECLARE_DYNAMIC(CPythonPropertySheet)

// Construction
public:
	CPythonPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0) :
		CPythonPropertySheetFramework<CPropertySheet>(nIDCaption, pParentWnd, iSelectPage) {;}
		CPythonPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0) :
		CPythonPropertySheetFramework<CPropertySheet>(pszCaption, pParentWnd, iSelectPage) {;}

	virtual ~CPythonPropertySheet();
	virtual void PostNcDestroy(void);
	// Generated message map functions
protected:
	//{{AFX_MSG(CPythonPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
};

#endif // __filename_h__
/////////////////////////////////////////////////////////////////////////////
