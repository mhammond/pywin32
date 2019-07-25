
class CSimpleCounter : public CComDualImpl<ISimpleCounter, &IID_ISimpleCounter, &LIBID_PyCOMTestLib>,
                       public ISupportErrorInfo,
                       public CComObjectRoot,
                       public CComCoClass<CSimpleCounter, &CLSID_CoSimpleCounter> {
   public:
    BEGIN_COM_MAP(CSimpleCounter)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISimpleCounter)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()
    DECLARE_REGISTRY_RESOURCEID(IDR_PYCOMTEST)
    //   DECLARE_NOT_AGGREGATABLE(CSimpleCounter)

    // CSimpleCounter methods
    STDMETHOD(get_Count)(long *retval);
    STDMETHOD(get_Item)(long Index, VARIANT *retval);
    STDMETHOD(get__NewEnum)(IUnknown **retval);

    STDMETHOD(get_LBound)(long *lbound);
    STDMETHOD(put_LBound)(long lbound);
    STDMETHOD(get_UBound)(long *ubound);
    STDMETHOD(put_UBound)(long ubound);

    STDMETHOD(GetBounds)(long *lbound, long *ubound);
    STDMETHOD(SetBounds)(long lbound, long ubound);
    STDMETHOD(put_TestProperty)(long propval1, long propval2) { return S_OK; }
    STDMETHOD(put_TestProperty2)(long propval1, long propval2, long propval3) { return S_OK; }
    STDMETHOD(get_TestPropertyWithDef)(long arg, long *ret)
    {
        *ret = arg;
        return S_OK;
    }
    STDMETHOD(get_TestPropertyNoDef)(long arg, long *ret)
    {
        *ret = arg;
        return S_OK;
    }
    // ISupportErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) { return (IID_ISimpleCounter == riid) ? S_OK : S_FALSE; }

    // helpers
   public:
    //   void Add(CComVariant& var) { m_VarVect.insert(m_VarVect.end(), var); }
    CSimpleCounter();

   protected:
    // internal data
    long m_minIndex;
    long m_maxIndex;
    // os_vector<CComVariant> m_VarVect;
};
