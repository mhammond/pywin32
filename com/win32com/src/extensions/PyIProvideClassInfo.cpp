// PyIProvideClassInfo

// @doc
#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_IPROVIDECLASSINFO

PyIProvideClassInfo::PyIProvideClassInfo(IUnknown *pDisp) : PyIUnknown(pDisp) { ob_type = &type; }

PyIProvideClassInfo::~PyIProvideClassInfo() {}

/*static*/ IProvideClassInfo *PyIProvideClassInfo::GetI(PyObject *self)
{
    return (IProvideClassInfo *)PyIUnknown::GetI(self);
}

// @pymethod <o PyITypeInfo>|PyIProvideClassInfo|GetClassInfo|Gets information about the CO_CLASS.
PyObject *PyIProvideClassInfo::GetClassInfo(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetClassInfo"))
        return NULL;

    IProvideClassInfo *pMy = GetI(self);
    if (pMy == NULL)
        return NULL;
    ITypeInfo *pti = NULL;
    PY_INTERFACE_PRECALL;
    SCODE sc = pMy->GetClassInfo(&pti);
    PY_INTERFACE_POSTCALL;
    if (S_OK != sc)  // S_OK only acceptable
        return PyCom_BuildPyException(sc, pMy, IID_IProvideClassInfo);
    return PyCom_PyObjectFromIUnknown(pti, IID_ITypeInfo);
}

// @object PyIProvideClassInfo|A Python interface to IProvideClassInfo
static struct PyMethodDef PyIProvideClassInfo_methods[] = {
    {"GetClassInfo", PyIProvideClassInfo::GetClassInfo,
     1},  // @pymeth GetClassInfo|Gets information about the CO_CLASS.
    {NULL, NULL}};

PyComTypeObject PyIProvideClassInfo::type("PyIProvideClassInfo",
                                          &PyIUnknown::type,  // @base PyIProvideClassInfo|PyIUnknown
                                          sizeof(PyIProvideClassInfo), PyIProvideClassInfo_methods,
                                          GET_PYCOM_CTOR(PyIProvideClassInfo));

//////////////////////////////////////////////////////////
// IProvideClassInfo2 client support.

PyIProvideClassInfo2::PyIProvideClassInfo2(IUnknown *pDisp) : PyIProvideClassInfo(pDisp) { ob_type = &type; }

PyIProvideClassInfo2::~PyIProvideClassInfo2() {}

/*static*/ IProvideClassInfo2 *PyIProvideClassInfo2::GetI(PyObject *self)
{
    return (IProvideClassInfo2 *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIID>|PyIProvideClassInfo2|GetGUID|Gets the GUID for the object.
PyObject *PyIProvideClassInfo2::GetGUID(PyObject *self, PyObject *args)
{
    int flags;
    // @pyparm int|flags||The flags for the GUID.
    if (!PyArg_ParseTuple(args, "i:GetGUID", &flags))
        return NULL;

    IProvideClassInfo2 *pMyInfo = GetI(self);
    if (pMyInfo == NULL)
        return NULL;
    GUID guid;
    PY_INTERFACE_PRECALL;
    SCODE sc = pMyInfo->GetGUID(flags, &guid);
    PY_INTERFACE_POSTCALL;
    if (FAILED(sc))
        return PyCom_BuildPyException(sc, pMyInfo, IID_IProvideClassInfo2);
    return PyWinObject_FromIID(guid);
}

// @object PyIProvideClassInfo2|
static struct PyMethodDef PyIProvideClassInfo2_methods[] = {
    {"GetGUID", PyIProvideClassInfo2::GetGUID,
     1},  // @pymeth GetGUID|Gets the default event sink IID for the object (if any).
    {NULL, NULL}};

PyComTypeObject PyIProvideClassInfo2::type(
    "PyIProvideClassInfo2",
    &PyIProvideClassInfo::type,  // @base PyIProvideClassInfo2|PyIProvideClassInfo
    sizeof(PyIProvideClassInfo2), PyIProvideClassInfo2_methods, GET_PYCOM_CTOR(PyIProvideClassInfo2));

#endif  // NO_PYCOM_IPROVIDECLASSINFO
