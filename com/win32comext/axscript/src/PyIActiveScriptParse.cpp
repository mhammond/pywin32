#include "stdafx.h"

PyIActiveScriptParse::PyIActiveScriptParse(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIActiveScriptParse::~PyIActiveScriptParse() {}

/* static */ IActiveScriptParse *PyIActiveScriptParse::GetI(PyObject *self)
{
    return (IActiveScriptParse *)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIActiveScriptParse::InitNew(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":InitNew"))
        return NULL;

    IActiveScriptParse *pIASP = GetI(self);
    if (pIASP == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIASP->InitNew();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);

    Py_INCREF(Py_None);
    return Py_None;
}

/* static */ PyObject *PyIActiveScriptParse::AddScriptlet(PyObject *self, PyObject *args)
{
    HRESULT hr;
    PyObject *ret = NULL;
    PyObject *obDefaultName, *obCode, *obItemName, *obSubItemName, *obEventName, *obDelimiter;
    int sourceContextCookie;
    int startingLineNumber;
    int flags;
    IActiveScriptParse *pIASP = GetI(self);
    if (pIASP == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "OOOOOOiii:AddScriptlet", &obDefaultName, &obCode, &obItemName, &obSubItemName,
                          &obEventName, &obDelimiter, &sourceContextCookie, &startingLineNumber, &flags))
        return NULL;
    BSTR bstrName;
    EXCEPINFO excepInfo;
    memset(&excepInfo, 0, sizeof excepInfo);
    WCHAR *defaultName = NULL, *code = NULL, *itemName = NULL, *subItemName = NULL, *eventName = NULL,
          *delimiter = NULL;
    if (!PyWinObject_AsWCHAR(obDefaultName, &defaultName, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obCode, &code, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obItemName, &itemName, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obSubItemName, &subItemName, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obEventName, &eventName, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obDelimiter, &delimiter, TRUE))
        goto done;
    {
        PY_INTERFACE_PRECALL;
        hr = pIASP->AddScriptlet(defaultName, code, itemName, subItemName, eventName, delimiter,
                                 (DWORD)sourceContextCookie, (ULONG)startingLineNumber, (DWORD)flags, &bstrName,
                                 &excepInfo);
        PY_INTERFACE_POSTCALL;
    }
    if (FAILED(hr)) {
        PyCom_BuildPyExceptionFromEXCEPINFO(hr, &excepInfo);
        goto done;
    }
    ret = MakeBstrToObj(bstrName);
done:
    PyWinObject_FreeWCHAR(defaultName);
    PyWinObject_FreeWCHAR(code);
    PyWinObject_FreeWCHAR(itemName);
    PyWinObject_FreeWCHAR(subItemName);
    PyWinObject_FreeWCHAR(eventName);
    PyWinObject_FreeWCHAR(delimiter);
    return ret;
}

/* static */ PyObject *PyIActiveScriptParse::ParseScriptText(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *obCode, *obItemName, *obDelimiter;
    WCHAR *code = NULL;
    WCHAR *itemName = NULL;
    PyObject *obContext;
    WCHAR *delimiter = NULL;
    int sourceContextCookie;
    int startingLineNumber;
    int flags;
    // Special handling for bWantResult.  If not specified, then
    // it looks at the flags for a reasonable default.  If specified
    // the flag is not used at all.
    BOOL bWantResult = (BOOL)-1;
    IActiveScriptParse *pIASP = GetI(self);
    if (pIASP == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "OOOOiii|i:ParseScriptText", &obCode, &obItemName, &obContext, &obDelimiter,
                          &sourceContextCookie, &startingLineNumber, &flags, &bWantResult))
        return NULL;
    IUnknown *punkContext = NULL;
    VARIANT *pResult = NULL;
    VARIANT varResult;
    EXCEPINFO excepInfo;
    memset(&excepInfo, 0, sizeof excepInfo);
    HRESULT hr;
    if (!PyWinObject_AsWCHAR(obCode, &code, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obItemName, &itemName, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obDelimiter, &delimiter, TRUE))
        goto done;

    if (bWantResult == -1)
        bWantResult = (flags & SCRIPTTEXT_ISEXPRESSION) != 0;
    if (obContext != Py_None) {
        if (!PyIBase::is_object(obContext, &PyIUnknown::type)) {
            PyErr_SetString(PyExc_ValueError, "argument is not a PyIUnknown");
            goto done;
        }
        punkContext = PyIUnknown::GetI(obContext);
        if (!punkContext)
            goto done;
        /* note: we don't explicitly hold a reference to punkContext */
    }
    if (bWantResult) {
        pResult = &varResult;
        VariantInit(&varResult);
    }
    {
        PY_INTERFACE_PRECALL;
        hr = pIASP->ParseScriptText(code, itemName, punkContext, delimiter, (DWORD)sourceContextCookie,
                                    (ULONG)startingLineNumber, (DWORD)flags, pResult, &excepInfo);
        PY_INTERFACE_POSTCALL;
    }
    if (FAILED(hr)) {
        PyCom_BuildPyExceptionFromEXCEPINFO(hr, &excepInfo);
        goto done;
    }

    if (bWantResult) {
        result = PyCom_PyObjectFromVariant(&varResult);
        VariantClear(&varResult);
    }
    else {
        Py_INCREF(Py_None);
        result = Py_None;
    }
done:
    PyWinObject_FreeWCHAR(code);
    PyWinObject_FreeWCHAR(itemName);
    PyWinObject_FreeWCHAR(delimiter);
    return result;
}

static struct PyMethodDef PyIActiveScriptParse_methods[] = {
    {"InitNew", PyIActiveScriptParse::InitNew, 1},
    {"AddScriptlet", PyIActiveScriptParse::AddScriptlet, 1},
    {"ParseScriptText", PyIActiveScriptParse::ParseScriptText, 1},
    {NULL}};

PyComTypeObject PyIActiveScriptParse::type("PyIActiveScriptParse", &PyIUnknown::type, sizeof(PyIActiveScriptParse),
                                           PyIActiveScriptParse_methods, GET_PYCOM_CTOR(PyIActiveScriptParse));
