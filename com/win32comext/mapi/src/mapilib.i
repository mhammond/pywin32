/* MAPI Support */

%{
#include "pymapiutil.h"
%}

typedef unsigned long BOOKMARK;
%apply unsigned long {BOOKMARK};

// A MAPIUID is close enough to an IID for now!
%typemap(in) MAPIUID *INPUT(MAPIUID temp)
{
	$target = &temp;
	if (!PyWinObject_AsIID($source, (IID *)$target))
		return NULL;
}

%typemap(in) MAPIUID *INPUT_NULLOK(MAPIUID temp)
{
	if ($source==Py_None)
		$target = NULL;
	else {
		$target = &temp;
		if (!PyWinObject_AsIID($source, (IID *)$target))
			return NULL;
	}
}

%typemap(in,numinputs=0) IMAPIProp **OUTPUT(IMAPIProp *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMAPITable **OUTPUT(IMAPITable *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMAPISession **OUTPUT(IMAPISession *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMAPIFolder **OUTPUT(IMAPIFolder *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMessage **OUTPUT(IMessage *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMsgStore **OUTPUT(IMsgStore *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMAPIProgress **OUTPUT(IMAPIProgress *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IAttach **OUTPUT(IAttach *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IProfAdmin **OUTPUT(IProfAdmin *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IProfSect **OUTPUT(IProfSect *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IProviderAdmin **OUTPUT(IProviderAdmin *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMAPIAdviseSink **OUTPUT(IMAPIAdviseSink *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IAddrBook **OUTPUT(IAddrBook *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMsgServiceAdmin **OUTPUT(IMsgServiceAdmin *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IMsgServiceAdmin2 **OUTPUT(IMsgServiceAdmin2 *temp)
{
  $target = &temp;
}
%typemap(in,numinputs=0) IStream **OUTPUT(IStream *temp)
{
  $target = &temp;
}


%typemap(argout) IMAPIProp **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIProp)
}
%typemap(argout) IMAPITable **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPITable)
}
%typemap(argout) IMAPISession **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPISession)
}
%typemap(argout) IMAPIFolder **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIFolder)
}
%typemap(argout) IMessage **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMessage)
}
%typemap(argout) IMsgStore **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMsgStore)
}
%typemap(argout) IMAPIProgress **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIProgress)
}
%typemap(argout) IAttach **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IAttachment)
}
%typemap(argout) IProfAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProfAdmin)
}
%typemap(argout) IProfSect **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProfSect)
}
%typemap(argout) IProviderAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IProviderAdmin)
}
%typemap(argout) IMAPIAdviseSink **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMAPIAdviseSink)
}
%typemap(argout) IAddrBook **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IAddrBook)
}
%typemap(argout) IMsgServiceAdmin **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMsgServiceAdmin)
}
%typemap(argout) IMsgServiceAdmin2 **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IMsgServiceAdmin2)
}
%typemap(argout) IStream **OUTPUT {
	MAKE_OUTPUT_INTERFACE($source, $target, IID_IStream)
}


%typemap(freearg) IMessage *INPUT,
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
						 IMsgServiceAdmin2 *INPUT,
						 IMsgServiceAdmin2 *INPUT_NULLOK,
						 IStream *INPUT,
						 IStream *INPUT_NULLOK
{
	if ($source) $source->Release();
}

%typemap(in) IMAPIProp *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProp, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMAPIProp *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProp, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMAPITable *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPITable, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMAPITable *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPITable, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMAPISession *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPISession, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMAPISession *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPISession, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMAPIFolder *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIFolder, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMAPIFolder *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIFolder, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMessage *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMessage, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMessage *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMessage, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMsgStore *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMsgStore *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMAPIProgress *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProgress, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IMAPIProgress *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIProgress, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IAttach *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAttachment, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IAttach *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAttachment, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IMAPIAdviseSink *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIAdviseSink, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IMAPIAdviseSink *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMAPIAdviseSink, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IProfAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IProfAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfAdmin, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IProfSect *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfSect, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IProfSect *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProfSect, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IProviderAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProviderAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IProviderAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IProviderAdmin, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IAddrBook *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAddrBook, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IAddrBook *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IAddrBook, (void **)&$target, 1))
		return NULL;
}
%typemap(in) IMsgServiceAdmin *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IMsgServiceAdmin *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IMsgServiceAdmin2 *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin2, (void **)&$target, 0))
		return NULL;
}

%typemap(in) IMsgServiceAdmin2 *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgServiceAdmin2, (void **)&$target, 1))
		return NULL;
}

%typemap(in) IStream *INPUT {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IStream, (void **)&$target, 0))
		return NULL;
}
%typemap(in) IStream *INPUT_NULLOK {
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IStream, (void **)&$target, 1))
		return NULL;
}

// Some ** special cases.
%typemap(freearg) IMsgStore **INPUT
{
	if ($source && *$source) (*$source)->Release();
}

%typemap(arginit) IMsgStore ** {
	$target = NULL;
}

%typemap(in) IMsgStore **INPUT(IMsgStore *temp)
{
	$target = &temp;
	if (!PyCom_InterfaceFromPyInstanceOrObject($source, IID_IMsgStore, (void **)$target, 0))
		return NULL;
}


%typemap(in) SRowSet *INPUT {
	if (!PyMAPIObject_AsSRowSet($source, &$target, FALSE))
		return NULL;
}

%typemap(in) SRowSet *INPUT_NULLOK {
	if (!PyMAPIObject_AsSRowSet($source, &$target, TRUE))
		return NULL;
}

%typemap(in) ADRLIST *INPUT {
	if (!PyMAPIObject_AsADRLIST($source, &$target, FALSE))
		return NULL;
}

%typemap(in) ADRLIST *INPUT_NULLOK {
	if (!PyMAPIObject_AsADRLIST($source, &$target, TRUE))
		return NULL;
}

%typemap(freearg) SRowSet *INPUT, SRowSet *INPUT_NULLOK {
	if ($source) PyMAPIObject_FreeSRowSet($source);
}

%typemap(freearg) ADRLIST *INPUT, ADRLIST *INPUT_NULLOK {
	if ($source) PyMAPIObject_FreeADRLIST($source);
}

%typemap(in,numinputs=0) SRowSet **OUTPUT (SRowSet *temp) {
	$target = &temp;
	*$target = NULL;
}

%typemap(in,numinputs=0) ADRLIST **OUTPUT (ADRLIST *temp) {
	$target = &temp;
	*$target = NULL;
}

%typemap(argout) SRowSet **OUTPUT {
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

%typemap(argout) SRowSet *OUTPUT {
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

%typemap(freearg) SRowSet **OUTPUT {
	if (*$source) PyMAPIObject_FreeSRowSet(*$source);
}

%typemap(freearg) SRowSet *OUTPUT {
	if ($source) PyMAPIObject_FreeSRowSet($source);
}

%typemap(argout) ADRLIST **OUTPUT {
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

%typemap(argout) ADRLIST *OUTPUT {
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

%typemap(freearg) ADDRLIST **OUTPUT {
	if (*$source) PyMAPIObject_FreeADDRLIST(*$source);
}

%typemap(freearg) ADDRLIST *OUTPUT {
	if ($source) PyMAPIObject_FreeADDRLIST($source);
}

%typemap(in) SRowSet *BOTH = SRowSet *INPUT;
%typemap(freearg) SRowSet *BOTH = SRowSet *INPUT;
%typemap(argout) SRowSet *BOTH = SRowSet *OUTPUT;

%typemap(in) ADRLIST *BOTH = ADRLIST *INPUT;
%typemap(freearg) ADRLIST *BOTH = ADRLIST *INPUT;
%typemap(argout) ADRLIST *BOTH = ADRLIST *OUTPUT;

%typemap(in,numinputs=0) MAPIERROR **OUTPUT (MAPIERROR *temp) {
	$target = &temp;
}

%typemap(argout) MAPIERROR **OUTPUT {
	PyObject_FromMAPIERROR(*$source, TRUE, TRUE);
}

%typemap(in,numinputs=0) MAPIINIT_0 *OUTPUT (MAPIINIT_0 temp) {
	$target = &temp;
}

%typemap(argout) MAPIINIT_0 *OUTPUT {
	Py_DECREF($target);
	$target = Py_BuildValue("ll",
		$source->ulVersion,
		$source->ulFlags);
}

%typemap(in) MAPIINIT_0 *INPUT(MAPIINIT_0 temp)
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

%typemap(in,numinputs=0) SPropTagArray **OUTPUT (SPropTagArray *temp)
{
	$target = &temp;
}

%typemap(argout) SPropTagArray **OUTPUT {
	if ($target == Py_None)
		Py_DECREF($target);
	$target = PyMAPIObject_FromSPropTagArray(*$source);
	if ($target==NULL) {
		$cleanup;
		return NULL;
	}
}

%typemap(freearg) SPropTagArray **OUTPUT {
	if (*$source) MAPIFreeBuffer(*$source);
}
%typemap(arginit) SPropTagArray **OUTPUT {
	$target = NULL;
}


%typemap(in) SPropTagArray *INPUT
{
	if (!PyMAPIObject_AsSPropTagArray($source, &$target))
		return NULL;
}

%typemap(freearg) SPropTagArray *INPUT
{
	if ($source) MAPIFreeBuffer($source);
}

%typemap(in) SRestriction *INPUT {
	if (!PyMAPIObject_AsSRestriction($source, &$target))
		return NULL;
}
%typemap(freearg) SRestriction *INPUT
{
	PyMAPIObject_FreeSRestriction($source);
}

%typemap(in) SSortOrderSet *INPUT
{
	if (!PyMAPIObject_AsSSortOrderSet($source, &$target))
		return NULL;
}

%typemap(freearg) SSortOrderSet *INPUT
{
	PyMAPIObject_FreeSSortOrderSet($source);
}


%typemap(in) SBinaryArray *INPUT (SBinaryArray temp)
{
	$target = &temp;
	$target->lpbin = NULL;
	$target->cValues = 0;
	if (!PyMAPIObject_AsSBinaryArray($source, $target))
		return NULL;
}

%typemap(freearg) SBinaryArray *INPUT
{
	PyMAPIObject_FreeSBinaryArray($source);
}

// A "MAPISTRINGARRAY" object - not a real type at all
// but suitable for "returned array of strings"
%typemap(in,numinputs=0) TCHAR **OUTPUT_ARRAY(TCHAR *temp)
{
  $target = &temp;
}

%typemap(argout) TCHAR **OUTPUT_ARRAY {
	$target = PyList_New(0);
	for (int __i=0; $source[__i] != NULL ;__i++) {
		PyObject *obNew = PyWinObject_FromTCHAR($source[__i]);
		PyList_Append($target, obNew);
		Py_XDECREF(obNew);
	}
	MAPIFreeBuffer($source);
}

%typemap(in,numinputs=0) char **OUTPUT_MAPI(char *temp)
{
  $target = &temp;
}

%typemap(argout) char **OUTPUT_MAPI {
	if (*$source==NULL) {
		$target = Py_None;
		Py_INCREF(Py_None);
	} else {
		$target = PyWinCoreString_FromString(*$source);
		MAPIFreeBuffer(*$source);
	}
}
