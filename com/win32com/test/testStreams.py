import pythoncom
import win32com.server.util
import win32com.test.util

import unittest

class Persists:
    _public_methods_ = [ 'GetClassID', 'IsDirty', 'Load', 'Save',
                         'GetSizeMax', 'InitNew' ]
    _com_interfaces_ = [ pythoncom.IID_IPersistStreamInit ]
    def __init__(self):
        self.data = "abcdefg"
        self.dirty = 1
    def GetClassID(self):
        return pythoncom.IID_NULL
    def IsDirty(self):
        return self.dirty
    def Load(self, stream):
        self.data = stream.Read(26)
    def Save(self, stream, clearDirty):
        stream.Write(self.data)
        if clearDirty:
            self.dirty = 0
    def GetSizeMax(self):
        return 1024

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


class StreamTest(win32com.test.util.TestCase):
    def _readWrite(self, data, write_stream, read_stream = None):
        if read_stream is None: read_stream = write_stream
        write_stream.Write(data)
        got = read_stream.Read(len(data))
        self.assertEqual(data, got)

    def testit(self):
        mydata = 'abcdefghijklmnopqrstuvwxyz'
    
        # First test the objects just as Python objects...
        s = Stream(mydata)
        p = Persists()
    
        p.Load(s)
        p.Save(s, 0)
        self.assertEqual(s.data, mydata)
    
        # Wrap the Python objects as COM objects, and make the calls as if
        # they were non-Python COM objects.
        s2 = win32com.server.util.wrap(s, pythoncom.IID_IStream)
        p2 = win32com.server.util.wrap(p, pythoncom.IID_IPersistStreamInit)

        self._readWrite(mydata, s, s)
        self._readWrite(mydata, s, s2)
        self._readWrite(mydata, s2, s)
        self._readWrite(mydata, s2, s2)

        self._readWrite("string with\0a NULL", s2, s2)
        # reset the stream
        s.Write(mydata)
        p2.Load(s2)
        p2.Save(s2, 0)
        self.assertEqual(s.data, mydata)

if __name__=='__main__':
    unittest.main()
