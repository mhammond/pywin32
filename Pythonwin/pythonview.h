// Pythonview.h : header file
//
#ifndef __PYTHONVIEW_H__
#define __PYTHONVIEW_H__

/////////////////////////////////////////////////////////////////////////////
// CPythonViewTemp
class CPythonViewImpl : public CScrollView
{
public:
	virtual void OnPrepareDC (CDC *pDC, CPrintInfo *pInfo);

};

class CPythonListViewImpl : public CListView
{
//	DECLARE_DYNCREATE(CPythonListViewImpl)
public:
	CPythonListViewImpl();
	~CPythonListViewImpl();
// Operations
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};

class CPythonTreeViewImpl : public CTreeView
{
//	DECLARE_DYNCREATE(CPythonTreeViewImpl)
public:
	CPythonTreeViewImpl();
	~CPythonTreeViewImpl();
// Operations
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};

class CPythonEditView : public CPythonViewFramework< CEditView > 
{
	DECLARE_DYNAMIC(CPythonEditView);
};

class CPythonView : public CPythonViewFramework<CPythonViewImpl>
{
	DECLARE_DYNAMIC(CPythonView);
};

class CPythonListView : public CPythonViewFramework<CPythonListViewImpl>
{
	DECLARE_DYNAMIC(CPythonListView);
};

class CPythonTreeView : public CPythonViewFramework<CPythonTreeViewImpl>
{
	DECLARE_DYNAMIC(CPythonTreeView);
};

class CPythonFormView : public CPythonViewFramework<CFormView>
{
public:
	CPythonFormView(UINT id) :
		CPythonViewFramework<CFormView>(id) {;}
	CPythonFormView(LPCTSTR id) :
		CPythonViewFramework<CFormView>(id) {;}
	DECLARE_DYNAMIC(CPythonFormView);
};

class CPythonCtrlView : public CPythonViewFramework<CCtrlView> 
{
	DECLARE_DYNAMIC(CPythonCtrlView);
public:
	CPythonCtrlView(LPCTSTR lpszClass, DWORD dwStyle) :
		CPythonViewFramework<CCtrlView>(lpszClass, dwStyle)
		{;}
};

//typedef CPythonViewFramework<CCtrlView> CPythonCtrlView;

#endif // __filename_h__
/////////////////////////////////////////////////////////////////////////////
