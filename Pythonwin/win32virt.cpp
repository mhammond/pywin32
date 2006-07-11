/*

	win32 virtuals manager/helper

	Created August 1994, Mark Hammond (MHammond@skippinet.com.au)

*/
#include "stdafx.h"
#include "win32win.h"
#include "win32dc.h"
#include "win32prinfo.h"
#include "win32doc.h"
//////////////////////////////////////////////////////////////////////
//
// virtuals helper
//
//////////////////////////////////////////////////////////////////////

extern BOOL bInFatalShutdown;

CVirtualHelper::CVirtualHelper(const char *iname, const void *iassoc, EnumVirtualErrorHandling veh/* = VEH_PRINT_ERROR */)
{
	handler=NULL;
	py_ob = NULL;
	retVal=NULL;
	csHandlerName = iname;
	vehErrorHandling = veh;
	ui_assoc_object *py_bob = ui_assoc_object::handleMgr.GetAssocObject( iassoc );
	if (bInFatalShutdown || py_bob==NULL)
		return;
	if (!py_bob->is_uiobject( &ui_assoc_object::type)) {
		TRACE("CVirtualHelper::CVirtualHelper Error: Call object is not of required type\n");
		return;
	}
	// ok - have the python data type - now see if it has an override.
	if (py_bob->virtualInst) {
		CEnterLeavePython _celp;
		PyObject *t, *v, *tb;
		PyErr_Fetch(&t,&v,&tb);
		handler = PyObject_GetAttrString(py_bob->virtualInst, (char *)iname);
		if (handler) {
			// explicitely check a method returned, else the classes
			// delegation may cause a circular call chain.
			if (!PyMethod_Check(handler)) {
				if (!PyCFunction_Check(handler)) {
					TRACE("Handler for object is not a method!\n");
				}
				DODECREF(handler);
				handler = NULL;
			}
		}
		PyErr_Restore(t,v,tb);
	}
	py_ob = py_bob;
	Py_INCREF(py_ob);
}
CVirtualHelper::~CVirtualHelper()
{
	// XXX - Gross hack for speed.  This is called for eachh window message
	// so only grabs the Python lock if the objects need Python,
	if ((retVal && retVal->ob_refcnt==1) || 
		(handler && handler->ob_refcnt==1) || 
		(py_ob && py_ob->ob_refcnt==1)) {
		CEnterLeavePython _celp;
		XDODECREF(retVal);
		XDODECREF(handler);
		XDODECREF(py_ob);
	} else {
		XDODECREF(retVal);
		XDODECREF(handler);
		XDODECREF(py_ob);
	}
}
PyObject *CVirtualHelper::GetHandler()
{
	return handler;
}
BOOL CVirtualHelper::do_call(PyObject *args)
{
	CEnterLeavePython _celp;
	XDODECREF(retVal);	// our old one.
	retVal = NULL;
	ASSERT(handler);	// caller must trap this.
	ASSERT(args);
	PyObject *result = gui_call_object(handler,args);
	DODECREF(args);
	if (result==NULL) {
		if (vehErrorHandling==VEH_PRINT_ERROR) {
			char msg[256];
			TRACE("CallVirtual : callback failed with exception\n");
			gui_print_error();
			// this will probably fail if we are already inside the exception handler
			PyObject *obRepr = PyObject_Repr(handler);
			char *szRepr = "<no representation (PyObject_Repr failed)>";
			if (obRepr)
				szRepr = PyString_AsString(obRepr);
			wsprintf(msg, "%s() virtual handler (%s) raised an exception", (const char *)csHandlerName, szRepr);
			Py_XDECREF(obRepr);
			PyErr_SetString(ui_module_error, msg);
			// send to the debugger
			TRACE(msg);
			TRACE("\n");
			// send to the app.
			gui_print_error();
		} else {
			// Error dialog.
			CString csAddnMsg = " when executing ";
			csAddnMsg += csHandlerName;
			csAddnMsg += " handler";

			ExceptionHandler(EHA_DISPLAY_DIALOG, NULL, csAddnMsg);
		}
		return FALSE;
	}
	retVal = result;
	return TRUE;
}

BOOL CVirtualHelper::call_args(PyObject *arglst)
{
	if (!handler) return FALSE;
	return do_call(arglst);
}

BOOL CVirtualHelper::call()
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("()");
	return do_call(arglst);
}
BOOL CVirtualHelper::call(int val)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(i)",val);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(int val, int val2)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(ii)",val, val2);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(int val1, int val2, int val3)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(iii)",val1, val2, val3);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(long val)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(l)",val);
	return do_call(arglst);
}

BOOL CVirtualHelper::call(const char *val)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(z)",val);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(const char *val, int ival)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("(zi)",val,ival);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(PyObject *ob)
{
	if (!handler) return FALSE;
	if (!ob)
		ob=Py_None;
	PyObject *arglst = Py_BuildValue("(O)",ob);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(PyObject *ob, PyObject *ob2)
{
	if (!handler) return FALSE;
	if (!ob)
		ob=Py_None;
	if (!ob2)
		ob2=Py_None;
	PyObject *arglst = Py_BuildValue("(OO)",ob, ob2);
	return do_call(arglst);
}
BOOL CVirtualHelper::call(PyObject *ob, PyObject *ob2, int i)
{
	if (!handler) return FALSE;
	if (!ob)
		ob=Py_None;
	if (!ob2)
		ob2=Py_None;
	PyObject *arglst = Py_BuildValue("(OOi)",ob, ob2, i);
	return do_call(arglst);
}

BOOL CVirtualHelper::call(CDC *pDC)
{
	if (!handler) return FALSE;
	PyObject *dc = (PyObject *) ui_assoc_object::make (ui_dc_object::type,
												   pDC)->GetGoodRet();
	if (!dc) return FALSE;
	PyObject *arglst = Py_BuildValue("(O)",dc);
	BOOL ret = do_call(arglst);
	DODECREF (dc); // the reference I created.
	return ret;
}
BOOL CVirtualHelper::call(CDC *pDC, CPrintInfo *pInfo)
{
	if (!handler) return FALSE;
	PyObject *dc = (PyObject *) ui_assoc_object::make (ui_dc_object::type,
												   pDC)->GetGoodRet();
	if (!dc) return FALSE;
        BOOL ret;
        PyObject *info = NULL;
        PyObject *arglst = NULL;
        if (pInfo != NULL) {
          info = (PyObject *) ui_assoc_object::make (ui_prinfo_object::type,
                                                     pInfo)->GetGoodRet();
          if (!info) return FALSE;
          arglst = Py_BuildValue("(OO)",dc, info);
        } else {
          arglst = Py_BuildValue("(Oz)",dc, NULL);
        }
        ret = do_call(arglst);
	DODECREF (dc); // the reference I created.
        if (pInfo != NULL) {
          DODECREF (info); // the reference I created.
        }
	return ret;
}
BOOL CVirtualHelper::call(CPrintInfo *pInfo)
{
	if (!handler) return FALSE;
        PyObject *info = NULL;
        PyObject *arglst;
        if (pInfo) {
          info = (PyObject *) ui_assoc_object::make (ui_prinfo_object::type,
                                                     pInfo)->GetGoodRet();
          if (!info) return FALSE;
          arglst = Py_BuildValue("(O)",info);
        } else {
          arglst = Py_BuildValue("(z)",NULL);
        }
        BOOL ret = do_call(arglst);
        DODECREF (info); // the reference I created.
	return ret;
}
BOOL CVirtualHelper::call(CWnd *pWnd)
{
	if (!handler) return FALSE;
	PyObject *wnd = PyWinObject_FromCWnd(pWnd);
	if (!wnd) return FALSE;
	PyObject *arglst = Py_BuildValue("(O)",wnd);
	BOOL ret = do_call(arglst);
	DODECREF (wnd); // the reference I created.
	return ret;
}

BOOL CVirtualHelper::call(CWnd *pWnd, int i)
{
	if (!handler) return FALSE;
	PyObject *wnd = PyWinObject_FromCWnd(pWnd);
	if (!wnd) return FALSE;
	PyObject *arglst = Py_BuildValue("(Oi)",wnd, i);
	BOOL ret = do_call(arglst);
	DODECREF (wnd); // the reference I created.
	return ret;
}

BOOL CVirtualHelper::call(CWnd *pWnd, int i, int i2)
{
	if (!handler) return FALSE;
	PyObject *wnd = PyWinObject_FromCWnd(pWnd);
	if (!wnd) return FALSE;
	PyObject *arglst = Py_BuildValue("(Oii)",wnd, i, i2);
	BOOL ret = do_call(arglst);
	DODECREF (wnd); // the reference I created.
	return ret;
}

BOOL CVirtualHelper::call(CDC *pDC, CWnd *pWnd, int i)
{
	PyObject *wnd ;
	if (pWnd==NULL) {
		wnd = Py_None;
		DOINCREF(wnd);
	} else {
		wnd = PyWinObject_FromCWnd(pWnd);
		if (!wnd) return FALSE;
	}
	PyObject *dc = (PyObject *) ui_assoc_object::make (ui_dc_object::type,
												   pDC)->GetGoodRet();
	if (!dc) {
		Py_DECREF(wnd);
		return FALSE;
	}
	PyObject *arglst = Py_BuildValue("(OOi)",dc, wnd, i);
	BOOL ret = do_call(arglst);
	Py_DECREF(wnd);
	Py_DECREF(dc);
	return ret;

}
BOOL CVirtualHelper::call(CView *pWnd, PyObject *ob)
{
	if (!handler) return FALSE;
	if (!ob)
		ob=Py_None;
	PyObject *wnd ;
	if (pWnd==NULL) {
		wnd = Py_None;
		DOINCREF(wnd);
	} else {
		wnd = PyWinObject_FromCWnd(pWnd);
		if (!wnd) return FALSE;
	}
	PyObject *arglst = Py_BuildValue("(OO)",wnd,ob);
	BOOL ret = do_call(arglst);
	DODECREF (wnd); // the reference I created.
	return ret;
}

BOOL CVirtualHelper::call(BOOL boolVal, CWnd *pWnd1, CWnd *pWnd2)
{
	if (!handler) return FALSE;
	PyObject *wnd1;
	if (pWnd1) {
		wnd1 = PyWinObject_FromCWnd(pWnd1);
		if (!wnd1) return FALSE;
	} else {
		Py_INCREF(Py_None);
		wnd1 = Py_None;
	}
	PyObject *wnd2;
	if (pWnd2) {
		wnd2 = 	PyWinObject_FromCWnd(pWnd2);
		if (!wnd2) return FALSE;
	} else {
		Py_INCREF(Py_None);
		wnd2 = Py_None;
	}
	PyObject *arglst = Py_BuildValue("(iOO)",boolVal, wnd1, wnd2);
	BOOL ret = do_call(arglst);
	DODECREF (wnd1); // the reference I created.
	DODECREF (wnd2); // the reference I created.
	return ret;
}

BOOL CVirtualHelper::call(CDocument *pDoc)
{
	if (!handler) return FALSE;
	PyObject *doc = (PyObject *) ui_assoc_object::make (PyCDocument::type,
												   pDoc)->GetGoodRet();
	if (!doc) return FALSE;
	PyObject *arglst = Py_BuildValue("(O)",doc);
	BOOL ret = do_call(arglst);
	DODECREF (doc); // ref I created.
	return ret;
}
BOOL CVirtualHelper::call(LPCREATESTRUCT lpcs, PyObject *ob)
{
	if (!handler) return FALSE;
	PyObject *cs = PyObjectFromCreateStruct(lpcs);
	if (!cs) return FALSE;
	if (ob==NULL) ob = Py_None;
	PyObject *arglst = Py_BuildValue("(O,O)", cs, ob );
	DODECREF(cs); // ref I created.
	BOOL ret = do_call(arglst);
	return ret;
}
BOOL CVirtualHelper::call(LPCREATESTRUCT lpcs)
{
	if (!handler) return FALSE;
	PyObject *cs = PyObjectFromCreateStruct(lpcs);
	if (!cs) return FALSE;
	PyObject *arglst = Py_BuildValue("(O)", cs);
	BOOL ret = do_call(arglst);
	DODECREF(cs); // my reference.
	return ret;
}

BOOL CVirtualHelper::call(const MSG *msg)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("((iiiii(ii)))",msg->hwnd,msg->message,msg->wParam,msg->lParam,msg->time,msg->pt.x,msg->pt.y);
	BOOL ret = do_call(arglst);
	return ret;
}

BOOL CVirtualHelper::call(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo)
{
	if (!handler) return FALSE;
	PyObject *arglst = Py_BuildValue("iill",nID, nCode, (long)pExtra, (long)pHandlerInfo);
	BOOL ret = do_call(arglst);
	return ret;
}

BOOL CVirtualHelper::retnone()
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	return (retVal==Py_None);
}

BOOL CVirtualHelper::retval( MSG *msg )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	CEnterLeavePython _celp;
	if (!PyArg_ParseTuple(retVal, "(iiiii(ii))", &msg->hwnd,&msg->message,&msg->wParam,&msg->lParam,&msg->time,&msg->pt.x,&msg->pt.y)) {
		PyErr_Clear();
		return FALSE;
	}
	return TRUE;
}
BOOL CVirtualHelper::retval( int &ret )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	if (retVal==Py_None) {
		ret = 0;
		return TRUE;
	}
	CEnterLeavePython _celp;
	ret = PyInt_AsLong(retVal);
	return !PyErr_Occurred();
}
BOOL CVirtualHelper::retval( long &ret )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	if (retVal==Py_None) {
		ret = 0;
		return TRUE;
	}
	CEnterLeavePython _celp;
	ret = PyInt_AsLong(retVal);
	if (PyErr_Occurred()) {
		PyErr_Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CVirtualHelper::retval( char *&ret )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	if (retVal==Py_None) {
		ret = NULL;
		return TRUE;
	}
	CEnterLeavePython _celp;
	ret = PyString_AsString(retVal);
	if (PyErr_Occurred()) {
		PyErr_Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CVirtualHelper::retval( CString &ret )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	if (retVal==Py_None) {
		ret.Empty();
		return TRUE;
	}
	CEnterLeavePython _celp;
	ret = PyString_AsString(retVal);
	if (PyErr_Occurred()) {
		PyErr_Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CVirtualHelper::retval( _object* &ret )
{
	ASSERT(retVal);
	if (!retVal)
		return FALSE;	// failed - assume didnt work in non debug
	ret = retVal;
/** what was I thinking?
	CEnterLeavePython _celp;
	if (!PyArg_Parse(retVal, "O",&ret)) {
		PyErr_Clear();
		return FALSE;
	}
**/
	return TRUE;
}

BOOL CVirtualHelper::retval( CREATESTRUCT &cs )
{
	ASSERT(retVal);
	if (!retVal || retVal==Py_None)
		return FALSE;	// failed - assume didnt work in non debug
	CEnterLeavePython _celp;
	if (!CreateStructFromPyObject(&cs, retVal)) {
		char msgBuf[128];

		gui_print_error();
		wsprintf(msgBuf, "virtual %s: The return value can not be converted from a CREATESTRUCT tuple",
				(const char *)csHandlerName );
		PyErr_SetString(PyExc_TypeError, msgBuf);
		gui_print_error();
		return FALSE;
	}
	return TRUE;
}

