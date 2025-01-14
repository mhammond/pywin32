// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//		are changed infrequently
//

#define WIN32_LEAN_AND_MEAN

#define _USING_V110_SDK71_
// MFC support for mbcs is going away, but no need for us to constantly be told...
#define NO_WARN_MBCS_MFC_DEPRECATION

#include <afxwin.h>  // MFC core and standard components

#include <afxext.h>    // MFC extensions
#include <afxcmn.h>    // common controls.
#include <afxrich.h>   // rich edit support.
#include <afxcview.h>  // control view support.
#include <afxpriv.h>   // private MFC stuff!
#include <afxtempl.h>  // for collection templates.
#include <afxmt.h>     // multi-threading
#include "limits.h"

// allow memory leaks to give me the line number.
// #define new DEBUG_NEW

/* don't really need to undef these anymore, but helpful to
programmers who forget to use the new names. */
#undef INCREF
#undef DECREF
// windows defines "small" as "char" which breaks Python's accu.h
#undef small
#include "Python.h"
#include "modsupport.h"
#include "traceback.h"
#include "pythonrun.h"
#include "import.h"  // Python: for dynamicattach routines.

#include "pywintypes.h"
// don't need all of these for all, but it can't hurt (and keep the speed up!)

#include "win32ui.h"

#include "win32uiExt.h"

#include "pythonframe.h"
#include "pythonview.h"  // App: Edit View.
#include "pythondoc.h"

#include "win32assoc.h"
#include "win32cmd.h"
#include "win32app.h"

// --- EOF --- //
