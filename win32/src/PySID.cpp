//
// @doc

#include "windows.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#ifndef MS_WINCE /* This code is not available on Windows CE */

// @pymethod <o PySID>|pywintypes|SID|Creates a new SID object
PyObject *PyWinMethod_NewSID(PyObject *self, PyObject *args)
{
	void *buf = NULL;
	int bufSize = 32;
	// @pyparm int|bufSize|32|Size for the SID buffer
	if (!PyArg_ParseTuple(args, "|i:SID", &bufSize)) {
		PyErr_Clear();
		// @pyparmalt1 string|buffer||A raw data buffer, assumed to hold the SID data.
		if (!PyArg_ParseTuple(args, "s#:SID", &buf, &bufSize)) {
			/* Special case for one step setup of the SID */
			PyErr_Clear();
			// @pyparmalt2 <o SID_IDENTIFIER_AUTHORITY>|idAuthority||The identifier authority.
			// @pyparmalt2 [int, ...]|subAuthorities||A list of sub authorities.
			SID_IDENTIFIER_AUTHORITY sid_ia;
			PyObject *obSubs;
			if (!PyArg_ParseTuple(args, "(bbbbbb)O:Initialize", 
				&sid_ia.Value[0], &sid_ia.Value[1],&sid_ia.Value[2],
				&sid_ia.Value[3],&sid_ia.Value[4],&sid_ia.Value[5],
				&obSubs))
				return NULL;
			long sub0, sub1, sub2, sub3, sub4, sub5, sub6, sub7;
			if (!PySequence_Check(obSubs)) {
				PyErr_SetString(PyExc_TypeError, "sub authorities must be a sequence of integers.");
				return NULL;
			}
			int numSubs = PySequence_Length(obSubs);
			if (numSubs>8) {
				PyErr_SetString(PyExc_TypeError, "sub authorities sequence size must be < 8");
				return NULL;
			}
#define GET_SUB(i) if (i<numSubs) { \
			PyObject *t = PySequence_GetItem(obSubs, i);\
			sub##i = PyInt_AsLong(t);\
			Py_XDECREF(t);\
		}
			GET_SUB(0);
			GET_SUB(1);
			GET_SUB(2);
			GET_SUB(3);
			GET_SUB(4);
			GET_SUB(5);
			GET_SUB(6);
			GET_SUB(7);
			PSID pNew;
			if (!AllocateAndInitializeSid(&sid_ia, numSubs, sub0, sub1, sub2, sub3, sub4, sub5, sub6, sub7, &pNew))
				return PyWin_SetAPIError("AllocateAndInitializeSid");
			return new PySID(pNew, true);
		}
	}
	return new PySID(bufSize, buf);
}

BOOL PyWinObject_AsSID(PyObject *ob, PSID *ppSID, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSID = NULL;
	} else if (!PySID_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PySID object");
		return FALSE;
	} else {
		*ppSID = ((PySID *)ob)->GetSID();
	}
	return TRUE;
}

PyObject *PyWinObject_FromSID(PSID pSID)
{
	if (pSID==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PySID(pSID);
}

// @pymethod |PySID|Initialize|Initialize the SID.
PyObject *PySID::Initialize(PyObject *self, PyObject *args)
{
	PySID *This = (PySID *)self;
	byte cnt;
	// @pyparm <o SID_IDENTIFIER_AUTHORITY>|idAuthority||The identifier authority.
	// @pyparm int|numSubauthorities||The number of sub authorities to allocate.
	SID_IDENTIFIER_AUTHORITY sid_ia;
	if (!PyArg_ParseTuple(args, "(bbbbbb)b:Initialize", 
		&sid_ia.Value[0], &sid_ia.Value[1],&sid_ia.Value[2],
		&sid_ia.Value[3],&sid_ia.Value[4],&sid_ia.Value[5],
		&cnt))
		return NULL;
	if (!InitializeSid(This->GetSID(), &sid_ia, cnt))
		return PyWin_SetAPIError("InitializeSid");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PySID|IsValid|Determines if the SID is valid.
PyObject *PySID::IsValid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsValid"))
		return NULL;
	PySID *This = (PySID *)self;
	return PyInt_FromLong( IsValidSid(This) );
}

// @pymethod |PySID|SetSubAuthority|Sets a SID SubAuthority
// @comm See the function GetSidSubAuthority
PyObject *PySID::SetSubAuthority(PyObject *self, PyObject *args)
{
	PySID *This = (PySID *)self;
	int num;
	long val;
	// @pyparm int|index||The index of the sub authority to set
	// @pyparm int|val||The value for the sub authority
	if (!PyArg_ParseTuple(args, "il", &num, &val))
		return NULL;
	if (num<0 || num>=*GetSidSubAuthorityCount(This->GetSID())) {
		PyErr_SetString(PyExc_ValueError, "The index is out of range");
		return NULL;
	}
	*GetSidSubAuthority(This->GetSID(), num) = val;
	Py_INCREF(Py_None);
	return Py_None;
}

// @object PySID|A Python object, representing a SID structure
static struct PyMethodDef PySID_methods[] = {
	{"Initialize",     PySID::Initialize, 1}, 	// @pymeth Initialize|Initialize the SID.
	{"IsValid",        PySID::IsValid, 1}, 	// @pymeth IsValid|Determines if the SID is valid.
	{"SetSubAuthority",PySID::SetSubAuthority, 1}, 	// @pymeth SetSubAuthority|Sets a SID SubAuthority
	{NULL}
};


PYWINTYPES_EXPORT PyTypeObject PySIDType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySID",
	sizeof(PySID),
	0,
	PySID::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySID::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	// @pymeth __cmp__|Used when objects are compared.
	PySID::compareFunc,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};


PySID::PySID(int bufSize, void *buf /* = NULL */)
{
	ob_type = &PySIDType;
	_Py_NewReference(this);
	m_psid = (PSID)malloc(bufSize);
	if (buf==NULL)
		memset(m_psid, 0, bufSize);
	else
		memcpy(m_psid, buf, bufSize);
	m_bFreeWithFreeSid = false;
}

PySID::PySID(PSID pOther, bool bFreeWithFreeSid /* = false */)
{
	ob_type = &PySIDType;
	_Py_NewReference(this);
	if (!bFreeWithFreeSid) {
		/* Take my own copy */
		DWORD size = GetLengthSid(pOther);
		m_psid = (PSID)malloc(size);
		CopySid(size, m_psid, pOther);
		m_bFreeWithFreeSid = false;
	} else {
		/* Take ownership */
		m_psid = pOther;
		m_bFreeWithFreeSid = true;
	}
}

PySID::~PySID()
{
	if (m_bFreeWithFreeSid)
		FreeSid(m_psid);
	else
		free(m_psid);
}

PyObject *PySID::getattr(PyObject *self, char *name)
{
	return Py_FindMethod(PySID_methods, self, name);
}

int PySID::compare(PyObject *ob)
{
	PSID p1 = NULL, p2 = NULL;
	PyWinObject_AsSID(this, &p1);
	PyWinObject_AsSID(ob, &p2);
	return EqualSid(p1, p2)==FALSE;
}


// @pymethod int|PySID|__cmp__|Used when objects are compared.
// @comm This method calls the Win32 API function EqualSid
int PySID::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PySID *)ob1)->compare(ob2);
}

/*static*/ void PySID::deallocFunc(PyObject *ob)
{
	delete (PySID *)ob;
}

#endif /* MS_WINCE */