// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#include <limits.h>

#include <Python.h>

// Must come after Python headers.
#include <atlbase.h>

#include "PythonCOM.h"
#include "PythonCOMServer.h"

#if _ATL_VER < 0x0200
typedef EXCEPINFO UserEXCEPINFO;
typedef VARIANT UserVARIANT;
typedef BSTR UserBSTR;
#endif

// NOTE - The standard "activscp.h" header is not good enough -
// need to use the IE4 SDK or MSVC6 etc.
#include "activscp.h"
#include "objsafe.h"

#if _ATL_VER < 0x0200
# include "datapath.h"
#endif

#include "multinfo.h"
#include "AXScript.h"

