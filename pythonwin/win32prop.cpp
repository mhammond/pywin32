/*

    propsheet data type

    Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

    propsheet is derived from window.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32dlg.h"
#include "win32prop.h"
#include <afxcmn.h>

#include "pythonpsheet.h"
#include "pythonppage.h"
#include "reswin32ui.h"

extern HGLOBAL MakeResourceFromDlgList(PyObject *tmpl);

CTabCtrl *PyGetTabCtrl(PyObject *self)
{
    return (CTabCtrl *)PyCWnd::GetPythonGenericWnd(self, &ui_tabctrl_object::type);
}
CTabCtrl *PyGetTabCtrlWithWnd(PyObject *self)
{
    CTabCtrl *pTab = PyGetTabCtrl(self);
    if (pTab->m_hWnd == NULL || !::IsWindow(pTab->m_hWnd))
        RETURN_ERR("The tab control has no window");
    return pTab;
}

//
// property sheet helpers
//

CPropertySheet *GetPropSheet(PyObject *self)
{
    return (CPropertySheet *)PyCWnd::GetPythonGenericWnd(self, &PyCPropertySheet::type);
}
CPythonPropertySheet *GetPythonPropSheet(PyObject *self)
{
    CPythonPropertySheet *ret = (CPythonPropertySheet *)PyCWnd::GetPythonGenericWnd(self, &PyCPropertySheet::type);
    if (!ret->IsKindOf(RUNTIME_CLASS(CPythonPropertySheet)))
        RETURN_TYPE_ERR("Object is not of the correct type");
    return ret;
}
//
// property page helpers
//
CPythonPropertyPage *GetPropPage(PyObject *self)
{
    return (CPythonPropertyPage *)PyCWnd::GetPythonGenericWnd(self, &PyCPropertyPage::type);
}

BOOL PropSheetCheckForPageCreate(UINT id)
{
    //	if (!CPythonPropertyPage::CheckTemplate(id))
    //		RETURN_ERR("The property page can not be located");
    return TRUE;
}
BOOL PropSheetCheckForPageCreate(LPCTSTR id)
{
    //	if (!CPythonPropertyPage::CheckTemplate(id))
    //		RETURN_ERR("The property page can not be located");
    return TRUE;
}

BOOL PropSheetCheckForDisplay(CPropertySheet *pSheet)
{
    int max = pSheet->GetPageCount();
    if (max == 0)
        RETURN_ERR("The property sheet has no pages");
    //	for (int i=0; i<max; i++) {
    //		CPythonPropertyPage *pPage = (CPythonPropertyPage *)pSheet->GetPage(i);
    // use help ID here
    //		if (!pPage->CheckTemplate()) {
    //			char buf[80];
    //			sprintf(buf, "Property Page %d can not be located", i );
    //			RETURN_ERR(buf);
    //		}
    //	}
    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// PropSheet object
//
//////////////////////////////////////////////////////////////////////
PyCPropertySheet::PyCPropertySheet()
{
    //	bManualDelete = TRUE;
}
PyCPropertySheet::~PyCPropertySheet() {}
// @pymethod <o PyCPropertySheet>|win32ui|CreatePropertySheet|Creates a property sheet object.
PyObject *PyCPropertySheet::create(PyObject *self, PyObject *args)
{
    PyObject *obParent = NULL, *obCaption;
    TCHAR *Caption;
    CWnd *pParent = NULL;
    int iSelect = 0;
    if (!PyArg_ParseTuple(
            args, "O|Oi",
            &obCaption,  // @pyparm <o PyResourceId>|caption||The caption for the property sheet, or id of the caption
            &obParent,   // @pyparm <o PyCWnd>|parent|None|The parent window of the property sheet.
            &iSelect))   // @pyparm int|select|0|The index of the first page to be selected.
        return NULL;
    if (obParent) {
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("parameter 2 must be a PyCWnd object");
        pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent);
    }
    CPythonPropertySheet *pPS;
    if (!PyWinObject_AsResourceId(obCaption, &Caption, FALSE))
        return NULL;

    if (IS_INTRESOURCE(Caption)) {
        GUI_BGN_SAVE;
        pPS = new CPythonPropertySheet(MAKEINTRESOURCE(Caption), pParent, iSelect);
        GUI_END_SAVE;
    }
    else {
        GUI_BGN_SAVE;
        pPS = new CPythonPropertySheet(Caption, pParent, iSelect);
        GUI_END_SAVE;
    }
    PyWinObject_FreeResourceId(Caption);
    PyCPropertySheet *ret = (PyCPropertySheet *)ui_assoc_object::make(PyCPropertySheet::type, pPS);
    return ret;
}

///////////////////////////////////////
//
// PropSheet Methods
//
// @pymethod |PyCPropertySheet|AddPage|Adds the supplied page with the rightmost tab in the property sheet.
PyObject *ui_propsheet_add_page(PyObject *self, PyObject *args)
{
    PyObject *obPage;
    CPythonPropertyPage *pPage;
    if (!PyArg_ParseTuple(args, "O", &obPage))
        // @pyparm <o PyCPropertyPage>|page||The page to be added.
        return NULL;
    if (!ui_base_class::is_uiobject(obPage, &PyCPropertyPage::type)) {
        RETURN_TYPE_ERR("passed object must be a PyCPropertyPage object");
    }
    pPage = GetPropPage(obPage);
    if (!pPage)
        return NULL;

    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    pPS->AddPage(pPage);  // @pyseemfc PyCPropertySheet|AddPage
    GUI_END_SAVE;
    // @comm Add pages to the property sheet in the left-to-right order you want them to appear.
    RETURN_NONE;
}
// @pymethod int|PyCPropertySheet|GetActiveIndex|Retrieves the index of the active page of the property sheet.
PyObject *ui_propsheet_get_active_index(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPS->GetActiveIndex();
    GUI_END_SAVE;

    return Py_BuildValue("i", rc);
}

// @pymethod int|PyCPropertySheet|GetPageIndex|Retrieves the index of the specified page of the property sheet.
PyObject *ui_propsheet_get_page_index(PyObject *self, PyObject *args)
{
    PyObject *obPage;
    // @pyparm <o PyCPropertyPage>|page||The page.
    if (!PyArg_ParseTuple(args, "O:GetPageIndex", &obPage))
        return NULL;
    CPythonPropertySheet *pPS = GetPythonPropSheet(self);
    if (!pPS)
        return NULL;
    CPythonPropertyPage *pPage = GetPropPage(obPage);
    if (!pPage)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPS->GetPageIndex(pPage);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod <o PyCPropertyPage>|PyCPropertySheet|EnableStackedTabs|Enables or disables stacked tabs.
PyObject *ui_propsheet_enable_stacked_tabs(PyObject *self, PyObject *args)
{
    BOOL stacked;
    if (!PyArg_ParseTuple(args, "i", &stacked))
        // @pyparm int|stacked||A boolean flag
        return NULL;
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    pPS->EnableStackedTabs(stacked);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod <o PyCPropertyPage>|PyCPropertySheet|GetPage|Returns the specified property page.
PyObject *ui_propsheet_get_page(PyObject *self, PyObject *args)
{
    int pagenum;
    CPropertyPage *pPage;
    if (!PyArg_ParseTuple(args, "i", &pagenum))
        // @pyparm int|pageNo||The index of the page toretrieve.
        return NULL;
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    pPage = pPS->GetPage(pagenum);
    GUI_END_SAVE;
    if (!pPage)
        RETURN_ERR("The page does not exist");
    // @pyseemfc PyCPropertySheet|GetPage
    PyCPropertyPage *ret = (PyCPropertyPage *)ui_assoc_object::make(PyCPropertyPage::type, pPage)->GetGoodRet();
    return ret;
}

// @pymethod <o PyCPropertyPage>|PyCPropertySheet|GetActivePage|Returns the currently active property page.
PyObject *ui_propsheet_get_active_page(PyObject *self, PyObject *args)
{
    CPropertyPage *pPage;
    if (!PyArg_ParseTuple(args, ":GetActivePage"))
        return NULL;
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    pPage = pPS->GetActivePage();
    GUI_END_SAVE;
    if (!pPage)
        RETURN_ERR("The page does not exist");
    // @pyseemfc PyCPropertySheet|GetActivePage
    PyCPropertyPage *ret = (PyCPropertyPage *)ui_assoc_object::make(PyCPropertyPage::type, pPage)->GetGoodRet();
    return ret;
}

// @pymethod |PyCPropertySheet|SetActivePage|Programmatically sets the active page object.
PyObject *ui_propsheet_set_active_page(PyObject *self, PyObject *args)
{
    PyObject *obPage;
    // @pyparm <o PyCPropertyPage>|page||The page.
    if (!PyArg_ParseTuple(args, "O:SetActivePage", &obPage))
        return NULL;
    CPythonPropertySheet *pPS = GetPythonPropSheet(self);
    if (!pPS)
        return NULL;
    CPythonPropertyPage *pPage = GetPropPage(obPage);
    if (!pPage)
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pPS->SetActivePage(pPage);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetActivePage failed");
    RETURN_NONE;
}

// @pymethod int|PyCPropertySheet|GetPageCount|Returns the number of pages.
PyObject *ui_propsheet_get_page_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pPS->GetPageCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod <o PyCTabCtrl>|PyCPropertySheet|GetTabCtrl|Returns the tab control used by the sheet.
PyObject *ui_propsheet_get_tab_ctrl(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    GUI_BGN_SAVE;
    CTabCtrl *pTab = pPS->GetTabControl();
    GUI_END_SAVE;
    if (pTab == NULL)
        RETURN_ERR("The property page does not have a tab control");
    return ui_assoc_object::make(ui_tabctrl_object::type, pTab)->GetGoodRet();
}

// @pymethod int|PyCPropertySheet|DoModal|Displays the property sheet as a modal dialog.
PyObject *ui_propsheet_do_modal(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CPropertySheet *pPS;
    if (!(pPS = GetPropSheet(self)))
        return NULL;
    if (!PropSheetCheckForDisplay(pPS))
        return NULL;
    Py_INCREF(self);  // make sure Python doesn't kill the object while in a modal call.
                      // really only for the common dialog(!?), and other non CPythonPropSheet's
    INT_PTR ret;
    GUI_BGN_SAVE;
    ret = pPS->DoModal();
    GUI_END_SAVE;
    Py_DECREF(self);
    return PyWinObject_FromDWORD_PTR(ret);
}

// @pymethod |PyCPropertySheet|CreateWindow|Displays the property sheet as a modeless dialog.
PyObject *ui_propsheet_create_window(PyObject *self, PyObject *args)
{
    PyObject *obParent = NULL;
    int dwStyle = WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | WS_VISIBLE;
    int dwExStyle = WS_EX_DLGMODALFRAME;
    CWnd *pParent = NULL;
    // @pyparm <o PyCWnd>|parent|None|The parent of the dialog.
    // @pyparm int|style|WS_SYSMENU\|WS_POPUP\|WS_CAPTION\|DS_MODALFRAME\|WS_VISIBLE|The style for the window.
    // @pyparm int|exStyle|WS_EX_DLGMODALFRAME|The extended style for the window.
    if (!PyArg_ParseTuple(args, "|Oll", &obParent, &dwStyle, &dwExStyle))
        return NULL;
    if (obParent && obParent != Py_None) {
        if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
            RETURN_TYPE_ERR("parameter 1 must be a PyCWnd object");
        pParent = (CWnd *)PyCWnd::GetPythonGenericWnd(obParent);
        if (!pParent)
            return NULL;
    }
    CPythonPropertySheet *pPS;
    if (!(pPS = GetPythonPropSheet(self)))
        return NULL;
    if (!PropSheetCheckForDisplay(pPS))
        return NULL;
    int rc;
    const char *failMsg = "Create() failed";
    GUI_BGN_SAVE;
    try {
        rc = pPS->Create(pParent, dwStyle, dwExStyle);
    }
    catch (...) {
        rc = NULL;
        failMsg = "Create() caused an exception - it is likely that the specified template can not be located";
    }
    GUI_END_SAVE;
    if (!rc)
        RETURN_ERR((char *)failMsg);
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|EndDialog|Closes the dialog, with the specified result.
PyObject *ui_propsheet_end_dialog(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    int result;
    // @pyparm int|result||The result to be returned by DoModal.
    if (!PyArg_ParseTuple(args, "i", &result))
        return NULL;
    GUI_BGN_SAVE;
    pPS->EndDialog(result);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertySheet|RemovePage|Removes the specified page from the sheet.
PyObject *ui_propsheet_remove_page(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS;
    if (!(pPS = GetPropSheet(self)))
        return NULL;
    PyObject *ob;
    // @pyparm int|offset||The page number to remove
    // @pyparmalt1 <o PyCPropertyPage>|page||The page to remove
    if (!PyArg_ParseTuple(args, "O", &ob))
        return NULL;
    if (PyLong_Check(ob)) {
        int id = (int)PyLong_AsLong(ob);
        GUI_BGN_SAVE;
        pPS->RemovePage(id);
        GUI_END_SAVE;
    }
    else if (ui_base_class::is_uiobject(ob, &PyCPropertyPage::type)) {
        CPythonPropertyPage *pPage = GetPropPage(ob);
        if (!pPage)
            return NULL;
        GUI_BGN_SAVE;
        pPS->RemovePage(pPage);
        GUI_END_SAVE;
    }
    else
        RETURN_TYPE_ERR("passed object must be an integer or PyCPropertyPage object");
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|SetTitle|Sets the caption for the property sheet.
PyObject *ui_propsheet_set_title(PyObject *self, PyObject *args)
{
    TCHAR *caption;
    PyObject *obcaption;
    // @pyparm string|title||The new caption
    if (!PyArg_ParseTuple(args, "O:SetTitle", &obcaption))
        return NULL;
    CPythonPropertySheet *pPS = GetPythonPropSheet(self);
    if (!pPS)
        return NULL;
    if (!PyWinObject_AsTCHAR(obcaption, &caption, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pPS->SetTitle(caption);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(caption);
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|PressButton|Simulates the choice of the specified button in a property sheet.
PyObject *ui_propsheet_press_button(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    int button;
    // @pyparm int|button||The button to press
    if (!PyArg_ParseTuple(args, "i", &button))
        return NULL;
    GUI_BGN_SAVE;
    pPS->PressButton(button);
    GUI_END_SAVE;

    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|SetWizardButtons|Enables the wizard buttons
PyObject *ui_propsheet_set_wizard_buttons(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    int flags;
    // @pyparm int|flags||The wizard flags
    if (!PyArg_ParseTuple(args, "i", &flags))
        return NULL;
    GUI_BGN_SAVE;
    pPS->SetWizardButtons(flags);
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|SetWizardMode|Enables the wizard mode
PyObject *ui_propsheet_set_wizard_mode(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    CHECK_NO_ARGS2(args, SetWizardMode);
    GUI_BGN_SAVE;
    pPS->SetWizardMode();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|SetFinishText|Sets the text for the Finish button
PyObject *ui_propsheet_set_finish_text(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    TCHAR *text;
    PyObject *obtext;
    // @pyparm string|text||The next for the button
    if (!PyArg_ParseTuple(args, "O", &obtext))
        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pPS->SetFinishText(text);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    RETURN_NONE;
}

// @pymethod |PyCPropertySheet|SetPSHBit|Sets or clears a bit in m_psh.dwFlags
PyObject *ui_propsheet_set_pshbit(PyObject *self, PyObject *args)
{
    CPropertySheet *pPS = pPS = GetPropSheet(self);
    if (!pPS)
        return NULL;
    DWORD bitMask = 0;
    BOOL bitValue = 0;

    if (!PyArg_ParseTuple(args, "ii",
                          &bitMask,    // @pyparm int|bitMask||The PSH_* bit mask constant
                          &bitValue))  // @pyparm int|bitValue||1 to set, 0 to clear
        return NULL;
    GUI_BGN_SAVE;
    if (bitValue) {
        pPS->m_psh.dwFlags |= bitMask;
    }
    else {
        pPS->m_psh.dwFlags &= ~bitMask;
    }
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCPropertySheet|OnInitDialog|Calls the default MFC OnInitDialog handler.
static PyObject *ui_propsheet_on_init_dialog(PyObject *self, PyObject *args)
{
    class HackProtected : public CPythonPropertySheet {
       public:
        BOOL BaseOnInitDialog(void) { return CPropertySheet::OnInitDialog(); }
    };
    CPythonPropertySheet *pPS = GetPythonPropSheet(self);
    if (pPS == NULL)
        return NULL;
    CHECK_NO_ARGS2(args, OnInitDialog);
    GUI_BGN_SAVE;
    BOOL rc = ((HackProtected *)pPS)->BaseOnInitDialog();
    GUI_END_SAVE;
    // @xref <vm PyCPropertySheet::OnInitDialog>
    return Py_BuildValue("i", rc);
}

// inherited from window
//
///////////////////////////////////////
// @object PyCPropertySheet|A class which encapsulates an MFC CPropertySheet object.  Derived from a <o PyCWnd> object.
static struct PyMethodDef ui_propsheet_methods[] = {
    {"AddPage", ui_propsheet_add_page,
     1},  // @pymeth AddPage|Adds the supplied page with the rightmost tab in the property sheet.
    {"CreateWindow", ui_propsheet_create_window,
     1},                                    // @pymeth CreateWindow|Displays the property sheet as a modeless dialog.
    {"DoModal", ui_propsheet_do_modal, 1},  // @pymeth DoModal|Displays the property sheet as a modal dialog.
    {"EnableStackedTabs", ui_propsheet_enable_stacked_tabs,
     1},                                        // @pymeth EnableStackedTabs|Enables or disables stacked tabs.
    {"EndDialog", ui_propsheet_end_dialog, 1},  // @pymeth EndDialog|Closes the dialog, with the specified result.
    {"GetActiveIndex", ui_propsheet_get_active_index,
     1},  // @pymeth GetActiveIndex|Retrieves the index of the active page of the property sheet.
    {"GetActivePage", ui_propsheet_get_active_page,
     1},                                    // @pymeth GetActivePage|Returns the currently active property page.
    {"GetPage", ui_propsheet_get_page, 1},  // @pymeth GetPage|Returns the specified property page.
    {"GetPageIndex", ui_propsheet_get_page_index,
     1},  // @pymeth GetPageIndex|Retrieves the index of the specified page of the property sheet.
    {"GetPageCount", ui_propsheet_get_page_count, 1},  // @pymeth GetPageCount|Returns the number of pages.
    {"GetTabCtrl", ui_propsheet_get_tab_ctrl, 1},      // @pymeth GetTabCtrl|Returns the tab control used by the sheet.
    {"OnInitDialog", ui_propsheet_on_init_dialog,
     1},  // @pymeth OnInitDialog|Calls the default MFC OnInitDialog handler.
    {"PressButton", ui_propsheet_press_button,
     1},  // @pymeth PressButton|Simulates the choice of the specified button in a property sheet.
    {"RemovePage", ui_propsheet_remove_page, 1},  // @pymeth RemovePage|Removes the specified page from the sheet.
    {"SetActivePage", ui_propsheet_set_active_page,
     1},                                      // @pymeth SetActivePage|Programmatically sets the active page object.
    {"SetTitle", ui_propsheet_set_title, 1},  // @pymeth SetTitle|Sets the caption for the property sheet.
    {"SetFinishText", ui_propsheet_set_finish_text, 1},  // @pymeth SetFinishText|Sets the text for the Finish button
    {"SetWizardMode", ui_propsheet_set_wizard_mode, 1},  // @pymeth SetWizardMode|Enables the wizard mode
    {"SetWizardButtons", ui_propsheet_set_wizard_buttons, 1},  // @pymeth SetWizardButtons|Enables the wizard buttons
    {"SetPSHBit", ui_propsheet_set_pshbit, 1},  // @pymeth SetPSHBit|Sets (or clears) a bit in m_psh.dwFlags.
    {NULL, NULL}};

ui_type_CObject PyCPropertySheet::type("PyCPropertySheet", &PyCWnd::type, RUNTIME_CLASS(CPropertySheet),
                                       sizeof(PyCPropertySheet), PYOBJ_OFFSET(PyCPropertySheet), ui_propsheet_methods,
                                       GET_PY_CTOR(PyCPropertySheet));

/////////////////////////////////////////////////////////////////////
//
// Property Page object
//
//////////////////////////////////////////////////////////////////////
PyCPropertyPage::PyCPropertyPage()
{
    bManualDelete = FALSE;  // don't "delete" the CWnd.
}
PyCPropertyPage::~PyCPropertyPage()
{
    //	CPythonPropertyPage *pPage = GetPropPage(this);
    //	delete pPage;
}
// @pymethod <o PyCPropertyPage>|win32ui|CreatePropertyPage|Creates a property page object.
PyObject *PyCPropertyPage::create(PyObject *self, PyObject *args)
{
    TCHAR *Template = NULL;
    PyObject *obTemplate = NULL;
    int idCaption = 0;
    if (!PyArg_ParseTuple(args, "O|i",
                          &obTemplate,  // @pyparm <o PyResourceId>|resource||String template name or inteter resource
                                        // ID to use for the page.
                          &idCaption))  // @pyparm int|caption|0|The ID if the string resource to use for the caption.
        return NULL;
    CPythonPropertyPage *pPP;
    if (!PyWinObject_AsResourceId(obTemplate, &Template, FALSE))
        return NULL;

    if (IS_INTRESOURCE(Template)) {
        if (!PropSheetCheckForPageCreate((UINT)Template))
            return NULL;
        GUI_BGN_SAVE;
        pPP = new CPythonPropertyPage((UINT)Template, idCaption);
        GUI_END_SAVE;
    }
    else {
        if (!PropSheetCheckForPageCreate(Template))
            return NULL;
        GUI_BGN_SAVE;
        pPP = new CPythonPropertyPage(Template, idCaption);
        GUI_END_SAVE;
    }
    PyWinObject_FreeResourceId(Template);
    PyCPropertyPage *ret = (PyCPropertyPage *)ui_assoc_object::make(PyCPropertyPage::type, pPP);
    return ret;
}

// @pymethod <o PyCPropertyPage>|win32ui|CreatePropertyPageIndirect|Creates a property page object from a template.
PyObject *PyCPropertyPage::createIndirect(PyObject *, PyObject *args)
{
    PyObject *obTemplate = NULL;
    int idCaption = 0;
    // @pyparm <o PyDialogTemplate>|resourceList||Definition of the page to be created.
    // @pyparm int|caption|0|The ID if the string resource to use for the caption.
    if (!PyArg_ParseTuple(args, "O|i", &obTemplate, &idCaption))
        return NULL;

    HGLOBAL h = MakeResourceFromDlgList(obTemplate);
    if (h == NULL)
        return NULL;

    CPythonPropertyPage *pPP = new CPythonPropertyPage(IDD_DUMMYPROPPAGE, idCaption);
    if (!pPP->SetTemplate(h))
        return NULL;
    PyCPropertyPage *ret = (PyCPropertyPage *)ui_assoc_object::make(PyCPropertyPage::type, pPP);
    return ret;
}

// @pymethod |PyCPropertyPage|CancelToClose|Changes the Cancel button to Close.
PyObject *ui_proppage_cancel_to_close(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS(args);
    GUI_BGN_SAVE;
    pPP->CancelToClose();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertyPage|SetModified|Sets the modified flag.
PyObject *ui_proppage_set_modified(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    BOOL bChanged = TRUE;
    // @pyparm int|bChanged|1|A flag to indicate the new modified state.
    if (!PyArg_ParseTuple(args, "|i", &bChanged))
        return NULL;
    GUI_BGN_SAVE;
    pPP->SetModified(bChanged);
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertyPage|SetPSPBit|Sets or clears a bit in m_psp.dwFlags
PyObject *ui_proppage_set_pspbit(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    DWORD bitMask = 0;
    BOOL bitValue = 0;

    if (!PyArg_ParseTuple(args, "ii",
                          &bitMask,    // @pyparm int|bitMask||The PSP_* bit mask constant
                          &bitValue))  // @pyparm int|bitValue||1 to set, 0 to clear
        return NULL;
    GUI_BGN_SAVE;
    if (bitValue) {
        pPP->m_psp.dwFlags |= bitMask;
    }
    else {
        pPP->m_psp.dwFlags &= ~bitMask;
    }
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertyPage|OnOK|Calls the default MFC OnOK handler.
PyObject *ui_proppage_on_ok(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnOK);
    // @xref <vm PyCDialog.OnOK>
    GUI_BGN_SAVE;
    pPP->CPropertyPage::OnOK();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertyPage|OnApply|Calls the default MFC OnApply handler.
PyObject *ui_proppage_on_apply(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnApply);
    // @xref <vm PyCPropertyPage.OnApply>
    GUI_BGN_SAVE;
    BOOL bOk = pPP->CPropertyPage::OnApply();
    GUI_END_SAVE;
    return PyLong_FromLong((long)bOk);
}
// @pymethod |PyCPropertyPage|OnReset|Calls the default MFC OnReset handler.
PyObject *ui_proppage_on_reset(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnReset);
    // @xref <vm PyCPropertyPage.OnReset>
    GUI_BGN_SAVE;
    pPP->CPropertyPage::OnReset();
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyCPropertyPage|OnQueryCancel|Calls the default MFC OnQueryCancel handler.
PyObject *ui_proppage_on_query_cancel(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnQueryCancel);
    // @xref <vm PyCPropertyPage.OnQueryCancel>
    GUI_BGN_SAVE;
    BOOL bOk = pPP->CPropertyPage::OnQueryCancel();
    GUI_END_SAVE;
    return PyLong_FromLong((long)bOk);
}
// @pymethod |PyCPropertyPage|OnWizardBack|Calls the default MFC OnWizardBack handler.
PyObject *ui_proppage_on_wizard_back(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnWizardBack);
    // @xref <vm PyCPropertyPage.OnWizardBack>
    GUI_BGN_SAVE;
    LRESULT result = pPP->CPropertyPage::OnWizardBack();
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(result);
}
// @pymethod |PyCPropertyPage|OnWizardNext|Calls the default MFC OnWizardNext handler.
PyObject *ui_proppage_on_wizard_next(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnWizardNext);
    // @xref <vm PyCPropertyPage.OnWizardNext>
    GUI_BGN_SAVE;
    LRESULT result = pPP->CPropertyPage::OnWizardNext();
    GUI_END_SAVE;
    return PyWinObject_FromPARAM(result);
}
// @pymethod |PyCPropertyPage|OnWizardFinish|Calls the default MFC OnWizardFinish handler.
PyObject *ui_proppage_on_wizard_finish(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnWizardFinish);
    // @xref <vm PyCPropertyPage.OnWizardFinish>
    GUI_BGN_SAVE;
    BOOL bOk = pPP->CPropertyPage::OnWizardFinish();
    GUI_END_SAVE;
    return PyLong_FromLong((long)bOk);
}
// @pymethod |PyCPropertyPage|OnCancel|Calls the default MFC OnCancel handler.
PyObject *ui_proppage_on_cancel(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnCancel);
    // @xref <vm PyCDialog.OnCancel>
    GUI_BGN_SAVE;
    pPP->CPropertyPage::OnCancel();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCPropertyPage|OnSetActive|Calls the default MFC OnSetActive handler.
// @xref <vm PyCPropertyPage.OnSetActive>
PyObject *ui_proppage_on_set_active(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnSetActive);
    // @xref <vm PyCPropertyPage.OnSetActive>
    GUI_BGN_SAVE;
    long rc = pPP->CPropertyPage::OnSetActive();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
    // @rdesc The result is true if the page should be made active.
    // Typically this result should be passed to the original OnSetActive handler.
}
// @pymethod int|PyCPropertyPage|OnKillActive|Calls the default MFC OnKillActive handler.
// @xref <vm PyCPropertyPage.OnKillActive>
PyObject *ui_proppage_on_kill_active(PyObject *self, PyObject *args)
{
    CPythonPropertyPage *pPP;
    if (!(pPP = GetPropPage(self)))
        return NULL;
    CHECK_NO_ARGS2(args, OnKillActive);
    // @xref <vm PyCPropertyPage.OnKillActive>
    GUI_BGN_SAVE;
    long rc = pPP->CPropertyPage::OnKillActive();
    GUI_END_SAVE;
    return PyLong_FromLong(rc);
    // @rdesc The result is true if the page should be deselected.
    // Typically this result should be passed to the original OnSetActive handler.
}

// @object PyCPropertyPage|A class which encapsulates an MFC CPropertyPage object.  Derived from a <o PyCDialog> object.
static struct PyMethodDef ui_proppage_methods[] = {
    {"CancelToClose", ui_proppage_cancel_to_close, 1},  // @pymeth CancelToClose|Changes the Cancel button to Close.
    {"OnCancel", ui_proppage_on_cancel, 1},             // @pymeth OnCancel|Calls the default MFC OnCancel handler.
    {"OnOK", ui_proppage_on_ok, 1},                     // @pymeth OnOK|Calls the default MFC OnOK handler.
    {"OnApply", ui_proppage_on_apply, 1},               // @pymeth OnApply|Calls the default MFC OnApply handler.
    {"OnReset", ui_proppage_on_reset, 1},               // @pymeth OnReset|Calls the default MFC OnReset handler.
    {"OnQueryCancel", ui_proppage_on_query_cancel,
     1},  // @pymeth OnQueryCancel|Calls the default MFC OnQueryCancel handler.
    {"OnWizardBack", ui_proppage_on_wizard_back,
     1},  // @pymeth OnWizardBack|Calls the default MFC OnWizardBack handler.
    {"OnWizardNext", ui_proppage_on_wizard_next,
     1},  // @pymeth OnWizardNext|Calls the default MFC OnWizardNext handler.
    {"OnWizardFinish", ui_proppage_on_wizard_finish,
     1},  // @pymeth OnWizardFinish|Calls the default MFC OnWizardFinish handler.
    {"OnSetActive", ui_proppage_on_set_active, 1},  // @pymeth OnSetActive|Calls the default MFC OnSetActive handler.
    {"OnKillActive", ui_proppage_on_kill_active,
     1},                                           // @pymeth OnKillActive|Calls the default MFC OnKillActive handler.
    {"SetModified", ui_proppage_set_modified, 1},  // @pymeth SetModified|Sets the modified flag (for the Apply button).
    {"SetPSPBit", ui_proppage_set_pspbit, 1},      // @pymeth SetPSPBit|Sets (or clears) a bit in m_psp.dwFlags.
    {NULL, NULL}};
// derived from dialog.
ui_type_CObject PyCPropertyPage::type("PyCPropertyPage", &PyCDialog::type, RUNTIME_CLASS(CPropertyPage),
                                      sizeof(PyCPropertyPage), PYOBJ_OFFSET(PyCPropertyPage), ui_proppage_methods,
                                      GET_PY_CTOR(PyCPropertyPage));

/////////////////////////////////////////////////////////////////////
//
// Tab Control Object
//
// inherited from window
//
///////////////////////////////////////
ui_tabctrl_object::ui_tabctrl_object() {}

ui_tabctrl_object::~ui_tabctrl_object() {}

// @pymethod int|PyCTabCtrl|SetCurSel|Sets the current selection of a tab control.
PyObject *ui_tabctrl_set_cur_sel(PyObject *self, PyObject *args)
{
    CTabCtrl *pTab;
    if (!(pTab = PyGetTabCtrlWithWnd(self)))
        return NULL;
    int tab;
    // @pyparm int|index||The index of the tab to set current.
    if (!PyArg_ParseTuple(args, "i", &tab))
        return NULL;
    int rc = pTab->SetCurSel(tab);
    if (rc == -1)
        RETURN_ERR("SetCurSel failed");
    return Py_BuildValue("i", rc);
    // @rdesc The zero-based index of the previously selected item.
}
// @pymethod int|PyCTabCtrl|GetCurSel|Gets the current selection of a tab control.
PyObject *ui_tabctrl_get_cur_sel(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CTabCtrl *pTab;
    if ((pTab = PyGetTabCtrl(self)) == NULL)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pTab->GetCurSel();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
    // @rdesc The zero-based index of the currently selected item, or -1 if no selection.
}
// @pymethod int|PyCTabCtrl|GetItemCountl|Returns the number of tabs in the control.
PyObject *ui_tabctrl_get_item_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CTabCtrl *pTab;
    if ((pTab = PyGetTabCtrl(self)) == NULL)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pTab->GetItemCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @object PyCTabCtrl|A class which encapsulates an MFC CTabCtrl object.  Derived from a <o PyCWnd> object.
static struct PyMethodDef ui_tabctrl_methods[] = {
    /*	{"AddTab",           ui_tabctrl_add_tab,            1},
        {"NextTab",          ui_tabctrl_next_tab,           1},
        {"RemoveTab",        ui_tabctrl_remove_tab,         1},*/
    {"GetCurSel", ui_tabctrl_get_cur_sel, 1},        // @pymeth GetCurSel|Gets the current selection of a tab control.
    {"GetItemCount", ui_tabctrl_get_item_count, 1},  // @pymeth GetItemCountl|Returns the number of tabs in the control.
    {"SetCurSel", ui_tabctrl_set_cur_sel, 1},        // @pymeth SetCurSel|Sets the current selection of a tab control.
    {NULL, NULL}};

ui_type_CObject ui_tabctrl_object::type("PyCTabCtrl", &PyCWnd::type, RUNTIME_CLASS(CTabCtrl), sizeof(ui_tabctrl_object),
                                        PYOBJ_OFFSET(ui_tabctrl_object), ui_tabctrl_methods,
                                        GET_PY_CTOR(ui_tabctrl_object));
