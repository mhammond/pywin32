/*

    win32 view data type

    Created Dec 1995, Mark Hammond (MHammond@skippinet.com.au)

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
#include "win32prinfo.h"
#include "win32control.h"
#include "win32ctrlList.h"
#include "win32ctrlTree.h"

CPythonEditView *GetEditViewPtr(PyObject *self)
{
    // need to only rtti check on CView, as CPythonEditView is not derived from CPythonView.
    return (CPythonEditView *)PyCWnd::GetGoodCppObject(self, &PyCView::type);
}

CView *PyCView::GetViewPtr(PyObject *self)
{
    // ditto!
    return (CView *)PyCWnd::GetGoodCppObject(self, &PyCView::type);
}

CScrollView *PyCScrollView::GetViewPtr(PyObject *self)
{
    // ditto!
    return (CScrollView *)PyCWnd::GetGoodCppObject(self, &PyCScrollView::type);
}

CListView *GetListViewPtr(PyObject *self)
{
    // ditto!
    return (CListView *)PyCWnd::GetGoodCppObject(self, &PyCListView::type);
}

CTreeView *GetTreeViewPtr(PyObject *self)
{
    // ditto!
    return (CTreeView *)PyCWnd::GetGoodCppObject(self, &PyCTreeView::type);
}

//
// Hack to get around protected members
//
class CProtectedView : public CView {
   public:
    void SetDocument(CDocument *pDoc) { m_pDocument = pDoc; }
    void BaseOnActivateView(BOOL bActivate, CView *pActivateView, CView *pDeactiveView)
    {
        CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
    }
    BOOL BasePreCreateWindow(CREATESTRUCT &cs) { return CView::PreCreateWindow(cs); }
    void BaseOnFilePrint() { CView::OnFilePrint(); }
    void BaseOnFilePrintPreview() { CView::OnFilePrintPreview(); }
    void BaseOnBeginPrinting(CDC *dc, CPrintInfo *pInfo) { CView::OnBeginPrinting(dc, pInfo); }
    void BaseOnEndPrinting(CDC *dc, CPrintInfo *pInfo) { CView::OnEndPrinting(dc, pInfo); }
    BOOL DoPreparePrinting(CPrintInfo *pInfo) { return CView::DoPreparePrinting(pInfo); }
    BOOL BaseOnPreparePrinting(CPrintInfo *pInfo) { return CView::OnPreparePrinting(pInfo); }
    void BaseOnPrepareDC(CDC *dc, CPrintInfo *pInfo) { CView::OnPrepareDC(dc, pInfo); }
    int BaseOnMouseActivate(CWnd *pWnd, UINT ht, UINT msg) { return CView::OnMouseActivate(pWnd, ht, msg); }
};

// And an extra hack for splitters!
void PyWin_SetViewDocument(CView *pView, CDocument *pDoc) { ((CProtectedView *)pView)->SetDocument(pDoc); }

// Hacks to get around protected members.
class CProtectedScrollView : public CScrollView {
   public:
    void BaseUpdateBars() { CScrollView::UpdateBars(); }
};

// @pymethod |PyCView|CreateWindow|Creates the window for a view.
static PyObject *PyCView_create_window(PyObject *self, PyObject *args)
{
    CView *pView = PyCView::GetViewPtr(self);
    if (!pView) {
        // PyCView::GetViewPtr will trace to the debug device - just so I know it is an OK one!
        TRACE("  ignore warning - RTTI Error detected and handled!");
        PyErr_Clear();
        pView = GetEditViewPtr(self);
    }
    if (!pView)
        return NULL;

    if (pView->m_hWnd != NULL)
        RETURN_ERR("The view already has a window");

    PyObject *parent;
    int id = AFX_IDW_PANE_FIRST;
    int style = AFX_WS_DEFAULT_VIEW;
    CRect rect(0, 0, 0, 0);
    if (!PyArg_ParseTuple(args, "O|ii(iiii):CreateWindow",
                          &parent,  // @pyparm <o PyCWnd>|parent||The parent window (usually a frame)
                          &id,      // @pyparm int|id|win32ui.AFX_IDW_PANE_FIRST|The child ID for the view
                          &style,   // @pyparm int|style|win32ui.AFX_WS_DEFAULT_VIEW|The style for the view
                          // @pyparm (left, top, right, bottom)|rect|(0,0,0,0)|The default position of the window.
                          &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;

    if (!ui_base_class::is_uiobject(parent, &PyCWnd::type))
        RETURN_TYPE_ERR("Argument must be a PyCWnd");
    CWnd *pWnd = GetWndPtr(parent);
    if (pWnd == NULL)
        return NULL;

    CCreateContext context;
    GUI_BGN_SAVE;
    context.m_pCurrentDoc = pView->GetDocument();
    //	if (!context.m_pCurrentDoc)
    //		RETURN_ERR("There is no document attached to the view");

    // must reset doc to NULL, else MFC asserts all over the place!
    // Create() resets this value (via the CreateContext)
    ((CProtectedView *)pView)->SetDocument(NULL);

    BOOL ok;
    ok = pView->Create(NULL, NULL, style, rect, pWnd, id, &context);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("Create() view failed\n");
    RETURN_NONE;
}

// @pymethod <o PyCDocument>|PyCView|GetDocument|Returns the document for a view.
static PyObject *PyCView_get_document(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CView *view = PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CDocument *pDoc = view->GetDocument();
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet();
}

// @pymethod |PyCView|OnInitialUpdate|Calls the underlying MFC OnInitialUpdate method.
PyObject *PyCView_on_initial_update(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnInitialUpdate);
    CView *view = PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    // @xref <vm PyCView.OnInitialUpdate>
    GUI_BGN_SAVE;
    view->CView::OnInitialUpdate();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCView|OnActivateView|Calls the underlying MFC OnActivateView method.
PyObject *PyCView_on_activate_view(PyObject *self, PyObject *args)
{
    // @xref <vm PyCView.OnActivateView>
    // @pyparm int|activate||Indicates whether the view is being activated or deactivated.
    // @pyparm <o PyCView>|activateView||The view object that is being activated.
    // @pyparm <o PyCView>|DeactivateView||The view object that is being deactivated.
    int activate;
    PyObject *obActivate, *obDeactivate;
    if (!PyArg_ParseTuple(args, "iOO:OnActivateView", &activate, &obActivate, &obDeactivate))
        return NULL;
    CView *view = PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;

    CView *pActivate = obActivate == Py_None ? NULL : PyCView::GetViewPtr(obActivate);
    if (PyErr_Occurred())
        return NULL;
    CView *pDevactive = obDeactivate == Py_None ? NULL : PyCView::GetViewPtr(obDeactivate);
    if (PyErr_Occurred())
        return NULL;
    GUI_BGN_SAVE;
    ((CProtectedView *)view)->BaseOnActivateView(activate, pActivate, pDevactive);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCView|OnMouseActivate|Calls the base MFC OnMouseActivate function.
// @xref <vm PyCWnd.OnMouseActivate>
static PyObject *PyCView_on_mouse_activate(PyObject *self, PyObject *args)
{
    extern CWnd *GetWndPtrFromParam(PyObject * ob, ui_type_CObject & type);

    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
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
    UINT rc = view->BaseOnMouseActivate(pWndArg, ht, msg);
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
}

// @pymethod tuple|PyCView|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
// @xref <vm PyCWnd.PreCreateWindow>
PyObject *PyCView_pre_create_window(PyObject *self, PyObject *args)
{
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = view->BasePreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CView::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod |PyCView|OnFilePrint|Calls the underlying MFC OnFilePrint method.
PyObject *PyCView_on_file_print(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnFilePrint);
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->BaseOnFilePrint();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCView|OnFilePrintPreview|Calls the underlying MFC OnFilePrintPreview method.
PyObject *PyCView_on_file_print_preview(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnFilePrint);
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->BaseOnFilePrintPreview();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCView|DoPreparePrinting|Invoke the Print dialog box and create a printer device context.
PyObject *PyCView_do_prepare_printing(PyObject *self, PyObject *args)
{
    PyObject *pyInfo;
    if (!PyArg_ParseTuple(args, "O:DoPreparePrinting", &pyInfo))
        return NULL;
    CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(pyInfo);
    if (!pInfo)
        return NULL;
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = view->DoPreparePrinting(pInfo);
    GUI_END_SAVE;
    return Py_BuildValue("i", ret);
    // @comm This function is usually called from <vm PyCView.OnPreparePrinting>
}

// @pymethod int|PyCView|OnPreparePrinting|Calls the underlying MFC OnPreparePrinting method.
// @xref <vm PyCView.OnPreparePrinting>
PyObject *PyCView_on_prepare_printing(PyObject *self, PyObject *args)
{
    PyObject *pyInfo;
    if (!PyArg_ParseTuple(args, "O:OnPreparePrinting", &pyInfo))
        return NULL;
    CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(pyInfo);
    if (!pInfo)
        return NULL;
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = view->BaseOnPreparePrinting(pInfo);
    GUI_END_SAVE;
    return Py_BuildValue("i", ret);
}

// @pymethod |PyCView|OnPrepareDC|Calls the underlying MFC OnPrepareDC method.
// @xref <vm PyCView.OnPrepareDC>
PyObject *PyCView_on_prepare_dc(PyObject *self, PyObject *args)
{
    PyObject *pyInfo, *pyDC;
    if (!PyArg_ParseTuple(args, "OO:OnPrepareDC", &pyDC, &pyInfo))
        return NULL;
    CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(pyInfo);
    if (!pInfo)
        return NULL;
    CDC *pDC = ui_dc_object::GetDC(pyDC);
    if (!pDC)
        RETURN_ERR("The DC is invalid");
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->BaseOnPrepareDC(pDC, pInfo);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCView|OnBeginPrinting|Calls the underlying MFC OnBeginPrinting method.
// @xref <vm PyCView.OnBeginPrinting>
PyObject *PyCView_on_begin_printing(PyObject *self, PyObject *args)
{
    PyObject *pyDC;
    PyObject *pyInfo;

    if (!PyArg_ParseTuple(args, "OO:OnBeginPrinting", &pyDC, &pyInfo))
        return NULL;
    if (!ui_base_class::is_uiobject(pyDC, &ui_dc_object::type))
        RETURN_TYPE_ERR("The first param must be a PyCDC object");
    CDC *pDC = ui_dc_object::GetDC(pyDC);
    if (!pDC)
        RETURN_ERR("The DC is invalid");
    CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(pyInfo);
    if (!pInfo)
        return NULL;
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->BaseOnBeginPrinting(pDC, pInfo);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCView|OnEndPrinting|Calls the underlying MFC OnEndPrinting method.
// @xref <vm PyCView.OnEndPrinting>
PyObject *PyCView_on_end_printing(PyObject *self, PyObject *args)
{
    PyObject *pyDC;
    PyObject *pyInfo;

    if (!PyArg_ParseTuple(args, "OO:OnEndPrinting", &pyDC, &pyInfo))
        return NULL;
    if (!ui_base_class::is_uiobject(pyDC, &ui_dc_object::type))
        RETURN_TYPE_ERR("The first param must be a PyCDC object");
    CDC *pDC = ui_dc_object::GetDC(pyDC);
    if (!pDC)
        RETURN_ERR("The DC is invalid");
    CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(pyInfo);
    if (!pInfo)
        return NULL;
    CProtectedView *view = (CProtectedView *)PyCView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    view->BaseOnEndPrinting(pDC, pInfo);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCView|A class which implements a generic CView.  Derived from a <o PyCWnd> object.
static struct PyMethodDef PyCView_methods[] = {
    {"CreateWindow", PyCView_create_window, 1},  // @pymeth CreateWindow|Create the window for a view.
    {"GetDocument", PyCView_get_document, 1},    // @pymeth GetDocument|Returns the document for a view.
    {"OnActivateView", PyCView_on_activate_view,
     1},  // @pymeth OnActivateView|Calls the underlying MFC OnActivateView method.
    {"OnInitialUpdate", PyCView_on_initial_update,
     1},  // @pymeth OnInitialUpdate|Calls the underlying MFC OnInitialUpdate method.
    {"OnMouseActivate", PyCView_on_mouse_activate,
     1},  // @pymeth OnMouseActivate|Calls the underlying MFC OnMouseActivate method.
    {"PreCreateWindow", PyCView_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"OnFilePrint", PyCView_on_file_print, 1},  // @pymeth OnFilePrint|Calls the underlying MFC OnFilePrint method.
    {"OnFilePrintPreview", PyCView_on_file_print_preview,
     1},  // @pymeth OnFilePrint|Calls the underlying MFC OnFilePrintPreview method.
    {"DoPreparePrinting", PyCView_do_prepare_printing,
     1},  // @pymeth DoPreparePrinting|Calls the underlying MFC DoPreparePrinting method.
    {"OnBeginPrinting", PyCView_on_begin_printing,
     1},  // @pymeth OnBeginPrinting|Calls the underlying MFC OnBeginPrinting method.
    {"OnEndPrinting", PyCView_on_end_printing,
     1},  // @pymeth OnEndPrinting|Calls the underlying MFC OnEndPrinting method.
    {NULL, NULL}};

// View type
ui_type_CObject PyCView::type("PyCView",
                              &PyCWnd::type,  // @base PyCView|PyCWnd
                              RUNTIME_CLASS(CView), sizeof(PyCView), PYOBJ_OFFSET(PyCView), PyCView_methods, NULL);

// @pymethod <o PyCScrollView>|win32ui|CreateView|Creates a generic view object.
PyObject *PyCScrollView::create(PyObject *self, PyObject *args)
{
    PyObject *doc;
    // @pyparm <o PyCDocument>|doc||The document to use with the view.
    if (!PyArg_ParseTuple(args, "O:CreateView", &doc))
        return NULL;
    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    GUI_BGN_SAVE;
    CPythonView *pView = new CPythonView();
    ((CProtectedView *)pView)->SetDocument(pDoc);
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCScrollView::type, pView, TRUE);
}

// @pymethod <o PyCDC>|PyCScrollView|GetDC|Gets the view's current DC.
static PyObject *ui_view_get_dc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;

    // create MFC device context
    GUI_BGN_SAVE;
    CDC *pDC = view->GetDC();
    if (pDC == NULL) {
        GUI_BLOCK_THREADS
        RETURN_ERR("Could not get the DC for the view.");
    }

    // update logical 0,0 position based on scroll position
    CPoint offset = view->GetDeviceScrollPosition();
    // really should be SetDeviceOrgEx (bad MS names, don'cha know)
    SetViewportOrgEx(pDC->GetSafeHdc(), -offset.x, -offset.y, NULL);
    GUI_END_SAVE;

    // create Python device context
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC);
    return dc;
}

// @pymethod (x,y)|PyCScrollView|GetDeviceScrollPosition|Returns the positon of the scroll bars in device units.
static PyObject *ui_view_get_dscroll_pos(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;

    GUI_BGN_SAVE;
    CPoint pos = view->GetDeviceScrollPosition();
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", pos.x, pos.y);
}

// @pymethod |PyCScrollView|ScrollToPosition|Scrolls to a given point in the view.
static PyObject *ui_view_scroll_to_position(PyObject *self, PyObject *args)
{
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;

    POINT position;
    // @pyparm (x,y)|position||The position to scroll to.
    if (!PyArg_ParseTuple(args, "(ii):ScrollToPosition", &position.x, &position.y))
        return NULL;

    GUI_BGN_SAVE;
    view->ScrollToPosition(position);
    GUI_END_SAVE;

    RETURN_NONE;
}

// @pymethod tuple|PyCScrollView|ResizeParentToFit|Lets the size of a view dictate the size of its frame window.
PyObject *ui_view_resize_parent_to_fit(PyObject *self, PyObject *args)
{
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    BOOL bShrink = TRUE;
    //@pyparm int|bShrinkOnly|1|The kind of resizing to perform. The default value, TRUE, shrinks the frame window if
    // appropriate.
    if (!PyArg_ParseTuple(args, "|i:ResizeParentToFit", &bShrink))
        return NULL;
    GUI_BGN_SAVE;
    view->ResizeParentToFit(bShrink);
    GUI_END_SAVE;
    // @comm This is recommended only for views in MDI child frame windows.
    // <nl>Use ResizeParentToFit in the OnInitialUpdate handler function of your View class.
    // <nl>You must ensure the parent's <om PyCFrameWnd.RecalcLayout> is called before using this method.
    RETURN_NONE;
}

// @pymethod |PyCScrollView|SetScaleToFitSize|Scales the viewport size to the current window size automatically.
static PyObject *ui_view_set_scale_to_fit_size(PyObject *self, PyObject *args)
{
    CScrollView *pView = PyCScrollView::GetViewPtr(self);
    if (pView == NULL)
        return NULL;
    SIZE size;
    // @pyparm (x,y)|size||The horizontal and vertical sizes to which the view is to be scaled. The scroll view's size
    // is measured in logical units.
    if (!PyArg_ParseTuple(args, "(ll)", &size.cx, &size.cy))
        return NULL;
    GUI_BGN_SAVE;
    pView->SetScaleToFitSize(size);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCScrollView|SetScrollSizes|Sets the sizes of the scroll bars
static PyObject *ui_view_set_scroll_sizes(PyObject *self, PyObject *args)
{
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    int map_mode;
    CSize total, page = CScrollView::sizeDefault, line = CScrollView::sizeDefault;
    if (!PyArg_ParseTuple(args, "i(ii)|(ii)(ii):SetScrollSizes",
                          &map_mode,             // @pyparm int|mapMode||The mapping mode for this view.
                          &total.cx, &total.cy,  // @pyparm (x,y)|sizeTotal||The total size of the view.  Sizes are in
                                                 // logical units.  Both x and y must be greater than zero.
                          &page.cx, &page.cy,    // @pyparm (x,y)|sizePage|win32ui.rectDefault|The number of untils to
                                                 // scroll in response to a page-down command.
                          &line.cx, &line.cy))   // @pyparm (x,y)|sizePage|win32ui.rectDefault|The number of untils to
                                                 // scroll in response to a line-down command.
        return NULL;

    GUI_BGN_SAVE;
    //  BOOL save = view->SetDynamicScrollBars (TRUE);
    view->SetScrollSizes(map_mode, total, page, line);
    //  view->SetDynamicScrollBars (save);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod (x,y)|PyCScrollView|GetScrollPosition|Returns the current position of the scroll bars (in logical units).
static PyObject *PyCView_get_scroll_pos(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CPoint pos = view->GetScrollPosition();
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", pos.x, pos.y);
}
// @pymethod (x,y)|PyCScrollView|GetTotalSize|Returns the total size of the view in logical units.
static PyObject *PyCView_get_total_size(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CSize size = view->GetTotalSize();
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", size.cx, size.cy);
}

// @pymethod |PyCScrollView|UpdateBars|Update the scroll bars state
static PyObject *ui_view_update_bars(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CScrollView *view = PyCScrollView::GetViewPtr(self);
    if (view == NULL)
        return NULL;
    GUI_BGN_SAVE;
    ((CProtectedScrollView *)view)->BaseUpdateBars();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCScrollView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonView, &PyCScrollView::type, self, args);
}

// @object PyCScrollView|A class which implements a generic CScrollView.  Derived from a <o PyCView> object.
static struct PyMethodDef PyCScrollView_methods[] = {
    {"GetDeviceScrollPosition", ui_view_get_dscroll_pos,
     1},  // @pymeth GetDeviceScrollPosition|Return the position of the scroll bars (device units).
    {"GetDC", ui_view_get_dc, 1},  // @pymeth GetDC|Get the views current <o PyCDC>
    {"GetScrollPosition", PyCView_get_scroll_pos,
     1},  // @pymeth GetScrollPosition|Return the position of the scroll bars (logical units).
    {"GetTotalSize", PyCView_get_total_size, 1},  // @pymeth GetTotalSize|Return the total size of the views.
    {"OnCommand", PyCView_on_command, 1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {"ResizeParentToFit", ui_view_resize_parent_to_fit,
     1},  // @pymeth ResizeParentToFit|Call ResizeParentToFit to let the size of your view dictate the size of its frame
          // window.
    {"SetScaleToFitSize", ui_view_set_scale_to_fit_size,
     1},  // @pymeth SetScaleToFitSize|Scales the viewport size to the current window size automatically.
    {"ScrollToPosition", ui_view_scroll_to_position, 1},  // @pymeth ScrollToPosition|Scroll to a specified point.
    {"SetScrollSizes", ui_view_set_scroll_sizes, 1},      // @pymeth SetScrollSizes|Set the scrolling sizes.
    {"UpdateBars", ui_view_update_bars, 1},               // @pymeth UpdateBars|Update the scroll bar state.
    {NULL, NULL}};

// View type
ui_type_CObject PyCScrollView::type("PyCScrollView",
                                    &PyCView::type,  // @base PyCScrollView|PyCView
                                    RUNTIME_CLASS(CScrollView), sizeof(PyCScrollView), PYOBJ_OFFSET(PyCScrollView),
                                    PyCScrollView_methods, GET_PY_CTOR(PyCScrollView));

///////////////////////////////////////
//
// Control View Methods
//
// inherited from view
//
///////////////////////////////////////

// @pymethod <o PyCCtrlView>|win32ui|CreateCtrlView|Creates a control view object.
PyObject *PyCCtrlView::create(PyObject *self, PyObject *args)
{
    PyObject *doc;
    TCHAR *szClass;
    PyObject *obClass;
    int style = 0;
    // @pyparm <o PyCDocument>|doc||The document.
    // @pyparm string|className||The class name of the control
    // @pyparm int|style|0|Additional style bits
    if (!PyArg_ParseTuple(args, "OO|i:CreateCtrlView", &doc, &obClass, &style))
        return NULL;
    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    CCtrlView *pView;
    if (!PyWinObject_AsTCHAR(obClass, &szClass, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pView = new CPythonCtrlView(szClass, style);
    ((CProtectedView *)pView)->SetDocument(pDoc);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szClass);
    return ui_assoc_object::make(PyCCtrlView::type, pView, TRUE);
}

// @pymethod |PyCCtrlView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCCtrlView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonCtrlView, &PyCCtrlView::type, self, args);
}

// @object PyCCtrlView|A class which implementes a CCtrlView (ie, a view based on a dialog resource.
static struct PyMethodDef PyCCtrlView_methods[] = {
    {"OnCommand", PyCCtrlView_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {NULL, NULL}};

PyCCtrlView_Type PyCCtrlView::type("PyCCtrlView",
                                   &PyCView::type,  // @base PyCCtrlView|PyCView
                                   &PyCWnd::type, RUNTIME_CLASS(CCtrlView), sizeof(PyCCtrlView),
                                   PYOBJ_OFFSET(PyCCtrlView), PyCCtrlView_methods, GET_PY_CTOR(PyCCtrlView));

/////////////////////////////////////////////////////////////////////
//
// Edit View object
//
//////////////////////////////////////////////////////////////////////
// @pymethod <o PyCEditView>|win32ui|CreateEditView|Creates a PyEditView object.
PyObject *PyCEditView::create(PyObject *self, PyObject *args)
{
    PyObject *doc;
    // @pyparm <o PyCDocument>|doc||The document to use with the view.
    if (!PyArg_ParseTuple(args, "O:CreateEditView", &doc))
        return NULL;
    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    CPythonEditView *pView = new CPythonEditView();
    ((CProtectedView *)pView)->SetDocument(pDoc);
    return ui_assoc_object::make(PyCEditView::type, pView, TRUE);
}

// @pymethod <o PyCEditCtrl>|PyCEditView|GetEditCtrl|returns the underlying edit control object.
static PyObject *ui_edit_get_edit_ctrl(PyObject *self, PyObject *args)
{
    CPythonEditView *pView = GetEditViewPtr(self);
    if (!pView)
        return NULL;
    GUI_BGN_SAVE;
    CEdit &ed = pView->GetEditCtrl();
    GUI_END_SAVE;
    return ui_assoc_object::make(UITypeFromCObject(&ed), &ed)->GetGoodRet();
}

// @pymethod |PyCEditView|SetModifiedFlag|Sets the modified flag for the view's document.
static PyObject *ui_edit_window_set_modified_flag(PyObject *self, PyObject *args)
{
    CPythonEditView *pView = GetEditViewPtr(self);
    if (!pView)
        return NULL;
    BOOL bState = TRUE;
    // @pyparm int|bModified|1|The modified state to set.
    if (!PyArg_ParseTuple(args, "|i:SetModifiedFlag", &bState))
        return NULL;
    GUI_BGN_SAVE;
    pView->GetDocument()->SetModifiedFlag(bState);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCEditView|IsModified|Indicates if the view's document has the modified flag set.
static PyObject *ui_edit_window_is_modified(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CPythonEditView *pView = GetEditViewPtr(self);
    if (!pView)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pView->GetDocument()->IsModified();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod |PyCEditView|LoadFile|Loads a file into the view.
static PyObject *ui_edit_window_load_file(PyObject *self, PyObject *args)
{
    USES_CONVERSION;
    TCHAR *fileName;
    PyObject *obfileName;
    // @pyparm string|fileName||The name of the file to be loaded.
    if (!PyArg_ParseTuple(args, "O:LoadFile", &obfileName))
        return NULL;

    CPythonEditView *pView;
    if (!(pView = GetEditViewPtr(self)))
        return NULL;
    if (!PyWinObject_AsTCHAR(obfileName, &fileName, FALSE))
        return NULL;
    CFile file;
    CFileException fe;
    if (!file.Open(fileName, CFile::modeRead | CFile::shareDenyWrite, &fe)) {
        PyWinObject_FreeTCHAR(fileName);
        long errCode = fe.m_lOsError;
        CString csMessage = GetAPIErrorString(errCode);
        if (csMessage.GetLength()) {
            LPTSTR msg = csMessage.GetBuffer(csMessage.GetLength());
            PyErr_SetString(PyExc_IOError, T2A(msg));
            csMessage.ReleaseBuffer();
        }
        else
            PyErr_SetString(PyExc_IOError, "Unknown IO error?");
        return NULL;
    }
    PyWinObject_FreeTCHAR(fileName);

    GUI_BGN_SAVE;
    pView->DeleteContents();
    CArchive loadArchive(&file, CArchive::load | CArchive::bNoFlushOnDelete);
    TRY
    {
        pView->BeginWaitCursor();
        pView->SerializeRaw(loadArchive);  // load me
        loadArchive.Close();
        file.Close();
    }
    CATCH_ALL(e)
    {
        file.Abort();  // will not throw an exception
        pView->EndWaitCursor();
        pView->DeleteContents();  // remove failed contents
        e->Delete();
        GUI_BLOCK_THREADS;
        PyErr_SetString(PyExc_IOError, "File error reading file");
        return NULL;
    }
    END_CATCH_ALL

    pView->EndWaitCursor();
    CDocument *pDocument = pView->GetDocument();
    if (pDocument)
        pDocument->SetModifiedFlag(FALSE);  // start off with unmodified
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCEditView|SaveFile|Saves the view to a file.
static PyObject *ui_edit_window_save_file(PyObject *self, PyObject *args)
{
    USES_CONVERSION;
    TCHAR *fileName;
    PyObject *obfileName;
    // @pyparm string|fileName||The name of the file to be written.
    if (!PyArg_ParseTuple(args, "O:SaveFile", &obfileName))
        return NULL;

    CPythonEditView *pView;
    if (!(pView = GetEditViewPtr(self)))
        return NULL;

    if (!PyWinObject_AsTCHAR(obfileName, &fileName, FALSE))
        return NULL;
    CFile file;
    CFileException fe;
    if (!file.Open(fileName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, &fe)) {
        long errCode = fe.m_lOsError;
        CString csMessage = GetAPIErrorString(errCode);
        if (csMessage.GetLength()) {
            LPTSTR msg = csMessage.GetBuffer(csMessage.GetLength());
            PyErr_SetString(PyExc_IOError, T2A(msg));
            csMessage.ReleaseBuffer();
        }
        else
            PyErr_SetString(PyExc_IOError, "Unknown IO error?");
        return NULL;
    }
    GUI_BGN_SAVE;
    CArchive saveArchive(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    TRY
    {
        pView->BeginWaitCursor();
        pView->SerializeRaw(saveArchive);  // save me
        saveArchive.Close();
        file.Close();
    }
    CATCH_ALL(e)
    {
        file.Abort();  // will not throw an exception
        pView->EndWaitCursor();
        e->Delete();
        GUI_BLOCK_THREADS;
        PyErr_SetString(PyExc_IOError, "File error saving file");
        return NULL;
    }
    END_CATCH_ALL

    pView->EndWaitCursor();
    CDocument *pDocument = pView->GetDocument();
    if (pDocument)
        pDocument->SetModifiedFlag(FALSE);  // start off with unmodified
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(fileName);
    RETURN_NONE;
}
// @pymethod tuple|PyCEditView|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
PyObject *PyCEditView_pre_create_window(PyObject *self, PyObject *args)
{
    CPythonEditView *pView;
    if (!(pView = GetEditViewPtr(self)))
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    BOOL ok;
    GUI_BGN_SAVE;
    ok = pView->PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)                                              // WARNING - If CPythonEditView::PreCreateWindow gets
        RETURN_ERR("CEditView::PreCreateWindow failed");  // fixed, this will break.
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod |PyCEditView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCEditView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonEditView, &PyCEditView::type, self, args);
}

///////////////////////////////////////
//
// Edit View Methods
//
// inherited from view
//
///////////////////////////////////////
// @object PyCEditView|A class which implementes a CView of a text file.  Derived from <o PyCView> and <o PyCEdit>
// objects.
static struct PyMethodDef ui_edit_window_methods[] = {
    {"IsModified", ui_edit_window_is_modified, 1},  // @pymeth IsModified|Indicates if the view's document is modified.
    {"LoadFile", ui_edit_window_load_file, 1},      // @pymeth LoadFile|Loads a named file into the view.
    {"SetModifiedFlag", ui_edit_window_set_modified_flag,
     1},                                        // @pymeth SetModifiedFlag|Sets the view's document modified flag.
    {"GetEditCtrl", ui_edit_get_edit_ctrl, 1},  // @pymeth GetEditCtrl|Returns the underlying <o PyCEdit> object
    {"PreCreateWindow", PyCEditView_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"SaveFile", ui_edit_window_save_file, 1},  // @pymeth SaveFile|Saves the view to a named file.
    {"OnCommand", PyCEditView_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {NULL, NULL}};

// @base PyCEditView|PyCCtrlView
PyCCtrlView_Type PyCEditView::type("PyCEditView", &PyCCtrlView::type, &PyCEdit::type, RUNTIME_CLASS(CEditView),
                                   sizeof(PyCEditView), PYOBJ_OFFSET(PyCEditView), ui_edit_window_methods,
                                   GET_PY_CTOR(PyCEditView));

/////////////////////////////////////////////////////////////////////
//
// List View object
//
//////////////////////////////////////////////////////////////////////
// @pymethod <o PyCListView>|win32ui|CreateListView|Creates a PyCListView object.
PyObject *PyCListView::create(PyObject *self, PyObject *args)
{
    PyObject *doc;
    // @pyparm <o PyCDocument>|doc||The document to use with the view.
    if (!PyArg_ParseTuple(args, "O:CreateListView", &doc))
        return NULL;
    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    GUI_BGN_SAVE;
    CListView *pView = new CPythonListView();
    ((CProtectedView *)pView)->SetDocument(pDoc);
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCListView::type, pView, TRUE);
}

// @pymethod tuple|PyCListView|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
PyObject *PyCListView_pre_create_window(PyObject *self, PyObject *args)
{
    CListView *pView;
    if (!(pView = GetListViewPtr(self)))
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pView->CListView::PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CListView::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod <o PyCListCtrl>|PyCListView|GetListCtrl|Returns the underlying list control object.
static PyObject *PyCListView_get_list_ctrl(PyObject *self, PyObject *args)
{
    CListView *pView;
    if (!(pView = GetListViewPtr(self)))
        return NULL;
    GUI_BGN_SAVE;
    CListCtrl &lc = pView->GetListCtrl();
    GUI_END_SAVE;
    return ui_assoc_object::make(UITypeFromCObject(&lc), &lc)->GetGoodRet();
}

// @pymethod |PyCListView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCListView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonListView, &PyCListView::type, self, args);
}

///////////////////////////////////////
// @object PyCListView|A class which implementes a CListView.  Derived from <o PyCView> and <o PyCListCtrl> objects.
static struct PyMethodDef ui_list_view_methods[] = {
    {"PreCreateWindow", PyCListView_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"GetListCtrl", PyCListView_get_list_ctrl, 1},  // @pymeth GetListCtrl|Returns the underlying list control object.
    {"OnCommand", PyCListView_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {NULL, NULL}};

PyCCtrlView_Type PyCListView::type("PyCListView",
                                   &PyCCtrlView::type,  // @base PyCListView|PyCCtrlView
                                   &PyCListCtrl::type, RUNTIME_CLASS(CListView), sizeof(PyCListView),
                                   PYOBJ_OFFSET(PyCListView), ui_list_view_methods, GET_PY_CTOR(PyCListView));

/////////////////////////////////////////////////////////////////////
//
// Tree View object
//
//////////////////////////////////////////////////////////////////////
// @pymethod <o PyCTreeView>|win32ui|CreateTreeView|Creates a PyCTreeView object.
PyObject *PyCTreeView::create(PyObject *self, PyObject *args)
{
    PyObject *doc;
    // @pyparm <o PyCDocument>|doc||The document to use with the view.
    if (!PyArg_ParseTuple(args, "O:CreateTreeView", &doc))
        return NULL;
    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    GUI_BGN_SAVE;
    CTreeView *pView = new CPythonTreeView();
    ((CProtectedView *)pView)->SetDocument(pDoc);
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCTreeView::type, pView, TRUE);
}
// @pymethod tuple|PyCTreeView|PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
PyObject *PyCTreeView_pre_create_window(PyObject *self, PyObject *args)
{
    CTreeView *pView;
    if (!(pView = GetTreeViewPtr(self)))
        return NULL;
    CREATESTRUCT cs;
    //@pyparm tuple|createStruct||A tuple representing a CREATESTRUCT structure.
    if (!CreateStructFromPyObject(&cs, args, "PreCreateWindow", TRUE))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pView->CTreeView::PreCreateWindow(cs);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CTreeView::PreCreateWindow failed");
    return PyObjectFromCreateStruct(&cs);
}

// @pymethod <o PyCTreeCtrl>|PyCTreeView|GetTreeCtrl|Returns the underlying tree control object.
static PyObject *PyCTreeView_get_tree_ctrl(PyObject *self, PyObject *args)
{
    CTreeView *pView;
    if (!(pView = GetTreeViewPtr(self)))
        return NULL;
    GUI_BGN_SAVE;
    CTreeCtrl &lc = pView->GetTreeCtrl();
    GUI_END_SAVE;
    return ui_assoc_object::make(UITypeFromCObject(&lc), &lc)->GetGoodRet();
}

// @pymethod |PyCTreeView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCTreeView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonTreeView, &PyCTreeView::type, self, args);
}

///////////////////////////////////////
// @object PyCTreeView|A class which implementes a CTreeView.  Derived from <o PyCView> and <o PyCTreeCtrl> objects.
static struct PyMethodDef ui_tree_view_methods[] = {
    {"PreCreateWindow", PyCTreeView_pre_create_window,
     1},  // @pymeth PreCreateWindow|Calls the underlying MFC PreCreateWindow method.
    {"GetTreeCtrl", PyCTreeView_get_tree_ctrl, 1},  // @pymeth GetTreeCtrl|Returns the underlying tree control object.
    {"OnCommand", PyCTreeView_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {NULL, NULL}};

PyCCtrlView_Type PyCTreeView::type("PyCTreeView",
                                   &PyCCtrlView::type,  // @base PyCTreeView|PyCCtrlView
                                   &PyCTreeCtrl::type, RUNTIME_CLASS(CTreeView), sizeof(PyCTreeView),
                                   PYOBJ_OFFSET(PyCTreeView), ui_tree_view_methods, GET_PY_CTOR(PyCTreeView));

///////////////////////////////////////
//
// Form View Methods
//
// inherited from view
//
///////////////////////////////////////

// @pymethod <o PyCFormView>|win32ui|CreateFormView|Creates a form view object.
PyObject *PyCFormView::create(PyObject *self, PyObject *args)
{
    PyObject *doc, *obTemplate;
    TCHAR *szTemplate = NULL;
    // @pyparm <o PyCDocument>|doc||The document to use with the view.
    // @pyparm int/str|Template||Name or ID of the dialog template resource
    if (!PyArg_ParseTuple(args, "OO:CreateFormView", &doc, &obTemplate))
        return NULL;

    if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
        RETURN_TYPE_ERR("Argument must be a PyCDocument");
    CDocument *pDoc = PyCDocument::GetDoc(doc);
    CFormView *pView;
    if (!PyWinObject_AsResourceId(obTemplate, &szTemplate))
        return NULL;

    GUI_BGN_SAVE;
    if (IS_INTRESOURCE(szTemplate))
        pView = new CPythonFormView(MAKEINTRESOURCE(szTemplate));
    else
        pView = new CPythonFormView(szTemplate);
    ((CProtectedView *)pView)->SetDocument(pDoc);
    GUI_END_SAVE;
    PyWinObject_FreeResourceId(szTemplate);
    return ui_assoc_object::make(PyCFormView::type, pView, TRUE);
}

// @pymethod |PyCFormView|OnCommand|Calls the standard Python framework OnCommand handler
PyObject *PyCFormView_on_command(PyObject *self, PyObject *args)
{
    // @xref <vm PyCWnd.OnCommand>
    // @pyparm int|wparam||
    // @pyparm int|lparam||
    return DoBaseOnCommand(CPythonFormView, &PyCFormView::type, self, args);
}

// @object PyCFormView|A class which implementes a CFormView (ie, a view based on a dialog resource.
static struct PyMethodDef PyCFormView_methods[] = {
    {"OnCommand", PyCFormView_on_command,
     1},  // @pymeth OnCommand|Calls the standard Python framework OnCommand handler
    {NULL, NULL}};

ui_type_CObject PyCFormView::type("PyCFormView",
                                  &PyCView::type,  // @base PyCFormView|PyCView
                                  RUNTIME_CLASS(CFormView), sizeof(PyCFormView), PYOBJ_OFFSET(PyCFormView),
                                  PyCFormView_methods, GET_PY_CTOR(PyCFormView));
