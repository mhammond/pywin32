#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"

// @doc
PyIPersistPropertyBag::PyIPersistPropertyBag(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIPersistPropertyBag::~PyIPersistPropertyBag() {}

/* static */ IPersistPropertyBag *PyIPersistPropertyBag::GetI(PyObject *self)
{
    return (IPersistPropertyBag *)PyIUnknown::GetI(self);
}

// @pymethod |PyIPersistPropertyBag|InitNew|Called by the container when the control is initialized to initialize the
// property bag.
PyObject *PyIPersistPropertyBag::InitNew(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":InitNew"))
        return NULL;

    IPersistPropertyBag *pIPPB = GetI(self);
    if (pIPPB == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIPPB->InitNew();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIPPB, IID_IPersistPropertyBag);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIPersistPropertyBag|Load|Called by the container to load the control's properties.
PyObject *PyIPersistPropertyBag::Load(PyObject *self, PyObject *args)
{
    PyObject *obBag;         // @pyparm <o PyIPropertyBag>|bag||the caller's property bag.
    PyObject *obLog = NULL;  // @pyparm <o PyIErrorLog>|log|None|the caller's error log, or None
    if (!PyArg_ParseTuple(args, "O|O:Load", &obBag, &obLog))
        return NULL;

    IPersistPropertyBag *pIPPB = GetI(self);
    if (pIPPB == NULL)
        return NULL;

    IPropertyBag *pIPB;
    if (!PyCom_InterfaceFromPyObject(obBag, IID_IPropertyBag, (LPVOID *)&pIPB, FALSE))
        return NULL;

    IErrorLog *pIEL = NULL;
    if (obLog != NULL && obLog != Py_None &&
        !PyCom_InterfaceFromPyObject(obLog, IID_IErrorLog, (LPVOID *)&pIEL, FALSE)) {
        pIPB->Release();
        return NULL;
    }

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIPPB->Load(pIPB, pIEL);
    pIPB->Release();
    pIEL->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIPPB, IID_IPersistPropertyBag);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIPersistPropertyBag|Save|Called by the container to save the object's properties.
PyObject *PyIPersistPropertyBag::Save(PyObject *self, PyObject *args)
{
    PyObject *obBag;        // @pyparm <o PyIPropertyBag>|bag||the caller's property bag.
    int clearDirty;         // @pyparm int|clearDirty||Specifies whether to clear the dirty flag.
    int saveAllProperties;  // @pyparm int|saveProperties||Specifies whether to save all properties or just those that
                            // have changed
    if (!PyArg_ParseTuple(args, "Oii:Save", &obBag, &clearDirty, &saveAllProperties))
        return NULL;

    IPersistPropertyBag *pIPPB = GetI(self);
    if (pIPPB == NULL)
        return NULL;

    IPropertyBag *pIPB;
    if (!PyCom_InterfaceFromPyObject(obBag, IID_IPropertyBag, (LPVOID *)&pIPB, FALSE))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIPPB->Save(pIPB, clearDirty, saveAllProperties);
    pIPB->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIPPB, IID_IPersistPropertyBag);

    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIPersistPropertyBag|A Python wrapper for a COM IPersistPropertyBag interface.
static struct PyMethodDef PyIPersistPropertyBag_methods[] = {
    {"InitNew", PyIPersistPropertyBag::InitNew,
     1},  // @pymeth InitNew|Called by the container when the control is initialized to initialize the property bag.
    {"Load", PyIPersistPropertyBag::Load, 1},  // @pymeth Load|Called by the container to load the control's properties.
    {"Save", PyIPersistPropertyBag::Save, 1},  // @pymeth Save|Called by the container to save the object's properties.
    {NULL}};

PyComTypeObject PyIPersistPropertyBag::type("PyIPersistPropertyBag",
                                            &PyIPersist::type,  // @base PyIPersistPropertyBag|PyIPersist
                                            sizeof(PyIPersistPropertyBag), PyIPersistPropertyBag_methods,
                                            GET_PYCOM_CTOR(PyIPersistPropertyBag));
