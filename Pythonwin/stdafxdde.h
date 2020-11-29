#define _USING_V110_SDK71_

#include <afxwin.h>  // MFC core and standard components
#include <afxext.h>  // MFC extensions
#include <afxmt.h>   // Thread safety!

#include <limits.h>
#include <basetyps.h>

// windows defines "small" as "char" which breaks Python's accu.h
#undef small
#include <Python.h>

// The Pythonwin stuff
#include "oleauto.h"
#include "pywintypes.h"
#include "win32ui.h"
#include "win32assoc.h"

#include "stddde.h"
