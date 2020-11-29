/*
  long awaited python toolbar class

  by Dave Brennan (brennan@hal.com)

  (Actually, is now all the win32ui supported PyCControlBar
  derived classes!!)

  Portions contributed by Kleanthis Kleanthous (kk@epsilon.com.gr)

  ToolbarCtrl contributed by Scott Deerwester (scott@HK.Super.NET)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/

#include "stdafx.h"
#include "win32win.h"
#include "win32toolbar.h"

#include "win32control.h"  // For tooltips.
#include "win32gdi.h"
#include "win32bitmap.h"
#include "pythoncbar.h"

class PyCDockContext : public ui_assoc_object {
   public:
    static ui_type type;
    PyCDockContext() { ; }
    static CDockContext *GetDockContext(PyObject *);
    MAKE_PY_CTOR(PyCDockContext);
    virtual PyObject *getattro(PyObject *obname);
    virtual int setattro(PyObject *obname, PyObject *v);

   protected:
    virtual ~PyCDockContext() { ; }
};

enum MyTypes {
    MT_INT,
    MT_RECT,
    MT_SIZE,
    MT_POINT,
};
struct MyMemberList {
    const char *name;
    MyTypes type;
    size_t off;
};

CDockContext *PyCDockContext::GetDockContext(PyObject *self) { return (CDockContext *)GetGoodCppObject(self, &type); }

// @pymethod int|PyCDockContext|StartDrag|
PyObject *PyCDockContext_StartDrag(PyObject *self, PyObject *args)
{
    CDockContext *pC = PyCDockContext::GetDockContext(self);
    if (!pC)
        return NULL;
    CPoint pt;
    // @pyparm int, int|pt||
    if (!PyArg_ParseTuple(args, "(ii)", &pt.x, &pt.y))
        return NULL;
    GUI_BGN_SAVE;
    pC->StartDrag(pt);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCDockContext|EndDrag|
PyObject *PyCDockContext_EndDrag(PyObject *self, PyObject *args)
{
    CDockContext *pC = PyCDockContext::GetDockContext(self);
    if (!pC)
        return NULL;
    CHECK_NO_ARGS2(args, "EndDrag");
    GUI_BGN_SAVE;
    pC->EndDrag();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCDockContext|StartResize|
PyObject *PyCDockContext_StartResize(PyObject *self, PyObject *args)
{
    CDockContext *pC = PyCDockContext::GetDockContext(self);
    if (!pC)
        return NULL;
    CPoint pt;
    int hittest;
    // @pyparm int|hittest||
    // @pyparm int, int|pt||
    if (!PyArg_ParseTuple(args, "i(ii)", &hittest, &pt.x, &pt.y))
        return NULL;
    GUI_BGN_SAVE;
    pC->StartResize(hittest, pt);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCDockContext|EndResize|
PyObject *PyCDockContext_EndResize(PyObject *self, PyObject *args)
{
    CDockContext *pC = PyCDockContext::GetDockContext(self);
    if (!pC)
        return NULL;
    CHECK_NO_ARGS2(args, EndResize);
    GUI_BGN_SAVE;
    pC->EndResize();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCDockContext|ToggleDocking|
PyObject *PyCDockContext_ToggleDocking(PyObject *self, PyObject *args)
{
    CDockContext *pC = PyCDockContext::GetDockContext(self);
    if (!pC)
        return NULL;
    CHECK_NO_ARGS2(args, ToggleDocking);
    GUI_BGN_SAVE;
    pC->ToggleDocking();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCDockContext|A class which encapsulates an MFC CDockContext object
static struct PyMethodDef PyCDockContext_methods[] = {
    {"EndDrag", PyCDockContext_EndDrag, 1},              // @pymeth EndDrag|
    {"StartDrag", PyCDockContext_StartDrag, 1},          // @pymeth StartDrag|
    {"EndResize", PyCDockContext_EndResize, 1},          // @pymeth EndResize|
    {"StartResize", PyCDockContext_StartResize, 1},      // @pymeth StartResize|
    {"ToggleDocking", PyCDockContext_ToggleDocking, 1},  // @pymeth ToggleDocking|
    {NULL, NULL}};

#define OFF(e) offsetof(CDockContext, e)
struct MyMemberList dcmembers[] = {
    {"ptLast", MT_POINT, OFF(m_ptLast)},                       // @prop x,y|ptLast|
    {"rectLast", MT_RECT, OFF(m_rectLast)},                    // @prop left, top, right, bottom|rectLast|
    {"sizeLast", MT_SIZE, OFF(m_sizeLast)},                    // @prop cx, cy|sizeLast|
    {"bDitherLast", MT_INT, OFF(m_bDitherLast)},               // @prop int|bDitherLast|
    {"rectDragHorz", MT_RECT, OFF(m_rectDragHorz)},            // @prop left, top, right, bottom|rectDragHorz|
    {"rectDragVert", MT_RECT, OFF(m_rectDragVert)},            // @prop left, top, right, bottom|rectDragVert|
    {"rectFrameDragHorz", MT_RECT, OFF(m_rectFrameDragHorz)},  // @prop left, top, right, bottom|rectFrameDragHorz|
    {"rectFrameDragVert", MT_RECT, OFF(m_rectFrameDragVert)},  // @prop left, top, right, bottom|rectFrameDragVert|
    {"dwDockStyle", MT_INT, OFF(m_dwDockStyle)},               // @prop int|dwDockStyle|allowable dock styles for bar
    {"dwOverDockStyle", MT_INT, OFF(m_dwOverDockStyle)},  // @prop int|dwOverDockStyle|style of dock that rect is over
    {"dwStyle", MT_INT, OFF(m_dwStyle)},                  // @prop int|dwStyle|style of control bar
    {"bFlip", MT_INT, OFF(m_bFlip)},                      // @prop int|bFlip|if shift key is down
    {"bForceFrame", MT_INT, OFF(m_bForceFrame)},          // @prop int|bForceFrame|if ctrl key is down
                                                          //	CDC* m_pDC;                 // where to draw during drag
    {"bDragging", MT_INT, OFF(m_bDragging)},              // @prop int|bDragging|
    {"nHitTest", MT_INT, OFF(m_nHitTest)},                // @prop int|nHitTest|
    {"uMRUDockID", MT_INT, OFF(m_uMRUDockID)},            // @prop int|uMRUDockID|
    {"rectMRUDockPos", MT_RECT, OFF(m_rectMRUDockPos)},   // @prop left, top, right, bottom|rectMRUDockPos|
    {"dwMRUFloatStyle", MT_INT, OFF(m_dwMRUFloatStyle)},  // @prop int|dwMRUFloatStyle|
    {"ptMRUFloatPos", MT_POINT, OFF(m_ptMRUFloatPos)},    // @prop x,y|ptMRUFloatPos|
    {NULL}                                                /* Sentinel */
};

PyObject *PyCDockContext::getattro(PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    CDockContext *pC = GetDockContext(this);
    if (!pC)
        return NULL;

    for (MyMemberList *pm = dcmembers; pm->name; pm++) {
        if (strcmp(name, pm->name) == 0) {
            void *pv = &((BYTE *)pC)[pm->off];
            switch (pm->type) {
                case MT_INT:
                    return PyInt_FromLong(*((int *)pv));
                    break;
                case MT_RECT: {
                    CRect *p = (CRect *)pv;
                    return Py_BuildValue("iiii", p->left, p->top, p->right, p->bottom);
                }
                case MT_POINT: {
                    CPoint *p = (CPoint *)pv;
                    return Py_BuildValue("ii", p->x, p->y);
                }
                case MT_SIZE: {
                    CSize *p = (CSize *)pv;
                    return Py_BuildValue("ii", p->cx, p->cy);
                }
                default:
                    Py_FatalError("Bad type for CDockContext");
            }
        }
    }
    return ui_assoc_object::getattro(obname);
}

int PyCDockContext::setattro(PyObject *obname, PyObject *value)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    CDockContext *pC = GetDockContext(this);
    if (!pC)
        return NULL;

    for (MyMemberList *pm = dcmembers; pm->name; pm++) {
        if (strcmp(name, pm->name) == 0) {
            void *pv = &((BYTE *)pC)[pm->off];
            switch (pm->type) {
                case MT_INT:
                    *((int *)pv) = PyInt_AsLong(value);
                    return 0;
                case MT_RECT: {
                    CRect *p = (CRect *)pv;
                    if (!PyArg_ParseTuple(value, "iiii", &p->left, &p->top, &p->right, &p->bottom))
                        return -1;
                }
                    return 0;
                case MT_POINT: {
                    CPoint *p = (CPoint *)pv;
                    if (!PyArg_ParseTuple(value, "ii", &p->x, &p->y))
                        return -1;
                    return 0;
                }
                case MT_SIZE: {
                    CSize *p = (CSize *)pv;
                    if (!PyArg_ParseTuple(value, "ii", &p->cx, &p->cy))
                        return -1;
                }
                    return 0;
                default:
                    Py_FatalError("Bad type for CDockContext");
            }
        }
    }
    return ui_assoc_object::setattro(obname, value);
}

ui_type PyCDockContext::type("PyCDockContext", &ui_assoc_object::type, sizeof(PyCDockContext),
                             PYOBJ_OFFSET(PyCDockContext), PyCDockContext_methods, GET_PY_CTOR(PyCDockContext));

// @pymethod <o PyCControlBar>|win32ui|CreateControlBar|Creates a control bar object.
PyObject *PyCControlBar::create(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":CreateControlBar"))
        return NULL;
    CControlBar *cb = new CPythonControlBar();
    return ui_assoc_object::make(PyCControlBar::type, cb)->GetGoodRet();
}

CControlBar *PyCControlBar::GetControlBar(PyObject *self) { return (CControlBar *)GetGoodCppObject(self, &type); }

// @pymethod int|PyCControlBar|CalcDynamicLayout|The framework calls this member function to calculate the dimensions of
// a dynamic toolbar.
PyObject *PyCControlBar_CalcDynamicLayout(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    int length, dwMode;
    // @pyparm int|length||The requested dimension of the control bar, either horizontal or vertical, depending on
    // dwMode.
    // @pyparm int|dwMode||A combination of flags.
    if (!PyArg_ParseTuple(args, "ii:CalcDynamicLayout", &length, &dwMode))
        return NULL;
    GUI_BGN_SAVE;
    CSize sz = pCtlBar->CControlBar::CalcDynamicLayout(length, dwMode);
    GUI_END_SAVE;
    return Py_BuildValue("ii", sz.cx, sz.cy);
}

// @pymethod int|PyCControlBar|CalcFixedLayout|Calculates the horizontal size of a control bar
PyObject *PyCControlBar_CalcFixedLayout(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    int stretch, horz;
    // @pyparm int|bStretch||Indicates whether the bar should be stretched to the size of the frame. The bStretch
    // parameter is nonzero when the bar is not a docking bar (not available for docking) and is 0 when it is docked or
    // floating (available for docking).
    // @pyparm int|bHorz||Indicates that the bar is horizontally or vertically oriented.
    if (!PyArg_ParseTuple(args, "ii:CalcFixedLayout", &stretch, &horz))
        return NULL;
    GUI_BGN_SAVE;
    CSize sz = pCtlBar->CControlBar::CalcFixedLayout(stretch, horz);
    GUI_END_SAVE;
    return Py_BuildValue("ii", sz.cx, sz.cy);
}

// @pymethod |PyCControlBar|EraseNonClient|
PyObject *PyCControlBar_EraseNonClient(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    if (!PyArg_ParseTuple(args, ":EraseNonClient"))
        return NULL;
    GUI_BGN_SAVE;
    pCtlBar->EraseNonClient();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCFrameWnd>|PyCControlBar|GetDockingFrame|Returns the frame window to which a control bar is docked.
PyObject *PyCControlBar_GetDockingFrame(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetDockingFrame"))
        return NULL;
    GUI_BGN_SAVE;
    CWnd *pWnd = pCtlBar->GetDockingFrame();
    GUI_END_SAVE;
    if (pWnd == NULL)
        RETURN_ERR("There is no docking frame window");
    return ui_assoc_object::make(PyCFrameWnd::type, pWnd)->GetGoodRet();
}

// @pymethod int|PyCControlBar|GetCount|Returns the number of non-HWND elements in the control bar.
PyObject *PyCControlBar_GetCount(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetCount"))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pCtlBar->GetCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}
// @pymethod int|PyCControlBar|IsFloating|Returns a nonzero value if the control bar in question is a floating control
// bar.
PyObject *PyCControlBar_IsFloating(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    if (!PyArg_ParseTuple(args, ":IsFloating"))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pCtlBar->IsFloating();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCControlBar|GetBarStyle|Retrieves the control bar style settings.
PyObject *PyCControlBar_GetBarStyle(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetBarStyle"))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pCtlBar->GetBarStyle();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod |PyCControlBar|SetBarStyle|Modifies the control bar style settings.
PyObject *PyCControlBar_SetBarStyle(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    int style;
    // @pyparm int|style||The new style
    if (!PyArg_ParseTuple(args, "i:SetBarStyle", &style))
        return NULL;
    GUI_BGN_SAVE;
    pCtlBar->SetBarStyle(style);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCControlBar|EnableDocking|pecifies whether the control bar supports docking and the sides of its parent
// window.
PyObject *PyCControlBar_EnableDocking(PyObject *self, PyObject *args)
{
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(self);
    if (!pCtlBar)
        return NULL;
    int style;
    // @pyparm int|style||Enables a control bar to be docked.
    if (!PyArg_ParseTuple(args, "i:EnableDocking", &style))
        return NULL;
    GUI_BGN_SAVE;
    pCtlBar->EnableDocking(style);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCControlBar|ShowWindow|Shows the toolbar, and recalculates the button layout.
static PyObject *PyCControlBar_ShowWindow(PyObject *self, PyObject *args)
{
    // proto the base class method here
    extern PyObject *ui_window_show_window(PyObject * self, PyObject * args);

    // @comm This method is provided for convenience.  For further details, see
    // <om PyCWnd.ShowWindow> and <om PyCFrameWnd.RecalcLayout>

    PyObject *ret = ui_window_show_window(self, args);
    // call base first
    CWnd *pWnd = GetWndPtr(self);
    if (!pWnd)
        return NULL;

    CWnd *parent = pWnd->GetParent();
    if (parent && parent->IsKindOf(RUNTIME_CLASS(CFrameWnd))) {
        GUI_BGN_SAVE;
        ((CFrameWnd *)parent)->RecalcLayout();
        GUI_END_SAVE;
    }
    return ret;
    // @rdesc The return value is that returned from <om PyCWnd.ShowWindow>
}

// @object PyCControlBar|A class which encapsulates an MFC <o CControlBar>.  Derived from a <o PyCWnd> object.
static struct PyMethodDef PyCControlBar_methods[] = {
    {"CalcDynamicLayout", PyCControlBar_CalcDynamicLayout,
     1},  // @pymeth CalcDynamicLayout|The framework calls this member function to calculate the dimensions of a dynamic
          // toolbar.
    {"CalcFixedLayout", PyCControlBar_CalcFixedLayout,
     1},  // @pymeth CalcFixedLayout|Calculates the horizontal size of a control bar
    {"EnableDocking", PyCControlBar_EnableDocking, 1},    // @pymeth EnableDocking|Specifies whether the control bar
                                                          // supports docking and the sides of its parent window.
    {"EraseNonClient", PyCControlBar_EraseNonClient, 1},  // @pymeth EraseNonClient|
    {"GetBarStyle", PyCControlBar_GetBarStyle, 1},  // @pymeth GetBarStyle|Retrieves the control bar style settings.
    {"GetCount", PyCControlBar_GetCount,
     1},  // @pymeth GetCount|Returns the number of non-HWND elements in the control bar.
    {"GetDockingFrame", PyCControlBar_GetDockingFrame,
     1},  // @pymeth GetDockingFrame|Returns the frame window to which a control bar is docked.
    {"IsFloating", PyCControlBar_IsFloating,
     1},  // @pymeth IsFloating|Returns a nonzero value if the control bar in question is a floating control bar.
    {"SetBarStyle", PyCControlBar_SetBarStyle, 1},  // @pymeth SetBarStyle|Modifies the control bar style settings.
    {"ShowWindow", PyCControlBar_ShowWindow,
     1},  // @pymeth ShowWindow|Shows the window, and recalculates the toolbar layout.
    {NULL, NULL}};

PyObject *PyCControlBar::getattro(PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    CControlBar *pCtlBar = PyCControlBar::GetControlBar(this);
    if (!pCtlBar)
        return NULL;
    if (strcmp(name, "dockSite") == 0) {  // @prop <o PyCFrameWnd>|dockSite|Current dock site, if dockable
        if (pCtlBar->m_pDockSite == NULL)
            RETURN_NONE;
        return ui_assoc_object::make(UITypeFromCObject(pCtlBar->m_pDockSite), pCtlBar->m_pDockSite);
    }
    if (strcmp(name, "dockBar") == 0) {  // @prop <o PyCWnd>|dockBar|Current dock bar, if dockable
        if (pCtlBar->m_pDockBar == NULL)
            RETURN_NONE;
        return ui_assoc_object::make(UITypeFromCObject(pCtlBar->m_pDockBar), pCtlBar->m_pDockBar);
    }
    if (strcmp(name, "dockContext") == 0) {  // @prop <o PyCDockContext>|dockContext|Used during dragging
        if (pCtlBar->m_pDockBar == NULL)
            RETURN_NONE;
        return ui_assoc_object::make(PyCDockContext::type, pCtlBar->m_pDockContext);
    }
    if (strcmp(name, "dwStyle") == 0)  // @prop int|dwStyle|creation style (used for layout)
        return PyInt_FromLong(pCtlBar->m_dwStyle);
    if (strcmp(name, "dwDockStyle") == 0)  // @prop int|dwDockStyle|indicates how bar can be docked
        return PyInt_FromLong(pCtlBar->m_dwStyle);

    return PyObject_GenericGetAttr(this, obname);
}

int PyCControlBar::setattro(PyObject *obname, PyObject *v)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;

    CControlBar *pCtlBar = PyCControlBar::GetControlBar(this);
    if (!pCtlBar)
        return -1;
    if (strcmp(name, "dwStyle") == 0) {
        pCtlBar->m_dwStyle = PyInt_AsLong(v);
        if (pCtlBar->m_dwStyle == -1 && PyErr_Occurred())
            return -1;
        return 0;
    }
    if (strcmp(name, "dwDockStyle") == 0) {
        pCtlBar->m_dwDockStyle = PyInt_AsLong(v);
        if (pCtlBar->m_dwDockStyle == -1 && PyErr_Occurred())
            return -1;
        return 0;
    }
    return PyObject_GenericSetAttr(this, obname, v);
}

ui_type_CObject PyCControlBar::type("PyCControlBar", &PyCWnd::type, RUNTIME_CLASS(CControlBar), sizeof(PyCControlBar),
                                    PYOBJ_OFFSET(PyCControlBar), PyCControlBar_methods, GET_PY_CTOR(PyCControlBar));

/* ToolBar Wish, er, ToDo List:

   change toolbar button styles (SetButtonInfo)

*/
#define MAKE_GET_INT_INT_METH(fnname, mfcName)             \
    PyObject *fnname(PyObject *self, PyObject *args)       \
    {                                                      \
        CToolBar *pToolBar = PyCToolBar::GetToolBar(self); \
        if (!pToolBar)                                     \
            return NULL;                                   \
        int val;                                           \
        if (!PyArg_ParseTuple(args, "i:" #mfcName, &val))  \
            return NULL;                                   \
        return Py_BuildValue("i", pToolBar->mfcName(val)); \
    }

#define MAKE_SETVOID_INT_METH(fnname, mfcName)             \
    PyObject *fnname(PyObject *self, PyObject *args)       \
    {                                                      \
        CToolBar *pToolBar = PyCToolBar::GetToolBar(self); \
        if (!pToolBar)                                     \
            return NULL;                                   \
        int val;                                           \
        if (!PyArg_ParseTuple(args, "i:" #mfcName, &val))  \
            return NULL;                                   \
        pToolBar->mfcName(val);                            \
        RETURN_NONE;                                       \
    }

/* static */ CToolBar *PyCToolBar::GetToolBar(PyObject *self) { return (CToolBar *)GetGoodCppObject(self, &type); }

// @pymethod <o PyCToolBar>|win32ui|CreateToolBar|Creates a toolbar object.
PyObject *PyCToolBar::create(PyObject *self, PyObject *args)
{
    PyObject *parent;
    int style;
    int id = AFX_IDW_TOOLBAR;
    if (!PyArg_ParseTuple(args, "Oi|i:CreateToolBar",
                          &parent,  // @pyparm <o PyCWnd>|parent||The parent window for the toolbar.
                          &style,   // @pyparm int|style||The style for the toolbar.
                          &id))     // @pyparm int|windowId|afxres.AFX_IDW_TOOLBAR|The child window ID.
        return NULL;
    if (!ui_base_class::is_uiobject(parent, &PyCWnd::type)) {
        RETURN_ERR("The parent param must be a window object.");
    }

    // @comm You must ensure no 2 toolbars share the same ID.
    CString error;
    CToolBar *tb = new CPythonToolBar();
    CFrameWnd *frame = (CFrameWnd *)PyCWnd::GetPythonGenericWnd(parent, &PyCFrameWnd::type);
    if (frame == NULL)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = tb->Create(frame, style, id);
    GUI_END_SAVE;
    if (!ok) {
        delete tb;
        RETURN_API_ERR("PyCToolBar.Create");
    }
    tb->m_bAutoDelete = TRUE;  // let MFC handle deletion
    return ui_assoc_object::make(PyCToolBar::type, tb)->GetGoodRet();
}

// @pymethod |PyCToolBar|SetButtons|Sets button styles and an index of button images within the bitmap.
PyObject *PyCToolBar_SetButtons(PyObject *self, PyObject *args)
{
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    PyObject *buttons;
    if (!PyArg_ParseTuple(args, "O:SetButtons",
                          &buttons))  // @pyparm tuple|buttons||A tuple containing the ID's of the buttons.
        return NULL;

    if (PyInt_Check(buttons)) {
        // @pyparmalt1 int|numButtons||The number of buttons to pre-allocate.  If this option is used, then <om
        // PyCToolBar.PySetButtonInfo> must be used.
        BOOL rc = pToolBar->SetButtons(NULL, PyInt_AsLong(buttons));
        if (!rc)
            RETURN_API_ERR("PyCToolBar.SetButtons");
        RETURN_NONE;
    }
    // Not an integer - normal tuple of buttons.
    if (!PyTuple_Check(buttons))
        RETURN_TYPE_ERR("SetButtons requires a tuple of IDs");

    // convert button tuple to array
    Py_ssize_t num_buttons = PyTuple_Size(buttons);
    UINT *button_list = new UINT[num_buttons];
    PyObject *o;
    for (Py_ssize_t i = 0; i < num_buttons; i++) {
        o = PyTuple_GetItem(buttons, i);
        if (!PyInt_Check(o)) {
            delete button_list;
            RETURN_ERR("SetButtons expected integer button ids.");
        }
        button_list[i] = PyInt_AsLong(o);
    }
    BOOL rc = pToolBar->SetButtons(button_list, PyWin_SAFE_DOWNCAST(num_buttons, Py_ssize_t, int));
    delete button_list;
    if (!rc)
        RETURN_API_ERR("PyCToolBar.SetButtons");
    RETURN_NONE;
}

// @pymethod |PyCToolBar|SetButtonInfo|Sets the button's command ID, style, and image number.
static PyObject *PyCToolBar_SetButtonInfo(PyObject *self, PyObject *args)
{
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;

    int nIndex;
    UINT nID;
    UINT nStyle;
    int iImage;
    if (!PyArg_ParseTuple(
            args, "iiii",
            &nIndex,   // @pyparm int|index||Index of the button or separator whose information is to be set.
            &nID,      // @pyparm int|ID||The value to which the button's command ID is set.
            &nStyle,   // @pyparm int|style||The new button style
            &iImage))  // @pyparm int|imageIx||New index for the button's image within the bitmap
        return NULL;

    // since the info is set through msgs we must protect state
    GUI_BGN_SAVE;
    pToolBar->SetButtonInfo(nIndex, nID, nStyle, iImage);
    GUI_END_SAVE;

    RETURN_NONE;
}

// @pymethod |PyCToolBar|GetToolTips|Returns the associated tooltips control
static PyObject *PyCToolBar_GetToolTips(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetToolTips);
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    CToolBarCtrl &toolBarCtrl = pToolBar->GetToolBarCtrl();
    CToolTipCtrl *pTTC = toolBarCtrl.GetToolTips();
    if (pTTC == NULL)
        RETURN_ERR("Toolbar hasn't a tool tip control");
    return ui_assoc_object::make(PyCToolTipCtrl::type, pTTC);
}

// @pymethod |PyCToolBar|SetToolTips|Sets the tooltips control
static PyObject *PyCToolBar_SetToolTips(PyObject *self, PyObject *args)
{
    PyObject *obTTC;
    if (!PyArg_ParseTuple(args, "O",
                          &obTTC))  // @pyparm <o PyCToolTipCtrl>|obTTC||The ToolTipCtrl ctrl to be set.
        return NULL;

    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;

    CToolTipCtrl *pTTC = (CToolTipCtrl *)GetWndPtr(obTTC);
    if (!pTTC)
        return NULL;

    CToolBarCtrl &toolBarCtrl = pToolBar->GetToolBarCtrl();
    toolBarCtrl.SetToolTips(pTTC);
    RETURN_NONE;
}

// @pymethod |PyCToolBar|SetBarStyle|Sets the toolbar part of style
static PyObject *PyCToolBar_SetBarStyle(PyObject *self, PyObject *args)
{
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    DWORD dwStyle;
    if (!PyArg_ParseTuple(args, "l",
                          &dwStyle))  // @pyparm long|style||The toolbar style to set.
        return NULL;
    pToolBar->SetBarStyle(dwStyle);
    RETURN_NONE;
}

// @pymethod |PyCToolBar|LoadBitmap|Loads the bitmap containing bitmap-button images.
static PyObject *PyCToolBar_LoadBitmap(PyObject *self, PyObject *args)
{
    BOOL rc;
    TCHAR *szId;
    PyObject *obId;
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    if (!PyArg_ParseTuple(args, "O:LoadBitmap",
                          &obId))  // @pyparm <o PyResourceId>|id||Name or id of the resource that contains the bitmap.
        return NULL;
    if (!PyWinObject_AsResourceId(obId, &szId, FALSE))
        return NULL;

    if (IS_INTRESOURCE(szId))
        rc = pToolBar->LoadBitmap(MAKEINTRESOURCE(szId));
    else
        rc = pToolBar->LoadBitmap(szId);
    PyWinObject_FreeResourceId(szId);

    if (!rc)
        RETURN_ERR("LoadBitmap failed");
    // @comm The bitmap should contain one image for each toolbar button. If the
    // images are not of the standard size (16 pixels wide and 15 pixels high),
    // call <om PyCToolBar.SetSizes> to set the button sizes and their images.
    RETURN_NONE;
}
// @pymethod |PyCToolBar|LoadToolBar|Loads a toolbar from a toolbar resource.
static PyObject *PyCToolBar_LoadToolBar(PyObject *self, PyObject *args)
{
    BOOL rc;
    TCHAR *szId;
    PyObject *obId;
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    if (!PyArg_ParseTuple(args, "O:LoadToolBar",
                          &obId))  // @pyparm <o PyResourceId>|id||Name or resource id of the resource
        return NULL;

    if (!PyWinObject_AsResourceId(obId, &szId, FALSE))
        return NULL;

    if (IS_INTRESOURCE(szId))
        rc = pToolBar->LoadToolBar(MAKEINTRESOURCE(szId));
    else
        rc = pToolBar->LoadToolBar(szId);
    PyWinObject_FreeResourceId(szId);
    if (!rc)
        RETURN_ERR("LoadToolBar failed");
    // @comm The bitmap should contain one image for each toolbar button. If the
    // images are not of the standard size (16 pixels wide and 15 pixels high),
    // call <om PyCToolBar.SetSizes> to set the button sizes and their images.
    RETURN_NONE;
}

// @pymethod |PyCToolBar|SetHeight|Sets the height of the toolbar.
// @pyparm int|height||The height in pixels of the toolbar.
MAKE_SETVOID_INT_METH(PyCToolBar_SetHeight, SetHeight)

// @pymethod |PyCToolBar|GetItemID|Returns the command ID of a button or separator at the given index.
// @pyparm int|index||Index of the item whose ID is to be retrieved.
MAKE_GET_INT_INT_METH(PyCToolBar_GetItemID, GetItemID)

// @pymethod |PyCToolBar|GetButtonStyle|Retrieves the style for a button.
// @pyparm int|index||Index of the item whose style is to be retrieved.
MAKE_GET_INT_INT_METH(PyCToolBar_GetButtonStyle, GetButtonStyle)

// @pymethod |PyCToolBar|SetBitmap|Sets a bitmapped image.
PyObject *PyCToolBar_SetBitmap(PyObject *self, PyObject *args)
{
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    // @pyparm int|hBitmap||The handle to a bitmap resource.
    // @comm Call this method to set the bitmap image for the toolbar. For example,
    // call SetBitmap to change the bitmapped image after the user takes an action on
    // a document that changes the action of a button.
    PyObject *obval;
    if (!PyArg_ParseTuple(args, "O:SetBitmap", &obval))
        return NULL;
    HBITMAP val;
    if (!PyWinObject_AsHANDLE(obval, (HANDLE *)&val))
        return NULL;
    if (!IsWin32s() && ::GetObjectType(val) != OBJ_BITMAP)
        RETURN_ERR("The bitmap handle is invalid");
    if (!pToolBar->SetBitmap(val))
        RETURN_ERR("SetBitmap failed");
    RETURN_NONE;
}

// @pymethod |PyCToolBar|SetSizes|Sets the size of each button.
static PyObject *PyCToolBar_SetSizes(PyObject *self, PyObject *args)
{
    SIZE sizeBut, sizeBmp;
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;

    if (!PyArg_ParseTuple(args, "(ii)(ii)", &sizeBut.cx,
                          &sizeBut.cy,                // @pyparm (cx, cy)|sizeButton||The size of each button.
                          &sizeBmp.cx, &sizeBmp.cy))  // @pyparm (cx, cy)|sizeButton||The size of each bitmap.
        return NULL;
    pToolBar->SetSizes(sizeBut, sizeBmp);
    RETURN_NONE;
}

// @pymethod |PyCToolBar|SetButtonStyle|Sets the style for a button.
static PyObject *PyCToolBar_SetButtonStyle(PyObject *self, PyObject *args)
{
    int index, style;
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    if (!PyArg_ParseTuple(args, "ii",
                          &index,   // @pyparm int|index||Index of the item whose style is to be set
                          &style))  // @pyparm int|style||The new style
        return NULL;
    pToolBar->SetButtonStyle(index, style);
    RETURN_NONE;
}

// @pymethod string|PyCToolBar|GetButtonText|Gets the text for a button.
PyObject *PyCToolBar_GetButtonText(PyObject *self, PyObject *args)
{
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    int index;
    // @pyparm int|index||Index of the item whose text is to be retrieved.
    if (!PyArg_ParseTuple(args, "i:GetButtonText", &index))
        return NULL;
    return PyWinObject_FromTCHAR(pToolBar->GetButtonText(index));
}

// @pymethod |PyCToolBar|SetButtonText|Sets the text for a button.
static PyObject *PyCToolBar_SetButtonText(PyObject *self, PyObject *args)
{
    int index;
    TCHAR *text;
    PyObject *obtext;
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;
    if (!PyArg_ParseTuple(args, "iO",
                          &index,    // @pyparm int|index||Index of the item whose style is to be set
                          &obtext))  // @pyparm string|text||The new text
        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE))
        return NULL;
    pToolBar->SetButtonText(index, text);
    PyWinObject_FreeTCHAR(text);
    RETURN_NONE;
}

// @pymethod <o PyCToolBarCtrl>|PyCToolBar|GetToolBarCtrl|Gets the toolbar control object for the toolbar
PyObject *PyCToolBar_GetToolBarCtrl(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBar *pToolBar = PyCToolBar::GetToolBar(self);
    if (!pToolBar)
        return NULL;

    CToolBarCtrl &rTBC = pToolBar->GetToolBarCtrl();
    // Note that below we take the address of rTBC because it's a reference and not a pointer
    // and ui_assoc_object::make expects a pointer.
    // We need to create a new class and not do a map lookup because in MFC CToolBarCtrl is
    // simply a casted CToolBarCtrl (afxext.inl) so the lookup will return the PyCToolBar object
    // which will fail the type tests.
    return ui_assoc_object::make(PyCToolBarCtrl::type, &rTBC, true)->GetGoodRet();
}

// @object PyCToolBar|A class which encapsulates an MFC <o CToolBar>.  Derived from a <o PyCControlBar> object.
static struct PyMethodDef PyCToolBar_methods[] = {
    {"GetButtonStyle", PyCToolBar_GetButtonStyle, 1},  // @pymeth GetButtonStyle|Retrieves the style for a button.
    {"GetButtonText", PyCToolBar_GetButtonText, 1},    // @pymeth GetButtonText|Gets the text for a button.
    {"GetItemID", PyCToolBar_GetItemID,
     1},  // @pymeth GetItemID|Returns the command ID of a button or separator at the given index.
    {"GetToolTips", PyCToolBar_GetToolTips, 1},  // @pymeth SetButtonInfo|Gets the associated tooltip control
    {"GetToolBarCtrl", PyCToolBar_GetToolBarCtrl,
     1},  // @pymeth GetToolBarCtrl|Returns the tool bar control object associated with the tool bar
    {"LoadBitmap", PyCToolBar_LoadBitmap, 1},    // @pymeth LoadBitmap|Loads the bitmap containing bitmap-button images.
    {"LoadToolBar", PyCToolBar_LoadToolBar, 1},  // @pymeth LoadToolBar|Loads a toolbar from a Toolbar resource.
    {"SetBarStyle", PyCToolBar_SetBarStyle, 1},  // @pymeth SetBarStyle|Sets toolbar's (CBRS_xxx) part of style
    {"SetBitmap", PyCToolBar_SetBitmap, 1},      // @pymeth SetBitmap|Sets a bitmapped image.
    {"SetButtonInfo", PyCToolBar_SetButtonInfo,
     1},  // @pymeth SetButtonInfo|Sets the button's command ID, style, and image number.
    {"SetButtons", PyCToolBar_SetButtons,
     1},  // @pymeth SetButtons|Sets button styles and an index of button images within the bitmap.
    {"SetButtonStyle", PyCToolBar_SetButtonStyle, 1},  // @pymeth SetButtonStyle|Sets the style for a button
    {"SetHeight", PyCToolBar_SetHeight, 1},            // @pymeth SetHeight|Sets the height of the toolbar.
    {"SetSizes", PyCToolBar_SetSizes, 1},              // @pymeth SetSizes|Sets the sizes for the toolbar items.
    {"SetToolTips", PyCToolBar_SetToolTips, 1},        // @pymeth SetButtonInfo|Sets the tooltips control
    {NULL, NULL}};

ui_type_CObject PyCToolBar::type("PyCToolBar", &PyCControlBar::type, RUNTIME_CLASS(CToolBar), sizeof(PyCToolBar),
                                 PYOBJ_OFFSET(PyCToolBar), PyCToolBar_methods, GET_PY_CTOR(PyCToolBar));

///////////////////////////////////////////////////////////////////////////////
//
// PyCToolBarCtrl object
//

#define MAKE_GET_BOOL_INT_METHOD(mfcName)                              \
    PyObject *PyCToolBarCtrl_##mfcName(PyObject *self, PyObject *args) \
    {                                                                  \
        CToolBarCtrl *pTBC = GetToolBarCtrl(self);                     \
        if (!pTBC)                                                     \
            return NULL;                                               \
        int nID;                                                       \
        if (!PyArg_ParseTuple(args, "i:" #mfcName, &nID))              \
            return NULL;                                               \
        GUI_BGN_SAVE;                                                  \
        int rc = pTBC->mfcName(nID);                                   \
        GUI_END_SAVE;                                                  \
        return Py_BuildValue("i", rc);                                 \
    }

#define MAKE_SET_INT_BOOL_METHOD(mfcName)                              \
    PyObject *PyCToolBarCtrl_##mfcName(PyObject *self, PyObject *args) \
    {                                                                  \
        CToolBarCtrl *pTBC = GetToolBarCtrl(self);                     \
        if (!pTBC)                                                     \
            return NULL;                                               \
        int nID;                                                       \
        int bSet;                                                      \
        if (!PyArg_ParseTuple(args, "ii:" #mfcName, &nID, &bSet))      \
            return NULL;                                               \
        GUI_BGN_SAVE;                                                  \
        BOOL rc = pTBC->mfcName(nID, bSet);                            \
        GUI_END_SAVE;                                                  \
        if (!rc)                                                       \
            RETURN_ERR("CToolBarCtrl::" #mfcName);                     \
        RETURN_NONE;                                                   \
    }

//#define MAKE_SET_INT_BOOL_METHOD(mfcName) MAKE_SET_INT_INT_METHOD(mfcName)

PyCToolBarCtrl::PyCToolBarCtrl()
{
    bmplist = new CPtrArray();
    strlist = new CPtrArray();
}

PyCToolBarCtrl::~PyCToolBarCtrl()
{
    INT_PTR i, n;
    n = bmplist->GetSize();
    for (i = 0; i < n; i++) {
        PyObject *o = (PyObject *)bmplist->GetAt(i);
        Py_DECREF(o);
    }
    delete bmplist;

    n = strlist->GetSize();
    for (i = 0; i < n; i++) PyWinObject_FreeMultipleString((TCHAR *)strlist->GetAt(i));
    delete strlist;
}

/* static */ CToolBarCtrl *GetToolBarCtrl(PyObject *self)
{
    // note we can only ask for a CWnd since the same object can be both
    // a PyCToolBar and a PyCToolBarCtrl instance and their only common
    // base class is PyCWnd. Otherwise the RTTI call will fail
    return (CToolBarCtrl *)PyCWnd::GetPythonGenericWnd(self);
}

// @pymethod <o PyCToolBarCtrl>|win32ui|CreateToolBarCtrl|Creates a toolbar control object.  <om
// PyCToolBarCtrl.CreateWindow> creates the actual control.
PyObject *PyCToolBarCtrl_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = new CToolBarCtrl();
    return ui_assoc_object::make(PyCToolBarCtrl::type, pTBC);
}

// @pymethod |PyCToolBarCtrl|TBUTTON tuple|Describes a TBUTTON tuple, used by the PyCToolBarCtrl AddButtons method
// @pyparm int|iBitmap||Zero-based index of button image
// @pyparm int|idCommand||Command to be sent when button pressed
// @pyparm int|fsState||Button state. Can be any of the TBSTATE values defined in win32con
// @pyparm int|fsStyle||Button style. Can be any of the TBSTYLE values defined in win32con
// @pyparm object|userob||Arbitrary Python object
// @pyparm int|iString||Zero-based index of button label string
// @comm Userob is any Python object at all, but no reference count is kept, so you must ensure the object remains
// referenced throughout.

// @pymethod int|PyCToolBarCtrl|AddBitmap|Add one or more button images to the list of button images

PyObject *PyCToolBarCtrl_AddBitmap(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (!pTBC)
        return NULL;
    int numButtons;
    PyObject *pBitmap = NULL;
    if (!PyArg_ParseTuple(args, "iO",
                          &numButtons,  // @pyparm int|numButtons||Number of button images in the bitmap.
                          &pBitmap))    // @pyparm <o PyBitmap>|bitmap||Bitmap containing button or buttons to be added
        return NULL;

    Py_INCREF(pBitmap);
    ((PyCToolBarCtrl *)self)->bmplist->Add((void *)pBitmap);
    GUI_BGN_SAVE;
    int rc = pTBC->AddBitmap(numButtons, ui_bitmap::GetBitmap(pBitmap));
    GUI_END_SAVE;

    // @pyseemfc CToolBarCtrl|AddBitmap
    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCToolBarCtrl|AddButtons|Add one or more buttons to the toolbar

PyObject *PyCToolBarCtrl_AddButtons(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    Py_ssize_t numButtons = PyTuple_Size(args);

    TBBUTTON *btn = new TBBUTTON[numButtons];

    if (btn == NULL)
        return NULL;

    Py_ssize_t i;
    PyObject *pButtonTuple;

    for (i = 0; i < numButtons; i++) {
        pButtonTuple = PySequence_GetItem(args, i);
        if (!PyArg_ParseTuple(pButtonTuple, "iibbOi", &btn[i].iBitmap, &btn[i].idCommand, &btn[i].fsState,
                              &btn[i].fsStyle, &btn[i].dwData, &btn[i].iString)) {
            delete btn;
            return NULL;
        }
    }

    // @pyseemfc CToolBarCtrl|AddButtons
    GUI_BGN_SAVE;
    int rc = pTBC->AddButtons(PyWin_SAFE_DOWNCAST(numButtons, Py_ssize_t, int), btn);
    GUI_END_SAVE;
    PyObject *ret = Py_BuildValue("i", rc);
    delete btn;
    return ret;
}

// @pymethod int|PyCToolBarCtrl|AddStrings|Add one or more strings to the toolbar

PyObject *PyCToolBarCtrl_AddStrings(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    TCHAR *strings;
    DWORD charcnt;

    if (!PyWinObject_AsMultipleString(args, &strings, FALSE, &charcnt))
        return NULL;

    // Add string pointer to list of things to be cleaned up at the end.
    // (XXX - is this really necessary?  It seems surprising the control
    // doesn't take its own copy???)
    ((PyCToolBarCtrl *)self)->strlist->Add(strings);

    // @pyparm string...|strings||Strings to add. Can give more than one string.
    GUI_BGN_SAVE;
    int rc = pTBC->AddStrings(strings);
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod |PyCToolBarCtrl|AutoSize|Resize the entire toolbar control

PyObject *PyCToolBarCtrl_AutoSize(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pTBC->AutoSize();  // @pyseemfc CToolBarCtrl|AutoSize
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCToolBarCtrl|CheckButton|Check or clear a given button in a toolbar control

PyObject *PyCToolBarCtrl_CheckButton(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID, bCheck = 1;
    if (!PyArg_ParseTuple(args, "i|i:CheckButton",
                          &nID,      // @pyparm int|nID||Command identifier of the button to check or clear.
                          &bCheck))  // @pyparm int|bCheck|1|1 to check, 0 to clear the button
        return NULL;
    GUI_BGN_SAVE;
    int rc = pTBC->CheckButton(nID, bCheck);
    GUI_END_SAVE;

    return Py_BuildValue("i", rc);  // @pyseemfc CToolBarCtrl|CheckButton
}

// @pymethod int|PyCToolBarCtrl|CommandToIndex|Retrieve the zero-based index for the button associated with the
// specified command identifier.

PyObject *PyCToolBarCtrl_CommandToIndex(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID;
    if (!PyArg_ParseTuple(args, "i:CommandToIndex",
                          &nID))  // @pyparm int|nID||Command identifier of the button you want to find.
        return NULL;

    GUI_BGN_SAVE;
    int rc = pTBC->CommandToIndex(nID);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CToolBarCtrl|CommandToIndex
}

// @pymethod |PyCToolBarCtrl|CreateWindow|Creates the window for a new toolbar object
static PyObject *PyCToolBarCtrl_CreateWindow(PyObject *self, PyObject *args)
{
    int style, id;
    PyObject *obParent;
    RECT rect;
    if (!PyArg_ParseTuple(
            args, "i(iiii)Oi:CreateWindow",
            &style,  // @pyparm int|style||The style for the button.  Use any of the win32con.BS_* constants.
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
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (!pTBC)
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pTBC->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CToolBarCtrl|Create
        RETURN_ERR("CToolBarCtrl::Create");
    RETURN_NONE;
}

// @pymethod |PyCToolBarCtrl|Customize|Display the Customize Toolbar dialog box.

PyObject *PyCToolBarCtrl_Customize(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pTBC->Customize();  // @pyseemfc CToolBarCtrl|Customize
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCToolBarCtrl|DeleteButton|Delete a button from the toolbar control.

PyObject *PyCToolBarCtrl_DeleteButton(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID;
    if (!PyArg_ParseTuple(args, "i:DeleteButton",
                          &nID))  // @pyparm int|nID||ID of the button to delete.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pTBC->DeleteButton(nID);
    GUI_END_SAVE;

    if (!ok)  // @pyseemfc CToolBarCtrl|DeleteButton
        RETURN_ERR("CToolBarCtrl::DeleteButton");

    RETURN_NONE;
}

// @pymethod |PyCToolBarCtrl|EnableButton|Enable or disable a toolbar control button.
// @pyparm int|nID||ID of the button to enable or disable.
// @pyparm int|bEnable|1|1 to enable, 0 to disable
// @pyseemfc CToolBarCtrl|EnableButton
MAKE_SET_INT_BOOL_METHOD(EnableButton)

// @pymethod int|PyCToolBarCtrl|GetBitmapFlags|retrieve the bitmap flags from the toolbar.

PyObject *PyCToolBarCtrl_GetBitmapFlags(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;

    GUI_BGN_SAVE;
    int rc = pTBC->GetBitmapFlags();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CToolBarCtrl|GetBitmapFlags
}

// @pymethod <om PyCToolBarCtrl.TBBUTTON>|PyCToolBarCtrl|GetButton|Retrieve information about the specified button in a
// toolbar control.

PyObject *PyCToolBarCtrl_GetButton(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID;
    if (!PyArg_ParseTuple(args, "i:GetButton",
                          &nID))  // @pyparm int|nID||ID of the button to retrieve.
        return NULL;
    TBBUTTON tbb;
    GUI_BGN_SAVE;
    BOOL ok = pTBC->GetButton(nID, &tbb);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CToolBarCtrl|GetButton
        RETURN_ERR("CToolBarCtrl::GetButton");
    return Py_BuildValue("iibbli", tbb.iBitmap, tbb.idCommand, tbb.fsState, tbb.fsStyle, tbb.dwData, tbb.iString);
}

// @pymethod int|PyCToolBarCtrl|GetButtonCount|Retrieve a count of the buttons currently in the toolbar control.

PyObject *PyCToolBarCtrl_GetButtonCount(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;

    GUI_BGN_SAVE;
    int rc = pTBC->GetButtonCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CToolBarCtrl|GetButtonCount
}

// @pymethod left, top, right, bottom|PyCToolBarCtrl|GetItemRect|Retrieve the bounding rectangle of a button in a
// toolbar control.

PyObject *PyCToolBarCtrl_GetItemRect(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID;
    if (!PyArg_ParseTuple(args, "i:GetItemRect",
                          &nID))  // @pyparm int|nID||ID of the button.
        return NULL;

    RECT r;
    GUI_BGN_SAVE;
    BOOL ok = pTBC->GetItemRect(nID, &r);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CToolBarCtrl|GetItemRect
        RETURN_ERR("CToolBarCtrl::GetItemRect");
    return Py_BuildValue("(iiii)", r.left, r.top, r.right, r.bottom);
}

// @pymethod left, top, right, bottom|PyCToolBarCtrl|GetRows|Retrieve the number of rows of buttons currently displayed

PyObject *PyCToolBarCtrl_GetRows(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pTBC->GetRows();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CToolBarCtrl|GetRows
}

// @pymethod |PyCToolBarCtrl|HideButton|Hide or show the specified button in a toolbar control.
// @pyparm int|nID||ID of the button to hide.
// @pyparm int|bEnable|1|1 to hide, 0 to show.
// @pyseemfc CToolBarCtrl|HideButton

MAKE_SET_INT_BOOL_METHOD(HideButton)

// @pymethod |PyCToolBarCtrl|Indeterminate|Mark or unmark the specified button as indeterminate
// @pyparm int|nID||ID of the button to mark.
// @pyparm int|bEnable|1|1 to hide, 0 to show.
// @pyseemfc CToolBarCtrl|Indeterminate

MAKE_SET_INT_BOOL_METHOD(Indeterminate)

// @pymethod int|PyCToolBarCtrl|InsertButton|Insert a button in a toolbar control.
// @comm The image and/or string whose index you provide must have
// previously been added to the toolbar control's list using
// <om PyCToolBarCtrl.AddBitmap>, <om PyCToolBarCtrl.AddString>,
// and/or <om PyCToolBarCtrl.AddStrings>.

PyObject *PyCToolBarCtrl_InsertButton(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nID;
    TBBUTTON btn;

    if (!PyArg_ParseTuple(
            args, "i(iibbOi)",
            &nID,  // @pyparm int|nID||Zero-based index of a button. This function inserts the new button to the left of
                   // this button.
            &btn.iBitmap,  // @pyparm <om PyCToolBarCtrl.TBBUTTON>|button||Bitmap containing button to be inserted
            &btn.idCommand, &btn.fsState, &btn.fsStyle, (PyObject *)&btn.dwData, &btn.iString))
        return NULL;

    // @pyseemfc CToolBarCtrl|InsertButton

    GUI_BGN_SAVE;
    BOOL ok = pTBC->InsertButton(nID, &btn);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CToolBarCtrl::InsertButton");
    RETURN_NONE;
}

// @pymethod int|PyCToolBarCtrl|IsButtonChecked|Determine whether the specified button in a toolbar control is checked.
// @pyparm int|nID||ID of the button to check.
// @pyseemfc CToolBarCtrl|IsButtonChecked
MAKE_GET_BOOL_INT_METHOD(IsButtonChecked)

// @pymethod int|PyCToolBarCtrl|IsButtonEnabled|Determine whether the specified button in a toolbar control is enabled.
// @pyparm int|nID||ID of the button to check.
// @pyseemfc CToolBarCtrl|IsButtonEnabled
MAKE_GET_BOOL_INT_METHOD(IsButtonEnabled)

// @pymethod int|PyCToolBarCtrl|IsButtonHidden|Determine whether the specified button in a toolbar control is hidden.
// @pyparm int|nID||ID of the button to check.
// @pyseemfc CToolBarCtrl|IsButtonHidden
MAKE_GET_BOOL_INT_METHOD(IsButtonHidden)

// @pymethod int|PyCToolBarCtrl|IsButtonIndeterminate|Determine whether the specified button in a toolbar control is
// indeterminate.
// @pyparm int|nID||ID of the button to check.
// @pyseemfc CToolBarCtrl|IsButtonIndeterminate
MAKE_GET_BOOL_INT_METHOD(IsButtonIndeterminate)

// @pymethod int|PyCToolBarCtrl|IsButtonPressed|Determine whether the specified button in a toolbar control is pressed.
// @pyparm int|nID||ID of the button to check.
// @pyseemfc CToolBarCtrl|IsButtonPressed
MAKE_GET_BOOL_INT_METHOD(IsButtonPressed)

// @pymethod |PyCToolBarCtrl|PressButton|Mark or unmark the specified button as pressed.
// @pyparm int|nID||ID of the button to mark.
// @pyparm int|bEnable|1|1 to mark, 0 to unmark.
// @pyseemfc CToolBarCtrl|PressButton

MAKE_SET_INT_BOOL_METHOD(PressButton)

// @pymethod |PyCToolBarCtrl|SetBitmapSize|Set the size of the actual bitmapped images to be added to a toolbar control.
PyObject *PyCToolBarCtrl_SetBitmapSize(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    SIZE sz = {16, 15};
    if (!PyArg_ParseTuple(args, "|ii:SetBitmapSize",
                          &sz.cx,     // @pyparm int|width|16|Width of bitmap images.
                          &sz.cy)) {  // @pyparm int|height|15|Height of bitmap images.
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "|(ii):SetBitmapSize",
                              &sz.cx,   // @pyparmalt1 int|width|16|Width of bitmap images.
                              &sz.cy))  // @pyparmalt1 int|height|15|Height of bitmap images.
            return NULL;
        return NULL;
    }

    GUI_BGN_SAVE;
    BOOL ok = pTBC->SetBitmapSize(sz);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CToolBarCtrl|SetBitmapSize
        RETURN_ERR("CToolBarCtrl::SetBitmapSize");
    RETURN_NONE;
}

// @pymethod |PyCToolBarCtrl|SetButtonSize|Set the size of the buttons to be added to a toolbar control.
PyObject *PyCToolBarCtrl_SetButtonSize(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    SIZE sz = {16, 15};
    if (!PyArg_ParseTuple(args, "|ii:SetButtonSize",
                          &sz.cx,     // @pyparm int|width|16|Width of buttons
                          &sz.cy)) {  // @pyparm int|height|15|Height of buttons
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "|(ii):SetButtonSize",
                              &sz.cx,   // @pyparmalt1 int|width|16|Width of bitmap images.
                              &sz.cy))  // @pyparmalt1 int|height|15|Height of bitmap images.
            return NULL;
    }

    GUI_BGN_SAVE;
    BOOL ok = pTBC->SetButtonSize(sz);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CToolBarCtrl|SetButtonSize
        RETURN_ERR("CToolBarCtrl::SetButtonSize");
    RETURN_NONE;
}

// @pymethod |PyCToolBarCtrl|SetCmdID|Set the command identifier which will be sent to the owner window when the
// specified button is pressed.
// @pyparm int|nIndex||The zero-based index of the button whose command ID is to be set.
// @pyparm int|nID||The command ID to set the selected button to.
// @pyseemfc CToolBarCtrl|SetCmdID

MAKE_SET_INT_BOOL_METHOD(SetCmdID)

// @pymethod left, top, right, bottom|PyCToolBarCtrl|SetRows|Ask the toolbar control to resize itself to the requested
// number of rows.
PyObject *PyCToolBarCtrl_SetRows(PyObject *self, PyObject *args)
{
    CToolBarCtrl *pTBC = GetToolBarCtrl(self);
    if (pTBC == NULL)
        return NULL;
    int nRows;
    int bLarger;
    RECT r;

    if (!PyArg_ParseTuple(args, "ii:SetRows",
                          &nRows,     // @pyparm int|nRows||Requested number of rows.
                          &bLarger))  // @pyparm int|bLarger||Tells whether to use more rows or fewer rows if the
                                      // toolbar cannot be resized to the requested number of rows.
        return NULL;

    GUI_BGN_SAVE;
    pTBC->SetRows(nRows, bLarger, &r);  // @pyseemfc CToolBarCtrl|SetRows
    GUI_END_SAVE;
    return Py_BuildValue("(iiii)", r.left, r.top, r.right, r.bottom);
}

// @object PyCToolBarCtrl|A class which encapsulates an MFC <o CToolBarCtrl>.  Derived from a <o PyCWnd> object. Created
// using <om PyCToolBar.GetToolBarCtrl>
static struct PyMethodDef PyCToolBarCtrl_methods[] = {
    {"AddBitmap", PyCToolBarCtrl_AddBitmap,
     1},  // @pymeth AddBitmap|Add one or more button images to the list of button images
    {"AddButtons", PyCToolBarCtrl_AddButtons, 1},    // @pymeth AddButtons|Add one or more buttons
    {"AddStrings", PyCToolBarCtrl_AddStrings, 1},    // @pymeth AddStrings|Add one or more strings
    {"AutoSize", PyCToolBarCtrl_AutoSize, 1},        // @pymeth AutoSize|Resize the entire toolbar
    {"CheckButton", PyCToolBarCtrl_CheckButton, 1},  // @pymeth CheckButton|Check or clear a button
    {"CommandToIndex", PyCToolBarCtrl_CommandToIndex,
     1},  // @pymeth CommandToIndex|Retrieve the zero-based index for the button associated with the specified command
          // identifier.
    {"CreateWindow", PyCToolBarCtrl_CreateWindow, 1},  // @pymeth CreateWindow|Create the actual control
    {"Customize", PyCToolBarCtrl_Customize, 1},        // @pymeth Customize|Display the customize toolbar dialog box
    {"DeleteButton", PyCToolBarCtrl_DeleteButton, 1},  // @pymeth DeleteButton|Delete a button from the toolbar control
    {"EnableButton", PyCToolBarCtrl_EnableButton,
     1},  // @pymeth EnableButton|Enable or disable a toolbar control button.
    {"GetBitmapFlags", PyCToolBarCtrl_GetBitmapFlags,
     1},  // @pymeth GetBitmapFlags|Retrieve the bitmap flags from the toolbar.
    {"GetButton", PyCToolBarCtrl_GetButton,
     1},  // @pymeth GetButton|Retrieve information about the specified button in a toolbar control.
    {"GetButtonCount", PyCToolBarCtrl_GetButtonCount,
     1},  // @pymeth GetButtonCount|Retrieve a count of the buttons currently in the toolbar control.
    {"GetItemRect", PyCToolBarCtrl_GetItemRect,
     1},  // @pymeth GetItemRect|Retrieve the bounding rectangle of a button in a toolbar control.
    {"GetRows", PyCToolBarCtrl_GetRows,
     1},  // @pymeth GetRows|Retrieve the number of rows of buttons currently displayed
    {"HideButton", PyCToolBarCtrl_HideButton,
     1},  // @pymeth HideButton|Hide or show the specified button in a toolbar control.
    {"Indeterminate", PyCToolBarCtrl_Indeterminate,
     1},  // @pymeth Indeterminate|Hide or show the specified button in a toolbar control.
    {"InsertButton", PyCToolBarCtrl_InsertButton, 1},  // @pymeth InsertButton|Insert a button into a toolbar control.
    {"IsButtonChecked", PyCToolBarCtrl_IsButtonChecked, 1},  // @pymeth IsButtonChecked|See if a button is checked.
    {"IsButtonEnabled", PyCToolBarCtrl_IsButtonEnabled, 1},  // @pymeth IsButtonEnabled|See if a button is enabled.
    {"IsButtonHidden", PyCToolBarCtrl_IsButtonHidden, 1},    // @pymeth IsButtonHidden|See if a button is checked.
    {"IsButtonIndeterminate", PyCToolBarCtrl_IsButtonIndeterminate,
     1},  // @pymeth IsButtonIndeterminate|See if a button is Indeterminate.
    {"IsButtonPressed", PyCToolBarCtrl_IsButtonPressed, 1},  // @pymeth IsButtonPressed|See if a button is pressed.
    {"PressButton", PyCToolBarCtrl_PressButton,
     1},  // @pymeth PressButton|Mark or unmark the specified button as pressed.
    {"SetBitmapSize", PyCToolBarCtrl_SetBitmapSize,
     1},  // @pymeth SetBitmapSize|Set the size of the actual bitmapped images to be added to a toolbar control.
    {"SetButtonSize", PyCToolBarCtrl_SetButtonSize,
     1},  // @pymeth SetButtonSize|Set the size of the actual buttons to be added to a toolbar control.
    {"SetCmdID", PyCToolBarCtrl_SetCmdID, 1},  // @pymeth SetCmdID|Set the command identifier which will be sent to the
                                               // owner window when the specified button is pressed.
    {"SetRows", PyCToolBarCtrl_SetRows,
     1},  // @pymeth SetRows|Ask the toolbar control to resize itself to the requested number of rows.
    {NULL, NULL}};

ui_type_CObject PyCToolBarCtrl::type("PyCToolBarCtrl", &PyCWnd::type, RUNTIME_CLASS(CToolBarCtrl),
                                     sizeof(PyCToolBarCtrl), PYOBJ_OFFSET(PyCToolBarCtrl), PyCToolBarCtrl_methods,
                                     GET_PY_CTOR(PyCToolBarCtrl));

///////////////////////////////////////////////////////////////////////////////
//
// CStatusBar object
//
/* static */ CStatusBar *PyCStatusBar::GetStatusBar(PyObject *self)
{
    return (CStatusBar *)GetGoodCppObject(self, &type);
}

// @pymethod <o PyCStatusBar>|win32ui|CreateStatusBar|Creates a statusbar object.
PyObject *PyCStatusBar::create(PyObject *self, PyObject *args)
{
    PyObject *parent;
    int style = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM;
    int id = AFX_IDW_STATUS_BAR;
    int ctrlStyle = 0;
    if (!PyArg_ParseTuple(
            args, "O|iii:CreateStatusBar",
            &parent,  // @pyparm <o PyCWnd>|parent||The parent window for the status bar.
            &style,   // @pyparm int|style|afxres.WS_CHILD \| afxres.WS_VISIBLE \| afxres.CBRS_BOTTOM|The style for the
                      // status bar.
            &id,      // @pyparm int|windowId|afxres.AFX_IDW_STATUS_BAR|The child window ID.
            &ctrlStyle))  // @pyparm int|ctrlStype|0|Additional styles for the creation of the embedded <o
                          // PyCStatusBarCtrl> object. <nl>Status bar styles supported are:<nl>commctrl.SBARS_SIZEGRIP -
                          // The status bar control includes a sizing grip at the right end of the status bar. A sizing
                          // grip is similar to a sizing border; it is a rectangular area that the user can click and
                          // drag to resize the parent window. <nl>commctrl.SBT_TOOLTIPS - The status bar supports
                          // tooltips.
        return NULL;
    if (!ui_base_class::is_uiobject(parent, &PyCWnd::type)) {
        RETURN_ERR("The parent param must be a window object.");
    }

    // @comm You must ensure no 2 status bars share the same ID.
    CString error;
    CStatusBar *sb = new CPythonStatusBar();
    CFrameWnd *frame = (CFrameWnd *)PyCWnd::GetPythonGenericWnd(parent, &PyCFrameWnd::type);
    if (frame == NULL)
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = sb->CreateEx(frame, ctrlStyle, style, id);  // @pyseemfc CStatusBar|CreateEx
    GUI_END_SAVE;
    if (!ok) {
        delete sb;
        RETURN_API_ERR("CStatusBar.CreateEx");
    }
    sb->m_bAutoDelete =
        TRUE;  // let MFC handle deletion??? really?? Cloned from toolbar - not so sure about status bar!!
    return ui_assoc_object::make(PyCStatusBar::type, sb)->GetGoodRet();
}

// @pymethod (id, style, width)|PyCStatusBar|GetPaneInfo|Returns the id, style, and width of the indicator pane at the
// location specified by index.
PyObject *PyCStatusBar_GetPaneInfo(PyObject *self, PyObject *args)
{
    CStatusBar *pStatusBar = PyCStatusBar::GetStatusBar(self);
    if (!pStatusBar)
        return NULL;

    UINT nIndex;
    if (!PyArg_ParseTuple(args, "i:GetPaneInfo",
                          &nIndex))  // @pyparm int|index||Index of the pane whose information is to be retrieved.
        return NULL;

    UINT nID;
    UINT nStyle;
    int cxWidth;

    GUI_BGN_SAVE;
    pStatusBar->GetPaneInfo(nIndex, nID, nStyle, cxWidth);  // @pyseemfc CStatusBar|GetPaneInfo
    GUI_END_SAVE;

    return Py_BuildValue("(iii)", nID, nStyle, cxWidth);
}

// @pymethod <o PyCStatusBarCtrl>|PyCStatusBar|GetStatusBarCtrl|Gets the statusbar control object for the statusbar.
PyObject *PyCStatusBar_GetStatusBarCtrl(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CStatusBar *pStatusBar = PyCStatusBar::GetStatusBar(self);
    if (!pStatusBar)
        return NULL;

    CStatusBarCtrl &rSBC = pStatusBar->GetStatusBarCtrl();  // @pyseemfc CStatusBar|GetStatusBarCtrl
    // Note that below we take the address of rTBC because it's a reference and not a pointer
    // and ui_assoc_object::make expects a pointer.
    // We need to create a new class and not do a map lookup because in MFC CToolBarCtrl is
    // simply a casted CToolBarCtrl (afxext.inl) so the lookup will return the PyCToolBar object
    // which will fail the type tests.
    return ui_assoc_object::make(PyCStatusBarCtrl::type, &rSBC, true)->GetGoodRet();
}

// @pymethod |PyCStatusBar|SetIndicators|Sets each indicator's ID.
PyObject *PyCStatusBar_SetIndicators(PyObject *self, PyObject *args)
{
    PyObject *buttons;
    if (!PyArg_ParseTuple(args, "O:SetIndicators",
                          &buttons)  // @pyparm tuple|indicators||A tuple containing the ID's of the indicators.
        || !PySequence_Check(buttons))
        RETURN_ERR("SetIndicators requires a tuple of IDs");
    CStatusBar *pSB = PyCStatusBar::GetStatusBar(self);
    if (!pSB)
        return NULL;
    // convert indicator sequence to array
    Py_ssize_t num_buttons = PySequence_Length(buttons);
    UINT *button_list = new UINT[num_buttons];
    PyObject *o;
    for (Py_ssize_t i = 0; i < num_buttons; i++) {
        o = PySequence_GetItem(buttons, i);
        if (!PyInt_Check(o)) {
            Py_XDECREF(o);
            delete button_list;
            RETURN_ERR("SetIndicators expected integer button ids.");
        }
        button_list[i] = PyInt_AsLong(o);
        Py_DECREF(o);
    }
    BOOL rc = pSB->SetIndicators(button_list, PyWin_SAFE_DOWNCAST(num_buttons, Py_ssize_t, int));
    delete button_list;
    if (!rc)
        RETURN_API_ERR("PyCStatusBar.SetIndicators");
    RETURN_NONE;
}

// @pymethod |PyCStatusBar|SetPaneInfo|Sets the specified indicator pane to a new ID, style, and width.
PyObject *PyCStatusBar_SetPaneInfo(PyObject *self, PyObject *args)
{
    CStatusBar *pStatusBar = PyCStatusBar::GetStatusBar(self);
    if (!pStatusBar)
        return NULL;

    UINT nIndex;
    UINT nID;
    UINT nStyle;
    int cxWidth;
    if (!PyArg_ParseTuple(
            args, "iiii:SetPaneInfo",
            &nIndex,  // @pyparm int|index||Index of the indicator pane whose style is to be set.
            &nID,     // @pyparm int|id||New ID for the indicator pane.
            &nStyle,  // @pyparm int|style||New style for the indicator pane.<nl>The following indicator styles are
                      // supported:<nl>afxres.SBPS_NOBORDERS - No 3-D border around the pane.<nl>afxres.SBPS_POPOUT -
                      // Reverse border so that text "pops out."<nl>afxres.SBPS_DISABLED - Do not draw
                      // text.<nl>afxres.SBPS_STRETCH - Stretch pane to fill unused space. Only one pane per status bar
                      // can have this style.<nl>afxres.SBPS_NORMAL - No stretch, borders, or pop-out.
            &cxWidth))  // @pyparm int|width||New width for the indicator pane.
        return NULL;

    GUI_BGN_SAVE;
    pStatusBar->SetPaneInfo(nIndex, nID, nStyle, cxWidth);  // @pyseemfc CStatusBar|SetPaneInfo
    GUI_END_SAVE;

    RETURN_NONE;
}

// @object PyCStatusBar|A class which encapsulates an MFC <o CStatusBar>.  Derived from a <o PyCControlBar> object.
static struct PyMethodDef PyCStatusBar_methods[] = {
    {"GetPaneInfo", PyCStatusBar_GetPaneInfo,
     1},  // @pymeth GetPaneInfo|Returns indicator ID, style, and width for a given pane index.
    {"GetStatusBarCtrl", PyCStatusBar_GetStatusBarCtrl,
     1},  // @pymeth GetStatusBarCtrl|Returns the status bar control object associated with the status bar.
    {"SetIndicators", PyCStatusBar_SetIndicators, 1},  // @pymeth SetIndicators|Sets each indicator's ID.
    {"SetPaneInfo", PyCStatusBar_SetPaneInfo,
     1},  // @pymeth SetPaneInfo|Sets indicator ID, style, and width for a given pane index.
    {NULL, NULL}};

ui_type_CObject PyCStatusBar::type("PyCStatusBar", &PyCControlBar::type, RUNTIME_CLASS(CStatusBar),
                                   sizeof(PyCStatusBar), PYOBJ_OFFSET(PyCStatusBar), PyCStatusBar_methods,
                                   GET_PY_CTOR(PyCStatusBar));
