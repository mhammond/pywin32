// PyITypeObjects.cpp
//
// Misc support for IType* helper objects.
//

// @doc

#include "stdafx.h"
#include "PythonCOM.h"

/////////////////////////////////////////////
//
// A little "memory manager", allowing me to
// build a block of memory in increments, but
// have it freed in one call.  Create a root, then
// pass it around, allocating more as necessary.
// Then free the root, and they all go away.
PyObject *PyObject_FromTYPEDESC(const TYPEDESC *td);

struct PyComAllocMore {
    PyComAllocMore *pNext;
    BOOL bForVariant;  // Should be block be VariantClear'd?
};

void *AllocMore(void *pRoot, size_t size, BOOL bForVariant = FALSE)
{
    PyComAllocMore *pRealRoot = ((PyComAllocMore *)pRoot) - 1;
    // pRoot must be a pointer from AllocateMoreBuffer();
    size += sizeof(PyComAllocMore);
    PyComAllocMore *pNewBlock = (PyComAllocMore *)malloc(size);
    if (pNewBlock == NULL)
        return PyErr_NoMemory();
    memset(pNewBlock, 0, size);
    pNewBlock->pNext = pRealRoot->pNext;
    pNewBlock->bForVariant = bForVariant;
    pRealRoot->pNext = pNewBlock;
    return (void *)(pNewBlock + 1);
}

static void FreeBlocks(PyComAllocMore *pRoot)
{
    while (pRoot) {
        PyComAllocMore *nextBlock = pRoot->pNext;
        if (pRoot->bForVariant)
            VariantClear((VARIANT *)(pRoot + 1));
        free(pRoot);
        pRoot = nextBlock;
    }
}

void *AllocateMoreBuffer(size_t size)
{
    size += sizeof(PyComAllocMore);
    PyComAllocMore *pRoot = (PyComAllocMore *)malloc(size);
    if (pRoot == NULL)
        return PyErr_NoMemory();
    memset(pRoot, 0, size);
    pRoot->pNext = 0;
    pRoot->bForVariant = FALSE;
    return (void *)(pRoot + 1);
}

void FreeMoreBuffer(void *pV)
{
    if (pV) {
        PyComAllocMore *pRoot = ((PyComAllocMore *)pV) - 1;
        FreeBlocks(pRoot->pNext);
        free(pRoot);
    }
}
/////////////////////////////////////////////////////////////////////////////
//
// ARRAYDESC support
//
/////////////////////////////////////////////////////////////////////////////
BOOL PyObject_AsARRAYDESC(PyObject *ob, ARRAYDESC **ppDesc, void *pRoot)
{
    PyErr_SetString(PyExc_ValueError, "SAFEARRAY descriptions are not yet supported");
    return FALSE;
}
PyObject *PyObject_FromARRAYDESC(ARRAYDESC *ad)
{
    PyObject *ret = PyTuple_New(1 + ad->cDims);
    PyTuple_SetItem(ret, 0, PyObject_FromTYPEDESC(&ad->tdescElem));
    for (int i = 0; i < ad->cDims; i++) {
        PyTuple_SetItem(ret, 1 + i, Py_BuildValue("(ii)", ad->rgbounds[i].cElements, ad->rgbounds[i].lLbound));
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
// TYPEDESC support
//
/////////////////////////////////////////////////////////////////////////////
BOOL PyObject_AsTYPEDESC(PyObject *ob, TYPEDESC *pDesc, void *pMore)
{
    BOOL rc = FALSE;
    if (PyInt_Check(ob)) {  // a simple VT
        pDesc->vt = (VARTYPE)PyInt_AsLong(ob);
        return TRUE;  // quick exit!
    }
    if (!PySequence_Check(ob) || PySequence_Length(ob) != 2) {
        PyErr_SetString(PyExc_TypeError, "The object is not an TYPEDESC");
        return FALSE;
    }
    PyObject *obType = PySequence_GetItem(ob, 0);
    PyObject *obExtra = PySequence_GetItem(ob, 1);

    if (!PyInt_Check(obType)) {
        PyErr_SetString(PyExc_TypeError, "The first sequence item must be an integer");
        goto done;
    }
    pDesc->vt = (VARTYPE)PyInt_AsLong(obType);
    switch (pDesc->vt) {
        case VT_PTR:
        case VT_SAFEARRAY:
            pDesc->lptdesc = (TYPEDESC *)AllocMore(pMore, sizeof(TYPEDESC));
            if (pDesc->lptdesc == NULL)
                goto done;
            if (!PyObject_AsTYPEDESC(obExtra, pDesc->lptdesc, pMore))
                goto done;
            break;
        case VT_CARRAY:
            if (!PyObject_AsARRAYDESC(obExtra, &pDesc->lpadesc, pMore))
                goto done;
            break;
        case VT_USERDEFINED:
            if (!PyInt_Check(obExtra)) {
                PyErr_SetString(PyExc_TypeError,
                                "If the TYPEDESC is of type VT_USERDEFINED, the object must be an integer");
                goto done;
            }
            pDesc->hreftype = (HREFTYPE)PyInt_AsLong(obExtra);
            break;
        default:
            break;
    }
    rc = TRUE;
done:
    Py_XDECREF(obType);
    Py_XDECREF(obExtra);
    return rc;
}

BOOL PyObject_AsTYPEDESC(PyObject *ob, TYPEDESC **ppDesc)
{
    *ppDesc = (TYPEDESC *)AllocateMoreBuffer(sizeof(TYPEDESC));
    if (*ppDesc == NULL)
        return FALSE;
    BOOL rc = PyObject_AsTYPEDESC(ob, *ppDesc, *ppDesc);
    if (!rc)
        FreeMoreBuffer(*ppDesc);
    return rc;
}

void PyObject_FreeTYPEDESC(TYPEDESC *pDesc) { FreeMoreBuffer(pDesc); }

// @object TYPEDESC|A typedesc is a complicated, recursive object,
// It may be either a simple Python type, or a tuple of (indirectType, object), where object
// may be a simple Python type, or a tuple of etc ...
PyObject *PyObject_FromTYPEDESC(const TYPEDESC *td)
{
    PyObject *p3 = NULL;
    if (td->vt == VT_PTR || td->vt == VT_SAFEARRAY)
        p3 = PyObject_FromTYPEDESC(td->lptdesc);
    else if (td->vt == VT_CARRAY)
        p3 = PyObject_FromARRAYDESC(td->lpadesc);
    else if (td->vt == VT_USERDEFINED)
        p3 = PyInt_FromLong(td->hreftype);

    if (p3) {
        PyObject *ret = Py_BuildValue("(iO)", td->vt, p3);
        Py_DECREF(p3);
        return ret;
    }
    else
        return PyInt_FromLong(td->vt);
}

/////////////////////////////////////////////////////////////////////////////
//
// ELEMDESC support
//
/////////////////////////////////////////////////////////////////////////////
BOOL PyObject_AsELEMDESC(PyObject *ob, ELEMDESC *pDesc, void *pMore)
{
    // pMore must have come from AllocateMoreBlock()
    BOOL rc = FALSE;
    if (!PySequence_Check(ob) || PySequence_Length(ob) != 3) {
        PyErr_SetString(PyExc_TypeError, "The object is not an ELEMDESC");
        return FALSE;
    }
    PyObject *obtd = PySequence_GetItem(ob, 0);
    PyObject *obParamFlags = PySequence_GetItem(ob, 1);
    PyObject *obDefaultVal = PySequence_GetItem(ob, 2);

    if (!PyInt_Check(obParamFlags)) {
        PyErr_SetString(PyExc_TypeError, "The second sequence item must be an integer");
        goto done;
    }
    pDesc->paramdesc.wParamFlags = (WORD)PyInt_AsLong(obParamFlags);

    if (obDefaultVal != Py_None) {
        pDesc->paramdesc.wParamFlags |= PARAMFLAG_FHASDEFAULT;
        pDesc->paramdesc.pparamdescex = (LPPARAMDESCEX)AllocMore(pMore, sizeof(PARAMDESCEX), TRUE);
        pDesc->paramdesc.pparamdescex->cBytes = sizeof(PARAMDESCEX);
        /// XXX - this leaks this variant :-(
        // To avoid, we could maybe alloc a new More block with VARIANT set
        // to True, then memcpy this variant into it??
        // Or have PyObject_FreeFUNCDESC() do the right thing, looking down
        // each elemdesc freeing the variant?
        // However, this is very very rarely used (possibly never in the real world!)
        VariantInit(&pDesc->paramdesc.pparamdescex->varDefaultValue);
        if (!PyCom_VariantFromPyObject(obDefaultVal, &pDesc->paramdesc.pparamdescex->varDefaultValue))
            goto done;
    }
    rc = PyObject_AsTYPEDESC(obtd, &pDesc->tdesc, pMore);
done:
    Py_XDECREF(obtd);
    Py_XDECREF(obParamFlags);
    Py_XDECREF(obDefaultVal);
    return rc;
}

BOOL PyObject_AsELEMDESCArray(PyObject *ob, ELEMDESC **ppDesc, short *pNum, void *pMore)
{
    if (!PySequence_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "ELEMDESCArray must be a sequence of ELEMDESCs");
        return FALSE;
    }
    *pNum = PySequence_Length(ob);
    *ppDesc = (ELEMDESC *)AllocMore(pMore, sizeof(ELEMDESC) * *pNum);
    if (*ppDesc == NULL)
        return NULL;

    for (int i = 0; i < *pNum; i++) {
        PyObject *sub = PySequence_GetItem(ob, i);
        if (sub == NULL)
            return FALSE;
        BOOL ok = PyObject_AsELEMDESC(sub, (*ppDesc) + i, pMore);
        Py_DECREF(sub);
        if (!ok)
            return FALSE;
    }
    return TRUE;
}

// @object ELEMDESC|An ELEMDESC is respresented as a tuple of
PyObject *PyObject_FromELEMDESC(const ELEMDESC *ed)
{
    // @tupleitem 0|<o TYPEDESC>|typeDesc|The type description.
    // @tupleitem 1|int|idlFlags|
    // @tupleitem 2|object|default|If PARAMFLAG_FHASDEFAULT are set, then this is the default value.
    PyObject *defaultValue = NULL;
    if (ed->idldesc.wIDLFlags & PARAMFLAG_FHASDEFAULT && ed->paramdesc.pparamdescex)
        defaultValue = PyCom_PyObjectFromVariant(&ed->paramdesc.pparamdescex->varDefaultValue);
    if (defaultValue == NULL) {
        defaultValue = Py_None;
        Py_INCREF(Py_None);
    }
    PyObject *td = PyObject_FromTYPEDESC(&ed->tdesc);
    PyObject *ret = Py_BuildValue("(OiO)", td, ed->paramdesc.wParamFlags, defaultValue);
    Py_DECREF(td);
    Py_DECREF(defaultValue);
    return ret;
}

PyObject *PyObject_FromELEMDESCArray(ELEMDESC *ed, int len)
{
    PyObject *ret = PyTuple_New(len);
    for (int i = 0; i < len; i++) PyTuple_SetItem(ret, i, PyObject_FromELEMDESC(ed + i));
    return ret;
}
