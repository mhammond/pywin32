import unittest

class PyHandleTestCase(unittest.TestCase):
    def testCleanup1(self):
        # We used to clobber all outstanding exceptions.
        def f1(invalidate):
            """ This function throws a ZeroDivisionError. """
            import win32event
            h = win32event.CreateEvent(None, 0, 0, None)
            if invalidate:
                import win32api
                win32api.CloseHandle(int(h))
            print "raise 1"
            1/0

        def f2(invalidate):
            """ This function should throw an IOError. """
            try:
                f1(invalidate)
            except ZeroDivisionError, exc:
                print "caught 1"
                raise IOError("raise 2")

        self.assertRaises(IOError, f2, False)
        # Now do it again, but so the auto object destruction
        # actually fails.
        self.assertRaises(IOError, f2, True)

    def testCleanup2(self):
        # Cause an exception during object destruction.
        # The worst this does is cause an ".XXX undetected error (why=3)" 
        # So avoiding that is the goal
        import win32event, win32api
        h = win32event.CreateEvent(None, 0, 0, None)
        # Close the handle underneath the object.
        win32api.CloseHandle(int(h))
        # Object destructor runs with the implicit close failing
        h = None

    def testCleanup3(self):
        # And again with a class - no __del__
        import win32event, win32api
        class Test:
            def __init__(self):
                self.h = win32event.CreateEvent(None, 0, 0, None)
                win32api.CloseHandle(int(self.h))
        t=Test()
        t = None

    def testCleanupGood(self):
        # And check that normal error semantics *do* work.
        import win32event, win32api
        h = win32event.CreateEvent(None, 0, 0, None)
        win32api.CloseHandle(int(h))
        self.assertRaises(win32api.error, h.Close)

if __name__ == '__main__':
    unittest.main()
