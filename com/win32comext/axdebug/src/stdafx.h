// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#include <limits.h>

#include <Python.h>
#include <modsupport.h>

// Must come after Python headers.
#include <atlbase.h>

#include "PythonCOM.h"
#include "PythonCOMServer.h"

#if _ATL_VER < 0x0200
typedef EXCEPINFO UserEXCEPINFO;
typedef VARIANT UserVARIANT;
typedef BSTR UserBSTR;
#endif

#include "activdbg.h"

// PythonAX Helpers.
extern BOOL PyAXDebug_PySOURCE_TEXT_ATTR_Length(  PyObject *obAttr, ULONG *pLength );
extern BOOL PyAXDebug_PyObject_AsSOURCE_TEXT_ATTR(  PyObject *obAttr, SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr );
extern PyObject *PyAXDebug_PyObject_FromSOURCE_TEXT_ATTR( const SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr);

