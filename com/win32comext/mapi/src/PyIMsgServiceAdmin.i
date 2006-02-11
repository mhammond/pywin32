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


HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig |CreateMsgService|Creates a message service. 
HRESULT CreateMsgService( 
	TCHAR *INPUT, // @pyparm string|serviceName||The name of the service.
	TCHAR *INPUT_NULLOK, // @pyparm string|displayName||Display name of the service, or None
	unsigned long ulUIParam, // @pyparm int|ulUIParam||Handle of the parent window for the configuration property sheet.
	unsigned long ulFlags // @pyparm int|ulFlags||Bitmask of flags that controls the display of the property sheet.
);

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
