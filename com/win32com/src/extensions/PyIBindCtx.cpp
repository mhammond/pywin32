// PyIBindCtx

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIBindCtx.h"

PyIBindCtx::PyIBindCtx(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIBindCtx::~PyIBindCtx()
{
}

/*static*/ IBindCtx *PyIBindCtx::GetI(PyObject *self)
{
	return (IBindCtx *)PyIUnknown::GetI(self);
}


// @pymethod <o PyIRunningObjectTable>|PyIBindCtx|GetRunningObjectTable|Retrieves an object interfacing to the Running Object Table.
PyObject *PyIBindCtx::GetRunningObjectTable(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":GetRunningObjectTable") )
		return NULL;
	IBindCtx *pMy = GetI(self);
	if (!pMy) return NULL;
	IRunningObjectTable *pROT;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->GetRunningObjectTable(&pROT);
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr)
		return PyCom_BuildPyException(hr, pMy, IID_IRunningObjectTable);
	return PyCom_PyObjectFromIUnknown(pROT, IID_IRunningObjectTable, FALSE);
}

// @object PyIBindCtx|A Python interface to IBindCtx.  Derived from <o PyIUnknown>
static struct PyMethodDef PyIBindCtx_methods[] =
{
	{"GetRunningObjectTable",         PyIBindCtx::GetRunningObjectTable,  1}, // @pymeth GetRunningObjectTable|Retrieves the running object table.
	{NULL,  NULL}        
};

PyComTypeObject PyIBindCtx::type("PyIBindCtx",
                 &PyIUnknown::type, // @base PyIBindCtx|PyIUnknown
                 sizeof(PyIBindCtx),
                 PyIBindCtx_methods,
				 GET_PYCOM_CTOR(PyIBindCtx));

