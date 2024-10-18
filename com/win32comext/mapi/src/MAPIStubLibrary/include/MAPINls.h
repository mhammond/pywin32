/*
 *	M A P I N L S . H
 *
 *	Internationalization Support Utilities
 *
 *  Copyright (c) 2009 Microsoft Corporation. All Rights Reserved.
 */

#ifndef _MAPINLS_H_
#define _MAPINLS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (WIN64) && !defined (_WIN64)
#define _WIN64
#endif

#if defined (WIN32) && !defined (_WIN32)
#define _WIN32
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* We don't want to include windows.h in case that conflicts with an */
/* earlier inclusion of compobj.h */

#if !defined(WINAPI)
	#if defined(_WIN64) && (_MSC_VER >= 800)
		#define WINAPI
	#elif defined(_M_ARM) && (_MSC_VER >= 800)
		#define WINAPI
	#elif defined(_WIN32) && (_MSC_VER >= 800)
		#define WINAPI				__stdcall
	#else
		#error	"Unknown Platform: MAPI is currently supported on Win32 and Win64"
	#endif
#endif

#if defined(DOS) || defined(_MAC)
#include <string.h>
#endif

#ifndef FAR
#define FAR
#endif

typedef unsigned char				    BYTE;
typedef unsigned short      			WORD;
typedef unsigned long       			DWORD;
typedef unsigned int	 				UINT;
typedef int		 						BOOL;

#ifndef __CHAR_DEFINED__
typedef char							CHAR;
#endif

#ifdef UNICODE
typedef WCHAR							TCHAR;
#else
typedef char							TCHAR;
#endif

#if !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short WCHAR;
#endif

typedef WCHAR FAR *						LPWSTR;
typedef const WCHAR FAR *				LPCWSTR;
typedef CHAR FAR *						LPSTR;
typedef const CHAR FAR *				LPCSTR;
typedef TCHAR FAR *						LPTSTR;
typedef const TCHAR FAR *				LPCTSTR;
typedef DWORD 							LCID;
typedef const void FAR *				LPCVOID;

#ifndef _MAC
#ifndef LPOLESTR
#if defined(_WIN64) || defined(_WIN32) || defined(_M_ARM)
#define LPOLESTR        LPWSTR
#define LPCOLESTR       LPCWSTR
#define OLECHAR         WCHAR
#define OLESTR(str)     L##str
#else
#error	"Unknown Platform: MAPI is currently supported on Win32 and Win64"
#endif /* _WIN64 || _WIN32*/
#endif /* LPOLESTR */
#endif /* _MAC */

#define NORM_IGNORECASE      			0x00000001     /* ignore case */
#define NORM_IGNORENONSPACE  			0x00000002     /* ignore diacritics */
#define NORM_IGNORESYMBOLS   			0x00000004     /* ignore symbols */

#if defined(_WIN64) || defined (_WIN32) || defined(_M_ARM) /* from winnls.h */
#define NORM_IGNOREKANATYPE				0x00010000     /* ignore kanatype */
#define NORM_IGNOREWIDTH				0x00020000	   /* ignore width */
#else
#error	"Unknown Platform: MAPI is currently supported on Win32 and Win64"
#endif

#if defined(DOS) || defined(_MAC)

#define	IsBadReadPtr(lp, cb)			(FALSE)
#define IsBadWritePtr(lp, cb)			(FALSE)
#define IsBadHugeReadPtr(lp, cb)		(FALSE)
#define IsBadHugeWritePtr(lp, cb)		(FALSE)
#define IsBadCodePtr(lpfn)				(FALSE)
#ifdef _MAC
#undef IsBadStringPtr
#endif
#define IsBadStringPtr(lpsz, cchMax)	(FALSE)
#define IsBadStringPtrA(lpsz, cchMax)	(FALSE)

#if defined(DOS)

#define	lstrcpyA						strcpy
#define	lstrlenA						strlen
#define	lstrcmpA						strcmp
#define lstrcmp							strcmp
#define lstrcmpi						strcmpi
#define lstrcpy							strcpy
#define lstrcat							strcat
#define lstrlen							strlen
#define wsprintf						sprintf

#endif
#endif

#if defined(DOS) || defined(WIN16)
/* Simulate effect of afx header */
#define __T(x)		x
#define _T(x)		__T(x)
#define TEXT		_T
#endif

#define CP_ACP		0		/* default to ANSI code page */
#define CP_OEMCP	1		/* default to OEM  code page */

LCID	WINAPI	MNLS_GetUserDefaultLCID(void);
UINT	WINAPI	MNLS_GetACP(void);
int		WINAPI	MNLS_CompareStringA(LCID Locale, DWORD dwCmpFlags,
					LPCSTR lpString1, int cchCount1, LPCSTR lpString2,
					int cchCount2);
int		WINAPI	MNLS_CompareStringW(LCID Locale, DWORD dwCmpFlags,
					LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2,
					int cchCount2);
int		WINAPI	MNLS_MultiByteToWideChar(UINT uCodePage, DWORD dwFlags,
					LPCSTR lpMultiByteStr, int cchMultiByte,
					LPWSTR lpWideCharStr, int cchWideChar);
int		WINAPI	MNLS_WideCharToMultiByte(UINT uCodePage, DWORD dwFlags,
					LPCWSTR lpWideCharStr, int cchWideChar,
					LPSTR lpMultiByteStr, int cchMultiByte,
					LPCSTR lpDefaultChar, BOOL FAR *lpfUsedDefaultChar);
int		WINAPI	MNLS_lstrlenW(LPCWSTR lpString);
int		WINAPI	MNLS_lstrcmpW(LPCWSTR lpString1, LPCWSTR lpString2);
LPWSTR	WINAPI	MNLS_lstrcpyW(LPWSTR lpString1, LPCWSTR lpString2);
BOOL	WINAPI	MNLS_IsBadStringPtrW(LPCWSTR lpsz, UINT ucchMax);

#if (defined (_WIN64) || defined(_WIN32) || defined(_M_ARM)) && !defined(_WINNT) && !defined(_WIN95) && !defined(_MAC)
#define _WINNT
#endif

#if !defined(_WINNT) && !defined(_WIN95)
#define GetUserDefaultLCID		MNLS_GetUserDefaultLCID
#define GetACP					MNLS_GetACP
#define MultiByteToWideChar		MNLS_MultiByteToWideChar
#define WideCharToMultiByte		MNLS_WideCharToMultiByte
#define CompareStringA			MNLS_CompareStringA
#endif

#if !defined(MAPI_NOWIDECHAR)

#define lstrlenW				MNLS_lstrlenW
#define lstrcmpW				MNLS_lstrcmpW
#define lstrcpyW				MNLS_lstrcpyW
#define CompareStringW			MNLS_CompareStringW

#if defined(_WINNT) || defined(_WIN95)
#define IsBadStringPtrW			MNLS_IsBadStringPtrW
#elif defined(_MAC)
#define IsBadStringPtrW(lpsz, cchMax)			(FALSE)
#else
#error	"Unknown Platform: MAPI is currently supported on Win32 and Win64"
#define IsBadStringPtrW			(FALSE)
#endif

#endif	/* ! MAPI_NOWIDECHAR */

#ifdef __cplusplus
}
#endif

#endif /* _MAPINLS_H_ */


