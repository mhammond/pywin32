// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws Windows NT :-(  However, we need to define this
// so we don't lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM

// PythonCOM.h pulls in Python.h and windows.h.
#include <PythonCOM.h>
#include <filter.h>
#include <filterr.h>
#include <ntquery.h>
#include <oleauto.h>
#include <ocidl.h>  // Used to be <multinfo.h>
