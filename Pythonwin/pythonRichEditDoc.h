
class CPythonRichEditDoc : public CPythonDocTemp<CRichEditDoc>
{
	DECLARE_DYNCREATE(CPythonRichEditDoc);
	virtual CRichEditCntrItem* CreateClientItem( REOBJECT* preo = NULL ) const;
protected:
	//{{AFX_MSG(CPythonRichEditDoc)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

