# General test module for win32api - please add some :)

import unittest

import win32clipboard

class CrashingTestCase(unittest.TestCase):
    def test_722082(self):
        class crasher(object):
            pass

        obj = crasher()
        win32clipboard.OpenClipboard()
        win32clipboard.EmptyClipboard()
        # This used to crash - now correctly raises type error.
        self.assertRaises(TypeError, win32clipboard.SetClipboardData, 0, obj )

if __name__ == '__main__':
    unittest.main()
