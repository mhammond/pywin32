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
DEFINE_GUID(CLSID_PythonTestPyCOMTest,
0xe743d9cd, 0xcb03, 0x4b04, 0xb5, 0x16, 0x11, 0xd3, 0xa8, 0x1c, 0x15, 0x97);

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

DWORD WINAPI PyCOMTestSessionThreadEntry(void* pv)
{
	// Init COM for the thread.
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CPyCOMTest::PyCOMTestSessionData* pS = (CPyCOMTest::PyCOMTestSessionData*)pv;
	// Unmarshal the interface pointer.
	IPyCOMTest *pi;
	HRESULT hr = CoGetInterfaceAndReleaseStream(pS->pStream, IID_IPyCOMTest, (void **)&pi);
	CComPtr<IPyCOMTest> p(pi);
	while (WaitForSingleObject(pS->m_hEvent, 0) != WAIT_OBJECT_0)
		p->Fire(pS->m_nID);
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

void CPyCOMTest::CreatePyCOMTestSession(PyCOMTestSessionData& rs)
{
	DWORD dwThreadID = 0;
	_ASSERTE(rs.m_hEvent == NULL);
	_ASSERTE(rs.m_hThread == NULL);
	_ASSERTE(rs.pStream == NULL);
	rs.m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_IPyCOMTest, (IPyCOMTest *)this, &rs.pStream );
	_ASSERTE(SUCCEEDED(hr) && rs.pStream != NULL);
	rs.m_hThread = CreateThread(NULL, 0, &PyCOMTestSessionThreadEntry, &rs, 0, &dwThreadID);
}

STDMETHODIMP CPyCOMTest::Start(long* pnID)
{
	if (pnID == NULL)
		return E_POINTER;
	*pnID = 0;
	HRESULT hRes = S_OK;
	m_cs.Lock();
	for (long i=0;i<nMaxSessions;i++)
	{
		if (m_rsArray[i].m_hEvent == NULL)
		{
			m_rsArray[i].m_nID = i;
			CreatePyCOMTestSession(m_rsArray[i]);
			*pnID = i;
			break;
		}
	}
	if (i == nMaxSessions) //fell through
		hRes = E_FAIL;
	m_cs.Unlock();
	return hRes;
}


STDMETHODIMP CPyCOMTest::Stop(long nID)
{
	HRESULT hRes = S_OK;
	m_cs.Lock();
	if (m_rsArray[nID].m_hEvent != NULL)
	{
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
	for (long i=0;i<nMaxSessions;i++)
	{
		if (m_rsArray[i].m_hEvent != NULL)
		{
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
STDMETHODIMP CPyCOMTest::Test3(TestAttributes1 in, TestAttributes1* out)
{
	*out = in;
	return S_OK;
}
STDMETHODIMP CPyCOMTest::Test4(TestAttributes2 in,TestAttributes2* out)
{
	*out = in;
	return S_OK;
}

STDMETHODIMP CPyCOMTest::Test5(TestAttributes1 *inout)
{
	return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetInterface(IPyCOMTest *ininterface, IPyCOMTest **outinterface)
{
	if (outinterface==NULL) return E_POINTER;
	*outinterface = ininterface;
	// Looks like I should definately AddRef() :-)
	ininterface->AddRef();
	return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSetInterfaceArray(SAFEARRAY *pin, SAFEARRAY **pout)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPyCOMTest::GetMultipleInterfaces(IPyCOMTest **outinterface1, IPyCOMTest **outinterface2)
{
	if (outinterface1==NULL || outinterface2==NULL) return E_POINTER;
	*outinterface1 = this;
	*outinterface2 = this;
	InternalAddRef(); // ??? Correct call?  AddRef fails compile...
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

STDMETHODIMP CPyCOMTest::TakeByRefTypedDispatch(IPyCOMTest **inout)
{
	return S_OK;
}
STDMETHODIMP CPyCOMTest::TakeByRefDispatch(IDispatch **inout)
{
	return S_OK;
}

STDMETHODIMP CPyCOMTest::SetIntSafeArray(SAFEARRAY* ints, int *resultSize)
{
	TCHAR buf[256];
	UINT cDims = SafeArrayGetDim(ints);
	*resultSize = 0;
	long ub=0, lb=0;
	if (cDims) {
		SafeArrayGetUBound(ints, 1, &ub);
		SafeArrayGetLBound(ints, 1, &lb);
		*resultSize = ub - lb + 1;
	}
	wsprintf(buf, _T("Have VARIANT SafeArray with %d dims and size %d\n"), cDims, *resultSize);
	OutputDebugString(buf);
	return S_OK;
}

STDMETHODIMP CPyCOMTest::SetVariantSafeArray(SAFEARRAY* vars, int *resultSize)
{
	TCHAR buf[256];
	UINT cDims = SafeArrayGetDim(vars);
	*resultSize = 0;
	long ub=0, lb=0;
	if (cDims) {
		SafeArrayGetUBound(vars, 1, &ub);
		SafeArrayGetLBound(vars, 1, &lb);
		*resultSize = ub - lb + 1;
	}
	wsprintf(buf, _T("Have VARIANT SafeArray with %d dims and size %d\n"), cDims, *resultSize);
	OutputDebugString(buf);
	return S_OK;
}

static HRESULT MakeFillIntArray(SAFEARRAY **ppRes, int len, VARENUM vt)
{
	HRESULT hr = S_OK;
	SAFEARRAY *psa;
	SAFEARRAYBOUND rgsabound[1] = { len, 0 };
	psa = SafeArrayCreate(VT_I4, 1, rgsabound);
	if (psa==NULL)
		return E_OUTOFMEMORY;
	long i;
	for (i=0;i<len;i++) {
		if (S_OK!=(hr=SafeArrayPutElement(psa, &i, &i))) {
			SafeArrayDestroy(psa);
			return hr;
		}
	}
	*ppRes = psa;
	return S_OK;
	}

STDMETHODIMP CPyCOMTest::GetSafeArrays(SAFEARRAY** attrs,
                                      SAFEARRAY**attrs2,
                                      SAFEARRAY** ints)
{
	HRESULT hr;
	*attrs = *attrs2 = *ints = NULL;
	if (S_OK != (hr=MakeFillIntArray(attrs, 5, VT_I4)))
		return hr;
	if (S_OK != (hr=MakeFillIntArray(attrs2, 10, VT_I4))) {
		SafeArrayDestroy(*attrs);
		return hr;
	}
	if (S_OK != (hr=MakeFillIntArray(ints, 20, VT_I4))) {
		SafeArrayDestroy(*attrs);
		SafeArrayDestroy(*attrs2);
		return hr;
	}
	return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSimpleSafeArray(SAFEARRAY** attrs)
{
	return MakeFillIntArray(attrs, 10, VT_I4);
}

STDMETHODIMP CPyCOMTest::CheckVariantSafeArray(SAFEARRAY** attrs, int *result)
{
	*result = 1;
	return S_OK;
}

STDMETHODIMP CPyCOMTest::GetSimpleCounter(ISimpleCounter** counter)
{
	if (counter==NULL) return E_POINTER;
	typedef CComObject<CSimpleCounter> CCounter;

	*counter = new CCounter();
	(*counter)->AddRef();
	if (*counter==NULL) return E_OUTOFMEMORY;
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
	if (result==NULL) return E_POINTER;
	if (!pLastArray)
		return E_FAIL;
	return SafeArrayCopy(pLastArray, result);
}

HRESULT CPyCOMTest::Fire(long nID)
{
	Lock();
	HRESULT hr = S_OK;
	IUnknown** pp = m_vec.begin();
	while (pp < m_vec.end() && hr == S_OK)
	{
		if (*pp != NULL)
		{
			CComQIPtr<IDispatch> pEvent = *pp;
			DISPID dispid;
			OLECHAR *names[] = { L"OnFire" };
			HRESULT hr = pEvent->GetIDsOfNames(IID_NULL, names, 1, 0, &dispid);
			if (SUCCEEDED(hr)) {
				CComVariant v(nID);
				DISPPARAMS params = { &v, NULL, 1, 0 };
				pEvent->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, NULL);
			}
//			IPyCOMTestEvent* pIEvent = (IPyCOMTestEvent*)*pp;
//			hr = pIEvent->Fire(nID);
		}
		pp++;
	}
	Unlock();
	return hr;
}

HRESULT CPyCOMTest::TestOptionals(BSTR strArg, short sarg, long larg, double darg, SAFEARRAY **pRet) 
{
	HRESULT hr = S_OK;
	SAFEARRAY *psa;
	SAFEARRAYBOUND rgsabound[1] = { 4, 0 };
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
	SAFEARRAYBOUND rgsabound[1] = { 3, 0 };
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

HRESULT CPyCOMTest::GetStruct(TestStruct1 *ret)
{
	TestStruct1 r;
	r.int_value = 99;
	r.str_value = SysAllocString(L"Hello from C++");
	*ret = r;
	return S_OK;
}
HRESULT CPyCOMTest::DoubleString(BSTR in, BSTR *out)
{
	*out = SysAllocStringLen(NULL, SysStringLen(in)*2);
	wcscpy(*out, in);
	wcscat(*out, in);
	return S_OK;
}

HRESULT CPyCOMTest::DoubleInOutString(BSTR *inout)
{
	BSTR newStr = SysAllocStringLen(NULL, SysStringLen(*inout)*2);
	wcscpy(newStr, *inout);
	wcscat(newStr, *inout);
	SysFreeString(*inout);
	*inout = newStr;
	return S_OK;
}

#define CHECK_HR(_hr) if (FAILED(hr=_hr)) { \
	printf("PyCOMTest: Failed at '%s', line %d: %d", __FILE__, __LINE__, hr); \
	return hr; \
}

#define CHECK_TRUE(v) if (!(v)) { \
	printf("PyCOMTest: Test value failed:%s\nAt '%s', line %d\n", #v, __FILE__, __LINE__); \
	return E_UNEXPECTED; \
}

HRESULT CPyCOMTest::TestMyInterface( IUnknown *unktester)
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
	CHECK_HR(tester->Test( var, i, &o));
	CHECK_TRUE( o );
	i = 1, o = 1;
	CHECK_HR(tester->Test( var, i, &o));
	CHECK_TRUE( !o );

	// TEST2
	QsAttribute ret_attr;
	QsAttribute attr = Attr1;
	CHECK_HR(tester->Test2( attr, &ret_attr));
	CHECK_TRUE( attr == ret_attr );

	attr = Attr3;
	CHECK_HR(tester->Test2( attr, &ret_attr));
	CHECK_TRUE( attr == ret_attr );

	// TEST5
	TestAttributes1 tattr = TestAttr1;
	CHECK_HR(tester->Test5( &tattr ));
	CHECK_TRUE( tattr == TestAttr1_1 );
	tattr = TestAttr1_1;
	CHECK_HR(tester->Test5( &tattr ));
	CHECK_TRUE( tattr == TestAttr1 );

	// STRINGS
	CComBSTR instr("Foo");
	CComBSTR outstr;
	CHECK_HR(tester->DoubleString(instr, &outstr));
	CHECK_TRUE(outstr == L"FooFoo");

	// Arrays
	int result;
	SAFEARRAY *array;
	CHECK_HR(MakeFillIntArray(&array, 5, VT_INT));
	CHECK_HR(tester->CheckVariantSafeArray(&array, &result));
	CHECK_TRUE(result==1);

	CHECK_HR(tester->SetIntSafeArray(array, &result));

	SafeArrayDestroy(array);

	return S_OK;
}

HRESULT CPyCOMTest::EarliestDate(DATE first, DATE second, DATE *pResult)
{
	if (!pResult)
		return E_POINTER;
	*pResult = first <= second ? first : second;
	return S_OK;
}

HRESULT CPyCOMTest::TestQueryInterface()
{
	IUnknown* pObj = 0;
	IPyCOMTest * pCOMTest = 0;
	HRESULT hr = S_OK;

	MULTI_QI mqi[1] = {
		&IID_IUnknown, NULL, E_FAIL
	};

	COSERVERINFO server = {
		(DWORD)0, 0, (COAUTHINFO*)NULL, (DWORD)0
	};

	// Create an instance of the test server
	hr = CoCreateInstanceEx(CLSID_PythonTestPyCOMTest, NULL, CLSCTX_LOCAL_SERVER, &server, 1, mqi);
	if (FAILED(hr)) { goto exit; }
	pObj = mqi[0].pItf;

	// Query for the custom interface
	hr = pObj->QueryInterface(IID_IPyCOMTest, (LPVOID*)&pCOMTest);
	if (FAILED(hr)) { goto exit; }

	hr = S_OK;

exit:
	if (pObj)     { pObj->Release(); pObj = 0; }
	if (pCOMTest) { pCOMTest->Release(); pCOMTest = 0; }
	return hr;
}


HRESULT CPyCOMTest::NotScriptable(int *val)
{
	(*val) ++;
	return S_OK;
}
