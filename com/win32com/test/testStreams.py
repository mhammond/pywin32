import pythoncom
import win32com.server.util
import util

class Persists:
    _public_methods_ = [ 'GetClassID', 'IsDirty', 'Load', 'Save',
                         'GetSizeMax', 'InitNew' ]
    _com_interfaces_ = [ pythoncom.IID_IPersistStreamInit ]

    def GetClassID(self):
        return pythoncom.IID_NULL

    def IsDirty(self):
        return 1

    def Load(self, stream):
        print "loaded:", stream.Read(26)

    def Save(self, stream, clearDirty):
        stream.Write('ABCDEFGHIJKLMNOPQRSTUVWXYZ')
        print "(saved state)"

    def GetSizeMax(self):
        return 26

    def InitNew(self):
        pass


class Stream:
    _public_methods_ = [ 'Read', 'Write' ]
    _com_interfaces_ = [ pythoncom.IID_IStream ]

    def __init__(self, data):
        self.data = data
        self.index = 0

    def Read(self, amount):
        result = self.data[self.index : self.index + amount]
        self.index = self.index + amount
        return result

    def Write(self, data):
        self.data = data
        self.index = 0
        return len(data)


def test():
    mydata = 'abcdefghijklmnopqrstuvwxyz'

    # First test the objects just as Python objects...
    s = Stream(mydata)
    p = Persists()

    p.Load(s)
    p.Save(s, 0)
    print "new state:", s.data

    # reset the stream
    s.Write(mydata)

    # Wrap the Python objects as COM objects, and make the calls as if
    # they were non-Python COM objects.
    s2 = win32com.server.util.wrap(s, pythoncom.IID_IStream)
    p2 = win32com.server.util.wrap(p, pythoncom.IID_IPersistStreamInit)

    print "read:", s2.Read(26)
    s2.Write("kilroy was here")
    print "new state:", s.data

    # reset the stream
    s.Write(mydata)

    p2.Load(s2)
    p2.Save(s2, 0)
    print "new state:", s.data

if __name__=='__main__':
    test()
    util.CheckClean()
