/* File : PyIMsgServiceAdmin.i */
 
%module IMsgServiceAdmin // An COM interface to MAPI's IMsgServiceAdmin interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMsgServiceAdmin.h"

PyIMsgServiceAdmin::PyIMsgServiceAdmin(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIMsgServiceAdmin::~PyIMsgServiceAdmin()
{
}

/*static*/ IMsgServiceAdmin *PyIMsgServiceAdmin::GetI(PyObject *self)
{
	return (IMsgServiceAdmin *)PyIUnknown::GetI(self);
}


%}

%native(GetLastError) GetLastError;
%{
// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
PyObject *PyIMsgServiceAdmin::GetLastError(PyObject *self, PyObject *args)
{
	HRESULT hr, hRes;
	ULONG flags = 0;
	MAPIERROR *me = NULL;
	
	IMsgServiceAdmin *_swig_self;
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

%native(CreateMsgService) CreateMsgService;
%{
// @pyswig |CreateMsgService|Creates a message service.
PyObject *PyIMsgServiceAdmin::CreateMsgService(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	HRESULT hRes;
	PyObject *obService;
	LPTSTR lpszService = NULL;
	PyObject *obDisplayName;
	LPTSTR lpszDisplayName = NULL;
	ULONG ulUIParam = 0;
	ULONG ulFlags = 0;
	
	IMsgServiceAdmin *_swig_self;
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
	hRes = _swig_self->CreateMsgService(lpszService, lpszDisplayName, ulUIParam, ulFlags);
	Py_END_ALLOW_THREADS
	
	if (FAILED(hRes))
		result = OleSetOleError(hRes);
	else
		result = Py_BuildValue("");

done:
	PyWinObject_FreeString(lpszService);
	PyWinObject_FreeString(lpszDisplayName);
	
	return result;
}
%}

%{
// @pyswig |ConfigureMsgService|Reconfigures a message service. 
PyObject *PyIMsgServiceAdmin::ConfigureMsgService(PyObject *self, PyObject *args)
{
	unsigned long ulUIParam;
	unsigned long ulFlags;
	unsigned long cValues;
	PyObject *obs;
	PyObject *obIID;
	IMsgServiceAdmin *_swig_self;
	if ((_swig_self=GetI(self))==NULL) return NULL;
	if (!PyArg_ParseTuple(args, "OiiO:ConfigureMsgService", 
	                     &obIID, // @pyparm <o PyIID>|iid||The unique identifier for the message service to configure.
						 &ulUIParam, // @pyparm int|ulUIParam||Handle of the parent window for the configuration property sheet.
						 &ulFlags, // @pyparm int|ulFlags||Bitmask of flags that controls the display of the property sheet.
						 &obs)) // @pyparm [values, ...]|[SPropValue, ...]||Property values describing the properties to display in the property sheet.  Should not be None if the service is to be configured without a message service.
		return NULL;
	CLSID iid;
	if (!PyWinObject_AsIID(obIID, &iid))
		return NULL;
	MAPIUID *pMAPIUID = (MAPIUID *)&iid;
	SPropValue *pv;
	if (obs==Py_None) {
		pv = NULL;
		cValues = 0;
	} else {
		if (!PyMAPIObject_AsSPropValueArray(obs, &pv, &cValues))
			return NULL;
	}
	HRESULT _result;
	Py_BEGIN_ALLOW_THREADS
	_result = _swig_self->ConfigureMsgService(pMAPIUID, ulUIParam, ulFlags, cValues, pv );
	Py_END_ALLOW_THREADS
    MAPIFreeBuffer(pv);
	if (FAILED(_result))
		return OleSetOleError(_result);
	 Py_INCREF(Py_None);
	 return Py_None;
}
%}
%native (ConfigureMsgService) ConfigureMsgService;

// @pyswig <o PyIMAPITable>|GetMsgServiceTable|Retrieves a table of services.
HRESULT GetMsgServiceTable(
	unsigned long ulFlags, // @pyparm int|flags||
	IMAPITable **OUTPUT
);

// @pyswig <o PyIMAPITable>|GetProviderTable|Retrieves a table of service providers.
HRESULT GetProviderTable(
	unsigned long ulFlags, // @pyparm int|flags||
	IMAPITable **OUTPUT
);

// @pyswig |DeleteMsgService|Deletes the specified service
HRESULT DeleteMsgService(
	MAPIUID *INPUT // @pyparm <o PyIID>|uuid||The ID of the service
);

// @pyswig |RenameMsgService|Renames the specified service
// @comm This is deprecated, and there is no replacement referenced to use instead.
HRESULT RenameMsgService(
	MAPIUID *INPUT, // @pyparm <o PyIID>|uuid||The ID of the service
	unsigned long flags, // @pyparm int|flags||
	TCHAR *newName // @pyparm string|newName||The new name for the service.
);
 
// @pyswig <o PyIProfSect>|OpenProfileSection|Opens a profile section
HRESULT OpenProfileSection(
	MAPIUID *INPUT, // @pyparm <o PyIID>|uuid||The ID of the service
	IID *INPUT_NULLOK, // @pyparm <o PyIID>|iid||The IID of the resulting object, or None for the default
	unsigned long ulFlags, // @pyparm int|flags||
	IProfSect **OUTPUT
);

// @pyswig <o PyIProfSect>|AdminProviders|Returns an object providing access
// to a provider administration object.
HRESULT AdminProviders(
	MAPIUID *INPUT, // @pyparm <o PyIID>|uuid||The ID of the service
	unsigned long ulFlags, // @pyparm int|flags||
	IProviderAdmin  **OUTPUT
);
