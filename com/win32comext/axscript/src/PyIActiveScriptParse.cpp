#include "stdafx.h"

PyIActiveScriptParse::PyIActiveScriptParse(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIActiveScriptParse::~PyIActiveScriptParse()
{
}

/* static */ IActiveScriptParse *PyIActiveScriptParse::GetI(PyObject *self)
{
	return (IActiveScriptParse *)PyIUnknown::GetI(self);
}

/* static */ PyObject *PyIActiveScriptParse::InitNew(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":InitNew") )
		return NULL;

	IActiveScriptParse *pIASP = GetI(self);
	if ( pIASP == NULL )
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pIASP->InitNew();
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return SetPythonCOMError(self, hr);

	Py_INCREF(Py_None);
	return Py_None;
}

/* static */ PyObject *PyIActiveScriptParse::AddScriptlet(PyObject *self, PyObject *args)
{
	const char *defaultName;
	const char *code;
	const char *itemName;
	const char *subItemName;
	const char *eventName;
	const char *delimiter;
	int sourceContextCookie;
	int startingLineNumber;
	int flags;
	if ( !PyArg_ParseTuple(args, "zsszsziii:AddScriptlet",
						   &defaultName,
						   &code,
						   &itemName,
						   &subItemName,
						   &eventName,
						   &delimiter,
						   &sourceContextCookie,
						   &startingLineNumber,
						   &flags) )
		return NULL;

	IActiveScriptParse *pIASP = GetI(self);
	if ( pIASP == NULL )
		return NULL;

	USES_CONVERSION;
	BSTR bstrName;
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIASP->AddScriptlet(A2OLE(defaultName),
									 A2OLE(code),
									 A2OLE(itemName),
									 A2OLE(subItemName),
									 A2OLE(eventName),
									 A2OLE(delimiter),
									 (DWORD)sourceContextCookie,
									 (ULONG)startingLineNumber,
									 (DWORD)flags,
									 &bstrName,
									 &excepInfo);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyExceptionFromEXCEPINFO(hr, &excepInfo);

	return MakeBstrToObj(bstrName);
}

/* static */ PyObject *PyIActiveScriptParse::ParseScriptText(PyObject *self, PyObject *args)
{
	const char *code;
	const char *itemName;
	PyObject *obContext;
	const char *delimiter;
	int sourceContextCookie;
	int startingLineNumber;
	int flags;
	// Special handling for bWantResult.  If not specified, then
	// it looks at the flags for a reasonable default.  If specified
	// the flag is not used at all.
	BOOL bWantResult = -1;
	if ( !PyArg_ParseTuple(args, "szOziii|i:ParseScriptText",
						   &code,
						   &itemName,
						   &obContext,
						   &delimiter,
						   &sourceContextCookie,
						   &startingLineNumber,
						   &flags,
						   &bWantResult) )
		return NULL;
	if (bWantResult==-1)
		bWantResult = (flags & SCRIPTTEXT_ISEXPRESSION) != 0;

	IActiveScriptParse *pIASP = GetI(self);
	if ( pIASP == NULL )
		return NULL;

	IUnknown *punkContext = NULL;
	if ( obContext != Py_None )
	{
		if ( !PyIBase::is_object(obContext, &PyIUnknown::type) )
		{
			PyErr_SetString(PyExc_ValueError, "argument is not a PyIUnknown");
			return NULL;
		}
		punkContext = PyIUnknown::GetI(obContext);
		if ( !punkContext )
			return NULL;
		/* note: we don't explicitly hold a reference to punkContext */
	}

	USES_CONVERSION;
	VARIANT *pResult = NULL;
	VARIANT varResult;
	if (bWantResult) {
		pResult = &varResult;
		VariantInit(&varResult);
	}

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	PY_INTERFACE_PRECALL;
	HRESULT hr = pIASP->ParseScriptText(A2OLE(code),
										A2OLE(itemName),
										punkContext,
										A2OLE(delimiter),
										(DWORD)sourceContextCookie,
										(ULONG)startingLineNumber,
										(DWORD)flags,
										pResult,
										&excepInfo);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyExceptionFromEXCEPINFO(hr, &excepInfo);

	if (bWantResult) {
		PyObject *result = PyCom_PyObjectFromVariant(&varResult);
		VariantClear(&varResult);
		return result;
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

static struct PyMethodDef PyIActiveScriptParse_methods[] =
{
	{ "InitNew", PyIActiveScriptParse::InitNew, 1 },
	{ "AddScriptlet", PyIActiveScriptParse::AddScriptlet, 1 },
	{ "ParseScriptText", PyIActiveScriptParse::ParseScriptText, 1 },
	{ NULL }
};

PyComTypeObject PyIActiveScriptParse::type("PyIActiveScriptParse",
		&PyIUnknown::type,
		sizeof(PyIActiveScriptParse),
		PyIActiveScriptParse_methods,
		GET_PYCOM_CTOR(PyIActiveScriptParse));
