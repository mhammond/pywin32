#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"

// @doc

// mem manager helpers.
extern void *AllocateMoreBuffer(size_t size);
extern void *AllocMore(void *pRoot, size_t size, BOOL bForVariant = FALSE);
extern void FreeMoreBuffer(void *);

static PyObject *MakeSCODEArray(SCODE *sa, int len)
{
    PyObject *ret = PyTuple_New(len);
    for (int i = 0; i < len; i++) PyTuple_SetItem(ret, i, PyInt_FromLong(sa[i]));
    return ret;
}

// @pymethod <o FUNCDESC>|pythoncom|FUNCDESC|Creates a new FUNCDESC object
PyObject *Py_NewFUNCDESC(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return new PyFUNCDESC();
}

PyObject *PyObject_FromFUNCDESC(FUNCDESC *desc) { return new PyFUNCDESC(desc); }

BOOL PyObject_AsFUNCDESC(PyObject *ob, FUNCDESC **ppfd)
{
    BOOL rc = FALSE;
    if (ob->ob_type != &PyFUNCDESC::Type) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyFUNCDESC");
        return FALSE;
    }
    PyFUNCDESC *pyfd = (PyFUNCDESC *)ob;

    FUNCDESC *fd = (FUNCDESC *)AllocateMoreBuffer(sizeof(FUNCDESC));
    if (fd == NULL) {
        PyErr_SetString(PyExc_MemoryError, "FUNCDESC");
        return FALSE;
    }
    fd->memid = pyfd->memid;
    fd->funckind = (FUNCKIND)pyfd->funckind;
    fd->invkind = (INVOKEKIND)pyfd->invkind;
    fd->callconv = (CALLCONV)pyfd->callconv;
    fd->cParamsOpt = pyfd->cParamsOpt;
    fd->oVft = pyfd->oVft;
    fd->wFuncFlags = pyfd->wFuncFlags;

    // Convert the args and rettype
    if (pyfd->rettype) {
        if (!PyObject_AsELEMDESC(pyfd->rettype, &fd->elemdescFunc, fd))
            goto done;
    }
    if (pyfd->args) {
        if (!PyObject_AsELEMDESCArray(pyfd->args, &fd->lprgelemdescParam, &fd->cParams, fd))
            goto done;
    }
    if (pyfd->scodeArray) {
        // Convert the scode array.
        if (!PySequence_Check(pyfd->scodeArray)) {
            PyErr_SetString(PyExc_TypeError, "SCODE array must be a sequence of integers!");
            goto done;
        }
        fd->cScodes = (short)PySequence_Length(pyfd->scodeArray);
        fd->lprgscode = (SCODE *)AllocMore(fd, sizeof(SCODE) * fd->cScodes);
        for (Py_ssize_t i = 0; i < fd->cScodes; i++) {
            PyObject *sub = PySequence_GetItem(ob, i);
            if (sub == NULL)
                goto done;
            BOOL ok = PyInt_Check(sub);
            if (ok)
                fd->lprgscode[i] = PyInt_AsLong(sub);
            else
                PyErr_SetString(PyExc_TypeError, "SCODE array must be a sequence of integers!");
            Py_DECREF(sub);
            if (!ok)
                goto done;
        }
    }
    rc = TRUE;
done:
    if (!rc && fd)
        FreeMoreBuffer(fd);
    else
        *ppfd = fd;
    return rc;
}

void PyObject_FreeFUNCDESC(FUNCDESC *pFuncDesc) { FreeMoreBuffer(pFuncDesc); }

// The object itself.
struct PyMethodDef PyFUNCDESC::methods[] = {{NULL}};

// @object FUNCDESC|A FUNCDESC object represents a COM TYPEATTR structure.

// Sequence stuff to provide compatibility with tuples.
static PySequenceMethods PyFUNCDESC_Sequence = {
    PyFUNCDESC::getlength,  // sq_length;
    NULL,                   // sq_concat;
    NULL,                   // sq_repeat;
    PyFUNCDESC::getitem,    // sq_item;
    NULL,                   // sq_slice;
    NULL,                   // sq_ass_item;
    NULL,                   // sq_ass_slice;
};

PyTypeObject PyFUNCDESC::Type = {
    PYWIN_OBJECT_HEAD "PyFUNCDESC",
    sizeof(PyFUNCDESC),
    0,
    PyFUNCDESC::deallocFunc, /* tp_dealloc */
    0,                       /* tp_print */
    0,                       /* tp_getattr */
    0,                       /* tp_setattr */
    0,                       /* tp_compare */
    0,                       /* tp_repr */
    0,                       /* tp_as_number */
    &PyFUNCDESC_Sequence,    /* tp_as_sequence */
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
    PyFUNCDESC::methods,     /* tp_methods */
    PyFUNCDESC::members,     /* tp_members */
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

#define OFF(e) offsetof(PyFUNCDESC, e)

/*static*/ struct PyMemberDef PyFUNCDESC::members[] = {
    {"memid", T_INT, OFF(memid)},               // @prop integer|memid|
    {"scodeArray", T_OBJECT, OFF(scodeArray)},  // @prop (int, ...)|scodeArray|
    {"args", T_OBJECT, OFF(args)},              // @prop (<o ELEMDESC>, ...)|args|
    {"funckind", T_INT, OFF(funckind)},         // @prop int|funckind|
    {"invkind", T_INT, OFF(invkind)},           // @prop int|invkind|
    {"callconv", T_INT, OFF(callconv)},         // @prop int|callconv|
    {"cParamsOpt", T_INT, OFF(cParamsOpt)},     // @prop int|cParamsOpt|
    {"oVft", T_INT, OFF(oVft)},                 // @prop int|oVft|
    {"rettype", T_OBJECT, OFF(rettype)},        // @prop <o ELEMDESC>|rettype|
    {"wFuncFlags", T_INT, OFF(wFuncFlags)},     // @prop int|wFuncFlags|
    {NULL}};

PyFUNCDESC::PyFUNCDESC()
{
    ob_type = &PyFUNCDESC::Type;
    _Py_NewReference(this);

    memid = 0;
    scodeArray = NULL;
    args = NULL;
    funckind = invkind = callconv = cParamsOpt = oVft = 0;
    rettype = NULL;
    wFuncFlags = 0;
}

PyFUNCDESC::PyFUNCDESC(const FUNCDESC *desc)
{
    ob_type = &PyFUNCDESC::Type;
    _Py_NewReference(this);

    scodeArray = MakeSCODEArray(desc->lprgscode, desc->cScodes);
    args = PyObject_FromELEMDESCArray(desc->lprgelemdescParam, desc->cParams);
    rettype = PyObject_FromELEMDESC(&desc->elemdescFunc);
    memid = desc->memid;
    funckind = desc->funckind;
    invkind = desc->invkind;
    callconv = desc->callconv;
    cParamsOpt = desc->cParamsOpt;
    oVft = desc->oVft;
    wFuncFlags = desc->wFuncFlags;
}

PyFUNCDESC::~PyFUNCDESC()
{
    Py_XDECREF(scodeArray);
    Py_XDECREF(args);
    Py_XDECREF(rettype);
}

/*static*/ void PyFUNCDESC::deallocFunc(PyObject *ob) { delete (PyFUNCDESC *)ob; }

// Sequence stuff to provide compatibility with tuples.
/* static */ Py_ssize_t PyFUNCDESC::getlength(PyObject *self)
{
    // NEVER CHANGE THIS - you will break all the old
    // code written when these object were tuples!
    return 10;
}

/* static */ PyObject *PyFUNCDESC::getitem(PyObject *self, Py_ssize_t index)
{
    PyFUNCDESC *p = (PyFUNCDESC *)self;
    PyObject *rc;
    switch (index) {
        case 0:  // @tupleitem 0|int|memid|
            return PyInt_FromLong(p->memid);
        case 1:  // @tupleitem 1|(int, ...)|scodeArray|
            rc = p->scodeArray ? p->scodeArray : Py_None;
            Py_INCREF(rc);
            return rc;
        case 2:  // @tupleitem 2|(<o ELEMDESC>, ...)|args|
            rc = p->args ? p->args : Py_None;
            Py_INCREF(rc);
            return rc;
        case 3:  // @tupleitem 3|int|funckind|
            return PyInt_FromLong(p->funckind);
        case 4:  // @tupleitem 4|int|invkind|
            return PyInt_FromLong(p->invkind);
        case 5:  // @tupleitem 5|int|callconv|
            return PyInt_FromLong(p->callconv);
        case 6:  // @tupleitem 6|int|cParamsOpt|
            return PyInt_FromLong(p->cParamsOpt);
        case 7:  // @tupleitem 7|int|oVft|
            return PyInt_FromLong(p->oVft);
        case 8:  // @tupleitem 8|<o ELEMDESC>|rettype|
            rc = p->rettype ? p->rettype : Py_None;
            Py_INCREF(rc);
            return rc;
        case 9:  // @tupleitem 9|int|wFuncFlags|
            return PyInt_FromLong(p->wFuncFlags);
    }
    PyErr_SetString(PyExc_IndexError, "index out of range");
    return NULL;
}
