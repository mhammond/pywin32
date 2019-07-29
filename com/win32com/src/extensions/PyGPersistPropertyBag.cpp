#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGPersistPropertyBag::InitNew(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("InitNew", NULL, NULL);
}

STDMETHODIMP PyGPersistPropertyBag::Load(
    /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
    /* [in] */ IErrorLog __RPC_FAR *pErrorLog)
{
    HRESULT hr;

    if (pPropBag == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *obLog;
    if (pErrorLog != NULL) {
        obLog = PyCom_PyObjectFromIUnknown(pErrorLog, IID_IErrorLog, TRUE);
        if (!obLog)
            return PyCom_SetCOMErrorFromPyException(GetIID());
    }
    else {
        Py_INCREF(Py_None);
        obLog = Py_None;
    }

    PyObject *obBag = PyCom_PyObjectFromIUnknown(pPropBag, IID_IPropertyBag, TRUE);
    if (!obBag) {
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
        Py_DECREF(obLog);
        return hr;
    }

    hr = InvokeViaPolicy("Load", NULL, "OO", obBag, obLog);
    Py_DECREF(obBag);
    Py_DECREF(obLog);
    return hr;
}

STDMETHODIMP PyGPersistPropertyBag::Save(
    /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
    /* [in] */ BOOL fClearDirty,
    /* [in] */ BOOL fSaveAllProperties)
{
    if (pPropBag == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *obBag = PyCom_PyObjectFromIUnknown(pPropBag, IID_IPropertyBag, TRUE);
    if (!obBag)
        return PyCom_SetCOMErrorFromPyException(GetIID());

    HRESULT hr = InvokeViaPolicy("Save", NULL, "Oii", obBag, (int)fClearDirty, (int)fSaveAllProperties);
    Py_DECREF(obBag);
    return hr;
}
