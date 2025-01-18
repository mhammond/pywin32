//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "structmember.h"

BOOL(WINAPI *setsecuritydescriptorcontrol)
(PSECURITY_DESCRIPTOR, SECURITY_DESCRIPTOR_CONTROL, SECURITY_DESCRIPTOR_CONTROL) = NULL;

void FreeSD_DACL(PSECURITY_DESCRIPTOR psd)
{
    // free DACL associated with an absolute SECURITY_DESCRIPTOR
    PACL pdacl;
    BOOL bPresent, bDefaulted;
    if (::GetSecurityDescriptorDacl(psd, &bPresent, &pdacl, &bDefaulted))
        if (pdacl && bPresent)
            free(pdacl);
}

void FreeSD_SACL(PSECURITY_DESCRIPTOR psd)
{
    PACL psacl;
    BOOL bPresent, bDefaulted;
    if (::GetSecurityDescriptorSacl(psd, &bPresent, &psacl, &bDefaulted))
        if (psacl && bPresent)
            free(psacl);
}

void FreeSD_Owner(PSECURITY_DESCRIPTOR psd)
{
    PSID psid;
    BOOL bDefaulted;
    if (::GetSecurityDescriptorOwner(psd, &psid, &bDefaulted))
        if (psid)
            free(psid);
}

void FreeSD_Group(PSECURITY_DESCRIPTOR psd)
{
    PSID psid;
    BOOL bDefaulted;
    if (::GetSecurityDescriptorGroup(psd, &psid, &bDefaulted))
        if (psid)
            free(psid);
}

void FreeAbsoluteSD(PSECURITY_DESCRIPTOR psd)
{
    FreeSD_DACL(psd);
    FreeSD_SACL(psd);
    FreeSD_Owner(psd);
    FreeSD_Group(psd);
    free(psd);
}

DWORD GetAclSize(PACL pacl)
{
    // docs say not to use acl members directly, but pacl.AclSize would be sooooo much cleaner
    DWORD retsize;
    ACL_SIZE_INFORMATION aclsize;
    if (!GetAclInformation(pacl, &aclsize, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
        return 0;
    retsize = aclsize.AclBytesFree + aclsize.AclBytesInUse;
    return retsize;
};

BOOL _IsSelfRelative(PSECURITY_DESCRIPTOR psd)
{
    // check if SD is relative or absolute
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD revision;
    if (!::GetSecurityDescriptorControl(psd, &sdc, &revision)) {
        return NULL;
    }
    if (sdc & SE_SELF_RELATIVE)
        return TRUE;
    return FALSE;
}

// @pymethod <o PySECURITY_DESCRIPTOR>|pywintypes|SECURITY_DESCRIPTOR|Creates a new SECURITY_DESCRIPTOR object
PyObject *PyWinMethod_NewSECURITY_DESCRIPTOR(PyObject *self, PyObject *args)
{
    Py_ssize_t descriptor_len = SECURITY_DESCRIPTOR_MIN_LENGTH;
    if (PyArg_ParseTuple(args, "|l:SECURITY_DESCRIPTOR", &descriptor_len)) {
        PyObject *ret = new PySECURITY_DESCRIPTOR(descriptor_len);
        if (((PySECURITY_DESCRIPTOR *)ret)->GetSD() == NULL) {
            if (!PyErr_Occurred())
                PyErr_SetString(PyExc_NotImplementedError, "Security descriptors are not supported on this platform");
            Py_DECREF(ret);
            ret = NULL;
        }
        return ret;
    }

    PyErr_Clear();
    PyObject *obsd = NULL;
    // @pyparmalt1 buffer|data||A buffer (eg, a string) with the raw bytes for the security descriptor.
    if (!PyArg_ParseTuple(args, "O:SECURITY_DESCRIPTOR", &obsd))
        return NULL;

    PyWinBufferView pybuf(obsd);
    if (!pybuf.ok())
        return NULL;
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)pybuf.ptr();

    if (!IsValidSecurityDescriptor(psd)) {
        PyErr_SetString(PyExc_ValueError, "Data is not a valid security descriptor");
        return NULL;
    }
    if (!_IsSelfRelative(psd)) {
        PyErr_SetString(PyExc_ValueError, "Security descriptor created from a buffer must be self relative");
        return NULL;
    }
    return new PySECURITY_DESCRIPTOR(psd);
}

BOOL PyWinObject_AsSECURITY_DESCRIPTOR(PyObject *ob, PSECURITY_DESCRIPTOR *ppSECURITY_DESCRIPTOR,
                                       BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppSECURITY_DESCRIPTOR = NULL;
    }
    else if (!PySECURITY_DESCRIPTOR_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PySECURITY_DESCRIPTOR object");
        return FALSE;
    }
    else {
        *ppSECURITY_DESCRIPTOR = ((PySECURITY_DESCRIPTOR *)ob)->GetSD();
    }
    return TRUE;
}

PyObject *PyWinObject_FromSECURITY_DESCRIPTOR(PSECURITY_DESCRIPTOR psd)
{
    if (psd == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return new PySECURITY_DESCRIPTOR(psd);
}

// @pymethod |PySECURITY_DESCRIPTOR|IsSelfRelative|Returns 1 if security descriptor is self relative, 0 if absolute
PyObject *PySECURITY_DESCRIPTOR::IsSelfRelative(PyObject *self, PyObject *args)
// should remove this, all SD's stored in Python objects are relative now
{
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd = This->GetSD();
    return Py_BuildValue("i", _IsSelfRelative(psd));
}

BOOL _MakeSelfRelativeSD(PSECURITY_DESCRIPTOR psd_absolute, PSECURITY_DESCRIPTOR *ppsd_relative)
{
    if (!IsValidSecurityDescriptor(psd_absolute)) {
        PyErr_SetString(PyExc_ValueError, "Invalid Security descriptor");
        return FALSE;
    }
    DWORD buflen = GetSecurityDescriptorLength(psd_absolute);
    DWORD orig_buflen = buflen;

    *ppsd_relative = malloc(buflen);
    if (*ppsd_relative == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen);
        return FALSE;
    }
    if (MakeSelfRelativeSD(psd_absolute, *ppsd_relative, &buflen))
        return TRUE;
    free(*ppsd_relative);
    // if function fails because buffer is too small, required len is returned
    if (!(buflen > orig_buflen)) {
        PyWin_SetAPIError("MakeSelfRelativeSD");
        return FALSE;
    }

    *ppsd_relative = malloc(buflen);
    if (*ppsd_relative == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen);
        return FALSE;
    }
    if (MakeSelfRelativeSD(psd_absolute, *ppsd_relative, &buflen))
        return TRUE;
    free(*ppsd_relative);
    *ppsd_relative = NULL;
    PyWin_SetAPIError("MakeSelfRelativeSD");
    return FALSE;
}

BOOL _MakeAbsoluteSD(PSECURITY_DESCRIPTOR psd_relative, PSECURITY_DESCRIPTOR *ppsd_absolute)
{
    PSECURITY_DESCRIPTOR psd_absolute = NULL;
    PACL pdacl = NULL;
    PACL psacl = NULL;
    PSID powner = NULL;
    PSID pgroup = NULL;
    DWORD sdsize = SECURITY_DESCRIPTOR_MIN_LENGTH;
    DWORD origsdsize = SECURITY_DESCRIPTOR_MIN_LENGTH;
    DWORD daclsize = 0;
    DWORD saclsize = 0;
    DWORD ownersize = 0;
    DWORD groupsize = 0;
    BOOL resize = FALSE;

    psd_absolute = malloc(sdsize);
    if (psd_absolute == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", sdsize);
        goto error_exit;
    }
    ZeroMemory(psd_absolute, sdsize);

    if (MakeAbsoluteSD(psd_relative, psd_absolute, &sdsize, pdacl, &daclsize, psacl, &saclsize, powner, &ownersize,
                       pgroup, &groupsize)) {
        *ppsd_absolute = psd_absolute;
        return TRUE;
    }
    if (sdsize > origsdsize) {
        resize = TRUE;
        free(psd_absolute);
        psd_absolute = malloc(sdsize);
        if (psd_absolute == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", sdsize);
            goto error_exit;
        }
        ZeroMemory(psd_absolute, sdsize);
    }
    if (daclsize > 0) {
        resize = TRUE;
        pdacl = (ACL *)malloc(daclsize);
        if (pdacl == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", daclsize);
            goto error_exit;
        }
    }
    if (saclsize > 0) {
        resize = TRUE;
        psacl = (ACL *)malloc(saclsize);
        if (psacl == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", saclsize);
            goto error_exit;
        }
    }
    if (ownersize > 0) {
        resize = TRUE;
        powner = (SID *)malloc(ownersize);
        if (powner == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", ownersize);
            goto error_exit;
        }
    }
    if (groupsize > 0) {
        resize = TRUE;
        pgroup = (SID *)malloc(groupsize);
        if (pgroup == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", groupsize);
            goto error_exit;
        }
    }
    if (resize && MakeAbsoluteSD(psd_relative, psd_absolute, &sdsize, pdacl, &daclsize, psacl, &saclsize, powner,
                                 &ownersize, pgroup, &groupsize)) {
        *ppsd_absolute = psd_absolute;
        return TRUE;
    }
    PyWin_SetAPIError("MakeAbsoluteSD");

error_exit:
    *ppsd_absolute = NULL;
    // *Don't* use FreeAbsoluteSD since function may exit without the sd having been constructed yet
    if (psd_absolute != NULL)
        free(psd_absolute);
    if (pdacl != NULL)
        free(pdacl);
    if (psacl != NULL)
        free(psacl);
    if (powner != NULL)
        free(powner);
    if (pgroup != NULL)
        free(pgroup);
    return FALSE;
}

BOOL PySECURITY_DESCRIPTOR::SetSD(PSECURITY_DESCRIPTOR psd)
{
    // replace security descriptor in object, always in relative format
    if (this->m_psd)
        free(this->m_psd);
    if (_IsSelfRelative(psd)) {
        DWORD sdsize = GetSecurityDescriptorLength(psd);
        this->m_psd = malloc(sdsize);
        memcpy(this->m_psd, psd, sdsize);
        return TRUE;
    }
    else {
        // should be last-ditch fallback, everything should pass SD already in self-relative form
        if (!_MakeSelfRelativeSD(psd, &(this->m_psd)))
            return FALSE;
        return TRUE;
    }
}

// @pymethod |PySECURITY_DESCRIPTOR|Initialize|Initialize the SD.
PyObject *PySECURITY_DESCRIPTOR::Initialize(PyObject *self, PyObject *args)
{
    PyObject *ret = NULL;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd = This->GetSD();
    PSECURITY_DESCRIPTOR psd_relative = NULL;
    if (!PyArg_ParseTuple(args, ":Initialize"))
        return NULL;
    if (!::InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
        return PyWin_SetAPIError("InitializeSecurityDescriptor");
    // above always returns in absolute format, change back to self-relative
    if (_MakeSelfRelativeSD(psd, &psd_relative))
        if (This->SetSD(psd_relative))
            ret = Py_None;
    if (psd_relative)
        free(psd_relative);
    Py_XINCREF(ret);
    return ret;
}

// @pymethod |PySECURITY_DESCRIPTOR|SetSecurityDescriptorDacl|Replaces DACL in a security descriptor.
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl(PyObject *self, PyObject *args)
{
    // ???????????????? make this one function with Set SACL ??????????
    PyObject *ret = NULL;
    PyObject *obDACL;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd = NULL, psd_relative = NULL;
    BOOL bDaclPresent, bDaclDefaulted;
    PACL pdacl;
    if (!PyArg_ParseTuple(args, "iOi:SetSecurityDescriptorDacl", &bDaclPresent, &obDACL, &bDaclDefaulted))
        return NULL;
    if (!PyWinObject_AsACL(obDACL, &pdacl, TRUE))
        return NULL;
    // @pyparm int|bDaclPresent||A flag indicating if the SE_DACL_PRESENT flag should be set.
    // @pyparm <o PyACL>|DACL||The DACL to set.  If None, a NULL ACL will be created allowing world access.
    // @pyparm int|bDaclDefaulted||A flag indicating if the SE_DACL_DEFAULTED flag should be set.

    PSECURITY_DESCRIPTOR obpsd = This->GetSD();
    // will alway be in relative format in python object, convert to absolute
    if (!_MakeAbsoluteSD(obpsd, &psd))
        return NULL;

    FreeSD_DACL(psd);
    if (!::SetSecurityDescriptorDacl(psd, bDaclPresent, pdacl, bDaclDefaulted)) {
        PyWin_SetAPIError("SetSecurityDescriptorDacl");
        goto done;
    }
    // replace security descriptor (in relative format) in object
    if (!_MakeSelfRelativeSD(psd, &psd_relative))
        goto done;
    if (This->SetSD(psd_relative))
        ret = Py_None;

done:
    // don't free the DACL, should still be the one pointed to by PyACL passed in
    if (psd != NULL) {
        FreeSD_Owner(psd);
        FreeSD_Group(psd);
        FreeSD_SACL(psd);
        free(psd);
    }

    if (psd_relative != NULL)
        free(psd_relative);
    Py_XINCREF(ret);
    return ret;
}

// @pymethod |PySECURITY_DESCRIPTOR|SetSecurityDescriptorSacl|Replaces system access control list (SACL) in the security
// descriptor.
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorSacl(PyObject *self, PyObject *args)
{
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd = NULL, psd_relative = NULL;
    PyObject *ret = NULL;
    PyObject *obSACL;
    PACL psacl;
    BOOL bSaclPresent, bSaclDefaulted;
    // @pyparm int|bSaclPresent||A flag indicating if SACL is to be used. If false, last 2 parms are ignored.
    // @pyparm <o PyACL>|SACL||The SACL to set in the security descriptor
    // @pyparm int|bSaclDefaulted||Flag, set to false if user has specifically set the SACL.
    if (!PyArg_ParseTuple(args, "iOi:SetSacl", &bSaclPresent, &obSACL, &bSaclDefaulted))
        return NULL;
    if (!PyWinObject_AsACL(obSACL, &psacl, TRUE))
        return NULL;

    PSECURITY_DESCRIPTOR obpsd = This->GetSD();
    if (!_MakeAbsoluteSD(obpsd, &psd))
        goto done;

    // free existing Sacl, allocated by _MakeAbsoluteSD above
    FreeSD_SACL(psd);
    if (!::SetSecurityDescriptorSacl(psd, bSaclPresent, psacl, bSaclDefaulted)) {
        PyWin_SetAPIError("SetSecurityDescriptorSacl");
        goto done;
    }
    // transform sd back into self-relative
    if (!_MakeSelfRelativeSD(psd, &psd_relative))
        goto done;
    // replace security descriptor in PyObject
    if (This->SetSD(psd_relative))
        ret = Py_None;

done:
    // don't free SACL, PyACL passed in as parm points to same place
    if (psd != NULL) {
        FreeSD_Owner(psd);
        FreeSD_Group(psd);
        FreeSD_DACL(psd);
        free(psd);
    }
    if (psd_relative != NULL)
        free(psd_relative);

    Py_XINCREF(ret);
    return ret;
}

// @pymethod <o PySID>|PySECURITY_DESCRIPTOR|GetSecurityDescriptorOwner|Return the owner of the security descriptor.
PyObject *PySECURITY_DESCRIPTOR::GetSecurityDescriptorOwner(PyObject *self, PyObject *args)
{
    PSID psd_sid;
    BOOL OwnerDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    DWORD sidsize = 0;
    PyObject *obNewSid = NULL;

    if (!PyArg_ParseTuple(args, ":GetSecurityDescriptorOwner"))
        return NULL;

    // get SID from SD
    if (!::GetSecurityDescriptorOwner(This->m_psd, &psd_sid, &OwnerDefaulted))
        return PyWin_SetAPIError("GetSecurityDescriptorOwner");

    if (psd_sid == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    // create and return pySID object
    return new PySID(psd_sid);
}

// @pymethod |PySECURITY_DESCRIPTOR|SetSecurityDescriptorOwner|Set owner SID.
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorOwner(PyObject *self, PyObject *args)
{
    // @pyparm <o PySID>|sid||The sid to be set as owner in the security descriptor.
    // @pyparm int|bOwnerDefaulted||Normally set to false since this explicitly set the owner.
    BOOL bOwnerDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSID NewOwnerSid = NULL;
    PyObject *obNewOwnerSid = NULL;
    PyObject *ret = NULL;
    if (!PyArg_ParseTuple(args, "Oi:SetSecurityDescriptorOwner", &obNewOwnerSid, &bOwnerDefaulted))
        return NULL;
    if (!PyWinObject_AsSID(obNewOwnerSid, &NewOwnerSid, TRUE))
        return NULL;
    PSECURITY_DESCRIPTOR psd = NULL, psd_relative = NULL;
    PSECURITY_DESCRIPTOR obpsd = This->GetSD();
    if (!_MakeAbsoluteSD(obpsd, &psd))
        goto done;
    // free old owner, allocated by above
    FreeSD_Owner(psd);
    if (!::SetSecurityDescriptorOwner(psd, NewOwnerSid, bOwnerDefaulted)) {
        PyWin_SetAPIError("SetSecurityDescriptorOwner");
        goto done;
    }
    if (!_MakeSelfRelativeSD(psd, &psd_relative))
        goto done;
    if (This->SetSD(psd_relative))
        ret = Py_None;

done:
    if (psd != NULL) {
        FreeSD_DACL(psd);
        FreeSD_SACL(psd);
        FreeSD_Group(psd);
        // *Don't* free owner memory, will still be referenced by passed in PySID
        free(psd);
    }
    if (psd_relative != NULL)
        free(psd_relative);
    Py_XINCREF(ret);
    return ret;
}

// @pymethod int|PySECURITY_DESCRIPTOR|SetSecurityDescriptorGroup|Set group SID.
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorGroup(PyObject *self, PyObject *args)
{
    // @pyparm <o PySID>|sid||The group sid to be set in the security descriptor.
    // @pyparm int|bOwnerDefaulted||Normally set to false since this explicitly set the owner.
    BOOL bGroupDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd = NULL;
    PSECURITY_DESCRIPTOR psd_relative = NULL;
    PSID NewGroupSid = NULL;
    PyObject *obNewGroupSid = NULL;
    PyObject *ret = NULL;

    if (!PyArg_ParseTuple(args, "Oi:SetSecurityDescriptorOwner", &obNewGroupSid, &bGroupDefaulted))
        return NULL;
    if (!PyWinObject_AsSID(obNewGroupSid, &NewGroupSid, TRUE))
        return NULL;
    if (NewGroupSid && !IsValidSid(NewGroupSid))
        return PyWin_SetAPIError("SetSecurityDescriptorGroup - invalid sid");
    if (!_MakeAbsoluteSD(This->GetSD(), &psd))
        goto done;
    FreeSD_Group(psd);
    if (!::SetSecurityDescriptorGroup(psd, NewGroupSid, bGroupDefaulted)) {
        PyWin_SetAPIError("SetSecurityDescriptorGroup");
        goto done;
    }
    if (!_MakeSelfRelativeSD(psd, &psd_relative))
        goto done;
    if (This->SetSD(psd_relative))
        ret = Py_None;

done:
    if (psd != NULL) {
        FreeSD_DACL(psd);
        FreeSD_SACL(psd);
        FreeSD_Owner(psd);
        free(psd);
        // *Don't* free group, will still be owned by passed in PySID
    }
    if (psd_relative != NULL)
        free(psd_relative);
    Py_XINCREF(ret);
    return ret;
}

// @pymethod <o PySID>|PySECURITY_DESCRIPTOR|GetSecurityDescriptorGroup|Return the group owning the security descriptor.
// SID is returned.
PyObject *PySECURITY_DESCRIPTOR::GetSecurityDescriptorGroup(PyObject *self, PyObject *args)
{
    PSID psd_sid;
    BOOL OwnerDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd;

    if (!PyArg_ParseTuple(args, ":GetSecurityDescriptorGroup"))
        return NULL;

    psd = This->GetSD();
    if (!IsValidSecurityDescriptor(psd))
        return PyWin_SetAPIError("GetSecurityDescriptorGroup - invalid sd");

    // get SID from SD
    if (!::GetSecurityDescriptorGroup(psd, &psd_sid, &OwnerDefaulted))
        return PyWin_SetAPIError("GetSecurityDescriptorGroup");

    if (psd_sid == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    // create and return pySID object
    return new PySID(psd_sid);
}

// @pymethod <o PyACL>|PySECURITY_DESCRIPTOR|GetSecurityDescriptorDacl|Return the discretionary ACL of the security
// descriptor.
PyObject *PySECURITY_DESCRIPTOR::GetSecurityDescriptorDacl(PyObject *self, PyObject *args)
{
    PACL pdacl;
    BOOL bDaclPresent, bDaclDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd;

    if (!PyArg_ParseTuple(args, ":GetSecurityDescriptorDacl"))
        return NULL;

    psd = This->GetSD();
    if (!IsValidSecurityDescriptor(psd))
        return PyWin_SetAPIError("SetSecurityDescriptorGroup - invalid sd");

    // get Dacl from SD
    if (!::GetSecurityDescriptorDacl(psd, &bDaclPresent, &pdacl, &bDaclDefaulted))
        return PyWin_SetAPIError("GetSecurityDescriptorDacl");

    if (!bDaclPresent || pdacl == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return new PyACL(pdacl);
}

// @pymethod <o PyACL>|PySECURITY_DESCRIPTOR|GetSecurityDescriptorSacl|Return system access control list (SACL) of SD
PyObject *PySECURITY_DESCRIPTOR::GetSecurityDescriptorSacl(PyObject *self, PyObject *args)
{
    PACL psacl;
    BOOL bSaclPresent, bSaclDefaulted;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd;

    if (!PyArg_ParseTuple(args, ":GetSecurityDescriptorSacl"))
        return NULL;

    psd = This->GetSD();
    if (!IsValidSecurityDescriptor(psd))
        return PyWin_SetAPIError("GetSecurityDescriptorSacl - invalid sd");

    // get Sacl from SD
    if (!::GetSecurityDescriptorSacl(psd, &bSaclPresent, &psacl, &bSaclDefaulted))
        return PyWin_SetAPIError("GetSecurityDescriptorSacl");

    if (!bSaclPresent || psacl == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return new PyACL(psacl);
}

// @pymethod (int,int)|PySECURITY_DESCRIPTOR|GetSecurityDescriptorControl|Returns tuple of Control bit flags and
// revision of SD.
PyObject *PySECURITY_DESCRIPTOR::GetSecurityDescriptorControl(PyObject *self, PyObject *args)
{
    SECURITY_DESCRIPTOR_CONTROL Control = NULL;
    DWORD dwRevision = NULL;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd;

    if (!PyArg_ParseTuple(args, ":GetSecurityDescriptorControl"))
        return NULL;

    psd = This->GetSD();
    if (!IsValidSecurityDescriptor(psd))
        return PyWin_SetAPIError("GetSecurityDescriptorControl - invalid sd");
    if (!::GetSecurityDescriptorControl(psd, &Control, &dwRevision))
        return PyWin_SetAPIError("GetSecurityDescriptorControl");
    return Py_BuildValue("(ii)", Control, dwRevision);
}

// @pymethod |PySECURITY_DESCRIPTOR|SetSecurityDescriptorControl|Sets the control bit flags related to inheritance for a
// security descriptor
// @comm Only exists on Windows 2000 or later
PyObject *PySECURITY_DESCRIPTOR::SetSecurityDescriptorControl(PyObject *self, PyObject *args)
{
    SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest, ControlBitsToSet;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    PSECURITY_DESCRIPTOR psd;
    if (setsecuritydescriptorcontrol == NULL) {
        PyErr_SetString(PyExc_NotImplementedError, "SetSecurityDescriptorControl does not exist on this platform");
        return NULL;
    }
    // @pyparm int|ControlBitsOfInterest||Bitmask of flags to be modified
    // @pyparm int|ControlBitsToSet||Bitmask containing flag values to set
    if (!PyArg_ParseTuple(args, "ll:SetSecurityDescriptorControl", &ControlBitsOfInterest, &ControlBitsToSet))
        return NULL;
    psd = This->GetSD();
    if (!(*setsecuritydescriptorcontrol)(psd, ControlBitsOfInterest, ControlBitsToSet))
        return PyWin_SetAPIError("SetSecurityDescriptorControl");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PySECURITY_DESCRIPTOR|IsValid|Determines if the security descriptor is valid.
PyObject *PySECURITY_DESCRIPTOR::IsValid(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":IsValid"))
        return NULL;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    return PyLong_FromLong(IsValidSecurityDescriptor(This->m_psd));
}

// @pymethod |PySECURITY_DESCRIPTOR|GetLength|return length of security descriptor (GetSecurityDescriptorLenght).
PyObject *PySECURITY_DESCRIPTOR::GetLength(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetLength"))
        return NULL;
    PySECURITY_DESCRIPTOR *This = (PySECURITY_DESCRIPTOR *)self;
    return PyLong_FromLong(GetSecurityDescriptorLength(This->m_psd));
}

// @object PySECURITY_DESCRIPTOR|A Python object, representing a SECURITY_DESCRIPTOR structure
struct PyMethodDef PySECURITY_DESCRIPTOR::methods[] = {
    {"Initialize", PySECURITY_DESCRIPTOR::Initialize, 1},  // @pymeth Initialize|Initializes the object.
    {"GetSecurityDescriptorOwner", PySECURITY_DESCRIPTOR::GetSecurityDescriptorOwner,
     1},  // @pymeth GetSecurityDescriptorOwner|Return the owner of the security descriptor. SID is returned.
    {"GetSecurityDescriptorGroup", PySECURITY_DESCRIPTOR::GetSecurityDescriptorGroup,
     1},  // @pymeth GetSecurityDescriptorOwner|Return the group owning the security descriptor. SID is returned.
    {"GetSecurityDescriptorDacl", PySECURITY_DESCRIPTOR::GetSecurityDescriptorDacl,
     1},  // @pymeth GetSecurityDescriptorDacl|Return the discretionary ACL of the security descriptor.
    {"GetSecurityDescriptorSacl", PySECURITY_DESCRIPTOR::GetSecurityDescriptorSacl,
     1},  // @pymeth GetSecurityDescriptorSacl|Return the system ACL of the security descriptor.
    {"GetSecurityDescriptorControl", PySECURITY_DESCRIPTOR::GetSecurityDescriptorControl,
     1},  // @pymeth GetSecurityDescriptorControl|Returns the control bit flags and revistion of the SD
    {"SetSecurityDescriptorOwner", PySECURITY_DESCRIPTOR::SetSecurityDescriptorOwner,
     1},  // @pymeth SetSecurityDescriptorOwner|Set the owner of the security descriptor. Returns non-zero on success.
    {"SetSecurityDescriptorGroup", PySECURITY_DESCRIPTOR::SetSecurityDescriptorGroup,
     1},  // @pymeth SetSecurityDescriptorGroup|Set the primary group of the security descriptor. Returns non-zero on
          // success.
    {"SetDacl", PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl,
     1},  // @pymeth SetDacl|Sets information in a discretionary access-control list.
    {"SetSecurityDescriptorDacl", PySECURITY_DESCRIPTOR::SetSecurityDescriptorDacl, 1},
    {"SetSecurityDescriptorSacl", PySECURITY_DESCRIPTOR::SetSecurityDescriptorSacl,
     1},  //@pymeth SetSecurityDescriptorSacl|Sets the system access control list in the security descriptor
    {"IsValid", PySECURITY_DESCRIPTOR::IsValid,
     1},  // @pymeth IsValid|Determine if security descriptor is valid (IsValidSecurityDescriptor)
    {"GetLength", PySECURITY_DESCRIPTOR::GetLength,
     1},  // @pymeth GetLength|Return length of security descriptor (GetSecurityDescriptorLength)
    {"IsSelfRelative", PySECURITY_DESCRIPTOR::IsSelfRelative,
     1},  // @pymeth IsSelfRelative|Returns true if SD is self-relative, false if absolute
    {"SetSecurityDescriptorControl", PySECURITY_DESCRIPTOR::SetSecurityDescriptorControl,
     1},  // @pymeth SetSecurityDescriptorControl|Sets control bitmask of a security descriptor
    {NULL}};

// Buffer interface in Python 3.0 has changed
/*static*/ int PySECURITY_DESCRIPTOR::getbufferinfo(PyObject *self, Py_buffer *view, int flags)
{
    PySECURITY_DESCRIPTOR *pysd = (PySECURITY_DESCRIPTOR *)self;
    return PyBuffer_FillInfo(view, self, pysd->m_psd, GetSecurityDescriptorLength(pysd->m_psd), 1, flags);
}

static PyBufferProcs PySECURITY_DESCRIPTOR_as_buffer = {
    PySECURITY_DESCRIPTOR::getbufferinfo,
    NULL  // Don't need to release any memory from Py_buffer struct
};

PYWINTYPES_EXPORT PyTypeObject PySECURITY_DESCRIPTORType = {
    PYWIN_OBJECT_HEAD "PySECURITY_DESCRIPTOR", sizeof(PySECURITY_DESCRIPTOR), 0,
    PySECURITY_DESCRIPTOR::deallocFunc, /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    PyObject_GenericSetAttr,            /* tp_setattro */
    // @comm Note the PySECURITY_DESCRIPTOR object supports the buffer interface.  Thus buffer(sd) can be used to obtain
    // the raw bytes.
    &PySECURITY_DESCRIPTOR_as_buffer,         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PySECURITY_DESCRIPTOR::methods,           /* tp_methods */
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

PySECURITY_DESCRIPTOR::PySECURITY_DESCRIPTOR(Py_ssize_t cb /*= 0*/)
{
    ob_type = &PySECURITY_DESCRIPTORType;
    _Py_NewReference(this);
    cb = max(cb, SECURITY_DESCRIPTOR_MIN_LENGTH);
    PSECURITY_DESCRIPTOR psd = malloc(cb);
    this->m_psd = NULL;
    if (::InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
        this->SetSD(psd);
    free(psd);
}

PySECURITY_DESCRIPTOR::PySECURITY_DESCRIPTOR(PSECURITY_DESCRIPTOR psd)
{
    ob_type = &PySECURITY_DESCRIPTORType;
    _Py_NewReference(this);
    this->m_psd = NULL;
    this->SetSD(psd);
}

PySECURITY_DESCRIPTOR::~PySECURITY_DESCRIPTOR(void)
{
    if (m_psd)
        free(m_psd);
}

/*static*/ void PySECURITY_DESCRIPTOR::deallocFunc(PyObject *ob) { delete (PySECURITY_DESCRIPTOR *)ob; }
