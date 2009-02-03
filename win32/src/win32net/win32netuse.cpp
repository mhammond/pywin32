// win32netuse.cpp
//
// NetUse functions
//
// @doc
#include "PyWinTypes.h"
#include "lm.h"
#include "lmuseflg.h"
#include "win32net.h"
#include "stddef.h"

#define UI0_ENTRY(name, t, r) { #name, t, offsetof(USE_INFO_0, ui0_##name), r }
// @object PyUSE_INFO_0|A dictionary holding the infomation in a Win32 USE_INFO_0 structure.
static struct PyNET_STRUCT_ITEM ui0[] = {
	UI0_ENTRY(local, NSI_WSTR, 0), // @prop string/<o PyUnicode>|local|
	UI0_ENTRY(remote, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remote|
	{NULL}
};

#define UI1_ENTRY(name, t, r) { #name, t, offsetof(USE_INFO_1, ui1_##name), r }
// @object PyUSE_INFO_1|A dictionary holding the infomation in a Win32 USE_INFO_1 structure.
static struct PyNET_STRUCT_ITEM ui1[] = {
	UI1_ENTRY(local, NSI_WSTR, 0), // @prop string/<o PyUnicode>|local|
	UI1_ENTRY(remote, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remote|
    UI1_ENTRY(password, NSI_WSTR, 0), // @prop strng/<o PyUnicode>|password|
    UI1_ENTRY(status, NSI_DWORD, 0), // @prop int|status|
    UI1_ENTRY(asg_type, NSI_DWORD, 0), // @prop int|asg_type|
    UI1_ENTRY(refcount, NSI_DWORD, 0), // @prop int|refcount|
    UI1_ENTRY(usecount, NSI_DWORD, 0), // @prop int|usecount|
	{NULL}
};

#define UI2_ENTRY(name, t, r) { #name, t, offsetof(USE_INFO_2, ui2_##name), r }
// @object PyUSE_INFO_2|A dictionary holding the infomation in a Win32 USE_INFO_2 structure.
static struct PyNET_STRUCT_ITEM ui2[] = {
	UI2_ENTRY(local, NSI_WSTR, 0), // @prop string/<o PyUnicode>|local|
	UI2_ENTRY(remote, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remote|
    UI2_ENTRY(password, NSI_WSTR, 0), // @prop strng/<o PyUnicode>|password|
    UI2_ENTRY(status, NSI_DWORD, 0), // @prop int |status|
    UI2_ENTRY(asg_type, NSI_DWORD, 0), // @prop int |asg_type|
    UI2_ENTRY(refcount, NSI_DWORD, 0), // @prop int |refcount|
    UI2_ENTRY(usecount, NSI_DWORD, 0), // @prop int |usecount|
	UI2_ENTRY(username, NSI_WSTR, 0), // @prop string/<o PyUnicode>|username|
	UI2_ENTRY(domainname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|domainname|
	{NULL}
};

#define UI3_ENTRY(name, t, r) { #name, t, offsetof(USE_INFO_3, ui3_##name), r }
// @object PyUSE_INFO_3|A dictionary holding the infomation in a Win32 USE_INFO_3 structure.
static struct PyNET_STRUCT_ITEM ui3[] = {
	UI2_ENTRY(local, NSI_WSTR, 0), // @prop string/<o PyUnicode>|local|
	UI2_ENTRY(remote, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remote|
    UI2_ENTRY(password, NSI_WSTR, 0), // @prop strng/<o PyUnicode>|password|
    UI2_ENTRY(status, NSI_DWORD, 0), // @prop int |status|
    UI2_ENTRY(asg_type, NSI_DWORD, 0), // @prop int |asg_type|
    UI2_ENTRY(refcount, NSI_DWORD, 0), // @prop int |refcount|
    UI2_ENTRY(usecount, NSI_DWORD, 0), // @prop int |usecount|
	UI2_ENTRY(username, NSI_WSTR, 0), // @prop string/<o PyUnicode>|username|
	UI2_ENTRY(domainname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|domainname|
    UI3_ENTRY(flags, NSI_DWORD, 0), // @prop int |flags|
};

// @object PyUSE_INFO_*|The following USE_INFO levels are supported.
static struct PyNET_STRUCT use_infos[] = { // @flagh Level|Data
	{ 0, ui0, sizeof(USE_INFO_0) },        // @flag 0|<o PyUSE_INFO_0>
	{ 1, ui1, sizeof(USE_INFO_1) },		   // @flag 1|<o PyUSE_INFO_1>
	{ 2, ui2, sizeof(USE_INFO_2) },		   // @flag 2|<o PyUSE_INFO_2>
	{ 3, ui3, sizeof(USE_INFO_3) },		   // @flag 3|<o PyUSE_INFO_3>
	{ 0, NULL, 0}
};

// @pymethod |win32net|NetUseAdd|Establishes connection between local or NULL device name and a shared resource through redirector
PyObject *
PyNetUseAdd(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	DWORD level;
	DWORD err = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the share data in the format of <o PyUSE_INFO_*>.
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &level, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, use_infos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

	err = NetUseAdd(szServer, level, buf, NULL);
	if (err) {
		ReturnNetError("NetUseAdd",err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetUseAdd
}

// @pymethod |win32net|NetUseDel|Ends connection to a shared resource.
PyObject *
PyNetUseDel(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|useName||The share name
	// @pyparm int|forceCond|0|Level of force to use. Can be USE_FORCE or USE_NOFORCE or USE_LOTS_OF_FORCE
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obUseName, *obServer;
	PyObject *ret = NULL;

	int forceCond = 0;
	DWORD err = 0;

	if (!PyArg_ParseTuple(args, "OO|i", &obServer, &obUseName, &forceCond))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obUseName, &szName, FALSE))
		goto done;

	err = NetUseDel(szServer, szName, (DWORD)forceCond);
	if (err) {
		ReturnNetError("NetUseDel",err);	
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);

done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);

	return ret;
	// @pyseeapi NetUseDel
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetUseEnum|Retrieves information about transport protocols that are currently managed by the redirector
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyUSE_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *
PyNetUseEnum(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szDomain = NULL;
	PyObject *obServer, *obDomain = Py_None;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The level of data required. Currently levels 0, 1 and
	// 2 are supported.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	if (!PyArg_ParseTuple(args, "Oi|ii", &obServer, &level, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, use_infos, &pInfo))
		goto done;

	err = NetUseEnum(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError("NetUseEnum",err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("Oll", list, totalEntries, resumeHandle);
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetUseEnum
}

// @pymethod dict|win32net|NetUseGetInfo|Retrieves information about the configuration elements for a workstation
PyObject *
PyNetUseGetInfo(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szUse = NULL;
	PyObject *obServer, *obUse;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int level = 0;
	DWORD err;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm string/<o PyUnicode>|usename||The name of the locally mapped resource.
	// @pyparm int|level|0|The information level contained in the data. NOTE: levels 302 and 402 don't seem to work correctly. They return error 124. So currently these info levels are not available.
	if (!PyArg_ParseTuple(args, "OO|i", &obServer, &obUse, &level))
		return NULL;

	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obUse, &szUse, TRUE))
		goto done;
	if (!FindNET_STRUCT(level, use_infos, &pInfo))
		goto done;

	err = NetUseGetInfo(szServer, szUse, level, &buf);

	if (err) {
		ReturnNetError("NetUseGetInfo",err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szUse);
	return ret;
	// @pyseeapi NetUseGetInfo
	// @rdesc The result will be a dictionary in one of the <o PyUSE_INFO_*>
	// formats, depending on the level parameter.
}
