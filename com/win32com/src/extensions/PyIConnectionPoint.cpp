#include "stdafx.h"
#include "PythonCOM.h"

// @doc
PyIConnectionPoint::PyIConnectionPoint(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIConnectionPoint::~PyIConnectionPoint()
{
}

/* static */ IConnectionPoint *PyIConnectionPoint::GetI(PyObject *self)
{
	return (IConnectionPoint *)PyIUnknown::GetI(self);
}

// @pymethod <o PyIID>|PyIConnectionPoint|GetConnectionInterface|Retrieves the IID of the interface represented by the connection point.
PyObject *PyIConnectionPoint::GetConnectionInterface(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":GetConnectionInterface") )
		return NULL;

	IConnectionPoint *pICP = GetI(self);
	if ( pICP == NULL )
		return NULL;

	IID iid;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pICP->GetConnectionInterface(&iid);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self,hr);

	return PyWinObject_FromIID(iid);
}

// @pymethod <o PyIConnectionPointContainer>|PyIConnectionPoint|GetConnectionPointContainer|Gets the connection point container for the object.
PyObject *PyIConnectionPoint::GetConnectionPointContainer(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":GetConnectionPointContainer") )
		return NULL;

	IConnectionPoint *pICP = GetI(self);
	if ( pICP == NULL )
		return NULL;

	IConnectionPointContainer *pCont;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pICP->GetConnectionPointContainer(&pCont);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self,hr);

	return PyCom_PyObjectFromIUnknown( pCont, IID_IConnectionPointContainer );
}

// @pymethod int|PyIConnectionPoint|Advise|Establishes a connection between the connection point object and the client's sink.
PyObject *PyIConnectionPoint::Advise(PyObject *self, PyObject *args)
{
	PyObject *obUnk;
	// @pyparm <o PyIUnknown>|unk||The client's advise sink
	if ( !PyArg_ParseTuple(args, "O:Advise", &obUnk) )
		return NULL;

	IUnknown *pUnk;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnk, FALSE))
		return NULL;

	IConnectionPoint *pICP = GetI(self);
	if ( pICP == NULL )
		return NULL;

	DWORD cookie;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pICP->Advise( pUnk, &cookie );
	pUnk->Release();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self,hr);
	// @rdesc The result is the connection point identifier used by <om PyIConnectionPoint::Unadvise>
	return PyInt_FromLong(cookie);
}

// @pymethod |PyIConnectionPoint|Unadvise|Terminates an advisory connection previously established through IConnectionPoint::Advise. The dwCookie parameter identifies the connection to terminate.
PyObject *PyIConnectionPoint::Unadvise(PyObject *self, PyObject *args)
{
	DWORD cookie;
	// @pyparm int|cookie||The connection token
	if ( !PyArg_ParseTuple(args, "i:Unadvise", &cookie) )
		return NULL;

	IConnectionPoint *pICP = GetI(self);
	if ( pICP == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pICP->Unadvise(cookie);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self,hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumConnections>|PyIConnectionPoint|EnumConnections|Creates an enumerator to iterate through the connections for the connection point
PyObject *PyIConnectionPoint::EnumConnections(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":EnumConnections") )
		return NULL;

	IConnectionPoint *pICP = GetI(self);
	if ( pICP == NULL )
		return NULL;

	IEnumConnections *p;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pICP->EnumConnections(&p);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self,hr);

	return PyCom_PyObjectFromIUnknown(p, IID_IEnumConnections, FALSE);
}

// @object PyIConnectionPoint|A Python wrapper of a COM IConnectionPoint interface.
static struct PyMethodDef PyIConnectionPoint_methods[] =
{
	{ "GetConnectionInterface", PyIConnectionPoint::GetConnectionInterface, 1 }, // @pymeth GetConnectionInterface|Retrieves the IID of the interface represented by the connection point.
	{ "GetConnectionPointContainer", PyIConnectionPoint::GetConnectionPointContainer, 1 }, // @pymeth GetConnectionPointContainer|Gets the connection point container for the object.
	{ "Advise", PyIConnectionPoint::Advise, 1 }, // @pymeth Advise|Establishes a connection between the connection point object and the client's sink.
	{ "Unadvise", PyIConnectionPoint::Unadvise, 1 }, // @pymeth Unadvise|Terminates an advisory connection previously established through <om PyIConnectionPoint::Advise>.
	{ "EnumConnections", PyIConnectionPoint::EnumConnections, 1 }, // @pymeth EnumConnections|Creates an enumerator to iterate through the connections for the connection point
	{ NULL }
};

PyComTypeObject PyIConnectionPoint::type("PyIConnectionPoint",
		&PyIUnknown::type, // @base PyIConnectionPoint|PyIUnknown
		sizeof(PyIConnectionPoint),
		PyIConnectionPoint_methods,
		GET_PYCOM_CTOR(PyIConnectionPoint));
