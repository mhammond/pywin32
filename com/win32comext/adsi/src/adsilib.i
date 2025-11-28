/* ADSI SWIG Support */

%{
#include "pyadsiutil.h"

extern PyObject *OleSetADSIError(HRESULT hr, IUnknown *pUnk, REFIID iid);
%}

// Custom error handling for ADSI.
%typemap(except) HRESULT {
	Py_BEGIN_ALLOW_THREADS
	$function
	Py_END_ALLOW_THREADS
	if (FAILED($source))  {
		$cleanup

#ifndef SWIG_THIS_IID
#error This interface must have SWIG_THIS_IID defined!
#endif
		return OleSetADSIError($source, _swig_self,  SWIG_THIS_IID);
	}
}

%typemap(except) HRESULT_KEEP_INFO {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($source))  {
           $cleanup
           return OleSetADSIError($source, _swig_self,  SWIG_THIS_IID);
      }
}

%typemap(ignore) IDirectoryObject **OUTPUT(IDirectoryObject *temp)
{
  $target = &temp;
}
%typemap(argout) IDirectoryObject **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IDirectoryObject)
}


%typemap(freearg) IDirectoryObject *,
                         IDirectoryObject *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(in) IDirectoryObject * {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IDirectoryObject, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IDirectoryObject *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IDirectoryObject, (void **)&$target, 1))
		return NULL;
}

%apply ADS_SEARCH_HANDLE {long};
typedef long ADS_SEARCH_HANDLE

// The types and structures.

%typemap(ignore) ADS_OBJECT_INFO **OUTPUT (ADS_OBJECT_INFO *temp) {
	$target = &temp;
	*$target = NULL;
}

%typemap(argout) ADS_OBJECT_INFO **OUTPUT {
	PyObject *o;
	o = PyADSIObject_FromADS_OBJECT_INFO(*$source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}
%typemap(freearg) ADS_OBJECT_INFO **OUTPUT {
	if (*$source) FreeADsMem(*$source);
}

/*************************

%typemap(in) ADS_OBJECT_INFO * {
	if (!PyADSIObject_AsADS_OBJECT_INFO($source, &$target, FALSE))
		return NULL;
}

%typemap(in) ADS_OBJECT_INFO *INPUT_NULLOK {
	if (!PyMAPIObject_AsADS_OBJECT_INFO($source, &$target, TRUE))
		return NULL;
}

%typemap(freearg) ADS_OBJECT_INFO *, ADS_OBJECT_INFO *INPUT_NULLOK {
	if ($source) PyMAPIObject_FreeADS_OBJECT_INFO($source);
}

%typemap(in) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *INPUT;
%typemap(freearg) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *INPUT;
%typemap(argout) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *OUTPUT;

************/

%typemap(argout) ADS_OBJECT_INFO *OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromADS_OBJECT_INFO($source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}

%typemap(freearg) ADS_OBJECT_INFO *OUTPUT {
	if ($source) FreeADsMem($source);
}
