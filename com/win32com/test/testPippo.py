import sys
import unittest
import pythoncom
from win32com.client import Dispatch

class PippoTester(unittest.TestCase):
    def setUp(self):
        try:
            self.object = Dispatch("Python.Test.Pippo")
        except pythoncom.com_error:
            # register the server
            import pippo_server
            pippo_server.main([pippo_server.__file__])
            self.object = Dispatch("Python.Test.Pippo")

    def testLeaks(self):
        try:
            gtrc = sys.gettotalrefcount
        except AttributeError:
            print "Please run this with python_d for leak tests"
            return
        # note creating self.object() should have consumed our "one time" leaks
        self.object.Method1()
        start = gtrc()
        for i in range(1000):
            object = Dispatch("Python.Test.Pippo")
            object.Method1()
        object = None
        end = gtrc()
        if end-start > 5:
            self.fail("We lost %d references!" % (end-start,))
if __name__=='__main__':
    unittest.main()
