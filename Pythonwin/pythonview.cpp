// Pythonview.cpp - implementation of a CEditView, especially set up for
// python support.
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"
#include "pythonwin.h"
#include "pythonview.h"
#include "win32ui.h"
#include "win32dc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPythonListView, CListView)
IMPLEMENT_DYNAMIC(CPythonTreeView, CTreeView)
IMPLEMENT_DYNAMIC(CPythonView, CScrollView);
IMPLEMENT_DYNAMIC(CPythonEditView, CEditView);
IMPLEMENT_DYNAMIC(CPythonFormView, CFormView);
IMPLEMENT_DYNAMIC(CPythonCtrlView, CCtrlView);

/////////////////////////////////////////////////////////////////////////////
// CPythonView

void CPythonViewImpl::OnPrepareDC (CDC *pDC, CPrintInfo *pInfo)
{
	// @pyvirtual |PyCScrollView|OnPrepareDC|Called to prepare the device context for a view.
	// @xref <om PyCView.OnPrepareDC>
	if (m_nMapMode == 0) {
		// base class will ASSERT
		CEnterLeavePython _celp;
		PyErr_SetString(ui_module_error, "Must call SetScrollSizes() or SetScaleToFitSize() before painting scroll view.");
		gui_print_error();
		return;
	}

	CVirtualHelper helper ("OnPrepareDC", this);
	helper.call (pDC, pInfo);
	CScrollView::OnPrepareDC (pDC, pInfo);
	// @pyparm <o PyCDC>|dc||The DC object.
}

/////////////////////////////////////////////////////////////////////////////
// CPythonView message handlers
/*
BOOL
CPythonView::SetDynamicScrollBars (BOOL dynamic)
{
  BOOL old = m_bInsideUpdate;

  // Prevent MFC from hiding/showing scrollbars by setting recursive
  // protection variable.
  if (dynamic)
    m_bInsideUpdate = FALSE;
  else
    m_bInsideUpdate = TRUE;

  return (old);
}
*/

//////////////////////////////////////////////////////////////////////////////////
//
// CPythonListView
//
CPythonListViewImpl::CPythonListViewImpl()
{
}
CPythonListViewImpl::~CPythonListViewImpl()
{
}

void CPythonListViewImpl::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	CVirtualHelper helper("DrawItem", this);
	PyObject *obData = PyWin_GetPythonObjectFromLong(lpDIS->itemData);
	if (obData==NULL) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert the Python object");
		gui_print_error();
		obData = Py_None;
	}

	// Get the MFC device context
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);
	PyObject *obDC = ui_dc_object::make(ui_dc_object::type, pDC);
	if (obDC==NULL) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert the DC object");
		gui_print_error();
		obDC = Py_None;
	}

	PyObject *args = Py_BuildValue("iiiiiiO(iiii)O",
			lpDIS->CtlType, lpDIS->CtlID, lpDIS->itemID,
			lpDIS->itemAction, lpDIS->itemState, lpDIS->hwndItem,
			obDC, 
			lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom,
			obData);
	ASSERT(args);
	if (!args) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert args - handler not called.");
		return; // not too much we can do
	}
	// make the call.
	helper.call_args(args);
	// Cleanup.
	Py_DECREF(args);
	// The DC is no longer valid.
	Python_delete_assoc(pDC);
}


//////////////////////////////////////////////////////////////////////////////////
//
// CPythonTreeView
//

CPythonTreeViewImpl::CPythonTreeViewImpl()
{
}
CPythonTreeViewImpl::~CPythonTreeViewImpl()
{
}

void CPythonTreeViewImpl::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	CVirtualHelper helper("DrawItem", this);
	PyObject *obData = PyWin_GetPythonObjectFromLong(lpDIS->itemData);
	if (obData==NULL) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert the Python object");
		gui_print_error();
		obData = Py_None;
	}

	// Get the MFC device context
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);
	PyObject *obDC = ui_dc_object::make(ui_dc_object::type, pDC);
	if (obDC==NULL) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert the DC object");
		gui_print_error();
		obDC = Py_None;
	}

	PyObject *args = Py_BuildValue("iiiiiiO(iiii)O",
			lpDIS->CtlType, lpDIS->CtlID, lpDIS->itemID,
			lpDIS->itemAction, lpDIS->itemState, lpDIS->hwndItem,
			obDC, 
			lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom,
			obData);
	ASSERT(args);
	if (!args) {
		gui_print_error();
		PyErr_SetString(ui_module_error, "DrawItem could not convert args - handler not called.");
		return; // not too much we can do
	}
	// make the call.
	helper.call_args(args);
	// Cleanup.
	Py_DECREF(args);
	// The DC is no longer valid.
	Python_delete_assoc(pDC);
}

