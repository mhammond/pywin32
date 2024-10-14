// PyIUnknown

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"

char *PyIUnknown::szErrMsgObjectReleased = "The COM object has been released.";

static LONG cUnknowns = 0;

LONG _PyCom_GetInterfaceCount(void) { return cUnknowns; }

PyIUnknown::PyIUnknown(IUnknown *punk)
{
    ob_type = &type;
    m_obj = punk;
    // refcnt of object managed by caller.
    InterlockedIncrement(&cUnknowns);
    PyCom_DLLAddRef();
}

PyIUnknown::~PyIUnknown()
{
    SafeRelease(this);
    InterlockedDecrement(&cUnknowns);
    PyCom_DLLReleaseRef();
}
// @method string|PyIUnknown|__repr__|Called to create a representation of a PyIUnknown object
PyObject *PyIUnknown::repr()
{
    // @comm The repr of this object displays both the object's address, and its attached IUnknown's address
    char buf[256];
    _snprintf(buf, 256, "<%hs at 0x%0p with obj at 0x%0p>", ob_type->tp_name, this, m_obj);
    return PyUnicode_FromString(buf);
}

/*static*/ IUnknown *PyIUnknown::GetI(PyObject *self)
{
    if (self == NULL) {
        PyCom_BuildInternalPyException("The Python object is invalid");
        return NULL;
    }
    PyIUnknown *pPyUnk = (PyIUnknown *)self;
    if (pPyUnk->m_obj == NULL) {
        PyCom_BuildInternalPyException(szErrMsgObjectReleased);
        return NULL;
    }
    return pPyUnk->m_obj;
}

/*static*/ void PyIUnknown::SafeRelease(PyIUnknown *ob)
{
    if (!ob)
        return;
    if (ob->m_obj) {
        // Safe for all objects which delete
        // itself ignoring a reference count.
        PyThreadState *_save;
        PYWINTYPES_TRY
        {
            _save = PyEval_SaveThread();
            long rcnt = ob->m_obj->Release();
            PyEval_RestoreThread(_save);
            ob->m_obj = NULL;
        }
        PYWINTYPES_EXCEPT
        {
            PyEval_RestoreThread(_save);
            PyCom_LogF(L"Win32 exception occurred releasing IUnknown at 0x%08p", ob->m_obj);
            ob->m_obj = NULL;
#ifdef _DEBUG
            DebugBreak();
#endif
            return;
        }
    }
}

// @pymethod int|PyIUnknown|__cmp__|Implements COM rules for object identity.
int PyIUnknown::compare(PyObject *other)
{
    // @comm As per the COM rules for object identity, both objects are queried for IUnknown, and these values compared.
    // The only meaningful test is for equality - the result of other comparisons is undefined
    // (ie, determined by the object's relative addresses in memory.
    IUnknown *pUnkOther;
    IUnknown *pUnkThis;
    if (!PyCom_InterfaceFromPyObject(this, IID_IUnknown, (void **)&pUnkThis, FALSE))
        return -1;
    // in a nod to rich comparisons, which end up calling this, we allow
    // 'other' to be an instance.
    if (!PyCom_InterfaceFromPyInstanceOrObject(other, IID_IUnknown, (void **)&pUnkOther, FALSE)) {
        pUnkThis->Release();
        return -1;
    }
    int rc = pUnkThis == pUnkOther ? 0 : (pUnkThis < pUnkOther ? -1 : 1);
    pUnkThis->Release();
    pUnkOther->Release();
    return rc;
}

// @pymethod <o PyIUnknown>|PyIUnknown|QueryInterface|Queries an object for a specific interface.
PyObject *PyIUnknown::QueryInterface(PyObject *self, PyObject *args)
{
    PyObject *obiid;
    PyObject *obUseIID = Py_None;
    // @pyparm IID|iid||The IID requested.
    // @pyparm IID|useIID|None|If provided and not None, will return an
    // interface for the specified IID if (and only if) a native interface can not be supported.
    // If the interface specified by iid is natively supported, this option is ignored.
    // @comm The useIID parameter is a very dangerous option, and should only
    // be used when you are sure you need it!
    // By specifying this parameter, you are telling the COM framework that regardless
    // of the true type of the result (as specified by iid), a Python wrapper
    // of type useIID will be created.  If iid does not derive from useIID,
    // then it is almost certain that using the object will cause an Access Violation.
    // <nl>For example, this option can be used to obtain a PyIUnknown object if
    // pythoncom does not natively support the interface.
    // Another example might be to return an unsupported persistence interface as a
    // PyIPersist instance.<nl>
    // For backwards compatibility: the integer 0 implies None, and the
    // integer 1 implies IID_IUnknown.
    // @rdesc The result is always an object derived from PyIUnknown.
    // Any error (including E_NOINTERFACE) will generate a <o com_error> exception.
    if (!PyArg_ParseTuple(args, "O|O:QueryInterface", &obiid, &obUseIID))
        return NULL;

    IID iid;
    if (!PyWinObject_AsIID(obiid, &iid))
        return NULL;

    IID useIID; /* used if obUseIID != Py_None */

    // This used to allow an int, with 1 indicating IUnknown
    // Doesn't seem to be used anywhere, so it has been removed
    if (obUseIID != Py_None)
        if (!PyWinObject_AsIID(obUseIID, &useIID))
            return NULL;

    IUnknown *pMyUnknown = GetI(self);
    if (pMyUnknown == NULL)
        return NULL;

    IUnknown *punk = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pMyUnknown->QueryInterface(iid, (LPVOID *)&punk);
    PY_INTERFACE_POSTCALL;

    /* Note that this failure may include E_NOINTERFACE */
    if (FAILED(hr) || punk == NULL)
        return PyCom_BuildPyException(hr, pMyUnknown, IID_IUnknown);

    /* Return a type based on the IID.  Note we can't ask PyCom_PyObjectFromIUnknown
       to own the reference, as we expect failure - and this will release our reference,
       which means we can't try again.  So a new ref is added should they work.
    */
    PyObject *rc = PyCom_PyObjectFromIUnknown(punk, iid, TRUE);

    /* we may have been asked to use a different interface */
    /* ??? useIID will be ignored if interface successfully created ???
      Apparently true and relies on a final QI somewhere? :()
    */
    if (rc == NULL && obUseIID != Py_None) {
        PyErr_Clear();
        rc = PyCom_PyObjectFromIUnknown(punk, useIID, TRUE);
    }
    PYCOM_RELEASE(punk);
    return rc;
}

// @object PyIUnknown|The base object for all PythonCOM objects.  Wraps a COM IUnknown object.
static struct PyMethodDef PyIUnknown_methods[] = {
    {"QueryInterface", PyIUnknown::QueryInterface, 1},  // @pymeth QueryInterface|Queries the object for an interface.
    {NULL, NULL}};
// @comm Note that there are no reference counting functions that are typically exposed via COM.
// This is because COM reference counts are automatically handled by PythonCOM - each interface
// object keeps exactly one COM reference, regardless of how many Python references.  When the
// Python object destructs due to its reference count hitting zero, the COM reference is then
// released.  It is not possible for force the closure of a PythonCOM object - the only
// way to ensure cleanup is to remove all Python references.

PyComTypeObject PyIUnknown::type("PyIUnknown", NULL, sizeof(PyIUnknown), PyIUnknown_methods,
                                 GET_PYCOM_CTOR(PyIUnknown));
