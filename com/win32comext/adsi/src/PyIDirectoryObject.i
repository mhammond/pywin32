%module IDirectoryObject // A COM interface to ADSI's IDirectoryObject interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{

#include "PyIDirectoryObject.h"

#define SWIG_THIS_IID IID_IDirectoryObject

PyIDirectoryObject::PyIDirectoryObject(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIDirectoryObject::~PyIDirectoryObject()
{
}

IDirectoryObject *PyIDirectoryObject::GetI(PyObject *self)
{
	return (IDirectoryObject *)PyIUnknown::GetI(self);
}

%}

// @pyswig <o PyADS_OBJECT_INFO>|GetObjectInformation|Retrieves an <o PyADS_OBJECT_INFO> object that contains information about the identity and location of a directory service object.
HRESULT GetObjectInformation(
	ADS_OBJECT_INFO **OUTPUT);

%{

// @pyswig (<o PyADS_ATTR_INFO>, ...)|GetObjectAttributes|Gets one or more specified attributes of the directory service object, as defined in the <o PyADS_ATTR_INFO> structure.
PyObject *PyIDirectoryObject::GetObjectAttributes(PyObject *self, PyObject *args)
{
	PyObject *obNames;
	IDirectoryObject *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm (<o PyUnicode>, ...)|names||
	if (!PyArg_ParseTuple(args, "O", &obNames))
		return NULL;
	WCHAR **names = NULL;
	DWORD cnames = -1;
	if (obNames != Py_None)
		if (!PyADSI_MakeNames(obNames, &names, &cnames))
			return NULL;

	PADS_ATTR_INFO attrs;
	DWORD cattrs;
	HRESULT _result;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->GetObjectAttributes(names, cnames, &attrs, &cattrs);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (FAILED(_result))
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	else if (attrs==NULL) {
		Py_INCREF(Py_None);
		ret = Py_None;
	} else
		ret = PyADSIObject_FromADS_ATTR_INFOs(attrs, cattrs);
	PyADSI_FreeNames(names, cnames);
	FreeADsMem(attrs);
	return ret;
}

%}
%native(GetObjectAttributes) GetObjectAttributes;

%{
// @pyswig int|SetObjectAttributes|Sets one or more specified attributes of the directory service object, as defined in the <o PyADS_ATTR_INFO> structure.
PyObject *PyIDirectoryObject::SetObjectAttributes(PyObject *self, PyObject *args)
{
	HRESULT _result;
	IDirectoryObject *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	PyObject *obAttr;
	// @pyparm (<o PyADS_ATTR_INFO>, ...)|attrs||The attributes to set
	if (!PyArg_ParseTuple(args, "O", &obAttr))
		return NULL;

	PADS_ATTR_INFO attr;
	DWORD cattr;
	if (!PyADSIObject_AsADS_ATTR_INFOs(obAttr, &attr, &cattr))
		return NULL;

	DWORD numset;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->SetObjectAttributes(attr, cattr, &numset);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (FAILED(_result)) {
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	} else
		ret = PyInt_FromLong(numset);
	PyADSIObject_FreeADS_ATTR_INFOs(attr, cattr);
	return ret;
};

%}

%native(SetObjectAttributes) SetObjectAttributes;

%{
// @pyswig <o PyIDispatch>|CreateDSObject|
PyObject *PyIDirectoryObject::CreateDSObject(PyObject *self, PyObject *args)
{
	HRESULT _result;
	IDirectoryObject *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	PyObject *obAttr, *obName;
	// @pyparm <o PyUnicode>|rdn||The relative distinguished name (relative path) of the object to be created.
	// @pyparm (<o PyADS_ATTR_INFO>, ...)|attrs||The attributes to set.
	if (!PyArg_ParseTuple(args, "OO:CreateDSObject", &obName, &obAttr))
		return NULL;

    WCHAR *szName= NULL;
    if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
        return NULL;

	PADS_ATTR_INFO attr;
	DWORD cattr;
	if (!PyADSIObject_AsADS_ATTR_INFOs(obAttr, &attr, &cattr)) {
        PyWinObject_FreeWCHAR(szName);
		return NULL;
    }
    IDispatch *pRet = NULL;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->CreateDSObject(szName, attr, cattr, &pRet);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (FAILED(_result)) {
		PyCom_BuildPyException(_result, _swig_self, IID_IDirectoryObject);
	} else
        ret = PyCom_PyObjectFromIUnknown(pRet, IID_IDispatch, FALSE);
	PyADSIObject_FreeADS_ATTR_INFOs(attr, cattr);
	return ret;
};

%}

%native(CreateDSObject) CreateDSObject;

// @pyswig |DeleteDSObject|Deletes a leaf object in a directory tree
// @pyparm string|rdn||The relative distinguished name (relative path) of the object to be deleted.
HRESULT DeleteDSObject(WCHAR *rdn);
