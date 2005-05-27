%module IADsDeleteOps // A COM interface to ADSI's IADsDeleteOps interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{

#include "PyIADsDeleteOps.h"
#define SWIG_THIS_IID IID_IADsDeleteOps

PyIADsDeleteOps::PyIADsDeleteOps(IUnknown *pDisp) :
	PyIDispatch(pDisp)
{
	ob_type = &type;
}

PyIADsDeleteOps::~PyIADsDeleteOps()
{
}

IADsDeleteOps *PyIADsDeleteOps::GetI(PyObject *self)
{
	return (IADsDeleteOps *)PyIDispatch::GetI(self);
}

%}

// @pyswig |DeleteObject|
// @pyswig int|flags|0|
HRESULT DeleteObject(LONG flags = 0);
