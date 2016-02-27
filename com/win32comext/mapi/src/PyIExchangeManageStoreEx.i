/* File : PyIExchangeManageStoreEx.i */

%module IExchangeManageStoreEx

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%typemap(python,ignore) IExchangeManageStoreEx **OUTPUT(IExchangeManageStoreEx *temp)
{
  $target = &temp;
}
%typemap(python,argout) IExchangeManageStoreEx **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IExchangeManageStoreEx)
}
%typemap(python,freearg) IExchangeManageStoreEx *INPUT,
			 IExchangeManageStoreEx *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(python,in) IExchangeManageStoreEx *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IExchangeManageStoreEx, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IExchangeManageStoreEx *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IExchangeManageStoreEx, (void **)&$target, 1))
		return NULL;
}		

%{
#include <initguid.h>
#include "IExchangeManageStoreEx.h"
#include "PyIExchangeManageStoreEx.h"

PyIExchangeManageStoreEx::PyIExchangeManageStoreEx(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

IExchangeManageStoreEx *PyIExchangeManageStoreEx::GetI(PyObject *self)
{
	return (IExchangeManageStoreEx *)PyIUnknown::GetI(self);
}

PyIExchangeManageStoreEx::~PyIExchangeManageStoreEx() {}
%}

/*
** See http://blogs.msdn.com/b/dvespa/archive/2014/01/15/new-mapi-interface-ignore-home-server.aspx
*/

%native(CreateStoreEntryID2) CreateStoreEntryID2;
%{
PyObject *PyIExchangeManageStoreEx::CreateStoreEntryID2(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obs = NULL;
	ULONG flags = 0;
	SPropValue *pPV;
	ULONG seqLen;
	SBinary sbEID = {0, NULL};

	IExchangeManageStoreEx *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;

	if (!PyArg_ParseTuple(args, "O|l:CreateStoreEntryID2", &obs, &flags))
		return NULL;
	
	if (!PySequence_Check(obs))
	{
		PyErr_SetString(PyExc_TypeError, "Properties must be a sequence of tuples");
		return NULL;
	}

	if (!PyMAPIObject_AsSPropValueArray(obs, &pPV, &seqLen))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->CreateStoreEntryID2(seqLen, pPV, flags, &sbEID.cb, (LPENTRYID *) &sbEID.lpb);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
								"y#",
#else
								"s#",
#endif
								sbEID.lpb, sbEID.cb);
								
	MAPIFreeBuffer((LPENTRYID)sbEID.lpb);
	MAPIFreeBuffer(pPV);
	
	return result;
}
%}
