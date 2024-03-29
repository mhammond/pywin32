// This file implements the IDebugStackFrame Interface and Gateway for Python.
// Generated by makegw.py

#include "stdafx.h"
#include "PyIDebugStackFrame.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIDebugStackFrame::PyIDebugStackFrame(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIDebugStackFrame::~PyIDebugStackFrame() {}

/* static */ IDebugStackFrame *PyIDebugStackFrame::GetI(PyObject *self)
{
    return (IDebugStackFrame *)PyIUnknown::GetI(self);
}

// @pymethod |PyIDebugStackFrame|GetCodeContext|Returns the current code context associated with the stack frame.
PyObject *PyIDebugStackFrame::GetCodeContext(PyObject *self, PyObject *args)
{
    IDebugStackFrame *pIDSF = GetI(self);
    if (pIDSF == NULL)
        return NULL;
    IDebugCodeContext *ppcc;
    if (!PyArg_ParseTuple(args, ":GetCodeContext"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIDSF->GetCodeContext(&ppcc);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);
    return PyCom_PyObjectFromIUnknown(ppcc, IID_IDebugCodeContext, FALSE);
}

// @pymethod <o unicode>|PyIDebugStackFrame|GetDescriptionString|Returns a short or long textual description of the
// stack frame.
PyObject *PyIDebugStackFrame::GetDescriptionString(PyObject *self, PyObject *args)
{
    IDebugStackFrame *pIDSF = GetI(self);
    if (pIDSF == NULL)
        return NULL;
    BSTR pbstrDescription;
    BOOL flong;
    // @pyparm int|fLong||If false, provide only the name of the function associated with the stack frame. When true it
    // may also provide the parameter(s) to the function or whatever else is relevant.
    if (!PyArg_ParseTuple(args, "i:GetDescriptionString", &flong))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIDSF->GetDescriptionString(flong, &pbstrDescription);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);
    PyObject *obpbstrDescription;

    obpbstrDescription = MakeBstrToObj(pbstrDescription);
    PyObject *pyretval = Py_BuildValue("O", obpbstrDescription);
    Py_XDECREF(obpbstrDescription);
    return pyretval;
}

// @pymethod <o unicode>|PyIDebugStackFrame|GetLanguageString|Returns a short or long textual description of the
// language.
PyObject *PyIDebugStackFrame::GetLanguageString(PyObject *self, PyObject *args)
{
    IDebugStackFrame *pIDSF = GetI(self);
    if (pIDSF == NULL)
        return NULL;
    BSTR pbstrDescription;
    BOOL flong;
    // @pyparm int|fLong||If False, just the language name should be provided, eg, "Python". If True a full product
    // description may be provided (eg, "Python X.X ActiveX Debugging Host")
    if (!PyArg_ParseTuple(args, "i:GetLanguageString", &flong))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIDSF->GetLanguageString(flong, &pbstrDescription);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);
    PyObject *obpbstrDescription;
    obpbstrDescription = MakeBstrToObj(pbstrDescription);
    PyObject *pyretval = Py_BuildValue("O", obpbstrDescription);
    Py_XDECREF(obpbstrDescription);
    return pyretval;
}

// @pymethod <o PyIDebugApplicationThread>|PyIDebugStackFrame|GetThread|Returns the thread associated with this stack
// frame.
PyObject *PyIDebugStackFrame::GetThread(PyObject *self, PyObject *args)
{
    IDebugStackFrame *pIDSF = GetI(self);
    if (pIDSF == NULL)
        return NULL;
    IDebugApplicationThread *ppat;
    if (!PyArg_ParseTuple(args, ":GetThread"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIDSF->GetThread(&ppat);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);
    return PyCom_PyObjectFromIUnknown(ppat, IID_IDebugApplicationThread, FALSE);
}

// @pymethod <o PyIDebugProperty>|PyIDebugStackFrame|GetDebugProperty|Returns the debug property.
PyObject *PyIDebugStackFrame::GetDebugProperty(PyObject *self, PyObject *args)
{
    IDebugStackFrame *pIDSF = GetI(self);
    if (pIDSF == NULL)
        return NULL;
    IDebugProperty *ppdp;
    if (!PyArg_ParseTuple(args, ":GetDebugProperty"))
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIDSF->GetDebugProperty(&ppdp);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);
    return PyCom_PyObjectFromIUnknown(ppdp, IID_IDebugProperty, FALSE);
}

// @object PyIDebugStackFrame|Description of the interface
static struct PyMethodDef PyIDebugStackFrame_methods[] = {
    {"GetCodeContext", PyIDebugStackFrame::GetCodeContext,
     1},  // @pymeth GetCodeContext|Returns the current code context associated with the stack frame.
    {"GetDescriptionString", PyIDebugStackFrame::GetDescriptionString,
     1},  // @pymeth GetDescriptionString|Returns a short or long textual description of the stack frame.
    {"GetLanguageString", PyIDebugStackFrame::GetLanguageString,
     1},  // @pymeth GetLanguageString|Returns a short or long textual description of the language.
    {"GetThread", PyIDebugStackFrame::GetThread,
     1},  // @pymeth GetThread|Returns the thread associated with this stack frame.
    {"GetDebugProperty", PyIDebugStackFrame::GetDebugProperty,
     1},  // @pymeth GetThread|Returns the debug property object associated with this stack frame.
    {NULL}};

PyComTypeObject PyIDebugStackFrame::type("PyIDebugStackFrame", &PyIUnknown::type, sizeof(PyIDebugStackFrame),
                                         PyIDebugStackFrame_methods, GET_PYCOM_CTOR(PyIDebugStackFrame));
// ---------------------------------------------------
//
// Gateway Implementation

STDMETHODIMP PyGDebugStackFrame::GetCodeContext(
    /* [out] */ IDebugCodeContext __RPC_FAR *__RPC_FAR *ppcc)
{
    PY_GATEWAY_METHOD;
    if (ppcc == NULL)
        return E_POINTER;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetCodeContext", &result);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params
    PyObject *obppcc;
    if (!PyArg_Parse(result, "O", &obppcc))
        return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    BOOL bPythonIsHappy = TRUE;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obppcc, IID_IDebugCodeContext, (void **)ppcc, FALSE /* bNoneOK */))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGDebugStackFrame::GetDescriptionString(
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR __RPC_FAR *pbstrDescription)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetDescriptionString", &result, "i", fLong);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params
    PyObject *obpbstrDescription;
    if (!PyArg_Parse(result, "O", &obpbstrDescription))
        return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    BOOL bPythonIsHappy = TRUE;
    if (!PyCom_BstrFromPyObject(obpbstrDescription, pbstrDescription))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGDebugStackFrame::GetLanguageString(
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR __RPC_FAR *pbstrDescription)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetLanguageString", &result, "i", fLong);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params
    PyObject *obpbstrDescription;
    if (!PyArg_Parse(result, "O", &obpbstrDescription))
        return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    BOOL bPythonIsHappy = TRUE;
    if (!PyCom_BstrFromPyObject(obpbstrDescription, pbstrDescription))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGDebugStackFrame::GetThread(
    /* [out] */ IDebugApplicationThread __RPC_FAR *__RPC_FAR *ppat)
{
    PY_GATEWAY_METHOD;
    if (ppat == NULL)
        return E_POINTER;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetThread", &result);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params
    PyObject *obppat;
    if (!PyArg_Parse(result, "O", &obppat))
        return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    BOOL bPythonIsHappy = TRUE;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obppat, IID_IDebugApplicationThread, (void **)ppat, FALSE /* bNoneOK */))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    Py_DECREF(result);
    return hr;
}

STDMETHODIMP PyGDebugStackFrame::GetDebugProperty(
    /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp)
{
    PY_GATEWAY_METHOD;
    if (ppdp == NULL)
        return E_POINTER;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetDebugProperty", &result);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params
    if (!PyCom_InterfaceFromPyInstanceOrObject(result, IID_IDebugProperty, (void **)ppdp, FALSE /* bNoneOK */))
        hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
    Py_DECREF(result);
    return hr;
}
