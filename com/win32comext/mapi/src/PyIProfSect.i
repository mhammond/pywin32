/* File : PyIProfSect.i */

%module IProfSect  // An interface for accessing profile sections

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIProfSect.h"

PyIProfSect::PyIProfSect(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIProfSect::~PyIProfSect()
{
}

/*static*/ IProfSect *PyIProfSect::GetI(PyObject *self)
{
	return (IProfSect *)PyIUnknown::GetI(self);
}


%}
