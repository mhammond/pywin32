/* File : PyIABContainer.i */

%module IABContainer // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIContainer.h"
#include "PyIABContainer.h"

PyIABContainer::PyIABContainer(IUnknown *pDisp) :
	PyIMAPIContainer(pDisp)
{
	ob_type = &type;
}

PyIABContainer::~PyIABContainer()
{
}

/*static*/ IABContainer *PyIABContainer::GetI(PyObject *self)
{
	return (IABContainer *)PyIUnknown::GetI(self);
}


%}
