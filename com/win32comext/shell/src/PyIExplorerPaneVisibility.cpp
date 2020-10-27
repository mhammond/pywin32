// This file implements the IExplorerPaneVisibility Interface and Gateway for Python.
#include "shell_pch.h"
#include "PyIExplorerPaneVisibility.h"
// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIExplorerPaneVisibility::PyIExplorerPaneVisibility(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIExplorerPaneVisibility::~PyIExplorerPaneVisibility() {}

/* static */ IExplorerPaneVisibility *PyIExplorerPaneVisibility::GetI(PyObject *self)
{
    return (IExplorerPaneVisibility *)PyIUnknown::GetI(self);
}

// @pymethod int|PyIExplorerPaneVisibility|GetPaneState|Description of Extract.
PyObject *PyIExplorerPaneVisibility::GetPaneState(PyObject *self, PyObject *args)
{
    IExplorerPaneVisibility *pIEI = GetI(self);
    if (pIEI == NULL)
        return NULL;
    // @pyparm guid|ep||Description for ep
    PyObject *obep;
    if (!PyArg_ParseTuple(args, "O:GetPaneState", &obep))
        return NULL;
    IID ep;
    if (!PyWinObject_AsIID(obep, &ep))
        return NULL;
    HRESULT hr;
    unsigned long state;
    PY_INTERFACE_PRECALL;
    hr = pIEI->GetPaneState(ep, &state);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIEI, IID_IExplorerPaneVisibility);
    return PyLong_FromUnsignedLong(state);
}

// @object PyIExplorerPaneVisibility|Description of the interface
static struct PyMethodDef PyIExplorerPaneVisibility_methods[] = {
    {"GetPaneState", PyIExplorerPaneVisibility::GetPaneState, 1},  // @pymeth Extract|Description of Extract
    {NULL}};

PyComTypeObject PyIExplorerPaneVisibility::type("PyIExplorerPaneVisibility", &PyIUnknown::type,
                                                sizeof(PyIExplorerPaneVisibility), PyIExplorerPaneVisibility_methods,
                                                GET_PYCOM_CTOR(PyIExplorerPaneVisibility));
