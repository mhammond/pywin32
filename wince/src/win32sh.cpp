#include <windows.h>
#include <tchar.h>
#include "Python.h"
#include "extapi.h"
#include "shellapi.h"
#include "shlobj.h"



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
PySHCreateShortcut(PyObject *self, PyObject *args)
{	TCHAR *lpShortcut=NULL;
	TCHAR *lpTarget=NULL;
	BOOL  bResult;

	if(!PyArg_ParseTuple(args, "tt:SHCreateShortcut", &lpShortcut, &lpTarget))
		return NULL;
	bResult=SHCreateShortcut(lpShortcut, lpTarget);
	LocalFree(lpShortcut);
	LocalFree(lpTarget);
	if (!bResult)
	{	return SetError("SHCreateShortcut", GetLastError());
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PySHGetShortcutTarget(PyObject *self, PyObject *args)
{	PyObject *obResult=NULL;
	TCHAR *lpShortcut=NULL;
	TCHAR *lpTarget=NULL;

	if(!PyArg_ParseTuple(args, "t:SHGetShortcutTarget", &lpShortcut))
		return NULL;
	lpTarget=(TCHAR *)LocalAlloc(LPTR, 512 * sizeof(TCHAR));
	if (!SHGetShortcutTarget(lpShortcut, lpTarget, 512))
	{	LocalFree(lpShortcut);
		LocalFree(lpTarget);
		return SetError("SHGetShortcutTarget", GetLastError());
	}
	obResult=Py_BuildValue("t", lpTarget);
	LocalFree(lpShortcut);
	LocalFree(lpTarget);
	return obResult;
}

static PyObject *
PySHLoadDIBitmap(PyObject *self, PyObject *args)
{	TCHAR *lpFilename=NULL;
	HANDLE hDIB=0;

	if(!PyArg_ParseTuple(args, "t:SHLoadDIBitmap", &lpFilename))
		return NULL;
	hDIB=SHLoadDIBitmap(lpFilename);
	LocalFree(lpFilename);
	if (hDIB==NULL)
	{	return SetError("SHLoadDIBitmap", GetLastError());
	}
	return Py_BuildValue("i", hDIB);
}

static PyObject *
PySHShowOutOfMemory(PyObject *self, PyObject *args)
{	HWND hWnd=NULL;
	UINT uFlags=0;

	if(!PyArg_ParseTuple(args, "|ii:SHShowOutOfMemory", &hWnd, &uFlags))
		return NULL;
	SHShowOutOfMemory(hWnd, uFlags);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PySHAddToRecentDocs(PyObject *self, PyObject *args)
{	TCHAR *lpFilename=NULL;

	if(!PyArg_ParseTuple(args, "n:SHAddToRecentDocs", &lpFilename))
		return NULL;
	SHAddToRecentDocs(SHARD_PATH, (LPCVOID)lpFilename);
	LocalFree(lpFilename);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PySHGetFileInfo(PyObject *self, PyObject *args)
{	PyObject *obFileinfo=NULL;
	TCHAR *lpFilename=NULL;
	SHFILEINFO *lpInfo=NULL;
	DWORD dwAttr=0;
	DWORD dwRet=0;
	UINT uFlags=0;

	if(!PyArg_ParseTuple(args, "tii:SHGetFileInfo", &lpFilename, &dwAttr, &uFlags))
		return NULL;
	lpInfo=(SHFILEINFO *)LocalAlloc(LPTR, sizeof(SHFILEINFO));
	dwRet=SHGetFileInfo(lpFilename, dwAttr, lpInfo, (UINT)sizeof(lpInfo), uFlags);
	LocalFree(lpFilename);
	obFileinfo=Py_BuildValue("iiitti", lpInfo->hIcon, lpInfo->iIcon, lpInfo->dwAttributes, lpInfo->szDisplayName, lpInfo->szTypeName, dwRet);
	if (lpInfo->hIcon)
		DestroyIcon(lpInfo->hIcon);
	LocalFree(lpInfo);
	return obFileinfo;
}

static PyObject *
PyShellExecuteEx(PyObject *self, PyObject *args)
{	PyObject *obInfo=NULL;
	SHELLEXECUTEINFO *lpInfo=NULL;
	BOOL bResult;

	lpInfo=(SHELLEXECUTEINFO *)LocalAlloc(LPTR, sizeof(SHELLEXECUTEINFO));
	if (lpInfo==NULL)
	{	LocalFree(lpInfo);
		PyErr_NoMemory();
		return NULL;
	}
	lpInfo->cbSize=sizeof(SHELLEXECUTEINFO);
	if(!PyArg_ParseTuple(args, "iitttti", &(lpInfo->fMask), &(lpInfo->hwnd), 
						&(lpInfo->lpVerb), &(lpInfo->lpFile), &(lpInfo->lpParameters), 
						&(lpInfo->lpDirectory), &(lpInfo->nShow) ))
	{	LocalFree(lpInfo);
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	bResult=ShellExecuteEx(lpInfo);
	Py_END_ALLOW_THREADS

	obInfo=Py_BuildValue("iii", lpInfo->hInstApp, lpInfo->hIcon, lpInfo->hProcess);
	LocalFree((VOID *)lpInfo->lpVerb);
	LocalFree((VOID *)lpInfo->lpFile);
	LocalFree((VOID *)lpInfo->lpParameters);
	LocalFree((VOID *)lpInfo->lpDirectory);
	LocalFree(lpInfo);
	if (!bResult)
	{	Py_DECREF(obInfo);
		return SetError("SHLoadDIBitmap", GetLastError());
	}
	return obInfo;
}



static PyMethodDef win32sh_methods[]=
{	{"SHCreateShortcut",	PySHCreateShortcut, 1},
	{"SHGetShortcutTarget",	PySHGetShortcutTarget, 1},
	{"SHLoadDIBitmap",		PySHLoadDIBitmap, 1},
	{"SHShowOutOfMemory",	PySHShowOutOfMemory, 1},
	{"SHAddToRecentDocs",	PySHAddToRecentDocs, 1},
	{"SHGetFileInfo",		PySHGetFileInfo, 1},
	{"ShellExecuteEx",		PyShellExecuteEx, 1},
	{NULL, NULL}
};



#define CONST_LONG(n) PyDict_SetItemString(d, #n, PyInt_FromLong((LONG)n))

extern "C" __declspec(dllexport) void initwin32sh(void)
{	PyObject *m=NULL;
	PyObject *d=NULL;

	m=Py_InitModule4("win32sh", win32sh_methods, "", (PyObject*)NULL, PYTHON_API_VERSION);
	d=PyModule_GetDict(m);
	obError=PyString_FromString("error");
	PyDict_SetItemString(d, "error", obError);

	// From shellapi.h
	CONST_LONG(SE_ERR_FNF);
	CONST_LONG(SE_ERR_PNF);
	CONST_LONG(SE_ERR_ACCESSDENIED);
	CONST_LONG(SE_ERR_OOM);
	CONST_LONG(SE_ERR_DLLNOTFOUND);
	CONST_LONG(SE_ERR_SHARE);
	CONST_LONG(SE_ERR_ASSOCINCOMPLETE);
	CONST_LONG(SE_ERR_DDETIMEOUT);
	CONST_LONG(SE_ERR_DDEFAIL);
	CONST_LONG(SE_ERR_DDEBUSY);
	CONST_LONG(SE_ERR_NOASSOC);
	CONST_LONG(NIM_ADD);
	CONST_LONG(NIM_MODIFY);
	CONST_LONG(NIM_DELETE);
	CONST_LONG(NIF_MESSAGE);
	CONST_LONG(NIF_ICON);
	CONST_LONG(NIF_TIP);
	CONST_LONG(SHGFI_ICON);
	CONST_LONG(SHGFI_DISPLAYNAME);
	CONST_LONG(SHGFI_TYPENAME);
	CONST_LONG(SHGFI_ATTRIBUTES);
	CONST_LONG(SHGFI_ICONLOCATION);
	CONST_LONG(SHGFI_EXETYPE);
	CONST_LONG(SHGFI_SYSICONINDEX);
	CONST_LONG(SHGFI_LINKOVERLAY);
	CONST_LONG(SHGFI_SELECTED);
	CONST_LONG(SHGFI_LARGEICON);
	CONST_LONG(SHGFI_SMALLICON);
	CONST_LONG(SHGFI_OPENICON);
	CONST_LONG(SHGFI_SHELLICONSIZE);
	CONST_LONG(SHGFI_PIDL);
	CONST_LONG(SHGFI_USEFILEATTRIBUTES);
	CONST_LONG(SHARD_PIDL);
	CONST_LONG(SHARD_PATH);
	CONST_LONG(SHORTCUT_OVERWRITE);

	// From shlobj.h
	CONST_LONG(CSIDL_DESKTOP);
	CONST_LONG(CSIDL_PROGRAMS);
	CONST_LONG(CSIDL_CONTROLS);
	CONST_LONG(CSIDL_PRINTERS);
	CONST_LONG(CSIDL_PERSONAL);
	CONST_LONG(CSIDL_STARTUP);
	CONST_LONG(CSIDL_RECENT);
	CONST_LONG(CSIDL_SENDTO);
	CONST_LONG(CSIDL_BITBUCKET);
	CONST_LONG(CSIDL_STARTMENU);
	CONST_LONG(CSIDL_DESKTOPDIRECTORY);
	CONST_LONG(CSIDL_DRIVES);
	CONST_LONG(CSIDL_NETWORK);
	CONST_LONG(CSIDL_NETHOOD);
	CONST_LONG(CSIDL_FONTS);
	CONST_LONG(CSIDL_TEMPLATES);
//	CONST_LONG(SFGAO_CANCOPY);
//	CONST_LONG(SFGAO_CANMOVE);
//	CONST_LONG(SFGAO_CANLINK);
	CONST_LONG(SFGAO_CANRENAME);
	CONST_LONG(SFGAO_CANDELETE);
	CONST_LONG(SFGAO_HASPROPSHEET);
	CONST_LONG(SFGAO_DROPTARGET);
	CONST_LONG(SFGAO_CAPABILITYMASK);
	CONST_LONG(SFGAO_LINK);
	CONST_LONG(SFGAO_SHARE);
	CONST_LONG(SFGAO_READONLY);
	CONST_LONG(SFGAO_GHOSTED);
	CONST_LONG(SFGAO_DISPLAYATTRMASK);
	CONST_LONG(SFGAO_FILESYSANCESTOR);
	CONST_LONG(SFGAO_FOLDER);
	CONST_LONG(SFGAO_FILESYSTEM);
	CONST_LONG(SFGAO_HASSUBFOLDER);
	CONST_LONG(SFGAO_CONTENTSMASK);
	CONST_LONG(SFGAO_VALIDATE);
	CONST_LONG(SFGAO_REMOVABLE);
}
