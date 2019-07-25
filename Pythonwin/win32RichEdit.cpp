/* win32RichEdit : implementation file

    Created March 1996, Mark Hammond (MHammond@skippinet.com.au)

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
#include "win32control.h"
#include "win32RichEdit.h"

typedef CPythonViewFramework<CRichEditView> CPythonRichEditView;
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

class CProtectedRichEditView : public CRichEditView {
   public:
    void WrapChanged(void) { CRichEditView::WrapChanged(); }
    void SetDocument(CDocument *pDoc)
    {
        if (pDoc)
            ASSERT_VALID(pDoc);
        m_pDocument = pDoc;
    }
};

CRichEditView *GetRichEditViewPtr(PyObject *self)
{
    // need to only rtti check on CView, as CPythonEditView is not derived from CPythonView.
    return (CRichEditView *)PyCWnd::GetPythonGenericWnd(self, &PyCRichEditView::type);
}

/////////////////////////////////////////////////////////////////////
//
// Rich Edit Document
//
//////////////////////////////////////////////////////////////////////
// @pymethod |PyCRichEditDoc|OnCloseDocument|Call the MFC OnCloseDocument handler.
// This routine is provided so a document object which overrides this method
// can call the original MFC version if required.
static PyObject *ui_re_doc_on_close(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, OnCloseDocument);
    CRichEditDoc *pDoc;
    if (!(pDoc = (CRichEditDoc *)PyCRichEditDoc::GetDoc(self)))
        return NULL;
    // @xref <vm PyCDocument.OnCloseDocument>
    GUI_BGN_SAVE;
    pDoc->CRichEditDoc::OnCloseDocument();  // @pyseemfc CRichEditDoc|OnCloseDocument
    GUI_END_SAVE;
    RETURN_NONE;
}

extern PyObject *ui_doc_set_path_name(PyObject *self, PyObject *args);

static PyObject *ui_re_doc_set_path_name(PyObject *self, PyObject *args)
{
    CRichEditDoc *pDoc;
    if (!(pDoc = (CRichEditDoc *)PyCDocument::GetDoc(self)))
        return NULL;
    if (pDoc->GetView() == NULL)
        RETURN_ERR("There is no view");
    return ui_doc_set_path_name(self, args);
}

// @object PyCRichEditDoc|A class which implements a CRichEditView object.  Derived from <o PyCDocument>.
static struct PyMethodDef PyCRichEditDoc_methods[] = {
    {"OnCloseDocument", ui_re_doc_on_close, 1},  // @pymeth OnCloseDocument|Call the MFC OnCloseDocument handler.
    {"SetPathName", ui_re_doc_set_path_name, 1},
    {NULL, NULL}};

ui_type_CObject PyCRichEditDoc::type("PyCRichEditDoc", &PyCDocument::type, RUNTIME_CLASS(CRichEditDoc),
                                     sizeof(PyCRichEditDoc), PYOBJ_OFFSET(PyCRichEditDoc), PyCRichEditDoc_methods,
                                     GET_PY_CTOR(PyCRichEditDoc));

/////////////////////////////////////////////////////////////////////
//
// Rich Edit View object
//
//////////////////////////////////////////////////////////////////////
// @pymethod <o PyCRichEditView>|win32ui|CreateRichEditView|Creates a PyRichEditView object.
PyObject *PyCRichEditView::create(PyObject *self, PyObject *args)
{
    PyObject *doc = Py_None;
    // @pyparm <o PyCDocument>|doc|None|The document to use with the view, or None for NULL.
    if (!PyArg_ParseTuple(args, "|O:CreateEditView", &doc))
        return NULL;
    CDocument *pDoc = NULL;
    if (doc != Py_None) {
        if (!ui_base_class::is_uiobject(doc, &PyCDocument::type))
            RETURN_TYPE_ERR("Argument must be a PyCDocument");
        pDoc = PyCDocument::GetDoc(doc);
        if (!pDoc)
            return NULL;
    }
    CPythonRichEditView *pView = new CPythonRichEditView();
    ((CProtectedRichEditView *)pView)->SetDocument(pDoc);
    return ui_assoc_object::make(PyCRichEditView::type, pView);
}

///////////////////////////////////////
//
// Rich Edit View Methods
//
// inherited from CtlView
//
///////////////////////////////////////
// @pymethod <o PyCRichEditCtrl>|PyCRichEditView|GetRichEditCtrl|Returns the underlying rich edit control object.
static PyObject *PyCRichEditView_get_rich_edit_ctrl(PyObject *self, PyObject *args)
{
    CRichEditView *pView = GetRichEditViewPtr(self);
    if (!pView)
        return NULL;
    CRichEditCtrl &ed = pView->GetRichEditCtrl();
    return ui_assoc_object::make(UITypeFromCObject(&ed), &ed)->GetGoodRet();
}

// @pymethod None|PyCRichEditView|SetWordWrap|Sets the wordwrap state for the control.
static PyObject *PyCRichEditView_set_word_wrap(PyObject *self, PyObject *args)
{
    CRichEditView *pView = GetRichEditViewPtr(self);
    if (!pView)
        return NULL;
    int wrap;
    // @pyparm int|wordWrap||The new word-wrap state.
    if (!PyArg_ParseTuple(args, "i:SetWordWrap", &wrap))
        return NULL;
    // @pyseemfc CRichEditCtrl|m_nWordWrap
    pView->m_nWordWrap = wrap;
    RETURN_NONE;
}

// @pymethod None|PyCRichEditView|WrapChanged|Calls the underlying WrapChanged method.
static PyObject *PyCRichEditView_wrap_changed(PyObject *self, PyObject *args)
{
    CProtectedRichEditView *pView = (CProtectedRichEditView *)GetRichEditViewPtr(self);
    if (!pView)
        return NULL;
    if (!PyArg_ParseTuple(args, ":WrapChanged"))
        return NULL;
    GUI_BGN_SAVE;
    pView->WrapChanged();
    GUI_END_SAVE;
    // @pyseemfc CRichEditCtrl|WrapChanged
    RETURN_NONE;
}

// @pymethod None|PyCRichEditView|SaveTextFile|Saves the contents of the control as a test file
// @comm Theere is no equivilent MFC method.  This is implemented in this module for performance reasons.
static PyObject *PyCRichEditView_save_text_file(PyObject *self, PyObject *args)
{
    // Ported from Python code!
    TCHAR *fileName;
    PyObject *obfileName;
    if (!PyArg_ParseTuple(args, "O:SaveTextFile",
                          &obfileName))  // @pyparm str|FileName||Name of file to save
        return NULL;
    if (!PyWinObject_AsTCHAR(obfileName, &fileName, FALSE))
        return NULL;
    // Changing mode here allows us to save Unix or PC
    FILE *f = _tfopen(fileName, _T("wb"));
    if (f == NULL) {
        PyErr_SetFromErrno(PyExc_IOError);
        PyWinObject_FreeTCHAR(fileName);
        return NULL;
    }
    CProtectedRichEditView *pView = (CProtectedRichEditView *)GetRichEditViewPtr(self);
    CRichEditCtrl &ctrl = pView->GetRichEditCtrl();
    GUI_BGN_SAVE;
    // The RTF control always has an "extra" \n
    long lineCount = ctrl.GetLineCount();
    for (long i = 0; i < lineCount; i++) {
        int size = 1024;
        CString csBuffer;  // use dynamic mem for buffer
        TCHAR *buf;
        int bytesCopied;
        // loop if buffer too small, increasing each time.
        while (size < 0x7FFF)  // reasonable line size max? - maxuint on 16 bit.
        {
            buf = csBuffer.GetBufferSetLength(size);
            if (buf == NULL) {
                GUI_BLOCK_THREADS;
                RETURN_ERR("Out of memory getting control line value");
            }
            bytesCopied = ctrl.GetLine(i, buf, size);
            if (bytesCopied != size)  // ok - get out.
                break;
            // buffer too small
            size += size;  // try doubling!
        }
        if (bytesCopied == size)  // hit max.
            --bytesCopied;        // so NULL doesnt overshoot.
        buf[bytesCopied] = 0;
        if (i < lineCount - 1)
            fwrite(buf, sizeof(TCHAR), bytesCopied, f);
        else
            fwrite(buf, sizeof(TCHAR), bytesCopied - 2, f);
    }
    fclose(f);
    PyWinObject_FreeTCHAR(fileName);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyCRichEditView|A class which implementes a CRichEditView.  Derived from <o PyCRichEditView> and <o
// PyCRichEditCtrl>.
static struct PyMethodDef PyCRichEditView_methods[] = {
    {"GetRichEditCtrl", PyCRichEditView_get_rich_edit_ctrl,
     1},  // @pymeth GetRichEditCtrl|Returns the underlying rich edit control object.
    {"SetWordWrap", PyCRichEditView_set_word_wrap, 1},  // @pymeth SetWordWrap|Sets the wordwrap state for the control.
    {"WrapChanged", PyCRichEditView_wrap_changed, 1},   // @pymeth WrapChanged|Calls the underlying WrapChanged method.
    {"SaveTextFile", PyCRichEditView_save_text_file, 1},  // @pymeth SaveTextFile|Saves the control to a text file
    {NULL, NULL}};

PyCCtrlView_Type PyCRichEditView::type("PyCRichEditView", &PyCCtrlView::type, &PyCRichEditCtrl::type,
                                       RUNTIME_CLASS(CRichEditView), sizeof(PyCRichEditView),
                                       PYOBJ_OFFSET(PyCRichEditView), PyCRichEditView_methods,
                                       GET_PY_CTOR(PyCRichEditView));
