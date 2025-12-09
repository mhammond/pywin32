// oleargs.cpp : ole args <--> python object implementation file
//
// $Id$

#include "stdafx.h"
#include "PythonCOM.h"
#include "PyRecord.h"

extern PyObject *PyObject_FromRecordInfo(IRecordInfo *, void *, ULONG, PyTypeObject *type = NULL);
extern PyObject *PyObject_FromSAFEARRAYRecordInfo(SAFEARRAY *psa);
extern BOOL PyObject_AsVARIANTRecordInfo(PyObject *ob, VARIANT *pv);
extern BOOL PyRecord_Check(PyObject *ob);

// Pointer to class defined in .py file.
static PyObject *PyVariant_Type;

// Do BYREF array's get the existing array backfilled with new elements
// (new behaviour that VB seems to want), or allocate a completely
// new array (old behaviour)
#define BYREF_ARRAY_USE_EXISTING_ARRAY

// Need to put this in pywintypes.h with rest of compatibility macros
#define PYWIN_BUFFER_CHECK(obj) (PyBytes_Check(obj) || PyByteArray_Check(obj) || PyMemoryView_Check(obj))

// A little helper just for this file
static PyObject *OleSetTypeError(TCHAR *msg)
{
    PyObject *obMsg = PyWinObject_FromTCHAR(msg);
    if (obMsg) {
        PyErr_SetObject(PyExc_TypeError, obMsg);
        Py_DECREF(obMsg);
    }
    return NULL;
}

BOOL MaybeExtractPyVariant(PyObject *obj, VARTYPE *vt, PyObject **pObjValue, BOOL *pConverted)
{
    // rely on the GIL to ensure there are no races.
    if (PyVariant_Type == NULL) {
        PyObject *mod = PyImport_ImportModule("win32com.client");
        if (mod) {
            PyVariant_Type = PyObject_GetAttrString(mod, "VARIANT");
            Py_DECREF(mod);
        }
        if (!PyVariant_Type)  // WTF?
            return FALSE;
    }
    int check = PyObject_IsInstance(obj, PyVariant_Type);
    if (check == -1)
        return FALSE;
    if (check != 1) {
        // Not that type, so all good but not converted.
        *pConverted = FALSE;
        return TRUE;
    }
    PyObject *obvt = PyObject_GetAttrString(obj, "varianttype");
    if (!obvt)
        return FALSE;
    *vt = (VARTYPE)PyLong_AsUnsignedLongMask(obvt);
    if (*vt == (VARTYPE)-1 && PyErr_Occurred()) {
        Py_DECREF(obvt);
        return FALSE;
    }
    Py_DECREF(obvt);
    PyObject *obValue = PyObject_GetAttrString(obj, "value");
    if (!obValue)
        return FALSE;
    // The result is a borrowed ref, but we can still be sure it
    // lives as long as obj itself.
    Py_DECREF(obValue);
    *pObjValue = obValue;
    *pConverted = TRUE;
    return TRUE;
}

// Returns FALSE on error.  If returns TRUE, pConverted may be TRUE or FALSE.
BOOL ConvertPyVariant(PyObject *obj, VARIANT *pResult, BOOL *pConverted)
{
    VARTYPE vt;
    PyObject *obUse;
    if (!MaybeExtractPyVariant(obj, &vt, &obUse, pConverted))
        return FALSE;
    if (!*pConverted)
        return TRUE;
    PythonOleArgHelper helper = PythonOleArgHelper();
    helper.m_reqdType = vt;
    // Here we can't handle BYREF as our 'helper', which holds the buffers,
    // doesn't live long enough to keep those buffers valid.
    if ((helper.m_reqdType & VT_BYREF) != 0) {
        // XXX - this message sucks :)
        PyErr_SetString(PyExc_ValueError, "win32com.client.VARIANT can't do VT_BYREF in this context");
        return FALSE;
    }
    helper.m_bParsedTypeInfo = TRUE;
    helper.m_convertDirection = POAH_CONVERT_UNKNOWN;
    BOOL ok = helper.MakeObjToVariant(obUse, pResult, NULL);
    if (ok)
        *pConverted = TRUE;
    return ok;
}

///////////////////////////////////////////////////////////
//
// Generic Python objects - to/from VARIANTS and Python objects.
//
//

// Given a Python object, make the best (ie, most appropriate) VARIANT.
// Should be used when the specific type of the variant is not known
// NOTE that passing by reference is not supported using this function
// you need to use the complicated ArgHelpers class for that!
BOOL PyCom_VariantFromPyObject(PyObject *obj, VARIANT *var)
{
    // First see if a special Python VARIANT object.
    BOOL didPyVariant;
    if (!ConvertPyVariant(obj, var, &didPyVariant))
        return FALSE;
    if (didPyVariant)
        return TRUE;
    BOOL bGoodEmpty = FALSE;  // Set if VT_EMPTY should really be used.
    V_VT(var) = VT_EMPTY;
    if (
        // In py3k we don't convert PyBytes_Check objects (ie, bytes) to BSTR...
        PyUnicode_Check(obj)) {
        if (!PyWinObject_AsBstr(obj, &V_BSTR(var))) {
            PyErr_SetString(PyExc_MemoryError, "Making BSTR for variant");
            return FALSE;
        }
        V_VT(var) = VT_BSTR;
    }
    // For Python 3, bool checks need to be above PyLong_Check, which now succeeds for booleans.
    else if (obj == Py_True) {
        V_VT(var) = VT_BOOL;
        V_BOOL(var) = VARIANT_TRUE;
    }
    else if (obj == Py_False) {
        V_VT(var) = VT_BOOL;
        V_BOOL(var) = VARIANT_FALSE;
    }
    else if (PyLong_Check(obj)) {
        int sign = _PyLong_Sign(obj);
        size_t nbits = _PyLong_NumBits(obj);
        if (nbits == (size_t)-1 && PyErr_Occurred())
            return FALSE;
        if (64 < nbits) {
            // too big for 64 bits!  Use a double.
            V_VT(var) = VT_R8;
            V_R8(var) = PyLong_AsDouble(obj);
            if (V_R8(var) == -1.0 && PyErr_Occurred())
                return FALSE;
        }
        else if (32 < nbits) {
            // between 32 and 64 use longlong
            // signed and using all bits use unsigned
            if (sign > 0 && 64 == nbits) {
                V_VT(var) = VT_UI8;
                V_UI8(var) = PyLong_AsUnsignedLongLong(obj);
                if (V_UI8(var) == (unsigned long long)-1 && PyErr_Occurred())
                    return FALSE;
            }
            else {
                // Negative so use signed
                V_VT(var) = VT_I8;
                V_I8(var) = PyLong_AsLongLong(obj);
                // Problem if value is between LLONG_MIN and -ULLONG_MAX
                if (V_I8(var) == -1 && PyErr_Occurred()) {
                    if (PyErr_ExceptionMatches(PyExc_OverflowError)) {
                        // Take now double
                        PyErr_Clear();
                        V_VT(var) = VT_R8;
                        V_R8(var) = PyLong_AsDouble(obj);
                        if (V_R8(var) == -1.0 && PyErr_Occurred())
                            return FALSE;
                    }
                    else {
                        return FALSE;
                    }
                }
            }
        }
        else {
            // less then 32 bit use standard long
            // positive and using all bits so unsigned
            if (sign > 0 && 32 == nbits) {
                V_VT(var) = VT_UI4;
                V_UI4(var) = PyLong_AsUnsignedLong(obj);
                if (V_UI4(var) == (unsigned long)-1 && PyErr_Occurred())
                    return FALSE;
            }
            else {
                // Negative so use signed
                V_VT(var) = VT_I4;
                V_I4(var) = PyLong_AsLong(obj);
                // Problem if value is between LONG_MIN and -ULONG_MAX
                if (V_I4(var) == -1 && PyErr_Occurred()) {
                    if (PyErr_ExceptionMatches(PyExc_OverflowError)) {
                        // Take now double
                        PyErr_Clear();
                        V_VT(var) = VT_I8;
                        V_I8(var) = PyLong_AsLongLong(obj);
                        if (V_I8(var) == -1 && PyErr_Occurred())
                            return FALSE;
                    }
                    else {
                        return FALSE;
                    }
                }
            }
        }
    }
    else if (PyFloat_Check(obj)) {
        V_VT(var) = VT_R8;
        V_R8(var) = PyFloat_AsDouble(obj);
        if (V_R8(var) == -1.0 && PyErr_Occurred())
            return FALSE;
    }
    else if (obj == Py_None) {
        V_VT(var) = VT_NULL;
    }
    else if (PyObject_HasAttrString(obj, "_oleobj_")) {
        if (PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IDispatch, (void **)&V_DISPATCH(var), FALSE))
            V_VT(var) = VT_DISPATCH;
        else {
            PyErr_Clear();
            // Try for IUnknown
            if (PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IUnknown, (void **)&V_UNKNOWN(var), FALSE))
                V_VT(var) = VT_UNKNOWN;
            else
                PyErr_Clear();
        }
    }
    else if (PyIBase::is_object(obj, &PyIDispatch::type)) {
        V_VT(var) = VT_DISPATCH;
        V_DISPATCH(var) = PyIDispatch::GetI(obj);
        V_DISPATCH(var)->AddRef();
    }
    else if (PyIBase::is_object(obj, &PyIUnknown::type)) {
        V_VT(var) = VT_UNKNOWN;
        V_UNKNOWN(var) = PyIUnknown::GetI(obj);
        V_UNKNOWN(var)->AddRef();
    }
    else if (obj->ob_type == &PyOleEmptyType) {
        bGoodEmpty = TRUE;
    }
    // code changed by ssc
    else if (obj->ob_type == &PyOleNothingType) {
        V_VT(var) = VT_DISPATCH;
        V_DISPATCH(var) = NULL;
    }
    // end code changed by ssc
    else if (obj->ob_type == &PyOleArgNotFoundType) {
        // use default parameter
        // Note the SDK documentation for FUNCDESC describes this behaviour
        // as correct.  However, IMAPI.Session.Logon, most DAO, etc do _not_ work
        // correctly in this case. ..Logon does work if the params are not
        // presented at all (ie, argCount < ..)  This is supported by the
        // PyOleMissing object, but should be handled before here.
        // Note that VB seems to use the "missing" rather than "empty"
        // behaviour (as logon works there)
        // Also note that MAPI still does _not_ work if a valid param with
        // VT_EMPTY is passed in, so that is also not an option - the param must
        // be _missing_.
        V_VT(var) = VT_ERROR;
        V_ERROR(var) = DISP_E_PARAMNOTFOUND;
    }
    else if (PyWinTime_Check(obj)) {
        V_VT(var) = VT_DATE;
        if (!PyWinObject_AsDATE(obj, &(V_DATE(var))))
            return FALSE;
    }
    else if (PYWIN_BUFFER_CHECK(obj)) {
        // We have a buffer object - convert to safe array of VT_UI1
        if (!PyCom_SAFEARRAYFromPyObject(obj, &V_ARRAY(var), VT_UI1))
            return FALSE;
        V_VT(var) = VT_ARRAY | VT_UI1;
    }
    // NOTE: PySequence_Check may return true for instance objects,
    // or ANY object with a __len__ attribute.
    // So make sure this check is after anything else which qualifies.
    else if (PySequence_Check(obj)) {
        V_ARRAY(var) = NULL;  // not a valid, existing array.
        BOOL is_record_item = false;
        if (PyObject_Length(obj) > 0) {
            PyObject *obItemCheck = PySequence_GetItem(obj, 0);
            is_record_item = PyRecord_Check(obItemCheck);
            Py_XDECREF(obItemCheck);
        }
        // If the sequence elements are PyRecord objects we do NOT package
        // them as VARIANT elements but put them directly into the SAFEARRAY.
        if (is_record_item) {
            if (!PyCom_SAFEARRAYFromPyObject(obj, &V_ARRAY(var), VT_RECORD))
                return FALSE;
            V_VT(var) = VT_ARRAY | VT_RECORD;
        }
        else {
            if (!PyCom_SAFEARRAYFromPyObject(obj, &V_ARRAY(var)))
                return FALSE;
            V_VT(var) = VT_ARRAY | VT_VARIANT;
        }
    }
    else if (PyRecord_Check(obj)) {
        if (!PyObject_AsVARIANTRecordInfo(obj, var))
            return FALSE;
        V_VT(var) = VT_RECORD;
    }
    // Decimal class from new _decimal module in Python 3.3 shows different name
    else if (strcmp(obj->ob_type->tp_name, "Decimal") == 0 || strcmp(obj->ob_type->tp_name, "decimal.Decimal") == 0) {
        // VT_DECIMAL supports more precision here, use in error case? leave existing behavior for now
        if (!PyObject_AsCurrency(obj, &V_CY(var)))
            return FALSE;
        V_VT(var) = VT_CY;
    }
    else if (obj->ob_type->tp_as_number) {
        V_VT(var) = VT_R8;
        V_R8(var) = PyFloat_AsDouble(obj);
        if (V_R8(var) == -1.0 && PyErr_Occurred())
            return FALSE;
    }

    if (V_VT(var) == VT_EMPTY && !bGoodEmpty) {
        // Must ensure we have a Python error set if we fail!
        if (!PyErr_Occurred()) {
            char *extraMessage = "";
            if (obj->ob_type->tp_as_buffer)
                extraMessage = " (but obtaining the buffer() of this object could)";
            PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to a COM VARIANT%s",
                         obj->ob_type->tp_name, extraMessage);
        }
        return FALSE;
    }
    return TRUE;
}

// Given a variant, turn it into a Python object of the closest type.
// Note that ByRef params are not supported here.
PyObject *PyCom_PyObjectFromVariant(const VARIANT *var)
{
    HRESULT hr;
    VARIANT varValue;
    PyObject *result = NULL;

    if (!var) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    /* skip past any variant references to a "real" variant
      (Why do we do this?  Why is it only a VARIANT?  what's the story, morning glory?
    */
    while (V_VT(var) == (VT_BYREF | VT_VARIANT)) var = V_VARIANTREF(var);

    /* ### note: we shouldn't see this, it is illegal in a VARIANT */
    if (V_ISVECTOR(var)) {
        return OleSetTypeError(_T("Can't convert vectors!"));
    }

    if (V_ISARRAY(var)) {
        SAFEARRAY FAR *psa;
        if (V_ISBYREF(var))
            psa = *V_ARRAYREF(var);
        else
            psa = V_ARRAY(var);
        if (psa == NULL) {  // A NULL array
            Py_INCREF(Py_None);
            return Py_None;
        }
        VARENUM rawVT = (VARENUM)(V_VT(var) & VT_TYPEMASK);
        return PyCom_PyObjectFromSAFEARRAY(psa, rawVT);
    }

    /* get a fully dereferenced copy of the variant */
    /* ### we may want to optimize this sometime... avoid copying values */
    VariantInit(&varValue);
    VariantCopyInd(&varValue, (VARIANT *)var);

    switch (V_VT(&varValue)) {
        case VT_BOOL:
            result = V_BOOL(&varValue) ? Py_True : Py_False;
            Py_INCREF(result);
            break;

        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_UINT:
            hr = VariantChangeType(&varValue, &varValue, 0, VT_UI4);
            if (FAILED(hr)) {
                TCHAR buf[200];
                wsprintf(buf, _T("Error converting integer variant (%08lx)"), hr);
                OleSetTypeError(buf);
                break;
            }
            // The result may be too large for a simple "long".  If so,
            // we must return a long.
            if (V_UI4(&varValue) <= INT_MAX)
                result = PyLong_FromLong(V_UI4(&varValue));
            else
                result = PyLong_FromUnsignedLong(V_UI4(&varValue));
            break;

        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_INT:
            hr = VariantChangeType(&varValue, &varValue, 0, VT_I4);
            if (FAILED(hr)) {
                TCHAR buf[200];
                wsprintf(buf, _T("Error converting integer variant (%08lx)"), hr);
                OleSetTypeError(buf);
                break;
            }
            result = PyLong_FromLong(V_I4(&varValue));
            break;

        case VT_UI8:
            // The result may be too large for a simple "long". If so,
            // we must return a long.
            if (V_UI8(&varValue) <= LONG_MAX)
                result = PyLong_FromLong((long)V_UI8(&varValue));
            else
                result = PyLong_FromUnsignedLongLong(V_UI8(&varValue));
            break;

        case VT_I8:
            if ((LONG_MIN <= V_I8(&varValue)) && (V_I8(&varValue) <= LONG_MAX))
                result = PyLong_FromLong((long)V_I8(&varValue));
            else
                result = PyLong_FromLongLong(V_I8(&varValue));
            break;

        case VT_HRESULT:
        case VT_ERROR:
            result = PyLong_FromLong(V_ERROR(&varValue));
            break;

        case VT_R4:
        case VT_R8:
            hr = VariantChangeType(&varValue, &varValue, 0, VT_R8);
            if (FAILED(hr)) {
                TCHAR buf[200];
                wsprintf(buf, _T("Error converting floating point variant (%08lx)"), hr);
                OleSetTypeError(buf);
                break;
            }
            result = PyFloat_FromDouble(V_R8(&varValue));
            break;

        case VT_DISPATCH: {
            IDispatch *pIDispatch = V_DISPATCH(&varValue);
            if (pIDispatch)
                result = PyCom_PyObjectFromIUnknown(pIDispatch, IID_IDispatch, TRUE);
            else {
                Py_INCREF(Py_None);
                result = Py_None;
            }
            break;
        }

        case VT_UNKNOWN: {
            IUnknown *punk = V_UNKNOWN(&varValue);
            if (punk)
                result = PyCom_PyObjectFromIUnknown(punk, IID_IUnknown, TRUE);
            else {
                Py_INCREF(Py_None);
                result = Py_None;
            }
            break;
        }

        case VT_BSTR:
            result = PyWinObject_FromBstr(V_BSTR(&varValue));
            break;

        case VT_NULL:
        case VT_EMPTY:
            Py_INCREF(Py_None);
            result = Py_None;
            break;

        case VT_DATE:
            result = PyWinObject_FromDATE(V_DATE(&varValue));
            break;

        case VT_CY:
            result = PyObject_FromCurrency(varValue.cyVal);
            break;

        case VT_DECIMAL:
            result = PyObject_FromDecimal(varValue.decVal);
            break;

        case VT_RECORD: {
            ULONG cb;
            V_RECORDINFO(&varValue)->GetSize(&cb);
            result = PyObject_FromRecordInfo(V_RECORDINFO(&varValue), V_RECORD(&varValue), cb);
        } break;
        default: {
            HRESULT hr = VariantChangeType(&varValue, &varValue, 0, VT_BSTR);
            if (FAILED(hr)) {
                TCHAR buf[200];
                wsprintf(buf, _T("The Variant type (0x%x) is not supported, and it can not be converted to a string"),
                         V_VT(var));
                OleSetTypeError(buf);
                break;
            }
            result = PyWinObject_FromBstr(V_BSTR(&varValue));
            break;
        }
    }

    VariantClear(&varValue);
    return result;
}

///////////////////////////////////////////////////////////
//
// SAFEARRAY support - to/from SAFEARRAYS and Python sequences.
//
//
// PyObject -> SafeArray
static BOOL PyCom_SAFEARRAYFromPyObjectBuildDimension(PyObject *obj, SAFEARRAY *pSA, VARENUM vt, UINT dimNo, UINT nDims,
                                                      SAFEARRAYBOUND *pBounds, LONG *pIndices)
{
    LONG numElements = pBounds[dimNo - 1].cElements;
    if ((LONG)PyObject_Length(obj) != numElements) {
        OleSetTypeError(_T("All dimensions must be a sequence of the same size"));
        return FALSE;
    }
    // See if we can take a short-cut for byte arrays - if
    // so, we can copy the entire dimension in one hit
    // (only support single segment buffers for now)
    if (dimNo == nDims && vt == VT_UI1 && obj->ob_type->tp_as_buffer) {
        void *sa_buf;
        PyWinBufferView pybuf(obj);
        if (!pybuf.ok())
            return FALSE;

        if (pybuf.len() != numElements) {
            OleSetTypeError(_T("Internal error - the buffer length is not the sequence length!"));
            return FALSE;
        }

        HRESULT hr = SafeArrayAccessData(pSA, &sa_buf);
        if (FAILED(hr)) {
            PyCom_BuildPyException(hr);
            return FALSE;
        }
        memcpy(sa_buf, pybuf.ptr(), pybuf.len());
        SafeArrayUnaccessData(pSA);
        // All done without a single loop :-)
        return TRUE;
    }
    // Otherwise just fall through into the standard mechanisms

    BOOL ok = TRUE;
    for (int index = 0; index < (int)numElements && ok; index++) {
        pIndices[dimNo - 1] = index;
        PyObject *item = PySequence_GetItem(obj, index);
        if (item == NULL)
            return FALSE;
        if (dimNo == nDims) {  // Last one - fill the data
            VARIANT element;
            LPVOID pvData;
            if (vt == VT_VARIANT) {  // simple conversion
                ok = PyCom_VariantFromPyObject(item, &element);
                pvData = &element;
            }
            else {
                // Complex conversion.
                if (vt & VT_ARRAY || vt & VT_BYREF) {
                    OleSetTypeError(_T("Internal error - unexpected argument - only simple VARIANTTYPE expected"));
                    ok = FALSE;
                }
                else {
                    PythonOleArgHelper helper;
                    helper.m_reqdType = vt;
                    ok = helper.MakeObjToVariant(item, &element);
                    switch (vt) {
                        case VT_RECORD:
                            pvData = V_RECORD(&element);
                            break;
                        case VT_DISPATCH:
                            pvData = V_DISPATCH(&element);
                            break;
                        case VT_UNKNOWN:
                            pvData = V_UNKNOWN(&element);
                            break;
                        case VT_BSTR:
                            pvData = V_BSTR(&element);
                            break;
                        default:
                            // The data is in a union - just use an
                            // arbitrary element.
                            pvData = &V_I4(&element);
                            break;
                    }
                }
            }
            if (ok) {
                HRESULT hr = SafeArrayPutElement(pSA, pIndices, pvData);
                VariantClear(&element);
                if (FAILED(hr)) {
                    PyCom_BuildInternalPyException("Could not set the SAFEARRAY element");
                    ok = FALSE;
                }
            }
        }
        else {
            // recurse down dimensions
            ok = PyCom_SAFEARRAYFromPyObjectBuildDimension(item, pSA, vt, dimNo + 1, nDims, pBounds, pIndices);
        }
        Py_DECREF(item);
    }
    return ok;
}

// Arbitrary-sized array dimensions contributed by Stefan Schukat Feb-2004
static long PyCom_CalculatePyObjectDimension(PyObject *obItemCheck, long lDimension, PyObject *ppyobDimensionDictionary)
{
    // Buffers are a special case - they define 1 new dimension.
    // Buffers supported sequence semantics in Python 2, but for some reason memoryview objects
    //	in py3k do not, so check separately
    if (PYWIN_BUFFER_CHECK(obItemCheck))
        return lDimension + 1;

    // Allow arbitrary sequences, but not strings or Unicode objects.
    if (PyBytes_Check(obItemCheck) || PyUnicode_Check(obItemCheck) || !PySequence_Check(obItemCheck))
        return lDimension;

    long lReturnDimension = lDimension;
    PyObject *ppyobDimension;
    PyObject *ppyobSize;
    PyObject *ppyobDimensionSize;
    PyObject *ppyobItem;
    Py_ssize_t lIndex;
    long lMinimalDimension = -1;
    long lActualDimension = -1;
    Py_ssize_t lObjectSize;

    // Retrieve the size of the object
    lObjectSize = PySequence_Length(obItemCheck);
    if (lObjectSize == -1) {
        /* has a __len__, but it failed.  Treat as not a sequence */
        assert(PyErr_Occurred());  // can't *really* have -1 elems! */
        PyErr_Clear();
    }
    if (lObjectSize != -1) {  // A real sequence of size zero should be OK though.
        ppyobSize = PyLong_FromSsize_t(lObjectSize);

        // Retrieve the stored size in this dimension
        ppyobDimension = PyLong_FromLong(lDimension);
        // Note: No ref added by PyDict_GetItem
        ppyobDimensionSize = PyDict_GetItem(ppyobDimensionDictionary, ppyobDimension);
        if (NULL == ppyobDimensionSize) {
            // Not found so first element defines the size in this dimension
            PyErr_Clear();
            PyDict_SetItem(ppyobDimensionDictionary, ppyobDimension, ppyobSize);
        }
        else {
            // Check if stored size in this dimension equals the size of the element to check
            Py_ssize_t lStoredSize = PyLong_AsSsize_t(ppyobDimensionSize);
            if (lStoredSize != lObjectSize) {
                // if not the same size => no new dimension
                Py_XDECREF(ppyobSize);
                Py_XDECREF(ppyobDimension);
                return lReturnDimension;
            }
        }
        Py_XDECREF(ppyobSize);
        Py_XDECREF(ppyobDimension);

        // A special case for a zero-length sequence - we accept this as
        // a new dimension, but no children to check.
        // ie an empty list has 1 dimension.
        if (lObjectSize == 0)
            return lReturnDimension + 1;

        // Now check for all elements in this list for their dimensionality
        // Their size is compared to the size stored in the dimension dictionary
        for (lIndex = 0; lIndex < lObjectSize; lIndex++) {
            ppyobItem = PySequence_GetItem(obItemCheck, lIndex);
            if (ppyobItem == NULL) {
                // Says it is a sequence, but getting the item failed.
                // (eg, may be a COM instance that has __getitem__, but fails when attempting)
                // Ignore the error, and pretend it is not a sequence.
                PyErr_Clear();
                break;
            }
            // Call method recursively
            lActualDimension = PyCom_CalculatePyObjectDimension(ppyobItem, lDimension + 1, ppyobDimensionDictionary);
            if (-1 == lMinimalDimension) {
                // First call so store it
                lMinimalDimension = lActualDimension;
                lReturnDimension = lActualDimension;
            }
            else {
                // Get the smallest dimension
                if (lActualDimension < lMinimalDimension) {
                    lMinimalDimension = lActualDimension;
                }
                // Check if all dimensions of the sublist are equal
                if (lReturnDimension != lActualDimension) {
                    // if not set the minimal dimension
                    lReturnDimension = lMinimalDimension;
                }
            }
            Py_XDECREF(ppyobItem);
        }
    }
    return lReturnDimension;
}

static BOOL PyCom_SAFEARRAYFromPyObjectEx(PyObject *obj, SAFEARRAY **ppSA, bool bAllocNewArray, VARENUM vt)
{
    // NOTE: We make no attempt to validate or free any existing array if asked to allocate a new one!

    // Seek down searching for total dimension count.
    // Item zero of each element will do for now
    // (as all must be same)
    // First we _will_ allow None here (just don't use it if it crashes :-)
    if (obj == Py_None) {
        if (bAllocNewArray)
            *ppSA = NULL;
        // Otherwise we leave it alone!
        return TRUE;
    }
    LONG cDims = 0;
    // Arbitrary-sized array dimensions contributed by Stefan Schukat Feb-2004
    // Allow arbitrary sized sequences to be transported to a COM server
    PyObject *ppyobDimensionDictionary = PyDict_New();
    // Calculate the unique dimension of the sequence
    cDims = PyCom_CalculatePyObjectDimension(obj, 0, ppyobDimensionDictionary);
    Py_DECREF(ppyobDimensionDictionary);

    if (cDims == 0) {
        OleSetTypeError(_T("Objects for SAFEARRAYS must be sequences (of sequences), or a buffer object."));
        return FALSE;
    }
    if (!bAllocNewArray) {
        if (SafeArrayGetDim(*ppSA) != (unsigned)cDims) {
            PyErr_SetString(PyExc_ValueError,
                            "When refilling a safe array, the sequence must have the same number of dimensions as the "
                            "existing array.");
            return FALSE;
        }
    }

    SAFEARRAYBOUND *pBounds = new SAFEARRAYBOUND[cDims];

    // Now run down again, setting up the bounds
    PyObject *obItemCheck = obj;
    Py_INCREF(obItemCheck);
    for (LONG dimLook = 1; dimLook <= cDims; dimLook++) {
        pBounds[dimLook - 1].lLbound = 0;  // always!
        // Don't use PySequence_Length due to memoryview not supporting sequence protocol
        pBounds[dimLook - 1].cElements = (ULONG)PyObject_Length(obItemCheck);
        if (!bAllocNewArray) {
            LONG exist_lbound, exist_ubound;
            SafeArrayGetLBound(*ppSA, dimLook, &exist_lbound);
            SafeArrayGetUBound(*ppSA, dimLook, &exist_ubound);
            if ((unsigned long)(exist_ubound - exist_lbound + 1) != pBounds[dimLook - 1].cElements) {
                PyErr_SetString(
                    PyExc_ValueError,
                    "When refilling a safe array, the sequences must be the same length as the existing array.");
                Py_XDECREF(obItemCheck);
                delete[] pBounds;
                return FALSE;
            }
        }
        // Don't need to do this check if buffer is last dim
        if (!PYWIN_BUFFER_CHECK(obItemCheck)) {
            PyObject *obSave = obItemCheck;
            if (pBounds[dimLook - 1].cElements) {
                obItemCheck = PySequence_GetItem(obItemCheck, 0);
                Py_DECREF(obSave);
                if (obItemCheck == NULL) {
                    delete[] pBounds;
                    return FALSE;
                }
            }
        }
    }
    Py_XDECREF(obItemCheck);

    if (bAllocNewArray) {
        // OK - Finally can create the array...
        if (vt == VT_RECORD) {
            // SAFEARRAYS of UDTs need a special treatment.
            obItemCheck = PySequence_GetItem(obj, 0);
            PyRecord *pyrec = (PyRecord *)obItemCheck;
            Py_XDECREF(obItemCheck);
            *ppSA = SafeArrayCreateEx(vt, cDims, pBounds, pyrec->pri);
        }
        else
            *ppSA = SafeArrayCreate(vt, cDims, pBounds);
        if (*ppSA == NULL) {
            delete[] pBounds;
            PyErr_SetString(PyExc_MemoryError, "CreatingSafeArray");
            return FALSE;
        }
    }

    LONG *indices = new LONG[cDims];
    // Get the data

    BOOL bOK = PyCom_SAFEARRAYFromPyObjectBuildDimension(obj, *ppSA, vt, 1, cDims, pBounds, indices);
    if (!bOK && bAllocNewArray && *ppSA) {
        SafeArrayDestroy(*ppSA);
        *ppSA = NULL;
    }
    delete[] indices;
    delete[] pBounds;

    return bOK;
}

BOOL PyCom_SAFEARRAYFromPyObject(PyObject *obj, SAFEARRAY **ppSA, VARENUM vt /*= VT_VARIANT*/)
{
    return PyCom_SAFEARRAYFromPyObjectEx(obj, ppSA, true, vt);
}

///////////////////////////
//
// SafeArray -> PyObject
/*
   Helper - Convert the current index to a Python object.
   No iteration - returns a simple object (not a tuple)
*/
static PyObject *PyCom_PyObjectFromSAFEARRAYDimensionItem(SAFEARRAY *psa, VARENUM vt, long *arrayIndices)
{
    PyObject *subitem = NULL;
    HRESULT hres = 0;
    switch (vt) {
        case VT_I2: {
            short sh;
            hres = SafeArrayGetElement(psa, arrayIndices, &sh);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromLong(sh);
            break;
        }
        case VT_I4:
        case VT_ERROR: {
            long ln;
            hres = SafeArrayGetElement(psa, arrayIndices, &ln);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromLong(ln);
            break;
        }
        case VT_I8: {
            LARGE_INTEGER ll;
            hres = SafeArrayGetElement(psa, arrayIndices, &ll);
            if (FAILED(hres))
                break;
            subitem = PyWinObject_FromPY_LONG_LONG(ll);
            break;
        }
        case VT_R4: {
            float fl;
            hres = SafeArrayGetElement(psa, arrayIndices, &fl);
            if (FAILED(hres))
                break;
            subitem = PyFloat_FromDouble(fl);
            break;
        }
        case VT_R8: {
            double db;
            hres = SafeArrayGetElement(psa, arrayIndices, &db);
            if (FAILED(hres))
                break;
            subitem = PyFloat_FromDouble(db);
            break;
        }
        case VT_CY: {
            CURRENCY c;
            hres = SafeArrayGetElement(psa, arrayIndices, &c);
            if (FAILED(hres))
                break;
            subitem = PyObject_FromCurrency(c);
            break;
        }
        case VT_DECIMAL: {
            DECIMAL dec;
            hres = SafeArrayGetElement(psa, arrayIndices, &dec);
            if (FAILED(hres))
                break;
            subitem = PyObject_FromDecimal(dec);
            break;
        }
        case VT_DATE: {
            DATE dt;
            hres = SafeArrayGetElement(psa, arrayIndices, &dt);
            if (FAILED(hres))
                break;
            subitem = PyWinObject_FromDATE(dt);
            break;
        }
        case VT_BSTR: {
            BSTR str;
            hres = SafeArrayGetElement(psa, arrayIndices, &str);
            if (FAILED(hres))
                break;
            subitem = PyWinObject_FromBstr(str, TRUE);
            break;
        }
        case VT_DISPATCH: {
            IDispatch *pDisp;
            hres = SafeArrayGetElement(psa, arrayIndices, &pDisp);
            if (FAILED(hres))
                break;
            subitem = PyCom_PyObjectFromIUnknown(pDisp, IID_IDispatch, FALSE);
            break;
        }
        // case VT_ERROR - handled above with I4
        case VT_BOOL: {
            bool b1;
            hres = SafeArrayGetElement(psa, arrayIndices, &b1);
            if (FAILED(hres))
                break;
            subitem = PyBool_FromLong(b1);
            break;
        }
        case VT_VARIANT: {
            VARIANT varValue;
            VariantInit(&varValue);
            hres = SafeArrayGetElement(psa, arrayIndices, &varValue);
            if (FAILED(hres))
                break;
            subitem = PyCom_PyObjectFromVariant(&varValue);
            VariantClear(&varValue);  // clean this up
            break;
        }
        case VT_UNKNOWN: {
            IUnknown *pUnk;
            hres = SafeArrayGetElement(psa, arrayIndices, &pUnk);
            if (FAILED(hres))
                break;
            subitem = PyCom_PyObjectFromIUnknown(pUnk, IID_IUnknown, FALSE);
            break;
        }
            // case VT_RECORD

        case VT_I1:
        case VT_UI1: {
            unsigned char i1;
            hres = SafeArrayGetElement(psa, arrayIndices, &i1);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromLong(i1);
            break;
        }
        case VT_UI2: {
            unsigned short s1;
            hres = SafeArrayGetElement(psa, arrayIndices, &s1);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromUnsignedLong(s1);
            break;
        }
        case VT_UI4: {
            unsigned long l1;
            hres = SafeArrayGetElement(psa, arrayIndices, &l1);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromUnsignedLong(l1);
            break;
        }
        case VT_UI8: {
            ULARGE_INTEGER ll;
            hres = SafeArrayGetElement(psa, arrayIndices, &ll);
            if (FAILED(hres))
                break;
            subitem = PyWinObject_FromUPY_LONG_LONG(ll);
            break;
        }
        case VT_INT: {
            int i1;
            hres = SafeArrayGetElement(psa, arrayIndices, &i1);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromLong(i1);
            break;
        }
        case VT_UINT: {
            unsigned int i1;
            hres = SafeArrayGetElement(psa, arrayIndices, &i1);
            if (FAILED(hres))
                break;
            subitem = PyLong_FromUnsignedLong(i1);
            break;
        }

        default: {
            TCHAR buf[200];
            wsprintf(buf, _T("The VARIANT type 0x%x is not supported for SAFEARRAYS"), vt);
            OleSetTypeError(buf);
        }
    }
    if (FAILED(hres)) {
        PyCom_BuildPyException(hres);
        Py_XDECREF(subitem);
        subitem = NULL;
    }
    // All done.
    return subitem;
}

/* Helper - Convert the specified dimension of the specified safe array to
   a Python object (a tuple)
*/
PyObject *PyCom_PyObjectFromSAFEARRAYBuildDimension(SAFEARRAY *psa, VARENUM vt, UINT dimNo, UINT nDims,
                                                    long *arrayIndices)
{
    long lb, ub;
    HRESULT hres = SafeArrayGetLBound(psa, dimNo, &lb);
    if (FAILED(hres))
        return PyCom_BuildPyException(hres);
    hres = SafeArrayGetUBound(psa, dimNo, &ub);
    if (FAILED(hres))
        return PyCom_BuildPyException(hres);
    // First we take a shortcut for VT_UI1 (ie, binary) buffers.
    if (vt == VT_UI1) {
        void *sa_buf;
        HRESULT hres = SafeArrayAccessData(psa, &sa_buf);
        if (FAILED(hres))
            return PyCom_BuildPyException(hres);
        long cElems = ub - lb + 1;
        long dataSize = cElems * sizeof(unsigned char);
        PyObject *ret = PyBuffer_New(dataSize);
        if (ret != NULL) {
            // Access the buffer object using the buffer interfaces.
            PyWinBufferView pybuf(ret, true);
            if (!pybuf.ok()) {
                SafeArrayUnaccessData(psa);
                Py_DECREF(ret);
                return NULL;
            }
            if (pybuf.len() != cElems) {
                PyErr_SetString(PyExc_RuntimeError, "buffer size is not what we created!");
                SafeArrayUnaccessData(psa);
                Py_DECREF(ret);
                return NULL;
            }
            memcpy(pybuf.ptr(), sa_buf, dataSize);
        }
        SafeArrayUnaccessData(psa);
        return ret;
    }
    // Another shortcut for VT_RECORD types.
    if (vt == VT_RECORD) {
        return PyObject_FromSAFEARRAYRecordInfo(psa);
    }
    // Normal SAFEARRAY case returning a tuple.

    PyObject *retTuple = PyTuple_New(ub - lb + 1);
    if (retTuple == NULL)
        return FALSE;
    int tupleIndex = 0;
    // Get a pointer for the dimension to iterate (the last one)
    long *pMyArrayIndex = arrayIndices + (dimNo - 1);
    *pMyArrayIndex = lb;
    BOOL bBuildItems = (nDims == dimNo);
    for (; *pMyArrayIndex <= ub; (*pMyArrayIndex)++, tupleIndex++) {
        PyObject *subItem = NULL;
        if (bBuildItems) {
            subItem = PyCom_PyObjectFromSAFEARRAYDimensionItem(psa, vt, arrayIndices);
        }
        else {
            // Recurse and build sub-array.
            subItem = PyCom_PyObjectFromSAFEARRAYBuildDimension(psa, vt, dimNo + 1, nDims, arrayIndices);
        }
        if (subItem == NULL) {
            Py_DECREF(retTuple);
            return NULL;
        }
        PyTuple_SET_ITEM(retTuple, tupleIndex, subItem);
    }
    return retTuple;
}

/* Actual doer - Convert the specified safe array to a Python object - either a
   single tuple, or a tuples of tuples for each dimension
*/
PyObject *PyCom_PyObjectFromSAFEARRAY(SAFEARRAY *psa, VARENUM vt /* = VT_VARIANT */)
{
    // Our caller must has resolved all byref and array references.
    if (vt & VT_ARRAY || vt & VT_BYREF) {
        OleSetTypeError(_T("Internal error - unexpected argument - only simple VARIANTTYPE expected"));
        return FALSE;
    }
    UINT nDim = SafeArrayGetDim(psa);
    LONG *pIndices = new LONG[nDim];
    PyObject *result = PyCom_PyObjectFromSAFEARRAYBuildDimension(psa, vt, 1, nDim, pIndices);
    delete[] pIndices;
    return result;
}

///////////////////////////////////////////////////////
//
// Python arg helper class
//
PythonOleArgHelper::PythonOleArgHelper()
{
    // First wipe myself out to zero!
    memset(this, 0, sizeof(*this));
    m_bIsOut = FALSE;
    m_reqdType = VT_VARIANT;
    m_bParsedTypeInfo = FALSE;
    m_convertDirection = POAH_CONVERT_UNKNOWN;
}
PythonOleArgHelper::~PythonOleArgHelper()
{
    Py_XDECREF(m_pyVariant);
    // First check we actually have ownership of any buffers.
    if (m_convertDirection == POAH_CONVERT_UNKNOWN || m_convertDirection == POAH_CONVERT_FROM_VARIANT)
        return;
    // OK - it is possible we own the buffers - check for sure based on the type...
    if (m_reqdType & VT_ARRAY) {
        // Array datatype - cleanup (but how?)
        if (m_reqdType & VT_BYREF) {
#ifndef BYREF_ARRAY_USE_EXISTING_ARRAY
            // We own array pointer - free it.
            if (m_arrayBuf) {
                HRESULT hr = SafeArrayDestroy(m_arrayBuf);
#ifdef _DEBUG
                if (hr != S_OK) {
                    PyCom_LogF(_T("SafeArrayDestroy failed with rc=%d"), hr);
                }
#endif
            }  // have array pointer
#endif  // BYREF_ARRAY_USE_EXISTING_ARRAY
        }  // BYREF array.
    }
    else {
        switch (m_reqdType) {
            case VT_BSTR | VT_BYREF:
                if (m_pValueHolder)
                    SysFreeString((BSTR)m_pValueHolder);
                break;
            case VT_DISPATCH | VT_BYREF:
                PYCOM_RELEASE(m_dispBuf);
                break;
            case VT_UNKNOWN | VT_BYREF:
                PYCOM_RELEASE(m_unkBuf);
                break;
            case VT_VARIANT | VT_BYREF:
                if (m_varBuf) {
                    PY_INTERFACE_PRECALL;
                    VariantClear(m_varBuf);
                    delete m_varBuf;
                    PY_INTERFACE_POSTCALL;
                }
                break;
                // default - take no action.
        }  // switch
    }
}

BOOL PythonOleArgHelper::ParseTypeInformation(PyObject *reqdObjectTuple)
{
    if (m_bParsedTypeInfo)
        return TRUE;
    PyErr_Clear();
    PyObject *typeDesc = PyTuple_GetItem(reqdObjectTuple, 0);
    if (typeDesc == NULL)
        return FALSE;
    m_reqdType = (VARTYPE)PyLong_AsLong(typeDesc);
    if (PyErr_Occurred())
        return FALSE;
    PyObject *paramFlags = PyTuple_GetItem(reqdObjectTuple, 1);
    if (paramFlags == NULL)
        return FALSE;
    DWORD pf = (DWORD)PyLong_AsLong(paramFlags);
    if (PyErr_Occurred())
        return FALSE;
    // If we have _no_ param flags, use the BYREF-ness of the param
    // to determine if we are possibly an out param.
    // If we have any flags, assume they are all valid.
    if (pf == 0)
        m_bIsOut = (m_reqdType & VT_BYREF) != 0;
    else
        m_bIsOut = (pf & (PARAMFLAG_FOUT | PARAMFLAG_FRETVAL)) != 0;
    m_bParsedTypeInfo = TRUE;
    return TRUE;
}

#define BREAK_FALSE \
    {               \
        rc = FALSE; \
        break;      \
    }
#define VALID_BYREF_MISSING(obUse) (obUse == Py_None || obUse->ob_type == &PyOleEmptyType)

BOOL PythonOleArgHelper::MakeObjToVariant(PyObject *obj, VARIANT *var, PyObject *reqdObjectTuple)
{
    // Check my logic still holds up - basically we can't call this twice on the same object.
    assert(m_convertDirection == POAH_CONVERT_UNKNOWN || m_convertDirection == POAH_CONVERT_FROM_VARIANT);
    // If this is the "driving" conversion, then we allocate buffers.
    // Otherwise, we are simply filling in the buffers as provided by the caller.
    if (reqdObjectTuple) {
        if (!ParseTypeInformation(reqdObjectTuple))
            return FALSE;
    }
    if (m_convertDirection == POAH_CONVERT_UNKNOWN && m_reqdType == VT_VARIANT) {
        BOOL converted;
        PyObject *newObj;
        VARTYPE newType;
        if (!MaybeExtractPyVariant(obj, &newType, &newObj, &converted))
            return FALSE;
        if (converted) {
            m_reqdType = newType;
            m_bIsOut = (m_reqdType & VT_BYREF) != 0;
            assert(!m_pyVariant);
            m_pyVariant = obj;
            Py_INCREF(m_pyVariant);
            obj = newObj;
        }
    }
    BOOL bCreateBuffers = (m_convertDirection == POAH_CONVERT_UNKNOWN);
    if (m_convertDirection == POAH_CONVERT_UNKNOWN)
        m_convertDirection = POAH_CONVERT_FROM_PYOBJECT;

    if (obj->ob_type == &PyOleEmptyType) {
        // Quick exit - use default parameter
        V_VT(var) = VT_ERROR;
        V_ERROR(var) = DISP_E_PARAMNOTFOUND;
        return TRUE;
    }
    if (m_reqdType & VT_ARRAY) {
        VARENUM rawVT = (VARENUM)(m_reqdType & VT_TYPEMASK);
        if (m_reqdType & VT_BYREF) {
            if (!VALID_BYREF_MISSING(obj)) {
                bool bNewArray = (V_VT(var) & ~VT_TYPEMASK) != (VT_BYREF | VT_ARRAY);
                assert(m_arrayBuf == NULL);  // shouldn't be anything else here!
                if (bNewArray) {
                    assert(V_VT(var) == VT_EMPTY);  // should we clear anything else?
                    V_ARRAYREF(var) = &m_arrayBuf;
                }
                // else m_arrayBuf remains NULL, and we reuse existing array.
                V_VT(var) = m_reqdType;
                // Refill the existing array.
                if (!PyCom_SAFEARRAYFromPyObjectEx(obj, V_ARRAYREF(var), bNewArray, rawVT))
                    return FALSE;
            }
            else {
                // If the variant is brand new (ie, VT_EMPTY), set it to VT_ARRAY
                // If not, it may be an "out" param getting filled, so we leave
                // it alone.
                if (V_VT(var) == VT_EMPTY) {
                    V_VT(var) = m_reqdType;
                    assert(m_arrayBuf == NULL);  // shouldn't be anything else here!
                    V_ARRAYREF(var) = &m_arrayBuf;
                }
                assert(V_VT(var) | VT_ARRAY);
            }
        }
        else {
            assert(V_VT(var) == VT_EMPTY ||
                   V_ARRAY(var) == NULL);  // Probably a mem leak - the existing array should be cleared!
            if (!PyCom_SAFEARRAYFromPyObject(obj, &V_ARRAY(var), rawVT))
                return FALSE;
            V_VT(var) = m_reqdType;
        }
        return TRUE;  // All done with array!
    }
    if (m_reqdType & VT_VECTOR) {  // we have been asked for an array.
        OleSetTypeError(_T("Sorry - can't support VT_VECTOR arguments"));
        return FALSE;
    }
    BOOL rc = TRUE;
    PyObject *obUse = NULL;
    switch (m_reqdType) {
        case VT_VARIANT:
            // If the m_reqdType is VARIANT or unknown, let the Python type decide.
            rc = PyCom_VariantFromPyObject(obj, var);
            m_reqdType = V_VT(var);
            break;

        case VT_VARIANT | VT_BYREF:
            if (bCreateBuffers) {
                m_varBuf = new VARIANT;
                VariantInit(m_varBuf);
                V_VARIANTREF(var) = m_varBuf;
            }
            else
                VariantClear(V_VARIANTREF(var));
            if (!VALID_BYREF_MISSING(obj)) {
                PyCom_VariantFromPyObject(obj, V_VARIANTREF(var));
            }
            else {
                // Byref variant itself must be NULL.
                // XXX - What is the correct VT_ in this scenario?
                V_VT(V_VARIANTREF(var)) = VT_NULL;
            }
            break;

        case VT_BSTR:
            if (PyBytes_Check(obj) || PyUnicode_Check(obj)) {
                if (!PyWinObject_AsBstr(obj, &V_BSTR(var)))
                    BREAK_FALSE
            }
            else {
                // Use str(object) instead!
                if ((obUse = PyObject_Str(obj)) == NULL)
                    BREAK_FALSE
                if (!PyWinObject_AsBstr(obUse, &V_BSTR(var)))
                    BREAK_FALSE
            }
            break;
        case VT_BSTR | VT_BYREF:
            if (bCreateBuffers)
                V_BSTRREF(var) = (BSTR *)&m_pValueHolder;
            else
                SysFreeString(*V_BSTRREF(var));

            *V_BSTRREF(var) = NULL;

            if (!VALID_BYREF_MISSING(obj)) {
                if (PyBytes_Check(obj) || PyUnicode_Check(obj)) {
                    if (!PyWinObject_AsBstr(obj, V_BSTRREF(var)))
                        BREAK_FALSE
                }
                else {
                    // Use str(object) instead!
                    if ((obUse = PyObject_Str(obj)) == NULL)
                        BREAK_FALSE
                    if (!PyWinObject_AsBstr(obUse, V_BSTRREF(var)))
                        BREAK_FALSE
                }
            }
            break;
        case VT_I8:
            if (!PyWinObject_AsPY_LONG_LONG(obj, &V_I8(var)))
                BREAK_FALSE;
            break;
        case VT_I8 | VT_BYREF:
            if (bCreateBuffers)
                V_I8REF(var) = &m_llBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if (!PyWinObject_AsPY_LONG_LONG(obj, V_I8REF(var)))
                    BREAK_FALSE;
            }
            else
                *V_I8REF(var) = 0;
            break;
        case VT_UI8:
            if (!PyWinObject_AsUPY_LONG_LONG(obj, &V_UI8(var)))
                BREAK_FALSE;
            break;
        case VT_UI8 | VT_BYREF:
            if (bCreateBuffers)
                V_UI8REF(var) = (ULONGLONG *)&m_llBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if (!PyWinObject_AsUPY_LONG_LONG(obj, V_UI8REF(var)))
                    BREAK_FALSE;
            }
            else
                *V_UI8REF(var) = 0;
            break;
        case VT_I4:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_I4(var) = PyLong_AsLong(obUse);
            if (V_I4(var) == -1 && PyErr_Occurred())
                BREAK_FALSE;
            break;
        case VT_I4 | VT_BYREF:
            if (bCreateBuffers)
                V_I4REF(var) = &m_lBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_I4REF(var) = PyLong_AsLong(obUse);
                if (*V_I4REF(var) == -1 && PyErr_Occurred())
                    BREAK_FALSE;
            }
            else
                *V_I4REF(var) = 0;
            break;
        case VT_UI4:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_UI4(var) = PyLong_AsUnsignedLongMask(obUse);
            if (V_UI4(var) == (unsigned long)-1 && PyErr_Occurred())
                BREAK_FALSE;
            break;
        case VT_UI4 | VT_BYREF:
            if (bCreateBuffers)
                V_UI4REF(var) = (unsigned long *)&m_lBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_UI4REF(var) = PyLong_AsUnsignedLongMask(obUse);
                if (*V_UI4REF(var) == (unsigned long)-1 && PyErr_Occurred())
                    BREAK_FALSE;
            }
            else
                *V_UI4REF(var) = 0;
            break;
        case VT_I2:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_I2(var) = (short)PyLong_AsLong(obUse);
            break;
        case VT_I2 | VT_BYREF:
            if (bCreateBuffers)
                V_I2REF(var) = &m_sBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_I2REF(var) = (short)PyLong_AsLong(obUse);
            }
            else
                *V_I2REF(var) = 0;
            break;
        case VT_UI2:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_UI2(var) = (short)PyLong_AsUnsignedLongMask(obUse);
            break;
        case VT_UI2 | VT_BYREF:
            if (bCreateBuffers)
                V_UI2REF(var) = (unsigned short *)&m_sBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_UI2REF(var) = (unsigned short)PyLong_AsUnsignedLongMask(obUse);
            }
            else
                *V_UI2REF(var) = 0;
            break;
        case VT_I1:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_I1(var) = (CHAR)PyLong_AsLong(obUse);
            break;
        case VT_I1 | VT_BYREF:
            if (bCreateBuffers)
                V_I1REF(var) = (char *)&m_sBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_I1REF(var) = (CHAR)PyLong_AsLong(obUse);
            }
            else
                *V_I1REF(var) = 0;
            break;
        case VT_UI1:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_UI1(var) = (BYTE)PyLong_AsLong(obUse);
            break;
        case VT_UI1 | VT_BYREF:
            if (bCreateBuffers)
                V_UI1REF(var) = (BYTE *)&m_sBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_UI1REF(var) = (BYTE)PyLong_AsLong(obUse);
            }
            else
                *V_UI1REF(var) = 0;
            break;
        case VT_BOOL:
            if ((obUse = PyNumber_Long(obj)) == NULL)
                BREAK_FALSE
            V_BOOL(var) = PyLong_AsLong(obUse) ? VARIANT_TRUE : VARIANT_FALSE;
            break;
        case VT_BOOL | VT_BYREF:
            if (bCreateBuffers)
                // this is used in MSVC4.2 and after
                var->pboolVal = &m_boolBuf;
            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *(var->pboolVal) = PyLong_AsLong(obj) ? VARIANT_TRUE : VARIANT_FALSE;
            }
            else
                *(var->pboolVal) = 0;
            break;
        case VT_R8:
            if ((obUse = PyNumber_Float(obj)) == NULL)
                BREAK_FALSE
            V_R8(var) = PyFloat_AsDouble(obUse);
            break;
        case VT_R8 | VT_BYREF:
            if (bCreateBuffers)
                V_R8REF(var) = &m_dBuf;
            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Float(obj)) == NULL)
                    BREAK_FALSE
                *V_R8REF(var) = PyFloat_AsDouble(obUse);
            }
            else
                *V_R8REF(var) = 0.0;
            break;
        case VT_R4:
            if ((obUse = PyNumber_Float(obj)) == NULL)
                BREAK_FALSE
            V_R4(var) = (float)PyFloat_AsDouble(obUse);
            break;
        case VT_R4 | VT_BYREF:
            if (bCreateBuffers)
                V_R4REF(var) = &m_fBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Float(obj)) == NULL)
                    BREAK_FALSE
                *V_R4REF(var) = (float)PyFloat_AsDouble(obUse);
            }
            else
                *V_R4REF(var) = (float)0.0;
            break;

        case VT_NULL:
            break;
        case VT_DISPATCH:
            V_DISPATCH(var) = NULL;
            if (!PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IDispatch, (void **)&V_DISPATCH(var), TRUE))
                BREAK_FALSE;
            // COM Reference added by InterfaceFrom...
            break;
        case VT_DISPATCH | VT_BYREF:
            if (bCreateBuffers) {
                V_DISPATCHREF(var) = &m_dispBuf;
                m_dispBuf = NULL;
            }
            else {
                if (*V_DISPATCHREF(var))
                    (*V_DISPATCHREF(var))->Release();
                *V_DISPATCHREF(var) = NULL;
            }

            if (!PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IDispatch, (void **)V_DISPATCHREF(var), TRUE))
                BREAK_FALSE;
            // COM Reference added by InterfaceFrom...
            break;
        case VT_UNKNOWN:
            V_UNKNOWN(var) = NULL;
            if (!PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IUnknown, (void **)&V_UNKNOWN(var), TRUE))
                BREAK_FALSE;
            // COM Reference added by InterfaceFrom...
            break;
        case VT_UNKNOWN | VT_BYREF:
            if (bCreateBuffers) {
                V_UNKNOWNREF(var) = &m_unkBuf;
                m_unkBuf = NULL;
            }
            else {
                if (*V_UNKNOWNREF(var))
                    (*V_UNKNOWNREF(var))->Release();
                *V_UNKNOWNREF(var) = NULL;
            }
            m_unkBuf = NULL;
            if (!PyCom_InterfaceFromPyInstanceOrObject(obj, IID_IDispatch, (void **)V_UNKNOWNREF(var), TRUE))
                BREAK_FALSE;
            // COM Reference added by InterfaceFrom...
            break;
        case VT_DATE:
            if (!PyWinObject_AsDATE(obj, &V_DATE(var)))
                BREAK_FALSE;
            break;
        case VT_DATE | VT_BYREF:
            if (bCreateBuffers)
                V_DATEREF(var) = &m_dateBuf;
            if (!VALID_BYREF_MISSING(obj)) {
                if (!PyWinTime_Check(obj))
                    BREAK_FALSE;
                if (!PyWinObject_AsDATE(obj, V_DATEREF(var)))
                    BREAK_FALSE;
            }
            else
                *V_DATEREF(var) = 0;
            break;
        case VT_ERROR:
            V_ERROR(var) = DISP_E_PARAMNOTFOUND;  // should this be PyObject_Int??
            break;
        case VT_ERROR | VT_BYREF:
            if (bCreateBuffers)
                V_ERRORREF(var) = &m_lBuf;

            if (!VALID_BYREF_MISSING(obj)) {
                if ((obUse = PyNumber_Long(obj)) == NULL)
                    BREAK_FALSE
                *V_ERRORREF(var) = PyLong_AsLong(obUse);
            }
            else
                *V_ERRORREF(var) = 0;
            break;
        case VT_EMPTY:
            if (obj != Py_None) {
                PyErr_SetString(PyExc_TypeError, "None must be used for VT_EMPTY variables.");
                BREAK_FALSE;
            }
            // Nothing else to do - the code below sets the VT up correctly.
            break;
        case VT_RECORD:
        case VT_RECORD | VT_BYREF:
            rc = PyObject_AsVARIANTRecordInfo(obj, var);
            break;
        case VT_CY:
            rc = PyObject_AsCurrency(obj, &V_CY(var));
            break;
        case VT_CY | VT_BYREF:
            if (bCreateBuffers)
                V_CYREF(var) = &m_cyBuf;
            if (!VALID_BYREF_MISSING(obj)) {
                if (!PyObject_AsCurrency(obj, V_CYREF(var)))
                    BREAK_FALSE;
            }
            else
                V_CYREF(var)->int64 = 0;
            break;
        case VT_DECIMAL:
            rc = PyObject_AsDecimal(obj, &V_DECIMAL(var));
            break;
        case VT_DECIMAL | VT_BYREF:
            if (bCreateBuffers)
                V_DECIMALREF(var) = &m_decBuf;
            if (!VALID_BYREF_MISSING(obj)) {
                if (!PyObject_AsDecimal(obj, V_DECIMALREF(var)))
                    BREAK_FALSE;
            }
            else
                memset(V_DECIMALREF(var), 0, sizeof(DECIMAL));
            break;
        default:
            // could try default, but this error indicates we need to
            // beef up the VARIANT support, rather than default.
            TCHAR buf[200];
            wsprintf(buf, _T("The VARIANT type is unknown (0x%08lx)"), m_reqdType);
            OleSetTypeError(buf);
            rc = FALSE;
            break;
    }
    Py_XDECREF(obUse);
    if (rc)
        V_VT(var) = m_reqdType;
    return rc;
}

PyObject *PythonOleArgHelper::MakeVariantToObj(VARIANT *var)
{
    // If m_bMadeObjToVariant == TRUE, then we have previously converted from
    // an Obj to a Variant - ie, this is a "normal" COM call.
    // Otherwise, this is probably handling a COM event, where this conversion is
    // performed first, then Python called, then the ObjToVariant conversion will
    // happen later.  In this case, remember the buffer for the Variant

    // Check my logic still holds up - basically we can't call this twice on the same object.
    assert(m_convertDirection == POAH_CONVERT_UNKNOWN || m_convertDirection == POAH_CONVERT_FROM_PYOBJECT);
    // If this is the "driving" conversion, then the callers owns the buffers - we just use-em
    if (m_convertDirection == POAH_CONVERT_UNKNOWN) {
        m_convertDirection = POAH_CONVERT_FROM_VARIANT;
        m_bIsOut = V_ISBYREF(var);  // assume byref args are out params.
        m_reqdType = V_VT(var);
    }

    PyObject *ret = PyCom_PyObjectFromVariant(var);
    // If this helper is for a Python Variant, update it.
    if (ret && m_pyVariant) {
        if (PyObject_SetAttrString(m_pyVariant, "value", ret) == -1) {
            Py_DECREF(ret);
            return NULL;
        }
    }
    return ret;
}

BOOL MakePythonArgumentTuples(PyObject **ppArgs, PythonOleArgHelper **ppHelpers, PyObject **ppNamedArgs,
                              PythonOleArgHelper **ppNamedHelpers, DISPPARAMS FAR *params)
{
    *ppArgs = *ppNamedArgs = NULL;
    *ppArgs = PyTuple_New(params->cArgs);
    if (*ppArgs == NULL)
        return FALSE;
    *ppNamedArgs = PyDict_New();
    if (*ppNamedArgs == NULL) {
        Py_DECREF(*ppArgs);
        return FALSE;
    }

    *ppHelpers = *ppNamedHelpers = NULL;
    *ppHelpers = new PythonOleArgHelper[params->cArgs];
    *ppNamedHelpers = new PythonOleArgHelper[params->cNamedArgs];
    if (params->cArgs > 0)
        for (int arg = params->cArgs - 1; arg >= 0; arg--)
            PyTuple_SetItem(*ppArgs, params->cArgs - arg - 1,
                            (*ppHelpers)[(unsigned)arg].MakeVariantToObj(params->rgvarg + (unsigned)arg));
    return TRUE;
}

BOOL PyCom_MakeOlePythonCall(PyObject *handler, DISPPARAMS FAR *params, VARIANT FAR *pVarResult,
                             EXCEPINFO FAR *pexcepinfo, UINT FAR *puArgErr, PyObject *addnlArgs)
{
    PythonOleArgHelper *pHelpers = NULL;
    PythonOleArgHelper *pNamedHelpers = NULL;
    PyObject *argList = NULL;
    PyObject *namedArgList = NULL;
    if (params) {
        if (!MakePythonArgumentTuples(&argList, &pHelpers, &namedArgList, &pNamedHelpers, params)) {
            PyErr_Clear();
            return FALSE;
        }
    }
    if (addnlArgs) {
        PyObject *varArgs = argList;
        argList = Py_BuildValue("OO", varArgs, addnlArgs);
        Py_DECREF(varArgs);
    }
    PyObject *result = PyObject_CallObject(handler, argList);
    Py_XDECREF(argList);
    Py_XDECREF(namedArgList);
    // handlers reference cleaned up by virtual manager.
    BOOL bOK = (result != NULL);
    if (result) {
        // If result is a tuple, then the Python code
        // wishes to set some "byval" arguments.
        // make the return type
        PyObject *simpleRet;
        if (PyTuple_Check(result) && PyTuple_Size(result)) {
            simpleRet = PyTuple_GetItem(result, 0);
            UINT retNumber = 1;
            UINT retTotal = (UINT)PyTuple_Size(result);

            // Params are reverse order - loop from the back.
            for (unsigned int param = params->cArgs; param != 0 && retNumber < retTotal; param--) {
                if (pHelpers[param - 1].m_bIsOut) {
                    PyObject *val = PyTuple_GetItem(result, retNumber);
                    pHelpers[param - 1].MakeObjToVariant(val, params->rgvarg + param - 1, NULL);
                    retNumber++;
                }
            }
        }
        else {
            simpleRet = result;
        }
        PyCom_VariantFromPyObject(simpleRet, pVarResult);
        Py_DECREF(result);
    }
    delete[] pHelpers;
    delete[] pNamedHelpers;
    return bOK;
}
