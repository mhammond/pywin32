// PyUnicode.cpp -- This used to be a Unicode string type for Python, before
// Python had unicode. Then we had py3k. Then we have covid. I don't know how
// any of this still manages to work more :)

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "malloc.h"
#include "tchar.h"
#include "locale.h"

BOOL PyWinObject_AsPfnAllocatedWCHAR(PyObject *stringObject, void *(*pfnAllocator)(Py_ssize_t), WCHAR **ppResult,
                                     BOOL bNoneOK /*= FALSE*/)
{
    BOOL rc = TRUE;
    if (PyBytes_Check(stringObject)) {
        // XXX - this was ported from the python 2 string api - which I thought
        // included the trailing \0. But the 3.x `Bytes` API does not (right?),
        // so there's some trailing \0 confusion here.
        Py_ssize_t cch = PyBytes_Size(stringObject);
        const char *buf = PyBytes_AsString(stringObject);
        if (buf == NULL)
            return FALSE;

        /* We assume that we dont need more 'wide characters' for the result
           then the number of bytes in the input. Often we
           will need less, as the input may contain multi-byte chars, but we
           should never need more
        */
        PYWIN_CHECK_SSIZE_DWORD(cch+1, FALSE);
        *ppResult = (LPWSTR)(*pfnAllocator)((cch + 1) * sizeof(WCHAR));
        if (*ppResult)
            /* convert and get the final character size */
            MultiByteToWideChar(CP_ACP, 0, buf, (DWORD)cch + 1, *ppResult, (DWORD)cch + 1);
    }
    else if (PyUnicode_Check(stringObject)) {
        // copy the value, including embedded NULLs
        TmpWCHAR v = stringObject;  if (!v) return FALSE;
        Py_ssize_t cch = v.length;
        *ppResult = (WCHAR *)pfnAllocator((cch + 1) * sizeof(WCHAR));
        if (*ppResult)
            memcpy(*ppResult, v, (cch + 1) * sizeof(WCHAR));
    }
    else if (stringObject == Py_None) {
        if (bNoneOK) {
            *ppResult = NULL;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
            rc = FALSE;
        }
    }
    else {
        const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
        PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
        rc = FALSE;
    }
    if (rc && !ppResult) {
        PyErr_SetString(PyExc_MemoryError, "Allocating WCHAR");
        return FALSE;
    }
    return rc;
}

// Get around calling conversion issues.
void *AllocViaCoTaskMemAlloc(Py_ssize_t cb) { return CoTaskMemAlloc(cb); }

BOOL PyWinObject_AsTaskAllocatedWCHAR(PyObject *stringObject, WCHAR **ppResult, BOOL bNoneOK /*= FALSE*/)
{
    return PyWinObject_AsPfnAllocatedWCHAR(stringObject, AllocViaCoTaskMemAlloc, ppResult, bNoneOK);
}

void PyWinObject_FreeTaskAllocatedWCHAR(WCHAR *str) { CoTaskMemFree(str); }

PyObject *PyWinCoreString_FromString(const char *str, Py_ssize_t len /*=(Py_ssize_t)-1*/)
{
    if (len == (Py_ssize_t)-1)
        len = strlen(str);
    return PyUnicode_DecodeMBCS(str, len, "ignore");
}

PyObject *PyWinCoreString_FromString(const WCHAR *str, Py_ssize_t len /*=(Py_ssize_t)-1*/)
{
    if (len == (Py_ssize_t)-1)
        len = wcslen(str);
    return PyUnicode_FromWideChar(str, len);
}

BOOL PyWinObject_AsChars(PyObject *stringObject, char **pResult, BOOL bNoneOK /*= FALSE*/,
                         DWORD *pResultLen /* = NULL */)
{
    PyObject *tempObject = NULL;
    if (stringObject == Py_None) {
        if (!bNoneOK) {
            PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
            return FALSE;
        }
        *pResult = NULL;
        if (pResultLen)
            *pResultLen = 0;
        return TRUE;
    }
    // Convert the string if a WIDE string.
    if (PyUnicode_Check(stringObject)) {
        // PyUnicode_EncodeMBCS was removed in Py 3.11.
        stringObject = tempObject = PyUnicode_AsMBCSString(stringObject);
        if (!stringObject)
            return FALSE;
    }
    if (!PyBytes_Check(stringObject)) {
        PyErr_Format(PyExc_TypeError, "Expected 'bytes', got '%s'", stringObject->ob_type->tp_name);
        return FALSE;
    }
    char *temp = PyBytes_AsString(stringObject);
    Py_ssize_t len = PyBytes_Size(stringObject);
    PYWIN_CHECK_SSIZE_DWORD(len, FALSE);
    *pResult = (char *)PyMem_Malloc(len + 1);
    if (*pResult) {
        memcpy(*pResult, temp, len + 1);
        if (pResultLen)
            *pResultLen = (DWORD)len;
    }
    Py_XDECREF(tempObject);
    return (*pResult != NULL);
}

void PyWinObject_FreeChars(char *str) { PyMem_Free(str); }

// Size info is available (eg, a fn returns a string and also fills in a size variable)
PyObject *PyWinObject_FromOLECHAR(const OLECHAR *str, Py_ssize_t numChars)
{
    if (str == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyUnicode_FromWideChar((OLECHAR *)str, numChars);
}

// No size info avail.
PyObject *PyWinObject_FromOLECHAR(const OLECHAR *str)
{
    if (str == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyUnicode_FromWideChar((OLECHAR *)str, wcslen(str));
}

PyObject *PyWinObject_FromBstr(const BSTR bstr, BOOL takeOwnership /*=FALSE*/)
{
    if (bstr == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = PyUnicode_FromWideChar(bstr, SysStringLen(bstr));
    if (takeOwnership)
        SysFreeString(bstr);
    return ret;
}

///////////////////////////////////////////////////////////
//
// Some utilities etc

PyWin_AutoFreeBstr::PyWin_AutoFreeBstr(BSTR bstr /*= NULL*/) : m_bstr(bstr) { return; }

PyWin_AutoFreeBstr::~PyWin_AutoFreeBstr() { SysFreeString(m_bstr); }

void PyWin_AutoFreeBstr::SetBstr(BSTR bstr)
{
    SysFreeString(m_bstr);
    m_bstr = bstr;
}

// String conversions
BOOL PyWinObject_AsBstr(PyObject *stringObject, BSTR *pResult, BOOL bNoneOK /*= FALSE*/, DWORD *pResultLen /*= NULL*/)
{
    BOOL rc = TRUE;
    // This used to support bytes as we moved to 3.x, but a BSTR has always been
    // unicode (ie, you'd never *try* and use bytes to create it), so there's no
    // sane b/w compat reason to support that any more.
    if (PyUnicode_Check(stringObject)) {
        // copy the value, including embedded NULLs
        // Py3.12+: only conversion yields the correct number of wide chars (incl. surrogate pairs).
        // For simplicity we use a temp buffer.
        TmpWCHAR tw = stringObject;  if (!tw) return FALSE;
        PYWIN_CHECK_SSIZE_DWORD(tw.length, NULL);
        // SysAllocStringLen allocates length+1 wchars (and puts a \0 at end); like PyUnicode_AsWideCharString
        *pResult = SysAllocStringLen(tw, (UINT)tw.length);
    }
    else if (stringObject == Py_None) {
        if (bNoneOK) {
            *pResult = NULL;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
            rc = FALSE;
        }
    }
    else {
        const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
        PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
        rc = FALSE;
    }
    if (rc && !pResult) {
        PyErr_SetString(PyExc_MemoryError, "Allocating BSTR");
        return FALSE;
    }
    if (rc && pResultLen)
        *pResultLen = SysStringLen(*pResult);
    return rc;
}

void PyWinObject_FreeBstr(BSTR str) { SysFreeString(str); }

// String conversions
// Convert a Python object to a WCHAR - allow embedded NULLs, None, etc.
// Must be freed with PyWinObject_FreeWCHAR / PyMem_Free
BOOL PyWinObject_AsWCHAR(PyObject *stringObject, WCHAR **pResult, BOOL bNoneOK /*= FALSE*/,
                         DWORD *pResultLen /*= NULL*/)
{
    BOOL rc = TRUE;
    Py_ssize_t resultLen = 0;
    // Do NOT accept 'bytes' for any 'WCHAR' API.
    if (PyUnicode_Check(stringObject)) {
        *pResult = PyUnicode_AsWideCharString(stringObject, &resultLen);
        if (*pResult == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Getting WCHAR string");
            return FALSE;
        }
    }
    else if (stringObject == Py_None) {
        if (bNoneOK) {
            *pResult = NULL;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
            rc = FALSE;
        }
    }
    else {
        const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
        PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
        rc = FALSE;
    }
    if (rc && pResultLen) {
        if (!PyWin_is_ssize_dword(resultLen)) {
            PyErr_SetString(PyExc_ValueError, "value is larger than a DWORD");
            rc = FALSE;
        }
        *pResultLen = (DWORD)resultLen;
    }
    return rc;
}

void PyWinObject_FreeWCHAR(WCHAR *str) { PyMem_Free(str); }

// Converts a series of consecutive null terminated strings into a list
// Note that a read overflow can result if the input is not properly terminated with an extra NULL.
// Should probably also add a counted version, as win32api uses for REG_MULTI_SZ
PyObject *PyWinObject_FromMultipleString(WCHAR *multistring)
{
    PyObject *obelement, *ret = NULL;
    size_t elementlen;
    if (multistring == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    elementlen = wcslen(multistring);
    while (elementlen) {
        obelement = PyWinObject_FromWCHAR(multistring, elementlen);
        if ((obelement == NULL) || (PyList_Append(ret, obelement) == -1)) {
            Py_XDECREF(obelement);
            Py_DECREF(ret);
            return NULL;
        }
        Py_DECREF(obelement);
        multistring += elementlen + 1;
        elementlen = wcslen(multistring);
    }
    return ret;
}

PyObject *PyWinObject_FromMultipleString(char *multistring)
{
    PyObject *obelement, *ret = NULL;
    size_t elementlen;
    if (multistring == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    elementlen = strlen(multistring);
    while (elementlen) {
        obelement = PyBytes_FromStringAndSize(multistring, elementlen);
        if ((obelement == NULL) || (PyList_Append(ret, obelement) == -1)) {
            Py_XDECREF(obelement);
            Py_DECREF(ret);
            return NULL;
        }
        Py_DECREF(obelement);
        multistring += elementlen + 1;
        elementlen = strlen(multistring);
    }
    return ret;
}

// Converts a sequence of str/unicode objects into a series of consecutive null-terminated
//	char strings with extra terminating null
BOOL PyWinObject_AsMultipleString(PyObject *ob, WCHAR **pmultistring, BOOL bNoneOK, DWORD *chars_returned)
{
    DWORD numStrings, i;
    WCHAR **wchars;
    BOOL rc = FALSE;

    *pmultistring = NULL;
    if (chars_returned)
        *chars_returned = 0;
    if (!PyWinObject_AsWCHARArray(ob, &wchars, &numStrings, bNoneOK))
        return FALSE;
    // Shortcut for None
    if (wchars == NULL)
        return TRUE;

    size_t len = numStrings + 1;  // One null for each string plus extra terminating null
    // Need to loop twice - once to get the buffer length
    for (i = 0; i < numStrings; i++) len += wcslen(wchars[i]);

    // Allocate the buffer
    *pmultistring = (WCHAR *)malloc(len * sizeof(WCHAR));
    if (*pmultistring == NULL)
        PyErr_NoMemory();
    else {
        WCHAR *p = *pmultistring;
        for (i = 0; i < numStrings; i++) {
            wcscpy(p, wchars[i]);
            p += wcslen(wchars[i]);
            *p++ = L'\0';
        }
        *p = L'\0';  // Add second terminator.
        rc = TRUE;
        if (chars_returned)
            *chars_returned = (DWORD)len;
    }
    PyWinObject_FreeWCHARArray(wchars, numStrings);
    return rc;
}

void PyWinObject_FreeMultipleString(WCHAR *pmultistring)
{
    if (pmultistring)
        free(pmultistring);
}

void PyWinObject_FreeMultipleString(char *pmultistring)
{
    if (pmultistring)
        free(pmultistring);
}

// Converts a aequence of string or unicode objects into an array of WCHAR
void PyWinObject_FreeWCHARArray(LPWSTR *wchars, DWORD str_cnt)
{
    if (wchars != NULL) {
        for (DWORD wchar_index = 0; wchar_index < str_cnt; wchar_index++) PyWinObject_FreeWCHAR(wchars[wchar_index]);
        free(wchars);
    }
}

BOOL PyWinObject_AsWCHARArray(PyObject *str_seq, LPWSTR **wchars, DWORD *str_cnt, BOOL bNoneOK)
{
    BOOL ret = FALSE;
    PyObject *str_tuple = NULL, *tuple_item;
    DWORD bufsize, tuple_index;
    *wchars = NULL;
    *str_cnt = 0;

    if (bNoneOK && str_seq == Py_None)
        return TRUE;
    if ((str_tuple = PyWinSequence_Tuple(str_seq, str_cnt)) == NULL)
        return FALSE;
    bufsize = *str_cnt * sizeof(LPWSTR);
    *wchars = (LPWSTR *)malloc(bufsize);
    if (*wchars == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
        goto done;
    }
    ZeroMemory(*wchars, bufsize);
    for (tuple_index = 0; tuple_index < *str_cnt; tuple_index++) {
        tuple_item = PyTuple_GET_ITEM(str_tuple, tuple_index);
        if (!PyWinObject_AsWCHAR(tuple_item, &((*wchars)[tuple_index]), FALSE)) {
            PyWinObject_FreeWCHARArray(*wchars, *str_cnt);
            *wchars = NULL;
            *str_cnt = 0;
            goto done;
        }
    }
    ret = TRUE;
done:
    Py_DECREF(str_tuple);
    return ret;
}

// Copy s null terminated string so that it can be deallocated with PyWinObject_FreeChars
WCHAR *PyWin_CopyString(const WCHAR *input)
{
    size_t len = wcslen(input);
    WCHAR *output = (WCHAR *)PyMem_Malloc((len + 1) * sizeof(WCHAR));
    if (output == NULL)
        return NULL;
    return wcsncpy(output, input, len + 1);
}

char *PyWin_CopyString(const char *input)
{
    size_t len = strlen(input);
    char *output = (char *)PyMem_Malloc((len + 1) * sizeof(char));
    if (output == NULL)
        return NULL;
    return strncpy(output, input, len + 1);
}
