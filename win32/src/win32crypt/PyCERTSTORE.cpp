// @doc
#include "win32crypt.h"

// @object PyCERTSTORE|Handle to a certificate store
struct PyMethodDef PyCERTSTORE::methods[] = {
	// @pymeth CertCloseStore|Closes the certificate store
	{"CertCloseStore", (PyCFunction)PyCERTSTORE::PyCertCloseStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertControlStore|Controls sychronization of the certificate store
	{"CertControlStore", (PyCFunction)PyCERTSTORE::PyCertControlStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertEnumCertificatesInStore|Lists all certificates in the store
	{"CertEnumCertificatesInStore", PyCERTSTORE::PyCertEnumCertificatesInStore, METH_NOARGS},
	// @pymeth CertEnumCTLsInStore|Finds all Certificate Trust Lists in store.
	{"CertEnumCTLsInStore", PyCERTSTORE::PyCertEnumCTLsInStore, METH_NOARGS},
	// @pymeth CertSaveStore|Serializes the store to memory or a file
	{"CertSaveStore", (PyCFunction)PyCERTSTORE::PyCertSaveStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddEncodedCertificateToStore|Imports an encoded certificate into the store
	{"CertAddEncodedCertificateToStore", (PyCFunction)PyCERTSTORE::PyCertAddEncodedCertificateToStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddCertificateContextToStore|Adds a certificate context to the store
	{"CertAddCertificateContextToStore", (PyCFunction)PyCERTSTORE::PyCertAddCertificateContextToStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddCertificateLinkToStore|Adds a link to a cert in another store
	{"CertAddCertificateLinkToStore", (PyCFunction)PyCERTSTORE::PyCertAddCertificateLinkToStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddCTLContextToStore|Adds a certificate trust list to the store
	{"CertAddCTLContextToStore", (PyCFunction)PyCERTSTORE::PyCertAddCTLContextToStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddCTLLinkToStore|Adds a link to a CTL in another store
	{"CertAddCTLLinkToStore", (PyCFunction)PyCERTSTORE::PyCertAddCTLLinkToStore, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertAddStoreToCollection|Adds a sibling store to a store collection
	{"CertAddStoreToCollection", (PyCFunction)PyCERTSTORE::PyCertAddStoreToCollection, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CertRemoveStoreFromCollection|Removes a sibling store from a store collection
	{"CertRemoveStoreFromCollection", (PyCFunction)PyCERTSTORE::PyCertRemoveStoreFromCollection, METH_KEYWORDS|METH_VARARGS},
	// @pymeth PFXExportCertStoreEx|Exports certificates and associated private keys in PKCS#12 format
	{"PFXExportCertStoreEx", (PyCFunction)PyCERTSTORE::PyPFXExportCertStoreEx, METH_KEYWORDS|METH_VARARGS},
	// xxxpymeth CertGetStoreProperty|Retrieves an attribute of the store
	// {"CertGetStoreProperty", (PyCFunction)PyCERTSTORE::PyCertGetStoreProperty, METH_KEYWORDS|METH_VARARGS},
	// xxxpymeth CertSetStoreProperty|Sets a property of the certificate store
	// {"CertSetStoreProperty", (PyCFunction)PyCERTSTORE::PyCertSetStoreProperty, METH_KEYWORDS|METH_VARARGS},
	{NULL}
};


PyTypeObject PyCERTSTOREType =
{
	PYWIN_OBJECT_HEAD
	"PyCERTSTORE",
	sizeof(PyCERTSTORE),
	0,
	PyCERTSTORE::deallocFunc,	// tp_dealloc
	0,							// tp_print
	0,							// tp_getattr
	0,							// tp_setattr
	0,							// tp_compare
	0,							// tp_repr
	0,							// tp_as_number
	0,							// tp_as_sequence
	0,							// tp_as_mapping
	0,
	0,							// tp_call
	0,							// tp_str
	PyCERTSTORE::getattro,
	PyCERTSTORE::setattro,
	0,							// PyBufferProcs *tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
	0,							// tp_doc
	0,							// traverseproc tp_traverse
	0,							// tp_clear
	0,							// richcmpfunc tp_richcompare
	0,							// tp_weaklistoffset
	0,							// getiterfunc tp_iter
	0,							// iternextfunc tp_iternext
	PyCERTSTORE::methods,
	PyCERTSTORE::members
};

struct PyMemberDef PyCERTSTORE::members[] = {
	// @prop int|HCERTSTORE|Integer handle
	{"HCERTSTORE", T_OBJECT, offsetof(PyCERTSTORE, obcertstore), READONLY, "Integer handle"},
	{NULL}
};

int PyCERTSTORE::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
	return PyObject_GenericSetAttr(self, obname,v);
}

PyObject *PyCERTSTORE::getattro(PyObject *self, PyObject *obname)
{
	return PyObject_GenericGetAttr(self,obname);
}

PyCERTSTORE::~PyCERTSTORE(void)
{
	if (hcertstore)
		CertCloseStore(hcertstore, 0);
	Py_XDECREF(obcertstore);
}

void PyCERTSTORE::deallocFunc(PyObject *ob)
{
	delete (PyCERTSTORE *)ob;
}

PyCERTSTORE::PyCERTSTORE(HCERTSTORE h)
{
	ob_type = &PyCERTSTOREType;
	_Py_NewReference(this);
	this->hcertstore=h;
	this->obcertstore=PyLong_FromVoidPtr((void *)h);
}

BOOL PyWinObject_AsCERTSTORE(PyObject *obhcertstore, HCERTSTORE *hcertstore, BOOL bNoneOK)
{
	if (bNoneOK && (obhcertstore==Py_None)){
		*hcertstore=NULL;
		return true;
		}
	if (obhcertstore->ob_type!=&PyCERTSTOREType){
		PyErr_SetString(PyExc_TypeError,"Object must be of type PyCERTSTORE");
		return FALSE;
		}
	*hcertstore=((PyCERTSTORE *)obhcertstore)->GetHCERTSTORE();
	return TRUE;
}

PyObject *PyWinObject_FromCERTSTORE(HCERTSTORE certstore)
{
	if (certstore==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	PyObject *ret = new PyCERTSTORE(certstore);
	if (ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "PyWinObject_FromCERTSTORE: Unable to allocate PyCERTSTORE");
	return ret;
}

// @pymethod |PyCERTSTORE|CertCloseStore|Closes the certificate store
PyObject *PyCERTSTORE::PyCertCloseStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Flags", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	DWORD dwFlags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|k:PyCERTSTORE::CertCloseStore", keywords,
		&dwFlags))	// @pyparm int|Flags|0|Combination of CERT_CLOSE_*_FLAG flags
		return NULL;
	if (hcertstore == NULL){
		PyErr_SetString(PyExc_SystemError, "Certificate store is already closed");
		return NULL;
		}
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertCloseStore(hcertstore, dwFlags);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("PyCERTSTORE::CertCloseStore");
	((PyCERTSTORE *)self)->hcertstore = NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyCERTSTORE|CertControlStore|Controls sychronization of the certificate store
PyObject *PyCERTSTORE::PyCertControlStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Flags", "CtrlType", "CtrlPara", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	DWORD dwFlags=0, dwCtrlType=0;
	HANDLE hevent=NULL;
	PyObject *obCtrlPara=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kkO:CertControlStore", keywords,
		&dwFlags,		// @pyparm int|Flags||One of the CERT_STORE_CTRL_*_FLAG flags
		&dwCtrlType,	// @pyparm int|CtrlType||One of the CERT_STORE_CTRL_* flags
		&obCtrlPara))	// @pyparm <o PyHANDLE>|CtrlPara||Event handle, can be None (not used with CERT_STORE_CTRL_COMMIT)
		return NULL;
	if (!PyWinObject_AsHANDLE(obCtrlPara, &hevent))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
    bsuccess = CertControlStore(hcertstore, dwFlags, dwCtrlType, hevent);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertControlStore");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod [<o PyCERT_CONTEXT>,...]|PyCERTSTORE|CertEnumCertificatesInStore|Lists all certificates in the store
PyObject *PyCERTSTORE::PyCertEnumCertificatesInStore(PyObject *self, PyObject *args)
{
	// METH_NOARGS
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCERT_CONTEXT pccert_context=NULL, py_pccert_context=NULL;
	PyObject *ret_item=NULL;
	DWORD err=0;
	PyObject *ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	do{
		Py_BEGIN_ALLOW_THREADS
		pccert_context=CertEnumCertificatesInStore(hcertstore, pccert_context);
		Py_END_ALLOW_THREADS
		if (pccert_context!=NULL){
			// increments reference count
			py_pccert_context=CertDuplicateCertificateContext(pccert_context);
			ret_item=PyWinObject_FromCERT_CONTEXT(py_pccert_context);
			if ((ret_item==NULL) || (PyList_Append(ret,ret_item)==-1)){
				Py_XDECREF(ret_item);
				Py_DECREF(ret);
				return NULL;
				}
			Py_DECREF(ret_item);
			}
		}
	while (pccert_context!=NULL);

	// Docs say this will return CRYPT_E_NOT_FOUND (0x80092004), but 0x00000002 (file not found) is returned on Win2K
	err=GetLastError();
	if ((err != CRYPT_E_NOT_FOUND) && (err !=ERROR_FILE_NOT_FOUND)){
		PyWin_SetAPIError("CertEnumCertificatesInStore", err);
		Py_DECREF(ret);
		ret=NULL;
		}
	return ret;
}

// @pymethod [<o PyCTL_CONTEXT>,...]|PyCERTSTORE|CertEnumCTLsInStore|Finds all Certificate Trust Lists in store
PyObject *PyCERTSTORE::PyCertEnumCTLsInStore(PyObject *self, PyObject *args)
{
	// METH_NOARGS
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCTL_CONTEXT ctl_context=NULL, py_ctl_context=NULL;
	PyObject *ret_item=NULL;
	DWORD err=0;
	PyObject *ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	do{
		Py_BEGIN_ALLOW_THREADS
		ctl_context=CertEnumCTLsInStore(hcertstore, ctl_context);
		Py_END_ALLOW_THREADS
		if (ctl_context!=NULL){
			py_ctl_context=CertDuplicateCTLContext(ctl_context);
			ret_item=PyWinObject_FromCTL_CONTEXT(py_ctl_context);
			if ((ret_item==NULL) || (PyList_Append(ret,ret_item)==-1)){
				Py_XDECREF(ret_item);
				Py_DECREF(ret);
				return NULL;
				}
			Py_DECREF(ret_item);
			}
		}
	while (ctl_context!=NULL);

	// Docs say this will return CRYPT_E_NOT_FOUND (0x80092004), but 0x00000002 (file not found) is returned on Win2K
	err=GetLastError();
	if ((err != CRYPT_E_NOT_FOUND) && (err !=ERROR_FILE_NOT_FOUND)){
		PyWin_SetAPIError("CertEnumCTLsInStore", err);
		Py_DECREF(ret);
		ret=NULL;
		}
	return ret;
}

// @pymethod |PyCERTSTORE|CertSaveStore|Serializes the store to memory or a file
PyObject *PyCERTSTORE::PyCertSaveStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"MsgAndCertEncodingType", "SaveAs", "SaveTo", "SaveToPara", "Flags", NULL};
	DWORD dwMsgAndCertEncodingType=0, dwSaveAs=0, dwSaveTo=0, dwFlags=0;
	void* pvSaveToPara=NULL;
	PyObject *obpvSaveToPara=NULL, *obfile_name=NULL, *ret=NULL;
	WCHAR *file_name=NULL;
	HANDLE file_handle=NULL;
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kkkO|k:PyCERTSTORE::CertSaveStore", keywords,
		&dwMsgAndCertEncodingType,	// @pyparm int|MsgAndCertEncodingType||Only used when saveas is CERT_STORE_SAVE_AS_PKCS7 - usually X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING
		&dwSaveAs,					// @pyparm int|SaveAs||One of the CERT_STORE_SAVE_AS_* constants
		&dwSaveTo,					// @pyparm int|SaveTo||One of the CERT_STORE_SAVE_TO_* constants (CERT_STORE_SAVE_TO_MEMORY not supported yet)
		&obpvSaveToPara,			// @pyparm <o PyHANDLE>/string|SaveToPara||File name or open file handle depending on SaveTo parm
		&dwFlags))					// @pyparm int|Flags|0|Reserved, use 0
		return NULL;
	switch(dwSaveTo){
		case CERT_STORE_SAVE_TO_FILENAME:{
			if (!PyWinObject_AsWCHAR(obpvSaveToPara, &file_name, FALSE))
				return NULL;
			pvSaveToPara=(void *)file_name;
			break;
			}
		case CERT_STORE_SAVE_TO_FILE:{
			if (!PyWinObject_AsHANDLE(obpvSaveToPara, &file_handle))
				return NULL;
			pvSaveToPara=(void *)file_handle;
			break;
			}
		default:{
			PyErr_SetString(PyExc_NotImplementedError,"CertSaveStore: specified SaveTo parameter is not supported yet");
			return NULL;
			}
		}
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertSaveStore(hcertstore, dwMsgAndCertEncodingType, dwSaveAs, dwSaveTo, pvSaveToPara, dwFlags);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		PyWin_SetAPIError("PyCERTSTORE::CertSaveStore");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if (file_name!=NULL)
		PyWinObject_FreeWCHAR(file_name);
	return ret;
}

// @pymethod <o PyCERT_CONTEXT>|PyCERTSTORE|CertAddEncodedCertificateToStore|Imports an encoded certificate into the store
PyObject *PyCERTSTORE::PyCertAddEncodedCertificateToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CertEncodingType", "CertEncoded", "AddDisposition", NULL};
	DWORD dwCertEncodingType=0;
	BYTE *pbCertEncoded=NULL;
	DWORD cbCertEncoded=0, dwAddDisposition=0;
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCERT_CONTEXT newcert_context=NULL;
	PyObject *obbuf;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kOk:CertAddEncodedCertificateToStore", keywords,
		&dwCertEncodingType,	// @pyparm int|CertEncodingType||Usually X509_ASN_ENCODING combined with PKCS_7_ASN_ENCODING 
		&obbuf,					// @pyparm buffer|CertEncoded||Data containing a serialized certificate
		&dwAddDisposition))		// @pyparm int|AddDisposition||Combination of CERT_STORE_ADD_* flags
		return NULL;
	if (!PyWinObject_AsReadBuffer(obbuf, (void **)&pbCertEncoded, &cbCertEncoded, FALSE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddEncodedCertificateToStore(hcertstore, dwCertEncodingType, pbCertEncoded, 
		   cbCertEncoded, dwAddDisposition, (const struct _CERT_CONTEXT **)&newcert_context);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("PyCERTSTORE::CertAddEncodedCertificateToStore");
	return PyWinObject_FromCERT_CONTEXT(newcert_context);
}

// @pymethod <o PyCERT_CONTEXT>|PyCERTSTORE|CertAddCertificateContextToStore|Adds a certificate context to the store
PyObject *PyCERTSTORE::PyCertAddCertificateContextToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CertContext", "AddDisposition", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCERT_CONTEXT pcert_context=NULL, newcert_context=NULL;
	DWORD dwAddDisposition;
	PyObject *obcertcontext;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CertAddCertificateContextToStore", keywords,
		&obcertcontext,		// @pyparm <o PyCERT_CONTEXT>|CertContext||Certificate context to be added
		&dwAddDisposition))	// @pyparm int|AddDisposition||CERT_STORE_ADD_* constant
		return NULL;
	if (!PyWinObject_AsCERT_CONTEXT(obcertcontext, &pcert_context, FALSE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddCertificateContextToStore(hcertstore, pcert_context, dwAddDisposition, &newcert_context);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertAddCertificateContextToStore");
	return PyWinObject_FromCERT_CONTEXT(newcert_context);
}

// @pymethod <o PyCERT_CONTEXT>|PyCERTSTORE|CertAddCertificateLinkToStore|Adds a link to a cert in another store
PyObject *PyCERTSTORE::PyCertAddCertificateLinkToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CertContext", "AddDisposition", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCERT_CONTEXT pcert_context=NULL, newcert_context=NULL;
	DWORD dwAddDisposition;
	PyObject *obcertcontext;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CertAddCertificateLinkToStore", keywords,
		&obcertcontext,		// @pyparm <o PyCERT_CONTEXT>|CertContext||Certificate context to be linked
		&dwAddDisposition))	// @pyparm int|AddDisposition||One of the CERT_STORE_ADD_* values
		return NULL;
	if (!PyWinObject_AsCERT_CONTEXT(obcertcontext, &pcert_context, FALSE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddCertificateLinkToStore(hcertstore, pcert_context, dwAddDisposition, &newcert_context);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertAddCertificateLinkToStore");
	return PyWinObject_FromCERT_CONTEXT(newcert_context);
}

// @pymethod <o PyCTL_CONTEXT>|PyCERTSTORE|CertAddCTLContextToStore|Adds a certificate trust list to the store
PyObject *PyCERTSTORE::PyCertAddCTLContextToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CtlContext", "AddDisposition", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCTL_CONTEXT pctl=NULL, new_pctl=NULL;
	DWORD dwAddDisposition;
	PyObject *obctl;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CertAddCTLContextToStore", keywords,
		&obctl,				// @pyparm <o PyCTL_CONTEXT>|CtlContext||CTL to be added
		&dwAddDisposition))	// @pyparm int|AddDisposition||CERT_STORE_ADD_* constant
		return NULL;
	if (!PyWinObject_AsCTL_CONTEXT(obctl, &pctl, FALSE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddCTLContextToStore(hcertstore, pctl, dwAddDisposition, &new_pctl);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertAddCTLContextToStore");
	return PyWinObject_FromCTL_CONTEXT(new_pctl);
}

// @pymethod <o PyCTL_CONTEXT>|PyCERTSTORE|CertAddCTLLinkToStore|Adds a link to a CTL in another store
PyObject *PyCERTSTORE::PyCertAddCTLLinkToStore(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CtlContext", "AddDisposition", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	PCCTL_CONTEXT pctl, new_pctl;
	DWORD dwAddDisposition;
	PyObject *obctl;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CertAddCTLLinkToStore", keywords,
		&obctl,				// @pyparm <o PyCTL_CONTEXT>|CtlContext||CTL to be linked
		&dwAddDisposition))	// @pyparm int|AddDisposition||One of the CERT_STORE_ADD_* values
		return NULL;
	if (!PyWinObject_AsCTL_CONTEXT(obctl, &pctl, FALSE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddCTLLinkToStore(hcertstore, pctl, dwAddDisposition, &new_pctl);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertAddCTLLinkToStore");
	return PyWinObject_FromCTL_CONTEXT(new_pctl);
}

// @pymethod |PyCERTSTORE|CertAddStoreToCollection|Adds a sibling store to a store collection
// @comm A collection store is created by using <om cryptoapi.CertOpenStore> with CERT_STORE_PROV_COLLECTION
PyObject *PyCERTSTORE::PyCertAddStoreToCollection(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"SiblingStore", "UpdateFlag", "Priority", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	HCERTSTORE sibling;
	DWORD flags=0, priority=0;
	PyObject *obsibling;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kk:CertAddStoreToCollection", keywords,
		&obsibling,	// @pyparm <o PyCERTSTORE>|SiblingStore||Store to be added to the collection
		&flags,		// @pyparm int|UpdateFlag|0|Can be CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG to enable changes to persist  
		&priority))	// @pyparm int|Priority|0|Determines order in which store are searched and updated
		return NULL;
	if (!PyWinObject_AsCERTSTORE(obsibling, &sibling, TRUE))
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertAddStoreToCollection(hcertstore, sibling, flags, priority);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertAddStoreToCollection");

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyCERTSTORE|CertRemoveStoreFromCollection|Removes a sibling store from a collection
PyObject *PyCERTSTORE::PyCertRemoveStoreFromCollection(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"SiblingStore", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	HCERTSTORE sibling;
	PyObject *obsibling;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CertRemoveStoreFromCollection", keywords,
		&obsibling))	// @pyparm <o PyCERTSTORE>|SiblingStore||Store to be removed from the collection
		return NULL;
	if (!PyWinObject_AsCERTSTORE(obsibling, &sibling, TRUE))
		return NULL;
	// does not return a value
	Py_BEGIN_ALLOW_THREADS
	CertRemoveStoreFromCollection(hcertstore, sibling);
	Py_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod bytes|PyCERTSTORE|PFXExportCertStoreEx|Exports certificates and associated private keys in PKCS#12 format
PyObject *PyCERTSTORE::PyPFXExportCertStoreEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Password", "Flags", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	DWORD flags = EXPORT_PRIVATE_KEYS|REPORT_NO_PRIVATE_KEY|REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY;
	PyObject *obpassword = Py_None;
	TmpWCHAR password;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Ok:PFXExportCertStoreEx", keywords,
		&obpassword,	// @pyparm str|Password|None|Passphrase to be used to encrypt the output
		&flags))		// @pyparm int|Flags|EXPORT_PRIVATE_KEYS\|REPORT_NO_PRIVATE_KEY\|REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY|Options to be used while exporting
		return NULL;
	if (!PyWinObject_AsWCHAR(obpassword, &password, TRUE))
		return NULL;
	CRYPT_DATA_BLOB out = {0};

	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = PFXExportCertStoreEx(hcertstore, &out, password, NULL, flags);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("PFXExportCertStoreEx");
	out.pbData = (BYTE *)malloc(out.cbData);
	if (out.pbData == NULL)
		return PyErr_NoMemory();

	PyObject *ret = NULL;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = PFXExportCertStoreEx(hcertstore, &out, password, NULL, flags);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		PyWin_SetAPIError("PFXExportCertStoreEx");
	else
		ret = PyString_FromStringAndSize((char *)out.pbData, out.cbData);
	free(out.pbData);
	return ret;
}



/*
// @pymethod |PyCERTSTORE|CertSetStoreProperty|Sets a property of the cerficate store
PyObject *PyCERTSTORE::PyCertSetStoreProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PropId", "Data", "Flags", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	DWORD flags = 0, propid;
	PyObject *obdata;
	CRYPT_DATA_BLOB cdb;
	BOOL bsuccess;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kO|k:CertSetStoreProperty", keywords,
		&propid,	// @pyparm int|PropId||Id of the property to set, CERT_STORE_LOCALIZED_NAME_PROP_ID or user-defined property id
		&obdata,	// @pyparm buffer|Data||The value for the property, use None to delete the property
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (propid == CERT_STORE_LOCALIZED_NAME_PROP_ID){
		if (!PyWinObject_AsWCHAR(obdata, (WCHAR **)&cdb.pbData, TRUE, &cdb.cbData))
			return NULL;
		cdb.cbData += 1;
		cdb.cbData *= sizeof(WCHAR);
		}
	else
		if (!PyWinObject_AsReadBuffer(obdata, (void **)&cdb.pbData, &cdb.cbData, TRUE))
			return NULL;

	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertSetStoreProperty(hcertstore, propid, flags, &cdb);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertSetStoreProperty");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod str|PyCERTSTORE|CertGetStoreProperty|Retrieves a property of the cerficate store
PyObject *PyCERTSTORE::PyCertGetStoreProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PropId", NULL};
	HCERTSTORE hcertstore=((PyCERTSTORE *)self)->GetHCERTSTORE();
	void *buf=NULL;
	DWORD propid, bufsize;
	BOOL bsuccess;
	PyObject *ret=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:CertGetStoreProperty", keywords,
		&propid))	// @pyparm int|PropId||Id of the property to get, CERT_STORE_LOCALIZED_NAME_PROP_ID or user-defined property id
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertGetStoreProperty(hcertstore, propid, buf, &bufsize);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("CertSetStoreProperty");
	buf=malloc(bufsize);
	if (buf==NULL)
		return PyErr_NoMemory();
	Py_BEGIN_ALLOW_THREADS
	bsuccess = CertGetStoreProperty(hcertstore, propid, buf, &bufsize);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		PyWin_SetAPIError("CertGetStoreProperty");
	else
		ret = PyString_FromStringAndSize((char *)buf, bufsize);
	free(buf);
	return ret;
}
*/
