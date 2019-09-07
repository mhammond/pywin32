/*

    dialog data type

    Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

    Coupla enhancements and bugfixes donated by Charles G. Waldman <cgw@pgt.com>

    dialog is derived from window.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32dc.h"
#include "win32dll.h"
#include "win32dlg.h"
#include "win32prinfo.h"

typedef CPythonDlgFramework<CDialog> CPythonDlg;

class CProtectedDialog : public CDialog {
   public:
    void BaseOnOK() { CDialog::OnOK(); }
    void BaseOnCancel() { CDialog::OnCancel(); }
    HGLOBAL GetDialogTemplate() { return m_hDialogTemplate; }
    BOOL BaseOnInitDialog() { return CDialog::OnInitDialog(); }
};

typedef CPythonPrtDlgFramework<CPrintDialog> CPythonPrtDlg;

class CProtectedPrintDialog : public CPrintDialog {
   public:
    void BaseOnOK() { CPrintDialog::OnOK(); }
    void BaseOnCancel() { CPrintDialog::OnCancel(); }
    HGLOBAL GetDialogTemplate() { return m_hDialogTemplate; }
    BOOL BaseOnInitDialog() { return CPrintDialog::OnInitDialog(); }
};

extern HGLOBAL MakeResourceFromDlgList(PyObject *tmpl);
extern PyObject *MakeDlgListFromResource(HGLOBAL res);

extern char *errmsgAlreadyInit;

// @pymethod list|win32ui|LoadDialogResource|Loads a dialog resource, and returns a list detailing the objects.
PyObject *ui_get_dialog_resource(PyObject *, PyObject *args)
{
    int idRes;
    HINSTANCE hMod = NULL, hOldRes = NULL;
    PyObject *obDLL = NULL;
    // @pyparm int|idRes||The ID of the dialog resource to load.
    // @pyparm <o PyDLL>|dll|None|The DLL object to load the dialog from.
    if (!PyArg_ParseTuple(args, "i|O:LoadDialogResource", &idRes, &obDLL))
        return NULL;
    if (obDLL && obDLL != Py_None) {
        // passed a DLL object.
        if (!ui_base_class::is_uiobject(obDLL, &dll_object::type))
            RETURN_TYPE_ERR("passed object must be a PyDLL object");
        hMod = ((dll_object *)obDLL)->GetDll();
    }
    if (hMod == NULL)
        hMod = AfxFindResourceHandle(MAKEINTRESOURCE(idRes), RT_DIALOG);
    else {
        hOldRes = AfxGetResourceHandle();
        AfxSetResourceHandle(hMod);
    }
    HGLOBAL hGlob;
    HRSRC hrsrc;
    hrsrc = ::FindResourceEx(hMod, RT_DIALOG, MAKEINTRESOURCE(idRes), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hrsrc)
        hGlob = ::LoadResource(hMod, hrsrc);
    if (hOldRes)
        AfxSetResourceHandle(hOldRes);
    if (hrsrc == NULL)
        RETURN_API_ERR("FindResourceEx");
    if (hGlob == NULL)
        RETURN_API_ERR("LoadResource");
    return MakeDlgListFromResource(hGlob);
}

CDialog *GetDialog(PyObject *self) { return (CDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCDialog::type); }
CPythonDlg *GetPythonDlg(PyObject *self) { return (CPythonDlg *)PyCWnd::GetPythonGenericWnd(self, &PyCDialog::type); }

CFileDialog *GetFileDialog(PyObject *self)
{
    return (CFileDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCFileDialog::type);
}
CFontDialog *GetFontDialog(PyObject *self)
{
    return (CFontDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCFontDialog::type);
}

CColorDialog *GetColorDialog(PyObject *self)
{
    return (CColorDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCColorDialog::type);
}

CPrintDialog *GetPrintDialog(PyObject *self)
{
    return (CPrintDialog *)PyCWnd::GetPythonGenericWnd(self, &PyCPrintDialog::type);
}

/////////////////////////////////////////////////////////////////////
//
// Utilities that work with dialogs!
//
//////////////////////////////////////////////////////////////////////
// @pymethod |PyCWinApp|InitDlgInstance|Calls critical InitInstance processing for a dialog based application.
PyObject *ui_init_dlg_instance(PyObject *self, PyObject *args)
{
    CDialog *pDlg;
    PyObject *obDlg;
    CProtectedWinApp *pApp = GetProtectedApp();
    if (!pApp)
        return NULL;

    if (!PyArg_ParseTuple(args, "O:InitDlgInstance", &obDlg))
        return NULL;
    // @pyparm <o PyCDialog>|dialog||The dialog object to be used as the main window for the application.
    if (!ui_base_class::is_uiobject(obDlg, &PyCDialog::type))
        RETURN_TYPE_ERR("First arg must be a PyCDialog");
    if (!(pDlg = GetDialog(obDlg)))
        return NULL;

    pApp->SetMainFrame(pDlg);
    RETURN_NONE;
}

////////////////////////////////////////////////////////////////////
//
// Dialog object
//
//////////////////////////////////////////////////////////////////////
PyCDialog::PyCDialog()
{
    // Memory tracking for dialogs is a real pain.  In normal MFC, many
    // dialogs are used as local variables - so lifetimes are not an issue.
    // However, Python must allocate dialogs with new() - so we assume that
    // when a dialog must stay alive, there will be a reference held.
    bManualDelete = TRUE;
    hTemplate = NULL;
    hInstance = NULL;
    hSaved = NULL;
    ddlist = PyList_New(0);
    dddict = PyDict_New();
}
PyCDialog::~PyCDialog()
{
    //	TRACE("Dialog object destructing\n");
    //	CDialog *pDlg;
    //	if ((pDlg=GetDialog(this)))
    //		delete pDlg;
    if (hSaved) {
        GlobalUnlock(hSaved);
        GlobalFree(hSaved);
    }

    ui_assoc_object::SetAssocInvalid();  // must call this explicitely, as I ignore SetAssocInvalid
    Py_XDECREF(ddlist);                  // we can not have the pointer deleted at window destruction time
    // for a dialog (as MFC still needs it after the dialog has completed
    BOOL bManDeleteSave = bManualDelete;
    Py_XDECREF(dddict);
}

PyObject *PyCDialog::getattro(PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (strcmp(name, "data") == 0) {
        Py_INCREF(dddict);
        return dddict;
    }
    if (strcmp(name, "datalist") == 0) {
        Py_INCREF(ddlist);
        return ddlist;
    }
    return PyObject_GenericGetAttr(this, obname);
}

static PyObject *set_exchange_error(const char *msg, int index)
{
    static char errBuf[256];
    snprintf(errBuf, 256, "Data exchange list index %d - %s", index, msg);
    PyErr_SetString(PyExc_TypeError, errBuf);
    return NULL;
}

static PyObject *do_exchange_edit(int id, int index, char *type, PyObject *oldVal, PyObject *o1, PyObject *o2,
                                  CDataExchange *pDX)
{
    // Note use of funky exception handlers to ensure thread-state remains correct even when MFC exception is thrown.
    PyObject *newOb;
    switch (type[0]) {
        case 'i': {
            int intVal = 0;
            if (oldVal)
                intVal = (int)PyInt_AsLong(oldVal);
            PyThreadState *_save = PyEval_SaveThread();
            TRY
            {
                DDX_Text(pDX, id, intVal);
                PyEval_RestoreThread(_save);
            }
            CATCH_ALL(e)
            {
                PyEval_RestoreThread(_save);
                THROW(e);
            }
            END_CATCH_ALL
            if (o1 && o2) {
                if (PyInt_Check(o1) && PyInt_Check(o2))
                    DDV_MinMaxInt(pDX, intVal, PyInt_AsLong(o1), PyInt_AsLong(o2));
                else
                    return set_exchange_error("Edit - must be tuple of control_id, key, 'i', intMin, intMax", index);
            }
            newOb = Py_BuildValue("i", intVal);
            break;
        }
        case 's': {
            CString csVal;
            TCHAR *strVal = NULL;
            if (PyWinObject_AsTCHAR(oldVal, &strVal, TRUE)) {
                csVal = strVal;
                PyWinObject_FreeTCHAR(strVal);
            }
            else {
                PyErr_Clear();
                csVal = _T("");
            }
            PyThreadState *_save = PyEval_SaveThread();
            TRY
            {
                DDX_Text(pDX, id, csVal);
                PyEval_RestoreThread(_save);
            }
            CATCH_ALL(e)
            {
                PyEval_RestoreThread(_save);
                THROW(e);
            }
            END_CATCH_ALL
            if (o1 && o2) {
                if (PyInt_Check(o1) && o2 == NULL)
                    DDV_MaxChars(pDX, csVal, PyInt_AsLong(o1));
                else
                    return set_exchange_error("Edit - must be tuple of control_id, key, 's', maxLength", index);
            }
            newOb = PyWinObject_FromTCHAR(csVal);
            break;
        }
        default:
            return set_exchange_error("type param must be 'i' or 's' for edit controls", index);
    }
    return newOb;
}
static PyObject *do_exchange_list_combo(int id, int index, char *type, PyObject *oldVal, PyObject *o1, PyObject *o2,
                                        CDataExchange *pDX, BOOL bList)
{
    if (o1 && o2)
        return set_exchange_error("List/ComboBox - must be tuple of control_id, key, 'i|s'", index);
    HWND hWndCtrl;
    pDX->m_pDlgWnd->GetDlgItem(id, &hWndCtrl);
    if (hWndCtrl == NULL)
        return set_exchange_error("There is no control with that ID", index);

    PyObject *newOb = NULL;
    switch (type[0]) {
        case 'i': {
            int intVal = 0;
            if (oldVal && oldVal != Py_None) {
                if (!PyInt_Check(oldVal))
                    return set_exchange_error("'i' format requires integers", index);
                intVal = (int)PyInt_AsLong(oldVal);
            }
            GUI_BGN_SAVE;
            if (bList)
                DDX_LBIndex(pDX, id, intVal);
            else
                DDX_CBIndex(pDX, id, intVal);
            GUI_END_SAVE;
            newOb = Py_BuildValue("i", intVal);
            break;
        }
        case 's': {
            TCHAR *strVal = NULL;
            if (!PyWinObject_AsTCHAR(oldVal, &strVal, TRUE))
                return set_exchange_error("'s' format requires strings", index);
            CString csVal(strVal ? strVal : _T(""));
            PyWinObject_FreeTCHAR(strVal);

            GUI_BGN_SAVE;
            if (bList)
                DDX_LBString(pDX, id, csVal);
            else
                DDX_CBString(pDX, id, csVal);
            GUI_END_SAVE;
            newOb = PyWinObject_FromTCHAR(csVal);
            break;
        }
        case 'S': {
            TCHAR *strVal = NULL;
            if (!PyWinObject_AsTCHAR(oldVal, &strVal, TRUE))
                return set_exchange_error("'S' format requires strings", index);
            CString csVal(strVal ? strVal : _T(""));
            PyWinObject_FreeTCHAR(strVal);
            GUI_BGN_SAVE;
            if (bList)
                DDX_LBStringExact(pDX, id, csVal);
            else
                DDX_CBStringExact(pDX, id, csVal);
            GUI_END_SAVE;
            newOb = PyWinObject_FromTCHAR(csVal);
            break;
        }
        case 'l': {
            HWND hWndCtrl = pDX->PrepareCtrl(id);
            if (pDX->m_bSaveAndValidate) {
                // ??? Needs to use LB_GETTEXTLEN to get length instead of fixed buffer size ???
                TCHAR buf[512];
                int count = (int)::SendMessage(hWndCtrl, bList ? LB_GETCOUNT : CB_GETCOUNT, 0, 0L);
                newOb = PyList_New(count);
                for (int i = 0; i < count; i++) {
                    GUI_BGN_SAVE;
                    ::SendMessage(hWndCtrl, bList ? LB_GETTEXT : CB_GETLBTEXT, i, (LPARAM)buf);
                    GUI_END_SAVE;
                    PyList_SET_ITEM(newOb, i, PyWinObject_FromTCHAR(buf));
                }
            }
            else {
                if (oldVal && PyList_Check(oldVal)) {
                    GUI_BGN_SAVE;
                    ::SendMessage(hWndCtrl, bList ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0L);
                    GUI_END_SAVE;
                    for (int i = 0; i < PyList_Size(oldVal); i++) {
                        PyObject *ob = PyList_GetItem(oldVal, i);
                        TCHAR *val;
                        if (ob && PyWinObject_AsTCHAR(ob, &val, FALSE)) {
                            GUI_BGN_SAVE;
                            ::SendMessage(hWndCtrl, bList ? LB_ADDSTRING : CB_ADDSTRING, 0, (LPARAM)val);
                            GUI_END_SAVE;
                            PyWinObject_FreeTCHAR(val);
                        }
                    }
                }
            }
            break;
        }
        default:
            return set_exchange_error("type param must be 'i','s','S' or 'l' for listbox/combo controls", index);
    }
    return newOb;
}

static PyObject *do_exchange_button(CDialog *pDlg, int id, int index, char *type, PyObject *oldVal, PyObject *o1,
                                    PyObject *o2, CDataExchange *pDX)
{
    if (o1 && o2)
        return set_exchange_error("Button - must be tuple of control_id, key, 'i|s'", index);
    CWnd *pWnd = pDlg->GetDlgItem(id);
    if (pWnd == NULL)
        return set_exchange_error("control with that ID does not exist", index);

    // Need to check certain attributes on the window.  MFC Asserts otherwise.
    HWND hwnd = pWnd->GetSafeHwnd();
    DWORD dwStyle = ::GetWindowLong(hwnd, GWL_STYLE);
    LRESULT dwCode = ::SendMessage(hwnd, WM_GETDLGCODE, 0, 0L);
    BOOL bRadio = (dwCode & DLGC_RADIOBUTTON) != 0;
    BOOL bCheck = (dwStyle & BS_CHECKBOX) != 0;
    if (!bRadio && !bCheck)
        return set_exchange_error("only radios and checkboxes are supported for button controls", index);
    if ((bRadio && (dwStyle & BS_AUTORADIOBUTTON) == 0) || (bCheck && (dwStyle & BS_AUTOCHECKBOX) == 0))
        return set_exchange_error("the button must have the 'auto' style set", index);
    if (bRadio) {
        if ((dwStyle & WS_GROUP) == 0)
            // Not a group leader - this is not considered an error condition,
            // the group leader provides data for the entire group.
            return NULL;
    }
    int intVal = 0;
    if (oldVal) {
        if (!PyInt_Check(oldVal))
            return set_exchange_error("the previous value was not a number!", index);
        intVal = (int)PyInt_AsLong(oldVal);
    }
    GUI_BGN_SAVE;
    if (bRadio)
        DDX_Radio(pDX, id, intVal);
    else
        DDX_Check(pDX, id, intVal);
    GUI_END_SAVE;
    return Py_BuildValue("i", intVal);
}

void Python_do_exchange(CDialog *pDlg, CDataExchange *pDX)
{
    CEnterLeavePython _celp;
    PyCDialog *dob = (PyCDialog *)ui_assoc_object::GetAssocObject(pDlg);
    if (!dob) {
        TRACE("do_exchange called on dialog with no Python object!\n");
        return;  // dont print an exception
    }
    for (int i = 0; i < PyList_Size(dob->ddlist); i++) {
        PyObject *ob = PyList_GetItem(dob->ddlist, i);
        if (ob == NULL)
            break;
        int id;
        PyObject *obAttr;
        PyObject *o1 = NULL, *o2 = NULL;
        char *szType = "s";
        if (!PyArg_ParseTuple(ob, "iO|sOO", &id, &obAttr, &szType, &o1, &o2)) {
            set_exchange_error("must be tuple of control_id, key ...", i);
            break;
        }
        if (id == 0 || id == -1) {
            set_exchange_error("control ID must be a value other than 0 or -1", i);
            break;
        }

        PyObject *oldOb = PyDict_GetItem(dob->dddict, obAttr);
        TCHAR szClassName[64];
        ::GetClassName(pDlg->GetDlgItem(id)->GetSafeHwnd(), szClassName, sizeof(szClassName) / sizeof(TCHAR));
        PyObject *newOb = NULL;
        try {
            if (_tcscmp(szClassName, _T("Edit")) == 0 || _tcscmp(szClassName, _T("Static")) == 0)
                newOb = do_exchange_edit(id, i, szType, oldOb, o1, o2, pDX);
            else if (_tcscmp(szClassName, _T("ListBox")) == 0)
                newOb = do_exchange_list_combo(id, i, szType, oldOb, o1, o2, pDX, TRUE);
            else if (_tcscmp(szClassName, _T("ComboBox")) == 0)
                newOb = do_exchange_list_combo(id, i, szType, oldOb, o1, o2, pDX, FALSE);
            else if (_tcscmp(szClassName, _T("Button")) == 0)
                newOb = do_exchange_button(pDlg, id, i, szType, oldOb, o1, o2, pDX);
            if (newOb) {
                PyDict_SetItem(dob->dddict, obAttr, newOb);
                Py_DECREF(newOb);
            }
        }
        catch (CNotSupportedException *e) {
            e->Delete();
            set_exchange_error("No control by that name, or other MFC 'NotSupported' exception", i);
        }
    }
    if (PyErr_Occurred())
        gui_print_error();
    Py_DECREF(dob);
}

// @pymethod <o PyCDialog>|win32ui|CreateDialog|Creates a dialog object.
PyObject *PyCDialog::create(PyObject *self, PyObject *args)
{
    int idRes;
    HINSTANCE hMod = NULL, hOldRes = NULL;
    PyObject *obDLL = NULL;
    if (!PyArg_ParseTuple(args, "i|O:CreateDialog",
                          &idRes,   // @pyparm int|idRes||The ID of the dialog resource to load.
                          &obDLL))  // @pyparm <o PyDLL>|dll|None|The DLL object to load the dialog from.
        return NULL;
    if (obDLL && obDLL != Py_None) {
        // passed a DLL object.
        if (!ui_base_class::is_uiobject(obDLL, &dll_object::type))
            RETURN_TYPE_ERR("passed object must be a PyDLL");
        hMod = ((dll_object *)obDLL)->GetDll();
        if (hMod == NULL)
            RETURN_ERR("Can not load from an uninitialised DLL object");
    }
    if (hMod == NULL)
        hMod = AfxFindResourceHandle(MAKEINTRESOURCE(idRes), RT_DIALOG);
    else {
        hOldRes = AfxGetResourceHandle();
        AfxSetResourceHandle(hMod);
    }

    HGLOBAL hGlob;
    HRSRC hrsrc;
    hrsrc = ::FindResourceEx(hMod, RT_DIALOG, MAKEINTRESOURCE(idRes), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hrsrc)
        hGlob = ::LoadResource(hMod, hrsrc);
    if (hOldRes)
        AfxSetResourceHandle(hOldRes);
    if (hrsrc == NULL)
        RETURN_API_ERR("FindResourceEx");
    if (hGlob == NULL)
        RETURN_API_ERR("LoadResource");

    GUI_BGN_SAVE;
    CDialog *pDlg = new CPythonDlg();
    GUI_END_SAVE;
    PyCDialog *ret = (PyCDialog *)ui_assoc_object::make(PyCDialog::type, pDlg, TRUE);
    if (ret) {
        ret->hTemplate = hGlob;
        ret->hInstance = hMod;
    }
    return ret;
}

// @pymethod <o PyCDialog>|win32ui|CreateDialogIndirect|Creates a dialog object from a template.
PyObject *PyCDialog::createIndirect(PyObject *, PyObject *args)
{
    PyObject *obList = NULL;
    // @pyparm list|obList||A list of [<o PyDLGTEMPLATE>, <o PyDLGITEMTEMPLATE>, ...], which describe the dialog to be
    // created.
    if (!PyArg_ParseTuple(args, "O:CreateDialogIndirect", &obList))
        return NULL;

    HGLOBAL h = MakeResourceFromDlgList(obList);
    if (h == NULL)
        return NULL;
    CDialog *pDlg = new CPythonDlg();
    PyCDialog *ret = (PyCDialog *)ui_assoc_object::make(PyCDialog::type, pDlg, TRUE);
    if (ret) {
        ret->hSaved = h;
        ret->hTemplate = (HGLOBAL)GlobalLock(h);
    }
    return ret;
    // @ comm The code for Dynamic Dialogs was supplied by Curt Hagenlocher \<curt@hagenlocher.org\>.  These notes are
    // also from Curt.<nl> Error checking is thorough but cryptic.  More intelligent error messages could be
    // produced.<nl> obList is a list containing one or more further lists.  The first of these is a description of the
    // dialog box itself.  The others are descriptions of the children (ie, the controls).<nl><nl> Dialog Header:<nl>
    //  [caption (bounds) style extended-style (font) menu window-class]<nl>
    // The first three parameters are required:<nl>
    // * caption must currently be a string object<nl>
    // * (bounds) must be a tuple (x, y, width, height) in dialog units.<nl>
    // * style is the window style, and must be a long object<nl>
    // The last four parameters are optional.  "None" is used as a placeholder, if necessary<nl>
    // * extended-style is the extended window style.  It must be None or a long.<nl>
    // * (font) must be a tuple (fontsize, fontname) or None<nl>
    // * menu is either a menu id (int) or a menu name (string object) or None<nl>
    // * window-class is the window class name of the dialog or None for default<nl><nl>
    // Dialog Item:<nl>
    //  [window-class text child-id (bounds) style extended-style extra-data]<nl>
    // The first five parameters are required:<nl>
    // * window-class describes the child window.  It can be a string object
    // ("EDIT" or "listbox") or a predefined integer type (eg, 133 is a combobox).<nl>
    // * text is the window text (string).  Each control type uses this differently.<nl>
    // * child-id is the id for the item.<nl>
    // * (bounds) is again a tuple (x, y, width, height) in dialog units<nl>
    // * style is the window style (long)<nl>
    // The next two parameters are optional:<nl>
    // * extended-style is the extended window style.  It must be None or a long.<nl>
    // * extra-data is a string with extra initialization data to be sent to the
    // control on creation.  I've never actually seen any control use this!
}

///////////////////////////////////////
//
// Dialog Methods
//
// @pymethod int|PyCDialog|DoModal|Create a modal window for the dialog box.
static PyObject *ui_dialog_do_modal(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, DoModal);
    CDialog *pDlg;
    if (!(pDlg = GetDialog(self)))
        return NULL;
    if (pDlg->m_hWnd)
        RETURN_ERR("cannot call DoModal on a dialog already constructed as modeless");
    if (pDlg->IsKindOf(RUNTIME_CLASS(CDialog))) {
        CProtectedDialog *pD = (CProtectedDialog *)pDlg;
        if (pD->GetDialogTemplate())
            RETURN_ERR("cannot call DoModal twice.");
    }
    PyCDialog *obDlg = (PyCDialog *)self;
    if (obDlg->hTemplate && !pDlg->InitModalIndirect(obDlg->hTemplate)) {
        RETURN_ERR("InitModalIndirect failed");
    }

    Py_INCREF(self);  // make sure Python doesnt kill the object while in a modal call.
                      // really only for the common dialog, and other non CPythonDlg's
    INT_PTR ret;
    GUI_BGN_SAVE;
    ret = pDlg->DoModal();  // @pyseemfc CDialog|DoModal
    GUI_END_SAVE;
    DODECREF(self);
    return PyWinObject_FromDWORD_PTR(ret);
    // @rdesc The return value from the dialog.  This is the value passed to <om PyCDialog.EndDialog>.
}
// @pymethod |PyCDialog|CreateWindow|Create a modeless window for the dialog box.
static PyObject *ui_dialog_create_window(PyObject *self, PyObject *args)
{
    PyObject *obParent = NULL;
    CWnd *pParent = NULL;
    // @pyparm <o PyCWnd>|obParent|None|The parent window for the new window
    if (!PyArg_ParseTuple(args, "|O:CreateWindow", &obParent))
        return NULL;
    if (obParent && obParent != Py_None) {
        // passed a DLL object.
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("passed object must be a PyCWnd object");
        pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent);
        if (!pParent)
            return NULL;
    }

    CPythonDlg *pDlg;
    if (!(pDlg = GetPythonDlg(self)))
        return NULL;
    if (pDlg->m_hWnd)
        RETURN_ERR("win32ui: dialog already created");

    PyCDialog *obDlg = (PyCDialog *)self;
    if (obDlg->hTemplate == NULL)
        RETURN_ERR("Internal Error - dialog has no template attached");
    int rc;
    GUI_BGN_SAVE;
    rc = pDlg->CreateIndirect(obDlg->hTemplate, pParent);  // @pyseemfc CDialog|CreateIndirect
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR("CreateIndirect failed");
    RETURN_NONE;
}

// @pymethod |PyCDialog|EndDialog|Ends a modal dialog box.
static PyObject *ui_dialog_end_dialog(PyObject *self, PyObject *args)
{
    CDialog *pDlg;
    if (!(pDlg = GetDialog(self)))
        return NULL;
    int result;
    // @pyparm int|result||The value to be returned by the <om PyCDialog.DoModal> method.
    if (!PyArg_ParseTuple(args, "i:EndDialog", &result))
        return NULL;
    GUI_BGN_SAVE;
    pDlg->EndDialog(result);  // @pyseemfc CDialog|EndDialog
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCDialog|GotoDlgCtrl|Moves the focus to the specified control in the dialog box.
static PyObject *ui_dialog_goto_dlg_ctrl(PyObject *self, PyObject *args)
{
    CDialog *pDlg;
    if (!(pDlg = GetDialog(self)))
        return NULL;
    // @pyparm <o PyCWnd>|control||The control to get the focus.
    PyObject *obWindow;
    if (!PyArg_ParseTuple(args, "O:GotoDlgCtrl", &obWindow))
        return NULL;
    if (!ui_base_class::is_uiobject(obWindow, &PyCWnd::type))
        RETURN_TYPE_ERR("Argument must be a PyCWnd object");
    CWnd *pChild = PyCWnd::GetPythonGenericWnd(obWindow);
    if (!pChild)
        return NULL;

    GUI_BGN_SAVE;
    pDlg->GotoDlgCtrl(pChild);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod (left, top, right, bottom)|PyCDialog|MapDialogRect|Converts the dialog-box units of a rectangle to screen
// units.
static PyObject *ui_dialog_map_dialog_rect(PyObject *self, PyObject *args)
{
    CDialog *pDlg;
    if (!(pDlg = GetDialog(self)))
        return NULL;
    // @pyparm (left, top, right, bottom)|rect||The rect to be converted
    RECT rect;
    if (!PyArg_ParseTuple(args, "(iiii):MapDialogRect", &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;
    GUI_BGN_SAVE;
    pDlg->MapDialogRect(&rect);
    GUI_END_SAVE;
    return Py_BuildValue("iiii", rect.left, rect.top, rect.right, rect.bottom);
}

// @pymethod |PyCDialog|OnOK|Calls the default MFC OnOK handler.
static PyObject *ui_dialog_on_ok(PyObject *self, PyObject *args)
{
    CProtectedDialog *pDlg;
    if (!(pDlg = (CProtectedDialog *)GetPythonDlg(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnOK);
    // @xref <vm PyCDialog.OnOK>
    GUI_BGN_SAVE;
    pDlg->BaseOnOK();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCDialog|OnCancel|Calls the default MFC OnCancel handler.
static PyObject *ui_dialog_on_cancel(PyObject *self, PyObject *args)
{
    CPythonDlg *pDlg;
    if (!(pDlg = GetPythonDlg(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnCancel);
    // @xref <vm PyCDialog.OnCancel>
    GUI_BGN_SAVE;
    // We call DoOnCancel rather than BaseOnCancel
    // so we can cleanup dialog templates correctly.
    pDlg->DoOnCancel();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod int|PyCDialog|OnInitDialog|Calls the default MFC OnInitDialog handler.
static PyObject *ui_dialog_on_init_dialog(PyObject *self, PyObject *args)
{
    CProtectedDialog *pDlg;
    if (!(pDlg = (CProtectedDialog *)GetPythonDlg(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnInitDialog);
    GUI_BGN_SAVE;
    int rc = pDlg->BaseOnInitDialog();
    GUI_END_SAVE;
    // @xref <vm PyCDialog::OnInitDialog>
    return Py_BuildValue("i", rc);
}

// inherited from window
//
///////////////////////////////////////
// @object PyCDialog|A class which encapsulates an MFC CDialog object.  Derived from a <o PyCWnd> object.
static struct PyMethodDef ui_dialog_methods[] = {
    {"CreateWindow", ui_dialog_create_window, 1},  // @pymeth CreateWindow|Creates a modless window for the dialog.
    {"DoModal", ui_dialog_do_modal, 1},            // @pymeth DoModal|Creates a modal window for the dialog.
    {"EndDialog", ui_dialog_end_dialog, 1},        // @pymeth EndDialog|Closes a modal dialog.
    {"GotoDlgCtrl", ui_dialog_goto_dlg_ctrl, 1},   // @pymeth GotoDlgCtrl|Sets focus to a specific control.
    {"MapDialogRect", ui_dialog_map_dialog_rect,
     1},  // @pymeth MapDialogRect|Converts the dialog-box units of a rectangle to screen units.
    {"OnCancel", ui_dialog_on_cancel, 1},           // @pymeth OnCancel|Calls the default MFC OnCancel handler.
    {"OnOK", ui_dialog_on_ok, 1},                   // @pymeth OnOK|Calls the default MFC OnOK handler.
    {"OnInitDialog", ui_dialog_on_init_dialog, 1},  // @pymeth OnInitDialog|Calls the default MFC OnInitDialog handler.
    {NULL, NULL}                                    /* sentinel */
};

ui_type_CObject PyCDialog::type("PyCDialog",
                                &PyCWnd::type,  // @base PyCDialog|PyCWnd
                                RUNTIME_CLASS(CDialog), sizeof(PyCDialog), PYOBJ_OFFSET(PyCDialog), ui_dialog_methods,
                                GET_PY_CTOR(PyCDialog));

// @object PyCCommonDialog|An abstract class which encapsulates an MFC CCommonDialog object.  Derived from a <o
// PyCDialog> object.
static struct PyMethodDef ui_common_dialog_methods[] = {{NULL, NULL}};

ui_type_CObject PyCCommonDialog::type("PyCCommonDialog", &PyCDialog::type,
                                      NULL,  // CCommonDialog doesnt have RTTI???
                                      sizeof(PyCCommonDialog), PYOBJ_OFFSET(PyCCommonDialog), ui_common_dialog_methods,
                                      NULL);

/////////////////////////////////////////////////////////////////////
//
// File Dialog object
//
//////////////////////////////////////////////////////////////////////
PyCFileDialog::PyCFileDialog() {}

PyCFileDialog::~PyCFileDialog()
{
    CFileDialog *pDlg = GetFileDialog(this);
    if (pDlg) {
        PyWinObject_FreeTCHAR((TCHAR *)pDlg->m_ofn.lpstrTitle);
        PyWinObject_FreeTCHAR((TCHAR *)pDlg->m_ofn.lpstrInitialDir);
    }
}

// @pymethod <o PyCFileDialog>|win32ui|CreateFileDialog|Creates a File Open/Save/etc Common Dialog.
PyObject *PyCFileDialog::ui_file_dialog_create(PyObject * /*self*/, PyObject *args)
{
    int bFileOpen;
    DWORD flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    TCHAR *szDefExt = NULL, *szFileName = NULL, *szFilter = NULL;
    PyObject *obDefExt = Py_None, *obFileName = Py_None, *obFilter = Py_None;
    CWnd *pParent = NULL;  // should mean same as GetApp()->m_pMainWnd
    PyObject *obParent = Py_None;

    if (!PyArg_ParseTuple(
            args, "i|OOiOO:CreateFileDialog",
            &bFileOpen,  // @pyparm int|bFileOpen||A flag indicating if the Dialog is a FileOpen or FileSave dialog.
            &obDefExt,   // @pyparm string|defExt|None|The default file extension for saved files. If None, no extension
                         // is supplied.
            &obFileName,  // @pyparm string|fileName|None|The initial filename that appears in the filename edit box. If
                          // None, no filename initially appears.
            &flags,       // @pyparm int|flags|win32con.OFN_HIDEREADONLY\|win32con.OFN_OVERWRITEPROMPT|The flags for the
                          // dialog.  See the API documentation for full details.
            &obFilter,  // @pyparm string|filter|None|A series of string pairs that specify filters you can apply to the
                        // file. If you specify file filters, only selected files will appear in the Files list box. The
                        // first string in the string pair describes the filter; the second string indicates the file
                        // extension to use. Multiple extensions may be specified using ';' as the delimiter. The string
                        // ends with two '\|' characters.  May be None.
            &obParent))  // @pyparm <o PyCWnd>|parent|None|The parent or owner window of the dialog.
        return NULL;

    if (obParent != Py_None) {
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("Parent arg must be a PyCWnd object");
        if (!(pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent)))
            return NULL;
    }

    CFileDialog *pDlg;
    PyCFileDialog *newObj = NULL;
    if (PyWinObject_AsTCHAR(obDefExt, &szDefExt, TRUE) && PyWinObject_AsTCHAR(obFileName, &szFileName, TRUE) &&
        PyWinObject_AsTCHAR(obFilter, &szFilter, TRUE)) {
        pDlg = new CFileDialog(bFileOpen, szDefExt, szFileName, flags, szFilter, pParent);
        if (!pDlg) {
            PyErr_SetString(ui_module_error, "Creating CFileDialog failed");  // pyseemfc CFileCialog|CFileDialog
            PyWinObject_FreeTCHAR(szDefExt);
            PyWinObject_FreeTCHAR(szFileName);
            PyWinObject_FreeTCHAR(szFilter);
        }
        else
            newObj = (PyCFileDialog *)ui_assoc_object::make(PyCFileDialog::type, pDlg, TRUE);
    }
    //	if (newObj)
    //		newObj->bManualDelete = TRUE;
    return newObj;
}
// @pymethod string|PyCFileDialog|GetPathName|Retrives the path name from the file dialog.
static PyObject *ui_file_dialog_get_path_name(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetPathName);
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    GUI_BGN_SAVE;
    CString cs = pDlg->GetPathName();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(cs);  // @pyseemfc CFileDialog|GetPathName
}
// @pymethod string|PyCFileDialog|GetFileName|Retrives the file name from the file dialog.
static PyObject *ui_file_dialog_get_file_name(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetFileName);
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    GUI_BGN_SAVE;
    CString cs = pDlg->GetFileName();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(cs);  // @pyseemfc CFileDialog|GetFileName
}

// @pymethod string|PyCFileDialog|GetPathNames|Retrieves the list of path names from the file dialog.
// @comm This method is useful when a multi-select dialog is used.
static PyObject *ui_file_dialog_get_path_names(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetPathName);
    CFileDialog *pDlg = GetFileDialog(self);
    CString str;
    POSITION pos;

    if (!pDlg)
        return NULL;

    PyObject *newOb = PyList_New(0);
    if (!newOb)
        return NULL;
    GUI_BGN_SAVE;
    pos = pDlg->GetStartPosition();
    GUI_END_SAVE;

    while (pos) {
        GUI_BGN_SAVE;
        str = pDlg->GetNextPathName(pos);
        GUI_END_SAVE;
        PyList_Append(newOb, PyWinObject_FromTCHAR(str));
    }
    return newOb;  // @pyseemfc CFileDialog|GetPathNames
}

// @pymethod string|PyCFileDialog|GetFileExt|Retrives the file extension from the file dialog.
static PyObject *ui_file_dialog_get_file_ext(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetFileExt);
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    GUI_BGN_SAVE;
    CString csRet = pDlg->GetFileExt();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(csRet);  // @pyseemfc CFileDialog|GetFileExt
}
// @pymethod string|PyCFileDialog|GetFileTitle|Retrives the file title from the file dialog.
static PyObject *ui_file_dialog_get_file_title(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetFileTitle);
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    GUI_BGN_SAVE;
    CString csRet = pDlg->GetFileTitle();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(csRet);  // @pyseemfc CFileDialog|GetFileTitle
}
// @pymethod int|PyCFileDialog|GetReadOnlyPref|Retrives the value of the "Read Only" checkbox on the file dialog.
static PyObject *ui_file_dialog_get_ro_pref(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetReadOnlyPref);
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    GUI_BGN_SAVE;
    BOOL bRet = pDlg->GetReadOnlyPref();  // @pyseemfc CFileDialog|GetReadOnlyPref
    GUI_END_SAVE;
    return PyBool_FromLong(bRet);
}

// @pymethod |PyCFileDialog|SetOFNTitle|Sets the Title for the dialog.
static PyObject *ui_file_dialog_set_ofn_title(PyObject *self, PyObject *args)
{
    PyObject *ob;
    TCHAR *title;
    // @pyparm string|title||The title for the dialog box.  May be None.
    if (!PyArg_ParseTuple(args, "O:SetOFNTitle", &ob))
        return NULL;
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    if (!PyWinObject_AsTCHAR(ob, &title, TRUE))
        return NULL;
    PyWinObject_FreeTCHAR((TCHAR *)pDlg->m_ofn.lpstrTitle);
    pDlg->m_ofn.lpstrTitle = title;
    RETURN_NONE;
}

// @pymethod |PyCFileDialog|SetOFNInitialDir|Sets the initial directory for the dialog.
static PyObject *ui_file_dialog_set_ofn_initialdir(PyObject *self, PyObject *args)
{
    PyObject *ob;
    TCHAR *initialdir;
    // @pyparm string|title||The initial directory for the dialog box.  May be None.
    if (!PyArg_ParseTuple(args, "O:SetOFNInitialDir", &ob))
        return NULL;
    CFileDialog *pDlg = GetFileDialog(self);
    if (!pDlg)
        return NULL;
    if (!PyWinObject_AsTCHAR(ob, &initialdir, TRUE))
        return NULL;
    PyWinObject_FreeTCHAR((TCHAR *)pDlg->m_ofn.lpstrInitialDir);
    pDlg->m_ofn.lpstrInitialDir = initialdir;
    RETURN_NONE;
}

///////////////////////////////////////
//
// File Dialog Methods
//
// inherited from a Dialog
//
///////////////////////////////////////
// @object PyCFileDialog|A class which encapsulates an MFC CFileDialog object.  Derived from a <o PyCDialog> object.
static struct PyMethodDef ui_file_dialog_methods[] = {
    {"GetPathName", (PyCFunction)ui_file_dialog_get_path_name, 1},  // @pymeth GetPathName|Retrieves the path name.
    {"GetFileName", (PyCFunction)ui_file_dialog_get_file_name, 1},  // @pymeth GetFileName|Retrieves the file name.
    {"GetFileExt", (PyCFunction)ui_file_dialog_get_file_ext, 1},    // @pymeth GetFileExt|Retrieves the file extension.
    {"GetFileTitle", (PyCFunction)ui_file_dialog_get_file_title, 1},  // @pymeth GetFileTitle|Retrieves the file title.
    {"GetPathNames", (PyCFunction)ui_file_dialog_get_path_names,
     1},  // @pymeth GetPathNames|Retrieves the list of path names from the file dialog.
    {"GetReadOnlyPref", (PyCFunction)ui_file_dialog_get_ro_pref,
     1},  // @pymeth GetReadOnlyPref|Retrieves the read-only preference.
    {"SetOFNTitle", (PyCFunction)ui_file_dialog_set_ofn_title,
     1},  // @pymeth SetOFNTitle|Sets the title for the dialog.
    {"SetOFNInitialDir", (PyCFunction)ui_file_dialog_set_ofn_initialdir,
     1},  // @pymeth SetOFNInitialDir|Sets the initial directory for the dialog.
    {NULL, NULL}};

ui_type_CObject PyCFileDialog::type("PyCFileDialog",
                                    &PyCCommonDialog::type,  // @base PyCFileDialog|PyCCommonDialog
                                    RUNTIME_CLASS(CFileDialog), sizeof(PyCFileDialog), PYOBJ_OFFSET(PyCFileDialog),
                                    ui_file_dialog_methods, GET_PY_CTOR(PyCFileDialog));

/////////////////////////////////////////////////////////////////////
//
// Font Dialog object
//
//////////////////////////////////////////////////////////////////////
PyCFontDialog::PyCFontDialog() { pInitLogFont = NULL; }
PyCFontDialog::~PyCFontDialog() { delete pInitLogFont; }

// @pymethod <o PyCFontDialog>|win32ui|CreateFontDialog|Creates a font selection dialog box.
PyObject *PyCFontDialog::ui_font_dialog_create(PyObject * /*self*/, PyObject *args)
{
    PyObject *obFont = Py_None;
    DWORD flags = CF_EFFECTS | CF_SCREENFONTS;
    PyObject *obDC = Py_None;
    PyObject *obParent = Py_None;
    CWnd *pParent = NULL;  // should mean same as GetApp()->m_pMainWnd
    LOGFONT *pFont = NULL;
    CDC *pDC = NULL;
    CHARFORMAT cf;
    memset(&cf, 0, sizeof(cf));

    cf.cbSize = sizeof(CHARFORMAT);

    if (!PyArg_ParseTuple(
            args, "|OiOO:CreateFontDialog",
            &obFont,     // @pyparm dict/tuple|font|None|A dictionary describing a LOGFONT, or a tuple describing a
                         // CHARFORMAT.
            &flags,      // @pyparm int|flags|win32con.CF_EFFECTS\|win32con.CF_SCREENFONTS|The choose-font flags to use.
            &obDC,       // @pyparm <o PyCDC>|dcPrinter|None|Show fonts available for the specified device.
            &obParent))  // @pyparm <o PyCWnd>|parent|None|The parent or owner window of the dialog.
        return NULL;

    if (obParent != Py_None) {
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("Parent arg must be a PyCWnd object");
        if (!(pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent)))
            return NULL;
    }
    BOOL bHaveCF = FALSE;
    if (obFont != Py_None) {
        if (PyTuple_Check(obFont)) {
            if (!ParseCharFormatTuple(obFont, &cf))
                return NULL;
            bHaveCF = TRUE;
        }
        else if (PyMapping_Check(obFont)) {
            pFont = new LOGFONT;
            if (!DictToLogFont(obFont, pFont))
                return NULL;
        }
        else {
            RETURN_ERR("Unknown object type for font object");
        }
    }
    CFontDialog *pDlg;
    if (bHaveCF)
        pDlg = new CFontDialog(cf, flags, pDC, pParent);
    else
        pDlg = new CFontDialog(pFont, flags, pDC, pParent);

    if (!pDlg) {
        delete pFont;
        RETURN_ERR("Creating CFontDialog failed");  // pyseemfc CFontDialog|CFontDialog
    }
    PyCFontDialog *newObj = (PyCFontDialog *)ui_assoc_object::make(PyCFontDialog::type, pDlg, TRUE);
    if (newObj && pFont)
        newObj->pInitLogFont = pFont;
    else
        delete pFont;  // may be NULL, but thats OK!
    return newObj;
}

#define MAKE_CSTRING_METH(fnname, mfcName)                  \
    static PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                       \
        CHECK_NO_ARGS2(args, mfcName);                      \
        CFontDialog *pDlg = GetFontDialog(self);            \
        if (!pDlg)                                          \
            return NULL;                                    \
        GUI_BGN_SAVE;                                       \
        CString ret = pDlg->mfcName();                      \
        GUI_END_SAVE;                                       \
        return PyWinObject_FromTCHAR(ret);                  \
    }

#define MAKE_INT_METH(fnname, mfcName)                      \
    static PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                       \
        CHECK_NO_ARGS2(args, mfcName);                      \
        CFontDialog *pDlg = GetFontDialog(self);            \
        if (!pDlg)                                          \
            return NULL;                                    \
        GUI_BGN_SAVE;                                       \
        int ret = pDlg->mfcName();                          \
        GUI_END_SAVE;                                       \
        return PyInt_FromLong(ret);                         \
    }

#define MAKE_INT_PTR_METH(fnname, mfcName)                  \
    static PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                       \
        CHECK_NO_ARGS2(args, mfcName);                      \
        CFontDialog *pDlg = GetFontDialog(self);            \
        if (!pDlg)                                          \
            return NULL;                                    \
        GUI_BGN_SAVE;                                       \
        INT_PTR ret = pDlg->mfcName();                      \
        GUI_END_SAVE;                                       \
        return PyWinObject_FromDWORD_PTR(ret);              \
    }

// @pymethod string|PyCFontDialog|GetFaceName|Returns the face name of the selected font.
// @pyseemfc CFontDialog|GetFaceName
MAKE_CSTRING_METH(ui_font_dialog_get_face_name, GetFaceName)
// @pymethod string|PyCFontDialog|GetStyleName|Returns the style name of the selected font.
// @pyseemfc CFontDialog|GetStyleName
MAKE_CSTRING_METH(ui_font_dialog_get_style_name, GetStyleName)

// @pymethod int|PyCFontDialog|GetSize|Returns he font's size, in tenths of a point.
// @pyseemfc CFontDialog|GetSize
MAKE_INT_METH(ui_font_dialog_get_size, GetSize)

// @pymethod int|PyCFontDialog|GetWeight|Returns the font's weight.
// @pyseemfc CFontDialog|GetWeight
MAKE_INT_METH(ui_font_dialog_get_weight, GetWeight)

// @pymethod int|PyCFontDialog|IsStrikeOut|Determines whether the font is displayed with strikeout.
// @pyseemfc CFontDialog|IsStrikeOut
MAKE_INT_METH(ui_font_dialog_is_strikeout, IsStrikeOut)

// @pymethod int|PyCFontDialog|IsUnderline|Determines whether the font is displayed with underline.
// @pyseemfc CFontDialog|IsUnderline
MAKE_INT_METH(ui_font_dialog_is_underline, IsUnderline)

// @pymethod int|PyCFontDialog|IsBold|Determines whether the font is displayed bold.
// @pyseemfc CFontDialog|IsBold
MAKE_INT_METH(ui_font_dialog_is_bold, IsBold)

// @pymethod int|PyCFontDialog|IsItalic|Determines whether the font is displayed with italic.
// @pyseemfc CFontDialog|IsItalic
MAKE_INT_METH(ui_font_dialog_is_italic, IsItalic)

// @pymethod int|PyCFontDialog|DoModal|Displays a dialog and allows the user to make a selection.
// @pyseemfc CFontDialog|DoModal
MAKE_INT_PTR_METH(ui_font_dialog_do_modal, DoModal)

// @pymethod int|PyCFontDialog|GetColor|Determines the color of the selected font.
// @pyseemfc CFontDialog|GetColor
MAKE_INT_METH(ui_font_dialog_get_color, GetColor)

// @pymethod dict|PyCFontDialog|GetCurrentFont|Returns a dictionary describing the current font.
static PyObject *ui_font_dialog_get_current_font(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetCurrentFont);
    CFontDialog *pDlg = GetFontDialog(self);
    if (!pDlg)
        return NULL;
    if (pDlg->m_hWnd == NULL) {
        return LogFontToDict(pDlg->m_lf);
    }
    else {
        LOGFONT lf;
        GUI_BGN_SAVE;
        pDlg->GetCurrentFont(&lf);
        GUI_END_SAVE;
        // @pyseemfc CFontDialog|GetCurrentFont
        return LogFontToDict(lf);
    }
}

// @pymethod tuple|PyCFontDialog|GetCharFormat|Returns the font selection in a CHARFORMAT tuple.
static PyObject *ui_font_dialog_get_char_format(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetCharFormat);
    CFontDialog *pDlg = GetFontDialog(self);
    if (!pDlg)
        return NULL;

    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));

    GUI_BGN_SAVE;
    pDlg->GetCharFormat(fmt);
    GUI_END_SAVE;
    // @pyseemfc CFontDialog|GetCharFormat
    return MakeCharFormatTuple(&fmt);
}

// @object PyCFontDialog|A class which encapsulates an MFC CFontDialog object.  Derived from a <o PyCDialog> object.
static struct PyMethodDef ui_font_dialog_methods[] = {
    {"DoModal", ui_font_dialog_do_modal,
     1},  // @pymeth DoModal|Displays a dialog and allows the user to make a selection.
    {"GetCurrentFont", ui_font_dialog_get_current_font,
     1},  // @pymeth GetCurrentFont|Returns a dictionary describing the current font.
    {"GetCharFormat", ui_font_dialog_get_char_format,
     1},  // @pymeth GetCharFormat|Returns the font selection in a CHARFORMAT tuple.
    {"GetColor", ui_font_dialog_get_color, 1},  // @pymeth GetColor|Determines the color of the selected font.
    {"GetFaceName", ui_font_dialog_get_face_name,
     1},  // @pymeth GetFaceName|Returns the face name of the selected font.
    {"GetStyleName", ui_font_dialog_get_style_name,
     1},                                          // @pymeth GetStyleName|Returns the style name of the selected font.
    {"GetSize", ui_font_dialog_get_size, 1},      // @pymeth GetSize|Returns he font's size, in tenths of a point.
    {"GetWeight", ui_font_dialog_get_weight, 1},  // @pymeth GetWeight|Returns the font's weight.
    {"IsStrikeOut", ui_font_dialog_is_strikeout,
     1},  // @pymeth IsStrikeOut|Determines whether the font is displayed with strikeout.
    {"IsUnderline", ui_font_dialog_is_underline,
     1},  // @pymeth IsUnderline|Determines whether the font is displayed with underline.
    {"IsBold", ui_font_dialog_is_bold, 1},  // @pymeth IsBold|Determines whether the font is displayed bold.
    {"IsItalic", ui_font_dialog_is_italic,
     1},  // @pymeth IsItalic|Determines whether the font is displayed with italic.
    {NULL, NULL}};

ui_type_CObject PyCFontDialog::type("PyCFontDialog",
                                    &PyCCommonDialog::type,  // @base PyCFontDialog|PyCCommonDialog
                                    RUNTIME_CLASS(CFontDialog), sizeof(PyCFontDialog), PYOBJ_OFFSET(PyCFontDialog),
                                    ui_font_dialog_methods, GET_PY_CTOR(PyCFontDialog));

/////////////////////////////////////////////////////////////////////
//
// Color Dialog object
//
//////////////////////////////////////////////////////////////////////
PyCColorDialog::PyCColorDialog() {}
PyCColorDialog::~PyCColorDialog() {}

// @pymethod <o PyCColorDialog>|win32ui|CreateColorDialog|Creates a color selection dialog box.
PyObject *PyCColorDialog::create(PyObject * /*self*/, PyObject *args)
{
    int color = 0;
    DWORD flags = 0;
    PyObject *obParent = Py_None;
    CWnd *pParent = NULL;  // should mean same as GetApp()->m_pMainWnd

    if (!PyArg_ParseTuple(args, "|iiO:CreateColorDialog",
                          &color,      // @pyparm int|initColor|0|The initial color.
                          &flags,      // @pyparm int|flags|0|The choose-color flags to use.
                          &obParent))  // @pyparm <o PyCWnd>|parent|None|The parent or owner window of the dialog.
        return NULL;

    if (obParent != Py_None) {
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("Parent arg must be a PyCWnd object");
        if (!(pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent)))
            return NULL;
    }
    CColorDialog *pDlg = new CColorDialog(color, flags, pParent);
    if (!pDlg) {
        RETURN_ERR("Creating CColorDialog failed");  // pyseemfc CColorDialog|CColorDialog
    }
    PyCColorDialog *newObj = (PyCColorDialog *)ui_assoc_object::make(PyCColorDialog::type, pDlg, TRUE);
    return newObj;
}

#undef MAKE_INT_METH
#define MAKE_INT_METH(fnname, mfcName)                      \
    static PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                       \
        CHECK_NO_ARGS2(args, mfcName);                      \
        CColorDialog *pDlg = GetColorDialog(self);          \
        if (!pDlg)                                          \
            return NULL;                                    \
        GUI_BGN_SAVE;                                       \
        int ret = pDlg->mfcName();                          \
        GUI_END_SAVE;                                       \
        return Py_BuildValue("i", ret);                     \
    }

// @pymethod int|PyCColorDialog|DoModal|Displays a dialog and allows the user to make a selection.
// @pyseemfc CColorDialog|DoModal
MAKE_INT_PTR_METH(ui_color_dialog_do_modal, DoModal)

// @pymethod int|PyCColorDialog|GetColor|Determines the selected color.
// @pyseemfc CColorDialog|GetColor
MAKE_INT_METH(ui_color_dialog_get_color, GetColor)

// @pymethod int|PyCColorDialog|GetSavedCustomColors|Returns the saved custom colors.
static PyObject *ui_color_dialog_get_saved_custom_colors(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, GetSavedCustomColors);
    CColorDialog *pDlg = GetColorDialog(self);
    if (!pDlg)
        return NULL;
    // @pyseemfc CColorDialog|GetSavedCustomColors
    GUI_BGN_SAVE;
    COLORREF *prc = pDlg->GetSavedCustomColors();
    GUI_END_SAVE;
    return PyInt_FromLong((long)*prc);
}

// @pymethod |PyCColorDialog|SetCurrentColor|Sets the currently selected color.
static PyObject *ui_color_dialog_set_current_color(PyObject *self, PyObject *args)
{
    int color;
    // @pyparm int|color||The color to set.
    if (!PyArg_ParseTuple(args, "i", &color))
        return NULL;
    CColorDialog *pDlg = GetColorDialog(self);
    if (!pDlg)
        return NULL;
    // @pyseemfc CColorDialog|SetCurrentColor
    GUI_BGN_SAVE;
    pDlg->SetCurrentColor((COLORREF)color);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod (int,...)|PyCColorDialog|GetCustomColors|Gets the 16 currently defined custom colors
static PyObject *ui_color_dialog_get_custom_colors(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    CColorDialog *pDlg = GetColorDialog(self);
    if (!pDlg)
        return NULL;
    PyObject *ret = PyTuple_New(16);
    for (int i = 0; i < 16; i++) PyTuple_SET_ITEM(ret, i, PyInt_FromLong(pDlg->m_cc.lpCustColors[i]));
    return ret;
}

// @pymethod |PyCColorDialog|SetCustomColors|Sets one or more custom colors
static PyObject *ui_color_dialog_set_custom_colors(PyObject *self, PyObject *args)
{
    PyObject *obCols;
    if (!PyArg_ParseTuple(args, "O", &obCols))
        return NULL;
    Py_ssize_t len = PySequence_Length(obCols);
    if (PyErr_Occurred() || len <= 0 || len > 16) {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "The argument must be a sequence of integers of length 1-16");
        return NULL;
    }
    CColorDialog *pDlg = GetColorDialog(self);
    if (!pDlg)
        return NULL;
    for (int i = 0; i < len; i++) {
        PyObject *obInt = NULL;
        PyObject *ob = PySequence_GetItem(obCols, i);
        if (ob != NULL)
            obInt = PyNumber_Int(ob);
        if (obInt == NULL) {
            Py_XDECREF(ob);
            PyErr_SetString(PyExc_TypeError, "The argument must be a sequence of integers of length 1-16");
            return NULL;
        }
        pDlg->m_cc.lpCustColors[i] = PyInt_AsLong(obInt);
        Py_DECREF(ob);
        Py_DECREF(obInt);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyCColorDialog|A class which encapsulates an MFC CColorDialog object.  Derived from a <o PyCDialog> object.
static struct PyMethodDef ui_color_dialog_methods[] = {
    {"GetColor", ui_color_dialog_get_color, 1},  // @pymeth GetColor|Determines the selected color.
    {"DoModal", ui_color_dialog_do_modal,
     1},  // @pymeth DoModal|Displays a dialog and allows the user to make a selection.
    {"GetSavedCustomColors", ui_color_dialog_get_saved_custom_colors,
     1},  // @pymeth GetSavedCustomColors|Returns the saved custom colors.
    {"SetCurrentColor", ui_color_dialog_set_current_color,
     1},  // @pymeth SetCurrentColor|Sets the currently selected color.
    {"SetCustomColors", ui_color_dialog_set_custom_colors,
     1},  // @pymeth SetCustomColors|Sets one or more custom colors
    {"GetCustomColors", ui_color_dialog_get_custom_colors,
     1},  // @pymeth GetCustomColors|Gets the currently defined custom colors.
    {NULL, NULL}};

ui_type_CObject PyCColorDialog::type("PyCColorDialog",
                                     &PyCCommonDialog::type,  // @base PyCColorDialog|PyCCommonDialog
                                     RUNTIME_CLASS(CColorDialog), sizeof(PyCColorDialog), PYOBJ_OFFSET(PyCColorDialog),
                                     ui_color_dialog_methods, GET_PY_CTOR(PyCColorDialog));

/////////////////////////////////////////////////////////////////////
//
// Print Dialog object
//
//////////////////////////////////////////////////////////////////////
// Derived CPrintDialog class

PyCPrintDialog::PyCPrintDialog() {}
PyCPrintDialog::~PyCPrintDialog() {}

// @pymethod <o PyCPrintDialog>|win32ui|CreatePrintDialog|Creates a print dialog object.
PyObject *PyCPrintDialog::create(PyObject *self, PyObject *args)
{
    int idRes;
    BOOL bPrintSetupOnly = FALSE;
    DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION;
    PyObject *obParent = NULL;
    CWnd *pParentWnd = NULL;
    HINSTANCE hMod = NULL, hOldRes = NULL;
    PyObject *obDLL = NULL;
    if (!PyArg_ParseTuple(
            args, "i|iiOO:CreatePrintDialog",
            &idRes,            // @pyparm int|idRes||The ID of the dialog resource to load.
            &bPrintSetupOnly,  // @pyparm int|bPrintSetupOnly|FALSE|Specifies whether the standard Windows Print dialog
                               // box or Print Setup dialog box is displayed.
            &dwFlags,          // @pyparm
                       // int|dwFlags|PD_ALLPAGES\|PD_USEDEVMODECOPIES\|PD_NOPAGENUMS\|PD_HIDEPRINTTOFILE\|PD_NOSELECTION|One
                       // or more flags you can use to customize the settings of the dialog box, combined using the
                       // bitwise OR operator.
            &obParent,  // @pyparm <o PyCWnd>|parent|None|A pointer to the dialog box parent or owner window.
            &obDLL))    // @pyparm <o PyDLL>|dll|None|The DLL object to load the dialog from.
        return NULL;
    if (obDLL && obDLL != Py_None) {
        // passed a DLL object.
        if (!ui_base_class::is_uiobject(obDLL, &dll_object::type))
            RETURN_TYPE_ERR("passed object must be a PyDLL");
        hMod = ((dll_object *)obDLL)->GetDll();
        if (hMod == NULL)
            RETURN_ERR("Can not load from an uninitialised DLL object");
    }
    if (obParent && obParent != Py_None) {
        // passed a PyCWnd object.
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("passed object must be a PyCWnd object");
        pParentWnd = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent);
        if (!pParentWnd)
            return NULL;
    }
    if (hMod == NULL)
        hMod = AfxFindResourceHandle(MAKEINTRESOURCE(idRes), RT_DIALOG);
    else {
        hOldRes = AfxGetResourceHandle();
        AfxSetResourceHandle(hMod);
    }

    HGLOBAL hGlob;
    HRSRC hrsrc;
    hrsrc = ::FindResourceEx(hMod, RT_DIALOG, MAKEINTRESOURCE(idRes), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hrsrc)
        hGlob = ::LoadResource(hMod, hrsrc);
    if (hOldRes)
        AfxSetResourceHandle(hOldRes);
    if (hrsrc == NULL)
        RETURN_API_ERR("FindResourceEx");
    if (hGlob == NULL)
        RETURN_API_ERR("LoadResource");

    GUI_BGN_SAVE;
    CPrintDialog *pDlg = new CPythonPrtDlg(bPrintSetupOnly, dwFlags, pParentWnd);
    GUI_END_SAVE;
    PyCPrintDialog *ret = (PyCPrintDialog *)ui_assoc_object::make(PyCPrintDialog::type, pDlg, TRUE);
    if (ret) {
        ret->hTemplate = hGlob;
        ret->hInstance = hMod;
    }
    return ret;
}

#undef MAKE_INT_METH
#define MAKE_INT_METH(fnname, mfcName)                      \
    static PyObject *fnname(PyObject *self, PyObject *args) \
    {                                                       \
        CHECK_NO_ARGS2(args, mfcName);                      \
        CPrintDialog *pDlg = GetPrintDialog(self);          \
        if (!pDlg)                                          \
            return NULL;                                    \
        GUI_BGN_SAVE;                                       \
        int ret = pDlg->mfcName();                          \
        GUI_END_SAVE;                                       \
        return Py_BuildValue("i", ret);                     \
    }

// @object PyCPrintDialog|An object which encapsulates an MFC CPrintDialog object.
// @base PyCPrintDialog|PyCCommonDialog
static struct PyMethodDef ui_print_dialog_methods[] = {{NULL, NULL}};

ui_type_CObject PyCPrintDialog::type("PyCPrintDialog",
                                     &PyCCommonDialog::type,  // @base PyCPrintDialog|PyCCommonDialog
                                     RUNTIME_CLASS(CPrintDialog), sizeof(PyCPrintDialog), PYOBJ_OFFSET(PyCPrintDialog),
                                     ui_print_dialog_methods, GET_PY_CTOR(PyCPrintDialog));
