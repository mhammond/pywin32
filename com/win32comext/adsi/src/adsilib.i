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
	if (FAILED($1))  {
		$cleanup
#ifndef SWIG_THIS_IID
#error This interface must have SWIG_THIS_IID defined!
#endif
		return OleSetADSIError($1, _swig_self,  SWIG_THIS_IID);
	}
}

%typemap(except) HRESULT_KEEP_INFO {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($1))  {
           $cleanup
           return OleSetADSIError($1, _swig_self,  SWIG_THIS_IID);
      }
}

%typemap(ignore) IDirectoryObject **OUTPUT(IDirectoryObject *temp)
{
  $1 = &temp;
}
%typemap(argout) IDirectoryObject **OUTPUT {
	MAKE_OUTPUT_INTERFACE($inout, $result, IID_IDirectoryObject)
}


%typemap(freearg) IDirectoryObject *,
                         IDirectoryObject *INPUT_NULLOK
{
	if ($1) $1->Release();
}

%typemap(in) IDirectoryObject * {
	if (!PyCom_InterfaceFromPyInstanceOrObject($inout, IID_IDirectoryObject, (void **)&$1, 0))
		return NULL;
}
%typemap(in) IDirectoryObject *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($inout, IID_IDirectoryObject, (void **)&$1, 1))
		return NULL;
}

%apply ADS_SEARCH_HANDLE {long};
typedef long ADS_SEARCH_HANDLE;

// The types and structures.

%typemap(ignore) ADS_OBJECT_INFO **OUTPUT (ADS_OBJECT_INFO *temp){
	$1 = &temp;
	*$1 = NULL;
}

%typemap(argout) ADS_OBJECT_INFO **OUTPUT {
	PyObject *o;
	o = PyADSIObject_FromADS_OBJECT_INFO(*$1);
	if (!$result) {
		$result = o;
	} else if ($result == Py_None) {
		Py_DECREF(Py_None);
		$result = o;
	} else {
		if (!PyList_Check($result)) {
			PyObject *o2 = $result;
			$result = PyList_New(0);
			PyList_Append($result,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($result,o);
		Py_XDECREF(o);
	}
}
%typemap(freearg) ADS_OBJECT_INFO **OUTPUT {
	if (*$1) FreeADsMem(*$1);
}

/*************************

%typemap(in) ADS_OBJECT_INFO * {
	if (!PyADSIObject_AsADS_OBJECT_INFO($inout, &$1, FALSE))
		return NULL;
}

%typemap(in) ADS_OBJECT_INFO *INPUT_NULLOK {
	if (!PyMAPIObject_AsADS_OBJECT_INFO($inout, &$1, TRUE))
		return NULL;
}

%typemap(freearg) ADS_OBJECT_INFO *, ADS_OBJECT_INFO *INPUT_NULLOK {
	if ($1) PyMAPIObject_FreeADS_OBJECT_INFO($1);
}

%typemap(in) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *INPUT;
%typemap(freearg) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *INPUT;
%typemap(argout) ADS_OBJECT_INFO *BOTH = ADS_OBJECT_INFO *OUTPUT;

************/

%typemap(argout) ADS_OBJECT_INFO *OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromADS_OBJECT_INFO($1);
	if (!$result) {
		$result = o;
	} else if ($result == Py_None) {
		Py_DECREF(Py_None);
		$result = o;
	} else {
		if (!PyList_Check($result)) {
			PyObject *o2 = $result;
			$result = PyList_New(0);
			PyList_Append($result,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($result,o);
		Py_XDECREF(o);
	}
}

%typemap(freearg) ADS_OBJECT_INFO *OUTPUT {
	if ($1) FreeADsMem($1);
}
