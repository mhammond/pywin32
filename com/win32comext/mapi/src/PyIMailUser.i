/* File : PyIMailUser.i */

%module IMailUser // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIContainer.h"
#include "PyIMailUser.h"

PyIMailUser::PyIMailUser(IUnknown *pDisp) :
	PyIMAPIContainer(pDisp)
{
	ob_type = &type;
}

PyIMailUser::~PyIMailUser()
{
}

/*static*/ IMailUser *PyIMailUser::GetI(PyObject *self)
{
	return (IMailUser *)PyIUnknown::GetI(self);
}


%}
