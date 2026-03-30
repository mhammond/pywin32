// @doc - This file contains autoduck documentation

#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"

// DEVMODEW support
// @object PyDEVMODEW|Unicode version of <o PyDEVMODE> object

struct PyMethodDef PyDEVMODEW::methods[] = {
    {"Clear", PyDEVMODEW::Clear, 1},  // @pymeth Clear|Resets all members of the structure
    {NULL}};

#define OFFW(e) offsetof(PyDEVMODEW, e)
struct PyMemberDef PyDEVMODEW::members[] = {
    // @prop int|SpecVersion|Should always be set to DM_SPECVERSION
    {"SpecVersion", T_USHORT, OFFW(devmode.dmSpecVersion), 0, "Should always be set to DM_SPECVERSION"},
    // @prop int|DriverVersion|Version nbr assigned to printer driver by vendor
    {"DriverVersion", T_USHORT, OFFW(devmode.dmDriverVersion), 0, "Version nbr assigned to printer driver by vendor"},
    // @prop int|Size|Size of structure
    {"Size", T_USHORT, OFFW(devmode.dmSize), READONLY, "Size of structure"},
    // @prop int|DriverExtra|Number of extra bytes allocated for driver data, can only be set when new object is created
    {"DriverExtra", T_USHORT, OFFW(devmode.dmDriverExtra), READONLY,
     "Number of extra bytes allocated for driver data, can only be set when new object is created"},
    // @prop int|Fields|Bitmask of win32con.DM_* constants indicating which members are set
    {"Fields", T_ULONG, OFFW(devmode.dmFields), 0,
     "Bitmask of win32con.DM_* constants indicating which members are set"},
    // @prop int|Orientation|Only applies to printers, DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE
    {"Orientation", T_SHORT, OFFW(devmode.dmOrientation), 0,
     "Only applies to printers, DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE"},
    // @prop int|PaperSize|Use 0 if PaperWidth and PaperLength are set, otherwise win32con.DMPAPER_* constant
    {"PaperSize", T_SHORT, OFFW(devmode.dmPaperSize), 0,
     "Use 0 if PaperWidth and PaperLength are set, otherwise win32con.DMPAPER_* constant"},
    // @prop int|PaperLength|Specified in 1/10 millimeters
    {"PaperLength", T_SHORT, OFFW(devmode.dmPaperLength), 0, "Specified in 1/10 millimeters"},
    // @prop int|PaperWidth|Specified in 1/10 millimeters
    {"PaperWidth", T_SHORT, OFFW(devmode.dmPaperWidth), 0, "Specified in 1/10 millimeters"},
    // @prop int|Position_x|Position of display relative to desktop
    {"Position_x", T_LONG, OFFW(devmode.dmPosition.x), 0, "Position of display relative to desktop"},
    // @prop int|Position_y|Position of display relative to desktop
    {"Position_y", T_LONG, OFFW(devmode.dmPosition.y), 0, "Position of display relative to desktop"},
    // @prop int|DisplayOrientation|Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270
    {"DisplayOrientation", T_ULONG, OFFW(devmode.dmDisplayOrientation), 0,
     "Display rotation: DMDO_DEFAULT,DMDO_90, DMDO_180, DMDO_270"},
    // @prop int|DisplayFixedOutput|DMDFO_DEFAULT, DMDFO_CENTER, DMDFO_STRETCH
    {"DisplayFixedOutput", T_ULONG, OFFW(devmode.dmDisplayFixedOutput), 0,
     "DMDFO_DEFAULT, DMDFO_CENTER, DMDFO_STRETCH"},
    // @prop int|Scale|Specified as percentage, eg 50 means half size of original
    {"Scale", T_SHORT, OFFW(devmode.dmScale), 0, "Specified as percentage, eg 50 means half size of original"},
    // @prop int|Copies|Nbr of copies to print
    {"Copies", T_SHORT, OFFW(devmode.dmCopies), 0, "Nbr of copies to print"},
    // @prop int|DefaultSource|DMBIN_* constant, or can be a printer-specific value
    {"DefaultSource", T_SHORT, OFFW(devmode.dmDefaultSource), 0,
     "DMBIN_* constant, or can be a printer-specific value"},
    // @prop int|PrintQuality|DMRES_* constant, interpreted as DPI if positive
    {"PrintQuality", T_SHORT, OFFW(devmode.dmPrintQuality), 0, "DMRES_* constant, interpreted as DPI if positive"},
    // @prop int|Color|DMCOLOR_COLOR or DMCOLOR_MONOCHROME
    {"Color", T_SHORT, OFFW(devmode.dmColor), 0, "DMCOLOR_COLOR or DMCOLOR_MONOCHROME"},
    // @prop int|Duplex|For printers that do two-sided printing: DMDUP_SIMPLEX, DMDUP_HORIZONTAL, DMDUP_VERTICAL
    {"Duplex", T_SHORT, OFFW(devmode.dmDuplex), 0,
     "For printers that do two-sided printing: DMDUP_SIMPLEX, DMDUP_HORIZONTAL, DMDUP_VERTICAL"},
    // @prop int|YResolution|Vertical printer resolution in DPI - if this is set, PrintQuality indicates horizontal DPI
    {"YResolution", T_SHORT, OFFW(devmode.dmYResolution), 0,
     "Vertical printer resolution in DPI - if this is set, PrintQuality indicates horizontal DPI"},
    // @prop int|TTOption|TrueType options: DMTT_BITMAP, DMTT_DOWNLOAD, DMTT_DOWNLOAD_OUTLINE, DMTT_SUBDEV
    {"TTOption", T_SHORT, OFFW(devmode.dmTTOption), 0,
     "TrueType options: DMTT_BITMAP, DMTT_DOWNLOAD, DMTT_DOWNLOAD_OUTLINE, DMTT_SUBDEV"},
    // @prop int|Collate|DMCOLLATE_TRUE or DMCOLLATE_FALSE
    {"Collate", T_SHORT, OFFW(devmode.dmCollate), 0, "DMCOLLATE_TRUE or DMCOLLATE_FALSE"},
    // @prop int|LogPixels|Pixels per inch (only for display devices
    {"LogPixels", T_USHORT, OFFW(devmode.dmLogPixels), 0, "Pixels per inch (only for display devices)"},
    // @prop int|BitsPerPel|Color resolution in bits per pixel
    {"BitsPerPel", T_ULONG, OFFW(devmode.dmBitsPerPel), 0, "Color resolution in bits per pixel"},
    // @prop int|PelsWidth|Pixel width of display
    {"PelsWidth", T_ULONG, OFFW(devmode.dmPelsWidth), 0, "Pixel width of display"},
    // @prop int|PelsHeight|Pixel height of display
    {"PelsHeight", T_ULONG, OFFW(devmode.dmPelsHeight), 0, "Pixel height of display"},
    // @prop int|DisplayFlags|Combination of DM_GRAYSCALE and DM_INTERLACED
    {"DisplayFlags", T_ULONG, OFFW(devmode.dmDisplayFlags), 0, "Combination of DM_GRAYSCALE and DM_INTERLACED"},
    // @prop int|DisplayFrequency|Refresh rate
    {"DisplayFrequency", T_ULONG, OFFW(devmode.dmDisplayFrequency), 0, "Refresh rate"},
    // @prop int|ICMMethod|Indicates where ICM is performed, one of win32con.DMICMMETHOD_* values
    {"ICMMethod", T_ULONG, OFFW(devmode.dmICMMethod), 0,
     "Indicates where ICM is performed, one of win32con.DMICMMETHOD_* values"},
    // @prop int|ICMIntent|Intent of ICM, one of win32con.DMICM_* values
    {"ICMIntent", T_ULONG, OFFW(devmode.dmICMIntent), 0, "Intent of ICM, one of win32con.DMICM_* values"},
    // @prop int|MediaType|win32con.DMMEDIA_*, can also be a printer-specific value greater then DMMEDIA_USER
    {"MediaType", T_ULONG, OFFW(devmode.dmMediaType), 0,
     "win32con.DMMEDIA_*, can also be a printer-specific value greater then DMMEDIA_USER"},
    // @prop int|DitherType|Dithering option, win32con.DMDITHER_*
    {"DitherType", T_ULONG, OFFW(devmode.dmDitherType), 0, "Dithering options, win32con.DMDITHER_*"},
    // @prop int|Reserved1|Reserved, use only 0
    {"Reserved1", T_ULONG, OFFW(devmode.dmReserved1), 0, "Reserved, use only 0"},
    // @prop int|Reserved2|Reserved, use only 0
    {"Reserved2", T_ULONG, OFFW(devmode.dmReserved2), 0, "Reserved, use only 0"},
    // @prop int|Nup|Controls printing of multiple logical pages per physical page, DMNUP_SYSTEM or DMNUP_ONEUP
    {"Nup", T_ULONG, OFFW(devmode.dmNup), 0,
     "Controls printing of multiple logical pages per physical page, DMNUP_SYSTEM or DMNUP_ONEUP"},
    // @prop int|PanningWidth|Not used, leave as 0
    {"PanningWidth", T_ULONG, OFFW(devmode.dmPanningWidth), 0, "Not used, leave as 0"},
    // @prop int|PanningHeight|Not used, leave as 0
    {"PanningHeight", T_ULONG, OFFW(devmode.dmPanningHeight), 0, "Not used, leave as 0"},
    {NULL}};

// @prop string|DeviceName|String of at most 32 chars
PyObject *PyDEVMODEW::get_DeviceName(PyObject *self, void *unused)
{
    PDEVMODEW pdevmode = ((PyDEVMODEW *)self)->pdevmode;
    if (pdevmode->dmDeviceName[CCHDEVICENAME - 1] == 0)  // in case DeviceName fills space and has no trailing NULL
        return PyWinObject_FromWCHAR(pdevmode->dmDeviceName);
    return PyWinObject_FromWCHAR(pdevmode->dmDeviceName, CCHDEVICENAME);
}

int PyDEVMODEW::set_DeviceName(PyObject *self, PyObject *v, void *unused)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Attributes of PyDEVMODEW can't be deleted");
        return -1;
    }
    WCHAR *devicename;
    DWORD cch;
    if (!PyWinObject_AsWCHAR(v, &devicename, FALSE, &cch))
        return -1;
    if (cch > CCHDEVICENAME) {
        PyErr_Format(PyExc_ValueError, "DeviceName must be a string of length %d or less", CCHDEVICENAME);
        PyWinObject_FreeWCHAR(devicename);
        return -1;
    }
    PDEVMODEW pdevmode = &((PyDEVMODEW *)self)->devmode;
    ZeroMemory(&pdevmode->dmDeviceName, sizeof(pdevmode->dmDeviceName));
    memcpy(&pdevmode->dmDeviceName, devicename, cch * sizeof(WCHAR));
    // update variable length DEVMODE with same value
    memcpy(((PyDEVMODEW *)self)->pdevmode, pdevmode, pdevmode->dmSize);
    PyWinObject_FreeWCHAR(devicename);
    return 0;
}

// @prop str|FormName|Name of form as returned by <om win32print.EnumForms>, at most 32 chars
PyObject *PyDEVMODEW::get_FormName(PyObject *self, void *unused)
{
    PDEVMODEW pdevmode = ((PyDEVMODEW *)self)->pdevmode;
    if (pdevmode->dmFormName[CCHFORMNAME - 1] == 0)  // If dmFormName occupies whole 32 chars, trailing NULL not present
        return PyWinObject_FromWCHAR(pdevmode->dmFormName);
    return PyWinObject_FromWCHAR(pdevmode->dmFormName, CCHFORMNAME);
}

int PyDEVMODEW::set_FormName(PyObject *self, PyObject *v, void *unused)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Attributes of PyDEVMODEW can't be deleted");
        return -1;
    }
    WCHAR *formname;
    DWORD cch;
    if (!PyWinObject_AsWCHAR(v, &formname, FALSE, &cch))
        return -1;
    if (cch > CCHFORMNAME) {
        PyErr_Format(PyExc_ValueError, "FormName must be a string of length %d or less", CCHFORMNAME);
        PyWinObject_FreeWCHAR(formname);
        return -1;
    }
    PDEVMODEW pdevmode = &((PyDEVMODEW *)self)->devmode;
    ZeroMemory(&pdevmode->dmFormName, sizeof(pdevmode->dmFormName));
    memcpy(&pdevmode->dmFormName, formname, cch * sizeof(WCHAR));
    // update variable length PDEVMODE with same value
    memcpy(((PyDEVMODEW *)self)->pdevmode, pdevmode, pdevmode->dmSize);
    PyWinObject_FreeWCHAR(formname);
    return 0;
}

// @prop str|DriverData|Driver data appended to end of structure
PyObject *PyDEVMODEW::get_DriverData(PyObject *self, void *unused)
{
    PDEVMODEW pdevmode = ((PyDEVMODEW *)self)->pdevmode;
    if (pdevmode->dmDriverExtra == 0) {  // No extra space allocated
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyBytes_FromStringAndSize((char *)((ULONG_PTR)pdevmode + pdevmode->dmSize), pdevmode->dmDriverExtra);
}

int PyDEVMODEW::set_DriverData(PyObject *self, PyObject *v, void *unused)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Attributes of PyDEVMODEW can't be deleted");
        return -1;
    }
    char *value;
    Py_ssize_t valuelen;
    if (PyBytes_AsStringAndSize(v, &value, &valuelen) == -1)
        return -1;
    PDEVMODEW pdevmode = ((PyDEVMODEW *)self)->pdevmode;
    if (valuelen > pdevmode->dmDriverExtra) {
        PyErr_Format(PyExc_ValueError, "Length of DriverData cannot be longer that DriverExtra (%d bytes)",
                     pdevmode->dmDriverExtra);
        return -1;
    }
    // This is not a real struct member, calculate address after end of fixed part of structure
    char *driverdata = (char *)((ULONG_PTR)pdevmode + pdevmode->dmSize);
    ZeroMemory(driverdata, pdevmode->dmDriverExtra);
    memcpy(driverdata, value, valuelen);
    return 0;
}

PyGetSetDef PyDEVMODEW::getset[] = {
    {"DeviceName", PyDEVMODEW::get_DeviceName, PyDEVMODEW::set_DeviceName, "String of at most 32 chars"},
    {"FormName", PyDEVMODEW::get_FormName, PyDEVMODEW::set_FormName,
     "Name of form as returned by EnumForms, at most 32 chars"},
    {"DriverData", PyDEVMODEW::get_DriverData, PyDEVMODEW::set_DriverData, "Driver data appended to end of structure"},
    {NULL}};

PYWINTYPES_EXPORT PyTypeObject PyDEVMODEWType = {
    PYWIN_OBJECT_HEAD "PyDEVMODEW",
    sizeof(PyDEVMODEW),
    0,
    PyDEVMODEW::deallocFunc,
    0,                                         // tp_print;
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // tp_as_number
    0,                                         // tp_as_sequence
    0,                                         // tp_as_mapping
    0,                                         // tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    0,                                         // tp_getattro
    0,                                         // tp_setattro
    0,                                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags;
    0,                                         // tp_doc
    0,                                         // traverseproc tp_traverse;
    0,                                         // tp_clear;
    0,                                         // tp_richcompare;
    0,                                         // tp_weaklistoffset;
    0,                                         // tp_iter
    0,                                         // iternextfunc tp_iternext
    PyDEVMODEW::methods,
    PyDEVMODEW::members,
    PyDEVMODEW::getset,  // tp_getset;
    0,                   // tp_base;
    0,                   // tp_dict;
    0,                   // tp_descr_get;
    0,                   // tp_descr_set;
    0,                   // tp_dictoffset;
    0,                   // tp_init;
    0,                   // tp_alloc;
    PyDEVMODEW::tp_new   // newfunc tp_new;
};

PyDEVMODEW::PyDEVMODEW(PDEVMODEW pdm)
{
    ob_type = &PyDEVMODEWType;
    memcpy(&devmode, pdm, pdm->dmSize);
    pdevmode = (PDEVMODEW)malloc(pdm->dmSize + pdm->dmDriverExtra);
    if (pdevmode == NULL)
        PyErr_Format(PyExc_MemoryError, "PyDEVMODE::PyDEVMODE - Unable to allocate DEVMODE of size %d",
                     pdm->dmSize + pdm->dmDriverExtra);
    else
        memcpy(pdevmode, pdm, pdm->dmSize + pdm->dmDriverExtra);
    _Py_NewReference(this);
}

PyDEVMODEW::PyDEVMODEW(void)
{
    ob_type = &PyDEVMODEWType;
    static WORD dmSize = sizeof(DEVMODEW);
    pdevmode = (PDEVMODEW)malloc(dmSize);
    ZeroMemory(pdevmode, dmSize);
    pdevmode->dmSize = dmSize;
    pdevmode->dmSpecVersion = DM_SPECVERSION;
    ZeroMemory(&devmode, dmSize);
    devmode.dmSize = dmSize;
    devmode.dmSpecVersion = DM_SPECVERSION;
    _Py_NewReference(this);
}

PyDEVMODEW::PyDEVMODEW(USHORT dmDriverExtra)
{
    ob_type = &PyDEVMODEWType;
    static WORD dmSize = sizeof(DEVMODEW);
    pdevmode = (PDEVMODEW)malloc(dmSize + dmDriverExtra);
    ZeroMemory(pdevmode, dmSize + dmDriverExtra);
    pdevmode->dmSize = dmSize;
    pdevmode->dmSpecVersion = DM_SPECVERSION;
    pdevmode->dmDriverExtra = dmDriverExtra;
    ZeroMemory(&devmode, dmSize);
    devmode.dmSize = dmSize;
    devmode.dmSpecVersion = DM_SPECVERSION;
    devmode.dmDriverExtra = dmDriverExtra;
    _Py_NewReference(this);
}

PyDEVMODEW::~PyDEVMODEW()
{
    if (pdevmode != NULL)
        free(pdevmode);
}

BOOL PyDEVMODEW_Check(PyObject *ob)
{
    if (Py_TYPE(ob) != &PyDEVMODEWType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyDEVMODEW");
        return FALSE;
    }
    return TRUE;
}

void PyDEVMODEW::deallocFunc(PyObject *ob) { delete (PyDEVMODEW *)ob; }

PDEVMODEW PyDEVMODEW::GetDEVMODE(void)
{
    // Propagate any changes made by python attribute logic from the fixed length DEVMODE
    // to the externally visible variable length DEVMODE before handing it off to anyone else
    memcpy(pdevmode, &devmode, devmode.dmSize);
    return pdevmode;
}

// @pymethod |PyDEVMODE|Clear|Resets all members of the structure
PyObject *PyDEVMODEW::Clear(PyObject *self, PyObject *args)
{
    PDEVMODEW pdevmode = ((PyDEVMODEW *)self)->pdevmode;
    USHORT dmDriverExtra = pdevmode->dmDriverExtra;
    WORD dmSize = pdevmode->dmSize;
    DWORD totalsize = dmSize + dmDriverExtra;
    ZeroMemory(pdevmode, totalsize);
    pdevmode->dmDriverExtra = dmDriverExtra;
    pdevmode->dmSize = dmSize;
    pdevmode->dmSpecVersion = DM_SPECVERSION;

    pdevmode = &((PyDEVMODEW *)self)->devmode;
    ZeroMemory(pdevmode, dmSize);
    pdevmode->dmDriverExtra = dmDriverExtra;
    pdevmode->dmSize = dmSize;
    pdevmode->dmSpecVersion = DM_SPECVERSION;
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyDEVMODEW::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
    USHORT DriverExtra = 0;
    static char *keywords[] = {"DriverExtra", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|H", keywords, &DriverExtra))
        return NULL;
    return new PyDEVMODEW(DriverExtra);
}

BOOL PyWinObject_AsDEVMODE(PyObject *ob, PDEVMODEW *ppDEVMODE, BOOL bNoneOk)
{
    if (ob == Py_None) {
        if (bNoneOk) {
            *ppDEVMODE = NULL;
            return TRUE;
        }
        else {
            PyErr_SetString(PyExc_ValueError, "PyDEVMODE cannot be None in this context");
            return FALSE;
        }
    }
    if (!PyDEVMODEW_Check(ob))
        return FALSE;
    *ppDEVMODE = ((PyDEVMODEW *)ob)->GetDEVMODE();
    return TRUE;
}

PyObject *PyWinObject_FromDEVMODE(PDEVMODEW pDEVMODE)
{
    static WORD dmSize = sizeof(DEVMODEW);
    if (pDEVMODE == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // make sure we can't overflow the fixed size DEVMODE in PyDEVMODE
    if (pDEVMODE->dmSize > dmSize) {
        PyErr_Format(PyExc_WindowsError, "DEVMODE structure of size %d greater than supported size of %d",
                     pDEVMODE->dmSize, dmSize);
        return NULL;
    }
    PyObject *ret = new PyDEVMODEW(pDEVMODE);
    // check that variable sized pdevmode is allocated
    if (((PyDEVMODEW *)ret)->GetDEVMODE() == NULL) {
        Py_DECREF(ret);
        ret = NULL;
    }
    return ret;
}
