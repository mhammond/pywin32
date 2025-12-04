/* File : win32process.i */

%module win32process // An interface to the win32 Process and Thread API's

%{
#define PY_SSIZE_T_CLEAN  // may inevitably be defined by swig_lib/python/python.swg already
#include "process.h"
#include "windows.h"
#include "Psapi.h"
#include "PyWinTypes.h"
%}

%include "typemaps.i"
%include "pywin32.i"

%{
#include "structmember.h"

#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);

typedef BOOL (WINAPI *EnumProcessesfunc)(DWORD *, DWORD, DWORD *);
static EnumProcessesfunc pfnEnumProcesses = NULL;
typedef BOOL (WINAPI *EnumProcessModulesfunc)(HANDLE, HMODULE *, DWORD, LPDWORD);
static EnumProcessModulesfunc pfnEnumProcessModules = NULL;
typedef DWORD (WINAPI *GetModuleFileNameExfunc)(HANDLE, HMODULE, WCHAR *, DWORD);
typedef BOOL (WINAPI *EnumProcessModulesExfunc)(HANDLE, HMODULE*, DWORD, LPDWORD, DWORD);
static EnumProcessModulesExfunc pfnEnumProcessModulesEx = NULL;
static GetModuleFileNameExfunc pfnGetModuleFileNameEx = NULL;
typedef DWORD (WINAPI *GetProcessIdfunc)(HANDLE);
static GetProcessIdfunc pfnGetProcessId = NULL;

typedef BOOL (WINAPI *GetProcessMemoryInfofunc)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD);
static GetProcessMemoryInfofunc pfnGetProcessMemoryInfo=NULL;
typedef BOOL (WINAPI *GetProcessTimesfunc)(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME, LPFILETIME);
static GetProcessTimesfunc pfnGetProcessTimes = NULL;
typedef BOOL (WINAPI *GetProcessIoCountersfunc)(HANDLE, PIO_COUNTERS);
static GetProcessIoCountersfunc pfnGetProcessIoCounters = NULL;
typedef BOOL (WINAPI *GetProcessShutdownParametersfunc)(LPDWORD, LPDWORD);
static GetProcessShutdownParametersfunc pfnGetProcessShutdownParameters = NULL;
typedef BOOL (WINAPI *SetProcessShutdownParametersfunc)(DWORD, DWORD);
static SetProcessShutdownParametersfunc pfnSetProcessShutdownParameters = NULL;
typedef BOOL (WINAPI *GetProcessWorkingSetSizefunc)(HANDLE, PSIZE_T, PSIZE_T);
static GetProcessWorkingSetSizefunc pfnGetProcessWorkingSetSize = NULL;
typedef BOOL (WINAPI *SetProcessWorkingSetSizefunc)(HANDLE, SIZE_T, SIZE_T);
static SetProcessWorkingSetSizefunc pfnSetProcessWorkingSetSize = NULL;

typedef HWINSTA (WINAPI *GetProcessWindowStationfunc)(void);
static GetProcessWindowStationfunc pfnGetProcessWindowStation = NULL;
typedef DWORD (WINAPI *GetGuiResourcesfunc)(HANDLE,DWORD);
static GetGuiResourcesfunc pfnGetGuiResources = NULL;
typedef BOOL (WINAPI *GetProcessPriorityBoostfunc)(HANDLE,PBOOL);
static GetProcessPriorityBoostfunc pfnGetProcessPriorityBoost = NULL;
typedef BOOL (WINAPI *SetProcessPriorityBoostfunc)(HANDLE,BOOL);
static SetProcessPriorityBoostfunc pfnSetProcessPriorityBoost = NULL;
typedef BOOL (WINAPI *GetThreadPriorityBoostfunc)(HANDLE,PBOOL);
static GetThreadPriorityBoostfunc pfnGetThreadPriorityBoost = NULL;
typedef BOOL (WINAPI *SetThreadPriorityBoostfunc)(HANDLE,BOOL);
static SetThreadPriorityBoostfunc pfnSetThreadPriorityBoost = NULL;
typedef BOOL (WINAPI *GetThreadIOPendingFlagfunc)(HANDLE,PBOOL);
static GetThreadIOPendingFlagfunc pfnGetThreadIOPendingFlag = NULL;
typedef BOOL (WINAPI *GetThreadTimesfunc)(HANDLE,LPFILETIME,LPFILETIME,LPFILETIME,LPFILETIME);
static GetThreadTimesfunc pfnGetThreadTimes =  NULL;
typedef	HANDLE (WINAPI *CreateRemoteThreadfunc)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
static CreateRemoteThreadfunc pfnCreateRemoteThread=NULL;
typedef DWORD (WINAPI *SetThreadIdealProcessorfunc)(HANDLE, DWORD);
static SetThreadIdealProcessorfunc pfnSetThreadIdealProcessor = NULL;
typedef DWORD (WINAPI *SetProcessAffinityMaskfunc)(HANDLE, DWORD_PTR);
static SetProcessAffinityMaskfunc pfnSetProcessAffinityMask = NULL;
typedef BOOL (WINAPI *IsWow64Processfunc)(HANDLE, PBOOL);
static IsWow64Processfunc pfnIsWow64Process = NULL;

// Support for a STARTUPINFO object.
class PySTARTUPINFO : public PyObject
{
public:
	STARTUPINFO *GetSI() {return &m_startupinfo;}

	PySTARTUPINFO(void);
	PySTARTUPINFO(const STARTUPINFO *pSI);
	~PySTARTUPINFO();

	/* Python support */

	static void deallocFunc(PyObject *ob);

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];

protected:
	STARTUPINFO m_startupinfo;
	PyObject *m_obStdIn, *m_obStdOut, *m_obStdErr;
	PyObject *m_obDesktop, *m_obTitle;
};
#define PySTARTUPINFO_Check(ob)	((ob)->ob_type == &PySTARTUPINFOType)

// @object PySTARTUPINFO|A Python object, representing an STARTUPINFO structure
// @comm Typically you create a PySTARTUPINFO (via <om win32process.STARTUPINFO>) object, and set its properties.
// The object can then be passed to any function which takes an STARTUPINFO object.
PyTypeObject PySTARTUPINFOType =
{
	PYWIN_OBJECT_HEAD
	"PySTARTUPINFO",
	sizeof(PySTARTUPINFO),
	0,
	PySTARTUPINFO::deallocFunc,		/* tp_dealloc */
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
	PySTARTUPINFO::getattro,		/* tp_getattr */
	PySTARTUPINFO::setattro,		/* tp_setattr */
	0,						/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	"A Python object, representing a STARTUPINFO structure",		/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	0,						/* tp_methods */
	PySTARTUPINFO::members,		/* tp_members */
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

#define OFF(e) offsetof(PySTARTUPINFO, e)

/*static*/ struct PyMemberDef PySTARTUPINFO::members[] = {
	{"dwX",              T_INT,  OFF(m_startupinfo.dwX)}, // @prop integer|dwX|Specifies the x offset, in pixels, of the upper left corner of a window if a new window is created. The offset is from the upper left corner of the screen.
	{"dwY",              T_INT,  OFF(m_startupinfo.dwY)}, // @prop integer|dwY|Specifies the y offset, in pixels, of the upper left corner of a window if a new window is created. The offset is from the upper left corner of the screen.
	{"dwXSize",          T_INT,  OFF(m_startupinfo.dwXSize)}, // @prop integer|dwXSize|Specifies the width, in pixels, of the window if a new window is created.
	{"dwYSize",          T_INT,  OFF(m_startupinfo.dwYSize)}, // @prop integer|dwYSize|Specifies the height, in pixels, of the window if a new window is created.
	{"dwXCountChars",    T_INT,  OFF(m_startupinfo.dwXCountChars)}, // @prop integer|dwXCountChars|For console processes, if a new console window is created, specifies the screen buffer width in character columns. This value is ignored in a GUI process.
	{"dwYCountChars",    T_INT,  OFF(m_startupinfo.dwYCountChars)}, // @prop integer|dwYCountChars|For console processes, if a new console window is created, specifies the screen buffer height in character rows.
	{"dwFillAttribute",  T_INT,  OFF(m_startupinfo.dwFillAttribute)}, // @prop integer|dwFillAttribute|Specifies the initial text and background colors if a new console window is created in a console application. These values are ignored in GUI applications
	{"dwFlags",          T_INT,  OFF(m_startupinfo.dwFlags)}, // @prop integer|dwFlags|This is a bit field that determines whether certain STARTUPINFO attributes are used when the process creates a window. To use many of the additional attributes, you typically must set the appropriate mask in this attribute, and also set the attributes themselves. Any combination of the win32con.STARTF_* flags can be specified.
	{"wShowWindow",	     T_USHORT,  OFF(m_startupinfo.wShowWindow)},//@prop integer|wShowWindow|Can be any of the SW_ constants defined in win32con. For GUI processes, this specifies the default value the first time ShowWindow is called.
	{NULL}
};


PySTARTUPINFO::PySTARTUPINFO()
{
	ob_type = &PySTARTUPINFOType;
	_Py_NewReference(this);
	memset(&m_startupinfo, 0, sizeof(m_startupinfo));
	m_startupinfo.cb = sizeof(m_startupinfo);
	m_obStdIn = m_obStdOut = m_obStdErr = NULL;
	m_obDesktop = m_obTitle = NULL;
}

PySTARTUPINFO::PySTARTUPINFO(const STARTUPINFO *pSI)
{
	ob_type = &PySTARTUPINFOType;
	_Py_NewReference(this);
	memcpy(&m_startupinfo, pSI, sizeof(m_startupinfo));
	m_obStdIn = m_obStdOut = m_obStdErr = NULL;
	if (pSI->lpDesktop)
		m_startupinfo.lpDesktop = PyWin_CopyString(pSI->lpDesktop);
	if (pSI->lpTitle)
		m_startupinfo.lpTitle = PyWin_CopyString(pSI->lpTitle);
}

PySTARTUPINFO::~PySTARTUPINFO(void)
{
	Py_XDECREF(m_obStdIn);
	Py_XDECREF(m_obStdOut);
	Py_XDECREF(m_obStdErr);
	PyWinObject_FreeTCHAR(m_startupinfo.lpDesktop);
	PyWinObject_FreeTCHAR(m_startupinfo.lpTitle);
}

PyObject *gethandle(PyObject *obHandle, HANDLE h)
{
	if (obHandle) {
		Py_INCREF(obHandle);
		return obHandle;
	}
	return PyWinLong_FromHANDLE(h);
}

PyObject *PySTARTUPINFO::getattro(PyObject *self, PyObject *obname)
{
	PySTARTUPINFO *pO = (PySTARTUPINFO *)self;
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name == NULL)
		return NULL;
	// @prop integer/<o PyHANDLE>|hStdInput|
	// @prop integer/<o PyHANDLE>|hStdOutput|
	// @prop integer/<o PyHANDLE>|hStdError|
	if (strcmp("hStdInput", name)==0)
		return gethandle(pO->m_obStdIn, pO->m_startupinfo.hStdInput);
	if (strcmp("hStdOutput", name)==0)
		return gethandle(pO->m_obStdOut, pO->m_startupinfo.hStdOutput);
	if (strcmp("hStdError", name)==0)
		return gethandle(pO->m_obStdErr, pO->m_startupinfo.hStdError);
	// @prop string/None|lpDesktop|
	if (strcmp("lpDesktop", name)==0)
		return PyWinObject_FromTCHAR(pO->m_startupinfo.lpDesktop);
	// @prop string/None|lpTitle|
	if (strcmp("lpTitle", name)==0)
		return PyWinObject_FromTCHAR(pO->m_startupinfo.lpTitle);
	return PyObject_GenericGetAttr(self, obname);
}

int sethandle(PyObject **pobHandle, HANDLE *ph, PyObject *v)
{
	HANDLE htmp;
	if (!PyWinObject_AsHANDLE(v, &htmp))
		return -1;
	*ph=htmp;
	Py_XDECREF(*pobHandle);
	if (PyHANDLE_Check(v)){
		*pobHandle = v;
		Py_INCREF(v);
		}
	else
		*pobHandle = NULL;
	return 0;
}

int PySTARTUPINFO::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete STARTUPINFO attributes");
		return -1;
	}
	PySTARTUPINFO *pO = (PySTARTUPINFO *)self;
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name == NULL)
		return -1;
	if (strcmp("hStdInput", name)==0)
		return sethandle( &pO->m_obStdIn, &pO->m_startupinfo.hStdInput, v);

	if (strcmp("hStdOutput", name)==0)
		return sethandle( &pO->m_obStdOut, &pO->m_startupinfo.hStdOutput, v);

	if (strcmp("hStdError", name)==0)
		return sethandle( &pO->m_obStdErr, &pO->m_startupinfo.hStdError, v);

	if (strcmp("lpDesktop", name)==0) {
		TCHAR *val;
		if (!PyWinObject_AsTCHAR(v, &val, TRUE))
			return -1;
		PyWinObject_FreeTCHAR(pO->m_startupinfo.lpDesktop);
		pO->m_startupinfo.lpDesktop = val;
		return 0;
		}

	if (strcmp("lpTitle", name)==0) {
		TCHAR *val;
		if (!PyWinObject_AsTCHAR(v, &val, TRUE))
			return -1;
		PyWinObject_FreeTCHAR(pO->m_startupinfo.lpTitle);
		pO->m_startupinfo.lpTitle=val;
		return 0;
		}
	return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PySTARTUPINFO::deallocFunc(PyObject *ob)
{
	delete (PySTARTUPINFO *)ob;
}

// A converter.
BOOL PyWinObject_AsSTARTUPINFO(PyObject *ob, STARTUPINFO **ppSI, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSI = NULL;
	} else if (!PySTARTUPINFO_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PySTARTUPINFO object");
		return FALSE;
	} else {
		*ppSI = ((PySTARTUPINFO *)ob)->GetSI();
	}
	return TRUE;
}

PyObject *PyWinObject_FromSTARTUPINFO(const STARTUPINFO *pSI)
{
	if (pSI==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *ret = new PySTARTUPINFO(pSI);
	if(ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "PySTARTUPINFO");
	return ret;
}

// @pyswig <o PySTARTUPINFO>|STARTUPINFO|Creates a new STARTUPINFO object.
static PyObject *mySTARTUPINFO(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":STARTUPINFO"))
		return NULL;
	return new PySTARTUPINFO();
}
%}
%native (STARTUPINFO) mySTARTUPINFO;


%typemap(python,in) STARTUPINFO *
{
	if (!PyWinObject_AsSTARTUPINFO($source, &$target, FALSE))
		return NULL;
}

%typemap(python,argout) STARTUPINFO *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromSTARTUPINFO($source);
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
%typemap(python,ignore) STARTUPINFO *OUTPUT(STARTUPINFO temp)
{
  $target = &temp;
}

%{
class PythonThreadData
{
public:
	PythonThreadData(PyObject *obFunc, PyObject *args)
		{m_obFunc = obFunc;Py_INCREF(obFunc);m_obArgs=args;Py_INCREF(args);}
	~PythonThreadData() {Py_DECREF(m_obFunc); Py_DECREF(m_obArgs);}
	PyObject *m_obFunc;
	PyObject *m_obArgs;
};

unsigned __stdcall ThreadEntryPoint( void *arg )
{
	CEnterLeavePython _celp;
	PythonThreadData *ptd = (PythonThreadData *)arg;
	PyObject *pyrc = PyObject_CallObject(ptd->m_obFunc, ptd->m_obArgs);
	delete ptd;
	if (pyrc==NULL) {
		fprintf(stderr, "Unhandled exception in beginthreadex created thread:\n");
		PyErr_Print();
		return -1;
	}
	int rc = 0;
	if (PyLong_Check(pyrc))
		rc = PyLong_AsLong(pyrc);
	Py_XDECREF(pyrc);
	return rc;
}

// @pyswig <o PyHANDLE>, int|beginthreadex|Creates a new thread
static PyObject *mybeginthreadex(PyObject *self, PyObject *args)
{
	PyObject *obFunc, *obArgs, *obSA;
	unsigned stackSize;
	unsigned long flags;
	if (!PyArg_ParseTuple(args, "OIOOk:beginthreadex",
		&obSA, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None
		&stackSize, // @pyparm int|stackSize||Stack size for the new thread, or zero for the default size.
		&obFunc, // @pyparm function|entryPoint||The thread function.
		&obArgs, // @pyparm tuple|args||Args passed to the function.
		&flags)) // @pyparm int|flags||Can be CREATE_SUSPENDED so thread doesn't run immediately
		return NULL;
	if (!PyCallable_Check(obFunc)) {
		PyErr_SetString(PyExc_TypeError, "function must be callable");
		return NULL;
	}
	if (!PyTuple_Check(obArgs)) {
		PyErr_SetString(PyExc_TypeError, "args must be a tuple");
		return NULL;
	}
	SECURITY_ATTRIBUTES *pSA;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES( obSA, &pSA, TRUE ))
		return NULL;

	PythonThreadData *ptd = new PythonThreadData(obFunc, obArgs);
	ULONG_PTR handle;
	unsigned tid;
	handle = _beginthreadex((void *)pSA, stackSize, ThreadEntryPoint, ptd, flags, &tid);
	if (handle==-1 || handle==NULL){
		delete ptd;
		return PyErr_SetFromErrno(PyExc_RuntimeError);
		}
	// @rdesc The result is a tuple of the thread handle and thread ID.
	return Py_BuildValue("Ni", PyWinObject_FromHANDLE((HANDLE)handle), tid);
}
%}
%native (beginthreadex) mybeginthreadex;

%{
// @pyswig <o PyHANDLE>, int|CreateRemoteThread|creates a thread that runs in
// the virtual address space of another process.
static PyObject *myCreateRemoteThread(PyObject *self, PyObject *args)
{
	CHECK_PFN(CreateRemoteThread);
	static char *fmt="OOnOOk:CreateRemoteThread";
	PyObject *obhprocess, *obFunc, *obParameter, *obSA;
	SIZE_T stackSize;
	DWORD flags;
	HANDLE hprocess;
	LPTHREAD_START_ROUTINE Func;
	VOID *Parameter;
	if (!PyArg_ParseTuple(args, fmt,
		&obhprocess, // @pyparm <o PyHANDLE>|hprocess||The handle to the remote process.
		&obSA, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None
		&stackSize, // @pyparm int|stackSize||Stack size for the new thread, or zero for the default size.
		&obFunc, // @pyparm function|entryPoint||The thread function's address.
		&obParameter, // @pyparm int|Parameter||Arg passed to the function in the form of a void pointer
		&flags)) // @pyparm int|flags||
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obFunc, (void **)&Func))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obParameter, &Parameter))
		return NULL;
	SECURITY_ATTRIBUTES *pSA;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES( obSA, &pSA, TRUE ))
		return NULL;

	HANDLE handle;
	DWORD tid;
	handle = (*pfnCreateRemoteThread)(hprocess, pSA, stackSize,
	                                  Func, Parameter,
	                                  flags, &tid);
	if (handle==INVALID_HANDLE_VALUE || handle==NULL) {
		return PyWin_SetAPIError("CreateRemoteThread");
	}
	// @rdesc The result is a tuple of the thread handle and thread ID.
	return Py_BuildValue("Ni", PyWinObject_FromHANDLE(handle), tid);
}
%}
%native (CreateRemoteThread) myCreateRemoteThread;


// Won't expose ExitThread!!!  May leak all sorts of things!

%{

static BOOL CreateEnvironmentString(PyObject *env, LPVOID *ppRet, BOOL *pRetIsUnicode)
{
	BOOL ok = FALSE;
	BOOL bIsUnicode = FALSE;
	if (env==Py_None) {
		*pRetIsUnicode = FALSE;
		*ppRet = NULL;
		return TRUE;
	}
	// First loop counting the size of the environment.
	if (!PyMapping_Check(env)) {
		PyErr_SetString(PyExc_TypeError, "environment parameter must be a dictionary object of strings or unicode objects.");
		return FALSE;
	}
	int i;
	size_t bufLen = 0;
	PyObject *keys = NULL, *vals = NULL;
	Py_ssize_t envLength = PyMapping_Length(env);
	LPVOID result = NULL;
	WCHAR *pUCur;
	char *pACur;
	TmpWCHAR tw;

	keys = PyMapping_Keys(env);
	vals = PyMapping_Values(env);
	if (!keys || !vals)
		goto done;

	for (i=0;i<envLength;i++) {
		PyObject *key = PyList_GetItem(keys, i); // no reference
		PyObject *val = PyList_GetItem(vals, i); // no ref.
		if (i==0) {
			if (PyBytes_Check(key)) {
				bIsUnicode = FALSE;
				bufLen += PyBytes_Size(key) + 1;
			} else if (PyUnicode_Check(key)) {
				bIsUnicode = TRUE;
				tw = key;  if (!tw) goto done;
				bufLen += wcslen(tw) + 1;  // PyUnicode_GetLength() and tw.length (incl \0 's) may be different
			} else {
				PyErr_SetString(PyExc_TypeError, "dictionary must have keys and values as strings or unicode objects.");
				goto done;
			}
		} else {
			if (bIsUnicode) {
				if (!PyUnicode_Check(key)) {
					PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
					goto done;
				}
				tw = key;  if (!tw) goto done;
				bufLen += wcslen(tw) + 1;
			}
			else {
				if (!PyBytes_Check(key)) {
					PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
					goto done;

				}
				bufLen += PyBytes_Size(key) + 1;
			}
		}
		if (bIsUnicode) {
			if (!PyUnicode_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				goto done;
			}
			tw = val;  if (!tw) goto done;
			bufLen += wcslen(tw) + 2;  // For the '=' and '\0'
		}
		else {
			if (!PyBytes_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				goto done;
			}
			bufLen += PyBytes_Size(val) + 2; // For the '=' and '\0'
		}
	}
	result = (LPVOID)malloc( (bIsUnicode ? sizeof(WCHAR) : sizeof(char)) * (bufLen + 1) );
	if (!result) {
		PyErr_SetString(PyExc_MemoryError, "allocating environment buffer");
		goto done;
	}
	pUCur = (WCHAR *)result;
	pACur = (char *)result;
	// Now loop filling it!
	for (i=0;i<envLength;i++) {
		PyObject *key = PyList_GetItem(keys, i);
		PyObject *val = PyList_GetItem(vals, i);
		if (bIsUnicode) {
			WCHAR *pTemp;
			if (!PyWinObject_AsWCHAR(key, &pTemp))
				goto done;
			wcscpy(pUCur, pTemp);
			pUCur += wcslen(pTemp);
			PyWinObject_FreeWCHAR(pTemp);
		} else {
			char *pTemp = PyBytes_AsString(key);
			strcpy(pACur, pTemp);
			pACur += strlen(pTemp);
		}
		if (bIsUnicode)
			*pUCur++ = L'=';
		else
			*pACur++ = '=';
		if (bIsUnicode) {
			WCHAR *pTemp;
			if (!PyWinObject_AsWCHAR(val, &pTemp))
				goto done;
			wcscpy(pUCur, pTemp);
			pUCur += wcslen(pTemp);
			PyWinObject_FreeWCHAR(pTemp);
		} else {
			char *pTemp = PyBytes_AsString(val);
			strcpy(pACur, pTemp);
			pACur += strlen(pTemp);
		}
		if (bIsUnicode)
			*pUCur++ = L'\0';
		else
			*pACur++ = '\0';
	}
	if (bIsUnicode) {
		*pUCur++ = L'\0';
//		assert(((unsigned)(pUCur - (WCHAR *)result))==bufLen);
	} else {
		*pACur++ = '\0';
//		assert(((unsigned)(pACur - (char *)result))==bufLen);
	}
	*pRetIsUnicode = bIsUnicode;
	*ppRet = result;
	ok = TRUE;
done:
	if (result && !ok) // failure after allocing buffer.
		free(result);
	Py_XDECREF(keys);
	Py_XDECREF(vals);
	return ok;
}

PyObject *MyCreateProcess(
	TCHAR *appName,
	TCHAR *cmdLine,
	SECURITY_ATTRIBUTES *psaP,
	SECURITY_ATTRIBUTES *psaT,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	PyObject *environment,
	TCHAR *directory,
	STARTUPINFO *si)
{
	if(!appName && !cmdLine) {
		PyErr_SetString(PyExc_TypeError, "The command line and application parameters can not both be None");
		return NULL;
	}
	PROCESS_INFORMATION pi;
	// Convert the environment.
	LPVOID pEnv;
	BOOL bEnvIsUnicode;
	if (!CreateEnvironmentString(environment, &pEnv, &bEnvIsUnicode))
		return NULL;

	if (bEnvIsUnicode)
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = CreateProcess(appName, cmdLine, psaP, psaT, bInheritHandles, dwCreationFlags, pEnv, directory, si, &pi);
    Py_END_ALLOW_THREADS

	free(pEnv);

	if (!ok)
		return PyWin_SetAPIError("CreateProcess");

	PyObject *ret = PyTuple_New(4);
	PyTuple_SET_ITEM(ret, 0, PyWinObject_FromHANDLE(pi.hProcess));
	PyTuple_SET_ITEM(ret, 1, PyWinObject_FromHANDLE(pi.hThread));
	PyTuple_SET_ITEM(ret, 2, PyLong_FromLong(pi.dwProcessId));
	PyTuple_SET_ITEM(ret, 3, PyLong_FromLong(pi.dwThreadId));
	return ret;
}
%}

// @pyswig <o PyHANDLE>, <o PyHANDLE>, int, int|CreateProcess|Creates a new process and its primary thread. The new process executes the specified executable file.
// @comm The result is a tuple of (hProcess, hThread, dwProcessId, dwThreadId)
%name(CreateProcess)
PyObject *MyCreateProcess(
	TCHAR *INPUT_NULLOK,  // @pyparm string|appName||name of executable module, or None
	TCHAR *INPUT_NULLOK,  // @pyparm string|commandLine||command line string, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|processAttributes||process security attributes, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|threadAttributes||thread security attributes, or None
	BOOL bInheritHandles, // @pyparm int|bInheritHandles||handle inheritance flag
	DWORD dwCreationFlags, // @pyparm int|dwCreationFlags||creation flags.  May be a combination of the following values from the win32con module:
			// @flagh Value|Meaning
			// @flag CREATE_BREAKAWAY_FROM_JOB|The child processes of a process associated with a job are not associated with the job.
			// If the calling process is not associated with a job, this flag has no effect. If the calling process is associated with a job, the job must set the JOB_OBJECT_LIMIT_BREAKAWAY_OK limit or CreateProcess will fail.

			// @flag CREATE_DEFAULT_ERROR_MODE|The new process does not inherit the error mode of the calling process. Instead, CreateProcess gives the new process the current default error mode. An application sets the current default error mode by calling SetErrorMode.
			// This flag is particularly useful for multi-threaded shell applications that run with hard errors disabled.
			// The default behavior for CreateProcess is for the new process to inherit the error mode of the caller. Setting this flag changes that default behavior.

			// @flag CREATE_FORCE_DOS|Windows NT/2000: This flag is valid only when starting a 16-bit bound application. If set, the system will force the application to run as an MS-DOS-based application rather than as an OS/2-based application.
			// @flag CREATE_NEW_CONSOLE|The new process has a new console, instead of inheriting the parent's console. This flag cannot be used with the DETACHED_PROCESS flag.
			// @flag CREATE_NEW_PROCESS_GROUP|The new process is the root process of a new process group. The process group includes all processes that are descendants of this root process. The process identifier of the new process group is the same as the process identifier, which is returned in the lpProcessInformation parameter. Process groups are used by the GenerateConsoleCtrlEvent function to enable sending a CTRL+C or CTRL+BREAK signal to a group of console processes.
			// @flag CREATE_NO_WINDOW|Windows NT/2000: This flag is valid only when starting a console application. If set, the console application is run without a console window.
			// @flag CREATE_SEPARATE_WOW_VDM|Windows NT/2000: This flag is valid only when starting a 16-bit Windows-based application. If set, the new process runs in a private Virtual DOS Machine (VDM). By default, all 16-bit Windows-based applications run as threads in a single, shared VDM. The advantage of running separately is that a crash only terminates the single VDM; any other programs running in distinct VDMs continue to function normally. Also, 16-bit Windows-based applications that are run in separate VDMs have separate input queues. That means that if one application stops responding momentarily, applications in separate VDMs continue to receive input. The disadvantage of running separately is that it takes significantly more memory to do so. You should use this flag only if the user requests that 16-bit applications should run in them own VDM.
			// @flag CREATE_SHARED_WOW_VDM|Windows NT/2000: The flag is valid only when starting a 16-bit Windows-based application. If the DefaultSeparateVDM switch in the Windows section of WIN.INI is TRUE, this flag causes the CreateProcess function to override the switch and run the new process in the shared Virtual DOS Machine.
			// @flag CREATE_SUSPENDED|The primary thread of the new process is created in a suspended state, and does not run until the ResumeThread function is called.
			// @flag CREATE_UNICODE_ENVIRONMENT|Indicates the format of the lpEnvironment parameter. If this flag is set, the environment block pointed to by lpEnvironment uses Unicode characters. Otherwise, the environment block uses ANSI characters.
			// @flag DEBUG_PROCESS|If this flag is set, the calling process is treated as a debugger, and the new process is debugged. The system notifies the debugger of all debug events that occur in the process being debugged.
			// If you create a process with this flag set, only the calling thread (the thread that called CreateProcess) can call the WaitForDebugEvent function.
			// @flag DEBUG_ONLY_THIS_PROCESS|If this flag is not set and the calling process is being debugged, the new process becomes another process being debugged by the calling process's debugger. If the calling process is not a process being debugged, no debugging-related actions occur.
			// @flag DETACHED_PROCESS|For console processes, the new process does not have access to the console of the parent process. The new process can call the AllocConsole function at a later time to create a new console. This flag cannot be used with the CREATE_NEW_CONSOLE flag.


			// @flag ABOVE_NORMAL_PRIORITY_CLASS|Indicates a process that has priority higher than NORMAL_PRIORITY_CLASS but lower than HIGH_PRIORITY_CLASS.
			// @flag BELOW_NORMAL_PRIORITY_CLASS|Indicates a process that has priority higher than IDLE_PRIORITY_CLASS but lower than NORMAL_PRIORITY_CLASS.
			// @flag HIGH_PRIORITY_CLASS|Indicates a process that performs time-critical tasks. The threads of a high-priority class process preempt the threads of normal-priority or idle-priority class processes. An example is the Task List, which must respond quickly when called by the user, regardless of the load on the system. Use extreme care when using the high-priority class, because a CPU-bound application with a high-priority class can use nearly all available cycles.
			// @flag IDLE_PRIORITY_CLASS|Indicates a process whose threads run only when the system is idle and are preempted by the threads of any process running in a higher priority class. An example is a screen saver. The idle priority class is inherited by child processes.
			// @flag NORMAL_PRIORITY_CLASS|Indicates a normal process with no special scheduling needs.
			// @flag REALTIME_PRIORITY_CLASS|Indicates a process that has the highest possible priority. The threads of a real-time priority class process preempt the threads of all other processes, including operating system processes performing important tasks. For example, a real-time process that executes for more than a very brief interval can cause disk caches not to flush or cause the mouse to be unresponsive.


	PyObject *env, // @pyparm dictionary/None|newEnvironment||A dictionary of string or Unicode pairs to define the environment for the process, or None to inherit the current environment.
	TCHAR *INPUT_NULLOK, // @pyparm string|currentDirectory||current directory name, or None
	STARTUPINFO *lpStartupInfo // @pyparm <o PySTARTUPINFO>|startupinfo||a STARTUPINFO object that specifies how the main window for the new process should appear.

);

%{
PyObject *MyCreateProcessAsUser(
	HANDLE h,
	TCHAR *appName,
	TCHAR *cmdLine,
	SECURITY_ATTRIBUTES *psaP,
	SECURITY_ATTRIBUTES *psaT,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	PyObject *environment,
	TCHAR *directory,
	STARTUPINFO *si)
{
	if(!appName && !cmdLine) {
		PyErr_SetString(PyExc_TypeError, "The command line and application parameters can not both be None");
		return NULL;
	}
	PROCESS_INFORMATION pi;
	// Convert the environment.
	LPVOID pEnv;
	BOOL bEnvIsUnicode;
	if (!CreateEnvironmentString(environment, &pEnv, &bEnvIsUnicode))
		return NULL;

	if (bEnvIsUnicode)
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = CreateProcessAsUser(h, appName, cmdLine, psaP, psaT, bInheritHandles, dwCreationFlags, pEnv, directory, si, &pi);
	Py_END_ALLOW_THREADS

	free(pEnv);

	if (!ok)
		return PyWin_SetAPIError("CreateProcessAsUser");

	PyObject *ret = PyTuple_New(4);
	PyTuple_SET_ITEM(ret, 0, PyWinObject_FromHANDLE(pi.hProcess));
	PyTuple_SET_ITEM(ret, 1, PyWinObject_FromHANDLE(pi.hThread));
	PyTuple_SET_ITEM(ret, 2, PyLong_FromLong(pi.dwProcessId));
	PyTuple_SET_ITEM(ret, 3, PyLong_FromLong(pi.dwThreadId));
	return ret;
}
%}

// @pyswig <o PyHANDLE>, <o PyHANDLE>, int, int|CreateProcessAsUser|Creates a new process in the context of the specified user.
// @comm The result is a tuple of (hProcess, hThread, dwProcessId, dwThreadId)
%name(CreateProcessAsUser)
PyObject *MyCreateProcessAsUser(
	HANDLE hToken, // @pyparm <o PyHANDLE>|hToken||Handle to a token that represents a logged-on user
	TCHAR *INPUT_NULLOK,  // @pyparm string|appName||name of executable module, or None
	TCHAR *INPUT_NULLOK,  // @pyparm string|commandLine||command line string, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|processAttributes||process security attributes, or None
	SECURITY_ATTRIBUTES *INPUT_NULLOK, // @pyparm <o PySECURITY_ATTRIBUTES>|threadAttributes||thread security attributes, or None
	BOOL bInheritHandles, // @pyparm int|bInheritHandles||handle inheritance flag
	DWORD dwCreationFlags, // @pyparm int|dwCreationFlags||creation flags
	PyObject *env, // @pyparm None|newEnvironment||A dictionary of string or Unicode pairs to define the environment for the process, or None to inherit the current environment.
	TCHAR *INPUT_NULLOK, // @pyparm string|currentDirectory||current directory name, or None
	STARTUPINFO *lpStartupInfo // @pyparm <o PySTARTUPINFO>|startupinfo||a STARTUPINFO object that specifies how the main window for the new process should appear.
);


%{
// GetCurrentProcess returns -1 which is INVALID_HANDLE_VALUE, so can't use swig typemap for HANDLE
// @pyswig int|GetCurrentProcess|Retrieves a pseudo handle for the current process.
static PyObject *MyGetCurrentProcess(PyObject *self, PyObject *args)
{
	if(!PyArg_ParseTuple(args,":GetCurrentProcess"))
		return NULL;
	return PyWinLong_FromHANDLE(GetCurrentProcess());
}
%}
%native (GetCurrentProcess) MyGetCurrentProcess;

// @pyswig int|GetProcessVersion|Obtains the major and minor version numbers of the system on which a specified process expects to run.
DWORD GetProcessVersion(
	DWORD ProcessId  // @pyparm int|processId||identifier specifying the process of interest.
);

// @pyswig int|GetCurrentProcessId|Retrieves the process identifier of the calling process.
DWORD GetCurrentProcessId();

// @pyswig <o PySTARTUPINFO>|GetStartupInfo|Retrieves the contents of the STARTUPINFO structure that was specified when the calling process was created.
void GetStartupInfo(
	STARTUPINFO *OUTPUT
);

// @pyswig int|GetPriorityClass|
DWORD GetPriorityClass(
	HANDLE hThread // @pyparm <o PyHANDLE>|handle||handle to the thread
);

// @pyswig int|GetExitCodeThread|
BOOLAPI GetExitCodeThread(
	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the thread
	DWORD *OUTPUT
);

// @pyswig int|GetExitCodeProcess|
BOOLAPI GetExitCodeProcess(
	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the process
	DWORD *OUTPUT
);

// @pyswig int, int|GetWindowThreadProcessId|Retrieves the identifier of the thread and process that created the specified window.
long GetWindowThreadProcessId(
	HWND hwnd, // @pyparm int|hwnd||handle to the window
	DWORD *OUTPUT
    // @rdesc The result is a tuple of (threadId, processId)
);

// @pyswig |SetThreadPriority|
BOOLAPI SetThreadPriority(
	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the thread
	int nPriority   // @pyparm int|nPriority||thread priority level
);

// @pyswig int|GetThreadPriority|
DWORD GetThreadPriority(
	HANDLE hThread // @pyparm <o PyHANDLE>|handle||handle to the thread
);

%{
// @pyswig bool|GetProcessPriorityBoost|Determines if dynamic priority adjustment is enabled for a process
static PyObject *PyGetProcessPriorityBoost(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessPriorityBoost);
	PyObject *obth;
	HANDLE th;
	BOOL ret;
	if (!PyArg_ParseTuple(args, "O:GetProcessPriorityBoost",
		&obth))		// @pyparm <o PyHANDLE>|Process||Handle to a process
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnGetProcessPriorityBoost)(th, &ret))
		return PyWin_SetAPIError("GetProcessPriorityBoost");
	return PyBool_FromLong(ret);
}

// @pyswig |SetProcessPriorityBoost|Enables or disables dynamic priority adjustment for a process
static PyObject *PySetProcessPriorityBoost(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetProcessPriorityBoost);
	PyObject *obth;
	HANDLE th;
	BOOL disable;
	if (!PyArg_ParseTuple(args, "Ol:SetProcessPriorityBoost",
		&obth,		// @pyparm <o PyHANDLE>|Process||Handle to a process
		&disable))	// @pyparm boolean|DisablePriorityBoost||True to disable or False to enable
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnSetProcessPriorityBoost)(th, disable))
		return PyWin_SetAPIError("SetProcessPriorityBoost");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig bool|GetThreadPriorityBoost|Determines if dynamic priority adjustment is enabled for a thread
static PyObject *PyGetThreadPriorityBoost(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetThreadPriorityBoost);
	PyObject *obth;
	HANDLE th;
	BOOL ret;
	if (!PyArg_ParseTuple(args, "O:GetThreadPriorityBoost",
		&obth))		// @pyparm <o PyHANDLE>|Thread||Handle to a thread
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnGetThreadPriorityBoost)(th, &ret))
		return PyWin_SetAPIError("GetThreadPriorityBoost");
	return PyBool_FromLong(ret);
}

// @pyswig |SetThreadPriorityBoost|Enables or disables dynamic priority adjustment for a thread
static PyObject *PySetThreadPriorityBoost(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetThreadPriorityBoost);
	PyObject *obth;
	HANDLE th;
	BOOL disable;
	if (!PyArg_ParseTuple(args, "Ol:SetThreadPriorityBoost",
		&obth,		// @pyparm <o PyHANDLE>|Thread||Handle to a thread
		&disable))	// @pyparm boolean|DisablePriorityBoost||True to disable or False to enable
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnSetThreadPriorityBoost)(th, disable))
		return PyWin_SetAPIError("SetThreadPriorityBoost");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig bool|GetThreadIOPendingFlag|Determines if thread has any outstanding IO requests
static PyObject *PyGetThreadIOPendingFlag(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetThreadIOPendingFlag);
	PyObject *obth;
	HANDLE th;
	BOOL ret;
	if (!PyArg_ParseTuple(args, "O:GetThreadIOPendingFlag",
		&obth))		// @pyparm <o PyHANDLE>|Thread||Handle to a thread
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnGetThreadPriorityBoost)(th, &ret))
		return PyWin_SetAPIError("GetThreadIOPendingFlag");
	return PyBool_FromLong(ret);
}

// @pyswig dict|GetThreadTimes|Returns a thread's time statistics
static PyObject *PyGetThreadTimes(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetThreadTimes);
	PyObject *obth;
	HANDLE th;
	FILETIME ft[4];
	if (!PyArg_ParseTuple(args, "O:GetThreadTimes",
		&obth))		// @pyparm <o PyHANDLE>|Thread||Handle to a thread
		return NULL;
	if (!PyWinObject_AsHANDLE(obth, &th))
		return NULL;
	if (!(*pfnGetThreadTimes)(th, &ft[0], &ft[1], &ft[2], &ft[3]))
		return PyWin_SetAPIError("GetThreadTimes");

	// UserTime and KernelTime are elapsed times, return as ints
	ULARGE_INTEGER usertime, kerneltime;
	kerneltime.LowPart=ft[2].dwLowDateTime;
	kerneltime.HighPart=ft[2].dwHighDateTime;
	usertime.LowPart=ft[3].dwLowDateTime;
	usertime.HighPart=ft[3].dwHighDateTime;
	return Py_BuildValue("{s:N, s:N, s:N, s:N}",
		"CreationTime", PyWinObject_FromFILETIME(ft[0]),
		"ExitTime",		PyWinObject_FromFILETIME(ft[1]),
		"KernelTime",	PyLong_FromUnsignedLongLong(kerneltime.QuadPart),
		"UserTime",		PyLong_FromUnsignedLongLong(usertime.QuadPart));
}

// @pyswig int|GetProcessId|Returns the Pid for a process handle
static PyObject *PyGetProcessId(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessId);
	PyObject *obhprocess;
	HANDLE hprocess;
	DWORD pid;
	if (!PyArg_ParseTuple(args, "O:GetProcessId",
		&obhprocess))	// @pyparm <o PyHANDLE>|Process||Handle to a process
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	pid=(*pfnGetProcessId)(hprocess);
	if (pid==0)
		return PyWin_SetAPIError("GetProcessId");
	return PyLong_FromUnsignedLong(pid);
}
%}
%native (GetProcessPriorityBoost) PyGetProcessPriorityBoost;
%native (SetProcessPriorityBoost) PySetProcessPriorityBoost;
%native (GetThreadPriorityBoost) PyGetThreadPriorityBoost;
%native (SetThreadPriorityBoost) PySetThreadPriorityBoost;
%native (GetThreadIOPendingFlag) PyGetThreadIOPendingFlag;
%native (GetThreadTimes) PyGetThreadTimes;
%native (GetProcessId) PyGetProcessId;

// @pyswig |SetPriorityClass|
BOOLAPI SetPriorityClass(
  	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the process
	DWORD dwPriorityClass   // @pyparm int|dwPriorityClass||priority class value
);

// @pyswig |AttachThreadInput|Attaches or detaches the input of two threads
BOOLAPI AttachThreadInput(
	DWORD idAttach,		// @pyparm int|idAttach||The id of a thread
	DWORD idAttachTo,	// @pyparm int|idAttachTo||The id of the thread to which it will be attached
	BOOL Attach		// @pyparm bool|Attach||Indicates whether thread should be attached or detached
);

%{
// This function does not exist on all platforms.
static PyObject *MySetThreadIdealProcessor( HANDLE hThread, DWORD dwIdealProc )
{
	CHECK_PFN(SetThreadIdealProcessor);
	DWORD rc = (*pfnSetThreadIdealProcessor)(hThread, dwIdealProc);
	if (rc==-1)
		return PyWin_SetAPIError("SetThreadIdealProcessor");
	return PyLong_FromLong(rc);
}
%}

// @pyswig int|SetThreadIdealProcessor|Used to specify a preferred processor for a thread. The system schedules threads on their preferred processors whenever possible.
%name(SetThreadIdealProcessor)
PyObject *MySetThreadIdealProcessor(
  HANDLE hThread,             // @pyparm <o PyHANDLE>|handle||handle to the thread of interest
  DWORD dwIdealProcessor  // @pyparm int|dwIdealProcessor||ideal processor number
);

%{
// @pyswig int, int|GetProcessAffinityMask|Gets a processor affinity mask for a specified process
// @rdesc The result is a tuple of (process affinity mask, system affinity mask)
static PyObject *MyGetProcessAffinityMask(PyObject *self, PyObject *args)
{
	HANDLE hProcess;
	PyObject *obhProcess;
	DWORD_PTR processmask, systemmask;
	if (!PyArg_ParseTuple(args, "O:GetProcessAffinityMask",
		&obhProcess))	// @pyparm <o PyHANDLE>|hProcess||handle to the process of interest
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;
	if (!GetProcessAffinityMask(hProcess, &processmask, &systemmask))
		return PyWin_SetAPIError("GetProcessAffinityMask");
	return Py_BuildValue("NN",
		PyLong_FromUnsignedLongLong(processmask),
		PyLong_FromUnsignedLongLong(systemmask));
}

// Appears to be some problem with the optimizer here, so I just leave it off!
#pragma optimize ("", off)
// @pyswig |SetProcessAffinityMask|Sets a processor affinity mask for a specified process.
// @comm This function does not exist on all platforms.
static PyObject *MySetProcessAffinityMask(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetProcessAffinityMask);
	DWORD_PTR dwMask;
	HANDLE hProcess;
	PyObject *obhProcess;
	// Mask is 64 bits on win64
#ifdef _WIN64
	static char *fmt="OK:SetProcessAffinityMask";
#else
	static char *fmt="Ok:SetProcessAffinityMask";
#endif
	if (!PyArg_ParseTuple(args, fmt,
		&obhProcess,	// @pyparm <o PyHANDLE>|hProcess||handle to the process of interest
		&dwMask))		// @pyparm int|mask||a processor affinity mask
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;
	if (!(*pfnSetProcessAffinityMask)(hProcess, dwMask))
		return PyWin_SetAPIError("SetProcessAffinityMask");
	Py_INCREF(Py_None);
	return Py_None;
}
#pragma optimize ("", on)

// @pyswig int|SetThreadAffinityMask|Sets a processor affinity mask for a specified thread.
static PyObject *MySetThreadAffinityMask(PyObject *self, PyObject *args)
{
	DWORD_PTR dwMask, prevMask;
	HANDLE hThread;
	PyObject *obhThread;
	// Mask is 64 bits on win64
#ifdef _WIN64
	static char *fmt="OK:SetThreadAffinityMask";
#else
	static char *fmt="Ok:SetThreadAffinityMask";
#endif
	if (!PyArg_ParseTuple(args, fmt,
		&obhThread,		// @pyparm <o PyHANDLE>|hThread||handle to the thread of interest
		&dwMask))		// @pyparm int|ThreadAffinityMask||a processor affinity mask
		return NULL;
	if (!PyWinObject_AsHANDLE(obhThread, &hThread))
		return NULL;
	prevMask=SetThreadAffinityMask(hThread, dwMask);
	if (prevMask==0)
		return PyWin_SetAPIError("SetThreadAffinityMask");
	return PyLong_FromUnsignedLongLong(prevMask);
}
%}
%native(GetProcessAffinityMask) MyGetProcessAffinityMask;
%native(SetProcessAffinityMask) MySetProcessAffinityMask;
%native(SetThreadAffinityMask) MySetThreadAffinityMask;

// Special result handling for SuspendThread and ResumeThread
%typedef DWORD DWORD_SR_THREAD
%typemap(python,out) DWORD_SR_THREAD {
	$target = PyLong_FromLong($source);
}
%typemap(python,except) DWORD_SR_THREAD {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($source==-1)  {
           $cleanup
           return PyWin_SetAPIError("$name");
      }
}

// @pyswig int|SuspendThread|Suspends the specified thread.
// @rdesc The return value is the thread's previous suspend count
DWORD_SR_THREAD SuspendThread(
	HANDLE hThread // @pyparm <o PyHANDLE>|handle||handle to the thread
);

// @pyswig int|ResumeThread|Resumes the specified thread. When the suspend count is decremented to zero, the execution of the thread is resumed.
// @rdesc The return value is the thread's previous suspend count
DWORD_SR_THREAD ResumeThread(
	HANDLE hThread // @pyparm <o PyHANDLE>|handle||handle to the thread
);

// @pyswig |TerminateProcess|Terminates the specified process and all of its threads.
BOOLAPI TerminateProcess(
	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the process
	DWORD exitCode  // @pyparm int|exitCode||The exit code for the process.
);

// @pyswig |ExitProcess|Ends a process and all its threads
void ExitProcess(
	DWORD exitCode  // @pyparm int|exitCode||Specifies the exit code for the process, and for all threads that are terminated as a result of this call
	// @comm ExitProcess is the preferred method of ending a process. This function provides
	// a clean process shutdown. This includes calling the entry-point function of all
	// attached dynamic-link libraries (DLLs) with a value indicating that the process
	// is detaching from the DLL. If a process terminates by calling
	// <om win32process.TerminateProcess>, the DLLs that the process is attached to are
	// not notified of the process termination.
);

// @pyswig (long,....)|EnumProcesses|Returns Pids for currently running processes
%native(EnumProcesses) PyEnumProcesses;
%{
PyObject *PyEnumProcesses(PyObject *self, PyObject *args)
{
	CHECK_PFN(EnumProcesses);
	DWORD *pids=NULL, *pid=NULL;
	DWORD nbr_pids_allocated=100, nbr_pids_returned=0, tuple_ind=0;
	DWORD bytes_allocated=0,bytes_returned=0;
	PyObject *ret=NULL, *obpid=NULL;
	if (!PyArg_ParseTuple(args, ":EnumProcesses"))
		return NULL;

	// function gives no indicator that not all were returned, so loop until fewer returned than allocated
	do{
		if (pids){
			nbr_pids_allocated*=2;
			free(pids);
			}
		bytes_allocated=nbr_pids_allocated*sizeof(DWORD);
		pids=(DWORD *)malloc(bytes_allocated);
		if (pids==NULL){
			PyErr_SetString(PyExc_MemoryError,"EnumProcesses: unable to allocate Pid list");
			return NULL;
			}
		if (!(*pfnEnumProcesses)(pids, bytes_allocated, &bytes_returned)){
			PyWin_SetAPIError("EnumProcesses",GetLastError());
			goto done;
			}
		nbr_pids_returned=bytes_returned/sizeof(DWORD);
		}
	while (nbr_pids_returned==nbr_pids_allocated);

	ret=PyTuple_New(nbr_pids_returned);
	if (ret==NULL){
		PyErr_SetString(PyExc_MemoryError,"EnumProcesses: unable to allocate return tuple");
		goto done;
		}
	pid=pids;
	for (tuple_ind=0;tuple_ind<nbr_pids_returned;tuple_ind++){
		obpid=Py_BuildValue("l",*pid);
		if (obpid==NULL){
			Py_DECREF(ret);
			ret=NULL;
			goto done;
			}
		PyTuple_SetItem(ret,tuple_ind,obpid);
		pid++;
		}
done:
	if (pids)
		free (pids);
	return ret;
}
%}

// @pyswig (long,....)|EnumProcessModules|Lists loaded modules for a process handle
%native(EnumProcessModules) PyEnumProcessModules;
%{
PyObject *PyEnumProcessModules(PyObject *self, PyObject *args)
{
	CHECK_PFN(EnumProcessModules);
	HMODULE *hmods=NULL, *hmod=NULL;
	HANDLE hprocess=NULL;
	DWORD nbr_hmods_allocated=100, nbr_hmods_returned=0, tuple_ind=0;
	DWORD bytes_allocated=0,bytes_needed=0;
	PyObject *ret=NULL, *obhmod=NULL, *obhprocess;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	if (!PyArg_ParseTuple(args, "O:EnumProcessModules", &obhprocess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	bytes_allocated=nbr_hmods_allocated*sizeof(HMODULE);
	do{
		if (hmods){
			free(hmods);
			bytes_allocated=bytes_needed; // unlike EnumProcesses, this one tells you if more space is needed
			}
		hmods=(HMODULE *)malloc(bytes_allocated);
		if (hmods==NULL){
			PyErr_SetString(PyExc_MemoryError,"EnumProcessModules: unable to allocate HMODULE list");
			return NULL;
			}
		if (!(*pfnEnumProcessModules)(hprocess, hmods, bytes_allocated, &bytes_needed)){
			PyWin_SetAPIError("EnumProcessModules",GetLastError());
			goto done;
			}
		}
	while (bytes_needed>bytes_allocated);

	nbr_hmods_returned=bytes_needed/sizeof(HMODULE);
	ret=PyTuple_New(nbr_hmods_returned);
	if (ret==NULL){
		PyErr_SetString(PyExc_MemoryError,"EnumProcessModules: unable to allocate return tuple");
		goto done;
		}
	hmod=hmods;
	for (tuple_ind=0;tuple_ind<nbr_hmods_returned;tuple_ind++){
		obhmod=PyWinLong_FromHANDLE(*hmod);
		if (obhmod==NULL){
			Py_DECREF(ret);
			ret=NULL;
			goto done;
			}
		PyTuple_SET_ITEM(ret,tuple_ind,obhmod);
		hmod++;
		}
done:
	if (hmods)
		free (hmods);
	return ret;
}
%}

// @pyswig (long,....)|EnumProcessModulesEx|Lists 32 or 64-bit modules load by a process
%native(EnumProcessModulesEx) PyEnumProcessModulesEx;
%{
PyObject *PyEnumProcessModulesEx(PyObject *self, PyObject *args)
{
	CHECK_PFN(EnumProcessModulesEx);
	HMODULE *hmods=NULL, *hmod=NULL;
	HANDLE hprocess=NULL;
	DWORD nbr_hmods_allocated=100, nbr_hmods_returned=0, tuple_ind=0;
	DWORD bytes_allocated=0,bytes_needed=0;
	DWORD FilterFlag = LIST_MODULES_DEFAULT;
	PyObject *ret=NULL, *obhmod=NULL, *obhprocess;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	// @pyparm int|FilterFlag|LIST_MODULES_DEFAULT|Controls whether 32 or 64-bit modules are returned
	if (!PyArg_ParseTuple(args, "O|k:EnumProcessModulesEx", &obhprocess, &FilterFlag))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	bytes_allocated=nbr_hmods_allocated*sizeof(HMODULE);
	do{
		if (hmods){
			free(hmods);
			bytes_allocated=bytes_needed; // unlike EnumProcesses, this one tells you if more space is needed
			}
		hmods=(HMODULE *)malloc(bytes_allocated);
		if (hmods==NULL){
			PyErr_SetString(PyExc_MemoryError,"EnumProcessModulesEx: unable to allocate HMODULE list");
			return NULL;
			}
		if (!(*pfnEnumProcessModulesEx)(hprocess, hmods, bytes_allocated, &bytes_needed, FilterFlag)){
			PyWin_SetAPIError("EnumProcessModulesEx",GetLastError());
			goto done;
			}
		}
	while (bytes_needed>bytes_allocated);

	nbr_hmods_returned=bytes_needed/sizeof(HMODULE);
	ret=PyTuple_New(nbr_hmods_returned);
	if (ret==NULL)
		goto done;
	hmod=hmods;
	for (tuple_ind=0;tuple_ind<nbr_hmods_returned;tuple_ind++){
		obhmod=PyWinLong_FromHANDLE(*hmod);
		if (obhmod==NULL){
			Py_DECREF(ret);
			ret=NULL;
			goto done;
			}
		PyTuple_SET_ITEM(ret,tuple_ind,obhmod);
		hmod++;
		}
done:
	if (hmods)
		free (hmods);
	return ret;
}
%}

// @pyswig <o PyUNICODE>|GetModuleFileNameEx|Return name of module loaded by another process (uses process handle, not pid)
%native(GetModuleFileNameEx) PyGetModuleFileNameEx;
%{
PyObject *PyGetModuleFileNameEx(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetModuleFileNameEx);
	WCHAR *fname=NULL;
	DWORD chars_allocated=256, chars_returned=0;
	// chars_allocated=5; // test allocation loop
	HMODULE hmod;
	HANDLE hprocess;
	PyObject *ret=NULL, *obhprocess, *obhmod;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	// @pyparm <o PyHANDLE>|hModule||Module handle
	if (!PyArg_ParseTuple(args, "OO:GetModuleFileNameEx", &obhprocess, &obhmod))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhmod, (HANDLE *)&hmod))
		return NULL;

	do{
		if (fname){
			free(fname);
			chars_allocated*=2;
			}
		fname=(WCHAR *)malloc(chars_allocated*sizeof(WCHAR));
		if (fname==NULL){
			PyErr_SetString(PyExc_MemoryError,"GetModuleFileNameEx: unable to allocate WCHAR buffer");
			return NULL;
			}
		chars_returned=(*pfnGetModuleFileNameEx)(hprocess, hmod, fname, chars_allocated);
		if (!chars_returned){
			PyWin_SetAPIError("GetModuleFileNameEx",GetLastError());
			goto done;
			}
		}
	while (chars_returned==chars_allocated);
	ret=PyWinObject_FromWCHAR(fname,chars_returned);

done:
	if (fname)
		free (fname);
	return ret;
}
%}

// @pyswig <o dict>|GetProcessMemoryInfo|Returns process memory statistics as a dict representing a PROCESS_MEMORY_COUNTERS struct
%native(GetProcessMemoryInfo) PyGetProcessMemoryInfo;
%{
PyObject *PyGetProcessMemoryInfo(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessMemoryInfo);
	HANDLE hProcess;
	PyObject *obhProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	DWORD cb=sizeof(PROCESS_MEMORY_COUNTERS);
	pmc.cb=cb;

	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	if (!PyArg_ParseTuple(args, "O:GetProcessMemoryInfo", &obhProcess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;

	if (!(*pfnGetProcessMemoryInfo)(hProcess, &pmc, cb)){
		PyWin_SetAPIError("GetProcessMemoryInfo",GetLastError());
		return NULL;
		}
	return Py_BuildValue("{s:k,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N}",
		"PageFaultCount", pmc.PageFaultCount,
		"PeakWorkingSetSize", PyLong_FromUnsignedLongLong(pmc.PeakWorkingSetSize),
		"WorkingSetSize", PyLong_FromUnsignedLongLong(pmc.WorkingSetSize),
		"QuotaPeakPagedPoolUsage", PyLong_FromUnsignedLongLong(pmc.QuotaPeakPagedPoolUsage),
		"QuotaPagedPoolUsage", PyLong_FromUnsignedLongLong(pmc.QuotaPagedPoolUsage),
		"QuotaPeakNonPagedPoolUsage", PyLong_FromUnsignedLongLong(pmc.QuotaPeakNonPagedPoolUsage),
		"QuotaNonPagedPoolUsage", PyLong_FromUnsignedLongLong(pmc.QuotaNonPagedPoolUsage),
		"PagefileUsage", PyLong_FromUnsignedLongLong(pmc.PagefileUsage),
		"PeakPagefileUsage", PyLong_FromUnsignedLongLong(pmc.PeakPagefileUsage));
}
%}

// @pyswig <o dict>|GetProcessTimes|Retrieve time statics for a process by handle.  (KernelTime and UserTime in 100 nanosecond units)
%native(GetProcessTimes) PyGetProcessTimes;
%{
PyObject *PyGetProcessTimes(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessTimes);
	HANDLE hProcess;
	PyObject *obhProcess;
	FILETIME CreationTime, ExitTime, KernelTime, UserTime;
	ULARGE_INTEGER ulKernelTime, ulUserTime;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	if (!PyArg_ParseTuple(args, "O:GetProcessTimes", &obhProcess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;

	if (!(*pfnGetProcessTimes)(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime)){
		PyWin_SetAPIError("GetProcessTimes",GetLastError());
		return NULL;
		}
	memcpy(&ulKernelTime,&KernelTime,sizeof(FILETIME));
	memcpy(&ulUserTime,&UserTime,sizeof(FILETIME));
	return Py_BuildValue("{s:N,s:N,s:N,s:N}",
		"CreationTime", PyWinObject_FromFILETIME(CreationTime),
		"ExitTime", PyWinObject_FromFILETIME(ExitTime),
		"KernelTime", PyLong_FromUnsignedLongLong(ulKernelTime.QuadPart),
		"UserTime", PyLong_FromUnsignedLongLong(ulUserTime.QuadPart));
}
%}

// @pyswig <o dict>|GetProcessIoCounters|Return I/O statistics for a process as a dictionary representing an IO_COUNTERS struct.
%native(GetProcessIoCounters) PyGetProcessIoCounters;
%{
PyObject *PyGetProcessIoCounters(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessIoCounters);
	HANDLE hProcess;
	PyObject *obhProcess;
	IO_COUNTERS ioc;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
	if (!PyArg_ParseTuple(args, "O:GetProcessIoCounters", &obhProcess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;
	if (!(*pfnGetProcessIoCounters)(hProcess, &ioc)){
		PyWin_SetAPIError("GetProcessIoCounters",GetLastError());
		return NULL;
		}
	return PyWinObject_FromIO_COUNTERS(&ioc);
}
%}

// @pyswig |GetProcessWindowStation|Returns a handle to the window station for the calling process
%native(GetProcessWindowStation) PyGetProcessWindowStation;
%{
PyObject *PyGetProcessWindowStation(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessWindowStation);
	if (!PyArg_ParseTuple(args, ":GetProcessWindowStation"))
		return NULL;
	HWINSTA hwinsta=(*pfnGetProcessWindowStation)();
	return PyWinObject_FromHANDLE(hwinsta);
}
%}

// @pyswig int,int|GetProcessWorkingSetSize|Returns min and max working set sizes for a process by handle
%native(GetProcessWorkingSetSize) PyGetProcessWorkingSetSize;
%{
PyObject *PyGetProcessWorkingSetSize(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessWorkingSetSize);
	SIZE_T MinimumWorkingSetSize=0,MaximumWorkingSetSize=0;
	HANDLE hProcess;
	PyObject *obhProcess;
	// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by <om win32api.OpenProcess>
	if (!PyArg_ParseTuple(args, "O:GetProcessWorkingSetSize", &obhProcess))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;
	if (!(*pfnGetProcessWorkingSetSize)(hProcess, &MinimumWorkingSetSize, &MaximumWorkingSetSize)){
		PyWin_SetAPIError("GetProcessWorkingSetSize",GetLastError());
		return NULL;
		}
	// integer promotion happens automatically, so this should work for both 32 and 64-bit SIZE_T
	return Py_BuildValue("NN",
		PyLong_FromUnsignedLongLong(MinimumWorkingSetSize),
		PyLong_FromUnsignedLongLong(MaximumWorkingSetSize));
}
%}

// @pyswig |SetProcessWorkingSetSize|Sets minimum and maximum working set sizes for a process
// @comm Set both min and max to -1 to have process swapped out completely
%native(SetProcessWorkingSetSize) PySetProcessWorkingSetSize;
%{
PyObject *PySetProcessWorkingSetSize(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetProcessWorkingSetSize);
	SIZE_T MinimumWorkingSetSize=0,MaximumWorkingSetSize=0;
	HANDLE hProcess;
	PyObject *obhProcess;

	static char *fmt="Onn:SetProcessWorkingSetSize";
	if (!PyArg_ParseTuple(args, fmt,
		&obhProcess,				// @pyparm <o PyHANDLE>|hProcess||Process handle as returned by OpenProcess
		&MinimumWorkingSetSize,		// @pyparm int|MinimumWorkingSetSize||Minimum number of bytes to keep in physical memory
		&MaximumWorkingSetSize))	// @pyparm int|MaximumWorkingSetSize||Maximum number of bytes to keep in physical memory
		return NULL;
	if (!PyWinObject_AsHANDLE(obhProcess, &hProcess))
		return NULL;
	if (!(*pfnSetProcessWorkingSetSize)(hProcess, MinimumWorkingSetSize, MaximumWorkingSetSize))
		return PyWin_SetAPIError("SetProcessWorkingSetSize");
	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig int,int|GetProcessShutdownParameters|Retrieves shutdown priority and flags for current process
// @comm Ranges are 000-0FF Reserved by windows, 100-1FF Last, 200-2FF Middle, 300-3FF First, 400-4FF Reserved by Windows
%native(GetProcessShutdownParameters) PyGetProcessShutdownParameters;
%{
PyObject *PyGetProcessShutdownParameters(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetProcessShutdownParameters);
	DWORD Level=0, Flags=0;
	if (!PyArg_ParseTuple(args, ":GetProcessShutdownParameters"))
		return NULL;
	if (!(*pfnGetProcessShutdownParameters)(&Level, &Flags)){
		PyWin_SetAPIError("GetProcessShutdownParameters",GetLastError());
		return NULL;
		}
	return Py_BuildValue("ll",Level,Flags);
}
%}

// @pyswig |SetProcessShutdownParameters|Sets shutdown priority and flags for current process
// @comm Ranges are 000-0FF Reserved by windows, 100-1FF Last, 200-2FF Middle, 300-3FF First, 400-4FF Reserved by windows
%native(SetProcessShutdownParameters) PySetProcessShutdownParameters;
%{
PyObject *PySetProcessShutdownParameters(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetProcessShutdownParameters);
	DWORD Level=0, Flags=0;
	// @pyparm int|Level||Priority, higher means earlier
	// @pyparm int|Flags||Currently only SHUTDOWN_NORETRY valid
	if (!PyArg_ParseTuple(args, "ll:SetProcessShutdownParameters", &Level, &Flags))
		return NULL;
	if (!(*pfnSetProcessShutdownParameters)(Level, Flags)){
		PyWin_SetAPIError("SetProcessShutdownParameters",GetLastError());
		return NULL;
		}
	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig int|GetGuiResources|Returns the number of GDI or user object handles held by a process
// @comm Available on Win2k and up
%native(GetGuiResources) PyGetGuiResources;
%{
PyObject *PyGetGuiResources(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetGuiResources);
	HANDLE hprocess;
	DWORD flags, handle_cnt;
	PyObject *obhprocess;
	// @pyparm <o PyHANDLE>|Process||Handle to a process as returned by <om win32api.OpenProcess>
	// @pyparm int|Flags||GR_GDIOBJECTS or GR_USEROBJECTS (from win32con)
	if (!PyArg_ParseTuple(args, "Ok:GetGuiResources", &obhprocess, &flags))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	handle_cnt=(*pfnGetGuiResources)(hprocess, flags);
	// can return 0 for a non-GUI process with no error occurring
	if ((handle_cnt==0)	&& (GetLastError()!=0))
		return PyWin_SetAPIError("GetGuiResources");
	return PyLong_FromUnsignedLong(handle_cnt);
}
%}

// @pyswig bool|IsWow64Process|Determines whether the specified process is running under WOW64.
// @rdesc If this function is not provided by the operating system, the
// return value is False (ie, a NotImplemented exception will never be thrown).
// However, if the function exists but fails, a win32process.error exception
// is thrown as normal.
%native(IsWow64Process) PyIsWow64Process;
%{
PyObject *PyIsWow64Process(PyObject *self, PyObject *args)
{
	if (pfnIsWow64Process==NULL)
		return PyBool_FromLong(FALSE);
	PyObject *obhprocess = Py_None;
	HANDLE hprocess;
	// @pyparm <o PyHANDLE>|Process|None|Handle to a process as returned by
	// <om win32api.OpenProcess>, <om win32api.GetCurrentProcess>, etc, or
	// will use the current process handle if None (the default) is passed.
	if (!PyArg_ParseTuple(args, "|O:IsWow64Process", &obhprocess))
		return NULL;
	BOOL ret;
	if (obhprocess == Py_None)
		hprocess = ::GetCurrentProcess();
	else if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	BOOL ok = (*pfnIsWow64Process)(hprocess, &ret);
	if (!ok)
		return PyWin_SetAPIError("IsWow64Process");
	return PyBool_FromLong(ret);
}
%}

%typedef VOID *LONG_VOIDPTR;
%typemap(python,except) LONG_VOIDPTR {
	Py_BEGIN_ALLOW_THREADS
	$function
	Py_END_ALLOW_THREADS
	if ($source==0)  {
		$cleanup;
		return PyWin_SetAPIError("$name");
	}
}

%typemap(python, in) LONG_VOIDPTR {
	if (!PyWinLong_AsVoidPtr($source, &$target))
		return NULL;
}
%typemap(python, out) LONG_VOIDPTR
{
	$target = PyWinLong_FromVoidPtr($source);
}


// @pyswig long|VirtualAllocEx|
LONG_VOIDPTR VirtualAllocEx(
	HANDLE hProcess, // @pyparm <o PyHANDLE>|hProcess||
	LONG_VOIDPTR lpAddress, // @pyparm long|address||
	ULONG_PTR dwSize, // @pyparm long|size||
	DWORD flAllocationType, // @pyparm long|allocationType||
	DWORD flProtect // @pyparm long|flProtect||
);

// @pyswig |VirtualFreeEx|
BOOLAPI VirtualFreeEx(
	HANDLE hProcess, // @pyparm <o PyHANDLE>|hProcess||
	LONG_VOIDPTR lpAddress, // @pyparm long|address||
	ULONG_PTR dwSize, // @pyparm long|size||
	DWORD dwFreeType // @pyparm long|freeType||
);

%native(ReadProcessMemory) PyReadProcessMemory;
%{
PyObject *PyReadProcessMemory(PyObject *self, PyObject *args)
{
	PyObject *obhprocess;
	PyObject *obAddress;
	Py_ssize_t size;
	// @pyswig bytes|ReadProcessMemory|
	// @pyparm <o PyHANDLE>|hProcess||
	// @pyparm int|address||
	// @pyparm int|size||
	if (!PyArg_ParseTuple(args, "OOn:ReadProcessMemory", &obhprocess, &obAddress, &size))
		return NULL;
	HANDLE hprocess;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	LPVOID address;
	if (!PyWinLong_AsVoidPtr(obAddress, &address))
		return NULL;
	VOID *buffer = malloc(size);
	if (buffer == NULL) {
		PyErr_SetString(PyExc_MemoryError, "Can't allocate buffer");
		return NULL;
	}
	SIZE_T sizeWritten = 0;
	PyObject *result = NULL;
	if (ReadProcessMemory(hprocess, address, buffer, size, &sizeWritten)) {
		result = PyBytes_FromStringAndSize((const char *)buffer, sizeWritten);
	} else {
		PyWin_SetAPIError("ReadProcessMemory");
	}
	free(buffer);
	return result;
}
%}

%native(WriteProcessMemory) PyWriteProcessMemory;
%{
PyObject *PyWriteProcessMemory(PyObject *self, PyObject *args)
{
	PyObject *obhprocess;
	PyObject *obAddress;
	void *buf;
	Py_ssize_t size;
	// @pyswig int|WriteProcessMemory|
	// @pyparm <o PyHANDLE>|hProcess||
	// @pyparm int|address||
	// @pyparm buffer|buf||
	if (!PyArg_ParseTuple(args, "OOs#:WriteProcessMemory", &obhprocess, &obAddress, &buf, &size))
		return NULL;
	HANDLE hprocess;
	if (!PyWinObject_AsHANDLE(obhprocess, &hprocess))
		return NULL;
	LPVOID address;
	if (!PyWinLong_AsVoidPtr(obAddress, &address))
		return NULL;
	SIZE_T sizeWritten = 0;
	if (!WriteProcessMemory(hprocess, address, buf, size, &sizeWritten)) {
		return PyWin_SetAPIError("WriteProcessMemory");
	}
	return PyLong_FromSsize_t(sizeWritten);
}
%}


%init %{

	if (PyType_Ready(&PySTARTUPINFOType) == -1)
		return NULL;

	FARPROC fp = NULL;
	HMODULE hmodule = PyWin_GetOrLoadLibraryHandle("psapi.dll");
	if (hmodule != NULL) {
		pfnEnumProcesses = (EnumProcessesfunc)GetProcAddress(hmodule, "EnumProcesses");
		pfnEnumProcessModules = (EnumProcessModulesfunc)GetProcAddress(hmodule, "EnumProcessModules");
		pfnEnumProcessModulesEx = (EnumProcessModulesExfunc)GetProcAddress(hmodule, "EnumProcessModulesEx");
		pfnGetModuleFileNameEx = (GetModuleFileNameExfunc)GetProcAddress(hmodule, "GetModuleFileNameExW");
		pfnGetProcessMemoryInfo = (GetProcessMemoryInfofunc)GetProcAddress(hmodule, "GetProcessMemoryInfo");
	}

	hmodule = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
	if (hmodule != NULL) {
		pfnGetProcessTimes=(GetProcessTimesfunc)GetProcAddress(hmodule,"GetProcessTimes");
		pfnGetProcessIoCounters=(GetProcessIoCountersfunc)GetProcAddress(hmodule,"GetProcessIoCounters");
		pfnGetProcessShutdownParameters=(GetProcessShutdownParametersfunc)GetProcAddress(hmodule,"GetProcessShutdownParameters");
		pfnSetProcessShutdownParameters=(SetProcessShutdownParametersfunc)GetProcAddress(hmodule,"SetProcessShutdownParameters");
		pfnGetProcessWorkingSetSize=(GetProcessWorkingSetSizefunc)GetProcAddress(hmodule,"GetProcessWorkingSetSize");
		pfnSetProcessWorkingSetSize=(SetProcessWorkingSetSizefunc)GetProcAddress(hmodule,"SetProcessWorkingSetSize");
		pfnGetProcessPriorityBoost=(GetProcessPriorityBoostfunc)GetProcAddress(hmodule,"GetProcessPriorityBoost");
		pfnSetProcessPriorityBoost=(SetProcessPriorityBoostfunc)GetProcAddress(hmodule,"SetProcessPriorityBoost");
		pfnGetThreadPriorityBoost=(GetThreadPriorityBoostfunc)GetProcAddress(hmodule,"GetThreadPriorityBoost");
		pfnSetThreadPriorityBoost=(SetThreadPriorityBoostfunc)GetProcAddress(hmodule,"SetThreadPriorityBoost");
		pfnGetThreadIOPendingFlag=(GetThreadIOPendingFlagfunc)GetProcAddress(hmodule,"GetThreadIOPendingFlag");
		pfnGetThreadTimes=(GetThreadTimesfunc)GetProcAddress(hmodule,"GetThreadTimes");
		pfnCreateRemoteThread=(CreateRemoteThreadfunc)GetProcAddress(hmodule,"CreateRemoteThread");
		pfnSetThreadIdealProcessor=(SetThreadIdealProcessorfunc)GetProcAddress(hmodule,"SetThreadIdealProcessor");
		pfnSetProcessAffinityMask=(SetProcessAffinityMaskfunc)GetProcAddress(hmodule,"SetProcessAffinityMask");
		pfnGetProcessId=(GetProcessIdfunc)GetProcAddress(hmodule, "GetProcessId");
		pfnIsWow64Process=(IsWow64Processfunc)GetProcAddress(hmodule, "IsWow64Process");
	}

	hmodule = PyWin_GetOrLoadLibraryHandle("user32.dll");
	if (hmodule != NULL) {
		pfnGetProcessWindowStation=(GetProcessWindowStationfunc)GetProcAddress(hmodule,"GetProcessWindowStation");
		pfnGetGuiResources=(GetGuiResourcesfunc)GetProcAddress(hmodule,"GetGuiResources");
	}

// *sob* - these symbols don't exist in the platform sdk needed to build
// using Python 2.3
#ifndef THREAD_MODE_BACKGROUND_BEGIN
#define THREAD_MODE_BACKGROUND_BEGIN 0x00010000
#endif
#ifndef THREAD_MODE_BACKGROUND_END
#define THREAD_MODE_BACKGROUND_END 0x00020000
#endif
%}

#define CREATE_SUSPENDED CREATE_SUSPENDED

#define MAXIMUM_PROCESSORS MAXIMUM_PROCESSORS

#define THREAD_PRIORITY_ABOVE_NORMAL THREAD_PRIORITY_ABOVE_NORMAL // Indicates 1 point above normal priority for the priority class.
#define THREAD_PRIORITY_BELOW_NORMAL THREAD_PRIORITY_BELOW_NORMAL // Indicates 1 point below normal priority for the priority class.
#define THREAD_PRIORITY_HIGHEST THREAD_PRIORITY_HIGHEST // Indicates 2 points above normal priority for the priority class.
#define THREAD_PRIORITY_IDLE THREAD_PRIORITY_IDLE // Indicates a base priority level of 1 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 16 for REALTIME_PRIORITY_CLASS processes.
#define THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST // Indicates 2 points below normal priority for the priority class.
#define THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL // Indicates normal priority for the priority class.
#define THREAD_PRIORITY_TIME_CRITICAL THREAD_PRIORITY_TIME_CRITICAL // Indicates a base priority level of 15 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 31 for REALTIME_PRIORITY_CLASS processes.
#define THREAD_MODE_BACKGROUND_BEGIN THREAD_MODE_BACKGROUND_BEGIN
#define THREAD_MODE_BACKGROUND_END THREAD_MODE_BACKGROUND_END

#define CREATE_DEFAULT_ERROR_MODE CREATE_DEFAULT_ERROR_MODE // The new process does not inherit the error mode of the calling process. Instead, CreateProcess gives the new process the current default error mode. An application sets the current default error mode by calling SetErrorMode.
// This flag is particularly useful for multi-threaded shell applications that run with hard errors disabled.

#define CREATE_NEW_CONSOLE CREATE_NEW_CONSOLE // The new process has a new console, instead of inheriting the parent's console. This flag cannot be used with the DETACHED_PROCESS flag.

#define CREATE_NEW_PROCESS_GROUP CREATE_NEW_PROCESS_GROUP // The new process is the root process of a new process group. The process group includes all processes that are descendants of this root process. The process identifier of the new process group is the same as the process identifier, which is returned in the lpProcessInformation parameter. Process groups are used by the GenerateConsoleCtrlEvent function to enable sending a ctrl+c or ctrl+break signal to a group of console processes.

#define CREATE_SEPARATE_WOW_VDM CREATE_SEPARATE_WOW_VDM // Windows NT: This flag is valid only when starting a 16-bit Windows-based application. If set, the new process is run in a private Virtual DOS Machine (VDM). By default, all 16-bit Windows-based applications are run as threads in a single, shared VDM. The advantage of running separately is that a crash only kills the single VDM; any other programs running in distinct VDMs continue to function normally. Also, 16-bit Windows-based applications that are run in separate VDMs have separate input queues. That means that if one application hangs momentarily, applications in separate VDMs continue to receive input. The disadvantage of running separately is that it takes significantly more memory to do so. You should use this flag only if the user requests that 16-bit applications should run in them own VDM.

#define CREATE_SHARED_WOW_VDM CREATE_SHARED_WOW_VDM // Windows NT: The flag is valid only when starting a 16-bit Windows-based application. If the DefaultSeparateVDM switch in the Windows section of WIN.INI is TRUE, this flag causes the CreateProcess function to override the switch and run the new process in the shared Virtual DOS Machine.

#define CREATE_UNICODE_ENVIRONMENT CREATE_UNICODE_ENVIRONMENT // If set, the environment block pointed to by lpEnvironment uses Unicode characters. If clear, the environment block uses ANSI characters.
#define CREATE_BREAKAWAY_FROM_JOB CREATE_BREAKAWAY_FROM_JOB
#define CREATE_PRESERVE_CODE_AUTHZ_LEVEL CREATE_PRESERVE_CODE_AUTHZ_LEVEL
#define CREATE_NO_WINDOW CREATE_NO_WINDOW

// If this flag is set, the calling process is treated as a debugger, and the new process is a process being debugged. The system notifies the debugger of all debug events that occur in the process being debugged.
// If you create a process with this flag set, only the calling thread (the thread that called CreateProcess) can call the WaitForDebugEvent function.
#define DEBUG_PROCESS DEBUG_PROCESS

#define DEBUG_ONLY_THIS_PROCESS DEBUG_ONLY_THIS_PROCESS // If not set and the calling process is being debugged, the new process becomes another process being debugged by the calling process's debugger. If the calling process is not a process being debugged, no debugging-related actions occur.

#define DETACHED_PROCESS DETACHED_PROCESS // For console processes, the new process does not have access to the console of the parent process. The new process can call the AllocConsole function at a later time to create a new console. This flag cannot be used with the CREATE_NEW_CONSOLE flag.

#define ABOVE_NORMAL_PRIORITY_CLASS ABOVE_NORMAL_PRIORITY_CLASS // Indicates a process that has priority above NORMAL_PRIORITY_CLASS but below HIGH_PRIORITY_CLASS.
#define BELOW_NORMAL_PRIORITY_CLASS BELOW_NORMAL_PRIORITY_CLASS // Indicates a process that has priority above IDLE_PRIORITY_CLASS but below NORMAL_PRIORITY_CLASS.
#define HIGH_PRIORITY_CLASS HIGH_PRIORITY_CLASS // Indicates a process that performs time-critical tasks that must be executed immediately for it to run correctly. The threads of a high-priority class process preempt the threads of normal-priority or idle-priority class processes. An example is the Task List, which must respond quickly when called by the user, regardless of the load on the system. Use extreme care when using the high-priority class, because a high-priority class CPU-bound application can use nearly all available cycles.
#define IDLE_PRIORITY_CLASS IDLE_PRIORITY_CLASS // Indicates a process whose threads run only when the system is idle and are preempted by the threads of any process running in a higher priority class. An example is a screen saver. The idle priority class is inherited by child processes.
#define NORMAL_PRIORITY_CLASS NORMAL_PRIORITY_CLASS // Indicates a normal process with no special scheduling needs.
#define REALTIME_PRIORITY_CLASS REALTIME_PRIORITY_CLASS // Indicates a process that has the highest possible priority. The threads of a real-time priority class process preempt the threads of all other processes, including operating system processes performing important tasks. For example, a real-time process that executes for more than a very brief interval can cause disk caches not to flush or cause the mouse to be unresponsive.

// Used with EnumProcessModulesEx
#define LIST_MODULES_32BIT LIST_MODULES_32BIT
#define LIST_MODULES_64BIT LIST_MODULES_64BIT
#define LIST_MODULES_ALL LIST_MODULES_ALL
#define LIST_MODULES_DEFAULT LIST_MODULES_DEFAULT

#define STARTF_FORCEONFEEDBACK STARTF_FORCEONFEEDBACK
// Indicates that the cursor is in feedback mode for two seconds after CreateProcess is called. If during those two seconds the process makes the first GUI call, the system gives five more seconds to the process. If during those five seconds the process shows a window, the system gives five more seconds to the process to finish drawing the window.
// The system turns the feedback cursor off after the first call to GetMessage, regardless of whether the process is drawing.
#define STARTF_FORCEOFFFEEDBACK STARTF_FORCEOFFFEEDBACK
// Indicates that the feedback cursor is forced off while the process is starting. The normal cursor is displayed.
#define STARTF_RUNFULLSCREEN STARTF_RUNFULLSCREEN
// Indicates that the process should be run in full-screen mode, rather than in windowed mode.
// This flag is only valid for console applications running on an x86 computer.

#define STARTF_USECOUNTCHARS STARTF_USECOUNTCHARS
// If this value is not specified, the dwXCountChars and dwYCountChars members are ignored.
#define STARTF_USEFILLATTRIBUTE STARTF_USEFILLATTRIBUTE
// If this value is not specified, the dwFillAttribute member is ignored.
#define STARTF_USEPOSITION STARTF_USEPOSITION
// If this value is not specified, the dwX and dwY members are ignored.
#define STARTF_USESHOWWINDOW STARTF_USESHOWWINDOW
// If this value is not specified, the wShowWindow member is ignored.
#define STARTF_USESIZE STARTF_USESIZE
// If this value is not specified, the dwXSize and dwYSize members are ignored.
#define STARTF_USESTDHANDLES STARTF_USESTDHANDLES
// Sets the standard input, standard output, and standard error handles for the process to the handles specified in the hStdInput, hStdOutput, and hStdError members of the STARTUPINFO structure. The CreateProcess function's fInheritHandles parameter must be set to TRUE for this to work properly.
// If this value is not specified, the hStdInput, hStdOutput, and hStdError members of the STARTUPINFO structure are ignored.
