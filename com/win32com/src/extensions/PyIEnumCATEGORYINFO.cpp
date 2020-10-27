// PyICategory

// @doc
#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_IENUMCATEGORYINFO

#include <comcat.h>
#include "PyIEnumGUID.h"

PyIEnumCATEGORYINFO::PyIEnumCATEGORYINFO(IUnknown *pDisp) : PyIUnknown(pDisp) { ob_type = &type; }

PyIEnumCATEGORYINFO::~PyIEnumCATEGORYINFO() {}

/*static*/ IEnumCATEGORYINFO *PyIEnumCATEGORYINFO::GetI(PyObject *self)
{
    return (IEnumCATEGORYINFO *)PyIUnknown::GetI(self);
}

// @pymethod ( (<o PyIID>, int, string), ...)|PyIEnumCATEGORYINFO|Next|Retrieves a specified number of items in the
// enumeration sequence.
PyObject *PyIEnumCATEGORYINFO::Next(PyObject *self, PyObject *args)
{
    long celt = 1;
    // @pyparm int|num|1|Number of items to retrieve.
    if (!PyArg_ParseTuple(args, "|l:Next", &celt))
        return NULL;

    IEnumCATEGORYINFO *pMy = GetI(self);
    if (pMy == NULL)
        return NULL;

    CATEGORYINFO *rgVar = new CATEGORYINFO[celt];
    if (rgVar == NULL) {
        PyErr_SetString(PyExc_MemoryError, "allocating result CATEGORYINFOs");
        return NULL;
    }

    int i;
    for (i = celt; i--;) memset(rgVar + i, '\0', sizeof(CATEGORYINFO));

    ULONG celtFetched = 0;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->Next(celt, rgVar, &celtFetched);
    PY_INTERFACE_POSTCALL;
    if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS && FAILED(hr)) {
        delete[] rgVar;
        return PyCom_BuildPyException(hr);
    }

    PyObject *result = PyTuple_New(celtFetched);
    if (result != NULL) {
        for (i = celtFetched; i--;) {
            PyObject *obNewIID = PyWinObject_FromIID(rgVar[i].catid);
            PyObject *ob = Py_BuildValue("OiN", obNewIID, rgVar[i].lcid, PyWinObject_FromWCHAR(rgVar[i].szDescription));
            Py_XDECREF(obNewIID);
            if (ob == NULL) {
                Py_DECREF(result);
                result = NULL;
                break;
            }
            PyTuple_SET_ITEM(result, i, ob);
        }
    }
    delete[] rgVar;
    return result;
    // @rdesc The result is a tuple of (IID object, LCID, string description) tuples,
    // one for each element returned.
}
// @pymethod |PyIEnumCATEGORYINFO|Skip|Skips over the next specified elementes.
PyObject *PyIEnumCATEGORYINFO::Skip(PyObject *self, PyObject *args)
{
    ULONG num;
    // @pyparm int|num||The number of elements being requested.
    if (!PyArg_ParseTuple(args, "l:Skip", &num))
        return NULL;

    IEnumCATEGORYINFO *pMy = GetI(self);
    if (pMy == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->Skip(num);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod |PyIEnumCATEGORYINFO|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumCATEGORYINFO::Reset(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Reset"))
        return NULL;

    IEnumCATEGORYINFO *pMy = GetI(self);
    if (pMy == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->Reset();
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIEnumCATEGORYINFO>|PyIEnumCATEGORYINFO|Clone|Creates another enumerator that contains the same
// enumeration state as the current one
PyObject *PyIEnumCATEGORYINFO::Clone(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Clone"))
        return NULL;

    IEnumCATEGORYINFO *pMy = GetI(self);
    if (pMy == NULL)
        return NULL;

    IEnumCATEGORYINFO *pNew = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = pMy->Clone(&pNew);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    return PyCom_PyObjectFromIUnknown(pNew, IID_IEnumCATEGORYINFO, FALSE);
}

// @object PyIEnumCATEGORYINFO|A Python interface to IEnumCATEGORYINFO
static struct PyMethodDef PyIEnumCATEGORYINFO_methods[] = {
    {"Next", PyIEnumCATEGORYINFO::Next,
     1},  // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
    {"Skip", PyIEnumCATEGORYINFO::Skip, 1},    // @pymeth Skip|Skips over the next specified elementes.
    {"Reset", PyIEnumCATEGORYINFO::Reset, 1},  // @pymeth Reset|Resets the enumeration sequence to the beginning.
    {"Clone", PyIEnumCATEGORYINFO::Clone,
     1},  // @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
    {NULL, NULL}};

PyComEnumTypeObject PyIEnumCATEGORYINFO::type("PyIEnumCATEGORYINFO",
                                              &PyIUnknown::type,  // @base PyIEnumCATEGORYINFO|PyIUnknown
                                              sizeof(PyIEnumCATEGORYINFO), PyIEnumCATEGORYINFO_methods,
                                              GET_PYCOM_CTOR(PyIEnumCATEGORYINFO));

#endif  // NO_PYCOM_IENUMCATEGORYINFO
