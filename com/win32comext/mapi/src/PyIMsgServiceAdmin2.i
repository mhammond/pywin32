/* File : PyIMsgServiceAdmin2.i */

%module IMsgServiceAdmin2

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "mapiaux.h"
#include "PyIMsgServiceAdmin.h"
#include "PyIMsgServiceAdmin2.h"

PyIMsgServiceAdmin2::PyIMsgServiceAdmin2(IUnknown *pDisp) :
	PyIMsgServiceAdmin(pDisp)
{
	ob_type = &type;
}

PyIMsgServiceAdmin2::~PyIMsgServiceAdmin2()
{
}

/*static*/ IMsgServiceAdmin2 *PyIMsgServiceAdmin2::GetI(PyObject *self)
{
	return (IMsgServiceAdmin2 *)PyIUnknown::GetI(self);
}


%}

%native(CreateMsgServiceEx) CreateMsgServiceEx;
%{
// @pyswig <o PyIID>|CreateMsgServiceEx|Creates a message service and returns the newly added service UID.
PyObject *PyIMsgServiceAdmin2::CreateMsgServiceEx(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obService;
	LPTSTR lpszService = NULL;
	PyObject *obDisplayName;
	LPTSTR lpszDisplayName = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	MAPIUID uidService = {0};

	IMsgServiceAdmin2 *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;

	if (!PyArg_ParseTuple(args, "OO|ll",
		&obService, // @pyparm string|serviceName||The name of the service.
		&obDisplayName, // @pyparm string|displayName||Display name of the service, or None
		&ulUIParam, // @pyparm int|uiParam|0|A handle of the parent window for any dialog boxes or windows that this method displays.
		&ulFlags)) // @pyparm int|flags||A bitmask of flags that controls how the message service is installed.
		return NULL;

	if (!PyWinObject_AsMAPIStr(obService, &lpszService, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obDisplayName, &lpszDisplayName, ulFlags & MAPI_UNICODE, TRUE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
		hRes = _swig_self->CreateMsgServiceEx(lpszService, lpszDisplayName, ulUIParam, ulFlags, &uidService);
	Py_END_ALLOW_THREADS

	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = PyWinObject_FromIID(reinterpret_cast<GUID &>(uidService));

done:
	PyWinObject_FreeString(lpszService);
	PyWinObject_FreeString(lpszDisplayName);

	return result;
}
%}
