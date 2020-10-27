// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#include <limits.h>

#include <Python.h>
#include <modsupport.h>

#include "PythonCOM.h"
#include "PythonCOMServer.h"

// We should not be using this!
#define OleSetOleError PyCom_BuildPyException

#if _ATL_VER < 0x0200
typedef EXCEPINFO UserEXCEPINFO;
typedef VARIANT UserVARIANT;
typedef BSTR UserBSTR;
#endif

#include <activdbg.h>

#if defined(__REQUIRED_RPCNDR_H_VERSION__)
// for some strange reason, these no longer exist in dbgprop.h !?!?
enum {
    EX_DBGPROP_INFO_ID = 0x0100,
    EX_DBGPROP_INFO_NTYPE = 0x0200,
    EX_DBGPROP_INFO_NVALUE = 0x0400,
    EX_DBGPROP_INFO_LOCKBYTES = 0x0800,
    EX_DBGPROP_INFO_DEBUGEXTPROP = 0x1000
};

#endif

// PythonAX Helpers.
extern BOOL PyAXDebug_PySOURCE_TEXT_ATTR_Length(PyObject *obAttr, ULONG *pLength);
extern BOOL PyAXDebug_PyObject_AsSOURCE_TEXT_ATTR(PyObject *obAttr, SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr);
extern PyObject *PyAXDebug_PyObject_FromSOURCE_TEXT_ATTR(const SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr);
