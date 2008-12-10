/******************************************************************
* Copyright (c) 1998-1999 Cisco Systems, Inc. All Rights Reserved
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted,
* provided that the above copyright notice appear in all copies and that
* both that copyright notice and this permission notice appear in
* supporting documentation.
*
*
* CISCO SYSTEMS, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL CISCO SYSTEMS BE LIABLE FOR ANY LOST REVENUE, 
* PROFIT OR DATA, OR FOR SPECIAL, INDIRECT, CONSEQUENTIAL OR INCIDENTAL
* DAMAGES OR ANY OTHER DAMAGES WHATSOEVER, HOWEVER CAUSED AND REGARDLESS
* OF THE THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************/


#ifndef _NTRES
#define _NTRES
#endif

#include "structmember.h"

#define MAX_COMMENT	256 * sizeof(TCHAR)
#define MAX_NAME	256 * sizeof(TCHAR)
class __declspec(dllexport) PyNETRESOURCE: public PyObject
{
public:
	NETRESOURCE *GetNetresource() {return &m_nr;}

	PyNETRESOURCE(void);
	PyNETRESOURCE(const NETRESOURCE *);
	~PyNETRESOURCE(void);

	/* Python support*/
	int compare(PyObject *ob);

	static void		deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int		setattro(PyObject *self, PyObject *obname, PyObject *v);
	static int		compareFunc(PyObject *ob1, PyObject *ob2);
	static struct PyMemberDef members[];

protected:
/* NETRESOURCE contains pointer to strings (LPTSTR) to four items.
	These are allocated and released by PyWinObject_AsWCHAR and PyWinObject_FreeWCHAR
*/
	NETRESOURCE m_nr;
};

extern __declspec(dllexport) PyTypeObject PyNETRESOURCEType;
#define PyNETRESOURCE_Check(ob)	((ob)->ob_type == &PyNETRESOURCEType)

__declspec(dllexport) BOOL PyWinObject_AsNETRESOURCE(PyObject *ob, NETRESOURCE **ppNetresource, BOOL bNoneOK = TRUE);
__declspec(dllexport) PyObject *PyWinObject_FromNETRESOURCE(const NETRESOURCE *pNetresource);
