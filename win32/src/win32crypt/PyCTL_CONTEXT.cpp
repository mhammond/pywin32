// @doc
#include "win32crypt.h"

// @object PyCTL_CONTEXT|Object containing a Certificate Trust List
struct PyMethodDef PyCTL_CONTEXT::methods[] = {
    // @pymeth CertFreeCTLContext|Closes the context handle
    {"CertFreeCTLContext", PyCTL_CONTEXT::PyCertFreeCTLContext, METH_NOARGS},
    // @pymeth CertEnumCTLContextProperties|Lists property id's for the context
    {"CertEnumCTLContextProperties", PyCTL_CONTEXT::PyCertEnumCTLContextProperties, METH_NOARGS},
    // @pymeth CertEnumSubjectInSortedCTL|Retrieves trusted subjects contained in CTL
    {"CertEnumSubjectInSortedCTL", PyCTL_CONTEXT::PyCertEnumSubjectInSortedCTL, METH_NOARGS},
    // @pymeth CertDeleteCTLFromStore|Removes the CTL from the store that it is contained in
    {"CertDeleteCTLFromStore", PyCTL_CONTEXT::PyCertDeleteCTLFromStore, METH_NOARGS},
    // @pymeth CertSerializeCTLStoreElement|Serializes the CTL and its properties
    {"CertSerializeCTLStoreElement", (PyCFunction)PyCTL_CONTEXT::PyCertSerializeCTLStoreElement,
     METH_KEYWORDS | METH_VARARGS},
    {NULL}};

PyTypeObject PyCTL_CONTEXTType = {PYWIN_OBJECT_HEAD "PyCTL_CONTEXT",
                                  sizeof(PyCTL_CONTEXT),
                                  0,
                                  PyCTL_CONTEXT::deallocFunc, /* tp_dealloc */
                                  0,                          /* tp_print */
                                  0,                          /* tp_getattr */
                                  0,                          /* tp_setattr */
                                  0,                          /* tp_compare */
                                  0,                          /* tp_repr */
                                  0,                          /* tp_as_number */
                                  0,                          /* tp_as_sequence */
                                  0,                          /* tp_as_mapping */
                                  0,
                                  0, /* tp_call */
                                  0, /* tp_str */
                                  PyCTL_CONTEXT::getattro,
                                  PyCTL_CONTEXT::setattro,
                                  0,                                         // PyBufferProcs *tp_as_buffer
                                  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
                                  0,                                         // tp_doc
                                  0,                                         // traverseproc tp_traverse
                                  0,                                         // tp_clear
                                  0,                                         // richcmpfunc tp_richcompare
                                  0,                                         // tp_weaklistoffset
                                  0,                                         // getiterfunc tp_iter
                                  0,                                         // iternextfunc tp_iternext
                                  PyCTL_CONTEXT::methods,
                                  PyCTL_CONTEXT::members};

struct PyMemberDef PyCTL_CONTEXT::members[] = {
    // @prop int|HCTL_CONTEXT|Raw message handle
    {"PCCTL_CONTEXT", T_OBJECT, offsetof(PyCTL_CONTEXT, obctl_context), READONLY, "Integet context handle"},
    {NULL} /* Sentinel */
};

int PyCTL_CONTEXT::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    return PyObject_GenericSetAttr(self, obname, v);
}

PyObject *PyCTL_CONTEXT::getattro(PyObject *self, PyObject *obname) { return PyObject_GenericGetAttr(self, obname); }

BOOL PyWinObject_AsCTL_CONTEXT(PyObject *ob, PCCTL_CONTEXT *ppctl_context, BOOL bNoneOK)
{
    if (bNoneOK && (ob == Py_None)) {
        *ppctl_context = NULL;
        return true;
    }
    if (ob->ob_type != &PyCTL_CONTEXTType) {
        PyErr_SetString(PyExc_TypeError, "Object must be of type PyCTL_CONTEXT");
        return FALSE;
    }
    *ppctl_context = ((PyCTL_CONTEXT *)ob)->GetCTL_CONTEXT();
    return TRUE;
}

PyObject *PyWinObject_FromCTL_CONTEXT(PCCTL_CONTEXT pcc)
{
    if (pcc == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyCTL_CONTEXT(pcc);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyWinObject_FromCTL_CONTEXT: Unable to create PyCTL_CONTEXT instance");
    return ret;
}

PyCTL_CONTEXT::~PyCTL_CONTEXT(void)
{
    if (pctl_context != NULL)
        CertFreeCTLContext(pctl_context);
    Py_XDECREF(this->obctl_context);
}

void PyCTL_CONTEXT::deallocFunc(PyObject *ob) { delete (PyCTL_CONTEXT *)ob; }

PyCTL_CONTEXT::PyCTL_CONTEXT(PCCTL_CONTEXT pcc)
{
    ob_type = &PyCTL_CONTEXTType;
    _Py_NewReference(this);
    this->pctl_context = pcc;
    this->obctl_context = PyLong_FromVoidPtr((void *)pcc);
    this->obdummy = NULL;
}

// @pymethod |PyCTL_CONTEXT|CertFreeCTLContext|Closes the CTL handle
PyObject *PyCTL_CONTEXT::PyCertFreeCTLContext(PyObject *self, PyObject *args)
{
    PCCTL_CONTEXT pcc = ((PyCTL_CONTEXT *)self)->GetCTL_CONTEXT();
    if (!CertFreeCTLContext(pcc))
        return PyWin_SetAPIError("CertFreeCTLContext");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod (int,...)|PyCTL_CONTEXT|CertEnumCTLContextProperties|Lists property id's for the context
PyObject *PyCTL_CONTEXT::PyCertEnumCTLContextProperties(PyObject *self, PyObject *args)
{
    PCCTL_CONTEXT pctl = ((PyCTL_CONTEXT *)self)->GetCTL_CONTEXT();
    PyObject *ret_item = NULL;
    DWORD err = 0, prop = 0;
    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    while (TRUE) {
        prop = CertEnumCTLContextProperties(pctl, prop);
        if (prop == 0)
            break;
        ret_item = PyLong_FromUnsignedLong(prop);
        if ((ret_item == NULL) || (PyList_Append(ret, ret_item) == -1)) {
            Py_XDECREF(ret_item);
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        Py_DECREF(ret_item);
    }
    return ret;
}

// @pymethod ((str,str),...)|PyCTL_CONTEXT|CertEnumSubjectInSortedCTL|Retrieves trusted subjects contained in CRL
// @rdesc Returns a sequence of tuples containing two strings (SubjectIdentifier, EncodedAttributes)
PyObject *PyCTL_CONTEXT::PyCertEnumSubjectInSortedCTL(PyObject *self, PyObject *args)
{
    PCCTL_CONTEXT pctl = ((PyCTL_CONTEXT *)self)->GetCTL_CONTEXT();
    void *ctxt = NULL;
    CRYPT_DER_BLOB subject, attr;
    PyObject *ret_item = NULL;
    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    while (CertEnumSubjectInSortedCTL(pctl, &ctxt, &subject, &attr)) {
        ret_item = Py_BuildValue("NN", PyString_FromStringAndSize((char *)subject.pbData, subject.cbData),
                                 PyString_FromStringAndSize((char *)attr.pbData, attr.cbData));
        if ((ret_item == NULL) || (PyList_Append(ret, ret_item) == -1)) {
            Py_XDECREF(ret_item);
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        Py_DECREF(ret_item);
    }
    return ret;
}

// @pymethod |PyCTL_CONTEXT|CertDeleteCTLFromStore|Removes the CTL from the store that it is contained in
PyObject *PyCTL_CONTEXT::PyCertDeleteCTLFromStore(PyObject *self, PyObject *args)
{
    PCCTL_CONTEXT pctl = ((PyCTL_CONTEXT *)self)->GetCTL_CONTEXT();
    if (!CertDeleteCTLFromStore(pctl))
        return PyWin_SetAPIError("CertDeleteCTLFromStore");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod string|PyCTL_CONTEXT|CertSerializeCTLStoreElement|Serializes the CTL and its properties
PyObject *PyCTL_CONTEXT::PyCertSerializeCTLStoreElement(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    PyObject *ret = NULL;
    DWORD flags = 0, bufsize = 0;
    PCCTL_CONTEXT pctl = ((PyCTL_CONTEXT *)self)->GetCTL_CONTEXT();
    BYTE *buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CertSerializeCTLStoreElement", keywords,
                                     &flags))  // @pyparm int|Flags|0|Reserved, use only 0 if passed in
        return NULL;
    if (!CertSerializeCTLStoreElement(pctl, flags, buf, &bufsize))
        return PyWin_SetAPIError("CertSerializeCTLStoreElement");
    buf = (BYTE *)malloc(bufsize);
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
    if (!CertSerializeCTLStoreElement(pctl, flags, buf, &bufsize))
        PyWin_SetAPIError("CertSerializeCTLStoreElement");
    else
        ret = PyString_FromStringAndSize((char *)buf, bufsize);
    free(buf);
    return ret;
}
