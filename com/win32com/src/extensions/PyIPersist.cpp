// PyIPersist

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIPersist.h"

PyIPersist::PyIPersist(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIPersist::~PyIPersist()
{
}

/* static */ IPersist *PyIPersist::GetI(PyObject *self)
{
	return (IPersist *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIID>|PyIPersist|GetClassID|Returns the class identifier (CLSID) for the component object.
PyObject *PyIPersist::GetClassID(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":GetClassID") )
		return NULL;

	IPersist *pIP = GetI(self);
	if ( pIP == NULL )
		return NULL;

	CLSID clsid;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIP->GetClassID(&clsid);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIP, IID_IPersist);

	return PyWinObject_FromIID(clsid);
}

// @object PyIPersist|A Python interface to IPersist
static struct PyMethodDef PyIPersist_methods[] =
{
	{ "GetClassID", PyIPersist::GetClassID, 1 }, // @pymeth GetClassID|Returns the class identifier (CLSID) for the component object.
	{ NULL }
};

PyComTypeObject PyIPersist::type("PyIPersist",
		&PyIUnknown::type, // @base PyIPersist|PyIUnknown
		sizeof(PyIPersist),
		PyIPersist_methods,
		GET_PYCOM_CTOR(PyIPersist));
