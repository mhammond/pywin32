// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//		are changed infrequently
//

//#define HIER_LIST Yay - no more - tree control now used!
#define WIN32_LEAN_AND_MEAN

#include <afxwin.h>			// MFC core and standard components

#if (_MFC_VER < 0x0600)
// See pythonpsheet.cpp for more details!
#define _WIN32_IE 0x0300 // Screw up with header sizes and MFC!!
#endif

#include <afxext.h> 		// MFC extensions
#include <afxcmn.h>         // common controls.
#include <afxrich.h>        // rich edit support.
#include <afxcview.h>		// control view support.
#include <afxpriv.h> 		// private MFC stuff!
#include <afxtempl.h> 		// for collection templates.
#include <afxmt.h>			// multi-threading
#include "limits.h"

// allow memory leaks to give me the line number.
//#define new DEBUG_NEW

/* dont really need to undef these anymore, but helpful to
programmers who forget to use the new names. */
#undef INCREF
#undef DECREF
#include "Python.h"
#include "modsupport.h"
#include "traceback.h"
#include "pythonrun.h"
#include "import.h"	// Python: for dynamicattach routines.
#include "graminit.h"

#include "pywintypes.h"
// dont need all of these for all, but it cant hurt (and keep the speed up!)

#include "win32ui.h"

#include "win32uiExt.h"

#include "pythonframe.h" 
#include "pythonview.h" // App: Edit View.
#include "pythondoc.h"

#include "win32assoc.h"
#include "win32cmd.h"
#include "win32app.h"

// --- EOF --- //
