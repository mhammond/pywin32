# The start of a win32gui generic demo.
# Feel free to contribute more demos back ;-)

import win32gui

def _MyCallback( hwnd, extra ):
    extra.append(hwnd)

def TestEnumWindows():
    windows = []
    win32gui.EnumWindows(_MyCallback, windows)
    print "Enumerated a total of %d windows" % (len(windows),)

print "Enumerating all windows..."
TestEnumWindows()
print "All tests done!"
