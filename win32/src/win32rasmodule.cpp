/***********************************************************

win32ras.cpp -- module for interface into RAS

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "windows.h"
#include "ras.h"
#include "raserror.h"

#include "Python.h"
#include "pywintypes.h"

static PyObject *module_error;
static PyObject *obHandleMap = NULL;

/* error helper */

void SetError(char *msg, char *fnName = NULL, DWORD code = 0)
{
	PyObject *v = Py_BuildValue("(izs)", 0, fnName, msg);
	if (v != NULL) {
		PyErr_SetObject(module_error, v);
		Py_DECREF(v);
	}
}
PyObject *ReturnError(char *msg, char *fnName = NULL, DWORD code = 0)
{
	SetError(msg, fnName, code);
	return NULL;
}
PyObject *ReturnRasError(char *fnName, long err = 0)
{
	const int bufSize = 512;
	char buf[bufSize];
	DWORD errorCode = err == 0 ? GetLastError() : err;
	BOOL bHaveMessage = FALSE;
	if (errorCode) {
		bHaveMessage = RasGetErrorString(errorCode, buf, bufSize)==0;
		if (!bHaveMessage) {
			bHaveMessage = (0 != FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, errorCode, 0, buf, bufSize, NULL));
		}
	}
	if (!bHaveMessage)
		strcpy(buf,"No error message is available");
	/* strip trailing cr/lf */
	int end = strlen(buf)-1;
	if (end>1 && (buf[end-1]=='\n' || buf[end-1]=='\r'))
		buf[end-1] = '\0';
	else
		if (end>0 && (buf[end]=='\n' || buf[end]=='\r'))
			buf[end]='\0';
	PyObject *v = Py_BuildValue("(iss)", errorCode, fnName, buf);
	if (v != NULL) {
		PyErr_SetObject(module_error, v);
		Py_DECREF(v);
	}
	return NULL;
}

// @object RASDIALPARAMS|A tuple that describes a Win32 RASDIALPARAMS structure
// @comm When used as a paramater, RASDIALPARAMS must be a sequence, of up to
// 6 items long.  All items must be strings - None is not allowed.
// <nl>When this is returned from a RAS function, all six fields will exist.
// <nl>RAS will often accept an empty string to mean "default" - ie, passing
// an empty string to phoneNumber uses the stored phone number.
// @tupleitem 0|string|entryName|name of RAS entry.
// @tupleitem 1|string|phoneNumber|phone number to be used.
// @tupleitem 2|string|callBackNumber|phone number to be used if callback is enabled.
// @tupleitem 3|string|userName|username to log on with.
// @tupleitem 4|string|password|password to use
// @tupleitem 5|string|domain|Network domain to log on to.
// @ex An example with win32ras.Dial|handle = win32ras.Dial(None, None, ("Entry Name",), None)
BOOL PyObjectToRasDialParams( PyObject *ob, RASDIALPARAMS *p )
{
	char *fnName = "<RasDialParams conversion>";
	p->dwSize = sizeof(RASDIALPARAMS);
	p->szEntryName[0] = 0;
	p->szPhoneNumber[0] = 0;
	p->szCallbackNumber[0] = 0;
	p->szUserName[0] = 0;
	p->szPassword[0] = 0;
	p->szDomain[0] = 0;
	if (!PySequence_Check(ob)) {
		SetError("The RasDialParams item must be a sequence", fnName);
		return FALSE;
	}
	char *dest;
	int size = PyObject_Length(ob);
	for (int num=0;num<size;num++) {
		switch (num) {
		case 0: dest = p->szEntryName; break;
		case 1: dest = p->szPhoneNumber; break;
		case 2: dest = p->szCallbackNumber; break;
		case 3: dest = p->szUserName; break;
		case 4: dest = p->szPassword; break;
		case 5: dest = p->szDomain; break;
		default:
			SetError("The RasDialParams sequence length must be less than 6", fnName);
			return FALSE;
		}
		char *src = PyString_AsString(PySequence_GetItem(ob, num));
		if (src==NULL) {
			SetError("The RasDialParams sequence is invalid - must be a tuple of strings.", fnName);
			return FALSE;
		}
		strcpy(dest, src);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// the RAS callback function.  This looks up a Python handler,
// and defers the call to it.
//
// @method |win32ras|RasDialFunc1|A placeholder for a RAS callback.
// @comm Certain RAS function require a callback function to be passed.
// This description describes the signature of the function you pass
// to these functions.
VOID CALLBACK PyRasDialFunc1(
    HRASCONN  hrasconn,	// handle to RAS connection
    UINT  unMsg,	// type of event that has occurred
    RASCONNSTATE  rascs,	// connection state about to be entered
    DWORD  dwError,	// error that may have occurred
    DWORD  dwExtendedError)	// extended error information for some errors
{
	CEnterLeavePython _celp;
	char *fnName = "<RAS Callback handler>";
	PyObject *handler = NULL;
	if (obHandleMap) {
		// NOTE:  As we hold the thread lock, assume noone else can mod this dict.
		PyObject *key = PyInt_FromLong((long)hrasconn);
		if (key==NULL) return;
		handler = PyDict_GetItem( obHandleMap, key );
		// If handler is NULL, check if None is in the map, and if so,
		// use and replace it.
		if (handler==NULL) {
			handler = PyDict_GetItem( obHandleMap, Py_None );
			if (handler) {
				PyDict_SetItem(obHandleMap, key, handler);
				PyDict_DelItem(obHandleMap, Py_None);
			}
		}
		Py_DECREF(key);
	}
	if (handler==NULL) {
		SetError("Warning - RAS callback has no handler!", fnName);
		PyErr_Print();
		return;
	}
	// @pyparm int|hrascon||The handle to the RAS session.
	// @pyparm int|msg||A message code identifying the reason for the callback.
	// @pyparm int|rascs||Connection state about to be entered.
	// @pyparm int|error||The error state of the connection
	// @pyparm int|extendedError||
	PyObject *args = Py_BuildValue("iiiii",hrasconn, unMsg, rascs, dwError, dwExtendedError);
	if (args==NULL) return;
	PyObject *res = PyEval_CallObject(handler, args);
	Py_DECREF(args);
	if (res==NULL) {
		PyErr_Print();
		SetError("RAS callback failed!", fnName);
		PyErr_Print();
		return;
	}
	Py_DECREF(res);
}

// @pymethod |win32ras|CreatePhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into which the user can enter information about the entry
static PyObject *
PyRasCreatePhonebookEntry( PyObject *self, PyObject *args )
{
	int hwnd;
	DWORD rc;
	LPTSTR fileName = NULL;
	if (!PyArg_ParseTuple(args, "i|s:CreatePhoneBookEntry", 
	          &hwnd,  // @pyparm int|hWnd||Handle to the parent window of the dialog box.
	          &fileName )) // @pyparm string|fileName|None|Specifies the filename of the phonebook entry.  Currently this is ignored.
		return NULL;
	if (hwnd != 0 && !IsWindow((HWND)hwnd))
		return ReturnError("The first paramater must be a valid window handle", "<CreatePhonebookEntry param conversion>");
	if ((rc=RasCreatePhonebookEntry((HWND)hwnd, fileName )))
		return ReturnRasError("RasCreatePhonebookEntry",rc);	// @pyseeapi RasCreatePhonebookEntry
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int, int|win32ras|Dial|Establishes a RAS connection to a RAS server.
static PyObject *
PyRasDial( PyObject *self, PyObject *args )
{
	DWORD rc;
	char *ignored;
//	PyObject *obExtensions;
	PyObject *obParams;
	PyObject *obCallback;
	RASDIALPARAMS dialParams;
	LPTSTR fileName;
	HRASCONN hRas = (HRASCONN)0;

	if (!PyArg_ParseTuple(args, "zzOO:Dial", 
	          &ignored,  // @pyparm (tuple)|RasDialExtensions||Ignored - must be None - a placeholder for future RASDIALEXTENSIONS support.
	          &fileName, // @pyparm string|fileName||Specifies the filename of the phonebook entry, or None.  Ignored on Win95.
	          &obParams,  // @pyparm <o RASDIALPARAMS>|RasDialParams||A tuple describing a RASDIALPARAMS structure.
			  &obCallback))// @pyparm method or hwnd|callback||The method to be called when RAS events occur, or None.  If not None, the function must have the signature of <om win32ras.RasDialFunc1>
		return NULL;
	if (ignored) {
		PyErr_SetString(PyExc_TypeError, "You must pass None as the first param");
		return NULL;
	}
	if (!PyObjectToRasDialParams( obParams, &dialParams ))
		return NULL;

	DWORD notType = 0;
	LPVOID pNotification;
	if (obCallback==Py_None) {
		pNotification = NULL;
	} else if (PyCallable_Check(obCallback)) {
		pNotification = PyRasDialFunc1;
		notType = 1;
	} else if (PyInt_Check(obCallback)) {
		pNotification = (LPVOID)PyInt_AsLong(obCallback);
		notType = 0xFFFFFFFF;
	} else
		return ReturnError("The callback object must be an integer handle, None, or a callable object", "<Dial param parsing>");
	// If we have any sort of callback, we must ensure threads are init'd.
	if (pNotification)
		PyEval_InitThreads();
	// If we have a callback, store it in our map with None as the key.
	// The callback routine will patch this once it knows the true key.
	// Before we do, we must check None is not already there
	if (notType==1) {
		if (obHandleMap==NULL && (obHandleMap = PyDict_New())==NULL)
			return NULL;
		if (PyMapping_HasKey(obHandleMap, Py_None)) {
			PyErr_SetString(PyExc_RuntimeError, "Another RAS callback is in the process of starting");
			return NULL;
		}
		PyDict_SetItem( obHandleMap, Py_None, obCallback );
	}

	// @pyseeapi RasDial
	Py_BEGIN_ALLOW_THREADS
	rc=RasDial( NULL, fileName, &dialParams, notType, pNotification, &hRas );
	Py_END_ALLOW_THREADS
	if (hRas==0 && notType==1) {
		PyDict_DelItem(obHandleMap, Py_None);
		PyErr_Clear();
	}
	return Py_BuildValue( "ii", hRas, rc );
	// @rdesc The return value is (handle, retCode).
	// <nl>It is possible for a valid handle to be returned even on failure.
	// <nl>If the returned handle is = 0, then it can be assumed invalid.
	// @comm Note - this handle must be closed using <om win32ras.HangUp>, or
	// else the RAS port will remain open, even after the program has terminated.
	// Your operating system may need rebooting to clean up otherwise!
}

// @pymethod |win32ras|EditPhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into which the user can enter information about the entry
static PyObject *
PyRasEditPhonebookEntry( PyObject *self, PyObject *args )
{
	int hwnd;
	DWORD rc;
	LPTSTR fileName;
	LPTSTR entryName;
	if (!PyArg_ParseTuple(args, "izs:EditPhoneBookEntry", 
	          &hwnd,  // @pyparm int|hWnd||Handle to the parent window of the dialog box.
	          &fileName, // @pyparm string|fileName||Specifies the filename of the phonebook entry, or None.  Currently this is ignored.
	          &entryName )) // @pyparm string|entryName|None|Specifies the name of the phonebook entry to edit
		return NULL;
	if (hwnd != 0 && !IsWindow((HWND)hwnd))
		return ReturnError("The first paramater must be a valid window handle", "<EditPhonebookEntry param parsing>");
	Py_BEGIN_ALLOW_THREADS
	rc=RasEditPhonebookEntry((HWND)hwnd, fileName, entryName );
	Py_END_ALLOW_THREADS
	if (rc)
		return ReturnRasError("RasEditPhonebookEntry",rc);	// @pyseeapi RasEditPhonebookEntry
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod list|win32ras|EnumConnections|Returns a list of tuples, one for each active connection.
static PyObject *
PyRasEnumConnections( PyObject *self, PyObject *args )
{
	DWORD rc;
	DWORD bufSize;
	DWORD noConns = 0;
	RASCONN tc;
	if (!PyArg_ParseTuple(args, ":EnumConnections"))
		return NULL;
	RASCONN *pCon = NULL;
	// make dummy call to determine buffer size.
	tc.dwSize = bufSize = sizeof(RASCONN);
	Py_BEGIN_ALLOW_THREADS
	rc = RasEnumConnections(&tc, &bufSize, &noConns);
	Py_END_ALLOW_THREADS
	if (rc!=0 && rc!=ERROR_BUFFER_TOO_SMALL)
		return ReturnRasError("RasEnumConnections(NULL)", rc);
	if (rc==ERROR_BUFFER_TOO_SMALL) {
		if (bufSize==0)
			return ReturnRasError("RasEnumConnections buffer size is invalid");
		pCon = (RASCONN *)malloc(bufSize);
		if (pCon==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating buffer for RAS connections");
			return NULL;
		}
		// @pyseeapi RasEnumConnections
		pCon[0].dwSize = sizeof(RASCONN);
		Py_BEGIN_ALLOW_THREADS
		rc=RasEnumConnections(pCon, &bufSize, &noConns);
		Py_END_ALLOW_THREADS
		if (rc!=0)
		return ReturnRasError("RasEnumConnections", rc);
	} else {
		pCon = &tc;
	}
	PyObject *ret = PyList_New(0);
	if (ret==NULL)
		return NULL;

	for (DWORD i=0;i<noConns;i++)
		PyList_Append( ret, Py_BuildValue("(isss)", pCon[i].hrasconn, pCon[i].szEntryName, pCon[i].szDeviceType, pCon[i].szDeviceName) );

	// @rdesc Each tuple is of format (handle, entryName, deviceType, deviceName)
	if (pCon && pCon != &tc)
		free(pCon);
	return ret;
}

// @pymethod |win32ras|EnumEntries|Returns a list of tuples, one for each phonebook entry.
static PyObject *
PyRasEnumEntries( PyObject *self, PyObject *args )
{
	DWORD rc;
	DWORD bufSize;
	DWORD noConns = 0;
	char *reserved = NULL;
	char *bookName = NULL;
	RASENTRYNAME tc;
	if (!PyArg_ParseTuple(args, "|zz:EnumEntries",
		       &reserved, // @pyparm string|reserved|None|Reserved - must be None
			   &bookName)) // @pyparm string|fileName|None|The name of the phonebook file, or None.
		return NULL;

	// make dummy call to determine buffer size.
	tc.dwSize = bufSize = sizeof(RASENTRYNAME);
	Py_BEGIN_ALLOW_THREADS
	RasEnumEntries(reserved, bookName, &tc, &bufSize, &noConns);
	Py_END_ALLOW_THREADS
	RASENTRYNAME *pE = NULL;
	if (bufSize) {
		pE = (RASENTRYNAME *)malloc(bufSize);
		if (pE==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating buffer for RAS entries");
			return NULL;
		}
		// @pyseeapi RasEnumEntries
		pE[0].dwSize = sizeof(RASENTRYNAME);
		Py_BEGIN_ALLOW_THREADS
		rc=RasEnumEntries(reserved, bookName, pE, &bufSize, &noConns);
		Py_END_ALLOW_THREADS
		if (rc!=0)
			return ReturnRasError("RasEnumEntries", rc);
	}
	PyObject *ret = PyList_New(0);
	if (ret==NULL)
		return NULL;

	for (DWORD i=0;i<noConns;i++)
		PyList_Append( ret, Py_BuildValue("(s)", pE[i].szEntryName ) );
	if (pE)
		free(pE);
	return ret;
}

// @pymethod (int, int, string, string)|win32ras|GetConnectStatus|Returns a tuple with connection information.
static PyObject *
PyRasGetConnectStatus( PyObject *self, PyObject *args )
{
	HRASCONN hras;
	DWORD rc;
	if (!PyArg_ParseTuple(args, "i:GetConnectStatus", 
	          &hras ))  // @pyparm int|hrasconn||Handle to the RAS session.
		return NULL;
	RASCONNSTATUS cs;
	// @pyseeapi RasGetConnectStatus
	cs.dwSize = sizeof(RASCONNSTATUS);
	if ((rc=RasGetConnectStatus(hras, &cs )))
		return ReturnRasError("RasGetConnectStatus",rc);	// @pyseeapi RasGetConnectStatus
	return Py_BuildValue("(iiss)", cs.rasconnstate, cs.dwError, cs.szDeviceType, cs.szDeviceName);
}

// @pymethod (s,s,s,s,s,s),i|win32ras|GetEntryDialParams|Returns a tuple with the most recently set dial paramaters for the specified entry.
static PyObject *
PyRasGetEntryDialParams( PyObject *self, PyObject *args )
{
	char *fileName;
	char *entryName;
	DWORD rc;
	if (!PyArg_ParseTuple(args, "zs:GetEntryDialParams", 
	          &fileName, // @pyparm string|fileName||The filename of the phonebook, or None.
			  &entryName))  // @pyparm string|entryName||The name of the entry to retrieve the params for.
		return NULL;

	RASDIALPARAMS dp;
	BOOL bPass;
	dp.dwSize = sizeof(RASDIALPARAMS);
	strncpy(dp.szEntryName, entryName, RAS_MaxEntryName + 1);
	dp.szEntryName[RAS_MaxEntryName] = '\0';
	// @pyseeapi RasGetEntryDialParams
	if ((rc=RasGetEntryDialParams(fileName, &dp, &bPass )))
		return ReturnRasError("RasGetEntryDialParams",rc);	// @pyseeapi RasGetConnectStatus
	return Py_BuildValue("(ssssss),i", 
		dp.szEntryName, dp.szPhoneNumber,
		dp.szCallbackNumber, dp.szUserName, 
		dp.szPassword, dp.szDomain, bPass );
	// @rdesc The return value is a tuple describing the params retrieved, plus a BOOL integer
	// indicating if the password was also retrieved.
}

// @pymethod string|win32ras|GetErrorString|Returns an error string for a RAS error code.
static PyObject *
PyRasGetErrorString( PyObject *self, PyObject *args )
{
	DWORD error;
	DWORD rc;
	if (!PyArg_ParseTuple(args, "i:GetErrorString", 
	          &error)) // @pyparm int|error||The error value being queried.
		return NULL;

	char buf[512];
	// @pyseeapi RasGetErrorString
	if (rc=RasGetErrorString(error, buf, sizeof(buf)))
		return ReturnRasError("RasGetErrorString");
	return Py_BuildValue("s", buf);
}

// @pymethod |win32ras|HangUp|Terminates a remote access session.
static PyObject *
PyRasHangUp( PyObject *self, PyObject *args )
{
	DWORD rc;
	HRASCONN hras;
	if (!PyArg_ParseTuple(args, "i:HangUp", 
	          &hras)) // @pyparm int|hras||The handle to the RAS connection to be terminated.
		return NULL;

	// @pyseeapi RasHangUp
	if (rc=RasHangUp(hras))
		return ReturnRasError("RasHangup");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod |win32ras|IsHandleValid|Indicates if the given RAS handle is valid.
static PyObject *
PyRasIsHandleValid( PyObject *self, PyObject *args )
{
	HRASCONN hras;
	if (!PyArg_ParseTuple(args, "i:IsHandleValid", 
	          &hras)) // @pyparm int|hras||The handle to the RAS connection being checked.
		return NULL;
	BOOL bRet = (hras>=0);
	return Py_BuildValue("i", bRet);
}


// @pymethod |win32ras|SetEntryDialParams|Sets the dial paramaters for the specified entry.
static PyObject *
PyRasSetEntryDialParams( PyObject *self, PyObject *args )
{
	char *fileName;
	PyObject *obParams;
	RASDIALPARAMS dialParams;
	DWORD rc;
	BOOL bRemPass;
	if (!PyArg_ParseTuple(args, "zOi:SetEntryDialParams", 
	          &fileName, // @pyparm string|fileName||The filename of the phonebook, or None.
			  &obParams,// @pyparm (tuple)|RasDialParams||A tuple describing a RASDIALPARAMS structure.
			  &bRemPass)) // @pyparm int|bSavePassword||Indicates whether to remove password from entry's parameters.
		return NULL;

	if (!PyObjectToRasDialParams( obParams, &dialParams ))
		return NULL;
	// @pyseeapi SetEntryDialParams
	if ((rc=RasSetEntryDialParams(fileName, &dialParams, bRemPass)))
		return ReturnRasError("SetEntryDialParams",rc);	// @pyseeapi RasGetConnectStatus
	Py_INCREF(Py_None);
	return Py_None;
}




/* List of functions exported by this module */
// @module win32ras|A module encapsulating the Windows Remote Access Service (RAS) API.
static struct PyMethodDef win32ras_functions[] = {
	{"CreatePhonebookEntry",        PyRasCreatePhonebookEntry,  METH_VARARGS}, // @pymeth CreatePhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into which the user can enter information about the entry.
	{"Dial",                        PyRasDial,  METH_VARARGS}, // @pymeth Dial|Establishes a RAS connection to a RAS server.
	{"EditPhonebookEntry",          PyRasEditPhonebookEntry, METH_VARARGS}, // @pymeth EditPhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into which the user can enter information about the entry
	{"EnumConnections",             PyRasEnumConnections, METH_VARARGS}, // @pymeth EnumConnections|Returns a list of tuples, one for each active connection.
	{"EnumEntries",                 PyRasEnumEntries, METH_VARARGS}, // @pymeth EnumEntries|Returns a list of tuples, one for each phonebook entry.
	{"GetConnectStatus",            PyRasGetConnectStatus, METH_VARARGS}, // @pymeth GetConnectStatus|Returns a tuple with connection information.
	{"GetEntryDialParams",          PyRasGetEntryDialParams, METH_VARARGS}, // @pymeth GetEntryDialParams|Returns a tuple with the most recently set dial paramaters for the specified entry.
	{"GetErrorString",              PyRasGetErrorString, METH_VARARGS}, // @pymeth GetErrorString|Returns an error string for a RAS error code.
	{"HangUp",                      PyRasHangUp, METH_VARARGS}, // @pymeth HangUp|Terminates a remote access session.
	{"IsHandleValid",				PyRasIsHandleValid, METH_VARARGS}, // @pymeth IsHandleValid|Indicates if the given RAS handle is valid.
	{"SetEntryDialParams",          PyRasSetEntryDialParams, METH_VARARGS}, // @pymeth SetEntryDialParams|Sets the dial paramaters for the specified entry.
	{NULL,			NULL}
};


int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyInt_FromLong(value);
	if (!okey || !oval) {
		Py_XDECREF(okey);
		Py_XDECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	Py_XDECREF(okey);
	Py_XDECREF(oval);
	return rc;
}
#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, tok)) return rc
#define ADD_ENUM(parta, partb) if (rc=AddConstant(dict,#parta "_" #partb, parta::partb)) return rc
#define ADD_ENUM3(parta, partb, partc) if (rc=AddConstant(dict,#parta "_" #partb "_" #partc, parta::partb::partc)) return rc

static int AddConstants(PyObject *dict)
{
	int rc;
	ADD_CONSTANT(RASCS_OpenPort); // @const win32ras|RASCS_OpenPort|Constant for RAS state.
    ADD_CONSTANT(RASCS_PortOpened); // @const win32ras|RASCS_PortOpened|Constant for RAS state.
    ADD_CONSTANT(RASCS_ConnectDevice); // @const win32ras|RASCS_ConnectDevice|Constant for RAS state.
    ADD_CONSTANT(RASCS_DeviceConnected); // @const win32ras|RASCS_DeviceConnected|Constant for RAS state.
    ADD_CONSTANT(RASCS_AllDevicesConnected); // @const win32ras|RASCS_AllDevicesConnected|Constant for RAS state.
    ADD_CONSTANT(RASCS_Authenticate); // @const win32ras|RASCS_Authenticate|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthNotify); // @const win32ras|RASCS_AuthNotify|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthRetry); // @const win32ras|RASCS_AuthRetry|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthCallback); // @const win32ras|RASCS_AuthCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthChangePassword); // @const win32ras|RASCS_AuthChangePassword|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthProject); // @const win32ras|RASCS_AuthProject|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthLinkSpeed); // @const win32ras|RASCS_AuthLinkSpeed|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthAck); // @const win32ras|RASCS_AuthAck|Constant for RAS state.
    ADD_CONSTANT(RASCS_ReAuthenticate); // @const win32ras|RASCS_ReAuthenticate|Constant for RAS state.
    ADD_CONSTANT(RASCS_Authenticated); // @const win32ras|RASCS_Authenticated|Constant for RAS state.
    ADD_CONSTANT(RASCS_PrepareForCallback); // @const win32ras|RASCS_PrepareForCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_WaitForModemReset); // @const win32ras|RASCS_WaitForModemReset|Constant for RAS state.
    ADD_CONSTANT(RASCS_WaitForCallback); // @const win32ras|RASCS_WaitForCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_Projected); // @const win32ras|RASCS_Projected|Constant for RAS state.
    ADD_CONSTANT(RASCS_StartAuthentication); // @const win32ras|RASCS_StartAuthentication|Constant for RAS state.
    ADD_CONSTANT(RASCS_CallbackComplete); // @const win32ras|RASCS_CallbackComplete|Constant for RAS state.
    ADD_CONSTANT(RASCS_LogonNetwork); // @const win32ras|RASCS_LogonNetwork|Constant for RAS state.
    ADD_CONSTANT(RASCS_Interactive); // @const win32ras|RASCS_Interactive|Constant for RAS state.
    ADD_CONSTANT(RASCS_RetryAuthentication); // @const win32ras|RASCS_RetryAuthentication|Constant for RAS state.
    ADD_CONSTANT(RASCS_CallbackSetByCaller); // @const win32ras|RASCS_CallbackSetByCaller|Constant for RAS state.
    ADD_CONSTANT(RASCS_PasswordExpired); // @const win32ras|RASCS_PasswordExpired|Constant for RAS state.
	ADD_CONSTANT(RASCS_Connected); // @const win32ras|RASCS_Connected|Constant for RAS state.
    ADD_CONSTANT(RASCS_Disconnected); // @const win32ras|RASCS_Disconnected|Constant for RAS state.
	return 0;
}

extern "C" __declspec(dllexport) void
initwin32ras(void)
{
  PyWinGlobals_Ensure();
  PyObject *dict, *module;
  module = Py_InitModule("win32ras", win32ras_functions);
  dict = PyModule_GetDict(module);
  module_error = PyWinExc_ApiError;
  Py_INCREF(module_error);
//  module_error = PyString_FromString("win32ras error");
  PyDict_SetItemString(dict, "error", module_error);
  AddConstants(dict);
}
