// -*- Mode: C++; tab-width: 4 -*-
// $Id$
//
// A simple interface to win32 Timers
//
// Author: Sam Rushing <rushing@nightmare.com>
//

// @doc - Contains autoduck comments for documentation

#include "pywintypes.h"
// #include "abstract.h"

static PyObject *timer_id_callback_map = NULL;

VOID CALLBACK py_win32_timer_callback(HWND hwnd, UINT msg, UINT_PTR timer_id, DWORD time)
{
    CEnterLeavePython _celp;
    PyObject *py_timer_id = PyWinLong_FromVoidPtr((void *)timer_id);
    if (!py_timer_id) {
        PyErr_Print();
        return;
    }
    // is this timer id recognized?
    PyObject *callback_function = PyDict_GetItem(timer_id_callback_map, py_timer_id);
    if (!callback_function) {
        ::KillTimer(NULL, timer_id);
        PyErr_Warn(PyExc_RuntimeWarning, "Unrecognized timer id");
        Py_DECREF(py_timer_id);
        return;
    }
    // call the user's function
    // create a 'death grip' on the callback function, just incase
    // the callback itself removes the function from the map.
    Py_INCREF(callback_function);
    PyObject *callback_args = Py_BuildValue("(Ok)", py_timer_id, time);
    PyObject *result = PyObject_CallObject(callback_function, callback_args);

    if (!result) {
        // Is this necessary, or will python already have flagged
        // an exception?  Can we even catch exceptions here?
        PyErr_Print();
    }

    // everything's ok, return
    Py_DECREF(callback_function);
    Py_XDECREF(callback_args);
    Py_XDECREF(result);
    Py_DECREF(py_timer_id);
    return;
}

// @pymethod int|timer|set_timer|Creates a timer that executes a callback function
// @rdesc Returns the id of the timer, which can be passed to kill_timer to stop it.
// @comm Uses the SetTimer function.
static PyObject *py_timer_set_timer(PyObject *self, PyObject *args)
{
    PyObject *callback;
    PyObject *py_timer_id;
    UINT elapse;
    UINT_PTR timer_id;

    if (!PyArg_ParseTuple(args, "kO:set_timer",
                          &elapse,     // @pyparm int|Elapse||Timer period, in milliseconds
                          &callback))  // @pyparm function|TimerFunc||Callback function.  Will be called with with 2 int
                                       // args: (timer_id, time)
        return NULL;
    // make sure the callback is a valid callable object
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a callable object");
        return NULL;
    }

    // create the win32 timer
    Py_BEGIN_ALLOW_THREADS;
    timer_id = ::SetTimer(NULL, 0, elapse, py_win32_timer_callback);
    Py_END_ALLOW_THREADS;

    if (!timer_id)
        return PyWin_SetAPIError("SetTimer");
    py_timer_id = PyWinLong_FromVoidPtr((void *)timer_id);
    if (!py_timer_id) {
        ::KillTimer(NULL, timer_id);
        return NULL;
    }

    // associate the timer id with the given callback function
    if (PyDict_SetItem(timer_id_callback_map, py_timer_id, callback) == -1) {
        ::KillTimer(NULL, timer_id);
        Py_DECREF(py_timer_id);
        return NULL;
    }
    // everything went ok.
    return py_timer_id;
}

// @pymethod boolean|timer|kill_timer|Creates a timer that executes a callback function
// @comm Uses the KillTimer API function.
static PyObject *py_timer_kill_timer(PyObject *self, PyObject *args)
{
    PyObject *py_timer_id;
    UINT_PTR timer_id;
    if (!PyArg_ParseTuple(args, "O:kill_timer",
                          &py_timer_id))  // @pyparm int|IDEvent||Timer id as returned by <om timer.set_timer>
        return NULL;
    if (!PyWinLong_AsVoidPtr(py_timer_id, (void **)&timer_id))
        return NULL;
    if (timer_id_callback_map)
        if (0 != PyDict_DelItem(timer_id_callback_map, py_timer_id))
            return NULL;

    BOOL rc;
    Py_BEGIN_ALLOW_THREADS;
    rc = ::KillTimer(NULL, timer_id);
    Py_END_ALLOW_THREADS;
    return PyBool_FromLong(rc);
}

#ifdef _DEBUG
static PyObject *py_timer_timer_map(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_INCREF(timer_id_callback_map);
    return (timer_id_callback_map);
}
#endif

// List of functions exported by this module
// @module timer|Extension that wraps Win32 Timer functions
static struct PyMethodDef timer_functions[] = {
    // @pymeth set_timer|Creates a timer that executes a callback function
    {"set_timer", py_timer_set_timer, METH_VARARGS,
     "int = set_timer(milliseconds, callback}\nCreates a timer that executes a callback function"},
    // @pymeth kill_timer|Stops a timer
    {"kill_timer", py_timer_kill_timer, METH_VARARGS, "boolean = kill_timer(timer_id)\nStops a timer"},
#ifdef _DEBUG
    {"_id_timer_map", py_timer_timer_map, 1},
#endif
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(timer)
{
    PYWIN_MODULE_INIT_PREPARE(timer, timer_functions, "Extension that wraps Win32 Timer functions");

    timer_id_callback_map = PyDict_New();
    if (!timer_id_callback_map)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyDict_SetItemString(dict, "error", PyWinExc_ApiError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "__version__", PyBytes_FromString("0.2")) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
