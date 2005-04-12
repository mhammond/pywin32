import unittest
import win32api, win32file, win32con, pywintypes, winerror
import sys
import os
import tempfile
import sets
import shutil

class TestSimpleOps(unittest.TestCase):
    def testSimpleFiles(self):
        try:
            fd, filename = tempfile.mkstemp()
        except AttributeError:
            self.fail("This test requires Python 2.3 or later")
        os.close(fd)
        os.unlink(filename)
        handle = win32file.CreateFile(filename, win32file.GENERIC_WRITE, 0, None, win32con.CREATE_NEW, 0, None)
        test_data = "Hello\0there"
        try:
            win32file.WriteFile(handle, test_data)
            handle.Close()
            # Try and open for read
            handle = win32file.CreateFile(filename, win32file.GENERIC_READ, 0, None, win32con.OPEN_EXISTING, 0, None)
            rc, data = win32file.ReadFile(handle, 1024)
            self.assertEquals(data, test_data)
        finally:
            handle.Close()
            try:
                os.unlink(filename)
            except os.error:
                pass

    # A simple test using normal read/write operations.
    def testMoreFiles(self):
        # Create a file in the %TEMP% directory.
        testName = os.path.join( win32api.GetTempPath(), "win32filetest.dat" )
        desiredAccess = win32file.GENERIC_READ | win32file.GENERIC_WRITE
        # Set a flag to delete the file automatically when it is closed.
        fileFlags = win32file.FILE_FLAG_DELETE_ON_CLOSE
        h = win32file.CreateFile( testName, desiredAccess, win32file.FILE_SHARE_READ, None, win32file.CREATE_ALWAYS, fileFlags, 0)
    
        # Write a known number of bytes to the file.
        data = "z" * 1025
    
        win32file.WriteFile(h, data)
    
        self.failUnless(win32file.GetFileSize(h) == len(data), "WARNING: Written file does not have the same size as the length of the data in it!")
    
        # Ensure we can read the data back.
        win32file.SetFilePointer(h, 0, win32file.FILE_BEGIN)
        hr, read_data = win32file.ReadFile(h, len(data)+10) # + 10 to get anything extra
        self.failUnless(hr==0, "Readfile returned %d" % hr)

        self.failUnless(read_data == data, "Read data is not what we wrote!")
    
        # Now truncate the file at 1/2 its existing size.
        newSize = len(data)/2
        win32file.SetFilePointer(h, newSize, win32file.FILE_BEGIN)
        win32file.SetEndOfFile(h)
        self.failUnless(win32file.GetFileSize(h) == newSize, "Truncated file does not have the expected size!")
    
        # GetFileAttributesEx/GetFileAttributesExW tests.
        self.failUnless(win32file.GetFileAttributesEx(testName) == win32file.GetFileAttributesExW(testName),
                        "ERROR: Expected GetFileAttributesEx and GetFileAttributesExW to return the same data")
    
        attr, ct, at, wt, size = win32file.GetFileAttributesEx(testName)
        self.failUnless(size==newSize, 
                        "Expected GetFileAttributesEx to return the same size as GetFileSize()")
        self.failUnless(attr==win32file.GetFileAttributes(testName), 
                        "Expected GetFileAttributesEx to return the same attributes as GetFileAttributes")
    
        h = None # Close the file by removing the last reference to the handle!

        self.failUnless(not os.path.isfile(testName), "After closing the file, it still exists!")

    def testFilePointer(self):
        # via [ 979270 ] SetFilePointer fails with negative offset

        # Create a file in the %TEMP% directory.
        filename = os.path.join( win32api.GetTempPath(), "win32filetest.dat" )

        f = win32file.CreateFile(filename,
                                win32file.GENERIC_READ|win32file.GENERIC_WRITE,
                                0,
                                None,
                                win32file.CREATE_ALWAYS,
                                win32file.FILE_ATTRIBUTE_NORMAL,
                                0)
        try:
            #Write some data
            data = 'Some data'
            (res, written) = win32file.WriteFile(f, data)
            
            self.failIf(res)
            self.assertEqual(written, len(data))
            
            #Move at the beginning and read the data
            win32file.SetFilePointer(f, 0, win32file.FILE_BEGIN)
            (res, s) = win32file.ReadFile(f, len(data))
            
            self.failIf(res)
            self.assertEqual(s, data)
            
            #Move at the end and read the data
            win32file.SetFilePointer(f, -len(data), win32file.FILE_END)
            (res, s) = win32file.ReadFile(f, len(data))
            
            self.failIf(res)
            self.failUnlessEqual(s, data)
        finally:
            f.Close()
            os.unlink(filename)

class TestOverlapped(unittest.TestCase):
    def testSimpleOverlapped(self):
        # Create a file in the %TEMP% directory.
        import win32event
        testName = os.path.join( win32api.GetTempPath(), "win32filetest.dat" )
        desiredAccess = win32file.GENERIC_WRITE
        overlapped = pywintypes.OVERLAPPED()
        evt = win32event.CreateEvent(None, 0, 0, None)
        overlapped.hEvent = evt
        # Create the file and write shit-loads of data to it.
        h = win32file.CreateFile( testName, desiredAccess, 0, None, win32file.CREATE_ALWAYS, 0, 0)
        chunk_data = "z" * 0x8000
        num_loops = 512
        expected_size = num_loops * len(chunk_data)
        for i in range(num_loops):
            win32file.WriteFile(h, chunk_data, overlapped)
            win32event.WaitForSingleObject(overlapped.hEvent, win32event.INFINITE)
            overlapped.Offset = overlapped.Offset + len(chunk_data)
        h.Close()
        # Now read the data back overlapped
        overlapped = pywintypes.OVERLAPPED()
        evt = win32event.CreateEvent(None, 0, 0, None)
        overlapped.hEvent = evt
        desiredAccess = win32file.GENERIC_READ
        h = win32file.CreateFile( testName, desiredAccess, 0, None, win32file.OPEN_EXISTING, 0, 0)
        buffer = win32file.AllocateReadBuffer(0xFFFF)
        while 1:
            try:
                hr, data = win32file.ReadFile(h, buffer, overlapped)
                win32event.WaitForSingleObject(overlapped.hEvent, win32event.INFINITE)
                overlapped.Offset = overlapped.Offset + len(data)
                if not data is buffer:
                    self.fail("Unexpected result from ReadFile - should be the same buffer we passed it")
            except win32api.error:
                break
        h.Close()

    def testCompletionPortsMultiple(self):
        # Mainly checking that we can "associate" an existing handle.  This
        # failed in build 203.
        import socket

        ioport = win32file.CreateIoCompletionPort(win32file.INVALID_HANDLE_VALUE,
                                                  0, 0, 0)
        socks = []
        for PORT in range(9123, 9125):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind(('', PORT))
            sock.listen(1)
            socks.append(sock)
            new = win32file.CreateIoCompletionPort(sock.fileno(), ioport, PORT, 0)
            assert new is ioport
        for s in socks:
            s.close()
        hv = int(ioport)
        ioport = new = None
        # The handle itself should be closed now (unless we leak references!)
        # Check that.
        try:
            win32file.CloseHandle(hv)
            raise RuntimeError, "Expected close to fail!"
        except win32file.error, (hr, func, msg):
            self.failUnlessEqual(hr, winerror.ERROR_INVALID_HANDLE)


class TestFindFiles(unittest.TestCase):
    def testIter(self):
        dir = os.path.join(os.getcwd(), "*")
        files = win32file.FindFilesW(dir)
        set1 = sets.Set()
        set1.update(files)
        set2 = sets.Set()
        for file in win32file.FindFilesIterator(dir):
            set2.add(file)
        assert len(set2) > 5, "This directory has less than 5 files!?"
        self.failUnlessEqual(set1, set2)

    def testBadDir(self):
        dir = os.path.join(os.getcwd(), "a dir that doesnt exist", "*")
        self.assertRaises(win32file.error, win32file.FindFilesIterator, dir)

    def testEmptySpec(self):
        spec = os.path.join(os.getcwd(), "*.foo_bar")
        num = 0
        for i in win32file.FindFilesIterator(spec):
            print "Got", i
            num += 1
        self.failUnlessEqual(0, num)

    def testEmptyDir(self):
        test_path = os.path.join(win32api.GetTempPath(), "win32file_test_directory")
        try:
            shutil.rmtree(test_path)
        except os.error:
            pass
        os.mkdir(test_path)
        try:
            num = 0
            for i in win32file.FindFilesIterator(os.path.join(test_path, "*")):
                num += 1
            # Expecting "." and ".." only
            self.failUnlessEqual(2, num)
        finally:
            shutil.rmtree(test_path)

class TestEncrypt(unittest.TestCase):
    def testEncrypt(self):
        fname = tempfile.mktemp("win32file_test")
        f = open(fname, "wb")
        f.write("hello")
        f.close()
        f = None
        try:
            try:
                win32file.EncryptFile(fname)
            except win32file.error, details:
                if details[0] != winerror.ERROR_ACCESS_DENIED:
                    raise
                print "It appears this is not NTFS - cant encrypt/decrypt"
            win32file.DecryptFile(fname)
        finally:
            if f is not None:
                f.close()
            os.unlink(fname)

if __name__ == '__main__':
    unittest.main()
