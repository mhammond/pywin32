# Test module for win32timezone

import sys
import unittest
import win32timezone
import doctest


class Win32TimeZoneTest(unittest.TestCase):
    def testWin32TZ(self):
        failed, total = doctest.testmod(win32timezone, verbose=False)
        self.assertFalse(failed)


if __name__ == "__main__":
    unittest.main()
