// @doc

#include "Python.h"

// Not used in py3k
#if (PY_VERSION_HEX < 0x03000000)

#include "malloc.h"
#include "io.h"
#include "fcntl.h"
#include "PyWinTypes.h"

#define DllExport _declspec(dllexport)

// These tell _PyPopen() wether to return 1, 2, or 3 file objects.
#define POPEN_1 1
#define POPEN_2 2
#define POPEN_3 3
#define POPEN_4 4

static PyObject *_PyPopen(char *, int, int);

static int _PyPclose(FILE *file);

/*
 * Internal dictionary mapping popen* file pointers to process handles,
 * for use when retrieving the process exit code.  See _PyPclose() below
 * for more information on this dictionary's use.
 */
static PyObject *_PyPopenProcs = NULL;

// @pymethod pipe|win32pipe|popen|Popen that works from a GUI.
// @rdesc The result of this function is a pipe (file) connected to the
// processes stdin or stdout, depending on the requested mode.
PyObject *PyPopen(PyObject *self, PyObject *args)
{
    char *cmdstring;
    char *mode = "r";
    int bufsize = -1;
    PyObject *f, *s;
    int tm = 0;

    if (!PyArg_ParseTuple(args, "s|s:popen",
                          &cmdstring,  // @pyparm string|cmdstring||The cmdstring to pass to the shell
                          &mode))      // @pyparm string|mode||Either 'r' or 'w'
        return NULL;

    s = PyTuple_New(0);

    if (*mode == 'r')
        tm = _O_RDONLY;
    else if (*mode != 'w') {
        PyErr_SetString(PyExc_ValueError, "mode must be 'r' or 'w'");
        return NULL;
    }
    else
        tm = _O_WRONLY;

    if (*(mode + 1) == 't')
        f = _PyPopen(cmdstring, tm | _O_TEXT, POPEN_1);
    else if (*(mode + 1) == 'b')
        f = _PyPopen(cmdstring, tm | _O_BINARY, POPEN_1);
    else
        f = _PyPopen(cmdstring, tm | _O_TEXT, POPEN_1);

    return f;
}

// @pymethod (pipe, pipe)|win32pipe|popen2|Variation on <om win32pipe.popen>
// @rdesc The result of this function is a pipe (file) connected to the
// processes stdin, and a pipe connected to the processes stdout.
PyObject *PyPopen2(PyObject *self, PyObject *args)
{
    char *cmdstring;
    char *mode = "t";
    PyObject *f;
    int tm = 0;

    if (!PyArg_ParseTuple(args, "s|s:popen2",
                          &cmdstring,  // @pyparm string|cmdstring||The cmdstring to pass to the shell
                          &mode))      // @pyparm string|mode||Either 't' or 'b'
        return NULL;

    if (*mode == 't')
        tm = _O_TEXT;
    else if (*mode != 'b') {
        PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
        return NULL;
    }
    else
        tm = _O_BINARY;

    f = _PyPopen(cmdstring, tm, POPEN_2);

    return f;
}

// @pymethod (pipe, pipe, pipe)|win32pipe|popen3|Variation on <om win32pipe.popen>
// @rdesc The result of this function is 3 pipes - the processes stdin, stdout and stderr
PyObject *PyPopen3(PyObject *self, PyObject *args)
{
    char *cmdstring;
    char *mode = "t";
    PyObject *f;
    int tm = 0;

    if (!PyArg_ParseTuple(args, "s|s:Popen3",
                          &cmdstring,  // @pyparm string|cmdstring||The cmdstring to pass to the shell
                          &mode))      // @pyparm string|mode||Either 't' or 'b'
        return NULL;

    if (*mode == 't')
        tm = _O_TEXT;
    else if (*mode != 'b') {
        PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
        return NULL;
    }
    else
        tm = _O_BINARY;

    f = _PyPopen(cmdstring, tm, POPEN_3);

    return f;
}

// @pymethod (pipe, pipe)|win32pipe|popen4|Variation on <om win32pipe.popen>
// @rdesc The result of this function is 2 pipes - the processes stdin,
// and stdout+stderr combined as a single pipe.
PyObject *PyPopen4(PyObject *self, PyObject *args)
{
    char *cmdstring;
    char *mode = "t";
    PyObject *f;
    int tm = 0;

    if (!PyArg_ParseTuple(args, "s|s:popen4",
                          &cmdstring,  // @pyparm string|cmdstring||The cmdstring to pass to the shell
                          &mode))      // @pyparm string|mode||Either 't' or 'b'
        return NULL;

    if (*mode == 't')
        tm = _O_TEXT;
    else if (*mode != 'b') {
        PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
        return NULL;
    }
    else
        tm = _O_BINARY;

    f = _PyPopen(cmdstring, tm, POPEN_4);

    return f;
}

static int _PyPopenCreateProcess(char *cmdstring, HANDLE hStdin, HANDLE hStdout, HANDLE hStderr, HANDLE *hProcess)
{
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    char *s1, *s2, *s3 = " /c ";
    DWORD i;
    size_t x;

    if (i = GetEnvironmentVariable("COMSPEC", NULL, 0)) {
        s1 = (char *)_alloca(i);
        if (!(x = GetEnvironmentVariable("COMSPEC", s1, i)))
            return FALSE;
        x = i + strlen(s3) + strlen(cmdstring) + 1;
        s2 = (char *)_alloca(x);
        ZeroMemory(s2, x);
        sprintf(s2, "%s%s%s", s1, s3, cmdstring);
    }
    // Could be an else here to try cmd.exe / command.com in the path
    // Now we'll just error out..
    else
        return FALSE;

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    siStartInfo.hStdInput = hStdin;
    siStartInfo.hStdOutput = hStdout;
    siStartInfo.hStdError = hStderr;
    siStartInfo.wShowWindow = SW_HIDE;
    if (CreateProcess(NULL, s2, NULL, NULL, TRUE, 0, /* no new console so Ctrl+C kills child too */
                      NULL, NULL, &siStartInfo, &piProcInfo)) {
        // Close the handles now so anyone waiting is woken.
        CloseHandle(piProcInfo.hThread);

        /* Return process handle */
        *hProcess = piProcInfo.hProcess;
        return TRUE;
    }
    return FALSE;
}

// The following code is based off of KB: Q190351
static PyObject *_PyPopen(char *cmdstring, int mode, int n)
{
    HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr, hChildStderrRd, hChildStderrWr,
        hChildStdinWrDup, hChildStdoutRdDup, hChildStderrRdDup, hProcess;  // hChildStdoutWrDup;

    SECURITY_ATTRIBUTES saAttr;
    BOOL fSuccess;
    int fd1, fd2, fd3;
    FILE *f1, *f2, *f3;
    long file_count;
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
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr, GetCurrentProcess(), &hChildStdinWrDup, 0, FALSE,
                               DUPLICATE_SAME_ACCESS);
    if (!fSuccess)
        return PyWin_SetAPIError("DuplicateHandle");

    // Close the inheritable version of ChildStdin
    // that we're using.
    CloseHandle(hChildStdinWr);

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
        return PyWin_SetAPIError("CreatePipe");

    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd, GetCurrentProcess(), &hChildStdoutRdDup, 0, FALSE,
                               DUPLICATE_SAME_ACCESS);
    if (!fSuccess)
        return PyWin_SetAPIError("DuplicateHandle");

    // Close the inheritable version of ChildStdout
    // that we're using.
    CloseHandle(hChildStdoutRd);

    if (n != POPEN_4) {
        if (!CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0))
            return PyWin_SetAPIError("CreatePipe");
        fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStderrRd, GetCurrentProcess(), &hChildStderrRdDup, 0,
                                   FALSE, DUPLICATE_SAME_ACCESS);
        if (!fSuccess)
            return PyWin_SetAPIError("DuplicateHandle");
        // Close the inheritable version of ChildStdErr that we're using.
        CloseHandle(hChildStderrRd);
    }

    switch (n) {
        case POPEN_1:
            switch (mode & (_O_RDONLY | _O_TEXT | _O_BINARY | _O_WRONLY)) {
                case _O_WRONLY | _O_TEXT:
                    // Case for writing to child Stdin in text mode.
                    fd1 = _open_osfhandle((INT_PTR)hChildStdinWrDup, mode);
                    f1 = _fdopen(fd1, "w");
                    f = PyFile_FromFile(f1, cmdstring, "w", _PyPclose);
                    PyFile_SetBufSize(f, 0);
                    // We don't care about these pipes anymore, so close them.
                    CloseHandle(hChildStdoutRdDup);
                    CloseHandle(hChildStderrRdDup);
                    break;

                case _O_RDONLY | _O_TEXT:
                    // Case for reading from child Stdout in text mode.
                    fd1 = _open_osfhandle((INT_PTR)hChildStdoutRdDup, mode);
                    f1 = _fdopen(fd1, "r");
                    f = PyFile_FromFile(f1, cmdstring, "r", _PyPclose);
                    PyFile_SetBufSize(f, 0);
                    // We don't care about these pipes anymore, so close them.
                    CloseHandle(hChildStdinWrDup);
                    CloseHandle(hChildStderrRdDup);
                    break;

                case _O_RDONLY | _O_BINARY:
                    // Case for readinig from child Stdout in binary mode.
                    fd1 = _open_osfhandle((INT_PTR)hChildStdoutRdDup, mode);
                    f1 = _fdopen(fd1, "rb");
                    f = PyFile_FromFile(f1, cmdstring, "rb", _PyPclose);
                    PyFile_SetBufSize(f, 0);
                    // We don't care about these pipes anymore, so close them.
                    CloseHandle(hChildStdinWrDup);
                    CloseHandle(hChildStderrRdDup);
                    break;

                case _O_WRONLY | _O_BINARY:
                    // Case for writing to child Stdin in binary mode.
                    fd1 = _open_osfhandle((INT_PTR)hChildStdinWrDup, mode);
                    f1 = _fdopen(fd1, "wb");
                    f = PyFile_FromFile(f1, cmdstring, "wb", _PyPclose);
                    PyFile_SetBufSize(f, 0);
                    // We don't care about these pipes anymore, so close them.
                    CloseHandle(hChildStdoutRdDup);
                    CloseHandle(hChildStderrRdDup);
                    break;
            }
            file_count = 1;
            break;

        case POPEN_2:
        case POPEN_4: {
            char *m1, *m2;
            PyObject *p1, *p2;

            if (mode & _O_TEXT) {
                m1 = "r";
                m2 = "w";
            }
            else {
                m1 = "rb";
                m2 = "wb";
            }

            fd1 = _open_osfhandle((INT_PTR)hChildStdinWrDup, mode);
            f1 = _fdopen(fd1, m2);
            fd2 = _open_osfhandle((INT_PTR)hChildStdoutRdDup, mode);
            f2 = _fdopen(fd2, m1);
            p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
            PyFile_SetBufSize(p1, 0);
            p2 = PyFile_FromFile(f2, cmdstring, m1, _PyPclose);
            PyFile_SetBufSize(p2, 0);

            if (n != 4)
                CloseHandle(hChildStderrRdDup);

            f = Py_BuildValue("OO", p1, p2);
            Py_XDECREF(p1);
            Py_XDECREF(p2);
            file_count = 2;
            break;
        }

        case POPEN_3: {
            char *m1, *m2;
            PyObject *p1, *p2, *p3;

            if (mode & _O_TEXT) {
                m1 = "r";
                m2 = "w";
            }
            else {
                m1 = "rb";
                m2 = "wb";
            }

            fd1 = _open_osfhandle((INT_PTR)hChildStdinWrDup, mode);
            f1 = _fdopen(fd1, m2);
            fd2 = _open_osfhandle((INT_PTR)hChildStdoutRdDup, mode);
            f2 = _fdopen(fd2, m1);
            fd3 = _open_osfhandle((INT_PTR)hChildStderrRdDup, mode);
            f3 = _fdopen(fd3, m1);
            p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
            p2 = PyFile_FromFile(f2, cmdstring, m1, _PyPclose);
            p3 = PyFile_FromFile(f3, cmdstring, m1, _PyPclose);
            PyFile_SetBufSize(p1, 0);
            PyFile_SetBufSize(p2, 0);
            PyFile_SetBufSize(p3, 0);
            f = Py_BuildValue("OOO", p1, p2, p3);
            Py_XDECREF(p1);
            Py_XDECREF(p2);
            Py_XDECREF(p3);
            file_count = 3;
            break;
        }
    }

    if (n == POPEN_4) {
        if (!_PyPopenCreateProcess(cmdstring, hChildStdinRd, hChildStdoutWr, hChildStdoutWr, &hProcess))
            return PyWin_SetAPIError("CreateProcess");
    }
    else {
        if (!_PyPopenCreateProcess(cmdstring, hChildStdinRd, hChildStdoutWr, hChildStderrWr, &hProcess))
            return PyWin_SetAPIError("CreateProcess");
    }

    /*
     * Insert the files we've created into the process dictionary
     * all referencing the list with the process handle and the
     * initial number of files (see description below in _PyPclose).
     * Since if _PyPclose later tried to wait on a process when all
     * handles weren't closed, it could create a deadlock with the
     * child, we spend some energy here to try to ensure that we
     * either insert all file handles into the dictionary or none
     * at all.  It's a little clumsy with the various popen modes
     * and variable number of files involved.
     */
    if (!_PyPopenProcs) {
        _PyPopenProcs = PyDict_New();
    }

    if (_PyPopenProcs) {
        PyObject *procObj, *hProcessObj, *intObj, *fileObj[3];
        int ins_rc[3];

        fileObj[0] = fileObj[1] = fileObj[2] = NULL;
        ins_rc[0] = ins_rc[1] = ins_rc[2] = 0;

        procObj = PyList_New(2);
        hProcessObj = PyLong_FromVoidPtr(hProcess);
        intObj = PyInt_FromLong(file_count);

        if (procObj && hProcessObj && intObj) {
            PyList_SetItem(procObj, 0, hProcessObj);
            PyList_SetItem(procObj, 1, intObj);

            fileObj[0] = PyLong_FromVoidPtr(f1);
            if (fileObj[0]) {
                ins_rc[0] = PyDict_SetItem(_PyPopenProcs, fileObj[0], procObj);
            }
            if (file_count >= 2) {
                fileObj[1] = PyLong_FromVoidPtr(f2);
                if (fileObj[1]) {
                    ins_rc[1] = PyDict_SetItem(_PyPopenProcs, fileObj[1], procObj);
                }
            }
            if (file_count >= 3) {
                fileObj[2] = PyLong_FromVoidPtr(f3);
                if (fileObj[2]) {
                    ins_rc[2] = PyDict_SetItem(_PyPopenProcs, fileObj[2], procObj);
                }
            }

            if (ins_rc[0] < 0 || !fileObj[0] || ins_rc[1] < 0 || (file_count > 1 && !fileObj[1]) || ins_rc[2] < 0 ||
                (file_count > 2 && !fileObj[2])) {
                /* Something failed - remove any dictionary
                 * entries that did make it.
                 */
                if (!ins_rc[0] && fileObj[0]) {
                    PyDict_DelItem(_PyPopenProcs, fileObj[0]);
                }
                if (!ins_rc[1] && fileObj[1]) {
                    PyDict_DelItem(_PyPopenProcs, fileObj[1]);
                }
                if (!ins_rc[2] && fileObj[2]) {
                    PyDict_DelItem(_PyPopenProcs, fileObj[2]);
                }
            }
        }

        /*
         * Clean up our localized references for the dictionary keys
         * and value since PyDict_SetItem will Py_INCREF any copies
         * that got placed in the dictionary.
         */
        Py_XDECREF(procObj);
        Py_XDECREF(fileObj[0]);
        Py_XDECREF(fileObj[1]);
        Py_XDECREF(fileObj[2]);
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
 *
 * This function uses the _PyPopenProcs dictionary in order to map the
 * input file pointer to information about the process that was
 * originally created by the popen* call that created the file pointer.
 * The dictionary uses the file pointer as a key (with one entry
 * inserted for each file returned by the original popen* call) and a
 * single list object as the value for all files from a single call.
 * The list object contains the Win32 process handle at [0], and a file
 * count at [1], which is initialized to the total number of file
 * handles using that list.
 *
 * This function closes whichever handle it is passed, and decrements
 * the file count in the dictionary for the process handle pointed to
 * by this file.  On the last close (when the file count reaches zero),
 * this function will wait for the child process and then return its
 * exit code as the result of the close() operation.  This permits the
 * files to be closed in any order - it is always the close() of the
 * final handle that will return the exit code.
 */
static int _PyPclose(FILE *file)
{
    int result;
    DWORD exit_code;
    HANDLE hProcess;
    PyObject *procObj, *hProcessObj, *intObj, *fileObj;
    long file_count;

    /* Close the file handle first, to ensure it can't block the
     * child from exiting if it's the last handle.
     */
    result = fclose(file);

    if (_PyPopenProcs) {
        CEnterLeavePython _celp;
        if ((fileObj = PyLong_FromVoidPtr(file)) != NULL &&
            (procObj = PyDict_GetItem(_PyPopenProcs, fileObj)) != NULL &&
            (hProcessObj = PyList_GetItem(procObj, 0)) != NULL && (intObj = PyList_GetItem(procObj, 1)) != NULL) {
            hProcess = PyLong_AsVoidPtr(hProcessObj);
            file_count = PyInt_AsLong(intObj);

            if (file_count > 1) {
                /* Still other files referencing process */
                file_count--;
                PyList_SetItem(procObj, 1, PyInt_FromLong(file_count));
            }
            else {
                Py_BEGIN_ALLOW_THREADS
                    /* Last file for this process */
                    if (result != EOF && WaitForSingleObject(hProcess, INFINITE) != WAIT_FAILED &&
                        GetExitCodeProcess(hProcess, &exit_code))
                {
                    /* Possible truncation here in 16-bit environments, but
                     * real exit codes are just the lower byte in any event.
                     */
                    result = exit_code;
                }
                else
                {
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
                Py_END_ALLOW_THREADS
            }

            /* Remove this file pointer from dictionary */
            PyDict_DelItem(_PyPopenProcs, fileObj);

            if (PyDict_Size(_PyPopenProcs) == 0) {
                Py_DECREF(_PyPopenProcs);
                _PyPopenProcs = NULL;
            }

        } /* if object retrieval ok */

        Py_XDECREF(fileObj);
    } /* if _PyPopenProcs */

    return result;
}

#endif  // PY_VERSION_HEX < 0x03000000
