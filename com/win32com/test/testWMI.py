from win32com.client import GetObject

def test():
    cses = GetObject("WinMgMts:").InstancesOf("Win32_Process")
    for cs in cses:
        print cs.Properties_("Caption").Value

if __name__=='__main__':
    test()
