// directsound.cpp :
// $Id$

// directsound wrapper contributed by Lars Immisch <lars@ibp.de>

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "directsound_pch.h"
#include "stddef.h" // for offsetof
#include "PythonCOMRegister.h" // For simpler registration of IIDs etc.
#include "PyIDirectSound.h"
#include "PyIDirectSoundBuffer.h"
#include "PyIDirectSoundNotify.h"


// @pymethod <o PyIUnknown>|directsound|DirectSoundCreate|Creates and initializes a new object that supports the IDirectSound interface.
static PyObject *directsound_DirectSoundCreate(PyObject *, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *obGUID = NULL, *obUnk = NULL;
	IUnknown *pUnkIn = NULL;
	GUID guid, *pguid = NULL;
	LPDIRECTSOUND ds;
	HRESULT hr;

	if (!PyArg_ParseTuple(args, "|OO:DirectSoundCreate", 
		&obGUID, // @pyparm <o PyIID>||guid|The identifier of the interface describing the type of interface pointer to return
		&obUnk))  // @pyparm <o PyIUknown>|unk||The IUnknown for COM aggregation.
	{
		return NULL;
	}

	if (obUnk)
	{
		if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE))
			goto done;
	}

	if (obGUID && obGUID != Py_None)
	{
		if (!PyWinObject_AsIID(obGUID, &guid))
			goto done;

		pguid = &guid;
	}

	Py_BEGIN_ALLOW_THREADS
	hr = ::DirectSoundCreate(pguid, &ds, pUnkIn);
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	ret = new PyIDirectSound(ds);
done:
	if (pUnkIn)
		pUnkIn->Release();

	return ret;
}

BOOL CALLBACK dsEnumCallback(LPGUID guid, LPCSTR desc, LPCSTR module, LPVOID context)
{
	PyObject *list = (PyObject*)context;
	PyObject *item = PyTuple_New(3);
	PyObject *oguid;

	// abort enumeration if we cannot create a tuple
	if (!item)
	{
		return FALSE;
	}

	if (guid)
	{
		oguid = PyWinObject_FromIID(*guid);
	}
	else
	{
		Py_INCREF(Py_None);
		oguid = Py_None;
	}

	if (PyTuple_SetItem(item, 0, oguid))
		return FALSE;

	if (PyTuple_SetItem(item, 1, desc ? PyString_FromString(desc) : PyString_FromString("")))
		return FALSE;

	if (PyTuple_SetItem(item, 2, module ? PyString_FromString(module) : PyString_FromString("")))
		return FALSE;

	if (PyList_Append(list, item))
		return FALSE;

	return TRUE;

}

// @pymethod <o list|directsound|DirectSoundEnumerate|Enumerates DirectSound drivers installed in the system.
static PyObject *directsound_DirectSoundEnumerate(PyObject *, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DirectSoundEnumerate"))
	{
		return NULL;
	}

	PyObject *list = PyList_New(0);	
	if (!list)
	{
		return NULL;
	}

	HRESULT hr;
	Py_BEGIN_ALLOW_THREADS
	hr = ::DirectSoundEnumerate(dsEnumCallback, list);
	Py_END_ALLOW_THREADS

	if (PyErr_Occurred())
	{
		return NULL;
	}

	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		return NULL;
	}

	return list;
}

/* List of module functions */
// @module directsound|A module, encapsulating the DirectSound interfaces
static struct PyMethodDef directsound_methods[]=
{
    { "DirectSoundCreate",    directsound_DirectSoundCreate, 1 }, // @pymeth DirectSoundCreate|Creates and initializes a new object that supports the IDirectSound interface.
	{ "DirectSoundEnumerate",      directsound_DirectSoundEnumerate, 1 },      // @pymeth DirectSoundEnumerate|The DirectSoundEnumerate function enumerates the DirectSound drivers installed in the system.
//	{ "DirectSoundCaptureCreate",  directsound_DirectSoundCaptureCreate, 1},   // @pymeth DirectSoundCaptureCreate|The DirectSoundCaptureCreate function creates and initializes an object that supports the IDirectSoundCapture interface
//	{ "DirectSoundCaptureEnumerate",  directsound_DirectSoundCaptureEnumerate, 1},   // @pymeth DirectSoundCaptureEnumerate|The DirectSoundCaptureEnumerate function enumerates the DirectSoundCapture objects installed in the system.
	{"DSCAPS",         PyWinMethod_NewDSCAPS, 1 },      // @pymeth DSCAPS|Creates a new <o PyDSCAPS> object.
	{"DSBCAPS",         PyWinMethod_NewDSBCAPS, 1 },      // @pymeth DSBCAPS|Creates a new <o PyDSBCAPS> object.
	{"DSBUFFERDESC",         PyWinMethod_NewDSBUFFERDESC, 1 },      // @pymeth DSBUFFERDESC|Creates a new <o PyDSBUFFERDESC> object.
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
	PYCOM_INTERFACE_CLIENT_ONLY   (DirectSound),
	PYCOM_INTERFACE_CLIENT_ONLY   (DirectSoundBuffer),
	PYCOM_INTERFACE_CLIENT_ONLY   (DirectSoundNotify),
};

/* Module initialisation */
extern "C" __declspec(dllexport) void initdirectsound()
{
	char *modName = "directsound";
	PyObject *oModule;
	// Create the module and add the functions
	oModule = Py_InitModule(modName, directsound_methods);
	if (!oModule) /* Eeek - some serious error! */
		return;
	PyObject *dict = PyModule_GetDict(oModule);
	if (!dict) return; /* Another serious error!*/

	// Register all of our interfaces, gateways and IIDs.
	PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(g_interfaceSupportData[0]));

	ADD_CONSTANT(DSCAPS_PRIMARYMONO);
	ADD_CONSTANT(DSCAPS_PRIMARYSTEREO);
	ADD_CONSTANT(DSCAPS_PRIMARY8BIT);
	ADD_CONSTANT(DSCAPS_PRIMARY16BIT);
	ADD_CONSTANT(DSCAPS_CONTINUOUSRATE);
	ADD_CONSTANT(DSCAPS_EMULDRIVER);
	ADD_CONSTANT(DSCAPS_CERTIFIED);
	ADD_CONSTANT(DSCAPS_SECONDARYMONO);
	ADD_CONSTANT(DSCAPS_SECONDARYSTEREO);
	ADD_CONSTANT(DSCAPS_SECONDARY8BIT);
	ADD_CONSTANT(DSCAPS_SECONDARY16BIT);
	ADD_CONSTANT(DSBPLAY_LOOPING); // @const directsound|DSBPLAY_LOOPING|text. 
    ADD_CONSTANT(DSBSTATUS_PLAYING);
	ADD_CONSTANT(DSBSTATUS_BUFFERLOST);
	ADD_CONSTANT(DSBSTATUS_LOOPING);
	ADD_CONSTANT(DSBLOCK_FROMWRITECURSOR);
	ADD_CONSTANT(DSBLOCK_ENTIREBUFFER);
	ADD_CONSTANT(DSSCL_NORMAL);
	ADD_CONSTANT(DSSCL_PRIORITY);
	ADD_CONSTANT(DSSCL_EXCLUSIVE);
	ADD_CONSTANT(DSSCL_WRITEPRIMARY);
	ADD_CONSTANT(DS3DMODE_NORMAL);
	ADD_CONSTANT(DS3DMODE_HEADRELATIVE);
	ADD_CONSTANT(DS3DMODE_DISABLE);
	ADD_CONSTANT(DSBCAPS_PRIMARYBUFFER);
	ADD_CONSTANT(DSBCAPS_STATIC);
	ADD_CONSTANT(DSBCAPS_LOCHARDWARE);
	ADD_CONSTANT(DSBCAPS_LOCSOFTWARE);
	ADD_CONSTANT(DSBCAPS_CTRL3D);
	ADD_CONSTANT(DSBCAPS_CTRLFREQUENCY);
	ADD_CONSTANT(DSBCAPS_CTRLPAN);
	ADD_CONSTANT(DSBCAPS_CTRLVOLUME);
	ADD_CONSTANT(DSBCAPS_CTRLPOSITIONNOTIFY);
	ADD_CONSTANT(DSBCAPS_CTRLDEFAULT);
	ADD_CONSTANT(DSBCAPS_CTRLALL);
	ADD_CONSTANT(DSBCAPS_STICKYFOCUS);
	ADD_CONSTANT(DSBCAPS_GLOBALFOCUS);
	ADD_CONSTANT(DSBCAPS_GETCURRENTPOSITION2);
	ADD_CONSTANT(DSBCAPS_MUTE3DATMAXDISTANCE);
	ADD_CONSTANT(DSCBCAPS_WAVEMAPPED);
	ADD_CONSTANT(DSSPEAKER_HEADPHONE);
	ADD_CONSTANT(DSSPEAKER_MONO);
	ADD_CONSTANT(DSSPEAKER_QUAD);
	ADD_CONSTANT(DSSPEAKER_STEREO);
	ADD_CONSTANT(DSSPEAKER_SURROUND);
	ADD_CONSTANT(DSSPEAKER_GEOMETRY_MIN);
	ADD_CONSTANT(DSSPEAKER_GEOMETRY_NARROW);
	ADD_CONSTANT(DSSPEAKER_GEOMETRY_WIDE);
	ADD_CONSTANT(DSSPEAKER_GEOMETRY_MAX);
	// real macros - todo if can be bothered
	// ADD_CONSTANT(DSSPEAKER_COMBINED);
	// ADD_CONSTANT(DSSPEAKER_CONFIG);
	// ADD_CONSTANT(DSSPEAKER_GEOMETRY);
	ADD_CONSTANT(DSBFREQUENCY_MIN);
	ADD_CONSTANT(DSBFREQUENCY_MAX);
	ADD_CONSTANT(DSBFREQUENCY_ORIGINAL);
	ADD_CONSTANT(DSBPAN_LEFT);
	ADD_CONSTANT(DSBPAN_CENTER);
	ADD_CONSTANT(DSBPAN_RIGHT);
	ADD_CONSTANT(DSBVOLUME_MIN);
	ADD_CONSTANT(DSBVOLUME_MAX);
	ADD_CONSTANT(DSBSIZE_MIN);
	ADD_CONSTANT(DSBSIZE_MAX);
	ADD_CONSTANT(DSCCAPS_EMULDRIVER);
	ADD_CONSTANT(DSCBLOCK_ENTIREBUFFER);
	ADD_CONSTANT(DSCBSTATUS_CAPTURING);
	ADD_CONSTANT(DSCBSTATUS_LOOPING);
	ADD_CONSTANT(DSCBSTART_LOOPING);
	ADD_CONSTANT(DSBPN_OFFSETSTOP);

	PyDict_SetItemString(dict, "DSCAPSType", (PyObject *)&PyDSCAPSType);
	PyDict_SetItemString(dict, "DSBCAPSType", (PyObject *)&PyDSBCAPSType);
	PyDict_SetItemString(dict, "DSBUFFERDESCType", (PyObject *)&PyDSBUFFERDESCType);
}
