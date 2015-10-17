/* File : PyIProfAdmin.i */

%module IProfAdmin // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIProfAdmin.h"

PyIProfAdmin::PyIProfAdmin(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIProfAdmin::~PyIProfAdmin()
{
}

/*static*/ IProfAdmin *PyIProfAdmin::GetI(PyObject *self)
{
	return (IProfAdmin *)PyIUnknown::GetI(self);
}


%}

%native(GetLastError) GetLastError;
%{
// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
PyObject *PyIProfAdmin::GetLastError(PyObject *self, PyObject *args)
{
	HRESULT hr, hRes;
	ULONG flags = 0;
	MAPIERROR *me = NULL;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
    if(!PyArg_ParseTuple(args,"l|l:GetLastError",
		&hr, // @pyparm int|hr||Contains the error code generated in the previous method call.
		&flags)) // @pyparm int|flags||Indicates for format for the output.
        return NULL;
		
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->GetLastError(hr, flags, &me);
	Py_END_ALLOW_THREADS

	if (FAILED(hRes))
		return OleSetOleError(hRes);
	
	if (me == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyObject_FromMAPIERROR(me, flags & MAPI_UNICODE, TRUE);
}
%}

HRESULT CreateProfile( 
	TCHAR *INPUT, // LPTSTR lpszProfileName, 
	TCHAR *INPUT_NULLOK, // LPTSTR lpszPassword,
	unsigned long ulUIParam,
	unsigned long ulFlags
);

HRESULT GetProfileTable(
	unsigned long ulFlags,
	IMAPITable **OUTPUT
);

HRESULT DeleteProfile(
	TCHAR *INPUT, // lpszProfileName
	unsigned long ulFlags
);

%apply INPUT_NULLOK2 {INPUT_NULLOK2};
HRESULT ChangeProfilePassword(
	TCHAR *INPUT, // lpszProfileName
	TCHAR *INPUT_NULLOK, // lpszOldPassword
	TCHAR *INPUT_NULLOK, // lpszNewPassword
	unsigned long ulFlags
);


%apply INPUT {INPUT2};

HRESULT CopyProfile(
	TCHAR *INPUT, // lpszOldProfileName
	TCHAR *INPUT_NULLOK, // lpszOldPassword
	TCHAR *INPUT, // lpszNewProfileName
	unsigned long ulUIParam,
	unsigned long ulFlags
);

HRESULT RenameProfile( 
	TCHAR *INPUT, // lpszOldProfileName,
	TCHAR *INPUT_NULLOK, // lpszOldPassword,
	TCHAR *INPUT2, // lpszNewProfileName,
	unsigned long ulUIParam,
	unsigned long ulFlags
);

HRESULT SetDefaultProfile( 
	TCHAR *INPUT_NULLOK, // lpszProfileName
	unsigned long ulFlags
);

HRESULT AdminServices(
	TCHAR *INPUT, // lpszProfileName,
	TCHAR *INPUT_NULLOK, // lpszPassword,
	unsigned long ulUIParam,
	unsigned long ulFlags,
	IMsgServiceAdmin **OUTPUT
);
