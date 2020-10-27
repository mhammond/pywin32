#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"
// @doc

PyIErrorLog::PyIErrorLog(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIErrorLog::~PyIErrorLog() {}

/* static */ IErrorLog *PyIErrorLog::GetI(PyObject *self) { return (IErrorLog *)PyIUnknown::GetI(self); }

// @pymethod |PyIErrorLog|AddError|Adds an error to the error log.
PyObject *PyIErrorLog::AddError(PyObject *self, PyObject *args)
{
    PyObject *obName;
    PyObject *obExcepInfo = Py_None;
    // @pyparm string|propName||The name of the error
    // @pyparm exception|excepInfo|None|A COM exception.  Must be a complete COM exception (ie, pythoncom.com_error, or
    // win32com.server.exceptions.COMException())
    if (!PyArg_ParseTuple(args, "O|O:AddError", &obName, &obExcepInfo))
        return NULL;

    BSTR propName;
    if (!PyWinObject_AsBstr(obName, &propName))
        return NULL;
    EXCEPINFO excepInfo;
    EXCEPINFO *pExcepInfo;
    if (obExcepInfo && obExcepInfo != Py_None) {
        if (!PyCom_ExcepInfoFromPyObject(obExcepInfo, &excepInfo))
            return NULL;
        pExcepInfo = &excepInfo;
    }
    else {
        pExcepInfo = NULL;
    }

    IErrorLog *pIEL = GetI(self);
    if (pIEL == NULL)
        return NULL;

    PY_INTERFACE_PRECALL;
    HRESULT hr = pIEL->AddError(propName, pExcepInfo);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeBstr(propName);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIEL, IID_IErrorLog);

    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIErrorLog|A Python wrapper for a COM IErrorLog interface.
// @comm The IErrorLog interface is an abstraction for an error log that is used to
// communicate detailed error information between a client and an object.
// The caller of the single interface method, <om PyIErrorLog::AddError>, simply logs an error
// where the error is an EXCEPINFO structure related to a specific property.
// The implementer of the interface is responsible for handling the error in whatever way it desires.
// <nl>IErrorLog is used in the protocol between a client that implements <o PyIPropertyBag> and an
// object that implements <o PyIPersistPropertyBag>.
static struct PyMethodDef PyIErrorLog_methods[] = {
    {"AddError", PyIErrorLog::AddError, 1},  // @pymeth AddError|Adds an error to the error log.
    {NULL}};

PyComTypeObject PyIErrorLog::type("PyIErrorLog",
                                  &PyIUnknown::type,  // @base PyIErrorLog|PyIUnknown
                                  sizeof(PyIErrorLog), PyIErrorLog_methods, GET_PYCOM_CTOR(PyIErrorLog));
