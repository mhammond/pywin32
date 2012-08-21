#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "propbag.h"
// @doc
PyIPropertyBag::PyIPropertyBag(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIPropertyBag::~PyIPropertyBag()
{
}

/* static */ IPropertyBag *PyIPropertyBag::GetI(PyObject *self)
{
	return (IPropertyBag *)PyIUnknown::GetI(self);
}

// @pymethod object|PyIPropertyBag|Read|Called by the control to read a property from the storage provided by the container.
PyObject *PyIPropertyBag::Read(PyObject *self, PyObject *args)
{
	int varType = VT_EMPTY;
	PyObject *obLog = NULL;
	PyObject *obName;
	// @pyparm str|propName||Name of the property to read.
	// @pyparm int|propType||The type of the object to read.  Must be a VT_* Variant Type constant.
	// @pyparm <o PyIErrorLog>|errorLog|None|The caller's <o PyIErrorLog> object in which the property bag stores any errors that occur during reads. Can be None in which case the caller is not interested in errors.
	if ( !PyArg_ParseTuple(args, "O|iO:Read", &obName, &varType, &obLog) )
		return NULL;

	IPropertyBag *pIPB = GetI(self);
	if ( pIPB == NULL )
		return NULL;

	TmpWCHAR Name;
	if (!PyWinObject_AsWCHAR(obName, &Name))
		return NULL;
	IErrorLog *pIEL = NULL;
	if ( obLog != NULL && obLog != Py_None &&
		!PyCom_InterfaceFromPyObject(obLog, IID_IErrorLog, (LPVOID*)&pIEL, FALSE) )
		return NULL;

	VARIANT var;
	VariantInit(&var);
	V_VT(&var) = varType;	// ### do we need to set anything more?

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIPB->Read(Name, &var, pIEL);
	if ( pIEL != NULL )
		pIEL->Release();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPB, IID_IPropertyBag);

	PyObject *result = PyCom_PyObjectFromVariant(&var);
	VariantClear(&var);
	// @comm The result is a Python object, mapped from a COM VARIANT of type as specified in the propType parameter.
	return result;
}

// @pymethod |PyIPropertyBag|Write|Called by the control to write each property in turn to the storage provided by the container.
PyObject *PyIPropertyBag::Write(PyObject *self, PyObject *args)
{
	PyObject *obName;
	PyObject *obValue;
	// @pyparm str|propName||Name of the property to read.
	// @pyparm object|value||The value for the property.  The value must be able to be converted to a COM VARIANT.
	if ( !PyArg_ParseTuple(args, "OO:Write", &obName, &obValue) )
		return NULL;

	IPropertyBag *pIPB = GetI(self);
	if ( pIPB == NULL )
		return NULL;

	TmpWCHAR Name;
	if ( !PyWinObject_AsWCHAR(obName, &Name))
		return NULL;
	VARIANT var;
	if ( !PyCom_VariantFromPyObject(obValue, &var) )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIPB->Write(Name, &var);
	VariantClear(&var);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPB, IID_IPropertyBag);

	Py_INCREF(Py_None);
	return Py_None;
}

// @object PyIPropertyBag|A Python wrapper for a COM IPropertyBag interface.
// @comm The IPropertyBag interface provides an object with a property bag in which the object can persistently save its properties.<nl>
// When a client wishes to have exact control over how individually named properties of an object are saved, it would attempt to use an object's IPersistPropertyBag interface as a persistence mechanism. In that case the client supplies a property bag to the object in the form of an IPropertyBag interface.
static struct PyMethodDef PyIPropertyBag_methods[] =
{
	{ "Read", PyIPropertyBag::Read, 1 }, // @pymeth Read|Called by the control to read a property from the storage provided by the container.
	{ "Write", PyIPropertyBag::Write, 1 }, // @pymeth Write|Called by the control to write each property in turn to the storage provided by the container.
	{ NULL }
};

PyComTypeObject PyIPropertyBag::type("PyIPropertyBag",
		&PyIUnknown::type, // @base PyIPropertyBag|PyIUnknown
		sizeof(PyIPropertyBag),
		PyIPropertyBag_methods,
		GET_PYCOM_CTOR(PyIPropertyBag));
