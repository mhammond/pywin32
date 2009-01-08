/* win32ctllist : implementation file

	List control object.  

	Created Feb 1997, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32dc.h"
#include "win32control.h"
#include "win32ctrltree.h"
#include "win32ImageList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const char *szErrTreeRequiresWindow = "The tree must have a window object for this operation";

PyCTreeCtrl::PyCTreeCtrl()
{
}
PyCTreeCtrl::~PyCTreeCtrl()
{
}
CTreeCtrl *GetTreeCtrl(PyObject *self, bool bNeedValidHwnd = true)
{
	extern CTreeView *GetTreeViewPtr(PyObject *self);
	CTreeCtrl *rc;

	if (ui_base_class::is_uiobject(self, &PyCTreeView::type)) {
		CTreeView *pView = GetTreeViewPtr(self);
		if (pView)
			rc = &(pView->GetTreeCtrl());
		else
			rc = NULL;
	} else
		rc = (CTreeCtrl *)PyCWnd::GetPythonGenericWnd(self, &PyCTreeCtrl::type);
	if (rc && bNeedValidHwnd && !::IsWindow(rc->m_hWnd))
		RETURN_ERR((char *)szErrTreeRequiresWindow);
	return rc;
}

// @pymethod <o PyCTreeCtrl>|win32ui|CreateTreeCtrl|Creates a tree control.
PyObject *PyCTreeCtrl_create(PyObject *self, PyObject *args)
{
	return ui_assoc_object::make( PyCTreeCtrl::type, new CTreeCtrl)->GetGoodRet();
}

// @pymethod |PyCTreeCtrl|CreateWindow|Creates the actual window for the object.
static PyObject *
PyCTreeCtrl_CreateWindow(PyObject *self, PyObject *args)
{
	extern CWnd *GetWndPtrFromParam(PyObject *ob, ui_type_CObject &type);

	CTreeCtrl *pT = GetTreeCtrl(self, false);
	if (!pT) return NULL;
	RECT rect;
	PyObject *obParent;
	long style;
	long id;
	if(!PyArg_ParseTuple(args, "l(iiii)Ol:Create",
		&style, // @pyparm int|style||The window style
		&rect.left,  &rect.top,  &rect.right,&rect.bottom, // @pyparm int, int, int, int|rect||The default rectangle
		&obParent, // @pyparm parent|<o PyCWnd>||The parent window
		&id))// @pyparm int|id||The control ID
		return NULL;

	CWnd *pParent = NULL;
	if (obParent != Py_None) {
		pParent = GetWndPtrFromParam(obParent, PyCWnd::type);
		if (pParent==NULL) return NULL;
	}

	GUI_BGN_SAVE;
	// @pyseemfc CTreeCtrl|Create
	BOOL ok = pT->Create(style, rect, pParent, id);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("CTreeCtrl::Create failed");
	RETURN_NONE;
}

#define MAKE_GET_INT_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	CHECK_NO_ARGS2(args,mfcName); \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	int ret = pList->mfcName(); \
	GUI_END_SAVE; \
	return Py_BuildValue("i",ret); \
}

#define MAKE_GET_ITEM_INT_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM htree; \
	PyObject *obtree; \
	if (!PyArg_ParseTuple( args, "O:" #mfcName, &obtree)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obtree, (HANDLE *)&htree)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	int ret = pList->mfcName(htree); \
	GUI_END_SAVE; \
	return Py_BuildValue("i",ret); \
}

#define MAKE_GET_ITEM_ITEM_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM htree; \
	PyObject *obtree; \
	if (!PyArg_ParseTuple( args, "O:" #mfcName, &obtree)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obtree, (HANDLE *)&htree)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	HTREEITEM item = pList->mfcName(htree); \
	GUI_END_SAVE; \
	if (item==NULL) \
		RETURN_ERR(#mfcName " failed"); \
	return PyWinLong_FromHANDLE(item); \
}

#define MAKE_GET_ITEM_ITEM_INT_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM htree; int code;\
	PyObject *obtree; \
	if (!PyArg_ParseTuple( args, "Oi:" #mfcName, &obtree, &code)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obtree, (HANDLE *)&htree)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	HTREEITEM item = pList->mfcName(htree, code); \
	GUI_END_SAVE; \
	if (item==NULL) \
		RETURN_ERR(#mfcName " failed"); \
	return PyWinLong_FromHANDLE(item); \
}

#define MAKE_GET_ITEM_VOID_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	if (!PyArg_ParseTuple( args, ":" #mfcName)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	HTREEITEM item = pList->mfcName(); \
	GUI_END_SAVE; \
	if (item==NULL) \
		RETURN_ERR(#mfcName " failed"); \
	return PyWinLong_FromHANDLE(item); \
}

#define MAKE_SET_ITEMS_INTS_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM hitem; int i1, i2;\
	PyObject *obitem; \
	if (!PyArg_ParseTuple( args, "Oii:" #mfcName, &obitem, &i1, &i2)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obitem, (HANDLE *)&hitem)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	BOOL ok = pList->mfcName(hitem, i1, i2); \
	GUI_END_SAVE; \
	if (!ok) \
		RETURN_ERR(#mfcName " failed"); \
	RETURN_NONE; \
}

#define MAKE_BOOL_ITEM_ACTION(fnname, mfcName) \
	PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM hitem; \
	PyObject *obitem; \
	if (!PyArg_ParseTuple( args, "O:" #mfcName, &obitem)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obitem, (HANDLE *)&hitem)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	BOOL ok = pList->mfcName(hitem); \
	GUI_END_SAVE; \
	if (!ok) \
		RETURN_ERR(#mfcName " failed"); \
	RETURN_NONE; \
}

#define MAKE_BOOL_ITEM_INT_ACTION(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	HTREEITEM hitem; int code;\
	PyObject *obitem; \
	if (!PyArg_ParseTuple( args, "Oi:" #mfcName, &obitem, &code)) \
		return NULL; \
	if (!PyWinObject_AsHANDLE(obitem, (HANDLE *)&hitem)) \
		return NULL; \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	GUI_BGN_SAVE; \
	BOOL ok = pList->mfcName(hitem, code); \
	GUI_END_SAVE; \
	if (!ok) \
		RETURN_ERR(#mfcName " failed"); \
	RETURN_NONE;  \
}

#define MAKE_SETBOOL_INT_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	int val; \
	if (!PyArg_ParseTuple( args, "i:" #mfcName, &val)) \
		return NULL; \
	GUI_BGN_SAVE; \
	BOOL ok = pList->mfcName(val); \
	GUI_END_SAVE; \
	if (!ok) \
		RETURN_ERR(#mfcName "failed"); \
	RETURN_NONE; \
}
#define MAKE_SETVOID_INT_METH(fnname, mfcName) \
PyObject *fnname( PyObject *self, PyObject *args ) { \
	CTreeCtrl *pList = GetTreeCtrl(self); \
	if (!pList) return NULL; \
	int val; \
	if (!PyArg_ParseTuple( args, "i:" #mfcName, &val)) \
		return NULL; \
	GUI_BGN_SAVE; \
	pList->mfcName(val); \
	GUI_END_SAVE; \
	RETURN_NONE; \
}

// @pymethod int|PyCTreeCtrl|GetCount|Retrieves the number of tree items associated with a tree view control.
MAKE_GET_INT_METH(PyCTreeCtrl_GetCount, GetCount )

// @pymethod int|PyCTreeCtrl|GetVisibleCount|Retrieves the number of visible tree items associated with a tree view control.
MAKE_GET_INT_METH(PyCTreeCtrl_GetVisibleCount, GetVisibleCount )

// @pymethod int|PyCTreeCtrl|GetIndent|Retrieves the offset (in pixels) of a tree view item from its parent.
MAKE_GET_INT_METH(PyCTreeCtrl_GetIndent, GetIndent )

// @pymethod |PyCTreeCtrl|SetIndent|Sets the offset (in pixels) of a tree view item from its parent.
// @pyparm int|indent||The new indent.
MAKE_SETVOID_INT_METH(PyCTreeCtrl_SetIndent, SetIndent)

// @pymethod HTREEITEM|PyCTreeCtrl|GetNextItem|Retrieves the next item.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|code||Specifies the relationship of the item to fetch.
MAKE_GET_ITEM_ITEM_INT_METH(PyCTreeCtrl_GetNextItem, GetNextItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetChildItem|Retrieves the first child item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetChildItem, GetChildItem )

// @pymethod |PyCTreeCtrl|SetItemImage|Sets the index of an items images.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|iImage||The offset of the image.
// @pyparm int|iSelectedImage||The offset of the selected image.
MAKE_SET_ITEMS_INTS_METH(PyCTreeCtrl_SetItemImage, SetItemImage )

// @pymethod |PyCTreeCtrl|SetItemState|Sets the state of item.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|state||The new state
// @pyparm int|stateMask||The mask for the new state
MAKE_SET_ITEMS_INTS_METH(PyCTreeCtrl_SetItemState, SetItemState )

// @pymethod int|PyCTreeCtrl|ItemHasChildren|Returns nonzero if the specified item has child items.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_INT_METH(PyCTreeCtrl_ItemHasChildren, ItemHasChildren )

// @pymethod HTREEITEM|PyCTreeCtrl|GetNextSiblingItem|Retrieves the next sibling of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetNextSiblingItem, GetNextSiblingItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetPrevSiblingItem|Retrieves the previous sibling of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetPrevSiblingItem, GetPrevSiblingItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetParentItem|Retrieves the parent item of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetParentItem, GetParentItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetFirstVisibleItem|Retrieves the first visible item of the tree view control.
MAKE_GET_ITEM_VOID_METH(PyCTreeCtrl_GetFirstVisibleItem, GetFirstVisibleItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetNextVisibleItem|Retrieves the next visible item of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetNextVisibleItem, GetNextVisibleItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetPrevVisibleItem|Retrieves the previous visible item of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_GET_ITEM_ITEM_METH(PyCTreeCtrl_GetPrevVisibleItem, GetPrevVisibleItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetSelectedItem|Retrieves the currently selected tree view item.
MAKE_GET_ITEM_VOID_METH(PyCTreeCtrl_GetSelectedItem, GetSelectedItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetDropHilightItem|Retrieves the target of a drag-and-drop operation.
MAKE_GET_ITEM_VOID_METH(PyCTreeCtrl_GetDropHilightItem, GetDropHilightItem )

// @pymethod HTREEITEM|PyCTreeCtrl|GetRootItem|Retrieves the root of the specified tree view item.
MAKE_GET_ITEM_VOID_METH(PyCTreeCtrl_GetRootItem, GetRootItem )


// @pymethod |PyCTreeCtrl|DeleteItem|Deletes the specified item.
// @pyparm HTREEITEM|item||The specified item
MAKE_BOOL_ITEM_ACTION(PyCTreeCtrl_DeleteItem, DeleteItem )

// @pymethod |PyCTreeCtrl|SelectItem|Selects a specified tree view item.
// @pyparm HTREEITEM|item||The specified item
MAKE_BOOL_ITEM_ACTION(PyCTreeCtrl_SelectItem, SelectItem )

// @pymethod |PyCTreeCtrl|SelectDropTarget|Redraws the tree item as the target of a drag-and-drop operation.
// @pyparm HTREEITEM|item||The specified item
MAKE_BOOL_ITEM_ACTION(PyCTreeCtrl_SelectDropTarget, SelectDropTarget )

// @pymethod |PyCTreeCtrl|SelectSetFirstVisible|Selects a specified tree view item as the first visible item.
// @pyparm HTREEITEM|item||The specified item
MAKE_BOOL_ITEM_ACTION(PyCTreeCtrl_SelectSetFirstVisible, SelectSetFirstVisible )

// @pymethod |PyCTreeCtrl|SortChildren|Sorts the children of a given parent item.
// @pyparm HTREEITEM|item||The specified parent item
MAKE_BOOL_ITEM_ACTION(PyCTreeCtrl_SortChildren, SortChildren )

// @pymethod |PyCTreeCtrl|Expand|Expands, or collapses, the child items of the specified tree view item.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|code||The action to take
MAKE_BOOL_ITEM_INT_ACTION(PyCTreeCtrl_Expand, Expand )

// @pymethod |PyCTreeCtrl|Select|Selects, scrolls into view, or redraws a specified tree view item.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|code||The action to take
MAKE_BOOL_ITEM_INT_ACTION(PyCTreeCtrl_Select, Select )

// @pymethod (int,int)|PyCTreeCtrl|GetItemImage|Retrieves the index of an items images.
// @pyparm HTREEITEM|item||The specified item
PyObject *PyCTreeCtrl_GetItemImage( PyObject *self, PyObject *args ) 
{
	HTREEITEM item;
	if (!PyArg_ParseTuple( args, "i:GetItemImage", &item))
		return NULL;
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	int res1, res2;
	GUI_BGN_SAVE;
	BOOL ok = pList->GetItemImage(item, res1, res2);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("GetItemImage failed");
	return Py_BuildValue("ii",res1, res2);
}

// @pymethod (int,int)|PyCTreeCtrl|GetItemState|Retrieves the state and mask of an item.
// @pyparm HTREEITEM|item||The specified item
// @pyparm int|stateMask||The mask for the result.
PyObject *PyCTreeCtrl_GetItemState( PyObject *self, PyObject *args ) 
{
	HTREEITEM item; UINT stateMask;
	if (!PyArg_ParseTuple( args, "ii:GetItemState", &item, &stateMask))
		return NULL;
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	GUI_BGN_SAVE;
	long state = pList->GetItemState(item, stateMask);
	GUI_END_SAVE;
	return PyInt_FromLong(state);
}

// @pymethod <o PyCImageList>|PyCTreeCtrl|GetImageList|Retrieves the current image list.
PyObject *PyCTreeCtrl_GetImageList( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList;
	if (!(pList=GetTreeCtrl(self)))
		return NULL;
	int nList;
	// @pyparm int|nImageList||Value specifying which image list to retrieve. It can be one of:
	// <nl>-	commctrl.LVSIL_NORMAL   Image list with large icons.
	// <nl>-	commctrl.LVSIL_SMALL   Image list with small icons.
	// <nl>-	commctrl.LVSIL_STATE   Image list with state images.
	if (!PyArg_ParseTuple(args, "i:GetImageList", &nList))
		return NULL;
	GUI_BGN_SAVE;
	CImageList *ret = pList->GetImageList(nList);
	GUI_END_SAVE;
	if (ret==NULL)
		RETURN_ERR("There is no image list available");
	return ui_assoc_object::make( PyCImageList::type, ret)->GetGoodRet();
}


// @pymethod int|PyCTreeCtrl|InsertItem|Inserts an item into the list.
PyObject *PyCTreeCtrl_InsertItem( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList;
	HTREEITEM ret = NULL;
	UINT mask;
	int image, selImage, state, stateMask;
	PyObject *obParent, *obInsertAfter;
	LPARAM lParam;
	HTREEITEM hParent, hInsertAfter;
	TCHAR *text=NULL;
	PyObject *obtext=Py_None;
	if (!(pList=GetTreeCtrl(self)))
		return NULL;

	if (PyArg_ParseTuple(args, "iOiiiiOOO:InsertItem", 
						&mask, // @pyparmalt1 int|mask||Integer specifying which attributes to set
						&obtext, // @pyparmalt1 string|text||The text of the item.
						&image, // @pyparmalt1 int|image||The index of the image to use.
						&selImage, // @pyparmalt1 int|selectedImage||The index of the items selected image.
						&state, // @pyparmalt1 int|state||The initial state of the item.
						&stateMask, // @pyparmalt1 int|stateMask||Specifies which bits of the state are valid.
						&lParam, // @pyparmalt1 object|lParam||A user defined object for the item.
						&obParent, // @pyparmalt1 HTREEITEM|parent||The parent of the item.
						&obInsertAfter)) { // @pyparmalt1 HTREEITEM|parent||The parent of the item.
		if (!PyWinObject_AsHANDLE(obParent, (HANDLE *)&hParent))
			return NULL;
		if (!PyWinObject_AsHANDLE(obInsertAfter, (HANDLE *)&hInsertAfter))
			return NULL;
		if (!PyWinObject_AsTCHAR(obtext, &text, TRUE))
			return NULL;
		GUI_BGN_SAVE;
		ret = pList->InsertItem(mask, text, image, selImage, state, stateMask, lParam, hParent, hInsertAfter);
	 	GUI_END_SAVE;
		goto done;
		}

	PyErr_Clear();
	hParent = TVI_ROOT;
	hInsertAfter = TVI_LAST;
	if (PyArg_ParseTuple(args, "Oii|O&O&:InsertItem", 
			&obtext, // @pyparmalt2 string|text||The text for the item.
			&image, // @pyparmalt2 int|image||The index of the image to use.
			&selImage, // @pyparmalt2 int|selectedImage||The index of the items selected image.
			PyWinObject_AsHANDLE, &hParent,	// @pyparmalt2 HTREEITEM|parent|commctrl.TVI_ROOT|The parent of the item.
			PyWinObject_AsHANDLE, &hInsertAfter)	// @pyparmalt2 HTREEITEM|insertAfter|commctrl.TVI_LAST|The item to insert the new item after, or TVI_FIRST, TVI_LAST or TVI_SORT
		&& PyWinObject_AsTCHAR(obtext, &text, FALSE)){
		GUI_BGN_SAVE;
		ret = pList->InsertItem(text, image, selImage, hParent, hInsertAfter);
		GUI_END_SAVE;
		goto done;
		}

	// This arg format conflicts with the above.  Handle's can be parsed as ints, so if both optional items are
	//	passed, they will be caught by the above and never get here !
	PyErr_Clear();
	hParent = TVI_ROOT;
	hInsertAfter = TVI_LAST;
	if (PyArg_ParseTuple(args, "O|O&O&:InsertItem", 
			&obtext,	// @pyparmalt3 string|text||The text for the item.
			PyWinObject_AsHANDLE, &hParent,	// @pyparmalt3 HTREEITEM|parent|commctrl.TVI_ROOT|The parent of the item.
			PyWinObject_AsHANDLE, &hInsertAfter)	// @pyparmalt3 HTREEITEM|parent|commctrl.TVI_LAST|The parent of the item.
		&& PyWinObject_AsTCHAR(obtext, &text, FALSE)){	
		GUI_BGN_SAVE;
		ret = pList->InsertItem(text, hParent, hInsertAfter);
		GUI_END_SAVE;
		goto done;
		}

	PyErr_Clear();
	PyObject *obTVItem;
	TV_INSERTSTRUCT tvItem;
	if (PyArg_ParseTuple(args, "O&O&O:InsertItem",
			PyWinObject_AsHANDLE, &tvItem.hParent, // @pyparm HTREEITEM|hParent||The parent item.  If commctrl.TVI_ROOT or 0, it is added to the root.
			PyWinObject_AsHANDLE, &tvItem.hInsertAfter, // @pyparm HTREEITEM|hInsertAfter||The item to insert after.  Can be an item or TVI_FIRST, TVI_LAST or TVI_SORT
			&obTVItem)) { // @pyparm <o TV_ITEM>|item||A tuple describing the new item.
		if (!PyWinObject_AsTV_ITEM(obTVItem, &tvItem.item))
			return NULL;
		GUI_BGN_SAVE;
		ret = pList->InsertItem(&tvItem);
		GUI_END_SAVE;
		PyWinObject_FreeTV_ITEM(&tvItem.item);
		goto done;
		}

	PyErr_Clear();
	RETURN_ERR("InsertItem could not parse the params.");
	// And you will beat your brains out determining why ...

done:
	PyWinObject_FreeTCHAR(text);
	if (ret==NULL)
		RETURN_ERR("InsertItem failed");
	return PyWinLong_FromHANDLE(ret);
}

// @pymethod int|PyCTreeCtrl|SetItem|Sets some of all of an items attributes.
PyObject *PyCTreeCtrl_SetItem( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList;
	PyObject *obTVItem;
	if (!(pList=GetTreeCtrl(self)))
		return NULL;
	if (!PyArg_ParseTuple(args, "O:SetItem",
		                 &obTVItem)) // @pyparm <o TV_ITEM>|item||A tuple describing the new item.
		return NULL;
	TV_ITEM tvItem;
	if (!PyWinObject_AsTV_ITEM(obTVItem, &tvItem))
		return NULL;
 	GUI_BGN_SAVE;
	BOOL ok = pList->SetItem(&tvItem);
 	GUI_END_SAVE;
	PyWinObject_FreeTV_ITEM(&tvItem);
	if (!ok)
		RETURN_ERR("SetItem failed");
	RETURN_NONE;
}

// @pymethod int|PyCTreeCtrl|SetImageList|Assigns an image list to a list view control.
PyObject *PyCTreeCtrl_SetImageList( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList;
	PyObject *obList;
	int imageType;
	if (!(pList=GetTreeCtrl(self)))
		return NULL;
	if (!PyArg_ParseTuple(args, "Oi:SetImageList", 
		                  &obList, // @pyparm <o PyCImageList>|imageList||The Image List to use.
						  &imageType )) // @pyparm int|imageType||Type of image list. It can be one of (COMMCTRL.) LVSIL_NORMAL, LVSIL_SMALL or LVSIL_STATE
		return NULL;
	CImageList *pImageList = PyCImageList::GetImageList(obList);
	if (pImageList==NULL) return NULL;
 	GUI_BGN_SAVE;
	CImageList *pOldList = pList->SetImageList( pImageList, imageType );
 	GUI_END_SAVE;
	if (pOldList==NULL)
		RETURN_NONE;
	return ui_assoc_object::make( PyCImageList::type, pOldList )->GetGoodRet();
}

// @pymethod <o TV_ITEM>|PyCTreeCtrl|GetItem|Retrieves the details of an items attributes.
PyObject *PyCTreeCtrl_GetItem( PyObject *self, PyObject *args )
{
	HTREEITEM item;
	UINT mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT; 

	if (!PyArg_ParseTuple( args, "i|i:GetItem", 
	                   &item, // @pyparm HTREEITEM|item||The item whose attributes are to be retrieved.
					   &mask)) // @pyparm int|mask|(all flags set)|The requested attributes.
		return NULL;

	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	TCHAR textBuf[256];
	TV_ITEM tvItem;
	tvItem.hItem = item;
	tvItem.pszText = textBuf;
	tvItem.cchTextMax = sizeof(textBuf)/sizeof(TCHAR);
	tvItem.mask = mask;
 	GUI_BGN_SAVE;
	BOOL ok = pList->GetItem( &tvItem);
 	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("GetItem failed");
	return PyWinObject_FromTV_ITEM(&tvItem);
}

// @pymethod int|PyCTreeCtrl|GetItemText|Retrieves the text of a list view item or subitem.
PyObject *PyCTreeCtrl_GetItemText( PyObject *self, PyObject *args )
{
	HTREEITEM item;
	if (!PyArg_ParseTuple( args, "i:GetItemText", 
	                   &item)) // @pyparm HTREEITEM|item||The item whose text is to be retrieved.
		return NULL;
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
 	GUI_BGN_SAVE;
	CString csText = pList->GetItemText(item);
 	GUI_END_SAVE;
	return PyWinObject_FromTCHAR(csText);
}

// @pymethod int|PyCTreeCtrl|SetItemText|Changes the text of a list view item or subitem.
PyObject *PyCTreeCtrl_SetItemText( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	TCHAR *text;
	PyObject *obtext;
	if (!PyArg_ParseTuple( args, "iO:SetItemText", 
	                   &item, // @pyparm HTREEITEM|item||The item whose text is to be retrieved.
					   &obtext)) // @pyparm string|text||String that contains the new item text.

		return NULL;
	if (!PyWinObject_AsTCHAR(obtext, &text, FALSE))
		return NULL;
 	GUI_BGN_SAVE;
	BOOL ok = pList->SetItemText(item, text);
 	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(text);
	if (!ok)
		RETURN_ERR("SetItemText failed");
	RETURN_NONE;
}

// @pymethod object|PyCTreeCtrl|GetItemData|Retrieves the application-specific value associated with an item.
PyObject *PyCTreeCtrl_GetItemData( PyObject *self, PyObject *args )
{
	HTREEITEM item;
	if (!PyArg_ParseTuple( args, "i:GetItemData", 
	                   &item)) // @pyparm HTREEITEM|item||The index of the item whose data is to be retrieved.

		return NULL;
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	GUI_BGN_SAVE;
	DWORD_PTR rc = pList->GetItemData(item);
	GUI_END_SAVE;
	return PyWinObject_FromDWORD_PTR(rc);
}

// @pymethod int|PyCTreeCtrl|SetItemData|Sets the item's application-specific value.
PyObject *PyCTreeCtrl_SetItemData( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	int data;
	if (!PyArg_ParseTuple( args, "ii:SetItemData", 
		                   &item, // @pyparm HTREEITEM|item||The item whose Data is to be set.
						   &data)) // @pyparm int|Data||New value for the data.
		return NULL;
	GUI_BGN_SAVE;
	BOOL ok = pList->SetItemData(item, data);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("SetItemData failed");
	// @comm Note that a reference count is not added to the object.  This it is your
	// responsibility to make sure the object remains alive while in the list.
	RETURN_NONE;
}

// @pymethod object|PyCTreeCtrl|DeleteAllItems|Deletes all items in the control
PyObject *PyCTreeCtrl_DeleteAllItems( PyObject *self, PyObject *args )
{
	if (!PyArg_ParseTuple( args, ":DeleteAllItems"))
		return NULL;
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	GUI_BGN_SAVE;
	BOOL ok = pList->DeleteAllItems();
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("DeleteAllItems failed");
	RETURN_NONE;
}

// @pymethod (int, int, int, int)|PyCTreeCtrl|GetItemRect|Retrieves the bounding rectangle of a tree view item.
PyObject *PyCTreeCtrl_GetItemRect( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	RECT rect;
	BOOL bTextOnly;
	if (!PyArg_ParseTuple( args, "ii:GetItemRect", 
		                   &item, // @pyparm HTREEITEM|item||The item whose Data is to be set.
						   &bTextOnly)) // @pyparm int|bTextOnly||f this parameter is nonzero, the bounding rectangle includes only the text of the item. Otherwise it includes the entire line that the item occupies in the tree view control.
		return NULL;
	GUI_BGN_SAVE;
	BOOL ok = pList->GetItemRect(item, &rect, bTextOnly);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("GetItemRect failed");
	return Py_BuildValue("(iiii)",rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod <o PyCEdit>|PyCTreeCtrl|GetEditControl|Retrieves the handle of the edit control used to edit the specified tree view item.
PyObject *PyCTreeCtrl_GetEditControl( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	if (!PyArg_ParseTuple( args, ":GetEditControl"))
		return NULL;
	GUI_BGN_SAVE;
	CEdit *pEdit = pList->GetEditControl();
	GUI_END_SAVE;
	if (pEdit==NULL)
		RETURN_ERR("GetEditControl failed");
	return ui_assoc_object::make(UITypeFromCObject(pEdit), pEdit)->GetGoodRet();
}

// @pymethod <o PyCEdit>|PyCTreeCtrl|EditLabel|Edits a specified tree view item in-place.
PyObject *PyCTreeCtrl_EditLabel( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	// @pyparm HTREEITEM|item||The item to edit.
	if (!PyArg_ParseTuple( args, "i:EditLabel", &item))
		return NULL;
	GUI_BGN_SAVE;
	CEdit *pEdit = pList->EditLabel(item);
	GUI_END_SAVE;
	if (pEdit==NULL)
		RETURN_ERR("EditLabel failed");
	return ui_assoc_object::make(UITypeFromCObject(pEdit), pEdit)->GetGoodRet();
}

// @pymethod int|PyCTreeCtrl|EnsureVisible|Ensures that a tree view item is visible in its tree view control.
PyObject *PyCTreeCtrl_EnsureVisible( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	// @pyparm HTREEITEM|item||The item to edit.
	if (!PyArg_ParseTuple( args, "i:EnsureVisible", &item))
		return NULL;
	GUI_BGN_SAVE;
	BOOL ok = pList->EnsureVisible(item);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("EnsureVisible failed");
	RETURN_NONE;
}

// @pymethod <o PyCImageList>|PyCTreeCtrl|CreateDragImage|Creates a dragging bitmap for the specified tree view item.
PyObject *PyCTreeCtrl_CreateDragImage( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	HTREEITEM item;
	// @pyparm HTREEITEM|item||The item to edit.
	if (!PyArg_ParseTuple( args, "i:CreateDragImage", &item))
		return NULL;
	GUI_BGN_SAVE;
	CImageList *pIL = pList->CreateDragImage(item);
	GUI_END_SAVE;
	if (pIL==NULL)
		RETURN_ERR("CreateDragImage failed");
	return ui_assoc_object::make(PyCImageList::type, pIL)->GetGoodRet();
}

// @pymethod <o PyCToolTopCtrl>|PyCTreeCtrl|GetToolTips|Returns the tooltip control
PyObject *PyCTreeCtrl_GetToolTips( PyObject *self, PyObject *args ) 
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	CHECK_NO_ARGS2(args,"GetToolTips");
	CToolTipCtrl*ret = pList->GetToolTips();
	return ui_assoc_object::make( PyCToolTipCtrl::type, ret)->GetGoodRet();
}

// @pymethod (int, int)|PyCTreeCtrl|HitTest|Determines which tree view item, if any, is at a specified position.
PyObject *PyCTreeCtrl_HitTest( PyObject *self, PyObject *args )
{
	CTreeCtrl *pList = GetTreeCtrl(self);
	if (!pList) return NULL;
	TVHITTESTINFO i;
	memset(&i, 0, sizeof(i));
	// @pyparm point|x,y||The point to test.
	if (!PyArg_ParseTuple( args, "(ii):HitTest", &i.pt.x, &i.pt.y))
		return NULL;
	GUI_BGN_SAVE;
	pList->HitTest( &i );
	GUI_END_SAVE;
	return Py_BuildValue("iN", i.flags, PyWinLong_FromHANDLE(i.hItem));
	// @rdesc The result is a tuple of (flags, hItem).
	// flags may be a combination of the following values:
	// @flagh Value|Description
	// @flag commctrl.TVHT_ABOVE|Above the client area.  
	// @flag commctrl.TVHT_BELOW|Below the client area.  
	// @flag commctrl.TVHT_NOWHERE|In the client area, but below the last item.  
	// @flag commctrl.TVHT_ONITEM|On the bitmap or label associated with an item.  
	// @flag commctrl.TVHT_ONITEMBUTTON|On the button associated with an item.  
	// @flag commctrl.TVHT_ONITEMICON|On the bitmap associated with an item.  
	// @flag commctrl.TVHT_ONITEMINDENT|In the indentation associated with an item.  
	// @flag commctrl.TVHT_ONITEMLABEL|On the label (string) associated with an item.  
	// @flag commctrl.TVHT_ONITEMRIGHT|In the area to the right of an item.  
	// @flag commctrl.TVHT_ONITEMSTATEICON|On the state icon for a tree view item that is in a user-defined state.  
	// @flag commctrl.TVHT_TOLEFT|To the left of the client area.  
	// @flag commctrl.TVHT_TORIGHT|To the right of the client area.  
}


// @object PyCTreeCtrl|A class which encapsulates an MFC CTreeCtrl object.  Derived from a <o PyCWnd> object.
static struct PyMethodDef PyCTreeCtrl_methods[] = {
	// Originally the same order as MFC doco.
	{"CreateWindow",   PyCTreeCtrl_CreateWindow, 1}, // @pymeth CreateWindow|Creates the actual window for the object.
	{"GetCount",        PyCTreeCtrl_GetCount,  1}, // @pymeth GetCount|Retrieves the number of tree items associated with a tree view control.
	{"GetIndent",      PyCTreeCtrl_GetIndent,  1}, // @pymeth GetIndent|Retrieves the offset (in pixels) of a tree view item from its parent.
	{"SetIndent",      PyCTreeCtrl_SetIndent,  1}, // @pymeth SetIndent|Sets the offset (in pixels) of a tree view item from its parent.
	{"GetImageList",   PyCTreeCtrl_GetImageList,  1}, // @pymeth GetImageList|Retrieves the current image list.
	{"SetImageList",   PyCTreeCtrl_SetImageList, 1}, // @pymeth SetImageList|Assigns an image list to a list view control.
	{"GetNextItem",    PyCTreeCtrl_GetNextItem,  1}, // @pymeth GetNextItem|Retrieves the next item.
	{"ItemHasChildren",PyCTreeCtrl_ItemHasChildren,  1}, // @pymeth ItemHasChildren|Returns nonzero if the specified item has child items.
	{"GetChildItem",   PyCTreeCtrl_GetChildItem,  1}, // @pymeth GetChildItem|Retrieves the child item of the specified tree view item.
	{"GetNextSiblingItem",PyCTreeCtrl_GetNextSiblingItem,  1}, // @pymeth GetNextSiblingItem|Retrieves the next sibling of the specified tree view item.
	{"GetPrevSiblingItem",PyCTreeCtrl_GetPrevSiblingItem,  1}, // @pymeth GetPrevSiblingItem|Retrieves the previous sibling of the specified tree view item.
	{"GetParentItem",PyCTreeCtrl_GetParentItem,  1}, // @pymeth GetParentItem|Retrieves the parent item of the specified tree view item.
	{"GetFirstVisibleItem",PyCTreeCtrl_GetFirstVisibleItem,  1}, // @pymeth GetFirstVisibleItem|Retrieves the first visible item of the specified tree view item.
	{"GetNextVisibleItem",PyCTreeCtrl_GetNextVisibleItem,  1}, // @pymeth GetNextVisibleItem|Retrieves the next visible item of the specified tree view item.
	{"GetPrevVisibleItem",PyCTreeCtrl_GetPrevVisibleItem,  1}, // @pymeth GetNextVisibleItem|Retrieves the previous visible item of the specified tree view item.
	{"GetSelectedItem",PyCTreeCtrl_GetSelectedItem,  1}, // @pymeth GetSelectedItem|Retrieves the currently selected tree view item.
	{"GetDropHilightItem",PyCTreeCtrl_GetDropHilightItem,  1}, // @pymeth GetDropHilightItem|Retrieves the target of a drag-and-drop operation.
	{"GetRootItem",       PyCTreeCtrl_GetRootItem,  1}, // @pymeth GetRootItem|Retrieves the root of the specified tree view item.
	{"GetToolTips",       PyCTreeCtrl_GetToolTips,  1}, // @pymeth GetToolTips|Returns the tooltip control
	{"GetItem",           PyCTreeCtrl_GetItem,  1}, // @pymeth GetItem|Retrieves the details of an items attributes.
	{"SetItem",           PyCTreeCtrl_SetItem, 1}, // @pymeth SetItem|Sets some of all of an items attributes.
	{"GetItemState",      PyCTreeCtrl_GetItemState,  1}, // @pymeth GetItemState|Retrieves the state of an item.
	{"SetItemState",      PyCTreeCtrl_SetItemState, 1}, // @pymeth SetItemState|Sets the state of an item.
	{"GetItemImage",      PyCTreeCtrl_GetItemImage,  1}, // @pymeth GetItemImage|Retrieves the index of an items images.
	{"SetItemImage",      PyCTreeCtrl_SetItemImage, 1}, // @pymeth SetItemImage|Sets the index of an items images.
	{"SetItemText",    PyCTreeCtrl_SetItemText, 1}, // @pymeth SetItemText|Changes the text of a list view item or subitem.
	{"GetItemText",    PyCTreeCtrl_GetItemText, 1}, // @pymeth GetItemText|Retrieves the text of a list view item or subitem.
	{"GetItemData",      PyCTreeCtrl_GetItemData,  1}, // @pymeth GetItemData|Retrieves the application-specific value associated with an item.
	{"SetItemData",      PyCTreeCtrl_SetItemData, 1}, // @pymeth SetItemData|Sets the item's application-specific value
	{"GetItemRect",      PyCTreeCtrl_GetItemRect, 1}, // @pymeth GetItemRect|Retrieves the bounding rectangle of a tree view item.
	{"GetEditControl",   PyCTreeCtrl_GetEditControl, 1}, // @pymeth GetEditControl|Retrieves the handle of the edit control used to edit the specified tree view item.
	{"GetVisibleCount",   PyCTreeCtrl_GetVisibleCount, 1}, // @pymeth GetVisibleCount|Retrieves the number of visible tree items associated with a tree view control.

	{"InsertItem",     PyCTreeCtrl_InsertItem,  1}, // @pymeth InsertItem|Inserts an item into the list.
	{"DeleteItem",     PyCTreeCtrl_DeleteItem,  1}, // @pymeth DeleteItem|Deletes an item from the list.
	{"DeleteAllItems", PyCTreeCtrl_DeleteAllItems,  1}, // @pymeth DeleteAllItems|Deletes all items from the list.
	{"Expand",         PyCTreeCtrl_Expand,  1}, // @pymeth Expand|Expands, or collapses, the child items of the specified tree view item.
	{"Select",         PyCTreeCtrl_Select,  1}, // @pymeth Select|Selects, scrolls into view, or redraws a specified tree view item.
	{"SelectItem",     PyCTreeCtrl_SelectItem,  1}, // @pymeth SelectItem|Selects a specified tree view item.
	{"SelectDropTarget",PyCTreeCtrl_SelectDropTarget,  1}, // @pymeth SelectDropTarget|Redraws the tree item as the target of a drag-and-drop operation.
	{"SelectSetFirstVisible",PyCTreeCtrl_SelectSetFirstVisible,  1}, // @pymeth SelectSetFirstVisible|Selects a specified tree view item as the first visible item.
	{"EditLabel",      PyCTreeCtrl_EditLabel,  1}, // @pymeth EditLabel|Edits a specified tree view item in-place.
	{"CreateDragImage",PyCTreeCtrl_CreateDragImage,  1}, // @pymeth CreateDragImage|Creates a dragging bitmap for the specified tree view item.
	{"SortChildren",   PyCTreeCtrl_SortChildren,  1}, // @pymeth SortChildren|Sorts the children of a given parent item.
	{"EnsureVisible",  PyCTreeCtrl_EnsureVisible,  1}, // @pymeth EnsureVisible|Ensures that a tree view item is visible in its tree view control.
	{"HitTest",        PyCTreeCtrl_HitTest, 1}, // @pymeth HitTest|Determines which tree view item, if any, is at a specified position.
	{NULL,			NULL}
};
// @comm Sam Rushing has found the following tidbits:<nl>
// You can implement dynamic collapsing and expanding of events for large
// collections yourself - see KB Q130697<nl>
// The MFC docs tell you to use TVE_COLLAPSERESET in order to
// throw away the child items when collapsing a node.  They neglect to
// tell you a very important tidbit: that you need to combine the flag
// with TVE_COLLAPSE.  This is pointed out in the docs for
// TreeView_Expand(), but not in those for CTreeCtrl::Expand.

ui_type_CObject PyCTreeCtrl::type("PyCTreeCtrl", 
									 &PyCWnd::type, 
									 RUNTIME_CLASS(CTreeCtrl), 
									 sizeof(PyCTreeCtrl), 
									 PYOBJ_OFFSET(PyCTreeCtrl), 
									 PyCTreeCtrl_methods, 
									 GET_PY_CTOR(PyCTreeCtrl));
