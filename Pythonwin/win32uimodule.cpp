/*

	first hack at a UI module, built using MFC V2.0 (on NT)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"
#include <commdlg.h>
#include "win32uiHostGlue.h"
#include "win32win.h"
#include "win32control.h"
#include "win32doc.h"
#include "win32menu.h"
#include "win32dlg.h"
#include "win32dc.h"
#include "win32gdi.h"
#include "win32brush.h"
#include "win32bitmap.h"
#include "win32font.h"
#include "win32dll.h"
#include "win32splitter.h"
#include "win32toolbar.h"
#include "win32prop.h"
#include "win32template.h"
#include "win32pen.h"
#include "win32RichEdit.h"
#include "win32RichEditDocTemplate.h"
#include "win32dlgbar.h"
#ifdef HIER_LIST
#include "win32hl.h"
#endif
#ifdef _DEBUG_HEAP
#	include "malloc.h"		// for _heapchk
#endif

#include "reswin32ui.h"
#include "sysmodule.h"

extern "C" __declspec(dllimport) int PyCallable_Check(PyObject *);	// python - object.c
extern DWORD DebuggerThreadFunc( LPDWORD lpdwWhatever );

static char BASED_CODE uiModName[] = "win32ui";
static char BASED_CODE errorName[] = "win32ui";

PYW_EXPORT PyObject *ui_module_error;
Win32uiHostGlue *pHostGlue = NULL;

// When TRUE, we are in an abort or after shutdown mode, and therefore should
// do nothing related to Python at all!
BOOL bInFatalShutdown = FALSE;

PyObject *ReturnAPIError(const char *fn)
{
    CString msg=GetAPIErrorString((char *)fn);
    PyErr_SetString(ui_module_error,msg.GetBuffer(0));
    return NULL;
}

/////////////////////////////////////////////////////////////////////////
//
// Windows API Hook
// This is used to trap a DESTROY message.  I use this, rather than
// the Translate functions, so that a non CWnd window can still be used.
// For example, in the future, the common dialog boxes may have Python
// support, so it is critical we know when they have been destroyed.

// max speed for hook function (even when debugging!!)
#pragma optimize("2", on)
// Windows hook.
static HHOOK hhook = 0;
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// hooking seems to slow down the system alot - go for speed
	// (but still gotta follow the rules!)
	if (nCode==HC_ACTION && !bInFatalShutdown) {	// I can process it.
	    CWPSTRUCT *cwp = (CWPSTRUCT *)lParam;
		MSG msg;
		msg.message = cwp->message;
		msg.hwnd = cwp->hwnd;
		msg.lParam=cwp->lParam;
		msg.wParam=cwp->wParam;
		msg.time=0 ; // set these to zero - better value?
		msg.pt.x = msg.pt.y = 0;
		Python_check_message(&msg);

		if (cwp->message==WM_NCDESTROY) {	// seems to be last decent message
			CWnd *wnd=CWnd::FromHandlePermanent(cwp->hwnd);
			if (wnd)
				Python_delete_assoc( wnd );
		}
	}
	return CallNextHookEx(hhook, nCode,wParam, lParam);
}
// back to default.
#pragma optimize("", on)

BOOL HookWindowsMessages()
{
	hhook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc,
                            (HINSTANCE) NULL, GetCurrentThreadId());
	if (!hhook)
		TRACE("Hooking windows messages failed\n");

	return (hhook!=0);
}

/////////////////////////////////////////////////////////////////////
//
// ui_type object
//
//////////////////////////////////////////////////////////////////////
ui_type::ui_type( const char *name, ui_type *pBase, int typeSize, struct PyMethodDef* methodList, ui_base_class * (* thector)() )
{
// originally, this copied the typeobject of the parent, but as it is impossible
// to gurantee order of static object construction, I went this way.  This is 
// probably better, as is forces _all_ python objects have the same type sig.
	static PyTypeObject type_template = {
		PyObject_HEAD_INIT(&PyType_Type)
		0,													/*ob_size*/
		"template",											/*tp_name*/
		sizeof(ui_base_class), 								/*tp_size*/
		0,													/*tp_itemsize*/
		/* methods */
		(destructor) ui_base_class::sui_dealloc, 			/*tp_dealloc*/
		0,													/*tp_print*/
		(getattrfunc) ui_base_class::sui_getattr, 			/*tp_getattr*/
		(setattrfunc) ui_base_class::sui_setattr,			/*tp_setattr*/
		0,													/*tp_compare*/
		(reprfunc)ui_base_class::sui_repr,					/*tp_repr*/
    	0,													/*tp_as_number*/
	};

	*((PyTypeObject *)this) = type_template;
	methods = methodList;
	// cast away const, as Python doesnt use it.
	tp_name = (char *)name;
	tp_basicsize = typeSize;
	base = pBase;
	ctor = thector;
}
ui_type::~ui_type()
{
}

//////////////////////////////
//
// ui_type_CObject
ui_type_CObject::CRuntimeClassTypeMap *ui_type_CObject::typemap = NULL;

ui_type_CObject::ui_type_CObject( const char *name, ui_type *pBaseType, CRuntimeClass *pRT, int typeSize, struct PyMethodDef* methodList, ui_base_class * (* thector)() ):
	  ui_type(name, pBaseType, typeSize, methodList, thector )
{
	pCObjectClass = pRT;
	if (pRT) {
		if (!typemap) typemap = new CRuntimeClassTypeMap;
		typemap->SetAt(pCObjectClass, this);
	}
}

ui_type_CObject::~ui_type_CObject()
{
	if (pCObjectClass) {
		typemap->RemoveKey(pCObjectClass);
		if (typemap->IsEmpty()) {
			delete typemap;
			typemap = NULL;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
// base class object
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
IMPLEMENT_DYNAMIC(ui_base_class, CObject);
#endif

ui_base_class::ui_base_class()
{
	strcpy(sig, SIG);
}
ui_base_class::~ui_base_class()
{
#ifdef TRACE_LIFETIMES
	TRACE("Destructing a '%s' at %p\n", ob_type->tp_name, this);
#endif
}

ui_base_class *ui_base_class::make( ui_type &makeTypeRef)
{
	ui_type *makeType = &makeTypeRef; // use to pass ptr as param!
	if (makeType->ctor==NULL) {
		RETURN_ERR("Internal error - the type does not declare a win32ui constructor");
	}
	
	ui_base_class *pNew = (*makeType->ctor)();
	pNew->ob_type = makeType;
	_Py_NewReference(pNew);
#ifdef _DEBUG	// this is really only for internal errors, and they should be ironed out!
	if (!pNew->is_uiobject(makeType))
		RETURN_ERR("Internal error - created type isnt what was requested!");
#endif
#ifdef TRACE_LIFETIMES
	TRACE("Constructing a '%s' at %p\n",pNew->ob_type->tp_name, pNew);
#endif
	return pNew;
}
/*static*/ BOOL ui_base_class::is_uiobject(PyObject *&o, ui_type *which)
{
	ui_base_class *ob = (ui_base_class *)o;
	if (ob==NULL || ob==Py_None)
		return FALSE;
	// quick fasttrack.
	if ((ui_type *)ob->ob_type==which)
		return TRUE;
	// if Python instance, my be able to derive the paired Python type.
	if (PyInstance_Check(ob)) {
		PyObject *obattr= PyObject_GetAttrString(ob, "_obj_");
		if (obattr==NULL) {
			PyErr_Clear();
			TRACE("is_uiobject fails due to object being an instance without an _obj_ attribute!\n");
			return FALSE;
		}
		if (obattr==Py_None) {
			TRACE("is_uiobject fails due to object being an instance with _obj_==None\n");
			return FALSE;
		}
		o = obattr;
		ob = (ui_base_class *)o;
	}
	if (memcmp(ob->sig, SIG, sizeof(SIG))) {
		TRACE("is_uiobject fails due to sig failure!\n");
		return FALSE;
	}
	return is_nativeuiobject(ob, which);
}

/*static*/BOOL ui_base_class::is_nativeuiobject(PyObject *ob, ui_type *which)
{
	// check for inheritance.
	ui_type *thisType = (ui_type *)ob->ob_type;
	while (thisType) {
		if (which==thisType)
			return TRUE;
		thisType = thisType->base;
	}
	return FALSE;
}
BOOL ui_base_class::is_uiobject(ui_type *which)
{
	PyObject *cpy = this;
	BOOL ret = is_uiobject(cpy,which);
#ifdef _DEBUG
	return ret && (cpy==this);
#endif
	return ret;
}

PyObject *
ui_base_class::sui_getattr(PyObject *self, char *name)
{
	return ((ui_base_class *)self)->getattr(name);
}

PyObject *
ui_base_class::getattr(char *name)
{
	// implement inheritance.
	PyObject *retMethod = NULL;
	ui_type *thisType = (ui_type *)ob_type;
	while (thisType) {
		retMethod = Py_FindMethod(thisType->methods, (PyObject *)this, name);
		if (retMethod)
			break;
		thisType = thisType->base;
		if (thisType)
			PyErr_Clear();
	}
	return retMethod;
}
int
ui_base_class::sui_setattr(PyObject *op, char *name, PyObject *v)
{
	ui_base_class* bc = (ui_base_class *)op;
	return bc->setattr(name,v);
}
int ui_base_class::setattr(char *name, PyObject *v)
{
	char buf[128];
	sprintf(buf, "%s has read-only attributes", ob_type->tp_name );
	PyErr_SetString(PyExc_TypeError, buf);
	return -1;
}
/*static*/ PyObject *
ui_base_class::sui_repr( PyObject *op )
{
	ui_base_class* w = (ui_base_class *)op;
	CString ret = w->repr();
	return Py_BuildValue("s",(const char *)ret);
}
CString ui_base_class::repr()
{
	CString csRet;
	char *buf = csRet.GetBuffer(50);
	sprintf(buf, "object '%s'", ob_type->tp_name);
	csRet.ReleaseBuffer();
	return csRet;
}
void ui_base_class::cleanup()
{
	const char *szTyp = ob_type ? ob_type->tp_name : "<bad type!>";
	TRACE("cleanup detected type %s, refcount = %d\n",szTyp,ob_refcnt);
}

/*static*/ void ui_base_class::sui_dealloc(PyObject *window)
{
	delete (ui_base_class *)window;
}

// @pymethod |PyAssocObject|GetMethodByType|Given a method name and a type object, return the attribute.
static PyObject *
ui_base_class_GetMethodByType(PyObject *self, PyObject *args)
{
	// @comm This function allows you to obtain attributes for base types.
	// For example, calling appObject.GetAttributeByType("Run", threadType) will return
	// the built-in Run method for the CWinThread object rather than the CWinApp object.
	PyObject *obType;
	char *attr;
	ui_base_class *pAssoc = (ui_base_class *)self;
	if (pAssoc==NULL) return NULL;
	if (!PyArg_ParseTuple(args, "sO:GetAttributeByType", &attr, &obType ))
		return NULL;

	// check it is one of ours.
	PyObject *retMethod = NULL;
	ui_type *thisType = (ui_type *)pAssoc->ob_type;
	while (thisType) {
		if ((PyObject *)thisType==obType)
			break;
		thisType = thisType->base;
	}
	if (thisType==NULL)
		RETURN_TYPE_ERR("The object is not of that type");

	return Py_FindMethod(thisType->methods, self, attr);
}


struct PyMethodDef ui_base_class::empty_methods[] = {
	{NULL,	NULL}
};

struct PyMethodDef ui_base_class_methods[] = {
	{"GetMethodByType", ui_base_class_GetMethodByType, 1},
	{NULL,	NULL}
};

ui_type ui_base_class::type( "PyCBase", 
							NULL, 
							sizeof(ui_base_class), 
							ui_base_class_methods, 
							NULL);


#ifdef _DEBUG
void DumpAssocPyObject( CDumpContext &dc , void *object )
{
	ui_assoc_object *py_bob = ui_assoc_object::handleMgr.GetAssocObject( object );
	if (py_bob==NULL)
		dc << ", have no attached Python object";
	else {
#if !defined(_MAC) && !defined(_AFX_PORTABLE)
	try
#endif
	{
		dc << ", Python object ";
		if (AfxIsValidAddress(py_bob, sizeof(ui_assoc_object))) {
			dc << py_bob << " with refcounf " << 
			py_bob->ob_refcnt;
		} else
			dc  << "<at invalid address!>";
	}
#if !defined(_MAC) && !defined(_AFX_PORTABLE)
		catch(int code) {
			// short form for trashed objects
			afxDump << "<Bad! (" << code << ")>";
		}
		catch(...) {
			// short form for trashed objects
			afxDump << "<Bad!>";
		}
#endif
	}
}

void ui_base_class::Dump( CDumpContext &dc ) const
{
	CObject::Dump(dc);
	dc << "Object of type " << ob_type->tp_name << ", ob_refcnt=" << ob_refcnt;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// Helpers for the application.  Avoid pulling python headers everywhere.
//
/////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllimport) void	PySys_SetPath(char *);
void PYW_EXPORT Python_addpath( const char *paths )
{
	char workBuf[MAX_PATH+20];
	char fullThisPath[MAX_PATH+20];
	char fullWorkBuf[MAX_PATH+20];
	
	PyObject *p = PySys_GetObject("path");
	if (!PyList_Check(p))
		return;

	int posFirst = 0;
	int posLast = 0;
	while (paths[posLast]) {
		// skip all ';'
		while (paths[posFirst]==';')
			posFirst++;
		posLast = posFirst;
		while (paths[posLast]!='\0' && paths[posLast]!=';')
			posLast++;
		int len = min(sizeof(workBuf)-1,posLast - posFirst);
		if (len>0) {
			strncpy(workBuf, paths+posFirst, len );
			workBuf[len]='\0';
			// Check if it is already on the path...
			if (!GetFullPath(fullWorkBuf, workBuf)) // not a valid path
				continue;	// ignore it.
			int listLen = PyList_Size(p);
			int itemNo;
			for (itemNo=0;itemNo<listLen;itemNo++) {
				char *thisPath = PyString_AsString(PyList_GetItem(p, itemNo));
				if (thisPath==NULL) return; // Serious error!!!
				if (GetFullPath(fullThisPath, thisPath) && strcmpi(fullThisPath, fullWorkBuf)==0) {
					// is there!
					break;
				}
			}
			if (itemNo>=listLen) { // not in list
				// Need to add it.
				PyObject *add = PyString_FromString(fullWorkBuf);
				if (add) {
					PyList_Insert(p, 0, add);
					Py_DECREF(add);
				}
			}
		}
		posFirst = posLast;
	}
}

#define GPEM_ERROR(what) {errorMsg = "<Error getting traceback - " ## what ## ">";goto done;}
static char *GetPythonTraceback(PyObject *exc_tb)
{
	char *result = NULL;
	char *errorMsg = NULL;
	PyObject *modStringIO = NULL;
	PyObject *modTB = NULL;
	PyObject *obFuncStringIO = NULL;
	PyObject *obStringIO = NULL;
	PyObject *obFuncTB = NULL;
	PyObject *argsTB = NULL;
	PyObject *obResult = NULL;

	/* Import the modules we need - cStringIO and traceback */
	modStringIO = PyImport_ImportModule("cStringIO");
	if (modStringIO==NULL) GPEM_ERROR("cant import cStringIO");
	modTB = PyImport_ImportModule("traceback");
	if (modTB==NULL) GPEM_ERROR("cant import traceback");

	/* Construct a cStringIO object */
	obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find cStringIO.StringIO");
	obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
	if (obStringIO==NULL) GPEM_ERROR("cStringIO.StringIO() failed");

	/* Get the traceback.print_exception function, and call it. */
	obFuncTB = PyObject_GetAttrString(modTB, "print_tb");
	if (obFuncTB==NULL) GPEM_ERROR("cant find traceback.print_tb");
	argsTB = Py_BuildValue("OOO", 
			exc_tb  ? exc_tb  : Py_None,
			Py_None, 
			obStringIO);
	if (argsTB==NULL) GPEM_ERROR("cant make print_tb arguments");

	obResult = PyObject_CallObject(obFuncTB, argsTB);
	if (obResult==NULL) GPEM_ERROR("traceback.print_tb() failed");

	/* Now call the getvalue() method in the StringIO instance */
	Py_DECREF(obFuncStringIO);
	obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find getvalue function");
	Py_DECREF(obResult);
	obResult = PyObject_CallObject(obFuncStringIO, NULL);
	if (obResult==NULL) GPEM_ERROR("getvalue() failed.");

	/* And it should be a string all ready to go - duplicate it. */
	if (!PyString_Check(obResult))
		GPEM_ERROR("getvalue() did not return a string");
	result = strdup(PyString_AsString(obResult));
done:
	if (result==NULL && errorMsg != NULL)
		result = strdup(errorMsg);
	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obFuncStringIO);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obFuncTB);
	Py_XDECREF(argsTB);
	Py_XDECREF(obResult);
	return result;
}

BOOL DisplayPythonTraceback(PyObject *exc_type, PyObject *exc_val, PyObject *exc_tb, const char *extraTitleMsg = NULL)
{
	class CTracebackDialog : public CDialog {
	public:
		CTracebackDialog(PyObject *exc_type, PyObject *exc_value, PyObject *exc_tb, const char *extraTitleMsg) : 
		  CDialog(IDD_LARGE_EDIT)
		{
			m_exc_type = exc_type;
			Py_XINCREF(exc_type);
			m_exc_value = exc_value;
			Py_XINCREF(exc_value);
			m_exc_tb = exc_tb;
			Py_XINCREF(exc_tb);
			m_extraTitleMsg = extraTitleMsg;
		}
		~CTracebackDialog()
		{
			Py_XDECREF(m_exc_tb);
			Py_XDECREF(m_exc_type);
			Py_XDECREF(m_exc_value);
		}
		BOOL OnInitDialog() {
			CDialog::OnInitDialog();
			CEnterLeavePython _celp;
			CString title("Python Traceback");
			if (m_extraTitleMsg)
				title = title + m_extraTitleMsg;

			SetWindowText(title);
			GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDOK)->SetWindowText("Close");
			char *msg = GetPythonTraceback(m_exc_tb);
			char *msg_free = msg;
			// Translate '\n' to '\r\n' - do it the easy way!
			CString useMsg;
			for (;*msg;msg++)
				if (*msg=='\n') 
					useMsg += "\r\n";
				else
					useMsg += *msg;
			free(msg_free);
			PyObject *obStrType = PyObject_Str(m_exc_type);
			char *szType = PyString_AsString(obStrType);
			useMsg += szType;
			useMsg += ": ";

			PyObject *obStrVal = PyObject_Str(m_exc_value);
			char *szVal = PyString_AsString(obStrVal);
			useMsg+=szVal;
#ifdef _DEBUG
			{
			// doesnt seem to like long strings.
			CString cs(useMsg);
			int i = 0;
			while (i<cs.GetLength()) {
				OutputDebugString(cs.Mid(i, 256));
				i = i + 256;
			}
			}
#endif
			GetDlgItem(IDC_EDIT1)->SetWindowText(useMsg);
			GetDlgItem(IDC_EDIT1)->SetFocus();
			return FALSE;
		};
		PyObject *m_exc_tb, *m_exc_type, *m_exc_value;
		const char *m_extraTitleMsg;
	};
	CTracebackDialog dlg(exc_type, exc_val, exc_tb, extraTitleMsg);
	GUI_BGN_SAVE;
	dlg.DoModal();
	GUI_END_SAVE;
	return TRUE;
}

int Python_run_command_with_log(const char *command, const char * logFileName = NULL)
{
	_ASSERTE(logFileName==NULL); // The logFileName param is no longer used!
	PyObject *m, *d, *v;
	m = PyImport_AddModule("__main__");
	if (m == NULL)
		return -1;
	d = PyModule_GetDict(m);
	v = PyRun_String((char *)command, file_input, d, d);
	if (v == NULL) {
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);
		DisplayPythonTraceback(type, value, traceback);
		PyErr_Restore(type, value, traceback);
/*******
		PyObject *fo = PyFile_FromString((char *)logFileName, "w" );
		if (fo==NULL)
			return -1;
        PyObject *old = PySys_GetObject( "stderr" );
		if (old==NULL)
			return -1;
		Py_INCREF(old);
		PySys_SetObject( "stderr", fo );
		PyErr_Print();
		PySys_SetObject( "stderr", old );
		Py_DECREF(old);
		Py_XDECREF(fo);
		return 1;	// indicate failure, with valid log.
*******/
		return 1;	// indicate failure, with traceback correctly shown.
	}
	DODECREF(v);
	return 0;
}

// The "Official" way to destroy an associated (ie, MFC) object.
// The object will be destroyed if appropriate.
// Requires the Python thread state be NOT acquired.
void Python_delete_assoc( void *ob )
{
	// Notify Python object of my attached object removal.
	{
	CVirtualHelper helper ("OnAttachedObjectDeath", ob);
	helper.call();
	}
	ui_assoc_object *pObj;
	if ((pObj=ui_assoc_object::GetPyObject(ob)) && !bInFatalShutdown) {
		CEnterLeavePython _celp; // KillAssoc requires it is held!
		pObj->KillAssoc();
	}
}

void Python_set_error(const char *msg)
{
}
// In DEBUG builds, access voilations will normally trip my debugger, and
// hence I dont want them trapped.  Stack Overflows normally mean runaway Python
// code, and I dont really want these trapped.
#ifdef _DEBUG
static int bTrapAccessViolations = FALSE;
#endif

// exception handler.
static DWORD FilterFunc (DWORD dwExceptionCode) {

	// Assume that we do not know how to handle the exception
	// by telling the system to continue to search for an SEH
	// handler.
	DWORD dwRet = EXCEPTION_CONTINUE_SEARCH;
	switch (dwExceptionCode) {
		case STATUS_STACK_OVERFLOW:
			OutputDebugString("win32ui has stack overflow!\n");
			PyErr_SetString(PyExc_SystemError,"Stack Overflow");
			dwRet = EXCEPTION_EXECUTE_HANDLER;
			break;
		case EXCEPTION_ACCESS_VIOLATION:
			OutputDebugString("win32ui has access vln!\n");
#ifdef _DEBUG
			if (!bTrapAccessViolations)
				return dwRet;
#endif // _DEBUG
			PyErr_SetString(PyExc_SystemError,"Access Violation");
			dwRet = EXCEPTION_EXECUTE_HANDLER;
			break;
		default:
			break;
	}
	return(dwRet);
}

PyObject *gui_call_object(PyObject *themeth, PyObject *thearglst)
{
	return PyEval_CallObject(themeth,thearglst);
}

void gui_print_error(void)
{
	// basic recursion control.
	static BOOL bInError = FALSE;
	if (bInError) return;
	bInError=TRUE;

	// Check if the exception is SystemExit - if so,
	// PyErr_Print will terminate then and there!  This is
	// not good (and not what we want!?
	PyObject *exception, *v, *tb;
	PyErr_Fetch(&exception, &v, &tb);
	PyErr_NormalizeException(&exception, &v, &tb);

	if (exception  && PyErr_GivenExceptionMatches(exception, PyExc_SystemExit)) {
		// Replace it with a RuntimeError.
		TRACE("WARNING!!  win32ui had a SystemError - Replacing with RuntimeError!!\n");
		Py_DECREF(exception);
		Py_XINCREF(PyExc_RuntimeError);
		PyErr_Restore(PyExc_RuntimeError, v, tb);
	} else
		PyErr_Restore(exception, v, tb);
	// Now print it.

	PyErr_Print();
	bInError=FALSE;
}

// A Python program can install a callback notifier, to make all
// callbacks!
static PyObject *pCallbackCaller = NULL;
PyObject *Python_do_callback(PyObject *themeth, PyObject *thearglst)
{
	PyObject *result;
	if (pCallbackCaller) {
		PyObject *newarglst = Py_BuildValue("(OO)",themeth,thearglst);
		result = gui_call_object( pCallbackCaller, newarglst );
		DODECREF(newarglst);
	} else
		result = gui_call_object( themeth, thearglst );
	DODECREF(thearglst);
	if (result==NULL) {
		TRACE("Python_do_callback: callback failed with exception\n");
		gui_print_error();
	}
	return result;
}

int Python_do_int_callback(PyObject *themeth, PyObject *thearglst)
{
	int retVal=UINT_MAX;	// an identifiable, but unlikely genuine value.
	BOOL isError = FALSE;
	PyObject *result = Python_do_callback(themeth, thearglst);
	if (result==NULL)
		return retVal;
	if (result==Py_None)	// allow for None==0
		retVal = 0;
	else if (result != Py_None && (!PyArg_Parse(result,"i",&retVal))) {
		TRACE("Python_do_int_callback: callback had bad return type\n");
		PyErr_SetString(ui_module_error, "Callback must return an integer, or None");
		gui_print_error();
	}
#ifdef _DEBUG_HEAP	// perform some diagnostics.  May help trap reference errors.
	if (_heapchk()!=_HEAPOK)
		TRACE("**** Warning-heap corrupt after application callback ****\n");
#endif
	DODECREF(result);
	return retVal;
}
int Python_callback(PyObject *method, int val)
{
	PyObject *meth = method;
	PyObject *thearglst = Py_BuildValue("(i)",val);
	return Python_do_int_callback(meth,thearglst);
}
int Python_callback(PyObject *method, int val1, int val2)
{
	PyObject *meth = method;
	PyObject *arglst = Py_BuildValue("(ii)",val1,val2);
	return Python_do_int_callback(meth,arglst);
}

int Python_callback(PyObject *method)
{
	PyObject *meth = method;
	PyObject *arglst = Py_BuildValue("()");
	return Python_do_int_callback(meth,arglst);
}
int Python_callback(PyObject *method, const MSG *msg)
{
	PyObject *meth = method;
	PyObject *arglst = Py_BuildValue("((iiiii(ii)))",msg->hwnd,msg->message,msg->wParam,msg->lParam,msg->time,msg->pt.x,msg->pt.y);
	return Python_do_int_callback(meth,arglst);
}
int Python_callback(PyObject *method, PyObject *object)
{
	PyObject *meth = method;
	PyObject *arglst = Py_BuildValue("(O)", object);
	return Python_do_int_callback(meth,arglst);
}


/////////////////////////////////////////////////////////////////////
//
// Helpers for the methods.
//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
// Python Functions etc.
//
/////////////////////////////////////////////////////////////////////
// @pymethod |win32ui|PrintTraceback|Prints a traceback using the internal Python mechanism.
static PyObject *
ui_python_print_traceback( PyObject *self, PyObject *args )
{
	PyObject *tb, *output;
	// @pyparm object|tb||The traceback to print.
	// @pyparm object|output||The object to write the traceback to.
	if (!PyArg_ParseTuple(args, "OO:PrintTraceback", &tb, &output))
		return NULL;
	PyTraceBack_Print(tb,output);
	RETURN_NONE;
}

// @pymethod |win32ui|OutputDebugString|Sends a string to the Windows debugging device.
static PyObject *
ui_output_debug(PyObject *self, PyObject *args)
{
	char *msg;
	// @pyparm string|msg||The string to write.
	if (!PyArg_ParseTuple(args, "s:OutputDebugString", &msg))
		return NULL;
	GUI_BGN_SAVE;
#ifdef BULLSHIT_BUG
	CString csuiod;
	char *uiod_base = csuiod.GetBuffer(strlen(msg));
	char *uiod = uiod_base;

	while (*msg)
	{
		// not sure what's going on here.  NT seems to add a \n each call..
		// Im sure msvc16 doesnt...(well, I _think_ Im sure..:)
		while (*msg && *msg!='\n')
			*uiod++ = *msg++;
		*uiod='\0';	// replace with NULL;
		if (*msg) {	// must be \n
			uiod=uiod_base;
			OutputDebugString(uiod) ;
			++msg;
		}
	}
#else
	OutputDebugString(msg);
#endif
	GUI_END_SAVE;
	RETURN_NONE;
}

/////////////////////////////////////////////////////////////////////
//
// Python Methods etc.
//
/////////////////////////////////////////////////////////////////////
// @pymethod <o PyCMDIFrameWnd>|win32ui|CreateMDIFrame|Creates an MDI Frame window.
static PyObject *
ui_create_mdi_frame(PyObject *self, PyObject *args)
{
	// @comm An MDI Frame Window is usually the main application window.
	// Therefore there is uaually only one of these windows per application.
	CHECK_NO_ARGS2(args,CreateMDIFrame);
	CWinApp *pApp = GetApp();
	if (pApp==NULL) return NULL;
	GUI_BGN_SAVE;
	CPythonMDIFrameWnd* pMainFrame = new CPythonMDIFrameWnd;
	GUI_END_SAVE;
	return ui_assoc_object::make(PyCMDIFrameWnd::type, pMainFrame)->GetGoodRet();
	// @rdesc The window object created.  An exception is raised if an error occurs.
	// @comm An application can only hae one main window.  This method will fail if the application
	// window already exists.
}

// @pymethod <o PyCMDIChildWnd>|win32ui|CreateMDIChild|Creates an MDI Child window.
static PyObject *
ui_create_mdi_child(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,CreateMDIChild);
	GUI_BGN_SAVE;
	CPythonMDIChildWnd* pFrame = new CPythonMDIChildWnd;
	GUI_END_SAVE;
	return ui_assoc_object::make(PyCMDIChildWnd::type, pFrame)->GetGoodRet();
	// @rdesc The window object created.  An exception is raised if an error occurs.
}

// @pymethod int|win32ui|Enable3dControls|Enables 3d controls for the application.
static PyObject *
ui_enable_3d_controls(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,Enable3dControls);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	GUI_BGN_SAVE;
	int rc = pApp->Enable3dControls();
	GUI_END_SAVE;

	return Py_BuildValue("i",rc);
	// @rdesc True if 3d controls could be enabled, false otherwise.
}

// @pymethod string|win32ui|GetCommandLine|Returns the application's command line.
static PyObject *
ui_get_command_line (PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetCommandLine);
	return Py_BuildValue("s", ::GetCommandLine()); // @pyseeapi GetCommandLine
}

// @pymethod int|win32ui|GetInitialStateRequest|Returns the requested state that the application start in.  This is the same as the paramaters available to <om PyCWnd.ShowWindow>
static PyObject *
ui_get_initial_state_request(PyObject *self, PyObject *args)
{
	// @comm In some cases, it may not be possible to start in the requested mode.  An application
	// may start in its default mode, then set its mode to match the value returned from this method.
	CHECK_NO_ARGS2(args,GetInitialStateRequest);
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	return Py_BuildValue("i", pApp->m_nCmdShow );
}
// @pymethod string|win32ui|GetName|Returns the name of the current executable.
static PyObject *
ui_get_name(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetName);
	// MFC AppName gives title, ExeName gives module name!
	char fileName[MAX_PATH+1];

	GetModuleFileName( GetModuleHandle(NULL), fileName, sizeof(fileName));
	return Py_BuildValue("s", fileName );
}

// @pymethod tuple|win32ui|GetRect|Returns the rectangle of the main application frame.  See <om PyCWnd.GetWindowRecr> for further details.
static PyObject *
ui_get_rect(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetRect);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	CWnd *pFrame = pApp->GetMainFrame();
	if (!pFrame)
		RETURN_ERR("The frame does not exist");

	CRect rect;
	GUI_BGN_SAVE;
	pFrame->GetWindowRect( &rect );
	GUI_END_SAVE;
	return Py_BuildValue("(iiii)",rect.left, rect.top, rect.right, rect.bottom);
	// @rdesc A tuple of integers with (left, top, right, bottom)
}
// @pymethod |win32ui|WriteProfileVal|Writes a value to the application's INI file.
static PyObject *
ui_write_profile_val(PyObject *self, PyObject *args)
{
	char *sect, *entry, *strVal;
	int intVal;
	// @pyparm string|section||The section in the INI file to write to.
	// @pyparm string|entry||The entry within the section in the INI file to write to.
	// @pyparm int/string|value||The value to write. The type of this parameter determines the method's return type.
	BOOL bHaveInt = TRUE;
	if (!PyArg_ParseTuple(args, "ssi:WriteProfileVal", &sect, &entry, &intVal)) {
		bHaveInt = FALSE;
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "ssz", &sect, &entry, &strVal)) {
			// set my own error
			PyErr_Clear();
			RETURN_TYPE_ERR("WriteProfileVal must have format (ssi) or (ssz)");
		}
	}
	BOOL rc;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;

	if (bHaveInt) {
//		TRACE("Write profile value (int)[%s] - %s=%d\n",sect,entry,intVal);
		GUI_BGN_SAVE;
		rc = pApp->WriteProfileInt( sect, entry, intVal );
		GUI_END_SAVE;
	}
	else {
//		TRACE("Write profile value (str)[%s] - %s=%s\n",sect,entry,strVal?strVal:"<NULL>");
		GUI_BGN_SAVE;
		rc = pApp->WriteProfileString( sect, entry, strVal );
		GUI_END_SAVE;
	}
	if (!rc)
		RETURN_ERR("WriteProfileInt/String failed");
	return Py_BuildValue("i",rc);
}
// @pymethod int/string|win32ui|GetProfileVal|Returns a value from the application's INI file.
static PyObject *
ui_get_profile_val(PyObject *self, PyObject *args)
{
	char *sect, *entry, *strDef;
	int intDef;
	BOOL bHaveInt = TRUE;
	// @pyparm string|section||The section in the INI file to read from.
	// @pyparm string|entry||The entry within the section in the INI file to read.
	// @pyparm int/string|defValue||The default value.  The type of this parameter determines the method's return type.
	if (!PyArg_ParseTuple(args, "ssi", &sect, &entry, &intDef)) {
		bHaveInt = FALSE;
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "sss:GetProfileVal", &sect, &entry, &strDef)) {
			// set my own error
			PyErr_Clear();
			RETURN_TYPE_ERR("GetProfileVal must have format (ssi) or (sss)");
		}
	}
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	if (bHaveInt) {
		GUI_BGN_SAVE;
		PyObject *rc = Py_BuildValue("i",pApp->GetProfileInt(sect, entry, intDef ));
		GUI_END_SAVE;
		return rc;
	}
	else {
		GUI_BGN_SAVE;
		CString res = pApp->GetProfileString(sect, entry, strDef );
		GUI_END_SAVE;
		return Py_BuildValue("s",(const char *)res);
	}
}
// @pymethod |win32ui|SetProfileFilename|Sets the name of the INI file used by the application.
static PyObject *
ui_set_profile_filename(PyObject *self, PyObject *args)
{
	char *filename;
	// @pyparm string|filename||The name of the ini file.
	if (!PyArg_ParseTuple(args, "s:SetProfileFilename", &filename))
		return NULL;
	// this is a memory leak!
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;

	char *newBuf = strdup(filename);
	pApp->m_pszProfileName = newBuf;
	RETURN_NONE;
}
// @pymethod string|win32ui|GetProfileFileName|Returns the name of the INI file used by the application.
static PyObject *
ui_get_profile_filename(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetProfileFileName);
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
	return Py_BuildValue("s", pApp->m_pszProfileName);
}
// @pymethod |win32ui|LoadStdProfileSettings|Loads MFC standard settings from the applications INI file.  This includes the Recent File List, etc.
static PyObject *
ui_load_std_profile_settings(PyObject *self, PyObject *args)
{
	int maxFiles = _AFX_MRU_COUNT;
	// @pyparm int|maxFiles|_AFX_MRU_COUNT|The maximum number of files to maintain on the Recently Used File list.
	if (!PyArg_ParseTuple(args, "|i:LoadStdProfileSettings", &maxFiles))
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	// @comm This function can only be called once in an applications lifetime, else an exception is raised.
	if (pApp->HaveLoadStdProfileSettings())
		RETURN_ERR("The profile settings have already been loaded.");
	GUI_BGN_SAVE;
	pApp->LoadStdProfileSettings(maxFiles);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |win32ui|SetStatusText|Sets the text in the status bar of the application.
static PyObject *
ui_set_status_text(PyObject *self, PyObject *args)
{
	char *msg;
	BOOL bForce = FALSE;
	// @pyparm string|msg||The message to write to the status bar.
	// @pyparm int|bForce|0|A flag indicating if the message should be forced to the status bar, or written in idle time.
	if (!PyArg_ParseTuple(args,"s|i:SetStatusText",&msg, &bForce))
		return NULL;

	// If the glue wants it, the glue can have it :-)
	if (pHostGlue && pHostGlue->bWantStatusBarText) {
		pHostGlue->SetStatusText(msg, bForce);
		RETURN_NONE;
	}
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	CWnd *pWnd = pApp->GetMainFrame();
	if (pWnd==NULL)
		RETURN_ERR("There is no main window");

	CWnd *pStatusBar = pWnd->GetDlgItem(AFX_IDW_STATUS_BAR);
	// Check for NULL or invalid handle. (GetSafe.. does the NULL!)
	if (!::IsWindow(pStatusBar->GetSafeHwnd()))
			RETURN_ERR("The status bar window is invalid");

	GUI_BGN_SAVE;
	pStatusBar->SetWindowText(msg);
	GUI_END_SAVE;
	if (bForce) {
		pStatusBar->InvalidateRect(NULL);
		pStatusBar->UpdateWindow();
	}
	RETURN_NONE;
}

// @pymethod list|win32ui|GetRecentFileList|Returns the entries in the applications Recent File List.
static PyObject *
ui_get_recent_file_list(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetRecentFileList);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	int cnt = pApp->GetProfileInt("Settings", "Recent File List Size", _AFX_MRU_COUNT);
	PyObject *list = PyList_New(cnt);
	if (list==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating list for MRU documents");
		return NULL;
	}
	for (int i=0;i<cnt;i++) {
		CString csFile(pApp->GetRecentFileName(i));
		// hack to non-const for Python
		char *name = (char *)(const char *)csFile;
		PyList_SetItem(list,i,PyString_FromString(name));
	}
	return list;
	// @rdesc A list of strings containing the fully qualified file names.
}
// @pymethod |win32ui|AddToRecentFileList|Adds an entry to the applications Recent File List.
static PyObject *
ui_add_to_recent_file_list(PyObject *self, PyObject *args)
{
	// @pyparm string|fileName||The file name to be added to the list.
	char *msg;
	if (!PyArg_ParseTuple(args,"s:AddToRecentFileList",&msg))
		return NULL;
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
    pApp->AddToRecentFileList(msg); // @pyseemfc CWinApp|AddToRecentFileList
	RETURN_NONE;
}

// @pymethod |win32ui|RemoveRecentFile|Removes the entry in the applications Recent File List at index.
static PyObject *
ui_remove_recent_file(PyObject *self, PyObject *args)
{
	int index = 0;
	// @pyparm int|index|0|Zero-based index of the file to be removed from the MRU (most recently used) file list.
	if (!PyArg_ParseTuple(args,"i:RemoveRecentFile",&index))
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	pApp->RemoveRecentFile(index);
	RETURN_NONE;
}

// @pymethod <o PyCWnd>|win32ui|GetMainFrame|Returns a window object for the main application frame.
static PyObject *
ui_get_main_frame(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetMainFrame);
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	CWnd *pFrame = pApp->GetMainFrame();
	if (!pFrame)
		RETURN_ERR("The frame does not exist");
	// Do some RTTI on the object.
	ui_type &makeType = UITypeFromCObject(pFrame);
	return ui_assoc_object::make(makeType, pFrame)->GetGoodRet();
}

// @pymethod |win32ui|StartDebuggerPump|Starts a recursive message loop, waiting for an application close message.
int bIsPumping = FALSE;
BOOL bDebuggerPumpStopRequested = FALSE;
static PyObject *
ui_start_debugger_pump(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,StartDebuggerPump);
	if (bIsPumping)
		RETURN_ERR("Error starting debugger pumper - already pumping");
	bIsPumping = TRUE;
	bDebuggerPumpStopRequested = FALSE;
	CProtectedWinThread *pThread = GetProtectedThread();
	if (!pThread) return NULL;
	GUI_BGN_SAVE;
	pThread->PumpMessages();
	if (!bDebuggerPumpStopRequested) // App shutdown request.
		PostQuitMessage(0);
	GUI_END_SAVE;
	RETURN_NONE;
	// @comm This function is used by the debugger.  It allows the debugger to
	// interact with the user, even while the Python code is stopped.
	// As the Python code may be responding to a Windows Event, this function
	// works around the inherent message queue problems.
}
// @pymethod |win32ui|StopDebuggerPump|Stops the debugger pump.  See <om win32ui.StartDebuggerPump>.
static PyObject *
ui_stop_debugger_pump(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,StopDebuggerPump);
	if (!bIsPumping)
		RETURN_ERR("Error stopping debugger pumper - pump not started");
	bIsPumping = FALSE;
	GUI_BGN_SAVE;
	bDebuggerPumpStopRequested = TRUE; // Set this BEFORE QuitMessage
	PostQuitMessage(0);
	GUI_END_SAVE;
	RETURN_NONE;
}
// @pymethod int|win32ui|PumpWaitingMessages|Recursively start a new message dispatching loop while any message remain in the queue.
static PyObject *
ui_pump_waiting_messages(PyObject *self, PyObject *args)
{
	// @pyparm int|firstMessage|WM_PAINT|The lowest message ID to retrieve
	// @pyparm int|lastMessage|WM_PAINT|The highest message ID to retrieve
	UINT firstMsg = WM_PAINT, lastMsg = WM_PAINT;
	if (!PyArg_ParseTuple (args, "|ii:PumpWaitingMessages", &firstMsg, &lastMsg))
		return NULL;
	CProtectedWinThread *pThread = GetProtectedThread();
	if (!pThread) return NULL;
	GUI_BGN_SAVE;
	bool rc = pThread->PumpWaitingMessages(firstMsg, lastMsg);
	GUI_END_SAVE;
	return PyInt_FromLong((int)rc==true);
	// @comm This allows an application which is performing a long operation to dispatch paint messages during the operation.
	// @rdesc The result is 1 if a WM_QUIT message was processed, otherwise 0.
}
// @pymethod |win32ui|CreateDebuggerThread|Starts a debugging thread (ie, creates the "break" button).
static PyObject *
ui_create_debugger_thread(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,CreateDebuggerThread);
	DWORD tid;
	DWORD param = 0;
	::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DebuggerThreadFunc, &param, 0, &tid );
	RETURN_NONE;
	// @comm This allows an application which is performing a long operation to dispatch paint messages during the operation.
}

// @pymethod |win32ui|DestroyDebuggerThread|Cleans up the debugger thread.  See <om win32ui.CreateDebuggerThread>.
static PyObject *
ui_destroy_debugger_thread(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,DestroyDebuggerThread);
	extern void StopDebuggerThread(void);
	GUI_BGN_SAVE;
	StopDebuggerThread();
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod int|win32ui|MessageBox|Display a message box.
static PyObject *
ui_message_box(PyObject * self, PyObject * args)
{
  char *message;
  long style = MB_OK;
  const char *title = NULL;
  // @pyparm string|message||The message to be displayed in the message box.
  // @pyparm string/None|title|None|The title for the message box.  If None, the applications title will be used.
  // @pyparm int|style|win32con.MB_OK|The style of the message box.
  if (!PyArg_ParseTuple(args, "s|zl:MessageBox", &message, &title, &style))
    return NULL;
  CWinApp *pApp = GetApp();
  if (pApp==NULL) return NULL;

  if (title==NULL)
  	title = pApp->m_pszAppName;
  int rc;
  GUI_BGN_SAVE;
  rc = ::MessageBox(pApp->m_pMainWnd->GetSafeHwnd(), message, title, style);
  GUI_END_SAVE;
  return Py_BuildValue("i",rc);
  // @rdesc An integer identifying the button pressed to dismiss the dialog.
}

// @pymethod string|win32ui|FullPath|Return the fully qualified path of a file name.
static PyObject *
ui_full_path(PyObject * self, PyObject * args)
{
	char *path;

	// @pyparm string|path||The path name.
	if (!PyArg_ParseTuple(args, "s:FullPath", &path))
		return NULL;
	char szOutPath[_MAX_PATH];
	if (!GetFullPath(szOutPath, path))
		RETURN_ERR("The file name is invalid");
	return Py_BuildValue("s", szOutPath);
}

// @pymethod int|win32ui|ComparePath|Compares 2 paths.
static PyObject *
ui_compare_path(PyObject * self, PyObject * args)
{
	BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);
	char *path1, *path2;
	// @pyparm string|path1||The path name.
	// @pyparm string|path2||The path name.
	if (!PyArg_ParseTuple(args, "ss:ComparePath", &path1, &path2))
		return NULL;
	return Py_BuildValue("i", AfxComparePath(path1, path2));
}

// @pymethod string|win32ui|GetFileTitle|Given a file name, return its title
static PyObject *
ui_get_file_title(PyObject * self, PyObject * args)
{
	UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
	char *fname;
	// @pyparm string|fileName||The file name.
	if (!PyArg_ParseTuple(args, "s:GetFileTitle", &fname))
		return NULL;
	char buf[_MAX_FNAME+1];
	if (AfxGetFileTitle(fname, buf, sizeof(buf))!=0)
		RETURN_ERR("AfxGetFileTitle failed");
	return PyString_FromString(buf);
}


// @pymethod |win32ui|DoWaitCursor|Dispay a wait cursor.
static PyObject *
ui_do_wait_cursor(PyObject * self, PyObject * args)
{
  int code;
  // @pyparm int|code||If this parameter is 0, the original cursor is restored. If 1, a wait cursor appears. If -1, the wait cursor ends.
  if (!PyArg_ParseTuple(args, "i:DoWaitCursor", &code))
    return NULL;
  CWinApp *pApp = GetApp();
  if (!pApp) return NULL;

  GUI_BGN_SAVE;
  pApp->DoWaitCursor(code);
  GUI_END_SAVE;
  RETURN_NONE;
}

// @pymethod object|win32ui|InstallCallBackCaller|Install a Python method which will dispatch all callbacks into Python.
static PyObject *
ui_install_callback_caller(PyObject *self, PyObject *args)
{
	PyObject *caller = NULL;
	if (!PyArg_ParseTuple(args,"|O:InstallCallBackCaller",&caller))
		return NULL;
	PyObject *retval = pCallbackCaller;
	if (caller==Py_None)
		caller = NULL;
	Py_XDECREF(pCallbackCaller);
	if (caller) {
		if (!PyCallable_Check(caller))
			RETURN_ERR("Argument must be a callable object");
		pCallbackCaller = caller;
		Py_INCREF(caller);
	} else
		pCallbackCaller = NULL;
	if (retval)
		return Py_BuildValue("O", retval);
	else
		RETURN_NONE;
	// @rdesc The previous callback caller.
}

// @pymethod int|win32ui|IsWin32s|Determines if the application is running under Win32s.
static PyObject *
ui_is_win32s(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,IsWin32s);
	return Py_BuildValue("i", IsWin32s());
}
// @pymethod int|win32ui|IsObject|Determines if the passed object is a win32ui object.
static PyObject *
ui_is_object(PyObject *self, PyObject *args)
{
  PyObject *obj;
  // @pyparm object|o||The object to check.
  if (!PyArg_ParseTuple(args, "O:IsObject", &obj))
    return NULL;
  return Py_BuildValue("i", ui_base_class::is_nativeuiobject(obj,&ui_base_class::type) ? 1 : 0 );
}

// @pymethod <o PyDLL>|win32ui|GetResource|Retrieve the object associated with the applications resources.
static PyObject *
ui_get_resource(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetResource);
	HINSTANCE dll = AfxGetResourceHandle();
	dll_object *ret = (dll_object *)ui_assoc_object::make(dll_object::type, dll)->GetGoodRet();
	return ret;
}

// @pymethod <o PyUnicode>|win32ui|LoadString|Loads a string from a resource file.
static PyObject *ui_load_string(PyObject *self, PyObject *args)
{
	UINT stringId;
	if ( !PyArg_ParseTuple(args, "i",
						   &stringId)) // @pyparm int|stringId||The ID of the string to load.
		return NULL;
	CString ret;
	if (!ret.LoadString(stringId))
		RETURN_API_ERR("LoadString failed");
	const TCHAR *data = ret;
	return PyWinObject_FromTCHAR((TCHAR *)data, ret.GetLength());
}

// @pymethod <o PyDLL>|win32ui|SetResource|Specifies the default DLL object for application resources.
static PyObject *
ui_set_resource(PyObject *self, PyObject *args)
{
	PyObject *obDLL;
	HINSTANCE hMod;
	if (!PyArg_ParseTuple(args,"O:SetResource",
	           &obDLL)) // @pyparm <o PyDll>|dll||The dll object to use for default resources.
		return NULL;
	if (!ui_base_class::is_uiobject(obDLL, &dll_object::type))
		RETURN_TYPE_ERR("passed object must be a PyDLL");
	hMod = ((dll_object *)obDLL)->GetDll();
	if (hMod==NULL)
		RETURN_ERR("Can not set resource to an uninitialised DLL object");
	// setup for return value
	HINSTANCE oldDll = AfxGetResourceHandle();
	dll_object *ret = (dll_object *)ui_assoc_object::make(dll_object::type, oldDll)->GetGoodRet();
	AfxSetResourceHandle(hMod);
	return ret;
	// @rdesc The previous default DLL object.
}

// @pymethod |win32ui|WinHelp|Invokes the Windows Help system.
static PyObject *
ui_win_help( PyObject *self, PyObject *args )
{
	UINT cmd = HELP_CONTEXT;
	PyObject *dataOb;
	DWORD data;
	if (!PyArg_ParseTuple(args, "iO:WinHelp",
			  &cmd,    // @pyparm int|cmd|win32con.HELP_CONTEXT|The type of help.  See the api for full details.
			  &dataOb))   // @pyparm int/string|data||Additional data specific to the help call.
		return NULL;
	if (PyString_Check(dataOb))
		data = (DWORD)PyString_AsString(dataOb);
	else if (PyInt_Check(dataOb))
		data = (DWORD)PyInt_AsLong(dataOb);
	else {
		RETURN_TYPE_ERR("First argument must be a string or an integer.");
	}
	CWinApp *pApp = GetApp();
	if (!pApp) return NULL;
		
	GUI_BGN_SAVE;
	pApp->WinHelp(data, cmd);
	GUI_END_SAVE;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32ui|SetAppHelpPath|Set the pApp->m_pszHelpFilePath variable.
static PyObject *
ui_set_app_help_path(PyObject * self, PyObject * args)
{
  char *name;
  long style = MB_OK;
  if (!PyArg_ParseTuple(args, "s:SetAppHelpPath", &name))
    return NULL;
  CProtectedWinApp *pApp = GetProtectedApp();
  if (pApp==NULL) return NULL;

  GUI_BGN_SAVE;
  free((void*)pApp->m_pszHelpFilePath);
  pApp->m_pszHelpFilePath=_tcsdup(_T(name));
  GUI_END_SAVE;
  RETURN_NONE;
}

// @pymethod |win32ui|SetRegistryKey|Causes application settings to be stored in the registry instead of INI files.
static PyObject *
ui_set_registry_key(PyObject *self, PyObject *args)
{
	char *szKey;
	if (!PyArg_ParseTuple(args,"s:SetRegistryKey",&szKey)) // @pyparm string|key||A string containing the name of the key.
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	GUI_BGN_SAVE;
	pApp->SetRegistryKey(szKey);
	GUI_END_SAVE;
	// @comm Causes application settings to be stored in the registry instead of INI files. This function sets m_pszRegistryKey, which
	// is then used by the GetProfileXXX and WriteProfileXXX member functions of CWinApp. If this function has been
	// called, the list of most recently-used (MRU) files is also stored in the registry. The registry key is usually the name of a
	// company. It is stored in a key of the following form:
	// HKEY_CURRENT_USER\\Software\\\<company name\>\\\<application name\>\\\<section name\>\\\<value name\>.
	RETURN_NONE;
}

// @pymethod |win32ui|GetAppRegistryKey|Returns the registry key for the application.
static PyObject *
ui_get_app_registry_key(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":SetRegistryKey"))
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;

	/* Avoid MFC assertion */
	if (pApp->m_pszRegistryKey == NULL || pApp->m_pszProfileName==NULL)
		RETURN_ERR("There is no registry key open");

	GUI_BGN_SAVE;
	HKEY hk = pApp->GetAppRegistryKey();
	GUI_END_SAVE;
	if (hk==0) RETURN_ERR("There is no registry key open");
	return PyWinObject_FromHKEY(hk);
}

// @pymethod int|win32ui|SetDialogBkColor|Sets the default background and text color for dialog boxes and message boxes within the application.
static PyObject *
ui_set_dialog_bk_color(PyObject *self, PyObject *args)
{
	int clrCtlBk = RGB(192, 192, 192);
	int clrCtlText = RGB(0, 0, 0);

	// @pyparm int|clrCtlBk|win32ui.RGB(192, 192, 192)|The color for the controls background.
	// @pyparm int|clrCtlText|win32ui.RGB(0, 0, 0)|The color for the controls text.
	if (!PyArg_ParseTuple(args,"|ii:SetDialogBkColor", &clrCtlBk, &clrCtlText))
		return NULL;
	CProtectedWinApp *pApp = GetProtectedApp();
	if (!pApp) return NULL;
	GUI_BGN_SAVE;
	pApp->SetDialogBkColor(clrCtlBk, clrCtlText);
	GUI_END_SAVE;
	RETURN_NONE;
	// @pyseemfc CWinApp|SetDialogBkColor
}

// @pymethod int|win32ui|EnableControlContainer|Enables support for containment of OLE controls.
static PyObject *
ui_enable_control_container(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":EnableControlContainer"))
		return NULL;
	GUI_BGN_SAVE;
	AfxEnableControlContainer();
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod int|win32ui|GetAppName|Returns the application name.
static PyObject *
ui_get_app_name(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":GetAppName"))
		return NULL;
	return Py_BuildValue("s", AfxGetAppName());
}

// @pymethod int|win32ui|SetAppName|Sets the name of the application.
static PyObject *
ui_set_app_name(PyObject * self, PyObject * args)
{
	char *name;
	long style = MB_OK;
	const char *title = NULL;
	// @pyparm string|appName||The new name for the application.  This is used for the default registry key, and the title bar of the application.
	if (!PyArg_ParseTuple(args, "s:SetAppName", &name))
		return NULL;
	CWinApp *pApp = GetApp();
	if (pApp==NULL) return NULL;

	GUI_BGN_SAVE;
	free((void*)pApp->m_pszAppName);
	pApp->m_pszAppName=_tcsdup(_T(name));
	GUI_END_SAVE;
	RETURN_NONE;
	// @pyseemfc CWinApp|m_pszAppName
}

// @pymethod int|win32ui|IsDebug|Returns a flag indicating if the current win32ui build is a DEBUG build.
static PyObject *
ui_is_debug(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":IsDebug"))
		return NULL;
#ifdef _DEBUG
	return PyInt_FromLong(1);
#else
	return PyInt_FromLong(0);
#endif
	// @comm This should not normally be of relevance to the Python
	// programmer.  However, under certain circumstances Python code may
	// wish to detect this.
}

// @pymethod string|win32ui|RegisterWndClass|Registers a window class
static PyObject *
ui_register_wnd_class(PyObject *self, PyObject *args)
{
	long style;
	long hCursor = 0, hBrush = 0, hIcon = 0;
	if (!PyArg_ParseTuple(args,"l|lll:RegisterWndClass",
		&style, // @pyparm int|style||Specifies the Windows class style or combination of styles
		&hCursor, // @pyparm int|hCursor|0|
		&hBrush, // @pyparm int|hBrush|0|
		&hIcon)) // @pyparm int|hIcon|0|
		return NULL;

	GUI_BGN_SAVE;
	LPCTSTR ret = AfxRegisterWndClass( style, (HCURSOR)hCursor, (HBRUSH)hBrush, (HICON)hIcon); 
	GUI_END_SAVE;
	return PyString_FromString(ret);
	// @comm The Microsoft Foundation Class Library automatically registers several standard window classes for you. Call this function if you want to register your own window classes.
}

// @pymethod <o PyCWinApp>|win32ui|GetThread|Retrieves the current thread object.
static PyObject *
ui_get_thread(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS2(args,GetThread);
	CWinThread *pThread = AfxGetThread();
	if (pThread==NULL)
		RETURN_ERR("AfxGetThread failed");
	return ui_assoc_object::make(PyCWinThread::type, pThread)->GetGoodRet();
}

// @pymethod object|win32ui|GetType|Retrieves a Python Type object given its name
static PyObject *
ui_get_type(PyObject *self, PyObject *args)
{
	extern ui_type_CObject *UITypeFromName( const char *name );
	char *name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;
	PyObject *ret = (PyObject *)UITypeFromName(name);
	if (ret==NULL)
		RETURN_ERR("There is no type with that name");
	Py_INCREF(ret);
	return ret;
}

// @pymethod int|win32ui|SetCurrentInstanceHandle|Sets the MFC variable afxCurrentInstanceHandle
static PyObject *
ui_set_afxCurrentInstanceHandle(PyObject *self, PyObject *args)
{
	HMODULE newVal;
	// @pyparm int|newVal||The new value for afxCurrentInstanceHandle
	if (!PyArg_ParseTuple(args, "l", &newVal))
		return NULL;
	HMODULE old = afxCurrentInstanceHandle;
	afxCurrentInstanceHandle = newVal;
	return PyInt_FromLong((long)old);
	// @rdesc The result is the previous value of afxCurrentInstanceHandle
}

// @pymethod int|win32ui|SetCurrentResourceHandle|Sets the MFC variable afxCurrentResourceHandle
static PyObject *
ui_set_afxCurrentResourceHandle(PyObject *self, PyObject *args)
{
	HMODULE newVal;
	// @pyparm int|newVal||The new value for afxCurrentResourceHandle
	if (!PyArg_ParseTuple(args, "l", &newVal))
		return NULL;
	HMODULE old = afxCurrentResourceHandle;
	afxCurrentResourceHandle = newVal;
	return PyInt_FromLong((long)old);
	// @rdesc The result is the previous value of afxCurrentResourceHandle
}

// @pymethod string|win32ui|GetBytes|Gets raw bytes from memory
static PyObject *ui_get_bytes(PyObject *self, PyObject *args)
{
	long address;
	int size;
	// @pyparm int|address||The memory address
	// @pyparm int|size||The size to get.
	// @comm This method is useful to help decode unknown notify messages.
	// You must be very carefull when using this method.
	// @rdesc The result is a string with a length of size.
	if (!PyArg_ParseTuple(args, "li|GetBytes", &address, &size))
		return NULL;
	return PyString_FromStringAndSize((char *)address, size);
}
// @pymethod string|win32ui|InitRichEdit|Initializes the rich edit framework.
static PyObject *ui_init_rich_edit(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	GUI_BGN_SAVE;
	BOOL ok = AfxInitRichEdit();
	GUI_END_SAVE;
	if (!ok) RETURN_ERR("AfxInitRichEdit failed");
	RETURN_NONE;
}

// @pymethod int|win32ui|GetDeviceCaps|Calls the API version of GetDeviceCaps.  See also <om PyCDC.GetDeviceCaps>
static PyObject *ui_get_device_caps( PyObject *, PyObject *args )
{
	// @pyparm int|hdc||
	// @pyparm int|index||
	int hdc, index;
	if (!PyArg_ParseTuple(args, "ii", &hdc, &index))
		return NULL;
	return PyInt_FromLong( ::GetDeviceCaps( (HDC)hdc, index) );
}

// @pymethod int|win32ui|TranslateMessage|Calls the API version of TranslateMessage.
static PyObject *ui_translate_message(PyObject *, PyObject *args)
{
	MSG _msg; MSG *msg=&_msg;
	if (!PyArg_ParseTuple(args, "(iiiii(ii))", &msg->hwnd,&msg->message,&msg->wParam,&msg->lParam,&msg->time,&msg->pt.x,&msg->pt.y))
		return NULL;
	GUI_BGN_SAVE;
	BOOL rc = ::TranslateMessage(msg);
	GUI_END_SAVE;
	return PyInt_FromLong(rc);
}

// @pymethod string/None|win32ui|TranslateVirtualKey|
static PyObject *ui_translate_vk(PyObject *, PyObject *args)
{
	int vk;
	// @pyparm int|vk||The key to translate
	if (!PyArg_ParseTuple(args, "i", &vk))
		return NULL;
	static HKL layout=GetKeyboardLayout(0);
	static BYTE State[256];
	if (GetKeyboardState(State)==FALSE)
		RETURN_ERR("Can't get keyboard state");
	char result[2];
	UINT sc=MapVirtualKeyEx(vk,0,layout);
	int nc = ToAsciiEx(vk,sc,State,(unsigned short *)result,0,layout);
	if (nc==-1) { // a dead char.
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyString_FromStringAndSize(result, nc);
}

// @pymethod <o PyUnicode>/None|win32ui|TranslateVirtualKeyW|
static PyObject *ui_translate_vkW(PyObject *, PyObject *args)
{
	int vk;
	// @pyparm int|vk||The key to translate
	if (!PyArg_ParseTuple(args, "i", &vk))
		return NULL;
	static HKL layout=GetKeyboardLayout(0);
	static BYTE State[256];
	if (GetKeyboardState(State)==FALSE)
		RETURN_ERR("Can't get keyboard state");
	WCHAR result[2];
	UINT sc=MapVirtualKeyEx(vk,0,layout);
	int nc = ToUnicodeEx(vk,sc,State,result,2, 0,layout);
	if (nc==-1) { // a dead char.
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyWinObject_FromWCHAR(result, nc);
}

extern PyObject *ui_get_dialog_resource( PyObject *, PyObject *args );
extern PyObject *ui_create_app( PyObject *, PyObject *args );
extern PyObject *ui_get_app( PyObject *, PyObject *args );
extern PyObject *PyCButton_create(PyObject *self, PyObject *args);
extern PyObject *PyCEdit_create(PyObject *self, PyObject *args);
extern PyObject *PyCProgressCtrl_create(PyObject *self, PyObject *args);
extern PyObject *PyCSliderCtrl_create(PyObject *self, PyObject *args);
extern PyObject *PyCStatusBarCtrl_create(PyObject *self, PyObject *args);
extern PyObject *PyCToolBarCtrl_create(PyObject *self, PyObject *args);
extern PyObject *ui_window_create(PyObject *self, PyObject *args);
extern PyObject *PyCImageList_Create(PyObject *self, PyObject *args);
extern PyObject *PyCRichEditCtrl_create(PyObject *self, PyObject *args);
extern PyObject *win32uiCreatePalette(PyObject *self, PyObject *args);
extern PyObject *ui_create_dc_from_handle (PyObject *self, PyObject *args);
extern PyObject *ui_create_frame(PyObject *self, PyObject *args);
extern PyObject *ui_get_halftone_brush(PyObject *self, PyObject *args);
extern PyObject *PyCTreeCtrl_create(PyObject *self, PyObject *args);
extern PyObject *PyCListCtrl_create(PyObject *self, PyObject *args);

/* List of functions exported by this module */

// @module win32ui|A module, encapsulating the Microsoft Foundation Classes.
static struct PyMethodDef ui_functions[] = {
	{"AddToRecentFileList",		ui_add_to_recent_file_list,	1}, // @pymeth AddToRecentFileList|Add a file name to the Recent File List.
	{"ComparePath",				ui_compare_path,	1}, // @pymeth ComparePath|Compares 2 paths.
	{"CreateMDIFrame",			ui_create_mdi_frame,	1}, // @pymeth CreateMDIFrame|Creates an MDI Frame window.
	{"CreateMDIChild",			ui_create_mdi_child,	1}, // @pymeth CreateMDIChild|Creates an MDI Child window.
	{"CreateBitmap",			ui_bitmap::create,	1}, // @pymeth CreateBitmap|Create a bitmap object.
	{"CreateBitmapFromHandle",	ui_bitmap::create_from_handle,	1}, // @pymeth CreateBitmapFromHandle|Creates a bitmap object from a HBITMAP.
	{"CreateBrush",             PyCBrush::create, 1}, // @pymeth CreateBrush|Creates a new GDI brush object.  Returns a <o PyCBrush> object.
	{"CreateButton",            PyCButton_create, 1}, // @pymeth CreateButton|Creates a button object.  <om PyCButton.CreateWindow> creates the actual control.
	{"CreateColorDialog",       PyCColorDialog::create, 1}, // @pymeth CreateColorDialog|Creates a color selection dialog box.
	{"CreateControl",           PyCWnd::CreateControl, 1}, // @pymeth CreateControl|Creates an OLE control.
	{"CreateControlBar",        PyCControlBar::create, 1}, // @pymeth CreateControlBar|Creates an ControlBar
	{"CreateCtrlView",			PyCCtrlView::create,	1}, // @pymeth CreateCtrlView|Creates a control view object.
	{"CreateDC",                ui_dc_object::create_dc, 1}, // @pymeth CreateDC|Creates a <o PyCDC> object.
	{"CreateDCFromHandle",      ui_create_dc_from_handle, 1}, // @pymeth CreateDCFromHandle|Creates a <o PyCDC> object from an integer handle.
	{"CreateDialog",			PyCDialog::create,	1}, // @pymeth CreateDialog|Creates a <o PyCDialog> object.
	{"CreateDialogBar",			PyCDialogBar::create,	1}, // @pymeth CreateDialogBar|Creates a <o PyCDialogBar> object.
	{"CreateDialogIndirect",	PyCDialog::createIndirect, 1}, // @pymeth CreateDialogIndirect|Creates a <o PyCDialog> object from a template.
	{"CreatePrintDialog",		PyCPrintDialog::create,	1}, // @pymeth CreatePrintDialog|Creates a <o PyCPrintDialog> object.
	{"CreateDocTemplate",       PyCDocTemplate::create, 1}, // @pymeth CreateDocTemplate|Create a <o PyCDocTemplate> object.
	{"CreateEdit",            PyCEdit_create, 1}, // @pymeth CreateEdit|Creates an edit object.  <om PyCEdit.CreateWindow> creates the actual control.
	{"CreateFileDialog",		PyCFileDialog::ui_file_dialog_create,	1}, // @pymeth CreateFileDialog|Creates a FileOpen common dialog.
	{"CreateFontDialog",		PyCFontDialog::ui_font_dialog_create,	1}, // @pymeth CreateFontDialog|Creates a font selection dialog box.
	{"CreateFormView",			PyCFormView::create,	1}, // @pymeth CreateFormView|Creates a form view object.
	{"CreateFrame",             ui_create_frame, 1}, // @pymeth CreateFrame|Creates a frame window.
	{"CreateImageList",			PyCImageList_Create,	1}, // @pymeth CreateImageList|Creates an <o PyCImageList> object.
	{"CreateListCtrl",          PyCListCtrl_create,     1}, // @pymeth CreateListCtrl|Creates a list control.
	{"CreateListView",			PyCListView::create,	1}, // @pymeth CreateListView|Creates a <o PyCListView> object.
	{"CreateTreeCtrl",          PyCTreeCtrl_create,     1}, // @pymeth CreateTreeCtrl|Creates a tree control.
	{"CreateTreeView",			PyCTreeView::create,	1}, // @pymeth CreateTreeView|Creates a <o PyCTreeView> object.
	{"CreatePalette",           win32uiCreatePalette,   1}, // @pymeth CreatePalette|Returns a HPALETTE
	{"CreatePopupMenu",			PyCMenu::create_popup,	1}, // @pymeth CreatePopupMenu|Creates a popup menu.
	{"CreateMenu",				PyCMenu::create_menu,	1}, // @pymeth CreateMenu|Creates a menu
	{"CreatePen",				ui_pen_object::create,	1}, // @pymeth CreatePen|Creates a <o PyCPen> object.
	{"CreateProgressCtrl",		PyCProgressCtrl_create, 1}, // @pymeth CreateProgressCtrl|Creates a progress bar object.  <om PyCProgressCtrl.CreateWindow> creates the actual control.
	{"CreatePropertyPage",		PyCPropertyPage::create,		1}, // @pymeth CreatePropertyPage|Creates a <o PyCPropertyPage> object.
	{"CreatePropertyPageIndirect", PyCPropertyPage::createIndirect, 1}, // @pymeth CreatePropertyPageIndirect|Creates a <o PyCPropertyPage> object from a template.
	{"CreatePropertySheet",		PyCPropertySheet::create,	1}, // @pymeth CreatePropertySheet|Creates a <o PyCPropertySheet> object
	{"CreateRichEditCtrl",	    PyCRichEditCtrl_create,	1}, // @pymeth CreateRichEditCtrl|Creates a rich edit control.
	{"CreateRichEditDocTemplate", PyCRichEditDocTemplate::create, 1}, // @pymeth CreateRichEditDocTemplate|Create a <o PyCRichEditDocTemplate> object.
	{"CreateRichEditView",		PyCRichEditView::create,	1}, // @pymeth CreateRichEditView|Creates a <o PyCRichEditView> object.
	{"CreateSliderCtrl",		PyCSliderCtrl_create, 1}, // @pymeth CreateSliderCtrl|Creates a slider control object.  <om PyCSliderCtrl.CreateWindow> creates the actual control.
	{"CreateSplitter",			PyCSplitterWnd::create,	1}, // @pymeth CreateSplitter|Creates a splitter window.
	{"CreateStatusBar",			PyCStatusBar::create,	1}, // @pymeth CreateStatusBar|Creates a status bar object.
	{"CreateStatusBarCtrl",		PyCStatusBarCtrl_create, 1}, // @pymeth CreateStatusBarCtrl|Creates a new status bar control object. <om PyCStatusBarCtrl.CreateWindow> creates the actual control.
	{"CreateFont",				PyCFont::create,	1}, // @pymeth CreateFont|Creates a <o PyCFont> object.
	{"CreateToolBar",			PyCToolBar::create,	1}, // @pymeth CreateToolBar|Creates a toolbar object.
	{"CreateToolBarCtrl",		PyCToolBarCtrl_create,	1}, // @pymeth CreateToolBar|Creates a toolbar object.
	{"CreateThread",            PyCWinThread::create, 1}, // @pymeth CreateThread|Creates a <o PyCWinThread> object.
	{"CreateView",				PyCScrollView::create,		1}, // @pymeth CreateView|Creates a <o PyCView> object.
	{"CreateEditView",			PyCEditView::create,	1}, // @pymeth CreateEditView|Creates an <o PyCEditView> object.
	{"CreateDebuggerThread",    ui_create_debugger_thread, 1}, // @pymeth CreateDebuggerThread|Starts a debugging thread.
	{"CreateWindowFromHandle",  PyCWnd::CreateWindowFromHandle, 1}, // @pymeth CreateWindowFromHandle|Creates a <o PyCWnd> from an integer containing a HWND
	{"CreateWnd",				ui_window_create, 1},		// @pymeth CreateWnd|Create a new unitialized <o PyCWnd> object
	{"DestroyDebuggerThread",   ui_destroy_debugger_thread, 1}, // @pymeth DestroyDebuggerThread|Cleans up the debugger thread.
	{"DoWaitCursor",			ui_do_wait_cursor,	1}, // @pymeth DoWaitCursor|Changes the cursor to/from a wait cursor.
	{"Enable3dControls",		ui_enable_3d_controls, 1 }, // @pymeth Enable3dControls|Enables 3d controls for the application.
	{"FindWindow",				PyCWnd::FindWindow,	1}, // @pymeth FindWindow|Searches for the specified top-level window
	{"FindWindowEx",			PyCWnd::FindWindowEx,	1}, // @pymeth FindWindowEx|Searches for the specified top-level or child window
	{"FullPath",				ui_full_path,	1}, // @pymeth FullPath|Returns the full path name of the file.
	{"GetActiveWindow",			PyCWnd::GetActiveWindow, 1}, // @pymeth GetActiveWindow|Retrieves the active window.
	{"GetApp",                  ui_get_app, 1 },    // @pymeth GetApp|Retrieves the application object.
	{"GetAppName",              ui_get_app_name, 1 },    // @pymeth GetAppName|Retrieves the name of the current application.
	{"GetAppRegistryKey",       ui_get_app_registry_key, 1}, // @pymeth GetAppRegistryKey|Returns the registry key for the application.
	{"GetBytes",                ui_get_bytes, 1}, // @pymeth GetBytes|Gets raw bytes from memory
	{"GetCommandLine",			ui_get_command_line,	1}, // @pymeth GetCommandLine|Returns the command line for hte application.
	{"GetDeviceCaps",           ui_get_device_caps, 1}, // @pymeth GetDeviceCaps|Calls the API version of GetDeviceCaps.  See also <om PyCDC.GetDeviceCaps>
	{"GetFileTitle",            ui_get_file_title, 1}, // @pymeth GetFileTitle|Given a file name, return its title
	{"GetFocus",				PyCWnd::GetFocus, 1}, // @pymeth GetFocus|Retrieves the window with the focus.
	{"GetForegroundWindow",		PyCWnd::GetForegroundWindow, 1}, // @pymeth GetForegroundWindow|Retrieves the foreground window.
	{"GetHalftoneBrush",		ui_get_halftone_brush,	1}, // @pymeth GetHalftoneBrush|Returns a halftone brush.
	{"GetInitialStateRequest",	ui_get_initial_state_request,	1}, // @pymeth GetInitialStateRequest|Returns the requested state that the application start in.  This is the same as the paramaters available to <om PyCWnd.ShowWindow>
	{"GetMainFrame",            ui_get_main_frame,	1}, // @pymeth GetMainFrame|Returns a window object for the main application frame.
	{"GetName",					ui_get_name,	1}, // @pymeth GetName|Returns the name of the current application.
	{"GetProfileFileName",		ui_get_profile_filename,	1}, // @pymeth GetProfileFileName|Returns the name of the INI file used by the application.
	{"GetProfileVal",			ui_get_profile_val,	1}, // @pymeth GetProfileVal|Returns a value from the applications INI file.
	{"GetRecentFileList",		ui_get_recent_file_list,	1}, // @pymeth GetRecentFileList|Returns the recent file list.
	{"GetResource",				ui_get_resource,		1}, // @pymeth GetResource|Gets a resource.
	{"GetThread",               ui_get_thread, 1 },    // @pymeth GetThread|Retrieves the current thread object.
	{"GetType",                 ui_get_type, 1 },    // @pymeth GetType|Retrieves a Python Type object given its name
	{"InitRichEdit",            ui_init_rich_edit, 1}, // @pymeth InitRichEdit|Initializes the rich edit framework.
	{"InstallCallbackCaller",	ui_install_callback_caller,	1}, // @pymeth InstallCallbackCaller|Installs a callback caller.
	{"IsDebug",				    ui_is_debug, 1}, // @pymeth IsDebug|Returns a flag indicating if the current win32ui build is a DEBUG build.
	{"IsWin32s",				ui_is_win32s, 1}, // @pymeth IsWin32s|Determines if the application is running under Win32s.
	{"IsObject",				ui_is_object, 1}, // @pymeth IsObject|Determines if the passed object is a win32ui object.
	{"LoadDialogResource",		ui_get_dialog_resource,		1}, // @pymeth LoadDialogResource|Loads a dialog resource, and returns a list detailing the objects.
	{"LoadLibrary",				dll_object::create,	1}, // @pymeth LoadLibrary|Creates a <o PyDLL> object.
	{"LoadMenu",				PyCMenu::load_menu,	1}, // @pymeth LoadMenu|Loads a menu.
	{"LoadStdProfileSettings",	ui_load_std_profile_settings,	1}, // @pymeth LoadStdProfileSettings|Loads standard application profile settings.
	{"LoadString",				ui_load_string,	1}, // @pymeth LoadString|Loads a string from a resource file.
	{"MessageBox",				ui_message_box,	1}, // @pymeth MessageBox|Displays a message box.
	{"OutputDebug",				ui_output_debug,	1},
	{"OutputDebugString",		ui_output_debug,	1}, // @pymeth OutputDebugString|Writes output to the Windows debugger.
	{"EnableControlContainer",  ui_enable_control_container, 1, }, // @pymeth EnableControlContainer|Call this function in your application object's InitInstance function to enable support for containment of OLE controls.
	{"PrintTraceback", 			ui_python_print_traceback,	1}, // @pymeth PrintTraceback|Prints a Traceback using the default Python traceback printer.
	{"PumpWaitingMessages",		ui_pump_waiting_messages, 1}, // @pymeth PumpWaitingMessages|Pumps all waiting messages to the application.
	{"RegisterWndClass",        ui_register_wnd_class, 1}, // @pymeth RegisterWndClass|Registers a window class
	{"RemoveRecentFile",		ui_remove_recent_file,	1}, // @pymeth RemoveRecentFile|Removes the recent file at list index.
	{"SetAppHelpPath",          ui_set_app_help_path, 1}, // @pymeth SetAppHelpPath|Sets the application help file path, i.e. the pApp->m_pszHelpFilePath member variable.
	{"SetAppName",              ui_set_app_name, 1}, // @pymeth SetAppName|Sets the application name.
	{"SetCurrentInstanceHandle",ui_set_afxCurrentInstanceHandle, 1}, // @pymeth SetCurrentInstanceHandle|Sets the MFC variable afxCurrentInstanceHandle.
	{"SetCurrentResourceHandle",ui_set_afxCurrentResourceHandle, 1}, // @pymeth SetCurrentResourceHandle|Sets the MFC variable afxCurrentResourceHandle.
	{"SetDialogBkColor",        ui_set_dialog_bk_color, 1}, // @pymeth SetDialogBkColor|Sets the default background and text color for dialog boxes and message boxes within the application.
	{"SetProfileFileName",		ui_set_profile_filename,	1}, // @pymeth SetProfileFileName|Sets the INI file name used by the application.
	{"SetRegistryKey",          ui_set_registry_key, 1 }, // @pymeth SetRegistryKey|Causes application settings to be stored in the registry instead of INI files.
	{"SetResource",				ui_set_resource,		1}, // @pymeth SetResource|Specifies the default DLL object for application resources.
	{"SetStatusText",			ui_set_status_text,	1}, // @pymeth SetStatusText|Sets the text in the status bar.
	{"StartDebuggerPump",		ui_start_debugger_pump,	1}, // @pymeth StartDebuggerPump|Starts the debugger message pump.
	{"StopDebuggerPump",		ui_stop_debugger_pump,	1}, // @pymeth StopDebuggerPump|Stops the debugger message pump.
	{"TranslateMessage",            ui_translate_message, 1}, // @pymeth TranslateMessage|Calls ::TranslateMessage.
	{"TranslateVirtualKey",         ui_translate_vk, 1}, // @pymeth TranslateVirtualKey|Translates a virtual key.
	{"TranslateVirtualKeyW",        ui_translate_vkW, 1},// @pymeth TranslateVirtualKeyW|Translates a virtual key.
	{"WinHelp",					ui_win_help,	1}, // @pymeth WinHelp|Invokes the Window Help engine.
	{"WriteProfileVal",			ui_write_profile_val,	1}, // @pymeth WriteProfileVal|Writes a value to the INI file.

	{NULL,			NULL}
};

static int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyInt_FromLong(value);
	if (!okey || !oval) {
		XDODECREF(okey);
		XDODECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	DODECREF(okey);
	DODECREF(oval);
	return rc;
}
#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, tok)) return rc
#define ADD_ENUM(parta, partb) if (rc=AddConstant(dict,#parta "_" #partb, parta::partb)) return rc
#define ADD_ENUM3(parta, partb, partc) if (rc=AddConstant(dict,#parta "_" #partb "_" #partc, parta::partb::partc)) return rc

int AddConstants(PyObject *dict)
{
	int rc;
#ifdef _DEBUG
	int debug = 1;
#else
	int debug = 0;
#endif
	ADD_CONSTANT(debug); // @const win32ui|debug|1 if we are current using a _DEBUG build of win32ui, else 0.
	ADD_CONSTANT(AFX_IDW_PANE_FIRST); // @const win32ui|AFX_IDW_PANE_FIRST|Id of the first splitter pane
	ADD_CONSTANT(AFX_IDW_PANE_LAST);  // @const win32ui|AFX_IDW_PANE_LAST|Id of the last splitter pane
	ADD_CONSTANT(AFX_WS_DEFAULT_VIEW); // @const win32ui|AFX_WS_DEFAULT_VIEW|
	ADD_CONSTANT(FWS_ADDTOTITLE);     // @const win32ui|FWS_ADDTOTITLE|MFC Frame Window style extension.  Add document title to window title.
	ADD_CONSTANT(FWS_PREFIXTITLE);    // @const win32ui|FWS_PREFIXTITLE|MFC Frame Window style extension.
	ADD_CONSTANT(FWS_SNAPTOBARS);     // @const win32ui|FWS_SNAPTOBARS|MFC Frame Window style extension.

	ADD_CONSTANT(IDD_ABOUTBOX);       // @const win32ui|IDD_ABOUTBOX|Id of built in 'About Box' dialog
	ADD_CONSTANT(IDD_DUMMYPROPPAGE);  // @const win32ui|IDD_DUMMYPROPPAGE|Id of built in dummy property page
	ADD_CONSTANT(IDD_PROPDEMO1);	  // @const win32ui|IDD_PROPDEMO1|Id of built in Property Page demo dialog 1
	ADD_CONSTANT(IDD_PROPDEMO2);	  // @const win32ui|IDD_PROPDEMO2|Id of built in Property Page demo dialog 2
	ADD_CONSTANT(IDB_DEBUGGER_HIER);  // @const win32ui|IDB_DEBUGGER_HIER|
	ADD_CONSTANT(IDB_HIERFOLDERS);	  // @const win32ui|IDB_HIERFOLDERS|Id of built in bitmap for default hierarchical list
	ADD_CONSTANT(IDB_BROWSER_HIER);	  // @const win32ui|IDB_BROWSER_HIER|Id of built in bitmap for the browser
	ADD_CONSTANT(IDD_GENERAL_STATUS); // @const win32ui|IDD_GENERAL_STATUS|Id of a general status dialog box (fairly small, 3 static controls, minimize box)
	ADD_CONSTANT(IDD_LARGE_EDIT);	  // @const win32ui|IDD_LARGE_EDIT|Id of built in 'Large Edit' dialog (dialog box with a large edit control)
	ADD_CONSTANT(IDD_TREE);	  // @const win32ui|IDD_TREE|Id of built in dialog with a tree control.
	ADD_CONSTANT(IDD_TREE_MB);// @const win32ui|IDD_TREE_MB|Id of built in dialog with a tree control with multiple buttons.
	ADD_CONSTANT(IDD_RUN_SCRIPT);	  // @const win32ui|IDD_RUN_SCRIPT|Id of built in 'Run Script' dialog
	ADD_CONSTANT(IDD_PP_EDITOR); 	  // @const win32ui|IDD_PP_EDITOR|Id of built in 'Editor' property page
	ADD_CONSTANT(IDD_PP_DEBUGGER); // @const win32ui|IDD_PP_DEBUGGER|
	ADD_CONSTANT(IDD_PP_FORMAT); 	  // @const win32ui|IDD_PP_FORMAT|Id of built in 'Format' property page
	ADD_CONSTANT(IDD_PP_IDE);    // @const win32ui|IDD_PP_IDE|Id of built in 'IDE' property page
	ADD_CONSTANT(IDD_PP_TABS); 	  // @const win32ui|IDD_PP_TABS|Id of built in 'Tabs and Whitespace' property page
	ADD_CONSTANT(IDD_PP_TOOLMENU);    // @const win32ui|IDD_PP_TOOLMENU|Id of built in 'ToolsMenu' property page
	ADD_CONSTANT(IDD_SIMPLE_INPUT);	  // @const win32ui|IDD_SIMPLE_INPUT|Id of built in 'Simple Input' property page.
	ADD_CONSTANT(IDD_SET_TABSTOPS);  // @const win32ui|IDD_SET_TABSTOPS|Id of built in 'Set Tab Stops' dialog

	ADD_CONSTANT(IDC_DBG_STEP);
	ADD_CONSTANT(IDC_DBG_STEPOUT);
	ADD_CONSTANT(IDC_DBG_STEPOVER);
	ADD_CONSTANT(IDC_DBG_GO);
	ADD_CONSTANT(IDC_DBG_ADD);
	ADD_CONSTANT(IDC_DBG_CLEAR);
	ADD_CONSTANT(IDC_DBG_CLOSE);
	ADD_CONSTANT(IDC_DBG_STACK);
	ADD_CONSTANT(IDC_DBG_BREAKPOINTS);
	ADD_CONSTANT(IDC_DBG_WATCH);

	ADD_CONSTANT(IDC_ABOUT_VERSION); // @const win32ui|IDC_ABOUT_VERSION|Id of 'Version' control
	ADD_CONSTANT(IDC_AUTO_RELOAD);		  // @const win32ui|IDC_AUTO_RELOAD|
	ADD_CONSTANT(IDC_BUTTON1);		  // @const win32ui|IDC_BUTTON1|
	ADD_CONSTANT(IDC_BUTTON2);		  // @const win32ui|IDC_BUTTON2|
	ADD_CONSTANT(IDC_BUTTON3);		  // @const win32ui|IDC_BUTTON3|
	ADD_CONSTANT(IDC_BUTTON4);		  // @const win32ui|IDC_BUTTON4|
	ADD_CONSTANT(IDC_CHECK1);		  // @const win32ui|IDC_CHECK1|
	ADD_CONSTANT(IDC_CHECK2);		  // @const win32ui|IDC_CHECK2|
	ADD_CONSTANT(IDC_CHECK3);		  // @const win32ui|IDC_CHECK3|
	ADD_CONSTANT(IDC_COMBO1);		  // @const win32ui|IDC_COMBO1|
	ADD_CONSTANT(IDC_COMBO2);		  // @const win32ui|IDC_COMBO2|
	ADD_CONSTANT(IDC_EDIT1);		  // @const win32ui|IDC_EDIT1|
	ADD_CONSTANT(IDC_EDIT2);// @const win32ui|IDC_EDIT2|
	ADD_CONSTANT(IDC_EDIT3);// @const win32ui|IDC_EDIT3|
	ADD_CONSTANT(IDC_EDIT4);// @const win32ui|IDC_EDIT4|
	ADD_CONSTANT(IDC_EDIT_TABS);// @const win32ui|IDC_EDIT_TABS|
	ADD_CONSTANT(IDC_EDITOR_COLOR); // @const win32ui|IDC_EDIT_COLOE|
	ADD_CONSTANT(IDC_FOLD_SHOW_LINES);
	ADD_CONSTANT(IDC_FOLD_ENABLE);
	ADD_CONSTANT(IDC_FOLD_ON_OPEN);
	ADD_CONSTANT(IDC_INDENT_SIZE);// @const win32ui|IDC_INDENT_SIZE|
	ADD_CONSTANT(IDC_KEYBOARD_CONFIG); // @const win32ui|IDC_KEYBOARD_CONFIG|
	ADD_CONSTANT(IDC_MARGIN_LINENUMBER);
	ADD_CONSTANT(IDC_MARGIN_FOLD);
	ADD_CONSTANT(IDC_MARGIN_MARKER);
	ADD_CONSTANT(IDC_LIST1);// @const win32ui|IDC_LIST1|
	ADD_CONSTANT(IDC_PROMPT_TABS);// @const win32ui|IDC_PROMPT_TABS|
	ADD_CONSTANT(IDC_PROMPT1);// @const win32ui|IDC_PROMPT1|
	ADD_CONSTANT(IDC_PROMPT2);// @const win32ui|IDC_PROMPT2|
	ADD_CONSTANT(IDC_PROMPT3);// @const win32ui|IDC_PROMPT3|
	ADD_CONSTANT(IDC_PROMPT4);// @const win32ui|IDC_PROMPT4|
	ADD_CONSTANT(IDC_RADIO1);// @const win32ui|IDC_RADIO1|
	ADD_CONSTANT(IDC_RADIO2);// @const win32ui|IDC_RADIO2|
	ADD_CONSTANT(IDC_TABTIMMY_NONE);
	ADD_CONSTANT(IDC_TABTIMMY_IND);
	ADD_CONSTANT(IDC_TABTIMMY_BG);
	ADD_CONSTANT(IDC_VIEW_WHITESPACE);// @const win32ui|IDC_VIEW_WHITESPACE|
	ADD_CONSTANT(IDC_VIEW_EOL);
	ADD_CONSTANT(IDC_VIEW_INDENTATIONGUIDES);
	ADD_CONSTANT(IDC_AUTOCOMPLETE); // @const win32ui|IDC_AUTOCOMPLETE|
	ADD_CONSTANT(IDC_CALLTIPS); // @const win32ui|IDC_CALLTIPS|


	ADD_CONSTANT(IDC_SPIN1); // @const win32ui|IDC_SPIN1|
	ADD_CONSTANT(IDC_SPIN2); // @const win32ui|IDC_SPIN2|
	
	ADD_CONSTANT(IDC_TAB_SIZE);// @const win32ui|IDC_TAB_SIZE|
	ADD_CONSTANT(IDC_USE_TABS);// @const win32ui|IDC_USE_TABS|
	ADD_CONSTANT(IDC_USE_SMART_TABS);// @const win32ui|IDC_USE_SMART_TABS|
	ADD_CONSTANT(IDC_VSS_INTEGRATE);// @const win32ui|IDC_VSS_INTEGRATE|

	ADD_CONSTANT(ID_INDICATOR_LINENUM);// @const win32ui|ID_INDICATOR_LINENUM|
	ADD_CONSTANT(ID_INDICATOR_COLNUM);// @const win32ui|ID_INDICATOR_COLNUM|
	
	ADD_CONSTANT(ID_FILE_NEW);// @const win32ui|ID_FILE_NEW|
	ADD_CONSTANT(ID_FILE_OPEN);// @const win32ui|ID_FILE_OPEN|
	ADD_CONSTANT(ID_FILE_CLOSE);// @const win32ui|ID_FILE_CLOSE|
	ADD_CONSTANT(ID_FILE_RUN);// @const win32ui|ID_FILE_RUN|
	ADD_CONSTANT(ID_FILE_IMPORT);// @const win32ui|ID_FILE_IMPORT|
	ADD_CONSTANT(ID_FILE_LOCATE);// @const win32ui|ID_FILE_LOCATE|
	ADD_CONSTANT(ID_FILE_CHECK);// @const win32ui|ID_FILE_CHECK|
	ADD_CONSTANT(ID_FILE_SAVE);// @const win32ui|ID_FILE_SAVE|
	ADD_CONSTANT(ID_FILE_SAVE_AS);// @const win32ui|ID_FILE_SAVE_AS|
	ADD_CONSTANT(ID_FILE_SAVE_ALL);// @const win32ui|ID_FILE_SAVE_ALL|

	ADD_CONSTANT(ID_FILE_PAGE_SETUP);// @const win32ui|ID_FILE_PAGE_SETUP|
	ADD_CONSTANT(ID_FILE_PRINT_SETUP);// @const win32ui|ID_FILE_PRINT_SETUP|
	ADD_CONSTANT(ID_FILE_PRINT);// @const win32ui|ID_FILE_PRINT|
	ADD_CONSTANT(ID_FILE_PRINT_PREVIEW);// @const win32ui|ID_FILE_PRINT_PREVIEW|
	ADD_CONSTANT(ID_HELP_PYTHON);// @const win32ui|ID_HELP_PYTHON|
	ADD_CONSTANT(ID_HELP_GUI_REF);// @const win32ui|ID_HELP_GUI_REF|
	ADD_CONSTANT(ID_HELP_OTHER);// @const win32ui|ID_HELP_OTHER|
	ADD_CONSTANT(ID_APP_ABOUT);// @const win32ui|ID_APP_ABOUT|
	ADD_CONSTANT(ID_APP_EXIT);// @const win32ui|ID_APP_EXIT|
	ADD_CONSTANT(ID_FILE_MRU_FILE1);// @const win32ui|ID_FILE_MRU_FILE1|
	ADD_CONSTANT(ID_FILE_MRU_FILE2);// @const win32ui|ID_FILE_MRU_FILE2|
	ADD_CONSTANT(ID_FILE_MRU_FILE3);// @const win32ui|ID_FILE_MRU_FILE3|
	ADD_CONSTANT(ID_FILE_MRU_FILE4);// @const win32ui|ID_FILE_MRU_FILE4|
	ADD_CONSTANT(ID_VIEW_BROWSE);// @const win32ui|ID_VIEW_BROWSE|
	ADD_CONSTANT(ID_VIEW_FIXED_FONT);// @const win32ui|ID_VIEW_FIXED_FONT|
	ADD_CONSTANT(ID_VIEW_INTERACTIVE);// @const win32ui|ID_VIEW_INTERACTIVE|
	ADD_CONSTANT(ID_VIEW_OPTIONS); // @const win32ui|ID_VIEW_OPTIONS|
	ADD_CONSTANT(ID_VIEW_TOOLBAR_DBG); // @const win32ui|ID_VIEW_TOOLBAR_DBG|
	ADD_CONSTANT(ID_VIEW_WHITESPACE); // @const win32ui|ID_VIEW_WHITESPACE|
	ADD_CONSTANT(ID_VIEW_INDENTATIONGUIDES); // @const win32ui|ID_VIEW_INDENTATIONGUIDES|
	ADD_CONSTANT(ID_VIEW_EOL); // @const win32ui|ID_VIEW_EOL|
	ADD_CONSTANT(ID_VIEW_FOLD_EXPAND); // @const win32ui|ID_VIEW_FOLD_EXPAND|
	ADD_CONSTANT(ID_VIEW_FOLD_EXPAND_ALL); // @const win32ui|ID_VIEW_FOLD_EXPAND_ALL|
	ADD_CONSTANT(ID_VIEW_FOLD_COLLAPSE); // @const win32ui|ID_VIEW_FOLD_COLLAPSE|
	ADD_CONSTANT(ID_VIEW_FOLD_COLLAPSE_ALL); // @const win32ui|ID_VIEW_FOLD_COLLAPSE_ALL|
	ADD_CONSTANT(ID_VIEW_FOLD_TOPLEVEL); // @const win32ui|ID_VIEW_FOLD_TOGGLE|
	ADD_CONSTANT(ID_NEXT_PANE);// @const win32ui|ID_NEXT_PANE|
	ADD_CONSTANT(ID_PREV_PANE);// @const win32ui|ID_PREV_PANE|
	ADD_CONSTANT(ID_WINDOW_NEW);// @const win32ui|ID_WINDOW_NEW|
	ADD_CONSTANT(ID_WINDOW_ARRANGE);// @const win32ui|ID_WINDOW_ARRANGE|
	ADD_CONSTANT(ID_WINDOW_CASCADE);// @const win32ui|ID_WINDOW_CASCADE|
	ADD_CONSTANT(ID_WINDOW_TILE_HORZ);// @const win32ui|ID_WINDOW_TILE_HORZ|
	ADD_CONSTANT(ID_WINDOW_TILE_VERT);// @const win32ui|ID_WINDOW_TILE_VERT|
	ADD_CONSTANT(ID_WINDOW_SPLIT);// @const win32ui|ID_WINDOW_SPLIT|
	ADD_CONSTANT(ID_EDIT_CLEAR);// @const win32ui|ID_EDIT_CLEAR|
	ADD_CONSTANT(ID_EDIT_CLEAR_ALL);// @const win32ui|ID_EDIT_CLEAR_ALL|
	ADD_CONSTANT(ID_EDIT_COPY);// @const win32ui|ID_EDIT_COPY|
	ADD_CONSTANT(ID_EDIT_CUT);// @const win32ui|ID_EDIT_CUT|
	ADD_CONSTANT(ID_EDIT_FIND);// @const win32ui|ID_EDIT_FIND|
	ADD_CONSTANT(ID_EDIT_GOTO_LINE); // @const win32ui|ID_EDIT_GOTO_LINE|
	ADD_CONSTANT(ID_EDIT_PASTE);// @const win32ui|ID_EDIT_PASTE|
	ADD_CONSTANT(ID_EDIT_REPEAT);// @const win32ui|ID_EDIT_REPEAT|
	ADD_CONSTANT(ID_EDIT_REPLACE);// @const win32ui|ID_EDIT_REPLACE|
	ADD_CONSTANT(ID_EDIT_SELECT_ALL);// @const win32ui|ID_EDIT_SELECT_ALL|
	ADD_CONSTANT(ID_EDIT_SELECT_BLOCK);// @const win32ui|ID_EDIT_SELECT_BLOCK|
	ADD_CONSTANT(ID_EDIT_UNDO);// @const win32ui|ID_EDIT_UNDO|
	ADD_CONSTANT(ID_EDIT_REDO);// @const win32ui|ID_EDIT_REDO|
	ADD_CONSTANT(ID_VIEW_TOOLBAR);// @const win32ui|ID_VIEW_TOOLBAR|
	ADD_CONSTANT(ID_VIEW_STATUS_BAR);// @const win32ui|ID_VIEW_STATUS_BAR|
	ADD_CONSTANT(ID_SEPARATOR);// @const win32ui|ID_SEPARATOR|

	ADD_CONSTANT(IDR_DEBUGGER); // @const win32ui|IDR_DEBUGGER|
	ADD_CONSTANT(IDR_PYTHONTYPE_CNTR_IP);// @const win32ui|IDR_PYTHONTYPE_CNTR_IP|
	ADD_CONSTANT(IDR_MAINFRAME);// @const win32ui|IDR_MAINFRAME|
	ADD_CONSTANT(IDR_PYTHONTYPE);// @const win32ui|IDR_PYTHONTYPE|
	ADD_CONSTANT(IDR_PYTHONCONTYPE);// @const win32ui|IDR_PYTHONCONTYPE|
	ADD_CONSTANT(IDR_TEXTTYPE);// @const win32ui|IDR_TEXTTYPE|
	ADD_CONSTANT(IDR_CNTR_INPLACE);// @const win32ui|IDR_CNTR_INPLACE|
	ADD_ENUM(CDocTemplate,windowTitle);// @const win32ui|CDocTemplate_windowTitle|
	ADD_ENUM(CDocTemplate,docName);// @const win32ui|CDocTemplate_docName|
	ADD_ENUM(CDocTemplate,fileNewName);// @const win32ui|CDocTemplate_fileNewName|
	ADD_ENUM(CDocTemplate,filterName);// @const win32ui|CDocTemplate_filterName|
	ADD_ENUM(CDocTemplate,filterExt);// @const win32ui|CDocTemplate_filterExt|
	ADD_ENUM(CDocTemplate,regFileTypeId);// @const win32ui|CDocTemplate_regFileTypeId|
	ADD_ENUM(CDocTemplate,regFileTypeName);// @const win32ui|CDocTemplate_regFileTypeName|

	ADD_ENUM3(CDocTemplate, Confidence, noAttempt); // @const win32ui|CDocTemplate_Confidence_noAttempt|
	ADD_ENUM3(CDocTemplate, Confidence, maybeAttemptForeign); // @const win32ui|CDocTemplate_Confidence_maybeAttemptForeign|
	ADD_ENUM3(CDocTemplate, Confidence, maybeAttemptNative); // @const win32ui|CDocTemplate_Confidence_maybeAttemptNative|
	ADD_ENUM3(CDocTemplate, Confidence, yesAttemptForeign); // @const win32ui|CDocTemplate_Confidence_yesAttemptForeign|
	ADD_ENUM3(CDocTemplate, Confidence, yesAttemptNative); // @const win32ui|CDocTemplate_Confidence_yesAttemptNative|
	ADD_ENUM3(CDocTemplate, Confidence, yesAlreadyOpen); // @const win32ui|CDocTemplate_Confidence_yesAlreadyOpen|

	ADD_ENUM(CRichEditView,WrapNone);// @const win32ui|CRichEditView_WrapNone|
	ADD_ENUM(CRichEditView,WrapToWindow);// @const win32ui|CRichEditView_WrapToWindow|
	ADD_ENUM(CRichEditView,WrapToTargetDevice);// @const win32ui|CRichEditView_WrapToTargetDevice|

	ADD_CONSTANT(PD_ALLPAGES); // @const win32ui|PD_ALLPAGES|The default flag that indicates that the All radio button is initially selected. This flag is used as a placeholder to indicate that the PD_PAGENUMS and PD_SELECTION flags are not specified. 
	ADD_CONSTANT(PD_COLLATE); // @const win32ui|PD_COLLATE|If this flag is set, the Collate check box is checked. If this flag is set when the PrintDlg function returns, the application must simulate collation of multiple copies. For more information, see the description of the PD_USEDEVMODECOPIESANDCOLLATE flag. 
	ADD_CONSTANT(PD_DISABLEPRINTTOFILE); // @const win32ui|PD_DISABLEPRINTTOFILE|Disables the Print to File check box.
	ADD_CONSTANT(PD_ENABLEPRINTHOOK); // @const win32ui|PD_ENABLEPRINTHOOK|Enables the hook procedure specified in the lpfnPrintHook member. This enables the hook procedure for the Print dialog box.
	ADD_CONSTANT(PD_ENABLEPRINTTEMPLATE); // @const win32ui|PD_ENABLEPRINTTEMPLATE|PD_ENABLEPRINTTEMPLATE
	ADD_CONSTANT(PD_ENABLEPRINTTEMPLATEHANDLE); // @const win32ui|PD_ENABLEPRINTTEMPLATEHANDLE|Indicates that the hPrintTemplate member identifies a data block that contains a preloaded dialog box template. This template replaces the default template for the Print dialog box. The system ignores the lpPrintTemplateName member if this flag is specified. 
	ADD_CONSTANT(PD_ENABLESETUPHOOK); // @const win32ui|PD_ENABLESETUPHOOK|Enables the hook procedure specified in the lpfnSetupHook member. This enables the hook procedure for the Print Setup dialog box.
	ADD_CONSTANT(PD_ENABLESETUPTEMPLATE); // @const win32ui|PD_ENABLESETUPTEMPLATE|Indicates that the hInstance and lpSetupTemplateName members specify a replacement for the default Print Setup dialog box template. 
	ADD_CONSTANT(PD_ENABLESETUPTEMPLATEHANDLE); // @const win32ui|PD_ENABLESETUPTEMPLATEHANDLE|Indicates that the hSetupTemplate member identifies a data block that contains a preloaded dialog box template. This template replaces the default template for the Print Setup dialog box. The system ignores the lpSetupTemplateName member if this flag is specified. 
	ADD_CONSTANT(PD_HIDEPRINTTOFILE); // @const win32ui|PD_HIDEPRINTTOFILE|Hides the Print to File check box.
	ADD_CONSTANT(PD_NONETWORKBUTTON); // @const win32ui|PD_NONETWORKBUTTON|Hides and disables the Network button. 
	ADD_CONSTANT(PD_NOPAGENUMS); // @const win32ui|PD_NOPAGENUMS|Disables the Pages radio button and the associated edit controls.
	ADD_CONSTANT(PD_NOSELECTION); // @const win32ui|PD_NOSELECTION|Disables the Selection radio button.
	ADD_CONSTANT(PD_NOWARNING); // @const win32ui|PD_NOWARNING|Prevents the warning message from being displayed when there is no default printer.
	ADD_CONSTANT(PD_PAGENUMS); // @const win32ui|PD_PAGENUMS|If this flag is set, the Pages radio button is selected. If this flag is set when the PrintDlg function returns, the nFromPage and nFromPage members indicate the starting and ending pages specified by the user.
	ADD_CONSTANT(PD_PRINTSETUP); // @const win32ui|PD_PRINTSETUP|Causes the system to display the Print Setup dialog box rather than the Print dialog box.
	ADD_CONSTANT(PD_PRINTTOFILE); // @const win32ui|PD_PRINTTOFILE|If this flag is set, the Print to File check box is selected. If this flag is set when the PrintDlg function returns, the offset indicated by the wOutputOffset member of the DEVNAMES structure contains the string "FILE:". When you call theStartDoc function to start the printing operation, specify this "FILE:" string in the lpszOutput member of theDOCINFO structure. Specifying this string causes the print subsystem to query the user for the name of the output file. 
	ADD_CONSTANT(PD_RETURNDC); // @const win32ui|PD_RETURNDC|Causes PrintDlg to return a device context matching the selections the user made in the dialog box. The device context is returned in hDC.
	ADD_CONSTANT(PD_RETURNDEFAULT); // @const win32ui|PD_RETURNDEFAULT|If this flag is set, the PrintDlg function does not display the dialog box. Instead, it sets the hDevNames and hDevMode members to handles toDEVMODE and DEVNAMES structures that are initialized for the system default printer. Both hDevNames and hDevMode must be NULL, or PrintDlg returns an error. If the system default printer is supported by an old printer driver (earlier than Windows version 3.0), only hDevNames is returned; hDevMode is NULL.
	ADD_CONSTANT(PD_RETURNIC); // @const win32ui|PD_RETURNIC|Similar to the PD_RETURNDC flag, except this flag returns an information context rather than a device context. If neither PD_RETURNDC nor PD_RETURNIC is specified, hDC is undefined on output.
	ADD_CONSTANT(PD_SELECTION); // @const win32ui|PD_SELECTION|If this flag is set, the Selection radio button is selected. If neither PD_PAGENUMS nor PD_SELECTION is set, the All radio button is selected. 
	ADD_CONSTANT(PD_SHOWHELP); // @const win32ui|PD_SHOWHELP|Causes the dialog box to display the Help button. The hwndOwner member must specify the window to receive the HELPMSGSTRING registered messages that the dialog box sends when the user clicks the Help button.
	ADD_CONSTANT(PD_USEDEVMODECOPIES); // @const win32ui|PD_USEDEVMODECOPIES|Same as PD_USEDEVMODECOPIESANDCOLLATE
	ADD_CONSTANT(PD_USEDEVMODECOPIESANDCOLLATE); // @const win32ui|PD_USEDEVMODECOPIESANDCOLLATE|This flag indicates whether your application supports multiple copies and collation. Set this flag on input to indicate that your application does not support multiple copies and collation. In this case, the nCopies member of the PRINTDLG structure always returns 1, and PD_COLLATE is never set in the Flags member. If this flag is not set, the application is responsible for printing and collating multiple copies. In this case, the nCopies member of the PRINTDLG structure indicates the number of copies the user wants to print, and the PD_COLLATE flag in the Flags member indicates whether the user wants collation. Regardless of whether this flag is set, an application can determine from nCopies and PD_COLLATE how many copies to render and whether to print them collated.  If this flag is set and the printer driver does not support multiple copies, the Copies edit control is disabled. Similarly, if this flag is set and the printer driver does not support collation, the Collate checkbox is disabled. The dmCopies and dmCollate members of theDEVMODE structure contain the copies and collate information used by the printer driver. If this flag is set and the printer driver supports multiple copies, the dmCopies member indicates the number of copies requested by the user. If this flag is set and the printer driver supports collation, the dmCollate member of the DEVMODE structure indicates whether the user wants collation. If this flag is not set, the dmCopies member always returns 1, and the dmCollate member is always zero.

	ADD_CONSTANT(PSWIZB_BACK); // @const win32ui|PSWIZB_BACK|Enable/Disable the Property sheet Back button
	ADD_CONSTANT(PSWIZB_NEXT); // @const win32ui|PSWIZB_NEXT|Enable/Disable the Property sheet Next button
	ADD_CONSTANT(PSWIZB_FINISH); // @const win32ui|PSWIZB_FINISH|Enable/Disable the Property sheet Finish button
	ADD_CONSTANT(PSWIZB_DISABLEDFINISH); // @const win32ui|PSWIZB_DISABLEDFINISH|Enable/Disable the Property sheet disabled Finish button

	ADD_CONSTANT(MFS_SYNCACTIVE); // @const win32ui|MFS_SYNCACTIVE|syncronize activation w/ parent
	ADD_CONSTANT(MFS_4THICKFRAME); // @const win32ui|MFS_4THICKFRAME|thick frame all around (no tiles)
	ADD_CONSTANT(MFS_THICKFRAME); // @const win32ui|MFS_THICKFRAME|use instead of WS_THICKFRAME
	ADD_CONSTANT(MFS_MOVEFRAME); // @const win32ui|MFS_MOVEFRAME|no sizing, just moving
	ADD_CONSTANT(MFS_BLOCKSYSMENU); // @const win32ui|MFS_BLOCKSYSMENU|block hit testing on system menu

	// Layout Modes for CalcDynamicLayout
	ADD_CONSTANT(LM_STRETCH); // @const win32ui|LM_STRETCH|same meaning as bStretch in CalcFixedLayout.  If set, ignores nLength and returns dimensions based on LM_HORZ state, otherwise LM_HORZ is used to determine if nLength is the desired horizontal or vertical length and dimensions are returned based on nLength
	ADD_CONSTANT(LM_HORZ); // @const win32ui|LM_HORZ|same as bHorz in CalcFixedLayout
	ADD_CONSTANT(LM_MRUWIDTH); // @const win32ui|LM_MRUWIDTH|Most Recently Used Dynamic Width
	ADD_CONSTANT(LM_HORZDOCK); // @const win32ui|LM_HORZDOCK|Horizontal Docked Dimensions
	ADD_CONSTANT(LM_VERTDOCK); // @const win32ui|LM_VERTDOCK|Vertical Docked Dimensions
	ADD_CONSTANT(LM_LENGTHY); // @const win32ui|LM_LENGTHY|Set if nLength is a Height instead of a Width
	ADD_CONSTANT(LM_COMMIT); // @const win32ui|LM_COMMIT|Remember MRUWidth

/**
	ADD_CONSTANT();
***/
	return rc;
}

extern bool CheckGoodWinApp();
extern HINSTANCE hWin32uiDll; // Handle to this DLL, from dllmain.cpp

/* Initialize this module. */
extern "C" __declspec(dllexport) void
initwin32ui(void)
{
  if (!CheckGoodWinApp()) {
	  PyErr_SetString(PyExc_RuntimeError, "The win32ui module could not initialize the application object.");
	  return;
  }
  PyWinGlobals_Ensure();
  PyObject *dict, *module;
  module = Py_InitModule(uiModName, ui_functions);
  dict = PyModule_GetDict(module);
  ui_module_error = PyString_FromString(errorName);
  PyDict_SetItemString(dict, "error", ui_module_error);
  PyObject *copyright = PyString_FromString("Copyright 1994-2000 Mark Hammond (MarkH@ActiveState.com)");
  PyDict_SetItemString(dict, "copyright", copyright);
  Py_XDECREF(copyright);
  PyObject *dllhandle = PyInt_FromLong((long)hWin32uiDll);
  PyDict_SetItemString(dict, "dllhandle", dllhandle);
  Py_XDECREF(dllhandle);
  // Ensure we have a __file__ attribute (Python itself normally
  // adds one, but if this is called not as part of the standard
  // import process, we dont have one!
  char pathName[MAX_PATH];
  GetModuleFileName(hWin32uiDll, pathName, sizeof(pathName)/sizeof(pathName[0]));
  PyObject *obPathName = PyString_FromString(pathName);
  PyDict_SetItemString(dict, "__file__", obPathName);
  Py_XDECREF(obPathName);

  HookWindowsMessages();	// need to be notified of certain events...
  AddConstants(dict);

  // Add all the types.
  PyObject *typeDict = PyDict_New();
  POSITION pos = ui_type_CObject::typemap->GetStartPosition();
  while (pos) {
	  CRuntimeClass *pRT;
	  ui_type_CObject *pT;
	  ui_type_CObject::typemap->GetNextAssoc(pos, pRT, pT);
	  PyObject *typeName = PyString_FromString(pT->tp_name);
	  PyDict_SetItem(typeDict, typeName, (PyObject *)pT);
	  Py_XDECREF(typeName);
  }
  PyObject *mapName = PyString_FromString("types");
  PyDict_SetItem(dict,mapName, typeDict);
  Py_XDECREF(mapName);
  Py_XDECREF(typeDict);
}

// Utilities for glue support.
BOOL Win32uiInitInstance()
{
	CVirtualHelper helper("InitInstance", GetApp(), VEH_DISPLAY_DIALOG);
	int rc = 0;
	if (helper.HaveHandler() && (!helper.call() || !helper.retval(rc))) {
		// An error here is pretty critical - so we display the traceback dialog.
//		AfxMessageBox("A Python error prevented the application from initializing");
	}
	return (rc==0);
}

// Run is the last thing _exited_.  During the Run call the ExitInstance call
// is made.  Whoever calls "Run" must call Win32uiFinalize after.
int Win32uiRun(void)
{ 
	int ret = -1;
	// An error here is too late for anything to usefully print it,
	// so we use a dialog.
	CVirtualHelper helper("Run", GetApp(), VEH_DISPLAY_DIALOG);
	if (!helper.HaveHandler())
		ret = GetApp()->CWinApp::Run();
	else {
		helper.call();
		helper.retval(ret);
	}
	return ret;
}

static PyThreadState *threadStateSave = NULL;

void Win32uiFinalize()
{
	// These are primarily here as a debugging aid.  Destroy what I created
	// to help MFC detect useful memory leak reports
	ui_assoc_object::handleMgr.cleanup();

	if (threadStateSave)
		PyEval_RestoreThread(threadStateSave);

	if (pHostGlue && pHostGlue->bShouldFinalizePython) {
		Py_Finalize();
	}
	bInFatalShutdown = TRUE;
}

int Win32uiExitInstance(void)
{
	int ret = 0;
	CVirtualHelper helper("ExitInstance", GetApp(), VEH_DISPLAY_DIALOG);
	if (helper.call()) {
		helper.retval(ret);
	}
	return ret;
}

BOOL Win32uiPreTranslateMessage(MSG *pMsg)
{
	BOOL ret = FALSE;
	switch (pMsg->message) {
		case WM_CHAR:
			ret=Python_check_key_message(pMsg);
			break;
		// this message is (seem to be!) trapped by both the hook and this, so
		// no need to waste lookups.
		case WM_MDIGETACTIVE:
			break;
// These 3 are also handled by both, but the hook only works for MFC Windows?
//		case WM_SYSKEYDOWN:
//		case WM_SYSKEYUP:
//		case WM_SYSCHAR:
//			break;
		default:
			ret=Python_check_message(pMsg);
	}
	return ret;
}

BOOL Win32uiOnIdle( LONG lCount )
{
	CVirtualHelper helper("OnIdle", GetApp());
	if (!helper.call(lCount)) return FALSE;
	int ret;
	if (!helper.retval(ret))
		return FALSE;
	return ret;
}

extern "C" PYW_EXPORT BOOL Win32uiApplicationInit(Win32uiHostGlue *pGlue, char *cmd, const char *additionalPaths)
{
#ifdef _DEBUG
	afxDump.SetDepth(1); // deep dump of objects at exit.
	bool bDebug = true;
#else
	bool bDebug = false;
#endif
	// We need to ensure that _this_ instance of
	// win32ui is attached to Python - otherwise there is
	// a risk that when Python does "import win32ui", it
	// will locate a different one, causing obvious grief!
	initwin32ui();

	// Set sys.argv if not already done!
	PyObject *argv = PySys_GetObject("argv");
	if (argv==NULL && __argv!=NULL && __argc > 0)
		PySys_SetArgv(__argc-1, __argv+1);
	// If the versions of the .h file are not in synch, then we are in trouble!
	if (pGlue->versionNo != WIN32UIHOSTGLUE_VERSION) {
		MessageBox(0, "There is a mismatch between version of the application and win32ui.pyd.\n\nIt is likely the application needs to be rebuilt.", "Error", MB_OK);
		return FALSE;
	}

	// Debug/Release mismatch means we are gunna die very soon anyway...
	// (although this is unlikely now Debug/Release have different names!)
	if (pGlue->bDebugBuild != bDebug) {
		MessageBox(0, "There is a mismatch between the Debug/Release versions of the application and win32ui.pyd", "Error", MB_OK);
		return FALSE;
	}

	// set up the glue class.
/****
	if (PyWin_MainModuleThreadState==NULL)
		PyWin_MainModuleThreadState = AfxGetModuleThreadState();
****/
	pGlue->pfnInitInstance = Win32uiInitInstance;
	pGlue->pfnExitInstance = Win32uiExitInstance;
	pGlue->pfnOnCmdMsg = Python_OnCmdMsg;
	pGlue->pfnPreTranslateMessage = Win32uiPreTranslateMessage;
	pGlue->pfnOnIdle = Win32uiOnIdle;
	pGlue->pfnRun = Win32uiRun;
	pGlue->pfnFinalize = Win32uiFinalize;
	pHostGlue = pGlue;
	if (additionalPaths)
		Python_addpath(additionalPaths);
	if (cmd!=NULL) {
		if (Python_run_command_with_log(cmd, NULL))
			return FALSE;
	} // Processing cmd.

	if (pGlue->bShouldAbandonThreadState) {
		// Abandon the thread state, saved until Finalize().
		threadStateSave = PyEval_SaveThread();
	}

	return TRUE;
}
