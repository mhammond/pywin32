// @doc - This file contains autoduck documentation

#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyGSecurityInformation.h"

// ---------------------------------------------------
//
// Gateway Implementation

void PyGSecurityInformation::FreeObjectInfo()
{
    PyWinObject_FreeWCHAR(ObjectInfo.pszObjectName);
    PyWinObject_FreeWCHAR(ObjectInfo.pszPageTitle);
    PyWinObject_FreeWCHAR(ObjectInfo.pszServerName);
    ZeroMemory(&ObjectInfo, sizeof(ObjectInfo));
}

void PyGSecurityInformation::FreeAccessRights(void)
{
    if (AccessRights != NULL) {
        for (ULONG i = 0; i < cAccessRights; i++) {
            if (AccessRights[i].pguid != NULL)
                free((VOID *)AccessRights[i].pguid);
            PyWinObject_FreeWCHAR((WCHAR *)AccessRights[i].pszName);
        }
        free(AccessRights);
    }
    AccessRights = NULL;
    cAccessRights = 0;
}

void PyGSecurityInformation::FreeInheritTypes(void)
{
    if (InheritTypes != NULL) {
        for (ULONG i = 0; i < cInheritTypes; i++) {
            if (InheritTypes[i].pguid != NULL)
                free((VOID *)InheritTypes[i].pguid);
            PyWinObject_FreeWCHAR((WCHAR *)InheritTypes[i].pszName);
        }
        free(InheritTypes);
    }
    InheritTypes = NULL;
    cInheritTypes = 0;
}

PyGSecurityInformation::~PyGSecurityInformation(void)
{
    FreeObjectInfo();
    FreeAccessRights();
    FreeInheritTypes();
}

// @object SI_OBJECT_INFO|Six-tuple representing SI_OBJECT_INFO struct
// @tupleitem 0|int|Flags|Combination of ntsecuritycon.SI_* flags specifying options
// @tupleitem 1|<o PyHANDLE>|hInstance|Handle to a module containing string resources (not supported yet, use 0)
// @tupleitem 2|<o PyUNICODE>|ServerName|Name of authenticating server if not local machine
// @tupleitem 3|<o PyUNICODE>|ObjectName|Name of object whose security will be displayed
// @tupleitem 4|<o PyUNICODE>|PageTitle|Title to be used for basic propery sheet (SI_PAGE_TITLE must be passed in Flags)
// @tupleitem 5|<o PyIID>|ObjectType|GUID identifying the type of object, usually IID_NULL

// @pymethod <o SI_OBJECT_INFO>|PyGSecurityInformation|GetObjectInformation|Returns information identifying the object
// whose security is to be editted, and which pages are to appear in the property sheet
// @rdesc Your implementation of this method should return a <o SI_OBJECT_INFO> tuple
// @comm Due to peculiarities of the underlying system calls, this method will only be called once,
// and subsequent calls will return the information obtained on the first call.  As a consequence, a new
// instance of the interface will need to be created for each object whose security is to be displayed.
STDMETHODIMP PyGSecurityInformation::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    PY_GATEWAY_METHOD;
    HRESULT hr;
    PyObject *result = NULL, *obhInstance, *obServerName, *obObjectName, *obPageTitle, *obObjectType;
    /* ??? This method will be called multiple times (twice, from observation), but the data from
            the first call apparently has to remain valid and constant. Why is it called multiple
            times if you can't change the returned data ???
            Only call the python gateway method once, and pass the same data back subsequently.
    */
    // FreeObjectInfo();
    // If string members of SI_OBJECT_INFO are freed after first call, page titles appear corrupted.
    // Usually only happens in debug mode, maybe some kind of race condition ?
    if (ObjectInfoAcquired)
        hr = S_OK;
    else {
        hr = InvokeViaPolicy("GetObjectInformation", &result, NULL);
        if ((!FAILED(hr)) &&
            PyArg_ParseTuple(result, "kOOOOO", &ObjectInfo.dwFlags, &obhInstance, &obServerName, &obObjectName,
                             &obPageTitle, &obObjectType) &&
            PyWinObject_AsHANDLE(obhInstance, (PHANDLE)&ObjectInfo.hInstance) &&
            PyWinObject_AsWCHAR(obServerName, &ObjectInfo.pszServerName, TRUE) &&
            PyWinObject_AsWCHAR(obObjectName, &ObjectInfo.pszObjectName, FALSE) &&
            PyWinObject_AsWCHAR(obPageTitle, &ObjectInfo.pszPageTitle, TRUE) &&
            PyWinObject_AsIID(obObjectType, &ObjectInfo.guidObjectType))
            hr = S_OK;
        else
            hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetObjectInformation");
    }
    ObjectInfoAcquired = TRUE;
    *pObjectInfo = ObjectInfo;
    Py_XDECREF(result);
    return hr;
}

// @pymethod <o PySECURITY_DESCRIPTOR>|PyGSecurityInformation|GetSecurity|Retrieves the object's current security
// settings
// @pyparm int|RequestedInformation||Combination of SECURITY_INFORMATION flags indicating which components of the
// object's security descriptor you should return
// @pyparm bool|Default||If true, return a default security descriptor rather than current security.  (invoked when
// 'Reset' button is clicked)
STDMETHODIMP PyGSecurityInformation::GetSecurity(SECURITY_INFORMATION RequestedInformation,
                                                 PSECURITY_DESCRIPTOR *ppSecurityDescriptor, BOOL fDefault)
{
    PY_GATEWAY_METHOD;
    PyObject *result = NULL;
    PSECURITY_DESCRIPTOR psd;

    HRESULT hr = InvokeViaPolicy("GetSecurity", &result, "kk", RequestedInformation, fDefault);
    if (!FAILED(hr) && PyWinObject_AsSECURITY_DESCRIPTOR(result, &psd, FALSE)) {
        // docs say system will free sd with LocalFree, so copy the returned PySECURITY_DESCRIPTOR
        DWORD buflen = GetSecurityDescriptorLength(psd);
        *ppSecurityDescriptor = LocalAlloc(LMEM_FIXED, buflen);
        if (*ppSecurityDescriptor == NULL)
            hr = E_OUTOFMEMORY;
        else {
            memcpy(*ppSecurityDescriptor, psd, buflen);
            hr = S_OK;
        }
    }
    else
        hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetSecurity");
    Py_XDECREF(result);
    return hr;
}

// @pymethod |PyGSecurityInformation|SetSecurity|Applies the modified security to the object
// @pyparm int|SecurityInformation||SECURITY_INFORMATION flags specifying which types of security information are to be
// applied
// @pyparm <o PySECURITY_DESCRIPTOR>|SecurityDescriptor||The security information to be applied to the object
// @rdesc Any returned value is ignored
STDMETHODIMP PyGSecurityInformation::SetSecurity(SECURITY_INFORMATION SecurityInformation,
                                                 PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    PY_GATEWAY_METHOD;
    HRESULT hr;
    PyObject *obsd = PyWinObject_FromSECURITY_DESCRIPTOR(pSecurityDescriptor);
    if (obsd == NULL)
        hr = E_OUTOFMEMORY;
    else
        hr = InvokeViaPolicy("SetSecurity", NULL, "kO", SecurityInformation, obsd);
    Py_XDECREF(obsd);
    return hr;
}

// @object SI_ACCESS|Tuple of 4 items representing SI_ACCESS struct
// @tupleitem 0|<o PyIID>|guid|GUID identifying the object type permissions apply to. Use GUID_NULL for object itself
// @tupleitem 1|int|mask|Bitmask of permissions
// @tupleitem 2|<o PyUNICODE>|Name|Description to be displayed for the permissions
// @tupleitem 3|int|Flags|Indicates which pages will display the permissions, and how they may be inherited. Combination
// of
//  SI_ACCESS_SPECIFIC, SI_ACCESS_GENERAL, SI_ACCESS_CONTAINER, SI_ACCESS_PROPERTY,
//  CONTAINER_INHERIT_ACE, INHERIT_ONLY_ACE, OBJECT_INHERIT_ACE

// @pymethod ((<o SI_ACCESS>,...)  int)|PyGSecurityInformation|GetAccessRights|Retrieves permission that can be set
// @pyparm <o PyIID>|ObjectType||GUID representing type of object, may be None
// @pyparm int|Flags||Indicates which page is requesting the access rights (SI_ADVANCED, SI_EDIT_AUDITS,
// SI_EDIT_PROPERTIES)
// @rdesc This method should return a 2-tuple containing a sequence of <o SI_ACCESS> tuples,
//  and a zero-based index indicating which of them is the default
STDMETHODIMP PyGSecurityInformation::GetAccessRights(
    const GUID *pguidObjectType,  // in
    DWORD dwFlags,                // in - SI_ADVANCED, SI_EDIT_AUDITS, SI_EDIT_PROPERTIES
    PSI_ACCESS *ppAccess,         // out
    ULONG *pcAccesses,            // out
    ULONG *piDefaultAccess)       // out
{
    PY_GATEWAY_METHOD;
    HRESULT hr;
    PyObject *result = NULL, *obObjectType = NULL;
    PyObject *obAccesses, *Accesses_tuple = NULL, *obAccess, *tpAccess;
    PyObject *si_access_guid, *si_access_Name;
    ULONG tuple_ind;

    // This method can be called multiple times, free any previous allocations
    // Unlike GetObjectInformation, it may choose to return different data based on
    // the guid and flags passed in
    FreeAccessRights();

    if (pguidObjectType == NULL) {
        Py_INCREF(Py_None);
        obObjectType = Py_None;
    }
    else {
        obObjectType = PyWinObject_FromIID(*pguidObjectType);
        if (obObjectType == NULL) {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    hr = InvokeViaPolicy("GetAccessRights", &result, "Ok", obObjectType, dwFlags);
    if (FAILED(hr))
        goto done;
    if (!PyArg_ParseTuple(result, "Ok", &obAccesses, piDefaultAccess)) {
        hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetAccessRights");
        goto done;
    }
    Accesses_tuple = PyWinSequence_Tuple(obAccesses, &cAccessRights);
    if (Accesses_tuple == NULL) {
        hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetAccessRights");
        goto done;
    }

    // piDefaultAccess is apparently not sanity checked, and can cause a crash in aclui.dll if
    // greater than actual number of SI_ACCESS's.
    if (*piDefaultAccess >= cAccessRights) {
        *piDefaultAccess = 0;
        PyErr_Warn(PyExc_RuntimeWarning, "DefaultAccess parameter exceeds count of AccessRights, using 0 instead");
    }
    AccessRights = (PSI_ACCESS)malloc(cAccessRights * sizeof(SI_ACCESS));
    if (AccessRights == NULL) {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    ZeroMemory(AccessRights, cAccessRights * sizeof(SI_ACCESS));
    hr = S_OK;
    for (tuple_ind = 0; tuple_ind < cAccessRights; tuple_ind++) {
        obAccess = PyTuple_GET_ITEM(Accesses_tuple, tuple_ind);
        tpAccess = PySequence_Tuple(obAccess);
        if ((tpAccess == NULL) ||
            (!PyArg_ParseTuple(tpAccess, "OkOk", &si_access_guid, &AccessRights[tuple_ind].mask, &si_access_Name,
                               &AccessRights[tuple_ind].dwFlags)) ||
            (!PyWinObject_AsWCHAR(si_access_Name, (WCHAR **)&AccessRights[tuple_ind].pszName)))
            hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetAccessRights");
        else if (si_access_guid != Py_None) {
            AccessRights[tuple_ind].pguid = (GUID *)malloc(sizeof(GUID));
            if (AccessRights[tuple_ind].pguid == NULL)
                hr = E_OUTOFMEMORY;
            else if (!PyWinObject_AsIID(si_access_guid, (GUID *)AccessRights[tuple_ind].pguid))
                hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetAccessRights");
        }
        Py_XDECREF(tpAccess);
        if (FAILED(hr))
            break;
    }

done:
    if (FAILED(hr))
        FreeAccessRights();
    *ppAccess = AccessRights;
    *pcAccesses = cAccessRights;

    Py_XDECREF(result);
    Py_XDECREF(obObjectType);
    Py_XDECREF(Accesses_tuple);
    return hr;
}

// @pymethod int|PyGSecurityInformation|MapGeneric|Translates generic access rights to specific equivalents
// @comm See <om win32security.MapGenericMask>
// @pyparm <o PyIID>|ObjectType||Type of object that permissions apply to, None or GUID_NULL indicates object itself
// @pyparm int|AceFlags||Flags from the ACE that contains the permissions
// @pyparm int|Mask||Bitmask containing access rights
// @rdesc This method should return the input bitmask will all generic rights mapped to specific rights
STDMETHODIMP PyGSecurityInformation::MapGeneric(const GUID *pguidObjectType, UCHAR *pAceFlags, ACCESS_MASK *pMask)
{
    PY_GATEWAY_METHOD;
    HRESULT hr;
    PyObject *result = NULL, *obObjectType = NULL;
    if (pguidObjectType == NULL) {  // docs say this can be NULL or GUID_NULL
        Py_INCREF(Py_None);
        obObjectType = Py_None;
    }
    else
        obObjectType = PyWinObject_FromIID(*pguidObjectType);

    if (obObjectType == NULL)
        hr = E_OUTOFMEMORY;
    else {
        hr = InvokeViaPolicy("MapGeneric", &result, "OBk", obObjectType, *pAceFlags, *pMask);
        if (!FAILED(hr)) {
            *pMask = PyInt_AsUnsignedLongMask(result);
            if ((*pMask == -1) && PyErr_Occurred())
                hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("MapGeneric");
            else
                hr = S_OK;
        }
    }
    Py_XDECREF(result);
    Py_XDECREF(obObjectType);
    return hr;
}

// @object SI_INHERIT_TYPE|Tuple of 3 items describing a method of inheritance
// @tupleitem 0|<o PyIID>|guid|GUID for type of child object, GUID_NULL indicates object itself
// @tupleitem 1|int|Flags|ACE inheritance flags, combination of OBJECT_INHERIT_ACE, CONTAINER_INHERIT_ACE,
// INHERIT_ONLY_ACE
// @tupleitem 2|<o PyUNICODE>|Name|Description that will be displayed on the Advanced page

// @pymethod (<o SI_INHERIT_TYPE>,...)|PyGSecurityInformation|GetInheritTypes|Requests types of inheritance that your
// implementation supports
// @rdesc Returns a sequence of <o SI_INHERIT_TYPE> tuples
STDMETHODIMP PyGSecurityInformation::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes, ULONG *pcInheritTypes)
{
    PY_GATEWAY_METHOD;
    PyObject *result = NULL, *InheritTypes_tuple = NULL, *obInheritType, *tpInheritType = NULL;
    PyObject *obguid, *obName;
    ULONG tuple_ind;
    // This can also be called repeatedly (every time you hit 'Edit' on the Advanced page)
    FreeInheritTypes();

    HRESULT hr = InvokeViaPolicy("GetInheritTypes", &result, NULL);
    if (FAILED(hr))
        goto done;
    InheritTypes_tuple = PyWinSequence_Tuple(result, &cInheritTypes);
    if (InheritTypes_tuple == NULL) {
        hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetInheritTypes");
        goto done;
    }
    InheritTypes = (PSI_INHERIT_TYPE)malloc(cInheritTypes * sizeof(SI_INHERIT_TYPE));
    if (InheritTypes == NULL) {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    ZeroMemory(InheritTypes, cInheritTypes * sizeof(SI_INHERIT_TYPE));
    hr = S_OK;
    for (tuple_ind = 0; tuple_ind < cInheritTypes; tuple_ind++) {
        obInheritType = PyTuple_GET_ITEM(InheritTypes_tuple, tuple_ind);
        tpInheritType = PySequence_Tuple(obInheritType);
        if ((tpInheritType == NULL) ||
            (!PyArg_ParseTuple(tpInheritType, "OkO", &obguid, &InheritTypes[tuple_ind].dwFlags, &obName)) ||
            (!PyWinObject_AsWCHAR(obName, (WCHAR **)&InheritTypes[tuple_ind].pszName)))
            hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetInheritTypes");
        else if (obguid != Py_None) {
            InheritTypes[tuple_ind].pguid = (GUID *)malloc(sizeof(GUID));
            if (InheritTypes[tuple_ind].pguid == NULL)
                hr = E_OUTOFMEMORY;
            else if (!PyWinObject_AsIID(obguid, (GUID *)InheritTypes[tuple_ind].pguid))
                hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetInheritTypes");
        }
        Py_XDECREF(tpInheritType);
        if (FAILED(hr))
            break;
    }

done:
    if (FAILED(hr))
        FreeInheritTypes();
    *ppInheritTypes = InheritTypes;
    *pcInheritTypes = cInheritTypes;
    Py_XDECREF(InheritTypes_tuple);
    Py_XDECREF(result);
    return hr;
}

// @pymethod |PyGSecurityInformation|PropertySheetPageCallback|Called by each page as it is created and destroyed
// @pyparm int|hwnd||Handle to the window for the page
// @pyparm int|Msg||Flag indicating type of event, one of PSPCB_CREATE,PSPCB_RELEASE,PSPCB_SI_INITDIALOG
// @pyparm int|Page||SI_PAGE_TYPE value indicating which page is making the call (ntsecuritycon.SI_PAGE_*)
// @rdesc Any returned value will be ignored
STDMETHODIMP PyGSecurityInformation::PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage)

{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("PropertySheetPageCallback", NULL, "Nkk", PyWinLong_FromHANDLE(hwnd), uMsg, uPage);
}
