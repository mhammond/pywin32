// @doc
#include "win32crypt.h"

// @object PyCERT_CONTEXT|Handle to a certificate context
struct PyMethodDef PyCERT_CONTEXT::methods[] = {
    // @pymeth CertFreeCertificateContext|Frees the context handle
    {"CertFreeCertificateContext", (PyCFunction)PyCERT_CONTEXT::PyCertFreeCertificateContext, METH_NOARGS},
    // @pymeth CertEnumCertificateContextProperties|Lists property ids for the certificate
    {"CertEnumCertificateContextProperties", (PyCFunction)PyCERT_CONTEXT::PyCertEnumCertificateContextProperties,
     METH_NOARGS},
    // @pymeth CryptAcquireCertificatePrivateKey|Retrieves the private key associated with the certificate
    {"CryptAcquireCertificatePrivateKey", (PyCFunction)PyCERT_CONTEXT::PyCryptAcquireCertificatePrivateKey,
     METH_KEYWORDS | METH_VARARGS},
    // @pymeth CertGetIntendedKeyUsage|Returns the intended key usage from the certificate extensions
    {"CertGetIntendedKeyUsage", (PyCFunction)PyCERT_CONTEXT::PyCertGetIntendedKeyUsage, METH_NOARGS},
    // @pymeth CertGetEnhancedKeyUsage|Finds the enhanced key usage property and/or extension for the certificate
    {"CertGetEnhancedKeyUsage", (PyCFunction)PyCERT_CONTEXT::PyCertGetEnhancedKeyUsage, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CertSerializeCertificateStoreElement|Serializes the certificate and its properties
    {"CertSerializeCertificateStoreElement", (PyCFunction)PyCERT_CONTEXT::PyCertSerializeCertificateStoreElement,
     METH_KEYWORDS | METH_VARARGS},
    // @pymeth CertVerifySubjectCertificateContext|Checks the validity of the certificate
    {"CertVerifySubjectCertificateContext", (PyCFunction)PyCERT_CONTEXT::PyCertVerifySubjectCertificateContext,
     METH_KEYWORDS | METH_VARARGS},
    // @pymeth CertDeleteCertificateFromStore|Removes the certificate from its store
    {"CertDeleteCertificateFromStore", (PyCFunction)PyCERT_CONTEXT::PyCertDeleteCertificateFromStore, METH_NOARGS},
    // @pymeth CertGetCertificateContextProperty|Retrieves the specified property from the certificate
    {"CertGetCertificateContextProperty", (PyCFunction)PyCERT_CONTEXT::PyCertGetCertificateContextProperty,
     METH_KEYWORDS | METH_VARARGS},
    // @pymeth CertSetCertificateContextProperty|Sets a property for a certificate
    {"CertSetCertificateContextProperty", (PyCFunction)PyCERT_CONTEXT::PyCertSetCertificateContextProperty,
     METH_KEYWORDS | METH_VARARGS},
    {NULL}};

PyTypeObject PyCERT_CONTEXTType = {PYWIN_OBJECT_HEAD "PyCERT_CONTEXT",
                                   sizeof(PyCERT_CONTEXT),
                                   0,
                                   PyCERT_CONTEXT::deallocFunc, /* tp_dealloc */
                                   0,                           /* tp_print */
                                   0,                           /* tp_getattr */
                                   0,                           /* tp_setattr */
                                   0,                           /* tp_compare */
                                   0,                           /* tp_repr */
                                   0,                           /* tp_as_number */
                                   0,                           /* tp_as_sequence */
                                   0,                           /* tp_as_mapping */
                                   0,
                                   0,                                         /* tp_call */
                                   0,                                         /* tp_str */
                                   PyCERT_CONTEXT::getattro,                  // tp_getattro
                                   PyCERT_CONTEXT::setattro,                  // tp_setattro
                                   0,                                         // PyBufferProcs *tp_as_buffer
                                   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
                                   0,                                         // tp_doc
                                   0,                                         // traverseproc tp_traverse
                                   0,                                         // tp_clear
                                   0,                                         // richcmpfunc tp_richcompare
                                   0,                                         // tp_weaklistoffset
                                   0,                                         // getiterfunc tp_iter
                                   0,                                         // iternextfunc tp_iternext
                                   PyCERT_CONTEXT::methods,
                                   PyCERT_CONTEXT::members};

struct PyMemberDef PyCERT_CONTEXT::members[] = {
    // @prop int|HANDLE|Pointer to CERT_CONTEXT struct
    {"HANDLE", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyCERTSTORE>|CertStore|Handle to the certificate store that contains this certificate
    {"CertStore", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop str|CertEncoded|Content of the certificate as encoded bytes
    {"CertEncoded", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop int|CertEncodingType|Method used to encode the certifcate, usually X509_ASN_ENCODING or PKCS_7_ASN_ENCODING
    {"CertEncodingType", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop int|Version|One of the CERT_V* values
    {"Version", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyUnicode>|Subject|Encoded CERT_NAME_INFO struct containing the subject name. Can be decoded
    //	using <om cryptoapi.CryptDecodeObjectEx> with X509_UNICODE_NAME, or formatted using <om cryptoapi.CertNameToStr>
    {"Subject", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyUnicode>|Issuer|Certificate Authority that issued certificate as encoded CERT_NAME_INFO.  Use
    //	<om cryptoapi.CryptDecodeObjectEx> to decode into individual components, or <om cryptoapi.CertNameToStr> to
    //	return a single formatted string
    {"Issuer", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyTime>|NotBefore|Beginning of certificate's period of validity
    {"NotBefore", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyTime>|NotAfter|End of certificate's period of validity
    {"NotAfter", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop str|SignatureAlgorithm|Object id of the certifcate's signature algorithm
    {"SignatureAlgorithm", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop (<o PyCERT_EXTENSION>,...)|Extension|Sequence of CERT_EXTENSION dicts containing certificate's extensions
    {"Extension", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop <o PyCERT_PUBLIC_KEY_INFO>|SubjectPublicKeyInfo|Encoded public key of certificate
    {"SubjectPublicKeyInfo", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    // @prop int|SerialNumber|Serial number assigned by the issuer
    {"SerialNumber", T_OBJECT, offsetof(PyCERT_CONTEXT, obdummy), READONLY},
    {NULL}};

int PyCERT_CONTEXT::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
    return PyObject_GenericSetAttr(self, obname, obvalue);
}

// @object PyCERT_EXTENSION|Dict containing a certificate extension
// @prop str|ObjId|The OID identifying the type of extension
// @prop boolean|Critical|If true, any contraints or limits contained in the extension should be considered absolute
// @prop str|Value|Binary string containing ASN encoded data.
//	To interpret or display extension data, see <om cryptoapi.CryptDecodeObjectEx> and <om cryptoapi.CryptFormatObject>.
/* @comm
    These extensions are not yet handled by CryptDecodeObjectEx, but can be formatted with CryptFormatObject.
        <nl>szOID_PRIVATEKEY_USAGE_PERIOD -- ???? CERT_PRIVATE_KEY_VALIDITY ????
        <nl>szOID_KEY_USAGE_RESTRICTION - CERT_KEY_USAGE_RESTRICTION_INFO
*/
PyObject *PyWinObject_FromCERT_EXTENSIONArray(PCERT_EXTENSION pce, DWORD ext_cnt)
{
    DWORD ext_ind;
    PyObject *ret, *ret_item;
    ret = PyTuple_New(ext_cnt);
    if (ret == NULL)
        return NULL;

    for (ext_ind = 0; ext_ind < ext_cnt; ext_ind++) {
        ret_item = Py_BuildValue(
            "{s:s,s:N,s:N}", "ObjId", pce[ext_ind].pszObjId, "Critical", PyBool_FromLong(pce[ext_ind].fCritical),
            "Value", PyString_FromStringAndSize((char *)pce[ext_ind].Value.pbData, pce[ext_ind].Value.cbData));
        if (ret_item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, ext_ind, ret_item);
    }
    return ret;
}

PyObject *PyCERT_CONTEXT::getattro(PyObject *self, PyObject *obname)
{
    PCCERT_CONTEXT pcc = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    if (strcmp(name, "HANDLE") == 0)
        return PyLong_FromVoidPtr((void *)pcc);
    if (strcmp(name, "CertStore") == 0) {
        HCERTSTORE h = NULL;
        // Increment the store's reference count since CertCloseStore is called when object is destroyed.
        // CertDuplicateStore throws an access violation if store handle is NULL
        if (pcc->hCertStore != NULL)
            h = CertDuplicateStore(pcc->hCertStore);
        return PyWinObject_FromCERTSTORE(h);
    }
    if (strcmp(name, "CertEncoded") == 0)
        return PyString_FromStringAndSize((char *)pcc->pbCertEncoded, pcc->cbCertEncoded);
    if (strcmp(name, "CertEncodingType") == 0)
        return PyLong_FromUnsignedLong(pcc->dwCertEncodingType);
    if (strcmp(name, "Version") == 0)
        return PyLong_FromUnsignedLong(pcc->pCertInfo->dwVersion);
    if (strcmp(name, "Issuer") == 0)
        return PyString_FromStringAndSize((char *)pcc->pCertInfo->Issuer.pbData, pcc->pCertInfo->Issuer.cbData);
    if (strcmp(name, "Subject") == 0)
        return PyString_FromStringAndSize((char *)pcc->pCertInfo->Subject.pbData, pcc->pCertInfo->Subject.cbData);
    if (strcmp(name, "NotBefore") == 0)
        return PyWinObject_FromFILETIME(pcc->pCertInfo->NotBefore);
    if (strcmp(name, "NotAfter") == 0)
        return PyWinObject_FromFILETIME(pcc->pCertInfo->NotAfter);
    if (strcmp(name, "SignatureAlgorithm") == 0)
        return PyWinObject_FromCRYPT_ALGORITHM_IDENTIFIER(&pcc->pCertInfo->SignatureAlgorithm);
    if (strcmp(name, "Extension") == 0)
        return PyWinObject_FromCERT_EXTENSIONArray(pcc->pCertInfo->rgExtension, pcc->pCertInfo->cExtension);
    if (strcmp(name, "SubjectPublicKeyInfo") == 0)
        return PyWinObject_FromCERT_PUBLIC_KEY_INFO(&pcc->pCertInfo->SubjectPublicKeyInfo);
    if (strcmp(name, "SerialNumber") == 0)
        return PyWinObject_FromCRYPT_INTEGER_BLOB(&pcc->pCertInfo->SerialNumber);
    return PyObject_GenericGetAttr(self, obname);
}

PyCERT_CONTEXT::~PyCERT_CONTEXT(void) { CertFreeCertificateContext(pccert_context); }

void PyCERT_CONTEXT::deallocFunc(PyObject *ob) { delete (PyCERT_CONTEXT *)ob; }

PyCERT_CONTEXT::PyCERT_CONTEXT(PCCERT_CONTEXT pccert_context)
{
    ob_type = &PyCERT_CONTEXTType;
    _Py_NewReference(this);
    this->pccert_context = pccert_context;
    this->obdummy = NULL;
}

BOOL PyWinObject_AsCERT_CONTEXT(PyObject *obpccert_context, PCCERT_CONTEXT *pccert_context, BOOL bNoneOK)
{
    if (bNoneOK && (obpccert_context == Py_None)) {
        *pccert_context = NULL;
        return true;
    }
    if (obpccert_context->ob_type != &PyCERT_CONTEXTType) {
        PyErr_SetString(PyExc_TypeError, "Object must be of type PyCERT_CONTEXT");
        return FALSE;
    }
    *pccert_context = ((PyCERT_CONTEXT *)obpccert_context)->GetPCCERT_CONTEXT();
    return TRUE;
}

PyObject *PyWinObject_FromCERT_CONTEXT(PCCERT_CONTEXT pcc)
{
    if (pcc == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyCERT_CONTEXT(pcc);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyWinObject_FromCERT_CONTEXT: Unable to allocate PyCERT_CONTEXT");
    return ret;
}

// @pymethod |PyCERT_CONTEXT|CertFreeCertificateContext|Frees the certificate context
PyObject *PyCERT_CONTEXT::PyCertFreeCertificateContext(PyObject *self, PyObject *args)
{
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertFreeCertificateContext(pccert_context);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertFreeCertificateContext");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod [int,...]|PyCERT_CONTEXT|CertEnumCertificateContextProperties|Lists property ids for the certificate
PyObject *PyCERT_CONTEXT::PyCertEnumCertificateContextProperties(PyObject *self, PyObject *args)
{
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    PyObject *ret_item = NULL;
    DWORD err = 0, dwPropId = 0;
    PyObject *ret = PyList_New(0);
    if (ret == NULL)
        return NULL;
    while (TRUE) {
        Py_BEGIN_ALLOW_THREADS dwPropId = CertEnumCertificateContextProperties(pccert_context, dwPropId);
        Py_END_ALLOW_THREADS if (dwPropId == 0) break;
        ret_item = PyLong_FromUnsignedLong(dwPropId);
        if ((ret_item == NULL) || (PyList_Append(ret, ret_item) == -1)) {
            Py_XDECREF(ret_item);
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        Py_DECREF(ret_item);
    }
    return ret;
}

// @pymethod (int,<o PyCRYPTPROV>)|PyCERT_CONTEXT|CryptAcquireCertificatePrivateKey|Retrieves the private key associated
// with the certificate
// @rdesc Returns the KeySpec (AT_KEYEXCHANGE or AT_SIGNATURE) and a CSP handle to the key
// @comm Only the owner of the certificate can use this method
PyObject *PyCERT_CONTEXT::PyCryptAcquireCertificatePrivateKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    HCRYPTPROV hcryptprov;
    DWORD flags = 0, keyspec;
    BOOL callerfree;
    PVOID reserved = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CryptAcquireCertificatePrivateKey", keywords,
                                     &flags))  // @pyparm int|Flags|0|Combination of CRYPT_ACQUIRE_*_FLAG constants
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CryptAcquireCertificatePrivateKey(pccert_context, flags, reserved, &hcryptprov, &keyspec, &callerfree);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CryptAcquireCertificatePrivateKey");

    /* If callerfree returns false, CSP handle shouldn't be freed, so increase its refcount since
        CryptReleaseContext is called when python object is destroyed */
    if (!callerfree)
        if (!CryptContextAddRef(hcryptprov, NULL, 0))
            return PyWin_SetAPIError("CryptContextAddRef");
    return Py_BuildValue("kN", keyspec, new PyCRYPTPROV(hcryptprov));
}

// @pymethod tuple|PyCERT_CONTEXT|CertGetEnhancedKeyUsage|Finds the enhanced key usage property and/or extension for the
// certificate
// @rdesc Returns a sequence of usage OIDs
PyObject *PyCERT_CONTEXT::PyCertGetEnhancedKeyUsage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    PyObject *ret = NULL;
    DWORD flags = 0, bufsize = 0;
    PCERT_ENHKEY_USAGE pceu = NULL;
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CertGetEnhancedKeyUsage", keywords,
                                     &flags))  // @pyparm int|Flags|0|CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                                               // CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, or 0
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertGetEnhancedKeyUsage(pccert_context, flags, pceu, &bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertGetEnhancedKeyUsage");
    pceu = (PCERT_ENHKEY_USAGE)malloc(bufsize);
    if (pceu == NULL)
        return PyErr_Format(PyExc_MemoryError, "Failed to allocate %d bytes", bufsize);
    Py_BEGIN_ALLOW_THREADS bsuccess = CertGetEnhancedKeyUsage(pccert_context, flags, pceu, &bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CertGetEnhancedKeyUsage");
    else ret = PyWinObject_FromCTL_USAGE(pceu);
    free(pceu);
    return ret;
}

// @pymethod int|PyCERT_CONTEXT|CertGetIntendedKeyUsage|Returns the intended key usage from the certificate extensions
// (szOID_KEY_USAGE or szOID_KEY_ATTRIBUTES)
// @rdesc Returns a combination of CERT_*_KEY_USAGE values
PyObject *PyCERT_CONTEXT::PyCertGetIntendedKeyUsage(PyObject *self, PyObject *args)
{
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    DWORD buf;
    DWORD bufsize = sizeof(DWORD);
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess =
        CertGetIntendedKeyUsage(pccert_context->dwCertEncodingType, pccert_context->pCertInfo, (BYTE *)&buf, bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertGetIntendedKeyUsage");
    return PyLong_FromUnsignedLong(buf);
}

// @pymethod string|PyCERT_CONTEXT|CertSerializeCertificateStoreElement|Serializes the certificate and its properties
PyObject *PyCERT_CONTEXT::PyCertSerializeCertificateStoreElement(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    PyObject *ret = NULL;
    DWORD flags = 0, bufsize = 0;
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    BYTE *buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CertSerializeCertificateStoreElement", keywords,
                                     &flags))  // @pyparm int|Flags|0|Reserved, use only 0 if passed in
        return NULL;

    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertSerializeCertificateStoreElement(pccert_context, flags, buf, &bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertSerializeCertificateStoreElement");
    buf = (BYTE *)malloc(bufsize);
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);

    Py_BEGIN_ALLOW_THREADS bsuccess = CertSerializeCertificateStoreElement(pccert_context, flags, buf, &bufsize);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CertSerializeCertificateStoreElement");
    else ret = PyString_FromStringAndSize((char *)buf, bufsize);
    free(buf);
    return ret;
}

// @pymethod int|PyCERT_CONTEXT|CertVerifySubjectCertificateContext|Checks the validity of the certificate
// @rdesc Returns flags indicating which validity checks failed, or 0 if all were successful.
PyObject *PyCERT_CONTEXT::PyCertVerifySubjectCertificateContext(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Issuer", "Flags", NULL};
    PyObject *obissuer;
    DWORD flags;
    PCCERT_CONTEXT issuer;
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:CertVerifySubjectCertificateContext", keywords,
            &obissuer,  // @pyparm <o PyCERT_CONTEXT>|Issuer||Certificate of authority that issued the certificate
            &flags))    // @pyparm int|Flags||Combination of CERT_STORE_REVOCATION_FLAG,CERT_STORE_SIGNATURE_FLAG and
                        // CERT_STORE_TIME_VALIDITY_FLAG indicating which checks should be performed
        return NULL;
    if (!PyWinObject_AsCERT_CONTEXT(obissuer, &issuer, TRUE))
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertVerifySubjectCertificateContext(pccert_context, issuer, &flags);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertVerifySubjectCertificateContext");
    return PyLong_FromUnsignedLong(flags);
}

// @pymethod |PyCERT_CONTEXT|CertDeleteCertificateFromStore|Removes the certificate from its store
PyObject *PyCERT_CONTEXT::PyCertDeleteCertificateFromStore(PyObject *self, PyObject *args)
{
    PCCERT_CONTEXT pcert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertDeleteCertificateFromStore(pcert_context);
    Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertDeleteCertificateFromStore");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod object|PyCERT_CONTEXT|CertGetCertificateContextProperty|Retrieves the specified property from the
// certificate
// @rdesc Type of object returned is dependent on the property id requested.
PyObject *PyCERT_CONTEXT::PyCertGetCertificateContextProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"PropId", NULL};
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    PyObject *ret = NULL;
    DWORD dwPropId, pcbData = 0;
    void *pvData = NULL;
    CRYPT_DATA_BLOB *pcdb = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:CertGetCertificateContextProperty", keywords,
                                     &dwPropId))  // @pyparm int|PropId||One of the CERT_*_PROP_ID constants
        return NULL;
    BOOL bsuccess;
    Py_BEGIN_ALLOW_THREADS bsuccess = CertGetCertificateContextProperty(pccert_context, dwPropId, pvData, &pcbData);
    Py_END_ALLOW_THREADS if (!bsuccess)
    {
        PyWin_SetAPIError("CertGetCertificateContextProperty");
        return NULL;
    }
    // some properties return no data, presence only is significant
    if (pcbData > 0) {
        pvData = malloc(pcbData);
        if (pvData == NULL)
            return PyErr_Format(PyExc_MemoryError, "CertGetCertificateContextProperty: unable to allocate %d bytes",
                                pcbData);
        ZeroMemory(pvData, pcbData);
        Py_BEGIN_ALLOW_THREADS bsuccess = CertGetCertificateContextProperty(pccert_context, dwPropId, pvData, &pcbData);
        Py_END_ALLOW_THREADS if (!bsuccess)
        {
            PyWin_SetAPIError("CertGetCertificateContextProperty");
            free(pvData);
            return NULL;
        }
    }
    // @flagh PropId|Returned value
    switch (dwPropId) {
        case CERT_ARCHIVED_PROP_ID:  // @flag CERT_ARCHIVED_PROP_ID|Boolean
            Py_INCREF(Py_True);
            ret = Py_True;  // no data returned, success is only indicator
            break;
        case CERT_DATE_STAMP_PROP_ID:  // @flag CERT_DATE_STAMP_PROP_ID|<o PyTime>
            ret = PyWinObject_FromFILETIME(*((FILETIME *)pvData));
            break;
        case CERT_ACCESS_STATE_PROP_ID:  // @flag CERT_ACCESS_STATE_PROP_ID|int
        case CERT_KEY_SPEC_PROP_ID:      // @flag CERT_KEY_SPEC_PROP_ID|int
            ret = PyLong_FromUnsignedLong(*((DWORD *)pvData));
            break;
        case CERT_DESCRIPTION_PROP_ID:    // @flag CERT_DESCRIPTION_PROP_ID|Unicode
        case CERT_FRIENDLY_NAME_PROP_ID:  // @flag CERT_FRIENDLY_NAME_PROP_ID|Unicode
        case CERT_PVK_FILE_PROP_ID:       // @flag CERT_PVK_FILE_PROP_ID|Unicode
        case CERT_AUTO_ENROLL_PROP_ID:    // @flag CERT_AUTO_ENROLL_PROP_ID|Unicode
            ret = PyWinObject_FromWCHAR((WCHAR *)pvData, pcbData / sizeof(WCHAR));
            break;
        // CERT_HASH_PROP_ID is same value as CERT_SHA1_HASH_PROP_ID (3)
        // @flag CERT_HASH_PROP_ID|String containing a hash
        case CERT_SHA1_HASH_PROP_ID:              // @flag CERT_SHA1_HASH_PROP_ID|String containing a hash
        case CERT_MD5_HASH_PROP_ID:               // @flag CERT_MD5_HASH_PROP_ID|String containing a hash
        case CERT_SIGNATURE_HASH_PROP_ID:         // @flag CERT_SIGNATURE_HASH_PROP_ID|String containing a hash
        case CERT_KEY_IDENTIFIER_PROP_ID:         // @flag CERT_KEY_IDENTIFIER_PROP_ID|String containing a hash
        case CERT_SUBJECT_NAME_MD5_HASH_PROP_ID:  // @flag CERT_SUBJECT_NAME_MD5_HASH_PROP_ID|String containing a hash
            ret = PyString_FromStringAndSize((char *)pvData, pcbData);
            // all hashes treated as raw binary data
            break;
        case CERT_KEY_PROV_HANDLE_PROP_ID:  // @flag CERT_KEY_PROV_HANDLE_PROP_ID|<o PyCRYPTPROV>
            ret = new PyCRYPTPROV(*((HCRYPTPROV *)pvData));
            break;
        case CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID:  // @flag CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID|String
                                                        // containing a hash
        case CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID:   // @flag CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID|String
                                                        // containing a hash
            ret = PyString_FromStringAndSize((char *)pvData, pcbData);
            /* MSDN claims these return a CRYPT_DATA_BLOB, but data is not valid when
               interpreted as such - cbdata is huge, and pbData is not a valid pointer.
               The returned pcbData exactly matches size of an MD5 hash, and it would
               actually have to be the size of the hash + sizeof(CRYPT_DATA_BLOB) since
               everything is returned in a single allocated block
            pcdb=(CRYPT_DATA_BLOB *)pvData;
            ret=PyString_FromStringAndSize((char *)pcdb->pbData,pcdb->cbData);
            */
            break;
        // CERT_CTL_USAGE_PROP_ID is same value as CERT_ENHKEY_USAGE_PROP_ID
        // @flag CERT_CTL_USAGE_PROP_ID|Encoded CTL_USAGE, decode as X509_ENHANCED_KEY_USAGE (CTL_USAGE and
        // CERT_ENHKEY_USAGE are identical)
        // @flag CERT_ENHKEY_USAGE_PROP_ID|Encoded CTL_USAGE. Can be decoded using <om cryptoapi.CryptDecodeObjectEx>
        // with X509_ENHANCED_KEY_USAGE
        case CERT_ENHKEY_USAGE_PROP_ID:
            ret = PyString_FromStringAndSize((char *)pvData, pcbData);
            break;
        case CERT_KEY_PROV_INFO_PROP_ID:  // @flag CERT_KEY_PROV_INFO_PROP_ID|CRYPT_KEY_PROV_INFO dict
            ret = PyWinObject_FromCRYPT_KEY_PROV_INFO((PCRYPT_KEY_PROV_INFO)pvData);
            break;
        case CERT_KEY_CONTEXT_PROP_ID:  // @flag CERT_KEY_CONTEXT_PROP_ID|Dict representing CERT_KEY_CONTEXT struct
            ret = Py_BuildValue("{s:N, s:k}", "CryptProv", new PyCRYPTPROV(((PCERT_KEY_CONTEXT)pvData)->hCryptProv),
                                "KeySpec", ((PCERT_KEY_CONTEXT)pvData)->dwKeySpec);
            break;
        // @flag CERT_NEXT_UPDATE_LOCATION_PROP_ID|Encoded CERT_ALT_NAME_INFO, decode using <om
        // cryptoapi.CryptDecodeObjectEx> with szOID_NEXT_UPDATE_LOCATION
        case CERT_NEXT_UPDATE_LOCATION_PROP_ID:
            ret = PyString_FromStringAndSize((char *)pvData, pcbData);
            break;
        // case CERT_PUBKEY_ALG_PROP_ID: // ???? This constant does not exist in my header files ????
        case CERT_ENROLLMENT_PROP_ID:  // CRYPT_DATA_BLOB, data will apparently have to be split out manually
        case CERT_CROSS_CERT_DIST_POINTS_PROP_ID:  // CRYPT_DATA_BLOB which contains encoded CROSS_CERT_DIST_POINTS_INFO
                                                   // (X509_CROSS_CERT_DIST_POINTS)
        default:
            PyErr_SetString(PyExc_NotImplementedError, "Not yet supported");
    }
    free(pvData);
    return ret;
}

// @pymethod |PyCERT_CONTEXT|CertSetCertificateContextProperty|Sets a property for a certificate
PyObject *PyCERT_CONTEXT::PyCertSetCertificateContextProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"PropId", "Data", "Flags", NULL};
    PCCERT_CONTEXT pccert_context = ((PyCERT_CONTEXT *)self)->GetPCCERT_CONTEXT();
    PyObject *obData;
    DWORD prop, flags = 0, dwData;
    FILETIME ftData;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kO|k:CertSetCertificateContextProperty", keywords,
                                     &prop,    // @pyparm int|PropId||Id of property to be set, CERT_*_PROP_ID
                                     &obData,  // @pyparm object|Data||The value to be set for the property.  Type is
                                               // dependent on PropId. Use None to delete a property.
                                     &flags))  // @pyparm int|Flags|0|Combination of CERT_SET_* flags
        return NULL;

    BOOL bsuccess;
    // When Data is None, property is to be deleted so no conversion necessary
    if (obData == Py_None) {
        Py_BEGIN_ALLOW_THREADS bsuccess = CertSetCertificateContextProperty(pccert_context, prop, flags, NULL);
        Py_END_ALLOW_THREADS if (!bsuccess) return PyWin_SetAPIError("CertSetCertificateContextProperty");
        else
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
    }

    PyObject *ret = NULL;
    CRYPT_DATA_BLOB cdb = {0, NULL};
    void *pvData = NULL;
    // @flagh PropId|Type of input
    switch (prop) {
        case CERT_ARCHIVED_PROP_ID:  // @flag CERT_ARCHIVED_PROP_ID|None causes Archived flag to be cleared, any other
                                     // causes it to be set
            pvData = &cdb;           // no actual data, non-NULL pvData indicates presence of flag
            break;
        case CERT_DATE_STAMP_PROP_ID:  // @flag CERT_DATE_STAMP_PROP_ID|<o PyTime> specifying when cert was added to
                                       // store
            if (!PyWinObject_AsFILETIME(obData, &ftData))
                goto cleanup;
            // ???? MSDN claims that pvData should point directly to a FILETIME, but that results in an access violation
            // ????
            cdb.cbData = sizeof(ftData);
            cdb.pbData = (BYTE *)&ftData;
            pvData = &cdb;
            break;
        case CERT_DESCRIPTION_PROP_ID:    // @flag CERT_DESCRIPTION_PROP_ID|Unicode string
        case CERT_FRIENDLY_NAME_PROP_ID:  // @flag CERT_FRIENDLY_NAME_PROP_ID|Unicode string
        case CERT_PVK_FILE_PROP_ID:       // @flag CERT_PVK_FILE_PROP_ID|Unicode string
        case CERT_AUTO_ENROLL_PROP_ID:    // @flag CERT_AUTO_ENROLL_PROP_ID|Unicode string
            if (!PyWinObject_AsWCHAR(obData, (WCHAR **)&cdb.pbData, FALSE, &cdb.cbData))
                goto cleanup;
            // size is apparently in bytes, not characters
            cdb.cbData *= sizeof(WCHAR);
            pvData = &cdb;
            break;

        case CERT_KEY_SPEC_PROP_ID:  // @flag CERT_KEY_SPEC_PROP_ID|Int, usually AT_KEYEXCHANGE or AT_SIGNATURE
            dwData = PyLong_AsUnsignedLong(obData);
            if (dwData == (DWORD)-1 && PyErr_Occurred())
                goto cleanup;
            pvData = &dwData;
            break;
        // CERT_HASH_PROP_ID is same value as CERT_SHA1_HASH_PROP_ID
        // @flag CERT_HASH_PROP_ID|String containing the hash
        case CERT_SHA1_HASH_PROP_ID:       // @flag CERT_SHA1_HASH_PROP_ID|String containing the hash
        case CERT_MD5_HASH_PROP_ID:        // @flag CERT_MD5_HASH_PROP_ID|String containingg the hash
        case CERT_SIGNATURE_HASH_PROP_ID:  // @flag CERT_SIGNATURE_HASH_PROP_ID|String containing the hash
        case CERT_KEY_IDENTIFIER_PROP_ID:  // @flag CERT_KEY_IDENTIFIER_PROP_ID|String containing the key id
        case CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID:  // @flag CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID|String
                                                        // containing the hash
        case CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID:   // @flag CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID|String
                                                        // containing the hash
        case CERT_SUBJECT_NAME_MD5_HASH_PROP_ID:  // @flag CERT_SUBJECT_NAME_MD5_HASH_PROP_ID|String containing the hash
        case CERT_RENEWAL_PROP_ID:                // @flag CERT_RENEWAL_PROP_ID|String containing the hash
        // @flag CERT_ENHKEY_USAGE_PROP_ID|String containing an encoded <o PyCTL_USAGE>.  Use <om
        // cryptoapi.CryptEncodeObjectEx> with X509_ENHANCED_KEY_USAGE.
        // @flag CERT_CTL_USAGE_PROP_ID|Same as CERT_ENHKEY_USAGE_PROP_ID
        case CERT_CTL_USAGE_PROP_ID:
            if (!PyWinObject_AsReadBuffer(obData, (void **)&cdb.pbData, &cdb.cbData))
                goto cleanup;
            pvData = &cdb;
            break;
        /*
        case CERT_KEY_PROV_INFO_PROP_ID:  // CRYPT_KEY_PROV_INFO
            pvData=malloc(sizeof(CRYPT_KEY_PROV_INFO));
            if (pvData==NULL)
                return PyErr_NoMemory();
            if (!PyWinObject_AsCRYPT_KEY_PROV_INFO(obData, (PCRYPT_KEY_PROV_INFO)pvData))
                goto cleanup;
            break;
        */
        /*
        case CERT_KEY_PROV_HANDLE_PROP_ID:

        case CERT_KEY_CONTEXT_PROP_ID:	// CERT_KEY_CONTEXT
        case CERT_NEXT_UPDATE_LOCATION_PROP_ID:  // encoded CERT_ALT_NAME_INFO, only used with CTL
        case CERT_ENROLLMENT_PROP_ID:  // CRYPT_DATA_BLOB, data will apparently have to be split out manually
        case CERT_CROSS_CERT_DIST_POINTS_PROP_ID: // CRYPT_DATA_BLOB which contains encoded CROSS_CERT_DIST_POINTS_INFO
        (X509_CROSS_CERT_DIST_POINTS) case CERT_PUBKEY_ALG_PARA_PROP_ID:
        */
        default:
            PyErr_Format(PyExc_NotImplementedError, "Property Id %d is not supported yet", prop);
            goto cleanup;
    }

    Py_BEGIN_ALLOW_THREADS bsuccess = CertSetCertificateContextProperty(pccert_context, prop, flags, pvData);
    Py_END_ALLOW_THREADS if (!bsuccess) PyWin_SetAPIError("CertSetCertificateContextProperty");
    else
    {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

cleanup:
    switch (prop) {
        case CERT_DESCRIPTION_PROP_ID:
        case CERT_FRIENDLY_NAME_PROP_ID:
        case CERT_PVK_FILE_PROP_ID:
        case CERT_AUTO_ENROLL_PROP_ID:
            PyWinObject_FreeWCHAR((WCHAR *)cdb.pbData);
            break;
        // case CERT_KEY_PROV_INFO_PROP_ID:
        //	free(pvData);
        //	break;
        default:
            break;
    }
    return ret;
}
