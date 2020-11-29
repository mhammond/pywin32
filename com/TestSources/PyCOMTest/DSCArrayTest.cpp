// DSCArrayTest.cpp : Implementation of DSCArrayTest
#include "preconn.h"
#include "connres.h"
#include "initguid.h"
#include "PyCOMTest.h"
#include "PyCOMImpl.h"

#include "DSCArrayTest.h"

/////////////////////////////////////////////////////////////////////////////
// DSCArrayTest

DSCArrayTest::DSCArrayTest() {}

void DSCArrayTest::FinalRelease() { m_spvarcArray.Clear(); }

STDMETHODIMP DSCArrayTest::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID *arr[] = {&IID_IArrayTest};
    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
        if (*arr[i] == riid)
            return S_OK;
    }
    return S_FALSE;
}

HRESULT CreateVector(long lArraySize, VARIANT &rVarout)
{
    SAFEARRAY *pSafeArray;
    long lIndex;
    VARIANT varElement;
    pSafeArray = SafeArrayCreateVector(VT_VARIANT, 0, lArraySize);

    for (lIndex = 0; lIndex < lArraySize; lIndex++) {
        VariantInit(&varElement);
        varElement.vt = VT_R8;
        varElement.dblVal = double(lIndex);
        SafeArrayPutElement(pSafeArray, &lIndex, (void *)&varElement);
        VariantClear(&varElement);
    }
    VariantInit(&rVarout);
    rVarout.vt = VT_ARRAY | VT_VARIANT;
    rVarout.parray = pSafeArray;
    return S_OK;
}

HRESULT CreateMatrix(long lXSize, long lYSize, VARIANT &rVarout)
{
    SAFEARRAY *pSafeArray;
    SAFEARRAYBOUND ArrayBounds[2];
    long lXIndex;
    long lYIndex;
    long lDimensions;
    long lIndexArray[2];
    VARIANT varElement;

    // Number of dimensions
    lDimensions = 2;
    // Set the elements in each dimension
    ArrayBounds[0].cElements = lXSize;
    ArrayBounds[0].lLbound = 0;
    ArrayBounds[1].cElements = lYSize;
    ArrayBounds[1].lLbound = 0;

    // Create the two dimensional array
    pSafeArray = SafeArrayCreate(VT_VARIANT, lDimensions, ArrayBounds);

    for (lYIndex = 0; lYIndex < lYSize; lYIndex++) {
        for (lXIndex = 0; lXIndex < lXSize; lXIndex++) {
            VariantInit(&varElement);
            lIndexArray[0] = lXIndex;
            lIndexArray[1] = lYIndex;
            varElement.vt = VT_R8;
            varElement.dblVal = double(lXIndex * 100 + lYIndex);
            SafeArrayPutElement(pSafeArray, lIndexArray, (void *)&varElement);
            VariantClear(&varElement);
        }
    }
    VariantInit(&rVarout);
    rVarout.vt = VT_ARRAY | VT_VARIANT;
    rVarout.parray = pSafeArray;
    return S_OK;
}

STDMETHODIMP DSCArrayTest::get_Array(VARIANT *pVal)
{
    HRESULT hr = E_POINTER;
    if (NULL != pVal) {
        hr = ::VariantCopy(pVal, &m_spvarcArray);
    }
    return hr;
}

STDMETHODIMP DSCArrayTest::put_Array(VARIANT newVal)
{
    m_spvarcArray.Clear();
    m_spvarcArray = newVal;
    return S_OK;
}

STDMETHODIMP DSCArrayTest::ReturnSampleArray(VARIANT *pVal)
{
    SAFEARRAY *pSafeArray;
    long lIndex;
    VARIANT varXAxis;
    VARIANT varYAxis;
    VARIANT varZValue;

    pSafeArray = SafeArrayCreateVector(VT_VARIANT, 0, 3);
    VariantInit(&varXAxis);
    VariantInit(&varYAxis);
    VariantInit(&varZValue);
    // Retrieve the axis variant which is a vector
    CreateVector(3, varXAxis);
    CreateVector(5, varYAxis);

    CreateMatrix(3, 5, varZValue);
    lIndex = 0;
    SafeArrayPutElement(pSafeArray, &lIndex, (void *)&varXAxis);
    lIndex = 1;
    SafeArrayPutElement(pSafeArray, &lIndex, (void *)&varYAxis);
    lIndex = 2;
    SafeArrayPutElement(pSafeArray, &lIndex, (void *)&varZValue);
    VariantClear(&varXAxis);
    VariantClear(&varYAxis);
    VariantClear(&varZValue);
    VariantInit(pVal);

    pVal->vt = VT_ARRAY | VT_VARIANT;
    pVal->parray = pSafeArray;
    return S_OK;
}
