/* File : win32security.i */
// @doc

%module win32security // An interface to the win32 security API's

%{
#define _WIN32_WINNT 0x0400 // make sure we get all the constants.
%}


%include "typemaps.i"
%include "pywin32.i"

%{
#include "PySecurityObjects.h"
#include "accctrl.h"
#include "aclapi.h"
%}

%apply LARGE_INTEGER {LUID};
typedef LARGE_INTEGER LUID;
%typemap(python,ignore) LUID *OUTPUT(LUID temp)
{
  $target = &temp;
}
%typemap(python,argout) LUID *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromLARGE_INTEGER(*((LARGE_INTEGER *)$source));
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}

// @object PyTOKEN_PRIVILEGES|An object representing Win32 token privileges.
// @comm This is a sequence (eg, list) of (id, attributes)
%{
PyObject *PyWinObject_FromTOKEN_PRIVILEGES(TOKEN_PRIVILEGES *pPriv)
{
	PyErr_SetString(PyExc_RuntimeError, "Not yet implemented");
	return NULL;
}

BOOL PyWinObject_AsTOKEN_PRIVILEGES(PyObject *ob, TOKEN_PRIVILEGES **ppRest, BOOL bNoneOK /*= TRUE*/)
{
	BOOL ok = FALSE;
	char *errMsg = "A TOKEN_PRIVILEGES object must be a tuple of (LARGE_INTEGER, int)";
	PyObject *subObj = NULL;
	if (!PySequence_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, errMsg);
		return NULL;
	}
	int num = PySequence_Length(ob);
	TOKEN_PRIVILEGES *pRet = (TOKEN_PRIVILEGES *)malloc(sizeof(LUID_AND_ATTRIBUTES) * num);
	if (pRet==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating TOKEN_PRIVILEGES");
		return NULL;
	}
	pRet->PrivilegeCount = num;
	for (int i =0;i<num;i++) {
		subObj = PySequence_GetItem(ob, i);
		if (subObj==NULL)
			goto done;
		if (!PySequence_Check(subObj)) {
			PyErr_SetString(PyExc_TypeError, errMsg);
			goto done;
		}
		PyObject *obLUID;
		if (!PyArg_ParseTuple(subObj, "Ol", &obLUID, &pRet->Privileges[i].Attributes))
			goto done;
		if (!PyWinObject_AsLARGE_INTEGER(obLUID, (LARGE_INTEGER *)&pRet->Privileges[i].Luid))
			goto done;
		Py_DECREF(subObj);
		subObj = NULL;
	}
	ok = TRUE;
done:
	Py_XDECREF(subObj);
	if (ok)
		*ppRest = pRet;
	else
		free(pRet);
	return ok;
}

void PyMAPIObject_FreeTOKEN_PRIVILEGES(TOKEN_PRIVILEGES *pPriv)
{
	free(pPriv);
}

%}

%typemap(python,in) TOKEN_PRIVILEGES *{
	if (!PyWinObject_AsTOKEN_PRIVILEGES($source, &$target, FALSE))
		return NULL;
}
%typemap(python,freearg) TOKEN_PRIVILEGES * {
	if ($source) PyMAPIObject_FreeTOKEN_PRIVILEGES($source);
}

%typemap(python,ignore) TOKEN_PRIVILEGES *OUTPUT(TOKEN_PRIVILEGES temp)
{
  $target = &temp;
}

%typemap(python,out) TOKEN_PRIVILEGES {
  $target = PyWinObject_FromTOKEN_PRIVILEGES($source);
}

%typemap(python,argout) TOKEN_PRIVILEGES *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromTOKEN_PRIVILEGES(*$source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}


%init %{
	// All errors raised by this module are of this type.
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);
%}

// @pyswig PyACL|ACL|Creates a new <o PyACL> object.
// @pyparm int|bufSize|64|The size of the buffer for the ACL.
%native(ACL) PyWinMethod_NewACL;
// @pyswig PySID|SID|Creates a new <o PySID> object.
%native(SID) PyWinMethod_NewSID;
// @pyswig PySECURITY_ATTRIBUTES|SECURITY_ATTRIBUTES|Creates a new <o PySECURITY_ATTRIBUTES> object.
%native(SECURITY_ATTRIBUTES) PyWinMethod_NewSECURITY_ATTRIBUTES;
// @pyswig PySECURITY_DESCRIPTOR|SECURITY_DESCRIPTOR|Creates a new <o PySECURITY_DESCRIPTOR> object.
%native(SECURITY_DESCRIPTOR) PyWinMethod_NewSECURITY_DESCRIPTOR;

// @pyswig |ImpersonateNamedPipeClient|Impersonates a named-pipe client application.
BOOLAPI ImpersonateNamedPipeClient(
	PyHANDLE hNamedPipe // @pyparm int|handle||handle of a named pipe.
);

// @pyswig |ImpersonateLoggedOnUser|Impersonates a logged on user.
BOOLAPI ImpersonateLoggedOnUser(
  PyHANDLE hToken  // @pyparm <o PyHANDLE>|handle||Handle to a token that represents a logged-on user
); 

// @pyswig |RevertToSelf|Terminates the impersonation of a client application.
BOOLAPI RevertToSelf();

// @pyswig <o PyHANDLE>|LogonUser|Attempts to log a user on to the local computer, that is, to the computer from which LogonUser was called. You cannot use LogonUser to log on to a remote computer.
BOOLAPI LogonUser(
    TCHAR *userName, // @pyparm string|userName||The name of the user account to log on to. 
    TCHAR *INPUT_NULLOK, // @pyparm string|domain||The name of the domain, or None for the current domain
    TCHAR *password, // @pyparm string|password||The password to use.
    DWORD logonType, // @pyparm int|logonType||Specifies the type of logon operation to perform.  Must be a combination of the LOGON32_LOGON* constants.
    DWORD logonProvider, // @pyparm int|logonProvider||Specifies the logon provider to use.
    PyHANDLE *OUTPUT
);


// @pyswig <o PySID>, string, int|LookupAccountName|Accepts the name of a system and an account as input. It retrieves a security identifier (SID) for the account and the name of the domain on which the account was found.
// @rdesc The result is a tuple of new SID object, the domain name where the account was found, and the type of account the SID is for.
%native(LookupAccountName) LookupAccountName;
%{
PyObject *LookupAccountName(PyObject *self, PyObject *args)
{
	PyObject *obSystemName;
	PyObject *obAcctName;
	PyObject *obDomain = NULL;
	TCHAR *szAcctName = NULL;
	TCHAR *szSystemName = NULL;
	TCHAR refDomain[MAX_PATH+1];
	DWORD refDomainSize = sizeof(refDomain);
	PSID pSid = NULL;
	DWORD sidSize = 0;
	PyObject *obNewSid = NULL; // XDECREF's on failure.
	SID_NAME_USE sidType;
	PyObject *result = NULL;

	if (!PyArg_ParseTuple(args, "OO:LookupAccountName", 
	                 &obSystemName, // @pyparm string|systemName||The system name, or None
					 &obAcctName))  // @pyparm string|accountName||The account name
		goto done;

	if (!PyWinObject_AsTCHAR(obSystemName, &szSystemName, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obAcctName, &szAcctName, FALSE))
		goto done;

	// Get the SID size.
	LookupAccountName(szSystemName, szAcctName, pSid, &sidSize, refDomain, &refDomainSize, &sidType);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		PyWin_SetAPIError("LookupAccountName");
		goto done;
	}

	obNewSid = new PySID(sidSize);
	PyWinObject_AsSID(obNewSid, &pSid);

	if (!LookupAccountName(szSystemName, szAcctName, pSid, &sidSize, refDomain, &refDomainSize, &sidType)) {
		PyWin_SetAPIError("LookupAccountName");
		goto done;
	}
	obDomain = PyWinObject_FromTCHAR(refDomain);
	result = Py_BuildValue("OOl", obNewSid, obDomain, sidType);

done:
	PyWinObject_FreeTCHAR(szSystemName);
	PyWinObject_FreeTCHAR(szAcctName);
	Py_XDECREF(obDomain);
	Py_XDECREF(obNewSid);
	return result;
}
%}

// @pyswig string, string, int|LookupAccountSid|Accepts a security identifier (SID) as input. It retrieves the name of the account for this SID and the name of the first domain on which this SID is found.
// @rdesc The result is a tuple of the name, the domain name where the account was found, and the type of account the SID is for.
%native(LookupAccountSid) LookupAccountSid;
%{
PyObject *LookupAccountSid(PyObject *self, PyObject *args)
{
	PyObject *obSystemName;
	PyObject *obRetAcctName = NULL;
	PyObject *obDomain = NULL;
	TCHAR szRetAcctName[256];
	DWORD retAcctNameSize = sizeof(szRetAcctName)/sizeof(TCHAR);
	TCHAR *szSystemName = NULL;
	TCHAR refDomain[256];
	DWORD refDomainSize = sizeof(refDomain)/sizeof(TCHAR);
	PSID pSid;
	PyObject *obSid;
	SID_NAME_USE sidType;
	PyObject *result = NULL;

	if (!PyArg_ParseTuple(args, "OO:LookupAccountSid", 
	                 &obSystemName, // @pyparm string|systemName||The system name, or None
					 &obSid))  // @pyparm <o PySID>|sid||The SID
		goto done;

	if (!PyWinObject_AsTCHAR(obSystemName, &szSystemName, TRUE))
		goto done;

	if (!PyWinObject_AsSID(obSid, &pSid))
		goto done;

	if (!LookupAccountSid(szSystemName, pSid, szRetAcctName, &retAcctNameSize, refDomain, &refDomainSize, &sidType)) {
		PyWin_SetAPIError("LookupAccountSid");
		goto done;
	}
	obRetAcctName = PyWinObject_FromTCHAR(szRetAcctName);
	obDomain = PyWinObject_FromTCHAR(refDomain);
	result = Py_BuildValue("OOl", obRetAcctName, obDomain, sidType);

done:
	PyWinObject_FreeTCHAR(szSystemName);
	Py_XDECREF(obRetAcctName);
	Py_XDECREF(obDomain);
	return result;
}
%}

%native(SetSecurityInfo) SetSecurityInfo;
%{
PyObject *SetSecurityInfo(PyObject *self, PyObject *args)
{
	PSID pSidOwner = NULL;
	PSID pSidGroup = NULL;
	PACL pDacl= NULL;
	PACL pSacl = NULL;
	PyObject *obHandle;
	PyObject *obSidOwner = Py_None;
	PyObject *obSidGroup = Py_None;
	PyObject *obDacl = Py_None;
	PyObject *obSacl = Py_None;
	SECURITY_INFORMATION info = 0;
	SE_OBJECT_TYPE typeHandle;
	HANDLE handle;

	if (!PyArg_ParseTuple(args, "Ol|OOOO:SetSecurityInfo",
				&obHandle,
				(long *)(&typeHandle),
				&obSidOwner,
				&obSidGroup,
				&obDacl,
				&obSacl))
		return NULL;

	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE ))
		return NULL;
	if (obSidOwner!=Py_None) {
		info |= OWNER_SECURITY_INFORMATION;
		if (!PyWinObject_AsSID(obSidOwner, &pSidOwner, FALSE))
			return NULL;
	}
	if (obSidGroup!=Py_None) {
		info |= GROUP_SECURITY_INFORMATION;
		if (!PyWinObject_AsSID(obSidGroup, &pSidGroup, FALSE))
			return NULL;
	}
	if (obDacl!=Py_None) {
		info |= DACL_SECURITY_INFORMATION;
		if (!PyWinObject_AsACL(obDacl, &pDacl, FALSE))
			return NULL;
	}
	if (obSacl!=Py_None) {
		info |= SACL_SECURITY_INFORMATION;
		if (!PyWinObject_AsACL(obSacl, &pSacl, FALSE))
			return NULL;
	}
	if (info==0) {
		PyErr_SetString(PyExc_TypeError, "No new security information was specified");
		return NULL;
	}
	if (!SetSecurityInfo(handle, typeHandle, info, pSidOwner, pSidGroup, pDacl, pSacl))
		return PyWin_SetAPIError("SetSecurityInfo");

	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig int|OpenProcessToken|
BOOLAPI OpenProcessToken(
	PyHANDLE ProcessHandle, // @pyparm int|processHandle||The handle of the process to open.
	DWORD DesiredAccess, // @pyparm int|desiredAccess||Desired access to process 
	HANDLE *OUTPUT
);

// @pyswig <o LARGE_INTEGER>|LookupPrivilegeValue|
BOOLAPI LookupPrivilegeValue(
	TCHAR *INPUT_NULLOK, // @pyparm string|systemName||String specifying the system
	TCHAR *lpName, // @pyparm string|privilegeName||String specifying the privilege
	LUID *OUTPUT 
); 
 
%{
BOOL MyAdjustTokenPrivileges(
	HANDLE TokenHandle,
	BOOL DisableAllPrivileges,
	TOKEN_PRIVILEGES *NewState)
{
	AdjustTokenPrivileges(TokenHandle, DisableAllPrivileges, NewState, 0, NULL, 0);
	return GetLastError()==0;
}
%}

// @pyswig |AdjustTokenPrivileges|
%name(AdjustTokenPrivileges) BOOLAPI MyAdjustTokenPrivileges(
	HANDLE TokenHandle, // @pyparm int|handle||handle to token that contains privileges
	BOOL DisableAllPrivileges, // @pyparm int|bDisableAllPrivileges||Flag for disabling all privileges
	TOKEN_PRIVILEGES *NewState // @pyparm <o PyTOKEN_PRIVILEGES>|NewState||The new state
);

// @pyswig object|GetTokenInformation|Retrieves a specified type of information about an access token. The calling process must have appropriate access rights to obtain the information.
%native(GetTokenInformation) PyGetTokenInformation;
%{
static PyObject *PyGetTokenInformation(PyObject *self, PyObject *args)
{
	int bufSize = 1024;
	PyObject *obHandle;
	TOKEN_INFORMATION_CLASS typ;
	if (!PyArg_ParseTuple(args, "Ol|i", 
		&obHandle, // @pyparm <o PyHANDLE>|handle||The handle to query the information for.
		(long *)&typ, // @pyparm int|TokenInformationClass||Specifies a value from the TOKEN_INFORMATION_CLASS enumerated type identifying the type of information the function retrieves.
		&bufSize)) // @pyparm int|bufSize|1024|The buffer size to use to query the information
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;

	DWORD retLength = 0;
	PyObject *ret = NULL;
	void *buf = malloc(bufSize);
	if (bufSize==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating buffer for token info");
		return NULL;
	}
	if (!GetTokenInformation(handle, typ, buf, bufSize, &retLength)) {
		PyWin_SetAPIError("GetTokenInformation");
		goto done;
	}
	// @rdesc The following types are supported
	// @flagh TokenInformationClass|Return type
	switch (typ) {
		case TokenUser: {
			// @flag TokenUser|(<o PySID>,int)
			TOKEN_USER *p = (TOKEN_USER *)buf;
			PyObject *obSid = PyWinObject_FromSID(p->User.Sid);
			ret = Py_BuildValue("(Ol)", obSid, p->User.Attributes );
			Py_XDECREF(obSid);
			break;
			}
		default:
			PyErr_SetString(PyExc_TypeError, "The TokenInformationClass param is not supported");
			break;
	}
done:
	free(buf);
	return ret;
}
%}

// @pyswig <o PyHandle>|OpenThreadToken|Opens the access token associated with a thread.
BOOLAPI OpenThreadToken(
  PyHANDLE ThreadHandle,  // @pyparm <o PyHANDLE>|handle||handle to thread
  DWORD DesiredAccess,  // @pyparm int|desiredAccess||access to process
  BOOL OpenAsSelf,      // @pyparm int|openAsSelf||Flag for process or thread security
  PyHANDLE *OUTPUT
);

// @pyswig <o PySECURITY_DESCRIPTOR>|GetFileSecurity|Obtains specified information about the security of a file or directory. The information obtained is constrained by the caller's access rights and privileges.
%native(GetFileSecurity) MyGetFileSecurity;
%{
static PyObject *MyGetFileSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obFname;
	unsigned long info = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION ;

	// @pyparm string|filename||The name of the file
	// @pyparm int|info|OWNER_SECURITY_INFORMATION \| GROUP_SECURITY_INFORMATION \| DACL_SECURITY_INFORMATION \| SACL_SECURITY_INFORMATION|Flags that specify the information requested.
	if (!PyArg_ParseTuple(args, "O|l", &obFname, &info))
		return NULL;

	SECURITY_DESCRIPTOR *psd = NULL;
	DWORD dwSize = 0;
	TCHAR *fname = NULL;
	if (!PyWinObject_AsTCHAR(obFname, &fname))
		goto done;

	if (GetFileSecurity(fname, info, psd, dwSize, &dwSize)) {
		PyErr_SetString(PyExc_RuntimeError, "Can't query for SECURITY_DESCRIPTOR size info?");
		goto done;
	}
	psd = (SECURITY_DESCRIPTOR *)malloc(dwSize);
	if (psd==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating SECURITY_DESCRIPTOR");
		goto done;
	}
    if (!GetFileSecurity(fname, info, psd, dwSize, &dwSize)) {
		PyWin_SetAPIError("GetFileSecurity");
		goto done;
	}
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd, dwSize);
done:
	PyWinObject_FreeTCHAR(fname);
	return rc;
}
%}

// @pyswig |SetFileSecurity|Sets information about the security of a file or directory. The information obtained is constrained by the caller's access rights and privileges.
%native(SetFileSecurity) MySetFileSecurity;
%{
static PyObject *MySetFileSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obFname;
	PyObject *obsd;
	unsigned long info;

	// @pyparm string|filename||The name of the file
	// @pyparm int|info||The type of information to set.
	// @pyparm <o PySECURITY_DESCRIPTOR>|security||The security information
	if (!PyArg_ParseTuple(args, "OlO", &obFname, &info, &obsd))
		return NULL;

	TCHAR *fname = NULL;
	if (!PyWinObject_AsTCHAR(obFname, &fname))
		goto done;

	SECURITY_DESCRIPTOR *psd;
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obsd, &psd))
		goto done;
	if (SetFileSecurity(fname, info, psd)) {
		PyWin_SetAPIError("SetFileSecurity");
		goto done;
	}
	rc = Py_None;
	Py_INCREF(rc);
done:
	PyWinObject_FreeTCHAR(fname);
	return rc;
}
%}

// @pyswig <o PySECURITY_DESCRIPTOR>|GetUserObjectSecurity|Obtains specified information about the security of a user object. The information obtained is constrained by the caller's access rights and privileges.
%native(GetUserObjectSecurity) MyGetUserObjectSecurity;
%{
static PyObject *MyGetUserObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obHandle;
	unsigned long info = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION ;

	// @pyparm <o PyHANDLE>|handle||The handle to the object
	// @pyparm int|info|OWNER_SECURITY_INFORMATION \| GROUP_SECURITY_INFORMATION \| DACL_SECURITY_INFORMATION \| SACL_SECURITY_INFORMATION|Flags that specify the information requested.
	if (!PyArg_ParseTuple(args, "O|l", &obHandle, &info))
		return NULL;

	SECURITY_DESCRIPTOR *psd = NULL;
	DWORD dwSize = 0;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		goto done;

	if (GetUserObjectSecurity(handle, &info, psd, dwSize, &dwSize)) {
		PyErr_SetString(PyExc_RuntimeError, "Can't query for SECURITY_DESCRIPTOR size info?");
		goto done;
	}
	psd = (SECURITY_DESCRIPTOR *)malloc(dwSize);
	if (psd==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating SECURITY_DESCRIPTOR");
		goto done;
	}
    if (!GetUserObjectSecurity(handle, &info, psd, dwSize, &dwSize)) {
		PyWin_SetAPIError("GetUserObjectSecurity");
		goto done;
	}
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd, dwSize);
done:
	return rc;
}
%}

// @pyswig |SetUserObjectSecurity|Sets information about the security of a user object. The information obtained is constrained by the caller's access rights and privileges.
%native(SetUserObjectSecurity) MySetUserObjectSecurity;
%{
static PyObject *MySetUserObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obHandle;
	PyObject *obsd;
	unsigned long info;

	// @pyparm <o PyHANDLE>|handle||The handle to set.
	// @pyparm int|info||The type of information to set.
	// @pyparm <o PySECURITY_DESCRIPTOR>|security||The security information
	if (!PyArg_ParseTuple(args, "OlO", &obHandle, &info, &obsd))
		return NULL;

	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		goto done;

	SECURITY_DESCRIPTOR *psd;
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obsd, &psd))
		goto done;
	if (SetUserObjectSecurity(handle, &info, psd)) {
		PyWin_SetAPIError("SetUserObjectSecurity");
		goto done;
	}
	rc = Py_None;
	Py_INCREF(rc);
done:
	return rc;
}
%}

// @pyswig <o PySECURITY_DESCRIPTOR>|GetKernelObjectSecurity|Obtains specified information about the security of a kernel object. The information obtained is constrained by the caller's access rights and privileges.
%native(GetKernelObjectSecurity) MyGetKernelObjectSecurity;
%{
static PyObject *MyGetKernelObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obHandle;
	unsigned long info = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION ;

	// @pyparm <o PyHANDLE>|handle||The handle to the object
	// @pyparm int|info|OWNER_SECURITY_INFORMATION \| GROUP_SECURITY_INFORMATION \| DACL_SECURITY_INFORMATION \| SACL_SECURITY_INFORMATION|Flags that specify the information requested.
	if (!PyArg_ParseTuple(args, "O|l", &obHandle, &info))
		return NULL;

	SECURITY_DESCRIPTOR *psd = NULL;
	DWORD dwSize = 0;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		goto done;

	if (GetKernelObjectSecurity(handle, info, psd, dwSize, &dwSize)) {
		PyErr_SetString(PyExc_RuntimeError, "Can't query for SECURITY_DESCRIPTOR size info?");
		goto done;
	}
	psd = (SECURITY_DESCRIPTOR *)malloc(dwSize);
	if (psd==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating SECURITY_DESCRIPTOR");
		goto done;
	}
    if (!GetKernelObjectSecurity(handle, info, psd, dwSize, &dwSize)) {
		PyWin_SetAPIError("GetKernelObjectSecurity");
		goto done;
	}
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd, dwSize);
done:
	return rc;
}
%}

// @pyswig |SetKernelObjectSecurity|Sets information about the security of a kernel object. The information obtained is constrained by the caller's access rights and privileges.
%native(SetKernelObjectSecurity) MySetKernelObjectSecurity;
%{
static PyObject *MySetKernelObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *rc = NULL;
	PyObject *obHandle;
	PyObject *obsd;
	unsigned long info;

	// @pyparm <o PyHANDLE>|handle||The handle to set.
	// @pyparm int|info||The type of information to set.
	// @pyparm <o PySECURITY_DESCRIPTOR>|security||The security information
	if (!PyArg_ParseTuple(args, "OlO", &obHandle, &info, &obsd))
		return NULL;

	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		goto done;

	SECURITY_DESCRIPTOR *psd;
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obsd, &psd))
		goto done;
	if (SetKernelObjectSecurity(handle, info, psd)) {
		PyWin_SetAPIError("SetKernelObjectSecurity");
		goto done;
	}
	rc = Py_None;
	Py_INCREF(rc);
done:
	return rc;
}
%}

#define TOKEN_ADJUST_DEFAULT TOKEN_ADJUST_DEFAULT // Required to change the default ACL, primary group, or owner of an access token.
#define TOKEN_ADJUST_GROUPS TOKEN_ADJUST_GROUPS // Required to change the groups specified in an access token.
#define TOKEN_ADJUST_PRIVILEGES TOKEN_ADJUST_PRIVILEGES // Required to change the privileges specified in an access token.
#define TOKEN_ALL_ACCESS TOKEN_ALL_ACCESS // Combines the STANDARD_RIGHTS_REQUIRED standard access rights and all individual access rights for tokens. 
#define TOKEN_ASSIGN_PRIMARY TOKEN_ASSIGN_PRIMARY // Required to attach a primary token to a process in addition to the SE_CREATE_TOKEN_NAME privilege. 
#define TOKEN_DUPLICATE TOKEN_DUPLICATE // Required to duplicate an access token. 
#define TOKEN_EXECUTE TOKEN_EXECUTE // Combines the STANDARD_RIGHTS_EXECUTE standard access rights and the TOKEN_IMPERSONATE access right. 
#define TOKEN_IMPERSONATE TOKEN_IMPERSONATE // Required to attach an impersonation access token to a process. 
#define TOKEN_QUERY TOKEN_QUERY // Required to query the contents of an access token. 
#define TOKEN_QUERY_SOURCE TOKEN_QUERY_SOURCE // Required to query the source of an access token. 
#define TOKEN_READ TOKEN_READ // Combines the STANDARD_RIGHTS_READ standard access rights and the TOKEN_QUERY access right. 
#define TOKEN_WRITE TOKEN_WRITE // Combines the STANDARD_RIGHTS_WRITE standard access rights and the TOKEN_ADJUST_PRIVILEGES, TOKEN_ADJUST_GROUPS, and TOKEN_ADJUST_DEFAULT access rights. 
 

#define SE_UNKNOWN_OBJECT_TYPE SE_UNKNOWN_OBJECT_TYPE
#define SE_FILE_OBJECT SE_FILE_OBJECT
#define SE_SERVICE SE_SERVICE
#define SE_PRINTER SE_PRINTER
#define SE_REGISTRY_KEY SE_REGISTRY_KEY
#define SE_LMSHARE SE_LMSHARE
#define SE_KERNEL_OBJECT SE_KERNEL_OBJECT
#define SE_WINDOW_OBJECT SE_WINDOW_OBJECT

#define OWNER_SECURITY_INFORMATION OWNER_SECURITY_INFORMATION // Indicates the owner identifier of the object is being referenced. 
#define GROUP_SECURITY_INFORMATION GROUP_SECURITY_INFORMATION // Indicates the primary group identifier of the object is being referenced. 
#define DACL_SECURITY_INFORMATION DACL_SECURITY_INFORMATION // Indicates the discretionary ACL of the object is being referenced. 
#define SACL_SECURITY_INFORMATION SACL_SECURITY_INFORMATION // Indicates the system ACL of the object is being referenced. 

/** if (_WIN32_WINNT >= 0x0500)
#define SE_DS_OBJECT SE_DS_OBJECT
#define SE_DS_OBJECT_ALL SE_DS_OBJECT_ALL
#define SE_PROVIDER_DEFINED_OBJECT SE_PROVIDER_DEFINED_OBJECT
**/

#define SidTypeUser SidTypeUser // Indicates a user SID. 
 
#define SidTypeGroup SidTypeGroup // Indicates a group SID. 
 
#define SidTypeDomain SidTypeDomain // Indicates a domain SID. 
 
#define SidTypeAlias SidTypeAlias // Indicates an alias SID. 
 
#define SidTypeWellKnownGroup SidTypeWellKnownGroup // Indicates an SID for a well-known group. 
 
#define SidTypeDeletedAccount SidTypeDeletedAccount // Indicates an SID for a deleted account. 
 
#define SidTypeInvalid SidTypeInvalid // Indicates an invalid SID. 
 
#define SidTypeUnknown SidTypeUnknown // Indicates an unknown SID type. 
 

#define LOGON32_LOGON_BATCH LOGON32_LOGON_BATCH //This logon type is intended for batch servers, where processes may be executing on behalf of a user without their direct intervention; or for higher performance servers that process many clear-text authentication attempts at a time, such as mail or web servers. LogonUser does not cache credentials for this logon type. 
#define LOGON32_LOGON_INTERACTIVE LOGON32_LOGON_INTERACTIVE //This logon type is intended for users who will be interactively using the machine, such as a user being logged on by a terminal server, remote shell, or similar process. This logon type has the additional expense of caching logon information for disconnected operation, and is therefore inappropriate for some client/server applications, such as a mail server. 
#define LOGON32_LOGON_SERVICE LOGON32_LOGON_SERVICE // Indicates a service-type logon. The account provided must have the service privilege enabled. 
#define LOGON32_LOGON_NETWORK LOGON32_LOGON_NETWORK // This logon type is intended for high performance servers to authenticate clear text passwords. LogonUser does not cache credentials for this logon type. This is the fastest logon path, but there are two limitations. First, the function returns an impersonation token, not a primary token. You cannot use this token directly in the CreateProcessAsUser function. However, you can call the DuplicateTokenEx function to convert the token to a primary token, and then use it in CreateProcessAsUser. Second, if you convert the token to a primary token and use it in CreateProcessAsUser to start a process, the new process will not be able to access other network resources, such as remote servers or printers, through the redirector.

#define LOGON32_PROVIDER_DEFAULT LOGON32_PROVIDER_DEFAULT // Use the standard logon provider for the system. This is the recommended value for the dwLogonProvider parameter. It provides maximum compatibility with current and future releases of Windows NT.  
#define LOGON32_PROVIDER_WINNT40 LOGON32_PROVIDER_WINNT40 // Use the Windows NT 4.0 logon provider 
#define LOGON32_PROVIDER_WINNT35 LOGON32_PROVIDER_WINNT35 // Use the Windows NT 3.5 logon provider.  

