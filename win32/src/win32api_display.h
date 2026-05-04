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
