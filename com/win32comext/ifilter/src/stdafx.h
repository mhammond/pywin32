// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws win95 and NT :-(  However, we need to define this
// so we dont lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM

// PyWinTypes.h pulls in Python.h and windows.h.
#include <Filter.h>
#include <Filterr.h>
#include <ntquery.h>

#include <windows.h>
#include <oleauto.h>
#include <ocidl.h> // Used to be <multinfo.h>

#include <Python.h>
#include <PythonCOM.h>
