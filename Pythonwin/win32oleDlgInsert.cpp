#include "stdafxole.h"

#include "win32dlg.h"
#include "win32oleDlgs.h"
#include "pythoncom.h"
#include "pywintypes.h"
// @doc

class OLEUIINSERTOBJECTHelper
{
public:
	OLEUIINSERTOBJECTHelper( OLEUIINSERTOBJECT *pCon );
	~OLEUIINSERTOBJECTHelper();
	BOOL ParseDict(PyObject *dict);
	BOOL BuildDict(PyObject *dict);
private:
	char fileNameBuf[MAX_PATH];
	OLEUIINSERTOBJECT *pConv;
};

OLEUIINSERTOBJECTHelper::OLEUIINSERTOBJECTHelper( OLEUIINSERTOBJECT *pCon )
{
	ASSERT(pCon);
	memset(pCon, 0, sizeof( OLEUIINSERTOBJECT ) );
	pCon->cbStruct = sizeof( OLEUIINSERTOBJECT );
	pCon->lpszFile = fileNameBuf;
	pCon->cchFile = sizeof(fileNameBuf);
	pConv = pCon;
}
OLEUIINSERTOBJECTHelper::~OLEUIINSERTOBJECTHelper()
{
}

BOOL OLEUIINSERTOBJECTHelper::ParseDict( PyObject *obDict )
{
	PyObject *ob;
	ob = PyObject_GetAttrString(obDict, "Flags");
	if (ob) pConv->dwFlags = PyInt_AsLong(ob);
	ob = PyObject_GetAttrString(obDict, "WndOwner");
	if (ob) {
		if (PyInt_Check(ob))
			pConv->hWndOwner = (HWND)PyInt_AsLong(ob);
		else {
			CWnd *pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(ob);
			if (pParent==NULL) {
				PyErr_SetString(PyExc_TypeError, "The WndOwner element must be a integer HWND or a window object");
				return FALSE;
			}
			pConv->hWndOwner = pParent->GetSafeHwnd();
		}
	}

	ob = PyObject_GetAttrString(obDict, "Caption");
	if (ob) pConv->lpszCaption = PyString_AsString(PyObject_Str(ob));
	// Hook not implemented
	// CustData not implemented
	ob = PyObject_GetAttrString(obDict, "Instance");
	if (ob) pConv->hInstance = (HINSTANCE)PyInt_AsLong(ob);
	ob = PyObject_GetAttrString(obDict, "Template");
	if (ob) pConv->lpszTemplate = PyString_AsString(PyObject_Str(ob));
	ob = PyObject_GetAttrString(obDict, "Resource");
	if (ob) pConv->hResource = (HRSRC)PyInt_AsLong(ob);

	// OLEUIINSERTOBJECT specifics
	// CLSID is out.
	ob = PyObject_GetAttrString(obDict, "File");
	if (ob) {
		char *szTemp = PyString_AsString(PyObject_Str(ob));
		if (szTemp==NULL) return FALSE;
		strncpy(fileNameBuf, szTemp, sizeof(fileNameBuf));
		fileNameBuf[sizeof(fileNameBuf)-1]='\0';
	}
	// CLSIDExcludeList not yet supported.
	ob = PyObject_GetAttrString(obDict, "iid");
	if (ob) {
		if (!PyWinObject_AsIID(ob, &pConv->iid))
			return FALSE;
	}
	ob = PyObject_GetAttrString(obDict, "oleRender");
	if (ob) pConv->oleRender = PyInt_AsLong(ob);
	// lpFormatEtc not supported.
	// lpIOleCloientSite not supported.
	// lpIStorage not supported
	// hMetaPict not supported.
	return TRUE;
}

BOOL OLEUIINSERTOBJECTHelper::BuildDict( PyObject *obDict )
{
	if (PyObject_SetAttrString(obDict, "Flags", PyInt_FromLong(pConv->dwFlags)))
		return FALSE;
	return TRUE;
}

// @pymethod <o PyCOleInsertDialog>|win32uiole|CreateInsertDialog|Creates a InsertObject dialog.
PyObject *PyCOleInsertDialog::create( PyObject * /*self*/, PyObject *args )
{
	CHECK_NO_ARGS2(args, CreateInsertDialog);
	COleInsertDialog *pDlg = new COleInsertDialog();
	if (!pDlg)
		RETURN_ERR("Creating COleInsertDialog failed"); // pyseemfc COleInsertDialog|COleInsertDialog
	PyCOleInsertDialog *newObj = 
		(PyCOleInsertDialog *)ui_assoc_object::make( PyCOleInsertDialog::type, pDlg);
	return newObj;
}

COleInsertDialog *GetOleInsertDialog(PyObject *self) 
{
	return (COleInsertDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCOleInsertDialog::type);
}

// @pymethod CLSID|PyCOleInsertDialog|GetClassID|Returns the CLSID associated with the selected item
PyObject *PyCOleInsertDialog_GetClassID( PyObject * self, PyObject *args )
{
	CHECK_NO_ARGS2(args, GetClassID);

	COleInsertDialog *pDlg = GetOleInsertDialog(self);
	if (!pDlg) return NULL;
	GUI_BGN_SAVE;
	IID iid = pDlg->GetClassID();
	GUI_END_SAVE;
	return PyWinObject_FromIID(iid);
}
// @pymethod CLSID|PyCOleInsertDialog|GetSelectionType|Returns the type of selection made
PyObject *PyCOleInsertDialog_GetSelectionType( PyObject * self, PyObject *args )
{
	CHECK_NO_ARGS2(args, GetSelectionType);

	COleInsertDialog *pDlg = GetOleInsertDialog(self);
	if (!pDlg) return NULL;
	GUI_BGN_SAVE;
	long rc = pDlg->GetSelectionType();
	GUI_END_SAVE;
	return PyInt_FromLong(rc);
}

// @pymethod CLSID|PyCOleInsertDialog|GetPathName|Returns the full path to the file selected in the dialog box
PyObject *PyCOleInsertDialog_GetPathName( PyObject * self, PyObject *args )
{
	CHECK_NO_ARGS2(args, GetPathName);

	COleInsertDialog *pDlg = GetOleInsertDialog(self);
	if (!pDlg) return NULL;
	GUI_BGN_SAVE;
	CString ret = pDlg->GetPathName();
	GUI_END_SAVE;
	return PyString_FromString((char *)(const char *)ret);
	// @comm Do not call this if the selection type is createNewItem,
}


// @object PyCOleInsertDialog|An OLE 'Insert Object' dialog.  Encapsulates an MFC <c COleInsertDialog> class
static struct PyMethodDef PyCOleInsertDialog_methods[] = {
	{ "GetClassID",         PyCOleInsertDialog_GetClassID, 1}, // @pymeth GetClassID|Returns the CLSID associated with the selected item
	{ "GetSelectionType",   PyCOleInsertDialog_GetSelectionType, 1}, // @pymeth GetSelectionType|Returns the type of selection made
	{ "GetPathName",        PyCOleInsertDialog_GetPathName, 1}, // @pymeth GetPathName|Returns the full path to the file selected in the dialog box
	{ NULL, NULL }
};

ui_type_CObject PyCOleInsertDialog::type("PyCOleInsertDialog", 
							 &PyCOleDialog::type, 
							 RUNTIME_CLASS(COleInsertDialog), 
							 sizeof(PyCOleInsertDialog), 
							 PyCOleInsertDialog_methods, 
							 GET_PY_CTOR(PyCOleInsertDialog) );

