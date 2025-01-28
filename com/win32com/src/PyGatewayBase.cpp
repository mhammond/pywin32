// PyGatewayBase - the IUnknown Gateway Interface

#include "stdafx.h"

#include "PythonCOM.h"
#include "PyFactory.h"

#include "PythonCOMServer.h"

// {25D29CD0-9B98-11d0-AE79-4CF1CF000000}
extern const GUID IID_IInternalUnwrapPythonObject = {
    0x25d29cd0, 0x9b98, 0x11d0, {0xae, 0x79, 0x4c, 0xf1, 0xcf, 0x0, 0x0, 0x0}};

extern PyObject *g_obMissing;

#include <malloc.h>

// Internal ErrorUtil helpers we reach in for.
// Free the strings from an excep-info.
extern void PyCom_CleanupExcepInfo(EXCEPINFO *pexcepinfo);
PYCOM_EXPORT BOOL PyCom_SetCOMErrorFromExcepInfo(const EXCEPINFO *pexcepinfo, REFIID riid);

// #define DEBUG_FULL
static LONG cGateways = 0;
LONG _PyCom_GetGatewayCount(void) { return cGateways; }

// Helper function to handle the IDispatch results
static HRESULT GetIDispatchErrorResult(PyObject *logProvider, EXCEPINFO *pexcepinfo)
{
    HRESULT hr;
    EXCEPINFO tei;
    BOOL bCleanupExcepInfo;
    if (pexcepinfo == NULL) {
        pexcepinfo = &tei;
        bCleanupExcepInfo = TRUE;
    }
    else
        bCleanupExcepInfo = FALSE;
    // Log the error
    PyCom_LoggerNonServerException(logProvider, L"Python error invoking COM method.");

    // Fill the EXCEPINFO with the details.
    PyCom_ExcepInfoFromPyException(pexcepinfo);
    // If the Python code is returning an IDispatch error,
    // return it directly (making the excepinfo available via
    // ISupportErrorIno.
    if (HRESULT_FACILITY(pexcepinfo->scode) == FACILITY_DISPATCH) {
        PyCom_SetCOMErrorFromExcepInfo(pexcepinfo, IID_IDispatch);
        PyCom_CleanupExcepInfo(pexcepinfo);
        hr = pexcepinfo->scode;
    }
    else {
        hr = DISP_E_EXCEPTION;  // and the EXCEPINFO remains valid.
        if (bCleanupExcepInfo)
            PyCom_CleanupExcepInfo(pexcepinfo);
    }
    return hr;
}
/////////////////////////////////////////////////////////////////////////////
//
void *PyGatewayBase::ThisAsIID(IID iid)
{
    if (iid == IID_IUnknown || iid == IID_IDispatch)
        // IDispatch * == IUnknown *
        return (IDispatch *)(PyGatewayBase *)this;
    else if (iid == IID_IDispatchEx)
        // IDispatchEx * probably == IUnknown *, but no real need to assume that!
        return (IDispatchEx *)this;
    else if (iid == IID_ISupportErrorInfo)
        return (ISupportErrorInfo *)this;
    else if (iid == IID_IInternalUnwrapPythonObject)
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
    Py_XINCREF(instance);  // instance should never be NULL - but what's an X between friends!

    PyCom_DLLAddRef();

#ifdef DEBUG_FULL
    PyCom_LogF(U"PyGatewayBase: created %s", m_pPyObject ? m_pPyObject->ob_type->tp_name : "<NULL>");
#endif
}

PyGatewayBase::~PyGatewayBase()
{
    InterlockedDecrement(&cGateways);
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase: deleted %s", m_pPyObject ? m_pPyObject->ob_type->tp_name : "<NULL>");
#endif

    if (m_pPyObject) {
        {
            CEnterLeavePython celp;
            Py_DECREF(m_pPyObject);
        }
    }
    if (m_pBaseObject) {
        m_pBaseObject->Release();
    }
    PyCom_DLLReleaseRef();
}

STDMETHODIMP PyGatewayBase::QueryInterface(REFIID iid, void **ppv)
{
#ifdef DEBUG_FULL
    {
        PY_GATEWAY_METHOD;  // apparently you have to have the thread lock to call the debug print function
        OLECHAR oleRes[128];
        char cRes[256];
        StringFromGUID2(iid, oleRes, sizeof(oleRes));
        // Only for a special debug build, don't worry about error checking
        WideCharToMultiByte(CP_ACP, 0, oleRes, -1, cRes, 256, NULL, NULL);
        PyCom_LogF(L"PyGatewayBase::QueryInterface: %s", cRes);
    }
#endif

    *ppv = NULL;

    // If one of our native interfaces (but NOT IUnknown if we have a base)
    // return this.
    // It is important is that IUnknown come from the base object
    // to ensure that we live by COM identity rules (other interfaces need
    // not abide by this rule - only IUnknown.)
    if ((m_pBaseObject == NULL || iid != IID_IUnknown) && (*ppv = ThisAsIID(iid)) != NULL) {
        AddRef();
        return S_OK;
    }

    // If we have a "base object", then we need to delegate _every_ remaining
    // QI to it.
    if (m_pBaseObject != NULL && (m_pBaseObject->QueryInterface(iid, ppv) == S_OK))
        return S_OK;

    // Call the Python policy to see if it (says it) supports the interface
    long supports = 0;
    {
        CEnterLeavePython celp;
        PyObject *ob = PyWinObject_FromIID(iid);
        if (!ob)
            return E_OUTOFMEMORY;

        PyObject *result = PyObject_CallMethod(m_pPyObject, "_QueryInterface_", "O", ob);
        Py_DECREF(ob);

        if (result) {
            if (PyLong_Check(result))
                supports = PyLong_AsLong(result);
            else if (PyIBase::is_object(result, &PyIUnknown::type)) {
                // We already have the object - return it without additional QI's etc.
                IUnknown *pUnk = PyIUnknown::GetI(result);
                if (pUnk) {
                    pUnk->AddRef();
                    *ppv = pUnk;
                    supports = 1;
                }
            }
            PyErr_Clear();  // ignore exceptions during conversion
            Py_DECREF(result);
        }
        else {
            //			PyRun_SimpleString("import traceback;traceback.print_exc()");
            PyErr_Clear();  // ### what to do with exceptions? ...
        }
    }

    if (supports != 1)
        return E_NOINTERFACE;

    // Make a new gateway object
    if (*ppv == NULL) {
        HRESULT hr = PyCom_MakeRegisteredGatewayObject(iid, m_pPyObject, this, ppv);
        if (FAILED(hr))
            return hr;
    }
    return S_OK;
}

STDMETHODIMP_(ULONG) PyGatewayBase::AddRef(void) { return InterlockedIncrement(&m_cRef); }

STDMETHODIMP_(ULONG) PyGatewayBase::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

STDMETHODIMP PyGatewayBase::GetTypeInfoCount(UINT FAR *pctInfo)
{
    if (pctInfo == NULL)
        return E_POINTER;

    *pctInfo = 0;
    {
        CEnterLeavePython celp;

        PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetTypeInfoCount_", NULL);

        if (result) {
            if (PyLong_Check(result))
                *pctInfo = PyLong_AsLong(result);
            PyErr_Clear();  // ignore exceptions during conversion
            Py_DECREF(result);
        }
        else {
            //			PyRun_SimpleString("import traceback;traceback.print_exc()");
            PyErr_Clear();  // ### what to do with exceptions? ...
        }
    }

    return S_OK;
}

STDMETHODIMP PyGatewayBase::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo FAR *FAR *pptInfo)
{
    HRESULT hr = E_FAIL;

    if (pptInfo == NULL)
        return E_POINTER;
    *pptInfo = NULL;

    CEnterLeavePython celp;

    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetTypeInfo_", "ii", itinfo, lcid);

    /* We expect a tuple containing (HRESULT, typeinfo) */
    if (result) {
        PyObject *pypti;
        if (!PyArg_ParseTuple(result, "iO", &hr, &pypti)) {
            Py_DECREF(result);
            return E_FAIL;
        }
        if (PyIBase::is_object(pypti, &PyITypeInfo::type)) {
            *pptInfo = PyITypeInfo::GetI(pypti);
            (*pptInfo)->AddRef();
        }
        Py_DECREF(result);
    }
    else {
        PyErr_Clear();  // ### what to do with exceptions? ...
        hr = DISP_E_EXCEPTION;
    }
    return hr;
}

static HRESULT getids_setup(UINT cNames, OLECHAR FAR *FAR *rgszNames, LCID lcid, PyObject **pPyArgList,
                            PyObject **pPyLCID)
{
    PyObject *argList = PyTuple_New(cNames);
    if (!argList) {
        PyErr_Clear(); /* ### what to do with exceptions? ... */
        return E_OUTOFMEMORY;
    }

    for (UINT i = 0; i < cNames; i++) {
        PyObject *ob = MakeOLECHARToObj(rgszNames[i]);
        if (!ob) {
            PyErr_Clear(); /* ### what to do with exceptions? ... */
            Py_DECREF(argList);
            return E_OUTOFMEMORY;
        }

        /* Note: this takes our reference for us (even if it fails) */
        if (PyTuple_SetItem(argList, i, ob) == -1) {
            PyErr_Clear(); /* ### what to do with exceptions? ... */
            Py_DECREF(argList);
            return E_FAIL;
        }
    }

    /* use the double stuff to keep lcid unsigned... */
    PyObject *py_lcid = PyLong_FromDouble((double)lcid);
    if (!py_lcid) {
        PyErr_Clear(); /* ### what to do with exceptions? ... */
        Py_DECREF(argList);
        return E_FAIL;
    }

    *pPyArgList = argList;
    *pPyLCID = py_lcid;

    return S_OK;
}

static HRESULT getids_finish(PyObject *result, UINT cNames, DISPID FAR *rgdispid)
{
    if (!result)
        return PyCom_SetCOMErrorFromPyException(IID_IDispatch);

    if (!PySequence_Check(result)) {
        Py_DECREF(result);
        return E_FAIL;
    }

    Py_ssize_t count = PyObject_Length(result);
    if (count != PyWin_SAFE_DOWNCAST(cNames, UINT, Py_ssize_t)) {
        PyErr_Clear(); /* ### toss any potential exception */
        Py_DECREF(result);
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    for (UINT i = 0; i < cNames; ++i) {
        PyObject *ob = PySequence_GetItem(result, i);
        if (!ob) {
            PyErr_Clear(); /* ### what to do with exceptions? ... */
            Py_DECREF(result);
            return E_FAIL;
        }
        if ((rgdispid[i] = PyLong_AsLong(ob)) == DISPID_UNKNOWN)
            hr = DISP_E_UNKNOWNNAME;

        Py_DECREF(ob);
    }

    Py_DECREF(result);

    return hr;
}

STDMETHODIMP PyGatewayBase::GetIDsOfNames(REFIID refiid, OLECHAR FAR *FAR *rgszNames, UINT cNames, LCID lcid,
                                          DISPID FAR *rgdispid)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetIDsOfNames");
#endif

    HRESULT hr;
    PyObject *argList;
    PyObject *py_lcid;

    PY_GATEWAY_METHOD;
    hr = getids_setup(cNames, rgszNames, lcid, &argList, &py_lcid);
    if (SUCCEEDED(hr)) {
        PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetIDsOfNames_", "OO", argList, py_lcid);
        Py_DECREF(argList);
        Py_DECREF(py_lcid);

        hr = getids_finish(result, cNames, rgdispid);
    }
    return hr;
}

static HRESULT invoke_setup(DISPPARAMS FAR *params, LCID lcid, PyObject **pPyArgList, PyObject **pPyLCID)
{
    HRESULT hr = S_OK;
    PyObject *py_lcid = NULL;
    /* Our named arg support is based on a few things.  First,
       GetIDsOfNames() documentation states all params are numbered 0->n.
       Secondly, we have always required that all args be present in the
       Python implementation, and the names of those args need not be
       identical to the names in a typelib.  Given these 2 facts, we can
       safely treat dispids as indexes into our arg tuple.
    */
    // num args is the greatest of cArgs and all the dispIDs presented.
    UINT i;
    UINT numArgs = params->cArgs;
    // handle 1 special case: 1 named arg with dispid of DISPID_PROPERTYPUT
    // We just treat that as positional, so it ends up at the end (which is
    // what we always did)
    UINT numNamedArgs =
        params->cNamedArgs != 1 || params->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT ? params->cNamedArgs : 0;

    for (i = 0; i < numNamedArgs; i++) {
        // make sure it's not a special DISPID we don't understand.
        if (params->rgdispidNamedArgs[i] < 0)
            return DISP_E_PARAMNOTFOUND;
        numArgs = max(numArgs, (UINT)params->rgdispidNamedArgs[i] + 1);
    }

    PyObject *argList = PyTuple_New(numArgs);
    if (!argList) {
        PyErr_Clear(); /* ### what to do with exceptions? ... */
        return E_OUTOFMEMORY;
    }
    // MUST exit from here on error via 'failed:'

    // Fill the positional args - they start at the end.
    for (i = params->cArgs; i != numNamedArgs; --i) {
        PyObject *ob = PyCom_PyObjectFromVariant(params->rgvarg + i - 1);
        if (!ob) {
            hr = E_OUTOFMEMORY;
            goto failed;
        }
        Py_ssize_t pyndx = params->cArgs - i;  // index in Python arg tuple.
        /* Note: this takes our reference for us (even if it fails) */
        if (PyTuple_SetItem(argList, pyndx, ob) == -1) {
            hr = E_FAIL;
            goto failed;
        }
    }
    // Fill the named params.
    for (i = 0; i < numNamedArgs; i++) {
        UINT ndx = params->rgdispidNamedArgs[i];
        assert(PyTuple_GET_ITEM(argList, ndx) == NULL);  // must not have seen it before
        PyObject *ob = PyCom_PyObjectFromVariant(params->rgvarg + i);
        if (!ob) {
            hr = E_OUTOFMEMORY;
            goto failed;
        }
        /* Note: this takes our reference for us (even if it fails) */
        if (PyTuple_SetItem(argList, ndx, ob) == -1) {
            hr = E_FAIL;
            goto failed;
        }
    }
    // and any ones missing get 'Missing' - all positional ones must
    // have been done
    for (i = params->cArgs - numNamedArgs; i < numArgs; i++) {
        if (PyTuple_GET_ITEM(argList, i) == NULL) {
            Py_INCREF(g_obMissing);
            PyTuple_SetItem(argList, i, g_obMissing);
        }
    }

    /* use the double stuff to keep lcid unsigned... */
    py_lcid = PyLong_FromDouble((double)lcid);
    if (!py_lcid) {
        PyErr_Clear(); /* ### what to do with exceptions? ... */
        Py_DECREF(argList);
        return E_FAIL;
    }

    *pPyArgList = argList;
    *pPyLCID = py_lcid;
    return hr;
failed:
    PyCom_LoggerException(NULL, L"Failed to setup call into Python gateway");
    PyErr_Clear();
    Py_DECREF(argList);
    assert(FAILED(hr));  // must have set this.
    return hr;
}

// Named support for [out] args is harder - utilities for sorting
struct NPI {
    DISPID id;
    VARIANT *v;
    unsigned offset;
};

int qsort_compare(const void *arg1, const void *arg2)
{
    // NOTE: We return in DESCENDING order
    return ((NPI *)arg2)->id - ((NPI *)arg1)->id;
}

// Given the COM params, fill a caller-allocated array of indexes into the
// params for all the BYREF's.  -1 will be set for all non-byref args.
// The array to fill should be the same size as pDispParams->cArgs.
// Example: if all args are positional and BYREF, pOffsets will be filled
// with [3, 2, 1, 0] - indicating pDispParams->rgvarg[3] is the first BYREF,
// and rgvarg[0] is the last.  Another example: only last param is BYREF,
// pOffsets will be filled with [0, -1, -1, -1].
// Named params are tricky - IDs could be in any order, but the tuple order
// is fixed - so we sort the named params by their ID to work out the order.
static void fill_byref_offsets(DISPPARAMS *pDispParams, unsigned *pOffsets, unsigned noffsets)
{
    // See above - special case DISPID_PROPERTYPUT.  All other negative
    // DISPIDs have already been rejected.
    UINT numNamedArgs = pDispParams->cNamedArgs != 1 || pDispParams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT
                            ? pDispParams->cNamedArgs
                            : 0;
    // init all.
    memset(pOffsets, -1, noffsets * sizeof(unsigned));
    unsigned ioffset = 0;
    unsigned idispparam;
    // positional args are in reverse order
    for (idispparam = pDispParams->cArgs; idispparam > numNamedArgs && ioffset < noffsets; idispparam--) {
        VARIANT *pv = pDispParams->rgvarg + idispparam - 1;
        // If this param is not byref, try the following one.
        if (!V_ISBYREF(pv))
            continue;
        pOffsets[ioffset] = idispparam - 1;
        ioffset++;
    }
    // named params could have their dispid in any order - so we sort
    // them - but only if necessary
    if (numNamedArgs && ioffset < noffsets) {
        //  NOTE: optimizations possible - if only 1 named param it's
        // obvious which one it is!  If 2 params it's very easy to work
        // it out - so we should only qsort for 3 or more.
        NPI *npi = (NPI *)_malloca(sizeof(NPI) * pDispParams->cNamedArgs);  // death if we fail :)
        for (unsigned i = 0; i < pDispParams->cNamedArgs; i++) {
            npi[i].id = pDispParams->rgdispidNamedArgs[i];
            npi[i].v = &pDispParams->rgvarg[i];
            npi[i].offset = i;
        }
        qsort(npi, pDispParams->cNamedArgs, sizeof(NPI), qsort_compare);
        // Now in descending order - we can just do the same loop again,
        // using our sorted array instead of the original.
        for (; idispparam > 0 && ioffset < noffsets; idispparam--) {
            VARIANT *pv = npi[idispparam - 1].v;
            // If this param is not byref, try the following one.
            if (!V_ISBYREF(pv))
                continue;
            pOffsets[ioffset] = npi[idispparam - 1].offset;
            ioffset++;
        }
    }
}

static HRESULT invoke_finish(PyObject *dispatcher,    /* The dispatcher for the gateway */
                             PyObject *result,        /* The PyObject returned from the Python call */
                             VARIANT FAR *pVarResult, /* Result variant passed by the caller */
                             UINT FAR *puArgErr,      /* May be NULL */
                             EXCEPINFO *einfo,        /* Exception info passed by the caller */
                             REFIID iid,              /* Should be IID_IDispatch or IID_IDispatchEx */
                             DISPPARAMS *pDispParams, /* the params passed to Invoke so byrefs can be handled */
                             bool bAllowHRAndArgErr   /* Invoke() or InvokeEx() functionality? */
)
{
    HRESULT hr = S_OK;
    PyObject *ob = NULL;
    PyObject *userResult = NULL;

    if (bAllowHRAndArgErr) {
        // We are expecting a tuple of (hresult, argErr, userResult)
        // or a simple HRESULT.
        if (PyNumber_Check(result)) {
            hr = PyLong_AsLong(result);
            Py_DECREF(result);
            return hr;
        }
        if (!PySequence_Check(result)) {
            Py_DECREF(result);
            return PyCom_SetCOMErrorFromSimple(E_FAIL, iid, L"The Python function did not return the correct type");
        }

        PyObject *ob = PySequence_GetItem(result, 0);
        if (!ob)
            goto done;
        hr = PyLong_AsLong(ob);
        Py_DECREF(ob);
        ob = NULL;

        Py_ssize_t count = PyObject_Length(result);
        if (count > 0) {
            if (puArgErr) {
                ob = PySequence_GetItem(result, 1);
                if (!ob)
                    goto done;

                *puArgErr = PyLong_AsLong(ob);
                Py_DECREF(ob);
                ob = NULL;
            }
            if (count > 1) {
                userResult = PySequence_GetItem(result, 2);
                if (!userResult)
                    goto done;
            }
        }
    }
    else {
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
    // BYREF, but Python won't.  This would mean Python and VB would see different results
    // from the same function.
    if (PyTuple_Check(userResult)) {
        unsigned cUserResult = PyWin_SAFE_DOWNCAST(PyTuple_Size(userResult), Py_ssize_t, UINT);
        unsigned firstByRef = 0;
        if (pVarResult) {
            ob = PySequence_GetItem(userResult, 0);
            if (!ob)
                goto done;
            if (!PyCom_VariantFromPyObject(ob, pVarResult))
                goto done;
            Py_DECREF(ob);
            ob = NULL;
            firstByRef = 1;
        }
        UINT max_args = min(cUserResult - firstByRef, pDispParams->cArgs);
        UINT *offsets = (UINT *)_malloca(sizeof(UINT) * max_args);
        // Get the offsets into our params of all BYREF args, in order.
        fill_byref_offsets(pDispParams, offsets, max_args);

        // Now loop over the params, and set any byref's
        UINT i;
        for (i = 0; i < max_args; i++) {
            UINT offset = offsets[i];
            if (offset == (UINT)-1) {
                // we've more args than BYREFs.
                PyCom_LoggerWarning(NULL, L"Too many results supplied - %d supplied, but only %d can be set",
                                    cUserResult, i);
                break;
            }
            VARIANT *pv = pDispParams->rgvarg + offset;
            assert(V_ISBYREF(pv));
            // Do the conversion thang
            ob = PyTuple_GetItem(userResult, i + firstByRef);
            if (!ob)
                goto done;
            Py_INCREF(ob);  // tuple fetch doesn't do this!
            // Need to use the ArgHelper to get correct BYREF semantics.
            PythonOleArgHelper arghelper;
            arghelper.m_reqdType = V_VT(pv);
            arghelper.m_convertDirection = POAH_CONVERT_FROM_VARIANT;
            if (!arghelper.MakeObjToVariant(ob, pv))
                goto done;
            Py_DECREF(ob);
            ob = NULL;
        }
    }
    else {
        // Result is not a tuple - check if result or
        // first byref we are trying to set.
        if (pVarResult) {
            PyCom_VariantFromPyObject(userResult, pVarResult);
            // If a Python error, it remains set for handling below...
        }
        else {
            // Single value for the first byref we find.  Probably
            // only 1, but do the whole byref processing to ensure
            // if there is more than 1, we get the right 1.
            UINT offset;
            fill_byref_offsets(pDispParams, &offset, 1);
            if (offset != (UINT)-1) {
                VARIANT *pv = pDispParams->rgvarg + offset;
                assert(V_ISBYREF(pv));
                PythonOleArgHelper arghelper;
                arghelper.m_reqdType = V_VT(pv);
                arghelper.m_convertDirection = POAH_CONVERT_FROM_VARIANT;
                arghelper.MakeObjToVariant(userResult, pv);
                // If a Python error, it remains set for handling below...
            }
        }
    }
done:
    // handle the error before the PyObject cleanups just
    // incase one of these objects destructs and in the process
    // clears the Python error condition.
    if (PyErr_Occurred())
        hr = GetIDispatchErrorResult(dispatcher, einfo);

    Py_DECREF(result);
    Py_XDECREF(userResult);
    Py_XDECREF(ob);
    return hr;
}

STDMETHODIMP PyGatewayBase::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *params,
                                   VARIANT FAR *pVarResult, EXCEPINFO FAR *pexcepinfo, UINT FAR *puArgErr)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::Invoke; dispid=%ld", dispid);
#endif

    HRESULT hr;

    if (pVarResult)
        V_VT(pVarResult) = VT_EMPTY;

    PY_GATEWAY_METHOD;
    PyObject *argList;
    PyObject *py_lcid;
    hr = invoke_setup(params, lcid, &argList, &py_lcid);
    if (SUCCEEDED(hr)) {
        PyObject *result = PyObject_CallMethod(m_pPyObject, "_Invoke_", "iOiO", dispid, py_lcid, wFlags, argList);

        Py_DECREF(argList);
        Py_DECREF(py_lcid);

        if (result == NULL)
            return GetIDispatchErrorResult(m_pPyObject, pexcepinfo);
        else
            hr = invoke_finish(m_pPyObject, result, pVarResult, puArgErr, pexcepinfo, IID_IDispatch, params, true);
    }
    return hr;
}

////////////////////////////////////////////////////////////////////////////
//
// The IDispatchEx implementation
//
//
STDMETHODIMP PyGatewayBase::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetDispID");
#endif
    PY_GATEWAY_METHOD;
    PyObject *obName = PyWinObject_FromBstr(bstrName, FALSE);
    if (obName == NULL)
        return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);

    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetDispID_", "Ol", obName, grfdex);
    Py_DECREF(obName);
    if (result) {
        if (PyLong_Check(result))
            *pid = PyLong_AsLong(result);
        else
            PyErr_SetString(PyExc_TypeError, "_GetDispID_ must return an integer object");
        Py_DECREF(result);
    }
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *params, VARIANT *pVarResult,
                                     EXCEPINFO *pexcepinfo, IServiceProvider *pspCaller)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::InvokeEx; dispid=%ld", id);
#endif

    HRESULT hr;

    if (pVarResult)
        V_VT(pVarResult) = VT_EMPTY;

    PY_GATEWAY_METHOD;
    PyObject *obISP = PyCom_PyObjectFromIUnknown(pspCaller, IID_IServiceProvider, TRUE);
    if (obISP == NULL)
        return GetIDispatchErrorResult(m_pPyObject, pexcepinfo);

    PyObject *argList;
    PyObject *py_lcid;
    hr = invoke_setup(params, lcid, &argList, &py_lcid);
    if (SUCCEEDED(hr)) {
        PyObject *result =
            PyObject_CallMethod(m_pPyObject, "_InvokeEx_", "iOiOOO", id, py_lcid, wFlags, argList, Py_None, obISP);

        Py_DECREF(argList);
        Py_DECREF(py_lcid);

        if (result == NULL)
            hr = GetIDispatchErrorResult(m_pPyObject, pexcepinfo);
        else {
            hr = invoke_finish(m_pPyObject, result, pVarResult, NULL, pexcepinfo, IID_IDispatchEx, params, false);
        }
    }
    Py_DECREF(obISP);
    return hr;
}

STDMETHODIMP PyGatewayBase::DeleteMemberByName(BSTR bstr, DWORD grfdex)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::DeleteMemberByName");
#endif
    PY_GATEWAY_METHOD;
    PyObject *obName = PyWinObject_FromBstr(bstr, FALSE);
    if (obName == NULL)
        return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);

    PyObject *result = PyObject_CallMethod(m_pPyObject, "_DeleteMemberByName_", "Ol", obName, grfdex);
    Py_DECREF(obName);
    Py_XDECREF(result);
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::DeleteMemberByDispID(DISPID id)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::DeleteMemberByDispID");
#endif
    PY_GATEWAY_METHOD;
    PyObject *result = PyObject_CallMethod(m_pPyObject, "_DeleteMemberByDispID_", "l", id);
    Py_XDECREF(result);
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetMemberProperties");
#endif
    PY_GATEWAY_METHOD;
    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetMemberProperties_", "ll", id, grfdexFetch);
    if (result) {
        if (PyLong_Check(result))
            *pgrfdex = PyLong_AsLong(result);
        else
            PyErr_SetString(PyExc_TypeError, "GetMemberProperties must return an integer object");
        Py_DECREF(result);
    }
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::GetMemberName(DISPID id, BSTR *pbstrName)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetMemberName");
#endif
    PY_GATEWAY_METHOD;
    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetMemberName_", "l", id);
    if (result) {
        PyWinObject_AsBstr(result, pbstrName);
        Py_DECREF(result);
    }
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetNextDispID");
#endif
    PY_GATEWAY_METHOD;
    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetNextDispID_", "ll", grfdex, id);
    if (result) {
        if (PyLong_Check(result))
            *pid = PyLong_AsLong(result);
        else
            PyErr_SetString(PyExc_TypeError, "GetNextDispID must return an integer object");
        Py_DECREF(result);
    }
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

STDMETHODIMP PyGatewayBase::GetNameSpaceParent(IUnknown **ppunk)
{
#ifdef DEBUG_FULL
    PyCom_LogF(L"PyGatewayBase::GetNameSpaceParent");
#endif
    PY_GATEWAY_METHOD;
    PyObject *result = PyObject_CallMethod(m_pPyObject, "_GetNameSpaceParent_", NULL);
    if (result) {
        PyCom_InterfaceFromPyInstanceOrObject(result, IID_IUnknown, (void **)ppunk, /* bNoneOK=*/FALSE);
        Py_DECREF(result);
    }
    return PyCom_SetCOMErrorFromPyException(IID_IDispatchEx);
}

////////////////////////////////////////////////////////////////////////////
//
// Extra Python helpers...
//
//
static PyObject *do_dispatch(PyObject *pPyObject, const char *szMethodName, const char *szFormat, va_list va)
{
    // Build the Invoke arguments...
    PyObject *args;
    if (szFormat)
        args = Py_VaBuildValue((char *)szFormat, va);
    else
        args = PyTuple_New(0);
    if (!args)
        return NULL;

    // make sure a tuple.
    if (!PyTuple_Check(args)) {
        PyObject *a = PyTuple_New(1);
        if (a == NULL) {
            Py_DECREF(args);
            return NULL;
        }
        PyTuple_SET_ITEM(a, 0, args);
        args = a;
    }

    PyObject *method = PyObject_GetAttrString(pPyObject, "_InvokeEx_");
    if (!method) {
        PyErr_SetString(PyExc_AttributeError, (char *)szMethodName);
        Py_DECREF(args);
        return NULL;
    }

    // Make the call to _Invoke_
    PyObject *result =
        PyObject_CallFunction(method, "siiOOO", szMethodName, 0, DISPATCH_METHOD, args, Py_None, Py_None);
    Py_DECREF(method);
    Py_DECREF(args);
    return result;
}

STDMETHODIMP PyGatewayBase::InvokeViaPolicy(const char *szMethodName, PyObject **ppResult /* = NULL */,
                                            const char *szFormat /* = NULL */, ...)
{
    va_list va;

    if (m_pPyObject == NULL || szMethodName == NULL)
        return E_POINTER;

    va_start(va, szFormat);
    PyObject *result = do_dispatch(m_pPyObject, szMethodName, szFormat, va);
    va_end(va);

    HRESULT hr = PyCom_SetAndLogCOMErrorFromPyExceptionEx(m_pPyObject, szMethodName, GetIID());

    if (ppResult)
        *ppResult = result;
    else
        Py_XDECREF(result);

    return hr;
}

STDMETHODIMP PyGatewayBase::InterfaceSupportsErrorInfo(REFIID riid)
{
    if (IsEqualGUID(riid, GetIID()))
        return S_OK;

    return S_FALSE;
}

STDMETHODIMP PyGatewayBase::Unwrap(
    /* [out] */ PyObject **pPyObject)
{
    if (pPyObject == NULL)
        return E_POINTER;
    *pPyObject = m_pPyObject;
    Py_INCREF(m_pPyObject);
    return S_OK;
}
