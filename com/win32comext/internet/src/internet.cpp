// AXControl.cpp :
// $Id$

// Interfaces that support the Internet COM interfaces

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "internet_pch.h"
#include "stddef.h" // for offsetof
#include "PythonCOMRegister.h" // For simpler registration of IIDs etc.

#include "PyIInternetProtocolRoot.h"
#include "PyIInternetProtocol.h"
#include "PyIInternetProtocolInfo.h"
#include "PyIInternetProtocolSink.h"
#include "PyIInternetPriority.h"
#include "PyIInternetBindInfo.h"


//////////////////////////////////////////////////////////////
//
// PROTOCOLDATA support
//
BOOL PyObject_AsPROTOCOLDATA(PyObject *ob, PROTOCOLDATA *pPD)
{
	if (ob!=Py_None) {
		PyErr_SetString(PyExc_TypeError, "Only None is support for PROTOCOLDATA objects");
		return FALSE;
	}
	pPD->grfFlags = 0;
	pPD->dwState = 0;
	pPD->pData = NULL;
	pPD->cbData = 0;
	return TRUE;
}

PyObject *PyObject_FromPROTOCOLDATA(PROTOCOLDATA *pPD)
{
	return Py_BuildValue("iiz#", pPD->grfFlags, pPD->dwState, pPD->pData, pPD->cbData);
}
//////////////////////////////////////////////////////////////
//
// BINDINFO support
//
BOOL PyObject_AsBINDINFO(PyObject *ob, BINDINFO *pPD)
{
	BOOL ok = FALSE;
	memset(pPD, 0, sizeof(BINDINFO));
	pPD->cbSize = sizeof(BINDINFO);
	PyObject *obExtra = Py_None;
	PyObject *obSTGM = Py_None;
	PyObject *obCustomVerb = Py_None;
	PyObject *obSA = Py_None;
	PyObject *obIID = Py_None;
	PyObject *obUnk = Py_None;
	if (!PyArg_ParseTuple(ob, "|OOllOlllOOOl",
		&obExtra,
		&obSTGM,
		&pPD->grfBindInfoF,
		&pPD->dwBindVerb,
		&obCustomVerb,
		&pPD->dwOptions,
		&pPD->dwOptionsFlags,
		&pPD->dwCodePage,
		&obSA,
		&obIID,
		&obUnk,
		&pPD->dwReserved))
		goto done;
	if (!PyWinObject_AsTaskAllocatedWCHAR(obExtra, &pPD->szExtraInfo, /*bNoneOK=*/TRUE, NULL))
		goto done;
	if (obSTGM != Py_None) {
		PyErr_SetString(PyExc_TypeError, "Sorry - dont support STGMEDIUM yet - must be None");
		goto done;
	}
	if (!PyWinObject_AsTaskAllocatedWCHAR(obCustomVerb, &pPD->szCustomVerb, /*bNoneOK=*/TRUE, NULL))
		goto done;
	SECURITY_ATTRIBUTES *pSA;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obSA, &pSA, TRUE))
		goto done;
	pPD->securityAttributes = *pSA;
	if (obIID != Py_None && !PyWinObject_AsIID(obIID, &pPD->iid))
		goto done;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, pPD->iid, (void **)&pPD->pUnk, TRUE))
		goto done;

	ok = TRUE;
done:
	// todo: cleanup if !ok
	return ok;
}

PyObject *PyObject_FromBINDINFO(BINDINFO *pPD)
{
	BOOL bNewFormat = pPD->cbSize >= offsetof(BINDINFO, dwOptions);
	int tupleSize = bNewFormat ? 12 : 5;
	PyObject *obRet = PyTuple_New(tupleSize);
	PyTuple_SET_ITEM(obRet, 0, PyWinObject_FromWCHAR(pPD->szExtraInfo));
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(obRet, 1, Py_None); // STGMEDUIM not yet supported.
	PyTuple_SET_ITEM(obRet, 2, PyInt_FromLong(pPD->grfBindInfoF)); 
	PyTuple_SET_ITEM(obRet, 3, PyInt_FromLong(pPD->dwBindVerb)); 
	PyTuple_SET_ITEM(obRet, 4, PyWinObject_FromWCHAR(pPD->szCustomVerb)); 
	if (bNewFormat) {
		PyTuple_SET_ITEM(obRet, 5, PyInt_FromLong(pPD->dwOptions)); 
		PyTuple_SET_ITEM(obRet, 6, PyInt_FromLong(pPD->dwOptionsFlags)); 
		PyTuple_SET_ITEM(obRet, 7, PyInt_FromLong(pPD->dwCodePage)); 
		PyTuple_SET_ITEM(obRet, 8, PyWinObject_FromSECURITY_ATTRIBUTES(pPD->securityAttributes));
		PyTuple_SET_ITEM(obRet, 9, PyWinObject_FromIID(pPD->iid));
		PyTuple_SET_ITEM(obRet, 10, PyCom_PyObjectFromIUnknown(pPD->pUnk, pPD->iid, /*bAddRef = */TRUE));
		PyTuple_SET_ITEM(obRet, 11, PyInt_FromLong(pPD->dwReserved));
	}
	return obRet;
}

//////////////////////////////////////////////////////////////
//
// The methods
//



/* List of module functions */
// @module internet|A module, encapsulating the ActiveX Internet interfaces
static struct PyMethodDef internet_methods[]=
{
	{ NULL, NULL },
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
	PYCOM_INTERFACE_FULL       (InternetProtocolRoot),
	PYCOM_INTERFACE_FULL       (InternetProtocol),
	PYCOM_INTERFACE_FULL       (InternetProtocolInfo),
	PYCOM_INTERFACE_FULL       (InternetProtocolSink),
	PYCOM_INTERFACE_FULL       (InternetPriority),
	PYCOM_INTERFACE_FULL       (InternetBindInfo),
};

/* Module initialisation */
extern "C" __declspec(dllexport) void initinternet()
{
	char *modName = "internet";
	PyObject *oModule;
	// Create the module and add the functions
	oModule = Py_InitModule(modName, internet_methods);
	if (!oModule) /* Eeek - some serious error! */
		return;
	PyObject *dict = PyModule_GetDict(oModule);
	if (!dict) return; /* Another serious error!*/

	// Register all of our interfaces, gateways and IIDs.
	PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo));

//	ADD_CONSTANT(); // @const internet||
}
