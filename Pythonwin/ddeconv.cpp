// @doc
#include "stdafxdde.h"
#include "ddemodule.h"

PythonDDEConv *PyDDEConv::GetConv(PyObject *self)
{
    return (PythonDDEConv *)ui_assoc_object::GetGoodCppObject(self, &type);
}

// @pymethod |PyDDEConv|ConnectTo|Connects to a server
PyObject *PyDDEConv_ConnectTo(PyObject *self, PyObject *args)
{
    PythonDDEConv *pConv = PyDDEConv::GetConv(self);
    if (!pConv)
        return NULL;
    TCHAR *szService = NULL, *szTopic = NULL;
    PyObject *obService, *obTopic, *ret = NULL;
    // @pyparm string|service||The service to connect to
    // @pyparm string|topic||The topic to connect to
    if (!PyArg_ParseTuple(args, "OO:ConnectTo", &obService, &obTopic))
        return NULL;
    if (PyWinObject_AsTCHAR(obService, &szService, FALSE) && PyWinObject_AsTCHAR(obTopic, &szTopic, FALSE)) {
        GUI_BGN_SAVE;
        BOOL ok = pConv->ConnectTo(szService, szTopic);
        GUI_END_SAVE;
        if (!ok)
            PyErr_SetString(dde_module_error, "ConnectTo failed");
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    PyWinObject_FreeTCHAR(szService);
    PyWinObject_FreeTCHAR(szTopic);
    return ret;
}

// @pymethod |PyDDEConv|Connected|Determines if the conversation is connected.
PyObject *PyDDEConv_Connected(PyObject *self, PyObject *args)
{
    PythonDDEConv *pConv = PyDDEConv::GetConv(self);
    if (!pConv)
        return NULL;
    if (!PyArg_ParseTuple(args, ":Connected"))
        return NULL;
    GUI_BGN_SAVE;
    BOOL rc = pConv->Connected();
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod |PyDDEConv|Exec|Executes a command.
PyObject *PyDDEConv_Exec(PyObject *self, PyObject *args)
{
    PythonDDEConv *pConv = PyDDEConv::GetConv(self);
    if (!pConv)
        return NULL;
    TCHAR *szCmd;
    PyObject *obCmd;
    // @pyparm string|Cmd||The Python statement to execute
    if (!PyArg_ParseTuple(args, "O:Exec", &obCmd))
        return NULL;
    if (!PyWinObject_AsTCHAR(obCmd, &szCmd, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pConv->Exec(szCmd);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szCmd);
    if (!ok)
        RETURN_DDE_ERR("Exec failed");
    RETURN_NONE;
}

// @pymethod |PyDDEConv|Request|Sends a request.
PyObject *PyDDEConv_Request(PyObject *self, PyObject *args)
{
    PythonDDEConv *pConv = PyDDEConv::GetConv(self);
    if (!pConv)
        return NULL;
    TCHAR *szCmd;
    PyObject *obCmd;
    if (!PyArg_ParseTuple(args, "O:Request", &obCmd))
        return NULL;
    if (!PyWinObject_AsTCHAR(obCmd, &szCmd, FALSE))
        return NULL;
    CString ret;
    GUI_BGN_SAVE;
    BOOL ok = pConv->Request(szCmd, ret);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szCmd);
    if (!ok)
        RETURN_DDE_ERR("Request failed");
    PyObject *result = PyWinObject_FromTCHAR((const TCHAR *)ret);
    return result;
}

// @pymethod |PyDDEConv|Poke|Sends a poke.
PyObject *PyDDEConv_Poke(PyObject *self, PyObject *args)
{
    PythonDDEConv *pConv = PyDDEConv::GetConv(self);
    if (!pConv)
        return NULL;
    TCHAR *szCmd;
    PyObject *obCmd;
    void *pData = NULL;  // may be empty, as for Netscape's use of Poke
    DWORD dwSize = 0;
    if (!PyArg_ParseTuple(args, "O|z#:Poke", &obCmd, &pData, &dwSize))
        return NULL;
    if (!PyWinObject_AsTCHAR(obCmd, &szCmd, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pConv->Poke(szCmd, pData, dwSize);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(szCmd);
    if (!ok)
        RETURN_DDE_ERR("Poke failed");
    RETURN_NONE;
}

// @object PyDDEConv|A DDE topic.
static struct PyMethodDef PyDDEConv_methods[] = {
    {"ConnectTo", PyDDEConv_ConnectTo, 1},  // @pymeth ConnectTo|Connects to a server
    {"Connected", PyDDEConv_Connected, 1},  // @pymeth Connected|Determines if a connection has been made.
    {"Exec", PyDDEConv_Exec, 1},            // @pymeth Exec|Executes a command.
    {"Request", PyDDEConv_Request, 1},      // @pymeth Request|Sends a request.
    {"Poke", PyDDEConv_Poke, 1},            // @pymeth Poke|Sends a poke.
    {NULL, NULL}                            // sentinel
};

ui_type_CObject PyDDEConv::type("PyDDEConv", &ui_assoc_CObject::type, RUNTIME_CLASS(CDDEConv), sizeof(PyDDEConv),
                                PYOBJ_OFFSET(PyDDEConv), PyDDEConv_methods, GET_PY_CTOR(PyDDEConv));
