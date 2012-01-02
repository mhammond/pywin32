// win32inet stuff that neets Winhttp.h

// This totally sucks - a single .cpp file can't #include both winhttp.h
// and wininet.h!  SWIG doesn't allow multiple .i files to build a single
// module!  Which leaves us with C++.
// The intent is to only wrap stuff which isn't otherwise doable from
// Python, such as the proxy stuff.

#include "pywintypes.h"
#include "pywinobjects.h"
#include "winhttp.h"

// @doc
typedef BOOL (WINAPI *funcWinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *);
static funcWinHttpGetIEProxyConfigForCurrentUser pfnWinHttpGetIEProxyConfigForCurrentUser=NULL;

typedef BOOL (WINAPI *funcWinHttpGetDefaultProxyConfiguration)(WINHTTP_PROXY_INFO *);
static funcWinHttpGetDefaultProxyConfiguration pfnWinHttpGetDefaultProxyConfiguration=NULL;

typedef BOOL (WINAPI *funcWinHttpGetProxyForUrl)(HINTERNET, LPCWSTR, WINHTTP_AUTOPROXY_OPTIONS*, WINHTTP_PROXY_INFO *);
static funcWinHttpGetProxyForUrl pfnWinHttpGetProxyForUrl=NULL;

typedef HINTERNET (WINAPI *funcWinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
static funcWinHttpOpen pfnWinHttpOpen=NULL;

typedef BOOL (WINAPI *funcWinHttpCloseHandle)(HINTERNET);
static funcWinHttpCloseHandle pfnWinHttpCloseHandle=NULL;

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
    HMODULE hmod = LoadLibrary(_T("Winhttp.dll"));
    if (!hmod)
        return; // nothing else to do!
    LOAD_PFN(WinHttpGetIEProxyConfigForCurrentUser);
    LOAD_PFN(WinHttpGetProxyForUrl);
    LOAD_PFN(WinHttpOpen);
    LOAD_PFN(WinHttpCloseHandle);
    LOAD_PFN(WinHttpGetDefaultProxyConfiguration);
    // winhttp.dll also provides the string resources for its errors.
    PyWin_RegisterErrorMessageModule(WINHTTP_ERROR_BASE, WINHTTP_ERROR_LAST, hmod);
}

// A handle used by WinHttpOpen; even though it is documented as a HINTERNET,
// our standard HINTERNET isn't suitable as (a) the callbacks fail and (b)
// the handle must be closed via WinHttpCloseHandle.
class PyHWINHTTP : public PyHANDLE
{
public:
	PyHWINHTTP(HINTERNET hInit) : PyHANDLE((HANDLE)hInit) {;}

	virtual BOOL Close(void) {
		BOOL ret=TRUE;
		// We've already checked we have the function-pointer - but
		// it can't hurt to check again!
		if (m_handle && pfnWinHttpCloseHandle){
			HINTERNET h=m_handle;
			m_handle = 0; // don't try again!
			ret=(*pfnWinHttpCloseHandle)(h);
			if (!ret)
				PyWin_SetAPIError("WinHttpCloseHandle");
			}
		return ret;
	}
	virtual const char *GetTypeName() {return "PyHWINHTTP";}
};

PyObject *PyObject_FromWinHttpHandle(HINTERNET hi)
{
	// Don't allow handles to be created if we can't close them!
	CHECK_PFN(WinHttpCloseHandle);
	PyHWINHTTP *ret=new PyHWINHTTP(hi);
	if (ret==NULL)
		return PyErr_NoMemory();
	if (PyErr_Occurred()){
		Py_DECREF(ret);
		ret = NULL;
		}
	return ret;
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

// @object PyWINHTTP_PROXY_INFO|A tuple representing a WINHTTP_PROXY_INFO structure.

PyObject *PyObject_FromWINHTTP_PROXY_INFO(WINHTTP_PROXY_INFO *i, BOOL bFreeStrings=TRUE)
{
    PyObject *ret = Py_BuildValue("kuu",
                        i->dwAccessType, // @tupleitem 0|int|dwAccessType|
                        i->lpszProxy, // @tupleitem 2|string|lpszProxy|
                        i->lpszProxyBypass); // @tupleitem 3|string|lpszProxy|
    if (i->lpszProxy) GlobalFree(i->lpszProxy);
    if (i->lpszProxyBypass) GlobalFree(i->lpszProxyBypass);
    return ret;
}

// @object PyWINHTTP_AUTOPROXY_OPTIONS|Used by <om win32inet.WinHTTPGetProxyForUrl>
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

// @pymethod <o PyWINHTTP_PROXY_INFO>|win32inet|WinHttpGetDefaultProxyConfiguration|
// Retrieves the default WinHTTP proxy configuration from the registry.
PyObject *PyWinHttpGetDefaultProxyConfiguration(PyObject *self, PyObject *args)
{
    CHECK_PFN(WinHttpGetDefaultProxyConfiguration);
    if (!PyArg_ParseTuple(args, ":WinHttpGetDefaultProxyConfiguration"))
        return NULL;
    WINHTTP_PROXY_INFO info;
    memset(&info, 0, sizeof(info));
    BOOL ok;
    Py_BEGIN_ALLOW_THREADS
    ok = (*pfnWinHttpGetDefaultProxyConfiguration)(&info);
    Py_END_ALLOW_THREADS
    if (!ok) {
        PyWin_SetAPIError("WinHttpGetDefaultProxyConfiguration");
        return NULL;
    }
    return PyObject_FromWINHTTP_PROXY_INFO(&info);
}

// @pymethod <o PyWINHTTP_PROXY_INFO>|win32inet|WinHttpGetProxyForUrl|Obtains
// the Internet Explorer proxy configuration for the specified URL.
PyObject *PyWinHttpGetProxyForUrl(PyObject *self, PyObject *args)
{
    CHECK_PFN(WinHttpGetProxyForUrl);
    PyObject *obHandle, *obURL, *obOptions;
    if (!PyArg_ParseTuple(args, "OOO:WinHttpGetProxyForUrl",
              &obHandle, // @pyparm <o HANDLE>/int|handle||
              &obURL, // @pyparm unicode/string|url||
              &obOptions // @pyparm <o PyWINHTTP_AUTOPROXY_OPTIONS>|options||
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
    ret = PyObject_FromWINHTTP_PROXY_INFO(&info);
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
    ret = PyObject_FromWinHttpHandle(hi);
done:
    if (ua)
        PyWinObject_FreeWCHAR(ua);
    if (proxy)
        PyWinObject_FreeWCHAR(proxy);
    if (proxy_bypass)
        PyWinObject_FreeWCHAR(proxy_bypass);
    return ret;
}
