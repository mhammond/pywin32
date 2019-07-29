/* win32splitter : implementation file


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
#include "win32control.h"
#include "win32splitter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////
//
// splitter object
//

PyCSplitterWnd::PyCSplitterWnd() {}
PyCSplitterWnd::~PyCSplitterWnd() {}

// @pymethod <o PyCSplitterWnd>|win32ui|CreateSplitter|Creates a splitter window object.
PyObject *PyCSplitterWnd::create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, CreateSplitter);
    return ui_assoc_object::make(PyCSplitterWnd::type, new CPythonSplitter);
}

// identical to ui_window, except no ASSERT_VALID checking if not initialised.
/*static*/ CPythonSplitter *PyCSplitterWnd::GetSplitterObject(PyObject *self)
{
    return (CPythonSplitter *)GetGoodCppObject(self, &type);
}

/////////////////////////////////////////////////////////////////////
//
// splitter methods
//
CWnd *GetWnd(PyObject *self) { return (CWnd *)PyCWnd::GetPythonGenericWnd(self); }
CView *GetView(PyObject *self) { return (CView *)PyCWnd::GetPythonGenericWnd(self); }

// @pymethod |PyCSplitterWnd|CreateStatic|Creates a static splitter window.
// @comm A static splitter window is a splitter where the number of panes are
// fixed at window creation time.  Currently this is the only splitter window
// supported by win32ui.
PyObject *ui_splitter_create_static(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int rows, cols;
    DWORD dwStyle = WS_CHILD | WS_VISIBLE;
    UINT nID = AFX_IDW_PANE_FIRST;
    PyObject *ob;
    if (!PyArg_ParseTuple(
            args, "Oii|ii",
            &ob,       // @pyparm <o PyCFrameWnd> or <o PyCSplitter>|parent||The parent window.
            &rows,     // @pyparm int|rows||The number of rows in the splitter.
            &cols,     // @pyparm int|cols||The number of columns in the splitter.
            &dwStyle,  // @pyparm int|style|WS_CHILD \| WS_VISIBLE|Specifies the window style
            &nID))     // @pyparm int|id|AFX_IDW_PANE_FIRST|The child window ID of the window. The ID can be
                       // AFX_IDW_PANE_FIRST unless the splitter window is nested inside another splitter window.
        return NULL;
    if (!(ui_base_class::is_uiobject(ob, &PyCFrameWnd::type) || ui_base_class::is_uiobject(ob, &PyCSplitterWnd::type)))
        RETURN_TYPE_ERR("First argument must be a PyFrameWnd or PyCSplitter.");

    // these will cause assert failures in MFC.
    if (!(rows >= 1 && rows <= 16) || !(cols >= 1 && cols <= 16) || !(cols > 1 || rows > 1))
        RETURN_TYPE_ERR("Row and column argument out of range");

    CWnd *pParent = GetWnd(ob);
    if (pParent == NULL)
        return NULL;
    int rc;
    GUI_BGN_SAVE;
    rc = wnd->CreateStatic(pParent, rows, cols, dwStyle, nID);  // @pyseemfc CSplitterWnd|CreateStatic
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("CSplitterWnd::CreateStatic failed");
    RETURN_NONE;
}

// @pymethod |PyCSplitterWnd|CreateView|Creates a view in a splitter window
PyObject *ui_splitter_create_view(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int row, col, width, height;
    PyObject *ob;
    if (!PyArg_ParseTuple(args, "Oii(ii)",
                          &ob,     // @pyparm <o PyCView>|view||The view to place in the splitter pane.
                          &row,    // @pyparm int|row||The row in the splitter to place the view.
                          &col,    // @pyparm int|col||The column in the splitter to place the view.
                          &width,  // @pyparm (int, int)|width, height||The initial size of the new view.
                          &height))
        return NULL;
    if (!ui_base_class::is_uiobject(ob, &PyCView::type))
        RETURN_TYPE_ERR("Argument must be a PyCView or child");
    CView *pView = GetView(ob);
    if (pView == NULL)
        return NULL;
    CCreateContext context;
    context.m_pLastView = pView;
    context.m_pCurrentDoc = pView->GetDocument();
    if (!context.m_pCurrentDoc)
        RETURN_ERR("There is no document attached to the view");

    extern void PyWin_SetViewDocument(CView * pView, CDocument * pDoc);
    PyWin_SetViewDocument(pView, NULL);

    // no thread state - CreateView is implemented by us and manages this.
    if (!wnd->CreateView(row, col, NULL, CSize(width, height), &context))  // @pyseemfc CSplitterWnd|CreateView
        return NULL;                                                       // exception set.
    RETURN_NONE;
}

// @pymethod int|PyCSplitterWnd|DoKeyboardSplit|
PyObject *ui_splitter_do_kb_split(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    CHECK_NO_ARGS2(args, DoKeyboardSplit);
    BOOL rc;
    GUI_BGN_SAVE;
    rc = wnd->DoKeyboardSplit();
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod <o PyCWnd>|PyCSplitterWnd|GetPane|Returns the <o PyCView> associated with the specified pane.
// @comm Theoretically the return value can be a <o PyCWnd> object, but currently it
// will always be a <o PyCView> or derived object.
PyObject *ui_splitter_get_pane(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int row, col;
    if (!PyArg_ParseTuple(args, "ii",
                          &row,   // @pyparm int|row||The row in the splitter.
                          &col))  // @pyparm int|col||The column in the splitter.
        return NULL;
    CWnd *pWnd;
    GUI_BGN_SAVE;
    pWnd = wnd->GetPane(row, col);
    GUI_END_SAVE;
    if (!pWnd)
        return NULL;
    return ui_assoc_object::make(UITypeFromCObject(pWnd), pWnd)->GetGoodRet();
}

// @pymethod |PyCSplitterWnd|SetRowInfo|Sets a new minimum height and ideal height for a row.
PyObject *ui_splitter_set_row_info(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int row, min, ideal;
    if (!PyArg_ParseTuple(
            args, "iii:SetRowInfo",
            &row,    // @pyparm int|row||The row in the splitter.
            &ideal,  // @pyparm int|ideal||Specifies an ideal height for the splitter window row in pixels.
            &min))   // @pyparm int|min||Specifies a minimum height for the splitter window row in pixels.
        return NULL;

    GUI_BGN_SAVE;
    wnd->SetRowInfo(row, ideal, min);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCSplitterWnd|SetColumnInfo|Sets a new minimum height and ideal height for a column
PyObject *ui_splitter_set_column_info(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int row, min, ideal;
    if (!PyArg_ParseTuple(
            args, "iii:SetColumnInfo",
            &row,    // @pyparm int|column||The column in the splitter.
            &ideal,  // @pyparm int|ideal||Specifies an ideal height for the splitter window column in pixels.
            &min))   // @pyparm int|min||Specifies a minimum height for the splitter window column in pixels.
        return NULL;

    GUI_BGN_SAVE;
    wnd->SetColumnInfo(row, ideal, min);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCSplitterWnd|IdFromRowCol|Gets the child window ID for the specified child.
PyObject *ui_splitter_id_from_row_col(PyObject *self, PyObject *args)
{
    CPythonSplitter *wnd = PyCSplitterWnd::GetSplitterObject(self);
    if (!wnd)
        return NULL;
    int row, col;
    if (!PyArg_ParseTuple(args, "ii:IdFromRowCol",
                          &row,   // @pyparm int|row||The row in the splitter.
                          &col))  // @pyparm int|col||The col in the splitter
        return NULL;

    int rc;
    GUI_BGN_SAVE;
    rc = wnd->IdFromRowCol(row, col);
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @object PyCSplitterWnd|A class which encapsulates an MFC <o CSplitterWnd>. Derived from a <o PyCWnd> object.
static struct PyMethodDef ui_splitter_window_methods[] = {
    {"GetPane", ui_splitter_get_pane,
     1},  // @pymeth GetPane|Returns the <o PyCWnd> object associated with a splitter window pane.
    {"CreatePane", ui_splitter_create_view, 1},
    {"CreateView", ui_splitter_create_view, 1},      // @pymeth CreateView|Creates a view in a splitter window
    {"CreateStatic", ui_splitter_create_static, 1},  // @pymeth CreateStatic|Creates a static splitter window.
    {"SetColumnInfo", ui_splitter_set_column_info,
     1},  // @pymeth SetColumnInfo|Sets a new minimum height and ideal height for a column
    {"SetRowInfo", ui_splitter_set_row_info,
     1},  // @pymeth SetRowInfo|Sets a new minimum height and ideal height for a row.
    {"IdFromRowCol", ui_splitter_id_from_row_col,
     1},  // @pymeth IdFromRowCol|Gets the child window ID for the specified child.
    {"DoKeyboardSplit", ui_splitter_do_kb_split, 1},  // @pymeth DoKeyboardSplit|
    {NULL, NULL}};

ui_type_CObject PyCSplitterWnd::type("PyCSplitterWnd", &PyCWnd::type, RUNTIME_CLASS(CSplitterWnd),
                                     sizeof(PyCSplitterWnd), PYOBJ_OFFSET(PyCSplitterWnd), ui_splitter_window_methods,
                                     GET_PY_CTOR(PyCSplitterWnd));

/////////////////////////////////////////////////////////////////////
//
// Python splitter
//
IMPLEMENT_DYNAMIC(CPythonSplitter, CSplitterWnd);
CPythonSplitter::CPythonSplitter() { bHaveAllChildren = FALSE; }
BOOL CPythonSplitter::CreateView(int row, int col, CRuntimeClass *pViewClass, SIZE sizeInit, CCreateContext *pContext)
{
    // NOTE NOTE NOTE
    // This is basically cloned from MFC CSplitterWnd::CreateView (winsplit.cpp)
    ASSERT_VALID(this);
    if (!(row >= 0 && row < m_nRows)) {
        PyErr_Format(PyExc_IndexError, "Row number %d is invalid - must be from 0-%d", row, m_nRows - 1);
        return FALSE;
    }
    if (!(col >= 0 && col < m_nCols)) {
        PyErr_Format(PyExc_IndexError, "Column number %d is invalid - must be from 0-%d", col, m_nCols - 1);
        return FALSE;
    }
    //	ASSERT(pViewClass != NULL);
    CWnd *child;
    {
        GUI_BGN_SAVE;
        child = GetDlgItem(IdFromRowCol(row, col));
        GUI_END_SAVE;
    }
    if (child != NULL) {
        PyErr_SetString(ui_module_error, "CreateView - pane already exists");
        return FALSE;
    }
    // set the initial size for that pane
    m_pColInfo[col].nIdealSize = sizeInit.cx;
    m_pRowInfo[row].nIdealSize = sizeInit.cy;

    if (pContext == NULL || pContext->m_pLastView == NULL) {
        PyErr_SetString(ui_module_error, "CreateView - Internal error - no valid context");
        return FALSE;
    }
    CWnd *pWnd = pContext->m_pLastView;
    ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CWnd)));
    ASSERT(pWnd->m_hWnd == NULL);  // not yet created

    DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
    //	if (afxData.bWin4)
    dwStyle &= ~WS_BORDER;

    // Create with the right size (wrong position)
    CRect rect(CPoint(0, 0), sizeInit);
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pWnd->Create(NULL, NULL, dwStyle, rect, this, IdFromRowCol(row, col), pContext);
    GUI_END_SAVE;
    if (!ok) {
        PyErr_SetString(ui_module_error, "CreateView: couldn't create client pane for splitter");
        // pWnd will be cleaned up by PostNcDestroy
        return FALSE;
    }
    //	ASSERT((int)_AfxGetDlgCtrlID(pWnd->m_hWnd) == IdFromRowCol(row, col));

    // send initial notification message
    //	if (bSendInitialUpdate)
    //		pWnd->SendMessage(WM_INITIALUPDATE);

    return TRUE;
}
void CPythonSplitter::PostNcDestroy()
{
    CSplitterWnd::PostNcDestroy();
    delete this;  // clean up the pointer I created
}

BEGIN_MESSAGE_MAP(CPythonSplitter, CSplitterWnd)
//{{AFX_MSG_MAP(CPythonSplitter)
ON_WM_SIZE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPythonSplitter::OnSize(UINT nType, int cx, int cy)
{
    // MFC will die, and this is not good for Python!
    if (!bHaveAllChildren) {
        // check them
        BOOL bFailed = FALSE;
        for (int col = 0; !bFailed && col < m_nCols; col++)
            for (int row = 0; !bFailed && row < m_nRows; row++) bFailed = (GetDlgItem(IdFromRowCol(row, col)) == NULL);
        bHaveAllChildren = !bFailed;
    }
    if (bHaveAllChildren)
        CSplitterWnd::OnSize(nType, cx, cy);
    else
        OutputDebugString(_T("Warning - Ignoring OnSize for splitter, due to missing children\n"));
}