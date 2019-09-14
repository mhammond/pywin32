// PyIClassFactory

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"  // For the DLL Ref functions.

PyIClassFactory::PyIClassFactory(IUnknown *pDisp) : PyIUnknown(pDisp)
{
    ob_type = &type;
    // Class Factory interfaces do not count towards DLL Ref counts,
    // but the PyIUnknown ctor Added a reference.
    PyCom_DLLReleaseRef();
}

PyIClassFactory::~PyIClassFactory()
{
    // Class Factory interfaces do not count towards DLL Ref counts,
    // but the PyIUnknown dtor Releases a reference.
    PyCom_DLLAddRef();
}

/*static*/ IClassFactory *PyIClassFactory::GetI(PyObject *self) { return (IClassFactory *)PyIUnknown::GetI(self); }

// @pymethod <o PyIUnknown>|PyIClassFactory|CreateInstance|Creates an uninitialized object.
PyObject *PyIClassFactory::CreateInstance(PyObject *self, PyObject *args)
{
    PyObject *obIID, *obUnk;
    // @pyparm <o PyIUnknown>|outerUnknown||Usually None, otherwise the outer unknown if the object is being created as
    // part of an aggregate.
    // @pyparm <o PyIID>|iid||The IID of the resultant object.
    if (!PyArg_ParseTuple(args, "OO:CreateInstance", &obUnk, &obIID))
        return NULL;

    IClassFactory *pClassFactory = GetI(self);
    if (pClassFactory == NULL)
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid))
        return NULL;

    IUnknown *pUnk = NULL;
    if (!PyCom_InterfaceFromPyObject(obUnk, IID_IUnknown, (void **)&pUnk, /*BOOL bNoneOK=*/TRUE))
        return NULL;
    IUnknown *pRet;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pClassFactory->CreateInstance(pUnk, iid, (void **)&pRet);
    if (pUnk)
        pUnk->Release();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pClassFactory, IID_IClassFactory);
    // @rdesc The result object will always be derived from PyIUnknown, but will be of the
    // type specified by iid.
    return PyCom_PyObjectFromIUnknown(pRet, iid);
}

// @pymethod |PyIClassFactory|LockServer|Called by the client of a class object to keep a server open in memory,
// allowing instances to be created more quickly.
PyObject *PyIClassFactory::LockServer(PyObject *self, PyObject *args)
{
    int bInc;
    // @pyparm int|bInc||1 of the server should be locked, 0 if the server should be unlocked.
    if (!PyArg_ParseTuple(args, "i:LockServer", &bInc))
        return NULL;
    IClassFactory *pClassFactory = GetI(self);
    if (pClassFactory == NULL)
        return NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pClassFactory->LockServer(bInc);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pClassFactory, IID_IClassFactory);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIClassFactory|An object which represents the IClassFactory interface.  Derived from <o PyIUnknown>
static struct PyMethodDef PyIClassFactory_methods[] = {
    {"CreateInstance", PyIClassFactory::CreateInstance, 1},  // @pymeth CreateInstance|Creates an uninitialized object.
    {"LockServer", PyIClassFactory::LockServer,
     1},  // @pymeth LockServer|Called by the client of a class object to keep a server open in memory, allowing
          // instances to be created more quickly.
    {NULL, NULL}};

PyComTypeObject PyIClassFactory::type("PyIClassFactory", &PyIUnknown::type, sizeof(PyIClassFactory),
                                      PyIClassFactory_methods, GET_PYCOM_CTOR(PyIClassFactory));
