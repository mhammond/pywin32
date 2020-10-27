// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// _WIN32_DCOM screws win95 and NT :-(  However, we need to define this
// so we dont lose all the constants etc that come with DCOM
//
#define _WIN32_DCOM

// PythonCOM.h pulls in Python.h and windows.h.
#include <PythonCOM.h>
#include <Filter.h>
#include <Filterr.h>

#define MISSING_PROPSTG
#ifdef MISSING_PROPSTG
// Ack - NTQuery.h is failing with the Vista SDK - pull in what we need
// Problem is missing propstg.h, and all the work-arounds are uglier than
// just these 3 prototypes.
// See http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=508254&SiteID=1

STDAPI LoadIFilter(PCWSTR pwcsPath, __in IUnknown *pUnkOuter, __deref_out void **ppIUnk);
STDAPI BindIFilterFromStorage(__in IStorage *pStg, __in IUnknown *pUnkOuter, __deref_out void **ppIUnk);

STDAPI BindIFilterFromStream(__in IStream *pStm, __in IUnknown *pUnkOuter, __deref_out void **ppIUnk);
#else
#include <ntquery.h>
#endif

#include <oleauto.h>
#include <ocidl.h>  // Used to be <multinfo.h>
