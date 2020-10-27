#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"

// @doc

// @object IDLDESC|An IDLDESC is respresented as
static PyObject *MakeIDLDesc(const IDLDESC *id)
{
    // @tupleitem 0|int|reserved|A reserved value!
    // @tupleitem 1|int|flags|IDL flags.
    return Py_BuildValue("li", id->dwReserved, id->wIDLFlags);
}

// @pymethod <o TYPEATTR>|pythoncom|TYPEATTR|Creates a new TYPEATTR object
PyObject *Py_NewTYPEATTR(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return new PyTYPEATTR();
}

PyObject *PyObject_FromTYPEATTR(TYPEATTR *desc) { return new PyTYPEATTR(desc); }

// @object TYPEATTR|A TYPEATTR object represents a COM TYPEATTR structure.
struct PyMethodDef PyTYPEATTR::methods[] = {{NULL}};

// Sequence stuff to provide compatibility with tuples.
static PySequenceMethods PyTYPEATTR_Sequence = {
    PyTYPEATTR::getlength,  // sq_length;
    NULL,                   // sq_concat;
    NULL,                   // sq_repeat;
    PyTYPEATTR::getitem,    // sq_item;
    NULL,                   // sq_slice;
    NULL,                   // sq_ass_item;
    NULL,                   // sq_ass_slice;
};

PyTypeObject PyTYPEATTR::Type = {
    PYWIN_OBJECT_HEAD "PyTYPEATTR",
    sizeof(PyTYPEATTR),
    0,
    PyTYPEATTR::deallocFunc, /* tp_dealloc */
    0,                       /* tp_print */
    0,                       /* tp_getattr */
    0,                       /* tp_setattr */
    0,                       /* tp_compare */
    0,                       /* tp_repr */
    0,                       /* tp_as_number */
    &PyTYPEATTR_Sequence,    /* tp_as_sequence */
    0,                       /* tp_as_mapping */
    0,                       /* tp_hash */
    0,                       /* tp_call */
    0,                       /* tp_str */
    PyObject_GenericGetAttr, /* tp_getattro */
    PyObject_GenericSetAttr, /* tp_setattro */
    0,                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,      /* tp_flags */
    0,                       /* tp_doc */
    0,                       /* tp_traverse */
    0,                       /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    PyTYPEATTR::methods,     /* tp_methods */
    PyTYPEATTR::members,     /* tp_members */
    0,                       /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    0,                       /* tp_init */
    0,                       /* tp_alloc */
    0,                       /* tp_new */
};

#define OFF(e) offsetof(PyTYPEATTR, e)

/*static*/ struct PyMemberDef PyTYPEATTR::members[] = {
    {"iid", T_OBJECT, OFF(iid)},                         // @prop <o PyIID>|iid|The IID
    {"lcid", T_INT, OFF(lcid)},                          // @prop int|lcid|The lcid
    {"memidConstructor", T_INT, OFF(memidConstructor)},  // @prop int|memidConstructor|ID of constructor
    {"memidDestructor", T_INT, OFF(memidDestructor)},    // @prop int|memidDestructor|ID of destructor
    {"cbSizeInstance", T_INT, OFF(cbSizeInstance)},  // @prop int|cbSizeInstance|The size of an instance of this type
    {"typekind", T_INT, OFF(typekind)},  // @prop int|typekind|The kind of type this information describes.  One of the
                                         // win32con.TKIND_* constants.
    {"cFuncs", T_INT, OFF(cFuncs)},      // @prop int|cFuncs|Number of functions.
    {"cVars", T_INT, OFF(cVars)},        // @prop int|cVars|Number of variables/data members.
    {"cImplTypes", T_INT, OFF(cImplTypes)},      // @prop int|cImplTypes|Number of implemented interfaces.
    {"cbSizeVft", T_INT, OFF(cbSizeVft)},        // @prop int|cbSizeVft|The size of this type's VTBL
    {"cbAlignment", T_INT, OFF(cbAlignment)},    // @prop int|cbAlignment|Byte alignment for an instance of this type.
    {"wTypeFlags", T_INT, OFF(wTypeFlags)},      // @prop int|wTypeFlags|One of the pythoncom TYPEFLAG_
    {"wMajorVerNum", T_INT, OFF(wMajorVerNum)},  // @prop int|wMajorVerNum|Major version number.
    {"wMinorVerNum", T_INT, OFF(wMinorVerNum)},  // @prop int|wMinorVerNum|Minor version number.
    {"tdescAlias", T_OBJECT, OFF(obDescAlias)},  // @prop <o TYPEDESC>|tdescAlias|If TypeKind == pythoncom.TKIND_ALIAS,
                                                 // specifies the type for which this type is an alias.
    {"idldescType", T_OBJECT, OFF(obIDLDesc)},   // @prop <o IDLDESC>|idldeskType|IDL attributes of the described type.
    {NULL}};

PyTYPEATTR::PyTYPEATTR()
{
    ob_type = &PyTYPEATTR::Type;
    _Py_NewReference(this);

    iid = NULL;
    lcid = 0;
    memidConstructor = 0;
    memidDestructor = 0;
    cbSizeInstance = 0;
    typekind = 0;
    cFuncs = 0;
    cVars = 0;
    cImplTypes = 0;
    cbSizeVft = 0;
    cbAlignment = 0;
    wTypeFlags = 0;
    wMajorVerNum = 0;
    wMinorVerNum = 0;
    obDescAlias = NULL;
    obIDLDesc = NULL;
}

PyTYPEATTR::PyTYPEATTR(const TYPEATTR *attr)
{
    ob_type = &PyTYPEATTR::Type;
    _Py_NewReference(this);

    iid = PyWinObject_FromIID(attr->guid);
    lcid = attr->lcid;
    memidConstructor = attr->memidConstructor;
    memidDestructor = attr->memidDestructor;
    cbSizeInstance = attr->cbSizeInstance;
    typekind = attr->typekind;
    cFuncs = attr->cFuncs;
    cVars = attr->cVars;
    cImplTypes = attr->cImplTypes;
    cbSizeVft = attr->cbSizeVft;
    cbAlignment = attr->cbAlignment;
    wTypeFlags = attr->wTypeFlags;
    wMajorVerNum = attr->wMajorVerNum;
    wMinorVerNum = attr->wMinorVerNum;

    // Some (only a few 16 bit MSOffice only one so far, and even then only occasionally!)
    // servers seem to send invalid tdescAlias when its not actually an alias.
    if (attr->typekind == TKIND_ALIAS)
        obDescAlias = PyObject_FromTYPEDESC(&attr->tdescAlias);
    else {
        Py_INCREF(Py_None);
        obDescAlias = Py_None;
    }

    obIDLDesc = MakeIDLDesc(&attr->idldescType);
}

PyTYPEATTR::~PyTYPEATTR()
{
    Py_XDECREF(iid);
    Py_XDECREF(obDescAlias);
    Py_XDECREF(obIDLDesc);
}

/*static*/ void PyTYPEATTR::deallocFunc(PyObject *ob) { delete (PyTYPEATTR *)ob; }

// Sequence stuff to provide compatibility with tuples.
/* static */ Py_ssize_t PyTYPEATTR::getlength(PyObject *self)
{
    // NEVER CHANGE THIS - you will break all the old
    // code written when these object were tuples!
    return 16;
}

/* static */ PyObject *PyTYPEATTR::getitem(PyObject *self, Py_ssize_t index)
{
    PyTYPEATTR *p = (PyTYPEATTR *)self;
    PyObject *rc;
    switch (index) {
        case 0:  // @tupleitem 0|<o PyIID>|IID|The IID
            rc = p->iid ? p->iid : Py_None;
            Py_INCREF(rc);
            return rc;
        case 1:  // @tupleitem 1|int|lcid|The lcid
            return PyInt_FromLong(p->lcid);
        case 2:  // @tupleitem 2|int|memidConstructor|ID of constructor
            return PyInt_FromLong(p->memidConstructor);
        case 3:  // @tupleitem 3|int|memidDestructor|ID of destructor,
            return PyInt_FromLong(p->memidDestructor);
        case 4:  // @tupleitem 4|int|cbSizeInstance|The size of an instance of this type
            return PyInt_FromLong(p->cbSizeInstance);
        case 5:  // @tupleitem 5|int|typekind|The kind of type this information describes.  One of the win32con.TKIND_*
                 // constants.
            return PyInt_FromLong(p->typekind);
        case 6:  // @tupleitem 6|int|cFuncs|Number of functions.
            return PyInt_FromLong(p->cFuncs);
        case 7:  // @tupleitem 7|int|cVars|Number of variables/data members.
            return PyInt_FromLong(p->cVars);
        case 8:  // @tupleitem 8|int|cImplTypes|Number of implemented interfaces.
            return PyInt_FromLong(p->cImplTypes);
        case 9:  // @tupleitem 9|int|cbSizeVft|The size of this type's VTBL
            return PyInt_FromLong(p->cbSizeVft);
        case 10:  // @tupleitem 10|int|cbAlignment|Byte alignment for an instance of this type.
            return PyInt_FromLong(p->cbAlignment);
        case 11:  // @tupleitem 11|int|wTypeFlags|One of the pythoncom TYPEFLAG_* constants
            return PyInt_FromLong(p->wTypeFlags);
        case 12:  // @tupleitem 12|int|wMajorVerNum|Major version number.
            return PyInt_FromLong(p->wMajorVerNum);
        case 13:  // @tupleitem 13|int|wMinorVerNum|Minor version number.
            return PyInt_FromLong(p->wMinorVerNum);
        case 14:  // @tupleitem 14|<o TYPEDESC>|obDescAlias|If TypeKind == pythoncom.TKIND_ALIAS, specifies the type for
                  // which this type is an alias.
            rc = p->obDescAlias ? p->obDescAlias : Py_None;
            Py_INCREF(rc);
            return rc;
        case 15:  // @tupleitem 15|<o IDLDESC>|obIDLDesc|IDL attributes of the described type.
            rc = p->obIDLDesc ? p->obIDLDesc : Py_None;
            Py_INCREF(rc);
            return rc;
    }
    PyErr_SetString(PyExc_IndexError, "index out of range");
    return NULL;
}
