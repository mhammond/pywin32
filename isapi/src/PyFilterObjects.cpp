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
#include "pyFilterObjects.h"

// @doc

// @object HTTP_FILTER_VERSION|A Python interface to the ISAPI HTTP_FILTER_VERSION
// structure.
PyTypeObject PyFILTER_VERSIONType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_VERSION",
	sizeof(PyFILTER_VERSION),
	0,
	PyFILTER_VERSION::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyFILTER_VERSION::getattro,		/* tp_getattro */
	PyFILTER_VERSION::setattro,		/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
};


PyFILTER_VERSION::PyFILTER_VERSION(HTTP_FILTER_VERSION *pfv)
{
	ob_type = &PyFILTER_VERSIONType;
	_Py_NewReference(this);
	m_pfv = pfv;
}

PyFILTER_VERSION::~PyFILTER_VERSION()
{
}

PyObject *PyFILTER_VERSION::getattro(PyObject *self, PyObject *obname)
{
	PyFILTER_VERSION *me = (PyFILTER_VERSION *)self;
	if (!me->m_pfv)
		return PyErr_Format(PyExc_RuntimeError, "FILTER_VERSION structure no longer exists");
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	// @prop int|ServerFilterVersion|(read-only)
	if (_tcscmp(name, _T("ServerFilterVersion"))==0) {
		return PyInt_FromLong(me->m_pfv->dwServerFilterVersion);
	}
	// @prop int|FilterVersion|
	if (_tcscmp(name, _T("FilterVersion"))==0) {
		return PyInt_FromLong(me->m_pfv->dwFilterVersion);
	}
	// @prop int|Flags|
	if (_tcscmp(name, _T("Flags"))==0) {
		return PyInt_FromLong(me->m_pfv->dwFlags);
	}
	// @prop string|FilterDesc|
	if (_tcscmp(name, _T("FilterDesc"))==0) {
		return PyString_FromString(me->m_pfv->lpszFilterDesc);
	}
	return PyErr_Format(PyExc_AttributeError, "PyFILTER_VERSION has no attribute '%s'", name);
}

int PyFILTER_VERSION::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	PyFILTER_VERSION *me = (PyFILTER_VERSION *)self;
	if (!me->m_pfv) {
		PyErr_Format(PyExc_RuntimeError, "FILTER_VERSION structure no longer exists");
		return -1;
	}
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete FILTER_VERSION attributes");
		return -1;
	}
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("FilterVersion"))==0) {
		if (!PyInt_Check(v)) {
			PyErr_Format(PyExc_ValueError, "FilterVersion must be an int (got %s)", v->ob_type->tp_name);
			return -1;
		}
		me->m_pfv->dwFilterVersion = PyInt_AsLong(v);
	}
	else if (_tcscmp(name, _T("Flags"))==0) {
		if (!PyInt_Check(v)) {
			PyErr_Format(PyExc_ValueError, "Flags must be an int (got %s)", v->ob_type->tp_name);
			return -1;
		}
		me->m_pfv->dwFlags = PyInt_AsLong(v);
	}
	else if (_tcscmp(name, _T("FilterDesc"))==0) {
		DWORD size;
		const char *bytes = PyISAPIString_AsBytes(v, &size);
		if (!bytes)
			return -1;
		if (size > SF_MAX_FILTER_DESC_LEN) {
			PyErr_Format(PyExc_ValueError, "String is too long - max of %d chars", SF_MAX_FILTER_DESC_LEN);
			return -1;
		}
		strcpy(me->m_pfv->lpszFilterDesc, bytes);
	} else {
		return PyObject_GenericSetAttr(self, obname, v);
	}
	return 0;
}


void PyFILTER_VERSION::deallocFunc(PyObject *ob)
{
	delete (PyFILTER_VERSION *)ob;
}


/////////////////////////////////////////////////////////////////////
// filter context wrapper
/////////////////////////////////////////////////////////////////////

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define ECBOFF(e) offsetof(PyHFC, e)

// @pymethod object|HTTP_FILTER_CONTEXT|GetData|Obtains the data passed to
// The HttpFilterProc function.  This is not techinally part of the
// HTTP_FILTER_CONTEXT structure, but packaged here for convenience.
PyObject * PyHFC::GetData(PyObject *self, PyObject *args)
{
	PyHFC *me = (PyHFC *)self;
	// @rdesc The result depends on the value of <om HTTP_FILTER_CONTEXT.NotificationType>
	// @flagh NotificationType|Result type
	switch (me->m_notificationType) {
		// @flag SF_NOTIFY_URL_MAP|<o HTTP_FILTER_URL_MAP>
		case SF_NOTIFY_URL_MAP:
			return new PyURL_MAP(me);
		// @flag SF_NOTIFY_PREPROC_HEADERS|<o HTTP_FILTER_PREPROC_HEADERS>
		case SF_NOTIFY_PREPROC_HEADERS:
			return new PyPREPROC_HEADERS(me);
		// @flag SF_NOTIFY_LOG|<o HTTP_FILTER_LOG>
		case SF_NOTIFY_LOG:
			return new PyFILTER_LOG(me);
		// @flag SF_NOTIFY_SEND_RAW_DATA|<o HTTP_FILTER_RAW_DATA>
		// @flag SF_NOTIFY_READ_RAW_DATA|<o HTTP_FILTER_RAW_DATA>
		case SF_NOTIFY_SEND_RAW_DATA:
		case SF_NOTIFY_READ_RAW_DATA:
			return new PyRAW_DATA(me);
		// @flag SF_NOTIFY_AUTHENTICATION|<o HTTP_FILTER_AUTHENT>
		case SF_NOTIFY_AUTHENTICATION:
			return new PyAUTHENT(me);
		// todo:
		// SF_NOTIFY_ACCESS_DENIED HTTP_FILTER_ACCESS_DENIED
		// SF_NOTIFY_SEND_RESPONSE HTTP_FILTER_SEND_RESPONSE
		// SF_NOTIFY_AUTH_COMPLETE HTTP_FILTER_AUTH_COMPLETE_INFO 
		default:
			PyErr_Format(PyExc_ValueError, "Don't understand data of type 0x%x", me->m_notificationType);
			return NULL;
	}
	/* not reached */
	assert(false);
}

// @pymethod |HTTP_FILTER_CONTEXT|WriteClient|
PyObject * PyHFC::WriteClient(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	char *buffer = NULL;
	Py_ssize_t buffLen = 0;
	int reserved = 0;

	PyHFC * phfc = (PyHFC *) self;
	// @pyparm string|data||
	// @pyparm int|reserverd|0|
	if (!PyArg_ParseTuple(args, "s#|l:WriteClient", &buffer, &buffLen, &reserved))
		return NULL;

	if (phfc->m_pfc){
		HTTP_FILTER_CONTEXT *fc = phfc->m_pfc->m_pHFC;
		Py_BEGIN_ALLOW_THREADS
		DWORD dwBufLen = Py_SAFE_DOWNCAST(buffLen, Py_ssize_t, DWORD);
		bRes = fc->WriteClient(fc, (void *)buffer, &dwBufLen, reserved);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyHFCError("WriteClient");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |HTTP_FILTER_CONTEXT|AddResponseHeaders|
PyObject * PyHFC::AddResponseHeaders(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	char * buffer = NULL;
	int reserved = 0;

	PyHFC * phfc = (PyHFC *) self;
	// @pyparm string|data||
	// @pyparm int|reserverd|0|
	if (!PyArg_ParseTuple(args, "s|l:AddResponseHeaders", &buffer, &reserved))
		return NULL;

	if (phfc->m_pfc){
		Py_BEGIN_ALLOW_THREADS
		bRes = phfc->m_pfc->AddResponseHeaders(buffer, reserved);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyHFCError("AddResponseHeaders");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod string|HTTP_FILTER_CONTEXT|GetServerVariable|
// @rdesc The result is a string object, unless the server variable name
// begins with 'UNICODE_', in which case it is a unicode object - see the
// ISAPI docs for more details.
PyObject * PyHFC::GetServerVariable(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	char *variable = NULL;
	PyObject *def = NULL;

	PyHFC * phfc = (PyHFC *) self;

	// @pyparm string|variable||
	// @pyparm object|default||If specified, the function will return this
	// value instead of raising an error if the variable could not be fetched.
	if (!PyArg_ParseTuple(args, "s|O:GetServerVariable", &variable, &def))
		return NULL;

	char buf[8192] = "";
	DWORD bufsize = sizeof(buf)/sizeof(buf[0]);
	char *bufUse = buf;
	if (phfc->m_pfc){
		bRes = phfc->m_pfc->GetServerVariable(variable, buf, &bufsize);
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
				bRes = phfc->m_pfc->GetServerVariable(variable, bufUse, &bufsize);
				if (bRes || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					break;
			}
		}
		if (!bufUse)
			return PyErr_NoMemory();		
		if (!bRes) {
			if (bufUse != buf)
				free(bufUse);
			if (def) {
				Py_INCREF(def);
				return def;
			}
			return SetPyHFCError("GetServerVariable");
		}
	}
	PyObject *ret = strncmp("UNICODE_", variable, 8) == 0 ?
	                  PyUnicode_FromWideChar((WCHAR *)bufUse, bufsize / sizeof(WCHAR)) :
	                  PyString_FromStringAndSize(bufUse, bufsize);
	if (bufUse != buf)
		free(bufUse);
	return ret;
}

// @pymethod |HTTP_FILTER_CONTEXT|SendResponseHeader|
PyObject * PyHFC::SendResponseHeader(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	char *status, *header;
	PyHFC * phfc = (PyHFC *) self;
	// @pyparm string|status||
	// @pyparm string|header||
	if (!PyArg_ParseTuple(args, "zz:SendResponseHeader", &status, &header))
		return NULL;

	if (!phfc->m_pfc)
		return PyErr_Format(PyExc_RuntimeError, "No filtercontext!");
	Py_BEGIN_ALLOW_THREADS
	// The Java code passes "\r\n" as first DWORD, and header in second,
	// but docs clearly have second as unused.  Either way, I can't see the
	// specific header!
	bRes = phfc->m_pfc->ServerSupportFunction(SF_REQ_SEND_RESPONSE_HEADER,
	                                          status, (DWORD)header, 0);
	Py_END_ALLOW_THREADS
	if (!bRes)
		return SetPyHFCError("SendResponseHeader");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |HTTP_FILTER_CONTEXT|DisableNotifications|
PyObject * PyHFC::DisableNotifications(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	DWORD flags;
	PyHFC * phfc = (PyHFC *) self;
	// @pyparm int|flags||
	if (!PyArg_ParseTuple(args, "l:DisableNotifications", &flags))
		return NULL;

	if (!phfc->m_pfc)
		return PyErr_Format(PyExc_RuntimeError, "No filtercontext!");
	Py_BEGIN_ALLOW_THREADS
	bRes = phfc->m_pfc->ServerSupportFunction(SF_REQ_DISABLE_NOTIFICATIONS,
	                                          0, flags, 0);
	Py_END_ALLOW_THREADS
	if (!bRes)
		return SetPyHFCError("DisableNotifications");
	Py_INCREF(Py_None);
	return Py_None;
}

// @object HTTP_FILTER_CONTEXT|A Python representation of an ISAPI
// HTTP_FILTER_CONTEXT structure.
static struct PyMethodDef PyHFC_methods[] = {
	{"GetData",                 PyHFC::GetData, 1},	 // @pymeth GetData|
	{"GetServerVariable",       PyHFC::GetServerVariable, 1}, // @pymeth GetServerVariable|
	{"WriteClient",             PyHFC::WriteClient, 1},  // @pymeth WriteClient|
	{"AddResponseHeaders",      PyHFC::AddResponseHeaders, 1}, // @pymeth AddResponseHeaders|Specifies a response header for IIS to send to the client.
	{"write",				    PyHFC::WriteClient, 1},			 // @pymeth write|A synonym for WriteClient, this allows you to 'print >> fc'
	{"SendResponseHeader",      PyHFC::SendResponseHeader, 1}, // @pymeth SendResponseHeader|
	{"DisableNotifications",    PyHFC::DisableNotifications, 1}, // @pymeth DisableNotifications|
	{NULL}
};


struct PyMemberDef PyHFC::members[] = {
	// @prop int|Revision|(read-only)
	{"Revision",			T_INT,ECBOFF(m_revision), READONLY}, 
	// @prop bool|fIsSecurePort|(read-only)
	{"fIsSecurePort",			T_INT,	   ECBOFF(m_isSecurePort), READONLY}, 
	// @prop int|NotificationType|(read-only)
	{"NotificationType",			T_INT,ECBOFF(m_notificationType), READONLY}, 
	{NULL}
};

PyTypeObject PyHFCType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_CONTEXT",
	sizeof(PyHFC),
	0,
	PyHFC::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyHFC::getattro,			/* tp_getattro */
	PyHFC::setattro,			/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
	0,					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	PyHFC_methods,				/* tp_methods */
	PyHFC::members,				/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0,					/* tp_new */
};


PyHFC::PyHFC(CFilterContext* pfc)
{
	ob_type = &PyHFCType;
	_Py_NewReference(this);

	m_pfc = pfc;

	HTTP_FILTER_CONTEXT *phfc;
	VOID *pData;
	pfc->GetFilterData(&phfc, &m_notificationType, &pData);

	m_revision = phfc->Revision;
	m_isSecurePort = phfc->fIsSecurePort;
}

PyHFC::~PyHFC()
{
	if (m_pfc)
		delete m_pfc;
}


PyObject *PyHFC::getattro(PyObject *self, PyObject *obname)
{
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);

	// other manual attributes.
	if (_tcscmp(name, _T("FilterContext"))==0) {
	// @prop object|FilterContext|Any object you wish to associate with the request.

		HTTP_FILTER_CONTEXT *pfc;
		((PyHFC *)self)->GetFilterContext()->GetFilterData(&pfc, NULL, NULL);
		PyObject *ret = (PyObject *)pfc->pFilterContext;
		if (!ret)
			ret = Py_None;
		Py_INCREF(ret);
		return ret;
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyHFC::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete ECB attributes");
		return -1;
	}

	// other manual attributes.
	if (_tcscmp(name, _T("FilterContext"))==0) {
		HTTP_FILTER_CONTEXT *pfc;
		((PyHFC *)self)->GetFilterContext()->GetFilterData(&pfc, NULL, NULL);
		// Use C++ reference so pyfc really *is* pFilterContext
		PyObject *&pyfc = (PyObject *&)pfc->pFilterContext;
		Py_XDECREF(pyfc);
		if (v == Py_None)
			pyfc = NULL;
		else {
			pyfc = v;
			// This reference cleaned up in SF_NOTIFY_END_OF_NET_SESSION
			// handler in pyISAPI.cpp.
			Py_INCREF(v);
		}
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
}


void PyHFC::deallocFunc(PyObject *ob)
{
	delete (PyHFC *)ob;
}


// Setup an exception
PyObject * SetPyHFCError(char *fnName, long err /*= 0*/)
{
	DWORD errorCode = err == 0 ? GetLastError() : err;
    if (PyHFC_Error==NULL) {
        PyObject *mod = PyImport_ImportModule("isapi");
        if (mod)
            PyHFC_Error = PyObject_GetAttrString(mod, "FilterError");
        else
            PyHFC_Error = PyExc_RuntimeError; // what's the alternative?
        Py_XDECREF(mod);
    }
	PyObject *v = Py_BuildValue("(izs)", errorCode, NULL, fnName);
	if (v != NULL) {
		PyErr_SetObject(PyHFC_Error, v);
		Py_DECREF(v);
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////
// PyURL_MAP object
/////////////////////////////////////////////////////////////////////////
// @object HTTP_FILTER_URL_MAP|A Python representation of an ISAPI
// HTTP_FILTER_URL_MAP structure.
PyTypeObject PyURL_MAPType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_URL_MAP",
	sizeof(PyURL_MAP),
	0,
	PyURL_MAP::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyURL_MAP::getattro,			/* tp_getattro */
	PyURL_MAP::setattro,			/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
};


PyURL_MAP::PyURL_MAP(PyHFC *pParent)
{
	ob_type = &PyURL_MAPType;
	_Py_NewReference(this);

	m_parent = pParent;
	Py_INCREF(m_parent);
}

PyURL_MAP::~PyURL_MAP()
{
	Py_XDECREF(m_parent);
}

HTTP_FILTER_URL_MAP *PyURL_MAP::GetURLMap()
{
	HTTP_FILTER_CONTEXT *pFC;
	void *vdata;
	DWORD requestType;
    m_parent->GetFilterContext()->GetFilterData(&pFC, &requestType, &vdata);
    assert(requestType==SF_NOTIFY_URL_MAP);
    return (HTTP_FILTER_URL_MAP *)vdata;

}

PyObject *PyURL_MAP::getattro(PyObject *self, PyObject *obname)
{
	HTTP_FILTER_URL_MAP *pMap = ((PyURL_MAP *)self)->GetURLMap();
	if (!pMap)
		return NULL;
	// @prop string|URL|
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("URL"))==0) {
		return PyString_FromString(pMap->pszURL);
	}
	// @prop string|PhysicalPath|
	if (_tcscmp(name, _T("PhysicalPath"))==0) {
		return PyString_FromString(pMap->pszPhysicalPath);
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyURL_MAP::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	HTTP_FILTER_URL_MAP *pMap = ((PyURL_MAP *)self)->GetURLMap();
	if (!pMap)
		return NULL;
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("PhysicalPath"))==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_TypeError, "PhysicalPath must be a string");
			return -1;
		}
		int cc = PyString_Size(v);
		if ((DWORD)cc >= pMap->cbPathBuff) {
			PyErr_Format(PyExc_ValueError, "The string is too long - got %d chars, but max is %d", cc, pMap->cbPathBuff-1);
			return -1;
		}
		strcpy(pMap->pszPhysicalPath, PyString_AS_STRING(v));
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
}


void PyURL_MAP::deallocFunc(PyObject *ob)
{
	delete (PyURL_MAP *)ob;
}

/////////////////////////////////////////////////////////////////////////
// PyPREPROC_HEADERS object
/////////////////////////////////////////////////////////////////////////
// @pymethod string|HTTP_FILTER_PREPROC_HEADERS|GetHeader|
PyObject * PyPREPROC_HEADERS_GetHeader(PyObject *self, PyObject *args)
{
	char buffer[8192];
	DWORD bufSize = sizeof(buffer) / sizeof(buffer[0]);
	char *name;
	PyObject *def = NULL;
	// @pyparm string|header||
	// @pyparm object|default||If specified, this will be returned on error.
	if (!PyArg_ParseTuple(args, "s|O:GetHeader", &name, &def))
		return NULL;
	BOOL ok;
	HTTP_FILTER_PREPROC_HEADERS *pp = ((PyPREPROC_HEADERS *)self)->GetPREPROC_HEADERS();
	HTTP_FILTER_CONTEXT *pfc = ((PyPREPROC_HEADERS *)self)->GetFILTER_CONTEXT();
	if (!pp || !pfc)
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = pp->GetHeader(pfc, name, buffer, &bufSize);
	Py_END_ALLOW_THREADS
	if (!ok || bufSize==0) {
		if (def == NULL)
			return SetPyHFCError("GetHeader");
		Py_INCREF(def);
		return def;
	}
	return PyString_FromStringAndSize(buffer, bufSize-1);
}

// @pymethod |HTTP_FILTER_PREPROC_HEADERS|SetHeader|
PyObject * PyPREPROC_HEADERS_SetHeader(PyObject *self, PyObject *args)
{
	BOOL ok;
	char *name, *val;
	HTTP_FILTER_PREPROC_HEADERS *pp = ((PyPREPROC_HEADERS *)self)->GetPREPROC_HEADERS();
	HTTP_FILTER_CONTEXT *pfc = ((PyPREPROC_HEADERS *)self)->GetFILTER_CONTEXT();
	if (!pp || !pfc)
		return NULL;
	// @pyparm string|name||
	// @pyparm string|val||
	if (!PyArg_ParseTuple(args, "ss:SetHeader", &name, &val))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = pp->SetHeader(pfc, name, val);
	Py_END_ALLOW_THREADS
	if (!ok)
		return SetPyHFCError("SetHeader");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |HTTP_FILTER_PREPROC_HEADERS|AddHeader|
PyObject * PyPREPROC_HEADERS_AddHeader(PyObject *self, PyObject *args)
{
	BOOL ok;
	char *name, *val;
	HTTP_FILTER_PREPROC_HEADERS *pp = ((PyPREPROC_HEADERS *)self)->GetPREPROC_HEADERS();
	HTTP_FILTER_CONTEXT *pfc = ((PyPREPROC_HEADERS *)self)->GetFILTER_CONTEXT();
	if (!pp || !pfc)
		return NULL;
	if (!PyArg_ParseTuple(args, "ss:AddHeader", &name, &val))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = pp->AddHeader(pfc, name, val);
	Py_END_ALLOW_THREADS
	if (!ok)
		return SetPyHFCError("AddHeader");
	Py_INCREF(Py_None);
	return Py_None;
}

// @object HTTP_FILTER_PREPROC_HEADERS|A Python representation of an ISAPI
// HTTP_FILTER_PREPROC_HEADERS structure.
static struct PyMethodDef PyPREPROC_HEADERS_methods[] = {
	{"GetHeader",		PyPREPROC_HEADERS_GetHeader, 1}, // @pymeth GetHeader|
	{"SetHeader",		PyPREPROC_HEADERS_SetHeader, 1}, // @pymeth SetHeader|
	{"AddHeader",		PyPREPROC_HEADERS_AddHeader, 1}, // @pymeth AddHeader|
	{NULL}
};

PyTypeObject PyPREPROC_HEADERSType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_PREPROC_HEADERS",
	sizeof(PyPREPROC_HEADERS),
	0,
	PyPREPROC_HEADERS::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
	0,					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	PyPREPROC_HEADERS_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0,					/* tp_new */
};

PyPREPROC_HEADERS::PyPREPROC_HEADERS(PyHFC *pParent)
{
	ob_type = &PyPREPROC_HEADERSType;
	_Py_NewReference(this);

	m_parent = pParent;
	Py_INCREF(m_parent);
}

PyPREPROC_HEADERS::~PyPREPROC_HEADERS()
{
	Py_XDECREF(m_parent);
}

HTTP_FILTER_CONTEXT *PyPREPROC_HEADERS::GetFILTER_CONTEXT()
{
	HTTP_FILTER_CONTEXT *pFC;
	m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	return pFC;
}

HTTP_FILTER_PREPROC_HEADERS *PyPREPROC_HEADERS::GetPREPROC_HEADERS()
{
	HTTP_FILTER_CONTEXT *pFC;
	void *vdata;
	DWORD requestType;
	m_parent->GetFilterContext()->GetFilterData(&pFC, &requestType, &vdata);
	assert(requestType==SF_NOTIFY_PREPROC_HEADERS);
	return (HTTP_FILTER_PREPROC_HEADERS *)vdata;
}


void PyPREPROC_HEADERS::deallocFunc(PyObject *ob)
{
	delete (PyPREPROC_HEADERS *)ob;
}

/////////////////////////////////////////////////////////////////////////
// HTTP_FILTER_RAW_DATA object
/////////////////////////////////////////////////////////////////////////

// @object HTTP_FILTER_RAW_DATA|A Python representation of an ISAPI
// HTTP_FILTER_RAW_DATA structure.

PyTypeObject PyRAW_DATAType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_RAW_DATA",
	sizeof(PyRAW_DATA),
	0,
	PyRAW_DATA::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyRAW_DATA::getattro,			/* tp_getattro */
	PyRAW_DATA::setattro,			/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
};

PyRAW_DATA::PyRAW_DATA(PyHFC *pParent)
{
	ob_type = &PyRAW_DATAType;
	_Py_NewReference(this);

	m_parent = pParent;
	Py_INCREF(m_parent);
}

PyRAW_DATA::~PyRAW_DATA()
{
	Py_XDECREF(m_parent);
}

HTTP_FILTER_CONTEXT *PyRAW_DATA::GetFILTER_CONTEXT()
{
	HTTP_FILTER_CONTEXT *pFC;
	m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	return pFC;
}

HTTP_FILTER_RAW_DATA *PyRAW_DATA::GetRAW_DATA()
{
	HTTP_FILTER_CONTEXT *pFC;
	void *vdata;
	DWORD requestType;
	m_parent->GetFilterContext()->GetFilterData(&pFC, &requestType, &vdata);
	assert(requestType==SF_NOTIFY_SEND_RAW_DATA || requestType==SF_NOTIFY_READ_RAW_DATA);
	return (HTTP_FILTER_RAW_DATA *)vdata;
}

PyObject *PyRAW_DATA::getattro(PyObject *self, PyObject *obname)
{
	HTTP_FILTER_RAW_DATA *pRD = ((PyRAW_DATA*)self)->GetRAW_DATA();
	if (!pRD)
		return NULL;
	// @prop string|InData|
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("InData"))==0) {
		if (pRD->pvInData==NULL) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		return PyString_FromStringAndSize((const char *)pRD->pvInData,
						  pRD->cbInData);
	}
	
	return PyObject_GenericGetAttr(self, obname);
}

int PyRAW_DATA::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	HTTP_FILTER_RAW_DATA *pRD = ((PyRAW_DATA*)self)->GetRAW_DATA();
	HTTP_FILTER_CONTEXT *pFC = NULL;
	((PyRAW_DATA *)self)->m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	if (!pRD || !pFC)
		return NULL;

	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("InData"))==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_TypeError,
			             "InData must be a string (got %s)", v->ob_type->tp_name);
			return -1;
		}
		int cch = PyString_Size(v);
		void *nb = pFC->AllocMem(pFC, cch+sizeof(char), 0);
		if (nb) {
			pRD->cbInData = cch;
			pRD->cbInBuffer = cch+1;
			pRD->pvInData =  nb;
		} else {
			PyErr_NoMemory();
			return -1;
		}
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
}

void PyRAW_DATA::deallocFunc(PyObject *ob)
{
	delete (PyRAW_DATA *)ob;
}

/////////////////////////////////////////////////////////////////////////
// HTTP_FILTER_AUTHENT object
/////////////////////////////////////////////////////////////////////////

// @object HTTP_FILTER_AUTHENT|A Python representation of an ISAPI
// HTTP_FILTER_AUTHENT structure.

PyTypeObject PyAUTHENTType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_AUTHENT",
	sizeof(PyAUTHENT),
	0,
	PyAUTHENT::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyAUTHENT::getattro,			/* tp_getattro */
	PyAUTHENT::setattro,			/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
};

PyAUTHENT::PyAUTHENT(PyHFC *pParent)
{
	ob_type = &PyAUTHENTType;
	_Py_NewReference(this);

	m_parent = pParent;
	Py_INCREF(m_parent);
}

PyAUTHENT::~PyAUTHENT()
{
	Py_XDECREF(m_parent);
}

HTTP_FILTER_CONTEXT *PyAUTHENT::GetFILTER_CONTEXT()
{
	HTTP_FILTER_CONTEXT *pFC;
	m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	return pFC;
}

HTTP_FILTER_AUTHENT *PyAUTHENT::GetAUTHENT()
{
	HTTP_FILTER_CONTEXT *pFC;
	void *vdata;
	DWORD requestType;
	m_parent->GetFilterContext()->GetFilterData(&pFC, &requestType, &vdata);
	assert(requestType==SF_NOTIFY_AUTHENTICATION);
	return (HTTP_FILTER_AUTHENT *)vdata;
}

PyObject *PyAUTHENT::getattro(PyObject *self, PyObject *obname)
{
	HTTP_FILTER_AUTHENT *pAE = ((PyAUTHENT*)self)->GetAUTHENT();
	if (!pAE)
		return NULL;
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	// @prop string|User|
	if (_tcscmp(name, _T("User"))==0) {
		if (pAE->pszUser==NULL) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		return PyString_FromString((const char *)pAE->pszUser);
	}
	// @prop string|Password|
	if (_tcscmp(name, _T("Password"))==0) {
		if (pAE->pszPassword==NULL) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		return PyString_FromString((const char *)pAE->pszPassword);
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyAUTHENT::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	HTTP_FILTER_AUTHENT *pAE = ((PyAUTHENT*)self)->GetAUTHENT();
	HTTP_FILTER_CONTEXT *pFC = NULL;
	((PyAUTHENT *)self)->m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	if (!pAE || !pFC)
		return NULL;

	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	if (_tcscmp(name, _T("User"))==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_TypeError,
			             "User must be a string (got %s)", v->ob_type->tp_name);
			return -1;
		}
		DWORD cch = PyString_Size(v);
		if (cch >= pAE->cbUserBuff) {
			PyErr_Format(PyExc_ValueError, "The value is too long - max size is %d", pAE->cbUserBuff);
			return -1;
		}
		strcpy(pAE->pszUser, PyString_AS_STRING(v));
		return 0;
	}
	if (_tcscmp(name, _T("Password"))==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_TypeError,
			             "Password must be a string (got %s)", v->ob_type->tp_name);
			return -1;
		}
		DWORD cch = PyString_Size(v);
		if (cch >= pAE->cbPasswordBuff) {
			PyErr_Format(PyExc_ValueError, "The value is too long - max size is %d", pAE->cbPasswordBuff);
			return -1;
		}
		strcpy(pAE->pszPassword, PyString_AS_STRING(v));
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
}

void PyAUTHENT::deallocFunc(PyObject *ob)
{
	delete (PyAUTHENT *)ob;
}

/////////////////////////////////////////////////////////////////////////
// PyFILTER_LOG object
/////////////////////////////////////////////////////////////////////////
// @object HTTP_FILTER_LOG|A Python representation of an ISAPI
// HTTP_FILTER_LOG structure.
PyTypeObject PyFILTER_LOGType =
{
	PYISAPI_OBJECT_HEAD
	"HTTP_FILTER_LOG",
	sizeof(PyFILTER_LOG),
	0,
	PyFILTER_LOG::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
	PyFILTER_LOG::getattro,			/* tp_getattro */
	PyFILTER_LOG::setattro,			/* tp_setattro */
	0,					/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/* tp_flags */
};


PyFILTER_LOG::PyFILTER_LOG(PyHFC *pParent)
{
	ob_type = &PyFILTER_LOGType;
	_Py_NewReference(this);

	m_parent = pParent;
	Py_INCREF(m_parent);
}

PyFILTER_LOG::~PyFILTER_LOG()
{
	Py_XDECREF(m_parent);
}

HTTP_FILTER_LOG *PyFILTER_LOG::GetFilterLog()
{
	HTTP_FILTER_CONTEXT *pFC;
	void *vdata;
	DWORD requestType;
    m_parent->GetFilterContext()->GetFilterData(&pFC, &requestType, &vdata);
    assert(requestType==SF_NOTIFY_LOG);
    return (HTTP_FILTER_LOG *)vdata;

}

PyObject *PyFILTER_LOG::getattro(PyObject *self, PyObject *obname)
{
	HTTP_FILTER_LOG *pLog = ((PyFILTER_LOG *)self)->GetFilterLog();
	if (!pLog)
		return NULL;
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	// @prop string|ClientHostName|
	if (_tcscmp(name, _T("ClientHostName"))==0)
		return PyString_FromString(pLog->pszClientHostName);
	// @prop string|ClientUserName|
	if (_tcscmp(name, _T("ClientUserName"))==0)
		return PyString_FromString(pLog->pszClientUserName);
	// @prop string|ServerName|
	if (_tcscmp(name, _T("ServerName"))==0)
		return PyString_FromString(pLog->pszServerName);
	// @prop string|Operation|
	if (_tcscmp(name, _T("Operation"))==0)
		return PyString_FromString(pLog->pszOperation);
	// @prop string|Target|
	if (_tcscmp(name, _T("Target"))==0)
		return PyString_FromString(pLog->pszTarget);
	// @prop string|Parameters|
	if (_tcscmp(name, _T("Parameters"))==0)
		return PyString_FromString(pLog->pszParameters);
	// @prop int|HttpStatus|
	if (_tcscmp(name, _T("HttpStatus"))==0)
		return PyInt_FromLong(pLog->dwHttpStatus);
	// @prop int|HttpStatus|
	if (_tcscmp(name, _T("Win32Status"))==0)
		return PyInt_FromLong(pLog->dwWin32Status);
	return PyObject_GenericGetAttr(self, obname);
}

// Note that to set the strings, we use the AllocMem function - this allows
// IIS to automatically free the memory once the request has completed.

#define CHECK_SET_FILTER_LOG_STRING(struct_elem) \
	if (_tcscmp(name, _T(#struct_elem))==0) { \
		if (!PyString_Check(v)) { \
			PyErr_Format(PyExc_TypeError, #struct_elem " must be a string"); \
			return -1; \
		} \
		int cc = PyString_Size(v) + sizeof(CHAR); \
		char *buf = (char *)pFC->AllocMem(pFC, cc, 0); \
		if (!buf) { \
			PyErr_NoMemory(); \
			return -1; \
		} \
		strncpy(buf, PyString_AS_STRING(v), cc); \
		pLog->psz##struct_elem = buf; \
		return 0; \
	}

#define CHECK_SET_FILTER_LOG_LONG(struct_elem) \
	if (_tcscmp(name, _T(#struct_elem))==0) { \
		if (!PyInt_Check(v)) { \
			PyErr_Format(PyExc_TypeError, #struct_elem " must be an integer"); \
			return -1; \
		} \
		pLog->dw##struct_elem = PyInt_AsLong(v); \
		return 0; \
	}

int PyFILTER_LOG::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	HTTP_FILTER_CONTEXT *pFC;
    ((PyFILTER_LOG *)self)->m_parent->GetFilterContext()->GetFilterData(&pFC, NULL, NULL);
	HTTP_FILTER_LOG *pLog = ((PyFILTER_LOG *)self)->GetFilterLog();
	if (!pLog || !pFC)
		return NULL;
	TCHAR *name=PYISAPI_ATTR_CONVERT(obname);
	CHECK_SET_FILTER_LOG_STRING(ClientHostName)
	CHECK_SET_FILTER_LOG_STRING(ClientUserName)
	CHECK_SET_FILTER_LOG_STRING(ServerName)
	CHECK_SET_FILTER_LOG_STRING(Operation)
	CHECK_SET_FILTER_LOG_STRING(Target)
	CHECK_SET_FILTER_LOG_STRING(Parameters)
	CHECK_SET_FILTER_LOG_LONG(HttpStatus);
	CHECK_SET_FILTER_LOG_LONG(Win32Status);
	return PyObject_GenericSetAttr(self, obname, v);
}

void PyFILTER_LOG::deallocFunc(PyObject *ob)
{
	delete (PyFILTER_LOG *)ob;
}

void InitFilterTypes()
{
	PyType_Ready(&PyFILTER_VERSIONType);
	PyType_Ready(&PyHFCType);
	PyType_Ready(&PyURL_MAPType);
	PyType_Ready(&PyPREPROC_HEADERSType);
	PyType_Ready(&PyRAW_DATAType);
	PyType_Ready(&PyAUTHENTType);
	PyType_Ready(&PyFILTER_LOGType);
}
