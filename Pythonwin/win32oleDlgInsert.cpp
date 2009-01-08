#include "stdafxole.h"

#include "win32dlg.h"
#include "win32oleDlgs.h"
#include "pythoncom.h"
#include "pywintypes.h"
// @doc

// XXX - is this actually used?????????
class OLEUIINSERTOBJECTHelper
{
public:
	OLEUIINSERTOBJECTHelper( OLEUIINSERTOBJECT *pCon );
	~OLEUIINSERTOBJECTHelper();
	BOOL ParseDict(PyObject *dict);
	BOOL BuildDict(PyObject *dict);
private:
	TCHAR fileNameBuf[MAX_PATH];
	OLEUIINSERTOBJECT *pConv;
};

OLEUIINSERTOBJECTHelper::OLEUIINSERTOBJECTHelper( OLEUIINSERTOBJECT *pCon )
{
	ASSERT(pCon);
	memset(pCon, 0, sizeof( OLEUIINSERTOBJECT ) );
	pCon->cbStruct = sizeof( OLEUIINSERTOBJECT );
	pCon->lpszFile = fileNameBuf;
	pCon->cchFile = sizeof(fileNameBuf)/sizeof(fileNameBuf[0]);
	pConv = pCon;
}
OLEUIINSERTOBJECTHelper::~OLEUIINSERTOBJECTHelper()
{
	if (pConv->lpszCaption)
		PyWinObject_FreeTCHAR((TCHAR *)pConv->lpszCaption);
	if (pConv->lpszTemplate)
		PyWinObject_FreeTCHAR((TCHAR *)pConv->lpszTemplate);
}

BOOL OLEUIINSERTOBJECTHelper::ParseDict( PyObject *obDict )
{
	// ??? This code used to leave a lot of exceptions hanging, py3k is much more sensitive to this
	//	than earlier version.  Changes to build for py3k are untested. ???
	PyObject *ob;
	ob = PyObject_GetAttrString(obDict, "Flags");
	if (ob){
		pConv->dwFlags = PyInt_AsLong(ob);
		if (pConv->dwFlags == (DWORD) -1 && PyErr_Occurred())
			return FALSE;
		}
	else
		PyErr_Clear();

	ob = PyObject_GetAttrString(obDict, "WndOwner");
	if (ob) {
		if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&pConv->hWndOwner)){
			CWnd *pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(ob);
			if (pParent==NULL) {
				PyErr_Clear();		// py3k doesn't like it when you overwrite an existing exception
				PyErr_SetString(PyExc_TypeError, "The WndOwner element must be a HWND (PyHANDLE or int) or a PyCWnd object");
				return FALSE;
			}
			pConv->hWndOwner = pParent->GetSafeHwnd();
		}
	}
	else
		PyErr_Clear();

	ob = PyObject_GetAttrString(obDict, "Caption");
	if (ob){
		// Need cast since lpszCaption is const.  Free'd in object dtor
		if (!PyWinObject_AsTCHAR(ob, (TCHAR **)&pConv->lpszCaption, FALSE))
			return FALSE;
		}
	else
		PyErr_Clear();

	// Hook not implemented
	// CustData not implemented
	ob = PyObject_GetAttrString(obDict, "Instance");
	if (ob){
		if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&pConv->hInstance))
			return FALSE;
		}
	else
		PyErr_Clear();

	ob = PyObject_GetAttrString(obDict, "Template");
	if (ob){
		// Need cast since lpszTemplate is const.  Free'd in object dtor
		if (!PyWinObject_AsTCHAR(ob, (TCHAR **)&pConv->lpszTemplate, FALSE))
			return FALSE;
		}
	else
		PyErr_Clear();

	ob = PyObject_GetAttrString(obDict, "Resource");
	if (ob){
		if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&pConv->hResource))
			return FALSE;
		}
	else
		PyErr_Clear();

	// OLEUIINSERTOBJECT specifics
	// CLSID is out.
	ob = PyObject_GetAttrString(obDict, "File");
	if (ob) {
		TCHAR *szTemp;
		if (!PyWinObject_AsTCHAR(ob, &szTemp, FALSE))
			return FALSE;
		_tcsncpy(fileNameBuf, szTemp, sizeof(fileNameBuf)/sizeof(fileNameBuf[0]));
		fileNameBuf[sizeof(fileNameBuf)-1]='\0';
		PyWinObject_FreeTCHAR(szTemp);
		}
	else
		PyErr_Clear();

	// CLSIDExcludeList not yet supported.
	ob = PyObject_GetAttrString(obDict, "iid");
	if (ob) {
		if (!PyWinObject_AsIID(ob, &pConv->iid))
			return FALSE;
		}
	else
		PyErr_Clear();

	ob = PyObject_GetAttrString(obDict, "oleRender");
	if (ob){
		pConv->oleRender = PyInt_AsLong(ob);
		if (pConv->oleRender == -1 && PyErr_Occurred())
			return FALSE;
		}
	else
		PyErr_Clear();

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
	return PyWinObject_FromTCHAR(ret);
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
							 PYOBJ_OFFSET(PyCOleInsertDialog), 
							 PyCOleInsertDialog_methods, 
							 GET_PY_CTOR(PyCOleInsertDialog) );

