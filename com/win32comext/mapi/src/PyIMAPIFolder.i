/* File : PyIMAPIFolder.i */

%module IMAPIFolder // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIContainer.h"
#include "PyIMAPIFolder.h"

PyIMAPIFolder::PyIMAPIFolder(IUnknown *pDisp) :
	PyIMAPIContainer(pDisp)
{
	ob_type = &type;
}

PyIMAPIFolder::~PyIMAPIFolder()
{
}

/*static*/ IMAPIFolder *PyIMAPIFolder::GetI(PyObject *self)
{
	return (IMAPIFolder *)PyIUnknown::GetI(self);
}


%}

%native(GetLastError) GetLastError;
%{
// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
PyObject *PyIMAPIFolder::GetLastError(PyObject *self, PyObject *args)
{
	HRESULT hr, hRes;
	ULONG flags = 0;
	MAPIERROR *me = NULL;
	
	IMAPIFolder *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
    if(!PyArg_ParseTuple(args,"l|l:GetLastError",
		&hr, // @pyparm int|hr||Contains the error code generated in the previous method call.
		&flags)) // @pyparm int|flags||Indicates for format for the output.
        return NULL;
		
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->GetLastError(hr, flags, &me);
	Py_END_ALLOW_THREADS

	if (FAILED(hRes))
		return OleSetOleError(hRes);
	
	if (me == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyObject_FromMAPIERROR(me, flags & MAPI_UNICODE, TRUE);
}
%}

%native(CreateFolder) CreateFolder;
%{
// @pyswig <o PyIMAPIFolder>|CreateFolder|Creates a folder object.
PyObject *PyIMAPIFolder::CreateFolder(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	ULONG ulFolderType;
	PyObject *obFolderName;
	LPTSTR lpszFolderName = NULL;
	PyObject *obFolderComment = Py_None;
	LPTSTR lpszFolderComment = NULL;
	PyObject *obInterface = Py_None;
	IID iid;
	LPIID lpInterface = NULL;
	ULONG ulFlags = 0;
	LPMAPIFOLDER lpFolder = NULL;
	
	IMAPIFolder *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "lO|OOl",
		&ulFolderType, // @pyparm int|folderType||The type of folder to create
		&obFolderName, // @pyparm string|folderName||The name of the folder.
		&obFolderComment, // @pyparm string|folderComment|None|A comment for the folder or None
		&obInterface, // @pyparm <o PyIID>|iid|None|The IID of the object to return.  Should usually be None.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obFolderName, &lpszFolderName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obFolderComment, &lpszFolderComment, ulFlags & MAPI_UNICODE, TRUE))
		goto done;
	if (obInterface != Py_None)
	{
		lpInterface = &iid;
		if (!PyWinObject_AsIID(obInterface, lpInterface))
			goto done;
	}
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->CreateFolder(ulFolderType, lpszFolderName, lpszFolderComment, lpInterface, ulFlags, &lpFolder);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		MAKE_OUTPUT_INTERFACE(&lpFolder, result, IID_IMAPIFolder);

done:
	PyWinObject_FreeString(lpszFolderName);
	PyWinObject_FreeString(lpszFolderComment);
	
	return result;
}
%}

// @pyswig <o PyIMessage>|CreateMessage|Creates a message in a folder
HRESULT CreateMessage( 
	IID *INPUT_NULLOK,    // @pyparm <o PyIID>|iid||The IID of the object to return.  Should usually be None.
	ULONG ulFlags,	// @pyparm int|flags||
	IMessage **OUTPUT);

// @pyswig int|CopyMessages|Copies the specified messages
HRESULT_KEEP_INFO CopyMessages(
	SBinaryArray *INPUT, // @pyparm <o PySBinaryArray>|msgs||
	IID *INPUT_NULLOK,    // @pyparm <o PyIID>|iid||IID representing the interface to be used to access the destination folder.  Should usually be None.
	IMAPIFolder *INPUT, // @pyparm <o PyIMAPIFolder>|folder||The destination folder
	unsigned long ulUIParam, // @pyparm long|ulUIParam||Handle of the parent window for any dialog boxes or windows this method displays.
	IMAPIProgress *INPUT_NULLOK, // @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	unsigned long ulFlags); // @pyparm int|flags||A bitmask of
	// @flagh Mask|Description
	// @flag MAPI_DECLINE_OK|Informs the message store provider to immediately return MAPI_E_DECLINE_COPY if it implements CopyMessage by calling the support object's IMAPISupport::DoCopyTo or IMAPISupport::DoCopyProps method. 
	// @flag MESSAGE_DIALOG |Displays a progress indicator as the operation proceeds. 
	// @flag MESSAGE_MOVE|The message or messages are to be moved rather than copied. If MESSAGE_MOVE is not set, the messages are copied. 

// @pyswig |DeleteFolder|Deletes a subfolder.
%native(DeleteFolder) DeleteFolder;
%{
PyObject *PyIMAPIFolder::DeleteFolder(PyObject *self, PyObject *args) 
{
	HRESULT hRes;
	PyObject *obEntryId, *obUIParam, *obProgress;
	ULONG cbEID;
	LPENTRYID eid;
	ULONG_PTR ulUIParam;
	LPMAPIPROGRESS lpProgress;
	ULONG flags = 0;
	
	IMAPIFolder *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
    if(!PyArg_ParseTuple(args,"OOO|l:DeleteFolder",
		&obEntryId, // @pyparm string|entryId||The EntryID of the subfolder to delete.
		&obUIParam, // @pyparm long|uiParam||Handle of the parent window of the progress indicator.
		&obProgress, // @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
		&flags)) 
        return NULL;
	if PyString_Check(obEntryId) {
		eid = (LPENTRYID)PyString_AsString(obEntryId);
		cbEID = PyString_Size(obEntryId);
	} else {
		PyErr_SetString(PyExc_TypeError, "EntryID must be a string");
		return NULL;
	}
	if (!PyWinLong_AsULONG_PTR(obUIParam, &ulUIParam))
		return NULL;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obProgress, IID_IMAPIProgress, (void **)&lpProgress, TRUE))
		return NULL;

	PY_INTERFACE_PRECALL;
	hRes = (HRESULT)_swig_self->DeleteFolder(cbEID, eid, ulUIParam, lpProgress, flags);
	PY_INTERFACE_POSTCALL;
	
	if (lpProgress)
		lpProgress->Release();
	
	if (FAILED(hRes))
		return OleSetOleError(hRes);

	return Py_BuildValue("i", hRes);
}
%}

// @pyswig int|DeleteMessages|Deletes the specified messages.
HRESULT_KEEP_INFO DeleteMessages(
	SBinaryArray *INPUT, // @pyparm <o PySBinaryArray>|msgs||
	unsigned long ulUIParam, // @pyparm int|uiParam||A HWND for the progress
	IMAPIProgress *INPUT_NULLOK,// @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	unsigned long ulFlags); // @pyparm int|flags||

// @pyswig int|EmptyFolder|deletes all messages and subfolders from a folder without deleting the folder itself.
HRESULT_KEEP_INFO EmptyFolder(
	ULONG ulUIParam, // @pyparm int|uiParam||A HWND for the progress
	IMAPIProgress *INPUT_NULLOK, // @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	ULONG ulFlags // @pyparm int|flags||
); 
 
// @pyswig |SetReadFlags|Sets or clears the MSGFLAG_READ flag in the PR_MESSAGE_FLAGS (PidTagMessageFlags) property of one or more of the folder's messages, and manages the sending of read reports.
HRESULT_KEEP_INFO SetReadFlags(
	SBinaryArray *INPUT, // @pyparm <o PySBinaryArray>|msgs||
	ULONG ulUIParam, // @pyparm int|uiParam||A HWND for the progress
	IMAPIProgress *INPUT_NULLOK, // @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	ULONG ulFlags // @pyparm int|flag||Bitmask of flags that controls the setting of a message's read flag - that is, the message's MSGFLAG_READ flag in its PR_MESSAGE_FLAGS property and the processing of read reports.
);
