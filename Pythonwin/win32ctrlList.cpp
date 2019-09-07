/* win32ctllist : implementation file

    List control object.

    Created May 1996, Mark Hammond (MHammond@skippinet.com.au)

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
#include "win32ctrlList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const char *szErrListRequiresWindow = "The list must have a window object for this operation";

PyCListCtrl::PyCListCtrl() {}
PyCListCtrl::~PyCListCtrl() {}
CListCtrl *GetListCtrl(PyObject *self, bool bNeedValidHwnd = true)
{
    extern CListView *GetListViewPtr(PyObject * self);
    CListCtrl *rc;

    if (ui_base_class::is_uiobject(self, &PyCListView::type)) {
        CListView *pView = GetListViewPtr(self);
        if (pView)
            rc = &(pView->GetListCtrl());
        else
            rc = NULL;
    }
    else
        rc = (CListCtrl *)PyCWnd::GetPythonGenericWnd(self, &PyCListCtrl::type);
    if (rc && bNeedValidHwnd && !::IsWindow(rc->m_hWnd))
        RETURN_ERR((char *)szErrListRequiresWindow);
    return rc;
}

// @pymethod <o PyCListCtrl>|win32ui|CreateListCtrl|Creates a list control.
PyObject *PyCListCtrl_create(PyObject *self, PyObject *args)
{
    return ui_assoc_object::make(PyCListCtrl::type, new CListCtrl)->GetGoodRet();
}

// @pymethod |PyCListCtrl|CreateWindow|Creates the actual window for the object.
static PyObject *PyCListCtrl_CreateWindow(PyObject *self, PyObject *args)
{
    extern CWnd *GetWndPtrFromParam(PyObject * ob, ui_type_CObject & type);

    CListCtrl *pT = GetListCtrl(self, false);
    if (!pT)
        return NULL;
    RECT rect;
    PyObject *obParent;
    long style;
    long id;
    if (!PyArg_ParseTuple(args, "l(iiii)Ol:Create",
                          &style,  // @pyparm int|style||The window style
                          &rect.left, &rect.top, &rect.right,
                          &rect.bottom,  // @pyparm int, int, int, int|rect||The default rectangle
                          &obParent,     // @pyparm parent|<o PyCWnd>||The parent window
                          &id))          // @pyparm int|id||The control ID
        return NULL;

    CWnd *pParent = NULL;
    if (obParent != Py_None) {
        pParent = GetWndPtrFromParam(obParent, PyCWnd::type);
        if (pParent == NULL)
            return NULL;
    }

    GUI_BGN_SAVE;
    // @pyseemfc CListCtrl|Create
    BOOL ok = pT->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CListCtrl::Create failed");
    RETURN_NONE;
}

#define MAKE_GET_INT_METH(fnname, mfcName)           \
    PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                \
        CHECK_NO_ARGS2(args, mfcName);               \
        CListCtrl *pList = GetListCtrl(self);        \
        if (!pList)                                  \
            return NULL;                             \
        GUI_BGN_SAVE;                                \
        int ret = pList->mfcName();                  \
        GUI_END_SAVE;                                \
        return Py_BuildValue("i", ret);              \
    }

#define MAKE_SETBOOL_INT_METH(fnname, mfcName)            \
    PyObject *fnname(PyObject *self, PyObject *args)      \
    {                                                     \
        CListCtrl *pList = GetListCtrl(self);             \
        if (!pList)                                       \
            return NULL;                                  \
        int val;                                          \
        if (!PyArg_ParseTuple(args, "i:" #mfcName, &val)) \
            return NULL;                                  \
        GUI_BGN_SAVE;                                     \
        BOOL ok = pList->mfcName(val);                    \
        GUI_END_SAVE;                                     \
        if (!ok)                                          \
            RETURN_ERR(#mfcName "failed");                \
        RETURN_NONE;                                      \
    }
#define MAKE_SETVOID_INT_METH(fnname, mfcName)            \
    PyObject *fnname(PyObject *self, PyObject *args)      \
    {                                                     \
        CListCtrl *pList = GetListCtrl(self);             \
        if (!pList)                                       \
            return NULL;                                  \
        int val;                                          \
        if (!PyArg_ParseTuple(args, "i:" #mfcName, &val)) \
            return NULL;                                  \
        GUI_BGN_SAVE;                                     \
        pList->mfcName(val);                              \
        GUI_END_SAVE;                                     \
        RETURN_NONE;                                      \
    }

// @pymethod |PyCListCtrl|DeleteItem|Deletes the specified item.
// @pyparm int|item||The item to delete.
MAKE_SETBOOL_INT_METH(PyCListCtrl_DeleteItem, DeleteItem)

// @pymethod int|PyCListCtrl|GetBkColor|Retrieves the background color of the control.
MAKE_GET_INT_METH(PyCListCtrl_GetBkColor, GetBkColor)

// @pymethod int|PyCListCtrl|GetTextColor|Retrieves the text color of a list view control.
MAKE_GET_INT_METH(PyCListCtrl_GetTextColor, GetTextColor)
// @pymethod |PyCListCtrl|SetTextColor|Sets the text color of a list view control.
// @pyparm int|color||The new color.
MAKE_SETBOOL_INT_METH(PyCListCtrl_SetTextColor, SetTextColor)

// @pymethod |PyCListCtrl|Update|Forces the control to repaint a specified item.
// @pyparm int|item||The new color.
MAKE_SETBOOL_INT_METH(PyCListCtrl_Update, Update)

// @pymethod |PyCListCtrl|Arrange|Aligns items on a grid.
// @pyparm int|code||Specifies the alignment style for the items
MAKE_SETBOOL_INT_METH(PyCListCtrl_Arrange, Arrange)

// @pymethod int|PyCListCtrl|GetTextBkColor|Retrieves the text background color of a list view control.
MAKE_GET_INT_METH(PyCListCtrl_GetTextBkColor, GetTextBkColor)
// @pymethod |PyCListCtrl|SetTextBkColor|Sets the text background color of a list view control.
// @pyparm int|color||The new background color.
MAKE_SETBOOL_INT_METH(PyCListCtrl_SetTextBkColor, SetTextBkColor)

// @pymethod int|PyCListCtrl|GetItemCount|Retrieves the number of items in a list view control.
MAKE_GET_INT_METH(PyCListCtrl_GetItemCount, GetItemCount)
// @pymethod |PyCListCtrl|SetItemCount|Prepares a list view control for adding a large number of items.
// @pyparm int|count||Number of items that the control will ultimately contain.
// @comm By calling this function before adding a large number of items,
// you enable a list view control to reallocate its internal data structures
// only once rather than every time you add an item.
MAKE_SETVOID_INT_METH(PyCListCtrl_SetItemCount, SetItemCount)

// @pymethod |PyCListCtrl|SetBkColor|Sets the background color of the control.
// @pyparm int|color||The new background color.
MAKE_SETBOOL_INT_METH(PyCListCtrl_SetBkColor, SetBkColor)

// @pymethod int|PyCListCtrl|GetTopIndex|Retrieves the index of the topmost visible item.
MAKE_GET_INT_METH(PyCListCtrl_GetTopIndex, GetTopIndex)

// @pymethod int|PyCListCtrl|GetCountPerPage|Calculates the number of items that can fit vertically in a list view
// control.
MAKE_GET_INT_METH(PyCListCtrl_GetCountPerPage, GetCountPerPage)

// @pymethod int|PyCListCtrl|GetSelectedCount|Retrieves the number of selected items in the list view control.
MAKE_GET_INT_METH(PyCListCtrl_GetSelectedCount, GetSelectedCount)

// @pymethod |PyCListCtrl|DeleteAllItems|Deletes all items from the list.
PyObject *PyCListCtrl_DeleteAllItems(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    if (!(pList = GetListCtrl(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "DeleteAllItems");
    GUI_BGN_SAVE;
    BOOL ok = pList->DeleteAllItems();
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("DeleteAllItems failed");
    RETURN_NONE;
}

// @pymethod <o PyCImageList>|PyCListCtrl|GetImageList|Retrieves the current image list.
PyObject *PyCListCtrl_GetImageList(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    if (!(pList = GetListCtrl(self)))
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
    if (ret == NULL)
        RETURN_ERR("There is no image list available");
    return ui_assoc_object::make(PyCImageList::type, ret)->GetGoodRet();
}

// @pymethod int|PyCListCtrl|InsertColumn|Inserts a column into a list control when in report view.
PyObject *PyCListCtrl_InsertColumn(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    int iColNo;
    PyObject *obLVCol;

    if (!(pList = GetListCtrl(self)))
        return NULL;

    if (!PyArg_ParseTuple(args, "iO:InsertColumn",
                          &iColNo,    // @pyparm int|colNo||The new column number
                          &obLVCol))  // @pyparm <o LV_COLUMN>|item||A tuple describing the new column.
        return NULL;
    LV_COLUMN lvCol;
    if (!PyWinObject_AsLV_COLUMN(obLVCol, &lvCol))
        return NULL;
    GUI_BGN_SAVE;
    int ret = pList->InsertColumn(iColNo, &lvCol);
    GUI_END_SAVE;
    PyWinObject_FreeLV_COLUMN(&lvCol);
    if (ret == -1)
        RETURN_ERR("InsertColumn failed");
    return Py_BuildValue("i", ret);
}

// @pymethod int|PyCListCtrl|SetColumn|Changes column state in a list control when in report view.
PyObject *PyCListCtrl_SetColumn(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    int iColNo;
    PyObject *obLVCol;

    if (!(pList = GetListCtrl(self)))
        return NULL;

    if (!PyArg_ParseTuple(args, "iO:InsertColumn",
                          &iColNo,    // @pyparm int|colNo||The to be modified column number
                          &obLVCol))  // @pyparm <o LV_COLUMN>|item||A tuple describing the modified column.
        return NULL;
    LV_COLUMN lvCol;
    if (!PyWinObject_AsLV_COLUMN(obLVCol, &lvCol))
        return NULL;
    GUI_BGN_SAVE;
    int ret = pList->SetColumn(iColNo, &lvCol);
    GUI_END_SAVE;
    PyWinObject_FreeLV_COLUMN(&lvCol);
    if (ret == -1)
        RETURN_ERR("SetColumn failed");
    return PyInt_FromLong(ret);
}

// @pymethod int|PyCListCtrl|InsertItem|Inserts an item into the list.
PyObject *PyCListCtrl_InsertItem(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    int ret;
    int item;
    TCHAR *text = NULL;
    int image;
    PyObject *obLVItem, *obtext;
    if (!(pList = GetListCtrl(self)))
        return NULL;

    if (PyArg_ParseTuple(args, "iOi:InsertItem",
                         &item,    // @pyparmalt1 int|item||The index of the item.
                         &obtext,  // @pyparmalt1 string|text||The text of the item.
                         &image)   // @pyparmalt1 int|image||The index of the image to use.
        && PyWinObject_AsTCHAR(obtext, &text, FALSE)) {
        GUI_BGN_SAVE;
        ret = pList->InsertItem(item, text, image);
        GUI_END_SAVE;
    }
    else {
        PyErr_Clear();
        if (PyArg_ParseTuple(args, "iO:InsertItem",
                             &item,    // @pyparmalt2 int|item||The index of the item.
                             &obtext)  // @pyparmalt2 string|text||The text of the item.
            && PyWinObject_AsTCHAR(obtext, &text, FALSE)) {
            GUI_BGN_SAVE;
            ret = pList->InsertItem(item, text);
            GUI_END_SAVE;
        }
        else {
            PyErr_Clear();
            if (PyArg_ParseTuple(args, "O:InsertItem",
                                 &obLVItem)) {  // @pyparm <o LV_ITEM>|item||A tuple describing the new item.
                LV_ITEM lvItem;
                if (!PyWinObject_AsLV_ITEM(obLVItem, &lvItem))
                    return NULL;
                GUI_BGN_SAVE;
                ret = pList->InsertItem(&lvItem);
                GUI_END_SAVE;
                PyWinObject_FreeLV_ITEM(&lvItem);
            }
            else {
                PyErr_Clear();
                RETURN_ERR("InsertItem requires (item, text, image), (item, text), or (itemObject)");
            }
        }
    }
    PyWinObject_FreeTCHAR(text);
    if (ret == -1)
        RETURN_ERR("InsertItem failed");
    return PyInt_FromLong(ret);
}

// @pymethod int|PyCListCtrl|SetItem|Sets some of all of an items attributes.
PyObject *PyCListCtrl_SetItem(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    PyObject *obLVItem;
    if (!(pList = GetListCtrl(self)))
        return NULL;
    if (!PyArg_ParseTuple(args, "O:SetItem",
                          &obLVItem))  // @pyparm <o LV_ITEM>|item||A tuple describing the new item.
        return NULL;
    LV_ITEM lvItem;
    if (!PyWinObject_AsLV_ITEM(obLVItem, &lvItem))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetItem(&lvItem);
    GUI_END_SAVE;
    PyWinObject_FreeLV_ITEM(&lvItem);
    if (!ok)
        RETURN_ERR("SetItem failed");
    RETURN_NONE;
}

// @pymethod int|PyCListCtrl|SetImageList|Assigns an image list to a list view control.
PyObject *PyCListCtrl_SetImageList(PyObject *self, PyObject *args)
{
    CListCtrl *pList;
    PyObject *obList;
    int imageType;
    if (!(pList = GetListCtrl(self)))
        return NULL;
    if (!PyArg_ParseTuple(args, "Oi:SetImageList",
                          &obList,      // @pyparm <o PyCImageList>|imageList||The Image List to use.
                          &imageType))  // @pyparm int|imageType||Type of image list. It can be one of (COMMCTRL.)
                                        // LVSIL_NORMAL, LVSIL_SMALL or LVSIL_STATE
        return NULL;
    CImageList *pImageList = PyCImageList::GetImageList(obList);
    if (pImageList == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CImageList *pOldList = pList->SetImageList(pImageList, imageType);
    GUI_END_SAVE;
    if (pOldList == NULL)
        RETURN_NONE;
    return ui_assoc_object::make(PyCImageList::type, pOldList)->GetGoodRet();
}

// @pymethod <o LV_COLUMN>|PyCListCtrl|GetColumn|Retrieves the details of a column in the control.
PyObject *PyCListCtrl_GetColumn(PyObject *self, PyObject *args)
{
    int col;
    if (!PyArg_ParseTuple(args, "i:GetColumn",
                          &col))  // @pyparm int|column||The index of the column whose attributes are to be retrieved.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    TCHAR textBuf[256];
    LV_COLUMN lvItem;
    lvItem.pszText = textBuf;
    lvItem.cchTextMax = sizeof(textBuf) / sizeof(TCHAR);
    lvItem.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    GUI_BGN_SAVE;
    BOOL ok = pList->GetColumn(col, &lvItem);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetColumn failed");
    return PyWinObject_FromLV_COLUMN(&lvItem);
}

// @pymethod int|PyCListCtrl|DeleteColumn|Deletes the specified column from the list control.
PyObject *PyCListCtrl_DeleteColumn(PyObject *self, PyObject *args)
{
    int col;
    if (!PyArg_ParseTuple(args, "i:DeleteColumn",
                          &col))  // @pyparm int|first||Index of the column to be removed.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->DeleteColumn(col);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("DeleteColumn failed");
    RETURN_NONE;
}

// @pymethod int|PyCListCtrl|SetColumnWidth|Sets the width of the specified column in the list control.
PyObject *PyCListCtrl_SetColumnWidth(PyObject *self, PyObject *args)
{
    int col, width;
    if (!PyArg_ParseTuple(args, "ii:SetColumnWidth",
                          &col,     // @pyparm int|first||Index of the column to be changed.
                          &width))  // @pyparm int|first||New width of the column.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetColumnWidth(col, width);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetColumnWidth failed");
    RETURN_NONE;
}

// @pymethod int|PyCListCtrl|GetColumnWidth|Gets the width of the specified column in the list control.
PyObject *PyCListCtrl_GetColumnWidth(PyObject *self, PyObject *args)
{
    int col;
    if (!PyArg_ParseTuple(args, "i:GetColumnWidth",
                          &col))  // @pyparm int|first||Index of the column whose width is to be retrieved.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    int width = pList->GetColumnWidth(col);
    GUI_END_SAVE;
    return Py_BuildValue("i", width);
}

// @pymethod int|PyCListCtrl|GetStringWidth|Gets the necessary column width to fully display this text in a column.
PyObject *PyCListCtrl_GetStringWidth(PyObject *self, PyObject *args)
{
    TCHAR *text;
    PyObject *obtext;
    if (!PyArg_ParseTuple(
            args, "O:GetStringWidth",
            &obtext))  // @pyparm int|first||String that contains the text whose width is to be determined.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    int width = pList->GetStringWidth(text);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    return Py_BuildValue("i", width);
    // @comm Doesn't take the size of an included Image in account, only the size of the text is determined.
}

// @pymethod <o LV_ITEM>|PyCListCtrl|GetItem|Retrieves the details of an items attributes.
PyObject *PyCListCtrl_GetItem(PyObject *self, PyObject *args)
{
    int item, sub = 0;
    if (!PyArg_ParseTuple(args, "i|i:GetItem",
                          &item,  // @pyparm int|item||The index of the item whose attributes are to be retrieved.
                          &sub))  // @pyparm int|sub||Specifies the subitem whose text is to be retrieved.
        return NULL;

    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    TCHAR textBuf[256];
    LV_ITEM lvItem;
    lvItem.iItem = item;
    lvItem.iSubItem = sub;
    lvItem.pszText = textBuf;
    lvItem.cchTextMax = sizeof(textBuf) / sizeof(TCHAR);
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    GUI_BGN_SAVE;
    BOOL ok = pList->GetItem(&lvItem);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetItem failed");
    return PyWinObject_FromLV_ITEM(&lvItem);
}

// @pymethod int|PyCListCtrl|GetItemText|Retrieves the text of a list view item or subitem.
PyObject *PyCListCtrl_GetItemText(PyObject *self, PyObject *args)
{
    int item, sub;
    // TCHAR buf[256];
    if (!PyArg_ParseTuple(args, "ii:GetItemText",
                          &item,  // @pyparm int|item||The index of the item whose text is to be retrieved.
                          &sub))  // @pyparm int|sub||Specifies the subitem whose text is to be retrieved.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    // int len = pList->GetItemText(item, sub, buf, sizeof(buf)/sizeof(TCHAR));
    CString s = pList->GetItemText(item, sub);
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(s);
}

// @pymethod int|PyCListCtrl|SetItemText|Changes the text of a list view item or subitem.
PyObject *PyCListCtrl_SetItemText(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item, sub;
    TCHAR *text;
    PyObject *obtext;
    if (!PyArg_ParseTuple(args, "iiO:SetItemText",
                          &item,     // @pyparm int|item||Index of the item whose text is to be set.
                          &sub,      // @pyparm int|sub||Index of the subitem, or zero to set the item label.
                          &obtext))  // @pyparm string|text||String that contains the new item text.

        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetItemText(item, sub, text);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    if (!ok)
        RETURN_ERR("SetItemText failed");
    RETURN_NONE;
}

// @pymethod int|PyCListCtrl|GetItemState|Retrieves the state of a list view item.
PyObject *PyCListCtrl_GetItemState(PyObject *self, PyObject *args)
{
    int item, mask;
    if (!PyArg_ParseTuple(args, "ii:GetItemState",
                          &item,   // @pyparm int|item||The index of the item whose position is to be retrieved.
                          &mask))  // @pyparm int|mask||Mask specifying which of the item's state flags to return.

        return NULL;
    GUI_BGN_SAVE;
    CListCtrl *pList = GetListCtrl(self);
    GUI_END_SAVE;
    if (!pList)
        return NULL;
    return Py_BuildValue("i", pList->GetItemState(item, mask));
}

// @pymethod int|PyCListCtrl|SetItemState|Changes the state of an item in a list view control.
PyObject *PyCListCtrl_SetItemState(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item, state, mask;
    if (!PyArg_ParseTuple(args, "iii:SetItemState",
                          &item,   // @pyparm int|item||Index of the item whose state is to be set.
                          &state,  // @pyparm int|state||New values for the state bits.
                          &mask))  // @pyparm int|mask||Mask specifying which state bits to change.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetItemState(item, state, mask);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemState failed");
    RETURN_NONE;
}
// @pymethod object|PyCListCtrl|GetItemData|Retrieves the application-specific value associated with an item.
PyObject *PyCListCtrl_GetItemData(PyObject *self, PyObject *args)
{
    int item;
    if (!PyArg_ParseTuple(args, "i:GetItemData",
                          &item))  // @pyparm int|item||The index of the item whose data is to be retrieved.

        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    PyObject *ret = PyWin_GetPythonObjectFromLong(pList->GetItemData(item));
    GUI_END_SAVE;
    // inc ref count for return value.
    Py_XINCREF(ret);
    return ret;
}

// @pymethod int|PyCListCtrl|SetItemData|Sets the item's application-specific value.
PyObject *PyCListCtrl_SetItemData(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    PyObject *data;
    if (!PyArg_ParseTuple(args, "iO:SetItemData",
                          &item,   // @pyparm int|item||Index of the item whose Data is to be set.
                          &data))  // @pyparm object|Data||New value for the data.
        return NULL;
    if (data == Py_None)
        data = NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetItemData(item, (DWORD_PTR)data);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemData failed");
    // @comm Note that a reference count is not added to the object.  This it is your
    // responsibility to make sure the object remains alive while in the list.
    RETURN_NONE;
}

// @pymethod int|PyCListCtrl|GetNextItem|Searches for a list view item with specified properties and with specified
// relationship to a given item.
PyObject *PyCListCtrl_GetNextItem(PyObject *self, PyObject *args)
{
    int item, flags;
    if (!PyArg_ParseTuple(
            args, "ii:GetNextItem",
            &item,    // @pyparm int|item||Index of the item to begin the searching with, or -1 to find the first item
                      // that matches the specified flags. The specified item itself is excluded from the search.
            &flags))  // @pyparm int|flags||Geometric relation of the requested item to the specified item,
                      // and the state of the requested item. The geometric relation can be one of these values:
                      // <nl>LVNI_ABOVE<nl>LVNI_ALL<nl>LVNI_BELOW<nl>LVNI_TOLEFT<nl>LVNI_TORIGHT<nl>
                      // The state can be zero, or it can be one or more of these values:
                      // <nl>LVNI_DROPHILITED<nl>LVNI_FOCUSED<nl>LVNI_HIDDEN<nl>LVNI_MARKED<nl>LVNI_SELECTED<nl>
                      // If an item does not have all of the specified state flags set, the search continues with the
                      // next item.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pList->GetNextItem(item, flags);
    GUI_END_SAVE;
    if (rc == (int)-1)
        RETURN_ERR("GetNextItem failed");
    return Py_BuildValue("i", rc);
    // @rdesc Returns an integer index, or raises a win32ui.error exception if not item can be found.
}

// @pymethod int|PyCListCtrl|RedrawItems|Forces a listview to repaint a range of items.
PyObject *PyCListCtrl_RedrawItems(PyObject *self, PyObject *args)
{
    int first, last;
    if (!PyArg_ParseTuple(args, "ii:RedrawItems",
                          &first,  // @pyparm int|first||Index of the first item to be repainted.
                          &last))  // @pyparm int|first||Index of the last item to be repainted.
        return NULL;
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->RedrawItems(first, last);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("RedrawItems failed");
    RETURN_NONE;
    // @comm The specified items are not actually repainted until the list view window receives a WM_PAINT message.
    // To repaint immediately, call the Windows UpdateWindow function after using this function.
}

// @pymethod (int, int, int, int)|PyCListCtrl|GetItemRect|Retrieves the bounding rectangle of a list view item.
PyObject *PyCListCtrl_GetItemRect(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    RECT rect;
    BOOL bTextOnly;
    if (!PyArg_ParseTuple(args, "ii:GetItemRect",
                          &item,        // @pyparm int|item||Index of the item whose Data is to be set.
                          &bTextOnly))  // @pyparm int|bTextOnly||f this parameter is nonzero, the bounding rectangle
                                        // includes only the text of the item. Otherwise it includes the entire line
                                        // that the item occupies in the list view control.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->GetItemRect(item, &rect, bTextOnly);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetItemRect failed");
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod <o PyCEdit>|PyCListCtrl|GetEditControl|Retrieves the handle of the edit control used to edit the specified
// list view item.
PyObject *PyCListCtrl_GetEditControl(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetEditControl"))
        return NULL;
    GUI_BGN_SAVE;
    CEdit *pEdit = pList->GetEditControl();
    GUI_END_SAVE;
    if (pEdit == NULL)
        RETURN_ERR("GetEditControl failed");
    return ui_assoc_object::make(UITypeFromCObject(pEdit), pEdit)->GetGoodRet();
}

// @pymethod <o PyCEdit>|PyCListCtrl|EditLabel|Edits a specified list view item in-place.
PyObject *PyCListCtrl_EditLabel(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    // @pyparm int|item||The index of item to edit.
    if (!PyArg_ParseTuple(args, "i:EditLabel", &item))
        return NULL;
    GUI_BGN_SAVE;
    CEdit *pEdit = pList->EditLabel(item);
    GUI_END_SAVE;
    if (pEdit == NULL)
        RETURN_ERR("EditLabel failed");
    return ui_assoc_object::make(UITypeFromCObject(pEdit), pEdit)->GetGoodRet();
}

// @pymethod int|PyCListCtrl|EnsureVisible|Ensures that a list view item is visible in its list view control.
PyObject *PyCListCtrl_EnsureVisible(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    BOOL bPartialOK;
    // @pyparm int|item||The index of item to edit.
    // @pyparm int|bPartialOK||Specifies whether partial visibility is acceptable.
    if (!PyArg_ParseTuple(args, "ii:EnsureVisible", &item, &bPartialOK))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->EnsureVisible(item, bPartialOK);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("EnsureVisible failed");
    RETURN_NONE;
}

// @pymethod <o PyCImageList>,(x,y)|PyCListCtrl|CreateDragImage|Creates a dragging bitmap for the specified list view
// item.
PyObject *PyCListCtrl_CreateDragImage(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    // @pyparm int|item||The index of the item to edit.
    if (!PyArg_ParseTuple(args, "i:CreateDragImage", &item))
        return NULL;
    POINT pt;
    CImageList *pIL = pList->CreateDragImage(item, &pt);
    if (pIL == NULL)
        RETURN_ERR("CreateDragImage failed");
    PyObject *newOb = ui_assoc_object::make(PyCImageList::type, pIL)->GetGoodRet();
    PyObject *ret = Py_BuildValue("O(ii)", newOb, pt.x, pt.y);
    Py_DECREF(newOb);
    return ret;
}

// @pymethod (int, int, int)|PyCListCtrl|HitTest|Determines which list view item, if any, is at a specified position.
PyObject *PyCListCtrl_HitTest(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    LVHITTESTINFO i;
    memset(&i, 0, sizeof(i));
    // @pyparm point|x,y||The point to test.
    if (!PyArg_ParseTuple(args, "(ii):HitTest", &i.pt.x, &i.pt.y))
        return NULL;
    GUI_BGN_SAVE;
    pList->HitTest(&i);
    GUI_END_SAVE;
    return Py_BuildValue("iii", i.flags, i.iItem, i.iSubItem);
    // @rdesc The result is a tuple of (flags, item, subItem).
    // flags may be a combination of the following values:
    // @flagh Value|Description
    // @flag commctrl.LVHT_ABOVE|The position is above the control's client area.
    // @flag commctrl.LVHT_BELOW|The position is below the control's client area.
    // @flag commctrl.LVHT_NOWHERE|The position is inside the list view control's client window, but it is not over a
    // list item.
    // @flag commctrl.LVHT_ONITEMICON|The position is over a list view item's icon.
    // @flag commctrl.LVHT_ONITEMLABEL|The position is over a list view item's text.
    // @flag commctrl.LVHT_ONITEMSTATEICON|The position is over the state image of a list view item.
    // @flag commctrl.LVHT_TOLEFT|The position is to the left of the list view control's client area.
    // @flag commctrl.LVHT_TORIGHT|The position is to the right of the list view control's client area.
}

// @pymethod (int, int)|PyCListCtrl|GetItemPosition|Determines the position of the specified item.
PyObject *PyCListCtrl_GetItemPosition(PyObject *self, PyObject *args)
{
    CListCtrl *pList = GetListCtrl(self);
    if (!pList)
        return NULL;
    int item;
    // @pyparm int|item||The item to determine the position for.
    if (!PyArg_ParseTuple(args, "i:GetItemPosition", &item))
        return NULL;
    POINT pt;
    GUI_BGN_SAVE;
    BOOL ok = pList->GetItemPosition(item, &pt);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetItemPosition failed");
    return Py_BuildValue("(ii)", pt.x, pt.y);
}

// @object PyCListCtrl|A class which encapsulates an MFC CListCtrl object.  Derived from a <o PyCWnd> object.
static struct PyMethodDef PyCListCtrl_methods[] = {
    {"Arrange", PyCListCtrl_Arrange, 1},            // @pymeth Arrange|Aligns items on a grid.
    {"CreateWindow", PyCListCtrl_CreateWindow, 1},  // @pymeth CreateWindow|Creates the actual window for the object.
    {"DeleteAllItems", PyCListCtrl_DeleteAllItems, 1},  // @pymeth DeleteAllItems|Deletes all items from the list.
    {"DeleteItem", PyCListCtrl_DeleteItem, 1},          // @pymeth DeleteItem|Deletes the specified item.
    {"GetTextColor", PyCListCtrl_GetTextColor,
     1},  // @pymeth GetTextColor|Retrieves the text color of a list view control.
    {"SetTextColor", PyCListCtrl_SetTextColor, 1},  // @pymeth SetTextColor|Sets the text color of a list view control.
    {"GetBkColor", PyCListCtrl_GetBkColor, 1},      // @pymeth GetBkColor|Retrieves the background color of the control.
    {"SetBkColor", PyCListCtrl_SetBkColor, 1},      // @pymeth SetBkColor|Sets the background color of the control.
    {"GetItem", PyCListCtrl_GetItem, 1},            // @pymeth GetItem|Retrieves the details of an items attributes.
    {"GetItemCount", PyCListCtrl_GetItemCount,
     1},  // @pymeth GetItemCount|Retrieves the number of items in a list view control.
    {"GetItemRect", PyCListCtrl_GetItemRect,
     1},  // @pymeth GetItemRect|Retrieves the bounding rectangle of a list view item.
    {"GetEditControl", PyCListCtrl_GetEditControl,
     1},  // @pymeth GetEditControl|Retrieves the handle of the edit control used to edit the specified list view item.
    {"EditLabel", PyCListCtrl_EditLabel, 1},  // @pymeth EditLabel|Edits a specified list view item in-place.
    {"EnsureVisible", PyCListCtrl_EnsureVisible,
     1},  // @pymeth EnsureVisible|Ensures that a list view item is visible in its list view control.
    {"CreateDragImage", PyCListCtrl_CreateDragImage,
     1},  // @pymeth CreateDragImage|Creates a dragging bitmap for the specified list view item.
    {"GetImageList", PyCListCtrl_GetImageList, 1},  // @pymeth GetImageList|Retrieves the current image list.
    {"GetNextItem", PyCListCtrl_GetNextItem, 1},    // @pymeth GetNextItem|Searches for a list view item with specified
                                                    // properties and with specified relationship to a given item.
    {"InsertColumn", PyCListCtrl_InsertColumn,
     1},  // @pymeth InsertColumn|Inserts a column into a list control when in report view.
    {"InsertItem", PyCListCtrl_InsertItem, 1},  // @pymeth InsertItem|Inserts an item into the list.
    {"SetImageList", PyCListCtrl_SetImageList,
     1},                                      // @pymeth SetImageList|Assigns an image list to a list view control.
    {"GetColumn", PyCListCtrl_GetColumn, 1},  // @pymeth GetColumn|Retrieves the details of a column in the control.
    {"GetTextBkColor", PyCListCtrl_GetTextBkColor,
     1},  // @pymeth GetTextBkColor|Retrieves the text background color of a list view control.
    {"SetTextBkColor", PyCListCtrl_SetTextBkColor,
     1},  // @pymeth SetTextBkColor|Sets the text background color of a list view control.
    {"GetTopIndex", PyCListCtrl_GetTopIndex,
     1},  // @pymeth GetTopIndex|Retrieves the index of the topmost visible item.
    {"GetCountPerPage", PyCListCtrl_GetCountPerPage,
     1},  // @pymeth GetCountPerPage|Calculates the number of items that can fit vertically in a list view control.
    {"GetSelectedCount", PyCListCtrl_GetSelectedCount,
     1},  // @pymeth GetSelectedCount|Retrieves the number of selected items in the list view control.
    {"SetItem", PyCListCtrl_SetItem, 1},  // @pymeth SetItem|Sets some of all of an items attributes.
    {"SetItemState", PyCListCtrl_SetItemState,
     1},  // @pymeth SetItemState|Changes the state of an item in a list view control.
    {"GetItemState", PyCListCtrl_GetItemState, 1},  // @pymeth GetItemState|Retrieves the state of a list view item.
    {"SetItemData", PyCListCtrl_SetItemData, 1},    // @pymeth SetItemData|Sets the item's application-specific value.
    {"GetItemData", PyCListCtrl_GetItemData,
     1},  // @pymeth GetItemData|Retrieves the application-specific value associated with an item.
    {"SetItemCount", PyCListCtrl_SetItemCount,
     1},  // @pymeth SetItemCount|Prepares a list view control for adding a large number of items.
    {"GetItemCount", PyCListCtrl_GetItemCount,
     1},  // @pymeth GetItemCount|Retrieves the number of items in a list view control.
    {"SetItemText", PyCListCtrl_SetItemText,
     1},  // @pymeth SetItemText|Changes the text of a list view item or subitem.
    {"GetItemText", PyCListCtrl_GetItemText,
     1},  // @pymeth GetItemText|Retrieves the text of a list view item or subitem.
    {"RedrawItems", PyCListCtrl_RedrawItems, 1},  // @pymeth RedrawItems|Redraws a range of items
    {"Update", PyCListCtrl_Update, 1},            // @pymeth Update|Forces the control to repaint a specified item.

    {"SetColumn", PyCListCtrl_SetColumn,
     1},  // @pymeth SetColumn|Sets the state of a column in a list control when in report view.
    {"DeleteColumn", PyCListCtrl_DeleteColumn,
     1},  // @pymeth DeleteColumn|Deletes the specified column from the list control.
    {"GetColumnWidth", PyCListCtrl_GetColumnWidth,
     1},  // @pymeth GetColumnWidth|Gets the width of the specified column in the list control.
    {"SetColumnWidth", PyCListCtrl_SetColumnWidth,
     1},  // @pymeth SetColumnWidth|Sets the width of the specified column in the list control.
    {"GetStringWidth", PyCListCtrl_GetStringWidth,
     1},  // @pymeth GetStringWidth|Gets the necessary column width to fully display this text in a column.
    {"HitTest", PyCListCtrl_HitTest,
     1},  // @pymeth HitTest|Determines which list view item, if any, is at a specified position.
    {"GetItemPosition", PyCListCtrl_GetItemPosition,
     1},  // @pymeth GetItemPosition|Determines the position of the specified item.
    {NULL, NULL}};

ui_type_CObject PyCListCtrl::type("PyCListCtrl", &PyCWnd::type, RUNTIME_CLASS(CListCtrl), sizeof(PyCListCtrl),
                                  PYOBJ_OFFSET(PyCListCtrl), PyCListCtrl_methods, GET_PY_CTOR(PyCListCtrl));
