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

#ifndef __PythonEngine_H
#define __PythonEngine_H

#include "ControlBlock.h"
#include "FilterContext.h"

/////////////////////////////////////////////////////
// Engine exception
////////////////////////////////////////////////////

class CPythonEngineException
{
public:
	CPythonEngineException(LPCTSTR errMsg = "") {m_errStr = strdup(errMsg);}
	~CPythonEngineException(){free(m_errStr);}
public:
	char *m_errStr;
};

class CPythonEngine
{
public:
	CPythonEngine();
	~CPythonEngine();
	bool InitMainInterp(void);
	void ShutdownInterp(void);
	bool LoadHandler(char *factory_name);
	PyObject *GetHandler() {return m_handler;}
	bool SetCallback(const char *callbackName);
	PyObject *Callback(const char *szFormat, ...);
protected:
	bool AddToPythonPath(LPCTSTR pPathName);
	PyObject *m_handler;
	// couple of vars used to ensure that we intialis exactly once
	static CRITICAL_SECTION m_initLock;
	static bool m_haveInit;
	PyObject *          m_callback;
};

// general error handler

void ExtensionError(CControlBlock *pcb, LPCTSTR errmsg);
void FilterError(CFilterContext *pfc,  LPCTSTR errmsg);
#endif // __PythonEngine_H