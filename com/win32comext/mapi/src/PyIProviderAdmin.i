/* File : PyIProviderAdmin.i */ 
   
%module IProviderAdmin // A COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"
  
%{

#include "PyIProviderAdmin.h"

PyIProviderAdmin::PyIProviderAdmin(IUnknown *pDisp) :
    PyIUnknown(pDisp)
{
    ob_type = &type;
}

PyIProviderAdmin::~PyIProviderAdmin()
{
}

/*static*/ IProviderAdmin *PyIProviderAdmin::GetI(PyObject *self)
{
    return (IProviderAdmin *)PyIUnknown::GetI(self);
}

%}

%native(GetLastError) GetLastError;
%{
// @pyswig <o MAPIERROR>|GetLastError|Returns the last error code for the object.
PyObject *PyIProviderAdmin::GetLastError(PyObject *self, PyObject *args)
{
	HRESULT hr, hRes;
	ULONG flags = 0;
	MAPIERROR *me = NULL;
	
	IProviderAdmin *_swig_self;
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

// @pyswig <o PyIMAPITable>|GetProviderTable|Retrieves a table of service providers.
HRESULT GetProviderTable(
	unsigned long ulFlags, // @pyparm int|flags||
	IMAPITable **OUTPUT
);

// @pyswig |DeleteProvider|Deletes the service provider from message service
HRESULT DeleteProvider(
    MAPIUID *INPUT // @pyparm <o PyIID>|uuid||The ID of the provider
);

// @pyswig <o PyIProfSect>|OpenProfileSection|
HRESULT OpenProfileSection(
    MAPIUID *INPUT, // @pyparm <o PyIID>|uuid||The ID of the service
    IID *INPUT_NULLOK, // @pyparm <o PyIID>|iid||The IID of the resulting object, or None for the default
    unsigned long ulFlags, // @pyparm int|flags||
    IProfSect **OUTPUT
);


%{

// as defined in MSDN, exerything [in] except last arg is [out]
//  HRESULT CreateProvider(LPTSTR lpszProvider,ULONG cValues,LPSPropValue lpProps,ULONG ulUIParam,ULONG ulFlags,MAPIUID FAR * lpUID);    
// @pyswig <o PyIID>|CreateProvider|Add a service provider to a message service.
PyObject *PyIProviderAdmin::CreateProvider(PyObject *self, PyObject *args) 
{
    HRESULT hr;IProviderAdmin *_swig_self;
    //Handle the 5 input variables
    TCHAR * lpszProvider;unsigned long cValues;PyObject *py_props;unsigned long ulUIParam;unsigned long ulFlags;
    //Parse the 5 input variables from Python input
    if(!PyArg_ParseTuple(args,"slOll",&lpszProvider,&cValues,&py_props,&ulUIParam,&ulFlags)) { return NULL; }

    //handle spropvalue structure
    SPropValue *pPropValue;ULONG len;  
    if (py_props==Py_None) {
        pPropValue = NULL;
        cValues = 0;
    } else {
        if (!PyMAPIObject_AsSPropValueArray(py_props, &pPropValue, &len))
            return NULL;
    }

    //Setup output variable
    CLSID iid;
    MAPIUID *pMAPIUID = (MAPIUID *)&iid;

    if ((_swig_self=GetI(self))==NULL) return NULL;
    Py_BEGIN_ALLOW_THREADS
    hr = (HRESULT )_swig_self->CreateProvider(lpszProvider,cValues,pPropValue,ulUIParam,ulFlags,pMAPIUID);
    Py_END_ALLOW_THREADS
    MAPIFreeBuffer(pPropValue);
    if (FAILED(hr)) { return NULL;}
    GUID *pTemp = (GUID *)&iid;
    PyObject *obiid = PyWinObject_FromIID(*pTemp);
    if (!obiid) return NULL;
    return obiid;
}

%}
%native (CreateProvider) CreateProvider;


