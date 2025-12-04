#include <new>
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyRecord.h"

extern PyObject *g_obPyCom_MapRecordGUIDToRecordClass;

// @doc

// The owner of the record buffer - many records may point here!
class PyRecordBuffer {
   public:
    PyRecordBuffer(int size)
    {
        data = PyMem_Malloc(size);
        if (data == NULL)
            PyErr_NoMemory();
        ref = 0;
    }
    ~PyRecordBuffer()
    {
        if (data)
            PyMem_Free(data);
    }
    void AddRef() { ref++; }
    void Release()
    {
        if (--ref == 0) {
            delete this;
            return;
        }
    }
    void *data;
    long ref;
};

BOOL PyRecord_Check(PyObject *ob) { return PyObject_IsInstance(ob, (PyObject *)&PyRecord::Type); }

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
    if (FAILED(hr))
        goto exit;
    hr = SafeArrayAccessData(psa, (void **)&source_data);
    if (FAILED(hr))
        goto exit;
    // Allocate a new chunk of memory
    hr = SafeArrayGetUBound(psa, 1, &ubound);
    if (FAILED(hr))
        goto exit;
    hr = SafeArrayGetLBound(psa, 1, &lbound);
    if (FAILED(hr))
        goto exit;
    nelems = ubound - lbound + 1;
    hr = info->GetSize(&cb_elem);
    if (FAILED(hr))
        goto exit;
    owner = new PyRecordBuffer(nelems * cb_elem);
    if (PyErr_Occurred())
        goto exit;
    owner->AddRef();  // unref'd at end - for successful failure cleanup
    ret_tuple = PyTuple_New(nelems);
    if (ret_tuple == NULL)
        goto exit;
    this_dest_data = (BYTE *)owner->data;
    for (i = 0; i < nelems; i++) {
        hr = info->RecordInit(this_dest_data);
        if (FAILED(hr))
            goto exit;

        hr = info->RecordCopy(source_data, this_dest_data);
        if (FAILED(hr))
            goto exit;
        PyRecord *rec = PyRecord::new_record(info, this_dest_data, owner);
        if (rec == NULL)
            goto exit;
        PyTuple_SET_ITEM(ret_tuple, i, rec);
        this_dest_data += cb_elem;
        source_data += cb_elem;
    }
    ret = ret_tuple;
    Py_INCREF(ret);  // for decref on cleanup.
exit:
    if (FAILED(hr)) {
        if (info)
            PyCom_BuildPyException(hr, info, IID_IRecordInfo);
        else
            PyCom_BuildPyException(hr);
        Py_XDECREF(ret);
        ret = NULL;
    }
    if (owner != NULL)
        owner->Release();
    Py_XDECREF(ret_tuple);
    if (info)
        info->Release();
    if (source_data != NULL)
        SafeArrayUnaccessData(psa);
    return ret;
}
// Creates a new Record by TAKING A COPY of the passed record.
// The optinal 'type' parameter is used by the 'tp_new' slot method to
// specify the subclass and must match the corresponding 'IRecordInfo' object
// passed in by the 'ri' parameter.
PyObject *PyObject_FromRecordInfo(IRecordInfo *ri, void *data, ULONG cbData, PyTypeObject *type = NULL)
{
    if ((data != NULL && cbData == 0) || (data == NULL && cbData != 0))
        return PyErr_Format(PyExc_RuntimeError, "Both or neither data and size must be given");
    ULONG cb;
    HRESULT hr = ri->GetSize(&cb);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
    if (cbData != 0 && cbData != cb)
        return PyErr_Format(PyExc_ValueError, "Expecting a string of %d bytes (got %d)", cb, cbData);
    PyRecordBuffer *owner = new PyRecordBuffer(cb);
    if (PyErr_Occurred()) {  // must be mem error!
        delete owner;
        return NULL;
    }
    hr = ri->RecordInit(owner->data);
    if (FAILED(hr)) {
        delete owner;
        return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
    }
    hr = data == NULL ? 0 : ri->RecordCopy(data, owner->data);
    if (FAILED(hr)) {
        delete owner;
        return PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
    }
    return PyRecord::new_record(ri, owner->data, owner, type);
}

// @pymethod <o PyRecord>|pythoncom|GetRecordFromGuids|Creates a new record object from the given GUIDs
PyObject *pythoncom_GetRecordFromGuids(PyObject *self, PyObject *args)
{
    PyObject *obGuid, *obInfoGuid, *obdata = Py_None;
    int major, minor, lcid;
    if (!PyArg_ParseTuple(args, "OiiiO|O:GetRecordFromGuids",
                          &obGuid,      // @pyparm <o PyIID>|iid||The GUID of the type library
                          &major,       // @pyparm int|verMajor||The major version number of the type lib.
                          &minor,       // @pyparm int|verMinor||The minor version number of the type lib.
                          &lcid,        // @pyparm int|lcid||The LCID of the type lib.
                          &obInfoGuid,  // @pyparm <o PyIID>|infoIID||The GUID of the record info in the library
                          &obdata))  // @pyparm string or buffer|data|None|The raw data to initialize the record with.
        return NULL;
    PyWinBufferView pybuf(obdata, false, true);  // None ok
    if (!pybuf.ok())
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
    PyObject *ret = PyObject_FromRecordInfo(i, pybuf.ptr(), pybuf.len());
    i->Release();
    return ret;
}

// @pymethod <o PyRecord>|pythoncom|GetRecordFromTypeInfo|Creates a new record object from a <o PyITypeInfo> interface
// @comm This function will fail if the specified type info does not have a guid defined
PyObject *pythoncom_GetRecordFromTypeInfo(PyObject *self, PyObject *args)
{
    PyObject *obtypeinfo, *ret;
    ITypeInfo *pITI = NULL;
    IRecordInfo *pIRI = NULL;
    HRESULT hr;
    if (!PyArg_ParseTuple(args, "O:GetRecordFromTypeInfo",
                          &obtypeinfo))  // @pyparm <o PyITypeInfo>|TypeInfo||The type information to be converted into
                                         // a PyRecord object
        return NULL;
    if (!PyCom_InterfaceFromPyInstanceOrObject(obtypeinfo, IID_ITypeInfo, (void **)&pITI, FALSE))
        return NULL;

    hr = GetRecordInfoFromTypeInfo(pITI, &pIRI);
    if (FAILED(hr))
        ret = PyCom_BuildPyException(hr);
    else
        ret = PyObject_FromRecordInfo(pIRI, NULL, 0);
    pITI->Release();
    if (pIRI != NULL)
        pIRI->Release();
    return ret;
}

// This function creates a new 'com_record' instance with placement new.
// If the particular Record GUID belongs to a registered subclass
// of the 'com_record' base type, it instantiates this subclass.
// The optinal 'type' parameter is used by the 'tp_new' slot method to
// specify the subclass right ahead and shortcut the type identification
// procedure. It must match the corresponding 'IRecordInfo' object
// passed in by the 'ri' parameter.
PyRecord *PyRecord::new_record(IRecordInfo *ri, PVOID data, PyRecordBuffer *owner,
                               PyTypeObject *type) /* default: type = NULL */
{
    GUID structguid;
    OLECHAR *guidString;
    PyObject *guidUnicode, *recordType;
    if (type == NULL) {
        // By default we create an instance of the base 'com_record' type.
        type = &PyRecord::Type;
        // Retrieve the GUID of the Record to be created.
        HRESULT hr = ri->GetGuid(&structguid);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr, ri, IID_IRecordInfo);
            return NULL;
        }
        hr = StringFromCLSID(structguid, &guidString);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr);
            return NULL;
        }
        guidUnicode = PyWinCoreString_FromString(guidString);
        if (guidUnicode == NULL) {
            ::CoTaskMemFree(guidString);
            return NULL;
        }
        recordType = PyDict_GetItem(g_obPyCom_MapRecordGUIDToRecordClass, guidUnicode);
        Py_DECREF(guidUnicode);
        // If the Record GUID is registered as a subclass of com_record
        // we return an object of the subclass type.
        if (recordType && PyObject_IsSubclass(recordType, (PyObject *)&PyRecord::Type)) {
            type = (PyTypeObject *)recordType;
        }
    }
    // Finally allocate the memory for the the appropriate
    // Record type and construct the instance with placement new.
    char *buf = (char *)PyRecord::Type.tp_alloc(type, 0);
    if (buf == NULL) {
        delete owner;
        PyErr_NoMemory();
        return NULL;
    }
    return new (buf) PyRecord(ri, data, owner);
}

PyRecord::PyRecord(IRecordInfo *ri, PVOID data, PyRecordBuffer *buf_owner)
{
    ri->AddRef();
    pri = ri;
    pdata = data;
    owner = buf_owner;
    owner->AddRef();
};

PyRecord::~PyRecord()
{
    owner->Release();
    pri->Release();
}

PyObject *PyRecord::tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *item, *guidUnicode;
    PyTypeObject *registeredType;
    int major, minor, lcid;
    GUID guid, infoGuid;
    if (type == &PyRecord::Type) {
        PyErr_SetString(PyExc_TypeError,
                        "Can't instantiate base class com_record. "
                        "Use the factory function win32com.client.Record instead.");
        return NULL;
    }
    // For subclasses of com_record try to get the record type information from the class variables of the derived type.
    if (!(guidUnicode = PyDict_GetItemString(type->tp_dict, "GUID"))) {
        PyErr_Format(PyExc_AttributeError, "Missing %s class attribute.", "GUID");
        return NULL;
    }
    if (!PyWinObject_AsIID(guidUnicode, &infoGuid)) {
        PyErr_Format(PyExc_ValueError, "Invalid value for %s class attribute.", "GUID");
        return NULL;
    }
    if (!(item = PyDict_GetItemString(type->tp_dict, "TLBID"))) {
        PyErr_Format(PyExc_AttributeError, "Missing %s class attribute.", "TLBID");
        return NULL;
    }
    if (!PyWinObject_AsIID(item, &guid)) {
        PyErr_Format(PyExc_ValueError, "Invalid value for %s class attribute.", "TLBID");
        return NULL;
    }
    if (!(item = PyDict_GetItemString(type->tp_dict, "MJVER"))) {
        PyErr_Format(PyExc_AttributeError, "Missing %s class attribute.", "MJVER");
        return NULL;
    }
    if (((major = PyLong_AsLong(item)) == -1 || major < 0)) {
        PyErr_Format(PyExc_ValueError, "Class attribute %s must be a non negative integer.", "MJVER");
        return NULL;
    }
    if (!(item = PyDict_GetItemString(type->tp_dict, "MNVER"))) {
        PyErr_Format(PyExc_AttributeError, "Missing %s class attribute.", "MNVER");
        return NULL;
    }
    if (((minor = PyLong_AsLong(item)) == -1 || minor < 0)) {
        PyErr_Format(PyExc_ValueError, "Class attribute %s must be a non negative integer.", "MNVER");
        return NULL;
    }
    if (!(item = PyDict_GetItemString(type->tp_dict, "LCID"))) {
        PyErr_Format(PyExc_AttributeError, "Missing %s class attribute.", "LCID");
        return NULL;
    }
    if (((lcid = PyLong_AsLong(item)) == -1 || lcid < 0)) {
        PyErr_Format(PyExc_ValueError, "Class attribute %s must be a non negative integer.", "LCID");
        return NULL;
    }
    // Instances can only be created for registerd subclasses.
    registeredType = (PyTypeObject *)PyDict_GetItem(g_obPyCom_MapRecordGUIDToRecordClass, guidUnicode);
    if (!(registeredType && type == registeredType)) {
        PyErr_Format(PyExc_TypeError, "Can't instantiate class %s because it is not registered.", type->tp_name);
        return NULL;
    }
    IRecordInfo *ri = NULL;
    HRESULT hr = GetRecordInfoFromGuids(guid, major, minor, lcid, infoGuid, &ri);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr);
    PyObject *ret = PyObject_FromRecordInfo(ri, NULL, 0, type);
    ri->Release();
    return ret;
}

int PyRecord::tp_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyRecord *pyrec = (PyRecord *)self;
    PyObject *obdata = NULL;
    if (!PyArg_ParseTuple(args, "|O:__init__",
                          &obdata))  // @pyparm string or buffer|data|None|The raw data to initialize the record with.
        return -1;
    if (obdata != NULL) {
        PyWinBufferView pybuf(obdata, false, false);  // None not ok
        if (!pybuf.ok())
            return -1;
        ULONG cb;
        HRESULT hr = pyrec->pri->GetSize(&cb);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
            return -1;
        }
        if (pybuf.len() != cb) {
            PyErr_Format(PyExc_ValueError, "Expecting a string of %d bytes (got %d)", cb, pybuf.len());
            return -1;
        }
        hr = pyrec->pri->RecordCopy(pybuf.ptr(), pyrec->pdata);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
            return -1;
        }
    }
    return 0;
}

PyTypeObject PyRecord::Type = {
    PYWIN_OBJECT_HEAD "com_record",
    sizeof(PyRecord),
    0,
    (destructor)PyRecord::tp_dealloc,         /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    &PyRecord::tp_repr,                       /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    PyRecord::getattro,                       /* tp_getattro */
    PyRecord::setattro,                       /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    PyRecord::tp_richcompare,                 /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyRecord::methods,                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)PyRecord::tp_init,              /* tp_init */
    0,                                        /* tp_alloc */
    (newfunc)PyRecord::tp_new,                /* tp_new */
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
    if (FAILED(hr) || pti == NULL) {
        PyCom_BuildPyException(hr);
        goto done;
    }
    hr = pti->GetTypeAttr(&pta);
    if (FAILED(hr) || pta == NULL) {
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
        obModDict = PyModule_GetDict(obModule);  // no ref added!
    if (obModDict)
        obFunc = PyDict_GetItemString(obModDict, "GetRecordFromGuids");  // no ref added!
    if (!obFunc) {
        PyErr_Clear();
        PyErr_SetString(PyExc_RuntimeError, "pythoncom.GetRecordFromGuids() can't be located!");
        goto done;
    }
    ret =
        Py_BuildValue("O(NHHiNN)", obFunc, PyWinObject_FromIID(pta->guid), pta->wMajorVerNum, pta->wMinorVerNum,
                      pta->lcid, PyWinObject_FromIID(structguid), PyBytes_FromStringAndSize((char *)pyrec->pdata, cb));

done:
    if (pta && pti)
        pti->ReleaseTypeAttr(pta);
    if (pti)
        pti->Release();
    Py_XDECREF(obModule);
    // obModDict and obFunc have no new reference.
    return ret;
}

// The object itself.
// Any method names should be "__blah__", as they override
// structure names!
struct PyMethodDef PyRecord::methods[] = {{"__reduce__", PyRecord_reduce, 1},  // This allows the copy module to work!
                                          {NULL}};

static BSTR *_GetFieldNames(IRecordInfo *pri, ULONG *pnum)
{
    ULONG num_names;
    HRESULT hr = pri->GetFieldNames(&num_names, NULL);
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr, pri, IID_IRecordInfo);
        return NULL;
    }
    BSTR *strings = new BSTR[num_names];
    if (strings == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    for (ULONG i = 0; i < num_names; i++) strings[i] = NULL;

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
    for (ULONG i = 0; i < num_names; i++) SysFreeString(strings[i]);
    delete[] strings;
}

void PyWinCoreString_Concat(register PyObject **pv, register PyObject *w)
{
    if (!w) {  // hrm - string version doesn't do this, but I saw PyObject_Repr() return NULL...
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

PyObject *PyRecord::tp_repr(PyObject *self)
{
    ULONG i;
    PyRecord *pyrec = (PyRecord *)self;
    ULONG num_names;
    BSTR *strings = _GetFieldNames(pyrec->pri, &num_names);
    if (strings == NULL)
        return NULL;
    PyObject *obrepr = NULL, *obattrname;
    BOOL bsuccess = FALSE;
    PyObject *comma = PyWinCoreString_FromString(_T(", "));
    PyObject *equals = PyWinCoreString_FromString(_T("="));
    PyObject *closing_paren = PyWinCoreString_FromString(_T(")"));
    obrepr = PyWinCoreString_FromString(_T("com_struct("));

    if (obrepr == NULL || comma == NULL || equals == NULL || closing_paren == NULL)
        goto done;
    for (i = 0; i < num_names && obrepr != NULL; i++) {
        obattrname = PyWinCoreString_FromString(strings[i]);
        if (obattrname == NULL)
            goto done;
        // must exit on error via loop_error from here...
        PyObject *sub_object = NULL;
        if (i > 0) {
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
    bsuccess = TRUE;
done:
    Py_XDECREF(comma);
    Py_XDECREF(equals);
    Py_XDECREF(closing_paren);
    if (strings)
        _FreeFieldNames(strings, num_names);
    if (!bsuccess) {
        Py_XDECREF(obrepr);
        obrepr = NULL;
    }
    return obrepr;
}

PyObject *PyRecord::getattro(PyObject *self, PyObject *obname)
{
    PyObject *res;
    PyRecord *pyrec = (PyRecord *)self;
    GUID structguid;
    OLECHAR *guidString;
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    if (strcmp(name, "__record_type_guid__") == 0) {
        HRESULT hr = pyrec->pri->GetGuid(&structguid);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
            return NULL;
        }
        hr = StringFromCLSID(structguid, &guidString);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr);
            return NULL;
        }
        res = PyWinCoreString_FromString(guidString);
        ::CoTaskMemFree(guidString);
        return res;
    }
    if (strcmp(name, "__record_type_name__") == 0) {
        BSTR rec_name;
        HRESULT hr = pyrec->pri->GetName(&rec_name);
        if (FAILED(hr))
            return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
        res = PyWinCoreString_FromString(rec_name);
        SysFreeString(rec_name);
        return res;
    }
    if (strcmp(name, "__members__") == 0) {
        ULONG cnames = 0;
        HRESULT hr = pyrec->pri->GetFieldNames(&cnames, NULL);
        if (FAILED(hr))
            return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
        BSTR *strs = (BSTR *)malloc(sizeof(BSTR) * cnames);
        if (strs == NULL)
            return PyErr_NoMemory();
        hr = pyrec->pri->GetFieldNames(&cnames, strs);
        if (FAILED(hr)) {
            free(strs);
            return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
        }
        res = PyList_New(cnames);
        for (ULONG i = 0; i < cnames && res != NULL; i++) {
            PyObject *item = PyWinCoreString_FromString(strs[i]);
            SysFreeString(strs[i]);
            if (item == NULL) {
                Py_DECREF(res);
                res = NULL;
            }
            else
                PyList_SET_ITEM(res, i, item);  // ref count swallowed.
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
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(wname);

    if (FAILED(hr)) {
        if (hr == TYPE_E_FIELDNOTFOUND) {
            // This is slightly suspect - throwing a unicode
            // object for an AttributeError in py2k - but this
            // is the value we asked COM for, so it makes sense...
            // (and PyErr_Format doesn't handle unicode in py2x)
            PyErr_SetObject(PyExc_AttributeError, obname);
            return NULL;
        }
        return PyCom_BuildPyException(hr, pyrec->pri, IID_IRecordInfo);
    }

    // Short-circuit sub-structs and arrays here, so we don't allocate a new chunk
    // of memory and copy it - we need sub-structs to persist.
    if (V_VT(&vret) == (VT_BYREF | VT_RECORD))
        return PyRecord::new_record(V_RECORDINFO(&vret), V_RECORD(&vret), pyrec->owner);
    else if (V_VT(&vret) == (VT_BYREF | VT_ARRAY | VT_RECORD)) {
        SAFEARRAY *psa = *V_ARRAYREF(&vret);
        if (SafeArrayGetDim(psa) != 1)
            return PyErr_Format(PyExc_TypeError, "Only support single dimensional arrays of records");
        IRecordInfo *sub = NULL;
        long ubound, lbound, nelems;
        int i;
        BYTE *this_data;
        PyObject *ret_tuple = NULL;
        ULONG element_size = 0;
        hr = SafeArrayGetUBound(psa, 1, &ubound);
        if (FAILED(hr))
            goto array_end;
        hr = SafeArrayGetLBound(psa, 1, &lbound);
        if (FAILED(hr))
            goto array_end;
        hr = SafeArrayGetRecordInfo(psa, &sub);
        if (FAILED(hr))
            goto array_end;
        hr = sub->GetSize(&element_size);
        if (FAILED(hr))
            goto array_end;
        nelems = ubound - lbound + 1;
        ret_tuple = PyTuple_New(nelems);
        if (ret_tuple == NULL)
            goto array_end;
        // We're dealing here with a Record field that is a SAFEARRAY of Records.
        // Therefore the VARIANT that was returned by the call to 'pyrec->pri->GetFieldNoCopy'
        // does contain a reference to the SAFEARRAY of Records, i.e. the actual data of the
        // Record elements of this SAFEARRAY is referenced by the 'pvData' field of the SAFEARRAY.
        // In this particular case the implementation of 'GetFieldNoCopy' returns a NULL pointer
        // in the last parameter, i.e. 'sub_data == NULL'.
        this_data = (BYTE *)psa->pvData;
        for (i = 0; i < nelems; i++) {
            PyRecord *rec = PyRecord::new_record(sub, this_data, pyrec->owner);
            if (rec == NULL) {
                Py_DECREF(ret_tuple);
                ret_tuple = NULL;
                goto array_end;
            }
            PyTuple_SET_ITEM(ret_tuple, i, rec);
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
    if (strings == NULL)
        return NULL;
    for (ULONG i = 0; i < num_names; i++) {
        ret = 0;
        PyObject *obattrname;
        obattrname = PyWinCoreString_FromString(strings[i]);
        if (obattrname == NULL)
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
        if (!other_sub) {
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

void PyRecord::tp_dealloc(PyRecord *self)
{
    self->~PyRecord();
    Py_TYPE(self)->tp_free((PyObject *)self);
}
