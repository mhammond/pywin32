// PyGatewayBase - the IUnknown Gateway Interface

#include "stdafx.h"

#include "PythonCOM.h"
#include "PyFactory.h"

#include "PythonCOMServer.h"

// {25D29CD0-9B98-11d0-AE79-4CF1CF000000}
extern const GUID IID_IInternalUnwrapPythonObject = 
	{ 0x25d29cd0, 0x9b98, 0x11d0, { 0xae, 0x79, 0x4c, 0xf1, 0xcf, 0x0, 0x0, 0x0 } };

extern void PyCom_LogF(const char *fmt, ...);
#define LogF PyCom_LogF

// #define DEBUG_FULL
static LONG cGateways = 0;
LONG _PyCom_GetGatewayCount(void)
{
	return cGateways;
}

// Helper function to handle the IDispatch results
static HRESULT GetIDispatchErrorResult(EXCEPINFO *pexcepinfo)
{
	HRESULT hr;
	EXCEPINFO tei;
	BOOL bCleanupExcepInfo;
	if (pexcepinfo==NULL) {
		pexcepinfo = &tei;
		bCleanupExcepInfo = TRUE;
	} else
		bCleanupExcepInfo = FALSE;
	// Fill the EXCEPINFO with the details.
	PyCom_ExcepInfoFromPyException(pexcepinfo);
	// If the Python code is returning an IDispatch error,
	// return it directly (making the excepinfo available via
	// ISupportErrorIno.
	if (HRESULT_FACILITY(pexcepinfo->scode)==FACILITY_DISPATCH) {
		PyCom_SetCOMErrorFromExcepInfo(pexcepinfo, IID_IDispatch);
		PyCom_CleanupExcepInfo(pexcepinfo);
		hr = pexcepinfo->scode;
	} else {
		hr = DISP_E_EXCEPTION; // and the EXCEPINFO remains valid.
		if (bCleanupExcepInfo)
			PyCom_CleanupExcepInfo(pexcepinfo);
	}
	return hr;
}
/////////////////////////////////////////////////////////////////////////////
//
void *PyGatewayBase::ThisAsIID(IID iid)
{
	if (iid==IID_IUnknown ||
		iid==IID_IDispatch)
		// IDispatch * == IUnknown *
		return (IDispatch *)(PyGatewayBase *)this;
#ifndef NO_PYCOM_IDISPATCHEX
	else if (iid==IID_IDispatchEx)
		// IDispatchEx * probably == IUnknown *, but no real need to assume that!
		return (IDispatchEx *)this;
#endif // NO_PYCOM_IDISPATCHEX
	else if (iid==IID_ISupportErrorInfo)
		return (ISupportErrorInfo *)this;
	else if (iid==IID_IInternalUnwrapPythonObject)
		return (IInternalUnwrapPythonObject *)this;
	else
		return NULL;
}

PyGatewayBase::PyGatewayBase(PyObject *instance)
{
	InterlockedIncrement(&cGateways);
	m_pBaseObject = NULL;
	m_cRef = 1;
	m_pPyObject = instance;
	Py_XINCREF(instance); // instance should never be NULL - but whats an X between friends!

	PyCom_DLLAddRef();

#ifdef DEBUG_FULL
	LogF("PyGatewayBase: created %s", m_pPyObject ? m_pPyObject->ob_type->tp_name : "<NULL>");
#endif
}

PyGatewayBase::~PyGatewayBase()
{
	InterlockedDecrement(&cGateways);
#ifdef DEBUG_FULL
	LogF("PyGatewayBase: deleted %s", m_pPyObject ? m_pPyObject->ob_type->tp_name : "<NULL>");
#endif

	if ( m_pPyObject )
	{
		{
			CEnterLeavePython celp;
			Py_DECREF(m_pPyObject);
		}
	}
	if (m_pBaseObject)
	{
		m_pBaseObject->Release();
	}
	PyCom_DLLReleaseRef();
}

STDMETHODIMP PyGatewayBase::QueryInterface(
	REFIID iid,
	void ** ppv
	)
{
#ifdef DEBUG_FULL
	{
		USES_CONVERSION;
		OLECHAR oleRes[128];
		StringFromGUID2(iid, oleRes, sizeof(oleRes));
		LogF("PyGatewayBase::QueryInterface: %s", OLE2T(oleRes));
	}
#endif

	*ppv = NULL;

	// If one of our native interfaces (but NOT IUnknown if we have a base)
	// return this.
	// It is important is that IUnknown come from the base object
	// to ensure that we live by COM identity rules (other interfaces need
	// not abide by this rule - only IUnknown.)
	if ( (m_pBaseObject==NULL || iid!=IID_IUnknown) && (*ppv=ThisAsIID(iid)) != NULL ) {
		AddRef();
		return S_OK;
	}

	// If we have a "base object", then we need to delegate _every_ remaining 
	// QI to it.
	if (m_pBaseObject != NULL && (m_pBaseObject->QueryInterface(iid, ppv)==S_OK))
		return S_OK;

	// Call the Python policy to see if it (says it) supports the interface
	long supports = 0;
	{
		CEnterLeavePython celp;
		PyObject * ob = PyWinObject_FromIID(iid);
		if ( !ob )
			return E_OUTOFMEMORY;

		PyObject *result = PyObject_CallMethod(m_pPyObject, "_QueryInterface_",
											   "O", ob);
		Py_DECREF(ob);

		if ( result )
		{
			if (PyInt_Check(result))
				supports = PyInt_AsLong(result);
			else if ( PyIBase::is_object(result, &PyIUnknown::type) ) {
				// We already have the object - return it without additional QI's etc.
				IUnknown *pUnk = PyIUnknown::GetI(result);
				if (pUnk) {
					pUnk->AddRef();
					*ppv = pUnk;
					supports = 1;
				}
			}
			PyErr_Clear(); // ignore exceptions during conversion 
			Py_DECREF(result);
		}
		else
		{
//			PyRun_SimpleString("import traceback;traceback.print_exc()");
			PyErr_Clear();	// ### what to do with exceptions? ... 
		}
	}

	if ( supports != 1 )
		return E_NOINTERFACE;

	// Make a new gateway object
	if (*ppv==NULL) {
		HRESULT hr = PyCom_MakeRegisteredGatewayObject(iid, m_pPyObject, ppv);
		if (FAILED(hr)) return hr;
	}
	// Now setup the base object pointer back to me.
	// Make sure it is actually a gateway, and not some other
	// IUnknown object grabbed from elsewhere.
	// Also makes sure we have the address of the object OK.
	IUnknown *pLook = (IUnknown *)(*ppv);
	IInternalUnwrapPythonObject *pTemp;
	if (pLook->QueryInterface(IID_IInternalUnwrapPythonObject, (void **)&pTemp)==S_OK) {
		// One of our objects, so set the base object if it doesnt already have one
		PyGatewayBase *pG = (PyGatewayBase *)pTemp;
		// Eeek - just these few next lines need to be thread-safe :-(
		PyWin_AcquireGlobalLock();
		if (pG->m_pBaseObject==NULL && pG != (PyGatewayBase *)this) {
			pG->m_pBaseObject = this;
			pG->m_pBaseObject->AddRef();
		}
		PyWin_ReleaseGlobalLock();
		pTemp->Release();
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) PyGatewayBase::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) PyGatewayBase::Release(void)
{
	LONG cRef = InterlockedDecrement(&m_cRef);
	if ( cRef == 0 )
		delete this;
	return cRef;
}

STDMETHODIMP PyGatewayBase::GetTypeInfoCount(
	UINT FAR* pctInfo
	)
{
	if (pctInfo==NULL)
		return E_POINTER;
	/* ### eventually, let Python be able to return type info */

	*pctInfo = 0;
	return S_OK;
}

STDMETHODIMP PyGatewayBase::GetTypeInfo(
	UINT itinfo,
	LCID lcid,
	ITypeInfo FAR* FAR* pptInfo
	)
{
	if (pptInfo==NULL)
		return E_POINTER;
    *pptInfo = NULL;

	/* ### eventually, let Python be able to return type info */

	return DISP_E_BADINDEX;
}

static HRESULT getids_setup(
	UINT cNames,
	OLECHAR FAR* FAR* rgszNames,
	LCID lcid,
	PyObject **pPyArgList,
	PyObject **pPyLCID
	)
{
	PyObject *argList = PyTuple_New(cNames);
	if ( !argList )
	{
		PyErr_Clear();	/* ### what to do with exceptions? ... */
		return E_OUTOFMEMORY;
	}

	for ( UINT i = 0; i < cNames; i++ )
	{
		PyObject *ob = MakeOLECHARToObj(rgszNames[i]);
		if ( !ob )
		{
			PyErr_Clear();	/* ### what to do with exceptions? ... */
			Py_DECREF(argList);
			return E_OUTOFMEMORY;
		}

		/* Note: this takes our reference for us (even if it fails) */
		if ( PyTuple_SetItem(argList, i, ob) == -1 )
		{
			PyErr_Clear();	/* ### what to do with exceptions? ... */
			Py_DECREF(argList);
			return E_FAIL;
		}
	}

	/* use the double stuff to keep lcid unsigned... */
	PyObject * py_lcid = PyLong_FromDouble((double)lcid);
	if ( !py_lcid )
	{
		PyErr_Clear();	/* ### what to do with exceptions? ... */
		Py_DECREF(argList);
		return E_FAIL;
	}

	*pPyArgList = argList;
	*pPyLCID = py_lcid;

	return S_OK;
}

static HRESULT getids_finish(
	PyObject *result,
	UINT cNames,
	DISPID FAR* rgdispid
	)
{
	if ( !result )
		return PyCom_SetCOMErrorFromPyException(IID_IDispatch);

	if ( !PySequence_Check(result) )
	{
		Py_DECREF(result);
		return E_FAIL;
	}

	UINT count = PyObject_Length(result);
	if ( count != cNames )
	{
		PyErr_Clear();	/* ### toss any potential exception */
		Py_DECREF(result);
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	for ( UINT i = 0; i < cNames; ++i )
	{
		PyObject *ob = PySequence_GetItem(result, i);
		if ( !ob )
		{
			PyErr_Clear();	/* ### what to do with exceptions? ... */
			Py_DECREF(result);
			return E_FAIL;
		}
		if ( (rgdispid[i] = PyInt_AsLong(ob)) == DISPID_UNKNOWN )
			hr = DISP_E_UNKNOWNNAME;

		Py_DECREF(ob);
	}

	Py_DECREF(result);

	return hr;
}

STDMETHODIMP PyGatewayBase::GetIDsOfNames(
	REFIID refiid,
	OLECHAR FAR* FAR* rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID FAR* rgdispid
	)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetIDsOfNames");
#endif

	HRESULT hr;
	PyObject *argList;
	PyObject *py_lcid;

	PY_GATEWAY_METHOD;
	hr = getids_setup(cNames, rgszNames, lcid, &argList, &py_lcid);
	if ( SUCCEEDED(hr) )
	{
		PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetIDsOfNames_",
											   "OO", argList, py_lcid);
		Py_DECREF(argList);
		Py_DECREF(py_lcid);

		hr = getids_finish(result, cNames, rgdispid);
	}
	return hr;
}

static HRESULT invoke_setup(
	DISPPARAMS FAR* params,
	LCID lcid,
	PyObject **pPyArgList,
	PyObject **pPyLCID
	)
{
	PyObject *argList = PyTuple_New(params->cArgs);
	if ( !argList )
	{
		PyErr_Clear();	/* ### what to do with exceptions? ... */
		return E_OUTOFMEMORY;
	}

	PyObject *ob;
	VARIANTARG FAR *pvarg;
	UINT i;
	for ( pvarg = params->rgvarg, i = params->cArgs; i--; ++pvarg )
	{
		ob = PyCom_PyObjectFromVariant(pvarg);
		if ( !ob )
		{
			PyErr_Clear();	/* ### what to do with exceptions? ... */
			Py_DECREF(argList);
			return E_OUTOFMEMORY;
		}

		/* Note: this takes our reference for us (even if it fails) */
		if ( PyTuple_SetItem(argList, i, ob) == -1 )
		{
			PyErr_Clear();	/* ### what to do with exceptions? ... */
			Py_DECREF(argList);
			return E_FAIL;
		}
	}

	/* use the double stuff to keep lcid unsigned... */
	PyObject * py_lcid = PyLong_FromDouble((double)lcid);
	if ( !py_lcid )
	{
		PyErr_Clear();	/* ### what to do with exceptions? ... */
		Py_DECREF(argList);
		return E_FAIL;
	}

	*pPyArgList = argList;
	*pPyLCID = py_lcid;
	return S_OK;
}

static HRESULT invoke_finish(
	PyObject *result, /* The PyObject returned from the Python call */
	VARIANT FAR* pVarResult, /* Result variant passed by the caller */
	UINT FAR* puArgErr, /* May be NULL */
	EXCEPINFO *einfo, /* Exception info passed by the caller */
	REFIID iid, /* Should be IID_IDispatch or IID_IDispatchEx */
	DISPPARAMS *pDispParams, /* the params passed to Invoke so byrefs can be handled */
	bool bAllowHRAndArgErr /* Invoke() or InvokeEx() functionality? */
	)
{
	HRESULT hr = S_OK;
	PyObject *ob = NULL;
	PyObject *userResult = NULL;

	if (bAllowHRAndArgErr) 
	{
		// We are expecting a tuple of (hresult, argErr, userResult)
		// or a simple HRESULT.
		if ( PyNumber_Check(result) )
		{
			hr = PyInt_AsLong(result);
			Py_DECREF(result);
			return hr;
		}
		if ( !PySequence_Check(result) )
		{
			Py_DECREF(result);
			return PyCom_SetCOMErrorFromSimple( E_FAIL, iid, "The Python function did not return the correct type");
		}

		PyObject *ob = PySequence_GetItem(result, 0);
		if ( !ob )
			goto done;
		hr = PyInt_AsLong(ob);
		Py_DECREF(ob);
		ob = NULL;

		int count = PyObject_Length(result);
		if ( count > 0 )
		{
			if ( puArgErr )
			{
				ob = PySequence_GetItem(result, 1);
				if ( !ob ) goto done;

				*puArgErr = PyInt_AsLong(ob);
				Py_DECREF(ob);
				ob = NULL;
			}
			if ( count > 1) {
				userResult = PySequence_GetItem(result, 2);
				if (!userResult) goto done;
			}
		}
	} else {
		// We are expecting only the actual result.
		userResult = result;
		Py_INCREF(userResult);
	}
	// If the actual result specified is not a tuple,
	// then the user may be specifying either the function result,
	// or one of the byrefs.
	// NOTE: We use a specific tuple check rather than a sequence
	// check to avoid strings, and also to allow lists to _not_ qualify
	// here - otherwise returning an array of objects would be difficult.
	// NOTE: Although this is not ideal, it would be evil if the parameters determined
	// how the Python result was unpacked.  VB, for example, will often pass everything
	// BYREF, but Python wont.  This would mean Python and VB would see different results
	// from the same function.
	if (PyTuple_Check(userResult)) {
		unsigned cUserResult = PyTuple_Size(userResult);
		unsigned firstByRef = 0;
		if ( pVarResult )
		{
			ob = PySequence_GetItem(userResult, 0);
			if (!ob) goto done;
			if (!PyCom_VariantFromPyObject(ob, pVarResult)) goto done;
			Py_DECREF(ob);
			ob = NULL;
			firstByRef = 1;
		}
		// Now loop over the params, and set any byref's
		unsigned ituple = firstByRef;
		unsigned idispparam;
		// args are in reverse order
		for (idispparam=pDispParams->cArgs;idispparam>0;idispparam--) {
			// If we havent been given enough values, then we are done.
			if (ituple >= cUserResult)
				break;
			VARIANT *pv = pDispParams->rgvarg+idispparam-1;
			// If this param is not byref, try the following one.
			if (!V_ISBYREF(pv))
				continue;
			// Do the conversion thang
			ob = PyTuple_GetItem(userResult, ituple);
			if (!ob) goto done;
			Py_INCREF(ob); // tuple fetch doesnt do this!
			// Need to use the ArgHelper to get correct BYREF semantics.
			PythonOleArgHelper arghelper;
			arghelper.m_reqdType = V_VT(pv);
			arghelper.m_convertDirection = POAH_CONVERT_FROM_VARIANT;
			if (!arghelper.MakeObjToVariant(ob, pv)) goto done;
			ituple++;
			Py_DECREF(ob);
			ob = NULL;
		}
	} else {
		// Result is not a tuple - check if result or
		// first byref we are trying to set.
		if (pVarResult) {
			PyCom_VariantFromPyObject(userResult, pVarResult);
			// If a Python error, it remains set for handling below...
		} else {
			// Single value for the first byref we find.
			unsigned idispparam;
			// args are in reverse order
			for (idispparam=pDispParams->cArgs;idispparam>0;idispparam--) {
				VARIANT *pv = pDispParams->rgvarg+idispparam-1;
				if (!V_ISBYREF(pv))
					continue;

				PythonOleArgHelper arghelper;
				arghelper.m_reqdType = V_VT(pv);
				arghelper.m_convertDirection = POAH_CONVERT_FROM_VARIANT;
				arghelper.MakeObjToVariant(userResult, pv);
				// If a Python error, it remains set for handling below...
				break;
			}
		}
	}
done:
	// handle the error before the PyObject cleanups just
	// incase one of these objects destructs and in the process
	// clears the Python error condition.
	if (PyErr_Occurred())
		hr = GetIDispatchErrorResult(einfo);

	Py_DECREF(result);
	Py_XDECREF(userResult);
	Py_XDECREF(ob);
	return hr;
}

STDMETHODIMP PyGatewayBase::Invoke(
	DISPID dispid,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS FAR* params,
	VARIANT FAR* pVarResult,
	EXCEPINFO FAR* pexcepinfo,
	UINT FAR* puArgErr
	)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::Invoke; dispid=%ld", dispid);
#endif

	HRESULT hr;

	if ( pVarResult )
		V_VT(pVarResult) = VT_EMPTY;

	/* ### for now: no named args unless it is a PUT operation,
	   ### OR all args are named args, and have contiguous DISPIDs
	*/
	if ( params->cNamedArgs )
	{
		if ( params->cNamedArgs != 1 || params->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT ) {
			if (params->cArgs != params->cNamedArgs)
				// Not all named args.
				return DISP_E_NONAMEDARGS;
			unsigned int argCheck;
			for (argCheck=0;argCheck<params->cNamedArgs;argCheck++)
				if (params->rgdispidNamedArgs[argCheck] != (DISPID)argCheck)
					return DISP_E_NONAMEDARGS;
			// OK - we will let it through.
		}
	}
	PY_GATEWAY_METHOD;
	PyObject *argList;
	PyObject *py_lcid;
	hr = invoke_setup(params, lcid, &argList, &py_lcid);
	if ( SUCCEEDED(hr) )
	{
		PyObject * result = PyObject_CallMethod(m_pPyObject,
												"_Invoke_",
												"iOiO",
												dispid, py_lcid, wFlags,
												argList);

		Py_DECREF(argList);
		Py_DECREF(py_lcid);

		if ( result==NULL )
			return GetIDispatchErrorResult(pexcepinfo);
		else
			hr = invoke_finish(result, pVarResult, puArgErr, pexcepinfo, IID_IDispatch, params, true);
	}
	return hr;
}

#ifndef NO_PYCOM_IDISPATCHEX
////////////////////////////////////////////////////////////////////////////
//
// The IDispatchEx implementation
//
//
STDMETHODIMP PyGatewayBase::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetDispID");
#endif
	PY_GATEWAY_METHOD;
	PyObject *obName = PyWinObject_FromBstr(bstrName, FALSE);
	if (obName==NULL) return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);

	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetDispID_",
											   "Ol", obName, grfdex);
	Py_DECREF(obName);
	if (result) {
		if (PyInt_Check(result))
			*pid = PyInt_AsLong(result);
		else
			PyErr_SetString(PyExc_TypeError, "_GetDispID_ must return an integer object");
		Py_DECREF(result);
	}
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *params, VARIANT *pVarResult, EXCEPINFO *pexcepinfo, IServiceProvider *pspCaller)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::InvokeEx; dispid=%ld", dispid);
#endif

	HRESULT hr;

	if ( pVarResult )
		V_VT(pVarResult) = VT_EMPTY;

	/* ### for now: no named args unless it is a PUT operation,
	   ### OR all args are named args, and have contiguous DISPIDs
	*/
	if ( params->cNamedArgs )
	{
		if ( params->cNamedArgs != 1 || params->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT ) {
			if (params->cArgs != params->cNamedArgs)
				// Not all named args.
				return DISP_E_NONAMEDARGS;
			unsigned int argCheck;
			for (argCheck=0;argCheck<params->cNamedArgs;argCheck++)
				if (params->rgdispidNamedArgs[argCheck] != (DISPID)argCheck)
					return DISP_E_NONAMEDARGS;
			// OK - we will let it through.
		}
	}
	PyObject *obISP = PyCom_PyObjectFromIUnknown(pspCaller, IID_IServiceProvider, TRUE);
	if (obISP==NULL)
		return GetIDispatchErrorResult(pexcepinfo);

	PY_GATEWAY_METHOD;
	PyObject *argList;
	PyObject *py_lcid;
	hr = invoke_setup(params, lcid, &argList, &py_lcid);
	if ( SUCCEEDED(hr) )
	{
		PyObject * result = PyObject_CallMethod(m_pPyObject,
												"_InvokeEx_",
												"iOiOOO",
												id, py_lcid, wFlags,
												argList, Py_None,
												obISP);

		Py_DECREF(argList);
		Py_DECREF(py_lcid);

		if ( result==NULL )
			hr = GetIDispatchErrorResult(pexcepinfo);
		else {
			hr = invoke_finish(result, pVarResult, NULL, pexcepinfo, IID_IDispatchEx, params, false);
		}
	}
	Py_DECREF(obISP);
	return hr;
}


STDMETHODIMP PyGatewayBase::DeleteMemberByName(BSTR bstr, DWORD grfdex)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::DeleteMemberByName");
#endif
	PY_GATEWAY_METHOD;
	PyObject *obName = PyWinObject_FromBstr(bstr, FALSE);
	if (obName==NULL) return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);

	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_DeleteMemberByName_",
											   "Ol", obName, grfdex);
	Py_DECREF(obName);
	Py_XDECREF(result);
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}


STDMETHODIMP PyGatewayBase::DeleteMemberByDispID(DISPID id)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::DeleteMemberByDispID");
#endif
	PY_GATEWAY_METHOD;
	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_DeleteMemberByDispID_",
											   "l", id);
	Py_XDECREF(result);
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}


STDMETHODIMP PyGatewayBase::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetMemberProperties");
#endif
	PY_GATEWAY_METHOD;
	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetMemberProperties_",
											   "ll", id, grfdexFetch);
	if (result) {
		if (PyInt_Check(result))
			*pgrfdex = PyInt_AsLong(result);
		else
			PyErr_SetString(PyExc_TypeError, "GetMemberProperties must return an integer object");
		Py_DECREF(result);
	}
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}


STDMETHODIMP PyGatewayBase::GetMemberName(DISPID id, BSTR *pbstrName)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetMemberName");
#endif
	PY_GATEWAY_METHOD;
	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetMemberName_",
											   "l", id);
	if (result) {
		PyWinObject_AsBstr(result, pbstrName);
		Py_DECREF(result);
	}
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}


STDMETHODIMP PyGatewayBase::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetNextDispID");
#endif
	PY_GATEWAY_METHOD;
	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetNextDispID_",
											   "ll", grfdex, id);
	if (result) {
		if (PyInt_Check(result))
			*pid = PyInt_AsLong(result);
		else
			PyErr_SetString(PyExc_TypeError, "GetNextDispID must return an integer object");
		Py_DECREF(result);
	}
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}


STDMETHODIMP PyGatewayBase::GetNameSpaceParent(IUnknown **ppunk)
{
#ifdef DEBUG_FULL
	LogF("PyGatewayBase::GetNameSpaceParent");
#endif
	PY_GATEWAY_METHOD;
	PyObject *result = PyObject_CallMethod(m_pPyObject,
											   "_GetNameSpaceParent_",
											   NULL);
	if (result) {
		PyCom_InterfaceFromPyInstanceOrObject(result, IID_IUnknown, (void **)ppunk, /* bNoneOK=*/FALSE);
		Py_DECREF(result);
	}
	return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

#endif // NO_PYCOM_IDISPATCHEX

////////////////////////////////////////////////////////////////////////////
//
// Extra Python helpers...
//
//
static PyObject *do_dispatch(
	PyObject *pPyObject,
	const char *szMethodName,
	const char *szFormat,
	va_list va
	)
{
	// Build the Invoke arguments...
	PyObject *args;
	if ( szFormat )
		args = Py_VaBuildValue((char *)szFormat, va);
	else
		args = PyTuple_New(0);
	if ( !args )
		return NULL;

	// make sure a tuple.
	if ( !PyTuple_Check(args) )
    {
		PyObject *a = PyTuple_New(1);
		if ( a == NULL )
		{
			Py_DECREF(args);
			return NULL;
		}
		PyTuple_SET_ITEM(a, 0, args);
		args = a;
    }

	PyObject *method = PyObject_GetAttrString(pPyObject, "_InvokeEx_");
	if ( !method )
    {
		PyErr_SetString(PyExc_AttributeError, (char *)szMethodName);
		return NULL;
    }

	// Make the call to _Invoke_
	PyObject *result = PyObject_CallFunction(method,
											 "siiOOO",
											 szMethodName,
											 0,
											 DISPATCH_METHOD,
											 args, Py_None, Py_None);
	Py_DECREF(method);
	Py_DECREF(args);
	return result;
}

STDMETHODIMP PyGatewayBase::InvokeViaPolicy(
	const char *szMethodName,
	PyObject **ppResult /* = NULL */,
	const char *szFormat /* = NULL */,
	...
	)
{
	va_list va;

	if ( m_pPyObject == NULL || szMethodName == NULL )
		return E_POINTER;

	va_start(va, szFormat);
	PyObject *result = do_dispatch(m_pPyObject, szMethodName, szFormat, va);
	va_end(va);

	HRESULT hr = PyCom_SetCOMErrorFromPyException(GetIID());

	if ( ppResult )
		*ppResult = result;
	else
		Py_XDECREF(result);

	return hr;
}

STDMETHODIMP PyGatewayBase::InterfaceSupportsErrorInfo(REFIID riid)
{
	if ( IsEqualGUID(riid, GetIID()) )
		return S_OK;

	return S_FALSE;
}

STDMETHODIMP PyGatewayBase::Unwrap(
            /* [out] */ PyObject **pPyObject)
{
	if (pPyObject==NULL)
		return E_POINTER;
	*pPyObject = m_pPyObject;
	Py_INCREF(m_pPyObject);
	return S_OK;
}

