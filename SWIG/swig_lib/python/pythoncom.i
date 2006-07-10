%{
#include "PythonCOM.h"

#define SWIG_PYTHONCOM
%}

typedef long HRESULT;	// This will raise COM Exception.
%typedef long HRESULT_KEEP; // This will keep HRESULT, and return
typedef long FLAGS;


%typemap(python,out) HRESULT {
	$target = Py_None;
	Py_INCREF(Py_None);
}

%typemap(python,except) HRESULT {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($source))  {
           $cleanup
#ifdef SWIG_THIS_IID
           return PyCom_BuildPyException($source, _swig_self,  SWIG_THIS_IID);
#else
           return PyCom_BuildPyException($source);
#endif
      }
}

// HRESULT_KEEP_INFO will raise an exception on failure,
// but still return the hresult to the caller
//typedef long HRESULT_KEEP_INFO;
%typedef long HRESULT_KEEP_INFO;

%typemap(python,out) HRESULT_KEEP_INFO {
	$target = PyInt_FromLong($source);
}

%typemap(python,except) HRESULT_KEEP_INFO {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($source))  {
           $cleanup
#ifdef SWIG_THIS_IID
           return PyCom_BuildPyException($source, _swig_self,  SWIG_THIS_IID);
#else
           return PyCom_BuildPyException($source);
#endif
      }
}

%typemap(python,in) IID *INPUT(IID temp)
{
	$target = &temp;
	if (!PyWinObject_AsIID($source, $target))
		return NULL;
}

%typemap(python,in) IID *INPUT_NULLOK(IID temp)
{
	if ($source==Py_None)
		$target = NULL;
	else {
		$target = &temp;
		if (!PyWinObject_AsIID($source, $target))
			return NULL;
	}
}

%typemap(python,ignore) IUnknown **OUTPUT(IUnknown *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IDispatch **OUTPUT(IDispatch *temp)
{
  $target = &temp;
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

%typemap(python,argout) IUnknown **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IUnknown)
//	$target = PyCom_PyObjectFromIUnknown(*$source, IID_IUnknown, FALSE /* bAddRef */);
}

%typemap(python,in) IUnknown *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IUnknown, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IUnknown *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IUnknown, (void **)&$target, 1))
		return NULL;
}

%typemap(python,argout) IDispatch **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IDispatch)
}

%typemap(python,freearg) IUnknown *INPUT,
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
	if ($source) $source->Release();
}

%typemap(python,arginit) IUnknown *,
                         IMAPISession *,
                         IMAPITable *,
                         IMAPIFolder *,
                         IMessage *,
                         IMAPIProp *,
						 IMAPIProgress *,
						 IAttach *
{
	$target = NULL;
}

// Variants!
// SWIG only does this funky stuff for pointers :-(
%typemap(python,ignore) VARIANT *OUTPUT( VARIANT temp)
{
  $target = &temp;
}

%typemap(python,argout) VARIANT *OUTPUT {
    PyObject *o;
    o = PyCom_PyObjectFromVariant($source);
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
