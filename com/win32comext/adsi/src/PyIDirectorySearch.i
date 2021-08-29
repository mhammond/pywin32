// @doc
%module IDirectorySearch // A COM interface to ADSI's IDirectorySearch interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{
#include "AdsErr.h"
#include "PyIDirectorySearch.h"

#define SWIG_THIS_IID IID_IDirectorySearch

PyIDirectorySearch::PyIDirectorySearch(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIDirectorySearch::~PyIDirectorySearch()
{
}

IDirectorySearch *PyIDirectorySearch::GetI(PyObject *self)
{
	return (IDirectorySearch *)PyIUnknown::GetI(self);
}

%}

%{

// @pyswig int, [int, ...]|SetSearchPreference|
// @rdesc The result is the hresult of the call, and a list of integer status
// codes for each of the preferences set.
PyObject *PyIDirectorySearch::SetSearchPreference(PyObject *self, PyObject *args)
{
	HRESULT _result;
	PyObject *obPrefs;
	IDirectorySearch *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm ADS_SEARCHPREF_INFO|prefs||
	if (!PyArg_ParseTuple(args, "O", &obPrefs))
		return NULL;
    ADS_SEARCHPREF_INFO *p;
    DWORD numPrefs, i;
    if (!PyADSIObject_AsADS_SEARCHPREF_INFOs(obPrefs, &p, &numPrefs))
        return NULL;
    PyObject *retStatus = PyList_New(numPrefs);
    if (!retStatus) {
        PyADSIObject_FreeADS_SEARCHPREF_INFOs(p, numPrefs);
        return NULL;
    }
	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->SetSearchPreference(p, numPrefs);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
    for (i=0;i<numPrefs;i++)
        PyList_SET_ITEM(retStatus, i, PyLong_FromLong(p[i].dwStatus));
    PyADSIObject_FreeADS_SEARCHPREF_INFOs(p, numPrefs);
    return Py_BuildValue("iN", _result, retStatus);
}

%}
%native(SetSearchPreference) SetSearchPreference;

%{

// @pyswig int|ExecuteSearch|Executes a search and passes the results to the caller.
// Some providers, such as LDAP, will defer the actual execution until the caller invokes the
// <om PyIDirectorySearch.GetFirstRow> method or the <om PyIDirectorySearch.GetNextRow> method.
// @rdesc The result is an integer search handle.  <om PyIDirectorySearch.CloseSearchHandle>
// should be called to close the handle.
PyObject *PyIDirectorySearch::ExecuteSearch(PyObject *self, PyObject *args)
{
	PyObject *obNames, *obFilter;
	IDirectorySearch *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm <o PyUnicode>|filter||
	// @pyparm [<o PyUnicode>, ...]|attrNames||
	if (!PyArg_ParseTuple(args, "OO", &obFilter, &obNames))
		return NULL;
    WCHAR *szFilter = NULL;
    if (!PyWinObject_AsWCHAR(obFilter, &szFilter, FALSE))
        return NULL;

	WCHAR **names = NULL;
	DWORD cnames = -1;
	if (obNames != Py_None)
		if (!PyADSI_MakeNames(obNames, &names, &cnames)) {
            PyWinObject_FreeWCHAR(szFilter);
			return NULL;
        }

	HRESULT _result;
    ADS_SEARCH_HANDLE handle;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->ExecuteSearch(szFilter, names, cnames, &handle);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (FAILED(_result))
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	else {
        ret = PyLong_FromSsize_t((Py_ssize_t)handle);
	} 
	PyADSI_FreeNames(names, cnames);
    PyWinObject_FreeWCHAR(szFilter);
	return ret;
}

%}
%native(ExecuteSearch) ExecuteSearch;

%{
// @pyswig int|GetNextRow|
// @pyparm int|handle||
// @rdesc The result is the HRESULT from the call - no exceptions are thrown
PyObject *PyIDirectorySearch::GetNextRow(PyObject *self, PyObject *args) {
    HRESULT_KEEP_INFO  _result;
    ADS_SEARCH_HANDLE  _arg0;

    IDirectorySearch *_swig_self;
    if ((_swig_self=GetI(self))==NULL) return NULL;
    if (!PyArg_ParseTuple(args,"n:GetNextRow",&_arg0))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT_KEEP_INFO )_swig_self->GetNextRow(_arg0);
    Py_END_ALLOW_THREADS

    if (FAILED(_result)) {
        return OleSetADSIError(_result, _swig_self,  SWIG_THIS_IID);
    }
    return PyLong_FromLong(_result);
}
%}
%native(GetNextRow) GetNextRow;

%{
// @pyswig int|GetFirstRow|
// @pyparm int|handle||
// @rdesc The result is the HRESULT from the call - no exceptions are thrown
PyObject *PyIDirectorySearch::GetFirstRow(PyObject *self, PyObject *args) {
    HRESULT_KEEP_INFO  _result;
    ADS_SEARCH_HANDLE  _arg0;

    IDirectorySearch *_swig_self;
    if ((_swig_self=GetI(self))==NULL) return NULL;
    if (!PyArg_ParseTuple(args,"n:GetFirstRow",&_arg0))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT_KEEP_INFO )_swig_self->GetFirstRow(_arg0);
    Py_END_ALLOW_THREADS

    if (FAILED(_result)) {
        return OleSetADSIError(_result, _swig_self,  SWIG_THIS_IID);
    }
    return PyLong_FromLong(_result);
}
%}
%native(GetFirstRow) GetFirstRow;

%{
// @pyswig int|GetPreviousRow|
// @pyparm int|handle||
// @rdesc The result is the HRESULT from the call - no exceptions are thrown
PyObject *PyIDirectorySearch::GetPreviousRow(PyObject *self, PyObject *args) {
    HRESULT_KEEP_INFO  _result;
    ADS_SEARCH_HANDLE  _arg0;

    IDirectorySearch *_swig_self;
    if ((_swig_self=GetI(self))==NULL) return NULL;
    if (!PyArg_ParseTuple(args,"n:GetPreviousRow",&_arg0))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT_KEEP_INFO )_swig_self->GetPreviousRow(_arg0);
    Py_END_ALLOW_THREADS

    if (FAILED(_result)) {
        return OleSetADSIError(_result, _swig_self,  SWIG_THIS_IID);
    }
    return PyLong_FromLong(_result);
}
%}
%native(GetPreviousRow) GetPreviousRow;

%{
// @pyswig |CloseSearchHandle|Closes a previously opened search handle.
// @pyparm int|handle||
PyObject *PyIDirectorySearch::CloseSearchHandle(PyObject *self, PyObject *args) {
    HRESULT  _result;
    ADS_SEARCH_HANDLE  _arg0;

    IDirectorySearch *_swig_self;
      if ((_swig_self=GetI(self))==NULL) return NULL;
    if (!PyArg_ParseTuple(args,"n:CloseSearchHandle",&_arg0))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT )_swig_self->CloseSearchHandle(_arg0);
    Py_END_ALLOW_THREADS

    if (FAILED(_result)) {
        return OleSetADSIError(_result, _swig_self, SWIG_THIS_IID);
    }
    Py_INCREF(Py_None);
    return Py_None;
}
%}
%native(CloseSearchHandle) CloseSearchHandle;

%{
// @pyswig |AdandonSearch|
// @pyparm int|handle||
PyObject *PyIDirectorySearch::AbandonSearch(PyObject *self, PyObject *args) {
    HRESULT  _result;
    ADS_SEARCH_HANDLE  _arg0;

    IDirectorySearch *_swig_self;
    if ((_swig_self=GetI(self))==NULL) return NULL;
    if (!PyArg_ParseTuple(args,"n:AbandonSearch",&_arg0))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT )_swig_self->AbandonSearch(_arg0);
    Py_END_ALLOW_THREADS

    if (FAILED(_result))  {
        return OleSetADSIError(_result, _swig_self,  SWIG_THIS_IID);
    }
    Py_INCREF(Py_None);
    return Py_None;
}
%}
%native(AbandonSearch) AbandonSearch;

%{
// @pyswig (name, type, values)|GetColumn|
PyObject *PyIDirectorySearch::GetColumn(PyObject *self, PyObject *args)
{
	PyObject *obName;
    Py_ssize_t handle;
	IDirectorySearch *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm int|handle||Handle to a search
	// @pyparm <o PyUnicode>|name||The column name to fetch
	if (!PyArg_ParseTuple(args, "nO:GetColumn", &handle, &obName))
		return NULL;
    WCHAR *szName= NULL;
    if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
        return NULL;

    ADS_SEARCH_COLUMN col;
    memset(&col, 0, sizeof(col));
	HRESULT _result;
    PyObject *ret = NULL;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->GetColumn((ADS_SEARCH_HANDLE)handle, szName, &col);
	Py_END_ALLOW_THREADS
	if (FAILED(_result))
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	else {
        PyObject *values = PyList_New(col.dwNumValues);
        if (values) {
            DWORD i;
            for (i=0;i<col.dwNumValues;i++) {
                PyList_SET_ITEM(values, i, PyADSIObject_FromADSVALUE(col.pADsValues[i]));
            }
            ret = Py_BuildValue("NiN", PyWinObject_FromWCHAR(col.pszAttrName), col.dwADsType, values);
        }
		_swig_self->FreeColumn(&col);
	}
    PyWinObject_FreeWCHAR(szName);
	return ret;
}
%}
%native(GetColumn) GetColumn;

%{
// @pyswig |GetNextColumnName|
// @rdesc Returns None when the underlying ADSI function return S_ADS_NOMORE_COLUMNS.
PyObject *PyIDirectorySearch::GetNextColumnName(PyObject *self, PyObject *args)
{
    Py_ssize_t handle;
	IDirectorySearch *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	if (!PyArg_ParseTuple(args, "n:GetNextColumnName", &handle))
		return NULL;
	HRESULT _result;
    PyObject *ret = NULL;
	WCHAR *szName = NULL;
	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->GetNextColumnName((ADS_SEARCH_HANDLE)handle, &szName);
	Py_END_ALLOW_THREADS
	if (FAILED(_result))
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	else if (_result == S_ADS_NOMORE_COLUMNS) {
		ret = Py_None;
		Py_INCREF(ret);
	} else {
		ret = PyWinObject_FromWCHAR(szName);
		FreeADsMem(szName);
	}
	return ret;
}
%}
%native(GetNextColumnName) GetNextColumnName;
