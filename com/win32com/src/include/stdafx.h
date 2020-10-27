// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws win95 and NT :-(  However, we need to define this
// so we dont lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM
#define _WIN32_WINNT 0x0501  // we use some of these features.

// objidl.h checks for this to define IContext and IEnumContextProps
#define USE_COM_CONTEXT_DEF

// PyWinTypes.h pulls in Python.h and windows.h.
#include <PyWinTypes.h>
