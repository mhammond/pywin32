// @doc
#include "stdafxdde.h"
#include "ddemodule.h"

BOOL PythonDDEServer::OnCreate()
{
    CVirtualHelper helper("OnCreate", this);
    if (helper.HaveHandler())
        return helper.call();
    else
        return TRUE;
}

void PythonDDEServer::Status(const TCHAR *pszFormat, ...)
{
    TCHAR buf[1024];
    va_list marker;
    va_start(marker, pszFormat);
    wvsprintf(buf, pszFormat, marker);
    va_end(marker);
    CVirtualHelper helper("Status", this);
    helper.call(buf);
}

CDDEServerSystemTopic *PythonDDEServer::CreateSystemTopic()
{
    CVirtualHelper helper("CreateSystemTopic", this);
    PyObject *ob;
    if (helper.call() && helper.retval(ob)) {
        CEnterLeavePython _celp;
        Py_XDECREF(m_obSystemTopic);
        CDDEServerSystemTopic *pT;
        if (pT = PyDDEServerSystemTopic::GetTopic(ob)) {
            m_obSystemTopic = ob;
            Py_INCREF(m_obSystemTopic);
            return pT;
        }
    }
    return new CDDEServerSystemTopic();
}

/*static*/ PythonDDEServer *PyDDEServer::GetServer(PyObject *self)
{
    return (PythonDDEServer *)ui_assoc_object::GetGoodCppObject(self, &type);
}

// @pymethod |PyDDEServer|Create|Create a server
PyObject *PyDDEServer_Create(PyObject *self, PyObject *args)
{
    TCHAR *serviceName;
    DWORD flags = 0;
    PyObject *observiceName;
    PythonDDEServer *pServer = PyDDEServer::GetServer(self);
    if (!pServer)
        return NULL;
    // @pyparm string|name||Name of the server to start.
    // @pyparm int|filterFlags|0|Filter flags.
    if (!PyArg_ParseTuple(args, "O|i:Create", &observiceName, &flags))
        return NULL;
    if (!PyWinObject_AsTCHAR(observiceName, &serviceName, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pServer->Create(serviceName, flags);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(serviceName);
    if (!ok)
        RETURN_DDE_ERR("The server could not be created");
    RETURN_NONE;
    // @comm Note there can only be one server per application.
}

// @pymethod |PyDDEServer|AddTopic|
PyObject *PyDDEServer_AddTopic(PyObject *self, PyObject *args)
{
    PyObject *obTopic;
    PythonDDEServer *pServer = PyDDEServer::GetServer(self);
    if (!pServer)
        return NULL;
    // @pyparm <o PyDDETopic>|topic||The topic to add.
    if (!PyArg_ParseTuple(args, "O:AddTopic", &obTopic))
        return NULL;
    GUI_BGN_SAVE;
    PythonDDETopic *pTopic = PyDDETopic::GetTopic(obTopic);
    BOOL ok = pTopic != NULL;
    ok = ok && pServer->AddTopic(pTopic);
    GUI_END_SAVE;
    if (!ok)
        RETURN_DDE_ERR("GetTopic or AddTopic failed.");
    RETURN_NONE;
}

// @pymethod int|PyDDEServer|GetLastError|
PyObject *PyDDEServer_GetLastError(PyObject *self, PyObject *args)
{
    PythonDDEServer *pServer = PyDDEServer::GetServer(self);
    if (!pServer)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetLastError"))
        return NULL;
    GUI_BGN_SAVE;
    DWORD dwErr = pServer->GetLastError();
    GUI_END_SAVE;
    return Py_BuildValue("i", dwErr);
}

// @pymethod |PyDDEServer|Destroy|
PyObject *PyDDEServer_Destroy(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Destroy"))
        return NULL;
    PythonDDEServer *pServer = PyDDEServer::GetServer(self);
    if (!pServer)
        return NULL;
    GUI_BGN_SAVE;
    delete pServer;
    GUI_END_SAVE;
    RETURN_NONE;
}
// @pymethod |PyDDEServer|Shutdown|
PyObject *PyDDEServer_Shutdown(PyObject *self, PyObject *args)
{
    // @comm Note the underlying DDE object (ie, Server, Topics and Items) are not cleaned up by this call.
    if (!PyArg_ParseTuple(args, ":Shutdown"))
        return NULL;
    PythonDDEServer *pServer = PyDDEServer::GetServer(self);
    if (!pServer)
        return NULL;
    GUI_BGN_SAVE;
    pServer->Shutdown();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @object PyDDEServer|A DDE server.
static struct PyMethodDef PyDDEServer_methods[] = {
    {"AddTopic", PyDDEServer_AddTopic, 1},          // @pymeth AddTopic|Adds a topic to the server.
    {"Create", PyDDEServer_Create, 1},              // @pymeth Create|Creates a DDE server
    {"Destroy", PyDDEServer_Destroy, 1},            // @pymeth Destroy|Destroys the underlying C++ object.
    {"GetLastError", PyDDEServer_GetLastError, 1},  // @pymeth GetLastError|Returns the last DDE error.
    {"Shutdown", PyDDEServer_Shutdown, 1},          // @pymeth Shutdown|Shutsdown the server.
    {NULL, NULL}                                    // sentinel
};

ui_type_CObject PyDDEServer::type("PyDDEServer", &ui_assoc_CObject::type, RUNTIME_CLASS(CDDEServer),
                                  sizeof(PyDDEServer), PYOBJ_OFFSET(PyDDEServer), PyDDEServer_methods,
                                  GET_PY_CTOR(PyDDEServer));
