#include "stdafx.h"
#include "PyIObjectSafety.h"

PyIObjectSafety::PyIObjectSafety(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIObjectSafety::~PyIObjectSafety() {}

/* static */ IObjectSafety *PyIObjectSafety::GetI(PyObject *self) { return (IObjectSafety *)PyIUnknown::GetI(self); }

/* static */ PyObject *PyIObjectSafety::GetInterfaceSafetyOptions(PyObject *self, PyObject *args)
{
    PyObject *obiid;
    if (!PyArg_ParseTuple(args, "O:GetInterfaceSafetyOptions", &obiid))
        return NULL;

    CLSID iid;
    if (!PyWinObject_AsIID(obiid, &iid))
        return NULL;

    IObjectSafety *pIOS = GetI(self);
    if (pIOS == NULL)
        return NULL;

    DWORD dwSupportedOptions;
    DWORD dwEnabledOptions;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pIOS->GetInterfaceSafetyOptions(iid, &dwSupportedOptions, &dwEnabledOptions);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);

    return Py_BuildValue("ii", (int)dwSupportedOptions, (int)dwEnabledOptions);
}

/* static */ PyObject *PyIObjectSafety::SetInterfaceSafetyOptions(PyObject *self, PyObject *args)
{
    PyObject *obiid;
    int optionSetMask;
    int enabledOptions;
    if (!PyArg_ParseTuple(args, "Oii:SetInterfaceSafetyOptions", &obiid, &optionSetMask, &enabledOptions))
        return NULL;

    CLSID iid;
    if (!PyWinObject_AsIID(obiid, &iid))
        return NULL;

    IObjectSafety *pIOS = GetI(self);
    if (pIOS == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIOS->SetInterfaceSafetyOptions(iid, (DWORD)optionSetMask, (DWORD)enabledOptions);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);

    Py_INCREF(Py_None);
    return Py_None;
}

static struct PyMethodDef PyIObjectSafety_methods[] = {
    {"GetInterfaceSafetyOptions", PyIObjectSafety::GetInterfaceSafetyOptions, 1},
    {"SetInterfaceSafetyOptions", PyIObjectSafety::SetInterfaceSafetyOptions, 1},
    {NULL}};

PyComTypeObject PyIObjectSafety::type("PyIObjectSafety", &PyIUnknown::type, sizeof(PyIObjectSafety),
                                      PyIObjectSafety_methods, GET_PYCOM_CTOR(PyIObjectSafety));
