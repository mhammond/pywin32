/* File : win32file.i */
// @doc

%module win32file // An interface to the win32 File API's

%{
//#define UNICODE
#ifndef MS_WINCE
//#define FAR
#include "winsock2.h"
#include "mswsock.h"
#include "assert.h"
#endif

#define NEED_PYWINOBJECTS_H
%}

%include "typemaps.i"
%include "pywin32.i"

#define FILE_GENERIC_READ FILE_GENERIC_READ
#define FILE_GENERIC_WRITE FILE_GENERIC_WRITE
#define FILE_ALL_ACCESS FILE_ALL_ACCESS

#define INVALID_HANDLE_VALUE (long)INVALID_HANDLE_VALUE

#define GENERIC_READ GENERIC_READ 
// Specifies read access to the object. Data can be read from the file and the file pointer can be moved. Combine with GENERIC_WRITE for read-write access. 
#define GENERIC_WRITE GENERIC_WRITE 
// Specifies write access to the object. Data can be written to the file and the file pointer can be moved. Combine with GENERIC_READ for read-write access. 
#define GENERIC_EXECUTE GENERIC_EXECUTE 
// Specifies execute access.

#ifndef MS_WINCE
#define FILE_SHARE_DELETE  FILE_SHARE_DELETE 
// Windows NT only: Subsequent open operations on the object will succeed only if delete access is requested. 
#endif
#define FILE_SHARE_READ FILE_SHARE_READ 
// Subsequent open operations on the object will succeed only if read access is requested. 
#define FILE_SHARE_WRITE FILE_SHARE_WRITE 
// Subsequent open operations on the object will succeed only if write access is requested. 
 
#define CREATE_NEW CREATE_NEW 
// Creates a new file. The function fails if the specified file already exists.
#define CREATE_ALWAYS CREATE_ALWAYS 
// Creates a new file. The function overwrites the file if it exists.
#define OPEN_EXISTING OPEN_EXISTING 
// Opens the file. The function fails if the file does not exist.
#define OPEN_ALWAYS OPEN_ALWAYS 
// Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDistribution were CREATE_NEW.
#define TRUNCATE_EXISTING TRUNCATE_EXISTING 
// Opens the file. Once opened, the file is truncated so that its size is zero bytes. The calling process must open the file with at least GENERIC_WRITE access. The function fails if the file does not exist.
 
#define FILE_ATTRIBUTE_ARCHIVE FILE_ATTRIBUTE_ARCHIVE 
// The file should be archived. Applications use this attribute to mark files for backup or removal.
#define FILE_ATTRIBUTE_COMPRESSED FILE_ATTRIBUTE_COMPRESSED 
// The file or directory is compressed. For a file, this means that all of the data in the file is compressed. For a directory, this means that compression is the default for newly created files and subdirectories.
#define FILE_ATTRIBUTE_HIDDEN FILE_ATTRIBUTE_HIDDEN 
// The file is hidden. It is not to be included in an ordinary directory listing.
#define FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_NORMAL 
// The file has no other attributes set. This attribute is valid only if used alone.
#ifndef MS_WINCE
#define FILE_ATTRIBUTE_OFFLINE FILE_ATTRIBUTE_OFFLINE 
// The data of the file is not immediately available. Indicates that the file data has been physically moved to offline storage.
#endif // MS_WINCE
#define FILE_ATTRIBUTE_READONLY FILE_ATTRIBUTE_READONLY 
// The file is read only. Applications can read the file but cannot write to it or delete it.
#define FILE_ATTRIBUTE_SYSTEM FILE_ATTRIBUTE_SYSTEM 
// The file is part of or is used exclusively by the operating system.
#define FILE_ATTRIBUTE_TEMPORARY FILE_ATTRIBUTE_TEMPORARY 
// The file is being used for temporary storage. File systems attempt to keep all of the data in memory for quicker access rather than flushing the data back to mass storage. A temporary file should be deleted by the application as soon as it is no longer needed.
 
#define FILE_FLAG_WRITE_THROUGH FILE_FLAG_WRITE_THROUGH 
// Instructs the system to write through any intermediate cache and go directly to disk. Windows can still cache write operations, but cannot lazily flush them.
#define FILE_FLAG_OVERLAPPED FILE_FLAG_OVERLAPPED 
// Instructs the system to initialize the object, so that operations that take a significant amount of time to process return ERROR_IO_PENDING. When the operation is finished, the specified event is set to the signaled state.
	// When you specify FILE_FLAG_OVERLAPPED, the ReadFile and WriteFile functions must specify an OVERLAPPED structure. That is, when FILE_FLAG_OVERLAPPED is specified, an application must perform overlapped reading and writing.
	// When FILE_FLAG_OVERLAPPED is specified, the system does not maintain the file pointer. The file position must be passed as part of the lpOverlapped parameter (pointing to an OVERLAPPED structure) to the ReadFile and WriteFile functions.
	// This flag also enables more than one operation to be performed simultaneously with the handle (a simultaneous read and write operation, for example).
#define FILE_FLAG_NO_BUFFERING FILE_FLAG_NO_BUFFERING 
// Instructs the system to open the file with no intermediate buffering or caching. 
	// When combined with FILE_FLAG_OVERLAPPED, the flag gives maximum asynchronous performance, 
	// because the I/O does not rely on the synchronous operations of the memory 
	// manager. However, some I/O operations will take longer, because data is 
	// not being held in the cache. An application must meet certain requirements 
	// when working with files opened with FILE_FLAG_NO_BUFFERING:
	// <nl>-	File access must begin at byte offsets within the file that are integer multiples of the volume's sector size.
	// <nl>-	File access must be for numbers of bytes that are integer multiples of the volume's sector size. 
	// For example, if the sector size is 512 bytes, an application can request reads and writes of 512, 1024, or 2048 bytes, but not of 335, 981, or 7171 bytes.
	// <nl>-	Buffer addresses for read and write operations must be aligned on addresses in memory that are integer multiples of the volume's sector size. 
	// One way to align buffers on integer multiples of the volume sector size is to use VirtualAlloc to allocate the 
	// buffers. It allocates memory that is aligned on addresses that are integer multiples of the operating system's memory page size. Because both memory page 
	// and volume sector sizes are powers of 2, this memory is also aligned on addresses that are integer multiples of a volume's sector size. An application can 
	// determine a volume's sector size by calling the GetDiskFreeSpace function. 
#define FILE_FLAG_RANDOM_ACCESS FILE_FLAG_RANDOM_ACCESS 
// Indicates that the file is accessed randomly. The system can use this as a hint to optimize file caching.
#define FILE_FLAG_SEQUENTIAL_SCAN FILE_FLAG_SEQUENTIAL_SCAN 
// Indicates that the file is to be accessed sequentially from beginning to end. The system can use this as a hint to optimize file caching. 
	// If an application moves the file pointer for random access, optimum caching may not occur; however, correct operation is still guaranteed.
	// Specifying this flag can increase performance for applications that read large files using sequential access. 
	// Performance gains can be even more noticeable for applications that read large files mostly sequentially, but occasionally skip over small ranges of bytes.
#define FILE_FLAG_DELETE_ON_CLOSE FILE_FLAG_DELETE_ON_CLOSE 
// Indicates that the operating system is to delete the file immediately after all of its handles have been closed, 
	// not just the handle for which you specified FILE_FLAG_DELETE_ON_CLOSE. Subsequent open requests for the file will fail, unless FILE_SHARE_DELETE is used. 
#define FILE_FLAG_BACKUP_SEMANTICS FILE_FLAG_BACKUP_SEMANTICS 
// Windows NT only: Indicates that the file is being opened or created for a backup or restore operation. 
	// The operating system ensures that the calling process overrides file security checks, provided it has the necessary permission to do so. The relevant permissions are SE_BACKUP_NAME and SE_RESTORE_NAME.
	// You can also set this flag to obtain a handle to a directory. A directory handle can be passed to some Win32 functions in place of a file handle.
#define FILE_FLAG_POSIX_SEMANTICS FILE_FLAG_POSIX_SEMANTICS 
// Indicates that the file is to be accessed according to POSIX rules. 
	// This includes allowing multiple files with names, differing only in case, for file systems that support such naming. 
	// Use care when using this option because files created with this flag may not be accessible by applications written for MS-DOS or Windows.

#ifndef MS_WINCE
#define SECURITY_ANONYMOUS SECURITY_ANONYMOUS 
// Specifies to impersonate the client at the Anonymous impersonation level.
#define SECURITY_IDENTIFICATION SECURITY_IDENTIFICATION 
// Specifies to impersonate the client at the Identification impersonation level.
#define SECURITY_IMPERSONATION SECURITY_IMPERSONATION 
// Specifies to impersonate the client at the Impersonation impersonation level.
#define SECURITY_DELEGATION SECURITY_DELEGATION 
// Specifies to impersonate the client at the Delegation impersonation level.
#define SECURITY_CONTEXT_TRACKING SECURITY_CONTEXT_TRACKING 
// Specifies that the security tracking mode is dynamic. If this flag is not specified, Security Tracking Mode is static.
#define SECURITY_EFFECTIVE_ONLY SECURITY_EFFECTIVE_ONLY 
// Specifies that only the enabled aspects 	
#endif // MS_WINCE

#ifndef MS_WINCE /* Not on CE */

// @pyswig int|AreFileApisANSI|Determines whether a set of Win32 file functions is using the ANSI or OEM character set code page. This function is useful for 8-bit console input and output operations.
BOOL AreFileApisANSI(void);

#endif // MS_WINCE

// @pyswig |CancelIo|Cancels pending IO requests for the object.
// @pyparm <o PyHANDLE>|handle||The handle being cancelled.
BOOLAPI CancelIo(PyHANDLE handle);

// @pyswig |CopyFile|Copies a file
BOOLAPI CopyFile(
    TCHAR *from, // @pyparm <o PyUnicode>|from||The name of the file to copy from
    TCHAR *to, // @pyparm <o PyUnicode>|to||The name of the file to copy to
    BOOL bFailIfExists); // @pyparm int|bFailIfExists||Indicates if the operation should fail if the file exists.

// @pyswig |CopyFileW|Copies a file (NT/2000 Unicode specific version)
BOOLAPI CopyFileW(
    WCHAR *from, // @pyparm <o PyUnicode>|from||The name of the file to copy from
    WCHAR *to, // @pyparm <o PyUnicode>|to||The name of the file to copy to
    BOOL bFailIfExists); // @pyparm int|bFailIfExists||Indicates if the operation should fail if the file exists.

// CopyFileEx

// @pyswig |CreateDirectory|Creates a directory
BOOLAPI CreateDirectory(
    TCHAR *name, // @pyparm <o PyUnicode>|name||The name of the directory to create
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig |CreateDirectoryW|Creates a directory (NT/2000 Unicode specific version)
BOOLAPI CreateDirectoryW(
    WCHAR *name, // @pyparm <o PyUnicode>|name||The name of the directory to create
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

#ifndef MS_WINCE
// @pyswig |CreateDirectoryEx|Creates a directory
BOOLAPI CreateDirectoryEx(
    TCHAR *templateName, // @pyparm <o PyUnicode>|templateName||Specifies the path of the directory to use as a template when creating the new directory. 
    TCHAR *newDirectory, // @pyparm <o PyUnicode>|newDirectory||Specifies the name of the new directory
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig |CreateDirectoryExW|Creates a directory (NT/2000 Unicode specific version)
BOOLAPI CreateDirectoryExW(
    WCHAR *templateName, // @pyparm <o PyUnicode>|templateName||Specifies the path of the directory to use as a template when creating the new directory. 
    WCHAR *newDirectory, // @pyparm <o PyUnicode>|newDirectory||Specifies the name of the new directory
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None
#endif // MS_WINCE

// @pyswig <o PyHANDLE>|CreateFile|Creates or opens the a file or other object and returns a handle that can be used to access the object.
// @comm The following objects can be opened:<nl>files<nl>pipes<nl>mailslots<nl>communications resources<nl>disk devices (Windows NT only)<nl>consoles<nl>directories (open only)
PyHANDLE CreateFile(
    TCHAR *lpFileName,	// @pyparm <o PyUnicode>|fileName||The name of the file
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||access (read-write) mode
			// Specifies the type of access to the object. An application can obtain read access, write access, read-write access, or device query access. This parameter can be any combination of the following values. 
			// @flagh Value|Meaning 
			// @flag 0|Specifies device query access to the object. An application can query device attributes without accessing the device.
			// @flag GENERIC_READ|Specifies read access to the object. Data can be read from the file and the file pointer can be moved. Combine with GENERIC_WRITE for read-write access.  
			// @flag GENERIC_WRITE|Specifies write access to the object. Data can be written to the file and the file pointer can be moved. Combine with GENERIC_READ for read-write access.
    DWORD dwShareMode,	// @pyparm int|shareMode||Set of bit flags that specifies how the object can be shared. If dwShareMode is 0, the object cannot be shared. Subsequent open operations on the object will fail, until the handle is closed. 
			// To share the object, use a combination of one or more of the following values:
			// @flagh Value|Meaning 
			// @flag FILE_SHARE_DELETE|Windows NT: Subsequent open operations on the object will succeed only if delete access is requested.  
			// @flag FILE_SHARE_READ|Subsequent open operations on the object will succeed only if read access is requested.
			// @flag FILE_SHARE_WRITE|Subsequent open operations on the object will succeed only if write access is requested.
    SECURITY_ATTRIBUTES *lpSecurityAttributes,	// @pyparm <o PySECURITY_ATTRIBUTES>|attributes||The security attributes, or None
    DWORD dwCreationDistribution,	// @pyparm int|creationDisposition||Specifies which action to take on files that exist, and which action to take when files do not exist. For more information about this parameter, see the Remarks section. This parameter must be one of the following values:
			// @flagh Value|Meaning
			// @flag CREATE_NEW|Creates a new file. The function fails if the specified file already exists. 
			// @flag CREATE_ALWAYS|Creates a new file. If the file exists, the function overwrites the file and clears the existing attributes. 
			// @flag OPEN_EXISTING|Opens the file. The function fails if the file does not exist. 
			//       See the Remarks section for a discussion of why you should use the OPEN_EXISTING flag if you are using the CreateFile function for devices, including the console. 
			// @flag OPEN_ALWAYS|Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDisposition were CREATE_NEW. 
			// @flag TRUNCATE_EXISTING|Opens the file. Once opened, the file is truncated so that its size is zero bytes. The calling process must open the file with at least GENERIC_WRITE access. The function fails if the file does not exist. 
    DWORD dwFlagsAndAttributes,	// @pyparm int|flagsAndAttributes||file attributes
    PyHANDLE INPUT_NULLOK // @pyparm <o PyHANDLE>|hTemplateFile||Specifies a handle with GENERIC_READ access to a template file. The template file supplies file attributes and extended attributes for the file being created.   Under Win95, this must be 0, else an exception will be raised.
);

// @pyswig <o PyHANDLE>|CreateFileW|An NT/2000 specific Unicode version of CreateFile - see <om win32file.CreateFile> for more information.
PyHANDLE CreateFileW(
    WCHAR *lpFileName,	// @pyparm <o PyUnicode>|fileName||The name of the file
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||access (read-write) mode
    DWORD dwShareMode,	// @pyparm int|shareMode||Set of bit flags that specifies how the object can be shared. If dwShareMode is 0, the object cannot be shared. Subsequent open operations on the object will fail, until the handle is closed. 
    SECURITY_ATTRIBUTES *lpSecurityAttributes,	// @pyparm <o PySECURITY_ATTRIBUTES>|attributes||The security attributes, or None
    DWORD dwCreationDistribution,	// @pyparm int|creationDisposition||Specifies which action to take on files that exist, and which action to take when files do not exist. For more information about this parameter, see <om win32file.CreateFile>
    DWORD dwFlagsAndAttributes,	// @pyparm int|flagsAndAttributes||file attributes
    PyHANDLE INPUT_NULLOK // @pyparm <o PyHANDLE>|hTemplateFile||Specifies a handle with GENERIC_READ access to a template file. The template file supplies file attributes and extended attributes for the file being created.
);

#ifndef MS_WINCE
// @pyswig <o PyHANDLE>|CreateIoCompletionPort|Can associate an instance of an opened file with a newly created or an existing input/output (I/O) completion port; or it can create an I/O completion port without associating it with a file.
HANDLE CreateIoCompletionPort (
  HANDLE FileHandle,              // @pyparm <o PyHANDLE>|handle||file handle to associate with the I/O completion port
  HANDLE INPUT_NULLOK,  // @pyparm <o PyHANDLE>|existing||handle to the I/O completion port
  DWORD CompletionKey,            // @pyparm int|completionKey||per-file completion key for I/O completion packets
  DWORD NumberOfConcurrentThreads // @pyparm int|numThreads||number of threads allowed to execute concurrently
);

// @pyswig |DefineDosDevice|Lets an application define, redefine, or delete MS-DOS device names. 
BOOLAPI DefineDosDevice(
    DWORD dwFlags,	// @pyparm int|flags||flags specifying aspects of device definition  
    TCHAR *lpDeviceName,	// @pyparm <o PyUnicode>|deviceName||MS-DOS device name string  
    TCHAR *lpTargetPath	// @pyparm <o PyUnicode>|targetPath||MS-DOS or path string for 32-bit Windows.
);
// @pyswig |DefineDosDeviceW|Lets an application define, redefine, or delete MS-DOS device names. (NT/2000 Unicode specific version)
BOOLAPI DefineDosDeviceW(
    DWORD dwFlags,	// @pyparm int|flags||flags specifying aspects of device definition  
    WCHAR *lpDeviceName,	// @pyparm <o PyUnicode>|deviceName||MS-DOS device name string  
    WCHAR *lpTargetPath	// @pyparm <o PyUnicode>|targetPath||MS-DOS or path string for 32-bit Windows.
);
#endif // MS_WINCE

// @pyswig |DeleteFile|Deletes a file.
BOOLAPI DeleteFile(TCHAR *fileName);
// @pyparm <o PyUnicode>|fileName||The filename to delete

// @pyswig |DeleteFileW|Deletes a file (NT/2000 Unicode specific version)
BOOLAPI DeleteFileW(WCHAR *fileName);
// @pyparm <o PyUnicode>|fileName||The filename to delete

%{
// @pyswig string|DeviceIoControl|Call DeviceIoControl
PyObject *MyDeviceIoControl(PyObject *self, PyObject *args)
{
    OVERLAPPED *pOverlapped;
    PyObject *obhFile;
    HANDLE hDevice;
    DWORD readSize;
    PyObject *obOverlapped = NULL;

    DWORD dwIoControlCode;
    char *writeData;
    DWORD writeSize;

    if (!PyArg_ParseTuple(args, "Ols#l|O:DeviceIoControl", 
        &obhFile, // @pyparm int|hFile||Handle to the file
        &dwIoControlCode, // @pyparm int|dwIoControlCode||IOControl Code to use.
        &writeData, &writeSize, // @pyparm string|data||The data to write.
        &readSize, // @pyparm int|readSize||Size of the buffer to create for the read.
        &obOverlapped)) // @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
        return NULL;
    if (obOverlapped==NULL)
        pOverlapped = NULL;
    else {
        if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
            return NULL;
    }
    if (!PyWinObject_AsHANDLE(obhFile, &hDevice))
        return NULL;

    void *readData = malloc(readSize);
    DWORD numRead;
    BOOL ok;
    Py_BEGIN_ALLOW_THREADS

    ok = DeviceIoControl(hDevice,
                         dwIoControlCode,
                         writeData,
                         writeSize,
                         readData, 
                         readSize, 
                         &numRead,
                         pOverlapped);

    Py_END_ALLOW_THREADS
    if (!ok) {
        free(readData);
        return PyWin_SetAPIError("DeviceIoControl");
    }
    
    PyObject *result = PyString_FromStringAndSize((char *)readData, numRead);
    free(readData);
    return result;
}
%}

%native (OVERLAPPED) PyWinMethod_NewOVERLAPPED;
%native(DeviceIoControl) MyDeviceIoControl;


//FileIOCompletionRoutine	

// @pyswig |FindClose|Closes a handle opened with <om win32file.FindOpen>
BOOLAPI FindClose(HANDLE hFindFile);	// @pyparm int|hFindFile||file search handle

#ifndef MS_WINCE 
// @pyswig |FindCloseChangeNotification|Closes a handle.
BOOLAPI FindCloseChangeNotification(
    HANDLE hChangeHandle 	// @pyparm int|hChangeHandle||handle to change notification to close
);

// @pyswig int|FindFirstChangeNotification|Creates a change notification handle and sets up initial change notification filter conditions. A wait on a notification handle succeeds when a change matching the filter conditions occurs in the specified directory or subtree. 
HANDLE FindFirstChangeNotification(
    TCHAR *lpPathName,	// @pyparm <o PyUnicode>|pathName||Name of directory to watch  
    BOOL bWatchSubtree,	// @pyparm int|bWatchSubtree||flag for monitoring directory or directory tree  
    DWORD dwNotifyFilter 	// @pyparm int|notifyFilter||filter conditions to watch for.  See <om win32api.FindFirstChangeNotification> for details.
);

//FindFirstFile	
//FindFirstFileEx	
// FindNextFile	

// @pyswig int|FindNextChangeNotification|Requests that the operating system signal a change notification handle the next time it detects an appropriate change,
BOOLAPI FindNextChangeNotification(
    HANDLE hChangeHandle 	//  @pyparm int|hChangeHandle||handle to change notification to signal  
);

#endif // MS_WINCE

%{
// @pyswig list|FindFilesW|Retrieves a list of matching filenames, using the Windows Unicode API.  An interface to the API FindFirstFileW/FindNextFileW/Find close functions.
static PyObject *
PyFindFilesW(PyObject *self, PyObject *args)
{
	char *fileSpec;
	// @pyparm string|fileSpec||A string that specifies a valid directory or path and filename, which can contain wildcard characters (* and ?).

	if (!PyArg_ParseTuple (args, "s:FindFilesW", &fileSpec))
		return NULL;
	WIN32_FIND_DATAW findData;
	// @pyseeapi FindFirstFile
	HANDLE hFind;
	int len=strlen(fileSpec);
	WCHAR *pBuf = new WCHAR[len+1];
	if (0==MultiByteToWideChar( CP_ACP, 0, fileSpec, len+1, pBuf, sizeof(WCHAR)*(len+1)))
		return PyWin_SetAPIError("MultiByteToWideChar");
	hFind =  ::FindFirstFileW(pBuf, &findData);
	delete [] pBuf;

	if (hFind==INVALID_HANDLE_VALUE) {
		if (::GetLastError()==ERROR_FILE_NOT_FOUND) {	// this is OK
			return PyList_New(0);
		}
		return PyWin_SetAPIError("FindFirstFileW");
	}
	PyObject *retList = PyList_New(0);
	if (!retList) {
		::FindClose(hFind);
		return NULL;
	}
	BOOL ok = TRUE;
	while (ok) {
		PyObject *obCreateTime = PyWinObject_FromFILETIME(findData.ftCreationTime);
		PyObject *obAccessTime = PyWinObject_FromFILETIME(findData.ftLastAccessTime);
		PyObject *obWriteTime = PyWinObject_FromFILETIME(findData.ftLastWriteTime);
		if (obCreateTime==NULL || obAccessTime==NULL || obWriteTime==NULL) {
			Py_XDECREF(obCreateTime);
			Py_XDECREF(obAccessTime);
			Py_XDECREF(obWriteTime);
			Py_DECREF(retList);
			::FindClose(hFind);
			return NULL;
		}
		PyObject *obName = PyWinObject_FromWCHAR(findData.cFileName);
		PyObject *obAltName = PyWinObject_FromWCHAR(findData.cAlternateFileName);
		PyObject *newItem = Py_BuildValue("lOOOllllOO",
		// @rdesc The return value is a list of tuples, in the same format as the WIN32_FIND_DATA structure:
			findData.dwFileAttributes, // @tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
			obCreateTime, // @tupleitem 1|<o PyTime>|createTime|File creation time.
    		obAccessTime, // @tupleitem 2|<o PyTime>|accessTime|File access time.
    		obWriteTime, // @tupleitem 3|<o PyTime>|writeTime|Time of last file write
    		findData.nFileSizeHigh, // @tupleitem 4|int|nFileSizeHigh|high order word of file size.
    		findData.nFileSizeLow,	// @tupleitem 5|int|nFileSizeLow|low order word of file size.
    		findData.dwReserved0,	// @tupleitem 6|int|reserved0|Reserved.
    		findData.dwReserved1,   // @tupleitem 7|int|reserved1|Reserved.
    		obName,                 // @tupleitem 8|Unicode|fileName|The name of the file.
    		obAltName);             // @tupleitem 9|Unicode|alternateFilename|Alternative name of the file, expressed in 8.3 format.
		Py_XDECREF(obName);
		Py_XDECREF(obAltName);
		if (newItem!=NULL) {
			PyList_Append(retList, newItem);
			Py_DECREF(newItem);
		}
		// @pyseeapi FindNextFile
		Py_DECREF(obCreateTime);
		Py_DECREF(obAccessTime);
		Py_DECREF(obWriteTime);
		ok=::FindNextFileW(hFind, &findData);
	}
	ok = (GetLastError()==ERROR_NO_MORE_FILES);
	// @pyseeapi FindClose
	::FindClose(hFind);
	if (!ok) {
		Py_DECREF(retList);
		return PyWin_SetAPIError("FindNextFileW");
	}
	return retList;
}
%}

%native(FindFilesW) PyFindFilesW;

// @pyswig |FlushFileBuffers|Clears the buffers for the specified file and causes all buffered data to be written to the file. 
BOOLAPI FlushFileBuffers(
   PyHANDLE hFile 	// @pyparm <o PyHANDLE>|hFile||open handle to file whose buffers are to be flushed 
);

#ifndef MS_WINCE
// @pyswig int|GetBinaryType|Determines whether a file is executable, and if so, what type of executable file it is. That last property determines which subsystem an executable file runs under.
BOOLAPI GetBinaryType(
    TCHAR *lpApplicationName,	// @pyparm <o PyUnicode>|appName||Fully qualified path of file to test
    unsigned long *OUTPUT	// DWORD
   );
#define SCS_32BIT_BINARY SCS_32BIT_BINARY // A Win32-based application
#define SCS_DOS_BINARY SCS_DOS_BINARY // An MS-DOS - based application
#define SCS_OS216_BINARY SCS_OS216_BINARY // A 16-bit OS/2-based application
#define SCS_PIF_BINARY SCS_PIF_BINARY // A PIF file that executes an MS-DOS - based application
#define SCS_POSIX_BINARY SCS_POSIX_BINARY // A POSIX - based application
#define SCS_WOW_BINARY SCS_WOW_BINARY // A 16-bit Windows-based application
#endif // MS_WINCE

//GetCurrentDirectory

#ifndef MS_WINCE
// @pyswig (int, int, int, int)|GetDiskFreeSpace|Determines the free space on a device.
BOOLAPI GetDiskFreeSpace(
    TCHAR *lpRootPathName,	// @pyparm <o PyUnicode>|rootPathName||address of root path
    unsigned long *OUTPUT,
    unsigned long *OUTPUT,
    unsigned long *OUTPUT,
    unsigned long *OUTPUT
// @rdesc The result is a tuple of integers representing (sectors per cluster, bytes per sector, number of free clusters, total number of clusters)
);

// GetDiskFreeSpaceEx	
// @pyswig long, long, long|GetDiskFreeSpaceEx|Determines the free space on a device.
BOOLAPI GetDiskFreeSpaceEx(
    TCHAR *lpRootPathName,	// @pyparm <o PyUnicode>|rootPathName||address of root path
    ULARGE_INTEGER *OUTPUT, 
    ULARGE_INTEGER *OUTPUT,
    ULARGE_INTEGER *OUTPUT 
// @rdesc The result is a tuple of long integers:
// @tupleitem 0|long integer|freeBytes|The total number of free bytes on the disk that are available to the user associated with the calling thread.
// @tupleitem 1|long integer|totalBytes|The total number of bytes on the disk that are available to the user associated with the calling thread.
// Windows 2000: If per-user quotas are in use, this value may be less than the total number of bytes on the disk. 
// @tupleitem 2|long integer|totalFreeBytes|The total number of free bytes on the disk. 
);

// @pyswig int|GetDriveType|Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive. 
long GetDriveType(
    TCHAR *rootPathName
// @rdesc The result is one of the DRIVE_* constants.
);
// @pyswig int|GetDriveTypeW|Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive. (NT/2000 Unicode specific version).
long GetDriveTypeW(
    WCHAR *rootPathName
// @rdesc The result is one of the DRIVE_* constants.
);

#define DRIVE_UNKNOWN DRIVE_UNKNOWN // The drive type cannot be determined.
#define DRIVE_NO_ROOT_DIR DRIVE_NO_ROOT_DIR // The root directory does not exist.
#define DRIVE_REMOVABLE DRIVE_REMOVABLE // The disk can be removed from the drive.
#define DRIVE_FIXED DRIVE_FIXED // The disk cannot be removed from the drive.
#define DRIVE_REMOTE DRIVE_REMOTE // The drive is a remote (network) drive.
#define DRIVE_CDROM DRIVE_CDROM // The drive is a CD-ROM drive.
#define DRIVE_RAMDISK DRIVE_RAMDISK // The drive is a RAM disk.

#endif // MS_WINCE


// @pyswig int|GetFileAttributes|Determines a files attributes.
DWORD GetFileAttributes(
    TCHAR *fileName); // @pyparm <o PyUnicode>|fileName||Name of the file to retrieve attributes for.

// @pyswig int|GetFileAttributesW|Determines a files attributes (NT/2000 Unicode specific version).
DWORD GetFileAttributesW(
    WCHAR *fileName); // @pyparm <o PyUnicode>|fileName||Name of the file to retrieve attributes for.

// @pyswig int|GetFileTime|Determine a file access/modification times.
DWORD GetFileTime(
    HANDLE handle, // @pyparm <o PyHANDLE>|handle||Handle to the file.
	FILETIME *OUTPUT, // @pyparm <o PyTime>|creationTime||
	FILETIME *OUTPUT, // @pyparm <o PyTime>|accessTime||
	FILETIME *OUTPUT // @pyparm <o PyTime>|writeTime||
);

%{
static PyObject *PyObject_FromFILEX_INFO(GET_FILEEX_INFO_LEVELS level, void *p)
{
	switch (level) {
		case GetFileExInfoStandard: {
			WIN32_FILE_ATTRIBUTE_DATA *pa = (WIN32_FILE_ATTRIBUTE_DATA *)p;
			return Py_BuildValue("iNNNN",
			             pa->dwFileAttributes,
						 PyWinObject_FromFILETIME(pa->ftCreationTime),
						 PyWinObject_FromFILETIME(pa->ftLastAccessTime),
						 PyWinObject_FromFILETIME(pa->ftLastWriteTime),
			             PyLong_FromTwoInts(pa->nFileSizeHigh, pa->nFileSizeLow));
			break;
		}
			
		default:
			PyErr_Format(PyExc_RuntimeError, "invalid level for FILEEX_INFO");
			return NULL;
	}
	assert(0); // "not reached";
	return NULL;
}

static PyObject *_DoGetFileAttributesEx(PyObject *self, PyObject *args, BOOL bUnicode)
{
	BOOL ok;
	PyObject *ret = NULL;
	PyObject *obName;
	char *fname = NULL; WCHAR *wname = NULL; // only one of these used.
	int level = (int)GetFileExInfoStandard;
	void *buffer = NULL;
	int nbytes;
	if (!PyArg_ParseTuple(args, "O|i", &obName, &level))
		return NULL;
	if (bUnicode)
		ok = PyWinObject_AsWCHAR(obName, &wname, FALSE);
	else
		ok = PyWinObject_AsString(obName, &fname, FALSE);
	if (!ok)
		goto done;

	switch (level) {
		case GetFileExInfoStandard:
			nbytes = sizeof WIN32_FILE_ATTRIBUTE_DATA;
			break;
		default:
			PyErr_Format(PyExc_ValueError, "Level '%d' is not supported", level);
			goto done;
	}
	buffer = malloc(nbytes);
	if (!buffer) {
		PyErr_Format(PyExc_MemoryError, "Error allocting buffer");
		goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	ok = bUnicode ? 
			GetFileAttributesExW(wname, (GET_FILEEX_INFO_LEVELS)level, buffer) :
			GetFileAttributesEx(fname, (GET_FILEEX_INFO_LEVELS)level, buffer);
	Py_END_ALLOW_THREADS
	if (!ok) {
		PyWin_SetAPIError("GetFileAttributesEx");
		goto done;
	}
	ret = PyObject_FromFILEX_INFO((GET_FILEEX_INFO_LEVELS)level, buffer);
done:
	PyWinObject_FreeString(fname);
	PyWinObject_FreeString(wname);
	if (buffer) free(buffer);
	return ret;
}

// @pyswig tuple|GetFileAttributesEx|Retrieves attributes for a specified file or directory.
// @pyparm string/unicode|filename||String that specifies a file or directory.
// Windows NT/2000: In the ANSI version of this function, the name is limited to 
// MAX_PATH characters. To extend this limit to nearly 32,000 wide characters, 
// call the Unicode version of the function (<om win32file.GetFileAttributesExW>) and prepend 
// "\\?\" to the path.
// @pyparm int|level|GetFileExInfoStandard|An integer that gives the set of attribute information to obtain.
// See the Win32 SDK documentation for more information.
// @rdesc The result is a tuple of:
// @tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
// @tupleitem 1|<o PyTime>|creationTime|Specifies when the file or directory was created. 
// @tupleitem 2|<o PyTime>|lastAccessTime|For a file, specifies when the file was last read from 
// or written to. For a directory, the structure specifies when the directory was created. For 
// both files and directories, the specified date will be correct, but the time of day will 
// always be set to midnight.
// @tupleitem 3|<o PyTime>|lastWriteTime|For a file, the structure specifies when the file was last 
// written to. For a directory, the structure specifies when the directory was created.
// @tupleitem 4|int/long|fileSize|The size of the file. This member has no meaning for directories. 
// @comm Not all file systems can record creation and last access time and not all file systems record 
// them in the same manner. For example, on Windows NT FAT, create time has a resolution of 
// 10 milliseconds, write time has a resolution of 2 seconds, and access time has a resolution 
// of 1 day (really, the access date). On NTFS, access time has a resolution of 1 hour. 
// Furthermore, FAT records times on disk in local time, while NTFS records times on disk in UTC, 
// so it is not affected by changes in time zone or daylight saving time.
static PyObject *PyGetFileAttributesEx(PyObject *self, PyObject *args)
{
	return _DoGetFileAttributesEx(self, args, FALSE);
}
// @pyswig tuple|GetFileAttributesEx|Retrieves attributes for a specified file or directory using Unicode paths.
// @comm See <om win32file.GetFileAttributesEx> for a description of the arguments and return type.
static PyObject *PyGetFileAttributesExW(PyObject *self, PyObject *args)
{
	return _DoGetFileAttributesEx(self, args, TRUE);
}

%}
%native(GetFileAttributesEx) PyGetFileAttributesEx;
%native(GetFileAttributesExW) PyGetFileAttributesExW;


%{
// @pyswig None|SetFileTime|Sets the date and time that a file was created, last accessed, or last modified.
static PyObject *PySetFileTime (PyObject *self, PyObject *args)
{
	PyObject *obHandle;       // @pyparm <o PyHANDLE>/int|handle||Previously opened handle (opened with GENERIC_WRITE access).
	PyObject *obTimeCreated;  // @pyparm <o PyTime>|CreatedTime||File created time. None for no change.
	PyObject *obTimeAccessed; // @pyparm <o PyTime>|AccessTime||File access time. None for no change.
	PyObject *obTimeWritten;  // @pyparm <o PyTime>|WrittenTime||File written time. None for no change.
	HANDLE hHandle;
	FILETIME TimeCreated, *lpTimeCreated;
	FILETIME TimeAccessed, *lpTimeAccessed;
	FILETIME TimeWritten, *lpTimeWritten;
	FILETIME LocalFileTime;
	
	if (!PyArg_ParseTuple(args, "OOOO:SetFileTime",
		&obHandle, &obTimeCreated, &obTimeAccessed, &obTimeWritten))
		return NULL;

    if (!PyWinObject_AsHANDLE(obHandle, &hHandle))
        return NULL;
	if (obTimeCreated == Py_None)
		lpTimeCreated= NULL;
	else
	{
		if (!PyWinObject_AsFILETIME(obTimeCreated, &LocalFileTime))
			return NULL;
		LocalFileTimeToFileTime(&LocalFileTime, &TimeCreated);
		lpTimeCreated= &TimeCreated;
	}
	if (obTimeAccessed == Py_None)
		lpTimeAccessed= NULL;
	else
	{
		if (!PyWinObject_AsFILETIME(obTimeAccessed, &LocalFileTime))
			return NULL;
		LocalFileTimeToFileTime(&LocalFileTime, &TimeAccessed);
		lpTimeAccessed= &TimeAccessed;
	}
	if (obTimeWritten == Py_None)
		lpTimeWritten= NULL;
	else
	{
		if (!PyWinObject_AsFILETIME(obTimeWritten, &LocalFileTime))
			return NULL;
		LocalFileTimeToFileTime(&LocalFileTime, &TimeWritten);
		lpTimeWritten= &TimeWritten;
	}
	if (!::SetFileTime(hHandle, lpTimeCreated, lpTimeAccessed, lpTimeWritten))
		return PyWin_SetAPIError("SetFileTime");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native(SetFileTime) PySetFileTime;

%{
// @pyswig tuple|GetFileInformationByHandle|Retrieves file information for a specified file. 
static PyObject *PyGetFileInformationByHandle(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	BOOL rc;
	BY_HANDLE_FILE_INFORMATION fi;
	// @pyparm <o PyHANDLE>/int|handle||Handle to the file for which to obtain information.<nl>This handle should not be a pipe handle. The GetFileInformationByHandle function does not work with pipe handles.
	if (!PyArg_ParseTuple(args, "O", &obHandle))
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	memset(&fi, 0, sizeof(fi));
	rc = GetFileInformationByHandle(handle, &fi);
	Py_END_ALLOW_THREADS
	if (!rc)
		return PyWin_SetAPIError("GetFileInformationByHandle");
	// @rdesc The result is a tuple of:
	return Py_BuildValue("iNNNiiiiii",
		fi.dwFileAttributes, // @tupleitem 0|int|dwFileAttributes|
		PyWinObject_FromFILETIME(fi.ftCreationTime), // @tupleitem 1|<o PyTime>|ftCreationTime|
		PyWinObject_FromFILETIME(fi.ftLastAccessTime),// @tupleitem 2|<o PyTime>|ftLastAccessTime|
		PyWinObject_FromFILETIME(fi.ftLastWriteTime),// @tupleitem 3|<o PyTime>|ftLastWriteTime|
		fi.dwVolumeSerialNumber,// @tupleitem 4|int|dwVolumeSerialNumber|
		fi.nFileSizeHigh,// @tupleitem 5|int|nFileSizeHigh|
		fi.nFileSizeLow,// @tupleitem 6|int|nFileSizeLow|
		fi.nNumberOfLinks,// @tupleitem 7|int|nNumberOfLinks|
		fi.nFileIndexHigh,// @tupleitem 8|int|nFileIndexHigh|
		fi.nFileIndexLow);// @tupleitem 9|int|nFileIndexLow|
	// @comm Depending on the underlying network components of the operating system and the type of server 
	// connected to, the GetFileInformationByHandle function may fail, return partial information, 
	// or full information for the given file. In general, you should not use GetFileInformationByHandle 
	// unless your application is intended to be run on a limited set of operating system configurations.
}

%}
%native(GetFileInformationByHandle) PyGetFileInformationByHandle;



//GetFileAttributesEx	


#ifndef MS_WINCE
%{
PyObject *MyGetCompressedFileSize(PyObject *self, PyObject *args)
{
	PyObject *obName;
	TCHAR *fname;
	if (!PyArg_ParseTuple(args, "O", &obName))
		return NULL;
	if (!PyWinObject_AsTCHAR(obName, &fname, FALSE))
		return NULL;
	DWORD dwSizeLow, dwSizeHigh;
    Py_BEGIN_ALLOW_THREADS
	dwSizeLow = GetCompressedFileSize(fname, &dwSizeHigh);
    Py_END_ALLOW_THREADS
	// If we failed ... 
	if (dwSizeLow == 0xFFFFFFFF && 
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("GetCompressedFileSize");
	return PyLong_FromTwoInts(dwSizeHigh, dwSizeLow);
}
%}
// @pyswig <o PyLARGE_INTEGER>|GetCompressedFileSize|Determines the compressed size of a file.
%native(GetCompressedFileSize) MyGetCompressedFileSize;

#endif
%{
PyObject *MyGetFileSize(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	if (!PyArg_ParseTuple(args, "O", &obHandle))
		return NULL;
	HANDLE hFile;
	if (!PyWinObject_AsHANDLE(obHandle, &hFile))
		return NULL;
	DWORD dwSizeLow=0, dwSizeHigh=0;
    Py_BEGIN_ALLOW_THREADS
	dwSizeLow = GetFileSize (hFile, &dwSizeHigh);
    Py_END_ALLOW_THREADS
	// If we failed ... 
	if (dwSizeLow == 0xFFFFFFFF && 
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("GetFileSize");
	return PyLong_FromTwoInts(dwSizeHigh, dwSizeLow);
}

%}
// @pyswig <o PyLARGE_INTEGER>|GetFileSize|Determines the size of a file.
%native(GetFileSize) MyGetFileSize;

// @object PyOVERLAPPEDReadBuffer|An alias for a standard Python buffer object.
// Previous versions of the Windows extensions had a custom object for
// holding a read buffer.  This has been replaced with the standard Python buffer object.
// <nl>Python does not provide a method for creating a read-write buffer
// of arbitary size, so currently this can only be created by <om win32file.AllocateReadBuffer>.
#ifndef MS_WINCE
%{
// @pyswig <o PyOVERLAPPEDReadBuffer>|AllocateReadBuffer|Allocated a buffer which can be used with an overlapped Read operation using <om win32file.Read>
PyObject *MyAllocateReadBuffer(PyObject *self, PyObject *args)
{
	int bufSize;
	// @pyparm int|bufSize||The size of the buffer to allocate.
	if (!PyArg_ParseTuple(args, "i", &bufSize))
		return NULL;
	return PyBuffer_New(bufSize);
}
%}

%native(AllocateReadBuffer) MyAllocateReadBuffer;
#endif

%{
// @pyswig (int, string)|ReadFile|Reads a string from a file
// @rdesc The result is a tuple of (hr, string/<o PyOVERLAPPEDReadBuffer>), where hr may be 
// 0, ERROR_MORE_DATA or ERROR_IO_PENDING.
// If the overlapped param is not None, then the result is a <o PyOVERLAPPEDReadBuffer>.  Once the overlapped IO operation
// has completed, you can convert this to a string (str(object))to obtain the data.
// While the operation is in progress, you can use the slice operations (object[:end]) to
// obtain the data read so far.
// You must use the OVERLAPPED API functions to determine how much of the data is valid.
PyObject *MyReadFile(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile;
	HANDLE hFile;
	DWORD bufSize;
	PyObject *obOverlapped = NULL;
	BOOL bBufMallocd = FALSE;
	PyObject *obBuf;

	if (!PyArg_ParseTuple(args, "OO|O:ReadFile", 
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		&obBuf, // @pyparm <o PyOVERLAPPEDReadBuffer>/int|buffer/bufSize||Size of the buffer to create for the read.  If a multi-threaded overlapped operation is performed, a buffer object can be passed.  If a buffer object is passed, the result is the buffer itself.
		&obOverlapped))	// @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
		return NULL;
	// @comm in a multi-threaded overlapped environment, it is likely to be necessary to pre-allocate the read buffer using the <om win32file.AllocateReadBuffer> method, otherwise the I/O operation may complete before you can assign to the resulting buffer.
	if (obOverlapped==NULL)
		pOverlapped = NULL;
	else {
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhFile, &hFile))
		return NULL;

	void *buf = NULL;
	PyObject *pORB = NULL;
	PyBufferProcs *pb = NULL;

	if (PyInt_Check(obBuf)) {
		bufSize = PyInt_AsLong(obBuf);
#ifndef MS_WINCE
		if (pOverlapped) {
			pORB = PyBuffer_New(bufSize);
			if (pORB==NULL) {
				PyErr_SetString(PyExc_MemoryError, "Allocating read buffer");
				return NULL;
			}
			pb = pORB->ob_type->tp_as_buffer;
			(*pb->bf_getreadbuffer)(pORB, 0, &buf);
		} else {
#endif
			buf = malloc(bufSize);
			bBufMallocd = TRUE;
#ifndef MS_WINCE
		}
#endif
		if (buf==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating read buffer");
			return NULL;
		}
	} 
#ifndef MS_WINCE
	else if (obBuf->ob_type->tp_as_buffer){
		pb = obBuf->ob_type->tp_as_buffer;
		pORB = obBuf;
		Py_INCREF(pORB);
		bufSize = (*pb->bf_getreadbuffer)(pORB, 0, &buf);
	}
#endif // MS_WINCE
	 else {
		PyErr_SetString(PyExc_TypeError, "Second param must be an integer or a buffer object");
		return NULL;
	}

	DWORD numRead;
	BOOL ok;
    Py_BEGIN_ALLOW_THREADS
	ok = ReadFile(hFile, buf, bufSize, &numRead, pOverlapped);
    Py_END_ALLOW_THREADS
	DWORD err = 0;
	if (!ok) {
		err = GetLastError();
		if (err!=ERROR_MORE_DATA && err != ERROR_IO_PENDING) {
			Py_XDECREF(pORB);
			if (bBufMallocd)
				free(buf);
			return PyWin_SetAPIError("ReadFile", err);
		}
	}
	PyObject *obRet;
	if (pOverlapped)
		obRet = pORB;
	else
		obRet = PyString_FromStringAndSize((char *)buf, numRead);

	PyObject *result = Py_BuildValue("iO", err, obRet);
	Py_XDECREF(obRet);
	if (bBufMallocd)
		free(buf);
	return result;
}

// @pyswig int, int|WriteFile|Writes a string to a file
// @rdesc The result is a tuple of (errCode, nBytesWritten).  If errCode is not zero,
// it will be ERROR_IO_PENDING (ie, it is an overlapped request).
// <nl>Any other error will raise an exception.
// @comm If you use an overlapped buffer, then it is your responsibility
// to ensure the string object passed remains valid until the operation
// comletes.  If Python garbage collection reclaims the buffer before the
// win32 API has finished with it, the results are unpredictable.
PyObject *MyWriteFile(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile;
	HANDLE hFile;
	char *writeData;
	DWORD writeSize;
	PyObject *obWriteData;
	PyObject *obOverlapped = NULL;
	PyBufferProcs *pb = NULL;

	if (!PyArg_ParseTuple(args, "OO|O:WriteFile", 
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		&obWriteData, // @pyparm string/<o PyOVERLAPPEDReadBuffer>|data||The data to write.
		&obOverlapped))	// @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
		return NULL;
	if (PyString_Check(obWriteData)) {
		writeData = PyString_AsString(obWriteData);
		writeSize = PyString_Size(obWriteData);
	} 
#ifndef MS_WINCE
	else if (obWriteData->ob_type->tp_as_buffer) {
		pb = obWriteData->ob_type->tp_as_buffer;
		writeSize = (*pb->bf_getreadbuffer)(obWriteData, 0, (void **)&writeData);
	} 
#endif // MS_WINCE
	else {
		PyErr_SetString(PyExc_TypeError, "This object can not be written");
		return NULL;
	}
	if (obOverlapped==NULL)
		pOverlapped = NULL;
	else {
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhFile, &hFile))
		return NULL;
	DWORD numWritten;
	BOOL ok;
	DWORD err = 0;
    Py_BEGIN_ALLOW_THREADS
	ok = WriteFile(hFile, writeData, writeSize, &numWritten, pOverlapped);
    Py_END_ALLOW_THREADS
	if (!ok) {
		err = GetLastError();
		if (err != ERROR_IO_PENDING)
			return PyWin_SetAPIError("WriteFile");
	}
	return Py_BuildValue("ll", err, numWritten);
}

// @pyswig |CloseHandle|Closes an open handle.
static PyObject *MyCloseHandle(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	if (!PyArg_ParseTuple(args, "O:CloseHandle",
			&obHandle)) // @pyparm <o PyHANDLE>/int|handle||A previously opened handle.
		return NULL;
	if (!PyWinObject_CloseHANDLE(obHandle))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

#ifndef MS_WINCE
// @pyswig |LockFileEx|locks a file. Wrapper for LockFileEx win32 API.
static PyObject *
MyLockFileEx(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile;
	HANDLE hFile;
	PyObject *obOverlapped = NULL;
    DWORD dwFlags, nbytesLow, nbytesHigh;

	if (!PyArg_ParseTuple(args, "OiiiO:LockFileEx", 
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
        &dwFlags, // @pyparm dwFlags|int||Flags that specify exclusive/shared and blocking/non-blocking mode
        &nbytesLow, // @pyparm nbytesLow|int||low-order part of number of bytes to lock
        &nbytesHigh, // @pyparm nbytesHigh|int||high-order part of number of bytes to lock
		&obOverlapped))	// @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
		return NULL;
	if (obOverlapped==NULL)
		pOverlapped = NULL;
	else {
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhFile, &hFile))
		return NULL;

	BOOL ok;
	DWORD err = 0;
    Py_BEGIN_ALLOW_THREADS
	ok = LockFileEx(hFile, dwFlags, 0, nbytesLow, nbytesHigh, pOverlapped);
    Py_END_ALLOW_THREADS

	if (ok == 0) {
		err = GetLastError();
		return PyWin_SetAPIError("LockFileEx", err);
	}

    Py_INCREF(Py_None);
    return Py_None;
}

// @pyswig |UnlockFileEx|Unlocks a file. Wrapper for UnlockFileEx win32 API.
static PyObject *
MyUnlockFileEx(PyObject *self, PyObject *args)
{
    OVERLAPPED *pOverlapped;
    PyObject *obhFile;
    HANDLE hFile;
    PyObject *obOverlapped = NULL;
    DWORD nbytesLow, nbytesHigh;

    if (!PyArg_ParseTuple(args, "OiiO:UnlockFileEx", 
        &obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
        &nbytesLow, // @pyparm nbytesLow|int||low-order part of number of
                    // bytes to lock
        &nbytesHigh, // @pyparm nbytesLow|int||high-order part of number of
                     // bytes to lock
        &obOverlapped))	// @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
        return NULL;
    if (obOverlapped==NULL)
        pOverlapped = NULL;
    else {
        if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
            return NULL;
    }
    if (!PyWinObject_AsHANDLE(obhFile, &hFile))
        return NULL;

    BOOL ok;
    DWORD err = 0;

    Py_BEGIN_ALLOW_THREADS
    ok = UnlockFileEx(hFile, 0, nbytesLow, nbytesHigh, pOverlapped);
    Py_END_ALLOW_THREADS

    if (ok == 0) {
        err = GetLastError();
        return PyWin_SetAPIError("UnlockFileEx", err);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#endif // MS_WINCE
%}

#ifndef MS_WINCE
%{
// @pyswig (int, int, int, <o PyOVERLAPPED>)|GetQueuedCompletionStatus|Attempts to dequeue an I/O completion packet from a specified input/output completion port.
// @comm This method never throws an API error.
// <nl>The result is a tuple of (rc, numberOfBytesTransferred, completionKey, overlapped)
// <nl>If the function succeeds, rc will be set to 1, otherwise it will be set to the win32 error code.
static PyObject *myGetQueuedCompletionStatus(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	DWORD timeout;
	// @pyparm <o PyHANDLE>|hPort||The handle to the completion port.
	// @pyparm int|timeOut||Timeout in milli-seconds.
	if (!PyArg_ParseTuple(args, "Ol", &obHandle, &timeout))
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;
	DWORD bytes = 0, key = 0;
	OVERLAPPED *pOverlapped = NULL;
	UINT errCode;
    Py_BEGIN_ALLOW_THREADS
	BOOL ok = GetQueuedCompletionStatus(handle, &bytes, &key, &pOverlapped, timeout);
	errCode = ok ? ok : GetLastError();
    Py_END_ALLOW_THREADS
	PyObject *obOverlapped = PyWinObject_FromOVERLAPPED(pOverlapped);
	PyObject *rc = Py_BuildValue("illO", errCode, bytes, key, obOverlapped);
	Py_XDECREF(obOverlapped);
	return rc;
}
%}

%native (GetQueuedCompletionStatus) myGetQueuedCompletionStatus;
#endif // MS_WINCE

%native(ReadFile) MyReadFile;
%native(WriteFile) MyWriteFile;
%native(CloseHandle) MyCloseHandle;

#ifndef MS_WINCE
// @pyswig int|GetFileType|Determines the type of a file.
unsigned long GetFileType( // DWORD
    PyHANDLE hFile // @pyparm <o PyHANDLE>|hFile||The handle to the file.
);
#define FILE_TYPE_UNKNOWN FILE_TYPE_UNKNOWN // The type of the specified file is unknown.
#define FILE_TYPE_DISK FILE_TYPE_DISK // The specified file is a disk file.
#define FILE_TYPE_CHAR FILE_TYPE_CHAR // The specified file is a character file, typically an LPT device or a console.
#define FILE_TYPE_PIPE FILE_TYPE_PIPE // The specified file is either a named or anonymous pipe.
 
#endif // MS_WINCE

// GetFullPathName	

#ifndef MS_WINCE
// @pyswig int|GetLogicalDrives|Returns a bitmaks of the logical drives installed.
unsigned long GetLogicalDrives( // DWORD
);

#endif // MS_WINCE
/**
GetLogicalDriveStrings	
GetShortPathName	
GetTempFileName	
GetTempPath	
GetVolumeInformation	
*/

#ifndef MS_WINCE
// @pyswig int|GetOverlappedResult|Determines the result of the most recent call with an OVERLAPPED object.
// @comm The result is the number of bytes transferred.  The overlapped object's attributes will be changed during this call.
BOOLAPI GetOverlappedResult(
	PyHANDLE hFile, 	// @pyparm <o PyHANDLE>|hFile||The handle to the pipe or file
	OVERLAPPED *lpOverlapped, // @pyparm <o PyOVERLAPPED>|overlapped||The overlapped object to check.
	unsigned long *OUTPUT, // lpNumberOfBytesTransferred
	BOOL bWait	// @pyparm int|bWait||Indicates if the function should wait for data to become available.
);

#endif // MS_WINCE

#ifndef MS_WINCE
// @pyswig |LockFile|Determines the type of a file.
BOOLAPI LockFile(
    PyHANDLE hFile,	// @pyparm <o PyHANDLE>|hFile||handle of file to lock 
    DWORD dwFileOffsetLow,	// @pyparm int|offsetLow||low-order word of lock region offset 
    DWORD dwFileOffsetHigh,	// @pyparm int|offsetHigh||high-order word of lock region offset  
    DWORD nNumberOfBytesToLockLow,	// @pyparm int|nNumberOfBytesToLockLow||low-order word of length to lock 
    DWORD nNumberOfBytesToLockHigh 	// @pyparm int|nNumberOfBytesToLockHigh||high-order word of length to lock 
   );

%native(LockFileEx) MyLockFileEx;

#endif // MS_WINCE


// @pyswig |MoveFile|Renames an existing file or a directory (including all its children). 
BOOLAPI MoveFile(
    TCHAR *lpExistingFileName,	// @pyparm <o PyUnicode>|existingFileName||Name of the existing file  
    TCHAR *lpNewFileName 	// @pyparm <o PyUnicode>|newFileName||New name for the file 
);
// @pyswig |MoveFileW|Renames an existing file or a directory (including all its children). (NT/2000 Unicode specific version).
BOOLAPI MoveFileW(
    WCHAR *lpExistingFileName,	// @pyparm <o PyUnicode>|existingFileName||Name of the existing file  
    WCHAR *lpNewFileName 	// @pyparm <o PyUnicode>|newFileName||New name for the file 
);

#ifndef MS_WINCE
// @pyswig |MoveFileEx|Renames an existing file or a directory (including all its children). 
BOOLAPI MoveFileEx(
    TCHAR *lpExistingFileName,	// @pyparm <o PyUnicode>|existingFileName||Name of the existing file  
    TCHAR *lpNewFileName, 	// @pyparm <o PyUnicode>|newFileName||New name for the file 
    DWORD dwFlags 	        // @pyparm int|flags||flag to determine how to move file 
);
// @pyswig |MoveFileExW|Renames an existing file or a directory (including all its children). (NT/2000 Unicode specific version).
BOOLAPI MoveFileExW(
    WCHAR *lpExistingFileName,	// @pyparm <o PyUnicode>|existingFileName||Name of the existing file  
    WCHAR *lpNewFileName, 	// @pyparm <o PyUnicode>|newFileName||New name for the file 
    DWORD dwFlags 	        // @pyparm int|flags||flag to determine how to move file 
);
#define MOVEFILE_COPY_ALLOWED MOVEFILE_COPY_ALLOWED // If the file is to be moved to a different volume, the function simulates the move by using the CopyFile and DeleteFile functions. Cannot be combined with the MOVEFILE_DELAY_UNTIL_REBOOT flag.
#define MOVEFILE_DELAY_UNTIL_REBOOT MOVEFILE_DELAY_UNTIL_REBOOT // Windows NT only: The function does not move the file until the operating system is restarted. The system moves the file immediately after AUTOCHK is executed, but before creating any paging files. Consequently, this parameter enables the function to delete paging files from previous startups.
#define MOVEFILE_REPLACE_EXISTING MOVEFILE_REPLACE_EXISTING // If a file of the name specified by lpNewFileName already exists, the function replaces its contents with those specified by lpExistingFileName.
#define MOVEFILE_WRITE_THROUGH MOVEFILE_WRITE_THROUGH // Windows NT only: The function does not return until the file has actually been moved on the disk. Setting this flag guarantees that a move perfomed as a copy and delete operation is flushed to disk before the function returns. The flush occurs at the end of the copy operation.<nl>This flag has no effect if the MOVEFILE_DELAY_UNTIL_REBOOT flag is set. 

#endif // MS_WINCE

#ifndef MS_WINCE
// @pyswig <o PyOVERLAPPED>|PostQueuedCompletionStatus|lets you post an I/O completion packet to an I/O completion port. The I/O completion packet will satisfy an outstanding call to the GetQueuedCompletionStatus function.
BOOLAPI PostQueuedCompletionStatus(
  PyHANDLE CompletionPort,  // @pyparm <o PyHANDLE>|handle||handle to an I/O completion port
  DWORD dwNumberOfBytesTransferred,  // @pyparm int|numberOfbytes||value to return via GetQueuedCompletionStatus' first result
  DWORD dwCompletionKey,  // // @pyparm int|completionKey||value to return via GetQueuedCompletionStatus' second result
  OVERLAPPED *lpOverlapped  // @pyparm <o PyOVERLAPPED>|overlapped||value to return via GetQueuedCompletionStatus' third result
);
#endif // MS_WINCE
							 
// QueryDosDevice	
// ReadDirectoryChangesW	
// ReadFileEx	

// @pyswig |RemoveDirectory|Removes an existing directory
BOOLAPI RemoveDirectory(
    TCHAR *lpPathName	// @pyparm <o PyUnicode>|lpPathName||Name of the path to remove.
);

//SearchPath	

#ifndef MS_WINCE
// @pyswig |SetCurrentDirectory|Sets the current directory.
BOOLAPI SetCurrentDirectory(
    TCHAR *lpPathName	// @pyparm <o PyUnicode>|lpPathName||Name of the path to set current.
);
#endif // MS_WINCE

// @pyswig |SetEndOfFile|Moves the end-of-file (EOF) position for the specified file to the current position of the file pointer. 
BOOL SetEndOfFile(
    PyHANDLE hFile	// @pyparm <o PyHANDLE>|hFile||handle of file whose EOF is to be set 
);

#ifndef MS_WINCE
// @pyswig |SetFileApisToANSI|Causes a set of Win32 file functions to use the ANSI character set code page. This function is useful for 8-bit console input and output operations.
void SetFileApisToANSI(void);

// @pyswig |SetFileApisToOEM|Causes a set of Win32 file functions to use the OEM character set code page. This function is useful for 8-bit console input and output operations.
void SetFileApisToOEM(void);
#endif

// @pyswig |SetFileAttributes|Changes a file's attributes.
BOOLAPI SetFileAttributes(
    TCHAR *lpFileName,	// @pyparm <o PyUnicode>|filename||filename 
    DWORD dwFileAttributes 	// @pyparm int|newAttributes||attributes to set 
);	

// @pyswig |SetFileAttributesW|Changes a file's attributes (NT/2000 Unicode specific version)
BOOLAPI SetFileAttributesW(
    WCHAR *lpFileName,	// @pyparm <o PyUnicode>|filename||filename 
    DWORD dwFileAttributes 	// @pyparm int|newAttributes||attributes to set 
);	
 
%{
// @pyswig |SetFilePointer|Moves the file pointer of an open file. 
PyObject *MySetFilePointer(PyObject *self, PyObject *args)
{
	PyObject *obHandle, *obOffset;
	DWORD iMethod;
	HANDLE handle;
	if (!PyArg_ParseTuple(args, "OOl", 
			&obHandle,  // @pyparm <o PyHANDLE>|handle||The file to perform the operation on.
			&obOffset, // @pyparm <o Py_LARGEINTEGER>|offset||Offset to move the file pointer.
			&iMethod)) // @pyparm int|moveMethod||Starting point for the file pointer move. This parameter can be one of the following values.
			              // @flagh Value|Meaning 
			              // @flag FILE_BEGIN|The starting point is zero or the beginning of the file. 
			              // @flag FILE_CURRENT|The starting point is the current value of the file pointer. 
			              // @flag FILE_END|The starting point is the current end-of-file position. 

		return NULL;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;
	long offHigh;
	unsigned offLow;
	if (!PyLong_AsTwoInts(obOffset, (int *)&offHigh, &offLow))
		return NULL;
    Py_BEGIN_ALLOW_THREADS
	offLow = SetFilePointer(handle, offLow, &offHigh, iMethod);
    Py_END_ALLOW_THREADS
	// If we failed ... 
	if (offLow == 0xFFFFFFFF && 
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("SetFilePointer");
	return PyLong_FromTwoInts(offHigh, offLow);
}
%}
%native(SetFilePointer) MySetFilePointer;

#define FILE_BEGIN FILE_BEGIN
#define FILE_END FILE_END
#define FILE_CURRENT FILE_CURRENT

#ifndef MS_WINCE
// @pyswig |SetVolumeLabel|Sets a volume label for a disk drive.
BOOLAPI SetVolumeLabel(
    TCHAR *lpRootPathName,	// @pyparm <o PyUnicode>|rootPathName||address of name of root directory for volume 
    TCHAR *lpVolumeName 	// @pyparm <o PyUnicode>|volumeName||name for the volume 
   );

// @pyswig |UnlockFile|Determines the type of a file.
BOOLAPI UnlockFile(
    PyHANDLE hFile,	// @pyparm <o PyHANDLE>|hFile||handle of file to unlock 
    DWORD dwFileOffsetLow,	// @pyparm int|offsetLow||low-order word of lock region offset 
    DWORD dwFileOffsetHigh,	// @pyparm int|offsetHigh||high-order word of lock region offset  
    DWORD nNumberOfBytesToUnlockLow,	// @pyparm int|nNumberOfBytesToUnlockLow||low-order word of length to unlock 
    DWORD nNumberOfBytesToUnlockHigh 	// @pyparm int|nNumberOfBytesToUnlockHigh||high-order word of length to unlock 
   );

%native(UnlockFileEx) MyUnlockFileEx;
#endif // MS_WINCE

// File Handle / File Descriptor APIs.
#ifndef MS_WINCE
// @pyswig long|_get_osfhandle|Gets operating-system file handle associated with existing stream
%name(_get_osfhandle)
PyObject *myget_osfhandle( int filehandle );

// @pyswig int|_open_osfhandle|Associates a C run-time file handle with a existing operating-system file handle.
%name(_open_osfhandle)
PyObject *myopen_osfhandle ( PyHANDLE osfhandle, int flags );


%{
PyObject *myget_osfhandle (int filehandle)
{
  long result = _get_osfhandle (filehandle);
  if (result == -1)
    return PyErr_SetFromErrno(PyExc_IOError);

  return Py_BuildValue ("l",result);
}

PyObject *myopen_osfhandle (PyHANDLE osfhandle, int flags)
{
  int result = _open_osfhandle ((long) osfhandle, flags);
  if (result == -1)
    return PyErr_SetFromErrno(PyExc_IOError);

  return Py_BuildValue ("i",result);
}

%}

// Overlapped Socket stuff
%{
#pragma comment(lib,"mswsock.lib") // too lazy to change the project file :-)
#pragma comment(lib,"ws2_32.lib")
%}

%native(AcceptEx) MyAcceptEx;

%native(GetAcceptExSockaddrs) MyGetAcceptExSockaddrs;

%{
// @pyswig |AcceptEx|Version of accept that uses Overlapped I/O
static PyObject *MyAcceptEx
(
	PyObject *self,
	PyObject *args
)
{
	OVERLAPPED *pOverlapped = NULL;
	SOCKET sListening;
	SOCKET sAccepting;
	PyObject *obOverlapped = NULL;
	DWORD dwBufSize = 0;
	PyObject *rv = NULL;
	PyObject *obListening = NULL;
	PyObject *obAccepting = NULL;
	PyObject *obBuf = NULL;
	PyObject *pORB = NULL;
	void *buf = NULL;
	DWORD cBytesRecvd = 0;
	int rc;
	int iMinBufferSize = (sizeof(SOCKADDR_IN) + 16) * 2;
	WSAPROTOCOL_INFO wsProtInfo;
	UINT cbSize = sizeof(wsProtInfo);
	PyBufferProcs *pb = NULL;

	if (!PyArg_ParseTuple(
		args,
		"OOOO:AcceptEx",
		&obListening, // @pyparm <o PySocket>/int|sListening||Socket that had listen() called on.
		&obAccepting, // @pyparm <o PySocket>/int|sAccepting||Socket that will be used as the incoming connection.
		&obBuf, // @pyparm <o buffer>|buffer||Buffer to read incoming data and connection point information into. This buffer MUST be big enough to recieve your connection endpoints... AF_INET sockets need to be at least 64 bytes. The correct minimum of the buffer is determined by the protocol family that the listening socket is using.
		&obOverlapped)) // @pyparm <o PyOVERLAPPED>|ol||An overlapped structure
	{
		return NULL;
	}

	// @comm In order to make sure the connection has been accepted, either use the hEvent in PyOVERLAPPED, GetOverlappedResult, or GetQueuedCompletionStatus.
	// @comm To use this with I/O completion ports, don't forget to attach sAccepting to your completion port.
	// @comm To have sAccepting inherit the properties of sListening, you need to do the following after a connection is successfully accepted:
	// @comm import struct
	// @comm sAccepting.setsockopt(socket.SOL_SOCKET, win32file.SO_UPDATE_ACCEPT_CONTEXT, struct.pack("I", sListening.fileno()))

	if (!PySocket_AsSOCKET(obListening, &sListening))
	{
		return NULL;
	}

	// Grab the protocol information for the socket
	// So we can compute the correct minimum buffer size.
	Py_BEGIN_ALLOW_THREADS
	rc = getsockopt(
		sListening,
		SOL_SOCKET,
		SO_PROTOCOL_INFO,
		(char *)&wsProtInfo,
		(int *)&cbSize);
	Py_END_ALLOW_THREADS
	if (rc == SOCKET_ERROR)
	{
		PyWin_SetAPIError("AcceptEx", WSAGetLastError());
		return NULL;
	}
	iMinBufferSize = (wsProtInfo.iMaxSockAddr + 16) * 2;

	if (!PySocket_AsSOCKET(obAccepting, &sAccepting))
	{
		return NULL;
	}

	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
	{
		return NULL;
	}

	if (obBuf->ob_type->tp_as_buffer)
	{
		pORB = obBuf;
		Py_INCREF(pORB);
		pb = pORB->ob_type->tp_as_buffer;
		dwBufSize = (*pb->bf_getreadbuffer)(pORB, 0, &buf);
		if (dwBufSize < (DWORD)iMinBufferSize )
		{
			PyErr_Format(
				PyExc_ValueError,
				"Second param must be at least %ld bytes long",
				iMinBufferSize);
			goto Error;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "Second param must be a buffer object");
		return NULL;
	}

	// Phew... finally, all the arguments are converted...
	Py_BEGIN_ALLOW_THREADS
	rc = AcceptEx(
		sListening,
		sAccepting,
		buf,
		dwBufSize - iMinBufferSize,
		wsProtInfo.iMaxSockAddr + 16,
		wsProtInfo.iMaxSockAddr + 16,
		&cBytesRecvd,
		pOverlapped);
	Py_END_ALLOW_THREADS
	if (!rc)
	{
		rc = WSAGetLastError();
		if (rc != ERROR_IO_PENDING)
		{
			PyWin_SetAPIError("AcceptEx", WSAGetLastError());
			goto Error;
		}
	}

	Py_DECREF(pORB);
	Py_INCREF(Py_None);
	rv = Py_None;
Cleanup:
	return rv;
Error:
	Py_DECREF(pORB);
	rv = NULL;
	goto Cleanup;
}

static PyObject *
MyMakeIPAddr(SOCKADDR_IN *addr)
{
	long x = ntohl(addr->sin_addr.s_addr);
	char buf[100];
	sprintf(buf, "%d.%d.%d.%d",
		(int) (x>>24) & 0xff, (int) (x>>16) & 0xff,
		(int) (x>> 8) & 0xff, (int) (x>> 0) & 0xff);
	return PyString_FromString(buf);
}

static PyObject *
MyMakeSockaddr(SOCKADDR *addr, INT cbAddr)
{
	if (cbAddr == 0)
	{
		/* No address -- may be recvfrom() from known socket */
		Py_INCREF(Py_None);
		return Py_None;
	}

	switch (addr->sa_family) {
	case AF_INET:
	{
		SOCKADDR_IN *a = (SOCKADDR_IN *) addr;
		PyObject *addrobj = MyMakeIPAddr(a);
		PyObject *ret = NULL;
		if (addrobj) {
			ret = Py_BuildValue("Oi", addrobj, ntohs(a->sin_port));
			Py_DECREF(addrobj);
		}
		return ret;
	}

	/* More cases here... */

	default:
		/* If we don't know the address family, don't raise an
		   exception -- return it as a tuple. */
		return Py_BuildValue("is#",
				     addr->sa_family,
				     addr->sa_data,
				     sizeof(addr->sa_data));

	}
}



// @pyswig (iFamily, <o LocalSockAddr>, <o RemoteSockAddr>)|GetAcceptExSockaddrs|Parses the connection endpoints from the buffer passed into AcceptEx
PyObject *MyGetAcceptExSockaddrs
(
	PyObject *self,
	PyObject *args
)
{
	PyObject *rv = NULL;
	PyObject *obAccepting = NULL;
	PyObject *obBuf = NULL;
	SOCKET sAccepting;
	int iMinBufferSize = (sizeof(SOCKADDR_IN) + 16) * 2;
	WSAPROTOCOL_INFO wsProtInfo;
	UINT cbSize = sizeof(wsProtInfo);
	SOCKADDR *psaddrLocal = NULL;
	SOCKADDR *psaddrRemote = NULL;
	void *buf = NULL;
	PyObject *pORB = NULL;
	INT cbLocal = 0;
	INT cbRemote = 0;
	SOCKADDR_IN *psaddrIN = NULL;
	PyObject *obTemp = NULL;
	int rc;
	DWORD dwBufSize;
	PyBufferProcs *pb = NULL;

	if (!PyArg_ParseTuple(
		args,
		"OO:GetAcceptExSockaddrs",
		&obAccepting, // @pyparm <o PySocket>/int|sAccepting||Socket that was passed into the sAccepting parameter of AcceptEx
		&obBuf)) // @pyparm <o PyOVERLAPPEDReadBuffer>|buffer||Buffer you passed into AcceptEx 
	{
		return NULL;
	}

	if (!PySocket_AsSOCKET(obAccepting, &sAccepting))
	{
		return NULL;
	}

	// Grab the protocol information for the socket
	// So we can compute the correct minimum buffer size.
	Py_BEGIN_ALLOW_THREADS
	rc = getsockopt(
		sAccepting,
		SOL_SOCKET,
		SO_PROTOCOL_INFO,
		(char *)&wsProtInfo,
		(int *)&cbSize);
	Py_END_ALLOW_THREADS
	if (rc == SOCKET_ERROR)
	{
			PyWin_SetAPIError("AcceptEx", WSAGetLastError());
			return NULL;
	}
	iMinBufferSize = (wsProtInfo.iMaxSockAddr + 16) * 2;

	if (obBuf->ob_type->tp_as_buffer)
	{
		pORB = obBuf;
		Py_INCREF(pORB);
		pb = pORB->ob_type->tp_as_buffer;
		dwBufSize = (*pb->bf_getreadbuffer)(pORB, 0, &buf);
		if (dwBufSize < (DWORD)iMinBufferSize )
		{
			PyErr_Format(
				PyExc_ValueError,
				"Second param must be at least %ld bytes long",
				iMinBufferSize);
			goto Error;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "Second param must be a buffer object");
		return NULL;
	}

	cbRemote = cbLocal = wsProtInfo.iMaxSockAddr + 16;
	Py_BEGIN_ALLOW_THREADS
	GetAcceptExSockaddrs(
		buf,
		dwBufSize - iMinBufferSize,
		cbLocal,
		cbRemote,
		&psaddrLocal,
		&cbLocal,
		&psaddrRemote,
		&cbRemote);
	Py_END_ALLOW_THREADS

	// Now construct the return value.
	rv = PyTuple_New(3);
	if (rv == NULL)
	{
		return NULL;
	}

	//@comm LocalSockAddr and RemoteSockAddr are ("xx.xx.xx.xx", port#) if iFamily == AF_INET
	//@comm otherwise LocalSockAddr and RemoteSockAddr are just binary strings
	//@comm and they should be unpacked with the struct module.

	// Stick in sa_family.
	obTemp = PyInt_FromLong((LONG)psaddrLocal->sa_family);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 0, obTemp);
	obTemp = NULL;

	// Construct local address.
	obTemp = MyMakeSockaddr(psaddrLocal, cbLocal);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 1, obTemp);
	obTemp = NULL;

	// Construct remote address.
	obTemp = MyMakeSockaddr(psaddrRemote, cbRemote);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 2, obTemp);
	obTemp = NULL;
	
Cleanup:
	return rv;
Error:
	Py_DECREF(rv);
	rv = NULL;
	goto Cleanup;
}


%}

%{
PyObject* MyWSAEventSelect
(
	SOCKET *s, 
	PyHANDLE hEvent,
	LONG lNetworkEvents
)
{
	int rc;
	Py_BEGIN_ALLOW_THREADS;
	rc = WSAEventSelect(*s, hEvent, lNetworkEvents);
	Py_END_ALLOW_THREADS;
	if (rc == SOCKET_ERROR)
	{
		PyWin_SetAPIError("WSAEventSelect", WSAGetLastError());
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

%}

// @pyswig |WSAEventSelect|Specifies an event object to be associated with the supplied set of FD_XXXX network events.
%name(WSAEventSelect) PyObject *MyWSAEventSelect
(
	SOCKET *s, // @pyparm <o PySocket>|socket||socket to attach to the event
	PyHANDLE hEvent, // @pyparm <o PyHandle>|hEvent||Event handle for the socket to become attached to.
	LONG lNetworkEvents // @pyparm int|networkEvents||A bitmask of network events that will cause hEvent to be signaled. e.g. (FD_CLOSE \| FD_READ)
);
%{

PyObject* MyWSAAsyncSelect
(
	SOCKET *s, 
	LONG hwnd,
	LONG wMsg,
	LONG lNetworkEvents
)
{
	int rc;
	Py_BEGIN_ALLOW_THREADS;
	rc = WSAAsyncSelect(*s, (HWND)hwnd, wMsg, lNetworkEvents);
	Py_END_ALLOW_THREADS;
	if (rc == SOCKET_ERROR)
	{
		PyWin_SetAPIError("WSAAsyncSelect", WSAGetLastError());
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

%}

// @pyswig |WSAEventSelect|Specifies an event object to be associated with the supplied set of FD_XXXX network events.
%name(WSAAsyncSelect) PyObject *MyWSAAsyncSelect
(
	SOCKET *s, // @pyparm <o PySocket>|socket||socket to attach to the event
	LONG hwnd, // @pyparm <o hwnd>|hwnd||Window handle for the socket to become attached to.
	LONG wMsg, // @pyparm <o int>|int||Window message that will be posted.
	LONG lNetworkEvents // @pyparm int|networkEvents||A bitmask of network events that will cause wMsg to be posted. e.g. (FD_CLOSE \| FD_READ)
);

%native(WSASend) MyWSASend;
%native(WSARecv) MyWSARecv;

%{
// @pyswig (rc, cBytesSent)|WSASend|Winsock send() equivalent function for Overlapped I/O.
PyObject *MyWSASend
(
	PyObject *self,
	PyObject *args
)
{
	SOCKET s;
	PyObject *obSocket = NULL;
	WSABUF wsBuf;
	DWORD cbSent = 0;
	OVERLAPPED *pOverlapped = NULL;
	int rc = 0;
	PyObject *rv = NULL;
	PyObject *obTemp = NULL;
	PyObject *obBuf = NULL;
	PyObject *obOverlapped = NULL;
	DWORD dwFlags = 0;
	PyBufferProcs *pb = NULL;

	if (!PyArg_ParseTuple(
		args,
		"OOO|i:WSASend",
		&obSocket, // @pyparm <o PySocket>/int|s||Socket to send data on.
		&obBuf, // @pyparm string/<o buffer>|buffer||Buffer to send data from.
		&obOverlapped, // @pyparm <o PyOVERLAPPED>|ol||An overlapped structure
		&dwFlags)) // @pyparm int|dwFlags||Optional send flags.
	{
		return NULL;
	}

	if (!PySocket_AsSOCKET(obSocket, &s))
	{
		return NULL;
	}

	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
	{
		return NULL;
	}

	if (PyString_Check(obBuf))
	{
		wsBuf.buf = PyString_AS_STRING(obBuf);
		wsBuf.len = PyString_GET_SIZE(obBuf);
	}
	else if (obBuf->ob_type->tp_as_buffer)
	{
		Py_INCREF(obBuf);
		pb = obBuf->ob_type->tp_as_buffer;
		wsBuf.len = (*pb->bf_getreadbuffer)(obBuf, 0, (void **)&wsBuf.buf);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "Second param must be a buffer object or a string.");
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS;
	rc = WSASend(
		s,
		&wsBuf,
		1,
		&cbSent,
		dwFlags,
		pOverlapped,
		NULL);
	Py_END_ALLOW_THREADS;

	if (rc == SOCKET_ERROR)
	{
		rc = WSAGetLastError();
		if (rc != ERROR_IO_PENDING)
		{
			PyWin_SetAPIError("WSASend", rc);
			goto Error;
		}
	}

	rv = PyTuple_New(2);
	if (rv == NULL)
	{
		goto Error;
	}

	obTemp = PyInt_FromLong(rc);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 0, obTemp);
	obTemp = NULL;

	obTemp = PyInt_FromLong(cbSent);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 1, obTemp);
	obTemp = NULL;

Cleanup:
	return rv;
Error:
	Py_XDECREF(obBuf);
	Py_XDECREF(rv);
	rv = NULL;
	goto Cleanup;
}

// @pyswig (rc, cBytesRecvd)|WSARecv|Winsock recv() equivalent function for Overlapped I/O.
PyObject *MyWSARecv
(
	PyObject *self,
	PyObject *args
)
{
	SOCKET s;
	PyObject *obSocket = NULL;
	WSABUF wsBuf;
	DWORD cbRecvd = 0;
	OVERLAPPED *pOverlapped = NULL;
	int rc = 0;
	PyObject *rv = NULL;
	PyObject *obTemp = NULL;
	PyObject *obBuf = NULL;
	PyObject *obOverlapped = NULL;
	DWORD dwFlags = 0;
	PyBufferProcs *pb = NULL;

	if (!PyArg_ParseTuple(
		args,
		"OOO|i:WSARecv",
		&obSocket, // @pyparm <o PySocket>/int|s||Socket to send data on.
		&obBuf, // @pyparm <o buffer>|buffer||Buffer to send data from.
		&obOverlapped, // @pyparm <o PyOVERLAPPED>|ol||An overlapped structure
		&dwFlags)) // @pyparm int|dwFlags||Optional reception flags.
	{
		return NULL;
	}

	if (!PySocket_AsSOCKET(obSocket, &s))
	{
		return NULL;
	}

	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
	{
		return NULL;
	}

	if (obBuf->ob_type->tp_as_buffer)
	{
		Py_INCREF(obBuf);
		pb = obBuf->ob_type->tp_as_buffer;
		wsBuf.len = (*pb->bf_getreadbuffer)(obBuf, 0, (void **)&wsBuf.buf);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "Second param must be a PyOVERLAPPEDReadBuffer object");
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS;
	rc = WSARecv(
		s,
		&wsBuf,
		1,
		&cbRecvd,
		&dwFlags,
		pOverlapped,
		NULL);
	Py_END_ALLOW_THREADS;

	if (rc == SOCKET_ERROR)
	{
		rc = WSAGetLastError();
		if (rc != ERROR_IO_PENDING)
		{
			PyWin_SetAPIError("WSASend", rc);
			goto Error;
		}
	}

	rv = PyTuple_New(2);
	if (rv == NULL)
	{
		goto Error;
	}

	obTemp = PyInt_FromLong(rc);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 0, obTemp);
	obTemp = NULL;

	obTemp = PyInt_FromLong(cbRecvd);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 1, obTemp);
	obTemp = NULL;

Cleanup:
	return rv;
Error:
	Py_DECREF(obBuf);
	Py_XDECREF(rv);
	rv = NULL;
	goto Cleanup;
}


%}

#define SO_UPDATE_ACCEPT_CONTEXT SO_UPDATE_ACCEPT_CONTEXT
#define SO_CONNECT_TIME SO_CONNECT_TIME

#define WSAEWOULDBLOCK WSAEWOULDBLOCK
#define WSAENETDOWN WSAENETDOWN
#define WSAENOTCONN WSAENOTCONN
#define WSAEINTR WSAEINTR
#define WSAEINPROGRESS WSAEINPROGRESS
#define WSAENETRESET WSAENETRESET
#define WSAENOTSOCK WSAENOTSOCK
#define WSAEFAULT WSAEFAULT
#define WSAEOPNOTSUPP WSAEOPNOTSUPP
#define WSAESHUTDOWN WSAESHUTDOWN
#define WSAEMSGSIZE WSAEMSGSIZE
#define WSAEINVAL WSAEINVAL
#define WSAECONNABORTED WSAECONNABORTED
#define WSAECONNRESET WSAECONNRESET
#define WSAEDISCON WSAEDISCON
#define WSA_IO_PENDING WSA_IO_PENDING
#define WSA_OPERATION_ABORTED WSA_OPERATION_ABORTED
#define FD_READ FD_READ
#define FD_WRITE FD_WRITE
#define FD_OOB FD_OOB
#define FD_ACCEPT FD_ACCEPT
#define FD_CONNECT FD_CONNECT
#define FD_CLOSE FD_CLOSE
#define FD_QOS FD_QOS
#define FD_GROUP_QOS FD_GROUP_QOS
#define FD_ROUTING_INTERFACE_CHANGE FD_ROUTING_INTERFACE_CHANGE
#define FD_ADDRESS_LIST_CHANGE FD_ADDRESS_LIST_CHANGE

#endif // MS_WINCE

// The communications related functions.
// The COMM port enhancements were added by Mark Hammond, and are
// (c) 2000-2001, ActiveState Tools Corp.

%{
// The comms port helpers.
extern PyObject *PyWinObject_FromCOMSTAT(const COMSTAT *pCOMSTAT);
extern BOOL PyWinObject_AsCOMSTAT(PyObject *ob, COMSTAT **ppCOMSTAT, BOOL bNoneOK = TRUE);
extern BOOL PyWinObject_AsDCB(PyObject *ob, DCB **ppDCB, BOOL bNoneOK = TRUE);
extern PyObject *PyWinObject_FromDCB(const DCB *pDCB);
extern PyObject *PyWinMethod_NewDCB(PyObject *self, PyObject *args);
extern PyObject *PyWinObject_FromCOMMTIMEOUTS( COMMTIMEOUTS *p);
extern BOOL PyWinObject_AsCOMMTIMEOUTS( PyObject *ob, COMMTIMEOUTS *p);

%}

%native (DCB) PyWinMethod_NewDCB;

%typemap(python,in) DCB *
{
	if (!PyWinObject_AsDCB($source, &$target, TRUE))
		return NULL;
}
%typemap(python,argout) DCB *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromDCB($source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}
%typemap(python,ignore) DCB *OUTPUT(DCB temp)
{
  $target = &temp;
  $target->DCBlength = sizeof( DCB ) ;
}

%typemap(python,in) COMSTAT *
{
	if (!PyWinObject_AsCOMSTAT($source, &$target, TRUE))
		return NULL;
}
%typemap(python,argout) COMSTAT *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromCOMSTAT(*$source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}
%typemap(python,ignore) COMSTAT *OUTPUT(COMSTAT temp)
{
  $target = &temp;
}


%typemap(python,in) COMMTIMEOUTS *(COMMTIMEOUTS temp)
{
	$target = &temp;
	if (!PyWinObject_AsCOMMTIMEOUTS($source, $target))
		return NULL;
}

%typemap(python,argout) COMMTIMEOUTS *OUTPUT {
    PyObject *o;
    o = PyWinObject_FromCOMMTIMEOUTS($source);
    if (!$target) {
      $target = o;
    } else if ($target == Py_None) {
      Py_DECREF(Py_None);
      $target = o;
    } else {
      if (!PyList_Check($target)) {
	PyObject *o2 = $target;
	$target = PyList_New(0);
	PyList_Append($target,o2);
	Py_XDECREF(o2);
      }
      PyList_Append($target,o);
      Py_XDECREF(o);
    }
}
%typemap(python,ignore) COMMTIMEOUTS *OUTPUT(COMMTIMEOUTS temp)
{
  $target = &temp;
}


// @pyswig <o PyDCB>|BuildCommDCB|Fills the specified DCB structure with values specified in a device-control string. The device-control string uses the syntax of the mode command
BOOLAPI BuildCommDCB(
  TCHAR *lpDef,  // @pyparm string|def||device-control string
  DCB *OUTOUT     // @pyparm <o PyDCB>|dcb||The device-control block
);

%{
// @pyswig int, <o PyCOMSTAT>|ClearCommError|retrieves information about a communications error and reports the current status of a communications device.
static PyObject *PyClearCommError(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	// @pyparm handle|<o PyHANDLE>||A handle to the device.
	if (!PyArg_ParseTuple(args, "O", &obHandle))
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;
	BOOL rc;
	DWORD int_ret;
	COMSTAT stat;
	Py_BEGIN_ALLOW_THREADS;
	rc = ClearCommError(handle, &int_ret, &stat);
	Py_END_ALLOW_THREADS;
	if (!rc)
		return PyWin_SetAPIError("ClearCommError");
	PyObject *obStat = PyWinObject_FromCOMSTAT(&stat);
	PyObject *ret = Py_BuildValue("iO", int_ret, obStat);
	Py_XDECREF(obStat);
	return ret;
}

%}
%native (ClearCommError) PyClearCommError;

// @pyswig |EscapeCommFunction|directs a specified communications device to perform an extended function. 
BOOLAPI EscapeCommFunction(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	int func // int|func||Specifies the code of the extended function to perform. This parameter can be one of the following values. 
	// @flagh Value|Meaning 
	// @flag CLRDTR|Clears the DTR (data-terminal-ready) signal. 
	// @flag CLRRTS|Clears the RTS (request-to-send) signal. 
	// @flag SETDTR|Sends the DTR (data-terminal-ready) signal. 
	// @flag SETRTS|Sends the RTS (request-to-send) signal. 
	// @flag SETXOFF|Causes transmission to act as if an XOFF character has been received. 
	// @flag SETXON|Causes transmission to act as if an XON character has been received. 
	// @flag SETBREAK|Suspends character transmission and places the transmission line in a break state until the ClearCommBreak function is called (or EscapeCommFunction is called with the CLRBREAK extended function code). The SETBREAK extended function code is identical to the SetCommBreak function. Note that this extended function does not flush data that has not been transmitted. 
	// @flag CLRBREAK|Restores character transmission and places the transmission line in a nonbreak state. The CLRBREAK extended function code is identical to the ClearCommBreak function. 
);

// @pyswig <o PyDCB>|GetCommState|Returns a device-control block (a DCB structure) with the current control settings for a specified communications device.
BOOLAPI GetCommState(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	DCB *OUTPUT
);
// @pyswig |SetCommState|Configures a communications device according to the specifications in a device-control block.
// The function reinitializes all hardware and control settings, but it does not empty output or input queues.
BOOLAPI SetCommState(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	DCB *dcb // @pyparm <o PyDCB>|dcb||The control settings.
);

// @pyswig |ClearCommBreak|Restores character transmission for a specified communications device and places the transmission line in a nonbreak state
BOOLAPI ClearCommBreak(
	PyHANDLE handle // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
);

// @pyswig int|GetCommMask|Retrieves the value of the event mask for a specified communications device.
BOOLAPI GetCommMask(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	unsigned long *OUTPUT
);

// @pyswig int|SetCommMask|Sets the value of the event mask for a specified communications device.
BOOLAPI SetCommMask(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	unsigned long val // @pyparm int|val||The new mask value.
);

// @pyswig int|GetCommModemStatus|Retrieves modem control-register values. 
BOOLAPI GetCommModemStatus(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	unsigned long *OUTPUT
);

// @pyswig <o PyCOMMTIMEOUTS>|GetCommTimeouts|Retrieves the time-out parameters for all read and write operations on a specified communications device. 
BOOLAPI GetCommTimeouts(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	COMMTIMEOUTS *OUTPUT
);

// @pyswig int|SetCommTimeouts|Sets the time-out parameters for all read and write operations on a specified communications device. 
BOOLAPI SetCommTimeouts(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	COMMTIMEOUTS *timeouts // @pyparm <o PyCOMMTIMEOUTS>|val||The new time-out parameters.
);

// @pyswig |PurgeComm|Discards all characters from the output or input buffer of a specified communications resource. It can also terminate pending read or write operations on the resource. 
BOOLAPI PurgeComm(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	unsigned long val // @pyparm int|action||The action to perform.  This parameter can be one or more of the following values.
	// @flagh Value|Meaning 
	// @flag PURGE_TXABORT|Terminates all outstanding overlapped write operations and returns immediately, even if the write operations have not been completed. 
	// @flag PURGE_RXABORT|Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed. 
	// @flag PURGE_TXCLEAR|Clears the output buffer (if the device driver has one). 
	// @flag PURGE_RXCLEAR|Clears the input buffer (if the device driver has one). 
);

// @pyswig |SetCommBreak|Suspends character transmission for a specified communications device and places the transmission line in a break state until the <om win32file.ClearCommBreak> function is called. 
BOOLAPI SetCommBreak(
	PyHANDLE handle // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
);

// @pyswig |SetupComm|Initializes the communications parameters for a specified communications device. 
BOOLAPI SetupComm(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	unsigned long dwInQueue, // @pyparm int|dwInQueue||Specifies the recommended size, in bytes, of the device's internal input buffer.
	unsigned long dwOutQueue // @pyparm int|dwOutQueue||Specifies the recommended size, in bytes, of the device's internal output buffer.
);

// @pyswig |TransmitCommChar|Transmits a specified character ahead of any pending data in the output buffer of the specified communications device.
BOOLAPI TransmitCommChar(
	PyHANDLE handle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
	char ch // @pyparm char|cChar||The character to transmit.
// @comm The TransmitCommChar function is useful for sending an interrupt character (such as a CTRL+C) to a host system. 
// <nl>If the device is not transmitting, TransmitCommChar cannot be called repeatedly. Once TransmitCommChar places a character in the output buffer, the character must be transmitted before the function can be called again. If the previous character has not yet been sent, TransmitCommChar returns an error.
);

%{
static PyObject *MyWaitCommEvent(PyObject *self, PyObject *args)
{
	PyObject *obHandle, *obOverlapped = Py_None;
	if (!PyArg_ParseTuple(args, "O|O", 
			&obHandle, // @pyparm <o PyHANDLE>|handle||The handle to the communications device.
			&obOverlapped))// @pyparm <o PyOVERLAPPED>|overlapped||This structure is required if hFile was opened with FILE_FLAG_OVERLAPPED. 
			// <nl>If hFile was opened with FILE_FLAG_OVERLAPPED, the lpOverlapped parameter must not be NULL. It must point to a valid OVERLAPPED structure. If hFile was opened with FILE_FLAG_OVERLAPPED and lpOverlapped is NULL, the function can incorrectly report that the operation is complete. 
			// <nl>If hFile was opened with FILE_FLAG_OVERLAPPED and lpOverlapped is not NULL, WaitCommEvent is performed as an overlapped operation. In this case, the OVERLAPPED structure must contain a handle to a manual-reset event object (created by using the CreateEvent function). 
			// <nl>If hFile was not opened with FILE_FLAG_OVERLAPPED, WaitCommEvent does not return until one of the specified events or an error occurs. 
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle, FALSE))
		return NULL;
	PyOVERLAPPED *pyoverlapped;
	if (!PyWinObject_AsPyOVERLAPPED(obOverlapped, &pyoverlapped, TRUE))
		return NULL;
	DWORD mask, *pmask;
	if (pyoverlapped)
		pmask = &pyoverlapped->m_overlapped.dwValue;
	else
		pmask = &mask;

	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = WaitCommEvent(handle, pmask, 
	                   pyoverlapped ? pyoverlapped->GetOverlapped() : NULL);
	Py_END_ALLOW_THREADS
	DWORD rc = ok ? 0 : GetLastError();
	if (rc!=0 && rc != ERROR_IO_PENDING)
		return PyWin_SetAPIError("WaitCommError", rc);
	return Py_BuildValue("ll", rc, *pmask);
	// @comm If an overlapped structure is passed, then the <om PyOVERLAPPED.dword> 
	// address is passed to the Win32 API as the mask.  This means that once the
	// overlapped operation has completed, this dword attribute can be used to
	// determine the type of event that occurred.
}
%}
%native (WaitCommEvent) MyWaitCommEvent;

// Some Win2k specific volume mounting functions, thanks to Roger Upole
%{

static BOOL (WINAPI *pfnGetVolumeNameForVolumeMountPointW)(LPCWSTR, LPCWSTR, DWORD) = NULL;
static BOOL (WINAPI *pfnSetVolumeMountPointW)(LPCWSTR, LPCWSTR) = NULL;
static BOOL (WINAPI *pfnDeleteVolumeMountPointW)(LPCWSTR) = NULL;
static BOOL (WINAPI *pfnCreateHardLinkW)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES ) = NULL;

#define VOLUME_POINTERS_NON_NULL \
            (pfnGetVolumeNameForVolumeMountPointW != NULL && \
            pfnSetVolumeMountPointW != NULL && \
            pfnDeleteVolumeMountPointW != NULL && \
            pfnCreateHardLinkW != NULL)

static BOOL _CheckVolumePfns()
{
    // Do a LoadLibrary for the function pointers, as these are
    // win2k specific.
    if (!VOLUME_POINTERS_NON_NULL) {

        HMODULE hMod = GetModuleHandle("kernel32.dll");
        if (hMod==0) {
            PyWin_SetAPIError("GetModuleHandle", E_HANDLE);
            return FALSE;
        }

        FARPROC fp = GetProcAddress(hMod, "GetVolumeNameForVolumeMountPointW");
        if (fp) pfnGetVolumeNameForVolumeMountPointW = (BOOL (WINAPI *)(LPCWSTR, LPCWSTR, DWORD))(fp);

        fp = GetProcAddress(hMod, "SetVolumeMountPointW");
        if (fp) pfnSetVolumeMountPointW = (BOOL (WINAPI *)(LPCWSTR, LPCWSTR))(fp);

        fp = GetProcAddress(hMod, "DeleteVolumeMountPointW");
        if (fp) pfnDeleteVolumeMountPointW = (BOOL (WINAPI *)(LPCWSTR))(fp);

        fp = GetProcAddress(hMod, "CreateHardLinkW");
        if (fp) pfnCreateHardLinkW = (BOOL (WINAPI *)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES))(fp);

    }
    return VOLUME_POINTERS_NON_NULL;
}

// @pyswig <o PyUnicode>|SetVolumeMountPoint|Mounts the specified volume at the specified volume mount point.
static PyObject*
py_SetVolumeMountPoint(PyObject *self, PyObject *args)
{
    // @ex Usage|SetVolumeMountPoint('h:\tmp\','c:\')
    // @pyparm string|mountPoint||The mount point - must be an existing empty directory on an NTFS volume
    // @pyparm string|volumeName||The volume to mount there 
    // @comm Note that both parameters must have trailing backslashes.
    // @rdesc The result is the GUID of the volume mounted, as a string.
    // @comm This method exists only on Windows 2000.If there
    // is an attempt to use it on these platforms, an error with E_NOTIMPL will be raised.
    PyObject *ret=NULL;
    PyObject *volume_obj = NULL, *mount_point_obj = NULL;
    // LPWSTR volume = NULL, mount_point = NULL;

    WCHAR *volume = NULL;
    WCHAR *mount_point = NULL;
    WCHAR volume_name[50];

    if (!_CheckVolumePfns())
        return NULL;
    if (!PyArg_ParseTuple(args,"OO", &mount_point_obj, &volume_obj))
        return NULL;

    if (!PyWinObject_AsWCHAR(mount_point_obj, &mount_point, false)){
        PyErr_SetString(PyExc_TypeError,"Mount point must be string or unicode");
        goto cleanup;
    }

    if (!PyWinObject_AsWCHAR(volume_obj, &volume, false)){
        PyErr_SetString(PyExc_TypeError,"Volume name must be string or unicode");
        goto cleanup;
    }

    assert(pfnGetVolumeNameForVolumeMountPointW);
    if (!(*pfnGetVolumeNameForVolumeMountPointW)(volume,volume_name,sizeof(volume_name)/sizeof(volume_name[0]))){
        PyWin_SetAPIError("GetVolumeNameForVolumeMountPoint");
        goto cleanup;
    }
    assert(pfnSetVolumeMountPointW);
    if (!(*pfnSetVolumeMountPointW)(mount_point, volume_name)){
        PyWin_SetAPIError("SetVolumeMountPoint");
        goto cleanup;
    }
    ret=PyWinObject_FromWCHAR(volume_name);
cleanup:
    PyWinObject_FreeWCHAR(volume);
    PyWinObject_FreeWCHAR(mount_point);

    return ret;
}


// @pyswig |DeleteVolumeMountPoint|Unmounts the volume from the specified volume mount point.
static PyObject*
py_DeleteVolumeMountPoint(PyObject *self, PyObject *args)
{
    // @ex Usage|DeleteVolumeMountPoint('h:\tmp\')
    // @pyparm string|mountPoint||The mount point to delete - must have a trailing backslash.
    // @comm Throws an error if it is not a valid mount point, returns None on success.
    // <nl>Use carefully - will remove drive letter assignment if no directory specified
    // @comm This method exists only on Windows 2000.If there
    // is an attempt to use it on these platforms, an error with E_NOTIMPL will be raised.

    PyObject *ret=NULL;
    PyObject *mount_point_obj = NULL;
    WCHAR *mount_point = NULL;

    if (!_CheckVolumePfns())
        return NULL;

    if (!PyArg_ParseTuple(args,"O", &mount_point_obj))
        return NULL;

    if (!PyWinObject_AsWCHAR(mount_point_obj, &mount_point, false)){
        PyErr_SetString(PyExc_TypeError,"Mount point must be string or unicode");
        goto cleanup;
    }

    assert(pfnDeleteVolumeMountPointW);
    if (!(*pfnDeleteVolumeMountPointW)(mount_point)){
        PyWin_SetAPIError("DeleteVolumeMountPoint");
    }
    else
        ret=Py_None;
    PyWinObject_FreeWCHAR(mount_point);

    cleanup:
    Py_XINCREF(ret);
    return ret;
}

// @pyswig |CreateHardLink|Establishes an NTFS hard link between an existing file and a new file.
static PyObject*
py_CreateHardLink(PyObject *self, PyObject *args)
{
    // @comm  An NTFS hard link is similar to a POSIX hard link.
    // <nl>This function reates a second directory entry for an existing file, can be different name in
    // same directory or any name in a different directory.
    // Both file paths must be on the same NTFS volume.<nl>To remove the link, simply delete 
    // it and the original file will still remain.
    // @ex Usage|CreateHardLink('h:\dir\newfilename.txt','h:\otherdir\existingfile.txt')
    // @pyparm string|fileName||The name of the new directory entry to be created.
    // @pyparm string|existingName||The name of the existing file to which the new link will point.
    // @pyparm <o PySECURITY_ATTRIBUTES>|security||a SECURITY_ATTRIBUTES structure that specifies a security descriptor for the new file.
    // If this parameter is None, it leaves the file's existing security descriptor unmodified. 
    // If this parameter is not None, it modifies the file's security descriptor. 
    // @comm This method exists only on Windows 2000.If there
    // is an attempt to use it on these platforms, an error with E_NOTIMPL will be raised.

    PyObject *ret=NULL;
    PyObject *new_file_obj;
    PyObject *existing_file_obj;
    PyObject *sa_obj = Py_None;
    WCHAR *new_file = NULL;
    WCHAR *existing_file = NULL;
    SECURITY_ATTRIBUTES *sa;

    if (!_CheckVolumePfns())
        return NULL;

    if (!PyArg_ParseTuple(args,"OO|O", &new_file_obj, &existing_file_obj, &sa_obj))
        return NULL;

    if (!PyWinObject_AsWCHAR(new_file_obj, &new_file, false)){
        PyErr_SetString(PyExc_TypeError,"New file name must be string or unicode");
        goto cleanup;
    }

    if (!PyWinObject_AsWCHAR(existing_file_obj, &existing_file, false)){
        PyErr_SetString(PyExc_TypeError,"Existing file name must be string or unicode");
        goto cleanup;
    }

    if (!PyWinObject_AsSECURITY_ATTRIBUTES(sa_obj, &sa, true)){
        PyErr_SetString(PyExc_TypeError,"3rd param must be a SECURITY_ATTRIBUTES, or None");
        goto cleanup;
    }

    assert(pfnCreateHardLinkW);
    if (!((*pfnCreateHardLinkW)(new_file, existing_file, sa))){
        PyWin_SetAPIError("CreateHardLink");
        goto cleanup;
    }
    ret=Py_None;
    Py_INCREF(Py_None);
cleanup:
    PyWinObject_FreeWCHAR(new_file);
    PyWinObject_FreeWCHAR(existing_file);
    return ret;
}

%}
%native (SetVolumeMountPoint) py_SetVolumeMountPoint;
%native (DeleteVolumeMountPoint) py_DeleteVolumeMountPoint;
%native (CreateHardLink) py_CreateHardLink;
// end of win2k volume mount functions.

#define EV_BREAK EV_BREAK // A break was detected on input. 
#define EV_CTS EV_CTS // The CTS (clear-to-send) signal changed state. 
#define EV_DSR EV_DSR // The DSR (data-set-ready) signal changed state. 
#define EV_ERR EV_ERR // A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define EV_RING EV_RING // A ring indicator was detected. 
#define EV_RLSD EV_RLSD // The RLSD (receive-line-signal-detect) signal changed state. 
#define EV_RXCHAR EV_RXCHAR // A character was received and placed in the input buffer. 
#define EV_RXFLAG EV_RXFLAG // The event character was received and placed in the input buffer. The event character is specified in the device's DCB structure, which is applied to a serial port by using the SetCommState function. 
#define EV_TXEMPTY EV_TXEMPTY // The last character in the output buffer was sent.
#define CBR_110 CBR_110 
#define CBR_19200 CBR_19200
#define CBR_300 CBR_300 
#define CBR_38400 CBR_38400
#define CBR_600 CBR_600 
#define CBR_56000 CBR_56000
#define CBR_1200 CBR_1200
#define CBR_57600 CBR_57600
#define CBR_2400 CBR_2400
#define CBR_115200 CBR_115200
#define CBR_4800 CBR_4800
#define CBR_128000 CBR_128000
#define CBR_9600 CBR_9600
#define CBR_256000 CBR_256000
#define CBR_14400 CBR_14400 
#define DTR_CONTROL_DISABLE DTR_CONTROL_DISABLE // Disables the DTR line when the device is opened and leaves it disabled. 
#define DTR_CONTROL_ENABLE DTR_CONTROL_ENABLE // Enables the DTR line when the device is opened and leaves it on. 
#define DTR_CONTROL_HANDSHAKE DTR_CONTROL_HANDSHAKE // Enables DTR handshaking. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function. 
#define RTS_CONTROL_DISABLE RTS_CONTROL_DISABLE // Disables the RTS line when the device is opened and leaves it disabled. 
#define RTS_CONTROL_ENABLE RTS_CONTROL_ENABLE // Enables the RTS line when the device is opened and leaves it on. 
#define RTS_CONTROL_HANDSHAKE RTS_CONTROL_HANDSHAKE // Enables RTS handshaking. The driver raises the RTS line when the "type-ahead" (input) buffer is less than one-half full and lowers the RTS line when the buffer is more than three-quarters full. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function. 
#define RTS_CONTROL_TOGGLE RTS_CONTROL_TOGGLE // Specifies that the RTS line will be high if bytes are available for transmission. After all buffered bytes have been sent, the RTS line will be low. 
#define EVENPARITY EVENPARITY
#define MARKPARITY MARKPARITY
#define NOPARITY NOPARITY
#define ODDPARITY ODDPARITY
#define SPACEPARITY SPACEPARITY
#define ONESTOPBIT ONESTOPBIT
#define ONE5STOPBITS ONE5STOPBITS
#define TWOSTOPBITS TWOSTOPBITS 
#define CLRDTR CLRDTR // Clears the DTR (data-terminal-ready) signal. 
#define CLRRTS CLRRTS // Clears the RTS (request-to-send) signal. 
#define SETDTR SETDTR // Sends the DTR (data-terminal-ready) signal. 
#define SETRTS SETRTS // Sends the RTS (request-to-send) signal. 
#define SETXOFF SETXOFF // Causes transmission to act as if an XOFF character has been received. 
#define SETXON SETXON // Causes transmission to act as if an XON character has been received. 
#define SETBREAK SETBREAK // Suspends character transmission and places the transmission line in a break state until the ClearCommBreak function is called (or EscapeCommFunction is called with the CLRBREAK extended function code). The SETBREAK extended function code is identical to the SetCommBreak function. Note that this extended function does not flush data that has not been transmitted. 
#define CLRBREAK CLRBREAK // Restores character transmission and places the transmission line in a nonbreak state. The CLRBREAK extended function code is identical to the ClearCommBreak function. 
#define PURGE_TXABORT PURGE_TXABORT // Terminates all outstanding overlapped write operations and returns immediately, even if the write operations have not been completed. 
#define PURGE_RXABORT PURGE_RXABORT // Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed. 
#define PURGE_TXCLEAR PURGE_TXCLEAR // Clears the output buffer (if the device driver has one). 
#define PURGE_RXCLEAR PURGE_RXCLEAR // Clears the input buffer (if the device driver has one). 


