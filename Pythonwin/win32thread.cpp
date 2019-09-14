/*

    MFC Thread data type

    Created Jan 1998, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32ui.h"
#include "win32win.h"

extern BOOL HookWindowsMessages();
extern BOOL Win32uiPreTranslateMessage(MSG *pMsg);

class CPythonWinThread : public CWinThread {
   public:
    CPythonWinThread()
    {
        obFunc = NULL;
        obArgs = NULL;
    }
    CPythonWinThread(AFX_THREADPROC pfnThreadProc) : CWinThread(pfnThreadProc, NULL)
    {
        m_pThreadParams = this;
        obFunc = NULL;
        obArgs = NULL;
    }

    ~CPythonWinThread()
    {
        Py_XDECREF(obFunc);
        Py_XDECREF(obArgs);
        Python_delete_assoc(this);
    }
    virtual BOOL PreTranslateMessage(MSG *pMsg)
    {
        if (Win32uiPreTranslateMessage(pMsg))
            return TRUE;
        else
            return CWinThread::PreTranslateMessage(pMsg);
    }

    virtual BOOL InitInstance()
    {
        HookWindowsMessages();
        CVirtualHelper helper("InitInstance", this);
        if (helper.HaveHandler() && helper.call()) {
            BOOL ret;
            helper.retval(ret);
            // The main app InitInstance assumes a zero return.
            return (ret == 0);
        }
        else
            return CWinThread::InitInstance();
    }
    virtual int ExitInstance()
    {
        CVirtualHelper helper("ExitInstance", this);
        if (helper.HaveHandler() && helper.call()) {
            int ret;
            helper.retval(ret);
            return ret;
        }
        else
            return CWinThread::ExitInstance();
    }
    virtual int Run()
    {
        int ret;
        CVirtualHelper helper("Run", this);
        if (!helper.HaveHandler())
            ret = CWinThread::Run();
        else {
            helper.call();
            helper.retval(ret);
        }
        return ret;
    }
    PyObject *obFunc;
    PyObject *obArgs;
};

void CProtectedWinThread::PumpIdle()
{
    long lIdleCount = 0;
    while (OnIdle(lIdleCount++))
        ;
    return;
}

void CProtectedWinThread::PumpMessages()
{
    ASSERT_VALID(this);

    // for tracking the idle time state
    BOOL bIdle = TRUE;
    LONG lIdleCount = 0;
#if _MFC_VER >= 0x0710
    _AFX_THREAD_STATE *pState = AfxGetThreadState();
    MSG &msgCur = pState->m_msgCur;
#else
    MSG &msgCur = m_msgCur;
#endif /* _MFC_VER_ */

    // acquire and dispatch messages until a WM_QUIT message is received.
    for (;;) {
        // phase1: check to see if we can do idle work
        while (bIdle && !::PeekMessage(&msgCur, NULL, NULL, NULL, PM_NOREMOVE)) {
            // call OnIdle while in bIdle state
            if (!OnIdle(lIdleCount++))
                bIdle = FALSE;  // assume "no idle" state
        }
        // phase2: pump messages while available
        do {
            // pump message, but quit on WM_QUIT
            if (!PumpMessage()) {
#if defined(_DEBUG)
#if _MFC_VER < 0x0710
                m_nDisablePumpCount--;  // application must NOT die
#else
                pState->m_nDisablePumpCount--;  // application must NOT die
#endif
#endif
                return;
            }

            // reset "no idle" state after pumping "normal" message
            if (IsIdleMessage(&msgCur)) {
                bIdle = TRUE;
                lIdleCount = 0;
            }

        } while (::PeekMessage(&msgCur, NULL, NULL, NULL, PM_NOREMOVE));
    }

    ASSERT(FALSE);  // not reachable
}

bool CProtectedWinThread::PumpWaitingMessages(UINT firstMsg, UINT lastMsg)
{
    bool bHaveQuit = false;
    MSG msg;
    if (::PeekMessage(&msg, NULL, firstMsg, lastMsg, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            bHaveQuit = true;
        ::DispatchMessage(&msg);
    }
    return bHaveQuit;
}
unsigned int ThreadWorkerEntryPoint(LPVOID lpvoid)
{
    CPythonWinThread *pThis = (CPythonWinThread *)lpvoid;
    CEnterLeavePython _celp;
    PyObject *result = PyEval_CallObject(pThis->obFunc, pThis->obArgs);
    if (result == NULL) {
        if (PyErr_Occurred() == PyExc_SystemExit)
            PyErr_Clear();
        else {
            ExceptionHandler(EHA_PRINT_ERROR, _T("Unhandled exception in thread"));
        }
    }
    else
        Py_DECREF(result);
    // Cleanup thread state?
    return 0;
}

PyCWinThread::PyCWinThread() {}

PyCWinThread::~PyCWinThread() {}

CWinThread *GetCWinThreadPtr(PyObject *self)
{
    return (CWinThread *)ui_assoc_object::GetGoodCppObject(self, &PyCWinThread::type);
}
CProtectedWinThread *GetCProtectedWinThreadPtr(PyObject *self)
{
    return (CProtectedWinThread *)ui_assoc_object::GetGoodCppObject(self, &PyCWinThread::type);
}

// @pymethod <o PyCWinThread>|win32ui|CreateThread|Creates a new <o PyCWinThread> object
PyObject *PyCWinThread::create(PyObject *self, PyObject *args)
{
    CPythonWinThread *pThread;
    PyObject *obFunc, *obArgs = Py_None;
    if (PyArg_ParseTuple(args, "|:CreateThread")) {
        pThread = new CPythonWinThread();
    }
    else if (PyArg_ParseTuple(args, "O|O:CreateThread", &obFunc, &obArgs)) {
        PyErr_Clear();
        if (!PyCallable_Check(obFunc)) {
            PyErr_SetString(PyExc_TypeError, "First argument must be a callable object");
            return NULL;
        }
        pThread = new CPythonWinThread(ThreadWorkerEntryPoint);
        pThread->obFunc = obFunc;
        pThread->obArgs = obArgs;
        Py_INCREF(obFunc);
        Py_INCREF(obArgs);
    }
    else {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "Must pass no arguments, or a function and optional arguments");
        return NULL;
    }
    return ui_assoc_object::make(PyCWinThread::type, pThread, TRUE);
}

// @pymethod |PyCWinThread|PumpIdle|Pumps all idle messages.
static PyObject *ui_thread_pump_idle(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, PumpIdle);
    CProtectedWinThread *pThread = GetCProtectedWinThreadPtr(self);
    if (!pThread)
        return NULL;
    GUI_BGN_SAVE;
    pThread->PumpIdle();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCWinThread|SetMainFrame|Sets the threads main frame
static PyObject *ui_thread_set_main_frame(PyObject *self, PyObject *args)
{
    PyObject *wndObject;
    if (!PyArg_ParseTuple(args, "O:SetMainFrame",
                          &wndObject))  // @pyparm <o PyCWnd>|mainFrame||The applications main frame.
        return NULL;
    CWinThread *pThread = GetCWinThreadPtr(self);
    if (!pThread)
        return NULL;

    if (wndObject == Py_None) {
        // @comm You can pass None to this function to reset the main frame.
        pThread->m_pMainWnd = NULL;  // Should I free this?  I dont think so!
    }
    else {
        CWnd *pMainWnd = GetWndPtr(wndObject);
        if (!pMainWnd)
            return NULL;
        pThread->m_pMainWnd = pMainWnd;
    }
    RETURN_NONE;
}

// @pymethod |PyCWinThread|SetThreadPriority|Sets the threads priority.  Returns TRUE if successful.
static PyObject *ui_thread_set_thread_priority(PyObject *self, PyObject *args)
{
    int priority;
    if (!PyArg_ParseTuple(args, "i:SetThreadPriority",
                          &priority))  // @pyparm <o PyCWnd>|priority||The threads priority.
        return NULL;
    CWinThread *pThread = GetCWinThreadPtr(self);
    if (!pThread)
        return NULL;

    long rc = pThread->SetThreadPriority(priority);
    return PyInt_FromLong(rc);
}

// @pymethod int|PyCWinThread|Run|Starts the message pump.  Advanced users only
static PyObject *ui_thread_run(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, "Run");
    CWinThread *pThread = GetCWinThreadPtr(self);
    if (!pThread)
        return NULL;
    GUI_BGN_SAVE;
    long rc = pThread->CWinThread::Run();
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod |PyCWinThread|CreateThread|Creates the actual thread behind the thread object.
static PyObject *ui_thread_create_thread(PyObject *self, PyObject *args)
{
    DWORD createFlags = 0;
    UINT stackSize = 0;
    if (!PyArg_ParseTuple(args, "|li:CreateThread", &createFlags, &stackSize))
        return NULL;
    CWinThread *pThread = GetCWinThreadPtr(self);
    if (!pThread)
        return NULL;
    PyEval_InitThreads();
    GUI_BGN_SAVE;
    BOOL ok = pThread->CreateThread(createFlags, stackSize);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("CreateThread failed");
    RETURN_NONE;
}

// @pymethod |PyCWinThread|PumpMessages|Pumps all messages to the application until a WM_QUIT message is received.
static PyObject *ui_thread_pump_messages(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":PumpMessages"))
        return NULL;
    CProtectedWinThread *pThread = GetProtectedThread();
    if (!pThread)
        return NULL;
    GUI_BGN_SAVE;
    pThread->PumpMessages();
    GUI_END_SAVE;
    RETURN_NONE;
    // @comm This allows an application which is performing a long operation to dispatch paint messages during the
    // operation.
}

// @object PyCWinThread|An application class.  Encapsulates an MFC <c CWinThread> class
static struct PyMethodDef PyCWinThread_methods[] = {
    {"CreateThread", ui_thread_create_thread,
     1},                                   // @pymeth CreateThread|Creates the actual thread behind the thread object.
    {"PumpIdle", ui_thread_pump_idle, 1},  // @pymeth PumpIdle|Pumps idle messages.
    {"PumpMessages", ui_thread_pump_messages,
     1},  // @pymeth PumpMessages|Pumps all messages to the application until a WM_QUIT message is received.
    {"Run", ui_thread_run, 1},                      // @pymeth Run|Starts the main application message pump.
    {"SetMainFrame", ui_thread_set_main_frame, 1},  // @pymeth SetMainFrame|Sets the C++ applications main frame
    {"SetThreadPriority", ui_thread_set_thread_priority, 1},  // @pymeth SetThreadPriority|Sets the threads priority
    {NULL, NULL}};
ui_type_CObject PyCWinThread::type("PyCWinThread", &PyCCmdTarget::type, RUNTIME_CLASS(CWinThread), sizeof(PyCWinThread),
                                   PYOBJ_OFFSET(PyCWinThread), PyCWinThread_methods, GET_PY_CTOR(PyCWinThread));
