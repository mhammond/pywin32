/* File : PyIMAPIStatus.i */

%module IMAPIStatus  // Provides status information about the MAPI
                     // subsystem, the integrated address book and the MAPI
                     // spooler.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMAPIStatus.h"

PyIMAPIStatus::PyIMAPIStatus(IUnknown *pDisp) :
    PyIMAPIProp(pDisp)
{
    ob_type = &type;
}

PyIMAPIStatus::~PyIMAPIStatus()
{
}

/*static*/ IMAPIStatus *PyIMAPIStatus::GetI(PyObject *self)
{
    return (IMAPIStatus *)PyIUnknown::GetI(self);
}

%}

// @pyswig |ChangePassword|
// @pyparm unicode|oldPassword||
// @pyparm unicode|newPassword||
// @pyparm int|ulFlags||
HRESULT ChangePassword(TCHAR *old, TCHAR *newPassword, ULONG ulFlags);

// @pyswig |SettingsDialog|
// @pyparm int|ulUIParam||
// @pyparm int|ulFlags||
HRESULT SettingsDialog(ULONG ulUIParam, ULONG ulFlags);

// @pyswig |ValidateState|
// @pyparm int|ulUIParam||
// @pyparm int|ulFlags||
HRESULT ValidateState(ULONG ulUIParam, ULONG ulFlags);

// @pyswig |FlushQueues|
// @pyparm int|ulUIParam||
// @pyparm string|transport||Blob of data
// @pyparm int|ulFlags||
%{
// @pyswig |FlushQueues|
PyObject *PyIMAPIStatus::FlushQueues(PyObject *self, PyObject *args) 
{
    IMAPIStatus *_swig_self;
    if ((_swig_self=GetI(self))==NULL) return NULL;
    HRESULT  _result;
    ULONG uiparam = 0, flags = 0;
    char *entryID;
    int cbEntryID;
    // @pyparm int|uiparam||
    // @pyparm string|entryID||A blob
    // @pyparm int|flags||
    if (!PyArg_ParseTuple(args, "lz#l:FlushQueues",
                          &uiparam,
                          &entryID,
                          &cbEntryID,
                          &flags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    _result = (HRESULT )_swig_self->FlushQueues(uiparam, cbEntryID, (ENTRYID *)entryID, flags);
    Py_END_ALLOW_THREADS
    if (FAILED(_result)) {
        return OleSetOleError(_result);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

%}
%native(FlushQueues) FlushQueues;
