/* File : PyIMAPIContainer.i */

%module IMAPIContainer // An COM interface to MAPI's IMAPIContainer interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIContainer.h"

PyIMAPIContainer::PyIMAPIContainer(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIMAPIContainer::~PyIMAPIContainer()
{
}

/*static*/ IMAPIContainer *PyIMAPIContainer::GetI(PyObject *self)
{
	return (IMAPIContainer *)PyIUnknown::GetI(self);
}

// @pyswig <o PyIInterface>|OpenEntry|Opens an object and returns an interface object for further access. 
PyObject *PyIMAPIContainer::OpenEntry(PyObject *self, PyObject *args) 
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

	IMAPIContainer *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
    // @pyparm string|entryId||The EntryID to open.
    // @pyparm <o PyIID>|iid||The IID of the returned interface, or None for the default interface.
    // @pyparm int|flags||Flags for the call.  May include MAPI_BEST_ACCESS, MAPI_DEFERRED_ERRORS, MAPI_MODIFY and possibly others (see the MAPI documentation)
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

// @pyswig <o PyIMAPITable>|GetContentsTable|Returns an object representing the container's contents table.
// @pyparm int|flags||The flags to use.
HRESULT GetContentsTable( unsigned long ulFlags, IMAPITable **OUTPUT);

// @pyswig <o PyIMAPITable>|GetHierarchyTable|Returns an object representing the container's hierarchy table.
// @pyparm int|flags||The flags to use.
HRESULT GetHierarchyTable( unsigned long ulFlags, IMAPITable **OUTPUT);

%native(OpenEntry) OpenEntry;

