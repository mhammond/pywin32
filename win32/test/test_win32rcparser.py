import sys, os
import unittest
import win32rcparser

class TestParser(unittest.TestCase):
    def setUp(self):
        rc_file = os.path.join(os.path.dirname(__file__), "win32rcparser", "test.rc")
        self.resources = win32rcparser.Parse(rc_file)

    def testStrings(self):
        for sid, expected in [
            ("IDS_TEST_STRING4", "Test 'single quoted' string"),
            ("IDS_TEST_STRING1", 'Test "quoted" string'),
            ("IDS_TEST_STRING3", 'String with single " quote'),
            ("IDS_TEST_STRING2", 'Test string'),
                             ]:
            got = self.resources.stringTable[sid].value
            self.assertEqual(got, expected)

if __name__=='__main__':
    unittest.main()

