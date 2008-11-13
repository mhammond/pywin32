/***********************************************************************

_win32sysloader.cpp -- a helper module for loading the
PyWin32 "system" modules pywintypes and pythoncom.

Note that this module does (and must) *NOT* link to pywintypesXX.dll

See pywintypes.py for more information.

********************************************************************/
#include "windows.h"
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

extern "C" __declspec(dllexport) void
init_win32sysloader(void)
{
    Py_InitModule("_win32sysloader", functions);
}
