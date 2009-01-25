import unittest
import win32event
import time
import os
import sys
from pywin32_testutil import int2long

class TestWaitableTimer(unittest.TestCase):
    def testWaitableFireLong(self):
        h = win32event.CreateWaitableTimer(None, 0, None)
        dt = int2long(-160) # 160 ns.
        win32event.SetWaitableTimer(h, dt, 0, None, None, 0)
        rc = win32event.WaitForSingleObject(h, 1000)
        self.failUnlessEqual(rc, win32event.WAIT_OBJECT_0)

    def testWaitableFire(self):
        h = win32event.CreateWaitableTimer(None, 0, None)
        dt = -160 # 160 ns.
        win32event.SetWaitableTimer(h, dt, 0, None, None, 0)
        rc = win32event.WaitForSingleObject(h, 1000)
        self.failUnlessEqual(rc, win32event.WAIT_OBJECT_0)

    def testWaitableTrigger(self):
        h = win32event.CreateWaitableTimer(None, 0, None)
        # for the sake of this, pass a long that doesn't fit in an int.
        dt = -2000000000
        win32event.SetWaitableTimer(h, dt, 0, None, None, 0)
        rc = win32event.WaitForSingleObject(h, 10) # 10 ms.
        self.failUnlessEqual(rc, win32event.WAIT_TIMEOUT)

class TestWaitFunctions(unittest.TestCase):
    def testMsgWaitForMultipleObjects(self):
        # this function used to segfault when called with an empty list
        res = win32event.MsgWaitForMultipleObjects([], 0, 0, 0)
        self.assertEquals(res, win32event.WAIT_TIMEOUT)

    def testMsgWaitForMultipleObjects2(self):
        # test with non-empty list
        event = win32event.CreateEvent(None, 0, 0, None)
        res = win32event.MsgWaitForMultipleObjects([event], 0, 0, 0)
        self.assertEquals(res, win32event.WAIT_TIMEOUT)

    def testMsgWaitForMultipleObjectsEx(self):
        # this function used to segfault when called with an empty list
        res = win32event.MsgWaitForMultipleObjectsEx([], 0, 0, 0)
        self.assertEquals(res, win32event.WAIT_TIMEOUT)

    def testMsgWaitForMultipleObjectsEx2(self):
        # test with non-empty list
        event = win32event.CreateEvent(None, 0, 0, None)
        res = win32event.MsgWaitForMultipleObjectsEx([event], 0, 0, 0)
        self.assertEquals(res, win32event.WAIT_TIMEOUT)

if __name__=='__main__':
    unittest.main()
