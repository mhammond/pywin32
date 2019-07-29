// DSCArrayTest.h : Declaration of the DSCArrayTest

#ifndef __ARRAYTEST_H_
#define __ARRAYTEST_H_

//#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// DSCArrayTest
class ATL_NO_VTABLE DSCArrayTest : public CComObjectRootEx<CComSingleThreadModel>,
                                   public CComCoClass<DSCArrayTest, &CLSID_ArrayTest>,
                                   public ISupportErrorInfo,
                                   public IDispatchImpl<IArrayTest, &IID_IArrayTest, &LIBID_PyCOMTestLib> {
   public:
    DECLARE_REGISTRY_RESOURCEID(IDR_ARRAYTEST)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(DSCArrayTest)
    COM_INTERFACE_ENTRY(IArrayTest)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    DSCArrayTest();
    void FinalRelease();

    // ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    // IArrayTest
   public:
    STDMETHOD(ReturnSampleArray)(/*[out, retval]*/ VARIANT *pVal);
    STDMETHOD(get_Array)(/*[out, retval]*/ VARIANT *pVal);
    STDMETHOD(put_Array)(/*[in]*/ VARIANT newVal);

   private:
    CComVariant m_spvarcArray;
};

#endif  //__ARRAYTEST_H_
