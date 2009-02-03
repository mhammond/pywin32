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
#ifndef __PyExtensionObjects_H__
#define __PyExtensionObjects_H__

#include "ControlBlock.h"
#include "tupleobject.h"

class PyVERSION_INFO :public PyObject
{
	HSE_VERSION_INFO * m_pvi;
public:
	PyVERSION_INFO(HSE_VERSION_INFO* pfv);
	~PyVERSION_INFO();
public:	
	void Reset() {m_pvi = NULL;}
	// Python support 
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *name, PyObject *v);
};

class PyECB :public PyObject
{
	CControlBlock * m_pcb;
	DWORD      m_version;            // Version info of this spec
	DWORD      m_totalBytes;         // Total bytes indicated from client
	DWORD      m_available;          // Available number of bytes
	DWORD	   m_HttpStatusCode;     // The status of the current transaction when the request is completed. 

public:
	PyECB(CControlBlock * pcb = NULL);
	~PyECB();

	BOOL Check() {
		if (!m_pcb || !m_pcb->GetECB()) {
			assert(!PyErr_Occurred());
			PyErr_SetString(PyExc_RuntimeError, "Invalid ECB (DoneWithSession has been called)");
			return FALSE;
		}
		return TRUE;
	}
	// Python support 
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);

	// class methods
	static PyObject * WriteClient(PyObject *self, PyObject *args); 
	static PyObject * GetServerVariable(PyObject *self, PyObject *args);
	static PyObject * ReadClient(PyObject *self, PyObject *args);

	// Server support function wrappers
	
	// these wrap the various server support functions supported through the
	// ServerSupportFunction routine.
	
	static PyObject * SendResponseHeaders(PyObject *self, PyObject * args); // HSE_REQ_SEND_RESPONSE_HEADER_EX 	
	static PyObject * Redirect(PyObject *self, PyObject * args);            //  HSE_REQ_SEND_URL_REDIRECT_RESP.
	static PyObject * IsKeepAlive(PyObject *self, PyObject * args); // Keep alive flag set
	static PyObject * DoneWithSession(PyObject *self, PyObject * args);     //HSE_REQ_DONE_WITH_SESSION
	static PyObject * GetImpersonationToken(PyObject *self, PyObject * args); // HSE_REQ_GET_IMPERSONATION_TOKEN
	static PyObject * GetAnonymousToken(PyObject *self, PyObject * args); // HSE_REQ_GET_ANONYMOUS_TOKEN
	static PyObject * TransmitFile(PyObject *self, PyObject * args); // HSE_REQ_TRANSMIT_FILE
	static PyObject * MapURLToPath(PyObject *self, PyObject * args); // HSE_REQ_MAP_URL_TO_PATH
	static PyObject * IsKeepConn(PyObject *self, PyObject * args); // HSE_REQ_IS_KEEP_CONN
	static PyObject * SetFlushFlag(PyObject *self, PyObject * args); // HSE_REQ_SET_FLUSH_FLAG
	static PyObject * ExecURL(PyObject *self, PyObject * args); // HSE_REQ_EXEC_URL
	static PyObject * GetExecURLStatus(PyObject *self, PyObject * args); // HSE_REQ_GET_EXEC_URL_STATUS
	static PyObject * IOCompletion(PyObject *self, PyObject * args); // HSE_REQ_IO_COMPLETION
	static PyObject * ReportUnhealthy(PyObject *self, PyObject * args); // HSE_REQ_REPORT_UNHEALTHY

	static PyObject * IsSessionActive(PyObject *self, PyObject * args);
	static struct PyMemberDef members[];
};

// error handling
static PyObject * PyECB_Error = NULL;
PyObject * SetPyECBError(char *fnName, long err=0);

#endif // __PyExtensionObjects_H__
