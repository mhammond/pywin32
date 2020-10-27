/*

    menu data types

    Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

    Note - menus are implemented totally in the API, and not using
    MFC at all (ie, using HMENU's rather than CMenu's)

    However, some of the menus that exist, and can be manipulated with this
    data type have underlying MFC menus.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32menu.h"
#include "win32dll.h"

extern CWnd *GetWndPtr(PyObject *);

bool PyCMenu::CheckCppObject(ui_type *ui_type_check) const
{
    if (!ui_assoc_object::CheckCppObject(ui_type_check))
        return false;
    HMENU handle = (HMENU)assoc;
    if (!::IsMenu(handle))
        RETURN_ERR("The menu associated with the object is not valid");
    return true;
}
// this returns a pointer that should not be stored.
HMENU PyCMenu::GetMenu(PyObject *self) { return (HMENU)GetGoodCppObject(self, &type); }

// @pymethod <o PyCMenu>|win32ui|CreateMenu|Creates a menu object.
PyObject *PyCMenu::create_menu(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    HMENU hMenu = CreateMenu();
    if (!hMenu)
        RETURN_API_ERR("CreateMenu");
    return ui_assoc_object::make(PyCMenu::type, hMenu);
}
// @pymethod <o PyCMenu>|win32ui|CreatePopupMenu|Creates a popup menu object.
PyObject *PyCMenu::create_popup(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
        RETURN_API_ERR("CreatePopupMenu");
    return ui_assoc_object::make(PyCMenu::type, hMenu);
}
// @pymethod <o PyCMenu>|win32ui|LoadMenu|Creates and loads a menu resource from a DLL.
PyObject *PyCMenu::load_menu(PyObject *self, PyObject *args)
{
    int id;
    PyObject *dllObj = NULL;
    HMODULE hMod = NULL;
    // @pyparm int|id||The Id of the menu to load.
    // @pyparm <o PyDLL>|dll|None|The DLL to load from.
    if (!PyArg_ParseTuple(args, "i|O", &id, &dllObj))
        return NULL;
    if (dllObj && dllObj != Py_None) {
        // passed a DLL object.
        if (!is_uiobject(dllObj, &dll_object::type))
            RETURN_TYPE_ERR("passed object must be a PyDLL object");
        hMod = ((dll_object *)dllObj)->GetDll();
    }
    else
        hMod = AfxFindResourceHandle(MAKEINTRESOURCE(id), RT_MENU);
    HMENU hMenu = ::LoadMenu(hMod, MAKEINTRESOURCE(id));
    if (!hMenu)
        RETURN_API_ERR("LoadMenu");
    return ui_assoc_object::make(PyCMenu::type, hMenu);
}

/////////////////////////////////////////////////////////////////////
//
// menu
//
//////////////////////////////////////////////////////////////////////
void PyCMenu::SetAssocInvalid()
{
    return;  // do nothing.  Dont call base as dont want my handle wiped.
}
// Menu Methods
// @pymethod |PyCMenu|AppendMenu|Appends a new item to the end of a menu. Python can specify the state of the menu item
// by setting values in nFlags.
PyObject *PyCMenu::AppendMenu(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    TCHAR *value = NULL;
    PyObject *obvalue = Py_None;
    int id = 0;
    int flags;
    if (!PyArg_ParseTuple(args, "i|iO",
                          &flags,  // @pyparm int|flags||Specifies information about the state of the new menu item when
                                   // it is added to the menu.  May be a combination of the win32con.MF_* values.
                          &id,     // @pyparm int|id|0|Specifies either the command ID of the new menu item.
                          &obvalue))  // @pyparm string/None|value|None|Specifies the content of the new menu item.  If
                                      // used, flags must contain win32con.MF_STRING.
        return NULL;
    if (!PyWinObject_AsTCHAR(obvalue, &value, TRUE))
        return NULL;
    if (!::AppendMenu(hMenu, flags, id, value)) {
        PyWinObject_FreeTCHAR(value);
        RETURN_API_ERR("::AppendMenu");
    }
    PyWinObject_FreeTCHAR(value);
    RETURN_NONE;
}

// @pymethod string|PyCMenu|DeleteMenu|Deletes the specified menu item.
PyObject *PyCMenu::DeleteMenu(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    int id, flags;
    if (!PyArg_ParseTuple(args, "ii",
                          &id,      // @pyparm int|id||The id of the item being deleted.
                          &flags))  // @pyparm int|flags||Specifies how the id parameter is interpreted. It must be one
                                    // of win32con.MF_BYCOMMAND or win32con.MF_BYPOSITION.
        return NULL;
    if (!::DeleteMenu(hMenu, id, flags))
        RETURN_API_ERR("::DeleteMenu");
    RETURN_NONE;
}

// @pymethod int|PyCMenu|EnableMenuItem|Enables, disables, or dims a menu item.
PyObject *PyCMenu::EnableMenuItem(PyObject *self, PyObject *args)
{
    // @comm The <om PyCMenu.CreateMenu>, <om PyCMenu.InsertMenu>, <om PyCMenu.ModifyMenu>,
    // and <om PyCMenu.LoadMenuIndirect> member functions can also set the state
    // (enabled, disabled, or dimmed) of a menu item.
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    int id, flags;
    if (!PyArg_ParseTuple(args, "ii",
                          &id,  // @pyparm int|id||Specifies the command ID of the menu item. This parameter can specify
                                // pop-up menu items as well as standard menu items.
                          &flags))  // @pyparm int|flags||Specifies the action to take. It can be a combination of
                                    // MF_DISABLED, MF_ENABLED, or MF_GRAYED, with MF_BYCOMMAND or MF_BYPOSITION
        return NULL;
    GUI_BGN_SAVE;
    int rc = ::EnableMenuItem(hMenu, id, flags);
    GUI_END_SAVE;

    return Py_BuildValue("i", rc);
}
// @pymethod int|PyCMenu|GetHandle|Returns the menu object's underlying hMenu.
PyObject *PyCMenu::GetHandle(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    return Py_BuildValue("i", hMenu);
}
// @pymethod int|PyCMenu|GetMenuItemCount|Determines the number of items in a menu.
PyObject *PyCMenu::GetMenuItemCount(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    return Py_BuildValue("i", ::GetMenuItemCount(hMenu));
    // @rdesc The number of items in the menu if the function is successful; otherwise -1.
}
// @pymethod int|PyCMenu|GetMenuItemID|Returns the item ID for the specified item in a pop-up menu.
PyObject *PyCMenu::GetMenuItemID(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    int pos;
    // @pyparm int|pos||The position (zero-based) of the menu item whose ID is being retrieved.
    if (!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
    // @comm If the specified item is a pop-up menu (as opposed to an item within the pop-up menu),
    // the return value is -1. If nPos corresponds to a SEPARATOR menu item,
    // the return value is 0.
    return Py_BuildValue("i", ::GetMenuItemID(hMenu, pos));
}
// @pymethod string|PyCMenu|GetMenuString|Returns the string for a specified menu item.
PyObject *PyCMenu::GetMenuString(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    int id, flags = MF_BYCOMMAND;
    if (!PyArg_ParseTuple(args, "i|i",
                          &id,      // @pyparm int|id||The id of the item being requested.
                          &flags))  // @pyparm int|flags|win32con.MF_BYCOMMAND|Specifies how the id parameter is
                                    // interpreted. It must be one of win32con.MF_BYCOMMAND or win32con.MF_BYPOSITION.
        return NULL;
    TCHAR buf[128];
    if (::GetMenuString(hMenu, id, buf, sizeof(buf) / sizeof(TCHAR), flags) == 0)
        buf[0] = 0;
    return PyWinObject_FromTCHAR(buf);
}

// @pymethod <o PyCMenu>|PyCMenu|GetSubMenu|Returns a submenu.
PyObject *PyCMenu::GetSubMenu(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    int pos;
    // @pyparm int|pos||The position (zero-based) of the menu item being retrieved.
    if (!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
    HMENU hSubMenu = ::GetSubMenu(hMenu, pos);
    if (hSubMenu == NULL)
        RETURN_ERR("There is no sub-menu at that position");
    return ui_assoc_object::make(PyCMenu::type, hSubMenu);
}
// @pymethod |PyCMenu|InsertMenu|Inserts an item into a menu.
PyObject *PyCMenu::InsertMenu(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    TCHAR *value = NULL;
    int id = 0;
    int flags;
    int pos;
    BOOL rc, bHaveInt = TRUE;
    PyObject *obsubMenu = NULL, *obvalue = Py_None;
    if (!PyArg_ParseTuple(
            args, "ii|OO:InsertMenu",
            &pos,        // @pyparm int|pos||The position (zero-based) the item should be inserted.
            &flags,      // @pyparm int|flags||Flags for the new item.
            &obsubMenu,  // @pyparm int/<o PyCMenu>|id|0|The ID for a new menu item, or handle to a submenu
            &obvalue))   // @pyparm string/None|value|None|A string for the menu item.
        return NULL;

    if (obsubMenu) {
        id = PyInt_AsLong(obsubMenu);
        if (id == -1 && PyErr_Occurred()) {
            PyErr_Clear();
            bHaveInt = FALSE;
        }
    }
    if (!PyWinObject_AsTCHAR(obvalue, &value, TRUE))
        return NULL;

    if (bHaveInt)
        rc = ::InsertMenu(hMenu, pos, flags, id, value);
    else {
        HMENU hsubMenu = GetMenu(obsubMenu);
        rc = ::InsertMenu(hMenu, pos, flags, (UINT_PTR)hsubMenu, value);
    }
    PyWinObject_FreeTCHAR(value);
    if (!rc)
        RETURN_API_ERR("::InsertMenu");
    RETURN_NONE;
}

// @pymethod |PyCMenu|ModifyMenu|Modify an item in a menu.
PyObject *PyCMenu::ModifyMenu(PyObject *self, PyObject *args)
{
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    TCHAR *value = NULL;
    PyObject *obvalue = Py_None;
    int id = 0;
    int flags;
    int pos;
    if (!PyArg_ParseTuple(args, "ii|iO",
                          &pos,       // @pyparm int|pos||The position (zero-based) the item to be changed.
                          &flags,     // @pyparm int|flags||Flags for the item.
                          &id,        // @pyparm int|id|0|The ID for the item.
                          &obvalue))  // @pyparm string/None|value|None|A string for the menu item.
    {
        return NULL;
    }
    if (!PyWinObject_AsTCHAR(obvalue, &value, TRUE))
        return NULL;
    BOOL rc = ::ModifyMenu(hMenu, pos, flags, id, value);
    PyWinObject_FreeTCHAR(value);
    if (!rc)
        RETURN_API_ERR("::ModifyMenu");
    RETURN_NONE;
}
// @pymethod |PyCMenu|TrackPopupMenu|Creates a popup menu anywhere on the screen.
PyObject *PyCMenu::TrackPopupMenu(PyObject *self, PyObject *args)
{
    // @comm The TrackPopupMenu function displays a floating pop-up menu at the
    // specified location and tracks the selection of items on the pop-up menu.
    // The floating pop-up menu can appear anywhere on the screen.
    HMENU hMenu = GetMenu(self);
    if (!hMenu)
        return NULL;
    HWND hTarget;
    PyObject *wndObject = NULL;
    int flags = TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON;
    int x, y;
    if (!PyArg_ParseTuple(
            args, "(ii)|iO", &x, &y,  // @pyparm (int, int)|(x,y)||The position for the menu..
            &flags,                   // @pyparm
                     // int|flags|win32con.TPM_LEFTALIGN\|win32con.TPM_LEFTBUTTON\|win32con.TPM_RIGHTBUTTON|Flags
                     // for the menu.
            &wndObject))  // @pyparm <o PyCWnd>|owner|(main application frame)|The owner of the menu.

        return NULL;
    if (wndObject) {
        CWnd *wnd = GetWndPtr(wndObject);
        if (wnd == NULL)
            return NULL;
        hTarget = wnd->m_hWnd;
    }
    else {
        CWinApp *pApp = GetApp();
        if (!pApp)
            return NULL;
        hTarget = pApp->m_pMainWnd->GetSafeHwnd();
    }

    GUI_BGN_SAVE;
    BOOL rc = ::TrackPopupMenu(hMenu, flags, x, y, 0, hTarget, NULL);
    GUI_END_SAVE;
    // @rdesc If the underlying MFC function fails, but TPM_RETURNCMD is set in the flags parameter, then None is
    // returned instead of the normal exception.
    if (!rc) {
        if (flags & TPM_RETURNCMD) {
            RETURN_NONE;
        }
        else {
            RETURN_API_ERR("TrackPopupMenu");
        }
    }
    return Py_BuildValue("i", rc);
}

// Menu Methods
// @object PyCMenu|A windows menu.  Encapsulates an MFC <c CMenu> class
static struct PyMethodDef ui_menu_methods[] = {
    {"AppendMenu", (PyCFunction)PyCMenu::AppendMenu,
     1},  // @pymeth AppendMenu|Appends a new item to the end of a menu. Python can specify the state of the menu item
          // by setting values in nFlags.
    {"DeleteMenu", (PyCFunction)PyCMenu::DeleteMenu, 1},  // @pymeth DeleteMenu|Deletes the specified menu item.
    {"Detach", (PyCFunction)PyCMenu::GetHandle, 1},
    {"EnableMenuItem", (PyCFunction)PyCMenu::EnableMenuItem,
     1},  // @pymeth EnableMenuItem|Enables, disables, or dims a menu item.
    {"GetHandle", (PyCFunction)PyCMenu::GetHandle, 1},  // @pymeth GetHandle|Returns the menu object's underlying hMenu.
    {"GetMenuItemCount", (PyCFunction)PyCMenu::GetMenuItemCount,
     1},  // @pymeth GetMenuItemCount|Determines the number of items in a menu.
    {"GetMenuItemID", (PyCFunction)PyCMenu::GetMenuItemID,
     1},  // @pymeth GetMenuItemID|Returns the item ID for the specified item in a pop-up menu.
    {"GetMenuString", (PyCFunction)PyCMenu::GetMenuString,
     1},  // @pymeth GetMenuString|Returns the string for a specified menu item.
    {"GetSubMenu", (PyCFunction)PyCMenu::GetSubMenu, 1},  // @pymeth GetSubMenu|Returns a submenu.
    {"InsertMenu", (PyCFunction)PyCMenu::InsertMenu, 1},  // @pymeth InsertMenu|Inserts an item into a menu.
    {"ModifyMenu", (PyCFunction)PyCMenu::ModifyMenu, 1},  // @pymeth ModifyMenu|Modify an item in a menu.
    {"TrackPopupMenu", (PyCFunction)PyCMenu::TrackPopupMenu,
     1},  // @pymeth TrackPopupMenu|Creates a popup menu anywhere on the screen.
    {NULL, NULL}};

ui_type PyCMenu::type("PyCMenu", &ui_assoc_object::type, sizeof(PyCMenu), PYOBJ_OFFSET(PyCMenu), ui_menu_methods,
                      GET_PY_CTOR(PyCMenu));
