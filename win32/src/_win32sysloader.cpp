/***********************************************************************

_win32sysloader.cpp -- a helper module for loading the
PyWin32 "system" modules pywintypes and pythoncom.

Note that this module does (and must) *NOT* link to pywintypesXX.dll

See pywintypes.py for more information.

********************************************************************/
#include "windows.h"
// Windows rpc.h defines "small" as "char" and Python 3's accu.h uses
// "small" as a structure element causing compilation errors :(
#ifdef small
#undef small
#endif
#include "Python.h"

// GetModuleHandle and GetModuleFilename rolled into 1
static PyObject *PyGetModuleFilename(PyObject *self, PyObject *args)
{
    PyObject *nameobj;
    if (!PyArg_ParseTuple(args, "U", &nameobj))
        return NULL;

    TCHAR *modName = PyUnicode_AsWideCharString(nameobj, NULL);
    if (!modName)
        return NULL;

    HINSTANCE hinst = GetModuleHandle(modName);
    PyMem_Free(modName);
    if (hinst == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    TCHAR buf[_MAX_PATH];
    if (GetModuleFileName(hinst, buf, sizeof(buf) / sizeof(buf[0])) == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyUnicode_FromWideChar(buf, wcslen(buf));
}

static PyObject *PyLoadModule(PyObject *self, PyObject *args)
{
    PyObject *nameobj;
    if (!PyArg_ParseTuple(args, "U", &nameobj))
        return NULL;

    TCHAR *modName = PyUnicode_AsWideCharString(nameobj, NULL);
    if (!modName)
        return NULL;

    HINSTANCE hinst = LoadLibraryEx(modName, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    PyMem_Free(modName);
    if (hinst == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    TCHAR buf[_MAX_PATH];
    if (GetModuleFileName(hinst, buf, sizeof(buf) / sizeof(buf[0])) == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyUnicode_FromWideChar(buf, wcslen(buf));
}

static struct PyMethodDef functions[] = {
    {"GetModuleFilename", PyGetModuleFilename, 1}, {"LoadModule", PyLoadModule, 1}, {NULL}};

extern "C" __declspec(dllexport) PyObject *PyInit__win32sysloader(void)
{
    static PyModuleDef _win32sysloader_def = {PyModuleDef_HEAD_INIT, "_win32sysloader",
                                              "Exists only to load Pywin32 system modules", -1, functions};
    PyObject *module = PyModule_Create(&_win32sysloader_def);
    return module;
}
