#include <windows.h>
#include "wisedll.h"
#include "Python.h"
#include "malloc.h"
#include <locale.h>

HINSTANCE hDllInst;

void GetVariable(LPDLLCALLPARAMS lpDllParams,char *szVariable,char *szValue);
void SetVariable(LPDLLCALLPARAMS lpDllParams,char *szVariable,char *szValue);
void PyWise_Initialize(void);
void PyWise_Finalize(void);

// Only 1 thread!
LPDLLCALLPARAMS g_params;
char *g_exist_locale = NULL; // We need to mess with the locale.
extern HWND hProgressDlg;   // The Progress dialog window handle


PyObject *PyWise_Error = NULL;

// win16
int CALLBACK LibMain(HINSTANCE hInst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
  hDllInst = hInst;
  return(1);
}
// Win32
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH: 
			hDllInst = hInstance;
			break;
	}
	return TRUE;
}


void ReportError(LPDLLCALLPARAMS lpDllParams, char *msg)
{
	char buf[512];
	wsprintf(buf, "Could not call the Python installer - Error %s", msg);
	MessageBox((HWND)lpDllParams->hWnd, buf, "Error", MB_OK);
}
/*--
GetPythonErrorMessage

Assuming a Python error has occurred in the current thread state,
return a "char *" which is a formatted traceback, or NULL if the
traceback can not be obtained.

If a non-NULL value is returned, it should be passed to "free()" when
it is no longer needed (the string is allocated using strdup!)

Uses the built-in library modules cStringIO and traceback, so if the
core Python library is not setup correctly, this will not work.

Regardless of if the function succeeds, the exception state id
always restored to how it was when we were entered.  The caller
may still wish to call PyErr_Clear() to clean it all up.

Mark Hammond - Jan 1998
--*/
#define GPEM_ERROR(what) {errorMsg = "<Error getting traceback - " ## what ## ">";goto done;}


char *GetPythonErrorMessage()
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
	PyObject *exc_typ, *exc_val, *exc_tb;
	/* Fetch the error state now before we cruch it */
	PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);

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
	obFuncTB = PyObject_GetAttrString(modTB, "print_exception");
	if (obFuncTB==NULL) GPEM_ERROR("cant find traceback.print_exception");
	argsTB = Py_BuildValue("OOOOO", 
			exc_typ ? exc_typ : Py_None, 
			exc_val ? exc_val : Py_None,
			exc_tb  ? exc_tb  : Py_None,
			Py_None, 
			obStringIO);
	if (argsTB==NULL) GPEM_ERROR("cant make print_exception arguments");

	obResult = PyObject_CallObject(obFuncTB, argsTB);
	if (obResult==NULL) GPEM_ERROR("traceback.print_exception() failed");

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

	/* Restore the exception state */
	PyErr_Restore(exc_typ, exc_val, exc_tb);
	return result;
}

void ReportPythonError(LPDLLCALLPARAMS lpDllParams, char *msg)
{
	char buf[4096];
	wsprintf(buf, "Could not call the Python installer - Python Error %s\n", msg);
	char *err = GetPythonErrorMessage();
	if (err) {
		strcat(buf, err);
		free(err);
	} else {
		strcat(buf, "<No error info is available>");
	}
	MessageBox((HWND)lpDllParams->hWnd, buf, "Python Error", MB_OK);
}

BOOL CallObject(LPDLLCALLPARAMS lpDllParams)
{
	char *strVal = (char *)alloca(strlen(lpDllParams->lpszParam)+1);
	strcpy(strVal, lpDllParams->lpszParam);
	char *realArgs = strchr(strVal, '|');
	if (realArgs!=NULL) {
		*realArgs = '\0';
		++realArgs;
	}
	char *sep = strrchr(strVal, '\\');
	char *fname;
	if (sep) {
		*sep = '\0';
		fname = sep+1;
		// Stick the Path on the Python sys.path.
		PyObject *obPath = PySys_GetObject("path");
		if (obPath==NULL) {
				ReportPythonError(lpDllParams, "adding directory to path");
				return FALSE;
		}
		PyObject *obNew = PyString_FromString(strVal);
		if (obNew==NULL) {
			Py_DECREF(obPath);
			ReportPythonError(lpDllParams, "allocating string for sys.path");
			return FALSE;
		}
		PyList_Append(obPath, obNew);
		Py_DECREF(obNew);
	} else {
		fname = strVal;
	}
	// Find the last "." in the name, and assume it is a module name.
	char *classNamePos = strrchr(fname, '.');
	if (classNamePos==NULL) {
		ReportError(lpDllParams, "locating module");
		return FALSE;
	}
	*classNamePos = '\0';
	++classNamePos; // skip the '.'
	PyObject *module = PyImport_ImportModule(fname);
	if (module==NULL) {
		ReportPythonError(lpDllParams, "importing module");
		return FALSE;
	}
	PyObject *pyclass = PyObject_GetAttrString(module, classNamePos);
	Py_DECREF(module);
	if (pyclass==NULL) {
		ReportPythonError(lpDllParams, "getting class/function");
		return FALSE;
	}
	PyObject *args = Py_BuildValue("iiiz",
		lpDllParams->hWnd,         // Handle to main window
		lpDllParams->bRunMode,     // The installation mode
		lpDllParams->fLogFile,     // A file handle to the log file
		realArgs);    // String parameter from Wise Installation System

	if (args==NULL) {
		Py_DECREF(pyclass);
		ReportPythonError(lpDllParams, "Making arguments");
		return FALSE;
	}
	PyObject *result = PyObject_CallObject(pyclass, args);
	Py_DECREF(pyclass);
	Py_DECREF(args);
	if (result==NULL) {
		ReportPythonError(lpDllParams, "calling function failed");
		return FALSE;
	}
	Py_DECREF(result);
	return TRUE;
}

// Returns FALSE if worked OK!
__declspec(dllexport) BOOL CALLBACK WiseRun_SimpleFile(LPDLLCALLPARAMS lpDllParams)
{
	if (lpDllParams->lpszParam) {
		g_params = lpDllParams;
		PyWise_Initialize();
		FILE *fp = fopen(lpDllParams->lpszParam, "r");
		int result = -1;
		if (fp) {
			result = PyRun_SimpleFile(fp, lpDllParams->lpszParam);
		}
		PyWise_Finalize();
		return (result==0) ? FALSE : TRUE;
	}
	return TRUE;
}

__declspec(dllexport) BOOL CALLBACK Wise_CallObject(LPDLLCALLPARAMS lpDllParams)
{
	if (lpDllParams->lpszParam) {
		g_params = lpDllParams;
		PyWise_Initialize();
		BOOL ok = CallObject(lpDllParams);
		PyWise_Finalize();
		return !ok;
	}
	return TRUE;
}


// GetCPU: Uses the new Win32 API call to get the CPU type including
//         the Pentium processor. You must pass the name of the variable
//         to save the name of the cpu into in the parameter field.

__declspec(dllexport) BOOL CALLBACK GetCPU(LPDLLCALLPARAMS lpDllParams)
{
   SYSTEM_INFO SystemInfo;
   GetSystemInfo(&SystemInfo);
   if (lpDllParams->lpszParam) {
      switch (SystemInfo.dwProcessorType) {
       case PROCESSOR_INTEL_386: SetVariable(lpDllParams,lpDllParams->lpszParam,"I386"); break;
       case PROCESSOR_INTEL_486: SetVariable(lpDllParams,lpDllParams->lpszParam,"I486"); break;
       case PROCESSOR_INTEL_PENTIUM: SetVariable(lpDllParams,lpDllParams->lpszParam,"PENTIUM"); break;
       case PROCESSOR_MIPS_R4000: SetVariable(lpDllParams,lpDllParams->lpszParam,"R4000"); break;
       case PROCESSOR_ALPHA_21064: SetVariable(lpDllParams,lpDllParams->lpszParam,"ALPHA"); break;
      }
   }
   return FALSE;
}

// GetVariable: Returns the value of a variable.
//
// lpDllParams  Parameter structure passed from Wise Installation
// szVariable   Name of the variable (without %'s) to get value for
// szValue      String that will hold the variables value

void GetVariable(LPDLLCALLPARAMS lpDllParams,char *szVariable,char *szValue)
{
   WORD i;
   char szVar[32];

   *szVar = '%';
   lstrcpy(&szVar[1],szVariable);
   lstrcat(szVar,"%");
   for (i = 0 ; (i < lpDllParams->wCurrReps) &&
      (lstrcmp(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar) != 0) ; i++) ;
   if (i < lpDllParams->wCurrReps) {
      lstrcpy(szValue,&lpDllParams->lpszRepStr[i * lpDllParams->wRepStrWidth]);
   } else *szValue = '\0';
}

// SetVariable: Sets/Creates a variable.
//
// lpDllParams  Parameter structure passed from Wise Installation
// szVariable   Name of the variable (without %'s) to set value for
// szValue      String that contains the variables new value

void SetVariable(LPDLLCALLPARAMS lpDllParams,char *szVariable,char *szValue)
{
   WORD i;
   char szVar[32];

   *szVar = '%';
   lstrcpy(&szVar[1],szVariable);
   lstrcat(szVar,"%");
   for (i = 0 ; (i < lpDllParams->wCurrReps) &&
      (lstrcmp(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar) != 0) ; i++) ;
   if (i >= lpDllParams->wCurrReps) {
      if (i >= lpDllParams->wMaxReplaces) return; // Too many variables
      lstrcpy(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar);
      lpDllParams->wCurrReps++;
   }
   lstrcpy(&lpDllParams->lpszRepStr[i * lpDllParams->wRepStrWidth],szValue);
}

//////////////////////////////////////////////////////
extern HWND ProgressInit(HWND parent);
extern void ProgressDone();
extern void ProgressHide();
extern void ProgressSetRange(int iMax, int iMin);
extern void ProgressSetStep(int iStep);
extern BOOL ProgressStepIt(char *text = NULL);
extern BOOL ProgressSetText(char *text);
extern BOOL ProgressSetTitle(char *text);


PyObject *PyProgressInit(PyObject *self, PyObject *args)
{
	int parent = g_params->hWnd;
	if (!PyArg_ParseTuple(args, "|i", &parent))
		return NULL;
	HWND ret = ProgressInit((HWND)parent);
	return PyInt_FromLong((long)ret);
}
PyObject *PyProgressDone(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	ProgressDone();
	Py_INCREF(Py_None);
	return Py_None;
}
PyObject *PyProgressHide(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	ProgressHide();
	Py_INCREF(Py_None);
	return Py_None;
}
PyObject *PyProgressSetStep(PyObject *self, PyObject *args)
{
	int step;
	if (!PyArg_ParseTuple(args, "i", &step))
		return NULL;
	ProgressSetStep(step);
	Py_INCREF(Py_None);
	return Py_None;
}
PyObject *PyProgressSetRange(PyObject *self, PyObject *args)
{
	int min, max;
	if (!PyArg_ParseTuple(args, "ii", &min, &max))
		return NULL;
	ProgressSetRange(min, max);
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *PyProgressStepIt(PyObject *self, PyObject *args)
{
	char *text = NULL;
	if (!PyArg_ParseTuple(args, "|z", &text))
		return NULL;
	BOOL bCont = ProgressStepIt(text);
	return PyInt_FromLong(bCont);
}
PyObject *PyProgressSetText(PyObject *self, PyObject *args)
{
	char *text;
	if (!PyArg_ParseTuple(args, "z", &text))
		return NULL;
	BOOL ok = ProgressSetText(text);
	return PyInt_FromLong(ok);
}
PyObject *PyProgressSetTitle(PyObject *self, PyObject *args)
{
	char *text;
	if (!PyArg_ParseTuple(args, "z", &text))
		return NULL;
	BOOL ok = ProgressSetTitle(text);
	return PyInt_FromLong(ok);
}


PyObject *PySetVariable(PyObject *self, PyObject *args)
{
	char *var, *val;
	if (!PyArg_ParseTuple(args, "ss", &var, &val))
		return NULL;
	SetVariable(g_params, var, val);
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *PyGetVariable(PyObject *self, PyObject *args)
{
	char *var;
	if (!PyArg_ParseTuple(args, "s", &var))
		return NULL;
	char val[4096];
	GetVariable(g_params, var, val);
	return PyString_FromString(val);
}

PyObject *PyMessageBox(PyObject *self, PyObject *args)
{
	int style = MB_OK;
	char titleBuf[256];
	char *title = NULL;
	char *msg;
	int hwnd = (int)hProgressDlg;
	if (hwnd==NULL) hwnd = g_params->hWnd;
	if (!PyArg_ParseTuple(args, "s|zii", &msg, &title, &style, &hwnd))
		return NULL;
	if (title==NULL) {
		GetWindowText((HWND)hwnd, titleBuf, sizeof(titleBuf));
		title = titleBuf;
	}
	int rc = MessageBox((HWND)hwnd, msg, title, style);
	return PyInt_FromLong(rc);
}

PyObject *PyDebugBreak(PyObject *self, PyObject *args)
{
	DebugBreak();
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *PyWriteToLog(PyObject *self, PyObject *args)
{
	char *str;
	int sizeStr;
	if (!PyArg_ParseTuple(args, "s#", &str, &sizeStr))
		return NULL;
	if (g_params==NULL || g_params->fLogFile==0) {
		PyErr_SetString(PyWise_Error, "The log file handle is invalid");
		return NULL;
	}

	DWORD numWritten;
	BOOL ok = WriteFile((HANDLE)g_params->fLogFile, str, sizeStr, &numWritten, NULL);
	return PyInt_FromLong(GetLastError());
}

static struct PyMethodDef pywise_functions[] = {
	{"WriteToLog",      PyWriteToLog, 1},
	{"DebugBreak",      PyDebugBreak, 1},
	{"MessageBox",      PyMessageBox, 1},
	{"ProgressInit",    PyProgressInit, 1},
	{"ProgressDone",    PyProgressDone, 1},
	{"ProgressHide",    PyProgressHide, 1},
	{"ProgressSetRange",PyProgressSetRange, 1},
	{"ProgressSetStep", PyProgressSetStep, 1},
	{"ProgressSetText", PyProgressSetText, 1},
	{"ProgressSetTitle",PyProgressSetTitle, 1},
	{"ProgressStepIt",  PyProgressStepIt, 1},
	{"GetVariable",     PyGetVariable, 1},
	{"SetVariable",     PySetVariable, 1},
	{NULL}
};

int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyLong_FromLong(value);
	if (!okey || !oval) {
		Py_XDECREF(okey);
		Py_XDECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	Py_XDECREF(okey);
	Py_XDECREF(oval);
	return rc;
}
#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, (long)tok)) return

void PyWise_Initialize(void)
{
  // Python insists on the default locale being set...
  char *exist_locale = setlocale(LC_ALL, "C");
  if (g_exist_locale) free(g_exist_locale);
  g_exist_locale = exist_locale ? strdup(exist_locale) : NULL;

  Py_Initialize();

  PyObject *dict, *module;
  module = Py_InitModule("pywise", pywise_functions);
  dict = PyModule_GetDict(module);
  PyWise_Error = PyErr_NewException("pywise.error", NULL, NULL);
  PyDict_SetItemString(dict, "error", PyWise_Error);
  int debug = 
#ifdef _DEBUG
   1;
#else
   0;
#endif
  int rc;
  AddConstant(dict, "_DEBUG", debug);
  ADD_CONSTANT(MB_ABORTRETRYIGNORE);
  ADD_CONSTANT(MB_OKCANCEL);
  ADD_CONSTANT(MB_RETRYCANCEL);
  ADD_CONSTANT(MB_YESNOCANCEL);
  ADD_CONSTANT(MB_OK);
  ADD_CONSTANT(MB_YESNO);
  ADD_CONSTANT(MB_YESNO);
  ADD_CONSTANT(MB_ICONHAND);
  ADD_CONSTANT(MB_ICONSTOP);
  ADD_CONSTANT(MB_ICONQUESTION);
  ADD_CONSTANT(MB_ICONEXCLAMATION);
  ADD_CONSTANT(MB_ICONASTERISK);
  ADD_CONSTANT(MB_ICONINFORMATION);
  ADD_CONSTANT(IDOK);
  ADD_CONSTANT(IDYES);
  ADD_CONSTANT(IDNO);
  ADD_CONSTANT(IDRETRY);
  ADD_CONSTANT(IDCANCEL);
  ADD_CONSTANT(HKEY_LOCAL_MACHINE);
  ADD_CONSTANT(HKEY_CURRENT_USER);
  ADD_CONSTANT(HKEY_CLASSES_ROOT);
  ADD_CONSTANT(HKEY_USERS);
}

void PyWise_Finalize(void)
{
  ProgressDone();
  Py_Finalize();
  setlocale(LC_ALL, g_exist_locale);
  free(g_exist_locale);
  g_exist_locale = NULL;

  PyWise_Error = NULL;
}
