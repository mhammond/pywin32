//
// @doc

#include "Python.h"
#ifndef MS_WINCE /* This source is not included for WinCE */
#include "windows.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "accctrl.h"
#include "aclapi.h"

// @pymethod <o PyACL>|pywintypes|ACL|Creates a new ACL object
PyObject *PyWinMethod_NewACL(PyObject *self, PyObject *args)
{
	int bufSize = 64;
	int aclrev = ACL_REVISION;
	// @pyparm int|bufSize|64|The size for the ACL.
	if (!PyArg_ParseTuple(args, "|ii:ACL", &bufSize, &aclrev))
		return NULL;
	return new PyACL(bufSize,aclrev);
}

BOOL PyWinObject_AsACL(PyObject *ob, PACL *ppACL, BOOL bNoneOK /*= FALSE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppACL = NULL;
	} else if (!PyACL_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyACL object");
		return FALSE;
	} else {
		*ppACL = ((PyACL *)ob)->GetACL();
	}
	return TRUE;
}

// @pymethod |PyACL|Initialize|Initialize the ACL.
// @comm It should not be necessary to call this, as the ACL object
// is initialised by Python.  This method gives you a chance to trap
// any errors that may occur.
PyObject *PyACL::Initialize(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	PACL pacl=This->GetACL();
	if (!PyArg_ParseTuple(args, ":Initialize"))
		return NULL;
	if (!::InitializeAcl(pacl, This->bufSize, pacl->AclRevision))
		return PyWin_SetAPIError("InitializeAcl");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyACL|IsValid|Determines if the ACL is valid (IsValidAcl)
PyObject *PyACL::IsValid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsValid"))
		return NULL;
	PyACL *This = (PyACL *)self;
	return PyInt_FromLong(IsValidAcl(This->GetACL()));
}

BOOL _ReorderACL(PACL pacl)
{
	// acls have to have ACCESS_DENIED_ACE's first - what a pain
	if (pacl->AceCount <= 1)
		return TRUE;
	BOOL ret = TRUE;
	DWORD aceind = 0;
	DWORD ace_insert_ind = (DWORD)-1;
	DWORD aclsize=pacl->AclSize;
	DWORD acecount=pacl->AceCount;
	DWORD aclrev = pacl->AclRevision;
	PACE_HEADER pace;

	// create new acl to copy ace's into in correct order
	PACL pacl_ordered=(ACL *)malloc(aclsize);
	ZeroMemory(pacl_ordered,aclsize);
	if (!::InitializeAcl(pacl_ordered,aclsize,aclrev))
		goto done;

	// add all denied ace's first
	for (aceind=0;aceind<acecount;aceind++){
		if (!GetAce(pacl, aceind, (void **) &pace)){
			PyWin_SetAPIError("ReorderACL");
			ret=FALSE;
			goto done;
			}
		if ((pace->AceType == ACCESS_DENIED_ACE_TYPE)||
			(pace->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)){
			ace_insert_ind++;
			if (!AddAce(pacl_ordered, aclrev, ace_insert_ind, pace, pace->AceSize)){
				PyWin_SetAPIError("ReorderACL");
				ret=FALSE;
				goto done;
				}
			}
		}
	// add all access allowed ace's
	for (aceind=0;aceind<acecount;aceind++){
		if (!GetAce(pacl, aceind, (void **) &pace)){
			PyWin_SetAPIError("ReorderACL");
			ret=FALSE;
			goto done;
			}
		if ((pace->AceType != ACCESS_DENIED_ACE_TYPE)&&
			(pace->AceType != ACCESS_DENIED_OBJECT_ACE_TYPE)){
			ace_insert_ind++;
			if (!AddAce(pacl_ordered, aclrev, ace_insert_ind, pace, pace->AceSize)){
				PyWin_SetAPIError("ReorderACL");
				ret=FALSE;
				goto done;
				}
			}
		}
	// copy reordered ACL back to old location
	memcpy(pacl,pacl_ordered,aclsize);
	done:
		free(pacl_ordered);
		return ret;
}

BOOL PyWinObject_AsTRUSTEE(PyObject *ob, TRUSTEE *ptrustee)
{
	PyObject *dict_item;
	char* err_msg="Trustee must be a dictionary containing {MultipleTrustee,MultipleTrusteeOperation,TrusteeForm,TrusteeType,Identifier}";
	if (!PyMapping_Check(ob)){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	/* Multiple trustees not currently supported according to SDK
	dict_item=PyMapping_GetItemString(ob,"MultipleTrustee");
	if (dict_item==NULL || dict_item==Py_None){
		PyErr_Clear();
		ptrustee->pMultipleTrustee=NULL;
		}
	else
		// hope nobody ever creates one that chains back to itself......
		if (!PyWinObject_AsTRUSTEE(dict_item, ptrustee->pMultipleTrustee, FALSE))
			return FALSE;
	Py_XDECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"MultipleTrusteeOperation");
	if (dict_item==NULL || dict_item==Py_None){
		PyErr_Clear();
		ptrustee->MultipleTrusteeOperation=NO_MULTIPLE_TRUSTEE;
		}
	else{
		if(!PyInt_Check(dict_item)){
			PyErr_SetString(PyExc_TypeError,"MultipleTrusteeOperation must be an int from MULTIPLE_TRUSTEE_OPERATION enum");
			Py_DECREF(dict_item);
			return FALSE;
			}
		ptrustee->MultipleTrusteeOperation=(MULTIPLE_TRUSTEE_OPERATION)PyLong_AsLong(dict_item);
		}
	Py_XDECREF(dict_item);
	*/
	ptrustee->MultipleTrusteeOperation=NO_MULTIPLE_TRUSTEE;
	ptrustee->pMultipleTrustee=NULL;
	dict_item=PyMapping_GetItemString(ob,"TrusteeForm");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if(!PyInt_Check(dict_item)){
		PyErr_SetString(PyExc_TypeError,"TrusteeForm must be an int from TRUSTEE_FORM enum");
		Py_DECREF(dict_item);
		return FALSE;
		}
	ptrustee->TrusteeForm=(TRUSTEE_FORM)PyLong_AsLong(dict_item);
	Py_DECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"TrusteeType");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if(!PyInt_Check(dict_item)){
		PyErr_SetString(PyExc_TypeError,"TrusteeType must be an int from TRUSTEE_TYPE enum");
		Py_DECREF(dict_item);
		return FALSE;
		}
	ptrustee->TrusteeType=(TRUSTEE_TYPE)PyLong_AsLong(dict_item);
	Py_DECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"Identifier");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}

	switch (ptrustee->TrusteeForm){
		case TRUSTEE_IS_SID:{
			if (!PyWinObject_AsSID(dict_item,(PSID *)&ptrustee->ptstrName,FALSE)){
				Py_DECREF(dict_item);
				return FALSE;
				}
			break;
			}
		case TRUSTEE_IS_NAME:{
			if (!PyString_Check(dict_item)){
				PyErr_SetString(PyExc_TypeError,"Identifier must be a string when TrusteeForm = TRUSTEE_IS_NAME");
				Py_DECREF(dict_item);
				return FALSE;
				}
			ptrustee->ptstrName=PyString_AsString(dict_item);
			break;
			}		
		default:{
			PyErr_SetString(PyExc_NotImplementedError, "TrusteeForm not yet supported");
			Py_DECREF(dict_item);
			return FALSE;
			}
		}
	Py_DECREF(dict_item);
	return TRUE;
}

PyObject *PyWinObject_FromTRUSTEE(TRUSTEE *ptrustee)
{
	PyObject *ret = PyDict_New();
	PyObject *dict_item;
	// first 2 members are not currently used
	PyDict_SetItemString(ret,"MultipleTrustee",Py_None);
	dict_item=PyLong_FromDouble(NO_MULTIPLE_TRUSTEE);
	PyDict_SetItemString(ret,"MultipleTrusteeOperation",dict_item);
	Py_DECREF(dict_item);
	dict_item=PyLong_FromDouble(ptrustee->TrusteeForm);
	PyDict_SetItemString(ret,"TrusteeForm",dict_item);
	Py_DECREF(dict_item);
	dict_item=PyLong_FromDouble(ptrustee->TrusteeType);
	PyDict_SetItemString(ret,"TrusteeType",dict_item);
	Py_DECREF(dict_item);
	switch (ptrustee->TrusteeForm){
		case TRUSTEE_IS_SID:{
			dict_item=PyWinObject_FromSID(ptrustee->ptstrName);
			break;
			}
		case TRUSTEE_IS_NAME:{
			dict_item=PyString_FromString(ptrustee->ptstrName);
			break;
			}		
		default:{
			PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
			return NULL;
			}
		}
	PyDict_SetItemString(ret,"Identifier",dict_item);
	Py_DECREF(dict_item);
	return ret;
}

BOOL PyWinObject_AsEXPLICIT_ACCESS(PyObject *ob, PEXPLICIT_ACCESS pexpl)
{
	PyObject *dict_item=NULL;
	char* err_msg="EXPLICIT_ACCESS must be a dictionary containing {AccessPermissions,AccessMode,Inheritance,Trustee}";
	if (!PyMapping_Check(ob)){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	dict_item=PyMapping_GetItemString(ob,"AccessPermissions");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if(!PyInt_Check(dict_item)){
		PyErr_SetString(PyExc_TypeError,"AccessPermissions must be an int");
		Py_DECREF(dict_item);
		return FALSE;
		}
	pexpl->grfAccessPermissions=PyLong_AsLong(dict_item);
	Py_DECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"AccessMode");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if(!PyInt_Check(dict_item)){
		PyErr_SetString(PyExc_TypeError,"AccessMode must be an int from ACCESS_MODE enum");
		Py_DECREF(dict_item);
		return FALSE;
		}
	pexpl->grfAccessMode=(ACCESS_MODE)PyLong_AsLong(dict_item);
	Py_DECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"Inheritance");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if(!PyInt_Check(dict_item)){
		PyErr_SetString(PyExc_TypeError,"Inheritance must be an int (combination of ace flags");
		Py_DECREF(dict_item);
		return FALSE;
		}
	pexpl->grfInheritance=PyLong_AsLong(dict_item);
	Py_DECREF(dict_item);

	dict_item=PyMapping_GetItemString(ob,"Trustee");
	if (dict_item==NULL){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if (!PyWinObject_AsTRUSTEE(dict_item,&pexpl->Trustee)){
		Py_DECREF(dict_item);
		return FALSE;
		}
	Py_DECREF(dict_item);
	return TRUE;
}


PyObject *PyWinObject_FromEXPLICIT_ACCESS(EXPLICIT_ACCESS *pexpl)
{
	PyObject *ret = PyDict_New();
	PyObject *dict_item;
	dict_item=PyLong_FromDouble(pexpl->grfAccessPermissions);
	PyDict_SetItemString(ret,"AccessPermissions",dict_item);
	Py_DECREF(dict_item);
	dict_item=PyLong_FromDouble(pexpl->grfAccessMode);
	PyDict_SetItemString(ret,"AccessMode",dict_item);
	Py_DECREF(dict_item);
	dict_item=PyLong_FromDouble(pexpl->grfInheritance);
	PyDict_SetItemString(ret,"Inheritance",dict_item);
	Py_DECREF(dict_item);
    dict_item=PyWinObject_FromTRUSTEE(&(pexpl->Trustee));
	PyDict_SetItemString(ret,"Trustee",dict_item);
	Py_DECREF(dict_item);
    return ret;
}

PyObject *PyACL::PyGetExplicitEntriesFromAcl(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	PyObject *ret=NULL;
	PyObject *obexpl;
	PEXPLICIT_ACCESS pList,pListstart = NULL;
	DWORD access_cnt = 0;
	DWORD access_ind, err;
	err = ::GetExplicitEntriesFromAcl(This->GetACL(), &access_cnt, &pListstart);
	if (err != ERROR_SUCCESS)
		return PyWin_SetAPIError("GetExplicitEntriesFromAcl",err);
	ret = PyTuple_New(access_cnt);
	pList=pListstart;
	for (access_ind=0; access_ind<access_cnt; access_ind++){
		obexpl = PyWinObject_FromEXPLICIT_ACCESS(pList);
		PyTuple_SetItem(ret, access_ind, obexpl);
		// Py_DECREF(obexpl);
		pList++;
		}
	LocalFree(pListstart);
	return ret;
}


// @pymethod |PyACL|AddAccessAllowedAce|Adds an access-allowed ACE to an DACL object. The access is granted to a specified SID.
PyObject *PyACL::AddAccessAllowedAce(PyObject *self, PyObject *args)
{
	DWORD access,revision;
	PyObject *obSID;
	PSID psid;
	PyACL *This = (PyACL *)self;
	PACL pdacl = This->GetACL();
	PACL pdacl_padded=NULL;
	// Need to support 2 arg styles for b/w compat.
	if (PyArg_ParseTuple(args, "lO:AddAccessAllowedAce", &access, &obSID)) {
		// We worked - is old style 
		// @pyparmalt1 int|access||Specifies the mask of access rights to be denied to the specified SID.
		// @pyparmalt1 <o PySID>|sid||A SID object representing a user, group, or logon account being denied access. 
		revision = ACL_REVISION;
	} else {
		// Try new style (we use new style last so that
		// exceptions report the new style rather than old
		// @pyparm int|revision||Pre-win2k, must be ACL_REVISION, otherwise also may be ACL_REVISION_DS.
		// @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
		// @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access. 
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "llO:AddAccessAllowedAce", &revision, &access, &obSID))
			return NULL;
	}
	if (!PyWinObject_AsSID(obSID, &psid, FALSE))
		return NULL;
	if (!::AddAccessAllowedAce(pdacl, revision, access, psid)){
		DWORD err=GetLastError();
		if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
			return PyWin_SetAPIError("AddAccessAllowedAce", err);
		// resize if dacl too small
		unsigned short required_size=pdacl->AclSize + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid);
		pdacl_padded = (ACL *)malloc(required_size);
		ZeroMemory (pdacl_padded, required_size);
		memcpy(pdacl_padded,pdacl,pdacl->AclSize);
		pdacl_padded->AclSize = required_size;
		if (!::AddAccessAllowedAce(pdacl_padded, revision, access,  psid)){
			free (pdacl_padded);
			return PyWin_SetAPIError("AddAccessAllowedAce");
			}
		This->SetACL(pdacl_padded);
		}
	if (pdacl_padded)
		free(pdacl_padded);
	Py_INCREF(Py_None);
	return Py_None;
	// @comm Note that early versions of this function supported only
	// two arguments.  This has been deprecated in preference of the
	// three argument version, which reflects the win32 API and the new
	// functions in this module.
}

// @pymethod |PyACL|AddAccessDeniedAce|Adds an access-denied ACE to an ACL object. The access is denied to a specified SID.
PyObject *PyACL::AddAccessDeniedAce(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	DWORD access,revision;
	PyObject *obSID;
	PSID psid;
	PyACL *This = (PyACL *)self;
	PACL pdacl = This->GetACL();
	PACL pdacl_padded = NULL;
	// Need to support 2 arg styles for b/w compat.
	if (PyArg_ParseTuple(args, "lO:AddAccessDeniedAce", &access, &obSID)) {
		// We worked - is old style 
		// @pyparmalt1 int|access||Specifies the mask of access rights to be denied to the specified SID.
		// @pyparmalt1 <o PySID>|sid||A SID object representing a user, group, or logon account being denied access. 
		revision = ACL_REVISION;
	} else {
		// Try new style (we use new style last so that
		// exceptions report the new style rather than old
		// @pyparm int|revision||Pre-win2k, must be ACL_REVISION, otherwise also may be ACL_REVISION_DS.
		// @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
		// @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access. 
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "llO:AddAccessDeniedAce", &revision, &access, &obSID))
			return NULL;
	}
	if (!PyWinObject_AsSID(obSID, &psid, FALSE))
		return NULL;
	if (!::AddAccessDeniedAce(pdacl, revision, access, psid)){
		DWORD err=GetLastError();
		if (err != ERROR_ALLOTTED_SPACE_EXCEEDED){
			PyWin_SetAPIError("AddAccessDeniedAce",err);
			goto done;
			}
		// resize if dacl too small
		unsigned short required_size=pdacl->AclSize + sizeof(ACCESS_DENIED_ACE) + GetLengthSid(psid);
		pdacl_padded = (ACL *)malloc(required_size);
		ZeroMemory(pdacl_padded,required_size);
		memcpy(pdacl_padded,pdacl,pdacl->AclSize);
		pdacl_padded->AclSize=required_size;
		if (!::AddAccessDeniedAce(pdacl_padded, revision, access,  psid)){
			PyWin_SetAPIError("AddAccessDeniedAce");
			goto done;
			}
		if (!_ReorderACL(pdacl_padded))
			goto done;
		This->SetACL(pdacl_padded);
		}
	else{
		if (!_ReorderACL(pdacl))
			goto done;
		}

	ret=Py_None;
	done:
		if (pdacl_padded)
			free(pdacl_padded);
		Py_XINCREF(ret);
		return ret;
	// @comm Note that early versions of this function supported only
	// two arguments.  This has been deprecated in preference of the
	// three argument version, which reflects the win32 API and the new
	// functions in this module.
}

// AddAuditAccessAce
// @pymethod |PyACL|AddAuditAccessAce|Adds an audit ACE to a Sacl
PyObject *PyACL::AddAuditAccessAce(PyObject *self, PyObject *args)
{
	DWORD accessmask,acerevision;
	BOOL  bAuditSuccess, bAuditFailure;
	PyObject *obSID;
	PSID psid;
	PACL psacl;
	PyACL *This = (PyACL *)self;
	psacl = This->GetACL();
	PACL psacl_padded=NULL;

	// @pyparm int|dwAceRevision||Revision of ACL: Pre-Win2k, must be ACL_REVISION. Win2K on up, can also be ACL_REVISION_DS
    // @pyparm int|dwAccessMask||Bitmask of access types to be audited
	// @pyparm <o PySID>|sid||SID for whom system audit messages will be generated
	// @pyparm int|bAuditSuccess||Set to 1 if access success should be audited, else 0
	// @pyparm int|bAuditFailure||Set to 1 if access failure should be audited, else 0

	if (!PyArg_ParseTuple(args, "llOii:AddAuditAccessAce", &acerevision, &accessmask, &obSID, &bAuditSuccess, &bAuditFailure))
		return NULL;
	if (!PyWinObject_AsSID(obSID, &psid, FALSE))
		return NULL;
	if (!::AddAuditAccessAce(psacl, acerevision, accessmask,  psid, bAuditSuccess, bAuditFailure)){
		DWORD err=GetLastError();
		if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
			return PyWin_SetAPIError("AddAuditAccessAce", err);
		// resize if acl too small
		unsigned short required_size=psacl->AclSize + sizeof(SYSTEM_AUDIT_ACE) + GetLengthSid(psid);
		psacl_padded = (ACL *)malloc(required_size);
		ZeroMemory (psacl_padded, required_size);
		memcpy(psacl_padded,psacl,psacl->AclSize);
		psacl_padded->AclSize = required_size;
		if (!::AddAuditAccessAce(psacl_padded, acerevision, accessmask,  psid, bAuditSuccess, bAuditFailure)){
			free (psacl_padded);
			return PyWin_SetAPIError("AddAuditAccessAce");
			}
		This->SetACL(psacl_padded);
		}
	if (psacl_padded)
		free(psacl_padded);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyACL|GetAclSize|Returns the storage size of the ACL.
PyObject *PyACL::GetAclSize(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	PACL pacl;

	if (!PyArg_ParseTuple(args, ":GetAclSize"))
		return NULL;

	pacl= This->GetACL();
	return Py_BuildValue("l", pacl->AclSize);
}

// @pymethod |PyACL|GetAclSize|Returns the storage size of the ACL.
PyObject *PyACL::GetAclRevision(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	PACL pacl;

	if (!PyArg_ParseTuple(args, ":GetAclSize"))
		return NULL;

	pacl= This->GetACL();
	return Py_BuildValue("l", pacl->AclRevision);
}

// @pymethod |PyACL|GetAceCount|Returns the number of ACEs in the ACL.
PyObject *PyACL::GetAceCount(PyObject *self, PyObject *args)
{
	PyACL *This = (PyACL *)self;
	PACL pacl;

	if (!PyArg_ParseTuple(args, ":GetAceCount"))
		return NULL;

	pacl= This->GetACL();
	return Py_BuildValue("l", pacl->AceCount);
}


// @pymethod |PyACL|GetAce|Gets an Ace from the ACL. Returns tuple ((aceType, AceFlags), Mask, SID). For details see the API documentation: http://msdn.microsoft.com/library/psdk/winbase/acctrlow_22r6.htm.
PyObject *PyACL::GetAce(PyObject *self, PyObject *args)
{
	DWORD index;
	PyObject *obNewSid;
	ACCESS_ALLOWED_ACE *pAce;
	ACE_HEADER *pAceHeader;
	LPVOID p;
	PyACL *This = (PyACL *)self;
	// @pyparm int|index||Zero-based index of the ACE to retrieve.
	if (!PyArg_ParseTuple(args, "l:GetAce", &index))
		return NULL;
	if (!::GetAce(This->GetACL(), index, &p))
		return PyWin_SetAPIError("GetAce");
	pAce= ((ACCESS_ALLOWED_ACE *)p);
	// create pySID object
	obNewSid= new PySID((PSID)(&pAce->SidStart));
	// get ACE Header pointer
	pAceHeader= &pAce->Header;
	return Py_BuildValue("(ll)lN", pAceHeader->AceType, pAceHeader->AceFlags, pAce->Mask, obNewSid);
}

// @pymethod |PyACL|DeleteAce|Deletes specified Ace from an ACL.
PyObject *PyACL::DeleteAce(PyObject *self, PyObject *args)
{
	DWORD index;
	PyACL *This = (PyACL *)self;
	// @pyparm int|index||Zero-based index of the ACE to delete.
	if (!PyArg_ParseTuple(args, "l:DeleteAce", &index))
		return NULL;
	if (!::DeleteAce(This->GetACL(), index))
		return PyWin_SetAPIError("DeleteAce");
    Py_INCREF(Py_None);
	return Py_None;
}

PyObject *PyACL::PySetEntriesInAcl(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PEXPLICIT_ACCESS pexpl=NULL, pexpl_start=NULL;
	ACL *new_acl=NULL;
	PyACL *This = (PyACL *)self;
	PyObject *obexpl=NULL, *obexpl_list=NULL;
	unsigned long expl_cnt, expl_ind;
	DWORD err;
	if (!PyArg_ParseTuple(args, "O:SetEntriesInAcl", &obexpl_list))
		return NULL;
	if (!PySequence_Check(obexpl_list)){
		PyErr_SetString(PyExc_TypeError, "Parm must be a list of EXPLICIT_ACCESS dictionaries");
		return NULL;
		}
	expl_cnt=PySequence_Length(obexpl_list);
	pexpl_start=(EXPLICIT_ACCESS *)calloc(expl_cnt, sizeof(EXPLICIT_ACCESS));
	pexpl=pexpl_start;
	for (expl_ind=0; expl_ind<expl_cnt; expl_ind++){
		obexpl = PySequence_GetItem(obexpl_list,expl_ind);
		if (!PyWinObject_AsEXPLICIT_ACCESS(obexpl,pexpl)){
			Py_DECREF(obexpl);
			goto done;
			}
		Py_DECREF(obexpl);
		pexpl++;
		}
	err = ::SetEntriesInAcl(expl_cnt,pexpl_start,This->GetACL(),&new_acl);
	if (err!=ERROR_SUCCESS){
		PyWin_SetAPIError("SetEntriesInAcl",err);
		goto done;
		}
	This->SetACL(new_acl);
	ret=Py_None;
	done:
		if (pexpl_start!=NULL)
			free(pexpl_start);
		if (new_acl)
			LocalFree(new_acl);
		Py_XINCREF(ret);
		return ret;
}


// @object PyACL|A Python object, representing a ACL structure
static struct PyMethodDef PyACL_methods[] = {
	{"Initialize",     PyACL::Initialize, 1}, 	// @pymeth Initialize|Initialize the ACL.
	{"IsValid",     PyACL::IsValid, 1}, 	// @pymeth IsValid|Validate the ACL.
	{"AddAccessAllowedAce",     PyACL::AddAccessAllowedAce, 1}, 	// @pymeth AddAccessAllowedAce|Adds an access-allowed ACE to an ACL object.
	{"AddAccessDeniedAce",     PyACL::AddAccessDeniedAce, 1}, 	// @pymeth AddAccessDeniedAce|Adds an access-denied ACE to an ACL object.
	{"AddAuditAccessAce",     PyACL::AddAuditAccessAce, 1}, 	// @pymeth AddAuditAccessAce|Adds an audit entry to a system access control list (SACL)
	{"GetAclSize", PyACL::GetAclSize, 1},  // @pymeth GetAclSize|Returns the storage size of the ACL.
	{"GetAclRevision", PyACL::GetAclRevision, 1},  // @pymeth GetAclRevision|Returns the revision nbr of the ACL.
	{"GetAceCount", PyACL::GetAceCount, 1},  // @pymeth GetAceCount|Returns the number of ACEs in the ACL.
	{"GetAce", PyACL::GetAce, 1},  // @pymeth GetAce|Returns an ACE from the ACL.
	{"DeleteAce", PyACL::DeleteAce, 1},  // @pymeth DeleteAce|Delete an access-control entry (ACE) from an ACL
	{"GetExplicitEntriesFromAcl", PyACL::PyGetExplicitEntriesFromAcl, 1},  // @pymeth GetExplicitEntriesFromAcl|Retrieve list of EXPLICIT_ACCESSs from the ACL
	{"SetEntriesInAcl", PyACL::PySetEntriesInAcl, 1},  // @pymeth SetEntriesInAcl|Adds a list of EXPLICIT_ACCESSs to an ACL
	{NULL}
};

PYWINTYPES_EXPORT PyTypeObject PyACLType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyACL",
	sizeof(PyACL),
	0,
	PyACL::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyACL::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	0,
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};


PyACL::PyACL(int createBufSize, int aclrev)
{
	ob_type = &PyACLType;
	_Py_NewReference(this);
	bufSize = createBufSize;
	buf = malloc(bufSize);
	memset(buf, 0, bufSize);

	::InitializeAcl(GetACL(), bufSize, aclrev);
}

PyACL::PyACL(PACL pacl)
{
	ob_type = &PyACLType;
	_Py_NewReference(this);
	bufSize = pacl->AclSize;
	buf = malloc(bufSize);
	memcpy(buf, (void *)pacl, bufSize);	
}

PyACL::~PyACL()
{
	free(buf);
}

PyObject *PyACL::getattr(PyObject *self, char *name)
{
	return Py_FindMethod(PyACL_methods, self, name);
}

/*static*/ void PyACL::deallocFunc(PyObject *ob)
{
	delete (PyACL *)ob;
}

#endif /* MS_WINCE */
