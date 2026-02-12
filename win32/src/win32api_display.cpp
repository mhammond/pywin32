// @doc - This file contains autoduck documentation
#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"
#include "win32api_display.h"

// from user32.dll, loaded in win32api's init function
ChangeDisplaySettingsExfunc pfnChangeDisplaySettingsEx = NULL;
EnumDisplayDevicesfunc pfnEnumDisplayDevices = NULL;
EnumDisplayMonitorsfunc pfnEnumDisplayMonitors = NULL;
EnumDisplaySettingsExfunc pfnEnumDisplaySettingsEx = NULL;
GetMonitorInfofunc pfnGetMonitorInfo = NULL;
MonitorFromPointfunc pfnMonitorFromPoint = NULL;
MonitorFromRectfunc pfnMonitorFromRect = NULL;
MonitorFromWindowfunc pfnMonitorFromWindow = NULL;

// @object PyDISPLAY_DEVICE|Python object wrapping a DISPLAY_DEVICE structure
struct PyMethodDef PyDISPLAY_DEVICE::methods[] = {
    {"Clear", PyDISPLAY_DEVICE::Clear, 1},  // @pymeth Clear|Resets all members of the structure
    {NULL}};

#define OFF(e) offsetof(PyDISPLAY_DEVICE, e)
struct PyMemberDef PyDISPLAY_DEVICE::members[] = {
    // @prop int|Size|Size of structure
    {"Size", T_ULONG, OFF(display_device.cb), READONLY, "Size of structure"},
    // DeviceName is a dummy so it will show up in property list, get and set handle manually
    // same for DeviceString, DeviceID, DeviceKey
    // @prop str|DeviceName|String of at most 32 chars
    {"DeviceName", T_OBJECT, OFF(obdummy), 0, "String of at most 32 chars"},
    // @prop str|DeviceString|String of at most 128 chars
    {"DeviceString", T_OBJECT, OFF(obdummy), 0, "String of at most 128 chars"},
    // @prop int|StateFlags|Bitmask of win32con.DISPLAY_DEVICE_* constants indicating current device status
    {"StateFlags", T_ULONG, OFF(display_device.StateFlags), 0,
     "Bitmask of DISPLAY_DEVICE_* constants indicating current device status"},
    // @prop str|DeviceID|String of at most 128 chars
    {"DeviceID", T_OBJECT, OFF(obdummy), 0, "String of at most 128 chars"},
    // @prop str|DeviceKey|String of at most 128 chars
    {"DeviceKey", T_OBJECT, OFF(obdummy), 0, "String of at most 128 chars"},
    {NULL}};

PyTypeObject PyDISPLAY_DEVICEType = {
    PYWIN_OBJECT_HEAD "PyDISPLAY_DEVICE",
    sizeof(PyDISPLAY_DEVICE),
    0,
    PyDISPLAY_DEVICE::deallocFunc,
    0,  // tp_print;
    0,  // tp_getattr
    0,  // tp_setattr
    0,  // tp_compare
    0,  // tp_repr
    0,  // tp_as_number
    0,  // tp_as_sequence
    0,  // tp_as_mapping
    0,
    0,                                         /* tp_call */
    0,                                         /* tp_str */
    PyDISPLAY_DEVICE::getattro,                // PyObject_GenericGetAttr
    PyDISPLAY_DEVICE::setattro,                // PyObject_GenericSetAttr
    0,                                         // tp_as_buffer;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags;
    0,                                         // tp_doc; /* Documentation string */
    0,                                         // traverseproc tp_traverse;
    0,                                         // tp_clear;
    0,                                         // tp_richcompare;
    0,                                         // tp_weaklistoffset;
    0,                                         // tp_iter
    0,                                         // iternextfunc tp_iternext
    PyDISPLAY_DEVICE::methods,
    PyDISPLAY_DEVICE::members,
    0,                        // tp_getset;
    0,                        // tp_base;
    0,                        // tp_dict;
    0,                        // tp_descr_get;
    0,                        // tp_descr_set;
    0,                        // tp_dictoffset;
    0,                        // tp_init;
    0,                        // tp_alloc;
    PyDISPLAY_DEVICE::tp_new  // newfunc tp_new;
};

PyDISPLAY_DEVICE::PyDISPLAY_DEVICE(PDISPLAY_DEVICE pdd)
{
    ob_type = &PyDISPLAY_DEVICEType;
    memcpy(&display_device, pdd, pdd->cb);
    obdummy = NULL;
    _Py_NewReference(this);
}

PyDISPLAY_DEVICE::PyDISPLAY_DEVICE(void)
{
    ob_type = &PyDISPLAY_DEVICEType;
    static DWORD cb = sizeof(DISPLAY_DEVICE);
    ZeroMemory(&display_device, cb);
    display_device.cb = cb;
    obdummy = NULL;
    _Py_NewReference(this);
}

PyDISPLAY_DEVICE::~PyDISPLAY_DEVICE() {}

BOOL PyDISPLAY_DEVICE_Check(PyObject *ob)
{
    if (Py_TYPE(ob) != &PyDISPLAY_DEVICEType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyDISPLAY_DEVICE");
        return FALSE;
    }
    return TRUE;
}

void PyDISPLAY_DEVICE::deallocFunc(PyObject *ob) { delete (PyDISPLAY_DEVICE *)ob; }

PDISPLAY_DEVICE PyDISPLAY_DEVICE::GetDISPLAY_DEVICE(void) { return &display_device; }

// @pymethod |PyDISPLAY_DEVICE|Clear|Resets all members of the structure
PyObject *PyDISPLAY_DEVICE::Clear(PyObject *self, PyObject *args)
{
    PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
    DWORD cb = pdisplay_device->cb;
    ZeroMemory(pdisplay_device, cb);
    pdisplay_device->cb = cb;
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyDISPLAY_DEVICE::getattro(PyObject *self, PyObject *obname)
{
    PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    if (strcmp(name, "DeviceName") == 0) {
        if (pdisplay_device->DeviceName[31] == 0)  // in case DeviceName fills space and has no trailing NULL
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceName);
        else
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceName, 32);
    }

    if (strcmp(name, "DeviceString") == 0) {
        if (pdisplay_device->DeviceString[127] == 0)  // in case DeviceString fills space and has no trailing NULL
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceString);
        else
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceString, 128);
    }

    if (strcmp(name, "DeviceID") == 0) {
        if (pdisplay_device->DeviceID[127] == 0)  // in case DeviceID fills space and has no trailing NULL
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceID);
        else
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceID, 128);
    }

    if (strcmp(name, "DeviceKey") == 0) {
        if (pdisplay_device->DeviceKey[127] == 0)  // in case DeviceKey fills space and has no trailing NULL
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceKey);
        else
            return PyWinObject_FromTCHAR(pdisplay_device->DeviceKey, 128);
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyDISPLAY_DEVICE::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;

    TCHAR *value = NULL;
    DWORD valuelen;

    if (strcmp(name, "DeviceName") == 0) {
        PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
        DWORD cch_max = sizeof(pdisplay_device->DeviceName) / sizeof(TCHAR);
        if (!PyWinObject_AsTCHAR(obvalue, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > cch_max) {
            PyErr_Format(PyExc_ValueError, "DeviceName must be a string of length %d or less", cch_max);
            PyWinObject_FreeTCHAR(value);
            return -1;
        }
        ZeroMemory(&pdisplay_device->DeviceName, sizeof(pdisplay_device->DeviceName));
        memcpy(&pdisplay_device->DeviceName, value, valuelen * sizeof(TCHAR));
        PyWinObject_FreeTCHAR(value);
        return 0;
    }
    if (strcmp(name, "DeviceString") == 0) {
        PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
        DWORD cch_max = sizeof(pdisplay_device->DeviceString) / sizeof(TCHAR);
        if (!PyWinObject_AsTCHAR(obvalue, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > cch_max) {
            PyErr_Format(PyExc_ValueError, "DeviceString must be a string of length %d or less", cch_max);
            PyWinObject_FreeTCHAR(value);
            return -1;
        }

        ZeroMemory(&pdisplay_device->DeviceString, sizeof(pdisplay_device->DeviceString));
        memcpy(&pdisplay_device->DeviceString, value, valuelen * sizeof(TCHAR));
        PyWinObject_FreeTCHAR(value);
        return 0;
    }
    if (strcmp(name, "DeviceID") == 0) {
        PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
        DWORD cch_max = sizeof(pdisplay_device->DeviceID) / sizeof(TCHAR);
        if (!PyWinObject_AsTCHAR(obvalue, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > cch_max) {
            PyErr_Format(PyExc_ValueError, "DeviceID must be a string of length %d or less", cch_max);
            PyWinObject_FreeTCHAR(value);
            return -1;
        }

        ZeroMemory(&pdisplay_device->DeviceID, sizeof(pdisplay_device->DeviceID));
        memcpy(&pdisplay_device->DeviceID, value, valuelen * sizeof(TCHAR));
        PyWinObject_FreeTCHAR(value);
        return 0;
    }
    if (strcmp(name, "DeviceKey") == 0) {
        PDISPLAY_DEVICE pdisplay_device = &((PyDISPLAY_DEVICE *)self)->display_device;
        DWORD cch_max = sizeof(pdisplay_device->DeviceKey) / sizeof(TCHAR);
        if (!PyWinObject_AsTCHAR(obvalue, &value, FALSE, &valuelen))
            return -1;
        if (valuelen > cch_max) {
            PyErr_Format(PyExc_ValueError, "DeviceKey must be a string of length %d or less", cch_max);
            PyWinObject_FreeTCHAR(value);
            return -1;
        }
        ZeroMemory(&pdisplay_device->DeviceKey, sizeof(pdisplay_device->DeviceKey));
        memcpy(&pdisplay_device->DeviceKey, value, valuelen * sizeof(TCHAR));
        PyWinObject_FreeTCHAR(value);
        return 0;
    }

    return PyObject_GenericSetAttr(self, obname, obvalue);
}

PyObject *PyDISPLAY_DEVICE::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
    return new PyDISPLAY_DEVICE();
}

BOOL PyWinObject_AsDISPLAY_DEVICE(PyObject *ob, PDISPLAY_DEVICE *ppDISPLAY_DEVICE, BOOL bNoneOk)
{
    if (ob == Py_None) {
        if (bNoneOk) {
            *ppDISPLAY_DEVICE = NULL;
            return TRUE;
        }
        else {
            PyErr_SetString(PyExc_ValueError, "PyDISPLAY_DEVICE cannot be None in this context");
            return FALSE;
        }
    }
    if (!PyDISPLAY_DEVICE_Check(ob))
        return FALSE;
    *ppDISPLAY_DEVICE = ((PyDISPLAY_DEVICE *)ob)->GetDISPLAY_DEVICE();
    return TRUE;
}

PyObject *PyWinObject_FromDISPLAY_DEVICE(PDISPLAY_DEVICE pDISPLAY_DEVICE)
{
    static DWORD cb = sizeof(DISPLAY_DEVICE);
    if (pDISPLAY_DEVICE == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // make sure we can't overflow the fixed size DISPLAY_DEVICE in PyDISPLAY_DEVICE
    if (pDISPLAY_DEVICE->cb > cb) {
        PyErr_Format(PyExc_WindowsError, "DISPLAY_DEVICE structure of size %d greater than supported size of %d",
                     pDISPLAY_DEVICE->cb, cb);
        return NULL;
    }
    PyObject *ret = new PyDISPLAY_DEVICE(pDISPLAY_DEVICE);
    // check that variable sized pdisplay_device is allocated
    if (((PyDISPLAY_DEVICE *)ret)->GetDISPLAY_DEVICE() == NULL) {
        Py_DECREF(ret);
        ret = NULL;
    }
    return ret;
}

// @pymethod int|win32api|ChangeDisplaySettings|Changes video mode for default display
// @rdesc Returns DISP_CHANGE_SUCCESSFUL on success, or one of the DISP_CHANGE_* error constants on failure
PyObject *PyChangeDisplaySettings(PyObject *self, PyObject *args)
{
    DWORD Flags;
    PDEVMODE pdevmode;
    PyObject *obdevmode;
    long ret;
    // @pyparm <o PyDEVMODE>|DevMode||A PyDEVMODE object as returned from EnumDisplaySettings, or None to reset to
    // default settings from registry
    // @pyparm int|Flags||One of the win32con.CDS_* constants, or 0
    if (!PyArg_ParseTuple(args, "Ol:ChangeDisplaySettings", &obdevmode, &Flags))
        return NULL;
    if (!PyWinObject_AsDEVMODE(obdevmode, &pdevmode, TRUE))
        return NULL;
    // DISP_CHANGE_* errors don't translate as win32 error codes, just return it
    ret = ::ChangeDisplaySettings(pdevmode, Flags);
    return PyLong_FromLong(ret);
}

// @pymethod int|win32api|ChangeDisplaySettingsEx|Changes video mode for specified display
// @rdesc Returns DISP_CHANGE_SUCCESSFUL on success, or one of the DISP_CHANGE_* error constants on failure
// @comm Accepts keyword arguments
PyObject *PyChangeDisplaySettingsEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    // for now there's no hwnd param parsing (required to be NULL anyway), and no lParam
    CHECK_PFN(ChangeDisplaySettingsEx);
    static char *keywords[] = {"DeviceName", "DevMode", "Flags", NULL};

    DWORD Flags = 0;
    TCHAR *DeviceName = NULL;
    PDEVMODE pdevmode;
    PyObject *obDeviceName = Py_None, *obdevmode = Py_None;
    long ret;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|OOk:ChangeDisplaySettingsEx", keywords,
            &obDeviceName,  // @pyparm str|DeviceName|None|Name of device as returned by <om
                            // win32api.EnumDisplayDevices>, use None for default display device
            &obdevmode,     // @pyparm <o PyDEVMODE>|DevMode|None|A PyDEVMODE object as returned from <om
                            // win32api.EnumDisplaySettings>, or None to reset to default settings from registry
            &Flags))        // @pyparm int|Flags|0|One of the win32con.CDS_* constants, or 0
        return NULL;

    if (!PyWinObject_AsDEVMODE(obdevmode, &pdevmode, TRUE))
        return NULL;
    if (!PyWinObject_AsTCHAR(obDeviceName, &DeviceName, TRUE))
        return NULL;
    // DISP_CHANGE_* errors don't translate as win32 error codes, just return it
    ret = (*pfnChangeDisplaySettingsEx)(DeviceName, pdevmode, (HWND)NULL, Flags, (LPVOID)NULL);
    PyWinObject_FreeTCHAR(DeviceName);
    return PyLong_FromLong(ret);
}

// @pymethod <o PyDISPLAY_DEVICE>|win32api|EnumDisplayDevices|Obtain information about the display devices in a system
// @comm Accepts keyword arguments
PyObject *PyEnumDisplayDevices(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(EnumDisplayDevices);
    static char *keywords[] = {"Device", "DevNum", "Flags", NULL};
    TCHAR *Device = NULL;
    PyObject *obDevice = Py_None, *ret = NULL;
    DWORD DevNum = 0;
    DWORD Flags = 0;
    DISPLAY_DEVICE display_device;

    // @pyparm string|Device|None|Name of device, use None to obtain information for the display adapter(s) on the
    // machine, based on DevNum
    // @pyparm int|DevNum|0|Index of device of interest, starting with zero
    // @pyparm int|Flags|0|Reserved, use 0 if passed in
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Okk:EnumDisplayDevices", keywords, &obDevice, &DevNum, &Flags))
        return NULL;
    if (!PyWinObject_AsTCHAR(obDevice, &Device, TRUE))
        return NULL;
    ZeroMemory(&display_device, sizeof(DISPLAY_DEVICE));
    display_device.cb = sizeof(DISPLAY_DEVICE);
    if (!(*pfnEnumDisplayDevices)(Device, DevNum, &display_device, Flags))
        PyWin_SetAPIError("EnumDisplayDevices");
    else
        ret = PyWinObject_FromDISPLAY_DEVICE(&display_device);
    PyWinObject_FreeTCHAR(Device);
    return ret;
}

// @pymethod <o PyDEVMODE>|win32api|EnumDisplaySettings|List available modes for specified display device
// @comm Accepts keyword arguments
PyObject *PyEnumDisplaySettings(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"DeviceName", "ModeNum", NULL};
    TCHAR *DeviceName = NULL;
    PyObject *obDeviceName = Py_None, *ret = NULL;
    DWORD ModeNum = 0;
    DEVMODE devmode;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|Ok:EnumDisplaySettings", keywords,
            &obDeviceName,  // @pyparm string|DeviceName|None|Name of device as returned by <om
                            // win32api.EnumDisplayDevices>, use None for default display device
            &ModeNum))      // @pyparm int|ModeNum|0|Index of setting to return, or one of ENUM_CURRENT_SETTINGS,
                            // ENUM_REGISTRY_SETTINGS
        return NULL;
    if (!PyWinObject_AsTCHAR(obDeviceName, &DeviceName, TRUE))
        return NULL;
    ZeroMemory(&devmode, sizeof(DEVMODE));
    devmode.dmSize = sizeof(DEVMODE);
    if (!EnumDisplaySettings(DeviceName, ModeNum, &devmode))
        // msdn says GetLastError should return something on win2k and up, I get 0
        PyWin_SetAPIError("EnumDisplaySettings");
    else
        ret = PyWinObject_FromDEVMODE(&devmode);
    PyWinObject_FreeTCHAR(DeviceName);
    return ret;
}

// @pymethod <o PyDEVMODE>|win32api|EnumDisplaySettingsEx|Lists available modes for a display device, with optional
// flags
// @comm Accepts keyword arguments
PyObject *PyEnumDisplaySettingsEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(EnumDisplaySettingsEx);
    static char *keywords[] = {"DeviceName", "ModeNum", "Flags", NULL};
    TCHAR *DeviceName = NULL;
    PyObject *obDeviceName = Py_None, *ret = NULL;
    DWORD ModeNum = 0;
    DEVMODE devmode;
    DWORD Flags = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Okk:EnumDisplaySettingsEx", keywords,
                                     &obDeviceName,  // @pyparm string|DeviceName|None|Name of device as returned by <om
                                                     // win32api.EnumDisplayDevices>. Can be None for default display
                                     &ModeNum,       // @pyparm int|ModeNum||Index of setting to return, or one of
                                                     // ENUM_CURRENT_SETTINGS, ENUM_REGISTRY_SETTINGS
                                     &Flags))        // @pyparm int|Flags|0|EDS_RAWMODE (2) is only defined flag
        return NULL;
    if (!PyWinObject_AsTCHAR(obDeviceName, &DeviceName, TRUE))
        return NULL;
    ZeroMemory(&devmode, sizeof(DEVMODE));
    devmode.dmSize = sizeof(DEVMODE);
    if (!(*pfnEnumDisplaySettingsEx)(DeviceName, ModeNum, &devmode, Flags))
        PyWin_SetAPIError("EnumDisplaySettingsEx");
    else
        ret = PyWinObject_FromDEVMODE(&devmode);
    PyWinObject_FreeTCHAR(DeviceName);
    return ret;
}

BOOL CALLBACK EnumDisplayMonitors_Callback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    PyObject *ret = Py_BuildValue("O&O&O&", PyWinObject_FromHANDLE, hMonitor, PyWinObject_FromHANDLE, hdcMonitor,
                                  PyWinObject_FromRECT, lprcMonitor);
    if (ret == NULL)
        return FALSE;
    if (PyList_Append((PyObject *)dwData, ret) == -1) {
        Py_DECREF(ret);
        return FALSE;
    }
    Py_DECREF(ret);
    return TRUE;
}

// @pymethod list|win32api|EnumDisplayMonitors|Lists display monitors for a given device context and area
// @rdesc Returns a sequence of tuples.  For each monitor found, returns a handle to the monitor,
// device context handle, and intersection rectangle: (hMonitor, hdcMonitor, <o PyRECT>)
// @comm Accepts keyword arguments
PyObject *PyEnumDisplayMonitors(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(EnumDisplayMonitors);
    static char *keywords[] = {"hdc", "rcClip", NULL};
    HDC hdc = NULL;
    RECT rect;
    LPRECT prect;
    PyObject *obhdc = Py_None, *obrect = Py_None;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|OO:EnumDisplayMonitors", keywords,
            &obhdc,    // @pyparm <o PyHANDLE>|hdc|None|Handle to device context, use None for virtual desktop
            &obrect))  // @pyparm <o PyRECT>|rcClip|None|Clipping rectangle, can be None
        return NULL;
    if (!PyWinObject_AsHANDLE(obhdc, (HANDLE *)&hdc))
        return NULL;
    if (obrect == Py_None)
        prect = NULL;
    else {
        if (!PyWinObject_AsRECT(obrect, &rect))
            return NULL;
        prect = &rect;
    }

    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    if (!(*pfnEnumDisplayMonitors)(hdc, prect, EnumDisplayMonitors_Callback, (LPARAM)ret)) {
        Py_DECREF(ret);
        return NULL;
    }
    return ret;
}

// @pymethod dict|win32api|GetMonitorInfo|Retrieves information for a monitor by handle
// @rdesc Returns a dictionary representing a MONITORINFOEX structure
// @comm Accepts keyword args
PyObject *PyGetMonitorInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(GetMonitorInfo);
    static char *keywords[] = {"hMonitor", NULL};
    PyObject *obhMonitor;
    HMONITOR hMonitor;
    MONITORINFOEX mi;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetMonitorInfo", keywords,
                                     &obhMonitor))  // @pyparm <o PyHANDLE>|hMonitor||Handle to a monitor
        return NULL;
    if (!PyWinObject_AsHANDLE(obhMonitor, (HANDLE *)&hMonitor))
        return NULL;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (!(*pfnGetMonitorInfo)(hMonitor, &mi)) {
        PyWin_SetAPIError("GetMonitorInfo");
        return NULL;
    }
    return Py_BuildValue("{s:O&,s:O&,s:k,s:N}", "Monitor", PyWinObject_FromRECT, &mi.rcMonitor, "Work",
                         PyWinObject_FromRECT, &mi.rcWork, "Flags", mi.dwFlags, "Device",
                         PyWinObject_FromTCHAR(mi.szDevice));
}

// @pymethod <o PyHANDLE>|win32api|MonitorFromPoint|Finds monitor that contains a point
// @comm Accepts keyword arguments
// @rdesc Returns None if no monitor was found
PyObject *PyMonitorFromPoint(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(MonitorFromPoint);
    static char *keywords[] = {"pt", "Flags", NULL};
    DWORD Flags = 0;
    HMONITOR hmonitor;
    PyObject *obpoint;
    POINT point;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|k:MonitorFromPoint", keywords,
            &obpoint,  // @pyparm (int, int)|pt||Tuple of 2 ints (x,y) specifying screen coordinates
            &Flags))   // @pyparm int|Flags|0|Flags that determine default behaviour, one of
                       // MONITOR_DEFAULTTONEAREST,MONITOR_DEFAULTTONULL,MONITOR_DEFAULTTOPRIMARY
        return NULL;
    if (!PyWinObject_AsPOINT(obpoint, &point))
        return NULL;
    hmonitor = (*pfnMonitorFromPoint)(point, Flags);
    if (hmonitor == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWinObject_FromHANDLE(hmonitor);
}

// @pymethod <o PyHANDLE>|win32api|MonitorFromRect|Finds monitor that has largest intersection with a rectangle
// @comm Accepts keyword arguments
// @rdesc Returns None if no monitor was found
PyObject *PyMonitorFromRect(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(MonitorFromRect);
    static char *keywords[] = {"rc", "Flags", NULL};
    DWORD Flags = 0;
    RECT rect;
    HMONITOR hmonitor;
    PyObject *obrect;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|k:MonitorFromRect", keywords,
            &obrect,  // @pyparm <o PyRECT>|rc||Rectangle to be examined
            &Flags))  // @pyparm int|Flags|0|Flags that determine default behaviour, one of
                      // MONITOR_DEFAULTTONEAREST,MONITOR_DEFAULTTONULL,MONITOR_DEFAULTTOPRIMARY
        return NULL;
    if (!PyWinObject_AsRECT(obrect, &rect))
        return NULL;
    hmonitor = (*pfnMonitorFromRect)(&rect, Flags);
    if (hmonitor == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWinObject_FromHANDLE(hmonitor);
}

// @pymethod <o PyHANDLE>|win32api|MonitorFromWindow|Finds monitor that contains a window
// @comm Accepts keyword arguments
// @rdesc Returns None if no monitor was found
PyObject *PyMonitorFromWindow(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(MonitorFromWindow);
    static char *keywords[] = {"hwnd", "Flags", NULL};
    DWORD Flags = 0;
    HWND hwnd;
    HMONITOR hmonitor;
    PyObject *obhwnd;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|k:MonitorFromWindow", keywords,
            &obhwnd,  // @pyparm <o PyHANDLE>|hwnd||Handle to a window
            &Flags))  // @pyparm int|Flags|0|Flags that determine default behaviour, one of
                      // MONITOR_DEFAULTTONEAREST,MONITOR_DEFAULTTONULL,MONITOR_DEFAULTTOPRIMARY
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    hmonitor = (*pfnMonitorFromWindow)(hwnd, Flags);
    if (hmonitor == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWinObject_FromHANDLE(hmonitor);
}
