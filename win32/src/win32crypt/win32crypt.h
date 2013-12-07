#define Py_USE_NEW_NAMES

// CRYPT_DECRYPT_MESSAGE_PARA.dwflags is in an ifdef for some unknown reason
#define CRYPT_DECRYPT_MESSAGE_PARA_HAS_EXTRA_FIELDS

#define DllExport _declspec(dllexport)
#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"

extern __declspec(dllexport) PyTypeObject PyCRYPTKEYType;
extern __declspec(dllexport) PyTypeObject PyCRYPTPROVType;
extern __declspec(dllexport) PyTypeObject PyCRYPTHASHType;
extern __declspec(dllexport) PyTypeObject PyCRYPTMSGType;
extern __declspec(dllexport) PyTypeObject PyCERTSTOREType;
extern __declspec(dllexport) PyTypeObject PyCERT_CONTEXTType;
extern __declspec(dllexport) PyTypeObject PyCTL_CONTEXTType;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCERT_CONTEXT : public PyObject
{
public:
	PyCERT_CONTEXT(PCCERT_CONTEXT pccert_context);
	~PyCERT_CONTEXT(void);

	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *, PyObject *, PyObject *);
	static PyObject *PyCertFreeCertificateContext(PyObject *self, PyObject *args);
	static PyObject *PyCertEnumCertificateContextProperties(PyObject *self, PyObject *args);
	static PyObject *PyCryptAcquireCertificatePrivateKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertGetIntendedKeyUsage(PyObject *self, PyObject *args);
	static PyObject *PyCertGetEnhancedKeyUsage(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertSerializeCertificateStoreElement(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertVerifySubjectCertificateContext(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertDeleteCertificateFromStore(PyObject *self, PyObject *args);
	static PyObject *PyCertGetCertificateContextProperty(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertSetCertificateContextProperty(PyObject *self, PyObject *args, PyObject *kwargs);
	PCCERT_CONTEXT GetPCCERT_CONTEXT(void) {return pccert_context;};
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	PCCERT_CONTEXT pccert_context;
	PyObject *obdummy;
};

// #define OFF(e) offsetof(PyCERT_CONTEXT, e)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCERTSTORE : public PyObject
{
public:
	PyCERTSTORE(HCERTSTORE hcertstore);
	~PyCERTSTORE(void);

	PyObject *obcertstore;
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static PyObject *PyCertCloseStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertControlStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertEnumCertificatesInStore(PyObject *self, PyObject *args);
	static PyObject *PyCertEnumCTLsInStore(PyObject *self, PyObject *args);
	static PyObject *PyCertSaveStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddEncodedCertificateToStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddCertificateContextToStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddCertificateLinkToStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddCTLContextToStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddCTLLinkToStore(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertAddStoreToCollection(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCertRemoveStoreFromCollection(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyPFXExportCertStoreEx(PyObject *self, PyObject *args, PyObject *kwargs);
	// static PyObject *PyCertGetStoreProperty(PyObject *self, PyObject *args, PyObject *kwargs);
	// static PyObject *PyCertSetStoreProperty(PyObject *self, PyObject *args, PyObject *kwargs);
	HCERTSTORE GetHCERTSTORE(void) {return hcertstore;};
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	HCERTSTORE hcertstore;
};


///////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCRYPTHASH : public PyObject
{
public:

	PyCRYPTHASH(HCRYPTHASH hcrypthash);
	~PyCRYPTHASH(void);

	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static PyObject *PyCryptDestroyHash(PyObject *self, PyObject *args);
	static PyObject *PyCryptDuplicateHash(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptSignHash(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptHashData(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptHashSessionKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptVerifySignature(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGetHashParam(PyObject *self, PyObject *args, PyObject *kwargs);
	HCRYPTHASH GetHCRYPTHASH(void) {return hcrypthash;};

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	HCRYPTHASH hcrypthash;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCRYPTKEY : public PyObject
{
public:

	PyCRYPTKEY(HCRYPTKEY hcryptkey,PyObject *obcryptprov);
	~PyCRYPTKEY(void);

	PyObject *obcryptprov, *obcryptkey, *obdummy;
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *name, PyObject *v);
	static PyObject *PyCryptDestroyKey(PyObject *self, PyObject *args);
	static PyObject *PyCryptExportKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGetKeyParam(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptDuplicateKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptEncrypt(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptDecrypt(PyObject *self, PyObject *args, PyObject *kwargs);
	HCRYPTKEY GetHCRYPTKEY(void) {return hcryptkey;};

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	HCRYPTKEY hcryptkey;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCRYPTPROV : public PyObject
{
public:
	PyCRYPTPROV(HCRYPTPROV hcryptprov);
	~PyCRYPTPROV(void);

	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *, PyObject *);
	static int setattro(PyObject *, PyObject *, PyObject *);
	static PyObject *PyCryptReleaseContext(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGenKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGetProvParam(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGetUserKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptGenRandom(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptCreateHash(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptImportKey(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptExportPublicKeyInfo(PyObject *self, PyObject *args, PyObject *kwargs);
	static PyObject *PyCryptImportPublicKeyInfo(PyObject *self, PyObject *args, PyObject *kwargs);
	HCRYPTPROV GetHCRYPTPROV(void) {return hcryptprov;};
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	HCRYPTPROV hcryptprov;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCRYPTMSG : public PyObject
{
public:

	PyCRYPTMSG(HCRYPTMSG hcryptmsg);
	~PyCRYPTMSG(void);

	PyObject *obcryptmsg, *obdummy;
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *name, PyObject *v);
	static PyObject *PyCryptMsgClose(PyObject *self, PyObject *args);
	HCRYPTMSG GetHCRYPTMSG(void) {return hcryptmsg;};

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	HCRYPTMSG hcryptmsg;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
class __declspec(dllexport) PyCTL_CONTEXT : public PyObject
{
public:

	PyCTL_CONTEXT(PCCTL_CONTEXT);
	~PyCTL_CONTEXT(void);

	PyObject *obctl_context, *obdummy;
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *name, PyObject *v);
	static PyObject *PyCertFreeCTLContext(PyObject *self, PyObject *args);
	static PyObject *PyCertEnumCTLContextProperties(PyObject *self, PyObject *args);
	static PyObject *PyCertEnumSubjectInSortedCTL(PyObject *self, PyObject *args);
	static PyObject *PyCertDeleteCTLFromStore(PyObject *self, PyObject *args);
	static PyObject *PyCertSerializeCTLStoreElement(PyObject *self, PyObject *args, PyObject *kwargs);
	PCCTL_CONTEXT GetCTL_CONTEXT(void) {return pctl_context;};

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	static struct PyMethodDef methods[];
protected:
	PCCTL_CONTEXT pctl_context;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL PyWinObject_AsDATA_BLOB(PyObject *ob, DATA_BLOB *b);
PyObject *PyWinObject_FromDATA_BLOB(DATA_BLOB *b);

BOOL PyWinObject_AsCRYPTPROTECT_PROMPTSTRUCT(PyObject *ob, CRYPTPROTECT_PROMPTSTRUCT* PromptStruct);

BOOL PyWinObject_AsCERTSTORE(PyObject *obhcertstore, HCERTSTORE *hcertstore, BOOL bNoneOK);
PyObject *PyWinObject_FromCERTSTORE(HCERTSTORE certstore);

BOOL PyWinObject_AsCRYPTMSG(PyObject *, HCRYPTMSG *, BOOL);
PyObject *PyWinObject_FromCRYPTMSG(HCRYPTMSG h);

BOOL PyWinObject_AsCERT_CONTEXT(PyObject *, PCCERT_CONTEXT *, BOOL);
PyObject *PyWinObject_FromCERT_CONTEXT(PCCERT_CONTEXT);

BOOL PyWinObject_AsCTL_CONTEXT(PyObject *, PCCTL_CONTEXT *, BOOL);
PyObject *PyWinObject_FromCTL_CONTEXT(PCCTL_CONTEXT);


BOOL PyWinObject_AsCTL_USAGE(PyObject *ob, CTL_USAGE *pcu);
PyObject *PyWinObject_FromCTL_USAGE(PCTL_USAGE pUsage);
void PyWinObject_FreeCTL_USAGE(CTL_USAGE *pcu);

BOOL PyWinObject_AsHCRYPTPROV(PyObject *obhcryptprov, HCRYPTPROV *hcryptprov, BOOL bNoneOK);
BOOL PyWinObject_AsHCRYPTKEY(PyObject *obhcryptkey, HCRYPTKEY *hcryptkey, BOOL bNoneOK);
BOOL PyWinObject_AsHCRYPTHASH(PyObject *obhcrypthash, HCRYPTHASH *hcrypthash, BOOL bNoneOK);

BOOL PyWinObject_AsCERT_CONTEXTArray(PyObject *obcerts, PCCERT_CONTEXT **pppcerts, DWORD *cert_cnt);
void PyWinObject_FreeCERT_CONTEXTArray(PCCERT_CONTEXT *ppcerts, DWORD cert_cnt);

BOOL PyWinObject_AsPCERT_SYSTEM_STORE_RELOCATE_PARA(PyObject *obpvPara, PCERT_SYSTEM_STORE_RELOCATE_PARA pcssrp);
PyObject *PyWinObject_FromCRYPT_KEY_PROV_INFO(PCRYPT_KEY_PROV_INFO pckpi);
PyObject *PyWinObject_FromCRYPT_ALGORITHM_IDENTIFIER(PCRYPT_ALGORITHM_IDENTIFIER pcai);
BOOL PyWinObject_AsCRYPT_ALGORITHM_IDENTIFIER(PyObject *obcai, PCRYPT_ALGORITHM_IDENTIFIER pcai);

PyObject *PyWinObject_FromCRYPT_BIT_BLOB(PCRYPT_BIT_BLOB pcbb);
BOOL PyWinObject_AsCRYPT_BIT_BLOB(PyObject *obcbb, PCRYPT_BIT_BLOB pcbb);

PyObject *PyWinObject_FromCERT_PUBLIC_KEY_INFO(PCERT_PUBLIC_KEY_INFO pcpki);
BOOL PyWinObject_AsCERT_PUBLIC_KEY_INFO(PyObject *obcpki, PCERT_PUBLIC_KEY_INFO pcpki);

// conversions for various cert extensions
PyObject *PyWinObject_FromCERT_NAME_INFO(PCERT_NAME_INFO pcni);
PyObject *PyWinObject_FromCERT_NAME_VALUE(PCERT_NAME_VALUE pcnv);
PyObject *PyWinObject_FromCERT_ALT_NAME_INFO(PCERT_ALT_NAME_INFO pcani);
PyObject *PyWinObject_FromCERT_ALT_NAME_ENTRY(DWORD dwCertEncodingType, PCERT_ALT_NAME_ENTRY pcane);
PyObject *PyWinObject_FromCRYPT_INTEGER_BLOB(PCRYPT_INTEGER_BLOB pcib);
PyObject *PyWinObject_FromCRYPT_OID_INFO(PCCRYPT_OID_INFO oid_info);
PyObject *PyWinObject_FromCTL_USAGE(PCTL_USAGE pUsage);
PyObject *PyWinObject_FromCERT_KEY_ATTRIBUTES_INFO(PCERT_KEY_ATTRIBUTES_INFO pckai);
PyObject *PyWinObject_FromCERT_BASIC_CONSTRAINTS_INFO(PCERT_BASIC_CONSTRAINTS_INFO pcbci);
PyObject *PyWinObject_FromCERT_BASIC_CONSTRAINTS2_INFO(PCERT_BASIC_CONSTRAINTS2_INFO pcbci);
PyObject *PyWinObject_FromCERT_POLICIES_INFO(PCERT_POLICIES_INFO pcpi);
PyObject *PyWinObject_FromCERT_AUTHORITY_KEY_ID_INFO(PCERT_AUTHORITY_KEY_ID_INFO pcaki);

// Functions for translating dicts into parameters for message handling methods
BOOL PyWinObject_AsCRYPT_DECRYPT_MESSAGE_PARA(PyObject *obcdmp, PCRYPT_DECRYPT_MESSAGE_PARA pcdmp);
void PyWinObject_FreeCRYPT_DECRYPT_MESSAGE_PARA(PCRYPT_DECRYPT_MESSAGE_PARA pcdmp);

BOOL PyWinObject_AsCRYPT_VERIFY_MESSAGE_PARA(PyObject *obcvmp, PCRYPT_VERIFY_MESSAGE_PARA pcvmp);
BOOL PyWinObject_AsCRYPT_ENCRYPT_MESSAGE_PARA(PyObject *obcemp, PCRYPT_ENCRYPT_MESSAGE_PARA pcemp);

BOOL PyWinObject_AsCRYPT_SIGN_MESSAGE_PARA(PyObject *obcsmp, PCRYPT_SIGN_MESSAGE_PARA pcsmp);
void PyWinObject_FreeCRYPT_SIGN_MESSAGE_PARA(PCRYPT_SIGN_MESSAGE_PARA pcsmp);

BOOL PyWinObject_AsCRYPT_ATTRIBUTE(PyObject *obca, PCRYPT_ATTRIBUTE pca);
void PyWinObject_FreeCRYPT_ATTRIBUTE(PCRYPT_ATTRIBUTE pca);

BOOL PyWinObject_AsCRYPT_ATTRIBUTEArray(PyObject *obattrs, PCRYPT_ATTRIBUTE *ppca, DWORD *attr_cnt);
void PyWinObject_FreeCRYPT_ATTRIBUTEArray(PCRYPT_ATTRIBUTE pca, DWORD attr_cnt);

BOOL PyWinObject_AsPBYTEArray(PyObject *str_seq, PBYTE **pbyte_array, DWORD **byte_lens, DWORD *str_cnt);
void PyWinObject_FreePBYTEArray(PBYTE *pbyte_array, DWORD *byte_lens, DWORD str_cnt);

BOOL PyWinObject_AsOIDArray(PyObject *str_seq, LPSTR **str_array, DWORD *str_cnt);
void PyWinObject_FreeOIDArray(LPSTR *str_array, DWORD str_cnt);

typedef struct _ENUM_ARG {
    BOOL        fAll;
    BOOL        fVerbose;
    DWORD       dwFlags;
    const void  *pvStoreLocationPara;
    HKEY        hKeyBase;
} ENUM_ARG, *PENUM_ARG;

class PyCRYPT_VERIFY_MESSAGE_PARA : public CRYPT_VERIFY_MESSAGE_PARA{
public:
	PyCRYPT_VERIFY_MESSAGE_PARA(){
		ZeroMemory((CRYPT_VERIFY_MESSAGE_PARA *)this, sizeof(CRYPT_VERIFY_MESSAGE_PARA));
	}
	~PyCRYPT_VERIFY_MESSAGE_PARA(){
		if (pvGetArg)
			free(pvGetArg);
	}
};

class PyCRYPT_DECRYPT_MESSAGE_PARA : public CRYPT_DECRYPT_MESSAGE_PARA{
public:
	PyCRYPT_DECRYPT_MESSAGE_PARA(){
		ZeroMemory((CRYPT_DECRYPT_MESSAGE_PARA *)this, sizeof(CRYPT_DECRYPT_MESSAGE_PARA));
	}
	~PyCRYPT_DECRYPT_MESSAGE_PARA(){
		PyWinObject_FreeCRYPT_DECRYPT_MESSAGE_PARA(this);
	}
};
