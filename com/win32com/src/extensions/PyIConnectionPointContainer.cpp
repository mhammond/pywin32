#include "stdafx.h"
#include "PythonCOM.h"
// @doc

PyIConnectionPointContainer::PyIConnectionPointContainer(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIConnectionPointContainer::~PyIConnectionPointContainer() {}

/* static */ IConnectionPointContainer *PyIConnectionPointContainer::GetI(PyObject *self)
{
    return (IConnectionPointContainer *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIEnumConnectionPoints>|PyIConnectionPointContainer|EnumConnectionPoints|Creates an enumerator object
// to iterate through all the connection points supported in the connectable object, one connection point per outgoing
// IID.
PyObject *PyIConnectionPointContainer::EnumConnectionPoints(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":EnumConnectionPoints"))
        return NULL;

    IConnectionPointContainer *pICPC = GetI(self);
    if (pICPC == NULL)
        return NULL;

    IEnumConnectionPoints *pE;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pICPC->EnumConnectionPoints(&pE);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);

    return PyCom_PyObjectFromIUnknown(pE, IID_IEnumConnectionPoints);
}

// @pymethod <o PyIConnectionPoint>|PyIConnectionPointContainer|FindConnectionPoint|Finds a connection point for the
// given IID
PyObject *PyIConnectionPointContainer::FindConnectionPoint(PyObject *self, PyObject *args)
{
    PyObject *obIID;
    // @pyparm <o PyIID>|iid||The IID of the requested connection.
    if (!PyArg_ParseTuple(args, "O:FindConnectionPoint", &obIID))
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IConnectionPointContainer *pICPC = GetI(self);
    if (pICPC == NULL)
        return NULL;

    IConnectionPoint *pCP;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pICPC->FindConnectionPoint(iid, &pCP);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return SetPythonCOMError(self, hr);

    return PyCom_PyObjectFromIUnknown(pCP, IID_IConnectionPoint);
}

// @object PyIConnectionPointContainer|A Python wrapper of a COM IConnectionPointContainer interface.
static struct PyMethodDef PyIConnectionPointContainer_methods[] = {
    {"EnumConnectionPoints", PyIConnectionPointContainer::EnumConnectionPoints,
     1},  // @pymeth EnumConnectionPoints|Creates an enumerator object to iterate through all the connection points
          // supported in the connectable object, one connection point per outgoing IID.
    {"FindConnectionPoint", PyIConnectionPointContainer::FindConnectionPoint,
     1},  // @pymeth FindConnectionPoint|Finds a connection point for the given IID.
    {NULL}};

PyComTypeObject PyIConnectionPointContainer::type("PyIConnectionPointContainer",
                                                  &PyIUnknown::type,  // @base PyIConnectionPointContainer|PyIUnknown
                                                  sizeof(PyIConnectionPointContainer),
                                                  PyIConnectionPointContainer_methods,
                                                  GET_PYCOM_CTOR(PyIConnectionPointContainer));
