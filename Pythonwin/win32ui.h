// -*- Mode: C++; tab-width: 4 -*-
//
// win32ui.h
//
// external declarations for the application.
//
//
#ifndef __WIN32UI_H__
#define __WIN32UI_H__

#ifdef FREEZE_WIN32UI
#	define PYW_EXPORT
#else
#	ifdef BUILD_PYW
#		define PYW_EXPORT __declspec(dllexport)
#	else
#		define PYW_EXPORT __declspec(dllimport)
#		ifdef _DEBUG
#			pragma comment(lib,"win32ui_d.lib")
#		else
#			pragma comment(lib,"win32ui.lib")
#		endif
#	endif
#endif

#include <afxtempl.h> // Bit of an unusual MFC header.
#include <afxext.h> // Also unusual - needed for CCreateContext.

// For MFC8 (VS2005), we need to nominate the MFC assembly - may as well do
// it here so its done once for all projects!
// BUT - this isn't needed any more for MFC9/VS2008
#if _MFC_VER >= 0x0800 && _MFC_VER < 0x0900
# pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.VC80.MFC' version='8.0.50727.762' processorArchitecture='*'  publicKeyToken='fc8b3b9a1e18e3b' language='*'\"")
#endif

#define DOINCREF(o) Py_INCREF(o)
#define DODECREF(o) Py_DECREF(o)
#define XDODECREF(o) Py_XDECREF(o)

inline PyObject *PyWinObject_FromTCHAR(CString *str )
{
	return PyWinObject_FromTCHAR((const TCHAR *)str);
}

// we cant use these memory operators - must use make and python handles delete
#undef NEWOBJ
#undef DEL


// implement a few byte overhead for type checking.
static char SIG[] = "py4w";

// Identical to Py_BEGIN_ALLOW_THREADS except no script "{" !!!
// means variables can be declared between the blocks
#define GUI_BGN_SAVE PyThreadState *_save = PyEval_SaveThread()
#define GUI_END_SAVE PyEval_RestoreThread(_save)
#define GUI_BLOCK_THREADS Py_BLOCK_THREADS

inline BOOL IsWin32s() {return FALSE;}

inline BOOL IsGdiHandleValid(HANDLE hobject) \
	{return hobject == NULL || IsWin32s() || ::GetObjectType(hobject) != 0;}


CString GetAPIErrorString(const char *fnName);
CString GetAPIErrorString(DWORD dwCode);

// The do/while clauses wrapped around these macro bodies are a cpp
// idiom - they allow you to unambiguously treat a macro 'call' - even
// one consisting of multiple statements - as a single statement,
// terminated by a semicolon. (SMR 960129)
extern PyObject *ReturnAPIError(const char *fn);
#define RETURN_NONE				do {Py_INCREF(Py_None);return Py_None;} while (0)
#define RETURN_ERR(err)			do {PyErr_SetString(ui_module_error,err);return NULL;} while (0)
#define RETURN_MEM_ERR(err)		do {PyErr_SetString(PyExc_MemoryError,err);return NULL;} while (0)
#define RETURN_TYPE_ERR(err)	do {PyErr_SetString(PyExc_TypeError,err);return NULL;} while (0)
#define RETURN_VALUE_ERR(err)	do {PyErr_SetString(PyExc_ValueError,err);return NULL;} while (0)
#define RETURN_API_ERR(fn) return ReturnAPIError(fn)

#define CHECK_NO_ARGS(args)		do {if (!PyArg_ParseTuple(args,"")) return NULL;} while (0)
#define CHECK_NO_ARGS2(args, fnName) do {if (!PyArg_ParseTuple(args,":"#fnName)) return NULL;} while (0)

extern PYW_EXPORT PyObject *ui_module_error;

// Note: design rules to be aware of when looking/coding/etc
// (Im making these up after most is coded already, and just about to implement!)
//
// All object creation must now be via ui_base_class::make
//
// For any object derived from ui_cmd_target, it is important there
// is exactly one c++ object per python object.  to support this,
// ui_cmd_target has a make that has an ASSOC object passed.  This will
// return a reference to an existing object if one already exists.
/*
 The general class hierarchy is:
		ui_base_class	Mainly Python helpers.
			|
			+ ui_assoc	All objects that maintain a mapping between
				|		an external C++ object and a Python object.
				|		(ie, all non trivial!)  Inherits all ui_base classes.
				|
				+ ui_assoc_CObject - base of all CObject partnered classes
					|
					+ ui_menu	Association is to the hMenu
					|
					+ ui_cmd_target		Does not define assoc.
					|	|
					|	+ ui_window 	Assoc is hWnd
					|		|
					|		+ ui_edit_window 	Inherits Window methods and Assoc.
					|		|
					|		+ ui_mdi_frame		Ditto.
					|		...
					|
					+ ui_dc		Assoc is hDC
					|
					+ ui_document	Assoc is CDocument pointer.


*/
//
// object types
//
// to make life convenient, I derive from PyObject, rather than "include"
// the structure at the start.  As PyObject has no virtual members, casts
// will offset the pointer.
// It is important that the functions which handle python methods
// only have self declared as "PyObject *", not "class *", as the
// vfptr stuffs things up.

class ui_base_class;
////////////////////

// helper typeobject class.
class PYW_EXPORT ui_type : public PyTypeObject {
public:
	ui_type( const char *name, ui_type *pBaseType, int typeSize, int pyobjOffset, struct PyMethodDef* methodList, ui_base_class * (* thector)() );
	~ui_type();
public:
	struct PyMethodDef* methods;
	ui_base_class * (* ctor)();
};

// a helper to calculate the offset from a ui_base_class child with a PyObject.
// Use a pointer value of 1 - can't use zero as casting NULL always ends up NULL.
#define PYOBJ_OFFSET(klass) ((BYTE *)(PyObject *)(klass *)1 - (BYTE *)(klass *)1)

// helper typeCObject class.
class PYW_EXPORT ui_type_CObject : public ui_type {
public:
	ui_type_CObject( const char *name, ui_type *pBaseType, CRuntimeClass *pRT, int typeSize, int pyobjOffset, struct PyMethodDef* methodList, ui_base_class * (* thector)() );
	~ui_type_CObject();
public:
	CRuntimeClass *pCObjectClass;
	// A map of CRuntimeClass to these objects.  Populated by the ctor.
	// Allows us to convert from an arbitary CObject to the best Python type.
	typedef CMap<CRuntimeClass *,CRuntimeClass *,ui_type_CObject *,ui_type_CObject *> CRuntimeClassTypeMap;
	static CRuntimeClassTypeMap* typemap;
};

PYW_EXPORT ui_type_CObject &UITypeFromCObject( CObject *ob );
PYW_EXPORT ui_type_CObject &UITypeFromHWnd( HWND hwnd );
PYW_EXPORT ui_type_CObject *UITypeFromName( const char *name );

CString GetReprText( PyObject *objectUse );

#ifdef _DEBUG
void DumpAssocPyObject( CDumpContext &dc , void *object );

#ifdef TRACK_PYTHON_OBJECTS
#define _DEBUG_TRACK_PYTHON_OBJECTS
#endif
#endif

#if defined(_DEBUG) && defined(TRACK_PYTHON_OBJECTS)
#define MAKE_PY_CTOR(classname) static ui_base_class * classname::PyObConstruct(void) {return new classname;}
#else
#define MAKE_PY_CTOR(classname) static ui_base_class * classname::PyObConstruct(void) { \
	BOOL bOld = AfxEnableMemoryTracking(FALSE); \
	ui_base_class * ret = new classname; \
	AfxEnableMemoryTracking(bOld); \
	return ret; }
#endif

#define GET_PY_CTOR(classname) classname::PyObConstruct

// general purpose base class for my C++ objects.
//
// Note that Python itself cannot create these data types itself - the program
// must call a module method to do so, so it is totally C++'s responsibility
// to enforce this.  To this end, all constructors are protected.

class PYW_EXPORT ui_base_class : 
#ifdef _DEBUG
			// In debug mode, we use MI!!  This gives us the ability
			// to dump these objects as MFC objects, aiding in leak detection
			// (now all we need do is track all those leaks :-)
			public CObject,
#endif
			public PyObject 
{
public:
	static ui_base_class *make( ui_type &type );

	// virtuals for Python support
	virtual CString repr();
	virtual PyObject *getattro(PyObject *obname);
	virtual int setattro(PyObject *obname, PyObject *v);
	virtual void cleanup();

	static ui_type type;							// my type.
protected:
	ui_base_class();
	virtual ~ui_base_class();

public:
	static BOOL is_uiobject( PyObject *&, ui_type *which);
	BOOL is_uiobject(ui_type *which);
	static void sui_dealloc(PyObject *ob);
	static PyObject *sui_repr(PyObject *ob);
	static PyObject *sui_getattro(PyObject *self, PyObject *obname);
	static int sui_setattro(PyObject *op, PyObject *obname, PyObject *v);
#ifdef _DEBUG
	DECLARE_DYNAMIC(ui_base_class)
	virtual void Dump( CDumpContext &dc ) const;
#endif
	PyObject *weakreflist; /* List of weak references */
private:
	char sig[sizeof(SIG)];
};

// for threading, must use GUI versions of these calls
PYW_EXPORT PyObject *gui_call_object(PyObject *themeth, PyObject *thearglist);
PYW_EXPORT void gui_print_error(void);
void gui_decref(PyObject *o);


//#endif // Py_ALLOBJECTS_H
//
// CreateContext used when creating frames etc.
//
class PYW_EXPORT PythonCreateContext : public CCreateContext {
public:
	PythonCreateContext();
	~PythonCreateContext();
	void SetPythonObject(PyObject *ob);
	void ReleasePythonObject();
	PyObject *GetPythonObject() {return m_PythonObject;}
private:
	PyObject *m_PythonObject;
};

enum EnumExceptionHandlerAction {
	EHA_PRINT_ERROR,
	EHA_DISPLAY_DIALOG
};

typedef void (*ExceptionHandlerFunc)(int action, const TCHAR *context, const TCHAR *extraTitleMsg);

PYW_EXPORT void ExceptionHandler(int action, const TCHAR *context=NULL, const TCHAR *extraTitleMsg=NULL);
PYW_EXPORT ExceptionHandlerFunc SetExceptionHandler(ExceptionHandlerFunc handler);

// A helper class for calling "virtual methods" - ie, given a C++ object
// call a Python method of that name on the attached Python object.

// The type of error handling we want...
enum EnumVirtualErrorHandling {
	VEH_PRINT_ERROR,
	VEH_DISPLAY_DIALOG
};

class PYW_EXPORT CVirtualHelper
{
public:
	CVirtualHelper(const char *iname, void *iassoc, EnumVirtualErrorHandling veh = VEH_PRINT_ERROR);
	~CVirtualHelper();

	BOOL HaveHandler() {return handler!=NULL;}
	// All the "call" functions return FALSE if the call failed, or no handler exists.
	BOOL call();
	BOOL call(int);
	BOOL call(DWORD, DWORD);
	BOOL call(BOOL, BOOL);
	BOOL call(int v1, DWORD v2) {return call((DWORD)v1, v2);}
	BOOL call(int, int, int);
	BOOL call(long);
	BOOL call(UINT_PTR);
	BOOL call(const char *);
	BOOL call(const WCHAR *);
	BOOL call(const char *, int);
	BOOL call(const WCHAR *val, int ival);
	BOOL call(CDC *, CPrintInfo *);
	BOOL call(CPrintInfo *);
	BOOL call(CDC *);
	BOOL call(CDocument *);
	BOOL call(CWnd *);
	BOOL call(CWnd *, int);
	BOOL call(CWnd *, int, int);
	BOOL call(BOOL, CWnd *, CWnd *);
	BOOL call(LPCREATESTRUCT);
	BOOL call(LPCREATESTRUCT, PyObject *);
	BOOL call(PyObject *);
	BOOL call(PyObject *, PyObject *);
	BOOL call(PyObject *, PyObject *, int);
	BOOL call(CView *pWnd, PyObject *ob);
	BOOL call(CDC *pDC, CWnd *pWnd, int i);
	BOOL call(const MSG *);
	BOOL call(WPARAM, LPARAM);
	BOOL call(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo);
	BOOL call_args(PyObject *arglst);
	// All the retval functions will ASSERT if the call failed!
	BOOL retval( int &ret );
	BOOL retval( long &ret );
	BOOL retval( PyObject* &ret );
	BOOL retval( CREATESTRUCT &cs );
	// Note the lack of 'char *' or 'WCHAR *' support - this makes it
	// too hard for memory management when converting between strings and
	// unicode. Use the CString one instead.
	BOOL retval( CString &ret );
	BOOL retval( MSG *msg);
	BOOL retval( HANDLE &ret );
	BOOL retnone();
	PyObject *GetHandler();
private:
	BOOL do_call(PyObject *args);
	PyObject *handler;
	PyObject *retVal;
	PyObject *py_ob;
	CString csHandlerName;
	EnumVirtualErrorHandling vehErrorHandling;
};

// These error functions are designed to be used "asynchronously" - ie, where
// there is no Python call to return NULL from.  These force an exception to
// be printed.
PYW_EXPORT PyObject *Python_do_callback(PyObject *themeth, PyObject *thearglst);
PYW_EXPORT int Python_callback(PyObject *);
PYW_EXPORT int Python_callback(PyObject *, int);
PYW_EXPORT int Python_callback(PyObject *, WPARAM);
PYW_EXPORT int Python_callback(PyObject *, LPARAM);
PYW_EXPORT int Python_callback(PyObject *, int, int);
PYW_EXPORT int Python_callback(PyObject *, const MSG *);
PYW_EXPORT int Python_callback(PyObject *method, PyObject *object);
int Python_run_command_with_log(const char *command);
PYW_EXPORT BOOL Python_check_message(const MSG *pMsg);	// TRUE if fully processed.
PYW_EXPORT BOOL Python_check_key_message(const MSG *pMsg);	// TRUE if fully processed.
PYW_EXPORT BOOL Python_OnCmdMsg(CCmdTarget *, UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO*pHandlerInfo );// TRUE if fully processed.
PYW_EXPORT BOOL Python_OnNotify (CWnd *pFrom, WPARAM, LPARAM lParam, LRESULT *pResult);

// Conversion routines
PYW_EXPORT BOOL CreateStructFromPyObject(LPCREATESTRUCT lpcs, PyObject *ob, const char *fnName = NULL, BOOL bFromTuple = FALSE);
PYW_EXPORT PyObject *PyObjectFromCreateStruct(LPCREATESTRUCT lpcs);

PYW_EXPORT BOOL DictToLogFont(PyObject *font_props, LOGFONT *pLF);
PYW_EXPORT PyObject *LogFontToDict(const LOGFONT &lf);

PYW_EXPORT BOOL ParseCharFormatTuple( PyObject *args, CHARFORMAT *pFmt);
PYW_EXPORT PyObject *MakeCharFormatTuple(CHARFORMAT *pFmt);
PYW_EXPORT BOOL ParseParaFormatTuple( PyObject *args, PARAFORMAT *pFmt);
PYW_EXPORT PyObject *MakeParaFormatTuple(PARAFORMAT *pFmt);

PYW_EXPORT PyObject *PyWinObject_FromLV_ITEM(LV_ITEM *pItem);
PYW_EXPORT BOOL PyWinObject_AsLV_ITEM( PyObject *args, LV_ITEM *pItem);
PYW_EXPORT void PyWinObject_FreeLV_ITEM(LV_ITEM *pItem);

PYW_EXPORT PyObject *PyWinObject_FromLV_COLUMN(LV_COLUMN *pCol);
PYW_EXPORT BOOL PyWinObject_AsLV_COLUMN( PyObject *args, LV_COLUMN *pCol);
PYW_EXPORT void PyWinObject_FreeLV_COLUMN(LV_COLUMN *pCol);

PYW_EXPORT BOOL PyWinObject_AsTV_ITEM( PyObject *args, TV_ITEM *pItem);
PYW_EXPORT PyObject *PyWinObject_FromTV_ITEM(TV_ITEM *pItem);
PYW_EXPORT void PyWinObject_FreeTV_ITEM(TV_ITEM *pItem);

PyObject *PyWin_GetPythonObjectFromLong(LONG_PTR val);

PYW_EXPORT PyObject *PyWinObject_FromRECT(RECT *p, bool bTakeCopy);
PYW_EXPORT PyObject *PyWinObject_FromRECT(const RECT &r);

PYW_EXPORT PyObject *PyWinObject_FromCWnd(CWnd *);

PYW_EXPORT void Python_do_exchange(CDialog *pDlg, CDataExchange *pDX);

// call when an external object dies.
PYW_EXPORT void Python_delete_assoc( void *ob );

PYW_EXPORT void Python_addpath( const TCHAR *paths );

BOOL AFXAPI PyAfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);
extern BOOL PASCAL AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
#endif // __filename_h__

