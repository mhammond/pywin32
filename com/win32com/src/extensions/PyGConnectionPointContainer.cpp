#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include <PyGConnectionPointContainer.h>
#include "olectl.h"  // For the error codes

STDMETHODIMP PyGConnectionPointContainer::EnumConnectionPoints(IEnumConnectionPoints **)
{
    /*
        PY_GATEWAY_METHOD;
        return InvokeViaPolicy("EnumConnectionPoints", NULL, NULL);
    */
    return E_NOTIMPL;
}

STDMETHODIMP PyGConnectionPointContainer::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
    PY_GATEWAY_METHOD;
    if (ppCP == NULL)
        return E_POINTER;
    *ppCP = NULL;
    PyObject *obIID = PyWinObject_FromIID(riid);
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("FindConnectionPoint", &result, "O", obIID);
    Py_XDECREF(obIID);
    if (FAILED(hr))
        return hr;
    if (result != Py_None && !PyCom_InterfaceFromPyObject(result, IID_IConnectionPoint, (void **)ppCP))
        hr = PyCom_SetCOMErrorFromPyException(GetIID());
    Py_XDECREF(result);
    return (hr == S_OK && *ppCP == NULL) ? CONNECT_E_NOCONNECTION : hr;
}
