/* win32control : implementation file

    Control object - base class for listboxes, editbox's, prompts, etc.

    Created August 1994, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32control.h"

#include "win32gdi.h"
#include "win32bitmap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define IS_LB_ERR(rc) (rc == LB_ERR || rc == LB_ERRSPACE)
#define IS_CB_ERR(rc) (rc == CB_ERR || rc == CB_ERRSPACE)

///////////////////////////////////////////////////////////////////////////
//
// The control objects.
//

///////////////////////////////////////////////////////////////////////////
//
// The control objects.
//
ui_control_object::ui_control_object() {}
ui_control_object::~ui_control_object() {}

/////////////////////////////////////////////////////////////////////
//
// ui_control_object
//
// @object PyCControl|A windows abstract control.  Derived from a <o PyCWnd> object.
static struct PyMethodDef ui_control_object_methods[] = {
    {NULL, NULL} /* sentinel */
};

ui_type_CObject ui_control_object::type("PyCControl", &PyCWnd::type, RUNTIME_CLASS(CObject), sizeof(ui_control_object),
                                        PYOBJ_OFFSET(ui_control_object), ui_control_object_methods, NULL);

/////////////////////////////////////////////////////////////////////
//
// PyCButton
//
static CButton *GetButton(PyObject *self)
{
    // note we can only ask for a CWnd, if the LB is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CButton *)PyCWnd::GetPythonGenericWnd(self);
}
PyCButton::PyCButton() {}
PyCButton::~PyCButton() {}
// @pymethod <o PyCButton>|win32ui|CreateButton|Creates a button object.  <om PyCButton.CreateWindow> creates the actual
// control.
PyObject *PyCButton_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CButton *pBut = new CButton();
    return ui_assoc_object::make(PyCButton::type, pBut);
}

// @pymethod |PyCButton|CreateWindow|Creates the window for a new button object.
static PyObject *PyCButton_create_window(PyObject *self, PyObject *args)
{
    TCHAR *caption;
    int style, id;
    PyObject *obParent, *obcaption;
    RECT rect;
    if (!PyArg_ParseTuple(
            args, "Oi(iiii)Oi:CreateWindow",
            &obcaption,  // @pyparm string|caption||The caption (text) for the button.
            &style,      // @pyparm int|style||The style for the button.  Use any of the win32con.BS_* constants.
            &rect.left, &rect.top, &rect.right, &rect.bottom,
            // @pyparm (left, top, right, bottom)|rect||The size and position of the button.
            &obParent,  // @pyparm <o PyCWnd>|parent||The parent window of the button.  Usually a <o PyCDialog>.
            &id))       // @pyparm int|id||The buttons control ID.
        return NULL;

    if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
        RETURN_TYPE_ERR("parent argument must be a window object");
    CWnd *pParent = GetWndPtr(obParent);
    if (pParent == NULL)
        return NULL;
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    if (!PyWinObject_AsTCHAR(obcaption, &caption, FALSE))
        return NULL;
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pBut->Create(caption, style, rect, pParent, id);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(caption);
    if (!ok)
        RETURN_ERR("CButton::Create");
    RETURN_NONE;
}

// @pymethod int|PyCButton|GetCheck|Retrieves the check state of a radio button or check box.
static PyObject *PyCButton_get_check(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pBut->GetCheck();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod |PyCButton|SetCheck|Sets or resets the state of a radio button or check box.
static PyObject *PyCButton_set_check(PyObject *self, PyObject *args)
{
    int check;
    if (!PyArg_ParseTuple(args, "i", &check))  // @pyparm int|idCheck||The ID of the button.
        return NULL;
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    pBut->SetCheck(check);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCButton|GetState|Returns the state of the button.
static PyObject *PyCButton_get_state(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pBut->GetState();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod int|PyCButton|SetState|Sets the state of the button.
static PyObject *PyCButton_set_state(PyObject *self, PyObject *args)
{
    int state;
    if (!PyArg_ParseTuple(args, "i", &state))  // @pyparm int|bHighlight||The new state for the button.
        return NULL;
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    pBut->SetState(state);
    GUI_END_SAVE;
    return Py_BuildValue("i", state);
    // @comm Highlighting affects the exterior of a button control. It has no effect on the check state of a radio
    // button or check box.
}
// @pymethod int|PyCButton|GetButtonStyle|Gets the style of the button.
static PyObject *PyCButton_get_style(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pBut->GetButtonStyle();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod int|PyCButton|SetButtonStyle|Sets the style of the button.
static PyObject *PyCButton_set_style(PyObject *self, PyObject *args)
{
    int style;
    BOOL bRedraw = TRUE;
    if (!PyArg_ParseTuple(args, "i|i",
                          &style,     // @pyparm int|style||The new style for the button.
                          &bRedraw))  // @pyparm int|bRedraw|1|Should the button be redrawn?
        return NULL;
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    pBut->SetButtonStyle(style, bRedraw);
    GUI_END_SAVE;
    return Py_BuildValue("i", style);
}
// @pymethod int|PyCButton|SetBitmap|Set the button's bitmap
static PyObject *PyCButton_set_bitmap(PyObject *self, PyObject *args)
{
    PyObject *obBitmap;
    if (!PyArg_ParseTuple(args, "O",
                          &obBitmap))  // @pyparm int|hBitmap|1|Handle of the new bitmap
        return NULL;
    HBITMAP hBitmap;
    if (!PyWinObject_AsHANDLE(obBitmap, (HANDLE *)&hBitmap))
        return NULL;
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    HBITMAP rc = pBut->SetBitmap(hBitmap);
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(rc);
}
// @pymethod int|PyCButton|GetBitmap|Get the button's bitmap
static PyObject *PyCButton_get_bitmap(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CButton *pBut = GetButton(self);
    if (!pBut)
        return NULL;
    GUI_BGN_SAVE;
    HBITMAP rc = pBut->GetBitmap();
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(rc);
}

// @object PyCButton|A windows button.  Encapsulates an MFC <c CButton> class.  Derived from <o PyCControl>.
static struct PyMethodDef PyCButton_methods[] = {
    {"CreateWindow", PyCButton_create_window, 1},  // @pymeth CreateWindow|Creates the window for a new button object.
    {"GetBitmap", PyCButton_get_bitmap, 1},        // @pymeth GetBitmap|Retrieves the bitmap associated with the button.
    {"SetBitmap", PyCButton_set_bitmap, 1},        // @pymeth SetBitmap|Sets the bitmap of a button.
    {"GetCheck", PyCButton_get_check, 1},  // @pymeth GetCheck|Retrieves the check state of a radio button or check box.
    {"SetCheck", PyCButton_set_check, 1},  // @pymeth SetCheck|Sets the check state of a radio button or check box.
    {"GetState", PyCButton_get_state, 1},  // @pymeth GetState|Retrieves the state of a radio button or check box.
    {"SetState", PyCButton_set_state, 1},  // @pymeth SetState|Sets the state of a radio button or check box.
    {"GetButtonStyle", PyCButton_get_style,
     1},  // @pymeth GetButtonStyle|Retrieves the style of a radio button or check box.
    {"SetButtonStyle", PyCButton_set_style,
     1},  // @pymeth SetButtonStyle|Sets the state of a radio button or check box.
    {NULL, NULL}};

ui_type_CObject PyCButton::type("PyCButton", &ui_control_object::type, RUNTIME_CLASS(CButton), sizeof(PyCButton),
                                PYOBJ_OFFSET(PyCButton), PyCButton_methods, GET_PY_CTOR(PyCButton));

/////////////////////////////////////////////////////////////////////
//
// PyCListBox
//
static CListBox *GetListBox(PyObject *self)
{
    // note we can only ask for a CWnd, if the LB is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CListBox *)PyCWnd::GetPythonGenericWnd(self);
}

PyCListBox::PyCListBox() {}
PyCListBox::~PyCListBox() {}
// @pymethod int|PyCListBox|AddString|Adds a string to a listbox.
static PyObject *PyCListBox_add_string(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    PyObject *ob;
    if (!PyArg_ParseTuple(args, "O", &ob))  // @pyparm any|object||Any object.  If not a string, __str__, __repr__ or a
                                            // default repr() will be used
        return NULL;
    CString cstrRepr = GetReprText(ob);
    //@pyseemfc CListBox|AddString
    GUI_BGN_SAVE;
    int rc = pLB->AddString(cstrRepr);
    GUI_END_SAVE;
    if (IS_LB_ERR(rc))
        RETURN_ERR("PyCListBox.AddString failed");
    return Py_BuildValue("i", rc);
    //@rdesc The zero based index of the new string.
}
// @pymethod int|PyCListBox|DeleteString|Deletes an item from a listbox.
static PyObject *PyCListBox_delete_string(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i", &pos))  // @pyparm int|pos||The zero based index of the item to delete.
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->DeleteString(pos);  // @pyseemfc CListBox|DeleteString
    GUI_END_SAVE;
    if (IS_LB_ERR(rc))
        RETURN_ERR("PyCListBox.DeleteString failed");
    return Py_BuildValue("i", rc);
    // @rdesc The count of the items remaining in the list.
}
// @pymethod int|PyCListBox|Dir|Fills a listbox with a directory listing.
static PyObject *PyCListBox_dir(PyObject *self, PyObject *args)
{
    int attr;
    TCHAR *szWild;
    PyObject *obWild;
    if (!PyArg_ParseTuple(args, "iO",
                          &attr,     // @pyparm int|attr||The attributes of the files to locate
                          &obWild))  // @pyparm string|wild||A file specification string - eg, *.*
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    if (!PyWinObject_AsTCHAR(obWild, &szWild, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->Dir(attr, szWild);  // @pyseemfc CListBox|Dir
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szWild);
    if (IS_LB_ERR(rc))
        RETURN_ERR("PyCListBox.Dir failed");
    return Py_BuildValue("i", rc);
    // @rdesc The index of the last file name added to the list.
}

// @pymethod int|PyCListBox|InsertString|Insert a string into a listbox.
static PyObject *PyCListBox_insert_string(PyObject *self, PyObject *args)
{
    int pos;
    PyObject *ob;
    if (!PyArg_ParseTuple(args, "iO",
                          &pos,  // @pyparm int|pos||The zero based index in the listbox to insert the new string
                          &ob))  // @pyparm any|object||The object to be added to the listbox
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    CString cstrRepr = GetReprText(ob);
    GUI_BGN_SAVE;
    int rc = pLB->InsertString(pos, cstrRepr);  // @pyseemfc CListBox|InsertString
    GUI_END_SAVE;
    if (IS_LB_ERR(rc))
        RETURN_ERR("PyCListBox.InsertString failed");
    return Py_BuildValue("i", rc);
    // @rdesc The zero based index of the new string added.
}
// @pymethod |PyCListBox|ResetContent|Clear all the items from a listbox.
static PyObject *PyCListBox_reset_content(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    pLB->ResetContent();  // @pyseemfc CListBox|ResetContent
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCListBox|GetCaretIndex|Returns the index of the item which has focus.
static PyObject *PyCListBox_get_caret_index(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);  // pyseemfc CListBox|GetCaretIndex
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetCaretIndex();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
    // @rdesc The zero-based index of the item that has the focus rectangle in a list box.
    // If the list box is a single-selection list box, the return value is the index of the item that is selected, if
    // any.
}

// @pymethod int|PyCListBox|GetCount|Returns the count of items in the listbox.
static PyObject *PyCListBox_get_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetCount
                                    // @rdesc Returns the number of items currently in the listbox.
}
// @pymethod int|PyCListBox|GetCurSel|Returns the index of the currently selected item.
static PyObject *PyCListBox_get_cur_sel(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetCurSel();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetCurSel
                                    //@comm Should not be called for a multiple selection listbox.
}
// @pymethod object|PyCListBox|GetItemData|Retrieves the application-specific object associated with an item.
PyObject *PyCListBox_GetItemData(PyObject *self, PyObject *args)
{
    int item;
    if (!PyArg_ParseTuple(args, "i:GetItemData",
                          &item))  // @pyparm int|item||The index of the item whose data is to be retrieved.

        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    DWORD_PTR rc = pLB->GetItemData(item);
    GUI_END_SAVE;
    return PyWinObject_FromDWORD_PTR(rc);
}

// @pymethod int|PyCListBox|GetItemValue|Retrieves the application-specific value associated with an item.
PyObject *PyCListBox_GetItemValue(PyObject *self, PyObject *args)
{
    int item;
    if (!PyArg_ParseTuple(args, "i:GetItemValue",
                          &item))  // @pyparm int|item||The index of the item whose data is to be retrieved.

        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    long rc = (long)pLB->GetItemData(item);
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod int|PyCListBox|GetSel|Returns the selection state of a specified item.
static PyObject *PyCListBox_get_sel(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i", &pos))  // @pyparm int|index||The index of the item to return the state for.
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetSel(pos);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetSel
                                    //@rdesc A +ve number if the item is selected, else zero.
}
// @pymethod int|PyCListBox|GetSelCount|Returns the number of selected items in a multiple selection listbox.
static PyObject *PyCListBox_get_sel_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int ret = pLB->GetSelCount();  // @pyseemfc CListBox|GetSelCount
    GUI_END_SAVE;
    if (ret == LB_ERR)
        RETURN_ERR("Listbox is a single selection listbox");
    return Py_BuildValue("i", ret);
}
// @pymethod list|PyCListBox|GetSelItems|Returns a list of the indexes of the currently selected items in a multiple
// selection listbox.
static PyObject *PyCListBox_get_sel_items(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int numItems;
    {
        GUI_BGN_SAVE;
        numItems = pLB->GetSelCount();  // @pyseemfc CListBox|GetSelCount
        GUI_END_SAVE;
    }
    if (numItems == 0)
        return PyList_New(0);  // return an empty list
    if (numItems == LB_ERR)
        RETURN_ERR("Listbox is a single selection listbox");
    int *rgItems = new int[numItems];
    if (rgItems == NULL)
        RETURN_ERR("Memory error");
    GUI_BGN_SAVE;
    int rc = pLB->GetSelItems(numItems, rgItems);
    GUI_END_SAVE;
    if (rc != numItems) {  // @pyseemfc CListBox|GetSelItems
        delete rgItems;
        RETURN_ERR("GetSelItems failed!");
    }
    PyObject *list;
    if ((list = PyList_New(numItems)) == NULL) {
        delete rgItems;
        return NULL;
    }
    for (int i = 0; i < numItems; i++) PyList_SetItem(list, i, Py_BuildValue("i", rgItems[i]));

    delete rgItems;
    return list;
}
// @pymethod list|PyCListBox|GetSelTextItems|Returns a list of the strings of the currently selected items in a multiple
// selection listbox.
static PyObject *PyCListBox_get_sel_text_items(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int numItems;
    {
        GUI_BGN_SAVE;
        numItems = pLB->GetSelCount();  // @pyseemfc CListBox|GetSelCount
        GUI_END_SAVE;
    }
    if (numItems == 0)
        return PyList_New(0);  // return an empty list
    if (numItems == LB_ERR)
        RETURN_ERR("Listbox is a single selection listbox");
    int *rgItems = new int[numItems];
    if (rgItems == NULL)
        RETURN_ERR("Memory error");
    GUI_BGN_SAVE;
    int rc = pLB->GetSelItems(numItems, rgItems);
    GUI_END_SAVE;
    if (rc != numItems) {  // @pyseemfc CListBox|GetSelItems
        delete rgItems;
        RETURN_ERR("GetSelItems failed!");
    }
    PyObject *list;
    if ((list = PyList_New(numItems)) == NULL) {
        delete rgItems;
        return NULL;
    }
    for (int i = 0; i < numItems; i++) {
        CString value;
        GUI_BGN_SAVE;
        pLB->GetText(rgItems[i], value);  // @pyseemfc CListBox|GetText
        GUI_END_SAVE;
        if (PyList_SetItem(list, i, PyWinObject_FromTCHAR(value)) == -1) {
            Py_DECREF(list);
            list = NULL;
            break;
        }
    }

    delete rgItems;
    return list;
}

// @pymethod string|PyCListBox|GetText|Returns the string for a specified item.
static PyObject *PyCListBox_get_text(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i", &pos))  //@pyparm int|index||The index of the item to retrieve the text of
        return NULL;
    CString cs;

    GUI_BGN_SAVE;
    int len = pLB->GetTextLen(pos);
    if (len < 0) {
        GUI_BLOCK_THREADS;
        RETURN_ERR("The item does not exist");
    }
    pLB->GetText(pos, cs.GetBufferSetLength(len));
    cs.ReleaseBuffer();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(cs);
}

// @pymethod int|PyCListBox|GetTextLen|Returns the length of the string for a specified item.
static PyObject *PyCListBox_get_text_len(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i",
                          &pos))  //@pyparm int|index||The index of the item to retrieve the length of the text.
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetTextLen(pos);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetTextLen
}

// @pymethod int|PyCListBox|GetTopIndex|Returns the index of the top most visible item.
static PyObject *PyCListBox_get_top_index(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetTopIndex();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetTopIndex
                                    // @rdesc The zero based index of the top most visible item.
}

// @pymethod |PyCListBox|SelectString|Searches for a list-box item that matches the specified string, and selects it.
static PyObject *PyCListBox_select_string(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    TCHAR *string;
    PyObject *obstring;
    int after;
    if (!PyArg_ParseTuple(args, "iO",
                          &after,  // @pyparm int|after||Contains the zero-based index of the item before the first item
                                   // to be searched, or -1 for the entire listbox.
                          &obstring))  // @pyparm string|string||The string to search for.
        return NULL;
    if (!PyWinObject_AsTCHAR(obstring, &string, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SelectString(after, string);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(string);
    if (rc == LB_ERR)  // @pyseemfc CListBox|SelectString
        RETURN_ERR("The string does not exist");
    RETURN_NONE;
    // @rdesc The return value is always None - an exception is raised if the string can not be located.
}

// @pymethod |PyCListBox|SelItemRange|Selects an item range.
static PyObject *PyCListBox_sel_item_range(PyObject *self, PyObject *args)
{
    int bSel, start, end;
    if (!PyArg_ParseTuple(args, "iii",
                          &bSel,   // @pyparm int|bSel||Should the selection specified be set or cleared?
                          &start,  // @pyparm int|start||The zero based index of the first item to select.
                          &end))   // @pyparm int|end||The zero based index of the last item to select.
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SelItemRange(bSel, start, end);
    GUI_END_SAVE;
    if (rc == LB_ERR)
        RETURN_ERR("SelItemRange failed");
    RETURN_NONE;
}

// @pymethod |PyCListBox|SetCaretIndex|Sets the focus rectange to a specified item.
static PyObject *PyCListBox_set_caret_index(PyObject *self, PyObject *args)
{
    int index;
    BOOL bScroll = TRUE;
    if (!PyArg_ParseTuple(args, "i|i",
                          &index,     // @pyparm int|index||The zero based index of the item.
                          &bScroll))  // @pyparm int|bScroll|1|Should the listbox scroll to the item?
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SetCaretIndex(index, bScroll);
    GUI_END_SAVE;
    if (rc == LB_ERR)  // @pyseemfc CListBox|SetCaretIndex
        RETURN_ERR("SetCaretIndex failed");
    RETURN_NONE;
}
// @pymethod |PyCListBox|SetSel|Selects an item in a multiple selection listbox.
static PyObject *PyCListBox_set_sel(PyObject *self, PyObject *args)
{
    int index;
    BOOL bSel = TRUE;
    if (!PyArg_ParseTuple(args, "i|i",
                          &index,  // @pyparm int|index||The zero based index of the item to select.
                          &bSel))  // @pyparm int|bSel|1|Should the item be selected or deselected?
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SetSel(index, bSel);
    GUI_END_SAVE;
    if (rc == LB_ERR)  // @pyseemfc CListBox|SetSel
        RETURN_ERR("SetSel failed");
    RETURN_NONE;
}
// @pymethod |PyCListBox|SetCurSel|Selects an item in a single selection listbox.
static PyObject *PyCListBox_set_cur_sel(PyObject *self, PyObject *args)
{
    int index;
    if (!PyArg_ParseTuple(args, "i", &index))  // @pyparm int|index||The zero based index of the item to select.
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SetCurSel(index);
    GUI_END_SAVE;
    if (rc == LB_ERR && index != LB_ERR)  // @pyseemfc CListBox|SetCurSel
        RETURN_ERR("SetCurSel failed");
    RETURN_NONE;
}
// @pymethod int|PyCListBox|SetItemData|Sets the item's application-specific object value.
PyObject *PyCListBox_SetItemData(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
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
    BOOL ok = pLB->SetItemData(item, (DWORD_PTR)data);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemData failed");
    // @comm Note that a reference count is not added to the object.  This it is your
    // responsibility to make sure the object remains alive while in the list.
    RETURN_NONE;
}

// @pymethod int|PyCListBox|SetItemValue|Sets the item's application-specific value.
PyObject *PyCListBox_SetItemValue(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    int item;
    int data;
    if (!PyArg_ParseTuple(args, "ii:SetItemValue",
                          &item,   // @pyparm int|item||Index of the item whose Data is to be set.
                          &data))  // @pyparm int|data||New value for the data.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pLB->SetItemData(item, (DWORD)data);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemValue failed");
    RETURN_NONE;
}

// @pymethod |PyCListBox|SetTabStops|Sets the tab stops for a listbox.
static PyObject *PyCListBox_set_tab_stops(PyObject *self, PyObject *args)
{
    CListBox *pLB = GetListBox(self);
    int index;
    BOOL rc;
    if (!pLB)
        return NULL;
    // @pyparm int|eachTabStop||The position for each tab stop.
    if (PyArg_ParseTuple(args, "i", &index)) {
        GUI_BGN_SAVE;
        rc = pLB->SetTabStops(index);
        GUI_END_SAVE;
    }
    else {
        PyObject *listOb;
        PyErr_Clear();
        // @pyparmalt1 list of integers|tabStops||Each individual tab stop.
        if (!PyArg_ParseTuple(args, "O", &listOb))
            return NULL;
        if (!PyList_Check(listOb))
            RETURN_TYPE_ERR("Param must be a list object");
        Py_ssize_t numChildren = PyList_Size(listOb);
        int *pArray = new int[numChildren];
        int tabVal;
        for (Py_ssize_t child = 0; child < numChildren; child++) {
            PyObject *obChild = PyList_GetItem(listOb, child);
            if (!PyArg_Parse(obChild, "i", &tabVal)) {
                delete pArray;
                RETURN_TYPE_ERR("List param must be a list of integers");
            }
            pArray[child] = tabVal;
        }
        GUI_BGN_SAVE;
        rc = pLB->SetTabStops(PyWin_SAFE_DOWNCAST(numChildren, Py_ssize_t, int), pArray);
        GUI_END_SAVE;
        delete pArray;
    }

    if (!rc)
        RETURN_ERR("SetTabStops failed");
    RETURN_NONE;
}

// @pymethod |PyCListBox|SetTopIndex|Sets the top index (top most visible item) of the listbox.
static PyObject *PyCListBox_set_top_index(PyObject *self, PyObject *args)
{
    int index;
    if (!PyArg_ParseTuple(
            args, "i", &index))  // @pyparm int|index||The zero based index of the item to place at the top of the list.
        return NULL;
    CListBox *pLB = GetListBox(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->SetTopIndex(index);
    GUI_END_SAVE;
    if (rc == LB_ERR)  // @pyseemfc CListBox|SetTopIndex
        RETURN_ERR("SetTopIndex failed");
    RETURN_NONE;
}

// @object PyCListBox|A windows listbox control.  Encapsulates an MFC <c CListBox> class.  Derived from a <o PyCControl>
// object.
static struct PyMethodDef PyCListBox_methods[] = {
    {"AddString", PyCListBox_add_string, 1},        // @pymeth AddString|Add a string to the listbox.
    {"DeleteString", PyCListBox_delete_string, 1},  // @pymeth DeleteString|Delete a string from the listbox.
    {"Dir", PyCListBox_dir, 1},                     // @pymeth Dir|Fill a listbox with a file specification.
    {"GetCaretIndex", PyCListBox_get_caret_index,
     1},                                    // @pymeth GetCaretIndex|Get the index of the item with the focus rectangle.
    {"GetCount", PyCListBox_get_count, 1},  // @pymeth GetCount|Get the count of items in the listbox.
    {"GetCurSel", PyCListBox_get_cur_sel,
     1},  // @pymeth GetCurSel|Get the current selection in a single selection listbox.
    {"GetItemData", PyCListBox_GetItemData,
     1},  // @pymeth GetItemData|Retrieves the application-specific object associated with a listbox entry
    {"GetItemValue", PyCListBox_GetItemValue,
     1},  // @pymeth GetItemValue|Retrieves the application-specific value associated with a listbox entry
    {"GetSel", PyCListBox_get_sel, 1},  // @pymeth GetSel|Get the selected items in a multiple selection listbox.
    {"GetSelCount", PyCListBox_get_sel_count,
     1},  // @pymeth GetSelCount|Get the number of selected items in a multtiple selection listbox.
    {"GetSelItems", PyCListBox_get_sel_items,
     1},  // @pymeth GetSelItems|Get the index of the selected items in a multiple selection listbox.
    {"GetSelTextItems", PyCListBox_get_sel_text_items,
     1},  // @pymeth GetSelTextItems|Get the text of the selected items in a multiple selection listbox.
    {"GetTopIndex", PyCListBox_get_top_index, 1},   // @pymeth GetTopIndex|Get the index of the topmost item.
    {"GetText", PyCListBox_get_text, 1},            // @pymeth GetText|Get the text associated with an item.
    {"GetTextLen", PyCListBox_get_text_len, 1},     // @pymeth GetTextLen|Get the length of an item
    {"InsertString", PyCListBox_insert_string, 1},  // @pymeth InsertString|Insert a string into the listbox.
    {"ResetContent", PyCListBox_reset_content, 1},  // @pymeth ResetContent|Remove all items from a listbox.
    {"SetCaretIndex", PyCListBox_set_caret_index,
     1},                                            // @pymeth SetCaretIndex|Set the focus rectange to a specified item.
    {"SelectString", PyCListBox_select_string, 1},  // @pymeth SelectString|Select an item, based on a string.
    {"SelItemRange", PyCListBox_sel_item_range,
     1},  // @pymeth SelItemRange|Select a range of items in a multiple selection listbox.
    {"SetCurSel", PyCListBox_set_cur_sel,
     1},  // @pymeth SetCurSel|Set the current selection in a single selection listbox.
    {"SetItemData", PyCListBox_SetItemData,
     1},  // @pymeth SetItemData|Sets the application-specific object associated with a listbox entry
    {"SetItemValue", PyCListBox_SetItemValue,
     1},  // @pymeth SetItemValue|Sets the application-specific value associated with a listbox entry
    {"SetSel", PyCListBox_set_sel, 1},             // @pymeth SetSel|Set the selection.
    {"SetTabStops", PyCListBox_set_tab_stops, 1},  // @pymeth SetTabStops|Set the tab stops for a listbox.
    {"SetTopIndex", PyCListBox_set_top_index, 1},  // @pymeth SetTopIndex|Set the top most visible item in a listbox.
    {NULL, NULL}};

ui_type_CObject PyCListBox::type("PyCListBox", &ui_control_object::type, RUNTIME_CLASS(CListBox), sizeof(PyCListBox),
                                 PYOBJ_OFFSET(PyCListBox), PyCListBox_methods, GET_PY_CTOR(PyCListBox));

/////////////////////////////////////////////////////////////////////
//
// PyCComboBox
//
static CComboBox *GetCombo(PyObject *self)
{
    // note we can only ask for a CWnd, if the LB is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CComboBox *)PyCWnd::GetPythonGenericWnd(self);
}
PyCComboBox::PyCComboBox() {}
PyCComboBox::~PyCComboBox() {}
// @pymethod int|PyCComboBox|AddString|Adds a string to a combobox.
static PyObject *PyCComboBox_add_string(PyObject *self, PyObject *args)
{
    CComboBox *pCB = GetCombo(self);
    if (!pCB)
        return NULL;
    PyObject *ob;
    if (!PyArg_ParseTuple(args, "O", &ob))  // @pyparm any|object||Any object.  If not a string, __str__, __repr__ or a
                                            // default repr() will be used
        return NULL;
    CString cstrRepr = GetReprText(ob);
    GUI_BGN_SAVE;
    int rc = pCB->AddString(cstrRepr);
    GUI_END_SAVE;
    //@pyseemfc CComboBox|AddString
    if (IS_CB_ERR(rc))
        RETURN_ERR("PyCComboBox.AddString failed");
    return Py_BuildValue("i", rc);
    //@rdesc The zero based index of the new string.
}
// @pymethod int|PyCComboBox|DeleteString|Deletes an item from a combobox.
static PyObject *PyCComboBox_delete_string(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i", &pos))  // @pyparm int|pos||The zero based index of the item to delete.
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->DeleteString(pos);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|DeleteString
                                    // @rdesc The count of the items remaining in the list.
}
// @pymethod int|PyCComboBox|Dir|Fills the list portion of a combobox with a directory listing.
static PyObject *PyCComboBox_dir(PyObject *self, PyObject *args)
{
    int attr;
    TCHAR *szWild;
    PyObject *obWild;
    if (!PyArg_ParseTuple(args, "iO",
                          &attr,     // @pyparm int|attr||The attributes of the files to locate
                          &obWild))  // @pyparm string|wild||A file specification string - eg, *.*
        return NULL;
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    if (!PyWinObject_AsTCHAR(obWild, &szWild, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->Dir(attr, szWild);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szWild);
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|Dir
                                    // @rdesc The index of the last file name added to the list.
}

// @pymethod int|PyCComboBox|InsertString|Insert a string into a combobox.
static PyObject *PyCComboBox_insert_string(PyObject *self, PyObject *args)
{
    int pos;
    PyObject *ob;
    if (!PyArg_ParseTuple(args, "iO",
                          &pos,  // @pyparm int|pos||The zero based index in the combobox to insert the new string
                          &ob))  // @pyparm any|object||The object to be added to the combobox
        return NULL;
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    CString cstrRepr = GetReprText(ob);
    GUI_BGN_SAVE;
    int rc = pLB->InsertString(pos, cstrRepr);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|InsertString
                                    // @rdesc The zero based index of the new string added.
}
// @pymethod |PyCComboBox|ResetContent|Clear all the items from a combobox.
static PyObject *PyCComboBox_reset_content(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    pLB->ResetContent();  // @pyseemfc CComboBox|ResetContent
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCComboBox|GetCount|Returns the count of items in the combobox.
static PyObject *PyCComboBox_get_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CComboBox *pCB = GetCombo(self);
    if (!pCB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pCB->GetCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CListBox|GetCount
                                    // @rdesc Returns the number of items currently in the combobox.
}
// @pymethod int|PyCComboBox|GetCurSel|Returns the index of the currently selected item.
static PyObject *PyCComboBox_get_cur_sel(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetCurSel();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|GetCurSel
                                    //@comm Should not be called for a multiple selection listbox.
}
// @pymethod int|PyCComboBox|GetEditSel|Returns the selection of the edit control portion of a combo box.
static PyObject *PyCComboBox_get_edit_sel(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetEditSel();  // @pyseemfc CComboBox|GetEditSel
    GUI_END_SAVE;
    if (IS_CB_ERR(rc))
        RETURN_ERR("GetEditSel failed");

    return Py_BuildValue("i", rc);
    // @rdesc A 32-bit value that contains the starting position in the low-order word and
    // the position of the first nonselected character after the end of
    // the selection in the high-order word. If this function is used on a combo box
    // without an edit control, an exception is raised.
}
// @pymethod int|PyCComboBox|GetExtendedUI|Indicates if the combo has the extended interface.
static PyObject *PyCComboBox_get_extended_ui(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetExtendedUI();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|GetExtendedUI
                                    // @rdesc Nonzero if the combo box has the extended user interface; otherwise 0.
}

// @pymethod object|PyCComboBox|GetItemData|Retrieves the application-specific object associated with an item.
PyObject *PyCComboBox_GetItemData(PyObject *self, PyObject *args)
{
    int item;
    if (!PyArg_ParseTuple(args, "i:GetItemData",
                          &item))  // @pyparm int|item||The index of the item whose data is to be retrieved.

        return NULL;
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    DWORD_PTR rc = pLB->GetItemData(item);
    GUI_END_SAVE;
    return PyWinObject_FromDWORD_PTR(rc);
}

// @pymethod int|PyCComboBox|GetItemValue|Retrieves the application-specific value associated with an item.
PyObject *PyCComboBox_GetItemValue(PyObject *self, PyObject *args)
{
    int item;
    if (!PyArg_ParseTuple(args, "i:GetItemValue",
                          &item))  // @pyparm int|item||The index of the item whose data is to be retrieved.

        return NULL;
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    long rc = (long)pLB->GetItemData(item);
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod string|PyCComboBox|GetLBText|Gets the string from the list of a combo box.
static PyObject *PyCComboBox_get_lb_text(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int pos;  // @pyparm int|index||The index of the item to return the string for.
    if (!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
    CString cs;
    // Prevent MFC ASSERTing when empty - dont use the CString version.
    GUI_BGN_SAVE;
    int size = pLB->GetLBTextLen(pos);
    if (size != LB_ERR) {
        pLB->GetLBText(pos, cs.GetBufferSetLength(size));
        cs.ReleaseBuffer();
    }
    GUI_END_SAVE;
    if (IS_CB_ERR(size))
        RETURN_ERR("GetLBText failed - invalid index");
    return PyWinObject_FromTCHAR(cs);
    // @rdesc The requested string. If index does
    // not specify a valid index, no exception is raised.
}
// @pymethod int|PyCComboBox|GetLBTextLen|Returns the length of a string in the list of a combobox.
static PyObject *PyCComboBox_get_lb_text_len(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int pos;  // @pyparm int|index||The index of the item to return the length of.
    if (!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pLB->GetLBTextLen(pos);
    GUI_END_SAVE;
    if (IS_CB_ERR(rc))
        RETURN_ERR("PyCComboBox.GetLBTextLen failed");
    return Py_BuildValue("i", rc);  // @pyseemfc CComboBox|GetLBTextLen
    // @ rdesc Returns the length of the string (in bytes), or raises an exception on error.
}
// @pymethod int|PyCComboBox|LimitText|Limits the amount of text the edit portion of a combo box can hold.
static PyObject *PyCComboBox_limit_text(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int pos;
    if (!PyArg_ParseTuple(args, "i", &pos))  // @pyparm int|max||The maximum number of characters the user can enter. If
                                             // zero, the size is set to (virtually) unlimited.
        return NULL;
    GUI_BGN_SAVE;
    long rc = pLB->LimitText(pos);
    GUI_END_SAVE;
    if (rc == CB_ERR)  // @pyseemfc CComboBox|LimitText
        RETURN_ERR("Combo does not have an edit box");
    RETURN_NONE;
}

// @pymethod |PyCComboBox|SelectString|Searches for a combobox item that matches the specified string, and selects it.
static PyObject *PyCComboBox_select_string(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    TCHAR *string;
    PyObject *obstring;
    int after;
    if (!PyArg_ParseTuple(args, "iO",
                          &after,  // @pyparm int|after||Contains the zero-based index of the item before the first item
                                   // to be searched, or -1 for the entire combobox.
                          &obstring))  // @pyparm string|string||The string to search for.
        return NULL;
    if (!PyWinObject_AsTCHAR(obstring, &string, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    long rc = pLB->SelectString(after, string);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(string);
    if (rc == CB_ERR)  // @pyseemfc CComboBoxBox|SelectString
        RETURN_ERR("The string does not exist");
    RETURN_NONE;
    // @rdesc The return value is always None - an exception is raised if the string can not be located.
}

// @pymethod |PyCComboBox|SetCurSel|Selects an item in a combobox.
static PyObject *PyCComboBox_set_cur_sel(PyObject *self, PyObject *args)
{
    int index;
    if (!PyArg_ParseTuple(args, "i", &index))  // @pyparm int|index||The zero based index of the item to select.
        return NULL;
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    GUI_BGN_SAVE;
    long rc = pLB->SetCurSel(index);
    GUI_END_SAVE;
    if (rc == CB_ERR && index != CB_ERR)  // @pyseemfc CComboBox|SetCurSel
        RETURN_ERR("SetCurSel failed");
    RETURN_NONE;
}
// @pymethod |PyCComboBox|SetEditSel|Sets the selection in the edit control portion of a combo box.
static PyObject *PyCComboBox_set_edit_sel(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int start, end;
    if (!PyArg_ParseTuple(
            args, "ii",
            &start,  // @pyparm int|start||Specifies the starting position. If the starting position is set to -1, then
                     // any existing selection is removed.
            &end))   // @pyparm int|end||Specifies the ending position. If the ending position is set to -1, then all
                     // text from the starting position to the last character in the edit control is selected.
        return NULL;
    GUI_BGN_SAVE;
    long rc = pLB->SetEditSel(start, end);
    GUI_END_SAVE;
    if (rc == CB_ERR)  // @pyseemfc PyCComboBox|SetEditSel
        RETURN_ERR("Combo is dropdown, or does not have an edit box");
    RETURN_NONE;
    // @rdesc The return value is always None - an exception is raised if the combo is a dropdown style, or does not
    // have an edit control.
}

// @pymethod |PyCComboBox|SetExtendedUI|Selects the Extended UI mode for a combo box.
static PyObject *PyCComboBox_set_extended_ui(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int flag = TRUE;  // @pyparm int|bExtended|1|Indicates if the combo should have the extended user interface.
    if (!PyArg_ParseTuple(args, "|i", &flag))
        return NULL;
    GUI_BGN_SAVE;
    long rc = pLB->SetExtendedUI(flag);
    GUI_END_SAVE;
    if (rc == CB_ERR)  // @pyseemfc CListBox|SetExtendedUI
        RETURN_ERR("SetExtendedUI failed");
    // @comm A combo box with the Extended UI flag set can be identified in the following ways:~
    // * Clicking the static control displays the list box only for combo boxes with the CBS_DROPDOWNLIST style.~
    // * Pressing the DOWN ARROW key displays the list box (F4 is disabled).~
    // * Scrolling in the static control is disabled when the item list is not visible (the arrow keys are disabled).
    RETURN_NONE;
}
// @pymethod int|PyCComboBox|SetItemData|Sets the item's application-specific object value.
PyObject *PyCComboBox_SetItemData(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
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
    BOOL ok = pLB->SetItemData(item, (DWORD_PTR)data);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemData failed");
    // @comm Note that a reference count is not added to the object.  This it is your
    // responsibility to make sure the object remains alive while in the list.
    RETURN_NONE;
}

// @pymethod int|PyCComboBox|SetItemValue|Sets the item's application-specific value.
PyObject *PyCComboBox_SetItemValue(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int item;
    int data;
    if (!PyArg_ParseTuple(args, "ii:SetItemValue",
                          &item,   // @pyparm int|item||Index of the item whose Data is to be set.
                          &data))  // @pyparm int|data||New value for the data.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pLB->SetItemData(item, (DWORD)data);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemValue failed");
    RETURN_NONE;
}
static PyObject *
// @pymethod |PyCComboBox|ShowDropDown|Shows or hides the listbox portion of a combo box.
PyCComboBox_show_drop_down(PyObject *self, PyObject *args)
{
    CComboBox *pLB = GetCombo(self);
    if (!pLB)
        return NULL;
    int flag = TRUE;  // @pyparm int|bShowIt|1|Indicates if the listbox should be shown or hidden.
    if (!PyArg_ParseTuple(args, "|i", &flag))
        return NULL;
    GUI_BGN_SAVE;
    pLB->ShowDropDown(flag);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @object PyCComboBox|A windows combo control.  Encapsulates an MFC <c CComboBox> class.  Derived from a <o PyCControl>
// object.
static struct PyMethodDef PyCComboBox_methods[] = {
    {"AddString", PyCComboBox_add_string, 1},  // @pymeth AddString|Add a string to the listbox portion of a combo box.
    {"DeleteString", PyCComboBox_delete_string,
     1},                          // @pymeth DeleteString|Delete a string to the listbox portion of a combo box.
    {"Dir", PyCComboBox_dir, 1},  // @pymeth Dir|Fill the listbox portion of a combo with a file specification.
    {"GetCount", PyCComboBox_get_count,
     1},  // @pymeth GetCount|Get the count of items in the listbox portion of a combo box.
    {"GetCurSel", PyCComboBox_get_cur_sel,
     1},  // @pymeth GetCurSel|Get the current selection in the listbox portion of a combo box.
    {"GetEditSel", PyCComboBox_get_edit_sel,
     1},  // @pymeth GetEditSel|Gets the edit control selection from a combo box.
    {"GetExtendedUI", PyCComboBox_get_extended_ui,
     1},  // @pymeth GetExtendedUI|Gets the ExtendedUI flag for a combo box.
    {"GetItemData", PyCComboBox_GetItemData,
     1},  // @pymeth GetItemData|Retrieves the application-specific object associated with a combobox entry
    {"GetItemValue", PyCComboBox_GetItemValue,
     1},  // @pymeth GetItemValue|Retrieves the application-specific value associated with a combobox entry
    {"GetLBText", PyCComboBox_get_lb_text, 1},  // @pymeth GetLBText|Gets the text from the edit control in a combo box.
    {"GetLBTextLen", PyCComboBox_get_lb_text_len,
     1},  // @pymeth GetLBTextLen|Gets the length of the text in the edit control of a combo box.
    {"InsertString", PyCComboBox_insert_string,
     1},  // @pymeth InsertString|Inserts a string into the listbox portion of a combo box.
    {"LimitText", PyCComboBox_limit_text,
     1},  // @pymeth LimitText|Limit the length of text in the edit control portion of a combo box.
    {"ResetContent", PyCComboBox_reset_content,
     1},  // @pymeth ResetContent|Remove all items from the listbox portion of a combo box.
    {"SelectString", PyCComboBox_select_string,
     1},  // @pymeth SelectString|Select a string in the listbox portion of a combo box.
    {"SetCurSel", PyCComboBox_set_cur_sel,
     1},  // @pymeth SetCurSel|Sets the current selection in the listbox portion of a combo box.
    {"SetEditSel", PyCComboBox_set_edit_sel,
     1},  // @pymeth SetEditSel|Sets the current selection in the edit control portion of a combo box.
    {"SetExtendedUI", PyCComboBox_set_extended_ui,
     1},  // @pymeth SetExtendedUI|Sets the ExtendedUI flag for a combo box.
    {"SetItemData", PyCComboBox_SetItemData,
     1},  // @pymeth SetItemData|Sets the application-specific object associated with a combobox entry
    {"SetItemValue", PyCComboBox_SetItemValue,
     1},  // @pymeth SetItemValue|Sets the application-specific value associated with a combobox entry
    {"ShowDropDown", PyCComboBox_show_drop_down, 1},  // @pymeth ShowDropDown|Shows the listbox portion of a combo box.
    {NULL, NULL}};

ui_type_CObject PyCComboBox::type("PyCComboBox", &ui_control_object::type, RUNTIME_CLASS(CComboBox),
                                  sizeof(PyCComboBox), PYOBJ_OFFSET(PyCComboBox), PyCComboBox_methods,
                                  GET_PY_CTOR(PyCComboBox));

/////////////////////////////////////////////////////////////////////
//
// PyCProgressCtrl
//
static CProgressCtrl *GetProgressCtrl(PyObject *self)
{
    // note we can only ask for a CWnd, if the PC is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CProgressCtrl *)PyCWnd::GetPythonGenericWnd(self);
}
PyCProgressCtrl::PyCProgressCtrl() {}
PyCProgressCtrl::~PyCProgressCtrl() {}

// @pymethod <o PyCProgressCtrl>|win32ui|CreateProgressCtrl|Creates a progress control object. <om
// PyProgressCtrl.Create> creates the actual control.
PyObject *PyCProgressCtrl_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CProgressCtrl *pPC = new CProgressCtrl();
    return ui_assoc_object::make(PyCProgressCtrl::type, pPC);
}

// @pymethod |PyCProgressCtrl|CreateWindow|Creates the actual control.
PyObject *PyCProgressCtrl_create_window(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    if (!PyArg_ParseTuple(
            args, "i(iiii)Oi:CreateWindow",
            &style,  // @pyparm int|style||The style for the control.
            &rect.left, &rect.top, &rect.right, &rect.bottom,
            // @pyparm (left, top, right, bottom)|rect||The size and position of the control.
            &obParent,  // @pyparm <o PyCWnd>|parent||The parent window of the control.  Usually a <o PyCDialog>.
            &id))       // @pyparm int|id||The control's ID.
        return NULL;

    if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
        RETURN_TYPE_ERR("parent argument must be a window object");
    CWnd *pParent = GetWndPtr(obParent);
    if (pParent == NULL)
        return NULL;
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pPC->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CProgressCtrl::Create");
    RETURN_NONE;
}
// @pymethod |PyCProgressCtrl|SetRange|Set the control's bounds
static PyObject *PyCProgressCtrl_set_range(PyObject *self, PyObject *args)
{
    int nLower, nUpper;
    if (!PyArg_ParseTuple(args, "ii",
                          &nLower,   // @pyparm int|nLower|1|Specifies the lower limit of the range (default is zero).
                          &nUpper))  // @pyparm int|nUpper|1|Specifies the upper limit of the range (default is 100).
        return NULL;
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;
    GUI_BGN_SAVE;
    pPC->SetRange(nLower, nUpper);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCProgressCtrl|SetPos|Set the control's position
static PyObject *PyCProgressCtrl_set_pos(PyObject *self, PyObject *args)
{
    int nPos;
    if (!PyArg_ParseTuple(args, "i",
                          &nPos))  // @pyparm int|nPos|1|New position of the progress bar control.
        return NULL;
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPC->SetPos(nPos);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCProgressCtrl|OffsetPos|Advances the progress bar control's current position by the increment
// specified
static PyObject *PyCProgressCtrl_offset_pos(PyObject *self, PyObject *args)
{
    int nPos;
    if (!PyArg_ParseTuple(args, "i",
                          &nPos))  // @pyparm int|nPos|1|Amount to advance the position.
        return NULL;
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPC->OffsetPos(nPos);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCProgressCtrl|SetStep|Specifies the step increment for a progress bar control.
static PyObject *PyCProgressCtrl_set_step(PyObject *self, PyObject *args)
{
    int nStep;
    if (!PyArg_ParseTuple(args, "i",
                          &nStep))  // @pyparm int|nStep|1|New step increment.
        return NULL;
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPC->SetStep(nStep);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCProgressCtrl|StepIt|Advances the current position for a progress bar control by the step increment.
// Returns previous position.
static PyObject *PyCProgressCtrl_step(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CProgressCtrl *pPC = GetProgressCtrl(self);
    if (!pPC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPC->StepIt();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @object PyCProgressCtrl|A windows progress bar control.  Encapsulates an MFC <c CProgressCtrl> class.  Derived from
// <o PyCControl>.
static struct PyMethodDef PyCProgressCtrl_methods[] = {
    {"CreateWindow", PyCProgressCtrl_create_window,
     1},  // @pymeth CreateWindow|Creates the window for a new progress bar object.
    {"SetRange", PyCProgressCtrl_set_range,
     1},                                     // @pymeth SetRange|Sets the lower and upper bounds for the progress bar.
    {"SetPos", PyCProgressCtrl_set_pos, 1},  // @pymeth SetPos|Set the control's position
    {"OffsetPos", PyCProgressCtrl_offset_pos,
     1},  // @pymeth OffsetPos|Advances the progress bar control's current position by the increment specified.
    {"SetStep", PyCProgressCtrl_set_step,
     1},                                  // @pymeth SetStep|Specifies the step increment for a progress bar control.
    {"StepIt", PyCProgressCtrl_step, 1},  // @pymeth StepIt|Advances the current position for a progress bar control by
                                          // the step increment. Returns previous position.
    {NULL, NULL}};

ui_type_CObject PyCProgressCtrl::type("PyCProgressCtrl", &ui_control_object::type, RUNTIME_CLASS(CProgressCtrl),
                                      sizeof(PyCProgressCtrl), PYOBJ_OFFSET(PyCProgressCtrl), PyCProgressCtrl_methods,
                                      GET_PY_CTOR(PyCProgressCtrl));

/////////////////////////////////////////////////////////////////////
//
// PyCSliderCtrl
//
static CSliderCtrl *GetSliderCtrl(PyObject *self)
{
    // note we can only ask for a CWnd, if the SC is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CSliderCtrl *)PyCWnd::GetPythonGenericWnd(self);
}
PyCSliderCtrl::PyCSliderCtrl() {}
PyCSliderCtrl::~PyCSliderCtrl() {}

// @pymethod <o PyCSliderCtrl>|win32ui|CreateSliderCtrl|Creates a Slider control object.
PyObject *PyCSliderCtrl_create(PyObject *self, PyObject *args)
{
    // @comm  The method <om PySliderCtrl.CreateWindow> is used to
    // create the actual control.
    CHECK_NO_ARGS(args);
    CSliderCtrl *pPC = new CSliderCtrl();
    return ui_assoc_object::make(PyCSliderCtrl::type, pPC);
}

// @pymethod |PyCSliderCtrl|CreateWindow|Creates the actual control.
PyObject *PyCSliderCtrl_create_window(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    if (!PyArg_ParseTuple(
            args, "i(iiii)Oi:CreateWindow",
            &style,  // @pyparm int|style||The style for the control.
            &rect.left, &rect.top, &rect.right, &rect.bottom,
            // @pyparm (left, top, right, bottom)|rect||The size and position of the control.
            &obParent,  // @pyparm <o PyCWnd>|parent||The parent window of the control.  Usually a <o PyCDialog>.
            &id))       // @pyparm int|id||The control's ID.
        return NULL;

    if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
        RETURN_TYPE_ERR("parent argument must be a window object");
    CWnd *pParent = GetWndPtr(obParent);
    if (pParent == NULL)
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pSC->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CSliderCtrl::Create");
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|GetLineSize|Get the control's position
static PyObject *PyCSliderCtrl_get_line_size(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetLineSize();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|SetLineSize|Set the control's line size.  Returns the previous line size.
static PyObject *PyCSliderCtrl_set_line_size(PyObject *self, PyObject *args)
{
    int nLineSize;
    if (!PyArg_ParseTuple(args, "i",
                          &nLineSize))  // @pyparm int|nLineSize|1|New line size of the Slider bar control
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->SetLineSize(nLineSize);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetPageSize|Get the control's position
static PyObject *PyCSliderCtrl_get_page_size(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetPageSize();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|SetPageSize|Set the control's page size  Returns the previous page size.
static PyObject *PyCSliderCtrl_set_page_size(PyObject *self, PyObject *args)
{
    int nPageSize;
    if (!PyArg_ParseTuple(args, "i",
                          &nPageSize))  // @pyparm int|nPageSize|1|New page size of the Slider bar control.
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->SetPageSize(nPageSize);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetRangeMax|Get the control's Maximum
static PyObject *PyCSliderCtrl_get_range_max(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetRangeMax();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetRangeMin|Get the control's Minimum
static PyObject *PyCSliderCtrl_get_range_min(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetRangeMin();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetRange|Get the control's min and max
static PyObject *PyCSliderCtrl_get_range(PyObject *self, PyObject *args)
{
    int nMin, nMax;
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->GetRange(nMin, nMax);
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", nMin, nMax);
}

// @pymethod int|PyCSliderCtrl|SetRangeMin|Set the control's minimum
static PyObject *PyCSliderCtrl_set_range_min(PyObject *self, PyObject *args)
{
    int nRangeMin;
    BOOL bRedraw = FALSE;
    if (!PyArg_ParseTuple(args, "i|i",
                          &nRangeMin,  // @pyparm int|nRangeMin|1|New minimum of the Slider bar control.
                          &bRedraw))   // @pyparm int|bRedraw|1|Should slider be redrawn?
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetRangeMin(nRangeMin, bRedraw);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|SetRangeMax|Set the control's maximum
static PyObject *PyCSliderCtrl_set_range_max(PyObject *self, PyObject *args)
{
    int nRangeMax;
    BOOL bRedraw = FALSE;
    if (!PyArg_ParseTuple(args, "i|i",
                          &nRangeMax,  // @pyparm int|nRangeMax|1|New maximum of the Slider bar control.
                          &bRedraw))   // @pyparm int|bRedraw|1|Should slider be redrawn?
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetRangeMax(nRangeMax, bRedraw);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|SetRange|Set the control's min and max
static PyObject *PyCSliderCtrl_set_range(PyObject *self, PyObject *args)
{
    int nRangeMax;
    int nRangeMin;
    BOOL bRedraw = FALSE;
    if (!PyArg_ParseTuple(args, "ii|i",
                          &nRangeMin,  // @pyparm int|nRangeMin|1|New minimum of the Slider bar control.
                          &nRangeMax,  // @pyparm int|nRangeMax|1|New maximum of the Slider bar control.
                          &bRedraw))   // @pyparm int|bRedraw|1|Should slider be redrawn?
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetRange(nRangeMin, nRangeMax, bRedraw);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|GetSelection|Get the control's selection start and end positions
static PyObject *PyCSliderCtrl_get_selection(PyObject *self, PyObject *args)
{
    int nMin, nMax;
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->GetSelection(nMin, nMax);
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", nMin, nMax);
}

// @pymethod int|PyCSliderCtrl|SetSelection|Set the control's selection start and end positions
static PyObject *PyCSliderCtrl_set_selection(PyObject *self, PyObject *args)
{
    int nRangeMax;
    int nRangeMin;
    if (!PyArg_ParseTuple(args, "ii",
                          &nRangeMin,   // @pyparm int|nRangeMin|1|New start of the Slider's selection.
                          &nRangeMax))  // @pyparm int|nRangeMax|1|New end of the Slider's selection.
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetSelection(nRangeMin, nRangeMax);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|GetChannelRect|Get the control's channel rectangle
static PyObject *PyCSliderCtrl_get_channel_rect(PyObject *self, PyObject *args)
{
    RECT rect;
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->GetChannelRect(&rect);
    GUI_END_SAVE;
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod int|PyCSliderCtrl|GetThumbRect|Get the control's thumb rectangle
static PyObject *PyCSliderCtrl_get_thumb_rect(PyObject *self, PyObject *args)
{
    RECT rect;
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->GetThumbRect(&rect);
    GUI_END_SAVE;
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod int|PyCSliderCtrl|GetPos|Get the control's position
static PyObject *PyCSliderCtrl_get_pos(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetPos();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|SetPos|Set the control's position
static PyObject *PyCSliderCtrl_set_pos(PyObject *self, PyObject *args)
{
    int nPos;
    if (!PyArg_ParseTuple(args, "i",
                          &nPos))  // @pyparm int|nPos|1|New position of the Slider bar control.
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetPos(nPos);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|GetNumTics|Get number of tics in the slider
static PyObject *PyCSliderCtrl_get_num_tics(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    unsigned int rc = pSC->GetNumTics();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetTicArray|Get a tuple of slider tic positions
static PyObject *PyCSliderCtrl_get_tic_array(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int n = pSC->GetNumTics();
    DWORD *array = pSC->GetTicArray();
    GUI_END_SAVE;
    PyObject *rc = PyTuple_New(n);
    for (int i = 0; i < n; i++) {
        PyTuple_SetItem(rc, i, Py_BuildValue("i", array[i]));
    }
    return rc;
}

// @pymethod int|PyCSliderCtrl|GetTic|Get the position of the specified tic number
static PyObject *PyCSliderCtrl_get_tic(PyObject *self, PyObject *args)
{
    int nTic;
    if (!PyArg_ParseTuple(args, "i",
                          &nTic))  // @pyparm int|nTic|1|Zero based index of the tic mark
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetTic(nTic);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|GetTicPos|Get the position of the specified tic number in client coordinates
static PyObject *PyCSliderCtrl_get_tic_pos(PyObject *self, PyObject *args)
{
    int nTic;
    if (!PyArg_ParseTuple(args, "i",
                          &nTic))  // @pyparm int|nTic|1|Zero based index of the tic mark
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->GetTicPos(nTic);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|SetTic|Set a tic at the specified position
static PyObject *PyCSliderCtrl_set_tic(PyObject *self, PyObject *args)
{
    int nTic;
    if (!PyArg_ParseTuple(args, "i",
                          &nTic))  // @pyparm int|nTic|1|Position of the desired tic mark
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pSC->SetTic(nTic);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCSliderCtrl|SetTicFreq|Set the tic frequency
static PyObject *PyCSliderCtrl_set_tic_freq(PyObject *self, PyObject *args)
{
    int nFreq;
    if (!PyArg_ParseTuple(args, "i",
                          &nFreq))  // @pyparm int|nFreq|1|Frequency of tic marks
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->SetTicFreq(nFreq);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|ClearSel|Clear the selection
static PyObject *PyCSliderCtrl_clear_sel(PyObject *self, PyObject *args)
{
    BOOL bRedraw;
    if (!PyArg_ParseTuple(args, "i",
                          &bRedraw))  // @pyparm int|bRedraw|1|Redraw the control?
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->ClearSel(bRedraw);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|VerifyPos|Verify the position is between configured min and max
static PyObject *PyCSliderCtrl_verify_pos(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
#if _MFC_VER >= 0x0710
    // This just vanished in VS7
    PyErr_SetString(PyExc_NotImplementedError, "VerifyPos does not appear in this version of MFC");
    return NULL;
#else
    GUI_BGN_SAVE;
    pSC->VerifyPos();
    GUI_END_SAVE;
#endif
    RETURN_NONE;
}

// @pymethod int|PyCSliderCtrl|ClearTics|Clear the control's tic marks
static PyObject *PyCSliderCtrl_clear_tics(PyObject *self, PyObject *args)
{
    BOOL bRedraw;
    if (!PyArg_ParseTuple(args, "i",
                          &bRedraw))  // @pyparm int|bRedraw|1|Redraw the control?
        return NULL;
    CSliderCtrl *pSC = GetSliderCtrl(self);
    if (!pSC)
        return NULL;
    GUI_BGN_SAVE;
    pSC->ClearTics(bRedraw);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCSliderCtrl|A windows Slider bar control.  Encapsulates an MFC <c CSliderCtrl> class.  Derived from <o
// PyCControl>.
static struct PyMethodDef PyCSliderCtrl_methods[] = {
    {"CreateWindow", PyCSliderCtrl_create_window,
     1},  // @pymeth CreateWindow|Creates the window for a new Slider bar object.
    {"GetLineSize", PyCSliderCtrl_get_line_size, 1},   // @pymeth GetLineSize|Get the control's line size
    {"SetLineSize", PyCSliderCtrl_set_line_size, 1},   // @pymeth SetLineSize|Set the control's line size
    {"GetPageSize", PyCSliderCtrl_get_page_size, 1},   // @pymeth GetPageSize|Get the control's Page size
    {"SetPageSize", PyCSliderCtrl_set_page_size, 1},   // @pymeth SetPageSize|Set the control's Page size
    {"GetRangeMax", PyCSliderCtrl_get_range_max, 1},   // @pymeth GetRangeMax|Get the control's maximum
    {"GetRangeMin", PyCSliderCtrl_get_range_min, 1},   // @pymeth GetRangeMin|Get the control's minimum
    {"GetRange", PyCSliderCtrl_get_range, 1},          // @pymeth GetRange|Get the control's minimum and maximum
    {"SetRangeMax", PyCSliderCtrl_set_range_max, 1},   // @pymeth GetRangeMax|Set the control's maximum
    {"SetRangeMin", PyCSliderCtrl_set_range_min, 1},   // @pymeth GetRangeMin|Set the control's minimum
    {"SetRange", PyCSliderCtrl_set_range, 1},          // @pymeth SetRange|Set the control's minimum and maximum
    {"GetSelection", PyCSliderCtrl_get_selection, 1},  // @pymeth GetSelection|Get the selection start and end positions
    {"SetSelection", PyCSliderCtrl_set_selection, 1},  // @pymeth SetSelection|Set the selection start and end positions
    {"GetChannelRect", PyCSliderCtrl_get_channel_rect, 1},  // @pymeth GetChannelRect|Get the control's channel rect
    {"GetThumbRect", PyCSliderCtrl_get_thumb_rect, 1},      // @pymeth GetThumbRect|Get the control's thumb rect
    {"GetPos", PyCSliderCtrl_get_pos, 1},                   // @pymeth GetPos|Get the control's position
    {"SetPos", PyCSliderCtrl_set_pos, 1},                   // @pymeth SetPos|Set the control's position
    {"GetNumTics", PyCSliderCtrl_get_num_tics, 1},          // @pymeth GetNumTics|Get the number of tics in the control
    {"GetTicArray", PyCSliderCtrl_get_tic_array, 1},        // @pymeth GetTicArray|Get the array of tic positions
    {"GetTic", PyCSliderCtrl_get_tic, 1},                   // @pymeth GetTic|Get the position of the specified tic
    {"GetTicPos", PyCSliderCtrl_get_tic_pos,
     1},  // @pymeth GetTicPos|Get the position of the specified tic in client coordinates
    {"SetTic", PyCSliderCtrl_set_tic, 1},           // @pymeth SetTic|Set a tick at the position
    {"SetTicFreq", PyCSliderCtrl_set_tic_freq, 1},  // @pymeth SetTicFreq|Set the tic mark frequency
    {"ClearSel", PyCSliderCtrl_clear_sel, 1},       // @pymeth ClearSel|Clear any control selection
    {"VerifyPos", PyCSliderCtrl_verify_pos, 1},     // @pymeth VerifyPos|Verify the positon between min and max
    {"ClearTics", PyCSliderCtrl_clear_tics, 1},     // @pymeth ClearTics|Clear any tic marks from the control
    {NULL, NULL}};

ui_type_CObject PyCSliderCtrl::type("PyCSliderCtrl", &ui_control_object::type, RUNTIME_CLASS(CSliderCtrl),
                                    sizeof(PyCSliderCtrl), PYOBJ_OFFSET(PyCSliderCtrl), PyCSliderCtrl_methods,
                                    GET_PY_CTOR(PyCSliderCtrl));

/////////////////////////////////////////////////////////////////////
//
// PyCStatusBarCtrl
//
static CStatusBarCtrl *GetStatusBarCtrl(PyObject *self)
{
    // note we can only ask for a CWnd, if the PC is created from a resource based
    // dialog.  This is also the technique MFC uses (specifically appdlg.cpp)
    return (CStatusBarCtrl *)PyCWnd::GetPythonGenericWnd(self);
}
PyCStatusBarCtrl::PyCStatusBarCtrl() {}
PyCStatusBarCtrl::~PyCStatusBarCtrl() {}

// @pymethod <o PyCStatusBarCtrl>|win32ui|CreateStatusBarCtrl|Creates a progress control object. <om
// PyStatusBarCtrl.Create> creates the actual control.
PyObject *PyCStatusBarCtrl_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CStatusBarCtrl *pPC = new CStatusBarCtrl();
    return ui_assoc_object::make(PyCStatusBarCtrl::type, pPC);
}

// @pymethod |PyCStatusBarCtrl|CreateWindow|Creates the actual control.
PyObject *PyCStatusBarCtrl_create_window(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    if (!PyArg_ParseTuple(
            args, "i(iiii)Oi:CreateWindow",
            &style,  // @pyparm int|style||The style for the control.
            &rect.left, &rect.top, &rect.right, &rect.bottom,
            // @pyparm (left, top, right, bottom)|rect||The size and position of the control.
            &obParent,  // @pyparm <o PyCWnd>|parent||The parent window of the control.  Usually a <o PyCDialog>.
            &id))       // @pyparm int|id||The control's ID.
        return NULL;

    if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
        RETURN_TYPE_ERR("parent argument must be a window object");
    CWnd *pParent = GetWndPtr(obParent);
    if (pParent == NULL)
        return NULL;
    CStatusBarCtrl *pPC = GetStatusBarCtrl(self);
    if (!pPC)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pPC->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CStatusBarCtrl::Create");
    RETURN_NONE;
}

// @pymethod (width, height, spacing)|PyCStatusBarCtrl|GetBorders|Retrieve the status bar control's current widths of
// the horizontal and vertical borders and of the space between rectangles.

static PyObject *PyCStatusBarCtrl_get_borders(PyObject *self, PyObject *args)
{
    int nHorz, nVert, nSpacing;

    CHECK_NO_ARGS(args);

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);
    if (!pSB)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pSB->GetBorders(nHorz, nVert, nSpacing);
    GUI_END_SAVE;

    if (!ok)
        RETURN_ERR("CStatusBar::GetBorders");

    return Py_BuildValue("(iii)", nHorz, nVert, nSpacing);
}

// @pymethod (int)|PyCStatusBarCtrl|GetParts|Retrieve coordinates of the parts in a status bar control.

static PyObject *PyCStatusBarCtrl_get_parts(PyObject *self, PyObject *args)
{
    int i;
    int nParts, nRequested = -1;

    // @comm This function, as designed in MFC, returns both the *number* of parts, and,
    // through an OUT parameter, an array of ints giving the coordinates of the
    // parts.  There is also an IN parameter saying how many coordinates to give
    // back.  Here, we're explicitly changing the semantics a bit.
    //
    // <nl>GetParts() -> Tuple of all coordinates
    // <nl>GetParts(n) -> Tuple of the first n coordinates (or all coordinates, if
    // fewer than n)
    //
    // So, in Python, you can't simultaneously find out how many coordinates there
    // are, and retrieve a subset of them.  In a reasonable universe, there would
    // have been GetParts() -> int, and GetCoords() -> List.  This means that I
    // need to call the MFC method twice; once to find out how many there are, and
    // another time to get them.

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);
    if (!pSB)
        return NULL;

    if (!PyArg_ParseTuple(args,
                          "|i:GetParts",  // @pyparm int|nParts||The number of coordinates to retrieve
                          &nRequested))
        return NULL;

    if (nRequested == 0)
        RETURN_NONE;

    GUI_BGN_SAVE;
    nParts = pSB->GetParts(0, NULL);

    if ((nRequested == -1) || (nRequested > nParts))
        nRequested = nParts;

    int *pParts = new int[nParts];

    (void)pSB->GetParts(nRequested, pParts);
    GUI_END_SAVE;

    PyObject *parts = PyTuple_New(nParts);
    for (i = 0; i < nParts; i++) {
        PyTuple_SetItem(parts, i, Py_BuildValue("i", parts[i]));
    }

    delete pParts;
    return parts;
}

// @pymethod (left, top, right, bottom)|PyCStatusBarCtrl|GetRect|Retrieves the bounding rectangle of a part in a status
// bar control.

static PyObject *PyCStatusBarCtrl_get_rect(PyObject *self, PyObject *args)
{
    int nPane;
    RECT rect;

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    if (!PyArg_ParseTuple(args,
                          "i:GetRect",  // @pyparm int|nPane||Zero-based index of the part whose bounding rectangle is
                                        // to be retrieved.
                          &nPane))
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pSB->GetRect(nPane, &rect);
    GUI_END_SAVE;

    if (!ok)
        RETURN_ERR("CStatusBarCtrl::GetRect");

    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod text|PyCStatusBarCtrl|GetText|Retrieve the text from the given part of a status bar control.

PyObject *PyCStatusBarCtrl_get_text(PyObject *self, PyObject *args)
{
    int attr;
    int nPane;

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|nPane||Zero-based index of the part whose text is to be retrieved.

    if (!PyArg_ParseTuple(args, "i:GetText", &nPane))
        return NULL;

    int len;
    GUI_BGN_SAVE;
    len = pSB->GetTextLength(nPane, &attr);
    if (!len) {
        RETURN_ERR("CStatusBarCtrl::GetTextLength");
    }

    TCHAR *buf = new TCHAR[len];
    pSB->GetText(buf, nPane, &attr);
    GUI_END_SAVE;

    PyObject *ret = PyWinObject_FromTCHAR(buf);
    delete buf;
    return ret;
}

// @pymethod int|PyCStatusBarCtrl|GetTextAttr|Retrieve the attributes of the text in the given part of a status bar
// control.

static PyObject *PyCStatusBarCtrl_get_text_attr(PyObject *self, PyObject *args)
{
    int attr;
    int nPane;

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|nPane||Zero-based index of the part whose text is to be retrieved.

    if (!PyArg_ParseTuple(args, "i:GetTextAttr", &nPane))
        return NULL;

    GUI_BGN_SAVE;
    (void)pSB->GetTextLength(nPane, &attr);
    GUI_END_SAVE;

    return Py_BuildValue("i", attr);
}

// @pymethod int|PyCStatusBarCtrl|GetTextLength|Retrieve the length the text in the given part of a status bar control.

static PyObject *PyCStatusBarCtrl_get_text_length(PyObject *self, PyObject *args)
{
    int attr;
    int nPane;

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|nPane||Zero-based index of the part whose text is to be retrieved.

    if (!PyArg_ParseTuple(args, "i:GetTextLength", &nPane))
        return NULL;

    int len;
    GUI_BGN_SAVE;
    len = pSB->GetTextLength(nPane, &attr);
    GUI_END_SAVE;

    return Py_BuildValue("i", len);
}

// @pymethod |PyCStatusBarCtrl|SetMinHeight|Set the minimum height of a status bar control's drawing area.

static PyObject *PyCStatusBarCtrl_set_min_height(PyObject *self, PyObject *args)
{
    int nHeight;
    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|nHeight||Minimum height

    if (!PyArg_ParseTuple(args, "i:SetMinHeight", &nHeight))
        return NULL;

    GUI_BGN_SAVE;
    pSB->SetMinHeight(nHeight);
    GUI_END_SAVE;

    RETURN_NONE;
}

// @pymethod |PyCStatusBarCtrl|SetParts|Sets the number of parts in a status bar control and the coordinate of the right
// edge of each part.

static PyObject *PyCStatusBarCtrl_set_parts(PyObject *self, PyObject *args)
{
    Py_ssize_t nParts = 0;
    Py_ssize_t i;
    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int...|coord||Coordinates of each part

    PyObject *obPart;

    nParts = PyTuple_Size(args);
    int *pParts = new int[nParts];
    for (i = 0; i < nParts; i++) {
        obPart = PyTuple_GetItem(args, i);
        if (PyArg_Parse(obPart, "i", &pParts[i])) {
            delete pParts;
            RETURN_TYPE_ERR("arguments must be integers");
        }
    }

    GUI_BGN_SAVE;
    pSB->SetParts(PyWin_SAFE_DOWNCAST(nParts, Py_ssize_t, int), pParts);
    GUI_END_SAVE;

    delete pParts;

    RETURN_NONE;
}

// @pymethod |PyCStatusBarCtrl|SetSimple|Specify whether a status bar control displays simple text or displays all
// control parts set by a previous call to SetParts.

static PyObject *PyCStatusBarCtrl_set_simple(PyObject *self, PyObject *args)
{
    int bSimple;
    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|bSimple||If non-zero, displays simple text.

    if (!PyArg_ParseTuple(args, "i:SetSimple", &bSimple))
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pSB->SetSimple(bSimple);
    GUI_END_SAVE;

    if (!ok) {
        RETURN_ERR("CStatusBarCtrl::SetSimple");
    }

    RETURN_NONE;
}

// @pymethod |PyCStatusBarCtrl|SetText|Set the text in the given part of a status bar control.

PyObject *PyCStatusBarCtrl_set_text(PyObject *self, PyObject *args)
{
    TCHAR *buf;
    PyObject *obbuf;
    int nPane, nType;

    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm string|text||The text to display
    // @pyparm int|nPane||Zero-based index of the part to set.
    // @pyparm int|nType||Type of drawing operation.
    // @comm The drawing type can be set to one of:~
    // * 0 - The text is drawn with a border to appear lower than
    // the plane of the status bar.~
    // * win32con.SBT_NOBORDERS - The text is drawn without borders.~
    // * win32con.SBT_OWNERDRAW - The text is drawn by the parent window.~
    // * win32con.SBT_POPOUT - The text is drawn with a border to appear
    // higher than the plane of the status bar.

    if (!PyArg_ParseTuple(args, "Oii:SetText", &obbuf, &nPane, &nType))
        return NULL;
    if (!PyWinObject_AsTCHAR(obbuf, &buf, FALSE))
        return NULL;
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pSB->SetText(buf, nPane, nType);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(buf);
    if (!ok) {
        RETURN_ERR("CStatusBarCtrl::SetText");
    }

    RETURN_NONE;
}

// @pymethod |PyCStatusBarCtrl|SetTipText|Sets the tooltip text for a pane in a status bar. The status bar must have
// been created with the afxres.SBT_TOOLTIPS control style to enable ToolTips.

PyObject *PyCStatusBarCtrl_set_tip_text(PyObject *self, PyObject *args)
{
    int nPane;
    TCHAR *buf;
    PyObject *obbuf;
    CStatusBarCtrl *pSB = GetStatusBarCtrl(self);

    if (!pSB)
        return NULL;

    // @pyparm int|nPane||The zero-based index of status bar pane to receive the tooltip text.
    // @pyparm string|text||The string containing the tooltip text.

    // @comm Pay attention, this tooltip text is ONLY displayed in two situations:
    // <nl>1. When the corresponding pane in the status bar contains only an icon.
    // <nl>2. When the corresponding pane in the status bar contains text that is truncated due to the size of the pane.
    // <nl>To make the tooltip appear even if the text is not truncated, you could add additional spaces to the end of
    // the pane text.

    if (!PyArg_ParseTuple(args, "iO:SetTipText", &nPane, &obbuf))
        return NULL;
    if (!PyWinObject_AsTCHAR(obbuf, &buf, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pSB->SetTipText(nPane, buf);  // @pyseemfc CStatusBarCtrl|SetTipText
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(buf);
    RETURN_NONE;
}

// @object PyCStatusBarCtrl|A windows progress bar control.  Encapsulates an MFC <c CStatusBarCtrl> class.  Derived from
// <o PyCControl>.
static struct PyMethodDef PyCStatusBarCtrl_methods[] = {
    {"CreateWindow", PyCStatusBarCtrl_create_window,
     1},  // @pymeth CreateWindow|Creates the window for a new progress bar object.
    {"GetBorders", PyCStatusBarCtrl_get_borders,
     1},  // @pymeth GetBorders|Retrieve the status bar control's current widths of the horizontal and vertical borders
          // and of the space between rectangles.
    {"GetParts", PyCStatusBarCtrl_get_borders,
     1},  // @pymeth GetParts|Retrieve coordinates of the parts in a status bar control.
    {"GetRect", PyCStatusBarCtrl_get_rect,
     1},  // @pymeth GetRect|Retrieves the bounding rectangle of a part in a status bar control.
    {"GetText", PyCStatusBarCtrl_get_text, 1},  // @pymeth GetText|Retrieves the text of a part in a status bar control.
    {"GetTextAttr", PyCStatusBarCtrl_get_text_attr,
     1},  // @pymeth GetTextAttr|Retrieves the text attributes of a part in a status bar control.
    {"GetTextLength", PyCStatusBarCtrl_get_text_length,
     1},  // @pymeth GetTextLength|Retrieves the length of the text in a part in a status bar control.
    {"SetMinHeight", PyCStatusBarCtrl_set_min_height,
     1},  // @pymeth SetMinHeight|Set the minimum height of a status bar control's drawing area.
    {"SetParts", PyCStatusBarCtrl_set_parts, 1},  // @pymeth SetParts|Sets the number of parts in a status bar control
                                                  // and the coordinate of the right edge of each part.
    {"SetText", PyCStatusBarCtrl_set_text,
     1},  // @pymeth SetText|Set the text in the given part of a status bar control.
    {"SetTipText", PyCStatusBarCtrl_set_tip_text,
     1},  // @pymeth SetTipText|Sets the tooltip text for a pane in a status bar.
    {NULL, NULL}};

ui_type_CObject PyCStatusBarCtrl::type("PyCStatusBarCtrl", &ui_control_object::type, RUNTIME_CLASS(CStatusBarCtrl),
                                       sizeof(PyCStatusBarCtrl), PYOBJ_OFFSET(PyCStatusBarCtrl),
                                       PyCStatusBarCtrl_methods, GET_PY_CTOR(PyCStatusBarCtrl));

// A spin control

static CSpinButtonCtrl *GetSpinButtonCtrl(PyObject *self)
{
    return (CSpinButtonCtrl *)PyCWnd::GetPythonGenericWnd(self);
}

// @pymethod int|PyCSpinButtonCtrl|GetPos|Obtains the current position for a spin button control.
PyObject *PyCSpinButtonCtrl_GetPos(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, "GetPos");
    CSpinButtonCtrl *pSB = GetSpinButtonCtrl(self);

    if (!pSB)
        return NULL;
    return PyInt_FromLong(pSB->GetPos());
}

// @pymethod int|PyCSpinButtonCtrl|SetPos|Sets the current position for a spin button control.
PyObject *PyCSpinButtonCtrl_SetPos(PyObject *self, PyObject *args)
{
    int pos;
    // @pyparm int|pos||The new position.
    if (!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
    CSpinButtonCtrl *pSB = GetSpinButtonCtrl(self);

    if (!pSB)
        return NULL;
    GUI_BGN_SAVE;
    int oldPos = pSB->SetPos(pos);
    GUI_END_SAVE;
    // @rdesc The result is the previous position.
    return PyInt_FromLong(oldPos);
}

// @pymethod int|PyCSpinButtonCtrl|SetRange|Sets the upper and lower limits (range) for a spin button control.
PyObject *PyCSpinButtonCtrl_SetRange(PyObject *self, PyObject *args)
{
    int min, max;
    if (!PyArg_ParseTuple(args, "ii", &min, &max))
        return NULL;
    CSpinButtonCtrl *pSB = GetSpinButtonCtrl(self);

    if (!pSB)
        return NULL;
    GUI_BGN_SAVE;
    pSB->SetRange(min, max);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCSpinButtonCtrl|SetRange32|Sets the 32 bit upper and lower limits (range) for a spin button control.
PyObject *PyCSpinButtonCtrl_SetRange32(PyObject *self, PyObject *args)
{
    int min, max;
    if (!PyArg_ParseTuple(args, "ii", &min, &max))
        return NULL;
    CSpinButtonCtrl *pSB = GetSpinButtonCtrl(self);

    if (!pSB)
        return NULL;
    GUI_BGN_SAVE;
    pSB->SetRange(min, max);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCSpinButtonCtrl|A windows spin button control.  Encapsulates an MFC CSpinButtonCtrl object.
static struct PyMethodDef PyCSpinButtonCtrl_methods[] = {
    {"GetPos", PyCSpinButtonCtrl_GetPos, 1},  // @pymeth GetPos|Obtains the current position for a spin button control.
    {"SetPos", PyCSpinButtonCtrl_SetPos, 1},  // @pymeth SetPos|Sets the current position for a spin button control.
    {"SetRange", PyCSpinButtonCtrl_SetRange,
     1},  // @pymeth SetRange|Sets the upper and lower limits (range) for a spin button control.
    {"SetRange32", PyCSpinButtonCtrl_SetRange32,
     1},  // @pymeth SetRange32|Sets the upper and lower limits (range) for a spin button control.
    {NULL}};

ui_type_CObject PyCSpinButtonCtrl::type("PyCSpinButtonCtrl", &ui_control_object::type, RUNTIME_CLASS(CSpinButtonCtrl),
                                        sizeof(PyCSpinButtonCtrl), PYOBJ_OFFSET(PyCSpinButtonCtrl),
                                        PyCSpinButtonCtrl_methods, GET_PY_CTOR(PyCSpinButtonCtrl));
