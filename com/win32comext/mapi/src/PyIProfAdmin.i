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

%native(CreateProfile) CreateProfile;
%{
// @pyswig |CreateProfile|Creates a new profile.
PyObject *PyIProfAdmin::CreateProfile(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obProfileName;
	LPTSTR lpszProfileName = NULL;
	PyObject *obPassword;
	LPTSTR lpszPassword = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "OO|ll",
		&obProfileName, // @pyparm string|oldProfileName||The name of the new profile. 
		&obPassword, // @pyparm string|Password|| Must be None
		&ulUIParam, // @pyparm int|uiParam|0|A handle of the parent window for any dialog boxes or windows that this method displays.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obProfileName, &lpszProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obPassword, &lpszPassword, ulFlags & MAPI_UNICODE, TRUE))
		goto done;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->CreateProfile(lpszProfileName, lpszPassword, ulUIParam, ulFlags);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue("");

done:
	PyWinObject_FreeString(lpszProfileName);
	PyWinObject_FreeString(lpszPassword);
	
	return result;
}
%}

HRESULT GetProfileTable(
	unsigned long ulFlags,
	IMAPITable **OUTPUT
);

%native(DeleteProfile) DeleteProfile;
%{
// @pyswig |DeleteProfile|Deletes a profile.
PyObject *PyIProfAdmin::DeleteProfile(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *obProfileName;
	LPTSTR lpszProfileName;
	ULONG ulFlags = 0;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "O|l",
		&obProfileName, // @pyparm string|oldProfileName||The name of the profile to be deleted. 
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obProfileName, &lpszProfileName, ulFlags & MAPI_UNICODE, FALSE))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->DeleteProfile(lpszProfileName, ulFlags);
	Py_END_ALLOW_THREADS
	
	PyWinObject_FreeString(lpszProfileName);
	if (FAILED(hRes))
		return OleSetOleError(hRes);
	
	return Py_BuildValue("");
}
%}

// Deprecated MAPI function.
%apply INPUT_NULLOK2 {INPUT_NULLOK2};
HRESULT ChangeProfilePassword(
	TCHAR *INPUT, // lpszProfileName
	TCHAR *INPUT_NULLOK, // lpszOldPassword
	TCHAR *INPUT_NULLOK, // lpszNewPassword
	unsigned long ulFlags
);

%native(CopyProfile) CopyProfile;
%{
// @pyswig |CopyProfile|Copies a profile.
PyObject *PyIProfAdmin::CopyProfile(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obOldProfileName;
	LPTSTR lpszOldProfileName = NULL;
	PyObject *obOldPassword;
	LPTSTR lpszOldPassword = NULL;
	PyObject *obNewProfileName;
	LPTSTR lpszNewProfileName = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "OOO|ll",
		&obOldProfileName, // @pyparm string|oldProfileName||The name of the profile to copy. 
		&obOldPassword, // @pyparm string|Password|| Must be None
		&obNewProfileName, // @pyparm string|newProfileName||The new name of the copied profile.
		&ulUIParam, // @pyparm int|uiParam|0|A handle of the parent window for any dialog boxes or windows that this method displays.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obOldProfileName, &lpszOldProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obOldPassword, &lpszOldPassword, ulFlags & MAPI_UNICODE, TRUE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obNewProfileName, &lpszNewProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->CopyProfile(lpszOldProfileName, lpszOldPassword, lpszNewProfileName, ulUIParam, ulFlags);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue("");

done:
	PyWinObject_FreeString(lpszOldProfileName);
	PyWinObject_FreeString(lpszOldPassword);
	PyWinObject_FreeString(lpszNewProfileName);
	
	return result;
}
%}

%native(RenameProfile) RenameProfile;
%{
// @pyswig |RenameProfile|Assigns a new name to a profile.
PyObject *PyIProfAdmin::RenameProfile(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obOldProfileName;
	LPTSTR lpszOldProfileName = NULL;
	PyObject *obOldPassword;
	LPTSTR lpszOldPassword = NULL;
	PyObject *obNewProfileName;
	LPTSTR lpszNewProfileName = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "OOO|ll",
		&obOldProfileName, // @pyparm string|oldProfileName||The current name of the profile to rename. 
		&obOldPassword, // @pyparm string|Password|| Must be None
		&obNewProfileName, // @pyparm string|newProfileName||The new name of the profile to rename.
		&ulUIParam, // @pyparm int|uiParam|0|A handle of the parent window for any dialog boxes or windows that this method displays.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obOldProfileName, &lpszOldProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obOldPassword, &lpszOldPassword, ulFlags & MAPI_UNICODE, TRUE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obNewProfileName, &lpszNewProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->RenameProfile(lpszOldProfileName, lpszOldPassword, lpszNewProfileName, ulUIParam, ulFlags);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue("");

done:
	PyWinObject_FreeString(lpszOldProfileName);
	PyWinObject_FreeString(lpszOldPassword);
	PyWinObject_FreeString(lpszNewProfileName);
	
	return result;
}
%}

%native(SetDefaultProfile) SetDefaultProfile;
%{
// @pyswig |SetDefaultProfile|Sets or clears a client's default profile.
PyObject *PyIProfAdmin::SetDefaultProfile(PyObject *self, PyObject *args)
{
	HRESULT hRes;
	PyObject *obProfileName;
	LPTSTR lpszProfileName;
	ULONG ulFlags = 0;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "O|l",
		&obProfileName, // @pyparm string|profileName||The name of the profile that will become the default, or None. Setting profileName to None indicates that SetDefaultProfile should remove the existing default profile, leaving the client without a default.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obProfileName, &lpszProfileName, ulFlags & MAPI_UNICODE, TRUE))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->SetDefaultProfile(lpszProfileName, ulFlags);
	Py_END_ALLOW_THREADS
	
	PyWinObject_FreeString(lpszProfileName);
	
	if (FAILED(hRes))
		return OleSetOleError(hRes);
	
	return Py_BuildValue("");
}
%}

%native(AdminServices) AdminServices;
%{
// @pyswig <o PyIProfAdmin>|AdminServices|Provides access to a message service administration object for making changes to the message services in a profile.
PyObject *PyIProfAdmin::AdminServices(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obProfileName;
	LPTSTR lpszProfileName = NULL;
	PyObject *obPassword = Py_None;
	LPTSTR lpszPassword = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	LPSERVICEADMIN lpServiceAdmin = NULL;
	
	IProfAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	
	if (!PyArg_ParseTuple(args, "O|Oll",
		&obProfileName, // @pyparm string|profileName||The name of the profile to be modified. 
		&obPassword, // @pyparm string|Password|None|
		&ulUIParam, // @pyparm int|uiParam|0|A handle of the parent window for any dialog boxes or windows that this method displays.
		&ulFlags)) // @pyparm int|flags|0|
		return NULL;

	if (!PyWinObject_AsMAPIStr(obProfileName, &lpszProfileName, ulFlags & MAPI_UNICODE, FALSE))
		goto done;
	if (!PyWinObject_AsMAPIStr(obPassword, &lpszPassword, ulFlags & MAPI_UNICODE, TRUE))
		goto done;
	
	Py_BEGIN_ALLOW_THREADS
	hRes = _swig_self->AdminServices(lpszProfileName, lpszPassword, ulUIParam, ulFlags, &lpServiceAdmin);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		MAKE_OUTPUT_INTERFACE(&lpServiceAdmin, result, IID_IMsgServiceAdmin);

done:
	PyWinObject_FreeString(lpszProfileName);
	PyWinObject_FreeString(lpszPassword);
	
	return result;
}
%}
