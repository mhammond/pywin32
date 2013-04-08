// PyComHelpers.cpp
//
// Most of the PyCom_ helpers.

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyWinObjects.h" // Until this is converted to the new API

extern PyObject *g_obPyCom_MapIIDToType;
extern PyObject *g_obPyCom_MapServerIIDToGateway;

// String conversions

// Convert a Python object to a BSTR - allow embedded NULLs, None, etc.
BOOL PyCom_BstrFromPyObject(PyObject *stringObject, BSTR *pResult, BOOL bNoneOK /*= FALSE*/)
{
	return PyWinObject_AsBstr(stringObject, pResult, bNoneOK);
}

// MakeBstrToObj - convert a BSTR into a Python string.
//
// ONLY USE THIS FOR TRUE BSTR's - Use the fn below for OLECHAR *'s.
// NOTE - does not use standard macros, so NULLs get through!
PyObject *MakeBstrToObj(const BSTR bstr)
{
	return PyWinObject_FromBstr(bstr, FALSE);
}

// Size info is available (eg, a fn returns a string and also fills in a size variable)
PyObject *MakeOLECHARToObj(const OLECHAR * str, int numChars)
{
	return PyWinObject_FromOLECHAR(str, numChars);
}

// No size info avail.
PyObject *MakeOLECHARToObj(const OLECHAR * str)
{
	return PyWinObject_FromOLECHAR(str);
}

// Currency conversions.
// Should probably place this in module dict so it can be DECREF'ed on finalization
// Also may get borked by a reload of the decimal module
static PyObject *Decimal_class = NULL;

PyObject *get_Decimal_class(void)
{
	// Try to import compiled _decimal module introduced in Python 3.3
	TmpPyObject decimal_module = PyImport_ImportModule("_decimal");

	// Look for python implemented module introduced in Python 2.4
	if (decimal_module==NULL){
		PyErr_Clear();
		decimal_module = PyImport_ImportModule("decimal");
		}
	if (decimal_module==NULL)
		return NULL;
	return PyObject_GetAttrString(decimal_module, "Decimal");
}

PyObject *PyObject_FromCurrency(CURRENCY &cy)
{
#if (PY_VERSION_HEX < 0x03000000)
	static char *divname = "__div__";
#else
	static char *divname = "__truediv__";
#endif
	if (Decimal_class == NULL){
		Decimal_class = get_Decimal_class();
		if (Decimal_class == NULL)
			return NULL;
		}

	TmpPyObject unscaled_result = PyObject_CallFunction(Decimal_class, "L", cy.int64);
	if (unscaled_result == NULL)
		return NULL;
	return PyObject_CallMethod(unscaled_result, divname, "l", 10000);
}

PYCOM_EXPORT BOOL PyObject_AsCurrency(PyObject *ob, CURRENCY *pcy)
{
	if (Decimal_class == NULL){
		Decimal_class = get_Decimal_class();
		if (Decimal_class == NULL)
			return FALSE;
		}

	int right_type = PyObject_IsInstance(ob, Decimal_class);
	if (right_type == -1)
		return FALSE;
	else if (right_type == 0){
		PyErr_Format(PyExc_TypeError,
			"Currency object must be a Decimal instance (got %s).",ob->ob_type->tp_name);
		return FALSE;
		}

	TmpPyObject scaled = PyObject_CallMethod(ob, "__mul__", "l", 10000);
	if (scaled == NULL)
		return FALSE;
	TmpPyObject longval = PyNumber_Long(scaled);
	if (longval == NULL)
		return FALSE;
	pcy->int64 = PyLong_AsLongLong(longval);
	if (pcy->int64 == -1 && PyErr_Occurred())
		return FALSE;
	return TRUE;
}

// If PyCom_PyObjectFromIUnknown is called with bAddRef==FALSE, the 
// caller is asking us to take ownership of the COM reference.  If we
// fail to create a Python object, we must release the reference.
#define POFIU_RELEASE_ON_FAILURE \
	if (!bAddRef) PYCOM_RELEASE(punk)


// Interface conversions
PyObject *PyCom_PyObjectFromIUnknown(IUnknown *punk, REFIID riid, BOOL bAddRef /* = FALSE */)
{
	// Quick exit.
	if (punk==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	// Look up the map, and create the object.
	PyObject *obiid = PyWinObject_FromIID(riid);
	if (!obiid){
		POFIU_RELEASE_ON_FAILURE
		return NULL;
	}
	PyObject *createType = PyDict_GetItem(g_obPyCom_MapIIDToType, obiid);
	Py_DECREF(obiid);
	if (createType==NULL) {
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "There is no interface object registered that supports this IID");
		POFIU_RELEASE_ON_FAILURE
		return NULL;
	}

	// ensure the object we fetched is actually one of our interface types
	if ( !PyComTypeObject::is_interface_type(createType) ) {
		PyErr_SetString(PyExc_TypeError, "The Python IID map is invalid - the value is not an interface type object");
		POFIU_RELEASE_ON_FAILURE
		return NULL;
	}

	// we can now safely cast the thing to a PyComTypeObject and use it
	PyComTypeObject *myCreateType = (PyComTypeObject *)createType;
	if (myCreateType->ctor==NULL) {
		PyErr_SetString(PyExc_TypeError, "The type does not declare a PyCom constructor");
		POFIU_RELEASE_ON_FAILURE
		return NULL;
	}

	PyIUnknown *ret = (*myCreateType->ctor)(punk);
#ifdef _DEBUG_LIFETIMES
	PyCom_LogF("Object %s created at 0x%0xld, IUnknown at 0x%0xld",
		 myCreateType->tp_name, ret, ret->m_obj);
#endif
	if (ret && bAddRef) punk->AddRef();
	return ret;
}


BOOL PyCom_InterfaceFromPyInstanceOrObject(PyObject *ob, REFIID iid, LPVOID *ppv, BOOL bNoneOK /* = TRUE */)
{
	if (ob == Py_None){
		*ppv = NULL;
		if (bNoneOK)
			return TRUE;
		PyErr_SetString(PyExc_TypeError, "None is not a valid interface object in this context");
		return FALSE;
		}

	if (PyObject_IsInstance(ob, (PyObject *)&PyIUnknown::type))
		return PyCom_InterfaceFromPyObject(ob, iid, ppv, bNoneOK );

	ob = PyObject_GetAttrString(ob, "_oleobj_");
	if (ob==NULL) {
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "The Python instance can not be converted to a COM object");
		return FALSE;
		}
	BOOL rc = PyCom_InterfaceFromPyObject(ob, iid, ppv, bNoneOK );
	Py_DECREF(ob);
	return rc;
}

BOOL PyCom_InterfaceFromPyObject(PyObject *ob, REFIID iid, LPVOID *ppv, BOOL bNoneOK /* = TRUE */)
{
	if ( ob == NULL )
	{
		// don't overwrite an error message
		if ( !PyErr_Occurred() )
			PyErr_SetString(PyExc_TypeError, "The Python object is NULL and no error occurred");
		return FALSE;
	}
	if ( ob == Py_None )
	{
		if ( bNoneOK )
		{
			*ppv = NULL;
			return TRUE;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "None is not a valid interface object in this context");
			return FALSE;
		}
	}

	if ( !PyIBase::is_object(ob, &PyIUnknown::type) )
	{
		PyErr_Format(PyExc_ValueError,
                             "argument is not a COM object (got type=%s)",
                             ob->ob_type->tp_name);
		return FALSE;
	}
	IUnknown *punk = PyIUnknown::GetI(ob);
	if ( !punk )
		return FALSE;	/* exception was set by GetI() */
	/* note: we don't explicitly hold a reference to punk */
	HRESULT hr;
	Py_BEGIN_ALLOW_THREADS
	hr = punk->QueryInterface(iid, ppv);
	Py_END_ALLOW_THREADS
	if ( FAILED(hr) )
	{
		PyCom_BuildPyException(hr);
		return FALSE;
	}
	/* note: the QI added a ref for the return value */

	return TRUE;
}

HRESULT PyCom_MakeRegisteredGatewayObject(REFIID iid, PyObject *instance, PyGatewayBase *base, void **ppv)
{
	if ( g_obPyCom_MapServerIIDToGateway == NULL )
		return E_NOINTERFACE;

	HRESULT hr = E_FAIL;

	{
		CEnterLeavePython celp;
		PyObject *keyObject = PyWinObject_FromIID(iid);
		if ( keyObject )
		{
			PyObject *valueObject = PyDict_GetItem(g_obPyCom_MapServerIIDToGateway,keyObject);
			Py_DECREF(keyObject);
			if ( valueObject )
			{
				pfnPyGatewayConstructor ctor = (pfnPyGatewayConstructor)PyLong_AsVoidPtr(valueObject);
				// ctor takes reference count to instance.
				hr = (*ctor)(instance, base, ppv, iid);
			}
			else
			{
				hr = E_NOINTERFACE;
			}
		}
	}

	return hr;
}

/* Converts a STATSTG structure to a Python tuple

  NOTE - DOES NOT free the string - this is the callers responsibility
  (see the STATSTG doco for details)
*/
// @object STATSTG|A tuple representing a STATSTG structure
PyObject *PyCom_PyObjectFromSTATSTG(STATSTG *pStat)
{
	if (pStat==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
 	PyObject *obSize = NULL;
	PyObject *obmtime = NULL;
	PyObject *obctime = NULL;
	PyObject *obatime = NULL;
	PyObject *obCLSID = NULL;
	obSize = PyWinObject_FromULARGE_INTEGER(pStat->cbSize);
	obmtime = PyWinObject_FromFILETIME(pStat->mtime);
	obctime = PyWinObject_FromFILETIME(pStat->ctime);
	obatime = PyWinObject_FromFILETIME(pStat->atime);
	obCLSID = PyWinObject_FromIID(pStat->clsid);
	
	PyObject *obName = MakeOLECHARToObj(pStat->pwcsName);
//	char *szName = pStat->pwcsName==NULL ? NULL : OLE2T(pStat->pwcsName);
	PyObject *result = Py_BuildValue("OiOOOOiiOii", 
		           obName, // @tupleitem 0|string|name|The name of the storage object
				   pStat->type, // @tupleitem 1|int|type|Indicates the type of storage object. This is one of the values from the storagecon.STGTY_* values.
				   obSize, // @tupleitem 2|<o ULARGE_INTEGER>|size|Specifies the size in bytes of the stream or byte array.
				   obmtime, // @tupleitem 3|<o PyTime>|modificationTime|Indicates the last modification time for this storage, stream, or byte array.
				   obctime,	 // @tupleitem 4|<o PyTime>|creationTime|Indicates the creation time for this storage, stream, or byte array.
				   obatime,	 // @tupleitem 5|<o PyTime>|accessTime|Indicates the last access time for this storage, stream or byte array.
				   pStat->grfMode, // @tupleitem 6|int|mode|Indicates the access mode specified when the object was opened. This member is only valid in calls to Stat methods.
				   pStat->grfLocksSupported,// @tupleitem 7|int|locksSupported|Indicates the types of region locking supported by the stream or byte array. See the storagecon.LOCKTYPES_* constants for the values available. This member is not used for storage objects.
				   obCLSID, // @tupleitem 8|<o PyIID>|clsid|Indicates the class identifier for the storage object; set to CLSID_NULL for new storage objects. This member is not used for streams or byte arrays.
				   pStat->grfStateBits, // @tupleitem 9|int|stateBits|Indicates the current state bits of the storage object, that is, the value most recently set by the <om PyIStorage.SetStateBits> method. This member is not valid for streams or byte arrays.
				   pStat->reserved);  // @tupleitem 10|int|storageFormat|Indicates the format of the storage object. This is one of the values from the STGFMT_* constants.  In some Win32 API documentation, this member is known as 'reserved'
	Py_XDECREF(obName);
	Py_XDECREF(obSize);
	Py_XDECREF(obmtime);
	Py_XDECREF(obctime);
	Py_XDECREF(obatime);
	Py_XDECREF(obCLSID);
	return result;
}

BOOL PyCom_PyObjectAsSTATSTG(PyObject *ob, STATSTG *pStat, DWORD flags /* = 0 */)
{
	char *szName;
	PyObject *obSize;
	PyObject *obmtime, *obctime, *obatime;
	PyObject *obCLSID;
	if (!PyArg_ParseTuple(ob, "ziOOOOiiOii",
		                &szName,
						&pStat->type,
						&obSize,
						&obmtime,
						&obctime,
						&obatime,
						&pStat->grfMode,
						&pStat->grfLocksSupported,
						&obCLSID,
						&pStat->grfStateBits,
						&pStat->reserved))
		return NULL;
	pStat->pwcsName = NULL; // XXX - need to fix this
	// When fixed, should honour the STATFLAG_NONAME
	if (!PyWinObject_AsULARGE_INTEGER(obSize, &pStat->cbSize))
		return FALSE;
	if (!PyWinTime_Check(obmtime) || !PyWinTime_Check(obctime) || !PyWinTime_Check(obatime)) {
		PyErr_SetString(PyExc_TypeError, "The time entries in a STATSTG tuple must be PyTime objects");
		return FALSE;
	}
	if (!PyWinObject_AsFILETIME(obmtime, &pStat->mtime))
		return FALSE;
	if (!PyWinObject_AsFILETIME(obctime, &pStat->ctime))
		return FALSE;
	if (!PyWinObject_AsFILETIME(obatime, &pStat->atime))
		return FALSE;
	if (!PyWinObject_AsIID(obCLSID, &pStat->clsid))
		return FALSE;
	return TRUE;
}

#ifndef NO_PYCOM_STGOPTIONS
BOOL PyCom_PyObjectAsSTGOPTIONS(PyObject *obstgoptions, STGOPTIONS **ppstgoptions)
{
	static char *stgmembers[]={"Version","reserved","SectorSize","TemplateFile",0};
	char *explain_format="STGOPTIONS must be a dictionary containing "\
			"{Version:int,reserved:0,SectorSize:int,TemplateFile:unicode}";
	PyObject *dummy_tuple=NULL;
	BOOL ret;

	if ((obstgoptions==Py_None)||(obstgoptions==NULL)){
		*ppstgoptions=NULL;
		return TRUE;
		}
	if (!PyDict_Check(obstgoptions)){
		PyErr_SetString(PyExc_TypeError,explain_format);
		return FALSE;
		}

	*ppstgoptions=new(STGOPTIONS);
	if (*ppstgoptions==NULL){
		PyErr_SetString(PyExc_MemoryError,"PyObjectAsSTGOPTIONS: Out of memory");
		return FALSE;
		}
	(*ppstgoptions)->usVersion=2;
	(*ppstgoptions)->reserved=0;
	(*ppstgoptions)->ulSectorSize=512;
	(*ppstgoptions)->pwcsTemplateFile=NULL;
	dummy_tuple=PyTuple_New(0);
	ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obstgoptions, "|lllu", stgmembers, 
		&(*ppstgoptions)->usVersion,
		&(*ppstgoptions)->reserved, 
		&(*ppstgoptions)->ulSectorSize,
		&(*ppstgoptions)->pwcsTemplateFile);
	Py_DECREF(dummy_tuple);
	if (!ret){
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError,explain_format);
		delete(*ppstgoptions);
		*ppstgoptions=NULL;
		}
	return ret;
}
#endif // NO_PYCOM_STGOPTIONS

PyObject *PyCom_PyObjectFromSTATPROPSETSTG(STATPROPSETSTG *pStg) 
{
	if (pStg==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *obfmtid = PyWinObject_FromIID(pStg->fmtid);
	PyObject *obclsid = PyWinObject_FromIID(pStg->clsid);
	PyObject *obmtime = PyWinObject_FromFILETIME(pStg->mtime);
	PyObject *obctime = PyWinObject_FromFILETIME(pStg->ctime);
	PyObject *obatime = PyWinObject_FromFILETIME(pStg->atime);
	PyObject *ret = Py_BuildValue("OOiOOO", obfmtid, obclsid, pStg->grfFlags, obmtime, obctime, obatime);
	Py_XDECREF(obfmtid);
	Py_XDECREF(obclsid);
	Py_XDECREF(obmtime);
	Py_XDECREF(obctime);
	Py_XDECREF(obatime);
	return ret;
}

BOOL PyCom_PyObjectAsSTATPROPSETSTG(PyObject *obstat, STATPROPSETSTG *pstat) 
{
	return PyArg_ParseTuple(obstat, "O&O&kO&O&O&:STATPROPSETSTG",
			PyWinObject_AsIID, &pstat->fmtid,
			PyWinObject_AsIID, &pstat->clsid,
			&pstat->grfFlags,
			PyWinObject_AsFILETIME, &pstat->mtime,
			PyWinObject_AsFILETIME, &pstat->ctime,
			PyWinObject_AsFILETIME, &pstat->atime);
}
