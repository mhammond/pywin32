// ISimpleCounter.cpp :

#include "preconn.h"
#include "connres.h"
#include "initguid.h"
#include "PyCOMTest.h"
#include "PyCOMImpl.h"

#include "SimpleCounter.h"

CSimpleCounter::CSimpleCounter()
{
    m_minIndex = 1;  // have 1 based index, just cos Python doesnt!
    m_maxIndex = 10;
}
STDMETHODIMP CSimpleCounter::get_Count(long *retval)
{
    if (retval == NULL) {
        return E_POINTER;
    }
    *retval = m_maxIndex - m_minIndex + 1;
    return S_OK;
}

STDMETHODIMP CSimpleCounter::get_Item(long Index, VARIANT *retval)
{
    if (retval == NULL) {
        return E_POINTER;
    }
    VariantInit(retval);
    retval->vt = VT_I4;
    retval->lVal = 0;
    // use 1-based index, VB like
    if ((Index < m_minIndex) || (Index > m_maxIndex)) {
        return E_INVALIDARG;
    }
    retval->lVal = Index;
    return S_OK;
}

STDMETHODIMP CSimpleCounter::get__NewEnum(IUnknown **retval)
{
    if (retval == NULL) {
        return E_POINTER;
    }
    *retval = NULL;
    typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;
    enumvar *p = new enumvar;
    _ASSERTE(p);

    long numElems = m_maxIndex - m_minIndex + 1;
    VARIANT *pVars = new VARIANT[numElems];
    long offset = 0;
    long i = m_minIndex;
    for (; i <= m_maxIndex; i++, offset++) {
        VariantInit(pVars + offset);
        pVars[offset].vt = VT_I4;
        pVars[offset].lVal = i;
    }

    HRESULT hRes = p->Init(pVars, pVars + numElems, NULL, AtlFlagTakeOwnership);
    if (SUCCEEDED(hRes)) {
        hRes = p->QueryInterface(IID_IEnumVARIANT, (void **)retval);
    }
    if (FAILED(hRes)) {
        delete p;
    }
    return hRes;
}

STDMETHODIMP CSimpleCounter::get_LBound(long *lbound)
{
    if (lbound == NULL) {
        return E_POINTER;
    }
    *lbound = m_minIndex;
    return S_OK;
}
STDMETHODIMP CSimpleCounter::put_LBound(long lbound)
{
    if (lbound > m_maxIndex)
        return E_INVALIDARG;
    m_minIndex = lbound;
    return S_OK;
}

STDMETHODIMP CSimpleCounter::get_UBound(long *ubound)
{
    if (ubound == NULL) {
        return E_POINTER;
    }
    *ubound = m_maxIndex;
    return S_OK;
}
STDMETHODIMP CSimpleCounter::put_UBound(long ubound)
{
    if (ubound < m_minIndex)
        return E_INVALIDARG;
    m_maxIndex = ubound;
    return S_OK;
}

STDMETHODIMP CSimpleCounter::GetBounds(long *lbound, long *ubound)
{
    if (lbound == NULL || ubound == NULL) {
        return E_POINTER;
    }
    *lbound = m_minIndex;
    *ubound = m_maxIndex;
    return S_OK;
}
STDMETHODIMP CSimpleCounter::SetBounds(long lbound, long ubound)
{
    if (ubound < lbound)
        return E_INVALIDARG;
    m_minIndex = lbound;
    m_maxIndex = ubound;
    return S_OK;
}
