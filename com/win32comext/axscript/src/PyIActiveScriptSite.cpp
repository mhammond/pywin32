// IActiveScriptSite client support.

// @doc

#include "stdafx.h"

PyIActiveScriptSite::PyIActiveScriptSite(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIActiveScriptSite::~PyIActiveScriptSite()
{
}

/*static*/ IActiveScriptSite *PyIActiveScriptSite::GetI(PyObject *self)
{
	return (IActiveScriptSite *)PyIUnknown::GetI(self);
}


// @pymethod int|PyIActiveScriptSite|GetLCID|
PyObject *PyIActiveScriptSite::GetLCID(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if (!PyArg_ParseTuple(args, ":GetLCID"))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;
	unsigned long lcid;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->GetLCID(&lcid);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	return PyInt_FromLong(lcid);
}

// @pymethod int|PyIActiveScriptSite|GetItemInfo|
PyObject *PyIActiveScriptSite::GetItemInfo(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obName;
	int mask;
	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;
	if (!PyArg_ParseTuple(args, "Oi:GetItemInfo", &obName, &mask))
		return NULL;
	OLECHAR *name;
	if (!PyWinObject_AsWCHAR(obName, &name))
		return NULL;
	IUnknown *punk = NULL;
	ITypeInfo *ptype = NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->GetItemInfo(name, mask, &punk, &ptype);
	PY_INTERFACE_POSTCALL;
	PyWinObject_FreeWCHAR(name);
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	PyObject *obDispatch = PyCom_PyObjectFromIUnknown(punk, IID_IUnknown);
	PyObject *obType = PyCom_PyObjectFromIUnknown(ptype, IID_ITypeInfo);
	PyObject *rc = NULL;
	if (obDispatch && obType)
		rc = Py_BuildValue("OO", obDispatch, obType);
	Py_XDECREF(obDispatch);
	Py_XDECREF(obType);
	return rc;
}
// @pymethod int|PyIActiveScriptSite|GetDocVersionString|
PyObject *PyIActiveScriptSite::GetDocVersionString(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if (!PyArg_ParseTuple(args, ":GetDocVersionString"))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	BSTR bstr;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->GetDocVersionString(&bstr);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	PyObject * rc = MakeBstrToObj(bstr);
	SysFreeString(bstr);
	return rc;
}
// @pymethod int|PyIActiveScriptSite|OnStateChange|
PyObject *PyIActiveScriptSite::OnStateChange(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	int state;
	if (!PyArg_ParseTuple(args, "i:OnStateChange", &state))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->OnStateChange((SCRIPTSTATE)state);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		// EEK - WSH appears to die if we QI for
		// IID_ISupportErrorInfo - we don't really
		// use this extended info (even if it did provide it)
		return SetPythonCOMError(NULL, sc);
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod int|PyIActiveScriptSite|OnEnterScript|
PyObject *PyIActiveScriptSite::OnEnterScript(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if (!PyArg_ParseTuple(args, ":OnEnterScript"))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->OnEnterScript();
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod int|PyIActiveScriptSite|OnLeaveScript|
PyObject *PyIActiveScriptSite::OnLeaveScript(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	if (!PyArg_ParseTuple(args, ":OnLeaveScript"))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->OnLeaveScript();
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|PyIActiveScriptSite|OnScriptError|
PyObject *PyIActiveScriptSite::OnScriptError(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obASE;
	if (!PyArg_ParseTuple(args, "O:OnScriptError", &obASE))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	IActiveScriptError *pASE;
	if (!PyCom_InterfaceFromPyObject(obASE, IID_IActiveScriptError, (void **)&pASE, FALSE))
		return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->OnScriptError(pASE);
	pASE->Release();
	PY_INTERFACE_POSTCALL;
	// no idea why, but under ASP, OnScriptError() will often return
	// with a KeyboardInterrup set!
	PyWin_MakePendingCalls();
	if (sc != E_FAIL && FAILED(sc)) // E_FAIL is documented as a normal retval.
		return SetPythonCOMError(self, sc);
	return PyInt_FromLong(sc);
}

// @pymethod int|PyIActiveScriptSite|OnScriptTerminate|
PyObject *PyIActiveScriptSite::OnScriptTerminate(PyObject *self, PyObject *args)
{
	PY_INTERFACE_METHOD;
	PyObject *obResult, *obException;
	if (!PyArg_ParseTuple(args, "OO:OnScriptTerminate", &obResult, &obException))
		return NULL;

	IActiveScriptSite *pMySite = GetI(self);
	if (pMySite==NULL) return NULL;

	VARIANT varResult;
	VARIANT *pVarResult = NULL;
	if (obResult!=Py_None) {
		pVarResult = &varResult;
		VariantInit(&varResult);
		if (!PyCom_VariantFromPyObject(obResult, pVarResult))
			return NULL;
	}
	EXCEPINFO excep;
	EXCEPINFO *pExcep = NULL;
	if (obException!=Py_None) {
		pExcep = &excep;
		memset(pExcep, 0, sizeof(EXCEPINFO));
		if (!PyCom_ExcepInfoFromPyObject(obException, pExcep))
			return NULL;
	}
	PY_INTERFACE_PRECALL;
	SCODE sc = pMySite->OnScriptTerminate(pVarResult, pExcep);
	PY_INTERFACE_POSTCALL;
	if (pVarResult)
		VariantClear(pVarResult);
	if (FAILED(sc))
		return SetPythonCOMError(self, sc);
	return PyInt_FromLong(sc);
}

// @object PyIActiveScriptSite|An object providing the IActiveScriptSite interface
static struct PyMethodDef PyIActiveScriptSite_methods[] =
{
	{"GetLCID",PyIActiveScriptSite::GetLCID,  1}, // @pymeth GetLCID|
	{"GetItemInfo",PyIActiveScriptSite::GetItemInfo,  1}, // @pymeth GetItemInfo|
	{"GetDocVersionString",PyIActiveScriptSite::GetDocVersionString,  1}, // @pymeth GetDocVersionString|
	{"OnStateChange",PyIActiveScriptSite::OnStateChange,  1}, // @pymeth OnStateChange|
	{"OnEnterScript",PyIActiveScriptSite::OnEnterScript,  1}, // @pymeth OnEnterScript|
	{"OnLeaveScript",PyIActiveScriptSite::OnLeaveScript,  1}, // @pymeth OnLeaveScript|
	{"OnScriptError",PyIActiveScriptSite::OnScriptError,  1}, // @pymeth OnScriptError|
	{"OnScriptTerminate",PyIActiveScriptSite::OnScriptTerminate,  1}, // @pymeth OnScriptTerminate|
	{NULL,  NULL}        
};

PyComTypeObject PyIActiveScriptSite::type("PyIActiveScriptSite",
                 &PyIUnknown::type,
                 sizeof(PyIActiveScriptSite),
                 PyIActiveScriptSite_methods,
				 GET_PYCOM_CTOR(PyIActiveScriptSite));

