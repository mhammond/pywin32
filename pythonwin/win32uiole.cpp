#include "stdafxole.h"

#include "win32dlg.h"
#include "win32oleDlgs.h"
#include "win32uioledoc.h"
#include "win32template.h"
#include "PythonCOM.h"
// not including this here causes compile errors when it *is*
// included by later headers, using MSVC6 standard headers
// (although replacing transact.h with a later Platform SDK
// version does *not* give the error.  Whatever.
#include "transact.h"
#if !defined(_M_X64) && !defined(_M_ARM64)
#include "afxdao.h"
#endif

#include "afxocc.h"
// Sorry about this - OLE support needs MFC private header.
// You MUST install MFC with source-code to build this extension.
// (and this source must be in "../src" relative to the MFC
// includes, which it is by default)

extern PyObject *PyCOleClientItem_Create(PyObject *self, PyObject *args);

// @doc

class WndHack : public CWnd {
   public:
    COleControlSite *GetCtrlSite() { return m_pCtrlSite; }
};

// @pymethod <o PyIDispatch>|win32uiole|GetIDispatchForWindow|Gets an OCX IDispatch pointer, if the window has one!
static PyObject *win32uiole_GetIDispatchForWindow(PyObject *self, PyObject *args)
{
    PyObject *obWnd;
    if (!PyArg_ParseTuple(args, "O:GetIDispatchForWindow", &obWnd))
        return NULL;
    WndHack *pWnd = (WndHack *)GetWndPtr(obWnd);
    if (!pWnd)
        return NULL;
    COleControlSite *pSite = pWnd->GetCtrlSite();
    if (pSite == NULL || pSite->m_pObject == NULL) {
        RETURN_ERR("There is no OLE object available");
    }
    IDispatch *disp = NULL;
    GUI_BGN_SAVE;
    SCODE sc = pSite->m_pObject->QueryInterface(IID_IDispatch, (void **)&disp);
    GUI_END_SAVE;
    if (FAILED(sc) || disp == NULL)
        RETURN_ERR("The OLE object does not support IDispatch");
    return PyCom_PyObjectFromIUnknown(disp, IID_IDispatch, FALSE);
}

// @pymethod int|win32uiole|OleGetUserCtrl|Returns the application name.
static PyObject *win32uiole_get_user_ctrl(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":OleGetUserCtrl"))
        return NULL;
    GUI_BGN_SAVE;
    int rc = AfxOleGetUserCtrl();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod int|win32uiole|OleSetUserCtrl|Sets or clears the user control flag.
static PyObject *win32uiole_set_user_ctrl(PyObject *self, PyObject *args)
{
    BOOL flag;
    if (!PyArg_ParseTuple(args, "i:OleSetUserCtrl", &flag))
        // @pyparm int|bUserCtrl||Specifies whether the user-control flag is to be set or cleared.
        return NULL;
    GUI_BGN_SAVE;
    AfxOleSetUserCtrl(flag);
    GUI_END_SAVE;
    RETURN_NONE;
}

// A DAO hack!
// @pymethod <o PyIDispatch>|win32uiole|DaoGetEngine|
static PyObject *DaoGetEngine(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, "DaoGetEngine");

#if defined(_M_X64) || defined(_M_ARM64)
    return NULL;
#else
    AfxDaoInit();
    DAODBEngine *pEngine = AfxDaoGetEngine();
    IDispatch *pDisp;
    HRESULT hr = pEngine->QueryInterface(IID_IDispatch, (void **)&pDisp);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    pEngine->Release();
    return PyCom_PyObjectFromIUnknown(pDisp, IID_IDispatch, FALSE);
#endif
}

// @pymethod |win32uiole|SetMessagePendingDelay|
static PyObject *win32uiole_SetMessagePendingDelay(PyObject *self, PyObject *args)
{
    // @pyparm int|delay||
    int delay;
    if (!PyArg_ParseTuple(args, "i", &delay))
        return NULL;
    AfxOleGetMessageFilter()->SetMessagePendingDelay(delay);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32uiole|EnableNotRespondingDialog|
static PyObject *win32uiole_EnableNotRespondingDialog(PyObject *self, PyObject *args)
{
    // @pyparm bool|enabled||
    int enabled;
    if (!PyArg_ParseTuple(args, "i", &enabled))
        return NULL;
    AfxOleGetMessageFilter()->EnableNotRespondingDialog(enabled);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32uiole|EnableBusyDialog|
static PyObject *win32uiole_EnableBusyDialog(PyObject *self, PyObject *args)
{
    // @pyparm bool|enabled||
    int enabled;
    if (!PyArg_ParseTuple(args, "i", &enabled))
        return NULL;
    AfxOleGetMessageFilter()->EnableBusyDialog(enabled);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32uiole|AfxOleInit|
static PyObject *win32uiole_AfxOleInit(PyObject *self, PyObject *args)
{
    // @pyparm bool|enabled||
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    BOOL rc = AfxOleInit();
    return PyBool_FromLong(rc);
}

// @module win32uiole|A module, encapsulating the Microsoft Foundation Classes OLE functionality.
static struct PyMethodDef win32uiole_functions[] = {
    {"AfxOleInit", win32uiole_AfxOleInit, 1},               // @pymeth AfxOleInit|
    {"CreateInsertDialog", PyCOleInsertDialog::create, 1},  // @pymeth CreateInsertDialog|Creates a InsertObject dialog.
    {"CreateOleClientItem", PyCOleClientItem_Create,
     1},  // @pymeth CreateOleClientItem|Creates a <o PyCOleClientItem> object.
    {"CreateOleDocument", PyCOleDocument::Create, 1},  // @pymeth CreateOleDocument|Creates a <o PyCOleDocument> object.
    {"DaoGetEngine", DaoGetEngine, 1},                 // @pymeth DaoGetEngine|
    {"GetIDispatchForWindow", win32uiole_GetIDispatchForWindow,
     1},  // @pymeth GetIDispatchForWindow|Gets an OCX IDispatch pointer, if the window has one!
    {"OleGetUserCtrl", win32uiole_get_user_ctrl, 1},  // @pymeth OleGetUserCtrl|Retrieves the current user-control flag.
    {"OleSetUserCtrl", win32uiole_set_user_ctrl, 1},  // @pymeth OleSetUserCtrl|Sets the current user-control flag.
    {"SetMessagePendingDelay", win32uiole_SetMessagePendingDelay, 1},        // @pymeth SetMessagePendingDelay|
    {"EnableNotRespondingDialog", win32uiole_EnableNotRespondingDialog, 1},  // @pymeth EnableNotRespondingDialog|
    {"EnableBusyDialog", win32uiole_EnableBusyDialog, 1},                    // @pymeth EnableNotRespondingDialog|
    {NULL, NULL}};

#define ADD_CONSTANT(tok)                                 \
    if (PyModule_AddIntConstant(module, #tok, tok) == -1) \
    return -1
#define ADD_ENUM(parta, partb)                                                  \
    if (PyModule_AddIntConstant(module, #parta "_" #partb, parta::partb) == -1) \
    return -1

int AddConstants(PyObject *module)
{
    ADD_ENUM(COleClientItem, emptyState);     // @const win32uiole|COleClientItem_emptyState|
    ADD_ENUM(COleClientItem, loadedState);    // @const win32uiole|COleClientItem_loadedState|
    ADD_ENUM(COleClientItem, openState);      // @const win32uiole|COleClientItem_openState|
    ADD_ENUM(COleClientItem, activeState);    // @const win32uiole|COleClientItem_activeState|
    ADD_ENUM(COleClientItem, activeUIState);  // @const win32uiole|COleClientItem_activeUIState|
    ADD_CONSTANT(OLE_CHANGED);  // @const win32uiole|OLE_CHANGED|representation of a draw aspect has changed
    ADD_CONSTANT(OLE_SAVED);    // @const win32uiole|OLE_SAVED|the item has committed its storage
    ADD_CONSTANT(OLE_CLOSED);   // @const win32uiole|OLE_CLOSED|the item has closed
    ADD_CONSTANT(OLE_RENAMED);  // @const win32uiole|OLE_RENAMED|the item has changed its moniker
    ADD_CONSTANT(
        OLE_CHANGED_STATE);  // @const win32uiole|OLE_CHANGED_STATE|the item state (open, active, etc.) has changed
    ADD_CONSTANT(OLE_CHANGED_ASPECT);  // @const win32uiole|OLE_CHANGED_ASPECT|the item draw aspect has changed
    return 0;
}

PYWIN_MODULE_INIT_FUNC(win32uiole)
{
    PYWIN_MODULE_INIT_PREPARE(win32uiole, win32uiole_functions,
                              "A module, encapsulating the Microsoft Foundation Classes OLE functionality.");

    if (AddConstants(module))
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PyCCommonDialog::type) == -1 || PyType_Ready(&PyCOleDialog::type) == -1 ||
        PyType_Ready(&PyCOleInsertDialog::type) == -1 || PyType_Ready(&PyCOleDocument::type) == -1 ||
        PyType_Ready(&PyCOleClientItem::type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
