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


// GetLastError|Returns a MAPIERROR structure containing information about the previous error on the table. 
HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

