from win32com.client import GetObject

import unittest

class Simple(unittest.TestCase):
    def testit(self):
        cses = GetObject("WinMgMts:").InstancesOf("Win32_Process")
        vals = []
        for cs in cses:
            val = cs.Properties_("Caption").Value
            vals.append(val)
        self.failIf(len(vals)<5, "We only found %d processes!" % len(vals))

if __name__=='__main__':
    unittest.main()

