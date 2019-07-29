#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);

// Macro to allow loading the correct ANSI/wide-character version of a function from a .dll
#ifdef UNICODE
#define A_OR_W "W"
#else
#define A_OR_W "A"
#endif

PyObject *PyChangeDisplaySettings(PyObject *self, PyObject *args);
PyObject *PyChangeDisplaySettingsEx(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyEnumDisplayDevices(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyEnumDisplayMonitors(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyEnumDisplaySettings(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyEnumDisplaySettingsEx(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyGetMonitorInfo(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyMonitorFromPoint(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyMonitorFromRect(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyMonitorFromWindow(PyObject *self, PyObject *args, PyObject *kwargs);

// from user32.dll
typedef LONG(WINAPI *ChangeDisplaySettingsExfunc)(LPCTSTR, LPDEVMODE, HWND, DWORD, LPVOID);
extern ChangeDisplaySettingsExfunc pfnChangeDisplaySettingsEx;
typedef BOOL(WINAPI *EnumDisplayDevicesfunc)(LPCTSTR, DWORD, PDISPLAY_DEVICE, DWORD);
extern EnumDisplayDevicesfunc pfnEnumDisplayDevices;
typedef BOOL(WINAPI *EnumDisplayMonitorsfunc)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
extern EnumDisplayMonitorsfunc pfnEnumDisplayMonitors;
typedef HMONITOR(WINAPI *MonitorFromWindowfunc)(HWND, DWORD);
extern MonitorFromWindowfunc pfnMonitorFromWindow;
typedef HMONITOR(WINAPI *MonitorFromRectfunc)(LPCRECT, DWORD);
extern MonitorFromRectfunc pfnMonitorFromRect;
typedef HMONITOR(WINAPI *MonitorFromPointfunc)(POINT, DWORD);
extern MonitorFromPointfunc pfnMonitorFromPoint;
typedef BOOL(WINAPI *GetMonitorInfofunc)(HMONITOR, LPMONITORINFOEX);
extern GetMonitorInfofunc pfnGetMonitorInfo;
typedef BOOL(WINAPI *EnumDisplaySettingsExfunc)(LPCTSTR, DWORD, LPDEVMODE, DWORD);
extern EnumDisplaySettingsExfunc pfnEnumDisplaySettingsEx;

extern __declspec(dllexport) PyTypeObject PyDISPLAY_DEVICEType;
extern PyObject *PyWinObject_FromDISPLAY_DEVICE(PDISPLAY_DEVICE);

class PyDISPLAY_DEVICE : public PyObject {
   public:
    static struct PyMemberDef members[];
    static struct PyMethodDef methods[];
    static void deallocFunc(PyObject *ob);
    PyDISPLAY_DEVICE(PDISPLAY_DEVICE);
    PyDISPLAY_DEVICE(void);
    static PyObject *getattro(PyObject *self, PyObject *name);
    static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
    static PyObject *Clear(PyObject *self, PyObject *args);
    static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
    PDISPLAY_DEVICE GetDISPLAY_DEVICE(void);
    PyObject *obdummy;

   protected:
    DISPLAY_DEVICE display_device;
    ~PyDISPLAY_DEVICE();
};
