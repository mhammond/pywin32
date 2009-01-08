// Contributed by kk@epsilon.com.gr
//
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"

#include "win32win.h"
#include "win32control.h"

#include "win32gdi.h"
#include "win32bitmap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//
// PyCToolTipCtrl
//
static CToolTipCtrl *GetToolTipCtrl(PyObject *self)
{
	return (CToolTipCtrl *)PyCWnd::GetPythonGenericWnd(self);
}
PyCToolTipCtrl::PyCToolTipCtrl()
{
}
PyCToolTipCtrl::~PyCToolTipCtrl()
{
}

// @pymethod <o PyCToolTipCtrl>|win32ui|CreateToolTipCtrl|Creates a progress control object. <om PyToolTipCtrl.Create> creates the actual control.
PyObject *
PyCToolTipCtrl_create(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CToolTipCtrl *pTTC = new CToolTipCtrl();
	return ui_assoc_object::make( PyCToolTipCtrl::type, pTTC );
}

// @pymethod |PyCToolTipCtrl|CreateWindow|Creates the actual control.
static PyObject *
PyCToolTipCtrl_create_window(PyObject *self, PyObject *args)
{
	int style;
	PyObject *obParent;
	if (!PyArg_ParseTuple(args, "Oi:CreateWindow", 
			   &obParent, // @pyparm <o PyCWnd>|parent||The parent window of the control.
			   &style)) // @pyparm int|style||The style for the control.
		return NULL;

	if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
		RETURN_TYPE_ERR("parent argument must be a window object");
	CWnd *pParent = GetWndPtr( obParent );
	if (pParent==NULL)
		return NULL;
	CToolTipCtrl *pTTC = GetToolTipCtrl(self);
	if (!pTTC)
		return NULL;

	BOOL ok;
	GUI_BGN_SAVE;
	ok = pTTC->Create(pParent,style);
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("CToolTipCtrl::Create");
	RETURN_NONE;
}


// @pymethod |PyCToolTipCtrl|UpdateTipText|Update the tool tip text for a control's tools
static PyObject *
PyCToolTipCtrl_update_tip_text(PyObject *self, PyObject *args)
	{
	PyObject *obWnd;
	TCHAR *pszText;
	PyObject *obText;
	UINT nIDTool;
	if (!PyArg_ParseTuple(args, "OOi:UpdateTipText", 
			   &obText,// @pyparm string|text||The text for the tool.
			   &obWnd, // @pyparm <o PyCWnd>|wnd||The window of the tool.
			   &nIDTool// @pyparm int|id||The id of the tool
			   )) 
		return NULL;

	CWnd *pWndToolOwner = NULL;
	if (obWnd != Py_None) 
		{
		if (!ui_base_class::is_uiobject(obWnd,&PyCWnd::type))
			RETURN_TYPE_ERR("wnd argument must be a window object");
		pWndToolOwner = GetWndPtr(obWnd);
		if (pWndToolOwner==NULL)
			RETURN_TYPE_ERR("The window is not a valid PyCWnd");
		}

	CToolTipCtrl *pTTC = GetToolTipCtrl(self);
	if (!pTTC)return NULL;
	if (!PyWinObject_AsTCHAR(obText, &pszText, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	pTTC->UpdateTipText(pszText,pWndToolOwner,nIDTool);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(pszText);
	RETURN_NONE;
	}



// @pymethod |PyCToolTipCtrl|AddTool|Adds a tool to tooltip control.
static PyObject *
PyCToolTipCtrl_add_tool(PyObject *self, PyObject *args)
	{
	PyObject *obWnd,*obRect;
	TCHAR *pszText;
	PyObject *obText;
	UINT nIDTool;
	if (!PyArg_ParseTuple(args, "OOOi:CreateWindow", 
			   &obWnd, // @pyparm <o PyCWnd>|wnd||The window of the tool.
			   &obText,// @pyparm string|text||The text for the tool.
			   &obRect, // @pyparm int, int, int, int|rect|None|The default rectangle
			   &nIDTool// @pyparm int|id||The id of the tool
			   )) 
		return NULL;

	CWnd *pWnd = NULL;
	if (obWnd != Py_None) 
		{
		if (!ui_base_class::is_uiobject(obWnd,&PyCWnd::type))
			RETURN_TYPE_ERR("wnd argument must be a window object");
		pWnd = GetWndPtr(obWnd);
		if (pWnd==NULL)
			RETURN_TYPE_ERR("The window is not a valid PyCWnd");
		}

	RECT rect;
	RECT *pRectTool=NULL;
	if (obRect != Py_None) 
		{
		if (!PyArg_ParseTuple(obRect, "iiii", &rect.left,  &rect.top,  &rect.right,&rect.bottom)) 
			{
			PyErr_Clear();
			RETURN_TYPE_ERR("Rect must be None or a tuple of (iiii)");
			}
		pRectTool=&rect;
		}


	CToolTipCtrl *pTTC = GetToolTipCtrl(self);
	if (!pTTC)return NULL;
	if (!PyWinObject_AsTCHAR(obText, &pszText, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	BOOL ok=pTTC->AddTool(pWnd,pszText,pRectTool,nIDTool);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(pszText);
	if (!ok)
		RETURN_ERR("CToolTipCtrl::AddTool");
	RETURN_NONE;
	}


// @pymethod int|PyCToolTipCtrl|SetMaxTipWidth|
static PyObject *
PyCToolTipCtrl_set_max_tip_width(PyObject *self, PyObject *args)
	{
	int width;
	if (!PyArg_ParseTuple(args, "i:SetMaxTipWidth", 
			   &width)) // @pyparm int|width||The new width
		return NULL;

	CToolTipCtrl *pTTC = GetToolTipCtrl(self);
	if (!pTTC)return NULL;

	GUI_BGN_SAVE;
	int rc = pTTC->SetMaxTipWidth(width);
	GUI_END_SAVE;
	return PyInt_FromLong(rc);
}



// @object PyCToolTipCtrl|A windows tooltip control.  Encapsulates an MFC <c CToolTipCtrl> class.  Derived from <o PyCControl>.
static struct PyMethodDef PyCToolTipCtrl_methods[] = {
	{"CreateWindow",    PyCToolTipCtrl_create_window,1}, // @pymeth CreateWindow|Creates the window for a new progress bar object.
	{"UpdateTipText", PyCToolTipCtrl_update_tip_text, 1}, // @pymeth UpdateTipText|Update the tool tip text for a control's tools
	{"AddTool", PyCToolTipCtrl_add_tool, 1}, // @pymeth AddTool|Adds a tool to tooltip control.
	{"SetMaxTipWidth", PyCToolTipCtrl_set_max_tip_width, 1}, // @pymeth SetMaxTipWidth|
	{NULL,				NULL}
};

ui_type_CObject PyCToolTipCtrl::type("PyCToolTipCtrl", 
				       &ui_control_object::type, 
				       RUNTIME_CLASS(CToolTipCtrl), 
				       sizeof(PyCToolTipCtrl), 
				       PYOBJ_OFFSET(PyCToolTipCtrl), 
				       PyCToolTipCtrl_methods, 
				       GET_PY_CTOR(PyCToolTipCtrl));
