# Pre-install script for win32all.  Note that as this is run before
# anything is installed, it must not reference any win32all modules!
import sys, os
import _winreg
# win32con constants (but we can't use win32con!)
MB_YESNO = 4
IDYES = 6
IDNO = 7

# for debugging outside of the distutils environment.
if 0:
    try:
        message_box
    except NameError:
        import win32gui
        def message_box(cap, tit, flags):
            return win32gui.MessageBox(0, cap, tit, flags)

# If there is an old WISE built win32all installed, insist on removing that.
app_title = "Python %d.%d combined Win32 extensions" % sys.version_info[:2]
key_name = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + \
           app_title

for root in [_winreg.HKEY_LOCAL_MACHINE, _winreg.HKEY_CURRENT_USER]:
    try:
        key = _winreg.OpenKey(root, key_name)
        prog, type_id = _winreg.QueryValueEx(key, "UninstallString")
    except OSError, details:
        prog = None
    if prog:
        rc = message_box("An existing version of win32all is installed.\r\n"
                         "This version must be removed before installing this "
                         "version.\r\nIf you do not uninstall the previous "
                         "version,\r\nthe installation will be cancelled."
                         "\r\n\r\nDo you wish to uninstall the existing version?",
                         "Uninstall existing version?",
                         MB_YESNO)
        if rc == IDNO:
            raise RuntimeError, "You must uninstall the previous version"
        # We know out versions used short-names, so a simple 'split()' will
        # do to parse our args
        f = os.popen(prog)
        f.read()
        f.close()
