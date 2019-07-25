// PyIBindCtx

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIBindCtx.h"

PyObject *PyWinObject_FromBIND_OPTS(BIND_OPTS *pbind_opts)
{
    return Py_BuildValue("{s:k,s:k,s:k,s:k}", "cbStruct", pbind_opts->cbStruct, "Flags", pbind_opts->grfFlags, "Mode",
                         pbind_opts->grfMode, "TickCountDeadline", pbind_opts->dwTickCountDeadline);
}

// @object PyBIND_OPTS|Dictionary representation of a BIND_OPTS struct
// May eventually be extended to include BIND_OPTS2 members
BOOL PyWinObject_AsBIND_OPTS(PyObject *obbind_opts, BIND_OPTS *pbind_opts)
{
    static char *BIND_OPTS_keywords[] = {"Flags", "Mode", "TickCountDeadline", "cbStruct", NULL};
    ZeroMemory(pbind_opts, sizeof(BIND_OPTS));
    if (!PyDict_Check(obbind_opts)) {
        PyErr_SetString(PyExc_TypeError, "BIND_OPTS must be a dictionary");
        return false;
    }
    PyObject *dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    BOOL bsuccess = PyArg_ParseTupleAndKeywords(
        dummy_args, obbind_opts, "kkk|k:BIND_OPTS", BIND_OPTS_keywords,
        &pbind_opts
             ->grfFlags,  // @prop int|Flags|Value from BIND_FLAGS enum: BIND_MAYBOTHERUSER, BIND_JUSTTESTEXISTENCE or 0
        &pbind_opts->grfMode,              // @prop int|Mode|Combination of storagecon.STGM_* values
        &pbind_opts->dwTickCountDeadline,  // @prop int|TickCountDeadline|Operation timeout in milliseconds
        &pbind_opts->cbStruct);            // @prop int|cbStruct|Size of struct, ignored on input
    pbind_opts->cbStruct = sizeof(BIND_OPTS);
    Py_DECREF(dummy_args);
    return bsuccess;
}

PyIBindCtx::PyIBindCtx(IUnknown *pDisp) : PyIUnknown(pDisp) { ob_type = &type; }

PyIBindCtx::~PyIBindCtx() {}

/*static*/ IBindCtx *PyIBindCtx::GetI(PyObject *self) { return (IBindCtx *)PyIUnknown::GetI(self); }

// @pymethod <o PyIRunningObjectTable>|PyIBindCtx|GetRunningObjectTable|Retrieves an object interfacing to the Running
// Object Table.
PyObject *PyIBindCtx::GetRunningObjectTable(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetRunningObjectTable"))
        return NULL;
    IBindCtx *pMy = GetI(self);
    if (!pMy)
        return NULL;
    IRunningObjectTable *pROT;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->GetRunningObjectTable(&pROT);
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr, pMy, IID_IRunningObjectTable);
    return PyCom_PyObjectFromIUnknown(pROT, IID_IRunningObjectTable, FALSE);
}

// @pymethod <o PyBIND_OPTS>|PyIBindCtx|GetBindOptions|Retrieves the bind options for the bind context
PyObject *PyIBindCtx::GetBindOptions(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    BIND_OPTS bind_opts;
    ZeroMemory(&bind_opts, sizeof(bind_opts));
    bind_opts.cbStruct = sizeof(bind_opts);

    if (!PyArg_ParseTuple(args, ":GetBindOptions"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pBC->GetBindOptions(&bind_opts);
    PY_INTERFACE_POSTCALL;
    if (hr != S_OK)
        return PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
    return PyWinObject_FromBIND_OPTS(&bind_opts);
}

// @pymethod |PyIBindCtx|SetBindOptions|Sets the bind options for the context
PyObject *PyIBindCtx::SetBindOptions(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    PyObject *obbind_opts;
    BIND_OPTS bind_opts;
    ZeroMemory(&bind_opts, sizeof(bind_opts));
    bind_opts.cbStruct = sizeof(bind_opts);

    if (!PyArg_ParseTuple(
            args, "O:SetBindOptions",
            &obbind_opts))  // @pyparm dict|bindopts||<o PyBIND_OPTS> dictionary containing the binding options
        return NULL;
    if (!PyWinObject_AsBIND_OPTS(obbind_opts, &bind_opts))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pBC->SetBindOptions(&bind_opts);
    PY_INTERFACE_POSTCALL;
    if (hr != S_OK)
        return PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIBindCtx|RegisterObjectParam|Adds an object to the context's keyed table of associated objects
PyObject *PyIBindCtx::RegisterObjectParam(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    PyObject *obkey, *obunk, *ret = NULL;
    WCHAR *key = NULL;
    IUnknown *punk = NULL;
    HRESULT hr;

    if (!PyArg_ParseTuple(args, "OO:RegisterObjectParam",
                          &obkey,   // @pyparm <o PyUnicode>|Key||The string key for the object to be registered
                          &obunk))  // @pyparm <o PyIUnknown>|punk||COM object to be registered with the bind context
        return NULL;
    if (PyWinObject_AsWCHAR(obkey, &key, FALSE) &&
        PyCom_InterfaceFromPyObject(obunk, IID_IUnknown, (void **)&punk, FALSE)) {
        PY_INTERFACE_PRECALL;
        hr = pBC->RegisterObjectParam(key, punk);
        PY_INTERFACE_POSTCALL;
        if (hr != S_OK)
            PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    if (punk != NULL)
        punk->Release();
    PyWinObject_FreeWCHAR(key);
    return ret;
}

// @pymethod |PyIBindCtx|RevokeObjectParam|Removes one of the bind context's registered objects
PyObject *PyIBindCtx::RevokeObjectParam(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    PyObject *obkey;
    WCHAR *key = NULL;
    HRESULT hr;
    if (!PyArg_ParseTuple(args, "O:RevokeObjectParam",
                          &obkey))  // @pyparm <o PyUnicode>|Key||The string key for the object to be removed
        return NULL;
    if (!PyWinObject_AsWCHAR(obkey, &key, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = pBC->RevokeObjectParam(key);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(key);
    if (hr != S_OK)
        return PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIUnknown>|PyIBindCtx|GetObjectParam|Returns one of the bind context's associated objects
PyObject *PyIBindCtx::GetObjectParam(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    PyObject *obkey;
    WCHAR *key = NULL;
    HRESULT hr;
    IUnknown *punk;
    if (!PyArg_ParseTuple(args, "O:GetObjectParam",
                          &obkey))  // @pyparm <o PyUnicode>|Key||The string key for the object to be returned
        return NULL;
    if (!PyWinObject_AsWCHAR(obkey, &key, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = pBC->GetObjectParam(key, &punk);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(key);
    if (hr != S_OK)
        return PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
    return PyCom_PyObjectFromIUnknown(punk, IID_IUnknown, FALSE);
}

// @pymethod <o PyIEnumString>|PyIBindCtx|EnumObjectParam|Creates an enumerator to list context's string keys
PyObject *PyIBindCtx::EnumObjectParam(PyObject *self, PyObject *args)
{
    IBindCtx *pBC = GetI(self);
    if (!pBC)
        return NULL;
    HRESULT hr;
    IEnumString *pIES;
    if (!PyArg_ParseTuple(args, ":EnumObjectParam"))
        return NULL;

    PY_INTERFACE_PRECALL;
    hr = pBC->EnumObjectParam(&pIES);
    PY_INTERFACE_POSTCALL;
    if (hr != S_OK)
        return PyCom_BuildPyException(hr, pBC, IID_IBindCtx);
    return PyCom_PyObjectFromIUnknown(pIES, IID_IEnumString, FALSE);
}

// @object PyIBindCtx|A Python interface to IBindCtx.  Derived from <o PyIUnknown>
static struct PyMethodDef PyIBindCtx_methods[] = {
    // @pymeth GetRunningObjectTable|Retrieves the running object table.
    {"GetRunningObjectTable", PyIBindCtx::GetRunningObjectTable, METH_VARARGS, "Retrieves the running object table."},
    // @pymeth GetBindOptions|Retrieves bind options
    {"GetBindOptions", PyIBindCtx::GetBindOptions, METH_VARARGS, "Retrieves bind options as a PyBIND_OPTS dict"},
    // @pymeth SetBindOptions|Sets the bind options for the bind context
    {"SetBindOptions", PyIBindCtx::SetBindOptions, METH_VARARGS, "Sets the bind options for the bind context"},
    // @pymeth RegisterObjectParam|Associates a COM object to the bind context
    {"RegisterObjectParam", PyIBindCtx::RegisterObjectParam, METH_VARARGS,
     "Associates a COM object to the bind context"},
    // @pymeth RevokeObjectParam|Removes one of the bind context's associated objects
    {"RevokeObjectParam", PyIBindCtx::RevokeObjectParam, METH_VARARGS,
     "Removes one of the bind context's associated objects"},
    // @pymeth GetObjectParam|Retrieves one of the contexts string-keyed objects
    {"GetObjectParam", PyIBindCtx::GetObjectParam, METH_VARARGS, "Retrieves one of the context's string-keyed objects"},
    // @pymeth EnumObjectParam|Creates an enumerator to list context's string keys
    {"EnumObjectParam", PyIBindCtx::EnumObjectParam, METH_VARARGS,
     "Creates an enumerator to list context's string keys"},
    {NULL, NULL}};

PyComTypeObject PyIBindCtx::type("PyIBindCtx",
                                 &PyIUnknown::type,  // @base PyIBindCtx|PyIUnknown
                                 sizeof(PyIBindCtx), PyIBindCtx_methods, GET_PYCOM_CTOR(PyIBindCtx));
