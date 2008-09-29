"""Test pywin32's error semantics"""
import unittest
import win32api, win32file, pywintypes
import pythoncom
import winerror

class TestBase(unittest.TestCase):
    def _testExceptionIndex(self, exc, index, expected):
        # check the exception itself can be indexed.
        self.failUnlessEqual(exc[index], expected)
        # and that exception.args can is the same.
        self.failUnlessEqual(exc.args[index], expected)

class TestAPISimple(TestBase):
    def _getInvalidHandleException(self):
        try:
            win32api.CloseHandle(1)
        except win32api.error, exc:
            return exc
        self.fail("Didn't get invalid-handle exception.")

    def testSimple(self):
        self.assertRaises(pywintypes.error, win32api.CloseHandle, 1)

    def testErrnoIndex(self):
        exc = self._getInvalidHandleException()
        self._testExceptionIndex(exc, 0, winerror.ERROR_INVALID_HANDLE)

    def testFuncIndex(self):
        exc = self._getInvalidHandleException()
        self._testExceptionIndex(exc, 1, "CloseHandle")

    def testMessageIndex(self):
        exc = self._getInvalidHandleException()
        expected = win32api.FormatMessage(winerror.ERROR_INVALID_HANDLE).rstrip()
        self._testExceptionIndex(exc, 2, expected)

    def testUnpack(self):
        try:
            win32api.CloseHandle(1)
            self.fail("expected exception!")
        except win32api.error, (werror, func, msg):
            self.failUnlessEqual(werror, winerror.ERROR_INVALID_HANDLE)
            self.failUnlessEqual(func, "CloseHandle")
            expected_msg = win32api.FormatMessage(winerror.ERROR_INVALID_HANDLE).rstrip()
            self.failUnlessEqual(msg, expected_msg)

class TestCOMSimple(TestBase):
    def _getException(self):
        try:
            pythoncom.StgOpenStorage("foo", None, 0)
        except pythoncom.com_error, exc:
            return exc
        self.fail("Didn't get storage exception.")

    def testIs(self):
        self.failUnless(pythoncom.com_error is pywintypes.com_error)

    def testSimple(self):
        self.assertRaises(pythoncom.com_error, pythoncom.StgOpenStorage, "foo", None, 0)

    def testErrnoIndex(self):
        exc = self._getException()
        self._testExceptionIndex(exc, 0, winerror.STG_E_INVALIDFLAG)

    def testMessageIndex(self):
        exc = self._getException()
        expected = win32api.FormatMessage(winerror.STG_E_INVALIDFLAG).rstrip()
        self._testExceptionIndex(exc, 1, expected)

if __name__ == '__main__':
    unittest.main()
