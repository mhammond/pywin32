# Test/Demo the native win32 file API for Python.
import win32file, win32api, pywintypes, win32event
import os
import time

# todo: Add more tests/demos to make this truly useful!
def OverlappedTest():
	# Create a file in the %TEMP% directory.
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
			
			assert data is buffer, "Unexpected result from ReadFile - should be the same buffer we passed it"
		except win32api.error:
			break
	h.Close()
# A simple test using normal read/write operations.
def Test():
	# Create a file in the %TEMP% directory.
	testName = os.path.join( win32api.GetTempPath(), "win32filetest.dat" )
	desiredAccess = win32file.GENERIC_READ | win32file.GENERIC_WRITE
	# Set a flag to delete the file automatically when it is closed.
	fileFlags = win32file.FILE_FLAG_DELETE_ON_CLOSE
	h = win32file.CreateFile( testName, desiredAccess, win32file.FILE_SHARE_READ, None, win32file.CREATE_ALWAYS, fileFlags, 0)
	
	# Write a known number of bytes to the file.
	data = "z" * 1024
	
	win32file.WriteFile(h, data)
	
	if win32file.GetFileSize(h) != len(data):
		print "WARNING: Written file does not have the same size as the length of the data in it!"
		print "Reported size is", win32file.GetFileSize(h), "but expected to be", len(data)

	# Now truncate the file at 1/2 its existing size.
	newSize = len(data)/2
	win32file.SetFilePointer(h, newSize, win32file.FILE_BEGIN)
	win32file.SetEndOfFile(h)
	if win32file.GetFileSize(h) != newSize:
		print "WARNING: Truncated file does not have the expected size!"
		print "Reported size is", win32file.GetFileSize(h), "but expected to be", newSize
	
	h = None # Close the file by removing the last reference to the handle!
	
	if os.path.isfile(testName):
		print "WARNING: After closing the file, it still exists!"
		

if __name__=='__main__':
	Test()
	OverlappedTest()
	print "Successfully performed some basic tests of win32file!"

