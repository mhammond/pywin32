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

#ifndef _WIN32_WCE	// exclude this code under WindowsCE...not supported by CE

#include "structmember.h"

class __declspec(dllexport) PyNCB : public PyObject  
{
public:

	PNCB GetNCB() {return &m_ncb;}

	PyNCB();
	PyNCB(const NCB *);
	~PyNCB();
	void Reset();

	static void	deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int	setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];

	NCB	m_ncb;
	DWORD dwStatus;		// status of this object (used during copy construct)
	PyObject *m_obuserbuffer;   // The object the user gave us for the buffer
	PyObject *m_obbuffer;   // The actual object providing the buffer.
};

extern __declspec(dllexport) PyTypeObject PyNCBType;
#define PyNCB_Check(ob)	((ob)->ob_type == &PyNCBType)

#endif // end of _WIN32_WCE exclude
