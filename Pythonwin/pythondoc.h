// pythondoc.h : interface of the CPythonDocTemp class
//
// @doc
/////////////////////////////////////////////////////////////////////////////
#ifndef __PYTHONDOC_H__
#define __PYTHONDOC_H__

////////////////////////

template <class P> class CPythonDocTemp : public P 
{
public:
	CPythonDocTemp();

// Implementation
public:
	virtual ~CPythonDocTemp();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
#ifdef _DEBUG
	virtual	void AssertValid() const;
	virtual	void Dump(CDumpContext& dc) const;
#endif
	void SetPathName( const char *pathName );
protected:
	virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo );
	virtual BOOL 	SaveModified();
	virtual BOOL 	OnOpenDocument( const TCHAR *);
	virtual BOOL 	OnSaveDocument( const TCHAR *);
	virtual void 	OnCloseDocument();
	virtual void 	DeleteContents();
	virtual	BOOL	OnNewDocument();
	virtual BOOL 	DoFileSave();
	virtual BOOL	DoSave(LPCTSTR lpszPathName, BOOL bReplace=TRUE);
	virtual void	PreCloseFrame( CFrameWnd *pWnd );
	virtual void    OnChangedViewList();

// Generated message map functions
protected:
public:

};

template <class P>
CPythonDocTemp<P>::CPythonDocTemp()
{
}

template <class P>
CPythonDocTemp<P>::~CPythonDocTemp()
{
	Python_delete_assoc( this );
}

template <class P>
BOOL CPythonDocTemp<P>::OnCmdMsg (UINT nID, int nCode,
		      void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo)
{
  // yield to Python first
  if (Python_OnCmdMsg (this, nID, nCode, pExtra, pHandlerInfo))
    return TRUE;
  else
    return P::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
}

template <class P>
BOOL CPythonDocTemp<P>::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	// @pyvirtual int|PyCDocument|DoSave|Called by the MFC architecture to save a document.
	// @pyparm string|fileName||The name of the file being saved.
	// @pyparm int|bReplace||TRUE if the file should be replaced.
	// @xref <om PyCDocument.DoSave>
	// @comm If a handler is defined for this function, it must call the
	// base class <om PyCDocument.DoSave> method.
	CVirtualHelper helper( "DoSave", this );
	if (helper.HaveHandler()) {
		if (!helper.call(lpszPathName, bReplace))
			return FALSE;
		int ret;
		// @rdesc TRUE if the document could be saved, else FALSE.
        if (helper.retval(ret))
			return ret;
		return FALSE;
	}
	return P::DoSave(lpszPathName, bReplace);
}

template <class P>
BOOL CPythonDocTemp<P>::DoFileSave()
{
	CVirtualHelper helper( "DoFileSave", this );
	// @pyvirtual int|PyCDocument|DoFileSave|Called by the MFC architecture.
	// @comm If a handler is defined for this function, it must call the
	// base class <om PyCDocument.DoFileSave> method.
	// @xref <om PyCDocument.DoFileSave>
	if (helper.HaveHandler()) {
		if (!helper.call())
			return FALSE;
		int ret;
		// @rdesc TRUE if the document could be saved, else FALSE.
        if (helper.retval(ret))
			return ret;
		return FALSE;
	}
	return P::DoFileSave();
}

template <class P>
BOOL CPythonDocTemp<P>::OnSaveDocument(const TCHAR *fileName)
{
	// @pyvirtual int|PyCDocument|OnSaveDocument|Called by the MFC architecture.
	// @pyparm string|fileName||The name of the file being saved.
	// @xref <om PyCDocument.OnSaveDocument>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "OnSaveDocument", this );
	if (helper.call(fileName)) {
		int ret;
		// @rdesc TRUE if the document could be saved, else FALSE.
        if (helper.retval(ret))
			return ret;
		return FALSE;
	}
	return FALSE;
//	return CDocument::OnSaveDocument(fileName);
}

template <class P>
BOOL CPythonDocTemp<P>::OnOpenDocument(const TCHAR *fileName)
{
	// @pyvirtual int|PyCDocument|OnOpenDocument|Called by the MFC architecture.
	// @xref <om PyCDocument.OnOpenDocument>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "OnOpenDocument", this );
	if (!helper.HaveHandler()) {
		CEnterLeavePython _celp; // grab lock to report error
		PyErr_SetString(ui_module_error,"PyCDocument::OnOpenDocument handler does not exist.");
		gui_print_error();
		return FALSE;
	}
	// @pyparm string|fileName||The name of the file being opened.
	if (helper.call(fileName)) {
		int ret;
		// @rdesc TRUE if the document could be opened, else FALSE.
        if (helper.retval(ret))
			return ret;
		return FALSE;
	}
	return FALSE; // failed!
}

template <class P>
BOOL CPythonDocTemp<P>::OnNewDocument()
{
	// @pyvirtual int|PyCDocument|OnNewDocument|Called by the MFC architecture.
	// @xref <om PyCDocument.OnNewDocument>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "OnNewDocument", this );
	if (!helper.HaveHandler()) {
		return P::OnNewDocument();
	}
	// from here, it means a Python exception occurred, and this has been reported.
	if (helper.call()) {
		int ret;
		// @rdesc TRUE if a new document could be created, else FALSE.
	if (helper.retval(ret))
			return ret;
		else {
			CEnterLeavePython _celp; // grab lock to report error
			PyErr_SetString(PyExc_TypeError, "PyCDocument.OnNewDocument - bad return type.");
			gui_print_error();
			return FALSE;
		}
	}
	return FALSE;
}
template <class P>
void CPythonDocTemp<P>::OnCloseDocument()
{
	// @pyvirtual |PyCDocument|OnCloseDocument|Called by the MFC architecture.
	// @xref <om PyCDocument.OnCloseDocument>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "OnCloseDocument", this );
	if (helper.HaveHandler()) {
		helper.call();
	} else
		P::OnCloseDocument();
}

template <class P>
void CPythonDocTemp<P>::PreCloseFrame( CFrameWnd *pWnd )
{
	// @pyvirtual |PyCDocument|PreCloseFrame|Called before the frame window is closed.
	CVirtualHelper helper( "PreCloseFrame", this );
	helper.call(pWnd);
	P::PreCloseFrame(pWnd);
	// @comm The MFC base implementation is always called after the Python handler returns.
}

template <class P>
void CPythonDocTemp<P>::DeleteContents()
{
	// @pyvirtual |PyCDocument|DeleteContents|Called by the MFC architecture when a document is newly created or closed.
	// @xref <om PyCDocument.DeleteContents>
	CVirtualHelper helper( "DeleteContents", this );
	if (!helper.call())
		P::DeleteContents();
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
}
template <class P>
BOOL CPythonDocTemp<P>::SaveModified()
{
	// @pyvirtual int|PyCDocument|SaveModified|Called by the MFC architecture when a document is closed.
	// @xref <om PyCDocument.SaveModified>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "SaveModified", this );
	if (!helper.HaveHandler())
		return P::SaveModified();
	if (helper.call()) {
		int ret;
		// @rdesc The handler should return TRUE if it is safe to continue and close
		// the document; 0 if the document should not be closed.
        if (helper.retval(ret))
			return ret;
	}
	return FALSE;
}
template <class P>
void CPythonDocTemp<P>::OnChangedViewList()
{
	// @pyvirtual int|PyCDocument|OnChangedViewList|Called by the MFC architecture when after a view is attached.
	// @xref <om PyCDocument.OnChangedViewList>
	// @comm If a handler is defined for this function, the base (MFC) function will not
	// be called.  If necessary, the handler must call this function explicitely.
	CVirtualHelper helper( "OnChangedViewList", this );
	if (helper.HaveHandler() && helper.call()) {
		return;
	}
	P::OnChangedViewList();
}

/////////////////////////////////////////////////////////////////////////////
// CPythonDocTemp serialization

template <class P>
void CPythonDocTemp<P>::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPythonDocTemp diagnostics

#ifdef _DEBUG
template <class P>
void CPythonDocTemp<P>::AssertValid() const
{
	P::AssertValid();
}

template <class P>
void CPythonDocTemp<P>::Dump(CDumpContext& dc) const
{
	P::Dump(dc);
}

#endif //_DEBUG

class CPythonDoc : public CPythonDocTemp<CDocument>
{
	DECLARE_DYNCREATE(CPythonDoc);
protected:
	//{{AFX_MSG(CPythonDoc)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __filename_h__
/////////////////////////////////////////////////////////////////////////////
