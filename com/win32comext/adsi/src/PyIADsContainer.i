%module IADsContainer // A COM interface to ADSI's IADsContainer interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{

#include "PyIADsContainer.h"
#define SWIG_THIS_IID IID_IADsContainer

PyIADsContainer::PyIADsContainer(IUnknown *pDisp) :
	PyIDispatch(pDisp)
{
	ob_type = &type;
}

PyIADsContainer::~PyIADsContainer()
{
}

IADsContainer *PyIADsContainer::GetI(PyObject *self)
{
	return (IADsContainer *)PyIDispatch::GetI(self);
}

%}

// @pyswig <o PyIDispatch>|GetObject|
// @pyparm string|class||Specifies the name of the object class as known in the underlying directory and identical to the one retrieved through the get_Class property method. If the class name is None, the provider returns the first item found in the container.
// @pyparm string|relativeName||Specifies the name of the object as known in the underlying directory and identical to the one retrieved through the get_Name property method.
HRESULT GetObject(WCHAR *INPUT_NULLOK, WCHAR *INPUT_NULLOK, IDispatch **OUTPUT);

// @pyswig int|get_Count|
HRESULT get_Count(long *OUTPUT);

// @pyswig object|get_Filter|
HRESULT get_Filter(VARIANT *OUTPUT);

%{
#define MAKE_PUT_VARIANT(name) \
	PyObject *PyIADsContainer::put_##name(PyObject *self, PyObject *args) { \
	VARIANT var; \
	HRESULT _result; \
	IADsContainer *_swig_self; \
	PyObject *_obj0; \
	if ((_swig_self=GetI(self))==NULL) return NULL; \
	if(!PyArg_ParseTuple(args,"O:put_Filter",&_obj0))  \
		return NULL; \
	VariantInit(&var); \
	if (!PyCom_VariantFromPyObject(_obj0, &var)) \
		return NULL; \
	Py_BEGIN_ALLOW_THREADS \
	_result = (HRESULT )_swig_self->put_##name(var); \
	Py_END_ALLOW_THREADS \
	VariantClear(&var); \
	if (FAILED(_result)) \
           return OleSetADSIError(_result, _swig_self, SWIG_THIS_IID); \
	Py_INCREF(Py_None); \
	return Py_None; \
}

// @pyswig |put_Filter|
// @pyparm object|val||

MAKE_PUT_VARIANT(Filter)
%}

%native (put_Filter) put_Filter;


// @pyswig object|get_Hints|
HRESULT get_Hints(VARIANT *OUTPUT);
%{
MAKE_PUT_VARIANT(Hints)
// @pyswig |put_Hints|
// @pyparm object|val||
%}
%native (put_Hints) put_Hints;
