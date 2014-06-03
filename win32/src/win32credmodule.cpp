// @doc
#define _WIN32_WINNT 0x501	// Credentials functions only available on WinXP
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "WinCred.h"

// @object PyCREDENTIAL_ATTRIBUTE|A dictionary containing information for a CREDENTIAL_ATTRIBUTE struct
// @pyseeapi CREDENTIAL_ATTRIBUTE
// @prop <o PyUnicode>|Keyword|Attribute name, at most CRED_MAX_STRING_LENGTH chars
// @prop int|Flags|Reserved, use only 0
// @prop str|Value|Attribute value, at most CRED_MAX_VALUE_SIZE bytes.  Unicode objects are treated as raw bytes.
PyObject *PyWinObject_FromCREDENTIAL_ATTRIBUTEArray(PCREDENTIAL_ATTRIBUTE attrs, DWORD attr_cnt)
{
	if ((attrs==NULL) || (attr_cnt==0))
		return PyTuple_New(0);
	PyObject *ret, *ret_item;
	ret=PyTuple_New(attr_cnt);
	if (ret==NULL)
		return NULL;
	for (DWORD attr_ind=0;attr_ind<attr_cnt;attr_ind++){
		ret_item=Py_BuildValue("{s:u,s:k,s:N}",
			"Keyword", attrs[attr_ind].Keyword,
			"Flags", attrs[attr_ind].Flags,
			"Value", PyString_FromStringAndSize((char *)attrs[attr_ind].Value, attrs[attr_ind].ValueSize));
		if (ret_item==NULL){
			Py_DECREF(ret);
			ret=NULL;
			break;
			}
		PyTuple_SET_ITEM(ret, attr_ind, ret_item);
		}
	return ret;
}

void PyWinObject_FreeCREDENTIAL_ATTRIBUTE(PCREDENTIAL_ATTRIBUTE attr)
{
	PyWinObject_FreeWCHAR(attr->Keyword);
	if (attr->Value != NULL)
		free(attr->Value);
	ZeroMemory(attr, sizeof(CREDENTIAL_ATTRIBUTE));
}

BOOL PyWinObject_AsCREDENTIAL_ATTRIBUTE(PyObject *obattr, PCREDENTIAL_ATTRIBUTE attr)
{
	static char *keywords[]={"Keyword","Flags","Value", NULL};
	PyObject *obKeyword, *obValue, *args;
	void *value;
	DWORD valuelen;
	BOOL ret;
	ZeroMemory(attr, sizeof(CREDENTIAL_ATTRIBUTE));
	if (!PyDict_Check(obattr)){
		PyErr_SetString(PyExc_TypeError, "CREDENTIAL_ATTRIBUTE must be a dict");
		return FALSE;
		}
	args=PyTuple_New(0);
	if (args==NULL)
		return FALSE;

	ret=PyArg_ParseTupleAndKeywords(args, obattr, "OkO:CREDENTIAL_ATTRIBUTE", keywords,
			&obKeyword, &attr->Flags, &obValue)
		&&PyWinObject_AsWCHAR(obKeyword, &attr->Keyword, FALSE)
		&&PyWinObject_AsReadBuffer(obValue, &value, &valuelen)
		&&((attr->Value=(LPBYTE)malloc(valuelen))!=NULL);
	if (ret){
		memcpy(attr->Value, value, valuelen);
		attr->ValueSize=valuelen;
		}
	else
		PyWinObject_FreeCREDENTIAL_ATTRIBUTE(attr);
	Py_DECREF(args);
	return ret;
}

void PyWinObject_FreeCREDENTIAL_ATTRIBUTEArray(PCREDENTIAL_ATTRIBUTE *attrs, DWORD attr_cnt)
{
	if (*attrs){
		for (DWORD attr_ind=0; attr_ind < attr_cnt; attr_ind++)
			PyWinObject_FreeCREDENTIAL_ATTRIBUTE(&(*attrs)[attr_ind]);
		free(*attrs);
		*attrs=NULL;
		}
}

BOOL PyWinObject_AsCREDENTIAL_ATTRIBUTEArray(PyObject *obattrs, PCREDENTIAL_ATTRIBUTE *attrs, DWORD *attr_cnt)
{
	PyObject *attr_tuple;
	DWORD attr_ind;
	BOOL ret=TRUE;
	*attrs=NULL;
	*attr_cnt=0;
	// accept either None or empty tuple for no attributes
	if (obattrs==Py_None)
		return TRUE;
	attr_tuple=PyWinSequence_Tuple(obattrs, attr_cnt);
	if (attr_tuple==NULL)
		return FALSE;
	if (*attr_cnt>0){
		*attrs=(PCREDENTIAL_ATTRIBUTE)malloc(*attr_cnt * sizeof(CREDENTIAL_ATTRIBUTE));
		if (*attrs==NULL){
			PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", *attr_cnt * sizeof(CREDENTIAL_ATTRIBUTE));
			ret=FALSE;
			}
		else {
			memset(*attrs, 0, *attr_cnt * sizeof(CREDENTIAL_ATTRIBUTE));
			for (attr_ind=0; attr_ind<*attr_cnt; attr_ind++){
				ret=PyWinObject_AsCREDENTIAL_ATTRIBUTE(PyTuple_GET_ITEM(attr_tuple, attr_ind), &(*attrs)[attr_ind]);
				if (!ret)
					break;
				}
		}
	}
	if (!ret)
		PyWinObject_FreeCREDENTIAL_ATTRIBUTEArray(attrs, *attr_cnt);
	Py_DECREF(attr_tuple);
	return ret;
}


// @object PyCREDENTIAL|A dictionary containing information for a CREDENTIAL struct
// @pyseeapi CREDENTIAL struct
// @prop int|Flags|Combination of CRED_FLAGS_PROMPT_NOW, CRED_FLAGS_USERNAME_TARGET
// @prop int|Type|Type of credential, one of CRED_TYPE_* values
// @prop <o PyUnicode>|TargetName|Target of credential, can end with * for wildcard matching
// @prop <o PyUnicode>|Comment|Descriptive text
// @prop <o PyTime>|LastWritten|Modification time, ignored on input
// @prop <o PyUnicode>|CredentialBlob|Contains password for username credential, or PIN for certificate credential.  This member is write-only.
// @prop int|Persist|Specifies scope of persistence, one of CRED_PERSIST_* values
// @prop tuple|Attributes|Tuple of <o PyCREDENTIAL_ATTRIBUTE> dicts containing application-specific data, can be None
// @prop <o PyUnicode>|TargetAlias|Alias for TargetName, only valid with CRED_TYPE_GENERIC
// @prop <o PyUnicode>|UserName|User to be authenticated by target. Can be of the form username@domain or domain\username.
// For CRED_TYPE_DOMAIN_CERTIFICATE, use <om win32cred.CredMarshalCredential> to marshal the SHA1 hash of user's certficate
PyObject *PyWinObject_FromCREDENTIAL(PCREDENTIAL credential)
{
	return Py_BuildValue("{s:k,s:k,s:u,s:u,s:N,s:N,s:k,s:N,s:u,s:u}",
		"Flags", credential->Flags,
		"Type", credential->Type,
		"TargetName", credential->TargetName,
		"Comment",credential->Comment,
		"LastWritten", PyWinObject_FromFILETIME(credential->LastWritten),
		"CredentialBlob", PyString_FromStringAndSize((char *)credential->CredentialBlob, credential->CredentialBlobSize),
		"Persist", credential->Persist,
		"Attributes",	PyWinObject_FromCREDENTIAL_ATTRIBUTEArray(credential->Attributes, credential->AttributeCount),
		"TargetAlias", credential->TargetAlias,
		"UserName", credential->UserName);
}

PyObject *PyWinObject_FromPCREDENTIALArray(PCREDENTIAL *credentials, DWORD cred_cnt)
{
	PyObject *ret, *ret_item;
	ret=PyTuple_New(cred_cnt);
	if (ret==NULL)
		return NULL;
	for (DWORD cred_ind=0;cred_ind<cred_cnt;cred_ind++){
		ret_item=PyWinObject_FromCREDENTIAL(credentials[cred_ind]);
		if (ret_item==NULL){
			Py_DECREF(ret);
			ret=NULL;
			break;
			}
		PyTuple_SET_ITEM(ret, cred_ind, ret_item);
		}
	return ret;
}

void PyWinObject_FreeCREDENTIAL(PCREDENTIAL cred)
{
	PyWinObject_FreeWCHAR(cred->TargetName);
	PyWinObject_FreeWCHAR(cred->Comment);
	PyWinObject_FreeWCHAR(cred->TargetAlias);
	PyWinObject_FreeWCHAR(cred->UserName);
	PyWinObject_FreeWCHAR((WCHAR *)cred->CredentialBlob);
	PyWinObject_FreeCREDENTIAL_ATTRIBUTEArray(&cred->Attributes, cred->AttributeCount);
	ZeroMemory(cred, sizeof(CREDENTIAL));
}

BOOL PyWinObject_AsCREDENTIAL(PyObject *obcred, PCREDENTIAL cred)
{
	static char *keywords[]={"Flags","Type","TargetName","Comment","LastWritten","CredentialBlob",
		"Persist", "Attributes", "TargetAlias", "UserName", NULL};

	PyObject *obTargetName=Py_None, *obComment=Py_None, *obLastWritten=Py_None, *obCredentialBlob=Py_None,
		*obPersist=Py_None, *obAttributes=Py_None,
		*obTargetAlias=Py_None, *obUserName=Py_None;
	PyObject *args;
	BOOL ret;
	ZeroMemory(cred, sizeof(CREDENTIAL));
	if (!PyDict_Check(obcred)){
		PyErr_SetString(PyExc_TypeError, "CREDENTIAL must be a dict");
		return FALSE;
		}
	args=PyTuple_New(0);
	if (args==NULL)
		return FALSE;

	ret=PyArg_ParseTupleAndKeywords(args, obcred, "|kkOOOOkOOO:CREDENTIAL", keywords,
			&cred->Flags, &cred->Type, &obTargetName, &obComment, &obLastWritten,
			&obCredentialBlob, &cred->Persist, &obAttributes, &obTargetAlias, &obUserName)
		&&PyWinObject_AsWCHAR(obTargetName, &cred->TargetName, TRUE)
		&&PyWinObject_AsWCHAR(obComment, &cred->Comment, TRUE)
		&&((obLastWritten==Py_None)||PyWinObject_AsFILETIME(obLastWritten, &cred->LastWritten))
		&&PyWinObject_AsWCHAR(obCredentialBlob, (WCHAR **)&cred->CredentialBlob, TRUE, &cred->CredentialBlobSize)
		&&PyWinObject_AsCREDENTIAL_ATTRIBUTEArray(obAttributes, &cred->Attributes, &cred->AttributeCount)
		&&PyWinObject_AsWCHAR(obTargetAlias, &cred->TargetAlias, TRUE)
		&&PyWinObject_AsWCHAR(obUserName, &cred->UserName, TRUE);
	// size of CredentialBlob is in bytes, not characters - actually throws an error if you pass in an odd number!
	cred->CredentialBlobSize*=sizeof(WCHAR);
	Py_DECREF(args);
	if (!ret)
		PyWinObject_FreeCREDENTIAL(cred);
	return ret;
}

// @object PyCREDENTIAL_TARGET_INFORMATION|A dictionary representing a CREDENTIAL_TARGET_INFORMATION struct
// @pyseeapi CREDENTIAL_TARGET_INFORMATION
// @prop <o PyUnicode>|TargetName|Target of credentials
// @prop <o PyUnicode>|NetbiosServerName|
// @prop <o PyUnicode>|DnsServerName|
// @prop <o PyUnicode>|NetbiosDomainName|
// @prop <o PyUnicode>|DnsDomainName|
// @prop <o PyUnicode>|DnsTreeName|
// @prop <o PyUnicode>|PackageName|Name of security package which mapped TargetName
// @prop int|Flags|CRED_TI_* flags
// @prop (int,...)|CredTypes|Tuple of CRED_TYPE_* values indicating which types of credentials are acceptable to target
PyObject *PyWinObject_FromCREDENTIAL_TARGET_INFORMATION(PCREDENTIAL_TARGET_INFORMATION targetinfo)
{
	PyObject *cred_types=PyTuple_New(targetinfo->CredTypeCount);
	if (cred_types==NULL)
		return NULL;
	for (DWORD cred_ind=0; cred_ind<targetinfo->CredTypeCount; cred_ind++){
		PyObject *cred_type=PyLong_FromUnsignedLong(targetinfo->CredTypes[cred_ind]);
		if (cred_type==NULL){
			Py_DECREF(cred_types);
			return NULL;
			}
		PyTuple_SET_ITEM(cred_types, cred_ind, cred_type);
		}
	return Py_BuildValue("{s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:k,s:N}",
		"TargetName",			targetinfo->TargetName,
		"NetbiosServerName",	targetinfo->NetbiosServerName,
		"DnsServerName",		targetinfo->DnsServerName,
		"NetbiosDomainName",	targetinfo->NetbiosDomainName,
		"DnsDomainName",		targetinfo->DnsDomainName,
		"DnsTreeName",			targetinfo->DnsTreeName,
		"PackageName",			targetinfo->PackageName,
		"Flags",				targetinfo->Flags,
		"CredTypes",			cred_types);
}

void PyWinObject_FreeCREDENTIAL_TARGET_INFORMATION(PCREDENTIAL_TARGET_INFORMATION targetinfo)
{
	PyWinObject_FreeWCHAR(targetinfo->TargetName);
	PyWinObject_FreeWCHAR(targetinfo->NetbiosServerName);
	PyWinObject_FreeWCHAR(targetinfo->DnsServerName);
	PyWinObject_FreeWCHAR(targetinfo->NetbiosDomainName);
	PyWinObject_FreeWCHAR(targetinfo->DnsDomainName);
	PyWinObject_FreeWCHAR(targetinfo->DnsTreeName);
	PyWinObject_FreeWCHAR(targetinfo->PackageName);
	if (targetinfo->CredTypeCount)
		free(targetinfo->CredTypes);
	ZeroMemory(targetinfo, sizeof(CREDENTIAL_TARGET_INFORMATION));
}

BOOL PyWinObject_AsCREDENTIAL_TARGET_INFORMATION(PyObject *obtargetinfo, PCREDENTIAL_TARGET_INFORMATION targetinfo)
{
	static char *keywords[]={"TargetName","NetbiosServerName","DnsServerName",
		"NetbiosDomainName","DnsDomainName","DnsTreeName",
		"PackageName","Flags","CredTypes", NULL};
	BOOL ret;
	PyObject *cred_types=Py_None, *args;
	PyObject *obTargetName=Py_None, *obNetbiosServerName=Py_None, *obDnsServerName=Py_None,
		*obNetbiosDomainName=Py_None, *obDnsDomainName=Py_None, *obDnsTreeName=Py_None, *obPackageName=Py_None;

	ZeroMemory(targetinfo,sizeof(CREDENTIAL_TARGET_INFORMATION));
	if (!PyDict_Check(obtargetinfo)){
		PyErr_SetString(PyExc_TypeError, "CREDENTIAL_TARGET_INFORMATION must be a dict");
		return FALSE;
		}
	args=PyTuple_New(0);
	if (args==NULL)
		return FALSE;

	ret=PyArg_ParseTupleAndKeywords(args, obtargetinfo, "|OOOOOOOkO:CREDENTIAL_TARGET_INFORMATION", keywords,
			&obTargetName, &obNetbiosServerName, &obDnsServerName,
			&obNetbiosDomainName, &obDnsDomainName, &obDnsTreeName, &obPackageName,
			&targetinfo->Flags, &cred_types)
		&&PyWinObject_AsWCHAR(obTargetName, &targetinfo->TargetName, TRUE)
		&&PyWinObject_AsWCHAR(obNetbiosServerName, &targetinfo->NetbiosServerName, TRUE)
		&&PyWinObject_AsWCHAR(obDnsServerName, &targetinfo->DnsServerName, TRUE)
		&&PyWinObject_AsWCHAR(obNetbiosDomainName, &targetinfo->NetbiosDomainName, TRUE)
		&&PyWinObject_AsWCHAR(obDnsDomainName, &targetinfo->DnsDomainName, TRUE)
		&&PyWinObject_AsWCHAR(obDnsTreeName, &targetinfo->DnsTreeName, TRUE)
		&&PyWinObject_AsWCHAR(obPackageName, &targetinfo->PackageName, TRUE)
		&&PyWinObject_AsDWORDArray(cred_types, &targetinfo->CredTypes, &targetinfo->CredTypeCount, TRUE);

	Py_DECREF(args);
	if (!ret)
		PyWinObject_FreeCREDENTIAL_TARGET_INFORMATION(targetinfo);
	return ret;
}

void PyWinObject_FreeCREDUI_INFO(PCREDUI_INFO uiinfo)
{
	if (uiinfo){
		PyWinObject_FreeWCHAR((WCHAR *)uiinfo->pszMessageText);
		PyWinObject_FreeWCHAR((WCHAR *)uiinfo->pszCaptionText);
		ZeroMemory(uiinfo,sizeof(CREDUI_INFO));
		free(uiinfo);
		}
}

// @object PyCREDUI_INFO|A dictionary representing a CREDUI_INFO structure, used with <om win32cred.CredUIPromptForCredentials>
// @comm All members are optional
// @pyseeapi CREDUI_INFO
// @prop <o PyHANDLE>|Parent|Handle to parent window, can be None
// @prop <o PyUnicode>|MessageText|Message to appear in dialog
// @prop <o PyUnicode>|CaptionText|Title of the dialog window
// @prop <o PyHANDLE>|Banner|Handle to a bitmap to be displayed
BOOL PyWinObject_AsCREDUI_INFO(PyObject *obuiinfo, PCREDUI_INFO *puiinfo)
{
	static char *keywords[]={"Parent", "MessageText", "CaptionText", "Banner", NULL};
	PyObject *obparent=Py_None, *obmessage=Py_None, *obcaption=Py_None, *obbanner=Py_None;
	PyObject *args;
	BOOL ret;
	*puiinfo=NULL;
	if (obuiinfo==Py_None)
		return TRUE;
	if (!PyDict_Check(obuiinfo)){
		PyErr_SetString(PyExc_TypeError, "CREDUI_INFO must be a dict");
		return FALSE;
		}
	*puiinfo=(PCREDUI_INFO)malloc(sizeof(CREDUI_INFO));
	if (*puiinfo==NULL){
		PyErr_SetString(PyExc_MemoryError, "Unable to allocate CREDUI_INFO struct");
		return FALSE;
		}
	ZeroMemory(*puiinfo,sizeof(CREDUI_INFO));
	(*puiinfo)->cbSize=sizeof(CREDUI_INFO);

	args=PyTuple_New(0);
	ret=(args!=NULL)&&
		PyArg_ParseTupleAndKeywords(args, obuiinfo, "|OOOO:CREDUI_INFO", keywords,
			&obparent, &obmessage, &obcaption, &obbanner)
		&&PyWinObject_AsWCHAR(obmessage, (WCHAR **)&(*puiinfo)->pszMessageText, TRUE)
		&&PyWinObject_AsWCHAR(obcaption, (WCHAR **)&(*puiinfo)->pszCaptionText, TRUE)
		&&PyWinObject_AsHANDLE(obparent, (HANDLE *)&(*puiinfo)->hwndParent)
		&&PyWinObject_AsHANDLE(obbanner, (HANDLE *)&(*puiinfo)->hbmBanner);

	Py_XDECREF(args);
	if (!ret){
		PyWinObject_FreeCREDUI_INFO(*puiinfo);
		*puiinfo=NULL;
		}
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Module methods
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// @pymethod <o PyUnicode>|win32cred|CredMarshalCredential|Marshals a credential into a unicode string
// @comm Credentials with Flags that contain CRED_FLAGS_USERNAME_TARGET can be marshalled to be passed as the username
// to functions that normally require a username/password combination, such as <om win32security.LogonUser> and <om win32net.NetUseAdd>
PyObject * PyCredMarshalCredential(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"CredType","Credential",NULL};
	CERT_CREDENTIAL_INFO cert_cred;
	USERNAME_TARGET_CREDENTIAL_INFO username_cred;
	PyObject *obcredential, *ret=NULL;
	PVOID credential=NULL;
	CRED_MARSHAL_TYPE credtype;
	WCHAR *output_cred=NULL, *username=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kO:CredMarshalCredential", keywords,
		&credtype,		// @pyparm int|CredType||CertCredential or UsernameTargetCredential
		&obcredential))	// @pyparm str/<o PyUnicode>|Credential||The credential to be marshalled.  Type is dependent on CredType.
		return NULL;

	// @flagh CredType|Type of Credential
	switch(credtype){
	// @flag CertCredential|String containing the SHA1 hash of user's certificate
		case CertCredential:{
			Py_ssize_t hashlen;
			char *hash;
			if (PyString_AsStringAndSize(obcredential, &hash, &hashlen)==-1)
				goto done;
			if (hashlen>CERT_HASH_LENGTH){
				PyErr_Format(PyExc_ValueError,"Certificate hash cannot be longer than %d characters", CERT_HASH_LENGTH);
				goto done;
				}
			ZeroMemory(&cert_cred,sizeof(cert_cred));
			cert_cred.cbSize=sizeof(cert_cred);
			memcpy(&cert_cred.rgbHashOfCert, hash, hashlen);
			credential=&cert_cred;
			break;
			}
		// @flag UsernameTargetCredential|Unicode string containing a username for which credentials exist in current logon session
		case UsernameTargetCredential:
			if (!PyWinObject_AsWCHAR(obcredential, &username))
				goto done;
			ZeroMemory(&username_cred,sizeof(username_cred));
			username_cred.UserName=username;
			credential=&username_cred;
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError,"Credential type %d is not supported", credtype);
			goto done;
		}
	if (!CredMarshalCredential(credtype, credential, &output_cred))
		PyWin_SetAPIError("CredMarshalCredential");
	else
		ret=PyWinObject_FromWCHAR(output_cred);
done:
	PyWinObject_FreeWCHAR(username);
	if (output_cred)
		CredFree(output_cred);
	return ret;
}

// @pymethod int,<o PyUnicode>|win32cred|CredUnmarshalCredential|Unmarshals credentials formatted using <om win32cred.CredMarshalCredential>
// @rdesc Returns the credential type and credentials.
PyObject * PyCredUnmarshalCredential(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"MarshaledCredential", NULL};
	PyObject *obcredential, *ret=NULL;
	PVOID credential=NULL;
	CRED_MARSHAL_TYPE credtype;
	WCHAR *input_cred=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CredUnmarshalCredential", keywords,
		&obcredential))		// @pyparm <o PyUnicode>|MarshaledCredential||Unicode string containing marshaled credential
		return NULL;
	if (!PyWinObject_AsWCHAR(obcredential, &input_cred, FALSE))
		return NULL;
	if (!CredUnmarshalCredential(input_cred, &credtype, &credential)){
		PyWin_SetAPIError("CredUnmarshalCredential");
		goto done;
		}

	// @flagh CredType|Type of output credentials
	switch(credtype){
		// @flag CertCredential|Character string containing SHA1 hash of a certificate
		case CertCredential:
			ret=Py_BuildValue("kN", credtype, 
				PyString_FromStringAndSize((char *)&((PCERT_CREDENTIAL_INFO)credential)->rgbHashOfCert, CERT_HASH_LENGTH));
			break;
		// @flag UsernameTargetCredential|Unicode string containing username
		case UsernameTargetCredential:
			ret=Py_BuildValue("kN", credtype, 
				PyWinObject_FromWCHAR(((PUSERNAME_TARGET_CREDENTIAL_INFO)credential)->UserName));
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError,"Credential type %d is not supported", credtype);
		}
done:
	PyWinObject_FreeWCHAR(input_cred);
	if (credential)
		CredFree(credential);
	return ret;
}

// @pymethod boolean|win32cred|CredIsMarshaledCredential|Checks if a string matches the form of a marshaled credential
PyObject * PyCredIsMarshaledCredential(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"MarshaledCredential", NULL};
	WCHAR *cred;
	PyObject *obcred, *ret=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CredIsMarshaledCredential", keywords,
		&obcred))	// @pyparm <o PyUnicode>|MarshaledCredential||Marshaled credential as returned by <om win32cred.CredMarshalCredential>
		return NULL;
	if (!PyWinObject_AsWCHAR(obcred, &cred, FALSE))
		return NULL;
	ret=PyBool_FromLong(CredIsMarshaledCredential(cred));
	PyWinObject_FreeWCHAR(cred);
	return ret;
}

// @pymethod (dict,...)|win32cred|CredEnumerate|Lists credentials for current logon session
// @rdesc Returns a sequence of <o PyCREDENTIAL> dictionaries
PyObject * PyCredEnumerate(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Filter","Flags", NULL};
	PyObject *obfilter=Py_None, *ret=NULL;
	WCHAR *filter=NULL;
	DWORD flags=0, cred_cnt;
	PCREDENTIAL *credentials=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Ok:CredEnumerate", keywords,
		&obfilter,	// @pyparm <o PyUnicode>|Filter|None|Matches credentials' target names by prefix, can be None
		&flags))	// @pyparm int|Flags|0|Reserved, use 0 if passed in
		return NULL;
	if (!PyWinObject_AsWCHAR(obfilter, &filter, TRUE))
		return NULL;
	if (!CredEnumerate(filter, flags, &cred_cnt, &credentials))
		PyWin_SetAPIError("CredEnumerate");
	else
		ret=PyWinObject_FromPCREDENTIALArray(credentials, cred_cnt);
	PyWinObject_FreeWCHAR(filter);
	if (credentials)
		CredFree(credentials);
	return ret;
}

// @pymethod dict|win32cred|CredGetTargetInfo|Determines type and location of credential target
// @rdesc Returns a <o PyCREDENTIAL_TARGET_INFORMATION> dict
// @comm The target information will not be available until an attempt is made to authenticate against it
PyObject *PyCredGetTargetInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "Flags", NULL};
	PyObject *obtargetname, *ret=NULL;
	WCHAR *targetname=NULL;
	DWORD flags=0;
	PCREDENTIAL_TARGET_INFORMATION targetinfo=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CredGetTargetInfo", keywords,
		&obtargetname,		// @pyparm <o PyUnicode>|TargetName||Name of server that is target of stored credentials
		&flags))			// @pyparm int|Flags|0|CRED_ALLOW_NAME_RESOLUTION, or 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		return NULL;
	if (!CredGetTargetInfo(targetname, flags, &targetinfo))
		PyWin_SetAPIError("CredGetTargetInfo");
	else
		ret=PyWinObject_FromCREDENTIAL_TARGET_INFORMATION(targetinfo);
	PyWinObject_FreeWCHAR(targetname);
	if (targetinfo)
		CredFree(targetinfo);
	return ret;
}

// @pymethod |win32cred|CredWriteDomainCredentials|Creates or updates credential for a domain or server
// @comm When updating a credential, to preserve a previously stored password use None or ''
// for CredentialBlob member of Credential and pass CRED_PRESERVE_CREDENTIAL_BLOB in Flags
PyObject *PyCredWriteDomainCredentials(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetInfo", "Credential", "Flags", NULL};
	PyObject *obtargetinfo, *obcred, *ret=NULL;
	DWORD flags=0;
	CREDENTIAL_TARGET_INFORMATION targetinfo={NULL};
	CREDENTIAL cred={NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|k:CredWriteDomainCredentials", keywords,
		&obtargetinfo,	// @pyparm dict|TargetInfo||<o PyCREDENTIAL_TARGET_INFORMATION> identifying the target domain. At least one of the Names is required
		&obcred,		// @pyparm dict|Credential||<o PyCREDENTIAL> dict containing the credentials to be stored
		&flags))		// @pyparm int|Flags|0|CRED_PRESERVE_CREDENTIAL_BLOB is only defined flag
		return NULL;
	if (PyWinObject_AsCREDENTIAL_TARGET_INFORMATION(obtargetinfo, &targetinfo)
		&&PyWinObject_AsCREDENTIAL(obcred, &cred)){
		if (CredWriteDomainCredentials(&targetinfo, &cred, flags)){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("CredWriteDomainCredentials");
		}
	PyWinObject_FreeCREDENTIAL(&cred);
	PyWinObject_FreeCREDENTIAL_TARGET_INFORMATION(&targetinfo);
	return ret;
}

// @pymethod (dict,...)|win32cred|CredReadDomainCredentials|Retrieves credentials for a domain or server
// @rdesc Returns a sequence of <o PyCREDENTIAL> dicts
PyObject *PyCredReadDomainCredentials(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetInfo", "Flags", NULL};
	PyObject *obtargetinfo, *ret=NULL;
	DWORD cred_cnt, flags=0;
	CREDENTIAL_TARGET_INFORMATION targetinfo={NULL};
	PCREDENTIAL *creds=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CredReadDomainCredentials", keywords,
		&obtargetinfo,		// @pyparm dict|TargetInfo||<o PyCREDENTIAL_TARGET_INFORMATION> identifying a domain or server. At least one of the Names is required.
		&flags))			// @pyparm int|Flags|0|CRED_CACHE_TARGET_INFORMATION is only valid flag
		return NULL;
	if (!PyWinObject_AsCREDENTIAL_TARGET_INFORMATION(obtargetinfo, &targetinfo))
		return NULL;
	if (!CredReadDomainCredentials(&targetinfo, flags, &cred_cnt, &creds))
		PyWin_SetAPIError("CredReadDomainCredentials");
	else
		ret=PyWinObject_FromPCREDENTIALArray(creds, cred_cnt);
	PyWinObject_FreeCREDENTIAL_TARGET_INFORMATION(&targetinfo);
	if (creds)
		CredFree(creds);
	return ret;
}

// @pymethod |win32cred|CredDelete|Deletes a stored credential
PyObject *PyCredDelete(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "Type", "Flags", NULL};
	PyObject *obtargetname, *ret=NULL;
	WCHAR *targetname;
	DWORD cred_type, flags=0;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|k:CredDelete", keywords,
		&obtargetname,	// @pyparm <o PyUnicode>|TargetName||Target of credential to be deleted
		&cred_type,		// @pyparm int|Type||One of the CRED_TYPE_* values
		&flags))		// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		return NULL;
	if (!CredDelete(targetname, cred_type, flags))
		PyWin_SetAPIError("CredDelete");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreeWCHAR(targetname);
	return ret;
}

// @pymethod |win32cred|CredWrite|Creates or updates a stored credential
// @comm When updating a credential, to preserve a previously stored password use None or ''
// for CredentialBlob member of Credential and pass CRED_PRESERVE_CREDENTIAL_BLOB in Flags
PyObject *PyCredWrite(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Credential", "Flags", NULL};
	PyObject *obcred, *ret=NULL;
	DWORD flags=0;
	CREDENTIAL cred;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|k:CredWrite", keywords,
		&obcred,		// @pyparm dict|Credential||<o PyCREDENTIAL> dict containing the credentials to be stored
		&flags))		// @pyparm int|Flags|0|CRED_PRESERVE_CREDENTIAL_BLOB is only defined flag
		return NULL;
	if (!PyWinObject_AsCREDENTIAL(obcred, &cred))
		return NULL;
	if (!CredWrite(&cred, flags))
		PyWin_SetAPIError("CredWrite");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreeCREDENTIAL(&cred);
	return ret;
}

// @pymethod dict|win32cred|CredRead|Retrieves a stored credential
// @rdesc Returns a <o PyCREDENTIAL> dict
PyObject *PyCredRead(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "Type", "Flags", NULL};
	PyObject *obtargetname, *ret=NULL;
	WCHAR *targetname=NULL;
	DWORD cred_type, flags=0;
	PCREDENTIAL cred = NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|k:CredRead", keywords,
		&obtargetname,		// @pyparm <o PyUnicode>|TargetName||The target of the credentials to retrieve
		&cred_type,			// @pyparm int|Type||One of the CRED_TYPE_* constants
		&flags))			// @pyparm int|Flags|0|Reserved, use 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		return NULL;
	if (!CredRead(targetname, cred_type, flags, &cred))
		PyWin_SetAPIError("CredRead");
	else
		ret=PyWinObject_FromCREDENTIAL(cred);
	PyWinObject_FreeWCHAR(targetname);
	if (cred)
		CredFree(cred);
	return ret;
}

// @pymethod dict|win32cred|CredRename|Changes the target name of stored credentials
// @comm CRED_FLAGS_USERNAME_TARGET credentials can't be renamed since their TargetName and Username must be equal
PyObject *PyCredRename(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"OldTargetName", "NewTargetName", "Type", "Flags", NULL};
	PyObject *oboldtargetname, *obnewtargetname, *ret=NULL;
	WCHAR *oldtargetname=NULL, *newtargetname=NULL;
	DWORD cred_type, flags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOk|k:CredRename", keywords,
		&oboldtargetname,	// @pyparm <o PyUnicode>|OldTargetName||The target of credential to be renamed
		&obnewtargetname,	// @pyparm <o PyUnicode>|NewTargetName||New target for the specified credential
		&cred_type,			// @pyparm int|Type||Type of the credential to be renamed (CRED_TYPE_*)
		&flags))			// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (PyWinObject_AsWCHAR(oboldtargetname, &oldtargetname, FALSE)
		&&PyWinObject_AsWCHAR(obnewtargetname, &newtargetname, FALSE)){
		if (!CredRename(oldtargetname, newtargetname, cred_type, flags))
			PyWin_SetAPIError("CredRename");
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(oldtargetname);
	PyWinObject_FreeWCHAR(newtargetname);
	return ret;
}

// @pymethod (<o PyUnicode>, <o PyUnicode>, boolean)|win32cred|CredUICmdLinePromptForCredentials|Prompt for username/passwd from a console app
// @rdesc Returns the username and password entered, and a boolean indicating if credential was saved
// @comm The command-line version of this function does not accept certificates, so Flags
// must contain CREDUI_FLAGS_EXCLUDE_CERTIFICATES or CREDUI_FLAGS_REQUIRE_SMARTCARD
PyObject *PyCredUICmdLinePromptForCredentials(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "AuthError", "UserName", "Password", "Save", "Flags",NULL};
	PyObject *obtargetname, *obusername=Py_None, *obpassword=Py_None, *ret=NULL;
	WCHAR *targetname=NULL, *username=NULL, *password=NULL;
	WCHAR *username_io=NULL, *password_io=NULL;
	// max constants don't include trailing NULL
	DWORD maxusername=CREDUI_MAX_USERNAME_LENGTH+1;
	DWORD maxpassword=CREDUI_MAX_PASSWORD_LENGTH+1;
	DWORD usernamelen, passwordlen;
	DWORD autherror=0, flags=CREDUI_FLAGS_EXCLUDE_CERTIFICATES, reterr;
	BOOL save=TRUE;
	PCtxtHandle reserved=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kOOkk:CredUICmdLinePromptForCredentials", keywords,
		&obtargetname,	// @pyparm <o PyUnicode>|TargetName||Server or domain against which to authenticate
		&autherror,		// @pyparm int|AuthError|0|Error code indicating why credentials are required, can be 0
		&obusername,	// @pyparm <o PyUnicode>|UserName|None|Default username, can be None.  At most CREDUI_MAX_USERNAME_LENGTH chars 
		&obpassword,	// @pyparm <o PyUnicode>|Password|None|Password, can be None.  At most CREDUI_MAX_PASSWORD_LENGTH chars
		&save,			// @pyparm boolean|Save|True|Specifies default value for Save prompt
		&flags))		// @pyparm int|Flags|CREDUI_FLAGS_EXCLUDE_CERTIFICATES|Combination of CREDUI_FLAGS_* values
		return NULL;

	username_io=(WCHAR *)malloc(maxusername * sizeof(WCHAR));
	if (username_io==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxusername);
		goto done;
		}
	ZeroMemory(username_io, maxusername*sizeof(WCHAR));
	password_io=(WCHAR *)malloc(maxpassword * sizeof(WCHAR));
	if (password_io==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxpassword);
		goto done;
		}
	ZeroMemory(password_io, maxpassword*sizeof(WCHAR));
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obusername, &username, TRUE, &usernamelen))
		goto done;
	if (!PyWinObject_AsWCHAR(obpassword, &password, TRUE, &passwordlen))
		goto done;
	if (usernamelen > (maxusername-1)){
		PyErr_Format(PyExc_ValueError, "UserName can be at most %d characters", maxusername-1);
		goto done;
		}
	if (passwordlen > (maxpassword-1)){
		PyErr_Format(PyExc_ValueError, "Password can be at most %d characters", maxpassword-1);
		goto done;
		}
	if (username!=NULL)
		wcsncpy(username_io, username, usernamelen);
	if (password!=NULL)
		wcsncpy(password_io, password, passwordlen);

	reterr=CredUICmdLinePromptForCredentials(targetname, reserved, autherror,
		username_io, maxusername, password_io, maxpassword,
		&save, flags);
	if (reterr==NO_ERROR)
		ret=Py_BuildValue("uuN", username_io, password_io, PyBool_FromLong(save));
	else
		PyWin_SetAPIError("CredUICmdLinePromptForCredentials", reterr);

	done:
	PyWinObject_FreeWCHAR(targetname);
	PyWinObject_FreeWCHAR(username);
	if (password!=NULL){
		SecureZeroMemory(password, passwordlen*sizeof(WCHAR));
		PyWinObject_FreeWCHAR(password);
		}
	if (username_io)
		free(username_io);
	if (password_io){
		SecureZeroMemory(password_io, maxpassword*sizeof(WCHAR));
		free (password_io);
		}
	return ret;
}

// @pymethod (<o PyUnicode>, <o PyUnicode>, boolean)|win32cred|CredUIPromptForCredentials|Initiates dialog to request user credentials
// @rdesc Returns the username, password, and a boolean indicating if credential was persisted
PyObject *PyCredUIPromptForCredentials(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "AuthError", "UserName", "Password", "Save", "Flags", "UiInfo", NULL};
	PyObject *obtargetname, *obusername=Py_None, *obpassword=Py_None, *ret=NULL;
	PyObject *obuiinfo=Py_None;
	WCHAR *targetname=NULL, *username=NULL, *password=NULL;
	WCHAR *username_io=NULL, *password_io=NULL;
	// max constants don't include trailing NULL
	DWORD maxusername=CREDUI_MAX_USERNAME_LENGTH+1;
	DWORD maxpassword=CREDUI_MAX_PASSWORD_LENGTH+1;
	DWORD usernamelen, passwordlen;
	DWORD autherror=0, flags=0, reterr;
	BOOL save=TRUE;
	PCtxtHandle reserved=NULL;
	PCREDUI_INFO uiinfo=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kOOkkO:CredUIPromptForCredentials", keywords,
		&obtargetname,	// @pyparm <o PyUnicode>|TargetName||Server or domain against which to authenticate
		&autherror,		// @pyparm int|AuthError|0|Error code indicating why credentials are required, can be 0
		&obusername,	// @pyparm <o PyUnicode>|UserName|None|Default username, can be None.  At most CREDUI_MAX_USERNAME_LENGTH chars 
		&obpassword,	// @pyparm <o PyUnicode>|Password|None|Password, can be None.  At most CREDUI_MAX_PASSWORD_LENGTH chars
		&save,			// @pyparm boolean|Save|True|Specifies whether Save checkbox defaults to checked or unchecked
		&flags,			// @pyparm int|Flags|0|Combination of CREDUI_FLAGS_* values
		&obuiinfo))		// @pyparm dict|UiInfo|None|<o PyCREDUI_INFO> dict for customizing the dialog, can be None
		return NULL;

	username_io=(WCHAR *)malloc(maxusername * sizeof(WCHAR));
	if (username_io==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxusername);
		goto done;
		}
	ZeroMemory(username_io, maxusername*sizeof(WCHAR));
	password_io=(WCHAR *)malloc(maxpassword * sizeof(WCHAR));
	if (password_io==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxpassword);
		goto done;
		}
	ZeroMemory(password_io, maxpassword*sizeof(WCHAR));
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obusername, &username, TRUE, &usernamelen))
		goto done;
	if (!PyWinObject_AsWCHAR(obpassword, &password, TRUE, &passwordlen))
		goto done;
	if (usernamelen > (maxusername-1)){
		PyErr_Format(PyExc_ValueError, "UserName can be at most %d characters", maxusername-1);
		goto done;
		}
	if (passwordlen > (maxpassword-1)){
		PyErr_Format(PyExc_ValueError, "Password can be at most %d characters", maxpassword-1);
		goto done;
		}
	if (username!=NULL)
		wcsncpy(username_io, username, usernamelen);
	if (password!=NULL)
		wcsncpy(password_io, password, passwordlen);

	if (!PyWinObject_AsCREDUI_INFO(obuiinfo, &uiinfo))
		goto done;

	reterr=CredUIPromptForCredentials(uiinfo, targetname, reserved, autherror,
		username_io, maxusername, password_io, maxpassword,
		&save, flags);
	if (reterr==NO_ERROR)
		ret=Py_BuildValue("uuN", username_io, password_io, PyBool_FromLong(save));
	else
		PyWin_SetAPIError("CredUIPromptForCredentials", reterr);

	done:
	PyWinObject_FreeWCHAR(targetname);
	PyWinObject_FreeWCHAR(username);
	if (password!=NULL){
		SecureZeroMemory(password, passwordlen*sizeof(WCHAR));
		PyWinObject_FreeWCHAR(password);
		}
	if (username_io)
		free(username_io);
	if (password_io){
		SecureZeroMemory(password_io, maxpassword*sizeof(WCHAR));
		free (password_io);
		}
	PyWinObject_FreeCREDUI_INFO(uiinfo);
	return ret;
}

// @pymethod |win32cred|CredUIConfirmCredentials|Confirms whether credentials entered by user are valid or not
// @comm This function should be called to confirm credentials entered via
// <om win32cred.CredUICmdLinePromptForCredentials> or <om win32cred.CredUIPromptForCredentials>
// if CREDUI_FLAGS_EXPECT_CONFIRMATION was passed in Flags to either function.<nl>
// Sequence of operations:<nl>
// Prompt for credentials<nl>
// Authenticate against target using credentials<nl>
// Call this function to indicate if authentication succeeded or not
PyObject *PyCredUIConfirmCredentials(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"TargetName", "Confirm", NULL};
	PyObject *obtargetname, *ret=NULL;
	WCHAR *targetname=NULL;
	BOOL confirm;
	DWORD err;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:CredUIConfirmCredentials", keywords,
		&obtargetname,	// @pyparm <o PyUnicode>|TargetName||Target of credentials that are pending confirmation
		&confirm))		// @pyparm boolean|Confirm||Indicates if authentication succeeded
		return NULL;
	if (!PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE))
		return NULL;
	err=CredUIConfirmCredentials(targetname, confirm);
	if (err==NO_ERROR){
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	else
		PyWin_SetAPIError("CredUIConfirmCredentials", err);
	PyWinObject_FreeWCHAR(targetname);
	return ret;
}

// @pymethod <o PyUnicode>|win32cred|CredUIReadSSOCredW|Retrieves single sign on username
PyObject *PyCredUIReadSSOCredW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Realm", NULL};
	PyObject *obrealm=Py_None, *ret=NULL;
	WCHAR *username=NULL, *realm=NULL;
	DWORD err;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:CredUIReadSSOCredW", keywords,
		&obrealm))	// @pyparm <o PyUnicode>|Realm|None|Realm for which to read username, can be None
		return NULL;
	if (!PyWinObject_AsWCHAR(obrealm, &realm, TRUE))
		return NULL;
	err=CredUIReadSSOCredW(realm, &username);
	if (err==ERROR_SUCCESS)
		ret=PyWinObject_FromWCHAR(username);
	else
		PyWin_SetAPIError("CredUIReadSSOCredW", err);
	PyWinObject_FreeWCHAR(realm);
	if (username)
		LocalFree(username);
	return ret;
}

// @pymethod |win32cred|CredUIStoreSSOCredW|Creates a single sign on credential
PyObject *PyCredUIStoreSSOCredW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Realm", "Username", "Password", "Persist", NULL};
	PyObject *obrealm=Py_None, *obusername, *obpassword, *ret=NULL;
	WCHAR *realm=NULL, *username=NULL, *password=NULL;
	BOOL persist;
	DWORD err, passwordlen=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOOk:CredUIStoreSSOCredW", keywords,
		&obrealm,		// @pyparm <o PyUnicode>|Realm||Realm for which to read username, can be None for default realm
		&obusername,	// @pyparm <o PyUnicode>|Username||Username for realm
		&obpassword,	// @pyparm <o PyUnicode>|Password||User's password
		&persist))		// @pyparm boolean|Persist||Specifies whether to save credential
		return NULL;
	if (PyWinObject_AsWCHAR(obrealm, &realm, TRUE)
		&&PyWinObject_AsWCHAR(obusername, &username, FALSE)
		&&PyWinObject_AsWCHAR(obpassword, &password, FALSE, &passwordlen)){
		err=CredUIStoreSSOCredW(realm, username, password, persist);
		if (err==ERROR_SUCCESS){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("CredUIStoreSSOCredW", err);
		}
	PyWinObject_FreeWCHAR(realm);
	PyWinObject_FreeWCHAR(username);
	if (password){
		SecureZeroMemory(password, passwordlen*sizeof(WCHAR));
		PyWinObject_FreeWCHAR(password);
		}
	return ret;
}

// @pymethod (<o PyUnicode>, <o PyUnicode>)|win32cred|CredUIParseUserName|Parses a full username into domain and username
// @rdesc Returns the username and domain
PyObject *PyCredUIParseUserName(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"UserName", NULL};
	PyObject *obusername, *ret=NULL;
	WCHAR *username=NULL, *username_out=NULL, *domain=NULL;
	// max constants don't include trailing NULL
	ULONG maxusername=CREDUI_MAX_USERNAME_LENGTH+1;
	ULONG maxdomain=CREDUI_MAX_DOMAIN_TARGET_LENGTH+1;
	DWORD err;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CredUIParseUserName", keywords,
		&obusername))	// @pyparm <o PyUnicode>|UserName||Username as returned by <om win32cred.CredUIPromptForCredentials>
		return NULL;
	if (!PyWinObject_AsWCHAR(obusername, &username, FALSE))
		return NULL;

	username_out=(WCHAR *)malloc(maxusername * sizeof(WCHAR));
	if (username_out==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxusername);
		goto done;
		}
	ZeroMemory(username_out, maxusername*sizeof(WCHAR));
	domain=(WCHAR *)malloc(maxdomain * sizeof(WCHAR));
	if (domain==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", maxdomain);
		goto done;
		}
	ZeroMemory(domain, maxdomain*sizeof(WCHAR));

	err=CredUIParseUserName(username,  username_out, maxusername, domain, maxdomain);
	if (err==NO_ERROR)
		ret=Py_BuildValue("uu", username_out, domain);
	else
		PyWin_SetAPIError("CredUIParseUserName", err);

	done:
	PyWinObject_FreeWCHAR(username);
	if (username_out!=NULL)
		free(username_out);
	if (domain!=NULL)
		free(domain);
	return ret;
}


// @module win32cred|Interface to credentials management functions.
// The functions in this module are only available on Windows XP and later.<nl>
// Functions operate only on the credential set of the calling user.<nl>
// User's profile must be loaded for stored credentials to be accessible.<nl>
// Each credential is uniquely identified by its TargetName and Type.<nl>
// All functions accept keyword arguments.
static struct PyMethodDef win32cred_functions[] = {
	// @pymeth CredMarshalCredential|Marshals a credential into a unicode string
	{"CredMarshalCredential", (PyCFunction)PyCredMarshalCredential, METH_VARARGS|METH_KEYWORDS, "Marshals a credential into a unicode string"},     
	// @pymeth CredUnmarshalCredential|Unmarshals credentials formatted using <om win32cred.CredMarshalCredential>
	{"CredUnmarshalCredential", (PyCFunction)PyCredUnmarshalCredential, METH_VARARGS|METH_KEYWORDS, "Unmarshals credentials formatted using <om win32cred.CredMarshalCredential>"},
	// @pymeth CredIsMarshaledCredential|Checks if a string matches the form of a marshaled credential
	{"CredIsMarshaledCredential", (PyCFunction)PyCredIsMarshaledCredential, METH_VARARGS|METH_KEYWORDS, "Checks if a string matches the form of a marshaled credential"},
	// @pymeth CredEnumerate|Lists stored credentials for current logon session
	{"CredEnumerate", (PyCFunction)PyCredEnumerate, METH_VARARGS|METH_KEYWORDS, "Lists stored credentials for current logon session"},	
	// @pymeth CredGetTargetInfo|Determines type and location of credential target
	{"CredGetTargetInfo", (PyCFunction)PyCredGetTargetInfo, METH_VARARGS|METH_KEYWORDS, "Determines type and location of credential target"},	
	// @pymeth CredWriteDomainCredentials|Creates or updates credential for a domain or server
	{"CredWriteDomainCredentials", (PyCFunction)PyCredWriteDomainCredentials, METH_VARARGS|METH_KEYWORDS, "Creates or updates credential for a domain or server"},
	// @pymeth CredReadDomainCredentials|Retrieves a user's credentials for a domain or server
	{"CredReadDomainCredentials", (PyCFunction)PyCredReadDomainCredentials, METH_VARARGS|METH_KEYWORDS, "Retrieves a user's credentials for a domain or server"},
	// @pymeth CredDelete|Deletes a stored credential
	{"CredDelete", (PyCFunction)PyCredDelete, METH_VARARGS|METH_KEYWORDS, "Deletes a stored credential"},
	// @pymeth CredWrite|Creates or updates a stored credential
	{"CredWrite", (PyCFunction)PyCredWrite, METH_VARARGS|METH_KEYWORDS, "Creates or updates a stored credential"},
	// @pymeth CredRead|Retrieves a stored credential
	{"CredRead", (PyCFunction)PyCredRead, METH_VARARGS|METH_KEYWORDS, "Retrieves a stored credential"},
	// @pymeth CredRename|Changes the target name of stored credentials
	{"CredRename", (PyCFunction)PyCredRename, METH_VARARGS|METH_KEYWORDS, "Changes the target name of stored credentials"},
	// @pymeth CredUICmdLinePromptForCredentials|Prompt for username/passwd from a console app
	{"CredUICmdLinePromptForCredentials", (PyCFunction)PyCredUICmdLinePromptForCredentials, METH_VARARGS|METH_KEYWORDS, "Prompt for username/passwd from a console app"},
	// @pymeth CredUIPromptForCredentials|Initiates dialog to request user credentials
	{"CredUIPromptForCredentials", (PyCFunction)PyCredUIPromptForCredentials, METH_VARARGS|METH_KEYWORDS, "Initiates dialog to request user credentials"},
	// @pymeth CredUIConfirmCredentials|Confirms whether credentials entered by user are valid or not
	{"CredUIConfirmCredentials", (PyCFunction)PyCredUIConfirmCredentials, METH_VARARGS|METH_KEYWORDS, "Confirms whether credentials entered by user are valid or not"},
	// @pymeth CredUIReadSSOCredW|Retrieves single sign on username
	{"CredUIReadSSOCredW", (PyCFunction)PyCredUIReadSSOCredW, METH_VARARGS|METH_KEYWORDS, "Retrieves single sign on username"},
	// @pymeth CredUIStoreSSOCredW|Creates a single sign on credential
	{"CredUIStoreSSOCredW", (PyCFunction)PyCredUIStoreSSOCredW, METH_VARARGS|METH_KEYWORDS, "Creates a single sign on credential"},
	// @pymeth CredUIParseUserName|Parses a full username into domain and username
	{"CredUIParseUserName", (PyCFunction)PyCredUIParseUserName, METH_VARARGS|METH_KEYWORDS, "Parses a full username into domain and username"},
	{NULL,	NULL}
};


PYWIN_MODULE_INIT_FUNC(win32cred)
{
	PYWIN_MODULE_INIT_PREPARE(win32cred, win32cred_functions,
				  "Interface to credentials management functions.");

	// CRED_MARSHAL_TYPE used with CredMarshalCredential and CredUnmarshalCredential
	PyModule_AddIntConstant(module, "CertCredential", CertCredential);
	PyModule_AddIntConstant(module, "UsernameTargetCredential", UsernameTargetCredential);
	// credential types
	PyModule_AddIntConstant(module, "CRED_TYPE_GENERIC", CRED_TYPE_GENERIC);
	PyModule_AddIntConstant(module, "CRED_TYPE_DOMAIN_PASSWORD", CRED_TYPE_DOMAIN_PASSWORD);
	PyModule_AddIntConstant(module, "CRED_TYPE_DOMAIN_CERTIFICATE", CRED_TYPE_DOMAIN_CERTIFICATE);
	PyModule_AddIntConstant(module, "CRED_TYPE_DOMAIN_VISIBLE_PASSWORD", CRED_TYPE_DOMAIN_VISIBLE_PASSWORD);
	// credential flags
	PyModule_AddIntConstant(module, "CRED_FLAGS_PROMPT_NOW", CRED_FLAGS_PROMPT_NOW);
	PyModule_AddIntConstant(module, "CRED_FLAGS_USERNAME_TARGET", CRED_FLAGS_USERNAME_TARGET);
	PyModule_AddIntConstant(module, "CRED_FLAGS_PASSWORD_FOR_CERT", CRED_FLAGS_PASSWORD_FOR_CERT);
	PyModule_AddIntConstant(module, "CRED_FLAGS_OWF_CRED_BLOB", CRED_FLAGS_OWF_CRED_BLOB);
	PyModule_AddIntConstant(module, "CRED_FLAGS_VALID_FLAGS", CRED_FLAGS_VALID_FLAGS);
	// persistence flags
	PyModule_AddIntConstant(module, "CRED_PERSIST_NONE", CRED_PERSIST_NONE);
	PyModule_AddIntConstant(module, "CRED_PERSIST_SESSION", CRED_PERSIST_SESSION);
	PyModule_AddIntConstant(module, "CRED_PERSIST_LOCAL_MACHINE", CRED_PERSIST_LOCAL_MACHINE);
	PyModule_AddIntConstant(module, "CRED_PERSIST_ENTERPRISE", CRED_PERSIST_ENTERPRISE);
	// CREDENTIAL_TARGET_INFORMATION flags
	PyModule_AddIntConstant(module, "CRED_TI_SERVER_FORMAT_UNKNOWN", CRED_TI_SERVER_FORMAT_UNKNOWN);
	PyModule_AddIntConstant(module, "CRED_TI_DOMAIN_FORMAT_UNKNOWN", CRED_TI_DOMAIN_FORMAT_UNKNOWN);
	PyModule_AddIntConstant(module, "CRED_TI_ONLY_PASSWORD_REQUIRED", CRED_TI_ONLY_PASSWORD_REQUIRED);
	PyModule_AddIntConstant(module, "CRED_TI_USERNAME_TARGET", CRED_TI_USERNAME_TARGET);
	PyModule_AddIntConstant(module, "CRED_TI_CREATE_EXPLICIT_CRED", CRED_TI_CREATE_EXPLICIT_CRED);
	PyModule_AddIntConstant(module, "CRED_TI_WORKGROUP_MEMBER", CRED_TI_WORKGROUP_MEMBER);
	PyModule_AddIntConstant(module, "CRED_TI_VALID_FLAGS", CRED_TI_VALID_FLAGS);
	// CredGetTargetInfo flag
	PyModule_AddIntConstant(module, "CRED_ALLOW_NAME_RESOLUTION", CRED_ALLOW_NAME_RESOLUTION);
	// flag used with CredReadDomainCredentials
	PyModule_AddIntConstant(module, "CRED_CACHE_TARGET_INFORMATION", CRED_CACHE_TARGET_INFORMATION);
	// flag used with CredWriteDomainCredentials
	PyModule_AddIntConstant(module, "CRED_PRESERVE_CREDENTIAL_BLOB", CRED_PRESERVE_CREDENTIAL_BLOB);
	// CredUIPromptForCredentials/CredUICmdLinePromptForCredentials options
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_PROMPT_VALID", CREDUI_FLAGS_PROMPT_VALID);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_INCORRECT_PASSWORD", CREDUI_FLAGS_INCORRECT_PASSWORD);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_DO_NOT_PERSIST", CREDUI_FLAGS_DO_NOT_PERSIST);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_REQUEST_ADMINISTRATOR", CREDUI_FLAGS_REQUEST_ADMINISTRATOR);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_EXCLUDE_CERTIFICATES", CREDUI_FLAGS_EXCLUDE_CERTIFICATES);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_REQUIRE_CERTIFICATE", CREDUI_FLAGS_REQUIRE_CERTIFICATE);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX", CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_ALWAYS_SHOW_UI", CREDUI_FLAGS_ALWAYS_SHOW_UI);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_REQUIRE_SMARTCARD", CREDUI_FLAGS_REQUIRE_SMARTCARD);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_PASSWORD_ONLY_OK", CREDUI_FLAGS_PASSWORD_ONLY_OK);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_VALIDATE_USERNAME", CREDUI_FLAGS_VALIDATE_USERNAME);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_COMPLETE_USERNAME", CREDUI_FLAGS_COMPLETE_USERNAME);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_PERSIST", CREDUI_FLAGS_PERSIST);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_SERVER_CREDENTIAL", CREDUI_FLAGS_SERVER_CREDENTIAL);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_EXPECT_CONFIRMATION", CREDUI_FLAGS_EXPECT_CONFIRMATION);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_GENERIC_CREDENTIALS", CREDUI_FLAGS_GENERIC_CREDENTIALS);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS", CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS);
	PyModule_AddIntConstant(module, "CREDUI_FLAGS_KEEP_USERNAME", CREDUI_FLAGS_KEEP_USERNAME);
	// size limits for various credential strings
	PyModule_AddIntConstant(module, "CRED_MAX_STRING_LENGTH", CRED_MAX_STRING_LENGTH);
	PyModule_AddIntConstant(module, "CRED_MAX_USERNAME_LENGTH", CRED_MAX_USERNAME_LENGTH);
	PyModule_AddIntConstant(module, "CRED_MAX_GENERIC_TARGET_NAME_LENGTH", CRED_MAX_GENERIC_TARGET_NAME_LENGTH);
	PyModule_AddIntConstant(module, "CRED_MAX_DOMAIN_TARGET_NAME_LENGTH", CRED_MAX_DOMAIN_TARGET_NAME_LENGTH);
	PyModule_AddIntConstant(module, "CRED_MAX_VALUE_SIZE", CRED_MAX_VALUE_SIZE);
	PyModule_AddIntConstant(module, "CRED_MAX_ATTRIBUTES", CRED_MAX_ATTRIBUTES);
	PyModule_AddIntConstant(module, "CREDUI_MAX_MESSAGE_LENGTH", CREDUI_MAX_MESSAGE_LENGTH);
	PyModule_AddIntConstant(module, "CREDUI_MAX_CAPTION_LENGTH", CREDUI_MAX_CAPTION_LENGTH);
	PyModule_AddIntConstant(module, "CREDUI_MAX_GENERIC_TARGET_LENGTH", CREDUI_MAX_GENERIC_TARGET_LENGTH);
	PyModule_AddIntConstant(module, "CREDUI_MAX_DOMAIN_TARGET_LENGTH", CREDUI_MAX_DOMAIN_TARGET_LENGTH);
	PyModule_AddIntConstant(module, "CREDUI_MAX_USERNAME_LENGTH", CREDUI_MAX_USERNAME_LENGTH);
	PyModule_AddIntConstant(module, "CREDUI_MAX_PASSWORD_LENGTH", CREDUI_MAX_PASSWORD_LENGTH);

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
