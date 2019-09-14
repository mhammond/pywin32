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
// #define UNICODE
// #define _UNICODE // for CRT string functions
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
#include "Dbt.h" // device notification
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
typedef BOOL (WINAPI *UpdateLayeredWindowfunc)(HWND,HDC,POINT *,SIZE *,HDC,POINT *,COLORREF,BLENDFUNCTION *,DWORD);
static UpdateLayeredWindowfunc pfnUpdateLayeredWindow=NULL;
typedef BOOL (WINAPI *AngleArcfunc)(HDC, int, int, DWORD, FLOAT, FLOAT);
static AngleArcfunc pfnAngleArc=NULL;
typedef BOOL (WINAPI *PlgBltfunc)(HDC,CONST POINT *,HDC,int,int,int,int,HBITMAP,int,int);
static PlgBltfunc pfnPlgBlt=NULL;
typedef BOOL (WINAPI *GetWorldTransformfunc)(HDC,XFORM *);
static GetWorldTransformfunc pfnGetWorldTransform=NULL;
typedef BOOL (WINAPI *SetWorldTransformfunc)(HDC,XFORM *);
static SetWorldTransformfunc pfnSetWorldTransform=NULL;
typedef BOOL (WINAPI *ModifyWorldTransformfunc)(HDC,XFORM *,DWORD);
static ModifyWorldTransformfunc pfnModifyWorldTransform=NULL;
typedef BOOL (WINAPI *CombineTransformfunc)(LPXFORM,CONST XFORM *,CONST XFORM *);
static CombineTransformfunc pfnCombineTransform=NULL;
typedef BOOL (WINAPI *GradientFillfunc)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);
static GradientFillfunc pfnGradientFill=NULL;
typedef BOOL (WINAPI *TransparentBltfunc)(HDC,int,int,int,int,HDC,int,int,int,int,UINT);
static TransparentBltfunc pfnTransparentBlt=NULL;
typedef BOOL (WINAPI *MaskBltfunc)(HDC,int,int,int,int,HDC,int,int,HBITMAP,int,int,DWORD);
static MaskBltfunc pfnMaskBlt=NULL;
typedef BOOL (WINAPI *AlphaBlendfunc)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
static AlphaBlendfunc pfnAlphaBlend=NULL;
typedef BOOL (WINAPI *AnimateWindowfunc)(HWND,DWORD,DWORD);
static AnimateWindowfunc pfnAnimateWindow=NULL;
typedef BOOL (WINAPI *GetMenuInfofunc)(HMENU, LPCMENUINFO);
static GetMenuInfofunc pfnGetMenuInfo=NULL;
typedef BOOL (WINAPI *SetMenuInfofunc)(HMENU, LPCMENUINFO);
static GetMenuInfofunc pfnSetMenuInfo=NULL;
typedef DWORD (WINAPI *GetLayoutfunc)(HDC);
static GetLayoutfunc pfnGetLayout=NULL;
typedef DWORD (WINAPI *SetLayoutfunc)(HDC, DWORD);
static SetLayoutfunc pfnSetLayout=NULL;
typedef int (WINAPI *DrawTextWfunc)(HDC,LPWSTR,int,LPRECT,UINT);
static DrawTextWfunc pfnDrawTextW = NULL;

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

// @object PyBLENDFUNCTION|Tuple of four small ints used to fill a BLENDFUNCTION struct
// Each int must fit in a byte (0-255).
// @pyseeapi BLENDFUNCTION
BOOL PyWinObject_AsBLENDFUNCTION(PyObject *obbl, BLENDFUNCTION *pbl)
{
	if (!PyTuple_Check(obbl)){
		PyErr_SetString(PyExc_TypeError, "BLENDFUNCTION must be a tuple of four small ints (0-255)");
		return FALSE;
		}
	return PyArg_ParseTuple(obbl, "BBBB:BLENDFUNCTION",
		&pbl->BlendOp,				// @tupleitem 0|int|BlendOp|Only defined value is AC_SRC_OVER (0)
		&pbl->BlendFlags,			// @tupleitem 1|int|BlendFlags|None currently defined, must be 0
		&pbl->SourceConstantAlpha,	// @tupleitem 2|int|SourceConstantAlpha|Transparency to be applied to entire source. (255 is opaque)
		&pbl->AlphaFormat);			// @tupleitem 3|int|AlphaFormat|Only defined flag is AC_SRC_ALPHA, used when src bitmap contains per-pixel alpha
}

// @object PySIZE|Tuple of two ints (cx,cy) representing a SIZE struct
BOOL PyWinObject_AsSIZE(PyObject *obsize, SIZE *psize)
{
	if (!PyTuple_Check(obsize)){
		PyErr_SetString(PyExc_TypeError, "SIZE must be a tuple of 2 ints (x,y)");
		return FALSE;
		}
	return PyArg_ParseTuple(obsize, "ll;SIZE must be a tuple of 2 ints (x,y)", 
		&psize->cx, &psize->cy);
}

// @object PyGdiHANDLE|Gdi objects such as brush (HBRUSH), pen (HPEN), font (HFONT), region (HRGN), bitmap (HBITMAP)
//	On destruction, the handle is closed using DeleteObject.  The object's Close() method also calls DeleteObject.
//	The gdi object should be deselected from any DC that it is selected into before it's closed.
//	Inherits the methods and properties of <o PyHANDLE>.
class PyGdiHANDLE: public PyHANDLE
{
public:
	PyGdiHANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void){
		BOOL ret=DeleteObject(m_handle);
		if (!ret)
			PyWin_SetAPIError("DeleteObject");
		m_handle = 0;
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PyGdiHANDLE";
		}
};

PyObject *PyWinObject_FromGdiHANDLE(HGDIOBJ h)
{
	PyObject *ret=new PyGdiHANDLE(h);
	if (ret==NULL) {
            DeleteObject(h);
	    PyErr_NoMemory();
        }
	return ret;
}
%}

// SWIG support for GDI handles.
%typemap(python,except) HPEN, HBRUSH, HFONT, HRGN, HBITMAP {
	Py_BEGIN_ALLOW_THREADS
	$function
	Py_END_ALLOW_THREADS
	if ($source==NULL){
		$cleanup
		return PyWin_SetAPIError("$name");
		}
}
/* ??? If you don't map these to a known type, swig obstinately ignores the input and output typemaps and tries to treat them as pointers.
		However, it doesn't seem to matter what you typedef them to as long as they have in and out typemaps. ??? */
typedef float HPEN, HBRUSH, HFONT, HRGN, HBITMAP;
%typemap(python,out) HPEN, HBRUSH, HFONT, HRGN, HBITMAP{
	$target = PyWinObject_FromGdiHANDLE($source);
}
%typemap(python,in) HPEN, HBRUSH, HFONT, HRGN, HBITMAP{
	if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}
%typemap(python,in) HRGN INPUT_NULLOK, HBRUSH INPUT_NULLOK, HBITMAP INPUT_NULLOK{
	if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}

%typedef int int_regiontype;
// Several functions return an int containg a region type (NULLREGION,SIMPLEREGION,COMPLEXREGION) or ERROR on failure
%typemap(python,except) int_regiontype{
	Py_BEGIN_ALLOW_THREADS
	$function
	Py_END_ALLOW_THREADS
	if ($source==ERROR){
		$cleanup
		return PyWin_SetAPIError("$name");
		}
}

%{
// @object PyHDEVNOTIFY|A handle returned by RegisterDeviceNotifications which
//      automatically calls UnregisterDeviceNotification on destruction.
//	Inherits the methods and properties of <o PyHANDLE>.
class PyHDEVNOTIFY: public PyHANDLE
{
public:
	PyHDEVNOTIFY(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void){
		BOOL ret=UnregisterDeviceNotification(m_handle);
		if (!ret)
			PyWin_SetAPIError("UnregisterDeviceNotification");
		m_handle = 0;
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PyHDEVNOTIFY";
		}
};

PyObject *PyWinObject_FromHDEVNOTIFY(HGDIOBJ h)
{
	PyObject *ret=new PyHDEVNOTIFY(h);
	if (ret==NULL) {
            UnregisterDeviceNotification(h);
	    PyErr_NoMemory();
        }
	return ret;
}
%}
// TODO: SWIG support for PyHDEVNOTIFY - but SWIG currently doesn't use it.


// Written to the module init function.
%init %{
PyEval_InitThreads(); /* Start the interpreter's thread-awareness */
PyDict_SetItemString(d, "dllhandle", PyWinLong_FromVoidPtr(g_dllhandle));
PyDict_SetItemString(d, "error", PyWinExc_ApiError);

if (PyType_Ready(&PyWNDCLASSType) == -1 ||
	PyType_Ready(&PyBITMAPType) == -1 ||
	PyType_Ready(&PyLOGFONTType) == -1)
	PYWIN_MODULE_INIT_RETURN_ERROR;

// Expose the window procedure and window class dicts to aid debugging
g_AtomMap = PyDict_New();
g_HWNDMap = PyDict_New();
g_DLGMap = PyDict_New();
#ifdef Py_DEBUG
PyDict_SetItemString(d, "g_AtomMap", g_AtomMap);
PyDict_SetItemString(d, "g_HWNDMap", g_HWNDMap);
PyDict_SetItemString(d, "g_DLGMap", g_DLGMap);
#endif

PyDict_SetItemString(d, "UNICODE",
#ifdef UNICODE
					Py_True
#else
					Py_False
#endif
						);

// hack borrowed from win32security since version of SWIG we use doesn't do keyword arguments
#ifdef WINXPGUI
for (PyMethodDef *pmd = winxpguiMethods; pmd->ml_name; pmd++)
#else
for (PyMethodDef *pmd = win32guiMethods; pmd->ml_name; pmd++)
#endif
	if	 (strcmp(pmd->ml_name, "SetLayeredWindowAttributes")==0
		||strcmp(pmd->ml_name, "GetLayeredWindowAttributes")==0
		||strcmp(pmd->ml_name, "UpdateLayeredWindow")==0
		||strcmp(pmd->ml_name, "AnimateWindow")==0
		||strcmp(pmd->ml_name, "GetOpenFileNameW")==0
		||strcmp(pmd->ml_name, "GetSaveFileNameW")==0
		||strcmp(pmd->ml_name, "SystemParametersInfo")==0
		||strcmp(pmd->ml_name, "DrawTextW")==0
		)
		pmd->ml_flags = METH_VARARGS | METH_KEYWORDS;

HMODULE hmodule=GetModuleHandle(TEXT("user32.dll"));
if (hmodule==NULL)
	hmodule=LoadLibrary(TEXT("user32.dll"));
if (hmodule){
	pfnSetLayeredWindowAttributes=(SetLayeredWindowAttributesfunc)GetProcAddress(hmodule,"SetLayeredWindowAttributes");
	pfnGetLayeredWindowAttributes=(GetLayeredWindowAttributesfunc)GetProcAddress(hmodule,"GetLayeredWindowAttributes");
	pfnUpdateLayeredWindow=(UpdateLayeredWindowfunc)GetProcAddress(hmodule,"UpdateLayeredWindow");
	pfnAnimateWindow=(AnimateWindowfunc)GetProcAddress(hmodule,"AnimateWindow");
	pfnGetMenuInfo=(GetMenuInfofunc)GetProcAddress(hmodule,"GetMenuInfo");
	pfnSetMenuInfo=(SetMenuInfofunc)GetProcAddress(hmodule,"SetMenuInfo");
	pfnDrawTextW=(DrawTextWfunc)GetProcAddress(hmodule, "DrawTextW");
	}

hmodule=GetModuleHandle(TEXT("gdi32.dll"));
if (hmodule==NULL)
	hmodule=LoadLibrary(TEXT("gdi32.dll"));
if (hmodule){
	pfnAngleArc=(AngleArcfunc)GetProcAddress(hmodule,"AngleArc");
	pfnPlgBlt=(PlgBltfunc)GetProcAddress(hmodule,"PlgBlt");
	pfnGetWorldTransform=(GetWorldTransformfunc)GetProcAddress(hmodule,"GetWorldTransform");
	pfnSetWorldTransform=(SetWorldTransformfunc)GetProcAddress(hmodule,"SetWorldTransform");
	pfnModifyWorldTransform=(ModifyWorldTransformfunc)GetProcAddress(hmodule,"ModifyWorldTransform");
	pfnCombineTransform=(CombineTransformfunc)GetProcAddress(hmodule,"CombineTransform");
	pfnMaskBlt=(MaskBltfunc)GetProcAddress(hmodule,"MaskBlt");
	pfnGetLayout=(GetLayoutfunc)GetProcAddress(hmodule,"GetLayout");
	pfnSetLayout=(SetLayoutfunc)GetProcAddress(hmodule,"SetLayout");
	}

hmodule=GetModuleHandle(TEXT("msimg32.dll"));
if (hmodule==NULL)
	hmodule=LoadLibrary(TEXT("msimg32.dll"));
if (hmodule){
	pfnGradientFill=(GradientFillfunc)GetProcAddress(hmodule,"GradientFill");
	pfnTransparentBlt=(TransparentBltfunc)GetProcAddress(hmodule,"TransparentBlt");
	pfnAlphaBlend=(AlphaBlendfunc)GetProcAddress(hmodule,"AlphaBlend");
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

// Handles types with no specific PyHANDLE subclass, returned to Python as plain ints or longs
typedef float HDC, HCURSOR, HINSTANCE, HMENU, HICON, HGDIOBJ, HIMAGELIST, HACCEL;
%typemap(python, in) HDC, HCURSOR, HINSTANCE, HMENU, HICON, HGDIOBJ, HIMAGELIST, HACCEL{
	if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}
%typemap(python, out) HDC, HCURSOR, HINSTANCE, HMENU, HICON, HGDIOBJ, HIMAGELIST, HACCEL{
	$target=PyWinLong_FromHANDLE($source);
}

%apply COLORREF {long};
typedef long COLORREF

typedef HANDLE WPARAM;
typedef HANDLE LPARAM;
typedef HANDLE LRESULT;
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
    PyObject *o = PyWinObject_FromMSG($source);
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
    if (!PyWinObject_AsMSG($source, $target))
        return NULL;
}
%typemap(python,ignore) RECT *OUTPUT(RECT rect_output)
{
  $target = &rect_output;
}

%typemap(python,in) RECT *INPUT(RECT rect_input)
{
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &rect_input.left, &rect_input.top, &rect_input.right, &rect_input.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
		}
		$target = &rect_input;
	} else {
		return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
	}
}

%typemap(python,in) RECT *INPUT_NULLOK(RECT rect_input_nullok)
{
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &rect_input_nullok.left, &rect_input_nullok.top, &rect_input_nullok.right, &rect_input_nullok.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers or None", "$name");
		}
		$target = &rect_input_nullok;
	} else {
		if ($source == Py_None) {
            $target = NULL;
        } else {
            PyErr_SetString(PyExc_TypeError, "This param must be a tuple of four integers or None");
            return NULL;
		}
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

%typemap(python,ignore) POINT *OUTPUT(POINT point_output)
{
  $target = &point_output;
}

%typemap(python,in) POINT *INPUT(POINT point_input) {
	if (!PyWinObject_AsPOINT($source, &point_input))
		return NULL;
	$target = &point_input;
}

%typemap(python,in) POINT INPUT {
	if (!PyWinObject_AsPOINT($source, &$target))
		return NULL;
}


%typemap(python,in) POINT *BOTH = POINT *INPUT;
%typemap(python,argout) POINT *BOTH = POINT *OUTPUT;

%typemap(python,in) SIZE *INPUT(SIZE size_input){
	if (!PyWinObject_AsSIZE($source, &size_input))
		return NULL;
	$target = &size_input;
}

// @object PyICONINFO|Tuple describing an icon or cursor
// @pyseeapi ICONINFO
%typemap(python,in) ICONINFO *INPUT(ICONINFO iconinfo_input) {
	PyObject *obmask, *obcolor;
	if (PyTuple_Check($source)) {
		if (!PyArg_ParseTuple($source, "lllOO", 
			&iconinfo_input.fIcon,		// @tupleitem 0|boolean|Icon|True indicates an icon, False for a cursor
			&iconinfo_input.xHotspot,	// @tupleitem 1|int|xHotSpot|For a cursor, X coord of hotspot.  Ignored for icons
			&iconinfo_input.yHotspot,	// @tupleitem 2|int|yHotSpot|For a cursor, Y coord of hotspot.  Ignored for icons
            &obmask,					// @tupleitem 3|<o PyGdiHANDLE>|hbmMask|Monochrome mask bitmap
			&obcolor))					// @tupleitem 4|<o PyGdiHANDLE>|hbmColor|Color bitmap, may be None for black and white icon
			return PyErr_Format(PyExc_TypeError, "%s: an ICONINFO must be a tuple of (int,int,int,HANDLE,HANDLE)", "$name");

		if (!PyWinObject_AsHANDLE(obmask, (HANDLE *)&iconinfo_input.hbmMask))
			return NULL;
		if (!PyWinObject_AsHANDLE(obcolor, (HANDLE *)&iconinfo_input.hbmColor))
			return NULL;
		$target = &iconinfo_input;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: an ICONINFO must be a tuple of (int,int,int,HANDLE,HANDLE)", "$name");
	}
}

%typemap(python,argout) ICONINFO *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("lllNN", $source->fIcon, $source->xHotspot, $source->yHotspot, 
		PyWinObject_FromGdiHANDLE($source->hbmMask), PyWinObject_FromGdiHANDLE($source->hbmColor));
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

%typemap(python,in) BLENDFUNCTION *INPUT(BLENDFUNCTION bf_input) {
	if (!PyWinObject_AsBLENDFUNCTION($source, &bf_input))
		return NULL;
	$target = &bf_input;
}

%typemap(python,argout) PAINTSTRUCT *OUTPUT {
    PyObject *o;
    o = Py_BuildValue("(Nl(iiii)llN)",
                PyWinLong_FromHANDLE($source->hdc),
                $source->fErase,
                $source->rcPaint.left, $source->rcPaint.top, $source->rcPaint.right, $source->rcPaint.bottom,
                $source->fRestore,
                $source->fIncUpdate,
                PyString_FromStringAndSize((char *)$source->rgbReserved,sizeof($source->rgbReserved)));
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

%typemap(python,ignore) PAINTSTRUCT *OUTPUT(PAINTSTRUCT ps_output)
{
  $target = &ps_output;
}

%typemap(python,in) PAINTSTRUCT *INPUT(PAINTSTRUCT ps_input) {
    char *szReserved;
    Py_ssize_t lenReserved;
	PyObject *obdc, *obReserved;
	if (PyTuple_Check($source)) {
		if (!PyArg_ParseTuple($source,
			"Ol(iiii)llO",
			&obdc,
			&ps_input.fErase,
			&ps_input.rcPaint.left, &ps_input.rcPaint.top, &ps_input.rcPaint.right, &ps_input.rcPaint.bottom,
			&ps_input.fRestore,
			&ps_input.fIncUpdate,
			&obReserved))
			return NULL;
		if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&ps_input.hdc))
			return NULL;
		if (PyString_AsStringAndSize(obReserved, &szReserved, &lenReserved)==-1)
			return NULL;
        if (lenReserved != sizeof(ps_input.rgbReserved))
            return PyErr_Format(PyExc_ValueError, "%s: last element must be string of %d bytes",
                                "$name", sizeof(ps_input.rgbReserved));
        memcpy(&ps_input.rgbReserved, szReserved, sizeof(ps_input.rgbReserved));
		$target = &ps_input;
    } else {
		return PyErr_Format(PyExc_TypeError, "%s: a PAINTSTRUCT must be a tuple", "$name");
	}
}

// @object TRACKMOUSEEVENT|A tuple of (dwFlags, hwndTrack, dwHoverTime)
%typemap(python,in) TRACKMOUSEEVENT *INPUT(TRACKMOUSEEVENT e){
	PyObject *obhwnd;
	e.cbSize = sizeof e;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "lOl", &e.dwFlags, &obhwnd, &e.dwHoverTime) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: a TRACKMOUSEEVENT must be a tuple of 3 integers", "$name");
		}
		if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&e.hwndTrack))
			return NULL;
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
			if (key==NULL){
				HandleError("Internal error converting Msg param of window procedure");
				return FALSE;
				}
			obFunc = PyDict_GetItem(obFuncOrMap, key);
			Py_DECREF(key);
		} else {
			obFunc = obFuncOrMap;
		}
	}
	if (obFunc==NULL)
		return FALSE;

	// We are dispatching to Python...
	PyObject *args = Py_BuildValue("NlNN", PyWinLong_FromHANDLE(hWnd), uMsg, 
		PyWinObject_FromPARAM(wParam), PyWinObject_FromPARAM(lParam));
	if (args==NULL){
		HandleError("Error building argument tuple for python callback");
		return FALSE;
		}
	PyObject *ret = PyObject_CallObject(obFunc, args);
	Py_DECREF(args);
	LRESULT rc = 0;
	if (ret){
		if (!PyWinObject_AsPARAM(ret, (LPARAM *)&rc))
			HandleError("WNDPROC return value cannot be converted to LRESULT");
		Py_DECREF(ret);
		}
	else
		HandleError("Python WNDPROC handler failed");
	*prc = rc;
	return TRUE;
}

LRESULT CALLBACK PyWndProcClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PyObject *obFunc = (PyObject *)GetClassLongPtr( hWnd, 0);
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
	PyObject *obFunc = (PyObject *)GetClassLongPtr( hWnd, 0);
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
	PyObject *key = PyWinLong_FromHANDLE(hWnd);
	PyObject *obInfo = PyDict_GetItem(g_HWNDMap, key);
	Py_DECREF(key);
	MYWNDPROC oldWndProc = NULL;
	PyObject *obFunc = NULL;
	if (obInfo!=NULL) { // Is one of ours!
		obFunc = PyTuple_GET_ITEM(obInfo, 0);
		PyObject *obOldWndProc = PyTuple_GET_ITEM(obInfo, 1);
		PyWinLong_AsVoidPtr(obOldWndProc, (void **)&oldWndProc);
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
		PyObject *key = PyWinLong_FromHANDLE(hWnd);
		if (PyDict_DelItem(g_HWNDMap, key) != 0)
			PyErr_Clear();
		Py_DECREF(key);
	}
	return rc;
}

INT_PTR CALLBACK PyDlgProcHDLG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL rc = FALSE;
	CEnterLeavePython _celp;
	if (uMsg==WM_INITDIALOG) {
		// The lparam is our PyObject.
		// Put our HWND in the map.
		PyObject *obTuple = (PyObject *)lParam;
		PyObject *obWndProc = PyTuple_GET_ITEM(obTuple, 0);
		// Replace the lParam with the one the user specified.
		lParam = 0;
		if (PyTuple_GET_ITEM(obTuple, 1) != Py_None)
			PyWinObject_AsPARAM( PyTuple_GET_ITEM(obTuple, 1), &lParam );

		PyObject *key = PyWinLong_FromHANDLE(hWnd);
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
	PyObject *key = PyWinLong_FromHANDLE(hWnd);
	obFunc = PyDict_GetItem(g_DLGMap, key);
	Py_XDECREF(key);
	if (!obFunc)
		PyErr_Clear();

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
		PyObject *key = PyWinLong_FromHANDLE(hWnd);

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

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyObject *PySetDialogProc(PyObject *self, PyObject *args);
	WNDCLASS m_WNDCLASS;
	PyObject *m_obMenuName, *m_obClassName, *m_obWndProc;
};
#define PyWNDCLASS_Check(ob)	((ob)->ob_type == &PyWNDCLASSType)

// @object PyWNDCLASS|A Python object, representing an WNDCLASS structure
// @comm Typically you create a PyWNDCLASS object, and set its properties.
// The object can then be passed to any function which takes an WNDCLASS object
PyTypeObject PyWNDCLASSType =
{
	PYWIN_OBJECT_HEAD
	"PyWNDCLASS",
	sizeof(PyWNDCLASS),
	0,
	PyWNDCLASS::deallocFunc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyWNDCLASS::getattro,	/* tp_getattro */
	PyWNDCLASS::setattro,	/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	PyWNDCLASS::methods,	/* tp_methods */
	PyWNDCLASS::members,	/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

#define OFF(e) offsetof(PyWNDCLASS, e)

/*static*/ struct PyMemberDef PyWNDCLASS::members[] = {
	{"style",            T_INT,  OFF(m_WNDCLASS.style)}, // @prop integer|style|
//	{"cbClsExtra",       T_INT,  OFF(m_WNDCLASS.cbClsExtra)}, // @prop integer|cbClsExtra|
	{"cbWndExtra",       T_INT,  OFF(m_WNDCLASS.cbWndExtra)}, // @prop integer|cbWndExtra|
	{NULL}

    // ack - these are also handled now explicitly, as T_LONGLONG is too 
    // stupid to handle ints :(
	// @prop integer|hInstance|
	// @prop integer|hIcon|
	// @prop integer|hCursor|
	// @prop integer|hbrBackground|
	// These 3 handled manually in PyWNDCLASS::getattro/setattro.  The pymeth below is used as an
	// end tag, so these props will be lost if below it
	// @prop string/<o PyUnicode>|lpszMenuName|
	// @prop string/<o PyUnicode>|lpszClassName|
	// @prop function|lpfnWndProc|

};

PyObject *PyWNDCLASS::PySetDialogProc(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SetDialogProc"))
		return NULL;
	PyWNDCLASS *pW = (PyWNDCLASS *)self;
	pW->m_WNDCLASS.lpfnWndProc = (WNDPROC)PyDlgProcClass;
	Py_INCREF(Py_None);
	return Py_None;
}

struct PyMethodDef PyWNDCLASS::methods[] = {
	{"SetDialogProc",    PyWNDCLASS::PySetDialogProc, 1}, 	// @pymeth SetDialogProc|Sets the WNDCLASS to be for a dialog box.
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

PyObject *PyWNDCLASS::getattro(PyObject *self, PyObject *obname)
{
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	PyWNDCLASS *pW = (PyWNDCLASS *)self;
	if (strcmp("lpszMenuName", name)==0) {
		PyObject *ret = pW->m_obMenuName ? pW->m_obMenuName : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	if (strcmp("lpszClassName", name)==0) {
		PyObject *ret = pW->m_obClassName ? pW->m_obClassName : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	if (strcmp("lpfnWndProc", name)==0) {
		PyObject *ret = pW->m_obWndProc ? pW->m_obWndProc : Py_None;
		Py_INCREF(ret);
		return ret;
	}
	if (strcmp("hInstance", name)==0)
		return PyWinLong_FromVoidPtr(pW->m_WNDCLASS.hInstance);

	if (strcmp("hIcon", name)==0)
		return PyWinLong_FromVoidPtr(pW->m_WNDCLASS.hIcon);

	if (strcmp("hCursor", name)==0)
		return PyWinLong_FromVoidPtr(pW->m_WNDCLASS.hCursor);

	if (strcmp("hbrBackground", name)==0)
		return PyWinLong_FromVoidPtr(pW->m_WNDCLASS.hbrBackground);

	return PyObject_GenericGetAttr(self, obname);
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

int PyWNDCLASS::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete WNDCLASS attributes");
		return -1;
	}
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return -1;
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
	if (strcmp("hInstance", name)==0)
		return PyWinLong_AsVoidPtr(v, (void **)&pW->m_WNDCLASS.hInstance) ? 0 : -1;

	if (strcmp("hIcon", name)==0)
		return PyWinLong_AsVoidPtr(v, (void **)&pW->m_WNDCLASS.hIcon) ? 0 : -1;

	if (strcmp("hCursor", name)==0)
		return PyWinLong_AsVoidPtr(v, (void **)&pW->m_WNDCLASS.hCursor) ? 0 : -1;

	if (strcmp("hbrBackground", name)==0)
		return PyWinLong_AsVoidPtr(v, (void **)&pW->m_WNDCLASS.hbrBackground) ? 0 : -1;

	return PyObject_GenericSetAttr(self, obname, v);
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
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
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
	PYWIN_OBJECT_HEAD
	"PyBITMAP",
	sizeof(PyBITMAP),
	0,
	PyBITMAP::deallocFunc,	/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyBITMAP::getattro,		/* tp_getattro */
	PyBITMAP::setattro,		/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	0,						/* tp_methods */
	PyBITMAP::members,		/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyBITMAP, e)

/*static*/ struct PyMemberDef PyBITMAP::members[] = {
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

PyObject *PyBITMAP::getattro(PyObject *self, PyObject *obname)
{
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	PyBITMAP *pB = (PyBITMAP *)self;
	if (strcmp("bmBits", name)==0) {
		return PyWinLong_FromVoidPtr(pB->m_BITMAP.bmBits);
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyBITMAP::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete BITMAP attributes");
		return -1;
	}
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return -1;
	if (strcmp("bmBits", name)==0) {
		PyBITMAP *pB = (PyBITMAP *)self;
		if (!PyWinLong_AsVoidPtr(v, &pB->m_BITMAP.bmBits))
			return -1;
		return 0;
	}
	return PyObject_GenericSetAttr(self, obname, v);
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

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
	LOGFONT m_LOGFONT;
};
#define PyLOGFONT_Check(ob)	((ob)->ob_type == &PyLOGFONTType)

// @object PyLOGFONT|A Python object, representing an PyLOGFONT structure
// @comm Typically you create a PyLOGFONT object, and set its properties.
// The object can then be passed to any function which takes an LOGFONT object
PyTypeObject PyLOGFONTType =
{
	PYWIN_OBJECT_HEAD
	"PyLOGFONT",
	sizeof(PyLOGFONT),
	0,
	PyLOGFONT::deallocFunc,	/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyLOGFONT::getattro,	/* tp_getattro */
	PyLOGFONT::setattro,	/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	0,						/* tp_methods */
	PyLOGFONT::members,		/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};
#undef OFF
#define OFF(e) offsetof(PyLOGFONT, e)

/*static*/ struct PyMemberDef PyLOGFONT::members[] = {
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

PyObject *PyLOGFONT::getattro(PyObject *self, PyObject *obname)
{
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	PyLOGFONT *pL = (PyLOGFONT *)self;
	if (strcmp("lfFaceName", name)==0) {
		return PyWinObject_FromTCHAR(pL->m_LOGFONT.lfFaceName);
	}
	return PyObject_GenericGetAttr(self, obname);
}

int PyLOGFONT::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete LOGFONT attributes");
		return -1;
	}
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return -1;
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
	return PyObject_GenericSetAttr(self, obname, v);
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
	PyObject *obProc, *obdc;
	PyObject *obExtra = Py_None;
	HDC hdc;
	// @pyparm <o PyHANDLE>|hdc||Handle to a device context for which to enumerate available fonts
	// @pyparm string/<o PyUnicode>|Family||Family of fonts to enumerate. If none, first member of each font family will be returned.
	// @pyparm function|EnumFontFamProc||The Python function called with each font family. This function is called with 4 arguments.
	// @pyparm object|Param||An arbitrary object to be passed to the callback function
	// @comm The parameters that the callback function will receive are as follows:<nl>
	//	<o PyLOGFONT> - contains the font parameters<nl>
	//	None - Placeholder for a TEXTMETRIC structure, not supported yet<nl>
	//	int - Font type, combination of DEVICE_FONTTYPE, RASTER_FONTTYPE, TRUETYPE_FONTTYPE<nl>
	//	object - The Param originally passed in to EnumFontFamilies

	if (!PyArg_ParseTuple(args, "OOO|O", &obdc, &obFamily, &obProc, &obExtra))
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyCallable_Check(obProc)) {
		PyErr_SetString(PyExc_TypeError, "The 3rd argument must be callable");
		return NULL;
	}
	TCHAR *szFamily;
	if (!PyWinObject_AsTCHAR(obFamily, &szFamily, TRUE))
		return NULL;
	PyObject *lparam = Py_BuildValue("OO", obProc, obExtra);
	int rc = EnumFontFamilies(hdc, szFamily, EnumFontFamProc, (LPARAM)lparam);
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
	if (!PyArg_ParseTuple(args, "O:set_logger", &logger))
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

// @pyswig <o PyGdiHandle>|CreateFontIndirect|function creates a logical font that has the specified characteristics.
// The font can subsequently be selected as the current font for any device context.
HFONT CreateFontIndirect(LOGFONT *lf);	// @pyparm <o PyLOGFONT>|lplf||A LOGFONT object as returned by <om win32gui.LOGFONT> 

%{
// @pyswig object|GetObject|Returns a struct containing the parameters used to create a GDI object
static PyObject *PyGetObject(PyObject *self, PyObject *args)
{
	HGDIOBJ hob;
	PyObject *ob;
	// @pyparm <o PyHANDLE>|handle||Handle to the object.
	if (!PyArg_ParseTuple(args, "O", &ob))
		return NULL;
	if (!PyWinObject_AsHANDLE(ob, &hob))
		return NULL;
	DWORD typ = GetObjectType(hob);
	if (typ==0)
		return PyWin_SetAPIError("GetObjectType");
	// @comm The result depends on the type of the handle.
	switch (typ) {
		// @flagh Object type as determined by <om win32gui.GetObjectType>|Returned object
		// @flag OBJ_FONT|<o PyLOGFONT>
		case OBJ_FONT: {
			LOGFONT lf;
			if (GetObject(hob, sizeof(LOGFONT), &lf)==0)
				return PyWin_SetAPIError("GetObject");
			return new PyLOGFONT(&lf);
		}
		// @flag OBJ_BITMAP|<o PyBITMAP>
		case OBJ_BITMAP: {
			BITMAP bm;
			if (GetObject(hob, sizeof(BITMAP), &bm)==0)
				return PyWin_SetAPIError("GetObject");
			return new PyBITMAP(&bm);
		}
		// @flag OBJ_PEN|Dict representing a LOGPEN struct
		case OBJ_PEN:{
			LOGPEN lp;
			if (GetObject(hob, sizeof(LOGPEN), &lp)==0)
				return PyWin_SetAPIError("GetObject");
			return Py_BuildValue("{s:I, s:l, s:k}",
				"Style", lp.lopnStyle,
				"Width", lp.lopnWidth.x,	// Docs say y member is not used, so ignore it
				"Color", lp.lopnColor);
		}
		default:
			PyErr_Format(PyExc_ValueError, "This GDI object type is not supported: %d", typ);
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
	PyObject *ob;
	// @pyparm <o PyHANDLE>|h||A handle to a GDI object
	if (!PyArg_ParseTuple(args, "O:GetObjectType", &ob))
		return NULL;
	if (!PyWinObject_AsHANDLE(ob, &h))
		return NULL;
	t=GetObjectType(h);
	if (t==0)
		return PyWin_SetAPIError("GetObjectType");
	return PyLong_FromUnsignedLong(t);
}
%}
%native (GetObjectType) PyGetObjectType;

%{
// NOTE: PyMakeBuffer() is a dumb name for lots of reasons, including that
// it implies the memory is "new" and "owned" by the caller.  The "natural"
// order of the params is wrong too.  So it's deprecated!

static PyObject *PyMakeBuffer(PyObject *self, PyObject *args)
{
	PyErr_Warn(PyExc_PendingDeprecationWarning, "PyMakeBuffer is deprecated; use PyGetMemory instead");
	size_t len;
	void *addr=NULL;
#ifdef _WIN64
	static char *input_fmt="L|L:PyMakeBuffer";
#else
	static char *input_fmt="l|l:PyMakeBuffer";
#endif
	if (!PyArg_ParseTuple(args, input_fmt, &len,&addr))
		return NULL;

	if(NULL == addr) 
		return PyBuffer_New(len);
	else {
		if (IsBadReadPtr(addr, len)) {
			PyErr_SetString(PyExc_ValueError,
			                "The value is not a valid address for reading");
			return NULL;
		}
		return PyBuffer_FromMemory(addr, len);
	}
}
%}
%native (PyMakeBuffer) PyMakeBuffer;

%{
// @pyswig object|PyGetMemory|Returns a buffer object from and address and length
static PyObject *PyGetMemory(PyObject *self, PyObject *args)
{
	void *addr;
	size_t len;
#ifdef _WIN64
	static char *input_fmt="LL:PyGetMemory";
#else
	static char *input_fmt="ll:PyGetMemory";
#endif
	// @pyparm int|addr||Address of the memory to reference.
	// @pyparm int|len||Number of bytes to return.
	// @comm If zero is passed a ValueError will be raised.
	if (!PyArg_ParseTuple(args, input_fmt, &addr, &len))
		return NULL;
	if (IsBadReadPtr(addr, len)) {
		PyErr_SetString(PyExc_ValueError,
		                "The value is not a valid address for reading");
		return NULL;
	}
	return PyBuffer_FromMemory(addr, len);
}
%}
%native (PyGetMemory) PyGetMemory;

%{
// @pyswig string|PyGetString|Returns a string from an address.
// @rdesc If win32gui.UNICODE is True, this will return a unicode object.
static PyObject *PyGetString(PyObject *self, PyObject *args)
{
	TCHAR *addr = 0;
	size_t len = -1;
#ifdef _WIN64
	static char *input_fmt="L|L:PyGetString";
#else
	static char *input_fmt="l|l:PyGetString";
#endif
	// @pyparm int|addr||Address of the memory to reference
	// @pyparm int|len||Number of characters to read.  If not specified, the
	// string must be NULL terminated.
	if (!PyArg_ParseTuple(args, input_fmt, &addr, &len))
		return NULL;
	if (addr==NULL){
		PyErr_SetString(PyExc_ValueError, "PyGetString: NULL is not valid pointer");
		return NULL;
	}
	if (len != -1){
		if (IsBadReadPtr(addr, len)) {
			PyErr_SetString(PyExc_ValueError, "The value is not a valid address for reading");
			return NULL;
			}
		return PyWinObject_FromTCHAR(addr, len);
	}
	// This should probably be in a __try just in case.
	if (IsBadStringPtr(addr, (DWORD_PTR)-1)) {
		PyErr_SetString(PyExc_ValueError, "The value is not a valid null-terminated string");
		return NULL;
	}
	return PyWinObject_FromTCHAR(addr);
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
	size_t maxLen = 0;
#ifdef _WIN64
	static char *input_fmt="LO|L:PySetString";
#else
	static char *input_fmt="lO|l:PySetString";
#endif

	// @pyparm int|addr||Address of the memory to reference 
	// @pyparm str|String||The string to copy
	// @pyparm int|maxLen||Maximum number of chars to copy (optional)
	if (!PyArg_ParseTuple(args, input_fmt, &addr,&str,&maxLen))
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
	void *addr;
	const void *src;
	PyObject *obaddr, *obsrc;
	Py_ssize_t nbytes;

	// @pyparm int|addr||Address of the memory to reference 
	// @pyparm string or buffer|String||The string to copy
	if (!PyArg_ParseTuple(args, "OO:PySetMemory", &obaddr, &obsrc))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obaddr, &addr))
		return NULL;
	if (PyObject_AsReadBuffer(obsrc, &src, &nbytes)==-1)
		return NULL;
	if (IsBadWritePtr(addr, nbytes)) {
		PyErr_SetString(PyExc_ValueError,
		                "The value is not a valid address for writing");
		return NULL;
	}
	memcpy(addr, src, nbytes);
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
	Py_ssize_t maxlen;

	// @pyparm array|array||array object to use
	// @pyparm int|index||index of offset
	if (!PyArg_ParseTuple(args, "Oi:PyGetArraySignedLong",&ob,&offset))
		return NULL;
	long *l;
	if (PyObject_AsReadBuffer(ob, (const void **) &l, &maxlen)==-1)
		return NULL;

	if(offset * sizeof(*l) > maxlen) {
		PyErr_SetString(PyExc_ValueError,"array index out of bounds");
		return NULL;
		}
	return PyInt_FromLong(l[offset]);
}
%}
%native (PyGetArraySignedLong) PyGetArraySignedLong;

%{
// @pyswig object|PyGetBufferAddressAndLen|Returns a buffer object address and len
static PyObject *PyGetBufferAddressAndLen(PyObject *self, PyObject *args)
{
	PyObject *ob;
	const void *addr;
	Py_ssize_t len = 0;

	// @pyparm buffer|obj||the buffer object
	if (!PyArg_ParseTuple(args, "O:PyGetBufferAddressAndLen", &ob))
		return NULL;
	if (PyObject_AsReadBuffer(ob, &addr, &len) == -1)
		return NULL;
	return Py_BuildValue("NN", PyWinLong_FromVoidPtr(addr), PyInt_FromSsize_t(len));
}
%}
%native (PyGetBufferAddressAndLen) PyGetBufferAddressAndLen;


%typedef TCHAR *STRING_OR_ATOM_CW
%typedef TCHAR *RESOURCE_ID
%typedef TCHAR *RESOURCE_ID_NULLOK

%typemap(python,arginit) STRING_OR_ATOM_CW, RESOURCE_ID, RESOURCE_ID_NULLOK{
	$target=NULL;
}

%typemap(python,in) RESOURCE_ID {
	if (!PyWinObject_AsResourceId($source, &$target, FALSE))
		return NULL;
}

%typemap(python,in) STRING_OR_ATOM_CW, RESOURCE_ID_NULLOK {
	if (!PyWinObject_AsResourceId($source, &$target, TRUE))
		return NULL;
}

// A hack for CreateWindow - need to post-process...
%typemap(python,freearg) STRING_OR_ATOM_CW {
	// Look up the WNDCLASS object by either atom->wndclass or name->atom->wndclass to set window proc
	PyObject *obwc=NULL;
	if (_result) {
		if (IS_INTRESOURCE($source))
			obwc = PyDict_GetItem(g_AtomMap, $target);
		else{
			// Use the name to retrieve the atom, and use it to retrieve the PyWNDCLASS
			PyObject *obatom=PyDict_GetItem(g_AtomMap, $target);
			if (obatom!=NULL)
				obwc = PyDict_GetItem(g_AtomMap, obatom);
			}
		// A HUGE HACK - set the class extra bytes.
		if (obwc)
			SetClassLongPtr(_result, 0, (LONG_PTR)((PyWNDCLASS *)obwc)->m_obWndProc);
		}
	PyWinObject_FreeResourceId($source);
}

%typemap(python,freearg) RESOURCE_ID,RESOURCE_ID_NULLOK {
	PyWinObject_FreeResourceId($source);
}

#ifndef MS_WINCE
// @pyswig int|FlashWindow|The FlashWindow function flashes the specified window one time. It does not change the active state of the window.
// @pyparm <o PyHANDLE>|hwnd||Handle to a window
// @pyparm int|bInvert||Indicates if window should toggle between active and inactive
BOOL FlashWindow(HWND hwnd, BOOL bInvert);

// @pyswig int|FlashWindowEx|The FlashWindowEx function flashes the specified window a specified number of times.
%{
PyObject *PyFlashWindowEx(PyObject *self, PyObject *args)
{
	PyObject *ret, *obhwnd;
	BOOL rc;
	FLASHWINFO f;
	f.cbSize = sizeof f;
	// @pyparm <o PyHANDLE>|hwnd||Handle to a window
	// @pyparm int|dwFlags||Combination of win32con.FLASHW_* flags
	// @pyparm int|uCount||Nbr of times to flash
	// @pyparm int|dwTimeout||Elapsed time between flashes, in milliseconds
	if (!PyArg_ParseTuple(args, "Oiii", &obhwnd, &f.dwFlags, &f.uCount, &f.dwTimeout))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&f.hwnd))
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


// @pyswig int|GetWindowLong|
// @pyparm int|hwnd||
// @pyparm int|index||
long GetWindowLong(HWND hwnd, int index);

// @pyswig int|GetClassLong|
// @pyparm int|hwnd||
// @pyparm int|index||
long GetClassLong(HWND hwnd, int index);

// @pyswig int|SetWindowLong|Places a long value at the specified offset into the extra window memory of the given window.
// @comm This function calls the SetWindowLongPtr Api function
%{
static PyObject *PySetWindowLong(PyObject *self, PyObject *args)
{
	HWND hwnd;
	int index;
	PyObject *ob, *obhwnd;
	LONG_PTR oldval, newval;
	if (!PyArg_ParseTuple(args, "OiO", 
		&obhwnd,	// @pyparm <o PyHANDLE>|hwnd||The handle to the window
		&index,		// @pyparm int|index||The index of the item to set.
		&ob))		// @pyparm object|value||The value to set.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	switch (index) {
		// @comm If index is GWLP_WNDPROC, then the value parameter
		// must be a callable object (or a dictionary) to use as the
		// new window procedure.
		case GWLP_WNDPROC:
		{
			if (!PyCallable_Check(ob) && !PyDict_Check(ob)) {
				PyErr_SetString(PyExc_TypeError, "object must be callable or a dictionary");
				return NULL;
			}

			PyObject *key = PyWinLong_FromHANDLE(hwnd);
			PyObject *value = Py_BuildValue("ON", ob, PyWinLong_FromVoidPtr((void *)GetWindowLongPtr(hwnd, GWLP_WNDPROC)));
			PyDict_SetItem(g_HWNDMap, key, value);
			Py_DECREF(value);
			Py_DECREF(key);
			newval = (LONG_PTR)PyWndProcHWND;
			break;
		}
		default:
			if (!PyWinLong_AsVoidPtr(ob, (void **)&newval))
				return NULL;
	}
	oldval = SetWindowLongPtr(hwnd, index, newval);
	return PyWinLong_FromVoidPtr((void *)oldval);
}
%}
%native (SetWindowLong) PySetWindowLong;

// @pyswig int|CallWindowProc|
%{
static PyObject *PyCallWindowProc(PyObject *self, PyObject *args)
{
	MYWNDPROC wndproc;
	WPARAM wparam;
	LPARAM lparam;
	HWND hwnd;
	PyObject *obwndproc, *obhwnd, *obwparam, *oblparam;
	UINT msg;
	if (!PyArg_ParseTuple(args, "OOIOO",
		&obwndproc,	// @pyparm int|wndproc||The wndproc to call - this is generally the return value of SetWindowLong(GWL_WNDPROC)
		&obhwnd,	// @pyparm <o PyHANDLE>|hwnd||Handle to the window
		&msg,		// @pyparm int|msg||A window message
		&obwparam,	// @pyparm int/str|wparam||Type is dependent on the message
		&oblparam))	// @pyparm int/str|lparam||Type is dependent on the message
		return NULL;
	if (!PyWinLong_AsVoidPtr(obwndproc, (void **)&wndproc))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyWinObject_AsPARAM(obwparam, &wparam))
		return NULL;
	if (!PyWinObject_AsPARAM(oblparam, (WPARAM *)&lparam))
		return NULL;
	LRESULT rc;
    Py_BEGIN_ALLOW_THREADS
	rc = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
    Py_END_ALLOW_THREADS
	return PyWinLong_FromVoidPtr((void *)rc);
}
%}
%native (CallWindowProc) PyCallWindowProc;

%typemap(python,in) WPARAM {
   if (!PyWinObject_AsPARAM($source, &$target))
       return NULL;
}

%typemap(python,in) LPARAM {
   if (!PyWinObject_AsPARAM($source, (WPARAM *)&$target))
       return NULL;
}

%{
// @pyswig int|SendMessage|Sends a message to the window.
// @pyparm int|hwnd||The handle to the Window
// @pyparm int|message||The ID of the message to post
// @pyparm int/str|wparam|None|Type depends on the message
// @pyparm int/str|lparam|None|Type depends on the message
static PyObject *PySendMessage(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *obhwnd, *obwparam=Py_None, *oblparam=Py_None;
	UINT msg;
	if (!PyArg_ParseTuple(args, "Oi|OO", &obhwnd, &msg, &obwparam, &oblparam))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	WPARAM wparam;
	LPARAM lparam;
	if (!PyWinObject_AsPARAM(obwparam, &wparam))
		return NULL;
	if (!PyWinObject_AsPARAM(oblparam, (WPARAM *)&lparam))
		return NULL;

	LRESULT rc;
    Py_BEGIN_ALLOW_THREADS
	rc = SendMessage(hwnd, msg, wparam, lparam);
    Py_END_ALLOW_THREADS

	return PyWinLong_FromVoidPtr((void *)rc);
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
	HWND hwnd;
	PyObject *obhwnd, *obwparam, *oblparam;
	UINT msg;
	UINT flags, timeout;
	if (!PyArg_ParseTuple(args, "OiOOii", &obhwnd, &msg, &obwparam, &oblparam, &flags, &timeout))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	WPARAM wparam;
	LPARAM lparam;
	if (!PyWinObject_AsPARAM(obwparam, &wparam))
		return NULL;
	if (!PyWinObject_AsPARAM(oblparam, (WPARAM *)&lparam))
		return NULL;

	LRESULT rc;
	DWORD_PTR dwresult;
	Py_BEGIN_ALLOW_THREADS
	rc = SendMessageTimeout(hwnd, msg, wparam, lparam, flags, timeout, &dwresult);
	Py_END_ALLOW_THREADS
	if (rc==0)
		return PyWin_SetAPIError("SendMessageTimeout");
	// @rdesc The result is the result of the SendMessageTimeout call, plus the last 'result' param.
	// If the timeout period expires, a pywintypes.error exception will be thrown,
	// with zero as the error code.  See the Microsoft documentation for more information.
	return Py_BuildValue("NN", PyWinLong_FromVoidPtr((void *)rc), PyWinObject_FromDWORD_PTR(dwresult));
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
UINT RegisterWindowMessage(TCHAR *lpString);

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
	PyObject *args = Py_BuildValue("(NO)", PyWinLong_FromHANDLE(hwnd), cb->extra);
	if (args == NULL)
		return FALSE;
	PyObject *ret = PyEval_CallObject(cb->func, args);
	Py_DECREF(args);
	if (ret == NULL)		
		return FALSE;
	if (ret != Py_None){
		result = PyInt_AsLong(ret);
		if (result == -1 && PyErr_Occurred())
			result = FALSE;
		}
	Py_DECREF(ret);
	return result;
}

// @pyswig |EnumWindows|Enumerates all top-level windows on the screen by passing the handle to each window, in turn, to an application-defined callback function.
static PyObject *PyEnumWindows(PyObject *self, PyObject *args)
{
	BOOL rc;
	PyObject *obFunc, *obOther;
	// @pyparm function|callback||A Python function to be used as the callback.  Function can return False to stop enumeration, or raise an exception.
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
	if (!rc){
		// Callback may have raised an exception already
		if (PyErr_Occurred())
			return NULL;
		return PyWin_SetAPIError("EnumWindows");
		}
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
	PyObject *obhwnd, *obFunc, *obOther;
	HWND hwnd;
	// @pyparm <o PyHANDLE>|hwnd||The handle to the window to enumerate.
	// @pyparm object|callback||A Python function to be used as the callback.
	// @pyparm object|extra||Any python object - this is passed to the callback function as the second param (first is the hwnd).
	if (!PyArg_ParseTuple(args, "OOO", &obhwnd, &obFunc, &obOther))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyCallable_Check(obFunc)) {
		PyErr_SetString(PyExc_TypeError, "First param must be a callable object");
		return NULL;
	}
	PyEnumWindowsCallback cb;
	cb.func = obFunc;
	cb.extra = obOther;
    Py_BEGIN_ALLOW_THREADS
	// According to MSDN, the return value is not used, and according to
	// #1350, may cause spurious exceptions.
	EnumChildWindows(hwnd, PyEnumWindowsProc, (LPARAM)&cb);
    Py_END_ALLOW_THREADS
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
	HINSTANCE hinst;
	HWND hwnd;
	LPARAM param=0;
	PyObject *obResId, *obDlgProc, *obhinst, *obhwnd;
	if (!PyArg_ParseTuple(args, "OOOO|l", 
		&obhinst,	// @pyparm <o PyHANDLE>|hInstance||Handle to module that contains the dialog template
		&obResId,	// @pyparm <o PyResourceId>|TemplateName||Name or resource id of the dialog resource
		&obhwnd,	// @pyparm <o PyHANDLE>|hWndParent||Handle to dialog's parent window
		&obDlgProc,	// @pyparm function|DialogFunc||Dialog box procedure to process messages
		&param))	// @pyparm int|InitParam|0|Initialization data to be passed to above procedure during WM_INITDIALOG processing
		return NULL;
	if (!PyWinObject_AsHANDLE(obhinst, (HANDLE *)&hinst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	LPTSTR resid;
	if (!PyWinObject_AsResourceId(obResId, &resid))
		return NULL;

	PyObject *obExtra = Py_BuildValue("Ol", obDlgProc, param);
	INT_PTR rc;
    Py_BEGIN_ALLOW_THREADS
	rc = DialogBoxParam(hinst, resid, hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
    Py_END_ALLOW_THREADS
	
	PyWinObject_FreeResourceId(resid);
	Py_DECREF(obExtra);
	if (rc==-1)
		return PyWin_SetAPIError("DialogBox");
	return PyWinLong_FromVoidPtr((void *)rc);
}
%}
%native (DialogBox) PyDialogBox;
// @pyswig int|DialogBoxParam|See <om win32gui.DialogBox>
%native (DialogBoxParam) PyDialogBox;



// @pyswig int|DialogBoxIndirect|Creates a modal dialog box from a template, see <om win32ui.CreateDialogIndirect>
%{
static PyObject *PyDialogBoxIndirect(PyObject *self, PyObject *args)
{
	HINSTANCE hinst;
	HWND hwnd;
	PyObject *obParam = Py_None;
	PyObject *obhinst, *obhwnd, *obList, *obDlgProc;
	BOOL bFreeString = FALSE;

	if (!PyArg_ParseTuple(args, "OOOO|O", 
		&obhinst,		// @pyparm <o PyHANDLE>|hInstance||Handle to module creating the dialog box
		&obList,		// @pyparm <o PyDialogTemplate>|controlList||Sequence of items defining the dialog box and subcontrols
		&obhwnd,		// @pyparm <o PyHANDLE>|hWndParent||Handle to dialog's parent window
		&obDlgProc,		// @pyparm function|DialogFunc||Dialog box procedure to process messages
		&obParam))		// @pyparm long|InitParam|0|Initialization data to be passed to above procedure during WM_INITDIALOG processing
		return NULL;
	if (!PyWinObject_AsHANDLE(obhinst, (HANDLE *)&hinst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	// We unpack the object in the dlgproc - but check validity now
	if (obParam != Py_None && !PyInt_Check(obParam) && !PyLong_Check(obParam)) {
		return PyErr_Format(PyExc_TypeError, "optional param must be None, or an integer (got %s)",
		                    obParam->ob_type->tp_name);
	}

	HGLOBAL h = MakeResourceFromDlgList(obList);
	if (h == NULL)
		return NULL;

	HGLOBAL templ = (HGLOBAL) GlobalLock(h);
	if (!templ) {
		GlobalFree(h);
		return PyWin_SetAPIError("GlobalLock (for template)");
	}

	PyObject *obExtra = Py_BuildValue("OO", obDlgProc, obParam);

	INT_PTR rc;
    Py_BEGIN_ALLOW_THREADS
	rc = DialogBoxIndirectParam(hinst, (const DLGTEMPLATE *) templ, hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
	GlobalUnlock(h);
	GlobalFree(h);
    Py_END_ALLOW_THREADS
	Py_DECREF(obExtra);
	if (rc==-1)
		return PyWin_SetAPIError("DialogBoxIndirect");

	return PyWinLong_FromVoidPtr((void *)rc);
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
	HINSTANCE hinst;
	HWND hwnd;
	LPARAM param=0;
	PyObject *obhinst, *obhwnd, *obList, *obDlgProc;
	BOOL bFreeString = FALSE;
	if (!PyArg_ParseTuple(args, "OOOO|l",
		&obhinst,		// @pyparm <o PyHANDLE>|hInstance||Handle to module creating the dialog box
		&obList,		// @pyparm <o PyDialogTemplate>|controlList||Sequence containing a <o PyDLGTEMPLATE>, followed by variable number of <o PyDLGITEMTEMPLATE>s
		&obhwnd,		// @pyparm <o PyHANDLE>|hWndParent||Handle to dialog's parent window
		&obDlgProc,		// @pyparm function|DialogFunc||Dialog box procedure to process messages
		&param))		// @pyparm int|InitParam|0|Initialization data to be passed to above procedure during WM_INITDIALOG processing
		return NULL;
	if (!PyWinObject_AsHANDLE(obhinst, (HANDLE *)&hinst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;

	HGLOBAL h = MakeResourceFromDlgList(obList);
	if (h == NULL)
		return NULL;

	PyObject *obExtra = Py_BuildValue("Ol", obDlgProc, param);

	HWND rc;
    Py_BEGIN_ALLOW_THREADS
	HGLOBAL templ = (HGLOBAL) GlobalLock(h);
	rc = CreateDialogIndirectParam(hinst, (const DLGTEMPLATE *) templ, hwnd, PyDlgProcHDLG, (LPARAM)obExtra);
	GlobalUnlock(h);
	GlobalFree(h);
    Py_END_ALLOW_THREADS
	Py_DECREF(obExtra);
	if (NULL == rc)
		return PyWin_SetAPIError("CreateDialogIndirect");

	return PyWinLong_FromHANDLE(rc);

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
	if (!PyWinObject_AsHANDLE(obhDlg, (HANDLE *)&hDlg))
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
int GetDlgCtrlID( HWND hwnd);

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
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
		if (chars_returned==0 && GetLastError()!=0){
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
	PyObject *obhwnd;
	TCHAR buffer[512];
	// @pyparm <o PyHANDLE>|hwnd||The handle to the window
	if (!PyArg_ParseTuple(args, "O", &obhwnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
	if (!PyArg_ParseTuple(args,":GetCursorInfo"))
		return NULL;
	if (!::GetCursorInfo(&ci))
		return PyWin_SetAPIError("GetCursorInfo");
	return Py_BuildValue("iN(ii)", ci.flags, PyWinLong_FromHANDLE(ci.hCursor), ci.ptScreenPos.x, ci.ptScreenPos.y);
}
%}
%native(GetCursorInfo) PyGetCursorInfo;
#endif

// @pyswig HACCEL|CreateAcceleratorTable|Creates an accelerator table
%{
PyObject *PyCreateAcceleratorTable(PyObject *self, PyObject *args)
{
    DWORD num, i;
    ACCEL *accels = NULL;
    PyObject *ret = NULL;
    PyObject *obAccels, *Accels_tuple;
    HACCEL ha;
    // @pyparm ( (int, int, int), ...)|accels||A sequence of (fVirt, key, cmd),
    // as per the Win32 ACCEL structure.
    if (!PyArg_ParseTuple(args, "O:CreateAcceleratorTable", &obAccels))
        return NULL;
    if ((Accels_tuple=PyWinSequence_Tuple(obAccels, &num)) == NULL)
		return NULL;

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
        PyObject *ob = PyTuple_GET_ITEM(Accels_tuple, i);
        if (!PyArg_ParseTuple(ob, "BHH:ACCEL", &p->fVirt, &p->key, &p->cmd))
            goto done;
    }
    ha = ::CreateAcceleratorTable(accels, num);
    if (ha)
        ret = PyWinLong_FromHANDLE(ha);
    else
        PyWin_SetAPIError("CreateAcceleratorTable");
done:
	Py_DECREF(Accels_tuple);
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
  HBRUSH INPUT_NULLOK,		// @pyparm <o PyGdiHANDLE>|hbrFlickerFreeDraw||handle to background brush, can be None
  int diFlags				// @pyparm int|diFlags||icon-drawing flags (win32con.DI_*)
);

// @pyswig int|CreateIconIndirect|Creates an icon or cursor from an ICONINFO structure. 
HICON CreateIconIndirect(ICONINFO *INPUT);	// @pyparm <o PyICONINFO>|iconinfo||Tuple defining the icon parameters

%{
// @pyswig <o PyHANDLE>|CreateIconFromResource|Creates an icon or cursor from resource bits describing the icon.
static PyObject *PyCreateIconFromResource(PyObject *self, PyObject *args)
{
	// @pyparm string|bits||The bits
	// @pyparm bool|fIcon||True if an icon, False if a cursor.
	// @pyparm int|ver|0x00030000|Specifies the version number of the icon or cursor
	// format for the resource bits pointed to by the presbits parameter.
	// This parameter can be 0x00030000.
	PBYTE bits;
	DWORD nBits;
	int isIcon;
	DWORD ver = 0x00030000;
	PyObject *obbits;
	if (!PyArg_ParseTuple(args, "Oi|i", &obbits, &isIcon, &ver))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obbits, (void **)&bits, &nBits, FALSE))
		return NULL;
	HICON ret = CreateIconFromResource(bits, nBits, isIcon, ver);
	if (!ret)
	    return PyWin_SetAPIError("CreateIconFromResource");
	return PyWinLong_FromHANDLE(ret);
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

%{
// @pyswig |DeleteObject|Deletes a logical pen, brush, font, bitmap, region, or palette, freeing all system resources associated with the object. After the object is deleted, the specified handle is no longer valid.
static PyObject *PyDeleteObject(PyObject *self, PyObject *args)
{
	PyObject *obhgdiobj;
	if (!PyArg_ParseTuple(args, "O:DeleteObject",
		&obhgdiobj))	// @pyparm <o PyGdiHANDLE>|handle||handle to the object to delete.
		return NULL;
	if (PyHANDLE_Check(obhgdiobj)){
		// Make sure we don't call Close() for any other type of PyHANDLE
		if (strcmp(((PyHANDLE *)obhgdiobj)->GetTypeName(),"PyGdiHANDLE")!=0){
			PyErr_SetString(PyExc_TypeError,"DeleteObject requires a PyGdiHANDLE");
			return NULL;
			}
		if (!((PyHANDLE *)obhgdiobj)->Close())
			return NULL;
		Py_INCREF(Py_None);
		return Py_None;
		}
	HGDIOBJ hgdiobj;
	if (!PyWinObject_AsHANDLE(obhgdiobj, &hgdiobj))
		return NULL;
	if (!DeleteObject(hgdiobj))
		return PyWin_SetAPIError("DeleteObject");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (DeleteObject) PyDeleteObject;

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

// @pyswig |PatBlt|Paints a rectangle by combining the current brush with existing colors
BOOLAPI PatBlt(
	HDC hdc,	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int XLeft,	// @pyparm int|XLeft||Horizontal pos
	int YLeft,	// @pyparm int|YLeft||Vertical pos
	int Width,	// @pyparm int|Width||Width of rectangular area
	int Height,	// @pyparm int|Height||Height of rectangular area
	DWORD Rop);	// @pyparm int|Rop||Raster operation, one of PATCOPY,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS

#ifndef MS_WINCE
// @pyswig int|SetStretchBltMode|Sets the stretching mode used by <om win32gui.StretchBlt>
// @rdesc If the function succeeds, the return value is the previous stretching mode.
// <nl>If the function fails, the return value is zero. 
int SetStretchBltMode(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int StretchMode);	// @pyparm int|StretchMode||One of BLACKONWHITE,COLORONCOLOR,HALFTONE,STRETCH_ANDSCANS,STRETCH_DELETESCANS,STRETCH_HALFTONE,STRETCH_ORSCANS, or WHITEONBLACK (from win32con)

// @pyswig int|GetStretchBltMode|Returns the stretching mode used by <om win32gui.StretchBlt>
// @rdesc Returns one of BLACKONWHITE,COLORONCOLOR,HALFTONE,STRETCH_ANDSCANS,STRETCH_DELETESCANS,STRETCH_HALFTONE,STRETCH_ORSCANS,WHITEONBLACK, or 0 on error.
int GetStretchBltMode(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
#endif	/* not MS_WINCE */

%{
// @pyswig |TransparentBlt|Transfers color from one DC to another, with one color treated as transparent
static PyObject *PyTransparentBlt(PyObject *self, PyObject *args)
{
	CHECK_PFN(TransparentBlt);
	PyObject *obsrc, *obdst;
	HDC src, dst;
	int src_x, src_y, src_width, src_height;
	int dst_x, dst_y, dst_width, dst_height;
	UINT transparent;
	BOOL ret;
	if (!PyArg_ParseTuple(args,"OiiiiOiiiiI:TransparentBlt",
		&obdst,			// @pyparm <o PyHANDLE>|Dest||Destination device context handle
		&dst_x,			// @pyparm int|XOriginDest||X pos of dest rect
		&dst_y,			// @pyparm int|YOriginDest||Y pos of dest rect
		&dst_width,		// @pyparm int|WidthDest||Width of dest rect
		&dst_height,	// @pyparm int|HeightDest||Height of dest rect
		&obsrc,			// @pyparm <o PyHANDLE>|Src||Source DC handle
		&src_x,			// @pyparm int|XOriginSrc||X pos of src rect
		&src_y,			// @pyparm int|YOriginSrc||Y pos of src rect
		&src_width,		// @pyparm int|WidthSrc||Width of src rect
		&src_height,	// @pyparm int|HeightSrc||Height of src rect
		&transparent))	// @pyparm int|Transparent||RGB color value that will be transparent
		return NULL;
	if (!PyWinObject_AsHANDLE(obdst, (HANDLE *)&dst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obsrc, (HANDLE *)&src))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnTransparentBlt)(
		dst, dst_x, dst_y, dst_width, dst_height,
		src, src_x, src_y, src_width, src_height,
		transparent);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("TransparentBlt");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig |MaskBlt|Combines the color data for the source and destination
// bitmaps using the specified mask and raster operation.
// @comm This function is not supported on Win9x.
// @pyseeapi MaskBlt
static PyObject *PyMaskBlt(PyObject *self, PyObject *args)
{
	CHECK_PFN(MaskBlt);
	PyObject *obsrc, *obdst, *obmask;
	HDC src, dst;
	HBITMAP mask;
	int dst_x, dst_y, dst_width, dst_height;
	int src_x, src_y;
	int mask_x, mask_y;
	DWORD rop;
	if (!PyArg_ParseTuple(args,"OiiiiOiiOiik:MaskBlt",
		&obdst,			// @pyparm <o PyHANDLE>|Dest||Destination device context handle
		&dst_x,			// @pyparm int|XDest||X pos of dest rect
		&dst_y,			// @pyparm int|YDest||Y pos of dest rect
		&dst_width,		// @pyparm int|Width||Width of rect to be copied
		&dst_height,	// @pyparm int|Height||Height of rect to be copied
		&obsrc,			// @pyparm <o PyHANDLE>|Src||Source DC handle
		&src_x,			// @pyparm int|XSrc||X pos of src rect
		&src_y,			// @pyparm int|YSrc||Y pos of src rect
		&obmask,		// @pyparm <o PyGdiHANDLE>|Mask||Handle to monochrome bitmap used to mask color
		&mask_x,		// @pyparm int|xMask||X pos in mask
		&mask_y,		// @pyparm int|yMask||Y pos in mask
		&rop))			// @pyparm int|Rop||Foreground and background raster operations.  See MSDN docs for how to construct this value.
		return NULL;
	if (!PyWinObject_AsHANDLE(obdst, (HANDLE *)&dst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obsrc, (HANDLE *)&src))
		return NULL;
	if (!PyWinObject_AsHANDLE(obmask, (HANDLE *)&mask))
		return NULL;
	if (!(*pfnMaskBlt)(
		dst, dst_x, dst_y, dst_width, dst_height,
		src, src_x, src_y,
		mask, mask_x, mask_y, rop))
		return PyWin_SetAPIError("MaskBlt");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig |AlphaBlend|Transfers color information using alpha blending
static PyObject *PyAlphaBlend(PyObject *self, PyObject *args)
{
	CHECK_PFN(AlphaBlend);
	PyObject *obsrc, *obdst, *obbl;
	HDC src, dst;
	int src_x, src_y, src_width, src_height;
	int dst_x, dst_y, dst_width, dst_height;
	BLENDFUNCTION bl;
	if (!PyArg_ParseTuple(args,"OiiiiOiiiiO:AlphaBlend",
		&obdst,			// @pyparm <o PyHANDLE>|Dest||Destination device context handle
		&dst_x,			// @pyparm int|XOriginDest||X pos of dest rect
		&dst_y,			// @pyparm int|YOriginDest||Y pos of dest rect
		&dst_width,		// @pyparm int|WidthDest||Width of dest rect
		&dst_height,	// @pyparm int|HeightDest||Height of dest rect
		&obsrc,			// @pyparm <o PyHANDLE>|Src||Source DC handle
		&src_x,			// @pyparm int|XOriginSrc||X pos of src rect
		&src_y,			// @pyparm int|YOriginSrc||Y pos of src rect
		&src_width,		// @pyparm int|WidthSrc||Width of src rect
		&src_height,	// @pyparm int|HeightSrc||Height of src rect
		&obbl))			// @pyparm <o PyBLENDFUNCTION>|blendFunction||Alpha blending parameters
		return NULL;
	if (!PyWinObject_AsHANDLE(obdst, (HANDLE *)&dst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obsrc, (HANDLE *)&src))
		return NULL;
	if (!PyWinObject_AsBLENDFUNCTION(obbl, &bl))
		return NULL;
	if (!(*pfnAlphaBlend)(
		dst, dst_x, dst_y, dst_width, dst_height,
		src, src_x, src_y, src_width, src_height,
		bl))
		return PyWin_SetAPIError("AlphaBlend");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (TransparentBlt) PyTransparentBlt;
%native (MaskBlt) PyMaskBlt;
%native (AlphaBlend) PyAlphaBlend;

// @pyswig int|ImageList_Add|Adds an image or images to an image list. 
// @rdesc Returns the index of the first new image if successful, or -1 otherwise. 
int ImageList_Add(HIMAGELIST himl, // @pyparm int|himl||Handle to the image list. 
                  HBITMAP hbmImage, // @pyparm <o PyGdiHANDLE>|hbmImage||Handle to the bitmap that contains the image or images. The number of images is inferred from the width of the bitmap. 
				  HBITMAP hbmMask); // @pyparm <o PyGdiHANDLE>|hbmMask||Handle to the bitmap that contains the mask. If no mask is used with the image list, this parameter is ignored


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

// @pyswig <o PyHANDLE>|FindWindow|Retrieves a handle to the top-level window whose class name and window name match the specified strings.
HWND FindWindow( 
	RESOURCE_ID_NULLOK className, // @pyparm <o PyResourceId>|ClassName||Name or atom of window class to find, can be None
	TCHAR *INPUT_NULLOK); // @pyparm string|WindowName||Title of window to find, can be None

#ifndef MS_WINCE
// @pyswig <o PyHANDLE>|FindWindowEx|Retrieves a handle to the top-level window whose class name and window name match the specified strings.
HWND FindWindowEx(
	HWND parent, // @pyparm <o PyHANDLE>|Parent||Window whose child windows will be searched.  If 0, desktop window is assumed.
	HWND childAfter, // @pyparm <o PyHANDLE>|ChildAfter||Child window after which to search in Z-order, can be 0 to search all
	RESOURCE_ID_NULLOK className, // @pyparm <o PyResourceId>|ClassName||Name or atom of window class to find, can be None
	TCHAR *INPUT_NULLOK); // @pyparm string|WindowName||Title of window to find, can be None

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

// @pyswig |HideCaret|Hides the caret
BOOLAPI HideCaret(HWND hWnd);	// @pyparm <o PyHANDLE>|hWnd||Window that owns the caret, can be 0.

// @pyswig |SetCaretPos|Changes the position of the caret
BOOLAPI SetCaretPos(
	int X,  // @pyparm int|x||horizontal position  
	int Y   // @pyparm int|y||vertical position
);

// @pyswig int,int|GetCaretPos|Returns the current caret position
BOOLAPI GetCaretPos(POINT *OUTPUT);

// @pyswig |ShowCaret|Shows the caret at its current position
BOOLAPI ShowCaret(HWND hWnd);	// @pyparm <o PyHANDLE>|hWnd||Window that owns the caret, can be 0.

// @pyswig boolean|ShowWindow|Shows or hides a window and changes its state
BOOL ShowWindow(
	HWND hWnd,		// @pyparm int|hWnd||The handle to the window
	int nCmdShow);	// @pyparm int|cmdShow||Combination of win32con.SW_* flags

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

// @pyswig (left, top, right, bottom)|GetClientRect|Returns the rectangle of the client area of a window, in client coordinates
// @pyparm int|hwnd||The handle to the window
BOOLAPI GetClientRect(HWND hWnd, RECT *OUTPUT);

// @pyswig HDC|GetDC|Gets the device context for the window.
// @pyparm int|hwnd||The handle to the window
HDC GetDC(  HWND hWnd );

// @pyswig int|SaveDC|Save the state of a device context
// @rdesc Returns a value identifying the state that can be passed to <om win32gui.RestoreDC>.  On error, returns 0.
int SaveDC(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to device context

// @pyswig |RestoreDC|Restores a device context state
BOOLAPI RestoreDC(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int SavedDC);	// @pyparm int|SavedDC||Identifier of state to be restored, as returned by <om win32gui.SaveDC>.

// @pyswig |DeleteDC|Deletes a DC
BOOLAPI DeleteDC(
    HDC dc // @pyparm int|hdc||The source DC
);

// @pyswig HDC|CreateCompatibleDC|Creates a memory device context (DC) compatible with the specified device. 
HDC CreateCompatibleDC(
  HDC hdc   // @pyparm int|dc||handle to DC
);

// @pyswig <o PyGdiHANDLE>|CreateCompatibleBitmap|Creates a bitmap compatible with the device that is associated with the specified device context. 
HBITMAP CreateCompatibleBitmap(
  HDC hdc,        // @pyparm int|hdc||handle to DC
  int nWidth,     // @pyparm int|width||width of bitmap, in pixels
  int nHeight     // @pyparm int|height||height of bitmap, in pixels
);

// @pyswig <o PyGdiHANDLE>|CreateBitmap|Creates a bitmap
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

// @pyswig <o PyHANDLE>|GetCurrentObject|Retrieves currently selected object from a DC
HGDIOBJ GetCurrentObject(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	UINT ObjectType);	// @pyparm int|ObjectType||Type of object to retrieve, one of win32con.OBJ_*;

HINSTANCE GetModuleHandle(TCHAR *INPUT_NULLOK);

// @pyswig (left, top, right, bottom)|GetWindowRect|Returns the rectangle for a window in screen coordinates
// @pyparm int|hwnd||The handle to the window
BOOLAPI GetWindowRect(HWND hWnd, RECT *OUTPUT);

// @pyswig <o PyHANDLE>|GetStockObject|Creates a handle to one of the standard system Gdi objects
HGDIOBJ GetStockObject(int object);	// @pyparm int|Object||One of *_BRUSH, *_PEN, *_FONT, or *_PALLETTE constants

// @pyswig |PostQuitMessage|
// @pyparm int|rc||
void PostQuitMessage(int rc);

#ifndef MS_WINCE
// @pyswig |WaitMessage|Waits for a message
BOOLAPI WaitMessage();
#endif	/* MS_WINCE */

// @pyswig |SetWindowPos|Sets the position and size of a window
BOOLAPI SetWindowPos(
	HWND hWnd,			// @pyparm <o PyHANDLE>|hWnd||Handle to the window
	HWND InsertAfter,	// @pyparm <o PyHANDLE>|InsertAfter||Window that hWnd will be placed below.  Can be a window handle or one of HWND_BOTTOM,HWND_NOTOPMOST,HWND_TOP, or HWND_TOPMOST
	int X,				// @pyparm int|X||New X coord
	int Y,				// @pyparm int|Y||New Y coord
	int cx,				// @pyparm int|cx||New width of window
	int cy,				// @pyparm int|cy||New height of window
	UINT Flags);		// @pyparm int|Flags||Combination of win32con.SWP_* flags

%{
// @pyswig tuple|GetWindowPlacement|Returns placement information about the current window.
static PyObject *
PyGetWindowPlacement(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *obhwnd;
	if (!PyArg_ParseTuple(args, "O:GetWindowPlacement", &obhwnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;

	WINDOWPLACEMENT pment;
	pment.length=sizeof(pment);
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = GetWindowPlacement(hwnd, &pment );
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
	PyObject *obhwnd;
	HWND hwnd;
	WINDOWPLACEMENT pment;
	pment.length=sizeof(pment);
	// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	// @pyparm (tuple)|placement||A tuple representing the WINDOWPLACEMENT structure.
	if (!PyArg_ParseTuple(args,"O(ii(ii)(ii)(iiii)):SetWindowPlacement",
	                      &obhwnd,
	                      &pment.flags, &pment.showCmd,
	                      &pment.ptMinPosition.x,&pment.ptMinPosition.y,
	                      &pment.ptMaxPosition.x,&pment.ptMaxPosition.y,
	                      &pment.rcNormalPosition.left, &pment.rcNormalPosition.top,
	                      &pment.rcNormalPosition.right, &pment.rcNormalPosition.bottom))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	BOOL rc;
	Py_BEGIN_ALLOW_THREADS
	rc = SetWindowPlacement(hwnd, &pment );
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

	// Save atom/PyWNDCLASS and name/atom pairs in global dict.  These are used in
	// CreateWindow to lookup the python window proc function for the class
	PyObject *ret = PyInt_FromLong(at);
	if (ret==NULL)
		return NULL;
	if (PyDict_SetItem(g_AtomMap, ((PyWNDCLASS *)obwc)->m_obClassName, ret)==-1){
		Py_DECREF(ret);
		return NULL;
		}
	if (PyDict_SetItem(g_AtomMap, ret, obwc)==-1){
		PyDict_DelItem(g_AtomMap, ((PyWNDCLASS *)obwc)->m_obClassName);
		Py_DECREF(ret);
		return NULL;
		}
	return ret;
}
%}
%native (RegisterClass) PyRegisterClass;

%{
// @pyswig |UnregisterClass|Unregisters a window class created by <om win32gui.RegisterClass>
static PyObject *PyUnregisterClass(PyObject *self, PyObject *args)
{
	LPTSTR atom;
	HINSTANCE hinst;
	PyObject *obhinst, *obatom, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO", 
		&obatom,		// @pyparm <o PyResourceId>|atom||The atom or classname identifying the class previously registered.
		&obhinst))		// @pyparm <o PyHANDLE>|hinst||The handle to the instance unregistering the class, can be None
		return NULL;
	if (!PyWinObject_AsHANDLE(obhinst, (HANDLE *)&hinst))
		return NULL;
	if (!PyWinObject_AsResourceId(obatom, &atom))
		return NULL;
	BOOL bsuccess=UnregisterClass(atom, hinst);
	if (!bsuccess){
		PyWinObject_FreeResourceId(atom);
		return PyWin_SetAPIError("UnregisterClass");
		}

	// Delete the atom/PyWNDCLASS and name/atom from the global dictionary.
	PyObject *val=PyDict_GetItem(g_AtomMap, obatom);
	if (val!=NULL){
		if (IS_INTRESOURCE(atom))	// val is the PyWNDCLASS, use it's name to delete the name/atom pair
			PyDict_DelItem(g_AtomMap, ((PyWNDCLASS *)val)->m_obClassName);
		else	// val is numeric atom, use it to delete the atom/PyWNDCLASS pair
			PyDict_DelItem(g_AtomMap, val);
		PyDict_DelItem(g_AtomMap, obatom);
		}

	// Don't throw an exception if dict items can't be deleted since UnregisterClass has already succeeded
	PyWinObject_FreeResourceId(atom);
	if (PyErr_Occurred())
		PyErr_Print();
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

	return PyWinLong_FromVoidPtr((void *)msg.wParam);

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
	WPARAM result = 0;
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
	return PyWinLong_FromVoidPtr((void *)result);
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
	PyObject *obhwnd, *obhicon=Py_None;
	memset(pnid, 0, sizeof(*pnid));
	pnid->cbSize = sizeof(*pnid);
	if (!PyArg_ParseTuple(ob, "O|iiiOO:NOTIFYICONDATA tuple", &obhwnd, &pnid->uID, &pnid->uFlags, &pnid->uCallbackMessage, &obhicon, &obTip))
		return FALSE;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&pnid->hWnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhicon, (HANDLE *)&pnid->hIcon))
		return NULL;
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
	PyObject *obhwnd, *obhicon=Py_None;
	memset(pnid, 0, sizeof(*pnid));
	pnid->cbSize = sizeof(*pnid);
	// @object PyNOTIFYICONDATA|Tuple used to fill a NOTIFYICONDATA struct as used with <om win32gui.Shell_NotifyIcon>
	// @pyseeapi NOTIFYICONDATA
	if (!PyArg_ParseTuple(ob, "O|iiiOOOiOi:NOTIFYICONDATA tuple", 
		&obhwnd,		// @tupleitem 0|<o PyHANDLE>|hWnd|Handle to window that will process icon's messages
		&pnid->uID,		// @tupleitem 1|int|ID|Unique id used when hWnd processes messages from more than one icon
		&pnid->uFlags,	// @tupleitem 2|int|Flags|Combination of win32gui.NIF_* flags
		&pnid->uCallbackMessage,	// @tupleitem 3|int|CallbackMessage|Message id to be pass to hWnd when processing messages
		&obhicon,		// @tupleitem 4|<o PyHANDLE>|hIcon|Handle to the icon to be displayed
		&obTip,			// @tupleitem 5|str|Tip|Tooltip text (optional)
		&obInfo,		// @tupleitem 6|str|Info|Balloon tooltip text (optional)
		&pnid->uTimeout,	// @tupleitem 7|int|Timeout|Timeout for balloon tooltip, in milliseconds (optional)
		&obInfoTitle,	// @tupleitem 8|str|InfoTitle|Title for balloon tooltip (optional)
		&pnid->dwInfoFlags))	// @tupleitem 9|int|InfoFlags|Combination of win32gui.NIIF_* flags (optional)
		return FALSE;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&pnid->hWnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhicon, (HANDLE *)&pnid->hIcon))
		return NULL;
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
#define NIF_STATE NIF_STATE
// #define NIF_GUID NIF_GUID
#define NIIF_WARNING NIIF_WARNING
#define NIIF_ERROR NIIF_ERROR
#define NIIF_NONE NIIF_NONE
#define NIIF_INFO NIIF_INFO
// #define NIIF_USER NIIF_USER
#define NIIF_ICON_MASK NIIF_ICON_MASK
#define NIIF_NOSOUND NIIF_NOSOUND
#endif

#define NIM_ADD NIM_ADD // Adds an icon to the status area. 
#define NIM_DELETE  NIM_DELETE // Deletes an icon from the status area. 
#define NIM_MODIFY  NIM_MODIFY // Modifies an icon in the status area.
#define NIM_SETVERSION NIM_SETVERSION
#ifdef NIM_SETFOCUS
#define NIM_SETFOCUS NIM_SETFOCUS // Give the icon focus.  
#endif

%typemap(python,in) NOTIFYICONDATA *{
	if (!PyObject_AsNOTIFYICONDATA($source, $target))
		return NULL;
}
%typemap(python,arginit) NOTIFYICONDATA *(NOTIFYICONDATA nid){
	ZeroMemory(&nid, sizeof(nid));
	$target = &nid;
}

// @pyswig |Shell_NotifyIcon|Adds, removes or modifies a taskbar icon.
BOOLAPI Shell_NotifyIcon(
	DWORD dwMessage,		// @pyparm int|Message||One of win32gui.NIM_* flags
	NOTIFYICONDATA *pnid);	// @pyparm <o PyNOTIFYICONDATA>|nid||Tuple containing NOTIFYICONDATA info

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
	HWND hwnd;
	PyObject *obhwnd;
	int line, size=0;
	if (!PyArg_ParseTuple(args, "Oi|i", &obhwnd, &line, &size))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	int numChars;
	TCHAR *buf;
	Py_BEGIN_ALLOW_THREADS
	if (size==0)
		size = Edit_LineLength(hwnd, line)+1;
	buf = (TCHAR *)malloc(size * sizeof(TCHAR));
	numChars = Edit_GetLine(hwnd, line, buf, size);
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
// @pyparm <o PyRECT>|prcRect||Pointer to rec (can be None)

BOOL TrackPopupMenu(HMENU hmenu, UINT flags, int x, int y, int reserved, HWND hwnd, const RECT *INPUT_NULLOK);

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
	char *address;
	PyObject *obaddress;
	if (!PyArg_ParseTuple(args, "O", &obaddress))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obaddress, (void **)&address))
		return NULL;
	return PyString_FromString(address);
}
%}
%native (lpstr) Pylpstr;

// @pyswig int|CommDlgExtendedError|
DWORD CommDlgExtendedError(void);

%typemap (python, in) OPENFILENAME *INPUT (int size){
	size = sizeof(OPENFILENAME);
	if (!PyString_Check($source)) {
		PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string (got type %s)",
		             size, $source->ob_type->tp_name);
		return NULL;
	}
	if (size != PyString_GET_SIZE($source)) {
		PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string (got string of %d bytes)",
		             size, PyString_GET_SIZE($source));
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
        PyList_SET_ITEM(objects_large, i, PyWinLong_FromHANDLE(rgLarge[i]));
        PyList_SET_ITEM(objects_small, i, PyWinLong_FromHANDLE(rgSmall[i]));
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
// @pyswig <o PyICONINFO>|GetIconInfo|Returns parameters for an icon or cursor
// @pyparm <o PyHANDLE>|hicon||The icon to query
// @rdesc The result is a tuple of (fIcon, xHotspot, yHotspot, hbmMask, hbmColor)
// The hbmMask and hbmColor items are bitmaps created for the caller, so must be freed.
BOOLAPI GetIconInfo( HICON hicon, ICONINFO *OUTPUT);
#endif	/* not MS_WINCE */

// @pyswig (int,int)|ScreenToClient|Convert screen coordinates to client coords
BOOLAPI ScreenToClient(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	POINT *BOTH);	// @pyparm (int,int)|Point||Screen coordinates to be converted

// @pyswig (int,int)|ClientToScreen|Convert client coordinates to screen coords
BOOLAPI ClientToScreen(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	POINT *BOTH);	// @pyparm (int,int)|Point||Client coordinates to be converted

// @pyswig |PaintDesktop|Fills a DC with the destop background
BOOLAPI PaintDesktop(
	HDC hdc);		// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig |RedrawWindow|Causes a portion of a window to be redrawn
BOOLAPI RedrawWindow(
	HWND hWnd,			// @pyparm <o PyHANDLE>|hWnd||Handle to window to be redrawn
	RECT *INPUT_NULLOK,	// @pyparm (int,int,int,int)|rcUpdate||Rectangle (left, top, right, bottom) identifying part of window to be redrawn, can be None
	HRGN INPUT_NULLOK,	// @pyparm <o PyGdiHANDLE>|hrgnUpdate||Handle to region to be redrawn, can be None to indicate entire client area
	UINT flags);		// @pyparm int|flags||Combination of win32con.RDW_* flags

%{
// @pyswig cx, cy|GetTextExtentPoint32|Computes the width and height of the specified string of text.
static PyObject *PyGetTextExtentPoint32(PyObject *self, PyObject *args)
{
	// @pyparm <o PyHANDLE>|hdc||The device context
	// @pyparm string|str||The string to measure.
	HDC hdc;
	PyObject *obString, *obdc;
	if (!PyArg_ParseTuple(args, "OO:GetTextExtentPoint32", &obdc, &obString))
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	TCHAR *szString = NULL;
	DWORD nchars;
	if (!PyWinObject_AsTCHAR(obString, &szString, FALSE, &nchars))
		return FALSE;
	SIZE size = {0,0};
	BOOL rc;
	Py_BEGIN_ALLOW_THREADS
	rc = GetTextExtentPoint32(hdc, szString, nchars, &size);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(szString);
	if (!rc)
		return PyWin_SetAPIError("GetTextExtentPoint32");
	return Py_BuildValue("ll", size.cx, size.cy);
}

// @pyswig dict|GetTextMetrics|Returns info for the font selected into a DC
static PyObject *PyGetTextMetrics(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	TEXTMETRICW tm;
	if (!PyArg_ParseTuple(args, "O:GetTextMetrics",
		&obdc))
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!GetTextMetricsW(hdc, &tm))
		return PyWin_SetAPIError("GetTextMetrics");
	return Py_BuildValue("{s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:N,s:N,s:N,s:N,s:B,s:B,s:B,s:B,s:B}",
		"Height", tm.tmHeight,
		"Ascent", tm.tmAscent,
		"Descent", tm.tmDescent,
		"InternalLeading", tm.tmInternalLeading,
		"ExternalLeading", tm.tmExternalLeading,
		"AveCharWidth", tm.tmAveCharWidth,
		"MaxCharWidth", tm.tmMaxCharWidth,
		"Weight", tm.tmWeight,
		"Overhang", tm.tmOverhang,
		"DigitizedAspectX", tm.tmDigitizedAspectX,
		"DigitizedAspectY", tm.tmDigitizedAspectY,
		"FirstChar", PyWinObject_FromWCHAR(&tm.tmFirstChar, 1),
		"LastChar", PyWinObject_FromWCHAR(&tm.tmLastChar, 1),
		"DefaultChar", PyWinObject_FromWCHAR(&tm.tmDefaultChar, 1),
		"BreakChar", PyWinObject_FromWCHAR(&tm.tmBreakChar, 1),
		"Italic", tm.tmItalic,
		"Underlined", tm.tmUnderlined,
		"StruckOut", tm.tmStruckOut,
		"PitchAndFamily", tm.tmPitchAndFamily,
		"CharSet", tm.tmCharSet); 
}

// @pyswig int|GetTextCharacterExtra|Returns the space between characters
static PyObject *PyGetTextCharacterExtra(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int ret;
	if (!PyArg_ParseTuple(args, "O:GetTextCharacterExtra",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetTextCharacterExtra(hdc);
	if (ret==0x80000000)
		return PyWin_SetAPIError("GetTextCharacterExtra");
	return PyInt_FromLong(ret);
}

// @pyswig int|SetTextCharacterExtra|Sets the spacing between characters
// @rdesc Returns the previous spacing
static PyObject *PySetTextCharacterExtra(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int newspacing, prevspacing;
	if (!PyArg_ParseTuple(args, "Oi:SetTextCharacterExtra",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newspacing))	// @pyparm int|CharExtra||Space between adjacent chars, in logical units
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevspacing=SetTextCharacterExtra(hdc, newspacing);
	if (prevspacing==0x80000000)
		return PyWin_SetAPIError("SetTextCharacterExtra");
	return PyInt_FromLong(prevspacing);
}

// @pyswig int|GetTextAlign|Returns horizontal and vertical alignment for text in a device context
// @rdesc Returns combination of win32con.TA_* flags
static PyObject *PyGetTextAlign(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int prevalign;
	if (!PyArg_ParseTuple(args, "O:GetTextAlign",
		&obdc))			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevalign=GetTextAlign(hdc);
	if (prevalign==GDI_ERROR)
		return PyWin_SetAPIError("GetTextAlign");
	return PyInt_FromLong(prevalign);
}

// @pyswig int|SetTextAlign|Sets horizontal and vertical alignment for text in a device context
// @rdesc Returns the previous alignment flags
static PyObject *PySetTextAlign(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int newalign, prevalign;
	if (!PyArg_ParseTuple(args, "Oi:SetTextAlign",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newalign))	// @pyparm int|Mode||Combination of win32con.TA_* constants
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevalign=SetTextAlign(hdc, newalign);
	if (prevalign==GDI_ERROR)
		return PyWin_SetAPIError("SetTextAlign");
	return PyInt_FromLong(prevalign);
}

// @pyswig <o PyUnicode>|GetTextFace|Retrieves the name of the font currently selected in a DC
// @comm Calls unicode api function (GetTextFaceW)
static PyObject *PyGetTextFace(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	WCHAR face[256];
	int returned_size;
	if (!PyArg_ParseTuple(args, "O:GetTextFace",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	returned_size=GetTextFaceW(hdc, 256, face);
	if (returned_size==0)
		return PyWin_SetAPIError("GetTextFace");
	// Char count includes trailing null
	return PyWinObject_FromWCHAR(face, returned_size-1);
}

// @pyswig int|GetMapMode|Returns the method a device context uses to translate logical units to physical units
// @rdesc Returns one of win32con.MM_* values
static PyObject *PyGetMapMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int ret;
	if (!PyArg_ParseTuple(args, "O:GetMapMode",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetMapMode(hdc);
	if (ret==0)
		return PyWin_SetAPIError("GetMapMode");
	return PyInt_FromLong(ret);
}

// @pyswig int|SetMapMode|Sets the method used for translating logical units to device units
// @rdesc Returns the previous mapping mode, one of win32con.MM_* constants
static PyObject *PySetMapMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int newmode, prevmode;
	if (!PyArg_ParseTuple(args, "Oi:SetMapMode",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newmode))		// @pyparm int|MapMode||The new mapping mode (win32con.MM_*)
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevmode=SetMapMode(hdc, newmode);
	if (prevmode==0)
		return PyWin_SetAPIError("SetMapMode");
	return PyInt_FromLong(prevmode);
}

// @pyswig int|GetGraphicsMode|Determines if advanced GDI features are enabled for a device context
// @rdesc Returns GM_COMPATIBLE or GM_ADVANCED 
static PyObject *PyGetGraphicsMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int ret;
	if (!PyArg_ParseTuple(args, "O:GetGraphicsMode",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetGraphicsMode(hdc);
	if (ret==0)
		return PyWin_SetAPIError("GetGraphicsMode");
	return PyInt_FromLong(ret);
}

// @pyswig int|SetGraphicsMode|Enables or disables advanced graphics features for a DC
// @rdesc Returns the previous mode, one of win32con.GM_COMPATIBLE or win32con.GM_ADVANCED 
static PyObject *PySetGraphicsMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int newmode, prevmode;
	if (!PyArg_ParseTuple(args, "Oi:SetGraphicsMode",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newmode))		// @pyparm int|Mode||GM_COMPATIBLE or GM_ADVANCED (from win32con)
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevmode=SetGraphicsMode(hdc, newmode);
	if (prevmode==0)
		return PyWin_SetAPIError("SetGraphicsMode");
	return PyInt_FromLong(prevmode);
}

// @pyswig int|GetLayout|Retrieves the layout mode of a device context
// @rdesc Returns one of win32con.LAYOUT_*
static PyObject *PyGetLayout(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetLayout);
	HDC hdc;
	PyObject *obdc;
	DWORD prevlayout;
	if (!PyArg_ParseTuple(args, "O:GetLayout",
		&obdc))			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevlayout=(*pfnGetLayout)(hdc);
	if (prevlayout==GDI_ERROR)
		return PyWin_SetAPIError("GetLayout");
	return PyLong_FromUnsignedLong(prevlayout);
}

// @pyswig int|SetLayout|Sets the layout for a device context
// @rdesc Returns the previous layout mode
static PyObject *PySetLayout(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetLayout);
	HDC hdc;
	PyObject *obdc;
	DWORD newlayout, prevlayout;
	if (!PyArg_ParseTuple(args, "Ok:SetLayout",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newlayout))	// @pyparm int|Layout||One of win32con.LAYOUT_* constants
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevlayout=(*pfnSetLayout)(hdc, newlayout);
	if (prevlayout==GDI_ERROR)
		return PyWin_SetAPIError("SetLayout");
	return PyLong_FromUnsignedLong(prevlayout);
}

// @pyswig int|GetPolyFillMode|Returns the polygon filling mode for a device context
// @rdesc Returns win32con.ALTERNATE or win32con.WINDING 
static PyObject *PyGetPolyFillMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int ret;
	if (!PyArg_ParseTuple(args, "O:GetPolyFillMode",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetPolyFillMode(hdc);
	if (ret==0)
		return PyWin_SetAPIError("GetPolyFillMode");
	return PyInt_FromLong(ret);
}

// @pyswig int|SetPolyFillMode|Sets the polygon filling mode for a device context
// @rdesc Returns the previous mode, one of win32con.ALTERNATE or win32con.WINDING 
static PyObject *PySetPolyFillMode(PyObject *self, PyObject *args)
{
	HDC hdc;
	PyObject *obdc;
	int newmode, prevmode;
	if (!PyArg_ParseTuple(args, "Oi:SetPolyFillMode",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newmode))		// @pyparm int|PolyFillMode||One of ALTERNATE or WINDING 
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	prevmode=SetPolyFillMode(hdc, newmode);
	if (prevmode==0)
		return PyWin_SetAPIError("SetPolyFillMode");
	return PyInt_FromLong(prevmode);
}
%}
%native (GetTextExtentPoint32) PyGetTextExtentPoint32;
%native (GetTextMetrics) PyGetTextMetrics;
%native (GetTextCharacterExtra) PyGetTextCharacterExtra;
%native (SetTextCharacterExtra) PySetTextCharacterExtra;
%native (GetTextAlign) PyGetTextAlign;
%native (SetTextAlign) PySetTextAlign;
%native (GetTextFace) PyGetTextFace;
%native (GetMapMode) PyGetMapMode;
%native (SetMapMode) PySetMapMode;
%native (GetGraphicsMode) PyGetGraphicsMode;
%native (SetGraphicsMode) PySetGraphicsMode;
%native (GetLayout) PyGetLayout;
%native (SetLayout) PySetLayout;
%native (GetPolyFillMode) PyGetPolyFillMode;
%native (SetPolyFillMode) PySetPolyFillMode;

%{
// @object PyXFORM|Dict representing an XFORM struct used as a world transformation matrix
//	All members are optional, defaulting to 0.0.
// @pyseeapi XFORM struct
BOOL PyWinObject_AsXFORM(PyObject *obxform, XFORM *pxform)
{
	static char *keywords[]={"M11","M12","M21","M22","Dx","Dy", NULL};
	ZeroMemory(pxform, sizeof(XFORM));
	if (!PyDict_Check(obxform)){
		PyErr_SetString(PyExc_TypeError,"XFORM must be a dict");
		return FALSE;
		}
	PyObject *dummy_tuple=PyTuple_New(0);
	if (dummy_tuple==NULL)
		return FALSE;
	BOOL ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obxform, "|ffffff", keywords,
		&pxform->eM11,	// @prop float|M11|Usage is dependent on operation performed, see MSDN docs
		&pxform->eM12,	// @prop float|M12|Usage is dependent on operation performed, see MSDN docs
		&pxform->eM21,	// @prop float|M21|Usage is dependent on operation performed, see MSDN docs
		&pxform->eM22,	// @prop float|M22|Usage is dependent on operation performed, see MSDN docs
		&pxform->eDx,	// @prop float|Dx|Horizontal offset in logical units
		&pxform->eDy);	// @prop float|Dy|Vertical offset in logical units
	Py_DECREF(dummy_tuple);
	return ret;
}

PyObject *PyWinObject_FromXFORM(XFORM *pxform)
{
	return Py_BuildValue("{s:f,s:f,s:f,s:f,s:f,s:f}",
		"M11",	pxform->eM11,
		"M12",	pxform->eM12,
		"M21",	pxform->eM21,
		"M22",	pxform->eM22,
		"Dx",	pxform->eDx,
		"Dy",	pxform->eDy);
}

// @pyswig <o PyXFORM>|GetWorldTransform|Retrieves a device context's coordinate space translation matrix
// @comm DC's mode must be set to GM_ADVANCED.  See <om win32gui.SetGraphicsMode>.
static PyObject *PyGetWorldTransform(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetWorldTransform);
	PyObject *obdc;
	HDC hdc;
	XFORM xform;
	if (!PyArg_ParseTuple(args, "O:GetWorldTransform",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!(*pfnGetWorldTransform)(hdc, &xform))
		return PyWin_SetAPIError("GetWorldTransform");
	return PyWinObject_FromXFORM(&xform);
}

// @pyswig |SetWorldTransform|Transforms a device context's coordinate space
// @comm DC's mode must be set to GM_ADVANCED.  See <om win32gui.SetGraphicsMode>.
static PyObject *PySetWorldTransform(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetWorldTransform);
	PyObject *obdc, *obxform;
	HDC hdc;
	XFORM xform;
	if (!PyArg_ParseTuple(args, "OO:SetWorldTransform",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obxform))	// @pyparm <o PyXFORM>|Xform||Matrix defining the transformation
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsXFORM(obxform, &xform))
		return NULL;
	if (!(*pfnSetWorldTransform)(hdc, &xform))
		return PyWin_SetAPIError("SetWorldTransform");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig |ModifyWorldTransform|Combines a coordinate tranformation with device context's current transformation
// @comm DC's mode must be set to GM_ADVANCED.  See <om win32gui.SetGraphicsMode>.
static PyObject *PyModifyWorldTransform(PyObject *self, PyObject *args)
{
	CHECK_PFN(ModifyWorldTransform);
	PyObject *obdc, *obxform;
	HDC hdc;
	XFORM xform;
	DWORD mode;
	if (!PyArg_ParseTuple(args, "OOk:ModifyWorldTransform",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obxform,	// @pyparm <o PyXFORM>|Xform||Transformation to be applied.  Ignored if Mode is MWT_IDENTITY.
		&mode))		// @pyparm int|Mode||One of win32con.MWT_* values specifying how transformations will be combined
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsXFORM(obxform, &xform))
		return NULL;
	if (!(*pfnModifyWorldTransform)(hdc, &xform, mode))
		return PyWin_SetAPIError("ModifyWorldTransform");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig <o PyXFORM>|CombineTransform|Combines two coordinate space transformations
static PyObject *PyCombineTransform(PyObject *self, PyObject *args)
{
	CHECK_PFN(CombineTransform);
	PyObject *obxform1, *obxform2;
	XFORM xform1, xform2, ret_xform;
	if (!PyArg_ParseTuple(args, "OO:CombineTransform",
		&obxform1,		// @pyparm <o PyXFORM>|xform1||First transformation
		&obxform2))		// @pyparm <o PyXFORM>|xform2||Second transformation
		return NULL;
	if (!PyWinObject_AsXFORM(obxform1, &xform1))
		return NULL;
	if (!PyWinObject_AsXFORM(obxform2, &xform2))
		return NULL;

	if (!(*pfnCombineTransform)(&ret_xform, &xform1, &xform2))
		return PyWin_SetAPIError("CombineTransform");
	return PyWinObject_FromXFORM(&ret_xform);
}
%}
%native (GetWorldTransform) PyGetWorldTransform;
%native (SetWorldTransform) PySetWorldTransform;
%native (ModifyWorldTransform) PyModifyWorldTransform;
%native (CombineTransform) PyCombineTransform;


// @pyswig (int,int)|GetWindowOrgEx|Retrievs the window origin for a DC
BOOLAPI GetWindowOrgEx(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	POINT *OUTPUT);

// @pyswig (int,int)|SetWindowOrgEx|Changes the window origin for a DC
// @rdesc Returns the previous origin
BOOLAPI SetWindowOrgEx(
	HDC hdc,	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int X,		// @pyparm int|X||New X coord in logical units
	int Y,		// @pyparm int|Y||New Y coord in logical units
	POINT *OUTPUT);

// @pyswig (int,int)|GetViewportOrgEx|Retrievs the origin for a DC's viewport
BOOLAPI GetViewportOrgEx(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	POINT *OUTPUT);

// @pyswig (int,int)|SetViewportOrgEx|Changes the viewport origin for a DC
// @rdesc Returns the previous origin as (x,y)
BOOLAPI SetViewportOrgEx(
	HDC hdc,	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int X,		// @pyparm int|X||New X coord in logical units
	int Y,		// @pyparm int|Y||New Y coord in logical units
	POINT *OUTPUT);



%{
PyObject *PyWinObject_FromSIZE(PSIZE psize)
{
	return Py_BuildValue("ll", psize->cx, psize->cy);
}

// @pyswig (int,int)|GetWindowExtEx|Retrieves the window extents for a DC
// @rdesc Returns the extents as (x,y) in logical units
static PyObject *PyGetWindowExtEx(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	SIZE sz;
	if (!PyArg_ParseTuple(args, "O:GetWindowExtEx",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!GetWindowExtEx(hdc, &sz))
		return PyWin_SetAPIError("GetWindowExtEx");
	return PyWinObject_FromSIZE(&sz);
}

// @pyswig (int,int)|SetWindowExtEx|Changes the window extents for a DC
// @rdesc Returns the previous extents
static PyObject *PySetWindowExtEx(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	SIZE sz;
	int x,y;
	if (!PyArg_ParseTuple(args, "Oii:SetWindowExtEx",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,			// @pyparm int|XExtent||New X extent in logical units
		&y))		// @pyparm int|YExtent||New Y extent in logical units
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!SetWindowExtEx(hdc, x, y, &sz))
		return PyWin_SetAPIError("SetWindowExtEx");
	return PyWinObject_FromSIZE(&sz);
}

// @pyswig (int,int)|GetViewportExtEx|Retrieves the viewport extents for a DC
// @rdesc Returns the extents as (x,y) in logical units
static PyObject *PyGetViewportExtEx(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	SIZE sz;
	if (!PyArg_ParseTuple(args, "O:GetViewportExtEx",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!GetViewportExtEx(hdc, &sz))
		return PyWin_SetAPIError("GetViewportExtEx");
	return PyWinObject_FromSIZE(&sz);
}

// @pyswig (int,int)|SetViewportExtEx|Changes the viewport extents for a DC
// @rdesc Returns the previous extents as (x,y) in logical units
static PyObject *PySetViewportExtEx(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	SIZE sz;
	int x,y;
	if (!PyArg_ParseTuple(args, "Oii:SetViewportExtEx",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,			// @pyparm int|XExtent||New X extent in logical units
		&y))		// @pyparm int|YExtent||New Y extent in logical units
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!SetViewportExtEx(hdc, x, y, &sz))
		return PyWin_SetAPIError("SetViewportExtEx");
	return PyWinObject_FromSIZE(&sz);
}
%}
%native (GetWindowExtEx) PyGetWindowExtEx;
%native (SetWindowExtEx) PySetWindowExtEx;
%native (GetViewportExtEx) PyGetViewportExtEx;
%native (SetViewportExtEx) PySetViewportExtEx;

%{
// @object PyTRIVERTEX|Dict representing a TRIVERTEX struct containing color information at a point
// @pyseeapi TRIVERTEX
BOOL PyWinObject_AsTRIVERTEX(PyObject *obtv, TRIVERTEX *ptv)
{
	static char *keywords[]={"x","y","Red","Green","Blue","Alpha", NULL};
	if (!PyDict_Check(obtv)){
		PyErr_SetString(PyExc_TypeError,"TRIVERTEX must be a dict");
		return FALSE;
		}
	PyObject *dummy_tuple=PyTuple_New(0);
	if (dummy_tuple==NULL)
		return FALSE;
	BOOL ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obtv, "llHHHH", keywords,
		&ptv->x,		// @prop int|x|X coord in logical units
		&ptv->y,		// @prop int|y|Y coord in logical units
		&ptv->Red,		// @prop int|Red|Red component
		&ptv->Green,	// @prop int|Green|Green component
		&ptv->Blue,		// @prop int|Blue|Blue component
		&ptv->Alpha);	// @prop int|Alpha|Transparency value
	Py_DECREF(dummy_tuple);
	return ret;
}

BOOL PyWinObject_AsTRIVERTEXArray(PyObject *obtvs, TRIVERTEX **ptvs, DWORD *item_cnt)
{
	BOOL ret=TRUE;
	DWORD bufsize, tuple_index;
	PyObject *trivertex_tuple=NULL, *tuple_item;
	*ptvs=NULL;
	*item_cnt=0;

	if ((trivertex_tuple=PyWinSequence_Tuple(obtvs, item_cnt))==NULL)
		return FALSE;
	bufsize=*item_cnt * sizeof(TRIVERTEX);
	*ptvs=(TRIVERTEX *)malloc(bufsize);
	if (*ptvs==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		ret=FALSE;
		}
	else
		for (tuple_index=0; tuple_index<*item_cnt; tuple_index++){
			tuple_item=PyTuple_GET_ITEM(trivertex_tuple,tuple_index);
			if (!PyWinObject_AsTRIVERTEX(tuple_item, &(*ptvs)[tuple_index])){
				ret=FALSE;
				break;
				}
			}
	if (!ret)
		if (*ptvs!=NULL){
			free(*ptvs);
			*ptvs=NULL;
			*item_cnt=0;
			}
	Py_XDECREF(trivertex_tuple);
	return ret;
}

BOOL PyWinObject_AsMeshArray(PyObject *obmesh, ULONG mode, void **pmesh, DWORD *item_cnt)
{
	BOOL ret=TRUE, triangle;
	DWORD bufsize, tuple_index;
	PyObject *mesh_tuple=NULL, *tuple_item;
	*pmesh=NULL;
	*item_cnt=0;

	if ((mesh_tuple=PyWinSequence_Tuple(obmesh, item_cnt))==NULL)
		return FALSE;
	switch (mode){
		case GRADIENT_FILL_TRIANGLE:
			bufsize=*item_cnt * sizeof(GRADIENT_TRIANGLE);
			triangle=TRUE;
			break;
		case GRADIENT_FILL_RECT_H:
		case GRADIENT_FILL_RECT_V:
			bufsize=*item_cnt * sizeof(GRADIENT_RECT);
			triangle=FALSE;
			break;
		default:
			PyErr_Format(PyExc_ValueError,"Unrecognized value for gradient fill mode: %d", mode);
			return FALSE;
		}

	*pmesh=malloc(bufsize);
	if (*pmesh==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		ret=FALSE;
		}
	else
		for (tuple_index=0; tuple_index<*item_cnt; tuple_index++){
			tuple_item=PyTuple_GET_ITEM(mesh_tuple,tuple_index);
			if (!PyTuple_Check(tuple_item)){
				PyErr_SetString(PyExc_TypeError,"Mesh elements must be tuples of 2 or 3 ints");
				ret=FALSE;
				break;
				}
			if (triangle){
				if (!PyArg_ParseTuple(tuple_item, "kkk:GRADIENT_TRIANGLE",
					&((GRADIENT_TRIANGLE *)(*pmesh))[tuple_index].Vertex1,
					&((GRADIENT_TRIANGLE *)(*pmesh))[tuple_index].Vertex2,
					&((GRADIENT_TRIANGLE *)(*pmesh))[tuple_index].Vertex3)){
					ret=FALSE;
					break;
					}
				}
			else
				if (!PyArg_ParseTuple(tuple_item, "kk:GRADIENT_RECT", 
					&((GRADIENT_RECT *)(*pmesh))[tuple_index].UpperLeft,
					&((GRADIENT_RECT *)(*pmesh))[tuple_index].LowerRight)){
					ret=FALSE;
					break;
					}
			}
	if (!ret)
		if (*pmesh!=NULL){
			free(*pmesh);
			*pmesh=NULL;
			*item_cnt=0;
			}
	Py_XDECREF(mesh_tuple);
	return ret;
}

// @pyswig |GradientFill|Shades triangles or rectangles by interpolating between vertex colors
static PyObject *PyGradientFill(PyObject *self, PyObject *args)
{
	CHECK_PFN(GradientFill);
	HDC hdc;
	PTRIVERTEX ptv=NULL;
	ULONG tv_cnt, mesh_cnt, mode;
	PVOID pmesh=NULL;
	BOOL bres;
	PyObject *obdc, *obtvs, *obmesh, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OOOk:GradientFill",
		&obdc,		// @pyparm int|hdc||Handle to device context
		&obtvs,		// @pyparm (<o PyTRIVERTEX>,...)|Vertex||Sequence of TRIVERTEX dicts defining color info
		&obmesh,	// @pyparm tuple|Mesh||Sequence of tuples containing either 2 or 3 ints that index into the trivertex array to define either triangles or rectangles
		&mode))		// @pyparm int|Mode||win32con.GRADIENT_FILL_* value defining whether to fill by triangle or by rectangle
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsTRIVERTEXArray(obtvs, &ptv, &tv_cnt))
		goto cleanup;
	if (!PyWinObject_AsMeshArray(obmesh, mode, &pmesh, &mesh_cnt))
		goto cleanup;
	Py_BEGIN_ALLOW_THREADS
	bres=(*pfnGradientFill)(hdc, ptv, tv_cnt, pmesh, mesh_cnt, mode);
	Py_END_ALLOW_THREADS
	if (!bres)
		PyWin_SetAPIError("GradientFill");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	cleanup:
	if (ptv)
		free(ptv);
	if (pmesh)
		free(pmesh);
	return ret;
}
%}
%native (GradientFill) PyGradientFill;


// @pyswig int|GetOpenFileName|Creates an Open dialog box that lets the user specify the drive, directory, and the name of a file or set of files to open.
// @comm The <om win32gui.GetOpenFileNameW> function is far more convenient to use.
// @rdesc If the user presses OK, the function returns TRUE.  Otherwise, use CommDlgExtendedError for error details
// (ie, a win32gui.error is raised).  If the user cancels the dialog, the winerror attribute of the exception will be zero.
// @pyparm string/bytes|OPENFILENAME||A string packed into an OPENFILENAME structure, probably via the struct module.

BOOL GetOpenFileName(OPENFILENAME *INPUT);

#ifndef MS_WINCE

%typemap (python, in) MENUITEMINFO *INPUT (Py_ssize_t target_size){
	if (0 != PyObject_AsReadBuffer($source, (const void **)&$target, &target_size))
		return NULL;
	if (sizeof(MENUITEMINFO) != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string/buffer (got %d bytes)", sizeof(MENUITEMINFO), target_size);
}

%typemap (python,in) MENUITEMINFO *BOTH(Py_ssize_t target_size) {
	if (0 != PyObject_AsWriteBuffer($source, (void **)&$target, &target_size))
		return NULL;
	if (sizeof(MENUITEMINFO) != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte buffer (got %d bytes)", sizeof(MENUITEMINFO), target_size);
}

%typemap (python, in) MENUINFO *INPUT (Py_ssize_t target_size){
	if (0 != PyObject_AsReadBuffer($source, (const void **)&$target, &target_size))
		return NULL;
	if (sizeof(MENUINFO) != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte string/buffer (got %d bytes)", sizeof(MENUINFO), target_size);
}

%typemap (python,in) MENUINFO *BOTH(Py_ssize_t target_size) {
	if (0 != PyObject_AsWriteBuffer($source, (void **)&$target, &target_size))
		return NULL;
	if (sizeof(MENUINFO) != target_size)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d-byte buffer (got %d bytes)", sizeof(MENUINFO), target_size);
}

// @pyswig |InsertMenuItem|Inserts a menu item
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uItem||The menu item identifier or the menu item position. 
// @pyparm int|fByPosition||Boolean value of True if uItem is set to a menu item position. This parameter is set to False if uItem is set to a menu item identifier.
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI InsertMenuItem(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *INPUT);

// @pyswig |SetMenuItemInfo|Sets menu information
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uItem||The menu item identifier or the menu item position. 
// @pyparm int|fByPosition||Boolean value of True if uItem is set to a menu item position. This parameter is set to False if uItem is set to a menu item identifier.
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI SetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *INPUT);

// @pyswig |GetMenuItemInfo|Gets menu information
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uItem||The menu item identifier or the menu item position. 
// @pyparm int|fByPosition||Boolean value of True if uItem is set to a menu item position. This parameter is set to False if uItem is set to a menu item identifier.
// @pyparm buffer|menuItem||A string or buffer in the format of a <o MENUITEMINFO> structure.
BOOLAPI GetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, MENUITEMINFO *BOTH);

#endif

#ifndef MS_WINCE
// @pyswig int|GetMenuItemCount|
// @pyparm int|hMenu||Handle to the menu
int GetMenuItemCount(HMENU hMenu);

// @pyswig (int, int, int, int)|GetMenuItemRect|
// @pyparm int|hWnd||
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uItem||
int GetMenuItemRect(HWND hWnd, HMENU hMenu, UINT uItem, RECT *OUTPUT);

// @pyswig int|GetMenuState|
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uID||
// @pyparm int|flags||
int GetMenuState(HMENU hMenu, UINT uID, UINT flags);

// @pyswig |SetMenuDefaultItem|
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|uItem||
// @pyparm int|fByPos||
BOOLAPI SetMenuDefaultItem(HMENU hMenu, UINT flags, UINT fByPos);

// @pyswig int|GetMenuDefaultItem|
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|fByPos||
// @pyparm int|flags||
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
// @pyparm int|hMenu||Handle to the menu
// @pyparm int|nPos||
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
  HBITMAP INPUT_NULLOK,		// @pyparm <o PyGdiHANDLE>|hBitmapUnchecked||handle to unchecked bitmap, can be None
  HBITMAP INPUT_NULLOK		// @pyparm <o PyGdiHANDLE>|hBitmapChecked||handle to checked bitmap, can be None
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

// @pyswig |SetMenuInfo|Sets information for a specified menu.
// @comm See win32gui_struct for helper functions.
// @comm This function will raise NotImplementedError on early platforms (eg, Windows NT.)
%{
PyObject *PySetMenuInfo(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetMenuInfo);
	PyObject *obMenu, *obInfo;
	HMENU hmenu;
	Py_ssize_t cbInfo;
	MENUINFO *pInfo;
	BOOL result;
	// @pyparm int|hmenu||handle to menu
	// @pyparm <o MENUINFO>|info||menu information in the format of a buffer.
	if (!PyArg_ParseTuple(args, "OO", &obMenu, &obInfo))
		return NULL;

	if (!PyWinObject_AsHANDLE(obMenu, (HANDLE *)&hmenu))
		return NULL;

	if (0 != PyObject_AsReadBuffer(obInfo, (const void **)&pInfo, &cbInfo))
		return NULL;
	if (sizeof(MENUINFO) != cbInfo)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d byte string/buffer (got %d bytes)", sizeof(MENUINFO), cbInfo);

	Py_BEGIN_ALLOW_THREADS
	result = (*pfnSetMenuInfo)(hmenu, pInfo);
	Py_END_ALLOW_THREADS
	if (!result)
		return PyWin_SetAPIError("SetMenuInfo");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (SetMenuInfo) PySetMenuInfo;


// @pyswig |GetMenuInfo|Gets information about a specified menu.
// @comm See win32gui_struct for helper functions.
// @comm This function will raise NotImplementedError on early platforms (eg, Windows NT.)
%{
PyObject *PyGetMenuInfo(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetMenuInfo);
	PyObject *obMenu, *obInfo;
	HMENU hmenu;
	Py_ssize_t cbInfo;
	MENUINFO *pInfo;
	BOOL result;
	// @pyparm int|hmenu||handle to menu
	// @pyparm buffer|info||A buffer to fill with the information.
	if (!PyArg_ParseTuple(args, "OO", &obMenu, &obInfo))
		return NULL;

	if (!PyWinObject_AsHANDLE(obMenu, (HANDLE *)&hmenu))
		return NULL;

	if (0 != PyObject_AsWriteBuffer(obInfo, (void **)&pInfo, &cbInfo))
		return NULL;
	if (sizeof(MENUINFO) != cbInfo)
		return PyErr_Format(PyExc_TypeError, "Argument must be a %d byte buffer (got %d bytes)", sizeof(MENUINFO), cbInfo);

	Py_BEGIN_ALLOW_THREADS
	result = (*pfnGetMenuInfo)(hmenu, pInfo);
	Py_END_ALLOW_THREADS
	if (!result)
		return PyWin_SetAPIError("GetMenuInfo");
	Py_INCREF(Py_None);
	return Py_None;
}
%}

%native (GetMenuInfo) PyGetMenuInfo;

// @pyswig |DrawFocusRect|Draws a standard focus outline around a rectangle
BOOLAPI DrawFocusRect(
	HDC hDC,		// @pyparm <o PyHANDLE>|hDC||Handle to a device context
	RECT *INPUT);	// @pyparm (int, int, int,int)|rc||Tuple of (left,top,right,bottom) defining the rectangle

// @pyswig (int, <o PyRECT>)|DrawText|Draws formatted text on a device context
// @rdesc Returns the height of the drawn text, and the rectangle coordinates
int DrawText(
	HDC hDC,			// @pyparm int/<o PyHANDLE>|hDC||The device context on which to draw
	TCHAR *INPUT,		// @pyparm str|String||The text to be drawn
	int nCount,			// @pyparm int|nCount||The number of characters, use -1 for simple null-terminated string
	RECT *BOTH,			// @pyparm <o PyRECT>|Rect||Tuple of 4 ints specifying the position (left, top, right, bottom)
	UINT uFormat);		// @pyparm int|Format||Formatting flags, combination of win32con.DT_* values

// @pyswig |LineTo|Draw a line from current position to specified point
BOOLAPI LineTo(
	HDC hdc,	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int XEnd,	// @pyparm int|XEnd||Horizontal position in logical units
	int YEnd);	// @pyparm int|YEnd||Vertical position in logical units

// @pyswig |Ellipse|Draws a filled ellipse on a device context
BOOLAPI Ellipse(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Device context on which to draw
	int LeftRect,		// @pyparm int|LeftRect||Left limit of ellipse
	int TopRect,		// @pyparm int|TopRect||Top limit of ellipse
	int RightRect,		// @pyparm int|RightRect||Right limit of ellipse
	int BottomRect);	// @pyparm int|BottomRect||Bottom limit of ellipse

// @pyswig |Pie|Draws a section of an ellipse cut by 2 radials
BOOLAPI Pie(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Device context on which to draw
	int LeftRect,	// @pyparm int|LeftRect||Left limit of ellipse
	int TopRect,	// @pyparm int|TopRect||Top limit of ellipse
	int RightRect,	// @pyparm int|RightRect||Right limit of ellipse
	int BottomRect,	// @pyparm int|BottomRect||Bottom limit of ellipse
	int XRadial1,	// @pyparm int|XRadial1||Horizontal pos of Radial1 endpoint
	int YRadial1,	// @pyparm int|YRadial1||Vertical pos of Radial1 endpoint
	int XRadial2,	// @pyparm int|XRadial2||Horizontal pos of Radial2 endpoint
	int YRadial2);	// @pyparm int|YRadial2||Vertical pos of Radial2 endpoint

// @pyswig |Arc|Draws an arc defined by an ellipse and 2 radials
BOOLAPI Arc(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Device context on which to draw
	int LeftRect,	// @pyparm int|LeftRect||Left limit of ellipse
	int TopRect,	// @pyparm int|TopRect||Top limit of ellipse
	int RightRect,	// @pyparm int|RightRect||Right limit of ellipse
	int BottomRect,	// @pyparm int|BottomRect||Bottom limit of ellipse
	int XStartArc,	// @pyparm int|XRadial1||Horizontal pos of Radial1 endpoint
	int YStartArc,	// @pyparm int|YRadial1||Vertical pos of Radial1 endpoint
	int XEndArc,	// @pyparm int|XRadial2||Horizontal pos of Radial2 endpoint
	int XEndArc);	// @pyparm int|YRadial2||Vertical pos of Radial2 endpoint

// @pyswig |ArcTo|Draws an arc defined by an ellipse and 2 radials
// @comm Draws exactly as <om win32gui.Arc>, but changes current drawing position
BOOLAPI ArcTo(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Device context on which to draw
	int LeftRect,	// @pyparm int|LeftRect||Left limit of ellipse
	int TopRect,	// @pyparm int|TopRect||Top limit of ellipse
	int RightRect,	// @pyparm int|RightRect||Right limit of ellipse
	int BottomRect,	// @pyparm int|BottomRect||Bottom limit of ellipse
	int XRadial1,	// @pyparm int|XRadial1||Horizontal pos of Radial1 endpoint
	int YRadial1,	// @pyparm int|YRadial1||Vertical pos of Radial1 endpoint
	int XRadial2,	// @pyparm int|XRadial2||Horizontal pos of Radial2 endpoint
	int YRadial2);	// @pyparm int|YRadial2||Vertical pos of Radial2 endpoint

%{
// @pyswig |AngleArc|Draws a line from current pos and a section of a circle's arc
static PyObject *PyAngleArc(PyObject *self, PyObject *args)
{
	CHECK_PFN(AngleArc);
	HDC hdc;
	int x,y;
	DWORD radius;
	FLOAT startangle, sweepangle;
	PyObject *obdc;
	if (!PyArg_ParseTuple(args, "Oiikff:AngleArc",
		&obdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,				// @pyparm int|Y||x pos of circle
		&y,				// @pyparm int|Y||y pos of circle
		&radius,		// @pyparm int|Radius||Radius of circle
		&startangle,	// @pyparm float|StartAngle||Angle where arc starts, in degrees
		&sweepangle))	// @pyparm float|SweepAngle||Angle that arc covers, in degrees
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!(*pfnAngleArc)(hdc, x, y, radius, startangle, sweepangle))
		return PyWin_SetAPIError("AngleArc");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (AngleArc) PyAngleArc;


// @pyswig |Chord|Draws a chord defined by an ellipse and 2 radials
BOOLAPI Chord(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Device context on which to draw
	int LeftRect,	// @pyparm int|LeftRect||Left limit of ellipse
	int TopRect,	// @pyparm int|TopRect||Top limit of ellipse
	int RightRect,	// @pyparm int|RightRect||Right limit of ellipse
	int BottomRect,	// @pyparm int|BottomRect||Bottom limit of ellipse
	int XRadial1,	// @pyparm int|XRadial1||Horizontal pos of Radial1 endpoint
	int YRadial1,	// @pyparm int|YRadial1||Vertical pos of Radial1 endpoint
	int XRadial2,	// @pyparm int|XRadial2||Horizontal pos of Radial2 endpoint
	int YRadial2);	// @pyparm int|YRadial2||Vertical pos of Radial2 endpoint

// @pyswig |ExtFloodFill|Fills an area with current brush
BOOLAPI ExtFloodFill(
	HDC hdc,		// @pyparm <o PyHANDLE>||hdc|Handle to a device context
	int XStart,		// @pyparm int|XStart||Horizontal starting pos
	int YStart,		// @pyparm int|YStart||Vertical starting pos
	COLORREF Color,	// @pyparm int|Color||RGB color value.  See <om win32api.RGB>.
	UINT FillType);	// @pyparm int|FillType||One of win32con.FLOODFILL* values

%{
// @pyswig int|SetPixel|Set the color of a single pixel
// @rdesc Returns the RGB color actually set, which may be different from the one passed in
static PyObject *PySetPixel(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	int x,y;
	COLORREF color, ret;
	if (!PyArg_ParseTuple(args, "Oiik:SetPixel",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,			// @pyparm int|X||Horizontal pos
		&y,			// @pyparm int|Y||Vertical pos
		&color))	// @pyparm int|Color||RGB color to be set.
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=SetPixel(hdc, x, y, color);
	if (ret==CLR_INVALID)
		return PyWin_SetAPIError("SetPixel");
	return PyLong_FromUnsignedLong(ret);
}

// @pyswig int|GetPixel|Returns the RGB color of a single pixel
static PyObject *PyGetPixel(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	int x,y;
	COLORREF ret;
	if (!PyArg_ParseTuple(args, "Oii:GetPixel",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,			// @pyparm int|XPos||Horizontal pos
		&y))		// @pyparm int|YPos||Vertical pos
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetPixel(hdc, x, y);
	if (ret==CLR_INVALID)
		return PyWin_SetAPIError("GetPixel");
	return PyLong_FromUnsignedLong(ret);
}

// @pyswig int|GetROP2|Returns the foreground mixing mode of a DC
// @rdesc Returns one of win32con.R2_* values
static PyObject *PyGetROP2(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	int ret;
	if (!PyArg_ParseTuple(args, "O:GetROP2",
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetROP2(hdc);
	if (ret==0)
		return PyWin_SetAPIError("GetROP2");
	return PyInt_FromLong(ret);
}

// @pyswig int|SetROP2|Sets the foreground mixing mode of a DC
// @rdesc Returns previous mode
static PyObject *PySetROP2(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	HDC hdc;
	int newmode, oldmode;
	if (!PyArg_ParseTuple(args, "Oi:SetROP2",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&newmode))	// @pyparm int|DrawMode||Mixing mode, one of win32con.R2_*.
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	oldmode=SetROP2(hdc, newmode);
	if (oldmode==0)
		return PyWin_SetAPIError("SetROP2");
	return PyInt_FromLong(oldmode);
}
%}
%native (SetPixel) PySetPixel;
%native (GetPixel) PyGetPixel;
%native (GetROP2) PyGetROP2;
%native (SetROP2) PySetROP2;

// @pyswig |SetPixelV|Sets the color of a single pixel to an approximation of specified color
BOOLAPI SetPixelV(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int X,				// @pyparm int|X||Horizontal pos
	int Y,				// @pyparm int|Y||Vertical pos
	COLORREF Color);	// @pyparm int|Color||RGB color to be set.


// @pyswig (int, int)|MoveToEx|Changes the current drawing position
// @rdesc Returns the previous position as (X, Y)
BOOLAPI MoveToEx(
	HDC hdc,	// @pyparm <o PyHANDLE>|hdc||Device context handle
	int X,	// @pyparm int|X||Horizontal pos in logical units
	int Y,	// @pyparm int|Y||Vertical pos in logical units
	POINT *OUTPUT);

// @pyswig (int,int)|GetCurrentPositionEx|Returns a device context's current drawing position
BOOLAPI GetCurrentPositionEx(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Device context
	POINT *OUTPUT);

// @pyswig int|GetArcDirection|Returns the direction in which rectangles and arcs are drawn
// @rdesc Recturns one of win32con.AD_* values
int GetArcDirection(
	HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig int|SetArcDirection|Sets the drawing direction for arcs and rectangles
// @rdesc Returns the previous direction, or 0 on error.
int SetArcDirection(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	int ArcDirection);	// @pyparm int|ArcDirection||One of win32con.AD_* constants

%{
BOOL PyWinObject_AsPOINTArray(PyObject *obpoints, POINT **ppoints, DWORD *item_cnt)
{
	BOOL ret=TRUE;
	DWORD bufsize, tuple_index;
	PyObject *points_tuple=NULL, *tuple_item;
	*ppoints=NULL;
	*item_cnt=0;

	if ((points_tuple=PyWinSequence_Tuple(obpoints, item_cnt))==NULL)
		return FALSE;

	bufsize=*item_cnt * sizeof(POINT);
	*ppoints=(POINT *)malloc(bufsize);
	if (*ppoints==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		ret=FALSE;
		}
	else
		for (tuple_index=0; tuple_index<*item_cnt; tuple_index++){
			tuple_item=PyTuple_GET_ITEM(points_tuple,tuple_index);
			if (!PyWinObject_AsPOINT(tuple_item, &(*ppoints)[tuple_index])){
				ret=FALSE;
				break;
				}
			}
	if (!ret)
		if (*ppoints!=NULL){
			free(*ppoints);
			*ppoints=NULL;
			*item_cnt=0;
			}
	Py_XDECREF(points_tuple);
	return ret;
}

// @pyswig |Polygon|Draws a closed filled polygon defined by a sequence of points
static PyObject *PyPolygon(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	DWORD point_cnt;
	PyObject *obpoints, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO:PolyGon",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obpoints))	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...)
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (!Polygon(hdc, points, point_cnt))
		PyWin_SetAPIError("PolyGon");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (points)
		free(points);
	return ret;
}

// @pyswig |Polyline|Connects a sequence of points using currently selected pen
static PyObject *PyPolyline(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	DWORD point_cnt;
	PyObject *obpoints, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO:Polyline", 
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obpoints))	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...)
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (!Polyline(hdc, points, point_cnt))
		PyWin_SetAPIError("Polyline");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (points)
		free(points);
	return ret;
}

// @pyswig |PolylineTo|Draws a series of lines starting from current position.  Updates current position with end point.
static PyObject *PyPolylineTo(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	DWORD point_cnt;
	PyObject *obpoints, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO:PolylineTo", 
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obpoints))	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...)
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (!PolylineTo(hdc, points, point_cnt))
		PyWin_SetAPIError("PolylineTo");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (points)
		free(points);
	return ret;
}

// @pyswig |PolyBezier|Draws a series of Bezier curves starting from first point specified.
// @comm Number of points must be a multiple of 3 plus 1.
static PyObject *PyPolyBezier(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	DWORD point_cnt;
	PyObject *obpoints, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO:PolyBezier", 
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obpoints))	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...).
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (!PolyBezier(hdc, points, point_cnt))
		PyWin_SetAPIError("PolyBezier");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (points)
		free(points);
	return ret;
}

// @pyswig |PolyBezierTo|Draws a series of Bezier curves starting from current drawing position.
// @comm Points must contain 3 points for each curve.  Current position is updated with last endpoint.
static PyObject *PyPolyBezierTo(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	DWORD point_cnt;
	PyObject *obpoints, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OO:PolyBezierTo", 
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&obpoints))	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...).
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (!PolyBezierTo(hdc, points, point_cnt))
		PyWin_SetAPIError("PolyBezierTo");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (points)
		free(points);
	return ret;
}

// @pyswig |PlgBlt|Copies color from a rectangle into a parallelogram
static PyObject *PyPlgBlt(PyObject *self, PyObject *args)
{
	CHECK_PFN(PlgBlt);
	HDC srcdc, dstdc;
	POINT *points=NULL;
	int x, y, width, height, xmask=0, ymask=0;
	DWORD point_cnt;
	HBITMAP mask;
	PyObject *obsrc, *obdst, *obmask=Py_None, *obpoints, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OOOiiii|Oii:PlgBlt",
		&obdst,		// @pyparm <o PyHANDLE>|Dest||Destination DC
		&obpoints,	// @pyparm tuple|Point||Sequence of 3 POINT tuples (x,y) describing a paralellogram
		&obsrc,		// @pyparm <o PyHANDLE>|Src||Source device context
		&x,			// @pyparm int|XSrc||Left edge of source rectangle
		&y,			// @pyparm int|YSrc||Top of source rectangle
		&width,		// @pyparm int|Width||Width of source rectangle
		&height,	// @pyparm int|Height||Height of source rectangle
		&obmask,	// @pyparm <o PyGdiHANDLE>|Mask|None|Handle to monochrome bitmap to mask source, can be None
		&xmask,		// @pyparm int|xMask|0|x pos in mask
		&ymask))	// @pyparm int|yMask|0|y pos in mask
		return NULL;
	if (!PyWinObject_AsHANDLE(obdst, (HANDLE *)&dstdc))
		return NULL;
	if (!PyWinObject_AsHANDLE(obsrc, (HANDLE *)&srcdc))
		return NULL;
	if (!PyWinObject_AsHANDLE(obmask, (HANDLE *)&mask))
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	if (point_cnt!=3)
		PyErr_SetString(PyExc_ValueError, "Points must contain exactly 3 points.");
	else if (!(*pfnPlgBlt)(dstdc, points, srcdc, x, y, width, height, mask, xmask, ymask))
		PyWin_SetAPIError("PlgBlt");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	free(points);
	return ret;
}

// @pyswig <o PyGdiHANDLE>|CreatePolygonRgn|Creates a region from a sequence of vertices
static PyObject *PyCreatePolygonRgn(PyObject *self, PyObject *args)
{
	POINT *points=NULL;
	DWORD point_cnt;
	int fillmode;
	PyObject *obpoints, *ret=NULL;
	HRGN hrgn;
	if (!PyArg_ParseTuple(args, "Oi:CreatePolygonRgn",
		&obpoints,	// @pyparm [(int,int),...]|Points||Sequence of POINT tuples: ((x,y),...).
		&fillmode))	// @pyparm int|PolyFillMode||Filling mode, one of ALTERNATE, WINDING 
		return NULL;
	if (!PyWinObject_AsPOINTArray(obpoints, &points, &point_cnt))
		return NULL;
	hrgn=CreatePolygonRgn(points, point_cnt, fillmode);
	if (hrgn==NULL)
		PyWin_SetAPIError("CreatePolygonRgn");
	else
		ret=PyWinObject_FromGdiHANDLE(hrgn);
	if (points)
		free(points);
	return ret;
}
%}
%native (Polygon) PyPolygon;
%native (Polyline) PyPolyline;
%native (PolylineTo) PyPolylineTo;
%native (PolyBezier) PyPolyBezier;
%native (PolyBezierTo) PyPolyBezierTo;
%native (PlgBlt) PyPlgBlt;
%native (CreatePolygonRgn) PyCreatePolygonRgn;

%{
//@pyswig int|ExtTextOut|Writes text to a DC.
static PyObject *PyExtTextOut(PyObject *self, PyObject *args)
{
	TCHAR *text=NULL;
	int x, y;
	DWORD strLen;
	UINT options;
	PyObject *obdc, *rectObject, *obtext, *widthObject = Py_None;
	RECT rect, *rectPtr;
	int *widths = NULL;
	HDC hdc;
	if (!PyArg_ParseTuple (args, "OiiiOO|O:ExtTextOut",
		&obdc,	// @pyparm <o PyHANDLE>|hdc||Handle to a device context
		&x,		// @pyparm x|int||The x coordinate to write the text to.
		&y,		// @pyparm y|int||The y coordinate to write the text to.
		&options,	// @pyparm nOptions|int||Specifies the rectangle type. This parameter can be one, both, or neither of ETO_CLIPPED and ETO_OPAQUE
		&rectObject,	// @pyparm <o PyRECT>|rect||Specifies the text's bounding rectangle.  (Can be None.)
		&obtext,	// @pyparm text|string||The text to write.
		&widthObject))	// @pyparm (width1, width2, ...)|tuple||Optional array of values that indicate distance between origins of character cells.
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
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

	if (!PyWinObject_AsTCHAR(obtext, &text, FALSE, &strLen))
		return NULL;

	// Parse out widths
	if (widthObject != Py_None) {
		BOOL error = !PyTuple_Check(widthObject);
		if (!error) {
			Py_ssize_t len = PyTuple_Size(widthObject);
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
			PyWinObject_FreeTCHAR(text);
			delete [] widths;
			return PyErr_Format(PyExc_TypeError,
			                    "The width param must be a tuple of integers with a length one less than that of the string");
		}
	}

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS;
	// @pyseeapi ExtTextOut
	ok = ExtTextOut(hdc, x, y, options, rectPtr, text, strLen, widths);
	Py_END_ALLOW_THREADS;
	PyWinObject_FreeTCHAR(text);
	delete [] widths;
	if (!ok)
		return PyWin_SetAPIError("ExtTextOut");
	Py_INCREF(Py_None);
	return Py_None;
	// @rdesc Always none.  If the function fails, an exception is raised.
}

%}
%native (ExtTextOut) PyExtTextOut;


// @pyswig int|GetTextColor|Returns the text color for a DC
// @rdesc Returns an RGB color.  On error, returns CLR_INVALID
COLORREF GetTextColor(
	HDC hdc);			// @pyparm int|hdc||Handle to a device context

// @pyswig int|SetTextColor|Changes the text color for a device context
// @rdesc Returns the previous color, or CLR_INVALID on failure
int SetTextColor(
	HDC hdc,			// @pyparm int|hdc||Handle to a device context
	COLORREF color);	// @pyparm int|color||The RGB color value - see <om win32api.RGB>

// @pyswig int|GetBkMode|Returns the background mode for a device context
// @rdesc Returns OPAQUE, TRANSPARENT, or 0 on failure
int GetBkMode(
	HDC hdc);				// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig int|SetBkMode|Sets the background mode for a device context
// @rdesc Returns the previous mode, or 0 on failure
int SetBkMode(
	HDC hdc,			// @pyparm int/<o PyHANDLE>|hdc||Handle to a device context
	int mode);			// @pyparm int|BkMode||OPAQUE or TRANSPARENT 

// @pyswig int|GetBkColor|Returns the background color for a device context
// @rdesc Returns an RGB color value.  On error, returns CLR_INVALID.
int GetBkColor(
	HDC hdc);			// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig int|SetBkColor|Sets the background color for a device context
// @rdesc Returns the previous color, or CLR_INVALID on failure
int SetBkColor(
	HDC hdc,			// @pyparm int/<o PyHANDLE>|hdc||Handle to a device context
	COLORREF col);			// @pyparm int|color||

// @pyswig <o PyRECT>|DrawEdge|Draws edge(s) of a rectangle
// @rdesc BF_ADJUST flag causes input rectange to be shrunk by size of border.. Rectangle is always returned.
BOOLAPI DrawEdge(
	/* ??? This function can change the input rectange if BF_ADJUST is in Flags.
		Need to send it back as output also. ??? */
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	RECT *BOTH,		// @pyparm <o PyRECT>|rc||Rectangle whose edge(s) will be drawn
	UINT edge,		// @pyparm int|edge||Combination of win32con.BDR_* flags, or one of win32con.EDGE_* flags
	UINT Flags);	// @pyparm int|Flags||Combination of win32con.BF_* flags

// @pyswig |FillRect|Fills a rectangular area with specified brush
int FillRect(
	HDC hDC,		// @pyparm <o PyHANDLE>|hDC||Handle to a device context
	RECT *INPUT,	// @pyparm <o PyRECT>|rc||Rectangle to be filled
	HBRUSH hbr);	// @pyparm <o PyGdiHANDLE>|hbr||Handle to brush to be used to fill area

// @pyswig |FillRgn|Fills a region with specified brush
BOOLAPI FillRgn(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to the device context
	HRGN hrgn,		// @pyparm <o PyGdiHANDLE>|hrgn||Handle to the region
	HBRUSH hbr);	// @pyparm <o PyGdiHANDLE>|hbr||Brush to be used

// @pyswig |PaintRgn|Paints a region with current brush
BOOLAPI PaintRgn(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to the device context
	HRGN hrgn);		// @pyparm <o PyGdiHANDLE>|hrgn||Handle to the region

// @pyswig |FrameRgn|Draws a frame around a region
BOOLAPI FrameRgn(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to the device context
	HRGN hrgn,		// @pyparm <o PyGdiHandle>|hrgn||Handle to the region
	HBRUSH hbr,		// @pyparm <o PyGdiHandle>|hbr||Handle to brush to be used
	int Width,		// @pyparm int|Width||Frame width
	int Height);	// @pyparm int|Height||Frame height

// @pyswig |InvertRgn|Inverts the colors in a region
BOOLAPI InvertRgn(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to the device context
	HRGN hrgn);		// @pyparm <o PyGdiHandle>|hrgn||Handle to the region

// @pyswig boolean|EqualRgn|Determines if 2 regions are equal
BOOL EqualRgn(
	HRGN SrcRgn1,	// @pyparm <o PyGdiHandle>|SrcRgn1||Handle to a region
	HRGN SrcRgn2);	// @pyparm <o PyGdiHandle>|SrcRgn2||Handle to a region

// @pyswig boolean|PtInRegion|Determines if a region contains a point
BOOL PtInRegion(
	HRGN hrgn,	// @pyparm <o PyGdiHandle>|hrgn||Handle to a region
	int X,		// @pyparm int|X||X coord
	int Y);		// @pyparm int|Y||Y coord

// @pyswig boolean|PtInRect|Determines if a rectangle contains a point
BOOL PtInRect(
	RECT *INPUT,	// @pyparm (int, int, int, int)|rect||The rect to check
	POINT INPUT);      // @pyparm (int,int)|point||The point

// @pyswig boolean|RectInRegion|Determines if a region and rectangle overlap at any point
BOOL RectInRegion(
	HRGN hrgn,		// @pyparm <o PyGdiHandle>|hrgn||Handle to a region
	RECT *INPUT);	// @pyparm <o PyRECT>|rc||Rectangle coordinates in logical units

// @pyswig |SetRectRgn|Makes an existing region rectangular
BOOLAPI SetRectRgn(
	HRGN hrgn,			// @pyparm <o PyGdiHandle>|hrgn||Handle to a region
	int LeftRect,		// @pyparm int|LeftRect||Left edge in logical units
	int TopRect,		// @pyparm int|TopRect||Top edge in logical units
	int RightRect,		// @pyparm int|RightRect||Right edge in logical units
	int BottomRect);	// @pyparm int|BottomRect||Bottom edge in logical units

// @pyswig int|CombineRgn|Combines two regions
// @rdesc Returns the type of region created, one of NULLREGION, SIMPLEREGION, COMPLEXREGION
int_regiontype CombineRgn(
	HRGN Dest,			// @pyparm <o PyGdiHandle>|Dest||Handle to existing region that will receive combined region
	HRGN Src1,			// @pyparm <o PyGdiHandle>|Src1||Handle to first region
	HRGN Src2,			// @pyparm <o PyGdiHandle>|Src2||Handle to second region
	int CombineMode);	// @pyparm int|CombineMode||One of RGN_AND,RGN_COPY,RGN_DIFF,RGN_OR,RGN_XOR 

// @pyswig |DrawAnimatedRects|Animates a rectangle in the manner of minimizing, mazimizing, or opening
BOOLAPI DrawAnimatedRects(
  HWND hwnd,	// @pyparm int|hwnd||handle to clipping window
  int idAni,	// @pyparm int|idAni||type of animation, win32con.IDANI_*
  RECT *INPUT,	// @pyparm <o PyRECT>|minCoords||rectangle coordinates (minimized)
  RECT *INPUT	// @pyparm <o PyRECT>|restCoords||rectangle coordinates (restored)
);

// @pyswig <o PyGdiHANDLE>|CreateSolidBrush|Creates a solid brush of specified color
HBRUSH CreateSolidBrush(
	COLORREF Color);	// @pyparm int|Color||RGB color value.  See <om win32api.RGB>.

// @pyswig <o PyGdiHANDLE>|CreatePatternBrush|Creates a brush using a bitmap as a pattern
HBRUSH CreatePatternBrush(
	HBITMAP hbmp);	// @pyparm <o PyGdiHANDLE>|hbmp||Handle to a bitmap

// @pyswig <o PyGdiHANDLE>|CreateHatchBrush|Creates a hatch brush with specified style and color
HBRUSH CreateHatchBrush(
  int Style,		// @pyparm int|Style||Hatch style, one of win32con.HS_* constants
  COLORREF clrref);	// @pyparm int|clrref||Rgb color value.  See <om win32api.RGB>.

// @pyswig <o PyGdiHANDLE>|CreatePen|Create a GDI pen
HPEN CreatePen(
	int PenStyle,		// @pyparm int|PenStyle||One of win32con.PS_* pen styles
	int Width,			// @pyparm int|Width||Drawing width in logical units.  Use zero for single pixel.
	COLORREF Color);	// @pyparm int|Color||RGB color value.  See <om win32api.RGB>.

// @pyswig int|GetSysColor|Returns the color of a window element
DWORD GetSysColor(int Index);	// @pyparm int|Index||One of win32con.COLOR_* values

// @pyswig <o PyGdiHANDLE>|GetSysColorBrush|Creates a handle to a system color brush
HBRUSH GetSysColorBrush(int nIndex);	// @pyparm int|Index||Index of a window element color (win32con.COLOR_*)

// @pyswig |InvalidateRect|Invalidates a rectangular area of a window and adds it to the window's update region
BOOLAPI InvalidateRect(
	HWND hWnd,			// @pyparm <o PyHANDLE>|hWnd||Handle to the window
	RECT *INPUT_NULLOK,	// @pyparm <o PyRECT>|Rect||Client coordinates defining area to be redrawn.  Use None for entire client area.
	BOOL bErase);		// @pyparm boolean|Erase||Indicates if background should be erased

#ifndef MS_WINCE
// Function is defined as returning int, but semantics are same as a boolean
// @pyswig |FrameRect|Draws an outline around a rectangle
BOOLAPI FrameRect(
	HDC hDC,		// @pyparm <o PyHANDLE>|hDC||Handle to a device context
	RECT *INPUT,	// @pyparm <o PyRECT>|rc||Rectangle around which to draw
	HBRUSH hbr);	// @pyparm <o PyGdiHANDLE>|hbr||Handle to brush created using CreateHatchBrush, CreatePatternBrush, CreateSolidBrush, or GetStockObject 
#endif	/* not MS_WINCE */

// @pyswig |InvertRect|Inverts the colors in a regtangular region
BOOLAPI InvertRect(
	HDC hDC,		// @pyparm <o PyHANDLE>|hDC||Handle to a device context
	RECT *INPUT);	// @pyparm <o PyRECT>|rc||Coordinates of rectangle to invert

// @pyswig <o PyHANDLE>|WindowFromDC|Finds the window associated with a device context
// @rdesc Returns a handle to the window, or 0 if the DC is not associated with a window
HWND WindowFromDC(
	HDC hDC);	// @pyparm <o PyHANDLE>|hDC||Handle to a device context

// @pyswig int|GetUpdateRgn|Copies the update region of a window into an existing region
// @rdesc Returns type of region, one of COMPLEXREGION, NULLREGION, or SIMPLEREGION 
int_regiontype GetUpdateRgn(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	HRGN hRgn,		// @pyparm <o PyGdiHANDLE>|hRgn||Handle to an existing region to receive update area
	BOOL Erase);	// @pyparm boolean|Erase||Indicates if window background is to be erased

// @pyswig int|GetWindowRgn|Copies the window region of a window into an existing region
// @rdesc Returns type of region, one of COMPLEXREGION, NULLREGION, or SIMPLEREGION 
int_regiontype GetWindowRgn(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	HRGN hRgn);		// @pyparm <o PyGdiHANDLE>|hRgn||Handle to an existing region that receives window region

// Function is declared as returning an int, but acts same as boolean
// @pyswig |SetWindowRgn|Sets the visible region of a window
// @comm On success, the system assumes ownership of the region so you should call the handle's Detach()
//	method to prevent it from being automatically closed.
BOOLAPI SetWindowRgn(
	HWND hWnd,			// @pyparm <o PyHANDLE>|hWnd||Handle to a window
	HRGN INPUT_NULLOK,	// @pyparm <o PyGdiHANDLE>|hRgn||Handle to region to be set, can be None
	BOOL Redraw);		// @pyparm boolean|Redraw||Indicates if window should be completely redrawn

#ifdef WINXPGUI
// @pyswig int, <o PyRECT>|GetWindowRgnBox|Returns the bounding box for a window's region
// @rdesc Returns type of region and rectangle coordinates in device units
int_regiontype GetWindowRgnBox(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to a window that has a window region. (see <om win32gui.SetWindowRgn>)
	RECT *OUTPUT);
// @comm Only available in winxpgui
#endif
// @pyswig |ValidateRgn|Removes a region from a window's update region
BOOLAPI ValidateRgn(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to the window
	HRGN hRgn);		// @pyparm <o PyGdiHANDLE>|hRgn||Region to be validated

// @pyswig |InvalidateRgn|Adds a region to a window's update region
BOOLAPI InvalidateRgn(
	HWND hWnd,		// @pyparm <o PyHANDLE>|hWnd||Handle to the window
	HRGN hRgn,		// @pyparm <o PyGdiHANDLE>|hRgn||Region to be redrawn
	BOOL Erase);	// @pyparm boolean|Erase||Indidates if background should be erased

// @pyswig int, <o PyRECT>|GetRgnBox|Calculates the bounding box of a region
// @rdesc Returns type of region (COMPLEXREGION, NULLREGION, or SIMPLEREGION) and rectangle in logical units 
int_regiontype GetRgnBox(
	HRGN hrgn,   // @pyparm <o PyGdiHANDLE>|hrgn||Handle to a region 
	RECT *OUTPUT);

// @pyswig int|OffsetRgn|Relocates a region
// @rdesc Returns type of region (COMPLEXREGION, NULLREGION, or SIMPLEREGION) 
int_regiontype OffsetRgn(
	HRGN hrgn,		// @pyparm <o PyGdiHANDLE>|hrgn||Handle to a region 
	int XOffset,	// @pyparm int|XOffset||Horizontal offset
	int YOffset);	// @pyparm int|YOffset||Vertical offset

// @pyswig |Rectangle|Creates a solid rectangle using currently selected pen and brush
BOOLAPI Rectangle(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to device context
	int nLeftRect,		// @pyparm int|LeftRect||Position of left edge of rectangle
	int nTopRect,		// @pyparm int|TopRect||Position of top edge of rectangle
	int nRightRect,		// @pyparm int|RightRect||Position of right edge of rectangle
	int nBottomRect);	// @pyparm int|BottomRect||Position of bottom edge of rectangle

// @pyswig |RoundRect|Draws a rectangle with elliptically rounded corners, filled using using current brush
BOOLAPI RoundRect(
	HDC hdc,			// @pyparm <o PyHANDLE>|hdc||Handle to device context
	int nLeftRect,		// @pyparm int|LeftRect||Position of left edge of rectangle
	int nTopRect,		// @pyparm int|TopRect||Position of top edge of rectangle
	int nRightRect,		// @pyparm int|RightRect||Position of right edge of rectangle
	int nBottomRect,	// @pyparm int|BottomRect||Position of bottom edge of rectangle
	int Width,			// @pyparm int|Width||Width of ellipse
	int Height);		// @pyparm int|Height||Height of ellipse

// @pyswig hdc, paintstruct|BeginPaint|
HDC BeginPaint(HWND hwnd, PAINTSTRUCT *OUTPUT);

// @pyswig |EndPaint|
// @pyparm int|hwnd||
// @pyparm paintstruct|ps||As returned from <om win32gui.BeginPaint>
BOOLAPI EndPaint(HWND hWnd,  PAINTSTRUCT *INPUT); 

// @pyswig |BeginPath|Initializes a path in a DC
BOOLAPI BeginPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig |EndPath|Finalizes a path begun by <om win32gui.BeginPath>
BOOLAPI EndPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig |AbortPath|Cancels a path begun by <om win32gui.BeginPath>
BOOLAPI AbortPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context

// @pyswig |CloseFigure|Closes a section of a path by connecting the beginning pos with the current pos
BOOLAPI CloseFigure(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains an open path. See <om win32gui.BeginPath>.

// @pyswig |FlattenPath|Flattens any curves in current path into a series of lines
BOOLAPI FlattenPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a closed path. See <om win32gui.EndPath>.

// @pyswig |FillPath|Fills a path with currently selected brush
// @comm Any open figures are closed and path is deselected from the DC.
BOOLAPI FillPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a finalized path. See <om win32gui.EndPath>.

// @pyswig |WidenPath|Widens current path by amount it would increase by if drawn with currently selected pen
BOOLAPI WidenPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a closed path. See <om win32gui.EndPath>.

// @pyswig |StrokePath|Draws current path with currently selected pen
BOOLAPI StrokePath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a closed path. See <om win32gui.EndPath>.

// @pyswig |StrokeAndFillPath|Combines operations of StrokePath and FillPath with no overlap
BOOLAPI StrokeAndFillPath(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a closed path. See <om win32gui.EndPath>.

// @pyswig float|GetMiterLimit|Retrieves the limit of miter joins for a DC
BOOLAPI GetMiterLimit(
	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	float *OUTPUT);

// @pyswig float|SetMiterLimit|Set the limit of miter joins for a DC
// @rdesc Returns the previous limit
BOOLAPI SetMiterLimit(
 	HDC hdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a device context
	float NewLimit,	// @pyparm float|NewLimit||New limit to be set
	float *OUTPUT);

// @pyswig <o PyGdiHANDLE>|PathToRegion|Converts a closed path in a DC to a region
// @comm On success, the path is deselected from the DC
HRGN PathToRegion(HDC hdc);	// @pyparm <o PyHANDLE>|hdc||Handle to a device context that contains a closed path. See <om win32gui.EndPath>.

%{
// @pyswig tuple,tuple|GetPath|Returns a sequence of points that describe the current path
// @rdesc Returns a sequence of POINT tuples, and a sequence of ints designating each point's function (combination of win32con.PT_* values)
static PyObject *PyGetPath(PyObject *self, PyObject *args)
{
	HDC hdc;
	POINT *points=NULL;
	BYTE *types=NULL;
	DWORD point_cnt=0, point_ind;
	PyObject *obpoints=NULL, *obtypes=NULL, *obdc, *ret=NULL;
	if (!PyArg_ParseTuple(args, "O:GetPath", 
		&obdc))		// @pyparm <o PyHANDLE>|hdc||Handle to a device context containing a finalized path.  See <om win32gui.EndPath>
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	point_cnt=GetPath(hdc, points, types, point_cnt);
	if (point_cnt==-1)
		return PyWin_SetAPIError("GetPath");

	points=(POINT *)malloc(point_cnt*sizeof(POINT));
	if (points==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes",point_cnt*sizeof(POINT));
		goto cleanup;
		}
	types=(BYTE *)malloc(point_cnt);
	if (types==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes",point_cnt);
		goto cleanup;
		}
	point_cnt=GetPath(hdc, points, types, point_cnt);
	if (point_cnt==-1){
		PyWin_SetAPIError("GetPath");
		goto cleanup;
		}
	obpoints=PyTuple_New(point_cnt);
	obtypes=PyTuple_New(point_cnt);
	if ((obpoints==NULL) || (obtypes==NULL))
		goto cleanup;
	for (point_ind=0; point_ind<point_cnt; point_ind++){
		PyObject *tuple_item=Py_BuildValue("ll", points[point_ind].x, points[point_ind].y);
		if (tuple_item==NULL)
			goto cleanup;
		PyTuple_SET_ITEM(obpoints, point_ind, tuple_item);
		tuple_item=PyInt_FromLong(types[point_ind]);
		if (tuple_item==NULL)
			goto cleanup;
		PyTuple_SET_ITEM(obtypes, point_ind, tuple_item);
		}
	ret=Py_BuildValue("OO", obpoints, obtypes);

	cleanup:
	Py_XDECREF(obpoints);
	Py_XDECREF(obtypes);
	if (points!=NULL)
		free(points);
	if (types!=NULL)
		free(types);
	return ret;
}
%}
%native (GetPath) PyGetPath;

// @pyswig <o PyGdiHandle>|CreateRoundRectRgn|Create a rectangular region with elliptically rounded corners,
HRGN CreateRoundRectRgn(
	int LeftRect,		// @pyparm int|LeftRect||Position of left edge of rectangle
	int TopRect,		// @pyparm int|TopRect||Position of top edge of rectangle
	int RightRect,		// @pyparm int|RightRect||Position of right edge of rectangle
	int BottomRect,		// @pyparm int|BottomRect||Position of bottom edge of rectangle
	int WidthEllipse,	// @pyparm int|WidthEllipse||Width of ellipse
	int HeightEllipse);	// @pyparm int|HeightEllipse||Height of ellipse

// @pyswig <o PyGdiHandle>|CreateRectRgnIndirect|Creates a rectangular region,
HRGN CreateRectRgnIndirect(RECT *INPUT);	// @pyparm <o PyRECT>|rc||Coordinates of rectangle

// @pyswig <o PyGdiHandle>|CreateEllipticRgnIndirect|Creates an ellipse region,
HRGN CreateEllipticRgnIndirect(RECT *INPUT);	// @pyparm <o PyRECT>|rc||Coordinates of bounding rectangle in logical units

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

// @pyswig (int, int)|GetCursorPos|retrieves the cursor's position, in screen coordinates. 
BOOLAPI GetCursorPos(
	POINT *OUTPUT);
 
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

// @pyswig |CreateCaret|Creates a new caret for a window
BOOLAPI CreateCaret(
	HWND hWnd,        // @pyparm int|hWnd||handle to owner window
	HBITMAP hBitmap,  // @pyparm <o PyGdiHANDLE>|hBitmap||handle to bitmap for caret shape
	int nWidth,       // @pyparm int|nWidth||caret width
	int nHeight       // @pyparm int|nHeight||caret height
); 

// @pyswig |DestroyCaret|Destroys caret for current task
BOOLAPI DestroyCaret();

// @pyswig int,<o PyRECT>|ScrollWindowEx|scrolls the content of the specified window's client area.
// @rdesc Returns the type of region invalidated by scrolling, and a rectangle defining the affected area.
int_regiontype ScrollWindowEx(
	HWND hWnd,			// @pyparm int|hWnd||handle to window to scroll
	int dx,				// @pyparm int|dx||Amount of horizontal scrolling, in device units
	int dy,				// @pyparm int|dy||Amount of vertical scrolling, in device units
	RECT *INPUT_NULLOK, // @pyparm <o PyRECT>|rcScroll||Scroll rectangle, can be None for entire client area
	RECT *INPUT_NULLOK,	// @pyparm <o PyRECT>|rcClip||Clipping rectangle, can be None
	HRGN INPUT_NULLOK,	// @pyparm <o PyGdiHandle>|hrgnUpdate||Handle to region which will be updated with area invalidated by scroll operation, can be None
	RECT *OUTPUT,
	UINT flags			// @pyparm int|flags||Scrolling flags, combination of SW_ERASE,SW_INVALIDATE,SW_SCROLLCHILDREN,SW_SMOOTHSCROLL.
						//	If SW_SMOOTHSCROLL is specified, use upper 16 bits to specify time in milliseconds.
); 


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
	Py_ssize_t len = PyTuple_Size(args);
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
	PyObject *obhwnd, *obInfo;

	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|nBar||Identifies the bar.
	// @pyparm <o PySCROLLINFO>|scollInfo||Scollbar info.
	// @pyparm int|bRedraw|1|Should the bar be redrawn?
	if (!PyArg_ParseTuple(args, "OiO|i:SetScrollInfo",
		&obhwnd, &nBar, &obInfo, &bRedraw))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
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
	PyObject *obhwnd;
	int nBar;
	UINT fMask = SIF_ALL;
	// @pyparm int|hwnd||The handle to the window.
	// @pyparm int|nBar||The scroll bar to examine.  Can be one of win32con.SB_CTL, win32con.SB_VERT or win32con.SB_HORZ
	// @pyparm int|mask|SIF_ALL|The mask for attributes to retrieve.
	if (!PyArg_ParseTuple(args, "Oi|i:GetScrollInfo", &obhwnd, &nBar, &fMask))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
	PyObject *obhwnd;
	TCHAR buf[256];
	// @pyparm <o PyHANDLE>|hwnd||The handle to the window
	if (!PyArg_ParseTuple(args, "O:GetClassName", &obhwnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	// dont bother with lock - no callback possible.
	int nchars = GetClassName(hwnd, buf, sizeof buf/sizeof buf[0]);
	if (nchars==0)
		return PyWin_SetAPIError("GetClassName");
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
	PyObject *ob, *obhwnd;
	PyObject *obParam = Py_None;
	// @pyparm int|hwnd||The handle to the window
	// @pyparm object|callback||A callback object, taking 3 params.
	// @pyparm object|param|None|The third param to the callback function.
	if (!PyArg_ParseTuple(args, "OO|O:ListView_SortItems", &obhwnd, &ob, &obParam))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
%}

%native (ListView_SortItems) PyListView_SortItems;

#ifndef MS_WINCE
%{
// @pyswig |ListView_SortItemsEx|Uses an application-defined comparison function to sort the items of a list view control.
static PyObject *
PyListView_SortItemsEx(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *ob, *obhwnd;
	PyObject *obParam = Py_None;
	// @pyparm int|hwnd||The handle to the window
	// @pyparm object|callback||A callback object, taking 3 params.
	// @pyparm object|param|None|The third param to the callback function.
	if (!PyArg_ParseTuple(args, "OO|O:ListView_SortItemsEx", &obhwnd, &ob, &obParam))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
	TCHAR *driver, *device, *dummyoutput=NULL;
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
		ret = PyWinLong_FromHANDLE(hdc);
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
	PyWinObject_FreeResourceId((WCHAR *)pofn->lpTemplateName);
	ZeroMemory(pofn, sizeof(OPENFILENAMEW));
}

// Forward declared so autoduck comments for parms will appear with GetOpenFileNameW
BOOL PyParse_OPENFILENAMEW_Args(PyObject *args, PyObject *kwargs, OPENFILENAMEW *pofn);

PyObject *PyReturn_OPENFILENAMEW_Output(OPENFILENAMEW *pofn)
{
	DWORD filechars, filterchars;
	// If OFN_ALLOWMULTISELECT is set, the terminator is 2 NULLs,
	// otherwise a single NULL.
	if (pofn->Flags & OFN_ALLOWMULTISELECT) {
		for (filechars=0;
		     filechars < pofn->nMaxFile-1 && !(pofn->lpstrFile[filechars]==0 && pofn->lpstrFile[filechars+1]==0);
		     filechars++)
		     ;
	} else {
		filechars = wcslen(pofn->lpstrFile);
	}
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
// separated by nulls, otherwise it will be the full path.  In other words, if you use the OFN_ALLOWMULTISELECT flag
// you should split this value on \0 characters and if the length of the result list is 1, it will be
// the full path, otherwise element 0 will be the directory and the rest of the elements will be filenames in
// this directory.<nl>
// Second is a unicode string containing user-selected filter, will be None if CustomFilter was not specified<nl>
// Third item contains flags pertaining to users input, such as OFN_READONLY and OFN_EXTENSIONDIFFERENT
// <nl>If the user presses cancel or an error occurs, a
// win32gui.error is raised.  If the user pressed cancel, the error number (ie, the winerror attribute of the exception) will be zero.
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
// @pyparm <o PyResourceId>|TemplateName|None|Name or resource id of dialog box template
static PyObject *PyGetSaveFileNameW(PyObject *self, PyObject *args, PyObject *kwargs)
{	
	PyObject *ret=NULL;
	OPENFILENAMEW ofn;

	if (!PyParse_OPENFILENAMEW_Args(args, kwargs, &ofn))
		return NULL;

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS;
	ok = GetSaveFileNameW(&ofn);
	Py_END_ALLOW_THREADS;
	if (!ok)
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

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS;
	ok = GetOpenFileNameW(&ofn);
	Py_END_ALLOW_THREADS;
	if (!ok)
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
		&obTemplateName))		// @pyparm <o PyResourceId>|TemplateName|None|Name or resource id of dialog box template
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

	ret=PyWinObject_AsHANDLE(obOwner, (PHANDLE)&pofn->hwndOwner) &&
		PyWinObject_AsHANDLE(obhInstance, (PHANDLE)&pofn->hInstance) &&
		PyWinObject_AsWCHAR(obFilter, (WCHAR **)&pofn->lpstrFilter, TRUE) &&
		PyWinObject_AsWCHAR(obInitialDir, (WCHAR **)&pofn->lpstrInitialDir, TRUE) &&
		PyWinObject_AsWCHAR(obTitle, (WCHAR **)&pofn->lpstrTitle, TRUE) &&
		PyWinObject_AsWCHAR(obDefExt, (WCHAR **)&pofn->lpstrDefExt, TRUE) &&
		PyWinObject_AsResourceIdW(obTemplateName, (WCHAR **)&pofn->lpTemplateName, TRUE);
		
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

BOOL PyWinObject_AsMINIMIZEDMETRICS(PyObject *ob, MINIMIZEDMETRICS *mm)
{
	static char *keywords[]={"iWidth","iHorzGap","iVertGap","iArrange",NULL};
	BOOL ret;
	ZeroMemory(mm, sizeof(MINIMIZEDMETRICS));
	mm->cbSize=sizeof(MINIMIZEDMETRICS);

	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError, "MINIMIZEDMETRICS must be a dict");
		return FALSE;
		}
	PyObject *dummy_args=PyTuple_New(0);
	if (dummy_args==NULL)	// should not happen, interpreter apparently caches the empty tuple
		return FALSE;
	ret=PyArg_ParseTupleAndKeywords(dummy_args, ob, "iiii:MINIMIZEDMETRICS", keywords,
		&mm->iWidth, &mm->iHorzGap, &mm->iVertGap, &mm->iArrange);
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

		// @flag SPI_GETMINIMIZEDMETRICS|Returns a dict representing a MINIMIZEDMETRICS struct.  Param is not used.
		case SPI_GETMINIMIZEDMETRICS:		
		// @flag SPI_SETMINIMIZEDMETRICS|Param should be a MINIMIZEDMETRICS dict as returned by SPI_GETMINIMIZEDMETRICS action
		case SPI_SETMINIMIZEDMETRICS:
			buflen = sizeof(MINIMIZEDMETRICS);
			uiParam=buflen;
			pvParam=malloc(buflen);
			if (pvParam==NULL){
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", buflen);
				goto done;
			}
			if (Action==SPI_GETMINIMIZEDMETRICS){
				if (obParam!=Py_None) {
					PyErr_Format(PyExc_ValueError,
				             "Don't supply a param for SPI_GETMINIMIZEDMETRICS");
					goto done;
					}
				memset(pvParam, 0, buflen);
				((MINIMIZEDMETRICS *)pvParam)->cbSize = buflen;
				}
			else
				if (!PyWinObject_AsMINIMIZEDMETRICS(obParam, (MINIMIZEDMETRICS *)pvParam))
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
		case SPI_GETMINIMIZEDMETRICS: {
			MINIMIZEDMETRICS *p = (MINIMIZEDMETRICS *)pvParam;
			ret = Py_BuildValue("{s:i,s:i,s:i,s:i}",
					"iWidth", p->iWidth,
					"iHorzGap", p->iHorzGap,
					"iVertGap", p->iVertGap,
					"iArrange", p->iArrange);
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
		case SPI_GETMINIMIZEDMETRICS:
		case SPI_SETMINIMIZEDMETRICS:
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
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
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
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!(*pfnGetLayeredWindowAttributes)(hwnd, &Key, &Alpha, &Flags))
		return PyWin_SetAPIError("GetLayeredWindowAttributes");
	return Py_BuildValue("kbk", Key, Alpha, Flags);
}
PyCFunction pfnPyGetLayeredWindowAttributes=(PyCFunction)PyGetLayeredWindowAttributes;
%}

// @pyswig |UpdateLayeredWindow|Updates the position, size, shape, content, and translucency of a layered window. 
// @comm This function is only available on Windows 2000 and later
// @comm Accepts keyword arguments.
%{
PyObject *PyUpdateLayeredWindow(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(UpdateLayeredWindow);
	static char *keywords[]={"hwnd","hdcDst","ptDst","size","hdcSrc",
		"ptSrc","Key","blend","Flags", NULL};
	HWND hwnd;
	HDC hdcDst, hdcSrc;
	PyObject *obhwnd, *obsrc=Py_None, *obdst=Py_None;
	PyObject *obptSrc=Py_None, *obptDst=Py_None, *obsize=Py_None, *obblend=Py_None;
	COLORREF crKey=0;
	POINT ptSrc, ptDst;
	POINT *pptSrc=NULL, *pptDst=NULL;
	SIZE size;
	SIZE *psize=NULL;
	BLENDFUNCTION blend={0,0,255,0};
	DWORD Flags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOOOOkOk:UpdateLayeredWindow", keywords,
		&obhwnd,	// @pyparm <o PyHANDLE>|hwnd||handle to layered window
		&obdst,		// @pyparm <o PyHANDLE>|hdcDst|None|handle to screen DC, can be None.  *Must* be None if hdcSrc is None
		&obptDst,	// @pyparm (x,y)|ptDst|None|New screen position, can be None.
		&obsize,	// @pyparm (cx, cy)|size|None|New size of the layered window, can be None.  *Must* be None if hdcSrc is None.
		&obsrc,		// @pyparm int|hdcSrc|None|handle to surface DC for the window, can be None
		&obptSrc,	// @pyparm (x,y)|ptSrc|None|layer position, can be None.  *Must* be None if hdcSrc is None.
		&crKey,		// @pyparm int|Key|0|Color key, generate using <om win32api.RGB>
		&obblend,	// @pyparm (int, int, int, int)|blend|(0,0,255,0)|<o PyBLENDFUNCTION> specifying alpha blending parameters
		&Flags))	// @pyparm int|Flags|0|One of the win32con.ULW_* values.  Use 0 if hdcSrc is None.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyWinObject_AsHANDLE(obdst, (HANDLE *)&hdcDst))
		return NULL;
	if (!PyWinObject_AsHANDLE(obsrc, (HANDLE *)&hdcSrc))
		return NULL;
	if (obblend!=Py_None)
		if (!PyWinObject_AsBLENDFUNCTION(obblend, &blend))
			return NULL;
	if (obptDst!=Py_None){
		if (!PyWinObject_AsPOINT(obptDst, &ptDst))
			return NULL;
		pptDst=&ptDst;
		}
	if (obsize!=Py_None){
		if (!PyWinObject_AsSIZE(obsize, &size))
			return NULL;
		psize=&size;
		}
	if (obptSrc!=Py_None){
		if (!PyWinObject_AsPOINT(obptSrc, &ptSrc))
			return NULL;
		pptSrc=&ptSrc;
		}

	BOOL ret;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnUpdateLayeredWindow)(hwnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, &blend, Flags);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("UpdateLayeredWindow");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnPyUpdateLayeredWindow=(PyCFunction)PyUpdateLayeredWindow;
%}
%native (UpdateLayeredWindow) pfnPyUpdateLayeredWindow;

%{
// @pyswig |AnimateWindow|Enables you to produce special effects when showing or hiding windows. There are three types of animation: roll, slide, and alpha-blended fade.
// @comm This function is available on Win2k and later
// @comm Accepts keyword args
PyObject *PyAnimateWindow(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(AnimateWindow);
	static char *keywords[]={"hwnd","Time","Flags", NULL};
	PyObject *obhwnd;
	HWND hwnd;
	DWORD duration, flags;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Okk", keywords,
		&obhwnd,	// @pyparm <o PyHANDLE>|hwnd||handle to window
		&duration,	// @pyparm int|Time||Duration of animation in ms
		&flags))	// @pyparm int|Flags||Animation type, combination of win32con.AW_* flags
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	BOOL ret;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnAnimateWindow)(hwnd, duration, flags);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("AnimateWindow");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnPyAnimateWindow=(PyCFunction)PyAnimateWindow;
%}
%native (AnimateWindow) pfnPyAnimateWindow;

%{
// @object PyLOGBRUSH|Dict representing a LOGBRUSH struct as used with <om win32gui.CreateBrushIndirect> and <om win32gui.ExtCreatePen>
// @pyseeapi LOGBRUSH
BOOL PyWinObject_AsLOGBRUSH(PyObject *oblb, LOGBRUSH *plb)
{
	static char *keywords[]={"Style","Color","Hatch", NULL};
	PyObject *obhatch;
	if (!PyDict_Check(oblb)){
		PyErr_SetString(PyExc_TypeError,"LOGBRUSH must be a dict");
		return FALSE;
		}
	PyObject *dummy_tuple=PyTuple_New(0);
	if (dummy_tuple==NULL)
		return FALSE;
	BOOL ret=PyArg_ParseTupleAndKeywords(dummy_tuple, oblb, "kkO", keywords,
		&plb->lbStyle,	// @prop int|Style|Brush style, one of win32con.BS_* values
		&plb->lbColor,	// @prop int|Color|RGB color value.  Can also be DIB_PAL_COLORS or DIB_RGB_COLORS if Style is BS_DIBPATTERN or BS_DIBPATTERNPT=
		&obhatch)		// @prop int/<o PyHANDLE>|Hatch|For BS_HATCH style, one of win32con.HS_*. Not used For BS_SOLID or BS_HOLLOW.
						// For a pattern brush, this should be a handle to a bitmap
		&&PyWinObject_AsHANDLE(obhatch, (HANDLE *)&plb->lbHatch);
	Py_DECREF(dummy_tuple);
	return ret;
}

// @pyswig <o PyGdiHANDLE>|CreateBrushIndirect|Creates a GDI brush from a LOGBRUSH struct
static PyObject *PyCreateBrushIndirect(PyObject *self, PyObject *args)
{
	PyObject *oblb;
	LOGBRUSH lb;
	HBRUSH hbrush;
	if (!PyArg_ParseTuple(args, "O:CreateBrushIndirect",
		&oblb))	// @pyparm <o PyLOGBRUSH>|lb||Dict containing brush creation parameters
	return NULL;
	if (!PyWinObject_AsLOGBRUSH(oblb, &lb))
		return NULL;
	hbrush=CreateBrushIndirect(&lb);
	if (hbrush==NULL)
		return PyWin_SetAPIError("CreateBrushIndirect");
	return PyWinObject_FromGdiHANDLE(hbrush);
}

// @pyswig <o PyHANDLE>|ExtCreatePen|Creates a GDI pen object
static PyObject *PyExtCreatePen(PyObject *self, PyObject *args)
{
	PyObject *oblb, *obcustom_style=Py_None;
	LOGBRUSH lb;
	HPEN hpen;
	DWORD style, width, custom_style_cnt;
	DWORD *custom_style=NULL;
	if (!PyArg_ParseTuple(args, "kkO|O:ExtCreatePen",
		&style,		// @pyparm int|PenStyle||Combination of win32con.PS_*.  Must contain either PS_GEOMETRIC or PS_COSMETIC.
		&width,		// @pyparm int|Width||Width of pen in logical units.  Must be 1 for PS_COSMETIC.
		&oblb,		// @pyparm <o PyLOGBRUSH>|lb||Dict containing brush creation parameters
		&obcustom_style))	// @pyparm (int, ...)|Style|None|Sequence containing lengths of dashes and spaces  Used only with PS_USERSTYLE, otherwise must be None.
		return NULL;
	if (!PyWinObject_AsLOGBRUSH(oblb, &lb))
		return NULL;
	if (!PyWinObject_AsDWORDArray(obcustom_style, &custom_style, &custom_style_cnt, TRUE))
		return NULL;
	hpen=ExtCreatePen(style, width, &lb, custom_style_cnt, custom_style);
	if (custom_style)
		free(custom_style);
	if (hpen==NULL)
		return PyWin_SetAPIError("ExtCreatePen");
	return PyWinObject_FromGdiHANDLE(hpen);
}
%}
%native (CreateBrushIndirect) PyCreateBrushIndirect;
%native (ExtCreatePen) PyExtCreatePen;

// @pyswig int,<o PyRECT>|DrawTextW|Draws Unicode text on a device context. 
// @comm Accepts keyword args.
// @rdesc Returns the height of the drawn text, and the rectangle coordinates
%{
PyObject *PyDrawTextW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(DrawTextW);
	static char *keywords[]={"hDC","String","Count","Rect","Format", NULL};
	HDC hdc;
	WCHAR *input_text;
	int len, height;
	RECT rc;
	UINT fmt;
	PyObject *obhdc, *obtxt, *obrc;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOiOI:DrawTextW", keywords,
		&obhdc,		// @pyparm <o PyHANDLE>|hDC||Handle to a device context
		&obtxt,		// @pyparm <o PyUnicode>|String||Text to be drawn
		&len,		// @pyparm int|Count||Number of characters to draw, use -1 for entire null terminated string
		&obrc,		// @pyparm <o PyRECT>|Rect||Rectangle in which to draw text
		&fmt))		// @pyparm int|Format||Formatting flags, combination of win32con.DT_* values
		return NULL;
	if (!PyWinObject_AsHANDLE(obhdc, (HANDLE *)&hdc))
		return NULL;
	if (!PyWinObject_AsRECT(obrc, &rc))
		return NULL;
	if (!PyWinObject_AsWCHAR(obtxt, &input_text, FALSE))
		return NULL;

	height=(*pfnDrawTextW)(hdc, input_text, len, &rc, fmt);
	PyWinObject_FreeWCHAR(input_text);
	if (!height)
		return PyWin_SetAPIError("DrawTextW");
	return Py_BuildValue("iN",
		height,
		PyWinObject_FromRECT(&rc));
}
PyCFunction pfnPyDrawTextW=(PyCFunction)PyDrawTextW;
%}
%native (DrawTextW) pfnPyDrawTextW;

%{
BOOL CALLBACK PyEnumPropsExCallback(HWND hwnd, LPWSTR propname, HANDLE propdata, ULONG_PTR callback_data)
{
	PyObject *args=NULL, *obret=NULL;
	BOOL ret;
	CEnterLeavePython _celp;
	PyObject **callback_objects=(PyObject **)callback_data;
	args=Py_BuildValue("NNNO",
		PyWinLong_FromHANDLE(hwnd),
		IS_INTRESOURCE(propname) ? PyWinLong_FromVoidPtr(propname):PyWinObject_FromWCHAR(propname),
		PyWinLong_FromHANDLE(propdata),
		callback_objects[1]);
	if (args==NULL)
		return FALSE;
	obret=PyObject_Call(callback_objects[0], args, NULL);
	if (obret==NULL)
		ret=FALSE;
	else
		ret=TRUE;

	Py_XDECREF(args);
	Py_XDECREF(obret);
	return ret;
}

// @pyswig |EnumPropsEx|Enumerates properties attached to a window.
// Each property is passed to a callback function, which receives 4 arguments:<nl>
//	Handle to the window, name of the property, handle to the property data, and Param object passed to this function
//  
PyObject *PyEnumPropsEx(PyObject *self, PyObject *args)
{
	HWND hwnd;
	PyObject *obhwnd, *callback, *callback_data;
	PyObject *callback_objects[2];

	if (!PyArg_ParseTuple(args, "OOO:EnumPropsEx",
		&obhwnd,			// @pyparm <o PyHANDLE>|hWnd||Handle to a window
		&callback,			// @pyparm function|EnumFunc||Callback function
		&callback_data))	// @pyparm object|Param||Arbitrary object to be passed to callback function
		return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyCallable_Check(callback)){
		PyErr_SetString(PyExc_TypeError,"EnumFunc must be callable");
		return NULL;
		}
	callback_objects[0]=callback;
	callback_objects[1]=callback_data;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = EnumPropsExW(hwnd, PyEnumPropsExCallback, (LPARAM)callback_objects);
	Py_END_ALLOW_THREADS
	if (!ok){
		if (!PyErr_Occurred())
			PyWin_SetAPIError("EnumPropsEx");
		return NULL;
		}
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native(EnumPropsEx) PyEnumPropsEx;

#ifdef WINXPGUI
// strictly available in win2kpro, but this will do for now...
HWND GetConsoleWindow();
#endif

%{
// @pyswig <o PyHDEVNOTIFY>|RegisterDeviceNotification|Registers the device or type of device for which a window will receive notifications.
PyObject *PyRegisterDeviceNotification(PyObject *self, PyObject *args)
{
	unsigned int flags;
	PyObject *obh, *obFilter;
	if (!PyArg_ParseTuple(args, "OOk:RegisterDeviceNotification",
			      &obh, // @pyparm <o PyHANDLE>|handle||The handle to a window or a service
			      &obFilter, // @pyparm buffer|filter||A buffer laid out like one of the DEV_BROADCAST_* structures, generally built by one of the win32gui_struct helpers.
			      &flags)) // @pyparm int|flags||
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obh, &handle))
		return NULL;
	const void *filter;
	Py_ssize_t nbytes;
	if (PyObject_AsReadBuffer(obFilter, &filter, &nbytes)==-1)
		return NULL;
	// basic sanity check.
	Py_ssize_t struct_bytes = ((DEV_BROADCAST_HDR *)filter)->dbch_size;
	if (nbytes != struct_bytes)
		return PyErr_Format(PyExc_ValueError,
				"buffer isn't a DEV_BROADCAST_* structure: "
				"structure says it has %d bytes, but %d was provided",
				(int)struct_bytes, (int)nbytes);
	// @pyseeapi RegisterDeviceNotification
	HDEVNOTIFY not;
	Py_BEGIN_ALLOW_THREADS
	not = RegisterDeviceNotification(handle, (void *)filter, flags);
	Py_END_ALLOW_THREADS
	if (not == NULL)
		return PyWin_SetAPIError("RegisterDeviceNotification");
	return PyWinObject_FromHDEVNOTIFY(not);
}
%}
%native(RegisterDeviceNotification) PyRegisterDeviceNotification;

// @pyswig |UnregisterDeviceNotification|Unregisters a Device Notification handle.
// It is generally not necessary to call this function manually, but in some cases,
// handle values may be extracted via the struct module and need to be closed explicitly.
BOOLAPI UnregisterDeviceNotification(HANDLE);

// @pyswig |RegisterHotKey|Registers a hotkey for a window
// @pyseeapi RegisterHotKey
// @pyparm <o PyHANDLE>|hWnd||Handle to window that will receive WM_HOTKEY messages
// @pyparm int|id||Unique id to be used for the hot key
// @pyparm int|Modifiers||Control keys, combination of win32con.MOD_*
// @pyparm int|vk||Virtual key code
BOOLAPI RegisterHotKey(HWND, int, UINT, UINT);
