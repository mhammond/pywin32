//
// PyIID.cpp -- IID type for Python
//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"

#ifndef NO_PYWINTYPES_IID
// @pymethod <o PyIID>|pywintypes|IID|Creates a new IID object
PyObject *PyWinMethod_NewIID(PyObject *self, PyObject *args)
{
    WCHAR *bstrIID;
    PyObject *obIID;
    IID iid;

    // @pyparm string/Unicode|iidString||A string representation of an IID, or a ProgID.
    // @pyparm bool|is_bytes|False|Indicates if the first param is actually the bytes of an IID structure.
    int isBytes = FALSE;
    if (!PyArg_ParseTuple(args, "O|i", &obIID, &isBytes))
        return NULL;
    if (isBytes) {
        PyWinBufferView pybuf(obIID);
        if (!pybuf.ok())
            return NULL;
        if (pybuf.len() < sizeof(IID))
            return PyErr_Format(PyExc_ValueError, "string too small - must be at least %d bytes (got %d)", sizeof(IID),
                                pybuf.len());
        iid = *((IID *)pybuf.ptr());
        return PyWinObject_FromIID(iid);
    }
    // Already an IID? Return self.
    if (PyIID_Check(obIID)) {
        Py_INCREF(obIID);
        return obIID;
    }
    if (!PyWinObject_AsWCHAR(obIID, &bstrIID))
        return NULL;

    HRESULT hr = CLSIDFromString(bstrIID, &iid);
    if (FAILED(hr)) {
        hr = CLSIDFromProgID(bstrIID, &iid);
        if (FAILED(hr)) {
            PyWinObject_FreeWCHAR(bstrIID);
            PyWin_SetBasicCOMError(hr);
            return NULL;
        }
    }
    PyWinObject_FreeWCHAR(bstrIID);
    /* iid -> PyObject */
    return PyWinObject_FromIID(iid);
}

static HRESULT myCLSIDFromString(OLECHAR *str, CLSID *clsid)
{
    HRESULT hr = CLSIDFromString(str, clsid);
    if (SUCCEEDED(hr))
        return hr;
    return CLSIDFromProgID(str, clsid);
}

BOOL PyWinObject_AsIID(PyObject *obCLSID, CLSID *clsid)
{
    BSTR bstrCLSID;
    if (PyIID_Check(obCLSID)) {
        *clsid = ((PyIID *)obCLSID)->m_iid;
    }
    else if (PyWinObject_AsBstr(obCLSID, &bstrCLSID, FALSE)) {
        HRESULT hr = myCLSIDFromString(bstrCLSID, clsid);
        PyWinObject_FreeBstr(bstrCLSID);
        if (FAILED(hr)) {
            PyWin_SetBasicCOMError(hr);
            return FALSE;
        }
    }
    else {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "Only strings and iids can be converted to a CLSID.");
        return FALSE;
    }
    return TRUE;
}

PyObject *PyWinObject_FromIID(const IID &riid)
{
    // Later we could cache common IIDs - say IUnknown, IDispatch and NULL?
    PyObject *rc = new PyIID(riid);
    if (rc == NULL)
        PyErr_SetString(PyExc_MemoryError, "allocating new PyIID object");
    return rc;
}

PyObject *PyWinCoreString_FromIID(const IID &riid)
{
    OLECHAR oleRes[128];
    if (StringFromGUID2(riid, oleRes, sizeof(oleRes)) == 0) {
        // Should never happen - 128 should be heaps big enough.
        PyErr_SetString(PyExc_ValueError, "The string is too long");
        return NULL;
    }
    return PyWinCoreString_FromString(oleRes);
}

static int getbufferinfo(PyObject *self, Py_buffer *view, int flags)
{
    PyIID *pyiid = (PyIID *)self;
    return PyBuffer_FillInfo(view, self, &pyiid->m_iid, sizeof(IID), 1, flags);
}

static PyBufferProcs PyIID_as_buffer = {
    getbufferinfo,
    NULL  // Don't need to release any memory from Py_buffer struct
};

// @object PyIID|A Python object, representing an IID/CLSID.
// <nl>All pythoncom functions that return a CLSID/IID will return one of these
// objects.  However, in almost all cases, functions that expect a CLSID/IID
// as a param will accept either a string object, or a native PyIID object.
PYWINTYPES_EXPORT PyTypeObject PyIIDType = {
    PYWIN_OBJECT_HEAD "PyIID", sizeof(PyIID), 0, PyIID::deallocFunc, /* tp_dealloc */
    0,                                                               /* tp_print */
    0,                                                               /* tp_getattr */
    0,                                                               /* tp_setattr */
    0,                                                               /* tp_compare */
    // @pymeth __repr__|Used whenever a repr() is called for the object
    PyIID::reprFunc, /* tp_repr */
    0,               /* tp_as_number */
    0,               /* tp_as_sequence */
    0,               /* tp_as_mapping */
    // @pymeth __hash__|Used when the hash value of an IID object is required
    PyIID::hashFunc, /* tp_hash */
    0,               /* tp_call */
    // @pymeth __str__|Used whenever a string representation of the IID is required.
    PyIID::strFunc, /* tp_str */
    0,              /*tp_getattro*/
    0,              /*tp_setattro*/
    // @comm Note that IID objects support the buffer interface.  Thus buffer(iid) can be used to obtain the raw bytes.
    &PyIID_as_buffer,       /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    0,                      /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    PyIID::richcompareFunc, /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    0,                      /* tp_methods */
    0,                      /* tp_members */
    0,                      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    0,                      /* tp_new */
};

PyIID::PyIID(REFIID riid)
{
    ob_type = &PyIIDType;
    _Py_NewReference(this);
    m_iid = riid;
}

int PyIID::IsEqual(REFIID riid) { return IsEqualIID(m_iid, riid); }

int PyIID::IsEqual(PyObject *ob)
{
    if (Py_TYPE(ob) != &PyIIDType)
        return 0;
    return IsEqualIID(m_iid, ((PyIID *)ob)->m_iid);
}

int PyIID::IsEqual(PyIID &iid) { return IsEqualIID(m_iid, iid.m_iid); }

// Py3k requires that objects implement richcompare to be used as dict keys
PyObject *PyIID::richcompare(PyObject *other, int op)
{
    if (!PyIID_Check(other)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    BOOL e = IsEqualIID(m_iid, ((PyIID *)other)->m_iid);
    PyObject *ret;
    if (op == Py_EQ) {
        ret = e ? Py_True : Py_False;
    }
    else if (op == Py_NE) {
        ret = e ? Py_False : Py_True;
    }
    else
        ret = Py_NotImplemented;
    Py_INCREF(ret);
    return ret;
}

Py_hash_t PyIID::hash(void)
{
    DWORD n[4];

    memcpy(n, &m_iid, sizeof(n));
    n[0] += n[1] + n[2] + n[3];
    if (n[0] == -1)
        return -2;
    return n[0];
}

PyObject *PyIID::str(void) { return PyWinCoreString_FromIID(m_iid); }

PyObject *PyIID::repr(void)
{
    OLECHAR oleRes[128];
    StringFromGUID2(m_iid, oleRes, sizeof(oleRes));
    WCHAR buf[128];
    wsprintfW(buf, L"IID('%ws')", oleRes);
    return PyWinCoreString_FromString(buf);
}

/*static*/ void PyIID::deallocFunc(PyObject *ob) { delete (PyIID *)ob; }

// Py3k requires that objects implement richcompare to be used as dict keys
PyObject *PyIID::richcompareFunc(PyObject *self, PyObject *other, int op)
{
    return ((PyIID *)self)->richcompare(other, op);
}

// @pymethod int|PyIID|__hash__|Used when the hash value of an IID object is required
Py_hash_t PyIID::hashFunc(PyObject *ob) { return ((PyIID *)ob)->hash(); }
// @pymethod string|PyIID|__str__|Used whenever a string representation of the IID is required.
PyObject *PyIID::strFunc(PyObject *ob) { return ((PyIID *)ob)->str(); }
// @pymethod string|PyIID|__repr__|
PyObject *PyIID::reprFunc(PyObject *ob) { return ((PyIID *)ob)->repr(); }
#endif  // NO_PYWINTYPES_IID
