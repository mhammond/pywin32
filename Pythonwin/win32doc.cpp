/*

    win32 document data type

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
#include "win32template.h"

/////////////////////////////////////////////////////////////////////
//
// Document object
//
//////////////////////////////////////////////////////////////////////
PyCDocument::PyCDocument() {}

PyCDocument::~PyCDocument() {}
/*static*/ CDocument *PyCDocument::GetDoc(PyObject *self) { return (CDocument *)GetGoodCppObject(self, &type); }

// @pymethod |PyCDocument|DoFileSave|Checks the file attributes.
// If the file is read only, a new name is prompted, else the
// file is saved (by calling DoSave)
PyObject *ui_doc_do_file_save(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, DoFileSave);
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pDoc->CDocument::DoFileSave();  // @pyundocmfc CDocument|DoFileSave
    GUI_END_SAVE;
    // @xref <vm PyCDocument.DoFileSave>
    if (rc == FALSE)
        RETURN_ERR("DoFileSave failed");
    RETURN_NONE;
}

// @pymethod |PyCDocument|DeleteContents|Call the MFC DeleteContents method.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_doc_delete_contents(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    // @xref <vm PyCDocument.DeleteContents>
    GUI_BGN_SAVE;
    pDoc->CDocument::DeleteContents();  // @pyseemfc CDocument|DeleteContents
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCDocument|DoSave|Calls the underlying MFC DoSave method.
PyObject *ui_doc_do_save(PyObject *self, PyObject *args)
{
    // @comm If invalid or no filename, will prompt for a name, else
    // will perform the actual saving of the document.
    TCHAR *fileName;
    PyObject *obfileName;
    int bReplace = TRUE;
    if (!PyArg_ParseTuple(args, "O|i",
                          &obfileName,  // @pyparm string|fileName||The name of the file to save to.
                          &bReplace))   // @pyparm int|bReplace|1|Should an existing file be silently replaced?.
        return NULL;
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyWinObject_AsTCHAR(obfileName, &fileName, FALSE))
        return NULL;
    // @xref <vm PyCDocument.DoSave>
    GUI_BGN_SAVE;
    BOOL rc = pDoc->CDocument::DoSave(fileName, bReplace);  // @pyundocmfc CDocument|DoSave
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(fileName);
    if (rc == FALSE)
        RETURN_ERR("DoSave failed");
    RETURN_NONE;
}

// @pymethod <o PyCView>|PyCDocument|GetFirstView|Returns the first view object attached to this document.
PyObject *ui_doc_get_first_view(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, GetFirstView);
    POSITION pos = pDoc->GetFirstViewPosition();  // @pyseemfc CDocument|GetFirstViewPosition
    if (pos == NULL)
        RETURN_NONE;
    GUI_BGN_SAVE;
    CView *pWnd = pDoc->GetNextView(pos);  // @pyseemfc CDocument|GetNextView
    GUI_END_SAVE;

    // @comm For more info, see <om PyCDocument.GetAllViews>
    ASSERT(pWnd);  // shouldnt be possible.
    return ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}

// @pymethod [<o PyCView>,...]|PyCDocument|GetAllViews|Returns a list of all views for the current document.
PyObject *ui_doc_get_all_views(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, GetAllViews);
    PyObject *retList = PyList_New(0);
    GUI_BGN_SAVE;
    POSITION pos = pDoc->GetFirstViewPosition();  // @pyseemfc CDocument|GetFirstViewPosition
    GUI_END_SAVE;
    while (pos != NULL) {
        GUI_BGN_SAVE;
        CView *pWnd = pDoc->GetNextView(pos);  // @pyseemfc CDocument|GetNextView
        GUI_END_SAVE;
        ASSERT(pWnd);  // shouldnt be possible.
        if (pWnd == NULL) {
            Py_DECREF(retList);
            RETURN_ERR("No view was available!");
        }
        PyObject *newObj = ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
        if (newObj == NULL) {
            Py_DECREF(retList);
            return NULL;
        }
        PyList_Append(retList, newObj);
        Py_DECREF(newObj);
    }
    return retList;
}

// @pymethod string|PyCDocument|GetPathName|Returns the full path name of the current document.
// The string will be empty if no path name has been set.
PyObject *ui_doc_get_path_name(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, GetPathName);
    GUI_BGN_SAVE;
    CString path = pDoc->GetPathName();  // @pyseemfc CDocument|GetPathName
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(path);
}
// @pymethod <o PyCDocTemplate>|PyCDocument|GetDocTemplate|Returns the template for the document.
PyObject *ui_doc_get_template(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, GetDocTemplate);
    GUI_BGN_SAVE;
    CDocTemplate *ret = pDoc->GetDocTemplate();
    GUI_END_SAVE;
    // @pyseemfc CDocument|GetDocTemplate
    return ui_assoc_object::make(PyCDocTemplate::type, ret)->GetGoodRet();
}

// @pymethod string|PyCDocument|GetTitle|Returns the title of the current document.
// This will often be the file name portion of the path name.
PyObject *ui_doc_get_title(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, GetTitle);
    GUI_BGN_SAVE;
    CString path = pDoc->GetTitle();  // @pyseemfc CDocument|GetTitle
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(path);
}
// @pymethod int|PyCDocument|IsModified|Return a flag indicating if the document has been modified.
PyObject *ui_doc_is_modified(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    CHECK_NO_ARGS2(args, IsModified);
    GUI_BGN_SAVE;
    int rc = pDoc->IsModified();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CDocument|IsModified
}

// @pymethod |PyCDocument|OnCloseDocument|Call the MFC OnCloseDocument handler.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_doc_on_close(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnCloseDocument);
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    // @xref <vm PyCDocument.OnCloseDocument>
    GUI_BGN_SAVE;
    pDoc->CDocument::OnCloseDocument();  // @pyseemfc CDocument|OnCloseDocument
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCDocument|OnNewDocument|Call the MFC OnNewDocument handler.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_doc_on_new(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    // @xref <vm PyCDocument.OnNewDocument>
    GUI_BGN_SAVE;
    BOOL ok = pDoc->CDocument::OnNewDocument();
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDocument|OnNewDocument
        RETURN_ERR("OnNewDocument failed");
    RETURN_NONE;
}

// @pymethod |PyCDocument|OnOpenDocument|Call the MFC OnOpenDocument handler.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_doc_on_open(PyObject *self, PyObject *args)
{
    TCHAR *pathName;
    PyObject *obpathName;
    if (!PyArg_ParseTuple(args, "O", &obpathName))  // @pyparm string|pathName||The full path of the file to open.
        return NULL;
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyWinObject_AsTCHAR(obpathName, &pathName, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pDoc->OnOpenDocument(pathName);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(pathName);
    if (!ok)  // @pyseemfc CDocument|OnOpenDocument
        RETURN_ERR("OnOpenDocument failed");
    RETURN_NONE;
}

// @pymethod |PyCDocument|OnSaveDocument|Call the MFC OnSaveDocument handler.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_doc_on_save(PyObject *self, PyObject *args)
{
    TCHAR *pathName;
    PyObject *obpathName;
    if (!PyArg_ParseTuple(args, "O", &obpathName))  // @pyparm string|pathName||The full path of the file to save.
        return NULL;
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyWinObject_AsTCHAR(obpathName, &pathName, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pDoc->OnSaveDocument(pathName);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(pathName);
    if (!ok)  // @pyseemfc CDocument|OnSaveDocument
        RETURN_ERR("OnSaveDocument failed");
    RETURN_NONE;
}

// @pymethod int|PyCDocument|SaveModified|Call the underlying MFC method.
static PyObject *ui_doc_save_modified(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, SaveModified);
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    // @xref <vm PyCDocument.SaveModified>
    GUI_BGN_SAVE;
    BOOL rc = pDoc->CDocument::SaveModified();  // @pyseemfc CDocument|SaveModified
    GUI_END_SAVE;
    // @rdesc Nonzero if it is safe to continue and close the document; 0 if the document should not be closed.
    return PyInt_FromLong(rc);
}

// @pymethod |PyCDocument|SetModifiedFlag|Set the "dirty" flag for the document.
static PyObject *ui_doc_set_modified_flag(PyObject *self, PyObject *args)
{
    BOOL bModified = TRUE;
    if (!PyArg_ParseTuple(args, "|i:SetModifiedFlag", &bModified))  // @pyparm int|bModified|1|Set dirty flag
        return NULL;

    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    GUI_BGN_SAVE;
    pDoc->SetModifiedFlag(bModified);  // @pyseemfc CDocument|SetModifiedFlag
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCDocument|SetPathName|Set the full path name for the document.
PyObject *ui_doc_set_path_name(PyObject *self, PyObject *args)
{
    TCHAR *path;
    PyObject *obpath;
    if (!PyArg_ParseTuple(args, "O:SetPathName", &obpath))  // @pyparm string|path||The full path of the file.
        return NULL;

    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyWinObject_AsTCHAR(obpath, &path, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pDoc->SetPathName(path);  // @pyseemfc CDocument|SetPathName
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(path);
    RETURN_NONE;
}

// @pymethod |PyCDocument|SetTitle|Set the title of the document (ie, the name
// to appear in the window caption for the document.
static PyObject *ui_doc_set_title(PyObject *self, PyObject *args)
{
    TCHAR *title;
    PyObject *obtitle;
    if (!PyArg_ParseTuple(args, "O", &obtitle))  // @pyparm string|title||The new title.
        return NULL;
    if (!PyWinObject_AsTCHAR(obtitle, &title, FALSE))
        return NULL;
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    GUI_BGN_SAVE;
    pDoc->SetTitle(title);  // @pyseemfc CDocument|SetTitle
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(title);
    RETURN_NONE;
}

// @pymethod |PyCDocument|UpdateAllViews|Informs each view when a document changes.
static PyObject *ui_doc_update_all_views(PyObject *self, PyObject *args)
{
    PyObject *obSender;
    PyObject *obHint = Py_None;
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyArg_ParseTuple(args, "O|O:UpdateAllViews",
                          &obSender,  // @pyparm <o PyCView>|sender||The view who initiated the update
                          &obHint))   // @pyparm object|hint|None|A hint for the update.
        return NULL;
    CView *pView = NULL;
    if (obSender != Py_None) {
        if (!(pView = PyCView::GetViewPtr(obSender)))
            return NULL;
    }
    if (obHint == Py_None)
        obHint = NULL;
    GUI_BGN_SAVE;
    pDoc->UpdateAllViews(pView, (LPARAM)obHint);  // @pyseemfc CDocument|UpdateAllViews
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCDocument|OnChangedViewList|Informs the document when a view is added or removed.
static PyObject *ui_doc_on_changed_view_list(PyObject *self, PyObject *args)
{
    CDocument *pDoc;
    if (!(pDoc = PyCDocument::GetDoc(self)))
        return NULL;
    if (!PyArg_ParseTuple(args, ":OnChangedViewList"))
        return NULL;
    GUI_BGN_SAVE;
    pDoc->CDocument::OnChangedViewList();
    GUI_END_SAVE;
    RETURN_NONE;
}

///////////////////////////////////////
//
// Document Methods
//
// inherited from assoc_object
//
///////////////////////////////////////
// @object PyCDocument|A document class.  Encapsulates an MFC <c CDocument> class
static struct PyMethodDef ui_doc_methods[] = {
    {"DeleteContents", ui_doc_delete_contents, 1},  // @pymeth DeleteContents|Call the MFC DeleteContents method.
    {"DoSave", ui_doc_do_save, 1},           // @pymeth DoSave|Save the file.  If necessary, prompt for file name.
    {"DoFileSave", ui_doc_do_file_save, 1},  // @pymeth DoFileSave|Check file attributes, and save the file.
    {"GetDocTemplate", ui_doc_get_template,
     1},  // @pymeth GetDocTemplate|Returns the <o PyCDocTemplate> for the document.
    {"GetAllViews", ui_doc_get_all_views,
     1},  // @pymeth GetAllViews|Returns a list of all views for the current document.
    {"GetFirstView", ui_doc_get_first_view,
     1},  // @pymeth GetFirstView|Returns the first view object attached to this document.
    {"GetPathName", ui_doc_get_path_name,
     1},                                // @pymeth GetPathName|Returns the full path name of the current document.
    {"GetTitle", ui_doc_get_title, 1},  // @pymeth GetTitle|Returns the title of the current document.
    {"IsModified", ui_doc_is_modified,
     1},  // @pymeth IsModified|Return a flag indicating if the document has been modified.
    {"OnChangedViewList", ui_doc_on_changed_view_list,
     1},  // @pymeth OnChangedViewList|Informs the document when a view is added or removed.
    {"OnCloseDocument", ui_doc_on_close, 1},           // @pymeth OnCloseDocument|Call the MFC OnCloseDocument handler.
    {"OnNewDocument", ui_doc_on_new, 1},               // @pymeth OnNewDocument|Call the MFC OnNewDocument handler.
    {"OnOpenDocument", ui_doc_on_open, 1},             // @pymeth OnOpenDocument|Call the MFC OnOpenDocument handler.
    {"OnSaveDocument", ui_doc_on_save, 1},             // @pymeth OnSaveDocument|Call the MFC OnSaveDocument handler.
    {"SetModifiedFlag", ui_doc_set_modified_flag, 1},  // @pymeth SetModifiedFlag|Set the "dirty" flag for the document.
    {"SaveModified", ui_doc_save_modified, 1},         // @pymeth SaveModified|Call the underlying MFC method.
    {"SetPathName", ui_doc_set_path_name, 1},          // @pymeth SetPathName|Set the full path name for the document.
    {"SetTitle", ui_doc_set_title, 1},                 // @pymeth SetTitle|Set the title of the document.
    {"UpdateAllViews", ui_doc_update_all_views,
     1},         // @pymeth UpdateAllViews|Informs each view when a document changes.
    {NULL, NULL} /* sentinel */
};
ui_type_CObject PyCDocument::type("PyCDocument",
                                  &PyCCmdTarget::type,  // @base PyCDocument|PyCCmdTarget
                                  RUNTIME_CLASS(CDocument), sizeof(PyCDocument), PYOBJ_OFFSET(PyCDocument),
                                  ui_doc_methods, GET_PY_CTOR(PyCDocument));
