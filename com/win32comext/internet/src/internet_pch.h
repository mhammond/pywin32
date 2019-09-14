// axcontrol_pch.h : header file for PCH generation for the olectl COM extension

#include <PythonCOM.h>
#include <PythonCOMServer.h>
#include <oleauto.h>
#include <ocidl.h>  // Used to be <multinfo.h>
#include <urlmon.h>

BOOL PyObject_AsPROTOCOLDATA(PyObject *ob, PROTOCOLDATA *pPD);
PyObject *PyObject_FromPROTOCOLDATA(PROTOCOLDATA *pPD);

BOOL PyObject_AsBINDINFO(PyObject *ob, BINDINFO *pPD);
PyObject *PyObject_FromBINDINFO(BINDINFO *pPD);

// We should not be using this!
#define OleSetOleError PyCom_BuildPyException
