// PyICatInformation

// @doc
#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_ICATINFORMATION

#include <comcat.h>
#include "PyICatInformation.h"

PyICatInformation::PyICatInformation(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyICatInformation::~PyICatInformation()
{
}

/*static*/ ICatInformation *PyICatInformation::GetI(PyObject *self)
{
	return (ICatInformation *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIEnumCATEGORYINFO>|PyICatInformation|EnumCategories|Returns an enumerator for the component categories registered on the system.
PyObject *PyICatInformation::EnumCategories(PyObject *self, PyObject *args)
{
	LCID lcid = 0;
	// @pyparm int|lcid|0|lcid
	if (!PyArg_ParseTuple(args, "|i:EnumCategories", &lcid))
		return NULL;

	ICatInformation *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IEnumCATEGORYINFO *pEnum = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->EnumCategories(lcid, &pEnum);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_ICatInformation);
	return PyCom_PyObjectFromIUnknown(pEnum, IID_IEnumCATEGORYINFO, FALSE);
}

// @pymethod <o PyIEnumGUID>|PyICatInformation|EnumClassesOfCategories|Returns an enumerator over the classes that implement one or more interfaces.
PyObject *PyICatInformation::EnumClassesOfCategories(PyObject *self, PyObject *args)
{
	PyObject *listImplemented = Py_None, *listRequired = Py_None;
	// @pyparm [<o PyIID>, ...]|listIIdImplemented|None|A sequence of <o PyIID> objects, or None.
	// @pyparm list iid|listIIdRequired|None|A sequence of <o PyIID> objects, or None.
	if (!PyArg_ParseTuple(args, "|OO:EnumClassesOfCategories", &listImplemented, &listRequired))
		return NULL;

	ICatInformation *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	ULONG cImplemented = (ULONG)0;
	GUID *pIDs = NULL;
	if (listImplemented!=Py_None) {
		if (!PySequence_Check(listImplemented)) {
			PyErr_SetString(PyExc_TypeError, "Only None or lists are supported for the params.");
			return NULL;
		}
		cImplemented = PySequence_Length(listImplemented);
		pIDs = new GUID[cImplemented];
		for (ULONG i=0;i<cImplemented;i++) {
			PyObject *ob = PySequence_GetItem(listImplemented, i);
			if (ob==NULL || PyWinObject_AsIID(ob, pIDs+i)==FALSE) {
				Py_XDECREF(ob);
				PyErr_SetString(PyExc_TypeError, "One of the GUID's in the list is invalid");
				delete pIDs;
				return NULL;
			}
			Py_DECREF(ob);
		}
	}

	ULONG cRequired = (ULONG)0;
	GUID iidTemp;
	GUID *pIDsReqd = &iidTemp;
	if (listRequired!=Py_None) {
		if (!PySequence_Check(listRequired)) {
			PyErr_SetString(PyExc_TypeError, "Only None or lists are supported for the params.");
			delete pIDs;
			return NULL;
		}
		cRequired = PySequence_Length(listRequired);
		pIDsReqd = new GUID[cRequired];
		for (ULONG i=0;i<cRequired;i++) {
			PyObject *ob = PySequence_GetItem(listRequired, i);
			if (ob==NULL || PyWinObject_AsIID(ob, pIDsReqd+i)==FALSE) {
				PyErr_SetString(PyExc_TypeError, "One of the GUID's in the required list is invalid");
				Py_XDECREF(ob);
				delete pIDs;
				delete pIDsReqd;
				return NULL;
			}
			Py_DECREF(ob);
		}
	}

	IEnumGUID *pEnum = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->EnumClassesOfCategories(cImplemented, pIDs, cRequired, pIDsReqd, &pEnum);
	PY_INTERFACE_POSTCALL;
	delete pIDs;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_ICatInformation);
	return PyCom_PyObjectFromIUnknown(pEnum, IID_IEnumGUID, FALSE);
}

// @pymethod string|PyICatInformation|GetCategoryDesc|Retrieves the localized description string for a specific category ID.
PyObject *PyICatInformation::GetCategoryDesc(PyObject *self, PyObject *args)
{
	LCID lcid = 0;
	PyObject *obCatId;
	// @pyparm int|lcid|0|lcid
	if (!PyArg_ParseTuple(args, "Oi:GetCategoryDesc", &obCatId, &lcid))
		return NULL;

	ICatInformation *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	CATID id;
	if (!PyWinObject_AsIID(obCatId, &id))
		return NULL;

	LPWSTR pResult;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->GetCategoryDesc(id, lcid, &pResult);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_ICatInformation);
	PyObject *rc = PyWinObject_FromWCHAR(pResult);
	// @comm The return type is a unicode object.
	CoTaskMemFree(pResult);
	return rc;
}

// @object PyICatInformation|A Python interface to ICatInformation
static struct PyMethodDef PyICatInformation_methods[] =
{
	{"EnumCategories",       PyICatInformation::EnumCategories,  1}, // @pymeth EnumCategories|Returns an enumerator for the component categories registered on the system.
	{"GetCategoryDesc",       PyICatInformation::GetCategoryDesc,  1}, // @pymeth GetCategoryDesc|Retrieves the localized description string for a specific category ID.
	{"EnumClassesOfCategories", PyICatInformation::EnumClassesOfCategories,  1}, // @pymeth EnumClassesOfCategories|Returns an enumerator over the classes that implement one or more interfaces.
	{NULL,  NULL}        
};

PyComTypeObject PyICatInformation::type("PyICatInformation",
                 &PyIUnknown::type, // @base PyICatInformation|PyIUnknown
                 sizeof(PyICatInformation),
                 PyICatInformation_methods,
				 GET_PYCOM_CTOR(PyICatInformation));

#endif // NO_PYCOM_ICATINFORMATION
