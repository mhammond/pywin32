#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGPersist::GetClassID(
    /* [out] */ CLSID __RPC_FAR *pClassID)
{
    if (pClassID == NULL)
        return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

    HRESULT hr;
    PY_GATEWAY_METHOD;
    PyObject *result;
    hr = InvokeViaPolicy("GetClassID", &result);
    if (SUCCEEDED(hr)) {
        hr = PyWinObject_AsIID(result, pClassID) ? S_OK : E_FAIL;
        Py_DECREF(result);

        // register the error if necessary
        PyCom_SetCOMErrorFromPyException(GetIID());
    }
    return hr;
}
