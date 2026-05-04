// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#include <limits.h>
#include <Python.h>

// Must come after Python headers.
#include <windows.h>

#include <Python.h>

#include "PythonCOM.h"
#include "PythonCOMServer.h"

// NOTE - The standard "activscp.h" header is not good enough -
// need to use the IE4 SDK or MSVC6 etc.
#include "activscp.h"
#include "objsafe.h"
#include "AXScript.h"
