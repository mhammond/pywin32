// win32splitter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPythonSplitter frame

class CPythonSplitter : public CPythonWndFramework<CSplitterWnd>
{
	DECLARE_DYNAMIC( CPythonSplitter );
public:
	CPythonSplitter();
	virtual BOOL CPythonSplitter::CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext);
protected:
	BOOL bHaveAllChildren;
	virtual void PostNcDestroy();
	// Generated message map functions
	//{{AFX_MSG(CPythonSplitter)
	afx_msg void OnSize( UINT nType, int cx, int cy );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void AssertValid() const { // MFCs version wont allow us to call it before created, and our framework want's to!
		CWnd::AssertValid();
	}
};

/////////////////////////////////////////////////////////
//
//	splitter_window
class PyCSplitterWnd : public PyCWnd {
public:
	static PyObject *create(PyObject *self, PyObject *args);
	static CPythonSplitter *GetSplitterObject(PyObject *self);
protected:
	PyCSplitterWnd();
	virtual ~PyCSplitterWnd();
public:
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCSplitterWnd)
};

/////////////////////////////////////////////////////////////////////////////
