import unittest
import pywintypes
import time

class TestCase(unittest.TestCase):
    def testPyTimeFormat(self):
        struct_current = time.localtime()
        pytime_current = pywintypes.Time(struct_current)
        # try and test all the standard parts of the format
        format_string = "%a %A %b %B %c %d %H %I %j %m %M %p %S %U %w %W %x %X %y %Y %Z"
        self.assertEquals(pytime_current.Format(format_string), time.strftime(format_string, struct_current))

    def testPyTimePrint(self):
        # This used to crash with an invalid, or too early time.
        # We don't really want to check that it does cause a ValueError
        # (as hopefully this wont be true forever).  So either working, or 
        # ValueError is OK.
        t = pywintypes.Time(-2)
        try:
            t.Format()
        except ValueError:
            return

if __name__ == '__main__':
    unittest.main()

