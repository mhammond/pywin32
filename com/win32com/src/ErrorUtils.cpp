// oleerr.cpp : Defines error codes
//
#include "stdafx.h"
#include "PythonCOM.h"
#include "oaidl.h"

#ifdef MS_WINCE
#include "olectl.h" // For connection point constants.
#endif

static const char *szBadStringObject = "<Bad String Object>";
extern PyObject *PyCom_InternalError;

void GetScodeString(SCODE sc, TCHAR *buf, int bufSize);
LPCSTR GetScodeRangeString(SCODE sc);
LPCSTR GetSeverityString(SCODE sc);
LPCSTR GetFacilityString(SCODE sc);

#if defined(MS_WINCE) && !defined(_ASSERTE) // No _ASSERTE on CE - who cares!
	#define _ASSERTE(condition)
#endif

// This module uses an ATL utility "A2BSTR".
// If this is not available, we provide one of our own
// suitable for Unicode only.
#ifndef ATLA2WHELPER
#	ifndef UNICODE
#		error("A2BSTR is only emulated for UNICODE builds")
#	endif
BSTR A2BSTR(const char *buf)
{
	int size=strlen(buf);
	int wideSize = size*2;
	LPWSTR wstr = (LPWSTR)malloc(wideSize);
	if (wstr==NULL) return NULL;
	/* convert and get the final character size */
	size = MultiByteToWideChar(CP_ACP, 0, buf, size, wstr, wideSize);
	return SysAllocStringLen(wstr, size);
}
#	define USES_CONVERSION
#endif

static const EXCEPINFO nullExcepInfo = { 0 };
static PyObject *PyCom_PyObjectFromIErrorInfo(IErrorInfo *, HRESULT errorhr);

////////////////////////////////////////////////////////////////////////
//
// Server Side Errors - translate a Python exception to COM error information
//
////////////////////////////////////////////////////////////////////////

// Generically fills an EXCEP_INFO.  The scode in the EXCEPINFO
// is the HRESULT as nominated by the user.
void PyCom_ExcepInfoFromPyException(EXCEPINFO *pExcepInfo)
{
	USES_CONVERSION;
	// If the caller did not provide a valid exception info, get out now!
	if (pExcepInfo==NULL) {
		PyErr_Clear(); // must leave Python in a clean state.
		return;
	}
	PyObject *exception, *v, *tb;
	*pExcepInfo = nullExcepInfo;
	PyErr_Fetch(&exception, &v, &tb);
	if (PyCom_ExcepInfoFromPyObject(v, pExcepInfo, NULL))
	{
		// done.
	}
	else
	{
		// Not a special exception object - do the best we can.
		PyObject *obException = PyObject_Str(exception);
		PyObject *obValue = PyObject_Str(v);
		char *szException = PyString_AsString(obException);
		char *szValue = PyString_AsString(obValue);
		char *szBaseMessage = "Unexpected Python Error: ";
		if (szException==NULL) szException = "<bad exception>";
		if (szValue==NULL) szValue = "<bad exception value>";
		int len = strlen(szBaseMessage) + strlen(szException) + 2 + strlen(szValue) + 1;
			// 2 for ": "
		// message could be quite long - be safe.
		char *tempBuf = new char[len];
		strcpy(tempBuf, szBaseMessage);
		strcat(tempBuf, szException);
		strcat(tempBuf, ": ");
		strcat(tempBuf, szValue);
		pExcepInfo->bstrDescription = A2BSTR(tempBuf);
		pExcepInfo->bstrSource = A2BSTR("Python COM Server Internal Error");

		// Map some well known exceptions to specific HRESULTs
		// Note: v can be NULL. This can happen via PyErr_SetNone().
		//       e.g.: KeyboardInterrupt
		if (PyErr_GivenExceptionMatches(exception, PyExc_MemoryError))
			pExcepInfo->scode = E_OUTOFMEMORY;
		else
			// Any other common Python exceptions we should map?
			pExcepInfo->scode = E_FAIL;

		delete [] tempBuf;
		Py_XDECREF(obException);
		Py_XDECREF(obValue);
	}
	Py_XDECREF(tb);
	Py_XDECREF(exception);
	Py_XDECREF(v);
	PyErr_Clear();
}

static BOOL PyCom_ExcepInfoFromServerExceptionInstance(PyObject *v, EXCEPINFO *pExcepInfo)
{
	USES_CONVERSION;
	BSTR temp;

	_ASSERTE(v != NULL);
	_ASSERTE(pExcepInfo != NULL);

	PyObject *ob = PyObject_GetAttrString(v, "description");
	if (ob && ob != Py_None) {
		if ( !PyWinObject_AsBstr(ob, &temp) )
			pExcepInfo->bstrDescription = A2BSTR(szBadStringObject);
		else
			pExcepInfo->bstrDescription = temp;
	} else {
		// No description - leave it empty.
		PyErr_Clear();
	}
	Py_XDECREF(ob);

	ob = PyObject_GetAttrString(v, "source");
	if (ob && ob != Py_None) {
		if ( !PyWinObject_AsBstr(ob, &temp) )
			pExcepInfo->bstrSource = A2BSTR(szBadStringObject);
		else
			pExcepInfo->bstrSource = temp;
	}
	else
		PyErr_Clear();
	Py_XDECREF(ob);

	ob = PyObject_GetAttrString(v, "helpfile");
	if (ob && ob != Py_None) {
		if ( !PyWinObject_AsBstr(ob, &temp) )
			pExcepInfo->bstrHelpFile = A2BSTR(szBadStringObject);
		else
			pExcepInfo->bstrHelpFile = temp;
	}
	else
		PyErr_Clear();
	Py_XDECREF(ob);

	ob = PyObject_GetAttrString(v, "code");
	if (ob && ob != Py_None) {
		pExcepInfo->wCode = (unsigned short)PyInt_AsLong(PyNumber_Int(ob));
	}
	else
		PyErr_Clear();
	Py_XDECREF(ob);

	ob = PyObject_GetAttrString(v, "scode");
	if (ob && ob != Py_None) {
		pExcepInfo->scode = PyInt_AsLong(PyNumber_Int(ob));
	}
	else
		PyErr_Clear();
	Py_XDECREF(ob);

	ob = PyObject_GetAttrString(v, "helpcontext");
	if (ob && ob != Py_None) {
		pExcepInfo->dwHelpContext = PyInt_AsLong(PyNumber_Int(ob));
	}
	else
		PyErr_Clear();
	Py_XDECREF(ob);
	return TRUE;
}

// Fill an exception info from a specific COM error raised by the
// Python code.  If the Python exception is not a specific COM error
// (ie, pythoncom.com_error, or a COM server exception instance)
// then return FALSE.
BOOL PyCom_ExcepInfoFromPyObject(PyObject *v, EXCEPINFO *pExcepInfo, HRESULT *phresult)
{
	_ASSERTE(pExcepInfo != NULL);
	if (v==NULL || pExcepInfo==NULL)
		return FALSE;

	// New handling for 1.5 exceptions.
	if (!PyErr_GivenExceptionMatches(v, PyWinExc_COMError))
		return FALSE;
	// It is a COM exception, but may be a server or client instance.
	// Explicit check for client.
	// Note that with class based exceptions, a simple pointer check fails.
	// Any class sub-classed from the client is considered a server error,
	// so we need to check the class explicitely.
	if (v==PyWinExc_COMError || // String exceptions
	      (PyInstance_Check(v) && // Class exceptions
		  (PyObject *)(((PyInstanceObject *)v)->in_class)==PyWinExc_COMError) )  {
		// Client side error - use abstract API to get at details.
		PyObject *ob;
		if (phresult) {
			ob = PySequence_GetItem(v, 0);
			if (ob) {
				*phresult = PyInt_AsLong(ob);
				Py_DECREF(ob);
			}
		}
		// item[1] is the scode description, which we dont need.
		ob = PySequence_GetItem(v, 2);
		if (ob) {
			USES_CONVERSION;
			int code, helpContext, scode;
			const char *source, *description, *helpFile;
			if ( !PyArg_ParseTuple(ob, "izzzii:ExceptionInfo",
								   &code,
								   &source,
								   &description,
								   &helpFile,
								   &helpContext,
								   &scode) ) {
				Py_DECREF(ob);
				return FALSE;
			}
			pExcepInfo->wCode = code;
			pExcepInfo->wReserved = 0;
			pExcepInfo->bstrSource = A2BSTR(source);
			pExcepInfo->bstrDescription = A2BSTR(description);
			pExcepInfo->bstrHelpFile = A2BSTR(helpFile);
			pExcepInfo->dwHelpContext = helpContext;
			pExcepInfo->pvReserved = 0;
			pExcepInfo->pfnDeferredFillIn = NULL;
			pExcepInfo->scode = scode;
			Py_DECREF(ob);
		}
		return TRUE;
	} else {
		// Server side error
		BOOL ok = PyCom_ExcepInfoFromServerExceptionInstance(v, pExcepInfo);
		if (ok && phresult)
			*phresult = pExcepInfo->scode;
		return ok;
	}
}

// Given an EXCEPINFO, register the error information with the
// IErrorInfo interface.
BOOL PyCom_SetCOMErrorFromExcepInfo(const EXCEPINFO *pexcepinfo, REFIID riid)
{
	ICreateErrorInfo *pICEI;
	HRESULT hr = CreateErrorInfo(&pICEI);
	if ( SUCCEEDED(hr) )
	{
		pICEI->SetGUID(riid);
		pICEI->SetHelpContext(pexcepinfo->dwHelpContext);
		if ( pexcepinfo->bstrDescription )
			pICEI->SetDescription(pexcepinfo->bstrDescription);
		if ( pexcepinfo->bstrHelpFile )
			pICEI->SetHelpFile(pexcepinfo->bstrHelpFile);
		if ( pexcepinfo->bstrSource )
			pICEI->SetSource(pexcepinfo->bstrSource);

		IErrorInfo *pIEI;
		Py_BEGIN_ALLOW_THREADS
		hr = pICEI->QueryInterface(IID_IErrorInfo, (LPVOID*) &pIEI);
		Py_END_ALLOW_THREADS
		if ( SUCCEEDED(hr) )
		{
			SetErrorInfo(0, pIEI);
			pIEI->Release();
		}
		pICEI->Release();			
	}
	return SUCCEEDED(hr);
}

void PyCom_CleanupExcepInfo(EXCEPINFO *pexcepinfo)
{
	if ( pexcepinfo->bstrDescription ) {
		SysFreeString(pexcepinfo->bstrDescription);
		pexcepinfo->bstrDescription = NULL;
	}
	if ( pexcepinfo->bstrHelpFile ) {
		SysFreeString(pexcepinfo->bstrHelpFile);
		pexcepinfo->bstrHelpFile = NULL;
	}
	if ( pexcepinfo->bstrSource ) {
		SysFreeString(pexcepinfo->bstrSource);
		pexcepinfo->bstrSource = NULL;
	}
}

HRESULT PyCom_SetCOMErrorFromSimple(HRESULT hr, REFIID riid /* = IID_NULL */, const char *description /* = NULL*/)
{
	// fast path...
	if ( hr == S_OK )
		return S_OK;

	// If you specify a description you should also specify the IID
	_ASSERTE(riid != IID_NULL || description==NULL);
	// Reset the error info for this thread.  "Inside OLE2" says we
	// can call IErrorInfo with NULL, but the COM documentation doesnt mention it.
	BSTR bstrDesc = NULL;
	if (description) bstrDesc = A2BSTR(description);

	EXCEPINFO einfo = {
		0,		// wCode
		0,		// wReserved
		NULL,	// bstrSource
		bstrDesc,	// bstrDescription
		NULL,	// bstrHelpFile
		0,		// dwHelpContext
		NULL,	// pvReserved
		NULL,	// pfnDeferredFillIn
		hr		// scode
	};
	HRESULT ret = PyCom_SetCOMErrorFromExcepInfo(&einfo, riid);
	PyCom_CleanupExcepInfo(&einfo);
	return ret;
}

PYCOM_EXPORT HRESULT PyCom_SetCOMErrorFromPyException(REFIID riid /* = IID_NULL */)
{
	if (!PyErr_Occurred())
		// No error occurred
		return S_OK;

	EXCEPINFO einfo;
	PyCom_ExcepInfoFromPyException(&einfo);

	// force this to a failure just in case we couldn't extract a proper
	// error value
	if ( einfo.scode == S_OK )
		einfo.scode = E_FAIL;

	PyCom_SetCOMErrorFromExcepInfo(&einfo, riid);
	PyCom_CleanupExcepInfo(&einfo);
	return einfo.scode;
}

////////////////////////////////////////////////////////////////////////
//
// Client Side Errors - translate a COM failure to a Python exception
//
////////////////////////////////////////////////////////////////////////
PyObject *PyCom_BuildPyException(HRESULT errorhr, IUnknown *pUnk /* = NULL */, REFIID iid /* = IID_NULL */)
{
	PyObject *obEI = NULL;
	char scodeStringBuf[512];
	GetScodeString(errorhr, scodeStringBuf, sizeof(scodeStringBuf));

#ifndef MS_WINCE // WINCE doesnt appear to have GetErrorInfo() - compiled, but doesnt link!
	if (pUnk != NULL) {
		_ASSERTE(iid != IID_NULL); // If you pass an IUnknown, you should pass the specific IID.
		// See if it supports error info.
		ISupportErrorInfo *pSEI;
		HRESULT hr;
		Py_BEGIN_ALLOW_THREADS
		hr = pUnk->QueryInterface(IID_ISupportErrorInfo, (void **)&pSEI);
		if (SUCCEEDED(hr)) {
			hr = pSEI->InterfaceSupportsErrorInfo(iid);
			pSEI->Release(); // Finished with this object
		}
		Py_END_ALLOW_THREADS
		if (SUCCEEDED(hr)) {
			IErrorInfo *pEI;
			Py_BEGIN_ALLOW_THREADS
			hr=GetErrorInfo(0, &pEI);
			Py_END_ALLOW_THREADS
			if (hr==S_OK) {
				obEI = PyCom_PyObjectFromIErrorInfo(pEI, errorhr);
				Py_BEGIN_ALLOW_THREADS
				pEI->Release();
				Py_END_ALLOW_THREADS
			}
		}
	}
#endif // MS_WINCE
	if (obEI==NULL)	{
		obEI = Py_None;
		Py_INCREF(Py_None);
	}
	PyObject *evalue = Py_BuildValue("isOO", errorhr, scodeStringBuf, obEI, Py_None);
	Py_DECREF(obEI);

	PyErr_SetObject(PyWinExc_COMError, evalue);
	Py_XDECREF(evalue);
	return NULL;
}

// Uses the HRESULT and an EXCEPINFO structure to create and
// set a pythoncom.com_error.
// Used rarely - currently by IDispatch and IActiveScriptParse* interfaces.
PyObject* PyCom_BuildPyExceptionFromEXCEPINFO(HRESULT hr, EXCEPINFO *pexcepInfo /* = NULL */, UINT nArgErr /* = -1 */)
{
	TCHAR buf[512];
	GetScodeString(hr, buf, sizeof(buf)/sizeof(TCHAR));
	PyObject *obScodeString = PyString_FromTCHAR(buf);
	PyObject *evalue;
	PyObject *obArg;

	if ( nArgErr != -1 ) {
		obArg = PyInt_FromLong(nArgErr);
	} else {
		obArg = Py_None;
		Py_INCREF(obArg);
	}
	if (pexcepInfo==NULL)
	{
		evalue = Py_BuildValue("iOzO", hr, obScodeString, NULL, obArg);
	}
	else
	{
		PyObject *obExcepInfo = PyCom_PyObjectFromExcepInfo(pexcepInfo);
		if ( obExcepInfo )
		{
			evalue = Py_BuildValue("iOOO", hr, obScodeString, obExcepInfo, obArg);
			Py_DECREF(obExcepInfo);
		}
		else
			evalue = NULL;

		/* done with the exception, free it */
		PyCom_CleanupExcepInfo(pexcepInfo);
	}
	Py_DECREF(obArg);
	PyErr_SetObject(PyWinExc_COMError, evalue);
	Py_XDECREF(evalue);
	Py_XDECREF(obScodeString);
	return NULL;
}

PyObject* PyCom_BuildInternalPyException(char *msg)
{
	PyErr_SetString(PyCom_InternalError, msg);
	return NULL;
}

PyObject *PyCom_PyObjectFromExcepInfo(const EXCEPINFO *pexcepInfo)
{
	EXCEPINFO filledIn;

	// Do a deferred fill-in if necessary
	if ( pexcepInfo->pfnDeferredFillIn )
	{
		filledIn = *pexcepInfo;
		(*pexcepInfo->pfnDeferredFillIn)(&filledIn);
		pexcepInfo = &filledIn;
	}

	 // ### should these by PyUnicode values?  Still strings for compatibility.
	PyObject *obSource = PyString_FromUnicode(pexcepInfo->bstrSource);
	PyObject *obDescription = PyString_FromUnicode(pexcepInfo->bstrDescription);
	PyObject *obHelpFile = PyString_FromUnicode(pexcepInfo->bstrHelpFile);
	PyObject *rc = Py_BuildValue("iOOOii",
						 (int)pexcepInfo->wCode,
						 obSource,
						 obDescription,
						 obHelpFile,
						 (int)pexcepInfo->dwHelpContext,
						 (int)pexcepInfo->scode);
	Py_XDECREF(obSource);
	Py_XDECREF(obDescription);
	Py_XDECREF(obHelpFile);
	return rc;
}

// NOTE - This MUST return the same object format as the above function
static PyObject *PyCom_PyObjectFromIErrorInfo(IErrorInfo *pEI, HRESULT errorhr)
{
	USES_CONVERSION;
	BSTR desc;
	BSTR source;
	BSTR helpfile;
	PyObject *obDesc;
	PyObject *obSource;
	PyObject *obHelpFile;

	HRESULT hr;

	Py_BEGIN_ALLOW_THREADS
	hr=pEI->GetDescription(&desc);
	Py_END_ALLOW_THREADS
	if (hr!=S_OK) {
		obDesc = Py_None;
		Py_INCREF(obDesc);
	} else
		obDesc = MakeBstrToObj(desc);
	Py_BEGIN_ALLOW_THREADS
	hr=pEI->GetSource(&source);
	Py_END_ALLOW_THREADS
	if (hr!=S_OK) {
		obSource = Py_None;
		Py_INCREF(obSource);
	} else
		obSource = MakeBstrToObj(source);
	Py_BEGIN_ALLOW_THREADS
	hr=pEI->GetHelpFile(&helpfile);
	Py_END_ALLOW_THREADS
	if (hr!=S_OK) {
		obHelpFile = Py_None;
		Py_INCREF(obHelpFile);
	} else
		obHelpFile = MakeBstrToObj(helpfile);
	DWORD helpContext = 0;
	pEI->GetHelpContext(&helpContext);
	PyObject *ret = Py_BuildValue("iOOOii",
						 0, // wCode remains zero, as scode holds our data.
						 // ### should these by PyUnicode values?
						 obSource,
						 obDesc,
						 obHelpFile,
						 (int)helpContext,
						 errorhr);
	Py_XDECREF(obSource);
	Py_XDECREF(obDesc);
	Py_XDECREF(obHelpFile);
	return ret;
}


////////////////////////////////////////////////////////////////////////
//
// Error string helpers - get SCODE, FACILITY etc strings
//
////////////////////////////////////////////////////////////////////////
#define _countof(array) (sizeof(array)/sizeof(array[0]))

void GetScodeString(HRESULT hr, LPTSTR buf, int bufSize)
{
	struct HRESULT_ENTRY
	{
		HRESULT hr;
		LPCTSTR lpszName;
	};
	#define MAKE_HRESULT_ENTRY(hr)    { hr, _T(#hr) }
	static const HRESULT_ENTRY hrNameTable[] =
	{
		MAKE_HRESULT_ENTRY(S_OK),
		MAKE_HRESULT_ENTRY(S_FALSE),

		MAKE_HRESULT_ENTRY(CACHE_S_FORMATETC_NOTSUPPORTED),
		MAKE_HRESULT_ENTRY(CACHE_S_SAMECACHE),
		MAKE_HRESULT_ENTRY(CACHE_S_SOMECACHES_NOTUPDATED),
		MAKE_HRESULT_ENTRY(CONVERT10_S_NO_PRESENTATION),
		MAKE_HRESULT_ENTRY(DATA_S_SAMEFORMATETC),
		MAKE_HRESULT_ENTRY(DRAGDROP_S_CANCEL),
		MAKE_HRESULT_ENTRY(DRAGDROP_S_DROP),
		MAKE_HRESULT_ENTRY(DRAGDROP_S_USEDEFAULTCURSORS),
		MAKE_HRESULT_ENTRY(INPLACE_S_TRUNCATED),
		MAKE_HRESULT_ENTRY(MK_S_HIM),
		MAKE_HRESULT_ENTRY(MK_S_ME),
		MAKE_HRESULT_ENTRY(MK_S_MONIKERALREADYREGISTERED),
		MAKE_HRESULT_ENTRY(MK_S_REDUCED_TO_SELF),
		MAKE_HRESULT_ENTRY(MK_S_US),
		MAKE_HRESULT_ENTRY(OLE_S_MAC_CLIPFORMAT),
		MAKE_HRESULT_ENTRY(OLE_S_STATIC),
		MAKE_HRESULT_ENTRY(OLE_S_USEREG),
		MAKE_HRESULT_ENTRY(OLEOBJ_S_CANNOT_DOVERB_NOW),
		MAKE_HRESULT_ENTRY(OLEOBJ_S_INVALIDHWND),
		MAKE_HRESULT_ENTRY(OLEOBJ_S_INVALIDVERB),
		MAKE_HRESULT_ENTRY(OLEOBJ_S_LAST),
		MAKE_HRESULT_ENTRY(STG_S_CONVERTED),
		MAKE_HRESULT_ENTRY(VIEW_S_ALREADY_FROZEN),

		MAKE_HRESULT_ENTRY(E_UNEXPECTED),
		MAKE_HRESULT_ENTRY(E_NOTIMPL),
		MAKE_HRESULT_ENTRY(E_OUTOFMEMORY),
		MAKE_HRESULT_ENTRY(E_INVALIDARG),
		MAKE_HRESULT_ENTRY(E_NOINTERFACE),
		MAKE_HRESULT_ENTRY(E_POINTER),
		MAKE_HRESULT_ENTRY(E_HANDLE),
		MAKE_HRESULT_ENTRY(E_ABORT),
		MAKE_HRESULT_ENTRY(E_FAIL),
		MAKE_HRESULT_ENTRY(E_ACCESSDENIED),

		MAKE_HRESULT_ENTRY(CACHE_E_NOCACHE_UPDATED),
		MAKE_HRESULT_ENTRY(CLASS_E_CLASSNOTAVAILABLE),
		MAKE_HRESULT_ENTRY(CLASS_E_NOAGGREGATION),
		MAKE_HRESULT_ENTRY(CLIPBRD_E_BAD_DATA),
		MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_CLOSE),
		MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_EMPTY),
		MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_OPEN),
		MAKE_HRESULT_ENTRY(CLIPBRD_E_CANT_SET),
		MAKE_HRESULT_ENTRY(CO_E_ALREADYINITIALIZED),
		MAKE_HRESULT_ENTRY(CO_E_APPDIDNTREG),
		MAKE_HRESULT_ENTRY(CO_E_APPNOTFOUND),
		MAKE_HRESULT_ENTRY(CO_E_APPSINGLEUSE),
		MAKE_HRESULT_ENTRY(CO_E_BAD_PATH),
		MAKE_HRESULT_ENTRY(CO_E_CANTDETERMINECLASS),
		MAKE_HRESULT_ENTRY(CO_E_CLASS_CREATE_FAILED),
		MAKE_HRESULT_ENTRY(CO_E_CLASSSTRING),
		MAKE_HRESULT_ENTRY(CO_E_DLLNOTFOUND),
		MAKE_HRESULT_ENTRY(CO_E_ERRORINAPP),
		MAKE_HRESULT_ENTRY(CO_E_ERRORINDLL),
		MAKE_HRESULT_ENTRY(CO_E_IIDSTRING),
		MAKE_HRESULT_ENTRY(CO_E_NOTINITIALIZED),
		MAKE_HRESULT_ENTRY(CO_E_OBJISREG),
		MAKE_HRESULT_ENTRY(CO_E_OBJNOTCONNECTED),
		MAKE_HRESULT_ENTRY(CO_E_OBJNOTREG),
		MAKE_HRESULT_ENTRY(CO_E_OBJSRV_RPC_FAILURE),
		MAKE_HRESULT_ENTRY(CO_E_SCM_ERROR),
		MAKE_HRESULT_ENTRY(CO_E_SCM_RPC_FAILURE),
		MAKE_HRESULT_ENTRY(CO_E_SERVER_EXEC_FAILURE),
		MAKE_HRESULT_ENTRY(CO_E_SERVER_STOPPING),
		MAKE_HRESULT_ENTRY(CO_E_WRONGOSFORAPP),
		MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB),
		MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_FMT),
		MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_GET),
		MAKE_HRESULT_ENTRY(CONVERT10_E_OLESTREAM_PUT),
		MAKE_HRESULT_ENTRY(CONVERT10_E_STG_DIB_TO_BITMAP),
		MAKE_HRESULT_ENTRY(CONVERT10_E_STG_FMT),
		MAKE_HRESULT_ENTRY(CONVERT10_E_STG_NO_STD_STREAM),
		MAKE_HRESULT_ENTRY(DISP_E_ARRAYISLOCKED),
		MAKE_HRESULT_ENTRY(DISP_E_BADCALLEE),
		MAKE_HRESULT_ENTRY(DISP_E_BADINDEX),
		MAKE_HRESULT_ENTRY(DISP_E_BADPARAMCOUNT),
		MAKE_HRESULT_ENTRY(DISP_E_BADVARTYPE),
		MAKE_HRESULT_ENTRY(DISP_E_EXCEPTION),
		MAKE_HRESULT_ENTRY(DISP_E_MEMBERNOTFOUND),
		MAKE_HRESULT_ENTRY(DISP_E_NONAMEDARGS),
		MAKE_HRESULT_ENTRY(DISP_E_NOTACOLLECTION),
		MAKE_HRESULT_ENTRY(DISP_E_OVERFLOW),
		MAKE_HRESULT_ENTRY(DISP_E_PARAMNOTFOUND),
		MAKE_HRESULT_ENTRY(DISP_E_PARAMNOTOPTIONAL),
		MAKE_HRESULT_ENTRY(DISP_E_TYPEMISMATCH),
		MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNINTERFACE),
		MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNLCID),
		MAKE_HRESULT_ENTRY(DISP_E_UNKNOWNNAME),
		MAKE_HRESULT_ENTRY(DRAGDROP_E_ALREADYREGISTERED),
		MAKE_HRESULT_ENTRY(DRAGDROP_E_INVALIDHWND),
		MAKE_HRESULT_ENTRY(DRAGDROP_E_NOTREGISTERED),
		MAKE_HRESULT_ENTRY(DV_E_CLIPFORMAT),
		MAKE_HRESULT_ENTRY(DV_E_DVASPECT),
		MAKE_HRESULT_ENTRY(DV_E_DVTARGETDEVICE),
		MAKE_HRESULT_ENTRY(DV_E_DVTARGETDEVICE_SIZE),
		MAKE_HRESULT_ENTRY(DV_E_FORMATETC),
		MAKE_HRESULT_ENTRY(DV_E_LINDEX),
		MAKE_HRESULT_ENTRY(DV_E_NOIVIEWOBJECT),
		MAKE_HRESULT_ENTRY(DV_E_STATDATA),
		MAKE_HRESULT_ENTRY(DV_E_STGMEDIUM),
		MAKE_HRESULT_ENTRY(DV_E_TYMED),
		MAKE_HRESULT_ENTRY(INPLACE_E_NOTOOLSPACE),
		MAKE_HRESULT_ENTRY(INPLACE_E_NOTUNDOABLE),
		MAKE_HRESULT_ENTRY(MEM_E_INVALID_LINK),
		MAKE_HRESULT_ENTRY(MEM_E_INVALID_ROOT),
		MAKE_HRESULT_ENTRY(MEM_E_INVALID_SIZE),
		MAKE_HRESULT_ENTRY(MK_E_CANTOPENFILE),
		MAKE_HRESULT_ENTRY(MK_E_CONNECTMANUALLY),
		MAKE_HRESULT_ENTRY(MK_E_ENUMERATION_FAILED),
		MAKE_HRESULT_ENTRY(MK_E_EXCEEDEDDEADLINE),
		MAKE_HRESULT_ENTRY(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED),
		MAKE_HRESULT_ENTRY(MK_E_INVALIDEXTENSION),
		MAKE_HRESULT_ENTRY(MK_E_MUSTBOTHERUSER),
		MAKE_HRESULT_ENTRY(MK_E_NEEDGENERIC),
		MAKE_HRESULT_ENTRY(MK_E_NO_NORMALIZED),
		MAKE_HRESULT_ENTRY(MK_E_NOINVERSE),
		MAKE_HRESULT_ENTRY(MK_E_NOOBJECT),
		MAKE_HRESULT_ENTRY(MK_E_NOPREFIX),
		MAKE_HRESULT_ENTRY(MK_E_NOSTORAGE),
		MAKE_HRESULT_ENTRY(MK_E_NOTBINDABLE),
		MAKE_HRESULT_ENTRY(MK_E_NOTBOUND),
		MAKE_HRESULT_ENTRY(MK_E_SYNTAX),
		MAKE_HRESULT_ENTRY(MK_E_UNAVAILABLE),
		MAKE_HRESULT_ENTRY(OLE_E_ADVF),
		MAKE_HRESULT_ENTRY(OLE_E_ADVISENOTSUPPORTED),
		MAKE_HRESULT_ENTRY(OLE_E_BLANK),
		MAKE_HRESULT_ENTRY(OLE_E_CANT_BINDTOSOURCE),
		MAKE_HRESULT_ENTRY(OLE_E_CANT_GETMONIKER),
		MAKE_HRESULT_ENTRY(OLE_E_CANTCONVERT),
		MAKE_HRESULT_ENTRY(OLE_E_CLASSDIFF),
		MAKE_HRESULT_ENTRY(OLE_E_ENUM_NOMORE),
		MAKE_HRESULT_ENTRY(OLE_E_INVALIDHWND),
		MAKE_HRESULT_ENTRY(OLE_E_INVALIDRECT),
		MAKE_HRESULT_ENTRY(OLE_E_NOCACHE),
		MAKE_HRESULT_ENTRY(OLE_E_NOCONNECTION),
		MAKE_HRESULT_ENTRY(OLE_E_NOSTORAGE),
		MAKE_HRESULT_ENTRY(OLE_E_NOT_INPLACEACTIVE),
		MAKE_HRESULT_ENTRY(OLE_E_NOTRUNNING),
		MAKE_HRESULT_ENTRY(OLE_E_OLEVERB),
		MAKE_HRESULT_ENTRY(OLE_E_PROMPTSAVECANCELLED),
		MAKE_HRESULT_ENTRY(OLE_E_STATIC),
		MAKE_HRESULT_ENTRY(OLE_E_WRONGCOMPOBJ),
		MAKE_HRESULT_ENTRY(OLEOBJ_E_INVALIDVERB),
		MAKE_HRESULT_ENTRY(OLEOBJ_E_NOVERBS),
		MAKE_HRESULT_ENTRY(REGDB_E_CLASSNOTREG),
		MAKE_HRESULT_ENTRY(REGDB_E_IIDNOTREG),
		MAKE_HRESULT_ENTRY(REGDB_E_INVALIDVALUE),
		MAKE_HRESULT_ENTRY(REGDB_E_KEYMISSING),
		MAKE_HRESULT_ENTRY(REGDB_E_READREGDB),
		MAKE_HRESULT_ENTRY(REGDB_E_WRITEREGDB),
		MAKE_HRESULT_ENTRY(RPC_E_ATTEMPTED_MULTITHREAD),
		MAKE_HRESULT_ENTRY(RPC_E_CALL_CANCELED),
		MAKE_HRESULT_ENTRY(RPC_E_CALL_REJECTED),
		MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_AGAIN),
		MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_INASYNCCALL),
		MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_INEXTERNALCALL),
		MAKE_HRESULT_ENTRY(RPC_E_CANTCALLOUT_ININPUTSYNCCALL),
		MAKE_HRESULT_ENTRY(RPC_E_CANTPOST_INSENDCALL),
		MAKE_HRESULT_ENTRY(RPC_E_CANTTRANSMIT_CALL),
		MAKE_HRESULT_ENTRY(RPC_E_CHANGED_MODE),
		MAKE_HRESULT_ENTRY(RPC_E_CLIENT_CANTMARSHAL_DATA),
		MAKE_HRESULT_ENTRY(RPC_E_CLIENT_CANTUNMARSHAL_DATA),
		MAKE_HRESULT_ENTRY(RPC_E_CLIENT_DIED),
		MAKE_HRESULT_ENTRY(RPC_E_CONNECTION_TERMINATED),
		MAKE_HRESULT_ENTRY(RPC_E_DISCONNECTED),
		MAKE_HRESULT_ENTRY(RPC_E_FAULT),
		MAKE_HRESULT_ENTRY(RPC_E_INVALID_CALLDATA),
		MAKE_HRESULT_ENTRY(RPC_E_INVALID_DATA),
		MAKE_HRESULT_ENTRY(RPC_E_INVALID_DATAPACKET),
		MAKE_HRESULT_ENTRY(RPC_E_INVALID_PARAMETER),
		MAKE_HRESULT_ENTRY(RPC_E_INVALIDMETHOD),
		MAKE_HRESULT_ENTRY(RPC_E_NOT_REGISTERED),
		MAKE_HRESULT_ENTRY(RPC_E_OUT_OF_RESOURCES),
		MAKE_HRESULT_ENTRY(RPC_E_RETRY),
		MAKE_HRESULT_ENTRY(RPC_E_SERVER_CANTMARSHAL_DATA),
		MAKE_HRESULT_ENTRY(RPC_E_SERVER_CANTUNMARSHAL_DATA),
		MAKE_HRESULT_ENTRY(RPC_E_SERVER_DIED),
		MAKE_HRESULT_ENTRY(RPC_E_SERVER_DIED_DNE),
		MAKE_HRESULT_ENTRY(RPC_E_SERVERCALL_REJECTED),
		MAKE_HRESULT_ENTRY(RPC_E_SERVERCALL_RETRYLATER),
		MAKE_HRESULT_ENTRY(RPC_E_SERVERFAULT),
		MAKE_HRESULT_ENTRY(RPC_E_SYS_CALL_FAILED),
		MAKE_HRESULT_ENTRY(RPC_E_THREAD_NOT_INIT),
		MAKE_HRESULT_ENTRY(RPC_E_UNEXPECTED),
		MAKE_HRESULT_ENTRY(RPC_E_WRONG_THREAD),
		MAKE_HRESULT_ENTRY(STG_E_ABNORMALAPIEXIT),
		MAKE_HRESULT_ENTRY(STG_E_ACCESSDENIED),
		MAKE_HRESULT_ENTRY(STG_E_CANTSAVE),
		MAKE_HRESULT_ENTRY(STG_E_DISKISWRITEPROTECTED),
		MAKE_HRESULT_ENTRY(STG_E_EXTANTMARSHALLINGS),
		MAKE_HRESULT_ENTRY(STG_E_FILEALREADYEXISTS),
		MAKE_HRESULT_ENTRY(STG_E_FILENOTFOUND),
		MAKE_HRESULT_ENTRY(STG_E_INSUFFICIENTMEMORY),
		MAKE_HRESULT_ENTRY(STG_E_INUSE),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDFLAG),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDFUNCTION),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDHANDLE),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDHEADER),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDNAME),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDPARAMETER),
		MAKE_HRESULT_ENTRY(STG_E_INVALIDPOINTER),
		MAKE_HRESULT_ENTRY(STG_E_LOCKVIOLATION),
		MAKE_HRESULT_ENTRY(STG_E_MEDIUMFULL),
		MAKE_HRESULT_ENTRY(STG_E_NOMOREFILES),
		MAKE_HRESULT_ENTRY(STG_E_NOTCURRENT),
		MAKE_HRESULT_ENTRY(STG_E_NOTFILEBASEDSTORAGE),
		MAKE_HRESULT_ENTRY(STG_E_OLDDLL),
		MAKE_HRESULT_ENTRY(STG_E_OLDFORMAT),
		MAKE_HRESULT_ENTRY(STG_E_PATHNOTFOUND),
		MAKE_HRESULT_ENTRY(STG_E_READFAULT),
		MAKE_HRESULT_ENTRY(STG_E_REVERTED),
		MAKE_HRESULT_ENTRY(STG_E_SEEKERROR),
		MAKE_HRESULT_ENTRY(STG_E_SHAREREQUIRED),
		MAKE_HRESULT_ENTRY(STG_E_SHAREVIOLATION),
		MAKE_HRESULT_ENTRY(STG_E_TOOMANYOPENFILES),
		MAKE_HRESULT_ENTRY(STG_E_UNIMPLEMENTEDFUNCTION),
		MAKE_HRESULT_ENTRY(STG_E_UNKNOWN),
		MAKE_HRESULT_ENTRY(STG_E_WRITEFAULT),
		MAKE_HRESULT_ENTRY(TYPE_E_AMBIGUOUSNAME),
		MAKE_HRESULT_ENTRY(TYPE_E_BADMODULEKIND),
		MAKE_HRESULT_ENTRY(TYPE_E_BUFFERTOOSMALL),
		MAKE_HRESULT_ENTRY(TYPE_E_CANTCREATETMPFILE),
		MAKE_HRESULT_ENTRY(TYPE_E_CANTLOADLIBRARY),
		MAKE_HRESULT_ENTRY(TYPE_E_CIRCULARTYPE),
		MAKE_HRESULT_ENTRY(TYPE_E_DLLFUNCTIONNOTFOUND),
		MAKE_HRESULT_ENTRY(TYPE_E_DUPLICATEID),
		MAKE_HRESULT_ENTRY(TYPE_E_ELEMENTNOTFOUND),
		MAKE_HRESULT_ENTRY(TYPE_E_INCONSISTENTPROPFUNCS),
		MAKE_HRESULT_ENTRY(TYPE_E_INVALIDSTATE),
		MAKE_HRESULT_ENTRY(TYPE_E_INVDATAREAD),
		MAKE_HRESULT_ENTRY(TYPE_E_IOERROR),
		MAKE_HRESULT_ENTRY(TYPE_E_LIBNOTREGISTERED),
		MAKE_HRESULT_ENTRY(TYPE_E_NAMECONFLICT),
		MAKE_HRESULT_ENTRY(TYPE_E_OUTOFBOUNDS),
		MAKE_HRESULT_ENTRY(TYPE_E_QUALIFIEDNAMEDISALLOWED),
		MAKE_HRESULT_ENTRY(TYPE_E_REGISTRYACCESS),
		MAKE_HRESULT_ENTRY(TYPE_E_SIZETOOBIG),
		MAKE_HRESULT_ENTRY(TYPE_E_TYPEMISMATCH),
		MAKE_HRESULT_ENTRY(TYPE_E_UNDEFINEDTYPE),
		MAKE_HRESULT_ENTRY(TYPE_E_UNKNOWNLCID),
		MAKE_HRESULT_ENTRY(TYPE_E_UNSUPFORMAT),
		MAKE_HRESULT_ENTRY(TYPE_E_WRONGTYPEKIND),
		MAKE_HRESULT_ENTRY(VIEW_E_DRAW),

		MAKE_HRESULT_ENTRY(CONNECT_E_NOCONNECTION),
		MAKE_HRESULT_ENTRY(CONNECT_E_ADVISELIMIT),
		MAKE_HRESULT_ENTRY(CONNECT_E_CANNOTCONNECT),
		MAKE_HRESULT_ENTRY(CONNECT_E_OVERRIDDEN),

#ifndef NO_PYCOM_IPROVIDECLASSINFO
		MAKE_HRESULT_ENTRY(CLASS_E_NOTLICENSED),
		MAKE_HRESULT_ENTRY(CLASS_E_NOAGGREGATION),
		MAKE_HRESULT_ENTRY(CLASS_E_CLASSNOTAVAILABLE),
#endif // NO_PYCOM_IPROVIDECLASSINFO

#ifndef MS_WINCE // ??
		MAKE_HRESULT_ENTRY(CTL_E_ILLEGALFUNCTIONCALL      ),
		MAKE_HRESULT_ENTRY(CTL_E_OVERFLOW                 ),
		MAKE_HRESULT_ENTRY(CTL_E_OUTOFMEMORY              ),
		MAKE_HRESULT_ENTRY(CTL_E_DIVISIONBYZERO           ),
		MAKE_HRESULT_ENTRY(CTL_E_OUTOFSTRINGSPACE         ),
		MAKE_HRESULT_ENTRY(CTL_E_OUTOFSTACKSPACE          ),
		MAKE_HRESULT_ENTRY(CTL_E_BADFILENAMEORNUMBER      ),
		MAKE_HRESULT_ENTRY(CTL_E_FILENOTFOUND             ),
		MAKE_HRESULT_ENTRY(CTL_E_BADFILEMODE              ),
		MAKE_HRESULT_ENTRY(CTL_E_FILEALREADYOPEN          ),
		MAKE_HRESULT_ENTRY(CTL_E_DEVICEIOERROR            ),
		MAKE_HRESULT_ENTRY(CTL_E_FILEALREADYEXISTS        ),
		MAKE_HRESULT_ENTRY(CTL_E_BADRECORDLENGTH          ),
		MAKE_HRESULT_ENTRY(CTL_E_DISKFULL                 ),
		MAKE_HRESULT_ENTRY(CTL_E_BADRECORDNUMBER          ),
		MAKE_HRESULT_ENTRY(CTL_E_BADFILENAME              ),
		MAKE_HRESULT_ENTRY(CTL_E_TOOMANYFILES             ),
		MAKE_HRESULT_ENTRY(CTL_E_DEVICEUNAVAILABLE        ),
		MAKE_HRESULT_ENTRY(CTL_E_PERMISSIONDENIED         ),
		MAKE_HRESULT_ENTRY(CTL_E_DISKNOTREADY             ),
		MAKE_HRESULT_ENTRY(CTL_E_PATHFILEACCESSERROR      ),
		MAKE_HRESULT_ENTRY(CTL_E_PATHNOTFOUND             ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDPATTERNSTRING     ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDUSEOFNULL         ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDFILEFORMAT        ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDPROPERTYVALUE     ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDPROPERTYARRAYINDEX),
		MAKE_HRESULT_ENTRY(CTL_E_SETNOTSUPPORTEDATRUNTIME ),
		MAKE_HRESULT_ENTRY(CTL_E_SETNOTSUPPORTED          ),
		MAKE_HRESULT_ENTRY(CTL_E_NEEDPROPERTYARRAYINDEX   ),
		MAKE_HRESULT_ENTRY(CTL_E_SETNOTPERMITTED          ),
		MAKE_HRESULT_ENTRY(CTL_E_GETNOTSUPPORTEDATRUNTIME ),
		MAKE_HRESULT_ENTRY(CTL_E_GETNOTSUPPORTED          ),
		MAKE_HRESULT_ENTRY(CTL_E_PROPERTYNOTFOUND         ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDCLIPBOARDFORMAT   ),
		MAKE_HRESULT_ENTRY(CTL_E_INVALIDPICTURE           ),
		MAKE_HRESULT_ENTRY(CTL_E_PRINTERERROR             ),
		MAKE_HRESULT_ENTRY(CTL_E_CANTSAVEFILETOTEMP       ),
		MAKE_HRESULT_ENTRY(CTL_E_SEARCHTEXTNOTFOUND       ),
		MAKE_HRESULT_ENTRY(CTL_E_REPLACEMENTSTOOLONG      ),
#endif // MS_WINCE
	};
	#undef MAKE_HRESULT_ENTRY

	// first ask the OS to give it to us..
	// ### should we get the Unicode version instead?
	int numCopied = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, buf, bufSize, NULL );
	if (numCopied>0) {
		if (numCopied<bufSize) {
			// trim trailing crap
			if (numCopied>2 && (buf[numCopied-2]=='\n'||buf[numCopied-2]=='\r'))
				buf[numCopied-2] = '\0';
		}
		return;
	}

	// else look for it in the table
	for (int i = 0; i < _countof(hrNameTable); i++)
	{
		if (hr == hrNameTable[i].hr) {
			_tcsncpy(buf, hrNameTable[i].lpszName, bufSize);
			return;
		}
	}
	// not found - make one up
	wsprintf(buf, _T("OLE error 0x%08x"), hr);
}

LPCSTR GetScodeRangeString(HRESULT hr)
{
	struct RANGE_ENTRY
	{
		HRESULT hrFirst;
		HRESULT hrLast;
		LPCSTR lpszName;
	};
	#define MAKE_RANGE_ENTRY(hrRange) \
		{ hrRange##_FIRST, hrRange##_LAST, \
			#hrRange "_FIRST..." #hrRange "_LAST" }

	static const RANGE_ENTRY hrRangeTable[] =
	{
		MAKE_RANGE_ENTRY(CACHE_E),
		MAKE_RANGE_ENTRY(CACHE_S),
		MAKE_RANGE_ENTRY(CLASSFACTORY_E),
		MAKE_RANGE_ENTRY(CLASSFACTORY_S),
		MAKE_RANGE_ENTRY(CLIENTSITE_E),
		MAKE_RANGE_ENTRY(CLIENTSITE_S),
		MAKE_RANGE_ENTRY(CLIPBRD_E),
		MAKE_RANGE_ENTRY(CLIPBRD_S),
		MAKE_RANGE_ENTRY(CONVERT10_E),
		MAKE_RANGE_ENTRY(CONVERT10_S),
		MAKE_RANGE_ENTRY(CO_E),
		MAKE_RANGE_ENTRY(CO_S),
		MAKE_RANGE_ENTRY(DATA_E),
		MAKE_RANGE_ENTRY(DATA_S),
		MAKE_RANGE_ENTRY(DRAGDROP_E),
		MAKE_RANGE_ENTRY(DRAGDROP_S),
		MAKE_RANGE_ENTRY(ENUM_E),
		MAKE_RANGE_ENTRY(ENUM_S),
		MAKE_RANGE_ENTRY(INPLACE_E),
		MAKE_RANGE_ENTRY(INPLACE_S),
		MAKE_RANGE_ENTRY(MARSHAL_E),
		MAKE_RANGE_ENTRY(MARSHAL_S),
		MAKE_RANGE_ENTRY(MK_E),
		MAKE_RANGE_ENTRY(MK_S),
		MAKE_RANGE_ENTRY(OLEOBJ_E),
		MAKE_RANGE_ENTRY(OLEOBJ_S),
		MAKE_RANGE_ENTRY(OLE_E),
		MAKE_RANGE_ENTRY(OLE_S),
		MAKE_RANGE_ENTRY(REGDB_E),
		MAKE_RANGE_ENTRY(REGDB_S),
		MAKE_RANGE_ENTRY(VIEW_E),
		MAKE_RANGE_ENTRY(VIEW_S),
		MAKE_RANGE_ENTRY(CONNECT_E),
		MAKE_RANGE_ENTRY(CONNECT_S),

	};
	#undef MAKE_RANGE_ENTRY

	// look for it in the table
	for (int i = 0; i < _countof(hrRangeTable); i++)
	{
		if (hr >= hrRangeTable[i].hrFirst && hr <= hrRangeTable[i].hrLast)
			return hrRangeTable[i].lpszName;
	}
	return NULL;    // not found
}

LPCSTR GetSeverityString(HRESULT hr)
{
	static LPCSTR rgszSEVERITY[] =
	{
		"SEVERITY_SUCCESS",
		"SEVERITY_ERROR",
	};
	return rgszSEVERITY[HRESULT_SEVERITY(hr)];
}

LPCSTR GetFacilityString(HRESULT hr)
{
	static LPCSTR rgszFACILITY[] =
	{
		"FACILITY_NULL",
		"FACILITY_RPC",
		"FACILITY_DISPATCH",
		"FACILITY_STORAGE",
		"FACILITY_ITF",
		"FACILITY_0x05",
		"FACILITY_0x06",
		"FACILITY_WIN32",
		"FACILITY_WINDOWS",
		"FACILITY_SSPI/FACILITY_MQ", // SSPI from ADSERR.H, MQ from mq.h
		"FACILITY_CONTROL",
		"FACILITY_EDK",
		"FACILITY_INTERNET",
		"FACILITY_MEDIASERVER",
		"FACILITY_MSMQ",
		"FACILITY_SETUPAPI",
	};
	if (HRESULT_FACILITY(hr) >= _countof(rgszFACILITY))
		switch (HRESULT_FACILITY(hr)) {
			case 0x7FF:
				return "FACILITY_BACKUP";
			case 0x800:
				return "FACILITY_EDB";
			case 0x900:
				return "FACILITY_MDSI";
			default:
				return "<Unknown Facility>";
		}
	return rgszFACILITY[HRESULT_FACILITY(hr)];
}
