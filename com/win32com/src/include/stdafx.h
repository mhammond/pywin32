// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws Windows NT :-(  However, we need to define this
// so we don't lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM

// objidl.h checks for this to define IContext and IEnumContextProps
#define USE_COM_CONTEXT_DEF

// PyWinTypes.h pulls in Python.h and windows.h.
#define PY_SSIZE_T_CLEAN
#include <PyWinTypes.h>
