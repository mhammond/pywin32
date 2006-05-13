// @doc - This file contains autoduck documentation

#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"

#ifdef MS_WINCE
#define DM_SPECVERSION 0
#endif

// @object PyDEVMODE|Python object wrapping a DEVMODE structure
struct PyMethodDef PyDEVMODE::methods[] = {
	{"Clear",     PyDEVMODE::Clear, 1}, 	// @pymeth Clear|Resets all members of the structure
	{NULL}
};

#define OFF(e) offsetof(PyDEVMODE, e)
struct PyMemberDef PyDEVMODE::members[] = {
	// DeviceName is a dummy so it will show up in property list, get and set handle manually
	// @prop str|DeviceName|String of at most 32 chars
	{"DeviceName",		T_OBJECT, OFF(obdummy), 0, "String of at most 32 chars"},
	// @prop int|SpecVersion|Should always be set to DM_SPECVERSION
	{"SpecVersion", 	T_USHORT, OFF(devmode.dmSpecVersion), 0, "Should always be set to DM_SPECVERSION"},
	// @prop int|DriverVersion|Version nbr assigned to printer driver by vendor
	{"DriverVersion", 	T_USHORT, OFF(devmode.dmDriverVersion), 0, "Version nbr assigned to printer driver by vendor"},
	// @prop int|Size|Size of structure
	{"Size",	 		T_USHORT, OFF(devmode.dmSize), READONLY, "Size of structure"},
	// @prop int|DriverExtra|Number of extra bytes allocated for driver data, can only be set when new object is created
	{"DriverExtra", 	T_USHORT, OFF(devmode.dmDriverExtra), READONLY, 
		"Number of extra bytes allocated for driver data, can only be set when new object is created"},
	// @prop int|Fields|Bitmask of win32con.DM_* constants indicating which members are set
	{"Fields",			T_ULONG,  OFF(devmode.dmFields), 0, "Bitmask of win32con.DM_* constants indicating which members are set"},
	// @prop int|Orientation|Only applies to printers, DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE
	{"Orientation", 	T_SHORT,  OFF(devmode.dmOrientation), 0, "Only applies to printers, DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE"},
	// @prop int|PaperSize|Use 0 if PaperWidth and PaperLength are set, otherwise win32con.DMPAPER_* constant
	{"PaperSize", 		T_SHORT,  OFF(devmode.dmPaperSize), 0, "Use 0 if PaperWidth and PaperLength are set, otherwise win32con.DMPAPER_* constant"},
	// @prop int|PaperLength|Specified in 1/10 millimeters
	{"PaperLength", 	T_SHORT,  OFF(devmode.dmPaperLength), 0, "Specified in 1/10 millimeters"},
	// @prop int|PaperWidth|Specified in 1/10 millimeters
	{"PaperWidth", 		T_SHORT,  OFF(devmode.dmPaperWidth), 0, "Specified in 1/10 millimeters"},
#ifndef MS_WINCE
	// @prop int|Position_x|Position of display relative to desktop
	{"Position_x", 		T_LONG,   OFF(devmode.dmPosition.x), 0, "Position of display relative to desktop"},
	// @prop int|Position_y|Position of display relative to desktop
	{"Position_y", 		T_LONG,   OFF(devmode.dmPosition.y), 0, "Position of display relative to desktop"},
#endif
	// @prop int|DisplayOrientation|Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270
	{"DisplayOrientation",T_ULONG,OFF(devmode.dmDisplayOrientation), 0, "Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270"},
	// @prop int|DisplayFixedOutput|DMDFO_DEFAULT, DMDFO_CENTER, DMDFO_STRETCH
	{"DisplayFixedOutput",T_ULONG,OFF(devmode.dmDisplayFixedOutput), 0, "DMDFO_DEFAULT, DMDFO_CENTER, DMDFO_STRETCH"}, 
	// @prop int|Scale|Specified as percentage, eg 50 means half size of original
	{"Scale",			T_SHORT,  OFF(devmode.dmScale), 0, "Specified as percentage, eg 50 means half size of original"},
	// @prop int|Copies|Nbr of copies to print
	{"Copies",			T_SHORT,  OFF(devmode.dmCopies), 0, "Nbr of copies to print"},
	// @prop int|DefaultSource|DMBIN_* constant, or can be a printer-specific value
	{"DefaultSource",	T_SHORT,  OFF(devmode.dmDefaultSource), 0, "DMBIN_* constant, or can be a printer-specific value"},
	// @prop int|PrintQuality|DMRES_* constant, interpreted as DPI if positive
	{"PrintQuality",	T_SHORT,  OFF(devmode.dmPrintQuality), 0, "DMRES_* constant, interpreted as DPI if positive"},
	// @prop int|Color|DMCOLOR_COLOR or DMCOLOR_MONOCHROME
	{"Color",			T_SHORT,  OFF(devmode.dmColor), 0, "DMCOLOR_COLOR or DMCOLOR_MONOCHROME"},
	// @prop int|Duplex|For printers that do two-sided printing: DMDUP_SIMPLEX, DMDUP_HORIZONTAL, DMDUP_VERTICAL
	{"Duplex",			T_SHORT,  OFF(devmode.dmDuplex), 0, "For printers that do two-sided printing: DMDUP_SIMPLEX, DMDUP_HORIZONTAL, DMDUP_VERTICAL"},
	// @prop int|YResolution|Vertical printer resolution in DPI - if this is set, PrintQuality indicates horizontal DPI
	{"YResolution", 	T_SHORT,  OFF(devmode.dmYResolution), 0, "Vertical printer resolution in DPI - if this is set, PrintQuality indicates horizontal DPI"},
	// @prop int|TTOption|TrueType options: DMTT_BITMAP, DMTT_DOWNLOAD, DMTT_DOWNLOAD_OUTLINE, DMTT_SUBDEV
	{"TTOption", 		T_SHORT,  OFF(devmode.dmTTOption), 0, "TrueType options: DMTT_BITMAP, DMTT_DOWNLOAD, DMTT_DOWNLOAD_OUTLINE, DMTT_SUBDEV"},
	// @prop int|Collate|DMCOLLATE_TRUE or DMCOLLATE_FALSE
	{"Collate", 		T_SHORT,   OFF(devmode.dmCollate), 0, "DMCOLLATE_TRUE or DMCOLLATE_FALSE"},
	// @prop str|FormName|Name of form as returned by <om win32print.EnumForms>, at most 32 chars
	{"FormName",		T_OBJECT,  OFF(obdummy), 0, "Name of form as returned by EnumForms, at most 32 chars"},  // same semantics as DeviceName
	// @prop int|LogPixels|Pixels per inch (only for display devices
	{"LogPixels", 		T_USHORT,  OFF(devmode.dmLogPixels), 0, "Pixels per inch (only for display devices)"},
	// @prop int|BitsPerPel|Color resolution in bits per pixel
	{"BitsPerPel", 		T_ULONG,   OFF(devmode.dmBitsPerPel), 0, "Color resolution in bits per pixel"},
	// @prop int|PelsWidth|Pixel width of display
	{"PelsWidth", 		T_ULONG,   OFF(devmode.dmPelsWidth), 0, "Pixel width of display"},
	// @prop int|PelsHeight|Pixel height of display
	{"PelsHeight", 		T_ULONG,   OFF(devmode.dmPelsHeight), 0, "Pixel height of display"},
	// @prop int|DisplayFlags|Combination of DM_GRAYSCALE and DM_INTERLACED
	{"DisplayFlags", 	T_ULONG,   OFF(devmode.dmDisplayFlags), 0, "Combination of DM_GRAYSCALE and DM_INTERLACED"},
	// @prop int|DisplayFrequency|Refresh rate
	{"DisplayFrequency",T_ULONG,   OFF(devmode.dmDisplayFrequency), 0, "Refresh rate"},
#ifdef MS_WINCE
	// @prop int|DisplayOrientation|Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270
	{"DisplayOrientation",T_ULONG,OFF(devmode.dmDisplayOrientation), 0, "Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270"},
#else
	// @prop int|ICMMethod|Indicates where ICM is performed, one of win32con.DMICMMETHOD_* values
	{"ICMMethod",		T_ULONG,   OFF(devmode.dmICMMethod), 0, "Indicates where ICM is performed, one of win32con.DMICMMETHOD_* values"},
	// @prop int|ICMIntent|Intent of ICM, one of win32con.DMICM_* values
	{"ICMIntent",		T_ULONG,   OFF(devmode.dmICMIntent), 0, "Intent of ICM, one of win32con.DMICM_* values"},
	// @prop int|MediaType|win32con.DMMEDIA_*, can also be a printer-specific value greater then DMMEDIA_USER
	{"MediaType",		T_ULONG,   OFF(devmode.dmMediaType), 0, "win32con.DMMEDIA_*, can also be a printer-specific value greater then DMMEDIA_USER"},
	// @prop int|DitherType|Dithering option, win32con.DMDITHER_*
	{"DitherType",		T_ULONG,   OFF(devmode.dmDitherType), 0, "Dithering options, win32con.DMDITHER_*"},
	// @prop int|Reserved1|Reserved, use only 0
	{"Reserved1",		T_ULONG,   OFF(devmode.dmReserved1), 0, "Reserved, use only 0"},
	// @prop int|Reserved2|Reserved, use only 0
	{"Reserved2",		T_ULONG,   OFF(devmode.dmReserved2), 0, "Reserved, use only 0"},
	// @prop str|DriverData|Driver data appended to end of structure
	{"DriverData",		T_OBJECT,  OFF(obdummy), 0, "Driver data appended to end of structure"},
#if WINVER >= 0x0500
	// @prop int|Nup|Controls printing of multiple logical pages per physical page, DMNUP_SYSTEM or DMNUP_ONEUP
	{"Nup",				T_ULONG,   OFF(devmode.dmNup), 0, "Controls printing of multiple logical pages per physical page, DMNUP_SYSTEM or DMNUP_ONEUP"},
	// @prop int|PanningWidth|Not used, leave as 0
	{"PanningWidth",	T_ULONG,   OFF(devmode.dmPanningWidth), 0, "Not used, leave as 0"},
	// @prop int|PanningHeight|Not used, leave as 0
	{"PanningHeight",	T_ULONG,   OFF(devmode.dmPanningHeight), 0, "Not used, leave as 0"},
#endif	// WINVER >= 0x0500
#endif	// !MS_WINCE
	{NULL}
};

PYWINTYPES_EXPORT PyTypeObject PyDEVMODEType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyDEVMODE",
	sizeof(PyDEVMODE),
	0,
	PyDEVMODE::deallocFunc,
	0,			// tp_print;
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// tp_as_number
	0,			// tp_as_sequence
	0,			// tp_as_mapping
	0,
	0,						/* tp_call */
	0,		/* tp_str */
	PyDEVMODE::getattro,	// PyObject_GenericGetAttr
	PyDEVMODE::setattro,	// PyObject_GenericSetAttr
	0,			// tp_as_buffer;
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags;
	0,			// tp_doc; /* Documentation string */
	0,			// traverseproc tp_traverse;
	0,			// tp_clear;
	0,			// tp_richcompare;
	0,			// tp_weaklistoffset;
	0,			// tp_iter
	0,			// iternextfunc tp_iternext
	PyDEVMODE::methods,
	PyDEVMODE::members,
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get;
	0,			// tp_descr_set;
	0,			// tp_dictoffset;
	0,			// tp_init;
	0,			// tp_alloc;
	PyDEVMODE::tp_new	// newfunc tp_new;
};

PyDEVMODE::PyDEVMODE(PDEVMODE pdm)
{
	ob_type = &PyDEVMODEType;
	memcpy(&devmode, pdm, pdm->dmSize);
	pdevmode=(PDEVMODE)malloc(pdm->dmSize + pdm->dmDriverExtra);
	if (pdevmode==NULL)
		PyErr_Format(PyExc_MemoryError, "PyDEVMODE::PyDEVMODE - Unable to allocate DEVMODE of size %d",
		pdm->dmSize + pdm->dmDriverExtra);
	else;
		memcpy(pdevmode, pdm, pdm->dmSize + pdm->dmDriverExtra);
	obdummy=NULL;
	_Py_NewReference(this);
}

PyDEVMODE::PyDEVMODE(void)
{
	ob_type = &PyDEVMODEType;
	static WORD dmSize=sizeof(DEVMODE);
	pdevmode=(PDEVMODE)malloc(dmSize);
	ZeroMemory(pdevmode,dmSize);
	pdevmode->dmSize=dmSize;
	pdevmode->dmSpecVersion=DM_SPECVERSION;
	ZeroMemory(&devmode,dmSize);
	devmode.dmSize=dmSize;
	devmode.dmSpecVersion=DM_SPECVERSION;
	obdummy=NULL;
	_Py_NewReference(this);
}

PyDEVMODE::PyDEVMODE(USHORT dmDriverExtra)
{
	ob_type = &PyDEVMODEType;
	static WORD dmSize=sizeof(DEVMODE);
	pdevmode=(PDEVMODE)malloc(dmSize+dmDriverExtra);
	ZeroMemory(pdevmode,dmSize+dmDriverExtra);
	pdevmode->dmSize=dmSize;
	pdevmode->dmSpecVersion=DM_SPECVERSION;
	pdevmode->dmDriverExtra=dmDriverExtra;
	ZeroMemory(&devmode,dmSize);
	devmode.dmSize=dmSize;
	devmode.dmSpecVersion=DM_SPECVERSION;
	devmode.dmDriverExtra=dmDriverExtra;
	obdummy=NULL;
	_Py_NewReference(this);
}

PyDEVMODE::~PyDEVMODE()
{
	if (pdevmode!=NULL)
		free(pdevmode);
}

BOOL PyDEVMODE_Check(PyObject *ob)
{
	if (ob->ob_type!=&PyDEVMODEType){
		PyErr_SetString(PyExc_TypeError,"Object must be a PyDEVMODE");	
		return FALSE;
		}
	return TRUE;
}

void PyDEVMODE::deallocFunc(PyObject *ob)
{
	delete (PyDEVMODE *)ob;
}

PDEVMODE PyDEVMODE::GetDEVMODE(void)
{
	return pdevmode;
}

// @pymethod |PyDEVMODE|Clear|Resets all members of the structure
PyObject *PyDEVMODE::Clear(PyObject *self, PyObject *args)
{
	PDEVMODE pdevmode=((PyDEVMODE *)self)->pdevmode;
	USHORT dmDriverExtra=pdevmode->dmDriverExtra;
	WORD dmSize=pdevmode->dmSize;
	DWORD totalsize=dmSize + dmDriverExtra;
	ZeroMemory(pdevmode, totalsize);
	pdevmode->dmDriverExtra=dmDriverExtra;
	pdevmode->dmSize=dmSize;
	pdevmode->dmSpecVersion=DM_SPECVERSION;

	pdevmode=&((PyDEVMODE *)self)->devmode;
	ZeroMemory(pdevmode, dmSize);
	pdevmode->dmDriverExtra=dmDriverExtra;
	pdevmode->dmSize=dmSize;
	pdevmode->dmSpecVersion=DM_SPECVERSION;
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *PyDEVMODE::getattro(PyObject *self, PyObject *obname)
{
	PDEVMODE pdevmode=((PyDEVMODE *)self)->pdevmode;
	char *name=PyString_AsString(obname);
	if (name==NULL)
		return NULL;
	if (strcmp(name,"DeviceName")==0)
		if (pdevmode->dmDeviceName[CCHDEVICENAME-1]==0)  // in case DeviceName fills space and has no trailing NULL
			return PyString_FromString((char *)&pdevmode->dmDeviceName);
		else
			return PyString_FromStringAndSize((char *)&pdevmode->dmDeviceName, CCHDEVICENAME);

	if (strcmp(name,"FormName")==0)
		if (pdevmode->dmFormName[CCHFORMNAME-1]==0)  // If dmFormName occupies whole 32 chars, trailing NULL not present
			return PyString_FromString((char *)&pdevmode->dmFormName);
		else
			return PyString_FromStringAndSize((char *)&pdevmode->dmFormName, CCHFORMNAME);
	
	if (strcmp(name,"DriverData")==0)
		if (pdevmode->dmDriverExtra==0){  // No extra space allocated
			Py_INCREF(Py_None);
			return Py_None;
			}
		else
			return PyString_FromStringAndSize((char *)((ULONG_PTR)pdevmode + pdevmode->dmSize), pdevmode->dmDriverExtra);

	return PyObject_GenericGetAttr(self,obname);
}

int PyDEVMODE::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
	char *name, *value;
	int valuelen;
	name=PyString_AsString(obname);
	if (name==NULL)
		return -1;
	if (strcmp(name,"DeviceName")==0){
		if (PyString_AsStringAndSize(obvalue, &value, &valuelen)==-1)
			return -1;
		if (valuelen > CCHDEVICENAME){
			PyErr_Format(PyExc_ValueError,"DeviceName must be a string of length %d or less", CCHDEVICENAME);
			return -1;
			}
		PDEVMODE pdevmode=&((PyDEVMODE *)self)->devmode;
		ZeroMemory(&pdevmode->dmDeviceName, CCHDEVICENAME);
		memcpy(&pdevmode->dmDeviceName, value, valuelen);
		// update variable length DEVMODE with same value
		memcpy(((PyDEVMODE *)self)->pdevmode, pdevmode, pdevmode->dmSize);
		return 0;
		}

	if (strcmp(name,"FormName")==0){
		if (PyString_AsStringAndSize(obvalue, &value, &valuelen)==-1)
			return -1;
		if (valuelen > CCHFORMNAME){
			PyErr_Format(PyExc_ValueError,"FormName must be a string of length %d or less", CCHFORMNAME);
			return -1;
			}
		PDEVMODE pdevmode=&((PyDEVMODE *)self)->devmode;
		ZeroMemory(&pdevmode->dmFormName, CCHFORMNAME);
		memcpy(&pdevmode->dmFormName, value, valuelen);
		// update variable length PDEVMODE with same value
		memcpy(((PyDEVMODE *)self)->pdevmode, pdevmode, pdevmode->dmSize);
		return 0;
		}

	if (strcmp(name,"DriverData")==0){
		if (PyString_AsStringAndSize(obvalue, &value, &valuelen)==-1)
			return -1;
		PDEVMODE pdevmode=((PyDEVMODE *)self)->pdevmode;
		if (valuelen > pdevmode->dmDriverExtra){
			PyErr_Format(PyExc_ValueError,"Length of DriverData cannot be longer that DriverExtra (%d bytes)", pdevmode->dmDriverExtra);
			return -1;
			}
		// This is not a real struct member, calculate address after end of fixed part of structure
		char *driverdata=(char *)((ULONG_PTR)pdevmode + pdevmode->dmSize); 
		ZeroMemory(driverdata, pdevmode->dmDriverExtra);
		memcpy(driverdata, value, valuelen);
		return 0;
		}

	int ret=PyObject_GenericSetAttr(self, obname, obvalue);
	// Propagate changes to the externally usable structure
	if (ret==0)
		memcpy(((PyDEVMODE *)self)->pdevmode, &((PyDEVMODE *)self)->devmode, ((PyDEVMODE *)self)->devmode.dmSize);
	return ret;
}

PyObject *PyDEVMODE::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
	USHORT DriverExtra=0;
	static char *keywords[]={"DriverExtra", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|H", keywords, &DriverExtra))
		return NULL;
	return new PyDEVMODE(DriverExtra);
}

BOOL PyWinObject_AsDEVMODE(PyObject *ob, PDEVMODE *ppDEVMODE, BOOL bNoneOk)
{
	if (ob==Py_None)
		if (bNoneOk){
			*ppDEVMODE=NULL;
			return TRUE;
			}
		else{
			PyErr_SetString(PyExc_ValueError,"PyDEVMODE cannot be None in this context");
			return FALSE;
			}
	if (!PyDEVMODE_Check(ob))
		return FALSE;
	*ppDEVMODE=((PyDEVMODE *)ob)->GetDEVMODE();
	return TRUE;
}

PyObject *PyWinObject_FromDEVMODE(PDEVMODE pDEVMODE)
{
	static WORD dmSize=sizeof(DEVMODE);
	if (pDEVMODE==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}

	// make sure we can't overflow the fixed size DEVMODE in PyDEVMODE
	if (pDEVMODE->dmSize>dmSize){
		PyErr_Format(PyExc_WindowsError,"DEVMODE structure of size %d greater than supported size of %d", pDEVMODE->dmSize, dmSize);
		return NULL;
		}
	PyObject *ret=new PyDEVMODE(pDEVMODE);
	// check that variable sized pdevmode is allocated
	if (((PyDEVMODE *)ret)->GetDEVMODE()==NULL){
		Py_DECREF(ret);
		ret=NULL;
		}
	return ret;
}
