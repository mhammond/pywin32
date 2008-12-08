// AXscript.cpp :
// $Id$

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "stdafx.h"
#include "PythonCOMRegister.h" // For simpler registration of IIDs etc.
#include "PyGActiveScriptError.h"
#include "PyIActiveScriptError.h"
#include "PyIObjectSafety.h"
#include "PyIProvideMultipleClassInfo.h"
#include "PyIActiveScriptParseProcedure.h"

// Some Dispatch helpers.  Very similar to the standard ones
// provided with PyGatewayBase, except we have special handling
// for EXCEPINFO
static PyObject *do_dispatch(
	PyObject *pPyObject,
	const char *szMethodName,
	const char *szFormat,
	va_list va
	)
{
	// Build the Invoke arguments...
	PyObject *args;
	if ( szFormat )
		args = Py_VaBuildValue((char *)szFormat, va);
	else
		args = PyTuple_New(0);
	if ( !args )
		return NULL;

	// make sure a tuple.
	if ( !PyTuple_Check(args) )
    {
		PyObject *a = PyTuple_New(1);
		if ( a == NULL )
		{
			Py_DECREF(args);
			return NULL;
		}
		PyTuple_SET_ITEM(a, 0, args);
		args = a;
    }

	PyObject *method = PyObject_GetAttrString(pPyObject, "_InvokeEx_");
	if ( !method )
    {
		PyErr_SetString(PyExc_AttributeError, (char *)szMethodName);
		return NULL;
    }

	// Make the call to _Invoke_
	PyObject *result = PyObject_CallFunction(method,
											 "siiOOO",
											 szMethodName,
											 0,
											 DISPATCH_METHOD,
											 args, Py_None, Py_None);
	Py_DECREF(method);
	Py_DECREF(args);
	return result;
}

HRESULT InvokeGatewayViaPolicy(
    PyGatewayBase *pGateway,
	const char *szMethodName,
	EXCEPINFO *pei,
	PyObject **ppResult /* = NULL */,
	const char *szFormat /* = NULL */,
	...
	)
{
	va_list va;

	if ( pGateway->m_pPyObject == NULL || szMethodName == NULL )
		return E_POINTER;

	va_start(va, szFormat);
	PyObject *result = do_dispatch(pGateway->m_pPyObject, szMethodName, szFormat, va);
	va_end(va);


	HRESULT hr = S_OK;
	if (PyErr_Occurred()) {
		PyCom_ExcepInfoFromPyException(pei);
		hr = DISP_E_EXCEPTION;
	}


	if ( ppResult )
		*ppResult = result;
	else
		Py_XDECREF(result);

	return hr;
}

/* List of module functions */
// @module axscript|A module, encapsulating the ActiveX Scripting interfaces
static struct PyMethodDef axcom_methods[]=
{
	{ NULL, NULL }
};

static int AddConstant(PyObject *dict, const char *key, long value)
{
	PyObject *oval = PyInt_FromLong(value);
	if (!oval)
	{
		return 1;
	}
	int rc = PyDict_SetItemString(dict, (char*)key, oval);
	Py_DECREF(oval);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)

static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] =
{
	PYCOM_INTERFACE_FULL       (ActiveScriptSite),
	PYCOM_INTERFACE_FULL       (ActiveScript),
	PYCOM_INTERFACE_FULL       (ActiveScriptParse),
	PYCOM_INTERFACE_FULL       (ActiveScriptError),
	PYCOM_INTERFACE_FULL       (ActiveScriptParseProcedure),
	PYCOM_INTERFACE_FULL       (ObjectSafety),

	PYCOM_INTERFACE_CLIENT_ONLY(ProvideMultipleClassInfo),

	PYCOM_INTERFACE_CATID_ONLY (ActiveScript),
	PYCOM_INTERFACE_CATID_ONLY (ActiveScriptParse),
};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(axscript)
{
	PYWIN_MODULE_INIT_PREPARE(axscript, axcom_methods,
	                          "A module, encapsulating the ActiveX Scripting interfaces.");

	// Register all of our interfaces, gateways and IIDs.
	PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo));

	// Add some symbolic constants to the module   
	ADD_CONSTANT(SCRIPTITEM_ISVISIBLE);
	ADD_CONSTANT(SCRIPTITEM_ISSOURCE);
	ADD_CONSTANT(SCRIPTITEM_GLOBALMEMBERS);
	ADD_CONSTANT(SCRIPTITEM_ISPERSISTENT);
	ADD_CONSTANT(SCRIPTITEM_CODEONLY);
	ADD_CONSTANT(SCRIPTITEM_NOCODE);
	ADD_CONSTANT(SCRIPTITEM_ALL_FLAGS);
	ADD_CONSTANT(SCRIPTTYPELIB_ISCONTROL);
	ADD_CONSTANT(SCRIPTTYPELIB_ISPERSISTENT);
//	ADD_CONSTANT(SCRIPTTYPELIB_ALL_FLAGS);
	ADD_CONSTANT(SCRIPTTEXT_ISVISIBLE);
	ADD_CONSTANT(SCRIPTTEXT_ISEXPRESSION);
	ADD_CONSTANT(SCRIPTTEXT_ISPERSISTENT);
	ADD_CONSTANT(SCRIPTTEXT_ALL_FLAGS);
	ADD_CONSTANT(SCRIPTINFO_IUNKNOWN);
	ADD_CONSTANT(SCRIPTINFO_ITYPEINFO);
	ADD_CONSTANT(SCRIPTINFO_ALL_FLAGS);
	ADD_CONSTANT(SCRIPTINTERRUPT_DEBUG);
	ADD_CONSTANT(SCRIPTINTERRUPT_RAISEEXCEPTION);
	ADD_CONSTANT(SCRIPTINTERRUPT_ALL_FLAGS);

	ADD_CONSTANT(SCRIPTPROC_HOSTMANAGESSOURCE);
	ADD_CONSTANT(SCRIPTPROC_IMPLICIT_THIS);
	ADD_CONSTANT(SCRIPTPROC_IMPLICIT_PARENTS);
	ADD_CONSTANT(SCRIPTPROC_ALL_FLAGS);


	ADD_CONSTANT(SCRIPTTHREADSTATE_NOTINSCRIPT);
	ADD_CONSTANT(SCRIPTTHREADSTATE_RUNNING);

	ADD_CONSTANT(SCRIPTSTATE_UNINITIALIZED);
	ADD_CONSTANT(SCRIPTSTATE_INITIALIZED);
	ADD_CONSTANT(SCRIPTSTATE_STARTED);
	ADD_CONSTANT(SCRIPTSTATE_CONNECTED);
	ADD_CONSTANT(SCRIPTSTATE_DISCONNECTED);
	ADD_CONSTANT(SCRIPTSTATE_CLOSED);
#ifndef SCRIPT_E_REPORTED
#define SCRIPT_E_REPORTED	0x80020101L
#endif
	ADD_CONSTANT(SCRIPT_E_REPORTED);

	// Multiinfo stuff
	ADD_CONSTANT(MULTICLASSINFO_GETTYPEINFO);
	ADD_CONSTANT(MULTICLASSINFO_GETNUMRESERVEDDISPIDS);
	ADD_CONSTANT(MULTICLASSINFO_GETIIDPRIMARY);
	ADD_CONSTANT(MULTICLASSINFO_GETIIDSOURCE);
	ADD_CONSTANT(TIFLAGS_EXTENDDISPATCHONLY);

	ADD_CONSTANT(INTERFACESAFE_FOR_UNTRUSTED_CALLER);
	ADD_CONSTANT(INTERFACESAFE_FOR_UNTRUSTED_DATA);
#ifdef INTERFACE_USES_DISPEX
	// ie 4 SDK has these!
	ADD_CONSTANT(INTERFACE_USES_DISPEX);	// Object knows to use IDispatchEx
	ADD_CONSTANT(INTERFACE_USES_SECURITY_MANAGER);	// Object knows to use IInternetHostSecurityManager
#endif

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
