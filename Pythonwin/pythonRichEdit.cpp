// pythonRichEdit.cpp : implementation file
//

#include "stdafx.h"
#include "pythonwin.h"
#include "win32ui.h"
#include "pythonRichEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonRichEditView

CPythonRichEditView::CPythonRichEditView() {}

CPythonRichEditView::~CPythonRichEditView() {}

BOOL CPythonRichEditView::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
    // yield to Python first
    if (Python_OnCmdMsg(this, nID, nCode, pExtra, pHandlerInfo))
        return TRUE;
    else
        return CRichEditView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CPythonRichEditView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    // yield to Python first
    if (Python_OnNotify(this, wParam, lParam, pResult))
        return TRUE;
    else
        return CRichEditView::OnNotify(wParam, lParam, pResult);
}

#ifdef PYWIN_WITH_WINDOWPROC
LRESULT CPythonRichEditView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // @pyvirtual int|PyCRichEditView|WindowProc|Default message handler.
    // @xref <om PyCRichEditView.WindowProc>
    LRESULT res;
    CVirtualHelper helper("WindowProc", this);
    if (!helper.HaveHandler() || !helper.call(message, wParam, lParam) || !helper.retval(res)) {
        try {
            return CRichEditView::WindowProc(message, wParam, lParam);
        }
        catch (...) {
            TRACE("RichEditView WindowProc caused access violation!");
            res = 0;
        }
    }
    return res;
}
#endif  // PYWIN_WITH_WINDOWPROC

void CPythonRichEditView::OnInitialUpdate()
{
    // @pyvirtual tuple|PyCRichEditView|OnInitialUpdate|Called before the first update for a view.
    // @xref <om PyCRichEditView.OnInitialUpdate>
    CVirtualHelper helper("OnInitialUpdate", this);
    if (helper.HaveHandler())
        helper.call();
    else
        CRichEditView::OnInitialUpdate();
}

#ifdef _DEBUG
void CPythonRichEditView::Dump(CDumpContext &dc) const
{
    try {
        CRichEditView::Dump(dc);
    }
    catch (...) {
        dc << "***** CRichEditView::Dump caused win32 exception";
    }
}
#endif

BEGIN_MESSAGE_MAP(CPythonRichEditView, CRichEditView)
//{{AFX_MSG_MAP(CPythonRichEditView)
// NOTE - the ClassWizard will add and remove mapping macros here.
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPythonRichEditView message handlers
