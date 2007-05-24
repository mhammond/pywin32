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

	virtual BOOL OnInitDialog();
	virtual ~CPythonPropertySheet();
	virtual void PostNcDestroy(void);
	virtual BOOL OnCmdMsg (UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo);
	virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
protected:
	CFont m_fntPage;
	RECT m_rctPage;
        BOOL m_customizeFont;
	virtual void BuildPropPageArray ();
	//{{AFX_MSG(CPythonPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnResizePage (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
};

#endif // __filename_h__
/////////////////////////////////////////////////////////////////////////////
