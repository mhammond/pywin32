/* File : exchange.i */

/*
   This is designed to be an interface to the Exchange specific
   MAPI API

   Note that the "mapi" module provides the "official" MAPI API.

   Sometimes it is unclear if a function a truly MAPI or not.
*/

%module exchange // A COM interface to Exchange's API

%{
// #define UNICODE
// #define _UNICODE
%}


%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"

#include "MAPIUtil.h"
#include "EdkMdb.h"

#define USES_IID_IExchangeManageStore
#include <edkguid.h>

#include "PyIExchangeManageStore.h"
#include "IExchangeManageStoreEx.h"
#include "PyIExchangeManageStoreEx.h"

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


#define ADD_IID(tok) AddIID(d, #tok, tok)

%}


// IExchangeManageStore::CreateStoreEntryId flags
#define OPENSTORE_USE_ADMIN_PRIVILEGE OPENSTORE_USE_ADMIN_PRIVILEGE
#define OPENSTORE_PUBLIC OPENSTORE_PUBLIC
#define OPENSTORE_HOME_LOGON OPENSTORE_HOME_LOGON
#define OPENSTORE_TAKE_OWNERSHIP OPENSTORE_TAKE_OWNERSHIP
#define OPENSTORE_OVERRIDE_HOME_MDB OPENSTORE_OVERRIDE_HOME_MDB


%init %{
	if ( PyCom_RegisterClientType(&PyIExchangeManageStore::type, &IID_IExchangeManageStore) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IExchangeManageStore);
	if ( PyCom_RegisterClientType(&PyIExchangeManageStoreEx::type, &IID_IExchangeManageStoreEx) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IExchangeManageStoreEx);
%}

/*
   Only include Ex2KSdk.lib functions for 32-bit builds.
*/
#ifdef SWIG_PY32BIT
%{
#include "EdkMAPI.h"
#include "EdkCfg.h"
#include "EdkUtils.h"
%}

// @pyswig int, int|HrGetExchangeStatus|Obtains the current state of the server on a computer.
// @rdesc The result is a tuple of serviceState, serverState
HRESULT HrGetExchangeStatus(
	char *server, // @pyparm string/<o PyUnicode>|server||The name of the server to query.
	unsigned long *OUTPUT,
	unsigned long *OUTPUT
);

// @pyswig string|HrGetMailboxDN|Retrieves the distinguished name (DN) for a mailbox
HRESULT HrGetMailboxDN(
	IMAPISession *INPUT, // @pyparm <o IMAPISession>|session||The root folder.
	char **OUTPUT_MAPI // mailboxDN
);

// @pyswig string|HrGetServerDN|Retrieves the distinguished name (DN) for a server
HRESULT HrGetServerDN(
	IMAPISession *INPUT, // @pyparm <o IMAPISession>|session||The root folder.
	char **OUTPUT_MAPI // mailboxDN
);

%native(HrMAPIFindDefaultMsgStore) PyHrMAPIFindDefaultMsgStore;
%{
// @pyswig string|HrMAPIFindDefaultMsgStore|Retrieves the entry identifier of the default information store.
static PyObject *PyHrMAPIFindDefaultMsgStore(PyObject *self, PyObject *args)
{
    HRESULT  _result;
	ULONG entryStrLen;
	ENTRYID *pID;
    IMAPISession * pS = NULL;
	PyObject *obSession;

	// @pyparm <o PyIMAPISession>|session||
    if(!PyArg_ParseTuple(args,"O:HrMAPIFindDefaultMsgStore",&obSession))
        return NULL;

	if (!PyCom_InterfaceFromPyInstanceOrObject(obSession, IID_IMAPISession, (void **)&pS, 0))
		return NULL;

     _result = (HRESULT )HrMAPIFindDefaultMsgStore(pS, &entryStrLen, &pID);
     if (FAILED(_result)) {
           return OleSetOleError(_result);
     }
	 PyObject *rc = PyBytes_FromStringAndSize((char *)pID, entryStrLen);
	 MAPIFreeBuffer(pID);
	 return rc;
}
%}

%native(HrMAPIFindIPMSubtree) PyHrMAPIFindIPMSubtree;
%{
// @pyswig string|HrMAPIFindIPMSubtree|Retrieves the entry ID of the IPM (interpersonal message) subtree folder
static PyObject *PyHrMAPIFindIPMSubtree(PyObject *self, PyObject *args)
{
    HRESULT  _result;
	ULONG entryStrLen;
	ENTRYID *pID;
    IMsgStore * pS = NULL;
	PyObject *obStore;

	// @pyparm <o PyIMsgStore>|msgStore||
    if(!PyArg_ParseTuple(args,"O:HrMAPIFindIPMSubtree",&obStore))
        return NULL;

	if (!PyCom_InterfaceFromPyInstanceOrObject(obStore, IID_IMsgStore, (void **)&pS, 0))
		return NULL;

     _result = (HRESULT )HrMAPIFindIPMSubtree(pS, &entryStrLen, &pID);
     if (FAILED(_result)) {
           return OleSetOleError(_result);
     }
	 PyObject *rc = PyBytes_FromStringAndSize((char *)pID, entryStrLen);
	 MAPIFreeBuffer(pID);
	 return rc;
}
%}


%native (HrMAPIFindInbox) PyHrMAPIFindInbox;
%{
// @pyswig string|HrMAPIFindInbox|Retrieves the Entry ID of the IPM inbox folder
static PyObject *PyHrMAPIFindInbox(PyObject *self, PyObject *args)
{
    HRESULT  _result;
	ULONG entryStrLen;
	ENTRYID *pID;
    IMsgStore * pS = NULL;
	PyObject *obStore;

	// @pyparm <o PyIMsgStore>|msgStore||
    if(!PyArg_ParseTuple(args,"O:HrMAPIFindInbox",&obStore))
        return NULL;

	if (!PyCom_InterfaceFromPyInstanceOrObject(obStore, IID_IMsgStore, (void **)&pS, 0))
		return NULL;

     _result = (HRESULT )HrMAPIFindInbox(pS, &entryStrLen, &pID);
     if (FAILED(_result)) {
           return OleSetOleError(_result);
     }
	 PyObject *rc = PyBytes_FromStringAndSize((char *)pID, entryStrLen);
	 MAPIFreeBuffer(pID);
	 return rc;
}
%}



%{
PyObject *MyHrMAPIFindSubfolderEx(
	IMAPIFolder *lpRootFolder,
	TCHAR chSep,
	TCHAR *lpszName)
{
	DWORD idSize;
	ENTRYID *id;
	HRESULT hr = HrMAPIFindSubfolderEx(lpRootFolder,chSep,lpszName,&idSize, &id);
	if (FAILED(hr))
		return OleSetOleError(hr);
	PyObject *rc = PyBytes_FromStringAndSize((char *)id, idSize);
	MAPIFreeBuffer(id);
	return rc;
}
%}

// @pyswig <o PyIMsgStore>|HrMAPIFindSubfolderEx|Retrieves a subfolder in an information store using the hierarchical path name of the folder.
%name(HrMAPIFindSubfolderEx) PyObject *MyHrMAPIFindSubfolderEx(
	IMAPIFolder *INPUT, // @pyparm <o PyIMAPIFolder>|rootFolder||The root folder.
	TCHAR chSep,	// @pyparm string/<o PyUnicode>|sep||The folder separator character.
	TCHAR *INPUT // @pyparm string/<o PyUnicode>|name||The folder name
);


// @pyswig string|HrMAPIFindFolder|Retrieves the entry ID for a folder in an information store using the hierarchical path name of the folder.
%native(HrMAPIFindFolder) PyHrMAPIFindFolder;
%{
PyObject *PyHrMAPIFindFolder(PyObject *self, PyObject *args)
{
	PyObject *obFolder;
	PyObject *obName;
	ULONG cbEID;
	LPENTRYID eid;
	HRESULT hr;
	PyObject *rc = NULL;
	if (!PyArg_ParseTuple(args, "OO:HrMAPIFindFolder",
		&obFolder, // @pyparm <o PyIMAPIFolder>|folder||The folder to search
		&obName))// @pyparm string|name||Name of the folder
		return NULL;
	TCHAR *szName = NULL;
	IMAPIFolder *pFolder = NULL;
	if (!PyWinObject_AsTCHAR(obName, &szName))
		goto done;
	if (!PyCom_InterfaceFromPyObject(obFolder, IID_IMAPIFolder, (void **)&pFolder, FALSE))
		goto done;
	{
	PY_INTERFACE_PRECALL;
	hr = HrMAPIFindFolder(pFolder, szName, &cbEID, &eid);
	PY_INTERFACE_POSTCALL;
	}
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto done;
	}
	rc = PyBytes_FromStringAndSize((char *)eid, cbEID);
done:
	if (pFolder) pFolder->Release();
	if (szName) PyWinObject_FreeTCHAR(szName);
	return rc;
}
%}

// @pyswig string|HrMAPIFindFolderEx|Retrieves the entry ID of a folder in an information store using the hierarchical path name of the folder.
%native(HrMAPIFindFolderEx) PyHrMAPIFindFolderEx;
%{
PyObject *PyHrMAPIFindFolderEx(PyObject *self, PyObject *args)
{
	PyObject *obMDB;
	PyObject *obSep, *obPath;
	ULONG cbEID;
	LPENTRYID eid;
	HRESULT hr;
	PyObject *rc = NULL;
	if (!PyArg_ParseTuple(args, "OOO:HrMAPIFindFolderEx",
		&obMDB, // @pyparm <o PyIMsgStore>|msgStore||The folder to search
		&obSep, // @pyparm string|sepString||The character separating the folder names - eg '\'
		&obPath))// @pyparm string|path||Path to the folder
		return NULL;
	TCHAR *szSep = NULL, *szPath = NULL;
	IMsgStore *pMDB = NULL;
	if (!PyWinObject_AsTCHAR(obSep, &szSep))
		goto done;
	if (!PyWinObject_AsTCHAR(obPath, &szPath))
		goto done;
	if (!PyCom_InterfaceFromPyObject(obMDB, IID_IMsgStore, (void **)&pMDB, FALSE))
		goto done;
	{
	PY_INTERFACE_PRECALL;
	hr = HrMAPIFindFolderEx(pMDB, szSep[0], szPath, &cbEID, &eid);
	PY_INTERFACE_POSTCALL;
	}
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto done;
	}
	rc = PyBytes_FromStringAndSize((char *)eid, cbEID);
done:
	if (pMDB) pMDB->Release();
	if (szSep) PyWinObject_FreeTCHAR(szSep);
	if (szPath) PyWinObject_FreeTCHAR(szPath);
	return rc;
}
%}

// @pyswig <o PyIMsgStore>|HrMAPIFindStore|Retrieves a pointer to the entry identifier of an information store from the display name of the store.
%native(HrMAPIFindStore) PyHrMAPIFindStore;
%{
PyObject *PyHrMAPIFindStore(PyObject *self, PyObject *args)
{
	PyObject *obSession;
	PyObject *obName;
	ULONG cbEID;
	LPENTRYID eid;
	HRESULT hr;
	PyObject *rc = NULL;
	if (!PyArg_ParseTuple(args, "OO:HrMAPIFindStore",
		&obSession, // @pyparm <o PyIMAPISession>|session||
		&obName)) // @pyparm string|name||
		return NULL;
	TCHAR *szName = NULL;
	IMAPISession *pSession = NULL;
	if (!PyWinObject_AsTCHAR(obName, &szName))
		goto done;
	if (!PyCom_InterfaceFromPyObject(obSession, IID_IMAPISession, (void **)&pSession, FALSE))
		goto done;
	{
	PY_INTERFACE_PRECALL;
	hr = HrMAPIFindStore(pSession, szName, &cbEID, &eid);
	PY_INTERFACE_POSTCALL;
	}
	if (FAILED(hr)) {
		OleSetOleError(hr);
		goto done;
	}
	rc = PyBytes_FromStringAndSize((char *)eid, cbEID);
done:
	if (pSession) pSession->Release();
	if (szName) PyWinObject_FreeTCHAR(szName);
	return rc;
}
%}

%native (HrCreateProfileName) PyHrCreateProfileName;
%{
// @pyswig string|HrCreateProfileName|Creates a profile with the specified name
static PyObject *PyHrCreateProfileName(PyObject *self, PyObject *args)
{
    HRESULT  _result;
	PyObject *obPrefix;
	char *prefix;
	// @pyparm string/<o PyUnicode>|profPrefix||A prefix for the new profile.
	if (!PyArg_ParseTuple(args, "O:HrCreateProfileName", &obPrefix))
		return NULL;
	if (!PyWinObject_AsChars(obPrefix, &prefix))
		return NULL;
	const int bufSize = MAX_PATH + 1;
	char buf[bufSize];
	_result = HrCreateProfileName(prefix, bufSize, buf);
	PyWinObject_FreeChars(prefix);
	if (FAILED(_result))
		return OleSetOleError(_result);
	return PyWinCoreString_FromString(buf);
}
%}

// @pyswig string|HrCreateDirEntryIdEx|Creates a directory identifier for a MAPI object, given the address of the object in the directory
%native(HrCreateDirEntryIdEx) PyHrCreateDirEntryIdEx;
%{
PyObject *PyHrCreateDirEntryIdEx(PyObject *self, PyObject *args)
{
	PyObject *ret = NULL;
	IAddrBook *pAddrBook;
	PyObject *obAddrBook, *obDN;
	char *szdn = NULL;
	LPENTRYID entryId;
	HRESULT hr;
	ULONG cbEntryId;
	if (!PyArg_ParseTuple(args, "OO:HrCreateDirEntryIdEx",
		&obAddrBook, // @pyparm <o PyIAddrBook>|addrBook||The address book interface
		&obDN))		 // @pyparm string|distinguishedName||The dn of the object to obtain the entry ID for.
		return NULL;

	if (!PyWinObject_AsChars(obDN, &szdn, FALSE))
        goto done;

	if (!PyCom_InterfaceFromPyInstanceOrObject(obAddrBook, IID_IAddrBook, (void **)&pAddrBook, /*BOOL bNoneOK=*/FALSE))
		goto done;

	hr = HrCreateDirEntryIdEx(pAddrBook, szdn, &cbEntryId, &entryId);
	if (FAILED(hr))
		return OleSetOleError(hr);

	ret = PyBytes_FromStringAndSize((char *)entryId, cbEntryId);
done:
	PyWinObject_FreeChars(szdn);
	if (pAddrBook) pAddrBook->Release();
	return ret;
}
%}


// @pyswig <o PyIMsgStore>|HrMailboxLogon|Logs on a server and mailbox.
%name(HrMailboxLogon) HRESULT MyHrMailboxLogon(
	IMAPISession *INPUT, // @pyparm <o PyIMAPISession>|session||The session object
	IMsgStore *INPUT, // @pyparm <o PyIMsgStore>|msgStore||
	char *INPUT, // @pyparm string/<o PyUnicode>|msgStoreDN||
	char *INPUT, // @pyparm string/<o PyUnicode>|mailboxDN||
	IMsgStore **OUTPUT
);

// @pyswig |HrMailboxLogoff|Logs off a server and mailbox.
%name(HrMailboxLogoff) HRESULT MyHrMailboxLogoff(
	IMsgStore **INPUT // @pyparm <o PyIMsgStore>|inbox||The open inbox.
);


// @pyswig <o PyIMAPIFolder>|HrMAPIOpenFolderEx|Opens a folder in the information store from the hierarchical path name of the folder.
%name(HrMAPIOpenFolderEx) HRESULT HrMAPIOpenFolderExW(
	IMsgStore *INPUT_NULLOK, // @pyparm <o PyIMsgStore>|msgStore||
	WCHAR INPUT, // @pyparm string/<o PyUnicode>|sep||The folder separator character.
	WCHAR *INPUT, // @pyparm string/<o PyUnicode>|name||The folder name
	IMAPIFolder **OUTPUT
);

// @pyswig |HrMAPISetPropBoolean|Sets a boolean property.
HRESULT HrMAPISetPropBoolean(
	IMAPIProp *INPUT, // @pyparm <o PyIMAPIProp>|obj||The object to set
	unsigned long ulPropTag, // @pyparm int|tag||The property tag
	int INPUT // int|val||The boolean property value.
);

// @pyswig |HrMAPISetPropLong|Sets a long property.
HRESULT HrMAPISetPropLong(
	IMAPIProp *INPUT, // @pyparm <o PyIMAPIProp>|obj||The object to set
	unsigned long ulPropTag, // @pyparm int|tag||The property tag
	long INPUT // int|val||The property value.
);

// @pyswig <o PyIMsgStore>|HrOpenExchangePublicStore|Retrieves an interface to the public information store provider.
HRESULT HrOpenExchangePublicStore(
	IMAPISession *INPUT,  // @pyparm <o PyIMAPISession>|session||The MAPI session object
	IMsgStore **OUTPUT
);

// @pyswig <o PyIMsgStore>|HrOpenExchangePrivateStore|Locates the primary user information store provider.
HRESULT HrOpenExchangePrivateStore(
	IMAPISession *INPUT,  // @pyparm <o PyIMAPISession>|session||The MAPI session object
	IMsgStore **OUTPUT
);

// @pyswig <o PyIMAPIFolder>|HrOpenExchangePublicFolders|Opens the root of the public folder hierarchy in the public information store.
HRESULT HrOpenExchangePublicFolders(
	IMsgStore *INPUT,  // @pyparm <o PyIMsgStore>|store||
	IMAPIFolder **OUTPUT
);

// @pyswig <o PyIMAPIProp>|HrOpenSessionObject|Retrieves a MAPI <o PyIMAPIProp> object for the current session object.
HRESULT HrOpenSessionObject(
	IMAPISession *INPUT,  // @pyparm <o PyIMAPISession>|session||The MAPI session object
	IMAPIProp **OUTPUT );

// @pyswig <o PyIMAPIProp>|HrOpenSiteContainer|Retrieves a MAPI <o PyIMAPIProp> object for a site object.
HRESULT HrOpenSiteContainer(
	IMAPISession *INPUT,  // @pyparm <o PyIMAPISession>|session||The MAPI session object
	IMAPIProp **OUTPUT );

// @pyswig <o PyIMAPIProp>|HrOpenSiteContainerAddressing|Retrieves a MAPI <o PyIMAPIProp> object for a site-addressing object.
HRESULT HrOpenSiteContainerAddressing(
	IMAPISession *INPUT, // @pyparm <o PyIMAPISession>|session||The MAPI session object
	IMAPIProp **OUTPUT
);
#endif /* SWIG_PY32BIT */
