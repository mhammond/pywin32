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

// @pyswig <o PyMAPIError>|GetLastError|Returns the last error associated with this object
// @pyparm int|hr||The HRESULT
// @pyparm int|flags||
HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig <o PyIMAPIFolder>|CreateFolder|Creates a folder object.
HRESULT CreateFolder(
	ULONG ulFolderType, // @pyparm int|folderType||The type of folder to create
	TCHAR *INPUT, // @pyparm string|folderName||The name of the folder.
	TCHAR *INPUT_NULLOK, // @pyparm string|folderComment||A comment for the folder or None
	IID *INPUT_NULLOK,    // @pyparm <o PyIID>|iid||The IID of the object to return.  Should usually be None.
	ULONG ulFlags, // @pyparm int|flags||
	IMAPIFolder **OUTPUT);

// @pyswig <o PyIMessage>|CreateMessage|Creates a message in a folder
HRESULT CreateMessage( 
	IID *INPUT_NULLOK,    // @pyparm <o PyIID>|iid||The IID of the object to return.  Should usually be None.
	ULONG ulFlags,	// @pyparm int|flags||
	IMessage **OUTPUT);

// @pyswig |CopyMessages|Copies the specified messages
HRESULT CopyMessages(
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

// @pyswig |DeleteMessages|Deletes the specified messages.
HRESULT DeleteMessages(
	SBinaryArray *INPUT, // @pyparm <o PySBinaryArray>|msgs||
	unsigned long ulUIParam, // @pyparm int|uiParam||A HWND for the progress
	IMAPIProgress *INPUT_NULLOK,// @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	unsigned long ulFlags); // @pyparm int|flags||

// @pyswig |EmptyFolder|deletes all messages and subfolders from a folder without deleting the folder itself.
HRESULT EmptyFolder(
	ULONG ulUIParam, // @pyparm int|uiParam||A HWND for the progress
	IMAPIProgress *INPUT_NULLOK, // @pyparm <o PyIMAPIProgress>|progress||A progress object, or None
	ULONG ulFlags // @pyparm int|flags||
); 
 
