from win32com.shell import shell
from win32com.shell.shellcon import *

sf = shell.SHGetDesktopFolder()
print "sf is", sf
enum = sf.EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN)
print "enum is", enum
for i in enum:
    name = sf.GetDisplayNameOf(i, SHGDN_NORMAL)
    print name

