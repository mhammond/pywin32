/* File : win32process.i */

%module win32process // An interface to the win32 Process and Thread API's

%{
#ifndef MS_WINCE
#define _WIN32_WINNT 0x0400
#include "process.h"
//#include "assert.h"
#endif
#include "windows.h"
#include "PyWinTypes.h"
%}

%include "typemaps.i"
%include "pywin32.i"

%apply HWND {long};
typedef long HWND

%{
#include "structmember.h"

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

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

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
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySTARTUPINFO",
	sizeof(PySTARTUPINFO),
	0,
	PySTARTUPINFO::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySTARTUPINFO::getattr,				/* tp_getattr */
	PySTARTUPINFO::setattr,				/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PySTARTUPINFO, e)

/*static*/ struct memberlist PySTARTUPINFO::memberlist[] = {
	{"dwX",              T_INT,  OFF(m_startupinfo.dwX)}, // @prop integer|dwX|Specifies the x offset, in pixels, of the upper left corner of a window if a new window is created. The offset is from the upper left corner of the screen.
	{"dwY",              T_INT,  OFF(m_startupinfo.dwY)}, // @prop integer|dwY|Specifies the y offset, in pixels, of the upper left corner of a window if a new window is created. The offset is from the upper left corner of the screen. 
	{"dwXSize",          T_INT,  OFF(m_startupinfo.dwXSize)}, // @prop integer|dwXSize|Specifies the width, in pixels, of the window if a new window is created.
	{"dwYSize",          T_INT,  OFF(m_startupinfo.dwYSize)}, // @prop integer|dwYSize|Specifies the height, in pixels, of the window if a new window is created.
	{"dwXCountChars",    T_INT,  OFF(m_startupinfo.dwXCountChars)}, // @prop integer|dwXCountChars|For console processes, if a new console window is created, specifies the screen buffer width in character columns. This value is ignored in a GUI process. 
	{"dwYCountChars",    T_INT,  OFF(m_startupinfo.dwYCountChars)}, // @prop integer|dwYCountChars|For console processes, if a new console window is created, specifies the screen buffer height in character rows.
	{"dwFillAttribute",  T_INT,  OFF(m_startupinfo.dwFillAttribute)}, // @prop integer|dwFillAttribute|Specifies the initial text and background colors if a new console window is created in a console application. These values are ignored in GUI applications
	{"dwFlags",          T_INT,  OFF(m_startupinfo.dwFlags)}, // @prop integer|dwFlags|This is a bit field that determines whether certain STARTUPINFO attributes are used when the process creates a window. To use many of the additional attributes, you typically must set the appropriate mask in this attribute, and also set the attributes themselves. Any combination of the win32con.STARTF_* flags can be specified. 
	{"wShowWindow",	     T_INT,  OFF(m_startupinfo.wShowWindow)},//@prop integer|wShowWindow|Can be any of the SW_ constants defined in win32con. For GUI processes, this specifies the default value the first time ShowWindow is called.
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
	m_obDesktop = pSI->lpDesktop ? PyWinObject_FromTCHAR(pSI->lpDesktop) : NULL;
	m_obTitle = pSI->lpTitle ? PyWinObject_FromTCHAR(pSI->lpTitle) : NULL;
}

PySTARTUPINFO::~PySTARTUPINFO(void)
{
	Py_XDECREF(m_obStdIn);
	Py_XDECREF(m_obStdOut);
	Py_XDECREF(m_obStdErr);
	Py_XDECREF(m_obDesktop);
	Py_XDECREF(m_obTitle);
}

PyObject *gethandle(PyObject *obHandle, HANDLE h)
{
	if (obHandle) {
		Py_INCREF(obHandle);
		return obHandle;
	}
	return PyInt_FromLong((long)h);
}

PyObject *PySTARTUPINFO::getattr(PyObject *self, char *name)
{
	PySTARTUPINFO *pO = (PySTARTUPINFO *)self;
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
	if (strcmp("lpDesktop", name)==0) {
		PyObject *rc = pO->m_obDesktop ? pO->m_obDesktop : Py_None;
		Py_INCREF(rc);
		return rc;
	}
	// @prop string/None|lpTitle|
	if (strcmp("lpTitle", name)==0) {
		PyObject *rc = pO->m_obTitle ? pO->m_obTitle : Py_None;
		Py_INCREF(rc);
		return rc;
	}
	return PyMember_Get((char *)self, memberlist, name);
}

int sethandle(PyObject **pobHandle, HANDLE *ph, PyObject *v)
{
	int rc = 0;
	Py_XDECREF(*pobHandle);
	*pobHandle = NULL;
	*ph = NULL;
	if (v==Py_None)
		; // Do nothing!
	else if (PyHANDLE_Check(v)) {
		*pobHandle = v;
		if (PyWinObject_AsHANDLE(v, ph))
			Py_INCREF(v);
		else
			rc = -1;
	} else if (PyInt_Check(v)) {
		*ph = (HANDLE)PyInt_AsLong(v);
	} else {
		PyErr_SetString(PyExc_TypeError, "Invalid object type for HANDLE");
		rc = -1;
	}
	return rc;
}

int PySTARTUPINFO::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete STARTUPINFO attributes");
		return -1;
	}
	PySTARTUPINFO *pO = (PySTARTUPINFO *)self;
	if (strcmp("hStdInput", name)==0)
		return sethandle( &pO->m_obStdIn, &pO->m_startupinfo.hStdInput, v);

	if (strcmp("hStdOutput", name)==0)
		return sethandle( &pO->m_obStdOut, &pO->m_startupinfo.hStdOutput, v);

	if (strcmp("hStdError", name)==0)
		return sethandle( &pO->m_obStdErr, &pO->m_startupinfo.hStdError, v);

	if (strcmp("lpDesktop", name)==0) {
		if (PyWinObject_AsTCHAR(v, &pO->m_startupinfo.lpDesktop, TRUE)) {
			Py_XDECREF(pO->m_obDesktop);
			pO->m_obDesktop = v;
			Py_INCREF(v);
			return 0;
		} else
			return -1;
	}
	if (strcmp("lpTitle", name)==0) {
		if (PyWinObject_AsTCHAR(v, &pO->m_startupinfo.lpTitle, TRUE)) {
			Py_XDECREF(pO->m_obTitle);
			pO->m_obTitle = v;
			Py_INCREF(v);
		} else
			return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
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
#ifdef MS_WINCE
	if ($source!=Py_None) {
		PyErr_SetString(PyExc_TypeError, "STARTUPINFO is not supported on Windows CE");
		return NULL;
	}
	$target = NULL;
#else
	if (!PyWinObject_AsSTARTUPINFO($source, &$target, FALSE))
		return NULL;
#endif
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

#ifndef MS_WINCE

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
	PyObject *pyrc = PyEval_CallObject(ptd->m_obFunc, ptd->m_obArgs);
	delete ptd;
	if (pyrc==NULL) {
		fprintf(stderr, "Unhandled exception in beginthreadex created thread:\n");
		PyErr_Print();
		return -1;
	}
	int rc = 0;
	if (PyInt_Check(pyrc))
		rc = PyInt_AsLong(pyrc);
	Py_XDECREF(pyrc);
	return rc;
}

// @pyswig <o PyHANDLE>, int|beginthreadex|Creates a new thread
static PyObject *mybeginthreadex(PyObject *self, PyObject *args)
{
	PyObject *obFunc, *obArgs, *obSA;
	unsigned stackSize, flags;
	if (!PyArg_ParseTuple(args, "OiOOi", 
		&obSA, // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None
		&stackSize, // @pyparm int|stackSize||Stack size for the new thread, or zero for the default size.
		&obFunc, // @pyparm function|entryPoint||The thread function.
		&obArgs, // @pyparm tuple|args||Args passed to the function.
		&flags)) // @pyparm int|flags||
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

	PyEval_InitThreads();
	PythonThreadData *ptd = new PythonThreadData(obFunc, obArgs);
	unsigned long handle;
	unsigned tid;
	handle = _beginthreadex((void *)pSA, stackSize, ThreadEntryPoint, ptd, flags, &tid);
	if (handle==-1) {
		delete ptd;
		PyErr_SetFromErrno(PyExc_RuntimeError);
	}
	// @comm The result is a tuple of the thread handle and thread ID.
	PyObject *obHandle = PyWinObject_FromHANDLE((HANDLE)handle);
	PyObject *rc = Py_BuildValue("Oi", obHandle, tid);
	Py_DECREF(obHandle);
	return rc;
}

%}

%native (beginthreadex) mybeginthreadex;

#endif // MS_WINCE

// Wont expose ExitThread!!!  May leak all sorts of things!

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
	unsigned bufLen = 0;
	PyObject *keys = NULL, *vals = NULL;
	int envLength = PyMapping_Length(env);
	LPVOID result = NULL;
	WCHAR *pUCur;
	char *pACur;

	keys = PyMapping_Keys(env);
	vals = PyMapping_Values(env);
	if (!keys || !vals)
		goto done;

	for (i=0;i<envLength;i++) {
		PyObject *key = PyList_GetItem(keys, i); // no reference
		PyObject *val = PyList_GetItem(vals, i); // no ref.
		if (i==0) {
			if (PyString_Check(key)) {
				bIsUnicode = FALSE;
				bufLen += PyString_Size(key) + 1;
			} else if (PyUnicode_Check(key)) {
				bIsUnicode = TRUE;
				bufLen += PyUnicode_Size(key) + 1;
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
				bufLen += PyUnicode_Size(key) + 1;
			}
			else {
				if (!PyString_Check(key)) {
					PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
					goto done;

				}
				bufLen += PyString_Size(key) + 1;
			}
		}
		if (bIsUnicode) {
			if (!PyUnicode_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				goto done;
			}
			bufLen += PyUnicode_Size(val) + 2; // For the '=' and '\0'
		}
		else {
			if (!PyString_Check(val)) {
				PyErr_SetString(PyExc_TypeError, "All dictionary items must be strings, or all must be unicode");
				goto done;
			}
			bufLen += PyString_Size(val) + 2; // For the '=' and '\0'
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
			char *pTemp = PyString_AsString(key);
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
			char *pTemp = PyString_AsString(val);
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

#ifndef MS_WINCE
	if (bEnvIsUnicode)
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif //MS_WINCE

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
	PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(pi.dwProcessId));
	PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(pi.dwThreadId));
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
			// @flag CREATE_BREAKAWAY_FROM_JOB|Windows 2000: The child processes of a process associated with a job are not associated with the job. 
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
			// Windows 95/98: This flag is not valid if the new process is a 16-bit application. 
			// @flag DEBUG_ONLY_THIS_PROCESS|If this flag is not set and the calling process is being debugged, the new process becomes another process being debugged by the calling process's debugger. If the calling process is not a process being debugged, no debugging-related actions occur. 
			// @flag DETACHED_PROCESS|For console processes, the new process does not have access to the console of the parent process. The new process can call the AllocConsole function at a later time to create a new console. This flag cannot be used with the CREATE_NEW_CONSOLE flag. 
			
			
			// @flag ABOVE_NORMAL_PRIORITY_CLASS|Windows 2000: Indicates a process that has priority higher than NORMAL_PRIORITY_CLASS but lower than HIGH_PRIORITY_CLASS. 
			// @flag BELOW_NORMAL_PRIORITY_CLASS|Windows 2000: Indicates a process that has priority higher than IDLE_PRIORITY_CLASS but lower than NORMAL_PRIORITY_CLASS. 
			// @flag HIGH_PRIORITY_CLASS|Indicates a process that performs time-critical tasks. The threads of a high-priority class process preempt the threads of normal-priority or idle-priority class processes. An example is the Task List, which must respond quickly when called by the user, regardless of the load on the system. Use extreme care when using the high-priority class, because a CPU-bound application with a high-priority class can use nearly all available cycles. 
			// @flag IDLE_PRIORITY_CLASS|Indicates a process whose threads run only when the system is idle and are preempted by the threads of any process running in a higher priority class. An example is a screen saver. The idle priority class is inherited by child processes. 
			// @flag NORMAL_PRIORITY_CLASS|Indicates a normal process with no special scheduling needs. 
			// @flag REALTIME_PRIORITY_CLASS|Indicates a process that has the highest possible priority. The threads of a real-time priority class process preempt the threads of all other processes, including operating system processes performing important tasks. For example, a real-time process that executes for more than a very brief interval can cause disk caches not to flush or cause the mouse to be unresponsive. 
			
	
	PyObject *env, // @pyparm dictionary/None|newEnvironment||A dictionary of string or Unicode pairs to define the environment for the process, or None to inherit the current environment.
	TCHAR *INPUT_NULLOK, // @pyparm string|currentDirectory||current directory name, or None
	STARTUPINFO *lpStartupInfo // @pyparm <o PySTARTUPINFO>|startupinfo||a STARTUPINFO object that specifies how the main window for the new process should appear.

);

#ifndef MS_WINCE
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
	PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(pi.dwProcessId));
	PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(pi.dwThreadId));
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
	PyObject *env, // @pyparm None|newEnvironment||A dictionary of stringor Unicode pairs to define the environment for the process, or None to inherit the current environment.
	TCHAR *INPUT_NULLOK, // @pyparm string|currentDirectory||current directory name, or None
	STARTUPINFO *lpStartupInfo // @pyparm <o PySTARTUPINFO>|startupinfo||a STARTUPINFO object that specifies how the main window for the new process should appear.
);

#endif // MS_WINCE


#ifndef MS_WINCE
// @pyswig int, int|GetProcessAffinityMask|Gets a processor affinity mask for a specified thread.
// @rdesc The result is a tuple of (process affinity mask, system affinity mask)
BOOLAPI GetProcessAffinityMask(
  HANDLE hProcess,             // @pyparm <o PyHANDLE>|hProcess||handle to the process of interest
  DWORD *OUTPUT, // lpProcessAffinityMask
  DWORD *OUTPUT // lpSystemAffinityMask 
);
#endif // MS_WINCE

// @pyswig int|GetProcessVersion|Obtains the major and minor version numbers of the system on which a specified process expects to run.
DWORD GetProcessVersion(
	DWORD ProcessId  // @pyparm int|processId||identifier specifying the process of interest.
);

// @pyswig int|GetCurrentProcess|Retrieves a pseudo handle for the current process. 
DWORD GetCurrentProcess();

// @pyswig int|GetCurrentProcessId|Retrieves the process identifier of the calling process.
DWORD GetCurrentProcessId();

#ifndef MS_WINCE
// @pyswig <o PySTARTUPINFO>|GetStartupInfo|Retrieves the contents of the STARTUPINFO structure that was specified when the calling process was created.
void GetStartupInfo(
	STARTUPINFO *OUTPUT
);

// @pyswig int|GetPriorityClass|
DWORD GetPriorityClass(
	HANDLE hThread // @pyparm <o PyHANDLE>|handle||handle to the thread
);

#endif // MS_WINCE

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

#ifndef MS_WINCE

// @pyswig |SetPriorityClass|
BOOLAPI SetPriorityClass(
  	HANDLE hThread, // @pyparm <o PyHANDLE>|handle||handle to the process
	DWORD dwPriorityClass   // @pyparm int|dwPriorityClass||priority class value
);

// @pyswig int|SetThreadAffinityMask|Sets a processor affinity mask for a specified thread.
DWORD SetThreadAffinityMask (
  HANDLE hThread,             // @pyparm <o PyHANDLE>|handle||handle to the thread of interest
  DWORD dwThreadAffinityMask  // @pyparm int|mask||a thread affinity mask
);

%{
// This function does not exist on all platforms.
static PyObject *MySetThreadIdealProcessor( HANDLE hThread, DWORD dwIdealProc )
{
	DWORD (WINAPI *pfnSetThreadIdealProcessor)( HANDLE hThread, DWORD dwIdealProc ) = NULL;
	HMODULE hmod = GetModuleHandle("kernel32.dll");
	if (hmod)
		pfnSetThreadIdealProcessor = (DWORD (WINAPI *)( HANDLE hThread, DWORD dwIdealProc ))
			GetProcAddress(hmod, "SetThreadIdealProcessor");
	if (pfnSetThreadIdealProcessor==NULL)
		return PyWin_SetAPIError("SetThreadIdealProcessor", E_NOTIMPL);
	DWORD rc = (*pfnSetThreadIdealProcessor)(hThread, dwIdealProc);
	if (rc==-1)
		return PyWin_SetAPIError("SetThreadIdealProcessor", E_NOTIMPL);
	return PyInt_FromLong(rc);
}
%}

// @pyswig int|SetThreadIdealProcessor|Used to specify a preferred processor for a thread. The system schedules threads on their preferred processors whenever possible.
%name(SetThreadIdealProcessor) 
PyObject *MySetThreadIdealProcessor(
  HANDLE hThread,             // @pyparm <o PyHANDLE>|handle||handle to the thread of interest
  DWORD dwIdealProcessor  // @pyparm int|dwIdealProcessor||ideal processor number
);

%{
// Appears to be some problem with the optimizer here, so I just leave it off!
#pragma optimize ("", off)
// This function does not exist on all platforms.
static PyObject *MySetProcessAffinityMask( HANDLE hThread, DWORD dwMask )
{
	DWORD (WINAPI *pfnSetProcessAffinityMask)( HANDLE hThread, DWORD dwMask ) = NULL;
	HMODULE hmod = GetModuleHandle("kernel32.dll");
	if (hmod)
		pfnSetProcessAffinityMask = (DWORD (WINAPI *)( HANDLE hThread, DWORD dwMask ))
			GetProcAddress(hmod, "SetProcessAffinityMask");
	if (pfnSetProcessAffinityMask==NULL)
		return PyWin_SetAPIError("SetProcessAffinityMask", E_NOTIMPL);
	BOOL ok = (*pfnSetProcessAffinityMask)(hThread, dwMask);
	if (!ok)
		return PyWin_SetAPIError("SetProcessAffinityMask");
	Py_INCREF(Py_None);
	return Py_None;
}
#pragma optimize ("", on)
%}

// @pyswig |SetProcessAffinityMask|Sets a processor affinity mask for a specified process.
%name(SetProcessAffinityMask)
PyObject *MySetProcessAffinityMask (
  HANDLE hThread,             // @pyparm <o PyHANDLE>|handle||handle to the process of interest
  DWORD dwThreadAffinityMask  // @pyparm int|mask||a thread affinity mask
);

#endif // MS_WINCE

// Special result handling for SuspendThread and ResumeThread
%typedef DWORD DWORD_SR_THREAD
%typemap(python,out) DWORD_SR_THREAD {
	$target = PyInt_FromLong($source);
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

#define CREATE_SUSPENDED CREATE_SUSPENDED 

#ifndef MS_WINCE
#define MAXIMUM_PROCESSORS MAXIMUM_PROCESSORS 
#endif // MS_WINCE

#define THREAD_PRIORITY_ABOVE_NORMAL THREAD_PRIORITY_ABOVE_NORMAL // Indicates 1 point above normal priority for the priority class. 
#define THREAD_PRIORITY_BELOW_NORMAL THREAD_PRIORITY_BELOW_NORMAL // Indicates 1 point below normal priority for the priority class. 
#define THREAD_PRIORITY_HIGHEST THREAD_PRIORITY_HIGHEST // Indicates 2 points above normal priority for the priority class. 
#define THREAD_PRIORITY_IDLE THREAD_PRIORITY_IDLE // Indicates a base priority level of 1 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 16 for REALTIME_PRIORITY_CLASS processes. 
#define THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST // Indicates 2 points below normal priority for the priority class. 
#define THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL // Indicates normal priority for the priority class. 
#define THREAD_PRIORITY_TIME_CRITICAL THREAD_PRIORITY_TIME_CRITICAL // Indicates a base priority level of 15 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 31 for REALTIME_PRIORITY_CLASS processes. 

#ifndef MS_WINCE
#define CREATE_DEFAULT_ERROR_MODE CREATE_DEFAULT_ERROR_MODE // The new process does not inherit the error mode of the calling process. Instead, CreateProcess gives the new process the current default error mode. An application sets the current default error mode by calling SetErrorMode.
// This flag is particularly useful for multi-threaded shell applications that run with hard errors disabled. 

#define CREATE_NEW_CONSOLE CREATE_NEW_CONSOLE // The new process has a new console, instead of inheriting the parent's console. This flag cannot be used with the DETACHED_PROCESS flag.

#define CREATE_NEW_PROCESS_GROUP CREATE_NEW_PROCESS_GROUP // The new process is the root process of a new process group. The process group includes all processes that are descendants of this root process. The process identifier of the new process group is the same as the process identifier, which is returned in the lpProcessInformation parameter. Process groups are used by the GenerateConsoleCtrlEvent function to enable sending a ctrl+c or ctrl+break signal to a group of console processes. 

#define CREATE_SEPARATE_WOW_VDM CREATE_SEPARATE_WOW_VDM // Windows NT: This flag is valid only when starting a 16-bit Windows-based application. If set, the new process is run in a private Virtual DOS Machine (VDM). By default, all 16-bit Windows-based applications are run as threads in a single, shared VDM. The advantage of running separately is that a crash only kills the single VDM; any other programs running in distinct VDMs continue to function normally. Also, 16-bit Windows-based applications that are run in separate VDMs have separate input queues. That means that if one application hangs momentarily, applications in separate VDMs continue to receive input. The disadvantage of running separately is that it takes significantly more memory to do so. You should use this flag only if the user requests that 16-bit applications should run in them own VDM. 

#define CREATE_SHARED_WOW_VDM CREATE_SHARED_WOW_VDM // Windows NT: The flag is valid only when starting a 16-bit Windows-based application. If the DefaultSeparateVDM switch in the Windows section of WIN.INI is TRUE, this flag causes the CreateProcess function to override the switch and run the new process in the shared Virtual DOS Machine. 

#define CREATE_UNICODE_ENVIRONMENT CREATE_UNICODE_ENVIRONMENT // If set, the environment block pointed to by lpEnvironment uses Unicode characters. If clear, the environment block uses ANSI characters. 
#endif // MS_WINCE

#define DEBUG_PROCESS DEBUG_PROCESS // If this flag is set, the calling process is treated as a debugger, and the new process is a process being debugged. The system notifies the debugger of all debug events that occur in the process being debugged.
// If you create a process with this flag set, only the calling thread (the thread that called CreateProcess) can call the WaitForDebugEvent function.
// Windows 95 and Windows 98: This flag is not valid if the new process is a 16-bit application. 
 
#define DEBUG_ONLY_THIS_PROCESS DEBUG_ONLY_THIS_PROCESS // If not set and the calling process is being debugged, the new process becomes another process being debugged by the calling process's debugger. If the calling process is not a process being debugged, no debugging-related actions occur. 

#ifndef MS_WINCE
#define DETACHED_PROCESS DETACHED_PROCESS // For console processes, the new process does not have access to the console of the parent process. The new process can call the AllocConsole function at a later time to create a new console. This flag cannot be used with the CREATE_NEW_CONSOLE flag. 

#ifdef ABOVE_NORMAL_PRIORITY_CLASS
#define ABOVE_NORMAL_PRIORITY_CLASS ABOVE_NORMAL_PRIORITY_CLASS // Windows 2000: Indicates a process that has priority above NORMAL_PRIORITY_CLASS but below HIGH_PRIORITY_CLASS.
#endif /* ABOVE_NORMAL_PRIORITY_CLASS */
#ifdef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS BELOW_NORMAL_PRIORITY_CLASS // Windows 2000: Indicates a process that has priority above IDLE_PRIORITY_CLASS but below NORMAL_PRIORITY_CLASS.
#endif
#define HIGH_PRIORITY_CLASS HIGH_PRIORITY_CLASS // Indicates a process that performs time-critical tasks that must be executed immediately for it to run correctly. The threads of a high-priority class process preempt the threads of normal-priority or idle-priority class processes. An example is the Task List, which must respond quickly when called by the user, regardless of the load on the system. Use extreme care when using the high-priority class, because a high-priority class CPU-bound application can use nearly all available cycles. 
#define IDLE_PRIORITY_CLASS IDLE_PRIORITY_CLASS // Indicates a process whose threads run only when the system is idle and are preempted by the threads of any process running in a higher priority class. An example is a screen saver. The idle priority class is inherited by child processes. 
#define NORMAL_PRIORITY_CLASS NORMAL_PRIORITY_CLASS // Indicates a normal process with no special scheduling needs. 
#define REALTIME_PRIORITY_CLASS REALTIME_PRIORITY_CLASS // Indicates a process that has the highest possible priority. The threads of a real-time priority class process preempt the threads of all other processes, including operating system processes performing important tasks. For example, a real-time process that executes for more than a very brief interval can cause disk caches not to flush or cause the mouse to be unresponsive. 

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

#endif // MS_WINCE
