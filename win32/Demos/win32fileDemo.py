# Test/Demo the native win32 file API for Python.
import win32file, win32api
import os

# todo: Add more tests/demos to make this truly useful!

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
		
	print "Successfully performed some basic tests of win32file!"
		
if __name__=='__main__':
	Test()
