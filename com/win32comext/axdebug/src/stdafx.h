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

#ifdef HAVE_SDK_ACTIVDBG
#include <activdbg.h>
#else
#include "activdbg.h"
#endif

#if defined(__REQUIRED_RPCNDR_H_VERSION__)
// for some strange reason, these no longer exist in dbgprop.h !?!?
enum __MIDL___MIDL_itf_dbgprop_0000_0001
    {	DBGPROP_ATTRIB_NO_ATTRIB	= 0,
	DBGPROP_ATTRIB_VALUE_IS_INVALID	= 0x8,
	DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE	= 0x10,
	DBGPROP_ATTRIB_VALUE_READONLY	= 0x800,
	DBGPROP_ATTRIB_ACCESS_PUBLIC	= 0x1000,
	DBGPROP_ATTRIB_ACCESS_PRIVATE	= 0x2000,
	DBGPROP_ATTRIB_ACCESS_PROTECTED	= 0x4000,
	DBGPROP_ATTRIB_ACCESS_FINAL	= 0x8000,
	DBGPROP_ATTRIB_STORAGE_GLOBAL	= 0x10000,
	DBGPROP_ATTRIB_STORAGE_STATIC	= 0x20000,
	DBGPROP_ATTRIB_STORAGE_FIELD	= 0x40000,
	DBGPROP_ATTRIB_STORAGE_VIRTUAL	= 0x80000,
	DBGPROP_ATTRIB_TYPE_IS_CONSTANT	= 0x100000,
	DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED	= 0x200000,
	DBGPROP_ATTRIB_TYPE_IS_VOLATILE	= 0x400000,
	DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS	= 0x800000
    };
typedef DWORD DBGPROP_ATTRIB_FLAGS;


enum __MIDL___MIDL_itf_dbgprop_0000_0002
    {	DBGPROP_INFO_NAME	= 0x1,
	DBGPROP_INFO_TYPE	= 0x2,
	DBGPROP_INFO_VALUE	= 0x4,
	DBGPROP_INFO_FULLNAME	= 0x20,
	DBGPROP_INFO_ATTRIBUTES	= 0x8,
	DBGPROP_INFO_DEBUGPROP	= 0x10,
	DBGPROP_INFO_AUTOEXPAND	= 0x8000000
    };
typedef DWORD DBGPROP_INFO_FLAGS;

enum {
   EX_DBGPROP_INFO_ID  =0x0100,
   EX_DBGPROP_INFO_NTYPE  =0x0200,
   EX_DBGPROP_INFO_NVALUE  =0x0400,
   EX_DBGPROP_INFO_LOCKBYTES  =0x0800,
   EX_DBGPROP_INFO_DEBUGEXTPROP  =0x1000
};

#endif

// PythonAX Helpers.
extern BOOL PyAXDebug_PySOURCE_TEXT_ATTR_Length(  PyObject *obAttr, ULONG *pLength );
extern BOOL PyAXDebug_PyObject_AsSOURCE_TEXT_ATTR(  PyObject *obAttr, SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr );
extern PyObject *PyAXDebug_PyObject_FromSOURCE_TEXT_ATTR( const SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr);

