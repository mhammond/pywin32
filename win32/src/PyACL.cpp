//
// @doc

#include "PyWinTypes.h"

#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#include "accctrl.h"
#include "aclapi.h"

addacefunc addaccessallowedace = NULL;
addacefunc addaccessdeniedace = NULL;
addaceexfunc addaccessallowedaceex = NULL;
addaceexfunc addaccessdeniedaceex = NULL;
addaceexfunc addmandatoryace = NULL;
addobjectacefunc addaccessallowedobjectace = NULL;
addobjectacefunc addaccessdeniedobjectace = NULL;
BOOL(WINAPI *addauditaccessaceex)(PACL, DWORD, DWORD, DWORD, PSID, BOOL, BOOL) = NULL;
BOOL(WINAPI *addauditaccessobjectace)(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID, BOOL, BOOL) = NULL;

// @pymethod <o PyACL>|pywintypes|ACL|Creates a new ACL object
PyObject *PyWinMethod_NewACL(PyObject *self, PyObject *args)
{
    int bufSize = 64;
    int aclrev = ACL_REVISION;
    // @pyparm int|bufSize|64|The size for the ACL.
    if (!PyArg_ParseTuple(args, "|ii:ACL", &bufSize, &aclrev))
        return NULL;
    return new PyACL(bufSize, aclrev);
}

BOOL PyWinObject_AsACL(PyObject *ob, PACL *ppACL, BOOL bNoneOK /*= FALSE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppACL = NULL;
    }
    else if (!PyACL_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyACL object");
        return FALSE;
    }
    else {
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
    PACL pacl = This->GetACL();
    if (!PyArg_ParseTuple(args, ":Initialize"))
        return NULL;
    if (!::InitializeAcl(pacl, pacl->AclSize, pacl->AclRevision))
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
    return PyLong_FromLong(IsValidAcl(This->GetACL()));
}

BOOL _ReorderACL(PACL pacl)
{
    // acls used to only have ACCESS_DENIED_ACE's first - more complicated rules with object & inherited ACEs
    if (pacl->AceCount <= 1)
        return TRUE;
    BOOL ret = TRUE;
    DWORD aceind = 0;
    DWORD ace_insert_ind;
    DWORD aclsize = pacl->AclSize;
    DWORD acecount = pacl->AceCount;
    DWORD aclrev = pacl->AclRevision;
    PACE_HEADER pace;
    // ??? put these in an array so they can be incremented in a loop ???
    DWORD access_denied_pos = 0, access_denied_object_pos = 0;
    DWORD access_allowed_pos = 0, access_allowed_object_pos = 0;
    DWORD inherited_pos = 0;

    // create new acl to copy ace's into in correct order
    PACL pacl_ordered = (ACL *)malloc(aclsize);
    if (pacl_ordered == NULL) {
        PyErr_Format(PyExc_MemoryError, "Error reordering ACL: Unable to allocate acl of size %d", aclsize);
        return FALSE;
    }
    ZeroMemory(pacl_ordered, aclsize);
    if (!::InitializeAcl(pacl_ordered, aclsize, aclrev)) {
        ret = FALSE;
        goto done;
    }

    /* add ACE's in correct order, keep index of position at which to add each type
       ACCESS_DENIED_ACE_TYPE
       ACCESS_DENIED_OBJECT_ACE_TYPE
       ACCESS_ALLOWED_ACE_TYPE
       ACCESS_ALLOWED_OBJECT_ACE_TYPE
       followed by inherited ACEs whose order should not be changed
    */
    for (aceind = 0; aceind < acecount; aceind++) {
        if (!GetAce(pacl, aceind, (void **)&pace)) {
            PyWin_SetAPIError("ReorderACL");
            ret = FALSE;
            goto done;
        }
        if (pace->AceFlags & INHERITED_ACE) {
            ace_insert_ind = inherited_pos;
            inherited_pos++;
        }
        else
            switch (pace->AceType) {
                case ACCESS_DENIED_ACE_TYPE:
                    ace_insert_ind = access_denied_pos;
                    access_denied_pos++;
                    access_denied_object_pos++;
                    access_allowed_pos++;
                    access_allowed_object_pos++;
                    inherited_pos++;
                    break;
                case ACCESS_DENIED_OBJECT_ACE_TYPE:
                    ace_insert_ind = access_denied_object_pos;
                    access_denied_object_pos++;
                    access_allowed_pos++;
                    access_allowed_object_pos++;
                    inherited_pos++;
                    break;
                case ACCESS_ALLOWED_ACE_TYPE:
                    ace_insert_ind = access_allowed_pos;
                    access_allowed_pos++;
                    access_allowed_object_pos++;
                    inherited_pos++;
                    break;
                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                    ace_insert_ind = access_allowed_object_pos;
                    access_allowed_object_pos++;
                    inherited_pos++;
                    break;
                default:  // there are several new types waiting in the wings, make no assumptions about them
                    PyErr_Format(PyExc_NotImplementedError, "Ace type %d is not supported yet", pace->AceType);
                    ret = FALSE;
                    goto done;
            }
        if (!AddAce(pacl_ordered, aclrev, ace_insert_ind, pace, pace->AceSize)) {
            PyWin_SetAPIError("ReorderACL");
            ret = FALSE;
            goto done;
        }
    }
    // copy reordered ACL back to old location
    memcpy(pacl, pacl_ordered, aclsize);
done:
    free(pacl_ordered);
    return ret;
}

PyObject *mapping_to_dict(PyObject *mapping)
{
    PyObject *items = NULL, *item = NULL, *new_dict = NULL;
    if (PyDict_Check(mapping)) {
        Py_INCREF(mapping);
        return mapping;
    }
    if (!PyMapping_Check(mapping)) {
        PyErr_SetString(PyExc_TypeError, "Object must be a mapping (dictionary, class instance, etc");
        return NULL;
    }

    // any generic mapping (theoretically) - tested using class that defines its own items method
    items = PyMapping_Items(mapping);
    if (items != NULL) {
        new_dict = PyDict_New();
        if (new_dict != NULL) {
            for (int item_ind = 0; item_ind < PySequence_Length(items); item_ind++) {
                item = PySequence_GetItem(items, item_ind);
                PyDict_SetItem(new_dict, PyTuple_GetItem(item, 0), PyTuple_GetItem(item, 1));
                Py_DECREF(item);
            }
        }
        Py_DECREF(items);
        return new_dict;
    }

    PyErr_Clear();
    // PyMapping_Items doesn't work for class instances (no "items" method ????)
    new_dict = PyObject_GetAttrString(mapping, "__dict__");
    return new_dict;
}

void PyWinObject_FreeTRUSTEE(PTRUSTEE_W ptrustee)
{
    if ((ptrustee->TrusteeForm == TRUSTEE_IS_NAME) && (ptrustee->ptstrName != NULL))
        PyWinObject_FreeWCHAR(ptrustee->ptstrName);
    /*
    if (ptrustee->pMultipleTrustee!=NULL){
        PyWinObject_FreeTRUSTEE(ptrustee->pMultipleTrustee);
        free(ptrustee->pMultipleTrustee);
        }
    */
}

// @object PyTRUSTEE|A dictionary representing a TRUSTEE structure.
// @prop int|TrusteeForm|
// @prop int|TrusteeType|
// @prop object|Identifier|Depends on the value of TrusteeForm (string or sid)
// @prop object|MultipleTrustee|default is None
// @prop object|MultipleTrusteeOperation|default is None
BOOL PyWinObject_AsTRUSTEE(PyObject *obtrustee, TRUSTEE_W *ptrustee)
{
    static char *trustee_items[] = {
        "TrusteeForm", "TrusteeType", "Identifier", "MultipleTrustee", "MultipleTrusteeOperation", 0};
    static char *err_msg =
        "Trustee must be a dictionary containing "
        "{MultipleTrustee,MultipleTrusteeOperation,TrusteeForm,TrusteeType,Identifier}";
    BOOL bsuccess = TRUE;
    PyObject *obMultipleTrustee = Py_None, *obIdentifier = NULL;
    PyObject *trustee_dict = mapping_to_dict(obtrustee);
    if (trustee_dict == NULL)
        return FALSE;

    ZeroMemory(ptrustee, sizeof(TRUSTEE_W));
    ptrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ptrustee->pMultipleTrustee = NULL;
    PyObject *dummy_tuple = PyTuple_New(0);
    bsuccess = PyArg_ParseTupleAndKeywords(dummy_tuple, trustee_dict, "llO|Ol", trustee_items, &ptrustee->TrusteeForm,
                                           &ptrustee->TrusteeType, &obIdentifier, &obMultipleTrustee,
                                           &ptrustee->MultipleTrusteeOperation);
    Py_DECREF(dummy_tuple);
    if (!bsuccess)
        PyErr_SetString(PyExc_TypeError, err_msg);
    else {
        ptrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ptrustee->pMultipleTrustee = NULL;
        /* Multiple trustees not currently supported according to SDK
        if ((obMultipleTrustee!=NULL)&&(obMultipleTrustee!=Py_None)){
            // hope nobody ever creates one that chains back to itself......
            ptrustee->pMultipleTrustee=new(TRUSTEE_W);
            bsuccess=(PyWinObject_AsTRUSTEE(obMultipleTrustee,ptrustee->pMultipleTrustee));
            }
        */
        switch (ptrustee->TrusteeForm) {
            case TRUSTEE_IS_SID: {
                if (!PyWinObject_AsSID(obIdentifier, (PSID *)&ptrustee->ptstrName, FALSE)) {
                    PyErr_SetString(PyExc_TypeError,
                                    "Identifier must be PySID object when TrusteeForm = TRUSTEE_IS_SID");
                    bsuccess = FALSE;
                }
                break;
            }
            case TRUSTEE_IS_NAME: {
                if (!PyWinObject_AsWCHAR(obIdentifier, &ptrustee->ptstrName, FALSE)) {
                    PyErr_SetString(PyExc_TypeError,
                                    "Identifier must be string/unicode when TrusteeForm = TRUSTEE_IS_NAME");
                    bsuccess = FALSE;
                }
                break;
            }
            case TRUSTEE_IS_OBJECTS_AND_SID:
            case TRUSTEE_IS_OBJECTS_AND_NAME: {
                // still need to add TRUSTEE_IS_OBJECTS_AND_SID and TRUSTEE_IS_OBJECTS_AND_NAME
                PyErr_SetString(PyExc_NotImplementedError, "TrusteeForm not yet supported");
                bsuccess = FALSE;
                break;
            }
            default: {
                PyErr_SetString(PyExc_ValueError, "Invalid value for TrusteeForm");
                bsuccess = FALSE;
            }
        }
    }
    Py_DECREF(trustee_dict);
    return bsuccess;
}

PyObject *PyWinObject_FromTRUSTEE(TRUSTEE_W *ptrustee)
{
    PyObject *obIdentifier = NULL;
    switch (ptrustee->TrusteeForm) {
        case TRUSTEE_IS_SID: {
            obIdentifier = PyWinObject_FromSID(ptrustee->ptstrName);
            break;
        }
        case TRUSTEE_IS_NAME: {
            obIdentifier = PyWinObject_FromWCHAR(ptrustee->ptstrName);
            break;
        }
        case TRUSTEE_IS_OBJECTS_AND_SID:
        case TRUSTEE_IS_OBJECTS_AND_NAME: {
            PyErr_SetString(PyExc_NotImplementedError, "TrusteeForm not yet supported");
            return FALSE;
        }
        default: {
            PyErr_SetString(PyExc_ValueError, "Invalid value for TrusteeForm");
            return FALSE;
        }
    }
    if (!obIdentifier)
        return NULL;
    return Py_BuildValue("{s:O,s:l,s:l,s:l,s:N}", "MultipleTrustee", Py_None, "MultipleTrusteeOperation",
                         NO_MULTIPLE_TRUSTEE, "TrusteeForm", ptrustee->TrusteeForm, "TrusteeType",
                         ptrustee->TrusteeType, "Identifier", obIdentifier);
}

BOOL PyWinObject_AsEXPLICIT_ACCESS(PyObject *ob, PEXPLICIT_ACCESS_W pexpl)
{
    static char *expl_items[] = {"AccessPermissions", "AccessMode", "Inheritance", "Trustee", 0};
    static char *err_msg =
        "EXPLICIT_ACCESS must be a dictionary containing "
        "{AccessPermissions:int,AccessMode:int,Inheritance:int,Trustee:<o PyTRUSTEE>}";
    PyObject *expl_dict = NULL, *obtrustee = NULL;
    BOOL bsuccess = FALSE;
    ZeroMemory(pexpl, sizeof(EXPLICIT_ACCESS_W));
    expl_dict = mapping_to_dict(ob);
    if (expl_dict == NULL)
        return FALSE;
    PyObject *dummy_tuple = PyTuple_New(0);
    bsuccess = PyArg_ParseTupleAndKeywords(dummy_tuple, expl_dict, "lllO", expl_items, &pexpl->grfAccessPermissions,
                                           &pexpl->grfAccessMode, &pexpl->grfInheritance, &obtrustee);
    Py_DECREF(dummy_tuple);
    if (!bsuccess)
        PyErr_SetString(PyExc_TypeError, err_msg);
    else
        bsuccess = PyWinObject_AsTRUSTEE(obtrustee, &pexpl->Trustee);
    Py_DECREF(expl_dict);
    return bsuccess;
}

PyObject *PyWinObject_FromEXPLICIT_ACCESS(EXPLICIT_ACCESS_W *pexpl)
{
    return Py_BuildValue("{s:l,s:l,s:l,s:N}", "AccessPermissions", pexpl->grfAccessPermissions, "AccessMode",
                         pexpl->grfAccessMode, "Inheritance", pexpl->grfInheritance, "Trustee",
                         PyWinObject_FromTRUSTEE(&(pexpl->Trustee)));
}

PyObject *PyACL::PyGetExplicitEntriesFromAcl(PyObject *self, PyObject *args)
{
    PyACL *This = (PyACL *)self;
    PyObject *ret = NULL;
    PyObject *obexpl;
    PEXPLICIT_ACCESS_W pList, pListstart = NULL;
    DWORD access_cnt = 0;
    DWORD access_ind, err;
    err = ::GetExplicitEntriesFromAclW(This->GetACL(), &access_cnt, &pListstart);
    if (err != ERROR_SUCCESS)
        return PyWin_SetAPIError("GetExplicitEntriesFromAcl", err);
    ret = PyTuple_New(access_cnt);
    if (!ret)
        goto done;
    pList = pListstart;
    for (access_ind = 0; access_ind < access_cnt; access_ind++) {
        obexpl = PyWinObject_FromEXPLICIT_ACCESS(pList);
        if (!obexpl) {
            Py_DECREF(ret);
            ret = NULL;
            goto done;
        }
        PyTuple_SetItem(ret, access_ind, obexpl);
        pList++;
    }
done:
    LocalFree(pListstart);
    return ret;
}

PyObject *addaceorig(addacefunc addfunc, CHAR *funcname, PyACL *This, DWORD revision, DWORD access, PyObject *obSID)
{
    /* AddAccessAllowedAce and AddAccessDeniedAce operate exactly the same */
    PACL pdacl = This->GetACL();
    PACL pdacl_padded = NULL;
    BOOL bsuccess;
    PSID psid;
    if (addfunc == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "%s not supported by this version of Windows", funcname);
    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;

    bsuccess = (*addfunc)(pdacl, revision, access, psid);
    if (bsuccess)
        bsuccess = _ReorderACL(pdacl);
    else {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError(funcname, err);
        // resize if dacl too small
        unsigned int required_size = pdacl->AclSize + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", funcname);
        pdacl_padded = (ACL *)malloc(required_size);
        if (pdacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "%s: unable to allocated %d bytes", funcname, required_size);

        ZeroMemory(pdacl_padded, required_size);
        memcpy(pdacl_padded, pdacl, pdacl->AclSize);
        pdacl_padded->AclSize = (unsigned short)required_size;
        bsuccess = (*addfunc)(pdacl_padded, revision, access, psid);
        if (bsuccess) {
            bsuccess = _ReorderACL(pdacl_padded);
            if (bsuccess)
                bsuccess = This->SetACL(pdacl_padded);
        }
        else
            PyWin_SetAPIError(funcname);
    }

    if (pdacl_padded)
        free(pdacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod |PyACL|AddAccessAllowedAce|Adds an access-allowed ACE to an DACL object. The access is granted to a
// specified SID.
PyObject *PyACL::AddAccessAllowedAce(PyObject *self, PyObject *args)
{
    DWORD access, revision;
    PyObject *obSID;
    // Need to support 2 arg styles for b/w compat.
    if (PyArg_ParseTuple(args, "lO:AddAccessAllowedAce", &access, &obSID)) {
        // We worked - is old style
        // @pyparmalt1 int|access||Specifies the mask of access rights to be denied to the specified SID.
        // @pyparmalt1 <o PySID>|sid||A SID object representing a user, group, or logon account being denied access.
        revision = ACL_REVISION;
    }
    else {
        // Try new style (we use new style last so that
        // exceptions report the new style rather than old
        // @pyparm int|revision||Pre-win2k, must be ACL_REVISION, otherwise also may be ACL_REVISION_DS.
        // @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
        // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access.
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "llO:AddAccessAllowedAce", &revision, &access, &obSID))
            return NULL;
    }
    return addaceorig(addaccessallowedace, "AddAccesAllowedAce", (PyACL *)self, revision, access, obSID);

    // @comm Note that early versions of this function supported only
    // two arguments.  This has been deprecated in preference of the
    // three argument version, which reflects the win32 API and the new
    // functions in this module.
}

// @pymethod |PyACL|AddAccessDeniedAce|Adds an access-denied ACE to an ACL object. The access is denied to a specified
// SID.
PyObject *PyACL::AddAccessDeniedAce(PyObject *self, PyObject *args)
{
    DWORD access, revision;
    PyObject *obSID;
    // Need to support 2 arg styles for b/w compat.
    if (PyArg_ParseTuple(args, "lO:AddAccessDeniedAce", &access, &obSID)) {
        // We worked - is old style
        // @pyparmalt1 int|access||Specifies the mask of access rights to be denied to the specified SID.
        // @pyparmalt1 <o PySID>|sid||A SID object representing a user, group, or logon account being denied access.
        revision = ACL_REVISION;
    }
    else {
        // Try new style (we use new style last so that
        // exceptions report the new style rather than old
        // @pyparm int|revision||Pre-win2k, must be ACL_REVISION, otherwise also may be ACL_REVISION_DS.
        // @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
        // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access.
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "llO:AddAccessDeniedAce", &revision, &access, &obSID))
            return NULL;
    }
    return addaceorig(addaccessdeniedace, "AddAccesDeniedAce", (PyACL *)self, revision, access, obSID);

    // @comm Note that early versions of this function supported only
    // two arguments.  This has been deprecated in preference of the
    // three argument version, which reflects the win32 API and the new
    // functions in this module.
}

PyObject *addaceex(addaceexfunc addfunc, CHAR *funcname, PyACL *This, DWORD revision, DWORD aceflags, DWORD access,
                   PyObject *obSID)
{
    /* AddAccessAllowedAceEx and AddAccessDeniedAceEx have the same signature and semantics */
    if (addfunc == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "%s not supported by this version of Windows", funcname);
    PSID psid;
    BOOL bsuccess;
    PACL pacl = This->GetACL();
    PACL pacl_padded = NULL;
    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;

    bsuccess = (*addfunc)(pacl, revision, aceflags, access, psid);
    if (bsuccess)
        bsuccess = _ReorderACL(pacl);
    else {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError(funcname, err);
        // resize if dacl too small
        unsigned int required_size = pacl->AclSize + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", funcname);
        pacl_padded = (ACL *)malloc(required_size);
        if (pacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "%s: unable to allocated %d bytes", funcname, required_size);

        ZeroMemory(pacl_padded, required_size);
        memcpy(pacl_padded, pacl, pacl->AclSize);
        pacl_padded->AclSize = (unsigned short)required_size;
        bsuccess = (*addfunc)(pacl_padded, revision, aceflags, access, psid);
        if (bsuccess) {
            bsuccess = _ReorderACL(pacl_padded);
            if (bsuccess)
                bsuccess = This->SetACL(pacl_padded);
        }
        else
            PyWin_SetAPIError(funcname);
    }
    if (pacl_padded)
        free(pacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod |PyACL|AddAccessAllowedAceEx|Add access allowed ACE to an ACL with ACE flags (Requires Win2k or higher)
PyObject *PyACL::AddAccessAllowedAceEx(PyObject *self, PyObject *args)
{
    DWORD access, revision, aceflags;
    PyObject *obSID;
    // @pyparm int|revision||Must be at least ACL_REVISION_DS
    // @pyparm int|aceflags||Combination of ACE inheritance flags
    // (CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE, and OBJECT_INHERIT_ACE)
    // @pyparm int|access||Specifies the mask of access rights to be granted to the specified SID.
    // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being granted access.
    if (!PyArg_ParseTuple(args, "lllO:AddAccessAllowedAceEx", &revision, &aceflags, &access, &obSID))
        return NULL;
    return addaceex(addaccessallowedaceex, "AddAccessAllowedAceEx", (PyACL *)self, revision, aceflags, access, obSID);
}

// @pymethod |PyACL|AddAccessDeniedAceEx|Add access denied ACE to an ACL with ACE flags (Requires Win2k or higher)
PyObject *PyACL::AddAccessDeniedAceEx(PyObject *self, PyObject *args)
{
    DWORD access, revision, aceflags;
    PyObject *obSID;
    // @pyparm int|revision||Must be at least ACL_REVISION_DS
    // @pyparm int|aceflags||Combination of ACE inheritance flags
    // (CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE, and OBJECT_INHERIT_ACE)
    // @pyparm int|access||Specifies the mask of access rights to be denied to the specified SID.
    // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being denied access.
    if (!PyArg_ParseTuple(args, "lllO:AddAccessDeniedAceEx", &revision, &aceflags, &access, &obSID))
        return NULL;
    return addaceex(addaccessdeniedaceex, "AddAccessDeniedAceEx", (PyACL *)self, revision, aceflags, access, obSID);
}

// @pymethod |PyACL|AddMandatoryAce|Adds a mandatory integrity level ACE to a SACL
PyObject *PyACL::AddMandatoryAce(PyObject *self, PyObject *args)
{
    DWORD access, revision, aceflags;
    PyObject *obSID;
    // @pyparm int|AceRevision||ACL_REVISION or ACL_REVISION_DS
    // @pyparm int|AceFlags||Combination of ACE inheritance flags
    // (CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE, and OBJECT_INHERIT_ACE)
    // @pyparm int|MandatoryPolicy||Access policy for processes with lower integrity level, combination of
    // SYSTEM_MANDATORY_LABEL_* flags
    // @pyparm <o PySID>|LabelSid||Integrity level SID.  This can be created using CreateWellKnownSid with Win*LabelSid.
    //	<nl>Also can be constructed manually using SECURITY_MANDATORY_LABEL_AUTHORITY and a SECURITY_MANDATORY_*_RID
    if (!PyArg_ParseTuple(args, "kkkO:AddMandatoryAce", &revision, &aceflags, &access, &obSID))
        return NULL;
    return addaceex(addmandatoryace, "AddMandatoryAce", (PyACL *)self, revision, aceflags, access, obSID);
}

PyObject *addobjectace(addobjectacefunc addfunc, CHAR *funcname, PyACL *This, DWORD revision, DWORD aceflags,
                       DWORD access, PyObject *obObjectTypeGuid, PyObject *obInheritedObjectTypeGuid, PyObject *obSID)
{
    /* AddAccessAllowedObjectAce and AddAccessDeniedObjectAce have same parameters */
    BOOL bsuccess;
    PACL pdacl = This->GetACL();
    PACL pdacl_padded = NULL;
    PSID psid;
    GUID ObjectTypeGuid, InheritedObjectTypeGuid;
    GUID *pObjectTypeGuid = NULL, *pInheritedObjectTypeGuid = NULL;

    if (addfunc == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "%s not supported by this version of Windows", funcname);
    if (obObjectTypeGuid != Py_None) {
        if (!PyWinObject_AsIID(obObjectTypeGuid, &ObjectTypeGuid))
            return NULL;
        pObjectTypeGuid = &ObjectTypeGuid;
    }
    if (obInheritedObjectTypeGuid != Py_None) {
        if (!PyWinObject_AsIID(obInheritedObjectTypeGuid, &InheritedObjectTypeGuid))
            return NULL;
        pInheritedObjectTypeGuid = &InheritedObjectTypeGuid;
    }

    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;

    bsuccess = (*addfunc)(pdacl, revision, aceflags, access, pObjectTypeGuid, pInheritedObjectTypeGuid, psid);
    if (bsuccess)
        bsuccess = _ReorderACL(pdacl);
    else {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError(funcname, err);
        // resize if dacl too small
        unsigned int required_size = pdacl->AclSize + sizeof(ACCESS_ALLOWED_OBJECT_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", funcname);
        pdacl_padded = (ACL *)malloc(required_size);
        if (pdacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "%s: unable to allocated %d bytes", funcname, required_size);

        ZeroMemory(pdacl_padded, required_size);
        memcpy(pdacl_padded, pdacl, pdacl->AclSize);
        pdacl_padded->AclSize = (unsigned short)required_size;
        bsuccess =
            (*addfunc)(pdacl_padded, revision, aceflags, access, pObjectTypeGuid, pInheritedObjectTypeGuid, psid);
        if (bsuccess) {
            bsuccess = _ReorderACL(pdacl_padded);
            if (bsuccess)
                bsuccess = This->SetACL(pdacl_padded);
        }
        else
            PyWin_SetAPIError(funcname);
    }
    if (pdacl_padded)
        free(pdacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod |PyACL|AddAccessAllowedObjectAce|Adds an ACCESS_ALLOWED_OBJECT_ACE to the ACL
PyObject *PyACL::AddAccessAllowedObjectAce(PyObject *self, PyObject *args)
{
    DWORD access, revision, aceflags;
    PyObject *obSID;
    PyObject *obObjectTypeGuid, *obInheritedObjectTypeGuid;
    // @pyparm int|AceRevision||Must be at least ACL_REVISION_DS
    // @pyparm int|AceFlags||Combination of ACE inheritance flags
    // (CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE, and OBJECT_INHERIT_ACE)
    // @pyparm int|AccessMask||Specifies the mask of access rights to be granted to the specified SID
    // @pyparm <o PyIID>|ObjectTypeGuid||GUID of object type or property set to which ace applies, can be None
    // @pyparm <o PyIID>|InheritedObjectTypeGuid||GUID of object type or property that will inherit ACE, can be None
    // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account being granted access.
    if (!PyArg_ParseTuple(args, "lllOOO:AddAccessAllowedObjectAce", &revision, &aceflags, &access, &obObjectTypeGuid,
                          &obInheritedObjectTypeGuid, &obSID))
        return NULL;

    return addobjectace(addaccessallowedobjectace, "AddAccessAllowedObjectAce", (PyACL *)self, revision, aceflags,
                        access, obObjectTypeGuid, obInheritedObjectTypeGuid, obSID);
}

// @pymethod |PyACL|AddAccessDeniedObjectAce|Adds an ACCESS_DENIED_OBJECT_ACE to the ACL
PyObject *PyACL::AddAccessDeniedObjectAce(PyObject *self, PyObject *args)
{
    DWORD access, revision, aceflags;
    PyObject *obSID;
    PyObject *obObjectTypeGuid, *obInheritedObjectTypeGuid;
    // @pyparm int|AceRevision||Must be at least ACL_REVISION_DS
    // @pyparm int|AceFlags||Combination of ACE inheritance flags
    // (CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE, and OBJECT_INHERIT_ACE)
    // @pyparm int|AccessMask||Specifies the mask of access rights to be granted to the specified SID
    // @pyparm <o PyIID>|ObjectTypeGuid||GUID of object type or property set to which ace applies, can be None
    // @pyparm <o PyIID>|InheritedObjectTypeGuid||GUID of object type or property that will inherit ACE, can be None
    // @pyparm <o PySID>|sid||A SID object representing a user, group, or logon account that will be denied access.
    if (!PyArg_ParseTuple(args, "lllOOO:AddAccessDeniedObjectAce", &revision, &aceflags, &access, &obObjectTypeGuid,
                          &obInheritedObjectTypeGuid, &obSID))
        return NULL;

    return addobjectace(addaccessdeniedobjectace, "AddAccessDeniedObjectAce", (PyACL *)self, revision, aceflags, access,
                        obObjectTypeGuid, obInheritedObjectTypeGuid, obSID);
}

// @pymethod |PyACL|AddAuditAccessAce|Adds an audit ACE to a Sacl
PyObject *PyACL::AddAuditAccessAce(PyObject *self, PyObject *args)
{
    DWORD accessmask, acerevision;
    BOOL bAuditSuccess, bAuditFailure;
    PyObject *obSID;
    PSID psid;
    PACL psacl;
    PyACL *This = (PyACL *)self;
    psacl = This->GetACL();
    PACL psacl_padded = NULL;
    BOOL bsuccess;
    // @pyparm int|dwAceRevision||Revision of ACL: Pre-Win2k, must be ACL_REVISION. Win2K on up, can also be
    // ACL_REVISION_DS
    // @pyparm int|dwAccessMask||Bitmask of access types to be audited
    // @pyparm <o PySID>|sid||SID for whom system audit messages will be generated
    // @pyparm int|bAuditSuccess||Set to 1 if access success should be audited, else 0
    // @pyparm int|bAuditFailure||Set to 1 if access failure should be audited, else 0

    if (!PyArg_ParseTuple(args, "llOii:AddAuditAccessAce", &acerevision, &accessmask, &obSID, &bAuditSuccess,
                          &bAuditFailure))
        return NULL;
    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;
    bsuccess = ::AddAuditAccessAce(psacl, acerevision, accessmask, psid, bAuditSuccess, bAuditFailure);
    if (!bsuccess) {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError("AddAuditAccessAce", err);
        // resize if acl too small
        unsigned int required_size = psacl->AclSize + sizeof(SYSTEM_AUDIT_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", __FUNCTION__);
        psacl_padded = (ACL *)malloc(required_size);
        if (psacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "AddAuditAccessAce: unable to allocated %d bytes", required_size);

        ZeroMemory(psacl_padded, required_size);
        memcpy(psacl_padded, psacl, psacl->AclSize);
        psacl_padded->AclSize = (unsigned short)required_size;
        bsuccess = ::AddAuditAccessAce(psacl_padded, acerevision, accessmask, psid, bAuditSuccess, bAuditFailure);
        if (bsuccess)
            bsuccess = This->SetACL(psacl_padded);
        else
            PyWin_SetAPIError("AddAuditAccessAce");
    }

    if (psacl_padded)
        free(psacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod |PyACL|AddAuditAccessAceEx|Adds an audit ACE to an Sacl, includes ace flags
PyObject *PyACL::AddAuditAccessAceEx(PyObject *self, PyObject *args)
{
    DWORD accessmask, acerevision, aceflags;
    BOOL bAuditSuccess, bAuditFailure;
    PyObject *obSID;
    PSID psid;
    PACL psacl;
    PyACL *This = (PyACL *)self;
    psacl = This->GetACL();
    PACL psacl_padded = NULL;
    BOOL bsuccess;
    if (addauditaccessaceex == NULL)
        return PyErr_Format(PyExc_NotImplementedError, "AddAuditAccessAceEx not supported by this version of Windows");

    // @pyparm int|dwAceRevision||Revision of ACL: Must be at least ACL_REVISION_DS
    // @pyparm int|AceFlags||Combination of
    // FAILED_ACCESS_ACE_FLAG,SUCCESSFUL_ACCESS_ACE_FLAG,CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE
    // and OBJECT_INHERIT_ACE
    // @pyparm int|dwAccessMask||Bitmask of access types to be audited
    // @pyparm <o PySID>|sid||SID for whom system audit messages will be generated
    // @pyparm int|bAuditSuccess||Set to 1 if access success should be audited, else 0
    // @pyparm int|bAuditFailure||Set to 1 if access failure should be audited, else 0

    if (!PyArg_ParseTuple(args, "lllOii:AddAuditAccessAceEx", &acerevision, &aceflags, &accessmask, &obSID,
                          &bAuditSuccess, &bAuditFailure))
        return NULL;
    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;
    bsuccess = (*addauditaccessaceex)(psacl, acerevision, aceflags, accessmask, psid, bAuditSuccess, bAuditFailure);
    if (!bsuccess) {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError("AddAuditAccessAceEx", err);
        // resize if acl too small
        unsigned int required_size = psacl->AclSize + sizeof(SYSTEM_AUDIT_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", __FUNCTION__);
        psacl_padded = (ACL *)malloc(required_size);
        if (psacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "AddAuditAccessAceEx: unable to allocated %d bytes", required_size);

        ZeroMemory(psacl_padded, required_size);
        memcpy(psacl_padded, psacl, psacl->AclSize);
        psacl_padded->AclSize = (unsigned short)required_size;
        bsuccess =
            (*addauditaccessaceex)(psacl_padded, acerevision, aceflags, accessmask, psid, bAuditSuccess, bAuditFailure);
        if (bsuccess)
            bsuccess = This->SetACL(psacl_padded);
        else
            PyWin_SetAPIError("AddAuditAccessAceEx");
    }

    if (psacl_padded)
        free(psacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod |PyACL|AddAuditAccessObjectAce|Adds an audit ACE for an object type identified by GUID
PyObject *PyACL::AddAuditAccessObjectAce(PyObject *self, PyObject *args)
{
    DWORD accessmask, acerevision, aceflags;
    BOOL bAuditSuccess, bAuditFailure;
    PyObject *obSID;
    GUID ObjectTypeGuid, InheritedObjectTypeGuid;
    GUID *pObjectTypeGuid = NULL, *pInheritedObjectTypeGuid = NULL;
    PyObject *obObjectTypeGuid, *obInheritedObjectTypeGuid;
    BOOL bsuccess;
    PSID psid;
    PACL psacl;
    PyACL *This = (PyACL *)self;
    psacl = This->GetACL();
    PACL psacl_padded = NULL;
    if (addauditaccessobjectace == NULL)
        return PyErr_Format(PyExc_NotImplementedError,
                            "AddAuditAccessObjectAce not supported by this version of Windows");

    // @pyparm int|dwAceRevision||Revision of ACL: Must be at least ACL_REVISION_DS
    // @pyparm int|AceFlags||Combination of
    // FAILED_ACCESS_ACE_FLAG,SUCCESSFUL_ACCESS_ACE_FLAG,CONTAINER_INHERIT_ACE,INHERIT_ONLY_ACE,INHERITED_ACE,NO_PROPAGATE_INHERIT_ACE
    // and OBJECT_INHERIT_ACE
    // @pyparm int|dwAccessMask||Bitmask of access types to be audited
    // @pyparm <o PyIID>|ObjectTypeGuid||GUID of object type or property set to which ace applies, can be None
    // @pyparm <o PyIID>|InheritedObjectTypeGuid||GUID of object type or property that will inherit ACE, can be None
    // @pyparm <o PySID>|sid||SID for whom system audit messages will be generated
    // @pyparm int|bAuditSuccess||Set to 1 if access success should be audited, else 0
    // @pyparm int|bAuditFailure||Set to 1 if access failure should be audited, else 0

    if (!PyArg_ParseTuple(args, "lllOOOii:AddAuditAccessObjectAce", &acerevision, &aceflags, &accessmask,
                          &obObjectTypeGuid, &obInheritedObjectTypeGuid, &obSID, &bAuditSuccess, &bAuditFailure))
        return NULL;
    if (obObjectTypeGuid != Py_None) {
        if (!PyWinObject_AsIID(obObjectTypeGuid, &ObjectTypeGuid))
            return NULL;
        pObjectTypeGuid = &ObjectTypeGuid;
    }
    if (obInheritedObjectTypeGuid != Py_None) {
        if (!PyWinObject_AsIID(obInheritedObjectTypeGuid, &InheritedObjectTypeGuid))
            return NULL;
        pInheritedObjectTypeGuid = &InheritedObjectTypeGuid;
    }
    if (!PyWinObject_AsSID(obSID, &psid, FALSE))
        return NULL;

    bsuccess = (*addauditaccessobjectace)(psacl, acerevision, aceflags, accessmask, pObjectTypeGuid,
                                          pInheritedObjectTypeGuid, psid, bAuditSuccess, bAuditFailure);
    if (!bsuccess) {
        DWORD err = GetLastError();
        if (err != ERROR_ALLOTTED_SPACE_EXCEEDED)
            return PyWin_SetAPIError("AddAuditAccessObjectAce", err);
        // resize if acl too small
        unsigned int required_size = psacl->AclSize + sizeof(SYSTEM_AUDIT_OBJECT_ACE) + GetLengthSid(psid);
        // max ACL size is USHRT_MAX
        if (required_size > USHRT_MAX)
            return PyErr_Format(PyExc_OverflowError, "%s: adding ACE would put ACL over size limit", __FUNCTION__);
        psacl_padded = (ACL *)malloc(required_size);
        if (psacl_padded == NULL)
            return PyErr_Format(PyExc_MemoryError, "AddAuditAccessObjectAce: unable to allocated %d bytes",
                                required_size);

        ZeroMemory(psacl_padded, required_size);
        memcpy(psacl_padded, psacl, psacl->AclSize);
        psacl_padded->AclSize = (unsigned short)required_size;
        bsuccess = (*addauditaccessobjectace)(psacl_padded, acerevision, aceflags, accessmask, pObjectTypeGuid,
                                              pInheritedObjectTypeGuid, psid, bAuditSuccess, bAuditFailure);
        if (bsuccess)
            bsuccess = This->SetACL(psacl_padded);
        else
            PyWin_SetAPIError("AddAuditAccessObjectAce");
    }
    if (psacl_padded)
        free(psacl_padded);
    if (bsuccess) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

// @pymethod int|PyACL|GetAclSize|Returns the storage size of the ACL.
PyObject *PyACL::GetAclSize(PyObject *self, PyObject *args)
{
    PyACL *This = (PyACL *)self;
    PACL pacl;

    if (!PyArg_ParseTuple(args, ":GetAclSize"))
        return NULL;

    pacl = This->GetACL();
    return Py_BuildValue("l", pacl->AclSize);
}

// @pymethod int|PyACL|GetAclRevision|Returns revision of the ACL.
PyObject *PyACL::GetAclRevision(PyObject *self, PyObject *args)
{
    PyACL *This = (PyACL *)self;
    PACL pacl;

    if (!PyArg_ParseTuple(args, ":GetAclSize"))
        return NULL;

    pacl = This->GetACL();
    return Py_BuildValue("l", pacl->AclRevision);
}

// @pymethod int|PyACL|GetAceCount|Returns the number of ACEs in the ACL.
PyObject *PyACL::GetAceCount(PyObject *self, PyObject *args)
{
    PyACL *This = (PyACL *)self;
    PACL pacl;

    if (!PyArg_ParseTuple(args, ":GetAceCount"))
        return NULL;

    pacl = This->GetACL();
    return Py_BuildValue("l", pacl->AceCount);
}

// @pymethod tuple|PyACL|GetAce|Gets an Ace from the ACL
// @rdesc Conventional ACE's (types ACCESS_ALLOWED_ACE, ACCESS_DENIED_ACE, SYSTEM_AUDIT_ACE) are returned
// as a tuple of:
// @tupleitem 0|(int, int)|aceType, AceFlags|
// @tupleitem 1|int|Mask|
// @tupleitem 2|<o PySID>|sid|
// <nl>Object ACE's (types ACCESS_ALLOWED_OBJECT_ACE, ACCESS_DENIED_OBJECT_ACE, SYSTEM_AUDIT_OBJECT_ACE)
// are returned as a tuple:
// @tupleitem 0|(int, int)|aceType, AceFlags|
// @tupleitem 1|int|mask|
// @tupleitem 2|<o PyIID>|ObjectType|
// @tupleitem 3|<o PyIID>|InheritedObjectType|
// @tupleitem 4|<o PySID>|sid|
// <nl>For details see the API documentation.
PyObject *PyACL::GetAce(PyObject *self, PyObject *args)
{
    DWORD index;
    ACE_HEADER *pAceHeader;
    LPVOID p;
    PyACL *This = (PyACL *)self;
    // @pyparm int|index||Zero-based index of the ACE to retrieve.
    if (!PyArg_ParseTuple(args, "l:GetAce", &index))
        return NULL;
    if (!::GetAce(This->GetACL(), index, &p))
        return PyWin_SetAPIError("GetAce");
    pAceHeader = (ACE_HEADER *)p;
    switch (pAceHeader->AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:
        case ACCESS_DENIED_ACE_TYPE:
        case SYSTEM_AUDIT_ACE_TYPE:
        case SYSTEM_MANDATORY_LABEL_ACE_TYPE: {
            ACCESS_ALLOWED_ACE *pAce = (ACCESS_ALLOWED_ACE *)p;
            return Py_BuildValue("(ll)lN", pAceHeader->AceType, pAceHeader->AceFlags, pAce->Mask,
                                 PyWinObject_FromSID((PSID)(&pAce->SidStart)));
        }
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE: {
            PyObject *obSID, *obObjectType = NULL, *obInheritedObjectType = NULL;
            ACCESS_ALLOWED_OBJECT_ACE *pObjectAce = (ACCESS_ALLOWED_OBJECT_ACE *)p;

            if (!(pObjectAce->Flags & ACE_OBJECT_TYPE_PRESENT)) {
                Py_INCREF(Py_None);
                obObjectType = Py_None;
            }
            if (!(pObjectAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)) {
                Py_INCREF(Py_None);
                obInheritedObjectType = Py_None;
            }
            // Struct members float depending on presence of GUIDs !
            if ((obObjectType == Py_None) &&
                (obInheritedObjectType == Py_None))  // neither GUID present, SID will be in first GUID position
                obSID = PyWinObject_FromSID((PSID)&pObjectAce->ObjectType);
            else if ((obObjectType == NULL) && (obInheritedObjectType == NULL)) {  // both present, SID in normal place
                obObjectType = PyWinObject_FromIID(pObjectAce->ObjectType);
                obInheritedObjectType = PyWinObject_FromIID(pObjectAce->InheritedObjectType);
                obSID = PyWinObject_FromSID((PSID)&pObjectAce->SidStart);
            }
            else {  // one present in position of first GUID, SID with be in position of second GUID
                obSID = PyWinObject_FromSID((PSID)&pObjectAce->InheritedObjectType);
                if (obObjectType == NULL)
                    obObjectType = PyWinObject_FromIID(pObjectAce->ObjectType);
                else
                    obInheritedObjectType = PyWinObject_FromIID(pObjectAce->ObjectType);
            }
            return Py_BuildValue("(ll)lNNN", pAceHeader->AceType, pAceHeader->AceFlags, pObjectAce->Mask, obObjectType,
                                 obInheritedObjectType, obSID);
        }
        default:
            PyErr_Format(PyExc_NotImplementedError, "Ace type %d is not supported yet", pAceHeader->AceType);
            return NULL;
    }
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
    PyObject *ret = NULL;
    PEXPLICIT_ACCESS_W pexpl = NULL, pexpl_start = NULL;
    ACL *new_acl = NULL;
    PyACL *This = (PyACL *)self;
    PyObject *obexpl = NULL, *obexpl_list = NULL;
    Py_ssize_t expl_cnt = 0, expl_ind = 0;
    DWORD err;
    if (!PyArg_ParseTuple(args, "O:SetEntriesInAcl", &obexpl_list))
        return NULL;
    if (!PySequence_Check(obexpl_list)) {
        PyErr_SetString(PyExc_TypeError, "Parm must be a list of EXPLICIT_ACCESS dictionaries");
        return NULL;
    }
    expl_cnt = PySequence_Length(obexpl_list);
    Py_ssize_t bytes_allocated = expl_cnt * sizeof(EXPLICIT_ACCESS_W);
    pexpl_start = (PEXPLICIT_ACCESS_W)malloc(bytes_allocated);
    ZeroMemory(pexpl_start, bytes_allocated);
    if (pexpl_start == NULL) {
        PyErr_SetString(PyExc_MemoryError, "SetEntriesInAcl: unable to allocate EXPLICIT_ACCESS_W");
        goto done;
    }
    pexpl = pexpl_start;
    for (expl_ind = 0; expl_ind < expl_cnt; expl_ind++) {
        obexpl = PySequence_GetItem(obexpl_list, expl_ind);
        if (!PyWinObject_AsEXPLICIT_ACCESS(obexpl, pexpl)) {
            Py_DECREF(obexpl);
            goto done;
        }
        Py_DECREF(obexpl);
        pexpl++;
    }
    err = ::SetEntriesInAclW(PyWin_SAFE_DOWNCAST(expl_cnt, Py_ssize_t, ULONG), pexpl_start, This->GetACL(), &new_acl);
    if (err != ERROR_SUCCESS) {
        PyWin_SetAPIError("SetEntriesInAcl", err);
        goto done;
    }
    if (This->SetACL(new_acl))
        ret = Py_None;
done:
    // have to free WCHAR name from trustee structures also
    if (pexpl_start != NULL) {
        pexpl = pexpl_start;
        for (expl_ind = 0; expl_ind < expl_cnt; expl_ind++) {
            PyWinObject_FreeTRUSTEE(&pexpl->Trustee);
            pexpl++;
        }
        free(pexpl_start);
    }
    if (new_acl)
        LocalFree(new_acl);
    Py_XINCREF(ret);
    return ret;
}

// @pymethod ACCESS_MASK|PyACL|GetEffectiveRightsFromAcl|Return access rights (ACCESS_MASK) that the ACL grants to
// specified trustee
PyObject *PyACL::PyGetEffectiveRightsFromAcl(PyObject *self, PyObject *args)
{
    DWORD err = 0;
    ACCESS_MASK access_mask = 0;
    PyACL *This = (PyACL *)self;
    PyObject *ret = NULL, *obTrustee = NULL;
    TRUSTEE_W trustee;
    // @pyparm <o PyTRUSTEE>|trustee||Dictionary representing a TRUSTEE structure
    if (!PyArg_ParseTuple(args, "O:GetEffectiveRightsFromAcl", &obTrustee))
        return NULL;
    if (!PyWinObject_AsTRUSTEE(obTrustee, &trustee))
        return NULL;
    err = GetEffectiveRightsFromAclW(This->GetACL(), &trustee, &access_mask);
    if (err != ERROR_SUCCESS)
        PyWin_SetAPIError("GetEffectiveRightsFromAcl", err);
    else
        ret = Py_BuildValue("l", access_mask);
    PyWinObject_FreeTRUSTEE(&trustee);
    return ret;
}

// @pymethod (SuccessfulAuditedRights,FailedAuditRights)|PyACL|GetAuditedPermissionsFromAcl|Return types of access for
// which ACL will generate an audit event for specified trustee
PyObject *PyACL::PyGetAuditedPermissionsFromAcl(PyObject *self, PyObject *args)
{
    DWORD err = 0;
    ACCESS_MASK success_mask = 0, fail_mask = 0;
    PyACL *This = (PyACL *)self;
    ACL *pacl = This->GetACL();
    PyObject *ret = NULL, *obTrustee = NULL;
    TRUSTEE_W trustee;
    // @pyparm <o PyTRUSTEE>|trustee||Dictionary representing a TRUSTEE structure
    if (!PyArg_ParseTuple(args, "O:GetAuditedPermissionsFromAcl", &obTrustee))
        return NULL;
    if (!PyWinObject_AsTRUSTEE(obTrustee, &trustee))
        return NULL;
    err = GetAuditedPermissionsFromAclW(This->GetACL(), &trustee, &success_mask, &fail_mask);
    if (err != ERROR_SUCCESS)
        PyWin_SetAPIError("GetAuditedPermissionsFromAcl", err);
    else
        ret = Py_BuildValue("ll", success_mask, fail_mask);
    PyWinObject_FreeTRUSTEE(&trustee);
    return ret;
}

// @object PyACL|A Python object, representing a ACL structure
struct PyMethodDef PyACL::methods[] = {
    {"Initialize", PyACL::Initialize, 1},  // @pymeth Initialize|Initialize the ACL.
    {"IsValid", PyACL::IsValid, 1},        // @pymeth IsValid|Validate the ACL.
    {"AddAccessAllowedAce", PyACL::AddAccessAllowedAce,
     1},  // @pymeth AddAccessAllowedAce|Adds an access-allowed ACE to an ACL object.
    {"AddAccessAllowedAceEx", PyACL::AddAccessAllowedAceEx,
     1},  // @pymeth AddAccessAllowedAceEx|Same as AddAccessAllowedAce, with addition of ace flags
    {"AddAccessAllowedObjectAce", PyACL::AddAccessAllowedObjectAce,
     1},  // @pymeth AddAccessAllowedObjectAce|Adds an ACCESS_ALLOWED_OBJECT_ACE to the ACL
    {"AddAccessDeniedAce", PyACL::AddAccessDeniedAce,
     1},  // @pymeth AddAccessDeniedAce|Adds an access-denied ACE to an ACL object.
    {"AddAccessDeniedAceEx", PyACL::AddAccessDeniedAceEx,
     1},  // @pymeth AddAccessDeniedAceEx|Adds an access-denied ACE to an ACL object
    {"AddMandatoryAce", PyACL::AddMandatoryAce,
     1},  // @pymeth AddMandatoryAce|Adds a mandatory integrity level ACE to a SACL
    {"AddAccessDeniedObjectAce", PyACL::AddAccessDeniedObjectAce,
     1},  // @pymeth AddAccessAllowedObjectAce|Adds an ACCESS_DENIED_OBJECT_ACE to the ACL
    {"AddAuditAccessAce", PyACL::AddAuditAccessAce,
     1},  // @pymeth AddAuditAccessAce|Adds an audit entry to a system access control list (SACL)
    {"AddAuditAccessAceEx", PyACL::AddAuditAccessAceEx,
     1},  // @pymeth AddAuditAccessAceEx|Adds an audit ACE to an SACL with inheritance flags
    {"AddAuditAccessObjectAce", PyACL::AddAuditAccessObjectAce,
     1},  // @pymeth AddAuditAccessObjectAce|Adds an audit ACE for an object type identified by GUID
    {"GetAclSize", PyACL::GetAclSize, 1},          // @pymeth GetAclSize|Returns the storage size of the ACL.
    {"GetAclRevision", PyACL::GetAclRevision, 1},  // @pymeth GetAclRevision|Returns the revision nbr of the ACL.
    {"GetAceCount", PyACL::GetAceCount, 1},        // @pymeth GetAceCount|Returns the number of ACEs in the ACL.
    {"GetAce", PyACL::GetAce, 1},                  // @pymeth GetAce|Returns an ACE from the ACL.
    {"DeleteAce", PyACL::DeleteAce, 1},            // @pymeth DeleteAce|Delete an access-control entry (ACE) from an ACL
    {"GetExplicitEntriesFromAcl", PyACL::PyGetExplicitEntriesFromAcl,
     1},  // @pymeth GetExplicitEntriesFromAcl|Retrieve list of EXPLICIT_ACCESSs from the ACL
    {"SetEntriesInAcl", PyACL::PySetEntriesInAcl,
     1},  // @pymeth SetEntriesInAcl|Adds a list of EXPLICIT_ACCESSs to an ACL
    {"GetEffectiveRightsFromAcl", PyACL::PyGetEffectiveRightsFromAcl,
     1},  //@pymeth GetEffectiveRightsFromAcl|Return access rights (ACCESS_MASK) that the ACL grants to specified
          // trustee
    {"GetAuditedPermissionsFromAcl", PyACL::PyGetAuditedPermissionsFromAcl,
     1},  //@pymeth GetAuditedPermissionsFromAcl|Return types of access for which ACL will generate an audit event for
          // specified trustee
    {NULL}};

PYWINTYPES_EXPORT PyTypeObject PyACLType = {
    PYWIN_OBJECT_HEAD "PyACL",
    sizeof(PyACL),
    0,
    PyACL::deallocFunc,                       /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    PyObject_GenericGetAttr,                  /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyACL::methods,                           /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0,                                        /* tp_new */
};

PyACL::PyACL(int createBufSize, int aclrev)
{
    ob_type = &PyACLType;
    _Py_NewReference(this);
    buf = malloc(createBufSize);
    memset(buf, 0, createBufSize);
    ::InitializeAcl((ACL *)buf, createBufSize, aclrev);
}

PyACL::PyACL(PACL pacl)
{
    ob_type = &PyACLType;
    _Py_NewReference(this);
    buf = malloc(pacl->AclSize);
    memcpy(buf, (void *)pacl, pacl->AclSize);
}

PyACL::~PyACL() { free(buf); }

/*static*/ void PyACL::deallocFunc(PyObject *ob) { delete (PyACL *)ob; }
