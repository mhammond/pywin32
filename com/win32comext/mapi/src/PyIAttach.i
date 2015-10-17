/* File : PyIAttach.i */

%module IAttach // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIAttach.h"

PyIAttach::PyIAttach(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIAttach::~PyIAttach()
{
}

/*static*/ IAttach *PyIAttach::GetI(PyObject *self)
{
	return (IAttach *)PyIUnknown::GetI(self);
}


%}

%native(GetLastError) GetLastError;
%{
// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
PyObject *PyIAttach::GetLastError(PyObject *self, PyObject *args)
{
	HRESULT hr, hRes;
	ULONG flags = 0;
	MAPIERROR *me = NULL;
	
	IAttach *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
    if(!PyArg_ParseTuple(args,"l|l:GetLastError",
		&hr, // @pyparm int|hr||Contains the error code generated in the previous method call.
		&flags)) // @pyparm int|flags||Indicates for format for the output.
        return NULL;
		
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->GetLastError(hr, flags, &me);
	Py_END_ALLOW_THREADS

	if (FAILED(hRes))
		return OleSetOleError(hRes);
	
	if (me == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyObject_FromMAPIERROR(me, flags & MAPI_UNICODE, TRUE);
}
%}
