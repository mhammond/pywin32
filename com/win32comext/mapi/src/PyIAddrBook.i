%module IAddrBook // An COM interface to MAPI's IAddrBook interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIAddrBook.h"

PyIAddrBook::PyIAddrBook(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIAddrBook::~PyIAddrBook()
{
}

/*static*/ IAddrBook *PyIAddrBook::GetI(PyObject *self)
{
	return (IAddrBook *)PyIUnknown::GetI(self);
}

%}

// @pyswig |ResolveName|Performs name resolution, assigning entry identifiers to recipients in a recipient list. 
HRESULT ResolveName(
	unsigned long ulUIParam, // @pyparm int|uiParm||hwnd of a dialogs parent.
	ULONG ulFlags, // @pyparm int|flags||Bitmask of flags that controls whether a dialog box can be displayed.
	TCHAR *INPUT_NULLOK, // @pyparm  string|entryTitle||
	ADRLIST *BOTH // @pyparm <o PyADRLIST>|ADRLIST||Partial addresses to resolve.
);

%{
// @pyswig <o PyIInterface>|OpenEntry|Opens a folder or message and returns an interface object for further access.
PyObject *PyIAddrBook::OpenEntry(PyObject *self, PyObject *args) 
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

	IAddrBook *_swig_self;
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

%native(OpenEntry) OpenEntry; // OpenEntry manually done :-(

// @pyswig int|CompareEntryIDs|Compares two entry identifiers belonging to a particular address book provider to determine if they refer to the same address book object
// @rdesc The result is set to TRUE if the two entry identifiers refer to the same object, and FALSE otherwise. 
%native(CompareEntryIDs) CompareEntryIDs;
%{
PyObject *PyIAddrBook::CompareEntryIDs(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	HRESULT hr;
	ULONG cb1, cb2;
	ULONG flags=0;
	ULONG ulResult;
	LPENTRYID peid1 = NULL, peid2 = NULL;
	IAddrBook *_swig_self;
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
