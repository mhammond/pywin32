/* File : PyIDistList.i */

%module IDistList // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIContainer.h"
#include "PyIDistList.h"

PyIDistList::PyIDistList(IUnknown *pDisp) :
	PyIMAPIContainer(pDisp)
{
	ob_type = &type;
}

PyIDistList::~PyIDistList()
{
}

/*static*/ IDistList *PyIDistList::GetI(PyObject *self)
{
	return (IDistList *)PyIUnknown::GetI(self);
}


%}
