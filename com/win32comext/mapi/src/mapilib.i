/* MAPI Support */

%{
#include "pymapiutil.h"
%}

typedef unsigned long BOOKMARK;
%apply unsigned long {BOOKMARK};

// A MAPIUID is close enough to an IID for now!
%typemap(python,in) MAPIUID *INPUT(MAPIUID temp)
{
	$target = &temp;
	if (!PyWinObject_AsIID($source, (IID *)$target))
		return NULL;
}

%typemap(python,in) MAPIUID *INPUT_NULLOK(MAPIUID temp)
{
	if ($source==Py_None)
		$target = NULL;
	else {
		$target = &temp;
		if (!PyWinObject_AsIID($source, (IID *)$target))
			return NULL;
	}
}

%typemap(python,ignore) IMAPIProp **OUTPUT(IMAPIProp *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMAPITable **OUTPUT(IMAPITable *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMAPISession **OUTPUT(IMAPISession *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMAPIFolder **OUTPUT(IMAPIFolder *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMessage **OUTPUT(IMessage *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMsgStore **OUTPUT(IMsgStore *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMAPIProgress **OUTPUT(IMAPIProgress *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IAttach **OUTPUT(IAttach *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IProfAdmin **OUTPUT(IProfAdmin *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IProfSect **OUTPUT(IProfSect *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IProviderAdmin **OUTPUT(IProviderAdmin *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMAPIAdviseSink **OUTPUT(IMAPIAdviseSink *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IAddrBook **OUTPUT(IAddrBook *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IMsgServiceAdmin **OUTPUT(IMsgServiceAdmin *temp)
{
  $target = &temp;
}
%typemap(python,ignore) IStream **OUTPUT(IStream *temp)
{
  $target = &temp;
}


%typemap(python,argout) IMAPIProp **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIProp)
}
%typemap(python,argout) IMAPITable **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPITable)
}
%typemap(python,argout) IMAPISession **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPISession)
}
%typemap(python,argout) IMAPIFolder **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIFolder)
}
%typemap(python,argout) IMessage **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMessage)
}
%typemap(python,argout) IMsgStore **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMsgStore)
}
%typemap(python,argout) IMAPIProgress **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIProgress)
}
%typemap(python,argout) IAttach **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IAttachment)
}
%typemap(python,argout) IProfAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProfAdmin)
}
%typemap(python,argout) IProfSect **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProfSect)
}
%typemap(python,argout) IProviderAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProviderAdmin)
}
%typemap(python,argout) IMAPIAdviseSink **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIAdviseSink)
}
%typemap(python,argout) IAddrBook **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IAddrBook)
}
%typemap(python,argout) IMsgServiceAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMsgServiceAdmin)
}
%typemap(python,argout) IStream **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IStream)
}


%typemap(python,freearg) IMessage *INPUT,
                         IMessage *INPUT_NULLOK,
                         IMAPITable *INPUT,
                         IMAPITable *INPUT_NULLOK,
						 IMAPISession *INPUT,
						 IMAPISession *INPUT_NULLOK,
						 IMAPIFolder *INPUT,
						 IMAPIFolder *INPUT_NULLOK,
						 IMAPIProp *INPUT,
						 IMAPIProp *INPUT_NULLOK,
						 IMAPIProgress *INPUT,
						 IMAPIProgress *INPUT_NULLOK,
						 IMsgStore *INPUT,
						 IMsgStore *INPUT_NULLOK,
						 IAttach *INPUT,
						 IAttach *INPUT_NULLOK,
						 IProfAdmin *INPUT,
						 IProfAdmin *INPUT_NULLOK,
						 IProfSect *INPUT,
						 IProfSect *INPUT_NULLOK,
                                                 IProviderAdmin *INPUT,
                                                 IProviderAdmin *INPUT_NULLOK,
						 IMAPIAdviseSink *INPUT,
						 IMAPIAdviseSink *INPUT_NULLOK,
						 IAddrBook *INPUT,
						 IAddrBook *INPUT_NULLOK,
						 IMsgServiceAdmin *INPUT,
						 IMsgServiceAdmin *INPUT_NULLOK,
						 IStream *INPUT,
						 IStream *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(python,in) IMAPIProp *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProp, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMAPIProp *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProp, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMAPITable *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPITable, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMAPITable *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPITable, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMAPISession *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPISession, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMAPISession *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPISession, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMAPIFolder *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIFolder, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMAPIFolder *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIFolder, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMessage *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMessage, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMessage *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMessage, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMsgStore *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMsgStore *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMAPIProgress *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProgress, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IMAPIProgress *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProgress, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IAttach *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAttachment, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IAttach *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAttachment, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IMAPIAdviseSink *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIAdviseSink, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IMAPIAdviseSink *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIAdviseSink, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IProfAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IProfAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfAdmin, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IProfSect *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfSect, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IProfSect *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfSect, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IProviderAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProviderAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IProviderAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProviderAdmin, (void **)&$target, 1))
		return NULL;
}

%typemap(python,in) IAddrBook *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAddrBook, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IAddrBook *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAddrBook, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IMsgServiceAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(python,in) IMsgServiceAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin, (void **)&$target, 1))
		return NULL;
}
%typemap(python,in) IStream *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IStream, (void **)&$target, 0))
		return NULL;
}
%typemap(python,in) IStream *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IStream, (void **)&$target, 1))
		return NULL;
}

// Some ** special cases.
%typemap(python,freearg) IMsgStore **INPUT
{
	if ($source && *$source) (*$source)->Release();
}

%typemap(python,arginit) IMsgStore ** {
	$target = NULL;
}

%typemap(python,in) IMsgStore **INPUT(IMsgStore *temp)
{
	$target = &temp;
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)$target, 0))
		return NULL;
}


%typemap(python,in) SRowSet *INPUT {
	if (!PyMAPIObject_AsSRowSet($source, &$target, FALSE))
		return NULL;
}

%typemap(python,in) SRowSet *INPUT_NULLOK {
	if (!PyMAPIObject_AsSRowSet($source, &$target, TRUE))
		return NULL;
}

%typemap(python,in) ADRLIST *INPUT {
	if (!PyMAPIObject_AsADRLIST($source, &$target, FALSE))
		return NULL;
}

%typemap(python,in) ADRLIST *INPUT_NULLOK {
	if (!PyMAPIObject_AsADRLIST($source, &$target, TRUE))
		return NULL;
}

%typemap(python,freearg) SRowSet *INPUT, SRowSet *INPUT_NULLOK {
	if ($source) PyMAPIObject_FreeSRowSet($source);
}

%typemap(python,freearg) ADRLIST *INPUT, ADRLIST *INPUT_NULLOK {
	if ($source) PyMAPIObject_FreeADRLIST($source);
}

%typemap(python,ignore) SRowSet **OUTPUT (SRowSet *temp) {
	$target = &temp;
	*$target = NULL;
}

%typemap(python,ignore) ADRLIST **OUTPUT (ADRLIST *temp) {
	$target = &temp;
	*$target = NULL;
}

%typemap(python,argout) SRowSet **OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromSRowSet(*$source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}

%typemap(python,argout) SRowSet *OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromSRowSet($source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}

%typemap(python,freearg) SRowSet **OUTPUT {
	if (*$source) PyMAPIObject_FreeSRowSet(*$source);
}

%typemap(python,freearg) SRowSet *OUTPUT {
	if ($source) PyMAPIObject_FreeSRowSet($source);
}

%typemap(python,argout) ADRLIST **OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromADRLIST(*$source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}

%typemap(python,argout) ADRLIST *OUTPUT {
	PyObject *o;
	o = PyMAPIObject_FromADRLIST($source);
	if (!$target) {
		$target = o;
	} else if ($target == Py_None) {
		Py_DECREF(Py_None);
		$target = o;
	} else {
		if (!PyList_Check($target)) {
			PyObject *o2 = $target;
			$target = PyList_New(0);
			PyList_Append($target,o2);
			Py_XDECREF(o2);
		}
		PyList_Append($target,o);
		Py_XDECREF(o);
	}
}

%typemap(python,freearg) ADDRLIST **OUTPUT {
	if (*$source) PyMAPIObject_FreeADDRLIST(*$source);
}

%typemap(python,freearg) ADDRLIST *OUTPUT {
	if ($source) PyMAPIObject_FreeADDRLIST($source);
}

%typemap(python,in) SRowSet *BOTH = SRowSet *INPUT;
%typemap(python,freearg) SRowSet *BOTH = SRowSet *INPUT;
%typemap(python,argout) SRowSet *BOTH = SRowSet *OUTPUT;

%typemap(python,in) ADRLIST *BOTH = ADRLIST *INPUT;
%typemap(python,freearg) ADRLIST *BOTH = ADRLIST *INPUT;
%typemap(python,argout) ADRLIST *BOTH = ADRLIST *OUTPUT;

%typemap(python,ignore) MAPIERROR **OUTPUT (MAPIERROR *temp) {
	$target = &temp;
}

%typemap(python,argout) MAPIERROR **OUTPUT {
	PyObject_FromMAPIERROR(*$source, TRUE, TRUE);
}

%typemap(python,ignore) MAPIINIT_0 *OUTPUT (MAPIINIT_0 temp) {
	$target = &temp;
}

%typemap(python,argout) MAPIINIT_0 *OUTPUT {
	Py_DECREF($target);
	$target = Py_BuildValue("ll", 
		$source->ulVersion,
		$source->ulFlags);
}

%typemap(python,in) MAPIINIT_0 *INPUT(MAPIINIT_0 temp)
{
	$target = &temp;
	if ($source==Py_None)
		$target = NULL;
	else {
		if (!PyArg_ParseTuple($source, "ii:MAPIINIT_0 tuple", &($target->ulVersion), &($target->ulFlags))) {
			$cleanup;
			return NULL;
		}
	}
}	

%typemap(python,ignore) SPropTagArray **OUTPUT (SPropTagArray *temp) 
{
	$target = &temp;
}

%typemap(python,argout) SPropTagArray **OUTPUT {
	$target = PyMAPIObject_FromSPropTagArray(*$source);
	if ($target==NULL) {
		$cleanup;
		return NULL;
	}
}

%typemap(python,freearg) SPropTagArray **OUTPUT {
	if (*$source) MAPIFreeBuffer(*$source);
}
%typemap(python,arginit) SPropTagArray **OUTPUT {
	$target = NULL;
}


%typemap(python,in) SPropTagArray *INPUT 
{
	if (!PyMAPIObject_AsSPropTagArray($source, &$target))
		return NULL;
}	

%typemap(python,freearg) SPropTagArray *INPUT 
{
	if ($source) MAPIFreeBuffer($source);
}	

%typemap(python,in) SRestriction *INPUT {
	if (!PyMAPIObject_AsSRestriction($source, &$target))
		return NULL;
}	
%typemap(python,freearg) SRestriction *INPUT
{
	PyMAPIObject_FreeSRestriction($source);
}

%typemap(python,in) SSortOrderSet *INPUT 
{
	if (!PyMAPIObject_AsSSortOrderSet($source, &$target))
		return NULL;
}

%typemap(python,freearg) SSortOrderSet *INPUT
{
	PyMAPIObject_FreeSSortOrderSet($source);
}


%typemap(python,in) SBinaryArray *INPUT (SBinaryArray temp)
{
	$target = &temp;
	$target->lpbin = NULL;
	$target->cValues = 0;
	if (!PyMAPIObject_AsSBinaryArray($source, $target))
		return NULL;
}

%typemap(python,freearg) SBinaryArray *INPUT
{
	PyMAPIObject_FreeSBinaryArray($source);
}

// A "MAPISTRINGARRAY" object - not a real type at all
// but suitable for "returned array of strings"
%typemap(python,ignore) TCHAR **OUTPUT_ARRAY(TCHAR *temp)
{
  $target = &temp;
}

%typemap(python,argout) TCHAR **OUTPUT_ARRAY {
	$target = PyList_New(0);
	for (int __i=0; $source[__i] != NULL ;__i++) {
		PyObject *obNew = PyWinObject_FromTCHAR($source[__i]);
		PyList_Append($target, obNew);
		Py_XDECREF(obNew);
	}
	MAPIFreeBuffer($source);
}

%typemap(python,ignore) char **OUTPUT_MAPI(char *temp)
{
  $target = &temp;
}

%typemap(python,argout) char **OUTPUT_MAPI {
	if (*$source==NULL) {
		$target = Py_None;
		Py_INCREF(Py_None);
	} else {
		$target = PyWinCoreString_FromString(*$source);
		MAPIFreeBuffer(*$source);
	}
}
