#define PY_SSIZE_T_CLEAN
#include "win32crypt.h"
// @doc
extern PyObject *dummy_tuple;  // = PyTuple_New(0); // set up in win32cryptmodule init

BOOL PyWinObject_AsDATA_BLOB(PyObject *ob, DATA_BLOB *b)
{
    PyWinBufferView pybuf(ob);
    if (!pybuf.ok())
        return FALSE;
    // note: this might be unsafe, as we give away the buffer pointer to a
    // client outside of the scope where our RAII object 'pybuf' resides.
    b->pbData = (BYTE *)pybuf.ptr();
    b->cbData = PyWin_SAFE_DOWNCAST(pybuf.len(), Py_ssize_t, int);
    return TRUE;
}

PyObject *PyWinObject_FromDATA_BLOB(DATA_BLOB *b) { return PyBytes_FromStringAndSize((char *)b->pbData, b->cbData); }

// @object PyCRYPTPROTECT_PROMPTSTRUCT|A tuple representing a CRYPTPROTECT_PROMPTSTRUCT structure
// @tupleitem 0|int|flags|Combination of CRYPTPROTECT_PROMPT_* flags
// @tupleitem 1|int|hwndApp|parent hwnd (default is 0)
// @tupleitem 2|<o PyUnicode>|prompt|A prompt string (default is None)
BOOL PyWinObject_AsCRYPTPROTECT_PROMPTSTRUCT(PyObject *ob, CRYPTPROTECT_PROMPTSTRUCT *PromptStruct)
{
    memset(PromptStruct, 0, sizeof(CRYPTPROTECT_PROMPTSTRUCT));
    PromptStruct->cbSize = sizeof(CRYPTPROTECT_PROMPTSTRUCT);
    if (!PyTuple_Check(ob)) {
        PyErr_Format(PyExc_TypeError, "CRYPTPROTECT_PROMPTSTRUCT must be None or a tuple (got %s)",
                     Py_TYPE(ob)->tp_name);
        return FALSE;
    }
    PyObject *obPrompt = Py_None;

    if (!PyArg_ParseTuple(ob, "k|O&O", &PromptStruct->dwPromptFlags, PyWinObject_AsHANDLE, &PromptStruct->hwndApp,
                          &obPrompt))
        return FALSE;
    return PyWinObject_AsWCHAR(obPrompt, (WCHAR **)(&PromptStruct->szPrompt), TRUE);
}

BOOL PyWinObject_AsPCERT_SYSTEM_STORE_RELOCATE_PARA(PyObject *obpvPara, PCERT_SYSTEM_STORE_RELOCATE_PARA pcssrp)
{
    BOOL bSuccess = TRUE;
    PyObject *seq_item = NULL;
    char *format_msg = "pvPara must be represented as a sequence of (PyHKEY, string/unicode)";
    if (!PySequence_Check(obpvPara)) {
        PyErr_SetString(PyExc_TypeError, format_msg);
        return FALSE;
    }
    if (PySequence_Length(obpvPara) != 2) {
        PyErr_SetString(PyExc_TypeError, format_msg);
        return FALSE;
    }
    seq_item = PySequence_GetItem(obpvPara, 0);
    bSuccess = PyWinObject_AsHKEY(seq_item, &pcssrp->hKeyBase);
    Py_DECREF(seq_item);
    if (bSuccess) {
        seq_item = PySequence_GetItem(obpvPara, 1);
        bSuccess = PyWinObject_AsWCHAR(seq_item, (WCHAR **)&pcssrp->pwszSystemStore, FALSE);
        Py_DECREF(seq_item);
    }
    return bSuccess;
}

PyObject *PyWinObject_FromCRYPT_INTEGER_BLOB(PCRYPT_INTEGER_BLOB pcib)
{
    // CRYPT_INTEGER_BLOB contains an array of bytes in little-endian format
    /*
    DWORD byte_index;
    int bytes_written;
    char *hex_string, *curr_pos;
    hex_string=(char *)malloc((pcib->cbData*2)+1);
    if (hex_string==NULL)
        return PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", (pcib->cbData*2)+1);
    curr_pos=hex_string;
    for (byte_index=0;byte_index<pcib->cbData;byte_index++){
        bytes_written=sprintf(curr_pos, "%.2X", pcib->pbData[(pcib->cbData-1) - byte_index]);
        curr_pos+=2;
        }
    return PyLong_FromString(hex_string, NULL, 16);
    */
    return PyBytes_FromStringAndSize((char *)pcib->pbData, pcib->cbData);
}

PyObject *PyWinObject_FromCRYPT_KEY_PROV_INFO(PCRYPT_KEY_PROV_INFO pckpi)
{
    // CRYPT_KEY_PROV_INFO.rgProvParam is an array of CRYPT_KEY_PROV_PARAM structs
    PyObject *obProvParam = PyTuple_New(pckpi->cProvParam);
    if (obProvParam == NULL)
        return NULL;
    for (DWORD i = 0; i < pckpi->cProvParam; i++) {
        PyObject *kpp, *data;
        /* This is a small subset of PP_* types returned by CryptGetProvParam.  According
            to MSDN, should only hold values that can be set using CryptSetProvParam */
        switch (pckpi->rgProvParam[i].dwParam) {
            case PP_CLIENT_HWND:
                data = PyWinObject_FromHANDLE((HANDLE *)pckpi->rgProvParam[i].pbData);
                break;
            case PP_USE_HARDWARE_RNG:
                data = PyBool_FromLong(*((BOOL *)pckpi->rgProvParam[i].pbData));
                break;
            case PP_KEYSET_SEC_DESCR:
                data = PyWinObject_FromSECURITY_DESCRIPTOR((PSECURITY_DESCRIPTOR)pckpi->rgProvParam[i].pbData);
                break;
            case PP_KEYEXCHANGE_PIN:
            case PP_SIGNATURE_PIN:
                // both return pin as NULL-terminated string
                data = PyBytes_FromString((char *)pckpi->rgProvParam[i].pbData);
                break;
            default:
                // Anything not handled specifically is dumped out as raw bytes
                PyErr_Warn(PyExc_RuntimeWarning, "Unsupported PP_ parameter returned as raw data"),
                    data =
                        PyBytes_FromStringAndSize((char *)pckpi->rgProvParam[i].pbData, pckpi->rgProvParam[i].cbData);
                break;
        }
        if (data == NULL) {
            Py_DECREF(obProvParam);
            return NULL;
        }
        kpp = Py_BuildValue("{s:k, s:k, s:N}", "Param", pckpi->rgProvParam[i].dwParam, "Flags",
                            pckpi->rgProvParam[i].dwFlags, "Data", data);
        if (kpp == NULL) {
            Py_DECREF(obProvParam);
            Py_DECREF(data);
            return NULL;
        }
        PyTuple_SET_ITEM(obProvParam, i, kpp);
    }
    return Py_BuildValue("{s:u, s:u, s:k, s:k, s:k, s:N}", "ContainerName", pckpi->pwszContainerName, "ProvName",
                         pckpi->pwszProvName, "ProvType", pckpi->dwProvType, "Flags", pckpi->dwFlags, "KeySpec",
                         pckpi->dwKeySpec, "ProvParam", obProvParam);
}

// @object PyCERT_OTHER_NAME|Dict containing {ObjId, Value}.
// ObjId is one of the string object id's identifying the type of name.
// Value is a binary string containing an encoded CERT_NAME_VALUE that can be decoded
// using X509_UNICODE_NAME_VALUE to return the actual unicode string
PyObject *PyWinObject_FromCERT_OTHER_NAME(PCERT_OTHER_NAME pcon)
{
    /* CERT_OTHER_NAME - struct isn't in the documentation anywhere
    from wincrypt.h:
        typedef struct _CERT_OTHER_NAME {LPSTR pszObjId; CRYPT_OBJID_BLOB Value;} CERT_OTHER_NAME
        Value blob is an encoded CERT_NAME_VALUE (according to a post in microsoft.public.platformsdk.security)
        - to be decoded with X509_UNICODE_NAME_VALUE
    */
    return Py_BuildValue("{s:s,s:N}", "ObjId", pcon->pszObjId, "Value",
                         PyBytes_FromStringAndSize((char *)pcon->Value.pbData, pcon->Value.cbData));
}

// @object PyCERT_ALT_NAME_ENTRY|Represented as a 2-tuple
// @comm First item is one of the CERT_ALT_NAME_* constants indicating the type.
// <nl>Second item is either a string, or for CERT_ALT_NAME_OTHER_NAME a <o PyCERT_OTHER_NAME>
PyObject *PyWinObject_FromCERT_ALT_NAME_ENTRY(PCERT_ALT_NAME_ENTRY pcane)
{
    switch (pcane->dwAltNameChoice) {
        // first 4 are all WCHAR pointers
        case CERT_ALT_NAME_RFC822_NAME:
        case CERT_ALT_NAME_DNS_NAME:
        case CERT_ALT_NAME_URL:
        case CERT_ALT_NAME_EDI_PARTY_NAME:
            return Py_BuildValue("kN", pcane->dwAltNameChoice, PyWinObject_FromWCHAR(pcane->pwszRfc822Name));
        case CERT_ALT_NAME_REGISTERED_ID:
            return Py_BuildValue("kN", pcane->dwAltNameChoice, PyBytes_FromString(pcane->pszRegisteredID));
        // these 3 all resolve to a CRYPTOAPI_BLOB
        case CERT_ALT_NAME_IP_ADDRESS:
        case CERT_ALT_NAME_X400_ADDRESS:
        case CERT_ALT_NAME_DIRECTORY_NAME:
            return Py_BuildValue("kN", pcane->dwAltNameChoice,
                                 PyBytes_FromStringAndSize((char *)pcane->IPAddress.pbData, pcane->IPAddress.cbData));
        case CERT_ALT_NAME_OTHER_NAME:
            // pOtherName points to a CERT_OTHER_NAME
            return Py_BuildValue("kN", pcane->dwAltNameChoice, PyWinObject_FromCERT_OTHER_NAME(pcane->pOtherName));
        default:
            return PyErr_Format(PyExc_NotImplementedError, "CERT_ALT_NAME_ENTRY %d is not yet supported",
                                pcane->dwAltNameChoice);
    }
}

// @object PyCERT_ALT_NAME_INFO|Sequence of <o PyCERT_ALT_NAME_ENTRY> objects
PyObject *PyWinObject_FromCERT_ALT_NAME_INFO(PCERT_ALT_NAME_INFO pcani)
{
    PyObject *ret = PyTuple_New(pcani->cAltEntry);
    if (ret == NULL)
        return NULL;
    for (DWORD i = 0; i < pcani->cAltEntry; i++) {
        PyObject *ret_item = PyWinObject_FromCERT_ALT_NAME_ENTRY(&pcani->rgAltEntry[i]);
        if (ret_item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, i, ret_item);
    }
    return ret;
}

PyObject *PyWinObject_FromCRYPT_ALGORITHM_IDENTIFIER(PCRYPT_ALGORITHM_IDENTIFIER pcai)
{
    /*
    ???? to do: Call CryptDecodeObject to decode 	pcai->Parameters if ObjId is one of the following:
        szOID_OIWSEC_dsa		X509_DSS_PARAMETERS
        szOID_RSA_RC2CBC		PKCS_RC2_CBC_PARAMETERS
        szOID_OIWSEC_desCBC		X509_OCTET_STRING
        szOID_RSA_DES_EDE3_CBC	X509_OCTET_STRING
        szOID_RSA_RC4			X509_OCTET_STRING
    ????
    void *buf=NULL;
    DWORD bufsize=0, ret_bufsize=0, err;
    if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RC2_CBC_PARAMETERS,
        pcai->Parameters.pbData, pcai->Parameters.cbData,
        CRYPT_DECODE_ALLOC_FLAG|CRYPT_DECODE_NOCOPY_FLAG,
        NULL, &buf, &ret_bufsize))
        err=GetLastError();
    */
    return Py_BuildValue("{s:s, s:N}", "ObjId", pcai->pszObjId, "Parameters",
                         PyBytes_FromStringAndSize((char *)pcai->Parameters.pbData, pcai->Parameters.cbData));
}

// @object PyCRYPT_ALGORITHM_IDENTIFIER|Dictionary containing information that identifies an encryption
//  algorithm and any extra parameters it requires
BOOL PyWinObject_AsCRYPT_ALGORITHM_IDENTIFIER(PyObject *obcai, PCRYPT_ALGORITHM_IDENTIFIER pcai)
{
    static char *cai_keys[] = {"ObjId", "Parameters", NULL};
    if (!PyDict_Check(obcai)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct a CRYPT_ALGORITHM_IDENTIFIER must be a dict");
        return FALSE;
    }
    ZeroMemory(pcai, sizeof(CRYPT_ALGORITHM_IDENTIFIER));
    Py_ssize_t cbData;
    BOOL ok =
        PyArg_ParseTupleAndKeywords(dummy_tuple, obcai, "sz#:CRYPT_ALGORITHM_IDENTIFIER", cai_keys,
                                    &pcai->pszObjId,  // @prop str|ObjId|An szOID_* string identifying the algorithm
                                    &pcai->Parameters.pbData,
                                    &cbData);  // @prop str|Parameters|Blob of binary data containing encoded parameters
    if (ok)
        pcai->Parameters.cbData = (DWORD)cbData;
    return ok;
}

PyObject *PyWinObject_FromCERT_PUBLIC_KEY_INFO(PCERT_PUBLIC_KEY_INFO pcpki)
{
    return Py_BuildValue("{s:N, s:N}", "Algorithm", PyWinObject_FromCRYPT_ALGORITHM_IDENTIFIER(&pcpki->Algorithm),
                         "PublicKey", PyWinObject_FromCRYPT_BIT_BLOB(&pcpki->PublicKey));
}

// @object PyCERT_PUBLIC_KEY_INFO|Dict containing an exported public key
// @prop <o PyCRYPT_ALGORITHM_IDENTIFIER>|Algorithm|Dict containing OID of the public key algorithm
// @prop <o PyCRYPT_BIT_BLOB>|PublicKey|Dict containing the encoded public key
BOOL PyWinObject_AsCERT_PUBLIC_KEY_INFO(PyObject *obcpki, PCERT_PUBLIC_KEY_INFO pcpki)
{
    static char *cpki_keys[] = {"Algorithm", "PublicKey", NULL};
    if (!PyDict_Check(obcpki)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct a CERT_PUBLIC_KEY_INFO must be a dict");
        return FALSE;
    }
    ZeroMemory(pcpki, sizeof(CERT_PUBLIC_KEY_INFO));
    return PyArg_ParseTupleAndKeywords(dummy_tuple, obcpki, "O&O&:CERT_PUBLIC_KEY_INFO", cpki_keys,
                                       PyWinObject_AsCRYPT_ALGORITHM_IDENTIFIER, &pcpki->Algorithm,
                                       PyWinObject_AsCRYPT_BIT_BLOB, &pcpki->PublicKey);
}

PyObject *PyWinObject_FromCRYPT_BIT_BLOB(PCRYPT_BIT_BLOB pcbb)
{
    return Py_BuildValue("{s:N,s:k}", "Data", PyBytes_FromStringAndSize((char *)pcbb->pbData, pcbb->cbData),
                         "UnusedBits", pcbb->cUnusedBits);
}

// @object PyCRYPT_BIT_BLOB|Dict containing raw data of a certain bit length
BOOL PyWinObject_AsCRYPT_BIT_BLOB(PyObject *obcbb, PCRYPT_BIT_BLOB pcbb)
{
    static char *cbb_keys[] = {"Data", "UnusedBits", NULL};
    PyObject *obdata;
    if (!PyDict_Check(obcbb)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct a CRYPT_BIT_BLOB must be a dict");
        return FALSE;
    }
    ZeroMemory(pcbb, sizeof(CRYPT_BIT_BLOB));
    PyWinBufferView pybuf;
    if (PyArg_ParseTupleAndKeywords(
            dummy_tuple, obcbb, "Ok:CRYPT_BIT_BLOB", cbb_keys,
            &obdata,             // @prop buffer|Data|Binary data
            &pcbb->cUnusedBits)  // @prop int|UnusedBits|Nbr of bits of last byte that are unused
    )
        if (pybuf.init(obdata)) {
            // note: this might be unsafe, as we give away the buffer pointer to a
            // client outside of the scope where our RAII object 'pybuf' resides.
            pcbb->pbData = (BYTE *)pybuf.ptr();
            pcbb->cbData = pybuf.len();
            return TRUE;
        }
    return FALSE;
}

// @object PyCERT_NAME_VALUE|Dict containing type (CERT_RDN_*) and a unicode string
PyObject *PyWinObject_FromCERT_NAME_VALUE(PCERT_NAME_VALUE pcnv)
{
    /* ???? Need some additional interpretation here, some of the CERT_RDN_* values can mean 8-bit characters
        or even an array of 32-bit ints */
    PyObject *ret = Py_BuildValue("{s:k,s:u#}", "ValueType", pcnv->dwValueType, "Value", pcnv->Value.pbData,
                                  (Py_ssize_t)(pcnv->Value.cbData / sizeof(WCHAR)));
    return ret;
}

PyObject *PyWinObject_FromCERT_RDN_ATTR(PCERT_RDN_ATTR pcra)
{
    /* Data contained in Value can be a number of different types based on ValueType.
        ValueType also includes flags, so cannot use == for comparison
        Should also do same conversions in PyWinObject_FromCERT_NAME_VALUE */
    PyObject *value;
    if ((pcra->dwValueType & CERT_RDN_BMP_STRING) || (pcra->dwValueType & CERT_RDN_UNICODE_STRING))
        value = PyWinObject_FromWCHAR((WCHAR *)pcra->Value.pbData, pcra->Value.cbData / sizeof(WCHAR));
    else if (pcra->dwValueType & CERT_RDN_UTF8_STRING)
        value = PyUnicode_DecodeUTF8((char *)pcra->Value.pbData, pcra->Value.cbData, NULL);
    else if ((pcra->dwValueType & CERT_RDN_INT4_STRING) || (pcra->dwValueType & CERT_RDN_UNIVERSAL_STRING)) {
        // data is an array of 32-bit ints
        int nbr_elements = pcra->Value.cbData / sizeof(INT32);
        value = PyTuple_New(nbr_elements);
        if (value != NULL) {
            for (int i = 0; i < nbr_elements; i++) {
                PyObject *item = PyLong_FromLong(((INT32 *)pcra->Value.pbData)[i]);
                if (item == NULL) {
                    Py_DECREF(value);
                    value = NULL;
                    break;
                }
                PyTuple_SET_ITEM(value, i, item);
            }
        }
    }
    else  // all others treated as raw bytes
        value = PyBytes_FromStringAndSize((char *)pcra->Value.pbData, pcra->Value.cbData);

    if (value == NULL)
        return NULL;
    return Py_BuildValue("{s:s, s:k, s:N}", "ObjId", pcra->pszObjId, "ValueType", pcra->dwValueType, "Value", value);
}

PyObject *PyWinObject_FromCERT_RDN(PCERT_RDN pcr)
{
    PyObject *ret = PyTuple_New(pcr->cRDNAttr);
    if (ret == NULL)
        return NULL;
    for (DWORD i = 0; i < pcr->cRDNAttr; i++) {
        PyObject *ret_item = PyWinObject_FromCERT_RDN_ATTR(&pcr->rgRDNAttr[i]);
        if (ret_item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, i, ret_item);
    }
    return ret;
}

// @object PyCERT_NAME_INFO|Sequence of CERT_RDN's
PyObject *PyWinObject_FromCERT_NAME_INFO(PCERT_NAME_INFO pcni)
{
    PyObject *ret = PyTuple_New(pcni->cRDN);
    if (ret == NULL)
        return NULL;
    for (DWORD i = 0; i < pcni->cRDN; i++) {
        PyObject *ret_item = PyWinObject_FromCERT_RDN(&pcni->rgRDN[i]);
        if (ret_item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, i, ret_item);
    }
    return ret;
}

PyObject *PyWinObject_FromCRYPT_OID_INFO(PCCRYPT_OID_INFO oid_info)
{
    return Py_BuildValue("{s:s,s:u,s:k,s:k,s:N}", "OID", oid_info->pszOID, "Name", oid_info->pwszName, "GroupId",
                         oid_info->dwGroupId, "Value", oid_info->dwValue,  // this is union, but all same size integers
                         "ExtraInfo",
                         PyBytes_FromStringAndSize((char *)oid_info->ExtraInfo.pbData, oid_info->ExtraInfo.cbData));
}

void PyWinObject_FreeCRYPT_DECRYPT_MESSAGE_PARA(PCRYPT_DECRYPT_MESSAGE_PARA pcdmp)
{
    if (pcdmp->rghCertStore != NULL) {
        for (DWORD i = 0; i < pcdmp->cCertStore; i++) {
            if (pcdmp->rghCertStore[i] != NULL) {
                CertCloseStore(pcdmp->rghCertStore[i], 0);
                pcdmp->rghCertStore[i] = NULL;
            }
        }
        free(pcdmp->rghCertStore);
    }
    ZeroMemory(pcdmp, sizeof(*pcdmp));
}

// @object PyCRYPT_DECRYPT_MESSAGE_PARA|Dict containing message decryption parameters,
//	used with <om cryptoapi.CryptDecodeMessage> and <om cryptoapi.CryptDecryptMessage>
// @prop (<o PyCERT_STORE>,...)|CertStores|Sequence of certificate stores to be searched for a certificate
//		with a private key that can be used to decrypt the message
// @prop int|MsgAndCertEncodingType|Encoding types, optional. Defaults to X509_ASN_ENCODING combined with
// PKCS_7_ASN_ENCODING
// @prop int|Flags|Optional.  CRYPT_MESSAGE_SILENT_KEYSET_FLAG can be used to suppress any dialogs that might be
// triggered by
//	accessing a key container, such as a request for a PIN.
BOOL PyWinObject_AsCRYPT_DECRYPT_MESSAGE_PARA(PyObject *obcdmp, PCRYPT_DECRYPT_MESSAGE_PARA pcdmp)
{
    static char *cdmp_keys[] = {"CertStores", "MsgAndCertEncodingType", "Flags", NULL};
    PyObject *obcertstores, *obcertstore;
    DWORD store_ind;
    ZeroMemory(pcdmp, sizeof(*pcdmp));
    pcdmp->cbSize = sizeof(*pcdmp);
    pcdmp->dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

    if (!PyDict_Check(obcdmp)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct a CRYPT_DECRYPT_MESSAGE_PARA must be a dict");
        return FALSE;
    }
    if (!PyArg_ParseTupleAndKeywords(dummy_tuple, obcdmp, "O|kk:CRYPT_DECRYPT_MESSAGE_PARA", cdmp_keys, &obcertstores,
                                     &pcdmp->dwMsgAndCertEncodingType, &pcdmp->dwFlags))
        return FALSE;
    TmpPyObject tuple_certstores = PyWinSequence_Tuple(obcertstores, &pcdmp->cCertStore);
    if (tuple_certstores == NULL)
        return FALSE;

    pcdmp->rghCertStore = (HCERTSTORE *)malloc(pcdmp->cCertStore * sizeof(HCERTSTORE));
    if (pcdmp->rghCertStore == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    ZeroMemory(pcdmp->rghCertStore, pcdmp->cCertStore * sizeof(HCERTSTORE));
    for (store_ind = 0; store_ind < pcdmp->cCertStore; store_ind++) {
        obcertstore = PyTuple_GET_ITEM((PyObject *)tuple_certstores, store_ind);
        if (!PyWinObject_AsCERTSTORE(obcertstore, &pcdmp->rghCertStore[store_ind], FALSE)) {
            PyWinObject_FreeCRYPT_DECRYPT_MESSAGE_PARA(pcdmp);
            return FALSE;
        }
        // This should not fail, just increments a reference count
        pcdmp->rghCertStore[store_ind] = CertDuplicateStore(pcdmp->rghCertStore[store_ind]);
    }
    return TRUE;
}

// @object PyCRYPT_ENCRYPT_MESSAGE_PARA|Dictionary of encryption parameters used with <om cryptoapi.CryptEncryptMessage>
BOOL PyWinObject_AsCRYPT_ENCRYPT_MESSAGE_PARA(PyObject *obcemp, PCRYPT_ENCRYPT_MESSAGE_PARA pcemp)
{
    static char *cemp_keys[] = {"ContentEncryptionAlgorithm",
                                "CryptProv",
                                "EncryptionAuxInfo",
                                "Flags",
                                "InnerContentType",
                                "MsgEncodingType",
                                NULL};
    PyObject *obcryptprov = Py_None, *obcai, *obauxinfo = Py_None;
    ZeroMemory(pcemp, sizeof(CRYPT_ENCRYPT_MESSAGE_PARA));
    pcemp->cbSize = sizeof(CRYPT_ENCRYPT_MESSAGE_PARA);
    pcemp->dwMsgEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

    if (!PyDict_Check(obcemp)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct a CRYPT_ENCRYPT_MESSAGE_PARA must be a dict");
        return FALSE;
    }
    if (!PyArg_ParseTupleAndKeywords(
            dummy_tuple, obcemp, "O|OOkkk:CRYPT_DECRYPT_MESSAGE_PARA", cemp_keys,
            &obcai,  // @prop <o PyCRYPT_ALGORITHM_IDENTIFIER>|ContentEncryptionAlgorithm|Identifies the algorithm to be
                     // used
            &obcryptprov,  // @prop <o PyCRYPTPROV>|CryptProv|Optional. Handle to provider that will perform encryption,
                           // can be None for default provider
            &obauxinfo,    // @prop object|EncryptionAuxInfo|Optional. Extra info required by some CSP's.  Not supported
                           // yet, use only None
            &pcemp->dwFlags,             // @prop int|Flags|Optional.  Combination of CRYPT_MESSAGE_*_FLAG constants
            &pcemp->dwInnerContentType,  // @prop int|InnerContentType|Optional.  Only used if message to be encrypted
                                         // is already encoded
            &pcemp->dwMsgEncodingType))  // @prop int|MsgEncodingType|Optional.  Defaults to X509_ASN_ENCODING combined
                                         // with PKCS_7_ASN_ENCODING
        return FALSE;
    if (obauxinfo != Py_None) {
        PyErr_SetString(PyExc_ValueError, "EncryptionAuxInfo must be None");
        return FALSE;
    }
    if (!PyWinObject_AsHCRYPTPROV(obcryptprov, &pcemp->hCryptProv, TRUE))
        return FALSE;
    if (!PyWinObject_AsCRYPT_ALGORITHM_IDENTIFIER(obcai, &pcemp->ContentEncryptionAlgorithm))
        return FALSE;
    return TRUE;
}

// @object PyGetSignerCertificate|Callback used with CRYPT_VERIFY_MESSAGE_PARA to locate a certficate by issuer and
// serial nbr. This function will receive 4 args:
//	1. Arbitrary context object given as GetArg in <o CRYPT_VERIFY_MESSAGE_PARA>
//	2. CertEncodingType (int) -  specifies the type of encoding used
//  3. SignerId - Dict containing issuer and serial nbr that uniquely identifies a certificate
//  4. <o PyCERTSTORE> containing certificates extracted from the message
//	Function must return a <o PyCERT_CONTEXT>.  If no certificate could be found, it should raise
//	pywintypes.error(winerror.CRYPT_E_NO_MATCH)
//  If this function is not specified, the default action is to locate a certificate encoded in the message.
PCCERT_CONTEXT WINAPI PyLocateCertificate(void *pvGetArg, DWORD dwCertEncodingType, PCERT_INFO pSignerId,
                                          HCERTSTORE hMsgCertStore)
{
    CEnterLeavePython _celp;
    PCCERT_CONTEXT ret = NULL;
    PyObject *callback_function = ((PyObject **)pvGetArg)[0];
    PyObject *callback_arg = ((PyObject **)pvGetArg)[1];

    {  // Block to ensure that TmpPyObjects are destroyed before _celp (ie while still hold GIL)
        TmpPyObject args = Py_BuildValue(
            "OkNN", callback_arg, dwCertEncodingType,
            Py_BuildValue("{s:N, s:N}", "Issuer",
                          PyBytes_FromStringAndSize((char *)pSignerId->Issuer.pbData, pSignerId->Issuer.cbData),
                          "SerialNumber", PyWinObject_FromCRYPT_INTEGER_BLOB(&pSignerId->SerialNumber)),
            PyWinObject_FromCERTSTORE(hMsgCertStore));
        if (args == NULL)
            return NULL;
        // Increment the refcount of the store so it isn't closed when args are DECREF'ed.
        CertDuplicateStore(hMsgCertStore);
        TmpPyObject obret = PyObject_Call(callback_function, args, NULL);
        if (obret == NULL)
            return NULL;

        if (!PyWinObject_AsCERT_CONTEXT(obret, &ret, FALSE))
            return NULL;
        // Increment the refcount so cert isn't freed when obret is DECREF'ed.
        CertDuplicateCertificateContext(ret);
    }
    return ret;
}

// @object PyCRYPT_VERIFY_MESSAGE_PARA|Dict of verification parameters to be used with <om cryptoapi.CryptDecodeMessage>
//	or <om cryptoapi.CryptVerifyMessageSignature>.  All parameters are optional.  Can be either an empty dict or None
//	to use all defaults.
BOOL PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(PyObject *obcvmp, PCRYPT_VERIFY_MESSAGE_PARA pcvmp)
{
    static char *cvmp_keys[] = {"MsgAndCertEncodingType", "CryptProv", "GetSignerCertificate", "GetArg", NULL};
    PyObject *obhcryptprov = Py_None;
    ZeroMemory(pcvmp, sizeof(*pcvmp));
    pcvmp->cbSize = sizeof(*pcvmp);
    pcvmp->dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

    // Python function and arbitrary arg are passed as a 2-tuple to the C callback function
    PyObject *callback_function = Py_None, *callback_arg = Py_None;
    if (obcvmp == Py_None)
        return TRUE;
    if (!PyDict_Check(obcvmp)) {
        PyErr_SetString(PyExc_TypeError,
                        "Object used to construct CRYPT_VERIFY_MESSAGE_PARA structure must be a dict or None");
        return FALSE;
    }
    if (!PyArg_ParseTupleAndKeywords(
            dummy_tuple, obcvmp, "|kOOO:CRYPT_VERIFY_MESSAGE_PARA", cvmp_keys,
            &pcvmp->dwMsgAndCertEncodingType,  // @prop int|MsgAndCertEncodingType|Encoding types, defaults to
                                               // X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING
            &obhcryptprov,  // @prop <o PyCRYPTPROV>|CryptProv|CSP to be used to verify signature. Use None for default
                            // provider.
            &callback_function,  // @prop function|<o PyGetSignerCertificate>|Callback function that locates signer's
                                 // certificate.
            &callback_arg))      // @prop object|GetArg|Argument to be passed to above function, can be any object.
        return FALSE;
    if (!PyWinObject_AsHCRYPTPROV(obhcryptprov, &pcvmp->hCryptProv, TRUE))
        return FALSE;
    if (callback_function != Py_None) {
        PyObject **callback_objects = (PyObject **)malloc(2 * sizeof(PyObject *));
        if (callback_objects == NULL) {
            PyErr_NoMemory();
            return FALSE;
        }
        callback_objects[0] = callback_function;
        callback_objects[1] = callback_arg;
        pcvmp->pfnGetSignerCertificate = PyLocateCertificate;
        pcvmp->pvGetArg = callback_objects;
    }
    return TRUE;
}

void PyWinObject_FreeCRYPT_SIGN_MESSAGE_PARA(PCRYPT_SIGN_MESSAGE_PARA pcsmp)
{
    PyWinObject_FreeCERT_CONTEXTArray(pcsmp->rgpMsgCert, pcsmp->cMsgCert);
    PyWinObject_FreeCRYPT_ATTRIBUTEArray(pcsmp->rgAuthAttr, pcsmp->cAuthAttr);
    PyWinObject_FreeCRYPT_ATTRIBUTEArray(pcsmp->rgUnauthAttr, pcsmp->cUnauthAttr);
}

// @object PyCRYPT_SIGN_MESSAGE_PARA|Dict of parms defining how a message will be signed
BOOL PyWinObject_AsCRYPT_SIGN_MESSAGE_PARA(PyObject *obcsmp, PCRYPT_SIGN_MESSAGE_PARA pcsmp)
{
    static char *csmp_keys[] = {"SigningCert",      "HashAlgorithm",   "HashAuxInfo", "MsgCert",
                                "MsgCrl",           "AuthAttr",        "UnauthAttr",  "Flags",
                                "InnerContentType", "MsgEncodingType", NULL};
    PyObject *obSigningCert, *obHashAlgorithm;
    PyObject *obMsgCert = Py_None;
    PyObject *obMsgCrl = Py_None, *obHashAuxInfo = Py_None;   // Not supported yet
    PyObject *obAuthAttr = Py_None, *obUnauthAttr = Py_None;  // CRYPT_ATTRIBUTE tuples

    ZeroMemory(pcsmp, sizeof(*pcsmp));
    pcsmp->cbSize = sizeof(*pcsmp);
    pcsmp->dwMsgEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

    if (!PyDict_Check(obcsmp)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct CRYPT_VERIFY_MESSAGE_PARA structure must be a dict");
        return FALSE;
    }
    if (!PyArg_ParseTupleAndKeywords(
            dummy_tuple, obcsmp, "OO|OOOOOkkk:CRYPT_SIGN_MESSAGE_PARA", csmp_keys,
            &obSigningCert,    // @prop <o PyCERT_CONTEXT>|SigningCert|Certficate to be used to sign message
            &obHashAlgorithm,  // @prop <o PyCRYPT_ALGORITHM_IDENTIFIER>|HashAlgorithm|Algorithm to be used for signed
                               // hash
            &obHashAuxInfo,    // @prop None|HashAuxInfo|Optional.  Param is reserved, use only None.
            &obMsgCert,   // @prop (<o PyCERT_CONTEXT>,...)|MsgCert|Optional sequence of certificate to be included in
                          // the message.
            &obMsgCrl,    // @prop (CRL_CONTEXT,...)|MsgCrl|Optional. Sequence of certificate revocation lists. Not yet
                          // supported, use only None.
            &obAuthAttr,  // @prop (<o PyCRYPT_ATTRIBUTE>,...)|AuthAttr|Sequence of canonical attributes to be added to
                          // the message
            &obUnauthAttr,    // @prop (<o PyCRYPT_ATTRIBUTE>,...)|UnauthAttr|Sequence of arbitrary attributes
            &pcsmp->dwFlags,  // @prop int|Flags|Optional CRYPT_MESSAGE_*_FLAG that indicates content type if output is
                              // to be further encoded.
            &pcsmp->dwInnerContentType,  // @prop int|InnerContentType|Optional, one of the CMSG_* content types if
                                         // message is already encoded, .
            &pcsmp->dwMsgEncodingType))  // @prop int|MsgEncodingType|Encoding types, optional. Defaults to
                                         // X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING
        return NULL;

    if (obMsgCrl != Py_None) {
        PyErr_SetString(PyExc_NotImplementedError, "CRYPT_SIGN_MESSAGE_PARA: MsgCrl parm not yet supported");
        return FALSE;
    }
    if (obHashAuxInfo != Py_None) {
        PyErr_SetString(PyExc_NotImplementedError, "CRYPT_SIGN_MESSAGE_PARA: HashAuxInfo parm not yet supported");
        return FALSE;
    }

    if (PyWinObject_AsCERT_CONTEXT(obSigningCert, &pcsmp->pSigningCert, FALSE) &&
        PyWinObject_AsCRYPT_ALGORITHM_IDENTIFIER(obHashAlgorithm, &pcsmp->HashAlgorithm) &&
        PyWinObject_AsCERT_CONTEXTArray(obMsgCert, &pcsmp->rgpMsgCert, &pcsmp->cMsgCert) &&
        PyWinObject_AsCRYPT_ATTRIBUTEArray(obAuthAttr, &pcsmp->rgAuthAttr, &pcsmp->cAuthAttr) &&
        PyWinObject_AsCRYPT_ATTRIBUTEArray(obUnauthAttr, &pcsmp->rgUnauthAttr, &pcsmp->cUnauthAttr))
        return TRUE;
    PyWinObject_FreeCRYPT_SIGN_MESSAGE_PARA(pcsmp);
    return FALSE;
}

void PyWinObject_FreeCRYPT_ATTRIBUTE(PCRYPT_ATTRIBUTE pca)
{
    if (pca->rgValue != NULL) {
        for (DWORD i = 0; i < pca->cValue; i++) {
            if (pca->rgValue[i].pbData != NULL) {
                free(pca->rgValue[i].pbData);
                pca->rgValue[i].pbData = NULL;
            }
        }
        free(pca->rgValue);
    }
    ZeroMemory(pca, sizeof(*pca));
}

// @object PyCRYPT_ATTRIBUTE|Dict representing a CRYPT_ATTRIBUTE struct
BOOL PyWinObject_AsCRYPT_ATTRIBUTE(PyObject *obca, PCRYPT_ATTRIBUTE pca)
{
    static char *ca_keys[] = {"ObjId", "Value", NULL};
    PyObject *obvalues, *obvalue;
    DWORD value_ind;
    BOOL ret = TRUE;

    ZeroMemory(pca, sizeof(*pca));
    if (!PyDict_Check(obca)) {
        PyErr_SetString(PyExc_TypeError, "Object used to construct CRYPT_ATTRIBUTE must be a dict");
        return FALSE;
    }
    if (!PyArg_ParseTupleAndKeywords(
            dummy_tuple, obca, "sO:CRYPT_ATTRIBUTE", ca_keys,
            &pca->pszObjId,  // @prop str|ObjId|An szOID_* string identifying the attribute
            &obvalues))      // @prop (buffer,...)|Value|A sequence of buffers containing the attribute values
        return FALSE;
    TmpPyObject tuple_values = PyWinSequence_Tuple(obvalues, &pca->cValue);
    if (tuple_values == NULL)
        return FALSE;

    pca->rgValue = (PCRYPT_ATTR_BLOB)malloc(pca->cValue * sizeof(PCRYPT_ATTR_BLOB));
    if (pca->rgValue == NULL) {
        PyErr_NoMemory();
        return FALSE;  // last exit without cleanup
    }
    ZeroMemory(pca->rgValue, pca->cValue * sizeof(PCRYPT_ATTR_BLOB));
    for (value_ind = 0; value_ind < pca->cValue; value_ind++) {
        obvalue = PyTuple_GET_ITEM((PyObject *)tuple_values, value_ind);
        PyWinBufferView pybuf(obvalue);
        if (!pybuf.ok()) {
            ret = FALSE;
            break;
        }
        // Don't know if these blobs are modified anywhere, so copy the data instead of using python's internal buffer
        pca->rgValue[value_ind].pbData = (BYTE *)malloc(pybuf.len());
        if (pca->rgValue[value_ind].pbData == NULL) {
            PyErr_NoMemory();
            ret = FALSE;
            break;
        }
        DWORD bufsize = pybuf.len();
        memcpy(pca->rgValue[value_ind].pbData, pybuf.ptr(), bufsize);
        pca->rgValue[value_ind].cbData = bufsize;
    }
    if (!ret)
        PyWinObject_FreeCRYPT_ATTRIBUTE(pca);
    return ret;
}

void PyWinObject_FreeCRYPT_ATTRIBUTEArray(PCRYPT_ATTRIBUTE pca, DWORD attr_cnt)
{
    if (pca != NULL) {
        for (DWORD i = 0; i < attr_cnt; i++) PyWinObject_FreeCRYPT_ATTRIBUTE(&pca[i]);
        free(pca);
    }
}

BOOL PyWinObject_AsCRYPT_ATTRIBUTEArray(PyObject *obattrs, PCRYPT_ATTRIBUTE *ppca, DWORD *attr_cnt)
{
    DWORD bufsize;
    *ppca = NULL;
    *attr_cnt = 0;

    if (obattrs == Py_None)
        return TRUE;
    TmpPyObject tuple_attrs = PyWinSequence_Tuple(obattrs, attr_cnt);
    if (tuple_attrs == NULL)
        return FALSE;
    bufsize = *attr_cnt * sizeof(CRYPT_ATTRIBUTES);
    *ppca = (PCRYPT_ATTRIBUTE)malloc(bufsize);
    if (*ppca == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    ZeroMemory(*ppca, bufsize);

    for (DWORD attr_ind = 0; attr_ind < *attr_cnt; attr_ind++) {
        PyObject *obattr = PyTuple_GET_ITEM((PyObject *)tuple_attrs, attr_ind);
        if (!PyWinObject_AsCRYPT_ATTRIBUTE(obattr, ppca[attr_ind])) {
            PyWinObject_FreeCRYPT_ATTRIBUTEArray(*ppca, *attr_cnt);
            *ppca = NULL;
            *attr_cnt = 0;
            return FALSE;
        }
    }
    return TRUE;
}

void PyWinObject_FreeCERT_CONTEXTArray(PCCERT_CONTEXT *ppcerts, DWORD cert_cnt)
{
    if (ppcerts != NULL) {
        for (DWORD i = 0; i < cert_cnt; i++) {
            if (ppcerts[i] != NULL) {
                CertFreeCertificateContext(ppcerts[i]);
                ppcerts[i] = NULL;
            }
        }
        free(ppcerts);
    }
}

BOOL PyWinObject_AsCERT_CONTEXTArray(PyObject *obcerts, PCCERT_CONTEXT **pppcerts, DWORD *cert_cnt)
{
    *pppcerts = NULL;
    *cert_cnt = 0;
    if (obcerts == Py_None)
        return TRUE;
    TmpPyObject tuple_certs = PyWinSequence_Tuple(obcerts, cert_cnt);
    if (tuple_certs == NULL)
        return FALSE;
    *pppcerts = (PCCERT_CONTEXT *)malloc(*cert_cnt * sizeof(PCCERT_CONTEXT));
    if (*pppcerts == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    ZeroMemory(*pppcerts, *cert_cnt * sizeof(PCCERT_CONTEXT));
    for (DWORD cert_ind = 0; cert_ind < *cert_cnt; cert_ind++) {
        PyObject *obcert = PyTuple_GET_ITEM((PyObject *)tuple_certs, cert_ind);
        if (!PyWinObject_AsCERT_CONTEXT(obcert, &((*pppcerts)[cert_ind]), FALSE)) {
            PyWinObject_FreeCERT_CONTEXTArray(*pppcerts, *cert_cnt);
            *pppcerts = NULL;
            *cert_cnt = 0;
            return FALSE;
        }
        // Increment reference count of context
        (*pppcerts)[cert_ind] = CertDuplicateCertificateContext((*pppcerts)[cert_ind]);
    }
    return TRUE;
}

void PyWinObject_FreePBYTEArray(PBYTE *pbyte_array, DWORD *byte_lens, DWORD str_cnt)
{
    if (pbyte_array) {
        for (DWORD i = 0; i < str_cnt; i++) {
            if (pbyte_array[i] != NULL) {
                // buffers are not currently copied
                // free (pbyte_array[i];
                pbyte_array[i] = NULL;
            }
        }
        free(pbyte_array);
    }
    if (byte_lens)
        free(byte_lens);
}

// Converts a sequence of strings into an array of PBYTE pointers and array of lengths
BOOL PyWinObject_AsPBYTEArray(PyObject *str_seq, PBYTE **pbyte_array, DWORD **byte_lens, DWORD *str_cnt)
{
    BOOL ret = FALSE;
    DWORD bufsize, tuple_index;
    *pbyte_array = NULL;
    *byte_lens = NULL;
    *str_cnt = 0;
    TmpPyObject str_tuple = PyWinSequence_Tuple(str_seq, str_cnt);
    if (str_tuple == NULL)
        return FALSE;

    bufsize = *str_cnt * sizeof(PBYTE);
    *pbyte_array = (PBYTE *)malloc(bufsize);
    if (*pbyte_array == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
        goto cleanup;
    }
    ZeroMemory(*pbyte_array, bufsize);

    bufsize = *str_cnt * sizeof(DWORD);
    *byte_lens = (DWORD *)malloc(bufsize);
    if (*byte_lens == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
        goto cleanup;
    }
    ZeroMemory(*byte_lens, bufsize);

    for (tuple_index = 0; tuple_index < *str_cnt; tuple_index++) {
        PyObject *tuple_item = PyTuple_GET_ITEM((PyObject *)str_tuple, tuple_index);
        PyWinBufferView pybuf(tuple_item);
        if (!pybuf.ok())
            goto cleanup;
        // note: this might be unsafe, as we give away the buffer pointer to a
        // client outside of the scope where our RAII object 'pybuf' resides.
        (*pbyte_array)[tuple_index] = (BYTE *)pybuf.ptr();
        (*byte_lens)[tuple_index] = pybuf.len();
    }
    ret = TRUE;

cleanup:
    if (!ret) {
        PyWinObject_FreePBYTEArray(*pbyte_array, *byte_lens, *str_cnt);
        *str_cnt = 0;
        *pbyte_array = NULL;
        *byte_lens = NULL;
    }
    return ret;
}

void PyWinObject_FreeOIDArray(LPSTR *str_array, DWORD str_cnt)
{
    if (str_array) {
        for (DWORD i = 0; i < str_cnt; i++) {
            if (str_array[i] != NULL) {
                // character strings are not currently copied
                // if (HIWORD(str_array[i]))
                //	free (str_array[i];
                str_array[i] = NULL;
            }
        }
        free(str_array);
    }
}

// Create an array of OIDs, which can be either null-terminated strings or ints
BOOL PyWinObject_AsOIDArray(PyObject *str_seq, LPSTR **str_array, DWORD *str_cnt)
{
    BOOL ret = FALSE;
    DWORD bufsize, tuple_ind;
    *str_array = NULL;
    *str_cnt = 0;
    TmpPyObject str_tuple = PyWinSequence_Tuple(str_seq, str_cnt);
    if (str_tuple == NULL)
        return FALSE;

    bufsize = *str_cnt * sizeof(LPSTR);
    *str_array = (LPSTR *)malloc(bufsize);
    if (*str_array == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    ZeroMemory(*str_array, bufsize);

    for (tuple_ind = 0; tuple_ind < *str_cnt; tuple_ind++) {
        PyObject *tuple_item = PyTuple_GET_ITEM((PyObject *)str_tuple, tuple_ind);
        (*str_array)[tuple_ind] = PyBytes_AsString(tuple_item);
        if ((*str_array)[tuple_ind] != NULL)
            continue;
        PyErr_Clear();
        (*str_array)[tuple_ind] = (LPSTR)PyLong_AsVoidPtr(tuple_item);
        if (((*str_array)[tuple_ind] == (LPSTR)-1) && PyErr_Occurred())
            goto cleanup;
        if (HIWORD((*str_array)[tuple_ind])) {
            PyErr_SetString(PyExc_ValueError, "Integer OID must have high order word clear");
            goto cleanup;
        }
    }
    ret = TRUE;

cleanup:
    if (!ret) {
        PyWinObject_FreeOIDArray(*str_array, *str_cnt);
        *str_cnt = 0;
        *str_array = NULL;
    }
    return ret;
}

PyObject *PyWinObject_FromCTL_USAGE(PCTL_USAGE pUsage)
{
    // also works for CERT_ENHKEY_USAGE structs
    PyObject *ret, *ret_item;
    DWORD usage_index;
    ret = PyTuple_New(pUsage->cUsageIdentifier);
    if (ret != NULL)
        for (usage_index = 0; usage_index < pUsage->cUsageIdentifier; usage_index++) {
            ret_item = PyBytes_FromString(pUsage->rgpszUsageIdentifier[usage_index]);
            if (ret_item == NULL) {
                Py_DECREF(ret);
                ret = NULL;
                break;
            }
            PyTuple_SET_ITEM(ret, usage_index, ret_item);
        }
    return ret;
}

// @object PyCTL_USAGE|Sequence of string OIDs (szOID_*).  This struct is identical to CERT_ENHKEY_USAGE.
BOOL PyWinObject_AsCTL_USAGE(PyObject *ob, CTL_USAGE *pcu)
{
    if (!PyWinObject_AsOIDArray(ob, &pcu->rgpszUsageIdentifier, &pcu->cUsageIdentifier)) {
        PyWinObject_FreeCTL_USAGE(pcu);
        return FALSE;
    }
    return TRUE;
}

void PyWinObject_FreeCTL_USAGE(CTL_USAGE *pcu)
{
    PyWinObject_FreeOIDArray(pcu->rgpszUsageIdentifier, pcu->cUsageIdentifier);
    ZeroMemory(pcu, sizeof(*pcu));
}

// @object PyCERT_KEY_ATTRIBUTES_INFO|Dict representing a CERT_KEY_ATTRIBUTES_INFO struct
// @prop str|KeyId|Usually a hash that uniquely identifies the key
// @prop <o PyCRYPT_BIT_BLOB>|IntendedKeyUsage|Contains a byte with CERT_*_KEY_USAGE flags
// @prop dict|PrivateKeyUsagePeriod|Private key's begin and end effective dates, may be None
PyObject *PyWinObject_FromCERT_KEY_ATTRIBUTES_INFO(PCERT_KEY_ATTRIBUTES_INFO pckai)
{
    PyObject *obusageperiod;
    if (pckai->pPrivateKeyUsagePeriod == NULL) {
        Py_INCREF(Py_None);
        obusageperiod = Py_None;
    }
    else {
        obusageperiod =
            Py_BuildValue("{s:N, s:N}", "NotBefore", PyWinObject_FromFILETIME(pckai->pPrivateKeyUsagePeriod->NotBefore),
                          "NotAfter", PyWinObject_FromFILETIME(pckai->pPrivateKeyUsagePeriod->NotAfter));
        if (obusageperiod == NULL)
            return NULL;
    }

    return Py_BuildValue("{s:N, s:N, s:N}", "KeyId",
                         PyBytes_FromStringAndSize((char *)pckai->KeyId.pbData, pckai->KeyId.cbData),
                         "IntendedKeyUsage", PyWinObject_FromCRYPT_BIT_BLOB(&pckai->IntendedKeyUsage),
                         "PrivateKeyUsagePeriod", obusageperiod);
}

// @object PyCERT_BASIC_CONSTRAINTS_INFO|Dict representing a CERT_BASIC_CONSTRAINTS_INFO struct
// @prop <o PyCRYPT_BIT_BLOB>|SubjectType|Contains a combination of CERT_CA_SUBJECT_FLAG,CERT_END_ENTITY_SUBJECT_FLAG
// @prop boolean|fPathLenConstraint|Indicates if PathLenConstraint member is used
// @prop int|PathLenConstraint|Limits number of intermediate CA's between root CA and end user
// @prop tuple|SubtreesConstraint|Sequence of encoded name blobs
PyObject *PyWinObject_FromCERT_BASIC_CONSTRAINTS_INFO(PCERT_BASIC_CONSTRAINTS_INFO pcbci)
{
    PyObject *sc = PyTuple_New(pcbci->cSubtreesConstraint);
    if (sc == NULL)
        return NULL;
    for (DWORD i = 0; i < pcbci->cSubtreesConstraint; i++) {
        PyObject *nb = PyBytes_FromStringAndSize((char *)pcbci->rgSubtreesConstraint[i].pbData,
                                                 pcbci->rgSubtreesConstraint[i].cbData);
        if (nb == NULL) {
            Py_DECREF(sc);
            return NULL;
        }
        PyTuple_SET_ITEM(sc, i, nb);
    }

    return Py_BuildValue("{s:N, s:N, s:k, s:N}", "SubjectType", PyWinObject_FromCRYPT_BIT_BLOB(&pcbci->SubjectType),
                         "fPathLenConstraint", PyBool_FromLong(pcbci->fPathLenConstraint), "PathLenConstraint",
                         pcbci->dwPathLenConstraint, "SubtreesConstraint", sc);
}

// @object PyCERT_BASIC_CONSTRAINTS2_INFO|Dict representing a CERT_BASIC_CONSTRAINTS2_INFO struct
// @prop boolean|fCA|Indicates if cert represents a certificate authority
// @prop boolean|fPathLenConstraint|Indicates if PathLenConstraint member is used
// @prop int|PathLenConstraint|Limits number of intermediate CA's between root CA and end user
PyObject *PyWinObject_FromCERT_BASIC_CONSTRAINTS2_INFO(PCERT_BASIC_CONSTRAINTS2_INFO pcbci)
{
    return Py_BuildValue("{s:N, s:N, s:k}", "fCA", PyBool_FromLong(pcbci->fCA), "fPathLenConstraint",
                         PyBool_FromLong(pcbci->fPathLenConstraint), "PathLenConstraint", pcbci->dwPathLenConstraint);
}

// @object PyCERT_POLICY_INFO|Dict containing a certificate policy
// @prop str|PolicyIdentifier|OID identifying the policy
// @prop tuple|PolicyQualifier|Sequence of CERT_POLICY_QUALIFIER dicts
PyObject *PyWinObject_FromCERT_POLICY_INFO(PCERT_POLICY_INFO pcpi)
{
    PyObject *quals = PyTuple_New(pcpi->cPolicyQualifier);
    if (quals == NULL)
        return NULL;
    for (DWORD qual_ind = 0; qual_ind < pcpi->cPolicyQualifier; qual_ind++) {
        PyObject *qual = Py_BuildValue(
            "{s:s,s:N}", "PolicyQualifierId", pcpi->rgPolicyQualifier[qual_ind].pszPolicyQualifierId, "Qualifier",
            PyBytes_FromStringAndSize((char *)pcpi->rgPolicyQualifier[qual_ind].Qualifier.pbData,
                                      pcpi->rgPolicyQualifier[qual_ind].Qualifier.cbData));
        if (qual == NULL) {
            Py_DECREF(quals);
            return NULL;
        }
        PyTuple_SET_ITEM(quals, qual_ind, qual);
    }
    return Py_BuildValue("{s:s, s:N}", "PolicyIdentifier", pcpi->pszPolicyIdentifier, "PolicyQualifier", quals);
}

PyObject *PyWinObject_FromCERT_POLICIES_INFO(PCERT_POLICIES_INFO pcpi)
{
    PyObject *ret = PyTuple_New(pcpi->cPolicyInfo);
    if (ret == NULL)
        return NULL;
    for (DWORD policy_ind = 0; policy_ind < pcpi->cPolicyInfo; policy_ind++) {
        PyObject *obpi = PyWinObject_FromCERT_POLICY_INFO(&pcpi->rgPolicyInfo[policy_ind]);
        if (obpi == NULL) {
            Py_DECREF(ret);
            return NULL;
        }
        PyTuple_SET_ITEM(ret, policy_ind, obpi);
    }
    return ret;
}

// @object PyCERT_AUTHORITY_KEY_ID_INFO|Dict containing the identity of a CA
// @prop str|KeyId|Unique identifier of private key, usually a hash
// @prop str|CertIssuer|Encoded DN of the Certificate Authority.  Decode using X509_UNICODE_NAME
// @prop int|CertSerialNumber|Serial nbr of the CA's signing certificate
PyObject *PyWinObject_FromCERT_AUTHORITY_KEY_ID_INFO(PCERT_AUTHORITY_KEY_ID_INFO pcaki)
{
    return Py_BuildValue("{s:N, s:N, s:N}", "KeyId",
                         PyBytes_FromStringAndSize((char *)pcaki->KeyId.pbData, pcaki->KeyId.cbData), "CertIssuer",
                         PyBytes_FromStringAndSize((char *)pcaki->CertIssuer.pbData, pcaki->CertIssuer.cbData),
                         "CertSerialNumber", PyWinObject_FromCRYPT_INTEGER_BLOB(&pcaki->CertSerialNumber));
}
