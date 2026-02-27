//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"

// @pymethod <o PyHANDLE>|pywintypes|HANDLE|Creates a new HANDLE object
PyObject *PyWinMethod_NewHANDLE(PyObject *self, PyObject *args)
{
    HANDLE hInit;
    PyObject *obhInit = Py_None;
    if (!PyArg_ParseTuple(args, "|O:HANDLE", &obhInit))
        return NULL;
    if (!PyWinObject_AsHANDLE(obhInit, &hInit))
        return NULL;
    return new PyHANDLE(hInit);
}

BOOL PyWinObject_AsHANDLE(PyObject *ob, HANDLE *pHANDLE)
{
    if (ob == Py_None) {
        *pHANDLE = (HANDLE)0;
    }
    else if (PyHANDLE_Check(ob)) {
        PyHANDLE *pH = (PyHANDLE *)ob;
        *pHANDLE = (HANDLE)(*pH);
    }
    else {  // Support integer objects for b/w compat.
        // treat int handles same a void pointers
        if (!PyWinLong_AsVoidPtr(ob, (void **)pHANDLE)) {
            PyErr_SetString(PyExc_TypeError, "The object is not a PyHANDLE object");
            return FALSE;
        }
    }
    return TRUE;
}

PyObject *PyWinObject_FromHANDLE(HANDLE h) { return new PyHANDLE(h); }

// For handles that aren't returned as PyHANDLE or a subclass thereof (HDC, HWND, etc).
// Treated same as void pointers.
// ??? Maybe make this a macro to avoid extra function call ???
PyObject *PyWinLong_FromHANDLE(HANDLE h) { return PyWinLong_FromVoidPtr(h); }

BOOL PyWinObject_CloseHANDLE(PyObject *obHandle)
{
    // PyWinObject_AsHANDLE checks this also, but need to make sure an override Close method is called
    if (PyHANDLE_Check(obHandle))
        return ((PyHANDLE *)obHandle)->Close();  // Python error already set.

    HANDLE h;
    BOOL ok;
    if (!PyWinObject_AsHANDLE(obHandle, &h))
        return FALSE;
    ok = h == 0 ? TRUE : ::CloseHandle(h);  // This can still trigger an Invalid Handle exception in debug mode
    if (!ok)
        PyWin_SetAPIError("CloseHandle");
    return ok;
}

// @pymethod |PyHANDLE|Close|Closes the underlying Win32 handle.
// @comm If the handle is already closed, no error is raised.
PyObject *PyHANDLE::Close(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Close"))
        return NULL;
    if (!((PyHANDLE *)self)->Close())
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|PyHANDLE|Detach|Detaches the Win32 handle from the handle object.
// @rdesc The result is the value of the handle before it is detached.  If the
// handle is already detached, this will return zero.
// @comm After calling this function, the handle is effectively invalidated,
// but the handle is not closed.  You would call this function when you
// need the underlying win32 handle to exist beyond the lifetime of the
// handle object.
PyObject *PyHANDLE::Detach(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Detach"))
        return NULL;
    PyHANDLE *pThis = (PyHANDLE *)self;
    PyObject *ret = PyWinLong_FromHANDLE(pThis->m_handle);
    if (ret != NULL)
        pThis->m_handle = 0;
    return ret;
}

// @object PyHANDLE|A Python object, representing a win32 HANDLE.
// @comm This object wraps a win32 HANDLE object, automatically closing it when the object
// is destroyed.  To guarantee cleanup, you can call either <om PyHANDLE.Close>, or
// <om win32api.CloseHandle>.
// <nl>Most functions which accept a handle object also accept an integer - however,
// use of the handle object is encouraged.

// @prop long|handle|Integer value of the handle
PyObject *PyHANDLE::get_handle(PyObject *self, void *unused)
{
    return PyWinLong_FromHANDLE(((PyHANDLE *)self)->m_handle);
}

PyGetSetDef PyHANDLE::getset[] = {{"handle", PyHANDLE::get_handle, NULL}, {NULL}};

struct PyMethodDef PyHANDLE::methods[] = {
    {"Close", PyHANDLE::Close, 1},    // @pymeth Close|Closes the handle
    {"close", PyHANDLE::Close, 1},    // @pymeth close|Synonym for <om PyHANDLE.Close>
    {"Detach", PyHANDLE::Detach, 1},  // @pymeth Detach|Detaches the Win32 handle from the handle object.
    {NULL}};

static PyNumberMethods PyHANDLE_NumberMethods = {
    PyHANDLE::binaryFailureFunc,  /* nb_add */
    PyHANDLE::binaryFailureFunc,  /* nb_subtract */
    PyHANDLE::binaryFailureFunc,  /* nb_multiply */
    PyHANDLE::binaryFailureFunc,  /* nb_remainder */
    PyHANDLE::binaryFailureFunc,  /* nb_divmod */
    PyHANDLE::ternaryFailureFunc, /* nb_power */
    PyHANDLE::unaryFailureFunc,   /* nb_negative */
    PyHANDLE::unaryFailureFunc,   /* nb_positive */
    PyHANDLE::unaryFailureFunc,   /* nb_absolute */
    // @pymeth  __bool__|Used for detecting true/false.
    PyHANDLE::nonzeroFunc,       /* is nb_bool in Python 3.0 */
    PyHANDLE::unaryFailureFunc,  /* nb_invert */
    PyHANDLE::binaryFailureFunc, /* nb_lshift */
    PyHANDLE::binaryFailureFunc, /* nb_rshift */
    PyHANDLE::binaryFailureFunc, /* nb_and */
    PyHANDLE::binaryFailureFunc, /* nb_xor */
    PyHANDLE::binaryFailureFunc, /* nb_or */
    PyHANDLE::intFunc,           /* nb_int */
    PyHANDLE::longFunc,          /* nb_long */
    PyHANDLE::unaryFailureFunc,  /* nb_float */
                                 // These removed in 3.0
};
// @pymeth __int__|Used when an integer representation of the handle object is required.

PYWINTYPES_EXPORT PyTypeObject PyHANDLEType = {
    PYWIN_OBJECT_HEAD "PyHANDLE", sizeof(PyHANDLE), 0, PyHANDLE::deallocFunc, /* tp_dealloc */
    0, 0,                                                                     /* tp_getattr */
    0,                                                                        /* tp_setattr */
    0,                                                                        /* tp_compare */
    PyHANDLE::strFunc,                                                        /* tp_repr */
    &PyHANDLE_NumberMethods,                                                  /* tp_as_number */
    0,                                                                        /* tp_as_sequence */
    0,                                                                        /* tp_as_mapping */
    // @pymeth __hash__|Used when the hash value of an object is required
    PyHANDLE::hashFunc, /* tp_hash */
    0,                  /* tp_call */
    // @pymeth __str__|Used when a string representation is required
    PyHANDLE::strFunc,                        /* tp_str */
    PyObject_GenericGetAttr,                  /* tp_getattro */
    PyObject_GenericSetAttr,                  /* tp_setattro */
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    PyHANDLE::richcompareFunc,                /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyHANDLE::methods,                        /* tp_methods */
    0,                                        /* tp_members */
    PyHANDLE::getset,                         /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0,                                        /* tp_new */
};

PyHANDLE::PyHANDLE(HANDLE hInit)
{
    ob_type = &PyHANDLEType;
    _Py_NewReference(this);
    m_handle = hInit;
}

PyHANDLE::~PyHANDLE(void)
{
    // can not call Close here, as it is a virtual, and therefore
    // will not correctly call a derived class.
}

BOOL PyHANDLE::Close(void)
{
    BOOL rc = TRUE;
    if (m_handle) {
        Py_BEGIN_ALLOW_THREADS
#ifdef Py_DEBUG
            __try
        {
#endif  // Py_DEBUG
            rc = CloseHandle(m_handle);
#ifdef Py_DEBUG
        }
        __except (1)
        {
            // according to the docs on CloseHandle(), this
            // can happen when run under the debugger.  This is a
            // PITA, as it makes it hard to debug whatever we are here
            // for (unless we are here to debug this!).  So we break into
            // the debugger, which gives the developer the option of continuing
            static bool is_first_exception = true;
            static bool break_on_exception = true;
            // It *seems* that handles that raise an exception under
            // the debugger actually *succeed* calling Close running normally.
            static bool raise_python_exception = false;
            if (break_on_exception) {
                if (is_first_exception)
                    break_on_exception = false;
                DebugBreak();
                // reset 'break_on_exception' to true if you want to
                // continue breaking on every invalid handle exception

                // set 'raise_python_exception' to true to send the exception to Python.
            }
            is_first_exception = false;
            if (raise_python_exception) {
                rc = FALSE;
                ::SetLastError(ERROR_INVALID_HANDLE);
            }
        }
#endif  // Py_DEBUG
        Py_END_ALLOW_THREADS m_handle = 0;
    }
    if (!rc)
        PyWin_SetAPIError("CloseHandle");
    return rc;
}

// @pymethod |PyHANDLE|__bool__|Used for detecting true/false.
// @rdesc The result is 1 if the attached handle is non zero, else 0.
/*static*/ int PyHANDLE::nonzeroFunc(PyObject *ob) { return ((PyHANDLE *)ob)->m_handle != 0; }

PyObject *PyHANDLE::richcompare(PyObject *other, int op)
{
    HANDLE hother;
    if (PyHANDLE_Check(other)) {
        hother = ((PyHANDLE *)other)->m_handle;
    }
    else if (PyLong_Check(other) || PyLong_Check(other)) {
        if (!PyWinLong_AsVoidPtr(other, &hother))
            return NULL;
    }
    else {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    BOOL e = m_handle == hother;
    PyObject *ret;
    if (op == Py_EQ)
        ret = e ? Py_True : Py_False;
    else if (op == Py_NE)
        ret = !e ? Py_True : Py_False;
    else
        ret = Py_NotImplemented;
    Py_INCREF(ret);
    return ret;
}

// @pymethod |PyHANDLE|__int__|Used when the handle as an integer is required.
// @comm To get the underling win32 handle from a PyHANDLE object, use int(handleObject)
PyObject *PyHANDLE::intFunc(PyObject *ob) { return PyWinLong_FromHANDLE(((PyHANDLE *)ob)->m_handle); }

// @pymethod |PyHANDLE|__long__|Used when the handle as an integer is required.
// @comm To get the underling win32 handle from a PyHANDLE object, use long(handleObject)
PyObject *PyHANDLE::longFunc(PyObject *ob) { return PyWinLong_FromHANDLE(((PyHANDLE *)ob)->m_handle); }

// @pymethod |PyHANDLE|__print__|Used when the HANDLE object is printed.
int PyHANDLE::printFunc(PyObject *ob, FILE *fp, int flags) { return ((PyHANDLE *)ob)->print(fp, flags); }

// @pymethod |PyHANDLE|__str__|Used when a string representation of the handle object is required.
PyObject *PyHANDLE::strFunc(PyObject *ob) { return ((PyHANDLE *)ob)->asStr(); }

PyObject *PyHANDLE::richcompareFunc(PyObject *ob, PyObject *other, int op)
{
    return ((PyHANDLE *)ob)->richcompare(other, op);
}

// @pymethod int|PyHANDLE|__hash__|Used when the hash value of a HANDLE object is required
Py_hash_t PyHANDLE::hashFunc(PyObject *ob) { return ((PyHANDLE *)ob)->hash(); }

Py_hash_t PyHANDLE::hash(void)
{
    // Just use the address.
#if PY_VERSION_HEX >= 0x03130000
    return Py_HashPointer(this);
#else
    return _Py_HashPointer(this);
#endif
}

int PyHANDLE::print(FILE *fp, int flags)
{
    TCHAR resBuf[160];
    wsprintf(resBuf, _T("<%hs at %Id (%Id)>"), GetTypeName(), this, m_handle);
    // ### ACK! Python uses a non-debug runtime. We can't use stream
    // ### functions when in DEBUG mode!!  (we link against a different
    // ### runtime library)  Hack it by getting Python to do the print!
    //
    // ### - Double Ack - Always use the hack!
    // #ifdef _DEBUG
    PyObject *ob = PyWinCoreString_FromString(resBuf);
    PyObject_Print(ob, fp, flags | Py_PRINT_RAW);
    Py_DECREF(ob);
    /***
    #else
        fputs(resBuf, fp);
    #endif
    ***/
    return 0;
}

PyObject *PyHANDLE::asStr(void)
{
    WCHAR resBuf[160];
    _snwprintf(resBuf, 160, L"<%hs:%Id>", GetTypeName(), (size_t)m_handle);
    return PyWinCoreString_FromString(resBuf);
}

char *failMsg = "bad operand type";
/*static*/ PyObject *PyHANDLE::unaryFailureFunc(PyObject *ob)
{
    PyErr_SetString(PyExc_TypeError, failMsg);
    return NULL;
}
/*static*/ PyObject *PyHANDLE::binaryFailureFunc(PyObject *ob1, PyObject *ob2)
{
    PyErr_SetString(PyExc_TypeError, failMsg);
    return NULL;
}
/*static*/ PyObject *PyHANDLE::ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3)
{
    PyErr_SetString(PyExc_TypeError, failMsg);
    return NULL;
}

/*static*/ void PyHANDLE::deallocFunc(PyObject *ob)
{
    // This can be delicate.  Setting an exception in a destructor is evil
    // (as it will cause gc to die with a fatal error, and if that doesn't
    // happen, make unrelated code appear to fail with the exception.)
    // Clearing any existing exceptions that may be set is also evil, as
    // we may be destructing as part of unwinding the stack handling an
    // existing exception. Therefore, we "push" any existing exception
    // contexts, and restoring it clobbers any we raise.
    PyObject *typ, *val, *tb;
    PyErr_Fetch(&typ, &val, &tb);
    ((PyHANDLE *)ob)->Close();
    delete (PyHANDLE *)ob;
    PyErr_Restore(typ, val, tb);
}

// A Registry handle.
// @object PyHKEY|A Python object, representing a win32 HKEY (a HANDLE to a registry key).
// See the <o PyHANDLE> object for more details
BOOL PyWinObject_AsHKEY(PyObject *ob, HKEY *pRes) { return PyWinObject_AsHANDLE(ob, (HANDLE *)pRes); }
PyObject *PyWinObject_FromHKEY(HKEY h) { return new PyHKEY(h); }
// @pymethod <o PyHKEY>|pywintypes|HKEY|Creates a new HKEY object
PyObject *PyWinMethod_NewHKEY(PyObject *self, PyObject *args)
{
    HANDLE hInit;
    PyObject *obhInit = Py_None;  // ??? hInit previously not initialized but treated as optional ???
    if (!PyArg_ParseTuple(args, "|O:HANDLERegistry", &obhInit))
        return NULL;
    if (!PyWinObject_AsHANDLE(obhInit, &hInit))
        return NULL;
    return new PyHKEY(hInit);
}

BOOL PyWinObject_CloseHKEY(PyObject *obHandle)
{
    if (PyHANDLE_Check(obHandle)) {
        // Make sure we don't call Close() for any other type of PyHANDLE
        if (strcmp(((PyHANDLE *)obHandle)->GetTypeName(), "PyHKEY") != 0) {
            PyErr_SetString(PyExc_TypeError, "HANDLE must be a PyHKEY");
            return FALSE;
        }
        // Python error already set.
        return ((PyHKEY *)obHandle)->Close();
    }
    HKEY hkey;
    if (!PyWinObject_AsHANDLE(obHandle, (HANDLE *)&hkey))
        return FALSE;
    long rc = ::RegCloseKey(hkey);
    BOOL ok = (rc == ERROR_SUCCESS);
    if (!ok)
        PyWin_SetAPIError("RegCloseKey", rc);
    return ok;
}

// The non-static member functions
BOOL PyHKEY::Close(void)
{
    LONG rc = m_handle ? RegCloseKey((HKEY)m_handle) : ERROR_SUCCESS;
    m_handle = 0;
    if (rc != ERROR_SUCCESS)
        PyWin_SetAPIError("RegCloseKey", rc);
    return rc == ERROR_SUCCESS;
}
