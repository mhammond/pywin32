// PyCOMTest.h : Declaration of the CPyCOMTest


#include "connres.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPyCOMTest

const int nMaxSessions = 10;

//class CPyComTestImpl

class CPyCOMTest : 
	public IDispatchImpl<IPyCOMTest, &IID_IPyCOMTest, &LIBID_PyCOMTestLib>, 
//	public CComDualImpl<IPyCOMTest, &IID_IPyCOMTest, &LIBID_PyCOMTestLib>, 
	public IConnectionPointContainerImpl<CPyCOMTest>,
	public IConnectionPointImpl<CPyCOMTest, &IID_IPyCOMTestEvent, CComDynamicUnkArray>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CPyCOMTest,&CLSID_CoPyCOMTest>
{
public:
	CPyCOMTest() :
	  pLastArray(NULL)
	{
		memset(m_rsArray, 0, nMaxSessions*sizeof(PyCOMTestSessionData));
		m_cy.int64 = 0;
		m_long = 0;
	}
	~CPyCOMTest();

BEGIN_COM_MAP(CPyCOMTest)
	COM_INTERFACE_ENTRY2(IDispatch, IPyCOMTest)
	COM_INTERFACE_ENTRY(IPyCOMTest)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
//	DECLARE_NOT_AGGREGATABLE(CPyCOMTest) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation.  The default is to support it

	DECLARE_REGISTRY_RESOURCEID(IDR_PYCOMTEST)

// Connection Point
	BEGIN_CONNECTION_POINT_MAP(CPyCOMTest)
		CONNECTION_POINT_ENTRY(IID_IPyCOMTestEvent)
	END_CONNECTION_POINT_MAP()
	
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IPyCOMTest
	STDMETHOD(Start)(long* pnID);
	STDMETHOD(Stop)(long nID);
	STDMETHOD(StopAll)();
	STDMETHOD(Test)(VARIANT, QsBoolean, QsBoolean *);
	STDMETHOD(Test2)(QsAttribute, QsAttribute *);
	STDMETHOD(Test3)(TestAttributes1, TestAttributes1 *);
	STDMETHOD(Test4)(TestAttributes2, TestAttributes2 *);
	STDMETHOD(Test5)(TestAttributes1 *);
	STDMETHOD(GetSetInterface)(IPyCOMTest *ininterface, IPyCOMTest **outinterface);
	STDMETHOD(GetSetInterfaceArray)(SAFEARRAY *pin, SAFEARRAY **pout);

	STDMETHOD(GetMultipleInterfaces)(IPyCOMTest **outinterface1, IPyCOMTest **outinterface2);
	STDMETHOD(GetSetDispatch)(IDispatch *indisp, IDispatch **outdisp);
	STDMETHOD(GetSetUnknown)(IUnknown *inunk, IUnknown **outunk);
	STDMETHOD(TakeByRefTypedDispatch)(IPyCOMTest **inout);
	STDMETHOD(TakeByRefDispatch)(IDispatch **inout);
	STDMETHOD(SetIntSafeArray)(SAFEARRAY* ints, int *retSize);
	STDMETHOD(SetVariantSafeArray)(SAFEARRAY* vars, int *retSize);
	STDMETHOD(GetSafeArrays)(SAFEARRAY** attrs,
                                      SAFEARRAY**attrs2,
                                      SAFEARRAY** ints);
	STDMETHOD(GetSimpleSafeArray)(SAFEARRAY** ints);
	STDMETHOD(GetSimpleCounter)(ISimpleCounter** counter);
	STDMETHOD(CheckVariantSafeArray)(SAFEARRAY **vals, int *result);

	STDMETHOD(SetVarArgs)(SAFEARRAY *);
	STDMETHOD(GetLastVarArgs)(SAFEARRAY **);

	// method to broadcast a call on the current connections
	STDMETHOD(Fire)(long  nID);
	STDMETHOD(TestOptionals)(BSTR strArg, short sarg, long larg, double darg, SAFEARRAY **pRet);
	STDMETHOD(TestOptionals2)(double dval, BSTR strval, short sval, SAFEARRAY **pRet);
	STDMETHOD(GetStruct)(TestStruct1 *ret);
	STDMETHOD(DoubleString)(BSTR inStr, BSTR *outStr);
	STDMETHOD(DoubleInOutString)(BSTR *str);
	STDMETHOD(TestMyInterface)(IUnknown *t);
	STDMETHOD(EarliestDate)(DATE first, DATE second, DATE *pResult);
	STDMETHOD(TestQueryInterface)();
	STDMETHOD(NotScriptable)(int *val);
	STDMETHOD(get_LongProp)(long *ret);
	STDMETHOD(put_LongProp)(long val);
	STDMETHOD(get_IntProp)(int *ret);
	STDMETHOD(put_IntProp)(int val);
	STDMETHOD(get_CurrencyProp)(CY *ret);
	STDMETHOD(put_CurrencyProp)(CY val);

	// info associated to each session
	struct PyCOMTestSessionData
	{
		IStream* pStream; // Stream for marshalling the data to the new thread.
		HANDLE m_hEvent;
		HANDLE m_hThread;
		int m_nID;
	};
protected:

	PyCOMTestSessionData m_rsArray[nMaxSessions];
	void CreatePyCOMTestSession(PyCOMTestSessionData& rs);

	_ThreadModel::AutoCriticalSection m_cs;
	SAFEARRAY *pLastArray;
	long m_long;
	CY m_cy;
};

class CPyCOMTest2 : public CPyCOMTest
{
	STDMETHOD(TestDerived)(QsAttribute, QsAttribute *);
};

