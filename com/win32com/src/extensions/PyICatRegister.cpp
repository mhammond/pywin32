#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_ICATREGISTER

#include "comcat.h"
#include "PyICatRegister.h"
/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

BOOL CATIDsFromPyObject(PyObject *obCatIds, CATID **ppCatIds, UINT *pNumIds)
{
    if (!PySequence_Check(obCatIds)) {
        PyErr_SetString(PyExc_TypeError, "Object must be a sequence of CATIDs");
        return FALSE;
    }
    Py_ssize_t len = PyObject_Length(obCatIds);
    CATID *ids = new CATID[len];
    BOOL rc = TRUE;
    for (Py_ssize_t i = 0; rc && i < len; i++) {
        PyObject *obThis = PySequence_GetItem(obCatIds, i);
        if (obThis == NULL) {
            rc = FALSE;
            break;
        }
        if (!PyWinObject_AsIID(obThis, ids + i)) {
            PyErr_SetString(PyExc_TypeError, "CATID is not valid");
            rc = FALSE;
        }
        Py_DECREF(obThis);
    }
    if (rc) {
        *ppCatIds = ids;
        *pNumIds = PyWin_SAFE_DOWNCAST(len, Py_ssize_t, UINT);
    }
    else {
        delete[] ids;
    }
    return rc;
}

void DeleteCATIDs(CATID *pCatIds) { delete[] pCatIds; }
PyICatRegister::PyICatRegister(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyICatRegister::~PyICatRegister() {}

/* static */ ICatRegister *PyICatRegister::GetI(PyObject *self) { return (ICatRegister *)PyIUnknown::GetI(self); }

// @pymethod |PyICatRegister|RegisterCategories|Registers one or more component categories. Each component category
// consists of a CATID and a list of locale-dependent description strings.
PyObject *PyICatRegister::RegisterCategories(PyObject *self, PyObject *args)
{
    PyObject *obCatList;
    // @pyparm [ (<o PyIID>, int, string), ...]|[ (catId, lcid, description), ...]||A sequence of category descriptions.
    if (!PyArg_ParseTuple(args, "O:RegisterCategories", &obCatList))
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    if (!PySequence_Check(obCatList)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a list of CATEGORYINFO tuples");
        return NULL;
    }
    Py_ssize_t noInfos = PyObject_Length(obCatList);
    CATEGORYINFO *infos = new CATEGORYINFO[noInfos];
    if (infos == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating CATEGORYINFO array");
        return NULL;
    }
    for (Py_ssize_t i = 0; i < noInfos; i++) {
        PyObject *obCatId;
        PyObject *obThis = PySequence_GetItem(obCatList, i);
        if (obThis == NULL)
            return NULL;
        BOOL ok = TRUE;
        PyObject *obDesc;
        if (!PyArg_ParseTuple(obThis, "OlO", &obCatId, (long *)&infos[i].lcid, &obDesc)) {
            Py_DECREF(obThis);
            PyErr_SetString(PyExc_TypeError, "Category infos must be CATID, lcid, description");
            delete[] infos;
            return NULL;
        }
        Py_DECREF(obThis);
        if (!PyWinObject_AsIID(obCatId, &infos[i].catid)) {
            delete[] infos;
            return NULL;
        }
        OLECHAR *oc;
        if (!PyWinObject_AsWCHAR(obDesc, &oc, FALSE)) {
            delete[] infos;
            return NULL;
        }
        wcsncpy(infos[i].szDescription, oc, sizeof(infos->szDescription) / sizeof(infos->szDescription[0]));
        PyWinObject_FreeWCHAR(oc);
    }

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->RegisterCategories(PyWin_SAFE_DOWNCAST(noInfos, Py_ssize_t, ULONG), infos);
    PY_INTERFACE_POSTCALL;
    delete[] infos;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod |PyICatRegister|UnregisterCategories|Unregister one or more previously registered categories.
PyObject *PyICatRegister::UnRegisterCategories(PyObject *self, PyObject *args)
{
    PyObject *obCatIds;
    // @pyparm [<o PyIID>, ...]|[catId, ...]||The list of category IDs to be unregistered.
    if (!PyArg_ParseTuple(args, "O:UnRegisterCategories", &obCatIds))
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    CATID *pids;
    UINT numIds;
    if (!CATIDsFromPyObject(obCatIds, &pids, &numIds))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->UnRegisterCategories(numIds, pids);
    PY_INTERFACE_POSTCALL;

    DeleteCATIDs(pids);

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyICatRegister|RegisterClassImplCategories|Registers the class as implementing one or more component
// categories.
PyObject *PyICatRegister::RegisterClassImplCategories(PyObject *self, PyObject *args)
{
    PyObject *obCLSID, *obCatIds;
    if (!PyArg_ParseTuple(args, "OO:RegisterClassImplCategories",
                          &obCLSID,    // @pyparm <o PyIID>|clsid||Class ID of the relevent class
                          &obCatIds))  // @pyparm [<o PyIID>, ...]|[catId, ...]||A sequence of category IDs to be
                                       // associated with the class.
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid)) {
        PyErr_SetString(PyExc_TypeError, "CLSID is not valid");
        return NULL;
    }

    CATID *pids;
    UINT numIds;
    if (!CATIDsFromPyObject(obCatIds, &pids, &numIds))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->RegisterClassImplCategories(clsid, numIds, pids);
    PY_INTERFACE_POSTCALL;

    DeleteCATIDs(pids);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyICatRegister|UnRegisterClassImplCategories|Unregisters the class as implementing one or more component
// categories.
PyObject *PyICatRegister::UnRegisterClassImplCategories(PyObject *self, PyObject *args)
{
    PyObject *obCLSID, *obCatIds;
    if (!PyArg_ParseTuple(args, "OO:UnRegisterClassImplCategories",
                          &obCLSID,    // @pyparm <o PyIID>|clsid||Class ID of the relevent class
                          &obCatIds))  // @pyparm [<o PyIID>, ...]|[catId, ...]||A sequence of category IDs to be
                                       // unregistered from the class.
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid)) {
        PyErr_SetString(PyExc_TypeError, "CLSID is not valid");
        return NULL;
    }

    CATID *pids;
    UINT numIds;
    if (!CATIDsFromPyObject(obCatIds, &pids, &numIds))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->UnRegisterClassImplCategories(clsid, numIds, pids);
    PY_INTERFACE_POSTCALL;

    DeleteCATIDs(pids);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyICatRegister|RegisterClassReqCategories|Registers the class as requiring one or more component
// categories.
PyObject *PyICatRegister::RegisterClassReqCategories(PyObject *self, PyObject *args)
{
    PyObject *obCLSID, *obCatIds;
    if (!PyArg_ParseTuple(args, "OO:RegisterClassReqCategories",
                          &obCLSID,    // @pyparm <o PyIID>|clsid||Class ID of the relevent class
                          &obCatIds))  // @pyparm [<o PyIID>, ...]|[catId, ...]||A sequence of category IDs to be
                                       // associated with the class.
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid)) {
        PyErr_SetString(PyExc_TypeError, "CLSID is not valid");
        return NULL;
    }

    CATID *pids;
    UINT numIds;
    if (!CATIDsFromPyObject(obCatIds, &pids, &numIds))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->RegisterClassReqCategories(clsid, numIds, pids);
    PY_INTERFACE_POSTCALL;

    DeleteCATIDs(pids);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);

    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyICatRegister|UnRegisterClassReqCategories|Unregisters the class as requiring one or more component
// categories.
PyObject *PyICatRegister::UnRegisterClassReqCategories(PyObject *self, PyObject *args)
{
    PyObject *obCLSID, *obCatIds;
    if (!PyArg_ParseTuple(args, "OO:UnRegisterClassReqCategories",
                          &obCLSID,    // @pyparm <o PyIID>|clsid||Class ID of the relevent class
                          &obCatIds))  // @pyparm [<o PyIID>, ...]|[catId, ...]||A sequence of category IDs to be
                                       // unregistered for the class.
        return NULL;

    ICatRegister *pICR = GetI(self);
    if (pICR == NULL)
        return NULL;

    CLSID clsid;
    if (!PyWinObject_AsIID(obCLSID, &clsid)) {
        PyErr_SetString(PyExc_TypeError, "CLSID is not valid");
        return NULL;
    }

    CATID *pids;
    UINT numIds;
    if (!CATIDsFromPyObject(obCatIds, &pids, &numIds))
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pICR->UnRegisterClassReqCategories(clsid, numIds, pids);
    PY_INTERFACE_POSTCALL;

    DeleteCATIDs(pids);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pICR, IID_ICatRegister);

    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyICatRegister|An interface to a COM ICatRegister interface.
static struct PyMethodDef PyICatRegister_methods[] = {
    {"RegisterCategories", PyICatRegister::RegisterCategories,
     1},  // @pymeth RegisterCategories|Registers one or more component categories. Each component category consists of
          // a CATID and a list of locale-dependent description strings.
    {"UnRegisterCategories", PyICatRegister::UnRegisterCategories,
     1},  // @pymeth UnRegisterCategories|Unregister one or more previously registered categories.
    {"RegisterClassImplCategories", PyICatRegister::RegisterClassImplCategories,
     1},  // @pymeth RegisterClassImplCategories|Registers the class as implementing one or more component categories.
    {"UnRegisterClassImplCategories", PyICatRegister::UnRegisterClassImplCategories,
     1},  // @pymeth UnRegisterClassImplCategories|Unregisters the class as implementing one or more component
          // categories.
    {"RegisterClassReqCategories", PyICatRegister::RegisterClassReqCategories,
     1},  // @pymeth RegisterClassReqCategories|Registers the class as requiring one or more component categories.
    {"UnRegisterClassReqCategories", PyICatRegister::UnRegisterClassReqCategories,
     1},  // @pymeth UnRegisterClassReqCategories|Unregisters the class as requiring one or more component categories.
    {NULL}};

PyComTypeObject PyICatRegister::type("PyICatRegister",
                                     &PyIUnknown::type,  // @base PyICatRegister|PyIUnknown
                                     sizeof(PyICatRegister), PyICatRegister_methods, GET_PYCOM_CTOR(PyICatRegister));

#endif  // NO_PYCOM_ICATREGISTER
