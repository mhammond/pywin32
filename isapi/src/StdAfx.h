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

#if !defined(AFX_STDAFX_H__E2A54271_C650_437E_999F_A5E3E2F41ACC__INCLUDED_)
#define AFX_STDAFX_H__E2A54271_C650_437E_999F_A5E3E2F41ACC__INCLUDED_

#include "tchar.h"

#include <httpext.h>
#include <httpfilt.h>
#include "Utils.h"

#define PY_SSIZE_T_CLEAN
// windows defines "small" as "char" which breaks Python's accu.h
#undef small
#include "Python.h"

// include structmember here to deal with warnings related to WRITE_RESTRICTED
#ifdef WRITE_RESTRICTED
#undef WRITE_RESTRICTED
#endif
#include "structmember.h"
// avoid anyone accidently using the wrong WRITE_RESTRICTED...
#undef WRITE_RESTRICTED

// ***** py3k support *****
// Note that when built for py3k, 'UNICODE' is defined, which conveniently
// means TCHAR is the same size as the native unicode object in all versions.
// Note however that ISAPI is always an ANSI API - so even when UNICODE is
// defined, most strings passed and received from ISAPI itself remain 'char *'
// in all versions.

// Macros to handle PyObject layout changes in Py3k
#define PYISAPI_OBJECT_HEAD PyVarObject_HEAD_INIT(NULL, 0)
////#define PYISAPI_ATTR_CONVERT PyUnicode_AsUnicode  // removed in Py3.12+

// A helper that on py3k takes a str or unicode as input and returns a
// string - exactly how the 's#' PyArg_ParseTuple format string does...
// On py2k accepts str objects only.
const char *PyISAPIString_AsBytes(PyObject *ob, DWORD *psize = NULL);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif  // !defined(AFX_STDAFX_H__E2A54271_C650_437E_999F_A5E3E2F41ACC__INCLUDED)
