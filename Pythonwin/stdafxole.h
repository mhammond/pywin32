// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//		are changed infrequently
//

#define WIN32_LEAN_AND_MEAN

#define _USING_V110_SDK71_

#include <afxwin.h>			// MFC core and standard components
#include <afxext.h> 		// MFC extensions
#include <afxcmn.h>         // common controls.
#include <afxrich.h>        // rich edit support.
#include <afxcview.h>		// control view support.
#include <afxpriv.h> 		// private MFC stuff!
#include <afxodlgs.h>
#include <afxmt.h>


#include "limits.h"

// allow memory leaks to give me the line number.
//#define new DEBUG_NEW

// windows defines "small" as "char" which breaks Python's accu.h
#undef small
#include "Python.h"
#include "modsupport.h"
#include "traceback.h"
#include "pythonrun.h"

// dont need all of these for all, but it cant hurt (and keep the speed up!)

#include "pywintypes.h"
#include "win32ui.h"
#include "win32assoc.h"
#include "win32cmd.h"
#include "win32win.h"

// --- EOF --- //
