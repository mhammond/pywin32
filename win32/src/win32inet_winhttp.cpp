// win32inet stuff that neets Winhttp.h

// This totally sucks - a single .cpp file can't #include both winhttp.h
// and wininet.h!  SWIG doesn't allow multiple .i files to build a single
// module!  Which leaves us with C++.
// The intent is to only wrap stuff which isn't otherwise doable from
// Python, such as the proxy stuff.

#include "windows.h"
#include "winhttp.h"
#include "pywintypes.h"

extern PyObject *PyObject_FromHINTERNET(HINTERNET hi);

// @doc
typedef BOOL (WINAPI *funcWinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *);
static funcWinHttpGetIEProxyConfigForCurrentUser pfnWinHttpGetIEProxyConfigForCurrentUser=NULL;

typedef BOOL (WINAPI *funcWinHttpGetProxyForUrl)(HINTERNET, LPCWSTR, WINHTTP_AUTOPROXY_OPTIONS*, WINHTTP_PROXY_INFO *);
static funcWinHttpGetProxyForUrl pfnWinHttpGetProxyForUrl=NULL;

typedef HINTERNET (WINAPI *funcWinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
static funcWinHttpOpen pfnWinHttpOpen=NULL;

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
    LOAD_PFN(WinHttpGetProxyForUrl);
    LOAD_PFN(WinHttpOpen);
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

// @object WINHTTP_AUTOPROXY_OPTIONS|Used by <om win32inet.WinHTTPGetProxyForUrl>
BOOL PyObject_AsWINHTTP_AUTOPROXY_OPTIONS(PyObject *ob, WINHTTP_AUTOPROXY_OPTIONS *out)
{
    int autoLogin = 1;
    PyObject *obAutoConfig;
    PyObject *obReserved = Py_None;
    if (!PyArg_ParseTuple(ob, "kkO|Oii",
                  &out->dwFlags, // @pyparm int|dwFlags||
                  &out->dwAutoDetectFlags, // @pyparm int|dwAutoDetectFlags||
                  &obAutoConfig, // @pyparm string|obAutoConfig||
                  &obReserved, // @pyparm object|obReserved|None|Must be None
                  &out->dwReserved, // @pyparm int|dwReserved|0|Must be zero
                  &autoLogin)) // @pyparm bool|autoLogin|1|
        return FALSE;
    out->fAutoLogonIfChallenged = autoLogin;
    if (obReserved != Py_None) {
        PyErr_SetString(PyExc_TypeError, "reserved param must be None");
        return FALSE;
    }
    if (!PyWinObject_AsWCHAR(obAutoConfig, (WCHAR **)&out->lpszAutoConfigUrl, TRUE))
        return FALSE;
    return TRUE;
}

void PyObject_CleanupWINHTTP_AUTOPROXY_OPTIONS(WINHTTP_AUTOPROXY_OPTIONS *out)
{
    if (out->lpszAutoConfigUrl)
        PyWinObject_FreeWCHAR((WCHAR *)out->lpszAutoConfigUrl);
}

// @pymethod tuple|win32inet|WinHttpGetIEProxyConfigForCurrentUser|Obtains
// the Internet Explorer proxy configuration for the current user.
PyObject *PyWinHttpGetProxyForUrl(PyObject *self, PyObject *args)
{
    CHECK_PFN(WinHttpGetProxyForUrl);
    PyObject *obHandle, *obURL, *obOptions;
    if (!PyArg_ParseTuple(args, "OOO:WinHttpGetProxyForUrl",
              &obHandle, // @pyparm <o HANDLE>/int|handle||
              &obURL, // @pyparm unicode/string|url||
              &obOptions // @pyparm tuple|options||
              ))
        return NULL;

    BOOL ok;
    HINTERNET hi;
    WCHAR *url = NULL;
    WINHTTP_AUTOPROXY_OPTIONS opts;
    WINHTTP_PROXY_INFO info;
    PyObject *ret = NULL;
    memset(&opts, 0, sizeof(opts));
    memset(&info, 0, sizeof(info));

    if (!PyWinObject_AsHANDLE(obHandle, (HANDLE *)&hi))
        goto done;

    if (!PyWinObject_AsWCHAR(obURL, &url, TRUE))
        goto done;

    if (!PyObject_AsWINHTTP_AUTOPROXY_OPTIONS(obOptions, &opts))
        goto done;

    Py_BEGIN_ALLOW_THREADS
    ok = (*pfnWinHttpGetProxyForUrl)(hi, url, &opts, &info);
    Py_END_ALLOW_THREADS
    if (!ok) {
        PyWin_SetAPIError("WinHttpGetProxyForUrl");
        goto done;
    }
    // @rdesc The result is a windows WINHTTP_PROXY_INFO
    // structure; a tuple of an int (bool) and 2 unicode strings
    // (dwAccessType, lpszProxy, lpszProxyBypass).
    // @pyseeapi WinHttpGetProxyForUrl
    // @pyseeapi WINHTTP_PROXY_INFO
    ret = Py_BuildValue("kuu",
                        info.dwAccessType,
                        info.lpszProxy,
                        info.lpszProxyBypass);
    if (info.lpszProxy) GlobalFree(info.lpszProxy);
    if (info.lpszProxyBypass) GlobalFree(info.lpszProxyBypass);
done:
    if (url)
        PyWinObject_FreeWCHAR(url);
    PyObject_CleanupWINHTTP_AUTOPROXY_OPTIONS(&opts);
    return ret;
}

// @pymethod <o PyHINTERNET>|win32inet|WinHttpOpen|Opens a winhttp session.
PyObject *PyWinHttpOpen(PyObject *self, PyObject *args)
{
    PyObject *obUA, *obProxyName, *obProxyBypass;
    DWORD dwAccessType, dwFlags;
    CHECK_PFN(WinHttpOpen);
    if (!PyArg_ParseTuple(args, "OkOOk:WinHttpOpen",
              &obUA, // @pyparm string|lpszUserAgent||
              &dwAccessType, // @pyparm int|dwAccessType||
              &obProxyName, // @pyparm string|lpszProxyName||
              &obProxyBypass, // @pyparm string|lpszProxyBypass||
              &dwFlags)) // @pyparm int|dwFlags||
        return NULL;

    HINTERNET hi;
    WCHAR *ua = NULL;
    WCHAR *proxy = NULL;
    WCHAR *proxy_bypass = NULL;
    PyObject *ret = NULL;

    if (!PyWinObject_AsWCHAR(obUA, &ua, TRUE))
        goto done;

    if (!PyWinObject_AsWCHAR(obProxyName, &proxy, TRUE))
        goto done;

    if (!PyWinObject_AsWCHAR(obProxyBypass, &proxy_bypass, TRUE))
        goto done;

    Py_BEGIN_ALLOW_THREADS
    hi = (*pfnWinHttpOpen)(ua, dwAccessType, proxy, proxy_bypass, dwFlags);
    Py_END_ALLOW_THREADS
    if (!hi) {
        PyWin_SetAPIError("WinHttpOpen");
        goto done;
    }
    // @pyseeapi WinHttpOpen
    ret = PyObject_FromHINTERNET(hi);
done:
    if (ua)
        PyWinObject_FreeWCHAR(ua);
    if (proxy)
        PyWinObject_FreeWCHAR(proxy);
    if (proxy_bypass)
        PyWinObject_FreeWCHAR(proxy_bypass);
    return ret;
}
