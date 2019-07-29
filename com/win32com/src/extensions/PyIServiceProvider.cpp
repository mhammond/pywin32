#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_ISERVICEPROVIDER

#include "PythonCOMServer.h"
#include "PyIServiceProvider.h"
// @doc
PyIServiceProvider::PyIServiceProvider(IUnknown *pDisp) : PyIUnknown(pDisp) { ob_type = &type; }

PyIServiceProvider::~PyIServiceProvider() {}

/*static*/ IServiceProvider *PyIServiceProvider::GetI(PyObject *self)
{
    return (IServiceProvider *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIUnknown>|PyIServiceProvider|QueryService|Creates or accesses the specified service and returns an
// interface object to the specified interface for the service.
PyObject *PyIServiceProvider::QueryService(PyObject *self, PyObject *args)
{
    PyObject *obiid, *obclsid;
    if (!PyArg_ParseTuple(
            args, "OO:QueryService",
            &obclsid,  // @pyparm <o PyIID>|clsid||Unique identifier for the requested service.
            &obiid))   // @pyparm <o PyIID>|iid||Unique identifier for the requested interface on the service.
        return NULL;
    GUID clsid;
    if (!PyWinObject_AsIID(obclsid, &clsid))
        return NULL;
    IID iid;
    if (!PyWinObject_AsIID(obiid, &iid))
        return NULL;

    IServiceProvider *pMy = GetI(self);
    if (!pMy)
        return NULL;

    IUnknown *pv;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->QueryService(clsid, iid, (void **)&pv);
    PY_INTERFACE_POSTCALL;
    if (S_OK != hr)
        return PyCom_BuildPyException(hr, pMy, IID_IServiceProvider);
    return PyCom_PyObjectFromIUnknown(pv, iid, /* bAddRef = */ FALSE);
}

// @object PyIServiceProvider|A Python interface to IServiceProvider
static struct PyMethodDef PyIServiceProvider_methods[] = {
    {"QueryService", PyIServiceProvider::QueryService,
     1},  // @pymeth QueryService|Creates or accesses the specified service and returns an interface object to the
          // specified interface for the service.
    {NULL, NULL}};

PyComTypeObject PyIServiceProvider::type("PyIServiceProvider",
                                         &PyIUnknown::type,  // @base PyIServiceProvider|PyIUnknown
                                         sizeof(PyIServiceProvider), PyIServiceProvider_methods,
                                         GET_PYCOM_CTOR(PyIServiceProvider));

STDMETHODIMP PyGServiceProvider::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    static const char *method_name = "QueryService";
    if (ppv == NULL)
        return E_POINTER;
    *ppv = NULL;
    PY_GATEWAY_METHOD;
    PyObject *obGUID = PyWinObject_FromIID(guidService);
    if (obGUID == NULL)
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE(method_name);

    PyObject *obIID = PyWinObject_FromIID(riid);
    if (obIID == NULL) {
        Py_DECREF(obGUID);
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE(method_name);
    }
    PyObject *result;
    HRESULT hr = InvokeViaPolicy(method_name, &result, "OO", obGUID, obIID);
    Py_DECREF(obIID);
    Py_DECREF(obGUID);
    if (FAILED(hr))
        return hr;

    PyCom_InterfaceFromPyInstanceOrObject(result, riid, ppv, TRUE);
    Py_XDECREF(result);
    return MAKE_PYCOM_GATEWAY_FAILURE_CODE(method_name);
}

#endif  // NO_PYCOM_ISERVICEPROVIDER
