# General test module for win32api - please add some :)

import unittest

import win32api, win32con

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

if __name__ == '__main__':
    unittest.main()
