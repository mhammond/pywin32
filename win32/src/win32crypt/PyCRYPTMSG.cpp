// @doc
#include "win32crypt.h"

// @object PyCRYPTMSG|Wrapper for a cryptographic message handle
struct PyMethodDef PyCRYPTMSG::methods[] = {
    // @pymeth CryptMsgClose|Closes the message handle
    {"CryptMsgClose", PyCRYPTMSG::PyCryptMsgClose, METH_NOARGS},
    {NULL}};

PyTypeObject PyCRYPTMSGType = {PYWIN_OBJECT_HEAD "PyCRYPTMSG",
                               sizeof(PyCRYPTMSG),
                               0,
                               PyCRYPTMSG::deallocFunc, /* tp_dealloc */
                               0,                       /* tp_print */
                               0,                       /* tp_getattr */
                               0,                       /* tp_setattr */
                               0,                       /* tp_compare */
                               0,                       /* tp_repr */
                               0,                       /* tp_as_number */
                               0,                       /* tp_as_sequence */
                               0,                       /* tp_as_mapping */
                               0,
                               0, /* tp_call */
                               0, /* tp_str */
                               PyCRYPTMSG::getattro,
                               PyCRYPTMSG::setattro,
                               0,                                         // PyBufferProcs *tp_as_buffer
                               Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
                               0,                                         // tp_doc
                               0,                                         // traverseproc tp_traverse
                               0,                                         // tp_clear
                               0,                                         // richcmpfunc tp_richcompare
                               0,                                         // tp_weaklistoffset
                               0,                                         // getiterfunc tp_iter
                               0,                                         // iternextfunc tp_iternext
                               PyCRYPTMSG::methods,
                               PyCRYPTMSG::members};

struct PyMemberDef PyCRYPTMSG::members[] = {
    // @prop int|HCRYPTMSG|Raw message handle
    {"HCRYPTMSG", T_OBJECT, offsetof(PyCRYPTMSG, obcryptmsg), READONLY, "Raw message handle"},
    {NULL} /* Sentinel */
};

int PyCRYPTMSG::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    return PyObject_GenericSetAttr(self, obname, v);
}

PyObject *PyCRYPTMSG::getattro(PyObject *self, PyObject *obname)
{
    /*
    char *name=PYWIN_ATTR_CONVERT(obname);
    if (name==NULL)
        return NULL;
    if (strcmp(name,"HCRYPTMSG")==0){
        HCRYPTMSG h=((PyCRYPTMSG *)self)->GetHCRYPTMSG();
        return PyLong_FromVoidPtr((void *)h);
        }
    */
    return PyObject_GenericGetAttr(self, obname);
}

BOOL PyWinObject_AsCRYPTMSG(PyObject *obHCRYPTMSG, HCRYPTMSG *hcryptmsg, BOOL bNoneOK)
{
    if (bNoneOK && (obHCRYPTMSG == Py_None)) {
        *hcryptmsg = NULL;
        return true;
    }
    if (obHCRYPTMSG->ob_type != &PyCRYPTMSGType) {
        PyErr_SetString(PyExc_TypeError, "Object must be of type PyCRYPTMSG");
        return FALSE;
    }
    *hcryptmsg = ((PyCRYPTMSG *)obHCRYPTMSG)->GetHCRYPTMSG();
    return TRUE;
}

PyObject *PyWinObject_FromCRYPTMSG(HCRYPTMSG h)
{
    if (h == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyCRYPTMSG(h);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyWinObject_FromCRYPTMSG: Unable to create PyCRYPTMSG instance");
    return ret;
}

PyCRYPTMSG::~PyCRYPTMSG(void)
{
    if (hcryptmsg != NULL)
        CryptMsgClose(hcryptmsg);
    Py_XDECREF(this->obcryptmsg);
}

void PyCRYPTMSG::deallocFunc(PyObject *ob) { delete (PyCRYPTMSG *)ob; }

PyCRYPTMSG::PyCRYPTMSG(HCRYPTMSG h)
{
    ob_type = &PyCRYPTMSGType;
    _Py_NewReference(this);
    this->hcryptmsg = h;
    this->obcryptmsg = PyLong_FromVoidPtr((void *)h);
    this->obdummy = NULL;
}

// @pymethod |PyCRYPTMSG|CryptMsgClose|Closes the message handle
PyObject *PyCRYPTMSG::PyCryptMsgClose(PyObject *self, PyObject *args)
{
    HCRYPTMSG h = ((PyCRYPTMSG *)self)->GetHCRYPTMSG();
    if (!CryptMsgClose(h))
        return PyWin_SetAPIError("CryptMsgClose");
    Py_INCREF(Py_None);
    return Py_None;
}
