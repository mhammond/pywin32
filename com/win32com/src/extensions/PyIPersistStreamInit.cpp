// PyIPersistStreamInit

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIStream.h"
#include "PyIPersistStreamInit.h"


/////////////////////////////////////////////////////////////
PyIPersistStreamInit::PyIPersistStreamInit(IUnknown *pDisp) :
	PyIPersistStream(pDisp)
{
	ob_type = &type;
}

PyIPersistStreamInit::~PyIPersistStreamInit()
{
}

/*static*/ IPersistStreamInit *PyIPersistStreamInit::GetI(PyObject *self)
{
	return (IPersistStreamInit *)PyIPersist::GetI(self);
}

// @pymethod |PyIPersistStreamInit|InitNew|Initializes the object to a default state.
PyObject *PyIPersistStreamInit::InitNew(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":InitNew") )
		return NULL;

	IPersistStreamInit *pIPSI = GetI(self);
	if ( pIPSI == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIPSI->InitNew();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPSI, IID_IPersistStreamInit);

	Py_INCREF(Py_None);
	return Py_None;
}

// @object PyIPersistStreamInit|A Python interface to IPersistStreamInit
static struct PyMethodDef PyIPersistStreamInit_methods[] =
{
	{"InitNew",      PyIPersistStreamInit::InitNew, 1}, // @pymeth InitNew|Initializes the object to a default state.
	{NULL,  NULL}        
};

PyComTypeObject PyIPersistStreamInit::type("PyIPersistStreamInit",
                 &PyIPersistStream::type, // @base PyIPersistStreamInit|PyIPersistStream
                 sizeof(PyIPersistStreamInit),
                 PyIPersistStreamInit_methods,
				 GET_PYCOM_CTOR(PyIPersistStreamInit));
