/* File : PyIExchangeManageStore.i */

%module IExchangeManageStore

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{
#include <edkmdb.h>
#define INITGUID
#include <initguid.h>
#define USES_IID_IExchangeManageStore
#include <edkguid.h>
%}

%typemap(python,ignore) IExchangeManageStore **OUTPUT(IExchangeManageStore *temp)
{
  $target = &temp;
}
%typemap(python,argout) IExchangeManageStore **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IExchangeManageStore)
}
%typemap(python,freearg) IExchangeManageStore *INPUT,
			 IExchangeManageStore *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(python,in) IExchangeManageStore *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IExchangeManageStore, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IExchangeManageStore *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IExchangeManageStore, (void **)&$target, 1))
		return NULL;
}		

%{

#include "PyIExchangeManageStore.h"

PyIExchangeManageStore::PyIExchangeManageStore(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

/*static*/ IExchangeManageStore *PyIExchangeManageStore::GetI(PyObject *self)
{
	return (IExchangeManageStore *)PyIUnknown::GetI(self);
}

PyIExchangeManageStore::~PyIExchangeManageStore()
{
}

PyObject *PyIExchangeManageStore::CreateStoreEntryID(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	char *serverDN;
	char *userDN;
	unsigned long flags = 0;
	SBinary sbEID = {0, NULL};	
	PyObject *result = NULL;

	IExchangeManageStore *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;

	if (!PyArg_ParseTuple(args, "ss|l:CreateStoreEntryID", &serverDN, &userDN, &flags))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->CreateStoreEntryID(serverDN, userDN, flags, &sbEID.cb, (LPENTRYID *) &sbEID.lpb);
	Py_END_ALLOW_THREADS

	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue("s#", sbEID.lpb, sbEID.cb);

	MAPIFreeBuffer((LPENTRYID)sbEID.lpb);
	
	return result;
}

%}

%native(CreateStoreEntryID) CreateStoreEntryID;
