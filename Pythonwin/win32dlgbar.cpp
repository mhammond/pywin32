/*
    DialogBar class for Pythonwin

    Created April 1998 by Mark Hammond.

*/
// @doc
#include "stdafx.h"
#include "win32win.h"
#include "win32toolbar.h"
#include "win32dlgbar.h"

CDialogBar *PyCDialogBar::GetDialogBar(PyObject *self) { return (CDialogBar *)GetGoodCppObject(self, &type); }

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
    if (db == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating CDialogBar object");
        return NULL;
    }
    db->m_bAutoDelete = TRUE;  // let MFC handle deletion
    return ui_assoc_object::make(PyCDialogBar::type, db)->GetGoodRet();
}

// @pymethod |PyCDialogBar|CreateWindow|Creates the window for the <o PyCDialogBar> object.
static PyObject *PyCDialogBar_CreateWindow(PyObject *self, PyObject *args)
{
    TCHAR *szTemplate;
    UINT style, id;
    PyObject *obParent, *obTemplate;
    // @pyparm <o PyCWnd>|parent||The parent window
    // @pyparm <o PyResourceId>|template||Template name or integer resource id
    // @pyparm int|style||The style for the window
    // @pyparm int|id||The ID of the window
    if (!PyArg_ParseTuple(args, "OOii", &obParent, &obTemplate, &style, &id))
        return NULL;

    CDialogBar *pDialog = PyCDialogBar::GetDialogBar(self);
    if (pDialog == NULL)
        return NULL;
    CWnd *pParent = NULL;
    if (obParent != Py_None) {
        pParent = PyCWnd::GetPythonGenericWnd(obParent, &PyCWnd::type);
        if (pParent == NULL)
            RETURN_TYPE_ERR("The parent window is not a valid PyCWnd");
    }
    if (!PyWinObject_AsResourceId(obTemplate, &szTemplate, FALSE))
        return NULL;
    BOOL rc;
    GUI_BGN_SAVE;
    if (IS_INTRESOURCE(szTemplate))
        rc = pDialog->Create(pParent, MAKEINTRESOURCE(szTemplate), style, id);
    else
        rc = pDialog->Create(pParent, szTemplate, style, id);
    GUI_END_SAVE;
    PyWinObject_FreeResourceId(szTemplate);
    if (!rc)
        RETURN_ERR("CDialogBar::Create failed");
    RETURN_NONE;
}

// @object PyCDialogBar|A class which encapsulates an MFC <o CDialogBar>.  Derived from a <o PyCControlBar> object.
static struct PyMethodDef PyCDialogBar_methods[] = {
    {"CreateWindow", PyCDialogBar_CreateWindow,
     1},  // @pymeth CreateWindow|Creates the window for the <o PyCDialogBar> object.
    {NULL, NULL}};

ui_type_CObject PyCDialogBar::type("PyCDialogBar", &PyCControlBar::type, RUNTIME_CLASS(CDialogBar),
                                   sizeof(PyCDialogBar), PYOBJ_OFFSET(PyCDialogBar), PyCDialogBar_methods,
                                   GET_PY_CTOR(PyCDialogBar));
