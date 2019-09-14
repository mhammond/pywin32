// axcontrol_pch.h : header file for PCH generation for the olectl COM extension

#include <PythonCOM.h>
#include <PythonCOMServer.h>
#include <oleauto.h>
#include <ocidl.h>  // Used to be <multinfo.h>

extern BOOL PyObject_AsLOGPALETTE(PyObject *pbLogPal, LOGPALETTE **ppLogPal);
extern void PyObject_FreeLOGPALETTE(LOGPALETTE *pLogPal);
extern PyObject *PyObject_FromLOGPALETTE(LOGPALETTE *pLP);

extern BOOL PyObject_AsDVTARGETDEVICE(PyObject *ob, DVTARGETDEVICE **pptd);
extern void PyObject_FreeDVTARGETDEVICE(DVTARGETDEVICE *ptd);
extern PyObject *PyObject_FromDVTARGETDEVICE(DVTARGETDEVICE *pTD);

// We should not be using this!
#define OleSetOleError PyCom_BuildPyException
