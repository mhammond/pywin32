/*

    win32 window data type

    Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32doc.h"
#include "win32dc.h"
#include "win32control.h"
#include "win32toolbar.h"
#include "win32menu.h"
#include "win32gdi.h"
#include "win32font.h"
#include "win32cmdui.h"
#include "win32rgn.h"
#include "reswin32ui.h"
#include "afxstat_.h"

static char *szErrMsgBadHandle = "The window handle does not specify a valid window";
#define CHECK_HWND_VALID(pWnd)       \
    if (!::IsWindow((pWnd)->m_hWnd)) \
        RETURN_ERR(szErrMsgBadHandle);

extern BOOL bInFatalShutdown;

IMPLEMENT_DYNAMIC(CPythonFrameWnd, CFrameWnd);
IMPLEMENT_DYNAMIC(CPythonMDIChildWnd, CMDIChildWnd);
IMPLEMENT_DYNAMIC(CPythonMDIFrameWnd, CMDIFrameWnd);

// Beat protected members!
class WndHack : public CWnd {
   public:
    LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
        return CWnd::DefWindowProc(message, wParam, lParam);
    }
    BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
    {
        return CWnd::OnWndMsg(message, wParam, lParam, pResult);
    }
    BOOL OnEraseBkgnd(CDC *pDC) { return CWnd::OnEraseBkgnd(pDC); }
    void OnPaint() { CWnd::OnPaint(); }
    HCURSOR OnQueryDragIcon() { return CWnd::OnQueryDragIcon(); }
    HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) { return CWnd::OnCtlColor(pDC, pWnd, nCtlColor); }
    void OnClose() { CWnd::OnClose(); }
    LRESULT OnNcHitTest(CPoint point) { return CWnd::OnNcHitTest(point); }
    BOOL OnSetCursor(CWnd *pWnd, UINT ht, UINT msg) { return CWnd::OnSetCursor(pWnd, ht, msg); }
    int OnMouseActivate(CWnd *pWnd, UINT ht, UINT msg) { return CWnd::OnMouseActivate(pWnd, ht, msg); }
    BOOL PreCreateWindow(CREATESTRUCT &cs) { return CWnd::PreCreateWindow(cs); }
    BOOL OnQueryNewPalette() { return CWnd::OnQueryNewPalette(); }
};

BOOL Python_check_message(const MSG *msg)  // TRUE if fully processed.
{
    // Our Python convention is TRUE means "pass it on", which we only want to do if the message if valid, and the
    // callback returns TRUE
    BOOL ret = FALSE;
    ui_assoc_object *pObj = NULL;
    PyObject *method;
    CWnd *pWnd = bInFatalShutdown ? NULL : CWnd::FromHandlePermanent(msg->hwnd);
    if (pWnd) {
        // is_uiobjects calls python methods, must already hold lock
        CEnterLeavePython _celp;
        if ((pObj = ui_assoc_object::GetAssocObject(pWnd)) && pObj->is_uiobject(&PyCWnd::type) &&
            ((PyCWnd *)pObj)->pMessageHookList &&
            ((PyCWnd *)pObj)->pMessageHookList->Lookup(msg->message, (void *&)method)) {
#ifdef TRACE_CALLBACKS
            TRACE("Message callback: message %04X, object %s (hwnd %p) (%p)\n", msg->message,
                  (const char *)GetReprText(pObj), pWnd, pWnd->GetSafeHwnd());
#endif
            ret = Python_callback(method, msg) == 0;
        }
    }

    Py_XDECREF(pObj);
    return ret;
}

BOOL Python_check_key_message(const MSG *msg)
{
    CEnterLeavePython _celp;
    ui_assoc_object *pObj = NULL;
    BOOL bPassOn = TRUE;
    CWnd *pWnd = msg->hwnd ? CWnd::FromHandlePermanent(msg->hwnd) : NULL;
    if (pWnd && (pObj = ui_assoc_object::GetAssocObject(pWnd)) && pObj->is_uiobject(&PyCWnd::type))
        bPassOn = ((PyCWnd *)pObj)->check_key_stroke(msg->wParam);
    Py_XDECREF(pObj);
    return !bPassOn;
}

// WARNING - the return ptr may be temporary.
CWnd *GetWndPtrFromParam(PyObject *ob, ui_type_CObject &type)
{
    if (PyLong_Check(ob) || PyLong_Check(ob)) {
        HWND hwnd = 0;
        if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&hwnd) || !IsWindow(hwnd))
            RETURN_ERR(szErrMsgBadHandle);
        CWnd *ret = CWnd::FromHandle(hwnd);
        if (ret == NULL)
            RETURN_ERR("The handle could not be converted to a window (CWnd::FromHandle() failed!)");
        return ret;
    }
    else if (ui_base_class::is_uiobject(ob, &type)) {
        return (CWnd *)PyCWnd::GetPythonGenericWnd(ob, &type);
    }
    else {
        char buf[128];
        snprintf(buf, sizeof(buf), "Argument must be a %s object, or integer containing a HWND", type.tp_name);
        RETURN_ERR(buf);
    }
}

PyObject *PyWinObject_FromCWnd(CWnd *pWnd)
{
    return (PyObject *)ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}
CWnd *GetWndPtr(PyObject *self) { return (CWnd *)PyCWnd::GetPythonGenericWnd(self); }
CWnd *GetWndPtrGoodHWnd(PyObject *self)
{
    CWnd *ret = PyCWnd::GetPythonGenericWnd(self);
    if (ret /*&& ret->m_hWnd*/ && !::IsWindow(ret->m_hWnd)) {
        RETURN_ERR(szErrMsgBadHandle);
    }
    return ret;
}

CFrameWnd *GetFramePtr(PyObject *self) { return (CFrameWnd *)PyCWnd::GetPythonGenericWnd(self, &PyCFrameWnd::type); }
CPythonMDIChildWnd *GetPythonFrame(PyObject *self)
{
    return (CPythonMDIChildWnd *)PyCWnd::GetPythonGenericWnd(self, &PyCFrameWnd::type);
}
CMDIFrameWnd *GetMDIFrame(PyObject *self)
{
    return (CMDIFrameWnd *)PyCWnd::GetPythonGenericWnd(self, &PyCMDIFrameWnd::type);
}

// @object SCROLLINFO tuple|Tuple representing a SCROLLINFO struct
// @tupleitem 0|int|addnMask|Additional mask information.  Python automatically fills the mask for valid items, so
// currently the only valid values are zero, and win32con.SIF_DISABLENOSCROLL.
// @tupleitem 1|int|min|The minimum scrolling position.  Both min and max, or neither, must be provided.
// @tupleitem 2|int|max|The maximum scrolling position.  Both min and max, or neither, must be provided.
// @tupleitem 3|int|page|Specifies the page size. A scroll bar uses this value to determine the appropriate size of the
// proportional scroll box.
// @tupleitem 4|int|pos|Specifies the position of the scroll box.
// @tupleitem 5|int|trackPos|Specifies the immediate position of a scroll box that the user
// is dragging. An application can retrieve this value while processing
// the SB_THUMBTRACK notification message. An application cannot set
// the immediate scroll position; the <om PyCWnd.SetScrollInfo> function ignores
// this member.
// @comm When returned from a method, will always be a tuple of size 6, and items may be None if not available.
// @comm When passed as an arg, it must have the addn mask attribute, but all other items may be None, or not exist.
BOOL ParseSCROLLINFOTuple(PyObject *args, SCROLLINFO *pInfo)
{
    PyObject *ob;
    Py_ssize_t len = PyTuple_Size(args);
    if (len < 1 || len > 5) {
        PyErr_SetString(PyExc_TypeError, "SCROLLINFO tuple has invalid size");
        return FALSE;
    }
    assert(!PyErr_Occurred());  //	PyErr_Clear(); // clear any errors, so I can detect my own.
    // 0 - mask.
    if ((ob = PyTuple_GetItem(args, 0)) == NULL)
        return FALSE;
    pInfo->fMask = (UINT)PyLong_AsLong(ob);
    // 1/2 - nMin/nMax
    if (len == 2) {
        PyErr_SetString(PyExc_TypeError, "SCROLLINFO - Both min and max, or neither, must be provided.");
        return FALSE;
    }
    if (len < 3)
        return TRUE;
    if ((ob = PyTuple_GetItem(args, 1)) == NULL)
        return FALSE;
    if (ob != Py_None) {
        pInfo->fMask |= SIF_RANGE;
        pInfo->nMin = PyLong_AsLong(ob);
        if ((ob = PyTuple_GetItem(args, 2)) == NULL)
            return FALSE;
        pInfo->nMax = PyLong_AsLong(ob);
    }
    // 3 == nPage.
    if (len < 4)
        return TRUE;
    if ((ob = PyTuple_GetItem(args, 3)) == NULL)
        return FALSE;
    if (ob != Py_None) {
        pInfo->fMask |= SIF_PAGE;
        pInfo->nPage = PyLong_AsLong(ob);
    }
    // 4 == nPos
    if (len < 5)
        return TRUE;
    if ((ob = PyTuple_GetItem(args, 4)) == NULL)
        return FALSE;
    if (ob != Py_None) {
        pInfo->fMask |= SIF_POS;
        pInfo->nPos = PyLong_AsLong(ob);
    }
    // 5 == trackpos
    if (len < 6)
        return TRUE;
    if ((ob = PyTuple_GetItem(args, 5)) == NULL)
        return FALSE;
    if (ob != Py_None) {
        pInfo->nTrackPos = PyLong_AsLong(ob);
    }
    return TRUE;
}

PyObject *MakeSCROLLINFOTuple(SCROLLINFO *pInfo)
{
    PyObject *ret = PyTuple_New(6);
    if (ret == NULL)
        return NULL;
    PyTuple_SET_ITEM(ret, 0, PyLong_FromLong(0));
    if (pInfo->fMask & SIF_RANGE) {
        PyTuple_SET_ITEM(ret, 1, PyLong_FromLong(pInfo->nMin));
        PyTuple_SET_ITEM(ret, 2, PyLong_FromLong(pInfo->nMax));
    }
    else {
        Py_INCREF(Py_None);
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(ret, 1, Py_None);
        PyTuple_SET_ITEM(ret, 2, Py_None);
    }
    if (pInfo->fMask & SIF_PAGE) {
        PyTuple_SET_ITEM(ret, 3, PyLong_FromLong(pInfo->nPage));
    }
    else {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(ret, 3, Py_None);
    }
    if (pInfo->fMask & SIF_POS) {
        PyTuple_SET_ITEM(ret, 4, PyLong_FromLong(pInfo->nPos));
    }
    else {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(ret, 4, Py_None);
    }
    PyTuple_SET_ITEM(ret, 5, PyLong_FromLong(pInfo->nTrackPos));
    return ret;
}

/////////////////////////////////////////////////////////////////////
//
// win32ui methods that deal with windows.
//
//////////////////////////////////////////////////////////////////////

// @pymethod <o PyCWnd>|win32ui|CreateWnd|Creates an unitialized <o PyCWnd>
PyObject *ui_window_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = new CPythonWndFramework<CWnd>();
    PyCWnd *pRet = (PyCWnd *)ui_assoc_object::make(PyCWnd::type, pWnd, TRUE);
    // We explicitly created this CWnd, so we must explicitly nuke it!
    if (pRet) {
        pRet->bManualDelete = TRUE;
    }
    return pRet;
}
// @pymethod |PyCWnd|CreateWindow|Creates the actual window
PyObject *ui_window_create_window(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    TCHAR *szClass = NULL, *szWndName = NULL;
    PyObject *obClass, *obWndName, *ret = NULL;
    CCreateContext *pCCPass = NULL;
    PythonCreateContext cc;
    PyObject *contextObject = Py_None;
    if (!PyArg_ParseTuple(args, "OOi(iiii)Oi|O:CreateWindow",
                          &obClass,    // @pyparm string|classId||The class ID for the window, or None
                          &obWndName,  // @pyparm string|windowName||The title for the window, or None
                          &style,      // @pyparm int|style||The style for the window.
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom)|rect||The size and position of the window.
                          &obParent,        // @pyparm <o PyCWnd>|parent||The parent window of the new window..
                          &id,              // @pyparm int|id||The control's ID.
                          &contextObject))  // @pyparm object|context|None|A CreateContext object.
        return NULL;

    CWnd *pParent;
    if (obParent == Py_None)
        pParent = NULL;
    else if (ui_base_class::is_uiobject(obParent, &PyCWnd::type)) {
        pParent = GetWndPtr(obParent);
        if (pParent == NULL)
            return NULL;
    }
    else
        RETURN_TYPE_ERR("parent argument must be a window object, or None");
    if (contextObject != Py_None) {
        cc.SetPythonObject(contextObject);
        pCCPass = &cc;
    }
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    if (PyWinObject_AsTCHAR(obClass, &szClass, TRUE) && PyWinObject_AsTCHAR(obWndName, &szWndName, TRUE)) {
        BOOL ok;
        GUI_BGN_SAVE;
        // @pyseemfc CWnd|Create
        ok = pWnd->Create(szClass, szWndName, style, rect, pParent, id, pCCPass);
        GUI_END_SAVE;
        if (!ok)
            PyErr_SetString(ui_module_error, "CWnd::Create");
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    PyWinObject_FreeTCHAR(szClass);
    PyWinObject_FreeTCHAR(szWndName);
    return ret;
}

// @pymethod |PyCWnd|CreateWindowEx|Creates the actual window using extended capabilities.
PyObject *ui_window_create_window_ex(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    TCHAR *szClass = NULL, *szWndName = NULL;
    PyObject *obClass, *obWndName;
    DWORD dwStyleEx;
    PyObject *csObject = Py_None;
    if (!PyArg_ParseTuple(args, "iOOi(iiii)Oi|O:CreateWindowEx",
                          &dwStyleEx,  // @pyparm int|styleEx||The extended style of the window being created.
                          &obClass,    // @pyparm string|classId||The class ID for the window.  May not be None.
                          &obWndName,  // @pyparm string|windowName||The title for the window, or None
                          &style,      // @pyparm int|style||The style for the window.
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom)|rect||The size and position of the window.
                          &obParent,   // @pyparm <o PyCWnd>|parent||The parent window of the new window..
                          &id,         // @pyparm int|id||The control's ID.
                          &csObject))  // @pyparm <o CREATESTRUCT>|createStruct|None|A CreateStruct object (ie, a tuple)
        return NULL;

    CREATESTRUCT cs;
    CREATESTRUCT *pcs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (csObject == Py_None)
        pcs = NULL;
    else {
        if (!CreateStructFromPyObject(&cs, csObject, "CreateEx", FALSE))
            return NULL;
        pcs = &cs;
    }

    CWnd *pParent;
    if (obParent == Py_None)
        pParent = NULL;
    else if (ui_base_class::is_uiobject(obParent, &PyCWnd::type))
        pParent = GetWndPtr(obParent);
    else
        RETURN_TYPE_ERR("parent argument must be a window object, or None");
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    if (!PyWinObject_AsTCHAR(obClass, &szClass, FALSE))
        return NULL;
    if (!PyWinObject_AsTCHAR(obWndName, &szWndName, TRUE)) {
        PyWinObject_FreeTCHAR(szClass);
        return NULL;
    }
    BOOL ok;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|CreateEx
    ok = pWnd->CreateEx(dwStyleEx, szClass, szWndName, style, rect, pParent, id, pcs);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szClass);
    PyWinObject_FreeTCHAR(szWndName);
    if (!ok)
        RETURN_ERR("CWnd::CreateEx");
    RETURN_NONE;
}

// @pymethod <o PyCWnd>|win32ui|CreateWindowFromHandle|Creates a <o PyCWnd> from an integer containing a HWND
PyObject *PyCWnd::CreateWindowFromHandle(PyObject *self, PyObject *args)
{
    PyObject *obhwnd;
    if (!PyArg_ParseTuple(args, "O:CreateWindowFromHandle",
                          &obhwnd))  // @pyparm int|hwnd||The window handle.
        return NULL;
    HWND hwnd;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    CWnd *pWnd = CWnd::FromHandle((HWND)hwnd);
    if (pWnd == NULL)
        RETURN_ERR("The window handle is invalid.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|CreateControl|Creates an OLE control.
PyObject *PyCWnd::CreateControl(PyObject *self, PyObject *args)
{
    PyObject *parent = Py_None;
    int id;
    int style;
    CRect rect(0, 0, 0, 0);
    PyObject *obPersist = Py_None;
    int bStorage = FALSE;
    TCHAR *szClass = NULL, *szWndName = NULL;
    PyObject *obClass, *obWndName;
    PyObject *obLicKey = Py_None;
    if (!PyArg_ParseTuple(args, "OOi(iiii)Oi|OiO:CreateControl",
                          &obClass,    // @pyparm string|classId||The class ID for the window.
                          &obWndName,  // @pyparm string|windowName||The title for the window.
                          &style,      // @pyparm int|style||The style for the control.
                                       // @pyparm (left, top, right, bottom)|rect||The default position of the window.
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          &parent,     // @pyparm <o PyCWnd>|parent||The parent window
                          &id,         // @pyparm int|id||The child ID for the view
                          &obPersist,  // @pyparm object|obPersist|None|Place holder for future support.
                          &bStorage,   // @pyparm int|bStorage|FALSE|Not used.
                          &obLicKey))  // @pyparm string|licKey|None|The license key for the control.
        return NULL;

    if (!PyWinObject_AsTCHAR(obClass, &szClass, FALSE))
        return NULL;
    CLSID clsid;
    HRESULT hr = AfxGetClassIDFromString(szClass, &clsid);
    PyWinObject_FreeTCHAR(szClass);
    if (FAILED(hr))
        RETURN_ERR("The CLSID is invalid");

    CWnd *pWnd = new CWnd;
    if (!ui_base_class::is_uiobject(parent, &PyCWnd::type))
        RETURN_TYPE_ERR("Argument must be a PyCWnd");
    CWnd *pWndParent = GetWndPtr(parent);
    if (pWnd == NULL || pWndParent == NULL)
        return NULL;
    PyWin_AutoFreeBstr bstrLicKey;
    if (obLicKey != Py_None && !PyWinObject_AsAutoFreeBstr(obLicKey, &bstrLicKey, TRUE))
        return NULL;

    // This will cause MFC to die after dumping a message to the debugger!
    if (afxOccManager == NULL)
        RETURN_ERR("win32ui.EnableControlContainer() has not been called yet.");
    if (!PyWinObject_AsTCHAR(obWndName, &szWndName, TRUE))
        return NULL;
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pWnd->CreateControl(clsid, szWndName, style, rect, pWndParent, id, NULL, bStorage, bstrLicKey);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szWndName);
    if (!ok)
        RETURN_ERR("CreateControl failed");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|GetActiveWindow|Retrieves the active window.
PyObject *PyCWnd::GetActiveWindow(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetActiveWindow);
    GUI_BGN_SAVE;
    CWnd *pWnd = CWnd::GetActiveWindow();
    GUI_END_SAVE;
    if (pWnd == NULL)
        RETURN_ERR("No window is active.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|GetForegroundWindow|Retrieves the foreground window.
PyObject *PyCWnd::GetForegroundWindow(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetForegroundWindow);
    GUI_BGN_SAVE;
    CWnd *pWnd = CWnd::GetForegroundWindow();
    GUI_END_SAVE;
    if (pWnd == NULL)
        RETURN_ERR("No window is is in the foreground.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|GetFocus|Retrieves the window with the focus.
PyObject *PyCWnd::GetFocus(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetFocus);
    GUI_BGN_SAVE;
    CWnd *pWnd = CWnd::GetFocus();
    GUI_END_SAVE;
    if (pWnd == NULL)
        RETURN_ERR("No window has focus.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|FindWindow|Searches for the specified top-level window
PyObject *PyCWnd::FindWindow(PyObject *self, PyObject *args)
{
    TCHAR *szClassName = NULL;
    TCHAR *szWndName = NULL;
    PyObject *obClassName, *obWndName;
    if (!PyArg_ParseTuple(args, "OO:FindWindow",
                          &obClassName,  // @pyparm string|className||The window class name to find, else None
                          &obWndName))   // @pyparm string|windowName||The window name (ie, title) to find, else None
        return NULL;
    if (!PyWinObject_AsTCHAR(obClassName, &szClassName, TRUE))
        return NULL;
    if (!PyWinObject_AsTCHAR(obWndName, &szWndName, TRUE)) {
        PyWinObject_FreeTCHAR(szClassName);
        return NULL;
    }
    GUI_BGN_SAVE;
    CWnd *pWnd = CWnd::FindWindow(szClassName, szWndName);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szClassName);
    PyWinObject_FreeTCHAR(szWndName);
    if (pWnd == NULL)
        RETURN_ERR("No window can be found.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod <o PyCWnd>|win32ui|FindWindowEx|Searches for the specified top-level or child window
PyObject *PyCWnd::FindWindowEx(PyObject *self, PyObject *args)
{
    TCHAR *szClassName = NULL;
    TCHAR *szWndName = NULL;
    PyObject *obClassName, *obWndName, *ret = NULL;
    PyObject *obParent;
    PyObject *obChildAfter;
    if (!PyArg_ParseTuple(args, "OOOO:FindWindowEx",
                          &obParent,  // @pyparm <o PyCWnd>|parentWindow||The parent whose children will be searched. If
                                      // None, the desktops window will be used.
                          &obChildAfter,  // @pyparm <o PyCWnd>|childAfter||The search begins with the next window in
                                          // the Z order.  If None, all children are searched.
                          &obClassName,   // @pyparm string|className||The window class name to find, else None
                          &obWndName))    // @pyparm string|windowName||The window name (ie, title) to find, else None
        return NULL;
    CWnd *pParent = NULL;
    if (obParent != Py_None)
        if ((pParent = GetWndPtrFromParam(obParent, PyCWnd::type)) == NULL)
            return NULL;
    CWnd *pChildAfter = NULL;
    if (obChildAfter != Py_None)
        if ((pChildAfter = GetWndPtrFromParam(obChildAfter, PyCWnd::type)) == NULL)
            return NULL;
    if (PyWinObject_AsTCHAR(obClassName, &szClassName, TRUE) && PyWinObject_AsTCHAR(obWndName, &szWndName, TRUE)) {
        GUI_BGN_SAVE;
        HWND hwnd = ::FindWindowEx(pParent->GetSafeHwnd(), pChildAfter->GetSafeHwnd(), szClassName, szWndName);
        GUI_END_SAVE;
        if (hwnd == NULL)
            PyErr_SetString(ui_module_error, "FindWindowEx: No window can be found.");
        else
            ret = PyCWnd::make(PyCWnd::type, NULL, hwnd)->GetGoodRet();
    }
    PyWinObject_FreeTCHAR(szClassName);
    PyWinObject_FreeTCHAR(szWndName);
    return ret;
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

/////////////////////////////////////////////////////////////////////
//
// Window object
//
//////////////////////////////////////////////////////////////////////
PyCWnd::PyCWnd()
{
    pMessageHookList = NULL;
    pKeyHookList = NULL;
    obKeyStrokeHandler = NULL;
    bDidSubclass = FALSE;
}
PyCWnd::~PyCWnd()
{
    free_hook_list(this, &pMessageHookList);
    free_hook_list(this, &pKeyHookList);
    Py_XDECREF(obKeyStrokeHandler);
    obKeyStrokeHandler = NULL;
    if (bManualDelete || bDidSubclass) {
        // Can't use GetWndPtr(this) as ob_type has been nuked.
        CWnd *pWnd = (CWnd *)this->assoc;  // get pointer before killing it.
        if (pWnd) {
            if (bDidSubclass) {
                pWnd->UnsubclassWindow();
                bDidSubclass = FALSE;
            }
            if (bManualDelete) {
                // Release the lock while we destroy the object.
                GUI_BGN_SAVE;
                delete pWnd;
                GUI_END_SAVE;
                bManualDelete = FALSE;
            }
        }
    }
}

BOOL PyCWnd::check_key_stroke(WPARAM ch)
{
    PyObject *pythonObject;
    BOOL bCallBase = TRUE;
    if (obKeyStrokeHandler != NULL)
        bCallBase = Python_callback(obKeyStrokeHandler, ch);

    if (bCallBase && pKeyHookList && pKeyHookList->Lookup((WORD)ch, (void *&)pythonObject))
        bCallBase = Python_callback(pythonObject, ch);
    return bCallBase;
}

CWnd *PyCWnd::GetPythonGenericWnd(PyObject *self, ui_type_CObject *pType)
{
    // Damn it - only pass PyCWnd::type so the RTTI check won't fail
    // for builtin controls.
    return (CWnd *)GetGoodCppObject(self, &type);
}

/*static*/ PyCWnd *PyCWnd::make(ui_type_CObject &makeType, CWnd *pSearch, HWND wnd /*=NULL*/)
{
    BOOL bManualDelete = FALSE;
    BOOL bDidSubclass = FALSE;
    ASSERT(pSearch || wnd);
    if (pSearch)
        wnd = pSearch->GetSafeHwnd();
    // must have a permanent object for this window.
    BOOL bMadeNew = FALSE;
    pSearch = CWnd::FromHandlePermanent(wnd);
    // FromHandlePerm is thread specific!
    if (pSearch == NULL && GetWindowLongPtr(wnd, GWLP_WNDPROC) == (LONG_PTR)AfxGetAfxWndProc()) {
        /*******
            Windows are per thread.  This gross hack lets me get windows across
            threads, but there must be a good reason for the restriction, and this
            hack only works with the main thread, rather than any thread.

                #include "D:\Program Files\DevStudio\VC\mfc\src\WINHAND_.H"
                extern AFX_MODULE_THREAD_STATE * PyWin_MainModuleThreadState;

                // Let's see if it is in the main thread state
                if (PyWin_MainModuleThreadState->m_pmapHWND &&
                    (AfxGetModuleThreadState() != PyWin_MainModuleThreadState)) {
                    // Gross hack - look it up in the internal map structure.
                    pSearch = (CWnd*)(PyWin_MainModuleThreadState->m_pmapHWND->LookupPermanent(wnd));
                    ASSERT(pSearch == NULL || pSearch->m_hWnd == wnd);
                }

                if (pSearch==NULL)
        *****/
        RETURN_ERR("The window was created in a different thread and can not be mapped.");
    }
    if (pSearch == NULL) {
        if (!IsWindow(wnd))
            RETURN_ERR("The window can not be created as it has an invalid handle");
        CWnd *pWnd;
        ASSERT(makeType.pCObjectClass);  // we want to know the exact type to create!
        if (makeType.pCObjectClass && makeType.pCObjectClass->m_pfnCreateObject) {
            pWnd = (CWnd *)makeType.pCObjectClass->CreateObject();
            if (pWnd == NULL) {
                PyErr_SetString(PyExc_MemoryError, "Can't create the window object");
                return NULL;
            }
            ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CWnd)));  // Must be a window object we just created!
        }
        else
            pWnd = new CWnd();  // this will except, rather than return NULL!

        //		pWnd->Attach(wnd);
        pWnd->SubclassWindow(wnd);
        pSearch = pWnd;  // this is now the object we use, and will get in the future from GetWindow()
        bManualDelete = bDidSubclass = bMadeNew = TRUE;
    }
    PyCWnd *obj = (PyCWnd *)ui_assoc_object::make(makeType, pSearch);
    if (obj && bMadeNew) {
        obj->bManualDelete = bManualDelete;
        obj->bDidSubclass = bDidSubclass;
    }
    return obj;
}
///////////////////////////////////
// Python methods
//

// @pymethod |PyCWnd|ActivateFrame|Searches upwards for a parent window which has
// a frame, and activates it.
static PyObject *ui_window_activate_frame(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    int cmdShow = SW_SHOW;
    if (!PyArg_ParseTuple(args, "|i:ActivateFrame", &cmdShow))  // @pyparm int|cmdShow|SW_SHOW|
        // The param passed to <mf CFrameWnd::ShowWindow>.  See also <om PyCWnd.ShowWindow>.
        return NULL;
    while (pWnd && !pWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd))) pWnd = pWnd->GetParent();
    if (!pWnd)
        RETURN_ERR("The specified window does not have a parent frame window");
    GUI_BGN_SAVE;
    ((CFrameWnd *)pWnd)->ActivateFrame(cmdShow);  // @pyseemfc CFrameWnd|ActivateFrame

    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|BringWindowToTop|Brings the window to the top of a stack of overlapping windows.
static PyObject *ui_window_bring_window_to_top(PyObject *self, PyObject *args)
{
    // @comm This method activates pop-up, top-level, and MDI child windows.
    // The BringWindowToTop member function should be used to uncover any window that is partially or
    // completely obscured by any overlapping windows.<nl>
    // Calling this method is similar to calling the <om PyCWnd.SetWindowPos> method to
    // change a window's position in the Z order. The BringWindowToTop method
    // does not change the window style to make it a top-level window of the desktop.
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->BringWindowToTop();  // @pyseemfc CWnd|BringWindowToTop
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod (left, top, right, bottom)|PyCWnd|CalcWindowRect|
// Computes the size of the window rectangle based on the desired client
// rectangle size.  The resulting size can then be used as the initial
// size for the window object.
static PyObject *ui_window_calc_window_rect(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CRect rect;
    UINT nAdjustType = CWnd::adjustBorder;
    // @pyparm (left, top, right, bottom)|rect||The size to calculate from
    // @pyparm int|nAdjustType|adjustBorder|An enumerated type used for in-place editing. It can have the following
    // values: CWnd::adjustBorder = 0, which means that scrollbar sizes are ignored in calculation; and
    // CWnd::adjustOutside = 1, which means that they are added into the final measurements of the rectangle.
    if (!PyArg_ParseTuple(args, "(iiii)|i:CalcWindowRect", &rect.left, &rect.top, &rect.right, &rect.bottom,
                          &nAdjustType))
        return NULL;
    GUI_BGN_SAVE;
    pWnd->CalcWindowRect(&rect, nAdjustType);  // @pyseemfc CWnd|CalcWindowRect
    GUI_END_SAVE;
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}
// @pymethod |PyCWnd|CheckRadioButton|Selects the specified radio button, and clears
// all others in the group.
static PyObject *ui_window_check_radio_button(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int idFirst, idLast, idCheck;
    if (!PyArg_ParseTuple(args, "iii:CheckRadioButton",
                          &idFirst,   // @pyparm int|idFirst||The identifier of the first radio button in the group.
                          &idLast,    // @pyparm int|idLast||The identifier of the last radio button in the group.
                          &idCheck))  // @pyparm int|idCheck||The identifier of the radio button to be checked.
        return NULL;
    GUI_BGN_SAVE;
    pWnd->CheckRadioButton(idFirst, idLast, idCheck);  // @pyseemfc CWnd|CheckRadioButton
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod PyCWnd|PyCWnd|ChildWindowFromPoint|Returns the child window that contains the point
static PyObject *ui_child_window_from_point(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CPoint pnt;
    int flag = 0;
    if (!PyArg_ParseTuple(args, "(ii)|i:ChildWindowFromPoint",
                          &pnt.x,  // @pyparm int|x||x coordinate of point
                          &pnt.y,  // @pyparm int|y||y coordinate of point
                          &flag))  // @pyparm int|flag|0|Specifies which child windows to skip
        return NULL;
    GUI_BGN_SAVE;
    CWnd *pChildWnd = pWnd->ChildWindowFromPoint(pnt, flag);  // @pyseemfc CWnd|ChildWindowFromPoint
    GUI_END_SAVE;
    return PyCWnd::make(UITypeFromCObject(pChildWnd), pChildWnd)->GetGoodRet();
}

// @pymethod int|PyCWnd|DefWindowProc|Calls the default message handler.
static PyObject *ui_window_def_window_proc(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int message;
    PyObject *obwparam, *oblparam;
    if (!PyArg_ParseTuple(args, "iOO:DefWindowProc",
                          &message,    // @pyparm int|message||The Windows message.
                          &obwparam,   // @pyparm int|idLast||The lParam for the message.
                          &oblparam))  // @pyparm int|idCheck||The wParam for the message.
        return NULL;
    PyWin_PARAMHolder wparam;
    PyWin_PARAMHolder lparam;
    if (!PyWinObject_AsPARAM(obwparam, &wparam) || !PyWinObject_AsPARAM(oblparam, &lparam))
        return NULL;
    GUI_BGN_SAVE;
    LRESULT rc = ((WndHack *)pWnd)->DefWindowProc(message, wparam, lparam);
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(rc);  // @pyseemfc CWnd|DefWindowProc
}

// @pymethod |PyCWnd|DlgDirList|Fill a list box with a file or directory listing.
static PyObject *ui_window_dlg_dir_list(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    TCHAR *defPath;
    PyObject *obdefPath;
    int nIDListBox, nIDStaticPath, nFileType;
    if (!PyArg_ParseTuple(
            args, "Oiii:DlgDirList",
            &obdefPath,      // @pyparm string|defPath||The file spec to fill the list box with
            &nIDListBox,     // @pyparm int|idListbox||The Id of the listbox control to fill.
            &nIDStaticPath,  // @pyparm int|idStaticPath||The Id of the static control used to display the current drive
                             // and directory. If idStaticPath is 0, it is assumed that no such control exists.
            &nFileType))     // @pyparm int|fileType||Specifies the attributes of the files to be displayed.
                             // It can be any combination of DDL_READWRITE, DDL_READONLY, DDL_HIDDEN, DDL_SYSTEM,
                             // DDL_DIRECTORY, DDL_ARCHIVE, DDL_POSTMSGS, DDL_DRIVES or DDL_EXCLUSIVE
        return NULL;
    if (!PyWinObject_AsTCHAR(obdefPath, &defPath, FALSE))
        return NULL;
    TCHAR pathBuf[MAX_PATH + 1];
    _tcsncpy(pathBuf, defPath, MAX_PATH);
    pathBuf[MAX_PATH] = '\0';
    int rc;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|DlgDirList
    rc = pWnd->DlgDirList(pathBuf, nIDListBox, nIDStaticPath, nFileType);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(defPath);
    if (!rc)
        RETURN_ERR("DlgDirList failed");
    RETURN_NONE;
}
// @pymethod |PyCWnd|DlgDirListComboBox|Fill a combo with a file or directory listing.  See <om PyCWnd.DlgDirList> for
// details.
static PyObject *ui_window_dlg_dir_list_combo(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    TCHAR *defPath;
    PyObject *obdefPath;
    int nIDListBox, nIDStaticPath, nFileType;
    if (!PyArg_ParseTuple(args, "Oiii:DlgDirListComboBox", &obdefPath, &nIDListBox, &nIDStaticPath, &nFileType))
        return NULL;
    if (!PyWinObject_AsTCHAR(obdefPath, &defPath, FALSE))
        return NULL;
    TCHAR pathBuf[MAX_PATH + 1];
    _tcsncpy(pathBuf, defPath, MAX_PATH);
    pathBuf[MAX_PATH] = '\0';
    int rc;
    GUI_BGN_SAVE;
    rc = pWnd->DlgDirListComboBox(pathBuf, nIDListBox, nIDStaticPath, nFileType);
    // @pyseemfc CWnd|DlgDirListComboBox

    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(defPath);
    if (!rc)
        RETURN_ERR("DlgDirListComboBox failed");
    RETURN_NONE;
}
// @pymethod string|PyCWnd|DlgDirSelect|
// Retrieves the current selection from a list box. It assumes that the list box has been filled by the <om
// PyCWnd.DlgDirList> member function and that the selection is a drive letter, a file, or a directory name.
static PyObject *ui_window_dlg_dir_select(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    int nIDListBox;
    // @pyparm int|idListbox||The Id of the listbox.
    if (!PyArg_ParseTuple(args, "i:DlgDirSelect", &nIDListBox))
        return NULL;
    TCHAR buf[MAX_PATH];
    GUI_BGN_SAVE;
    int rc = pWnd->DlgDirSelect(buf, sizeof(buf) / sizeof(TCHAR), nIDListBox);
    // @pyseemfc CWnd|DlgDirSelect
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("DlgDirSelect failed");
    return PyWinObject_FromTCHAR(buf);
}

// @pymethod string|PyCWnd|DlgDirSelectComboBox|
// Retrieves the current selection from the list box of a combo box. It assumes that the list box has been filled by the
// <om PyCWnd.DlgDirListComboBox> member function and that the selection is a drive letter, a file, or a directory name.
static PyObject *ui_window_dlg_dir_select_combo(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    int nIDListBox;
    // @pyparm int|idListbox||The Id of the combobox.
    if (!PyArg_ParseTuple(args, "i:DlgDirSelectComboBox", &nIDListBox))
        return NULL;
    TCHAR buf[MAX_PATH];
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|DlgDirSelectComboBox
    int rc = pWnd->DlgDirSelectComboBox(buf, sizeof(buf) / sizeof(TCHAR), nIDListBox);
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("DlgDirSelectComboBox failed");
    return PyWinObject_FromTCHAR(buf);
}

// @pymethod |PyCWnd|DragAcceptFiles|Indicates that the window and children supports files dropped from file manager
static PyObject *ui_window_drag_accept_files(PyObject *self, PyObject *args)
{
    BOOL accept = TRUE;
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    // @pyparm int|bAccept|1|A flag indicating if files are accepted.
    if (!PyArg_ParseTuple(args, "|i:DragAcceptFiles", &accept))
        return NULL;
    GUI_BGN_SAVE;
    pWnd->DragAcceptFiles(accept);
    // @pyseemfc CWnd|DragAcceptFiles

    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCWnd|DestroyWindow|Destroy the window attached to the object.
static PyObject *ui_window_destroy_window(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    BOOL rc;
    GUI_BGN_SAVE;
    rc = pWnd->DestroyWindow();
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("DestroyWindow could not destroy the window");
    RETURN_NONE;
    // @comm The DestroyWindow member function sends appropriate messages
    // to the window to deactivate it and remove the input focus.
    // It also destroys the window's menu, flushes the application queue,
    // destroys outstanding timers, removes Clipboard ownership, and breaks the
    // Clipboard-viewer chain if CWnd is at the top of the viewer chain.
    // It sends WM_DESTROY and WM_NCDESTROY messages to the window.
}

// @pymethod |PyCWnd|DrawMenuBar|Redraws the menu bar.  Can be called if the menu changes.
static PyObject *ui_window_draw_menu_bar(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->DrawMenuBar();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCWnd|EnableWindow|Enables or disables the window.  Typically used for dialog controls.
static PyObject *ui_window_enable_window(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    BOOL bEnable = TRUE;
    // @pyparm int|bEnable|1|A flag indicating if the window is to be enabled or disabled.
    if (!PyArg_ParseTuple(args, "|i:EnableWindow", &bEnable))
        return NULL;
    int rc;
    GUI_BGN_SAVE;
    rc = pWnd->EnableWindow(bEnable);
    // @pyseemfc CWnd|EnableWindow
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
    // @rdesc Returns the state before the EnableWindow member function was called
}

// @pymethod int|PyCWnd|GetCheckedRadioButton|Returns the ID of the checked radio button, or 0 if none is selected.
static PyObject *ui_window_get_checked_rb(PyObject *self, PyObject *args)
{
    int idFirst, idLast;
    if (!PyArg_ParseTuple(args, "ii:GetCheckedRadioButton",
                          &idFirst,  // @pyparm int|idFirst||The Id of the first radio button in the group.
                          &idLast))  // @pyparm int|idLast||The Id of the last radio button in the group.
        return NULL;
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetCheckedRadioButton

    GUI_BGN_SAVE;
    int rc = pWnd->GetCheckedRadioButton(idFirst, idLast);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod (left, top, right, bottom)|PyCWnd|GetClientRect|Returns the client coordinates of the window.  left and top
// will be zero.
static PyObject *ui_window_get_client_rect(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    CRect rect;
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->GetClientRect(&rect);
    GUI_END_SAVE;
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod |PyCWnd|SetDlgItemText|Sets the text for the child window or control with the specified ID.
static PyObject *ui_window_set_dlg_item_text(PyObject *self, PyObject *args)
{
    int id;
    TCHAR *szText;
    PyObject *obText;
    // @pyparm int|idControl||The Id of the control
    // @pyparm string|text||The new text
    if (!PyArg_ParseTuple(args, "iO:SetDlgItemText", &id, &obText))
        return NULL;
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    if (!PyWinObject_AsTCHAR(obText, &szText, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pWnd->SetDlgItemText(id, szText);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szText);
    RETURN_NONE;
    // @pyseemfc CWnd|SetDlgItemText
}

// @pymethod <o PyCWnd>|PyCWnd|GetDlgItem|Returns a window object for the child window or control with the specified ID.
// The type of the return object will be as specific as possible, but will always
// be derived from an <o PyCWnd> object.
static PyObject *ui_window_get_dlg_item(PyObject *self, PyObject *args)
{
    int id;
    if (!PyArg_ParseTuple(args, "i:GetDlgItem", &id))  // @pyparm int|idControl||The Id of the control to be retrieved.
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pChild = pWnd->GetDlgItem(id);
    GUI_END_SAVE;
    // @pyseemfc CWnd|GetDlgItem

    if (!pChild)
        RETURN_ERR("No dialog control with that ID");
    return PyCWnd::make(UITypeFromCObject(pChild), pChild)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> (or derived) object, or a win32ui.error exception is raised.
}

// @pymethod string|PyCWnd|GetDlgItemText|Returns the text of child window or control with the specified ID.
static PyObject *ui_window_get_dlg_item_text(PyObject *self, PyObject *args)
{
    int id;
    if (!PyArg_ParseTuple(args, "i:GetDlgItemText",
                          &id))  // @pyparm int|idControl||The Id of the control to be retrieved.
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CString csRet;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    pWnd->GetDlgItemText(id, csRet);
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(csRet);
    // @pyseemfc CWnd|GetDlgItemText
}

// @pymethod int|PyCWnd|GetDlgItemInt|Returns the integer value of a child window or control with the specified ID.
static PyObject *ui_window_get_dlg_item_int(PyObject *self, PyObject *args)
{
    int id;
    BOOL bUnsigned = TRUE;
    // @pyparm int|idControl||The Id of the control to be retrieved.
    // @pyparm int|bUnsigned|1|Should the function check for a minus sign
    if (!PyArg_ParseTuple(args, "i|i:GetDlgItemInt", &id, &bUnsigned))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CHECK_HWND_VALID(pWnd);
    BOOL bWorked;
    GUI_BGN_SAVE;
    int res = (int)pWnd->GetDlgItemInt(id, &bWorked, bUnsigned);
    GUI_END_SAVE;
    if (!bWorked)
        RETURN_VALUE_ERR("The dialog item could not be converted to an integer");
    // @rdesc If the value can not be converted, a ValueError is raised.
    return PyLong_FromLong(res);
    // @pyseemfc CWnd|GetDlgItemInt
}

// @pymethod int|PyCWnd|GetDlgCtrlID|Returns the ID of this child window.
static PyObject *ui_window_get_dlg_ctrl_id(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetDlgCtrlId
    GUI_BGN_SAVE;
    int rc = pWnd->GetDlgCtrlID();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod <o PyCMenu>|PyCWnd|GetMenu|Returns the menu object for the window's menu.
static PyObject *ui_window_get_menu(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetMenu
    GUI_BGN_SAVE;
    HMENU hMenu = ::GetMenu(pWnd->m_hWnd);
    GUI_END_SAVE;
    if (hMenu == NULL)
        RETURN_ERR("The window has no menu");
    return ui_assoc_object::make(PyCMenu::type, hMenu)->GetGoodRet();
    // @rdesc The result is a <o PyMenu> object, or an exception is thrown.
}
// @pymethod <o PyCWnd>|PyCWnd|GetParent|Returns the window's parent.
static PyObject *ui_window_get_parent(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetParent
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetParent();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;

    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetParentFrame|Returns the window's frame.
static PyObject *ui_window_get_parent_frame(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetParentFrame
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetParentFrame();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;

    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetParentOwner|Returns the child window's parent window or owner window.
static PyObject *ui_window_get_parent_owner(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetParentOwner
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetParentOwner();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;

    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetLastActivePopup|Returns the last active popup Window, or the Window itself.
static PyObject *ui_window_get_last_active_popup(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetLastActivePopup
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetLastActivePopup();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetTopLevelParent|Returns the top-level parent of the window.
static PyObject *ui_window_get_top_level_parent(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetTopLevelParent
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetTopLevelParent();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetTopLevelFrame|Returns the top-level frame of the window.
static PyObject *ui_window_get_top_level_frame(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetTopLevelFrame
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetTopLevelFrame();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod <o PyCWnd>|PyCWnd|GetTopLevelOwner|Returns the top-level owner of the window.
static PyObject *ui_window_get_top_level_owner(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetTopLevelOwner
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    CWnd *pParent = pWnd->GetTopLevelOwner();
    GUI_END_SAVE;
    if (!pParent)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pParent), pParent)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> object, or None if no Window can be found.
}

// @pymethod int|PyCWnd|GetSafeHwnd|Returns the HWnd of this window.
static PyObject *ui_window_get_safe_hwnd(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetSafeHwnd
    GUI_BGN_SAVE;
    HWND hwnd = pWnd->GetSafeHwnd();
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(hwnd);
}

// @pymethod <o SCROLLINFO tuple>|PyCWnd|GetScrollInfo|Returns information about a scroll bar
static PyObject *ui_window_get_scroll_info(PyObject *self, PyObject *args)
{
    int nBar;
    UINT nMask = SIF_ALL;
    // @pyparm int|nBar||The scroll bar to examine.  Can be one of win32con.SB_BOTH, win32con.SB_VERT or
    // win32con.SB_HORZ
    // @pyparm int|mask|SIF_ALL|The mask for attributes to retrieve.
    if (!PyArg_ParseTuple(args, "i|i:GetScrollInfo", &nBar, &nMask))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    SCROLLINFO info;
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = nMask;  // Is this necessary?
    GUI_BGN_SAVE;
    BOOL ok = pWnd->GetScrollInfo(nBar, &info, nMask);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetScrollInfo failed");
    return MakeSCROLLINFOTuple(&info);
}

// @pymethod int|PyCWnd|GetScrollPos|Retrieves the current position of the scroll box of a scroll bar.
static PyObject *ui_window_get_scroll_pos(PyObject *self, PyObject *args)
{
    int nBar;
    // @pyparm int|nBar||The scroll bar to examine.  Can be one of win32con.SB_VERT or win32con.SB_HORZ
    if (!PyArg_ParseTuple(args, "i:GetScrollPos", &nBar))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    long pos = pWnd->GetScrollPos(nBar);
    GUI_END_SAVE;
    return PyLong_FromLong(pos);
}

// @pymethod int|PyCWnd|GetStyle|Retrieves the window style
static PyObject *ui_window_get_style(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetStyle"))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    DWORD ret = pWnd->GetStyle();
    GUI_END_SAVE;
    return PyLong_FromLong(ret);
}

// @pymethod int|PyCWnd|GetExStyle|Retrieves the window's extended style
static PyObject *ui_window_get_ex_style(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetExStyle"))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    DWORD ret = pWnd->GetExStyle();
    GUI_END_SAVE;
    return PyLong_FromLong(ret);
}

// @pymethod <o PyCMenu>|PyCWnd|GetSystemMenu|Returns the menu object for the window's system menu.
static PyObject *ui_window_get_system_menu(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    // @pyseemfc CWnd|GetSystemMenu
    GUI_BGN_SAVE;
    HMENU hMenu = ::GetSystemMenu(pWnd->m_hWnd, FALSE);
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCMenu::type, hMenu)->GetGoodRet();
}
// @pymethod <o PyCWnd>|PyCWnd|GetTopWindow|Identifies the top-level child window in a linked list of child windows.
PyObject *PyCWnd::get_top_window(PyObject *self, PyObject *args)
{
    // @comm Searches for the top-level child window that belongs to this window. If this window has no children, this
    // function returns None
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    CWnd *pRel = pWnd->GetTopWindow();
    GUI_END_SAVE;
    if (!pRel)
        RETURN_NONE;
    // @pyseemfc CWnd|GetTopWindow
    return PyCWnd::make(UITypeFromCObject(pRel), pRel)->GetGoodRet();
    // @rdesc If no child windows exist, the value is None.
}

// @pymethod <o PyCWnd>|PyCWnd|GetWindow|Returns a window, with the specified relationship to this window.
PyObject *PyCWnd::get_window(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    int type;
    // @pyparm int|type||
    // Specifies the relationship between the current and the returned window. It can take one of the following values:
    // GW_CHILD, GW_HWNDFIRST, GW_HWNDLAST, GW_HWNDNEXT, GW_HWNDPREV or GW_OWNER
    if (!PyArg_ParseTuple(args, "i:GetWindow", &type))
        return NULL;
    // @pyseemfc CWnd|GetWindow
    GUI_BGN_SAVE;
    CWnd *pRel = pWnd->GetWindow(type);
    GUI_END_SAVE;
    if (!pRel)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pRel), pRel)->GetGoodRet();
    // @rdesc The result is a <o PyCWnd> or None if no Window can be found.
}

// @pymethod tuple|PyCWnd|GetWindowPlacement|Returns placement information about the current window.
static PyObject *ui_window_get_window_placement(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    WINDOWPLACEMENT pment;
    pment.length = sizeof(pment);
    // @pyseemfc CWnd|GetWindowPlacement
    GUI_BGN_SAVE;
    BOOL bsuccess = pWnd->GetWindowPlacement(&pment);
    GUI_END_SAVE;
    if (!bsuccess)
        return NULL;
    // @rdesc The result is a tuple of
    // (flags, showCmd, (minposX, minposY), (maxposX, maxposY), (normalposX, normalposY))
    // @flagh Item|Description
    // @flag flags|One of the WPF_* constants
    // @flag showCmd|Current state - one of the SW_* constants.
    // @flag minpos|Specifies the coordinates of the window's upper-left corner when the window is minimized.
    // @flag maxpos|Specifies the coordinates of the window's upper-left corner when the window is maximized.
    // @flag normalpos|Specifies the window's coordinates when the window is in the restored position.
    return Py_BuildValue("(ii(ii)(ii)(iiii))", pment.flags, pment.showCmd, pment.ptMinPosition.x, pment.ptMinPosition.y,
                         pment.ptMaxPosition.x, pment.ptMaxPosition.y, pment.rcNormalPosition.left,
                         pment.rcNormalPosition.top, pment.rcNormalPosition.right, pment.rcNormalPosition.bottom);
}

// @pymethod (left, top, right, bottom)|PyCWnd|GetWindowRect|Returns the screen coordinates of the windows upper left
// corner
static PyObject *ui_window_get_window_rect(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    CRect rect;
    GUI_BGN_SAVE;
    pWnd->GetWindowRect(&rect);
    GUI_END_SAVE;
    // @pyseemfc CWnd|GetWindowRect
    return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
}
// @pymethod string|PyCWnd|GetWindowText|Returns the windows text.
static PyObject *ui_window_get_window_text(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    CString csText;
    // @pyseemfc CWnd|Py_BuildValue
    GUI_BGN_SAVE;
    pWnd->GetWindowText(csText);
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(csText);
}
// @pymethod object|PyCWnd|HookKeyStroke|Hook a key stroke handler
static PyObject *ui_window_hook_key_stroke(PyObject *self, PyObject *args)
{
    // @comm The handler object passed will be called as the application receives WM_CHAR message for the specified
    // character code. The handler will be called with 2 arguments<nl>
    // * The handler object (as per all hook functions)<nl>
    // * The keystroke being handled.<nl>
    // If the handler returns TRUE, then the keystroke will be passed on to the
    // default handler, otherwise the keystroke will be consumed.<nl>
    // Note: This handler will not be called if a <om PyCWnd.HookAllKeyStrokes> hook is in place.

    // @pyparm object|obHandler||The handler of the keystroke.  This must be a callable object.
    // @pyparm int|ch||The ID for the keystroke to be handled.
    // This may be an ascii code, or a virtual key code.
    // @rdesc The return value is the previous handler, or None.
    PyCWnd *s = (PyCWnd *)self;
    return add_hook_list(s, args, &s->pKeyHookList);
}

// @pymethod |PyCWnd|HookAllKeyStrokes|Hook a key stroke handler for all key strokes.
static PyObject *ui_window_hook_all_key_strokes(PyObject *self, PyObject *args)
{
    // @comm The handler object passed will be called as the application receives WM_CHAR messages.
    // The handler will be called with 2 arguments<nl>
    // * The handler object (as per all hook functions).<nl>
    // * The keystroke being handled.<nl>
    // If the handler returns TRUE, then the keystroke will be passed on to the
    // default handler, otherwise it will be consumed.<nl>
    // Note: This handler will prevent any <om PyCWnd.HookKeyStroke> hooks from being called.
    PyCWnd *s = (PyCWnd *)self;
    PyObject *obHandler;

    // @pyparm object|obHandler||The handler for the keystrokes.  This must be a callable object.
    if (!PyArg_ParseTuple(args, "O:HookAllKeyStrokes", &obHandler))
        return NULL;
    if (!PyCallable_Check(obHandler))
        RETURN_ERR("The parameter must be a callable object");
    Py_XDECREF(s->obKeyStrokeHandler);
    s->obKeyStrokeHandler = obHandler;
    Py_INCREF(s->obKeyStrokeHandler);
    RETURN_NONE;
}

// @pymethod object|PyCWnd|HookMessage|Hook a message notification handler
static PyObject *ui_window_hook_message(PyObject *self, PyObject *args)
{
    // @comm The handler object passed will be called as the application receives messages with the specified ID.
    // Note that it is not possible for PythonWin to consume a message - it is always passed on to the default handler.
    // The handler will be called with 2 arguments<nl>
    // * The handler object (as per all hook functions).<nl>
    // * A tuple representing the message.<nl>
    // The message tuple is in the following format:
    // @tupleitem 0|int|hwnd|The hwnd of the window.
    // @tupleitem 1|int|message|The message.
    // @tupleitem 2|int|wParam|The wParam sent with the message.
    // @tupleitem 3|int|lParam|The lParam sent with the message.
    // @tupleitem 4|int|time|The time the message was posted.
    // @tupleitem 5|int, int|point|The point where the mouse was when the message was posted.

    // @pyparm object|obHandler||The handler for the message notification.  This must be a callable object.
    // @pyparm int|message||The ID of the message to be handled.
    // @rdesc The return value is the previous handler, or None.
    PyCWnd *s = (PyCWnd *)self;
    return add_hook_list(s, args, &s->pMessageHookList);
}
// @pymethod int|PyCWnd|IsChild|Determines if a given window is a child of this window.
PyObject *ui_window_is_child(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    PyObject *ob;
    // @pyparm <o PyCWnd>|obWnd||The window to be checked
    if (!PyArg_ParseTuple(args, "O:IsChild", &ob))
        return NULL;
    CWnd *pTest = GetWndPtrFromParam(ob, PyCWnd::type);
    if (pTest == NULL)
        return NULL;
    // @pyseemfc CWnd|IsChild
    GUI_BGN_SAVE;
    int isChild = pWnd->IsChild(pTest);
    GUI_END_SAVE;
    return Py_BuildValue("i", isChild);
}

// @pymethod int|PyCWnd|IsDlgButtonChecked|Determines if a dialog button is checked.
PyObject *ui_window_is_dlg_button_checked(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int id;
    // @pyparm int|idCtl||The ID of the button to check.
    if (!PyArg_ParseTuple(args, "i:IsDlgButtonChecked", &id))
        return NULL;
    // @pyseemfc CWnd|IsDlgButtonChecked
    GUI_BGN_SAVE;
    int rc = pWnd->IsDlgButtonChecked(id);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod int|PyCWnd|IsIconic|Determines if the window is currently displayed as an icon.
static PyObject *ui_window_is_iconic(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, IsIconic);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pWnd->IsIconic();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}
// @pymethod int|PyCWnd|IsZoomed|Determines if the window is currently maximised.
static PyObject *ui_window_is_zoomed(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, IsZoomed);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pWnd->IsZoomed();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod int|PyCWnd|IsWindowVisible|Determines if the window is currently visible.
static PyObject *ui_window_is_window_visible(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, IsWindowVisible);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    long rc = pWnd->IsWindowVisible();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod int|PyCWnd|IsWindowEnabled|Determines if the window is currently enabled.
static PyObject *ui_window_is_window_enabled(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, IsWindowEnabled);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pWnd->IsWindowEnabled();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod |PyCWnd|MessageBox|Display a message box.
static PyObject *ui_window_message_box(PyObject *self, PyObject *args)
{
    TCHAR *message, *title = NULL;
    PyObject *obmessage, *obtitle = Py_None;
    long style = MB_OK;

    if (!PyArg_ParseTuple(args, "O|Ol:MessageBox",
                          &obmessage,  // @pyparm string|message||The message to be displayed in the message box.
                          &obtitle,    // @pyparm string/None|title|None|The title for the message box.  If None, the
                                       // applications title will be used.
                          &style))     // @pyparm int|style|win32con.MB_OK|The style of the message box.
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    if (!PyWinObject_AsTCHAR(obmessage, &message, FALSE))
        return NULL;
    if (!PyWinObject_AsTCHAR(obtitle, &title, TRUE)) {
        PyWinObject_FreeTCHAR(message);
        return NULL;
    }
    int rc;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|MessageBox

    rc = pWnd->MessageBox(message, title, style);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(message);
    PyWinObject_FreeTCHAR(title);
    return Py_BuildValue("i", rc);
    // @rdesc An integer identifying the button pressed to dismiss the dialog.
}

// @pymethod int|PyCWnd|ModifyStyle|Modifies the style of a window.
// @rdesc The result is true if the style was changed, or false if the style
// is already the same as requested and no change was made.
static PyObject *ui_window_modify_style(PyObject *self, PyObject *args)
{
    unsigned flags = 0;
    int add;
    int remove;
    if (!PyArg_ParseTuple(
            args, "ii|i:ModifyStyle",
            &remove,  // @pyparm int|remove||Specifies window styles to be removed during style modification.
            &add,     // @pyparm int|add||Specifies window styles to be added during style modification.
            &flags))  // @pyparm int|flags|0|Flags to be passed to SetWindowPos, or zero if SetWindowPos should not be
                      // called. The default is zero.
        return NULL;

    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    BOOL rc;
    GUI_BGN_SAVE;
    rc = pWnd->ModifyStyle(remove, add, flags);
    // @pyseemfc CWnd|ModifyStyle
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
    // @comm If nFlags is nonzero, ModifyStyle calls the Windows API function ::SetWindowPos and redraws the window by
    // combining nFlags with the following four preset flags: <nl>* SWP_NOSIZE	Retains the current size. <nl>*
    // SWP_NOMOVE	Retains the current position. <nl>* SWP_NOZORDER	Retains the current Z order. <nl>*
    // SWP_NOACTIVATE	Does not activate the window. <nl>See also <om PyCWnd.ModifyStyleEx>
}

// @pymethod int|PyCWnd|ModifyStyleEx|Modifies the extended style of a window.
// @rdesc The result is true if the style was changed, or false if the style
// is already the same as requested and no change was made.
static PyObject *ui_window_modify_style_ex(PyObject *self, PyObject *args)
{
    unsigned flags = 0;
    int add;
    int remove;
    if (!PyArg_ParseTuple(
            args, "ii|i:ModifyStyleEx",
            &remove,  // @pyparm int|remove||Specifies extended window styles to be removed during style modification.
            &add,  // @pyparm int|add||Specifies extended extended window styles to be added during style modification.
            &flags))  // @pyparm int|flags|0|Flags to be passed to SetWindowPos, or zero if SetWindowPos should not be
                      // called. The default is zero.
        return NULL;

    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    BOOL rc;
    GUI_BGN_SAVE;
    rc = pWnd->ModifyStyleEx(remove, add, flags);
    // @pyseemfc CWnd|ModifyStyleEx
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
    // @comm If nFlags is nonzero, ModifyStyleEx calls the Windows API function ::SetWindowPos and redraws the window by
    // combining nFlags with the following four preset flags: <nl>* SWP_NOSIZE	Retains the current size. <nl>*
    // SWP_NOMOVE	Retains the current position. <nl>* SWP_NOZORDER	Retains the current Z order. <nl>*
    // SWP_NOACTIVATE	Does not activate the window. <nl>See also <om PyCWnd.ModifyStyle>
}

// @pymethod |PyCWnd|MoveWindow|Move a window to a new location.
static PyObject *ui_window_move_window(PyObject *self, PyObject *args)
{
    CRect rect;
    BOOL bRepaint = TRUE;
    if (!PyArg_ParseTuple(
            args, "(iiii)|i:MoveWindow",
            // @pyparm (left, top, right, bottom)|rect||The new location of the window, relative to the parent.
            &rect.left, &rect.top, &rect.right, &rect.bottom,
            &bRepaint))  // @pyparm int|bRepaint|1|Indicates if the window should be repainted after the move.
        return NULL;

    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->MoveWindow(rect, bRepaint);
    // @pyseemfc CWnd|MoveWindow
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCWnd|OnCtlColor|Calls the default MFC OnCtlColor handler.
// @xref <vm PyCWnd.OnCtlColor>
static PyObject *ui_window_on_ctl_color(PyObject *self, PyObject *args)
{
    PyObject *obDC;
    PyObject *obControl;
    int nCtlColor;
    if (!PyArg_ParseTuple(args, "OOi:OnCtlColor",
                          &obDC,        // @pyparm <o PyCDC>|dc||The dc
                          &obControl,   // @pyparm <o PyCWin>|control||The control that want's it's color changed
                          &nCtlColor))  // @pyparm int|type||Type of control
        return NULL;

    CDC *pDC;
    if (!(pDC = ui_dc_object::GetDC(obDC)))
        return NULL;

    CWnd *pCtl = GetWndPtr(obControl);

    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    HBRUSH brush = pWnd->WndHack::OnCtlColor(pDC, pCtl, nCtlColor);
    GUI_END_SAVE;
    // @pyseemfc CWnd|OnCtlColor
    return PyWinLong_FromHANDLE(brush);
}

// @pymethod int|PyCWnd|OnEraseBkgnd|Calls the default MFC OnEraseBkgnd handler.
// @xref <vm PyCWnd.OnEraseBkgnd>
static PyObject *ui_window_on_erase_bkgnd(PyObject *self, PyObject *args)
{
    PyObject *obDC;
    if (!PyArg_ParseTuple(args, "O:OnEraseBkgnd",
                          &obDC))  // @pyparm <o PyCDC>|dc||The dc
        return NULL;

    CDC *pDC;
    if (!(pDC = ui_dc_object::GetDC(obDC)))
        return NULL;

    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pWnd->WndHack::OnEraseBkgnd(pDC);
    GUI_END_SAVE;
    // @pyseemfc CWnd|OnEraseBkgnd
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCWnd|OnQueryDragIcon|Calls the default MFC OnQueryDragIcon handler.
static PyObject *ui_window_on_query_drag_icon(PyObject *self, PyObject *args)
{
    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CHECK_NO_ARGS2(args, OnQueryDragIcon);
    // @xref <vm PyCWnd::OnQueryDragIcon>
    GUI_BGN_SAVE;
    HICON rc = pWnd->OnQueryDragIcon();
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(rc);
}

// @pymethod int|PyCWnd|OnPaint|Calls the default MFC OnPaint handler.
static PyObject *ui_window_on_paint(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OnPaint"))
        return NULL;

    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->WndHack::OnPaint();
    GUI_END_SAVE;
    // @pyseemfc CWnd|OnEraseBkgnd
    // @xref <vm PyCWnd.OnPaint>
    RETURN_NONE;
}

// @pymethod int|PyCWnd|OnClose|Calls the default MFC OnClose handler.
// @xref <vm PyCWnd.OnClose>
static PyObject *ui_window_on_close(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OnPaint"))
        return NULL;

    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->WndHack::OnClose();
    GUI_END_SAVE;
    // @pyseemfc CWnd|OnClose
    RETURN_NONE;
}

// @pymethod (int,int)|PyCWnd|OnWndMsg|Calls the default MFC Window Message handler.
static PyObject *ui_window_on_wnd_msg(PyObject *self, PyObject *args)
{
    LRESULT res;
    int msg;
    PyWin_PARAMHolder wParam;
    PyWin_PARAMHolder lParam;
    PyObject *obwParam, *oblParam;
    CRect rect;
    BOOL bRepaint = TRUE;
    if (!PyArg_ParseTuple(args, "iOO:OnWndMsg",
                          &msg,        // @pyparm int|msg||The message
                          &obwParam,   // @pyparm int|wParam||The wParam for the message
                          &oblParam))  // @pyparm int|lParam||The lParam for the message
        return NULL;

    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;
    if (!PyWinObject_AsPARAM(obwParam, &wParam))
        return NULL;
    if (!PyWinObject_AsPARAM(oblParam, &lParam))
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pWnd->WndHack::OnWndMsg(msg, wParam, lParam, &res);
    GUI_END_SAVE;
    // @pyseemfc CWnd|OnWndMsg
    // @rdesc The return value is a tuple of (int, int), being the
    // return value from the MFC function call, and the value of the
    // lResult param.  Please see the MFC documentation for more details.
    return Py_BuildValue("iN", rc, PyWinObject_FromPARAM(res));
}

// @pymethod |PyCWnd|PostMessage|Post a message to the window.
PyObject *ui_window_post_message(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    UINT message;
    PyObject *obwParam = Py_None, *oblParam = Py_None;
    if (!PyArg_ParseTuple(args, "i|OO:PostMessage",
                          &message,    // @pyparm int|idMessage||The ID of the message to post.
                          &obwParam,   // @pyparm int|wParam|0|The wParam for the message
                          &oblParam))  // @pyparm int|lParam|0|The lParam for the message
        return NULL;
    PyWin_PARAMHolder wParam;
    PyWin_PARAMHolder lParam;
    if (obwParam != Py_None && !PyWinObject_AsPARAM(obwParam, &wParam))
        return NULL;
    if (oblParam != Py_None && !PyWinObject_AsPARAM(oblParam, &lParam))
        return NULL;
    // @pyseemfc CWnd|PostMessage
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    BOOL ok = pWnd->PostMessage(message, wParam, lParam);
    GUI_END_SAVE;
    if (!ok)
        RETURN_API_ERR("CWnd::PostMessage");
    RETURN_NONE;
}

// @pymethod |PyCWnd|RedrawWindow|Updates the specified rectangle or region in the given window's client area.
static PyObject *ui_window_redraw_window(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    // @pyparm (left, top, right, bottom)|rect|None|A rect, or None
    // @pyparm PyCRgn|object|PyCRgn or None|A region
    // @pyparm int|flags|RDW_INVALIDATE \| RDW_UPDATENOW \| RDW_ERASE |
    int flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE;
    PyObject *obRect = Py_None;
    PyObject *obRgn = Py_None;
    if (!PyArg_ParseTuple(args, "|OOi:RedrawWindow", &obRect, &obRgn, &flags))
        return NULL;

    CRgn *pRgn = NULL;
    if (obRgn != Py_None) {
        pRgn = PyCRgn::GetRgn(obRgn);
        if (!pRgn)
            RETURN_TYPE_ERR("obRegion invalid");
    }
    RECT rect;
    RECT *pRect;
    if (obRect == Py_None)
        pRect = NULL;
    else {
        if (!PyArg_ParseTuple(obRect, "(iiii)", &rect.left, &rect.top, &rect.right, &rect.bottom))
            return NULL;
        pRect = &rect;
    }
    // @pyseemfc CWnd|RedrawWindow
    GUI_BGN_SAVE;
    BOOL ok = pWnd->RedrawWindow(pRect, pRgn, flags);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("RedrawWindow failed");
    RETURN_NONE;
}

// @pymethod (x,y) or (l, t, r, b)|PyCWnd|ClientToScreen|Converts the client coordinates of a given point on the display
// to screen coordinates.
static PyObject *ui_window_client_to_screen(PyObject *self, PyObject *args)
{
    // @comm The new screen coordinates are relative to the upper-left corner of the system display.
    // This function assumes that the given pointis in client coordinates.
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CPoint pnt;
    CRect rect;
    BOOL bHaveRect = TRUE;
    // @pyparm (x,y)|point||The client coordinates.
    // @pyparmalt1 (left, top, right, bottom)|rect||The client coordinates.
    if (!PyArg_ParseTuple(args, "(iiii):ClientToScreen", &rect.left, &rect.top, &rect.right, &rect.bottom)) {
        PyErr_Clear();
        bHaveRect = FALSE;
        if (!PyArg_ParseTuple(args, "(ii):ClientToScreen", &pnt.x, &pnt.y)) {
            return NULL;
        }
    }

    // @pyseemfc CWnd|ClientToScreen
    GUI_BGN_SAVE;
    if (bHaveRect)
        pWnd->ClientToScreen(&rect);
    else
        pWnd->ClientToScreen(&pnt);
    GUI_END_SAVE;
    if (bHaveRect)
        return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
    else
        return Py_BuildValue("(ii)", pnt.x, pnt.y);
}

// @pymethod (left, top, right, bottom) or (x, y)|PyCWnd|ScreenToClient|Converts the screen coordinates of a given point
// or rectangle on the display to client coordinates.
static PyObject *ui_window_screen_to_client(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CRect rect;
    CPoint pnt;
    BOOL bHaveRect = TRUE;
    // @pyparm (left, top, right, bottom) or (x,y)|rect||The coordinates to convert.
    // @pyparmalt1 (x,y)|pnt||The coordinates to convert.
    if (!PyArg_ParseTuple(args, "(iiii):ScreenToClient", &rect.left, &rect.top, &rect.right, &rect.bottom)) {
        PyErr_Clear();
        bHaveRect = FALSE;
        if (!PyArg_ParseTuple(args, "(ii):ScreenToClient", &pnt.x, &pnt.y)) {
            return NULL;
        }
    }
    // @pyseemfc CWnd|ScreenToClient
    GUI_BGN_SAVE;
    if (bHaveRect) {
        pWnd->ScreenToClient(&rect);
    }
    else {
        pWnd->ScreenToClient(&pnt);
    }
    GUI_END_SAVE;
    // @rdesc The result is the same size as the input argument.
    if (bHaveRect) {
        return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
    }
    else {
        return Py_BuildValue("(ii)", pnt.x, pnt.y);
    }
}

// @pymethod <o PyCWnd>|PyCWnd|SetActiveWindow|Sets the window active.  Returns the previously active window, or None.
static PyObject *ui_window_set_active_window(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    CHECK_NO_ARGS(args);
    CWnd *pRel;
    GUI_BGN_SAVE;
    pRel = pWnd->SetActiveWindow();
    GUI_END_SAVE;
    if (!pRel)
        RETURN_NONE;
    return PyCWnd::make(UITypeFromCObject(pRel), pRel)->GetGoodRet();
    // @rdesc The result is the previous window with focus, or None.
}
// @pymethod |PyCWnd|SetRedraw|Allows changes to be redrawn or to prevent changes from being redrawn.
static PyObject *ui_window_set_redraw(PyObject *self, PyObject *args)
{
    CWnd *pView = GetWndPtr(self);
    if (!pView)
        return NULL;
    BOOL bState = TRUE;
    // @pyparm int|bState|1|Specifies the state of the redraw flag.
    if (!PyArg_ParseTuple(args, "i:SetRedraw", &bState))
        return NULL;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|SetRedraw
    pView->SetRedraw(bState);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCWnd|SetScrollInfo|Set information about a scroll bar
static PyObject *ui_window_set_scroll_info(PyObject *self, PyObject *args)
{
    int nBar;
    BOOL bRedraw = TRUE;
    PyObject *obInfo;
    // @pyparm int|nBar||The scroll bar to examine.  Can be one of win32con.SB_BOTH, win32con.SB_VERT or
    // win32con.SB_HORZ
    // @pyparm <o SCROLLINFO tuple>|ScrollInfo||The information to set
    // @pyparm int|redraw|1|A flag indicating if the scrollbar should be re-drawn.
    if (!PyArg_ParseTuple(args, "iO|i:SetScrollInfo", &nBar, &obInfo, &bRedraw))
        return NULL;
    SCROLLINFO info;
    info.cbSize = sizeof(SCROLLINFO);
    if (!ParseSCROLLINFOTuple(obInfo, &info))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pWnd->SetScrollInfo(nBar, &info, bRedraw);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetScrollInfo failed");
    RETURN_NONE;
}

// @pymethod int|PyCWnd|SetScrollPos|Sets the current position of the scroll box of a scroll bar.
static PyObject *ui_window_set_scroll_pos(PyObject *self, PyObject *args)
{
    int nBar;
    BOOL bRedraw = TRUE;
    int nPos;
    // @pyparm int|nBar||The scroll bar to set.  Can be one of win32con.SB_VERT or win32con.SB_HORZ
    // @pyparm int|nPos||The new position
    // @pyparm int|redraw|1|A flag indicating if the scrollbar should be redrawn.
    if (!PyArg_ParseTuple(args, "ii|i:SetScrollPos", &nBar, &nPos, &bRedraw))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    long rc = pWnd->SetScrollPos(nBar, nPos, bRedraw);
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod |PyCWnd|SetWindowPlacement|Sets the windows placement
static PyObject *ui_window_set_window_placement(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    WINDOWPLACEMENT pment;
    pment.length = sizeof(pment);
    // @pyparm (tuple)|placement||A tuple representing the WINDOWPLACEMENT structure.
    if (!PyArg_ParseTuple(args, "ii(ii)(ii)(iiii):SetWindowPlacement", &pment.flags, &pment.showCmd,
                          &pment.ptMinPosition.x, &pment.ptMinPosition.y, &pment.ptMaxPosition.x,
                          &pment.ptMaxPosition.y, &pment.rcNormalPosition.left, &pment.rcNormalPosition.top,
                          &pment.rcNormalPosition.right, &pment.rcNormalPosition.bottom))
        return NULL;
    int rc;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|SetWindowPlacement
    rc = pWnd->SetWindowPlacement(&pment);
    GUI_END_SAVE;
    if (!rc)
        RETURN_API_ERR("CWnd::SetWindowPlacement");
    RETURN_NONE;
}
// @pymethod |PyCWnd|SetWindowPos|Sets the windows position information
static PyObject *ui_window_set_window_pos(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    int x, y, cx, cy;
    int flags;
    PyObject *obAfter;
    // @pyparm int|hWndInsertAfter||A hwnd, else one of the win32con.HWND_* constants.
    // @pyparm (x,y,cx,cy)|position||The new position of the window.
    // @pyparm int|flags||Window positioning flags.
    if (!PyArg_ParseTuple(args, "O(iiii)i:SetWindowPos", &obAfter, &x, &y, &cx, &cy, &flags))
        return NULL;
    // It appears we took the easy way above, and assume a handle rather
    // than either int or PyWnd object.  So we jump hoops to convert back
    // to a CWnd, so we can call CWnd::SetWindowPos rather than
    // ::SetWindowPos
    HWND insertAfter;
    if (!PyWinObject_AsHANDLE(obAfter, (HANDLE *)&insertAfter))
        return NULL;
    CWnd wndInsertAfter;
    wndInsertAfter.Attach(insertAfter);
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|SetWindowPos
    BOOL ok = pWnd->SetWindowPos(&wndInsertAfter, x, y, cx, cy, flags);
    GUI_END_SAVE;
    wndInsertAfter.Detach();
    if (!ok)
        RETURN_ERR("SetWindowPos failed");
    RETURN_NONE;
}

// @pymethod |PyCWnd|SetWindowText|Sets the window's text.
static PyObject *ui_window_set_window_text(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    TCHAR *msg;
    PyObject *obmsg;
    // @pyparm string|text||The windows text.
    if (!PyArg_ParseTuple(args, "O:SetWindowText", &obmsg))
        return NULL;
    if (!PyWinObject_AsTCHAR(obmsg, &msg, FALSE))
        return NULL;
    // @pyseemfc CWnd|SetWindowText
    GUI_BGN_SAVE;
    pWnd->SetWindowText(msg);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(msg);
    RETURN_NONE;
}

// @pymethod |PyCWnd|SendMessage|Send a message to the window.
PyObject *ui_window_send_message(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtrGoodHWnd(self);
    if (!pWnd)
        return NULL;
    assert(!PyErr_Occurred());  // lingering exception?
    int message;
    PyWin_PARAMHolder wp;
    PyWin_PARAMHolder lp;
    {
        // more general purpose args.
        PyObject *obwParam = Py_None, *oblParam = Py_None;
        if (!PyArg_ParseTuple(args, "i|OO:SendMessage",
                              &message,    // @pyparm int|idMessage||The ID of the message to send.
                              &obwParam,   // @pyparm int|wParam|0|The wParam for the message
                              &oblParam))  // @pyparm int|lParam|0|The lParam for the message
            return NULL;
        if (obwParam != Py_None && !PyWinObject_AsPARAM(obwParam, &wp))
            return NULL;
        if (oblParam != Py_None) {
            if (!PyWinObject_AsPARAM(oblParam, &lp))
                return NULL;
        }
        else if (wp.bufferView.ok() && PyTuple_Size(args) == 2) {
            // old code compatibily: (msg, buffer_ob) -> lparam==&buffer, wparam=len(buffer)
            lp = (WPARAM)wp.bufferView.ptr();
            wp = (WPARAM)wp.bufferView.len();  // doesn't release the held bufferView so far
            assert(wp.bufferView.ok());
        }
    }
    LRESULT rc;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|SendMessage
    rc = pWnd->SendMessage(message, wp, lp);
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(rc);
}
// @pymethod |PyCWnd|SendMessageToDescendants|Send a message to all descendant windows.
PyObject *ui_window_send_message_to_desc(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    UINT message;
    BOOL bDeep = TRUE;
    PyObject *obwParam = Py_None, *oblParam = Py_None;
    if (!PyArg_ParseTuple(
            args, "i|OOi:SendMessageToDescendants",
            &message,   // @pyparm int|idMessage||The ID of the message to send.
            &obwParam,  // @pyparm int|wParam|0|The wParam for the message
            &oblParam,  // @pyparm int|lParam|0|The lParam for the message
            &bDeep))    // @pyparm int|bDeep|1|Indicates if the message should be recursively sent to all children
        return NULL;
    PyWin_PARAMHolder wParam;
    PyWin_PARAMHolder lParam;
    if (obwParam != Py_None && !PyWinObject_AsPARAM(obwParam, &wParam))
        return NULL;
    if (oblParam != Py_None && !PyWinObject_AsPARAM(oblParam, &lParam))
        return NULL;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|SendMessageToDescendants
    pWnd->SendMessageToDescendants(message, wParam, lParam, bDeep);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|ShowScrollBar|Shows or hides a scroll bar.
// An application should not call ShowScrollBar to hide a scroll bar while processing a scroll-bar notification message.
static PyObject *ui_window_show_scrollbar(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int bar;
    BOOL bShow = TRUE;
    if (!PyArg_ParseTuple(args, "i|i:ShowScrollBar",
                          &bar,  // @pyparm int|nBar||Specifies whether the scroll bar is a control or part of a
                                 // window's nonclient area. If it is part of the nonclient area, nBar also indicates
                                 // whether the scroll bar is positioned horizontally, vertically, or both. It must be
                                 // one of win32con.SB_BOTH, win32con.SB_HORZ or win32con.SB_VERT.
                          &bShow))  // @pyparm int|bShow|1|Indicates if the scroll bar should be shown or hidden.

        return NULL;
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|ShowScrollBar
    pWnd->ShowScrollBar(bar, bShow);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCWnd|ShowWindow|Sets the visibility state of the window.
PyObject *ui_window_show_window(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int style = SW_SHOWNORMAL;
    // @pyparm int|style|win32con.SW_SHOWNORMAL|Specifies how the window is to be shown.
    // It must be one of win32con.SW_HIDE, win32con.SW_MINIMIZE, win32con.SW_RESTORE, win32con.SW_SHOW,
    // win32con.SW_SHOWMAXIMIZED win32con.SW_SHOWMINIMIZED, win32con.SW_SHOWMINNOACTIVE, win32con.SW_SHOWNA,
    // win32con.SW_SHOWNOACTIVATE,  or win32con.SW_SHOWNORMAL
    if (!PyArg_ParseTuple(args, "|i:ShowWindow", &style))
        return NULL;
    int rc;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    rc = pWnd->ShowWindow(style);
    // @pyseemfc CWnd|ShowWindow
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
    // @rdesc Returns TRUE is the window was previously visible.
}
// @pymethod int|PyCWnd|UpdateData|Initialises data in a dialog box, or to retrieves and validates dialog data.
// Returns nonzero if the operation is successful; otherwise 0. If bSaveAndValidate is TRUE, then a return value of
// nonzero means that the data is successfully validated.
static PyObject *ui_window_update_data(PyObject *self, PyObject *args)
{
    int bSAV = TRUE;
    // @pyparm int|bSaveAndValidate|1|Flag that indicates whether dialog box is being initialized (FALSE) or data is
    // being retrieved (TRUE).
    if (!PyArg_ParseTuple(args, "|i:UpdateData", &bSAV))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    BOOL rc = FALSE;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    // @pyseemfc CWnd|UpdateData
    rc = pWnd->UpdateData(bSAV);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod |PyCWnd|UpdateWindow|Updates a window.  This forces a paint message to be sent to the window, if any part
// of the window is marked as invalid.
static PyObject *ui_window_update_window(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    pWnd->UpdateWindow();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|InvalidateRect|Invalidates an area of a window.
static PyObject *ui_window_invalidate_rect(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    BOOL erase = TRUE;

    if (!pWnd)
        return NULL;
    CRect rect(CFrameWnd::rectDefault), *r;
    // @pyparm (left, top, right, bottom)|rect|(0,0,0,0)|Rectangle to be
    // updated.  If default param is used, the entire window is invalidated.
    // @pyparm int|bErase|1|Specifies whether the background within the update region is to be erased.
    if (!PyArg_ParseTuple(args, "|(iiii)i:InvalidateRect", &rect.left, &rect.top, &rect.right, &rect.bottom, &erase)) {
        return NULL;
    }
    if (rect == CFrameWnd::rectDefault)
        r = NULL;
    else
        r = &rect;
    CHECK_HWND_VALID(pWnd);
    GUI_BGN_SAVE;
    pWnd->InvalidateRect(r, erase);
    // @pyseemfc CWnd|InvalidateRect
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCDC>|PyCWnd|GetDC|Gets the windows current DC object.
static PyObject *ui_window_get_dc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = (CWnd *)GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;

    // create MFC device context
    CDC *pDC = pWnd->GetDC();
    if (pDC == NULL)
        RETURN_ERR("Could not get the DC for the window.");

    // create Python device context
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC)->GetGoodRet();
    return dc;
    // @rdesc The result is a <o PyCDC>, or a win32ui.error exception is raised.
}

// @pymethod |PyCWnd|SetCapture|Causes all subsequent mouse input to be sent to the window object regardless of the
// position of the cursor.
static PyObject *ui_window_set_capture(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;

    GUI_BGN_SAVE;
    pWnd->SetCapture();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCWnd|SetFocus|Claims the input focus.  The object that previously had the focus loses it.
static PyObject *ui_window_set_focus(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->SetFocus();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|SetFont|Sets the window's current font to the specified font.
static PyObject *ui_window_set_font(PyObject *self, PyObject *args)
{
    PyObject *pfont;
    int redraw = 1;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL) {
        return NULL;
        // @pyparm <o PyCFont>|font||The new font to use.
        // @pyparm int|bRedraw|1|If TRUE, redraw the window.
    }
    else if (!PyArg_ParseTuple(args, "O|i", &pfont, &redraw)) {
        return NULL;
    }
    else if (!ui_base_class::is_uiobject(pfont, &PyCFont::type)) {
        RETURN_ERR("First argument must be a font object.");
    }
    else {
        GUI_BGN_SAVE;
        pWnd->SetFont(((PyCFont *)pfont)->GetFont(), redraw);
        GUI_END_SAVE;
        RETURN_NONE;
    }
}

// @pymethod |PyCWnd|SetForegroundWindow|Puts the window into the foreground and activates the window.
static PyObject *ui_window_set_foreground_window(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    BOOL rc;
    GUI_BGN_SAVE;
    rc = pWnd->SetForegroundWindow();
    GUI_END_SAVE;
    if (!rc)
        RETURN_API_ERR("SetForegroundWindow");
    RETURN_NONE;
}

// @pymethod |PyCWnd|SetMenu|Sets the menu for a window.
PyObject *ui_window_set_menu(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    PyObject *menuObject;
    //@pyparm PyCMenu|menuObj||The menu object to set, or None to remove the window.
    if (!PyArg_ParseTuple(args, "O:SetMenu", &menuObject))
        return NULL;
    CMenu *pMenu = NULL;
    if (menuObject != Py_None) {
        if (!ui_base_class::is_uiobject(menuObject, &PyCMenu::type))
            RETURN_TYPE_ERR("passed object must None or be a PyCMenu");
        HMENU hMenu = PyCMenu::GetMenu(menuObject);
        if (hMenu == NULL)
            return NULL;
        pMenu = CMenu::FromHandle(hMenu);
        if (pMenu == NULL)
            RETURN_TYPE_ERR("The menu object is invalid");
    }
    GUI_BGN_SAVE;
    BOOL ok = pWnd->SetMenu(pMenu);
    GUI_END_SAVE;
    if (!ok)
        RETURN_API_ERR("CWnd::SetMenu");
    RETURN_NONE;
}

// @pymethod |PyCWnd|ReleaseCapture|Releases the mouse capture for this window.  See <om PyCWnd.SetCapture>.
static PyObject *ui_window_release_capture(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *view = GetWndPtr(self);
    if (view == NULL)
        return NULL;

    GUI_BGN_SAVE;
    if (view->GetCapture() == view)
        ReleaseCapture();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|ReleaseDC|Releases a device context, freeing it for use by other applications.
static PyObject *ui_window_release_dc(PyObject *self, PyObject *args)
{
    PyObject *obDC;
    CWnd *view = GetWndPtr(self);
    if (view == NULL)
        return NULL;

    // @pyparm <o PyCDC>|dc||The DC to be released.
    if (!PyArg_ParseTuple(args, "O:ReleaseDC", &obDC))
        return NULL;

    // Get the MFC device context
    CDC *pDC;
    if (!(pDC = ui_dc_object::GetDC(obDC)))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = view->ReleaseDC(pDC);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("ReleaseDC failed");
    RETURN_NONE;
}

// @pymethod int|PyCWnd|MouseCaptured|Returns 1 if the window has the mouse capture, else 0
static PyObject *ui_window_mouse_captured(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *view = GetWndPtr(self);
    if (view == NULL)
        return NULL;

    // Returns true if this win has the mouse captured
    GUI_BGN_SAVE;
    BOOL is = view->GetCapture() == view;
    GUI_END_SAVE;
    return Py_BuildValue("i", is);
}

// @pymethod int|PyCWnd|UpdateDialogControls|Updates the state of dialog buttons and other controls in a dialog box or
// window that uses the <om PyCCmdUI.HookCommandUpdate> callback mechanism.
static PyObject *ui_window_udc(PyObject *self, PyObject *args)
{
    PyObject *obTarget;
    BOOL bDisable;
    // @pyparm <o PyCCmdTarget>|pTarget||The main frame window of the application, and is used for routing update
    // messages.
    // @pyparm int|disableIfNoHandler||Flag that indicates whether a control that has no update handler should be
    // automatically displayed as disabled.
    if (!PyArg_ParseTuple(args, "Oi:UpdateDialogControls", &obTarget, &bDisable))
        return NULL;

    extern CCmdTarget *GetCCmdTargetPtr(PyObject * self);
    CCmdTarget *pTarget;
    if (!(pTarget = GetCCmdTargetPtr(obTarget)))
        return NULL;
    CWnd *view = GetWndPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->UpdateDialogControls(pTarget, bDisable);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|ShowCaret|Shows the caret
PyObject *ui_window_show_caret(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, ShowCaret);
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->ShowCaret();
    GUI_END_SAVE;
    RETURN_NONE;
    // @comm See also <om PyCWnd.HideCaret>
}

// @pymethod |PyCWnd|HideCaret|Hides the caret
PyObject *ui_window_hide_caret(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, HideCaret);
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->HideCaret();
    GUI_END_SAVE;
    RETURN_NONE;
    // @comm See also <om PyCWnd.ShowCaret>
}

// @pymethod <o PyCDC>, <o PAINTSTRUCT>|PyCWnd|BeginPaint|Prepares a window for painting
// @rdesc You must pass the PAINTSTRUCT param to the <om PyCWnd.EndPaint> method.
PyObject *ui_window_begin_paint(PyObject *self, PyObject *args)
{
    PAINTSTRUCT ps;
    if (!PyArg_ParseTuple(args, ":BeginPaint"))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CDC *pTemp = pWnd->BeginPaint(&ps);
    GUI_END_SAVE;
    if (pTemp == NULL)
        RETURN_ERR("BeginPaint failed");
    PyObject *obDC = ui_assoc_object::make(ui_dc_object::type, pTemp)->GetGoodRet();
    PyObject *obRet = Py_BuildValue("O(Ni(iiii)iiN)", obDC, PyWinLong_FromHANDLE(ps.hdc), ps.fErase, ps.rcPaint.left,
                                    ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, ps.fRestore, ps.fIncUpdate,
                                    PyBytes_FromStringAndSize((char *)ps.rgbReserved, sizeof(ps.rgbReserved)));
    Py_XDECREF(obDC);
    return obRet;
}

// @pymethod |PyCWnd|EndPaint|Ends painting
PyObject *ui_window_end_paint(PyObject *self, PyObject *args)
{
    PAINTSTRUCT ps;
    PyObject *obString;
    PyObject *obhdc;
    // @pyparm <o PAINTSTRUCT>|paintStruct||The object returned from <om PyCWnd.BeginPaint>
    if (!PyArg_ParseTuple(args, "(Oi(iiii)iiO)", &obhdc, &ps.fErase, &ps.rcPaint.left, &ps.rcPaint.top,
                          &ps.rcPaint.right, &ps.rcPaint.bottom, &ps.fRestore, &ps.fIncUpdate, &obString))
        return NULL;

    if (!PyBytes_Check(obString) || PyBytes_Size(obString) != sizeof(ps.rgbReserved))
        RETURN_TYPE_ERR("Last tuple must be a string of a specific size!");
    memcpy(ps.rgbReserved, PyBytes_AsString(obString), sizeof(ps.rgbReserved));

    if (!PyWinObject_AsHANDLE(obhdc, (HANDLE *)&ps.hdc))
        return NULL;

    CWnd *pWnd = GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;

    GUI_BGN_SAVE;
    pWnd->EndPaint(&ps);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|CenterWindow|Centers a window relative to its parent.
static PyObject *ui_window_center_window(PyObject *self, PyObject *args)
{
    PyObject *obAltWnd = NULL;
    CWnd *view = GetWndPtr(self);
    if (view == NULL)
        return NULL;

    // @pyparm <o PyCWnd>|altwin|None|alternate window relative to which it will be centered (other than the parent
    // window).
    if (!PyArg_ParseTuple(args, "|O:CenterWindow", &obAltWnd))
        return NULL;

    // Get the MFC device context
    CWnd *pAltWnd = NULL;
    if (obAltWnd) {
        if (obAltWnd != Py_None) {
            if (!(pAltWnd = GetWndPtr(obAltWnd)))
                RETURN_TYPE_ERR("Argument must be a PyCWnd or None");
        }
    }
    GUI_BGN_SAVE;
    view->CenterWindow(pAltWnd);  // @pyseemfc CWnd|CenterWindow
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|PumpWaitingMessages|Pump messages associate with a window.
PyObject *ui_window_pump_waiting_messages(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    UINT firstMsg;
    UINT lastMsg;
    if (!PyArg_ParseTuple(args, "ii:PumpWaitingMessages",
                          &firstMsg,  // @pyparm int|firstMsg||First message ID to process
                          &lastMsg))  // @pyparm int|lastMsg||First message ID to process
        return NULL;
    // @pyseemfc CWnd|PeekMessage and DispatchMessage
    CHECK_HWND_VALID(pWnd);
    MSG msg;
    GUI_BGN_SAVE;
    if (::PeekMessage(&msg, pWnd->m_hWnd, firstMsg, lastMsg, PM_REMOVE)) {
        ::DispatchMessage(&msg);
    }
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|LockWindowUpdate|Disables drawing in the given window
static PyObject *ui_lock_window_update(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->LockWindowUpdate();  // @pyseemfc CWnd|LockWindowUpdate
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|UnlockWindowUpdate|Unlocks a window that was locked with LockWindowUpdate
static PyObject *ui_unlock_window_update(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    GUI_BGN_SAVE;
    pWnd->UnlockWindowUpdate();  // @pyseemfc CWnd|UnLockWindowUpdate
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCDC>|PyCWnd|GetWindowDC|Gets the windows current DC object.
static PyObject *ui_window_get_window_dc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = (CWnd *)GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;

    // create MFC device context
    CDC *pDC = pWnd->GetWindowDC();
    if (pDC == NULL)
        RETURN_ERR("Could not get the DC for the window.");

    // create Python device context
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC)->GetGoodRet();
    return dc;
}

// @pymethod <o PyCDC>|PyCWnd|GetDCEx|Gets the windows current DC object with extended caps.
static PyObject *ui_window_get_dc_ex(PyObject *self, PyObject *args)
{
    CWnd *pWnd = (CWnd *)GetWndPtr(self);
    if (pWnd == NULL)
        return NULL;

    PyObject *objRgn;
    DWORD flags;
    if (!PyArg_ParseTuple(args, "Oi:GetDCEx", &objRgn, &flags))
        return NULL;

    CRgn *prgnClip;
    if (objRgn == Py_None)
        prgnClip = NULL;
    else {
        prgnClip = PyCRgn::GetRgn(objRgn);
        if (!prgnClip)
            return NULL;
    }

    // create MFC device context
    CDC *pDC = pWnd->GetDCEx(prgnClip, flags);
    if (pDC == NULL)
        RETURN_ERR("Could not get the DC for the window.");

    // create Python device context
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC)->GetGoodRet();
    return dc;
}
// @pymethod int|PyCWnd|IsWindow|determines whether the specified window handle identifies an existing window
static PyObject *ui_window_is_window(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    BOOL bAns = ::IsWindow(pWnd->m_hWnd);
    return Py_BuildValue("i", bAns);
}

// @pymethod |PyCWnd|MapWindowPoints|Converts (maps) a set of points from the coordinate space of a window to the
// coordinate space of another window.
static PyObject *ui_window_map_window_points(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    PyObject *pPyCWnd;
    PyObject *point_list;
    // @pyparm <o PyCWnd>|wnd||
    // @pyparm [ (x,y), ...]|points||The points to map

    if (!PyArg_ParseTuple(args, "OO:MapWindowPoints", &pPyCWnd, &point_list))
        return NULL;

    CWnd *pWndTo;
    if (pPyCWnd == Py_None)
        pWndTo = NULL;  // i.e screen coordinates conversion
    else if (ui_base_class::is_uiobject(pPyCWnd, &PyCWnd::type))
        pWndTo = GetWndPtr(pPyCWnd);
    else
        RETURN_TYPE_ERR("1st argument must be a window object, or None");

    if (!PyList_Check(point_list))
        return NULL;

    // Convert the list of point tuples into an array of POINT structs
    Py_ssize_t num = PyList_Size(point_list);
    Py_ssize_t i;
    POINT *point_array = new POINT[num];
    for (i = 0; i < num; i++) {
        PyObject *point_tuple = PyList_GetItem(point_list, i);
        if (!PyTuple_Check(point_tuple) || PyTuple_Size(point_tuple) != 2) {
            PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
            delete[] point_array;
            return NULL;
        }
        else {
            long x, y;
            PyObject *px, *py;
            px = PyTuple_GetItem(point_tuple, 0);
            py = PyTuple_GetItem(point_tuple, 1);
            if ((!PyLong_Check(px)) || (!PyLong_Check(py))) {
                PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
                delete[] point_array;
                return NULL;
            }
            else {
                x = PyLong_AsLong(px);
                y = PyLong_AsLong(py);
                point_array[i].x = x;
                point_array[i].y = y;
            }
        }
    }
    // we have an array of POINT structs, now we
    // can finally call the mfc function.
    GUI_BGN_SAVE;
    pWnd->MapWindowPoints(pWndTo, point_array, PyWin_SAFE_DOWNCAST(num, Py_ssize_t, UINT));
    GUI_END_SAVE;

    // create a list
    // copy mapped points
    // return list of points
    PyObject *list = PyList_New(num);
    for (i = 0; i < num; i++) PyList_SetItem(list, i, Py_BuildValue("(ii)", point_array[i].x, point_array[i].y));

    delete[] point_array;

    return list;
    // @rdesc A list of the mapped points from the coordinate space of the CWnd to the coordinate space of another
    // window.
}
// @pymethod int|PyCWnd|SetTimer|Installs a system timer
static PyObject *ui_window_set_timer(PyObject *self, PyObject *args)
{
    UINT nIDEvent, nElapse;
    // @pyparm int|idEvent||The ID of the event
    // @pyparm int|elapse||How often the timer should fire.
    if (!PyArg_ParseTuple(args, "ii:SetTimer", &nIDEvent, &nElapse))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    // @pyseemfc CWnd|SetTimer
    GUI_BGN_SAVE;
    UINT_PTR id = pWnd->SetTimer(nIDEvent, nElapse, NULL);
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(id);
}

// @pymethod int|PyCWnd|KillTimer|Kills a system timer
static PyObject *ui_window_kill_timer(PyObject *self, PyObject *args)
{
    UINT nID;
    if (!PyArg_ParseTuple(args, "i:KillTimer", &nID))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    // @pyseemfc CWnd|KillTimer
    GUI_BGN_SAVE;
    BOOL br = pWnd->KillTimer(nID);
    GUI_END_SAVE;
    return Py_BuildValue("i", br);
}

// @pymethod |PyCWnd|InvalidateRgn|Invalidates a region of the window
PyObject *ui_window_invalidate_rgn(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    PyObject *objRgn;
    BOOL bErase = TRUE;
    // @pyparm <o PyCRgn>|region||The region to erase.
    // @pyparm int|bErase|1|Indicates if the region should be erased.
    if (!PyArg_ParseTuple(args, "O|i:InvalidateRgn", &objRgn, &bErase))
        return NULL;
    CRgn *pRgn = PyCRgn::GetRgn(objRgn);
    if (!pRgn)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->InvalidateRgn(pRgn, bErase);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCWnd|RunModalLoop|Begins a modal loop for the window.
PyObject *ui_window_run_modal_loop(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    DWORD dwFlags;
    // @pyparm int|flags||
    if (!PyArg_ParseTuple(args, "i:RunModalLoop", &dwFlags))
        return NULL;
    GUI_BGN_SAVE;
    int nResult = pWnd->RunModalLoop(dwFlags);
    GUI_END_SAVE;
    return Py_BuildValue("i", nResult);
}
// @pymethod |PyCWnd|EndModalLoop|Ends a modal loop.
PyObject *ui_window_end_modal_loop(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    int nResult;
    // @pyparm int|result||The result as returned to RunModalLoop
    if (!PyArg_ParseTuple(args, "i:EndModalLoop", &nResult))
        return NULL;
    GUI_BGN_SAVE;
    pWnd->EndModalLoop(nResult);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWnd|RepositionBars|Repositions the windows control bars.( UINT nIDFirst, UINT nIDLast, UINT
// nIDLeftOver, UINT nFlag = CWnd::reposDefault, LPRECT lpRectParam = NULL, LPCRECT lpRectClient = NULL, BOOL bStretch =
// TRUE );
PyObject *ui_window_reposition_bars(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    UINT nIDFirst, nIDLast, nIDLeftOver;
    // @pyparm int|idFirst||The ID of the first control to reposition.
    // @pyparm int|idLast||The ID of the last control to reposition.
    // @pyparm int|idLeftOver||
    if (!PyArg_ParseTuple(args, "iii:RepositionBars", &nIDFirst, &nIDLast, &nIDLeftOver))
        return NULL;
    GUI_BGN_SAVE;
    pWnd->RepositionBars(nIDFirst, nIDLast, nIDLeftOver);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCWnd|OnNcHitTest|Calls the base MFC OnNcHitTest function.
// @xref <vm PyCWnd.OnNcHitTest>
static PyObject *ui_window_on_nc_hit_test(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    CPoint p;
    if (!PyArg_ParseTuple(args, "(ii):OnNcHitTest", &p.x, &p.y))  // @pyparm int, int|x, y||The point
        return NULL;
    GUI_BGN_SAVE;
    LRESULT rc = ((WndHack *)pWnd)->OnNcHitTest(p);
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(rc);
}

// @pymethod int|PyCWnd|OnSetCursor|Calls the base MFC OnSetCursor function.
// @xref <vm PyCWnd.OnSetCursor>
static PyObject *ui_window_on_set_cursor(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    PyObject *obWnd;
    int ht, msg;
    // @pyparm <o PyCWnd>|wnd||
    // @pyparm int|hittest||
    // @pyparm int|message||
    if (!PyArg_ParseTuple(args, "Oii:OnSetCursor", &obWnd, &ht, &msg))
        return NULL;
    CWnd *pWndArg = GetWndPtrFromParam(obWnd, PyCWnd::type);
    if (pWndArg == NULL)
        return NULL;
    GUI_BGN_SAVE;
    UINT rc = ((WndHack *)pWnd)->OnSetCursor(pWndArg, ht, msg);
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod int|PyCWnd|OnMouseActivate|Calls the base MFC OnMouseActivate function.
// @xref <vm PyCWnd.OnMouseActivate>
static PyObject *ui_window_on_mouse_activate(PyObject *self, PyObject *args)
{
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    PyObject *obWnd;
    int ht, msg;
    // @pyparm <o PyCWnd>|wnd||
    // @pyparm int|hittest||
    // @pyparm int|message||
    if (!PyArg_ParseTuple(args, "Oii:OnMouseActivate", &obWnd, &ht, &msg))
        return NULL;
    CWnd *pWndArg = GetWndPtrFromParam(obWnd, PyCWnd::type);
    if (pWndArg == NULL)
        return NULL;
    GUI_BGN_SAVE;
    UINT rc = ((WndHack *)pWnd)->OnMouseActivate(pWndArg, ht, msg);
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod tuple|PyCWnd|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
// @xref <vm PyCWnd.PreCreateWindow>
static PyObject *ui_window_pre_create_window(PyObject *self, PyObject *args)
{
    WndHack *pWnd = (WndHack *)GetWndPtr(self);
    if (!pWnd)
        return NULL;

    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pWnd->WndHack::PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CWnd::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod int|PyCWnd|OnQueryNewPalette|Calls the underlying MFC OnQueryNewPalette method.
// @xref <vm PyCWnd.OnQueryNewPalette>
PyObject *ui_window_on_query_new_palette(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnQueryNewPalette);
    WndHack *wnd = (WndHack *)GetWndPtr(self);
    if (wnd == NULL)
        return NULL;
    GUI_BGN_SAVE;
    int rc = wnd->WndHack::OnQueryNewPalette();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod HICON|PyCWnd|SetIcon|Calls the underlying MFC SetIcon method.
PyObject *ui_window_set_icon(PyObject *self, PyObject *args)
{
    PyObject *obiconprev;
    BOOL bBigIcon = TRUE;
    if (!PyArg_ParseTuple(args, "Oi:SetIcon", &obiconprev, &bBigIcon))
        return NULL;
    HICON hiconprev;
    if (!PyWinObject_AsHANDLE(obiconprev, (HANDLE *)&hiconprev))
        return NULL;
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    HICON hiconRetVal = pWnd->SetIcon(hiconprev, bBigIcon);
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(hiconRetVal);
}

///////////////////////////////////////
//
// Window Methods
//
///////////////////////////////////////
// @object PyCWnd|A base window class.  Encapsulates an MFC <c CWnd> class
static struct PyMethodDef PyCWnd_methods[] = {
    // Window Methods
    {"ActivateFrame", ui_window_activate_frame,
     1},  // @pymeth ActivateFrame|Searches upwards for a parent window which has a frame, and activates it.
    {"BringWindowToTop", ui_window_bring_window_to_top,
     1},  // @pymeth BringWindowToTop|Brings the window to the top of a stack of overlapping windows.
    {"BeginPaint", ui_window_begin_paint, 1},           // @pymeth BeginPaint|Prepares the window for painting.
    {"CalcWindowRect", ui_window_calc_window_rect, 1},  // @pymeth CalcWindowRect|Computes the size of the window
                                                        // rectangle based on the desired client rectangle size.
    {"CenterWindow", ui_window_center_window, 1},       // @pymeth CenterWindow|Centers a window relative to its parent.
    {"CheckRadioButton", ui_window_check_radio_button, 1},  // @pymeth CheckRadioButton|Selects a specified radio button
    {"ChildWindowFromPoint", ui_child_window_from_point,
     1},  // @pymeth ChildWindowFromPoint|Identifies the child window that contains the point
    {"ClientToScreen", ui_window_client_to_screen,
     1},                                           // @pymeth ClientToScreen|Convert coordinates from Client to Screen
    {"CreateWindow", ui_window_create_window, 1},  // @pymeth CreateWindow|Create the underlying window object
    {"CreateWindowEx", ui_window_create_window_ex,
     1},  // @pymeth CreateWindowEx|Creates the actual window for the PyCWnd object using extended attributes.
    {"DefWindowProc", ui_window_def_window_proc, 1},  // @pymeth DefWindowProc|Calls the default message handler.
    {"DestroyWindow", ui_window_destroy_window,
     1},                                        // @pymeth DestroyWindow|Destroys the window attached to the object.
    {"DlgDirList", ui_window_dlg_dir_list, 1},  // @pymeth DlgDirList|Fill a listbox control with a file specification.
    {"DlgDirListComboBox", ui_window_dlg_dir_list_combo,
     1},  // @pymeth DlgDirListComboBox|Fill a combobox control with a file specification.
    {"DlgDirSelect", ui_window_dlg_dir_select,
     1},  // @pymeth DlgDirSelect|Retrieves the current selection from a list box.
    {"DlgDirSelectComboBox", ui_window_dlg_dir_select_combo,
     1},  // @pymeth DlgDirSelectComboBox|Retrieves the current selection from a combo box.
    {"DragAcceptFiles", ui_window_drag_accept_files,
     1},  // @pymeth DragAcceptFiles|Indicate the window can accept files dragges from file manager.
    {"DrawMenuBar", ui_window_draw_menu_bar, 1},    // @pymeth DrawMenuBar|Redraw the windows menu bar.
    {"EnableWindow", ui_window_enable_window, 1},   // @pymeth EnableWindow|Enable or disable the window.
    {"EndModalLoop", ui_window_end_modal_loop, 1},  // @pymeth EndModalLoop|Ends a modal loop.
    {"EndPaint", ui_window_end_paint, 1},           // @pymeth EndPaint|Ends painting in a window
    {"GetCheckedRadioButton", ui_window_get_checked_rb,
     1},  // @pymeth GetCheckedRadioButton|Get the ID of the checked a radio button in a group.
    {"GetClientRect", ui_window_get_client_rect, 1},  // @pymeth GetClientRect|Gets the client rectangle for thewindow.
    {"GetDC", ui_window_get_dc, 1},                   // @pymeth GetDC|Gets the window's current device context.
    {"GetDCEx", ui_window_get_dc_ex, 1},              // @pymeth GetDCEx|Gets the window's current device context.
    {"GetDlgCtrlID", ui_window_get_dlg_ctrl_id, 1},   // @pymeth GetDlgCtrlID|Get the current window's control id.
    {"GetDlgItem", ui_window_get_dlg_item, 1},        // @pymeth GetDlgItem|Get a child control by Id
    {"GetDlgItemInt", ui_window_get_dlg_item_int,
     1},  // @pymeth GetDlgItemInt|Returns the integer value of a child window or control with the specified ID.
    {"GetDlgItemText", ui_window_get_dlg_item_text,
     1},  // @pymeth GetDlgItemText|Returns the text of child window or control with the specified ID.
    {"GetLastActivePopup", ui_window_get_last_active_popup,
     1},                                 // @pymeth GetLastActivePopup|Identifies the most recently active pop-up window
    {"GetMenu", ui_window_get_menu, 1},  // @pymeth GetMenu|Get the current menu for a window.
    {"GetParent", ui_window_get_parent, 1},             // @pymeth GetParent|Get the parent window.
    {"GetParentFrame", ui_window_get_parent_frame, 1},  // @pymeth GetParentFrame|Returns the window's frame.
    {"GetParentOwner", ui_window_get_parent_owner,
     1},  // @pymeth GetParent|Returns the child window's parent window or owner window.
    {"GetSafeHwnd", ui_window_get_safe_hwnd, 1},      // @pymeth GetSafeHwnd|Returns the HWnd of this window.
    {"GetScrollInfo", ui_window_get_scroll_info, 1},  // @pymeth GetScrollInfo|Retrieve information about a scroll bar
    {"GetScrollPos", ui_window_get_scroll_pos,
     1},  // @pymeth GetScrollPos|Retrieves the current position of the scroll box of a scroll bar.
    {"GetStyle", ui_window_get_style, 1},             // @pymeth GetStyle|Retrieves the window style
    {"GetExStyle", ui_window_get_ex_style, 1},        // @pymeth GetExStyle|Retrieves the window extended style
    {"GetSystemMenu", ui_window_get_system_menu, 1},  // @pymeth GetSystemMenu|Get the system menu for the window.
    {"GetTopLevelFrame", ui_window_get_top_level_frame, 1},  // @pymeth GetTopLevelFrame|Get the top-level frame window.
    {"GetTopLevelOwner", ui_window_get_top_level_owner, 1},  // @pymeth GetTopLevelOwner|Get the top-level owner window.
    {"GetTopLevelParent", ui_window_get_top_level_parent,
     1},  // @pymeth GetTopLevelParent|Get the top-level parent window.
    {"GetTopWindow", PyCWnd::get_top_window,
     1},                                   // @pymeth GetTopWindow|Get the top level window attached to this window.
    {"GetWindow", PyCWnd::get_window, 1},  // @pymeth GetWindow|Get a specified window (eg, parent, child, etc).
    {"GetWindowDC", ui_window_get_window_dc, 1},  // @pymeth GetWindowDC|Obtains the <o PyDC> for a window.
    {"GetWindowPlacement", ui_window_get_window_placement,
     1},  // @pymeth GetWindowPlacement|Gets the window's current placement information.
    {"GetWindowRect", ui_window_get_window_rect, 1},  // @pymeth GetWindowRect|Get the windows rectangle.
    {"GetWindowText", ui_window_get_window_text, 1},  // @pymeth GetWindowText|Get the window's current text.
    {"HideCaret", ui_window_hide_caret, 1},           // @pymeth HideCaret|Hides the caret
    {"HookAllKeyStrokes", ui_window_hook_all_key_strokes,
     1},  // @pymeth HookAllKeyStrokes|Hook a handler for all keystroke messages.
    {"HookKeyStroke", ui_window_hook_key_stroke, 1},  // @pymeth HookKeyStroke|Hook a keystroke handler.
    {"HookMessage", ui_window_hook_message, 1},       // @pymeth HookMessage|Hook a message notification handler.
    {"InvalidateRect", ui_window_invalidate_rect,
     1},  // @pymeth InvalidateRect|Invalidate a specified rectangle in a window.
    {"InvalidateRgn", ui_window_invalidate_rgn, 1},  // @pymeth InvalidateRgn|Invalidate a specified region of a window.
    {"IsChild", ui_window_is_child, 1},              // @pymeth IsChild|Indicates if a window is a child.
    {"IsDlgButtonChecked", ui_window_is_dlg_button_checked,
     1},                                   // @pymeth IsDlgButtonChecked|Indicates if a dialog botton is checked.
    {"IsIconic", ui_window_is_iconic, 1},  // @pymeth IsIconic|Indicates if the window is currently minimised.
    {"IsZoomed", ui_window_is_zoomed, 1},  // @pymeth IsZoomed|Indicates if the window is currently maximised.
    {"IsWindow", ui_window_is_window,
     1},  // @pymeth IsWindow|determines whether the specified window handle identifies an existing window.
    {"IsWindowVisible", ui_window_is_window_visible,
     1},  // @pymeth IsWindowVisible|Determines if the window is currently visible.
    {"IsWindowEnabled", ui_window_is_window_enabled,
     1},                                     // @pymeth IsWindowVisible|Determines if the window is currently enabled.
    {"KillTimer", ui_window_kill_timer, 1},  // @pymeth KillTimer|Destroys a system timer
    {"LockWindowUpdate", ui_lock_window_update, 1},  // @pymeth LockWindowUpdate|Disables drawing in the given window
    {"MapWindowPoints", ui_window_map_window_points,
     1},  // @pymeth MapWindowPoints|Converts (maps) a set of points from the coordinate space of the CWnd to the
          // coordinate space of another window.
    {"MouseCaptured", ui_window_mouse_captured,
     1},  // @pymeth MouseCaptured|Indicates if the window currently has the mouse captured.
    {"MessageBox", ui_window_message_box, 1},         // @pymeth MessageBox|Displays a message box.
    {"ModifyStyle", ui_window_modify_style, 1},       // @pymeth ModifyStyle|Modifies the style of a window.
    {"ModifyStyleEx", ui_window_modify_style_ex, 1},  // @pymeth ModifyStyleEx|Modifies the style of a window.
    {"MoveWindow", ui_window_move_window, 1},         // @pymeth MoveWindow|Moves the window to a new location.
    {"OnClose", ui_window_on_close, 1},               // @pymeth OnClose|Calls the default MFC OnClose handler.
    {"OnCtlColor", ui_window_on_ctl_color, 1},        // @pymeth OnCtlColor|Calls the default MFC OnCtlColor handler.
    {"OnEraseBkgnd", ui_window_on_erase_bkgnd, 1},  // @pymeth OnEraseBkgnd|Calls the default MFC OnEraseBkgnd handler.
    {"OnNcHitTest", ui_window_on_nc_hit_test, 1},   // @pymeth OnNcHitTest|Calls the base MFC OnNcHitTest function.
    {"OnPaint", ui_window_on_paint, 1},             // @pymeth OnPaint|Calls the default MFC OnPaint handler.
    {"OnQueryDragIcon", ui_window_on_query_drag_icon,
     1},  // @pymeth OnQueryDragIcon|Calls the default MFC OnQueryDragIcon handler.
    {"OnQueryNewPalette", ui_window_on_query_new_palette,
     1},  // @pymeth OnQueryNewPalette|Calls the underlying MFC OnQueryNewPalette method.
    {"OnSetCursor", ui_window_on_set_cursor, 1},  // @pymeth OnSetCursor|Calls the default MFC OnSetCursor message
    {"OnMouseActivate", ui_window_on_mouse_activate,
     1},                                    // @pymeth OnMouseActivate|Calls the default MFC OnMouseActicate message
    {"OnWndMsg", ui_window_on_wnd_msg, 1},  // @pymeth OnWndMsg|Calls the default MFC Window Message handler.
    {"PreCreateWindow", ui_window_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"PumpWaitingMessages", ui_window_pump_waiting_messages,
     1},  // @pymeth PumpWaitingMessages|Calls the Peek/Dispatch loop on the wnd.
    {"RedrawWindow", ui_window_redraw_window,
     1},  // @pymeth RedrawWindow|Updates the specified rectangle or region in the given window's client area.
    {"ReleaseCapture", ui_window_release_capture,
     1},  // @pymeth ReleaseCapture|Releases the mouse capture for the window.
    {"ReleaseDC", ui_window_release_dc,
     1},  // @pymeth ReleaseDC|Releases a device context, freeing it for use by other applications.
    {"RepositionBars", ui_window_reposition_bars,
     1},  // @pymeth RepositionBars|Repositions the control bars for the window.
    {"RunModalLoop", ui_window_run_modal_loop, 1},  // @pymeth RunModalLoop|Starts a modal loop for the window.
    {"PostMessage", ui_window_post_message, 1},     // @pymeth PostMessage|Post a message to the window.
    {"SendMessageToDescendants", ui_window_send_message_to_desc,
     1},  // @pymeth SendMessageToDescendants|Send a message to a window's children.
    {"SendMessage", ui_window_send_message, 1},           // @pymeth SendMessage|Send a message to the window.
    {"SetActiveWindow", ui_window_set_active_window, 1},  // @pymeth SetActiveWindow|Sets the window active.
    {"SetForegroundWindow", ui_window_set_foreground_window,
     1},  // @pymeth SetForegroundWindow|Puts the window into the foreground and activates the window.
    {"SetWindowPos", ui_window_set_window_pos, 1},  // @pymeth SetWindowPos|Sets the windows position information.
    {"ScreenToClient", ui_window_screen_to_client,
     1},  // @pymeth ScreenToClient|Converts from screen coordinates to client coordinates.
    {"SetCapture", ui_window_set_capture, 1},  // @pymeth SetCapture|Captures the mouse input for thw window.
    {"SetDlgItemText", ui_window_set_dlg_item_text,
     1},  // @pymeth SetDlgItemText|Sets the text for the child window or control with the specified ID.
    {"SetFocus", ui_window_set_focus, 1},    // @pymeth SetFocus|Sets focus to the window.
    {"SetFont", ui_window_set_font, 1},      // @pymeth SetFont|Sets the window's current font to the specified font.
    {"SetIcon", ui_window_set_icon, 1},      // @pymeth SetIcon | Sets the handle to a specific icon.
    {"SetMenu", ui_window_set_menu, 1},      // @pymeth SetMenu|Sets the menu for a window.
    {"SetRedraw", ui_window_set_redraw, 1},  // @pymeth SetRedraw|Sets the redraw flag for the window.
    {"SetScrollPos", ui_window_set_scroll_pos,
     1},  // @pymeth SetScrollPos|Sets the current position of the scroll box of a scroll bar.
    {"SetScrollInfo", ui_window_set_scroll_info, 1},  // @pymeth SetScrollInfo|Set information about a scroll bar
    {"SetTimer", ui_window_set_timer, 1},             // @pymeth SetTimer|Installs a system timer
    {"SetWindowPlacement", ui_window_set_window_placement,
     1},                                              // @pymeth SetWindowPlacement|Sets the window's placement options.
    {"SetWindowText", ui_window_set_window_text, 1},  // @pymeth SetWindowText|Sets the window's text.
    {"ShowCaret", ui_window_show_caret, 1},           // @pymeth ShowCaret|Shows the caret
    {"ShowScrollBar", ui_window_show_scrollbar, 1},   // @pymeth ShowScrollBar|Shows/Hides the window's scroll bars.
    {"ShowWindow", ui_window_show_window, 1},         // @pymeth ShowWindow|Shows the window.
    {"UnlockWindowUpdate", ui_unlock_window_update,
     1},  // @pymeth UnLockWindowUpdate|Unlocks a window that was locked with LockWindowUpdate
    {"UpdateData", ui_window_update_data, 1},  // @pymeth UpdateData|Updates a windows dialog data.
    {"UpdateDialogControls", ui_window_udc,
     1},  // @pymeth UpdateDialogControls|Updates the state of dialog buttons and other controls in a dialog box or
          // window that uses the <om PyCCmdUI.HookCommandUpdate> callback mechanism.
    {"UpdateWindow", ui_window_update_window, 1},  // @pymeth UpdateWindow|Updates a window.
    {NULL, NULL}};

CString PyCWnd::repr()
{
    CString csRet;
    CString base_repr = PyCCmdTarget::repr();
    UINT_PTR numMsg = pMessageHookList ? pMessageHookList->GetCount() : 0;
    UINT_PTR numKey = pKeyHookList ? pKeyHookList->GetCount() : 0;
    TCHAR *hookStr = obKeyStrokeHandler ? _T(" (AllKeys Hook Active)") : _T("");
    csRet.Format(_T("%s, mh=%Iu, kh=%Iu%s"), (const TCHAR *)base_repr, numMsg, numKey, hookStr);
    return csRet;
}

ui_type_CObject PyCWnd::type("PyCWnd",
                             &PyCCmdTarget::type,  // @base PyCWnd|PyCCmdTarget
                             RUNTIME_CLASS(CWnd), sizeof(PyCWnd), PYOBJ_OFFSET(PyCWnd), PyCWnd_methods,
                             GET_PY_CTOR(PyCWnd));

/////////////////////////////////////////////////////////////////////
//
// Frame Window objects
//
// MDIFrameWindow is the application frame, MDIChildWindow is the child frame.
//
//////////////////////////////////////////////////////////////////////
// @pymethod <c PyCDocument>|PyCFrameWnd|GetActiveDocument|Gets the currently active document, else None
static PyObject *ui_frame_get_active_document(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS(args);
    GUI_BGN_SAVE;
    CDocument *pDoc = pFrame->GetActiveDocument();  // @pyseemfc CFrameWnd|GetActiveDocument
    GUI_END_SAVE;
    if (!pDoc)
        RETURN_NONE;
    return ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet();
}

// @pymethod <o PyFrameWnd>|win32ui|CreateFrame|Creates a Frame window.
PyObject *ui_create_frame(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, CreateFrame);
    GUI_BGN_SAVE;
    CPythonFrameWnd *pFrame = new CPythonFrameWnd;
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCFrameWnd::type, pFrame, TRUE)->GetGoodRet();
    // @rdesc The window object (not the OS window) created.  An exception is raised if an error occurs.
}

// @pymethod tuple|PyCFrameWnd|CreateWindow|Creates the actual window for the PyCFrameWnd object.
static PyObject *PyCFrameWnd_CreateWindow(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;

    PythonCreateContext cc;
    RECT rect = CFrameWnd::rectDefault;
    PyObject *obRect = Py_None;
    PyObject *obParent = Py_None;
    PyObject *obContext = Py_None;
    PyObject *obMenuID = Py_None;
    TCHAR *szClass = NULL, *szTitle = NULL, *szMenuName = NULL;
    PyObject *obClass, *obTitle, *ret = NULL;
    DWORD styleEx = 0;
    DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
    if (!PyArg_ParseTuple(
            args, "OO|lOOOOl:Create",
            &obClass,    // @pyparm string|wndClass||The window class name, or None
            &obTitle,    // @pyparm string|title||The window title
            &style,      // @pyparm int|style| WS_VISIBLE \| WS_OVERLAPPEDWINDOW|The window style
            &obRect,     // @pyparm int, int, int, int|rect|None|The default rectangle
            &obParent,   // @pyparm parent|<o PyCWnd>|None|The parent window
            &obContext,  // @pyparm tuple|createContext|None|A tuple representing a CREATECONTEXT structure.
            &obMenuID,   // @pyparm string or int|menuId||The string or integer id for the menu.
            &styleEx))   // @pyparm int|styleEx||The extended style of the window being created.
        return NULL;

    CCreateContext *pContext = NULL;
    if (obContext != Py_None) {
        cc.SetPythonObject(obContext);
        pContext = &cc;
    }

    if (obRect != Py_None) {
        if (!PyArg_ParseTuple(obRect, "iiii:RECT", &rect.left, &rect.top, &rect.right, &rect.bottom))
            return NULL;
    }

    CFrameWnd *pParent = NULL;
    if (obParent != Py_None) {
        pParent = GetFramePtr(obParent);
        if (pParent == NULL)
            RETURN_TYPE_ERR("The parent window is not a valid PyFrameWnd");
    }

    if (PyWinObject_AsTCHAR(obClass, &szClass, TRUE) && PyWinObject_AsTCHAR(obTitle, &szTitle, FALSE) &&
        PyWinObject_AsResourceId(obMenuID, &szMenuName, TRUE)) {
        BOOL ok;
        GUI_BGN_SAVE;
        // @pyseemfc CFrameWnd|Create
        ok = pFrame->Create(szClass, szTitle, style, rect, pParent, szMenuName, styleEx, pContext);
        GUI_END_SAVE;
        if (!ok)
            PyErr_SetString(ui_module_error, "CFrameWnd::Create failed");
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    PyWinObject_FreeTCHAR(szClass);
    PyWinObject_FreeTCHAR(szTitle);
    PyWinObject_FreeResourceId(szMenuName);
    return ret;
}

// @pymethod tuple|PyCFrameWnd|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
// @xref <vm PyCWnd.PreCreateWindow>
static PyObject *PyCFrameWnd_pre_create_window(PyObject *self, PyObject *args)
{
    class CProtectedFrameWnd : public CFrameWnd {
       public:
        BOOL PreCreateWindowBase(CREATESTRUCT &cs) { return CFrameWnd::PreCreateWindow(cs); }
    };

    CFrameWnd *pFrame = (CFrameWnd *)GetFramePtr(self);
    if (!pFrame)
        return NULL;

    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = ((CProtectedFrameWnd *)pFrame)->PreCreateWindowBase(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CFrameWnd::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod |PyCFrameWnd|LoadAccelTable|Loads an accelerator table.
static PyObject *PyCFrameWnd_LoadAccelTable(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *obID;
    if (!PyArg_ParseTuple(args, "O", &obID))
        return NULL;
    // @pyparm <o PyResourceId>|id||Name or id of the resource that contains the table
    TCHAR *res;
    if (!PyWinObject_AsResourceId(obID, &res, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pFrame->LoadAccelTable(res);
    GUI_END_SAVE;
    PyWinObject_FreeResourceId(res);
    if (!ok)
        RETURN_ERR("LoadAccelTable failed");
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|LoadFrame|Loads a Windows frame window and associated resources
static PyObject *ui_frame_load_frame(PyObject *self, PyObject *args)
{
    int idResource = IDR_PYTHONTYPE;
    long style = -1;
    CPythonMDIChildWnd *pFrame = GetPythonFrame(self);
    BOOL bMakeVisible = TRUE;
    if (!pFrame)
        return NULL;
    if (pFrame->m_hWnd != NULL)
        RETURN_ERR("The frame already has a window");
    PythonCreateContext cc;
    PyObject *wndParent = Py_None;
    PyObject *contextObject = Py_None;
    if (!PyArg_ParseTuple(
            args, "|ilOO:LoadFrame",
            &idResource,  // @pyparm int|idResource|IDR_PYTHONTYPE|The Id of the resources (menu, icon, etc) for this
                          // window
            &style,       // @pyparm long|style|-1|The window style.  Note -1 implies
                          // win32con.WS_OVERLAPPEDWINDOW\|win32con.FWS_ADDTOTITLE
            &wndParent,   // @pyparm <o PyCWnd>|wndParent|None|The parent of the window, or None.
            &contextObject))  // @pyparm object|context|None|An object passed to the OnCreateClient for the frame,
        return NULL;
    if (style == -1)
        style = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE;  // default frame styles;
    CWnd *pParent = NULL;
    if (wndParent != Py_None) {
        pParent = GetWndPtrFromParam(wndParent, PyCWnd::type);
        if (pParent == NULL)
            return NULL;
    }
    cc.SetPythonObject(contextObject);
    //	cc. = idResource
    // OnCreateClient will be called during this!
    CProtectedWinApp *pApp = GetProtectedApp();
    if (!pApp)
        return NULL;
    BOOL ok;
    CWnd *pMain = pApp->GetMainFrame();
    if (pMain == NULL)
        RETURN_ERR("There is no main application frame - an MDI child can not be created.");
    if (!pMain->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)))
        RETURN_ERR("There is no MDI Frame Window available - an MDI child can not be created.");

    GUI_BGN_SAVE;
    ok = pFrame->LoadFrame(idResource, style, pParent, &cc);  // @pyseemfc CFrameWnd|LoadFrame
    GUI_END_SAVE;
    if (!ok) {
        RETURN_ERR("LoadFrame failed\n");
        // frame will be deleted in PostNcDestroy cleanup
    }
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|RecalcLayout|Called by the framework when the standard control bars are toggled on or off or
// when the frame window is resized.
static PyObject *ui_frame_recalc_layout(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    BOOL bNotify = TRUE;
    if (!PyArg_ParseTuple(args, "|i:RecalcLayout",
                          &bNotify))  // @pyparm int|bNotify|1|Notify flag
        return NULL;
    GUI_BGN_SAVE;
    pFrame->RecalcLayout(bNotify);  // @pyseemfc CFrameWnd|RecalcLayout
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCFrameWnd|EnableDocking|Enable dockable control bars in a frame window
PyObject *PyCFrameWnd_EnableDocking(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    int style;
    // @pyparm int|style||Specifies which sides of the frame window can serve as docking sites for control bars.
    if (!PyArg_ParseTuple(args, "i:EnableDocking", &style))
        return NULL;
    GUI_BGN_SAVE;
    pFrame->EnableDocking(style);
    GUI_END_SAVE;
    RETURN_NONE;
    // @comm By default, control bars will be docked to a side of the frame window in the following order: top, bottom,
    // left, right.
}

// @pymethod |PyCFrameWnd|DockControlBar|Docks a control bar.
static PyObject *PyCFrameWnd_DockControlBar(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *ob;
    int docBarId = 0;
    CRect rect(0, 0, 0, 0);
    if (!PyArg_ParseTuple(
            args, "O|i(iiii):DockControlBar",
            &ob,        // @pyparm <o PyCControlBar>|controlBar||The control bar to dock.
            &docBarId,  // @pyparm int|dockBarId|0|Determines which sides of the frame window to consider for docking.
            // @pyparm left, top, right, bottom|int, int, int, int|0,0,0,0|Determines, in screen coordinates, where the
            // control bar will be docked in the nonclient area of the destination frame window.
            &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;
    CControlBar *pControlBar = PyCControlBar::GetControlBar(ob);
    if (pControlBar == NULL)
        return NULL;
    CRect *pRect = (rect.left == rect.right == 0) ? NULL : &rect;
    PyObject *rc;
    GUI_BGN_SAVE;
    __try {
        pFrame->DockControlBar(pControlBar, docBarId, pRect);  // @pyseemfc CFrameWnd|DockControlBar
        rc = Py_None;
        Py_INCREF(Py_None);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        rc = NULL;  // Can't set Python error till we have the lock back.
    }
    GUI_END_SAVE;
    if (rc == NULL)
        PyErr_SetString(ui_module_error, "DockControlBar caused exception.");

    return rc;
}

// @pymethod |PyCFrameWnd|FloatControlBar|Floats a control bar.
static PyObject *PyCFrameWnd_FloatControlBar(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *ob;
    int style = CBRS_ALIGN_TOP;
    CPoint pt;
    if (!PyArg_ParseTuple(args, "O(ii)|i:FloatControlBar",
                          &ob,           // @pyparm <o PyCControlBar>|controlBar||The control bar to dock.
                          &pt.x, &pt.y,  // @pyparm x,y|int, int||The location, in screen coordinates, where the top
                                         // left corner of the control bar will be placed.
                          &style))  // @pyparm int|style|CBRS_ALIGN_TOP|Determines which sides of the frame window to
                                    // consider for docking.
        return NULL;
    CControlBar *pControlBar = PyCControlBar::GetControlBar(ob);
    if (pControlBar == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pFrame->FloatControlBar(pControlBar, pt, style);  // @pyseemfc CFrameWnd|FloatControlBar
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCFrameWnd|ShowControlBar|Shows a control bar.
static PyObject *PyCFrameWnd_ShowControlBar(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *ob;
    BOOL bShow, bDelay;
    if (!PyArg_ParseTuple(args, "Oii:ShowControlBar",
                          &ob,       // @pyparm <o PyCControlBar>|controlBar||The control bar to dock.
                          &bShow,    // @pyparm int|bShow||Show or hide flag.
                          &bDelay))  // @pyparm int|bDelay||If TRUE, delay showing the control bar. If FALSE, show the
                                     // control bar immediately.

        return NULL;
    CControlBar *pControlBar = PyCControlBar::GetControlBar(ob);
    if (pControlBar == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pFrame->ShowControlBar(pControlBar, bShow, bDelay);  // @pyseemfc CFrameWnd|ShowControlBar
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|SaveBarState|Saves a control bars settings
static PyObject *PyCFrameWnd_SaveBarState(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    TCHAR *profileName;
    PyObject *obprofileName;
    if (!PyArg_ParseTuple(args, "O:SaveBarState",
                          &obprofileName))  // @pyparm string|profileName||Name of a section in the initialization file
                                            // or a key in the Windows registry where state information is stored.
        return NULL;
    if (!PyWinObject_AsTCHAR(obprofileName, &profileName, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pFrame->SaveBarState(profileName);  // @pyseemfc CFrameWnd|SaveBarState
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(profileName);
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|LoadBarState|Loads a control bars settings
static PyObject *PyCFrameWnd_LoadBarState(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    TCHAR *profileName;
    PyObject *obprofileName;
    if (!PyArg_ParseTuple(args, "O:LoadBarState",
                          &obprofileName))  // @pyparm string|profileName||Name of a section in the initialization file
                                            // or a key in the Windows registry where state information is stored.
        return NULL;
    if (!PyWinObject_AsTCHAR(obprofileName, &profileName, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    PYWINTYPES_TRY
    {
        pFrame->LoadBarState(profileName);  // @pyseemfc CFrameWnd|LoadBarState
    }
    PYWINTYPES_EXCEPT
    {
        GUI_BLOCK_THREADS;
        PyWinObject_FreeTCHAR(profileName);
        RETURN_ERR("LoadBarState failed (with win32 exception!)");
    }
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(profileName);
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|BeginModalState|Sets the frame window to modal.
static PyObject *PyCFrameWnd_BeginModalState(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, BeginModalState);
    GUI_BGN_SAVE;
    pFrame->BeginModalState();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCFrameWnd|EndModalState|Ends the frame window's modal state. Enables all of the windows disabled by <om
// PyCFrameWnd.BeginModalState>.
static PyObject *PyCFrameWnd_EndModalState(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, EndModalState);
    GUI_BGN_SAVE;
    pFrame->EndModalState();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCFrameWnd|InModalState|Returns a value indicating whether or not a frame window is in a modal state.
static PyObject *PyCFrameWnd_InModalState(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, InModalState);
    GUI_BGN_SAVE;
    int rc = pFrame->InModalState();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCFrameWnd|IsTracking|Determines if splitter bar is currently being moved.
static PyObject *PyCFrameWnd_IsTracking(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, EndModalState);
    GUI_BGN_SAVE;
    int rc = pFrame->IsTracking();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod string|PyCFrameWnd|GetMessageString|Retrieves message corresponding to a command ID.
static PyObject *PyCFrameWnd_GetMessageString(PyObject *self, PyObject *args)
{
    int id;
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &id))  // @pyparm int|id||The ID to be retrieved
        return NULL;
    CString csRet;
    // @xref <vm PyCMDIChildWnd.GetMessageString>
    GUI_BGN_SAVE;
    pFrame->CFrameWnd::GetMessageString(id, csRet);
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(csRet);
}

// @pymethod <o PyCControlBar>|PyCFrameWnd|GetControlBar|Retrieves the specified control bar.
static PyObject *PyCFrameWnd_GetControlBar(PyObject *self, PyObject *args)
{
    int id;
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &id))  // @pyparm int|id||The ID of the toolbar to be retrieved
        return NULL;
    GUI_BGN_SAVE;
    CControlBar *pRet = pFrame->GetControlBar(id);
    GUI_END_SAVE;
    if (pRet == NULL)
        RETURN_ERR("There is no control bar with that ID");
    return ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
}
// @pymethod <o PyCWnd>|PyCFrameWnd|GetMessageBar|Retrieves the message bar for the frame.
static PyObject *PyCFrameWnd_GetMessageBar(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, GetMessageBar);
    GUI_BGN_SAVE;
    CWnd *pRet = pFrame->GetMessageBar();
    GUI_END_SAVE;
    if (pRet == NULL)
        RETURN_ERR("There is no message bar.");
    return ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
}

// @pymethod string|PyCFrameWnd|ShowOwnedWindows|Shows all windows that are descendants of the <o PyCFrameWnd> object.
static PyObject *PyCFrameWnd_ShowOwnedWindows(PyObject *self, PyObject *args)
{
    BOOL bShow = TRUE;
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    if (!PyArg_ParseTuple(args, "|i", &bShow))  // @pyparm int|bShow|1|Flag
        return NULL;
    CString csRet;
    GUI_BGN_SAVE;
    pFrame->CFrameWnd::ShowOwnedWindows(bShow);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCFrameWnd|SetActiveView|Sets the active view for a frame.
static PyObject *PyCFrameWnd_SetActiveView(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *ob;
    BOOL bNotify = TRUE;
    if (!PyArg_ParseTuple(
            args, "O|i:SetActiveView",
            &ob,        // @pyparm <o PyCView>|view||The view to set active.
            &bNotify))  // @pyparm int|bNotify|1|Specifies whether the view is to be notified of activation. If TRUE,
                        // OnActivateView is called for the new view; if FALSE, it is not.
        return NULL;
    CView *pView = NULL;
    if (ob != Py_None) {
        pView = PyCView::GetViewPtr(ob);
        if (pView == NULL)
            return NULL;
    }
    GUI_BGN_SAVE;
    pFrame->SetActiveView(pView, bNotify);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCView>|PyCFrameWnd|GetActiveView|Retrieves the active view.
static PyObject *PyCFrameWnd_GetActiveView(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, GetActiveView);
    GUI_BGN_SAVE;
    CView *pRet = pFrame->GetActiveView();
    GUI_END_SAVE;
    if (pRet == NULL)
        RETURN_ERR("There is no active view.");
    return ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
}

// @pymethod int|PyCFrameWnd|OnBarCheck|Changes the state of the specified controlbar.
static PyObject *PyCFrameWnd_OnBarCheck(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    int id;
    if (!PyArg_ParseTuple(args, "i:OnBarCheck", &id))  // @pyparm int|id||The control ID of the control bar.
        return NULL;
    GUI_BGN_SAVE;
    long rc = pFrame->OnBarCheck(id);
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod int|PyCFrameWnd|OnUpdateControlBarMenu|Checks the state of a menu item
static PyObject *PyCFrameWnd_OnUpdateControlBarMenu(PyObject *self, PyObject *args)
{
    CFrameWnd *pFrame = GetFramePtr(self);
    if (!pFrame)
        return NULL;
    PyObject *obCU;
    if (!PyArg_ParseTuple(args, "O:OnUpdateControlBarMenu", &obCU))  // @pyparm <o PyCCmdUI>|cmdUI||A cmdui object
        return NULL;
    CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(obCU);
    if (pCU == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pFrame->OnUpdateControlBarMenu(pCU);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCFrameWnd|A windows frame window.  Encapsulates an MFC <c CFrameWnd> class.  Derived from a <o PyCWnd>
// object.
static struct PyMethodDef PyCFrameWnd_methods[] = {
    {"BeginModalState", PyCFrameWnd_BeginModalState, 1},  // @pymeth BeginModalState|Sets the frame window to modal.
    {"CreateWindow", PyCFrameWnd_CreateWindow,
     1},  // @pymeth CreateWindow|Creates the underlying window for the object.
    {"EndModalState", PyCFrameWnd_EndModalState,
     1},  // @pymeth EndModalState|Ends the frame window's modal state. Enables all of the windows disabled by <om
          // PyCFrameWnd.BeginModalState>.
    {"DockControlBar", PyCFrameWnd_DockControlBar, 1},  // @pymeth DockControlBar|Docks a control bar.
    {"EnableDocking", PyCFrameWnd_EnableDocking,
     1},  // @pymeth EnableDocking|Enable dockable control bars in a frame window
    {"FloatControlBar", PyCFrameWnd_FloatControlBar, 1},  // @pymeth FloatControlBar|Floats a control bar.
    {"GetActiveDocument", ui_frame_get_active_document,
     1},                                              // @pymeth GetActiveDocument|Returns the currently active document
    {"GetControlBar", PyCFrameWnd_GetControlBar, 1},  // @pymeth GetControlBar|Retrieves the specified control bar.
    {"GetMessageString", PyCFrameWnd_GetMessageString,
     1},  // @pymeth GetMessageString|Retrieves message corresponding to a command ID.
    {"GetMessageBar", PyCFrameWnd_GetMessageBar, 1},  // @pymeth GetMessageBar|Retrieves the message bar for the frame.
    {"IsTracking", PyCFrameWnd_IsTracking,
     1},  // @pymeth IsTracking|Determines if splitter bar is currently being moved.
    {"InModalState", PyCFrameWnd_InModalState,
     1},  // @pymeth InModalState|Returns a value indicating whether or not a frame window is in a modal state.
    {"LoadAccelTable", PyCFrameWnd_LoadAccelTable, 1},  // @pymeth LoadAccelTable|Loads an accelerator table.
    {"LoadFrame", ui_frame_load_frame, 1},              // @pymeth LoadFrame|Creates the MDI Window's frame
    {"LoadBarState", PyCFrameWnd_LoadBarState, 1},      // @pymeth LoadBarState|Loads a control bars settings
    {"PreCreateWindow", PyCFrameWnd_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"SaveBarState", PyCFrameWnd_SaveBarState, 1},      // @pymeth SaveBarState|Saves a control bars settings
    {"ShowControlBar", PyCFrameWnd_ShowControlBar, 1},  // @pymeth ShowControlBar|Shows a control bar.
    {"RecalcLayout", ui_frame_recalc_layout,
     1},  // @pymeth RecalcLayout|Called by the framework when the standard control bars are toggled on or off or when
          // the frame window is resized.
    {"GetActiveView", PyCFrameWnd_GetActiveView, 1},  // @pymeth GetActiveView|Retrieves the active view.
    {"OnBarCheck", PyCFrameWnd_OnBarCheck, 1},  // @pymeth OnBarCheck|Changes the state of the specified controlbar.
    {"OnUpdateControlBarMenu", PyCFrameWnd_OnUpdateControlBarMenu,
     1},                                              // @pymeth OnUpdateControlBarMenu|Checks the state of a menu item
    {"SetActiveView", PyCFrameWnd_SetActiveView, 1},  // @pymeth SetActiveView|Sets the active view for a frame.
    {NULL, NULL}};

ui_type_CObject PyCFrameWnd::type("PyCFrameWnd",
                                  &PyCWnd::type,  // @base PyCFrameWnd|PyCWnd
                                  RUNTIME_CLASS(CFrameWnd), sizeof(PyCFrameWnd), PYOBJ_OFFSET(PyCFrameWnd),
                                  PyCFrameWnd_methods, GET_PY_CTOR(PyCFrameWnd));

// @pymethod tuple|PyCMDIFrameWnd|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
static PyObject *ui_mdi_frame_window_pre_create_window(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pWnd = GetMDIFrame(self);
    if (!pWnd)
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    // @xref <vm PyCWnd.PreCreateWindow>
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pWnd->CMDIFrameWnd::PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CMDIFrameWnd::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod None|PyCMDIFrameWnd|OnContextHelp|Calls the underlying MFC OnContextHelp method.
PyObject *ui_mdi_frame_on_context_help(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pWnd = GetMDIFrame(self);
    if (!pWnd)
        return NULL;
    GUI_BGN_SAVE;
    pWnd->CMDIFrameWnd::OnContextHelp();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod (<o PyCMDIChildWnd>, int)|PyCMDIFrameWnd|MDIGetActive|Retrieves the current active MDI child window, along
// with a flag indicating whether the child window is maximized.
static PyObject *ui_mdi_frame_window_mdi_get_active(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pFrame = GetMDIFrame(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, MDIGetActive);
    BOOL bIsMax;
    GUI_BGN_SAVE;
    CMDIChildWnd *pRet = pFrame->MDIGetActive(&bIsMax);
    GUI_END_SAVE;
    if (pRet == NULL)
        RETURN_ERR("There is no active window.");
    PyObject *obui = ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
    if (obui == NULL)
        return NULL;
    PyObject *ret = Py_BuildValue("Oi", obui, bIsMax);
    Py_XDECREF(obui);
    return ret;
}

// @pymethod |PyCMDIFrameWnd|MDINext|Activates the next MDI window
static PyObject *ui_mdi_frame_window_mdi_next(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pFrame = GetMDIFrame(self);
    if (!pFrame)
        return NULL;
    // @comm Unlike MFC, this version supports the fNext param in the WM_MDINEXT message.
    // @pyparm int|fNext|0|Indicates if the next (0) or previous (non-zero) window is requested.
    int fnext = 0;
    if (!PyArg_ParseTuple(args, "i", &fnext))
        return NULL;
    GUI_BGN_SAVE;
    ::SendMessage(pFrame->m_hWndMDIClient, WM_MDINEXT, 0, fnext);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCMDIFrameWnd>|PyCMDIFrameWnd|GetMDIClient|Returns the MDI client window
static PyObject *ui_mdi_frame_get_mdi_client(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pFrame = GetMDIFrame(self);
    if (!pFrame)
        return NULL;
    CHECK_NO_ARGS2(args, GetMDIClient);

    if (!pFrame->m_hWndMDIClient)
        RETURN_ERR("MDIGetClient call but MDIFrameWnd has not been created");

    CWnd *pWnd = CWnd::FromHandle(pFrame->m_hWndMDIClient);
    if (pWnd == NULL)
        RETURN_ERR("The window handle is invalid.");
    return PyCWnd::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}

// @pymethod <o PyCMDIFrameWnd>|PyCMDIFrameWnd|MDIActivate|Activate an MDI child window
static PyObject *ui_mdi_frame_mdi_activate(PyObject *self, PyObject *args)
{
    CMDIFrameWnd *pFrame = GetMDIFrame(self);
    if (!pFrame)
        return NULL;

    PyObject *ob;
    if (!PyArg_ParseTuple(args, "O:MDIActivate",
                          &ob))  // @pyparm <o PyCWnd>|window||The window to activate.
        return NULL;

    CWnd *pWndActivate = GetWndPtr(ob);
    if (!pWndActivate)
        RETURN_ERR("Argument is not a valid PyCWnd");

    GUI_BGN_SAVE;
    pFrame->MDIActivate(pWndActivate);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCMDIFrameWnd|PreTranslateMessage|Calls the base PreTranslateMessage handler
static PyObject *ui_mdi_frame_pre_translate_message(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.PreTranslateMessage>
    CMDIFrameWnd *pFrame = GetMDIFrame(self);
    if (!pFrame)
        return NULL;
    if (!pFrame->IsKindOf(RUNTIME_CLASS(CPythonMDIFrameWnd)))
        RETURN_TYPE_ERR("The frame window is not a Python frame");
    CPythonMDIFrameWnd *pWnd = (CPythonMDIFrameWnd *)pFrame;
    MSG _msg;
    MSG *msg = &_msg;
    if (!PyArg_ParseTuple(args, "(iiiii(ii))", &msg->hwnd, &msg->message, &msg->wParam, &msg->lParam, &msg->time,
                          &msg->pt.x, &msg->pt.y))
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pWnd->_BasePreTranslateMessage(msg);
    GUI_END_SAVE;
    return Py_BuildValue("i(iiiii(ii))", rc, msg->hwnd, msg->message, msg->wParam, msg->lParam, msg->time, msg->pt.x,
                         msg->pt.y);
}

// @pymethod |PyCMDIFrameWnd|OnCommand|Calls the standard Python framework OnCommand handler
static PyObject *ui_mdi_frame_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonMDIFrameWnd, &PyCMDIFrameWnd::type, self, args);
}

// @pymethod |PyCMDIFrameWnd|OnClose|Calls the standard Python framework OnClose handler
static PyObject *ui_mdi_frame_on_close(PyObject *self, PyObject *args)
{
    return DoBaseOnClose(CPythonMDIFrameWnd, &PyCMDIFrameWnd::type, self, args);
}

// @object PyCMDIFrameWnd|A main application frame window.  Encapsulates an MFC <c CMDIFrameWnd> class
static struct PyMethodDef PyCMDIFrameWnd_methods[] = {
    {"GetMDIClient", ui_mdi_frame_get_mdi_client, 1},  // @pymeth GetMDIClient|Returns the MDI client window
    {"MDIGetActive", ui_mdi_frame_window_mdi_get_active,
     1},  // @pymeth MDIGetActive|Retrieves the current active MDI child window, along with a flag indicating whether
          // the child window is maximized.
    {"MDIActivate", ui_mdi_frame_mdi_activate, 1},  // @pymeth MDIActivate|Activate an MDI child window
    {"MDINext", ui_mdi_frame_window_mdi_next, 1},   // @pymeth MDINext|Activates the next MDI window
    {"PreCreateWindow", ui_mdi_frame_window_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"PreTranslateMessage", ui_mdi_frame_pre_translate_message,
     1},  // @pymeth PreTranslateMessage|Calls the underlying MFC PreTranslateMessage method.
    {"OnCommand", ui_mdi_frame_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {"OnContextHelp", ui_mdi_frame_on_context_help,
     1},                                    // @pymeth OnContextHelp|Calls the underlying MFC OnContextHelp method.
    {"OnClose", ui_mdi_frame_on_close, 1},  // @pymeth OnClose|Calls the standard Python framework OnClose handler
    {NULL, NULL}};

ui_type_CObject PyCMDIFrameWnd::type("PyCMDIFrameWnd",
                                     &PyCFrameWnd::type,  // @base PyCMDIFrameWnd|PyCFrameWnd
                                     RUNTIME_CLASS(CMDIFrameWnd), sizeof(PyCMDIFrameWnd), PYOBJ_OFFSET(PyCMDIFrameWnd),
                                     PyCMDIFrameWnd_methods, GET_PY_CTOR(PyCMDIFrameWnd));

// @pymethod |PyCMDIChildWnd|ActivateFrame|Calls the underlying MFC ActivateFrame method.
static PyObject *ui_mdi_child_window_activate_frame(PyObject *self, PyObject *args)
{
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    int cmdShow = -1;
    //@pyparm int|cmdShow|-1|The status of the window.
    if (!PyArg_ParseTuple(args, "|i:ActivateFrame", &cmdShow))
        return NULL;
    // @xref <vm PyCMDIChildWnd.ActivateFrame>
    GUI_BGN_SAVE;
    pWnd->CMDIChildWnd::ActivateFrame(cmdShow);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod tuple|PyCMDIChildWnd|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
static PyObject *ui_mdi_child_window_pre_create_window(PyObject *self, PyObject *args)
{
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    // @xref <vm PyCWnd.PreCreateWindow>
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pWnd->CMDIChildWnd::PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CMDIChildWnd::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod tuple|PyCMDIChildWnd|CreateWindow|Creates the actual window for the PyCWnd object.
// @comm You do not need to call this method if you use the MFC Document/View framework.
static PyObject *ui_mdi_child_window_create_window(PyObject *self, PyObject *args)
{
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    PythonCreateContext cc;
    RECT rect = CMDIChildWnd::rectDefault;
    PyObject *obRect = Py_None;
    PyObject *obParent = Py_None;
    PyObject *obContext = Py_None;
    TCHAR *szClass = NULL, *szTitle = NULL;
    PyObject *obClass, *obTitle;
    DWORD style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
    if (!PyArg_ParseTuple(
            args, "OO|lOOO:CreateWindow",
            &obClass,     // @pyparm string|wndClass||The window class name, or None
            &obTitle,     // @pyparm string|title||The window title
            &style,       // @pyparm int|style|WS_CHILD \| WS_VISIBLE \| WS_OVERLAPPEDWINDOW|The window style
            &obRect,      // @pyparm int, int, int, int|rect|None|The default rectangle
            &obParent,    // @pyparm parent|<o PyCWnd>|None|The parent window
            &obContext))  // @pyparm tuple|createContext|None|A tuple representing a CREATECONTEXT structure.
        return NULL;

    CCreateContext *pContext = NULL;
    if (obContext != Py_None) {
        cc.SetPythonObject(obContext);
        pContext = &cc;
    }
    if (obRect != Py_None) {
        if (!PyArg_ParseTuple(obRect, "iiii:RECT", &rect.left, &rect.top, &rect.right, &rect.bottom))
            return NULL;
    }
    CMDIFrameWnd *pParent = NULL;
    if (obParent != Py_None) {
        pParent = GetMDIFrame(obParent);
        if (pParent == NULL)
            RETURN_TYPE_ERR("The parent window is not a valid PyCMDIFrameWnd");
    }

    if (!PyWinObject_AsTCHAR(obClass, &szClass, TRUE))
        return NULL;
    if (!PyWinObject_AsTCHAR(obTitle, &szTitle, FALSE)) {
        PyWinObject_FreeTCHAR(szClass);
        return NULL;
    }

    GUI_BGN_SAVE;
    BOOL ok = pWnd->Create(szClass, szTitle, style, rect, pParent, pContext);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szClass);
    PyWinObject_FreeTCHAR(szTitle);
    if (!ok)
        RETURN_ERR("CMDIChildWnd::Create");
    RETURN_NONE;
}

// @pymethod |PyCMDIChildWnd|GetMDIFrame|Returns the MDI parent frame
static PyObject *ui_mdi_child_window_get_mdi_frame(PyObject *self, PyObject *args)
{
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    CHECK_NO_ARGS2(args, GetMDIFrame);

    GUI_BGN_SAVE;
    CMDIFrameWnd *pFrame = pWnd->GetMDIFrame();
    GUI_END_SAVE;

    return ui_assoc_object::make(UITypeFromCObject(pFrame), pFrame)->GetGoodRet();
}

// @pymethod |PyCMDIChildWnd|MDIActivate|Activates the MDI frame independent of the main frame.
static PyObject *ui_mdi_child_window_mdi_activate_frame(PyObject *self, PyObject *args)
{
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    //@pyparm int|cmdShow|-1|The status of the window.
    if (!PyArg_ParseTuple(args, ":MDIActivate"))
        return NULL;
    // @xref <vm PyCWnd.OnMDIActivate>
    GUI_BGN_SAVE;
    pWnd->MDIActivate();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCMDIChildWnd|PreTranslateMessage|Calls the base PreTranslateMessage handler
static PyObject *ui_mdi_child_window_pre_translate_message(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.PreTranslateMessage>
    CPythonMDIChildWnd *pWnd = GetPythonFrame(self);
    if (!pWnd)
        return NULL;
    MSG _msg;
    MSG *msg = &_msg;
    if (!PyArg_ParseTuple(args, "(iiiii(ii))", &msg->hwnd, &msg->message, &msg->wParam, &msg->lParam, &msg->time,
                          &msg->pt.x, &msg->pt.y))
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pWnd->_BasePreTranslateMessage(msg);
    GUI_END_SAVE;
    return Py_BuildValue("i(iiiii(ii))", rc, msg->hwnd, msg->message, msg->wParam, msg->lParam, msg->time, msg->pt.x,
                         msg->pt.y);
}

// @pymethod |PyCMDIChildWnd|OnCommand|Calls the standard Python framework OnCommand handler
static PyObject *ui_mdi_child_window_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonMDIChildWnd, &PyCMDIChildWnd::type, self, args);
}

// @pymethod |PyCMDIChildWnd|OnClose|Calls the standard Python framework OnClose handler
static PyObject *ui_mdi_child_window_on_close(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnClose>
    return DoBaseOnClose(CPythonMDIChildWnd, &PyCMDIChildWnd::type, self, args);
}

// @object PyCMDIChildWnd|A windows frame window.  Encapsulates an MFC <c CMDIChildWindow> class
static struct PyMethodDef PyCMDIChildWnd_methods[] = {
    {"ActivateFrame", ui_mdi_child_window_activate_frame,
     1},  // @pymeth ActivateFrame|Calls the underlying MFC ActivateFrame method.
    {"CreateWindow", ui_mdi_child_window_create_window,
     1},  // @pymeth CreateWindow|Creates the actual window for the PyCWnd object.
    {"GetMDIFrame", ui_mdi_child_window_get_mdi_frame, 1},  // @pymeth GetMDIFrame|Returns the MDI parent frame
    {"MDIActivate", ui_mdi_child_window_mdi_activate_frame,
     1},  // @pymeth MDIActivate|Activates the MDI frame independent of the main frame.
    {"PreCreateWindow", ui_mdi_child_window_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"PreTranslateMessage", ui_mdi_child_window_pre_translate_message,
     1},  // @pymeth PreTranslateMessage|Calls the underlying MFC PreTranslateMessage method.
    {"OnCommand", ui_mdi_child_window_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {"OnClose", ui_mdi_child_window_on_close,
     1},  // @pymeth OnClose|Calls the standard Python framework OnClose handler
    {NULL, NULL}};

ui_type_CObject PyCMDIChildWnd::type("PyCMDIChildWnd",
                                     &PyCFrameWnd::type,  // @base PyCMDIChildWnd|PyCFrameWnd
                                     RUNTIME_CLASS(CMDIChildWnd), sizeof(PyCMDIChildWnd), PYOBJ_OFFSET(PyCMDIChildWnd),
                                     PyCMDIChildWnd_methods, GET_PY_CTOR(PyCMDIChildWnd));
