#include "stdafxole.h"
#include "win32dc.h"
#include "win32uioledoc.h"
#include "PythonCOM.h"
// @doc

class PythonOleClientItem : public COleClientItem {
   public:
    PythonOleClientItem(COleDocument *pContainerDoc = NULL) : COleClientItem(pContainerDoc) { ; }
    virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam)
    {
        // @pyvirtual |PyCOleClientItem|OnChange|
        CVirtualHelper helper("OnChange", this);
        if (helper.HaveHandler())
            // @pyparm int|wNotification||
            // @pyparm int|dwParam||
            helper.call(wNotification, dwParam);
        else
            COleClientItem::OnChange(wNotification, dwParam);
    }
    virtual void OnActivate()
    {
        // @pyvirtual |PyCOleClientItem|OnActivate|
        CVirtualHelper helper("OnActivate", this);
        if (helper.HaveHandler())
            helper.call();
        else
            COleClientItem::OnActivate();
    }
    virtual void OnGetItemPosition(CRect &rPosition)
    {
        // @pyvirtual (int, int, int, int)|PyCOleClientItem|OnGetItemPosition|
        CVirtualHelper helper("OnGetItemPosition", this);
        if (helper.call()) {
            PyObject *ret;
            helper.retval(ret);
            CEnterLeavePython _celp;
            PyArg_ParseTuple(ret, "iiii", &rPosition.left, &rPosition.top, &rPosition.right, &rPosition.bottom);
        }
    }
    virtual void OnDeactivateUI(BOOL bUndoable)
    {
        // @pyvirtual |PyCOleClientItem|OnDeactivateUI|
        // @pyparm int|bUndoable||
        CVirtualHelper helper("OnDeactivateUI", this);
        if (helper.HaveHandler())
            helper.call(bUndoable);
        else
            COleClientItem::OnDeactivateUI(bUndoable);
    }
    virtual BOOL OnChangeItemPosition(const CRect &rectPos)
    {
        // @pyvirtual int|PyCOleClientItem|OnChangeItemPosition|
        // @pyparm (int, int, int, int)|(left, top, right, bottom)||The new position
        CVirtualHelper helper("OnChangeItemPosition", this);
        BOOL bRet;
        PyObject *args = Py_BuildValue("(iiii)", rectPos.left, rectPos.top, rectPos.right, rectPos.bottom);
        if (helper.HaveHandler() && helper.call_args(args)) {
            // Note = args decref'd by caller
            helper.retval(bRet);
        }
        else
            bRet = COleClientItem::OnChangeItemPosition(rectPos);
        return bRet;
    }
    BOOL BaseOnChangeItemPosition(const CRect &rectPos) { return COleClientItem::OnChangeItemPosition(rectPos); }
};

// @pymethod <o PyCOleClientItem>|win32uiole|CreateOleClientItem|Creates a <o PyCOleClientItem> object.
PyObject *PyCOleClientItem_Create(PyObject *self, PyObject *args)
{
    PyObject *obDoc;
    if (!PyArg_ParseTuple(args, "O:CreateOleClientItem", &obDoc))
        return NULL;
    if (!PyCOleClientItem::is_uiobject(obDoc, &PyCOleDocument::type))
        RETURN_TYPE_ERR("the first argument must be a document object");
    COleDocument *pDoc = PyCOleDocument::GetDoc(obDoc);
    if (pDoc == NULL)
        return NULL;
    COleClientItem *pNew = new PythonOleClientItem(pDoc);
    return ui_assoc_object::make(PyCOleClientItem::type, pNew);
}

/*static*/ COleClientItem *PyCOleClientItem::GetOleClientItem(PyObject *self)
{
    return (COleClientItem *)GetGoodCppObject(self, &type);
}

// @pymethod |PyCOleClientItem|CreateNewItem|Creates an embedded item.
static PyObject *PyCOleClientItem_CreateNewItem(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    PyObject *obclsid;
    CLSID clsid;
    if (!PyArg_ParseTuple(args, "O:CreateNewItem", &obclsid))
        return NULL;
    if (!PyWinObject_AsIID(obclsid, &clsid))
        return NULL;
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pCI->CreateNewItem(clsid);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CreateNewItem failed");
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|Close|Closes the item
static PyObject *PyCOleClientItem_Close(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "Close");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    pCI->Close();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|DoVerb|Executes the specified verb.
static PyObject *PyCOleClientItem_DoVerb(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    long iVerb;
    PyObject *obView;
    if (!PyArg_ParseTuple(args, "lO", &iVerb, &obView))
        return NULL;

    if (!PyCOleClientItem::is_uiobject(obView, &PyCView::type))
        RETURN_TYPE_ERR("the first argument must be a view object");
    CView *pView = PyCView::GetViewPtr(obView);

    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL bOK = pCI->DoVerb(iVerb, pView);
    GUI_END_SAVE;
    if (!bOK)
        RETURN_ERR("DoVerb failed");
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|Draw|Draws the OLE item into the specified bounding rectangle using the specified device
// context.
static PyObject *PyCOleClientItem_Draw(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    PyObject *obDC;
    RECT rect;
    int aspect = -1;
    if (!PyArg_ParseTuple(args, "O(iiii)|i", &obDC, &rect.left, &rect.top, &rect.right, &rect.bottom, &aspect))
        return NULL;

    if (!PyCOleClientItem::is_uiobject(obDC, &ui_dc_object::type))
        RETURN_TYPE_ERR("the first argument must be a view object");
    CDC *pDC = ui_dc_object::GetDC(obDC);

    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL bOK = pCI->Draw(pDC, &rect, (DVASPECT)aspect);
    GUI_END_SAVE;
    if (!bOK)
        RETURN_ERR("Draw failed");
    RETURN_NONE;
}

// @pymethod <o PyCView>|PyCOleClientItem|GetActiveView|Obtains the active view for the item
static PyObject *PyCOleClientItem_GetActiveView(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetActiveView");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    CView *pWnd = pCI->GetActiveView();
    GUI_END_SAVE;

    return ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}

// @pymethod <o PyCDocument>|PyCOleClientItem|GetDocument|Obtains the current document for the item
static PyObject *PyCOleClientItem_GetDocument(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetDocument");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    CDocument *pDoc = pCI->GetDocument();
    GUI_END_SAVE;

    return ui_assoc_object::make(UITypeFromCObject(pDoc), pDoc)->GetGoodRet();
}

// @pymethod <o PyCWnd>|PyCOleClientItem|GetInPlaceWindow|Obtains the window in which the item has been opened for
// in-place editing.
static PyObject *PyCOleClientItem_GetInPlaceWindow(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetInPlaceWindow");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    CWnd *pWnd = pCI->GetInPlaceWindow();
    GUI_END_SAVE;

    return ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}

// @pymethod |PyCOleClientItem|GetItemState|Obtains the OLE item's current state
static PyObject *PyCOleClientItem_GetItemState(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetItemState");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pCI->GetItemState();
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod <o PyIUnknown>|PyCOleClientItem|GetObject|Returns the COM object to the item.  This is the m_lpObject
// variable in MFC.
static PyObject *PyCOleClientItem_GetObject(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetObject");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    if (pCI->m_lpObject == NULL)
        RETURN_NONE;
    return PyCom_PyObjectFromIUnknown(pCI->m_lpObject, IID_IUnknown, TRUE);
}

// @pymethod |PyCOleClientItem|GetStorage|Returns the COM object used for storage
static PyObject *PyCOleClientItem_GetStorage(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "GetObject");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    if (pCI->m_lpStorage == NULL)
        RETURN_NONE;
    return PyCom_PyObjectFromIUnknown(pCI->m_lpStorage, IID_IStorage, TRUE);
}

// @pymethod |PyCOleClientItem|OnActivate|Calls the underlying MFC method.
static PyObject *PyCOleClientItem_OnActivate(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "OnActivate");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    pCI->COleClientItem::OnActivate();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|OnChange|Calls the underlying MFC method.
static PyObject *PyCOleClientItem_OnChange(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    long l1, l2;
    if (!PyArg_ParseTuple(args, "ll", &l1, &l2))
        return NULL;
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    pCI->COleClientItem::OnChange((OLE_NOTIFICATION)l1, l2);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCOleClientItem|OnChangeItemPosition|Calls the underlying MFC method.
static PyObject *PyCOleClientItem_OnChangeItemPosition(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    RECT rect;
    if (!PyArg_ParseTuple(args, "iiii", &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL bRet = ((PythonOleClientItem *)pCI)->BaseOnChangeItemPosition(rect);
    GUI_END_SAVE;
    // @rdesc The result is a BOOL indicating if the function succeeded.  No exception is thrown.
    return PyInt_FromLong(bRet);
}

// @pymethod int|PyCOleClientItem|OnDeactivateUI|Calls the underlying MFC method.
static PyObject *PyCOleClientItem_OnDeactivateUI(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    int bUndoable;
    if (!PyArg_ParseTuple(args, "i", &bUndoable))
        return NULL;
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    pCI->COleClientItem::OnDeactivateUI(bUndoable);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|Run|Runs the application associated with this item.
static PyObject *PyCOleClientItem_Run(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    CHECK_NO_ARGS2(args, "Run");
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    pCI->Run();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCOleClientItem|SetItemRects|Sets the bounding rectangle or the visible rectangle of the OLE item.
static PyObject *PyCOleClientItem_SetItemRects(PyObject *self, PyObject *args)
{
    COleClientItem *pCI;
    RECT rectPos = {-1, -1, -1, -1};
    RECT rectClip = {-1, -1, -1, -1};
    if (!PyArg_ParseTuple(args, "|(iiii)(iiii)", &rectPos.left, &rectPos.top, &rectPos.right, &rectPos.bottom,
                          &rectClip.left, &rectClip.top, &rectClip.right, &rectClip.bottom))
        return NULL;
    RECT *pRectPos = rectPos.left == -1 && rectPos.right == -1 ? NULL : &rectPos;
    RECT *pRectClip = rectClip.left == -1 && rectClip.right == -1 ? NULL : &rectClip;
    if (!(pCI = PyCOleClientItem::GetOleClientItem(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pCI->SetItemRects(pRectPos, pRectClip);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetItemRects failed");
    RETURN_NONE;
}

// @object PyCOleClientItem|An OLE client item class.  Encapsulates an MFC <c COleClientItem> class
static struct PyMethodDef PyCOleClientItem_methods[] = {
    {"CreateNewItem", PyCOleClientItem_CreateNewItem, 1},  // @pymeth CreateNewItem|Creates an embedded item.
    {"Close", PyCOleClientItem_Close, 1},                  // @pymeth Close|Closes the item.
    {"DoVerb", PyCOleClientItem_DoVerb, 1},                // @pymeth DoVerb|Executes the specified verb.
    {"Draw", PyCOleClientItem_Draw,
     1},  // @pymeth Draw|Draws the OLE item into the specified bounding rectangle using the specified device context.
    {"GetActiveView", PyCOleClientItem_GetActiveView, 1},  // @pymeth GetActiveView|Obtains the active view for the item
    {"GetDocument", PyCOleClientItem_GetDocument, 1},  // @pymeth GetDocument|Obtains the current document for the item
    {"GetInPlaceWindow", PyCOleClientItem_GetInPlaceWindow,
     1},  // @pymeth GetInPlaceWindow|Obtains the window in which the item has been opened for in-place editing.
    {"GetItemState", PyCOleClientItem_GetItemState, 1},  // @pymeth GetItemState|Obtains the OLE item's current state
    {"GetObject", PyCOleClientItem_GetObject,
     1},  // @pymeth GetObject|Returns the COM object to the item.  This is the m_lpObject variable in MFC.
    {"GetStorage", PyCOleClientItem_GetStorage, 1},  // @pymeth GetStorage|Returns the COM object used for storage
    {"OnActivate", PyCOleClientItem_OnActivate, 1},  // @pymeth OnActivate|Calls the underlying MFC handler.
    {"OnChange", PyCOleClientItem_OnChange, 1},      // @pymeth OnChange|Calls the underlying MFC handler.
    {"OnChangeItemPosition", PyCOleClientItem_OnChangeItemPosition,
     1},  // @pymeth OnChangeItemPosition|Calls the underlying MFC method.
    {"OnDeactivateUI", PyCOleClientItem_OnDeactivateUI, 1},  // @pymeth OnDeactivateUI|Calls the underlying MFC method.
    {"Run", PyCOleClientItem_Run, 1},  // @pymeth Run|Runs the application associated with this item.
    {"SetItemRects", PyCOleClientItem_SetItemRects,
     1},  // @pymeth SetItemRects|Sets the bounding rectangle or the visible rectangle of the OLE item.
    {NULL},
};

ui_type_CObject PyCOleClientItem::type("PyCOleClientItem",
                                       &PyCCmdTarget::type,  // should be CDocItem when we support it.
                                       RUNTIME_CLASS(COleClientItem), sizeof(PyCOleClientItem),
                                       PYOBJ_OFFSET(PyCOleClientItem), PyCOleClientItem_methods,
                                       GET_PY_CTOR(PyCOleClientItem));
