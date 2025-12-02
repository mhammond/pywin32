%{
#include "PythonCOM.h"

#define SWIG_PYTHONCOM
%}

typedef long HRESULT;	// This will raise COM Exception.
%{
typedef long HRESULT_KEEP; // This will keep HRESULT, and return
%}
typedef long FLAGS;


%typemap(out) HRESULT {
	$result = Py_None;
	Py_INCREF(Py_None);
}

%typemap(except) HRESULT {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($1))  {
           $cleanup
#ifdef SWIG_THIS_IID
           return PyCom_BuildPyException($1, _swig_self,  SWIG_THIS_IID);
#else
           return PyCom_BuildPyException($1);
#endif
      }
}

// HRESULT_KEEP_INFO will raise an exception on failure,
// but still return the hresult to the caller
//typedef long HRESULT_KEEP_INFO;
%{
typedef long HRESULT_KEEP_INFO;
%}

%typemap(out) HRESULT_KEEP_INFO {
	$result = PyLong_FromLong($1);
}

%typemap(except) HRESULT_KEEP_INFO {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($1))  {
           $cleanup
#ifdef SWIG_THIS_IID
           return PyCom_BuildPyException($1, _swig_self,  SWIG_THIS_IID);
#else
           return PyCom_BuildPyException($1);
#endif
      }
}

%typemap(in) IID *INPUT(IID temp)
{
	$1 = &temp;
	if (!PyWinObject_AsIID($input, $1))
		return NULL;
}

%typemap(in) IID *INPUT_NULLOK(IID temp)
{
	if ($input==Py_None)
		$1 = NULL;
	else {
		$1 = &temp;
		if (!PyWinObject_AsIID($input, $1))
			return NULL;
	}
}

%typemap(ignore) IUnknown **OUTPUT(IUnknown *temp)
{
  $1 = &temp;
}
%typemap(ignore) IDispatch **OUTPUT(IDispatch *temp)
{
  $1 = &temp;
}

%{
#define MAKE_OUTPUT_INTERFACE(source, target, iid) \
{ \
    PyObject *o; \
    o = PyCom_PyObjectFromIUnknown(*source, iid, FALSE /* bAddRef */); \
    if (target==NULL) \
      target = o; \
	else if (target == Py_None) { /* has been incref'd already!*/ \
		Py_DECREF(Py_None); \
		target = o; \
    } else { \
      if (!PyList_Check(target)) { \
        PyObject *o2 = target; \
        target = PyList_New(0); \
        PyList_Append(target,o2); \
        Py_XDECREF(o2); \
      } \
      PyList_Append(target,o); \
      Py_XDECREF(o); \
    } \
}
%}

%typemap(argout) IUnknown **OUTPUT {
	MAKE_OUTPUT_INTERFACE($1, $result, IID_IUnknown)
//	$result = PyCom_PyObjectFromIUnknown(*$1, IID_IUnknown, FALSE /* bAddRef */);
}

%typemap(in) IUnknown *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($input, IID_IUnknown, (void **)&$1, 0))
		return NULL;
}

%typemap(in) IUnknown *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($input, IID_IUnknown, (void **)&$1, 1))
		return NULL;
}

%typemap(argout) IDispatch **OUTPUT {
	MAKE_OUTPUT_INTERFACE($1, $result, IID_IDispatch)
}

%typemap(freearg) IUnknown *INPUT,
                         IUnknown *INPUT_NULLOK,
                         IMessage *INPUT,
                         IMessage *INPUT_NULLOK,
                         IMAPITable *INPUT,
                         IMAPITable *INPUT_NULLOK,
						 IMAPISession *INPUT,
						 IMAPISession *INPUT_NULLOK,
						 IMAPIFolder *INPUT,
						 IMAPIFolder *INPUT_NULLOK,
						 IMAPIProp *INPUT,
						 IMAPIProp *INPUT_NULLOK,
						 IMAPIProgress *INPUT,
						 IMAPIProgress *INPUT_NULLOK,
						 IAttach *INPUT,
						 IAttach *INPUT_NULLOK
{
	if ($1) $1->Release();
}

%typemap(arginit) IUnknown *,
                         IMAPISession *,
                         IMAPITable *,
                         IMAPIFolder *,
                         IMessage *,
                         IMAPIProp *,
						 IMAPIProgress *,
						 IAttach *
{
	$1 = NULL;
}

// Variants!
// SWIG only does this funky stuff for pointers :-(
%typemap(ignore) VARIANT *OUTPUT( VARIANT temp)
{
  $1 = &temp;
}

%typemap(argout) VARIANT *OUTPUT {
    PyObject *o;
    o = PyCom_PyObjectFromVariant($1);
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
