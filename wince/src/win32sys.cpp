#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Python.h"
#include "extapi.h"



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


static PyObject *
PyGetStoreInformation(PyObject *self, PyObject *args)
{	STORE_INFORMATION *lpInfo=NULL;
	PyObject *obInfo=NULL;
	BOOL bResult;

	if(!PyArg_ParseTuple(args, ":GetStoreInformation"))
		return NULL;
	lpInfo=(STORE_INFORMATION *)LocalAlloc(LPTR, sizeof(STORE_INFORMATION));
	if (!GetStoreInformation(lpInfo)) {
		LocalFree(lpInfo);
		return SetError("GetStoreInformation", GetLastError());
	}
	obInfo=Py_BuildValue("ii", lpInfo->dwStoreSize, lpInfo->dwFreeSize);
	LocalFree(lpInfo);
	return obInfo;
}


static PyObject *
PyGetSystemPowerStatusEx(PyObject *self, PyObject *args)
{	SYSTEM_POWER_STATUS_EX *lpInfo=NULL;
	PyObject *obInfo=NULL;
	BOOL bResult;

	if(!PyArg_ParseTuple(args, ":SystemPowerStatusEx"))
		return NULL;
	lpInfo=(SYSTEM_POWER_STATUS_EX *)LocalAlloc(LPTR, sizeof(SYSTEM_POWER_STATUS_EX));
	if (!GetSystemPowerStatusEx(lpInfo, TRUE)) {
		LocalFree(lpInfo);
		return SetError("GetSystemPowerStatusEx", GetLastError());
	}
	obInfo=Py_BuildValue("iiiiiiiii", lpInfo->ACLineStatus, lpInfo->BatteryFlag, lpInfo->BatteryLifePercent, lpInfo->BatteryLifeTime, lpInfo->BatteryFullLifeTime, lpInfo->BackupBatteryFlag, lpInfo->BackupBatteryLifePercent, lpInfo->BackupBatteryLifeTime, lpInfo->BackupBatteryLifeTime);
	LocalFree(lpInfo);
	return obInfo;
}


static PyObject *
PyGetVersionEx(PyObject *self, PyObject *args)
{	OSVERSIONINFO *lpInfo=NULL;
	PyObject *obInfo=NULL;
	BOOL bResult;

	if(!PyArg_ParseTuple(args, ":GetVersionEx"))
		return NULL;
	lpInfo=(OSVERSIONINFO *)LocalAlloc(LPTR, sizeof(OSVERSIONINFO));
	lpInfo->dwOSVersionInfoSize=(DWORD)sizeof(OSVERSIONINFO);
	if (!GetVersionEx(lpInfo)) {
		LocalFree(lpInfo);
		return SetError("GetVersionEx", GetLastError());
	}
	obInfo=Py_BuildValue("iiiin", lpInfo->dwMajorVersion, lpInfo->dwMinorVersion, lpInfo->dwBuildNumber, lpInfo->dwPlatformId, lpInfo->szCSDVersion);
	LocalFree(lpInfo);
	return obInfo;
}

static PyObject *
PyMessageBeep(PyObject *self, PyObject *args)
{	UINT uType=0xFFFFFFFF;

	if(!PyArg_ParseTuple(args, "|i:MessageBeep", &uType))
		return NULL;
	if(!MessageBeep(uType))
		return SetError("MessageBeep", GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyMessageBox(PyObject *self, PyObject *args)
{	TCHAR *lpTitle=NULL;
	TCHAR *lpText=NULL;
	UINT  uType=0;
	HWND  hWnd=0;
	int   n;

	if(!PyArg_ParseTuple(args, "itti:MessageBox", &hWnd, &lpText, &lpTitle, &uType))
		return NULL;
	n=MessageBox(hWnd, lpText, lpTitle, uType);
	LocalFree(lpText);
	LocalFree(lpTitle);
	return PyInt_FromLong(n);
}

static PyObject *
PySleep(PyObject *self, PyObject *args)
{	DWORD n=0;
	if(!PyArg_ParseTuple(args, "i:Sleep", &n))
		return NULL;
	Sleep(n);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyMAKELANGID(PyObject *self, PyObject *args)
{	int lang, sub;
	if(!PyArg_ParseTuple(args, "ii:MAKELANGID", &lang, &sub))
		return NULL;
	return PyInt_FromLong(MAKELANGID(lang, sub));
}

static PyObject *
PyHIWORD(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:HIWORD", &n))
		return NULL;
	return PyInt_FromLong(HIWORD(n));
}

static PyObject *
PyLOWORD(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:LOWORD", &n))
		return NULL;
	return PyInt_FromLong(LOWORD(n));
}

static PyObject *
PyHIBYTE(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:HIBYTE", &n))
		return NULL;
	return PyInt_FromLong(HIBYTE(n));
}

static PyObject *
PyLOBYTE(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:LOBYTE", &n))
		return NULL;
	return PyInt_FromLong(LOBYTE(n));
}

static PyObject *
PyRGB(PyObject *self, PyObject *args)
{	int r,g,b;
	if(!PyArg_ParseTuple(args, "iii:RGB", &r, &g, &b)) 
		return NULL;
	return PyInt_FromLong(RGB(r,g,b));
}



static PyMethodDef win32sys_methods[]=
{	{"GetStoreInformation",		PyGetStoreInformation, 1},
	{"GetSystemPowerStatusEx",	PyGetSystemPowerStatusEx, 1},
	{"GetVersionEx",			PyGetVersionEx, 1},
	{"MessageBeep",				PyMessageBeep, 1},
	{"MessageBox",				PyMessageBox, 1},
	{"Sleep",					PySleep, 1},
	{"HIBYTE",					PyHIBYTE, 1},
	{"LOBYTE",					PyLOBYTE, 1},
	{"HIWORD",					PyHIWORD, 1},
	{"LOWORD",					PyLOWORD, 1},
	{"RGB",						PyRGB, 1},
	{"MAKELANGID",				PyMAKELANGID, 1},
	{NULL, NULL}
};



extern "C" __declspec(dllexport) 
void initwin32sys(void)
{	PyObject *m=NULL;
	PyObject *d=NULL;

	InitCommonControls();

	m=Py_InitModule4("win32sys", win32sys_methods, "", (PyObject*)NULL, PYTHON_API_VERSION);
	d=PyModule_GetDict(m);
	obError=PyString_FromString("error");
	PyDict_SetItemString(d, "error", obError);
}
