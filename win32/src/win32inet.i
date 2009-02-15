/* File : win32inet.i */
// @doc
%module win32inet // An interface to the Windows internet (wininet) API
%{
// #define UNICODE
// #define _UNICODE
#include "Windows.h"
#include "WinInet.h"
#undef BOOLAPI // wininet.h defines this!
%}

%include "typemaps.i"
%include "pywin32.i"

%{
#undef PyHANDLE // undef earlier define, so we are back to the class.
#include "pywinobjects.h"

void CALLBACK PyHINTERNET_StatusChange(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength);
PyObject *PyObject_FromHINTERNET(HINTERNET hi);

// The Context passed to the status callback function holds 3 PyObject pointers:
//	1. The PyHINTERNET handle itself, so that that the m_handle can
//		be cleared when the handle is closed automatically by the closure
//		of its parent handle.  This doesn't actually hold a reference,
//		so we depend on the handle's Close method to set it to NULL.
//	2. The python callback function, which can be set by InternetSetStatusCallback,
//		or InternetSetOption with INTERNET_OPTION_CALLBACK.
//	3. The Context object to be passed to above function, which can be anything.
class PyCallbackContext{
	public:
	PyObject *obPyHINTERNET;
	PyObject *obCallback;
	PyObject *obContext;
	PyCallbackContext(PyObject *context, HINTERNET hParent=NULL){
		obPyHINTERNET=NULL;
		// Handles inherit the callback function of their parent, so query the
		// parent handle for its python callback so we can emulate this.
		PyCallbackContext *parent_context;
		DWORD bufsize=sizeof(parent_context);
		if (hParent
			&&InternetQueryOption(hParent, INTERNET_OPTION_CONTEXT_VALUE,
				&parent_context, &bufsize)
			&&parent_context
			&&parent_context->obCallback)
			obCallback=parent_context->obCallback;
		else
			obCallback=Py_None;
		Py_INCREF(obCallback);

		if (context)
			obContext=context;
		else
			obContext=Py_None;
		Py_INCREF(obContext);
	}
	~PyCallbackContext(){
		Py_XDECREF(obCallback);
		Py_XDECREF(obContext);
		}
};

// NOTE: The PyHINTERNET class is only suitable for HINTERNET's returned
// from the win32inet functions.  The WinHttp functions should use a
// HWINHTTP.

// @object PyHINTERNET|An object that wraps a HINTERNET handle.  When the
// handle object is destroyed, it is automatically closed.
// See the <o PyHANDLE> object for more details.
class PyHINTERNET : public PyHANDLE
{
public:
    PyHINTERNET(HANDLE hInit) : PyHANDLE(hInit) {
		// Register generic callback function if handle has not already inherited
		//	the callback from its parent handle.
		DWORD bufsize;
		void *callback_function;
		bufsize=sizeof(callback_function);
		if (!InternetQueryOption(hInit, INTERNET_OPTION_CALLBACK, &callback_function, &bufsize)){
			PyWin_SetAPIError("InternetQueryOption");
			return;
			}
		if (callback_function==NULL)
			if (InternetSetStatusCallback(hInit, PyHINTERNET_StatusChange) == INTERNET_INVALID_STATUS_CALLBACK){
				PyWin_SetAPIError("InternetSetStatusCallback");
				return;
				}

		// Some functions will already have set the callback context
		PyCallbackContext *context;
		bufsize=sizeof(context);
		if (!InternetQueryOption(hInit, INTERNET_OPTION_CONTEXT_VALUE, &context, &bufsize)){
			PyWin_SetAPIError("InternetQueryOption");
			return;
			}
		if (context==NULL){
			// Currently, all functions that create a handle from a parent handle
			//	already set the context.  Otherwise, will need to pass the parent
			//	handle to this constructor.
			context=new PyCallbackContext(NULL, NULL);
			if (context==NULL){
				PyErr_NoMemory();
				return;
				}
			if (!InternetSetOption(hInit, INTERNET_OPTION_CONTEXT_VALUE, &context, bufsize)){
				delete context;
				PyWin_SetAPIError("InternetSetOption");
				return;
				}
			}
		context->obPyHINTERNET=this;
    }
   virtual BOOL Close(void) {
		BOOL ret=TRUE;
		if (m_handle){
			HINTERNET h=m_handle;
			m_handle = 0; // don't try again!
			// When this object is destroyed, need to make sure the callback function
			//	doesn't try to reference it.
			PyCallbackContext *context;
			DWORD bufsize=sizeof(context);
			if (InternetQueryOption(h, INTERNET_OPTION_CONTEXT_VALUE, &context, &bufsize)
				&&context)
				context->obPyHINTERNET=NULL;
			ret=InternetCloseHandle(h);
			if (!ret)
				PyWin_SetAPIError("InternetCloseHandle");
			}
        return ret;
    }
    virtual const char *GetTypeName() {return "PyHINTERNET";}
    void ClearHandle(){
		m_handle=NULL;
		}
};

// Convert status information passed to Callback to Python Object to
//	be passed to Python callback
PyObject *PyWinObject_FromStatusInformation(DWORD status, void *buf, DWORD bufsize)
{
	/* Statuses documented as passing NULL:
		INTERNET_STATUS_CLOSING_CONNECTION, INTERNET_STATUS_CONNECTION_CLOSED,
		INTERNET_STATUS_RECEIVING_RESPONSE, INTERNET_STATUS_SENDING_REQUEST
	*/
	if (buf==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	switch(status){
		case INTERNET_STATUS_COOKIE_RECEIVED:
		case INTERNET_STATUS_COOKIE_SENT:
		case INTERNET_STATUS_RESPONSE_RECEIVED:
		case INTERNET_STATUS_REQUEST_SENT:{
			// Buffer is a pointer to a DWORD.
			DWORD *bytes_received=(DWORD *)buf;
			return PyLong_FromUnsignedLong(*bytes_received);
			}
		case INTERNET_STATUS_HANDLE_CREATED:
		case INTERNET_STATUS_REQUEST_COMPLETE:{
			// buf points to INTERNET_ASYNC_RESULT struct
			INTERNET_ASYNC_RESULT *ias=(INTERNET_ASYNC_RESULT *)buf;
			return Py_BuildValue("{s:N, s:k}",
				"Result", PyWinLong_FromHANDLE((HANDLE)ias->dwResult),
				"Error", ias->dwError);			
			}
		case INTERNET_STATUS_RESOLVING_NAME:
			return PyWinObject_FromTCHAR((TCHAR *)buf);
		// This always returns a character string, even when compiled with UNICODE defined
		case INTERNET_STATUS_NAME_RESOLVED:
		// ??? MSDN claims the 2 below return pointer to SOCKADDR struct,
		//	but it appears to be a plain string ??? 
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
			return PyString_FromString((char *)buf);
		case INTERNET_STATUS_COOKIE_HISTORY:{	// InternetCookieHistory struct
			InternetCookieHistory *ich=(InternetCookieHistory *)buf;
			return Py_BuildValue("{s:N, s:N, s:N, s:N}",
				"Accepted", PyBool_FromLong(ich->fAccepted),
				"Leashed", PyBool_FromLong(ich->fLeashed),
				"Downgraded", PyBool_FromLong(ich->fDowngraded),
				"Rejected", PyBool_FromLong(ich->fRejected));
			}
		case INTERNET_STATUS_HANDLE_CLOSING:
			// Documentation does not specify what the buffer contains, but it
			//	appears to be the handle itself.
			return PyWinLong_FromHANDLE(*(HANDLE *)buf);

		// Below are not documented, hopefully will be caught by NULL check above
		case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		case INTERNET_STATUS_DETECTING_PROXY:
		default:
			// Any we don't know about, just return raw data.  This is probably going to be
			//	useless to calling python app, as it may be a pointer to anything.  Should
			//	probably just throw an error to avoid confusion in the future if more statuses
			//	are recognized.
			return PyString_FromStringAndSize((char *)buf, bufsize);
	}
}

// Callback is attached to all PyHINTERNET's to catch the
// INTERNET_STATUS_HANDLE_CLOSING event.  This is done because when a parent
// handle is closed, any handles created from it are automatically closed.
// If the Pytbon object is not aware that its handle is closed, it will
// attempt to close it again.  If another handle has already been created
// with the same handle value, the new handle will be closed prematurely.
void CALLBACK PyHINTERNET_StatusChange(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	// According to the docs, the callback won't be executed if the context is NULL,
	//	but just to be safe...
	if (!dwContext)
		return;

	// Acquire the thread lock, since this is executed asynchronously
	CEnterLeavePython _celp;
	// Call the python callback function if one has been registered
	PyCallbackContext *context=(PyCallbackContext *)dwContext;
	if (context->obCallback!=Py_None){
		PyObject *args=Py_BuildValue("NOkN",
			PyWinLong_FromHANDLE(hInternet),
			context->obContext,
			dwInternetStatus,
			PyWinObject_FromStatusInformation(dwInternetStatus,
				lpvStatusInformation, dwStatusInformationLength));
		if (args==NULL)
			PyErr_Print();
		else{
			PyObject *obret=PyObject_Call(context->obCallback, args, NULL);
			Py_DECREF(args);
			if (obret==NULL)
				PyErr_Print();
			else
				Py_DECREF(obret);
			}
		}
	
	// When handle is closed automatically by the closure of its parent handle,
	//	must make sure the handle is cleared, so that when the Python
	//	object is destroyed it won't attempt to close the handle again.
	//	PyHINTERNET::Close sets obPyHINTERNET to NULL so that we
	//	won't try to access the object after it's destroyed.
	if (dwInternetStatus==INTERNET_STATUS_HANDLE_CLOSING){
		if (context->obPyHINTERNET)
			((PyHINTERNET *)context->obPyHINTERNET)->ClearHandle();
		delete context;
		}
}
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


typedef HANDLE HINTERNET;

typedef HINTERNET PyHINTERNET;
%{
PyObject *PyObject_FromHINTERNET(HINTERNET hi)
{
	/* Check if the handle has already been encapsulated in a PyHINTERNET, and return
		a reference to the existing object if so.
	*/
	PyCallbackContext *context;
	DWORD bufsize=sizeof(context);
	if (InternetQueryOption(hi, INTERNET_OPTION_CONTEXT_VALUE, &context, &bufsize)
			&&context
			&&context->obPyHINTERNET){
		Py_INCREF(context->obPyHINTERNET);
		return context->obPyHINTERNET;
		}
	
	PyHINTERNET *ret=new PyHINTERNET(hi);
	if (ret==NULL)
		return PyErr_NoMemory();
	if (PyErr_Occurred()){
		Py_DECREF(ret);
		ret = NULL;
		}
	return ret;
}
#define PyHINTERNET HINTERNET // Use a #define so we can undef it later if we need the true defn.
%}

%typemap(python,in) HINTERNET {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}

%typemap(python,in) PyHINTERNET {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}

%typemap(python,in) PyHINTERNET INPUT_NULLOK {
    if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
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
    DWORD cch, cchallocated = 0;
    TCHAR *buf=NULL, *szUrl = NULL, *szCookieName = NULL;
    if (!PyArg_ParseTuple(args, "OO:InternetGetCookie",
		&obUrl,		// @pyparm string|Url||Site for which to retrieve cookie
		&obCookieName))	// @pyparm string|CookieName||Name of cookie (documented on MSDN as not implemented)
        return NULL;
    if (!PyWinObject_AsTCHAR(obUrl, &szUrl))
        goto done;
    if (!PyWinObject_AsTCHAR(obCookieName, &szCookieName, TRUE))
        goto done;
    InternetGetCookie(szUrl, szCookieName, NULL, &cchallocated);
    if (!cchallocated) { // assume this means some other failure.
        PyWin_SetAPIError("InternetGetCookie");
        goto done;
    }
    // Note: the docs for InternetGetCookie appear to lie: when NULL is passed
    // you get back the number of *bytes* needed, not TCHARs - however, if
    // NULL is not passed, you do get TCHARs back.  In other words, if UNICODE
    // is defined, 'cchallocated' is double the value we get back for 'cch' in
    // our next call which actually gets the data. However - we don't rely on
    // this behaviour - it just means we actually end up allocating double the
    // buffer we need.
    // The number of chars theoretically includes the \0 - but see below.
    buf = (TCHAR *)malloc(cchallocated * sizeof(TCHAR));
    if (buf==NULL){
		PyErr_NoMemory();
		goto done;
		}
    cch = cchallocated;
    Py_BEGIN_ALLOW_THREADS
    ok = InternetGetCookie(szUrl, szCookieName, buf, &cch);
    Py_END_ALLOW_THREADS
    if (!ok)
        PyWin_SetAPIError("InternetGetCookie");
    else {
        // Note that on win2k only, and only when UNICODE is defined, we
        // see 'cch' be one less than we expect - ie, it is the number of
        // chars *not* including the NULL.
#ifdef UNICODE
        if (LOBYTE(LOWORD(GetVersion())) <= 5 && cch && cch < cchallocated &&
            buf[cch-1] != _T('\0'))
            cch += 1;
#endif
        ret=PyWinObject_FromTCHAR(buf, cch-1);
    }
done:
    if (buf) free(buf);
    if (szUrl) PyWinObject_FreeTCHAR(szUrl);
    if (szCookieName) PyWinObject_FreeTCHAR(szCookieName);
    return ret;
}
%}
%native (InternetGetCookie) PyInternetGetCookie;

// @pyswig |InternetAttemptConnect|Attempts to make a connection to the Internet.
DWORDAPI InternetAttemptConnect(
    DWORD reserved=0); // @pyparm int|Reserved|0|Use only 0.

// @pyswig |InternetCheckConnection|Allows an application to check if a connection to the Internet can be established
BOOLAPI InternetCheckConnection(
    TCHAR *INPUT_NULLOK,	// @pyparm string|Url||Url to attempt to connect to, can be None
    DWORD flags=0,			// @pyparm int|Flags|0|FLAG_ICC_FORCE_CONNECTION is only defined flag
    DWORD reserved=0);		// @pyparm int|Reserved|0|Use only 0.

// @pyswig |InternetGoOnline|Prompts the user for permission to initiate connection to a URL.
BOOLAPI InternetGoOnline(
    TCHAR *lpszUrl,	// @pyparm string|Url||Web site to connect to
    HWND hwnd=NULL,		// @pyparm int|Parent|None|Handle to parent window
    DWORD Flags=0);	// @pyparm int|Flags|0|INTERNET_GOONLINE_REFRESH is only available flag

// @pyswig |InternetCloseHandle|
// @comm It should not be necessary to call this function - all handles are
// <o PyHINTERNET> objects, so can have their Close method called, and will
// otherwise be automatically closed.
BOOLAPI InternetCloseHandle(
    HINTERNET handle // @pyparm <o PyHINTERNET>|handle||
);

// @pyswig |InternetConnect|Opens an FTP, Gopher, or HTTP session for a given site.
// @comm Accepts keyword args
%{
PyObject *PyInternetConnect(PyObject *self, PyObject *args, PyObject *kwargs)
{
	HINTERNET hInternet, hret=NULL;
	PyObject *obhInternet, *obServerName, *obUsername, *obPassword, *obContext=Py_None;
	TCHAR *ServerName=NULL, *Username=NULL, *Password=NULL;
	WORD ServerPort;
	DWORD Service, Flags;
	PyObject *ret =	NULL;
	PyCallbackContext *context = NULL;
	static char *keywords[]={"Internet","ServerName","ServerPort","Username","Password","Service","Flags","Context", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOHOOkk|O:InternetConnect", keywords,
		&obhInternet,	// @pyparm <o PyHINTERNET>|Internet||Valid HINTERNET handle returned by a previous call to <om win32inet.InternetOpen>.
		&obServerName,	// @pyparm string|ServerName||A string that contains the host name of an Internet
						//	server. Alternately, the string can contain the IP number of the site,
						//	in ASCII dotted-decimal format (for example, 11.0.1.45).
		&ServerPort,	// @pyparm int|ServerPort||Number of the TCP/IP port on the server to connect to.
						//	These flags set only the port that will be used. The service is set by 
						//	the value of dwService. This can be one of the INTERNET_DEFAULT_*_PORT
						//	constants or INTERNET_INVALID_PORT_NUMBER, which uses the default
						//	port for the service specified by dwService. 
		&obUsername,	// @pyparm string|Username||A string that contains the name of the user
						//	to log on. If this parameter is	None, the function uses	an appropriate
						//	default, except	for	HTTP; a	NULL parameter in HTTP causes the server
						//	to return an error.	For	the	FTP	protocol, the default is "anonymous". 
		&obPassword,	// @pyparm string|Password||Address	of a null-terminated string	that
						//	contains the password to use to	log	on.	If both	Password
						//	and	Username are None, the function	uses the default
						//	"anonymous"	password. In the case of FTP, the default password
						//	is the user's e-mail name. If lpszPassword is None,	but	lpszUsername
						//	is not None, the function uses a blank password.
		&Service,		// @pyparm int|Service||Iinteger value that contains the type	of service to
						//	access.	This can be	one	of INTERNET_SERVICE_FTP, INTERNET_SERVICE_GOPHER,
						//	or INTERNET_SERVICE_HTTP.
		&Flags,			// @pyparm int|Flags||Integer value that contains the flags specific to
						//	the	service	used. When the value of	dwService is INTERNET_SERVICE_FTP,
						//	INTERNET_FLAG_PASSIVE causes the application to	use	passive	FTP	semantics.
		&obContext))	// @pyparm object|Context|None|Arbitrary object to be passed to callback function
		return NULL;
	if (!PyWinObject_AsHANDLE(obhInternet, &hInternet))
		goto done;
	if (!PyWinObject_AsTCHAR(obServerName, &ServerName, FALSE))
		goto done;
	if (!PyWinObject_AsTCHAR(obUsername, &Username, TRUE))
		goto done;
	if (!PyWinObject_AsTCHAR(obPassword, &Password, TRUE))
		goto done;
	context=new PyCallbackContext(obContext, hInternet);
	if (context==NULL){
		PyErr_NoMemory();
		goto done;
		}
		
	Py_BEGIN_ALLOW_THREADS
	hret=InternetConnect(hInternet, ServerName, ServerPort, Username, Password,
		Service, Flags, (DWORD_PTR)context);
	Py_END_ALLOW_THREADS

	// In ansynchronous mode, returned handle may be NULL.  Handle will be passed to
	//	callback when it is created.
	if (hret)
		ret	= PyObject_FromHINTERNET(hret);
	else{
		DWORD err=GetLastError();
		if (err==ERROR_IO_PENDING){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("InternetConnect", err);
		}
		
	done:
	PyWinObject_FreeTCHAR(ServerName);
	PyWinObject_FreeTCHAR(Username);
	PyWinObject_FreeTCHAR(Password);
	/* Even if error occurs, a handle has been created and its callback will free the context
	if (ret==NULL && context!=NULL)
		delete context;
	*/
    return ret;
}
PyCFunction pfnPyInternetConnect = (PyCFunction)PyInternetConnect;
%}
%native (InternetConnect) pfnPyInternetConnect;

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
    DWORD dwFlags // @pyparm int|flags||Combination of INTERNET_FLAG_ASYNC,INTERNET_FLAG_FROM_CACHE, or INTERNET_FLAG_OFFLINE
);

%{
// @pyswig <o PyHINTERNET>|InternetOpenUrl|Opens a resource specified by a
// complete FTP, Gopher, or HTTP URL.
// @comm Accepts keyword args.
// @rdesc Returns None in async mode (Internet handle created with INTERNET_FLAG_ASYNC).
//	When handle is created, it will be passed to callback function of parent handle.
PyObject *PyInternetOpenUrl(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obURL, *obHeaders=Py_None, *obH, *obContext=Py_None;
	TCHAR *szURL = NULL;
	TCHAR *szHeaders = NULL;
	DWORD headerLen	= 0;
	HINTERNET hiret=NULL, hiin;
	DWORD flags=0;
	PyObject *ret =	NULL;
	PyCallbackContext *context = NULL;
	static char	*keywords[]={"Internet","Url","Headers","Flags","Context", NULL};
	// @pyparm <o PyHINTERNET>|Internet||Internet handle as returned by <om win32inet.InternetOpen>
	// @pyparm string|Url||A string that contains the URL to begin reading.  Only URLs beginning with ftp:, gopher:, http:, or https: are supported.
	// @pyparm string|Headers|None|a string	variable that contains the headers to be sent to the HTTP server.
	// @pyparm int|Flags|0|INTERNET_FLAG_*
	// @pyparm object|Context|None|An arbitrary object to be passed to the status callback function
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OkO:InternetOpenUrl", keywords,
		&obH, &obURL, &obHeaders, &flags, &obContext))
		return NULL;

	if (!PyWinObject_AsHANDLE(obH, (HANDLE *)&hiin))
		goto done;
	if (!PyWinObject_AsTCHAR(obURL,	&szURL,	FALSE))
		goto done;
	if (!PyWinObject_AsTCHAR(obHeaders,	&szHeaders,	TRUE, &headerLen))
		goto done;
	context=new PyCallbackContext(obContext, hiin);
	if (context==NULL){
		PyErr_NoMemory();
		goto done;
		}
		
	Py_BEGIN_ALLOW_THREADS
	hiret =	InternetOpenUrl(hiin, szURL, szHeaders,	headerLen, flags, (DWORD_PTR)context);
	Py_END_ALLOW_THREADS
	/*	In async mode (INTERNET_FLAG_ASYNC), returns NULL and GetLastError yields ERROR_IO_PENDING.
		When handle	is created,	it is passed to	parent handle's	callback function with status
			INTERNET_STATUS_HANDLE_CREATED in an INTERNET_ASYNC_RESULT struct.
	*/
	if (hiret)
		ret	= PyObject_FromHINTERNET(hiret);
	else{
		DWORD err=GetLastError();
		if (err==ERROR_IO_PENDING){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("InternetOpenUrl", err);
		}

done:
	PyWinObject_FreeTCHAR(szURL);
	PyWinObject_FreeTCHAR(szHeaders);
	/*
	Even if operation fails and no handle is returned, a handle is
	created and then closed, which will already have deleted the context.
	if (ret==NULL && context!=NULL)
		delete context;
	*/
	return ret;
}
PyCFunction pfnPyInternetOpenUrl = (PyCFunction)PyInternetOpenUrl;
%}
%native (InternetOpenUrl) pfnPyInternetOpenUrl;

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
    if (!PyWinObject_AsTCHAR(obURL, &szURL, FALSE))
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
    char *buf = NULL;
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
    buf = (char *)malloc(size);
    if (!buf) {
        PyErr_NoMemory();
        return NULL;
    }
    if (!PyWinObject_AsHANDLE(obH, (HANDLE *)&hiin))
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

%{
// @pyswig int|InternetWriteFile|Writes data to a handle opened by <om win32inet.FtpOpenFile>.
PyObject *PyInternetWriteFile(PyObject *self, PyObject *args)
{
    PyObject *obFile, *obBuffer;
	void *buf;
    DWORD bufsize, bytes_written;
    HINTERNET hFile;
	BOOL ok;
	
	if (!PyArg_ParseTuple(args,	"OO:InternetWriteFile",
		&obFile,	// @pyparm <o PyHINTERNET>|File||Writeable internet	handle
		&obBuffer))	// @pyparm string|Buffer||String or	buffer containing data to be written
		return NULL;
	if (!PyWinObject_AsHANDLE(obFile, &hFile))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obBuffer,	&buf, &bufsize,	FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = InternetWriteFile(hFile, buf, bufsize, &bytes_written);
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("InternetWriteFile");
	return PyLong_FromUnsignedLong(bytes_written);
}
%}
%native	(InternetWriteFile)	PyInternetWriteFile;

%{
// @pyswig <o PyHINTERNET>|FtpOpenFile|Initiates access to a remote file on an FTP server for reading or writing.
// @comm Accepts keyword args
PyObject *PyFtpOpenFile(PyObject *self, PyObject *args, PyObject *kwargs)
{
	HINTERNET hConnect, hret=NULL;
	TCHAR *FileName=NULL;
	DWORD Access, Flags;
	PyCallbackContext *context=NULL;
	PyObject *obConnect, *obFileName, *obContext=Py_None;
	PyObject *ret=NULL;
	static char *keywords[]={"Connect","FileName","Access","Flags","Context", NULL};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOkk|O:FtpOpenFile", keywords,
		&obConnect,		// @pyparm <o PyHINTERNET>|hConnect||Valid HINTERNET handle to an FTP session.
		&obFileName,	// @pyparm string|FileName||The name of the file to access on the remote system.
		&Access,		// @pyparm int|Access||Integer value that determines how the file will be accessed. This can be GENERIC_READ or GENERIC_WRITE, but not both.
		&Flags,			// @pyparm int|Flags||Integer value that contains the conditions under which the
						//	transfers occur. The application should select one transfer type and
						//	any of the flags that indicate how the caching of the file will be
						//	controlled.  The transfer type can be one of the FTP_TRANSFER_TYPE* values
		&obContext))	// @pyparm object|Context|None|Arbitrary object that will be passed to handle's callback function
		return NULL;
	if (!PyWinObject_AsHANDLE(obConnect, &hConnect))
		return NULL;
	if (!PyWinObject_AsTCHAR(obFileName, &FileName, FALSE))
		return NULL;
	context = new PyCallbackContext(obContext, hConnect);
	if (context==NULL)
		PyErr_NoMemory();
	else{
		Py_BEGIN_ALLOW_THREADS
		hret=FtpOpenFile(hConnect, FileName, Access, Flags, (DWORD_PTR)context);
		Py_END_ALLOW_THREADS
		if (hret==NULL)
			PyWin_SetAPIError("FtpOpenFile");
		else
			ret=PyObject_FromHINTERNET(hret);
		}
	PyWinObject_FreeTCHAR(FileName);
	/* Do not need to free context on error, since a handle is always created,
		but then immediately closed if an error occurs.
	if (ret==NULL && context!=NULL)
		delete context;
	*/
	return ret;
}
PyCFunction pfnPyFtpOpenFile = (PyCFunction)PyFtpOpenFile;
%}
%native (FtpOpenFile) pfnPyFtpOpenFile;

%{
// @pyswig <o PyHINTERNET>|FtpCommand|Allows an application to send commands directly to an FTP server.
// @comm This function may cause a crash on 32-bit XP and Vista due to an internal error in win32inet.dll.
// @comm Accepts keyword args
PyObject *PyFtpCommand(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obConnect, *obCommand, *obContext=Py_None, *ret=NULL;
	HINTERNET hConnect, hret=NULL;
	BOOL ExpectResponse;
	DWORD Flags;
	TCHAR *Command=NULL;
	PyCallbackContext *context=NULL;
	static char *keywords[]={"Connect","ExpectResponse","Flags","Command","Context", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OlkO|O:FtpCommand", keywords,
		&obConnect,		// @pyparm <o PyHINTERNET>|Connect||Valid HINTERNET	handle to an FTP session.
		&ExpectResponse,	// @pyparm	bool|ExpectResponse||Boolean value	that indicates whether or not
							//	the application expects a response	from the FTP server.
							//	This must be set to True if a response	is expected, or	False otherwise. 
		&Flags,	// @pyparm int|Flags||Unsigned long integer value that contains the flags that
				// control this function. This can be set to	either FTP_TRANSFER_TYPE_ASCII or
				// FTP_TRANSFER_TYPE_BINARY
		&obCommand,		// @pyparm string|Command||The command to send to the FTP server. 
		&obContext))	// @pyparm object|Context|None|Arbitrary object	to be passed to	callback
		return NULL;
	if (!PyWinObject_AsHANDLE(obConnect, &hConnect))
		return NULL;
	if (!PyWinObject_AsTCHAR(obCommand, &Command, FALSE))
		return NULL;
	context = new PyCallbackContext(obContext, hConnect);
	if (context==NULL)
		PyErr_NoMemory();
	else{
		BOOL bsuccess;
		Py_BEGIN_ALLOW_THREADS
		bsuccess=FtpCommand(hConnect, ExpectResponse, Flags, Command, (DWORD_PTR)context, &hret);
		Py_END_ALLOW_THREADS
		if (!bsuccess)
			PyWin_SetAPIError("FtpCommand");
		else{
			// Handle is only returned if ExpectResponse is True
			if (hret != NULL)
				ret=PyObject_FromHINTERNET(hret);
			else{
				Py_INCREF(Py_None);
				ret=Py_None;
				// ??? Context may be leaked here ???
				// delete context;
				}
			}
		}
	PyWinObject_FreeTCHAR(Command);
	return ret;
}
PyCFunction pfnPyFtpCommand = (PyCFunction)PyFtpCommand;
%}
%native (FtpCommand) pfnPyFtpCommand;


%{
// @pyswig object|InternetQueryOption|Retrieves an option for an internet handle
// @pyseeapi InternetQueryOption
// @rdesc The type of object returned is dependent on the option requested
PyObject *PyInternetQueryOption(PyObject *self, PyObject *args)
{
	HINTERNET h;
	DWORD option, bufsize=0;
	void *buf=NULL;
	PyObject *ret=NULL;
	if (!PyArg_ParseTuple(args, "O&k:InternetQueryOption",
		PyWinObject_AsHANDLE,
		&h,			// @pyparm <o PyHINTERNET>|hInternet||Internet handle, or None for global defaults
		&option))	// @pyparm int|Option||INTERNET_OPTION_* value
		return NULL;

	// Special handling for context object and callback function, which are both stored in
	//	a PyCallbackContext instance which is the handle's *real* context pointer.
	if (option==INTERNET_OPTION_CALLBACK || option==INTERNET_OPTION_CONTEXT_VALUE){
		PyCallbackContext *context;
		DWORD bufsize=sizeof(context);
		if (!InternetQueryOption(h, INTERNET_OPTION_CONTEXT_VALUE, &context, &bufsize))
			return PyWin_SetAPIError("InternetQueryOption");
		if (context==NULL){
			// This should not happen ...
			PyErr_SetString(PyExc_RuntimeError, "??? Internal error - PyHINTERNET does not have a PyCallbackContext ???");
			return NULL;
			}
		if (option==INTERNET_OPTION_CALLBACK){
			Py_INCREF(context->obCallback);
			return context->obCallback;
			}
		Py_INCREF(context->obContext);
		return context->obContext;
		}			
				
	InternetQueryOption(h, option, buf, &bufsize);
	DWORD rc=GetLastError();
	if (rc!=ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("InternetQueryOption", rc);

	/* ??? There is apparently a bug in INTERNET_OPTION_SECURITY_CERTIFICATE.
		Returned buffer size is always 1, so allocate a large buffer as workaround ??? */
	if (option==INTERNET_OPTION_SECURITY_CERTIFICATE && bufsize==1)
		bufsize=8192;
		
	buf=malloc(bufsize);
	if (buf==NULL)
		return PyErr_NoMemory();
	if (!InternetQueryOption(h, option, buf, &bufsize)){
		PyWin_SetAPIError("InternetQueryOption");
		goto done;
		}
	switch (option){
		// @flagh Option|Returned type
		// @flag INTERNET_OPTION_CALLBACK|Python callback function
		// @flag INTERNET_OPTION_CONTEXT_VALUE|Context object
		case INTERNET_OPTION_SEND_TIMEOUT:	// @flag INTERNET_OPTION_SEND_TIMEOUT|Int - timeout in millseconds
											// @flag INTERNET_OPTION_CONTROL_SEND_TIMEOUT|Int - timeout in millseconds							
		case INTERNET_OPTION_RECEIVE_TIMEOUT:	// @flag INTERNET_OPTION_RECEIVE_TIMEOUT|Int - timeout in millseconds
												// @flag INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT|Int - timeout in millseconds
		case INTERNET_OPTION_CODEPAGE:		// @flag INTERNET_OPTION_CODEPAGE|Int - Codepage of host part of URL
		case INTERNET_OPTION_CODEPAGE_PATH:	// @flag INTERNET_OPTION_CODEPAGE_PATH|Int - Codepage for URL
		case INTERNET_OPTION_CODEPAGE_EXTRA:	// @flag INTERNET_OPTION_CODEPAGE_EXTRA|Int - Codepage for path part of URL
		case INTERNET_OPTION_CONNECT_RETRIES:		// @flag INTERNET_OPTION_CONNECT_RETRIES|Int - Number of time to try to reconnect to host
		case INTERNET_OPTION_CONNECT_TIMEOUT:		// @flag INTERNET_OPTION_CONNECT_TIMEOUT|Int - Connection timeout in milliseconds
		case INTERNET_OPTION_CONNECTED_STATE:		// @flag INTERNET_OPTION_CONNECTED_STATE|Int - Connection state, INTERNET_STATE_*
		case INTERNET_OPTION_HANDLE_TYPE:		// @flag INTERNET_OPTION_HANDLE_TYPE|Int, INTERNET_HANDLE_TYPE_*
		case INTERNET_OPTION_ERROR_MASK:		// @flag INTERNET_OPTION_ERROR_MASK|Int, combination of INTERNET_ERROR_MASK_*
		case INTERNET_OPTION_EXTENDED_ERROR:		// @flag INTERNET_OPTION_EXTENDED_ERROR|Int, ERROR_INTERNET_*
		case INTERNET_OPTION_FROM_CACHE_TIMEOUT:		// @flag INTERNET_OPTION_FROM_CACHE_TIMEOUT|Int - Timeout in ms before cached copy is used
		case INTERNET_OPTION_IDN:		// @flag INTERNET_OPTION_IDN|Int, INTERNET_FLAG_IDN_*
		case INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER:		// @flag INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER|Int
		case INTERNET_OPTION_MAX_CONNS_PER_SERVER:		// @flag INTERNET_OPTION_MAX_CONNS_PER_SERVER|Int
		case INTERNET_OPTION_READ_BUFFER_SIZE:		// @flag INTERNET_OPTION_READ_BUFFER_SIZE|Int
		case INTERNET_OPTION_WRITE_BUFFER_SIZE:		// @flag INTERNET_OPTION_WRITE_BUFFER_SIZE|Int
		case INTERNET_OPTION_REQUEST_FLAGS:		// @flag INTERNET_OPTION_REQUEST_FLAGS|Int, combination of INTERNET_REQFLAG_*
		case INTERNET_OPTION_REQUEST_PRIORITY:	// @flag INTERNET_OPTION_REQUEST_PRIORITY|Int
		case INTERNET_OPTION_SECURITY_FLAGS:	// @flag INTERNET_OPTION_SECURITY_FLAGS|Int, SECURITY_FLAG_*
		case INTERNET_OPTION_SECURITY_KEY_BITNESS:		// @flag INTERNET_OPTION_SECURITY_KEY_BITNESS|Int		
			ret=PyLong_FromUnsignedLong(*(unsigned long *)buf);
			break;
		case INTERNET_OPTION_BYPASS_EDITED_ENTRY:		// @flag INTERNET_OPTION_BYPASS_EDITED_ENTRY|Boolean
		case INTERNET_OPTION_HTTP_DECODING:		// @flag INTERNET_OPTION_HTTP_DECODING|Boolean
		case INTERNET_OPTION_IGNORE_OFFLINE:		// @flag INTERNET_OPTION_IGNORE_OFFLINE|Boolean
			ret=PyBool_FromLong(*(BOOL *)buf);
			break;
		case INTERNET_OPTION_DATAFILE_NAME:		// @flag INTERNET_OPTION_DATAFILE_NAME|String - Name of internet cache file
		case INTERNET_OPTION_USERNAME:		// @flag INTERNET_OPTION_USERNAME|String - Username passed to InternetConnect
		case INTERNET_OPTION_PASSWORD:		// @flag INTERNET_OPTION_PASSWORD|String - Password passed to InternetConnect
		case INTERNET_OPTION_PROXY_PASSWORD:		// @flag INTERNET_OPTION_PROXY_PASSWORD|String
		case INTERNET_OPTION_PROXY_USERNAME:		// @flag INTERNET_OPTION_PROXY_USERNAME|String
		case INTERNET_OPTION_SECONDARY_CACHE_KEY:		// @flag INTERNET_OPTION_SECONDARY_CACHE_KEY|String
		case INTERNET_OPTION_SECURITY_CERTIFICATE:		// @flag INTERNET_OPTION_SECURITY_CERTIFICATE|String
		case INTERNET_OPTION_URL:		// @flag INTERNET_OPTION_URL|String
		case INTERNET_OPTION_USER_AGENT:		// @flag INTERNET_OPTION_USER_AGENT|String
			ret=PyWinObject_FromTCHAR((TCHAR *)buf);
			break;			
		case INTERNET_OPTION_CACHE_TIMESTAMPS:{		// @flag INTERNET_OPTION_CACHE_TIMESTAMPS|dict - Expiration and last modified times
			INTERNET_CACHE_TIMESTAMPS *ct=(INTERNET_CACHE_TIMESTAMPS *)buf;
			ret=Py_BuildValue("{s:N, s:N}",
				"Expires", PyWinObject_FromFILETIME(ct->ftExpires),
				"LastModified", PyWinObject_FromFILETIME(ct->ftLastModified));
			break;
			}
		case INTERNET_OPTION_HTTP_VERSION:{		// @flag INTERNET_OPTION_HTTP_VERSION|dict - HTTP_VERSION_INFO
			HTTP_VERSION_INFO *vi=(HTTP_VERSION_INFO *)buf;
			ret=Py_BuildValue("{s:k, s:k}", 
				"MajorVersion", vi->dwMajorVersion, 
				"MinorVersion", vi->dwMinorVersion);
			break;
			}
		case INTERNET_OPTION_VERSION:{	// @flag INTERNET_OPTION_VERSION|dict - INTERNET_VERSION_INFO
			INTERNET_VERSION_INFO *vi=(INTERNET_VERSION_INFO *)buf;
			ret=Py_BuildValue("{s:k, s:k}", 
				"MajorVersion", vi->dwMajorVersion,
				"MinorVersion", vi->dwMinorVersion);
			break;
			}
		case INTERNET_OPTION_PARENT_HANDLE:		// @flag INTERNET_OPTION_PARENT_HANDLE|<o PyHINTERNET>
			ret=PyWinLong_FromHANDLE(*(HINTERNET *)buf);
			break;
		case INTERNET_OPTION_PROXY:{	// @flag INTERNET_OPTION_PROXY|dict - INTERNET_PROXY_INFO
			INTERNET_PROXY_INFO *pi=(INTERNET_PROXY_INFO *)buf;
			ret=Py_BuildValue("{s:k, s:N, s:N}",
				"AccessType", pi->dwAccessType,
				"Proxy", PyWinObject_FromTCHAR((TCHAR *)pi->lpszProxy),
				"ProxyBypass", PyWinObject_FromTCHAR((TCHAR *)pi->lpszProxyBypass));
			break;
			}

		case INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO:	// @flag INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO|Not yet supported (INTERNET_DIAGNOSTIC_SOCKET_INFO)
		case INTERNET_OPTION_PER_CONNECTION_OPTION:		// @flag INTERNET_OPTION_PER_CONNECTION_OPTION|Not yet supported (INTERNET_PER_CONN_OPTION_LIST)
		case INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT:	// @flag INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT|Not yet supported (INTERNET_CERTIFICATE_INFO)

		case INTERNET_OPTION_ALTER_IDENTITY:		// @flag INTERNET_OPTION_ALTER_IDENTITY|Not supported
		case INTERNET_OPTION_ASYNC:		// @flag INTERNET_OPTION_ASYNC|Not supported
		case INTERNET_OPTION_ASYNC_ID:		// @flag INTERNET_OPTION_ASYNC_ID|Not supported
		case INTERNET_OPTION_ASYNC_PRIORITY:		// @flag INTERNET_OPTION_ASYNC_PRIORITY|Not supported
		case INTERNET_OPTION_CACHE_STREAM_HANDLE:		// @flag INTERNET_OPTION_CACHE_STREAM_HANDLE|Not supported
		case INTERNET_OPTION_CALLBACK_FILTER:		// @flag INTERNET_OPTION_CALLBACK_FILTER|Not supported
		case INTERNET_OPTION_CLIENT_CERT_CONTEXT:		// @flag INTERNET_OPTION_CLIENT_CERT_CONTEXT|Not supported
		case INTERNET_OPTION_DATA_RECEIVE_TIMEOUT:		// @flag INTERNET_OPTION_DATA_RECEIVE_TIMEOUT|Not supported
		case INTERNET_OPTION_DATA_SEND_TIMEOUT:		// @flag INTERNET_OPTION_DATA_SEND_TIMEOUT|Not supported
		case INTERNET_OPTION_CONNECT_BACKOFF:		// @flag INTERNET_OPTION_CONNECT_BACKOFF|Not supported
		case INTERNET_OPTION_CONNECT_TIME:		// @flag INTERNET_OPTION_CONNECT_TIME|Not supported
		case INTERNET_OPTION_DISABLE_AUTODIAL:		// @flag INTERNET_OPTION_DISABLE_AUTODIAL|Not supported
		case INTERNET_OPTION_DISCONNECTED_TIMEOUT:		// @flag INTERNET_OPTION_DISCONNECTED_TIMEOUT|Not supported
		case INTERNET_OPTION_IDENTITY:		// @flag INTERNET_OPTION_IDENTITY|Not supported
		case INTERNET_OPTION_IDLE_STATE:		// @flag INTERNET_OPTION_IDLE_STATE|Not supported
		case INTERNET_OPTION_KEEP_CONNECTION:		// @flag INTERNET_OPTION_KEEP_CONNECTION|Not supported
		case INTERNET_OPTION_LISTEN_TIMEOUT:		// @flag INTERNET_OPTION_LISTEN_TIMEOUT|Not supported
		case INTERNET_OPTION_OFFLINE_MODE:		// @flag INTERNET_OPTION_OFFLINE_MODE|Not supported
		case INTERNET_OPTION_OFFLINE_SEMANTICS:		// @flag INTERNET_OPTION_OFFLINE_SEMANTICS|Not supported
		case INTERNET_OPTION_POLICY:		// @flag INTERNET_OPTION_POLICY|Not supported
		case INTERNET_OPTION_RECEIVE_THROUGHPUT:		// @flag INTERNET_OPTION_RECEIVE_THROUGHPUT|Not supported
		case INTERNET_OPTION_REMOVE_IDENTITY:		// @flag INTERNET_OPTION_REMOVE_IDENTITY|Not supported
		case INTERNET_OPTION_SEND_THROUGHPUT:		// @flag INTERNET_OPTION_SEND_THROUGHPUT|Not supported

		case INTERNET_OPTION_DATAFILE_EXT:		// @flag INTERNET_OPTION_DATAFILE_EXT|Only valid for InternetSetOption
		case INTERNET_OPTION_DIGEST_AUTH_UNLOAD:	// @flag INTERNET_OPTION_DIGEST_AUTH_UNLOAD|Only valid for InternetSetOption
		case INTERNET_OPTION_END_BROWSER_SESSION:	// @flag INTERNET_OPTION_END_BROWSER_SESSION|Only valid for InternetSetOption
		case INTERNET_OPTION_REFRESH:		// @flag INTERNET_OPTION_REFRESH|Only valid for InternetSetOption
		case INTERNET_OPTION_RESET_URLCACHE_SESSION:	// @flag INTERNET_OPTION_RESET_URLCACHE_SESSION|Only valid for InternetSetOption
		case INTERNET_OPTION_SETTINGS_CHANGED:	// @flag INTERNET_OPTION_SETTINGS_CHANGED|Only valid for InternetSetOption
		default:
			PyErr_Format(PyExc_NotImplementedError, "Option %d is not supported", option);
		}
	done:
	free(buf);
	return ret;
}
%}
%native (InternetQueryOption) PyInternetQueryOption;

%{
void PyWinObject_FreeINTERNET_PROXY_INFO(INTERNET_PROXY_INFO *pipi)
{
	PyWinObject_FreeTCHAR((TCHAR *)pipi->lpszProxy);
	PyWinObject_FreeTCHAR((TCHAR *)pipi->lpszProxyBypass);
	ZeroMemory(pipi, sizeof(*pipi));
}
	
BOOL PyWinObject_AsINTERNET_PROXY_INFO(PyObject *ob, INTERNET_PROXY_INFO *pipi)
{
	static char *keywords[] = {"AccessType","Proxy","ProxyBypass", NULL};
	PyObject *obProxy, *obProxyBypass, *obdummy;
	TCHAR *Proxy=NULL, *ProxyBypass=NULL;
	BOOL bsuccess;
	ZeroMemory(pipi, sizeof(*pipi));
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError, "INTERNET_PROXY_INFO must be a dict");
		return FALSE;
		}
	obdummy=PyTuple_New(0);
	if (obdummy==NULL)
		return FALSE;
	bsuccess=PyArg_ParseTupleAndKeywords(obdummy, ob, "kOO", keywords,
			&pipi->dwAccessType,
			&obProxy,
			&obProxyBypass)
		&&PyWinObject_AsTCHAR(obProxy, &Proxy, TRUE)
		&&PyWinObject_AsTCHAR(obProxyBypass, &ProxyBypass, TRUE);
	pipi->lpszProxy=Proxy;
	pipi->lpszProxyBypass=ProxyBypass;
	if (!bsuccess)
		PyWinObject_FreeINTERNET_PROXY_INFO(pipi);
	Py_DECREF(obdummy);
	return bsuccess;
}

// @pyswig |InternetSetOption|Sets an option for an internet handle
// @pyseeapi InternetSetOption
PyObject *PyInternetSetOption(PyObject *self, PyObject *args)
{
	HINTERNET h;
	DWORD option, bufsize=0;
	void *buf=NULL;
	PyObject *obbuf, *ret=NULL;
	TCHAR *tchar_buf=NULL;
	if (!PyArg_ParseTuple(args, "O&kO:InternetSetOption",
		PyWinObject_AsHANDLE,
		&h,			// @pyparm <o PyHINTERNET>|hInternet||Internet handle, or None for global defaults
		&option,	// @pyparm int|Option||The option to set, INTERNET_OPTION_*
		&obbuf))	// @pyparm object|Buffer||Type is dependent on Option
		return NULL;
		
	// Special handling for context object and callback function, which are both stored in
	//	a PyCallbackContext instance which is the handle's *real* context pointer.
	if (option==INTERNET_OPTION_CALLBACK || option==INTERNET_OPTION_CONTEXT_VALUE){
		PyCallbackContext *context;
		DWORD bufsize=sizeof(context);
		if (!InternetQueryOption(h, INTERNET_OPTION_CONTEXT_VALUE, &context, &bufsize))
			return PyWin_SetAPIError("InternetQueryOption");
		if (context==NULL){
			// This should not happen ...
			PyErr_SetString(PyExc_RuntimeError, "??? Internal error - PyHINTERNET does not have a PyCallbackContext ???");
			return NULL;
			}
		if (option==INTERNET_OPTION_CALLBACK){
			if (!PyCallable_Check(obbuf)){
				PyErr_SetString(PyExc_TypeError,"Callback must be callable");
				return NULL;
				}
			Py_XDECREF(context->obCallback);
			Py_INCREF(obbuf);
			context->obCallback=obbuf;
			}
		else{
			Py_XDECREF(context->obContext);
			Py_INCREF(obbuf);
			context->obContext=obbuf;
			}
		Py_INCREF(Py_None);
		return Py_None;
		}	
		
	switch (option){
		// @flagh Option|Type of input object
		// @flag INTERNET_OPTION_CALLBACK|Python function called on status change
		// @flag INTERNET_OPTION_CONTEXT_VALUE|Any Python object to be passed to callback function
		case INTERNET_OPTION_SEND_TIMEOUT:	// @flag INTERNET_OPTION_SEND_TIMEOUT|Int - timeout in millseconds
											// @flag INTERNET_OPTION_CONTROL_SEND_TIMEOUT|Int - timeout in millseconds							
		case INTERNET_OPTION_RECEIVE_TIMEOUT:	// @flag INTERNET_OPTION_RECEIVE_TIMEOUT|Int - timeout in millseconds
												// @flag INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT|Int - timeout in millseconds
		case INTERNET_OPTION_CODEPAGE:		// @flag INTERNET_OPTION_CODEPAGE|Int - Codepage of host part of URL
		case INTERNET_OPTION_CODEPAGE_PATH:	// @flag INTERNET_OPTION_CODEPAGE_PATH|Codepage for URL
		case INTERNET_OPTION_CODEPAGE_EXTRA:	// @flag INTERNET_OPTION_CODEPAGE_EXTRA|Int - Codepage for path part of URL
		case INTERNET_OPTION_CONNECT_RETRIES:		// @flag INTERNET_OPTION_CONNECT_RETRIES|Int - Number of time to try to reconnect to host
		case INTERNET_OPTION_CONNECT_TIMEOUT:		// @flag INTERNET_OPTION_CONNECT_TIMEOUT|Int - Connection timeout in milliseconds
		case INTERNET_OPTION_CONNECTED_STATE:		// @flag INTERNET_OPTION_CONNECTED_STATE|Int - Connection state, INTERNET_STATE_*
		case INTERNET_OPTION_ERROR_MASK:		// @flag INTERNET_OPTION_ERROR_MASK|Int, combination of INTERNET_ERROR_MASK_*
		case INTERNET_OPTION_FROM_CACHE_TIMEOUT:		// @flag INTERNET_OPTION_FROM_CACHE_TIMEOUT|Int - Timeout in ms before cached copy is used
		case INTERNET_OPTION_IDN:		// @flag INTERNET_OPTION_IDN|Int, INTERNET_FLAG_IDN_*
		case INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER:		// @flag INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER|Int
		case INTERNET_OPTION_MAX_CONNS_PER_SERVER:		// @flag INTERNET_OPTION_MAX_CONNS_PER_SERVER|Int
		case INTERNET_OPTION_READ_BUFFER_SIZE:		// @flag INTERNET_OPTION_READ_BUFFER_SIZE|Int
		case INTERNET_OPTION_WRITE_BUFFER_SIZE:		// @flag INTERNET_OPTION_WRITE_BUFFER_SIZE|Int
		case INTERNET_OPTION_REQUEST_PRIORITY:	// @flag INTERNET_OPTION_REQUEST_PRIORITY|Int
			bufsize=sizeof(unsigned long);
			buf=malloc(bufsize);
			if (buf==NULL)
				return PyErr_NoMemory();
			*(unsigned long*)buf=PyLong_AsUnsignedLong(obbuf);
			if (*(long *)buf==-1 && PyErr_Occurred())
				goto done;
			break;
		case INTERNET_OPTION_DIGEST_AUTH_UNLOAD:	// @flag INTERNET_OPTION_DIGEST_AUTH_UNLOAD|None
		case INTERNET_OPTION_END_BROWSER_SESSION:	// @flag INTERNET_OPTION_END_BROWSER_SESSION|None
		case INTERNET_OPTION_REFRESH:		// @flag INTERNET_OPTION_REFRESH|None
		case INTERNET_OPTION_RESET_URLCACHE_SESSION:	// @flag INTERNET_OPTION_RESET_URLCACHE_SESSION|None
		case INTERNET_OPTION_SETTINGS_CHANGED:	// @flag INTERNET_OPTION_SETTINGS_CHANGED|None
			if (obbuf!=Py_None){
				PyErr_Format(PyExc_ValueError, "Object for option %d must be None", option);
				goto done;
				}
			break;
		case INTERNET_OPTION_BYPASS_EDITED_ENTRY:	// @flag INTERNET_OPTION_BYPASS_EDITED_ENTRY|Boolean
		case INTERNET_OPTION_HTTP_DECODING:		// @flag INTERNET_OPTION_HTTP_DECODING|Boolean
		case INTERNET_OPTION_IGNORE_OFFLINE:	// @flag INTERNET_OPTION_IGNORE_OFFLINE|Boolean
			bufsize=sizeof(BOOL);
			buf=malloc(bufsize);
			if (buf==NULL)
				return PyErr_NoMemory();
			*(BOOL *)buf=PyObject_IsTrue(obbuf);
			break;
		case INTERNET_OPTION_USERNAME:		// @flag INTERNET_OPTION_USERNAME|String - Username passed to InternetConnect
		case INTERNET_OPTION_PASSWORD:		// @flag INTERNET_OPTION_PASSWORD|String - Password passed to InternetConnect
		case INTERNET_OPTION_PROXY_PASSWORD:	// @flag INTERNET_OPTION_PROXY_PASSWORD|String
		case INTERNET_OPTION_PROXY_USERNAME:	// @flag INTERNET_OPTION_PROXY_USERNAME|String
		case INTERNET_OPTION_SECONDARY_CACHE_KEY:	// @flag INTERNET_OPTION_SECONDARY_CACHE_KEY|String
		case INTERNET_OPTION_USER_AGENT:	// @flag INTERNET_OPTION_USER_AGENT|String
		case INTERNET_OPTION_DATAFILE_EXT:	// @flag INTERNET_OPTION_DATAFILE_EXT|String - Extension to use for download cache file
			if (!PyWinObject_AsTCHAR(obbuf, &tchar_buf, TRUE, &bufsize))
				goto done;
			buf=tchar_buf;
			break;
		case INTERNET_OPTION_PROXY:		// @flag INTERNET_OPTION_PROXY|Dict representing INTERNET_PROXY_INFO struct
			bufsize=sizeof(INTERNET_PROXY_INFO);
			buf=malloc(bufsize);
			if (buf==NULL){
				PyErr_NoMemory();
				goto done;
				}
			if (!PyWinObject_AsINTERNET_PROXY_INFO(obbuf, (INTERNET_PROXY_INFO *)buf))
				goto done;
			break;

		case INTERNET_OPTION_HTTP_VERSION:		// @flag INTERNET_OPTION_HTTP_VERSION|Not yet supported - HTTP_VERSION_INFO
		case INTERNET_OPTION_PER_CONNECTION_OPTION:		// @flag INTERNET_OPTION_PER_CONNECTION_OPTION|Not yet supported (INTERNET_PER_CONN_OPTION_LIST)

		case INTERNET_OPTION_ALTER_IDENTITY:		// @flag INTERNET_OPTION_ALTER_IDENTITY|Not supported
		case INTERNET_OPTION_ASYNC:		// @flag INTERNET_OPTION_ASYNC|Not supported
		case INTERNET_OPTION_ASYNC_ID:		// @flag INTERNET_OPTION_ASYNC_ID|Not supported
		case INTERNET_OPTION_ASYNC_PRIORITY:		// @flag INTERNET_OPTION_ASYNC_PRIORITY|Not supported
		case INTERNET_OPTION_CACHE_STREAM_HANDLE:		// @flag INTERNET_OPTION_CACHE_STREAM_HANDLE|Not supported
		case INTERNET_OPTION_CALLBACK_FILTER:		// @flag INTERNET_OPTION_CALLBACK_FILTER|Not supported
		case INTERNET_OPTION_CLIENT_CERT_CONTEXT:		// @flag INTERNET_OPTION_CLIENT_CERT_CONTEXT|Not supported
		case INTERNET_OPTION_DATA_RECEIVE_TIMEOUT:		// @flag INTERNET_OPTION_DATA_RECEIVE_TIMEOUT|Not supported
		case INTERNET_OPTION_DATA_SEND_TIMEOUT:		// @flag INTERNET_OPTION_DATA_SEND_TIMEOUT|Not supported
		case INTERNET_OPTION_CONNECT_BACKOFF:		// @flag INTERNET_OPTION_CONNECT_BACKOFF|Not supported
		case INTERNET_OPTION_CONNECT_TIME:		// @flag INTERNET_OPTION_CONNECT_TIME|Not supported
		case INTERNET_OPTION_DISABLE_AUTODIAL:		// @flag INTERNET_OPTION_DISABLE_AUTODIAL|Not supported
		case INTERNET_OPTION_DISCONNECTED_TIMEOUT:		// @flag INTERNET_OPTION_DISCONNECTED_TIMEOUT|Not supported
		case INTERNET_OPTION_IDENTITY:		// @flag INTERNET_OPTION_IDENTITY|Not supported
		case INTERNET_OPTION_IDLE_STATE:		// @flag INTERNET_OPTION_IDLE_STATE|Not supported
		case INTERNET_OPTION_KEEP_CONNECTION:		// @flag INTERNET_OPTION_KEEP_CONNECTION|Not supported
		case INTERNET_OPTION_LISTEN_TIMEOUT:		// @flag INTERNET_OPTION_LISTEN_TIMEOUT|Not supported
		case INTERNET_OPTION_OFFLINE_MODE:		// @flag INTERNET_OPTION_OFFLINE_MODE|Not supported
		case INTERNET_OPTION_OFFLINE_SEMANTICS:		// @flag INTERNET_OPTION_OFFLINE_SEMANTICS|Not supported
		case INTERNET_OPTION_POLICY:		// @flag INTERNET_OPTION_POLICY|Not supported
		case INTERNET_OPTION_RECEIVE_THROUGHPUT:		// @flag INTERNET_OPTION_RECEIVE_THROUGHPUT|Not supported
		case INTERNET_OPTION_REMOVE_IDENTITY:		// @flag INTERNET_OPTION_REMOVE_IDENTITY|Not supported
		case INTERNET_OPTION_SEND_THROUGHPUT:		// @flag INTERNET_OPTION_SEND_THROUGHPUT|Not supported

		case INTERNET_OPTION_CACHE_TIMESTAMPS:	// @flag INTERNET_OPTION_CACHE_TIMESTAMPS|Only valid for InternetQueryOption
		case INTERNET_OPTION_HANDLE_TYPE:		// @flag INTERNET_OPTION_HANDLE_TYPE|Only valid for InternetQueryOption
		case INTERNET_OPTION_DATAFILE_NAME:		// @flag INTERNET_OPTION_DATAFILE_NAME|Only valid for InternetQueryOption
		case INTERNET_OPTION_PARENT_HANDLE:		// @flag INTERNET_OPTION_PARENT_HANDLE|Only valid for InternetQueryOption
		case INTERNET_OPTION_SECURITY_CERTIFICATE:		// @flag INTERNET_OPTION_SECURITY_CERTIFICATE|Only valid for InternetQueryOption
		case INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT:	// @flag INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT|Only valid for InternetQueryOption
		case INTERNET_OPTION_SECURITY_FLAGS:	// @flag INTERNET_OPTION_SECURITY_FLAGS|Only valid for InternetQueryOption
		case INTERNET_OPTION_SECURITY_KEY_BITNESS:		// @flag INTERNET_OPTION_SECURITY_KEY_BITNESS|Only valid for InternetQueryOption
		case INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO:	// @flag INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO|Only valid for InternetQueryOption
		case INTERNET_OPTION_VERSION:	// @flag INTERNET_OPTION_VERSION|Only valid for InternetQueryOption
		case INTERNET_OPTION_EXTENDED_ERROR:		// @flag INTERNET_OPTION_EXTENDED_ERROR|Only valid for InternetQueryOption
		case INTERNET_OPTION_REQUEST_FLAGS:		// @flag INTERNET_OPTION_REQUEST_FLAGS|Only valid for InternetQueryOption
		case INTERNET_OPTION_URL:		// @flag INTERNET_OPTION_URL|Only valid for InternetQueryOption
		default:
			PyErr_Format(PyExc_NotImplementedError, "Option %d is not supported", option);
			goto done;
		}

	if (!InternetSetOption(h, option, buf, bufsize))
		PyWin_SetAPIError("InternetSetOption");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}

	done:
	if (tchar_buf)
		PyWinObject_FreeTCHAR(tchar_buf);
	else if (buf != NULL){
		if (option==INTERNET_OPTION_PROXY)
			PyWinObject_FreeINTERNET_PROXY_INFO((INTERNET_PROXY_INFO *)buf);
		free(buf);
		}
	return ret;
}
%}
%native (InternetSetOption) PyInternetSetOption;

%{
// @object PyUrlCacheHANDLE|Handle used to enumerate the browser cache.
//	Created by <om win32inet.FindFirstUrlCacheEntry>.  Object's Close()
//	method calls FindCloseUrlCache to free the handle.
class PyUrlCacheHANDLE : public PyHANDLE
{
public:
	PyUrlCacheHANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void) {
		BOOL ret=TRUE;
		if (m_handle){
			ret=FindCloseUrlCache(m_handle);
			if (!ret)
				PyWin_SetAPIError("FindCloseUrlCache");
			m_handle = NULL;
			}
        return ret;
    }
    virtual const char *GetTypeName() {return "PyUrlCacheHANDLE";}
};

PyObject *PyWinObject_FromUrlCacheHANDLE(HANDLE h)
{	
	PyUrlCacheHANDLE *ret=new PyUrlCacheHANDLE(h);
	if (ret==NULL)
		PyErr_NoMemory();
	return ret;
}

PyObject *PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(INTERNET_CACHE_ENTRY_INFO *buf)
{
	ULARGE_INTEGER ul;
	ul.LowPart=buf->dwSizeLow;
	ul.HighPart=buf->dwSizeHigh;
	return Py_BuildValue("{s:k, s:N, s:N, s:k, s:k, s:k, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:k}",
		"StructSize", buf->dwStructSize,
		"SourceUrlName", PyWinObject_FromTCHAR(buf->lpszSourceUrlName),
		"LocalFileName", PyWinObject_FromTCHAR(buf->lpszLocalFileName),
		"CacheEntryType", buf->CacheEntryType,
		"UseCount", buf->dwUseCount,
		"HitRate", buf->dwHitRate,
		"Size", PyWinObject_FromULARGE_INTEGER(ul),
		"LastModifiedTime", PyWinObject_FromFILETIME(buf->LastModifiedTime),
		"ExpireTime", PyWinObject_FromFILETIME(buf->ExpireTime),
		"LastAccessTime", PyWinObject_FromFILETIME(buf->LastAccessTime),
		"LastSyncTime", PyWinObject_FromFILETIME(buf->LastSyncTime),
		"HeaderInfo", PyWinObject_FromTCHAR(buf->lpHeaderInfo, buf->dwHeaderInfoSize),
		"FileExtension", PyWinObject_FromTCHAR(buf->lpszFileExtension),
		"ExemptDelta", buf->dwExemptDelta);
}

// @pyswig (<o PyUrlCacheHANDLE>, dict)|FindFirstUrlCacheEntry|Initiates an enumeration of the browser cache
// @pyseeapi FindFirstUrlCacheEntry
// @rdesc Returns a handle that can be passed to <om win32inet.FindNextUrlCacheEntry>, and a dict
//	containing information for the first entry found.  Throws error code ERROR_NO_MORE_ITEMS
//	if no items are found.
// @comm Accepts keyword args
PyObject *PyFindFirstUrlCacheEntry(PyObject *self, PyObject *args, PyObject *kwargs)
{
	TCHAR *pattern=NULL;
	PyObject *obpattern=Py_None, *ret=NULL;
	static char *keywords[]={"SearchPattern", NULL};
	INTERNET_CACHE_ENTRY_INFO *buf=NULL;
	DWORD err, bufsize=512;
	HANDLE h;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:FindFirstUrlCacheEntry", keywords,
		&obpattern))	// @pyparm str|SearchPattern|None|Type of entry to find, can be 'visited:', 'cookie:', or None
		return NULL;
	if (!PyWinObject_AsTCHAR(obpattern, &pattern, TRUE))
		return NULL;
	while (1){
		if (buf)
			free(buf);
		buf=(INTERNET_CACHE_ENTRY_INFO *)malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			break;
			}
		h = FindFirstUrlCacheEntry(pattern, buf, &bufsize);
		if (h==NULL){
			err=GetLastError();
			if (err!=ERROR_INSUFFICIENT_BUFFER){
				PyWin_SetAPIError("FindFirstUrlCacheEntry", err);
				break;
				}
			}
		else{
			ret=Py_BuildValue("NN", PyWinObject_FromUrlCacheHANDLE(h), PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(buf));
			break;
			}
		}
		
	PyWinObject_FreeTCHAR(pattern);
	if (buf)
		free(buf);
	return ret;
};

// @pyswig dict|FindNextUrlCacheEntry|Continues enumeration of cached files
// @pyseeapi FindNextUrlCacheEntry
// @rdesc Returns a dict representing a INTERNET_CACHE_ENTRY_INFO strunct
// @comm Accepts keyword args
PyObject *PyFindNextUrlCacheEntry(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obh, *ret=NULL;
	static char *keywords[]={"EnumHandle", NULL};
	INTERNET_CACHE_ENTRY_INFO *buf=NULL;
	DWORD err, bufsize=512;
	HANDLE h;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:FindNextUrlCacheEntry", keywords,
		&obh))	// @pyparm <o PyUrlCacheHANDLE>|EnumHandle||Cache enumeration handle as returned by <om win32inet.FindFirstUrlCacheEntry>
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	while (1){
		if (buf)
			free(buf);
		buf=(INTERNET_CACHE_ENTRY_INFO *)malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			break;
			}
		if (FindNextUrlCacheEntry(h, buf, &bufsize)){
			ret=PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(buf);
			break;
			}

		err=GetLastError();
		if (err!=ERROR_INSUFFICIENT_BUFFER){
			PyWin_SetAPIError("FindNextUrlCacheEntry", err);
			break;
			}
		}  
  
	if (buf)
		free(buf);
	return ret;
};

// @pyswig (<o PyUrlCacheHANDLE>, dict)|FindFirstUrlCacheEntryEx|Initiates an enumeration of the browser cache
// @pyseeapi FindFirstUrlCacheEntryEx
// @rdesc Returns a handle that can be passed to <om win32inet.FindNextUrlCacheEntry>, and a dict
//	containing information for the first entry found.  Throws error code ERROR_NO_MORE_ITEMS
//	if no items are found.
// @comm Accepts keyword args
PyObject *PyFindFirstUrlCacheEntryEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
	TCHAR *pattern=NULL;
	PyObject *obpattern=Py_None, *ret=NULL;
	static char *keywords[]={"SearchPattern","Flags","Filter","GroupId", NULL};
	INTERNET_CACHE_ENTRY_INFO *buf=NULL;
	DWORD Flags=0, Filter=0;
	GROUPID GroupId=0;
	LPVOID Reserved=NULL, GroupAttributes=NULL;
	DWORD err, bufsize=512;
	HANDLE h;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OkkK:FindFirstUrlCacheEntryEx", keywords,
		&obpattern,		// @pyparm str|SearchPattern|None|Type of entry to find, can be 'visited:', 'cookie:', or None
		&Flags,			// @pyparm int|Flags|0|None currently defined
		&Filter,		// @pyparm int|Filter|0|Types of entries to return, combination of *_CACHE_ENTRY values
		&GroupId))		// @pyparm int|GroupId|0|Cache group to enumerate, use 0 for all
		return NULL;
	if (!PyWinObject_AsTCHAR(obpattern, &pattern, TRUE))
		return NULL;
	while (1){
		if (buf)
			free(buf);
		buf=(INTERNET_CACHE_ENTRY_INFO *)malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			break;
			}
		h = FindFirstUrlCacheEntryEx(pattern, Flags, Filter, GroupId, buf, &bufsize,
			GroupAttributes, NULL, Reserved);
		if (h==NULL){
			err=GetLastError();
			if (err!=ERROR_INSUFFICIENT_BUFFER){
				PyWin_SetAPIError("FindFirstUrlCacheEntryEx", err);
				break;
				}
			}
		else{
			ret=Py_BuildValue("NN", PyWinObject_FromUrlCacheHANDLE(h), PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(buf));
			break;
			}
		}  
  
	PyWinObject_FreeTCHAR(pattern);
	if (buf)
		free(buf);
	return ret;
};

// @pyswig dict|FindNextUrlCacheEntryEx|Continues enumeration of cached files
// @pyseeapi FindNextUrlCacheEntryEx
// @rdesc Returns a dict representing a INTERNET_CACHE_ENTRY_INFO strunct
// @comm Accepts keyword args
PyObject *PyFindNextUrlCacheEntryEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obh, *ret=NULL;
	static char *keywords[]={"EnumHandle", NULL};
	INTERNET_CACHE_ENTRY_INFO *buf=NULL;
	DWORD err, bufsize=512;
	HANDLE h;
	LPVOID GroupAttributes=NULL, Reserved=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:FindNextUrlCacheEntryEx", keywords,
		&obh))	// @pyparm <o PyUrlCacheHANDLE>|EnumHandle||Cache enumeration handle as returned by <om win32inet.FindFirstUrlCacheEntryEx>
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	while (1){
		if (buf)
			free(buf);
		buf=(INTERNET_CACHE_ENTRY_INFO *)malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			break;
			}
		if (FindNextUrlCacheEntryEx(h, buf, &bufsize, GroupAttributes, NULL, Reserved)){
			ret=PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(buf);
			break;
			}

		err=GetLastError();
		if (err!=ERROR_INSUFFICIENT_BUFFER){
			PyWin_SetAPIError("FindNextUrlCacheEntryEx", err);
			break;
			}
		}  
  
	if (buf)
		free(buf);
	return ret;
};

// @pyswig |FindCloseUrlCache|Closes a cache enumeration handle
// @pyseeapi FindCloseUrlCache
// @comm Accepts keyword args
PyObject *PyFindCloseUrlCache(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obh, *ret=NULL;
	HANDLE h;
	static char *keywords[]={"EnumHandle", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:FindCloseUrlCache", keywords,
		&obh))	// @pyparm <o PyUrlCacheHANDLE>|EnumHandle||Cache enumeration handle as returned by <om win32inet.FindFirstUrlCacheEntry>
		return NULL;
		
	if (PyHANDLE_Check(obh)){
		if (!((PyHANDLE *)obh)->Close())
			return NULL;
		Py_INCREF(Py_None);
		return Py_None;
		}
		
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!FindCloseUrlCache(h))
		return PyWin_SetAPIError("FindCloseUrlCache");
	Py_INCREF(Py_None);
	return Py_None;
};

// @pyswig (<o PyUrlCacheHANDLE>, int)|FindFirstUrlCacheGroup|Initiates enumeration of Url cache groups
// @pyseeapi FindFirstUrlCacheGroup
// @rdesc Returns a handle that can be passed to <om win32inet.FindNextUrlCacheGroup>, and the id
//	of the first group found.
// @comm Accepts keyword args
PyObject *PyFindFirstUrlCacheGroup(PyObject *self, PyObject *args, PyObject *kwargs)
{
	GROUPID groupid;
	HANDLE h;
	DWORD Filter=CACHEGROUP_SEARCH_ALL;
	// all other parameters are reserved
	LPVOID Reserved=NULL, SearchCondition=NULL;
	DWORD Flags=0, dwSearchCondition=0;
	static char *keywords[]={"Filter", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:FindFirstUrlCacheGroup", keywords,
		&Filter))	// @pyparm int|Filter|CACHEGROUP_SEARCH_ALL|CACHEGROUP_SEARCH_*
		return NULL;
	h = FindFirstUrlCacheGroup(Flags, Filter, SearchCondition, dwSearchCondition, &groupid, Reserved);
	if (h==NULL)
		return PyWin_SetAPIError("FindFirstUrlCacheGroup");
	return Py_BuildValue("NN", PyWinObject_FromUrlCacheHANDLE(h), PyLong_FromLongLong(groupid));
};

// @pyswig int|FindNextUrlCacheGroup|Continues enumeration of cache groups
// @pyseeapi FindNextUrlCacheGroup
// @comm Accepts keyword args
PyObject *PyFindNextUrlCacheGroup(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obh, *ret=NULL;
	static char *keywords[]={"Find", NULL};
	GROUPID groupid;
	LPVOID Reserved=NULL;
	HANDLE h;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:FindNextUrlCacheGroup", keywords,
		&obh))	// @pyparm <o PyHANDLE>|Find||Group enumeration handle as returned by <om win32inet.FindFirstUrlCacheGroup>
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!FindNextUrlCacheGroup(h, &groupid, Reserved))
		return PyWin_SetAPIError("FindNextUrlCacheGroup");
	return PyLong_FromLongLong(groupid);
};

PyObject *PyWinObject_FromDWORDArray(DWORD *dwords, DWORD nbr_dwords)
{
	PyObject *ret=PyTuple_New(nbr_dwords);
	if (ret==NULL)
		return NULL;
	for (DWORD dword_ind=0; dword_ind < nbr_dwords; dword_ind++){
		PyObject *item=PyLong_FromUnsignedLong(dwords[dword_ind]);
		if (item==NULL){
			Py_DECREF(ret);
			return NULL;
			}
		PyTuple_SET_ITEM(ret, dword_ind, item);
		}
	return ret;
}

// @pyswig dict|GetUrlCacheEntryInfo|Retrieves cache info for a URL
// @pyseeapi GetUrlCacheEntryInfo
// @rdesc Returns a dict representing a INTERNET_CACHE_ENTRY_INFO strunct
// @comm Accepts keyword args
PyObject *PyGetUrlCacheEntryInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *oburl, *ret=NULL;
	INTERNET_CACHE_ENTRY_INFO *buf=NULL;
	DWORD err, bufsize=512;
	TCHAR *url=NULL;
	
	static char *keywords[]={"UrlName", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetUrlCacheEntryInfo", keywords,
		&oburl))	// @pyparm str|UrlName||Cache enumeration handle as returned by <om win32inet.FindFirstUrlCacheEntry>
		return NULL;
	if (!PyWinObject_AsTCHAR(oburl, &url, FALSE))
		return NULL;
	while (1){
		if (buf)
			free(buf);
		buf=(INTERNET_CACHE_ENTRY_INFO *)malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			break;
			}
		if (GetUrlCacheEntryInfo(url, buf, &bufsize)){
			ret=PyWinObject_FromINTERNET_CACHE_ENTRY_INFO(buf);
			break;
			}

		err=GetLastError();
		if (err!=ERROR_INSUFFICIENT_BUFFER){
			PyWin_SetAPIError("GetUrlCacheEntryInfo", err);
			break;
			}
		}
	
	PyWinObject_FreeTCHAR(url);
	if (buf)
		free(buf);
	return ret;
};

// @pyswig |DeleteUrlCacheGroup|Deletes a cache group
// @pyseeapi DeleteUrlCacheGroup
// @comm Accepts keyword args
PyObject *PyDeleteUrlCacheGroup(PyObject *self, PyObject *args, PyObject *kwargs)
{
	GROUPID groupid;
	LPVOID Reserved=NULL;
	DWORD Flags=CACHEGROUP_FLAG_FLUSHURL_ONDELETE;

	static char *keywords[]={"GroupId","Flags", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "K|k:DeleteUrlCacheGroup", keywords,
			&groupid,		// @pyparm int|GroupId||Group id
			&Flags))	// @pyparm int|Attributes|CACHEGROUP_FLAG_FLUSHURL_ONDELETE|Combination of CACHEGROUP_FLAG_* flags
		return NULL;
	if (!DeleteUrlCacheGroup(groupid, Flags, Reserved))
		return PyWin_SetAPIError("DeleteUrlCacheGroup");
	Py_INCREF(Py_None);
	return Py_None;
};

// @pyswig long|CreateUrlCacheGroup|Creates a new cache group
// @pyseeapi CreateUrlCacheGroup
// @comm Accepts keyword args
PyObject *PyCreateUrlCacheGroup(PyObject *self, PyObject *args, PyObject *kwargs)
{
	GROUPID groupid;
	LPVOID Reserved=NULL;
	DWORD Flags=0;

	static char *keywords[]={"Flags", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CreateUrlCacheGroup", keywords,
			&Flags))	// @pyparm int|Flags|0|Combination of CACHEGROUP_FLAG_* flags
		return NULL;
	groupid = CreateUrlCacheGroup(Flags, Reserved);
	if (!groupid)
		return PyWin_SetAPIError("CreateUrlCacheGroup");
	return PyLong_FromLongLong(groupid);
};

// @pyswig str|CreateUrlCacheEntry|Creates a cache entry for a URL
// @rdesc Returns the filename to which content should be written
// @pyseeapi CreateUrlCacheEntry
// @comm Accepts keyword args
PyObject *PyCreateUrlCacheEntry(PyObject *self, PyObject *args, PyObject *kwargs)
{
	TCHAR *UrlName=NULL, *FileExtension=NULL;
	TCHAR filename[MAX_PATH+1];
	PyObject *obUrlName, *obFileExtension, *ret=NULL;
	DWORD ExpectedFileSize, Reserved=0;

	static char *keywords[]={"UrlName","ExpectedFileSize","FileExtension", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OkO:CreateUrlCacheEntry", keywords,
			&obUrlName,			// @pyparm str|UrlName||The Url for which to create an entry
			&ExpectedFileSize,	// @pyparm int|ExpectedFileSize||Size of content, use 0 if unknown
			&obFileExtension))	// @pyparm str|FileExtension||Extension to use for filename
		return NULL;
		
	if (PyWinObject_AsTCHAR(obUrlName, &UrlName, FALSE)
		&&PyWinObject_AsTCHAR(obFileExtension, &FileExtension, TRUE)){
		if (!CreateUrlCacheEntry(UrlName, ExpectedFileSize, FileExtension, filename, Reserved))
			PyWin_SetAPIError("CreateUrlCacheEntry");
		else
			ret = PyWinObject_FromTCHAR(filename);
		}
	PyWinObject_FreeTCHAR(UrlName);
	PyWinObject_FreeTCHAR(FileExtension);
	return ret;
};

// @pyswig str|CommitUrlCacheEntry|Commits a cache entry
// @pyseeapi CommitUrlCacheEntry
// @comm Accepts keyword args
PyObject *PyCommitUrlCacheEntry(PyObject *self, PyObject *args, PyObject *kwargs)
{
	TCHAR *UrlName=NULL, *LocalFileName=NULL, *OriginalUrl=NULL;
	// ??? Header info is defined as LPWSTR in UNICODE mode, but LPBYTE in ansi mode ???
#ifdef UNICODE
	WCHAR *HeaderInfo=NULL;
#else
	LPBYTE HeaderInfo=NULL;
#endif
	PyObject *obUrlName, *obLocalFileName, *obHeaderInfo=Py_None, *obOriginalUrl=Py_None;
	FILETIME ExpireTime={0,0}, LastModifiedTime={0,0};
	PyObject *obExpireTime=Py_None, *obLastModifiedTime=Py_None;
	DWORD CacheEntryType=NORMAL_CACHE_ENTRY, HeaderSize=0;
	TCHAR *FileExtension=NULL;	// reserved
	PyObject *ret=NULL;
	
	static char *keywords[]={"UrlName","LocalFileName","ExpireTime","LastModifiedTime",
		"CacheEntryType","HeaderInfo","OriginalUrl", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OOkOO:CommitUrlCacheEntry", keywords,
			&obUrlName,				// @pyparm str|UrlName||The Url for which to create an entry
			&obLocalFileName,		// @pyparm str|LocalFileName||Filename returned from <om win32inet.CreateUrlCacheEntry>.
									//	Can be None when creating a history entry.
			&obExpireTime,			// @pyparm <o PyTime>|ExpireTime|None|Time at which entry expires
			&obLastModifiedTime,	// @pyparm <o PyTime>|LastModifiedTime|None|Modification time of URL
			&CacheEntryType,		// @pyparm int|CacheEntryType|NORMAL_CACHE_ENTRY|Combination of *_CACHE_ENTRY flags
			&obHeaderInfo,			// @pyparm str|HeaderInfo|None|Header data used to request Url
			&obOriginalUrl))		// @pyparm str|OriginalUrl|None|If redirected, original site requested
		return NULL;
		
	if (PyWinObject_AsTCHAR(obUrlName, &UrlName, FALSE)
		&&PyWinObject_AsTCHAR(obLocalFileName, &LocalFileName, TRUE)
		&&(obExpireTime==Py_None || PyWinObject_AsFILETIME(obExpireTime, &ExpireTime))
		&&(obLastModifiedTime==Py_None || PyWinObject_AsFILETIME(obLastModifiedTime, &LastModifiedTime))
		&&PyWinObject_AsTCHAR(obHeaderInfo, (TCHAR **)&HeaderInfo, TRUE, &HeaderSize)
		&&PyWinObject_AsTCHAR(obOriginalUrl, &OriginalUrl, TRUE)){
		if (!CommitUrlCacheEntry(UrlName, LocalFileName,
				ExpireTime, LastModifiedTime, CacheEntryType,
				HeaderInfo, HeaderSize, FileExtension, OriginalUrl))
			PyWin_SetAPIError("CommitUrlCacheEntry");
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeTCHAR(UrlName);
	PyWinObject_FreeTCHAR(LocalFileName);
	PyWinObject_FreeTCHAR(OriginalUrl);
	PyWinObject_FreeTCHAR((TCHAR *)HeaderInfo);
	return ret;
};

// @pyswig |SetUrlCacheEntryGroup|Associates a cache entry with a group
// @pyseeapi SetUrlCacheEntryGroup
// @comm Accepts keyword args
PyObject *PySetUrlCacheEntryGroup(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obUrlName, *ret=NULL;
	TCHAR *UrlName=NULL;
	DWORD Flags;
	GROUPID GroupId;
	LPBYTE GroupAttributes=NULL;
	LPVOID Reserved=NULL;
	
	static char *keywords[]={"UrlName","Flags","GroupId", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OkK:SetUrlCacheEntryGroup", keywords,
		&obUrlName,		// @pyparm str|UrlName||Url whose cache is to be added to the group
		&Flags,			// @pyparm int|Flags||INTERNET_CACHE_GROUP_ADD or INTERNET_CACHE_GROUP_REMOVE
		&GroupId))		// @pyparm int|GroupId||Id of a cache group
		return NULL;
	if (!PyWinObject_AsTCHAR(obUrlName, &UrlName, FALSE))
		return NULL;
	if (!SetUrlCacheEntryGroup(UrlName, Flags, GroupId, GroupAttributes, NULL, Reserved))
		PyWin_SetAPIError("SetUrlCacheEntryGroup");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreeTCHAR(UrlName);
	return ret;
};

BOOL PyWinObject_AsINTERNET_CACHE_GROUP_INFO(PyObject *ob, INTERNET_CACHE_GROUP_INFO *GroupInfo)
{
	PyObject *obGroupName=Py_None, *obOwnerStorage=Py_None;
	ZeroMemory(GroupInfo, sizeof(INTERNET_CACHE_GROUP_INFO));
	PyObject *dummy_tuple=PyTuple_New(0);
	static char *keywords[]={"GroupSize","GroupFlags","GroupType","DiskUsage",
		"DiskQuota","OwnerStorage","GroupName", NULL};
	TCHAR *GroupName=NULL;
	DWORD namelen, dword_cnt;
	DWORD *OwnerStorage;
	
	BOOL bsuccess = PyArg_ParseTupleAndKeywords(dummy_tuple, ob, "|kkkkkOO:INTERNET_CACHE_GROUP_INFO", keywords,	
		&GroupInfo->dwGroupSize,
		&GroupInfo->dwGroupFlags,
		&GroupInfo->dwGroupType,
		&GroupInfo->dwDiskUsage,
		&GroupInfo->dwDiskQuota,
		&obOwnerStorage,
		&obGroupName)
		&&PyWinObject_AsDWORDArray(obOwnerStorage, &OwnerStorage, &dword_cnt, TRUE)
		&&PyWinObject_AsTCHAR(obGroupName, &GroupName, TRUE, &namelen);
	
	if (bsuccess && OwnerStorage){
		if (dword_cnt != GROUP_OWNER_STORAGE_SIZE){
			PyErr_Format(PyExc_ValueError, "OwnerStorage must contain %d ints", GROUP_OWNER_STORAGE_SIZE);
			bsuccess=FALSE;
			}
		else{
			for (DWORD dword_ind=0; dword_ind < GROUP_OWNER_STORAGE_SIZE; dword_ind++)
				GroupInfo->dwOwnerStorage[dword_ind] = OwnerStorage[dword_ind];
			}
		}
	if (bsuccess && GroupName)
		_tcsncpy(GroupInfo->szGroupName, GroupName, min(namelen, GROUPNAME_MAX_LENGTH));
	Py_DECREF(dummy_tuple);
	PyWinObject_FreeTCHAR(GroupName);
	if (OwnerStorage)
		free(OwnerStorage);
	return bsuccess;
}

// @pyswig dict|GetUrlCacheGroupAttribute|Retrieves attributes for a cache group
// @pyseeapi GetUrlCacheGroupAttribute
// @rdesc Returns a dict representing a INTERNET_CACHE_GROUP_INFO struct
// @comm Accepts keyword args
PyObject *PyGetUrlCacheGroupAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *ret=NULL;
	GROUPID groupid;
	DWORD bufsize, Attributes=CACHEGROUP_ATTRIBUTE_GET_ALL;
	LPVOID Reserved=NULL;
	DWORD Flags=0; // reserved
	INTERNET_CACHE_GROUP_INFO buf={0};

	static char *keywords[]={"GroupId","Attributes", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "K|k:GetUrlCacheGroupAttribute", keywords,
			&groupid,		// @pyparm int|GroupId||Group id
			&Attributes))	// @pyparm int|Attributes|CACHEGROUP_ATTRIBUTE_GET_ALL|Attributes to retrieve, CACHEGROUP_ATTRIBUTE_*
		return NULL;
	bufsize=sizeof(buf);
	buf.dwGroupSize=sizeof(buf);
	if (!GetUrlCacheGroupAttribute(groupid, Flags, Attributes, &buf, &bufsize, Reserved))
		return PyWin_SetAPIError("GetUrlCacheGroupAttribute");
	return Py_BuildValue("{s:k, s:k, s:k, s:k, s:k, s:N, s:N}",
		"GroupSize", buf.dwGroupSize,
		"GroupFlags", buf.dwGroupFlags,
		"GroupType", buf.dwGroupType,
		"DiskUsage", buf.dwDiskUsage,
		"DiskQuota", buf.dwDiskQuota,
		"OwnerStorage", PyWinObject_FromDWORDArray(buf.dwOwnerStorage, GROUP_OWNER_STORAGE_SIZE),
		"GroupName", PyWinObject_FromTCHAR(buf.szGroupName));
};

// @pyswig |SetUrlCacheGroupAttribute|Changes the attributes of a cache group
// @pyseeapi SetUrlCacheGroupAttribute
// @comm Accepts keyword args
PyObject *PySetUrlCacheGroupAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obGroupInfo, *ret=NULL;
	DWORD Flags=0, Attributes;
	GROUPID GroupId;
	LPVOID Reserved=NULL;
	INTERNET_CACHE_GROUP_INFO GroupInfo={0};

	static char *keywords[]={"GroupId","Attributes","GroupInfo","Flags", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "KkO|k:SetUrlCacheGroupAttribute", keywords,
		&GroupId,		// @pyparm int|GroupId||Id of a cache group
		&Attributes,	// @pyparm int|Attributes||Bitmask of CACHEGROUP_ATTRIBUTE_* flags indicating which attributes to set
		&obGroupInfo,	// @pyparm dict|GroupInfo||INTERNET_CACHE_GROUP_INFO dict as returned by <om win32inet.GetUrlCacheGroupAttribute>
		&Flags))		// @pyparm int|Flags|0|Reserved, use 0
		return NULL;
	if (!PyWinObject_AsINTERNET_CACHE_GROUP_INFO(obGroupInfo, &GroupInfo))
		return NULL;
	if (!SetUrlCacheGroupAttribute(GroupId, Flags, Attributes, &GroupInfo, Reserved))
		PyWin_SetAPIError("SetUrlCacheGroupAttribute");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	return ret;
};

PyCFunction pfnPyFindFirstUrlCacheEntry = (PyCFunction)PyFindFirstUrlCacheEntry;
PyCFunction pfnPyFindNextUrlCacheEntry = (PyCFunction)PyFindNextUrlCacheEntry;
PyCFunction pfnPyFindFirstUrlCacheEntryEx = (PyCFunction)PyFindFirstUrlCacheEntryEx;
PyCFunction pfnPyFindNextUrlCacheEntryEx = (PyCFunction)PyFindNextUrlCacheEntryEx;
PyCFunction pfnPyFindCloseUrlCache = (PyCFunction)PyFindCloseUrlCache;
PyCFunction pfnPyFindFirstUrlCacheGroup = (PyCFunction)PyFindFirstUrlCacheGroup;
PyCFunction pfnPyFindNextUrlCacheGroup = (PyCFunction)PyFindNextUrlCacheGroup;
PyCFunction pfnPyGetUrlCacheEntryInfo = (PyCFunction)PyGetUrlCacheEntryInfo;
PyCFunction pfnPyDeleteUrlCacheGroup = (PyCFunction)PyDeleteUrlCacheGroup;
PyCFunction pfnPyCreateUrlCacheGroup = (PyCFunction)PyCreateUrlCacheGroup;
PyCFunction pfnPyCreateUrlCacheEntry = (PyCFunction)PyCreateUrlCacheEntry;
PyCFunction pfnPyCommitUrlCacheEntry = (PyCFunction)PyCommitUrlCacheEntry;
PyCFunction pfnPySetUrlCacheEntryGroup = (PyCFunction)PySetUrlCacheEntryGroup;
PyCFunction pfnPyGetUrlCacheGroupAttribute = (PyCFunction)PyGetUrlCacheGroupAttribute;
PyCFunction pfnPySetUrlCacheGroupAttribute = (PyCFunction)PySetUrlCacheGroupAttribute;
%}

%native (FindFirstUrlCacheEntry) pfnPyFindFirstUrlCacheEntry;
%native (FindNextUrlCacheEntry) pfnPyFindNextUrlCacheEntry;
%native (FindFirstUrlCacheEntryEx) pfnPyFindFirstUrlCacheEntryEx;
%native (FindNextUrlCacheEntryEx) pfnPyFindNextUrlCacheEntryEx;
%native (FindCloseUrlCache) pfnPyFindCloseUrlCache;
%native (FindFirstUrlCacheGroup) pfnPyFindFirstUrlCacheGroup;
%native (FindNextUrlCacheGroup) pfnPyFindNextUrlCacheGroup;
%native (GetUrlCacheEntryInfo) pfnPyGetUrlCacheEntryInfo;
%native (DeleteUrlCacheGroup) pfnPyDeleteUrlCacheGroup;
%native (CreateUrlCacheGroup) pfnPyCreateUrlCacheGroup;
%native (CreateUrlCacheEntry) pfnPyCreateUrlCacheEntry;
%native (CommitUrlCacheEntry) pfnPyCommitUrlCacheEntry;
%native (SetUrlCacheEntryGroup) pfnPySetUrlCacheEntryGroup;
%native (GetUrlCacheGroupAttribute) pfnPyGetUrlCacheGroupAttribute;
%native (SetUrlCacheGroupAttribute) pfnPySetUrlCacheGroupAttribute;

// @pyswig |DeleteUrlCacheEntry|Deletes the cache entry for a URL
// @pyparm str|UrlName||Cached url to be deleted
BOOLAPI DeleteUrlCacheEntry(TCHAR *lpszUrlName);

%{
extern void init_win32inetstuff();
extern PyObject *PyWinHttpGetIEProxyConfigForCurrentUser(PyObject *, PyObject *);
extern PyObject *PyWinHttpGetDefaultProxyConfiguration(PyObject *, PyObject *);
extern PyObject *PyWinHttpGetProxyForUrl(PyObject *, PyObject *);
extern PyObject *PyWinHttpOpen(PyObject *, PyObject *);
%}

%native(WinHttpGetIEProxyConfigForCurrentUser) PyWinHttpGetIEProxyConfigForCurrentUser;
%native(WinHttpGetDefaultProxyConfiguration) PyWinHttpGetDefaultProxyConfiguration;
%native(WinHttpGetProxyForUrl) PyWinHttpGetProxyForUrl;
%native(WinHttpOpen) PyWinHttpOpen;

%init %{
	PyDict_SetItemString(d,	"error", PyWinExc_ApiError);
	HMODULE	hmod = GetModuleHandle(TEXT("wininet.dll"));
	assert(hmod);
	PyWin_RegisterErrorMessageModule(INTERNET_ERROR_BASE,
									 INTERNET_ERROR_LAST,
									 hmod);
	for	(PyMethodDef *pmd =	win32inetMethods;pmd->ml_name;pmd++)
		if	 ((strcmp(pmd->ml_name,	"InternetOpenUrl")==0)
			||(strcmp(pmd->ml_name,	"FtpOpenFile")==0)
			||(strcmp(pmd->ml_name,	"FtpCommand")==0)
			||(strcmp(pmd->ml_name,	"InternetConnect")==0)
			||(strcmp(pmd->ml_name, "FindFirstUrlCacheEntry")==0)
			||(strcmp(pmd->ml_name, "FindNextUrlCacheEntry")==0)
			||(strcmp(pmd->ml_name, "FindFirstUrlCacheEntryEx")==0)
			||(strcmp(pmd->ml_name, "FindNextUrlCacheEntryEx")==0)
			||(strcmp(pmd->ml_name, "FindCloseUrlCache")==0)
			||(strcmp(pmd->ml_name, "FindFirstUrlCacheGroup")==0)
			||(strcmp(pmd->ml_name, "FindNextUrlCacheGroup")==0)
			||(strcmp(pmd->ml_name, "GetUrlCacheGroupAttribute")==0)
			||(strcmp(pmd->ml_name, "GetUrlCacheEntryInfo")==0)
			||(strcmp(pmd->ml_name, "DeleteUrlCacheGroup")==0)
			||(strcmp(pmd->ml_name, "CreateUrlCacheGroup")==0)
			||(strcmp(pmd->ml_name, "CreateUrlCacheEntry")==0)
			||(strcmp(pmd->ml_name, "CommitUrlCacheEntry")==0)
			||(strcmp(pmd->ml_name, "SetUrlCacheEntryGroup")==0)
			||(strcmp(pmd->ml_name, "SetUrlCacheGroupAttribute")==0)
			){
			pmd->ml_flags =	METH_VARARGS | METH_KEYWORDS;
			}
	init_win32inetstuff();
%}
