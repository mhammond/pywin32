//
// @doc

#include "Python.h"
#ifndef MS_WINCE /* This source is not included for WinCE */
#include "windows.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

// @pymethod <o PyACL>|pywintypes|ACL|Creates a new ACL object
PyObject *PyWinMethod_NewACL(PyObject *self, PyObject *args)
{
	int bufSize = 64;
	// @pyparm int|bufSize|64|The size for the ACL.
	if (!PyArg_ParseTuple(args, "|i:ACL", &bufSize))
		return NULL;
	return new PyACL(bufSize);
}

BOOL PyWinObject_AsACL(PyObject *ob, PACL *ppACL, BOOL bNoneOK /*= FALSE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppACL = NULL;
	} else if (!PyACL_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyACL object");
		return FALSE;
	} else {
		*ppACL = ((PyACL *)ob)->GetACL();
	}
	return TRUE;
}

// @pymethod |PyACL|Initialize|Initialize the ACL.
// @comm It should not be necessary to call this, as the ACL object
// is initialised by Python.  This method gives you a chance to trap
// any errors that may occur.
PyObject *PyACL::Initialize(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	if (!PyArg_ParseTuple(args, ":Initialize"))
		return NULL;
	if (!::InitializeAcl(This->GetACL(), This->bufSize, ACL_REVISION))
		return PyWin_SetAPIError("InitializeAcl");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyACL|AddAccessAllowedAce|Adds an access-allowed ACE to an ACL object. The access is granted to a specified SID.
PyObject *PyACL::AddAccessAllowedAce(PyObject *self, PyObject *args)
{
	DWORD access;
	PyObject *obSID;
	PSID psid;
	PyACL *This = (PyACL *)self;
	// @pyparm int|access||Specifies the mask of access rights to be granted to the specified SID.
	// @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being granted access. 
	if (!PyArg_ParseTuple(args, "lO:AddAccessAllowedAce", &access, &obSID))
		return NULL;
	if (!PyWinObject_AsSID(obSID, &psid, FALSE))
		return NULL;
	if (!::AddAccessAllowedAce(This->GetACL(), ACL_REVISION, access, psid))
		return PyWin_SetAPIError("AddAccessAllowedAce");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyACL|AddAccessDeniedAce|Adds an access-denied ACE to an ACL object. The access is denied to a specified SID.
PyObject *PyACL::AddAccessDeniedAce(PyObject *self, PyObject *args)
{
	DWORD access;
	PyObject *obSID;
	PSID psid;
	PyACL *This = (PyACL *)self;
	// @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
	// @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access. 
	if (!PyArg_ParseTuple(args, "lO:AddAccessDeniedAce", &access, &obSID))
		return NULL;
	if (!PyWinObject_AsSID(obSID, &psid, FALSE))
		return NULL;
	if (!::AddAccessDeniedAce(This->GetACL(), ACL_REVISION, access, psid))
		return PyWin_SetAPIError("AddAccessDeniedAce");
	Py_INCREF(Py_None);
	return Py_None;
}

// @object PyACL|A Python object, representing a ACL structure
static struct PyMethodDef PyACL_methods[] = {
	{"Initialize",     PyACL::Initialize, 1}, 	// @pymeth Initialize|Initialize the ACL.
	{"AddAccessAllowedAce",     PyACL::AddAccessAllowedAce, 1}, 	// @pymeth AddAccessAllowedAce|Adds an access-allowed ACE to an ACL object.
	{"AddAccessDeniedAce",     PyACL::AddAccessDeniedAce, 1}, 	// @pymeth AddAccessDeniedAce|Adds an access-denied ACE to an ACL object.
	{NULL}
};


PYWINTYPES_EXPORT PyTypeObject PyACLType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyACL",
	sizeof(PyACL),
	0,
	PyACL::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyACL::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	0,
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};


PyACL::PyACL(int createBufSize)
{
	ob_type = &PyACLType;
	_Py_NewReference(this);
	bufSize = createBufSize;
	buf = malloc(bufSize);
	memset(buf, 0, bufSize);

	::InitializeAcl(GetACL(), bufSize, ACL_REVISION);
}

PyACL::~PyACL()
{
}

PyObject *PyACL::getattr(PyObject *self, char *name)
{
	return Py_FindMethod(PyACL_methods, self, name);
}

/*static*/ void PyACL::deallocFunc(PyObject *ob)
{
	delete (PyACL *)ob;
}

#endif /* MS_WINCE */