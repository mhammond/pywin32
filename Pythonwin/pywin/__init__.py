# See if we run in Unicode mode.
# This may be referenced all over the place, so we save it globally.
import win32api, win32con, __builtin__

is_platform_unicode = hasattr(__builtin__, "unicode") and win32api.GetVersionEx()[3] == win32con.VER_PLATFORM_WIN32_NT

del win32api, win32con, __builtin__
