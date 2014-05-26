/* File : PyIMAPIProp.i */

%module IMAPIProp // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"

// A little helper just for this file
static PyObject* OleSetTypeError(char *msg)
{
	PyErr_SetString(PyExc_TypeError, msg);
	return NULL;
}


PyIMAPIProp::PyIMAPIProp(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIMAPIProp::~PyIMAPIProp()
{
}

/*static*/ IMAPIProp *PyIMAPIProp::GetI(PyObject *self)
{
	return (IMAPIProp *)PyIUnknown::GetI(self);
}

// @pyswig int, [items, ]|GetProps|Returns a list of property values.
PyObject *PyIMAPIProp::GetProps(PyObject *self, PyObject *args) {
    PyObject * _resultobj;
    HRESULT  _result;
    SPropTagArray * _arg0 = NULL;
    unsigned long  _arg1=0;
    PyObject * _obj0 = 0;

	IMAPIProp *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm <o PySPropTagArray>|propList||The list of properties
	// @pyparm int|flags|0|
    if(!PyArg_ParseTuple(args,"O|l:GetProps",&_obj0,&_arg1)) 
        return NULL;
	if (!PyMAPIObject_AsSPropTagArray(_obj0, &_arg0))
		return NULL;
	ULONG numValues;
	SPropValue *pv;
	Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT )_swig_self->GetProps(_arg0,_arg1, &numValues, &pv);
	Py_END_ALLOW_THREADS
	PyMAPIObject_FreeSPropTagArray(_arg0);
    if (FAILED(_result))  {
       return OleSetOleError(_result);
    }

	_resultobj = PyTuple_New(numValues);
	if (_resultobj==NULL) {
		MAPIFreeBuffer(pv);
		PyErr_SetString(PyExc_MemoryError, "Allocating SRowSet result");
		return NULL;
	}
	for (ULONG i=0;i<numValues;i++) {
		PyObject *newOb = PyMAPIObject_FromSPropValue(pv+i);
		if (newOb==NULL) {
			MAPIFreeBuffer(pv);
			Py_DECREF(_resultobj);
			return NULL;
		}
		PyTuple_SetItem(_resultobj, i, newOb);
		// SetItem() keeps our reference to newOb
	}
	MAPIFreeBuffer(pv);	// all done with it now.
	PyObject *realrc = Py_BuildValue("iO", _result, _resultobj);
	Py_DECREF(_resultobj);
	return realrc;
}

// @pyswig int, [problems, ]|DeleteProps|Deletes a set of properties.
PyObject *PyIMAPIProp::DeleteProps(PyObject *self, PyObject *args) 
{
	PyObject *obs;
    SPropTagArray * _arg0;
	HRESULT hr;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	PyObject *myob = NULL;
	// @pyparm <o PySPropTagArray>|propList||The list of properties
    if(!PyArg_ParseTuple(args,"O:DeleteProps",&obs))
		return NULL;
	if (!PyMAPIObject_AsSPropTagArray(obs, &_arg0))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->DeleteProps( _arg0, NULL);
	Py_END_ALLOW_THREADS
	PyMAPIObject_FreeSPropTagArray(_arg0);
	if (FAILED(hr))
		return OleSetOleError(hr);
	return Py_BuildValue("iz", hr, NULL); // None used as place holder for problem array later.
}

// @pyswig int, [problems, ]|SetProps|Sets a set of properties.
PyObject *PyIMAPIProp::SetProps(PyObject *self, PyObject *args) 
{
	PyObject *obs;
	HRESULT hr;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	PyObject *myob = NULL;
	// @pyparm [<o PySPropValue>, ]|propList||The list of properties
    if(!PyArg_ParseTuple(args,"O:SetProps",&obs))
		return NULL;
	if (!PySequence_Check(obs)) {
		PyErr_SetString(PyExc_TypeError, "Properties must be a sequence of tuples");
		return NULL;
	}
	SPropValue *pPV;
	ULONG seqLen;
	if (!PyMAPIObject_AsSPropValueArray(obs, &pPV, &seqLen))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->SetProps( seqLen, pPV, NULL);
	Py_END_ALLOW_THREADS
	MAPIFreeBuffer(pPV);
	if (FAILED(hr))
		return OleSetOleError(hr);
	return Py_BuildValue("iz", hr, NULL); // None used as place holder for proplem array later.
}

// @pyswig int, [problems, ]|CopyTo|Copies an object to another
PyObject *PyIMAPIProp::CopyTo(PyObject *self, PyObject *args) 
{
	IID *pExclude = NULL;
	ULONG ciidExclude = 0;
	IID iid;
	IUnknown *pUnk = NULL;
	SPropTagArray *pta = NULL;
	PyObject *result = NULL;
	PyObject *obIIDExclude, *obPropTags, *obDest, *obIID;
	ULONG ulUIParm, flags;
	char *szIgnore;
	HRESULT hr;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	// @pyparm [<o PyIID>, ]|IIDExcludeList||A sequence of IIDs to exclude.
	// @pyparm <o PySPropTagArray>|propTags||The property tags to copy
	// @pyparm int|uiFlags||Flags for the progress object
	// @pyparm None|progress||Reserved - must pass None
	// @pyparm <o PyIID>|resultIID||IID of the destination object
	// @pyparm <o PyIMAPIProp>|dest||The destination object
	// @pyparm int|flags||flags
    if(!PyArg_ParseTuple(args,"OOlzOOl:CopyTo",&obIIDExclude, &obPropTags, &ulUIParm, &szIgnore, &obIID, &obDest, &flags))
		return NULL;
	if (obIIDExclude==Py_None)
		pExclude = NULL;
	else {
		if (!PySequence_Check(obIIDExclude)) {
			OleSetTypeError("Argument 1 must be a sequence of IID's, or None");
			goto error;
		}
		ciidExclude = PySequence_Length(obIIDExclude);
		pExclude = new IID[ciidExclude];
		if (pExclude==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating array of IID's");
			goto error;
		}
		for (ULONG i=0;i<ciidExclude;i++) {
			PyObject *ob = PySequence_GetItem(obIIDExclude, (int)i);
			BOOL ok = PyWinObject_AsIID(ob, pExclude+i);
			Py_XDECREF(ob);
			if (!ok) {
				goto error;
			}
		}
	}
	// PropTagArray
	if (!PyMAPIObject_AsSPropTagArray(obPropTags, &pta))
		goto error;
	// IID.
	if (!PyWinObject_AsIID(obIID, &iid))
		goto error;
	// IUnknown.
	if (!PyCom_InterfaceFromPyInstanceOrObject(obDest, IID_IUnknown, (void **)&pUnk, 0))
		goto error;
	// Finally make the call.
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->CopyTo(ciidExclude, pExclude, pta, ulUIParm, NULL, &iid, (void *)pUnk, flags, NULL );
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto error;
	}
	result = Py_BuildValue("iz", hr, NULL);
error:
	free(pExclude);
	PyMAPIObject_FreeSPropTagArray(pta);
	if (pUnk)
		pUnk->Release();
	return result;
}

// @pyswig int, [problems, ]|CopyProps|Copies a set of properties to another object
PyObject *PyIMAPIProp::CopyProps(PyObject *self, PyObject *args) 
{
	IID iid;
	IUnknown *pUnk = NULL;
	SPropTagArray *pta = NULL;
	PyObject *result = NULL;
	PyObject *obPropTags, *obDest, *obIID;
	ULONG ulUIParm, flags;
	char *szIgnore;
	HRESULT hr;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	// @pyparm <o PySPropTagArray>|propTags||The property tags to copy
	// @pyparm int|uiFlags||Flags for the progress object
	// @pyparm None|progress||Reserved - must pass None
	// @pyparm <o PyIID>|resultIID||IID of the destination object
	// @pyparm <o PyIMAPIProp>|dest||The destination object
	// @pyparm int|flags||flags
    if(!PyArg_ParseTuple(args,"OlzOOl:CopyProps",&obPropTags, &ulUIParm, &szIgnore, &obIID, &obDest, &flags))
		return NULL;
	// PropTagArray
	if (!PyMAPIObject_AsSPropTagArray(obPropTags, &pta))
		goto error;
	// IID.
	if (!PyWinObject_AsIID(obIID, &iid))
		goto error;
	// IUnknown.
	if (!PyCom_InterfaceFromPyInstanceOrObject(obDest, IID_IUnknown, (void **)&pUnk, 0))
		goto error;
	// Finally make the call.
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->CopyProps(pta, ulUIParm, NULL, &iid, (void *)pUnk, flags, NULL );
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto error;
	}
	result = Py_BuildValue("iz", hr, NULL);
error:
	PyMAPIObject_FreeSPropTagArray(pta);
	if (pUnk)
		pUnk->Release();
	return result;
}

// @pyswig <o PyIUnknown>|OpenProperty|Returns an interface object to be used to access a property. 
PyObject *PyIMAPIProp::OpenProperty(PyObject *self, PyObject *args) 
{
	IID iid;
	IUnknown *pUnk = NULL;
	PyObject *obIID;
	ULONG propTag, flags, interfaceOptions;
	HRESULT hr;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	// @pyparm ULONG|propTag||The property tag to open
	// @pyparm <o PyIID>|iid||The IID of the resulting interface.
	// @pyparm int|interfaceOptions||Data that relates to the interface identified by the lpiid parameter. 
	// @pyparm int|flags||flags
    if(!PyArg_ParseTuple(args,"kOll:OpenProperty",&propTag, &obIID, &interfaceOptions, &flags))
		return NULL;
	// IID.
	if (!PyWinObject_AsIID(obIID, &iid))
		return NULL;
	// Make the call.
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->OpenProperty(propTag, &iid, interfaceOptions, flags, &pUnk);
	Py_END_ALLOW_THREADS
	if (FAILED(hr)) {
		return OleSetOleError(hr);
	}
	return PyCom_PyObjectFromIUnknown(pUnk, iid, /*BOOL bAddRef*/ FALSE);
}

// @pyswig <o PySPropTagArray>|GetIDsFromNames|Determines property IDs
PyObject *PyIMAPIProp::GetIDsFromNames(PyObject *self, PyObject *args)
{
	PyObject *obNameIds;
	ULONG flags=0;
	// @pyparm PyMAPINAMEIDArray|nameIds||Sequence of name ids
	// @pyparm int|flags|0|
	if (!PyArg_ParseTuple(args, "O|l:GetIDsFromNames", &obNameIds, &flags))
		return NULL;
	MAPINAMEID **ppIds;
	ULONG numIds = 0;
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	if (!PyMAPIObject_AsMAPINAMEIDArray(obNameIds, &ppIds, &numIds, TRUE))
		return NULL;
	SPropTagArray *pTagResult;
	HRESULT hr;
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->GetIDsFromNames(numIds, ppIds, flags, &pTagResult);
	Py_END_ALLOW_THREADS
	PyMAPIObject_FreeMAPINAMEIDArray(ppIds);
	if (FAILED(hr))
		return OleSetOleError(hr);
	PyObject *rc = PyMAPIObject_FromSPropTagArray(pTagResult);
	MAPIFreeBuffer(pTagResult);
	return rc;
}
%}

// @pyswig HRESULT, <o PySPropTagArray>, <o PyMAPINAMEIDArray>|GetNamesFromIDs|Determines property names
%native (GetNamesFromIDs) GetNamesFromIDs;
%{
PyObject *PyIMAPIProp::GetNamesFromIDs(PyObject *self, PyObject *args)
{
	IMAPIProp *pMAPIProp;
	if ((pMAPIProp=GetI(self))==NULL) return NULL;
	PyObject *obTags, *obiid = Py_None;
	ULONG flags=0;
	// @pyparm <o PySPropTagArray>|propTags||Sequence of property tags, or None
	// @pyparm <o PyIID>|propSetGuid|None|a globally unique identifier, identifying a property set, or None
	// @pyparm int|flags|0|
	if (!PyArg_ParseTuple(args, "O|Ol:GetIDsFromNames", &obTags, &obiid, &flags))
		return NULL;
	GUID guid, *pguid = NULL;
	if (obiid != Py_None) {
		pguid = &guid;
		if (!PyWinObject_AsIID(obiid, pguid))
			return NULL;
	}
	SPropTagArray *pta = NULL;
	if (obTags != Py_None)
		if (!PyMAPIObject_AsSPropTagArray(obTags, &pta))
			return NULL;

	ULONG numNames = 0;
	MAPINAMEID **ppNames = NULL;
	HRESULT hr;
	Py_BEGIN_ALLOW_THREADS
	hr = pMAPIProp->GetNamesFromIDs( &pta, pguid, flags, &numNames, &ppNames );
	Py_END_ALLOW_THREADS
	PyObject *result;
	if (SUCCEEDED(hr)) {
		PyObject *obNames = PyMAPIObject_FromMAPINAMEIDArray(ppNames, numNames);
		PyObject *obTags = PyMAPIObject_FromSPropTagArray(pta);
		result = Py_BuildValue("lOO", hr, obTags, obNames);
		Py_XDECREF(obTags);
		Py_XDECREF(obNames);
	} else {
		OleSetOleError(hr);
		result = NULL;
	}
	if (ppNames) MAPIFreeBuffer(ppNames);
	if (pta) PyMAPIObject_FreeSPropTagArray(pta);
	return result;
}
%}

// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
// @pyparm int|hr||Contains the error code generated in the previous method call.
// @pyparm int|flags||Indicates for format for the output.
HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig |SaveChanges|Saves pending changes to the object
// @pyparm int|flags||flags
HRESULT SaveChanges(unsigned long flags);

%native(GetProps) GetProps; // GetProps manually done :-(

%native(SetProps) SetProps;

%native(CopyTo) CopyTo;

%native(CopyProps) CopyProps;

%native(DeleteProps) DeleteProps;

%native(GetIDsFromNames) GetIDsFromNames;

%native(OpenProperty) OpenProperty;

// @pyswig <o PySPropTagArray>|GetPropList|Gets a list of properties
// @pyparm int|flags||flags
HRESULT GetPropList(
	unsigned long ulFlags,
	SPropTagArray **OUTPUT
);

