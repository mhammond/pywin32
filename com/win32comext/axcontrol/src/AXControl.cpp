// AXControl.cpp :
// $Id$

// Interfaces that support the ActiveX Control interfaces.
// First interfaces (and inspiration to actually create this module)
// by Ryan Hughes 

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "axcontrol_pch.h"
#include "stddef.h" // for offsetof
#include "olectl.h"
#include "PythonCOMRegister.h" // For simpler registration of IIDs etc.

#include "PyIOleClientSite.h"
#include "PyIOleObject.h"
#include "PyIOleWindow.h"
#include "PyIOleInPlaceObject.h"
#include "PyIViewObject.h"
#include "PyIViewObject2.h"
#include "PyIOleControl.h"
#include "PyIOleInPlaceSite.h"
#include "PyIOleInPlaceSiteEx.h"
#include "PyIOleInPlaceSiteWindowless.h"
#include "PyISpecifyPropertyPages.h"
#include "PyIObjectWithSite.h"
#include "PyIOleCommandTarget.h"

BOOL PyObject_AsLOGPALETTE(PyObject *pbLogPal, LOGPALETTE **ppLogPal)
{
	*ppLogPal = NULL;
	PyErr_SetString(PyExc_ValueError, "LOGPALETTE is not yet supported!");
	return FALSE;
}

void PyObject_FreeLOGPALETTE(LOGPALETTE *pLogPal)
{
	if (pLogPal) free(pLogPal);
}

PyObject *PyObject_FromLOGPALETTE( LOGPALETTE *pLP)
{
	PyObject *entries = PyTuple_New(pLP->palNumEntries);
	for (int i=0;i<pLP->palNumEntries;i++) {
		PyTuple_SET_ITEM(entries, i, Py_BuildValue("bbbb", 
			pLP->palPalEntry[i].peRed,
			pLP->palPalEntry[i].peGreen,
			pLP->palPalEntry[i].peBlue,
			pLP->palPalEntry[i].peFlags));
	}
	PyObject *rc = Py_BuildValue("lO", pLP->palVersion, entries);
	Py_DECREF(entries);
	return rc;
}

PyObject *PyObject_FromDVTARGETDEVICE( DVTARGETDEVICE *pTD)
{
#define GET_WCHAR_FROM_OFFSET(off) PyWinObject_FromOLECHAR( pTD->off==0 ? NULL : (OLECHAR *) (((BYTE *)pTD) + pTD->off) )
	PyObject *obDriverName = GET_WCHAR_FROM_OFFSET(tdDriverNameOffset);
	PyObject *obDeviceName = GET_WCHAR_FROM_OFFSET(tdDeviceNameOffset);
	PyObject *obPortName = GET_WCHAR_FROM_OFFSET(tdPortNameOffset);
    PyObject *obExtDevmodeOffset = GET_WCHAR_FROM_OFFSET(tdExtDevmodeOffset);
	PyObject *rc = Py_BuildValue("OOOO", obDriverName, obDeviceName, obPortName, obExtDevmodeOffset);
	Py_XDECREF(obDriverName);
	Py_XDECREF(obDeviceName);
	Py_XDECREF(obPortName);
	Py_XDECREF(obExtDevmodeOffset);
	return rc;
}

BOOL PyObject_AsDVTARGETDEVICE( PyObject *ob, DVTARGETDEVICE **ppTD)
{
	BSTR bstrDriverName, bstrDeviceName, bstrPortName, bstrExtDevmodeOffset;
	PyObject *obDriverName, *obDeviceName, *obPortName, *obExtDevmodeOffset;
	int cchDriverName, cchDeviceName, cchPortName, cchExtDevmodeOffset;
	int cch;
	int cb;
	BYTE *pBase;
	BYTE *pCur;

	DVTARGETDEVICE *pTD = *ppTD;
	BOOL ok = FALSE;
	if (!PyArg_ParseTuple(ob, "OOOO", &obDriverName, &obDeviceName, &obPortName, &obExtDevmodeOffset))
		return NULL;
	if (!PyWinObject_AsBstr(obDriverName, &bstrDriverName))
		goto done;
	if (!PyWinObject_AsBstr(obDeviceName, &bstrDeviceName))
		goto done;
	if (!PyWinObject_AsBstr(obPortName, &bstrPortName))
		goto done;
	if (!PyWinObject_AsBstr(obExtDevmodeOffset, &bstrExtDevmodeOffset))
		goto done;
	cchDriverName = bstrDriverName ? SysStringLen(bstrDriverName) + 1 : 0;
	cchDeviceName = bstrDeviceName ? SysStringLen(bstrDeviceName) + 1 : 0;
	cchPortName = bstrPortName ? SysStringLen(bstrPortName) + 1 : 0;
	cchExtDevmodeOffset = bstrExtDevmodeOffset ? SysStringLen(bstrExtDevmodeOffset) + 1 : 0;
	cch = cchDriverName + cchDeviceName + cchPortName + cchExtDevmodeOffset;
	cb = sizeof(DVTARGETDEVICE) + (cch * sizeof(WCHAR));
	*ppTD = (DVTARGETDEVICE *)malloc(cb);
	if (pTD==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating DVTARGETDEVICE");
		goto done;
	}
	pTD->tdSize = cb;
	pBase = (BYTE *)(pTD);
	pCur = pBase + offsetof(DVTARGETDEVICE, tdData);

#define COPY_BSTR(bstr, cch, off) \
	if (bstr==NULL) \
		pTD->off = 0; \
	else { \
		pTD->off = (pCur-pBase); \
		wcscpy( (WCHAR *)pCur, bstr); \
		pCur += ( cch * sizeof(WCHAR)); \
	}
	COPY_BSTR(bstrDriverName, cchDriverName, tdDriverNameOffset);
	COPY_BSTR(bstrDeviceName, cchDeviceName, tdDeviceNameOffset);
	COPY_BSTR(bstrPortName, cchPortName, tdPortNameOffset);
	COPY_BSTR(bstrExtDevmodeOffset, cchExtDevmodeOffset, tdExtDevmodeOffset);

	ok = TRUE;
done:
	SysFreeString(bstrDriverName);
	SysFreeString(bstrDeviceName);
	SysFreeString(bstrPortName);
	SysFreeString(bstrExtDevmodeOffset);

	return ok;
}

void PyObject_FreeDVTARGETDEVICE(DVTARGETDEVICE *pTD)
{
	free(pTD);
}

//////////////////////////////////////////////////////////////
//
// The methods
//

// @pymethod <o PyIOleObject>|axcontrol|OleCreate|Creates a new embedded object identified by a CLSID.
static PyObject *axcontrol_OleCreate(PyObject *self, PyObject *args)
{
	IUnknown *pResult;
	PyObject *obCLSID;
	// @pyparm IID|clsid||A CLSID in string or native format
	PyObject *obIID;
	// @pyparm IID|clsid||A IID in string or native format
	PyObject *obFormatEtc;
	DWORD renderopt=0;
	PyObject *obOleClientSite;
	PyObject *obStorage;

	if (!PyArg_ParseTuple(args, "OOiOOO:OleCreate",
		               &obCLSID, // @pyparm <o PyIID>|obCLSID||The <o PyIID> CLSID for the OLE object to create.
		               &obIID, // @pyparm <o PyIID>|obIID||The <o PyIID> for the interface to return.
					   &renderopt, // @pyparm <o DWORD>|renderopt||The <o DWORD> renderopt for redering the Display.
		               &obFormatEtc, // @pyparm <o FORMATETC>|obFormatEtc||The <o FORMATETC> structure.
		               &obOleClientSite, // @pyparm <o PyIOleClientSite>|obOleClientSite||The <o PyIOleClientSite> interface to the container.
		               &obStorage)) // @pyparm <o PyIStorage>|obStorage||The <o PyIStorage> interface.
		return NULL;

	CLSID clsid;
	//REFCLSID rclsid = &clsid;
	if (!PyWinObject_AsIID(obCLSID, &clsid))
		return NULL;
	
	IID iid;
	//REFIID riid = &iid;
	if (!PyWinObject_AsIID(obIID, &iid))
		return NULL;
	
	IOleClientSite *pIOleClientSite;
	if (!PyCom_InterfaceFromPyObject(obOleClientSite, IID_IOleClientSite, (void **)&pIOleClientSite, FALSE))
		return NULL;

	IStorage *pIStorage;
	if (!PyCom_InterfaceFromPyObject(obStorage, IID_IStorage, (void **)&pIStorage, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = OleCreate(clsid, iid, renderopt, NULL, pIOleClientSite, pIStorage, (LPVOID*) &pResult);
	pIOleClientSite->Release();
	pIStorage->Release();
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) return OleSetOleError(hr);
	return PyCom_PyObjectFromIUnknown(pResult, iid, FALSE);
}

// @pymethod <o PyIUnknown>|axcontrol|OleLoadPicture|Creates a new picture object and initializes it from the contents of a stream.
static PyObject *axcontrol_OleLoadPicture(PyObject *, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *obStream, *obIIDAPI, *obIIDRet = NULL;
	LONG size;
	BOOL runMode;
	if (!PyArg_ParseTuple(args, "OiiO|O", 
		&obStream, // @pyparm <o PyIStream>|stream||The stream that contains picture's data.
		&size,  // @pyparm int|size||Number of bytes read from the stream
		&runMode, // @pyparm int|runMode||The opposite of the initial value of the KeepOriginalFormat property. If TRUE, KeepOriginalFormat is set to FALSE and vice-versa.
		&obIIDAPI, // @pyparm <o PyIID>||iid|The identifier of the interface describing the type of interface pointer to return
		&obIIDRet)) {// @pyparm <o PyIID>||iidRet|The IID to use for the return object - use only if pythoncom does not support the native interface requested.
		return NULL;
	}

	IUnknown *pUnk = NULL;
	IStream* pStream = NULL;
	IID iidAPI, iidRet;
	HRESULT hr;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obStream, IID_IStream, (void **)&pStream, FALSE))
		goto done;

	if (!PyWinObject_AsIID(obIIDAPI, &iidAPI))
		goto done;
	if (obIIDRet == NULL)
		iidRet = iidAPI;
	else {
		if (!PyWinObject_AsIID(obIIDRet, &iidRet))
			goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	hr = ::OleLoadPicture(pStream, size, runMode, iidAPI, (LPVOID*)&pUnk);
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	ret = PyCom_PyObjectFromIUnknown(pUnk, iidRet, FALSE);
done:
	if (pStream)
		pStream->Release();
	return ret;
}

// @pymethod <o PyIUnknown>|axcontrol|OleLoadPicturePath|Creates a new picture object and initializes it from the contents of a stream.
static PyObject *axcontrol_OleLoadPicturePath(PyObject *, PyObject *args)
{
	PyObject *ret = NULL;
	WCHAR *szPath = NULL;
	PyObject *obPath, *obUnk, *obIIDAPI, *obIIDRet = NULL;
	int reserved, clr;
	if (!PyArg_ParseTuple(args, "OOiiO|O", 
		&obPath, // @pyparm string/unicode|url_or_path||The path or url to the file you want to open.
		&obUnk,  // @pyparm <o PyIUknown>|unk||The IUnknown for COM aggregation.
		&reserved, // @pyparm int|reserved||reserved
		&clr, // @pyparm int|clr||The color you want to reserve to be transparent. 
		&obIIDAPI, // @pyparm <o PyIID>||iid|The identifier of the interface describing the type of interface pointer to return
		&obIIDRet)) {// @pyparm <o PyIID>||iidRet|The IID to use for the return object - use only if pythoncom does not support the native interface requested.
		return NULL;
	}

	IUnknown *pUnkRet = NULL;
	IUnknown * pUnkIn = NULL;
	IID iidAPI, iidRet;
	HRESULT hr;
	if (!PyWinObject_AsWCHAR(obPath, &szPath, FALSE))
		goto done;

	if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE))
		goto done;

	if (!PyWinObject_AsIID(obIIDAPI, &iidAPI))
		goto done;
	if (obIIDRet == NULL)
		iidRet = iidAPI;
	else {
		if (!PyWinObject_AsIID(obIIDRet, &iidRet))
			goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	hr = ::OleLoadPicturePath(szPath, pUnkIn, (DWORD)reserved, (OLE_COLOR)clr, iidAPI, (LPVOID*)&pUnkRet);
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	ret = PyCom_PyObjectFromIUnknown(pUnkRet, iidRet, FALSE);
done:
	if (pUnkIn)
		pUnkIn->Release();
	if (szPath)
		PyWinObject_FreeWCHAR(szPath);
	return ret;
}



/* List of module functions */
// @module axcontrol|A module, encapsulating the ActiveX Control interfaces
static struct PyMethodDef axcontrol_methods[]=
{
    { "OleCreate",    axcontrol_OleCreate, 1 }, // @pymeth OleCreate|Creates a new embedded object identified by a CLSID.
	{ "OleLoadPicture",      axcontrol_OleLoadPicture, 1 },      // @pymeth OleLoadPicture|Creates a new picture object and initializes it from the contents of a stream.
	{ "OleLoadPicturePath",  axcontrol_OleLoadPicturePath, 1},   // @pymeth OleLoadPicturePath|Creates a new picture object and initializes it from the contents of a stream.
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
	PYCOM_INTERFACE_FULL       (OleControl),
	PYCOM_INTERFACE_FULL       (OleClientSite),
	PYCOM_INTERFACE_FULL       (OleObject),
	PYCOM_INTERFACE_IID_ONLY   (OleLink),
	PYCOM_INTERFACE_FULL       (OleInPlaceObject),
	PYCOM_INTERFACE_FULL       (ViewObject),
	PYCOM_INTERFACE_FULL       (ViewObject2),
	PYCOM_INTERFACE_FULL       (OleInPlaceSite),
	PYCOM_INTERFACE_FULL       (OleInPlaceSiteEx),
	PYCOM_INTERFACE_FULL       (OleInPlaceSiteWindowless),
	PYCOM_INTERFACE_FULL       (SpecifyPropertyPages),
	PYCOM_INTERFACE_FULL       (ObjectWithSite),
	PYCOM_INTERFACE_FULL       (OleCommandTarget),
};

/* Module initialisation */
extern "C" __declspec(dllexport) void initaxcontrol()
{
	char *modName = "axcontrol";
	PyObject *oModule;
	// Create the module and add the functions
	oModule = Py_InitModule(modName, axcontrol_methods);
	if (!oModule) /* Eeek - some serious error! */
		return;
	PyObject *dict = PyModule_GetDict(oModule);
	if (!dict) return; /* Another serious error!*/

	// Register all of our interfaces, gateways and IIDs.
	PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo));

	ADD_CONSTANT(OLECLOSE_SAVEIFDIRTY); // @const axcontrol|OLECLOSE_SAVEIFDIRTY|The object should be saved if it is dirty. 
	ADD_CONSTANT(OLECLOSE_NOSAVE); // @const axcontrol|OLECLOSE_NOSAVE|The object should not be saved, even if it is dirty. This flag is typically used when an object is being deleted. 
	ADD_CONSTANT(OLECLOSE_PROMPTSAVE); // @const axcontrol|OLECLOSE_PROMPTSAVE|If the object is dirty, the <om PyIOleObject.Close> implementation should display a dialog box to let the end user determine whether to save the object. However, if the object is in the running state but its user interface is invisible, the end user should not be prompted, and the close should be handled as if OLECLOSE_SAVEIFDIRTY had been specified. 
}
