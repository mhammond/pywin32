// shell_pch.h : header file for PCH generation for the shell COM extension

#include <windows.h>
#include <oleauto.h>
#include <ocidl.h> // Used to be <multinfo.h>
#include <Python.h>
#include <PythonCOM.h>
#include <PythonCOMServer.h>
#include <shlobj.h>

BOOL PyObject_AsPIDL(PyObject *ob, LPITEMIDLIST *ppidl, BOOL bNoneOK = FALSE);
PyObject *PyObject_FromPIDL(LPCITEMIDLIST pidl, BOOL bFreeSystemPIDL);
void PyObject_FreePIDL( LPCITEMIDLIST pidl );

BOOL PyObject_AsPIDLArray(PyObject *obSeq, UINT *pcidl, LPCITEMIDLIST **ppidl);
void PyObject_FreePIDLArray(UINT cidl, LPCITEMIDLIST *pidl);
PyObject *PyObject_FromPIDLArray(UINT cidl, LPCITEMIDLIST *pidl);

BOOL PyObject_AsCMINVOKECOMMANDINFO(PyObject *ob, CMINVOKECOMMANDINFO *ppci);
void PyObject_FreeCMINVOKECOMMANDINFO( CMINVOKECOMMANDINFO *pci );
PyObject *PyObject_FromCMINVOKECOMMANDINFO(const CMINVOKECOMMANDINFO *pci);

BOOL PyObject_AsSTRRET( PyObject *obout, STRRET &out );
void PyObject_FreeSTRRET(STRRET &);
PyObject *PyObject_FromSTRRET(STRRET *pci, ITEMIDLIST *pidl, BOOL bFree);

BOOL PyObject_AsMSG( PyObject *obpmsg, MSG *msg );
PyObject *PyObject_FromMSG(const MSG *msg);

BOOL PyObject_AsFOLDERSETTINGS( PyObject *ob, FOLDERSETTINGS *pf);
PyObject *PyObject_FromFOLDERSETTINGS( const FOLDERSETTINGS *pf);

BOOL PyObject_AsRECT( PyObject *ob, RECT *r);
PyObject *PyObject_FromRECT(const RECT *r);
