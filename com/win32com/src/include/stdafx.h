// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws win95 and NT :-(  However, we need to define this
// so we dont lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM

#include <limits.h>

#include <Python.h>

#ifndef MS_WINCE // win32 wont need that soon?
// Must come after Python headers.
#include <windows.h>
#endif
