# re-implementation of the MS "Scp" sample functions
# Adds and removes an ActiveDirectory "Service Connection Point",
# including managing the security on the object.
# This is likely to become a 'module' rather than a demo, once
# we had reasonable Python signatures for the functions
# (ie, once we have one, real, service that uses it :)
from win32com.adsi.adsicon import *
from win32com.adsi import adsi
import win32api, win32con
from win32com.client import Dispatch

# Returns distinguished name of SCP.
def ScpCreate(
    service_binding_info, 
    service_class_name,      # Service class string to store in SCP.
    account_name = None,    # Logon account that needs access to SCP.
    container_name = None,
    keywords = None,
    object_class = "serviceConnectionPoint",
    dns_name_type = "A",
    dn = None,
             ):
    container_name = container_name or service_class_name
    # Get the DNS name of the local computer
    dns_name = win32api.GetComputerNameEx(win32con.ComputerNameDnsFullyQualified)
    # Get the distinguished name of the computer object for the local computer
    if dn is None:
        dn = win32api.GetComputerObjectName(win32con.NameFullyQualifiedDN)
    
    # Compose the ADSpath and bind to the computer object for the local computer
    comp = adsi.ADsGetObject("LDAP://" + dn, adsi.IID_IDirectoryObject)
    
    # Publish the SCP as a child of the computer object

    keywords = keywords or []
    # Fill in the attribute values to be stored in the SCP.
    attrs = [
        ("cn", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (container_name,)),
        ("objectClass", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (object_class,)),
        ("keywords", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, keywords),
        ("serviceDnsName", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (dns_name,)),
        ("serviceDnsNameType", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (dns_name_type,)),
        ("serviceClassName", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (service_class_name,)),
        ("serviceBindingInformation", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, (service_binding_info,)),
    ]
    new = comp.CreateDSObject("cn=" + container_name, attrs)
    print "got new", new
    # Wrap in a usable IDispatch object.
    new = Dispatch(new)
    AllowAccessToScpProperties(account_name, new)

def ScpDelete(service_class_name, container_name = None, dn = None):
    container_name = container_name or service_class_name
    if dn is None:
        dn = win32api.GetComputerObjectName(win32con.NameFullyQualifiedDN)
        print "DN is", dn
    
    # Compose the ADSpath and bind to the computer object for the local computer
    comp = adsi.ADsGetObject("LDAP://" + dn, adsi.IID_IDirectoryObject)
    comp.DeleteDSObject("cn=" + container_name)
    print "Deleted!"

def AllowAccessToScpProperties(
    accountSAM, #Service account to allow access.
    scpObject):  # The IADs SCP object.
    
    attribute = "nTSecurityDescriptor";
 
    # If no service account is specified, service runs under LocalSystem.
    # So allow access to the computer account of the service's host.
    if accountSAM:
        trustee = accountSAM
    else:
        # Get the SAM account name of the computer object for the server.
        trustee = win32api.GetComputerObjectName(win32con.NameSamCompatible)
    
    # Get the nTSecurityDescriptor
    sd = getattr(scpObject, attribute)

    acl = sd.DiscretionaryAcl

    ace1 = Dispatch(adsi.CLSID_AccessControlEntry)
    ace2 = Dispatch(adsi.CLSID_AccessControlEntry)

    # Set the properties of the two ACEs.
                            
    # Allow read and write access to the property.
    ace1.AccessMask = ADS_RIGHT_DS_READ_PROP | ADS_RIGHT_DS_WRITE_PROP
    ace2.AccessMask = ADS_RIGHT_DS_READ_PROP | ADS_RIGHT_DS_WRITE_PROP
                            
    # Set the trustee, which is either the service account or the 
    # host computer account.
    ace1.Trustee = trustee
    ace2.Trustee = trustee
                            
    # Set the ACE type.
    ace1.AceType = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT
    ace2.AceType = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT
                            
    # Set AceFlags to zero because ACE is not inheritable.
    ace1.AceFlags = 0
    ace2.AceFlags = 0
 
    # Set Flags to indicate an ACE that protects a specified object.
    ace1.Flags = ADS_FLAG_OBJECT_TYPE_PRESENT
    ace2.Flags = ADS_FLAG_OBJECT_TYPE_PRESENT
 
    # Set ObjectType to the schemaIDGUID of the attribute.
    ace1.ObjectType = "{28630eb8-41d5-11d1-a9c1-0000f80367c1}" # serviceDNSName
    ace2.ObjectType = "{b7b1311c-b82e-11d0-afee-0000f80367c1}" # serviceBindingInformation
 
    # Add the ACEs to the DACL.
    acl.AddAce(ace1)
 
    # Do it again for the second ACE.
    acl.AddAce(ace2)
 
    # Write the modified DACL back to the security descriptor.
    sd.DiscretionaryAcl = acl
    # Write the ntSecurityDescriptor property to the property cache.
    setattr(scpObject, attribute, sd)
    # SetInfo updates the SCP object in the directory.
    scpObject.SetInfo()
    print "Set security on object for account '%s'" % (trustee,)

if __name__=='__main__':
    ScpDelete("PythonSCPTest")
    ScpCreate("2222", "PythonSCPTest", None, keywords = "mark was here".split())
