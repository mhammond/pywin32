#include "Windows.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "Rapi.h"
#include "math.h"

// Identical to PyW32_BEGIN_ALLOW_THREADS except no script "{" !!!
// means variables can be declared between the blocks
#define PyW32_BEGIN_ALLOW_THREADS PyThreadState *_save = PyEval_SaveThread();
#define PyW32_END_ALLOW_THREADS PyEval_RestoreThread(_save);
#define PyW32_BLOCK_THREADS Py_BLOCK_THREADS

class PyCEHKEY : public PyHANDLE {
   public:
    PyCEHKEY(HKEY hInit) : PyHANDLE((HANDLE)hInit) {}
    virtual BOOL Close(void);
    virtual const char *GetTypeName() { return "PyCEHKEY"; }
};

// @object PyCEHKEY|A Python object, representing a remote Windows CE Registry handle
BOOL PyWinObject_AsCEHKEY(PyObject *ob, HKEY *pRes, BOOL bNoneOK = FALSE)
{
    return PyWinObject_AsHANDLE(ob, (HANDLE *)pRes, bNoneOK);
}
PyObject *PyWinObject_FromCEHKEY(HKEY h) { return new PyCEHKEY(h); }
// @pymethod <o PyCEHKEY>|wincerapi|CEHKEY|Creates a new CEHKEY object
PyObject *PyWinMethod_NewCEHKEY(PyObject *self, PyObject *args)
{
    long hInit;
    if (!PyArg_ParseTuple(args, "|l:CEHKEY", &hInit))
        return NULL;
    return new PyCEHKEY((HKEY)hInit);
}

BOOL PyWinObject_CloseCEHKEY(PyObject *obHandle)
{
    BOOL ok;
    if (PyHANDLE_Check(obHandle))
        // Python error already set.
        ok = ((PyCEHKEY *)obHandle)->Close();
    else if
        PyInt_Check(obHandle)
        {
            PyW32_BEGIN_ALLOW_THREADS long rc = ::CeRegCloseKey((HKEY)PyInt_AsLong(obHandle));
            PyW32_END_ALLOW_THREADS ok = (rc == ERROR_SUCCESS);
            if (!ok)
                PyWin_SetAPIError("CeRegCloseKey", rc);
        }
    else {
        PyErr_SetString(PyExc_TypeError, "A handle must be a CEHKEY object or an integer");
        return FALSE;
    }
    return ok;
}

// The non-static member functions
BOOL PyCEHKEY::Close(void)
{
    long rc = m_handle ? CeRegCloseKey((HKEY)m_handle) : 0;
    m_handle = 0;
    if (rc != 0)
        PyWin_SetAPIError("CeRegCloseKey", rc);
    return rc == 0;
}

//////////////////////////////////////////////////////////////////
//
// The methods.

// @pymethod (<o PyHKEY>, int)|wincerapi|CeRegCreateKeyEx|Creates the specified key, or opens the key if it already
// exists.
PyObject *PyCeRegCreateKeyEx(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obSubKey;
    HKEY retKey;
    long rc;
    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|subKey||The name of a key that this method opens or creates.
    // This key must be a subkey of the key identified by the key parameter.
    // If key is one of the predefined keys, subKey may be None. In that case,
    // the handle returned is the same hkey handle passed in to the function.
    if (!PyArg_ParseTuple(args, "OO:CeRegCreateKeyEx", &obKey, &obSubKey))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *subKey;
    if (!PyWinObject_AsTCHAR(obSubKey, &subKey, 1))
        return NULL;
    // @pyseeapi CeRegCreateKey
    DWORD disposition;
    rc = CeRegCreateKeyEx(hKey, subKey, 0, NULL, 0, 0, NULL, &retKey, &disposition);
    PyWinObject_FreeTCHAR(subKey);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegCreateKey", rc);
    PyObject *h = PyWinObject_FromCEHKEY(retKey);
    PyObject *prc = Py_BuildValue("Oi", h, disposition);
    Py_DECREF(h);
    return prc;
    // @rdesc The return value is the handle of the opened key, and a flag
    // indicating if the key was opened or created.
    // If the function fails, an exception is raised.
}
// @pymethod |wincerapi|CeRegDeleteKey|Deletes the specified key.  This method can not delete keys with subkeys.
PyObject *PyCeRegDeleteKey(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obSubKey;
    long rc;
    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|subKey||The name of the key to delete.
    // This key must be a subkey of the key identified by the key parameter.
    // This value must not be None, and the key may not have subkeys.
    if (!PyArg_ParseTuple(args, "OO:CeRegDeleteKey", &obKey, &obSubKey))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *subKey;
    if (!PyWinObject_AsTCHAR(obSubKey, &subKey, 1))
        return NULL;
    // @pyseeapi CeRegDeleteKey
    rc = CeRegDeleteKey(hKey, subKey);
    PyWinObject_FreeTCHAR(subKey);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegDeleteKey", rc);
    Py_INCREF(Py_None);
    return Py_None;
    // @comm If the method succeeds, the entire key, including all of its values, is removed.
    // If the method fails, and exception is raised.
}
// @pymethod |wincerapi|CeRegDeleteValue|Removes a named value from the specified registry key.
PyObject *PyCeRegDeleteValue(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obSubKey;
    long rc;
    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|value||The name of the value to remove.
    if (!PyArg_ParseTuple(args, "OO:CeRegDeleteValue", &obKey, &obSubKey))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *subKey;
    if (!PyWinObject_AsTCHAR(obSubKey, &subKey, TRUE))
        return NULL;
    // @pyseeapi CeRegDeleteValue
    PyW32_BEGIN_ALLOW_THREADS rc = CeRegDeleteValue(hKey, subKey);
    PyW32_END_ALLOW_THREADS PyWinObject_FreeTCHAR(subKey);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegDeleteValue", rc);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod (string, string)|wincerapi|CeRegEnumKeyEx|Enumerates subkeys of the specified open registry key. The
// function retrieves the name of one subkey each time it is called.
PyObject *PyCeRegEnumKeyEx(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    int index;
    long rc;
    TCHAR *retBuf;
    DWORD len;

    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm int|index||The index of the key to retrieve.
    if (!PyArg_ParseTuple(args, "Oi:CeRegEnumKey", &obKey, &index))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;

    if ((rc = CeRegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, &len, NULL, NULL, NULL, NULL, NULL, NULL)) !=
        ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegQueryInfoKey", rc);
    ++len;  // include null terminator
    retBuf = (TCHAR *)malloc(len * sizeof(TCHAR));

    // @pyseeapi CeRegEnumKey
    PyObject *prc = NULL;
    rc = CeRegEnumKeyEx(hKey, index, retBuf, &len, NULL, NULL, NULL, NULL);
    if (rc == ERROR_SUCCESS) {
        PyObject *r1 = PyWinObject_FromTCHAR(retBuf, len);
        prc = Py_BuildValue("Oz", r1, NULL);  // place-holder for class
        Py_XDECREF(r1);
    }
    else
        PyWin_SetAPIError("CeRegEnumKey", rc);
    free(retBuf);
    return prc;
}

// Note that fixupMultiSZ and countString have both had changes
// made to support "incorrect strings".  The registry specification
// calls for strings to be terminated with 2 null bytes.  It seems
// some commercial packages install strings which dont conform,
// causing this code to fail - however, "regedit" etc still work
// with these strings (ie only we dont!).
static void fixupMultiSZ(TCHAR **str, TCHAR *data, int len)
{
    TCHAR *P;
    int i;
    TCHAR *Q;

    Q = data + len;
    for (P = data, i = 0; P < Q && *P != _T('\0'); P++, i++) {
        str[i] = P;
        for (; *P != _T('\0'); P++)
            ;
    }
}

static int countStrings(TCHAR *data, int len)
{
    int strings;
    TCHAR *P;
    TCHAR *Q = data + len;

    for (P = data, strings = 0; P < Q && *P != _T('\0'); P++, strings++)
        for (; P < Q && *P != _T('\0'); P++)
            ;

    return strings;
}

/* Convert PyObject into Registry data.
   Allocates space as needed. */
static int Py2Reg(PyObject *value, DWORD typ, BYTE **retDataBuf, DWORD *retDataSize)
{
    int i, j;
    switch (typ) {
        case REG_DWORD:
            if (value != Py_None && !PyInt_Check(value))
                return 0;
            *retDataBuf = (BYTE *)PyMem_NEW(DWORD, 1);
            *retDataSize = sizeof(DWORD);
            if (value == Py_None) {
                DWORD zero = 0;
                memcpy(*retDataBuf, &zero, sizeof(DWORD));
            }
            else
                memcpy(*retDataBuf, &PyInt_AS_LONG((PyIntObject *)value), sizeof(DWORD));
            break;
        case REG_SZ:
        case REG_EXPAND_SZ: {
            int numChars;
            TCHAR *temp = NULL;
            if (value == Py_None)
                numChars = 1;
            else {
                if (!PyWinObject_AsTCHAR(value, &temp, 0))
                    return 0;
                numChars = _tcslen(temp) + 1;
            }
            *retDataSize = numChars * sizeof(TCHAR);
            *retDataBuf = (BYTE *)PyMem_NEW(DWORD, *retDataSize);
            if (temp == NULL)
                _tcscpy((TCHAR *)*retDataBuf, _T(""));
            else
                _tcscpy((TCHAR *)*retDataBuf, temp);
            if (temp)
                PyWinObject_FreeTCHAR(temp);
        } break;
        case REG_MULTI_SZ: {
            DWORD cch = 0;
            PyObject *t;

            if (value == Py_None)
                i = 0;
            else {
                if (!PyList_Check(value))
                    return 0;
                i = PyList_Size(value);
            }
            for (j = 0; j < i; j++) {
                t = PyList_GET_ITEM((PyListObject *)value, j);
                TCHAR *temp;
                if (!PyWinObject_AsTCHAR(t, &temp, 0))
                    return 0;
                cch += _tcslen(temp) + 1;
                PyWinObject_FreeTCHAR(temp);
            }

            cch++;  // extra null.
            *retDataSize = cch * sizeof(TCHAR);
            *retDataBuf = (BYTE *)PyMem_NEW(TCHAR, *retDataSize);
            TCHAR *P = (TCHAR *)*retDataBuf;

            for (j = 0; j < i; j++) {
                t = PyList_GET_ITEM((PyListObject *)value, j);
                TCHAR *temp = _T("");
                PyWinObject_AsTCHAR(t, &temp, 0);
                _tcscpy(P, temp);
                P += _tcslen(temp) + 1;
                PyWinObject_FreeTCHAR(temp);
            }
            // And doubly-terminate the list...
            *P = _T('\0');
            break;
        }
        case REG_BINARY:
        // ALSO handle ALL unknown data types here.  Even if we cant support
        // it natively, we should handle the bits.
        default:
            if (value == Py_None)
                *retDataSize = 0;
            else {
                if (!PyString_Check(value))
                    return 0;
                *retDataSize = PyString_Size(value);
                *retDataBuf = (BYTE *)PyMem_NEW(char, *retDataSize);
                memcpy(*retDataBuf, PyString_AS_STRING((PyStringObject *)value), *retDataSize);
            }
            break;
    }

    return 1;
}

/* Convert Registry data into PyObject*/
PyObject *Reg2Py(BYTE *retDataBuf, DWORD retDataSize, DWORD typ)
{
    PyObject *obData;
    // couple of helers used for strings.
    int numChars = retDataSize / sizeof(TCHAR);
    TCHAR *sz = (TCHAR *)retDataBuf;

    switch (typ) {
        case REG_DWORD:
            if (retDataSize == 0)
                obData = Py_BuildValue("i", 0);
            else
                obData = Py_BuildValue("i", *(int *)retDataBuf);
            break;
        case REG_SZ:
        case REG_EXPAND_SZ:
            // retDataBuf may or may not have a trailing NULL in
            // the buffer.
            if (numChars && sz[numChars - 1] == _T('\0'))
                --numChars;
            if (numChars == 0)
                sz = _T("");
            obData = PyWinObject_FromTCHAR(sz, numChars);
            break;
        case REG_MULTI_SZ:
            if (retDataSize == 0)
                obData = PyList_New(0);
            else {
                int index = 0;
                int s = countStrings(sz, numChars);
                TCHAR **str = (TCHAR **)malloc(sizeof(TCHAR *) * s);

                fixupMultiSZ(str, sz, numChars);
                obData = PyList_New(s);
                for (index = 0; index < s; index++) {
                    PyObject *n = PyWinObject_FromTCHAR(str[index]);
                    PyList_SetItem(obData, index, n);
                }
                free(str);
                break;
            }
        case REG_BINARY:
        // ALSO handle ALL unknown data types here.  Even if we cant support
        // it natively, we should handle the bits.
        default:
            if (retDataSize == 0) {
                Py_INCREF(Py_None);
                obData = Py_None;
            }
            else
                obData = PyBuffer_FromMemory(retDataBuf, retDataSize);
            break;
    }
    if (obData == NULL)
        return NULL;
    else
        return obData;
}

// @pymethod (string,object,type)|wincerapi|CeRegEnumValue|Enumerates values of the specified open registry key. The
// function retrieves the name of one subkey each time it is called.
PyObject *PyCeRegEnumValue(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    int index;
    long rc;
    TCHAR *retValueBuf = NULL;
    BYTE *retDataBuf = NULL;
    DWORD retValueSize;
    DWORD retDataSize = 1024;
    DWORD typ;
    PyObject *prc = NULL;
    PyObject *obData = NULL;
    PyObject *obValueBuf = NULL;

    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm int|index||The index of the key to retrieve.
    // @pyparm int|bufSize|1024|The size of the buffer to allocate for the result.

    if (!PyArg_ParseTuple(args, "Oi|i:CeRegEnumValue", &obKey, &index, &retDataSize))
        goto done;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        goto done;

    if ((rc = CeRegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &retValueSize, NULL, NULL, NULL)) !=
        ERROR_SUCCESS) {
        PyWin_SetAPIError("CeRegQueryInfoKey", rc);
        goto done;
    }
    // retValueSize is in characters.
    ++retValueSize;  // include null terminators
    retValueBuf = (TCHAR *)malloc(retValueSize * sizeof(TCHAR));
    retDataBuf = (BYTE *)malloc(retDataSize);
    if (retValueBuf == NULL || retDataBuf == NULL) {
        PyErr_NoMemory();
        goto done;
    }

    // @pyseeapi CeRegEnumValue
    {
        PyW32_BEGIN_ALLOW_THREADS rc =
            CeRegEnumValue(hKey, index, retValueBuf, &retValueSize, NULL, &typ, (BYTE *)retDataBuf, &retDataSize);
        PyW32_END_ALLOW_THREADS
    }
    if (rc != ERROR_SUCCESS) {
        PyWin_SetAPIError("CeRegEnumValue", rc);
        goto done;
    }
    obData = Reg2Py(retDataBuf, retDataSize, typ);
    if (obData == NULL)
        goto done;
    obValueBuf = PyWinObject_FromTCHAR(retValueBuf, retValueSize);
    if (obValueBuf == NULL)
        goto done;
    prc = Py_BuildValue("OOi", obValueBuf, obData, typ);
done:
    Py_XDECREF(obValueBuf);
    Py_XDECREF(obData);
    if (retValueBuf)
        free(retValueBuf);
    if (retDataBuf)
        free(retDataBuf);
    return prc;
    // @comm This function is typically called repeatedly, until an exception is raised, indicating no more values.
}

// @pymethod <o PyHKEY>|wincerapi|CeRegOpenKeyEx|Opens the specified key.
PyObject *PyCeRegOpenKeyEx(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obSubKey;

    int res = 0;
    HKEY retKey;
    long rc;
    REGSAM sam = KEY_READ;
    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|subKey||The name of a key that this method opens.
    // This key must be a subkey of the key identified by the key parameter.
    // If key is one of the predefined keys, subKey may be None. In that case,
    // the handle returned is the same key handle passed in to the function.
    // @pyparm int|reserved|0|Reserved.  Must be zero.
    // @pyparm int|sam|KEY_READ|Specifies an access mask that describes the desired security access for the new key.
    // This parameter can be a combination of the following win32con constants:
    // <nl>KEY_ALL_ACCESS<nl>KEY_CREATE_LINK<nl>KEY_CREATE_SUB_KEY<nl>KEY_ENUMERATE_SUB_KEYS<nl>KEY_EXECUTE<nl>KEY_NOTIFY<nl>KEY_QUERY_VALUE<nl>KEY_READ<nl>KEY_SET_VALUE<nl>KEY_WRITE<nl>
    if (!PyArg_ParseTuple(args, "OO|ii:CeRegOpenKeyEx", &obKey, &obSubKey, &res, &sam))
        return NULL;
    // @pyseeapi CeRegOpenKeyEx
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *subKey;
    if (!PyWinObject_AsTCHAR(obSubKey, &subKey, 1))
        return NULL;

    PyW32_BEGIN_ALLOW_THREADS rc = CeRegOpenKeyEx(hKey, subKey, res, sam, &retKey);
    PyW32_END_ALLOW_THREADS PyWinObject_FreeTCHAR(subKey);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegOpenKeyEx", rc);
    return PyWinObject_FromCEHKEY(retKey);

    // @rdesc The return value is the handle of the opened key.
    // If the function fails, an exception is raised.
}

static double LI2double(LARGE_INTEGER *li)
{
    double d = li->LowPart;
    d = d + pow(2.0, 32.0) * li->HighPart;
    return d;
}

// @pymethod (int, int, long)|wincerapi|CeRegQueryInfoKey|Returns the number of
// subkeys, the number of values a key has,
// and if available the last time the key was modified as
// 100's of nanoseconds since Jan 1, 1600.
PyObject *PyCeRegQueryInfoKey(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    long rc;
    DWORD nSubKeys, nValues;
    FILETIME ft;
    LARGE_INTEGER li;
    PyObject *l;

    // @pyparm <o PyHKEY>/int|key||An already open key, or or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    if (!PyArg_ParseTuple(args, "O:CeRegQueryInfoKey", &obKey))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    // @pyseeapi CeRegQueryInfoKey
    if ((rc = CeRegQueryInfoKey(hKey, NULL, NULL, 0, &nSubKeys, NULL, NULL, &nValues, NULL, NULL, NULL, &ft)) !=
        ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegQueryInfoKey", rc);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    if (!(l = PyLong_FromDouble(LI2double(&li))))
        return NULL;
    return Py_BuildValue("iiO", nSubKeys, nValues, l);
}

// @pymethod (object,type)|wincerapi|CeRegQueryValueEx|Retrieves the type and data for a specified value name associated
// with an open registry key.
PyObject *PyCeRegQueryValueEx(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obValueName;

    long rc;
    BYTE *retBuf;
    DWORD bufSize;
    DWORD typ;

    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|valueName||The name of the value to query.
    if (!PyArg_ParseTuple(args, "OO:CeRegQueryValueEx", &obKey, &obValueName))
        return NULL;
    // @pyseeapi CeRegQueryValueEx

    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *valueName;
    if (!PyWinObject_AsTCHAR(obValueName, &valueName, 1))
        return NULL;
    if ((rc = CeRegQueryValueEx(hKey, valueName, NULL, NULL, NULL, &bufSize)) != ERROR_SUCCESS) {
        PyWinObject_FreeTCHAR(valueName);
        return PyWin_SetAPIError("CeRegQueryValueEx", rc);
    }
    retBuf = (BYTE *)malloc(bufSize);
    if (retBuf == NULL)
        return PyErr_NoMemory();

    rc = CeRegQueryValueEx(hKey, valueName, NULL, &typ, retBuf, &bufSize);
    PyWinObject_FreeTCHAR(valueName);
    if (rc != ERROR_SUCCESS) {
        free(retBuf);
        return PyWin_SetAPIError("CeRegQueryValueEx", rc);
    }
    PyObject *obData = Reg2Py(retBuf, bufSize, typ);
    free(retBuf);
    if (obData == NULL)
        return NULL;
    PyObject *result = Py_BuildValue("Oi", obData, typ);
    Py_DECREF(obData);
    return result;
    // @comm Values in the registry have name, type, and data components. This method
    // retrieves the data for the given value.
}

// @pymethod |wincerapi|CeRegSetValueEx|Stores data in the value field of an open registry key.
PyObject *PyCeRegSetValueEx(PyObject *self, PyObject *args)
{
    HKEY hKey;
    PyObject *obKey;
    PyObject *obValueName;
    PyObject *obRes;
    PyObject *value;
    BYTE *data;
    DWORD len;
    DWORD typ;

    DWORD rc;

    // @pyparm <o PyHKEY>/int|key||An already open key, or any one of the following win32con
    // constants:<nl>HKEY_CLASSES_ROOT<nl>HKEY_CURRENT_USER<nl>HKEY_LOCAL_MACHINE<nl>HKEY_USERS
    // @pyparm string|valueName||The name of the value to set.
    // If a value with this name is not already present in the key, the method adds it to the key.
    // <nl>If this parameter is None or an empty string and the type parameter is the wincerapi.REG_SZ type, this
    // function sets the same value the <om wincerapi.CeRegSetValue> method would set.
    // @pyparm any|reserved||Place holder for reserved argument.  Zero will always be passed to the API function.
    // @pyparm int|type||Type of data.
    // @flagh Value|Meaning
    // @flag REG_BINARY|Binary data in any form.
    // @flag REG_DWORD|A 32-bit number.
    // @flag REG_DWORD_LITTLE_ENDIAN|A 32-bit number in little-endian format. This is equivalent to REG_DWORD.<nl>In
    // little-endian format, a multi-byte value is stored in memory from the lowest byte (the little end) to the highest
    // byte. For example, the value 0x12345678 is stored as (0x78 0x56 0x34 0x12) in little-endian format. Windows NT
    // and Windows 95 are designed to run on little-endian computer architectures. A user may connect to computers that
    // have big-endian architectures, such as some UNIX systems.
    // @flag REG_DWORD_BIG_ENDIAN|A 32-bit number in big-endian format.
    // In big-endian format, a multi-byte value is stored in memory from the highest byte (the big end) to the lowest
    // byte. For example, the value 0x12345678 is stored as (0x12 0x34 0x56 0x78) in big-endian format.
    // @flag REG_EXPAND_SZ|A null-terminated string that contains unexpanded references to environment variables (for
    // example, %PATH%). It will be a Unicode or ANSI string depending on whether you use the Unicode or ANSI functions.
    // @flag REG_LINK|A Unicode symbolic link.
    // @flag REG_MULTI_SZ|An array of null-terminated strings, terminated by two null characters.
    // @flag REG_NONE|No defined value type.
    // @flag REG_RESOURCE_LIST|A device-driver resource list.
    // @flag REG_SZ|A null-terminated string. It will be a Unicode or ANSI string depending on whether you use the
    // Unicode or ANSI functions

    // @pyparm registry data|value||The value to be stored with the specified value name.
    if (!PyArg_ParseTuple(args, "OOOiO:CeRegSetValueEx", &obKey, &obValueName, &obRes, &typ, &value))
        return NULL;
    if (!PyWinObject_AsCEHKEY(obKey, &hKey))
        return NULL;
    TCHAR *valueName;
    if (!PyWinObject_AsTCHAR(obValueName, &valueName))
        return NULL;
    // @pyseeapi CeRegSetValueEx
    if (!Py2Reg(value, typ, &data, &len)) {
        PyWinObject_FreeTCHAR(valueName);
        PyErr_SetObject(PyExc_ValueError, Py_BuildValue("sO", "Data didn't match Registry Type", data));
        return NULL;
    }
    PyW32_BEGIN_ALLOW_THREADS rc = CeRegSetValueEx(hKey, valueName, NULL, typ, data, len);
    PyW32_END_ALLOW_THREADS PyWinObject_FreeTCHAR(valueName);
    if (rc != ERROR_SUCCESS)
        return PyWin_SetAPIError("CeRegSetValueEx", rc);
    Py_INCREF(Py_None);
    return Py_None;
    // @comm  This method can also set additional value and type information for the specified key.
    // <nl>The key identified by the key parameter must have been opened with KEY_SET_VALUE access.
    // To open the key, use the <om wincerapi.CeRegCreateKeyEx> or <om wincerapi.CeRegOpenKeyEx> methods.
    // <nl>Value lengths are limited by available memory.
    // Long values (more than 2048 bytes) should be stored as files with the filenames stored in the configuration
    // registry. This helps the registry perform efficiently. <nl>The key identified by the key parameter must have been
    // opened with KEY_SET_VALUE access.
}
