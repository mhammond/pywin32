/*

	win32 app data type

	Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32ui.h"
#include "win32doc.h"
#include "win32template.h"

extern CWnd *GetWndPtr(PyObject *self);

PyObject *PyCWinApp::pExistingAppObject = NULL;
char *errmsgAlreadyInit = "The application has already been initialised";

/////////////////////////////////////////////////////////////////////
//
// CProtectedWinApp Application helpers.
//
//////////////////////////////////////////////////////////////////////
CString CProtectedWinApp::GetRecentFileName(int index)
{
	if (index>=0 && index < _AFX_MRU_MAX_COUNT) {
		return (*m_pRecentFileList)[index];
	}
	else {
		ASSERT(0);
		return CString();
	}
}

void CProtectedWinApp::RemoveRecentFile(int index)
{
	if (index>=0 && index < _AFX_MRU_MAX_COUNT) {
		m_pRecentFileList->Remove(index);
	}
}

PyObject *CProtectedWinApp::MakePyDocTemplateList()
{
	PyObject *retList = PyList_New(0);
	if (retList==NULL)
		return NULL;
	POSITION posTempl = m_pDocManager ? m_pDocManager->GetFirstDocTemplatePosition() : NULL;
	while (posTempl) {
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(posTempl);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		PyObject *newListItem = ui_assoc_object::make(PyCDocTemplate::type, pTemplate)->GetGoodRet();
		if (newListItem==NULL) {
			Py_DECREF(retList);
			return NULL;
		}
		PyList_Append(retList, newListItem);
		Py_DECREF(newListItem);
	}
	return retList;
}

// FindOpenDocument - if the C++ framework has a document with this name open,
// then return a pointer to it, else NULL.
CDocument *CProtectedWinApp::FindOpenDocument(const char *lpszFileName)
{
	POSITION posTempl = m_pDocManager->GetFirstDocTemplatePosition();
	CDocument* pOpenDocument = NULL;

	char szPath[_MAX_PATH];
	if (!GetFullPath(szPath, lpszFileName))
		strcpy(szPath, lpszFileName);

	while (posTempl) {
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(posTempl);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		// go through all documents
		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc) {
			CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
			if (lstrcmpi(pDoc->GetPathName(), szPath) == 0)
				return pDoc;
		}
	}
	return NULL;
}

CProtectedDocManager *CProtectedWinApp::GetDocManager()
{
	CProtectedDocManager *ret = (CProtectedDocManager *)m_pDocManager;
	if (!ret->IsKindOf(RUNTIME_CLASS(CDocManager)))
		RETURN_ERR("There is not a valid Document Manager");
	return ret;
}

extern BOOL bDebuggerPumpStopRequested;

/////////////////////////////////////////////////////////////////////
//
// Application object
//
//////////////////////////////////////////////////////////////////////
PyCWinApp::PyCWinApp()
{
	ASSERT(pExistingAppObject== NULL);
}

PyCWinApp::~PyCWinApp()
{
	XDODECREF(pExistingAppObject);
	pExistingAppObject = NULL;
}

// @pymethod |PyCWinApp|AddDocTemplate|Adds a template to the application list.
static PyObject *
ui_app_add_doc_template(PyObject *self, PyObject *args)
{
	PyObject *obTemplate;
	if (!PyArg_ParseTuple(args,"O:AddDocTemplate",
	                      &obTemplate))     // @pyparm <o PyCDocTemplate>|template||The template to be added.
		return NULL;

	if (!ui_base_class::is_uiobject(obTemplate, &PyCDocTemplate::type))
		RETURN_TYPE_ERR("The parameter must be a template object");

	CDocTemplate *pTempl = PyCDocTemplate::GetTemplate(obTemplate);
	if (pTempl==NULL)
		return NULL;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	// walk all templates in the application looking for it.
	CDocTemplate* pTemplate;
	POSITION pos = pApp->m_pDocManager ? pApp->m_pDocManager->GetFirstDocTemplatePosition() : NULL;
	while (pos != NULL) {
		pTemplate = pApp->m_pDocManager->GetNextDocTemplate(pos);
		if (pTemplate==pTempl)
			RETURN_ERR("The template is already in the application list");
	}
	GUI_BGN_SAVE;
	pApp->AddDocTemplate(pTempl);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCWinApp|RemoveDocTemplate|Removes a template to the application list.
static PyObject *
ui_app_remove_doc_template(PyObject *self, PyObject *args)
{
	// @comm Note that MFC does not provide an equivilent function.
	PyObject *obTemplate;
	if (!PyArg_ParseTuple(args,"O:RemoveDocTemplate",
	                      &obTemplate))     // @pyparm <o PyCDocTemplate>|template||The template to be removed.  Must have previously been added by <om PyCWinApp.AddDocTemplate>.
		return NULL;

	if (!ui_base_class::is_uiobject(obTemplate, &PyCDocTemplate::type))
		RETURN_TYPE_ERR("The parameter must be a template object");

	CDocTemplate *pTempl = PyCDocTemplate::GetTemplate(obTemplate);
	if (pTempl==NULL)
		return NULL;
	GUI_BGN_SAVE;
	BOOL ok = PyCDocTemplate::RemoveDocTemplateFromApp( pTempl );
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("The template is not in the application template list");
	RETURN_NONE;
}


static PyObject *
ui_init_mdi_instance(PyObject *self, PyObject *args)
{
	RETURN_NONE;
}

// @pymethod |PyCWinApp|OpenDocumentFile|Opens a document file by name.
static PyObject *
ui_open_document_file(PyObject *self, PyObject *args)
{
	char *fileName;
	if (!PyArg_ParseTuple(args, "s:OpenDocumentFile",
	                       &fileName )) // @pyparm string|fileName||The name of the document to open.
		return NULL;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;

	if (((CProtectedWinApp *)pApp)->GetMainFrame()->GetSafeHwnd()==0)
		RETURN_ERR("There is no main frame in which to create the document");

	GUI_BGN_SAVE;
	CDocument *pDoc = pApp->OpenDocumentFile(fileName);
	GUI_END_SAVE;
	if (PyErr_Occurred())
		return NULL;
	if (pDoc==NULL)
		RETURN_NONE;
	return ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet();
}

// @pymethod <o PyCDocument>|PyCWinApp|FindOpenDocument|Returns an existing document with the specified file name.
static PyObject *
ui_find_open_document(PyObject *self, PyObject *args)
{
	char *fileName;
	// @pyparm string|fileName||The fully qualified filename to search for.
	if (!PyArg_ParseTuple(args,"s", &fileName))
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	// Let MFC framework search for a filename for us.
	GUI_BGN_SAVE;
	CDocument *pDoc=pApp->FindOpenDocument(fileName);
	GUI_END_SAVE;
	if (pDoc==NULL)
		RETURN_NONE;
	return ui_assoc_object::make(PyCDocument::type, pDoc)->GetGoodRet();
}

// @pymethod <o PyCWinApp>|win32ui|GetApp|Retrieves the application object.
PyObject *
ui_get_app(PyObject *self, PyObject *args)
{
	// @comm There will only ever be one application object per application.
	CHECK_NO_ARGS2(args,GetApp);
	CWinApp *pApp = GetApp();
	if (pApp==NULL) return NULL;
	return ui_assoc_object::make(PyCWinApp::type, pApp)->GetGoodRet();
}

// @pymethod |PyCWinApp|OnFileNew|Calls the underlying OnFileNew MFC method.
static PyObject *
ui_on_file_new(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,OnFileNew);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	GUI_BGN_SAVE;
	pApp->OnFileNew();
	GUI_END_SAVE;
	RETURN_NONE;
}
// @pymethod |PyCWinApp|OnFileOpen|Calls the underlying OnFileOpen MFC method.
static PyObject *
ui_on_file_open(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,OnFileNew);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	GUI_BGN_SAVE;
	pApp->OnFileOpen();
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod int|PyCWinApp|LoadCursor|Loads a cursor.
static PyObject *ui_load_cursor(PyObject *self, PyObject *args)
{
	UINT cid;
	char *csid;
	HCURSOR hc;
	if ( PyArg_ParseTuple(args, "i",
						   &cid)) // @pyparm int|cursorId||The ID of the cursor to load.
		hc = GetApp()->LoadCursor(cid);
	else {
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "s",
						   &csid)) // @pyparmalt1 string|cursorId||The ID of the cursor to load.
			hc = GetApp()->LoadCursor(csid);
		else
			RETURN_TYPE_ERR("The first param must be an integer or a string");
	}
	if (hc==0)
		RETURN_API_ERR("LoadCursor");
	return PyInt_FromLong((long)hc);
}

// @pymethod int|PyCWinApp|LoadStandardCursor|Loads a standard cursor.
static PyObject *ui_load_standard_cursor(PyObject *self, PyObject *args)
{
	UINT cid;
	char *csid;
	HCURSOR hc;
	if ( PyArg_ParseTuple(args, "i",
						   &cid)) // @pyparm int|cursorId||The ID of the cursor to load.
		hc = GetApp()->LoadStandardCursor(MAKEINTRESOURCE(cid));
	else {
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "s", // @pyparmalt1 string|cursorId||The ID of the cursor to load.
						   &csid))
			hc = GetApp()->LoadStandardCursor(csid);
		else
			RETURN_TYPE_ERR("The first param must be an integer or a string");
	}
	if (hc==0)
		RETURN_API_ERR("LoadStandardCursor");
	return PyInt_FromLong((long)hc);
}

// @pymethod int|PyCWinApp|LoadOEMCursor|Loads an OEM cursor.
static PyObject *ui_load_oem_cursor(PyObject *self, PyObject *args)
{
	UINT cid;
	HCURSOR hc;
	if ( !PyArg_ParseTuple(args, "i",
						   &cid)) // @pyparm int|cursorId||The ID of the cursor to load.
		return NULL;
	hc = GetApp()->LoadOEMCursor(cid);
	if (hc==0)
		RETURN_API_ERR("LoadOEMCursor");
	return PyInt_FromLong((long)hc);
}

// @pymethod int|PyCWinApp|LoadIcon|Loads an icon resource.
static PyObject *
ui_load_icon(PyObject *self, PyObject *args)
{
	int idResource;
	// @pyparm int|idResource||The ID of the icon to load.
	if (!PyArg_ParseTuple(args,"i:LoadIcon", &idResource))
		return NULL;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	return Py_BuildValue("i", pApp->LoadIcon(idResource));
}

// @pymethod int|PyCWinApp|LoadStandardIcon|Loads an icon resource.
static PyObject *
ui_load_standard_icon(PyObject *self, PyObject *args)
{
	char *resName;
	// @pyparm string|resourceName||The name of the standard icon to load.
	if (!PyArg_ParseTuple(args,"s:LoadStandardIcon", &resName))
		return NULL;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	return Py_BuildValue("i", pApp->LoadStandardIcon(resName));
}


// @pymethod int|PyCWinApp|Run|Starts the message pump.  Advanced users only
static PyObject *
ui_app_run(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args, "Run");
	GUI_BGN_SAVE;
	long rc = AfxGetApp()->CWinApp::Run();
	GUI_END_SAVE;

	return PyInt_FromLong(rc);
}

// @pymethod int|PyCWinApp|IsInproc|Returns a flag to indicate if the created CWinApp was in the DLL, or an external EXE.
static PyObject *
ui_app_isinproc(PyObject *self, PyObject *args)
{
	extern BOOL PyWin_bHaveMFCHost;
	CHECK_NO_ARGS2(args, IsInproc);
	return PyInt_FromLong(!PyWin_bHaveMFCHost);
}

// @pymethod [<o PyCDocTemplate>,...]|PyCWinApp|GetDocTemplateList|Returns a list of all document templates.
static PyObject *
ui_app_get_doc_template_list(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args, GetDocTemplateList);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	return pApp->MakePyDocTemplateList();
}

extern PyObject *ui_init_dlg_instance(PyObject *self, PyObject *args);

// @object PyCWinApp|An application class.  Encapsulates an MFC <c CWinApp> class
static struct PyMethodDef PyCWinApp_methods[] = {
	{"AddDocTemplate",    		ui_app_add_doc_template, 1 }, // @pymeth AddDocTemplate|Adds a template to the application list.
	{"FindOpenDocument",		ui_find_open_document,	1}, // @pymeth FindOpenDocument|Returns an existing document with the specified file name.
	{"GetDocTemplateList",      ui_app_get_doc_template_list, 1}, // @pymeth GetDocTemplateList|Returns a list of all document templates in use.
    {"InitMDIInstance",			ui_init_mdi_instance,	1},
    {"InitDlgInstance",			ui_init_dlg_instance,	1}, // @pymeth InitDlgInstance|Calls critical InitInstance processing for a dialog based application.
	{"LoadCursor",              ui_load_cursor,         1},	//@pymeth LoadCursor|Loads a cursor.
	{"LoadStandardCursor",      ui_load_standard_cursor,1},	//@pymeth LoadStandardCursor|Loads a standard cursor.
	{"LoadOEMCursor",           ui_load_oem_cursor,     1},	//@pymeth LoadOEMCursor|Loads an OEM cursor.
	{"LoadIcon",				ui_load_icon,			1}, // @pymeth LoadIcon|Loads an icon resource.
	{"LoadStandardIcon",		ui_load_standard_icon,  1}, // @pymeth LoadStandardIcon|Loads an icon resource.
    {"OpenDocumentFile",		ui_open_document_file,	1}, // @pymeth OpenDocumentFile|Opens a document file by name.
	{"OnFileNew",               ui_on_file_new,         1}, // @pymeth OnFileNew|Calls the underlying OnFileNew MFC method.
	{"OnFileOpen",              ui_on_file_open,         1}, // @pymeth OnFileOpen|Calls the underlying OnFileOpen MFC method.
	{"RemoveDocTemplate",    	ui_app_remove_doc_template, 1}, // @pymeth RemoveDocTemplate|Removes a template to the application list.
	{"Run",    					ui_app_run, 1}, // @pymeth Run|Starts the main application message pump.
	{"IsInproc",    			ui_app_isinproc, 1}, // @pymeth IsInproc|Returns a flag to indicate if the created CWinApp was in the DLL, or an external EXE.
	{NULL,			NULL}
};
ui_type_CObject PyCWinApp::type("PyCWinApp", 
								&PyCWinThread::type,
								RUNTIME_CLASS(CWinApp), 
								sizeof(PyCWinApp), 
								PyCWinApp_methods, 
								GET_PY_CTOR(PyCWinApp) );

void PyCWinApp::cleanup()
{
	PyCWinThread::cleanup();
	// total hack!
	while (pExistingAppObject)
		DODECREF(pExistingAppObject); // this may delete it.
}
