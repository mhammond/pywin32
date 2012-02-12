/*
 ======================================================================
 Copyright 2002-2003 by Blackdog Software Pty Ltd.

                         All Rights Reserved

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that copyright notice and this permission
 notice appear in supporting documentation, and that the name of 
 Blackdog Software not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.

 BLACKDOG SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 NO EVENT SHALL BLACKDOG SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ======================================================================
 */

#include "stdafx.h"
#include "Utils.h"

extern HINSTANCE g_hInstance;

static bool g_bRegisteredEventSource = false;

static WCHAR *source_name = L"ISAPI Filter or Extension";

// some py3k-friendly type conversions.
const char *PyISAPIString_AsBytes(PyObject *ob, DWORD *psize /* = NULL */)
{
	PyObject *obNew = NULL;
#if (PY_VERSION_HEX >= 0x03000000)
	// py3k - check for unicode object and use utf-8 encoding.
	if (PyUnicode_Check(ob)) {
		obNew = ob = PyUnicode_AsUTF8String(ob);
		if (ob == NULL)
			return NULL;
	}
#endif
	// These 'PyString_' calls are all mapped to the bytes API in py3k...
	if (!PyString_Check(ob)) {
		PyErr_Format(PyExc_ValueError, "Expected a string object (got %s)", ob->ob_type->tp_name);
		return NULL;
	}
	if (psize)
		*psize = PyString_Size(ob);
	const char *result = PyString_AsString(ob);
	Py_XDECREF(obNew);
	return result;
}

// returns the pathname of this module

TCHAR *GetModulePath(void)
{
	// directory values
	TCHAR szFilePath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];

	// find out where the exe lives
	// NOTE: the long file name does not get returned (don't know why)
	::GetModuleFileName(g_hInstance, szFilePath, sizeof(szFilePath));
	::_tsplitpath( szFilePath, szDrive, szDir, NULL, NULL );
	int dir_len = _tcslen(szDir);
	if (dir_len && szDir[dir_len-1] == _T('\\'))
		szDir[dir_len-1] = _T('\0');

	TCHAR *result = (TCHAR *)malloc((_tcslen(szDrive)+_tcslen(szDir)+1)*sizeof(TCHAR));
	if (result) {
		_tcscpy(result, szDrive);
		_tcscat(result, szDir);
	}
	return result;
}

// Formats a system error code

char *FormatSysError(const DWORD nErrNo)
{
	// This should never happen, so we can be a little brutal.
	char *result = (char *)malloc(1024);
	if (!result) return NULL;
	result[0] = '\0';
	int nLen =FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
				  NULL,
				  nErrNo,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  result,
				  1024,
				  NULL);
	if (nLen > 2) {
		if ( result[nLen - 1] == '\n')
			result[nLen - 1] = 0;
		if (result[nLen - 2] == '\r') 
			result[nLen - 2] = 0;
	}
	return result;
}

// format an error 
char *HTMLErrorResp(const char *msg)
{
	const char *htmlBody =  "<html><head><title>Python ISAPI Error</title></head>"
				"<body><h2>An Error occured while processing your request</h2>"
			    "<font color=\"Red\"> %s </font></body></html>";
	// should not need the "+1" as the "%s" will be consumed, but...
	int newLen = strlen(htmlBody) + strlen(msg) + 1;
	char *result = (char *)malloc(newLen);
	if (result)
		sprintf(result, htmlBody, msg);
	return result;
}

// register the event source with the event log.
static void CheckRegisterEventSourceFile()
{
	WCHAR mod_name[MAX_PATH] = L"";
	if (g_bRegisteredEventSource)
		return;

	GetModuleFileNameW(g_hInstance, mod_name,
			  sizeof mod_name/sizeof WCHAR);
	if (!mod_name[0]) {
		OutputDebugString(_T("GetModuleFileNameW failed!"));
		return;
	}

	HKEY hkey;
	WCHAR keyName[MAX_PATH];

	wcscpy(keyName, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
	wcscat(keyName, source_name);

	BOOL rc = FALSE;
	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, 
		               keyName, 
		               0, 
		               NULL, 
		               REG_OPTION_NON_VOLATILE, 
		               KEY_WRITE, NULL, 
		               &hkey, 
		               NULL) == ERROR_SUCCESS) {
		RegSetValueExW(hkey, L"EventMessageFile", 0, REG_SZ, 
		               (const BYTE *)mod_name,
		               wcslen(mod_name)*sizeof(WCHAR));
		DWORD types = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
		RegSetValueExW(hkey, L"TypesSupported", 0, REG_DWORD,
		               (const BYTE *)&types, sizeof(types));
		RegCloseKey(hkey);
	}
	g_bRegisteredEventSource = true;
}

// Write stuff to the event log.
BOOL WriteEventLogMessage(WORD eventType, DWORD eventID, WORD num_inserts,
                          const char **inserts)
{
	BOOL ok = FALSE;
	HANDLE hEventSource;

	CheckRegisterEventSourceFile();

	hEventSource = RegisterEventSourceW(NULL, source_name);
	if (hEventSource) {
		ReportEventA(hEventSource, // handle of event source
		            eventType,  // event type
		            0,                    // event category
		            eventID,                 // event ID
		            NULL,                 // current user's SID
		            num_inserts,           // strings in lpszStrings
		            0,                    // no bytes of raw data
		            inserts,          // array of error strings
		            NULL);                // no raw data
		DeregisterEventSource(hEventSource);
		ok = TRUE;
	}
	return ok;
}
