/*

	win32 template data type

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
#include "win32template.h"
#include "win32doc.h"
#include "win32menu.h"

extern CFrameWnd *GetFramePtr(PyObject *self);

class CProtectedDocument : public CDocument
{
public:
	void SetPathName(TCHAR *pathName ) {m_strPathName = pathName;}
};

/////////////////////////////////////////////////////////////////////
//
// Document object
//
//////////////////////////////////////////////////////////////////////
PyCDocTemplate::PyCDocTemplate()
{
}

PyCDocTemplate::~PyCDocTemplate()
{
	CPythonDocTemplate *pTemp = GetTemplate(this);
	if (pTemp==NULL)
		return;	// no more to do.
	RemoveDocTemplateFromApp(pTemp);
}

void PyCDocTemplate::cleanup()
{
	PyCCmdTarget::cleanup();
	CPythonDocTemplate *pTempl = GetTemplate(this);
	if (pTempl==NULL)
		OutputDebugString(_T("PyCDocTemplate::cleanup could not cleanup template!\n"));
	else {
		RemoveDocTemplateFromApp( pTempl );
		delete pTempl;
	}
}

BOOL PyCDocTemplate::RemoveDocTemplateFromApp( CDocTemplate *pTemplate )
{
	//  (Must keep templates in the same order (I think!)
	// Loop over each item, putting it at the end of the list.  When I get back to the first one, I know I am finished.

	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	CProtectedDocManager *pDocMgr =  pApp->GetDocManager();
	if (pDocMgr==NULL)
		return FALSE;
	CPtrList &templateList = pDocMgr->GetTemplateList();
	ASSERT_VALID(pTemplate);
	if (templateList.IsEmpty()) return FALSE;
	CDocTemplate *headItem = (CDocTemplate *)templateList.RemoveHead();
	CDocTemplate *item;
	if (headItem==pTemplate) return TRUE;
	BOOL ret = FALSE;
	templateList.AddTail(headItem);
	while (templateList.GetHead()!=headItem) {
		item = (CDocTemplate *)templateList.RemoveHead();
		if (item==pTemplate)
			ret = TRUE;
		else
			templateList.AddTail(item);
	}
	return ret;
}

CPythonDocTemplate *PyCDocTemplate::GetTemplate(PyObject *self)
{
	return (CPythonDocTemplate*)GetGoodCppObject( self, &type);
}

// @pymethod <o PyCDocTemplate>|win32ui|CreateDocTemplate|Creates a document template object.
PyObject *
PyCDocTemplate::create(PyObject *self, PyObject *args)
{
	UINT idResource;
	// @pyparm int|idRes||The ID for resources for documents of this type.
	if (!PyArg_ParseTuple(args,"i:CreateDocTemplate", &idResource))
		return NULL;

	CPythonDocTemplate *pMFCTemplate = new CPythonDocTemplate(idResource);
	return ui_assoc_object::make(PyCDocTemplate::type, pMFCTemplate, TRUE);
}

PyObject *
PyCDocTemplate::DoCreateDocHelper(PyObject *self, PyObject *args, CRuntimeClass *pClass, ui_type_CObject &pydoc_type)
{
	TCHAR *fileName = NULL;	// default, untitled document
	PyObject *ret = NULL;
	PyObject *obfileName=Py_None;
	if (!PyArg_ParseTuple(args,"|O", &obfileName))
		return NULL;
	if (!PyWinObject_AsTCHAR(obfileName, &fileName, TRUE))
		return NULL;
	// must exit via 'done' from here...
	CDocument *pDoc = NULL;
	if (fileName) {
		CProtectedWinApp *pApp = GetProtectedApp();
		if (!pApp) goto done;
		// need to look for an open doc of same name, and return that object.
		// Let MFC framework search for a filename for us.
		pDoc=pApp->FindOpenDocument(fileName);
	}
	// no name given, or no open document of that name
	if (pDoc==NULL) {
		CPythonDocTemplate *pMFCTemplate = GetTemplate(self);
		if (pMFCTemplate==NULL)
			goto done;
		CObject *pOb;
		GUI_BGN_SAVE;
		pOb = pClass->CreateObject();
		GUI_END_SAVE;
		if (pOb==NULL) {
			PyErr_NoMemory();
			goto done;
		}
		if (!pOb->IsKindOf( RUNTIME_CLASS(CDocument))) {
			PyErr_SetString(ui_module_error, "Internal error: Unknown created instead of a document");
			goto done;
		}
		pDoc = (CDocument *)pOb;
		pMFCTemplate->AddDocument(pDoc);
		ASSERT_VALID(pDoc);
		ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDocument)));
		if (fileName)
			((CProtectedDocument *)pDoc)->SetPathName(fileName);
//		else {
//			CString strDocName;
//			VERIFY(strDocName.LoadString(AFX_IDS_UNTITLED));
//			pDoc->SetTitle(strDocName);
//		}
	}
	ret = ui_assoc_object::make(pydoc_type, pDoc);
done:
	PyWinObject_FreeTCHAR(fileName);
	return ret;
}

// @pymethod <o PyCDocument>|PyCDocTemplate|DoCreateDoc|Creates an underlying document object.
PyObject *
PyCDocTemplate::DoCreateDoc(PyObject *self, PyObject *args)
{
	// @pyparm string|fileName|None|The name of the file to load.
	return DoCreateDocHelper(self, args, RUNTIME_CLASS(CPythonDoc), PyCDocument::type);
}


// CreateNewFrame should no longer be used.
PyObject *
PyCDocTemplate::CreateNewFrame(PyObject *self, PyObject *args)
{
	PyObject *obDoc;
	PyObject *obWndOther;

	if (!PyArg_ParseTuple(args,"|OO:CreateNewFrame",
	                      &obDoc,     //  <o PyCDocument>|doc|None|A document for the frame.
	                      &obWndOther))//  <o PyCMDIChildWnd>|wndOther|None|A window to base the new one on.
		return NULL;
	// If the doc parameter is None, a new document will be created.
	// Otherwise, the new view will be associated with the existing document specified by pDoc.
	// The wndOther parameter is used to implement the Window New command.
	// It provides a frame window on which to model the new frame window. 
	// The new frame window is usually created invisible. 
	CDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	GUI_BGN_SAVE;
	CFrameWnd *pFrame = new CPythonMDIChildWnd();
	GUI_END_SAVE;
	return ui_assoc_object::make(PyCMDIChildWnd::type, pFrame, TRUE );
}

static BOOL AFXAPI PyAfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
#ifndef _MAC
	// it is necessary to convert the paths first
	TCHAR szTemp1[_MAX_PATH];
	AfxFullPath(szTemp1, lpszPath1);
	TCHAR szTemp2[_MAX_PATH];
	AfxFullPath(szTemp2, lpszPath2);
	return lstrcmpi(szTemp1, szTemp2) == 0;
#else
	FSSpec fssTemp1;
	FSSpec fssTemp2;
	if (!UnwrapFile(lpszPath1, &fssTemp1) || !UnwrapFile(lpszPath2, &fssTemp2))
		return FALSE;
	return fssTemp1.vRefNum == fssTemp2.vRefNum &&
		fssTemp1.parID == fssTemp2.parID &&
		EqualString(fssTemp1.name, fssTemp2.name, false, true);
#endif
}

// @pymethod list|PyCDocTemplate|GetDocumentList|Return a list of all open documents.
PyObject *
PyCDocTemplate::GetDocumentList(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":GetDocumentList"))
		return NULL;
	CPythonDocTemplate *pTemplate = GetTemplate(self);
	PyObject *newOb = PyList_New(0);
	POSITION posDoc = pTemplate->GetFirstDocPosition();
	while (posDoc) {
		CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
		PyList_Append(newOb, ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet());
	}
	return newOb;
}

// @pymethod <o PyCDocument>|PyCDocTemplate|FindOpenDocument|Returns an existing document with the specified file name.
PyObject *
PyCDocTemplate::FindOpenDocument(PyObject *self, PyObject *args)
{
	TCHAR *fileName;
	PyObject *obfileName;
	// @pyparm string|fileName||The fully qualified filename to search for.
	if (!PyArg_ParseTuple(args,"O:FindOpenDocument", &obfileName))
		return NULL;
	if (!PyWinObject_AsTCHAR(obfileName, &fileName, FALSE))
		return NULL;
	CPythonDocTemplate *pTemplate = GetTemplate(self);
	POSITION posDoc = pTemplate->GetFirstDocPosition();
	while (posDoc) {
		CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
		if (PyAfxComparePath(pDoc->GetPathName(), fileName)){
			PyWinObject_FreeTCHAR(fileName);
			return ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet();
			}
		}
	PyWinObject_FreeTCHAR(fileName);
	RETURN_NONE;
}
// @pymethod string|PyCDocTemplate|GetDocString|Retrieves a specific substring describing the document type.
PyObject *
PyCDocTemplate::GetDocString(PyObject *self, PyObject *args)
{
	CDocTemplate::DocStringIndex docIndex;
	// @pyparm int|docIndex||The document index.  Must be one of the win32ui.CDocTemplate_* constants.
	if (!PyArg_ParseTuple(args, "i:GetDocString", &docIndex))
		return NULL;
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	CString csRet;
	GUI_BGN_SAVE;
	BOOL ok = pTempl->GetDocString(csRet, docIndex);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("PyCDocTemplate::GetDocString failed");
	// @comm For more information on the doc strings, please see <om PyCDocTemplate::SetDocStrings>
	return PyWinObject_FromTCHAR(csRet);
}

// @pymethod |PyCDocTemplate|GetResourceID|Returns the resource ID in use.
PyObject *
PyCDocTemplate::GetResourceID(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetResourceID);
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	return Py_BuildValue("i", pTempl->GetResourceID());
}

// @pymethod <o PyCMenu>|PyCDocTemplate|GetSharedMenu|Returns the shared menu object for all frames using this template.
PyObject *
PyCDocTemplate::GetSharedMenu(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	// @pyseemfc CWnd|m_hMenuShared
	HMENU hMenu = pTempl->m_hMenuShared;
	if (hMenu==NULL)
		RETURN_ERR("The template has no menu");
	return ui_assoc_object::make(PyCMenu::type, hMenu)->GetGoodRet();
}

// @pymethod |PyCDocTemplate|InitialUpdateFrame|Calls the default OnInitialFrame handler.
PyObject *
PyCDocTemplate::InitialUpdateFrame(PyObject *self, PyObject *args)
{
	PyObject *obDoc;
	PyObject *obFrame;
	int bMakeVisible = TRUE;

	if (!PyArg_ParseTuple(args,"OO|i:InitialUpdateFrame",
	                      &obFrame,// @pyparm <o PyCFrameWnd>|frame|None|The frame window.
						  &obDoc, // @pyparm <o PyCDocument>|doc|None|A document for the frame.
	                      &bMakeVisible))// @pyparm int|bMakeVisible|1|Indicates of the frame should be shown.
		return NULL;
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	CDocument *pDoc = PyCDocument::GetDoc(obDoc);
	CFrameWnd *pFrame = GetFramePtr(obFrame);
	if (pFrame==NULL || pDoc==NULL) return NULL;
	// @xref <vm PyCDocTemplate.InitialUpdateFrame>
	GUI_BGN_SAVE;
	pTempl->CMultiDocTemplate::InitialUpdateFrame(pFrame, pDoc, bMakeVisible);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCDocTemplate|OpenDocumentFile|Opens a document file, creating a view and frame.
PyObject *
PyCDocTemplate::OpenDocumentFile(PyObject *self, PyObject *args)
{
	TCHAR *fileName = NULL;
	PyObject *obfileName=Py_None;
	BOOL bMakeVisible = TRUE;
	if (!PyArg_ParseTuple(args,"|Oi:OpenDocumentFile",
	                      &obfileName,     // @pyparm string|filename||Name of file to open, or None
	                      &bMakeVisible))// @pyparm int|bMakeVisible|1|Indicates if the document should be created visible.
		return NULL;
	CMultiDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	if (!PyWinObject_AsTCHAR(obfileName, &fileName, TRUE))
		return NULL;
	GUI_BGN_SAVE;
	CDocument *pDocument = pTempl->CMultiDocTemplate::OpenDocumentFile(fileName, bMakeVisible);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(fileName);
	if (PyErr_Occurred())
		return NULL;
	if (pDocument==NULL)
		RETURN_NONE;
	return ui_assoc_object::make(PyCDocument::type, pDocument )->GetGoodRet();
}

// @pymethod |PyCDocTemplate|SetDocStrings|Assigns the document strings for the template.
PyObject *
PyCDocTemplate::SetDocStrings(PyObject *self, PyObject *args)
{
	char *docStrings;
	if (!PyArg_ParseTuple(args,"s:SetDocStrings",
	                      &docStrings))     // @pyparm string|docStrings||The document strings.
		return NULL;
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	pTempl->m_strDocStrings = docStrings;
	// @comm The string must be a \n seperated list of docstrings.
	// The elements are:
	// @flagh elementName|Description
	// @flag windowTitle|Title used for the window (only for SDI applications)
	// @flag docName|Root for the default document name.
	// @flag fileNewName|Name of the document type, as displayed in the "File/New" dialog
	// @flag filterName|Description of the document type and a wildcard spec for the file open dialog.
	// @flag filterExt|Extension for documents of this file type.
	// @flag regFileTypeId|Internal Id of the document as registered in the registry.  Used to associate the extension with the file type.
	// @flag regFileTypeName|Name of the document, as stored in the reigstry.  This is the name presented to the user.
	RETURN_NONE;
}
// @pymethod |PyCDocTemplate|SetContainerInfo|Sets the resources to be used when an OLE 2 object is in-place activated.
PyObject *
PyCDocTemplate::SetContainerInfo(PyObject *self, PyObject *args)
{
	int id;
	if (!PyArg_ParseTuple(args,"i:SetContainerInfo",
	                      &id))     // @pyparm int|id||The resource ID.
		return NULL;
	CPythonDocTemplate *pTempl = GetTemplate(self);
	if (pTempl==NULL)
		return NULL;
	GUI_BGN_SAVE;
	pTempl->SetContainerInfo(id);
	GUI_END_SAVE;
	RETURN_NONE;
}


// @object PyCDocTemplate|A document template class.  Encapsulates an MFC <c CDocTemplate> class
static struct PyMethodDef PyCDocTemplate_methods[] = {
	{"CreateNewFrame",    PyCDocTemplate::CreateNewFrame, 1},
	{"DoCreateDoc",       PyCDocTemplate::DoCreateDoc,      1}, // @pymeth DoCreateDoc|Creates an underlying document object.
	{"FindOpenDocument",  PyCDocTemplate::FindOpenDocument, 1}, // @pymeth FindOpenDocument|Returns an existing document with the specified file name.
	{"GetDocString",      PyCDocTemplate::GetDocString, 1}, // @pymeth GetDocString|Retrieves a specific substring describing the document type.
	{"GetDocumentList",   PyCDocTemplate::GetDocumentList, 1}, // @pymeth GetDocumentList|Return a list of all open documents.
	{"GetResourceID",     PyCDocTemplate::GetResourceID,  1}, // @pymeth GetResourceID|Returns the resource ID in use.
	{"GetSharedMenu",     PyCDocTemplate::GetSharedMenu,  1}, // @pymeth GetSharedMenu|Returns the shared menu object for all frames using this template.
	{"InitialUpdateFrame",PyCDocTemplate::InitialUpdateFrame, 1}, // @pymeth InitialUpdateFrame|Calls the default OnInitialFrame handler.
	{"SetContainerInfo",  PyCDocTemplate::SetContainerInfo, 1}, // @pymeth SetContainerInfo|Sets the resources to be used when an OLE 2 object is in-place activated.
	{"SetDocStrings",     PyCDocTemplate::SetDocStrings, 1}, // @pymeth SetDocStrings|Assigns the document strings for the template.
	{"OpenDocumentFile",  PyCDocTemplate::OpenDocumentFile, 1}, // @pymeth OpenDocumentFile|Opens a document file, creating a view and frame.
	{NULL,			NULL}
};
ui_type_CObject PyCDocTemplate::type("PyCDocTemplate", 
									 &PyCCmdTarget::type, 
									 RUNTIME_CLASS(CDocTemplate), 
									 sizeof(PyCDocTemplate), 
									 PYOBJ_OFFSET(PyCDocTemplate), 
									 PyCDocTemplate_methods, 
									 GET_PY_CTOR(PyCDocTemplate) );

// The MFC class.
CPythonDocTemplate::CPythonDocTemplate(UINT idResource) : 
	CMultiDocTemplate(idResource, NULL, NULL, NULL)
{
}

// The MFC class.
CPythonDocTemplate::~CPythonDocTemplate()
{
	Python_delete_assoc( this ); // notify Python of my death.
}

void CPythonDocTemplate::InitialUpdateFrame( CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible)
{
	// @pyvirtual |PyCDocTemplate|InitialUpdateFrame|Called to perform the initial frame update.
	// The default behaviour is to call OnInitialUpdate on all views.
	CVirtualHelper helper("InitialUpdateFrame", this);
	if (!helper.HaveHandler()) {
		CMultiDocTemplate::InitialUpdateFrame(pFrame, pDoc, bMakeVisible);
		return;
	}
	PyObject *frame = (PyObject *)ui_assoc_object::make (PyCFrameWnd::type,
												   pFrame)->GetGoodRet();
	PyObject *doc = (PyObject *) ui_assoc_object::make (PyCDocument::type,
												   pDoc)->GetGoodRet();

	// @pyparm <o PyCFrameWnd>|frame||The frame window.
	// @pyparm <o PyCDocument>|frame||The document attached to the frame.
	// @pyparm int|bMakeVisible||Indicates if the frame should be made visible.
	PyObject *arglst = Py_BuildValue("(OOi)",frame, doc, bMakeVisible);
	XDODECREF(frame);
	XDODECREF(doc);
	helper.call_args(arglst);
	return;
}

CFrameWnd* CPythonDocTemplate::CreateNewFrame( CDocument* pDoc, CFrameWnd* pOther )
{
	BOOL ok;
	// @pyvirtual <o PyCMDIChildWnd>|PyCDocTemplate|CreateNewFrame|Called to create a new frame window.
	CVirtualHelper helper("CreateNewFrame", this);
	ok = helper.call(pDoc);
	PyObject *retObject=NULL;
	// ??? This needs to differentiate between callback error and wrong type ???
	ok = ok && helper.retval( retObject );
	ok = ok && ui_base_class::is_uiobject( retObject, &PyCFrameWnd::type );
	if (!ok) {
		CEnterLeavePython _celp;
		if (PyErr_Occurred())
			gui_print_error();
		const char *typ_str = retObject ? retObject->ob_type->tp_name : "<null>";
		PyErr_Format(PyExc_TypeError,
					 "PyCTemplate::CreateNewFrame must return a PyCFrameWnd object (got %s).",
					 typ_str);
		gui_print_error();
		return NULL;
	}
	CFrameWnd *pWnd = GetFramePtr( retObject );
	return pWnd;
}

CDocument* CPythonDocTemplate::CreateNewDocument()
{
	// @pyvirtual <o PyCDocument>|PyCDocTemplate|CreateNewDocument|Called to create a new document object.
	CVirtualHelper helper("CreateNewDocument", this);
	BOOL ok = helper.HaveHandler();
	if (!ok) {
		CEnterLeavePython _celp;
		PyErr_SetString(ui_module_error, "PyCTemplate::CreateNewDocument handler does not exist.");
		TRACE0("CPythonDocTemplate::CreateNewDocument fails due to no handler\n");
		return NULL;
	}
		
	ok = ok && helper.call();
	PyObject *retObject=NULL;
	ok = ok && helper.retval( retObject );
	ok = ok && ui_base_class::is_uiobject( retObject, &PyCDocument::type );

	if (!ok) {
		CEnterLeavePython _celp;
		if (PyErr_Occurred())
			gui_print_error();
		const char *typ_str = retObject ? retObject->ob_type->tp_name : "<null>";
		PyErr_Format(PyExc_TypeError,
					 "PyCTemplate::CreateNewDocument must return a PyCDocument object (got %s).",
					 typ_str);
		TRACE0("CPythonDocTemplate::CreateNewDocument fails due to return type error\n");
		return NULL;
	}
	CDocument *pDoc = PyCDocument::GetDoc( retObject );
	return pDoc;
}

CDocument* CPythonDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible /*= TRUE*/ )
{
	// @pyvirtual <o PyCDocument>|PyCDocTemplate|OpenDocumentFile|Called when a document file is to be opened.
	CVirtualHelper helper("OpenDocumentFile", this);
	BOOL ok = helper.HaveHandler();
	if (!ok)
		return CMultiDocTemplate::OpenDocumentFile(lpszPathName, bMakeVisible);
		
	ok = ok && helper.call(lpszPathName, bMakeVisible);
	PyObject *retObject=NULL;
	ok = ok && helper.retval( retObject );
	if (retObject==Py_None) // If we failed, get out.
		return NULL;
	ok = ok && ui_base_class::is_uiobject( retObject, &PyCDocument::type );

	if (!ok) {
		CEnterLeavePython _celp;
		if (PyErr_Occurred())
			gui_print_error();
		const char *typ_str = retObject ? retObject->ob_type->tp_name : "<null>";
		PyErr_Format(PyExc_TypeError,
					 "PyCTemplate::OpenDocumentFile must return a PyCDocument object (got %s).",
					 typ_str);
		TRACE0("CPythonDocTemplate::CreateNewDocument fails due to return type error\n");
		return NULL;
	}
	return PyCDocument::GetDoc( retObject );
}

#ifndef _MAC
CDocTemplate::Confidence CPythonDocTemplate::MatchDocType(LPCTSTR lpszPathName,
	CDocument*& rpDocMatch)
{
	DWORD dwFileType=0;
#else
CDocTemplate::Confidence CPythonDocTemplate::MatchDocType(LPCTSTR lpszPathName,
	DWORD dwFileType, CDocument*& rpDocMatch)
{
#endif
	// @pyvirtual int\|<o PyCDocument>|PyCDocTemplate|MatchDocType|Queries if the template can open the specified file name.
	// @comm This method should call PyCDocTemplate.FindOpenDocument to return an already open
	// document if one exists, else it should return one of the win32ui.CDocTemplate_Confidence_* constants.
	CVirtualHelper helper("MatchDocType", this);
	if (helper.HaveHandler()) {
		// @pyparm string|fileName||The name of the file to open.
		// @pyparm int|fileType||Only used on the mac.
		helper.call(lpszPathName, dwFileType);
		PyObject *ret;
		if (!helper.retval(ret))
			return CDocTemplate::noAttempt;
		if (PyInt_Check(ret))
			return (CDocTemplate::Confidence)PyInt_AsLong(ret);
		if (ui_base_class::is_uiobject(ret, &PyCDocument::type)) {
			CDocument *pDoc = PyCDocument::GetDoc(ret);
			rpDocMatch = pDoc;
			return yesAlreadyOpen;
		}
		CEnterLeavePython _celp;
		const char *typ_str = ret ? ret->ob_type->tp_name : "<null>";
		PyErr_Format(PyExc_TypeError,"PyCTemplate::MatchDocType must return an integer or PyCDocument object (got %s).",
					 typ_str);
 		gui_print_error();
		return CDocTemplate::noAttempt;
	} else {
		return CDocTemplate::noAttempt;
	}
}
