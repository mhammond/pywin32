/*

    win32 command target

    Created July 1994, Mark Hammond (MHammond@skippinet.com.au)

  @doc
*/
#include "stdafx.h"

CCmdTarget *GetCCmdTargetPtr(PyObject *self)
{
    return (CCmdTarget *)ui_assoc_object::GetGoodCppObject(self, &PyCCmdTarget::type);
}

/////////////////////////////////////////////////////////////////////
//
// Command target object
//
//////////////////////////////////////////////////////////////////////
// @pymethod |PyCCmdTarget|BeginWaitCursor|
// Displays the cursor as an hourglass.  This can be used when you expect a
// command to take a noticeable time to execute (eg, when a document
// loads or saves itself to a file.).
// <nl>The actions of BeginWaitCursor are not always effective outside of a single
// message handler as other actions, such as OnSetCursor handling, could change
// the cursor.
// <nl>Call EndWaitCursor to restore the previous cursor.
static PyObject *PyCCmdTarget_begin_wait_cursor(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CCmdTarget *pCC = GetCCmdTargetPtr(self);
    if (!pCC)
        return NULL;
    pCC->BeginWaitCursor();  // @pyseemfc CWnd|BeginWaitCursor
    RETURN_NONE;
}

// @pymethod |PyCCmdTarget|EndWaitCursor|Ends a wait cursor.  Should only be called after <om PyCWnd.BeginWaitCursor>.
static PyObject *PyCCmdTarget_end_wait_cursor(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CCmdTarget *pCC = GetCCmdTargetPtr(self);
    if (!pCC)
        return NULL;
    pCC->EndWaitCursor();
    RETURN_NONE;
}

// @pymethod |PyCCmdTarget|RestoreWaitCursor|Restores the appropriate hourglass cursor after the system cursor has
// changed.
static PyObject *PyCCmdTarget_restore_wait_cursor(PyObject *self, PyObject *args)
{
    // @comm Call this function to restore the appropriate hourglass cursor after
    // the system cursor has changed (for example, after a message box has opened
    // and then closed while in the middle of a lengthy operation).
    CHECK_NO_ARGS(args);
    CCmdTarget *pCC = GetCCmdTargetPtr(self);
    if (!pCC)
        return NULL;
    pCC->RestoreWaitCursor();
    RETURN_NONE;
}

// @pymethod object|PyCCmdTarget|HookOleEvent|Hook an OLE Event.
static PyObject *PyCCmdTarget_hook_ole_event(PyObject *self, PyObject *args)
{
    PyCCmdTarget *s = (PyCCmdTarget *)self;
    // @rdesc The return value is the previous handler, or None.
    return add_hook_list(s, args, &s->pOleEventHookList);
}

// @pymethod object|PyCCmdTarget|HookCommand|Hook a windows command handler.
static PyObject *PyCCmdTarget_hook_command(PyObject *self, PyObject *args)
{
    // @comm obHandler will be called as the application receives command notification messages with the specified ID.
    // Command notification messages are usually sent in response to menu or toolbar commands.
    // <nl>When updating a user interface element, Pythonwin will first check if a
    // handler has been installed via  <om PyCCmdTarget.HookCommandUpdate>.  If so, this alone
    // determines the state of the interface object.  If no Update handler exists,
    // 	PythonWin will automatically enable a menu/toolbar item if a command handler exists
    // The handler will be called with 2 arguments
    // <nl>* The command id being handled.
    // <nl>* The command notification code.
    // <nl>If the handler returns TRUE, then the command will be passed on to the
    // default handler, otherwise the message will be consumed.
    // <nl>This method is best suited to handling messages from user interface
    // elements, such as menus, toolbars, etc.  To handle notification messages from a control,
    // you should use <om PyCCmdTarget.HookNotify>

    // @pyparm object|obHandler||The handler for the command message.  This must be a callable object.
    // @pyparm int|id||The ID of the command to be handled, or zero to handle all command messages.
    PyCCmdTarget *s = (PyCCmdTarget *)self;
    // @rdesc The return value is the previous handler, or None.
    return add_hook_list(s, args, &s->pCommandHookList);
}
// @pymethod object|PyCCmdTarget|HookCommandUpdate|Hook a windows command update handler.
static PyObject *PyCCmdTarget_hook_command_update(PyObject *self, PyObject *args)
{
    // @comm The handler object passed will be called as
    // the application updates user interface elements
    // with the specified ID.
    // See <om PyCCmdTarget.HookCommand> for a description
    // of the rules used to determine command routing and updating.

    // @pyparm object|obHandler||The handler for the command message.  This must be a callable object.
    // @pyparm int|id||The ID of the command to be handled.
    PyCCmdTarget *s = (PyCCmdTarget *)self;
    // @rdesc The return value is the previous handler, or None.
    return add_hook_list(s, args, &s->pCommandUpdateHookList);
}

// @pymethod object|PyCCmdTarget|HookNotify|Hook a windows command handler.
static PyObject *PyCCmdTarget_hook_notify(PyObject *self, PyObject *args)
{
    // @comm obHandler will be called as the application receives control notification messages.
    // These may also be handled via  <om PyCCmdTarget.HookCommand>, but this method is specific
    // to control notifications, and therefore provides more information.
    //
    // The handler will be called with 2 arguments<nl>
    // * A tuple describing standard notification information.<nl>
    // * A tuple describing extra notification params, or an integer containing the address of the first byte of the
    // extended information.<nl> If the handler returns TRUE, then the command will be passed on to the default handler,
    // otherwise the message will be consumed.
    //
    // Certain notification codes are recognised internally, and these are converted to a Python tuple.
    // If the extra information is not recognised, the address is passed.  These addresses could be
    // extracted using <om win32ui.GetBytes> and the struct module, or using
    // Sam Rushing's calldll/dynwin module. (It would be possible to extend Pythonwin so a program
    // can install certain knowledge about handlers, but this has not been implemented.)
    // @pyparm object|obHandler||The handler for the command message.  This must be a callable object.
    // @pyparm int|id||The ID of the command to be handled, or zero to handle all command messages.
    PyCCmdTarget *s = (PyCCmdTarget *)self;
    // @rdesc The return value is the previous handler, or None.
    return add_hook_list(s, args, &s->pNotifyHookList);
}

// @object PyCCmdTarget|An abstract command target class.  Encapsulates an MFC <c CCmdTarget> class
static struct PyMethodDef PyCCmdTarget_methods[] = {
    {"BeginWaitCursor", PyCCmdTarget_begin_wait_cursor,
     1},                                                 // @pymeth BeginWaitCursor|Displays the cursor as an hourglass.
    {"EndWaitCursor", PyCCmdTarget_end_wait_cursor, 1},  // @pymeth EndWaitCursor|End a wait cursor.
    {"HookCommand", PyCCmdTarget_hook_command, 1},       // @pymeth HookCommand|Hook a command handler.
    {"HookCommandUpdate", PyCCmdTarget_hook_command_update,
     1},  // @pymeth HookCommandUpdate|Hook a windows command update handler.
    {"HookOleEvent", PyCCmdTarget_hook_ole_event, 1},  // @pymeth HookOleEvent|Hooks an OLE event.
    {"HookNotify", PyCCmdTarget_hook_notify, 1},       // @pymeth HookNotify|Hook a control notification handler.
    {"RestoreWaitCursor", PyCCmdTarget_restore_wait_cursor,
     1},  // @pymeth RestoreWaitCursor|Restores the appropriate hourglass cursor after the system cursor has changed.
    {NULL, NULL}};

ui_type_CObject PyCCmdTarget::type("PyCCmdTarget", &ui_assoc_CObject::type, RUNTIME_CLASS(CCmdTarget),
                                   sizeof(PyCCmdTarget), PYOBJ_OFFSET(PyCCmdTarget), PyCCmdTarget_methods, NULL);
PyCCmdTarget::PyCCmdTarget()
{
    pOleEventHookList = NULL;
    pCommandHookList = NULL;
    pNotifyHookList = NULL;
    pCommandUpdateHookList = NULL;
    //	virtuals.SetOwner(this);
}
PyCCmdTarget::~PyCCmdTarget()
{
    free_hook_list(this, &pNotifyHookList);
    free_hook_list(this, &pOleEventHookList);
    free_hook_list(this, &pCommandHookList);
    free_hook_list(this, &pCommandUpdateHookList);
}

CString PyCCmdTarget::repr()
{
    CString csRet;
    SSIZE_T numCmd = pCommandHookList ? pCommandHookList->GetCount() : 0;
    SSIZE_T numNotify = pNotifyHookList ? pNotifyHookList->GetCount() : 0;
    SSIZE_T numCmdUpdate = pCommandUpdateHookList ? pCommandUpdateHookList->GetCount() : 0;
    SSIZE_T numOle = pOleEventHookList ? pOleEventHookList->GetCount() : 0;
    csRet.Format(_T(", notify=%Iu,ch/u=%Iu/%Iu"), numNotify, numCmd, numCmdUpdate);
    return ui_assoc_object::repr() + csRet;
}
/////////////////////////////////////////////////////////////////////
//
// add_hook_list
//
// keep a reference to the hooked object.
// Return old handler, or None
PyObject *add_hook_list(PyObject *hookedObject, PyObject *args, CMapWordToPtr **ppList)
{
    CMapWordToPtr *&pList = *ppList;
    if (pList == NULL)
        pList = new CMapWordToPtr();
    PyObject *method;
    int message;
    if (!PyArg_ParseTuple(args, "Oi", &method, &message))
        return NULL;
    if (method != Py_None && !PyCallable_Check(method))
        RETURN_ERR("The parameter must be a callable object or None");

    void *oldMethod = NULL;
    // note I maybe decref, then maybe incref.  To ensure the object will
    // not be destroyed (ie, ref go to zero) between the 2 calls), I
    // add a temporary reference first.
    DOINCREF(hookedObject);
    if (pList->Lookup(message, oldMethod)) {
        pList->RemoveKey(message);
        // oldMethod is returned - don't drop its reference.
        DODECREF(hookedObject);
    }
    if (method != Py_None) {
        Py_INCREF(method);
        pList->SetAt(message, method);
        Py_INCREF(hookedObject);
    }
    DODECREF(hookedObject);  // remove temp reference added above.
    if (oldMethod)
        return (PyObject *)oldMethod;
    else
        RETURN_NONE;
    //	RETURN_NONE;
}
//
// free_hook_list
//
// this is a bit nasty!  This function is called when the window itself
// is closed.  As all the hooks into the window are decref'd, it is possible
// (actually, likely!) that one of the member DODECREFS will also cause the
// window object itself to destruct (as the member function in my list was the
// last remaining (indirect) reference to the window) which also calls this.
// Therefore I set the list value to NULL before freeing the members, so
// the recursive call is not harmful.
void free_hook_list(PyObject *hookedObject, CMapWordToPtr **ppList)
{
    CMapWordToPtr *pList = *ppList;
    if (pList == NULL)
        return;  // nothing to do.
    *ppList = NULL;
    POSITION pos;
    void *method;
    WORD message;
    // Iterate through the entire map
    for (pos = pList->GetStartPosition(); pos != NULL;) {
        pList->GetNextAssoc(pos, message, method);
        Py_XDECREF((PyObject *)method);
        Py_XDECREF(hookedObject);
    }
    delete pList;
}
