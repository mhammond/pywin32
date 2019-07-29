//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "structmember.h"

#ifdef NO_PYWINTYPES_SECURITY

BOOL PyWinObject_AsSECURITY_ATTRIBUTES(PyObject *ob, SECURITY_ATTRIBUTES **ppSECURITY_ATTRIBUTES,
                                       BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppSECURITY_ATTRIBUTES = NULL;
    }
    else {
        if (bNoneOK)
            PyErr_SetString(PyExc_TypeError, "This build of pywintypes only supports None as a SECURITY_ATTRIBUTE");
        else
            PyErr_SetString(
                PyExc_TypeError,
                "This function can not work in this build, as only None may be used as a SECURITY_ATTRIBUTE");
        return FALSE;
    }
    return TRUE;
}

#else

// @pymethod <o PySECURITY_ATTRIBUTES>|pywintypes|SECURITY_ATTRIBUTES|Creates a new SECURITY_ATTRIBUTES object
PyObject *PyWinMethod_NewSECURITY_ATTRIBUTES(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":SECURITY_ATTRIBUTES"))
        return NULL;
    return new PySECURITY_ATTRIBUTES();
}

PyObject *PyWinObject_FromSECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa) { return new PySECURITY_ATTRIBUTES(sa); }

BOOL PyWinObject_AsSECURITY_ATTRIBUTES(PyObject *ob, SECURITY_ATTRIBUTES **ppSECURITY_ATTRIBUTES,
                                       BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppSECURITY_ATTRIBUTES = NULL;
    }
    else if (!PySECURITY_ATTRIBUTES_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PySECURITY_ATTRIBUTES object");
        return FALSE;
    }
    else {
        PySECURITY_ATTRIBUTES *pysa = (PySECURITY_ATTRIBUTES *)ob;
        *ppSECURITY_ATTRIBUTES = pysa->GetSA();
        // in case the PySECURITY_DESCRIPTOR has been manipulated and points to a different address now
        if (pysa->m_obSD == Py_None)
            (*ppSECURITY_ATTRIBUTES)->lpSecurityDescriptor = NULL;
        else
            (*ppSECURITY_ATTRIBUTES)->lpSecurityDescriptor = ((PySECURITY_DESCRIPTOR *)pysa->m_obSD)->GetSD();
    }
    return TRUE;
}

// @object PySECURITY_ATTRIBUTES|A Python object, representing a SECURITY_ATTRIBUTES structure
struct PyMethodDef PySECURITY_ATTRIBUTES::methods[] = {{NULL}};

PYWINTYPES_EXPORT PyTypeObject PySECURITY_ATTRIBUTESType = {
    PYWIN_OBJECT_HEAD "PySECURITY_ATTRIBUTES",
    sizeof(PySECURITY_ATTRIBUTES),
    0,
    PySECURITY_ATTRIBUTES::deallocFunc,                              /* tp_dealloc */
    0,                                                               /* tp_print */
    0,                                                               /* tp_getattr */
    0,                                                               /* tp_setattr */
    0,                                                               /* tp_compare */
    0,                                                               /* tp_repr */
    0,                                                               /* tp_as_number */
    0,                                                               /* tp_as_sequence */
    0,                                                               /* tp_as_mapping */
    0,                                                               /* tp_hash */
    0,                                                               /* tp_call */
    0,                                                               /* tp_str */
    PySECURITY_ATTRIBUTES::getattro,                                 /* tp_getattro */
    PySECURITY_ATTRIBUTES::setattro,                                 /* tp_setattro */
    0,                                                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                                              /* tp_flags */
    "A Python object, representing a SECURITY_ATTRIBUTES structure", /* tp_doc */
    0,                                                               /* tp_traverse */
    0,                                                               /* tp_clear */
    0,                                                               /* tp_richcompare */
    0,                                                               /* tp_weaklistoffset */
    0,                                                               /* tp_iter */
    0,                                                               /* tp_iternext */
    PySECURITY_ATTRIBUTES::methods,                                  /* tp_methods */
    PySECURITY_ATTRIBUTES::members,                                  /* tp_members */
    0,                                                               /* tp_getset */
    0,                                                               /* tp_base */
    0,                                                               /* tp_dict */
    0,                                                               /* tp_descr_get */
    0,                                                               /* tp_descr_set */
    0,                                                               /* tp_dictoffset */
    0,                                                               /* tp_init */
    0,                                                               /* tp_alloc */
    0,                                                               /* tp_new */
};

#define OFF(e) offsetof(PySECURITY_ATTRIBUTES, e)

/*static*/ struct PYWINTYPES_EXPORT PyMemberDef PySECURITY_ATTRIBUTES::members[] = {
    {"bInheritHandle", T_INT, OFF(m_sa.bInheritHandle)},  // @prop boolean|bInheritHandle|Specifies whether the returned
                                                          // handle is inherited when a new process is created. If this
                                                          // member is TRUE, the new process inherits the handle.
    {"SECURITY_DESCRIPTOR", T_OBJECT,
     OFF(m_obSD)},  // @prop <o PySECURITY_DESCRIPTOR>|SECURITY_DESCRIPTOR|A PySECURITY_DESCRIPTOR, or None
    {NULL}};

// @comm On platforms that support security descriptor operations, SECURITY_DESCRIPTOR
//   defaults to a blank security descriptor with no owner, group, dacl, or sacl.
// Set to None to use a NULL security descriptor instead.
// When PySECURITY_ATTRIBUTES is created on Windows 95/98/Me, SECURITY_DESCRIPTOR defaults
//   to None and should not be changed.
// When SECURITY_DESCRIPTOR is not None, any of its methods can be invoked directly
//   on the PySECURITY_ATTRIBUTES object

PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(void)
{
    ob_type = &PySECURITY_ATTRIBUTESType;
    _Py_NewReference(this);
    m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    m_obSD = new PySECURITY_DESCRIPTOR(SECURITY_DESCRIPTOR_MIN_LENGTH);
    m_sa.lpSecurityDescriptor = ((PySECURITY_DESCRIPTOR *)m_obSD)->GetSD();
    // On win95/98/me (or any platform that doesn't have NT security) the
    // initialization of the SECURITY_DESCRIPTOR should fail, leaving the
    // sd NULL.
    if (m_sa.lpSecurityDescriptor == NULL) {
        Py_DECREF(m_obSD);
        Py_INCREF(Py_None);
        m_obSD = Py_None;
    }
    m_sa.bInheritHandle = TRUE;
}
PySECURITY_ATTRIBUTES::PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &sa)
{
    ob_type = &PySECURITY_ATTRIBUTESType;
    _Py_NewReference(this);
    m_sa = sa;
    if (sa.lpSecurityDescriptor == NULL) {
        // ???? could change existing behaviour - what happened with a NULL previously ????
        // This is consistent with PyWinObject_FromSECURITY_DESCRIPTOR
        Py_INCREF(Py_None);
        m_obSD = Py_None;
    }
    else {
        m_obSD = new PySECURITY_DESCRIPTOR(sa.lpSecurityDescriptor);
        // above creates a copy, put pointer to copy back in SECURITY_ATTRIBUTES structure
        m_sa.lpSecurityDescriptor = ((PySECURITY_DESCRIPTOR *)m_obSD)->GetSD();
    }
}

PySECURITY_ATTRIBUTES::~PySECURITY_ATTRIBUTES() { Py_XDECREF(m_obSD); }

PyObject *PySECURITY_ATTRIBUTES::getattro(PyObject *self, PyObject *obname)
{
    PyObject *res = PyObject_GenericGetAttr(self, obname);
    if (res != NULL)
        return res;

    // let it inherit methods from PySECURITY_DESCRIPTOR for backward compatibility
    PySECURITY_ATTRIBUTES *This = (PySECURITY_ATTRIBUTES *)self;
    if (This->m_obSD != Py_None) {
        PyErr_Clear();
        res = PyObject_GenericGetAttr(This->m_obSD, obname);
    }
    return res;
}

int PySECURITY_ATTRIBUTES::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete SECURITY_ATTRIBUTES attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    if (strcmp(name, "SECURITY_DESCRIPTOR") == 0) {
        PySECURITY_ATTRIBUTES *This = (PySECURITY_ATTRIBUTES *)self;
        PSECURITY_DESCRIPTOR psd;
        if (!PyWinObject_AsSECURITY_DESCRIPTOR(v, &psd, TRUE))
            return -1;
        Py_XDECREF(This->m_obSD);
        Py_INCREF(v);
        This->m_obSD = v;
        This->m_sa.lpSecurityDescriptor = psd;
        return 0;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PySECURITY_ATTRIBUTES::deallocFunc(PyObject *ob) { delete (PySECURITY_ATTRIBUTES *)ob; }

#endif /* MS_WINCE */
