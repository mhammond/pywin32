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

// returns the pathname of this module

char *GetModulePath(void)
{
	// directory values
	TCHAR szFilePath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];

	// find out where the exe lives
	// NOTE: the long file name does not get returned (don't know why)
	::GetModuleFileName(g_hInstance, szFilePath, sizeof(szFilePath));
	::_splitpath( szFilePath, szDrive, szDir, NULL, NULL );
	int dir_len = strlen(szDir);
	if (dir_len && szDir[dir_len-1] == '\\')
		szDir[dir_len-1] = '\0';

	char *result = (char *)malloc(strlen(szDrive)+strlen(szDir)+1);
	if (result) {
		strcpy(result, szDrive);
		strcat(result, szDir);
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
	int nLen =FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
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
char *HTMLErrorResp(LPCTSTR msg)
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
