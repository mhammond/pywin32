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

#include "structmember.h"
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
	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
};

class PyECB :public PyObject
{
	CControlBlock * m_pcb;

	DWORD      m_version;            // Version info of this spec
	HCONN      m_connID;             // Context number not to be modified!

	PyObject * m_method;             // REQUEST_METHOD
	PyObject * m_queryString;        // QUERY_STRING
	PyObject * m_pathInfo;           // PATH_INFO
	PyObject * m_pathTranslated;     // PATH_TRANSLATED

	DWORD      m_totalBytes;         // Total bytes indicated from client
	DWORD      m_available;          // Available number of bytes
	PyObject * m_data;               // Pointer to cbAvailable bytes
	PyObject * m_contentType;        // Content type of client data

	DWORD	   m_HttpStatusCode;     // The status of the current transaction when the request is completed. 
	PyObject * m_logData;            // log data string 

	bool	   m_bAsyncDone;		// sent the async done

public:
	PyECB(CControlBlock * pcb = NULL);
	~PyECB();
	
	bool FinishedResponse(void) { return m_bAsyncDone; }
public:	
	// Python support 
	static void deallocFunc(PyObject *ob);
	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);

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

	static PyObject * IsSessionActive(PyObject *self, PyObject * args);

protected:

#pragma warning( disable : 4251 )
	static struct memberlist PyECB_memberlist[];
#pragma warning( default : 4251 )

};

// error handling
static PyObject * PyECB_Error = NULL;
PyObject * SetPyECBError(char *fnName, long err=0);


#endif // __PyExtensionObjects_H__