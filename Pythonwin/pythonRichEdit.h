// pythonRichEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPythonRichEditView window

class CPythonRichEditView : public CRichEditView
{
// Construction
public:
	CPythonRichEditView();

// Attributes
public:

// Operations
public:
	void SetDocument( CDocument *pDocument ) {m_pDocument = pDocument;}

// Overrides
	virtual void OnInitialUpdate();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPythonRichEditView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPythonRichEditView();

	// Generated message map functions
protected:
#ifdef _DEBUG
	void CPythonRichEditView::Dump( CDumpContext &dc ) const;
#endif

	//{{AFX_MSG(CPythonRichEditView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo );
	virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult);
#ifdef PYWIN_WITH_WINDOWPROC
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
