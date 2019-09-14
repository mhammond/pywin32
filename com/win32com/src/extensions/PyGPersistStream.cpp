#include "stdafx.h"
#include "PyIPersistStream.h"
#include "PyIStream.h"

STDMETHODIMP PyGPersistStream::IsDirty(void)
{
    HRESULT hr;
    PY_GATEWAY_METHOD;
    PyObject *result;
    hr = InvokeViaPolicy("IsDirty", &result);
    if (SUCCEEDED(hr)) {
        /* returning 0 means not dirty. *anything* else means dirty */

        int dirty = PyInt_AsLong(result);
        Py_DECREF(result);
        hr = dirty == 0 ? S_FALSE : S_OK;
    }
    return hr;
}

STDMETHODIMP PyGPersistStream::Load(
    /* [unique][in] */ IStream __RPC_FAR *pStm)
{
    if (pStm == NULL)
        return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

    HRESULT hr;
    PY_GATEWAY_METHOD;
    PyObject *obStm = NULL;

    obStm = PyCom_PyObjectFromIUnknown(pStm, IID_IStream, TRUE);
    if (obStm == NULL)
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    else {
        hr = InvokeViaPolicy("Load", NULL, "O", obStm);
        Py_DECREF(obStm);
    }
    return hr;
}

STDMETHODIMP PyGPersistStream::Save(
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ BOOL fClearDirty)
{
    if (pStm == NULL)
        return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

    HRESULT hr;
    PY_GATEWAY_METHOD;
    PyObject *obStm = NULL;

    obStm = PyCom_PyObjectFromIUnknown(pStm, IID_IStream, TRUE);
    if (obStm == NULL)
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    else {
        hr = InvokeViaPolicy("Save", NULL, "Oi", obStm, (int)fClearDirty);
        Py_DECREF(obStm);
    }
    return hr;
}

STDMETHODIMP PyGPersistStream::GetSizeMax(
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize)
{
    if (pcbSize == NULL)
        return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

    HRESULT hr;
    PY_GATEWAY_METHOD;
    PyObject *result;
    hr = InvokeViaPolicy("GetSizeMax", &result);
    if (SUCCEEDED(hr)) {
        BOOL ok = PyWinObject_AsULARGE_INTEGER(result, pcbSize);
        Py_DECREF(result);
        PyErr_Clear(); /* just in case */
        hr = ok ? S_OK : E_FAIL;
    }
    return hr;
}
