// shell.cpp :
// $Id$

// Interfaces that support the Explorer Shell interfaces.

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "shell_pch.h"
#include "PyIShellLink.h"
#include "PythonCOMRegister.h" // For simpler registration of IIDs etc.

void PyShell_FreeMem(void *p)
{
	IMalloc *pMalloc;
	if (SHGetMalloc(&pMalloc)==S_OK) {
		pMalloc->Free(p);
		pMalloc->Release();
	}
}

void *PyShell_AllocMem(size_t cb)
{
	IMalloc *pMalloc;
	if (SHGetMalloc(&pMalloc)==S_OK) {
		void *rc = pMalloc->Alloc(cb);
		pMalloc->Release();
		return rc;
	}
	return NULL;
}

// Some magic hackery macros :-)
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)
UINT PyShell_ILGetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
		cbTotal += sizeof(pidl->mkid.cb);	// Null terminator
		while (pidl->mkid.cb)
		{
		    cbTotal += pidl->mkid.cb;
		    pidl = _ILNext(pidl);
		}
    }

    return cbTotal;
}

PyObject *PyObject_FromPIDL(LPITEMIDLIST pidl)
{
	return PyString_FromStringAndSize((char *)pidl, PyShell_ILGetSize(pidl) );
}
// @object PyIDL|A Python representation of an IDL.  Implemented as a Python string.
BOOL PyObject_AsPIDL(PyObject *ob, LPCITEMIDLIST *ppidl, BOOL bNoneOK = FALSE)
{
	if (ob==Py_None) {
		if (!bNoneOK) {
			PyErr_SetString(PyExc_TypeError, "None is not a valid ITEMIDLIST in this context");
			return FALSE;
		}
		*ppidl = NULL;
		return TRUE;
	}
	if (!PyString_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "Only strings and None are valid ITEMIDLIST objects.");
		return FALSE;
	}
	size_t cb = PyString_Size(ob);
	void *buf = PyShell_AllocMem( cb );
	if (buf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating memory for a PIDL");
		return FALSE;
	}
	memcpy( buf, PyString_AsString(ob), cb);
	*ppidl = (LPCITEMIDLIST)buf;
	return TRUE;
}
void PyObject_FreePIDL( LPCITEMIDLIST pidl )
{
	PyShell_FreeMem( (void *)pidl);
}

//////////////////////////////////////////////////
//
// WIN32_FIND_DATA implementation.
// NOTE: Cloned from win32api.cpp
PyObject *PyObject_FromWIN32_FIND_DATA(WIN32_FIND_DATAA &findData)
{
	PyObject *obCreateTime = PyWinObject_FromFILETIME(findData.ftCreationTime);
	PyObject *obAccessTime = PyWinObject_FromFILETIME(findData.ftLastAccessTime);
	PyObject *obWriteTime = PyWinObject_FromFILETIME(findData.ftLastWriteTime);
	if (obCreateTime==NULL || obAccessTime==NULL || obWriteTime==NULL)
		return NULL;

	PyObject *ret = Py_BuildValue("lOOOllllss",
		// @rdesc The return value is a list of tuples, in the same format as the WIN32_FIND_DATA structure:
			findData.dwFileAttributes, // @tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
			obCreateTime, // @tupleitem 1|<o PyTime>|createTime|File creation time.
    		obAccessTime, // @tupleitem 2|<o PyTime>|accessTime|File access time.
    		obWriteTime, // @tupleitem 3|<o PyTime>|writeTime|Time of last file write
    		findData.nFileSizeHigh, // @tupleitem 4|int|nFileSizeHigh|high order word of file size.
    		findData.nFileSizeLow,	// @tupleitem 5|int|nFileSizeLow|low order word of file size.
    		findData.dwReserved0,	// @tupleitem 6|int|reserved0|Reserved.
    		findData.dwReserved1,   // @tupleitem 7|int|reserved1|Reserved.
    		findData.cFileName,		// @tupleitem 8|string|fileName|The name of the file.
    		findData.cAlternateFileName ); // @tupleitem 9|string|alternateFilename|Alternative name of the file, expressed in 8.3 format.
	Py_DECREF(obCreateTime);
	Py_DECREF(obAccessTime);
	Py_DECREF(obWriteTime);
	return ret;
}

//////////////////////////////////////////////////////////////
//
// The methods
//

// @pymethod (<o PyIDL>, string displayName, iImage)|shell|SHBrowseForFolder|Displays a dialog box that enables the user to select a shell folder.
static PyObject *PySHBrowseForFolder( PyObject *self, PyObject *args)
{
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(BROWSEINFO));
	PyObject *rc = NULL;
	PyObject *obPIDL = Py_None;
	PyObject *obTitle = Py_None;
	PyObject *none_for_now = Py_None;
	TCHAR retPath[MAX_PATH];
	bi.pszDisplayName = retPath;
	LPITEMIDLIST pl = NULL;

	if(!PyArg_ParseTuple(args, "|lOOlOl:SHBrowseForFolder",
			&bi.hwndOwner, // @pyparm int|hwndOwner|0|
			&obPIDL,		// @pyparm <o PyIDL>|pidlRoot|None|
			&obTitle,		// @pyparm <o Unicode>/string|title|None|
			&bi.ulFlags,	// @pyparm int|flags|0|
			&none_for_now,  // @pyparm object|callback||Not yet supported - must be None
			&bi.lParam))   // @pyparm int|callbackParam|0|
		return NULL;
	if (none_for_now != Py_None) {
		PyErr_SetString(PyExc_TypeError, "Callback item must be None");
		goto done;
	}
	if (!PyObject_AsPIDL(obPIDL, &bi.pidlRoot, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obTitle, (TCHAR **)&bi.lpszTitle, TRUE))
		goto done;

	{ // new scope to avoid warnings about the goto
	PY_INTERFACE_PRECALL;
	pl = SHBrowseForFolder(&bi);
	PY_INTERFACE_POSTCALL;
	}

	// @rdesc The result is ALWAYS a tuple of 3 items.  If the user cancels the
	// dialog, all items are None.  If the dialog is closed normally, the result is
	// a tuple of (PIDL, DisplayName, iImageList)
	if (pl){
		PyObject *obPidl = PyObject_FromPIDL(pl);
		PyObject *obDisplayName = PyWinObject_FromTCHAR(bi.pszDisplayName);
		rc = Py_BuildValue("OOi", obPidl, obDisplayName, bi.iImage);
		PyShell_FreeMem(pl);
		Py_XDECREF(obPidl);
		Py_XDECREF(obDisplayName);
	}
	else {
		rc = Py_BuildValue("OOO", Py_None, Py_None, Py_None);
	}
done:
	if (bi.pidlRoot) PyObject_FreePIDL(bi.pidlRoot);
	if (bi.lpszTitle) PyWinObject_FreeTCHAR((TCHAR *)bi.lpszTitle);
	return rc;
}

// @pymethod string/<o PyUnicode>|shell|SHGetPathFromIDList|Converts an IDLIST to a path.
static PyObject *PySHGetPathFromIDList(PyObject *self, PyObject *args)
{
	char buffer[MAX_PATH];
	PyObject *rc;
	LPCITEMIDLIST pidl;
	PyObject *obPidl;

	if (!PyArg_ParseTuple(args, "O:SHGetPathFromIDList", &obPidl))
		// @pyparm <o PyIDL>|idl||The ITEMIDLIST
		return NULL;
	if (!PyObject_AsPIDL(obPidl, &pidl))
		return NULL;

	PY_INTERFACE_PRECALL;
	BOOL ok = SHGetPathFromIDList(pidl, buffer);
	PY_INTERFACE_POSTCALL;
	if (!ok) {
		PyWin_SetAPIError("SHGetPathFromIDList");
		rc = NULL;
	} else
		rc = PyWinObject_FromTCHAR(buffer);
	PyObject_FreePIDL(pidl);
	return rc;
}

// @pymethod string/<o PyUnicode>|shell|SHGetSpecialFolderPath|Retrieves the path of a special folder. 
static PyObject *PySHGetSpecialFolderPath(PyObject *self, PyObject *args)
{
	HWND hwndOwner;
	int nFolder;
	BOOL bCreate = FALSE;
	if(!PyArg_ParseTuple(args, "li|i:SHGetSpecialFolderPath",
			&hwndOwner, // @pyparm int|hwndOwner||
			&nFolder, // @pyparm int|nFolder||One of the CSIDL_* constants specifying the path.
			&bCreate)) // @pyparm int|bCreate|0|Should the path be created.
		return NULL;

	typedef HRESULT (WINAPI * PFNSHGetSpecialFolderPath)(HWND, LPWSTR,  int, BOOL );

	// @comm This method is only available in shell version 4.71.  If the function is not available, a COM Exception with HRESULT=E_NOTIMPL will be raised.
	HMODULE hmod = GetModuleHandle("shell32.dll");
	PFNSHGetSpecialFolderPath pfnSHGetSpecialFolderPath = (PFNSHGetSpecialFolderPath)GetProcAddress(hmod, "SHGetSpecialFolderPathW");
	if (pfnSHGetSpecialFolderPath==NULL)
		return OleSetOleError(E_NOTIMPL);

	WCHAR buf[MAX_PATH+1];
	PY_INTERFACE_PRECALL;
	HRESULT hr = (*pfnSHGetSpecialFolderPath)(hwndOwner, buf, nFolder, bCreate);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return OleSetOleError(hr);
	return PyWinObject_FromWCHAR(buf);
}

// @pymethod <o PyIDL>|shell|SHGetSpecialFolderLocation|Retrieves the PIDL of a special folder.
static PyObject *PySHGetSpecialFolderLocation(PyObject *self, PyObject *args)
{
	HWND hwndOwner;
	int nFolder;
	if(!PyArg_ParseTuple(args, "li|i:SHGetSpecialFolderLocation",
			&hwndOwner, // @pyparm int|hwndOwner||
			&nFolder)) // @pyparm int|nFolder||One of the CSIDL_* constants specifying the path.
		return NULL;

	LPITEMIDLIST pidl;
	PY_INTERFACE_PRECALL;
	HRESULT hr = SHGetSpecialFolderLocation(hwndOwner, nFolder, &pidl);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return OleSetOleError(hr);
	PyObject *rc = PyObject_FromPIDL(pidl);
	PyShell_FreeMem(pidl);
	return rc;
}

// @pymethod string/<o PyUnicode>|shell|SHGetFolderPath|Retrieves the path of a folder. 
static PyObject *PySHGetFolderPath(PyObject *self, PyObject *args)
{
	HWND hwndOwner;
	int nFolder;
	long flags;
	PyObject *obHandle;
	BOOL bCreate = FALSE;
	if(!PyArg_ParseTuple(args, "liOl:SHGetFolderPath",
			&hwndOwner, // @pyparm int|hwndOwner||
			&nFolder, // @pyparm int|nFolder||One of the CSIDL_* constants specifying the path.
			&obHandle, // @pyparm <o PyHANDLE>|handle||An access token that can be used to represent a particular user, or None
			&flags)) // @pyparm int|flags||Controls which path is returned.  May be SHGFP_TYPE_CURRENT or SHGFP_TYPE_DEFAULT
		return NULL;

	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, TRUE))
		return NULL;

	typedef HRESULT (WINAPI * PFNSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPWSTR);

	// @comm This method is only available if you have shfolder.dll installed, included with certain shell updates.
	HMODULE hmod = LoadLibrary("shfolder.dll");
	PFNSHGetFolderPath pfnSHGetFolderPath = NULL;
	if (hmod) pfnSHGetFolderPath=(PFNSHGetFolderPath)GetProcAddress(hmod, "SHGetFolderPathW");
	if (pfnSHGetFolderPath==NULL) {
		if (hmod) FreeLibrary(hmod);
		return OleSetOleError(E_NOTIMPL);
	}

	WCHAR buf[MAX_PATH+1];
	PY_INTERFACE_PRECALL;
	HRESULT hr = (*pfnSHGetFolderPath)(hwndOwner, nFolder, handle, flags, buf);
	PY_INTERFACE_POSTCALL;

	FreeLibrary(hmod);
	if (FAILED(hr))
		return OleSetOleError(hr);
	return PyWinObject_FromWCHAR(buf);
}

// @pymethod <o PyIDL>|shell|SHGetFolderLocation|Retrieves the PIDL of a folder.
static PyObject *PySHGetFolderLocation(PyObject *self, PyObject *args)
{
	HWND hwndOwner;
	int nFolder;
	long flags = 0;
	PyObject *obHandle;
	BOOL bCreate = FALSE;
	if(!PyArg_ParseTuple(args, "liO|l:SHGetFolderLocation",
			&hwndOwner, // @pyparm int|hwndOwner||
			&nFolder, // @pyparm int|nFolder||One of the CSIDL_* constants specifying the path.
			&obHandle, // @pyparm <o PyHANDLE>|handle||An access token that can be used to represent a particular user, or None
			&flags)) // @pyparm int|reserved||Must be 0
		return NULL;

	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, TRUE))
		return NULL;

	LPITEMIDLIST pidl;

	typedef HRESULT (WINAPI * PFNSHGetFolderLocation)(HWND, int, LPITEMIDLIST *);

	// @comm This method is only available if you have a late version of shfolder.dll installed, included with certain shell updates.
	HMODULE hmod = LoadLibrary("shfolder.dll");
	PFNSHGetFolderLocation pfnSHGetFolderLocation = NULL;
	if (hmod) pfnSHGetFolderLocation=(PFNSHGetFolderLocation)GetProcAddress(hmod, "SHGetFolderLocationW");
	if (pfnSHGetFolderLocation==NULL) {
		if (hmod) FreeLibrary(hmod);
		return OleSetOleError(E_NOTIMPL);
	}
	PY_INTERFACE_PRECALL;
	HRESULT hr = (*pfnSHGetFolderLocation)(hwndOwner, nFolder, &pidl);
	PY_INTERFACE_POSTCALL;
	FreeLibrary(hmod);
	if (FAILED(hr))
		return OleSetOleError(hr);
	PyObject *rc = PyObject_FromPIDL(pidl);
	PyShell_FreeMem(pidl);
	return rc;
}

// @pymethod |shell|SHAddToRecentDocs|Adds a document to the shell's list of recently used documents or clears all documents from the list. The user gains access to the list through the Start menu of the Windows taskbar.
static PyObject *PySHAddToRecentDocs(PyObject *self, PyObject *args)
{
	int flags;
	void *whatever;
	if(!PyArg_ParseTuple(args, "iz:SHAddToRecentDocs",
			&flags, // @pyparm int|flags||Flag that indicates the meaning of the whatever parameter
			&whatever)) // @pyparm string|whatever||A path or <o PyIDL>
		return NULL;

	PY_INTERFACE_PRECALL;
	SHAddToRecentDocs(flags, whatever);
	PY_INTERFACE_POSTCALL;
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod <o PyIDL>|shell|SHChangeNotify|Notifies the system of an event that an application has performed. An application should use this function if it performs an action that may affect the shell.
static PyObject *PySHChangeNotify(PyObject *self, PyObject *args)
{
	int flags;
	DWORD eventId;
	void *whatever1;
	void *whatever2;
	if(!PyArg_ParseTuple(args, "lizz:SHChangeNotify",
			&eventId, // @pyparm int|eventId||Describes the event that has occurred.  One of the SHCNE_* constants.
			&flags, // @pyparm int|flags||Flags that indicate the meaning of the other params.
			&whatever1, // @pyparm string|whatever||A path or <o PyIDL>
			&whatever2)) // @pyparm string|whatever||A path or <o PyIDL>
		return NULL;
	// @todo This function does not support integers 
	PY_INTERFACE_PRECALL;
	SHChangeNotify(eventId, flags, whatever1, whatever2);
	PY_INTERFACE_POSTCALL;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIDL>|shell|SHEmptyRecycleBin|Empties the recycle bin on the specified drive.
static PyObject *PySHEmptyRecycleBin(PyObject *self, PyObject *args)
{
	HWND hwnd;
	char *path;
	DWORD flags;
	if(!PyArg_ParseTuple(args, "lzl:SHEmptyRecycleBin",
			&hwnd, // @pyparm int|hwnd||
			&path, // @pyparm string|path||A NULL-terminated string that contains the path of the root drive on which the recycle bin is located. This parameter can contain the address of a string formatted with the drive, folder, and subfolder names (c:\windows\system . . .). It can also contain an empty string or NULL. If this value is an empty string or NULL, all recycle bins on all drives will be emptied.
			&flags)) // @pyparm int|flags||One of the SHERB_* values.
		return NULL;

	typedef HRESULT (* PFNSHEmptyRecycleBin)(HWND, LPSTR, DWORD );
	// @comm This method is only available in shell version 4.71.  If the function is not available, a COM Exception with HRESULT=E_NOTIMPL will be raised.
	HMODULE hmod = GetModuleHandle("shell32.dll");
	PFNSHEmptyRecycleBin pfnSHEmptyRecycleBin = (PFNSHEmptyRecycleBin)GetProcAddress(hmod, "SHEmptyRecycleBinA");
	if (pfnSHEmptyRecycleBin==NULL)
		return OleSetOleError(E_NOTIMPL);

	PY_INTERFACE_PRECALL;
	HRESULT hr = (*pfnSHEmptyRecycleBin)(hwnd, path, flags);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return OleSetOleError(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

/* List of module functions */
// @module shell|A module, encapsulating the ActiveX Control interfaces
static struct PyMethodDef shell_methods[]=
{
    { "SHGetPathFromIDList",    PySHGetPathFromIDList, 1 }, // @pymeth SHGetPathFromIDList|Converts an <o PyIDL> to a path.
    { "SHBrowseForFolder",    PySHBrowseForFolder, 1 }, // @pymeth SHBrowseForFolder|Displays a dialog box that enables the user to select a shell folder.
    { "SHGetFolderPath", PySHGetFolderPath, 1 }, // @pymeth SHGetFolderPath|Retrieves the path of a folder.
    { "SHGetFolderLocation", PySHGetFolderLocation, 1 }, // @pymeth SHGetFolderLocation|Retrieves the <o PyIDL> of a folder.
    { "SHGetSpecialFolderPath", PySHGetSpecialFolderPath, 1 }, // @pymeth SHGetSpecialFolderPath|Retrieves the path of a special folder.
    { "SHGetSpecialFolderLocation", PySHGetSpecialFolderLocation, 1 }, // @pymeth SHGetSpecialFolderLocation|Retrieves the <o PyIDL> of a special folder.
    { "SHAddToRecentDocs", PySHAddToRecentDocs, 1 }, // @pymeth SHAddToRecentDocs|Adds a document to the shell's list of recently used documents or clears all documents from the list. The user gains access to the list through the Start menu of the Windows taskbar.
    { "SHChangeNotify", PySHChangeNotify, 1 }, // @pymeth SHChangeNotify|Notifies the system of an event that an application has performed. An application should use this function if it performs an action that may affect the shell.
    { "SHEmptyRecycleBin", PySHEmptyRecycleBin, 1 }, // @pymeth SHEmptyRecycleBin|Empties the recycle bin on the specified drive.
	{ NULL, NULL },
};


static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] =
{
	PYCOM_INTERFACE_CLIENT_ONLY       (ShellLink),
	PYCOM_INTERFACE_CLSID_ONLY		  (ShellLink),
};

static int AddConstant(PyObject *dict, const char *key, long value)
{
	PyObject *oval = PyInt_FromLong(value);
	if (!oval)
	{
		return 1;
	}
	int rc = PyDict_SetItemString(dict, (char*)key, oval);
	Py_DECREF(oval);
	return rc;
}
static int AddIID(PyObject *dict, const char *key, REFGUID guid)
{
	PyObject *obiid = PyWinObject_FromIID(guid);
	if (!obiid) return 1;
	int rc = PyDict_SetItemString(dict, (char*)key, obiid);
	Py_DECREF(obiid);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)
#define ADD_IID(tok) AddIID(dict, #tok, tok)

/* Module initialisation */
extern "C" __declspec(dllexport) void initshell()
{
	char *modName = "shell";
	PyObject *oModule;
	// Create the module and add the functions
	oModule = Py_InitModule(modName, shell_methods);
	if (!oModule) /* Eeek - some serious error! */
		return;
	PyObject *dict = PyModule_GetDict(oModule);
	if (!dict) return; /* Another serious error!*/

	// Register all of our interfaces, gateways and IIDs.
	PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData, sizeof(g_interfaceSupportData)/sizeof(PyCom_InterfaceSupportInfo));

	ADD_CONSTANT(SLR_NO_UI);
// Some of these are win2k only...
//	ADD_CONSTANT(SLR_NOLINKINFO);
//	ADD_CONSTANT(SLR_INVOKE_MSI);
    ADD_CONSTANT(SLR_ANY_MATCH);
    ADD_CONSTANT(SLR_UPDATE);
    ADD_CONSTANT(SLR_NOUPDATE);
//	ADD_CONSTANT(SLR_NOSEARCH);
//	ADD_CONSTANT(SLR_NOTRACK);
    ADD_CONSTANT(SLGP_SHORTPATH);
    ADD_CONSTANT(SLGP_UNCPRIORITY);
    ADD_CONSTANT(SLGP_RAWPATH);
	ADD_CONSTANT(HOTKEYF_ALT);
	ADD_CONSTANT(HOTKEYF_CONTROL);
	ADD_CONSTANT(HOTKEYF_EXT);
	ADD_CONSTANT(HOTKEYF_SHIFT);
	ADD_IID(CLSID_ShellLink);
}
