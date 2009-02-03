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

class CPythonEngine
{
public:
	CPythonEngine();
	~CPythonEngine();
	bool InitMainInterp(void);
	char m_module_name[_MAX_FNAME];
	static PyObject *          m_reload_exception;
protected:

	bool AddToPythonPath(LPCTSTR pPathName);
	void FindModuleName();
	// couple of vars used to ensure that we intialis exactly once
	static CRITICAL_SECTION m_initLock;
	static bool m_haveInit;
};

typedef enum {
	HANDLER_INIT,
	HANDLER_DO,
	HANDLER_TERM,
} HANDLER_TYPE;

class CPythonHandler
{
public:
	CPythonHandler();
	bool Init(CPythonEngine *engine,
			  const char *factory, const char *nameinit, const char *namedo,
			  const char *nameterm);
	void Term();
	PyObject *Callback(HANDLER_TYPE typ, const char *szFormat, ...);
protected:
	PyObject *DoCallback(HANDLER_TYPE typ, PyObject *args);
	
	bool LoadHandler(bool reload);
	bool CPythonHandler::CheckCallback(const char *cbname, PyObject **cb);
	const char *m_namefactory;
	const char *m_nameinit;
	const char *m_namedo;
	const char *m_nameterm;
	PyObject *m_callback_init; // reference to instance methods.87
	PyObject *m_callback_do;
	PyObject *m_callback_term;
	PyObject *m_handler; // reference to the class instance.
	CPythonEngine *m_engine;
};
// general error handler

void ExtensionError(CControlBlock *pcb, const char *errmsg);
void FilterError(CFilterContext *pfc,  const char *errmsg);

class CEnterLeavePython {
public:
	CEnterLeavePython() : state(PyGILState_Ensure()) {;}
	~CEnterLeavePython() {PyGILState_Release(state);}
protected:
	PyGILState_STATE state;

};

#endif // __PythonEngine_H