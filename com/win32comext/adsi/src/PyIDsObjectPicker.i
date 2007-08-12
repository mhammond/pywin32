%module IDsObjectPicker // A COM interface to ADSI's IDsObjectPicker interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{

#include "Objsel.h"
#include "PyIDsObjectPicker.h"

extern BOOL PyObject_AsDSOP_SCOPE_INIT_INFOs(PyObject *ob, DSOP_SCOPE_INIT_INFO**p, ULONG *n);

#define SWIG_THIS_IID IID_IDsObjectPicker

PyIDsObjectPicker::PyIDsObjectPicker(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIDsObjectPicker::~PyIDsObjectPicker()
{
}

IDsObjectPicker *PyIDsObjectPicker::GetI(PyObject *self)
{
	return (IDsObjectPicker *)PyIUnknown::GetI(self);
}

// @pyswig |Initialize|Initializes the IDsObjectPicker interface with information about the scopes, filters, and options used by the object picker dialog box.
PyObject *PyIDsObjectPicker::Initialize(PyObject *self, PyObject *args)
{
    HRESULT hr;
    PyObject *ret;
	PyObject *obTargetComputer;
    PyObject *obScopeInfos;
	IDsObjectPicker *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    PyObject *obAttributeNames = Py_None;
    DSOP_INIT_INFO ii;
    memset(&ii, sizeof(ii), 0);
    ii.cbSize = sizeof(ii);
	if (!PyArg_ParseTuple(args, "OO|lO:Initialize",
                                     &obTargetComputer, // @pyparm <o PyUnicode>|targetComputer||
                                     &obScopeInfos, // @pyparm <o PyDSOP_SCOPE_INIT_INFOs>|scopeInfos||
                                     &ii.flOptions, // @pyparm int|options|0|
                                     &obAttributeNames)) // @pyparm [<o PyUnicode>, ...]|attrNames|None|
        return NULL;
    if (!PyWinObject_AsWCHAR(obTargetComputer, (WCHAR **)&ii.pwzTargetComputer, TRUE))
		goto done;
    if (!PyObject_AsDSOP_SCOPE_INIT_INFOs(obScopeInfos, &ii.aDsScopeInfos, &ii.cDsScopeInfos))
        goto done;
    if (!PyWinObject_AsWCHARArray(obAttributeNames, (WCHAR ***)&ii.apwzAttributeNames, &ii.cAttributesToFetch, TRUE))
        goto done;
    Py_BEGIN_ALLOW_THREADS
    hr = _swig_self->Initialize(&ii);
    Py_END_ALLOW_THREADS
    if (FAILED(hr))
		PyCom_BuildPyException(hr, _swig_self, IID_IDsObjectPicker);
    else {
        ret = Py_None;
        Py_INCREF(ret);
    }
done:
    PyWinObject_FreeWCHAR((WCHAR *)ii.pwzTargetComputer);
    PyWinObject_FreeWCHARArray((WCHAR **)ii.apwzAttributeNames, ii.cAttributesToFetch);
    return ret;
}
%}

%native(Initialize) Initialize;

%typemap(python,ignore) IDataObject  **OUTPUT(IDataObject *temp)
{
  $target = &temp;
}
%typemap(python,argout) IDataObject **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IDataObject)
}

// @pyswig <o PyIDataObject>|InvokeDialog|Displays a modal object picker dialog box and returns the user's selections.
// @pyparm int|hwnd||
HRESULT InvokeDialog(HWND hwnd, IDataObject **OUTPUT);
