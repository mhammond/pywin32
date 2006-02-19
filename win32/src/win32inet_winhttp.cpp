// win32inet stuff that neets Winhttp.h

// This totally sucks - a single .cpp file can't #include both winhttp.h
// and wininet.h!  SWIG doesn't allow multiple .i files to build a single
// module!  Which leaves us with C++.
// The intent is to only wrap stuff which isn't otherwise doable from
// Python, such as the proxy stuff.

#include "windows.h"
#include "winhttp.h"
#include "pywintypes.h"

// @doc
typedef BOOL (WINAPI *funcWinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *);
static funcWinHttpGetIEProxyConfigForCurrentUser pfnWinHttpGetIEProxyConfigForCurrentUser=NULL;

#define CHECK_PFN(fname) \
  if (pfn##fname==NULL) \
    return PyErr_Format(PyExc_NotImplementedError, \
                        "%s is not available on this platform", #fname);

#define LOAD_PFN(name) \
    pfn##name=(func##name)loadwinhttpfunc(#name, hmod)
    
static FARPROC loadwinhttpfunc(char *funcname, HMODULE hmodule)
{
	if (hmodule==NULL)
		return NULL;
	return GetProcAddress(hmodule, funcname);
}

void init_win32inetstuff()
{
    HMODULE hmod = LoadLibrary("Winhttp.dll");
    if (!hmod)
        return; // nothing else to do!
    LOAD_PFN(WinHttpGetIEProxyConfigForCurrentUser);
}

// @pymethod tuple|win32inet|WinHttpGetIEProxyConfigForCurrentUser|Obtains
// the Internet Explorer proxy configuration for the current user.
PyObject *PyWinHttpGetIEProxyConfigForCurrentUser(PyObject *self, PyObject *args)
{
    CHECK_PFN(WinHttpGetIEProxyConfigForCurrentUser);
    if (!PyArg_ParseTuple(args, ":WinHttpGetIEProxyConfigForCurrentUser"))
        return NULL;
    // damn it - still gotta loadlib as this is only available later.
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG cfg;
    BOOL ok = (*pfnWinHttpGetIEProxyConfigForCurrentUser)(&cfg);
    if (!ok)
        return PyWin_SetAPIError("WinHttpGetIEProxyConfigForCurrentUser");
    PyObject *ret = Py_BuildValue("iuuu",
                                  cfg.fAutoDetect,
                                  cfg.lpszAutoConfigUrl,
                                  cfg.lpszProxy, cfg.lpszProxyBypass);
    if (cfg.lpszAutoConfigUrl) GlobalFree(cfg.lpszAutoConfigUrl);
    if (cfg.lpszProxy) GlobalFree(cfg.lpszProxy);
    if (cfg.lpszProxyBypass) GlobalFree(cfg.lpszProxyBypass);
    // @rdesc The result is a windows WINHTTP_CURRENT_USER_IE_PROXY_CONFIG
    // structure; a tuple of an int (bool) and 3 unicode strings
    // (fAutoDetect, lpszAutoConfigUrl, lpszProxy, lpszProxyBypass).
    // @pyseeapi WinHttpGetIEProxyConfigForCurrentUser
    // @pyseeapi WINHTTP_CURRENT_USER_IE_PROXY_CONFIG
    return ret;
}
