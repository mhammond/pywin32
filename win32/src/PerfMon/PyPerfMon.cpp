/***********************************************************

PerfMonModule.cpp -- implementation of Performance Monitor module


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "PyWinTypes.h"
#include "Pyperfmon.h"
#include "loadperf.h"

extern PyObject *PerfmonMethod_NewPERF_COUNTER_DEFINITION(PyObject *self, PyObject *args);
extern PyObject *PerfmonMethod_NewPERF_OBJECT_TYPE(PyObject *self, PyObject *args);
extern PyObject *PerfmonMethod_NewPerfMonManager(PyObject *self, PyObject *args);

// @pymethod |perfmon|LoadPerfCounterTextStrings|
PyObject *PyLoadPerfCounterTextStrings(PyObject *self, PyObject *args)
{
    BOOL bQuiet = 1;
    char *cmdLine;

    if (!PyArg_ParseTuple(args, "s|i:LoadPerfCounterTextStrings", &cmdLine, &bQuiet))
        return NULL;

    LONG rc = LoadPerfCounterTextStringsA(cmdLine, 1);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("LoadPerfCounterTextStrings", rc);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |perfmon|UnloadPerfCounterTextStrings|
PyObject *PyUnloadPerfCounterTextStrings(PyObject *self, PyObject *args)
{
    BOOL bQuiet = 1;
    char *cmdLine;
    if (!PyArg_ParseTuple(args, "s|i:UnloadPerfCounterTextStrings", &cmdLine, &bQuiet))
        return NULL;

    LONG rc = UnloadPerfCounterTextStringsA(cmdLine, 1);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("UnloadPerfCounterTextStrings", rc);
    Py_INCREF(Py_None);
    return Py_None;
}

/* List of functions exported by this module */
// @module perfmon|A module which wraps Performance Monitor functions.
static struct PyMethodDef perfmon_functions[] = {
    {"LoadPerfCounterTextStrings", PyLoadPerfCounterTextStrings, 1},      // @pymeth LoadPerfCounterTextStrings|
    {"UnloadPerfCounterTextStrings", PyUnloadPerfCounterTextStrings, 1},  // @pymeth UnloadPerfCounterTextStrings|
    {"CounterDefinition", PerfmonMethod_NewPERF_COUNTER_DEFINITION,
     1},  // @pymeth CounterDefinition|Creates a new <o PyPERF_COUNTER_DEFINITION> object
    {"ObjectType", PerfmonMethod_NewPERF_OBJECT_TYPE,
     1},  // @pymeth ObjectType|Creates a new <o PyPERF_OBJECT_TYPE> object
    {"PerfMonManager", PerfmonMethod_NewPerfMonManager,
     1},  // @pymeth PerfMonManager|Creates a new <o PyPerfMonManager> objects>
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(perfmon)
{
    PYWIN_MODULE_INIT_PREPARE(perfmon, perfmon_functions,
                              "Contains functions and objects wrapping the Performance Monitor APIs");
    if (PyType_Ready(&PyPerfMonManager::type) == -1 || PyType_Ready(&PyPERF_COUNTER_DEFINITION::type) == -1 ||
        PyType_Ready(&PyPERF_OBJECT_TYPE::type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
