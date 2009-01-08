/*

	win32 CmdUI implementation.

	Created March 1995, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"
#include "win32menu.h"
#include "win32cmdui.h"

inline void*GetPythonOleProcAddress(const char *procName)
{
	HMODULE hMod = NULL;
	for (int i=15;hMod==NULL && i<40;i++) {
		TCHAR buf[20];
#ifdef _DEBUG
		wsprintf(buf, _T("PythonCOM%d_d.dll"), i);
#else
		wsprintf(buf, _T("PythonCOM%d.dll"), i);
#endif
		hMod = GetModuleHandle(buf);
	}
	if (hMod) {
		void *rc = GetProcAddress(hMod, procName);
		if (rc==NULL)
			RETURN_ERR("The Python COM extensions do not export the required functions");
		return rc;
	} else {
		RETURN_ERR("The PythonCOM module must be imported before OLE functions can be used");
	}
}

static BOOL (*pfnMakeOlePythonCall)(PyObject *handler, DISPPARAMS FAR* params, VARIANT FAR* pVarResult,
	EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr, PyObject *addnlArgs) = NULL;

// General command handler for Python.
BOOL 
Python_OnCmdMsg (CCmdTarget *obj, UINT nID, int nCode, 
		 void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo)
{
	// Let MFC deal with the file menu.
	if (nCode==CN_UPDATE_COMMAND_UI && nID==ID_FILE_MRU_FILE1)
		return FALSE;

#ifndef _AFX_NO_OCC_SUPPORT
	// OLE control events are a special case
	if (nCode == CN_EVENT)
	{
		AFX_EVENT *pEvent = (AFX_EVENT*)pExtra;
		PyObject *method;

		// check if obj is really a CWnd (a CDocument could end up here)
		if ( ! ( obj->IsKindOf( RUNTIME_CLASS( CWnd ) ) ) ) {
			// better quit, otherwise we're in trouble
			return FALSE;
		}
		else {
			// everything's fine

			CWnd *control = ((CWnd *)obj)->GetDlgItem(nID);
			CEnterLeavePython _celp;
			PyCCmdTarget *pObj = (PyCCmdTarget *) ui_assoc_CObject::GetAssocObject(control);
			if (pObj && pObj->pOleEventHookList && 
				pObj->pOleEventHookList->Lookup ((unsigned short)pEvent->m_dispid, (void *&)method)) {
					if (pfnMakeOlePythonCall==NULL) {
						pfnMakeOlePythonCall = (BOOL (*)(PyObject *, DISPPARAMS FAR* , VARIANT FAR* ,EXCEPINFO FAR* , UINT FAR*, PyObject * ))
								GetPythonOleProcAddress("PyCom_MakeOlePythonCall");

						ASSERT(pfnMakeOlePythonCall);
					}
					if (pfnMakeOlePythonCall==NULL) {
						Py_DECREF(pObj);
						return FALSE;
					}
					VARIANT result;
					VariantInit(&result);
					(*pfnMakeOlePythonCall)(method, pEvent->m_pDispParams, &result, pEvent->m_pExcepInfo, pEvent->m_puArgError, NULL);
					VariantClear(&result);
					if (PyErr_Occurred())	// if any Python exception, pretend it was OK
						gui_print_error();
					Py_DECREF(pObj);
					return TRUE;
			}
			Py_XDECREF(pObj);
		}
	}
#endif // !_AFX_NO_OCC_SUPPORT

	CEnterLeavePython _celp;
	PyCCmdTarget *pObj = (PyCCmdTarget *) ui_assoc_CObject::GetAssocObject(obj);
	// Must exit via 'exit' from here...
	BOOL rc = FALSE; // default not handled.
	// Give Python code the chance to handle other stuff.
	if (pObj != NULL &&
		pObj->is_uiobject (&PyCCmdTarget::type)) {

		if (nCode == CN_UPDATE_COMMAND_UI) {
			CCmdUI *pUI = (CCmdUI *)pExtra;
			PyObject *method;
			if (pObj->pCommandUpdateHookList && 
				pObj->pCommandUpdateHookList->Lookup (nID, (void *&)method)) {
				// I have a specific user interface element.
				// create a PyCCmdUI object.
				PyObject *ob = ui_assoc_object::make( PyCCmdUI::type, pUI );
				if (ob==NULL) {
					OutputDebugString(_T("Could not make object for CCmdUI handler"));
					goto done;
				}
				{
					Python_callback (method, ob);
					if (PyErr_Occurred())	// if any Python exception, pretend it was OK
						// XXX - Python_callback always calls
						// gui_print_error() on failure, which
						// clears the error - so we can't get here?
						gui_print_error();
					// object is no longer valid.
					GUI_BGN_SAVE;
					Python_delete_assoc(ob);
					GUI_END_SAVE;
					DODECREF(ob);
				}
				rc = TRUE;
			} else if (pObj->pCommandHookList && 
				       pObj->pCommandHookList->Lookup (nID, (void *&)method)) {
				// we have a handler for the command itself, but not the 
				// user interface element.  Enable the element.
				pUI->Enable();
				rc = TRUE; // did handle it.
			} // else RC remains FALSE.
		} else { // is the command itself.
			// allow either a general or specific handler to be called
			PyObject *method = NULL;
			if (pObj->pCommandHookList) {
				pObj->pCommandHookList->Lookup (nID, (void *&)method);
				if (method==NULL) pObj->pCommandHookList->Lookup (0, (void *&)method);
			}
			if (method) {
					// perform the callback.
				rc = Python_callback (method, nID, nCode);
				// This is dodgy - we have to rely on -1 and can't check PyErr_Occurred(),
				// as Python_callback will have called gui_print_error() which clears
				// the error.
				if (rc==-1) {
					// Raise a *new* exception then print that too.
					char buf[128];
					sprintf(buf, "Error in Command Message handler for command ID %u, Code %d", nID, nCode);
					PyErr_SetString(ui_module_error, buf);
					gui_print_error();
					rc = TRUE;			// to avoid other code handling it.
				} else
					rc = !rc;
			}
		}
	}
done:
	Py_XDECREF(pObj);
	return rc;
}


PyCCmdUI::PyCCmdUI()
{
}
PyCCmdUI::~PyCCmdUI()
{
}

CCmdUI *PyCCmdUI::GetCCmdUIPtr(PyObject *self)
{
	return (CCmdUI *)GetGoodCppObject( self, &type);
}

// @pymethod |PyCCmdUI|Enable|Enables or disables the user-interface item for this command.
static PyObject *
PyCCmdUI_Enable(PyObject *self, PyObject *args)
{
	BOOL bEnable = TRUE;
	if (!PyArg_ParseTuple(args,"|i:Enable", &bEnable)) // @pyparm int|bEnable|1|TRUE if the item should be enabled, false otherwise.
		return NULL;

	CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(self);
	if (!pCU)
		return NULL;
	GUI_BGN_SAVE;
	pCU->Enable(bEnable);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCCmdUI|SetCheck|Sets the check state of the user-interface item for this command.
static PyObject *
PyCCmdUI_SetCheck(PyObject *self, PyObject *args)
{
	int state = 1;
	if (!PyArg_ParseTuple(args,"|i:SetCheck", &state)) // @pyparm int|state|1|0 for unchecked, 1 for checked, or 2 for indeterminate.
		return NULL;

	CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(self);
	if (!pCU)
		return NULL;
	GUI_BGN_SAVE;
	pCU->SetCheck(state);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCCmdUI|SetRadio|Like the SetCheck member function, but operates on radio groups.
static PyObject *
PyCCmdUI_SetRadio(PyObject *self, PyObject *args)
{
	BOOL bOn = TRUE;
	if (!PyArg_ParseTuple(args,"|i:SetRadio", &bOn)) // @pyparm int|bOn|1|TRUE if the item should be enabled, false otherwise.
		return NULL;

	CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(self);
	if (!pCU)
		return NULL;
	GUI_BGN_SAVE;
	pCU->SetRadio(bOn);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCCmdUI|SetText|Sets the text for the user-interface item for this command.
static PyObject *
PyCCmdUI_SetText(PyObject *self, PyObject *args)
{
	TCHAR *txt;
	PyObject *obtxt;
	if (!PyArg_ParseTuple(args,"O:SetText", &obtxt)) // @pyparm string|text||The text for the interface element.
		return NULL;

	CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(self);
	if (!pCU)
		return NULL;
	if (!PyWinObject_AsTCHAR(obtxt, &txt, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	pCU->SetText(txt);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(txt);
	RETURN_NONE;
}

// @pymethod |PyCCmdUI|ContinueRouting|Tells the command-routing mechanism to continue routing the current message down the chain of handlers.
static PyObject *
PyCCmdUI_ContinueRouting(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args, ContinueRouting);
	CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(self);
	if (!pCU)
		return NULL;
	GUI_BGN_SAVE;
	pCU->ContinueRouting();
	GUI_END_SAVE;
	RETURN_NONE;
}

// @object PyCCmdUI|A class for manipulating user-interface elements.  Encapsulates an MFC <c CCmdUI> class
static struct PyMethodDef PyCCmdUI_methods[] = {
	{"Enable",			PyCCmdUI_Enable,         1},// @pymeth Enable|Enables or disables the user-interface item for this command.
	{"SetCheck",		PyCCmdUI_SetCheck,       1},// @pymeth SetCheck|Sets the check state of the user-interface item for this command.
	{"SetRadio",		PyCCmdUI_SetRadio,       1},// @pymeth SetRadio|Like the SetCheck member function, but operates on radio groups.
	{"SetText",			PyCCmdUI_SetText,        1},// @pymeth SetText|Sets the text for the user-interface item for this command.
	{"ContinueRouting",	PyCCmdUI_ContinueRouting,1},// @pymeth ContinueRouting|Tells the command-routing mechanism to continue routing the current message down the chain of handlers.
	{NULL, NULL }
};

PyObject *
PyCCmdUI::getattro(PyObject *obname)
{
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (strcmp(name, "m_nIndex")==0) { // @prop int|m_nIndex|
		CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(this);
		if (!pCU)
			return NULL;
		return PyInt_FromLong(pCU->m_nIndex);
	} else if (strcmp(name, "m_nID")==0) { // @prop int|m_nID|
		CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(this);
		if (!pCU)
			return NULL;
		return PyInt_FromLong(pCU->m_nID);
	} else if (strcmp(name, "m_pMenu")==0) { // @prop <o PyCMenu>|m_pMenu|
		CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(this);
		if (!pCU)
			return NULL;
		if (pCU->m_pMenu)
			return ui_assoc_object::make(PyCMenu::type, pCU->m_pMenu->GetSafeHmenu());
		Py_INCREF(Py_None);
		return Py_None;
	} else if (strcmp(name, "m_pSubMenu")==0) { // @prop <o PyCMenu>|m_pSubMenu|
		CCmdUI *pCU = PyCCmdUI::GetCCmdUIPtr(this);
		if (!pCU)
			return NULL;
		if (pCU->m_pSubMenu)
			return ui_assoc_object::make(PyCMenu::type, pCU->m_pSubMenu->GetSafeHmenu());
		Py_INCREF(Py_None);
		return Py_None;
	}
	return ui_assoc_object::getattro(obname);
}


ui_type PyCCmdUI::type("PyCCmdUI", 
					   &ui_assoc_object::type, 
					   sizeof(PyCCmdUI), 
					   PYOBJ_OFFSET(PyCCmdUI), 
					   PyCCmdUI_methods, 
					   GET_PY_CTOR(PyCCmdUI));

