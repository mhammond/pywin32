// IProvideClassInfo client support.

#include "stdafx.h"
#include "PyIProvideMultipleClassInfo.h"

///////////////////////////////////////////////////////////////

PyIProvideMultipleClassInfo::PyIProvideMultipleClassInfo(IUnknown *pDisp) :
	PyIProvideClassInfo2(pDisp)
{
	ob_type = &type;
}

PyIProvideMultipleClassInfo::~PyIProvideMultipleClassInfo()
{
}

/*static*/ IProvideMultipleClassInfo *PyIProvideMultipleClassInfo::GetI(PyObject *self)
{
	return (IProvideMultipleClassInfo *)PyIUnknown::GetI(self);
}


// @pymethod int|PyIProvideMultipleClassInfo|GetMultiTypeInfoCount|
PyObject *PyIProvideMultipleClassInfo::GetMultiTypeInfoCount(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetMultiTypeInfoCount"))
		return NULL;

	IProvideMultipleClassInfo *pMyInfo = GetI(self);
	if (pMyInfo==NULL) return NULL;
	unsigned long num;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyInfo->GetMultiTypeInfoCount(&num);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	return PyInt_FromLong(num);
}

// @pymethod (various - depends on flags param)|PyIProvideMultipleClassInfo|GetInfoOfIndex|
PyObject *PyIProvideMultipleClassInfo::GetInfoOfIndex(PyObject *self, PyObject *args)
{
	int item, flags;
	if (!PyArg_ParseTuple(args, "ii:GetInfoOfIndex", &item, &flags))
		return NULL;
	// Must be a reason for providing the flags attribute - possible
	// performance.  Therefore support each item seperately...
	IProvideMultipleClassInfo *pMyInfo = GetI(self);
	if (pMyInfo==NULL) return NULL;
	if (flags==MULTICLASSINFO_GETTYPEINFO) {
		ITypeInfo *pti;
		unsigned long tiFlags;
		PY_INTERFACE_PRECALL;
		SCODE sc = pMyInfo->GetInfoOfIndex(item, flags, &pti, &tiFlags, NULL, NULL, NULL);
		PY_INTERFACE_POSTCALL;
		if (FAILED(sc))
			return SetPythonCOMError(self, sc);
		PyObject *obti = PyCom_PyObjectFromIUnknown(pti, IID_ITypeInfo);
		PyObject *rc = Py_BuildValue("Oi", obti,tiFlags);
		Py_XDECREF(obti);
		return rc;
	}
	if (flags==MULTICLASSINFO_GETNUMRESERVEDDISPIDS) {
		unsigned long reserved;
		PY_INTERFACE_PRECALL;
		SCODE sc = pMyInfo->GetInfoOfIndex(item, flags, NULL, NULL, &reserved, NULL, NULL);
		PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
			return SetPythonCOMError(self, sc);
		return PyInt_FromLong(reserved);
	}
	if (flags==MULTICLASSINFO_GETIIDPRIMARY) {
		IID iid;
		PY_INTERFACE_PRECALL;
		SCODE sc = pMyInfo->GetInfoOfIndex(item, flags, NULL, NULL, NULL, &iid, NULL);
		PY_INTERFACE_POSTCALL;
		if (FAILED(sc))
			return SetPythonCOMError(self, sc);
		return PyWinObject_FromIID(iid);
	}
	if (flags==MULTICLASSINFO_GETIIDSOURCE) {
		IID iid;
		PY_INTERFACE_PRECALL;
		SCODE sc = pMyInfo->GetInfoOfIndex(item, flags, NULL, NULL, NULL, NULL, &iid);
		PY_INTERFACE_POSTCALL;
		if (FAILED(sc))
			return SetPythonCOMError(self, sc);
		return PyWinObject_FromIID(iid);
	}
	PyErr_SetString(PyExc_TypeError, "The flags param is invalid.  Note that you can not 'or' flags together - retrieve each element individually");
	return NULL;
}

// @object PyIProvideMultipleClassInfo|
static struct PyMethodDef PyIProvideMultipleClassInfo_methods[] =
{
	{"GetMultiTypeInfoCount",PyIProvideMultipleClassInfo::GetMultiTypeInfoCount,  1}, // @pymeth GetMultiTypeInfoCount|
	{"GetInfoOfIndex",PyIProvideMultipleClassInfo::GetInfoOfIndex,  1}, // @pymeth GetInfoOfIndex|
	{NULL,  NULL}        
};

PyComTypeObject PyIProvideMultipleClassInfo::type("PyIProvideMultipleClassInfo",
                 &PyIProvideClassInfo2::type,
                 sizeof(PyIProvideMultipleClassInfo),
                 PyIProvideMultipleClassInfo_methods,
				 GET_PYCOM_CTOR(PyIProvideMultipleClassInfo));
