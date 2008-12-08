// @doc - This file contains autoduck documentation
# include "PythonCOM.h"
# include "PythonCOMRegister.h"
# include "aclui.h"
# include "PyGSecurityInformation.h"

// @pymethod |authorization|EditSecurity|Creates a security editor dialog
static PyObject *PyEditSecurity(PyObject *self, PyObject *args, PyObject *kwargs)
{
	ISecurityInformation *isi;
	HWND hwnd;
	PyObject *obhwnd, *obisi, *ret=NULL;
	BOOL bsuccess;
	static char *keywords[]={"hwndOwner", "psi", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:EditSecurity", keywords, 
		&obhwnd,	// @pyparm <o PyHANDLE>|hwndOwner||Handle to window that owns dialog, can be None
		&obisi))	// @pyparm <o PyGSecurityInformation>|psi||Class instance that implements the ISecurityInformation interface
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyCom_InterfaceFromPyObject(obisi, IID_ISecurityInformation, (void **)&isi, FALSE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS;
	bsuccess=EditSecurity(hwnd, isi);
	Py_END_ALLOW_THREADS;
	if (!bsuccess)
		ret=PyWin_SetAPIError("EditSecurity");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	isi->Release();
	return ret;
}

static const PyCom_InterfaceSupportInfo interface_info[] =
{
	PYCOM_INTERFACE_SERVER_ONLY (SecurityInformation)
};

// @module win32com.authorization.authorization|Module containing support for authorization COM interfaces
static struct PyMethodDef authorization_methods[]=
{
	{ "EditSecurity", (PyCFunction)PyEditSecurity, METH_VARARGS|METH_KEYWORDS, "Creates a security descriptor editor dialog"}, // @pymeth EditSecurity|Creates a security descriptor editor dialog
	{NULL}
};


PYWIN_MODULE_INIT_FUNC(authorization)
{
	PYWIN_MODULE_INIT_PREPARE(authorization, authorization_methods,
	                          "Module containing support for authorization COM interfaces.");
	PyCom_RegisterExtensionSupport(dict, interface_info, sizeof(interface_info)/sizeof(PyCom_InterfaceSupportInfo));
	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
