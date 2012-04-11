#include "stdafx.h"
#include "PythonCOM.h"
#include "PyRecord.h"

// @doc



// The owner of the record buffer - many records may point here!
class PyRecordBuffer
{
public:
	PyRecordBuffer(int size)
	{
		data = PyMem_Malloc(size);
		if (data==NULL)
			PyErr_NoMemory();
		ref = 0;
	}
	~PyRecordBuffer()
	{
		if (data) PyMem_Free(data);
	}
	void AddRef() {
		ref++;
	}
	void Release() {
		if (--ref==0) {
			delete this;
			return;
		}
	}
	void *data;
	long ref;
};

BOOL PyRecord_Check(PyObject *ob) {return ((ob)->ob_type == &PyRecord::Type);}

BOOL PyObject_AsVARIANTRecordInfo(PyObject *ob, VARIANT *pv)
{
	if (!PyRecord_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "Only com_record objects can be used as records");
		return NULL;
	}
	PyRecord *pyrec = (PyRecord *)ob;
	HRESULT hr = pyrec->pri->RecordCreateCopy(pyrec->pdata, &V_RECORD(pv));
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
		return FALSE;
	}
	V_RECORDINFO(pv) = pyrec->pri;
	pyrec->pri->AddRef();
	return TRUE;
}

PyObject *PyObject_FromSAFEARRAYRecordInfo(SAFEARRAY *psa)
{
	PyObject *ret = NULL, *ret_tuple = NULL;
	IRecordInfo *info = NULL;
	BYTE *source_data = NULL, *this_dest_data = NULL;
	long lbound, ubound, nelems, i;
	ULONG cb_elem;
	PyRecordBuffer *owner = NULL;
	HRESULT hr = SafeArrayGetRecordInfo(psa, &info);
	if (FAILED(hr)) goto exit;
	hr = SafeArrayAccessData(psa, (void **)&source_data);
	if (FAILED(hr)) goto exit;
	// Allocate a new chunk of memory
	hr = SafeArrayGetUBound(psa, 1, &ubound);
	if (FAILED(hr)) goto exit;
	hr = SafeArrayGetLBound(psa, 1, &lbound);
	if (FAILED(hr)) goto exit;
	nelems = ubound-lbound;
	hr = info->GetSize(&cb_elem);
	if (FAILED(hr)) goto exit;
	owner = new PyRecordBuffer(nelems * cb_elem);
	if (PyErr_Occurred()) goto exit;
	owner->AddRef(); // unref'd at end - for successful failure cleanup
	ret_tuple = PyTuple_New(nelems);
	if (ret_tuple==NULL) goto exit;
	this_dest_data = (BYTE *)owner->data;
	for (i=0;i<nelems;i++) {
		hr = info->RecordInit(this_dest_data);
		if (FAILED(hr)) goto exit;

		hr = info->RecordCopy(source_data, this_dest_data);
		if (FAILED(hr)) goto exit;
		PyTuple_SET_ITEM(ret_tuple, i, new PyRecord(info, this_dest_data, owner));
		this_dest_data += cb_elem;
		source_data += cb_elem;
	}
	ret = ret_tuple;
	Py_INCREF(ret); // for decref on cleanup.
exit:
	if (FAILED(hr)) {
		if (info)
			PyCom_BuildPyException(hr, info, IID_IRecordInfo);
		else
			PyCom_BuildPyException(hr);
		Py_XDECREF(ret);
		ret = NULL;
	}
	if (owner != NULL) owner->Release();
	Py_XDECREF(ret_tuple);
	if (info) info->Release();
	if (source_data!=NULL) SafeArrayUnaccessData(psa);
	return ret;
}
// Creates a new Record by TAKING A COPY of the passed record.
PyObject *PyObject_FromRecordInfo(IRecordInfo *ri, void *data, ULONG cbData)
{
	if ((data != NULL && cbData==0) || (data==NULL && cbData != 0))
		return PyErr_Format(PyExc_RuntimeError, "Both or neither data and size must be given");
	ULONG cb;
	HRESULT hr = ri->GetSize(&cb);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
	if (cbData != 0 && cbData != cb)
		return PyErr_Format(PyExc_ValueError, "Expecting a string of %d bytes (got %d)", cb, cbData);
	PyRecordBuffer *owner = new PyRecordBuffer(cb);
	if (PyErr_Occurred()) { // must be mem error!
		delete owner;
		return NULL;
	}
	hr = ri->RecordInit(owner->data);
	if (FAILED(hr)) {
		delete owner;
		return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
	}
	hr = data==NULL ? 0 : ri->RecordCopy(data, owner->data);
	if (FAILED(hr)) {
		delete owner;
		return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
	}
	return new PyRecord(ri, owner->data, owner);
}

// @pymethod <o PyRecord>|pythoncom|GetRecordFromGuids|Creates a new record object from the given GUIDs
PyObject *pythoncom_GetRecordFromGuids(PyObject *self, PyObject *args)
{
	void *data = NULL;
	PyObject *obGuid, *obInfoGuid, *obdata=Py_None;
	int major, minor, lcid;
	int cb = 0;
	if (!PyArg_ParseTuple(args, "OiiiO|O:GetRecordFromGuids", 
		&obGuid, // @pyparm <o PyIID>|iid||The GUID of the type library
		&major, // @pyparm int|verMajor||The major version number of the type lib.
		&minor, // @pyparm int|verMinor||The minor version number of the type lib.
		&lcid, // @pyparm int|lcid||The LCID of the type lib.
		&obInfoGuid, // @pyparm <o PyIID>|infoIID||The GUID of the record info in the library
		&obdata)) // @pyparm string or buffer|data|None|The raw data to initialize the record with.
		return NULL;
	if (!PyWinObject_AsReadBuffer(obdata, &data, &cb, TRUE))
		return NULL;
	GUID guid, infoGuid;
	if (!PyWinObject_AsIID(obGuid, &guid))
		return NULL;
	if (!PyWinObject_AsIID(obInfoGuid, &infoGuid))
		return NULL;
	IRecordInfo *i = NULL;
	HRESULT hr = GetRecordInfoFromGuids(guid, major, minor, lcid, infoGuid, &i);
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	PyObject *ret = PyObject_FromRecordInfo(i, data, cb);
	i->Release();
	return ret;
}

// @pymethod <o PyRecord>|pythoncom|GetRecordFromTypeInfo|Creates a new record object from a <o PyITypeInfo> interface
// @comm This function will fail if the specified type info does not have a guid defined
PyObject *pythoncom_GetRecordFromTypeInfo(PyObject *self, PyObject *args)
{
	PyObject *obtypeinfo, *ret;
	ITypeInfo *pITI=NULL;
	IRecordInfo *pIRI=NULL;
	HRESULT hr;
	if (!PyArg_ParseTuple(args, "O:GetRecordFromTypeInfo", 
		&obtypeinfo)) // @pyparm <o PyITypeInfo>|TypeInfo||The type information to be converted into a PyRecord object
		return NULL;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obtypeinfo, IID_ITypeInfo, (void **)&pITI, FALSE))
		return NULL;

	hr=GetRecordInfoFromTypeInfo(pITI, &pIRI);
	if (FAILED(hr))
		ret=PyCom_BuildPyException(hr);
	else
		ret = PyObject_FromRecordInfo(pIRI, NULL, 0);
	pITI->Release();
	if (pIRI!=NULL)
		pIRI->Release();
	return ret;
}

PyRecord::PyRecord(IRecordInfo *ri, PVOID data, PyRecordBuffer *owner)
{
	ob_type = &PyRecord::Type;
	_Py_NewReference(this);
	ri->AddRef();
	pri = ri;
	pdata = data;
	this->owner = owner;
	owner->AddRef();
};

PyRecord::~PyRecord()
{
	owner->Release();
	pri->Release();
}


PyTypeObject PyRecord::Type =
{
	PYWIN_OBJECT_HEAD
	"com_record",
	sizeof(PyRecord),
	0,
	PyRecord::tp_dealloc,	/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	&PyRecord::tp_repr,		/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyRecord::getattro,		/* tp_getattro */
	PyRecord::setattro,		/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	PyRecord::tp_richcompare,			/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	PyRecord::methods,		/* tp_methods */
	0,						/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

static PyObject *PyRecord_reduce(PyObject *self, PyObject *args)
{
	PyObject *ret = NULL;
	PyRecord *pyrec = (PyRecord *)self;
	PyObject *obModule = NULL, *obModDict = NULL, *obFunc = NULL;
	ITypeInfo *pti = NULL;
	TYPEATTR *pta = NULL;
	ULONG cb;
	HRESULT hr;
	GUID structguid;
	if (!PyArg_ParseTuple(args, ":reduce"))
		return NULL;
	hr = pyrec->pri->GetTypeInfo(&pti);
	if (FAILED(hr)||pti==NULL) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	hr = pti->GetTypeAttr(&pta);
	if (FAILED(hr)||pta==NULL) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	hr = pyrec->pri->GetGuid(&structguid);
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	hr = pyrec->pri->GetSize(&cb);
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr);
		goto done;
	}
	obModule = PyImport_ImportModule("pythoncom");
	if (obModule)
		obModDict = PyModule_GetDict(obModule); // no ref added!
	if (obModDict)
		obFunc = PyDict_GetItemString(obModDict, "GetRecordFromGuids"); // no ref added!
	if (!obFunc) {
		PyErr_Clear();
		PyErr_SetString(PyExc_RuntimeError, "pythoncom.GetRecordFromGuids() can't be located!");
		goto done;
	}
	ret = Py_BuildValue("O(NHHiNN)",
						obFunc,
						PyWinObject_FromIID(pta->guid),
						pta->wMajorVerNum,
						pta->wMinorVerNum,
						pta->lcid,
						PyWinObject_FromIID(structguid),
						PyString_FromStringAndSize((char *)pyrec->pdata, cb));

done:
	if (pta&& pti)
		pti->ReleaseTypeAttr(pta);
	if (pti) pti->Release();
	Py_XDECREF(obModule);
	// obModDict and obFunc have no new reference.
	return ret;
}

// The object itself.
// Any method names should be "__blah__", as they override
// structure names!
struct PyMethodDef PyRecord::methods[] = {
	{"__reduce__",      PyRecord_reduce, 1}, // This allows the copy module to work!
	{NULL}
};

static BSTR *_GetFieldNames(IRecordInfo *pri, ULONG *pnum)
{
	ULONG num_names;
	HRESULT hr = pri->GetFieldNames(&num_names, NULL);
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr, pri, IID_IRecordInfo);
		return NULL;
	}
	BSTR *strings = new BSTR [num_names];
	if (strings==NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	for (ULONG i = 0; i < num_names; i++)
		strings[i] = NULL;

	hr = pri->GetFieldNames(&num_names, strings);
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr, pri, IID_IRecordInfo);
		return NULL;
	}
	*pnum = num_names;
	return strings;
}
static void _FreeFieldNames(BSTR *strings, ULONG num_names)
{
	for (ULONG i = 0; i < num_names; i++)
		SysFreeString(strings[i]);
	delete[] strings;
}

#if (PY_VERSION_HEX < 0x03000000)
#define PyWinCoreString_ConcatAndDel PyString_ConcatAndDel
#define PyWinCoreString_Concat PyString_Concat
#else
// Unicode versions of '_Concat' etc have different sigs.  Make them the
// same here...
void PyWinCoreString_Concat(register PyObject **pv, register PyObject *w)
{
	if (!w) { // hrm - string version doesn't do this, but I saw PyObject_Repr() return NULL...
		Py_XDECREF(*pv);
		*pv = NULL;
		return;
	}
	PyObject *tmp = PyUnicode_Concat(*pv, w);
	Py_DECREF(*pv);
	*pv = tmp;
}

void PyWinCoreString_ConcatAndDel(register PyObject **pv, register PyObject *w)
{
	PyWinCoreString_Concat(pv, w);
	Py_XDECREF(w);
}

#endif

PyObject *PyRecord::tp_repr(PyObject *self)
{
	ULONG i;
	PyRecord *pyrec = (PyRecord *)self;
	ULONG num_names;
	BSTR *strings = _GetFieldNames(pyrec->pri, &num_names);
	if (strings==NULL)
		return NULL;
	PyObject *obrepr=NULL, *obattrname;
	BOOL bsuccess=FALSE;
	PyObject *comma = PyWinCoreString_FromString(_T(", "));
	PyObject *equals = PyWinCoreString_FromString(_T("="));
	PyObject *closing_paren=PyWinCoreString_FromString(_T(")"));
	obrepr = PyWinCoreString_FromString(_T("com_struct("));

	if (obrepr==NULL || comma==NULL || equals==NULL || closing_paren==NULL)
		goto done;
	for (i = 0; i < num_names && obrepr != NULL; i++) {
		obattrname=PyWinCoreString_FromString(strings[i]);
		if (obattrname==NULL)
			goto done;
		// must exit on error via loop_error from here...
		PyObject *sub_object = NULL;
		if (i > 0){
			PyWinCoreString_Concat(&obrepr, comma);
			if (!obrepr)
				goto loop_error;
			}
		PyWinCoreString_Concat(&obrepr, obattrname);
		if (!obrepr)
			goto loop_error;
		PyWinCoreString_Concat(&obrepr, equals);
		if (!obrepr)
			goto loop_error;
		sub_object = PyRecord::getattro(self, obattrname);
		if (!sub_object)
			goto loop_error;
		PyWinCoreString_ConcatAndDel(&obrepr, PyObject_Repr(sub_object));
		Py_DECREF(sub_object);
		Py_DECREF(obattrname);
		continue;

		// loop error handler.
		loop_error:
		Py_DECREF(obattrname);
		goto done;
		}
	PyWinCoreString_Concat(&obrepr, closing_paren);
	bsuccess=TRUE;
done:
	Py_XDECREF(comma);
	Py_XDECREF(equals);
	Py_XDECREF(closing_paren);
	if (strings)
		_FreeFieldNames(strings, num_names);
	if (!bsuccess){
		Py_XDECREF(obrepr);
		obrepr=NULL;
		}
	return obrepr;
}

PyObject *PyRecord::getattro(PyObject *self, PyObject *obname)
{
	PyObject *res;
	PyRecord *pyrec = (PyRecord *)self;
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	if (strcmp(name, "__members__")==0) {
		ULONG cnames = 0;
		HRESULT hr = pyrec->pri->GetFieldNames(&cnames, NULL);
		if (FAILED(hr))
			return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
		BSTR *strs = (BSTR *)malloc(sizeof(BSTR) * cnames);
		if (strs==NULL)
			return PyErr_NoMemory();
		hr = pyrec->pri->GetFieldNames(&cnames, strs);
		if (FAILED(hr)) {
			free(strs);
			return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
		}
		res = PyList_New(cnames);
		for (ULONG i=0;i<cnames && res != NULL;i++) {
			PyObject *item = PyWinCoreString_FromString(strs[i]);
			SysFreeString(strs[i]);
			if (item==NULL) {
				Py_DECREF(res);
				res = NULL;
			} else
				PyList_SET_ITEM(res, i, item); // ref count swallowed.
		}
		free(strs);
		return res;
	}

	res = PyObject_GenericGetAttr(self, obname);
	if (res != NULL)
		return res;

	PyErr_Clear();
	WCHAR *wname;
	if (!PyWinObject_AsWCHAR(obname, &wname))
		return NULL;

	VARIANT vret;
	VariantInit(&vret);
	void *sub_data = NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pyrec->pri->GetFieldNoCopy(pyrec->pdata, wname, &vret, &sub_data);
	PyWinObject_FreeWCHAR(wname);
	PY_INTERFACE_POSTCALL;

	if (FAILED(hr)) {
		if (hr == TYPE_E_FIELDNOTFOUND){
			// This is slightly suspect - throwing a unicode
			// object for an AttributeError in py2k - but this
			// is the value we asked COM for, so it makes sense...
			// (and PyErr_Format doesn't handle unicode in py2x)
			PyErr_SetObject(PyExc_AttributeError, obname);
			return NULL;
			}
		return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
	}

	// Short-circuit sub-structs and arrays here, so we dont allocate a new chunk
	// of memory and copy it - we need sub-structs to persist.
	if (V_VT(&vret)==(VT_BYREF | VT_RECORD))
		return new PyRecord(V_RECORDINFO(&vret), V_RECORD(&vret), pyrec->owner);
	else if (V_VT(&vret)==(VT_BYREF | VT_ARRAY | VT_RECORD)) {
		SAFEARRAY *psa = *V_ARRAYREF(&vret);
		int d = SafeArrayGetDim(psa);
		if (sub_data==NULL)
			return PyErr_Format(PyExc_RuntimeError, "Did not get a buffer for the array!");
		if (SafeArrayGetDim(psa) != 1)
			return PyErr_Format(PyExc_TypeError, "Only support single dimensional arrays of records");
		IRecordInfo *sub = NULL;
		long ubound, lbound, nelems;
		int i;
		BYTE *this_data;
		PyObject *ret_tuple = NULL;
		ULONG element_size = 0;
		hr = SafeArrayGetUBound(psa, 1, &ubound);
		if (FAILED(hr)) goto array_end;
		hr = SafeArrayGetLBound(psa, 1, &lbound);
		if (FAILED(hr)) goto array_end;
		hr = SafeArrayGetRecordInfo(psa, &sub);
		if (FAILED(hr)) goto array_end;
		hr = sub->GetSize(&element_size);
		if (FAILED(hr)) goto array_end;
		nelems = ubound-lbound;
		ret_tuple = PyTuple_New(nelems);
		if (ret_tuple==NULL) goto array_end;
		this_data = (BYTE *)sub_data;
		for (i=0;i<nelems;i++) {
			PyTuple_SET_ITEM(ret_tuple, i, new PyRecord(sub, this_data, pyrec->owner));
			this_data += element_size;
		}
array_end:
		if (sub)
			sub->Release();
		if (FAILED(hr)) 
			return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
		return ret_tuple;
	}

	// This default conversion we use is a little slow (but it will do!)
	// For arrays, the pparray->pvData member is *not* set, since the actual data
	// pointer from the record is returned in sub_data, so set it here.
	if (V_ISARRAY(&vret) && V_ISBYREF(&vret))
		(*V_ARRAYREF(&vret))->pvData = sub_data;
	PyObject *ret = PyCom_PyObjectFromVariant(&vret);

//	VariantClear(&vret);
	return ret;
}

int PyRecord::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	VARIANT val;
	VariantInit(&val);
	PyRecord *pyrec = (PyRecord *)self;

	if (!PyCom_VariantFromPyObject(v, &val))
		return -1;

	WCHAR *wname;
	if (!PyWinObject_AsWCHAR(obname, &wname, FALSE))
		return -1;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pyrec->pri->PutField(INVOKE_PROPERTYPUT, pyrec->pdata, wname, &val);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeWCHAR(wname);
	VariantClear(&val);
	if (FAILED(hr)) {
		PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
		return -1;
	}
	return 0;
}

PyObject *PyRecord::tp_richcompare(PyObject *self, PyObject *other, int op)
{
	PyObject *ret = NULL;
	if (op != Py_EQ && op != Py_NE) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	int success = op == Py_EQ ? TRUE : FALSE;

	if (self->ob_type != other->ob_type) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	PyRecord *pyself = (PyRecord *)self;
	PyRecord *pyother = (PyRecord *)other;
	if (!pyself->pri->IsMatchingType(pyother->pri)) {
		// Not matching types, so can't compare.
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	// Need to do a recursive compare, as some elements may be pointers
	// (eg, strings, objects)
	ULONG num_names;
	BSTR *strings = _GetFieldNames(pyself->pri, &num_names);
	if (strings==NULL) return NULL;
	for (ULONG i=0;i<num_names;i++) {
		ret = 0;
		PyObject *obattrname;
		obattrname=PyWinCoreString_FromString(strings[i]);
		if (obattrname==NULL)
			goto done;
		// There appear to be several problems here.  This will leave an exception hanging
		//	if an attribute is not found, and should probably return False if other does not
		//	have an attr that self does ???
		//	MarkH: but is that possible in practice?  For structures,
		//	an attribute must be found, and the set must be identical
		//	(we have already checked the 'type' is the same above)
		//	(defense against COM errors etc would be nice though :)
		PyObject *self_sub = PyRecord::getattro(self, obattrname);
		if (!self_sub) {
			Py_DECREF(obattrname);
			goto done;
		}
		PyObject *other_sub = PyRecord::getattro(other, obattrname);
		if (!other_sub){
			Py_DECREF(obattrname);
			Py_DECREF(self_sub);
			goto done;
		}
		int c = PyObject_RichCompareBool(self_sub, other_sub, op);
		Py_DECREF(self_sub);
		Py_DECREF(other_sub);
		Py_DECREF(obattrname);
		if (c == -1)
			goto done;
		if (c != success) {
			ret = PyBool_FromLong(c);
			goto done;
		}
	}
	ret = PyBool_FromLong(success);
done:
	_FreeFieldNames(strings, num_names);
	return ret;
}

void PyRecord::tp_dealloc(PyObject *ob)
{
	delete (PyRecord *)ob;
}
