# The start of some general demos of the win32api module.

# Please contribute how to use your favourite function :)
import win32api, win32con

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
