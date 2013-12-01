/* File : PyIConverterSession.i */

%module IConverterSession

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%typemap(python,ignore) IConverterSession **OUTPUT(IConverterSession *temp)
{
  $target = &temp;
}
%typemap(python,argout) IConverterSession **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IConverterSession)
}
%typemap(python,freearg) IConverterSession *INPUT,
			 IConverterSession *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(python,in) IConverterSession *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IConverterSession, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IConverterSession *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IConverterSession, (void **)&$target, 1))
		return NULL;
}		

%{
#include <MapiUtil.h>
#include <initguid.h>
#include "IConverterSession.h"
#include "PyIConverterSession.h"

#include "PYIStream.h"
#include "PYIMapiProp.h"
#include "PYIMessage.h"

PyIConverterSession::PyIConverterSession(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

/*static*/ IConverterSession *PyIConverterSession::GetI(PyObject *self)
{
	return (IConverterSession *)PyIUnknown::GetI(self);
}

PyIConverterSession::~PyIConverterSession()
{
}

PyObject *PyIConverterSession::MIMEToMAPI(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *result = NULL;
	PyObject *obStream;
	PyObject *obMsg;
	unsigned long flags;
	
	IConverterSession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "OO|l:MIMEToMAPI", &obStream, &obMsg, &flags))
		return NULL;

	IStream *pStream = NULL;
	IMessage *pMsg = NULL;
		
	if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
		goto done;
	if (!PyCom_InterfaceFromPyObject(obMsg, IID_IMessage, (void **)&pMsg, FALSE))
		goto done;
		
	PY_INTERFACE_PRECALL;
	hRes = _swig_self->MIMEToMAPI(pStream, pMsg, NULL, flags);
	PY_INTERFACE_POSTCALL;
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
	{
		Py_INCREF(Py_None);
		result = Py_None;
	}
	
done:
	if (pStream)
		pStream->Release();
	if (pMsg)
		pMsg->Release();
		
	return result;
}

PyObject *PyIConverterSession::MAPIToMIMEStm(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *result = NULL;
	PyObject *obStream;
	PyObject *obMsg;
	unsigned long flags;
	
	IConverterSession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "OO|l:MAPIToMIMEStm", &obMsg, &obStream, &flags))
		return NULL;

	IStream *pStream = NULL;
	IMessage *pMsg = NULL;

	if (!PyCom_InterfaceFromPyObject(obMsg, IID_IMessage, (void **)&pMsg, FALSE))
		goto done;	
	if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
		goto done;
		
	PY_INTERFACE_PRECALL;
	hRes = _swig_self->MAPIToMIMEStm(pMsg, pStream, flags);
	PY_INTERFACE_POSTCALL;
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
	{
		Py_INCREF(Py_None);
		result = Py_None;
	}
		
done:
	if (pStream)
		pStream->Release();
	if (pMsg)
		pMsg->Release();
		
	return result;
}

PyObject *PyIConverterSession::SetAdrBook(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *result = NULL;
	PyObject *obIAddrBook;

	IConverterSession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "O:SetAdrBook", &obIAddrBook))
		return NULL;
	
	IAddrBook *pAddrBook = NULL;
	
	if (obIAddrBook != Py_None && !PyCom_InterfaceFromPyObject(obIAddrBook, IID_IAddrBook, (void **)&pAddrBook, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	hRes = _swig_self->SetAdrBook(pAddrBook);
	PY_INTERFACE_POSTCALL;
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
	{
		Py_INCREF(Py_None);
		result = Py_None;
	}
	
	if (pAddrBook)
		pAddrBook->Release();
		
	return result;
}
%}

%native(MIMEToMAPI) MIMEToMAPI;
%native(MAPIToMIMEStm) MAPIToMIMEStm;
%native(SetAdrBook) SetAdrBook;
