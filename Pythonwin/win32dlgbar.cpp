/* 
	DialogBar class for Pythonwin

	Created April 1998 by Mark Hammond.

*/
// @doc
#include "stdafx.h"
#include "win32win.h"
#include "win32toolbar.h"
#include "win32dlgbar.h"


CDialogBar *PyCDialogBar::GetDialogBar (PyObject *self)
{
	return (CDialogBar *)GetGoodCppObject( self, &type);
}

/////////////////////////////////////////////////////////////////////
//
// Dialog Bar object
//
//////////////////////////////////////////////////////////////////////

// @pymethod <o PyCDialogBar>|win32ui|CreateDialogBar|Creates a <o PyCDialogBar> object.
PyObject *PyCDialogBar::create(PyObject *self, PyObject *args)
{
   	CHECK_NO_ARGS2(args, "Create");
    GUI_BGN_SAVE;
    CDialogBar *db = new CDialogBar();
    GUI_END_SAVE;
	if (db==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating CDialogBar object");
		return NULL;
	}
    db->m_bAutoDelete = TRUE;  // let MFC handle deletion
    return ui_assoc_object::make (PyCDialogBar::type, db)->GetGoodRet();
}

// @pymethod |PyCDialogBar|CreateWindow|Creates the window for the <o PyCDialogBar> object.
static PyObject *PyCDialogBar_CreateWindow(PyObject *self, PyObject *args)
{
    BOOL bHaveSz = TRUE;
	char *szTemplate;
	UINT style, id, idTemplate;
	PyObject *obParent;
	// @pyparm <o PyCWnd>|parent||The parent window
	// @pyparm string|template||The template to load the resource from
	// @pyparm int|style||The style for the window
	// @pyparm int|id||The ID of the window
    if (!PyArg_ParseTuple(args, "Osii", &obParent, &szTemplate, &style, &id)) {
        PyErr_Clear();
		// @pyparmalt1 <o PyCWnd>|parent||The parent window
		// @pyparmalt1 int|resourceId||The resource ID to load the resource from
		// @pyparmalt1 int|style||The style for the window
		// @pyparmalt1 int|id||The ID of the window
        if (!PyArg_ParseTuple(args, "Oiii", &obParent, &idTemplate, &style, &id))
            RETURN_TYPE_ERR("CreateWindow arguments must have format of either 'Osii' or 'Oiii'");
        bHaveSz = FALSE;
    }
    CDialogBar *pDialog = PyCDialogBar::GetDialogBar(self);
    if (pDialog==NULL) return NULL;
	CWnd *pParent = NULL;
	if (obParent != Py_None) {
		pParent = PyCWnd::GetPythonGenericWnd(obParent, &PyCWnd::type);
		if (pParent==NULL)
			RETURN_TYPE_ERR("The parent window is not a valid PyCWnd");
	}
    GUI_BGN_SAVE;
    BOOL rc = bHaveSz ?
        pDialog->Create(pParent, szTemplate, style, id) :
        pDialog->Create(pParent, idTemplate, style, id);
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("CDialogBar::Create failed");
    RETURN_NONE;
}

// @object PyCDialogBar|A class which encapsulates an MFC <o CDialogBar>.  Derived from a <o PyCControlBar> object.
static struct PyMethodDef PyCDialogBar_methods[] =
{
	{ "CreateWindow", PyCDialogBar_CreateWindow, 1}, // @pymeth CreateWindow|Creates the window for the <o PyCDialogBar> object.
	{ NULL,			NULL }
};

ui_type_CObject PyCDialogBar::type ("PyCDialogBar",
					&PyCControlBar::type, 
					RUNTIME_CLASS(CDialogBar),
					sizeof(PyCDialogBar),
					PyCDialogBar_methods,
					GET_PY_CTOR(PyCDialogBar));
