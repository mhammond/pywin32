/***********************************************************************

_win32sysloader.cpp -- a helper module for loading the
PyWin32 "system" modules pywintypes and pythoncom.

Note that this module does (and must) *NOT* link to pywintypesXX.dll

See pywintypes.py for more information.

********************************************************************/
#include "windows.h"
// Windows rpc.h defines "small" as "char" and Python 3.x's accu.h uses
// "small" as a structure element causing compilation errors :(
#ifdef small
#undef small
#endif
#include "Python.h"

// GetModuleHandle and GetModuleFilename rolled into 1
static PyObject *PyGetModuleFilename(PyObject *self, PyObject *args)
{
	// For py3k, will be built with UNICODE defined
#ifdef UNICODE
	static char *fmt="u";
#else
	static char *fmt="s";
#endif

	TCHAR *modName=NULL;
    if (!PyArg_ParseTuple(args, fmt, &modName))
        return NULL;
    HINSTANCE hinst = GetModuleHandle(modName);
	if (hinst == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    TCHAR buf[_MAX_PATH];
    if (GetModuleFileName(hinst, buf, sizeof(buf)/sizeof(buf[0]))==0) {
        Py_INCREF(Py_None);
        return Py_None;
    }
#ifdef UNICODE
    return PyUnicode_FromUnicode(buf, wcslen(buf));
#else
	return PyString_FromString(buf);
#endif
}

static PyObject *PyLoadModule(PyObject *self, PyObject *args)
{
#ifdef UNICODE
	static char *fmt="u";
#else
	static char *fmt="s";
#endif
    TCHAR *modName=NULL;
    if (!PyArg_ParseTuple(args, fmt, &modName))
        return NULL;
    HINSTANCE hinst = LoadLibrary(modName);

    if (hinst == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    TCHAR buf[_MAX_PATH];
    if (GetModuleFileName(hinst, buf, sizeof(buf)/sizeof(buf[0]))==0) {
        Py_INCREF(Py_None);
        return Py_None;
    }
#ifdef UNICODE
    return PyUnicode_FromUnicode(buf, wcslen(buf));
#else
	return PyString_FromString(buf);
#endif
}

static struct PyMethodDef functions[] = {
    {"GetModuleFilename",   PyGetModuleFilename,1},
    {"LoadModule",          PyLoadModule,1},
    { NULL }
};

extern "C" __declspec(dllexport)
#if (PY_VERSION_HEX < 0x03000000)
void init_win32sysloader(void)
{
	PyObject *module=Py_InitModule("_win32sysloader", functions);
}
#else
PyObject *PyInit__win32sysloader(void)
{
	static PyModuleDef _win32sysloader_def = {
		PyModuleDef_HEAD_INIT,
		"_win32sysloader",
		"Exists only to load Pywin32 system modules",
		-1,
		functions
		};
	PyObject *module=PyModule_Create(&_win32sysloader_def);
	return module;
}
#endif

