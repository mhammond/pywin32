/* File : PyIMAPISession.i */

%module IMAPISession // An COM interface to MAPI's ISession interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPISession.h"

PyIMAPISession::PyIMAPISession(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIMAPISession::~PyIMAPISession()
{
}

/*static*/ IMAPISession *PyIMAPISession::GetI(PyObject *self)
{
	return (IMAPISession *)PyIUnknown::GetI(self);
}
%}

%native(OpenEntry) OpenEntry; // OpenEntry manually done :-(
%{
// @pyswig <o PyIInterface>|OpenEntry|Opens an object and returns an interface object for further access. 
PyObject *PyIMAPISession::OpenEntry(PyObject *self, PyObject *args) 
{
    HRESULT  _result;
    char *entryString;
	int entryStrLen;
    IID iid;
	IID *pIID;
    PyObject * objIID = 0;
    unsigned long  flags;
    IUnknown * pUnk = NULL;
	ULONG resType;
	PyObject *obEntry;

	IMAPISession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    // @pyparm string|entryId||The EntryID to open.
    // @pyparm <o PyIID>|iid||The IID of the returned interface, or None for the default interface.
    // @pyparm int|flags||Flags for the call.  May include MAPI_BEST_ACCESS, MAPI_DEFERRED_ERRORS, MAPI_MODIFY and possibly others (see the MAPI documentation)
    if(!PyArg_ParseTuple(args,"OOl:OpenEntry", &obEntry, &objIID, &flags)) 
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

%native(OpenMsgStore) OpenMsgStore;
%{
// @pyswig <o PyIUnknown>|OpenMsgStore|Opens a message store.
PyObject *PyIMAPISession::OpenMsgStore(PyObject *self, PyObject *args) 
{
    HRESULT  _result;
    char * entryString;
	int entryStrLen;
    IID iid;
	IID *pIID;
    PyObject * objIID = 0;
    unsigned long  ulParm;
    unsigned long  flags;
    IMsgStore * pMS = NULL;

	IMAPISession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    // @pyparm int|uiParam||Handle to the parent window for dialogs.
    // @pyparm string|entryId||The entry ID of the message store to open.
    // @pyparm <o PyIID>|iid||The IID of the interface returned, or None
    // @pyparm int|flags||Options for the call.
    if(!PyArg_ParseTuple(args,"ls#Ol:OpenMsgStore",&ulParm,&entryString,&entryStrLen, &objIID,&flags)) 
        return NULL;
	if (objIID==Py_None)
		pIID = NULL;
	else {
		pIID = &iid;
		if (!PyWinObject_AsIID(objIID, pIID))
			return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
     _result = (HRESULT )_swig_self->OpenMsgStore(ulParm, entryStrLen,(ENTRYID *)entryString,pIID,flags,&pMS);
	Py_END_ALLOW_THREADS
     if (FAILED(_result)) {
           return OleSetOleError(_result);
     }
    // @comm The result is the interface specified by the IID, or IID_IMsgStore if None is used.
     return PyCom_PyObjectFromIUnknown(pMS, pIID ? *pIID : IID_IMsgStore, FALSE /*bAddRef*/ );
}
%}


%native(QueryIdentity) QueryIdentity;
%{
// @pyswig string|QueryIdentity|Returns the entry identifier of the object that provides the primary identity for the session.
PyObject *PyIMAPISession::QueryIdentity(PyObject *self, PyObject *args) 
{
	ULONG cb;
	LPENTRYID peid;
	IMAPISession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    if(!PyArg_ParseTuple(args,":QueryIdentity")) 
        return NULL;

	HRESULT _result;
	Py_BEGIN_ALLOW_THREADS
	_result = _swig_self->QueryIdentity(&cb, &peid);
	Py_END_ALLOW_THREADS
	PyObject *rc;
	if (_result==S_OK)
		rc = PyString_FromStringAndSize((char *)peid, cb);
	else if (FAILED(_result)) {
           rc = OleSetOleError(_result);
    } else {
		rc = Py_None;
		Py_INCREF(Py_None);
	}
	return rc;
}
%}

// @pyswig int|Advise|
// @rdesc The result is an integer which should be passed to
// <om PyIMAPISession.Unadvise>
%native(Advise) Advise;
%{
PyObject *PyIMAPISession::Advise(PyObject *self, PyObject *args)
{
	IMAPISession *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;

	// @pyparm string|entryId||The entryID of the object
	// @pyparm int|mask||
	// @pyparm <o PyIMAPIAdviseSink>|sink||
	PyObject *obEntry, *obSink;
	int mask;
	if(!PyArg_ParseTuple(args,"OkO:Advise",&obEntry, &mask, &obSink))
		return NULL;
	char *entryString;
	Py_ssize_t entryStrLen;
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
	IMAPIAdviseSink *psink = NULL;
	if (!PyCom_InterfaceFromPyObject(obSink, IID_IMAPIAdviseSink, (void **)&psink, FALSE))
		return NULL;
	unsigned long connection;
	HRESULT _result;
	PyObject *rc;
	Py_BEGIN_ALLOW_THREADS
	_result = _swig_self->Advise(entryStrLen, (LPENTRYID)entryString,
	                             mask, psink, &connection); 
	Py_END_ALLOW_THREADS
	if (FAILED(_result))
		rc = OleSetOleError(_result);
	else
		rc = PyLong_FromUnsignedLong(connection);
	{
	Py_BEGIN_ALLOW_THREADS
	psink->Release();
	Py_END_ALLOW_THREADS
	}
	return rc;
}
%}

// @pyswig |Unadvise|
// @pyparm int|connection||Value returned from <om PyIMAPISession.Advise>
HRESULT Unadvise(unsigned long connection); 


// @pyswig int|CompareEntryIDs|Compares two entry identifiers belonging to a particular address book provider to determine if they refer to the same address book object
// @rdesc The result is set to TRUE if the two entry identifiers refer to the same object, and FALSE otherwise. 
%native(CompareEntryIDs) CompareEntryIDs;
%{
PyObject *PyIMAPISession::CompareEntryIDs(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	HRESULT hr;
	ULONG cb1, cb2;
	ULONG flags=0;
	ULONG ulResult;
	LPENTRYID peid1 = NULL, peid2 = NULL;
	IMAPISession *_swig_self;
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


HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig <o PyIMAPITable>|GetMsgStoresTable|Provides access to the message store table - a table with information about all of the message stores in the session profile.
HRESULT GetMsgStoresTable(
    unsigned long ulFlags, // @pyparm int|flags||Flags that control the opening.
    IMAPITable **OUTPUT 
);

// @pyswig <o PyIMAPITable>|GetStatusTable|Provides access to the status table - a table with information about all of the MAPI resources in the session.
HRESULT GetStatusTable(
    unsigned long ulFlags, // @pyparm int|flags||Flags that control the opening.
    IMAPITable **OUTPUT 
);

// @pyswig |Logoff|Ends a MAPI session.
HRESULT Logoff( 
    unsigned long ulUIParam,  // @pyparm int|uiParm||hwnd of a dialog is to be displayed.
    unsigned long ulFlags,    // @pyparm int|flags||Bitmask of flags that control the logoff operation.
    unsigned long ulReserved ); // @pyparm int|reserved||Reserved; must be zero. 

// @pyswig <o PyIAddrBook>|OpenAddressBook|Opens the integrated address book.
HRESULT OpenAddressBook(
	unsigned long ulUIParm, // @pyparm int|uiParm||hwnd of a dialog is to be displayed.
	IID *INPUT_NULLOK, // @pyparm <o PyIID>|iid||The IID of the interface, or None.
	unsigned long flags, // @pyparm int|flags||Flags that control the opening - AB_NO_DIALOG.
	IAddrBook **OUTPUT
);

// @pyswig <o PyIProfSection>|OpenProfileSection|Opens a section of the current profile and returns an object for futher access
HRESULT OpenProfileSection(
	MAPIUID *INPUT, // @pyparm <o PyIID>|iidSection||The MAPIIID of the profile section
	IID *INPUT_NULLOK, // @pyparm <o PyIID>|iid||The IID of the interface, or None.
	unsigned long flags, // @pyparm int|flags||Flags that control the opening.
	IProfSect **OUTPUT);
