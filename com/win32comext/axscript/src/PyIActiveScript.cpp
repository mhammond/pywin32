#include "stdafx.h"

PyIActiveScript::PyIActiveScript(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIActiveScript::~PyIActiveScript()
{
}

/* static */ IActiveScript *PyIActiveScript::GetI(PyObject *self)
{
	return (IActiveScript *)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIActiveScript::SetScriptSite(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obSite;
	if ( !PyArg_ParseTuple(args, "O:SetScriptSite", &obSite) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	if ( !PyIBase::is_object(obSite, &PyIUnknown::type) )
	{
		PyErr_SetString(PyExc_ValueError, "argument is not a site");
		return NULL;
	}
	IUnknown *punk = PyIUnknown::GetI(obSite);
	if ( !punk )
		return NULL;	/* exception was set by GetI() */
	/* note: we don't explicitly hold a reference to punk */

	IActiveScriptSite *pIASS;
	HRESULT hr;
	Py_BEGIN_ALLOW_THREADS
	hr = punk->QueryInterface(IID_IActiveScriptSite, (LPVOID*)&pIASS);
	Py_END_ALLOW_THREADS
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	PY_INTERFACE_PRECALL;
	hr = pIAS->SetScriptSite(pIASS);
	pIASS->Release();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::GetScriptSite(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obiid = NULL;
	if ( !PyArg_ParseTuple(args, "|O:GetScriptSite", &obiid) )
		return NULL;

	CLSID iid = IID_IActiveScriptSite;
	if ( obiid && !PyWinObject_AsIID(obiid, &iid) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	IUnknown *punk;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetScriptSite(iid, (LPVOID*)&punk);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return PyCom_PyObjectFromIUnknown(punk, iid, FALSE);
}

/* static */ PyObject *PyIActiveScript::SetScriptState(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	int state;
	if ( !PyArg_ParseTuple(args, "i:SetScriptState", &state) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->SetScriptState((SCRIPTSTATE)state);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::GetScriptState(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if ( !PyArg_ParseTuple(args, ":GetScriptState") )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	SCRIPTSTATE state;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetScriptState(&state);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return Py_BuildValue("i", (int)state);
}

/* static */ PyObject *PyIActiveScript::Close(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if ( !PyArg_ParseTuple(args, ":Close") )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->Close();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::AddNamedItem(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obName;
	int flags;
	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, "Oi:AddNamedItem", &obName, &flags) )
		return NULL;
	OLECHAR *name;
	if (!PyWinObject_AsWCHAR(obName, &name))
		return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->AddNamedItem(name, (DWORD)flags);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeWCHAR(name);
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::AddTypeLib(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obiid;
	int major;
	int minor;
	int flags;
	if ( !PyArg_ParseTuple(args, "Oiii:AddTypeLib", &obiid, &major, &minor, &flags) )
		return NULL;

	CLSID libiid;
	if ( !PyWinObject_AsIID(obiid, &libiid) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->AddTypeLib(libiid, (DWORD)major, (DWORD)minor, (DWORD)flags);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::GetScriptDispatch(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obItemName = Py_None;
	if ( !PyArg_ParseTuple(args, "|O:GetScriptDispatch", &obItemName) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	BSTR pstrItemName = NULL;
	if (!PyWinObject_AsBstr(obItemName, &pstrItemName, TRUE))
		return NULL;

	IDispatch *pdisp;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetScriptDispatch(pstrItemName, &pdisp);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeBstr(pstrItemName);
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	if ( !pdisp )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyCom_PyObjectFromIUnknown((IUnknown *)pdisp, IID_IDispatch, FALSE);
}

/* static */ PyObject *PyIActiveScript::GetCurrentScriptThreadID(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if ( !PyArg_ParseTuple(args, ":GetCurrentScriptThreadID") )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	SCRIPTTHREADID id;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetCurrentScriptThreadID(&id);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return Py_BuildValue("i", (int)id);
}

/* static */ PyObject *PyIActiveScript::GetScriptThreadID(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	int win32ThreadID;
	if ( !PyArg_ParseTuple(args, "i:GetScriptThreadID", &win32ThreadID) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	SCRIPTTHREADID id;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetScriptThreadID((DWORD)win32ThreadID, &id);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return Py_BuildValue("i", (int)id);
}

/* static */ PyObject *PyIActiveScript::GetScriptThreadState(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	int id;
	if ( !PyArg_ParseTuple(args, "i:GetScriptThreadState", &id) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	SCRIPTTHREADSTATE state;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->GetScriptThreadState((SCRIPTTHREADID)id, &state);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return Py_BuildValue("i", (int)state);
}

/* static */ PyObject *PyIActiveScript::InterruptScriptThread(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	int id;
	PyObject *obExcepInfo;
	int flags;
	if ( !PyArg_ParseTuple(args, "iOi:InterruptScriptThread", &id, &obExcepInfo, &flags) )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	if ( !PyCom_ExcepInfoFromPyObject(obExcepInfo, &excepInfo) )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->InterruptScriptThread((SCRIPTTHREADID)id, &excepInfo, (DWORD)flags);
	SysFreeString(excepInfo.bstrSource);
	SysFreeString(excepInfo.bstrDescription);
	SysFreeString(excepInfo.bstrHelpFile);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScript::Clone(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if ( !PyArg_ParseTuple(args, ":Clone") )
		return NULL;

	IActiveScript *pIAS = GetI(self);
	if ( pIAS == NULL )
		return NULL;

	IActiveScript *pIASClone;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIAS->Clone(&pIASClone);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	return new PyIActiveScript(pIASClone);
}

static struct PyMethodDef PyIActiveScript_methods[] =
{
	{ "SetScriptSite", PyIActiveScript::SetScriptSite, 1 },
	{ "SetScriptState", PyIActiveScript::SetScriptState, 1 },
	{ "GetScriptState", PyIActiveScript::GetScriptState, 1 },
	{ "Close", PyIActiveScript::Close, 1 },
	{ "AddNamedItem", PyIActiveScript::AddNamedItem, 1 },
	{ "AddTypeLib", PyIActiveScript::AddTypeLib, 1 },
	{ "GetScriptDispatch", PyIActiveScript::GetScriptDispatch, 1 },
	{ "GetCurrentScriptThreadID", PyIActiveScript::GetCurrentScriptThreadID, 1 },
	{ "GetScriptThreadID", PyIActiveScript::GetScriptThreadID, 1 },
	{ "GetScriptThreadState", PyIActiveScript::GetScriptThreadState, 1 },
	{ "InterruptScriptThread", PyIActiveScript::InterruptScriptThread, 1 },
	{ "Clone", PyIActiveScript::Clone, 1 },
	{ NULL }
};

PyComTypeObject PyIActiveScript::type("PyIActiveScript",
		&PyIUnknown::type,
		sizeof(PyIActiveScript),
		PyIActiveScript_methods,
		GET_PYCOM_CTOR(PyIActiveScript));
