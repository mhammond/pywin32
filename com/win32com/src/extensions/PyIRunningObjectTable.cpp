// PyIRunningObjectTable

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIRunningObjectTable.h"
#include "PyIStream.h"
#include "PyIPersist.h"
#include "PyIMoniker.h"

PyIRunningObjectTable::PyIRunningObjectTable(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIRunningObjectTable::~PyIRunningObjectTable()
{
}

/*static*/ IRunningObjectTable *PyIRunningObjectTable::GetI(PyObject *self)
{
	return (IRunningObjectTable *)PyIUnknown::GetI(self);
}


// @pymethod int|PyIRunningObjectTable|IsRunning|Checks whether an object is running.
PyObject *PyIRunningObjectTable::IsRunning(PyObject *self, PyObject *args)
{
	PyObject *obMoniker;
	// @pyparm <o PyIMoniker>|objectName||The <o PyIMoniker> interface on the moniker to search for in the Running Object Table.
	if (!PyArg_ParseTuple(args, "O:IsRunning", &obMoniker))
		return NULL;

	IRunningObjectTable *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IMoniker *pMoniker;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obMoniker, IID_IMoniker, (void **)&pMoniker, FALSE))
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->IsRunning(pMoniker);
	pMoniker->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IRunningObjectTable);
	return PyInt_FromLong(hr);
	// @rvalue S_OK (ie, 0)|The object identified by objectName is running. 
	// @rvalue S_FALSE (ie, 1)|There is no entry for objectName in the ROT, or that the object it identifies is no longer running (in which case, the entry is revoked). 
}

// @pymethod <o PyIUnknown>|PyIRunningObjectTable|GetObject|Checks whether an object is running.
PyObject *PyIRunningObjectTable::GetObject(PyObject *self, PyObject *args)
{
	PyObject *obMoniker;
	// @pyparm <o PyIMoniker>|objectName||The <o PyIMoniker> interface on the moniker to search for in the Running Object Table.
	if (!PyArg_ParseTuple(args, "O:GetObject", &obMoniker))
		return NULL;

	IRunningObjectTable *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IMoniker *pMoniker;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obMoniker, IID_IMoniker, (void **)&pMoniker, FALSE))
		return NULL;
	IUnknown *punk = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->GetObject(pMoniker, &punk);
	pMoniker->Release();
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IRunningObjectTable);
	return PyCom_PyObjectFromIUnknown(punk, IID_IUnknown, FALSE);
}

// @pymethod <o PyIEnumMoniker>|PyIRunningObjectTable|EnumRunning|Creates an enumerator that can list the monikers of all the objects currently registered in the Running Object Table (ROT).
PyObject *PyIRunningObjectTable::EnumRunning(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":EnumRunning"))
		return NULL;

	IRunningObjectTable *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IEnumMoniker *pEnumMoniker = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->EnumRunning(&pEnumMoniker);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IRunningObjectTable);
	return PyCom_PyObjectFromIUnknown(pEnumMoniker, IID_IEnumMoniker, FALSE);
}

// @object PyIRunningObjectTable|A Python interface to IRunningObjectTable
static struct PyMethodDef PyIRunningObjectTable_methods[] =
{
	{"IsRunning",         PyIRunningObjectTable::IsRunning,  1}, // @pymeth IsRunning|Checks whether an object is running.
	{"GetObject",         PyIRunningObjectTable::GetObject,  1}, // @pymeth GetObject|Checks whether an object is running.
	{"EnumRunning",       PyIRunningObjectTable::EnumRunning,  1}, // @pymeth EnumRunning|Creates an enumerator that can list the monikers of all the objects currently registered in the Running Object Table (ROT).
	{NULL,  NULL}        
};

PyComEnumProviderTypeObject PyIRunningObjectTable::type("PyIRunningObjectTable",
                 &PyIUnknown::type, // @base PyIRunningObjectTable|PyIUnknown
                 sizeof(PyIRunningObjectTable),
                 PyIRunningObjectTable_methods,
                 GET_PYCOM_CTOR(PyIRunningObjectTable),
                 "EnumRunning");
