import sys, os
import unittest

import pythoncom
from win32com.shell import shell
from win32com.shell.shellcon import *
from win32com.storagecon import *

class ShellTester(unittest.TestCase):
    def testShellLink(self):
        desktop = str(shell.SHGetSpecialFolderPath(0, CSIDL_DESKTOP))
        num = 0
        shellLink = pythoncom.CoCreateInstance(shell.CLSID_ShellLink, None, pythoncom.CLSCTX_INPROC_SERVER, shell.IID_IShellLink)
        persistFile = shellLink.QueryInterface(pythoncom.IID_IPersistFile)
        for base_name in os.listdir(desktop):
            name = os.path.join(desktop, base_name)
            try:
                persistFile.Load(name,STGM_READ)
            except pythoncom.com_error:
                continue
            # Resolve is slow - avoid it for our tests.
            #shellLink.Resolve(0, shell.SLR_ANY_MATCH | shell.SLR_NO_UI)
            fname, findData = shellLink.GetPath(0)
            unc = shellLink.GetPath(shell.SLGP_UNCPRIORITY)[0]
            num += 1
        self.failIf(num<3, "Only found %d items on the desktop??" % num)

    def testShellFolder(self):
        sf = shell.SHGetDesktopFolder()
        names_1 = []
        for i in sf: # Magically calls EnumObjects
            name = sf.GetDisplayNameOf(i, SHGDN_NORMAL)
            names_1.append(name)
        
        # And get the enumerator manually    
        enum = sf.EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN)
        names_2 = []
        for i in enum:
            name = sf.GetDisplayNameOf(i, SHGDN_NORMAL)
            names_2.append(name)
        names_1.sort()
        names_2.sort()
        self.assertEqual(names_1, names_2)

if __name__=='__main__':
    unittest.main()
