/* File : PyIMsgStore.i */

%module IMsgStore // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMsgStore.h"

PyIMsgStore::PyIMsgStore(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIMsgStore::~PyIMsgStore()
{
}

/*static*/ IMsgStore *PyIMsgStore::GetI(PyObject *self)
{
	return (IMsgStore *)PyIUnknown::GetI(self);
}
%}

%native(OpenEntry) OpenEntry;
%{
// @pyswig <o PyIInterface>|OpenEntry|Opens a folder or message and returns an interface object for further access.
PyObject *PyIMsgStore::OpenEntry(PyObject *self, PyObject *args) 
{
    HRESULT  _result;
    char * entryString;
	int entryStrLen;
    IID iid;
	IID *pIID;
    PyObject * objIID = 0;
    unsigned long  flags;
    IUnknown * pUnk = NULL;
	ULONG resType;
	PyObject *obEntry;

	IMsgStore *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm string|entryId||The entryID of the object
	// @pyparm <o PyIID>|iid||The IID of the object to return, or None for the default IID
	// @pyparm int|flags||Bitmask of flags that controls how the object is opened.
    if(!PyArg_ParseTuple(args,"OOl:OpenEntry",&obEntry, &objIID,&flags)) 
        return NULL;
	if (obEntry==Py_None) {
		entryString = NULL;
		entryStrLen = 0;
	} else if PyString_Check(obEntry) {
		entryString = PyString_AsString(obEntry);
		entryStrLen = PyString_Size(obEntry);
	} else {
		PyErr_SetString(PyExc_TypeError, "EntryID must be a string or None");
		return NULL;
	}
	if (objIID==Py_None)
		pIID = NULL;
	else {
		pIID = &iid;
		if (!PyWinObject_AsIID(objIID, pIID))
			return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
     _result = (HRESULT )_swig_self->OpenEntry(entryStrLen,(ENTRYID *)entryString,pIID,flags, &resType, &pUnk);
	Py_END_ALLOW_THREADS
     if (FAILED(_result)) {
           return OleSetOleError(_result);
     }
	 return PyMAPIObject_FromTypedUnknown( resType, pUnk, FALSE /*bAddRef*/);
}

%}

%native(GetReceiveFolder) GetReceiveFolder;
%{
// @pyswig <o PyIID>, string|GetReceiveFolder|Obtains the folder that was established as the destination for incoming messages of a specified message class or the default receive folder for the message store.
PyObject *PyIMsgStore::GetReceiveFolder(PyObject *self, PyObject *args) 
{
	HRESULT  _result;
	unsigned long  flags;
	PyObject *obClass;
	TCHAR *szClass;
	TCHAR *sz_explicit_class = NULL;
	ULONG eid_cb;
	LPENTRYID eid_out = NULL;
	PyObject *rc = NULL;

	IMsgStore *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	// @pyparm string|||Message class that is associated with a receive folder. If thid parameter is set to None or an empty string, GetReceiveFolder returns the default receive folder for the message store. 
	// @pyparm int|flags||
	if(!PyArg_ParseTuple(args,"Ol:OpenEntry",&obClass, &flags))
		goto done;

	if (!PyWinObject_AsTCHAR(obClass, &szClass, TRUE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	_result = (HRESULT )_swig_self->GetReceiveFolder(szClass, flags, &eid_cb, &eid_out, &sz_explicit_class);
	Py_END_ALLOW_THREADS
	if (FAILED(_result)) {
		OleSetOleError(_result);
		goto done;
	}

	rc = Py_BuildValue("NN", PyString_FromStringAndSize((char *)eid_out, eid_cb),
	                         PyWinObject_FromTCHAR(sz_explicit_class));
	MAPIFreeBuffer(eid_out);
	MAPIFreeBuffer(sz_explicit_class);
	PyWinObject_FreeTCHAR(szClass);
done:
	return rc;
}

%}

// @pyswig <o PyIMAPITable>|GetReceiveFolderTable|provides access to the receive folder table, a table that includes information about all of the receive folders for the message store.
HRESULT GetReceiveFolderTable(
    unsigned long ulFlags, // @pyparm int|flags||Bitmask of flags that controls table access
    IMAPITable **OUTPUT 
);


// @pyswig int|CompareEntryIDs|Compares two entry identifiers belonging to a particular address book provider to determine if they refer to the same address book object
// @rdesc The result is set to TRUE if the two entry identifiers refer to the same object, and FALSE otherwise. 
%native(CompareEntryIDs) CompareEntryIDs;
%{
PyObject *PyIMsgStore::CompareEntryIDs(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	HRESULT hr;
	ULONG cb1, cb2;
	ULONG flags=0;
	ULONG ulResult;
	LPENTRYID peid1 = NULL, peid2 = NULL;
	IMsgStore *_swig_self;
	PyObject *obE1, *obE2;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    if(!PyArg_ParseTuple(args,"OO|i:CompareEntryIDs", 
		&obE1, // @pyparm string|entryId||The first entry ID to be compared
		&obE2, // @pyparm string|entryId||The second entry ID to be compared
		&flags)) // @pyparm int|flags|0|Reserved - must be zero.
        goto done;

	if (!PyWinObject_AsString(obE1, (char **)&peid1, FALSE, &cb1))
        goto done;

	if (!PyWinObject_AsString(obE2, (char **)&peid2, FALSE, &cb2))
        goto done;

	Py_BEGIN_ALLOW_THREADS
	hr=_swig_self->CompareEntryIDs(cb1, peid1, cb2, peid2, flags, &ulResult);
	Py_END_ALLOW_THREADS
	if (FAILED(hr))
		rc =  OleSetOleError(hr);
	else
		rc = PyInt_FromLong(ulResult);
done:
	PyWinObject_FreeString((char *)peid1);
	PyWinObject_FreeString((char *)peid2);
	return rc;
}
%}

// @pyswig <o PyMAPIError>|GetLastError|Returns the last error associated with this object
// @pyparm int|hr||The HRESULT
// @pyparm int|flags||
HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig int|AbortSubmit|Attempts to remove a message from the outgoing queue.
%native(AbortSubmit) AbortSubmit;
%{
PyObject *PyIMsgStore::AbortSubmit(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	HRESULT hr;
	ULONG cb;
	ULONG flags=0;
	LPENTRYID peid = NULL;
	IMsgStore *_swig_self;
	PyObject *obE;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	if(!PyArg_ParseTuple(args,"O|i:AbortSubmit", 
		&obE, // @pyparm string|entryId||The entry ID of the item to be aborted.
		&flags)) // @pyparm int|flags|0|Reserved - must be zero.
		goto done;

	if (!PyWinObject_AsString(obE, (char **)&peid, FALSE, &cb))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	hr=_swig_self->AbortSubmit(cb, peid, flags);
	Py_END_ALLOW_THREADS
	if (FAILED(hr))
		rc =  OleSetOleError(hr);
	else {
		rc = Py_None;
		Py_INCREF(Py_None);
	}
done:
	PyWinObject_FreeString((char *)peid);
	return rc;
}
%}
