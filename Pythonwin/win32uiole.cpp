#include "stdafxole.h"

#include "win32dlg.h"
#include "win32oleDlgs.h"
#include "win32uioledoc.h"
#include "win32template.h"
#include "PythonCOM.h"
#include "afxdao.h"

// Sorry about this - OLE support needs MFC private header.
// Adding MFC source path to include path causes grief!
//#include "c:\program files\DevStudio\VC\mfc\src\occimpl.h"
#include "..\src\occimpl.h"

extern PyObject *PyCOleClientItem_Create(PyObject *self, PyObject *args);

// @doc

class WndHack : public CWnd {
public:
	COleControlSite *GetCtrlSite() {return m_pCtrlSite;}
};

// @pymethod <o PyIDispatch>|win32uiole|GetIDispatchForWindow|Gets an OCX IDispatch pointer, if the window has one!
static PyObject *
win32uiole_GetIDispatchForWindow(PyObject *self, PyObject *args)
{
	PyObject *obWnd;
	if (!PyArg_ParseTuple(args, "O:GetIDispatchForWindow", &obWnd))
		return NULL;
	WndHack *pWnd = (WndHack *)GetWndPtr(obWnd);
	if (!pWnd)
		return NULL;
	COleControlSite *pSite = pWnd->GetCtrlSite();
	if (pSite==NULL || pSite->m_pObject==NULL) {
		RETURN_ERR("There is no OLE object available");

	}
	IDispatch *disp = NULL;
	GUI_BGN_SAVE;
	SCODE sc = pSite->m_pObject->QueryInterface(IID_IDispatch, (void**)&disp);
	GUI_END_SAVE;
	if (FAILED(sc) || disp == NULL)
		RETURN_ERR("The OLE object does not support IDispatch");
	return PyCom_PyObjectFromIUnknown(disp, IID_IDispatch, FALSE);
}

// @pymethod int|win32uiole|OleGetUserCtrl|Returns the application name.
static PyObject *
win32uiole_get_user_ctrl(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":OleGetUserCtrl"))
		return NULL;
	GUI_BGN_SAVE;
	int rc = AfxOleGetUserCtrl();
	GUI_END_SAVE;
	return Py_BuildValue("i", rc);
}

// @pymethod int|win32uiole|OleSetUserCtrl|Sets or clears the user control flag.
static PyObject *
win32uiole_set_user_ctrl(PyObject *self, PyObject *args)
{
	BOOL flag;
	if (!PyArg_ParseTuple(args,"i:OleSetUserCtrl", &flag))
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

	AfxDaoInit();
	DAODBEngine* pEngine = AfxDaoGetEngine();
	IDispatch *pDisp;
	HRESULT hr = pEngine->QueryInterface(IID_IDispatch, (void **)&pDisp);
	if (FAILED(hr))
		return OleSetOleError(hr);
	pEngine->Release();
	return PyCom_PyObjectFromIUnknown(pDisp, IID_IDispatch, FALSE);
}

// @module win32uiole|A module, encapsulating the Microsoft Foundation Classes OLE functionality.
static struct PyMethodDef uiole_functions[] = {
	{"CreateInsertDialog",   PyCOleInsertDialog::create, 1}, // @pymeth CreateInsertDialog|Creates a InsertObject dialog.
	{"CreateOleClientItem",  PyCOleClientItem_Create, 1}, // @pymeth CreateOleClientItem|Creates a <o PyCOleClientItem> object.
	{"CreateOleDocument",    PyCOleDocument::Create, 1}, // @pymeth CreateOleDocument|Creates a <o PyCOleDocument> object.
	{"DaoGetEngine",         DaoGetEngine, 1}, // @pymeth DaoGetEngine|
	{"GetIDispatchForWindow",win32uiole_GetIDispatchForWindow,1}, // @pymeth GetIDispatchForWindow|Gets an OCX IDispatch pointer, if the window has one!
	{"OleGetUserCtrl",       win32uiole_get_user_ctrl, 1}, // @pymeth OleGetUserCtrl|Retrieves the current user-control flag.
	{"OleSetUserCtrl",       win32uiole_set_user_ctrl, 1}, // @pymeth OleSetUserCtrl|Sets the current user-control flag.
	{NULL,			NULL}
};

#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, tok)) return rc
#define ADD_ENUM(parta, partb) if (rc=AddConstant(dict,#parta "_" #partb, parta::partb)) return rc

static int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyInt_FromLong(value);
	if (!okey || !oval) {
		XDODECREF(okey);
		XDODECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	DODECREF(okey);
	DODECREF(oval);
	return rc;
}

int AddConstants(PyObject *dict)
{
  int rc;
  ADD_ENUM(COleClientItem, emptyState);// @const win32uiole|COleClientItem_emptyState|
  ADD_ENUM(COleClientItem, loadedState);// @const win32uiole|COleClientItem_loadedState|
  ADD_ENUM(COleClientItem, openState);// @const win32uiole|COleClientItem_openState|
  ADD_ENUM(COleClientItem, activeState);// @const win32uiole|COleClientItem_activeState|
  ADD_ENUM(COleClientItem, activeUIState);// @const win32uiole|COleClientItem_activeUIState|
  ADD_CONSTANT(OLE_CHANGED);        // @const win32uiole|OLE_CHANGED|representation of a draw aspect has changed
  ADD_CONSTANT(OLE_SAVED);          // @const win32uiole|OLE_SAVED|the item has committed its storage
  ADD_CONSTANT(OLE_CLOSED);         // @const win32uiole|OLE_CLOSED|the item has closed
  ADD_CONSTANT(OLE_RENAMED);        // @const win32uiole|OLE_RENAMED|the item has changed its moniker
  ADD_CONSTANT(OLE_CHANGED_STATE);  // @const win32uiole|OLE_CHANGED_STATE|the item state (open, active, etc.) has changed
  ADD_CONSTANT(OLE_CHANGED_ASPECT); // @const win32uiole|OLE_CHANGED_ASPECT|the item draw aspect has changed
  return 0;
}

extern "C" __declspec(dllexport) void
initwin32uiole(void)
{
  PyObject *module = Py_InitModule("win32uiole", uiole_functions);
  PyObject *dict = PyModule_GetDict(module);
  AddConstants(dict);
 }
