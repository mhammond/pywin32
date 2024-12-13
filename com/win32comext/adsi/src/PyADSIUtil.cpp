// @doc
#include "Python.h"
#include "pyerrors.h"  // for PyErr_Warn in 2.5...
#include "Windows.h"
#include "PyWinTypes.h"
#include "PythonCOM.h"
#include "PyADSIUtil.h"
#include "structmember.h"
#include "wchar.h"

BOOL GetADSIErrorString(HRESULT hr, WCHAR *buf, int nchars);

PyObject *OleSetADSIError(HRESULT hr, IUnknown *pUnk, REFIID iid)
{
    // If the HRESULT is an ADSI one, we do things differently!
    if (hr & 0x00005000) {
        WCHAR szErrorBuf[MAX_PATH] = {0};
        WCHAR szNameBuf[MAX_PATH] = {0};
        DWORD dwErrCode = 0;
        ADsGetLastError(&dwErrCode, szErrorBuf, MAX_PATH - 1, szNameBuf, MAX_PATH - 1);

        if (dwErrCode == 0)
            dwErrCode = hr;

        if (!szErrorBuf[0])
            GetADSIErrorString(hr, szErrorBuf, MAX_PATH - 1);
        // Trim trailing /r/n
        WCHAR *szEnd = szErrorBuf + wcslen(szErrorBuf) - 1;
        while (szEnd > szErrorBuf && (*szEnd == L'\r' || *szEnd == L'\n')) *szEnd-- = L'\0';
        // Only do this if we have a useful message - otherwise
        // let the default handling do the best it can.
        if (szErrorBuf[0]) {
            EXCEPINFO info;
            memset(&info, 0, sizeof(info));
            info.scode = dwErrCode;

            info.bstrSource = SysAllocString(szNameBuf);
            info.bstrDescription = SysAllocString(szErrorBuf);
            // Technically, we probably should return DISP_E_EXCEPTION so we
            // appear to follow COM's rules - however, we really don't
            // _need_ to (as only Python sees this result), and having the native
            // HRESULT is preferable.
            return PyCom_BuildPyExceptionFromEXCEPINFO(dwErrCode, &info);
        }
    }
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
        // Get extended error value.
        WCHAR szErrorBuf[MAX_PATH] = {0};
        WCHAR szNameBuf[MAX_PATH] = {0};
        DWORD dwErrCode = 0;
        ADsGetLastError(&dwErrCode, szErrorBuf, MAX_PATH - 1, szNameBuf, MAX_PATH - 1);
        // Only do this if we have a useful message - otherwise
        // let the default handling do the best it can.
        if (dwErrCode == 0)
            dwErrCode = hr;
        if (szErrorBuf[0]) {
            EXCEPINFO info;
            memset(&info, 0, sizeof(info));
            info.scode = dwErrCode;

            info.bstrSource = SysAllocString(szNameBuf);
            info.bstrDescription = SysAllocString(szErrorBuf);
            // Technically, we probably should return DISP_E_EXCEPTION so we
            // appear to follow COM's rules - however, we really don't
            // _need_ to (as only Python sees this result), and having the native
            // HRESULT is preferable.
            return PyCom_BuildPyExceptionFromEXCEPINFO(dwErrCode, &info);
        }
    }
    // Do the normal thing.
    return PyCom_BuildPyException(hr, pUnk, iid);
}

// @object PyADSVALUE|A tuple:
// @tupleitem 0|object|value|The value as a Python object.
// @tupleitem 1|int|type|The AD type of the value.
PyObject *PyADSIObject_FromADSVALUE(ADSVALUE &v)
{
    PyObject *ob = NULL;
    switch (v.dwType) {
        case ADSTYPE_DN_STRING:
            ob = PyWinObject_FromWCHAR(v.DNString);
            break;
        case ADSTYPE_CASE_EXACT_STRING:
            ob = PyWinObject_FromWCHAR(v.CaseExactString);
            break;
        case ADSTYPE_CASE_IGNORE_STRING:
            ob = PyWinObject_FromWCHAR(v.CaseIgnoreString);
            break;
        case ADSTYPE_PRINTABLE_STRING:
            ob = PyWinObject_FromWCHAR(v.PrintableString);
            break;
        case ADSTYPE_NUMERIC_STRING:
            ob = PyWinObject_FromWCHAR(v.NumericString);
            break;
        case ADSTYPE_BOOLEAN:
            ob = v.Boolean ? Py_True : Py_False;
            Py_INCREF(ob);
            break;
        case ADSTYPE_INTEGER:
            ob = PyLong_FromLong(v.Integer);
            break;
        case ADSTYPE_OCTET_STRING: {
            DWORD bufSize = v.OctetString.dwLength;
            if (!(ob = PyBuffer_New(bufSize)))
                return NULL;
            PyWinBufferView pybuf(ob, true);
            if (!pybuf.ok()) {
                Py_DECREF(ob);
                return NULL;
            }
            memcpy(pybuf.ptr(), v.OctetString.lpValue, bufSize);
        } break;
        case ADSTYPE_UTC_TIME:
            ob = PyWinObject_FromSYSTEMTIME(v.UTCTime);
            break;
        case ADSTYPE_LARGE_INTEGER:
            ob = PyWinObject_FromLARGE_INTEGER(v.LargeInteger);
            break;
        case ADSTYPE_OBJECT_CLASS:
            ob = PyWinObject_FromWCHAR(v.ClassName);
            break;
        case ADSTYPE_PROV_SPECIFIC: {
            DWORD bufSize = v.ProviderSpecific.dwLength;
            if (!(ob = PyBuffer_New(bufSize)))
                return NULL;
            PyWinBufferView pybuf(ob, true);
            if (!pybuf.ok()) {
                Py_DECREF(ob);
                return NULL;
            }
            memcpy(pybuf.ptr(), v.ProviderSpecific.lpValue, bufSize);
            break;
        }
        case ADSTYPE_NT_SECURITY_DESCRIPTOR: {
            // Get a pointer to the security descriptor.
            PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)(v.SecurityDescriptor.lpValue);
            DWORD SDSize = v.SecurityDescriptor.dwLength;
            // eeek - we don't pass the length - pywintypes relies on
            // GetSecurityDescriptorLength - make noise if this may bite us.
            if (SDSize != GetSecurityDescriptorLength(pSD))
                PyErr_Warn(PyExc_RuntimeWarning, "Security-descriptor size mis-match");
            ob = PyWinObject_FromSECURITY_DESCRIPTOR(pSD);
            break;
        }
        default: {
            char msg[100];
            sprintf(msg, "Unknown ADS type code 0x%x - None will be returned", v.dwType);
            PyErr_Warn(PyExc_RuntimeWarning, msg);
            ob = Py_None;
            Py_INCREF(ob);
        }
    }
    if (ob == NULL)
        return NULL;
    PyObject *ret = Py_BuildValue("Oi", ob, (int)v.dwType);
    Py_DECREF(ob);
    return ret;
}

BOOL PyADSIObject_AsTypedValue(PyObject *val, ADSVALUE &v)
{
    BOOL ok = TRUE;
    switch (v.dwType) {
        // OK - get lazy - we know it's a union!
        case ADSTYPE_DN_STRING:
        case ADSTYPE_CASE_EXACT_STRING:
        case ADSTYPE_CASE_IGNORE_STRING:
        case ADSTYPE_PRINTABLE_STRING:
        case ADSTYPE_NUMERIC_STRING:
        case ADSTYPE_OBJECT_CLASS:
            ok = PyWinObject_AsWCHAR(val, &v.DNString, FALSE);
            break;
        case ADSTYPE_BOOLEAN:
            v.Boolean = PyLong_AsLong(val);
            break;
        case ADSTYPE_INTEGER:
            v.Integer = PyLong_AsLong(val);
            break;
        case ADSTYPE_UTC_TIME:
            ok = PyWinObject_AsSYSTEMTIME(val, &v.UTCTime);
            break;
        case ADSTYPE_LARGE_INTEGER:
            ok = PyWinObject_AsLARGE_INTEGER(val, &v.LargeInteger);
            break;
        default:
            PyErr_SetString(PyExc_TypeError, "Can't convert to this type");
            return FALSE;
    }
    return ok;
}

BOOL PyADSIObject_AsADSVALUE(PyObject *ob, ADSVALUE &v)
{
    if (!PyTuple_Check(ob) || PyTuple_Size(ob) < 1 || PyTuple_Size(ob) > 2) {
        PyErr_SetString(PyExc_ValueError,
                        "ADSVALUE must be a tuple of (value, type) (but type may be None or omitted)");
        return FALSE;
    }
    PyObject *val = PyTuple_GET_ITEM(ob, 0);
    PyObject *obtype = NULL;
    DWORD dwType;
    if (PyTuple_Size(ob) > 1)
        obtype = PyTuple_GET_ITEM(ob, 1);
    if (obtype == NULL || obtype == Py_None) {
        if (PyBytes_Check(val) || PyUnicode_Check(val))
            dwType = ADSTYPE_PRINTABLE_STRING;
        else if (val == Py_True || val == Py_False)
            dwType = ADSTYPE_BOOLEAN;
        else if (PyLong_Check(val))
            dwType = ADSTYPE_INTEGER;
        else if (PyWinTime_Check(val))
            dwType = ADSTYPE_UTC_TIME;
        else {
            PyErr_SetString(PyExc_ValueError, "No type given, and can't deduce it!");
            return FALSE;
        }
    }
    else if (PyLong_Check(obtype))
        dwType = PyLong_AsLong(obtype);
    else {
        PyErr_SetString(PyExc_TypeError, "The type specified must be None or a string");
        return FALSE;
    }
    v.dwType = (ADSTYPE)dwType;
    return PyADSIObject_AsTypedValue(val, v);
}

void PyADSIObject_FreeADSVALUE(ADSVALUE &v)
{
    switch (v.dwType) {
        case ADSTYPE_DN_STRING:
        case ADSTYPE_CASE_EXACT_STRING:
        case ADSTYPE_CASE_IGNORE_STRING:
        case ADSTYPE_PRINTABLE_STRING:
        case ADSTYPE_NUMERIC_STRING:
        case ADSTYPE_OBJECT_CLASS:
            PyWinObject_FreeWCHAR(v.DNString);
        default:;
    }
    // force 'null' reset if called again.
    v.dwType = ADSTYPE_INTEGER;
    v.Integer = 0;
}

// Helpers for passing arrays of Unicode around.
BOOL PyADSI_MakeNames(PyObject *obNames, WCHAR ***names, DWORD *pcnames)
{
    if (!PySequence_Check(obNames)) {
        PyErr_SetString(PyExc_TypeError, "names must be a sequence of strings");
        return FALSE;
    }
    *names = NULL;
    int cnames = PySequence_Length(obNames);
    WCHAR **buf = (WCHAR **)malloc(cnames * sizeof(WCHAR *));
    if (buf == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    memset(buf, 0, cnames * sizeof(WCHAR *));
    int i = 0;
    for (i = 0; i < cnames; i++) {
        PyObject *ob = PySequence_GetItem(obNames, i);
        if (ob == NULL)
            goto done;
        BOOL ok = PyWinObject_AsWCHAR(ob, &buf[i], FALSE);
        Py_DECREF(ob);
        if (!ok)
            goto done;
    }
    *names = buf;
    *pcnames = cnames;
done:
    if (*names == NULL) {
        PyADSI_FreeNames(buf, cnames);
    }
    return (*names != NULL);
}

void PyADSI_FreeNames(WCHAR **names, DWORD cnames)
{
    for (int i = 0; i < (int)cnames; i++)
        if (names[i] != NULL)
            PyWinObject_FreeWCHAR(names[i]);
    free(names);
}

// @object PyADS_OBJECT_INFO|Represents a ADS_OBJECT_INFO structure.
class PyADS_OBJECT_INFO : public PyObject {
   public:
    PyADS_OBJECT_INFO(void)
    {
        ob_type = &Type;
        _Py_NewReference(this);
        obRDN = obObjectDN = obParentDN = obClassName = NULL;
    }
    PyADS_OBJECT_INFO(const ADS_OBJECT_INFO *pInfo)
    {
        ob_type = &Type;
        _Py_NewReference(this);
        obRDN = PyWinObject_FromWCHAR(pInfo->pszRDN);
        obObjectDN = PyWinObject_FromWCHAR(pInfo->pszObjectDN);
        obParentDN = PyWinObject_FromWCHAR(pInfo->pszParentDN);
        obClassName = PyWinObject_FromWCHAR(pInfo->pszClassName);
    }
    ~PyADS_OBJECT_INFO()
    {
        Py_XDECREF(obRDN);
        Py_XDECREF(obObjectDN);
        Py_XDECREF(obParentDN);
        Py_XDECREF(obClassName);
    }

    /* Python support */
    static void deallocFunc(PyObject *ob) { delete (PyADS_OBJECT_INFO *)ob; }

    static struct PyMemberDef memberlist[];
    static PyTypeObject Type;

   protected:
    PyObject *obRDN, *obObjectDN, *obParentDN, *obClassName;
};

PyTypeObject PyADS_OBJECT_INFO::Type = {
    PYWIN_OBJECT_HEAD "PyADS_OBJECT_INFO",
    sizeof(PyADS_OBJECT_INFO),
    0,
    PyADS_OBJECT_INFO::deallocFunc, /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,
    0,                       /* tp_call */
    0,                       /* tp_str */
    PyObject_GenericGetAttr, /* tp_getattro */
    PyObject_GenericSetAttr, /* tp_setattro */
};

#define OFF(e) offsetof(PyADS_OBJECT_INFO, e)

/*static*/ struct PyMemberDef PyADS_OBJECT_INFO::memberlist[] = {
    {"RDN", T_OBJECT, OFF(obRDN)},              // @prop unicode|RDN|The name
    {"ObjectDN", T_OBJECT, OFF(obObjectDN)},    // @prop unicode|ObjectDN|
    {"ParentDN", T_OBJECT, OFF(obParentDN)},    // @prop unicode|ParentDN|
    {"ClassName", T_OBJECT, OFF(obClassName)},  // @prop unicode|ClassName|
    {NULL}};

PyObject *PyADSIObject_FromADS_OBJECT_INFO(ADS_OBJECT_INFO *info) { return new PyADS_OBJECT_INFO(info); }

// @object PyADS_ATTR_INFO|Represents a ADS_ATTR_INFO structure.
class PyADS_ATTR_INFO : public PyObject {
   public:
    PyADS_ATTR_INFO(void)
    {
        ob_type = &Type;
        _Py_NewReference(this);
        dwControlCode = 0;
        dwADsType = ADSTYPE_INVALID;
        obValues = PyList_New(0);
        obName = Py_None;
        Py_INCREF(obName);
        bufName = NULL;
        bufValues = NULL;
    }
    PyADS_ATTR_INFO(const ADS_ATTR_INFO *pInfo)
    {
        ob_type = &Type;
        _Py_NewReference(this);
        bufName = NULL;
        bufValues = NULL;
        obName = PyWinObject_FromWCHAR(pInfo->pszAttrName);
        dwControlCode = pInfo->dwControlCode;
        dwADsType = pInfo->dwADsType;
        obValues = PyList_New(pInfo->dwNumValues);
        if (obValues) {
            for (DWORD i = 0; i < pInfo->dwNumValues; i++) {
                PyList_SET_ITEM(obValues, i, PyADSIObject_FromADSVALUE(pInfo->pADsValues[i]));
            }
        }
    }
    ~PyADS_ATTR_INFO()
    {
        InvalidateName();
        InvalidateValues();
        Py_XDECREF(obName);
        Py_XDECREF(obValues);
    }
    void InvalidateName(void)
    {
        if (bufName) {
            PyWinObject_FreeWCHAR(bufName);
            bufName = NULL;
        }
    }
    void InvalidateValues(void)
    {
        if (bufValues) {
            for (int i = 0; i < bufValuesNum; i++) {
                PyADSIObject_FreeADSVALUE(bufValues[i]);
            }
            free(bufValues);
            bufValues = NULL;
        }
    }
    BOOL FillADS_ATTR_INFO(ADS_ATTR_INFO *pInfo)
    {
        pInfo->dwControlCode = dwControlCode;
        pInfo->dwADsType = dwADsType;
        if (bufName == NULL) {
            if (!PyWinObject_AsWCHAR(obName, &bufName, FALSE))
                return FALSE;
        }
        pInfo->pszAttrName = bufName;
        if (bufValues == NULL) {
            if (!PySequence_Check(obValues)) {
                PyErr_SetString(PyExc_TypeError, "Values attribute must be a sequence!");
                return FALSE;
            }
            int n = bufValuesNum = PySequence_Length(obValues);
            bufValues = (ADSVALUE *)malloc(n * sizeof(ADSVALUE));
            memset(bufValues, 0, n * sizeof(ADSVALUE));
            if (bufValues == NULL) {
                PyErr_NoMemory();
                return FALSE;
            }

            for (int i = 0; i < n; i++) {
                PyObject *ob = PySequence_GetItem(obValues, i);
                if (ob == NULL)
                    return FALSE;
                BOOL ok = PyADSIObject_AsADSVALUE(ob, bufValues[i]);
                Py_DECREF(ob);
                if (!ok)
                    return FALSE;
            }
        }
        pInfo->pADsValues = bufValues;
        pInfo->dwNumValues = bufValuesNum;
        return TRUE;
    }

    /* Python support */
    static void deallocFunc(PyObject *ob) { delete (PyADS_ATTR_INFO *)ob; }

    static PyObject *getattro(PyObject *self, PyObject *obname)
    {
        char *name = PYWIN_ATTR_CONVERT(obname);
        if (name == NULL)
            return NULL;
        if (strcmp(name, "__members__") == 0) {
            PyObject *ret = PyList_New(4);
            if (ret) {
                PyList_SET_ITEM(ret, 0, PyBytes_FromString("AttrName"));
                PyList_SET_ITEM(ret, 1, PyBytes_FromString("ControlCode"));
                PyList_SET_ITEM(ret, 2, PyBytes_FromString("ADsType"));
                PyList_SET_ITEM(ret, 3, PyBytes_FromString("Values"));
            }
            return ret;
        }
        if (strcmp(name, "Values") == 0) {
            PyObject *ret = ((PyADS_ATTR_INFO *)self)->obValues ? ((PyADS_ATTR_INFO *)self)->obValues : Py_None;
            Py_INCREF(ret);
            return ret;
        }
        return PyObject_GenericGetAttr(self, obname);
    }

    // #pragma warning( disable : 4251 )
    static struct PyMemberDef memberlist[];
    // #pragma warning( default : 4251 )
    static PyTypeObject Type;

   protected:
    DWORD dwControlCode;
    ADSTYPE dwADsType;
    PyObject *obName;
    PyObject *obValues;
    WCHAR *bufName;
    ADSVALUE *bufValues;
    int bufValuesNum;
};

PyTypeObject PyADS_ATTR_INFO::Type = {
    PYWIN_OBJECT_HEAD "PyADS_ATTR_INFO",
    sizeof(PyADS_ATTR_INFO),
    0,
    PyADS_ATTR_INFO::deallocFunc, /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,
    0,                         /* tp_call */
    0,                         /* tp_str */
    PyADS_ATTR_INFO::getattro, /* tp_getattro */
    PyObject_GenericSetAttr,   /* tp_setattro */
};

#undef OFF
#define OFF(e) offsetof(PyADS_ATTR_INFO, e)

/*static*/ struct PyMemberDef PyADS_ATTR_INFO::memberlist[] = {
    {"AttrName", T_OBJECT, OFF(obName)},         // @prop unicode|AttrName|The name
    {"ControlCode", T_INT, OFF(dwControlCode)},  // @prop integer|ControlCode|
    {"ADsType", T_INT, OFF(dwADsType)},          // @prop integer|ADsType|
    {NULL}};
// @prop [<o PyADSVALUE>, ...]|Values|

PyObject *PyADSIObject_FromADS_ATTR_INFOs(ADS_ATTR_INFO *infos, DWORD cinfos)
{
    PyObject *ret = PyTuple_New(cinfos);
    for (DWORD i = 0; ret != NULL && i < cinfos; i++) {
        PyObject *n = new PyADS_ATTR_INFO(infos + i);
        if (n == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, (int)i, n);
    }
    return ret;
}

BOOL _Make_ATTR_INFO(PyObject *ob, ADS_ATTR_INFO *pBase, DWORD index)
{
    PyObject *obName, *obValues;
    ADS_ATTR_INFO *pThis = pBase + index;
    PyObject *sub = PySequence_GetItem(ob, index);
    if (!sub)
        return FALSE;

    if (!PyArg_ParseTuple(sub, "OllO:ADS_ATTR_INFO tuple", &obName, &pThis->dwControlCode, &pThis->dwADsType,
                          &obValues))
        return FALSE;
    if (!PyWinObject_AsWCHAR(obName, &pThis->pszAttrName, FALSE))
        return FALSE;
    if (!PySequence_Check(obValues)) {
        PyErr_Format(PyExc_TypeError, "4th item in an ATTR_INFO structure must be a sequence (got %s)",
                     obValues->ob_type->tp_name);
        return FALSE;
    }
    DWORD nValues = PySequence_Length(obValues);
    pThis->pADsValues = (PADSVALUE)malloc(nValues * sizeof(ADSVALUE));
    if (!pThis->pADsValues) {
        PyErr_NoMemory();
        return FALSE;
    }
    memset(pThis->pADsValues, 0, nValues * sizeof(ADSVALUE));
    pThis->dwNumValues = nValues;
    DWORD i;
    BOOL ok;
    for (i = 0; i < nValues; i++) {
        PyObject *val = PySequence_GetItem(obValues, i);
        if (!val)
            return FALSE;
        pThis->pADsValues[i].dwType = pThis->dwADsType;
        ok = PyADSIObject_AsTypedValue(val, pThis->pADsValues[i]);
        Py_DECREF(val);
    }
    return TRUE;
}

void PyADSIObject_FreeADS_ATTR_INFOs(ADS_ATTR_INFO *pval, DWORD cinfos)
{
    if (!pval)
        return;
    DWORD i;
    for (i = 0; i < cinfos; i++) {
        ADS_ATTR_INFO *pThis = pval + i;
        PyWinObject_FreeWCHAR(pThis->pszAttrName);
        if (pThis->pADsValues) {
            DWORD valnum;
            for (valnum = 0; valnum < pThis->dwNumValues; valnum++)
                PyADSIObject_FreeADSVALUE(pThis->pADsValues[valnum]);
            free(pThis->pADsValues);
        }
    }
    free(pval);
}
BOOL PyADSIObject_AsADS_ATTR_INFOs(PyObject *ob, ADS_ATTR_INFO **ppret, DWORD *pcinfos)
{
    if (!PySequence_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "ADS_ATTR_INFOs must be a sequence");
        return FALSE;
    }
    DWORD i;
    // Use C++ reference to make working with ppret more convenient.
    ADS_ATTR_INFO *&pret = *ppret;
    DWORD &nitems = *pcinfos;

    nitems = PySequence_Length(ob);
    pret = (PADS_ATTR_INFO)malloc(nitems * sizeof(ADS_ATTR_INFO));
    if (!pret) {
        PyErr_NoMemory();
        return FALSE;
    }
    memset(pret, 0, nitems * sizeof(ADS_ATTR_INFO));
    BOOL ok = TRUE;
    for (i = 0; ok && i < nitems; i++) {
        ok = _Make_ATTR_INFO(ob, pret, i);
    }
    if (!ok && pret) {
        PyADSIObject_FreeADS_ATTR_INFOs(pret, nitems);
        pret = 0;
        nitems = 0;
    }
    return ok;
}

// @object PyADS_SEARCHPREF_INFO|A tuple of:
// @tupleitem 0|int|attr_id|
// @tupleitem 1|<o PyADSVALUE>|value|
void PyADSIObject_FreeADS_SEARCHPREF_INFOs(ADS_SEARCHPREF_INFO *pattr, DWORD cattr)
{
    if (!pattr)
        return;
    DWORD i;
    for (i = 0; i < cattr; i++) PyADSIObject_FreeADSVALUE(pattr[i].vValue);
    free(pattr);
}

BOOL PyADSIObject_AsADS_SEARCHPREF_INFOs(PyObject *ob, ADS_SEARCHPREF_INFO **ppret, DWORD *pcinfos)
{
    BOOL ret = FALSE;
    if (!PySequence_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "ADS_SEARCHPREF_INFOs must be a sequence");
        return FALSE;
    }
    // Use C++ reference to make working with ppret more convenient.
    ADS_SEARCHPREF_INFO *&pret = *ppret;
    DWORD &nitems = *pcinfos;
    nitems = PySequence_Length(ob);

    pret = (ADS_SEARCHPREF_INFO *)malloc(sizeof(ADS_SEARCHPREF_INFO) * nitems);
    if (!pret) {
        PyErr_NoMemory();
        return NULL;
    }
    memset(pret, 0, sizeof(ADS_SEARCHPREF_INFO) * nitems);
    PyObject *sub = NULL;
    PyObject *obValue;  // no reference
    DWORD i;
    for (i = 0; i < nitems; i++) {
        PyObject *sub = PySequence_GetItem(ob, i);
        if (!sub)
            goto done;
        if (!PyArg_ParseTuple(sub, "iO:ADS_SEARCHPREF_INFO tuple", &pret[i].dwSearchPref, &obValue))
            goto done;
        if (!PyADSIObject_AsADSVALUE(obValue, pret[i].vValue))
            goto done;
        Py_DECREF(sub);
        sub = NULL;
    }
    ret = TRUE;
done:
    Py_XDECREF(sub);
    if (!ret && pret)
        PyADSIObject_FreeADS_SEARCHPREF_INFOs(pret, nitems);
    return ret;
}

///////////////////////////////////////////////////////
//
// Error string utility.
//
// AdsErr.h is built from a message file.
// Therefore, there _must_ be a DLL around we can call
// FormatMessage with.
// However, it's not obvious, and this code was cut directly from MSDN.
#include "AdsErr.h"
typedef struct tagADSERRMSG {
    HRESULT hr;
    LPCWSTR pszError;
} ADSERRMSG;

#define ADDADSERROR(x) x, L## #x

const ADSERRMSG adsErr[] = {
    ADDADSERROR(E_ADS_BAD_PATHNAME),
    ADDADSERROR(E_ADS_INVALID_DOMAIN_OBJECT),
    ADDADSERROR(E_ADS_INVALID_USER_OBJECT),
    ADDADSERROR(E_ADS_INVALID_COMPUTER_OBJECT),
    ADDADSERROR(E_ADS_UNKNOWN_OBJECT),
    ADDADSERROR(E_ADS_PROPERTY_NOT_SET),
    ADDADSERROR(E_ADS_PROPERTY_NOT_SUPPORTED),
    ADDADSERROR(E_ADS_PROPERTY_INVALID),
    ADDADSERROR(E_ADS_BAD_PARAMETER),
    ADDADSERROR(E_ADS_OBJECT_UNBOUND),
    ADDADSERROR(E_ADS_PROPERTY_NOT_MODIFIED),
    ADDADSERROR(E_ADS_PROPERTY_MODIFIED),
    ADDADSERROR(E_ADS_CANT_CONVERT_DATATYPE),
    ADDADSERROR(E_ADS_PROPERTY_NOT_FOUND),
    ADDADSERROR(E_ADS_OBJECT_EXISTS),
    ADDADSERROR(E_ADS_SCHEMA_VIOLATION),
    ADDADSERROR(E_ADS_COLUMN_NOT_SET),
    ADDADSERROR(E_ADS_INVALID_FILTER),
    ADDADSERROR(0),
};

/////////////////////////////////////////////
//
// Error message specific to ADSI
//
////////////////////////////////////////////
BOOL GetADSIErrorString(HRESULT hr, WCHAR *buf, int nchars)
{
    if (hr & 0x00005000) {
        int idx = 0;
        while (adsErr[idx].hr != 0) {
            if (adsErr[idx].hr == hr) {
                wcsncpy(buf, adsErr[idx].pszError, nchars);
                return TRUE;
            }
            idx++;
        }
    }
    buf[0] = L'\0';
    return FALSE;
}
