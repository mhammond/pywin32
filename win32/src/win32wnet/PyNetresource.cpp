/******************************************************************
* Copyright (c) 1998-1999 Cisco Systems, Inc. All Rights Reserved
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted,
* provided that the above copyright notice appear in all copies and that
* both that copyright notice and this permission notice appear in
* supporting documentation.
*
*
* CISCO SYSTEMS, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL CISCO SYSTEMS BE LIABLE FOR ANY LOST REVENUE, 
* PROFIT OR DATA, OR FOR SPECIAL, INDIRECT, CONSEQUENTIAL OR INCIDENTAL
* DAMAGES OR ANY OTHER DAMAGES WHATSOEVER, HOWEVER CAUSED AND REGARDLESS
* OF THE THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************/
// @doc

#if	defined(_WIN32_WCE_) // defined by the Windows CE compiler environment

#	ifndef UNICODE
#	define UNICODE
#	endif

#	ifndef _UNICODE
#	define _UNICODE
#	endif

#endif

#include <windows.h>
#include "Python.h"
#include "PyWinTypes.h"
#include "netres.h"			// C++ header file for NETRESOURCE object


/* Main PYTHON entry point for creating a new reference.  Registered by win32wnet module */

// @pymethod <o NETRESOURCE>|win32wnet|NETRESOURCE|Creates a new <o NETRESOURCE> object.

PyObject *PyWinMethod_NewNETRESOURCE(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":NETRESOURCE"))	// no arguments
		return NULL;
	return new PyNETRESOURCE();	// call the C++ constructor
}


BOOL PyWinObject_AsNETRESOURCE(PyObject *ob, NETRESOURCE **ppNetresource, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) // Py_None has a direct value in C ? (from M. Hammond's code)
	{
		*ppNetresource = NULL;
	} 
	else if (!PyNETRESOURCE_Check(ob)) 
	{
		PyErr_SetString(PyExc_TypeError, "The object is not a PyNETRESOURCE object");
		return FALSE;
	} 
	else 
	{
		*ppNetresource = ((PyNETRESOURCE *)ob)->GetNetresource();
	}
	return TRUE;
}


PyObject *PyWinObject_FromNETRESOURCE(const NETRESOURCE *pNetresource)
{
	if (pNetresource==NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *ret = new PyNETRESOURCE(pNetresource);
	if(ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "Allocating pNetresource");
	return ret;
}


// @object NETRESOURCE|A Python object that encapsulates a Win32 NETRESOURCE structure.
__declspec(dllexport)
PyTypeObject PyNETRESOURCEType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyNETRESOURCE",
	sizeof(PyNETRESOURCE),
	0,
	PyNETRESOURCE::deallocFunc,			/* tp_dealloc */
	0,									/* tp_print */
	PyNETRESOURCE::getattr,				/* tp_getattr */
	PyNETRESOURCE::setattr,				/* tp_setattr */
	PyNETRESOURCE::compareFunc,			/* tp_compare */
	0,										/* tp_repr */
	0,										/* tp_as_number */
	0,										/* tp_as_sequence */
	0,										/* tp_as_mapping */
	0,										/* hash? */
	0,										/* tp_call */
	0,										/* tp_str */
};


#define OFF(e) offsetof(PyNETRESOURCE, e)

struct memberlist PyNETRESOURCE::memberlist[] =
{
	{"dwScope",		T_LONG,	OFF(m_nr.dwScope),	0}, // @prop integer|dwScope|
	{"dwType",		T_LONG,	OFF(m_nr.dwType),	0}, // @prop integer|dwType|
	{"dwDisplayType", T_LONG,OFF(m_nr.dwDisplayType),	0}, // @prop integer|dwDisplayType|
	{"dwUsage",		T_LONG,	OFF(m_nr.dwUsage),	0}, // @prop integer|dwUsage|
	{"lpLocalName",	T_STRING, OFF(m_nr.lpLocalName),	0}, // @prop string|localName|
	{"lpRemoteName",T_STRING, OFF(m_nr.lpRemoteName),	0},// @prop string|remoteName|
	{"lpComment",	T_STRING, OFF(m_nr.lpComment),	0},// @prop string|comment|
	{"lpProvider",	T_STRING, OFF(m_nr.lpProvider),	0},// @prop string|provider|
	{NULL}
};

PyNETRESOURCE::PyNETRESOURCE(void)
{
	ob_type = &PyNETRESOURCEType;
	_Py_NewReference(this);
	memset(&m_nr, 0, sizeof(m_nr));
	m_nr.lpLocalName = szLName;
	m_nr.lpRemoteName = szRName;
	m_nr.lpProvider = szProv;
	m_nr.lpComment = szComment;
	szLName[0] = _T('\0');
	szRName[0] = _T('\0');
	szProv[0] = _T('\0');
	szComment[0] = _T('\0');
}

PyNETRESOURCE::PyNETRESOURCE(const NETRESOURCE *pO)
{
	ob_type = &PyNETRESOURCEType;
	_Py_NewReference(this);
	m_nr.dwScope = pO->dwScope;
	m_nr.dwType = pO->dwType;
	m_nr.dwDisplayType = pO->dwDisplayType;
	m_nr.dwUsage = pO->dwUsage;
	m_nr.lpLocalName = szLName;
	m_nr.lpRemoteName = szRName;
	m_nr.lpProvider = szProv;
	m_nr.lpComment = szComment;

	if (pO->lpLocalName == NULL)
		szLName[0] = _T('\0');
	else
	{
		_tcsncpy(szLName, pO->lpLocalName, MAX_NAME);
		szLName[MAX_NAME-1] = _T('\0');		// explicit termination!
	}


	if (pO->lpRemoteName == NULL)
		szRName[0] = _T('\0');
	else
	{
		_tcsncpy(szRName, pO->lpRemoteName, MAX_NAME);
		szRName[MAX_NAME-1] = _T('\0');		// explicit termination!
	}


	if (pO->lpProvider == NULL)
		szProv[0] = _T('\0');
	else
	{
		_tcsncpy(szProv, pO->lpProvider, MAX_NAME);
		szProv[MAX_NAME-1] = _T('\0');		// explicit termination!
	}

	if (pO->lpComment == NULL)
		szComment[0] = _T('\0');
	else
	{
		_tcsncpy(szComment, pO->lpComment, MAX_COMMENT);
		szComment[MAX_COMMENT-1] = _T('\0');
	}

}

PyNETRESOURCE::~PyNETRESOURCE(void)
{

}


PyObject *PyNETRESOURCE::getattr(PyObject *self, char *name)
{
#ifdef UNICODE
	PyNETRESOURCE *This = (PyNETRESOURCE *)self;

	if (strcmp(name, "lpProvider") == 0)
		return PyWinObject_FromWCHAR(This->m_nr.lpProvider);
	else if
		(strcmp(name, "lpRemoteName") == 0)
		return PyWinObject_FromWCHAR(This->m_nr.lpRemoteName);
	else if
		(strcmp(name, "lpLocalName") == 0)
		return PyWinObject_FromWCHAR(This->m_nr.lpLocalName);
	else if
		(strcmp(name, "lpComment") == 0)
		return PyWinObject_FromWCHAR(This->m_nr.lpComment);
	else
#endif
	return PyMember_Get((char *)self, memberlist, name);
}



int PyNETRESOURCE::setattr(PyObject *self, char *name, PyObject *v)
{
	PyNETRESOURCE *This = (PyNETRESOURCE *)self;

	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete NETRESOURCE attributes");
		return -1;
	}

// the following specific string attributes can be set, all others generate an error.

	if (PyString_Check(v))
	{
		int ret;
		TCHAR *value;
		if (!PyWinObject_AsTCHAR(v, &value, FALSE))
			return -1;
		if (strcmp (name, "lpProvider") == 0)
		{
			_tcsncpy(This->szProv, value, MAX_NAME);	// no overflow allowed!
			This->szProv[MAX_NAME-1] = _T('\0');				// make sure NULL terminated!
			ret = 0;
		}
		else
		if (strcmp(name, "lpRemoteName") == 0)
		{
			_tcsncpy(This->szRName, value, MAX_NAME);
			This->szRName[MAX_NAME-1] = _T('\0');
			ret = 0;
		}
		else
		if (strcmp(name, "lpLocalName") == 0)
		{
			_tcsncpy(This->szLName, value, MAX_NAME);
			This->szLName[MAX_NAME-1] = _T('\0');
			ret = 0;
		}
		else
		if (strcmp(name, "lpComment") == 0)
		{
			_tcsncpy(This->szComment, value, MAX_COMMENT);
			This->szComment[MAX_COMMENT-1] = _T('\0');
			ret = 0;
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "The attribute is not a PyNETRESOURCE string");
			ret = -1;
		}
		PyWinObject_FreeTCHAR(value);
		return ret;
	} // PyString_Check

	return PyMember_Set((char *)self, memberlist, name, v);
}



void PyNETRESOURCE::deallocFunc(PyObject *ob)
{
	delete (PyNETRESOURCE *)ob;
}

int PyNETRESOURCE::compare(PyObject *ob)  // only returns 0 or 1  (1 means equal)
{
	NETRESOURCE * p_nr;

	if (!PyWinObject_AsNETRESOURCE(ob, &p_nr, FALSE))	// sets error exception
		return NULL;
	// do integer tests first
	if ((m_nr.dwType != p_nr->dwType) ||
		(m_nr.dwScope != p_nr->dwScope) ||
		(m_nr.dwUsage != p_nr->dwUsage) ||
		(m_nr.dwDisplayType != p_nr->dwDisplayType))
		return (0);
		
	if ((_tcscmp(szComment, GetComment())) ||
		(_tcscmp(szLName, GetLName())) ||
		(_tcscmp(szProv, GetProvider())) ||
		(_tcscmp(szRName, GetRName())))
		return 0;

	return 1;
};

int PyNETRESOURCE::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyNETRESOURCE *)ob1)->compare(ob2);
}
