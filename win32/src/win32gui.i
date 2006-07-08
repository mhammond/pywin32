/* File : win32gui.i */
// @doc

%ifdef WINXPGUI
%module winxpgui 
%else
%module win32gui // A module which provides an interface to the native win32
                 // GUI API.<nl>Note that a module <o winxpgui> also exists, 
                 // which has the same methods as win32gui, but has an XP
                 // manifest and is setup for side-by-side sharing support for
                 // certain system DLLs, notably commctl32.
%endif

%{
#define _WIN32_IE 0x0501 // to enable balloon notifications in Shell_NotifyIcon
#define _WIN32_WINNT 0x0501
#ifdef WINXPGUI
// This changes the entire world for XP!
#define ISOLATION_AWARE_ENABLED 1
#endif

%}
%include "typemaps.i"
%include "pywintypes.i"

%{
#undef PyHANDLE
#include "pywinobjects.h"
#include "winuser.h"
#include "commctrl.h"
#include "windowsx.h" // For edit control hacks.
#include "malloc.h"

#ifdef MS_WINCE
#include "winbase.h"
#define IS_INTRESOURCE(res) (((DWORD)(res) & 0xffff0000) == 0)
#endif

#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);
typedef BOOL (WINAPI *SetLayeredWindowAttributesfunc)(HWND, COLORREF, BYTE,DWORD);
static SetLayeredWindowAttributesfunc pfnSetLayeredWindowAttributes=NULL;
typedef BOOL (WINAPI *GetLayeredWindowAttributesfunc)(HWND, COLORREF *, BYTE *, DWORD *);
static GetLayeredWindowAttributesfunc pfnGetLayeredWindowAttributes=NULL;

static PyObject *g_AtomMap = NULL; // Mapping class atoms to Python WNDPROC
static PyObject *g_HWNDMap = NULL; // Mapping HWND to Python WNDPROC
static PyObject *g_DLGMap = NULL;  // Mapping Dialog HWND to Python WNDPROC

static	HWND	hDialogCurrent = NULL;	// see MS TID Q71450 and PumpMessages for this

extern HGLOBAL MakeResourceFromDlgList(PyObject *tmpl);
extern PyObject *MakeDlgListFromResource(HGLOBAL res);
HINSTANCE g_dllhandle;

static PyObject *logger = NULL;

void HandleError(char *prefix)
{
	BOOL do_stderr = TRUE;
	if (logger) {
		PyObject *exc_typ = NULL, *exc_val = NULL, *exc_tb = NULL;
		PyErr_Fetch( &exc_typ, &exc_val, &exc_tb);

		PyObject *kw = PyDict_New();
		PyObject *exc_info = Py_BuildValue("OOO", exc_typ, exc_val, exc_tb);
		if (kw)
			PyDict_SetItemString(kw, "exc_info", exc_info);
		Py_XDECREF(exc_info);
		PyObject *args = Py_BuildValue("(s)", prefix);
		PyObject *method = PyObject_GetAttrString(logger, "error");
		PyObject *result = NULL;
		if (method && kw && args)
			result = PyObject_Call(method, args, kw);
		Py_XDECREF(method);
		Py_XDECREF(kw);
		Py_XDECREF(args);
		if (result) {
			do_stderr = FALSE;
			Py_DECREF(result);
		}
	}
	if (do_stderr) {
		PySys_WriteStderr(prefix);
		PySys_WriteStderr("\n");
		PyErr_Print();
	}
}
%}

// Written to the module init function.
%init %{
PyEval_InitThreads(); /* Start the interpreter's thread-awareness */
PyDict_SetItemString(d, "dllhandle", PyLong_FromVoidPtr(g_dllhandle));
PyDict_SetItemString(d, "error", PyWinExc_ApiError);

// hack borrowed from win32security since version of SWIG we use doesn't do keyword arguments
#ifdef WINXPGUI
for (PyMethodDef *pmd = winxpguiMethods; pmd->ml_name; pmd++)
#else
for (PyMethodDef *pmd = win32guiMethods; pmd->ml_name; pmd++)
#endif
	if (strcmp(pmd->ml_name, "SetLayeredWindowAttributes")==0 ||
		strcmp(pmd->ml_name, "GetLayeredWindowAttributes")==0 ||
		strcmp(pmd->ml_name, "GetOpenFileNameW")==0 ||
		strcmp(pmd->ml_name, "GetSaveFileNameW")==0 ||
		strcmp(pmd->ml_name, "SystemParametersInfo")==0)
		pmd->ml_flags = METH_VARARGS | METH_KEYWORDS;

HMODULE hmodule=GetModuleHandle("user32.dll");
if (hmodule==NULL)
	hmodule=LoadLibrary("user32.dll");
if (hmodule){
	pfnSetLayeredWindowAttributes=(SetLayeredWindowAttributesfunc)GetProcAddress(hmodule,"SetLayeredWindowAttributes");
	pfnGetLayeredWindowAttributes=(GetLayeredWindowAttributesfunc)GetProcAddress(hmodule,"GetLayeredWindowAttributes");
	}

%}

%{
#ifdef MS_WINCE
typedef HANDLE HINST_ARG;
// WinCE gives a compile error this with dllexport
#define DECLSPEC_DLLMAIN
#else
typedef HINSTANCE HINST_ARG;
#define DECLSPEC_DLLMAIN __declspec(dllexport)
#endif

extern "C" DECLSPEC_DLLMAIN BOOL WINAPI DllMain(HINST_ARG hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if ( dwReason == DLL_PROCESS_ATTACH )
		g_dllhandle = (HINSTANCE)hInstance;
	return TRUE;
}
%}

// Custom 'exception handlers' for simple types that exist only to
// manage the thread-lock.
%typemap(python,except) int {
    Py_BEGIN_ALLOW_THREADS
    $function
    Py_END_ALLOW_THREADS
}

%apply HCURSOR {long};
typedef long HCURSOR;

%apply HINSTANCE {long};
typedef long HINSTANCE;

%apply HMENU {long};
typedef long HMENU

%apply HICON {long};
typedef long HICON

%apply HBITMAP {long};
typedef long HBITMAP

%apply HGDIOBJ {long};
typedef long HGDIOBJ

%apply HWND {long};
typedef long HWND

%apply HFONT {long};
typedef long HFONT

%apply HDC {long};
typedef long HDC

%apply HBRUSH {long};
typedef long HBRUSH

%apply HPEN {long};
typedef long HPEN

%apply HRGN {long};
typedef long HRGN

%apply HIMAGELIST {long};
typedef long HIMAGELIST

%apply HACCEL {long};
typedef long HACCEL

%apply COLORREF {long};
typedef long COLORREF

typedef long WPARAM;
typedef long LPARAM;
typedef long LRESULT;
typedef int UINT;

%typedef void *NULL_ONLY

%typemap(python,in) NULL_ONLY {
	if ($source != Py_None) {
		PyErr_SetString(PyExc_TypeError, "This param must be None");
		return NULL;
	}
	$target = NULL;
}

%typemap(python,ignore) MSG *OUTPUT(MSG temp)
{
  $target = &temp;
  memset($target, 0, sizeof(MSG));
}

%typemap(python,argout) MSG *OUTPUT{
    PyObject *o;
    o = Py_BuildValue("iiiii(ii)",
					$source->hwnd,
					$source->message,
					$source->wParam,
					$source->lParam,
					$source->time,
					$source->pt.x,
					$source->pt.y);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

%typemap(python,in) MSG *INPUT {
    $target = (MSG *)_alloca(sizeof(MSG));
    if (!PyArg_ParseTuple($source, "iiiii(ii):MSG param for $name",
            &$target->hwnd,
            &$target->message,
            &$target->wParam,
            &$target->lParam,
            &$target->time,
            &$target->pt.x,
            &$target->pt.y)) {
        return NULL;
    }
}
%typemap(python,ignore) RECT *OUTPUT(RECT temp)
{
  $target = &temp;
}

%typemap(python,in) RECT *INPUT {
    RECT r;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &r.left, &r.top, &r.right, &r.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
		}
		$target = &r;
	} else {
		return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
	}
}

%typemap(python,in) RECT *INPUT_NULLOK {
    RECT r;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &r.left, &r.top, &r.right, &r.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers or None", "$name");
		}
		$target = &r;
	} else {
		if ($source == Py_None) {
            $target = NULL;
        } else {
            PyErr_SetString(PyExc_TypeError, "This param must be a tuple of four integers or None");
            return NULL;
		}
	}
}

%typemap(python,in) struct HRGN__ *NONE_ONLY {
 /* Currently only allow NULL as a value -- I don't know of the
    'right' way to do this.   DAA 1/9/2000 */
	if ($source == Py_None) {
        $target = NULL;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: This HRGN must currently be None", "$name");
	}
}

%typemap(python,argout) RECT *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("llll", $source->left, $source->top, $source->right, $source->bottom);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

%typemap(python,in) RECT *BOTH = RECT *INPUT;
%typemap(python,argout) RECT *BOTH = RECT *OUTPUT;

%typemap(python,argout) POINT *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("ll", $source->x, $source->y);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

%typemap(python,ignore) POINT *OUTPUT(POINT temp)
{
  $target = &temp;
}

%typemap(python,in) POINT *INPUT {
    POINT r;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "ll", &r.x, &r.y) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: a POINT must be a tuple of integers", "$name");
		}
		$target = &r;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a POINT must be a tuple of integers", "$name");
	}
}


%typemap(python,in) POINT *BOTH = POINT *INPUT;
%typemap(python,argout) POINT *BOTH = POINT *OUTPUT;

%typemap(python,in) SIZE *INPUT {
    SIZE s;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "ll", &s.cx, &s.cy) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: a SIZE must be a tuple of integers", "$name");
		}
		$target = &s;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a SIZE must be a tuple of integers", "$name");
	}
}

%typemap(python,in) ICONINFO *INPUT {
    ICONINFO s;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "lllll", &s.fIcon, &s.xHotspot, &s.yHotspot,
                                               &s.hbmMask, &s.hbmColor) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: a ICONINFO must be a tuple of integers", "$name");
		}
		$target = &s;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a ICONINFO must be a tuple of integers", "$name");
	}
}

%typemap(python,argout) ICONINFO *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("lllll", $source->fIcon, $source->xHotspot,
	                 $source->yHotspot, $source->hbmMask, $source->hbmColor);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

%typemap(python,ignore) ICONINFO *OUTPUT(ICONINFO temp)
{
  $target = &temp;
}

%typemap(python,in) BLENDFUNCTION *INPUT {
    BLENDFUNCTION bf;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "bbbb:" "$name" " tuple",
                             &bf.BlendOp, &bf.BlendFlags,
                             &bf.SourceConstantAlpha, &bf.AlphaFormat) == 0) {
            return NULL;
		}
		$target = &bf;
	} else {
		return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
	}
}

%typemap(python,argout) PAINTSTRUCT *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("(ll(iiii)lls#)",
                $source->hdc,
                $source->fErase,
                $source->rcPaint.left, $source->rcPaint.top, $source->rcPaint.right, $source->rcPaint.bottom,
                $source->fRestore,
                $source->fIncUpdate,
                (char *)$source->rgbReserved,
                sizeof($source->rgbReserved));
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

%typemap(python,ignore) PAINTSTRUCT *OUTPUT(PAINTSTRUCT temp)
{
  $target = &temp;
}

%typemap(python,in) PAINTSTRUCT *INPUT {
    PAINTSTRUCT r;
    char *szReserved;
    int lenReserved;
	if (PyTuple_Check($source)) {
		if (!PyArg_ParseTuple($source,
                             "ll(iiii)lls#",
                            &r.hdc,
                            &r.fErase,
                            &r.rcPaint.left, &r.rcPaint.top, &r.rcPaint.right, &r.rcPaint.bottom,
                            &r.fRestore,
                            &r.fIncUpdate,
                            &szReserved,
                            &lenReserved)) {
			return NULL;
		}
        if (lenReserved != sizeof(r.rgbReserved))
            return PyErr_Format(PyExc_ValueError, "%s: last element must be string of %d bytes",
                                "$name", sizeof(r.rgbReserved));
        memcpy(&r.rgbReserved, szReserved, sizeof(r.rgbReserved));
		$target = &r;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a PAINTSTRUCT must be a tuple", "$name");
	}
}

// @object TRACKMOUSEEVENT|A tuple of (dwFlags, hwndTrack, dwHoverTime)
%typemap(python,in) TRACKMOUSEEVENT *INPUT {
    TRACKMOUSEEVENT e;
	e.cbSize = sizeof e;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "lll", &e.dwFlags, &e.hwndTrack, &e.dwHoverTime) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: a TRACKMOUSEEVENT must be a tuple of 3 integers", "$name");
		}
		$target = &e;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a TRACKMOUSEEVENT must be a tuple of 3 integers", "$name");
	}
}

%typemap(python,except) LRESULT {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
}

%typemap(python,except) BOOL {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
}

%typemap(python,except) HWND, HDC, HMENU, HICON, HBITMAP, HIMAGELIST {
      Py_BEGIN_ALLOW_THREADS
      SetLastError(0);
      $function
      Py_END_ALLOW_THREADS
      DWORD le;
      if ($source==0 && (le=GetLastError())) {
           $cleanup
           return PyWin_SetAPIError("$name", le);
      }
}

%{

#ifdef STRICT
#define MYWNDPROC WNDPROC
#else
#define MYWNDPROC FARPROC
#endif

// Returns TRUE if a call was made (and the rc is in the param)
// Returns FALSE if nothing could be done (so the caller should probably
// call its default)
// NOTE: assumes thread state already acquired.
BOOL PyWndProc_Call(PyObject *obFuncOrMap, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *prc)
{
	// oldWndProc may be:
	//  NULL : Call DefWindowProc
	//  -1   : Assumed a dialog proc, and returns FALSE
	// else  : A valid WndProc to call.

	PyObject *obFunc = NULL;
	if (obFuncOrMap!=NULL) {
		if (PyDict_Check(obFuncOrMap)) {
			PyObject *key = PyInt_FromLong(uMsg);
			obFunc = PyDict_GetItem(obFuncOrMap, key);
			Py_DECREF(key);
		} else {
			obFunc = obFuncOrMap;
		}
	}
	if (obFunc==NULL) {
		PyErr_Clear();
		return FALSE;
	}
	// We are dispatching to Python...
	PyObject *args = Py_BuildValue("llll", hWnd, uMsg, wParam, lParam);
	PyObject *ret = PyObject_CallObject(obFunc, args);
	Py_DECREF(args);
	LRESULT rc = 0;
	if (ret) {
		if (ret != Py_None) // can remain zero for that!
			rc = PyInt_AsLong(ret);
		Py_DECREF(ret);
	}
	else
		HandleError("Python WNDPROC handler failed");
	*prc = rc;
	return TRUE;
}

LRESULT CALLBACK PyWndProcClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PyObject *obFunc = (PyObject *)GetClassLong( hWnd, 0);
	LRESULT rc = 0;
	CEnterLeavePython _celp;
	if (!PyWndProc_Call(obFunc, hWnd, uMsg, wParam, lParam, &rc)) {
		_celp.release();
		rc = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return rc;
}

LRESULT CALLBACK PyDlgProcClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PyObject *obFunc = (PyObject *)GetClassLong( hWnd, 0);
	LRESULT rc = 0;
	CEnterLeavePython _celp;
	if (!PyWndProc_Call(obFunc, hWnd, uMsg, wParam, lParam, &rc)) {
		_celp.release();
		rc = DefDlgProc(hWnd, uMsg, wParam, lParam);
	}
	return rc;
}

LRESULT CALLBACK PyWndProcHWND(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CEnterLeavePython _celp;
	PyObject *key = PyInt_FromLong((long)hWnd);
	PyObject *obInfo = PyDict_GetItem(g_HWNDMap, key);
	Py_DECREF(key);
	MYWNDPROC oldWndProc = NULL;
	PyObject *obFunc = NULL;
	if (obInfo!=NULL) { // Is one of ours!
		obFunc = PyTuple_GET_ITEM(obInfo, 0);
		PyObject *obOldWndProc = PyTuple_GET_ITEM(obInfo, 1);
		oldWndProc = (MYWNDPROC)PyInt_AsLong(obOldWndProc);
	}
	LRESULT rc = 0;
	if (!PyWndProc_Call(obFunc, hWnd, uMsg, wParam, lParam, &rc))
		if (oldWndProc) {
			_celp.release();
			rc = CallWindowProc(oldWndProc, hWnd, uMsg, wParam, lParam);
		}

#ifdef WM_NCDESTROY
	if (uMsg==WM_NCDESTROY) {
#else // CE doesnt have this message!
	if (uMsg==WM_DESTROY) {
#endif
		_celp.acquire(); // in case we released above - safe if already acquired.
		PyObject *key = PyInt_FromLong((long)hWnd);
		if (PyDict_DelItem(g_HWNDMap, key) != 0)
			PyErr_Clear();
		Py_DECREF(key);
	}
	return rc;
}

BOOL CALLBACK PyDlgProcHDLG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL rc = FALSE;
	CEnterLeavePython _celp;
	if (uMsg==WM_INITDIALOG) {
		// The lparam is our PyObject.
		// Put our HWND in the map.
		PyObject *obTuple = (PyObject *)lParam;
		PyObject *obWndProc = PyTuple_GET_ITEM(obTuple, 0);
		// Replace the lParam with the one the user specified.
		lParam = PyInt_AsLong( PyTuple_GET_ITEM(obTuple, 1) );
		PyObject *key = PyInt_FromLong((long)hWnd);
		if (g_DLGMap==NULL)
			g_DLGMap = PyDict_New();
		if (g_DLGMap)
			PyDict_SetItem(g_DLGMap, key, obWndProc);
		Py_DECREF(key);
		// obWndProc has no reference.
		rc = TRUE;
	} else if(uMsg == WM_ACTIVATE) {	// see MS TID Q71450 and PumpMessages
		if(0 == wParam)
			hDialogCurrent = NULL;
		else
			hDialogCurrent = hWnd;
	}
	// If our HWND is in the map, then call it.
	PyObject *obFunc = NULL;
	if (g_DLGMap) {
		PyObject *key = PyInt_FromLong((long)hWnd);
		obFunc = PyDict_GetItem(g_DLGMap, key);
		Py_XDECREF(key);
		if (!obFunc)
			PyErr_Clear();
	}
	if (obFunc) {
		LRESULT lrc;
		if (PyWndProc_Call(obFunc, hWnd, uMsg, wParam, lParam, &lrc))
			rc = (BOOL)lrc;
	}

#ifdef WM_NCDESTROY
	if (uMsg==WM_NCDESTROY) {
#else // CE doesnt have this message!
	if (uMsg==WM_DESTROY) {
#endif
		PyObject *key = PyInt_FromLong((long)hWnd);

		if (g_DLGMap != NULL)
			if (PyDict_DelItem(g_DLGMap, key) != 0)
				PyErr_Clear();
		Py_DECREF(key);
	}
	return rc;
}

#include "structmember.h"

// Support for a WNDCLASS object.
class PyWNDCLASS : public PyObject
{
public:
	WNDCLASS *GetWC() {return &m_WNDCLASS;}

	PyWNDCLASS(void);
	~PyWNDCLASS();

	/* Python support */
	static PyObject *meth_SetDialogProc(PyObject *self, PyObject *args);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

	WNDCLASS m_WNDCLASS;
	PyObject *m_obMenuName, *m_obClassName, *m_obWndProc;
};
#define PyWNDCLASS_Check(ob)	((ob)->ob_type == &PyWNDCLASSType)

// @object PyWNDCLASS|A Python object, representing an WNDCLASS structure
// @comm Typically you create a PyWNDCLASS object, and set its properties.
// The object can then be passed to any function which takes an WNDCLASS object
PyTypeObject PyWNDCLASSType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyWNDCLASS",
	sizeof(PyWNDCLASS),
	0,
	PyWNDCLASS::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyWNDCLASS::getattr,				/* tp_getattr */
	PyWNDCLASS::setattr,				/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyWNDCLASS, e)

/*static*/ struct memberlist PyWNDCLASS::memberlist[] = {
	{"style",            T_INT,  OFF(m_WNDCLASS.style)}, // @prop integer|style|
//	{"cbClsExtra",       T_INT,  OFF(m_WNDCLASS.cbClsExtra)}, // @prop integer|cbClsExtra|
	{"cbWndExtra",       T_INT,  OFF(m_WNDCLASS.cbWndExtra)}, // @prop integer|cbWndExtra|
	{"hInstance",        T_INT,  OFF(m_WNDCLASS.hInstance)}, // @prop integer|hInstance|
	{"hIcon",            T_INT,  OFF(m_WNDCLASS.hIcon)}, // @prop integer|hIcon|
	{"hCursor",          T_INT,  OFF(m_WNDCLASS.hCursor)}, // @prop integer|hCursor|
	{"hbrBackground",    T_INT,  OFF(m_WNDCLASS.hbrBackground)}, // @prop integer|hbrBackground|
	{NULL}	/* Sentinel */
	// These 3 handled manually in PyWNDCLASS::getattr/setattr.  The pymeth below is used as an
	// end tag, so these props will be lost if below it
	// @prop string/<o PyUnicode>|lpszMenuName|
	// @prop string/<o PyUnicode>|lpszClassName|
	// @prop function|lpfnWndProc|

};

static PyObject *meth_SetDialogProc(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SetDialogProc"))
		return NULL;
	PyWNDCLASS *pW = (PyWNDCLASS *)self;
	pW->m_WNDCLASS.lpfnWndProc = (WNDPROC)PyDlgProcClass;
	Py_INCREF(Py_None);
	return Py_None;
}

static struct PyMethodDef PyWNDCLASS_methods[] = {
	{"SetDialogProc",     meth_SetDialogProc, 1}, 	// @pymeth SetDialogProc|Sets the WNDCLASS to be for a dialog box.
	// @pymethod |PyWNDCLASS|SetDialogProc|Sets the WNDCLASS to be for a dialog box
	{NULL}
};


PyWNDCLASS::PyWNDCLASS()
{
	ob_type = &PyWNDCLASSType;
	_Py_NewReference(this);
	memset(&m_WNDCLASS, 0, sizeof(m_WNDCLASS));
	m_WNDCLASS.cbClsExtra = sizeof(PyObject *);
	m_WNDCLASS.lpfnWndProc = PyWndProcClass;
	m_obMenuName = m_obClassName = m_obWndProc = NULL;
}

PyWNDCLASS::~PyWNDCLASS(void)
{
	Py_XDECREF(m_obMenuName);
	Py_XDECREF(m_obClassName);
	Py_XDECREF(m_obWndProc);
}

PyObject *PyWNDCLASS::getattr(PyObject *self, char *name)
{
	PyObject *ret = Py_FindMethod(PyWNDCLASS_methods, self, name);
	if (ret != NULL)
		return ret;
	PyErr_Clear();
	PyWNDCLASS *pW = (PyWNDCLASS *)self;
	if (strcmp("lpszMenuName", name)==0) {
		ret = pW->m_obMenuName ? pW->m_obMenuName : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	if (strcmp("lpszClassName", name)==0) {
		ret = pW->m_obClassName ? pW->m_obClassName : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	if (strcmp("lpfnWndProc", name)==0) {
		ret = pW->m_obWndProc ? pW->m_obWndProc : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	return PyMember_Get((char *)self, memberlist, name);
}

int SetTCHAR(PyObject *v, PyObject **m, LPCTSTR *ret)
{
#ifdef UNICODE
	if (!PyUnicode_Check(v)) {
		PyErr_SetString(PyExc_TypeError, "Object must be a Unicode");
		return -1;
	}
	Py_XDECREF(*m);
	*m = v;
	Py_INCREF(v);
	*ret = PyUnicode_AsUnicode(v);
	return 0;
#else
	if (!PyString_Check(v)) {
		PyErr_SetString(PyExc_TypeError, "Object must be a string");
		return -1;
	}
	Py_XDECREF(*m);
	*m = v;
	Py_INCREF(v);
	*ret = PyString_AsString(v);
	return 0;
#endif
}
int PyWNDCLASS::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete WNDCLASS attributes");
		return -1;
	}
	PyWNDCLASS *pW = (PyWNDCLASS *)self;
	if (strcmp("lpszMenuName", name)==0) {
		return SetTCHAR(v, &pW->m_obMenuName, &pW->m_WNDCLASS.lpszMenuName);
	}
	if (strcmp("lpszClassName", name)==0) {
		return SetTCHAR(v, &pW->m_obClassName, &pW->m_WNDCLASS.lpszClassName);
	}
	if (strcmp("lpfnWndProc", name)==0) {
		if (!PyCallable_Check(v) && !PyDict_Check(v)) {
			PyErr_SetString(PyExc_TypeError, "lpfnWndProc must be callable, or a dictionary");
			return -1;
		}
		Py_XDECREF(pW->m_obWndProc);
		pW->m_obWndProc = v;
		Py_INCREF(v);
		return 0;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyWNDCLASS::deallocFunc(PyObject *ob)
{
	delete (PyWNDCLASS *)ob;
}

static PyObject *MakeWNDCLASS(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return new PyWNDCLASS();
}

%}
%native (WNDCLASS) MakeWNDCLASS;

%{
// Support for a BITMAP object.
class PyBITMAP : public PyObject
{
public:
	BITMAP *GetBM() {return &m_BITMAP;}
	PyBITMAP(void);
	PyBITMAP(const BITMAP *pBM);
	~PyBITMAP();

	/* Python support */
	static void deallocFunc(PyObject *ob);
	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )
	BITMAP m_BITMAP;
};
#define PyBITMAP_Check(ob)	((ob)->ob_type == &PyBITMAPType)

// @object PyBITMAP|A Python object, representing an PyBITMAP structure
// @comm Typically you get one of these from GetObject.  Note that currently 
// the bitmap bits are not exposed via this type - but the value of the 
// pointer is.  You can use the struct and win32gui functions to unpack 
// these bits manually if you really need them.
// Note that you are still responsible for the life of the win32 bitmap object.
// The object can then be passed to any function which takes an BITMAP object
PyTypeObject PyBITMAPType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyBITMAP",
	sizeof(PyBITMAP),
	0,
	PyBITMAP::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyBITMAP::getattr,				/* tp_getattr */
	PyBITMAP::setattr,				/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};
#undef OFF
#define OFF(e) offsetof(PyBITMAP, e)

/*static*/ struct memberlist PyBITMAP::memberlist[] = {
	{"bmType",           T_LONG,  OFF(m_BITMAP.bmType)}, // @prop integer|bmType|
	{"bmWidth",           T_LONG,  OFF(m_BITMAP.bmWidth)}, // @prop integer|bmWidth|
	{"bmHeight",           T_LONG,  OFF(m_BITMAP.bmHeight)}, // @prop integer|bmHeight|
	{"bmWidthBytes",     T_LONG,  OFF(m_BITMAP.bmWidthBytes)}, // @prop integer|bmWidthBytes|
	{"bmPlanes",           T_SHORT,  OFF(m_BITMAP.bmPlanes)}, // @prop integer|bmPlanes|
	{"bmBitsPixel",           T_SHORT,  OFF(m_BITMAP.bmBitsPixel)}, // @prop integer||
	{NULL}
};


PyBITMAP::PyBITMAP()
{
	ob_type = &PyBITMAPType;
	_Py_NewReference(this);
	memset(&m_BITMAP, 0, sizeof(m_BITMAP));
}

PyBITMAP::PyBITMAP(const BITMAP *pBM)
{
	ob_type = &PyBITMAPType;
	_Py_NewReference(this);
	memcpy(&m_BITMAP, pBM, sizeof(m_BITMAP));
}

PyBITMAP::~PyBITMAP(void)
{
}

PyObject *PyBITMAP::getattr(PyObject *self, char *name)
{
	PyBITMAP *pB = (PyBITMAP *)self;
	if (strcmp("bmBits", name)==0) {
		return PyLong_FromVoidPtr(pB->m_BITMAP.bmBits);
	}
	return PyMember_Get((char *)self, memberlist, name);
}

int PyBITMAP::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete BITMAP attributes");
		return -1;
	}
	if (strcmp("bmBits", name)==0) {
		PyBITMAP *pB = (PyBITMAP *)self;
		pB->m_BITMAP.bmBits = PyLong_AsVoidPtr(v);
		return PyErr_Occurred() ? -1 : 0;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyBITMAP::deallocFunc(PyObject *ob)
{
	delete (PyBITMAP *)ob;
}

// Support for a LOGFONT object.
class PyLOGFONT : public PyObject
{
public:
	LOGFONT *GetLF() {return &m_LOGFONT;}

	PyLOGFONT(void);
	PyLOGFONT(const LOGFONT *pLF);
	~PyLOGFONT();

	/* Python support */

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

	LOGFONT m_LOGFONT;
};
#define PyLOGFONT_Check(ob)	((ob)->ob_type == &PyLOGFONTType)

// @object PyLOGFONT|A Python object, representing an PyLOGFONT structure
// @comm Typically you create a PyLOGFONT object, and set its properties.
// The object can then be passed to any function which takes an LOGFONT object
PyTypeObject PyLOGFONTType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyLOGFONT",
	sizeof(PyLOGFONT),
	0,
	PyLOGFONT::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyLOGFONT::getattr,				/* tp_getattr */
	PyLOGFONT::setattr,				/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};
#undef OFF
#define OFF(e) offsetof(PyLOGFONT, e)

/*static*/ struct memberlist PyLOGFONT::memberlist[] = {
	{"lfHeight",           T_LONG,  OFF(m_LOGFONT.lfHeight)}, // @prop integer|lfHeight|
	{"lfWidth",            T_LONG,  OFF(m_LOGFONT.lfWidth)}, // @prop integer|lfWidth|
	{"lfEscapement",       T_LONG,  OFF(m_LOGFONT.lfEscapement)}, // @prop integer|lfEscapement|
	{"lfOrientation",      T_LONG,  OFF(m_LOGFONT.lfOrientation)}, // @prop integer|lfOrientation|
	{"lfWeight",           T_LONG,  OFF(m_LOGFONT.lfWeight)}, // @prop integer|lfWeight|
	{"lfItalic",           T_BYTE,  OFF(m_LOGFONT.lfItalic)}, // @prop integer|lfItalic|
	{"lfUnderline",        T_BYTE,  OFF(m_LOGFONT.lfUnderline)}, // @prop integer|lfUnderline|
	{"lfStrikeOut",        T_BYTE,  OFF(m_LOGFONT.lfStrikeOut)}, // @prop integer|lfStrikeOut|
	{"lfCharSet",          T_BYTE,  OFF(m_LOGFONT.lfCharSet)}, // @prop integer|lfCharSet|
	{"lfOutPrecision",     T_BYTE,  OFF(m_LOGFONT.lfOutPrecision)}, // @prop integer|lfOutPrecision|
	{"lfClipPrecision",    T_BYTE,  OFF(m_LOGFONT.lfClipPrecision)}, // @prop integer|lfClipPrecision|
	{"lfQuality",          T_BYTE,  OFF(m_LOGFONT.lfQuality)}, // @prop integer|lfQuality|
	{"lfPitchAndFamily",   T_BYTE,  OFF(m_LOGFONT.lfPitchAndFamily)}, // @prop integer|lfPitchAndFamily|
	{"lfFaceName",         T_LONG, 0}, // @prop string|lfFaceName|Name of the typeface, at most 31 characters
	{NULL}	/* Sentinel */
};


PyLOGFONT::PyLOGFONT()
{
	ob_type = &PyLOGFONTType;
	_Py_NewReference(this);
	memset(&m_LOGFONT, 0, sizeof(m_LOGFONT));
}

PyLOGFONT::PyLOGFONT(const LOGFONT *pLF)
{
	ob_type = &PyLOGFONTType;
	_Py_NewReference(this);
	memcpy(&m_LOGFONT, pLF, sizeof(m_LOGFONT));
}

PyLOGFONT::~PyLOGFONT(void)
{
}

PyObject *PyLOGFONT::getattr(PyObject *self, char *name)
{
	PyLOGFONT *pL = (PyLOGFONT *)self;
	if (strcmp("lfFaceName", name)==0) {
		return PyWinObject_FromTCHAR(pL->m_LOGFONT.lfFaceName);
	}
	return PyMember_Get((char *)self, memberlist, name);
}

int PyLOGFONT::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete LOGFONT attributes");
		return -1;
	}
	if (strcmp("lfFaceName", name)==0) {
		PyLOGFONT *pL = (PyLOGFONT *)self;
		TCHAR *face;
		DWORD facesize;
		if (!PyWinObject_AsTCHAR(v, &face, FALSE, &facesize))
			return -1;
		if (facesize >= LF_FACESIZE){	// LF_FACESIZE includes the trailing NULL
			PyErr_Format(PyExc_ValueError, "lfFaceName must be less than %d characters", LF_FACESIZE);
			PyWinObject_FreeTCHAR(face);
			return -1;
			}
		_tcsncpy( pL->m_LOGFONT.lfFaceName, face, LF_FACESIZE );
		PyWinObject_FreeTCHAR(face);
		return 0;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyLOGFONT::deallocFunc(PyObject *ob)
{
	delete (PyLOGFONT *)ob;
}

static PyObject *MakeLOGFONT(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return new PyLOGFONT();
}

BOOL PyWinObject_AsLOGFONT(PyObject *ob, LOGFONT *plf)
{
	if (!PyLOGFONT_Check(ob)){
		PyErr_SetString(PyExc_TypeError, "Object must be a PyLOGFONT");
		return FALSE;
		}
	*plf=*((PyLOGFONT *)ob)->GetLF();
	return TRUE;
}

BOOL CALLBACK EnumFontFamProc(const LOGFONT FAR *lpelf, const TEXTMETRIC *lpntm, DWORD FontType, LPARAM lParam)
{
	PyObject *obFunc;
	PyObject *obExtra;
	if (!PyArg_ParseTuple((PyObject *)lParam, "OO", &obFunc, &obExtra))
		return 0;
	PyObject *FontArg = new PyLOGFONT(lpelf);
	PyObject *params = Py_BuildValue("OOiO", FontArg, Py_None, FontType, obExtra);
	Py_XDECREF(FontArg);

	long iret = 0;
	PyObject *ret = PyObject_CallObject(obFunc, params);
	Py_XDECREF(params);
	if (ret) {
		iret = PyInt_AsLong(ret);
		Py_DECREF(ret);
	}
	return iret;
}

// @pyswig int|EnumFontFamilies|Enumerates the available font families.
static PyObject *PyEnumFontFamilies(PyObject *self, PyObject *args)
{
	PyObject *obFamily;
	PyObject *obProc;
	PyObject *obExtra = Py_None;
	long hdc;
	// @pyparm int|hdc||Handle to a device context for which to enumerate available fonts
	// @pyparm string/<o PyUnicode>|Family||Family of fonts to enumerate. If none, first member of each font family will be returned.
	// @pyparm function|EnumFontFamProc||The Python function called with each font family. This function is called with 4 arguments.
	// @pyparm object|Param||An arbitrary object to be passed to the callback function
	// @comm The parameters that the callback function will receive are as follows:<nl>
	//	<o PyLOGFONT> - contains the font parameters<nl>
	//	None - Placeholder for a TEXTMETRIC structure, not supported yet<nl>
	//	int - Font type, combination of DEVICE_FONTTYPE, RASTER_FONTTYPE, TRUETYPE_FONTTYPE<nl>
	//	object - The Param originally passed in to EnumFontFamilies

	if (!PyArg_ParseTuple(args, "lOO|O", &hdc, &obFamily, &obProc, &obExtra))
		return NULL;
	if (!PyCallable_Check(obProc)) {
		PyErr_SetString(PyExc_TypeError, "The 3rd argument must be callable");
		return NULL;
	}
	TCHAR *szFamily;
	if (!PyWinObject_AsTCHAR(obFamily, &szFamily, TRUE))
		return NULL;
	PyObject *lparam = Py_BuildValue("OO", obProc, obExtra);
	int rc = EnumFontFamilies((HDC)hdc, szFamily, EnumFontFamProc, (LPARAM)lparam);
	Py_XDECREF(lparam);
	PyWinObject_FreeTCHAR(szFamily);
	return PyInt_FromLong(rc);

}
%}

%{
// @pyswig |set_logger|Sets a logger object for exceptions and error information
// @comm Once a logger has been set for the module, unhandled exceptions, such as
// from a window's WNDPROC, will be written (via logger.exception()) to the log
// instead of to stderr.
// <nl>Note that using this with the Python 2.3 logging package will prevent the
// traceback from being written to the log.  However, it is possible to use
// the Python 2.4 logging package directly with Python 2.3
PyObject *set_logger(PyObject *self, PyObject *args)
{
	Py_XDECREF(logger);
	logger = NULL;
	// @pyparm object|logger||A logger object, generally from the standard logger package.
	if (!PyArg_ParseTuple(args, "O|set_logger", &logger))
		return NULL;
	if (logger==Py_None)
		logger = NULL;
	Py_XINCREF(logger);
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (set_logger) set_logger;

%typemap(python,in) LOGFONT *{
	if (!PyLOGFONT_Check($source))
		return PyErr_Format(PyExc_TypeError, "Must be a LOGFONT object (got %s)",
		                    $source->ob_type->tp_name);
	$target = &(((PyLOGFONT *)$source)->m_LOGFONT);
}

// @pyswig <o PyLOGFONT>|LOGFONT|Creates a LOGFONT object.
%native (LOGFONT) MakeLOGFONT;
%native (EnumFontFamilies) PyEnumFontFamilies;

// @pyswig int|CreateFontIndirect|function creates a logical font that has the specified characteristics.
// The font can subsequently be selected as the current font for any device context.
HFONT CreateFontIndirect(LOGFONT *lf);	// @pyparm <o PyLOGFONT>|lplf||A LOGFONT object as returned by <om win32gui.LOGFONT> 

%{
// @pyswig object|GetObject|
static PyObject *PyGetObject(PyObject *self, PyObject *args)
{
	long hob;
	// @pyparm int|handle||Handle to the object.
	if (!PyArg_ParseTuple(args, "l", &hob))
		return NULL;
	DWORD typ = GetObjectType((HGDIOBJ)hob);
	// @comm The result depends on the type of the handle.
	// For example, if the handle identifies a Font, a <o LOGFONT> object
	// is returned.
	switch (typ) {
		case OBJ_FONT: {
			LOGFONT lf;
			if (GetObject((HGDIOBJ)hob, sizeof(LOGFONT), &lf)==0)
				return PyWin_SetAPIError("GetObject");
			return new PyLOGFONT(&lf);
		}
		case OBJ_BITMAP: {
			BITMAP bm;
			if (GetObject((HGDIOBJ)hob, sizeof(BITMAP), &bm)==0)
				return PyWin_SetAPIError("GetObject");
			return new PyBITMAP(&bm);
		}
		default:
			PyErr_SetString(PyExc_ValueError, "This GDI object type is not supported");
			return NULL;
	}
}
%}
%native (GetObject) PyGetObject;

%{
// @pyswig int|GetObjectType|Returns the type (OBJ_* constant) of a GDI handle
static PyObject *PyGetObjectType(PyObject *self, PyObject *args)
{
	HANDLE h;
	DWORD t;
	// @pyparm int|h||A handle to a GDI object
	if (!PyArg_ParseTuple(args, "l:GetObjectType", &h))
		return NULL;
	t=GetObjectType(h);
	if (t==0)
		return PyWin_SetAPIError("GetObjectType");
	return PyLong_FromUnsignedLong(t);
}
%}
%native (GetObjectType) PyGetObjectType;

%{
// @pyswig object|PyMakeBuffer|Returns a buffer object from addr,len or just len
static PyObject *PyMakeBuffer(PyObject *self, PyObject *args)
{
	long len,addr = 0;
	// @pyparm int|len||length of the buffer object
	// @pyparm int|addr||Address of the memory to reference
	if (!PyArg_ParseTuple(args, "l|l:PyMakeBuffer", &len,&addr))
		return NULL;

	if(0 == addr) 
		return PyBuffer_New(len);
	else
		return PyBuffer_FromMemory((void *) addr, len);

}
%}
%native (PyMakeBuffer) PyMakeBuffer;

%{
// @pyswig object|PyGetString|Returns a string object from an address.
static PyObject *PyGetString(PyObject *self, PyObject *args)
{
	TCHAR *addr = 0;
	int len = -1;
	// @pyparm int|addr||Address of the memory to reference
	// @pyparm int|len||Number of characters to read.  If not specified, the
	// string must be NULL terminated.
	if (!PyArg_ParseTuple(args, "l|i:PyGetString",&addr, &len))
		return NULL;

	if (len==-1)
		len = _tcslen(addr);

    if (len == 0) return PyUnicodeObject_FromString("");
    if (IsBadReadPtr(addr, len)) {
        PyErr_SetString(PyExc_ValueError,
                        "The value is not a valid address for reading");
        return NULL;
    }
    return PyWinObject_FromTCHAR(addr, len);
}
%}
%native (PyGetString) PyGetString;

%{
// @pyswig object|PySetString|Copies a string to an address (null terminated).
// You almost certainly should use <om win32gui.PySetMemory> instead.
static PyObject *PySetString(PyObject *self, PyObject *args)
{
	TCHAR *addr = 0;
	PyObject *str;
	TCHAR *source;
	long maxLen = 0;

	// @pyparm int|addr||Address of the memory to reference 
	// @pyparm str|String||The string to copy
	// @pyparm int|maxLen||Maximum number of chars to copy (optional)
	if (!PyArg_ParseTuple(args, "lO|l:PySetString",&addr,&str,&maxLen))
		return NULL;

	if (!PyWinObject_AsTCHAR(str, &source)) {
		PyErr_SetString(PyExc_TypeError,"String must by string type");
		return NULL;
	}

    if (!maxLen)
        maxLen = _tcslen(source)+1;

    if (IsBadWritePtr(addr, maxLen)) {
        PyErr_SetString(PyExc_ValueError,
                        "The value is not a valid address for writing");
        return NULL;
    }
    _tcsncpy( addr, source, maxLen);
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (PySetString) PySetString;

%{
// @pyswig object|PySetMemory|Copies bytes to an address.
static PyObject *PySetMemory(PyObject *self, PyObject *args)
{
	long addr;
	char *src;
	int nbytes;

	// @pyparm int|addr||Address of the memory to reference 
	// @pyparm string or buffer|String||The string to copy
	if (!PyArg_ParseTuple(args, "ls#:PySetMemory",&addr,&src,&nbytes))
		return NULL;

	if (IsBadWritePtr((void *)addr, nbytes)) {
		PyErr_SetString(PyExc_ValueError,
		                "The value is not a valid address for writing");
		return NULL;
	}
	memcpy( (void *)addr, src, nbytes);
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (PySetMemory) PySetMemory;



%{
// @pyswig object|PyGetArraySignedLong|Returns a signed long from an array object at specified index
static PyObject *PyGetArraySignedLong(PyObject *self, PyObject *args)
{
	PyObject *ob;
	int offset;
	int maxlen;

	// @pyparm array|array||array object to use
	// @pyparm int|index||index of offset
	if (!PyArg_ParseTuple(args, "Oi:PyGetArraySignedLong",&ob,&offset))
		return NULL;

	PyBufferProcs *pb = ob->ob_type->tp_as_buffer;
	if (pb != NULL && pb->bf_getreadbuffer) {
		long *l;
		maxlen = pb->bf_getreadbuffer(ob,0,(void **) &l);
		if(-1 == maxlen) {
			PyErr_SetString(PyExc_ValueError,"Could not get array address");
			return NULL;
		}
		if(offset * sizeof(*l) > (unsigned)maxlen) {
			PyErr_SetString(PyExc_ValueError,"array index out of bounds");
			return NULL;
		}
		return PyInt_FromLong(l[offset]);

	} else {
			PyErr_SetString(PyExc_TypeError,"array passed is not an array");
			return NULL;
	}
}
%}
%native (PyGetArraySignedLong) PyGetArraySignedLong;

%{
// @pyswig object|PyGetBufferAddressAndLen|Returns a buffer object address and len
static PyObject *PyGetBufferAddressAndLen(PyObject *self, PyObject *args)
{
	PyObject *O = NULL;
	void *addr = NULL;
	int len = 0;

	// @pyparm int|obj||the buffer object
	if (!PyArg_ParseTuple(args, "O:PyGetBufferAddressAndLen", &O))
		return NULL;

	if(!PyBuffer_Check(O)) {
		PyErr_SetString(PyExc_TypeError,"item must be a buffer type");
		return NULL;
	}

	PyBufferProcs *pb = O->ob_type->tp_as_buffer;
	if (NULL != pb  && NULL != pb->bf_getreadbuffer) 
		len = pb->bf_getreadbuffer(O,0,&addr);

	if(NULL == addr) {
		PyErr_SetString(PyExc_ValueError,"Could not get buffer address");
		return NULL;
	}
	return Py_BuildValue("ll",(long) addr, len);
}
%}
%native (PyGetBufferAddressAndLen) PyGetBufferAddressAndLen;


%typedef TCHAR *STRING_OR_ATOM
%typedef TCHAR *STRING_OR_ATOM_CW

%typemap(python,in) STRING_OR_ATOM, STRING_OR_ATOM_CW {
	if (PyWinObject_AsTCHAR($source, &$target, TRUE))
		;
	else { 
		PyErr_Clear();
		if (PyInt_Check($source))
			$target = (LPTSTR) PyInt_AsLong($source);
		else {
			return PyErr_Format(PyExc_TypeError, 
			                    "Must pass an integer or a string (got '%s')",
			                    $source->ob_type->tp_name);
		}
	}
}

// A hack for CreateWindow - need to post-process...
%typemap(python,freearg) STRING_OR_ATOM_CW {
	if (PyUnicode_Check($target) || PyString_Check($target))
		PyWinObject_FreeTCHAR($source);
	else {
		// A HUGE HACK - set the class extra bytes.
		if (_result) {
			PyObject *obwc = PyDict_GetItem(g_AtomMap, PyInt_FromLong((ATOM)$source));
			if (obwc)
				SetClassLong(_result, 0, (long)((PyWNDCLASS *)obwc)->m_obWndProc);
		}
	}
}

%typedef TCHAR *RESOURCE_ID

%typemap(python,in) RESOURCE_ID {
#ifdef UNICODE
	if (PyUnicode_Check($source)) {
		if (!PyWinObject_AsTCHAR($source, &$target, TRUE))
			return NULL;
	}
#else
	if (PyString_Check($source)) {
		$target = PyString_AsString($source);
	}
#endif
	else {
		if (PyInt_Check($source))
			$target = MAKEINTRESOURCE(PyInt_AsLong($source));
	}
}

%typemap(python,freearg) RESOURCE_ID {
#ifdef UNICODE
	if (PyUnicode_Check($target))
		PyWinObject_FreeTCHAR($source);
#else
	if (PyString_Check($target))
		;
#endif
	else 
		;
}

#ifndef MS_WINCE
// @pyswig int|FlashWindow|The FlashWindow function flashes the specified window one time. It does not change the active state of the window.
// @pyparm int|hwnd||
// @pyparm int|bInvert||
BOOL FlashWindow(HWND hwnd, BOOL bInvert);
// @pyswig int|FlashWindowEx|The FlashWindowEx function flashes the specified window a specified number of times.

%{
PyObject *PyFlashWindowEx(PyObject *self, PyObject *args)
{
	PyObject *ret;
	BOOL rc;
	FLASHWINFO f;
	f.cbSize = sizeof f;
	// @pyparm int|hwnd||
	// @pyparm int|dwFlags||
	// @pyparm int|uCount||
	// @pyparm int|dwTimeout||
	if (!PyArg_ParseTuple(args, "iiii", &f.hwnd, &f.dwFlags, &f.uCount, &f.dwTimeout))
		return NULL;
    // not on NT
	HMODULE hmod = GetModuleHandle(_T("user32"));
    BOOL (WINAPI *pfnFW)(PFLASHWINFO) = NULL;
    if (hmod)
        pfnFW = (BOOL (WINAPI *)(PFLASHWINFO))GetProcAddress(hmod, "FlashWindowEx");
    if (pfnFW==NULL)
        return PyErr_Format(PyExc_NotImplementedError,
                            "FlashWindowsEx is not supported on this version of windows");
	Py_BEGIN_ALLOW_THREADS
	rc = (*pfnFW)(&f);
	Py_END_ALLOW_THREADS
	ret = rc ? Py_True : Py_False;
	Py_INCREF(ret);
	return ret;
}
%}
%native(FlashWindowEx) PyFlashWindowEx;
#endif // MS_WINCE

// To avoid LoadLibrary etc (ie, keep my life simple) for functions
// that don't exist on NT, only put them in winxpgui.
%ifdef WINXPGUI

// @pyswig |AnimateWindow|Enables you to produce special effects when showing or hiding windows. There are three types of animation: roll, slide, and alpha-blended fade.
// @comm To avoid complications with Windows NT, this function only exists in winxpgui (not win32gui)
BOOLAPI AnimateWindow(
  HWND hwnd,     // @pyparm int|hwnd||handle to window
  DWORD dwTime,  // @pyparm int|dwTime||duration of animation
  DWORD dwFlags  // @pyparm int|dwFlags||animation type
);

// @pyswig |UpdateLayeredWindow|Updates the position, size, shape, content, and translucency of a layered window. 
// @comm To avoid complications with Windows NT, this function only exists in winxpgui (not win32gui)
BOOLAPI UpdateLayeredWindow(
  HWND hwnd,             // @pyparm int|hwnd||handle to layered window
  HDC hdcDst,            // @pyparm int|hdcDst||handle to screen DC
  POINT *INPUT,         // @pyparm (x,y)|pointDest||new screen position
  SIZE *INPUT,           // @pyparm (cx, cy)|size||new size of the layered window
  HDC hdcSrc,            // @pyparm int|hdcSrc||handle to surface DC
  POINT *INPUT,         // @pyparm (x,y)|pointSrc||layer position
  COLORREF crKey,        // @pyparm int|colorKey||color key
  BLENDFUNCTION *INPUT, // @pyparm (int, int, int, int)|blend||blend function
  DWORD dwFlags          // @pyparm int|flags||options
);

%endif // End of winxpgui only functions


// @pyswig int|GetWindowLong|
// @pyparm int|hwnd||
// @pyparm int|index||
long GetWindowLong(HWND hwnd, int index);

// @pyswig int|GetClassLong|
// @pyparm int|hwnd||
// @pyparm int|index||
long GetClassLong(HWND hwnd, int index);

// @pyswig int|SetWindowLong|
%{
static PyObject *PySetWindowLong(PyObject *self, PyObject *args)
{
	HWND hwnd;
	int index;
	PyObject *ob;
	long l;
	// @pyparm int|hwnd||The handle to the window
	// @pyparm int|index||The index of the item to set.
	// @pyparm object|value||The value to set.
	if (!PyArg_ParseTuple(args, "liO", &hwnd, &index, &ob))
		return NULL;
	switch (index) {
		// @comm If index is GWL_WNDPROC, then the value parameter
		// must be a callable object (or a dictionary) to use as the
		// new window procedure.
		case GWL_WNDPROC:
		{
			if (!PyCallable_Check(ob) && !PyDict_Check(ob)) {
				PyErr_SetString(PyExc_TypeError, "object must be callable or a dictionary");
				return NULL;
			}
			if (g_HWNDMap==NULL)
				g_HWNDMap = PyDict_New();

			PyObject *key = PyInt_FromLong((long)hwnd);
			PyObject *value = Py_BuildValue("Ol", ob, GetWindowLong(hwnd, GWL_WNDPROC));
			PyDict_SetItem(g_HWNDMap, key, value);
			Py_DECREF(value);
			Py_DECREF(key);
			l = (long)PyWndProcHWND;
			break;
		}
		default:
			if (!PyInt_Check(ob)) {
				return PyErr_Format(PyExc_TypeError, 
				                    "object must be an integer (got '%s')",
				                    ob->ob_type->tp_name);
			}
			l = PyInt_AsLong(ob);
	}
	long ret = SetWindowLong(hwnd, index, l);
	return PyInt_FromLong(ret);
}
%}
%native (SetWindowLong) PySetWindowLong;

// @pyswig int|CallWindowProc|
%{
static PyObject *PyCallWindowProc(PyObject *self, PyObject *args)
{
	long wndproc, hwnd, wparam, lparam;
	UINT msg;
        // @pyparm int|wndproc||The wndproc to call - this is generally the return
        // value of SetWindowLong(GWL_WNDPROC)
        // @pyparm int|hwnd||
        // @pyparm int|msg||
        // @pyparm int|wparam||
        // @pyparm int|lparam||
	if (!PyArg_ParseTuple(args, "llill", &wndproc, &hwnd, &msg, &wparam, &lparam))
		return NULL;
	LRESULT rc;
    Py_BEGIN_ALLOW_THREADS
	rc = CallWindowProc((MYWNDPROC)wndproc, (HWND)hwnd, msg, wparam, lparam);
    Py_END_ALLOW_THREADS
	return PyInt_FromLong(rc);
}
%}
%native (CallWindowProc) PyCallWindowProc;

%typemap(python,in) WPARAM {
   if (!make_param($source, (long *)&$target))
       return NULL;
}

%typemap(python,in) LPARAM {
   if (!make_param($source, (long *)&$target))
       return NULL;
}

%{
static BOOL make_param(PyObject *ob, long *pl)
{
	long &l = *pl;
	if (ob==NULL || ob==Py_None)
		l = 0;
	else
#ifdef UNICODE
#define TCHAR_DESC "Unicode"
	if (PyUnicode_Check(ob))
		l = (long)PyUnicode_AsUnicode(ob);
#else
#define TCHAR_DESC "String"	
	if (PyString_Check(ob))
		l = (long)PyString_AsString(ob);
#endif
	else if (PyInt_Check(ob))
		l = PyInt_AsLong(ob);
	else {
		PyBufferProcs *pb = ob->ob_type->tp_as_buffer;
		if (pb != NULL && pb->bf_getreadbuffer) {
			if(-1 == pb->bf_getreadbuffer(ob,0,(void **) &l))
				return FALSE;
		} else {
			PyErr_SetString(PyExc_TypeError, "Must be a" TCHAR_DESC ", int, or buffer object");
			return FALSE;
		}
	}
	return TRUE;
}

// @pyswig int|SendMessage|Sends a message to the window.
// @pyparm int|hwnd||The handle to the Window
// @pyparm int|message||The ID of the message to post
// @pyparm int|wparam|0|An integer whose value depends on the message
// @pyparm int|lparam|0|An integer whose value depends on the message
static PyObject *PySendMessage(PyObject *self, PyObject *args)
{
	long hwnd;
	PyObject *obwparam=NULL, *oblparam=NULL;
	UINT msg;
	if (!PyArg_ParseTuple(args, "li|OO", &hwnd, &msg, &obwparam, &oblparam))
		return NULL;
	long wparam, lparam;
	if (!make_param(obwparam, &wparam))
		return NULL;
	if (!make_param(oblparam, &lparam))
		return NULL;

	LRESULT rc;
    Py_BEGIN_ALLOW_THREADS
	rc = SendMessage((HWND)hwnd, msg, wparam, lparam);
    Py_END_ALLOW_THREADS

	return PyInt_FromLong(rc);
}
%}
%native (SendMessage) PySendMessage;

%{
// @pyswig int,int|SendMessageTimeout|Sends a message to the window.
// @pyparm int|hwnd||The handle to the Window
// @pyparm int|message||The ID of the message to post
// @pyparm int|wparam||An integer whose value depends on the message
// @pyparm int|lparam||An integer whose value depends on the message
// @pyparm int|flags||Send options
// @pyparm int|timeout||Timeout duration in milliseconds.
static PyObject *PySendMessageTimeout(PyObject *self, PyObject *args)
{
	long hwnd;
	PyObject *obwparam, *oblparam;
	UINT msg;
	UINT flags, timeout;
	if (!PyArg_ParseTuple(args, "liOOii", &hwnd, &msg, &obwparam, &oblparam, &flags, &timeout))
		return NULL;
	long wparam, lparam;
	if (!make_param(obwparam, &wparam))
		return NULL;
	if (!make_param(oblparam, &lparam))
		return NULL;

	LRESULT rc;
	DWORD dwresult;
	Py_BEGIN_ALLOW_THREADS
	rc = SendMessageTimeout((HWND)hwnd, msg, wparam, lparam, flags, timeout, &dwresult);
	Py_END_ALLOW_THREADS
	if (rc==0)
		return PyWin_SetAPIError("SendMessageTimeout");
	// @rdesc The result is the result of the SendMessageTimeout call, plus the last 'result' param.
	// If the timeout period expires, a pywintypes.error exception will be thrown,
	// with zero as the error code.  See the Microsoft documentation for more information.
	return Py_BuildValue("ii", rc, dwresult);
}
%}
%native (SendMessageTimeout) PySendMessageTimeout;

// @pyswig |PostMessage|
// @pyparm int|hwnd||The handle to the Window
// @pyparm int|message||The ID of the message to post
// @pyparm int|wparam|0|An integer whose value depends on the message
// @pyparm int|lparam|0|An integer whose value depends on the message
BOOLAPI PostMessage(HWND hwnd, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

// @pyswig |PostThreadMessage|
// @pyparm int|threadId||The ID of the thread to post the message to.
// @pyparm int|message||The ID of the message to post
// @pyparm int|wparam||An integer whose value depends on the message
// @pyparm int|lparam||An integer whose value depends on the message
BOOLAPI PostThreadMessage(DWORD dwThreadId, UINT msg, WPARAM wParam, LPARAM lParam);

#ifndef MS_WINCE
// @pyswig int|ReplyMessage|Used to reply to a message sent through the SendMessage function without returning control to the function that called SendMessage. 
BOOLAPI ReplyMessage(int lResult); // @pyparm int|result||Specifies the result of the message processing. The possible values are based on the message sent.
#endif	/* not MS_WINCE */

// @pyswig int|RegisterWindowMessage|Defines a new window message that is guaranteed to be unique throughout the system. The message value can be used when sending or posting messages.
// @pyparm string/unicode|name||The string
LRESULT RegisterWindowMessage(TCHAR *lpString);

// @pyswig int|DefWindowProc|
// @pyparm int|hwnd||The handle to the Window
// @pyparm int|message||The ID of the message to send
// @pyparm int|wparam||An integer whose value depends on the message
// @pyparm int|lparam||An integer whose value depends on the message
LRESULT DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

%{
struct PyEnumWindowsCallback {
	PyObject *func;
	PyObject *extra;
};

BOOL CALLBACK PyEnumWindowsProc(
  HWND hwnd,      // handle to parent window
  LPARAM lParam   // application-defined value
) {
	BOOL result = TRUE;
	PyEnumWindowsCallback *cb = (PyEnumWindowsCallback *)lParam;
	CEnterLeavePython _celp;
	PyObject *args = Py_BuildValue("(iO)", hwnd, cb->extra);
	PyObject *ret = PyEval_CallObject(cb->func, args);
	Py_XDECREF(args);
	if (ret && PyInt_Check(ret))
		result = PyInt_AsLong(ret);
	Py_XDECREF(ret);
	return result;
}

// @pyswig |EnumWindows|Enumerates all top-level windows on the screen by passing the handle to each window, in turn, to an application-defined callback function. EnumWindows continues until the last top-level window is enumerated or the callback function returns FALSE
static PyObject *PyEnumWindows(PyObject *self, PyObject *args)
{
	BOOL rc;
	PyObject *obFunc, *obOther;
	// @pyparm object|callback||A Python function to be used as the callback.
	// @pyparm object|extra||Any python object - this is passed to the callback function as the second param (first is the hwnd).
	if (!PyArg_ParseTuple(args, "OO", &obFunc, &obOther))
		return NULL;
	if (!PyCallable_Check(obFunc)) {
		PyErr_SetString(PyExc_TypeError, "First param must be a callable object");
		return NULL;
	}
	PyEnumWindowsCallback cb;
	cb.func = obFunc;
	cb.extra = obOther;
    Py_BEGIN_ALLOW_THREADS
	rc = EnumWindows(PyEnumWindowsProc, (LPARAM)&cb);
    Py_END_ALLOW_THREADS
	if (!rc)
		return PyWin_SetAPIError("EnumWindows");
	Py_INCREF(Py_None);
	return Py_None;
}

#ifndef MS_WINCE
// @pyswig |EnumThreadWindows|Enumerates all top-level windows associated with a thread on the screen by passing the handle to each window, in turn, to an application-defined callback function. EnumThreadWindows continues until the last top-level window associated with the thread is enumerated or the callback function returns FALSE
static PyObject *PyEnumThreadWindows(PyObject *self, PyObject *args)
{
	BOOL rc;
	PyObject *obFunc, *obOther;
	DWORD dwThreadId;
	// @pyparm int|dwThreadId||The id of the thread for which the windows need to be enumerated.
	// @pyparm object|callback||A Python function to be used as the callback.
	// @pyparm object|extra||Any python object - this is passed to the callback function as the second param (first is the hwnd).
	if (!PyArg_ParseTuple(args, "lOO", &dwThreadId, &obFunc, &obOther))
		return NULL;
	if (!PyCallable_Check(obFunc)) {
		PyErr_SetString(PyExc_TypeError, "Second param must be a callable object");
		return NULL;
	}
	PyEnumWindowsCallback cb;
	cb.func = obFunc;
	cb.extra = obOther;
    Py_BEGIN_ALLOW_THREADS
	rc = EnumThreadWindows(dwThreadId, PyEnumWindowsProc, (LPARAM)&cb);
    Py_END_ALLOW_THREADS
	if (!rc)
		return PyWin_SetAPIError("EnumThreadWindows");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig |EnumChildWindows|Enumerates the child windows that belong to the specified parent window by passing the handle to each child window, in turn, to an application-defined callback function. EnumChildWindows continues until the last child window is enumerated or the callback function returns FALSE.
static PyObject *PyEnumChildWindows(PyObject *self, PyObject *args)
{
	BOOL rc;
	PyObject *obFunc, *obOther;
	long hwnd;
	// @pyparm int|hwnd||The handle to the window to enumerate.
	// @pyparm object|callback||A Python function to be used as the callback.
	// @pyparm object|extra||Any python object - this is passed to the callback function as the second param (first is the hwnd).
	if (!PyArg_ParseTuple(args, "lOO", &hwnd, &obFunc, &obOther))
		return NULL;
	if (!PyCallable_Check(obFunc)) {
		PyErr_SetString(PyExc_TypeError, "First param must be a callable object");
		return NULL;
	}
	PyEnumWindowsCallback cb;
	cb.func = obFunc;
	cb.extra = obOther;
    Py_BEGIN_ALLOW_THREADS
	rc = EnumChildWindows((HWND)hwnd, PyEnumWindowsProc, (LPARAM)&cb);
    Py_END_ALLOW_THREADS
	if (!rc)
		return PyWin_SetAPIError("EnumChildWindows");
	Py_INCREF(Py_None);
	return Py_None;
}

#endif	/* not MS_WINCE */
%}
%native (EnumWindows) PyEnumWindows;
#ifndef MS_WINCE
%native (EnumThreadWindows) PyEnumThreadWindows;
%native (EnumChildWindows) PyEnumChildWindows;
#endif	/* not MS_WINCE */


// @pyswig int|DialogBox|Creates a modal dialog box.
%{
static PyObject *PyDialogBox(PyObject *self, PyObject *args)
{
	/// XXX - todo - add support for a dialogproc!
	long hinst, hwnd, param=0;
	PyObject *obResId, *obDlgProc;
	BOOL bFreeString = FALSE;
	if (!PyArg_ParseTuple(args, "lOlO|l", &hinst, &obResId, &hwnd, &obDlgProc, &param))
		return NULL;
	LPTSTR resid;
	if (PyInt_Check(obResId))
		resid = (LPTSTR)MAKEINTRESOURCE(PyInt_AsLong(obResId));
	else {
		if (!PyWinObject_AsTCHAR(obResId, &resid)) {
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "Resource ID must be a string or int");
			return NULL;
		}
		bFreeString = TRUE;
	}
	PyObject *obExtra = Py_BuildValue("Ol", obDlgProc, param);

	int rc;
    Py_BEGIN_ALLOW_THREADS
	rc = DialogBoxParam((HINSTANCE)hinst, resid, (HWND)hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
    Py_END_ALLOW_THREADS
	Py_DECREF(obExtra);
	if (bFreeString)
		PyWinObject_FreeTCHAR(resid);
	if (rc==-1)
		return PyWin_SetAPIError("DialogBox");

	return PyInt_FromLong(rc);
}
%}
%native (DialogBox) PyDialogBox;
// @pyswig int|DialogBoxParam|See <om win32gui.DialogBox>
%native (DialogBoxParam) PyDialogBox;



// @pyswig int|DialogBoxIndirect|Creates a modal dialog box from a template, see <om win32ui.CreateDialogIndirect>
%{
static PyObject *PyDialogBoxIndirect(PyObject *self, PyObject *args)
{
	/// XXX - todo - add support for a dialogproc!
	long hinst, hwnd, param=0;
	PyObject *obList, *obDlgProc;
	BOOL bFreeString = FALSE;
	// @pyparm int|hinst||
	// @pyparm object|controlList||
	// @pyparm int|hwnd||
	// @pyparm object|dlgproc||
	// @pyparm int|param|0|
	if (!PyArg_ParseTuple(args, "lOlO|l", &hinst, &obList, &hwnd, &obDlgProc, &param))
		return NULL;
	
	HGLOBAL h = MakeResourceFromDlgList(obList);
	if (h == NULL)
		return NULL;

	PyObject *obExtra = Py_BuildValue("Ol", obDlgProc, param);

	int rc;
    Py_BEGIN_ALLOW_THREADS
	HGLOBAL templ = (HGLOBAL) GlobalLock(h);
	rc = DialogBoxIndirectParam((HINSTANCE)hinst, (const DLGTEMPLATE *) templ, (HWND)hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
	GlobalUnlock(h);
	GlobalFree(h);
    Py_END_ALLOW_THREADS
	Py_DECREF(obExtra);
	if (rc==-1)
		return PyWin_SetAPIError("DialogBoxIndirect");

	return PyInt_FromLong(rc);
}
%}
%native (DialogBoxIndirect) PyDialogBoxIndirect;
// @pyswig int|DialogBoxIndirectParam|See <om win32gui.DialogBoxIndirect>
%native (DialogBoxIndirectParam) PyDialogBoxIndirect;


// @pyswig int|CreateDialogIndirect|Creates a modeless dialog box from a template, see <om win32ui.CreateDialogIndirect>
%{
static PyObject *PyCreateDialogIndirect(PyObject *self, PyObject *args)
{
	/// XXX - todo - add support for a dialogproc!
	long hinst, hwnd, param=0;
	PyObject *obList, *obDlgProc;
	BOOL bFreeString = FALSE;
	// @pyparm int|hinst||
	// @pyparm object|controlList||
	// @pyparm int|hwnd||
	// @pyparm object|dlgproc||
	// @pyparm int|param|0|
	if (!PyArg_ParseTuple(args, "lOlO|l", &hinst, &obList, &hwnd, &obDlgProc, &param))
		return NULL;
	
	HGLOBAL h = MakeResourceFromDlgList(obList);
	if (h == NULL)
		return NULL;

	PyObject *obExtra = Py_BuildValue("Ol", obDlgProc, param);

	HWND rc;
    Py_BEGIN_ALLOW_THREADS
	HGLOBAL templ = (HGLOBAL) GlobalLock(h);
	rc = CreateDialogIndirectParam((HINSTANCE)hinst, (const DLGTEMPLATE *) templ, (HWND)hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
	GlobalUnlock(h);
	GlobalFree(h);
    Py_END_ALLOW_THREADS
	Py_DECREF(obExtra);
	if (NULL == rc)
		return PyWin_SetAPIError("CreateDialogIndirect");

	return PyInt_FromLong((long) rc);

}
%}
%native (CreateDialogIndirect) PyCreateDialogIndirect;
// @pyswig int|DialogBoxIndirectParam|See <om win32gui.CreateDialogIndirect>
%native (CreateDialogIndirectParam) PyCreateDialogIndirect;

// @pyswig |EndDialog|Ends a dialog box.
// @pyparm int|hwnd||Handle to the window.
// @pyparm int|result||result

BOOLAPI EndDialog( HWND hwnd, int result );

// @pyswig HWND|GetDlgItem|Retrieves the handle to a control in the specified dialog box. 
HWND GetDlgItem(
	HWND hDlg,		// @pyparm <o PyHANDLE>|hDlg||Handle to a dialog window
	int nIDDlgItem	// @pyparm int|IDDlgItem||Identifier of one of the dialog's controls
	); 

// @pyswig |GetDlgItemInt|Returns the integer value of a dialog control
%{
static PyObject *PyGetDlgItemInt(PyObject *self, PyObject *args)
{
	BOOL bTranslated, bSigned;
	int id;
	UINT val;
	HWND hDlg;
	PyObject *obhDlg;
	if (!PyArg_ParseTuple(args, "Oii:GetDlgItemInt", 
		&obhDlg,	// @pyparm <o PyHANDLE>|hDlg||Handle to a dialog window
		&id,		// @pyparm int|IDDlgItem||Identifier of one of the dialog's controls
		&bSigned))	// @pyparm boolean|Signed||Indicates whether control value should be interpreted as signed
		return NULL;
	if (!PyWinObject_AsHANDLE(obhDlg, (HANDLE *)&hDlg), FALSE)
		return NULL;

	val=GetDlgItemInt(hDlg, id, &bTranslated, bSigned);
	if (!bTranslated)
		return PyWin_SetAPIError("GetDlgItemInt");
	if (bSigned)
		return PyLong_FromLong(val);
	return PyLong_FromUnsignedLong(val);
}
%}
%native (GetDlgItemInt) PyGetDlgItemInt;

// @pyswig |SetDlgItemInt|Places an integer value in a dialog control
BOOLAPI SetDlgItemInt(
	HWND hDlg,		// @pyparm <o PyHANDLE>|hDlg||Handle to a dialog window
	int nIDDlgItem,	// @pyparm int|IDDlgItem||Identifier of one of the dialog's controls
	UINT uValue,	// @pyparm int|Value||Value to placed in the control
	BOOL bSigned	// @pyparm boolean|Signed||Indicates if the input value is signed
);

// @pyswig int|GetDlgCtrlID|Retrieves the identifier of the specified control.
// @pyparm int|hwnd||The handle to the control
HWND GetDlgCtrlID( HWND hwnd);

// @pyswig string|GetDlgItemText|Returns the text of a dialog control
%native (GetDlgItemText) PyGetDlgItemText;
%{
static PyObject *PyGetDlgItemText(PyObject *self, PyObject *args)
{	
	int dlgitem;
	HWND hwnd;
	TCHAR *buf=NULL;
	UINT chars_returned;
	DWORD chars_allocated=128, bufsize;
	#ifdef Py_DEBUG
	chars_allocated=3;
	#endif
	PyObject *ret=NULL, *obhwnd;
	if (!PyArg_ParseTuple(args, "Oi",
		&obhwnd,	// @pyparm <o PyHANDLE>|hDlg||Handle to a dialog window
		&dlgitem))	// @pyparm int|IDDlgItem||The Id of a control within the dialog
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd, FALSE))
		return NULL;
	// If text is too long for buffer, it's truncated and truncated size returned
	// Loop until fewer characters returned than were allocated
	while(TRUE){
		if (buf!=NULL){
			free(buf);
			chars_allocated*=2;
			}
		bufsize=chars_allocated*sizeof(TCHAR);
		buf=(TCHAR *)malloc(bufsize);
		if (buf==NULL)
			return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		chars_returned=GetDlgItemText(hwnd, dlgitem, buf, chars_allocated);
		if (chars_returned==0){
			PyWin_SetAPIError("GetDlgItemText");
			break;
			}
		// Return count doesn't include trailing NULL
		if (chars_returned+1 < chars_allocated){
			ret=PyWinObject_FromTCHAR(buf, chars_returned);
			break;
			}
		}
	if (buf!=NULL)
		free(buf);
	return ret;
}
%}

// @pyswig |SetDlgItemText|Sets the text for a window or control
BOOLAPI SetDlgItemText(
	HWND hDlg,		// @pyparm <o PyHANDLE>|hDlg||Handle to a dialog window
	int nIDDlgItem,	// @pyparm int|IDDlgItem||The Id of a control within the dialog
	TCHAR *text);	// @pyparm str/unicode|String||The text to put in the control

// @pyswig HWND|GetNextDlgTabItem|Retrieves a handle to the first control that has the WS_TABSTOP style that precedes (or follows) the specified control.
HWND GetNextDlgTabItem(
	HWND hDlg,       // @pyparm int|hDlg||handle to dialog box
	HWND hCtl,       // @pyparm int|hCtl||handle to known control
	BOOL bPrevious); // @pyparm int|bPrevious||direction flag

// @pyswig HWND|GetNextDlgGroupItem|Retrieves a handle to the first control in a group of controls that precedes (or follows) the specified control in a dialog box.
HWND GetNextDlgGroupItem(
	HWND hDlg,       // @pyparm int|hDlg||handle to dialog box
	HWND hCtl,       // @pyparm int|hCtl||handle to known control
	BOOL bPrevious); // @pyparm int|bPrevious||direction flag


// @pyswig |SetWindowText|Sets the window text.
BOOLAPI SetWindowText(HWND hwnd, TCHAR *text);

%{
// @pyswig string|GetWindowText|Get the window text.
static PyObject *PyGetWindowText(PyObject *self, PyObject *args)
{
    HWND hwnd;
    int len;
   
	TCHAR buffer[512];
	// @pyparm int|hwnd||The handle to the window
	if (!PyArg_ParseTuple(args, "l", &hwnd))
		return NULL;
    Py_BEGIN_ALLOW_THREADS
    len = GetWindowText(hwnd, buffer, sizeof(buffer)/sizeof(TCHAR));
    Py_END_ALLOW_THREADS
    // @comm Note that previous versions of PyWin32 returned a (empty) Unicode 
    // object when the string was empty, or an MBCS encoded string value 
    // otherwise.  A String is now returned in all cases.
	return PyWinObject_FromTCHAR(buffer, len);
}
%}
%native (GetWindowText) PyGetWindowText;

int GetWindowTextLength(HWND hwnd);

// @pyswig |InitCommonControls|Initializes the common controls.
void InitCommonControls();

%{
// @pyswig |InitCommonControlsEx|Initializes specific common controls.
static PyObject *PyInitCommonControlsEx(PyObject *self, PyObject *args)
{
	int flag;
	// @pyparm int|flag||One of the ICC_ constants
	if (!PyArg_ParseTuple(args, "i", &flag))
		return NULL;
	INITCOMMONCONTROLSEX cc;
	cc.dwSize = sizeof(cc);
	cc.dwICC = flag;
	if (!InitCommonControlsEx(&cc))
		return PyWin_SetAPIError("InitCommonControlsEx");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (InitCommonControlsEx) PyInitCommonControlsEx;

// @pyswig HCURSOR|LoadCursor|Loads a cursor.
HCURSOR LoadCursor(
	HINSTANCE hInst, // @pyparm int|hinstance||The module to load from
	RESOURCE_ID name // @pyparm int|resid||The resource ID
);

// @pyswig HCURSOR|SetCursor|
HCURSOR SetCursor(
	HCURSOR hc // @pyparm int|hcursor||
);

// @pyswig HCURSOR|GetCursor|
HCURSOR GetCursor();

#ifndef MS_WINCE
%{
// @pyswig flags, hcursor, (x,y)|GetCursorInfo|Retrieves information about the global cursor.
PyObject *PyGetCursorInfo(PyObject *self, PyObject *args)
{
	CURSORINFO ci;
	ci.cbSize = sizeof(ci);
	if (!PyArg_NoArgs(args))
		return NULL;
	if (!::GetCursorInfo(&ci))
		return PyWin_SetAPIError("GetCursorInfo");
	return Py_BuildValue("ii(ii)", ci.flags, ci.hCursor, ci.ptScreenPos.x, ci.ptScreenPos.y);
}
%}
%native(GetCursorInfo) PyGetCursorInfo;
#endif

// @pyswig HACCEL|CreateAcceleratorTable|Creates an accelerator table
%{
PyObject *PyCreateAcceleratorTable(PyObject *self, PyObject *args)
{
    int num, i;
    ACCEL *accels = NULL;
    PyObject *ret = NULL;
    PyObject *obAccels;
    HACCEL ha;
    // @pyparm ( (int, int, int), ...)|accels||A sequence of (fVirt, key, cmd),
    // as per the Win32 ACCEL structure.
    if (!PyArg_ParseTuple(args, "O:CreateAcceleratorTable", &obAccels))
        return NULL;
    if (!PySequence_Check(obAccels))
        return PyErr_Format(PyExc_TypeError, "accels must be a sequence of tuples (got '%s')",
                            obAccels->ob_type->tp_name);
    num = PySequence_Length(obAccels);
    if (num==0) {
        PyErr_SetString(PyExc_ValueError, "Can't create an accelerator with zero items");
        goto done;
    }
    accels = (ACCEL *)malloc(num * sizeof(ACCEL));
    if (!accels) {
        PyErr_NoMemory();
        goto done;
    }
    for (i=0;i<num;i++) {
        ACCEL *p = accels+i;
        PyObject *ob = PySequence_GetItem(obAccels, i);
        if (!ob) goto done;
        if (!PyArg_ParseTuple(ob, "BHH:ACCEL", &p->fVirt, &p->key, &p->cmd)) {
            Py_DECREF(ob);
            goto done;
        }
        Py_DECREF(ob);
    }
    ha = ::CreateAcceleratorTable(accels, num);
    if (ha)
        ret = PyLong_FromVoidPtr((void *)ha);
    else
        PyWin_SetAPIError("CreateAcceleratorTable");
done:
    if (accels)
        free(accels);
    return ret;
}
%}
%native (CreateAcceleratorTable) PyCreateAcceleratorTable;

// @pyswig |DestroyAccleratorTable|Destroys an accelerator table
// @pyparm int|haccel||
BOOLAPI DestroyAcceleratorTable(HACCEL haccel);

// @pyswig HMENU|LoadMenu|Loads a menu
// @pyparm int|hinstance||
// @pyparm int/string|resource_id||
HMENU LoadMenu(HINSTANCE hInst, RESOURCE_ID name);

// @pyswig |DestroyMenu|Destroys a previously loaded menu.
BOOLAPI DestroyMenu( HMENU hmenu );

#ifndef MS_WINCE
// @pyswig |SetMenu|Sets the menu for the specified window.
// @pyparm int|hwnd||
// @pyparm int|hmenu||
BOOLAPI SetMenu( HWND hwnd, HMENU hmenu );
#endif

// @pyswig |GetMenu|Gets the menu for the specified window.
HMENU GetMenu( HWND hwnd);

// @pyswig HCURSOR|LoadIcon|Loads an icon
// @pyparm int|hinstance||
// @pyparm int/string|resource_id||
HICON LoadIcon(HINSTANCE hInst, RESOURCE_ID name);

#ifndef MS_WINCE
// @pyswig HICON|CopyIcon|Copies an icon
// @pyparm int|hicon||Existing icon
HICON CopyIcon(HICON hicon);
#endif

// @pyswig |DrawIcon|Draws an icon or cursor into the specified device context.
// To specify additional drawing options, use the <om win32gui.DrawIconEx> function. 
BOOLAPI DrawIcon(
  HDC hDC,      // @pyparm int|hDC||handle to DC
  int X,        // @pyparm int|X||x-coordinate of upper-left corner
  int Y,        // @pyparm int|Y||y-coordinate of upper-left corner
  HICON hIcon   // @pyparm int|hicon||handle to icon
);

// @pyswig |DrawIconEx|Draws an icon or cursor into the specified device context,
// performing the specified raster operations, and stretching or compressing the
// icon or cursor as specified.
BOOLAPI DrawIconEx(
  HDC hdc,                   // @pyparm int|hDC||handle to device context
  int xLeft,                 // @pyparm int|xLeft||x-coord of upper left corner
  int yTop,                  // @pyparm int|yTop||y-coord of upper left corner
  HICON hIcon,               // @pyparm int|hIcon||handle to icon
  int cxWidth,               // @pyparm int|cxWidth||icon width
  int cyWidth,               // @pyparm int|cyWidth||icon height
  int istepIfAniCur,        // @pyparm int|istepIfAniCur||frame index, animated cursor
  HBRUSH hbrFlickerFreeDraw, // @pyparm int|hbrFlickerFreeDraw||handle to background brush
  int diFlags               // @pyparm int|diFlags||icon-drawing flags
);

// @pyswig int|CreateIconIndirect|Creates an icon or cursor from an ICONINFO structure. 
HICON CreateIconIndirect(ICONINFO *INPUT);

%{
// @pyswig int|CreateIconFromResource|Creates an icon or cursor from resource bits describing the icon.
static PyObject *PyCreateIconFromResource(PyObject *self, PyObject *args)
{
	// @pyparm string|bits||The bits
	// @pyparm bool|fIcon||True if an icon, False if a cursor.
	// @pyparm int|ver|0x00030000|Specifies the version number of the icon or cursor
	// format for the resource bits pointed to by the presbits parameter.
	// This parameter can be 0x00030000.
	char *bits;
	int nBits;
	int isIcon;
	int ver = 0x00030000;
	if (!PyArg_ParseTuple(args, "s#i|i", &bits, &nBits, &isIcon, &ver))
		return NULL;
	HICON ret = CreateIconFromResource((PBYTE)bits, nBits, isIcon, ver);
	if (!ret)
	    return PyWin_SetAPIError("CreateIconFromResource");
	return PyLong_FromVoidPtr(ret);
}
%}
%native (CreateIconFromResource) PyCreateIconFromResource;

// @pyswig HANDLE|LoadImage|Loads a bitmap, cursor or icon
HANDLE LoadImage(HINSTANCE hInst, // @pyparm int|hinst||Handle to an instance of the module that contains the image to be loaded. To load an OEM image, set this parameter to zero. 
				 RESOURCE_ID name, // @pyparm int/string|name||Specifies the image to load. If the hInst parameter is non-zero and the fuLoad parameter omits LR_LOADFROMFILE, name specifies the image resource in the hInst module. If the image resource is to be loaded by name, the name parameter is a string that contains the name of the image resource.
				 UINT type, // @pyparm int|type||Specifies the type of image to be loaded.
				 int cxDesired, // @pyparm int|cxDesired||Specifies the width, in pixels, of the icon or cursor. If this parameter is zero and the fuLoad parameter is LR_DEFAULTSIZE, the function uses the SM_CXICON or SM_CXCURSOR system metric value to set the width. If this parameter is zero and LR_DEFAULTSIZE is not used, the function uses the actual resource width. 
				 int cyDesired, // @pyparm int|cyDesired||Specifies the height, in pixels, of the icon or cursor. If this parameter is zero and the fuLoad parameter is LR_DEFAULTSIZE, the function uses the SM_CYICON or SM_CYCURSOR system metric value to set the height. If this parameter is zero and LR_DEFAULTSIZE is not used, the function uses the actual resource height. 
				 UINT fuLoad); // @pyparm int|fuLoad||

#define	IMAGE_BITMAP	IMAGE_BITMAP
#define	IMAGE_CURSOR	IMAGE_CURSOR
#define	IMAGE_ICON		IMAGE_ICON

#define	LR_DEFAULTCOLOR	LR_DEFAULTCOLOR
#ifndef MS_WINCE
#define	LR_CREATEDIBSECTION	LR_CREATEDIBSECTION
#define	LR_DEFAULTSIZE	LR_DEFAULTSIZE
#define	LR_LOADFROMFILE	LR_LOADFROMFILE
#define	LR_LOADMAP3DCOLORS	LR_LOADMAP3DCOLORS
#define	LR_LOADTRANSPARENT	LR_LOADTRANSPARENT
#define	LR_MONOCHROME	LR_MONOCHROME
#define	LR_SHARED	LR_SHARED
#define	LR_VGACOLOR	LR_VGACOLOR
#endif	/* not MS_WINCE */

// @pyswig |DeleteObject|Deletes a logical pen, brush, font, bitmap, region, or palette, freeing all system resources associated with the object. After the object is deleted, the specified handle is no longer valid.
BOOLAPI DeleteObject(HANDLE h); // @pyparm int|handle||handle to the object to delete.

// @pyswig |BitBlt|Performs a bit-block transfer of the color data corresponding
// to a rectangle of pixels from the specified source device context into a
// destination device context. 
BOOLAPI BitBlt(
  HDC hdcDest, // @pyparm int|hdcDest||handle to destination DC
  int nXDest,  // @pyparm int|x||x-coord of destination upper-left corner
  int nYDest,  // @pyparm int|y||y-coord of destination upper-left corner
  int nWidth,  // @pyparm int|width||width of destination rectangle
  int nHeight, // @pyparm int|height||height of destination rectangle
  HDC hdcSrc,  // @pyparm int|hdcSrc||handle to source DC
  int nXSrc,   // @pyparm int|nXSrc||x-coordinate of source upper-left corner
  int nYSrc,   // @pyparm int|nYSrc||y-coordinate of source upper-left corner
  DWORD dwRop  // @pyparm int|dwRop||raster operation code
);

// @pyswig |StretchBlt|Copies a bitmap from a source rectangle into a destination
// rectangle, stretching or compressing the bitmap to fit the dimensions of the
// destination rectangle, if necessary
BOOLAPI StretchBlt(
  HDC hdcDest,      // @pyparm int|hdcDest||handle to destination DC
  int nXOriginDest, // @pyparm int|x||x-coord of destination upper-left corner
  int nYOriginDest, // @pyparm int|y||y-coord of destination upper-left corner
  int nWidthDest,   // @pyparm int|width||width of destination rectangle
  int nHeightDest,  // @pyparm int|height||height of destination rectangle
  HDC hdcSrc,       // @pyparm int|hdcSrc||handle to source DC
  int nXOriginSrc,  // @pyparm int|nXSrc||x-coord of source upper-left corner
  int nYOriginSrc,  // @pyparm int|nYSrc||y-coord of source upper-left corner
  int nWidthSrc,    // @pyparm int|nWidthSrc||width of source rectangle
  int nHeightSrc,   // @pyparm int|nHeightSrc||height of source rectangle
  DWORD dwRop       // @pyparm int|dwRop||raster operation code
);

#ifndef MS_WINCE
// @pyswig int|SetStretchBltMode|
// @rdesc If the function succeeds, the return value is the previous stretching mode.
// <nl>If the function fails, the return value is zero. 
int SetStretchBltMode(HDC dc, int mode);
// @pyparm int|dc||
// @pyparm int|mode||
#endif	/* not MS_WINCE */

%ifdef WINXPGUI
// @pyswig |MaskBlt|Combines the color data for the source and destination
// bitmaps using the specified mask and raster operation.
// @comm This function is available only in winxpgui, as it is not supported
// on Win9x.
BOOLAPI MaskBlt(
  HDC hdcDest,     // handle to destination DC
  int nXDest,      // x-coord of destination upper-left corner
  int nYDest,      // y-coord of destination upper-left corner 
  int nWidth,      // width of source and destination
  int nHeight,     // height of source and destination
  HDC hdcSrc,      // handle to source DC
  int nXSrc,       // x-coord of upper-left corner of source
  int nYSrc,       // y-coord of upper-left corner of source
  HBITMAP hbmMask, // handle to monochrome bit mask
  int xMask,       // horizontal offset into mask bitmap
  int yMask,       // vertical offset into mask bitmap
  DWORD dwRop      // raster operation code
);
%endif

// @pyswig int|ImageList_Add|Adds an image or images to an image list. 
// @rdesc Returns the index of the first new image if successful, or -1 otherwise. 
int ImageList_Add(HIMAGELIST himl, // @pyparm int|himl||Handle to the image list. 
                  HBITMAP hbmImage, // @pyparm int|hbmImage||Handle to the bitmap that contains the image or images. The number of images is inferred from the width of the bitmap. 
				  HBITMAP hbmMask); // @pyparm int|hbmMask||Handle to the bitmap that contains the mask. If no mask is used with the image list, this parameter is ignored


// @pyswig HIMAGELIST|ImageList_Create|Create an image list
HIMAGELIST ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);


#define	ILC_COLOR	ILC_COLOR
#ifndef MS_WINCE
#define	ILC_COLOR4	ILC_COLOR4
#define	ILC_COLOR8	ILC_COLOR8
#define	ILC_COLOR16	ILC_COLOR16
#define	ILC_COLOR24	ILC_COLOR24
#define	ILC_COLOR32	ILC_COLOR32
#endif	/* not MS_WINCE */
#define	ILC_COLORDDB	ILC_COLORDDB
#define	ILC_MASK	ILC_MASK

// @pyswig BOOL |ImageList_Destroy|Destroy an imagelist
BOOLAPI ImageList_Destroy(HIMAGELIST himl);

// @pyswig BOOL |ImageList_Draw|Draw an image on an HDC
BOOLAPI ImageList_Draw(HIMAGELIST himl,int i,HDC hdcDst, int x, int y, UINT fStyle);

// @pyswig BOOL |ImageList_DrawEx|Draw an image on an HDC
BOOLAPI ImageList_DrawEx(HIMAGELIST himl,int i,HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);

#define	ILD_BLEND25	ILD_BLEND25
#define	ILD_FOCUS	ILD_FOCUS
#define	ILD_BLEND50	ILD_BLEND50
#define	ILD_SELECTED	ILD_SELECTED
#define	ILD_BLEND	ILD_BLEND
#define	ILD_MASK	ILD_MASK
#define	ILD_NORMAL	ILD_NORMAL
#define	ILD_TRANSPARENT	ILD_TRANSPARENT

// @pyswig HICON|ImageList_GetIcon|Extract an icon from an imagelist
HICON ImageList_GetIcon(HIMAGELIST himl, int i, UINT flag);

// @pyswig int|ImageList_GetImageCount|Return count of images in imagelist
int ImageList_GetImageCount(HIMAGELIST himl);

// @pyswig HANDLE|ImageList_LoadImage|Loads bitmaps, cursors or icons, creates imagelist
HIMAGELIST ImageList_LoadImage(HINSTANCE hInst, RESOURCE_ID name,
				 int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);

// @pyswig HANDLE|ImageList_LoadBitmap|Creates an image list from the specified bitmap resource.
HIMAGELIST ImageList_LoadBitmap(HINSTANCE hInst, TCHAR *name,
				 int cx, int cGrow, COLORREF crMask);

// @pyswig BOOL|ImageList_Remove|Remove an image from an imagelist
BOOLAPI ImageList_Remove(HIMAGELIST himl, int i);

// @pyswig BOOL|ImageList_Replace|Replace an image in an imagelist with a bitmap image
int ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);

// @pyswig BOOL|ImageList_ReplaceIcon|Replace an image in an imagelist with an icon image
int ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);

// @pyswig COLORREF|ImageList_SetBkColor|Set the background color for the imagelist
COLORREF ImageList_SetBkColor(HIMAGELIST himl,COLORREF clrbk);

// @pyswig |ImageList_SetOverlayImage|Adds a specified image to the list of images to be used as overlay masks. An image list can have up to four overlay masks in version 4.70 and earlier and up to 15 in version 4.71. The function assigns an overlay mask index to the specified image. 
BOOLAPI ImageList_SetOverlayImage(
    HIMAGELIST himl, // @pyparm int|hImageList||
    int iImage, // @pyparm int|iImage||
    int iOverlay // @pyparm int|iOverlay||
);


#define	CLR_NONE	CLR_NONE

// @pyswig int|MessageBox|Displays a message box
// @pyparm int|parent||The parent window
// @pyparm string/<o PyUnicode>|text||The text for the message box
// @pyparm string/<o PyUnicode>|caption||The caption for the message box
// @pyparm int|flags||
int MessageBox(HWND parent, TCHAR *text, TCHAR *caption, DWORD flags);

// @pyswig |MessageBeep|Plays a waveform sound.
// @pyparm int|type||The type of the beep
BOOLAPI MessageBeep(UINT type);

// @pyswig int|CreateWindow|Creates a new window.
HWND CreateWindow( 
	STRING_OR_ATOM_CW lpClassName, // @pyparm int/string|className||
	TCHAR *INPUT_NULLOK, // @pyparm string|windowTitle||
	DWORD dwStyle, // @pyparm int|style||The style for the window.
	int x,  // @pyparm int|x||
	int y,  // @pyparm int|y||
	int nWidth, // @pyparm int|width||
	int nHeight, // @pyparm int|height||
	HWND hWndParent, // @pyparm int|parent||Handle to the parent window.
	HMENU hMenu, // @pyparm int|menu||Handle to the menu to use for this window.
	HINSTANCE hInstance, // @pyparm int|hinstance||
	NULL_ONLY null // @pyparm None|reserved||Must be None
);

// @pyswig |DestroyWindow|
// @pyparm int|hwnd||The handle to the window
BOOLAPI DestroyWindow(HWND hwnd);

// @pyswig int|EnableWindow|Enables and disables keyboard and mouse input to a window
// @rdesc Returns True if window was already disabled when call was made, False otherwise
BOOL EnableWindow(
	HWND hwnd,	// @pyparm <o PyHANDLE>|hWnd||Handle to window
	BOOL bEnable);	// @pyparm boolean|bEnable||True to enable input to the window, False to disable input

// @pyswig int|FindWindow|Retrieves a handle to the top-level window whose class name and window name match the specified strings.
HWND FindWindow( 
	STRING_OR_ATOM className, // @pyparm int/string|className||
	TCHAR *INPUT_NULLOK); // @pyparm string|WindowName||

#ifndef MS_WINCE
// @pyswig int|FindWindowEx|Retrieves a handle to the top-level window whose class name and window name match the specified strings.
HWND FindWindowEx(
	HWND parent, // @pyparm int|hwnd||
	HWND childAfter, // @pyparm int|childAfter||
	STRING_OR_ATOM className, // @pyparm int/string|className||
	TCHAR *INPUT_NULLOK); // @pyparm string|WindowName||

// @pyswig |DragAcceptFiles|Registers whether a window accepts dropped files.
// @pyparm int|hwnd||Handle to the Window
// @pyparm int|fAccept||Value that indicates if the window identified by the hWnd parameter accepts dropped files.
// This value is True to accept dropped files or False to discontinue accepting dropped files. 
void DragAcceptFiles(HWND hWnd, BOOL fAccept);

// @pyswig |DragDetect|captures the mouse and tracks its movement until the user releases the left button, presses the ESC key, or moves the mouse outside the drag rectangle around the specified point.
// @pyparm int|hwnd||Handle to the Window
// @pyparm (int, int)|point||Initial position of the mouse, in screen coordinates. The function determines the coordinates of the drag rectangle by using this point.
// @rdesc If the user moved the mouse outside of the drag rectangle while holding down the left button , the return value is nonzero.
// <nl>If the user did not move the mouse outside of the drag rectangle while holding down the left button , the return value is zero.
BOOL DragDetect(HWND hWnd, POINT INPUT);

// @pyswig |SetDoubleClickTime|
// @pyparm int|newVal||
BOOLAPI SetDoubleClickTime(UINT val);
#endif	/* not MS_WINCE */

// @pyswig int|GetDoubleClickTime|
UINT GetDoubleClickTime();

// @pyswig |HideCaret|
BOOLAPI HideCaret(HWND hWnd);

// @pyswig |SetCaretPos|
BOOLAPI SetCaretPos(
	int X,  // @pyparm int|x||horizontal position  
	int Y   // @pyparm int|y||vertical position
);

/*BOOLAPI GetCaretPos(
  POINT *lpPoint   // address of structure to receive coordinates
);*/

// @pyswig |ShowCaret|
BOOLAPI ShowCaret(HWND hWnd);

// @pyswig int|ShowWindow|
// @pyparm int|hwnd||The handle to the window
// @pyparm int|cmdShow||
BOOL ShowWindow(HWND hWndMain, int nCmdShow);

// @pyswig int|IsWindowVisible|Indicates if the window has the WS_VISIBLE style.
// @pyparm int|hwnd||The handle to the window
BOOL IsWindowVisible(HWND hwnd);

// @pyswig int|IsWindowEnabled|Indicates if the window is enabled.
// @pyparm int|hwnd||The handle to the window
BOOL IsWindowEnabled(HWND hwnd);

// @pyswig |SetFocus|Sets focus to the specified window.
// @pyparm int|hwnd||The handle to the window
HWND SetFocus(HWND hwnd);

// @pyswig |GetFocus|Returns the HWND of the window with focus.
HWND GetFocus();

// @pyswig |UpdateWindow|
// @pyparm int|hwnd||The handle to the window
BOOLAPI UpdateWindow(HWND hWnd);

// @pyswig |BringWindowToTop|
// @pyparm int|hwnd||The handle to the window
BOOLAPI BringWindowToTop(HWND hWnd);

// @pyswig HWND|SetActiveWindow|
// @pyparm int|hwnd||The handle to the window
HWND SetActiveWindow(HWND hWnd);

// @pyswig HWND|GetActiveWindow|
HWND GetActiveWindow();

// @pyswig HWND|SetForegroundWindow|
// @pyparm int|hwnd||The handle to the window
BOOLAPI SetForegroundWindow(HWND hWnd);

// @pyswig HWND|GetForegroundWindow|
HWND GetForegroundWindow();

// @pyswig (left, top, right, bottom)|GetClientRect|
// @pyparm int|hwnd||The handle to the window
BOOLAPI GetClientRect(HWND hWnd, RECT *OUTPUT);

// @pyswig HDC|GetDC|Gets the device context for the window.
// @pyparm int|hwnd||The handle to the window
HDC GetDC(  HWND hWnd );

// @pyswig |DeleteDC|Deletes a DC
BOOLAPI DeleteDC(
    HDC dc // @pyparm int|hdc||The source DC
);

// @pyswig HDC|CreateCompatibleDC|Creates a memory device context (DC) compatible with the specified device. 
HDC CreateCompatibleDC(
  HDC hdc   // @pyparm int|dc||handle to DC
);

// @pyswig HBITMAP|CreateCompatibleBitmap|Creates a bitmap compatible with the device that is associated with the specified device context. 
HBITMAP CreateCompatibleBitmap(
  HDC hdc,        // @pyparm int|hdc||handle to DC
  int nWidth,     // @pyparm int|width||width of bitmap, in pixels
  int nHeight     // @pyparm int|height||height of bitmap, in pixels
);

// @pyswig HBITMAP|CreateBitmap|Creates a bitmap
HBITMAP CreateBitmap(
  int nWidth,         // @pyparm int|width||bitmap width, in pixels
  int nHeight,        // @pyparm int|height||bitmap height, in pixels
  UINT cPlanes,       // @pyparm int|cPlanes||number of color planes
  UINT cBitsPerPel,   // @pyparm int|cBitsPerPixel||number of bits to identify color
  NULL_ONLY null // @pyparm None|bitmap bits||Must be None
);

// @pyswig HGDIOBJ|SelectObject|Selects an object into the specified device context (DC). The new object replaces the previous object of the same type. 
HGDIOBJ SelectObject(
  HDC hdc,        // @pyparm int|hdc||handle to DC
  HGDIOBJ object     // @pyparm int|object||The GDI object
);

HINSTANCE GetModuleHandle(TCHAR *INPUT_NULLOK);

// @pyswig (left, top, right, bottom)|GetWindowRect|
// @pyparm int|hwnd||The handle to the window
BOOLAPI GetWindowRect(HWND hWnd, RECT *OUTPUT);

// @pyswig int|GetStockObject|
HANDLE GetStockObject(int object);

// @pyswig |PostQuitMessage|
// @pyparm int|rc||
void PostQuitMessage(int rc);

#ifndef MS_WINCE
// @pyswig |WaitMessage|Waits for a message
BOOLAPI WaitMessage();
#endif	/* MS_WINCE */

// @pyswig int|SetWindowPos|
BOOL SetWindowPos(  HWND hWnd,             // handle to window
  HWND hWndInsertAfter,  // placement-order handle
  int X,                 // horizontal position
  int Y,                 // vertical position  
  int cx,                // width
  int cy,                // height
  UINT uFlags            // window-positioning flags
);

%{
// @pyswig tuple|GetWindowPlacement|Returns placement information about the current window.
static PyObject *
PyGetWindowPlacement(PyObject *self, PyObject *args)
{
	int hwnd;
	if (!PyArg_ParseTuple(args, "i:GetWindowPlacement", &hwnd))
		return NULL;

	WINDOWPLACEMENT pment;
	pment.length=sizeof(pment);
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = GetWindowPlacement( (HWND)hwnd, &pment );
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("GetWindowPlacement");
	// @rdesc The result is a tuple of
	// (flags, showCmd, (minposX, minposY), (maxposX, maxposY), (normalposX, normalposY))
	// @flagh Item|Description
	// @flag flags|One of the WPF_* constants
	// @flag showCmd|Current state - one of the SW_* constants.
	// @flag minpos|Specifies the coordinates of the window's upper-left corner when the window is minimized.
	// @flag maxpos|Specifies the coordinates of the window's upper-left corner when the window is maximized. 
	// @flag normalpos|Specifies the window's coordinates when the window is in the restored position.
	return Py_BuildValue("(ii(ii)(ii)(iiii))",pment.flags, pment.showCmd,
	                     pment.ptMinPosition.x,pment.ptMinPosition.y,
	                     pment.ptMaxPosition.x,pment.ptMaxPosition.y,
	                     pment.rcNormalPosition.left, pment.rcNormalPosition.top,
	                     pment.rcNormalPosition.right, pment.rcNormalPosition.bottom);
}
// @pyswig |SetWindowPlacement|Sets the windows placement
static PyObject *
PySetWindowPlacement(PyObject *self, PyObject *args)
{
	int hwnd;
	WINDOWPLACEMENT pment;
	pment.length=sizeof(pment);
	// @pyparm (tuple)|placement||A tuple representing the WINDOWPLACEMENT structure.
	if (!PyArg_ParseTuple(args,"i(ii(ii)(ii)(iiii)):SetWindowPlacement",
	                      &hwnd,
	                      &pment.flags, &pment.showCmd,
	                      &pment.ptMinPosition.x,&pment.ptMinPosition.y,
	                      &pment.ptMaxPosition.x,&pment.ptMaxPosition.y,
	                      &pment.rcNormalPosition.left, &pment.rcNormalPosition.top,
	                      &pment.rcNormalPosition.right, &pment.rcNormalPosition.bottom))
		return NULL;
	BOOL rc;
	Py_BEGIN_ALLOW_THREADS
	rc = SetWindowPlacement( (HWND)hwnd, &pment );
	Py_END_ALLOW_THREADS
	if (!rc)
		return PyWin_SetAPIError("SetWindowPlacement");
	Py_INCREF(Py_None);
	return Py_None;
}

%}
%native (GetWindowPlacement) PyGetWindowPlacement;
%native (SetWindowPlacement) PySetWindowPlacement;

%{
// @pyswig int|RegisterClass|Registers a window class.
static PyObject *PyRegisterClass(PyObject *self, PyObject *args)
{
	PyObject *obwc;
	// @pyparm <o PyWNDCLASS>|wndClass||An object describing the window class.
	if (!PyArg_ParseTuple(args, "O", &obwc))
		return NULL;
	if (!PyWNDCLASS_Check(obwc)) {
		PyErr_SetString(PyExc_TypeError, "The object must be a WNDCLASS object");
		return NULL;
	}
	ATOM at = RegisterClass( &((PyWNDCLASS *)obwc)->m_WNDCLASS );
	if (at==0)
		return PyWin_SetAPIError("RegisterClass");
	// Save the atom in a global dictionary.
	if (g_AtomMap==NULL)
		g_AtomMap = PyDict_New();

	PyObject *key = PyInt_FromLong(at);
	PyDict_SetItem(g_AtomMap, key, obwc);
	return key;
}
%}
%native (RegisterClass) PyRegisterClass;

%{
// @pyswig |UnregisterClass|
static PyObject *PyUnregisterClass(PyObject *self, PyObject *args)
{
	long atom, hinst;
	// @pyparm int|atom||The atom identifying the class previously registered.
	// @pyparm int|hinst||The handle to the instance unregistering the class.
	if (!PyArg_ParseTuple(args, "ll", &atom, &hinst))
		return NULL;

	if (!UnregisterClass((LPCTSTR)atom, (HINSTANCE)hinst))
		return PyWin_SetAPIError("UnregisterClass");

	// Delete the atom from the global dictionary.
	if (g_AtomMap) {
		PyObject *key = PyInt_FromLong(atom);
		PyDict_DelItem(g_AtomMap, key);
		Py_DECREF(key);
	}
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (UnregisterClass) PyUnregisterClass;

%{
// @pyswig |PumpMessages|Runs a message loop until a WM_QUIT message is received.
// @rdesc Returns exit code from PostQuitMessage when a WM_QUIT message is received
static PyObject *PyPumpMessages(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	MSG msg;
	int rc;

    Py_BEGIN_ALLOW_THREADS
	while ((rc=GetMessage(&msg, 0, 0, 0))==1) {
		if(NULL == hDialogCurrent || !IsDialogMessage(hDialogCurrent,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    Py_END_ALLOW_THREADS

	if (-1 == rc)
		return PyWin_SetAPIError("GetMessage");

	return PyInt_FromLong(msg.wParam);

	// @xref <om win32gui.PumpWaitingMessages>
}

// @pyswig int|PumpWaitingMessages|Pumps all waiting messages for the current thread.
// @rdesc Returns non-zero (exit code from PostQuitMessage) if a WM_QUIT message was received, else 0
static PyObject *PyPumpWaitingMessages(PyObject *self, PyObject *args)
{
	UINT firstMsg = 0, lastMsg = 0;
	if (!PyArg_ParseTuple (args, "|ii:PumpWaitingMessages", &firstMsg, &lastMsg))
		return NULL;
	// @pyseeapi PeekMessage and DispatchMessage

    MSG msg;
	long result = 0;
	// Read all of the messages in this next loop, 
	// removing each message as we read it.
	Py_BEGIN_ALLOW_THREADS
	while (PeekMessage(&msg, NULL, firstMsg, lastMsg, PM_REMOVE)) {
		// If it's a quit message, we're out of here.
		if (msg.message == WM_QUIT) {
			if(0 != msg.wParam)
				result = msg.wParam;
			else
				result = 1;
			break;
		}
		// Otherwise, dispatch the message.
		if(NULL == hDialogCurrent || !IsDialogMessage(hDialogCurrent,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} // End of PeekMessage while loop
	// @xref <om win32gui.PumpMessages>
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(result);
}

%}
%native (PumpMessages) PyPumpMessages;
%native (PumpWaitingMessages) PyPumpWaitingMessages;

// @pyswig MSG|GetMessage|
BOOL GetMessage(MSG *OUTPUT, 
                HWND hwnd, // @pyparm int|hwnd||
                UINT min, // @pyparm int|min||
                UINT max); // @pyparm int|max||

// @pyswig int|TranslateMessage|
// @pyparm MSG|msg||
BOOL TranslateMessage(MSG *INPUT);

// @pyswig int|DispatchMessage|
// @pyparm MSG|msg||
LRESULT DispatchMessage(MSG *INPUT);

// @pyswig int|TranslateAccelerator|
int TranslateAccelerator(
    HWND hwnd, // @pyparm int|hwnd||
    HACCEL haccel, // @pyparm int|haccel||
    MSG *INPUT // @pyparm MSG|msg||
);

// @pyswig MSG|PeekMessage|
BOOL PeekMessage(MSG *OUTPUT, 
                 HWND hwnd, // @pyparm int|hwnd||
                 UINT min, // @pyparm int|filterMin||
                 UINT max, // @pyparm int|filterMax||
                 UINT remove); // @pyparm int|removalOptions||

// DELETE ME!
%{
static PyObject *Unicode(PyObject *self, PyObject *args)
{
	char *text;
#if PY_VERSION_HEX > 0x2030300
	PyErr_Warn(PyExc_PendingDeprecationWarning, "win32gui.Unicode will die!");
#endif
	if (!PyArg_ParseTuple(args, "s", &text))
		return NULL;
	return PyUnicodeObject_FromString(text);
}
%}
%native (Unicode) Unicode;

%{
static PyObject *
PyHIWORD(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:HIWORD", &n))
		return NULL;
	return PyInt_FromLong(HIWORD(n));
}
%}
%native (HIWORD) PyHIWORD;

%{
static PyObject *
PyLOWORD(PyObject *self, PyObject *args)
{	int n;
	if(!PyArg_ParseTuple(args, "i:LOWORD", &n))
		return NULL;
	return PyInt_FromLong(LOWORD(n));
}
%}
%native (LOWORD) PyLOWORD;

// Should go in win32sh?
%{
#ifdef MS_WINCE
BOOL PyObject_AsNOTIFYICONDATA(PyObject *ob, NOTIFYICONDATA *pnid)
{
	PyObject *obTip=NULL;
	memset(pnid, 0, sizeof(*pnid));
	pnid->cbSize = sizeof(*pnid);
	if (!PyArg_ParseTuple(ob, "l|iiilO:NOTIFYICONDATA tuple", &pnid->hWnd, &pnid->uID, &pnid->uFlags, &pnid->uCallbackMessage, &pnid->hIcon, &obTip))
		return FALSE;
	if (obTip) {
		TCHAR *szTip;
		if (!PyWinObject_AsTCHAR(obTip, &szTip))
			return NULL;
		_tcsncpy(pnid->szTip, szTip, sizeof(pnid->szTip)/sizeof(TCHAR));
		PyWinObject_FreeTCHAR(szTip);
	}
	return TRUE;
}
#else	// MS_WINCE
BOOL PyObject_AsNOTIFYICONDATA(PyObject *ob, NOTIFYICONDATA *pnid)
{
	PyObject *obTip=NULL, *obInfo=NULL, *obInfoTitle=NULL;
	memset(pnid, 0, sizeof(*pnid));
	pnid->cbSize = sizeof(*pnid);
	if (!PyArg_ParseTuple(ob, "l|iiilOOiOi:NOTIFYICONDATA tuple", 
	                     &pnid->hWnd, &pnid->uID, &pnid->uFlags, 
	                     &pnid->uCallbackMessage, &pnid->hIcon, &obTip, 
	                     &obInfo, &pnid->uTimeout, &obInfoTitle, 
	                     &pnid->dwInfoFlags))
		return FALSE;
	if (obTip) {
		TCHAR *szTip;
		if (!PyWinObject_AsTCHAR(obTip, &szTip))
			return NULL;
		_tcsncpy(pnid->szTip, szTip, sizeof(pnid->szTip)/sizeof(TCHAR));
		PyWinObject_FreeTCHAR(szTip);
	}
	if (obInfo) {
		TCHAR *szInfo;
		if (!PyWinObject_AsTCHAR(obInfo, &szInfo))
			return NULL;
		_tcsncpy(pnid->szInfo, szInfo, sizeof(pnid->szInfo)/sizeof(TCHAR));
		PyWinObject_FreeTCHAR(szInfo);
	}
	if (obInfoTitle) {
		TCHAR *szInfoTitle;
		if (!PyWinObject_AsTCHAR(obInfoTitle, &szInfoTitle))
			return NULL;
		_tcsncpy(pnid->szInfoTitle, szInfoTitle, sizeof(pnid->szInfoTitle)/sizeof(TCHAR));
		PyWinObject_FreeTCHAR(szInfoTitle);
	}
	return TRUE;
}
#endif // MS_WINCE
%}
#define NIF_ICON NIF_ICON
#define NIF_MESSAGE NIF_MESSAGE
#define NIF_TIP NIF_TIP

#ifndef MS_WINCE
#define NIF_INFO NIF_INFO
#define NIIF_WARNING NIIF_WARNING
#define NIIF_ERROR NIIF_ERROR
#define NIIF_NONE NIIF_NONE
#define NIIF_INFO NIIF_INFO
#endif

#define NIM_ADD NIM_ADD // Adds an icon to the status area. 
#define NIM_DELETE  NIM_DELETE // Deletes an icon from the status area. 
#define NIM_MODIFY  NIM_MODIFY // Modifies an icon in the status area.  
#ifdef NIM_SETFOCUS
#define NIM_SETFOCUS NIM_SETFOCUS // Give the icon focus.  
#endif

%typemap(python,in) NOTIFYICONDATA *{
	if (!PyObject_AsNOTIFYICONDATA($source, $target))
		return NULL;
}
%typemap(python,arginit) NOTIFYICONDATA *{
	NOTIFYICONDATA nid;
	$target = &nid;
}
// @pyswig |Shell_NotifyIcon|Adds, removes or modifies a taskbar icon,
BOOLAPI Shell_NotifyIcon(DWORD dwMessage, NOTIFYICONDATA *pnid);

#ifdef MS_WINCE
HWND    CommandBar_Create(HINSTANCE hInst, HWND hwndParent, int idCmdBar);

BOOLAPI CommandBar_Show(HWND hwndCB, BOOL fShow);

int     CommandBar_AddBitmap(HWND hwndCB, HINSTANCE hInst, int idBitmap,
								  int iNumImages, int iImageWidth,
								  int iImageHeight);

HWND    CommandBar_InsertComboBox(HWND hwndCB, HINSTANCE hInstance,
									   int  iWidth, UINT dwStyle,
									   WORD idComboBox, WORD iButton);

BOOLAPI CommandBar_InsertMenubar(HWND hwndCB, HINSTANCE hInst,
									  WORD idMenu, WORD iButton);

BOOLAPI CommandBar_InsertMenubarEx(HWND hwndCB,
						               HINSTANCE hinst,
						               TCHAR *pszMenu,
						               WORD iButton);

BOOLAPI CommandBar_DrawMenuBar(HWND hwndCB,
		                       WORD iButton);

HMENU   CommandBar_GetMenu(HWND hwndCB, WORD iButton);

BOOLAPI CommandBar_AddAdornments(HWND hwndCB,
								 DWORD dwFlags,
								 DWORD dwReserved);

int     CommandBar_Height(HWND hwndCB);

/////////////////////////////////////////////////////////
//
// Edit control stuff!
#endif

// A hack that needs to be replaced with a general buffer inteface.
%{
static PyObject *PyEdit_GetLine(PyObject *self, PyObject *args)
{
	long hwnd;
	int line, size=0;
	if (!PyArg_ParseTuple(args, "li|i", &hwnd, &line, &size))
		return NULL;
	int numChars;
	TCHAR *buf;
	Py_BEGIN_ALLOW_THREADS
	if (size==0)
		size = Edit_LineLength((HWND)hwnd, line)+1;
	buf = (TCHAR *)malloc(size * sizeof(TCHAR));
	numChars = Edit_GetLine((HWND)hwnd, line, buf, size);
	Py_END_ALLOW_THREADS
	PyObject *ret;
	if (numChars==0) {
		Py_INCREF(Py_None);
		ret = Py_None;
	} else
		ret = PyWinObject_FromTCHAR(buf, numChars);
	free(buf);
	return ret;
}
%}
%native (Edit_GetLine) PyEdit_GetLine;


#ifdef MS_WINCE
%{
#include "dbgapi.h"
static PyObject *PyNKDbgPrintfW(PyObject *self, PyObject *args)
{
	PyObject *obtext;
	if (!PyArg_ParseTuple(args, "O", &obtext))
		return NULL;
	TCHAR *text;
	if (!PyWinObject_AsTCHAR(obtext, &text))
		return NULL;
	NKDbgPrintfW(_T("%s"), text);
	PyWinObject_FreeTCHAR(text);
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (NKDbgPrintfW) PyNKDbgPrintfW;
#endif

// DAVID ASCHER DAA

// Need to complete the documentation!

// @pyswig int|GetSystemMenu|
// @pyparm int|hwnd||The handle to the window
// @pyparm int|bRevert||
// @rdesc The result is a HMENU to the menu.
HMENU GetSystemMenu(HWND hWnd, BOOL bRevert); 

// @pyswig |DrawMenuBar|
// @pyparm int|hwnd||The handle to the window
BOOLAPI DrawMenuBar(HWND hWnd);

// @pyswig |MoveWindow|
BOOLAPI MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
// @pyparm int|hwnd||The handle to the window
// @pyparm int|x||
// @pyparm int|y||
// @pyparm int|width||
// @pyparm int|height||
// @pyparm int|bRepaint||
#ifndef MS_WINCE
// @pyswig |CloseWindow|
BOOLAPI CloseWindow(HWND hWnd);
#endif

// @pyswig |DeleteMenu|
// @pyparm int|hmenu||The handle to the menu
// @pyparm int|position||The position to delete.
// @pyparm int|flags||
BOOLAPI DeleteMenu(HMENU hMenu, UINT uPosition, UINT uFlags);

// @pyswig |RemoveMenu|
// @pyparm int|hmenu||The handle to the menu
// @pyparm int|position||The position to delete.
// @pyparm int|flags||
BOOLAPI RemoveMenu(HMENU hMenu, UINT uPosition, UINT uFlags);

// @pyswig int|CreateMenu|
// @rdesc The result is a HMENU to the new menu.
HMENU CreateMenu();
// @pyswig int|CreatePopupMenu|
// @rdesc The result is a HMENU to the new menu.
HMENU CreatePopupMenu(); 


// @pyswig int|TrackPopupMenu|Display popup shortcut menu
// @pyparm int|hmenu||The handle to the menu
// @pyparm uint|flags||flags
// @pyparm int|x||x pos
// @pyparm int|y||y pos
// @pyparm int|reserved||reserved
// @pyparm hwnd|hwnd||owner window
// @pyparm REC|prcRect||Pointer to rec (can be None)

LRESULT TrackPopupMenu(HMENU hmenu, UINT flags, int x, int y, int reserved, HWND hwnd, const RECT *INPUT_NULLOK);

#define	TPM_CENTERALIGN	TPM_CENTERALIGN
#define	TPM_LEFTALIGN	TPM_LEFTALIGN
#define	TPM_RIGHTALIGN	TPM_RIGHTALIGN
#define	TPM_BOTTOMALIGN	TPM_BOTTOMALIGN
#define	TPM_TOPALIGN	TPM_TOPALIGN
#define	TPM_VCENTERALIGN	TPM_VCENTERALIGN
#define	TPM_NONOTIFY	TPM_NONOTIFY
#define	TPM_RETURNCMD	TPM_RETURNCMD
#ifndef MS_WINCE
#define	TPM_LEFTBUTTON	TPM_LEFTBUTTON
#define	TPM_RIGHTBUTTON	TPM_RIGHTBUTTON
#endif	/* not MS_WINCE */

%{
#include "commdlg.h"

PyObject *Pylpstr(PyObject *self, PyObject *args) {
	long address;
	PyArg_ParseTuple(args, "l", &address);
	return PyString_FromString((char *)address);
}

%}
%native (lpstr) Pylpstr;

// @pyswig int|CommDlgExtendedError|
DWORD CommDlgExtendedError(void);

%typemap (python, in) OPENFILENAME *INPUT (int size){
	size = sizeof(OPENFILENAME);
/*	$source = PyObject_Str($source); */
	if ( (! PyString_Check($source)) || (size != PyString_GET_SIZE($source)) ) {
		PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string", size);
		return NULL;
	}
	$target = ( OPENFILENAME *)PyString_AS_STRING($source);
}

#ifndef MS_WINCE
// @pyswig int|ExtractIcon|
// @pyparm int|hinstance||
// @pyparm string/<o PyUnicode>|moduleName||
// @pyparm int|index||
// @comm You must destroy the icon handle returned by calling the <om win32gui.DestroyIcon> function. 
// @rdesc The result is a HICON.
HICON ExtractIcon(HINSTANCE hinst, TCHAR *modName, UINT index);
#endif	/* not MS_WINCE */

// @pyswig int|ExtractIconEx|
// @pyparm string|moduleName||
// @pyparm int|index||
// @pyparm int|numIcons|1|
// @comm You must destroy each icon handle returned by calling the <om win32gui.DestroyIcon> function. 
// @rdesc If index==-1, the result is an integer with the number of icons in
// the file, otherwise it is 2 arrays of icon handles.
%{
static PyObject *PyExtractIconEx(PyObject *self, PyObject *args)
{
    int i;
    PyObject *obFname;
    TCHAR *fname;
    int index, nicons=1, nicons_got;
    if (!PyArg_ParseTuple(args, "Oi|i", &obFname, &index, &nicons))
        return NULL;
    if (!PyWinObject_AsTCHAR(obFname, &fname, TRUE))
		return NULL;
#ifndef MS_WINCE // CE doesn't have this special "-1" handling.
    if (index==-1) {
        nicons = (int)ExtractIconEx(fname, index, NULL, NULL, 0);
        PyWinObject_FreeTCHAR(fname);
        return PyInt_FromLong(nicons);
    }
#endif // MS_WINCE
    if (nicons<=0) {
        PyWinObject_FreeTCHAR(fname);
        return PyErr_Format(PyExc_ValueError, "Must supply a valid number of icons to fetch.");
    }
    HICON *rgLarge = NULL;
    HICON *rgSmall = NULL;
    PyObject *ret = NULL;
    PyObject *objects_large = NULL;
    PyObject *objects_small = NULL;
    rgLarge = (HICON *)calloc(nicons, sizeof(HICON));
    if (rgLarge==NULL) {
        PyErr_NoMemory();
        goto done;
    }
    rgSmall = (HICON *)calloc(nicons, sizeof(HICON));
    if (rgSmall==NULL) {
        PyErr_NoMemory();
        goto done;
    }
    nicons_got = (int)ExtractIconEx(fname, index, rgLarge, rgSmall, nicons);
    if (nicons_got==-1) {
        PyWin_SetAPIError("ExtractIconEx");
        goto done;
    }
#ifdef MS_WINCE
    /* On WinCE >= 2.1 the API actually returns a HICON */
    nicons_got = 1;
#endif
    // Asking for 1 always says it got 2!?
    nicons = min(nicons, nicons_got);
    objects_large = PyList_New(nicons);
    if (!objects_large) goto done;
    objects_small = PyList_New(nicons);
    if (!objects_small) goto done;
    for (i=0;i<nicons;i++) {
        PyList_SET_ITEM(objects_large, i, PyInt_FromLong((long)rgLarge[i]));
        PyList_SET_ITEM(objects_small, i, PyInt_FromLong((long)rgSmall[i]));
    }
    ret = Py_BuildValue("OO", objects_large, objects_small);
done:
    PyWinObject_FreeTCHAR(fname);
    Py_XDECREF(objects_large);
    Py_XDECREF(objects_small);
    if (rgLarge) free(rgLarge);
    if (rgSmall) free(rgSmall);
    return ret;
}
%}
%native (ExtractIconEx) PyExtractIconEx;

// @pyswig |DestroyIcon|
// @pyparm int|hicon||The icon to destroy.
BOOLAPI DestroyIcon( HICON hicon);

#ifndef MS_WINCE
// @pyswig tuple|GetIconInfo|
// @pyparm int|hicon||The icon to query
// @rdesc The result is a tuple of (fIcon, xHotspot, yHotspot, hbmMask, hbmColor)
// The hbmMask and hbmColor items are bitmaps created for the caller, so must be freed.
BOOLAPI GetIconInfo( HICON hicon, ICONINFO *OUTPUT);
#endif	/* not MS_WINCE */

// @pyswig |ScreenToClient|Convert screen coordinates to client coords
BOOLAPI ScreenToClient(HWND hWnd,POINT *BOTH);

// @pyswig |ClientToScreen|Convert client coordinates to screen coords
BOOLAPI ClientToScreen(HWND hWnd,POINT *BOTH);

%{
// @pyswig cx, cy|GetTextExtentPoint32|Computes the width and height of the specified string of text.
static PyObject *PyGetTextExtentPoint32(PyObject *self, PyObject *args)
{
	// @pyparm int|dc||The device context
	// @pyparm string|str||The string to measure.
	int dc;
	PyObject *obString;
	if (!PyArg_ParseTuple(args, "iO:GetTextExtentPoint32", &dc, &obString))
		return NULL;
	TCHAR *szString = NULL;
	DWORD nchars;
	if (!PyWinObject_AsTCHAR(obString, &szString, FALSE, &nchars))
		return FALSE;
	SIZE size = {0,0};
	BOOL rc;
	Py_BEGIN_ALLOW_THREADS
	rc = GetTextExtentPoint32( (HDC)dc, szString, nchars, &size);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(szString);
	if (!rc)
		return PyWin_SetAPIError("GetTextExtentPoint32");
	return Py_BuildValue("ll", size.cx, size.cy);
}
%}

%native (GetTextExtentPoint32) PyGetTextExtentPoint32;

// @pyswig int|GetOpenFileName|Creates an Open dialog box that lets the user specify the drive, directory, and the name of a file or set of files to open.
// @rdesc If the user presses OK, the function returns TRUE.  Otherwise, use CommDlgExtendedError for error details.

BOOL GetOpenFileName(OPENFILENAME *INPUT);

#ifndef MS_WINCE

%typemap (python, in) MENUITEMINFO *INPUT (int target_size){
	if (0 != PyObject_AsReadBuffer($source, (const void **)&$target, &target_size))
		return NULL;
	if (sizeof MENUITEMINFO != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string/buffer (got %d bytes)", sizeof MENUITEMINFO, target_size);
}

%typemap (python,in) MENUITEMINFO *BOTH(int target_size) {
	if (0 != PyObject_AsWriteBuffer($source, (void **)&$target, &target_size))
		return NULL;
	if (sizeof MENUITEMINFO != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte buffer (got %d bytes)", sizeof MENUITEMINFO, target_size);
}

%typemap (python, in) MENUINFO *INPUT (int target_size){
	if (0 != PyObject_AsReadBuffer($source, (const void **)&$target, &target_size))
		return NULL;
	if (sizeof MENUINFO != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string/buffer (got %d bytes)", sizeof MENUINFO, target_size);
}

%typemap (python,in) MENUINFO *BOTH(int target_size) {
	if (0 != PyObject_AsWriteBuffer($source, (void **)&$target, &target_size))
		return NULL;
	if (sizeof MENUINFO != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte buffer (got %d bytes)", sizeof MENUINFO, target_size);
}

// @pyswig |InsertMenuItem|Inserts a menu item
// @pyparm int|hMenu||
// @pyparm int|fByPosition||
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI InsertMenuItem(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *INPUT);

// @pyswig |SetMenuItemInfo|Sets menu information
// @pyparm int|hMenu||
// @pyparm int|fByPosition||
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI SetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *INPUT);

// @pyswig |GetMenuItemInfo|Gets menu information
// @pyparm int|hMenu||
// @pyparm int|fByPosition||
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI GetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *BOTH);

#endif

#ifndef MS_WINCE
// @pyswig int|GetMenuItemCount|
int GetMenuItemCount(HMENU hMenu);

// @pyswig int|GetMenuItemRect|
int GetMenuItemRect(HWND hWnd, HMENU hMenu, UINT uItem, RECT *OUTPUT);

// @pyswig int|GetMenuState|
int GetMenuState(HMENU hMenu, UINT uID, UINT flags);

// @pyswig |SetMenuDefaultItem|
BOOLAPI SetMenuDefaultItem(HMENU hMenu, UINT flags, UINT fByPos);

// @pyswig |GetMenuDefaultItem|
int GetMenuDefaultItem(HMENU hMenu, UINT fByPos, UINT flags);
#endif	/* not MS_WINCE */

// @pyswig |AppendMenu|
BOOLAPI AppendMenu(HMENU hMenu, UINT uFlags, UINT uIDNewItem, TCHAR *lpNewItem);

// @pyswig |InsertMenu|
BOOLAPI InsertMenu(HMENU hMenu, UINT uPosition, UINT uFlags, UINT uIDNewItem, TCHAR *INPUT_NULLOK);

// @pyswig |EnableMenuItem|
BOOL EnableMenuItem(HMENU hMenu, UINT uIDEnableItem, UINT uEnable);

// @pyswig int|CheckMenuItem|
int CheckMenuItem(HMENU hMenu, UINT uIDCheckItem, UINT uCheck);

// @pyswig HMENU|GetSubMenu|
HMENU GetSubMenu(HMENU hMenu, int nPos);

#ifndef MS_WINCE
// @pyswig |ModifyMenu|Changes an existing menu item. This function is used to specify the content, appearance, and behavior of the menu item.
BOOLAPI ModifyMenu(
  HMENU hMnu, // @pyparm int|hMnu||handle to menu
  UINT uPosition, // @pyparm int|uPosition||menu item to modify
  UINT uFlags,          // @pyparm int|uFlags||options
  UINT uIDNewItem,  // @pyparm int|uIDNewItem||identifier, menu, or submenu
  TCHAR *INPUT	   // @pyparm string|newItem||menu item content
);

// @pyswig int|GetMenuItemID|Retrieves the menu item identifier of a menu item located at the specified position in a menu. 
UINT GetMenuItemID(
  HMENU hMenu,  // @pyparm int|hMenu||handle to menu
  int nPos      // @pyparm int|nPos||position of menu item
 );

// @pyswig |SetMenuItemBitmaps|Associates the specified bitmap with a menu item. Whether the menu item is selected or clear, the system displays the appropriate bitmap next to the menu item.
BOOLAPI SetMenuItemBitmaps(
  HMENU hMenu,               // @pyparm int|hMenu||handle to menu
  UINT uPosition,            // @pyparm int|uPosition||menu item
  UINT uFlags,               // @pyparm int|uFlags||options
  HBITMAP hBitmapUnchecked,  // @pyparm int|hBitmapUnchecked||handle to unchecked bitmap
  HBITMAP hBitmapChecked     // @pyparm int|hBitmapChecked||handle to checked bitmap
);
#endif	/* not MS_WINCE */

// @pyswig |CheckMenuRadioItem|Checks a specified menu item and makes it a
// radio item. At the same time, the function clears all other menu items in
// the associated group and clears the radio-item type flag for those items.
BOOLAPI CheckMenuRadioItem(
  HMENU hMenu,               // @pyparm int|hMenu||handle to menu
  UINT idFirst,  // @pyparm int|idFirst||identifier or position of first item
  UINT idLast,  // @pyparm int|idLast||identifier or position of last item
  UINT idCheck,  // @pyparm int|idCheck||identifier or position of item to check
  UINT uFlags               // @pyparm int|uFlags||options
);

%ifdef WINXPGUI
// @pyswig |SetMenuInfo|Sets information for a specified menu.
// @comm To avoid complications with Windows NT, this function only exists in winxpgui (not win32gui)
BOOLAPI SetMenuInfo(
  HMENU hmenu,       // @pyparm int|hmenu||handle to menu
  MENUINFO *INPUT  // @pyparm <o MENUINFO>|info||menu information in the format of a buffer.
  // See win32gui_struct for helper functions.
);

// @pyswig |GetMenuInfo|Gets information about a specified menu.
// @comm To avoid complications with Windows NT, this function only exists in winxpgui (not win32gui)
BOOLAPI GetMenuInfo(
	HMENU hMenu, // @pyparm int|hmenu||handle to menu
	MENUINFO *BOTH // @pyparm buffer|info||A buffer to fill with the information.
);
%endif


// @pyswig |DrawFocusRect|
BOOLAPI DrawFocusRect(HDC hDC,  RECT *INPUT);

// @pyswig (int, RECT)|DrawText|Draws formatted text on a device context
// @rdesc Returns the height of the drawn text, and the rectangle coordinates
int DrawText(
	HDC hDC,			// @pyparm int/<o PyHANDLE>|hDC||The device context on which to draw
	TCHAR *INPUT,		// @pyparm str|String||The text to be drawn
	int nCount,			// @pyparm int|nCount||The number of characters, use -1 for simple null-terminated string
	RECT *BOTH,			// @pyparm tuple|Rect||Tuple of 4 ints specifying the position (left, top, right, bottom)
	UINT uFormat);		// @pyparm int|Format||Formatting flags, combination of win32con.DT_* values

%{
//@pyswig int|ExtTextOut|Writes text to a DC.
static PyObject *PyExtTextOut(PyObject *self, PyObject *args)
{
	char *text;
	int strLen, x, y;
	UINT options;
	PyObject *rectObject, *widthObject = NULL;
	RECT rect, *rectPtr;
	int *widths = NULL;
	int hdc;
	if (!PyArg_ParseTuple (args, "iiiiOs#|O",
		&hdc,
		&x,		// @pyparm x|int||The x coordinate to write the text to.
		&y,		// @pyparm y|int||The y coordinate to write the text to.
		&options,	// @pyparm nOptions|int||Specifies the rectangle type. This parameter can be one, both, or neither of ETO_CLIPPED and ETO_OPAQUE
		&rectObject,// @pyparm (left, top, right, bottom)|rect||Specifies the text's bounding rectangle.  (Can be None.)
		&text,	// @pyparm text|string||The text to write.
		&strLen,
		&widthObject))	// @pyparm (width1, width2, ...)|tuple||Optional array of values that indicate distance between origins of character cells.
		return NULL;

	// Parse out rectangle object
	if (rectObject != Py_None) {
		if (!PyArg_ParseTuple(rectObject, "iiii", &rect.left,
			&rect.top, &rect.right, &rect.bottom))
			return NULL;
		rectPtr = &rect;
	}
	else
		rectPtr = NULL;

	// Parse out widths
	if (widthObject) {
		BOOL error = !PyTuple_Check(widthObject);
		if (!error) {
			int len = PyTuple_Size(widthObject);
			if (len == (strLen - 1)) {
				widths = new int[len + 1];
				for (int i = 0; i < len; i++) {
					PyObject *item = PyTuple_GetItem(widthObject, i);
					if (!PyInt_Check(item))
						error = TRUE;
					else 
						widths[i] = PyInt_AsLong(item);
				}
			}
		}
		if (error) {
			delete [] widths;
			return PyErr_Format(PyExc_TypeError,
			                    "The width param must be a tuple of integers with a length one less than that of the string");
		}
	}

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS;
	// @pyseeapi ExtTextOut
	ok = ExtTextOut((HDC)hdc, x, y, options, rectPtr, text, strLen, widths);
	Py_END_ALLOW_THREADS;
	delete [] widths;
	if (!ok)
		return PyWin_SetAPIError("ExtTextOut");
	Py_INCREF(Py_None);
	return Py_None;
	// @rdesc Always none.  If the function fails, an exception is raised.
}

%}
%native (ExtTextOut) PyExtTextOut;


// @pyswig int|SetTextColor|Changes the text color for a device context
// @rdesc Returns the previous color, or CLR_INVALID on failure
int SetTextColor(
	HDC hdc,			// @pyparm int|hdc||Handle to a device context
	COLORREF color);	// @pyparm int|color||The RGB color value - see <om win32api.RGB>

// @pyswig int|SetBkMode|Sets the background mode for a device context
// @rdesc Returns the previous mode, or 0 on failure
int SetBkMode(
	HDC hdc,			// @pyparm int/<o PyHANDLE>|hdc||Handle to a device context
	int mode);			// @pyparm int|BkMode||OPAQUE or TRANSPARENT 

// @pyswig int|SetBkColor|Sets the background color for a device context
// @rdesc Returns the previous color, or CLR_INVALID on failure
int SetBkColor(
	HDC hdc,			// @pyparm int/<o PyHANDLE>|hdc||Handle to a device context
	COLORREF col);			// @pyparm int|color||

// @pyswig |DrawEdge|
BOOLAPI DrawEdge(HDC hdc, RECT *INPUT, UINT edge, UINT grfFlags); 
// @pyswig |FillRect|
int FillRect(HDC hDC,   RECT *INPUT, HBRUSH hbr);
// @pyswig |DrawAnimatedRects|
BOOLAPI DrawAnimatedRects(
  HWND hwnd,            // @pyparm int|hwnd||handle to clipping window
  int idAni,            // @pyparm int|idAni||type of animation
  RECT *INPUT, // @pyparm RECT|minCoords||rectangle coordinates (minimized)
  RECT *INPUT // // @pyparm RECT|restCoords||rectangle coordinates (restored)
);
// @pyswig |CreateSolidBrush|
HBRUSH CreateSolidBrush(COLORREF color);
// @pyswig |CreatePen|
HPEN CreatePen(int fnPenStyle, int nWidth, COLORREF crColor);
// @pyswig |GetSysColor|
DWORD GetSysColor(int nIndex);
// @pyswig |GetSysColorBrush|
HBRUSH GetSysColorBrush(int nIndex);
// @pyswig |InvalidateRect|
BOOLAPI InvalidateRect(HWND hWnd,  RECT *INPUT_NULLOK , BOOL bErase);
#ifndef MS_WINCE
// @pyswig |FrameRect|
int FrameRect(HDC hDC,   RECT *INPUT, HBRUSH hbr);
#endif	/* not MS_WINCE */
// @pyswig |GetUpdateRgn|
int GetUpdateRgn(HWND hWnd, HRGN hRgn, BOOL bErase);
// @pyswig |Rectangle|
BOOLAPI Rectangle(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);

// @pyswig hdc, paintstruct|BeginPaint|
HDC BeginPaint(HWND hwnd, PAINTSTRUCT *OUTPUT);

// @pyswig |EndPaint|
// @pyparm int|hwnd||
// @pyparm paintstruct|ps||As returned from <om win32gui.BeginPaint>
BOOLAPI EndPaint(HWND hWnd,  PAINTSTRUCT *INPUT); 

// @pyswig int|CreateWindowEx|Creates a new window with Extended Style.
HWND CreateWindowEx( 
	DWORD dwExStyle,      // @pyparm int|dwExStyle||extended window style
	STRING_OR_ATOM_CW lpClassName, // @pyparm int/string|className||
	TCHAR *INPUT_NULLOK, // @pyparm string|windowTitle||
	DWORD dwStyle, // @pyparm int|style||The style for the window.
	int x,  // @pyparm int|x||
	int y,  // @pyparm int|y||
	int nWidth, // @pyparm int|width||
	int nHeight, // @pyparm int|height||
	HWND hWndParent, // @pyparm int|parent||Handle to the parent window.
	HMENU hMenu, // @pyparm int|menu||Handle to the menu to use for this window.
	HINSTANCE hInstance, // @pyparm int|hinstance||
	NULL_ONLY null // @pyparm None|reserved||Must be None
);

// @pyswig int|GetParent|Retrieves a handle to the specified child window's parent window.
HWND GetParent(
	HWND hWnd // @pyparm int|child||handle to child window
); 

// @pyswig int|SetParent|changes the parent window of the specified child window. 
HWND SetParent(
	HWND hWndChild, // @pyparm int|child||handle to window whose parent is changing
	HWND hWndNewParent // @pyparm int|child||handle to new parent window
); 

// @pyswig |GetCursorPos|retrieves the cursor's position, in screen coordinates. 
BOOLAPI GetCursorPos(
  POINT *OUTPUT   // @pyparm int, int|point||address of structure for cursor position
);
 
// @pyswig int|GetDesktopWindow|returns the desktop window 
HWND GetDesktopWindow();

// @pyswig int|GetWindow|returns a window that has the specified relationship (Z order or owner) to the specified window.  
HWND GetWindow(
	HWND hWnd,  // @pyparm int|hWnd||handle to original window
	UINT uCmd   // @pyparm int|uCmd||relationship flag
);
// @pyswig int|GetWindowDC|returns the device context (DC) for the entire window, including title bar, menus, and scroll bars.
HDC GetWindowDC(
	HWND hWnd   // @pyparm int|hWnd||handle of window
); 

#ifndef MS_WINCE
// @pyswig |IsIconic|determines whether the specified window is minimized (iconic).
BOOL IsIconic(  HWND hWnd   // @pyparm int|hWnd||handle to window
); 
#endif	/* not MS_WINCE */


// @pyswig |IsWindow|determines whether the specified window handle identifies an existing window.
BOOL IsWindow(  HWND hWnd   // @pyparm int|hWnd||handle to window
); 

// @pyswig |IsChild|Tests whether a window is a child window or descendant window of a specified parent window
BOOL IsChild(  
	HWND hWndParent,   // @pyparm int|hWndParent||handle to parent window
	HWND hWnd   // @pyparm int|hWnd||handle to window to test
); 

// @pyswig |ReleaseCapture|Releases the moust capture for a window.
BOOLAPI ReleaseCapture();
// @pyswig int|GetCapture|Returns the window with the mouse capture.
HWND GetCapture();
// @pyswig |SetCapture|Captures the mouse for the specified window.
HWND SetCapture(HWND hWnd);

#ifndef MS_WINCE
// @pyswig |_TrackMouseEvent|Posts messages when the mouse pointer leaves a window or hovers over a window for a specified amount of time.
// @pyparm <o TRACKMOUSEEVENT>|tme||
BOOLAPI _TrackMouseEvent(TRACKMOUSEEVENT *INPUT);
#endif

// @pyswig int|ReleaseDC|Releases a device context.
int ReleaseDC(
	HWND hWnd,  // @pyparm int|hWnd||handle to window
	HDC hDC     // @pyparm int|hDC||handle to device context
); 

// @pyswig |CreateCaret|
BOOLAPI CreateCaret(
	HWND hWnd,        // @pyparm int|hWnd||handle to owner window
	HBITMAP hBitmap,  // @pyparm int|hBitmap||handle to bitmap for caret shape
	int nWidth,       // @pyparm int|nWidth||caret width
	int nHeight       // @pyparm int|nHeight||caret height
); 

// @pyswig |DestroyCaret|
BOOLAPI DestroyCaret();

// @pyswig int|ScrollWindowEx|scrolls the content of the specified window's client area. 
int ScrollWindowEx(
	HWND hWnd,        // @pyparm int|hWnd||handle to window to scroll
	int dx,           // @pyparm int|dx||amount of horizontal scrolling
	int dy,           // @pyparm int|dy||amount of vertical scrolling
	RECT *INPUT_NULLOK, // @pyparm int|prcScroll||address of structure with scroll rectangle
	RECT *INPUT_NULLOK,  // @pyparm int|prcClip||address of structure with clip rectangle
	struct HRGN__ *NONE_ONLY,  // @pyparm int|hrgnUpdate||handle to update region
	RECT *INPUT_NULLOK, // @pyparm int|prcUpdate||address of structure for update rectangle
	UINT flags        // @pyparm int|flags||scrolling flags
); 

// Get/SetScrollInfo

%{

#define GUI_BGN_SAVE PyThreadState *_save = PyEval_SaveThread()
#define GUI_END_SAVE PyEval_RestoreThread(_save)
#define GUI_BLOCK_THREADS Py_BLOCK_THREADS
#define RETURN_NONE				do {Py_INCREF(Py_None);return Py_None;} while (0)
#define RETURN_ERR(err)			do {PyErr_SetString(ui_module_error,err);return NULL;} while (0)
#define RETURN_MEM_ERR(err)		do {PyErr_SetString(PyExc_MemoryError,err);return NULL;} while (0)
#define RETURN_TYPE_ERR(err)	do {PyErr_SetString(PyExc_TypeError,err);return NULL;} while (0)
#define RETURN_VALUE_ERR(err)	do {PyErr_SetString(PyExc_ValueError,err);return NULL;} while (0)
#define RETURN_API_ERR(fn) return ReturnAPIError(fn)

#define CHECK_NO_ARGS(args)		do {if (!PyArg_ParseTuple(args,"")) return NULL;} while (0)
#define CHECK_NO_ARGS2(args, fnName) do {if (!PyArg_ParseTuple(args,":"#fnName)) return NULL;} while (0)

// @object PySCROLLINFO|A tuple representing a SCROLLINFO structure
// @tupleitem 0|int|addnMask|Additional mask information.  Python automatically fills the mask for valid items, so currently the only valid values are zero, and win32con.SIF_DISABLENOSCROLL.
// @tupleitem 1|int|min|The minimum scrolling position.  Both min and max, or neither, must be provided.
// @tupleitem 2|int|max|The maximum scrolling position.  Both min and max, or neither, must be provided.
// @tupleitem 3|int|page|Specifies the page size. A scroll bar uses this value to determine the appropriate size of the proportional scroll box.
// @tupleitem 4|int|pos|Specifies the position of the scroll box.
// @tupleitem 5|int|trackPos|Specifies the immediate position of a scroll box that the user 
// is dragging. An application can retrieve this value while processing 
// the SB_THUMBTRACK notification message. An application cannot set 
// the immediate scroll position; the <om PyCWnd.SetScrollInfo> function ignores 
// this member.
// @comm When passed to Python, will always be a tuple of size 6, and items may be None if not available.
// @comm When passed from Python, it must have the addn mask attribute, but all other items may be None, or not exist.
BOOL ParseSCROLLINFOTuple( PyObject *args, SCROLLINFO *pInfo)
{
	static char *err_msg="SCROLLINFO must be a tuple of 1-6 ints";
	PyObject *obMin=Py_None, *obMax=Py_None, *obPage=Py_None, *obPos=Py_None, *obTrackPos=Py_None;
	int len = PyTuple_Size(args);
	if (len<1 || len > 6) {
		PyErr_SetString(PyExc_TypeError, err_msg);
		return FALSE;
	}
	if (!PyArg_ParseTuple(args, "l|OOOOO", &pInfo->fMask, &obMin, &obMax, &obPage, &obPos, &obTrackPos)){
		PyErr_SetString(PyExc_TypeError, err_msg);
		return FALSE;
	}
	PyErr_Clear(); // clear any errors, so I can detect my own.
	// 1/2 - nMin/nMax
	if ((obMin==Py_None && obMax!=Py_None) || (obMin!=Py_None && obMax==Py_None)){
		PyErr_SetString(PyExc_TypeError, "SCROLLINFO - Both min and max, or neither, must be provided.");
		return FALSE;
	}
	if (obMin!=Py_None){
		if (((pInfo->nMin=PyInt_AsLong(obMin))==-1)&&PyErr_Occurred())
			return FALSE;
		if (((pInfo->nMax=PyInt_AsLong(obMax))==-1)&&PyErr_Occurred())
			return FALSE;
		pInfo->fMask |= SIF_RANGE;
	}
	if (obPage!=Py_None){
		if (((pInfo->nPage=PyInt_AsLong(obPage))==-1)&&PyErr_Occurred())
			return FALSE;
		pInfo->fMask |= SIF_PAGE;
	}
	if (obPos!=Py_None){
		if (((pInfo->nPos=PyInt_AsLong(obPos))==-1)&&PyErr_Occurred())
			return FALSE;
		pInfo->fMask |= SIF_POS;
	}
	if (obTrackPos!=Py_None){
		if (((pInfo->nTrackPos=PyInt_AsLong(obTrackPos))==-1)&&PyErr_Occurred())
			return FALSE;
		pInfo->fMask |= SIF_TRACKPOS;
	}
	return TRUE;
}

PyObject *MakeSCROLLINFOTuple(SCROLLINFO *pInfo)
{
	PyObject *ret = PyTuple_New(6);
	if (ret==NULL) return NULL;
	PyTuple_SET_ITEM(ret, 0, PyInt_FromLong(0));
	if (pInfo->fMask & SIF_RANGE) {
		PyTuple_SET_ITEM(ret, 1, PyInt_FromLong(pInfo->nMin));
		PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(pInfo->nMax));
	} else {
		Py_INCREF(Py_None);
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 1, Py_None);
		PyTuple_SET_ITEM(ret, 2, Py_None);
	}
	if (pInfo->fMask & SIF_PAGE) {
		PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(pInfo->nPage));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 3, Py_None);
	}
	if (pInfo->fMask & SIF_POS) {
		PyTuple_SET_ITEM(ret, 4, PyInt_FromLong(pInfo->nPos));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 4, Py_None);
	}
	PyTuple_SET_ITEM(ret, 5, PyInt_FromLong(pInfo->nTrackPos));
	return ret;
}

// @pyswig |SetScrollInfo|Sets information about a scroll-bar
// @rdesc  Returns an int with the current position of the scroll box.
static PyObject *PySetScrollInfo(PyObject *self, PyObject *args) {
	int nBar;
	HWND hwnd;
	BOOL bRedraw = TRUE;
	PyObject *obInfo;

	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|nBar||Identifies the bar.
	// @pyparm <o PySCROLLINFO>|scollInfo||Scollbar info.
	// @pyparm int|bRedraw|1|Should the bar be redrawn?
	if (!PyArg_ParseTuple(args, "liO|i:SetScrollInfo",
						  &hwnd, &nBar, &obInfo, &bRedraw)) {
		PyWin_SetAPIError("SetScrollInfo");
		return NULL;
	}
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	if (ParseSCROLLINFOTuple(obInfo, &info) == 0)
		return NULL;

	GUI_BGN_SAVE;
	int rc = SetScrollInfo(hwnd, nBar, &info, bRedraw);
	GUI_END_SAVE;
	return PyInt_FromLong(rc);
}
%}
%native (SetScrollInfo) PySetScrollInfo;

%{
// @pyswig <o PySCROLLINFO>|GetScrollInfo|Returns information about a scroll bar
static PyObject *
PyGetScrollInfo (PyObject *self, PyObject *args)
{
	HWND hwnd;
	int nBar;
	UINT fMask = SIF_ALL;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|nBar||The scroll bar to examine.  Can be one of win32con.SB_CTL, win32con.SB_VERT or win32con.SB_HORZ
	// @pyparm int|mask|SIF_ALL|The mask for attributes to retrieve.
	if (!PyArg_ParseTuple(args, "li|i:GetScrollInfo", &hwnd, &nBar, &fMask))
		return NULL;
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = fMask;
	GUI_BGN_SAVE;
	BOOL ok = GetScrollInfo(hwnd, nBar, &info);
	GUI_END_SAVE;
	if (!ok) {
		PyWin_SetAPIError("GetScrollInfo");
		return NULL;
	}
	return MakeSCROLLINFOTuple(&info);
}
%}
%native (GetScrollInfo) PyGetScrollInfo;

%{
// @pyswig string|GetClassName|Retrieves the name of the class to which the specified window belongs. 
static PyObject *
PyGetClassName(PyObject *self, PyObject *args)
{
	HWND hwnd;
	TCHAR buf[256];
	// @pyparm int|hwnd||The handle to the window
	if (!PyArg_ParseTuple(args, "i:GetClassName", &hwnd))
		return NULL;
	// dont bother with lock - no callback possible.
	int nchars = GetClassName(hwnd, buf, sizeof buf/sizeof buf[0]);
	if (nchars==0)
		PyWin_SetAPIError("GetClassName");
	return PyWinObject_FromTCHAR(buf, nchars);
}
%}
%native (GetClassName) PyGetClassName;

// @pyswig int|WindowFromPoint|Retrieves a handle to the window that contains the specified point.
// @pyparm (int, int)|point||The point.
HWND WindowFromPoint(POINT INPUT);

// @pyswig int|ChildWindowFromPoint|Determines which, if any, of the child windows belonging to a parent window contains the specified point.
// @pyparm int|hwndParent||The parent.
// @pyparm (int, int)|point||The point.
HWND ChildWindowFromPoint(HWND INPUT, POINT INPUT);

#ifndef MS_WINCE
// @pyswig int|ChildWindowFromPoint|Determines which, if any, of the child windows belonging to a parent window contains the specified point.
// @pyparm int|hwndParent||The parent.
// @pyparm (int, int)|point||The point.
// @pyparm int|flags||Specifies which child windows to skip. This parameter can be one or more of the CWP_* constants.
HWND ChildWindowFromPointEx(HWND INPUT, POINT INPUT, int flags);
#endif

// Sorting for controls
%{
#if (PY_VERSION_HEX >= 0x02030000) // PyGILState only in 2.3+

// Callbacks
struct PySortCallback {
	PyObject *fn;
	PyObject *data;
};

int CALLBACK CompareFunc(); 

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, 
LPARAM lParamSort);

static int CALLBACK PySortFunc(
	LPARAM lParam1,
	LPARAM lParam2, 
	LPARAM lParamSort
    )
{
	int rc = 0;
	PyObject *result = NULL;
	PyObject *args = NULL;
	PyGILState_STATE state = PyGILState_Ensure();
	PySortCallback *pc = (PySortCallback *)lParamSort;
	if (!pc) {
		PySys_WriteStderr("Control sort function callback with no data!\n");
		goto done;
	}
	assert(!PyErr_Occurred());
	args = Py_BuildValue("llO", lParam1, lParam2, pc->data);
	if (!args) goto done;
	result = PyEval_CallObject(pc->fn, args);
	// API says must return 0, but there might be a good reason.
	if (!result) goto done;
	if (!PyInt_Check(result)) {
		PyErr_SetString(PyExc_TypeError, "The sort function must return an integer");
		goto done;
	}
	rc = PyInt_AsLong(result);
done:
	if (PyErr_Occurred())
		HandleError("ListView sort callback failed!");
	Py_XDECREF(args);
	Py_XDECREF(result);
	PyGILState_Release(state);
	return rc;
}


// @pyswig |ListView_SortItems|Uses an application-defined comparison function to sort the items of a list view control.
static PyObject *
PyListView_SortItems(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *ob;
	PyObject *obParam = Py_None;
	// @pyparm int|hwnd||The handle to the window
	// @pyparm object|callback||A callback object, taking 3 params.
	// @pyparm object|param|None|The third param to the callback function.
	if (!PyArg_ParseTuple(args, "iO|O:ListView_SortItems", &hwnd, &ob, &obParam))
		return NULL;
	if (!PyCallable_Check(ob))
		return PyErr_Format(PyExc_TypeError,
		                    "2nd param must be callable (got type %s)", ob->ob_type->tp_name);
	PySortCallback cb = {ob, obParam};
	BOOL ok;
	GUI_BGN_SAVE;
	ok = ListView_SortItems(hwnd, PySortFunc, &cb);
	GUI_END_SAVE;
	if (!ok) {
		PyWin_SetAPIError("ListView_SortItems");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}
#else // PYVERSION
static PyObject *PyListView_SortItems(PyObject *self, PyObject *args)
{
	PyErr_SetString(PyExc_NotImplementedError,
					"This requires Python 2.3 or greater");
	return NULL;
}
#endif // PYVERSION 2.3+
%}

%native (ListView_SortItems) PyListView_SortItems;

#ifndef MS_WINCE
%{
#if (PY_VERSION_HEX >= 0x02030000) // PyGILState only in 2.3+
// @pyswig |ListView_SortItemsEx|Uses an application-defined comparison function to sort the items of a list view control.
static PyObject *
PyListView_SortItemsEx(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *ob;
	PyObject *obParam = Py_None;
	// @pyparm int|hwnd||The handle to the window
	// @pyparm object|callback||A callback object, taking 3 params.
	// @pyparm object|param|None|The third param to the callback function.
	if (!PyArg_ParseTuple(args, "iO|O:ListView_SortItemsEx", &hwnd, &ob, &obParam))
		return NULL;
	if (!PyCallable_Check(ob))
		return PyErr_Format(PyExc_TypeError,
		                    "2nd param must be callable (got type %s)", ob->ob_type->tp_name);
	PySortCallback cb = {ob, obParam};
	BOOL ok;
	GUI_BGN_SAVE;
	ok = ListView_SortItemsEx(hwnd, PySortFunc, &cb);
	GUI_END_SAVE;
	if (!ok) {
		PyWin_SetAPIError("ListView_SortItemsEx");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}
#else // PYVERSION
static PyObject *PyListView_SortItemsEx(PyObject *self, PyObject *args)
{
	PyErr_SetString(PyExc_NotImplementedError,
	                "This requires Python 2.3 or greater");
	return NULL;
}
#endif // PYVERSION
%}
%native (ListView_SortItemsEx) PyListView_SortItemsEx;
#endif	// !MS_WINCE

%typemap(python,in) DEVMODE *INPUT
{
	if(!PyWinObject_AsDEVMODE($source, &$target, TRUE))
	return NULL;
}

%typemap(python,arginit) DEVMODE *
{
	$target = NULL;
}

// @pyswig int|CreateDC|Creates a device context for a printer or display device
// @pyparm string|Driver||Name of display or print provider, usually DISPLAY or WINSPOOL
// @pyparm string|Device||Name of specific device, eg printer name returned from GetDefaultPrinter
// @pyparm <o PyDEVMODE>|InitData||A PyDEVMODE that specifies printing parameters, use None for printer defaults

%native (CreateDC) PyCreateDC;
%{
static PyObject *PyCreateDC(PyObject *self, PyObject *args)
{
	PDEVMODE pdevmode;
	PyObject *obdevmode=NULL;
	PyObject *obdriver, *obdevice;
	char *driver, *device, *dummyoutput=NULL;
	HDC hdc;
	if (!PyArg_ParseTuple(args, "OOO", &obdriver, &obdevice, &obdevmode))
		return NULL;
	if (!PyWinObject_AsDEVMODE(obdevmode, &pdevmode, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obdriver, &driver, FALSE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obdevice, &device, TRUE)) {
		PyWinObject_FreeTCHAR(driver);
		return NULL;
	}
	PyObject *ret;
	hdc=CreateDC(driver, device, dummyoutput, pdevmode);
	if (hdc!=NULL)
		ret = Py_BuildValue("l",hdc);
	else {
		PyWin_SetAPIError("CreateDC",GetLastError());
		ret = NULL;
	}
	PyWinObject_FreeTCHAR(driver);
	PyWinObject_FreeTCHAR(device);
	return ret;
}
%}

%{
void PyWinObject_FreeOPENFILENAMEW(OPENFILENAMEW *pofn)
{
	if (pofn->lpstrFile!=NULL)
		free(pofn->lpstrFile);
	if (pofn->lpstrCustomFilter!=NULL)
		free(pofn->lpstrCustomFilter);				
	// these are all defined as CONST in the structure
	PyWinObject_FreeWCHAR((WCHAR *)pofn->lpstrFilter);
	PyWinObject_FreeWCHAR((WCHAR *)pofn->lpstrInitialDir);
	PyWinObject_FreeWCHAR((WCHAR *)pofn->lpstrTitle);
	PyWinObject_FreeWCHAR((WCHAR *)pofn->lpstrDefExt);
	// lpTemplateName can also be a resource id
	if ((pofn->lpTemplateName!=NULL) && !IS_INTRESOURCE(pofn->lpTemplateName))
		PyWinObject_FreeWCHAR((WCHAR *)pofn->lpTemplateName);
	ZeroMemory(pofn, sizeof(OPENFILENAMEW));
}

// Forward declared so autoduck comments for parms will appear with GetOpenFileNameW
BOOL PyParse_OPENFILENAMEW_Args(PyObject *args, PyObject *kwargs, OPENFILENAMEW *pofn);

PyObject *PyReturn_OPENFILENAMEW_Output(OPENFILENAMEW *pofn)
{
	DWORD filechars, filterchars;
	// there is no returned length, and lpstrFile can contain NULL's if multiple files are selected
	// Walk the string backwards until a non-NULL is found
	for (filechars=pofn->nMaxFile; filechars>0; filechars--)
		if (pofn->lpstrFile[filechars-1]!=0)
			break;

	if (pofn->lpstrCustomFilter==NULL)
		return Py_BuildValue("NOk",
			PyWinObject_FromWCHAR(pofn->lpstrFile, filechars),
			Py_None,
			pofn->Flags);
	// if CustomFilter if present, can contain NULL's also
	for (filterchars=pofn->nMaxCustFilter; filterchars>0; filterchars--)
		if (pofn->lpstrCustomFilter[filterchars-1]!=0)
			break;
	return Py_BuildValue("NNk",
		PyWinObject_FromWCHAR(pofn->lpstrFile, filechars),
		// include trailing NULL so returned value can be passed back in as a filter unmodified
		PyWinObject_FromWCHAR(pofn->lpstrCustomFilter, filterchars+1),
		pofn->Flags);
}
%}


%native (GetSaveFileNameW) pfnPyGetSaveFileNameW;
%native (GetOpenFileNameW) pfnPyGetOpenFileNameW;

%{
// @pyswig (<o PyUNICODE>,<o PyUNICODE>,int)|GetSaveFileNameW|Creates a dialog for user to specify location to save a file or files
// @comm Accepts keyword arguments, all arguments optional
// @rdesc Returns a tuple of 3 values (<o PyUNICODE>, <o PyUNICODE>, int):<nl>
// First is the selected file(s). If multiple files are selected, returned string will be the directory followed by files names
// separated by nulls, otherwise it will be the full path.<nl>
// Second is a unicode string containing user-selected filter, will be None if CustomFilter was not specified<nl>
// Third item contains flags pertaining to users input, such as OFN_READONLY and OFN_EXTENSIONDIFFERENT
// @pyparm <o PyHANDLE>|hwndOwner|None|Handle to window that owns dialog
// @pyparm <o PyHANDLE>|hInstance|None|Handle to module that contains dialog template
// @pyparm <o PyUNICODE>|Filter|None|Contains pairs of descriptions and filespecs separated by NULLS, with a final trailing NULL.
// Example: 'Python Scripts\0*.py;*.pyw;*.pys\0Text files\0*.txt\0'
// @pyparm <o PyUNICODE>|CustomFilter|None|Description to be used for filter that user selected or typed, can also contain a filespec as above
// @pyparm int|FilterIndex|0|Specifies which of the filters is initially selected, use 0 for CustomFilter
// @pyparm <o PyUNICODE>|File|None|The file name initially displayed
// @pyparm int|MaxFile|1024|Number of characters to allocate for selected filename(s), override if large number of files expected
// @pyparm <o PyUNICODE>|InitialDir|None|The starting directory
// @pyparm <o PyUNICODE>|Title|None|The title of the dialog box
// @pyparm int|Flags|0|Combination of win32con.OFN_* constants
// @pyparm <o PyUNICODE>|DefExt|None|The default extension to use
// @pyparm <o PyUNICODE>|TemplateName|None|Name of dialog box template
static PyObject *PyGetSaveFileNameW(PyObject *self, PyObject *args, PyObject *kwargs)
{	
	PyObject *ret=NULL;
	OPENFILENAMEW ofn;

	if (!PyParse_OPENFILENAMEW_Args(args, kwargs, &ofn))
		return NULL;

	if (!GetSaveFileNameW(&ofn))
		PyWin_SetAPIError("GetSaveFileNameW", CommDlgExtendedError());
	else
		ret=PyReturn_OPENFILENAMEW_Output(&ofn);

	PyWinObject_FreeOPENFILENAMEW(&ofn);
	return ret;
}

// @pyswig (<o PyUNICODE>,<o PyUNICODE>, int)|GetOpenFileNameW|Creates a dialog to allow user to select file(s) to open
// @comm Accepts keyword arguments, all arguments optional
// Input parameters and return values are identical to <om win32gui.GetSaveFileNameW>
static PyObject *PyGetOpenFileNameW(PyObject *self, PyObject *args, PyObject *kwargs)
{	
	PyObject *ret=NULL;
	OPENFILENAMEW ofn;

	if (!PyParse_OPENFILENAMEW_Args(args, kwargs, &ofn))
		return NULL;

	if (!GetOpenFileNameW(&ofn))
		PyWin_SetAPIError("GetOpenFileNameW", CommDlgExtendedError());
	else
		ret=PyReturn_OPENFILENAMEW_Output(&ofn);

	PyWinObject_FreeOPENFILENAMEW(&ofn);
	return ret;
}

BOOL PyParse_OPENFILENAMEW_Args(PyObject *args, PyObject *kwargs, OPENFILENAMEW *pofn)
{
	BOOL ret=FALSE;
	static char * keywords[]={"hwndOwner", "hInstance", "Filter", "CustomFilter",
		"FilterIndex", "File", "MaxFile", "InitialDir",
		"Title", "Flags", "DefExt", "TemplateName", NULL};
	PyObject *obFilter=Py_None, *obCustomFilter=Py_None, *obFile=Py_None, *obInitialDir=Py_None, 
		*obTitle=Py_None, *obDefExt=Py_None, *obTemplateName=Py_None,
		*obOwner=Py_None, *obhInstance=Py_None;
	WCHAR *initfile=NULL, *customfilter=NULL;
	DWORD bufsize, initfilechars, customfilterchars;
	long template_id;
	ZeroMemory(pofn, sizeof(OPENFILENAMEW));
	// ??? may need to set size to OPENFILENAME_SIZE_VERSION_400 to be compatible with NT
	pofn->lStructSize=sizeof(OPENFILENAMEW);
	pofn->nMaxFile=1024; 	// default to large buffer since multiple files can be selected

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOOlOlOOlOO:OPENFILENAME", keywords,
		&obOwner,				// @pyparm <o PyHANDLE>|hwndOwner|None|Handle to window that owns dialog
		&obhInstance,			// @pyparm <o PyHANDLE>|hInstance|None|Handle to module that contains dialog template
		&obFilter,				// @pyparm <o PyUNICODE>|Filter|None|Contains pairs of descriptions and filespecs separated by NULLS, with a final trailing NULL.
								// Example: 'Python Scripts\0*.py;*.pyw;*.pys\0Text files\0*.txt\0'
		&obCustomFilter,		// @pyparm <o PyUNICODE>|CustomFilter|None|Description to be used for filter that user selected or typed, can also contain a filespec as above
		&pofn->nFilterIndex,	// @pyparm int|FilterIndex|0|Specifies which of the filters is initially selected, use 0 for CustomFilter
		&obFile,				// @pyparm <o PyUNICODE>|File|None|The file name initially displayed
		&pofn->nMaxFile,		// @pyparm int|MaxFile|1024|Number of characters to allocate for selected filename, override if large number of files expected
		&obInitialDir,			// @pyparm <o PyUNICODE>|InitialDir|None|The starting directory
		&obTitle,				// @pyparm <o PyUNICODE>|Title|None|The title of the dialog box
		&pofn->Flags,			// @pyparm int|Flags|0|Combination of win32con.OFN_* constants
		&obDefExt,				// @pyparm <o PyUNICODE>|DefExt|None|The default extension to use
		&obTemplateName))		// @pyparm <o PyUNICODE>|TemplateName|None|Name of dialog box template
		goto done;

	// CustomFilter will have user-selected (or typed) wildcard pattern appended to it
	if (obCustomFilter!=Py_None){
		if (!PyWinObject_AsWCHAR(obCustomFilter, &customfilter, FALSE, &customfilterchars))
			goto done;
		pofn->nMaxCustFilter=customfilterchars+256;
		bufsize=pofn->nMaxCustFilter*sizeof(WCHAR);
		pofn->lpstrCustomFilter=(LPWSTR)malloc(bufsize);
		if (pofn->lpstrCustomFilter==NULL){
			PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes for CustomFilter", bufsize);
			goto done;
			}
		ZeroMemory(pofn->lpstrCustomFilter, bufsize);
		memcpy(pofn->lpstrCustomFilter, customfilter, customfilterchars*sizeof(WCHAR));
		}

	// lpstrFile buffer receives full path and possibly multiple file names, allocate extra space
	if (!PyWinObject_AsWCHAR(obFile, &initfile, TRUE, &initfilechars))
		goto done;
	pofn->nMaxFile=max(pofn->nMaxFile, initfilechars+1);
	bufsize=pofn->nMaxFile*sizeof(WCHAR);
	pofn->lpstrFile=(LPWSTR)malloc(bufsize);
	if (pofn->lpstrFile==NULL){
		PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes for File buffer", bufsize);
		goto done;
		}
	ZeroMemory(pofn->lpstrFile, bufsize);
	if (initfile!=NULL)
		memcpy(pofn->lpstrFile, initfile, initfilechars*sizeof(WCHAR));

	// lpTemplateName can be a string or a numeric resource id
	if (obTemplateName!=Py_None)
		if (PyInt_Check(obTemplateName) || PyLong_Check(obTemplateName)){
			template_id=PyInt_AsLong(obTemplateName);
			if (template_id==-1 && PyErr_Occurred())
				goto done;
			if (!IS_INTRESOURCE(template_id)){
				PyErr_Format(PyExc_ValueError, "%d is not a valid Resource Id", template_id);
				goto done;
				}
			pofn->lpTemplateName=MAKEINTRESOURCEW(template_id);
			}
		else
			if (!PyWinObject_AsWCHAR(obTemplateName, (WCHAR **)&pofn->lpTemplateName))
				goto done;

	ret=PyWinObject_AsHANDLE(obOwner, (PHANDLE)&pofn->hwndOwner, TRUE) &&
		PyWinObject_AsHANDLE(obhInstance, (PHANDLE)&pofn->hInstance, TRUE) &&
		PyWinObject_AsWCHAR(obFilter, (WCHAR **)&pofn->lpstrFilter, TRUE) &&
		PyWinObject_AsWCHAR(obInitialDir, (WCHAR **)&pofn->lpstrInitialDir, TRUE) &&
		PyWinObject_AsWCHAR(obTitle, (WCHAR **)&pofn->lpstrTitle, TRUE) &&
		PyWinObject_AsWCHAR(obDefExt, (WCHAR **)&pofn->lpstrDefExt, TRUE);
		
	done:
	if (!ret)
		PyWinObject_FreeOPENFILENAMEW(pofn);
	PyWinObject_FreeWCHAR(initfile);
	PyWinObject_FreeWCHAR(customfilter);
	return ret;
}

// Swig 1.2 chokes on functions that takes keywords
PyCFunction pfnPyGetSaveFileNameW=(PyCFunction)PyGetSaveFileNameW;
PyCFunction pfnPyGetOpenFileNameW=(PyCFunction)PyGetOpenFileNameW;
%}

%native (SystemParametersInfo) pfnPySystemParametersInfo;
// @pyswig |SystemParametersInfo|Queries or sets system-wide parameters. This function can also update the user profile while setting a parameter. 
// @rdesc SPI_SET functions all return None on success.  Types returned by SPI_GET functions are dependent on the operation
// @comm Param and WinIni are not used with any of the SPI_GET operations<nl>
// Boolean parameters can be any object that can be evaluated as True or False
%{
BOOL PyObject_AsUINT(PyObject *ob, UINT *puint)
{
	// PyLong_AsUnsignedLong throws a bogus error in 2.3 if passed an int, and there is no PyInt_AsUnsignedLong
	// ref: http://mail.python.org/pipermail/patches/2004-September/016060.html
	// And for some reason none of the Unsigned*Mask functions check for overflow ???

	__int64 UINT_candidate=PyLong_AsLongLong(ob);
	if (UINT_candidate==-1 && PyErr_Occurred())
		return FALSE;
	if (UINT_candidate<0 || UINT_candidate>UINT_MAX){
		PyErr_Format(PyExc_ValueError, "Parameter must be in range 0 - %d", UINT_MAX);
		return FALSE;
		}
	*puint=(UINT)UINT_candidate;
	return TRUE;
}

BOOL PyWinObject_AsNONCLIENTMETRICS(PyObject *ob, NONCLIENTMETRICS *ncm)
{
	static char *keywords[]={"iBorderWidth","iScrollWidth","iScrollHeight",
		"iCaptionWidth","iCaptionHeight","lfCaptionFont",
		"iSmCaptionWidth","iSmCaptionHeight","lfSmCaptionFont",
		"iMenuWidth","iMenuHeight","lfMenuFont","lfStatusFont",
		"lfMessageFont", NULL};
	BOOL ret;
	ZeroMemory(ncm, sizeof(NONCLIENTMETRICS));
	ncm->cbSize=sizeof(NONCLIENTMETRICS);

	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError, "NONCLIENTMETRICS must be a dict");
		return FALSE;
		}
	PyObject *dummy_args=PyTuple_New(0);
	if (dummy_args==NULL)	// should not happen, interpreter apparently caches the empty tuple
		return FALSE;
	ret=PyArg_ParseTupleAndKeywords(dummy_args, ob, "iiiiiO&iiO&iiO&O&O&:NONCLIENTMETRICS", keywords,
		&ncm->iBorderWidth, &ncm->iScrollWidth, &ncm->iScrollHeight,
		&ncm->iCaptionWidth, &ncm->iCaptionHeight, 
		PyWinObject_AsLOGFONT, &ncm->lfCaptionFont,
		&ncm->iSmCaptionWidth, &ncm->iSmCaptionHeight,
		PyWinObject_AsLOGFONT, &ncm->lfSmCaptionFont,
		&ncm->iMenuWidth, &ncm->iMenuHeight, 
		PyWinObject_AsLOGFONT, &ncm->lfMenuFont, 
		PyWinObject_AsLOGFONT, &ncm->lfStatusFont,
		PyWinObject_AsLOGFONT, &ncm->lfMessageFont);
	Py_DECREF(dummy_args);
	return ret;
}

static PyObject *PySystemParametersInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Action", "Param", "WinIni",  NULL};
	UINT Action, uiParam=0, WinIni=0;
	PVOID pvParam=NULL;
	PyObject *obParam=Py_None, *ret=NULL;
	DWORD buflen;
	BOOL boolParam;
	UINT uintParam;
#ifndef MS_WINCE
	long longParam;
#endif

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k|Ok", keywords,	
		&Action,	// @pyparm int|Action||System parameter to query or set, one of the SPI_GET* or SPI_SET* constants
		&obParam,	// @pyparm  object|Param|None|depends on action to be taken
		&WinIni))	// @pyparm int|WinIni|0|Flags specifying whether change should be permanent, and if all windows should be notified of change. Combination of SPIF_UPDATEINIFILE, SPIF_SENDCHANGE, SPIF_SENDWININICHANGE
		return NULL;

	// @flagh Action|Input/return type
	switch (Action){
#ifndef MS_WINCE
		// @flag SPI_GETDESKWALLPAPER|Returns the path to the bmp used as wallpaper
		case SPI_GETDESKWALLPAPER:
			uiParam=MAX_PATH;
			buflen=uiParam*sizeof(TCHAR);
			pvParam=malloc(buflen);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen);
				goto done;
				}
			break;
		// @flag SPI_SETDESKWALLPAPER|Param should be a string specifying a .bmp file
		case SPI_SETDESKWALLPAPER:
			if (!PyWinObject_AsTCHAR(obParam, (TCHAR **)&pvParam, TRUE, &buflen))
				goto done;
			uiParam=buflen;
			break;
			
		// Below actions return a boolean pointed to by Param
		// @flag SPI_GETDROPSHADOW|Returns a boolean
		case SPI_GETDROPSHADOW:
		// @flag SPI_GETFLATMENU|Returns a boolean
		case SPI_GETFLATMENU:
		// @flag SPI_GETFONTSMOOTHING|Returns a boolean
		case SPI_GETFONTSMOOTHING:
		// @flag SPI_GETICONTITLEWRAP|Returns a boolean
		case SPI_GETICONTITLEWRAP:
		// @flag SPI_GETSNAPTODEFBUTTON|Returns a boolean
		case SPI_GETSNAPTODEFBUTTON:
		// @flag SPI_GETBEEP|Returns a boolean
		case SPI_GETBEEP:
		// @flag SPI_GETBLOCKSENDINPUTRESETS|Returns a boolean
		case SPI_GETBLOCKSENDINPUTRESETS:
		// @flag SPI_GETMENUUNDERLINES|Returns a boolean
		// @flag SPI_GETKEYBOARDCUES|Returns a boolean
		case SPI_GETKEYBOARDCUES:
		// @flag SPI_GETKEYBOARDPREF|Returns a boolean
		case SPI_GETKEYBOARDPREF:
		// @flag SPI_GETSCREENSAVEACTIVE|Returns a boolean
		case SPI_GETSCREENSAVEACTIVE:
		// @flag SPI_GETSCREENSAVERRUNNING|Returns a boolean
		case SPI_GETSCREENSAVERRUNNING:
		// @flag SPI_GETMENUDROPALIGNMENT|Returns a boolean (True indicates left aligned, False right aligned)
		case SPI_GETMENUDROPALIGNMENT:
		// @flag SPI_GETMENUFADE|Returns a boolean
		case SPI_GETMENUFADE:
		// @flag SPI_GETLOWPOWERACTIVE|Returns a boolean
		case SPI_GETLOWPOWERACTIVE:
		// @flag SPI_GETPOWEROFFACTIVE|Returns a boolean
		case SPI_GETPOWEROFFACTIVE:
		// @flag SPI_GETCOMBOBOXANIMATION|Returns a boolean
		case SPI_GETCOMBOBOXANIMATION:
		// @flag SPI_GETCURSORSHADOW|Returns a boolean
		case SPI_GETCURSORSHADOW:
		// @flag SPI_GETGRADIENTCAPTIONS|Returns a boolean
		case SPI_GETGRADIENTCAPTIONS:
		// @flag SPI_GETHOTTRACKING|Returns a boolean
		case SPI_GETHOTTRACKING:
		// @flag SPI_GETLISTBOXSMOOTHSCROLLING|Returns a boolean
		case SPI_GETLISTBOXSMOOTHSCROLLING:
		// @flag SPI_GETMENUANIMATION|Returns a boolean
		case SPI_GETMENUANIMATION:
		// @flag SPI_GETSELECTIONFADE|Returns a boolean
		case SPI_GETSELECTIONFADE:
		// @flag SPI_GETTOOLTIPANIMATION|Returns a boolean
		case SPI_GETTOOLTIPANIMATION:
		// @flag SPI_GETTOOLTIPFADE|Returns a boolean (TRUE=fade, False=slide)
		case SPI_GETTOOLTIPFADE:
		// @flag SPI_GETUIEFFECTS|Returns a boolean
		case SPI_GETUIEFFECTS:
		// @flag SPI_GETACTIVEWINDOWTRACKING|Returns a boolean
		case SPI_GETACTIVEWINDOWTRACKING:
		// @flag SPI_GETACTIVEWNDTRKZORDER|Returns a boolean
		case SPI_GETACTIVEWNDTRKZORDER:
		// @flag SPI_GETDRAGFULLWINDOWS|Returns a boolean
		case SPI_GETDRAGFULLWINDOWS:    
		// @flag SPI_GETSHOWIMEUI|Returns a boolean
		case SPI_GETSHOWIMEUI:
		// @flag SPI_GETMOUSECLICKLOCK|Returns a boolean
		case SPI_GETMOUSECLICKLOCK:
		// @flag SPI_GETMOUSESONAR|Returns a boolean
		case SPI_GETMOUSESONAR:
		// @flag SPI_GETMOUSEVANISH|Returns a boolean
		case SPI_GETMOUSEVANISH:
		// @flag SPI_GETSCREENREADER|Returns a boolean
		case SPI_GETSCREENREADER:
#endif	// !MS_WINCE
		// @flag SPI_GETSHOWSOUNDS|Returns a boolean
		case SPI_GETSHOWSOUNDS:
			pvParam=&boolParam;
			break;
		
#ifndef MS_WINCE
		// Actions in this section accept a boolean as pvParam
		// @flag SPI_SETDROPSHADOW|Param must be a boolean
		case SPI_SETDROPSHADOW:
		// @flag SPI_SETDROPSHADOW|Param must be a boolean
		case SPI_SETFLATMENU:
		// @flag SPI_SETMENUUNDERLINES|Param must be a boolean
		// @flag SPI_SETKEYBOARDCUES|Param must be a boolean
		case SPI_SETKEYBOARDCUES:
		// @flag SPI_SETMENUFADE|Param must be a boolean
		case SPI_SETMENUFADE:
		// @flag SPI_SETCOMBOBOXANIMATION|Param must be a boolean
		case SPI_SETCOMBOBOXANIMATION:
		// @flag SPI_SETCURSORSHADOW|Param must be a boolean
		case SPI_SETCURSORSHADOW:
		// @flag SPI_SETGRADIENTCAPTIONS|Param must be a boolean
		case SPI_SETGRADIENTCAPTIONS:
		// @flag SPI_SETHOTTRACKING|Param must be a boolean
		case SPI_SETHOTTRACKING:
		// @flag SPI_SETLISTBOXSMOOTHSCROLLING|Param must be a boolean
		case SPI_SETLISTBOXSMOOTHSCROLLING:
		// @flag SPI_SETMENUANIMATION|Param must be a boolean
		case SPI_SETMENUANIMATION:
		// @flag SPI_SETSELECTIONFADE|Param must be a boolean
		case SPI_SETSELECTIONFADE:
		// @flag SPI_SETTOOLTIPANIMATION|Param must be a boolean
		case SPI_SETTOOLTIPANIMATION:
		// @flag SPI_SETTOOLTIPFADE|Param must be a boolean
		case SPI_SETTOOLTIPFADE:
		// @flag SPI_SETUIEFFECTS|Param must be a boolean
		case SPI_SETUIEFFECTS:
		// @flag SPI_SETACTIVEWINDOWTRACKING|Param must be a boolean
		case SPI_SETACTIVEWINDOWTRACKING:
		// @flag SPI_SETACTIVEWNDTRKZORDER|Param must be a boolean
		case SPI_SETACTIVEWNDTRKZORDER:
		// @flag SPI_SETMOUSESONAR|Param must be a boolean
		case SPI_SETMOUSESONAR:
		// @flag SPI_SETMOUSEVANISH|Param must be a boolean
		case SPI_SETMOUSEVANISH:
		// @flag SPI_SETMOUSECLICKLOCK|Param must be a boolean
		case SPI_SETMOUSECLICKLOCK:
			pvParam=(PVOID)PyObject_IsTrue(obParam);
			if (pvParam==(PVOID)-1)
				goto done;
			break;

		// These accept a boolean placed in uiParam
		// @flag SPI_SETFONTSMOOTHING|Param should specify a boolean
		case SPI_SETFONTSMOOTHING:
		// @flag SPI_SETICONTITLEWRAP|Param should specify a boolean
		case SPI_SETICONTITLEWRAP:
		// @flag SPI_SETSNAPTODEFBUTTON|Param is a boolean
		case SPI_SETSNAPTODEFBUTTON:
		// @flag SPI_SETBEEP|Param is a boolean
		case SPI_SETBEEP:
		// @flag SPI_SETBLOCKSENDINPUTRESETS|Param is a boolean
		case SPI_SETBLOCKSENDINPUTRESETS:
		// @flag SPI_SETKEYBOARDPREF|Param is a boolean
		case SPI_SETKEYBOARDPREF:
		// @flag SPI_SETMOUSEBUTTONSWAP|Param is a boolean
		case SPI_SETMOUSEBUTTONSWAP:
		// @flag SPI_SETSCREENSAVEACTIVE|Param is a boolean
		case SPI_SETSCREENSAVEACTIVE:
		// @flag SPI_SETMENUDROPALIGNMENT|Param is a boolean (True=left aligned, False=right aligned)
		case SPI_SETMENUDROPALIGNMENT:
		// @flag SPI_SETLOWPOWERACTIVE|Param is a boolean
		case SPI_SETLOWPOWERACTIVE:
		// @flag SPI_SETPOWEROFFACTIVE|Param is a boolean
		case SPI_SETPOWEROFFACTIVE:
		// @flag SPI_SETDRAGFULLWINDOWS|Param is a boolean
		case SPI_SETDRAGFULLWINDOWS:
		// @flag SPI_SETSHOWIMEUI|Param is a boolean
		case SPI_SETSHOWIMEUI:
		// @flag SPI_SETSCREENREADER|Param is a boolean
		case SPI_SETSCREENREADER:
#endif	// !MS_WINCE
		// @flag SPI_SETSHOWSOUNDS|Param is a boolean
		case SPI_SETSHOWSOUNDS:
			uiParam=(UINT)PyObject_IsTrue(obParam);
			if (uiParam==(UINT)-1)
				goto done;
			break;

#ifndef MS_WINCE
		// These accept an int placed in uiParam
		// @flag SPI_SETMOUSETRAILS|Param should be an int specifying the nbr of cursors in the trail (0 or 1 means disabled)
		case SPI_SETMOUSETRAILS:
#endif	// !MS_WINCE
		// @flag SPI_SETWHEELSCROLLLINES|Param is an int specifying nbr of lines
		case SPI_SETWHEELSCROLLLINES:
#ifndef MS_WINCE
		// @flag SPI_SETKEYBOARDDELAY|Param is an int in the range 0 - 3
		case SPI_SETKEYBOARDDELAY:
		// @flag SPI_SETKEYBOARDSPEED|Param is an int in the range 0 - 31
		case SPI_SETKEYBOARDSPEED:
		// @flag SPI_SETDOUBLECLICKTIME|Param is an int (in milliseconds),  Use <om win32gui.GetDoubleClickTime> to retrieve the value.
		case SPI_SETDOUBLECLICKTIME:
		// @flag SPI_SETDOUBLECLKWIDTH|Param is an int.  Use win32api.GetSystemMetrics(SM_CXDOUBLECLK) to retrieve the value.
		case SPI_SETDOUBLECLKWIDTH:
		// @flag SPI_SETDOUBLECLKHEIGHT|Param is an int,  Use win32api.GetSystemMetrics(SM_CYDOUBLECLK) to retrieve the value.
		case SPI_SETDOUBLECLKHEIGHT:
		// @flag SPI_SETMOUSEHOVERHEIGHT|Param is an int
		case SPI_SETMOUSEHOVERHEIGHT:
		// @flag SPI_SETMOUSEHOVERWIDTH|Param is an int
		case SPI_SETMOUSEHOVERWIDTH:
		// @flag SPI_SETMOUSEHOVERTIME|Param is an int
		case SPI_SETMOUSEHOVERTIME:
		// @flag SPI_SETSCREENSAVETIMEOUT|Param is an int specifying the timeout in seconds
		case SPI_SETSCREENSAVETIMEOUT:
		// @flag SPI_SETMENUSHOWDELAY|Param is an int specifying the shortcut menu delay in milliseconds
		case SPI_SETMENUSHOWDELAY:
		// @flag SPI_SETLOWPOWERTIMEOUT|Param is an int (in seconds)
		case SPI_SETLOWPOWERTIMEOUT:
		// @flag SPI_SETPOWEROFFTIMEOUT|Param is an int (in seconds)
		case SPI_SETPOWEROFFTIMEOUT:
		// @flag SPI_SETDRAGHEIGHT|Param is an int. Use win32api.GetSystemMetrics(SM_CYDRAG) to retrieve the value.
		case SPI_SETDRAGHEIGHT:
		// @flag SPI_SETDRAGWIDTH|Param is an int. Use win32api.GetSystemMetrics(SM_CXDRAG) to retrieve the value.
		case SPI_SETDRAGWIDTH:
		// @flag SPI_SETBORDER|Param is an int
		case SPI_SETBORDER:
#endif	// !MS_WINCE
			if (!PyObject_AsUINT(obParam, &uiParam))
				goto done;
			break;

		// below Actions all return a UINT pointed to by Param
		// @flag SPI_GETFONTSMOOTHINGCONTRAST|Returns an int
		case SPI_GETFONTSMOOTHINGCONTRAST:
#ifndef MS_WINCE
		// @flag SPI_GETFONTSMOOTHINGTYPE|Returns an int
		case SPI_GETFONTSMOOTHINGTYPE:
		// @flag SPI_GETMOUSETRAILS|Returns an int specifying the nbr of cursor images in the trail, 0 or 1 indicates disabled
		case SPI_GETMOUSETRAILS:
		// @flag SPI_GETWHEELSCROLLLINES|Returns the nbr of lines to scroll for the mouse wheel
		case SPI_GETWHEELSCROLLLINES:
		// @flag SPI_GETKEYBOARDDELAY|Returns an int
		case SPI_GETKEYBOARDDELAY:
		// @flag SPI_GETKEYBOARDSPEED|Returns an int
		case SPI_GETKEYBOARDSPEED:
		// @flag SPI_GETMOUSESPEED|Returns an int
		case SPI_GETMOUSESPEED:
		// @flag SPI_GETMOUSEHOVERHEIGHT|Returns an int
		case SPI_GETMOUSEHOVERHEIGHT:
		// @flag SPI_GETMOUSEHOVERWIDTH|Returns an int
		case SPI_GETMOUSEHOVERWIDTH:
		// @flag SPI_GETMOUSEHOVERTIME|Returns an int
		case SPI_GETMOUSEHOVERTIME:
#endif	// !MS_WINCE
		// @flag SPI_GETSCREENSAVETIMEOUT|Returns an int (idle time in seconds)
		case SPI_GETSCREENSAVETIMEOUT:
#ifndef MS_WINCE
		// @flag SPI_GETMENUSHOWDELAY|Returns an int (shortcut delay in milliseconds)
		case SPI_GETMENUSHOWDELAY:
		// @flag SPI_GETLOWPOWERTIMEOUT|Returns an int (in seconds)
		case SPI_GETLOWPOWERTIMEOUT:
		// @flag SPI_GETPOWEROFFTIMEOUT|Returns an int (in seconds)
		case SPI_GETPOWEROFFTIMEOUT:
		// @flag SPI_GETACTIVEWNDTRKTIMEOUT|Returns an int (milliseconds)
		case SPI_GETACTIVEWNDTRKTIMEOUT:
		// @flag SPI_GETBORDER|Returns an int
		case SPI_GETBORDER:
		// @flag SPI_GETCARETWIDTH|Returns an int
		case SPI_GETCARETWIDTH:
		// @flag SPI_GETFOREGROUNDFLASHCOUNT|Returns an int
		case SPI_GETFOREGROUNDFLASHCOUNT:
		// @flag SPI_GETFOREGROUNDLOCKTIMEOUT|Returns an int
		case SPI_GETFOREGROUNDLOCKTIMEOUT:
		// @flag SPI_GETFOCUSBORDERHEIGHT|Returns an int
		case SPI_GETFOCUSBORDERHEIGHT:
		// @flag SPI_GETFOCUSBORDERWIDTH|Returns an int
		case SPI_GETFOCUSBORDERWIDTH:
		// @flag SPI_GETMOUSECLICKLOCKTIME|Returns an int (in milliseconds)
		case SPI_GETMOUSECLICKLOCKTIME:
#endif	// !MS_WINCE
			pvParam=&uintParam;
			break;
		
		// Actions that take pvParam as an unsigned int
		// @flag SPI_SETFONTSMOOTHINGCONTRAST|Param should be an int in the range 1000 to 2200
		case SPI_SETFONTSMOOTHINGCONTRAST:
#ifndef MS_WINCE
		// @flag SPI_SETFONTSMOOTHINGTYPE|Param should be one of the FE_FONTSMOOTHING* constants
		case SPI_SETFONTSMOOTHINGTYPE:
		// @flag SPI_SETMOUSESPEED|Param should be an int in the range 1 - 20
		case SPI_SETMOUSESPEED:
		// @flag SPI_SETACTIVEWNDTRKTIMEOUT|Param is an int (in milliseconds)
		case SPI_SETACTIVEWNDTRKTIMEOUT:
		// @flag SPI_SETCARETWIDTH|Param is an int (in pixels)
		case SPI_SETCARETWIDTH:
		// @flag SPI_SETFOREGROUNDFLASHCOUNT|Param is an int
		case SPI_SETFOREGROUNDFLASHCOUNT:
		// @flag SPI_SETFOREGROUNDLOCKTIMEOUT|Param is an int (in milliseconds)
		case SPI_SETFOREGROUNDLOCKTIMEOUT:
		// @flag SPI_SETFOCUSBORDERHEIGHT|Returns an int
		case SPI_SETFOCUSBORDERHEIGHT:
		// @flag SPI_SETFOCUSBORDERWIDTH|Returns an int
		case SPI_SETFOCUSBORDERWIDTH:
		// @flag SPI_SETMOUSECLICKLOCKTIME|Param is an int (in milliseconds)
		case SPI_SETMOUSECLICKLOCKTIME:
#endif	// !MS_WINCE
			if (!PyObject_AsUINT(obParam, (UINT *)&pvParam))
				goto done;
			break;
			
#ifndef MS_WINCE
		// @flag SPI_GETICONTITLELOGFONT|Returns a <o PyLOGFONT>,
		case SPI_GETICONTITLELOGFONT:
			uiParam=sizeof(LOGFONT);
			pvParam=malloc(uiParam);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", uiParam);
				goto done;
				}
			break;
		// @flag SPI_SETICONTITLELOGFONT|Param must be a <o PyLOGFONT>,
		case SPI_SETICONTITLELOGFONT:
			if (!PyLOGFONT_Check(obParam)){
				PyErr_SetString(PyExc_TypeError, "Param must be a LOGFONT");
				goto done;
				}
			pvParam=((PyLOGFONT *)obParam)->GetLF();
			uiParam=sizeof(LOGFONT);
			break;


		// Set operations that take no parameter
		// @flag SPI_SETLANGTOGGLE|Param is ignored. Sets the language toggle hotkey from registry key HKCU\keyboard layout\toggle 
		case SPI_SETLANGTOGGLE:
		// @flag SPI_SETICONS|Reloads the system icons.  Param is not used
		case SPI_SETICONS:
			break;
#endif	// !MS_WINCE

		// @flag SPI_GETMOUSE|Returns a tuple of 3 ints containing the x and y mouse thresholds and the acceleration factor.
		case SPI_GETMOUSE:
		// @flag SPI_SETMOUSE|Param should be a sequence of 3 ints
		case SPI_SETMOUSE:{
			buflen=3*sizeof(UINT);
			pvParam=malloc(buflen);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", buflen);
				goto done;
				}
			if (Action==SPI_SETMOUSE){
				PyObject *param_tuple=PySequence_Tuple(obParam);
				if (param_tuple==NULL)
					goto done;
				if (PyTuple_GET_SIZE(param_tuple) != 3){
					PyErr_SetString(PyExc_ValueError,"Param must be a sequence of 3 ints");
					Py_DECREF(param_tuple);
					goto done;
					}
				if (!PyArg_ParseTuple(param_tuple, "kkk", &((UINT *)pvParam)[0], &((UINT *)pvParam)[1], &((UINT *)pvParam)[2])){
					Py_DECREF(param_tuple);
					goto done;
					}
				Py_DECREF(param_tuple);
				}
			break;
			}

#ifndef MS_WINCE
		// @flag SPI_GETDEFAULTINPUTLANG|Returns an int (locale id for default language)
			case SPI_GETDEFAULTINPUTLANG:
			pvParam=&longParam;
			break;
		// @flag SPI_SETDEFAULTINPUTLANG|Param is an int containing a locale id
		case SPI_SETDEFAULTINPUTLANG:
			// input is a HKL, which is actually a HANDLE, which can be treated as a long
			longParam=PyInt_AsLong(obParam);
			if (longParam==-1 && PyErr_Occurred())
				goto done;
			pvParam=&longParam;
			break;
		// @flag SPI_GETANIMATION|Returns an int
		case SPI_GETANIMATION:
		// @flag SPI_SETANIMATION|Param is an int
		case SPI_SETANIMATION:
			buflen=sizeof(ANIMATIONINFO);
			pvParam=malloc(buflen);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", buflen);
				goto done;
				}
			ZeroMemory(pvParam, buflen);
			uiParam=buflen;
			((ANIMATIONINFO *)pvParam)->cbSize=buflen;
			if (Action==SPI_SETANIMATION){
				((ANIMATIONINFO *)pvParam)->iMinAnimate=PyInt_AsLong(obParam);
				if (((ANIMATIONINFO *)pvParam)->iMinAnimate==-1 && PyErr_Occurred())
					goto done;
				}
			break;
		// @flag SPI_ICONHORIZONTALSPACING|Functions as both a get and set operation.  If Param is None, functions as a get operation, otherwise Param is an int to be set as the new value
		case SPI_ICONHORIZONTALSPACING:
		// @flag SPI_ICONVERTICALSPACING|Functions as both a get and set operation.  If Param is None, functions as a get operation, otherwise Param is an int to be set as the new value
		case SPI_ICONVERTICALSPACING:
			if (obParam==Py_None)	// indicates a get operation
				pvParam=&uintParam;
			else			// for set operation, value is passed in uiParam
				if (!PyObject_AsUINT(obParam, &uiParam))
					goto done;
			break;
		// @flag SPI_GETNONCLIENTMETRICS|Param must be None.  The result is a dict.
		case SPI_GETNONCLIENTMETRICS:
		// @flag SPI_SETNONCLIENTMETRICS|Param is a dict in the form of a NONCLIENTMETRICS struct, as returned by SPI_GETNONCLIENTMETRICS operation
		case SPI_SETNONCLIENTMETRICS:
			buflen = sizeof(NONCLIENTMETRICS);
			pvParam=malloc(buflen);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", buflen);
				goto done;
			}
			if (Action==SPI_GETNONCLIENTMETRICS){
				if (obParam!=Py_None) {
					PyErr_Format(PyExc_ValueError,
				             "Don't supply a param for SPI_GETNONCLIENTMETRICS");
					goto done;
					}
				memset(pvParam, 0, buflen);
				((NONCLIENTMETRICS *)pvParam)->cbSize = buflen;
				}
			else
				if (!PyWinObject_AsNONCLIENTMETRICS(obParam, (NONCLIENTMETRICS *)pvParam))
					goto done;
			break;
#endif	// !MS_WINCE

		// below are not handled yet
		// @flag SPI_SETDESKPATTERN|Unsupported (obsolete)
		// @flag SPI_GETFASTTASKSWITCH|Unsupported (obsolete)
		// @flag SPI_SETFASTTASKSWITCH|Unsupported (obsolete)
		// @flag SPI_SETSCREENSAVERRUNNING|Unsupported (documented as internal use only)
		// @flag SPI_SCREENSAVERRUNNING|Same as SPI_SETSCREENSAVERRUNNING
		// @flag SPI_SETPENWINDOWS|Unsupported (only relevant for win95)
		// @flag SPI_GETWINDOWSEXTENSION|Unsupported (only relevant for win95)
		// @flag SPI_GETGRIDGRANULARITY|Unsupported (obsolete)
		// @flag SPI_SETGRIDGRANULARITY|Unsupported (obsolete)
		// @flag SPI_LANGDRIVER|Unsupported (use is not documented)
		// @flag SPI_GETFONTSMOOTHINGORIENTATION|Unsupported (use is not documented)
		// @flag SPI_SETFONTSMOOTHINGORIENTATION|Unsupported (use is not documented)
		// @flag SPI_SETHANDHELD|Unsupported (use is not documented)
		// @flag SPI_GETMINIMIZEDMETRICS|Not implemented yet
		// @flag SPI_SETMINIMIZEDMETRICS|Not implemented yet
		// @flag SPI_GETICONMETRICS|Not implemented yet
		// @flag SPI_SETICONMETRICS|Not implemented yet
		// @flag SPI_GETWORKAREA|Not implemented yet
		// @flag SPI_SETWORKAREA|Not implemented yet
		// @flag SPI_GETSERIALKEYS|Not implemented yet
		// @flag SPI_SETSERIALKEYS|Not implemented yet
		// @flag SPI_SETMOUSEKEYS|Not implemented yet
		// @flag SPI_GETMOUSEKEYS|Not implemented yet
		// @flag SPI_GETHIGHCONTRAST|Not implemented yet
		// @flag SPI_SETHIGHCONTRAST|Not implemented yet
		// @flag SPI_GETSOUNDSENTRY|Not implemented yet
		// @flag SPI_SETSOUNDSENTRY|Not implemented yet
		// @flag SPI_GETSTICKYKEYS|Not implemented yet
		// @flag SPI_SETSTICKYKEYS|Not implemented yet
		// @flag SPI_GETTOGGLEKEYS|Not implemented yet
		// @flag SPI_SETTOGGLEKEYS|Not implemented yet
		// @flag SPI_GETACCESSTIMEOUT|Not implemented yet
		// @flag SPI_SETACCESSTIMEOUT|Not implemented yet
		// @flag SPI_GETFILTERKEYS|Not implemented yet
		// @flag SPI_SETFILTERKEYS|Not implemented yet
		default:
			PyErr_Format(PyExc_NotImplementedError, "Action %d is not supported yet", Action);
			goto done;
		}
		
	if (!SystemParametersInfo(Action, uiParam, pvParam, WinIni)){
		PyWin_SetAPIError("SystemParametersInfo");
		goto done;
		}

	switch (Action){
#ifndef MS_WINCE
		case SPI_GETDESKWALLPAPER:
			ret=PyWinObject_FromTCHAR((TCHAR *)pvParam);
			break;
		case SPI_GETDROPSHADOW:
		case SPI_GETFLATMENU:
		case SPI_GETFONTSMOOTHING:
		case SPI_GETICONTITLEWRAP:
		case SPI_GETSNAPTODEFBUTTON:
		case SPI_GETBEEP:
		case SPI_GETBLOCKSENDINPUTRESETS:
		case SPI_GETKEYBOARDCUES:
		case SPI_GETKEYBOARDPREF:
		case SPI_GETSCREENSAVEACTIVE:
		case SPI_GETSCREENSAVERRUNNING:
		case SPI_GETMENUDROPALIGNMENT:
		case SPI_GETMENUFADE:
		case SPI_GETLOWPOWERACTIVE:
		case SPI_GETPOWEROFFACTIVE:
		case SPI_GETCOMBOBOXANIMATION:
		case SPI_GETCURSORSHADOW:
		case SPI_GETGRADIENTCAPTIONS:
		case SPI_GETHOTTRACKING:
		case SPI_GETLISTBOXSMOOTHSCROLLING:
		case SPI_GETMENUANIMATION:
		case SPI_GETSELECTIONFADE:
		case SPI_GETTOOLTIPANIMATION:
		case SPI_GETTOOLTIPFADE:
		case SPI_GETUIEFFECTS:
		case SPI_GETACTIVEWINDOWTRACKING:
		case SPI_GETACTIVEWNDTRKZORDER:
		case SPI_GETDRAGFULLWINDOWS:    
		case SPI_GETSHOWIMEUI:
		case SPI_GETMOUSECLICKLOCK:
		case SPI_GETMOUSESONAR:
		case SPI_GETMOUSEVANISH:
		case SPI_GETSCREENREADER:
#endif	// !MS_WINCE
		case SPI_GETSHOWSOUNDS:
			ret=PyBool_FromLong(boolParam);
			break;
#ifndef MS_WINCE
		case SPI_GETFONTSMOOTHINGTYPE:
		case SPI_GETMOUSETRAILS:
		case SPI_GETKEYBOARDDELAY:
		case SPI_GETKEYBOARDSPEED:
		case SPI_GETMOUSESPEED:
		case SPI_GETMOUSEHOVERHEIGHT:
		case SPI_GETMOUSEHOVERWIDTH:
		case SPI_GETMOUSEHOVERTIME:
		case SPI_GETMENUSHOWDELAY:
		case SPI_GETLOWPOWERTIMEOUT:
		case SPI_GETPOWEROFFTIMEOUT:
		case SPI_GETACTIVEWNDTRKTIMEOUT:
		case SPI_GETBORDER:
		case SPI_GETCARETWIDTH:
		case SPI_GETFOREGROUNDFLASHCOUNT:
		case SPI_GETFOREGROUNDLOCKTIMEOUT:
		case SPI_GETFOCUSBORDERHEIGHT:
		case SPI_GETFOCUSBORDERWIDTH:
		case SPI_GETMOUSECLICKLOCKTIME:
#endif	// !MS_WINCE
		case SPI_GETFONTSMOOTHINGCONTRAST:
		case SPI_GETWHEELSCROLLLINES:
		case SPI_GETSCREENSAVETIMEOUT:
			ret=PyLong_FromUnsignedLong(uintParam);
			break;
#ifndef MS_WINCE
		case SPI_GETDEFAULTINPUTLANG:
			ret=PyLong_FromLong(longParam);
			break;
		case SPI_GETICONTITLELOGFONT:
			ret=new PyLOGFONT((LOGFONT *)pvParam);
			break;
#endif	// !MS_WINCE
		case SPI_GETMOUSE:
			ret=Py_BuildValue("kkk", ((UINT *)pvParam)[0], ((UINT *)pvParam)[1], ((UINT *)pvParam)[2]);
			break;
#ifndef MS_WINCE
		case SPI_GETANIMATION:
			ret=PyInt_FromLong(((ANIMATIONINFO *)pvParam)->iMinAnimate);
			break;
		// these 2 can be a get or set, use Param==Py_None to mean a get
		case SPI_ICONHORIZONTALSPACING:
		case SPI_ICONVERTICALSPACING:
			if (obParam==Py_None)
				ret=PyLong_FromUnsignedLong(uintParam);
			else{
				Py_INCREF(Py_None);
				ret=Py_None;
				}
			break;

		case SPI_GETNONCLIENTMETRICS: {
			NONCLIENTMETRICS *p = (NONCLIENTMETRICS *)pvParam;
			ret = Py_BuildValue("{s:i,s:i,s:i,s:i,s:i,s:N,s:i,s:i,s:N,s:i,s:i,s:N,s:N,s:N}",
					"iBorderWidth", p->iBorderWidth,
					"iScrollWidth", p->iScrollWidth,
					"iScrollHeight", p->iScrollHeight,
					"iCaptionWidth", p->iCaptionWidth,
					"iCaptionHeight", p->iCaptionHeight,
					"lfCaptionFont", new PyLOGFONT(&p->lfCaptionFont),
					"iSmCaptionWidth", p->iSmCaptionWidth,
					"iSmCaptionHeight", p->iSmCaptionHeight,
					"lfSmCaptionFont", new PyLOGFONT(&p->lfSmCaptionFont),
					"iMenuWidth", p->iMenuWidth,
					"iMenuHeight", p->iMenuHeight,
					"lfMenuFont", new PyLOGFONT(&p->lfMenuFont),
					"lfStatusFont", new PyLOGFONT(&p->lfStatusFont),
					"lfMessageFont",new PyLOGFONT(&p->lfMessageFont));
			break;
		}
#endif	// !MS_WINCE

		default:
			Py_INCREF(Py_None);
			ret=Py_None;
		}

	done:
	switch (Action){
#ifndef MS_WINCE
		case SPI_GETDESKWALLPAPER:
		case SPI_GETICONTITLELOGFONT:
		case SPI_GETANIMATION:
		case SPI_SETANIMATION:
#endif	// !MS_WINCE
		case SPI_GETNONCLIENTMETRICS:
		case SPI_SETNONCLIENTMETRICS:
		case SPI_GETMOUSE:
		case SPI_SETMOUSE:
			if (pvParam!=NULL)
				free(pvParam);
			break;
		case SPI_SETDESKWALLPAPER:
			PyWinObject_FreeTCHAR((TCHAR *)pvParam);
			break;
		}
	return ret;
}
PyCFunction pfnPySystemParametersInfo=(PyCFunction)PySystemParametersInfo;
%}

%native (SetLayeredWindowAttributes) pfnPySetLayeredWindowAttributes;
%{
// @pyswig |SetLayeredWindowAttributes|Sets the opacity and transparency color key of a layered window.
// @comm This function only exists on Win2k and later
// @comm Accepts keyword arguments
PyObject *PySetLayeredWindowAttributes(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(SetLayeredWindowAttributes);
	static char *keywords[]={"hwnd", "Key", "Alpha", "Flags",  NULL};
	HWND hwnd;
	COLORREF Key;
	BYTE Alpha;
	DWORD Flags;
	PyObject *obhwnd;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Okbk:SetLayeredWindowAttributes", keywords,
		&obhwnd,	// @pyparm <o PyHANDLE>|hwnd||handle to the layered window
		&Key,		// @pyparm int|Key||Specifies the color key.  Use <om win32api.RGB> to generate value.
		&Alpha,		// @pyparm int|Alpha||Opacity, in the range 0-255
		&Flags))	// @pyparm int|Flags||Combination of win32con.LWA_* values
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd, FALSE))
		return NULL;
	if (!(*pfnSetLayeredWindowAttributes)(hwnd,Key,Alpha,Flags))
		return PyWin_SetAPIError("SetLayeredWindowAttributes");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnPySetLayeredWindowAttributes=(PyCFunction)PySetLayeredWindowAttributes;
%}

%native (GetLayeredWindowAttributes) pfnPyGetLayeredWindowAttributes;
%{
// @pyswig (int,int,int)|GetLayeredWindowAttributes|Retrieves the layering parameters of a window with the WS_EX_LAYERED extended style
// @comm This function only exists on WinXP and later.
// @comm Accepts keyword arguments.
// @rdesc Returns a tuple of (color key, alpha, flags)
PyObject *PyGetLayeredWindowAttributes(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(GetLayeredWindowAttributes);
	static char *keywords[]={"hwnd",  NULL};
	HWND hwnd;
	COLORREF Key;
	BYTE Alpha;
	DWORD Flags;
	PyObject *obhwnd;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetLayeredWindowAttributes", keywords,
		&obhwnd))	// @pyparm <o PyHANDLE>|hwnd||Handle to a layered window
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd, FALSE))
		return NULL;
	if (!(*pfnGetLayeredWindowAttributes)(hwnd, &Key, &Alpha, &Flags))
		return PyWin_SetAPIError("GetLayeredWindowAttributes");
	return Py_BuildValue("kbk", Key, Alpha, Flags);
}
PyCFunction pfnPyGetLayeredWindowAttributes=(PyCFunction)PyGetLayeredWindowAttributes;
%}
