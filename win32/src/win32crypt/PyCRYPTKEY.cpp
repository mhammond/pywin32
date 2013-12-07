// @doc
#include "win32crypt.h"

// @object PyCRYPTKEY|Handle to a cryptographic key
struct PyMethodDef PyCRYPTKEY::methods[] = {
	// @pymeth CryptDestroyKey|Releases the handle to the key
	{"CryptDestroyKey", PyCRYPTKEY::PyCryptDestroyKey, METH_NOARGS},
	// @pymeth CryptExportKey|Securely exports key or key pair
	{"CryptExportKey", (PyCFunction)PyCRYPTKEY::PyCryptExportKey, METH_KEYWORDS|METH_VARARGS}, 
	// @pymeth CryptGetKeyParam|Retrieves key parameters
	{"CryptGetKeyParam", (PyCFunction)PyCRYPTKEY::PyCryptGetKeyParam, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CryptDuplicateKey|Creates an independent copy of the key
	{"CryptDuplicateKey", (PyCFunction)PyCRYPTKEY::PyCryptDuplicateKey, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CryptEncrypt|Encrypts data
	{"CryptEncrypt", (PyCFunction)PyCRYPTKEY::PyCryptEncrypt, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CryptDecrypt|Decrypts data
	{"CryptDecrypt", (PyCFunction)PyCRYPTKEY::PyCryptDecrypt, METH_KEYWORDS|METH_VARARGS},
	{NULL}
};


PyTypeObject PyCRYPTKEYType =
{
	PYWIN_OBJECT_HEAD
	"PyCRYPTKEY",
	sizeof(PyCRYPTKEY),
	0,
	PyCRYPTKEY::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	0,		/* tp_getattr */
	0,		/* tp_setattr */
	0,		/* tp_compare */
	0,		/* tp_repr */
	0,		/* tp_as_number */
	0,		/* tp_as_sequence */
	0,		/* tp_as_mapping */
	0,
	0,		/* tp_call */
	0,		/* tp_str */
	PyCRYPTKEY::getattro,
	PyCRYPTKEY::setattro,
	0,			// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,			// tp_doc
	0,			// traverseproc tp_traverse
	0,			// tp_clear
	0,			// richcmpfunc tp_richcompare
	0,			// tp_weaklistoffset
	0,			// getiterfunc tp_iter
	0,			// iternextfunc tp_iternext
	PyCRYPTKEY::methods,
	PyCRYPTKEY::members
};

struct PyMemberDef PyCRYPTKEY::members[] = {
	// @prop int|HCRYPTPROV|CSP used by the key
	{"HCRYPTPROV", T_OBJECT, offsetof(PyCRYPTKEY, obcryptprov), READONLY, "CSP used by the key"},
	// @prop int|HCRYPTKEY|Plain integer handle to the key
	{"HCRYPTKEY", T_OBJECT, offsetof(PyCRYPTKEY, obcryptkey), READONLY, "Plain integer handle to the key"},
	{NULL}	/* Sentinel */
};

int PyCRYPTKEY::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	return PyObject_GenericSetAttr(self, obname, v);
}

PyObject *PyCRYPTKEY::getattro(PyObject *self, PyObject *obname)
{
	/*
	char *name=PYWIN_ATTR_CONVERT(obname);
	if (name==NULL)
		return NULL;
	if (strcmp(name,"HCRYPTKEY")==0){
		HCRYPTKEY h=((PyCRYPTKEY *)self)->GetHCRYPTKEY();
		return PyLong_FromVoidPtr((void *)h);
		}
	*/
	return PyObject_GenericGetAttr(self,obname);
}

BOOL PyWinObject_AsHCRYPTKEY(PyObject *obhcryptkey, HCRYPTKEY *hcryptkey, BOOL bNoneOK)
{
	if (bNoneOK && (obhcryptkey==Py_None)){
		*hcryptkey=NULL;
		return true;
		}
	if (obhcryptkey->ob_type!=&PyCRYPTKEYType){
		PyErr_SetString(PyExc_TypeError,"Object must be of type PyCRYPTKEY");
		return FALSE;
		}
	*hcryptkey=((PyCRYPTKEY *)obhcryptkey)->GetHCRYPTKEY();
	return TRUE;
}

PyCRYPTKEY::~PyCRYPTKEY(void)
{
	Py_XDECREF(this->obcryptprov);
	Py_XDECREF(this->obcryptkey);
	CryptDestroyKey(hcryptkey);
}

void PyCRYPTKEY::deallocFunc(PyObject *ob)
{
	delete (PyCRYPTKEY *)ob;
}

PyCRYPTKEY::PyCRYPTKEY(HCRYPTKEY h, PyObject *obcryptprov)
{
	ob_type = &PyCRYPTKEYType;
	_Py_NewReference(this);
	this->hcryptkey=h;
	this->obcryptprov=obcryptprov;
	Py_INCREF(obcryptprov);
	this->obcryptkey=PyLong_FromVoidPtr((void *)h);
	this->obdummy=NULL;
}

// @pymethod |PyCRYPTKEY|CryptDestroyKey|Releases the handle to the key (does not delete permanent keys)
PyObject *PyCRYPTKEY::PyCryptDestroyKey(PyObject *self, PyObject *args)
{	
	HCRYPTKEY hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();
	if(!CryptDestroyKey(hcryptkey))
		return PyWin_SetAPIError("CryptDestroyKey");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod str|PyCRYPTKEY|CryptExportKey|Exports key or key pair as an encrypted blob
// @rdesc Returns a binary blob that can be imported via <om PyCRYPTPROV::CryptImportKey>
PyObject *PyCRYPTKEY::PyCryptExportKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ExpKey", "BlobType", "Flags", NULL};
	DWORD dwFlags=0, dwBlobType=0, dwDataLen=0;
	PyObject *obhcryptkeyexp=NULL, *ret=NULL;
	BYTE *pbData=NULL;
	HCRYPTKEY hcryptkey, hcryptkeyexp;
	hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|k:CryptExportKey", keywords,
		&obhcryptkeyexp,	// @pyparm <o PyCRYPTKEY>|ExpKey||Public key or session key of destination user.  Use None if exporting a PUBLICKEYBLOB
		&dwBlobType,		// @pyparm int|BlobType||One of OPAQUEKEYBLOB,PRIVATEKEYBLOB,PUBLICKEYBLOB,SIMPLEBLOB,PLAINTEXTKEYBLOB,SYMMETRICWRAPKEYBLOB 
		&dwFlags))			// @pyparm int|Flags|0|Combination of CRYPT_DESTROYKEY,CRYPT_SSL2_FALLBACK,CRYPT_OAEP or 0
		return NULL;
	if (!PyWinObject_AsHCRYPTKEY(obhcryptkeyexp, &hcryptkeyexp, TRUE))
		return NULL;
	if (!CryptExportKey(hcryptkey, hcryptkeyexp, dwBlobType, dwFlags, NULL, &dwDataLen)){
		PyWin_SetAPIError("CryptExportKey");
		return NULL;
		}
	pbData=(BYTE *)malloc(dwDataLen);
	if (pbData==NULL)
		return PyErr_Format(PyExc_MemoryError, "PyCRYPTKEY::CryptExportKey: Unable to allocate %d bytes", dwDataLen);
	if (CryptExportKey(hcryptkey, hcryptkeyexp, dwBlobType, dwFlags, pbData, &dwDataLen))
		ret=PyString_FromStringAndSize((char *)pbData,dwDataLen);
	else
		PyWin_SetAPIError("CryptExportKey");
	if (pbData != NULL)
		free(pbData);
	return ret;
}


// @pymethod object|PyCRYPTKEY|CryptGetKeyParam|Retrieves key parameters
// @rdesc Type of returned object is dependent on the requested attribute
PyObject *PyCRYPTKEY::PyCryptGetKeyParam(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Param", "Flags", NULL};
	PyObject *ret=NULL;
	DWORD dwFlags=0, dwParam=0, dwDataLen=0;
	BYTE *pbData = NULL;
	HCRYPTKEY hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k|k:CryptGetKeyParam", keywords,
		&dwParam,	// @pyparm int|Param||One of the KP_* constants
		&dwFlags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!CryptGetKeyParam(hcryptkey, dwParam, pbData, &dwDataLen, dwFlags))
		return PyWin_SetAPIError("CryptGetKeyParam");

	pbData=(BYTE *)malloc(dwDataLen);
	if (pbData==NULL)
		return PyErr_Format(PyExc_MemoryError, "PyCRYPTKEY::CryptGetKeyParam: Unable to allocate %d bytes", dwDataLen);

	if (!CryptGetKeyParam(hcryptkey, dwParam, pbData, &dwDataLen, dwFlags)){
		PyWin_SetAPIError("CryptGetKeyParam",GetLastError());
		goto done;
		}

	switch (dwParam){
		case KP_ALGID:
		case KP_MODE:
		case KP_MODE_BITS:
		case KP_EFFECTIVE_KEYLEN:
		case KP_BLOCKLEN:
		case KP_PERMISSIONS:
		case KP_PADDING:
		case KP_KEYLEN:
			ret=Py_BuildValue("l",*((DWORD *)pbData));
			break;
		case KP_P:
		case KP_Q:
		case KP_G:
		case KP_IV:
		case KP_SALT:
			ret=PyString_FromStringAndSize((char *)pbData,dwDataLen);
			break;
		default:
			PyErr_SetString(PyExc_NotImplementedError, "The Param specified is not yet supported");
			break;
		}
	done:
	if (pbData != NULL)
		free(pbData);
	return ret;
}

// @pymethod <o PyCRYPTKEY>|PyCRYPTKEY|CryptDuplicateKey|Creates an independent copy of the key
PyObject *PyCRYPTKEY::PyCryptDuplicateKey(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Reserved", "Flags", NULL};
	PyObject *ret=NULL;
	DWORD dwFlags=0, dwReserved=0;
	HCRYPTKEY hcryptkey, hcryptkeydup;
	hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|kk:CryptDuplicateKey", keywords,
		&dwReserved,	// @pyparm int|Reserved|0|Use 0 if passed in
		&dwFlags))		// @pyparm int|Flags|0|Also reserved, use 0
		return NULL;
	if (CryptDuplicateKey(hcryptkey, &dwReserved, dwFlags, &hcryptkeydup))
		ret = new PyCRYPTKEY(hcryptkeydup, ((PyCRYPTKEY *)self)->obcryptprov);
	else
		PyWin_SetAPIError("CryptDuplicateKey",GetLastError());
	return ret;
}

// @pymethod str|PyCRYPTKEY|CryptEncrypt|Encrypts and optionally hashes data
PyObject *PyCRYPTKEY::PyCryptEncrypt(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Final", "Data", "Hash", "Flags", NULL};
	PyObject *obdata, *ret=NULL, *obcrypthash=Py_None;
	BOOL Final;
	DWORD err=0, bytes_to_encrypt=0, dwFlags=0, dwDataLen=0, dwBufLen=0;
	BYTE *pbData=NULL, *origdata;
	HCRYPTHASH hcrypthash=NULL;
	HCRYPTKEY hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "lO|Ok:CryptEncrypt", keywords,
		&Final,			// @pyparm int|Final||Boolean, use True if this is final encryption operation		
		&obdata,		// @pyparm buffer|Data||Data to be encrypted
		&obcrypthash,	// @pyparm <o PyCRYPTHASH>|Hash|None|Hash to be updated with data passed in, can be None
		&dwFlags))		// @pyparm int|Flags|0|Reserved, use 0 if passed in
		return NULL;
	if (!PyWinObject_AsHCRYPTHASH(obcrypthash, &hcrypthash, TRUE))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obdata, (void **)&origdata, &bytes_to_encrypt, FALSE))
		return NULL;
	dwDataLen=bytes_to_encrypt;    // read/write - receives bytes needed for encrypted data
	dwBufLen=bytes_to_encrypt;

	// First call to get required buffer size - don't pass hash, or it will be updated twice
	if (!CryptEncrypt(hcryptkey, NULL, Final, dwFlags, NULL, &dwDataLen, dwBufLen))
		return PyWin_SetAPIError("CryptEncrypt");
	pbData=(BYTE *)malloc(dwDataLen);
	if (pbData==NULL)
		return PyErr_NoMemory();
	memcpy(pbData,origdata,bytes_to_encrypt);
	dwBufLen=dwDataLen;
	dwDataLen=bytes_to_encrypt;
	if (!CryptEncrypt(hcryptkey, hcrypthash, Final, dwFlags, pbData, &dwDataLen, dwBufLen))
		PyWin_SetAPIError("CryptEncrypt");
	else
		ret=PyString_FromStringAndSize((char *)pbData, dwDataLen);
	free(pbData);
	return ret;
}
		
// @pymethod str|PyCRYPTKEY|CryptDecrypt|Decrypts data
PyObject *PyCRYPTKEY::PyCryptDecrypt(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Final", "Data", "Hash", "Flags", NULL};
	PyObject *obdata, *ret=NULL, *obcrypthash=Py_None;
	BOOL Final;
	DWORD err=0, bytes_to_decrypt=0, dwFlags=0, dwDataLen=0;
	BYTE *pbData=NULL, *origdata=NULL;
	HCRYPTHASH hcrypthash=NULL;
	HCRYPTKEY hcryptkey=((PyCRYPTKEY *)self)->GetHCRYPTKEY();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "lO|Ok:CryptDecrypt", keywords,
		&Final,			// @pyparm int|Final||Boolean, use True is this is last (or only) operation
		&obdata,		// @pyparm buffer|Data||Data to be decrypted
		&obcrypthash,	// @pyparm <o PyCRYPTHASH>|Hash|None|Hash to be used in signature verification, can be None
		&dwFlags))		// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!PyWinObject_AsHCRYPTHASH(obcrypthash, &hcrypthash, TRUE))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obdata, (void **)&origdata, &bytes_to_decrypt, FALSE))
		return NULL;

	// data buffer is read-write, do not pass in python's buffer
	pbData=(BYTE *)malloc(bytes_to_decrypt);
	if (pbData==NULL)
		return PyErr_NoMemory();
	memcpy(pbData,origdata,bytes_to_decrypt);
	dwDataLen=bytes_to_decrypt;    // read/write - receives length of plaintext
	// Due to padding, should never occur that buffer needed for plaintext is larger than encrypted data
	if (!CryptDecrypt(hcryptkey, hcrypthash, Final, dwFlags, pbData, &dwDataLen))
		PyWin_SetAPIError("CryptDecrypt");
	else
		ret=PyString_FromStringAndSize((char *)pbData, dwDataLen);
	free(pbData);
	return ret;
}
