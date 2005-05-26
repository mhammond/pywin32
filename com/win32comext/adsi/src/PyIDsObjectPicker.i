%module IDsObjectPicker // A COM interface to ADSI's IDsObjectPicker interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%apply HWND {long};
typedef long HWND

%{

#include "Objsel.h"
#include "PyIDsObjectPicker.h"

extern BOOL PyObject_AsDSOP_SCOPE_INIT_INFOs(PyObject *ob, DSOP_SCOPE_INIT_INFO**p, ULONG *n);

static void PyWinObject_FreeWCHARArray(LPWSTR *wchars, DWORD str_cnt)
{
	if (wchars!=NULL){
		for (DWORD wchar_index=0; wchar_index<str_cnt; wchar_index++)
			PyWinObject_FreeWCHAR(wchars[wchar_index]);
		free(wchars);
		}
}

static BOOL PyWinObject_AsWCHARArray(PyObject *str_seq, LPWSTR **wchars, DWORD *str_cnt, BOOL bNoneOK = FALSE)
{
    if (bNoneOK && str_seq==Py_None) {
        *wchars = NULL;
        *str_cnt = 0;
        return TRUE;
    }
	BOOL ret=FALSE;
	PyObject *str_tuple=NULL, *tuple_item;
	DWORD bufsize, tuple_index;
	*wchars=NULL;
	*str_cnt=0;
	if ((str_tuple=PySequence_Tuple(str_seq))==NULL)
		return FALSE;
	*str_cnt=PyTuple_Size(str_tuple);
	bufsize=*str_cnt * sizeof(LPWSTR);
	*wchars=(LPWSTR *)malloc(bufsize);
	if (*wchars==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		goto done;
		}
	ZeroMemory(*wchars, bufsize);
	for (tuple_index=0;tuple_index<*str_cnt;tuple_index++){
		tuple_item=PyTuple_GET_ITEM(str_tuple, tuple_index);
		if (!PyWinObject_AsWCHAR(tuple_item, &((*wchars)[tuple_index]), FALSE)){
			PyWinObject_FreeWCHARArray(*wchars, *str_cnt);
			*wchars=NULL;
			*str_cnt=0;
			goto done;
			}
		}
	ret=TRUE;
done:
	Py_XDECREF(str_tuple);
	return ret;
}

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
