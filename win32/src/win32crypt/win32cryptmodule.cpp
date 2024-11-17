// @doc
#include "win32crypt.h"

// @pymethod bytes|win32crypt|CryptProtectData|Encrypts data using a session key derived from current user's logon
// credentials
static PyObject *PyCryptProtectData(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"DataIn", "DataDescr", "OptionalEntropy", "Reserved", "PromptStruct", "Flags", NULL};
    PyObject *obDataIn, *obDataDescr = Py_None, *obOptionalEntropy = Py_None, *obReserved = Py_None,
                        *obPromptStruct = Py_None;
    DWORD Flags = 0;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|OOOOk:CryptProtectData", keywords,
            &obDataIn,           // @pyparm bytes|DataIn||Data to be encrypted.
            &obDataDescr,        // @pyparm <o PyUnicode>|DataDescr|None|Description to add to the data
            &obOptionalEntropy,  // @pyparm bytes|OptionalEntropy|None|Extra entropy (eg password) for encryption
                                 // process, can be None
            &obReserved,         // @pyparm None|Reserved|None|Must be None
            &obPromptStruct,     //@pyparm <o PyCRYPTPROTECT_PROMPTSTRUCT>|PromptStruct|None|Contains options for UI
                                 // display during encryption and decryption, can be None
            &Flags))             //@pyparm int|Flags|0|Combination of CRYPTPROTECT_* flags
        return NULL;

    void *pReserved = NULL;
    if (obReserved != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Reserved must be None");
        return NULL;
    }

    TmpWCHAR DataDescr;
    if (!PyWinObject_AsWCHAR(obDataDescr, &DataDescr, TRUE))
        return NULL;

    DATA_BLOB DataIn = {0}, OptionalEntropy = {0}, *pOptionalEntropy = NULL, DataOut = {0};
    if (!PyWinObject_AsDATA_BLOB(obDataIn, &DataIn))
        return NULL;
    if (obOptionalEntropy != Py_None) {
        if (!PyWinObject_AsDATA_BLOB(obOptionalEntropy, &OptionalEntropy))
            return NULL;
        pOptionalEntropy = &OptionalEntropy;
    }

    CRYPTPROTECT_PROMPTSTRUCT PromptStruct = {0}, *pPromptStruct = NULL;
    if (obPromptStruct != Py_None) {
        if (!PyWinObject_AsCRYPTPROTECT_PROMPTSTRUCT(obPromptStruct, &PromptStruct))
            return NULL;
        pPromptStruct = &PromptStruct;
    }

    PyObject *ret = NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptProtectData(&DataIn, DataDescr, pOptionalEntropy, pReserved, pPromptStruct, Flags, &DataOut);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptProtectData");
    else
    {
        ret = PyWinObject_FromDATA_BLOB(&DataOut);
        LocalFree(DataOut.pbData);
    }
    PyWinObject_FreeWCHAR((WCHAR *)PromptStruct.szPrompt);
    return ret;
}

// @pymethod (str, bytes)|win32crypt|CryptUnprotectData|Decrypts data that was encrypted using <om
// win32crypt.CryptProtectData>.
// @rdesc The result is a tuple of (description, data) where description
// is the description that was passed to <om win32crypt.CryptProtectData>, and
// data is the unencrypted data.
static PyObject *PyCryptUnprotectData(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"DataIn", "OptionalEntropy", "Reserved", "PromptStruct", "Flags", NULL};
    PyObject *obDataIn, *obOptionalEntropy = Py_None, *obReserved = Py_None, *obPromptStruct = Py_None;
    DWORD Flags = 0;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|OOOk:CryptUnprotectData", keywords,
            &obDataIn,           // @pyparm bytes|DataIn||Data to be decrypted.
            &obOptionalEntropy,  // @pyparm bytes|OptionalEntropy|None|Extra entropy passed to CryptProtectData
            &obReserved,         // @pyparm None|Reserved|None|Must be None
            &obPromptStruct,     //@pyparm <o PyCRYPTPROTECT_PROMPTSTRUCT>|PromptStruct|None|Contains options for UI
                                 // display during encryption and decryption, can be None
            &Flags))             //@pyparm int|Flags|0|Combination of CRYPTPROTECT_* flags
        return NULL;

    void *pReserved = NULL;
    if (obReserved != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Reserved must be None");
        return NULL;
    }

    DATA_BLOB DataIn = {0}, OptionalEntropy = {0}, *pOptionalEntropy = NULL, DataOut = {0};
    if (!PyWinObject_AsDATA_BLOB(obDataIn, &DataIn))
        return NULL;
    if (obOptionalEntropy != Py_None) {
        if (!PyWinObject_AsDATA_BLOB(obOptionalEntropy, &OptionalEntropy))
            return NULL;
        pOptionalEntropy = &OptionalEntropy;
    }

    CRYPTPROTECT_PROMPTSTRUCT PromptStruct = {0}, *pPromptStruct = NULL;
    if (obPromptStruct != Py_None) {
        if (!PyWinObject_AsCRYPTPROTECT_PROMPTSTRUCT(obPromptStruct, &PromptStruct))
            return NULL;
        pPromptStruct = &PromptStruct;
    }

    WCHAR *DataDescr = NULL;
    PyObject *ret = NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptUnprotectData(&DataIn, &DataDescr, pOptionalEntropy, pReserved, pPromptStruct, Flags, &DataOut);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptUnprotectData");
    else
    {
        ret = Py_BuildValue("NN", PyWinObject_FromWCHAR(DataDescr), PyWinObject_FromDATA_BLOB(&DataOut));
        if (DataDescr)
            LocalFree(DataDescr);
        LocalFree(DataOut.pbData);
    }
    PyWinObject_FreeWCHAR((WCHAR *)PromptStruct.szPrompt);
    return ret;
}

// @pymethod string|win32crypt|CertAlgIdToOID|Converts an integer ALG_ID to it's szOID_ string representation
// @comm If there is no corresponding OID, None is returned
static PyObject *PyCertAlgIdToOID(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"AlgId", NULL};
    ALG_ID algid;
    LPCSTR szoid;
    // @pyparm int|AlgId||An algorithm identifier
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I:CertAlgIdToOID", keywords, &algid))
        return NULL;
    Py_BEGIN_ALLOW_THREADS szoid = CertAlgIdToOID(algid);
    Py_END_ALLOW_THREADS if (szoid == NULL)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyBytes_FromString(szoid);
}

// @pymethod int|win32crypt|CertOIDToAlgId|Converts a string object identfier to a numeric algorith identifier
// @comm If no matching ALG_ID is found, 0 is returned
static PyObject *PyCertOIDToAlgId(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ObjId", NULL};
    ALG_ID algid;
    LPCSTR szoid;
    // @pyparm string|ObjId||String szOID_* identifier
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:CertOIDToAlgId", keywords, &szoid))
        return NULL;
    Py_BEGIN_ALLOW_THREADS algid = CertOIDToAlgId(szoid);
    Py_END_ALLOW_THREADS return PyLong_FromUnsignedLong(algid);
}

// @pymethod <o PyCRYPTPROV>|win32crypt|CryptAcquireContext|Retrieve handle to a cryptographic service provider
static PyObject *PyCryptAcquireContext(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Container", "Provider", "ProvType", "Flags", NULL};
    PyObject *obcontainer_name, *obprovider_name, *ret = NULL;
    DWORD dwProvType = 0, dwFlags = 0;
    WCHAR *container_name = NULL, *provider_name = NULL;
    HCRYPTPROV hcryptprov;
    // @pyparm <o PyUnicode>|Container||Name of key container, can be none to use a Provider's default key container
    // (usually username)
    // @pyparm <o PyUnicode>|Provider||Name of cryptographic provider. (MS_*_PROV) Use None for user's default provider.
    // @pyparm int|ProvType||One of the PROV_* constants
    // @pyparm int|Flags||Combination of
    // CRYPT_VERIFYCONTEXT,CRYPT_NEWKEYSET,CRYPT_MACHINE_KEYSET,CRYPT_DELETEKEYSET,CRYPT_SILENT
    // @rdesc Returns None if CRYPT_DELETEKEYSET is specified, otherwise returns a handle to the provider
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOkk:CryptAcquireContext", keywords, &obcontainer_name,
                                     &obprovider_name, &dwProvType, &dwFlags))
        return NULL;
    if (!PyWinObject_AsWCHAR(obcontainer_name, &container_name, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obprovider_name, &provider_name, TRUE))
        goto done;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptAcquireContext(&hcryptprov, container_name, provider_name, dwProvType, dwFlags);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptAcquireContext");
    else
    {
        if (dwFlags & CRYPT_DELETEKEYSET) {  // when key is being deleted, no context is returned
            Py_INCREF(Py_None);
            ret = Py_None;
        }
        else
            ret = new PyCRYPTPROV(hcryptprov);
    }
done:
    if (container_name != NULL)
        PyWinObject_FreeWCHAR(container_name);
    if (provider_name != NULL)
        PyWinObject_FreeWCHAR(provider_name);
    return ret;
}

// @pymethod [(<o PyUnicode>,int),...]|win32crypt|CryptEnumProviders|List cryptography providers
// @rdesc Returns a sequence of tuples containing provider name and type
static PyObject *PyCryptEnumProviders(PyObject *self, PyObject *args)
{
    DWORD dwFlags = 0, dwIndex = 0, dwReserved = NULL, dwProvType = 0, cbProvName = 0;
    WCHAR *pszProvName = NULL;
    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    PyObject *ret_item = NULL;
    DWORD err = 0;
    BOOL bsuccess;

    while (TRUE) {
        cbProvName = 0;
        pszProvName = NULL;
        ret_item = NULL;
        Py_BEGIN_ALLOW_THREADS bsuccess = CryptEnumProviders(dwIndex, NULL, dwFlags, &dwProvType, NULL, &cbProvName);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            err = GetLastError();
            break;
        }
        pszProvName = (WCHAR *)malloc(cbProvName);
        if (pszProvName == NULL) {
            PyErr_Format(PyExc_MemoryError, "CryptEnumProviders: Unable to allocate %d bytes", cbProvName);
            break;
        }
        Py_BEGIN_ALLOW_THREADS bsuccess =
            CryptEnumProviders(dwIndex, NULL, dwFlags, &dwProvType, pszProvName, &cbProvName);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            err = GetLastError();
            break;
        }
        ret_item = Py_BuildValue("uk", pszProvName, dwProvType);
        if ((ret_item == NULL) || (PyList_Append(ret, ret_item) == -1))
            break;
        Py_DECREF(ret_item);
        free(pszProvName);
        dwIndex++;
    }
    // cleanup in case loop exited with error
    Py_XDECREF(ret_item);
    if (pszProvName)
        free(pszProvName);
    if (err != ERROR_NO_MORE_ITEMS) {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptEnumProviders", err);
    }
    return ret;
}

// @pymethod [(<o PyUnicode>,int),...]|win32crypt|CryptEnumProviderTypes|Lists available local cryptographic provider
// types
// @rdesc Returns a sequence of tuples containing name and identifier of provider types
// @comm Windows XP sp3 has a bug that causes this function to always fail with ERROR_MORE_DATA (234)
// See KB959160 for a hotfix
static PyObject *PyCryptEnumProviderTypes(PyObject *self, PyObject *args)
{
    DWORD dwFlags = 0, dwIndex = 0, dwReserved = NULL, dwProvType = 0, cbTypeName = 0;
    // both parameters reserved, declared as METH_NOARGS
    // if (!PyArg_ParseTupleAndKeywords(args, "|kk:CryptEnumProviderTypes", &dwFlags, &dwReserved))
    //	return NULL;
    WCHAR *pszTypeName = NULL;
    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    PyObject *ret_item = NULL;
    BOOL bsuccess;
    DWORD err = 0;

    while (TRUE) {
        cbTypeName = 0;
        pszTypeName = NULL;
        ret_item = NULL;
        Py_BEGIN_ALLOW_THREADS bsuccess =
            CryptEnumProviderTypes(dwIndex, NULL, dwFlags, &dwProvType, NULL, &cbTypeName);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            err = GetLastError();
            break;
        }
        pszTypeName = (WCHAR *)malloc(cbTypeName);
        if (pszTypeName == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", cbTypeName);
            break;
        }
        Py_BEGIN_ALLOW_THREADS bsuccess =
            CryptEnumProviderTypes(dwIndex, NULL, dwFlags, &dwProvType, pszTypeName, &cbTypeName);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            err = GetLastError();
            break;
        }
        // some provider types don't have names
        if (cbTypeName)
            ret_item = Py_BuildValue("uk", pszTypeName, dwProvType);
        else
            ret_item = Py_BuildValue("Ok", Py_None, dwProvType);
        if ((ret_item == NULL) || (PyList_Append(ret, ret_item) == -1))
            break;
        Py_DECREF(ret_item);
        free(pszTypeName);
        dwIndex++;
    }

    Py_XDECREF(ret_item);
    if (pszTypeName)
        free(pszTypeName);
    if (err != ERROR_NO_MORE_ITEMS) {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptEnumProviderTypes", err);
    }
    return ret;
}

// @pymethod <o PyUnicode>|win32crypt|CryptGetDefaultProvider|Returns default provider for local machine or current user
static PyObject *PyCryptGetDefaultProvider(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ProvType", "Flags", NULL};
    DWORD dwFlags = 0, dwIndex = 0, dwProvType = 0, cbProvName = 0;
    // Reserved arg is passed as a pointer, ignore it for now
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kk:CryptGetDefaultProvider", keywords,
                                     &dwProvType,  // @pyparm int|ProvType||Type of provider (PROV_* constant)
                                     &dwFlags))    // @pyparm int|Flags||CRYPT_MACHINE_DEFAULT or CRYPT_USER_DEFAULT
        return NULL;

    WCHAR *pszProvName = NULL;
    PyObject *ret = NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptGetDefaultProvider(dwProvType, NULL, dwFlags, NULL, &cbProvName);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptGetDefaultProvider");
    pszProvName = (WCHAR *)malloc(cbProvName);
    if (pszProvName == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", cbProvName);
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptGetDefaultProvider(dwProvType, NULL, dwFlags, pszProvName, &cbProvName);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptGetDefaultProvider");
    else ret = PyWinObject_FromWCHAR(pszProvName);
    free(pszProvName);
    return ret;
}

// @pymethod |win32crypt|CryptSetProviderEx|Sets default provider (for machine or user) for specified type
static PyObject *PyCryptSetProviderEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ProvName", "ProvType", "Flags", NULL};
    DWORD dwFlags = 0, dwProvType = 0, cbProvName = 0;
    PyObject *obProvName = NULL;
    WCHAR *ProvName = NULL;
    PyObject *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Okk:CryptSetProviderEx", keywords,
            &obProvName,  // @pyparm <o PyUnicode>|ProvName||Name of new default provider (MS_*_PROV value)
            &dwProvType,  // @pyparm int|ProvType||One of the PROV_* provider types
            &dwFlags))    // @pyparm int|Flags||CRYPT_MACHINE_DEFAULT or CRYPT_USER_DEFAULT.  Combine with
                          // CRYPT_DELETE_DEFAULT to remove default.
        return NULL;
    if (!PyWinObject_AsWCHAR(obProvName, &ProvName, TRUE))
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptSetProviderEx(ProvName, dwProvType, NULL, dwFlags);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptSetProviderEx");
    else
    {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    PyWinObject_FreeWCHAR(ProvName);
    return ret;
}

// @pymethod <o PyUnicode>|win32crypt|CryptFindLocalizedName|Returns localized name for predefined system stores (Root,
// My, .Default, .LocalMachine)
static PyObject *PyCryptFindLocalizedName(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CryptName", NULL};
    LPCWSTR localized_name = NULL;
    WCHAR *store_name = NULL;
    PyObject *obstore_name = NULL, *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CryptFindLocalizedName", keywords,
                                     &obstore_name))  // @pyparm <o PyUnicode>|CryptName||Name of a system store
        return NULL;
    if (!PyWinObject_AsWCHAR(obstore_name, &store_name, FALSE))
        return NULL;
    Py_BEGIN_ALLOW_THREADS localized_name = CryptFindLocalizedName(store_name);
    Py_END_ALLOW_THREADS if (localized_name == NULL)
    {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    else ret = PyWinObject_FromWCHAR(localized_name);
    PyWinObject_FreeWCHAR(store_name);
    return ret;
}

BOOL WINAPI CertEnumSystemStoreLocationCallback(LPCWSTR pvszStoreLocations, DWORD dwFlags, void *pvReserved, void *ret)
{
    CEnterLeavePython _celp;
    PyObject *storename = PyWinObject_FromWCHAR(pvszStoreLocations);
    if ((storename == NULL) || (PyList_Append((PyObject *)ret, storename) == -1)) {
        Py_XDECREF(storename);
        return FALSE;
    }
    Py_DECREF(storename);
    return TRUE;
}

// @pymethod [<o PyUnicode>,...]|win32crypt|CertEnumSystemStoreLocation|Lists system store locations
static PyObject *PyCertEnumSystemStoreLocation(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    DWORD dwFlags = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CertEnumSystemStoreLocation", keywords,
                                     &dwFlags))  // @pyparm int|Flags|0|Reserved, must be 0 if passed in
        return NULL;

    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertEnumSystemStoreLocation(dwFlags, ret, CertEnumSystemStoreLocationCallback);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CertEnumSystemStoreLocation");
    }
    return ret;
}

BOOL WINAPI CertEnumSystemStoreCallback(const void *pvSystemStore, DWORD dwFlags, PCERT_SYSTEM_STORE_INFO pStoreInfo,
                                        void *pvReserved, void *ret)
{
    CEnterLeavePython _celp;
    PyObject *storename = PyWinObject_FromWCHAR((WCHAR *)pvSystemStore);
    if (storename == NULL || PyList_Append((PyObject *)ret, storename) == -1) {
        Py_DECREF(storename);
        return FALSE;
    }
    return TRUE;
}

// @pymethod [<o PyUnicode>,...]|win32crypt|CertEnumSystemStore|Lists system stores
static PyObject *PyCertEnumSystemStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", "SystemStoreLocationPara", NULL};
    DWORD dwFlags = 0;
    CERT_SYSTEM_STORE_RELOCATE_PARA cssrp;
    void *pvSystemStoreLocationPara = NULL;
    PyObject *ret = NULL, *obSystemStoreLocationPara = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k|O:CertEnumSystemStore", keywords,
            &dwFlags,                     // @pyparm int|dwFlags||CERT_SYSTEM_STORE_* location, can be combined with
                                          // CERT_SYSTEM_STORE_RELOCATE_FLAG
            &obSystemStoreLocationPara))  // @pyparm <o
                                          // PyCERT_SYSTEM_STORE_RELOCATE_PARA>|pvSystemStoreLocationPara|None|Optional
                                          // - If flags contains CERT_SYSTEM_STORE_RELOCATE_FLAG must be a sequence
                                          // (PyHkey, unicode) representing a CERT_SYSTEM_STORE_RELOCATE_PARA, otherwise
                                          // should be a unicode store name
        return NULL;

    if (obSystemStoreLocationPara != NULL) {
        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
            if (!PyWinObject_AsPCERT_SYSTEM_STORE_RELOCATE_PARA(obSystemStoreLocationPara, &cssrp))
                return NULL;
            pvSystemStoreLocationPara = (void *)&cssrp;
        }
        else if (!PyWinObject_AsWCHAR(obSystemStoreLocationPara, (WCHAR **)&pvSystemStoreLocationPara, TRUE))
            return NULL;
    }
    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CertEnumSystemStore(dwFlags, pvSystemStoreLocationPara, ret, CertEnumSystemStoreCallback);
    Py_END_ALLOW_THREADS

        if (!bsuccess)
    {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CertEnumSystemStore");
    }

    if (pvSystemStoreLocationPara != NULL) {
        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
            PyWinObject_FreeWCHAR((WCHAR *)cssrp.pwszSystemStore);
        else
            PyWinObject_FreeWCHAR((WCHAR *)pvSystemStoreLocationPara);
    }
    return ret;
}

BOOL WINAPI CertEnumPhysicalStoreCallback(const void *pvSystemStore, DWORD dwFlags, LPCWSTR pwszStoreName,
                                          PCERT_PHYSICAL_STORE_INFO pStoreInfo, void *pvReserved, void *ret)
{
    CEnterLeavePython _celp;
    PyObject *storename = PyWinObject_FromWCHAR(pwszStoreName);
    if (storename == NULL || PyList_Append((PyObject *)ret, storename) == -1) {
        Py_XDECREF(storename);
        return FALSE;
    }
    return TRUE;
}

// @pymethod [<o PyUnicode>,...]|win32crypt|CertEnumPhysicalStore|Lists physical stores on computer
static PyObject *PyCertEnumPhysicalStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SystemStore", "Flags", NULL};
    DWORD dwFlags = 0;
    void *pvSystemStore = NULL;
    TmpWCHAR SystemStore;
    PyObject *ret = NULL, *obSystemStore = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CertEnumPhysicalStore", keywords,
                                     &obSystemStore,  //@pyparm <o PyUnicode>|pvSystemStore||Name of system store to
                                                      // enumerate physical locations for
                                     &dwFlags))       // @pyparm int|dwFlags||CERT_SYSTEM_STORE_* constant,
                                                      // CERT_SYSTEM_STORE_RELOCATE_FLAG  not supported yet
        return NULL;
    // pvSystemStore can also be a CERT_SYSTEM_STORE_RELOCATE_PARA pointer, not supported yet
    if (!PyWinObject_AsWCHAR(obSystemStore, &SystemStore))
        return NULL;
    pvSystemStore = (WCHAR *)SystemStore;
    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CertEnumPhysicalStore(pvSystemStore, dwFlags, (void *)ret, CertEnumPhysicalStoreCallback);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CertEnumPhysicalStore");
    }
    return ret;
}

// @pymethod <o PyCERTSTORE>|win32crypt|CertOpenStore|Opens a certificate store
static PyObject *PyCertOpenStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StoreProvider", "MsgAndCertEncodingType", "CryptProv", "Flags", "Para", NULL};
    HCERTSTORE hcertstore = NULL;
    HCRYPTPROV hcryptprov = NULL;
    void *pvPara = NULL;
    PyObject *ret = NULL, *obhcryptprov = NULL, *obStoreProvider = NULL, *obpvPara = NULL;
    LPCSTR StoreProvider = NULL;
    DWORD dwFlags = 0, err = 0, dwEncodingType = 0;
    BOOL free_wchar = FALSE;
    CRYPT_DATA_BLOB crypt_data_blob;
    CERT_SYSTEM_STORE_RELOCATE_PARA cssrp;
    PyWinBufferView pybuf;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O&kOkO:CertOpenStore", keywords, PyWinLong_AsVoidPtr,
            &StoreProvider,   // @pyparm int|StoreProvider||CERT_STORE_PROV_*, currently does not accept string provider
                              // names
            &dwEncodingType,  // @pyparm int|MsgAndCertEncodingType||Only used with CERT_STORE_PROV_MSG,
                              // CERT_STORE_PROV_PKCS7, and CERT_STORE_PROV_FILENAME. Usually should be
                              // X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING
            &obhcryptprov,    // @pyparm <o PyCRYPTPROV>|CryptProv||Handle to a CSP, can be None to use default provider
            &dwFlags,         // @pyparm int|Flags||Combination of CERT_STORE_*_FLAG flags
            &obpvPara))  // @pyparm object|Para|None|<o PyCERT_SYSTEM_STORE_RELOCATE_PARA>, or data specific to provider
        return NULL;
    if (!PyWinObject_AsHCRYPTPROV(obhcryptprov, &hcryptprov, TRUE))
        return NULL;
    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        if (!PyWinObject_AsPCERT_SYSTEM_STORE_RELOCATE_PARA(obpvPara, &cssrp))
            return NULL;
        pvPara = (void *)&cssrp;
    }
    else {
        switch ((ULONG_PTR)StoreProvider) {
            case CERT_STORE_PROV_PHYSICAL:
            case CERT_STORE_PROV_FILENAME:
            case CERT_STORE_PROV_SYSTEM:
            case CERT_STORE_PROV_SYSTEM_REGISTRY:
            case CERT_STORE_PROV_LDAP: {
                if (!PyWinObject_AsWCHAR(obpvPara, (WCHAR **)&pvPara))
                    return NULL;
                free_wchar = TRUE;
                break;
            }
            case CERT_STORE_PROV_REG: {
                if (!PyWinObject_AsHKEY(obpvPara, (HKEY *)&pvPara))
                    return NULL;
                break;
            }
            case CERT_STORE_PROV_FILE: {
                if (!PyWinObject_AsHANDLE(obpvPara, (HANDLE *)&pvPara))
                    return NULL;
                break;
            }
            case CERT_STORE_PROV_SERIALIZED:
            case CERT_STORE_PROV_PKCS7: {
                if (!pybuf.init(obpvPara))
                    return NULL;
                crypt_data_blob.pbData = (BYTE *)pybuf.ptr();
                crypt_data_blob.cbData = pybuf.len();
                pvPara = (void *)&crypt_data_blob;
                break;
            }
            case CERT_STORE_PROV_MEMORY: {
                // pvPara is not used, warn if something passed in
                if (obpvPara != Py_None)
                    PyErr_Warn(PyExc_RuntimeWarning, "Para ignored for CERT_STORE_PROV_MEMORY");
                break;
            }
            default: {
                PyErr_SetString(PyExc_NotImplementedError, "Specified store provider type not supported");
                return NULL;
            }
        }
    }

    Py_BEGIN_ALLOW_THREADS hcertstore = CertOpenStore(StoreProvider, dwEncodingType, hcryptprov, dwFlags, pvPara);
    Py_END_ALLOW_THREADS if (hcertstore == NULL)
    {
        err = GetLastError();
        // when delete is specified, return value is always NULL
        if ((dwFlags & CERT_STORE_DELETE_FLAG) && (err == 0)) {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
        else
            PyWin_SetAPIError("CertOpenStore", err);
    }
    else ret = PyWinObject_FromCERTSTORE(hcertstore);
    if (free_wchar)
        PyWinObject_FreeWCHAR((WCHAR *)pvPara);
    return ret;
}

// @pymethod <o PyCERTSTORE>|win32crypt|CertOpenSystemStore|Opens most commonly used Certificate Stores
static PyObject *PyCertOpenSystemStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SubsystemProtocol", "Prov", NULL};
    HCERTSTORE hcertstore = NULL;
    HCRYPTPROV hcryptprov = NULL;
    PyObject *obstore_name, *obhcryptprov = Py_None, *ret = NULL;
    WCHAR *store_name;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|O:CertOpenSystemStore", keywords,
            &obstore_name,   // @pyparm <o PyUnicode>|SubsystemProtocol||Name of store to open, will be created if it
                             // doesn't already exist
            &obhcryptprov))  // @pyparm <o PyCRYPTPROV>|Prov|None|Handle to CSP, use None for default provider
        return NULL;
    if (!PyWinObject_AsHCRYPTPROV(obhcryptprov, &hcryptprov, TRUE))
        return NULL;
    if (!PyWinObject_AsWCHAR(obstore_name, &store_name, FALSE))
        return NULL;
    Py_BEGIN_ALLOW_THREADS hcertstore = CertOpenSystemStore(hcryptprov, store_name);
    Py_END_ALLOW_THREADS if (hcertstore == NULL) PyWin_SetAPIError("CertOpenSystemStore");
    else ret = PyWinObject_FromCERTSTORE(hcertstore);
    PyWinObject_FreeWCHAR(store_name);
    return ret;
}

// @pymethod |win32crypt|CertRegisterSystemStore|Registers a certificate store
static PyObject *PyCertRegisterSystemStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SystemStore", "Flags", NULL};
    DWORD dwFlags = 0;
    void *pvSystemStore = NULL;
    CERT_SYSTEM_STORE_RELOCATE_PARA cssrp;
    PyObject *ret = NULL, *obSystemStore = NULL;
    // pStoreInfo and pvReserved currently must be NULL, do not accept as parms for now
    PCERT_SYSTEM_STORE_INFO pStoreInfo = NULL;
    void *pvReserved = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:CertRegisterSystemStore", keywords,
            &obSystemStore,  //@pyparm <o PyUnicode>|SystemStore||string/unicode name of store to be registered, or a
                             // sequence of (PyHkey, unicode) representing a CERT_SYSTEM_STORE_RELOCATE_PARA struct
            &dwFlags))  // @pyparm int|Flags||One of the CERT_SYSTEM_STORE_* location constants, can also be combined
                        // with CERT_SYSTEM_STORE_RELOCATE_FLAG and CERT_STORE_CREATE_NEW_FLAG
        return NULL;

    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        if (!PyWinObject_AsPCERT_SYSTEM_STORE_RELOCATE_PARA(obSystemStore, &cssrp))
            return NULL;
        pvSystemStore = (void *)&cssrp;
    }
    else if (!PyWinObject_AsWCHAR(obSystemStore, (WCHAR **)&pvSystemStore))
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertRegisterSystemStore(pvSystemStore, dwFlags, pStoreInfo, pvReserved);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CertRegisterSystemStore");
    else
    {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
        PyWinObject_FreeWCHAR((WCHAR *)cssrp.pwszSystemStore);
    else
        PyWinObject_FreeWCHAR((WCHAR *)pvSystemStore);
    return ret;
};

// @pymethod |win32crypt|CertUnregisterSystemStore|Unregisters a certificate store
static PyObject *PyCertUnregisterSystemStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SystemStore", "Flags", NULL};
    DWORD dwFlags = 0;
    void *pvSystemStore = NULL;
    PyObject *ret = NULL, *obSystemStore = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:CertUnregisterSystemStore", keywords,
            &obSystemStore,  //@pyparm <o PyUnicode>|SystemStore||Name of System store to be unregistered
            &dwFlags))       // @pyparm int|Flags||CERT_SYSTEM_STORE_RELOCATE_FLAG, CERT_STORE_DELETE_FLAG
                             // (CERT_SYSTEM_STORE_RELOCATE_FLAG  not supported yet)
        return NULL;
    if (!PyWinObject_AsWCHAR(obSystemStore, (WCHAR **)&pvSystemStore))
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertUnregisterSystemStore(pvSystemStore, dwFlags);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CertUnregisterSystemStore");
    else
    {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    PyWinObject_FreeWCHAR((WCHAR *)pvSystemStore);
    return ret;
};

// @pymethod dict|win32crypt|CryptFindOIDInfo|Returns information about an algorithm identifier or object identifier
static PyObject *PyCryptFindOIDInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeyType", "Key", "GroupId", NULL};
    PVOID key;
    DWORD keytype, groupid = 0;
    PyObject *obkey;
    ALG_ID alg_ids[2];
    PCCRYPT_OID_INFO oid_info = NULL;
    // @rdesc Returns a dictionary of CRYPT_OID_INFO data
    // @flagh KeyType|Type of Key
    // @flag CRYPT_OID_INFO_OID_KEY|An szOID_* character string
    // @flag CRYPT_OID_INFO_NAME_KEY|A unicode name
    // @flag CRYPT_OID_INFO_ALGID_KEY|An ALG_ID, one of the CALG_* integer constants
    // @flag CRYPT_OID_INFO_SIGN_KEY|A tuple of 2 CALG_* integers (hash algorithm, public key algorithm)
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "kO|k", keywords,
            &keytype,  // @pyparm int|KeyType||One of
                       // CRYPT_OID_INFO_OID_KEY,CRYPT_OID_INFO_NAME_KEY,CRYPT_OID_INFO_ALGID_KEY,CRYPT_OID_INFO_SIGN_KEY
            &obkey,     // @pyparm object|Key||Type is dependent on KeyType
            &groupid))  // @pyparm int|GroupId|0|CRYPT_*_GROUP_ID constant, or 0
        return NULL;

    switch (keytype) {
        case CRYPT_OID_INFO_OID_KEY:
            key = PyBytes_AsString(obkey);
            if (key == NULL)
                return NULL;
            break;
        case CRYPT_OID_INFO_NAME_KEY:
            if (!PyWinObject_AsWCHAR(obkey, (WCHAR **)&key, FALSE))
                return NULL;
            break;
        case CRYPT_OID_INFO_ALGID_KEY:
            alg_ids[0] = PyLong_AsLong(obkey);
            if (alg_ids[0] == (ALG_ID)-1 && PyErr_Occurred())
                return NULL;
            key = (PVOID)&alg_ids[0];
            break;
        case CRYPT_OID_INFO_SIGN_KEY:
            if (!PyTuple_Check(obkey)) {
                PyErr_SetString(PyExc_TypeError,
                                "Key must be a tuple of 2 ints when KeyType is CRYPT_OID_INFO_SIGN_KEY");
                return NULL;
            }
            if (!PyArg_ParseTuple(obkey, "II", &alg_ids[0], &alg_ids[1]))
                return NULL;
            key = (PVOID)&alg_ids;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unrecognized key type");
            return NULL;
    }
    Py_BEGIN_ALLOW_THREADS oid_info = CryptFindOIDInfo(keytype, key, groupid);
    Py_END_ALLOW_THREADS if (oid_info == NULL) return PyWin_SetAPIError("CryptFindOIDInfo");
    // docs say do NOT free the returned CRYPT_OID_INFO
    return PyWinObject_FromCRYPT_OID_INFO(oid_info);
}

// @pymethod object|win32crypt|CryptGetKeyIdentifierProperty|Retrieves a property from a certificate by its key
// indentifier
// @comm CERT_KEY_PROV_INFO_PROP_ID is only property currently supported
static PyObject *PyCryptGetKeyIdentifierProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeyIdentifier", "PropId", "Flags", "ComputerName", NULL};
    CRYPT_HASH_BLOB chb;
    DWORD propid = CERT_KEY_PROV_INFO_PROP_ID;
    DWORD flags = 0, bufsize = 0;
    PyObject *ret = NULL, *obkeyid, *obcomputername = Py_None;
    TmpWCHAR computername;
    VOID *buf, *reserved = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kkO:CryptGetKeyIdentifierProperty", keywords,
                                     &obkeyid,  // @pyparm string|KeyIdentifier||Hash that identifies a certificate key
                                     &propid,  // @pyparm int|PropId|CERT_KEY_PROV_INFO_PROP_ID|Property identifier, one
                                               // of the CERT_*_PROP_ID values
                                     &flags,   // @pyparm int|Flags|0|Use CRYPT_KEYID_MACHINE_FLAG for machine keyset.
                                               // (CRYPT_KEYID_ALLOC_FLAG is always added to Flags)
                                     &obcomputername))  // @pyparm <o PyUnicode>|ComputerName|None|Name of remote
                                                        // computer, use None for local machine
        return NULL;
    // This flag lets system allocate buffer of sufficient size to be freed with LocalFree
    flags |= CRYPT_KEYID_ALLOC_FLAG;
    PyWinBufferView pybuf(obkeyid);
    if (!pybuf.ok())
        return NULL;
    chb.pbData = (BYTE *)pybuf.ptr();
    chb.cbData = pybuf.len();
    if (!PyWinObject_AsWCHAR(obcomputername, &computername, TRUE))
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptGetKeyIdentifierProperty(&chb, propid, flags, computername, reserved, &buf, &bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptGetKeyIdentifierProperty");

    /* Usually only CERT_KEY_PROV_INFO_PROP_ID is used with this function.
        However, according to the docs other certificate properties can be requested.
        Not safe to just return the buffer unformatted since many crypto structs contain
        pointers instead of just raw data */
    if (propid == CERT_KEY_PROV_INFO_PROP_ID)
        ret = PyWinObject_FromCRYPT_KEY_PROV_INFO((PCRYPT_KEY_PROV_INFO)buf);
    else
        PyErr_SetString(PyExc_NotImplementedError, "Only CERT_KEY_PROV_INFO_PROP_ID is currently supported");
    if (buf)
        LocalFree(buf);
    return ret;
}

/*
BOOL WINAPI CryptSetKeyIdentifierProperty(
  const CRYPT_HASH_BLOB* pKeyIdentifier,
  DWORD dwPropId,
  DWORD dwFlags,
  LPCWSTR pwszComputerName,
  void* pvReserved,
  const void* pvData
);
*/

BOOL WINAPI CryptEnumKeyIdentifierProperties_callback(const CRYPT_HASH_BLOB *key_id, DWORD dwFlags, void *pvReserved,
                                                      void *pvArg, DWORD cProp, DWORD *rgdwPropId, void **rgpvData,
                                                      DWORD *rgcbData)
{
    CEnterLeavePython _celp;
    PyObject *props, *prop, *prop_data, *ret_item;
    props = PyTuple_New(cProp);
    if (props == NULL)
        return FALSE;
    for (DWORD prop_index = 0; prop_index < cProp; prop_index++) {
        /* PropId can be any of the CERT_*_PROP_ID values, but usually CERT_KEY_PROV_INFO_PROP_ID
            is the only one that matters.  Returning as a string is dangerous since many structs
            contain pointers to other places in buffer, which are no longer valid after the callback
            returns.  However, if an error is thrown for them, you'll never see the one that counts. */
        if (rgdwPropId[prop_index] == CERT_KEY_PROV_INFO_PROP_ID)
            prop_data = PyWinObject_FromCRYPT_KEY_PROV_INFO((PCRYPT_KEY_PROV_INFO)rgpvData[prop_index]);
        else {
            PyErr_Warn(PyExc_RuntimeWarning, "Key identifier property returned as raw data"),
                prop_data = PyBytes_FromStringAndSize((char *)rgpvData[prop_index], rgcbData[prop_index]);
        }
        if (prop_data == NULL) {
            Py_DECREF(props);
            return FALSE;
        }
        prop = Py_BuildValue("{s:k,s:N}", "PropId", rgdwPropId[prop_index], "Data", prop_data);
        if (prop == NULL) {
            Py_DECREF(props);
            Py_DECREF(prop_data);
            return FALSE;
        }
        PyTuple_SET_ITEM(props, prop_index, prop);
    }

    ret_item = Py_BuildValue("{s:N, s:N}", "KeyIdentifier",
                             PyBytes_FromStringAndSize((char *)key_id->pbData, key_id->cbData), "Props", props);
    if (ret_item == NULL) {
        Py_DECREF(props);
        return FALSE;
    }
    if (PyList_Append((PyObject *)pvArg, ret_item) == -1) {
        Py_DECREF(ret_item);
        return FALSE;
    }
    Py_DECREF(ret_item);
    return TRUE;
}

// @pymethod list|win32crypt|CryptEnumKeyIdentifierProperties|Enumerates private keys for certificates and their
// properties
static PyObject *PyCryptEnumKeyIdentifierProperties(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeyIdentifier", "PropId", "Flags", "ComputerName", NULL};
    PCRYPT_HASH_BLOB pchb = NULL;
    CRYPT_HASH_BLOB chb;
    DWORD propid = 0, flags = 0;
    PyObject *ret, *obkeyid = Py_None, *obcomputername = Py_None;
    TmpWCHAR computername;
    PVOID reserved = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|OkkO:CryptEnumKeyIdentifierProperties", keywords,
            &obkeyid,  // @pyparm string|KeyIdentifier|None|Id of a certificate key, can be None for all keys
            &propid,   // @pyparm int|PropId|0|CERT_*_PROP_ID constant. Limits returned values to specified propery, Use
                       // 0 for all
            &flags,    // @pyparm int|Flags|0|Can be CRYPT_KEYID_MACHINE_FLAG to list keys for local machine, or remote
                       // machine if ComputerName is given
            &obcomputername))  // @pyparm <o PyUnicode>|ComputerName|None|Name of remote computer, use None for local
                               // machine
        return NULL;
    PyWinBufferView pybuf;
    if (obkeyid != Py_None) {
        if (!pybuf.init(obkeyid))
            return NULL;
        chb.pbData = (BYTE *)pybuf.ptr();
        chb.cbData = pybuf.len();
    }
    if (!PyWinObject_AsWCHAR(obcomputername, &computername, TRUE))
        return NULL;

    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptEnumKeyIdentifierProperties(
        pchb, propid, flags, computername, reserved, (void *)ret, CryptEnumKeyIdentifierProperties_callback);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptEnumKeyIdentifierProperties");
    }
    return ret;
}

BOOL WINAPI CryptEnumOIDInfo_callback(PCCRYPT_OID_INFO pInfo, void *ret)
{
    CEnterLeavePython _celp;
    PyObject *ret_item;
    ret_item = PyWinObject_FromCRYPT_OID_INFO(pInfo);
    if (ret_item == NULL)
        return FALSE;
    if (PyList_Append((PyObject *)ret, ret_item) == -1) {
        Py_DECREF(ret_item);
        return FALSE;
    }
    Py_DECREF(ret_item);
    return TRUE;
}

// @pymethod list|win32crypt|CryptEnumOIDInfo|Lists registered Object Identifiers that belong to specified group
static PyObject *PyCryptEnumOIDInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"GroupId", NULL};
    PyObject *ret = NULL;
    DWORD groupid = 0, flags = 0;  // Flags is reserved
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CryptEnumOIDInfo", keywords,
                                     &groupid))  // @pyparm int|GroupId|0|The type of OIDs to enmerate, one of the
                                                 // CRYPT_*_OID_GROUP_ID constants or 0 to list all
        return NULL;
    ret = PyList_New(0);
    if (ret == NULL)
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptEnumOIDInfo(groupid, flags, (void *)ret, CryptEnumOIDInfo_callback);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        Py_DECREF(ret);
        ret = NULL;
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptEnumOIDInfo");
    }
    return ret;
}

// @pymethod <o PyCERT_CONTEXT>|win32crypt|CertAddSerializedElementToStore|Imports a serialized Certificate context,
// CRL, or CTL
// @comm Currently only Certificate contexts are supported
static PyObject *PyCertAddSerializedElementToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CertStore", "Element", "AddDisposition", "ContextTypeFlags", "Flags", NULL};
    PyObject *obcertstore = NULL, *obdata;
    DWORD flags = 0, contexttype = CERT_STORE_CERTIFICATE_CONTEXT_FLAG;
    DWORD contexttype_out, adddisposition;
    HCERTSTORE hcertstore;
    const VOID *context;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOk|kk:CertAddSerializedElementToStore", keywords,
            &obcertstore,  // @pyparm <o PyCERTSTORE>|CertStore||Certificate Store to which the context will be added,
                           // can be None
            &obdata,       // @pyparm buffer|Element||Serialized data
            &adddisposition,  // @pyparm int|AddDisposition||one of CERT_STORE_ADD_* values
            &contexttype,     // @pyparm int|ContextTypeFlags|CERT_STORE_CERTIFICATE_CONTEXT_FLAG|One of
                              // CERT_STORE_*_CONTEXT_FLAG constants
            &flags))          // @pyparm int|Flags|0|Reserved, use only 0
        return NULL;
    if (!PyWinObject_AsCERTSTORE(obcertstore, &hcertstore, TRUE))
        return NULL;
    PyWinBufferView pybuf(obdata);
    if (!pybuf.ok())
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertAddSerializedElementToStore(
        hcertstore, (BYTE *)pybuf.ptr(), pybuf.len(), adddisposition, flags, contexttype, &contexttype_out, &context);
    Py_END_ALLOW_THREADS

        if (!bsuccess) return PyWin_SetAPIError("CertAddSerializedElementToStore");
    if (contexttype_out == CERT_STORE_CERTIFICATE_CONTEXT)
        return PyWinObject_FromCERT_CONTEXT((PCCERT_CONTEXT)context);
    else if (contexttype_out == CERT_STORE_CTL_CONTEXT)
        return PyWinObject_FromCTL_CONTEXT((PCCTL_CONTEXT)context);
    else  // CERT_STORE_CRL_CONTEXT  not supported yet
        return PyErr_Format(PyExc_NotImplementedError, "Context type %d is not yet supported", contexttype_out);
}

// @pymethod dict|win32crypt|CryptQueryObject|Determines the cryptographic type of input data
// @rdesc Returns a dictionary containing
//	<nl>{MsgAndCertEncodingType:int,	## encoding type, usually X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING
//	<nl>ContentType:int,				## One of the CERT_QUERY_CONTENT_* constants
//	<nl>FormatType:int,					## One of the CERT_QUERY_FORMAT_* constants
//	<nl>CertStore:<o PyCERTSTORE>,		## Handle to certificate store containing all certficates in the object, may be
// None 	<nl>Msg:<o PyCRYPTMSG>,				## If input doesn't contains a PKCS7 message, will be None
// <nl>Context:<o PyCERT_CONTEXT>}		## A certificate, CRL, or CTL context depending on ContentType, may be None
static PyObject *PyCryptQueryObject(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ObjectType", "Object", "ExpectedContentTypeFlags", "ExpectedFormatTypeFlags",
                               "Flags",      NULL};
    void *input, *context;
    CERT_BLOB blob_input;
    TmpWCHAR fname_input;
    DWORD contenttype = CERT_QUERY_CONTENT_FLAG_ALL, contenttypeout;
    DWORD formattype = CERT_QUERY_FORMAT_FLAG_ALL, formattypeout;
    DWORD objecttype, encoding, flags = 0;  // Flags are reserved
    PyObject *obinput = NULL;
    PyObject *obcontext;
    HCERTSTORE hcertstore = NULL;
    HCRYPTMSG hcryptmsg = NULL;
    PyWinBufferView pybuf;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "kO|kkk:CryptQueryObject", keywords,
            &objecttype,   // @pyparm int|ObjectType||Type of input, CERT_QUERY_OBJECT_BLOB or CERT_QUERY_OBJECT_FILE
            &obinput,      // @pyparm str|Object||Raw data or a filename containing the data to be queried depending on
                           // ObjectType
            &contenttype,  // @pyparm int|ExpectedContentTypeFlags|CERT_QUERY_CONTENT_FLAG_ALL|One of the
                           // CERT_QUERY_CONTENT_FLAG_* constants
            &formattype,   // @pyparm int|ExpectedFormatTypeFlags|CERT_QUERY_FORMAT_FLAG_ALL|One of the
                           // CERT_QUERY_FORMAT_FLAG_* constants
            &flags))       // @pyparm int|Flags|0|Reserved, use only 0
        return NULL;
    switch (objecttype) {
        case CERT_QUERY_OBJECT_BLOB:
            if (!pybuf.init(obinput))
                return NULL;
            blob_input.pbData = (BYTE *)pybuf.ptr();
            blob_input.cbData = pybuf.len();
            input = (void *)&blob_input;
            break;
        case CERT_QUERY_OBJECT_FILE:
            if (!PyWinObject_AsWCHAR(obinput, &fname_input))
                return NULL;
            input = (void *)fname_input;
            break;
        default:
            return PyErr_Format(PyExc_ValueError, "Invalid input type specified: %d", objecttype);
    }

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS;
    bsuccess = CryptQueryObject(objecttype, input, contenttype, formattype, flags, &encoding, &contenttypeout,
                                &formattypeout, &hcertstore, &hcryptmsg, (const void **)&context);
    Py_END_ALLOW_THREADS;
    if (!bsuccess)
        return PyWin_SetAPIError("CryptQueryObject");

    switch (contenttypeout) {
        case CERT_QUERY_CONTENT_CERT:
        case CERT_QUERY_CONTENT_SERIALIZED_CERT:
            obcontext = PyWinObject_FromCERT_CONTEXT((PCCERT_CONTEXT)context);
            break;
        case CERT_QUERY_CONTENT_CTL:
        case CERT_QUERY_CONTENT_SERIALIZED_CTL:
            obcontext = PyWinObject_FromCTL_CONTEXT((PCCTL_CONTEXT)context);
            break;
        case CERT_QUERY_CONTENT_CRL:
        case CERT_QUERY_CONTENT_SERIALIZED_CRL:
            // obcontext=new PyCRL_CONTEXT(context);
            obcontext = PyLong_FromVoidPtr(context);
            break;
        default:
            Py_INCREF(Py_None);
            obcontext = Py_None;
    }
    return Py_BuildValue("{s:k,s:k,s:k,s:N,s:N,s:N}", "MsgAndCertEncodingType", encoding, "ContentType", contenttypeout,
                         "FormatType", formattypeout, "CertStore", PyWinObject_FromCERTSTORE(hcertstore), "Msg",
                         PyWinObject_FromCRYPTMSG(hcryptmsg), "Context", obcontext);
}

// @pymethod dict|win32crypt|CryptDecodeMessage|Decodes and decrypts a message, and verifies its signatures
// @rdesc Output params are returned as a dict containing:
//	<nl>{MsgType:int},					&nbsp&nbsp##Type of message decoded, one of
// CMSG_DATA,CMSG_SIGNED,CMSG_ENVELOPED,CMSG_SIGNED_AND_ENVELOPED,CMSG_HASHED 	<nl>InnerContentType:int,
//&nbsp&nbsp##Type of decoded content returned, uses same set of values as MsgType.  CMSG_DATA indicates unencoded data.
//	<nl>Decoded:str,					&nbsp&nbsp##The decoded data, will be None if ReturnData is False.
//	<nl>XchgCert:<o PyCERT_CONTEXT>,	&nbsp&nbsp##Certificate used to decode message
//	<nl>SignerCert:<o PyCERT_CONTEXT>}	&nbsp&nbsp##Certificate used to sign message
// @comm Only one level of encoding is interpreted.  Some types of messages will need multiple calls to completely
// decode.
//	For example, to decode a message created by <om win32crypt.CryptSignAndEncryptMessage>, one pass with
// CMSG_ENVELOPED_FLAG 	and a second pass using CMSG_SIGNED_FLAG are required to recover the original message text.
static PyObject *PyCryptDecodeMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"EncodedBlob", "DecryptPara",          "VerifyPara", "MsgTypeFlags",
                               "SignerIndex", "PrevInnerContentType", "ReturnData", NULL};
    BYTE *output_buf = NULL;
    DWORD signer_ind = 0, output_bufsize = 0;
    DWORD msg_type, msg_type_flags = CMSG_ALL_FLAGS, inner_type, prev_inner_type = 0;
    PCCERT_CONTEXT exchange_cert = NULL, signer_cert = NULL;
    PyCRYPT_DECRYPT_MESSAGE_PARA cdmp;
    PyCRYPT_VERIFY_MESSAGE_PARA cvmp;
    PyObject *obbuf, *obcdmp, *obcvmp = Py_None;
    BOOL returndata = TRUE, bsuccess;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|Okkkl:CryptDecodeMessage", keywords,
            &obbuf,   // @pyparm buffer|EncodedBlob||Data to be decoded
            &obcdmp,  // @pyparm dict|DecryptPara||<o PyCRYPT_DECRYPT_MESSAGE_PARA> containing decryption parms
            &obcvmp,  // @pyparm dict|VerifyPara|None|<o PyCRYPT_VERIFY_MESSAGE_PARA> containing signature verification
                      // parms
            &msg_type_flags,  // @pyparm int|MsgTypeFlags|CMSG_ALL_FLAGS|Combination of CMSG_DATA_FLAG,
                              // CMSG_SIGNED_FLAG, CMSG_ENVELOPED_FLAG, CMSG_SIGNED_AND_ENVELOPED_FLAG, or
                              // CMSG_HASHED_FLAG
            &signer_ind,  // @pyparm int|SignerIndex|0|Index of the signer to verify,  ignored if message is not signed.
            &prev_inner_type,  // @pyparm int|PrevInnerContentType|0|Content type returned from previous call, used
                               // during subsequent pass on a nested message
            &returndata))      // @pyparm boolean|ReturnData|True|Indicates if decoded data should be returned.
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(obcvmp, &cvmp))
        return NULL;
    if (!PyWinObject_AsCRYPT_DECRYPT_MESSAGE_PARA(obcdmp, &cdmp))
        return NULL;

    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptDecodeMessage(msg_type_flags, &cdmp, &cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(), prev_inner_type,
                           &msg_type, &inner_type, output_buf, &output_bufsize, &exchange_cert, &signer_cert);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        // Callback may raise an exception
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptDecodeMessage");
        return NULL;
    }
    if (!returndata)
        return Py_BuildValue("{s:k,s:k,s:O,s:N,s:N}", "MsgType", msg_type, "InnerContentType", inner_type, "Decoded",
                             Py_None, "XchgCert", PyWinObject_FromCERT_CONTEXT(exchange_cert), "SignerCert",
                             PyWinObject_FromCERT_CONTEXT(signer_cert));

    // if decoded data is requested, call function again with allocated output buffer
    // Any certs returned from from first call are kept
    PyObject *ret = NULL;
    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL)
        return PyErr_NoMemory();
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptDecodeMessage(msg_type_flags, &cdmp, &cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(), prev_inner_type,
                           &msg_type, &inner_type, output_buf, &output_bufsize, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptDecodeMessage");
    }
    else ret = Py_BuildValue("{s:k,s:k,s:N,s:N,s:N}", "MsgType", msg_type, "InnerContentType", inner_type, "Decoded",
                             PyBytes_FromStringAndSize((char *)output_buf, output_bufsize), "XchgCert",
                             PyWinObject_FromCERT_CONTEXT(exchange_cert), "SignerCert",
                             PyWinObject_FromCERT_CONTEXT(signer_cert));

    free(output_buf);
    if (!ret) {
        if (signer_cert)
            CertFreeCertificateContext(signer_cert);
        if (exchange_cert)
            CertFreeCertificateContext(exchange_cert);
    }
    return ret;
}

// @pymethod str|win32crypt|CryptEncryptMessage|Encrypts and encodes a message
static PyObject *PyCryptEncryptMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"EncryptPara", "RecipientCert", "ToBeEncrypted", NULL};
    PyObject *obbuf, *ret = NULL, *obcemp, *obrecipients;
    CRYPT_ENCRYPT_MESSAGE_PARA cemp = {0};
    BYTE *outputbuf = NULL;
    DWORD output_bufsize = 0, recipient_cnt = 0;
    PCCERT_CONTEXT *recipients = NULL;
    BOOL bsuccess;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOO:CryptEncryptMessage", keywords,
            &obcemp,        // @pyparm <o PyCRYPT_ENCRYPT_MESSAGE_PARA>|EncryptPara||Encryption parameters
            &obrecipients,  // @pyparm (<o PyCERT_CONTEXT>,...)|RecipientCert||Sequence of handles to recipients'
                            // certificates
            &obbuf))        // @pyparm buffer|ToBeEncrypted||Data to be encrypted
        return NULL;

    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsCRYPT_ENCRYPT_MESSAGE_PARA(obcemp, &cemp))
        return NULL;
    if (!PyWinObject_AsCERT_CONTEXTArray(obrecipients, &recipients, &recipient_cnt))
        return NULL;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptEncryptMessage(&cemp, recipient_cnt, recipients, (BYTE *)pybuf.ptr(),
                                                          pybuf.len(), outputbuf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptEncryptMessage");
    else
    {
        outputbuf = (BYTE *)malloc(output_bufsize);
        if (outputbuf == NULL)
            PyErr_Format(PyExc_MemoryError, "CryptEncryptMessage: Unable to allocate %d bytes", output_bufsize);
        else {
            Py_BEGIN_ALLOW_THREADS bsuccess = CryptEncryptMessage(&cemp, recipient_cnt, recipients, (BYTE *)pybuf.ptr(),
                                                                  pybuf.len(), outputbuf, &output_bufsize);
            Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptEncryptMessage");
            else ret = PyBytes_FromStringAndSize((char *)outputbuf, output_bufsize);
        }
    }

    PyWinObject_FreeCERT_CONTEXTArray(recipients, recipient_cnt);
    if (outputbuf != NULL)
        free(outputbuf);
    return ret;
}

// @pymethod str, <o PyCERT_CONTEXT>|win32crypt|CryptDecryptMessage|Decrypts an encrypted and encoded message
// @rdesc Returns the decrypted message and a handle to the certificate used to decrypt it
static PyObject *PyCryptDecryptMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"DecryptPara", "EncryptedBlob", NULL};
    PyObject *obbuf, *ret = NULL, *obcdmp;
    PyCRYPT_DECRYPT_MESSAGE_PARA cdmp;
    BYTE *output_buf = NULL;
    DWORD output_bufsize = 0;
    PCCERT_CONTEXT exchange_cert = NULL;
    BOOL bsuccess;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:CryptDecryptMessage", keywords,
                                     &obcdmp,  // @pyparm <o PyCRYPT_DECRYPT_MESSAGE_PARA>|DecryptPara||Dictionary
                                               // containing decryption parameters
                                     &obbuf))  // @pyparm buffer|EncryptedBlob||Buffer containing an encrypted message
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsCRYPT_DECRYPT_MESSAGE_PARA(obcdmp, &cdmp))
        return NULL;

    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptDecryptMessage(&cdmp, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize, NULL);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptDecryptMessage");

    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptDecryptMessage(&cdmp, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize, &exchange_cert);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptDecryptMessage");
    else ret = Py_BuildValue("NN", PyBytes_FromStringAndSize((char *)output_buf, output_bufsize),
                             PyWinObject_FromCERT_CONTEXT(exchange_cert));
    free(output_buf);
    return ret;
}

// @pymethod str|win32crypt|CryptSignAndEncryptMessage|Encrypts, encodes and signs a message using a certificate
static PyObject *PyCryptSignAndEncryptMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignPara", "EncryptPara", "RecipientCert", "ToBeSignedAndEncrypted", NULL};
    PyObject *ret = NULL, *obcsmp, *obcemp, *obrecipients, *obinput_buf;
    CRYPT_SIGN_MESSAGE_PARA csmp = {0};
    CRYPT_ENCRYPT_MESSAGE_PARA cemp = {0};
    PCCERT_CONTEXT *recipients = NULL;
    BYTE *output_buf = NULL;
    DWORD recipient_cnt = 0, output_bufsize = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOOO:CryptSignAndEncryptMessage", keywords,
            &obcsmp,        // @pyparm <o PyCRYPT_SIGN_MESSAGE_PARA>|SignPara||Message signing parameters
            &obcemp,        // @pyparm <o PyCRYPT_ENCRYPT_MESSAGE_PARA>|EncryptPara||Encryption parameters
            &obrecipients,  // @pyparm (<o PyCERT_CONTEXT>,...)|RecipientCert||Sequence of certificates of intended
                            // recipients
            &obinput_buf))  // @pyparm str|ToBeSignedAndEncrypted||Buffer containing data to be encoded in the message
        return NULL;

    PyWinBufferView pybuf(obinput_buf);
    if (!pybuf.ok())
        goto cleanup;
    if (!(PyWinObject_AsCRYPT_SIGN_MESSAGE_PARA(obcsmp, &csmp) &&
          PyWinObject_AsCRYPT_ENCRYPT_MESSAGE_PARA(obcemp, &cemp) &&
          PyWinObject_AsCERT_CONTEXTArray(obrecipients, &recipients, &recipient_cnt)))
        goto cleanup;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptSignAndEncryptMessage(
        &csmp, &cemp, recipient_cnt, recipients, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CryptSignAndEncryptMessage");
        goto cleanup;
    }

    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL) {
        PyErr_Format(PyExc_MemoryError, "CryptSignAndEncryptMessage: Unable to allocate %d bytes", output_bufsize);
        goto cleanup;
    }

    Py_BEGIN_ALLOW_THREADS bsuccess = CryptSignAndEncryptMessage(
        &csmp, &cemp, recipient_cnt, recipients, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CryptSignAndEncryptMessage");
        goto cleanup;
    }
    ret = PyBytes_FromStringAndSize((char *)output_buf, output_bufsize);

cleanup:
    PyWinObject_FreeCRYPT_SIGN_MESSAGE_PARA(&csmp);
    PyWinObject_FreeCERT_CONTEXTArray(recipients, recipient_cnt);
    if (output_buf != NULL)
        free(output_buf);
    return ret;
}

// @pymethod (<o PyCERT_CONTEXT>, str)|win32crypt|CryptVerifyMessageSignature|Verifies the signature of an encoded
// message
// @rdesc Returns the signing certificate and the decoded data.  If ReturnData parm is False, None is returned for data.
static PyObject *PyCryptVerifyMessageSignature(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignedBlob", "SignerIndex", "VerifyPara", "ReturnData", NULL};
    BYTE *output_buf = NULL;
    DWORD signer_ind = 0, output_bufsize = 0;
    PCCERT_CONTEXT signer_cert = NULL;
    PyCRYPT_VERIFY_MESSAGE_PARA cvmp;
    PyObject *obbuf, *obcvmp = Py_None;
    BOOL returndata = FALSE, bsuccess;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|kOl:CryptVerifyMessageSignature", keywords,
            &obbuf,       // @pyparm str|SignedBlob||Buffer containing a signed message
            &signer_ind,  // @pyparm int|SignerIndex|0|Index of the signer to verify, zero-based
            &obcvmp,  // @pyparm <o PyCRYPT_VERIFY_MESSAGE_PARA>|VerifyPara|None|Signature verification parameters, use
                      // None for defaults
            &returndata))  // @pyparm boolean|ReturnData|False|Indicates if decoded data should be returned.
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;

    if (!PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(obcvmp, &cvmp))
        return NULL;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptVerifyMessageSignature(&cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(),
                                                                  output_buf, &output_bufsize, &signer_cert);
    Py_END_ALLOW_THREADS
        // Callback may have already set an exception
        if (!bsuccess)
    {
        if (!PyErr_Occurred())
            PyWin_SetAPIError("CryptVerifyMessageSignature");
        return NULL;
    }
    if (!returndata)
        return Py_BuildValue("{s:N, s:O}", "SignerCert", PyWinObject_FromCERT_CONTEXT(signer_cert), "Decoded", Py_None);

    // Call function again to get decoded data if requested
    PyObject *ret = NULL;
    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL)
        PyErr_NoMemory();
    else {
        Py_BEGIN_ALLOW_THREADS bsuccess = CryptVerifyMessageSignature(&cvmp, signer_ind, (BYTE *)pybuf.ptr(),
                                                                      pybuf.len(), output_buf, &output_bufsize, NULL);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            // Callback may have already set an exception
            if (!PyErr_Occurred())
                PyWin_SetAPIError("CryptVerifyMessageSignature");
        }
        else ret = Py_BuildValue("{s:N, s:N}", "SignerCert", PyWinObject_FromCERT_CONTEXT(signer_cert), "Decoded",
                                 PyBytes_FromStringAndSize((char *)output_buf, output_bufsize));
    }
    if (output_buf)
        free(output_buf);
    if (signer_cert && !ret)
        CertFreeCertificateContext(signer_cert);
    return ret;
}

// @pymethod <o PyCERTSTORE>|win32crypt|CryptGetMessageCertificates|Extracts certificates encoded in a message
static PyObject *PyCryptGetMessageCertificates(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignedBlob", "MsgAndCertEncodingType", "CryptProv", "Flags", NULL};
    DWORD flags = 0, encoding_type = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    HCERTSTORE hcertstore = NULL;
    HCRYPTPROV csp = NULL;
    PyObject *obbuf, *obcsp = Py_None;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|kOk:CryptGetMessageCertificates", keywords,
            &obbuf,          // @pyparm buffer|SignedBlob||Buffer containing a signed message
            &encoding_type,  // @pyparm int|MsgAndCertEncodingType|X509_ASN_ENCODING\|PKCS_7_ASN_ENCODING|Message and
                             // certificate encoding types
            &obcsp,          // @pyparm <o PyCRYPTPROV>|CryptProv|None|Handle to a CSP, use None for default
            &flags))         // @pyparm int|Flags|0|Same flags used with <om win32crypt.CertOpenStore>
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsHCRYPTPROV(obcsp, &csp, TRUE))
        return NULL;
    Py_BEGIN_ALLOW_THREADS hcertstore =
        CryptGetMessageCertificates(encoding_type, csp, flags, (BYTE *)pybuf.ptr(), pybuf.len());
    Py_END_ALLOW_THREADS if (hcertstore == NULL) return PyWin_SetAPIError("CryptGetMessageCertificates");
    return PyWinObject_FromCERTSTORE(hcertstore);
}

// @pymethod int|win32crypt|CryptGetMessageSignerCount|Finds the number of signers of an encoded message
static PyObject *PyCryptGetMessageSignerCount(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignedBlob", "MsgEncodingType", NULL};
    DWORD encoding_type = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    long signer_cnt;
    PyObject *obbuf;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|k:CryptGetMessageSignerCount", keywords,
            &obbuf,           // @pyparm buffer|SignedBlob||Buffer containing a signed message
            &encoding_type))  // @pyparm int|MsgEncodingType|X509_ASN_ENCODING\|PKCS_7_ASN_ENCODING|Message encoding
                              // type
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    Py_BEGIN_ALLOW_THREADS signer_cnt = CryptGetMessageSignerCount(encoding_type, (BYTE *)pybuf.ptr(), pybuf.len());
    Py_END_ALLOW_THREADS if (signer_cnt == -1) return PyWin_SetAPIError("CryptGetMessageSignerCount");
    return PyLong_FromLong(signer_cnt);
}

// @pymethod str|win32crypt|CryptSignMessage|Signs and encodes a message
static PyObject *PyCryptSignMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignPara", "ToBeSigned", "DetachedSignature", NULL};
    PyObject *ret = NULL, *obcsmp, *obmsgs;
    CRYPT_SIGN_MESSAGE_PARA csmp = {0};
    BYTE *output_buf = NULL;
    BOOL detached_sig = FALSE;
    BYTE **msgs = NULL;
    DWORD msg_cnt, output_bufsize = 0, *msg_sizes = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|l:CryptSignMessage", keywords,
            &obcsmp,  // @pyparm <o PyCRYPT_SIGN_MESSAGE_PARA>|SignPara||Message signing parameters
            &obmsgs,  // @pyparm (str,...)|ToBeSigned||Sequence of strings containing message data.  Can only contain 1
                      // string if DetachedSignature parm is False.
            &detached_sig))  // @pyparm boolean|DetachedSignature|False|If True, only the signature itself is encoded in
                             // output msg.
        return NULL;
    if (!PyWinObject_AsCRYPT_SIGN_MESSAGE_PARA(obcsmp, &csmp))
        return NULL;  // last exit without cleanup
    if (!PyWinObject_AsPBYTEArray(obmsgs, &msgs, &msg_sizes, &msg_cnt))
        goto cleanup;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptSignMessage(&csmp, detached_sig, msg_cnt, (const BYTE **)msgs, msg_sizes, output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CryptSignMessage");
        goto cleanup;
    }
    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL) {
        PyErr_Format(PyExc_MemoryError, "CryptSignMessage: Unable to allocate %d bytes", output_bufsize);
        goto cleanup;
    }

    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptSignMessage(&csmp, detached_sig, msg_cnt, (const BYTE **)msgs, msg_sizes, output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CryptSignMessage");
        goto cleanup;
    }
    ret = PyBytes_FromStringAndSize((char *)output_buf, output_bufsize);

cleanup:
    PyWinObject_FreeCRYPT_SIGN_MESSAGE_PARA(&csmp);
    PyWinObject_FreePBYTEArray(msgs, msg_sizes, msg_cnt);
    if (output_buf != NULL)
        free(output_buf);
    return ret;
}

// @pymethod <o PyCERT_CONTEXT>|win32crypt|CryptVerifyDetachedMessageSignature|Verifies a signature that is encoded
// separately from the data
// @rdesc Returns the signing certificate
static PyObject *PyCryptVerifyDetachedMessageSignature(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SignerIndex", "DetachedSignBlob", "ToBeSigned", "VerifyPara", NULL};
    PyObject *obsig, *ret = NULL, *obmsgs, *obcvmp = Py_None;
    PyCRYPT_VERIFY_MESSAGE_PARA cvmp;
    BYTE **msgs = NULL;
    DWORD signer_ind, msg_cnt = 0, *msg_sizes = NULL;
    PCCERT_CONTEXT signer_cert;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "kOO|O:CryptVerifyDetachedMessageSignature", keywords,
            &signer_ind,  // @pyparm int|SignerIndex||Index of the signer to verify
            &obsig,       // @pyparm buffer|DetachedSignBlob||Buffer containing an encoded signature
            &obmsgs,      // @pyparm (buffer,...)|ToBeSigned||Sequence of buffers containing message data.
            &obcvmp))  // @pyparm <o PyCRYPT_VERIFY_MESSAGE_PARA>|VerifyPara|None|Signature verification parameters, use
                       // None for defaults
        return NULL;
    PyWinBufferView pybuf(obsig);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(obcvmp, &cvmp))
        return NULL;
    if (!PyWinObject_AsPBYTEArray(obmsgs, &msgs, &msg_sizes, &msg_cnt))
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptVerifyDetachedMessageSignature(
        &cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(), msg_cnt, (const BYTE **)msgs, msg_sizes, &signer_cert);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptVerifyDetachedMessageSignature");
    else ret = PyWinObject_FromCERT_CONTEXT(signer_cert);

    PyWinObject_FreePBYTEArray(msgs, msg_sizes, msg_cnt);
    return ret;
}

// @pymethod dict|win32crypt|CryptDecryptAndVerifyMessageSignature|Decrypts and decodes a signed message, and verifies
// its signatures
// @comm Usage is similar to CryptDecodeMessage, except that it undoes all levels of encoding and
//	returns the bare message.   This function is the counterpart of CryptSignAndEncryptMessage.
// @rdesc Output params are returned as a dict containing:
//	<nl>Decrypted:str,					&nbsp&nbsp##The decrypted message contents
//	<nl>XchgCert:<o PyCERT_CONTEXT>,	&nbsp&nbsp##Certificate whose private key was used to decrypt message
//	<nl>SignerCert:<o PyCERT_CONTEXT>	&nbsp&nbsp##Certificate used to sign message
static PyObject *PyCryptDecryptAndVerifyMessageSignature(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"EncryptedBlob", "DecryptPara", "VerifyPara", "SignerIndex", NULL};
    BYTE *output_buf = NULL;
    DWORD signer_ind = 0, output_bufsize = 0;
    PCCERT_CONTEXT exchange_cert, signer_cert;
    PyCRYPT_DECRYPT_MESSAGE_PARA cdmp;
    PyCRYPT_VERIFY_MESSAGE_PARA cvmp;
    PyObject *obbuf, *obcdmp, *obcvmp = Py_None, *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|Ok:CryptDecryptAndVerifyMessageSignature", keywords,
            &obbuf,        // @pyparm buffer|EncryptedBlob||Encoded message to be decrypted.
            &obcdmp,       // @pyparm <o PyCRYPT_DECRYPT_MESSAGE_PARA>|DecryptPara||Decryption parms
            &obcvmp,       // @pyparm <o PyCRYPT_VERIFY_MESSAGE_PARA>|VerifyPara|None|Signature verification parms
            &signer_ind))  // @pyparm int|SignerIndex|0|Index of the signer to verify, zero-based.
        return NULL;
    PyWinBufferView pybuf(obbuf);
    if (!pybuf.ok())
        return NULL;
    if (!PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(obcvmp, &cvmp))
        return NULL;
    if (!PyWinObject_AsCRYPT_DECRYPT_MESSAGE_PARA(obcdmp, &cdmp))
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptDecryptAndVerifyMessageSignature(
        &cdmp, &cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize, NULL, NULL);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptDecryptAndVerifyMessageSignature");

    output_buf = (BYTE *)malloc(output_bufsize);
    if (output_buf == NULL)
        return PyErr_NoMemory();
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptDecryptAndVerifyMessageSignature(&cdmp, &cvmp, signer_ind, (BYTE *)pybuf.ptr(), pybuf.len(), output_buf,
                                              &output_bufsize, &exchange_cert, &signer_cert);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptDecryptAndVerifyMessageSignature");
    else ret = Py_BuildValue(
        "{s:N,s:N,s:N}", "Decrypted", PyBytes_FromStringAndSize((char *)output_buf, output_bufsize), "XchgCert",
        PyWinObject_FromCERT_CONTEXT(exchange_cert), "SignerCert", PyWinObject_FromCERT_CONTEXT(signer_cert));

    free(output_buf);
    return ret;
}

BOOL PyWinObject_AsOID(PyObject *oboid, LPSTR *objid, BOOLEAN *oid_is_str)
{
    // ObjId can be szOID_* string or one of the numeric identifiers cast to LPSTR
    *objid = (LPSTR)PyLong_AsVoidPtr(oboid);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        *objid = PyBytes_AsString(oboid);
        if (*objid == NULL)
            return FALSE;
        *oid_is_str = TRUE;
        return TRUE;
    }
    // Hi-order word of int identifier must be 0 to distinguish it from a real pointer
    if (HIWORD(*objid)) {
        PyErr_Format(PyExc_ValueError, "%d is an invalid value for object identifier", *objid);
        return FALSE;
    }
    *oid_is_str = FALSE;
    return TRUE;
}

// @pymethod str|win32crypt|CryptEncodeObjectEx|Serializes and ASN encodes cryptographic structures
static PyObject *PyCryptEncodeObjectEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StructType", "StructInfo", "Flags", "CertEncodingType", "EncodePara", NULL};
    void *input_buf = NULL, *output_buf = NULL;
    DWORD input_bufsize = 0, output_bufsize = 0;
    DWORD flags = 0, encoding = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    PyObject *obstruct, *obstructtype, *obpara = Py_None, *ret = NULL;
    LPSTR structtype;
    BOOLEAN oid_is_str;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|kkO:CryptEncodeObjectEx", keywords,
                                     &obstructtype,  // @pyparm str/int|StructType||OID identifying type of data to be
                                                     // encoded, either szOID_* string or a numeric id
                                     &obstruct,      // @pyparm dict|StructInfo||Information to be encoded.  Contents of
                                                     // dict are dependent on StructType
                                     &flags,     // @pyparm int|Flags|0|Encoding options, combination of CRYPT_UNICODE_*
                                                 // constants.  CRYPT_ENCODE_ALLOC_FLAG is added to flags..
                                     &encoding,  // @pyparm int|CertEncodingType|X509_ASN_ENCODING combined with
                                                 // PKCS_7_ASN_ENCODING|Encoding types
                                     &obpara))   // @pyparm object|EncodePara|None|Not supported, use only None
        return NULL;
    flags |= CRYPT_ENCODE_ALLOC_FLAG;

    if (!PyWinObject_AsOID(obstructtype, &structtype, &oid_is_str))
        return NULL;
    if (obpara != Py_None) {
        PyErr_SetString(PyExc_NotImplementedError, "EncodePara not yet supported");
        return NULL;
    }

    // @flagh StructType|Type of input
    if ((oid_is_str && (strcmp(structtype, szOID_ENHANCED_KEY_USAGE) ==
                        0)) ||  // @flag szOID_ENHANCED_KEY_USAGE|<o PyCTL_USAGE> (sequence of OID's)
        (structtype == X509_ENHANCED_KEY_USAGE)) {  // @flag X509_ENHANCED_KEY_USAGE|<o PyCTL_USAGE> (sequence of OID's)
        input_bufsize = sizeof(CTL_USAGE);
        input_buf = malloc(input_bufsize);
        if (input_buf == NULL)
            return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", input_bufsize);
        if (!PyWinObject_AsCTL_USAGE(obstruct, (CTL_USAGE *)input_buf))
            goto cleanup;
    }
    else if ((oid_is_str &&
              (strcmp(structtype, szOID_KEY_USAGE) == 0)) ||  // @flag szOID_KEY_USAGE|<o PyCRYPT_BIT_BLOB>
             (structtype == X509_KEY_USAGE) ||                // @flag X509_KEY_USAGE|<o PyCRYPT_BIT_BLOB>
             (structtype == X509_BITS)) {                     // @flag X509_BITS|<o PyCRYPT_BIT_BLOB>
        input_bufsize = sizeof(CRYPT_BIT_BLOB);
        input_buf = malloc(input_bufsize);
        if (input_buf == NULL)
            return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", input_bufsize);
        if (!PyWinObject_AsCRYPT_BIT_BLOB(obstruct, (CRYPT_BIT_BLOB *)input_buf))
            goto cleanup;
    }
    else {
        if (oid_is_str)
            PyErr_Format(PyExc_NotImplementedError, "CryptEncodeObjectEx: Type %s is not yet supported", structtype);
        else
            PyErr_Format(PyExc_NotImplementedError, "CryptEncodeObjectEx: Type %d is not yet supported", structtype);
        goto cleanup;
    }
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptEncodeObjectEx(encoding, structtype, input_buf, flags, NULL, &output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptDecodeObjectEx");
    else ret = PyBytes_FromStringAndSize((char *)output_buf, output_bufsize);

cleanup:
    if ((oid_is_str && (strcmp(structtype, szOID_ENHANCED_KEY_USAGE) == 0)) || (structtype == X509_ENHANCED_KEY_USAGE))
        PyWinObject_FreeCTL_USAGE((CTL_USAGE *)input_buf);
    if (input_buf)
        free(input_buf);
    if (output_buf)
        LocalFree(output_buf);
    return ret;
}

// @pymethod object|win32crypt|CryptDecodeObjectEx|Decodes ASN encoded data
// @rdesc Type of object returned is dependent on the StructType to be decoded
static PyObject *PyCryptDecodeObjectEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StructType", "Encoded", "Flags", "CertEncodingType", "DecodePara", NULL};
    void *output_buf = NULL;
    DWORD output_bufsize = 0;
    DWORD flags = 0, encoding = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    PyObject *obstructtype, *obencoded, *obpara = Py_None, *ret = NULL;
    LPSTR structtype;
    BOOLEAN oid_is_str;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|kkO:CryptDecodeObjectEx", keywords,
                                     &obstructtype,  // @pyparm str/int|StructType||An OID identifying the type of data
                                                     // to be decoded, can be either str or int
                                     &obencoded,  // @pyparm str|Encoded||String or buffer containing ASN encoded data
                                     &flags,      // @pyparm int|Flags|0|Encoding options, can be combination
                                              // CRYPT_UNICODE_* constants.  CRYPT_ENCODE_ALLOC_FLAG is added to flags..
                                     &encoding,  // @pyparm int|CertEncodingType|X509_ASN_ENCODING combined with
                                                 // PKCS_7_ASN_ENCODING|Encoding types
                                     &obpara))   // @pyparm object|DecodePara|None|Not supported, use only None
        return NULL;
    flags |= CRYPT_ENCODE_ALLOC_FLAG;

    if (!PyWinObject_AsOID(obstructtype, &structtype, &oid_is_str))
        return NULL;
    PyWinBufferView pybuf(obencoded);
    if (!pybuf.ok())
        return NULL;
    if (obpara != Py_None) {
        PyErr_SetString(PyExc_NotImplementedError, "DecodePara not yet supported");
        return NULL;
    }

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptDecodeObjectEx(encoding, structtype, (BYTE *)pybuf.ptr(), pybuf.len(), flags,
                                                          NULL, &output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CryptDecodeObjectEx");
        goto cleanup;
    }

    // StructType can be either an OID string, or int constant cast to pointer
    // @flagh OID|Object returned
    if ((oid_is_str &&
         (strcmp(structtype, szOID_ENHANCED_KEY_USAGE) == 0)) ||  // @flag szOID_ENHANCED_KEY_USAGE|Sequence of OIDs
        (structtype == X509_ENHANCED_KEY_USAGE))                  // @flag X509_ENHANCED_KEY_USAGE|Sequence of OIDs
        ret = PyWinObject_FromCTL_USAGE((CTL_USAGE *)output_buf);
    else if ((oid_is_str &&
              (strcmp(structtype, szOID_KEY_USAGE) == 0)) ||  // @flag szOID_KEY_USAGE|<o PyCRYPT_BIT_BLOB>
             (structtype == X509_KEY_USAGE) ||                // @flag X509_KEY_USAGE|<o PyCRYPT_BIT_BLOB>
             (structtype == X509_BITS))                       // @flag X509_BITS|<o PyCRYPT_BIT_BLOB>
        ret = PyWinObject_FromCRYPT_BIT_BLOB((CRYPT_BIT_BLOB *)output_buf);
    else if ((oid_is_str && ((strcmp(structtype, szOID_SUBJECT_ALT_NAME) ==
                              0) ||  // @flag szOID_SUBJECT_ALT_NAME|<o PyCERT_ALT_NAME_INFO>
                             (strcmp(structtype, szOID_SUBJECT_ALT_NAME2) ==
                              0) ||  // @flag szOID_SUBJECT_ALT_NAME2|<o PyCERT_ALT_NAME_INFO>
                             (strcmp(structtype, szOID_ISSUER_ALT_NAME) ==
                              0) ||  // @flag szOID_ISSUER_ALT_NAME|<o PyCERT_ALT_NAME_INFO>
                             (strcmp(structtype, szOID_ISSUER_ALT_NAME2) ==
                              0) ||  // @flag szOID_ISSUER_ALT_NAME2|<o PyCERT_ALT_NAME_INFO>
                             (strcmp(structtype, szOID_NEXT_UPDATE_LOCATION) ==
                              0))) ||              // @flag szOID_NEXT_UPDATE_LOCATION|<o PyCERT_ALT_NAME_INFO>
             (structtype == X509_ALTERNATE_NAME))  // @flag X509_ALTERNATE_NAME|<o PyCERT_ALT_NAME_INFO>
        ret = PyWinObject_FromCERT_ALT_NAME_INFO((PCERT_ALT_NAME_INFO)output_buf);
    else if ((structtype == X509_NAME_VALUE) ||          // @flag X509_NAME_VALUE|<o PyCERT_NAME_VALUE>
             (structtype == X509_UNICODE_ANY_STRING) ||  // @flag X509_UNICODE_ANY_STRING|<o PyCERT_NAME_VALUE>
             (structtype == X509_UNICODE_NAME_VALUE))    // @flag X509_UNICODE_NAME_VALUE|<o PyCERT_NAME_VALUE>
        ret = PyWinObject_FromCERT_NAME_VALUE((PCERT_NAME_VALUE)output_buf);
    else if ((structtype == X509_NAME) ||        // @flag X509_NAME|<o PyCERT_NAME_INFO>
             (structtype == X509_UNICODE_NAME))  // @flag X509_UNICODE_NAME|<o PyCERT_NAME_INFO>
        ret = PyWinObject_FromCERT_NAME_INFO((PCERT_NAME_INFO)output_buf);
    else if ((oid_is_str && (strcmp(structtype, szOID_KEY_ATTRIBUTES) ==
                             0)) ||                // @flag szOID_KEY_ATTRIBUTES|<o PyCERT_KEY_ATTRIBUTES_INFO>
             (structtype == X509_KEY_ATTRIBUTES))  // @flag X509_KEY_ATTRIBUTES|<o PyCERT_KEY_ATTRIBUTES_INFO>
        ret = PyWinObject_FromCERT_KEY_ATTRIBUTES_INFO((PCERT_KEY_ATTRIBUTES_INFO)output_buf);
    else if ((oid_is_str && (strcmp(structtype, szOID_BASIC_CONSTRAINTS) ==
                             0)) ||                   // @flag szOID_BASIC_CONSTRAINTS|<o PyCERT_BASIC_CONSTRAINTS_INFO>
             (structtype == X509_BASIC_CONSTRAINTS))  // @flag X509_BASIC_CONSTRAINTS|<o PyCERT_BASIC_CONSTRAINTS_INFO>
        ret = PyWinObject_FromCERT_BASIC_CONSTRAINTS_INFO((PCERT_BASIC_CONSTRAINTS_INFO)output_buf);
    else if ((oid_is_str && (strcmp(structtype, szOID_BASIC_CONSTRAINTS2) ==
                             0)) ||  // @flag szOID_BASIC_CONSTRAINTS2|<o PyCERT_BASIC_CONSTRAINTS2_INFO>
             (structtype ==
              X509_BASIC_CONSTRAINTS2))  // @flag X509_BASIC_CONSTRAINTS2|<o PyCERT_BASIC_CONSTRAINTS2_INFO>
        ret = PyWinObject_FromCERT_BASIC_CONSTRAINTS2_INFO((PCERT_BASIC_CONSTRAINTS2_INFO)output_buf);
    else if ((oid_is_str &&
              ((strcmp(structtype, szOID_CERT_POLICIES) ==
                0) ||  // @flag szOID_CERT_POLICIES|Sequence of <o PyCERT_POLICY_INFO> objects
               (strcmp(structtype, szOID_APPLICATION_CERT_POLICIES) ==
                0))) ||  // @flag szOID_APPLICATION_CERT_POLICIES|Sequence of <o PyCERT_POLICY_INFO> objects
             (structtype == X509_CERT_POLICIES))  // @flag X509_CERT_POLICIES|Sequence of <o PyCERT_POLICY_INFO> objects
        ret = PyWinObject_FromCERT_POLICIES_INFO((PCERT_POLICIES_INFO)output_buf);
    else if (oid_is_str && (strcmp(structtype, szOID_SUBJECT_KEY_IDENTIFIER) ==
                            0))  // @flag szOID_SUBJECT_KEY_IDENTIFIER|Binary string containing the key identifier
        ret = PyBytes_FromStringAndSize((char *)((CRYPT_DATA_BLOB *)output_buf)->pbData,
                                        ((CRYPT_DATA_BLOB *)output_buf)->cbData);
    else if ((oid_is_str && (strcmp(structtype, szOID_AUTHORITY_KEY_IDENTIFIER) ==
                             0)) ||  // @flag szOID_AUTHORITY_KEY_IDENTIFIER|<o PyCERT_AUTHORITY_KEY_ID_INFO>
             (structtype == X509_AUTHORITY_KEY_ID))  // @flag X509_AUTHORITY_KEY_ID|<o PyCERT_AUTHORITY_KEY_ID_INFO>
        ret = PyWinObject_FromCERT_AUTHORITY_KEY_ID_INFO((PCERT_AUTHORITY_KEY_ID_INFO)output_buf);
    else
        PyErr_SetString(PyExc_NotImplementedError, "CryptDecodeObjectEx: Type is not yet supported");

cleanup:
    if (output_buf)
        LocalFree(output_buf);
    return ret;
}

/*

X509_CRL_DIST_POINTS CRL_DIST_POINTS_INFO

szOID_CRL_DIST_POINTS CRL_DIST_POINTS_INFO

 szOID_FRESHEST_CRL CRL_DIST_POINTS_INFO

 typedef struct _CRL_DIST_POINTS_INFO {
    DWORD cDistPoint;
    PCRL_DIST_POINT rgDistPoint;
} CRL_DIST_POINTS_INFO, *PCRL_DIST_POINTS_INFO;

typedef struct _CRL_DIST_POINT {
    CRL_DIST_POINT_NAME DistPointName;
    CRYPT_BIT_BLOB ReasonFlags;
    CERT_ALT_NAME_INFO CRLIssuer;
} CRL_DIST_POINT, *PCRL_DIST_POINT;


X509_KEY_USAGE_RESTRICTION szOID_KEY_USAGE_RESTRICTION
typedef struct _CERT_KEY_USAGE_RESTRICTION_INFO {
    DWORD cCertPolicyId;
    PCERT_POLICY_ID rgCertPolicyId;
    CRYPT_BIT_BLOB RestrictedKeyUsage;
} CERT_KEY_USAGE_RESTRICTION_INFO, *PCERT_KEY_USAGE_RESTRICTION_INFO;

typedef struct _CERT_POLICY_ID {
    DWORD cCertPolicyElementId;
    LPSTR* rgpszCertPolicyElementId;
} CERT_POLICY_ID, *PCERT_POLICY_ID;

*/

// @pymethod str|win32crypt|CertNameToStr|Converts an encoded CERT_NAME_INFO into a formatted string
// @comm Usually this encoded data is contained in a CERT_NAME_BLOB
static PyObject *PyCertNameToStr(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Name", "StrType", "CertEncodingType", NULL};
    CERT_NAME_BLOB cnb = {0, NULL};
    WCHAR *output_buf = NULL;
    DWORD output_buflen = 0;
    DWORD encoding = X509_ASN_ENCODING, strtype = CERT_SIMPLE_NAME_STR;
    PyObject *obname, *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kk:CertNameToStr", keywords,
                                     &obname,  // @pyparm str|Name||String containing an encoded CERT_NAME_INFO, as used
                                               // with certificate Issuer and Subject
                                     &strtype,    // @pyparm int|StrType|CERT_SIMPLE_NAME_STR|Type of string to format,
                                                  // one of CERT_SIMPLE_NAME_STR,CERT_OID_NAME_STR,CERT_X500_NAME_STR
                                     &encoding))  // @pyparm int|CertEncodingType|X509_ASN_ENCODING|Input encoding
        return NULL;
    PyWinBufferView pybuf(obname);
    if (!pybuf.ok())
        return NULL;
    cnb.pbData = (BYTE *)pybuf.ptr();
    cnb.cbData = pybuf.len();

    Py_BEGIN_ALLOW_THREADS output_buflen = CertNameToStr(encoding, &cnb, strtype, output_buf, output_buflen);
    Py_END_ALLOW_THREADS if (output_buflen == 0) return PyWin_SetAPIError("CertNameToStr");
    output_buf = (WCHAR *)malloc(output_buflen * sizeof(WCHAR));
    if (output_buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", output_buflen);
    Py_BEGIN_ALLOW_THREADS output_buflen = CertNameToStr(encoding, &cnb, strtype, output_buf, output_buflen);
    Py_END_ALLOW_THREADS if (output_buflen == 0) PyWin_SetAPIError("CertNameToStr");
    else ret = PyWinObject_FromWCHAR(output_buf);
    free(output_buf);
    return ret;
}

// @pymethod str|win32crypt|CryptFormatObject|Formats an encoded buffer into a readable string
// @comm Will handle all of the common certificate extension types
// @pyseeapi CryptFormatObject
static PyObject *PyCryptFormatObject(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StructType",   "Encoded", "FormatStrType", "CertEncodingType", "FormatType",
                               "FormatStruct", NULL};
    void *output_buf = NULL;
    DWORD output_bufsize = 0;
    PyObject *obencoded, *oboid, *obfmt_struct = Py_None, *ret = NULL;
    DWORD encoding = X509_ASN_ENCODING, string_fmt = 0, fmt_type = 0;
    void *fmt_struct = NULL;
    LPSTR oid;
    BOOLEAN oid_is_str;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|kkkO:CryptFormatObject", keywords,
                                     &oboid,  // @pyparm str/int|StructType||OID identifying the type of encoded data,
                                              // one of the szOID_* strings or an integer OID
                                     &obencoded,  // @pyparm str|Encoded||String containing encoded data to be formatted
                                     &string_fmt,  // @pyparm int|FormatStrType|0|String formatting options, combination
                                                   // of CRYPT_FORMAT_STR_MULTI_LINE, CRYPT_FORMAT_STR_NO_HEX
                                     &encoding,    // @pyparm int|CertEncodingType|X509_ASN_ENCODING|Input encoding
                                     &fmt_type,    // @pyparm int|FormatType|0|Reserved, use only 0
                                     &obfmt_struct))  // @pyparm None|FormatStruct|None|Reserved, use only None
        return NULL;
    if (!PyWinObject_AsOID(oboid, &oid, &oid_is_str))
        return NULL;
    PyWinBufferView pybuf(obencoded);
    if (!pybuf.ok())
        return NULL;
    if (obfmt_struct != Py_None) {
        PyErr_SetString(PyExc_ValueError, "FormatStruct must be None");
        return NULL;
    }

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CryptFormatObject(encoding, fmt_type, string_fmt, fmt_struct, oid,
                                                        (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptFormatObject");
    output_buf = malloc(output_bufsize);
    if (output_buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", output_bufsize);

    Py_BEGIN_ALLOW_THREADS bsuccess = CryptFormatObject(encoding, fmt_type, string_fmt, fmt_struct, oid,
                                                        (BYTE *)pybuf.ptr(), pybuf.len(), output_buf, &output_bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptFormatObject");
    else ret = PyWinObject_FromWCHAR((WCHAR *)output_buf);
    free(output_buf);
    return ret;
}

// @pymethod <o PyCERTSTORE>|win32crypt|PFXImportCertStore|Creates a certificate store from PKCS#12 data (*.PFX files)
// @pyseeapi PFXImportCertStore
// @comm MSDN docs specify that *one* of the Flags can be used, but in practice a combination is allowed
// @comm Depending on the encrypting application, a blank password ("") may be treated differently that a NULL
// password (None), so if you have a PFX with no password try both.
static PyObject *PyPFXImportCertStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"PFX", "Password", "Flags", NULL};
    CRYPT_DATA_BLOB input_buf;
    HCERTSTORE hcertstore;
    TmpWCHAR password;
    DWORD flags;
    PyObject *obinput_buf, *obpassword;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOk:PFXImportCertStore", keywords,
            &obinput_buf,  // @pyparm bytes|PFX||Buffer containing PKCS#12-formatted certificate(s)
            &obpassword,   // @pyparm str|Password||Password used to encrypt the data, may be None
            &flags))       // @pyparm int|Flags||Allowed flags are
                           // CRYPT_EXPORTABLE,CRYPT_USER_PROTECTED,CRYPT_MACHINE_KEYSET, and CRYPT_USER_KEYSET
        return NULL;
    if (!PyWinObject_AsDATA_BLOB(obinput_buf, &input_buf))
        return NULL;
    if (!PyWinObject_AsWCHAR(obpassword, &password, TRUE))
        return NULL;
    Py_BEGIN_ALLOW_THREADS hcertstore = PFXImportCertStore(&input_buf, password, flags);
    Py_END_ALLOW_THREADS if (hcertstore == NULL) return PyWin_SetAPIError("PFXImportCertStore");
    return PyWinObject_FromCERTSTORE(hcertstore);
}

// @pymethod boolean|win32crypt|PFXVerifyPassword|Checks if a PFX blob can be decrypted with given password
// @pyseeapi PFXVerifyPassword
static PyObject *PyPFXVerifyPassword(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"PFX", "Password", "Flags", NULL};
    CRYPT_DATA_BLOB input_buf;
    TmpWCHAR password;
    DWORD flags;
    PyObject *obinput_buf, *obpassword;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOk:PFXVerifyPassword", keywords,
            &obinput_buf,  // @pyparm bytes|PFX||Buffer containing PKCS#12-formatted certificate(s)
            &obpassword,   // @pyparm str|Password||Password used to encrypt the data, may be None
            &flags))       // @pyparm int|Flags||Allowed flags are
                           // CRYPT_EXPORTABLE,CRYPT_USER_PROTECTED,CRYPT_MACHINE_KEYSET, and CRYPT_USER_KEYSET
        return NULL;
    if (!PyWinObject_AsDATA_BLOB(obinput_buf, &input_buf))
        return NULL;
    if (!PyWinObject_AsWCHAR(obpassword, &password, TRUE))
        return NULL;
    BOOL out;
    Py_BEGIN_ALLOW_THREADS out = PFXVerifyPassword(&input_buf, password, flags);
    Py_END_ALLOW_THREADS return PyBool_FromLong(out);
}

// @pymethod boolean|win32crypt|PFXIsPFXBlob|Checks if data buffer contains a PFX blob
// @pyseeapi PFXIsPFXBlob
static PyObject *PyPFXIsPFXBlob(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"PFX", NULL};
    CRYPT_DATA_BLOB input_buf;
    PyObject *obinput_buf;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:PFXIsPFXBlob", keywords,
                                     &obinput_buf))  // @pyparm bytes|PFX||Buffer containing data to be checked
        return NULL;
    if (!PyWinObject_AsDATA_BLOB(obinput_buf, &input_buf))
        return NULL;
    BOOL out;
    Py_BEGIN_ALLOW_THREADS out = PFXIsPFXBlob(&input_buf);
    Py_END_ALLOW_THREADS return PyBool_FromLong(out);
}

// @pymethod str|win32crypt|CryptBinaryToString|Formats a binary buffer into the specified type of string
// @pyseeapi CryptBinaryToString
static PyObject *PyCryptBinaryToString(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Binary", "Flags", NULL};
    WCHAR *output_buf = NULL;
    PyObject *obinput_buf;
    DWORD flags, output_size;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:CryptBinaryToString", keywords,
            &obinput_buf,  // @pyparm bytes|Binary||Buffer containing raw data to be formatted
            &flags))       // @pyparm int|Flags||Type of output desired, win32cryptcon.CRYPT_STRING_* value
        return NULL;
    PyWinBufferView pybuf(obinput_buf);
    if (!pybuf.ok())
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptBinaryToString((BYTE *)pybuf.ptr(), pybuf.len(), flags, output_buf, &output_size);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptBinaryToString");
    output_buf = (WCHAR *)malloc(output_size * sizeof(WCHAR));
    if (output_buf == NULL)
        return PyErr_NoMemory();

    PyObject *ret = NULL;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptBinaryToString((BYTE *)pybuf.ptr(), pybuf.len(), flags, output_buf, &output_size);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CryptBinaryToString");
    else ret = PyWinObject_FromWCHAR(output_buf, output_size);
    free(output_buf);
    return ret;
}

// @pymethod bytes, int, int|win32crypt|CryptStringToBinary|Converts a formatted string back into raw bytes
// @pyseeapi CryptStringToBinary
// @rdesc Returns the decoded binary data, number of header characters skipped, and CRYPT_STRING_* value
// denoting the type of data found (used if input Flags is one of *_ANY values)
static PyObject *PyCryptStringToBinary(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"String", "Flags", NULL};
    TmpWCHAR input_buf;
    PyObject *obinput_buf, *oboutput_buf;
    BYTE *output_buf = NULL;
    DWORD input_size, output_size, flags, skip, out_flags;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:CryptStringToBinary", keywords,
            &obinput_buf,  // @pyparm str|String||Formatted string to be converted to raw binary data
            &flags))       // @pyparm int|Flags||Input format (win32cryptcon.CRYPT_STRING_*)
        return NULL;
    if (!PyWinObject_AsWCHAR(obinput_buf, &input_buf, FALSE, &input_size))
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptStringToBinary(input_buf, input_size, flags, output_buf, &output_size, &skip, &out_flags);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptStringToBinary");
    oboutput_buf = PyBytes_FromStringAndSize(NULL, output_size);
    if (oboutput_buf == NULL)
        return NULL;
    output_buf = (BYTE *)PyBytes_AS_STRING(oboutput_buf);
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptStringToBinary(input_buf, input_size, flags, output_buf, &output_size, &skip, &out_flags);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        Py_DECREF(oboutput_buf);
        return PyWin_SetAPIError("CryptStringToBinary");
    }
    return Py_BuildValue("Nkk", oboutput_buf, skip, out_flags);
}

// @module win32crypt|An interface to the win32 Cryptography API
static struct PyMethodDef win32crypt_functions[] = {
    // @pymeth CryptProtectData|Encrypts data using a session key derived from current user's logon credentials
    {"CryptProtectData", (PyCFunction)PyCryptProtectData, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptUnprotectData|Decrypts data that was encrypted using <om win32crypt.CryptProtectData>
    {"CryptUnprotectData", (PyCFunction)PyCryptUnprotectData, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptEnumProviders|Lists available cryptographic providers
    {"CryptEnumProviders", PyCryptEnumProviders, METH_NOARGS},
    // @pymeth CryptEnumProviderTypes|Lists available local cryptographic provider types
    {"CryptEnumProviderTypes", PyCryptEnumProviderTypes, METH_NOARGS},
    // @pymeth CryptGetDefaultProvider|Returns default provider for local machine or current user
    {"CryptGetDefaultProvider", (PyCFunction)PyCryptGetDefaultProvider, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptSetProviderEx|Sets default provider (for machine or user) for specified type
    {"CryptSetProviderEx", (PyCFunction)PyCryptSetProviderEx, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptAcquireContext|Retrieve handle to a cryptographic service provider
    {"CryptAcquireContext", (PyCFunction)PyCryptAcquireContext, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CryptFindLocalizedName|Return localized name for predefined system stores (Root, My, .Default,
    //.LocalMachine)
    {"CryptFindLocalizedName", (PyCFunction)PyCryptFindLocalizedName, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertEnumSystemStore|Lists system stores
    {"CertEnumSystemStore", (PyCFunction)PyCertEnumSystemStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertEnumSystemStoreLocation|Lists system store locations
    {"CertEnumSystemStoreLocation", (PyCFunction)PyCertEnumSystemStoreLocation, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertEnumPhysicalStore|Lists physical stores on computer
    {"CertEnumPhysicalStore", (PyCFunction)PyCertEnumPhysicalStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertRegisterSystemStore|Creates a new system certificate store
    {"CertRegisterSystemStore", (PyCFunction)PyCertRegisterSystemStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertUnregisterSystemStore|Unregister specified store, optionally deleting it
    {"CertUnregisterSystemStore", (PyCFunction)PyCertUnregisterSystemStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertOpenStore|Opens a certificate store
    {"CertOpenStore", (PyCFunction)PyCertOpenStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CertOpenSystemStore|Opens most commonly used Certificate Stores
    {"CertOpenSystemStore", (PyCFunction)PyCertOpenSystemStore, METH_VARARGS | METH_KEYWORDS},
    //@pymeth CryptFindOIDInfo|Retreives information about an object identifier or alorithm identifier
    {"CryptFindOIDInfo", (PyCFunction)PyCryptFindOIDInfo, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CertAlgIdToOID|Converts an integer ALG_ID to it's szOID_ string representation
    {"CertAlgIdToOID", (PyCFunction)PyCertAlgIdToOID, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CertOIDToAlgId|Converts a string object identfier to a numeric algorith identifier
    {"CertOIDToAlgId", (PyCFunction)PyCertOIDToAlgId, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptGetKeyIdentifierProperty|Retrieves a property from a certificate by it's key indentifier
    {"CryptGetKeyIdentifierProperty", (PyCFunction)PyCryptGetKeyIdentifierProperty, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptEnumKeyIdentifierProperties|Lists private keys for user or machine
    {"CryptEnumKeyIdentifierProperties", (PyCFunction)PyCryptEnumKeyIdentifierProperties, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptEnumOIDInfo|Lists registered object identfiers
    {"CryptEnumOIDInfo", (PyCFunction)PyCryptEnumOIDInfo, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CertAddSerializedElementToStore|Creates a new Certificate, CRL, or CTL context from serialized data
    {"CertAddSerializedElementToStore", (PyCFunction)PyCertAddSerializedElementToStore, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptQueryObject|Determines the type of serialized or encoded data
    {"CryptQueryObject", (PyCFunction)PyCryptQueryObject, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptDecodeMessage|Decrypts an encoded message and verifies a signature
    {"CryptDecodeMessage", (PyCFunction)PyCryptDecodeMessage, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptEncryptMessage|Encrypts and encodes a message
    {"CryptEncryptMessage", (PyCFunction)PyCryptEncryptMessage, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptDecryptMessage|Decrypts an encrypted and encoded message
    {"CryptDecryptMessage", (PyCFunction)PyCryptDecryptMessage, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptSignAndEncryptMessage|Decrypts an encrypted and encoded message
    {"CryptSignAndEncryptMessage", (PyCFunction)PyCryptSignAndEncryptMessage, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptVerifyMessageSignature|Verifies a message signature
    {"CryptVerifyMessageSignature", (PyCFunction)PyCryptVerifyMessageSignature, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptGetMessageCertificates|Extracts certificates encoded in a message
    {"CryptGetMessageCertificates", (PyCFunction)PyCryptGetMessageCertificates, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptGetMessageSignerCount|Finds the number of signers of an encoded message
    {"CryptGetMessageSignerCount", (PyCFunction)PyCryptGetMessageSignerCount, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptSignMessage|Signs and encodes a message
    {"CryptSignMessage", (PyCFunction)PyCryptSignMessage, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptVerifyDetachedMessageSignature|Verifies a signature that is encoded separately from the data
    {"CryptVerifyDetachedMessageSignature", (PyCFunction)PyCryptVerifyDetachedMessageSignature,
     METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptDecryptAndVerifyMessageSignature|Decrypts and decodes a signed message, and verifies its signatures
    {"CryptDecryptAndVerifyMessageSignature", (PyCFunction)PyCryptDecryptAndVerifyMessageSignature,
     METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptEncodeObjectEx|Serializes and ASN encodes cryptographic structures
    {"CryptEncodeObjectEx", (PyCFunction)PyCryptEncodeObjectEx, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptDecodeObjectEx|Decodes ASN encodes data
    {"CryptDecodeObjectEx", (PyCFunction)PyCryptDecodeObjectEx, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CertNameToStr|Converts an encoded CERT_NAME_INFO into a formatted string
    {"CertNameToStr", (PyCFunction)PyCertNameToStr, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptFormatObject|Formats an encoded buffer into a readable string
    {"CryptFormatObject", (PyCFunction)PyCryptFormatObject, METH_VARARGS | METH_KEYWORDS},
    // @pymeth PFXImportCertStore|Creates a certificate store from PKCS#12 data (*.PFX files)
    {"PFXImportCertStore", (PyCFunction)PyPFXImportCertStore, METH_VARARGS | METH_KEYWORDS},
    // @pymeth PFXVerifyPassword|Checks if a PFX blob can be decrypted with given password
    {"PFXVerifyPassword", (PyCFunction)PyPFXVerifyPassword, METH_VARARGS | METH_KEYWORDS},
    // @pymeth PFXIsPFXBlob|Checks if data buffer contains a PFX blob
    {"PFXIsPFXBlob", (PyCFunction)PyPFXIsPFXBlob, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptBinaryToString|Formats a binary buffer into the specified type of string
    {"CryptBinaryToString", (PyCFunction)PyCryptBinaryToString, METH_VARARGS | METH_KEYWORDS},
    // @pymeth CryptStringToBinary|Converts a formatted string back into raw bytes
    {"CryptStringToBinary", (PyCFunction)PyCryptStringToBinary, METH_VARARGS | METH_KEYWORDS},
    {NULL, NULL}};

PyObject *dummy_tuple = NULL;

PYWIN_MODULE_INIT_FUNC(win32crypt)
{
    PYWIN_MODULE_INIT_PREPARE(win32crypt, win32crypt_functions, "Support for Windows cryptography functions");

    if (PyType_Ready(&PyCRYPTPROVType) == -1 || PyType_Ready(&PyCRYPTKEYType) == -1 ||
        PyType_Ready(&PyCRYPTHASHType) == -1 || PyType_Ready(&PyCRYPTMSGType) == -1 ||
        PyType_Ready(&PyCERTSTOREType) == -1 || PyType_Ready(&PyCERT_CONTEXTType) == -1 ||
        PyType_Ready(&PyCTL_CONTEXTType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    dummy_tuple = PyTuple_New(0);

    return module;
}
