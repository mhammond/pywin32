// -*- Mode: C++; tab-width: 4 -*-
// $Id$
//
// A simple interface to win32 Timers
//
// Author: Sam Rushing <rushing@nightmare.com>
//

#include "windows.h"
#include "Python.h"
#include "pywintypes.h"
//#include "abstract.h"

static PyObject *timer_module_error;
static PyObject *timer_id_callback_map = NULL;

VOID CALLBACK
py_win32_timer_callback (HWND hwnd, UINT msg, UINT event, DWORD time)
{
  // do we have a valid callback dictionary?
  
  if (timer_id_callback_map) {
	CEnterLeavePython _celp;
	PyObject * py_event = Py_BuildValue ("i", (int) event);
	
    // is this timer id recognized?
	PyObject * callback_function = \
	  PyDict_GetItem (timer_id_callback_map, py_event);

	// call the user's function
	if (callback_function) {
	  // create a 'death grip' on the callback function, just incase
	  // the callback itself removes the function from the map.
	  Py_INCREF(callback_function);
	  PyObject * callback_args = Py_BuildValue ("(il)", (int) event, (long) time);
	  PyObject * result = \
		PyEval_CallObject (callback_function, callback_args);

	  if (!result) {
		// Is this necessary, or will python already have flagged
		// an exception?  Can we even catch exceptions here?
		PyErr_Print();
	  }

	  // everything's ok, return
	  Py_DECREF(callback_function);
	  Py_XDECREF(callback_args);
	  Py_XDECREF(result);
	  Py_DECREF (py_event);
	  return;
	}
	// invalid key or callback: kill the timer (note there is no
	// key to remove - we have already determined it is not there!)
	Py_DECREF (py_event);
	::KillTimer (NULL, event);
	return;
  } else {
	// the id/callback map is NULL
	::KillTimer (NULL, event);
  }
}

static PyObject * 
py_timer_set_timer (PyObject * self, PyObject * args)
{
  PyObject *callback;
  PyObject * py_timer_id;
  int elapse;
  UINT timer_id;
  
  if (!PyArg_ParseTuple (args, "iO", &elapse, &callback)) {
    return NULL;
  }
  
  // make sure the callback is a valid callable object
  if (!PyCallable_Check (callback)) {
    PyErr_SetString (timer_module_error, "argument must be a callable object");
    return NULL;
  }

  // create the win32 timer
  Py_BEGIN_ALLOW_THREADS;
  timer_id = ::SetTimer (NULL, 0, (UINT) elapse, (TIMERPROC) py_win32_timer_callback);
  Py_END_ALLOW_THREADS;

  if (!timer_id) {
    PyErr_SetString (timer_module_error, "win32 SetTimer() failed");
    return NULL;
  }

  py_timer_id = PyInt_FromLong((long) timer_id);
  if (!py_timer_id)
	  return NULL;

  // associate the timer id with the given callback function
  if (PyObject_SetItem (timer_id_callback_map,
						py_timer_id,
						callback) == -1) {
    ::KillTimer (NULL, timer_id);
	Py_DECREF(py_timer_id);
    PyErr_SetString (timer_module_error,
					 "internal error, couldn't set timer id callback item");
    return NULL;
  }
  // everything went ok.
  return py_timer_id;
}

static PyObject *
py_timer_kill_timer (PyObject * self, PyObject * args)
{
  PyObject * py_timer_id;

  if (!PyArg_ParseTuple (args, "O", &py_timer_id)) {
	return NULL;
  } else if (timer_id_callback_map) {
	  if (0 != PyDict_DelItem (timer_id_callback_map, py_timer_id)) {
		  return NULL;
	  }
  }
  int rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = ::KillTimer (NULL, (int) PyInt_AsLong (py_timer_id));
  Py_END_ALLOW_THREADS;
  return Py_BuildValue ("i", rc);  
}

#ifdef _DEBUG
static PyObject *
py_timer_timer_map (PyObject * self, PyObject * args)
{
  if (!PyArg_ParseTuple (args, "")) {
	return NULL;
  }
  Py_INCREF (timer_id_callback_map);
  return (timer_id_callback_map);
}
#endif

// List of functions exported by this module
static struct PyMethodDef timer_functions[] = {
  {"set_timer",		py_timer_set_timer,		1},
  {"kill_timer",	py_timer_kill_timer,	1},
#ifdef _DEBUG
  {"_id_timer_map",	py_timer_timer_map,		1},
#endif
  {NULL,			NULL}
};

extern"C" __declspec(dllexport) void
inittimer(void)
{
  PyObject *dict, *module;
  module = Py_InitModule("timer", timer_functions);
  if (!module) /* Eeek - some serious error! */
    return;
  dict = PyModule_GetDict(module);
  if (!dict) return; /* Another serious error!*/
  timer_module_error = PyString_FromString("timer error");
  PyDict_SetItemString(dict, "error", timer_module_error);
  PyDict_SetItemString(dict, "__version__", PyString_FromString("0.2"));
  timer_id_callback_map = PyDict_New();
}
