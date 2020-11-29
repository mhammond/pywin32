// @doc - This file contains autoduck documentation
#include "shell_pch.h"
#include "PyIUniformResourceLocator.h"

PyIUniformResourceLocator::PyIUniformResourceLocator(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIUniformResourceLocator::~PyIUniformResourceLocator() {}

IUniformResourceLocator *PyIUniformResourceLocator::GetI(PyObject *self)
{
    return (IUniformResourceLocator *)PyIUnknown::GetI(self);
}

// @pymethod str|PyIUniformResourceLocator|GetURL|Returns the URL for the shortcut
PyObject *PyIUniformResourceLocator::GetURL(PyObject *self, PyObject *args)
{
    IUniformResourceLocator *pIURL = GetI(self);
    if (pIURL == NULL)
        return NULL;
    PyObject *ret;
    TCHAR *url;
    if (!PyArg_ParseTuple(args, ":GetURL"))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIURL->GetURL(&url);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIURL, IID_IUniformResourceLocator);
    if (url == NULL) {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    else {
        ret = PyWinObject_FromTCHAR(url);
        CoTaskMemFree(url);
    }
    return ret;
}

// @pymethod |PyIUniformResourceLocator|SetURL|Sets the URL for the shortcut
PyObject *PyIUniformResourceLocator::SetURL(PyObject *self, PyObject *args)
{
    IUniformResourceLocator *pIURL = GetI(self);
    if (pIURL == NULL)
        return NULL;
    PyObject *oburl;
    LPTSTR url;
    DWORD flags;
    if (!PyArg_ParseTuple(args, "O|k:SetURL",
                          &oburl,   // @pyparm str|URL||The url to be set
                          &flags))  // @pyparm int|InFlags|0|One of the shellcon.IURL_SETURL* flags
        return NULL;
    if (!PyWinObject_AsTCHAR(oburl, &url))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIURL->SetURL(url, flags);
    PY_INTERFACE_POSTCALL;

    PyWinObject_FreeTCHAR(url);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIURL, IID_IUniformResourceLocator);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|PyIUniformResourceLocator|InvokeCommand|Performs one of the object's predefined actions
PyObject *PyIUniformResourceLocator::InvokeCommand(PyObject *self, PyObject *args)
{
    IUniformResourceLocator *pIURL = GetI(self);
    if (pIURL == NULL)
        return NULL;
    URLINVOKECOMMANDINFO parms;
    ZeroMemory(&parms, sizeof(parms));
    parms.dwcbSize = sizeof(parms);
    PyObject *obVerb;
    if (!PyArg_ParseTuple(args, "O|kl:InvokeCommand",
                          &obVerb,             // @pyparm str|Verb||The verb to be invoked
                          &parms.dwFlags,      // @pyparm int|Flags|0|Combination of shellcon.IURL_INVOKECOMMAND_* flags
                          &parms.hwndParent))  // @pyparm <o PyHANDLE>|hwndParent|0|Handle to parent window
        return NULL;
    if (!PyWinObject_AsTCHAR(obVerb, (TCHAR **)&parms.pcszVerb, TRUE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIURL->InvokeCommand(&parms);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeTCHAR((TCHAR *)parms.pcszVerb);

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIURL, IID_IUniformResourceLocator);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIUniformResourceLocator|Interface to an internet shortcut

static struct PyMethodDef PyIUniformResourceLocator_methods[] = {
    // @pymeth GetURL|Returns the URL for the shortcut
    {"GetURL", PyIUniformResourceLocator::GetURL, METH_VARARGS, "Returns the URL for the shortcut"},
    // @pymeth SetURL|Sets the URL for the shortcut
    {"SetURL", PyIUniformResourceLocator::SetURL, METH_VARARGS, "Sets the URL for the shortcut"},
    // @pymeth InvokeCommand|Performs one of the object's predefined actions
    {"InvokeCommand", PyIUniformResourceLocator::InvokeCommand, METH_VARARGS,
     "Performs one of the object's predefined actions"},
    {NULL}};

PyComTypeObject PyIUniformResourceLocator::type("PyIUniformResourceLocator", &PyIUnknown::type,
                                                sizeof(PyIUniformResourceLocator), PyIUniformResourceLocator_methods,
                                                GET_PYCOM_CTOR(PyIUniformResourceLocator));
