/***********************************************************

win2kras.cpp -- module for Windows 200 extensions to RAS

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#ifndef WINVER
#define WINVER 0x500
#endif

#include "windows.h"
#include "ras.h"
#include "raserror.h"

#include "Python.h"
#include "pywintypes.h"

typedef PyObject* (*PFNReturnRasError)(char *fnName, long err);

PFNReturnRasError pfnReturnRasError = NULL;

static PyObject *ReturnRasError(char *fnName, long err = 0)
{
	if (pfnReturnRasError==NULL)
		Py_FatalError("No ras pfn!");
	return (*pfnReturnRasError)(fnName, err);
}

class PyRASEAPUSERIDENTITY : public PyObject
{
public:
	PyRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *);
	~PyRASEAPUSERIDENTITY();

	/* Python support */
	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static PyTypeObject type;
	RASEAPUSERIDENTITY *m_identity;
};

#define PyRASEAPUSERIDENTITY_Check(ob)	((ob)->ob_type == &PyRASEAPUSERIDENTITY::type)

BOOL PyWinObject_AsRASEAPUSERIDENTITY(PyObject *ob, RASEAPUSERIDENTITY **ppRASEAPUSERIDENTITY, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppRASEAPUSERIDENTITY = NULL;
	} else if (!PyRASEAPUSERIDENTITY_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyRASEAPUSERIDENTITY object");
		return FALSE;
	} else {
		*ppRASEAPUSERIDENTITY = ((PyRASEAPUSERIDENTITY *)ob)->m_identity;
	}
	return TRUE;
}

PyObject *PyWinObject_FromRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *pRASEAPUSERIDENTITY)
{
	if (pRASEAPUSERIDENTITY==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PyRASEAPUSERIDENTITY(pRASEAPUSERIDENTITY);
}

PyTypeObject PyRASEAPUSERIDENTITY::type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyRASEAPUSERIDENTITY",
	sizeof(PyRASEAPUSERIDENTITY),
	0,
	PyRASEAPUSERIDENTITY::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyRASEAPUSERIDENTITY::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
	0,		/*tp_getattro*/
	0,		/*tp_setattro*/
	0,	/*tp_as_buffer*/
};


PyRASEAPUSERIDENTITY::PyRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *identity)
{
	ob_type = &type;
	_Py_NewReference(this);
	m_identity = identity;
}

PyRASEAPUSERIDENTITY::~PyRASEAPUSERIDENTITY()
{
	if (m_identity) {
		// kinda-like an assert ;-)
		RasFreeEapUserIdentity(m_identity);
	}
}

PyObject *PyRASEAPUSERIDENTITY::getattr(PyObject *self, char *name)
{
	PyRASEAPUSERIDENTITY *py = (PyRASEAPUSERIDENTITY *)self;
	if (strcmp(name, "szUserName")==0 || strcmp(name, "userName")==0)
		return PyString_FromString( py->m_identity->szUserName);
	else if (strcmp(name, "pbEapInfo")==0 || strcmp(name, "eapInfo")==0)
		return PyBuffer_FromMemory( py->m_identity->pbEapInfo, py->m_identity->dwSizeofEapInfo );
	return PyErr_Format(PyExc_AttributeError, "RASEAPUSERIDENTITY objects have no attribute '%s'", name);
}

/*static*/ void PyRASEAPUSERIDENTITY::deallocFunc(PyObject *ob)
{
	delete (PyRASEAPUSERIDENTITY *)ob;
}


// @pymethod |win2kras|PyRasGetEapUserIdentity|Sets the dial paramaters for the specified entry.
static PyObject *
PyRasGetEapUserIdentity( PyObject *self, PyObject *args )
{
	char *phoneBook, *entry;
	int flags;
	int hwnd = 0;
	if (!PyArg_ParseTuple(args, "zsi|i:GetEapUserIdentity", 
			  &phoneBook, // @pyparm string|phoneBook||string containing the full path of the phone-book (PBK) file. If this parameter is None, the function will use the system phone book.
			  &entry,// @pyparm string|entry||string containing an existing entry name.
			  &flags,  // @pyparm int|flags||Specifies zero or more of the following flags that qualify the authentication process.
						// @flagh Flag|Description 
						// @flag RASEAPF_NonInteractive|Specifies that the authentication protocol should not bring up a graphical user-interface. If this flag is not present, it is okay for the protocol to display a user interface. 
						// @flag RASEAPF_Logon|Specifies that the user data is obtained from Winlogon. 
						// @flag RASEAPF_Preview|Specifies that the user should be prompted for identity information before dialing. 
			  &hwnd))   // @pyparm int|hwnd|0|Handle to the parent window for the UI dialog.

		return NULL;

	// @pyseeapi RasGetEapUserIdentity
	DWORD rc;
	RASEAPUSERIDENTITY *identity;
	Py_BEGIN_ALLOW_THREADS
	rc = RasGetEapUserIdentity(phoneBook, entry, flags, (HWND)hwnd, &identity);
	Py_END_ALLOW_THREADS
	if (rc != 0)
		return ReturnRasError("RasGetEapUserIdentity",rc);
	return PyWinObject_FromRASEAPUSERIDENTITY(identity);
}

/* List of functions exported by this module */
// @module win2kras|A module encapsulating the Windows 2000 extensions to the Remote Access Service (RAS) API.
static struct PyMethodDef win2kras_functions[] = {
	{"GetEapUserIdentity",       PyRasGetEapUserIdentity,  METH_VARARGS}, // @pymeth RasGetEapUserIdentity|Retrieves identity information for the current user. Use this information to call RasDial with a phone-book entry that requires Extensible Authentication Protocol (EAP).
	{NULL,			NULL}
};

int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyInt_FromLong(value);
	if (!okey || !oval) {
		Py_XDECREF(okey);
		Py_XDECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	Py_XDECREF(okey);
	Py_XDECREF(oval);
	return rc;
}

#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, tok)) return rc

static int AddConstants(PyObject *dict)
{
	int rc;
	ADD_CONSTANT(RASEAPF_NonInteractive); // @const win2kras|RASEAPF_NonInteractive|Specifies that the authentication protocol should not bring up a graphical user-interface. If this flag is not present, it is okay for the protocol to display a user interface.
	ADD_CONSTANT(RASEAPF_Logon); // @const win2kras|RASEAPF_Logon|Specifies that the user data is obtained from Winlogon.
	ADD_CONSTANT(RASEAPF_Preview); // @const win2kras|RASEAPF_Preview|Specifies that the user should be prompted for identity information before dialing.
	return 0;
}

extern "C" __declspec(dllexport) void
initwin2kras(void)
{
	PyWinGlobals_Ensure();
	PyObject *dict, *module;
	module = Py_InitModule("win2kras", win2kras_functions);
	dict = PyModule_GetDict(module);
	AddConstants(dict);
#ifdef _DEBUG
	const char *modName = "win32ras_d.pyd";
#else
	const char *modName = "win32ras.pyd";
#endif
	HMODULE hmod = GetModuleHandle(modName);
	if (hmod==NULL) {
		PyErr_SetString(PyExc_RuntimeError, "You must import 'win32ras' before importing this module");
		return;
	}
	FARPROC fp = GetProcAddress(hmod, "ReturnRasError");
	if (fp==NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Could not locate 'ReturnRasError' in 'win32ras'");
		return;
	}
	pfnReturnRasError = (PFNReturnRasError)fp;
}
