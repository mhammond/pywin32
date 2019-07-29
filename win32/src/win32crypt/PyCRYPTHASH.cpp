// @doc
#include "win32crypt.h"

// @object PyCRYPTHASH|Handle to a cryptographic hash
struct PyMethodDef PyCRYPTHASH::methods[] = {
    // @pymeth CryptDestroyHash|Frees the hash object
    {"CryptDestroyHash", PyCRYPTHASH::PyCryptDestroyHash, METH_NOARGS},
    // @pymeth CryptDuplicateHash|Clones the hash object
    {"CryptDuplicateHash", (PyCFunction)PyCRYPTHASH::PyCryptDuplicateHash, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptHashData|Adds data to the hash
    {"CryptHashData", (PyCFunction)PyCRYPTHASH::PyCryptHashData, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptHashSessionKey|Hashes a session key
    {"CryptHashSessionKey", (PyCFunction)PyCRYPTHASH::PyCryptHashSessionKey, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptSignHash|Signs the hash
    {"CryptSignHash", (PyCFunction)PyCRYPTHASH::PyCryptSignHash, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptVerifySignature|Verifies that a signature matches hashed data
    {"CryptVerifySignature", (PyCFunction)PyCRYPTHASH::PyCryptVerifySignature, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CryptGetHashParam|Retrieves the specified attribute of the hash
    {"CryptGetHashParam", (PyCFunction)PyCRYPTHASH::PyCryptGetHashParam, METH_KEYWORDS | METH_VARARGS},
    {NULL}};

PyTypeObject PyCRYPTHASHType = {PYWIN_OBJECT_HEAD "PyCRYPTHASH",
                                sizeof(PyCRYPTHASH),
                                0,
                                PyCRYPTHASH::deallocFunc, /* tp_dealloc */
                                0,                        /* tp_print */
                                0,                        /* tp_getattr */
                                0,                        /* tp_setattr */
                                0,                        /* tp_compare */
                                0,                        /* tp_repr */
                                0,                        /* tp_as_number */
                                0,                        /* tp_as_sequence */
                                0,                        /* tp_as_mapping */
                                0,
                                0, /* tp_call */
                                0, /* tp_str */
                                PyCRYPTHASH::getattro,
                                PyCRYPTHASH::setattro,
                                0,                                         // PyBufferProcs *tp_as_buffer
                                Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
                                0,                                         // tp_doc
                                0,                                         // traverseproc tp_traverse
                                0,                                         // tp_clear
                                0,                                         // richcmpfunc tp_richcompare
                                0,                                         // tp_weaklistoffset
                                0,                                         // getiterfunc tp_iter
                                0,                                         // iternextfunc tp_iternext
                                PyCRYPTHASH::methods,
                                PyCRYPTHASH::members};

struct PyMemberDef PyCRYPTHASH::members[] = {
    {NULL} /* Sentinel */
};

int PyCRYPTHASH::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    return PyObject_GenericSetAttr(self, obname, v);
}

PyObject *PyCRYPTHASH::getattro(PyObject *self, PyObject *obname) { return PyObject_GenericGetAttr(self, obname); }

BOOL PyWinObject_AsHCRYPTHASH(PyObject *obhcrypthash, HCRYPTHASH *hcrypthash, BOOL bNoneOK)
{
    if (bNoneOK && (obhcrypthash == Py_None)) {
        *hcrypthash = NULL;
        return true;
    }
    if (obhcrypthash->ob_type != &PyCRYPTHASHType) {
        PyErr_SetString(PyExc_TypeError, "Object must be of type PyCRYPTHASH");
        return FALSE;
    }
    *hcrypthash = ((PyCRYPTHASH *)obhcrypthash)->GetHCRYPTHASH();
    return TRUE;
}

PyCRYPTHASH::~PyCRYPTHASH(void) { CryptDestroyHash(hcrypthash); }

void PyCRYPTHASH::deallocFunc(PyObject *ob) { delete (PyCRYPTHASH *)ob; }

PyCRYPTHASH::PyCRYPTHASH(HCRYPTHASH hcrypthash)
{
    ob_type = &PyCRYPTHASHType;
    _Py_NewReference(this);
    this->hcrypthash = hcrypthash;
}

// @pymethod |PyCRYPTHASH|CryptDestroyHash|Frees the hash object
PyObject *PyCRYPTHASH::PyCryptDestroyHash(PyObject *self, PyObject *args)
{
    // METH_NOARGS
    PyObject *ret = NULL;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();
    if (CryptDestroyHash(hcrypthash))
        ret = Py_None;
    else
        PyWin_SetAPIError("CryptDestroyHash");
    Py_XINCREF(ret);
    return ret;
}

// @pymethod <o PyCRYPTHASH>|PyCRYPTHASH|CryptDuplicateHash|Clones the hash object
PyObject *PyCRYPTHASH::PyCryptDuplicateHash(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Flags", NULL};
    PyObject *ret = NULL;
    HCRYPTHASH hcrypthash, hcrypthashdup;
    DWORD dwFlags = 0, dwReserved = 0;
    hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:CryptDuplicateHash", keywords,
                                     &dwFlags))  // @pyparm int|Flags|0|Reserved, use 0 if passed
        return NULL;
    if (CryptDuplicateHash(hcrypthash, &dwReserved, dwFlags, &hcrypthashdup))
        ret = new PyCRYPTHASH(hcrypthashdup);
    else
        PyWin_SetAPIError("CryptDuplicateHash");
    return ret;
}

// @pymethod |PyCRYPTHASH|CryptHashData|Adds data to the hash
PyObject *PyCRYPTHASH::PyCryptHashData(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Data", "Flags", NULL};
    DWORD dwFlags = 0;  // CRYPT_USERDATA or 0
    DWORD dwDataLen = 0;
    BYTE *pbData = NULL;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();
    PyObject *obdata;
    // @comm If Flags is CRYPT_USERDATA, provider is expected to prompt user to
    //   enter data.  MSDN says that MS CSPs ignore this flag
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CryptHashData", keywords,
                                     &obdata,    // @pyparm string|Data||Data to be hashed
                                     &dwFlags))  // @pyparm int|Flags|0|CRYPT_USERDATA or 0
        return NULL;
    if (!PyWinObject_AsReadBuffer(obdata, (void **)&pbData, &dwDataLen, FALSE))
        return NULL;
    if (dwFlags & CRYPT_USERDATA)
        dwDataLen = 0;
    if (CryptHashData(hcrypthash, pbData, dwDataLen, dwFlags)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWin_SetAPIError("CryptHashData");
}

// @pymethod |PyCRYPTHASH|CryptHashSessionKey|Hashes a session key
PyObject *PyCRYPTHASH::PyCryptHashSessionKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Key", "Flags", NULL};
    PyObject *obhcryptkey = NULL;
    HCRYPTKEY hcryptkey;
    DWORD dwFlags = 0;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CryptHashSessionKey", keywords,
                                     &obhcryptkey,  // @pyparm <o PyCRYPTKEY>|Key||The session key to be hashed
                                     &dwFlags))     // @pyparm int|Flags|0|CRYPT_LITTLE_ENDIAN or 0
        return NULL;
    if (!PyWinObject_AsHCRYPTKEY(obhcryptkey, &hcryptkey, FALSE))
        return NULL;
    if (CryptHashSessionKey(hcrypthash, hcryptkey, dwFlags)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWin_SetAPIError("CryptHashSessionKey");
}

// @pymethod string|PyCRYPTHASH|CryptSignHash|Signs the hash
// @comm This methods signs only the hash, not the data that the hash represents
PyObject *PyCRYPTHASH::PyCryptSignHash(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"KeySpec", "Flags", NULL};
    PyObject *ret = NULL;
    LPCTSTR sDescription = NULL;  // no longer used
    DWORD dwKeySpec = 0;          // AT_SIGNATURE or AT_KEYEXCHANGE
    DWORD dwFlags = 0;            // CRYPT_X931_FORMAT or CRYPT_NOHASHOID
    DWORD dwSigLen = 0;
    BYTE *pbSignature = NULL;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k|k:CryptSignHash", keywords,
            &dwKeySpec,  // @pyparm int|KeySpec||The key to be used to sign the hash, AT_KEYEXCHANGE,AT_SIGNATURE
            &dwFlags))   // @pyparm int|Flags|0|CRYPT_NOHASHOID,CRYPT_X931_FORMAT or 0
        return NULL;
    if (!CryptSignHash(hcrypthash, dwKeySpec, sDescription, dwFlags, pbSignature, &dwSigLen))
        PyWin_SetAPIError("CryptSignHash", GetLastError());
    pbSignature = (BYTE *)malloc(dwSigLen);
    if (pbSignature == NULL)
        return PyErr_Format(PyExc_MemoryError, "PyCRYPTHASH::CryptSignHash: Unable to allocate %d bytes", dwSigLen);
    if (!CryptSignHash(hcrypthash, dwKeySpec, sDescription, dwFlags, pbSignature, &dwSigLen))
        PyWin_SetAPIError("PyCRYPTHASH::CryptSignHash", GetLastError());
    else
        ret = PyString_FromStringAndSize((char *)pbSignature, dwSigLen);

    if (pbSignature != NULL)
        free(pbSignature);
    return ret;
}

// @pymethod |PyCRYPTHASH|CryptVerifySignature|Verifies that a signature matches hashed data
PyObject *PyCRYPTHASH::PyCryptVerifySignature(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Signature", "PubKey", "Flags", NULL};
    PyObject *obhcryptkey = NULL, *obsig;
    HCRYPTKEY hcryptkey;
    LPCTSTR sDescription = NULL;  // no longer used
    DWORD dwFlags = 0;            // CRYPT_X931_FORMAT or CRYPT_NOHASHOID
    DWORD dwSigLen = 0;
    BYTE *pbSignature = NULL;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|k:CryptVerifySignature", keywords,
                                     &obsig,        // @pyparm string|Signature||Signature data to verify
                                     &obhcryptkey,  // @pyparm <o PyCRYPTKEY>|PubKey||Public key of signer
                                     &dwFlags))     // @pyparm int|Flags|0|CRYPT_NOHASHOID,CRYPT_X931_FORMAT or 0
        return NULL;
    if (!PyWinObject_AsHCRYPTKEY(obhcryptkey, &hcryptkey, FALSE))
        return NULL;
    if (!PyWinObject_AsReadBuffer(obsig, (void **)&pbSignature, &dwSigLen, FALSE))
        return NULL;
    if (CryptVerifySignature(hcrypthash, pbSignature, dwSigLen, hcryptkey, sDescription, dwFlags)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWin_SetAPIError("PyCRYPTHASH::CryptVerifySignature");
}

// @pymethod int/str|PyCRYPTHASH|CryptGetHashParam|Retrieves the specified attribute of the hash
// @comm After this method has been called, no more data can be hashed
// @rdesc Type of returned object is dependent on the Param passed in
PyObject *PyCRYPTHASH::PyCryptGetHashParam(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Param", "Flags", NULL};
    DWORD param, buflen = 0, flags = 0;
    PBYTE buf = NULL;
    PyObject *ret = NULL;
    HCRYPTHASH hcrypthash = ((PyCRYPTHASH *)self)->GetHCRYPTHASH();

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k|k:CryptGetHashParam", keywords,
            &param,   // @pyparm int|Param||The parameter to retrieve: HP_ALGID, HP_HASHSIZE, or HP_HASHVAL
            &flags))  // @pyparm int|Flags|0|Reserved, use 0 if passed in
        return NULL;
    if (!CryptGetHashParam(hcrypthash, param, buf, &buflen, flags))
        return PyWin_SetAPIError("PyCRYPTHASH::CryptGetHashParam");
    buf = (PBYTE)malloc(buflen);
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen);
    if (!CryptGetHashParam(hcrypthash, param, buf, &buflen, flags))
        PyWin_SetAPIError("PyCRYPTHASH::CryptGetHashParam");
    else {
        switch (param) {
            case HP_ALGID:
            case HP_HASHSIZE:
                ret = PyLong_FromUnsignedLong(*((unsigned long *)buf));
                break;
            case HP_HASHVAL:
                ret = PyString_FromStringAndSize((char *)buf, buflen);
                break;
            default:
                PyErr_Format(PyExc_NotImplementedError, "Hash parameter %d is not yet supported", param);
        }
    }
    free(buf);
    return ret;
}
