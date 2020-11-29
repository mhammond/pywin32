#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

STDMETHODIMP PyGErrorLog::AddError(
    /* [in] */ LPCOLESTR pszPropName,
    /* [in] */ EXCEPINFO __RPC_FAR *pExcepInfo)
{
    if (pszPropName == NULL || pExcepInfo == NULL)
        return E_POINTER;

    PY_GATEWAY_METHOD;
    PyObject *obExcepInfo = PyCom_PyObjectFromExcepInfo(pExcepInfo);
    if (!obExcepInfo)
        return PyCom_SetCOMErrorFromPyException(GetIID());

    PyObject *obName = PyWinObject_FromWCHAR(pszPropName);
    HRESULT hr = InvokeViaPolicy("AddError", NULL, "OO", obName, obExcepInfo);
    Py_DECREF(obExcepInfo);
    Py_XDECREF(obName);
    return hr;
}
