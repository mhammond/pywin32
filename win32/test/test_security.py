import win32api, win32con, win32security

pwr_sid=win32security.LookupAccountName('','Power Users')[0]
admin_sid=win32security.LookupAccountName('','Administrator')[0]

sd=win32security.SECURITY_DESCRIPTOR()
for x in xrange(0,100000):
    sd.SetSecurityDescriptorOwner(admin_sid,0)

sd=win32security.SECURITY_DESCRIPTOR()
for x in xrange(0,100000):
    sd.SetSecurityDescriptorGroup(pwr_sid,0)

sd=win32security.SECURITY_DESCRIPTOR()
dacl=win32security.ACL()
dacl.AddAccessAllowedAce(win32security.ACL_REVISION,win32con.GENERIC_READ,pwr_sid)
dacl.AddAccessAllowedAce(win32security.ACL_REVISION,win32con.GENERIC_ALL,admin_sid)
for x in xrange(0,1000000):
    sd.SetSecurityDescriptorDacl(1,dacl,0)

sd=win32security.SECURITY_DESCRIPTOR()
sacl=win32security.ACL()
sacl.AddAuditAccessAce(win32security.ACL_REVISION,win32con.DELETE,admin_sid,1,1)
sacl.AddAuditAccessAce(win32security.ACL_REVISION,win32con.GENERIC_ALL,pwr_sid,1,1)
for x in xrange(0,1000000):
    sd.SetSecurityDescriptorSacl(1,sacl,0)

