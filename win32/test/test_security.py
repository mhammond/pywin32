import sys, os
import unittest

import win32api, win32con, win32security

class SecurityTests(unittest.TestCase):
    def setUp(self):
        self.pwr_sid=win32security.LookupAccountName('','Power Users')[0]
        self.admin_sid=win32security.LookupAccountName('','Administrator')[0]

    def tearDown(self):
        pass

    def testMemory(self):
        pwr_sid = self.pwr_sid
        admin_sid = self.admin_sid
        sd1=win32security.SECURITY_DESCRIPTOR()
        sd2=win32security.SECURITY_DESCRIPTOR()
        sd3=win32security.SECURITY_DESCRIPTOR()
        dacl=win32security.ACL()
        dacl.AddAccessAllowedAce(win32security.ACL_REVISION,win32con.GENERIC_READ,pwr_sid)
        dacl.AddAccessAllowedAce(win32security.ACL_REVISION,win32con.GENERIC_ALL,admin_sid)
        sd4=win32security.SECURITY_DESCRIPTOR()
        sacl=win32security.ACL()
        sacl.AddAuditAccessAce(win32security.ACL_REVISION,win32con.DELETE,admin_sid,1,1)
        sacl.AddAuditAccessAce(win32security.ACL_REVISION,win32con.GENERIC_ALL,pwr_sid,1,1)
        for x in xrange(0,200000):
            sd1.SetSecurityDescriptorOwner(admin_sid,0)
            sd2.SetSecurityDescriptorGroup(pwr_sid,0)
            sd3.SetSecurityDescriptorDacl(1,dacl,0)
            sd4.SetSecurityDescriptorSacl(1,sacl,0)

if __name__=='__main__':
    unittest.main()
