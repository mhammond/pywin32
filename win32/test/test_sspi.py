# Some tests of the win32security sspi functions.
# Stolen from Roger's original test_sspi.c, a version of which is in "Demos"
# See also the other SSPI demos.
import win32security, sspi, sspicon, win32api
import unittest

class TestSSPI(unittest.TestCase):

    def _doAuth(self, pkg_name):
        sspiclient=sspi.ClientAuth(pkg_name,targetspn=win32api.GetUserName())
        sspiserver=sspi.ServerAuth(pkg_name)

        sec_buffer=None
        err = 1
        while err != 0:
            err, sec_buffer = sspiclient.authorize(sec_buffer)
            err, sec_buffer = sspiserver.authorize(sec_buffer)
        return sspiclient, sspiserver

    def _doTestImpersonate(self, pkg_name):
        # Just for the sake of code exercising!
        sspiclient, sspiserver = self._doAuth(pkg_name)
        sspiserver.ctxt.ImpersonateSecurityContext()
        sspiserver.ctxt.RevertSecurityContext()

    def testImpersonateKerberos(self):
        self._doTestImpersonate("Kerberos")

    def testImpersonateNTLM(self):
        self._doTestImpersonate("NTLM")

    def _doTestEncrypt(self, pkg_name):

        sspiclient, sspiserver = self._doAuth(pkg_name)

        pkg_size_info=sspiclient.ctxt.QueryContextAttributes(sspicon.SECPKG_ATTR_SIZES)
        msg='some data to be encrypted ......'

        trailersize=pkg_size_info['SecurityTrailer']
        encbuf=win32security.SecBufferDescType()
        encbuf.append(win32security.SecBufferType(len(msg), sspicon.SECBUFFER_DATA))
        encbuf.append(win32security.SecBufferType(trailersize, sspicon.SECBUFFER_TOKEN))
        encbuf[0].Buffer=msg
        sspiclient.ctxt.EncryptMessage(0,encbuf,1)
        sspiserver.ctxt.DecryptMessage(encbuf,1)
        self.failUnlessEqual(msg, encbuf[0].Buffer)
        # and test the higher-level functions
        self.assertEqual(sspiserver.decrypt(sspiclient.encrypt("hello")), "hello")
        self.assertEqual(sspiclient.decrypt(sspiserver.encrypt("hello")), "hello")

    def testEncryptNTLM(self):
        self._doTestEncrypt("NTLM")
    
    def testEncryptKerberos(self):
        self._doTestEncrypt("Kerberos")

    def _doTestSign(self, pkg_name):

        sspiclient, sspiserver = self._doAuth(pkg_name)

        pkg_size_info=sspiclient.ctxt.QueryContextAttributes(sspicon.SECPKG_ATTR_SIZES)
        msg='some data to be encrypted ......'
        
        sigsize=pkg_size_info['MaxSignature']
        sigbuf=win32security.SecBufferDescType()
        sigbuf.append(win32security.SecBufferType(len(msg), sspicon.SECBUFFER_DATA))
        sigbuf.append(win32security.SecBufferType(sigsize, sspicon.SECBUFFER_TOKEN))
        sigbuf[0].Buffer=msg
        sspiclient.ctxt.MakeSignature(0,sigbuf,1)
        sspiserver.ctxt.VerifySignature(sigbuf,1)
        # and test the higher-level functions
        self.assertEqual(sspiserver.unsign(sspiclient.sign("hello")), "hello")
        # and the other way
        self.assertEqual(sspiclient.unsign(sspiserver.sign("hello")), "hello")

    def testSignNTLM(self):
        self._doTestSign("NTLM")
    
    def testSignKerberos(self):
        self._doTestSign("Kerberos")

if __name__=='__main__':
    unittest.main()
