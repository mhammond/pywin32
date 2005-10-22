/* File : win32inet.i */
// @doc

%module win32inet // An interface to the Windows internet (wininet) API

%{
#include "Windows.h"
#include "WinInet.h"
#undef BOOLAPI // wininet.h defines this!
%}

%include "typemaps.i"
%include "pywin32.i"

%apply HWND {long};
typedef long HWND

%{
#undef PyHANDLE // undef earlier define, so we are back to the class.
#include "pywinobjects.h"

// @object PyHINTERNET|An object that wraps a HINTERNET handle.  When the
// handle object is destroyed, it is automatically closed.
// See the <o PyHANDLE> object for more details.
class PyHINTERNET : public PyHANDLE
{
public:
    PyHINTERNET(HANDLE hInit) : PyHANDLE(hInit) {}
    virtual BOOL Close(void) {
        if (m_handle && !InternetCloseHandle((HINTERNET)m_handle)) {
            m_handle = 0; // don't try again!
            PyWin_SetAPIError("InternetCloseHandle");
            return FALSE;
        }
        m_handle = 0;
        return TRUE;
    }
    virtual const char *GetTypeName() {return "PyHINTERNET";}
};
%}

%typemap(python,ignore) HINTERNET *OUTPUT(HINTERNET temp)
{
  $target = &temp;
}

%typemap(python,except) PyHINTERNET {
    Py_BEGIN_ALLOW_THREADS
    $function
    Py_END_ALLOW_THREADS
    if ($source==0 || $source==INVALID_HANDLE_VALUE)  {
        $cleanup
        return PyWin_SetAPIError("$name");
    }
}

%typemap(python,except) HINTERNET {
    Py_BEGIN_ALLOW_THREADS
    $function
    Py_END_ALLOW_THREADS
    if ($source==0 || $source==INVALID_HANDLE_VALUE)  {
        $cleanup
        return PyWin_SetAPIError("$name");
    }
}

%apply long {HINTERNET};
typedef long HINTERNET;
typedef HINTERNET PyHINTERNET;
%{
PyObject *PyObject_FromHINTERNET(HINTERNET hi)
{
    return new PyHINTERNET(hi);
}
#define PyHINTERNET HINTERNET // Use a #define so we can undef it later if we need the true defn.
%}

%typemap(python,in) HINTERNET {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target, FALSE))
		return NULL;
}

%typemap(python,in) PyHINTERNET {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target, FALSE))
		return NULL;
}

%typemap(python,in) PyHINTERNET INPUT_NULLOK {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target, TRUE))
		return NULL;
}

%typemap(python,ignore) PyHINTERNET *OUTPUT(HINTERNET temp)
{
  $target = &temp;
}

%typemap(python,out) PyHINTERNET {
  $target = PyObject_FromHINTERNET($source);
}

%typemap(python,argout) PyHINTERNET *OUTPUT {
    PyObject *o;
    o = PyObject_FromHINTERNET(*$source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

// @pyswig |InternetSetCookie|Creates a cookie associated with the specified URL.
BOOLAPI InternetSetCookie(
    TCHAR *lpszUrl, // @pyparm string|url||
    TCHAR *INPUT_NULLOK, // @pyparm string|lpszCookieName||
    TCHAR *lpszCookieData // @pyparm string|data||
);

%{
// @pyswig string|InternetGetCookie|Retrieves the cookie for the specified URL
PyObject *PyInternetGetCookie(PyObject *self, PyObject *args)
{
    PyObject *obUrl, *obCookieName;
    PyObject *ret = NULL;
    BOOL ok = FALSE;
    DWORD cb = 0;
    TCHAR *szUrl = NULL, *szCookieName = NULL;
    if (!PyArg_ParseTuple(args, "OO:InternetGetCookie", &obUrl, &obCookieName))
        return NULL;
    if (!PyWinObject_AsTCHAR(obUrl, &szUrl))
        goto done;
    if (!PyWinObject_AsTCHAR(obCookieName, &szCookieName, TRUE))
        goto done;
    InternetGetCookie(szUrl, szCookieName, NULL, &cb);
    if (!cb) { // assume this means some other failure.
        PyWin_SetAPIError("InternetGetCookie");
        goto done;
    }
    // cb includes the NULL - Python adds one for the null.
    ret = PyString_FromStringAndSize(NULL, cb-1);
    Py_BEGIN_ALLOW_THREADS
    ok = InternetGetCookie(szUrl, szCookieName, PyString_AS_STRING(ret), &cb);
    Py_END_ALLOW_THREADS
    if (!ok) {
        PyWin_SetAPIError("InternetGetCookie");
        goto done;
    }
done:
    if (!ok) {
        Py_XDECREF(ret);
        ret = NULL;
    }
    if (szUrl) PyWinObject_FreeTCHAR(szUrl);
    if (szCookieName) PyWinObject_FreeTCHAR(szCookieName);
    return ret;
}
%}
%native (InternetGetCookie) PyInternetGetCookie;

// @pyswig |InternetAttemptConnect|Attempts to make a connection to the Internet.
BOOLAPI InternetAttemptConnect(
    DWORD reserved); // @pyparm int|reserved||

// @pyswig |InternetCheckConnection|Allows an application to check if a connection to the Internet can be established
BOOLAPI InternetCheckConnection(
    TCHAR *lpszUrl, // @pyparm string|url||
    DWORD flags, // @pyparm int|flags||
    DWORD reserved);

// @pyswig |InternetGoOnline|Prompts the user for permission to initiate connection to a URL.
BOOLAPI InternetGoOnline(
    TCHAR *lpszUrl, // @pyparm string|url||
    HWND hwnd, // @pyparm int|hwnd||
    DWORD reserved);

// @pyswig |InternetCloseHandle|
// @comm It should not be necessary to call this function - all handles are
// <o PyHINTERNET> objects, so can have their Close method called, and will
// otherwise be automatically closed.
BOOLAPI InternetCloseHandle(
    HINTERNET handle // @pyparm <o PyHINTERNET>|handle||
);

// @pyswig |InternetConnect|Opens an FTP, Gopher, or HTTP session for a given site.
PyHINTERNET InternetConnect(
    HINTERNET hInternet, // <o PyHINTERNET>|hInternet||Valid HINTERNET handle returned by a previous call to <om win32inet.InternetOpen>.
    TCHAR *lpszServerName, // string|serverName||A string that contains the host name of an Internet
                           // server. Alternately, the string can contain the IP number of the site,
                           // in ASCII dotted-decimal format (for example, 11.0.1.45).
    int nServerPort, // int|serverPort||Number of the TCP/IP port on the server to connect to.
                    // These flags set only the port that will be used. The service is set by 
                    // the value of dwService. This can be one of the INTERNET_DEFAULT_*_PORT
                    // constants or INTERNET_INVALID_PORT_NUMBER, which uses the default
                    // port for the service specified by dwService. 
    TCHAR *INPUT_NULLOK, // @pyparm string|lpszUsername||A string that contains the name of the user
                         // to log on. If this parameter is None, the function uses an appropriate
                         // default, except for HTTP; a NULL parameter in HTTP causes the server
                         // to return an error. For the FTP protocol, the default is "anonymous". 
    TCHAR *INPUT_NULLOK, // @pyparm string|lpszPassword||Address of a null-terminated string that
                         // contains the password to use to log on. If both lpszPassword
                         // and lpszUsername are None, the function uses the default
                         // "anonymous" password. In the case of FTP, the default password
                         // is the user's e-mail name. If lpszPassword is None, but lpszUsername
                         // is not None, the function uses a blank password.
    DWORD dwService, // @pyparm int|dwService||Iinteger value that contains the type of service to
                     // access. This can be one of INTERNET_SERVICE_FTP, INTERNET_SERVICE_GOPHER,
                     // or INTERNET_SERVICE_HTTP.
    DWORD dwFlags,   // @pyparm int|flags||Integer value that contains the flags specific to
                     // the service used. When the value of dwService is INTERNET_SERVICE_FTP,
                     // INTERNET_FLAG_PASSIVE causes the application to use passive FTP semantics.
    DWORD context   // @pyparm int|context||Must be zero.
);

// @pyswig |InternetOpen|Initializes an application's use of the Microsoft® Win32® Internet functions.
PyHINTERNET InternetOpen(
    TCHAR *lpszAgent, // @pyparm string|agent||A string that contains the name of the application
                      // or entity calling the Internet functions. This name is used as the user
                      // agent in the HTTP protocol. 
    DWORD dwAccessType, // pyparm int|accessType||dwAccessType|Type of access required. This can be one
                        // of INTERNET_OPEN_TYPE_DIRECT, INTERNET_OPEN_TYPE_PRECONFIG,
                        // INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY or
                        // INTERNET_OPEN_TYPE_PROXY
    TCHAR *INPUT_NULLOK, // @pyparm string|proxyName||
    TCHAR *INPUT_NULLOK, // @pyparm string|proxyBypass||
    DWORD dwFlags // @pyparm int|flags||
);

%{
// @pyswig <o PyHINTERNET>|InternetOpenUrl|Opens a resource specified by a
// complete FTP, Gopher, or HTTP URL.
PyObject *PyInternetOpenUrl(PyObject *self, PyObject *args)
{
    PyObject *obURL, *obHeaders, *obH;
    TCHAR *szURL = NULL;
    TCHAR *szHeaders = NULL;
    DWORD headerLen = 0;
    HINTERNET hiret, hiin;
    DWORD flags;
    PyObject *ret = NULL;
    // @pyparm <o PyHINTERNET>|hInternet||
    // @pyparm string|url||A string that contains the URL to begin reading. Only URLs beginning with ftp:, gopher:, http:, or https: are supported.
    // @pyparm string|headers||a string variable that contains the headers to be sent to the HTTP server.
    // @pyparm int|flags||
    if (!PyArg_ParseTuple(args, "OOOi:InternetOpenUrl", &obH, &obURL, &obHeaders, &flags))
        return NULL;
    if (!PyWinObject_AsHANDLE(obH, (HANDLE *)&hiin, FALSE))
        goto done;
    if (!PyWinObject_AsString(obURL, &szURL, FALSE))
        goto done;
    if (!PyWinObject_AsString(obHeaders, &szHeaders, TRUE, &headerLen))
        goto done;
    Py_BEGIN_ALLOW_THREADS
    hiret = InternetOpenUrl(hiin, szURL, szHeaders, headerLen, flags, 0);
    Py_END_ALLOW_THREADS
    if (!hiret) {
        PyWin_SetAPIError("InternetOpenUrl");
        goto done;
    }
    ret = PyObject_FromHINTERNET(hiret);
done:
    if (szURL)
        PyWinObject_FreeTCHAR(szURL);
    if (szHeaders)
        PyWinObject_FreeTCHAR(szHeaders);
    return ret;
}
%}
%native (InternetOpenUrl) PyInternetOpenUrl;


%{
// @pyswig string|InternetCanonicalizeUrl|Canonicalizes a URL, which includes
// converting unsafe characters and spaces into escape sequences.
PyObject *PyInternetCanonicalizeUrl(PyObject *self, PyObject *args)
{
    DWORD flags = 0;
    TCHAR *szURL = NULL;
    TCHAR *buf = NULL;
    PyObject *ret = NULL;
    PyObject *obURL;
    // @pyparm string|url||The URL to canonicalize.
    // @pyparm int|flags|0|integer value that contains the flags that control
    // canonicalization. This can be one of the following values:
    // @flag ICU_BROWSER_MODE|Does not encode or decode characters after "#" or "?", and does not remove trailing white space after "?". If this value is not specified, the entire URL is encoded and trailing white space is removed. 
    // @flag ICU_DECODE|Converts all %XX sequences to characters, including escape sequences, before the URL is parsed. 
    // @flag ICU_ENCODE_PERCENT|Encodes any percent signs encountered. By default, percent signs are not encoded. This value is available in Microsoft® Internet Explorer 5 and later versions of the Win32® Internet functions. 
    // @flag ICU_ENCODE_SPACES_ONLY|Encodes spaces only. 
    // @flag ICU_NO_ENCODE|Does not convert unsafe characters to escape sequences. 
    // @flag ICU_NO_META|Does not remove meta sequences (such as "." and "..") from the URL. 
    // If no flags are specified (dwFlags = 0), the function converts all unsafe characters and meta sequences (such as \.,\ .., and \...) to escape sequences.
    if (!PyArg_ParseTuple(args, "O|i:InternetCanonicalizeUrl", &obURL, &flags))
        return NULL;
    if (!PyWinObject_AsString(obURL, &szURL, FALSE))
        return NULL;
    DWORD cch = INTERNET_MAX_URL_LENGTH + 1;
    buf = (TCHAR *)malloc(cch * sizeof(TCHAR));
    if (!buf) {
        PyErr_NoMemory();
        goto done;
    }
    if (!InternetCanonicalizeUrl(szURL, buf, &cch, flags)) {
        if (GetLastError()!= ERROR_INSUFFICIENT_BUFFER) {
            PyWin_SetAPIError("InternetCanonicalizeUrl");
            goto done;
        }
        // size includes NULL
        buf = (TCHAR *)realloc(buf, cch * sizeof(TCHAR));
        if (!buf) {
            PyErr_NoMemory();
            goto done;
        }
        if (!InternetCanonicalizeUrl(szURL, buf, &cch, flags)) {
            PyWin_SetAPIError("InternetCanonicalizeUrl");
            goto done;
        }
    }
    ret = PyWinObject_FromTCHAR(buf);
done:
    if (buf) free(buf);
    if (szURL) PyWinObject_FreeTCHAR(szURL);
    return ret;
}
%}
%native (InternetCanonicalizeUrl) PyInternetCanonicalizeUrl;

%{
// @pyswig int, string|InternetGetLastResponseInfo|Retrieves the last Win32® Internet function error description or server response on the thread calling this function.
PyObject *PyInternetGetLastResponseInfo(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":InternetGetLastResponseInfo"))
        return NULL;
    DWORD err;
    DWORD cch = 0;
    TCHAR *buf = NULL;
    InternetGetLastResponseInfo(&err, NULL, &cch);
    if (!cch) { // assume this means some other failure.
        PyWin_SetAPIError("InternetGetLastResponseInfo");
        return NULL;
    }
    cch += 1; // size does NOT include NULL
    buf = (TCHAR *)malloc(cch * sizeof(TCHAR));
    if (!buf) {
        PyErr_NoMemory();
        return NULL;
    }
    if (!InternetGetLastResponseInfo(&err, buf, &cch)) {
        PyWin_SetAPIError("InternetGetLastResponseInfo");
        free(buf);
        return NULL;
    }
    PyObject *obMsg = PyWinObject_FromTCHAR(buf);
    free(buf);
    return Py_BuildValue("iN", err, obMsg);
}
%}
%native (InternetGetLastResponseInfo) PyInternetGetLastResponseInfo;

%{
// @pyswig string|InternetReadFile|Reads data from a handle opened by the
// <om win32inet.InternetOpenUrl>, <om win32inet.FtpOpenFile>,
// <om win32inet.GopherOpenFile>, or <om win32inet.HttpOpenRequest> function.
PyObject *PyInternetReadFile(PyObject *self, PyObject *args)
{
    PyObject *ret = NULL;
    PyObject *obH;
    DWORD size;
    DWORD read = 0;
    TCHAR *buf = NULL;
    HINTERNET hiin;
    BOOL ok;
    // @pyparm <o PyHINTERNET>|hInternet||
    // @pyparm int|size||Number of bytes to read.
    if (!PyArg_ParseTuple(args, "Oi:InternetReadFile", &obH, &size))
        return NULL;
    // todo - allow buffer of size, like win32file - but who really cares for
    // this?  No asynch IO, so only advantage is less heap thrashing.
    if (size==0)
        return PyErr_Format(PyExc_ValueError, "Can't read zero bytes");
    buf = (TCHAR *)malloc(size);
    if (!buf) {
        PyErr_NoMemory();
        return NULL;
    }
    if (!PyWinObject_AsHANDLE(obH, (HANDLE *)&hiin, FALSE))
        goto done;
    Py_BEGIN_ALLOW_THREADS
    ok = InternetReadFile(hiin, buf, size, &read);
    Py_END_ALLOW_THREADS
    if (!ok) {
        PyWin_SetAPIError("InternetReadFile");
        goto done;
    }
    ret = PyString_FromStringAndSize(buf, read);
    // @rdesc The result will be a string of zero bytes when the end is reached.
done:
    if (buf) free(buf);
    return ret;
}
%}
%native (InternetReadFile) PyInternetReadFile;

// @pyswig <o PyINTERNET>|FtpOpenFile|Initiates access to a remote file on an FTP server for reading or writing.
HINTERNET FtpOpenFile(
    HINTERNET hConnect, // @pyparm <o PyHINTERNET>|hConnect||Valid HINTERNET handle to an FTP session.
    TCHAR *lpszFileName, // @pyparm string|filename||The name of the file to access on the remote system.
    DWORD dwAccess, // @pyparm int|access||Integer value that determines how the file will be accessed. This can be GENERIC_READ or GENERIC_WRITE, but not both.
    DWORD dwFlags,  // @pyparm int|flags||Integer value that contains the conditions under which the
                    // transfers occur. The application should select one transfer type and
                    // any of the flags that indicate how the caching of the file will be
                    // controlled.  The transfer type can be one of the FTP_TRANSFER_TYPE* values
    DWORD dwContext // @pyparm int|context||Most be zero
);

// @pyswig <o PyHINTERNET>|FtpCommand|Allows an application to send commands directly to an FTP server.
BOOLAPI FtpCommand(
    HINTERNET hConnect, // @pyparm <o PyHINTERNET>|hConnect||Valid HINTERNET handle to an FTP session.
    BOOL fExpectResponse, // @pyparm bool|fExpectResponse||Boolean value that indicates whether or not
                          // the application expects a response from the FTP server.
                          // This must be set to True if a response is expected, or False otherwise. 
    DWORD dwFlags, // @pyparm int|flags||Unsigned long integer value that contains the flags that
                   // control this function. This can be set to either FTP_TRANSFER_TYPE_ASCII or
                   // FTP_TRANSFER_TYPE_BINARY
    TCHAR *lpszCommand, // @pyparm string|command||The command to send to the FTP server. 
    DWORD dwContext,// @pyparm int|context||Must be zero.
    PyHINTERNET *OUTPUT
);

%init %{
    PyDict_SetItemString(d, "error", PyWinExc_ApiError);
    HMODULE hmod = GetModuleHandle("wininet.dll");
    assert(hmod);
    PyWin_RegisterErrorMessageModule(INTERNET_ERROR_BASE,
                                     INTERNET_ERROR_LAST,
                                     hmod);
%}
