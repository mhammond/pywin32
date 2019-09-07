// General utilities and conversion routines for MAPI support.
#include "pywintypes.h"
#include "PythonCOM.h"
#include "PyMAPIUtil.h"
// @doc

PyObject *PyMAPIObject_FromTypedUnknown(ULONG typ, IUnknown *pUnk, BOOL bAddRef)
{
    const IID *pIID;
    switch (typ) {
        case MAPI_FOLDER:
            pIID = &IID_IMAPIFolder;
            break;
        case MAPI_SESSION:
            pIID = &IID_IMAPISession;
            break;
        case MAPI_MESSAGE:
            pIID = &IID_IMessage;
            break;
        case MAPI_ATTACH:
            pIID = &IID_IAttachment;
            break;
        case MAPI_MAILUSER:
            pIID = &IID_IMailUser;
            break;
        case MAPI_DISTLIST:
            pIID = &IID_IDistList;
            break;
        case MAPI_ADDRBOOK:
            pIID = &IID_IAddrBook;
            break;
        case MAPI_ABCONT:
            pIID = &IID_IABContainer;
            break;
        case MAPI_STATUS:
            pIID = &IID_IMAPIStatus;
            break;
        case MAPI_PROFSECT:
            pIID = &IID_IProfSect;
            break;

        case MAPI_STORE:
        case MAPI_FORMINFO:
            pIID = &IID_IUnknown;
            break;
        default:
            pIID = &IID_IUnknown;
            break;
    }
    return PyCom_PyObjectFromIUnknown(pUnk, *pIID, bAddRef);
}

PyObject *PyObject_FromMAPIERROR(MAPIERROR *e, BOOL bIsUnicode, BOOL free_buffer)
{
    PyObject *obError = PyWinObject_FromMAPIStr((LPTSTR)e->lpszError, bIsUnicode);
    PyObject *obComp = PyWinObject_FromMAPIStr((LPTSTR)e->lpszComponent, bIsUnicode);

    PyObject *ret = Py_BuildValue("lOOll", e->ulVersion, obError, obComp, e->ulLowLevelError, e->ulContext);
    Py_XDECREF(obError);
    Py_XDECREF(obComp);
    if (free_buffer)
        MAPIFreeBuffer(e);
    return ret;
}

BOOL AllocMVBuffer(PyObject *seq, size_t itemSize, void *pAllocMoreLinkBlock, void **pbuf, ULONG *pLen)
{
    if (!PySequence_Check(seq)) {
        PyErr_SetString(PyExc_TypeError, "A multi-valued SPropValue item must be a sequence");
        return FALSE;
    }
    *pLen = PySequence_Length(seq);
    int bufSize = *pLen * itemSize;
    HRESULT hr = MAPIAllocateMore(bufSize, pAllocMoreLinkBlock, (void **)pbuf);
    if (S_OK != hr) {
        OleSetOleError(hr);
        return FALSE;
    }
    return TRUE;
}

#define MAKE_MV(type, pAllocMoreLinkBlock, array, cvals, PyConverter)                   \
    ok = AllocMVBuffer(ob, sizeof(type), pAllocMoreLinkBlock, (void **)&array, &cvals); \
    if (!ok)                                                                            \
        break;                                                                          \
    for (i = 0; !PyErr_Occurred() && i < cvals; i++) {                                  \
        PyObject *obmv = PySequence_GetItem(ob, i);                                     \
        if (obmv == NULL)                                                               \
            break;                                                                      \
        array[i] = (type)PyConverter(obmv);                                             \
        Py_DECREF(obmv);                                                                \
    }                                                                                   \
    break;

#define MAKEB_MV(type, pAllocMoreLinkBlock, array, cvals, PyConverter)                  \
    ok = AllocMVBuffer(ob, sizeof(type), pAllocMoreLinkBlock, (void **)&array, &cvals); \
    if (!ok)                                                                            \
        break;                                                                          \
    for (i = 0; ok && !PyErr_Occurred() && i < cvals; i++) {                            \
        PyObject *obmv = PySequence_GetItem(ob, i);                                     \
        if (obmv == NULL)                                                               \
            break;                                                                      \
        ok = PyConverter(obmv, array + i);                                              \
        Py_DECREF(obmv);                                                                \
    }                                                                                   \
    break;

// @object PySPropValue|A MAPI property value.  Property values can either be passed from
// python into MAPI functions, or returned from MAPI functions to Python.
BOOL PyMAPIObject_AsSPropValue(PyObject *Valob, SPropValue *pv, void *pAllocMoreLinkBlock)
{
    PyObject *ob;
    // @pyparm ULONG|propType||The type of the MAPI property
    // @pyparm object|value||The property value
    if (!PyArg_ParseTuple(Valob, "kO:SPropValue item", &pv->ulPropTag, &ob)) {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "An SPropValue item must be a tuple of (integer, object)");
        return NULL;
    }
    BOOL ok = TRUE;
    unsigned int i;
    PyErr_Clear();
    // @comm The parameters can be one of the following pairs of values.
    // @flagh propType|value
    switch (PROP_TYPE(pv->ulPropTag)) {
        // @flag PT_I2|An integer
        case PT_I2:  //		case PT_SHORT:
            pv->Value.i = (int)PyInt_AsLong(ob);
            break;
        // @flag PT_MV_I2|A sequence of integers
        case PT_MV_I2:
            MAKE_MV(short int, pAllocMoreLinkBlock, pv->Value.MVi.lpi, pv->Value.MVi.cValues, PyInt_AsLong)
        // @flag PT_I4|An integer
        case PT_I4:  //		case PT_LONG:
            pv->Value.l = PyInt_AsLong(ob);
            break;
        // @flag PT_MV_I4|A sequence of integers
        case PT_MV_I4:
            MAKE_MV(long, pAllocMoreLinkBlock, pv->Value.MVl.lpl, pv->Value.MVl.cValues, PyInt_AsLong)
        // @flag PT_R4|A float
        case PT_R4:  //		case PT_FLOAT:
            pv->Value.flt = (float)PyFloat_AsDouble(ob);
            break;
        // @flag PT_MV_R4|A sequence of floats
        case PT_MV_R4:
            MAKE_MV(float, pAllocMoreLinkBlock, pv->Value.MVflt.lpflt, pv->Value.MVflt.cValues, PyFloat_AsDouble)
        // @flag PT_R8|A float
        case PT_R8:  //		case PT_DOUBLE:
            pv->Value.dbl = PyFloat_AsDouble(ob);
            break;
        // @flag PT_MV_R8|A sequence of floats
        case PT_MV_R8:
            MAKE_MV(double, pAllocMoreLinkBlock, pv->Value.MVdbl.lpdbl, pv->Value.MVdbl.cValues, PyFloat_AsDouble)
        // @flag PT_BOOLEAN|A boolean value (or an int)
        case PT_BOOLEAN:
            pv->Value.b = PyInt_AsLong(ob) ? VARIANT_TRUE : VARIANT_FALSE;
            break;

            /*
                    case PT_CURRENCY:
                        p->Value.cur ??
                        break;

            */
        // @flag PT_APPTIME|A <o PyTime> object
        case PT_APPTIME:
            ok = PyWinObject_AsDATE(ob, &pv->Value.at);
            break;

        // @flag PT_MV_APPTIME|An sequence of <o PyTime> object
        case PT_MV_APPTIME:
            MAKEB_MV(double, pAllocMoreLinkBlock, pv->Value.MVat.lpat, pv->Value.MVat.cValues, PyWinObject_AsDATE)

        // @flag PT_SYSTIME|A <o PyTime> object
        case PT_SYSTIME:
            ok = PyWinObject_AsFILETIME(ob, &pv->Value.ft);
            break;

        // @flag PT_MV_APPTIME|An sequence of <o PyTime> object
        case PT_MV_SYSTIME:
            MAKEB_MV(FILETIME, pAllocMoreLinkBlock, pv->Value.MVft.lpft, pv->Value.MVft.cValues, PyWinObject_AsFILETIME)

        // @flag PT_STRING8|A string or <o PyUnicode>
        case PT_STRING8: {  // Copy into new MAPI memory block
            DWORD bufLen;
            char *str;
            ok = PyWinObject_AsString(ob, &str, FALSE, &bufLen);
            if (ok) {
                bufLen++;
                HRESULT hr = MAPIAllocateMore(bufLen, pAllocMoreLinkBlock, (void **)&pv->Value.lpszA);
                if (S_OK != hr) {
                    OleSetOleError(hr);
                    ok = FALSE;
                }
                else {
                    memcpy(pv->Value.lpszA, str, bufLen - sizeof(char));
                    // Null terminate
                    memcpy(((char *)pv->Value.lpszA) + (bufLen - sizeof(char)), "\0", sizeof(char));
                }
            }
            PyWinObject_FreeString(str);
            break;
        }

        // @flag PT_STRING8|A sequence of string or <o PyUnicode> objects.
        case PT_MV_STRING8:
            ok = AllocMVBuffer(ob, sizeof(char *), pAllocMoreLinkBlock, (void **)&pv->Value.MVszA.lppszA,
                               &pv->Value.MVszA.cValues);
            if (!ok)
                break;
            for (i = 0; ok && !PyErr_Occurred() && i < pv->Value.MVszA.cValues; i++) {
                PyObject *obmv = PySequence_GetItem(ob, i);
                if (obmv == NULL)
                    break;

                DWORD bufLen;
                char *str;
                ok = PyWinObject_AsString(obmv, &str, FALSE, &bufLen);
                if (ok) {
                    bufLen++;
                    HRESULT hr = MAPIAllocateMore(bufLen, pAllocMoreLinkBlock, (void **)&pv->Value.MVszA.lppszA[i]);
                    if (S_OK != hr) {
                        OleSetOleError(hr);
                        ok = FALSE;
                    }
                    else {
                        memcpy(pv->Value.MVszA.lppszA[i], str, bufLen - sizeof(char));
                        // Null terminate
                        memcpy(((char *)pv->Value.MVszA.lppszA[i]) + (bufLen - sizeof(char)), "\0", sizeof(char));
                    }
                }
                PyWinObject_FreeString(str);
                Py_DECREF(obmv);
            }
            break;

        // @flag PT_UNICODE|A string or <o PyUnicode>
        case PT_UNICODE: {  // Bit of a hack - need to copy into MAPI block.
            BSTR wstr = NULL;
            ok = PyWinObject_AsBstr(ob, &wstr, FALSE);
            if (ok) {
                DWORD bufSize = sizeof(WCHAR) * (SysStringLen(wstr) + 1);
                HRESULT hr = MAPIAllocateMore(bufSize, pAllocMoreLinkBlock, (void **)&pv->Value.lpszW);
                if (S_OK != hr) {
                    OleSetOleError(hr);
                    ok = FALSE;
                }
                else {
                    memcpy(pv->Value.lpszW, wstr, bufSize - 2);
                    // Null terminate
                    memcpy(((char *)pv->Value.lpszW) + (bufSize - 2), "\0\0", 2);
                }
            }
            SysFreeString(wstr);
            break;
        }

        // @flag PT_MV_UNICODE|A sequence of string or <o PyUnicode>
        case PT_MV_UNICODE:
            ok = AllocMVBuffer(ob, sizeof(char *), pAllocMoreLinkBlock, (void **)&pv->Value.MVszW.lppszW,
                               &pv->Value.MVszW.cValues);
            if (!ok)
                break;
            for (i = 0; ok && !PyErr_Occurred() && i < pv->Value.MVszW.cValues; i++) {
                PyObject *obmv = PySequence_GetItem(ob, i);
                if (obmv == NULL)
                    break;

                BSTR wstr = NULL;
                ok = PyWinObject_AsBstr(obmv, &wstr, FALSE);
                if (ok) {
                    DWORD bufSize = sizeof(WCHAR) * (SysStringLen(wstr) + 1);
                    HRESULT hr = MAPIAllocateMore(bufSize, pAllocMoreLinkBlock, (void **)&pv->Value.MVszW.lppszW[i]);
                    if (S_OK != hr) {
                        OleSetOleError(hr);
                        ok = FALSE;
                    }
                    else {
                        memcpy(pv->Value.MVszW.lppszW[i], wstr, bufSize - 2);
                        // Null terminate
                        memcpy(((char *)pv->Value.MVszW.lppszW[i]) + (bufSize - 2), "\0\0", 2);
                    }
                }
                SysFreeString(wstr);
                Py_DECREF(obmv);
            }
            break;

        // @flag PT_BINARY|A string containing binary data
        case PT_BINARY:
            pv->Value.bin.lpb = (unsigned char *)PyString_AsString(ob);
            pv->Value.bin.cb = PyString_Size(ob);
            break;

        // @flag PT_MV_BINARY|A sequence of strings containing binary data
        case PT_MV_BINARY:
            ok = AllocMVBuffer(ob, sizeof(SBinary), pAllocMoreLinkBlock, (void **)&pv->Value.MVbin.lpbin,
                               &pv->Value.MVbin.cValues);
            for (i = 0; !PyErr_Occurred() && i < pv->Value.MVbin.cValues; i++) {
                PyObject *obmv = PySequence_GetItem(ob, i);
                if (obmv == NULL)
                    break;
                if (!PyString_Check(obmv)) {
                    Py_DECREF(obmv);
                    PyErr_SetString(PyExc_TypeError, "PT_MV_BINARY elements must be strings");
                    break;
                }
                pv->Value.MVbin.lpbin[i].lpb = (unsigned char *)PyString_AsString(obmv);
                pv->Value.MVbin.lpbin[i].cb = PyString_Size(obmv);
                Py_DECREF(obmv);
            }
            break;

        // @flag PT_CLSID|A <o PyIID>
        case PT_CLSID: {
            HRESULT hr = MAPIAllocateMore(sizeof(CLSID), pAllocMoreLinkBlock, (void **)&pv->Value.lpguid);
            if (S_OK != hr) {
                OleSetOleError(hr);
                ok = FALSE;
            }
            else
                ok = PyWinObject_AsIID(ob, pv->Value.lpguid);
            break;
        }

        // @flag PT_MV_CLSID|A sequence of <o PyIID> objects
        case PT_MV_CLSID:
            MAKEB_MV(CLSID, pAllocMoreLinkBlock, pv->Value.MVguid.lpguid, pv->Value.MVguid.cValues, PyWinObject_AsIID)

        // @flag PT_I8|A <o PyLARGE_INTEGER>
        case PT_I8:
            //		case PT_LONGLONG:
            ok = PyWinObject_AsLARGE_INTEGER(ob, &pv->Value.li);
            break;

        // @flag PT_MV_I8|A sequence of <o PyLARGE_INTEGER>
        case PT_MV_I8:
            MAKEB_MV(LARGE_INTEGER, pAllocMoreLinkBlock, pv->Value.MVli.lpli, pv->Value.MVli.cValues,
                     PyWinObject_AsLARGE_INTEGER)

        // @flag PT_ERROR|An integer error code.
        case PT_ERROR:
            pv->Value.err = (SCODE)PyInt_AsLong(ob);
            break;

        // @flag PT_NULL|Anything!
        case PT_NULL:
            pv->Value.x = 0;
            break;

        default: {
            char buf[128];
            sprintf(buf, "Unsupported MAPI property type 0x%X", PROP_TYPE(pv->ulPropTag));
            PyErr_SetString(PyExc_TypeError, buf);
            ok = FALSE;
        }
    }
    ok = (ok && !PyErr_Occurred());
    return ok;
}

PyObject *PyMAPIObject_FromSPropValue(SPropValue *pv)
{
    PyObject *val;
    ULONG i;
    switch (PROP_TYPE(pv->ulPropTag)) {
        case PT_I2:  //		case PT_SHORT:
            val = PyInt_FromLong(pv->Value.i);
            break;
        case PT_I4:  //		case PT_LONG:
            val = PyInt_FromLong(pv->Value.l);
            break;
        case PT_R4:  //		case PT_FLOAT:
            val = PyFloat_FromDouble(pv->Value.flt);
            break;
        case PT_R8:  //		case PT_DOUBLE:
            val = PyFloat_FromDouble(pv->Value.dbl);
            break;
        case PT_BOOLEAN:
            val = pv->Value.b ? Py_True : Py_False;
            Py_INCREF(val);
            break;
            /*
                    case PT_CURRENCY:
                        pv->Value.cur??
                        break;
            */
        case PT_APPTIME:
            val = PyWinObject_FromDATE(pv->Value.at);
            break;
        case PT_SYSTIME:
            val = PyWinObject_FromFILETIME(pv->Value.ft);
            break;
        case PT_STRING8:
            val = PyString_FromString(pv->Value.lpszA);
            break;
        case PT_UNICODE:
            val = PyWinObject_FromWCHAR(pv->Value.lpszW);
            break;
        case PT_BINARY:
            val = PyString_FromStringAndSize((char *)pv->Value.bin.lpb, pv->Value.bin.cb);
            break;

        case PT_CLSID:
            val = PyWinObject_FromIID(*pv->Value.lpguid);
            break;
        case PT_I8:
            //		case PT_LONGLONG:
            val = PyWinObject_FromLARGE_INTEGER(pv->Value.li);
            break;
        case PT_ERROR:
            val = PyInt_FromLong(pv->Value.err);
            break;

        case PT_NULL:
            val = Py_None;
            Py_INCREF(Py_None);
            break;

        case PT_MV_I2:
            val = PyTuple_New(pv->Value.MVi.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVi.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyInt_FromLong(pv->Value.MVi.lpi[i]));
            }
            break;
        case PT_MV_LONG:
            val = PyTuple_New(pv->Value.MVi.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVl.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyInt_FromLong(pv->Value.MVl.lpl[i]));
            }
            break;
        case PT_MV_R4:
            val = PyTuple_New(pv->Value.MVflt.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVflt.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyFloat_FromDouble(pv->Value.MVflt.lpflt[i]));
            }
            break;
        case PT_MV_DOUBLE:
            val = PyTuple_New(pv->Value.MVdbl.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVdbl.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyFloat_FromDouble(pv->Value.MVdbl.lpdbl[i]));
            }
            break;
            /*
                    case PT_MV_CURRENCY:
                        MVcur
                        SCurrencyArray
            */

        case PT_MV_APPTIME:
            val = PyTuple_New(pv->Value.MVat.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVat.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyWinObject_FromDATE(pv->Value.MVat.lpat[i]));
            }
            break;
        case PT_MV_SYSTIME:
            val = PyTuple_New(pv->Value.MVft.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVft.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyWinObject_FromFILETIME(pv->Value.MVft.lpft[i]));
            }
            break;

        case PT_MV_BINARY:
            val = PyTuple_New(pv->Value.MVbin.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVbin.cValues; i++)
                    PyTuple_SET_ITEM(
                        val, i,
                        PyString_FromStringAndSize((char *)pv->Value.MVbin.lpbin[i].lpb, pv->Value.MVbin.lpbin[i].cb));
            }
            break;
        case PT_MV_STRING8:
            val = PyTuple_New(pv->Value.MVszA.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVszA.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyString_FromString(pv->Value.MVszA.lppszA[i]));
            }
            break;
        case PT_MV_UNICODE:
            val = PyTuple_New(pv->Value.MVszW.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVszW.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyWinObject_FromWCHAR(pv->Value.MVszW.lppszW[i]));
            }
            break;

        case PT_MV_CLSID:
            val = PyTuple_New(pv->Value.MVguid.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVguid.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyWinObject_FromIID(pv->Value.MVguid.lpguid[i]));
            }
            break;

        case PT_MV_I8:
            val = PyTuple_New(pv->Value.MVli.cValues);
            if (val) {
                for (i = 0; i < pv->Value.MVli.cValues; i++)
                    PyTuple_SET_ITEM(val, i, PyWinObject_FromLARGE_INTEGER(pv->Value.MVli.lpli[i]));
            }
            break;

        case PT_OBJECT:
            val = PyInt_FromLong(pv->Value.x);
            break;

        default:
            printf("File %s: Unsupported MAPI property type 0x%X", __FILE__, PROP_TYPE(pv->ulPropTag));
            /* Dont set exception, as this prevents otherwise valid props from
               being returned
            */
            val = Py_None;
            Py_INCREF(Py_None);
            break;
    }

    PyObject *rc = PyTuple_New(2);
    if (rc == NULL) {
        Py_DECREF(val);
        PyErr_SetString(PyExc_MemoryError, "Tuple(2) for PROP result");
        return NULL;
    }
    PyTuple_SET_ITEM(rc, 0, PyLong_FromUnsignedLong(pv->ulPropTag));
    PyTuple_SET_ITEM(rc, 1, val);
    return rc;
}

PyObject *PyMAPIObject_FromSPropValueArray(SPropValue *pv, ULONG nvalues)
{
    PyObject *ret = PyList_New(nvalues);
    if (!ret)
        return NULL;
    ULONG i;
    for (i = 0; i < nvalues; i++) {
        PyObject *sub = PyMAPIObject_FromSPropValue(pv + i);
        if (!sub) {
            Py_DECREF(ret);
            return NULL;
        }
        PyList_SET_ITEM(ret, i, sub);
    }
    return ret;
}

// @object PySPropValueArray|A sequence of <o PySPropValue>, as passed to many MAPI functions.
BOOL PyMAPIObject_AsSPropValueArray(PyObject *obs, SPropValue **ppv, ULONG *pcValues)
{
    int seqLen = PySequence_Length(obs);
    SPropValue *pPV;
    HRESULT hr;
    if (S_OK != (hr = MAPIAllocateBuffer(sizeof(SPropValue) * seqLen, (void **)&pPV))) {
        OleSetOleError(hr);
        return FALSE;
    }
    for (ULONG i = 0; i < (ULONG)seqLen; i++) {
        PyObject *myob = PySequence_GetItem(obs, i);
        if (myob == NULL) {
            MAPIFreeBuffer(pPV);
            return FALSE;
        }
        BOOL rc = PyMAPIObject_AsSPropValue(myob, pPV + i, pPV);
        Py_DECREF(myob);
        if (!rc) {
            MAPIFreeBuffer(pPV);
            return FALSE;
        }
    }
    *pcValues = seqLen;
    *ppv = pPV;
    return TRUE;
}

// @object PySRowSet|A sequence of <o PySRow> objects, as passed to many MAPI functions.
BOOL PyMAPIObject_AsSRowSet(PyObject *obSeq, SRowSet **ppResult, BOOL bNoneOK)
{
    if (ppResult == NULL || obSeq == NULL)
        return FALSE;
    if (obSeq == NULL)
        return FALSE;
    *ppResult = NULL;
    if (obSeq == Py_None) {
        if (bNoneOK)
            return TRUE;
        PyErr_SetString(PyExc_ValueError, "None is not a valid SRowSet/ADRLIST in this context");
        return FALSE;
    }
    PyObject *rowObject = NULL;
    PyObject *propObject = NULL;
    BOOL rc = FALSE;
    HRESULT hr;
    ULONG i;
    DWORD allocSize;

    int seqLen = PySequence_Length(obSeq);

    if (seqLen == -1) {
        PyErr_SetString(PyExc_TypeError, "ADRLIST/SRowSet items must be a sequence");
        goto done;
    }

    allocSize = sizeof(SRowSet) + (sizeof(SRow) * seqLen);
    if (S_OK != (hr = MAPIAllocateBuffer(allocSize, (void **)ppResult))) {
        OleSetOleError(hr);
        goto done;
    }
    ZeroMemory(*ppResult, allocSize);  // so cleanup works correctly.
    (*ppResult)->cRows = seqLen;

    for (i = 0; i < (ULONG)seqLen; i++) {
        rowObject = PySequence_GetItem(obSeq, i);
        if (rowObject == NULL)
            goto done;
        // myob is expected to represent an SRow structure.  This is really an array
        // of property values.
        SRow *pRow = (*ppResult)->aRow + i;
        pRow->cValues = PySequence_Length(rowObject);
        if (pRow->cValues == -1)
            goto done;

        if (pRow->cValues == 0)
            pRow->lpProps = NULL;
        else {
            allocSize = sizeof(SPropValue) * pRow->cValues;
            hr = MAPIAllocateBuffer(allocSize, (void **)&pRow->lpProps);
            if (FAILED(hr)) {
                OleSetOleError(hr);
                goto done;
            }
            for (ULONG j = 0; j < pRow->cValues; j++) {
                propObject = PySequence_GetItem(rowObject, j);
                if (propObject == NULL)
                    goto done;
                if (!PyMAPIObject_AsSPropValue(propObject, pRow->lpProps + j, *ppResult))
                    goto done;
                Py_DECREF(propObject);
                propObject = NULL;  // important for cleanup
            }
        }
        Py_DECREF(rowObject);
        rowObject = NULL;  // important for cleanup
    }
    rc = TRUE;
done:
    if (!rc && (*ppResult)) {
        PyMAPIObject_FreeSRowSet(*ppResult);
    }
    Py_XDECREF(propObject);
    Py_XDECREF(rowObject);
    return rc;
}

void PyMAPIObject_FreeSRowSet(SRowSet *pResult)
{
    if (pResult) {
        for (ULONG i = 0; i < pResult->cRows; i++) MAPIFreeBuffer(pResult->aRow[i].lpProps);
        MAPIFreeBuffer(pResult);
    }
}

// @object PySRow|Identical to a <o PySValue> object
PyObject *PyMAPIObject_FromSRow(SRow *pr)
{
    PyObject *result = PyTuple_New(pr->cValues);
    if (result == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating SRow result");
        return NULL;
    }
    for (ULONG i = 0; i < pr->cValues; i++) {
        PyObject *obNew = PyMAPIObject_FromSPropValue(pr->lpProps + i);
        if (obNew == NULL) {
            Py_DECREF(result);
            return NULL;
        }
        PyTuple_SET_ITEM(result, i, obNew);
    }
    return result;
}

PyObject *PyMAPIObject_FromSRowSet(SRowSet *prs)
{
    PyObject *result = PyTuple_New(prs->cRows);
    if (result == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating SRowSet result");
        return NULL;
    }
    for (ULONG i = 0; i < prs->cRows; i++) {
        PyObject *obNew = PyMAPIObject_FromSRow(prs->aRow + i);
        if (obNew == NULL) {
            Py_DECREF(result);
            return NULL;
        }
        PyTuple_SET_ITEM(result, i, obNew);
    }
    return result;
}

// @object PySPropTagArray|A sequence of integers
BOOL PyMAPIObject_AsSPropTagArray(PyObject *obta, SPropTagArray **ppta)
{
    if (obta == Py_None) {
        *ppta = NULL;
        return TRUE;
    }
    BOOL bSeq = TRUE;
    int seqLen;
    if (PySequence_Check(obta)) {
        seqLen = PySequence_Length(obta);
    }
    else if (PyInt_Check(obta)) {
        seqLen = 1;
        bSeq = FALSE;
    }
    else {
        PyErr_SetString(PyExc_TypeError, "SPropTagArray must be a sequence of integers");
        return FALSE;
    }

    DWORD cBytes = (seqLen * sizeof(ULONG)) + sizeof(ULONG);
    SPropTagArray *pta;
    HRESULT hr = MAPIAllocateBuffer(cBytes, (void **)&pta);
    if (FAILED(hr)) {
        OleSetOleError(hr);
        return FALSE;
    }
    pta->cValues = seqLen;
    if (bSeq) {
        for (ULONG i = 0; i < (ULONG)seqLen; i++) {
            PyObject *obItem = PySequence_GetItem(obta, i);
            if (obItem == NULL) {
                MAPIFreeBuffer(pta);
                return FALSE;
            }
            pta->aulPropTag[i] = PyLong_AsUnsignedLong(obItem);
            if (PyErr_Occurred()) {
                Py_DECREF(obItem);
                MAPIFreeBuffer(pta);
                return FALSE;
            }
            Py_DECREF(obItem);
        }
    }
    else {
        // Simple int.
        pta->aulPropTag[0] = PyLong_AsUnsignedLong(obta);
    }
    *ppta = pta;
    return TRUE;
}

void PyMAPIObject_FreeSPropTagArray(SPropTagArray *pta)
{
    if (pta)
        MAPIFreeBuffer(pta);
}

PyObject *PyMAPIObject_FromSPropTagArray(SPropTagArray *pta)
{
    if (!pta) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = PyTuple_New(pta->cValues);
    for (ULONG i = 0; i < pta->cValues; i++) {
        PyTuple_SET_ITEM(ret, i, PyLong_FromUnsignedLong(pta->aulPropTag[i]));
    }
    return ret;
}

// @object PySBinaryArray|A sequence of strings containing binary data.
BOOL PyMAPIObject_AsSBinaryArray(PyObject *ob, SBinaryArray *pba)
{
    BOOL bSeq = TRUE;
    int seqLen;
    if (PyString_Check(ob)) {
        seqLen = 1;
        bSeq = FALSE;
    }
    else if (PySequence_Check(ob)) {
        seqLen = PySequence_Length(ob);
    }
    else {
        PyErr_SetString(PyExc_TypeError, "SBinaryArray must be a sequence of strings");
        return FALSE;
    }
    DWORD cBytes = (seqLen * sizeof(SBinary));
    SBinary *pBin;
    HRESULT hr = MAPIAllocateBuffer(cBytes, (void **)&pBin);
    pba->lpbin = pBin;
    if (FAILED(hr)) {
        OleSetOleError(hr);
        return FALSE;
    }
    pba->cValues = seqLen;
    if (bSeq) {
        for (ULONG i = 0; i < (ULONG)seqLen; i++) {
            PyObject *obItem = PySequence_GetItem(ob, i);
            if (obItem == NULL) {
                MAPIFreeBuffer(pba);
                return FALSE;
            }
            if (!PyString_Check(obItem)) {
                PyErr_SetString(PyExc_TypeError, "SBinary must be a string");
                Py_DECREF(obItem);
                MAPIFreeBuffer(pba);
                return FALSE;
            }
            pBin[i].cb = PyString_Size(obItem);
            pBin[i].lpb = (LPBYTE)PyString_AsString(obItem);
            Py_DECREF(obItem);
        }
    }
    else {
        if (!PyString_Check(ob)) {
            PyErr_SetString(PyExc_TypeError, "SBinary must be a string");
            MAPIFreeBuffer(pba);
            return FALSE;
        }
        // Simple string
        pBin[0].cb = PyString_Size(ob);
        pBin[0].lpb = (LPBYTE)PyString_AsString(ob);
    }
    return TRUE;
}

void PyMAPIObject_FreeSBinaryArray(SBinaryArray *pv)
{
    if (pv->lpbin)
        MAPIFreeBuffer(pv->lpbin);
}

// @object PyMAPINAMEIDArray|A sequence (<o PyIID>, string/int) objects
BOOL PyMAPIObject_AsMAPINAMEIDArray(PyObject *ob, MAPINAMEID ***pppNameId, ULONG *pNumIds, BOOL bNoneOK /*= FALSE*/)
{
    if (bNoneOK && ob == Py_None) {
        *pppNameId = NULL;
        *pNumIds = 0;
        return TRUE;
    }
    PyErr_Clear();
    ULONG len = (ULONG)PySequence_Length(ob);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError, "MAPINAMEID array list be a sequence of tuples");
        return FALSE;
    }
    MAPINAMEID **ppNew = NULL;
    MAPINAMEID *prgIds = NULL;
    IID *pIIDs = NULL;
    HRESULT hr = MAPIAllocateBuffer(len * sizeof(MAPINAMEID *), (void **)&ppNew);
    if (SUCCEEDED(hr))
        hr = MAPIAllocateMore(len * sizeof(MAPINAMEID), ppNew, (void **)&prgIds);
    if (SUCCEEDED(hr))
        hr = MAPIAllocateMore(len * sizeof(IID), ppNew, (void **)&pIIDs);
    if (FAILED(hr)) {
        MAPIFreeBuffer(ppNew);
        OleSetOleError(hr);
        return FALSE;
    }
    for (ULONG i = 0; i < len; i++) {
        ppNew[i] = prgIds + i;
        MAPINAMEID *pNew = prgIds + i;
        PyObject *obIID, *obPropId;
        PyObject *pMe = PySequence_GetItem(ob, i);
        if (pMe == NULL) {
            goto loop_error;
        }
        if (!PyArg_ParseTuple(pMe, "OO", &obIID, &obPropId)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError, "MAPINAMEIDArray must be a sequence of (iid, string/int) tuples");
            goto loop_error;
        }

        pNew->lpguid = pIIDs + i;
        BSTR bstrVal;
        if (!PyWinObject_AsIID(obIID, pIIDs + i))
            goto loop_error;
        if (PyInt_Check(obPropId)) {
            pNew->ulKind = MNID_ID;
            pNew->Kind.lID = PyLong_AsUnsignedLong(obPropId);
        }
        else if (PyWinObject_AsBstr(obPropId, &bstrVal)) {
            // Make a copy of the string
            pNew->ulKind = MNID_STRING;
            DWORD strLen = SysStringLen(bstrVal);
            hr = MAPIAllocateMore(sizeof(WCHAR) * (strLen + 1), ppNew, (void **)&pNew->Kind.lpwstrName);
            if (FAILED(hr)) {
                PyWinObject_FreeBstr(bstrVal);
                OleSetOleError(hr);
                goto loop_error;
            }
            wcsncpy(pNew->Kind.lpwstrName, bstrVal, strLen + 1);
            PyWinObject_FreeBstr(bstrVal);
        }
        else {
            PyErr_SetString(PyExc_TypeError, "The type of property ID is invalid - must be string/unicode or int");
            goto loop_error;
        }
        Py_DECREF(pMe);
        continue;
    loop_error:
        Py_XDECREF(pMe);
        MAPIFreeBuffer(ppNew);
        return NULL;
    }
    *pppNameId = ppNew;
    *pNumIds = len;
    return TRUE;
}

void PyMAPIObject_FreeMAPINAMEIDArray(MAPINAMEID **pv) { MAPIFreeBuffer(pv); }

PyObject *PyMAPIObject_FromMAPINAMEIDArray(MAPINAMEID **pp, ULONG numEntries)
{
    PyObject *ret = PyList_New(numEntries);
    if (ret == NULL)
        return NULL;
    for (int i = 0; (ULONG)i < numEntries; i++) {
        MAPINAMEID *pLook = pp[i];
        PyObject *value, *guid;
        if (pLook == NULL) {
            value = Py_None;
            Py_INCREF(Py_None);
            guid = Py_None;
            Py_INCREF(Py_None);
        }
        else {
            value = pLook->ulKind == MNID_STRING ? PyWinObject_FromOLECHAR(pLook->Kind.lpwstrName)
                                                 : PyInt_FromLong(pLook->Kind.lID);
            guid = PyWinObject_FromIID(*pLook->lpguid);
        }
        PyObject *newItem = PyTuple_New(2);
        PyTuple_SetItem(newItem, 0, guid);
        PyTuple_SetItem(newItem, 1, value);
        PyList_SetItem(ret, i, newItem);
    }
    return ret;
}
// Restriction stuff!
BOOL PyMAPIObject_AsSingleSRestriction(PyObject *ob, SRestriction *pRest, void *pAllocMoreLinkBlock);

// @object PySExistRestriction|
BOOL PyMAPIObject_AsSExistRestriction(PyObject *ob, SExistRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm ULONG|propTag||The property ID to check for existance.
    // @pyparm int|reserved1|0|
    // @pyparm int|reserved2|0|
    pRest->ulReserved1 = pRest->ulReserved2 = 0;
    if (!PyArg_ParseTuple(ob, "k|ll:SExistRestriction tuple", &pRest->ulPropTag, &pRest->ulReserved1,
                          &pRest->ulReserved2))
        return FALSE;
    return TRUE;
}

// @object PySPropertyRestriction|
BOOL PyMAPIObject_AsSPropertyRestriction(PyObject *ob, SPropertyRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm int|relOp||
    // @pyparm ULONG|propTag||The property ID.
    // @pyparm <o PySPropValue>|propertyValue||
    PyObject *subOb;
    if (!PyArg_ParseTuple(ob, "lkO:SPropertyRestriction tuple", &pRest->relop, &pRest->ulPropTag, &subOb))
        return FALSE;
    HRESULT hr;
    if (FAILED((hr = MAPIAllocateMore(sizeof(SPropValue), pAllocMoreLinkBlock, (void **)&pRest->lpProp)))) {
        OleSetOleError(hr);
        return FALSE;
    }
    return PyMAPIObject_AsSPropValue(subOb, pRest->lpProp, pAllocMoreLinkBlock);
}

// @object PySContentRestriction|
BOOL PyMAPIObject_AsSContentRestriction(PyObject *ob, SContentRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm int|fuzzyLevel||
    // @pyparm ULONG|propTag||The property ID.
    // @pyparm <o PySPropValue>|propertyValue||
    PyObject *subOb;
    if (!PyArg_ParseTuple(ob, "lkO:SContentRestriction tuple", &pRest->ulFuzzyLevel, &pRest->ulPropTag, &subOb))
        return FALSE;
    HRESULT hr;
    if (FAILED((hr = MAPIAllocateMore(sizeof(SPropValue), pAllocMoreLinkBlock, (void **)&pRest->lpProp)))) {
        OleSetOleError(hr);
        return FALSE;
    }
    return PyMAPIObject_AsSPropValue(subOb, pRest->lpProp, pAllocMoreLinkBlock);
}

// @object PySNotRestriction|
BOOL PyMAPIObject_AsSNotRestriction(PyObject *ob, SNotRestriction *pRest, void *pAllocMoreLinkBlock)
{
    PyObject *subOb;
    pRest->ulReserved = 0;
    // @pyparm <o PySRestriction>|restriction||
    // @pyparm int|reserved|0|
    if (!PyArg_ParseTuple(ob, "O|l:SNotRestriction tuple", &subOb, &pRest->ulReserved))
        return FALSE;

    HRESULT hr;
    if (FAILED((hr = MAPIAllocateMore(sizeof(SRestriction), pAllocMoreLinkBlock, (void **)&pRest->lpRes)))) {
        OleSetOleError(hr);
        return FALSE;
    }
    return PyMAPIObject_AsSingleSRestriction(subOb, pRest->lpRes, pAllocMoreLinkBlock);
}

// @object PySAndRestriction|
BOOL PyMAPIObject_AsSAndRestriction(PyObject *ob, SAndRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm [<o PySRestriction>, ...]|restriction||A sequence of <o PySRestriction> objects.
    pRest->cRes = PySequence_Length(ob);
    if (pRest->cRes == (ULONG)-1 && PyErr_Occurred())
        return FALSE;

    HRESULT hr;
    BOOL ok = !FAILED(
        (hr = MAPIAllocateMore(sizeof(SRestriction) * pRest->cRes, pAllocMoreLinkBlock, (void **)&pRest->lpRes)));
    if (!ok) {
        OleSetOleError(hr);
        return FALSE;
    }
    for (ULONG i = 0; ok && i < pRest->cRes; i++) {
        // Each object is a restriction structure.
        PyObject *subOb = PySequence_GetItem(ob, i);
        ok = (subOb != NULL);
        ok = ok && PyMAPIObject_AsSingleSRestriction(subOb, pRest->lpRes + i, pAllocMoreLinkBlock);
        Py_XDECREF(subOb);
    }
    return ok;
}

// @object PySOrRestriction|
BOOL PyMAPIObject_AsSOrRestriction(PyObject *ob, SOrRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm [<o PySRestriction>, ...]|restriction||A sequence of <o PySRestriction> objects.
    pRest->cRes = PySequence_Length(ob);
    if (pRest->cRes == (ULONG)-1 && PyErr_Occurred())
        return FALSE;

    HRESULT hr;
    BOOL ok = !FAILED(
        (hr = MAPIAllocateMore(sizeof(SRestriction) * pRest->cRes, pAllocMoreLinkBlock, (void **)&pRest->lpRes)));
    if (!ok) {
        OleSetOleError(hr);
        return FALSE;
    }
    for (ULONG i = 0; ok && i < pRest->cRes; i++) {
        // Each object is a restriction structure.
        PyObject *subOb = PySequence_GetItem(ob, i);
        ok = (subOb != NULL);
        ok = ok && PyMAPIObject_AsSingleSRestriction(subOb, pRest->lpRes + i, pAllocMoreLinkBlock);
        Py_XDECREF(subOb);
    }
    return ok;
}

// @object PySBitMaskRestriction|
BOOL PyMAPIObject_AsSBitMaskRestriction(PyObject *ob, SBitMaskRestriction *pRest, void *pAllocMoreLinkBlock)
{
    // @pyparm int|relBMR||
    // @pyparm ULONG|propTag||The property ID.
    // @pyparm int|ulMask|0|
    pRest->ulMask = 0;
    if (!PyArg_ParseTuple(ob, "lk|l", &pRest->relBMR, &pRest->ulPropTag, &pRest->ulMask))
        return FALSE;
    return TRUE;
}

// @object PySRestriction|
BOOL PyMAPIObject_AsSingleSRestriction(PyObject *ob, SRestriction *pRest, void *pAllocMoreLinkBlock)
{
    if (!PySequence_Check(ob) || PySequence_Length(ob) != 2) {
        PyErr_SetString(PyExc_TypeError, "The SRestriction object must be a sequence of length 2");
        return FALSE;
    }
    PyObject *obResType = PySequence_GetItem(ob, 0);
    if (obResType == NULL)
        return FALSE;
    if (!PyInt_Check(obResType)) {
        PyErr_SetString(PyExc_TypeError, "SRestriction must be a sequence of (integer, object)");
        Py_DECREF(obResType);
        return FALSE;
    }
    // @pyparm int|restrictionType||An integer describing the contents of the second parameter.
    // @pyparm object|restriction||An object in one of the formats describe below.
    pRest->rt = PyInt_AsLong(obResType);
    Py_DECREF(obResType);
    PyObject *subOb = PySequence_GetItem(ob, 1);
    if (subOb == NULL)
        return FALSE;

    // @comm The parameters can be one of the following pairs of values.
    // @flagh restrictionType|restrictionValue
    BOOL ok;
    switch (pRest->rt) {
        case RES_AND:
            // @flag RES_AND|<o PySAndRestriction>
            ok = PyMAPIObject_AsSAndRestriction(subOb, &pRest->res.resAnd, pAllocMoreLinkBlock);
            break;
        case RES_OR:
            // @flag RES_OR|<o PySOrRestriction>
            ok = PyMAPIObject_AsSOrRestriction(subOb, &pRest->res.resOr, pAllocMoreLinkBlock);
            break;
        case RES_PROPERTY:
            // @flag RES_PROPERTY|<o PySPropertyRestriction>
            ok = PyMAPIObject_AsSPropertyRestriction(subOb, &pRest->res.resProperty, pAllocMoreLinkBlock);
            break;
        case RES_EXIST:
            // @flag RES_EXIST|<o PySExistRestriction>
            ok = PyMAPIObject_AsSExistRestriction(subOb, &pRest->res.resExist, pAllocMoreLinkBlock);
            break;
        case RES_NOT:
            // @flag RES_NOT|<o PySNotRestriction>
            ok = PyMAPIObject_AsSNotRestriction(subOb, &pRest->res.resNot, pAllocMoreLinkBlock);
            break;
        case RES_CONTENT:
            // @flag RES_CONTENT|<o PySContentRestriction>
            ok = PyMAPIObject_AsSContentRestriction(subOb, &pRest->res.resContent, pAllocMoreLinkBlock);
            break;
        case RES_BITMASK:
            // @flag RES_BITMASK|<o PySBitMaskRestriction>
            ok = PyMAPIObject_AsSBitMaskRestriction(subOb, &pRest->res.resBitMask, pAllocMoreLinkBlock);
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unsupported restriction type!  Please fix in mapiutil.cpp!!!");
            ok = FALSE;
    }
    Py_DECREF(subOb);
    return ok;
}

BOOL PyMAPIObject_AsSRestriction(PyObject *ob, SRestriction **pRest, BOOL bNoneOK /*= TRUE*/)
{
    HRESULT hr;
    if (ob == Py_None) {
        if (bNoneOK) {
            *pRest = NULL;
            return TRUE;
        }
        else {
            PyErr_SetString(PyExc_ValueError, "None is not a valid SRestriction in this context");
            return FALSE;
        }
    }
    if (S_OK != (hr = MAPIAllocateBuffer(sizeof(SRestriction), (void **)pRest))) {
        OleSetOleError(hr);
        return FALSE;
    }
    BOOL ok = PyMAPIObject_AsSingleSRestriction(ob, *pRest, *pRest);
    if (!ok)
        MAPIFreeBuffer(*pRest);
    return ok;
}

void PyMAPIObject_FreeSRestriction(SRestriction *pRest) { MAPIFreeBuffer(pRest); }

// Sort stuff.

// @object PySSortOrderItem|An item in a SortOrderSet.
BOOL PyMAPIObject_BuildSSortOrderSet(PyObject *obSorts, SSortOrderSet *psos)
{
    // @pyparm int|propTag||A property tag.
    // @pyparm int|order||The order in which the data is to be sorted. Possible values are:
    // mapi.TABLE_SORT_ASCEND, mapi.TABLE_SORT_COMBINE and mapi.TABLE_SORT_DESCEND
    BOOL ok = TRUE;
    for (ULONG i = 0; ok && i < psos->cSorts; i++) {
        SSortOrder *psThis = &psos->aSort[i];
        PyObject *obThis = PySequence_GetItem(obSorts, i);
        if (obThis == NULL)
            return FALSE;
        BOOL ok = PyArg_ParseTuple(obThis, "ll", &psThis->ulPropTag, &psThis->ulOrder);
        Py_DECREF(obThis);
    }
    return ok;
}

// @object PySSortOrderSet|An object describing a SortOrderSet.
// @pyparm ( <o PySSortOrderItem>, ...)|sortItems||The items to sort by
// @pyparm int|cCategories|0|
// @pyparm int|cExpanded|0|
BOOL PyMAPIObject_AsSSortOrderSet(PyObject *obsos, SSortOrderSet **ppsos, BOOL bNoneOK /*= TRUE */)
{
    if (obsos == Py_None) {
        if (bNoneOK) {
            *ppsos = NULL;
            return TRUE;
        }
        else {
            PyErr_SetString(PyExc_ValueError, "None is not a valid SSortOrderSet in this context");
            return FALSE;
        }
    }
    else {
        ULONG cCategories = 0, cExpanded = 0;
        PyObject *obSorts;
        if (!PyArg_ParseTuple(obsos, "O|ll", &obSorts, &cCategories, &cExpanded)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "A SortOrder object must be tuple of (sequence, cCategories=0, cExpanded=0)");
            return FALSE;
        }
        if (!PySequence_Check(obSorts)) {
            PyErr_SetString(PyExc_TypeError, "The first object must be a sequence");
            return FALSE;
        }
        ULONG numSorts = PySequence_Length(obSorts);
        HRESULT hr;
        if (S_OK !=
            (hr = MAPIAllocateBuffer(sizeof(SSortOrderSet) + (sizeof(SSortOrder) * numSorts), (void **)ppsos))) {
            OleSetOleError(hr);
            return FALSE;
        }
        BOOL ok = TRUE;
        (*ppsos)->cSorts = numSorts;
        (*ppsos)->cCategories = cCategories;
        (*ppsos)->cExpanded = cExpanded;
        ok = ok && PyMAPIObject_BuildSSortOrderSet(obSorts, *ppsos);
        if (!ok)
            MAPIFreeBuffer(*ppsos);
        return ok;
    }
}

void PyMAPIObject_FreeSSortOrderSet(SSortOrderSet *ppsos) { MAPIFreeBuffer(ppsos); }

PyObject *PyMAPIObject_FromSPropProblemArray(SPropProblemArray *ppa)
{
    if (!ppa) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *result = PyTuple_New(ppa->cProblem);
    if (!result) {
        PyErr_SetString(PyExc_MemoryError, "Allocating SPropProblemArray result");
        return NULL;
    }

    for (ULONG i = 0; i < ppa->cProblem; i++) {
        PyObject *obNew =
            Py_BuildValue("kki", ppa->aProblem[i].ulIndex, ppa->aProblem[i].ulPropTag, ppa->aProblem[i].scode);
        if (!obNew) {
            Py_DECREF(result);
            return NULL;
        }
        PyTuple_SET_ITEM(result, i, obNew);
    }
    return result;
}

PyObject *PyWinObject_FromMAPIStr(LPTSTR str, BOOL isUnicode)
{
    if (str == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    if (isUnicode) {
        return PyUnicode_FromWideChar((LPCWSTR)str, wcslen((LPCWSTR)str));
    }
    else {
#if PY_MAJOR_VERSION >= 3
        return (PyObject *)PyUnicode_DecodeMBCS((LPSTR)str, strlen((LPSTR)str), NULL);
#else
        return PyString_FromString((LPSTR)str);
#endif
    }
}

BOOL PyWinObject_AsMAPIStr(PyObject *stringObject, LPTSTR *pResult, BOOL asUnicode, BOOL bNoneOK /*= FALSE*/,
                           DWORD *pResultLen /* = NULL */)
{
#if PY_MAJOR_VERSION >= 3
    if (asUnicode)
        return PyWinObject_AsWCHAR(stringObject, (LPWSTR *)pResult, bNoneOK, pResultLen);
    else
        return PyWinObject_AsString(stringObject, (LPSTR *)pResult, bNoneOK, pResultLen);
#else
    if (asUnicode && PyUnicode_Check(stringObject))
        return PyWinObject_AsWCHAR(stringObject, (LPWSTR *)pResult, bNoneOK, pResultLen);
    // allows already encoded string pass-through workaround (backwards compat)
    return PyWinObject_AsString(stringObject, (LPSTR *)pResult, bNoneOK, pResultLen);
#endif
}
