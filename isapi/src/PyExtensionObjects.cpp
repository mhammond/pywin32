/*
 ======================================================================
 Copyright 2002-2003 by Blackdog Software Pty Ltd.

                         All Rights Reserved

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that copyright notice and this permission
 notice appear in supporting documentation, and that the name of 
 Blackdog Software not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.

 BLACKDOG SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 NO EVENT SHALL BLACKDOG SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ======================================================================
 */

#include "stdafx.h"
#include "Utils.h"
#include "PyExtensionObjects.h"
#include "PythonEng.h"

// @doc
// @object HSE_VERSION_INFO|An object used by ISAPI GetExtensionVersion
PyTypeObject PyVERSION_INFOType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"HSE_VERSION_INFO",
	sizeof(PyVERSION_INFO),
	0,
	PyVERSION_INFO::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyVERSION_INFO::getattr,		/* tp_getattr */
	PyVERSION_INFO::setattr,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
};


PyVERSION_INFO::PyVERSION_INFO(HSE_VERSION_INFO  *pvi)
{
	ob_type = &PyVERSION_INFOType;
	_Py_NewReference(this);
	m_pvi = pvi;
}

PyVERSION_INFO::~PyVERSION_INFO()
{
}

PyObject *PyVERSION_INFO::getattr(PyObject *self, char *name)
{
	PyVERSION_INFO *me = (PyVERSION_INFO *)self;
	if (!me->m_pvi)
		return PyErr_Format(PyExc_RuntimeError, "VERSION_INFO structure no longer exists");
	if (strcmp(name, "ExtensionDesc")==0) {
		return PyString_FromString(me->m_pvi->lpszExtensionDesc);
	}
	return PyErr_Format(PyExc_AttributeError, "PyVERSION_INFO has no attribute '%s'", name);
}

int PyVERSION_INFO::setattr(PyObject *self, char *name, PyObject *v)
{
	PyVERSION_INFO *me = (PyVERSION_INFO *)self;
	if (!me->m_pvi) {
		PyErr_Format(PyExc_RuntimeError, "VERSION_INFO structure no longer exists");
		return -1;
	}
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete VERSION_INFO attributes");
		return -1;
	}
	// @prop string|ExtensionDesc|The description of the extension.
	else if (strcmp(name, "ExtensionDesc")==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_ValueError, "FilterDesc must be a string (got %s)", v->ob_type->tp_name);
			return -1;
		}
		if (PyString_Size(v) > HSE_MAX_EXT_DLL_NAME_LEN) {
			PyErr_Format(PyExc_ValueError, "String is too long - max of %d chars", HSE_MAX_EXT_DLL_NAME_LEN);
			return -1;
		}
		strcpy(me->m_pvi->lpszExtensionDesc, PyString_AsString(v));
	} else {
		PyErr_SetString(PyExc_AttributeError, "can't modify read only VERSION_INFO attributes.");
		return -1;
	}
	return 0;
}


void PyVERSION_INFO::deallocFunc(PyObject *ob)
{
	delete (PyVERSION_INFO *)ob;
}


/////////////////////////////////////////////////////////////////////
// Extension block wrapper
/////////////////////////////////////////////////////////////////////

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define ECBOFF(e) offsetof(PyECB, e)

// @object EXTENSION_CONTROL_BLOCK|A python representation of an ISAPI
// EXTENSION_CONTROL_BLOCK.
struct memberlist PyECB::PyECB_memberlist[] = {
	{"Version",			T_INT,	   ECBOFF(m_version), READONLY}, 
	{"ConnID",			T_INT,	   ECBOFF(m_connID), READONLY}, 

	{"Method",			T_OBJECT,  ECBOFF(m_method), READONLY}, 
	{"QueryString",		T_OBJECT,  ECBOFF(m_queryString), READONLY}, 
	{"PathInfo",		T_OBJECT,  ECBOFF(m_pathInfo), READONLY}, 
	{"PathTranslated",	T_OBJECT,  ECBOFF(m_pathTranslated), READONLY}, 

	{"TotalBytes",		T_INT,	   ECBOFF(m_totalBytes), READONLY}, 
	{"AvailableBytes",	T_INT,	   ECBOFF(m_available), READONLY}, 
	{"AvailableData",	T_OBJECT,  ECBOFF(m_data), READONLY}, 
	{"ContentType",		T_OBJECT,  ECBOFF(m_contentType), READONLY}, 
	
	{"HttpStatusCode",	T_INT,  ECBOFF(m_HttpStatusCode)},  
	{"LogData",			T_OBJECT,  ECBOFF(m_logData)},
	{NULL}
};

static struct PyMethodDef PyECB_methods[] = {
	{"write",				    PyECB::WriteClient, 1},			 // @pymeth write|A synonym for WriteClient, this allows you to 'print >> ecb'
	{"WriteClient",				PyECB::WriteClient, 1},			 // @pymeth WriteClient|
	{"GetServerVariable",		PyECB::GetServerVariable, 1},	 // @pymeth GetServerVariable|
	{"ReadClient",				PyECB::ReadClient, 1},			 // @pymeth ReadClient|
	{"SendResponseHeaders",	    PyECB::SendResponseHeaders, 1},  // @pymeth SendResponseHeaders|
	{"TransmitFile",	    PyECB::TransmitFile, 1},  // @pymeth TransmitFile|
	{"MapURLToPath",	    PyECB::MapURLToPath, 1},  // @pymeth MapURLToPath|
	
	{"DoneWithSession",	        PyECB::DoneWithSession, 1},      // @pymeth DoneWithSession|
	{"close",                   PyECB::DoneWithSession, 1},      // @pymeth close|A synonym for DoneWithSession.
	{"IsSessionActive",			PyECB::IsSessionActive,1},       // @pymeth IsSessionActive|Indicates if DoneWithSession has been called
	{"Redirect",				PyECB::Redirect,1},              // @pymeth Redirect|
	{"IsKeepAlive",				PyECB::IsKeepAlive,1},           // @pymeth IsKeepAlive|
	{"GetImpersonationToken",   PyECB::GetImpersonationToken, 1}, // @pymeth GetImpersonationToken|
	{"IsKeepConn",              PyECB::IsKeepConn, 1}, // @pymeth IsKeepConn|Calls ServerSupportFunction with HSE_REQ_IS_KEEP_CONN
	{NULL}
};

PyTypeObject PyECBType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"EXTENSION_CONTROL_BLOCK",
	sizeof(PyECB),
	0,
	PyECB::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyECB::getattr,		/* tp_getattr */
	PyECB::setattr,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
};


PyECB::PyECB(CControlBlock * pcb):
		
	m_version(0),          // @prop integer|Version|Version info of this spec (read-only)
	m_connID(0),           // @prop integer|ConnID|Context number (read-only)

	m_method(NULL),         // @prop string|Method|REQUEST_METHOD
	m_queryString(NULL),    // @prop string|QueryString|QUERY_STRING
	m_pathInfo(NULL),       // @prop string|PathInfo|PATH_INFO
	m_pathTranslated(NULL), // @prop string|PathTranslated|PATH_TRANSLATED

	m_totalBytes(0),       // @prop int|TotalBytes|Total bytes indicated from client
	m_available(0),        // @prop int|AvailableBytes|Available number of bytes
	m_data(NULL),          // @prop string|AvailableData|Pointer to cbAvailable bytes
	m_contentType(NULL),   // @prop string|ContentType|Content type of client data

	m_HttpStatusCode(0),   // @prop int|HttpStatusCode|The status of the current transaction when the request is completed. 
	m_logData(NULL),       // @prop string|LogData|log data string
	
	m_bAsyncDone(false)    // async done
{
	ob_type = &PyECBType;
	_Py_NewReference(this);

	m_pcb = pcb;

	EXTENSION_CONTROL_BLOCK * pecb = pcb->GetECB();

	m_version = pecb->dwVersion; 
		
	m_connID		 = pecb->ConnID; 
	m_HttpStatusCode = pecb->dwHttpStatusCode; 
	m_logData		 = PyString_FromString("");
	m_method		 = PyString_FromString(pecb->lpszMethod); 
	m_queryString	 = PyString_FromString(pecb->lpszQueryString); 
	m_pathInfo       = PyString_FromString(pecb->lpszPathInfo); 
	m_pathTranslated = PyString_FromString(pecb->lpszPathTranslated);
	m_totalBytes	 = pecb->cbTotalBytes; 
	m_available		 = pecb->cbAvailable;
	m_data			 = PyString_FromStringAndSize((const char *) pecb->lpbData, pecb->cbAvailable); 
	m_contentType    = PyString_FromString(pecb->lpszContentType); 
}

PyECB::~PyECB()
{
	Py_XDECREF(m_logData);
	Py_XDECREF(m_method); 
	Py_XDECREF(m_queryString); 
	Py_XDECREF(m_pathInfo); 
	Py_XDECREF(m_pathTranslated);
	Py_XDECREF(m_data); 
	Py_XDECREF(m_contentType); 

	if (m_pcb)
		delete m_pcb;
}	


PyObject *PyECB::getattr(PyObject *self, char *name)
{
	// see if its a member variable
	for (int i=0; i<ARRAYSIZE(PyECB::PyECB_memberlist); i++){
		if (PyECB::PyECB_memberlist[i].name && _tcsicmp(name, PyECB::PyECB_memberlist[i].name) == 0)
			return PyMember_Get((char *)self, PyECB::PyECB_memberlist, name);
	}

	// see if its the special members attribute
	if (_tcscmp(name, _T("__members__"))==0)
		return PyMember_Get((char *)self, PyECB::PyECB_memberlist, name);

	if (_tcscmp(name, _T("softspace"))==0) // help 'print' semantics.
		return PyInt_FromLong(1);

	// must be a method
	return Py_FindMethod(PyECB_methods, self, name);
}

int PyECB::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete ECB attributes");
		return -1;
	}

	if (_tcscmp(name, _T("HttpStatusCode"))==0){
		int res = PyMember_Set((char *)self, PyECB::PyECB_memberlist, name, v);
		if (res == 0){
			DWORD status = PyInt_AsLong(v);
			PyECB * pecb = (PyECB *) self;
			if (pecb->m_pcb)
				pecb->m_pcb->SetStatus(status);
				
		}

		return res;
	}
	
	if ( _tcscmp(name, _T("LogData"))==0){
		int res = PyMember_Set((char *)self, PyECB::PyECB_memberlist, name, v);
		if (res == 0){
			char * logMsg = PyString_AsString(v);
			PyECB * pecb = (PyECB *) self;
			if (pecb->m_pcb)
				pecb->m_pcb->SetLogMessage(logMsg);
	
		}

		return res;
	}

	PyErr_SetString(PyExc_AttributeError, "can't modify read only ECB attributes only HTTPStatusCode and LogData can be changed.");
	return -1;

}


void PyECB::deallocFunc(PyObject *ob)
{
	delete (PyECB *)ob;
}


// @pymethod int|EXTENSION_CONTROL_BLOCK|WriteClient|
PyObject * PyECB::WriteClient(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * buffer = NULL;
	DWORD buffLen = 0;
	int reserved = 0;

	PyECB * pecb = (PyECB *) self;
	// @pyparm string/buffer|data||The data to write
	// @pyparm int|reserved|0|
	if (!PyArg_ParseTuple(args, "s#|l:WriteClient", &buffer, &buffLen, &reserved))
		return NULL;

	DWORD bytesWritten = 0;
	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->WriteClient(buffer, &buffLen, reserved);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyECBError("WriteClient");
	}
	return PyInt_FromLong(buffLen);
	// @rdesc the result is the number of bytes written.
}

// @pymethod string|EXTENSION_CONTROL_BLOCK|GetServerVariable|
PyObject * PyECB::GetServerVariable(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * variable = NULL;
	PyObject *def = NULL;

	PyECB * pecb = (PyECB *) self;
	// @pyparm string|variable||
	// @pyparm object|default||If specified, the function will return this
	// value instead of raising an error if the variable could not be fetched.
	if (!PyArg_ParseTuple(args, "s|O:GetServerVariable", &variable, &def))
		return NULL;

	char buf[8192] = "";
	DWORD bufsize = sizeof(buf);
	char *bufUse = buf;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->GetServerVariable(variable, buf, &bufsize);
		if (!bRes && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// Although the IIS docs say it should be good, IIS5
			// returns -1 for 'bufsize' and MS samples show not
			// to trust it too.  Like the MS sample, we max out
			// at some value - we choose 64k.  We double each
			// time, meaning we get 3 goes around the loop
			bufUse = NULL;
			bufsize = sizeof(buf);
			for (int i=0;i<3;i++) {
				bufsize *= 2;
				bufUse = (char *)realloc(bufUse, bufsize);
				if (!bufUse)
					break;
				bRes = pecb->m_pcb->GetServerVariable(variable, bufUse, &bufsize);
				if (bRes || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					break;
			}
		}
		Py_END_ALLOW_THREADS
		if (!bufUse)
			return PyErr_NoMemory();
		if (!bRes) {
			if (bufUse != buf)
				free(bufUse);
			if (def) {
				Py_INCREF(def);
				return def;
			}
			return SetPyECBError("GetServerVariable");
		}
	}
	PyObject *ret = strncmp("UNICODE_", variable, 8) == 0 ?
	                  PyUnicode_FromWideChar((WCHAR *)bufUse, bufsize / sizeof(WCHAR)) :
	                  PyString_FromStringAndSize(bufUse, bufsize);
	if (bufUse != buf)
		free(bufUse);
	return ret;
}

// @pymethod string|EXTENSION_CONTROL_BLOCK|ReadClient|
PyObject * PyECB::ReadClient(PyObject *self, PyObject *args)
{

	PyECB * pecb = (PyECB *) self;

	BOOL bRes = FALSE;
	BYTE * pBuff = NULL;
	DWORD nSize = 0;

	if (pecb->m_pcb){
		nSize = pecb->m_totalBytes - pecb->m_available;
	}
	// @pyparm int|nbytes|0|
	if (!PyArg_ParseTuple(args, "|l:ReadClient", &nSize))
		return NULL;
	
	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		if (nSize < 1)
			nSize = 1;

		DWORD orgSize = nSize;
		DWORD bytesGot= nSize;

		pBuff = new BYTE[nSize];
		bRes = pecb->m_pcb->ReadClient(pBuff, &bytesGot);
		if (bytesGot<orgSize){
			DWORD extraBytes = orgSize-bytesGot;
			DWORD offset=bytesGot;
            while (extraBytes > 0){
				bytesGot=extraBytes;
                bRes = pecb->m_pcb->ReadClient(&pBuff[offset], &bytesGot);
                if (bytesGot <1)
                    break;
                
                extraBytes -= bytesGot;
				offset += bytesGot;
			}
			if (extraBytes>0)
				nSize -= extraBytes;
		}


		Py_END_ALLOW_THREADS
		if (!bRes){
			delete [] pBuff;
			return SetPyECBError("ReadClient");
		}
	}

	PyObject * pyRes = NULL;
	if (nSize>0)
		pyRes =PyString_FromStringAndSize((LPCTSTR) pBuff, nSize);
	else
		pyRes = PyString_FromString("");

	delete [] pBuff;

	return pyRes;

}

// The following are wrappers for the various ServerSupportFunction
// @pymethod |EXTENSION_CONTROL_BLOCK|SendResponseHeaders|Calls ServerSupportFunction with HSE_REQ_SEND_RESPONSE_HEADER_EX 
PyObject * PyECB::SendResponseHeaders(PyObject *self, PyObject * args)
{
	BOOL bRes = FALSE;
	TCHAR * reply = NULL;
	TCHAR * headers = NULL;
	int bKeepAlive = 0;

	PyECB * pecb = (PyECB *) self;

	// @pyparm string|reply||
	// @pyparm string|headers||
	// @pyparm bool|keepAlive|False|
	if (!PyArg_ParseTuple(args, "ss|i:SendResponseHeaders", &reply, &headers, &bKeepAlive))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->WriteHeaders(reply,headers,(bKeepAlive!=0)?true:false);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyECBError("SendResponseHeaders");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |EXTENSION_CONTROL_BLOCK|Redirect|Calls ServerSupportFunction with HSE_REQ_SEND_URL_REDIRECT_RESP 
PyObject * PyECB::Redirect(PyObject *self, PyObject * args)
{
	BOOL bRes = FALSE;
	TCHAR * url = NULL;

	PyECB * pecb = (PyECB *) self;

	// @pyparm string|url||The URL to redirect to
	if (!PyArg_ParseTuple(args, "s:Redirect", &url))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->Redirect(url);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyECBError("Redirect");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|GetImpersonationToken|Calls ServerSupportFunction with HSE_REQ_GET_IMPERSONATION_TOKEN 
PyObject * PyECB::GetImpersonationToken(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetImpersonationToken"))
		return NULL;

	PyECB * pecb = (PyECB *) self;
	HANDLE handle;
	BOOL bRes;
	Py_BEGIN_ALLOW_THREADS
	bRes = pecb->m_pcb->GetImpersonationToken(&handle);
	Py_END_ALLOW_THREADS
	if (!bRes)
			return SetPyECBError("GetImpersonationToken");
	return PyLong_FromVoidPtr(handle);
}

// @pymethod int|EXTENSION_CONTROL_BLOCK|IsKeepConn|Calls ServerSupportFunction with HSE_REQ_IS_KEEP_CONN
PyObject * PyECB::IsKeepConn(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsKeepConn"))
		return NULL;

	PyECB * pecb = (PyECB *) self;
	BOOL bRes, bIs;
	Py_BEGIN_ALLOW_THREADS
	bRes = pecb->m_pcb->IsKeepConn(&bIs);
	Py_END_ALLOW_THREADS
	if (!bRes)
		return SetPyECBError("IsKeepCon");
	return PyBool_FromLong(bIs);
}


class PyTFD {
public:
	PyTFD(PyObject *aCallable, PyObject *aArg) {
		callable = aCallable;
		Py_INCREF(callable);
		arg = aArg;
		Py_INCREF(arg);
	}
	void Cleanup() { // NOTE: This not valid after Cleanup!
		Py_DECREF(callable);
		Py_DECREF(arg);
		delete this; // unusual, but ok :)
	}
	PyObject *callable;
	PyObject *arg;
};

VOID WINAPI transmitFileCompletion(EXTENSION_CONTROL_BLOCK * pECB,
                                   PVOID    pContext,
                                   DWORD    cbIO,
                                   DWORD    dwError)
{
	PyTFD *context = (PyTFD *)pContext;
	CEnterLeavePython celp;

	CControlBlock * pcb = new CControlBlock(pECB);
	// PyECB takes ownership of pcb - so when it dies, so does pcb.
	PyECB *pyECB = new PyECB(pcb);
	if (pyECB && pcb) {
		Py_INCREF(pyECB);
		PyObject *realArgs = Py_BuildValue("NOii", pyECB, context->arg, cbIO, dwError);
		if (realArgs) {
			PyObject *rc = PyObject_Call(context->callable, realArgs, NULL);
			if (rc)
				Py_DECREF(rc);
			else
				ExtensionError(pcb, "TransmitFile callback failed");
			Py_DECREF(realArgs);
		Py_DECREF(pyECB);
		}
	}
	context->Cleanup(); // let's hope we never get called again ;)
}


// @pymethod int|EXTENSION_CONTROL_BLOCK|TransmitFile|Calls ServerSupportFunction with HSE_REQ_TRANSMIT_FILE
PyObject * PyECB::TransmitFile(PyObject *self, PyObject *args)
{
	PY_LONG_LONG hFile; // int no good for 64bit - but can find no "pointer" format!
	HSE_TF_INFO info;
	memset(&info, 0, sizeof(info));
	PyObject *obCallback, *obCallbackParam;
	if (!PyArg_ParseTuple(args, "OOKsiiz#z#i:TransmitFile",
			      &obCallback, // @pyparm callable|callback||
			      &obCallbackParam, // @pyparm object|param||Any object - passed as 2nd arg to callback.
			      &hFile, // @pyparm int|hFile||
			      &info.pszStatusCode, // @pyparm string|statusCode||
			      &info.BytesToWrite, // @pyparm int|BytesToWrite||
			      &info.Offset, // @pyparm int|Offset||
			      &info.pHead, // @pyparm string|head||
			      &info.HeadLength,
			      &info.pTail, // @pyparm string|tail||
			      &info.TailLength,
			      &info.dwFlags // @pyparm int|flags||
			      ))
		return NULL;
	info.hFile = (HANDLE)hFile;
	// @comm The callback is called with 4 args - (<o PyECB>, param, cbIO, dwErrCode)

	if (!PyCallable_Check(obCallback))
		return PyErr_Format(PyExc_TypeError, "Callback is not callable");
	// The 'pContext' is a pointer to a PyTFD structure.  The callback
	// also free's the memory.
	PyTFD *context = new PyTFD(obCallback, obCallbackParam);
	if (!context)
		return PyErr_NoMemory();
	info.pfnHseIO = transmitFileCompletion;
	info.pContext = context;

	PyECB * pecb = (PyECB *) self;

	BOOL bRes;
	Py_BEGIN_ALLOW_THREADS
	bRes = pecb->m_pcb->TransmitFile(&info);
	Py_END_ALLOW_THREADS
	if (!bRes) {
		// ack - the completion routine will not be called - clean up!
		context->Cleanup();
		return SetPyECBError("TransmitFile");
	}
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |EXTENSION_CONTROL_BLOCK|IsKeepAlive|
// @comm This method simply checks a HTTP_CONNECTION header for 'keep-alive',
// making it fairly useless.  See <om EXTENSION_CONTROL_BLOCK.IsKeepCon>
PyObject * PyECB::IsKeepAlive(PyObject *self, PyObject * args)
{
	bool bKeepAlive = false;

	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, ":IsKeepAlive"))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bKeepAlive = pecb->m_pcb->IsKeepAlive();
		Py_END_ALLOW_THREADS
	}

	return PyInt_FromLong((bKeepAlive)?1:0);
}

// @pymethod |EXTENSION_CONTROL_BLOCK|MapURLToPath|
PyObject * PyECB::MapURLToPath(PyObject *self, PyObject * args)
{
	PyECB * pecb = (PyECB *) self;
	// todo - handle ERROR_INSUFFICIENT_BUFFER - but 4k will do for now.
	char buffer[1024*4];

	char *url;
	if (!PyArg_ParseTuple(args, "s:MapURLToPath", &url))
		return NULL;
	strncpy(buffer, url, sizeof(buffer));
	buffer[sizeof(buffer)-1] = '\0';
	DWORD bufSize = sizeof(buffer);
	BOOL ok;

	Py_BEGIN_ALLOW_THREADS
	ok = pecb->m_pcb->MapURLToPath(buffer, &bufSize);
	Py_END_ALLOW_THREADS
	if (!ok)
		return SetPyECBError("MapURLToPath");
	return PyString_FromString(buffer);
}

// @pymethod |EXTENSION_CONTROL_BLOCK|DoneWithSession|Calls ServerSupportFunction with HSE_REQ_DONE_WITH_SESSION 
PyObject * PyECB::DoneWithSession(PyObject *self, PyObject * args)
{
	DWORD status = HSE_STATUS_SUCCESS;
	PyECB * pecb = (PyECB *) self;

	// @pyparm int|status|HSE_STATUS_SUCCESS|An optional status.
	// HSE_STATUS_SUCCESS_AND_KEEP_CONN is supported by IIS to keep the connection alive.
	if (!PyArg_ParseTuple(args, "|i:DoneWithSession", &status))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		pecb->m_pcb->DoneWithSession(status);
		pecb->m_bAsyncDone = true;
		Py_END_ALLOW_THREADS
	}
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod bool|EXTENSION_CONTROL_BLOCK|IsSessionActive|Indicates if <om EXTENSION_CONTROL_BLOCK.DoneWithSession>
// has been called.
PyObject * PyECB::IsSessionActive(PyObject *self, PyObject * args)
{
	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, ":IsSessionActive"))
		return NULL;
	
	BOOL bActive = FALSE;
	if (pecb->m_pcb){
		bActive = (pecb->m_bAsyncDone) ? FALSE : TRUE;
	}
	return PyBool_FromLong(bActive);
}

// Setup an exception

PyObject * SetPyECBError(char *fnName, long err /*= 0*/)
{
	DWORD errorCode = err == 0 ? GetLastError() : err;
    if (PyECB_Error==NULL) {
        PyObject *mod = PyImport_ImportModule("isapi");
        if (mod)
            PyECB_Error = PyObject_GetAttrString(mod, "ExtensionError");
        else
            PyECB_Error = PyExc_RuntimeError; // what's the alternative?
        Py_XDECREF(mod);
    }
	PyObject *v = Py_BuildValue("(izs)", errorCode, NULL, fnName);
	if (v != NULL) {
		PyErr_SetObject(PyECB_Error, v);
		Py_DECREF(v);
	}
	return NULL;
}
