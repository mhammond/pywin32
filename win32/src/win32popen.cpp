#include "windows.h"

// @doc

#include "Python.h"
#include "malloc.h"
#include "io.h"
#include "fcntl.h"
#include "PyWinTypes.h"

#define DllExport   _declspec(dllexport)

extern bool g_fUsingWin9x;
extern CHAR g_szModulePath[];	

// These tell _PyPopen() wether to return 1, 2, or 3 file objects.
#define POPEN_1 1
#define POPEN_2 2
#define POPEN_3 3
#define POPEN_4 4

static PyObject *_PyPopen(char *, int, int);

static int _PyPclose(FILE *file);

/*
 * Internal dictionary mapping popen* file pointers to process handles,
 * in order to maintain a link to the process handle until the file is
 * closed, at which point the process exit code is returned to the caller.
 */
static PyObject *_PyPopenProcs = NULL;


// @pymethod pipe|win32pipe|popen|Popen that works from a GUI.
// @rdesc The result of this function is a pipe (file) connected to the
// processes stdin or stdout, depending on the requested mode.
PyObject *PyPopen(PyObject *self, PyObject *args)
{
	char *cmdstring;
	char *mode="r";
	int bufsize=-1;
	PyObject *f,*s;
	int tm=0;
  
	if (!PyArg_ParseTuple(args, "s|s:popen",
						  &cmdstring,    // @pyparm string|cmdstring||The cmdstring to pass to the shell
						  &mode))        // @pyparm string|mode||Either 'r' or 'w'
		return NULL;

	s = PyTuple_New(0);
      
	if (*mode == 'r')
		tm = _O_RDONLY;
	else if (*mode != 'w')
	{
		PyErr_SetString(PyExc_ValueError, "mode must be 'r' or 'w'");
		return NULL;
	}
	else
		tm = _O_WRONLY;
     
	if (*(mode+1) == 't')
		f = _PyPopen(cmdstring, tm | _O_TEXT , POPEN_1);
	else if (*(mode+1) == 'b')
		f = _PyPopen(cmdstring, tm | _O_BINARY , POPEN_1);
	else
		f = _PyPopen(cmdstring, tm | _O_TEXT, POPEN_1);
  
	return f;
  
}

// @pymethod (pipe, pipe)|win32pipe|popen2|Variation on <om win32pipe.popen>
// @rdesc The result of this function is a pipe (file) connected to the
// processes stdin, and a pipe connected to the processes stdout.
PyObject *PyPopen2(PyObject *self, PyObject  *args)
{
	char *cmdstring;
	char *mode="t";
	PyObject *f;
	int tm=0;
  
	if (!PyArg_ParseTuple(args, "s|s:popen2",
						  &cmdstring,    // @pyparm string|cmdstring||The cmdstring to pass to the shell
						  &mode))        // @pyparm string|mode||Either 't' or 'b'
		return NULL;
  
	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b')
    {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
    }
	else
		tm = _O_BINARY;
  
	f = _PyPopen(cmdstring, tm , POPEN_2);
  
	return f;
}

// @pymethod (pipe, pipe, pipe)|win32pipe|popen3|Variation on <om win32pipe.popen>
// @rdesc The result of this function is 3 pipes - the processes stdin, stdout and stderr
PyObject *PyPopen3(PyObject *self, PyObject  *args)
{
	char *cmdstring;
	char *mode="t";
	PyObject *f;
	int tm=0;
  
	if (!PyArg_ParseTuple(args, "s|s:Popen3",
						  &cmdstring,    // @pyparm string|cmdstring||The cmdstring to pass to the shell
						  &mode))        // @pyparm string|mode||Either 't' or 'b'
		return NULL;
  
	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b')
    {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
    }
	else
		tm = _O_BINARY;
  
	f = _PyPopen(cmdstring, tm , POPEN_3);
  
	return f;
}

// @pymethod (pipe, pipe)|win32pipe|popen4|Variation on <om win32pipe.popen>
// @rdesc The result of this function is 2 pipes - the processes stdin, 
// and stdout+stderr combined as a single pipe.
PyObject *PyPopen4(PyObject *self, PyObject  *args)
{
	char *cmdstring;
	char *mode="t";
	PyObject *f;
	int tm=0;
  
	if (!PyArg_ParseTuple(args, "s|s:popen4",
						  &cmdstring,    // @pyparm string|cmdstring||The cmdstring to pass to the shell
						  &mode))        // @pyparm string|mode||Either 't' or 'b'
		return NULL;
  
	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b')
    {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
    }
	else
		tm = _O_BINARY;
  
	f = _PyPopen(cmdstring, tm , POPEN_4);
  
	return f;
}
												
static int _PyPopenCreateProcess(char *cmdstring, FILE *file,
				 HANDLE hStdin,
				 HANDLE hStdout,
				 HANDLE hStderr)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	char *s1,*s2, *s3=" /c ";
	const char *szConsoleSpawn = "win32popenWin9x.exe \"";
	int i;
	int x;

	if (i = GetEnvironmentVariable("COMSPEC",NULL,0))
    {
		s1 = (char *)_alloca(i);
		if (!(x = GetEnvironmentVariable("COMSPEC", s1, i)))
			return x;
		if (!g_fUsingWin9x)
		{
			x = i + strlen(s3) + strlen(cmdstring) + 1;
			s2 = (char *)_alloca(x);
			ZeroMemory(s2, x);
			sprintf(s2, "%s%s%s", s1, s3, cmdstring);
		}
		else
		{
			//
			// Oh gag, we're on Win9x. Use the workaround listed in
			// KB: Q150956
			//
			x = i + strlen(s3) + strlen(cmdstring) + 1 + strlen(g_szModulePath) + 
				strlen(szConsoleSpawn) + 1;
			s2 = (char *)_alloca(x);
			ZeroMemory(s2, x);
			sprintf(
				s2,
				"%s%s%s%s%s\"",
				g_szModulePath,
				szConsoleSpawn,
				s1,
				s3,
				cmdstring);
		}
    }
	// Could be an else here to try cmd.exe / command.com in the path
	// Now we'll just error out..
	else
		return -1;
  
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.hStdInput = hStdin;
	siStartInfo.hStdOutput = hStdout;
	siStartInfo.hStdError = hStderr;
	siStartInfo.wShowWindow = SW_HIDE;
	if ( CreateProcess(NULL,
					   s2,
					   NULL,
					   NULL,
					   TRUE,
					   CREATE_NEW_CONSOLE,
					   NULL,
					   NULL,
					   &siStartInfo,
					   &piProcInfo) ) {
		// Close the handles now so anyone waiting is woken.
		CloseHandle(piProcInfo.hThread);

		/*
		 * Try to insert our process handle into the internal
		 * dictionary so we can find it later when trying 
		 * to close this file.
		 */
		if (!_PyPopenProcs)
		   _PyPopenProcs = PyDict_New();
		if (_PyPopenProcs) {
		   PyObject *hProcessObj, *fileObj;

		   hProcessObj = PyLong_FromVoidPtr(piProcInfo.hProcess);
		   fileObj = PyLong_FromVoidPtr(file);

		   if (!hProcessObj || !fileObj ||
		       PyDict_SetItem(_PyPopenProcs,
				      fileObj, hProcessObj) < 0) {
		      /* Insert failure - close handle to prevent leak */
		      CloseHandle(piProcInfo.hProcess);
		   }
		}
		return TRUE;
	}
	return FALSE;
}


// The following code is based off of KB: Q190351
static PyObject *_PyPopen(char *cmdstring, int mode, int n)
{
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr,
		hChildStderrRd, hChildStderrWr, hChildStdinWrDup, hChildStdoutRdDup,
		hChildStderrRdDup; // hChildStdoutWrDup;
      
	SECURITY_ATTRIBUTES saAttr;
	BOOL fSuccess;
	int fd1, fd2, fd3;
	FILE *f1, *f2, *f3;
	PyObject *f;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
		return PyWin_SetAPIError("CreatePipe");


	// Create new output read handle and the input write handle. Set
	// the inheritance properties to FALSE. Otherwise, the child inherits
	// the these handles; resulting in non-closeable handles to the pipes
	// being created.
	fSuccess = DuplicateHandle( GetCurrentProcess(), hChildStdinWr,
								GetCurrentProcess(), &hChildStdinWrDup, 0,
								FALSE,
								DUPLICATE_SAME_ACCESS);
	if (!fSuccess)
		return PyWin_SetAPIError("DuplicateHandle");

	// Close the inheritable version of ChildStdin
	// that we're using.
	CloseHandle(hChildStdinWr);

	if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
		return PyWin_SetAPIError("CreatePipe");

	fSuccess = DuplicateHandle( GetCurrentProcess(), hChildStdoutRd,
								GetCurrentProcess(), &hChildStdoutRdDup, 0,
								FALSE,
								DUPLICATE_SAME_ACCESS);
	if (!fSuccess)
		return PyWin_SetAPIError("DuplicateHandle");

	// Close the inheritable version of ChildStdout
	// that we're using.
	CloseHandle(hChildStdoutRd);

	if (n != POPEN_4)
	{
		if (!CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0))
			return PyWin_SetAPIError("CreatePipe");
		fSuccess = DuplicateHandle( GetCurrentProcess(), hChildStderrRd,
									GetCurrentProcess(), &hChildStderrRdDup, 0,
									FALSE,
									DUPLICATE_SAME_ACCESS);
		if (!fSuccess)
			return PyWin_SetAPIError("DuplicateHandle");
		// Close the inheritable version of ChildStdErr that we're using.
		CloseHandle(hChildStderrRd);
	}
      
	switch (n)
	{
	case POPEN_1:
	    switch (mode & (_O_RDONLY | _O_TEXT | _O_BINARY | _O_WRONLY))
		{
		case _O_WRONLY | _O_TEXT:
			// Case for writing to child Stdin in text mode.
			fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
			f1 = _fdopen(fd1, "w");
			f = PyFile_FromFile(f1, cmdstring, "w", _PyPclose);
			PyFile_SetBufSize(f, 0);
			// We don't care about these pipes anymore, so close them.
			CloseHandle(hChildStdoutRdDup);
			CloseHandle(hChildStderrRdDup);
			break;

		case _O_RDONLY | _O_TEXT:
			// Case for reading from child Stdout in text mode.
			fd1 = _open_osfhandle((long)hChildStdoutRdDup, mode);
			f1 = _fdopen(fd1, "r");
			f = PyFile_FromFile(f1, cmdstring, "r", _PyPclose);
			PyFile_SetBufSize(f, 0);
			// We don't care about these pipes anymore, so close them.
			CloseHandle(hChildStdinWrDup);
			CloseHandle(hChildStderrRdDup);
			break;

		case _O_RDONLY | _O_BINARY:
			// Case for readinig from child Stdout in binary mode.
			fd1 = _open_osfhandle((long)hChildStdoutRdDup, mode);
			f1 = _fdopen(fd1, "rb");
			f = PyFile_FromFile(f1, cmdstring, "rb", _PyPclose);
			PyFile_SetBufSize(f, 0);
			// We don't care about these pipes anymore, so close them.
			CloseHandle(hChildStdinWrDup);
			CloseHandle(hChildStderrRdDup);
			break;

		case _O_WRONLY | _O_BINARY:
			// Case for writing to child Stdin in binary mode.
			fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
			f1 = _fdopen(fd1, "wb");
			f = PyFile_FromFile(f1, cmdstring, "wb", _PyPclose);
			PyFile_SetBufSize(f, 0);
			// We don't care about these pipes anymore, so close them.
			CloseHandle(hChildStdoutRdDup);
			CloseHandle(hChildStderrRdDup);
			break;
		}
		break;
	
	case POPEN_2:
	case POPEN_4:
	{
	    char *m1, *m2;
	    PyObject *p1, *p2;
	    
	    if (mode && _O_TEXT)
		{
			m1="r";
			m2="w";
		}
	    else
		{
			m1="rb";
			m2="wb";
		}

	    fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
	    f1 = _fdopen(fd1, m2);
	    fd2 = _open_osfhandle((long)hChildStdoutRdDup, mode);
	    f2 = _fdopen(fd2, m1);
	    p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
		PyFile_SetBufSize(p1, 0);
	    p2 = PyFile_FromFile(f2, cmdstring, m1, fclose);
		PyFile_SetBufSize(p2, 0);

	    if (n != 4)
			CloseHandle(hChildStderrRdDup);

	    f = Py_BuildValue("OO",p1,p2);
	    break;
	}
	
	case POPEN_3:
	{
	    char *m1, *m2;
	    PyObject *p1, *p2, *p3;
	    
	    if (mode && _O_TEXT)
		{
			m1="r";
			m2="w";
		}
	    else
		{
			m1="rb";
			m2="wb";
		}

	    fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
	    f1 = _fdopen(fd1, m2);
		fd2 = _open_osfhandle((long)hChildStdoutRdDup, mode);
	    f2 = _fdopen(fd2, m1);
		fd3 = _open_osfhandle((long)hChildStderrRdDup, mode);
	    f3 = _fdopen(fd3, m1);
	    p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
	    p2 = PyFile_FromFile(f2, cmdstring, m1, fclose);
	    p3 = PyFile_FromFile(f3, cmdstring, m1, fclose);
		PyFile_SetBufSize(p1, 0);
		PyFile_SetBufSize(p2, 0);
		PyFile_SetBufSize(p3, 0);
	    f = Py_BuildValue("OOO",p1,p2,p3);
	    break;
	}
	}

	if (n == POPEN_4)
	{
		if (!_PyPopenCreateProcess(cmdstring,f1,
								   hChildStdinRd,
								   hChildStdoutWr,
								   hChildStdoutWr))
			return PyWin_SetAPIError("CreateProcess");
	}
	else
	{
		if (!_PyPopenCreateProcess(cmdstring,f1,
								   hChildStdinRd,
								   hChildStdoutWr,
								   hChildStderrWr))
			return PyWin_SetAPIError("CreateProcess");
	}

    // Child is launched. Close the parents copy of those pipe handles
	// that only the child should have open.
	// You need to make sure that no handles to the write end of the
	// output pipe are maintained in this process or else the pipe will
	// not close when the child process exits and the ReadFile will hang.
	if (!CloseHandle(hChildStdinRd))
		return PyWin_SetAPIError("CloseHandle");
      
	if (!CloseHandle(hChildStdoutWr))
		return PyWin_SetAPIError("CloseHandle");
      
	if ((n != 4) && (!CloseHandle(hChildStderrWr)))
		return PyWin_SetAPIError("CloseHandle");

	return f;
}

/*
 * Wrapper for fclose() to use for popen* files, so we can retrieve the
 * exit code for the child process and return as a result of the close.
 */
static int _PyPclose(FILE *file)
{
   int result;
   DWORD exit_code;
   HANDLE hProcess;
   PyObject *hProcessObj, *fileObj;
   
   /* Close the file handle first, to ensure it can't block the
    * child from exiting when we wait for it below.
    */
   result = fclose(file);

   if (_PyPopenProcs) {
      fileObj = PyLong_FromVoidPtr(file);
      if (fileObj) {
	 hProcessObj = PyDict_GetItem(_PyPopenProcs, fileObj);
	 if (hProcessObj) {
	    hProcess = PyLong_AsVoidPtr(hProcessObj);
	    if (result != EOF &&
		WaitForSingleObject(hProcess, INFINITE) != WAIT_FAILED &&
		GetExitCodeProcess(hProcess, &exit_code)) {
	       /* Possible truncation here in 16-bit environments, but
		* real exit codes are just the lower byte in any event.
		*/
	       result = exit_code;
	    } else {
	       /* Indicate failure - this will cause the file object
		* to raise an I/O error and translate the last Win32
		* error code from errno.  We do have a problem with
		* last errors that overlap the normal errno table,
		* but that's a consistent problem with the file object.
		*/
	       if (result != EOF) {
		  /* If the error wasn't from the fclose(), then
		   * set errno for the file object error handling.
		   */
		  errno = GetLastError();
	       }
	       result = -1;
	    }

	    /* Free up the native handle at this point */
	    CloseHandle(hProcess);

	    /* Remove from dictionary and flush dictionary if empty */
	    PyDict_DelItem(_PyPopenProcs, fileObj);
	    if (PyDict_Size(_PyPopenProcs) == 0) {
	       Py_DECREF(_PyPopenProcs);
	       _PyPopenProcs = NULL;
	    }
	 } /* if hProcessObj */
      } /* if fileObj */
   } /* if _PyPopenProcs */

   return result;
}
