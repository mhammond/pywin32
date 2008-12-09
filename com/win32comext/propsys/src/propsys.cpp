// propsys.cpp :
// $Id$

// Interfaces that support the Vista IProperty* interfaces.
// Although propsys.dll existed before Vista, this module didn't - so we
// don't LoadLibrary() stuff that only exist in Vista - sue me/or back-port
// if you care :)

// This source file contains autoduck documentation.
// @doc

// Any python API functions that use 's#' format must use Py_ssize_t for length
#define PY_SSIZE_T_CLEAN

#include "propsys.h"
#include "PythonCOM.h"
#include "PythonCOMRegister.h"

// @pymethod |propsys|PSRegisterPropertySchema|
static PyObject *PyPSRegisterPropertySchema(PyObject *self, PyObject *args)
{
	PyObject *obfname;
	// @pyparm unicode|filename||
	if (!PyArg_ParseTuple(args, "O:PSRegisterPropertySchema", &obfname))
		return NULL;
	WCHAR *sz;
	if (!PyWinObject_AsWCHAR(obfname, &sz, FALSE))
		return FALSE;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = PSRegisterPropertySchema(sz);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeWCHAR(sz);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |propsys|PSUnregisterPropertySchema|
static PyObject *PyPSUnregisterPropertySchema(PyObject *self, PyObject *args)
{
	PyObject *obfname;
	// @pyparm unicode|filename||
	if (!PyArg_ParseTuple(args, "O:PSUnregisterPropertySchema", &obfname))
		return NULL;
	WCHAR *sz;
	if (!PyWinObject_AsWCHAR(obfname, &sz, FALSE))
		return FALSE;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = PSUnregisterPropertySchema(sz);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeWCHAR(sz);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

/* List of module functions */
// @module propsys|A module, encapsulating the Vista Property System interfaces
static struct PyMethodDef propsys_methods[]=
{
	{ "PSRegisterPropertySchema", PyPSRegisterPropertySchema, 1 }, // @pymeth PyPSRegisterPropertySchema|
	{ "PSUnregisterPropertySchema", PyPSUnregisterPropertySchema, 1 }, // @pymeth PyPSRegisterPropertySchema|
	{ NULL, NULL },
};


//static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] =
//{
//};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(propsys)
{
	PYWIN_MODULE_INIT_PREPARE(propsys, propsys_methods,
	                          "A module, encapsulating the Vista Property System interfaces");

	if (PyDict_SetItemString(dict, "error", PyWinExc_COMError) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;

	// Register all of our interfaces, gateways and IIDs.
	//PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo));

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
