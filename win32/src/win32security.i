/* File : win32security.i */
// @doc

%module win32security // An interface to the win32 security API's

%{
#define _WIN32_WINNT 0x0500 // We are 2k specific
%}


%include "typemaps.i"
%include "pywin32.i"

%{
#include "windows.h"
#include "PySecurityObjects.h"
#include "accctrl.h"
#include "aclapi.h"
#include "Ntsecapi.h"
#include "subauth.h"
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
	PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
	return NULL;
}



PyObject *PyWinObject_FromTOKEN_GROUPS(TOKEN_GROUPS *tg)
{
	unsigned int groupInd;
	PyObject *groups = PyTuple_New(tg->GroupCount);
	PyObject *group = NULL;
	PyObject *groupSID = NULL;
	for (groupInd = 0; groupInd < tg->GroupCount; groupInd++){
		groupSID = PyWinObject_FromSID(tg->Groups[groupInd].Sid);
		group = Py_BuildValue("(Ol)", groupSID, tg->Groups[groupInd].Attributes );
		PyTuple_SET_ITEM(groups, groupInd, group);
		Py_DECREF(groupSID);
		}
	return groups;
}

BOOL PyWinObject_AsLSA_UNICODE_STRING(PyObject *obstr, LSA_UNICODE_STRING *plsaus, BOOL bNoneOk)
{
	TCHAR *ret_string = NULL;
	USHORT strlen = 0;
	if (!PyWinObject_AsTCHAR(obstr, &ret_string, bNoneOk))
		return FALSE;
	strlen = wcslen(ret_string);
	plsaus->Buffer = ret_string;
	plsaus->Length = strlen * sizeof(WCHAR);
	plsaus->MaximumLength= (strlen+1) * sizeof(WCHAR);
	return TRUE;
}

PyObject* PyWinObject_FromLSA_UNICODE_STRING(LSA_UNICODE_STRING lsaus)
{
	return PyWinObject_FromWCHAR(lsaus.Buffer, lsaus.Length/sizeof(WCHAR));
}

BOOL PyWinObject_AsTOKEN_GROUPS(PyObject *groups, TOKEN_GROUPS **ptg)
{
	BOOL ok = FALSE;
	char *errMsg = "TOKEN_GROUPS must be a sequence of (PySID,int)";
	if (!PySequence_Check(groups)) {
		PyErr_SetString(PyExc_TypeError, errMsg);
		return NULL;
	}
	int groupind = 0;
	int groupcnt = PySequence_Length(groups);
	TOKEN_GROUPS *tg = (TOKEN_GROUPS *)malloc(sizeof(DWORD) + (sizeof(SID_AND_ATTRIBUTES) * groupcnt));

	tg->GroupCount = groupcnt;
	PyObject *group = NULL;
	PyObject *sid = NULL;
	PSID psid;
	for (groupind=0; groupind<groupcnt; groupind++){
		group = PySequence_GetItem(groups, groupind);
		if (!PySequence_Check(group)){
			PyErr_SetString(PyExc_TypeError, errMsg);
			goto done;
			}
		if (!PyArg_ParseTuple(group, "Ol", &sid, &tg->Groups[groupind].Attributes)){
			PyErr_SetString(PyExc_TypeError, errMsg);
			goto done;
			}
		if (!PySID_Check(sid)){
			PyErr_SetString(PyExc_TypeError, errMsg);
			goto done;
			}
		psid = ((PySID *)sid)->GetSID();
		if (!IsValidSid(psid)){
			PyErr_SetString(PyExc_TypeError,"Invalid Sid");
			goto done;
			}
		tg->Groups[groupind].Sid = psid;
		}
	ok = TRUE;
	done:
		if (ok)
			*ptg = tg;
		else
			free(tg);
		return ok;
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
	// space for the array and the priv. count.
	TOKEN_PRIVILEGES *pRet = (TOKEN_PRIVILEGES *)malloc((sizeof(LUID_AND_ATTRIBUTES) * num) + sizeof(DWORD));
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

	pSid = (PSID)malloc(sidSize);

	if (!LookupAccountName(szSystemName, szAcctName, pSid, &sidSize, refDomain, &refDomainSize, &sidType)) {
		PyWin_SetAPIError("LookupAccountName");
		goto done;
	}
	obDomain = PyWinObject_FromTCHAR(refDomain);
	obNewSid = new PySID(pSid);
	result = Py_BuildValue("OOl", obNewSid, obDomain, sidType);

done:
	if (pSid)
		free (pSid);
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

%{/* from MS knowledge base article Q198907
    GetBinarySid() accepts a buffer that contains the textual
    representation of a SID. This function returns NULL
    if it fails. If the SID can be constructed successfully,
    a valid binary SID is returned. 

    This function requires TCHAR.H and the C runtime library.

    The following are macros defined in TCHAR.H that allow this
    function to be compiled with or without UNICODE defined. To
    replace these macros with direct calls to their corresponding
    ANSI functions first make sure this module is not compiled
    with UNICODE (or _UNICODE) defined.

      TCHAR           ANSI
     _stscanf() ->   sscanf()
     _tcschr()  ->   strchr()

*/ 

PSID GetBinarySid(
    LPTSTR TextualSid  // Buffer for Textual representation of SID.
    )
{
    PSID  pSid = 0;
    SID_IDENTIFIER_AUTHORITY identAuthority;
    TCHAR buffer[1024];
    int   i;

    LPTSTR ptr, ptr1;


    BYTE  nByteAuthorityCount = 0;
    DWORD dwSubAuthority[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    ZeroMemory(&identAuthority, sizeof(identAuthority));

    lstrcpy(buffer, TextualSid);

    // S-SID_REVISION- + identifierauthority- + subauthorities- + NULL

    // Skip S
    if (!(ptr = _tcschr(buffer, _T('-'))))
    {
        return pSid;
    }

    // Skip -
    ptr++;

    // Skip SID_REVISION
    if (!(ptr = _tcschr(ptr, _T('-'))))
    {
        return pSid;
    }

    // Skip -
    ptr++;

    // Skip identifierauthority
    if (!(ptr1 = _tcschr(ptr, _T('-'))))
    {
        return pSid;
    }
    *ptr1= 0;

    if ((*ptr == '0') && (*(ptr+1) == 'x'))
    {
        _stscanf(ptr, _T("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
            &identAuthority.Value[0],
            &identAuthority.Value[1],
            &identAuthority.Value[2],
            &identAuthority.Value[3],
            &identAuthority.Value[4],
            &identAuthority.Value[5]);
    }
    else
    {
        DWORD value;

        _stscanf(ptr, _T("%lu"), &value);

        identAuthority.Value[5] = (BYTE)(value & 0x000000FF);
        identAuthority.Value[4] = (BYTE)(value & 0x0000FF00) >> 8;
        identAuthority.Value[3] = (BYTE)(value & 0x00FF0000) >> 16;
        identAuthority.Value[2] = (BYTE)(value & 0xFF000000) >> 24;
    }

    // Skip -
    *ptr1 = '-';
    ptr = ptr1;
    ptr1++;

    for (i = 0; i < 8; i++)
    {
        // get subauthority
        if (!(ptr = _tcschr(ptr, '-')))
        {
            break;
        }
        *ptr=0;
        ptr++;
        nByteAuthorityCount++;
    }

    for (i = 0; i < nByteAuthorityCount; i++)
    {
        // Get subauthority.
        _stscanf(ptr1, _T("%lu"), &dwSubAuthority[i]);
        ptr1 += lstrlen(ptr1) + 1;
    }

    if (!AllocateAndInitializeSid(&identAuthority,
        nByteAuthorityCount,
        dwSubAuthority[0],
        dwSubAuthority[1],
        dwSubAuthority[2],
        dwSubAuthority[3],
        dwSubAuthority[4],
        dwSubAuthority[5],
        dwSubAuthority[6],
        dwSubAuthority[7],
        &pSid))
    {
        pSid = 0;
    }

    return pSid;
} 
%}

%{
// @pyswig <o PySID>|GetBinarySid|Accepts a SID string (eg: S-1-5-32-544) and returns the SID as a PySID object.
static PyObject *PyGetBinarySid (PyObject *self, PyObject *args)
{
	PyObject *obTextualSid; // @pyparm string|SID||Textual representation of a SID. Textual SID example: S-1-5-32-544
	TCHAR *TextualSid= NULL;
	PSID pSid;
	PyObject *obSid;
	
	if (!PyArg_ParseTuple(args, "O:GetBinarySid",
		&obTextualSid))	
		return NULL;
	if (!PyWinObject_AsTCHAR(obTextualSid, &TextualSid))
	{
		PyErr_SetString(PyExc_ValueError, "Textual SID invalid");
		return NULL;
	}
	if (NULL == (pSid= GetBinarySid(TextualSid)))
	{
		PyErr_SetString(PyExc_ValueError, "SID conversion failed");
		return NULL;
	}
	PyWinObject_FreeTCHAR(TextualSid);

	obSid= new PySID(pSid);
	return obSid;
}
%}
%native(GetBinarySid) PyGetBinarySid;

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
 

// @pyswig <o PyUnicode>|LookupPrivilegeName|return the text name for a privilege LUID
%native(LookupPrivilegeName) LookupPrivilegeName;
%{
PyObject *LookupPrivilegeName(PyObject *self, PyObject *args)
{
	PyObject *obsystem_name = NULL;
	PyObject *obluid = NULL;
	PyObject *ret = NULL;
    LUID priv_value;           // @pyparm int|luid||64 bit value representing a privilege

	DWORD origbufsize = 6;
	DWORD bufsize = 0;
	if (!PyArg_ParseTuple(args, "OO:LookupPrivilegeName", 
		&obsystem_name, // @pyparm string/<o PyUnicode>|obsystem_name||System name, local system assumed if not specified
		&obluid))  // @pyparm LARGE_INTEGER|LUID||64 bit value representing a privilege
		return NULL;
	TCHAR *system_name = NULL;
	TCHAR *priv_name = NULL;

	if (!PyWinObject_AsTCHAR(obsystem_name, &system_name, TRUE))
		goto done;
	if (!PyWinObject_AsLARGE_INTEGER(obluid, (LARGE_INTEGER *)&priv_value))
		goto done;

	// if first call fails due to too small buffer, get required size
	priv_name = (TCHAR *)malloc(origbufsize*sizeof(TCHAR));
	if (priv_name == NULL){
		PyErr_SetString(PyExc_MemoryError, "Unable to allocate memory for privilege name");
		return NULL;
		}

	bufsize = origbufsize;
    if (!::LookupPrivilegeName(system_name, &priv_value, priv_name, &bufsize)){
		if (bufsize <= origbufsize){
			PyWin_SetAPIError("LookupPrivilegeName");
			goto done;
			}
		else{
			free (priv_name);
			bufsize += 1;
			priv_name = (TCHAR *)malloc(bufsize*sizeof(TCHAR));
			if (priv_name == NULL){
				PyErr_SetString(PyExc_MemoryError, "Unable to allocate memory for privilege name");
				return NULL;
				}
			if (!::LookupPrivilegeName(system_name, &priv_value, priv_name, &bufsize)){
				PyWin_SetAPIError("LookupPrivilegeName");
				goto done;
				}
			}
		}

	ret = PyWinObject_FromTCHAR(priv_name);
	done:
		if (obsystem_name != NULL)
			PyWinObject_FreeTCHAR(system_name);
		if (priv_name != NULL)
			free(priv_name);
		return ret;
}
%}


// @pyswig <o PyUnicode>|LookupPrivilegeDisplayName|returns long description for a privilege LUID
%native(LookupPrivilegeDisplayName) LookupPrivilegeDisplayName;
%{
PyObject *LookupPrivilegeDisplayName(PyObject *self, PyObject *args)
{
	PyObject *obsystem_name = NULL;
	PyObject *obpriv_name = NULL;
	PyObject *ret = NULL;

	DWORD origbufsize = 6, bufsize = 0;
	DWORD language_id = 0;
	if (!PyArg_ParseTuple(args, "OO:LookupPrivilegeDisplayName", 
		&obsystem_name, // @pyparm string/<o PyUnicode>|obsystem_name||System name, local system assumed if not specified
		&obpriv_name))  // @pyparm string/<o PyUnicode>|obpriv_name||Name of privilege, Se...Privilege string constants
		return NULL;

	TCHAR *system_name = NULL;
	TCHAR *priv_name = NULL;
	TCHAR *priv_desc = NULL;
	if (!PyWinObject_AsTCHAR(obsystem_name, &system_name, TRUE))
		goto done;
	if (!PyWinObject_AsTCHAR(obpriv_name, &priv_name, FALSE))
		goto done;

	// if first call fails due to too small buffer, get required size
	priv_desc = (TCHAR *)malloc(origbufsize*sizeof(TCHAR));
	if (priv_desc == NULL){
		PyErr_SetString(PyExc_MemoryError, "Unable to allocate memory for privilege description");
		return NULL;
		}
	bufsize = origbufsize;
    if (!::LookupPrivilegeDisplayName(system_name, priv_name, priv_desc, &bufsize, &language_id)){
		if (bufsize <= origbufsize){
			PyWin_SetAPIError("LookupPrivilegeDisplayName");
			goto done;
			}
		else{
			free (priv_desc);
			bufsize += 1;
			priv_desc = (TCHAR *)malloc(bufsize*sizeof(TCHAR));
			if (priv_desc == NULL){
				PyErr_SetString(PyExc_MemoryError, "Unable to allocate memory for privilege description");
				return NULL;
				}
			if (!::LookupPrivilegeDisplayName(system_name, priv_name, priv_desc, &bufsize, &language_id)){
				PyWin_SetAPIError("LookupPrivilegeDisplayName");
				goto done;
				}
			}
		}

	ret = PyWinObject_FromTCHAR(priv_desc);
	done:
		if (system_name != NULL)
			PyWinObject_FreeTCHAR(system_name);
		if (priv_name != NULL)
			PyWinObject_FreeTCHAR(priv_name);
		if (priv_desc != NULL)
			free(priv_desc);
		return ret;
}
%}


%{
BOOL MyAdjustTokenPrivileges(
	HANDLE TokenHandle,
	BOOL DisableAllPrivileges,
	TOKEN_PRIVILEGES *NewState)
{
	AdjustTokenPrivileges(TokenHandle, DisableAllPrivileges, NewState, 0, NULL, 0);
	// Note that AdjustTokenPrivileges may succeed, and yet
	// some privileges weren't actually adjusted.
	// You've got to check GetLastError() to be sure!
	DWORD rc = GetLastError();
	return rc==0 || rc==ERROR_NOT_ALL_ASSIGNED;
}
%}

// @pyswig |AdjustTokenPrivileges|
%name(AdjustTokenPrivileges) BOOLAPI MyAdjustTokenPrivileges(
	HANDLE TokenHandle, // @pyparm int|handle||handle to token that contains privileges
	BOOL DisableAllPrivileges, // @pyparm int|bDisableAllPrivileges||Flag for disabling all privileges
	TOKEN_PRIVILEGES *NewState // @pyparm <o PyTOKEN_PRIVILEGES>|NewState||The new state
);



// @pyswig <o PyTOKEN_GROUPS>|AdjustTokenGroups|Sets the groups associated to an access token,returns previous group info
%native(AdjustTokenGroups) PyAdjustTokenGroups;
%{
static PyObject *PyAdjustTokenGroups(PyObject *self, PyObject *args)
{
	PyObject *obHandle=NULL;
	PyObject *obtg = NULL;
	PyObject *ret = NULL;
	HANDLE th;
	TOKEN_GROUPS *newstate;
	BOOL reset = 0;

	if (!PyArg_ParseTuple(args, "OiO", 
		&obHandle, // @pyparm <o PyHANDLE>|obHandle||The handle to access token to be modified
		&reset,    // @pyparm int|ResetToDefault||Sets groups to default enabled/disabled states,
		&obtg))     // @pyparm PyTOKEN_GROUPS|NewState||Groups and attributes to be set for token
	if (!PyWinObject_AsHANDLE(obHandle, &th, FALSE))
		return NULL;
    if (!PyWinObject_AsTOKEN_GROUPS(obtg, &newstate))
		return NULL;

	DWORD bufsize = 0, origbufsize = 10;  //pick a number out of a hat
    TOKEN_GROUPS *oldstate = (TOKEN_GROUPS *)malloc(origbufsize);
	if (oldstate==NULL) {
		PyErr_SetString(PyExc_MemoryError, "AdjustTokenGroups: unable to allocate memory");
		return NULL;
		}

	if (!AdjustTokenGroups(th, reset, newstate, origbufsize, oldstate, &bufsize)){
		if (bufsize <= origbufsize){
			PyWin_SetAPIError("AdjustTokenGroups");
			goto done;
			}
		free (oldstate);
		oldstate = (TOKEN_GROUPS *)malloc(bufsize);
		if (oldstate==NULL) {
			PyErr_SetString(PyExc_MemoryError, "AdjustTokenGroups: unable to allocate memory");
			return NULL;
			}
		if (!AdjustTokenGroups(th, reset, newstate, bufsize, oldstate, &bufsize)){
			PyWin_SetAPIError("AdjustTokenGroups");
			goto done;
			}
		}
	ret = PyWinObject_FromTOKEN_GROUPS(oldstate);
	done:
		if (oldstate != NULL)
			free(oldstate);
		return ret;
}
%}



// @pyswig object|GetTokenInformation|Retrieves a specified type of information about an access token. The calling process must have appropriate access rights to obtain the information.
%native(GetTokenInformation) PyGetTokenInformation;
%{
static PyObject *PyGetTokenInformation(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	int bufSize = 0;
	DWORD retLength = 0;
	DWORD dwordbuf;
	void *buf = NULL;
	TOKEN_INFORMATION_CLASS typ;
	if (!PyArg_ParseTuple(args, "Ol", 
		&obHandle, // @pyparm <o PyHANDLE>|handle||The handle to query the information for.
		(long *)&typ)) // @pyparm int|TokenInformationClass||Specifies a value from the TOKEN_INFORMATION_CLASS enumerated type identifying the type of information the function retrieves.
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;

	PyObject *ret = NULL;
    // null buffer call doesn't seem to work for these two (both return DWORDS, not structs), special case them
	if ((typ==TokenSessionId) || (typ == TokenSandBoxInert)){
		bufSize = sizeof(DWORD);
		if (!GetTokenInformation(handle, typ, &dwordbuf, bufSize, &retLength)) {
			PyWin_SetAPIError("GetTokenInformation");
			goto done;
			}
		}
	else{
	    // first call with NULL in the TokenInformation buffer pointer should return the required size
		GetTokenInformation(handle, typ, buf, bufSize, &retLength);
		if (retLength == 0){
			PyWin_SetAPIError("GetTokenInformation - size call");
			goto done;
			}

		bufSize = retLength;
		buf = malloc(retLength);
		if (buf==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating buffer for token info");
			return NULL;
			}

		if (!GetTokenInformation(handle, typ, buf, bufSize, &retLength)) {
			PyWin_SetAPIError("GetTokenInformation");
			goto done;
			}
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
		case TokenOwner: {
			// @flag TokenOwner|<o PySID>
			TOKEN_OWNER *p = (TOKEN_OWNER *)buf;
			ret = PyWinObject_FromSID(p->Owner);
			break;
			}
		case TokenGroups: {
			// @flag TokenGroups|((<o PySID>,int),)
			// returns a list of tuples containing (group Sid, attribute flags)
			TOKEN_GROUPS *tg = (TOKEN_GROUPS *)buf;
			ret = PyWinObject_FromTOKEN_GROUPS(tg);
			break;
			}
		case TokenRestrictedSids: {
			// @flag TokenRestrictedSids|((<o PySID>,int),)
			TOKEN_GROUPS *tg = (TOKEN_GROUPS *)buf;
			ret = PyWinObject_FromTOKEN_GROUPS(tg);
			break;
			}
		case TokenPrivileges: {
			// @flag TokenPrivileges|((int,int),)
			// returns PyTOKEN_PRIVILEGES (tuple of LUID and attribute flags for each privilege)
			// attributes are combination of SE_PRIVILEGE_ENABLED,SE_PRIVILEGE_ENABLED_BY_DEFAULT,SE_PRIVILEGE_USED_FOR_ACCESS
			// should make this into body of PyWinObject_FromTOKEN_PRIVILEGES
			unsigned int privInd;
			PyObject *priv = NULL;
			PyObject *obluid = NULL;
			PLUID pluid;
			TOKEN_PRIVILEGES *tp = (TOKEN_PRIVILEGES *)buf;
			PyObject *privs = PyTuple_New(tp->PrivilegeCount);
			for (privInd = 0; privInd < tp->PrivilegeCount; privInd++){
				pluid = &tp->Privileges[privInd].Luid;
				obluid = PyWinObject_FromLARGE_INTEGER(*((LARGE_INTEGER *) pluid));
				priv = Py_BuildValue("(Ol)",obluid,tp->Privileges[privInd].Attributes );
				PyTuple_SET_ITEM(privs, privInd, priv);
				Py_DECREF(obluid);
				}
			ret = privs;
			break;
			}
		case TokenPrimaryGroup: {
			TOKEN_PRIMARY_GROUP *pg = (TOKEN_PRIMARY_GROUP *)buf;
            ret = PyWinObject_FromSID(pg->PrimaryGroup);
			break;
			}
		case TokenSource: {
			// @flag TokenSource|(string,LUID)
			TOKEN_SOURCE *ts = (TOKEN_SOURCE *)buf;
			PLUID pluid = &ts->SourceIdentifier;
			PyObject *obluid = PyWinObject_FromLARGE_INTEGER(*((LARGE_INTEGER *) pluid));
			ret = Py_BuildValue("(s#O)",ts->SourceName,8,obluid);
			Py_DECREF(obluid);
			break;
			}
		case TokenDefaultDacl: {
			TOKEN_DEFAULT_DACL *dacl = (TOKEN_DEFAULT_DACL *)buf;
			ret = new PyACL(dacl->DefaultDacl);
			break;
			}
		case TokenType: {
			// returns TokenPrimary or TokenImpersonation
			TOKEN_TYPE *tt = (TOKEN_TYPE *)buf;
			ret=Py_BuildValue("i",*tt);
			break;
			}
		case TokenImpersonationLevel: {
			SECURITY_IMPERSONATION_LEVEL *sil = (SECURITY_IMPERSONATION_LEVEL *)buf;
			ret=Py_BuildValue("i",*sil);
			break;
			}
		case TokenSandBoxInert: {
			// ??? I get "The parameter is incorrect" error for this one, maybe only valid for impersonation token ???
			ret = Py_BuildValue("l",dwordbuf);
			break;
			}
		case TokenSessionId: {
			// always returns zero when handle does not refer to a Terminal Services client session
			//  - not yet tested with such
			ret = Py_BuildValue("l",dwordbuf);
			break;
			}
		default:
			PyErr_SetString(PyExc_TypeError, "The TokenInformationClass param is not supported yet");
			break;
	}
done:
    if (buf != NULL)
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

	PSECURITY_DESCRIPTOR psd = NULL;
	DWORD dwSize = 0;
	TCHAR *fname = NULL;
	if (!PyWinObject_AsTCHAR(obFname, &fname))
		goto done;

	if (GetFileSecurity(fname, info, psd, dwSize, &dwSize)) {
		PyErr_SetString(PyExc_RuntimeError, "Can't query for SECURITY_DESCRIPTOR size info?");
		goto done;
	}
	psd = (PSECURITY_DESCRIPTOR)malloc(dwSize);
	if (psd==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating SECURITY_DESCRIPTOR");
		goto done;
	}
    if (!GetFileSecurity(fname, info, psd, dwSize, &dwSize)) {
		PyWin_SetAPIError("GetFileSecurity");
		goto done;
	}
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd);
done:
	PyWinObject_FreeTCHAR(fname);
	if (psd)
		free(psd);
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

	PSECURITY_DESCRIPTOR psd;
	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obsd, &psd))
		goto done;
	if (!SetFileSecurity(fname, info, psd)) {
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
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd);
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

	PSECURITY_DESCRIPTOR psd;
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
	rc = PyWinObject_FromSECURITY_DESCRIPTOR(psd);
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

	PSECURITY_DESCRIPTOR psd;
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

// @pyswig object|SetTokenInformation|Set a specified type of information in an access token
%native(SetTokenInformation) PySetTokenInformation;
%{
static PyObject *PySetTokenInformation(PyObject *self, PyObject *args)
{
	PyObject *obth;
	HANDLE th;
	PyObject *obinfo;
	int bufsize = 0;
	void *buf = NULL;
	TOKEN_INFORMATION_CLASS typ;

	if (!PyArg_ParseTuple(args, "OiO", 
		&obth,        // @pyparm <o PyHANDLE>|handle||Handle to an access token to be modified
		(long *)&typ, // @pyparm int|TokenInformationClass||Specifies a value from the TOKEN_INFORMATION_CLASS enumerated type identifying the type of information the function retrieves.
		&obinfo))     // @pyparm <o PyACL>/<o PySID>/int|obinfo||PyACL, PySID, or int depending on type parm
		return NULL;

	if (!PyWinObject_AsHANDLE(obth, &th, FALSE ))
		return NULL;

	switch (typ) {
		case TokenOwner: {
			PSID psid;
			TOKEN_OWNER towner; 
			if (!PyWinObject_AsSID(obinfo, &psid, FALSE))
				return NULL;
			if (!IsValidSid(psid)){
				PyErr_SetString(PyExc_ValueError, "Invalid SID");
				return NULL;
				}
			towner.Owner = psid;
			buf = (void *)&towner;
			if (!IsValidSid(towner.Owner)){
				PyErr_SetString(PyExc_ValueError, "Invalid SID in tokenowner");
				return NULL;
				}
			bufsize = sizeof(TOKEN_OWNER);
			break;
			}
		case TokenPrimaryGroup: {
			PSID psid;
			TOKEN_PRIMARY_GROUP tpg;
			if (!PyWinObject_AsSID(obinfo, &psid, FALSE))
				return NULL;
			if (!IsValidSid(psid)){
				PyErr_SetString(PyExc_ValueError, "Invalid SID");
				return NULL;
				}
			ZeroMemory(&tpg,sizeof(TOKEN_PRIMARY_GROUP));
			tpg.PrimaryGroup = psid;
			buf = (void *)&tpg;
			bufsize = sizeof(TOKEN_PRIMARY_GROUP);
			break;
			}
		case TokenDefaultDacl: {
			PACL pacl;
			TOKEN_DEFAULT_DACL tdd;
			if (!PyWinObject_AsACL(obinfo, &pacl, TRUE))
				return NULL;
			tdd.DefaultDacl = pacl;
			buf = (void *)&tdd;
			bufsize = sizeof(TOKEN_DEFAULT_DACL);
			break;
			}
		case TokenSessionId: {
			DWORD sessionid = PyLong_AsUnsignedLong(obinfo);
			buf = (void *)&sessionid;
			bufsize = sizeof(DWORD);
			break;
			}
		default:
			PyErr_SetString(PyExc_TypeError, "Invalid TokenInformationClass parm");
			return NULL;
	}
	if (!SetTokenInformation(th,typ,buf,bufsize)){
		PyWin_SetAPIError("SetTokenInformation");
		return NULL;
		}

	Py_INCREF(Py_None);
	return Py_None;

}
%}

// @pyswig <o PyHandle>|GetPolicyHandle|Opens a policy handle for the specified system
%native(GetPolicyHandle) PyGetPolicyHandle;
%{
static PyObject *PyGetPolicyHandle(PyObject *self, PyObject *args)
{
	PyObject *obsystem_name = NULL;
	PyObject *ret = NULL;
	DWORD access_mask = 0;
	LSA_UNICODE_STRING system_name;
	NTSTATUS ntsResult;
	LSA_HANDLE lsahPolicyHandle;
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;  // reserved, must be zeros or NULL
	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

	if (!PyArg_ParseTuple(args, "Oi:GetPolicyHandle", 
		&obsystem_name, // @pyparm string/<o PyUnicode>|obsystem_name||System name, local system assumed if not specified
		&access_mask))  // @pyparm int|access_mask||Bitmask of requested access types
		return NULL;
	if (!PyWinObject_AsLSA_UNICODE_STRING(obsystem_name, &system_name, TRUE))
		goto done;

	ntsResult = LsaOpenPolicy(&system_name, &ObjectAttributes, access_mask, &lsahPolicyHandle);
	if (ntsResult != STATUS_SUCCESS){
		PyWin_SetAPIError("GetPolicyHandle",LsaNtStatusToWinError(ntsResult));
		goto done;
		}
	ret = PyWinObject_FromHANDLE(lsahPolicyHandle);
	done:
		PyWinObject_FreeTCHAR(system_name.Buffer);
		return ret;
}
%}

// @pyswig |LsaClose|Closes a policy handle created by GetPolicyHandle
%native(LsaClose) PyLsaClose;
%{
static PyObject *PyLsaClose(PyObject *self, PyObject *args)
{
	PyObject *obhandle;
	LSA_HANDLE lsah;
	NTSTATUS err;
	if (!PyArg_ParseTuple(args, "O:LsaClose", &obhandle))
		return NULL; 
	if (!PyWinObject_AsHANDLE(obhandle, &lsah))
		return NULL;
	err=LsaClose(lsah);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaClose",LsaNtStatusToWinError(err));
		return NULL;
		}
	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig |LsaQueryInformationPolicy|Retrieves information from the policy handle
%native(LsaQueryInformationPolicy) PyLsaQueryInformationPolicy;
%{
static PyObject *PyLsaQueryInformationPolicy(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PyObject *obhandle;
	LSA_HANDLE lsah;
	NTSTATUS err;
	void* buf = NULL;
	POLICY_INFORMATION_CLASS info_class;
	if (!PyArg_ParseTuple(args, "Oi:LsaQueryInformationPolicy", &obhandle, (long *)&info_class))
		return NULL; 
	if (!PyWinObject_AsHANDLE(obhandle, &lsah))
		return NULL;;
	
	err = LsaQueryInformationPolicy(lsah, info_class, &buf);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaQueryInformationPolicy",LsaNtStatusToWinError(err));
		return NULL;
		}

	switch (info_class){
		case PolicyAuditEventsInformation:{
			// return tuple of
			//   int (auditing enabled),
			//   tuple of event auditing options, indexed by POLICY_AUDIT_EVENT_TYPE values
			POLICY_AUDIT_EVENTS_INFO *info = (POLICY_AUDIT_EVENTS_INFO *)buf;
			PyObject *events = PyTuple_New(info->MaximumAuditEventCount);
			DWORD *auditing_option = info->EventAuditingOptions;
			for (unsigned long event_ind=0;event_ind<info->MaximumAuditEventCount;event_ind++){
				PyTuple_SetItem(events, event_ind, Py_BuildValue("i", *auditing_option));
				auditing_option++;
				}
			ret=Py_BuildValue("iO",info->AuditingMode,events);
			Py_DECREF(events);
			break;
			}

		case PolicyDnsDomainInformation:{
			POLICY_DNS_DOMAIN_INFO *info = (POLICY_DNS_DOMAIN_INFO *)buf;
			PyObject *domain_name =     PyWinObject_FromLSA_UNICODE_STRING(info->Name);
			PyObject *dns_domain_name = PyWinObject_FromLSA_UNICODE_STRING(info->DnsDomainName);
			PyObject *dns_forest_name = PyWinObject_FromLSA_UNICODE_STRING(info->DnsForestName);
			PyObject *domain_guid = PyWinUnicodeObject_FromIID(info->DomainGuid);
			PyObject *domain_sid = PyWinObject_FromSID(info->Sid);
			ret = Py_BuildValue("(OOOOO)",domain_name,dns_domain_name,dns_forest_name,domain_guid,domain_sid);
			Py_DECREF(domain_name);
			Py_DECREF(dns_domain_name);
			Py_DECREF(dns_forest_name);
			Py_DECREF(domain_guid);
			Py_DECREF(domain_sid);
			break;
			}

		case PolicyPrimaryDomainInformation:{
			POLICY_PRIMARY_DOMAIN_INFO *info = (POLICY_PRIMARY_DOMAIN_INFO *)buf;
			PyObject *domain_name = PyWinObject_FromLSA_UNICODE_STRING(info->Name);
			PyObject *domain_sid = PyWinObject_FromSID(info->Sid);
			ret = Py_BuildValue("(OO)",domain_name,domain_sid);
			Py_DECREF(domain_name);
			Py_DECREF(domain_sid);
			break;
			}

		case PolicyAccountDomainInformation:{
			POLICY_ACCOUNT_DOMAIN_INFO *info = (POLICY_ACCOUNT_DOMAIN_INFO *)buf;
			PyObject *domain_name = PyWinObject_FromLSA_UNICODE_STRING(info->DomainName);
			PyObject *domain_sid = PyWinObject_FromSID(info->DomainSid);
			ret = Py_BuildValue("(OO)",domain_name,domain_sid);
			Py_DECREF(domain_name);
			Py_DECREF(domain_sid);
			break;
			}

		case PolicyLsaServerRoleInformation:{
			POLICY_LSA_SERVER_ROLE_INFO *info = (POLICY_LSA_SERVER_ROLE_INFO *)buf;
			ret=Py_BuildValue("i",info->LsaServerRole);
			break;
			}

		case PolicyModificationInformation:{
			/* ???????? This alway blows up in the LsaQueryInformationPolicy call
			   (87, 'LsaQueryInformationPolicy', 'The parameter is incorrect.')
			   Tried it with local handle, PDC and BDC with no luck
			   Running test case with everything hardcoded produced same result
			   Maybe only works locally on PDC ?
			   Data conversions below are untested
			*/
			POLICY_MODIFICATION_INFO *info = (POLICY_MODIFICATION_INFO *)buf;
			PyObject *modserial = PyWinObject_FromLARGE_INTEGER(info->ModifiedId);
			FILETIME modtimeft;
			memcpy(&modtimeft, &(info->DatabaseCreationTime), sizeof(FILETIME));
			PyObject *modtime = PyWinObject_FromFILETIME(modtimeft);
			ret = Py_BuildValue("(OO)",modserial,modtime);
			Py_DECREF(modserial);
			Py_DECREF(modtime);
			break;
			}
		default:
			PyErr_SetString(PyExc_NotImplementedError, "The POLICY_INFORMATION_CLASS specified is not supported yet");
			break;
		}

	LsaFreeMemory(buf);
	return ret;
}
%}

// @pyswig |LsaSetInformationPolicy|Sets policy options
%native(LsaSetInformationPolicy) PyLsaSetInformationPolicy;
%{
static PyObject *PyLsaSetInformationPolicy(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PyObject *obhandle=NULL;
	PyObject *obinfo=NULL;
	LSA_HANDLE lsah;
	NTSTATUS err;
	void* buf = NULL;
	POLICY_INFORMATION_CLASS info_class;
	if (!PyArg_ParseTuple(args, "OiO:PyLsaSetInformationPolicy", &obhandle, (long *)&info_class, &obinfo))
		return NULL; 
	if (!PyWinObject_AsHANDLE(obhandle, &lsah))
		return NULL;;
	switch (info_class){
		case PolicyAuditEventsInformation:{
			// input is tuple of (bool, tuple of eventauditingoptions)
			BOOL auditing_mode;
			unsigned long *auditing_options = NULL, *auditing_option = NULL;
			PyObject *obauditing_options = NULL, *obauditing_option = NULL;
			int option_ind, option_cnt;
			POLICY_AUDIT_EVENTS_INFO info;

			if (!PyArg_ParseTuple(obinfo, "iO:PyLsaSetInformationPolicy", &auditing_mode, &obauditing_options)){
				PyErr_SetString(PyExc_TypeError, "Info for PolicyAuditEventsInformation must be (int, sequence of ints)");
				return NULL; 
				}
			if (!PySequence_Check(obauditing_options)){
				PyErr_SetString(PyExc_TypeError, "Info for PolicyAuditEventsInformation must be (int, sequence of ints)");
				return NULL; 
				}

			option_cnt = PySequence_Length(obauditing_options);
			auditing_options = (unsigned long *)calloc(option_cnt, sizeof(unsigned long));
			auditing_option = auditing_options;

			info.AuditingMode = auditing_mode;
			info.EventAuditingOptions = auditing_options;
			info.MaximumAuditEventCount = option_cnt;

			for (option_ind=0; option_ind<option_cnt; option_ind++){
				obauditing_option = PySequence_GetItem(obauditing_options, option_ind);
				if(!PyInt_Check(obauditing_option)){
					Py_DECREF(obauditing_option);
					PyErr_SetString(PyExc_TypeError, "Info for PolicyAuditEventsInformation must be (int, sequence of ints)");
					goto done;
					}
				*auditing_option=PyInt_AsLong(obauditing_option);
				Py_DECREF(obauditing_option);
				auditing_option++;
				}
			err = LsaSetInformationPolicy(lsah, info_class, &info);
			if (err != STATUS_SUCCESS){
				PyWin_SetAPIError("LsaSetInformationPolicy",LsaNtStatusToWinError(err));
				goto done;
				}
			ret = Py_None;
			done:
				if (auditing_options)
					free (auditing_options);
				Py_XINCREF(ret);
				return ret;
			break;
			}

		default:{
			PyErr_SetString(PyExc_NotImplementedError, "The specified POLICY_INFORMATION_CLASS is not supported yet");
			return NULL;
			}
		}
}
%}

// @pyswig |LsaAddAccountRights|Adds a list of privliges to an account - account is created if it doesn't already exist
%native(LsaAddAccountRights) PyLsaAddAccountRights;
%{
static PyObject *PyLsaAddAccountRights(PyObject *self, PyObject *args)
{
	PyObject *privs=NULL, *priv=NULL, *policy_handle=NULL;
	PyObject *obsid=NULL, *ret=NULL;
	PSID psid=NULL;
	PLSA_UNICODE_STRING plsau=NULL, plsau_start=NULL;
	DWORD priv_cnt=0,priv_ind=0;
	HANDLE hpolicy;
	NTSTATUS err;
	if (!PyArg_ParseTuple(args, "OOO:LsaAddAccountRights", &policy_handle, &obsid, &privs))
		return NULL;
	if (!PyWinObject_AsHANDLE(policy_handle, &hpolicy))
		return NULL;
	if (!PyWinObject_AsSID(obsid, &psid, FALSE))
		return NULL;
	if (!PySequence_Check(privs))
		return NULL;
	priv_cnt=PySequence_Length(privs);
	plsau_start=(PLSA_UNICODE_STRING)calloc(priv_cnt,sizeof(LSA_UNICODE_STRING));
	plsau=plsau_start;
	for (priv_ind=0; priv_ind<priv_cnt; priv_ind++){
		plsau->Buffer=NULL;
		priv=PySequence_GetItem(privs, priv_ind);
		if (!PyWinObject_AsLSA_UNICODE_STRING(priv,plsau,FALSE)){
			Py_DECREF(priv);
			goto done;
			}
		Py_DECREF(priv);
		plsau++;
		}
	err=LsaAddAccountRights(hpolicy, psid, plsau_start, priv_cnt);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaAddAccountRights",LsaNtStatusToWinError(err));
		goto done;
		}
	ret=Py_None;

	done:
	if (plsau_start){
		plsau=plsau_start;
		for (priv_ind=0; priv_ind<priv_cnt; priv_ind++){
			// in case object in privs is not a string
			if(plsau->Buffer==NULL)
				break;
			PyWinObject_FreeTCHAR(plsau->Buffer);
			plsau++;
			}
		free(plsau_start);
		}
	Py_XINCREF(ret);
	return ret;
}
%}

// @pyswig |LsaRemoveAccountRights|Removes privs from an account - if AllRights parm is true, account is *deleted*
%native(LsaRemoveAccountRights) PyLsaRemoveAccountRights;
%{
static PyObject *PyLsaRemoveAccountRights(PyObject *self, PyObject *args)
{
	PyObject *privs=NULL, *priv=NULL, *policy_handle=NULL;
	PyObject *obsid=NULL, *ret=NULL;
	PSID psid=NULL;
	BOOL AllRights=FALSE;
	PLSA_UNICODE_STRING plsau=NULL, plsau_start=NULL;
	DWORD priv_cnt=0,priv_ind=0;
	HANDLE hpolicy;
	NTSTATUS err;
	if (!PyArg_ParseTuple(args, "OOiO:LsaAddAccountRights", &policy_handle, &obsid,  &AllRights, &privs))
		return NULL;
	if (!PyWinObject_AsHANDLE(policy_handle, &hpolicy))
		return NULL;
	if (!PyWinObject_AsSID(obsid, &psid, FALSE))
		return NULL;
	if (!PySequence_Check(privs))
		return NULL;
	priv_cnt=PySequence_Length(privs);
	plsau_start=(PLSA_UNICODE_STRING)calloc(priv_cnt,sizeof(LSA_UNICODE_STRING));
	plsau=plsau_start;
	for (priv_ind=0; priv_ind<priv_cnt; priv_ind++){
		plsau->Buffer=NULL;
		priv=PySequence_GetItem(privs, priv_ind);
		if (!PyWinObject_AsLSA_UNICODE_STRING(priv,plsau,FALSE)){
			Py_DECREF(priv);
			goto done;
			}
		Py_DECREF(priv);
		plsau++;
		}
	err=LsaRemoveAccountRights(hpolicy, psid, AllRights, plsau_start, priv_cnt);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaRemoveAccountRights",LsaNtStatusToWinError(err));
		goto done;
		}
	ret=Py_None;

	done:
	if (plsau_start){
		plsau=plsau_start;
		for (priv_ind=0; priv_ind<priv_cnt; priv_ind++){
			// in case object in privs is not a string
			if(plsau->Buffer==NULL)
				break;
			PyWinObject_FreeTCHAR(plsau->Buffer);
			plsau++;
			}
		free(plsau_start);
		}
	Py_XINCREF(ret);
	return ret;
}
%}

// @pyswig |LsaEnumerateAccountRights|Lists privileges held by SID
%native(LsaEnumerateAccountRights) PyLsaEnumerateAccountRights;
%{
static PyObject *PyLsaEnumerateAccountRights(PyObject *self, PyObject *args)
{
	PyObject *privs=NULL, *priv=NULL, *policy_handle=NULL;
	PyObject *obsid=NULL, *ret=NULL;
	PSID psid=NULL;
	PLSA_UNICODE_STRING plsau=NULL, plsau_start=NULL;
	ULONG priv_cnt=0,priv_ind=0;
	HANDLE hpolicy;
	NTSTATUS err;
	if (!PyArg_ParseTuple(args, "OO:LsaAddAccountRights", &policy_handle, &obsid))
		return NULL;
	if (!PyWinObject_AsHANDLE(policy_handle, &hpolicy))
		return NULL;
	if (!PyWinObject_AsSID(obsid, &psid, FALSE))
		return NULL;
	err=LsaEnumerateAccountRights(hpolicy,psid, &plsau_start, &priv_cnt);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaEnumerateAccountRights",LsaNtStatusToWinError(err));
		goto done;
		}
	privs=PyTuple_New(priv_cnt);
	plsau=plsau_start;
	for (priv_ind=0; priv_ind<priv_cnt; priv_ind++){
        priv=PyWinObject_FromLSA_UNICODE_STRING(*plsau);
		PyTuple_SetItem(privs, priv_ind, priv);
		plsau++;
		}
	done:
	if (plsau_start)
		LsaFreeMemory(plsau_start);
	return privs;
}
%}

// @pyswig |LsaEnumerateAccountsWithUserRight|Return SIDs that hold specified priv
%native(LsaEnumerateAccountsWithUserRight) PyLsaEnumerateAccountsWithUserRight;
%{
static PyObject *PyLsaEnumerateAccountsWithUserRight(PyObject *self, PyObject *args)
{
	PyObject *obpriv=NULL, *policy_handle=NULL;
	PyObject *sids=NULL, *sid=NULL;
	PSID psid=NULL;
	LSA_UNICODE_STRING lsau;
	ULONG sid_cnt=0, sid_ind=0;
	HANDLE hpolicy;
	LSA_ENUMERATION_INFORMATION *buf;
	void *buf_start;
	NTSTATUS err;
	if (!PyArg_ParseTuple(args, "OO:LsaEnumerateAccountsWithUserRight", &policy_handle, &obpriv))
		return NULL;
	if (!PyWinObject_AsHANDLE(policy_handle, &hpolicy))
		return NULL;
	if (!PyWinObject_AsLSA_UNICODE_STRING(obpriv,&lsau,FALSE))
		return NULL;
	err=LsaEnumerateAccountsWithUserRight(hpolicy,&lsau,&buf_start,&sid_cnt);
	if (err != STATUS_SUCCESS){
		PyWin_SetAPIError("LsaEnumerateAccountsWithUserRight",LsaNtStatusToWinError(err));
		goto done;
		}

	sids=PyTuple_New(sid_cnt);
	buf=(LSA_ENUMERATION_INFORMATION *)buf_start;
	for (sid_ind=0; sid_ind<sid_cnt; sid_ind++){
        sid=PyWinObject_FromSID(buf->Sid);
		PyTuple_SetItem(sids, sid_ind, sid);
		buf++;
		}
	done:
	if (buf_start)
		LsaFreeMemory(buf_start);
	PyWinObject_FreeTCHAR(lsau.Buffer);
	return sids;
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
 
// SE_OBJECT_TYPE - securable objects
#define SE_UNKNOWN_OBJECT_TYPE SE_UNKNOWN_OBJECT_TYPE
#define SE_FILE_OBJECT SE_FILE_OBJECT
#define SE_SERVICE SE_SERVICE
#define SE_PRINTER SE_PRINTER
#define SE_REGISTRY_KEY SE_REGISTRY_KEY
#define SE_LMSHARE SE_LMSHARE
#define SE_KERNEL_OBJECT SE_KERNEL_OBJECT
#define SE_WINDOW_OBJECT SE_WINDOW_OBJECT
#define SE_DS_OBJECT SE_DS_OBJECT
#define SE_DS_OBJECT_ALL SE_DS_OBJECT_ALL
#define SE_PROVIDER_DEFINED_OBJECT SE_PROVIDER_DEFINED_OBJECT
#define SE_WMIGUID_OBJECT SE_WMIGUID_OBJECT
#define SE_REGISTRY_WOW64_32KEY SE_REGISTRY_WOW64_32KEY

// group sid attributes
#define SE_GROUP_ENABLED SE_GROUP_ENABLED
#define SE_GROUP_ENABLED_BY_DEFAULT SE_GROUP_ENABLED_BY_DEFAULT
#define SE_GROUP_LOGON_ID SE_GROUP_LOGON_ID
#define SE_GROUP_MANDATORY SE_GROUP_MANDATORY
#define SE_GROUP_OWNER SE_GROUP_OWNER
#define SE_GROUP_RESOURCE SE_GROUP_RESOURCE
#define SE_GROUP_USE_FOR_DENY_ONLY SE_GROUP_USE_FOR_DENY_ONLY

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
#define SidTypeComputer SidTypeComputer // Indicates a computer SID

// TokenInformationClass constatns
#define TokenDefaultDacl TokenDefaultDacl 
#define TokenGroups TokenGroups
#define TokenGroupsAndPrivileges TokenGroupsAndPrivileges
#define TokenImpersonationLevel TokenImpersonationLevel
#define TokenOwner TokenOwner
#define TokenPrimaryGroup TokenPrimaryGroup 
#define TokenPrivileges TokenPrivileges 
#define TokenRestrictedSids TokenRestrictedSids 
#define TokenSandBoxInert TokenSandBoxInert 
#define TokenSessionId TokenSessionId
#define TokenSource TokenSource
#define TokenStatistics TokenStatistics
#define TokenType TokenType 
#define TokenUser TokenUser 

// TOKEN_TYPE constants
#define TokenPrimary TokenPrimary
#define TokenImpersonation TokenImpersonation

// SECURITY_IMPERSONATION_LEVEL constants
#define SecurityAnonymous SecurityAnonymous
#define SecurityIdentification SecurityIdentification
#define SecurityImpersonation SecurityImpersonation
#define SecurityDelegation SecurityDelegation

#define LOGON32_LOGON_BATCH LOGON32_LOGON_BATCH //This logon type is intended for batch servers, where processes may be executing on behalf of a user without their direct intervention; or for higher performance servers that process many clear-text authentication attempts at a time, such as mail or web servers. LogonUser does not cache credentials for this logon type. 
#define LOGON32_LOGON_INTERACTIVE LOGON32_LOGON_INTERACTIVE //This logon type is intended for users who will be interactively using the machine, such as a user being logged on by a terminal server, remote shell, or similar process. This logon type has the additional expense of caching logon information for disconnected operation, and is therefore inappropriate for some client/server applications, such as a mail server. 
#define LOGON32_LOGON_SERVICE LOGON32_LOGON_SERVICE // Indicates a service-type logon. The account provided must have the service privilege enabled. 
#define LOGON32_LOGON_NETWORK LOGON32_LOGON_NETWORK // This logon type is intended for high performance servers to authenticate clear text passwords. LogonUser does not cache credentials for this logon type. This is the fastest logon path, but there are two limitations. First, the function returns an impersonation token, not a primary token. You cannot use this token directly in the CreateProcessAsUser function. However, you can call the DuplicateTokenEx function to convert the token to a primary token, and then use it in CreateProcessAsUser. Second, if you convert the token to a primary token and use it in CreateProcessAsUser to start a process, the new process will not be able to access other network resources, such as remote servers or printers, through the redirector.

#define LOGON32_PROVIDER_DEFAULT LOGON32_PROVIDER_DEFAULT // Use the standard logon provider for the system. This is the recommended value for the dwLogonProvider parameter. It provides maximum compatibility with current and future releases of Windows NT.  
#define LOGON32_PROVIDER_WINNT40 LOGON32_PROVIDER_WINNT40 // Use the Windows NT 4.0 logon provider 
#define LOGON32_PROVIDER_WINNT35 LOGON32_PROVIDER_WINNT35 // Use the Windows NT 3.5 logon provider.  

// SECURITY_IDENTIFIER_AUTHORITY Values ??? last byte of a 6 - byte array ???
#define SECURITY_NULL_SID_AUTHORITY         0
#define SECURITY_WORLD_SID_AUTHORITY        1
#define SECURITY_LOCAL_SID_AUTHORITY        2
#define SECURITY_CREATOR_SID_AUTHORITY      3
#define SECURITY_NON_UNIQUE_AUTHORITY       4
#define SECURITY_NT_AUTHORITY               5
#define SECURITY_RESOURCE_MANAGER_AUTHORITY 9

// SECURITY_DESCRIPTOR_CONTROL flags
#define SE_DACL_AUTO_INHERITED SE_DACL_AUTO_INHERITED	// win2k and up
#define SE_SACL_AUTO_INHERITED SE_SACL_AUTO_INHERITED	// win2k and up
#define SE_DACL_PROTECTED SE_DACL_PROTECTED				// win2k and up
#define SE_SACL_PROTECTED SE_SACL_PROTECTED				// win2k and up
#define SE_DACL_DEFAULTED SE_DACL_DEFAULTED
#define SE_DACL_PRESENT SE_DACL_PRESENT
#define SE_GROUP_DEFAULTED SE_GROUP_DEFAULTED
#define SE_OWNER_DEFAULTED SE_OWNER_DEFAULTED
#define SE_SACL_PRESENT SE_SACL_PRESENT
#define SE_SELF_RELATIVE SE_SELF_RELATIVE
#define SE_SACL_DEFAULTED SE_SACL_DEFAULTED

// ACL revisions
#define ACL_REVISION ACL_REVISION
#define ACL_REVISION_DS ACL_REVISION_DS

// ACE types
#define ACCESS_ALLOWED_ACE_TYPE ACCESS_ALLOWED_ACE_TYPE					// Access-allowed ACE that uses the ACCESS_ALLOWED_ACE structure. 
#define ACCESS_ALLOWED_OBJECT_ACE_TYPE ACCESS_ALLOWED_OBJECT_ACE_TYPE	// Windows 2000/XP: Object-specific access-allowed ACE that uses the ACCESS_ALLOWED_OBJECT_ACE structure. 
#define ACCESS_DENIED_ACE_TYPE ACCESS_DENIED_ACE_TYPE					// Access-denied ACE that uses the ACCESS_DENIED_ACE structure. 
#define ACCESS_DENIED_OBJECT_ACE_TYPE ACCESS_DENIED_OBJECT_ACE_TYPE		// Windows 2000/XP: Object-specific access-denied ACE that uses the ACCESS_DENIED_OBJECT_ACE structure. 
#define SYSTEM_AUDIT_ACE_TYPE SYSTEM_AUDIT_ACE_TYPE						// System-audit ACE that uses the SYSTEM_AUDIT_ACE structure. 
#define SYSTEM_AUDIT_OBJECT_ACE_TYPE SYSTEM_AUDIT_OBJECT_ACE_TYPE 

// policy privileges to be used with GetPolicyHandle
#define POLICY_VIEW_LOCAL_INFORMATION POLICY_VIEW_LOCAL_INFORMATION
#define POLICY_VIEW_AUDIT_INFORMATION POLICY_VIEW_AUDIT_INFORMATION
#define POLICY_GET_PRIVATE_INFORMATION POLICY_GET_PRIVATE_INFORMATION
#define POLICY_TRUST_ADMIN POLICY_TRUST_ADMIN
#define POLICY_CREATE_ACCOUNT POLICY_CREATE_ACCOUNT
#define POLICY_CREATE_SECRET POLICY_CREATE_SECRET
#define POLICY_CREATE_PRIVILEGE POLICY_CREATE_PRIVILEGE
#define POLICY_SET_DEFAULT_QUOTA_LIMITS POLICY_SET_DEFAULT_QUOTA_LIMITS
#define POLICY_SET_AUDIT_REQUIREMENTS POLICY_SET_AUDIT_REQUIREMENTS
#define POLICY_AUDIT_LOG_ADMIN POLICY_AUDIT_LOG_ADMIN
#define POLICY_SERVER_ADMIN POLICY_SERVER_ADMIN
#define POLICY_LOOKUP_NAMES POLICY_LOOKUP_NAMES
#define POLICY_NOTIFICATION POLICY_NOTIFICATION
#define POLICY_ALL_ACCESS POLICY_ALL_ACCESS
#define POLICY_READ POLICY_READ
#define POLICY_WRITE POLICY_WRITE
#define POLICY_EXECUTE POLICY_EXECUTE

//POLICY_INFORMATION_CLASS values
#define PolicyAuditLogInformation PolicyAuditLogInformation
#define PolicyAuditEventsInformation PolicyAuditEventsInformation
#define PolicyPrimaryDomainInformation PolicyPrimaryDomainInformation
#define PolicyPdAccountInformation PolicyPdAccountInformation
#define PolicyAccountDomainInformation PolicyAccountDomainInformation
#define PolicyLsaServerRoleInformation PolicyLsaServerRoleInformation
#define PolicyReplicaSourceInformation PolicyReplicaSourceInformation 
#define PolicyDefaultQuotaInformation PolicyDefaultQuotaInformation
#define PolicyModificationInformation PolicyModificationInformation
#define PolicyAuditFullSetInformation PolicyAuditFullSetInformation
#define PolicyAuditFullQueryInformation PolicyAuditFullQueryInformation
#define PolicyDnsDomainInformation PolicyDnsDomainInformation

// POLICY_AUDIT_EVENT_TYPE values
#define AuditCategorySystem AuditCategorySystem
#define AuditCategoryLogon AuditCategoryLogon
#define AuditCategoryObjectAccess AuditCategoryObjectAccess
#define AuditCategoryPrivilegeUse AuditCategoryPrivilegeUse
#define AuditCategoryDetailedTracking AuditCategoryDetailedTracking
#define AuditCategoryPolicyChange AuditCategoryPolicyChange
#define AuditCategoryAccountManagement AuditCategoryAccountManagement
#define AuditCategoryDirectoryServiceAccess AuditCategoryDirectoryServiceAccess
#define AuditCategoryAccountLogon AuditCategoryAccountLogon

// EventAuditingOptions flags - bitmask of these is returned/set for each index in the above enum
#define POLICY_AUDIT_EVENT_UNCHANGED POLICY_AUDIT_EVENT_UNCHANGED  // For set operations, specify this value to leave the current options unchanged. This is the default. 
#define POLICY_AUDIT_EVENT_SUCCESS POLICY_AUDIT_EVENT_SUCCESS      // Generate audit records for successful events of this type. 
#define POLICY_AUDIT_EVENT_FAILURE POLICY_AUDIT_EVENT_FAILURE      // Generate audit records for failed attempts to cause an event of this type to occur. 
#define POLICY_AUDIT_EVENT_NONE POLICY_AUDIT_EVENT_NONE            // Do not generate audit records for events of this type. 

// POLICY_LSA_SERVER_ROLE values
#define PolicyServerRoleBackup PolicyServerRoleBackup 
#define PolicyServerRolePrimary PolicyServerRolePrimary

// POLICY_SERVER_ENABLE_STATE 
// markh fails with these!?
//#define PolicyServerEnabled PolicyServerEnabled 
//#define PolicyServerDisabled PolicyServerDisabled

// POLICY_NOTIFICATION_INFORMATION_CLASS
#define PolicyNotifyAuditEventsInformation PolicyNotifyAuditEventsInformation
#define PolicyNotifyAccountDomainInformation PolicyNotifyAccountDomainInformation
#define PolicyNotifyServerRoleInformation PolicyNotifyServerRoleInformation
#define PolicyNotifyDnsDomainInformation PolicyNotifyDnsDomainInformation
#define PolicyNotifyDomainEfsInformation PolicyNotifyDomainEfsInformation
#define PolicyNotifyDomainKerberosTicketInformation PolicyNotifyDomainKerberosTicketInformation
#define PolicyNotifyMachineAccountPasswordInformation PolicyNotifyMachineAccountPasswordInformation

// TRUSTED_INFORMATION_CLASS
#define TrustedDomainNameInformation TrustedDomainNameInformation
#define TrustedControllersInformation TrustedControllersInformation
#define TrustedPosixOffsetInformation TrustedPosixOffsetInformation
#define TrustedPasswordInformation TrustedPasswordInformation
#define TrustedDomainInformationBasic TrustedDomainInformationBasic
#define TrustedDomainInformationEx TrustedDomainInformationEx
#define TrustedDomainAuthInformation TrustedDomainAuthInformation
#define TrustedDomainFullInformation TrustedDomainFullInformation
#define TrustedDomainAuthInformationInternal TrustedDomainAuthInformationInternal
#define TrustedDomainFullInformationInternal TrustedDomainFullInformationInternal
#define TrustedDomainInformationEx2Internal TrustedDomainInformationEx2Internal
#define TrustedDomainFullInformation2Internal TrustedDomainFullInformation2Internal

// AceFlags
#define CONTAINER_INHERIT_ACE CONTAINER_INHERIT_ACE
#define FAILED_ACCESS_ACE_FLAG FAILED_ACCESS_ACE_FLAG
#define INHERIT_ONLY_ACE INHERIT_ONLY_ACE
#define INHERITED_ACE INHERITED_ACE
#define NO_PROPAGATE_INHERIT_ACE NO_PROPAGATE_INHERIT_ACE
#define OBJECT_INHERIT_ACE OBJECT_INHERIT_ACE
#define SUCCESSFUL_ACCESS_ACE_FLAG SUCCESSFUL_ACCESS_ACE_FLAG
#define NO_INHERITANCE NO_INHERITANCE
#define SUB_CONTAINERS_AND_OBJECTS_INHERIT SUB_CONTAINERS_AND_OBJECTS_INHERIT
#define SUB_CONTAINERS_ONLY_INHERIT SUB_CONTAINERS_ONLY_INHERIT
#define SUB_OBJECTS_ONLY_INHERIT SUB_OBJECTS_ONLY_INHERIT


// ACCESS_MODE - used in SetEntriesInAcl
#define NOT_USED_ACCESS NOT_USED_ACCESS  
#define GRANT_ACCESS GRANT_ACCESS
#define SET_ACCESS SET_ACCESS
#define DENY_ACCESS DENY_ACCESS 
#define REVOKE_ACCESS REVOKE_ACCESS
#define SET_AUDIT_SUCCESS SET_AUDIT_SUCCESS
#define SET_AUDIT_FAILURE SET_AUDIT_FAILURE

// TRUSTEE_FORM enum
#define TRUSTEE_IS_SID TRUSTEE_IS_SID
#define TRUSTEE_IS_NAME TRUSTEE_IS_NAME
#define TRUSTEE_BAD_FORM TRUSTEE_BAD_FORM
#define TRUSTEE_IS_OBJECTS_AND_SID TRUSTEE_IS_OBJECTS_AND_SID
#define TRUSTEE_IS_OBJECTS_AND_NAME TRUSTEE_IS_OBJECTS_AND_NAME

// TRUSTEE_TYPE
#define TRUSTEE_IS_UNKNOWN TRUSTEE_IS_UNKNOWN
#define TRUSTEE_IS_USER TRUSTEE_IS_USER
#define TRUSTEE_IS_GROUP TRUSTEE_IS_GROUP
#define TRUSTEE_IS_DOMAIN TRUSTEE_IS_DOMAIN
#define TRUSTEE_IS_ALIAS TRUSTEE_IS_ALIAS
#define TRUSTEE_IS_WELL_KNOWN_GROUP TRUSTEE_IS_WELL_KNOWN_GROUP
#define TRUSTEE_IS_DELETED TRUSTEE_IS_DELETED
#define TRUSTEE_IS_INVALID TRUSTEE_IS_INVALID
#define TRUSTEE_IS_COMPUTER TRUSTEE_IS_COMPUTER

#define SE_PRIVILEGE_ENABLED_BY_DEFAULT SE_PRIVILEGE_ENABLED_BY_DEFAULT
#define SE_PRIVILEGE_ENABLED SE_PRIVILEGE_ENABLED
#define SE_PRIVILEGE_USED_FOR_ACCESS SE_PRIVILEGE_USED_FOR_ACCESS
