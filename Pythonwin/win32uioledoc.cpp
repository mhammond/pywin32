#include "stdafxole.h"
#include "win32app.h"
#include "win32uioledoc.h"
#include "win32template.h"
#include "pywintypes.h"
//
// OLE Document Object
//
// @doc

// POSITION is pointer-sized value
#define PyObject_FromPOSITION PyWinLong_FromVoidPtr
#define POSITION_FORMATCHAR "l"

class CProtectedDocument : public CDocument {
   public:
    void SetPathName(const TCHAR *pathName) { m_strPathName = pathName; }
};

/*static*/ COleDocument *PyCOleDocument::GetDoc(PyObject *self)
{
    return (COleDocument *)GetGoodCppObject(self, &type);
}

// @pymethod <o PyCOleDocument>|win32uiole|CreateOleDocument|Creates an OLE document.
PyObject *PyCOleDocument::Create(PyObject *self, PyObject *args)
{
    TCHAR *fileName = NULL;  // default, untitled document
    PyObject *obTemplate, *obfileName = Py_None;
    // @pyparm <o PyCDocTemplate>|template||The template to be attached to this document.
    // @pyparm string|fileName|None|The filename for the document.
    if (!PyArg_ParseTuple(args, "O|O", &obTemplate, &obfileName))
        return NULL;
    if (!PyCOleDocument::is_uiobject(obTemplate, &PyCDocTemplate::type))
        RETURN_TYPE_ERR("First param must be a document template");
    if (!PyWinObject_AsTCHAR(obfileName, &fileName, TRUE))
        return NULL;

    COleDocument *pDoc = NULL;
    if (fileName) {
        CProtectedWinApp *pApp = GetProtectedApp();
        if (!pApp)
            return NULL;
        // need to look for an open doc of same name, and return that object.
        // Let MFC framework search for a filename for us.
        CDocument *pLookDoc = pApp->FindOpenDocument(fileName);
        if (pLookDoc->IsKindOf(RUNTIME_CLASS(COleDocument)))
            pDoc = (COleDocument *)pLookDoc;
    }
    // no name given, or no open document of that name
    if (pDoc == NULL) {
        CPythonDocTemplate *pMFCTemplate = PyCDocTemplate::GetTemplate(obTemplate);
        if (pMFCTemplate == NULL)
            return NULL;
        GUI_BGN_SAVE;
        pDoc = new COleDocument();
        GUI_END_SAVE;
        if (pDoc == NULL)
            RETURN_MEM_ERR("error creating document object");
        pMFCTemplate->AddDocument(pDoc);
        ASSERT_VALID(pDoc);
        ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDocument)));
        if (fileName)
            ((CProtectedDocument *)pDoc)->SetPathName(fileName);
    }
    PyWinObject_FreeTCHAR(fileName);  // ??? This leaks from error returns above ???
    return ui_assoc_object::make(PyCOleDocument::type, pDoc);
}

// @pymethod |PyCOleDocument|EnableCompoundFile|Call this function if you want to store the document using the
// compound-file format.
static PyObject *PyCOleDocument_EnableCompoundFile(PyObject *self, PyObject *args)
{
    // @pyparm int|bEnable|1|Specifies whether compound file support is enabled or disabled.
    int bEnable = 1;
    if (!PyArg_ParseTuple(args, "|i:EnableCompoundFile", &bEnable))
        return NULL;
    COleDocument *pDoc = PyCOleDocument::GetDoc(self);
    if (pDoc == NULL)
        return NULL;
    GUI_BGN_SAVE;
    pDoc->EnableCompoundFile(bEnable);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCOleClientItem>|PyCOleDocument|GetInPlaceActiveItem|Obtains the OLE item that is currently activated
// in place in the frame window containing the view identified by obWnd.
static PyObject *PyCOleDocument_GetInPlaceActiveItem(PyObject *self, PyObject *args)
{
    PyObject *obWnd;
    // @pyparm <o PyCWnd>|wnd||The window.
    if (!PyArg_ParseTuple(args, "O", &obWnd))
        return NULL;

    if (!PyCOleDocument::is_uiobject(obWnd, &PyCWnd::type))
        RETURN_TYPE_ERR("the first argument must be a Windows object");
    CWnd *pWnd = (CWnd *)PyCWnd::GetPythonGenericWnd(obWnd, &PyCWnd::type);

    COleDocument *pDoc = PyCOleDocument::GetDoc(self);
    if (pDoc == NULL)
        return NULL;
    GUI_BGN_SAVE;
    COleClientItem *pRet = pDoc->GetInPlaceActiveItem(pWnd);
    GUI_END_SAVE;
    if (pRet == NULL)
        RETURN_NONE;
    return ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
}

// @pymethod POSITION|PyCOleDocument|GetStartPosition|Obtains the position of the first item in the document.
static PyObject *PyCOleDocument_GetStartPosition(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, "GetStartPosition");
    COleDocument *pDoc = PyCOleDocument::GetDoc(self);
    if (pDoc == NULL)
        return NULL;
    GUI_BGN_SAVE;
    POSITION pos = pDoc->GetStartPosition();
    GUI_END_SAVE;
    return PyWinLong_FromVoidPtr((void *)pos);
}

// @pymethod (POSITION, <o PyCOleClientItem>)|PyCOleDocument|GetNextItem|Call this function repeatedly to access each of
// the items in your document.
static PyObject *PyCOleDocument_GetNextItem(PyObject *self, PyObject *args)
{
    POSITION position;
    // @pyparm POSITION|pos||The position to iterate from.
    if (!PyArg_ParseTuple(args, POSITION_FORMATCHAR, &position))
        return NULL;
    COleDocument *pDoc = PyCOleDocument::GetDoc(self);
    if (pDoc == NULL)
        return NULL;
    GUI_BGN_SAVE;
    CDocItem *pRet = pDoc->GetNextItem(position);
    GUI_END_SAVE;
    PyObject *obDocItem = ui_assoc_object::make(UITypeFromCObject(pRet), pRet)->GetGoodRet();
    if (obDocItem == NULL)
        return NULL;
    PyObject *ret = Py_BuildValue("NN", obDocItem, PyObject_FromPOSITION(position));
    return ret;
}

///////////////////////////////////////
//
// OLE Document Methods
//
///////////////////////////////////////
// @object PyCOleDocument|An OLE document class.  Encapsulates an MFC <c COleDocument> class
static struct PyMethodDef PyCOleDocument_methods[] = {
    {"EnableCompoundFile", PyCOleDocument_EnableCompoundFile,
     1},  // @pymeth EnableCompoundFile|Call this function if you want to store the document using the compound-file
          // format
    {"GetStartPosition", PyCOleDocument_GetStartPosition,
     1},  // @pymeth GetStartPosition|Obtains the position of the first item in the document.
    {"GetNextItem", PyCOleDocument_GetNextItem,
     1},  // @pymeth GetNextItem|Call this function repeatedly to access each of the items in your document.
    {"GetInPlaceActiveItem", PyCOleDocument_GetInPlaceActiveItem,
     1},  // @pymeth GetInPlaceActiveItem|Obtains the OLE item that is currently activated in place in the frame window
          // containing the view identified by obWnd.
    {NULL, NULL} /* sentinel */
};
ui_type_CObject PyCOleDocument::type("PyCOleDocument", &PyCDocument::type, RUNTIME_CLASS(COleDocument),
                                     sizeof(PyCOleDocument), PYOBJ_OFFSET(PyCOleDocument), PyCOleDocument_methods,
                                     GET_PY_CTOR(PyCOleDocument));
