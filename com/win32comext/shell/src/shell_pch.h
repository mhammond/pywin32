// shell_pch.h : header file for PCH generation for the shell COM extension

#include <PythonCOM.h>
#include <PythonCOMServer.h>
#include <oleauto.h>
#include <ocidl.h>  // Used to be <multinfo.h>
#include <wininet.h>
#include <shlobj.h>

BOOL PyObject_AsPIDL(PyObject *ob, LPITEMIDLIST *ppidl, BOOL bNoneOK = FALSE, UINT *pcb = NULL);
PyObject *PyObject_FromPIDL(LPCITEMIDLIST pidl, BOOL bFreeSystemPIDL);
void PyObject_FreePIDL(LPCITEMIDLIST pidl);

BOOL PyObject_AsPIDLArray(PyObject *obSeq, UINT *pcidl, LPCITEMIDLIST **ppidl);
void PyObject_FreePIDLArray(UINT cidl, LPCITEMIDLIST *pidl);
PyObject *PyObject_FromPIDLArray(UINT cidl, LPCITEMIDLIST *pidl);

BOOL PyObject_AsCMINVOKECOMMANDINFO(PyObject *ob, CMINVOKECOMMANDINFO *ppci);
void PyObject_FreeCMINVOKECOMMANDINFO(CMINVOKECOMMANDINFO *pci);
PyObject *PyObject_FromCMINVOKECOMMANDINFO(const CMINVOKECOMMANDINFO *pci);

void PyObject_FreeSTRRET(STRRET &);
PyObject *PyObject_FromSTRRET(STRRET *pci, ITEMIDLIST *pidl, BOOL bFree);

BOOL PyObject_AsMSG(PyObject *obpmsg, MSG *msg);
PyObject *PyObject_FromMSG(const MSG *msg);

BOOL PyObject_AsFOLDERSETTINGS(PyObject *ob, FOLDERSETTINGS *pf);
PyObject *PyObject_FromFOLDERSETTINGS(const FOLDERSETTINGS *pf);

BOOL PyObject_AsRECT(PyObject *ob, RECT *r);
PyObject *PyObject_FromRECT(const RECT *r);

BOOL PyObject_AsEXPLORER_BROWSER_OPTIONS(PyObject *, EXPLORER_BROWSER_OPTIONS *);
PyObject *PyObject_FromEXPLORER_BROWSER_OPTIONS(EXPLORER_BROWSER_OPTIONS);

BOOL PyObject_AsEXPLORER_BROWSER_FILL_FLAGS(PyObject *, EXPLORER_BROWSER_FILL_FLAGS *);
PyObject *PyObject_FromEXPLORER_BROWSER_FILL_FLAGS(EXPLORER_BROWSER_FILL_FLAGS);

BOOL PyObject_AsSHCOLUMNID(PyObject *ob, SHCOLUMNID *p);
PyObject *PyObject_FromSHCOLUMNID(LPCSHCOLUMNID p);

BOOL PyObject_AsSHCOLUMNINIT(PyObject *, SHCOLUMNINIT *);
PyObject *PyObject_FromSHCOLUMNINIT(LPCSHCOLUMNINIT);

BOOL PyObject_AsSHCOLUMNINFO(PyObject *, SHCOLUMNINFO *);
PyObject *PyObject_FromSHCOLUMNINFO(LPCSHCOLUMNINFO);

BOOL PyObject_AsSHCOLUMNDATA(PyObject *, SHCOLUMNDATA *);
void PyObject_FreeSHCOLUMNDATA(SHCOLUMNDATA *p);
PyObject *PyObject_FromSHCOLUMNDATA(LPCSHCOLUMNDATA);

PyObject *PyObject_FromFOLDERSETTINGS(const FOLDERSETTINGS *pf);
BOOL PyObject_AsFOLDERSETTINGS(PyObject *ob, FOLDERSETTINGS *pf);

BOOL PyWinObject_AsSHELL_ITEM_RESOURCE(PyObject *ob, SHELL_ITEM_RESOURCE *psir);
PyObject *PyWinObject_FromSHELL_ITEM_RESOURCE(const SHELL_ITEM_RESOURCE *psir);

inline BOOL PyObject_AsPCUIDLIST_RELATIVE(PyObject *ob, PCUIDLIST_RELATIVE *ppidl, BOOL bNoneOK = FALSE,
                                          UINT *pcb = NULL)
{
    return PyObject_AsPIDL(ob, (LPITEMIDLIST *)ppidl, bNoneOK, pcb);
}
inline void PyObject_FreePCUIDLIST_RELATIVE(PCUIDLIST_RELATIVE pidl) { PyObject_FreePIDL((LPCITEMIDLIST)pidl); }

inline BOOL PyObject_AsPCIDLIST_ABSOLUTE(PyObject *ob, PCUIDLIST_ABSOLUTE *ppidl, BOOL bNoneOK = FALSE,
                                         UINT *pcb = NULL)
{
    return PyObject_AsPIDL(ob, (LPITEMIDLIST *)ppidl, bNoneOK, pcb);
}
inline void PyObject_FreePCIDLIST_ABSOLUTE(PCIDLIST_ABSOLUTE pidl) { PyObject_FreePIDL((LPCITEMIDLIST)pidl); }

inline PyObject *PyObject_FromPCIDLIST_ABSOLUTE(PCUIDLIST_ABSOLUTE pidl, BOOL bFreeSystemPIDL)
{
    return PyObject_FromPIDL((LPITEMIDLIST)pidl, bFreeSystemPIDL);
}

// TRANSFER_SOURCE_FLAGS enum isn't in Vista SDK, instead was a sequence of #defines
#ifndef TRANSFER_SOURCE_FLAGS
#define TRANSFER_SOURCE_FLAGS DWORD
#endif
