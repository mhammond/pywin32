/*

  printer info data types

  Created March 1999, Roger Burnham (rburnham@cri-inc.com)

  These are implemented using CPrintInfo, CPrintDialog and PRINTDLG.

  NOT implemented:
        CPrintDialog::GetDevMode 
        typedef struct tagPD {  // pd 
           DWORD     lStructSize; 
           HWND      hwndOwner; 
           HANDLE    hDevMode; 
           HANDLE    hDevNames; 
           ... 
           HINSTANCE hInstance; 
           DWORD     lCustData; 
           LPPRINTHOOKPROC lpfnPrintHook; 
           LPSETUPHOOKPROC lpfnSetupHook; 
           LPCTSTR    lpPrintTemplateName; 
           LPCTSTR    lpSetupTemplateName; 
           HANDLE    hPrintTemplate; 
           HANDLE    hSetupTemplate; 
        } PRINTDLG; 
 
  Note:  If you use the custom print dialog, see the Knowledge Base article,

      HOWTO: Customize the Common Print Dialog Box
      Article ID: Q132909 

      And, you MUST NOT change the control ID's in this dialog.

  Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"
#include <dlgs.h>

#include "win32win.h"
#include "win32dlg.h"
#include "win32prinfo.h"

extern CPrintDialog *GetPrintDialog(PyObject *self);

// this returns a pointer that should not be stored.
void *ui_prinfo_object::GetGoodCppObject(ui_type *ui_type_check) const
{
  CPrintInfo *pPrInfo = (CPrintInfo *)
    ui_assoc_object::GetGoodCppObject(ui_type_check);
  if (pPrInfo==NULL) {
    PyErr_Clear();
    RETURN_NONE;
  }
  return pPrInfo;
}

// this returns a pointer that should not be stored.
CPrintInfo *ui_prinfo_object::GetPrintInfo(PyObject *self)
{
  return (CPrintInfo *)ui_assoc_object::GetGoodCppObject( self, &type);
}

void ui_prinfo_object::SetAssocInvalid()
{
  return; // do nothing.  Dont call base as dont want my handle wiped.
}

ui_prinfo_object::~ui_prinfo_object()
{
  if (m_deletePrInfo) {
    CPrintInfo *pPrInfo = GetPrintInfo(this);
    if (pPrInfo) {
      if (pPrInfo->m_lpUserData) {
        XDODECREF((PyObject*)pPrInfo->m_lpUserData);
        pPrInfo->m_lpUserData = NULL;
      }
      if (pPrInfo->m_pPD->m_pd.lCustData) {
        XDODECREF((PyObject*)pPrInfo->m_pPD->m_pd.lCustData);
        pPrInfo->m_pPD->m_pd.lCustData = NULL;
      }
    };
  }
}

// CPrintInfo member access

// @pymethod |PyCPrintInfo|DocObject|Return true if the document being printed is a DocObject.
// @pyseemfc CPrintInfo|m_bDocObject
static PyObject *ui_is_doc_object(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, DocObject);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_bDocObject);
}

// @pymethod |PyCPrintInfo|GetDwFlags|A flags specifying DocObject printing operations. Valid only if data member m_bDocObject is TRUE.
// @pyseemfc CPrintInfo|m_dwFlags
static PyObject *ui_get_dwflags(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetDwFlags);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_dwFlags);
}

// @pymethod |PyCPrintInfo|SetDwFlags|Set a flag specifying DocObject printing operations. Valid only if data member m_bDocObject is TRUE.
// @pyseemfc CPrintInfo|m_dwFlags
static PyObject *ui_set_dwflags(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetDwFlags",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_dwFlags = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetDocOffsetPage|Get the number of pages preceding the first page of a particular DocObject in a combined DocObject print job.
// @pyseemfc CPrintInfo|m_nOffsetPage
static PyObject *ui_get_doc_offset_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetDocOffsetPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_nOffsetPage);
}

// @pymethod |PyCPrintInfo|SetDocOffsetPage|Set the number of pages preceding the first page of a particular DocObject in a combined DocObject print job.
// @pyseemfc CPrintInfo|m_nOffsetPage
static PyObject *ui_set_doc_offset_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetDocOffsetPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_nOffsetPage = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|SetPrintDialog|Set a pointer to the CPrintDialog object used to display the Print dialog box for the print job. 
// @pyseemfc CPrintInfo|m_pPD
static PyObject *ui_set_print_dialog(PyObject * self, PyObject * args)
{
  PyObject *pyDlg;
  if (!PyArg_ParseTuple(args,"O:SetPrintDialog",&pyDlg))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  CPrintDialog *pDlg = GetPrintDialog(pyDlg);
  if (!pDlg)
    return NULL;
  // Convert the pyDlg from possibly an instance to the PyCPrintDialog.
  ui_assoc_object::GetGoodCppObject(pyDlg, &PyCPrintDialog::type);
  PyCPrintDialog *pyPrintDialog = (PyCPrintDialog *)pyDlg;

  delete pInfo->m_pPD;
  pInfo->m_pPD = (CPrintDialog *)pDlg;
  pInfo->m_pPD->m_pd.nMinPage = 1;
  pInfo->m_pPD->m_pd.nMaxPage = 0xffff;
  pInfo->m_pPD->m_pd.hInstance = pyPrintDialog->hInstance;
  pInfo->m_pPD->m_pd.lpPrintTemplateName = MAKEINTRESOURCE(PRINTDLGORD);
  pInfo->m_pPD->m_pd.Flags |= PD_ENABLEPRINTTEMPLATE;
//  pInfo->m_pPD->m_pd.Flags |= PD_PAGENUMS;
  pInfo->m_pPD->m_pd.hDC = NULL;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetDirect|TRUE if the Print dialog box will be bypassed for direct printing; FALSE otherwise. 
// @pyseemfc CPrintInfo|m_bDirect
static PyObject *ui_get_direct(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetDirect);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_bDirect);
}

// @pymethod |PyCPrintInfo|SetDirect|Sets to TRUE if the Print dialog box will be bypassed for direct printing; FALSE otherwise. 
// @pyseemfc CPrintInfo|m_bDirect
static PyObject *ui_set_direct(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetDirect",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_bDirect = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetPreview|A flag indicating whether the document is being previewed. 
// @pyseemfc CPrintInfo|m_bPreview
static PyObject *ui_get_preview(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetPreview);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_bPreview);
}

// @pymethod |PyCPrintInfo|SetPreview|Set whether the document is being previewed. 
// @pyseemfc CPrintInfo|m_bPreview
static PyObject *ui_set_preview(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetPreview",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_bPreview = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetContinuePrinting|A flag indicating whether the framework should continue the print loop. 
// @pyseemfc CPrintInfo|m_bContinuePrinting
static PyObject *ui_get_continue_printing(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetContinuePrinting);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_bContinuePrinting);
}

// @pymethod |PyCPrintInfo|SetContinuePrinting|Set whether the framework should continue the print loop. 
// @pyseemfc CPrintInfo|m_bContinuePrinting
static PyObject *ui_set_continue_printing(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetContinuePrinting",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_bContinuePrinting = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetCurPage|Get the number of the current page. 
// @pyseemfc CPrintInfo|m_nCurPage
static PyObject *ui_get_cur_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetCurPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_nCurPage);
}

// @pymethod |PyCPrintInfo|SetCurPage|Set the number of the current page.
// @pyseemfc CPrintInfo|m_nCurPage
static PyObject *ui_set_cur_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetCurPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_nCurPage = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetNumPreviewPages|Get the number of pages displayed in preview mode.
// @pyseemfc CPrintInfo|m_nNumPreviewPages
static PyObject *ui_get_num_preview_pages(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetNumPreviewPages);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("i", pInfo->m_nNumPreviewPages);
}

// @pymethod |PyCPrintInfo|SetNumPreviewPages|Set the number of pages displayed in preview mode.
// @pyseemfc CPrintInfo|m_nNumPreviewPages
static PyObject *ui_set_num_preview_pages(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetNumPreviewPages",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_nNumPreviewPages = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetUserData|Get a user-created structure.
// @pyseemfc CPrintInfo|m_lpUserData
static PyObject *ui_get_user_data(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetUserData);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  if (pInfo->m_lpUserData) {
    return Py_BuildValue ("O", pInfo->m_lpUserData);
  } else {
    return Py_BuildValue ("z", NULL);
  }
}

// @pymethod |PyCPrintInfo|SetUserData|Set a user-created structure.
// @pyseemfc CPrintInfo|m_lpUserData
static PyObject *ui_set_user_data(PyObject * self, PyObject * args)
{
  PyObject *var;
  if (!PyArg_ParseTuple(args,"O:SetUserData",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  if (pInfo->m_lpUserData) XDODECREF((PyObject*)pInfo->m_lpUserData);
  pInfo->m_lpUserData = var;
  DOINCREF(var);
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetDraw|Get the usable drawing area of the page in logical coordinates. 
// @pyseemfc CPrintInfo|m_rectDraw
static PyObject *ui_get_draw(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetDraw);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("(iiii)",
                        pInfo->m_rectDraw.left,
                        pInfo->m_rectDraw.top,
                        pInfo->m_rectDraw.right,
                        pInfo->m_rectDraw.bottom);
}

// @pymethod |PyCPrintInfo|SetDraw|Set the usable drawing area of the page in logical coordinates. 
// @pyseemfc CPrintInfo|m_rectDraw
static PyObject *ui_set_draw(PyObject * self, PyObject * args)
{
  int left, top, right, bottom;
  if (!PyArg_ParseTuple(args,"(iiii):SetDraw",&left,&top,&right,&bottom))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_rectDraw.left = left;
  pInfo->m_rectDraw.top = top;
  pInfo->m_rectDraw.right = right;
  pInfo->m_rectDraw.bottom = bottom;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetPageDesc|Get the format string used to display the page numbers during print preview
// @pyseemfc CPrintInfo|m_strPageDesc
static PyObject *ui_get_page_desc(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetPageDesc);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  return Py_BuildValue ("s", pInfo->m_strPageDesc);
}

// @pymethod |PyCPrintInfo|SetPageDesc|Set the format string used to display the page numbers during print preview
// @pyseemfc CPrintInfo|m_strPageDesc
static PyObject *ui_set_page_desc(PyObject * self, PyObject * args)
{
  char *var;
  if (!PyArg_ParseTuple(args,"s:SetPageDesc",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_strPageDesc = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetMinPage|Get the number of the first page of the document.
// @pyseemfc CPrintInfo|GetMinPage
static PyObject *ui_get_min_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetMinPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  UINT ret = pInfo->GetMinPage();
  GUI_END_SAVE;
  return Py_BuildValue ("i", ret);
}

// @pymethod |PyCPrintInfo|SetMinPage|Set the number of the first page of the document.
// @pyseemfc CPrintInfo|SetMinPage
static PyObject *ui_set_min_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetMinPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  pInfo->SetMinPage(var);
  GUI_END_SAVE;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetMaxPage|Get the number of the last page of the document.
// @pyseemfc CPrintInfo|GetMaxPage
static PyObject *ui_get_max_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetMaxPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  UINT ret = pInfo->GetMaxPage();
  GUI_END_SAVE;
  return Py_BuildValue ("i", ret);
}

// @pymethod |PyCPrintInfo|SetMaxPage|Set the number of the last page of the document.
// @pyseemfc CPrintInfo|SetMaxPage
static PyObject *ui_set_max_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetMaxPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  pInfo->SetMaxPage(var);
  GUI_END_SAVE;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetOffsetPage|Get the number of pages preceding the first page of a DocObject item being printed in a combined DocObject print job.  This currently does NOT work, as, if I include the symbol pInfo->GetOffsetPage(), the link fails to find its definition.  Allways returns 0.
// @pyseemfc CPrintInfo|GetOffsetPage
static PyObject *ui_get_offset_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetOffsetPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  UINT ret = 0; /* pInfo->GetOffsetPage() */ 
  GUI_END_SAVE;
  return Py_BuildValue ("i", ret);
}

// @pymethod |PyCPrintInfo|GetFromPage|The number of the first page to be printed.
// @pyseemfc CPrintInfo|GetFromPage
static PyObject *ui_get_from_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetFromPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  UINT ret = pInfo->GetFromPage();
  GUI_END_SAVE;
  return Py_BuildValue ("i", ret);
}

// @pymethod |PyCPrintInfo|GetToPage|The number of the last page to be printed.
// @pyseemfc CPrintInfo|GetToPage
static PyObject *ui_get_to_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetToPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  UINT ret = pInfo->GetToPage();
  GUI_END_SAVE;
  return Py_BuildValue ("i", ret);
}

// CPrintInfo->CPrintDialog access

// @pymethod |PyCPrintInfo|SetHDC|Sets the printer DC compatible with the users choices, call after the print dialog DoModal finishes.
// @pyseemfc CPrintInfo|m_pPD
// @pyseemfc CPrintDialog|m_pd.hDC
static PyObject *ui_set_hdc(PyObject * self, PyObject * args)
{
  PyObject *obdc;
  // @pyparm int|hdc||The DC.
  if (!PyArg_ParseTuple(args, "O:SetHDC", &obdc))
	  return NULL;
  HDC dc;
  if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&dc))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.Flags |= PD_RETURNDC;
  pInfo->m_pPD->m_pd.hDC = dc;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|CreatePrinterDC|Handle to the newly created printer device context, call only after DoModal finishes.
// @pyseemfc CPrintDialog|CreatePrinterDC
static PyObject *ui_create_printer_dc(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, CreatePrinterDC);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  HDC hDC = pInfo->m_pPD->CreatePrinterDC();
  GUI_END_SAVE;
  return PyWinLong_FromHANDLE(hDC);
}

// @pymethod |PyCPrintInfo|DoModal|Call DoModal on the dialog.
// @pyseemfc CPrintDialog|DoModal
static PyObject *ui_do_modal(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, DoModal);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  INT_PTR res = pInfo->m_pPD->DoModal();
  GUI_END_SAVE;
  return PyWinObject_FromDWORD_PTR(res);
}

#undef MAKE_INT_METH
#define MAKE_INT_METH(fnname, mfcName) \
static PyObject *fnname( PyObject *self, PyObject *args ) { \
  CHECK_NO_ARGS2(args, mfcName); \
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self); \
  if (!pInfo) return NULL; \
  GUI_BGN_SAVE; \
  int ret = pInfo->m_pPD->mfcName(); \
  GUI_END_SAVE; \
  return Py_BuildValue ("i", ret); \
}

#undef MAKE_STR_METH
#define MAKE_STR_METH(fnname, mfcName) \
static PyObject *fnname( PyObject *self, PyObject *args ) { \
  CHECK_NO_ARGS2(args, mfcName); \
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self); \
  if (!pInfo) return NULL; \
  GUI_BGN_SAVE; \
  CString ret = pInfo->m_pPD->mfcName(); \
  ::GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode); \
  GUI_END_SAVE; \
  return Py_BuildValue ("s", ret); \
}

// @pymethod |PyCPrintInfo|GetCopies|The number of copies requested, call only after DoModal finishes.
// @pyseemfc CPrintDialog|GetCopies
MAKE_INT_METH(ui_get_copies, GetCopies)

// @pymethod |PyCPrintInfo|GetDefaults|Nonzero if the function was successful; otherwise 0.  Call this function to retrieve the device defaults of the default printer without displaying a dialog box. The retrieved values are placed in the m_pd structure.  In some cases, a call to this function will call the constructor for CPrintDialog with bPrintSetupOnly set to FALSE. In these cases, a printer DC and hDevNames and hDevMode (two handles located in the m_pd data member) are automatically allocated.  If the constructor for CPrintDialog was called with bPrintSetupOnly set to FALSE, this function will not only return hDevNames and hDevMode (located in m_pd.hDevNames and m_pd.hDevMode) to the caller, but will also return a printer DC in m_pd.hDC. It is the responsibility of the caller to delete the printer DC and call the WindowsGlobalFree function on the handles when you are finished with the CPrintDialog object.
// @pyseemfc CPrintDialog|GetDefaults
MAKE_INT_METH(ui_get_defaults, GetDefaults)

// @pymethod |PyCPrintInfo|FreeDefaults|After a call to GetDefaults, and you are through with the CPrintDialog object, this call deletes the printer DC and calls GlobalFree function on the handles.
// @pyseemfc CPrintDialog|GetDefaults
static PyObject *ui_free_defaults(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, FreeDefaults);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  BOOL ok = DeleteDC(pInfo->m_pPD->GetPrinterDC());
  GUI_END_SAVE;
  if (!ok)
    RETURN_ERR("DeleteDC failed");
  char complaint[256];
  HGLOBAL res = GlobalFree(pInfo->m_pPD->m_pd.hDevNames);
  if (res) {
    sprintf (complaint, "GlobalFree(pInfo->m_pPD->m_pd.hDevNames) failed: %ld", GetLastError());
    RETURN_ERR(complaint);
  }
  res = GlobalFree(pInfo->m_pPD->m_pd.hDevMode);
  if (res) {
    sprintf (complaint, "GlobalFree(pInfo->m_pPD->m_pd.hDevMode) failed: %ld", GetLastError());
    RETURN_ERR(complaint);
  }
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetDeviceName|The name of the currently selected printer, call only after DoModal finishes.
// @pyseemfc CPrintDialog|GetDeviceName
MAKE_STR_METH(ui_get_device_name, GetDeviceName)

// @pymethod |PyCPrintInfo|GetDriverName|The name of the currently selected printer device driver, call only after DoModal finishes.
// @pyseemfc CPrintDialog|GetDriverName
MAKE_STR_METH(ui_get_driver_name, GetDriverName)

// @pymethod |PyCPrintInfo|GetDlgFromPage|Retrieves the starting page of the print range.
// @pyseemfc CPrintDialog|GetDlgFromPage
MAKE_INT_METH(ui_get_dlg_from_page, GetFromPage)

// @pymethod |PyCPrintInfo|GetDlgToPage|Retrieves the ending page of the print range.
// @pyseemfc CPrintDialog|GetDlgToPage
MAKE_INT_METH(ui_get_dlg_to_page, GetToPage)

// @pymethod |PyCPrintInfo|GetPortName|The name of the currently selected printer port, call only after DoModal finishes.
// @pyseemfc CPrintDialog|GetPortName
MAKE_STR_METH(ui_get_port_name, GetPortName)

// @pymethod |PyCPrintInfo|GetPrinterDC|A handle to the printer device context if successful; otherwise NULL.  If the bPrintSetupOnly parameter of the CPrintDialog constructor was FALSE (indicating that the Print dialog box is displayed), then GetPrinterDC returns a handle to the printer device context. You must call the WindowsDeleteDC function to delete the device context when you are done using it.
// @pyseemfc CPrintDialog|GetPrinterDC
static PyObject *ui_get_printer_dc(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetPrinterDC);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  GUI_BGN_SAVE;
  HDC hDC = pInfo->m_pPD->GetPrinterDC();
  GUI_END_SAVE;
  return PyWinLong_FromHANDLE(hDC);
}

// @pymethod |PyCPrintInfo|PrintAll|Nonzero if all pages in the document are to be printed; otherwise 0, call only after DoModal finishes.
// @pyseemfc CPrintDialog|PrintAll
MAKE_INT_METH(ui_print_all, PrintAll)

// @pymethod |PyCPrintInfo|PrintCollate|Nonzero if the user selects the collate check box in the dialog box; otherwise 0, call only after DoModal finishes.
// @pyseemfc CPrintDialog|PrintCollate
MAKE_INT_METH(ui_print_collate, PrintCollate)

// @pymethod |PyCPrintInfo|PrintRange|Nonzero if only a range of pages in the document are to be printed; otherwise 0, call only after DoModal finishes.
// @pyseemfc CPrintDialog|PrintRange
MAKE_INT_METH(ui_print_range, PrintRange)

// @pymethod |PyCPrintInfo|PrintSelection|Nonzero if only the selected items are to be printed; otherwise 0., call only after DoModal finishes
// @pyseemfc CPrintDialog|PrintSelection
MAKE_INT_METH(ui_print_selection, PrintSelection)


// CPrintInfo->CPrintDialog->PRINTDLG access

// @pymethod |PyCPrintInfo|GetHDC|Identifies a device context or an information context, depending on whether the Flags member specifies the PD_RETURNDC or PC_RETURNIC flag. If neither flag is specified, the value of this member is undefined. If both flags are specified, PD_RETURNDC has priority.
// @pyseemfc PRINTDLG|hDC
static PyObject *ui_get_hdc(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetHDC);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  HDC hDC = pInfo->m_pPD->m_pd.hDC;
  return PyWinLong_FromHANDLE(hDC);
}

// @pymethod |PyCPrintInfo|GetFlags|A set of bit flags that you can use to initialize the Print common dialog box. When the dialog box returns, it sets these flags to indicate the user's input. This member can be a combination of the following flags: PD_ALLPAGES, PD_COLLATE, PD_DISABLEPRINTTOFILE, PD_ENABLEPRINTHOOK, PD_ENABLEPRINTTEMPLATE, PD_ENABLEPRINTTEMPLATEHANDLE, PD_ENABLESETUPHOOK, PD_ENABLESETUPTEMPLATE, PD_ENABLESETUPTEMPLATEHANDLE, PD_HIDEPRINTTOFILE, PD_NONETWORKBUTTON, PD_NOPAGENUMS, PD_NOSELECTION, PD_NOWARNING, PD_PAGENUMS, PD_PRINTSETUP, PD_PRINTTOFILE, PD_RETURNDC, PD_RETURNDEFAULT, PD_RETURNIC, PD_SELECTION, PD_SHOWHELP, PD_USEDEVMODECOPIES, PD_USEDEVMODECOPIESANDCOLLATE.
// @pyseemfc PRINTDLG|Flags
static PyObject *ui_get_flags(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetFlags);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  DWORD ret = pInfo->m_pPD->m_pd.Flags;
  return Py_BuildValue ("i", ret);
}

// @pymethod |PyCPrintInfo|SetFlags|A set of bit flags that you can use to initialize the Print common dialog box. When the dialog box returns, it sets these flags to indicate the user's input. This member can be a combination of the following flags: PD_ALLPAGES, PD_COLLATE, PD_DISABLEPRINTTOFILE, PD_ENABLEPRINTHOOK, PD_ENABLEPRINTTEMPLATE, PD_ENABLEPRINTTEMPLATEHANDLE, PD_ENABLESETUPHOOK, PD_ENABLESETUPTEMPLATE, PD_ENABLESETUPTEMPLATEHANDLE, PD_HIDEPRINTTOFILE, PD_NONETWORKBUTTON, PD_NOPAGENUMS, PD_NOSELECTION, PD_NOWARNING, PD_PAGENUMS, PD_PRINTSETUP, PD_PRINTTOFILE, PD_RETURNDC, PD_RETURNDEFAULT, PD_RETURNIC, PD_SELECTION, PD_SHOWHELP, PD_USEDEVMODECOPIES, PD_USEDEVMODECOPIESANDCOLLATE.
// @pyseemfc PRINTDLG|Flags
static PyObject *ui_set_flags(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetFlags",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.Flags = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|SetFromPage|The number of the first page to be printed.
// @pyseemfc PRINTDLG|nFromPage
static PyObject *ui_set_from_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetFromPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.nFromPage  = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|SetToPage|The number of the last page to be printed.
// @pyseemfc PRINTDLG|nToPage
static PyObject *ui_set_to_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetToPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.nToPage  = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetPRINTDLGMinPage|Get the minimum value for the page range specified in the From and To page edit controls. If nMinPage equals nMaxPage, the Pages radio button and the starting and ending page edit controls are disabled. 
// @pyseemfc PRINTDLG|nMinPage
static PyObject *ui_get_printdlg_min_page(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetPRINTDLGMinPage);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  DWORD res = pInfo->m_pPD->m_pd.nMinPage;
  return Py_BuildValue ("i", res);
}

// @pymethod |PyCPrintInfo|SetPRINTDLGMinPage|Set the minimum value for the page range specified in the From and To page edit controls. If nMinPage equals nMaxPage, the Pages radio button and the starting and ending page edit controls are disabled. 
// @pyseemfc PRINTDLG|nMinPage
static PyObject *ui_set_printdlg_min_page(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetPRINTDLGMinPage",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.nMinPage = var;
  RETURN_NONE;
}

// @pymethod |PyCPrintInfo|GetPRINTDLGCopies|Get the initial number of copies for the Copies edit control if hDevMode is NULL; otherwise, the dmCopies member of theDEVMODE structure contains the initial value. When PrintDlg returns, nCopies contains the actual number of copies to print. This value depends on whether the application or the printer driver is responsible for printing multiple copies. If the PD_USEDEVMODECOPIESANDCOLLATE flag is set in the Flags member, nCopies is always 1 on return, and the printer driver is responsible for printing multiple copies. If the flag is not set, the application is responsible for printing the number of copies specified by nCopies. For more information, see the description of the PD_USEDEVMODECOPIESANDCOLLATE flag. 
// @pyseemfc PRINTDLG|nCopies
static PyObject *ui_get_printdlg_copies(PyObject * self, PyObject * args)
{
  CHECK_NO_ARGS2(args, GetPRINTDLGCopies);
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  DWORD res = pInfo->m_pPD->m_pd.nCopies;
  return Py_BuildValue ("i", res);
}

// @pymethod |PyCPrintInfo|SetPRINTDLGCopies|Set the initial number of copies for the Copies edit control if hDevMode is NULL; otherwise, the dmCopies member of theDEVMODE structure contains the initial value. When PrintDlg returns, nCopies contains the actual number of copies to print. This value depends on whether the application or the printer driver is responsible for printing multiple copies. If the PD_USEDEVMODECOPIESANDCOLLATE flag is set in the Flags member, nCopies is always 1 on return, and the printer driver is responsible for printing multiple copies. If the flag is not set, the application is responsible for printing the number of copies specified by nCopies. For more information, see the description of the PD_USEDEVMODECOPIESANDCOLLATE flag. 
// @pyseemfc PRINTDLG|nCopies
static PyObject *ui_set_printdlg_copies(PyObject * self, PyObject * args)
{
  int var;
  if (!PyArg_ParseTuple(args,"i:SetPRINTDLGCopies",&var))
    return NULL;
  CPrintInfo *pInfo = ui_prinfo_object::GetPrintInfo(self);
  if (!pInfo)
    return NULL;
  pInfo->m_pPD->m_pd.nCopies = var;
  RETURN_NONE;
}

/////////////////////////////////////////////////////////////////////
// PrintInfo Methods
// @object PyCPrintInfo|Encapsulates an MFC CPrintInfo class, its member <c CPrintDialog> class, and the <c PRINTDLG> structure member of the CPrintDialog.
static struct PyMethodDef ui_prinfo_methods[] = {

  // CPrintInfo 

  {"DocObject",             ui_is_doc_object,         1}, // @pymeth DocObject|A flag indicating whether the document being printed is a DocObject. 
  {"GetDwFlags",            ui_get_dwflags,           1}, // @pymeth GetDwFlags|A flags specifying DocObject printing operations. Valid only if data member m_bDocObject is TRUE.
  {"SetDwFlags",            ui_set_dwflags,           1}, // @pymeth SetDwFlags|Set a flag specifying DocObject printing operations. Valid only if data member m_bDocObject is TRUE.
  {"GetDocOffsetPage",      ui_get_doc_offset_page,   1}, // @pymeth GetDocOffsetPage|Get the number of pages preceding the first page of a particular DocObject in a combined DocObject print job.
  {"SetDocOffsetPage",      ui_set_doc_offset_page,   1}, // @pymeth SetDocOffsetPage|Set the number of pages preceding the first page of a particular DocObject in a combined DocObject print job.
  {"SetPrintDialog",        ui_set_print_dialog,      1}, // @pymeth SetPrintDialog|Set a pointer to the CPrintDialog object used to display the Print dialog box for the print job. 
  {"GetDirect",             ui_get_direct,            1}, // @pymeth GetDirect|TRUE if the Print dialog box will be bypassed for direct printing; FALSE otherwise. 
  {"SetDirect",             ui_set_direct,            1}, // @pymeth SetDirect|Sets to TRUE if the Print dialog box will be bypassed for direct printing; FALSE otherwise. 
  {"GetPreview",            ui_get_preview,           1}, // @pymeth GetPreview|A flag indicating whether the document is being previewed. 
  {"SetPreview",            ui_set_preview,           1}, // @pymeth SetPreview|Set whether the document is being previewed. 
  {"GetContinuePrinting",   ui_get_continue_printing, 1}, // @pymeth GetContinuePrinting|A flag indicating whether the framework should continue the print loop. 
  {"SetContinuePrinting",   ui_set_continue_printing, 1}, // @pymeth SetContinuePrinting|Set whether the framework should continue the print loop. 
  {"GetCurPage",            ui_get_cur_page,          1}, // @pymeth GetCurPage|Get the number of the current page. 
  {"SetCurPage",            ui_set_cur_page,          1}, // @pymeth SetCurPage|Set the number of the current page. 
  {"GetNumPreviewPages",    ui_get_num_preview_pages, 1}, // @pymeth GetNumPreviewPages|Get the number of pages displayed in preview mode.
  {"SetNumPreviewPages",    ui_set_num_preview_pages, 1}, // @pymeth SetNumPreviewPages|Set the number of pages displayed in preview mode.
  {"GetUserData",           ui_get_user_data,         1}, // @pymeth GetUserData|Get a user-created structure.
  {"SetUserData",           ui_set_user_data,         1}, // @pymeth SetUserData|Set a user-created structure.
  {"GetDraw",               ui_get_draw,              1}, // @pymeth GetDraw|Get the usable drawing area of the page in logical coordinates. 
  {"SetDraw",               ui_set_draw,              1}, // @pymeth SetDraw|Set the usable drawing area of the page in logical coordinates. 
  {"GetPageDesc",           ui_get_page_desc,         1}, // @pymeth GetPageDesc|Get the format string used to display the page numbers during print preview
  {"SetPageDesc",           ui_set_page_desc,         1}, // @pymeth SetPageDesc|Set the format string used to display the page numbers during print preview
  {"GetMinPage",            ui_get_min_page,          1}, // @pymeth GetMinPage|Get the number of the first page of the document.
  {"SetMinPage",            ui_set_min_page,          1}, // @pymeth SetMinPage|Set the number of the first page of the document.
  {"GetMaxPage",            ui_get_max_page,          1}, // @pymeth GetMaxPage|Get the number of the last page of the document.
  {"SetMaxPage",            ui_set_max_page,          1}, // @pymeth SetMaxPage|Set the number of the last page of the document.
  {"GetOffsetPage",         ui_get_offset_page,       1}, // @pymeth GetOffsetPage|Get the number of pages preceding the first page of a DocObject item being printed in a combined DocObject print job. 
  {"GetFromPage",           ui_get_from_page,         1}, // @pymeth GetFromPage|The number of the first page to be printed.
  {"GetToPage",             ui_get_to_page,           1}, // @pymeth GetToPage|The number of the last page to be printed.

  // CPrintDialog (CPrintInfo->m_pPD)

  {"SetHDC",                ui_set_hdc,               1}, // @pymeth SetHDC|Sets the printer DC compatible with the users choices, call after the print dialog DoModal finishes.
  {"CreatePrinterDC",       ui_create_printer_dc,     1}, // @pymeth CreatePrinterDC|Handle to the newly created printer device context, call only after DoModal finishes.
  {"DoModal",               ui_do_modal,              1}, // @pymeth DoModal|Call DoModal on the dialog.
  {"GetCopies",             ui_get_copies,            1}, // @pymeth GetCopies|The number of copies requested, call only after DoModal finishes.
  {"GetDefaults",           ui_get_defaults,          1}, // @pymeth GetDefaults|Retrieves device defaults without displaying a dialog box.
  {"FreeDefaults",          ui_free_defaults,         1}, // @pymeth FreeDefaults|After a call to GetDefaults, and you are through with the CPrintDialog object, this call deletes the printer DC and calls GlobalFree function on the handles.
  {"GetDeviceName",         ui_get_device_name,       1}, // @pymeth GetDeviceName|The name of the currently selected printer, call only after DoModal finishes.
  {"GetDriverName",         ui_get_driver_name,       1}, // @pymeth GetDriverName|The name of the currently selected printer device driver, call only after DoModal finishes.
  {"GetDlgFromPage",        ui_get_dlg_from_page,     1}, // @pymeth GetDlgFromPage|Retrieves the starting page of the print range.
  {"GetDlgToPage",          ui_get_dlg_to_page,       1}, // @pymeth GetDlgToPage|Retrieves the ending page of the print range.
  {"GetPortName",           ui_get_port_name,         1}, // @pymeth GetPortName|The name of the currently selected printer port, call only after DoModal finishes.
  {"GetPrinterDC",          ui_get_printer_dc,        1}, // @pymeth GetPrinterDC|A handle to the printer device context if successful; otherwise NULL.  If the bPrintSetupOnly parameter of the CPrintDialog constructor was FALSE (indicating that the Print dialog box is displayed), then GetPrinterDC returns a handle to the printer device context. You must call the WindowsDeleteDC function to delete the device context when you are done using it.
  {"PrintAll",              ui_print_all,             1}, // @pymeth PrintAll|Nonzero if all pages in the document are to be printed; otherwise 0, call only after DoModal finishes.
  {"PrintCollate",          ui_print_collate,         1}, // @pymeth PrintCollate|Nonzero if the user selects the collate check box in the dialog box; otherwise 0, call only after DoModal finishes.
  {"PrintRange",            ui_print_range,           1}, // @pymeth PrintRange|Nonzero if only a range of pages in the document are to be printed; otherwise 0, call only after DoModal finishes.
  {"PrintSelection",        ui_print_selection,       1}, // @pymeth PrintSelection|Nonzero if only the selected items are to be printed; otherwise 0., call only after DoModal finishes

  // PRINTDLG (CPrintInfo->CPrintDialog->m_pd)

  {"GetHDC",                ui_get_hdc,               1}, // @pymeth GetHDC|Identifies a device context or an information context, depending on whether the Flags member specifies the PD_RETURNDC or PC_RETURNIC flag. If neither flag is specified, the value of this member is undefined. If both flags are specified, PD_RETURNDC has priority. 
  {"GetFlags",              ui_get_flags,             1}, // @pymeth GetFlags|A set of bit flags that you can use to initialize the Print common dialog box. When the dialog box returns, it sets these flags to indicate the user's input.
  {"SetFlags",              ui_set_flags,             1}, // @pymeth SetFlags|A set of bit flags that you can use to initialize the Print common dialog box. When the dialog box returns, it sets these flags to indicate the user's input.
  {"SetFromPage",           ui_set_from_page,         1}, // @pymeth SetFromPage|The number of the first page to be printed.
  {"SetToPage",             ui_set_to_page,           1}, // @pymeth SetToPage|The number of the first page to be printed.
  {"GetPRINTDLGMinPage",    ui_get_printdlg_min_page, 1}, // @pymeth GetPRINTDLGMinPage|Get the minimum value for the page range specified in the From and To page edit controls. If nMinPage equals nMaxPage, the Pages radio button and the starting and ending page edit controls are disabled. 
  {"SetPRINTDLGMinPage",    ui_set_printdlg_min_page, 1}, // @pymeth SetPRINTDLGMinPage|Set the minimum value for the page range specified in the From and To page edit controls. If nMinPage equals nMaxPage, the Pages radio button and the starting and ending page edit controls are disabled. 
  {"GetPRINTDLGCopies",     ui_get_printdlg_copies,   1}, // @pymeth GetPRINTDLGCopies|Gets the initial number of copies for the Copies edit control if hDevMode is NULL; otherwise, the dmCopies member of the DEVMODE structure contains the initial value.
  {"SetPRINTDLGCopies",     ui_set_printdlg_copies,   1}, // @pymeth SetPRINTDLGCopies|Sets the initial number of copies for the Copies edit control if hDevMode is NULL; otherwise, the dmCopies member of the DEVMODE structure contains the initial value.
  {NULL, NULL}
};

ui_type ui_prinfo_object::type(
                               "PyCPrintInfo", 
                               &ui_assoc_object::type, 
                               sizeof(ui_prinfo_object), 
                               PYOBJ_OFFSET(ui_prinfo_object), 
                               ui_prinfo_methods, 
                               GET_PY_CTOR(ui_prinfo_object));

