# General test module for win32api - please add some :)

import unittest

import win32api, win32con, win32event
import sys, os

class CurrentUserTestCase(unittest.TestCase):
    def testGetCurrentUser(self):
        name = "%s\\%s" % (win32api.GetDomainName(), win32api.GetUserName())
        self.failUnless(name == win32api.GetUserNameEx(win32api.NameSamCompatible))

class TimeZone(unittest.TestCase):
    def testTimezone(self):
        # GetTimeZoneInformation
        rc, tzinfo = win32api.GetTimeZoneInformation()
        if rc == win32con.TIME_ZONE_ID_DAYLIGHT:
            tz_str = tzinfo[4]
            tz_time = tzinfo[5]
        else:
            tz_str = tzinfo[1]
            tz_time = tzinfo[2]
        print "Time zone in effect is", tz_str.encode()
        print "Next timezone change happens at", tz_time.Format()

class Registry(unittest.TestCase):
    key_name = r'PythonTestHarness\Whatever'
    def test1(self):
        # This used to leave a stale exception behind.
        def reg_operation():
            hkey = win32api.RegCreateKey(win32con.HKEY_CURRENT_USER, self.key_name)
            x = 3/0 # or a statement like: raise 'error'
        # do the test
        try:
            try:
                try:
                    reg_operation()
                except:
                    1/0 # Force exception
            finally:
                win32api.RegDeleteKey(win32con.HKEY_CURRENT_USER, self.key_name)
        except ZeroDivisionError:
            pass
    def testNotifyChange(self):
        def change():
            hkey = win32api.RegCreateKey(win32con.HKEY_CURRENT_USER, self.key_name)
            try:
                win32api.RegSetValue(hkey, None, win32con.REG_SZ, "foo")
            finally:
                win32api.RegDeleteKey(win32con.HKEY_CURRENT_USER, self.key_name)

        evt = win32event.CreateEvent(None,0,0,None)
        ## REG_NOTIFY_CHANGE_LAST_SET - values
        ## REG_CHANGE_NOTIFY_NAME - keys
        ## REG_NOTIFY_CHANGE_SECURITY - security descriptor
        ## REG_NOTIFY_CHANGE_ATTRIBUTES
        win32api.RegNotifyChangeKeyValue(win32con.HKEY_CURRENT_USER,1,win32api.REG_NOTIFY_CHANGE_LAST_SET,evt,True)
        ret_code=win32event.WaitForSingleObject(evt,0)
        # Should be no change.
        self.failUnless(ret_code==win32con.WAIT_TIMEOUT)
        change()
        # Our event should now be in a signalled state.
        ret_code=win32event.WaitForSingleObject(evt,0)
        self.failUnless(ret_code==win32con.WAIT_OBJECT_0)

class FileNames(unittest.TestCase):
    def testShortLongPathNames(self):
        try:
            me = __file__
        except NameError:
            me = sys.argv[0]
        fname = os.path.abspath(me)
        short_name = win32api.GetShortPathName(fname)
        long_name = win32api.GetLongPathName(short_name)
        self.failUnless(long_name==fname, \
                        "Expected long name ('%s') to be original name ('%s')" % (long_name, fname))
        long_name = win32api.GetLongPathNameW(short_name)
        self.failUnless(type(long_name)==unicode, "GetLongPathNameW returned type '%s'" % (type(long_name),))
        self.failUnless(long_name==fname, \
                        "Expected long name ('%s') to be original name ('%s')" % (long_name, fname))

if __name__ == '__main__':
    unittest.main()
