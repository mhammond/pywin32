/* File : mapi.i */

/* 
   This is designed to be an interface to the MAPI API

   Note that the "exchange" module provides alot of MAPI-like
   functions, although technically these are not part of MAPI.

   The intent is that this module implementes the "official" API,
   although sometimes it is unclear if a function a truly MAPI or not
*/

%module mapi // A COM interface to MAPI


//%{
//#define UNICODE
//%}


%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"
#include <mapiutil.h>
#include "PyIMAPIProp.h"
#include "PyIMAPIStatus.h"
#include "PyIMAPITable.h"
#include "PyIMAPISession.h"
#include "PyIMAPIContainer.h"
#include "PyIMAPIFolder.h"
#include "PyIMessage.h"
#include "PyIMsgStore.h"
#include "PyIAttach.h"
#include "PyIProfAdmin.h"
#include "PyIAddrBook.h"
#include "PyIMailUser.h"
#include "PyIDistList.h"
#include "PyIABContainer.h"
#include "PyIProfSect.h"
#include "PyIMsgServiceAdmin.h"
#include "PyIMAPIAdviseSink.h"
#include "IConverterSession.h"
#include "PyIConverterSession.h"

#include "MAPISPI.H"
#include "MAPISPI.H"
#include "IMESSAGE.H"
%}

%{
static int AddIID(PyObject *dict, const char *key, REFGUID guid)
{
	PyObject *obiid = PyWinObject_FromIID(guid);
	if (!obiid) return 1;
	int rc = PyDict_SetItemString(dict, (char*)key, obiid);
	Py_DECREF(obiid);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)
#define ADD_IID(tok) AddIID(d, #tok, tok)


// @pyswig <o PyUnicode>|HexFromBin|converts a binary number into a string representation of a hexadecimal number.
// @comm Note: This function may not be supported in future versions of MAPI.
static PyObject *PyHexFromBin(PyObject *self, PyObject *args) 
{
	char *szData;
	int dataSize;
	// @pyparm string|val||Converts an EntryID into a hex string representation.
	if (!PyArg_ParseTuple(args, "s#:HexFromBin", &szData, &dataSize))
		return NULL;
	TCHAR *buf = (TCHAR *)malloc(((dataSize*sizeof(TCHAR))*2)+1);
	HexFromBin((LPBYTE)szData, dataSize, buf);
	PyObject *result;
	result = PyWinObject_FromTCHAR(buf);
	free(buf);
	return result;
}

// @pyswig <o PyUnicode>|BinFromHex|converts a hexadecimal number into a binary string
static PyObject *PyBinFromHex(PyObject *self, PyObject *args) 
{
	PyObject *obHex;
	// @pyparm string/<o PyUnicode>|val||The string to be converted.
	if (!PyArg_ParseTuple(args, "O:BinFromHex", &obHex))
		return NULL;
	DWORD strSize;
	TCHAR *tchar;
	if (!PyWinObject_AsTCHAR( obHex, &tchar, FALSE, &strSize ))
		return NULL;

	BYTE *buf = (BYTE *)malloc(((strSize*sizeof(TCHAR))/2)+1);
	if (!FBinFromHex(tchar, buf)) {
		PyErr_SetString(PyExc_ValueError, "FBinFromHex failed - input data is invalid");
		return NULL;
	}
	PyObject *rc = PyString_FromStringAndSize((char *)buf, strSize/2);
	free(buf);
	PyWinObject_FreeTCHAR(tchar);
	return rc;
}

// @pyswig |MAPIUninitialize|Decrements the reference count, cleans up, and deletes per-instance global data for the MAPI DLL.
static PyObject *PyMAPIUninitialize(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":MAPIUninitialize"))
		return NULL;
	PyObject *rc;
	__try {
		MAPIUninitialize();
		rc = Py_None;
		Py_INCREF(Py_None);
	}
	__except (GetExceptionCode() == STATUS_INVALID_HANDLE) {
		PyWin_SetAPIError("MAPIUninitialize", ERROR_INVALID_HANDLE);
		rc = NULL;
	}
	return rc;
}

%}

%init %{
	if ( PyCom_RegisterClientType(&PyIMAPISession::type, &IID_IMAPISession) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPISession);

	if ( PyCom_RegisterClientType(&PyIMAPIStatus::type, &IID_IMAPIStatus) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPIStatus);

	if ( PyCom_RegisterClientType(&PyIMAPITable::type, &IID_IMAPITable) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPITable);

	if ( PyCom_RegisterClientType(&PyIMAPIProp::type, &IID_IMAPIProp) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPIProp);

	if ( PyCom_RegisterClientType(&PyIMAPIFolder::type, &IID_IMAPIFolder) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPIFolder);

	if ( PyCom_RegisterClientType(&PyIMAPIContainer::type, &IID_IMAPIContainer) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPIContainer);

	if ( PyCom_RegisterClientType(&PyIMessage::type, &IID_IMessage) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMessage);

	if ( PyCom_RegisterClientType(&PyIMsgStore::type, &IID_IMsgStore) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMsgStore);

	if ( PyCom_RegisterClientType(&PyIAttach::type, &IID_IAttachment) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IAttachment);

	if ( PyCom_RegisterClientType(&PyIProfAdmin::type, &IID_IProfAdmin) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IProfAdmin);

	if ( PyCom_RegisterClientType(&PyIAddrBook::type, &IID_IAddrBook) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IAddrBook);

	if ( PyCom_RegisterClientType(&PyIDistList::type, &IID_IDistList) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IDistList);

	if ( PyCom_RegisterClientType(&PyIMailUser::type, &IID_IMailUser) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMailUser);

	if ( PyCom_RegisterClientType(&PyIABContainer::type, &IID_IABContainer) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IABContainer);

	if ( PyCom_RegisterClientType(&PyIProfSect::type, &IID_IProfSect) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IProfSect);

	if ( PyCom_RegisterClientType(&PyIMsgServiceAdmin::type, &IID_IMsgServiceAdmin) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMsgServiceAdmin);

        if ( PyCom_RegisterGatewayObject(IID_IMAPIAdviseSink, GET_PYGATEWAY_CTOR(PyGMAPIAdviseSink), "IMAPIAdviseSink") != 0) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IMAPIAdviseSink);

	if ( PyCom_RegisterClientType(&PyIConverterSession::type, &IID_IConverterSession) != 0 ) return MODINIT_ERROR_RETURN;
	{
		ADD_IID(IID_IConverterSession);
		ADD_IID(CLSID_IConverterSession);
	}
	
	ADD_IID(PS_PUBLIC_STRINGS);
	ADD_IID(PS_MAPI);
	ADD_IID(PS_ROUTING_EMAIL_ADDRESSES);
	ADD_IID(PS_ROUTING_ADDRTYPE);
	ADD_IID(PS_ROUTING_DISPLAY_NAME);
	ADD_IID(PS_ROUTING_ENTRYID);
	ADD_IID(PS_ROUTING_SEARCH_KEY);
%}

#define NO_ATTACHMENT NO_ATTACHMENT // The attachment has just been created. 
#define ATTACH_BY_VALUE ATTACH_BY_VALUE // The PR_ATTACH_DATA_BIN property contains the attachment data. 
#define ATTACH_BY_REFERENCE ATTACH_BY_REFERENCE // The PR_ATTACH_PATHNAME or PR_ATTACH_LONG_PATHNAME property contains a fully qualified path identifying the attachment to recipients with access to a common file server. 
#define ATTACH_BY_REF_RESOLVE ATTACH_BY_REF_RESOLVE // The PR_ATTACH_PATHNAME or PR_ATTACH_LONG_PATHNAME property contains a fully qualified path identifying the attachment. 
#define ATTACH_BY_REF_ONLY ATTACH_BY_REF_ONLY // The PR_ATTACH_PATHNAME or PR_ATTACH_LONG_PATHNAME property contains a fully qualified path identifying the attachment.
#define ATTACH_EMBEDDED_MSG ATTACH_EMBEDDED_MSG // The PR_ATTACH_DATA_OBJ property contains an embedded object that supports the IMessage interface. 
#define ATTACH_OLE ATTACH_OLE // The attachment is an embedded OLE object

#define CONVENIENT_DEPTH CONVENIENT_DEPTH // Fills the hierarchy table with containers from multiple levels. If CONVENIENT_DEPTH is not set, the hierarchy table contains only the container's immediate child containers. 

#define FOLDER_GENERIC FOLDER_GENERIC // A generic folder should be created. 
#define FOLDER_SEARCH FOLDER_SEARCH // A search-results folder should be created. 
#define FORCE_SAVE FORCE_SAVE // Changes should be written to the object, overriding any previous changes made to the object, and the object closed. Read/write access must have been set for the operation to succeed. The FORCE_SAVE flag is used after a previous call to SaveChanges returned MAPI_E_OBJECT_CHANGED. 

#define KEEP_OPEN_READONLY KEEP_OPEN_READONLY // Changes should be committed and the object should be kept open for reading. No further changes will be made. 
#define KEEP_OPEN_READWRITE KEEP_OPEN_READWRITE // Changes should be committed and the object should be kept open for read/write access. This flag is usually set when the object was initially opened for read/write access. Subsequent changes to the object are allowed. 

#define MAPI_DIALOG MAPI_DIALOG 
#define MAPI_ASSOCIATED MAPI_ASSOCIATED // The container's associated contents table should be returned rather than the standard contents table. This flag is used only with folders. The messages that are included in the associated contents table were created with the MAPI_ASSOCIATED flag set in the call to IMAPIFolder::CreateMessage. Clients typically use the associated contents table to retrieve forms and views. 
#define MAPI_ALLOW_OTHERS MAPI_ALLOW_OTHERS // The shared session should be returned, allowing subsequent clients to acquire the session without providing any user credentials. 
#define MAPI_EXPLICIT_PROFILE MAPI_EXPLICIT_PROFILE // The default profile should not be used, and the user should be required to supply a profile. 
#define MAPI_EXTENDED MAPI_EXTENDED // Log on with extended capabilities. This flag should always be set. The older MAPILogon function is no longer available. 
#define MAPI_FORCE_DOWNLOAD MAPI_FORCE_DOWNLOAD // An attempt should be made to download all of the user's messages before returning. If the MAPI_FORCE_DOWNLOAD flag is not set, messages can be downloaded in the background after the call to MAPILogonEx returns. 
#define MAPI_LOGON_UI MAPI_LOGON_UI // A dialog box should be displayed to prompt the user for logon information if required. When the MAPI_LOGON_UI flag is not set, the calling client does not display a logon dialog box and returns an error value if the user is not logged on. MAPI_LOGON_UI and MAPI_PASSWORD_UI are mutually exclusive. 
#define MAPI_NEW_SESSION MAPI_NEW_SESSION // An attempt should be made to create a new MAPI session rather than acquire the shared session. If the MAPI_NEW_SESSION flag is not set, MAPILogonEx uses an existing shared session even if the lpszprofileName parameter is not NULL. 
#define MAPI_NO_MAIL MAPI_NO_MAIL // MAPI should not inform the MAPI spooler of the session's existence. The result is that no messages can be sent or received within the session except through a tightly coupled store and transport pair. A calling client sets this flag if it is acting as an agent, if configuration work must be done, or if the client is browsing the available message stores. 
#define MAPI_MULTITHREAD_NOTIFICATIONS MAPI_MULTITHREAD_NOTIFICATIONS // MAPI should generate notifications using a thread dedicated to notification handling rather than the first thread used to call <om mapi.MAPIInitialize>.
#define MAPI_NT_SERVICE MAPI_NT_SERVICE // The caller is running as a Windows NT service. Callers that are not running as a Windows NT service should not set this flag; callers that are running as a service must set this flag. 
#define MAPI_PASSWORD_UI MAPI_PASSWORD_UI // A dialog box should be displayed to prompt the user for the profile password. MAPI_PASSWORD_UI cannot be set if MAPI_LOGON_UI is set because the calling client can only present one of the two dialog boxes. This dialog box does not allow the profile name to be changed; the lpszProfileName parameter must be non-NULL. 
#define MAPI_SERVICE_UI_ALWAYS MAPI_SERVICE_UI_ALWAYS // MAPILogonEx should display a configuration dialog box for each message service in the profile. The dialog boxes are displayed after the profile has been chosen but before any message service is logged on. The MAPI common dialog box for logon also contains a check box that requests the same operation. 
#define MAPI_TIMEOUT_SHORT MAPI_TIMEOUT_SHORT // The logon should fail if blocked for more than a few seconds. 
#define MAPI_UNICODE MAPI_UNICODE // The passed-in strings are in Unicode format. If the MAPI_UNICODE flag is not set, the strings are in ANSI format. 
#define MAPI_USE_DEFAULT MAPI_USE_DEFAULT // The messaging subsystem should substitute the profile name of the default profile for the lpszProfileName parameter. The MAPI_EXPLICIT_PROFILE flag is ignored unless lpszProfileName is NULL or empty. 
#define MAPI_BEST_ACCESS MAPI_BEST_ACCESS
#define MAPI_MODIFY MAPI_MODIFY

#define MAPI_DEFERRED_ERRORS MAPI_DEFERRED_ERRORS // Allows a method to return successfully, possibly before the changes have been fully committed. 
#define MAPI_INIT_VERSION MAPI_INIT_VERSION

#define MDB_NO_DIALOG MDB_NO_DIALOG // Prevents the display of logon dialog boxes. If this flag is set, and OpenMsgStore does not have enough configuration information to open the message store without the user's help, it returns MAPI_E_LOGON_FAILED. If this flag is not set, the message store provider can prompt the user to correct a name or password, to insert a disk, or to perform other actions necessary to establish connection to the message store. 
#define MDB_NO_MAIL MDB_NO_MAIL // The message store should not be used for sending or receiving mail. When this flag is set, MAPI does not notify the MAPI spooler that this message store is being opened. 
#define MDB_TEMPORARY MDB_TEMPORARY // Instructs MAPI that the message store is not permanent and should not be added to the message store table. This flag is used to log on the message store so that information can be retrieved programmatically from the profile section. 
#define MDB_WRITE MDB_WRITE // Requests read/write access to the message store. 

#define OPEN_IF_EXISTS OPEN_IF_EXISTS // Does not fail if the specified folder already exists.
#define RTF_SYNC_BODY_CHANGED RTF_SYNC_BODY_CHANGED // The plain text version of the message has changed. 
#define RTF_SYNC_RTF_CHANGED RTF_SYNC_RTF_CHANGED // The RTF version of the message has changed. 

#define DEL_FOLDERS DEL_FOLDERS // All subfolders of the subfolder pointed to by lpEntryID should be deleted.
#define DEL_MESSAGES DEL_MESSAGES // All messages in the subfolder pointed to by lpEntryID should be deleted.
#define FOLDER_DIALOG FOLDER_DIALOG // A progress indicator should be displayed while the operation proceeds.
#define MESSAGE_DIALOG MESSAGE_DIALOG // Displays a progress indicator as the operation proceeds.
#define SHOW_SOFT_DELETES ((ULONG) 0x00000002) // Shows items that are currently marked as soft deleted.
#define DELETE_HARD_DELETE ((ULONG) 0x00000010) // Permanently removes all messages, including soft-deleted ones.

#define MAPI_CREATE MAPI_CREATE // The object will be created if necessary.
#define MAPI_E_CALL_FAILED MAPI_E_CALL_FAILED						
#define MAPI_E_NOT_ENOUGH_MEMORY MAPI_E_NOT_ENOUGH_MEMORY
#define MAPI_E_INVALID_PARAMETER MAPI_E_INVALID_PARAMETER
#define MAPI_E_INTERFACE_NOT_SUPPORTED MAPI_E_INTERFACE_NOT_SUPPORTED
#define MAPI_E_NO_ACCESS MAPI_E_NO_ACCESS

#define MAPI_E_NO_SUPPORT MAPI_E_NO_SUPPORT
#define	MAPI_E_BAD_CHARWIDTH MAPI_E_BAD_CHARWIDTH
#define MAPI_E_STRING_TOO_LONG MAPI_E_STRING_TOO_LONG
#define MAPI_E_UNKNOWN_FLAGS MAPI_E_UNKNOWN_FLAGS
#define MAPI_E_INVALID_ENTRYID MAPI_E_INVALID_ENTRYID
#define MAPI_E_INVALID_OBJECT MAPI_E_INVALID_OBJECT
#define MAPI_E_OBJECT_CHANGED MAPI_E_OBJECT_CHANGED
#define MAPI_E_OBJECT_DELETED MAPI_E_OBJECT_DELETED
#define MAPI_E_BUSY MAPI_E_BUSY
#define MAPI_E_NOT_ENOUGH_DISK MAPI_E_NOT_ENOUGH_DISK
#define MAPI_E_NOT_ENOUGH_RESOURCES MAPI_E_NOT_ENOUGH_RESOURCES
#define MAPI_E_NOT_FOUND MAPI_E_NOT_FOUND
#define MAPI_E_VERSION MAPI_E_VERSION
#define MAPI_E_LOGON_FAILED MAPI_E_LOGON_FAILED
#define MAPI_E_SESSION_LIMIT MAPI_E_SESSION_LIMIT
#define MAPI_E_USER_CANCEL MAPI_E_USER_CANCEL
#define MAPI_E_UNABLE_TO_ABORT MAPI_E_UNABLE_TO_ABORT
#define MAPI_E_NETWORK_ERROR MAPI_E_NETWORK_ERROR
#define MAPI_E_DISK_ERROR MAPI_E_DISK_ERROR
#define MAPI_E_TOO_COMPLEX MAPI_E_TOO_COMPLEX
#define MAPI_E_BAD_COLUMN MAPI_E_BAD_COLUMN
#define MAPI_E_EXTENDED_ERROR MAPI_E_EXTENDED_ERROR
#define MAPI_E_COMPUTED MAPI_E_COMPUTED
#define MAPI_E_CORRUPT_DATA MAPI_E_CORRUPT_DATA
#define MAPI_E_UNCONFIGURED MAPI_E_UNCONFIGURED
#define MAPI_E_FAILONEPROVIDER MAPI_E_FAILONEPROVIDER
#define MAPI_E_UNKNOWN_CPID MAPI_E_UNKNOWN_CPID
#define MAPI_E_UNKNOWN_LCID MAPI_E_UNKNOWN_LCID

/* Flavors of E_ACCESSDENIED, used at logon */

#define MAPI_E_PASSWORD_CHANGE_REQUIRED MAPI_E_PASSWORD_CHANGE_REQUIRED
#define MAPI_E_PASSWORD_EXPIRED MAPI_E_PASSWORD_EXPIRED
#define MAPI_E_INVALID_WORKSTATION_ACCOUNT MAPI_E_INVALID_WORKSTATION_ACCOUNT
#define MAPI_E_INVALID_ACCESS_TIME MAPI_E_INVALID_ACCESS_TIME
#define MAPI_E_ACCOUNT_DISABLED MAPI_E_ACCOUNT_DISABLED

/* MAPI base function and status object specific errors and warnings */

#define MAPI_E_END_OF_SESSION MAPI_E_END_OF_SESSION
#define MAPI_E_UNKNOWN_ENTRYID MAPI_E_UNKNOWN_ENTRYID
#define MAPI_E_MISSING_REQUIRED_COLUMN MAPI_E_MISSING_REQUIRED_COLUMN
#define MAPI_W_NO_SERVICE MAPI_W_NO_SERVICE

/* Property specific errors and warnings */

#define MAPI_E_BAD_VALUE MAPI_E_BAD_VALUE
#define MAPI_E_INVALID_TYPE MAPI_E_INVALID_TYPE
#define MAPI_E_TYPE_NO_SUPPORT MAPI_E_TYPE_NO_SUPPORT
#define MAPI_E_UNEXPECTED_TYPE MAPI_E_UNEXPECTED_TYPE
#define MAPI_E_TOO_BIG MAPI_E_TOO_BIG
#define MAPI_E_DECLINE_COPY MAPI_E_DECLINE_COPY
#define MAPI_E_UNEXPECTED_ID MAPI_E_UNEXPECTED_ID

#define MAPI_W_ERRORS_RETURNED MAPI_W_ERRORS_RETURNED

/* Table specific errors and warnings */

#define MAPI_E_UNABLE_TO_COMPLETE MAPI_E_UNABLE_TO_COMPLETE
#define MAPI_E_TIMEOUT MAPI_E_TIMEOUT
#define MAPI_E_TABLE_EMPTY MAPI_E_TABLE_EMPTY
#define MAPI_E_TABLE_TOO_BIG MAPI_E_TABLE_TOO_BIG

#define MAPI_E_INVALID_BOOKMARK MAPI_E_INVALID_BOOKMARK

#define MAPI_W_POSITION_CHANGED MAPI_W_POSITION_CHANGED
#define MAPI_W_APPROX_COUNT MAPI_W_APPROX_COUNT

/* Transport specific errors and warnings */

#define MAPI_E_WAIT MAPI_E_WAIT
#define MAPI_E_CANCEL MAPI_E_CANCEL
#define MAPI_E_NOT_ME MAPI_E_NOT_ME

#define MAPI_W_CANCEL_MESSAGE MAPI_W_CANCEL_MESSAGE

/* Message Store, Folder, and Message specific errors and warnings */

#define MAPI_E_CORRUPT_STORE MAPI_E_CORRUPT_STORE
#define MAPI_E_NOT_IN_QUEUE MAPI_E_NOT_IN_QUEUE
#define MAPI_E_NO_SUPPRESS MAPI_E_NO_SUPPRESS
#define MAPI_E_COLLISION MAPI_E_COLLISION
#define MAPI_E_NOT_INITIALIZED MAPI_E_NOT_INITIALIZED
#define MAPI_E_NON_STANDARD MAPI_E_NON_STANDARD
#define MAPI_E_NO_RECIPIENTS MAPI_E_NO_RECIPIENTS
#define MAPI_E_SUBMITTED MAPI_E_SUBMITTED
#define MAPI_E_HAS_FOLDERS MAPI_E_HAS_FOLDERS
#define MAPI_E_HAS_MESSAGES MAPI_E_HAS_MESSAGES
#define MAPI_E_FOLDER_CYCLE MAPI_E_FOLDER_CYCLE

#define MAPI_W_PARTIAL_COMPLETION MAPI_W_PARTIAL_COMPLETION

/* Address Book specific errors and warnings */

#define MAPI_E_AMBIGUOUS_RECIP MAPI_E_AMBIGUOUS_RECIP

#define MODRECIP_ADD MODRECIP_ADD // The recipients should be added to the recipient list. 

#define MODRECIP_MODIFY MODRECIP_MODIFY // The recipients should replace existing recipients. All of the existing properties are replaced by those in the corresponding ADRENTRY structure. 

#define MODRECIP_REMOVE MODRECIP_REMOVE // Existing recipients should be removed from the recipient list using as an index the PR_ROWID property included in the property value array of each recipient entry in the mods parameter.

#define MAPI_TO MAPI_TO // The recipient is a primary (To) recipient. Clients are required to handle primary recipients; all other types are optional. 

#define MAPI_CC MAPI_CC // The recipient is a carbon copy (CC) recipient, a recipient that receives a message in addition to the primary recipients. 

#define MAPI_BCC MAPI_BCC // The recipient is a blind carbon copy (BCC) recipient. Primary and carbon copy recipients are unaware of the existence of BCC recipients. 

#define MAPI_P1 MAPI_P1 // The recipient did not successfully receive the message on the previous attempt. This is a resend of an earlier transmission. 

#define MAPI_SUBMITTED MAPI_SUBMITTED // The recipient has already received the message and does not need to receive it again. This is a resend of an earlier transmission. This flag is set in conjunction with the MAPI_TO, MAPI_CC, and MAPI_BCC values. 

#define MAPI_DEFAULT_SERVICES MAPI_DEFAULT_SERVICES // MAPI should populate the new profile with the message services that are included in the [Default Services] section of the MAPISVC.INF file. 

#define MAPI_NO_IDS MAPI_NO_IDS // Requests that only names stored as Unicode strings be returned. 

#define MAPI_NO_STRINGS MAPI_NO_STRINGS // Requests that only names stored as numeric identifiers be returned. 

// #define MSG_SERVICE_UI_READ_ONLY MSG_SERVICE_UI_READ_ONLY // The message service should display its configuration property sheet but not enable the user to change it. Most message services ignore this flag.

#define SERVICE_UI_ALLOWED SERVICE_UI_ALLOWED // The message service should display its configuration property sheet only if the service is not completely configured. 

#define SERVICE_UI_ALWAYS SERVICE_UI_ALWAYS // The message service must always display its configuration property sheet. If SERVICE_UI_ALWAYS is not set, a configuration property sheet can still be displayed if SERVICE_UI_ALLOWED is set and valid configuration information is not available from the property value array in the lpProps parameter. Either SERVICE_UI_ALLOWED or SERVICE_UI_ALWAYS must be set for a property sheet to be displayed.

#define AB_NO_DIALOG AB_NO_DIALOG 

#define BOOKMARK_BEGINNING BOOKMARK_BEGINNING // Starts the seek operation from the beginning of the table. 
#define BOOKMARK_CURRENT BOOKMARK_CURRENT // Starts the seek operation from the row in the table where the cursor is located. 
#define BOOKMARK_END BOOKMARK_END // Starts the seek operation from the end of the table.

#define TBL_ASYNC TBL_ASYNC // Starts the operation asynchronously and returns before the operation completes. 

#define TBL_BATCH TBL_BATCH // Defers evaluation of the filter until the data in the table is required.

#define RES_AND RES_AND // SRestriction structure describes an AND restriction, which applies a bitwise AND operation to a restriction. 

#define RES_BITMASK RES_BITMASK // SRestriction structure describes a bitmask restriction, which applies a bitmask to a property value. 

#define RES_COMMENT RES_COMMENT // SRestriction structure describes a comment restriction, which associates a comment with a restriction. 

#define RES_COMPAREPROPS RES_COMPAREPROPS // SRestriction structure describes a compare properties restriction, which compares two property values. 

#define RES_CONTENT RES_CONTENT // SRestriction structure describes a content restriction, which searches a property value for specific content. 

#define RES_EXIST RES_EXIST // SRestriction structure describes an exist restriction, which determines if a property is supported. 

#define RES_NOT RES_NOT // SRestriction structure describes a NOT restriction, which applies a logical NOT operation to a restriction. 

#define RES_OR RES_OR // SRestriction structure describes an OR restriction, which applies a logical OR operation to a restriction. 

#define RES_PROPERTY RES_PROPERTY // SRestriction structure describes a property restriction, which determines if a property value matches a particular value. 

#define RES_SIZE RES_SIZE // SRestriction structure describes a size restriction, which determines if a property value is a particular size. 

#define RES_SUBRESTRICTION RES_SUBRESTRICTION // SRestriction structure describes a subobject restriction, which applies a restriction to a message's attachments or recipients. 

#define RELOP_GE RELOP_GE // The comparison is made based on a greater or equal first value. 

#define RELOP_GT RELOP_GT // The comparison is made based on a greater first value. 

#define RELOP_LE RELOP_LE // The comparison is made based on a lesser or equal first value. 

#define RELOP_LT RELOP_LT // The comparison is made based on a lesser first value. 

#define RELOP_NE RELOP_NE // The comparison is made based on unequal values. 

#define RELOP_RE RELOP_RE // The comparison is made based on LIKE (regular expression) values. 

#define RELOP_EQ RELOP_EQ // The comparison is made based on equal values. 

#define BMR_EQZ BMR_EQZ // Perform a bitwise AND operation of the mask in the ulMask member with the property represented by the ulPropTag member and test for being equal to zero. 

#define BMR_NEZ BMR_NEZ // Perform a bitwise AND operation of the mask in the ulMask member with the property represented by the ulPropTag member and test for being not equal to zero. 

#define DIR_BACKWARD DIR_BACKWARD // Searches backward from the row identified by the bookmark. 

#define FL_FULLSTRING FL_FULLSTRING // To match, the lpProp search string must be completely contained in the property identified by ulPropTag. 

#define FL_PREFIX FL_PREFIX // To match, the lpProp search string must appear at the beginning of the property identified by ulPropTag. The two strings should be compared only up to the length of the search string indicated by lpProp. 

#define FL_SUBSTRING FL_SUBSTRING // To match, the lpProp search string must be contained anywhere within the property identified by ulPropTag. 

#define FL_IGNORECASE FL_IGNORECASE // The comparison should be made without considering case. 

#define FL_IGNORENONSPACE FL_IGNORENONSPACE // The comparison should ignore Unicode-defined nonspacing characters such as diacritical marks. 

#define FL_LOOSE FL_LOOSE // The comparison should result in a match whenever possible, ignoring case and nonspacing characters

#define STATUS_DEFAULT_STORE STATUS_DEFAULT_STORE

#define TABLE_SORT_ASCEND TABLE_SORT_ASCEND // The table should be sorted in ascending order. 

#define TABLE_SORT_COMBINE TABLE_SORT_COMBINE // The sort operation should create a category that combines the property identified as the sort key column in the ulPropTag member with the sort key column specified in the previous SSortOrder structure.<nl>TABLE_SORT_COMBINE can only be used when the SSortOrder structure is being used as an entry in an SSortOrderSet structure to specify multiple sort orders for a categorized sort. TABLE_SORT_COMBINE cannot be used in the first SSortOrder structure in an SSortOrderSet structure. 

#define TABLE_SORT_DESCEND TABLE_SORT_DESCEND // The table should be sorted in descending order. 

#define TBL_ALL_COLUMNS TBL_ALL_COLUMNS  // The table should return all available columns. 

// IMAPIStatus consts.
#define STATUS_FLUSH_QUEUES STATUS_FLUSH_QUEUES
#define STATUS_INBOUND_FLUSH STATUS_INBOUND_FLUSH
#define STATUS_OUTBOUND_FLUSH STATUS_OUTBOUND_FLUSH
#define FLUSH_UPLOAD FLUSH_UPLOAD
#define FLUSH_DOWNLOAD FLUSH_DOWNLOAD
#define FLUSH_FORCE FLUSH_FORCE
#define FLUSH_NO_UI FLUSH_NO_UI
#define FLUSH_ASYNC_OK FLUSH_ASYNC_OK

// IConverterSession Constants - http://msdn2.microsoft.com/en-us/library/bb905201.aspx
#define CCSF_SMTP             CCSF_SMTP // the converter is being passed an SMTP message
#define CCSF_NOHEADERS        CCSF_NOHEADERS // the converter should ignore the headers on the outside message
#define CCSF_USE_TNEF         CCSF_USE_TNEF // the converter should embed TNEF in the MIME message
#define CCSF_INCLUDE_BCC      CCSF_INCLUDE_BCC // the converter should include Bcc recipients
#define CCSF_8BITHEADERS      CCSF_8BITHEADERS // the converter should allow 8 bit headers
#define CCSF_USE_RTF          CCSF_USE_RTF // the converter should do HTML->RTF conversion
#define CCSF_PLAIN_TEXT_ONLY  CCSF_PLAIN_TEXT_ONLY // the converter should just send plain text
#define CCSF_NO_MSGID         CCSF_NO_MSGID // don't include Message-Id field in outgoing messages
#define CCSF_EMBEDDED_MESSAGE CCSF_EMBEDDED_MESSAGE // sent/unsent information is persisted in X-Unsent
#define CCSF_PRESERVE_SOURCE  CCSF_PRESERVE_SOURCE // don't modify the source message

// StreamOnFile (SOF)
#define SOF_UNIQUEFILENAME	SOF_UNIQUEFILENAME // A temporary file is to be created for the IStream object

// @object MAPIINIT_0|A MAPIINIT_0 is represented as a tuple of:
// @tupleitem 0|int|version|This must be MAPI_INIT_VERSION.
// @tupleitem 1|int|flags|MAPI initlization flags.
// @flagh Value|Meaning
// @flag MAPI_MULTITHREAD_NOTIFICATIONS|MAPI should generate notifications using a thread dedicated to notification handling rather than the first thread used to call <om mapi.MAPIInitialize>.
// @flag MAPI_NT_SERVICE|The caller is running as a NT service. Callers that are not running in a Windows NT service should not set this flag; callers that are running as a service must set this flag.
// @comm Multithreaded clients should set MAPI_MULTITHREAD_NOTIFICATIONS flag so that they can receive notifications on threads other than the first thread to call <om mapi.MAPIInitialize>.

// @pyswig |MAPIInitialize|Increments the MAPI subsystem reference count and initializes global data for the MAPI DLL. 
HRESULT MAPIInitialize
(
	MAPIINIT_0 *INPUT // @pyparm <o MAPIINIT_0>|init||MAPI Initialization flags.
);

// @pyswig <o PyIMAPISession>|MAPILogonEx|
HRESULT MAPILogonEx( 
	ULONG INPUT, // @pyparm int|hWnd||Handle to the window to which the logon dialog box is modal. If no dialog box is displayed during the call, the hWnd parameter is ignored. This parameter can be zero. 
	TCHAR *inNullString, // @pyparm <o PyUnicode>|profileName||A string containing the name of the profile to use when logging on. This string is limited to 64 characters.
	TCHAR *inNullString, // @pyparm <o PyUnicode>|password||A string containing the password of the profile. This parameter can be None whether or not the profileName parameter is None. This string is limited to 64 characters.
	FLAGS flFlags, // @pyparm int|uiFlags||Bitmask of flags used to control how logon is performed.  See the MAPI documentation for details.
	IMAPISession **OUTPUT 
);

// @pyswig <o PyIProfAdmin>|MAPIAdminProfiles|
HRESULT MAPIAdminProfiles( 
	unsigned long ulFlags, // @pyparm int|fFlags||
	IProfAdmin **OUTPUT
);

%native (MAPIUninitialize) PyMAPIUninitialize;

%native (HexFromBin) PyHexFromBin;
%native (BinFromHex) PyBinFromHex;


// @pyswig <o SRowSet>|HrQueryAllRows|
HRESULT HrQueryAllRows( 
	IMAPITable *INPUT, // @pyparm <o PyIMAPITable>|table||
	SPropTagArray *INPUT, // @pyparm <o PySPropTagArray>|properties||A sequence of property tags indicating table columns. These tags are used to select the specific columns to be retrieved. If this parameter is None, HrQueryAllRows retrieves the entire column set of the current table view passed in the table parameter. 
	SRestriction *INPUT, // @pyparm <o PySRestriction>|restrictions||Defines the retrieval restrictions. If this parameter is None, HrQueryAllRows makes no restrictions. 
	SSortOrderSet *INPUT, // @pyparm <o PySSortOrderSet>|sortOrderSet||Identifies the sort order of the columns to be retrieved. If this parameter is None, the default sort order for the table is used.
	long crowsMax, // @pyparm int|rowsMax||Maximum number of rows to be retrieved. If the value of the rowsMax parameter is zero, no limit on the number of rows retrieved is set.
	SRowSet **OUTPUT);

// @pyswig int|RTFSync|
HRESULT RTFSync(
	IMessage *INPUT, // @pyparm <o PyIMessage>|message||The message.
	unsigned long ulFlags, // @pyparm int|flags||
	int *OUTPUT // lpfMessageUpdated 
);

// @pyswig <o PyIStream>|WrapCompressedRTFStream|
HRESULT WrapCompressedRTFStream(
  IStream *INPUT, // @pyparm <o PyIStream>|stream||Message stream
  unsigned long ulflags, // @pyparm int|flags||
  IStream **OUTPUT
);

%native(MAPIUIDFromBinary) MAPIUIDFromBinary;
%{
PyObject *MAPIUIDFromBinary(PyObject *self, PyObject *args)
{
	char *szVal;
	int szSize;
	if (!PyArg_ParseTuple(args, "s#:MAPIUIDFromBinary", &szVal, &szSize))
		return NULL;
	if (szSize != sizeof(MAPIUID)) {
		PyErr_SetString(PyExc_ValueError, "The string is not a valid MAPIUID (bad size)");
		return NULL;
	}
	MAPIUID uid;
	memcpy(&uid, szVal, szSize);
	GUID *pTemp = (GUID *)&uid;
	return PyWinObject_FromIID( *pTemp );
}
%}

// @pyswig object|OpenIMsgSession|
%native(OpenIMsgSession) PyOpenIMsgSession;
%{
PyObject *PyOpenIMsgSession(PyObject *self, PyObject *args)
{
	long flags = 0;
	if (!PyArg_ParseTuple(args, "|l:OpenIMsgOnIStg", &flags))
		return NULL;
	LPMALLOC pMalloc = MAPIGetDefaultMalloc();
	LPMSGSESS pSession = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = ::OpenIMsgSession( pMalloc, flags, &pSession);
	pMalloc->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return OleSetOleError(hr);
	return PyInt_FromLong((long)pSession);
}
%}
// @pyswig |CloseIMsgSession|
%native(CloseIMsgSession) PyCloseIMsgSession;
%{
PyObject *PyCloseIMsgSession(PyObject *self, PyObject *args)
{
	long session = 0;
	if (!PyArg_ParseTuple(args, "l:CloseIMsgSession", &session))
		return NULL;
	PY_INTERFACE_PRECALL;
	::CloseIMsgSession( (LPMSGSESS)session );
	PY_INTERFACE_POSTCALL;
	Py_INCREF(Py_None);
	return Py_None;
}
%}


// @pyswig <o PyIMessage>|OpenIMsgOnIStg|Builds a new IMessage object on top of an existing OLE IStorage object, to be used within a message session. 
%native(OpenIMsgOnIStg) PyOpenIMsgOnIStg;
%{
PyObject *PyOpenIMsgOnIStg(PyObject *self, PyObject *args)
{
	PyObject *obMapiSupport;
	PyObject *obStorage;
	PyObject *obCallRelease = Py_None;
	long dwCallRelFlags = 0;
	long flags = 0;
	HRESULT hr = E_FAIL;
	PyObject *rc = NULL;
	long lSession;

	if (!PyArg_ParseTuple(args, "lOO|Oll:OpenIMsgOnIStg",
		&lSession, // @pyparm object|session||
		&obMapiSupport, // @pyparm <o PyIMAPISupport>|support||May be None
		&obStorage, // @pyparm <o PyIStorage>|storage||A <o PyIStorage> object that is open and has read-only or read/write access. Because IMessage does not support write-only access, OpenIMsgOnIStg does not accept a storage object opened in write-only mode.
		&obCallRelease, // @pyparm object|callback|None|Only None is supported.
		&dwCallRelFlags, // @pyparm int|callbackData|0|
		&flags)) // @pyparm int|flags|0|
		return NULL;

	if (obMapiSupport != Py_None || obCallRelease != Py_None) {
		PyErr_SetString(PyExc_TypeError, "Only None us supported for MAPISupport and CallRelease");
		return NULL;
	}
	IMAPISupport *pSupport = NULL;
	IStorage *pStorage = NULL;
	IMessage *pRet = NULL;

	if (!PyCom_InterfaceFromPyObject(obStorage, IID_IStorage, (void **)&pStorage, FALSE))
		goto done;

	{
	PY_INTERFACE_PRECALL;
	LPMALLOC pMalloc = MAPIGetDefaultMalloc();
	hr = ::OpenIMsgOnIStg((LPMSGSESS)lSession,
					  MAPIAllocateBuffer,
					  MAPIAllocateMore,
					  MAPIFreeBuffer,
					  pMalloc,
					  pSupport,
					  pStorage,
					NULL, dwCallRelFlags, flags, &pRet);
	if (pMalloc) pMalloc->Release();
	PY_INTERFACE_POSTCALL;
	}
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto done;
	}
	rc = PyCom_PyObjectFromIUnknown(pRet, IID_IMessage, /*BOOL bAddRef =*/ FALSE);
done:
	if (pSupport) pSupport->Release();
	if (pStorage) pStorage->Release();
	return rc;
}
%}

%{
// Code for converting RTF to HTML.
// Found at http://www.wischik.com/lu/programmer/mapi_utils.html
// MarkH converted it to Python, but was too slow.  Moving to a regex 
// based parser was too much work.

// DECODERTFHTML -- Given an uncompressed RTF body of the message,
// and assuming that it contains encoded-html, this function
// turns it onto regular html.
// [in] (buf,*len) indicate the start and length of the uncompressed RTF body.
// [out] the buffer is overwritten with the HTML version, null-terminated,
// and *len indicates the length of this HTML.
//
// Notes: (1) because of how the encoding works, the HTML version is necessarily
// shorter than the encoded version. That's why it's safe for the function to
// place the decoded html in the same buffer that formerly held the encoded stuff.
// (2) Some messages include characters \'XX, where XX is a hexedecimal number.
// This function simply converts this into ASCII. The conversion will only make
// sense if the right code-page is being used. I don't know how rtf specifies which
// code page it wants.
// (3) By experiment, I discovered that \pntext{..} and \liN and \fi-N are RTF
// markup that should be removed. There might be other RTF markup that should
// also be removed. But I don't know what else.
//
void decodertfhtml(char *buf,unsigned int *len)
{ // c -- pointer to where we're reading from
  // d -- pointer to where we're writing to. Invariant: d<c
  // max -- how far we can read from (i.e. to the end of the original rtf)
  // ignore_tag -- stores 'N': after \mhtmlN, we will ignore the subsequent \htmlN.
  char *c=buf, *max=buf+*len, *d=buf; int ignore_tag=-1;
  // First, we skip forwards to the first \htmltag.
  while (c<max && strncmp(c,"{\\*\\htmltag",11)!=0) c++;
  //
  // Now work through the document. Our plan is as follows:
  // * Ignore { and }. These are part of RTF markup.
  // * Ignore \htmlrtf...\htmlrtf0. This is how RTF keeps its equivalent markup separate from the html.
  // * Ignore \r and \n. The real carriage returns are stored in \par tags.
  // * Ignore \pntext{..} and \liN and \fi-N. These are RTF junk.
  // * Convert \par and \tab into \r\n and \t
  // * Convert \'XX into the ascii character indicated by the hex number XX
  // * Convert \{ and \} into { and }. This is how RTF escapes its curly braces.
  // * When we get \*\mhtmltagN, keep the tag, but ignore the subsequent \*\htmltagN
  // * When we get \*\htmltagN, keep the tag as long as it isn't subsequent to a \*\mhtmltagN
  // * All other text should be kept as it is.
  while (c<max)
  { if (*c=='{') c++;
    else if (*c=='}') c++;
    else if (strncmp(c,"\\*\\htmltag",10)==0)
    { c+=10; int tag=0; while (*c>='0' && *c<='9') {tag=tag*10+*c-'0'; c++;}
      if (*c==' ') c++;
      if (tag==ignore_tag) {while (c<max && *c!='}') c++; if (*c=='}') c++;}
      ignore_tag=-1;
    }
    else if (strncmp(c,"\\*\\mhtmltag",11)==0)
    { c+=11; int tag=0; while (*c>='0' && *c<='9') {tag=tag*10+*c-'0'; c++;}
      if (*c==' ') c++;
      ignore_tag=tag;
    }
    else if (strncmp(c,"\\par",4)==0) {strcpy(d,"\r\n"); d+=2; c+=4; if (*c==' ') c++;}
    else if (strncmp(c,"\\tab",4)==0) {strcpy(d,"   "); d+=3; c+=4; if (*c==' ') c++;}
    else if (strncmp(c,"\\li",3)==0)
    { c+=3; while (*c>='0' && *c<='9') c++; if (*c==' ') c++;
    }
    else if (strncmp(c,"\\fi-",4)==0)
    { c+=4; while (*c>='0' && *c<='9') c++; if (*c==' ') c++;
    }
    else if (strncmp(c,"\\'",2)==0)
    { unsigned int hi=c[2], lo=c[3];
      if (hi>='0' && hi<='9') hi-='0'; else if (hi>='A' && hi<='Z') hi=hi-'A'+10; else if (hi>='a' && hi<='z') hi=hi-'a'+10;
      if (lo>='0' && lo<='9') lo-='0'; else if (lo>='A' && lo<='Z') lo=lo-'A'+10; else if (lo>='a' && lo<='z') lo=lo-'a'+10;
      *((unsigned char*)d) = (unsigned char)(hi*16+lo);
      c+=4; d++;
    }
    else if (strncmp(c,"\\pntext",7)==0) {c+=7; while (c<max && *c!='}') c++;}
    else if (strncmp(c,"\\htmlrtf",8)==0)
    { c++; while (c<max && strncmp(c,"\\htmlrtf0",9)!=0) c++;
      if (c<max) c+=9; if (*c==' ') c++;
    }
    else if (*c=='\r' || *c=='\n') c++;
    else if (strncmp(c,"\\{",2)==0) {*d='{'; d++; c+=2;}
    else if (strncmp(c,"\\}",2)==0) {*d='}'; d++; c+=2;}
    else {*d=*c; c++; d++;}
  }
  *d=0; d++;
  *len = (unsigned int)(d-buf);
}


bool isrtfhtml(const char *buf,unsigned int len)
{ // We look for the words "\fromhtml" somewhere in the file.
  // If the rtf encodes text rather than html, then instead
  // it will only find "\fromtext".
  for (const char *c=buf; c<buf+len; c++)
  { if (strncmp(c,"\\from",5)==0) return strncmp(c,"\\fromhtml",9)==0;
  } return false;
}

// @pyswig |RTFStreamToHTML|
static PyObject *MyRTFStreamToHTML(PyObject *self, PyObject *args)
{
  PyObject *obStream;
  HRESULT hr;
  // @pyparm <o PyIStream>|The stream to read the uncompressed RTF from||
  if  (!PyArg_ParseTuple(args, "O:RTFStreamToHTML", &obStream))
    return NULL;
  IStream *pStream = NULL;

  if (!PyCom_InterfaceFromPyObject(obStream, IID_IStream, (void **)&pStream, FALSE))
    return NULL;

  // all exit from here via 'exit', and no Python until POSTCALL
  PY_INTERFACE_PRECALL;
  PyObject *ret = NULL;
  unsigned int bufsize=10240; 
  char *htmlbuf = (char *)malloc(bufsize);
  unsigned int htmlsize=0; bool done=(htmlbuf==NULL);
  while (!done)
  { ULONG red; hr = pStream->Read(htmlbuf+htmlsize, bufsize-htmlsize, &red);
    if (hr!=S_OK) {htmlbuf[htmlsize]=0; done=true;}
    else
    { htmlsize+=red; done = (red < bufsize-htmlsize);
      if (!done)
      { unsigned int newsize=2*htmlsize;
        htmlbuf = (char *)realloc(htmlbuf, newsize);
        bufsize=newsize;
      }
    }
  }
  bool ok;
  if (htmlbuf) {
    ok = isrtfhtml(htmlbuf,htmlsize);
    if (ok)
      decodertfhtml(htmlbuf,&htmlsize);
  }
  PY_INTERFACE_POSTCALL;
  if (htmlbuf==0) {
    PyErr_NoMemory();
    goto exit;
  }
  if (!ok) {
    Py_INCREF(Py_None);
    ret = Py_None;
    goto exit;
  }
  ret = PyString_FromStringAndSize(htmlbuf, htmlsize-1);
exit:
  if (pStream) pStream->Release();
  if (htmlbuf)
    free(htmlbuf);
  return ret;
}
%}
%native(RTFStreamToHTML) MyRTFStreamToHTML;

// @pyswig <o PyIStream>|OpenStreamOnFile|Allocates and initializes an OLE IStream object to access the contents of a file. 
%native(OpenStreamOnFile) PyOpenStreamOnFile;
%{
PyObject *PyOpenStreamOnFile(PyObject *self, PyObject *args)
{	
		HRESULT hRes;
		unsigned long flags = 0;
		IStream *pStream;
		PyObject *obFileName;
		char *filename = NULL;
		PyObject *obPrefix = Py_None;
		char *prefix = NULL;
		
		if (!PyArg_ParseTuple(args, "O|lO:OpenStreamOnFile",
			&obFileName, // @pyparm string|filename||
			&flags, // @pyparm int|flags|0|
			&obPrefix)) // @pyparm string|prefix|None|
			return NULL;

		if (!PyWinObject_AsString(obFileName, &filename, TRUE))
			goto done;

		if (!PyWinObject_AsString(obPrefix, &prefix, TRUE))
			goto done;

		PY_INTERFACE_PRECALL;
		// mapiutil.h incorrectly declares OpenStreamOnFile taking type LPTSTR
		hRes = OpenStreamOnFile(MAPIAllocateBuffer, MAPIFreeBuffer, flags, (LPTSTR)filename, (LPTSTR)prefix, &pStream);
		PY_INTERFACE_POSTCALL;

	done:
		PyWinObject_FreeString(filename);
		PyWinObject_FreeString(prefix);

		if (PyErr_Occurred())
			return NULL;

		if (FAILED(hRes))
			return OleSetOleError(hRes);	
				
		return PyCom_PyObjectFromIUnknown(pStream, IID_IStream, FALSE);	
}
%}

// @pyswig item|HrGetOneProp|Retrieves the value of a single property from an IMAPIProp object.
%native(HrGetOneProp) PyHrGetOneProp;
%{
PyObject *PyHrGetOneProp(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *obProp;
	ULONG propTag;
	IMAPIProp *pProp = NULL;
	PyObject *ret = NULL;
	SPropValue *pPV = NULL;
	
	if (!PyArg_ParseTuple(args, "Ok:HrGetOneProp",
		&obProp, // @pyparm <o PyIMAPIProp>|prop||Object to retrieve property value from.
		&propTag))// @pyparm ULONG|propTag||The property tag to open.
		return NULL;
		
	if (!PyCom_InterfaceFromPyObject(obProp, IID_IMAPIProp, (void **)&pProp, FALSE))
		goto done;
	
	PY_INTERFACE_PRECALL;
	hRes = HrGetOneProp(pProp, propTag, &pPV);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hRes))
	{
		OleSetOleError(hRes);
		goto done;
	}
	if ((ret = PyMAPIObject_FromSPropValue(pPV)) == NULL)
		goto done;
	
	// PyMAPIObject_FromSPropValue does not raise an exception for types
	// it cannot handle so that GetProps doesn't blow up. Since we are processing
	// only a single item, we test for this condition, and raise an exception.
	if (PyTuple_GET_ITEM(ret, 1) == Py_None &&
		PyLong_AsUnsignedLong(PyTuple_GET_ITEM(ret, 0)) != PT_NULL)
	{
		char buf[128];
		sprintf(buf, "Unsupported MAPI property type 0x%X", PROP_TYPE(pPV->ulPropTag));
		PyErr_SetString(PyExc_TypeError, buf);
		Py_DECREF(ret);
		ret = NULL;
	}
done:
	if (pProp) pProp->Release();
	MAPIFreeBuffer(pPV);
	
	return ret;
}
%}

// @pyswig item|HrSetOneProp|Sets the value of a single property on a IMAPIProp object.
%native(HrSetOneProp) PyHrSetOneProp;
%{
PyObject *PyHrSetOneProp(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *obProp;
	PyObject *obPropValue;
	ULONG propTag;
	IMAPIProp *pProp = NULL;
	PyObject *ret = NULL;
	SPropValue *pPV = NULL;
	
	if (!PyArg_ParseTuple(args, "OO:HrSetOneProp",
		&obProp, // @pyparm <o PyIMAPIProp>|prop||Object to set property value on.
		&obPropValue))// @pyparm <o PySPropValue>|propValue||Property value to set.
		return NULL;
		
	if (!PyCom_InterfaceFromPyObject(obProp, IID_IMAPIProp, (void **)&pProp, FALSE))
		goto done;
	if (S_OK != (hRes=MAPIAllocateBuffer(sizeof(SPropValue), (void **)&pPV)))
	{
		OleSetOleError(hRes);
		goto done;
	}
	if (!PyMAPIObject_AsSPropValue(obPropValue, pPV, pPV))
		goto done;
	
	PY_INTERFACE_PRECALL;
	hRes = HrSetOneProp(pProp, pPV);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hRes))
	{
		OleSetOleError(hRes);
		goto done;
	}
	Py_INCREF(Py_None);
	ret = Py_None;
done:
	if (pProp) pProp->Release();
	MAPIFreeBuffer(pPV);
	
	return ret;
}
	
%}
