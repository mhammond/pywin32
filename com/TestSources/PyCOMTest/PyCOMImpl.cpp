// PyCOMTest.cpp : Implementation of CConnectApp and DLL registration.

#include "preconn.h"
#include "PyCOMTest.h"
#include "PyCOMImpl.h"
#include "SimpleCounter.h"
#include "objbase.h"

#include "stdio.h"
#include "string.h"
#include "initguid.h"

// The CLSID of the Python test vtable component
// {e743d9cd-cb03-4b04-b516-11d3a81c1597}
DEFINE_GUID(CLSID_PythonTestPyCOMTest, 0xe743d9cd, 0xcb03, 0x4b04, 0xb5, 0x16, 0x11, 0xd3, 0xa8, 0x1c, 0x15, 0x97);

/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CPyCOMTest::InterfaceSupportsErrorInfo(REFIID riid)
{
    if (riid == IID_IPyCOMTest)
        return S_OK;
    return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//

DWORD WINAPI PyCOMTestSessionThreadEntry(void *pv)
{
    // Init COM for the thread.
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CPyCOMTest::PyCOMTestSessionData *pS = (CPyCOMTest::PyCOMTestSessionData *)pv;
    // Unmarshal the interface pointer.
    IPyCOMTest *pi;
    HRESULT hr = CoGetInterfaceAndReleaseStream(pS->pStream, IID_IPyCOMTest, (void **)&pi);
    CComPtr<IPyCOMTest> p(pi);
    while (WaitForSingleObject(pS->m_hEvent, 0) != WAIT_OBJECT_0) p->Fire(pS->m_nID);
    p.Release();
    CoUninitialize();
    return 0;
}

CPyCOMTest::~CPyCOMTest()
{
    if (pLastArray) {
        SafeArrayDestroy(pLastArray);
        pLastArray = NULL;
    }
    StopAll();
}

void CPyCOMTest::CreatePyCOMTestSession(PyCOMTestSessionData &rs)
{
    DWORD dwThreadID = 0;
    _ASSERTE(rs.m_hEvent == NULL);
    _ASSERTE(rs.m_hThread == NULL);
    _ASSERTE(rs.pStream == NULL);
    rs.m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_IPyCOMTest, (IPyCOMTest *)this, &rs.pStream);
    _ASSERTE(SUCCEEDED(hr) && rs.pStream != NULL);
    rs.m_hThread = CreateThread(NULL, 0, &PyCOMTestSessionThreadEntry, &rs, 0, &dwThreadID);
}

STDMETHODIMP CPyCOMTest::Start(long *pnID)
{
    if (pnID == NULL)
        return E_POINTER;
    *pnID = 0;
    HRESULT hRes = S_OK;
    m_cs.Lock();
    long i = 0;
    for (; i < nMaxSessions; i++) {
        if (m_rsArray[i].m_hEvent == NULL) {
            m_rsArray[i].m_nID = i;
            CreatePyCOMTestSession(m_rsArray[i]);
            *pnID = i;
            break;
        }
    }
    if (i == nMaxSessions)  // fell through
        hRes = E_FAIL;
    m_cs.Unlock();
    return hRes;
}

STDMETHODIMP CPyCOMTest::Stop(long nID)
{
    HRESULT hRes = S_OK;
    m_cs.Lock();
    if (m_rsArray[nID].m_hEvent != NULL) {
        SetEvent(m_rsArray[nID].m_hEvent);
        WaitForSingleObject(m_rsArray[nID].m_hThread, INFINITE);
        CloseHandle(m_rsArray[nID].m_hThread);
        memset(&m_rsArray[nID], 0, sizeof(PyCOMTestSessionData));
    }
    else
        hRes = E_INVALIDARG;
    m_cs.Unlock();
    return hRes;
}

STDMETHODIMP CPyCOMTest::StopAll()
{
    m_cs.Lock();
    for (long i = 0; i < nMaxSessions; i++) {
        if (m_rsArray[i].m_hEvent != NULL) {
            SetEvent(m_rsArray[i].m_hEvent);
            WaitForSingleObject(m_rsArray[i].m_hThread, INFINITE);
            CloseHandle(m_rsArray[i].m_hThread);
            memset(&m_rsArray[i], 0, sizeof(PyCOMTestSessionData));
        }
    }
    m_cs.Unlock();
    return S_OK;
}

//////////////////////
//
// My test specific stuff
//
STDMETHODIMP CPyCOMTest::Test(VARIANT, QsBoolean in, QsBoolean *out)
{
    *out = in;
    return S_OK;
}
STDMETHODIMP CPyCOMTest::Test2(QsAttribute in, QsAttribute *out)
{
    *out = in;
    return S_OK;
}
STDMETHODIMP CPyCOMTest::Test3(TestAttributes1 in, TestAttributes1 *out)
{
    *out = in;
    return S_OK;
}
STDMETHODIMP CPyCOMTest::Test4(TestAttributes2 in, TestAttributes2 *out)
{
    *out = in;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::Test5(TestAttributes1 *inout) { return S_OK; }
STDMETHODIMP CPyCOMTest::Test6(QsAttributeWide in, QsAttributeWide *out)
{
    *out = in;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::TestInOut(float *fval, QsBoolean *bval, long *lval)
{
    *fval *= 2;
    *lval *= 2;
    *bval = !*bval;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetInterface(IPyCOMTest *ininterface, IPyCOMTest **outinterface)
{
    if (outinterface == NULL)
        return E_POINTER;
    *outinterface = ininterface;
    // Looks like I should definately AddRef() :-)
    ininterface->AddRef();
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetInterfaceArray(SAFEARRAY *pin, SAFEARRAY **pout) { return E_NOTIMPL; }

STDMETHODIMP CPyCOMTest::GetMultipleInterfaces(IPyCOMTest **outinterface1, IPyCOMTest **outinterface2)
{
    if (outinterface1 == NULL || outinterface2 == NULL)
        return E_POINTER;
    *outinterface1 = this;
    *outinterface2 = this;
    InternalAddRef();  // ??? Correct call?  AddRef fails compile...
    InternalAddRef();
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetDispatch(IDispatch *indisp, IDispatch **outdisp)
{
    *outdisp = indisp;
    indisp->AddRef();
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetUnknown(IUnknown *inunk, IUnknown **outunk)
{
    *outunk = inunk;
    inunk->AddRef();
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetVariant(VARIANT var, VARIANT *out)
{
    VariantClear(out);  // necessary?
    return VariantCopy(out, &var);
}

STDMETHODIMP CPyCOMTest::GetSetInt(int invar, int *outvar)
{
    if (!outvar)
        return E_POINTER;
    *outvar = invar;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetUnsignedInt(unsigned int invar, unsigned int *outvar)
{
    if (!outvar)
        return E_POINTER;
    *outvar = invar;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetLong(long invar, long *outvar)
{
    if (!outvar)
        return E_POINTER;
    *outvar = invar;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetUnsignedLong(unsigned long invar, unsigned long *outvar)
{
    if (!outvar)
        return E_POINTER;
    *outvar = invar;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetVariantAndType(VARIANT var, unsigned short *vt, VARIANT *vout)
{
    if (!vt || !vout)
        return E_POINTER;
    VariantClear(vout);
    *vt = V_VT(&var);
    return VariantCopy(vout, &var);
}

STDMETHODIMP CPyCOMTest::TestByRefVariant(VARIANT *v)
{
    if (V_VT(v) == VT_I4) {
        V_I4(v) *= 2;
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CPyCOMTest::TestByRefString(BSTR *v)
{
    BSTR out = SysAllocStringLen(NULL, SysStringLen(*v) * 2);
    wcscpy(out, *v);
    wcscat(out, *v);
    SysFreeString(*v);
    *v = out;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::TakeByRefTypedDispatch(IPyCOMTest **inout) { return S_OK; }
STDMETHODIMP CPyCOMTest::TakeByRefDispatch(IDispatch **inout) { return S_OK; }

HRESULT _SetArrayHelper(VARTYPE expectedType, SAFEARRAY *vals, int *resultSize)
{
    VARTYPE gotType;
    HRESULT hr = SafeArrayGetVartype(vals, &gotType);
    if (FAILED(hr)) {
        return hr;
    }
    if (gotType != expectedType) {
        return E_UNEXPECTED;
    }
    UINT cDims = SafeArrayGetDim(vals);
    *resultSize = 0;
    long ub = 0, lb = 0;
    if (cDims) {
        SafeArrayGetUBound(vals, 1, &ub);
        SafeArrayGetLBound(vals, 1, &lb);
        *resultSize = ub - lb + 1;
    }
    return S_OK;
}

STDMETHODIMP CPyCOMTest::SetBinSafeArray(SAFEARRAY *buf, int *resultSize)
{
    return _SetArrayHelper(VT_UI1, buf, resultSize);
}

STDMETHODIMP CPyCOMTest::SetIntSafeArray(SAFEARRAY *ints, int *resultSize)
{
    return _SetArrayHelper(VT_I4, ints, resultSize);
}

STDMETHODIMP CPyCOMTest::SetLongLongSafeArray(SAFEARRAY *ints, int *resultSize)
{
    return _SetArrayHelper(VT_I8, ints, resultSize);
}

STDMETHODIMP CPyCOMTest::SetULongLongSafeArray(SAFEARRAY *ints, int *resultSize)
{
    return _SetArrayHelper(VT_UI8, ints, resultSize);
}

STDMETHODIMP CPyCOMTest::SetVariantSafeArray(SAFEARRAY *vars, int *resultSize)
{
    return _SetArrayHelper(VT_VARIANT, vars, resultSize);
}

STDMETHODIMP CPyCOMTest::SetDoubleSafeArray(SAFEARRAY *vals, int *resultSize)
{
    return _SetArrayHelper(VT_R8, vals, resultSize);
}

STDMETHODIMP CPyCOMTest::SetFloatSafeArray(SAFEARRAY *vals, int *resultSize)
{
    return _SetArrayHelper(VT_R4, vals, resultSize);
}

STDMETHODIMP CPyCOMTest::ChangeDoubleSafeArray(SAFEARRAY **vals)
{
    UINT cDims = SafeArrayGetDim(*vals);
    if (cDims != 1) {
        return E_UNEXPECTED;
    }
    HRESULT hr;
    long ub = 0, lb = 0;
    SafeArrayGetUBound(*vals, 1, &ub);
    SafeArrayGetLBound(*vals, 1, &lb);
    for (long i = lb; i <= ub; i++) {
        double val;
        hr = SafeArrayGetElement(*vals, &i, &val);
        if (FAILED(hr))
            return hr;
        val *= 2;
        hr = SafeArrayPutElement(*vals, &i, &val);
        if (FAILED(hr))
            return hr;
    }
    return S_OK;
}

static HRESULT MakeFillIntArray(SAFEARRAY **ppRes, int len, VARENUM vt)
{
    HRESULT hr = S_OK;
    SAFEARRAY *psa;
    SAFEARRAYBOUND rgsabound[1] = {len, 0};
    psa = SafeArrayCreate(vt, 1, rgsabound);
    if (psa == NULL)
        return E_OUTOFMEMORY;
    long i;
    for (i = 0; i < len; i++) {
        if (S_OK != (hr = SafeArrayPutElement(psa, &i, &i))) {
            SafeArrayDestroy(psa);
            return hr;
        }
    }
    *ppRes = psa;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSafeArrays(SAFEARRAY **attrs, SAFEARRAY **attrs2, SAFEARRAY **ints)
{
    HRESULT hr;
    *attrs = *attrs2 = *ints = NULL;
    if (S_OK != (hr = MakeFillIntArray(attrs, 5, VT_I4)))
        return hr;
    if (S_OK != (hr = MakeFillIntArray(attrs2, 10, VT_I4))) {
        SafeArrayDestroy(*attrs);
        return hr;
    }
    if (S_OK != (hr = MakeFillIntArray(ints, 20, VT_I4))) {
        SafeArrayDestroy(*attrs);
        SafeArrayDestroy(*attrs2);
        return hr;
    }
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetByteArray(long sizeBytes, SAFEARRAY **array)
{
    SAFEARRAYBOUND bound = {static_cast<ULONG>(sizeBytes), 0};
    *array = SafeArrayCreate(VT_UI1, 1, &bound);
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSimpleSafeArray(SAFEARRAY **attrs) { return MakeFillIntArray(attrs, 10, VT_I4); }

STDMETHODIMP CPyCOMTest::CheckVariantSafeArray(SAFEARRAY **attrs, int *result)
{
    *result = 1;
    return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSimpleCounter(ISimpleCounter **counter)
{
    if (counter == NULL)
        return E_POINTER;
    typedef CComObject<CSimpleCounter> CCounter;

    *counter = new CCounter();
    if (*counter == NULL)
        return E_OUTOFMEMORY;
    (*counter)->AddRef();
    return S_OK;
}

STDMETHODIMP CPyCOMTest::SetVarArgs(SAFEARRAY *vararg)
{
    if (pLastArray) {
        SafeArrayDestroy(pLastArray);
        pLastArray = NULL;
    }
    return SafeArrayCopy(vararg, &pLastArray);
}

STDMETHODIMP CPyCOMTest::GetLastVarArgs(SAFEARRAY **result)
{
    if (result == NULL)
        return E_POINTER;
    if (!pLastArray)
        return E_FAIL;
    return SafeArrayCopy(pLastArray, result);
}

HRESULT CPyCOMTest::Fire(long nID)
{
    Lock();
    HRESULT hr = S_OK;
    IUnknown **pp = m_vec.begin();
    while (pp < m_vec.end() && hr == S_OK) {
        if (*pp != NULL) {
            CComQIPtr<IDispatch> pEvent = *pp;
            DISPID dispid;
            OLECHAR *names[] = {L"OnFire"};
            hr = pEvent->GetIDsOfNames(IID_NULL, names, 1, 0, &dispid);
            if (SUCCEEDED(hr)) {
                CComVariant v(nID);
                DISPPARAMS params = {&v, NULL, 1, 0};
                hr = pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
            }
            if (FAILED(hr))
                break;
            // call FireWithNamedParams a variety of ways.
            // See https://learn.microsoft.com/en-ca/previous-versions/windows/desktop/automat/passing-parameters
            // "Passing Parameters (Component Automation)" for details.

            OLECHAR *names2[] = {L"OnFireWithNamedParams"};
            hr = pEvent->GetIDsOfNames(IID_NULL, names2, 1, 0, &dispid);
            if (SUCCEEDED(hr)) {
                // First call without named params - order is reversed
                // (ie, last in array is first presented to Python.)
                LONG out_result1 = nID + 1;
                LONG out_result2 = nID + 2;
                CComVariant v[4];
                // the "out2" outVal;
                V_VT(&v[0]) = VT_I4 | VT_BYREF;
                v[0].plVal = &out_result2;
                // the "out1" outVal;
                V_VT(&v[1]) = VT_I4 | VT_BYREF;
                v[1].plVal = &out_result1;
                // the bool
                v[2] = VARIANT_TRUE;  // the bool
                V_VT(&v[2]) = VT_BOOL;
                // the first param.
                v[3] = nID;
                DISPPARAMS params = {v, NULL, 4, 0};
                hr = pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                // all known impls return these values in the out pointer.
                _ASSERTE(out_result1 == nID + 3);
                _ASSERTE(out_result2 == nID + 4);
            }
            // Now with various combinations of named args.  Like Python, this
            // assumes that param DISPIDs start with zero, are sequential and
            // in the same order as the IDL signature.
            if (SUCCEEDED(hr)) {
                // Call again - this time with named params.
                LONG out_result1 = nID + 1;
                LONG out_result2 = nID + 2;
                CComVariant v[4];

                // the "out2" outVal;
                V_VT(&v[3]) = VT_I4 | VT_BYREF;
                v[3].plVal = &out_result2;
                // the "out1" outVal;
                V_VT(&v[2]) = VT_I4 | VT_BYREF;
                v[2].plVal = &out_result1;
                // the bool
                v[1] = VARIANT_TRUE;  // the bool
                V_VT(&v[1]) = VT_BOOL;
                // the first param.
                v[0] = nID;
                // Build 210 and earlier, this was the only way params *could* be passed,
                // which happily was the same way MSOffice did it.
                DISPID namedIDs[4] = {0, 1, 2, 3};
                DISPPARAMS params = {v, namedIDs, 4, 4};
                hr = pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                // all known impls return nID+1 in the out pointer.
                _ASSERTE(out_result1 == nID + 3);
                _ASSERTE(out_result2 == nID + 4);
            }
            // Try some other funky combinations to mess with Python :)
            if (SUCCEEDED(hr)) {
                // First 2 positional, 2nd 2 by name.
                LONG out_result1 = nID + 1;
                LONG out_result2 = nID + 2;

                CComVariant v[4];
                // the first param.
                v[3] = nID;
                // 2nd positional
                v[2] = VARIANT_TRUE;  // the bool
                V_VT(&v[2]) = VT_BOOL;
                // named ones up front.

                // the "out2" outVal (dispid=3)
                V_VT(&v[1]) = VT_I4 | VT_BYREF;
                v[1].plVal = &out_result2;
                // the "out1" outVal (dispid=2)
                V_VT(&v[0]) = VT_I4 | VT_BYREF;
                v[0].plVal = &out_result1;

                DISPID namedIDs[2] = {2, 3};
                DISPPARAMS params = {v, namedIDs, 4, 2};
                hr = pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                // all known impls return nID+1 in the out pointer.
                _ASSERTE(out_result1 == nID + 3);
                _ASSERTE(out_result2 == nID + 4);
            }

            if (SUCCEEDED(hr)) {
                // Only pass the 2 out params - Python must ensure earlier
                // ones are also passed.
                LONG out_result1 = nID + 1;
                LONG out_result2 = nID + 2;

                CComVariant v[4];
                // the "out2" outVal (dispid=3)
                V_VT(&v[0]) = VT_I4 | VT_BYREF;
                v[0].plVal = &out_result2;
                // the "out1" outVal (dispid=2)
                V_VT(&v[1]) = VT_I4 | VT_BYREF;
                v[1].plVal = &out_result1;

                DISPID namedIDs[2] = {3, 2};
                DISPPARAMS params = {v, namedIDs, 2, 2};

                hr = pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                // all known impls return nID+1 in the out pointer.
                _ASSERTE(out_result1 == nID + 3);
                _ASSERTE(out_result2 == nID + 4);
            }
            //			IPyCOMTestEvent* pIEvent = (IPyCOMTestEvent*)*pp;
            //			hr = pIEvent->Fire(nID);
        }
        pp++;
    }
    Unlock();
    _ASSERTE(SUCCEEDED(hr));
    return hr;
}

HRESULT CPyCOMTest::FireWithNamedParams(long nID, VARIANT_BOOL b, int *outVal1, int *outVal2)
{
    _ASSERTE(b == VARIANT_TRUE);
    _ASSERTE(nID + 1 == *outVal1);
    _ASSERTE(nID + 2 == *outVal2);
    *outVal1 = (int)nID + 3;
    *outVal2 = (int)nID + 4;
    return S_OK;
}

HRESULT CPyCOMTest::TestOptionals(BSTR strArg, short sarg, long larg, double darg, SAFEARRAY **pRet)
{
    HRESULT hr = S_OK;
    SAFEARRAY *psa;
    SAFEARRAYBOUND rgsabound[1] = {4, 0};
    psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
    CComVariant v(strArg);
    long ix[1];
    ix[0] = 0;
    SafeArrayPutElement(psa, ix, &v);
    v = sarg;
    ix[0] = 1;
    SafeArrayPutElement(psa, ix, &v);
    v = larg;
    ix[0] = 2;
    SafeArrayPutElement(psa, ix, &v);
    v = darg;
    ix[0] = 3;
    SafeArrayPutElement(psa, ix, &v);
    *pRet = psa;
    return hr;
}

HRESULT CPyCOMTest::TestOptionals2(double dval, BSTR strval, short sval, SAFEARRAY **pRet)
{
    HRESULT hr = S_OK;
    SAFEARRAY *psa;
    SAFEARRAYBOUND rgsabound[1] = {3, 0};
    psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
    long ix[1];
    CComVariant v(dval);
    ix[0] = 0;
    SafeArrayPutElement(psa, ix, &v);
    v = strval;
    ix[0] = 1;
    SafeArrayPutElement(psa, ix, &v);
    v = sval;
    ix[0] = 2;
    SafeArrayPutElement(psa, ix, &v);
    *pRet = psa;
    return hr;
}

HRESULT CPyCOMTest::TestOptionals3(double dval, short sval, IPyCOMTest **outinterface2) { return S_OK; }

HRESULT CPyCOMTest::GetStruct(TestStruct1 *ret)
{
    TestStruct1 r;
    r.int_value = 99;
    r.str_value = SysAllocString(L"Hello from C++");
    *ret = r;
    return S_OK;
}

HRESULT CPyCOMTest::ModifyStruct(TestStruct1 *prec)
{
    prec->int_value = 100;
    prec->str_value = SysAllocString(L"Nothing is as constant as change");
    return S_OK;
}

HRESULT CPyCOMTest::VerifyArrayOfStructs(TestStruct2 *prec, VARIANT_BOOL *is_ok)
{
    long i;
    TestStruct1 *pdata = NULL;
    HRESULT hr;

    hr = SafeArrayAccessData(prec->array_of_records, reinterpret_cast<void **>(&pdata));
    if (FAILED(hr)) {
        return E_FAIL;
    }
    *is_ok = VARIANT_TRUE;
    for (i = 0; i < prec->rec_count; i++) {
        if (_wcsicmp(pdata[i].str_value, L"This is record number") != 0 || pdata[i].int_value != i + 1) {
            *is_ok = VARIANT_FALSE;
            break;
        }
    }
    return S_OK;
}

HRESULT CPyCOMTest::DoubleString(BSTR in, BSTR *out)
{
    *out = SysAllocStringLen(NULL, SysStringLen(in) * 2);
    wcscpy(*out, in);
    wcscat(*out, in);
    return S_OK;
}

HRESULT CPyCOMTest::DoubleInOutString(BSTR *inout)
{
    BSTR newStr = SysAllocStringLen(NULL, SysStringLen(*inout) * 2);
    wcscpy(newStr, *inout);
    wcscat(newStr, *inout);
    SysFreeString(*inout);
    *inout = newStr;
    return S_OK;
}

#define CHECK_HR(_hr)                                                             \
    if (FAILED(hr = _hr)) {                                                       \
        printf("PyCOMTest: Failed at '%s', line %d: %d", __FILE__, __LINE__, hr); \
        return hr;                                                                \
    }

#define CHECK_TRUE(v)                                                                          \
    if (!(v)) {                                                                                \
        printf("PyCOMTest: Test value failed:%s\nAt '%s', line %d\n", #v, __FILE__, __LINE__); \
        return E_UNEXPECTED;                                                                   \
    }

HRESULT CPyCOMTest::TestMyInterface(IUnknown *unktester)
{
    if (!unktester)
        return E_POINTER;
    CComQIPtr<IPyCOMTest, &IID_IPyCOMTest> tester(unktester);
    if (!tester)
        return E_NOINTERFACE;
    HRESULT hr;

    // TEST
    QsBoolean i = 0, o = 0;
    CComVariant var(99);
    CHECK_HR(tester->Test(var, i, &o));
    CHECK_TRUE(o);
    i = 1, o = 1;
    CHECK_HR(tester->Test(var, i, &o));
    CHECK_TRUE(!o);

    // TEST2
    QsAttribute ret_attr;
    QsAttribute attr = Attr1;
    CHECK_HR(tester->Test2(attr, &ret_attr));
    CHECK_TRUE(attr == ret_attr);

    attr = Attr3;
    CHECK_HR(tester->Test2(attr, &ret_attr));
    CHECK_TRUE(attr == ret_attr);

    // TEST6
    QsAttributeWide ret_wideAttr;
    QsAttributeWide wideAttr;

    wideAttr = WideAttr1;
    CHECK_HR(tester->Test6(wideAttr, &ret_wideAttr));
    CHECK_TRUE(wideAttr == ret_wideAttr);

    wideAttr = WideAttr2;
    CHECK_HR(tester->Test6(wideAttr, &ret_wideAttr));
    CHECK_TRUE(wideAttr == ret_wideAttr);

    wideAttr = WideAttr3;
    CHECK_HR(tester->Test6(wideAttr, &ret_wideAttr));
    CHECK_TRUE(wideAttr == ret_wideAttr);

    wideAttr = WideAttr4;
    CHECK_HR(tester->Test6(wideAttr, &ret_wideAttr));
    CHECK_TRUE(wideAttr == ret_wideAttr);

    wideAttr = WideAttr5;
    CHECK_HR(tester->Test6(wideAttr, &ret_wideAttr));
    CHECK_TRUE(wideAttr == ret_wideAttr);

    // TEST5
    TestAttributes1 tattr = TestAttr1;
    CHECK_HR(tester->Test5(&tattr));
    CHECK_TRUE(tattr == TestAttr1_1);
    tattr = TestAttr1_1;
    CHECK_HR(tester->Test5(&tattr));
    CHECK_TRUE(tattr == TestAttr1);

    float fval = 2.0;
    long lval = 4;
    i = VARIANT_TRUE;
    CHECK_HR(tester->TestInOut(&fval, &i, &lval));
    CHECK_TRUE(fval == 4.0);
    CHECK_TRUE(lval == 8);
    CHECK_TRUE(i == VARIANT_FALSE);
    CHECK_HR(tester->TestInOut(&fval, &i, &lval));
    CHECK_TRUE(fval == 8.0);
    CHECK_TRUE(lval == 16);
    CHECK_TRUE(i == VARIANT_TRUE);

    // STRINGS
    CComBSTR instr("Foo");
    CComBSTR outstr;
    CHECK_HR(tester->DoubleString(instr, &outstr));
    CHECK_TRUE(outstr == L"FooFoo");

    instr = L"Foo";
    CHECK_HR(tester->TestByRefString(&instr));
    CHECK_TRUE(instr == L"FooFoo");

    // Arrays
    int result;
    SAFEARRAY *array;
    CHECK_HR(MakeFillIntArray(&array, 5, VT_INT));
    CHECK_HR(tester->CheckVariantSafeArray(&array, &result));
    CHECK_TRUE(result == 1);

    CHECK_HR(tester->SetIntSafeArray(array, &result));

    SafeArrayDestroy(array);

    CHECK_HR(MakeFillIntArray(&array, 5, VT_I8));
    CHECK_HR(tester->CheckVariantSafeArray(&array, &result));
    CHECK_TRUE(result == 1);
    CHECK_HR(tester->SetLongLongSafeArray(array, &result));
    SafeArrayDestroy(array);

    CHECK_HR(MakeFillIntArray(&array, 5, VT_UI8));
    CHECK_HR(tester->CheckVariantSafeArray(&array, &result));
    CHECK_TRUE(result == 1);
    CHECK_HR(tester->SetULongLongSafeArray(array, &result));
    SafeArrayDestroy(array);

    long lresult;
    CHECK_HR(tester->put_LongProp(4));
    CHECK_HR(tester->get_LongProp(&lresult));
    CHECK_TRUE(lresult == 4);
    CHECK_HR(tester->put_LongProp(-4));
    CHECK_HR(tester->get_LongProp(&lresult));
    CHECK_TRUE(lresult == -4);
    unsigned long ulresult;
    CHECK_HR(tester->put_ULongProp(0x80000001));
    CHECK_HR(tester->get_ULongProp(&ulresult));
    CHECK_TRUE(ulresult == 0x80000001);

    CHECK_HR(tester->put_IntProp(4));
    CHECK_HR(tester->get_IntProp(&result));
    CHECK_TRUE(result == 4);
    CY cy = {123, 456};
    CY cresult;
    CHECK_HR(tester->put_CurrencyProp(cy));
    CHECK_HR(tester->get_CurrencyProp(&cresult));
    CHECK_TRUE(cresult.int64 == cy.int64);

    // interface tests
    CComPtr<IPyCOMTest> param(tester);
    CComPtr<IPyCOMTest> obresult;
    CHECK_HR(tester->GetSetInterface(param, &obresult));

    VARIANT v1, v2;
    VariantInit(&v1);
    VariantInit(&v2);
    V_VT(&v1) = VT_I4;
    V_I4(&v1) = 99;
    CHECK_HR(tester->GetSetVariant(v1, &v2));
    CHECK_TRUE(V_VT(&v2) == VT_I4);
    CHECK_TRUE(V_I4(&v2) == 99);
    CHECK_HR(tester->TestByRefVariant(&v2));
    CHECK_TRUE(V_VT(&v2) == VT_I4);
    CHECK_TRUE(V_I4(&v2) == 198);
    VariantClear(&v1);
    VariantClear(&v2);

    // Make a vtable call on the returned object, so we
    // crash if a bad vtable.  Don't care about the value tho.
    CHECK_HR(obresult->get_IntProp(&result));

    return S_OK;
}

HRESULT CPyCOMTest::EarliestDate(DATE first, DATE second, DATE *pResult)
{
    if (!pResult)
        return E_POINTER;
    *pResult = first <= second ? first : second;
    return S_OK;
}

HRESULT CPyCOMTest::MakeDate(double val, DATE *pResult)
{
    if (!pResult)
        return E_POINTER;
    *pResult = (DATE)val;
    return S_OK;
}

HRESULT CPyCOMTest::TestQueryInterface()
{
    IUnknown *pObj = 0;
    IPyCOMTest *pCOMTest = 0;
    HRESULT hr = S_OK;

    MULTI_QI mqi[1] = {&IID_IUnknown, NULL, E_FAIL};

    COSERVERINFO server = {(DWORD)0, 0, (COAUTHINFO *)NULL, (DWORD)0};

    // Create an instance of the test server
    hr = CoCreateInstanceEx(CLSID_PythonTestPyCOMTest, NULL, CLSCTX_LOCAL_SERVER, &server, 1, mqi);
    if (FAILED(hr)) {
        goto exit;
    }
    pObj = mqi[0].pItf;

    // Query for the custom interface
    hr = pObj->QueryInterface(IID_IPyCOMTest, (LPVOID *)&pCOMTest);
    if (FAILED(hr)) {
        goto exit;
    }

    hr = S_OK;

exit:
    if (pObj) {
        pObj->Release();
        pObj = 0;
    }
    if (pCOMTest) {
        pCOMTest->Release();
        pCOMTest = 0;
    }
    return hr;
}

HRESULT CPyCOMTest::DoubleCurrencyByVal(CY *v)
{
    v->int64 *= 2;
    return S_OK;
}

HRESULT CPyCOMTest::DoubleCurrency(CY v, CY *ret)
{
    ret->int64 = v.int64 * 2;
    return S_OK;
}

HRESULT CPyCOMTest::AddCurrencies(CY v1, CY v2, CY *pret)
{
    pret->int64 = v1.int64 + v2.int64;
    return S_OK;
}

HRESULT CPyCOMTest::NotScriptable(int *val)
{
    (*val)++;
    return S_OK;
}

HRESULT CPyCOMTest::put_LongProp(long val)
{
    m_long = val;
    return S_OK;
}

HRESULT CPyCOMTest::get_LongProp(long *ret)
{
    if (!ret)
        return E_POINTER;
    *ret = m_long;
    return S_OK;
}

HRESULT CPyCOMTest::put_ULongProp(unsigned long val)
{
    m_ulong = val;
    return S_OK;
}

HRESULT CPyCOMTest::get_ULongProp(unsigned long *ret)
{
    if (!ret)
        return E_POINTER;
    *ret = m_ulong;
    return S_OK;
}

HRESULT CPyCOMTest::put_IntProp(int val)
{
    m_long = val;
    return S_OK;
}

HRESULT CPyCOMTest::get_IntProp(int *ret)
{
    if (!ret)
        return E_POINTER;
    *ret = (int)m_long;
    return S_OK;
}

HRESULT CPyCOMTest::put_CurrencyProp(CY val)
{
    m_cy = val;
    return S_OK;
}

HRESULT CPyCOMTest::get_CurrencyProp(CY *ret)
{
    if (!ret)
        return E_POINTER;
    *ret = (CY)m_cy;
    return S_OK;
}

HRESULT CPyCOMTest::get_ParamProp(int which, int *ret)
{
    if (!ret)
        return E_POINTER;
    *ret = which == 0 ? m_paramprop1 : m_paramprop2;
    return S_OK;
}

HRESULT CPyCOMTest::put_ParamProp(int which, int val)
{
    if (which == 0)
        m_paramprop1 = val;
    else
        m_paramprop2 = val;
    return S_OK;
}

HRESULT CPyCOMTest::None() { return S_OK; }

HRESULT CPyCOMTest::def() { return S_OK; }
