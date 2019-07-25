// @doc
#include "win32crypt.h"

// @object PyCRYPTPROV|Handle to a cryptographic provider, created using <om cryptoapi.CryptAcquireContext>
struct PyMethodDef PyCRYPTPROV::methods[] = {
    // @pymeth CryptReleaseContext|Releases the CSP handle
    {"CryptReleaseContext", (PyCFunction)PyCRYPTPROV::PyCryptReleaseContext, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptGenKey|Generates a key pair or a session key
    {"CryptGenKey", (PyCFunction)PyCRYPTPROV::PyCryptGenKey, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptGetProvParam|Retrieves specified attribute of provider
    {"CryptGetProvParam", (PyCFunction)PyCRYPTPROV::PyCryptGetProvParam, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptGetUserKey|Returns a handle to one of user's key pairs
    {"CryptGetUserKey", (PyCFunction)PyCRYPTPROV::PyCryptGetUserKey, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptGenRandom|Generates random data of specified length
    {"CryptGenRandom", (PyCFunction)PyCRYPTPROV::PyCryptGenRandom, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptCreateHash|Creates a hash object for hashing large amounts of data
    {"CryptCreateHash", (PyCFunction)PyCRYPTPROV::PyCryptCreateHash, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptImportKey|Imports a key exported by <om PyCRYPTKEY::CryptExportKey>
    {"CryptImportKey", (PyCFunction)PyCRYPTPROV::PyCryptImportKey, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptExportPublicKeyInfo|Exports a public key to send to other users
    {"CryptExportPublicKeyInfo", (PyCFunction)PyCRYPTPROV::PyCryptExportPublicKeyInfo, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptImportPublicKeyInfo|Imports another user's public key
    {"CryptImportPublicKeyInfo", (PyCFunction)PyCRYPTPROV::PyCryptImportPublicKeyInfo, METH_KEYWORDS | METH_VARARGS},
    {NULL}};

PyTypeObject PyCRYPTPROVType = {PYWIN_OBJECT_HEAD "PyCRYPTPROV",
                                sizeof(PyCRYPTPROV),
                                0,
                                PyCRYPTPROV::deallocFunc, /* tp_dealloc */
                                0,                        /* tp_print */
                                0,                        /* tp_getattr */
                                0,                        /* tp_setattr */
                                0,                        /* tp_compare */
                                0,                        /* tp_repr */
                                0,                        /* tp_as_number */
                                0,                        /* tp_as_sequence */
                                0,                        /* tp_as_mapping */
                                0,
                                0,                                         /* tp_call */
                                0,                                         /* tp_str */
                                PyCRYPTPROV::getattro,                     /* tp_getattr */
                                PyCRYPTPROV::setattro,                     /* tp_setattr */
                                0,                                         // PyBufferProcs *tp_as_buffer
                                Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
                                0,                                         // tp_doc
                                0,                                         // traverseproc tp_traverse
                                0,                                         // tp_clear
                                0,                                         // richcmpfunc tp_richcompare
                                0,                                         // tp_weaklistoffset
                                0,                                         // getiterfunc tp_iter
                                0,                                         // iternextfunc tp_iternext
                                PyCRYPTPROV::methods,
                                PyCRYPTPROV::members};

struct PyMemberDef PyCRYPTPROV::members[] = {{NULL}};

int PyCRYPTPROV::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    return PyObject_GenericSetAttr(self, obname, v);
}

PyObject *PyCRYPTPROV::getattro(PyObject *self, PyObject *obname) { return PyObject_GenericGetAttr(self, obname); }

PyCRYPTPROV::~PyCRYPTPROV(void) { CryptReleaseContext(hcryptprov, 0); }

void PyCRYPTPROV::deallocFunc(PyObject *ob) { delete (PyCRYPTPROV *)ob; }

PyCRYPTPROV::PyCRYPTPROV(HCRYPTPROV hcryptprov)
{
    ob_type = &PyCRYPTPROVType;
    _Py_NewReference(this);
    this->hcryptprov = hcryptprov;
}

BOOL PyWinObject_AsHCRYPTPROV(PyObject *obhcryptprov, HCRYPTPROV *hcryptprov, BOOL bNoneOK)
{
    if (bNoneOK && (obhcryptprov == Py_None)) {
        *hcryptprov = NULL;
        return true;
    }
    if (obhcryptprov->ob_type != &PyCRYPTPROVType) {
        PyErr_SetString(PyExc_TypeError, "Object must be of type PyCRYPTPROV");
        return FALSE;
    }
    *hcryptprov = ((PyCRYPTPROV *)obhcryptprov)->GetHCRYPTPROV();
    return TRUE;
}

// @pymethod |PyCRYPTPROV|CryptReleaseContext|Releases the CSP handle
PyObject *PyCRYPTPROV::PyCryptReleaseContext(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    HCRYPTPROV hc = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    DWORD dwFlags = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CryptReleaseContext", keywords,
                                     &dwFlags))  // @pyparm int|Flags|0|Reserved, use 0 if passed in
        return NULL;
    if (CryptReleaseContext(hc, dwFlags)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyWin_SetAPIError("CryptReleaseContext");
    return NULL;
}

// @pymethod <o PyCRYPTKEY>|PyCRYPTPROV|CryptGenKey|Generates a key pair or a session key
// @comm Differs from Api call in that the length is passed in separately
PyObject *PyCRYPTPROV::PyCryptGenKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Algid", "Flags", "KeyLen", NULL};
    HCRYPTPROV hc = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    DWORD dwFlags = 0, dwkeylen = 0;
    HCRYPTKEY hcryptkey;
    ALG_ID alg_id;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ik|k:CryptGenKey", keywords,
            &alg_id,   // @pyparm int|Algid||Algorithm identifier, one of the CALG_* values, or
                       // AT_KEYEXCHANGE/AT_SIGNATURE
            &dwFlags,  // @pyparm int|Flags||Combination of
                       // CRYPT_CREATE_SALT,CRYPT_EXPORTABLE,CRYPT_NO_SALT,CRYPT_PREGEN,CRYPT_USER_PROTECTED,CRYPT_ARCHIVABLE
            &dwkeylen))  // @pyparm int|KeyLen|0|Length of key to generate, can be 0 to use provider's default key
                         // length
        return NULL;
    // uppermost 16 bits of flags are actually the length
    dwkeylen = dwkeylen << 16;
    dwFlags = dwFlags | dwkeylen;
    if (CryptGenKey(hc, alg_id, dwFlags, &hcryptkey))
        return new PyCRYPTKEY(hcryptkey, self);
    PyWin_SetAPIError("CryptGenKey", GetLastError());
    return NULL;
}

// @pymethod |PyCRYPTPROV|CryptGetProvParam|Retrieves specified attribute of provider
// @rdesc Type of returned object is dependent on the attribute requested
PyObject *PyCRYPTPROV::PyCryptGetProvParam(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Param", "Flags", NULL};
    PyObject *ret = NULL, *obdata = NULL;
    HCRYPTPROV hc = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    DWORD dwFlags = 0, dwParam = 0, dwDataLen = 0;
    DWORD err = 0;
    BYTE *pbData = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k|k:CryptGetProvParam", keywords,
            &dwParam,   // @pyparm int|Param||One of the PP_* values
            &dwFlags))  // @pyparm int|Flags|0|If param if PP_KEYSET_SEC_DESCR, can be a combination of
                        // OWNER_SECURITY_INFORMATION,GROUP_SECURITY_INFORMATION,DACL_SECURITY_INFORMATION,SACL_SECURITY_INFORMATION
        return NULL;

    switch (dwParam) {
        case PP_USE_HARDWARE_RNG:
            // for PP_USE_HARDWARE_RNG, flags must be zero and only return value is boolean result of function
            ret = PyBool_FromLong(CryptGetProvParam(hc, dwParam, NULL, 0, 0));
            break;
        case PP_ENUMCONTAINERS: {
            // first call returns max size of container name, does not change it again
            dwFlags = dwFlags | CRYPT_FIRST;
            if (!CryptGetProvParam(hc, dwParam, pbData, &dwDataLen, dwFlags)) {
                PyWin_SetAPIError("CryptGetProvParam", GetLastError());
                break;
            }
            pbData = (BYTE *)malloc(dwDataLen);
            if (pbData == NULL) {
                PyErr_Format(PyExc_MemoryError, "CryptGetProvParam: Unable to allocate %d bytes", dwDataLen);
                break;
            }
            ret = PyList_New(0);
            if (ret == NULL)
                break;
            while (CryptGetProvParam(hc, dwParam, pbData, &dwDataLen, dwFlags)) {
                obdata = PyWinCoreString_FromString((char *)pbData);
                if ((obdata == NULL) || (PyList_Append(ret, obdata) == -1)) {
                    Py_XDECREF(obdata);
                    break;
                }
                Py_DECREF(obdata);
                // remove first flag after first real call with a data buffer
                if (dwFlags & CRYPT_FIRST)
                    dwFlags = dwFlags ^ CRYPT_FIRST;
            }
            err = GetLastError();
            if (err != ERROR_NO_MORE_ITEMS) {
                Py_DECREF(ret);
                ret = NULL;
                if (!PyErr_Occurred())
                    PyWin_SetAPIError("CryptGetProvParam", err);
            }
            break;
        }
        case PP_ENUMALGS: {
            PROV_ENUMALGS prov_enumalgs;
            dwDataLen = sizeof(PROV_ENUMALGS);
            dwFlags = dwFlags | CRYPT_FIRST;
            ret = PyList_New(0);
            if (ret == NULL)
                break;
            while (CryptGetProvParam(hc, dwParam, (BYTE *)&prov_enumalgs, &dwDataLen, dwFlags)) {
                obdata = Py_BuildValue(
                    "{s:I,s:k,s:N}", "Algid", prov_enumalgs.aiAlgid, "BitLen", prov_enumalgs.dwBitLen,
                    // Name length include the terminating NULL
                    "Name", PyWinCoreString_FromString(prov_enumalgs.szName, prov_enumalgs.dwNameLen - 1));
                if ((obdata == NULL) || (PyList_Append(ret, obdata) == -1)) {
                    Py_XDECREF(obdata);
                    break;
                }
                Py_DECREF(obdata);
                // remove first flag after first real call with a data buffer
                if (dwFlags & CRYPT_FIRST)
                    dwFlags = dwFlags ^ CRYPT_FIRST;
            }
            err = GetLastError();
            if (err != ERROR_NO_MORE_ITEMS) {
                Py_DECREF(ret);
                ret = NULL;
                if (!PyErr_Occurred())
                    PyWin_SetAPIError("CryptGetProvParam", err);
            }
            break;
        }
        case PP_ENUMALGS_EX: {
            PROV_ENUMALGS_EX prov_enumalgs_ex;
            dwDataLen = sizeof(PROV_ENUMALGS_EX);
            dwFlags = dwFlags | CRYPT_FIRST;
            ret = PyList_New(0);
            if (ret == NULL)
                break;
            while (CryptGetProvParam(hc, dwParam, (BYTE *)&prov_enumalgs_ex, &dwDataLen, dwFlags)) {
                obdata = Py_BuildValue(
                    "{s:I,s:k,s:k,s:k,s:k,s:N,s:N}", "Algid", prov_enumalgs_ex.aiAlgid, "DefaultLen",
                    prov_enumalgs_ex.dwDefaultLen, "MinLen", prov_enumalgs_ex.dwMinLen, "MaxLen",
                    prov_enumalgs_ex.dwMaxLen, "Protocols", prov_enumalgs_ex.dwProtocols,
                    // Name lengths include the terminating NULL
                    "Name", PyWinCoreString_FromString(prov_enumalgs_ex.szName, prov_enumalgs_ex.dwNameLen - 1),
                    "LongName",
                    PyWinCoreString_FromString(prov_enumalgs_ex.szLongName, prov_enumalgs_ex.dwLongNameLen - 1));
                if ((obdata == NULL) || (PyList_Append(ret, obdata) == -1)) {
                    Py_XDECREF(obdata);
                    break;
                }
                Py_DECREF(obdata);
                if (dwFlags & CRYPT_FIRST)
                    dwFlags = dwFlags ^ CRYPT_FIRST;
            }
            err = GetLastError();
            if (err != ERROR_NO_MORE_ITEMS) {
                Py_DECREF(ret);
                ret = NULL;
                if (!PyErr_Occurred())
                    PyWin_SetAPIError("CryptGetProvParam", err);
            }
            break;
        }
        default: {
            if (!CryptGetProvParam(hc, dwParam, pbData, &dwDataLen, dwFlags)) {
                PyWin_SetAPIError("CryptGetProvParam");
                break;
            }
            pbData = (BYTE *)malloc(dwDataLen);
            if (pbData == NULL) {
                PyErr_Format(PyExc_MemoryError, "CryptGetProvParam: Unable to allocate %d bytes", dwDataLen);
                break;
            }
            if (!CryptGetProvParam(hc, dwParam, pbData, &dwDataLen, dwFlags)) {
                PyWin_SetAPIError("CryptGetProvParam");
                break;
            }
            switch (dwParam) {
                case PP_KEYSET_SEC_DESCR:
                    ret = PyWinObject_FromSECURITY_DESCRIPTOR(pbData);
                    break;
                case PP_KEYSPEC:
                case PP_KEYSTORAGE:
                case PP_SIG_KEYSIZE_INC:
                case PP_KEYX_KEYSIZE_INC:
                case PP_PROVTYPE:
                case PP_IMPTYPE:
                case PP_KEYSET_TYPE:
                case PP_SYM_KEYSIZE:
                    ret = Py_BuildValue("k", *pbData);
                    break;
                case PP_NAME:
                case PP_CONTAINER:
                case PP_UNIQUE_CONTAINER:
                case PP_KEYEXCHANGE_PIN:
                case PP_SIGNATURE_PIN:
                case PP_ADMIN_PIN:
                    ret = PyString_FromString((char *)pbData);
                    break;
                case PP_VERSION:  // return as string or tuple of 2 numbers ???????
                default: {
                    PyErr_SetString(PyExc_NotImplementedError,
                                    "The provider parameter specified is not yet implemented");
                    break;
                }
            }
        }
    }
    if (pbData != NULL)
        free(pbData);
    return ret;
}

// @pymethod <o PyCRYPTKEY>|PyCRYPTPROV|CryptGetUserKey|Returns a handle to one of user's key pairs
PyObject *PyCRYPTPROV::PyCryptGetUserKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeySpec", NULL};
    HCRYPTKEY hcryptkey = NULL;
    DWORD dwKeySpec = 0;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k", keywords,
                                     &dwKeySpec))  // @pyparm int|KeySpec||AT_KEYEXCHANGE or AT_SIGNATURE (some
                                                   // providers may implement extra key specs)
        return NULL;
    if (CryptGetUserKey(hcryptprov, dwKeySpec, &hcryptkey))
        return new PyCRYPTKEY(hcryptkey, self);
    PyWin_SetAPIError("PyCRYPTPROV::CryptGetUserKey");
    return NULL;
}

// @pymethod string|PyCRYPTPROV|CryptGenRandom|Generates random data of specified length
PyObject *PyCRYPTPROV::PyCryptGenRandom(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Len", "SeedData", NULL};
    PyObject *ret = NULL;
    DWORD dwLen = 0, seedlen = 0;
    BYTE *pbBuffer = NULL;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    char *seeddata = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k|z#", keywords,
                                     &dwLen,                // @pyparm int|Len||Number of bytes to generate
                                     &seeddata, &seedlen))  // @pyparm string|SeedData|None|Random seed data
        return NULL;
    pbBuffer = (BYTE *)malloc(dwLen + 1);
    if (pbBuffer == NULL)
        return PyErr_Format(PyExc_MemoryError, "CryptGenRandom: Unable to allocate %d bytes", dwLen + 1);

    // initialize buffer with char string if passed if
    ZeroMemory(pbBuffer, dwLen + 1);
    if (seeddata != NULL)
        memcpy(pbBuffer, seeddata, min(dwLen, seedlen));
    if (CryptGenRandom(hcryptprov, dwLen, pbBuffer))
        ret = PyString_FromStringAndSize((char *)pbBuffer, dwLen);
    else
        PyWin_SetAPIError("PyCRYPTPROV::CryptGenRandom");
    if (pbBuffer != NULL)
        free(pbBuffer);
    return ret;
}

// @pymethod <o PyCRYPTHASH>|PyCRYPTPROV|CryptCreateHash|Creates a hash object for hashing large amounts of data
PyObject *PyCRYPTPROV::PyCryptCreateHash(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Algid", "Key", "Flags", NULL};
    PyObject *ret = NULL;
    DWORD dwFlags = 0;
    ALG_ID alg_id = 0;
    PyObject *obhcryptkey = Py_None;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    HCRYPTKEY hcryptkey;
    HCRYPTHASH hcrypthash;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I|Ok", keywords,
                                     &alg_id,       // @pyparm int|Algid||An algorithm identifier, CALG_*.
                                     &obhcryptkey,  // @pyparm <o PyCRYPTKEY>|Key|None|Used only for keyed hashes (MAC
                                                    // or HMAC), use None otherwise
                                     &dwFlags))     // @pyparm int|Flags|0|Reserved, use 0 if passed in
        return NULL;
    if (!PyWinObject_AsHCRYPTKEY(obhcryptkey, &hcryptkey, TRUE))
        return NULL;
    if (CryptCreateHash(hcryptprov, alg_id, hcryptkey, dwFlags, &hcrypthash))
        ret = new PyCRYPTHASH(hcrypthash);
    else
        PyWin_SetAPIError("PyCRYPTPROV::CryptCreateHash");
    return ret;
}

// @pymethod <o PyCRYPTKEY>|PyCRYPTPROV|CryptImportKey|Imports a key exported by <om PyCRYPTKEY::CryptExportKey>
PyObject *PyCRYPTPROV::PyCryptImportKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Data", "PubKey", "Flags", NULL};
    PBYTE buf;
    DWORD buflen, flags = 0;
    HCRYPTKEY retkey = NULL, pubkey = NULL;
    PyObject *obpubkey = Py_None, *obbuf;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|Ok", keywords,
                                     &obbuf,     // @pyparm buffer|Data||The key blob to be imported
                                     &obpubkey,  // @pyparm <o PyCRYPTKEY>|PubKey|None|Key to be used to decrypt the
                                                 // blob, not used for importing public keys
                                     &flags))    // @pyparm int|Flags|0|Combination of CRYPT_EXPORTABLE, CRYPT_OAEP,
                                                 // CRYPT_NO_SALT, CRYPT_USER_PROTECTED
        return NULL;
    if (!PyWinObject_AsHCRYPTKEY(obpubkey, &pubkey, TRUE))
        return NULL;
    if (!PyWinObject_AsReadBuffer(obbuf, (void **)&buf, &buflen, FALSE))
        return NULL;
    if (!CryptImportKey(hcryptprov, buf, buflen, pubkey, flags, &retkey))
        return PyWin_SetAPIError("PyCRYPTPROV::CryptImportKey");
    return new PyCRYPTKEY(retkey, self);
}

// @pymethod <o PyCERT_PUBLIC_KEY_INFO>|PyCRYPTPROV|CryptExportPublicKeyInfo|Exports a public key to send to other users
// Returned dict can be serialized for sending to another python application using pickle.dump
PyObject *PyCRYPTPROV::PyCryptExportPublicKeyInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeySpec", "CertEncodingType", NULL};
    PCERT_PUBLIC_KEY_INFO buf = NULL;
    DWORD keyspec, encoding = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, buflen = 0;
    PyObject *ret = NULL;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k|k:CryptExportPublicKeyInfo", keywords,
                                     &keyspec,    // @pyparm int|KeySpec||AT_KEYEXCHANGE or AT_SIGNATURE
                                     &encoding))  // @pyparm int|CertEncodingType|X509_ASN_ENCODING combined with
                                                  // PKCS_7_ASN_ENCODING|Specifies encoding for exported key info
        return NULL;
    if (!CryptExportPublicKeyInfo(hcryptprov, keyspec, encoding, buf, &buflen))
        return PyWin_SetAPIError("CryptExportPublicKeyInfo");
    buf = (PCERT_PUBLIC_KEY_INFO)malloc(buflen);
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "CryptExportPublicKeyInfo: Unable to allocate %d bytes", buflen);
    if (!CryptExportPublicKeyInfo(hcryptprov, keyspec, encoding, buf, &buflen))
        PyWin_SetAPIError("CryptExportPublicKeyInfo");
    else
        ret = PyWinObject_FromCERT_PUBLIC_KEY_INFO(buf);
    free(buf);
    return ret;
}

// @pymethod <o PyCRYPTKEY>|PyCRYPTPROV|CryptImportPublicKeyInfo|Imports another user's public key
PyObject *PyCRYPTPROV::PyCryptImportPublicKeyInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Info", "CertEncodingType", NULL};
    CERT_PUBLIC_KEY_INFO buf;
    HCRYPTKEY hcryptkey = NULL;
    DWORD encoding = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    HCRYPTPROV hcryptprov = ((PyCRYPTPROV *)self)->GetHCRYPTPROV();
    PyObject *obinfo;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CryptImportPublicKeyInfo", keywords,
                                     &obinfo,  // @pyparm dict|Info||<o PyCERT_PUBLIC_KEY_INFO> dictionary as returned
                                               // by <om PyCRYPTPROV::CryptExportPublicKeyInfo>
                                     &encoding))  // @pyparm int|CertEncodingType|X509_ASN_ENCODING combined with
                                                  // PKCS_7_ASN_ENCODING|Specifies encoding for exported key info
        return NULL;
    if (!PyWinObject_AsCERT_PUBLIC_KEY_INFO(obinfo, &buf))
        return NULL;
    if (!CryptImportPublicKeyInfo(hcryptprov, encoding, &buf, &hcryptkey))
        return PyWin_SetAPIError("CryptImportPublicKeyInfo");
    return new PyCRYPTKEY(hcryptkey, self);
}
