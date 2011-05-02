/* File : adsi.i */

/* 
   This is designed to be an interface to the ADSI API

*/

%module adsi // A COM interface to ADSI

// @ comm Generally you will not use this module (win32com.adsi.adsi) 
// directly, but use win32com.adsi - this top-level interface does
// smarter integration with Python IDispatch support, so offers a more
// convenient technique.

//%{
//#define UNICODE
//%}


%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{
#include "objsel.h"
#include "PyIADs.h"
#include "PyIEnumVARIANT.h"
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"
#include "PyIDirectoryObject.h"
#include "PyIDirectorySearch.h"
#include "PyIADsContainer.h"
#include "PyIADsUser.h"
#include "PyIADsDeleteOps.h"
#include "PyIDsObjectPicker.h"
#include "ADSIID.h"

extern PyTypeObject PyDSOP_SCOPE_INIT_INFOsType;
extern PyObject* PyIADs_getattro(PyObject *ob, PyObject *obname);
extern PyObject* PyIADsUser_getattro(PyObject *ob, PyObject *obname);

%}

%{
static int AddIID(PyObject *dict, const char *key, REFGUID guid)
{
	PyObject *obiid = PyWinObject_FromIID(guid);
	if (!obiid) return 1;
	int rc = PyDict_SetItemString(dict, (char*)key, obiid);
	Py_DECREF(obiid);
	return rc;
}

#define ADD_CONSTANT(tok) AddConstant(dict, #tok, tok)
#define ADD_IID(tok) AddIID(d, #tok, tok)

// @pyswig com_object|ADsOpenObject|Binds to an ADSI object using explicit username and password credentials.
static PyObject *PyADsOpenObject(PyObject *self, PyObject *args)
{
	HRESULT hr;
	IUnknown *pOb = NULL;
	PyObject *obPath, *obUserName, *obPassword, *obiid = NULL;
	PyObject *ret = NULL;
	long lres = 0;
	if (!PyArg_ParseTuple(args, "OOO|lO:ADsOpenObject",
			&obPath, // @pyparm unicode|path||
			&obUserName, // @pyparm unicode|username||
			&obPassword,// @pyparm unicode|password||
			&lres, // @pyparm int|reserved|0|
			&obiid)) // @pyparm <o PyIID>|iid|IID_IDispatch|The requested interface
		return NULL;
	IID iid = IID_IDispatch;
	WCHAR *path = NULL, *userName = NULL, *password = NULL;
	if (obiid != NULL && !PyWinObject_AsIID(obiid, &iid))
		goto done;
	if (!PyWinObject_AsWCHAR(obPath, &path, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obUserName, &userName, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obPassword, &password, TRUE))
		goto done;
	Py_BEGIN_ALLOW_THREADS;
	hr = ADsOpenObject(path, userName, password, (DWORD)lres, iid, (void **)&pOb);
	Py_END_ALLOW_THREADS;
	if (FAILED(hr))
		ret = OleSetADSIError(hr, NULL, IID_NULL);
	else
		ret = PyCom_PyObjectFromIUnknown(pOb, iid, FALSE);
done:
	PyWinObject_FreeWCHAR(path);
	PyWinObject_FreeWCHAR(userName);
	PyWinObject_FreeWCHAR(password);
	return ret;
}
%}
%native (ADsGetObject) PyADsGetObject;

%{
// @pyswig com_object|ADsGetObject|Binds to an object given its path and a specified interface identifier (IID).
static PyObject *PyADsGetObject(PyObject *self, PyObject *args)
{
	HRESULT hr;
	IUnknown *pOb = NULL;
	PyObject *obPath, *obiid = NULL;
	PyObject *ret = NULL;
	if (!PyArg_ParseTuple(args, "O|O:ADsGetObject",
			&obPath, // @pyparm unicode|path||
			&obiid)) // @pyparm <o PyIID>|iid|IID_IDispatch|The requested interface
		return NULL;
	IID iid = IID_IDispatch;
	WCHAR *path = NULL;
	if (obiid != NULL && !PyWinObject_AsIID(obiid, &iid))
		goto done;
	if (!PyWinObject_AsWCHAR(obPath, &path, FALSE))
		goto done;
	Py_BEGIN_ALLOW_THREADS;
	hr = ADsGetObject(path, iid, (void **)&pOb);
	Py_END_ALLOW_THREADS;
	if (FAILED(hr))
		ret = OleSetADSIError(hr, NULL, IID_NULL);
	else
		ret = PyCom_PyObjectFromIUnknown(pOb, iid, FALSE);
done:
	PyWinObject_FreeWCHAR(path);
	return ret;
}
%}
%native (ADsOpenObject) PyADsOpenObject;

%{

class PyIADsEnumVARIANT : public PyIEnumVARIANT {
	PyIADsEnumVARIANT(IUnknown *pdisp) : PyIEnumVARIANT(pdisp) {;}
	virtual ~PyIADsEnumVARIANT() {
		if (m_obj) {
			ADsFreeEnumerator((IEnumVARIANT *)m_obj);
			m_obj = NULL; // so base dtor doesnt "Release"
		}
	}
};

// @pyswig <o PyIEnumerator>|ADsBuildEnumerator|Builds an enumerator object for the specified ADSI container object.
static PyObject *PyADsBuildEnumerator(PyObject *self, PyObject *args)
{
	HRESULT hr;
	IUnknown *pOb = NULL;
	PyObject *obCont;
	PyObject *ret = NULL;
	if (!PyArg_ParseTuple(args, "O:ADsBuildEnumerator",
			&obCont)) // @pyparm <o PyIADsContainer>|container||
		return NULL;
	IADsContainer *pC = NULL;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obCont, IID_IADsContainer, (void **)&pC, FALSE))
		return NULL;
	IEnumVARIANT *pev;
	Py_BEGIN_ALLOW_THREADS;
	hr = ADsBuildEnumerator(pC, &pev);
	pC->Release();
	Py_END_ALLOW_THREADS;
	if (FAILED(hr))
		ret = OleSetADSIError(hr, NULL, IID_NULL);
	else
		ret = PyIADsEnumVARIANT::PyObConstruct(pev);
	return ret;
}
%}
%native (ADsBuildEnumerator) PyADsBuildEnumerator;

%{
// @pyswig <o PyIEnumerator>|ADsEnumerateNext|
static PyObject *PyADsEnumerateNext(PyObject *self, PyObject *args)
{
	long celt = 1;
	PyObject *obEnum;
	// @pyparm <o PyIEnumVARIANT>|enum||The enumerator.
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "O|l:ADsEnumerateNext", &obEnum, &celt) )
		return NULL;

	IEnumVARIANT *pev;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obEnum, IID_IEnumVARIANT, (LPVOID *)&pev, FALSE))
		return NULL;

	VARIANT *rgVar = new VARIANT[celt];
	if ( rgVar == NULL ) {
		pev->Release();
		PyErr_SetString(PyExc_MemoryError, "allocating result VARIANTs");
		return NULL;
	}
	int i;
	for ( i = celt; i--; )
		VariantInit(&rgVar[i]);

	ULONG celtFetched;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pev->Next(celt, rgVar, &celtFetched);
	pev->Release();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
	{
		delete [] rgVar;
		return PyCom_BuildPyException(hr);
	}

	PyObject *result = PyTuple_New(celtFetched);
	if ( result != NULL )
	{
		for ( i = celtFetched; i--; )
		{
			PyObject *ob = PyCom_PyObjectFromVariant(&rgVar[i]);
			if ( ob == NULL )
			{
				Py_DECREF(result);
				result = NULL;
				break;
			}
			PyTuple_SET_ITEM(result, i, ob);
		}
	}

	for ( i = celtFetched; i--; )
		VariantClear(&rgVar[i]);
	delete [] rgVar;

	return result;
	// @rdesc The result is a tuple of Python objects converted from Variants,
	// one for each element returned.  Note that if zero elements are returned, it is not considered
	// an error condition - an empty tuple is simply returned.
}
%}
%native (ADsEnumerateNext) PyADsEnumerateNext;

%{
// @pyswig (int, unicode, unicode)|ADsGetLastError|
static PyObject *PyADsGetLastError(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":ADsGetLastError") )
		return NULL;
	WCHAR szErrorBuf[MAX_PATH] = {0};
	WCHAR szNameBuf[MAX_PATH] = {0};
	DWORD dwErrCode = 0;
	ADsGetLastError( &dwErrCode,
			 szErrorBuf,
			 MAX_PATH-1,
			 szNameBuf,
			 MAX_PATH-1);
	return Py_BuildValue("iuu", dwErrCode, szErrorBuf, szNameBuf);
}
%}
%native (ADsGetLastError) PyADsGetLastError;

%{
// @pyswig <o PyDS_SELECTION_LIST>|StringAsDS_SELECTION_LIST|Unpacks a string (generally fetched via <om PyIDataObject.GetData>) into a <o PyDS_SELECTION_LIST> list.
// @pyparm str|buf||The raw buffer
extern PyObject *PyStringAsDS_SELECTION_LIST(PyObject *self, PyObject *args);
%}
%native (StringAsDS_SELECTION_LIST) PyStringAsDS_SELECTION_LIST;

%init %{
	PyDict_SetItemString(d, "error", PyWinExc_COMError);

	// @pyswig <o DSOP_SCOPE_INIT_INFOs>|DSOP_SCOPE_INIT_INFOs|The type object for <o PyDSOP_SCOPE_INIT_INFOs> objects.
	// @pyparm int|size||The number of <o PyDSOP_SCOPE_INIT_INFO> objects to create in the array.
	if (PyType_Ready(&PyDSOP_SCOPE_INIT_INFOsType) != 0)
		return MODINIT_ERROR_RETURN;
	PyDict_SetItemString(d, "DSOP_SCOPE_INIT_INFOs", (PyObject *)&PyDSOP_SCOPE_INIT_INFOsType);

	AddIID(d, "LIBID_ADs", LIBID_ADs);

	ADD_IID(IID_IADsNamespaces);
	ADD_IID(IID_IADsDomain);
	ADD_IID(IID_IADsComputerOperations);
	ADD_IID(IID_IADsComputer);
	ADD_IID(IID_IADsGroup);
	ADD_IID(IID_IADsMembers);
	ADD_IID(IID_IADsPrintQueue);
	ADD_IID(IID_IADsPrintQueueOperations);
	ADD_IID(IID_IADsPrintJobOperations);
	ADD_IID(IID_IADsPrintJob);
	ADD_IID(IID_IADsCollection);
	ADD_IID(IID_IADsServiceOperations);
	ADD_IID(IID_IADsService);
	ADD_IID(IID_IADsFileServiceOperations);
	ADD_IID(IID_IADsFileService);
	ADD_IID(IID_IADsResource);
	ADD_IID(IID_IADsSession);
	ADD_IID(IID_IADsFileShare);
//	ADD_IID(IID_IADsSchema);
	ADD_IID(IID_IADsClass);
	ADD_IID(IID_IADsProperty);
	ADD_IID(IID_IADsSyntax);
	ADD_IID(IID_IADsLocality);
	ADD_IID(IID_IADsO);
	ADD_IID(IID_IADsOU);
	ADD_IID(IID_IADsOpenDSObject);
	ADD_IID(IID_IADsSearch);
	ADD_IID(IID_IADsPropertyList);
//	ADD_IID(IID_IDSObject);
//	ADD_IID(IID_IDSSearch);
//	ADD_IID(IID_IDSAttrMgmt);

	ADD_IID(CLSID_AccessControlEntry);
	ADD_IID(CLSID_AccessControlList);
	ADD_IID(CLSID_SecurityDescriptor);
    ADD_IID(CLSID_DsObjectPicker);
//	ADD_IID(IID_IDirectoryAttrMgmt);

	AddIID(d, "CLSID_ADsDSOObject", CLSID_ADsDSOObject);
	AddIID(d, "DBGUID_LDAPDialect", DBGUID_LDAPDialect);
	AddIID(d, "DBPROPSET_ADSISEARCH", DBPROPSET_ADSISEARCH);

	if ( PyCom_RegisterClientType(&PyIADs::type, &IID_IADs) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IADs);
	// Patch up getattro for all types deriving from IADs
	PyIADs::type.tp_getattro = PyIADs_getattro;
	PyIADsUser::type.tp_getattro = PyIADsUser_getattro;


	if ( PyCom_RegisterClientType(&PyIDirectoryObject::type, &IID_IDirectoryObject) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IDirectoryObject);

	if ( PyCom_RegisterClientType(&PyIDirectorySearch::type, &IID_IDirectorySearch) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IDirectorySearch);

	if ( PyCom_RegisterClientType(&PyIADsUser::type, &IID_IADsUser) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IADsUser);

	if ( PyCom_RegisterClientType(&PyIADsContainer::type, &IID_IADsContainer) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IADsContainer);

	if ( PyCom_RegisterClientType(&PyIDsObjectPicker::type, &IID_IDsObjectPicker) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IDsObjectPicker);

	if ( PyCom_RegisterClientType(&PyIADsDeleteOps::type, &IID_IADsDeleteOps) != 0 ) return MODINIT_ERROR_RETURN;
	ADD_IID(IID_IADsDeleteOps);
%}
