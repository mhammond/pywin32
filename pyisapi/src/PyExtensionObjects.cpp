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


PyTypeObject PyVERSION_INFOType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyVERSION_INFO",
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


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define ECBOFF(e) offsetof(PyECB, e)


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
	{"SendResponseHeaders",	    PyECB::SendResponseHeaders, 1},  	
	{"DoneWithSession",	        PyECB::DoneWithSession, 1}, 
	{"IsSessionActive",			PyECB::IsSessionActive,1},
	{"Redirect",				PyECB::Redirect,1},
	{"IsKeepAlive",				PyECB::IsKeepAlive,1},
	{"GetImpersonationToken",   PyECB::GetImpersonationToken, 1}, // @pymeth GetImpersonationToken|
	{NULL}
};

PyTypeObject PyECBType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyECB",
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
	m_version(0),          // Version info of this spec
	m_connID(0),           // Context number not to be modified!

	m_method(NULL),         // REQUEST_METHOD
	m_queryString(NULL),    // QUERY_STRING
	m_pathInfo(NULL),       // PATH_INFO
	m_pathTranslated(NULL), // PATH_TRANSLATED

	m_totalBytes(0),       // Total bytes indicated from client
	m_available(0),        // Available number of bytes
	m_data(NULL),          // Pointer to cbAvailable bytes
	m_contentType(NULL),   // Content type of client data

	m_HttpStatusCode(0),   // The status of the current transaction when the request is completed. 
	m_logData(NULL),       // log data string 
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


PyObject * PyECB::WriteClient(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * buffer = NULL;
	int buffLen = 0;
	int reserved = 0;

	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, "s#|l:WriteClient", &buffer, &buffLen, &reserved))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->WriteStream(buffer, buffLen, reserved);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyECBError("WriteClient");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject * PyECB::GetServerVariable(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * variable = NULL;

	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, "s:GetServerVariable", &variable))
		return NULL;

	char buf[8192] = "";
	DWORD bufsize = sizeof(buf)/sizeof(buf[0]);

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		bRes = pecb->m_pcb->GetServerVariable(variable, buf, &bufsize);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyECBError("GetServerVariable");
	}
	return PyString_FromStringAndSize(buf, bufsize);
}

PyObject * PyECB::ReadClient(PyObject *self, PyObject *args)
{

	PyECB * pecb = (PyECB *) self;

	BOOL bRes = FALSE;
	BYTE * pBuff = NULL;
	DWORD nSize = 0;

	if (pecb->m_pcb){
		nSize = pecb->m_totalBytes - pecb->m_available;
	}

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

// HSE_REQ_SEND_RESPONSE_HEADER_EX 
	
PyObject * PyECB::SendResponseHeaders(PyObject *self, PyObject * args)
{
	BOOL bRes = FALSE;
	TCHAR * reply = NULL;
	TCHAR * headers = NULL;
	int bKeepAlive = 0;

	PyECB * pecb = (PyECB *) self;

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

//  HSE_REQ_SEND_URL_REDIRECT_RESP.

PyObject * PyECB::Redirect(PyObject *self, PyObject * args)
{
	BOOL bRes = FALSE;
	TCHAR * url = NULL;

	PyECB * pecb = (PyECB *) self;

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

// HSE_REQ_GET_IMPERSONATION_TOKEN
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
 //HSE_REQ_DONE_WITH_SESSION

PyObject * PyECB::DoneWithSession(PyObject *self, PyObject * args)
{
	BOOL bRes = FALSE;
	int bKeepAlive = 0;

	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, "|i:DoneWithSession", &bKeepAlive))
		return NULL;

	if (pecb->m_pcb){
		Py_BEGIN_ALLOW_THREADS
		pecb->m_pcb->SignalAsyncRequestDone((bKeepAlive!=0)?true:false);
		pecb->m_bAsyncDone = true;
		Py_END_ALLOW_THREADS
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject * PyECB::IsSessionActive(PyObject *self, PyObject * args)
{
	PyECB * pecb = (PyECB *) self;

	if (!PyArg_ParseTuple(args, ":SessionActive"))
		return NULL;
	
	BOOL bActive = FALSE;
	if (pecb->m_pcb){
		bActive = (pecb->m_bAsyncDone) ? FALSE : TRUE;
	}

	return PyInt_FromLong(bActive);
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
