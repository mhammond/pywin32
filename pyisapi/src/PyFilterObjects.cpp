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


PyTypeObject PyFILTER_VERSIONType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyFILTER_VERSION",
	sizeof(PyFILTER_VERSION),
	0,
	PyFILTER_VERSION::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyFILTER_VERSION::getattr,		/* tp_getattr */
	PyFILTER_VERSION::setattr,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
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

PyObject *PyFILTER_VERSION::getattr(PyObject *self, char *name)
{
	PyFILTER_VERSION *me = (PyFILTER_VERSION *)self;
	if (!me->m_pfv)
		return PyErr_Format(PyExc_RuntimeError, "FILTER_VERSION structure no longer exists");
	if (strcmp(name, "ServerFilterVersion")==0) {
		return PyInt_FromLong(me->m_pfv->dwServerFilterVersion);
	}
	if (strcmp(name, "FilterVersion")==0) {
		return PyInt_FromLong(me->m_pfv->dwFilterVersion);
	}
	if (strcmp(name, "Flags")==0) {
		return PyInt_FromLong(me->m_pfv->dwFlags);
	}
	if (strcmp(name, "FilterDesc")==0) {
		return PyString_FromString(me->m_pfv->lpszFilterDesc);
	}
	return PyErr_Format(PyExc_AttributeError, "PyFILTER_VERSION has no attribute '%s'", name);
}

int PyFILTER_VERSION::setattr(PyObject *self, char *name, PyObject *v)
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
	if (strcmp(name, "FilterVersion")==0) {
		if (!PyInt_Check(v)) {
			PyErr_Format(PyExc_ValueError, "FilterVersion must be an int (got %s)", v->ob_type->tp_name);
			return -1;
		}
		me->m_pfv->dwFilterVersion = PyInt_AsLong(v);
	}
	else if (strcmp(name, "Flags")==0) {
		if (!PyInt_Check(v)) {
			PyErr_Format(PyExc_ValueError, "Flags must be an int (got %s)", v->ob_type->tp_name);
			return -1;
		}
		me->m_pfv->dwFlags = PyInt_AsLong(v);
	}
	else if (strcmp(name, "FilterDesc")==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_ValueError, "FilterDesc must be a string (got %s)", v->ob_type->tp_name);
			return -1;
		}
		if (PyString_Size(v) > SF_MAX_FILTER_DESC_LEN) {
			PyErr_Format(PyExc_ValueError, "String is too long - max of %d chars", SF_MAX_FILTER_DESC_LEN);
			return -1;
		}
		strcpy(me->m_pfv->lpszFilterDesc, PyString_AsString(v));
	} else {
		PyErr_SetString(PyExc_AttributeError, "can't modify read only FILTER_VERSION attributes.");
		return -1;
	}
	return 0;
}


void PyFILTER_VERSION::deallocFunc(PyObject *ob)
{
	delete (PyFILTER_VERSION *)ob;
}


/////////////////////////////////////////////////////////////////////
// Extension block wrapper
/////////////////////////////////////////////////////////////////////


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define ECBOFF(e) offsetof(PyHFC, e)

PyObject * PyHFC::GetData(PyObject *self, PyObject *args)
{
	PyHFC *me = (PyHFC *)self;
	switch (me->m_notificationType) {
		case SF_NOTIFY_URL_MAP:
			return new PyURL_MAP(me);
		case SF_NOTIFY_PREPROC_HEADERS:
			return new PyPREPROC_HEADERS(me);
		default:
			PyErr_Format(PyExc_ValueError, "Don't understand data of type 0x%x", me->m_notificationType);
			return NULL;
	}
	/* not reached */
	assert(false);
}

PyObject * PyHFC::WriteClient(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * buffer = NULL;
	int buffLen = 0;
	int reserved = 0;

	PyHFC * phfc = (PyHFC *) self;
	if (!PyArg_ParseTuple(args, "s#|l:WriteClient", &buffer, &buffLen, &reserved))
		return NULL;

	if (phfc->m_pfc){
		Py_BEGIN_ALLOW_THREADS
		bRes = phfc->m_pfc->WriteClient(buffer, buffLen, reserved);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyHFCError("WriteClient");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject * PyHFC::GetServerVariable(PyObject *self, PyObject *args)
{
	BOOL bRes = FALSE;
	TCHAR * variable = NULL;

	PyHFC * phfc = (PyHFC *) self;

	if (!PyArg_ParseTuple(args, "s:GetServerVariable", &variable))
		return NULL;

	char buf[8192] = "";
	DWORD bufsize = sizeof(buf)/sizeof(buf[0]);
	if (phfc->m_pfc){
		Py_BEGIN_ALLOW_THREADS
		bRes = phfc->m_pfc->GetServerVariable(variable, buf, &bufsize);
		Py_END_ALLOW_THREADS
		if (!bRes)
			return SetPyHFCError("GetServerVariable");
	}
	return PyString_FromStringAndSize(buf, bufsize);
}

// @pymethod |SendResponseHeader||
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

static struct PyMethodDef PyHFC_methods[] = {
	{"GetData",                 PyHFC::GetData, 1},	 // @pymeth |
	{"GetServerVariable",       PyHFC::GetServerVariable, 1}, // @pymeth |
	{"WriteClient",             PyHFC::WriteClient, 1},  // @pymeth |
	{"SendResponseHeader",      PyHFC::SendResponseHeader, 1}, // @pymeth |
	{NULL}
};


struct memberlist PyHFC::PyHFC_memberlist[] = {
	{"Revision",			T_INT,ECBOFF(m_revision), READONLY}, 
	{"fIsSecurePort",			T_INT,	   ECBOFF(m_isSecurePort), READONLY}, 
	{"NotificationType",			T_INT,ECBOFF(m_notificationType), READONLY}, 
	{NULL}
};

PyTypeObject PyHFCType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyHFC",
	sizeof(PyHFC),
	0,
	PyHFC::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyHFC::getattr,		/* tp_getattr */
	PyHFC::setattr,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
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


PyObject *PyHFC::getattr(PyObject *self, char *name)
{
	// see if its a member variable
	for (int i=0; i<ARRAYSIZE(PyHFC::PyHFC_memberlist); i++){
		if (PyHFC::PyHFC_memberlist[i].name && _tcsicmp(name, PyHFC::PyHFC_memberlist[i].name) == 0)
			return PyMember_Get((char *)self, PyHFC::PyHFC_memberlist, name);
	}

	// see if its the special members attribute
	if (_tcscmp(name, _T("__members__"))==0)
		return PyMember_Get((char *)self, PyHFC::PyHFC_memberlist, name);

	// must be a method
	return Py_FindMethod(PyHFC_methods, self, name);
}

int PyHFC::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete ECB attributes");
		return -1;
	}

	PyErr_SetString(PyExc_AttributeError, "can't modify read only ECB attributes.");
	return -1;
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
PyTypeObject PyURL_MAPType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyURL_MAP",
	sizeof(PyURL_MAP),
	0,
	PyURL_MAP::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyURL_MAP::getattr,		/* tp_getattr */
	PyURL_MAP::setattr,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
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

PyObject *PyURL_MAP::getattr(PyObject *self, char *name)
{
	HTTP_FILTER_URL_MAP *pMap = ((PyURL_MAP *)self)->GetURLMap();
	if (!pMap)
		return NULL;

	if (strcmp(name, "URL")==0) {
		return PyString_FromString(pMap->pszURL);
	}
	if (strcmp(name, "PhysicalPath")==0) {
		return PyString_FromString(pMap->pszPhysicalPath);
	}
	PyErr_Format(PyExc_AttributeError, "PyURL_MAP objects have no attribute '%s'", name);
	return NULL;
}

int PyURL_MAP::setattr(PyObject *self, char *name, PyObject *v)
{
	HTTP_FILTER_URL_MAP *pMap = ((PyURL_MAP *)self)->GetURLMap();
	if (!pMap)
		return NULL;
	if (strcmp(name, "PhysicalPath")==0) {
		if (!PyString_Check(v)) {
			PyErr_Format(PyExc_TypeError, "PhysicalPath must be a string");
			return -1;
		}
		int cc = PyString_Size(v);
		if ((DWORD)cc >= pMap->cbPathBuff) {
			PyErr_Format(PyExc_ValueError, "The string is too long - got %d chars, but max is %d", cc, pMap->cbPathBuff-1);
			return -1;
		}
		_tcscpy(pMap->pszPhysicalPath, PyString_AS_STRING(v));
		return 0;
	}
	PyErr_SetString(PyExc_AttributeError, "can't modify read only PyURL_MAP attributes.");
	return -1;
}


void PyURL_MAP::deallocFunc(PyObject *ob)
{
	delete (PyURL_MAP *)ob;
}

/////////////////////////////////////////////////////////////////////////
// PyPREPROC_HEADERS object
/////////////////////////////////////////////////////////////////////////
PyObject * PyPREPROC_HEADERS_GetHeader(PyObject *self, PyObject *args)
{
	TCHAR buffer[8192];
	DWORD bufSize = sizeof(buffer) / sizeof(TCHAR);
	char *name;
	if (!PyArg_ParseTuple(args, "s:GetHeader", &name))
		return NULL;
	BOOL ok;
	HTTP_FILTER_PREPROC_HEADERS *pp = ((PyPREPROC_HEADERS *)self)->GetPREPROC_HEADERS();
	HTTP_FILTER_CONTEXT *pfc = ((PyPREPROC_HEADERS *)self)->GetFILTER_CONTEXT();
	if (!pp || !pfc)
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = pp->GetHeader(pfc, name, buffer, &bufSize);
	Py_END_ALLOW_THREADS
	if (!ok)
		return SetPyHFCError("GetHeader");
	return PyString_FromStringAndSize(buffer, bufSize-1);
}

PyObject * PyPREPROC_HEADERS_SetHeader(PyObject *self, PyObject *args)
{
	BOOL ok;
	char *name, *val;
	HTTP_FILTER_PREPROC_HEADERS *pp = ((PyPREPROC_HEADERS *)self)->GetPREPROC_HEADERS();
	HTTP_FILTER_CONTEXT *pfc = ((PyPREPROC_HEADERS *)self)->GetFILTER_CONTEXT();
	if (!pp || !pfc)
		return NULL;
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

static struct PyMethodDef PyPREPROC_HEADERS_methods[] = {
	{"GetHeader",		PyPREPROC_HEADERS_GetHeader, 1}, // @pymeth |
	{"SetHeader",		PyPREPROC_HEADERS_SetHeader, 1}, // @pymeth |
	{"AddHeader",		PyPREPROC_HEADERS_AddHeader, 1}, // @pymeth |
	{NULL}
};

PyTypeObject PyPREPROC_HEADERSType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyPREPROC_HEADERS",
	sizeof(PyPREPROC_HEADERS),
	0,
	PyPREPROC_HEADERS::deallocFunc,	/* tp_dealloc */
	0,					/* tp_print */
	PyPREPROC_HEADERS::getattr,		/* tp_getattr */
	0,		/* tp_setattr */
	0,
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,
	0,					/* tp_call */
	0,					/* tp_str */
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

PyObject *PyPREPROC_HEADERS::getattr(PyObject *self, char *name)
{
	return Py_FindMethod(PyPREPROC_HEADERS_methods, self, name);
}


void PyPREPROC_HEADERS::deallocFunc(PyObject *ob)
{
	delete (PyPREPROC_HEADERS *)ob;
}

