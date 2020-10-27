// axcom.cpp :
// $Id$

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "stdafx.h"
#include "PythonCOMRegister.h"
#include "PyIActiveScriptDebug.h"
#include "PyIActiveScriptSiteDebug.h"
#include "PyIMachineDebugManager.h"
#include "PyIRemoteDebugApplication.h"
#include "PyIRemoteDebugApplicationEvents.h"
#include "PyIEnumRemoteDebugApplications.h"
#include "PyIDebugDocumentInfo.h"
#include "PyIDebugDocumentProvider.h"
#include "PyIDebugDocument.h"
#include "PyIDebugDocumentText.h"
#include "PyIDebugApplicationNode.h"
#include "PyIDebugApplicationNodeEvents.h"
#include "PyIProcessDebugManager.h"
#include "PyIApplicationDebugger.h"
#include "PyIDebugApplication.h"
#include "PyIDebugDocumentContext.h"
#include "PyIDebugCodeContext.h"
#include "PyIEnumDebugCodeContexts.h"
#include "PyIDebugExpressionContext.h"
#include "PyIDebugExpression.h"
#include "PyIDebugExpressionCallBack.h"
#include "PyIDebugStackFrame.h"
#include "PyIEnumDebugStackFrames.h"
#include "PyIDebugStackFrameSniffer.h"
#include "PyIDebugStackFrameSnifferEx.h"
#include "PyIRemoteDebugApplicationThread.h"
#include "PyIDebugApplicationThread.h"
#include "PyIDebugProperties.h"
#include "PyIEnumDebugApplicationNodes.h"
#include "PyIEnumRemoteDebugApplicationThreads.h"
#include "PyIDebugDocumentHelper.h"
#include "PyIDebugDocumentTextEvents.h"
#include "PyIDebugDocumentTextAuthor.h"
#include "PyIDebugDocumentTextExternalAuthor.h"
#include "PyIDebugDocumentHost.h"
#include "PyIDebugSyncOperation.h"
#include "PyIMachineDebugManagerEvents.h"
#include "PyIDebugSessionProvider.h"
#include "PyIEnumDebugExpressionContexts.h"
#include "PyIProvideExpressionContexts.h"
#include "PyIActiveScriptErrorDebug.h"
#include "PyIEnumDebugPropertyInfo.h"

// Headers needed for the f_trace hacks.
#include "compile.h"
#include "frameobject.h"

static PyObject *axdebug_Error; /* 'Python level' errors */

BOOL PyAXDebug_PySOURCE_TEXT_ATTR_Length(PyObject *obAttr, ULONG *pLength)
{
    if (!PySequence_Check(obAttr))
        return FALSE;
    DWORD seqLen = (DWORD)PySequence_Length(obAttr);
    DWORD attrLen = 0;
    BOOL ok = TRUE;
    for (DWORD i = 0; ok && i < seqLen; i++) {
        PyObject *ob = PySequence_GetItem(obAttr, i);
        if (!ob) {
            ok = FALSE;
            break;
        }
        if (PyInt_Check(ob)) {
            ++attrLen;
        }
        else if (PySequence_Check(ob) && PySequence_Length(ob) == 2) {
            PyObject *obRepeat = PySequence_GetItem(ob, 1);
            if (obRepeat == NULL)
                ok = FALSE;
            else {
                attrLen += PyInt_AsLong(obRepeat);
                Py_DECREF(obRepeat);
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Invalid format in SOURCE_TEXT_ATTR object");
            ok = FALSE;
        }
        Py_DECREF(ob);
    }
    if (ok)
        *pLength = attrLen;
    return ok;
}

BOOL PyAXDebug_PyObject_AsSOURCE_TEXT_ATTR(PyObject *obAttr, SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr)
{
    BOOL ok = PySequence_Check(obAttr);
    if (ok) {
        // We support 2 formats in the tuple.
        // Either: a simple integer
        // Or    : a tuple with an integer, repeatCount.
        // Either case, the total _must_ be the same as the requested size.
        DWORD seqLen = (DWORD)PySequence_Length(obAttr);
        DWORD attrLen = 0;
        for (DWORD i = 0; ok && i < seqLen; i++) {
            PyObject *ob = PySequence_GetItem(obAttr, i);
            if (ob) {
                if (PyInt_Check(ob)) {
                    pstaTextAttr[attrLen] = (SOURCE_TEXT_ATTR)PyInt_AsLong(ob);
                    ++attrLen;
                }
                else if (PySequence_Check(ob) && PySequence_Length(ob) == 2) {
                    PyObject *obAttr = PySequence_GetItem(ob, 0);
                    PyObject *obRepeat = PySequence_GetItem(ob, 1);
                    if (obAttr == NULL || obRepeat == NULL)
                        ok = FALSE;
                    else {
                        SOURCE_TEXT_ATTR attr = (SOURCE_TEXT_ATTR)PyInt_AsLong(obAttr);
                        DWORD len = (DWORD)PyInt_AsLong(obRepeat);
                        if (attrLen + len <= numAttr) {
                            while (len--) pstaTextAttr[attrLen++] = attr;
                        }
                    }
                    Py_XDECREF(obAttr);
                    Py_XDECREF(obRepeat);
                }
                else {
                    PyErr_SetString(PyExc_ValueError,
                                    "Attributes must be a sequence of [attribute|(attribute, repeat)]");
                    ok = FALSE;
                }
                Py_DECREF(ob);
            }
            else
                ok = FALSE;
        }
        if (attrLen != numAttr) {
            PyErr_SetString(PyExc_ValueError, "The attributes must resolve to the exact length as the text");
            ok = FALSE;
        }
    }
    return ok;
}

PyObject *PyAXDebug_PyObject_FromSOURCE_TEXT_ATTR(const SOURCE_TEXT_ATTR *pstaTextAttr, ULONG numAttr)
{
    PyObject *obattr = PyTuple_New(numAttr);
    if (obattr)
        for (ULONG i = 0; i < numAttr; i++) {
            PyTuple_SET_ITEM(obattr, i, PyInt_FromLong(pstaTextAttr[i]));
        }
    return obattr;
}

// A few hacks to get debugging doing the right thing.
static PyObject *GetStackAddress(PyObject *, PyObject *)
{
    int i;
    return PyWinLong_FromVoidPtr(&i);
}

static PyObject *GetThreadStateHandle(PyObject *self, PyObject *args)
{
    // We _must_ have the thread-lock to be called!
    PyThreadState *myState = PyThreadState_Swap(NULL);
    PyThreadState_Swap(myState);
    return PyWinLong_FromVoidPtr(myState);
}
static PyObject *SetThreadStateTrace(PyObject *self, PyObject *args)
{
    PyObject *obhandle;
    PyObject *func;
    if (!PyArg_ParseTuple(args, "OO", &obhandle, &func))
        return NULL;
    PyThreadState *state;
    if (!PyWinLong_AsVoidPtr(obhandle, (void **)&state))
        return NULL;
#if (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 2) || PY_MAJOR_VERSION > 2
#pragma message("XXXXXXXXX - upgrade this for new tracing features.")
/***
        XXX - maybe use PyEval_SetTrace ????
    Py_XDECREF(state->c_tracefunc);
    state->c_tracefunc = func;
    state->tracing = TRUE;
***/
#else
    Py_XDECREF(state->sys_tracefunc);
    state->sys_tracefunc = func;
    Py_INCREF(func);
#endif
    // Loop back over all frames, setting each frame back to our
    // first script block frame with the tracer.
    PyFrameObject *frame = state ? state->frame : NULL;
    bool bFoundFirstScriptBlock = false;
    while (frame) {
        if (strncmp(PyString_AsString(frame->f_code->co_filename), "<Script ", 8) == 0)
            bFoundFirstScriptBlock = true;
        else {
            if (bFoundFirstScriptBlock)
                break;
        }
        Py_XDECREF(frame->f_trace);
        frame->f_trace = func;
        Py_INCREF(func);
        frame = frame->f_back;
    }
    Py_INCREF(Py_None);
    return Py_None;
}
/* List of module functions */
// @module axdebug|A module, encapsulating the ActiveX Debugging interfaces
static struct PyMethodDef axdebug_methods[] = {{"GetStackAddress", GetStackAddress, 1},
                                               {"GetThreadStateHandle", GetThreadStateHandle, 1},
                                               {"SetThreadStateTrace", SetThreadStateTrace, 1},
                                               {NULL, NULL}};

// special case so we use __uuidof, to avoid needing a .lib we don't have!
#undef PYCOM_INTERFACE_FULL
#define PYCOM_INTERFACE_FULL PYCOM_INTERFACE_FULL_UUIDOF

// The list of interfaces and gateways we support.
static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] = {
    PYCOM_INTERFACE_FULL(ActiveScriptErrorDebug),
    PYCOM_INTERFACE_FULL(ActiveScriptDebug),
    PYCOM_INTERFACE_FULL(ActiveScriptSiteDebug),

    PYCOM_INTERFACE_FULL(ApplicationDebugger),

    PYCOM_INTERFACE_FULL(DebugApplication),
    PYCOM_INTERFACE_FULL(DebugApplicationNode),
    PYCOM_INTERFACE_FULL(DebugApplicationNodeEvents),
    PYCOM_INTERFACE_FULL(DebugApplicationThread),
    PYCOM_INTERFACE_FULL(DebugCodeContext),

    PYCOM_INTERFACE_FULL(DebugDocument),
    PYCOM_INTERFACE_FULL(DebugDocumentContext),
    PYCOM_INTERFACE_FULL(DebugDocumentHelper),
    PYCOM_INTERFACE_FULL(DebugDocumentHost),
    PYCOM_INTERFACE_FULL(DebugDocumentInfo),
    PYCOM_INTERFACE_FULL(DebugDocumentProvider),
    PYCOM_INTERFACE_FULL(DebugDocumentText),

    PYCOM_INTERFACE_FULL(DebugDocumentTextAuthor),
    PYCOM_INTERFACE_FULL(DebugDocumentTextEvents),
    PYCOM_INTERFACE_FULL(DebugDocumentTextExternalAuthor),

    PYCOM_INTERFACE_FULL(DebugExpression),
    PYCOM_INTERFACE_FULL(DebugExpressionCallBack),
    PYCOM_INTERFACE_FULL(DebugExpressionContext),

    PYCOM_INTERFACE_FULL(DebugProperty),

    PYCOM_INTERFACE_FULL(DebugSessionProvider),

    PYCOM_INTERFACE_FULL(DebugStackFrame),
    PYCOM_INTERFACE_FULL(DebugStackFrameSniffer),
    PYCOM_INTERFACE_FULL(DebugStackFrameSnifferEx),

    PYCOM_INTERFACE_FULL(DebugSyncOperation),

    PYCOM_INTERFACE_FULL(EnumDebugApplicationNodes),
    PYCOM_INTERFACE_FULL(EnumDebugCodeContexts),
    PYCOM_INTERFACE_FULL(EnumDebugExpressionContexts),
    PYCOM_INTERFACE_FULL(EnumDebugPropertyInfo),
    PYCOM_INTERFACE_FULL(EnumDebugStackFrames),
    PYCOM_INTERFACE_FULL(EnumRemoteDebugApplications),
    PYCOM_INTERFACE_FULL(EnumRemoteDebugApplicationThreads),

    PYCOM_INTERFACE_FULL(MachineDebugManager),
    PYCOM_INTERFACE_FULL(MachineDebugManagerEvents),
    PYCOM_INTERFACE_FULL(ProcessDebugManager),
    PYCOM_INTERFACE_FULL(ProvideExpressionContexts),
    PYCOM_INTERFACE_FULL(RemoteDebugApplication),
    PYCOM_INTERFACE_FULL(RemoteDebugApplicationEvents),
    PYCOM_INTERFACE_FULL(RemoteDebugApplicationThread),

    PYCOM_INTERFACE_CLSID_ONLY(MachineDebugManager),  // @const axdebug|CLSID_MachineDebugManager|An IID object
    PYCOM_INTERFACE_CLSID_ONLY(ProcessDebugManager),  // @const axdebug|CLSID_ProcessDebugManager|An IID object
    PYCOM_INTERFACE_CLSID_ONLY(
        DefaultDebugSessionProvider),  // @const axdebug|CLSID_DefaultDebugSessionProvider|An IID object
};

#define ADD_CONSTANT(tok)                                 \
    if (PyModule_AddIntConstant(module, #tok, tok) == -1) \
        PYWIN_MODULE_INIT_RETURN_ERROR;

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(axdebug)
{
    PYWIN_MODULE_INIT_PREPARE(axdebug, axdebug_methods, "A module, encapsulating the ActiveX Debugging interfaces");

    PyEval_InitThreads();

    // Add some symbolic constants to the module
    axdebug_Error = PyErr_NewException("axdebug.error", NULL, NULL);
    if (axdebug_Error == NULL || PyDict_SetItemString(dict, "error", axdebug_Error) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // AX-Debugging interface registration
    PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData,
                                   sizeof(g_interfaceSupportData) / sizeof(PyCom_InterfaceSupportInfo));

    ADD_CONSTANT(APPBREAKFLAG_DEBUGGER_BLOCK);  // @const axdebug|APPBREAKFLAG_DEBUGGER_BLOCK|Languages should break
                                                // immediately with BREAKREASON_DEBUGGER_BLOCK
    ADD_CONSTANT(APPBREAKFLAG_DEBUGGER_HALT);   // @const axdebug|APPBREAKFLAG_DEBUGGER_HALT|Languages should break
                                                // immediately with BREAKREASON_DEBUGGER_HALT
    ADD_CONSTANT(APPBREAKFLAG_STEP);            // @const axdebug|APPBREAKFLAG_STEP|
                                                //	ADD_CONSTANT(APPBREAKFLAG_APPBREAKFLAG_NESTED);
    ADD_CONSTANT(APPBREAKFLAG_STEPTYPE_SOURCE);
    ADD_CONSTANT(APPBREAKFLAG_STEPTYPE_BYTECODE);
    ADD_CONSTANT(APPBREAKFLAG_STEPTYPE_MACHINE);
    ADD_CONSTANT(APPBREAKFLAG_STEPTYPE_MASK);
    ADD_CONSTANT(APPBREAKFLAG_IN_BREAKPOINT);

    ADD_CONSTANT(BREAKPOINT_DELETED);   // @const axdebug|BREAKPOINT_DELETED|
    ADD_CONSTANT(BREAKPOINT_DISABLED);  // @const axdebug|BREAKPOINT_DISABLED|
    ADD_CONSTANT(BREAKPOINT_ENABLED);   // @const axdebug|BREAKPOINT_ENABLED|

    ADD_CONSTANT(BREAKREASON_STEP);        // @const axdebug|BREAKREASON_STEP|Caused by the stepping mode
    ADD_CONSTANT(BREAKREASON_BREAKPOINT);  // @const axdebug|BREAKREASON_BREAKPOINT|Caused by an explicit breakpoint
    ADD_CONSTANT(
        BREAKREASON_DEBUGGER_BLOCK);  // @const axdebug|BREAKREASON_DEBUGGER_BLOCK|Caused by another thread breaking
    ADD_CONSTANT(
        BREAKREASON_HOST_INITIATED);  // @const axdebug|BREAKREASON_HOST_INITIATED|Caused by host requested break
    ADD_CONSTANT(
        BREAKREASON_LANGUAGE_INITIATED);  // @const axdebug|BREAKREASON_LANGUAGE_INITIATED|Caused by a scripted break
    ADD_CONSTANT(
        BREAKREASON_DEBUGGER_HALT);   // @const axdebug|BREAKREASON_DEBUGGER_HALT|Caused by debugger IDE requested break
    ADD_CONSTANT(BREAKREASON_ERROR);  // @const axdebug|BREAKREASON_ERROR|Caused by an execution error

    ADD_CONSTANT(BREAKRESUMEACTION_ABORT);      // @const axdebug|BREAKRESUMEACTION_ABORT|Abort the application
    ADD_CONSTANT(BREAKRESUMEACTION_CONTINUE);   // @const axdebug|BREAKRESUMEACTION_CONTINUE|Continue running
    ADD_CONSTANT(BREAKRESUMEACTION_STEP_INTO);  // @const axdebug|BREAKRESUMEACTION_STEP_INTO|Step into a procedure
    ADD_CONSTANT(BREAKRESUMEACTION_STEP_OVER);  // @const axdebug|BREAKRESUMEACTION_STEP_OVER|Step over a procedure
    ADD_CONSTANT(
        BREAKRESUMEACTION_STEP_OUT);  // @const axdebug|BREAKRESUMEACTION_STEP_OUT|Step out of the current procedure

    ADD_CONSTANT(DEBUG_TEXT_ISEXPRESSION);      // @const axdebug|DEBUG_TEXT_ISEXPRESSION|Indicates that the text is an
                                                // expression as opposed to a statement. This flag may affect the way in
                                                // which the text is parsed by some languages.
    ADD_CONSTANT(DEBUG_TEXT_ALLOWBREAKPOINTS);  // @const axdebug|DEBUG_TEXT_ALLOWBREAKPOINTS|Allow breakpoints during
                                                // the evaluation of the text. If this flag is not set then breakpoints
                                                // will be ignored during the evaluation of the text.

    ADD_CONSTANT(DOCUMENTNAMETYPE_APPNODE);  // @const axdebug|DOCUMENTNAMETYPE_APPNODE|Gets the name as it appears in
                                             // the app tree
    ADD_CONSTANT(DOCUMENTNAMETYPE_TITLE);    // @const axdebug|DOCUMENTNAMETYPE_TITLE|Gets the name as it appears on the
                                             // doc viewer title bar
    ADD_CONSTANT(DOCUMENTNAMETYPE_FILE_TAIL);  // @const axdebug|DOCUMENTNAMETYPE_FILE_TAIL|Gets the filename without a
                                               // path (for save as...)
    ADD_CONSTANT(DOCUMENTNAMETYPE_URL);  // @const axdebug|DOCUMENTNAMETYPE_URL|Gets the URL of the document, if any

    ADD_CONSTANT(DBGPROP_ATTRIB_NO_ATTRIB);             // @const axdebug|DBGPROP_ATTRIB_NO_ATTRIB|
    ADD_CONSTANT(DBGPROP_ATTRIB_VALUE_IS_INVALID);      // @const axdebug|DBGPROP_ATTRIB_VALUE_IS_INVALID|
    ADD_CONSTANT(DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE);   // @const axdebug|DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE|
    ADD_CONSTANT(DBGPROP_ATTRIB_VALUE_READONLY);        // @const axdebug|DBGPROP_ATTRIB_VALUE_READONLY|
    ADD_CONSTANT(DBGPROP_ATTRIB_ACCESS_PUBLIC);         // @const axdebug|DBGPROP_ATTRIB_ACCESS_PUBLIC|
    ADD_CONSTANT(DBGPROP_ATTRIB_ACCESS_PRIVATE);        // @const axdebug|DBGPROP_ATTRIB_ACCESS_PRIVATE|
    ADD_CONSTANT(DBGPROP_ATTRIB_ACCESS_PROTECTED);      // @const axdebug|DBGPROP_ATTRIB_ACCESS_PROTECTED|
    ADD_CONSTANT(DBGPROP_ATTRIB_ACCESS_FINAL);          // @const axdebug|DBGPROP_ATTRIB_ACCESS_FINAL|
    ADD_CONSTANT(DBGPROP_ATTRIB_STORAGE_GLOBAL);        // @const axdebug|DBGPROP_ATTRIB_STORAGE_GLOBAL|
    ADD_CONSTANT(DBGPROP_ATTRIB_STORAGE_STATIC);        // @const axdebug|DBGPROP_ATTRIB_STORAGE_STATIC|
    ADD_CONSTANT(DBGPROP_ATTRIB_STORAGE_FIELD);         // @const axdebug|DBGPROP_ATTRIB_STORAGE_FIELD|
    ADD_CONSTANT(DBGPROP_ATTRIB_STORAGE_VIRTUAL);       // @const axdebug|DBGPROP_ATTRIB_STORAGE_VIRTUAL|
    ADD_CONSTANT(DBGPROP_ATTRIB_TYPE_IS_CONSTANT);      // @const axdebug|DBGPROP_ATTRIB_TYPE_IS_CONSTANT|
    ADD_CONSTANT(DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED);  // @const axdebug|DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED|
    ADD_CONSTANT(DBGPROP_ATTRIB_TYPE_IS_VOLATILE);      // @const axdebug|DBGPROP_ATTRIB_TYPE_IS_VOLATILE|
    ADD_CONSTANT(DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS);  // @const axdebug|DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS|

    ADD_CONSTANT(DBGPROP_INFO_NAME);        // @const axdebug|DBGPROP_INFO_NAME|
    ADD_CONSTANT(DBGPROP_INFO_TYPE);        // @const axdebug|DBGPROP_INFO_TYPE|
    ADD_CONSTANT(DBGPROP_INFO_VALUE);       // @const axdebug|DBGPROP_INFO_VALUE|
    ADD_CONSTANT(DBGPROP_INFO_FULLNAME);    // @const axdebug|DBGPROP_INFO_FULLNAME|
    ADD_CONSTANT(DBGPROP_INFO_ATTRIBUTES);  // @const axdebug|DBGPROP_INFO_ATTRIBUTES|
    ADD_CONSTANT(DBGPROP_INFO_DEBUGPROP);   // @const axdebug|DBGPROP_INFO_DEBUGPROP|
    ADD_CONSTANT(DBGPROP_INFO_AUTOEXPAND);  // @const axdebug|DBGPROP_INFO_AUTOEXPAND|

    ADD_CONSTANT(
        ERRORRESUMEACTION_ReexecuteErrorStatement);  // @const axdebug|ERRORRESUMEACTION_ReexecuteErrorStatement|
    ADD_CONSTANT(
        ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller);  // @const
                                                             // axdebug|ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller|
    ADD_CONSTANT(ERRORRESUMEACTION_SkipErrorStatement);  // @const axdebug|ERRORRESUMEACTION_SkipErrorStatement|

    ADD_CONSTANT(EX_DBGPROP_INFO_ID);            // @const axdebug|EX_DBGPROP_INFO_ID|
    ADD_CONSTANT(EX_DBGPROP_INFO_NTYPE);         // @const axdebug|EX_DBGPROP_INFO_NTYPE|
    ADD_CONSTANT(EX_DBGPROP_INFO_NVALUE);        // @const axdebug|EX_DBGPROP_INFO_NVALUE|
    ADD_CONSTANT(EX_DBGPROP_INFO_LOCKBYTES);     // @const axdebug|EX_DBGPROP_INFO_LOCKBYTES|
    ADD_CONSTANT(EX_DBGPROP_INFO_DEBUGEXTPROP);  // @const axdebug|EX_DBGPROP_INFO_DEBUGEXTPROP|

    ADD_CONSTANT(SOURCETEXT_ATTR_KEYWORD);         // @const axdebug|SOURCETEXT_ATTR_KEYWORD|
    ADD_CONSTANT(SOURCETEXT_ATTR_COMMENT);         // @const axdebug|SOURCETEXT_ATTR_COMMENT|
    ADD_CONSTANT(SOURCETEXT_ATTR_NONSOURCE);       // @const axdebug|SOURCETEXT_ATTR_NONSOURCE|
    ADD_CONSTANT(SOURCETEXT_ATTR_OPERATOR);        // @const axdebug|SOURCETEXT_ATTR_OPERATOR|
    ADD_CONSTANT(SOURCETEXT_ATTR_NUMBER);          // @const axdebug|SOURCETEXT_ATTR_NUMBER|
    ADD_CONSTANT(SOURCETEXT_ATTR_STRING);          // @const axdebug|SOURCETEXT_ATTR_STRING|
    ADD_CONSTANT(SOURCETEXT_ATTR_FUNCTION_START);  // @const axdebug|SOURCETEXT_ATTR_FUNCTION_START|

    ADD_CONSTANT(
        TEXT_DOC_ATTR_READONLY);  // @const axdebug|TEXT_DOC_ATTR_READONLY|Indicates that the document is read-only.

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
