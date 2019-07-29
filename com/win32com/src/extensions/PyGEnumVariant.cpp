#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyIEnumVARIANT.h"

STDMETHODIMP PyGEnumVARIANT::Next(
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ VARIANT __RPC_FAR *rgVar,
    /* [out] */ ULONG __RPC_FAR *pCeltFetched)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("Next", &result, "i", celt);
    if (FAILED(hr))
        return hr;

    if (!PySequence_Check(result))
        goto error;
    int len;
    len = PyObject_Length(result);
    if (len == -1)
        goto error;
    if (len > (int)celt)
        len = celt;

    if (pCeltFetched)
        *pCeltFetched = len;

    int i;
    for (i = 0; i < len; ++i) {
        PyObject *ob = PySequence_GetItem(result, i);
        if (ob == NULL)
            goto error;

        if (!PyCom_VariantFromPyObject(ob, &rgVar[i])) {
            Py_DECREF(ob);
            Py_DECREF(result);
            return PyCom_SetCOMErrorFromPyException(IID_IEnumVARIANT);
        }
        Py_DECREF(ob);
    }

    Py_DECREF(result);

    return len < (int)celt ? S_FALSE : S_OK;

error:
    PyErr_Clear();  // just in case
    PyCom_LogF("PyGEnumVariant::Next got a bad return value");
    Py_DECREF(result);
    return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumVARIANT, "Next() did not return a sequence of objects");
}

STDMETHODIMP PyGEnumVARIANT::Skip(
    /* [in] */ ULONG celt)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("Skip", NULL, "i", celt);
}

STDMETHODIMP PyGEnumVARIANT::Reset(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("Reset");
}

STDMETHODIMP PyGEnumVARIANT::Clone(
    /* [out] */ IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("Clone", &result);
    if (FAILED(hr))
        return hr;

    /*
    ** Make sure we have the right kind of object: we should have some kind
    ** of IUnknown subclass wrapped into a PyIUnknown instance.
    */
    if (!PyIBase::is_object(result, &PyIUnknown::type)) {
        /* the wrong kind of object was returned to us */
        Py_DECREF(result);
        return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumVARIANT);
    }

    /*
    ** Get the IUnknown out of the thing. note that the Python ob maintains
    ** a reference, so we don't have to explicitly AddRef() here.
    */
    IUnknown *punk = ((PyIUnknown *)result)->m_obj;
    if (!punk) {
        /* damn. the object was released. */
        Py_DECREF(result);
        return PyCom_SetCOMErrorFromSimple(E_FAIL, IID_IEnumVARIANT);
    }

    /*
    ** Get the interface we want. note it is returned with a refcount.
    ** This QI is actually going to instantiate a PyGEnumVARIANT.
    */
    Py_BEGIN_ALLOW_THREADS hr = punk->QueryInterface(IID_IEnumVARIANT, (LPVOID *)ppEnum);
    Py_END_ALLOW_THREADS

        /* done with the result; this DECREF is also for <punk> */
        Py_DECREF(result);

    return PyCom_SetCOMErrorFromSimple(hr, IID_IEnumVARIANT);
}
