#include <windows.h>
#include <tchar.h>
#include <ras.h>
#include <raserror.h>
#include "Python.h"
#include "extapi.h"




static PyObject *obHandleMap=NULL;
static PyObject *obError=NULL;


static PyObject *
SetError(CHAR *szMsg, DWORD dwCode=0)
{	PyObject *ob=Py_BuildValue("(si)", szMsg, dwCode);
	if (ob != NULL) {
		PyErr_SetObject(obError, ob);
		Py_DECREF(ob);
	}
	return NULL;
}



BOOL PyObjectToRasDialParams(PyObject *ob, RASDIALPARAMS *p)
{	TCHAR *lpDest=NULL;
	CHAR *lpSrc=NULL;
	DWORD dwLen=0;
	DWORD dwNum=0;

	if (!PySequence_Check(ob)) {
		SetError("RASDIALPARAMS must be a sequence of strings");
		return FALSE;
	}
	memset(p, 0, sizeof(RASDIALPARAMS));
	p->dwSize = sizeof(RASDIALPARAMS);
	dwLen=PyObject_Length(ob);
	for (dwNum=0; dwNum < dwLen; dwNum++) {
		switch (dwNum) {
		case 0: lpDest=p->szEntryName; break;
		case 1: lpDest=p->szPhoneNumber; break;
		case 2: lpDest=p->szCallbackNumber; break;
		case 3: lpDest=p->szUserName; break;
		case 4: lpDest=p->szPassword; break;
		case 5: lpDest=p->szDomain; break;
		default:
			SetError("RASDIALPARAMS must be a sequence of strings");
			return FALSE;
		}
		lpSrc=PyString_AsString(PySequence_GetItem(ob, dwNum));
		if (lpSrc==NULL) {
			SetError("RASDIALPARAMS must be a sequence of strings");
			return FALSE;
		}
		wsprintf(lpDest, TEXT("%hs"), lpSrc);
	}
	return TRUE;
}

void SetRasHandler(HRASCONN  hConn, PyObject *obVal)
{	PyObject *obKey;
	if (obHandleMap==NULL && (obHandleMap=PyDict_New())==NULL)
		return;
	Py_INCREF(obVal);
	obKey=PyInt_FromLong((long)hConn);
	PyDict_SetItem(obHandleMap, obKey, obVal);
}

VOID CALLBACK 
PyRasDialFunc1(HRASCONN hConn, UINT uMsg, RASCONNSTATE rcs, DWORD dwError, DWORD dwExtendedError)
{	PyObject *obHandler=NULL;
	PyObject *obResult;
	PyObject *obKey;
	PyObject *obArgs;

	if (obHandleMap) {
		obKey=PyInt_FromLong((long)hConn);
		if (obKey==NULL) return;
		obHandler=PyDict_GetItem(obHandleMap, obKey);
		Py_DECREF(obKey);
	}
	if (obHandler==NULL)
		return;
	obArgs=Py_BuildValue("iiiii", hConn, uMsg, rcs, dwError, dwExtendedError);
	if (obArgs==NULL) return;
	obResult=PyEval_CallObject(obHandler, obArgs);
	Py_DECREF(obArgs);
	if (obResult==NULL)
		return;
	Py_DECREF(obResult);
}

static PyObject *
PyRasDeleteEntry(PyObject *self, PyObject *args)
{	TCHAR *lpPhoneBook=NULL;
	TCHAR *lpEntry=NULL;
	DWORD dwResult=0;

	if (!PyArg_ParseTuple(args, "nt:RasDeleteEntry", &lpPhoneBook, &lpEntry))
		return NULL;
	dwResult=RasDeleteEntry(lpPhoneBook, lpEntry);
	LocalFree(lpPhoneBook);
	LocalFree(lpEntry);
	if (dwResult)
	{	return SetError("RasDeleteEntry", GetLastError());
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRasDial(PyObject *self, PyObject *args)
{	PyObject *obExtensions=NULL;
	PyObject *obPhoneBook=NULL;
	PyObject *obParams=NULL;
	PyObject *obCallback=NULL;
	LPVOID lpNotification=NULL;
	RASDIALPARAMS dp;
	HRASCONN hConn=0;
	DWORD dwResult=0;
	DWORD dwNotify=0;
	HWND hWnd=0;

	if (!PyArg_ParseTuple(args, "OOOO:RasDial", &obExtensions, &obPhoneBook, &obParams, &obCallback))
		return NULL;
	if (!PyObjectToRasDialParams(obParams, &dp))
		return NULL;

	// Lots of things are not supported on WinCE
	obExtensions= Py_None;
	obPhoneBook= Py_None;
	obCallback = Py_None;

	if (obCallback==Py_None) {
		dwNotify=0xFFFFFFFF;
//		hWnd=GetActiveWindow();
//		lpNotification=&hWnd;
//		dwNotify=0;
		lpNotification=NULL;
	} else if (PyCallable_Check(obCallback)) {
		lpNotification=PyRasDialFunc1;
		dwNotify=1;
	} else
		return SetError("Callback object must be None or callable");
	dwResult=RasDial(NULL, NULL, &dp, dwNotify, lpNotification, &hConn);
	if (dwResult)
	{	return SetError("Api Error", GetLastError());
	}
	if (dwNotify != 0xFFFFFFFF)
		SetRasHandler(hConn, obCallback);
	return Py_BuildValue("i", hConn);
}

static PyObject *
PyRasEnumConnections(PyObject *self, PyObject *args)
{	PyObject *obResult=NULL;
	RASCONN *lpRC=NULL;
	RASCONN rc;
	DWORD dwResult=0;
	DWORD dwBufSize=0;
	DWORD dwCount=0;
	DWORD n=0;

	if (!PyArg_ParseTuple(args, ":EnumConnections"))
		return NULL;
	rc.dwSize=sizeof(RASCONN);
	dwBufSize=sizeof(RASCONN);
	dwResult=RasEnumConnections(&rc, &dwBufSize, &dwCount);
	if (dwResult != 0 && dwResult != ERROR_BUFFER_TOO_SMALL)
		return SetError("RasEnumConnections", GetLastError());
	if (dwResult==ERROR_BUFFER_TOO_SMALL) {
		lpRC=(RASCONN *)LocalAlloc(LPTR, dwBufSize);
		if (lpRC==NULL) {
			return SetError("LocalAlloc", GetLastError());
		}
		lpRC[0].dwSize=sizeof(RASCONN);
		if ((dwResult=RasEnumConnections(lpRC, &dwBufSize, &dwCount)) != 0)
			return SetError("RasEnumConnections", GetLastError());
	} else {
		lpRC=&rc;
	}
	obResult=PyList_New(0);
	for (n=0; n < dwCount; n++) {
		PyList_Append(obResult, Py_BuildValue("(it)", lpRC[n].hrasconn, lpRC[n].szEntryName));
	}
	if (lpRC && lpRC != &rc)
		LocalFree(lpRC);
	return obResult;
}

static PyObject *
PyRasEnumEntries(PyObject *self, PyObject *args)
{	PyObject *obResult=NULL;
	TCHAR *lpReserved=NULL;
	TCHAR *lpPhoneBook=NULL;
	RASENTRYNAME *lpRN=NULL;
	RASENTRYNAME rn;
	DWORD dwResult=0;
	DWORD dwBufSize=0;
	DWORD dwCount=0;
	DWORD n=0;

	if (!PyArg_ParseTuple(args, "|nn:EnumEntries", &lpReserved, &lpPhoneBook))
		return NULL;

	rn.dwSize=sizeof(RASENTRYNAME);
	dwBufSize=sizeof(RASENTRYNAME);
	RasEnumEntries(lpReserved, lpPhoneBook, &rn, &dwBufSize, &dwCount);

	if (dwBufSize) {
		lpRN=(RASENTRYNAME *)LocalAlloc(LPTR, dwBufSize);
		if (lpRN==NULL) {
			return SetError("LocalAlloc", GetLastError());
		}
		lpRN[0].dwSize=sizeof(RASENTRYNAME);
		dwResult=RasEnumEntries(lpReserved, lpPhoneBook, lpRN, &dwBufSize, &dwCount);
	}
	LocalFree(lpReserved);
	LocalFree(lpPhoneBook);
	if (dwResult != 0)
		return SetError("RasEnumEntries", GetLastError());
	obResult=PyList_New(0);
	for (n=0; n < dwCount; n++) {
		PyList_Append(obResult, Py_BuildValue("t", lpRN[n].szEntryName));
	}
	if (lpRN)
		LocalFree(lpRN);
	return obResult;
}

static PyObject *
PyRasGetConnectStatus(PyObject *self, PyObject *args)
{	PyObject *obResult=NULL;
	RASCONNSTATUS cs;
	HRASCONN hConn;
	DWORD dwResult;

	if (!PyArg_ParseTuple(args, "i:RasGetConnectStatus", &hConn))
		return NULL;
	cs.dwSize=sizeof(RASCONNSTATUS);
	if ((dwResult=RasGetConnectStatus(hConn, &cs)))
		return SetError("RasGetConnectStatus", GetLastError());
	obResult=Py_BuildValue("(iinn)", cs.rasconnstate, cs.dwError, cs.szDeviceType, cs.szDeviceName);
	return obResult;
}

static PyObject *
PyRasGetEntryDialParams(PyObject *self, PyObject *args)
{	TCHAR *lpPhoneBook=NULL;
	TCHAR *lpEntryName=NULL;
	RASDIALPARAMS dp;
	DWORD dwResult=0;
	BOOL b;

	if (!PyArg_ParseTuple(args, "nt:GetEntryDialParams", &lpPhoneBook, &lpEntryName))
		return NULL;
	memset(&dp, 0, sizeof(RASDIALPARAMS));
	dp.dwSize=sizeof(RASDIALPARAMS);
	lstrcpy(dp.szEntryName, lpEntryName);
	dwResult=RasGetEntryDialParams(lpPhoneBook, &dp, &b);
	LocalFree(lpPhoneBook);
	LocalFree(lpEntryName);
	if (dwResult)
		return SetError("RasGetEntryDialParams", GetLastError());
	return Py_BuildValue("(nnnnnn)",  dp.szEntryName, dp.szPhoneNumber, dp.szCallbackNumber, dp.szUserName, dp.szPassword, dp.szDomain);

}

static PyObject *
PyRasHangup(PyObject *self, PyObject *args)
{	HRASCONN hConn=0;
	DWORD dwResult=0;

	if (!PyArg_ParseTuple(args, "i:RasHangUp", &hConn))
		return NULL;
	if (dwResult=RasHangUp(hConn))
		return SetError("RasHangUp", GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRasRenameEntry(PyObject *self, PyObject *args)
{	TCHAR *lpPhoneBook=NULL;
	TCHAR *lpOldEntry=NULL;
	TCHAR *lpNewEntry=NULL;
	DWORD dwResult=0;

	if (!PyArg_ParseTuple(args, "ntt:GetEntryDialParams", &lpPhoneBook, &lpOldEntry, &lpNewEntry))
		return NULL;
	dwResult=RasRenameEntry(lpPhoneBook, lpOldEntry, lpNewEntry);
	LocalFree(lpPhoneBook);
	LocalFree(lpOldEntry);
	LocalFree(lpNewEntry);
	if (dwResult)
		return SetError("RasRenameEntry", GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRasSetEntryDialParams(PyObject *self, PyObject *args)
{	PyObject *obParams=NULL;
	TCHAR *lpPhoneBook=NULL;
	RASDIALPARAMS dp;
	DWORD dwResult=0;
	BOOL bRmPw;

	if (!PyArg_ParseTuple(args, "nOi:SetEntryDialParams", &lpPhoneBook, &obParams, &bRmPw))
		return NULL;
	memset(&dp, 0, sizeof(RASDIALPARAMS));
	if (!PyObjectToRasDialParams(obParams, &dp))
		return NULL;
	dwResult=RasSetEntryDialParams(lpPhoneBook, &dp, bRmPw);
	LocalFree(lpPhoneBook);
	if (dwResult)
		return SetError("RasSetEntryDialParams", GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRasValidateEntryName(PyObject *self, PyObject *args)
{	TCHAR *lpPhoneBook=NULL;
	TCHAR *lpEntryName=NULL;
	DWORD dwResult=0;

	if (!PyArg_ParseTuple(args, "nt:RasValidateEntryName", &lpPhoneBook, &lpEntryName))
		return NULL;
	dwResult=RasValidateEntryName(lpPhoneBook, lpEntryName);
	LocalFree(lpPhoneBook);
	LocalFree(lpEntryName);
	return Py_BuildValue("i", dwResult);
}



//RasDeleteEntry(PhoneBook, EntryName)
//RasDial(Extensions, PhoneBook, (szEntry, szPhoneNumber, szCallbackNumber, szUserName, szPassword, szDomain), Callback)
//RasEnumConnections()
//RasEnumEntries(|Reserved, PhoneBook)
//RasGetConnectStatus(hConn)
//RasGetEntryDevConfig 
//RasGetEntryDialParams(PhoneBook, EntryName) 
//RasGetEntryProperties 
//RasHangup(hConn)
//RasRenameEntry(PhoneBook, OldEntry, NewEntry)
//RasSetEntryDevConfig 
//RasSetEntryDialParams(PhoneBook, (EntryName, szPhoneNumber, szCallbackNumber, szUserName, szPassword, szDomain), bSavePassword)
//RasSetEntryProperties 
//RasValidateEntryName(PhoneBook, EntryName)

static PyMethodDef win32ras_methods[]=
{	{"RasDeleteEntry", 			PyRasDeleteEntry, 1},
	{"RasDial", 				PyRasDial, 1},
	{"RasEnumConnections", 		PyRasEnumConnections, 1},
	{"RasEnumEntries", 			PyRasEnumEntries, 1},
	{"RasGetConnectStatus", 	PyRasGetConnectStatus, 1}, 
//	{"RasGetEntryDevConfig", 	PyRasGetEntryDevConfig, 1},
	{"RasGetEntryDialParams", 	PyRasGetEntryDialParams, 1},
//	{"RasGetEntryProperties", 	PyRasGetEntryProperties, 1},
	{"RasHangup", 				PyRasHangup, 1},
	{"RasRenameEntry", 			PyRasRenameEntry, 1},
//	{"RasSetEntryDevConfig", 	PyRasSetEntryDevConfig, 1},
	{"RasSetEntryDialParams", 	PyRasSetEntryDialParams, 1},
//	{"RasSetEntryProperties", 	PyRasSetEntryProperties, 1},
	{"RasValidateEntryName", 	PyRasValidateEntryName, 1},
	{NULL, NULL}
};



#define CONST_LONG(n) PyDict_SetItemString(d, #n, PyInt_FromLong((LONG)n))

extern "C" __declspec(dllexport) void initwin32ras(void)
{	PyObject *m=NULL;
	PyObject *d=NULL;

	m=Py_InitModule4("win32ras", win32ras_methods, "", (PyObject*)NULL, PYTHON_API_VERSION);
	d=PyModule_GetDict(m);
	obError=PyString_FromString("error");
	PyDict_SetItemString(d, "error", obError);

	CONST_LONG(RASCS_OpenPort);
	CONST_LONG(RASCS_PortOpened);
	CONST_LONG(RASCS_ConnectDevice);	
	CONST_LONG(RASCS_DeviceConnected);
	CONST_LONG(RASCS_AllDevicesConnected);
	CONST_LONG(RASCS_Authenticate);
	CONST_LONG(RASCS_AuthNotify);
	CONST_LONG(RASCS_AuthRetry);
	CONST_LONG(RASCS_AuthCallback);
	CONST_LONG(RASCS_AuthChangePassword);
	CONST_LONG(RASCS_AuthProject);
	CONST_LONG(RASCS_AuthLinkSpeed);
	CONST_LONG(RASCS_AuthAck);
	CONST_LONG(RASCS_ReAuthenticate);
	CONST_LONG(RASCS_Authenticated);
	CONST_LONG(RASCS_PrepareForCallback);
	CONST_LONG(RASCS_WaitForModemReset);
	CONST_LONG(RASCS_WaitForCallback);
	CONST_LONG(RASCS_Projected);

//  Not supported on CE
//	CONST_LONG(RASCS_StartAuthentication);
//	CONST_LONG(RASCS_CallbackComplete);
//	CONST_LONG(RASCS_LogonNetwork);

	CONST_LONG(RASCS_Interactive);
	CONST_LONG(RASCS_RetryAuthentication);
	CONST_LONG(RASCS_CallbackSetByCaller);
	CONST_LONG(RASCS_PasswordExpired);
	CONST_LONG(RASCS_Connected);
	CONST_LONG(RASCS_Disconnected);

}
