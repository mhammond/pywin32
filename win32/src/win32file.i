/* File : win32file.i */
// @doc

%module win32file // An interface to the win32 File API's
// <nl>This module includes the transactional NTFS operations introduced with
// Vista.  The transacted functions are not wrapped separately, but are invoked by
// passing a transaction handle to the corresponding Unicode API function.
// This makes it simple to convert a set of file operations into a transaction by
// simply adding Transaction=<o PyHANDLE> to the passed arguments.
// If Transaction is None, 0, or not specified, the non-transacted API function will
// be called.
// <nl>Functions combined in this manner:
//		<nl>CreateFile / CreateFileTransacted
//		<nl>DeleteFile / DeleteFileTransacted
//		<nl>CreateDirectoryEx / CreateDirectoryTransacted
//		<nl>MoveFileWithProgress / MoveFileTransacted
//		<nl>CopyFileEx / CopyFileTransacted
//		<nl>GetFileAttributes / GetFileAttributesTransacted
//		<nl>SetFileAttributes / SetFileAttributesTransacted
//		<nl>CreateHardLink / CreateHardLinkTransacted
//		<nl>CreateSymbolicLink / CreateSymbolicLinkTransacted
//		<nl>RemoveDirectory / RemoveDirectoryTransacted

%{
// We use the deprecated API
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// bug #752
// Include windows.h before winsock2 because with older SDKs the latter may
// include windows.h with wrong pragma pack directive in effect, making
// incorrect sizeof and layout of some WINAPI structs.
// One does not simple include windows.h before winsock2, because windows.h
// pulls old winsock.h (MSDN: for historical reasons) which conflicts with
// winsock2 causing compilation errors.
// To avoid inclusion of winsock.h we define WIN32_LEAN_AND_MEAN macro to
// drop some headers from compilation. We then have to explicitly include ole2
// and windefs affected by the macro.
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "ole2.h"
#include "Winefs.h"

#include "winsock2.h"
#include "mswsock.h"
#include "pywintypes.h"
#include "winbase.h"
#include "assert.h"
#include <stddef.h>
#include "sfc.h"

// pyconfig.h defines socklen_t, which conflicts with below header
#ifdef socklen_t
#	undef socklen_t
#endif
#include "Ws2tcpip.h"
#include "Wspiapi.h" // for WspiapiGetAddrInfo/WspiapiFreeAddrInfo

#define NEED_PYWINOBJECTS_H
#include "win32file_comm.h"
%}

%include "typemaps.i"
%include "pywin32.i"

%{

#include "datetime.h" // python's datetime header.

%}

#define FILE_GENERIC_READ FILE_GENERIC_READ
#define FILE_GENERIC_WRITE FILE_GENERIC_WRITE
#define FILE_ALL_ACCESS FILE_ALL_ACCESS

#define GENERIC_READ GENERIC_READ
// Specifies read access to the object. Data can be read from the file and the file pointer can be moved. Combine with GENERIC_WRITE for read-write access.
#define GENERIC_WRITE GENERIC_WRITE
// Specifies write access to the object. Data can be written to the file and the file pointer can be moved. Combine with GENERIC_READ for read-write access.
#define GENERIC_EXECUTE GENERIC_EXECUTE
// Specifies execute access.

#define FILE_SHARE_DELETE  FILE_SHARE_DELETE
// Windows NT only: Subsequent open operations on the object will succeed only if delete access is requested.
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
#define FILE_ATTRIBUTE_DIRECTORY FILE_ATTRIBUTE_DIRECTORY
// The file is a directory
#define FILE_ATTRIBUTE_COMPRESSED FILE_ATTRIBUTE_COMPRESSED
// The file or directory is compressed. For a file, this means that all of the data in the file is compressed. For a directory, this means that compression is the default for newly created files and subdirectories.
#define FILE_ATTRIBUTE_HIDDEN FILE_ATTRIBUTE_HIDDEN
// The file is hidden. It is not to be included in an ordinary directory listing.
#define FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_NORMAL
// The file has no other attributes set. This attribute is valid only if used alone.
#define FILE_ATTRIBUTE_OFFLINE FILE_ATTRIBUTE_OFFLINE
// The data of the file is not immediately available. Indicates that the file data has been physically moved to offline storage.
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
#define FILE_FLAG_OPEN_REPARSE_POINT FILE_FLAG_OPEN_REPARSE_POINT
// used to open a handle for use with DeviceIoControl and FSCTL_GET_REPARSE_POINT/FSCTL_SET_REPARSE_POINT)

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

// @pyswig int|AreFileApisANSI|Determines whether a set of Win32 file functions is using the ANSI or OEM character set code page. This function is useful for 8-bit console input and output operations.
BOOL AreFileApisANSI(void);

// @pyswig |CancelIo|Cancels pending IO requests for the object.
// @pyparm <o PyHANDLE>|handle||The handle being cancelled.
BOOLAPI CancelIo(PyHANDLE handle);

// @pyswig |CopyFile|Copies a file
BOOLAPI CopyFile(
    TCHAR *from, // @pyparm string|from||The name of the file to copy from
    TCHAR *to, // @pyparm string|to||The name of the file to copy to
    BOOL bFailIfExists); // @pyparm int|bFailIfExists||Indicates if the operation should fail if the file exists.

// @pyswig |CopyFileW|Copies a file
BOOLAPI CopyFileW(
    WCHAR *from, // @pyparm string|from||The name of the file to copy from
    WCHAR *to, // @pyparm string|to||The name of the file to copy to
    BOOL bFailIfExists); // @pyparm int|bFailIfExists||Indicates if the operation should fail if the file exists.

// @pyswig |CreateDirectory|Creates a directory
BOOLAPI CreateDirectory(
    TCHAR *name, // @pyparm string|name||The name of the directory to create
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig |CreateDirectoryW|Creates a directory
BOOLAPI CreateDirectoryW(
    WCHAR *name, // @pyparm string|name||The name of the directory to create
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig |CreateDirectoryEx|Creates a directory
BOOLAPI CreateDirectoryEx(
    TCHAR *templateName, // @pyparm string|templateName||Specifies the path of the directory to use as a template when creating the new directory.
    TCHAR *newDirectory, // @pyparm string|newDirectory||Specifies the name of the new directory
    SECURITY_ATTRIBUTES *pSA); // @pyparm <o PySECURITY_ATTRIBUTES>|sa||The security attributes, or None

// @pyswig <o PyHANDLE>|CreateFile|Creates or opens the a file or other object and returns a handle that can be used to access the object.
// @comm The following objects can be opened:<nl>files<nl>pipes<nl>mailslots<nl>communications resources<nl>disk devices (Windows NT only)<nl>consoles<nl>directories (open only)
PyHANDLE CreateFile(
    TCHAR *lpFileName,	// @pyparm string|fileName||The name of the file
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
    DWORD dwCreationDisposition,	// @pyparm int|CreationDisposition||Specifies which action to take on files that exist, and which action to take when files do not exist. For more information about this parameter, see the Remarks section. This parameter must be one of the following values:
			// @flagh Value|Meaning
			// @flag CREATE_NEW|Creates a new file. The function fails if the specified file already exists.
			// @flag CREATE_ALWAYS|Creates a new file. If the file exists, the function overwrites the file and clears the existing attributes.
			// @flag OPEN_EXISTING|Opens the file. The function fails if the file does not exist.
			//       See the Remarks section for a discussion of why you should use the OPEN_EXISTING flag if you are using the CreateFile function for devices, including the console.
			// @flag OPEN_ALWAYS|Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDisposition were CREATE_NEW.
			// @flag TRUNCATE_EXISTING|Opens the file. Once opened, the file is truncated so that its size is zero bytes. The calling process must open the file with at least GENERIC_WRITE access. The function fails if the file does not exist.
    DWORD dwFlagsAndAttributes,	// @pyparm int|flagsAndAttributes||file attributes
    PyHANDLE INPUT_NULLOK // @pyparm <o PyHANDLE>|hTemplateFile||Specifies a handle with GENERIC_READ access to a template file. The template file supplies file attributes and extended attributes for the file being created.
);

// CreateIoCompletionPort gets special treatment due to its special result
// code handling.

%{
// @pyswig <o PyHANDLE>|CreateIoCompletionPort|Can associate an instance of an opened file with a newly created or an existing input/output (I/O) completion port; or it can create an I/O completion port without associating it with a file.
// @rdesc If an existing handle to a completion port is passed, the result
// of this function will be that same handle.  See MSDN for more details.
PyObject *MyCreateIoCompletionPort(PyObject *self, PyObject *args)
{
    PyObject *obFileHandle, *obExistingHandle, *obkey;
    DWORD nt;
    ULONG_PTR key;
    PyObject *obRet = NULL;
    if (!PyArg_ParseTuple(args, "OOOi:CreateIoCompletionPort",
                          &obFileHandle, // @pyparm <o PyHANDLE>|handle||file handle to associate with the I/O completion port
                          &obExistingHandle, // @pyparm <o PyHANDLE>|existing||handle to the I/O completion port
                          &obkey, // @pyparm int|completionKey||per-file completion key for I/O completion packets
                          &nt)) // @pyparm int|numThreads||number of threads allowed to execute concurrently
        return NULL;
    if (!PyWinLong_AsVoidPtr(obkey, (void **)&key))
        return NULL;
    HANDLE hFile, hExisting;
    if (!PyWinObject_AsHANDLE(obFileHandle, &hFile))
        return NULL;
    if (!PyWinObject_AsHANDLE(obExistingHandle, &hExisting))
        return NULL;
    if (hExisting) {
        obRet = obExistingHandle;
        Py_INCREF(obRet);
    }
    HANDLE hRet;
    Py_BEGIN_ALLOW_THREADS
    hRet = CreateIoCompletionPort(hFile, hExisting, key, nt);
    Py_END_ALLOW_THREADS
    if (!hRet) {
        Py_XDECREF(obRet);
        return PyWin_SetAPIError("CreateIoCompletionPort");
    }
    if (obRet==NULL) // New handle returned
        obRet = PyWinObject_FromHANDLE(hRet);
    else
        // it better have returned the same object!
        assert(hRet == hExisting);
    return obRet;
}

%}

%native (CreateIoCompletionPort) MyCreateIoCompletionPort;

// @pyswig <o PyHANDLE>|CreateMailslot|Creates a mailslot on the local machine
// @pyseeapi CreateMailslot
PyHANDLE CreateMailslot(
	TCHAR  *Name,		// @pyparm str|Name||Name of the mailslot, of the form \\.\mailslot\[path]name
	DWORD MaxMessageSize,	// @pyparm int|MaxMessageSize||Largest message size.  Use 0 for unlimited.
	DWORD ReadTimeout,		// @pyparm int|ReadTimeout||Timeout in milliseconds.  Use -1 for no timeout.
	SECURITY_ATTRIBUTES *INPUT_NULLOK	// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes||Determines if returned handle is inheritable, can be None
);

// @pyswig (int,int,int,int)|GetMailslotInfo|Retrieves information about a mailslot
// @rdesc Returns (maximum message size, next message size, message count, timeout)
// @pyseeapi GetMailslotInfo
BOOLAPI GetMailslotInfo(
	HANDLE Mailslot,	// @pyparm <o PyHANDLE>|Mailslot||Handle to a mailslot
	DWORD *OUTPUT,
	DWORD *OUTPUT,
	DWORD *OUTPUT,
	DWORD *OUTPUT);

// @pyswig |SetMailslotInfo|Sets a mailslot's timeout
// @pyseeapi SetMailslotInfo
BOOLAPI SetMailslotInfo(
	HANDLE Mailslot,	// @pyparm <o PyHANDLE>|Mailslot||Handle to a mailslot
	DWORD ReadTimeout);	// @pyparm int|ReadTimeout||Timeout in milliseconds, use -1 for no timeout

// @pyswig |DefineDosDevice|Lets an application define, redefine, or delete MS-DOS device names.
BOOLAPI DefineDosDevice(
    DWORD dwFlags,	// @pyparm int|flags||flags specifying aspects of device definition
    TCHAR *lpDeviceName,	// @pyparm string|deviceName||MS-DOS device name string
    TCHAR *lpTargetPath	// @pyparm string|targetPath||MS-DOS or path string for 32-bit Windows.
);
// @pyswig |DefineDosDeviceW|Lets an application define, redefine, or delete MS-DOS device names.
BOOLAPI DefineDosDeviceW(
    DWORD dwFlags,	// @pyparm int|flags||flags specifying aspects of device definition
    WCHAR *lpDeviceName,	// @pyparm string|deviceName||MS-DOS device name string
    WCHAR *lpTargetPath	// @pyparm string|targetPath||MS-DOS or path string for 32-bit Windows.
);

// @pyswig |DeleteFile|Deletes a file.
BOOLAPI DeleteFile(TCHAR *fileName);
// @pyparm string|fileName||The filename to delete

%{
// theoretically this could be in pywintypes, but this is the only place
// it is called...

static PyObject *PyBuffer_FromReadWriteMemory(void *buf, Py_ssize_t size){
	Py_buffer info;
	/* PyBUF_CONTIG contains PyBUF_ND, so that Py_buffer.shape is filled in.
		Apparently the shape is now required for even simple contiguous byte
		buffers. (see get_shape0 in memoryobject.c)
		Since PyBuffer_FillInfo is specifically for this case, it should probably
		set the shape unconditionally.
	*/
	if (PyBuffer_FillInfo(&info, NULL, buf, size, 0, PyBUF_CONTIG) == -1)
		return NULL;
	return PyMemoryView_FromBuffer(&info);
}


// @pyswig str/buffer|DeviceIoControl|Sends a control code to a device or file system driver
// @comm Accepts keyword args
// @rdesc If a preallocated output buffer is passed in, the returned object
//	may be the original buffer, or a view of the buffer with only the actual
//	size of the retrieved data.
//	<nl>If OutBuffer is a buffer size and the operation is synchronous (ie no
//	Overlapped is passed in), returns a plain string containing the retrieved
//	data.  For an async operation, a new writeable buffer is returned.
PyObject *py_DeviceIoControl(PyObject *self, PyObject *args, PyObject *kwargs)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile, *obInBuffer, *obOutBuffer, *ret=NULL;
	HANDLE hDevice;
	PyObject *obOverlapped = Py_None;

	DWORD dwIoControlCode;
	void *OutBuffer=NULL;
	DWORD OutBufferSize=0;
	BOOL bBuffer=FALSE;

	static char *keywords[]={"Device","IoControlCode","InBuffer","OutBuffer","Overlapped", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OkOO|O:DeviceIoControl", keywords,
		&obhFile,			// @pyparm <o PyHANDLE>|Device||Handle to a file, device, or volume
		&dwIoControlCode,	// @pyparm int|IoControlCode||IOControl Code to use, from winioctlcon
		&obInBuffer,		// @pyparm str/buffer|InBuffer||The input data for the operation, can be None for some operations.
		&obOutBuffer,		// @pyparm int/buffer|OutBuffer||Size of the buffer to allocate for output, or a writeable buffer
							//	as returned by <om win32file.AllocateReadBuffer>.
		&obOverlapped))		// @pyparm <o PyOVERLAPPED>|Overlapped|None|An overlapped object for async operations.  Device
							//	handle must have been opened with FILE_FLAG_OVERLAPPED.
		return NULL;

	if (!PyWinObject_AsHANDLE(obhFile, &hDevice))
		return NULL;
	PyWinBufferView out_buf, in_buf(obInBuffer, false, true); // None Ok
	if (!in_buf.ok())
		return NULL;
	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped, TRUE))
		return NULL;

	OutBufferSize=PyLong_AsLong(obOutBuffer);
	if (OutBufferSize!=(DWORD)-1 || !PyErr_Occurred()) {
		// Return a writable buffer in asynch mode, otherwise a plain string
		//	for backward compatibility
		if (pOverlapped != NULL) {
			ret = PyBuffer_New(OutBufferSize);
			if (ret == NULL)
				return NULL;
			if (!out_buf.init(ret, true)) {
				Py_DECREF(ret);
				return NULL;
				}
			OutBuffer = out_buf.ptr();
			OutBufferSize = out_buf.len();
			}
		else {
			ret = PyBytes_FromStringAndSize(NULL, OutBufferSize);
			if (ret==NULL)
				return NULL;
			OutBuffer=PyBytes_AS_STRING(ret);
			}
		}
	else {
		PyErr_Clear();
		if (out_buf.init(obOutBuffer, true, true)) {
			OutBuffer = out_buf.ptr();
			OutBufferSize = out_buf.len();
			Py_INCREF(obOutBuffer);
			ret=obOutBuffer;
			bBuffer=TRUE;
			}
		else {
			PyErr_Clear();
			return PyErr_Format(PyExc_TypeError,
				"OutBuffer must be either a buffer size or writeable buffer object, not %s",
				obOutBuffer->ob_type->tp_name);
			}
		}

	DWORD numRead;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS

	ok = DeviceIoControl(hDevice,
                         dwIoControlCode,
                         in_buf.ptr(),
                         in_buf.len(),
                         OutBuffer,
                         OutBufferSize,
                         &numRead,
                         pOverlapped);
	Py_END_ALLOW_THREADS

    if (!ok){
		DWORD err=GetLastError();
		// This error code is returned for a pending overlapped operation.
		if (err==ERROR_IO_PENDING)
			return ret;
		Py_DECREF(ret);
		return PyWin_SetAPIError("DeviceIoControl", err);
		}

	// If returned size less than requested buffer size, return only length of valid data
	if (numRead < OutBufferSize){
		if (bBuffer){
			// Create a view of existing buffer with actual output size
			// Memoryview object in py3k supports slicing
			PyObject *resized=PySequence_GetSlice(ret, 0, numRead);
			Py_DECREF(ret);
			ret=resized;
			}
		else
			_PyBytes_Resize(&ret, numRead);
		}
	return ret;
}
PyCFunction pfnpy_DeviceIoControl=(PyCFunction)py_DeviceIoControl;
%}
%native(DeviceIoControl) pfnpy_DeviceIoControl;


%native (OVERLAPPED) PyWinMethod_NewOVERLAPPED;



//FileIOCompletionRoutine

// @pyswig |FindClose|Closes a find handle.
BOOLAPI FindClose(HANDLE hFindFile);	// @pyparm int|hFindFile||file search handle

// @pyswig |FindCloseChangeNotification|Closes a handle.
BOOLAPI FindCloseChangeNotification(
    HANDLE hChangeHandle 	// @pyparm int|hChangeHandle||handle to change notification to close
);

// @pyswig int|FindFirstChangeNotification|Creates a change notification handle and sets up initial change notification filter conditions. A wait on a notification handle succeeds when a change matching the filter conditions occurs in the specified directory or subtree.
HANDLE FindFirstChangeNotification(
    TCHAR *lpPathName,	// @pyparm string|pathName||Name of directory to watch
    BOOL bWatchSubtree,	// @pyparm int|bWatchSubtree||flag for monitoring directory or directory tree
    DWORD dwNotifyFilter 	// @pyparm int|notifyFilter||filter conditions to watch for.  See <om win32api.FindFirstChangeNotification> for details.
);

// @pyswig int|FindNextChangeNotification|Requests that the operating system signal a change notification handle the next time it detects an appropriate change,
BOOLAPI FindNextChangeNotification(
    HANDLE hChangeHandle 	//  @pyparm int|hChangeHandle||handle to change notification to signal
);


%{

typedef struct {
	PyObject_HEAD
	HANDLE hFind;
	WIN32_FIND_DATAW buffer;
	BOOL seen_first;
	BOOL empty;
} FindFileIterator;


static void
ffi_dealloc(FindFileIterator *it)
{
	if (it->hFind != INVALID_HANDLE_VALUE)
		::FindClose(it->hFind);
	PyObject_Del(it);
}

static PyObject *
ffi_iternext(PyObject *iterator)
{
	FindFileIterator *ffi = (FindFileIterator *)iterator;
	if (ffi->empty) {
		PyErr_SetNone(PyExc_StopIteration);
		return NULL;
	}
	if (!ffi->seen_first)
		ffi->seen_first = TRUE;
	else {
		BOOL ok;
		Py_BEGIN_ALLOW_THREADS
		memset(&ffi->buffer, 0, sizeof(ffi->buffer));
		ok = ::FindNextFileW(ffi->hFind, &ffi->buffer);
		Py_END_ALLOW_THREADS
		if (!ok) {
			if (GetLastError()==ERROR_NO_MORE_FILES) {
				PyErr_SetNone(PyExc_StopIteration);
				return NULL;
			}
			return PyWin_SetAPIError("FindNextFileW");
		}
	}
	return PyObject_FromWIN32_FIND_DATAW(&ffi->buffer);
}

PyTypeObject FindFileIterator_Type = {
	PYWIN_OBJECT_HEAD
	"FindFileIterator",				/* tp_name */
	sizeof(FindFileIterator),			/* tp_basicsize */
	0,					/* tp_itemsize */
	/* methods */
	(destructor)ffi_dealloc, 		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT, /* tp_flags */
	0,					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	PyObject_SelfIter,	/* tp_iter */
	(iternextfunc)ffi_iternext,		/* tp_iternext */
	0,					/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
};
%}

// @pyswig |FlushFileBuffers|Clears the buffers for the specified file and causes all buffered data to be written to the file.
BOOLAPI FlushFileBuffers(
   PyHANDLE hFile 	// @pyparm <o PyHANDLE>|hFile||open handle to file whose buffers are to be flushed
);

// @pyswig int|GetBinaryType|Determines whether a file is executable, and if so, what type of executable file it is. That last property determines which subsystem an executable file runs under.
BOOLAPI GetBinaryType(
    TCHAR *lpApplicationName,	// @pyparm string|appName||Fully qualified path of file to test
    unsigned long *OUTPUT	// DWORD
   );
#define SCS_32BIT_BINARY SCS_32BIT_BINARY // A Win32-based application
#define SCS_DOS_BINARY SCS_DOS_BINARY // An MS-DOS - based application
#define SCS_OS216_BINARY SCS_OS216_BINARY // A 16-bit OS/2-based application
#define SCS_PIF_BINARY SCS_PIF_BINARY // A PIF file that executes an MS-DOS - based application
#define SCS_POSIX_BINARY SCS_POSIX_BINARY // A POSIX - based application
#define SCS_WOW_BINARY SCS_WOW_BINARY // A 16-bit Windows-based application

//GetCurrentDirectory

// @pyswig (int, int, int, int)|GetDiskFreeSpace|Determines the free space on a device.
BOOLAPI GetDiskFreeSpace(
    TCHAR *lpRootPathName,	// @pyparm string|rootPathName||address of root path
    unsigned long *OUTPUT,
    unsigned long *OUTPUT,
    unsigned long *OUTPUT,
    unsigned long *OUTPUT
// @rdesc The result is a tuple of integers representing (sectors per cluster, bytes per sector, number of free clusters, total number of clusters)
);

// GetDiskFreeSpaceEx
// @pyswig long, long, long|GetDiskFreeSpaceEx|Determines the free space on a device.
BOOLAPI GetDiskFreeSpaceEx(
    TCHAR *lpRootPathName,	// @pyparm string|rootPathName||address of root path
    ULARGE_INTEGER *OUTPUT,
    ULARGE_INTEGER *OUTPUT,
    ULARGE_INTEGER *OUTPUT
// @rdesc The result is a tuple of long integers:
// @tupleitem 0|long integer|freeBytes|The total number of free bytes on the disk that are available to the user associated with the calling thread.
// @tupleitem 1|long integer|totalBytes|The total number of bytes on the disk that are available to the user associated with the calling thread.
// If per-user quotas are in use, this value may be less than the total number of bytes on the disk.
// @tupleitem 2|long integer|totalFreeBytes|The total number of free bytes on the disk.
);

// @pyswig int|GetDriveType|Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive.
long GetDriveType(
    TCHAR *rootPathName // @pyparm string|rootPathName||
// @rdesc The result is one of the DRIVE_* constants.
);
// @pyswig int|GetDriveTypeW|Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive.
long GetDriveTypeW(
    WCHAR *rootPathName // @pyparm string|rootPathName||
// @rdesc The result is one of the DRIVE_* constants.
);

#define DRIVE_UNKNOWN DRIVE_UNKNOWN // The drive type cannot be determined.
#define DRIVE_NO_ROOT_DIR DRIVE_NO_ROOT_DIR // The root directory does not exist.
#define DRIVE_REMOVABLE DRIVE_REMOVABLE // The disk can be removed from the drive.
#define DRIVE_FIXED DRIVE_FIXED // The disk cannot be removed from the drive.
#define DRIVE_REMOTE DRIVE_REMOTE // The drive is a remote (network) drive.
#define DRIVE_CDROM DRIVE_CDROM // The drive is a CD-ROM drive.
#define DRIVE_RAMDISK DRIVE_RAMDISK // The drive is a RAM disk.

// @pyswig int|GetFileAttributes|Determines a files attributes.
DWORD GetFileAttributes(
    TCHAR *fileName); // @pyparm string|fileName||Name of the file to retrieve attributes for.

// @pyswig int|GetFileAttributesW|Determines a files attributes
DWORD GetFileAttributesW(
    WCHAR *fileName); // @pyparm string|fileName||Name of the file to retrieve attributes for.

// @pyswig (<o PyDateTime>, <o PyDateTime>, <o PyDateTime>)|GetFileTime|Returns a file's creation, last access, and modification times.
// @comm Times are returned in UTC time.
BOOLAPI GetFileTime(
    HANDLE handle, // @pyparm <o PyHANDLE>|handle||Handle to the file.
	FILETIME *OUTPUT, // @pyparm <o PyDateTime>|creationTime||
	FILETIME *OUTPUT, // @pyparm <o PyDateTime>|accessTime||
	FILETIME *OUTPUT // @pyparm <o PyDateTime>|writeTime||
);


%{
// Helper for SetFileTime - see comments below.
static BOOL PyWinTime_DateTimeCheck(PyObject *ob)
{
	return PyDateTimeAPI && PyDateTime_Check(ob);
}

// @pyswig |SetFileTime|Sets the date and time that a file was created, last accessed, or last modified.
static PyObject *PySetFileTime (PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obHandle;       // @pyparm <o PyHANDLE>|File||Previously opened handle (opened with FILE_WRITE_ATTRIBUTES access).
	PyObject *obCreationTime = Py_None;  // @pyparm <o PyDateTime>|CreationTime|None|File created time. None for no change.
	PyObject *obLastAccessTime = Py_None; // @pyparm <o PyDateTime>|LastAccessTime|None|File access time. None for no change.
	PyObject *obLastWriteTime = Py_None;  // @pyparm <o PyDateTime>|LastWriteTime|None|File written time. None for no change.
	BOOL UTCTimes = FALSE;    // @pyparm boolean|UTCTimes|False|If True, input times are treated as UTC and no conversion is done,
							  // otherwise they are treated as local times.  Defaults to False for backward compatibility.
							  // This parameter is ignored in Python 3, where you should always pass datetime objects
							  // with timezone information.

	static char *keywords[] = {"File", "CreationTime", "LastAccessTime", "LastWriteTime", "UTCTimes", NULL};
	HANDLE hHandle;
	FILETIME CreationTime, *lpCreationTime = NULL;
	FILETIME LastAccessTime, *lpLastAccessTime = NULL;
	FILETIME LastWriteTime, *lpLastWriteTime = NULL;
	FILETIME FileTime;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOOi:SetFileTime", keywords,
		&obHandle, &obCreationTime, &obLastAccessTime, &obLastWriteTime, &UTCTimes))
		return NULL;

    if (!PyWinObject_AsHANDLE(obHandle, &hHandle))
        return NULL;
	if (obCreationTime != Py_None){
		if (!PyWinObject_AsFILETIME(obCreationTime, &FileTime))
			return NULL;
		// Do no conversion if given a timezone-aware object, or told input is UTC
		if (UTCTimes || PyWinTime_DateTimeCheck(obCreationTime))
			CreationTime = FileTime;
		else
			LocalFileTimeToFileTime(&FileTime, &CreationTime);
		lpCreationTime = &CreationTime;
	}
	if (obLastAccessTime != Py_None){
		if (!PyWinObject_AsFILETIME(obLastAccessTime, &FileTime))
			return NULL;
		if (UTCTimes || PyWinTime_DateTimeCheck(obLastAccessTime))
			LastAccessTime = FileTime;
		else
			LocalFileTimeToFileTime(&FileTime, &LastAccessTime);
		lpLastAccessTime= &LastAccessTime;
	}
	if (obLastWriteTime != Py_None){
		if (!PyWinObject_AsFILETIME(obLastWriteTime, &FileTime))
			return NULL;
		if (UTCTimes || PyWinTime_DateTimeCheck(obLastWriteTime))
			LastWriteTime = FileTime;
		else
			LocalFileTimeToFileTime(&FileTime, &LastWriteTime);
		lpLastWriteTime= &LastWriteTime;
	}
	if (!::SetFileTime(hHandle, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
		return PyWin_SetAPIError("SetFileTime");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnPySetFileTime = (PyCFunction)PySetFileTime;
%}
%native(SetFileTime) pfnPySetFileTime;

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
	return Py_BuildValue("kNNNkkkkkk",
		fi.dwFileAttributes, // @tupleitem 0|int|dwFileAttributes|
		PyWinObject_FromFILETIME(fi.ftCreationTime), // @tupleitem 1|<o PyDateTime>|ftCreationTime|
		PyWinObject_FromFILETIME(fi.ftLastAccessTime),// @tupleitem 2|<o PyDateTime>|ftLastAccessTime|
		PyWinObject_FromFILETIME(fi.ftLastWriteTime),// @tupleitem 3|<o PyDateTime>|ftLastWriteTime|
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

%{
PyObject *MyGetCompressedFileSize(PyObject *self, PyObject *args)
{
	PyObject *obName;
	TCHAR *fname;
	if (!PyArg_ParseTuple(args, "O", &obName))
		return NULL;
	if (!PyWinObject_AsTCHAR(obName, &fname, FALSE))
		return NULL;
	ULARGE_INTEGER ulsize;
    Py_BEGIN_ALLOW_THREADS
	ulsize.LowPart = GetCompressedFileSize(fname, &ulsize.HighPart);
    Py_END_ALLOW_THREADS

    PyWinObject_FreeTCHAR(fname);
	// If we failed ...
	if (ulsize.LowPart == 0xFFFFFFFF &&
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("GetCompressedFileSize");
	return PyWinObject_FromULARGE_INTEGER(ulsize);
}
%}
// @pyswig long|GetCompressedFileSize|Determines the compressed size of a file.
%native(GetCompressedFileSize) MyGetCompressedFileSize;

%{
PyObject *MyGetFileSize(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	if (!PyArg_ParseTuple(args, "O", &obHandle))
		return NULL;
	HANDLE hFile;
	if (!PyWinObject_AsHANDLE(obHandle, &hFile))
		return NULL;
	ULARGE_INTEGER ulsize;
    Py_BEGIN_ALLOW_THREADS
	ulsize.LowPart = GetFileSize (hFile, &ulsize.HighPart);
    Py_END_ALLOW_THREADS
	// If we failed ...
	if (ulsize.LowPart == 0xFFFFFFFF &&
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("GetFileSize");
	return PyWinObject_FromULARGE_INTEGER(ulsize);
}

%}
// @pyswig long|GetFileSize|Determines the size of a file.
%native(GetFileSize) MyGetFileSize;

// @object PyOVERLAPPEDReadBuffer|An alias for a standard Python buffer object.
// Previous versions of the Windows extensions had a custom object for
// holding a read buffer.  This has been replaced with the standard Python buffer object.
// <nl>Python does not provide a method for creating a read-write buffer
// of arbitrary size, so currently this can only be created by <om win32file.AllocateReadBuffer>.
%{
// @pyswig <o PyOVERLAPPEDReadBuffer>|AllocateReadBuffer|Allocates a buffer which can be used with an overlapped Read operation using <om win32file.ReadFile>
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

%{
// @pyswig (int, string)|ReadFile|Reads a string from a file
// @rdesc The result is a tuple of (hr, string/<o PyOVERLAPPEDReadBuffer>), where hr may be
// 0, ERROR_MORE_DATA or ERROR_IO_PENDING.
// If the overlapped param is not None, then the result is a <o PyOVERLAPPEDReadBuffer>.  Once the overlapped IO operation
// has completed, you can convert this to a string (str(object)) [py2k] or (bytes(object)) [py3k] to obtain the data.
// While the operation is in progress, you can use the slice operations (object[:end]) to
// obtain the data read so far.
// You must use the OVERLAPPED API functions to determine how much of the data is valid.
PyObject *MyReadFile(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped=NULL;
	PyObject *obhFile;
	HANDLE hFile;
	DWORD bufSize;
	PyObject *obOverlapped = Py_None;
	BOOL bBufMallocd = FALSE;
	PyObject *obBuf, *obRet=NULL;

	if (!PyArg_ParseTuple(args, "OO|O:ReadFile",
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		// @pyparm <o PyOVERLAPPEDReadBuffer>/int|buffer/bufSize||Size of the buffer to create for the result,
		// or a buffer to fill with the result. If a buffer object and overlapped is passed, the result is
		// the buffer itself.  If a buffer but no overlapped is passed, the result is a new string object,
		// built from the buffer, but with a length that reflects the data actually read.
		&obBuf,
		&obOverlapped))	// @pyparm <o PyOVERLAPPED>|overlapped|None|An overlapped structure
		return NULL;
	if (!PyWinObject_AsHANDLE(obhFile, &hFile))
		return NULL;

	// @comm in a multi-threaded overlapped environment, it is likely to be necessary to pre-allocate the read buffer using the <om win32file.AllocateReadBuffer> method, otherwise the I/O operation may complete before you can assign to the resulting buffer.
	if (obOverlapped!=Py_None){
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
		}

	void *buf = NULL;
	PyWinBufferView pybuf;

	bufSize = PyLong_AsLong(obBuf);
	if ((bufSize!=(DWORD)-1) || !PyErr_Occurred()){
		if (pOverlapped){
			obRet = PyBuffer_New(bufSize);
			if (obRet==NULL)
				return NULL;
			// This should never fail
			if (!pybuf.init(obRet, true)) {
				Py_DECREF(obRet);
				return NULL;
				}
			buf = pybuf.ptr();
			bufSize = pybuf.len();
			}
		else{
			obRet=PyBytes_FromStringAndSize(NULL, bufSize);
			if (obRet==NULL)
				return NULL;
			buf=PyBytes_AS_STRING(obRet);
			bBufMallocd=TRUE;
			}
		}
	else{
		PyErr_Clear();
		if (!pybuf.init(obBuf, true)) {
			PyErr_SetString(PyExc_TypeError, "Second param must be an integer or writeable buffer object");
			return NULL;
			}
		buf = pybuf.ptr();
		bufSize = pybuf.len();
		// If they didn't pass an overlapped, then we can't return the
		// original buffer as they have no way to know how many bytes
		// were read - so leave obRet NULL and the ret will be a new
		// string object, built from buffer, but the correct length.
		if (pOverlapped){
			obRet = obBuf;
			Py_INCREF(obBuf);
			}
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
			Py_XDECREF(obRet);
			return PyWin_SetAPIError("ReadFile", err);
		}
	}
	if (obRet==NULL)
		obRet=PyBytes_FromStringAndSize((char *)buf, numRead);
	else if (bBufMallocd && (numRead < bufSize))
		_PyBytes_Resize(&obRet, numRead);
	if (obRet==NULL)
		return NULL;
	return Py_BuildValue("iN", err, obRet);
}

// @pyswig int, int|WriteFile|Writes a string to a file
// @rdesc The result is a tuple of (errCode, nBytesWritten).  If errCode is not zero,
// it will be ERROR_IO_PENDING (ie, it is an overlapped request).
// <nl>Any other error will raise an exception.
// @comm If you use an overlapped buffer, then it is your responsibility
// to ensure the string object passed remains valid until the operation
// completes.  If Python garbage collection reclaims the buffer before the
// win32 API has finished with it, the results are unpredictable.
PyObject *MyWriteFile(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile;
	HANDLE hFile;
	PyObject *obWriteData;
	PyObject *obOverlapped = NULL;

	if (!PyArg_ParseTuple(args, "OO|O:WriteFile",
		&obhFile, // @pyparm <o PyHANDLE>/int|hFile||Handle to the file
		&obWriteData, // @pyparm string/<o PyOVERLAPPEDReadBuffer>|data||The data to write.
		&obOverlapped))	// @pyparm <o PyOVERLAPPED>|ol|None|An overlapped structure
		return NULL;
	PyWinBufferView pybuf(obWriteData);
	if (!pybuf.ok())
		return NULL;

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
	ok = WriteFile(hFile, pybuf.ptr(), pybuf.len(), &numWritten, pOverlapped);
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

// @pyswig |LockFileEx|Locks a file. Wrapper for LockFileEx win32 API.
static PyObject *
MyLockFileEx(PyObject *self, PyObject *args)
{
	OVERLAPPED *pOverlapped;
	PyObject *obhFile;
	HANDLE hFile;
	PyObject *obOverlapped = NULL;
    DWORD dwFlags, nbytesLow, nbytesHigh;

	if (!PyArg_ParseTuple(args, "OkkkO:LockFileEx",
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

    if (!PyArg_ParseTuple(args, "OkkO:UnlockFileEx",
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

%}

%{

// See Q192800 for an interesting discussion on overlapped and IOCP.

PyObject *PyWinObject_FromQueuedOVERLAPPED(OVERLAPPED *p)
{
	if (p==NULL || p==(OVERLAPPED *)-1) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	// We know this is a pointer to an OVERLAPPED inside a PyObject
	// extract it back out.
	size_t off = offsetof(PyOVERLAPPED, m_overlapped);
	PyOVERLAPPED *po = (PyOVERLAPPED *)(((LPBYTE)p) - off);
	// Hope like hell it hasn't already died on us (PostQueuedCompletionStatus
	// makes it impossible it has died, but other functions do not as they
	// don't know if the OVERLAPPED will end up in a IOCP)
	// Also check it is a valid write pointer (we don't write to it, but all
	// PyObjects are writable, so that extra check is worthwhile)
	// This is NOT foolproof - screw up reference counting and things may die!
	if (po->ob_refcnt<=0 || po->ob_type==0 || IsBadWritePtr(po, sizeof(PyOVERLAPPED))) {
		PyErr_SetString(PyExc_RuntimeError, "This overlapped object has lost all its references so was destroyed");
		return NULL;
	}
	// consume reference added when it was posted, if added.
	if (po->m_overlapped.isArtificialReference)
		po->m_overlapped.isArtificialReference = FALSE;
	else
		// Overlapped we didn't actually queue so no artificial refcount
		Py_INCREF(po);
	return po;
}

BOOL PyWinObject_AsQueuedOVERLAPPED(PyObject *ob, OVERLAPPED **ppOverlapped, BOOL bNoneOK = TRUE)
{
	PyOVERLAPPED *po = NULL;
	if (!PyWinObject_AsPyOVERLAPPED(ob, &po, bNoneOK))
		return FALSE;
	if (bNoneOK && po==NULL) {
		*ppOverlapped = NULL;
		return TRUE;
	}
	assert(po);
	if (!po)
		return FALSE;

	PyOVERLAPPED *pO = (PyOVERLAPPED *)po;
	// Add a fake reference so the object lives while in the queue, and add the flag
	Py_INCREF(ob);
	pO->m_overlapped.isArtificialReference = TRUE;
	*ppOverlapped = po->GetOverlapped();
	return TRUE;
}

// @pyswig (int, int, int, <o PyOVERLAPPED>)|GetQueuedCompletionStatus|Attempts to dequeue an I/O completion packet from a specified input/output completion port.
// @comm This method never throws an API error.
// <nl>The result is a tuple of (rc, numberOfBytesTransferred, completionKey, overlapped)
// <nl>If the function succeeds, rc will be set to 0, otherwise it will be set to the win32 error code.
static PyObject *myGetQueuedCompletionStatus(PyObject *self, PyObject *args)
{
	PyObject *obHandle;
	DWORD timeout;
	// @pyparm <o PyHANDLE>|hPort||The handle to the completion port.
	// @pyparm int|timeOut||Timeout in milli-seconds.
	if (!PyArg_ParseTuple(args, "Ol", &obHandle, &timeout))
		return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	OVERLAPPED *pOverlapped = NULL;
	UINT errCode;
	Py_BEGIN_ALLOW_THREADS
	BOOL ok = GetQueuedCompletionStatus(handle, &bytes, &key, &pOverlapped, timeout);
	errCode = ok ? 0 : GetLastError();
	Py_END_ALLOW_THREADS
	PyObject *rc = Py_BuildValue("ilNN", errCode, bytes,
		PyWinObject_FromULONG_PTR(key),
		PyWinObject_FromQueuedOVERLAPPED(pOverlapped));
	return rc;
}

// @pyswig None|PostQueuedCompletionStatus|lets you post an I/O completion packet to an I/O completion port. The I/O completion packet will satisfy an outstanding call to the GetQueuedCompletionStatus function.
PyObject *myPostQueuedCompletionStatus(PyObject *self, PyObject *args)
{
	PyObject *obHandle, *obOverlapped = NULL, *obkey=Py_None;
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	// @pyparm <o PyHANDLE>|handle||handle to an I/O completion port
	// @pyparm int|numberOfBytes|0|value to return via GetQueuedCompletionStatus' first result
	// @pyparm int|completionKey|0|value to return via GetQueuedCompletionStatus' second result
	// @pyparm <o PyOVERLAPPED>|overlapped|None|value to return via GetQueuedCompletionStatus' third result
	if (!PyArg_ParseTuple(args, "O|iOO", &obHandle, &bytesTransferred, &obkey, &obOverlapped))
		return NULL;
	if (obkey!=Py_None)
		if (!PyWinLong_AsVoidPtr(obkey, (void **)&key))
			return NULL;
	HANDLE handle;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;
	OVERLAPPED *pOverlapped;
	if (!PyWinObject_AsQueuedOVERLAPPED(obOverlapped, &pOverlapped, TRUE))
		return NULL;
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ::PostQueuedCompletionStatus(handle, bytesTransferred, key, pOverlapped);
	Py_END_ALLOW_THREADS
	if (!ok)
		return PyWin_SetAPIError("PostQueuedCompletionStatus");
	Py_INCREF(Py_None);
	return Py_None;
	// @comm Note that if you post overlapped objects, but your post is closed
	// before all pending requests are processed, the overlapped objects
	// (including its 'handle' and 'object' members) will leak.
	// See MS KB article Q192800 for a summary of this.
}

%}

%native (GetQueuedCompletionStatus) myGetQueuedCompletionStatus;
%native (PostQueuedCompletionStatus) myPostQueuedCompletionStatus;

%native(ReadFile) MyReadFile;
%native(WriteFile) MyWriteFile;
%native(CloseHandle) MyCloseHandle;

// @pyswig int|GetFileType|Determines the type of a file.
unsigned long GetFileType( // DWORD
    PyHANDLE hFile // @pyparm <o PyHANDLE>|hFile||The handle to the file.
);
#define FILE_TYPE_UNKNOWN FILE_TYPE_UNKNOWN // The type of the specified file is unknown.
#define FILE_TYPE_DISK FILE_TYPE_DISK // The specified file is a disk file.
#define FILE_TYPE_CHAR FILE_TYPE_CHAR // The specified file is a character file, typically an LPT device or a console.
#define FILE_TYPE_PIPE FILE_TYPE_PIPE // The specified file is either a named or anonymous pipe.

// @pyswig int|GetLogicalDrives|Returns a bitmaks of the logical drives installed.
unsigned long GetLogicalDrives( // DWORD
);

/**
GetLogicalDriveStrings
GetShortPathName
GetTempFileName
GetTempPath
GetVolumeInformation
*/

// @pyswig int|GetOverlappedResult|Determines the result of the most recent call with an OVERLAPPED object.
// @comm The result is the number of bytes transferred.  The overlapped object's attributes will be changed during this call.
BOOLAPI GetOverlappedResult(
	PyHANDLE hFile, 	// @pyparm <o PyHANDLE>|hFile||The handle to the pipe or file
	OVERLAPPED *lpOverlapped, // @pyparm <o PyOVERLAPPED>|overlapped||The overlapped object to check.
	unsigned long *OUTPUT, // lpNumberOfBytesTransferred
	BOOL bWait	// @pyparm int|bWait||Indicates if the function should wait for data to become available.
);

// @pyswig |LockFile|Locks a specified file for exclusive access by the calling process.
BOOLAPI LockFile(
    PyHANDLE hFile,	// @pyparm <o PyHANDLE>|hFile||handle of file to lock
    DWORD dwFileOffsetLow,	// @pyparm int|offsetLow||low-order word of lock region offset
    DWORD dwFileOffsetHigh,	// @pyparm int|offsetHigh||high-order word of lock region offset
    DWORD nNumberOfBytesToLockLow,	// @pyparm int|nNumberOfBytesToLockLow||low-order word of length to lock
    DWORD nNumberOfBytesToLockHigh 	// @pyparm int|nNumberOfBytesToLockHigh||high-order word of length to lock
   );

%native(LockFileEx) MyLockFileEx;

// @pyswig |MoveFile|Renames an existing file or a directory (including all its children).
BOOLAPI MoveFile(
    TCHAR *lpExistingFileName,	// @pyparm string|existingFileName||Name of the existing file
    TCHAR *lpNewFileName 	// @pyparm string|newFileName||New name for the file
);
// @pyswig |MoveFileW|Renames an existing file or a directory (including all its children).
BOOLAPI MoveFileW(
    WCHAR *lpExistingFileName,	// @pyparm string|existingFileName||Name of the existing file
    WCHAR *lpNewFileName 	// @pyparm string|newFileName||New name for the file
);

// @pyswig |MoveFileEx|Renames an existing file or a directory (including all its children).
BOOLAPI MoveFileEx(
    TCHAR *lpExistingFileName,	// @pyparm string|existingFileName||Name of the existing file
    TCHAR *INPUT_NULLOK, 	// @pyparm string|newFileName||New name for the file, can be None for delayed delete operation
    DWORD dwFlags 	        // @pyparm int|flags||flag to determine how to move file (win32file.MOVEFILE_*)
);
// @pyswig |MoveFileExW|Renames an existing file or a directory (including all its children).
BOOLAPI MoveFileExW(
    WCHAR *lpExistingFileName,	// @pyparm string|existingFileName||Name of the existing file
    WCHAR *INPUT_NULLOK, 	// @pyparm string|newFileName||New name for the file, can be None for delayed delete operation
    DWORD dwFlags 	        // @pyparm int|flags||flag to determine how to move file (win32file.MOVEFILE_*)
);
#define MOVEFILE_COPY_ALLOWED MOVEFILE_COPY_ALLOWED // If the file is to be moved to a different volume, the function simulates the move by using the CopyFile and DeleteFile functions. Cannot be combined with the MOVEFILE_DELAY_UNTIL_REBOOT flag.
#define MOVEFILE_DELAY_UNTIL_REBOOT MOVEFILE_DELAY_UNTIL_REBOOT // Windows NT only: The function does not move the file until the operating system is restarted. The system moves the file immediately after AUTOCHK is executed, but before creating any paging files. Consequently, this parameter enables the function to delete paging files from previous startups.
#define MOVEFILE_REPLACE_EXISTING MOVEFILE_REPLACE_EXISTING // If a file of the name specified by lpNewFileName already exists, the function replaces its contents with those specified by lpExistingFileName.
#define MOVEFILE_WRITE_THROUGH MOVEFILE_WRITE_THROUGH // Windows NT only: The function does not return until the file has actually been moved on the disk. Setting this flag guarantees that a move performed as a copy and delete operation is flushed to disk before the function returns. The flush occurs at the end of the copy operation.<nl>This flag has no effect if the MOVEFILE_DELAY_UNTIL_REBOOT flag is set.
#define MOVEFILE_CREATE_HARDLINK MOVEFILE_CREATE_HARDLINK
#define MOVEFILE_FAIL_IF_NOT_TRACKABLE MOVEFILE_FAIL_IF_NOT_TRACKABLE

// @pyswig string|QueryDosDevice|Returns the mapping for a device name, or all device names
%native (QueryDosDevice) MyQueryDosDevice;
%{
static PyObject *MyQueryDosDevice(PyObject *self, PyObject *args)
{
	PyObject *obdevicename, *ret=NULL;
	TCHAR *devicename, *targetpath=NULL;
	DWORD retlen, buflen, err;
	// @pyparm string|DeviceName||Name of device to query, or None to return all defined devices
	// @rdesc Returns a string containing substrings separated by NULLs with 2 terminating NULLs
	if (!PyArg_ParseTuple(args, "O:QueryDosDevice", &obdevicename))
		return NULL;
	if (!PyWinObject_AsTCHAR(obdevicename, &devicename, TRUE))
		return NULL;

	if (devicename==NULL)	// this returns a huge string
		buflen=8192;
	else
		buflen=256;
	// function returns ERROR_INSUFFICIENT_BUFFER with no indication of how much memory is actually needed
	while (true){
		if (targetpath){
			free(targetpath);
			buflen*=2;
			}
		targetpath=(TCHAR *)malloc(buflen *sizeof(TCHAR));
		if (targetpath==NULL){
			PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buflen);
			break;
			}
		retlen=QueryDosDevice(devicename, targetpath, buflen);
		if (retlen!=0){
			ret=PyWinObject_FromTCHAR(targetpath, retlen);
			break;
			}
		err=GetLastError();
		if (err!=ERROR_INSUFFICIENT_BUFFER){
			PyWin_SetAPIError("QueryDosDevice",err);
			break;
			}
		}
	PyWinObject_FreeTCHAR(devicename);
	if (targetpath)
		free(targetpath);
	return ret;
}
%}

%{
static PyObject *PyObject_FromFILE_NOTIFY_INFORMATION(void *buffer, DWORD nbytes)
{
	FILE_NOTIFY_INFORMATION *p = (FILE_NOTIFY_INFORMATION *)buffer;
	PyObject *ret = PyList_New(0);
	// comparing against the sizeof(FILE_NOTIFY_INFORMATION) fails when
	// the filename is exactly 1 byte!  Not clear the best way to
	// check this, but this works for now - is it at least the size of
	// the *head* of the struct.
	if (nbytes < sizeof(DWORD)*3+2)
		return ret;
	DWORD nbytes_read = 0;
	while (1) {
		PyObject *fname = PyWinObject_FromOLECHAR(p->FileName, p->FileNameLength/sizeof(WCHAR));
		if (!fname) {
			Py_DECREF(ret);
			return NULL;
		}
		PyObject *ob = Py_BuildValue("iN", p->Action, fname);
		if (ob==NULL) {
			Py_DECREF(ret);
			return NULL;
		}
		PyList_Append(ret, ob);
		Py_DECREF(ob);
		if (p->NextEntryOffset==0)
			break;
		p = (FILE_NOTIFY_INFORMATION *)(((BYTE *)p) + p->NextEntryOffset);
		nbytes_read += p->NextEntryOffset;
		if (nbytes_read > nbytes) {
			PyErr_SetString(PyExc_RuntimeError, "internal error decoding - running off end of buffer before seeing end-of-buffer marker");
			Py_DECREF(ret);
			return NULL;
		}
	 }
	 return ret;
}

// @pyswig |ReadDirectoryChangesW|retrieves information describing the changes occurring within a directory.
static PyObject *PyReadDirectoryChangesW(PyObject *self, PyObject *args)
{
	BOOL ok;
	HANDLE handle;
	BOOL bWatchSubtree;
	DWORD filter;
	DWORD bytes_returned;
	PyObject *obBuffer, *obhandle;
	PyObject *ret = NULL;
	PyObject *obOverlapped = Py_None;
	PyObject *obOverlappedRoutine = Py_None;
	if (!PyArg_ParseTuple(args, "OOii|OO:ReadDirectoryChangesW",
	                      &obhandle, // @pyparm <o PyHANDLE>|handle||Handle to the directory to be monitored. This directory must be opened with the FILE_LIST_DIRECTORY access right.
	                      &obBuffer, // @pyparm int|size||Size of the buffer to allocate for the results.
	                      &bWatchSubtree, // @pyparm int|bWatchSubtree||Specifies whether the ReadDirectoryChangesW function will monitor the directory or the directory tree. If TRUE is specified, the function monitors the directory tree rooted at the specified directory. If FALSE is specified, the function monitors only the directory specified by the hDirectory parameter.
	                      &filter, // @pyparm int|dwNotifyFilter||Specifies filter criteria the function checks to determine if the wait operation has completed. This parameter can be one or more of the FILE_NOTIFY_CHANGE_* values.
	                      &obOverlapped, // @pyparm <o PyOVERLAPPED>|overlapped|None|An overlapped object.  The directory must also be opened with FILE_FLAG_OVERLAPPED.
	                      &obOverlappedRoutine))
		return NULL;

	// @comm If you pass an overlapped object, you almost certainly
	// must pass a buffer object for the asynchronous results - failure
	// to do so may crash Python as the asynchronous result writes to
	// invalid memory.
	OVERLAPPED *pOverlapped = NULL;
	if (obOverlapped && obOverlapped != Py_None)
		if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
			return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	// Todo: overlappedRoutine support.
	if (obOverlappedRoutine != Py_None)
		return PyErr_Format(PyExc_ValueError, "overlappedRoutine must be None");

	PyWinBufferView pybuf;
	void *buf = NULL;
	DWORD bufSize = 0;
	BOOL bBufMallocd = FALSE;
	bufSize = PyLong_AsUnsignedLong(obBuffer);
	if ((bufSize!=(DWORD)-1) || !PyErr_Occurred()){
		buf = malloc(bufSize);
		if (buf==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating read buffer");
			goto done;
		}
		bBufMallocd = TRUE;
	}
	else{
		PyErr_Clear();
		if (!pybuf.init(obBuffer, true)) {
			PyErr_SetString(PyExc_TypeError, "buffer param must be an integer or a buffer object");
			goto done;
			}
		buf = pybuf.ptr();
		bufSize = pybuf.len();
		}

	// OK, have a buffer and a size.
	Py_BEGIN_ALLOW_THREADS
	ok = ::ReadDirectoryChangesW(handle, buf, bufSize, bWatchSubtree, filter, &bytes_returned, pOverlapped, NULL);
	Py_END_ALLOW_THREADS
	if (!ok) {
		return PyWin_SetAPIError("ReadDirectoryChangesW");
		goto done;
	}
	// If they passed a size, we return the buffer, already unpacked.
	if (bBufMallocd) {
		ret = PyObject_FromFILE_NOTIFY_INFORMATION(buf, bytes_returned);
	} else {
		// asynch call - bytes_returned is undefined - so just return None.
		ret = Py_None;
		Py_INCREF(Py_None);
	}
    // @rdesc If a buffer size is passed, the result is a list of (action, filename)
    // @rdesc If a buffer is passed, the result is None - you must use the overlapped
    // object to determine when the information is available and how much is valid.
    // The buffer can then be passed to <om win32file.FILE_NOTIFY_INFORMATION>
    // @comm The FILE_NOTIFY_INFORMATION structure used by this function
    // is variable length, depending on the length of the filename.
    // The size of the buffer must be at least 6 bytes long + the length
    // of the filenames returned.  The number of notifications that can be
    // returned for a given buffer size depends on the filename lengths.
done:
	if (bBufMallocd && buf)
		free(buf);
	return ret;
}

// @pyswig [(action, filename), ...|FILE_NOTIFY_INFORMATION|Decodes a PyFILE_NOTIFY_INFORMATION buffer.
PyObject *PyFILE_NOTIFY_INFORMATION(PyObject *self, PyObject *args)
{
	// @pyparm string|buffer||The buffer to decode.
	// @pyparm int|size||The number of bytes to refer to.  Generally this
	// will be smaller than the size of the buffer (and certainly never greater!)
	// @comm See <om win32file.ReadDirectoryChangesW> for more information.
	DWORD size;
	PyObject *obbuf;
	if (!PyArg_ParseTuple(args, "Oi", &obbuf, &size))
		return NULL;
	PyWinBufferView pybuf(obbuf);
	if (!pybuf.ok())
		return NULL;
	if (size > pybuf.len())
		return PyErr_Format(PyExc_ValueError, "buffer is only %d bytes long, but %d bytes were requested",
		                    pybuf.len(), size);

	return PyObject_FromFILE_NOTIFY_INFORMATION(pybuf.ptr(), size);
}

%}
%native(ReadDirectoryChangesW) PyReadDirectoryChangesW;
%native(FILE_NOTIFY_INFORMATION) PyFILE_NOTIFY_INFORMATION;

// ReadFileEx
// SearchPath

// @pyswig |SetCurrentDirectory|Sets the current directory.
%name(SetCurrentDirectory) BOOLAPI SetCurrentDirectoryW(
    WCHAR *lpPathName	// @pyparm str/string|lpPathName||Name of the path to set current.
);

// @pyswig |SetEndOfFile|Moves the end-of-file (EOF) position for the specified file to the current position of the file pointer.
BOOL SetEndOfFile(
    PyHANDLE hFile	// @pyparm <o PyHANDLE>|hFile||handle of file whose EOF is to be set
);

// @pyswig |SetFileApisToANSI|Causes a set of Win32 file functions to use the ANSI character set code page. This function is useful for 8-bit console input and output operations.
void SetFileApisToANSI(void);

// @pyswig |SetFileApisToOEM|Causes a set of Win32 file functions to use the OEM character set code page. This function is useful for 8-bit console input and output operations.
void SetFileApisToOEM(void);

// @pyswig |SetFileAttributes|Changes a file's attributes.
BOOLAPI SetFileAttributes(
    TCHAR *lpFileName,	// @pyparm string|filename||filename
    DWORD dwFileAttributes 	// @pyparm int|newAttributes||attributes to set
);


%{
// @pyswig |SetFilePointer|Moves the file pointer of an open file.
PyObject *MySetFilePointer(PyObject *self, PyObject *args)
{
	PyObject *obHandle, *obOffset;
	DWORD iMethod;
	HANDLE handle;
	if (!PyArg_ParseTuple(args, "OOl:SetFilePointer",
			&obHandle,  // @pyparm <o PyHANDLE>|handle||The file to perform the operation on.
			&obOffset, // @pyparm <o Py_LARGEINTEGER>|offset||Offset to move the file pointer.
			&iMethod)) // @pyparm int|moveMethod||Starting point for the file pointer move. This parameter can be one of the following values.
			              // @flagh Value|Meaning
			              // @flag FILE_BEGIN|The starting point is zero or the beginning of the file.
			              // @flag FILE_CURRENT|The starting point is the current value of the file pointer.
			              // @flag FILE_END|The starting point is the current end-of-file position.

		return NULL;
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
		return NULL;

	LARGE_INTEGER offset;
	if (!PyWinObject_AsLARGE_INTEGER(obOffset, &offset))
		return NULL;

    Py_BEGIN_ALLOW_THREADS
	offset.LowPart = SetFilePointer(handle, offset.LowPart, &offset.HighPart, iMethod);
    Py_END_ALLOW_THREADS
	// If we failed ...
	if (offset.LowPart == 0xFFFFFFFF &&
	    GetLastError() != NO_ERROR )
		return PyWin_SetAPIError("SetFilePointer");
	return PyWinObject_FromLARGE_INTEGER(offset);
}
%}
%native(SetFilePointer) MySetFilePointer;

#define FILE_BEGIN FILE_BEGIN
#define FILE_END FILE_END
#define FILE_CURRENT FILE_CURRENT

// @pyswig |SetVolumeLabel|Sets a volume label for a disk drive.
BOOLAPI SetVolumeLabel(
    TCHAR *lpRootPathName,	// @pyparm string|rootPathName||address of name of root directory for volume
    TCHAR *lpVolumeName 	// @pyparm string|volumeName||name for the volume
   );

// @pyswig |UnlockFile|Unlocks a region of a file locked by <om win32file.LockFile> or <om win32file.LockFileEx>
BOOLAPI UnlockFile(
    PyHANDLE hFile,	// @pyparm <o PyHANDLE>|hFile||handle of file to unlock
    DWORD dwFileOffsetLow,	// @pyparm int|offsetLow||low-order word of lock region offset
    DWORD dwFileOffsetHigh,	// @pyparm int|offsetHigh||high-order word of lock region offset
    DWORD nNumberOfBytesToUnlockLow,	// @pyparm int|nNumberOfBytesToUnlockLow||low-order word of length to unlock
    DWORD nNumberOfBytesToUnlockHigh 	// @pyparm int|nNumberOfBytesToUnlockHigh||high-order word of length to unlock
   );

%native(UnlockFileEx) MyUnlockFileEx;

// File Handle / File Descriptor APIs.
// @pyswig long|_get_osfhandle|Gets operating-system file handle associated with existing stream
// @pyparm int|fd||File descriptor as returned by file.fileno()
%name(_get_osfhandle)
PyObject *myget_osfhandle( int filehandle );

// @pyswig int|_open_osfhandle|Associates a C run-time file handle with a existing operating-system file handle.
// @pyparm <o PyHANDLE>|osfhandle||An open file handle
// @pyparm int|flags||O_APPEND,O_RDONLY, or O_TEXT
%name(_open_osfhandle)
PyObject *myopen_osfhandle ( PyHANDLE osfhandle, int flags );


%{
PyObject *myget_osfhandle (int filehandle)
{
  HANDLE result = (HANDLE)_get_osfhandle (filehandle);
  if (result == (HANDLE)-1)
    return PyErr_SetFromErrno(PyExc_IOError);

  return PyWinLong_FromHANDLE(result);
}

PyObject *myopen_osfhandle (PyHANDLE osfhandle, int flags)
{
  int result = _open_osfhandle ((ULONG_PTR)osfhandle, flags);
  if (result == -1)
    return PyErr_SetFromErrno(PyExc_IOError);

  return PyLong_FromLong(result);
}

%}

// @pyswig int|_setmaxstdio|Set the maximum allowed number of open stdio handles
// @rdesc Returns the number that was set, or -1 on failure.
int _setmaxstdio(
   int newmax	// @pyparm int|newmax||Maximum number of open stdio streams, 2048 max
);

// @pyswig int| _getmaxstdio|Returns the maximum number of CRT io streams.
int _getmaxstdio( void );

%{
// @pyswig |TransmitFile|Transmits a file over a socket
// TransmitFile(sock, filehandle, bytes_to_write, bytes_per_send, overlap, flags [, (prepend_buf, postpend_buf)])
// @rdesc Returns 0 on completion, or ERROR_IO_PENDING if an overlapped operation has been queued
static PyObject *py_TransmitFile( PyObject *self, PyObject *args, PyObject *kwargs ) {
	PyObject *obhFile;
	HANDLE hFile;
	SOCKET s;
	PyObject *obOverlapped = NULL;
	PyObject *obSocket;
	PyObject *obHead=Py_None, *obTail=Py_None;
	DWORD flags, bytes_to_write, bytes_per_send;
	OVERLAPPED *pOverlapped;
	int error, rc;

	static char *keywords[]={"Socket","File","NumberOfBytesToWrite", "NumberOfBytesPerSend",
		"Overlapped","Flags","Head","Tail", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOiiOi|OO:TransmitFile", keywords,
		&obSocket, // @pyparm <o PySocket>/int|Socket||Socket that will be used to send the file
		&obhFile, // @pyparm <o PyHANDLE>/int|File||Handle to the file
		&bytes_to_write, // @pyparm int|NumberOfBytesToWrite||The number of bytes in the file to transmit, use 0 for entire file.
		&bytes_per_send, // @pyparm int|NumberOfBytesPerSend||The size, in bytes, of each block of data sent in each send operation.
		&obOverlapped, // @pyparm <o PyOVERLAPPED>|Overlapped||An overlapped structure, can be None.
		&flags, // @pyparm int|Flags||A set of flags used to modify the behavior of the TransmitFile function call. (win32file.TF_*)
		&obHead, // @pyparm buffer|Head|None|Buffer to send on the socket before the file
		&obTail))	// @pyparm buffer|Tail|None|Buffer to send on the socket after the file
		return NULL;

	if (!PySocket_AsSOCKET(obSocket, &s)) {
		return NULL;
	}
	GUID guid = WSAID_TRANSMITFILE;
	DWORD dwBytes;
	LPFN_TRANSMITFILE lpfnTransmitFile = NULL;

	error = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID),
				   &lpfnTransmitFile, sizeof(lpfnTransmitFile), &dwBytes, NULL, NULL);
	if (error == SOCKET_ERROR) {
		rc = WSAGetLastError();
		PyWin_SetAPIError("WSAIoctl", rc);
		return NULL;
	}
	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped, TRUE))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhFile, &hFile)) {
		return NULL;
	}
	TRANSMIT_FILE_BUFFERS tf_buffers;
	TRANSMIT_FILE_BUFFERS *ptf_buffers;
	PyWinBufferView head(obHead, false, true);
	if (!head.ok())
		return NULL;
	tf_buffers.Head = head.ptr();
	tf_buffers.HeadLength = head.len();
	PyWinBufferView tail(obTail, false, true);
	if (!tail.ok())
		return NULL;
	tf_buffers.Tail = tail.ptr();
	tf_buffers.TailLength = tail.len();

	if (tf_buffers.Head || tf_buffers.Tail)
		ptf_buffers = &tf_buffers;
	else
		ptf_buffers = NULL;

	rc=0;
	Py_BEGIN_ALLOW_THREADS;
	if (!lpfnTransmitFile(s, hFile, bytes_to_write, bytes_per_send, pOverlapped, ptf_buffers, flags))
		rc = WSAGetLastError();
	Py_END_ALLOW_THREADS;

	if (rc == 0 || rc == ERROR_IO_PENDING || rc == WSA_IO_PENDING)
		return PyLong_FromLong(rc);
	return PyWin_SetAPIError("TransmitFile", rc);
}
PyCFunction pfnpy_TransmitFile=(PyCFunction)py_TransmitFile;
%}
%native(TransmitFile) pfnpy_TransmitFile;

////////////////////////////////////////////////////////////////////////////////
%{
// @pyswig (int, int)|ConnectEx|Version of connect that uses Overlapped I/O
// ConnectEx(sock, (addr, port), buf, overlap)
// @rdesc Returns the completion code and number of bytes sent.
//	The completion code will be 0 for a completed operation, or ERROR_IO_PENDING for a pending overlapped operation.
// @rdesc If the platform does not support ConnectEx (eg, Windows 2000), an
// exception will be thrown indicating the WSAIoctl function (which is used to
// fetch the function pointer) failed with error code WSAEINVAL (10022).
static PyObject *py_ConnectEx( PyObject *self, PyObject *args, PyObject *kwargs ) {
	OVERLAPPED *pOverlapped = NULL;
	SOCKET sConnecting;
	PyObject *obOverlapped = NULL;
	PyObject *obConnecting = NULL;
	PyObject *obBuf = Py_None;
	PyObject *addro;
	int rc, error;
	DWORD sent=0;
	static char *keywords[]={"s","name","Overlapped","SendBuffer", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOO|O:ConnectEx", keywords,
		&obConnecting, // @pyparm <o PySocket>/int|s||A bound, unconnected socket that will be used to connect
		&addro, // @pyparm tuple|name||Address to connect to (host, port)
		&obOverlapped, // @pyparm <o PyOVERLAPPED>|Overlapped||An overlapped structure
		&obBuf)) // @pyparm buffer|SendBuffer|None|Buffer to send on the socket after connect
		return NULL;

	if (!PySocket_AsSOCKET(obConnecting, &sConnecting)) {
		return NULL;
	}
	PyWinBufferView pybuf(obBuf, false, true); // None Ok
	if (!pybuf.ok())
		return NULL;

	GUID guid = WSAID_CONNECTEX;
	DWORD dwBytes;
	LPFN_CONNECTEX lpfnConnectEx = NULL;

	error = WSAIoctl(sConnecting, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID),
				   &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
	if (error == SOCKET_ERROR) {
		rc = WSAGetLastError();
		PyWin_SetAPIError("WSAIoctl", rc);
		return NULL;
	}
	// convert the address
	//
	char pbuf[30];
	char *hptr, *pptr;
	PyObject *hobj = NULL;
	PyObject *pobj = (PyObject *)NULL;
	TmpPyObject host_idna, port_idna;

	struct addrinfo hints, *res;
	if (!PyArg_ParseTuple(addro, "OO:getaddrinfo", &hobj, &pobj)) {
		return NULL;
	}
	if (hobj == Py_None) {
		hptr = NULL;
	} else if (PyUnicode_Check(hobj)) {
		host_idna = PyObject_CallMethod(hobj, "encode", "s", "idna");
		if (!host_idna)
			return NULL;
		hptr = PyBytes_AsString(host_idna);
	} else if (PyBytes_Check(hobj)) {
		hptr = PyBytes_AsString(hobj);
	} else {
		PyErr_SetString(PyExc_TypeError,
				"getaddrinfo() argument 1 must be string or None");
		return NULL;
	}

	if (pobj == Py_None) {
		pptr = NULL;
	} else if (PyUnicode_Check(pobj)) {
		port_idna = PyObject_CallMethod(pobj, "encode", "s", "idna");
		if (!port_idna)
			return NULL;
		pptr = PyBytes_AsString(port_idna);
	} else if (PyBytes_Check(pobj)) {
		pptr = PyBytes_AsString(pobj);
	} else if (PyLong_Check(pobj)) {
		PyOS_snprintf(pbuf, sizeof(pbuf), "%ld", PyLong_AsLong(pobj));
		pptr = pbuf;
	} else {
		PyErr_SetString(PyExc_TypeError, "Port must be int, string, or None");
		return NULL;
	}

	WSAPROTOCOL_INFO prot_info;
	int prot_info_len = sizeof(WSAPROTOCOL_INFO);
	error = getsockopt(sConnecting, SOL_SOCKET, SO_PROTOCOL_INFO,
						(char*)&prot_info, &prot_info_len);
	if (error)
	{
		PyWin_SetAPIError("getsockopt", WSAGetLastError());
		return NULL;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = prot_info.iAddressFamily;
	hints.ai_socktype = prot_info.iSocketType;
	hints.ai_protocol = prot_info.iProtocol;
	error = WspiapiGetAddrInfo(hptr, pptr, &hints, &res);
	if (error)
	{
		PyWin_SetAPIError("getaddrinfo", WSAGetLastError());
		return NULL;
	}
	// done screwing with the address

	if (!PyWinObject_AsOVERLAPPED(obOverlapped, &pOverlapped))
	{
		WspiapiFreeAddrInfo(res);
		return NULL;
	}

	rc=0;
	Py_BEGIN_ALLOW_THREADS;
	if (!lpfnConnectEx(sConnecting, res->ai_addr, (int)res->ai_addrlen, pybuf.ptr(), pybuf.len(), &sent, pOverlapped))
		rc=WSAGetLastError();
	Py_END_ALLOW_THREADS;
	WspiapiFreeAddrInfo(res);
	if (rc==0 || rc == ERROR_IO_PENDING)
		return Py_BuildValue("ii", rc, sent);
	return PyWin_SetAPIError("ConnectEx", rc);
}
PyCFunction pfnpy_ConnectEx=(PyCFunction)py_ConnectEx;
%}
%native(ConnectEx) pfnpy_ConnectEx;

////////////////////////////////////////////////////////////////////////////////
%native(AcceptEx) MyAcceptEx;

%native(GetAcceptExSockaddrs) MyGetAcceptExSockaddrs;

%{
// @pyswig |AcceptEx|Version of accept that uses Overlapped I/O
// @rdesc The result is 0 or ERROR_IO_PENDING.  All other values will raise
// win32file.error.  Specifically: if the win32 function returns FALSE,
// WSAGetLastError() is checked for ERROR_IO_PENDING.
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
	PyObject *obListening = NULL;
	PyObject *obAccepting = NULL;
	PyObject *obBuf = NULL;
	DWORD cBytesRecvd = 0;
	BOOL ok;
	int rc = 0;
	int iMinBufferSize = (sizeof(SOCKADDR_IN) + 16) * 2;
	WSAPROTOCOL_INFO wsProtInfo;
	UINT cbSize = sizeof(wsProtInfo);

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
	// @ex To have sAccepting inherit the properties of sListening, you need to do the following after a connection is successfully accepted|
	// import struct
	// sAccepting.setsockopt(socket.SOL_SOCKET, win32file.SO_UPDATE_ACCEPT_CONTEXT, struct.pack("I", sListening.fileno()))
	// @comm Pass a buffer of exactly the size returned by <om win32file.CalculateSocketEndPointSize>
	// to have AcceptEx return without reading any bytes from the remote connection.

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
		PyWin_SetAPIError("getsockopt", WSAGetLastError());
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

	PyWinBufferView pybuf(obBuf, true);
	if (!pybuf.ok())
		return NULL;
	if (pybuf.len() < (DWORD)iMinBufferSize ) {
		PyErr_Format(
			PyExc_ValueError,
			"Second param must be at least %ld bytes long",
			iMinBufferSize);
		return NULL;
		}

	// Phew... finally, all the arguments are converted...
	Py_BEGIN_ALLOW_THREADS
	ok = AcceptEx(
		sListening,
		sAccepting,
		pybuf.ptr(),
		pybuf.len() - iMinBufferSize,
		wsProtInfo.iMaxSockAddr + 16,
		wsProtInfo.iMaxSockAddr + 16,
		&cBytesRecvd,
		pOverlapped);
	Py_END_ALLOW_THREADS
	if (!ok)
	{
		rc = WSAGetLastError();
		if (rc != ERROR_IO_PENDING)
			return PyWin_SetAPIError("AcceptEx", WSAGetLastError());
	}
	return PyLong_FromLong(rc);
}

static PyObject *
MyMakeIPAddr(SOCKADDR_IN *addr)
{
	long x = ntohl(addr->sin_addr.s_addr);
	char buf[100];
	sprintf(buf, "%d.%d.%d.%d",
		(int) (x>>24) & 0xff, (int) (x>>16) & 0xff,
		(int) (x>> 8) & 0xff, (int) (x>> 0) & 0xff);
	return PyBytes_FromString(buf);
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
		return Py_BuildValue("iN",
			addr->sa_family,
			PyBytes_FromStringAndSize(addr->sa_data,sizeof(addr->sa_data)));

	}
}

// @pyswig int|CalculateSocketEndPointSize|Calculate how many bytes are needed for the connection endpoints data for a socket.
PyObject *MyCalculateSocketEndPointSize(PyObject *self, PyObject *args)
{
	// @comm This function allows you to determine the minumum buffer size
	// which can be passed to <om win32file.AcceptEx>
	PyObject *obs;
	// @pyparm <o PySocket>/int|socket||The socket for which to determine the size.
	if (!PyArg_ParseTuple(args, "O", &obs))
		return NULL;
	SOCKET s;
	if (!PySocket_AsSOCKET(obs, &s))
		return NULL;

	WSAPROTOCOL_INFO wsProtInfo;
	UINT cbSize = sizeof(wsProtInfo);
	int rc;

	// Grab the protocol information for the socket
	Py_BEGIN_ALLOW_THREADS
	rc = getsockopt(
		s,
		SOL_SOCKET,
		SO_PROTOCOL_INFO,
		(char *)&wsProtInfo,
		(int *)&cbSize);
	Py_END_ALLOW_THREADS
	if (rc == SOCKET_ERROR)
	{
		PyWin_SetAPIError("getsockopt", WSAGetLastError());
		return NULL;
	}
	return PyLong_FromLong((wsProtInfo.iMaxSockAddr + 16) * 2);
}
%}

%native(CalculateSocketEndPointSize) MyCalculateSocketEndPointSize;

%{
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
	PyObject *pORB = NULL;
	INT cbLocal = 0;
	INT cbRemote = 0;
	SOCKADDR_IN *psaddrIN = NULL;
	PyObject *obTemp = NULL;
	int rc;

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
			PyWin_SetAPIError("getsockopt", WSAGetLastError());
			return NULL;
	}
	iMinBufferSize = (wsProtInfo.iMaxSockAddr + 16) * 2;
	PyWinBufferView pybuf(obBuf);
	if (!pybuf.ok())
		return NULL;

	if (pybuf.len() < (DWORD)iMinBufferSize )
	{
		PyErr_Format(
			PyExc_ValueError,
			"Second param must be at least %ld bytes long",
			iMinBufferSize);
		goto Error;
	}

	cbRemote = cbLocal = wsProtInfo.iMaxSockAddr + 16;
	Py_BEGIN_ALLOW_THREADS
	GetAcceptExSockaddrs(
		pybuf.ptr(),
		pybuf.len() - iMinBufferSize,
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
	obTemp = PyLong_FromLong((LONG)psaddrLocal->sa_family);
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

%native(WSAEnumNetworkEvents) MyWSAEnumNetworkEvents;

%{
static int
MyCopyEvent(PyObject *dict, WSANETWORKEVENTS *events, long event, int eventbit)
{
	int res = 0;

	if (events->lNetworkEvents & event)
	{
		PyObject *key, *value;

		key = PyLong_FromLong(event);
		if (key == NULL)
		{
			return -1;
		}
		value = PyLong_FromLong(events->iErrorCode[eventbit]);
		if (value == NULL)
		{
			Py_DECREF(key);
			return -1;
		}
		res = PyDict_SetItem(dict, key, value);
		Py_DECREF(key);
		Py_DECREF(value);
	}
	return res;
}

// @pyswig dict|WSAEnumNetworkEvents|Return network events that caused the event associated with the socket to be signaled.
// @rdesc A dictionary mapping network events that occurred for the specified socket since the last call to this function (e.g. FD_READ, FD_WRITE) to their associated error code, or 0 if the event occurred without an error. The events returned are a subset of events previously registered for this socket with WSAEventSelect.
static PyObject*
MyWSAEnumNetworkEvents(PyObject *self, PyObject *args)
{
	PyObject *socket, *event = NULL;
	// @pyparm <o PySocket>|s||Socket to check for netork events, previously registered for network event notification with WSAEventSelect.
	SOCKET s;
	// @pyparm <o PyHANDLE>|hEvent||Optional handle to the event associated with socket s in the last call to WSAEventSelect. If specified, the event will be reset.
	HANDLE hEvent = NULL;
	WSANETWORKEVENTS wsaevents;
	int rc;
	PyObject *events;

	if (!PyArg_ParseTuple(args, "O|O:WSAEnumNetworkEvents", &socket, &event))
	{
		return NULL;
	}
	if (!PySocket_AsSOCKET(socket, &s))
	{
		return NULL;
	}
	if (event != NULL && !PyWinObject_AsHANDLE(event, &hEvent))
	{
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS;
	rc = WSAEnumNetworkEvents(s, hEvent, &wsaevents);
	Py_END_ALLOW_THREADS;
	if (rc == SOCKET_ERROR)
	{
		PyWin_SetAPIError("WSAEnumNetworkEvents", WSAGetLastError());
		return NULL;
	}

	events = PyDict_New();
	if (events == NULL)
	{
		return NULL;
	}
	if (MyCopyEvent(events, &wsaevents, FD_READ, FD_READ_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_WRITE, FD_WRITE_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_OOB, FD_OOB_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_ACCEPT, FD_ACCEPT_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_CONNECT, FD_CONNECT_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_CLOSE, FD_CLOSE_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_QOS, FD_QOS_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_GROUP_QOS, FD_GROUP_QOS_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_ROUTING_INTERFACE_CHANGE, FD_ROUTING_INTERFACE_CHANGE_BIT) ||
	    MyCopyEvent(events, &wsaevents, FD_ADDRESS_LIST_CHANGE, FD_ADDRESS_LIST_CHANGE_BIT))
	{
		Py_DECREF(events);
		return NULL;
	}
	return events;
}
%}

%{

PyObject* MyWSAAsyncSelect
(
	SOCKET *s,
	HWND hwnd,
	LONG wMsg,
	LONG lNetworkEvents
)
{
	int rc;
	Py_BEGIN_ALLOW_THREADS;
	rc = WSAAsyncSelect(*s, hwnd, wMsg, lNetworkEvents);
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

// @pyswig |WSAAsyncSelect|Request windows message notification for the supplied set of FD_XXXX network events.
%name(WSAAsyncSelect) PyObject *MyWSAAsyncSelect
(
	SOCKET *s, // @pyparm <o PySocket>|socket||socket to attach to the event
	HWND hwnd, // @pyparm <o hwnd>|hwnd||Window handle for the socket to become attached to.
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

	PyWinBufferView pybuf(obBuf);
	if (!pybuf.ok())
		return NULL;
	wsBuf.buf = (CHAR *)pybuf.ptr();
	wsBuf.len = pybuf.len();

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

	obTemp = PyLong_FromLong(rc);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 0, obTemp);
	obTemp = NULL;

	obTemp = PyLong_FromLong(cbSent);
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

	PyWinBufferView pybuf(obBuf, true);
	if (!pybuf.ok())
		return NULL;
	wsBuf.buf = (CHAR *)pybuf.ptr();
	wsBuf.len = pybuf.len();

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
			PyWin_SetAPIError("WSARecv", rc);
			goto Error;
		}
	}

	rv = PyTuple_New(2);
	if (rv == NULL)
	{
		goto Error;
	}

	obTemp = PyLong_FromLong(rc);
	if (obTemp == NULL)
	{
		goto Error;
	}
	PyTuple_SET_ITEM(rv, 0, obTemp);
	obTemp = NULL;

	obTemp = PyLong_FromLong(cbRecvd);
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
#define SO_UPDATE_CONNECT_CONTEXT SO_UPDATE_CONNECT_CONTEXT
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
#define WSAENOBUFS WSAENOBUFS
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
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
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
// @pyswig |WaitCommEvent|Waits for an event to occur for a specified communications device. The set of events that are monitored by this function is contained in the event mask associated with the device handle.
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
	if (!PyWinObject_AsHANDLE(obHandle, &handle))
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
		return PyWin_SetAPIError("WaitCommEvent", rc);
	return Py_BuildValue("ll", rc, *pmask);
	// @rdesc The result is a tuple of (rc, mask_val), where rc is zero for success, or
	// the result of calling GetLastError() otherwise.  The mask_val is the new mask value
	// once the function has returned, but if an Overlapped object is passed, this value
	// will generally be meaningless.  See the comments for more details.
	// @comm If an overlapped structure is passed, then the <om PyOVERLAPPED.dword>
	// address is passed to the Win32 API as the mask.  This means that once the
	// overlapped operation has completed, this dword attribute can be used to
	// determine the type of event that occurred.
}
%}
%native (WaitCommEvent) MyWaitCommEvent;

// Some Win2k specific volume mounting functions, thanks to Roger Upole
%{
#define CHECK_PFN(fname) if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);

typedef BOOL (WINAPI *GetVolumeNameForVolumeMountPointfunc)(LPCWSTR, LPCWSTR, DWORD);
static GetVolumeNameForVolumeMountPointfunc pfnGetVolumeNameForVolumeMountPoint = NULL;
typedef BOOL (WINAPI *SetVolumeMountPointfunc)(LPCWSTR, LPCWSTR);
static SetVolumeMountPointfunc pfnSetVolumeMountPoint = NULL;
typedef BOOL (WINAPI *DeleteVolumeMountPointfunc)(LPCWSTR);
static DeleteVolumeMountPointfunc pfnDeleteVolumeMountPoint = NULL;
typedef BOOL (WINAPI *GetVolumePathNamefunc)(WCHAR *, WCHAR *, DWORD);
static GetVolumePathNamefunc pfnGetVolumePathName=NULL;
typedef BOOL (WINAPI *GetVolumePathNamesForVolumeNamefunc)(LPCWSTR,LPWSTR,DWORD,PDWORD);
static GetVolumePathNamesForVolumeNamefunc pfnGetVolumePathNamesForVolumeName = NULL;

typedef BOOL (WINAPI *EncryptFilefunc)(WCHAR *);
static EncryptFilefunc pfnEncryptFile=NULL;
typedef BOOL (WINAPI *DecryptFilefunc)(WCHAR *, DWORD);
static DecryptFilefunc pfnDecryptFile=NULL;
typedef BOOL (WINAPI *EncryptionDisablefunc)(WCHAR *, BOOL);
static EncryptionDisablefunc pfnEncryptionDisable=NULL;
typedef BOOL (WINAPI *FileEncryptionStatusfunc)(WCHAR *, LPDWORD);
static FileEncryptionStatusfunc pfnFileEncryptionStatus=NULL;
typedef DWORD (WINAPI *QueryUsersOnEncryptedFilefunc)(WCHAR *, PENCRYPTION_CERTIFICATE_HASH_LIST *);
static QueryUsersOnEncryptedFilefunc pfnQueryUsersOnEncryptedFile=NULL;
typedef BOOL (WINAPI *FreeEncryptionCertificateHashListfunc)(PENCRYPTION_CERTIFICATE_HASH_LIST);
static FreeEncryptionCertificateHashListfunc pfnFreeEncryptionCertificateHashList=NULL;
typedef DWORD (WINAPI *QueryRecoveryAgentsOnEncryptedFilefunc)(WCHAR *, PENCRYPTION_CERTIFICATE_HASH_LIST *);
static QueryRecoveryAgentsOnEncryptedFilefunc pfnQueryRecoveryAgentsOnEncryptedFile=NULL;
typedef DWORD (WINAPI *RemoveUsersFromEncryptedFilefunc)(WCHAR *, PENCRYPTION_CERTIFICATE_HASH_LIST);
static RemoveUsersFromEncryptedFilefunc pfnRemoveUsersFromEncryptedFile=NULL;
typedef DWORD (WINAPI *AddUsersToEncryptedFilefunc)(WCHAR *, PENCRYPTION_CERTIFICATE_LIST);
static AddUsersToEncryptedFilefunc pfnAddUsersToEncryptedFile=NULL;
typedef DWORD (WINAPI *DuplicateEncryptionInfoFilefunc)(LPWSTR,LPWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES);
static DuplicateEncryptionInfoFilefunc pfnDuplicateEncryptionInfoFile = NULL;

typedef BOOL (WINAPI *CreateHardLinkfunc)(LPWSTR, LPWSTR, LPSECURITY_ATTRIBUTES);
static CreateHardLinkfunc pfnCreateHardLink=NULL;
typedef BOOL (WINAPI *CreateHardLinkTransactedfunc)(LPWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, HANDLE);
static CreateHardLinkTransactedfunc pfnCreateHardLinkTransacted=NULL;
typedef BOOLEAN (WINAPI *CreateSymbolicLinkfunc)(LPWSTR,LPWSTR,DWORD);
static CreateSymbolicLinkfunc pfnCreateSymbolicLink=NULL;
typedef BOOLEAN (WINAPI *CreateSymbolicLinkTransactedfunc)(LPCWSTR,LPCWSTR,DWORD,HANDLE);
static CreateSymbolicLinkTransactedfunc pfnCreateSymbolicLinkTransacted=NULL;

typedef BOOL (WINAPI *BackupReadfunc)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL, BOOL, LPVOID*);
static BackupReadfunc pfnBackupRead=NULL;
typedef BOOL (WINAPI *BackupSeekfunc)(HANDLE, DWORD, DWORD, LPDWORD, LPDWORD, LPVOID*);
static BackupSeekfunc pfnBackupSeek=NULL;
typedef BOOL (WINAPI *BackupWritefunc)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL, BOOL, LPVOID*);
static BackupWritefunc pfnBackupWrite=NULL;

typedef BOOL (WINAPI *SetFileShortNamefunc)(HANDLE, LPCWSTR);
static SetFileShortNamefunc pfnSetFileShortName=NULL;
typedef BOOL (WINAPI *CopyFileExfunc)(LPWSTR,LPWSTR,LPPROGRESS_ROUTINE,LPVOID,LPBOOL,DWORD);
static CopyFileExfunc pfnCopyFileEx=NULL;
typedef BOOL (WINAPI *MoveFileWithProgressfunc)(LPWSTR,LPWSTR,LPPROGRESS_ROUTINE,LPVOID,DWORD);
static MoveFileWithProgressfunc pfnMoveFileWithProgress=NULL;
typedef BOOL (WINAPI *ReplaceFilefunc)(LPCWSTR,LPCWSTR,LPCWSTR,DWORD,LPVOID,LPVOID);
static ReplaceFilefunc pfnReplaceFile=NULL;

typedef DWORD (WINAPI *OpenEncryptedFileRawfunc)(LPCWSTR,ULONG,PVOID *);
static OpenEncryptedFileRawfunc pfnOpenEncryptedFileRaw=NULL;
typedef DWORD (WINAPI *ReadEncryptedFileRawfunc)(PFE_EXPORT_FUNC,PVOID,PVOID);
static ReadEncryptedFileRawfunc pfnReadEncryptedFileRaw=NULL;
typedef DWORD (WINAPI *WriteEncryptedFileRawfunc)(PFE_IMPORT_FUNC,PVOID,PVOID);
static WriteEncryptedFileRawfunc pfnWriteEncryptedFileRaw=NULL;
typedef void (WINAPI *CloseEncryptedFileRawfunc)(PVOID);
static CloseEncryptedFileRawfunc pfnCloseEncryptedFileRaw=NULL;

// Transactional NTFS functions
typedef HANDLE (WINAPI *CreateFileTransactedfunc)(LPWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE,HANDLE,PUSHORT,PVOID);
static CreateFileTransactedfunc pfnCreateFileTransacted=NULL;
typedef BOOL (WINAPI *DeleteFileTransactedfunc)(LPWSTR,HANDLE);
static DeleteFileTransactedfunc pfnDeleteFileTransacted=NULL;
typedef BOOL (WINAPI *MoveFileTransactedfunc)(LPWSTR,LPWSTR,LPPROGRESS_ROUTINE,LPVOID,DWORD,HANDLE);
static MoveFileTransactedfunc pfnMoveFileTransacted=NULL;
typedef BOOL (WINAPI *CopyFileTransactedfunc)(LPWSTR,LPWSTR,LPPROGRESS_ROUTINE,LPVOID,LPBOOL,DWORD,HANDLE);
static CopyFileTransactedfunc pfnCopyFileTransacted=NULL;

typedef DWORD (WINAPI *GetFileAttributesTransactedAfunc)(LPSTR,GET_FILEEX_INFO_LEVELS,LPVOID,HANDLE);
static GetFileAttributesTransactedAfunc pfnGetFileAttributesTransactedA=NULL;
typedef DWORD (WINAPI *GetFileAttributesTransactedWfunc)(LPWSTR,GET_FILEEX_INFO_LEVELS,LPVOID,HANDLE);
static GetFileAttributesTransactedWfunc pfnGetFileAttributesTransactedW=NULL;

typedef BOOL (WINAPI *SetFileAttributesTransactedfunc)(LPWSTR,DWORD,HANDLE);
static SetFileAttributesTransactedfunc pfnSetFileAttributesTransacted=NULL;
typedef BOOL (WINAPI *CreateDirectoryTransactedfunc)(LPWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,HANDLE);
static CreateDirectoryTransactedfunc pfnCreateDirectoryTransacted=NULL;
typedef BOOL (WINAPI *RemoveDirectoryTransactedfunc)(LPWSTR,HANDLE);
static RemoveDirectoryTransactedfunc pfnRemoveDirectoryTransacted=NULL;
typedef HANDLE (WINAPI *FindFirstFileTransactedfunc)(LPWSTR,FINDEX_INFO_LEVELS,LPVOID,FINDEX_SEARCH_OPS,LPVOID,DWORD,HANDLE);
static FindFirstFileTransactedfunc pfnFindFirstFileTransacted=NULL;

typedef HANDLE (WINAPI *FindFirstStreamfunc)(LPWSTR, STREAM_INFO_LEVELS, LPVOID, DWORD);
static FindFirstStreamfunc pfnFindFirstStream=NULL;
typedef BOOL (WINAPI *FindNextStreamfunc)(HANDLE, LPVOID);
static FindNextStreamfunc pfnFindNextStream=NULL;
typedef HANDLE (WINAPI *FindFirstStreamTransactedfunc)(LPWSTR, STREAM_INFO_LEVELS, LPVOID, DWORD, HANDLE);
static FindFirstStreamTransactedfunc pfnFindFirstStreamTransacted=NULL;
typedef HANDLE (WINAPI *FindFirstFileNamefunc)(LPCWSTR,DWORD,LPDWORD,PWCHAR);
static FindFirstFileNamefunc pfnFindFirstFileName = NULL;
typedef HANDLE (WINAPI *FindFirstFileNameTransactedfunc)(LPCWSTR,DWORD,LPDWORD,PWCHAR,HANDLE);
static FindFirstFileNameTransactedfunc pfnFindFirstFileNameTransacted = NULL;
typedef BOOL (WINAPI *FindNextFileNamefunc)(HANDLE,LPDWORD,PWCHAR);
static FindNextFileNamefunc pfnFindNextFileName = NULL;
typedef DWORD (WINAPI *GetFinalPathNameByHandlefunc)(HANDLE,LPWSTR,DWORD,DWORD);
static GetFinalPathNameByHandlefunc pfnGetFinalPathNameByHandle = NULL;
typedef DWORD (WINAPI *GetLongPathNamefunc)(LPCWSTR,LPWSTR,DWORD);
static GetLongPathNamefunc pfnGetLongPathName = NULL;
typedef DWORD (WINAPI *GetLongPathNameTransactedfunc)(LPCWSTR,LPWSTR,DWORD,HANDLE);
static GetLongPathNameTransactedfunc pfnGetLongPathNameTransacted = NULL;
typedef DWORD (WINAPI *GetFullPathNameTransactedWfunc)(LPCWSTR,DWORD,LPWSTR,LPWSTR*,HANDLE);
static GetFullPathNameTransactedWfunc pfnGetFullPathNameTransactedW = NULL;
typedef DWORD (WINAPI *GetFullPathNameTransactedAfunc)(LPCSTR,DWORD,LPSTR,LPSTR*,HANDLE);
static GetFullPathNameTransactedAfunc pfnGetFullPathNameTransactedA = NULL;

typedef BOOL (WINAPI *Wow64DisableWow64FsRedirectionfunc)(PVOID*);
static Wow64DisableWow64FsRedirectionfunc pfnWow64DisableWow64FsRedirection = NULL;
typedef BOOL (WINAPI *Wow64RevertWow64FsRedirectionfunc)(PVOID);
static Wow64RevertWow64FsRedirectionfunc pfnWow64RevertWow64FsRedirection = NULL;

typedef BOOL (WINAPI *GetFileInformationByHandleExfunc)(HANDLE,FILE_INFO_BY_HANDLE_CLASS,LPVOID,DWORD);
static GetFileInformationByHandleExfunc pfnGetFileInformationByHandleEx = NULL;
typedef BOOL (WINAPI *SetFileInformationByHandlefunc)(HANDLE,FILE_INFO_BY_HANDLE_CLASS,LPVOID,DWORD);
static SetFileInformationByHandlefunc pfnSetFileInformationByHandle = NULL;

typedef HANDLE (WINAPI *ReOpenFilefunc)(HANDLE, DWORD, DWORD, DWORD);
static ReOpenFilefunc pfnReOpenFile = NULL;

typedef HANDLE (WINAPI *OpenFileByIdfunc)(HANDLE, LPFILE_ID_DESCRIPTOR, DWORD, DWORD,
	LPSECURITY_ATTRIBUTES, DWORD);
static OpenFileByIdfunc pfnOpenFileById = NULL;

// From sfc.dll
typedef BOOL (WINAPI *SfcGetNextProtectedFilefunc)(HANDLE,PPROTECTED_FILE_DATA);
static SfcGetNextProtectedFilefunc pfnSfcGetNextProtectedFile = NULL;
typedef BOOL (WINAPI *SfcIsFileProtectedfunc)(HANDLE,LPCWSTR);
static SfcIsFileProtectedfunc pfnSfcIsFileProtected = NULL;


// @pyswig string|SetVolumeMountPoint|Mounts the specified volume at the specified volume mount point.
// @comm Accepts keyword args.
static PyObject*
py_SetVolumeMountPoint(PyObject	*self, PyObject	*args, PyObject *kwargs)
{
	// @ex Usage|SetVolumeMountPoint('h:\tmp\','c:\')
	// @comm Note that both	parameters must	have trailing backslashes.
	// @rdesc The result is	the	GUID of	the	volume mounted,	as a string.
	// @comm This method exists only on Windows 2000 or later.  On earlier platforms, NotImplementedError will be raised.
	CHECK_PFN(GetVolumeNameForVolumeMountPoint);
	CHECK_PFN(SetVolumeMountPoint);
	PyObject *ret=NULL;
	PyObject *volume_obj = NULL, *mount_point_obj =	NULL;
	WCHAR *volume =	NULL;
	WCHAR *mount_point = NULL;
	WCHAR volume_name[50];
	static char *keywords[]={"VolumeMountPoint", "VolumeName", NULL};
	if (!PyArg_ParseTupleAndKeywords(args,kwargs,"OO:SetVolumeMountPoint",keywords,
		&mount_point_obj,	// @pyparm string|VolumeMountPoint||The mount point - must be an existing empty directory on an NTFS volume
		&volume_obj))		// @pyparm string|VolumeName||The volume to	mount there
		return NULL;

	if (PyWinObject_AsWCHAR(mount_point_obj, &mount_point, false)
		&&PyWinObject_AsWCHAR(volume_obj, &volume, false)){
		if (!(*pfnGetVolumeNameForVolumeMountPoint)(volume,volume_name,sizeof(volume_name)/sizeof(volume_name[0])))
			PyWin_SetAPIError("GetVolumeNameForVolumeMountPoint");
		else if (!(*pfnSetVolumeMountPoint)(mount_point, volume_name))
			PyWin_SetAPIError("SetVolumeMountPoint");
		else
			ret=PyWinObject_FromWCHAR(volume_name);
		}
	PyWinObject_FreeWCHAR(volume);
	PyWinObject_FreeWCHAR(mount_point);
	return ret;
}
PyCFunction pfnpy_SetVolumeMountPoint=(PyCFunction)py_SetVolumeMountPoint;

// @pyswig |DeleteVolumeMountPoint|Unmounts the volume from the specified volume mount point.
// @comm Accepts keyword args.
static PyObject*
py_DeleteVolumeMountPoint(PyObject *self, PyObject *args, PyObject *kwargs)
{
	// @ex Usage|DeleteVolumeMountPoint('h:\tmp\')
	// @comm Throws	an error if	it is not a	valid mount	point, returns None	on success.
	// <nl>Use carefully - will	remove drive letter	assignment if no directory specified
	// @comm This method requires Windows 2000 or later.  On earlier platforms, NotImplementedError will be raised.
	CHECK_PFN(DeleteVolumeMountPoint);
	PyObject *ret=NULL;
	PyObject *mount_point_obj =	NULL;
	WCHAR *mount_point = NULL;
	static char *keywords[]={"VolumeMountPoint", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:DeleteVolumeMountPoint", keywords,
		&mount_point_obj))	// @pyparm string|VolumeMountPoint||The mount point to delete - must have a trailing backslash.
		return NULL;
	if (!PyWinObject_AsWCHAR(mount_point_obj, &mount_point,	FALSE))
		return NULL;

	if (!(*pfnDeleteVolumeMountPoint)(mount_point))
		PyWin_SetAPIError("DeleteVolumeMountPoint");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreeWCHAR(mount_point);
	return ret;
}
PyCFunction pfnpy_DeleteVolumeMountPoint=(PyCFunction)py_DeleteVolumeMountPoint;

// @pyswig string|GetVolumeNameForVolumeMountPoint|Returns unique volume name.
// @comm Requires Win2K or later.
// @comm Accepts keyword args.
static PyObject *py_GetVolumeNameForVolumeMountPoint(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *ret=NULL;
	PyObject *obvolume_name = NULL, *obmount_point = NULL;

	WCHAR *mount_point = NULL;
	WCHAR volume_name[50];
	CHECK_PFN(GetVolumeNameForVolumeMountPoint);
	static char *keywords[]={"VolumeMountPoint", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetVolumeNameForVolumeMountPoint", keywords,
		&obmount_point))	// @pyparm string|VolumeMountPoint||Volume mount point or root drive - trailing backslash required
		return NULL;
	if (!PyWinObject_AsWCHAR(obmount_point, &mount_point, false))
		return NULL;
	if (!(*pfnGetVolumeNameForVolumeMountPoint)(mount_point, volume_name, sizeof(volume_name)/sizeof(volume_name[0])))
		PyWin_SetAPIError("GetVolumeNameForVolumeMountPoint");
	else
		ret=PyWinObject_FromWCHAR(volume_name);
	PyWinObject_FreeWCHAR(mount_point);
	return ret;
}
PyCFunction pfnpy_GetVolumeNameForVolumeMountPoint=(PyCFunction)py_GetVolumeNameForVolumeMountPoint;

// @pyswig string|GetVolumePathName|Returns volume mount point for a path
// @comm Api gives no indication of how much memory is needed, so function assumes returned path
//       will not be longer that length of input path + 1.
//       Use GetFullPathName first for relative paths, or GetLongPathName for 8.3 paths.
//       Optional second parm can also be used to override the buffer size for returned path
// @comm Accepts keyword args.
static PyObject *py_GetVolumePathName(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *ret=NULL;
	PyObject *obpath = NULL;
	WCHAR *path=NULL, *mount_point=NULL;
	DWORD pathlen, bufsize=0;
	CHECK_PFN(GetVolumePathName);
	static char *keywords[]={"FileName","BufferLength", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|l:GetVolumePathName", keywords,
		&obpath,	// @pyparm string|FileName||File/dir for which to return volume mount point
		&bufsize))	// @pyparm int|BufferLength|0|Optional parm to allocate extra space for returned string
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE, &pathlen))
		return NULL;

	// yet another function that doesn't tell us how much memory it needs ...
	if (bufsize>0)
		bufsize+=1;
	else
		bufsize=pathlen+2;  // enough to accommodate trailing null, and possibly extra backslash
	mount_point=(WCHAR *)malloc(bufsize*sizeof(WCHAR));
	if (mount_point==NULL)
		PyErr_SetString(PyExc_MemoryError,"GetVolumePathName: Unable to allocate return buffer");
	else
		if (!(*pfnGetVolumePathName)(path, mount_point, bufsize))
			PyWin_SetAPIError("GetVolumePathName");
		else
			ret=PyWinObject_FromWCHAR(mount_point);
	if (path != NULL)
		PyWinObject_FreeWCHAR(path);
	if (mount_point!=NULL)
		free(mount_point);
	return ret;
}
PyCFunction pfnpy_GetVolumePathName=(PyCFunction)py_GetVolumePathName;

// @pyswig [string,...]|GetVolumePathNamesForVolumeName|Returns mounted paths for a volume
// @comm Requires WinXP or later
// @comm Accepts keyword args
static PyObject *py_GetVolumePathNamesForVolumeName(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obvolume, *ret=NULL;
	WCHAR *volume=NULL, *paths=NULL;
	// Preallocate for most common case: 'x:\\' + 2 nulls
	DWORD buf_len=5, reqd_len=0, err;
	static char *keywords[]={"VolumeName", NULL};
	CHECK_PFN(GetVolumePathNamesForVolumeName);
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetVolumePathNamesForVolumeName", keywords,
		&obvolume))		// @pyparm string|VolumeName||Name of a volume as returned by <om win32file.GetVolumeNameForVolumeMountPoint>
		return NULL;
	if (!PyWinObject_AsWCHAR(obvolume, &volume, FALSE))
		return NULL;

	while(true){
		if (paths)
			free(paths);
		paths=(WCHAR *)malloc(buf_len*sizeof(WCHAR));
		if (paths==NULL){
			PyErr_Format(PyExc_MemoryError,"Unable to allocate %d characters", buf_len);
			break;
			}
		if ((*pfnGetVolumePathNamesForVolumeName)(volume, paths, buf_len, &reqd_len)){
			ret=PyWinObject_FromMultipleString(paths);
			break;
			}
		err=GetLastError();
		if (err!=ERROR_MORE_DATA){
			PyWin_SetAPIError("GetVolumePathNamesForVolumeName", err);
			break;
			}
		buf_len=reqd_len+2;
		}
	PyWinObject_FreeWCHAR(volume);
	if (paths)
		free(paths);
	return ret;
}
PyCFunction pfnpy_GetVolumePathNamesForVolumeName=(PyCFunction)py_GetVolumePathNamesForVolumeName;

// @pyswig |CreateHardLink|Establishes an NTFS hard link between an existing file and a new file.
static PyObject*
py_CreateHardLink(PyObject *self, PyObject *args, PyObject *kwargs)
{
    // @comm  An NTFS hard link is similar to a POSIX hard link.
    // <nl>This function creates a second directory entry for an existing file, can be different name in
    // same directory or any name in a different directory.
    // Both file paths must be on the same NTFS volume.<nl>To remove the link, simply delete
    // it and the original file will still remain.
    // @ex Usage|CreateHardLink('h:\dir\newfilename.txt','h:\otherdir\existingfile.txt')
	// @comm This method exists on Windows 2000 and later.  Otherwise NotImplementedError will be raised.
	// @comm Accepts keyword args.
	// @comm If the Transaction parameter is specified, CreateHardLinkTransacted will be called (requires Vista or later)
	PyObject *ret=NULL;
	PyObject *new_file_obj;
	PyObject *existing_file_obj;
	PyObject *trans_obj=Py_None, *sa_obj = Py_None;
	WCHAR *new_file = NULL;
	WCHAR *existing_file = NULL;
	SECURITY_ATTRIBUTES *sa;
	HANDLE htrans;
	static char *keywords[]={"FileName","ExistingFileName","SecurityAttributes","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OO:CreateHardLink", keywords,
		&new_file_obj,		// @pyparm string|FileName||The name of the new directory entry to be created.
		&existing_file_obj,	// @pyparm string|ExistingFileName||The name of the existing file to which the new link will point.
		&sa_obj,			// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes|None|Optional SECURITY_ATTRIBUTES object. MSDN describes this parameter as reserved, so use only None
		&trans_obj))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction, as returned by <om win32transaction.CreateTransaction>
		return NULL;
	if (!PyWinObject_AsHANDLE(trans_obj, &htrans))
		return NULL;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(sa_obj, &sa, TRUE))
		return NULL;
	if (htrans){
		CHECK_PFN(CreateHardLinkTransacted);
		}
	else{
		CHECK_PFN(CreateHardLink);
		}
	if (PyWinObject_AsWCHAR(new_file_obj, &new_file, FALSE)
		&&PyWinObject_AsWCHAR(existing_file_obj, &existing_file, FALSE)){
		BOOL bsuccess;
		if (htrans)
			bsuccess=(*pfnCreateHardLinkTransacted)(new_file, existing_file, sa, htrans);
		else
			bsuccess=(*pfnCreateHardLink)(new_file, existing_file, sa);
		if (!bsuccess)
			PyWin_SetAPIError("CreateHardLink");
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(new_file);
	PyWinObject_FreeWCHAR(existing_file);
	return ret;
}
PyCFunction pfnpy_CreateHardLink=(PyCFunction)py_CreateHardLink;

// @pyswig |CreateSymbolicLink|Creates a symbolic link (reparse point)
static PyObject *py_CreateSymbolicLink(PyObject *self, PyObject *args, PyObject *kwargs)
{
	// @comm This method only exists on Vista and later.
	// @comm Accepts keyword args.
	// @comm Requires SeCreateSymbolicLink priv.
	// @comm If the Transaction parameter is passed in, CreateSymbolicLinkTransacted will be called
	WCHAR *linkname=NULL, *targetname=NULL;
	PyObject *oblinkname, *obtargetname, *obtrans=Py_None, *ret=NULL;
	DWORD flags=0;
	HANDLE htrans;
	static char *keywords[]={"SymlinkFileName","TargetFileName","Flags","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|kO:CreateSymbolicLink", keywords,
		&oblinkname,	// @pyparm string|SymlinkFileName||Path of the symbolic link to be created
		&obtargetname,	// @pyparm string|TargetFileName||The name of file to which link will point
		&flags,			// @pyparm int|Flags|0|SYMBOLIC_LINK_FLAG_DIRECTORY and SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE are the only defined flags
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction, as returned by <om win32transaction.CreateTransaction>
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
			return NULL;
	if (htrans){
		CHECK_PFN(CreateSymbolicLinkTransacted);
		}
	else{
		CHECK_PFN(CreateSymbolicLink);
		}

	if (PyWinObject_AsWCHAR(oblinkname, &linkname, FALSE) && PyWinObject_AsWCHAR(obtargetname, &targetname, FALSE)){
		BOOLEAN bsuccess;
		if (htrans)
			bsuccess=(*pfnCreateSymbolicLinkTransacted)(linkname, targetname, flags, htrans);
		else
			bsuccess=(*pfnCreateSymbolicLink)(linkname, targetname, flags);
		if (!bsuccess)
			PyWin_SetAPIError("CreateSymbolicLink");
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(linkname);
	PyWinObject_FreeWCHAR(targetname);
	return ret;
}
PyCFunction pfnpy_CreateSymbolicLink=(PyCFunction)py_CreateSymbolicLink;

// @pyswig |EncryptFile|Encrypts specified file (requires Win2k or higher and NTFS)
static PyObject*
py_EncryptFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(EncryptFile);
	// @pyparm string|filename||File to encrypt
	PyObject *ret=NULL, *obfname=NULL;
	WCHAR *fname = NULL;

	if (!PyArg_ParseTuple(args,"O:EncryptFile", &obfname))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!(*pfnEncryptFile)(fname))
		PyWin_SetAPIError("EncryptFile");
	else
		ret=Py_None;
	PyWinObject_FreeWCHAR(fname);
	Py_XINCREF(ret);
	return ret;
}

// @pyswig |DecryptFile|Decrypts specified file (requires Win2k or higher and NTFS)
static PyObject*
py_DecryptFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(DecryptFile);
	// @pyparm string|filename||File to decrypt
	PyObject *ret=NULL, *obfname=NULL;
	WCHAR *fname = NULL;
	DWORD reserved=0;

	if (!PyArg_ParseTuple(args,"O:DecryptFile", &obfname))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!(*pfnDecryptFile)(fname,reserved))
		PyWin_SetAPIError("DecryptFile");
	else
		ret=Py_None;
	PyWinObject_FreeWCHAR(fname);
	Py_XINCREF(ret);
	return ret;
}

// @pyswig |EncryptionDisable|Enables/disables encryption for a directory (requires Win2k or higher and NTFS)
static PyObject*
py_EncryptionDisable(PyObject *self, PyObject *args)
{
	CHECK_PFN(EncryptionDisable);
	// @pyparm string|DirName||Directory to enable or disable
	// @pyparm boolean|Disable||Set to False to enable encryption
	PyObject *ret=NULL, *obfname=NULL;
	WCHAR *fname = NULL;
	BOOL Disable;

	if (!PyArg_ParseTuple(args,"Oi:EncryptionDisable", &obfname, &Disable))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!(*pfnEncryptionDisable)(fname,Disable))
		PyWin_SetAPIError("EncryptionDisable");
	else
		ret=Py_None;
	PyWinObject_FreeWCHAR(fname);
	Py_XINCREF(ret);
	return ret;
}

// @pyswig int|FileEncryptionStatus|retrieves the encryption status of the specified file.
// @rdesc The result is documented as being one of FILE_ENCRYPTABLE,
// FILE_IS_ENCRYPTED, FILE_SYSTEM_ATTR, FILE_ROOT_DIR, FILE_SYSTEM_DIR,
// FILE_UNKNOWN, FILE_SYSTEM_NOT_SUPPORT, FILE_USER_DISALLOWED,
// or FILE_READ_ONLY
// @comm Requires Windows 2000 or higher.
static PyObject*
py_FileEncryptionStatus(PyObject *self, PyObject *args)
{
	CHECK_PFN(FileEncryptionStatus);
	// @pyparm string|FileName||file to query
	PyObject *ret=NULL, *obfname=NULL;
	WCHAR *fname = NULL;
	DWORD Status=0;

	if (!PyArg_ParseTuple(args,"O:FileEncryptionStatus", &obfname))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!(*pfnFileEncryptionStatus)(fname, &Status))
		PyWin_SetAPIError("FileEncryptionStatus");
	else
		ret=Py_BuildValue("i",Status);
	PyWinObject_FreeWCHAR(fname);
	return ret;
}

void PyWinObject_FreePENCRYPTION_CERTIFICATE_LIST(PENCRYPTION_CERTIFICATE_LIST pecl)
{
	DWORD cert_ind=0;
	PENCRYPTION_CERTIFICATE *ppec=NULL;
	if (pecl->pUsers != NULL){
		ppec=pecl->pUsers;
		for (cert_ind=0;cert_ind<pecl->nUsers;cert_ind++){
			if (*ppec != NULL){
				if ((*ppec)->pCertBlob != NULL)
					free ((*ppec)->pCertBlob);
					// don't free PENCRYPTION_CERTIFICATE->pCertBlob->pbData or PENCRYPTION_CERTIFICATE->pUserSid,
					// both have internal pointers from Python string and Sid objects
				free (*ppec);
				}
			ppec++;
			}
		free(pecl->pUsers);
		}
}

void PyWinObject_FreePENCRYPTION_CERTIFICATE_HASH_LIST(PENCRYPTION_CERTIFICATE_HASH_LIST pechl)
{
	DWORD hash_ind=0;
	PENCRYPTION_CERTIFICATE_HASH *ppech=NULL;
	if (pechl->pUsers != NULL){
		ppech=pechl->pUsers;
		for (hash_ind=0;hash_ind<pechl->nCert_Hash;hash_ind++){
			if (*ppech != NULL){
				// PENCRYPTION_CERTIFICATE_HASH->pHash->pbData and PENCRYPTION_CERTIFICATE_HASH->pUserSid
				// will be freed when corresponding python objects are deallocated
				if ((*ppech)->lpDisplayInformation != NULL)
					PyWinObject_FreeWCHAR((*ppech)->lpDisplayInformation);
				if ((*ppech)->pHash != NULL)
					free ((*ppech)->pHash);
				free (*ppech);
				}
			ppech++;
			}
		free(pechl->pUsers);
		}
}

PyObject *PyWinObject_FromPENCRYPTION_CERTIFICATE_LIST(PENCRYPTION_CERTIFICATE_LIST pecl)
{
	DWORD user_cnt;
	PENCRYPTION_CERTIFICATE *user_item=NULL;
	PyObject *ret_item=NULL;
	PyObject *ret=PyTuple_New(pecl->nUsers);
	if (!ret)
		return NULL;
	user_item=pecl->pUsers;
	for (user_cnt=0; user_cnt < pecl->nUsers; user_cnt++){
		ret_item=Py_BuildValue("NN",
			PyWinObject_FromSID((*user_item)->pUserSid),
			PyBytes_FromStringAndSize((char *)(*user_item)->pCertBlob->pbData, (*user_item)->pCertBlob->cbData));
		// ??? This doesn't return EFS_CERTIFICATE_BLOB.dwCertEncodingType ???
		if (!ret_item){
			Py_DECREF(ret);
			return NULL;
			}
		PyTuple_SET_ITEM(ret, user_cnt, ret_item);
		user_item++;
		}
	return ret;
}

PyObject *PyWinObject_FromPENCRYPTION_CERTIFICATE_HASH_LIST(PENCRYPTION_CERTIFICATE_HASH_LIST pechl)
{
	DWORD user_cnt;
	PENCRYPTION_CERTIFICATE_HASH *user_item=NULL;
	PyObject *ret_item=NULL;
	PyObject *ret=PyTuple_New(pechl->nCert_Hash);
	if (!ret)
		return NULL;
	user_item=pechl->pUsers;
	for (user_cnt=0; user_cnt < pechl->nCert_Hash; user_cnt++){
		ret_item=Py_BuildValue("NNu",
			PyWinObject_FromSID((*user_item)->pUserSid),
			PyBytes_FromStringAndSize((char *)(*user_item)->pHash->pbData, (*user_item)->pHash->cbData),
			(*user_item)->lpDisplayInformation);
		if (!ret_item){
			Py_DECREF(ret);
			return NULL;
			}
		PyTuple_SET_ITEM(ret, user_cnt, ret_item);
		user_item++;
		}
	return ret;
}

BOOL PyWinObject_AsPENCRYPTION_CERTIFICATE_LIST(PyObject *obcert_list, PENCRYPTION_CERTIFICATE_LIST pecl)
{
	char *format_msg="ENCRYPTION_CERTIFICATE_LIST must be represented as a sequence of sequences of (PySID, str, int dwCertEncodingType )";
	BOOL bSuccess=TRUE;
	DWORD cert_ind=0;
	PENCRYPTION_CERTIFICATE *ppec=NULL;
	PyObject *obcert=NULL;
	PyObject *obsid=NULL, *obcert_member=NULL;

	if (!PySequence_Check(obcert_list)){
		PyErr_SetString(PyExc_TypeError,format_msg);
		return FALSE;
		}
	Py_ssize_t ssize_cert_cnt=PySequence_Length(obcert_list);
	PYWIN_CHECK_SSIZE_DWORD(ssize_cert_cnt, FALSE);
	DWORD cert_cnt=(DWORD)ssize_cert_cnt;
	pecl->nUsers=cert_cnt;
	ppec=(PENCRYPTION_CERTIFICATE *)malloc(cert_cnt*sizeof(PENCRYPTION_CERTIFICATE));
	if (ppec==NULL){
		PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsENCRYPTION_CERTIFICATE_LIST: unable to allocate hash list");
		return NULL;
		}
	ZeroMemory(ppec,cert_cnt*sizeof(PENCRYPTION_CERTIFICATE));
	pecl->pUsers=ppec;

	for (cert_ind=0;cert_ind<cert_cnt;cert_ind++){
		obcert=PySequence_GetItem(obcert_list, cert_ind);
		if (!PySequence_Check(obcert)){
			PyErr_SetString(PyExc_TypeError,format_msg);
			bSuccess=FALSE;
			}
		if (bSuccess)
			if (PySequence_Length(obcert)!=3){
				PyErr_SetString(PyExc_TypeError,format_msg);
				bSuccess=FALSE;
				}
		if (bSuccess){
			*ppec=new(ENCRYPTION_CERTIFICATE);
			if (*ppec==NULL){
				PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsENCRYPTION_CERTIFICATE_LIST: unable to allocate ENCRYPTION_CERTIFICATE");
				bSuccess=FALSE;
				}
			}
		if (bSuccess){
			ZeroMemory(*ppec,sizeof(ENCRYPTION_CERTIFICATE));
			(*ppec)->cbTotalLength=sizeof(ENCRYPTION_CERTIFICATE);
			obcert_member=PySequence_GetItem(obcert,0);
			bSuccess=PyWinObject_AsSID(obcert_member, (PSID *)&((*ppec)->pUserSid));
			Py_DECREF(obcert_member);
			}

		if (bSuccess){
			(*ppec)->pCertBlob=new(EFS_CERTIFICATE_BLOB);
			if ((*ppec)->pCertBlob==NULL){
				PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsENCRYPTION_CERTIFICATE_LIST: unable to allocate EFS_CERTIFICATE_BLOB");
				bSuccess=FALSE;
				}
			}
		if (bSuccess){
			ZeroMemory((*ppec)->pCertBlob,sizeof(EFS_CERTIFICATE_BLOB));
			obcert_member=PySequence_GetItem(obcert,1);
			if (!PyLong_Check(obcert_member)){
				PyErr_SetString(PyExc_TypeError,"Second item (dwCertEncodingType) of ENCRYPTION_CERTIFICATE must be an integer");
				bSuccess=FALSE;
				}
			else
				(*ppec)->pCertBlob->dwCertEncodingType=PyLong_AsLong(obcert_member);
			Py_DECREF(obcert_member);
			}

		if (bSuccess){
			obcert_member=PySequence_GetItem(obcert,2);
			Py_ssize_t cbData;
			if (PyBytes_AsStringAndSize(obcert_member,
					(char **)&((*ppec)->pCertBlob->pbData),
					&cbData)==-1){
				PyErr_SetString(PyExc_TypeError,"Third item of ENCRYPTION_CERTIFICATE must be a string containing encoded certificate data");
				bSuccess=FALSE;
				} else {
					(*ppec)->pCertBlob->cbData = PyWin_SAFE_DOWNCAST(cbData, Py_ssize_t, DWORD);
				}
			Py_DECREF(obcert_member);
			}
		Py_DECREF(obcert);
		if (!bSuccess)
			break;
		ppec++;
		}
	return bSuccess;
}

BOOL PyWinObject_AsPENCRYPTION_CERTIFICATE_HASH_LIST(PyObject *obhash_list, PENCRYPTION_CERTIFICATE_HASH_LIST pechl)
{
	char *err_msg="ENCRYPTION_CERTIFICATE_HASH_LIST must be represented as a sequence of sequences of (PySID, bytes, string)";
	BOOL bSuccess=TRUE;
	DWORD hash_ind=0;
	PENCRYPTION_CERTIFICATE_HASH *ppech=NULL;
	PyObject *obsid=NULL, *obDisplayInformation=NULL, *obhash=NULL;
	PyObject *obhash_item=NULL;

	if (!PySequence_Check(obhash_list)){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	Py_ssize_t ssize_hash_cnt=PySequence_Length(obhash_list);
	PYWIN_CHECK_SSIZE_DWORD(ssize_hash_cnt, FALSE);
	DWORD hash_cnt=(DWORD)ssize_hash_cnt;

	pechl->nCert_Hash=hash_cnt;
	ppech=(PENCRYPTION_CERTIFICATE_HASH *)malloc(hash_cnt*sizeof(PENCRYPTION_CERTIFICATE_HASH));
	if (ppech==NULL){
		PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsENCRYPTION_CERTIFICATE_HASH_LIST: unable to allocate ENCRYPTION_CERTIFICATE_HASH_LIST");
		return FALSE;
		}
	ZeroMemory(ppech,hash_cnt*sizeof(PENCRYPTION_CERTIFICATE_HASH));
	pechl->pUsers=ppech;

	for (hash_ind=0;hash_ind<hash_cnt;hash_ind++){
		obhash=PySequence_GetItem(obhash_list, hash_ind);
		if (!PySequence_Check(obhash)){
			PyErr_SetString(PyExc_TypeError,err_msg);
			bSuccess=FALSE;
			}
		if (bSuccess)
			if (PySequence_Length(obhash)!=3){
				PyErr_SetString(PyExc_TypeError,err_msg);
				bSuccess=FALSE;
				}
		if (bSuccess){
			*ppech=new(ENCRYPTION_CERTIFICATE_HASH);
			if (*ppech==NULL){
				PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsPENCRYPTION_CERTIFICATE_HASH_LIST: unable to allocate EMCRYPTION_CERTIFICATE_HASH");
				bSuccess=FALSE;
				}
			}
		if (bSuccess){
			ZeroMemory(*ppech,sizeof(ENCRYPTION_CERTIFICATE_HASH));
			(*ppech)->cbTotalLength=sizeof(ENCRYPTION_CERTIFICATE_HASH);
			obhash_item=PySequence_GetItem(obhash,0);
			bSuccess=PyWinObject_AsSID(obhash_item, (PSID *)&((*ppech)->pUserSid));
			Py_DECREF(obhash_item);
			}

		if (bSuccess){
			(*ppech)->pHash=new(EFS_HASH_BLOB);
			if ((*ppech)->pHash==NULL){
				PyErr_SetString(PyExc_MemoryError,"PyWinObject_AsPENCRYPTION_CERTIFICATE_HASH_LIST: unable to allocate EFS_HASH_BLOB");
				bSuccess=FALSE;
				}
			}

		if (bSuccess){
			ZeroMemory((*ppech)->pHash,sizeof(EFS_HASH_BLOB));
			obhash_item=PySequence_GetItem(obhash,1);
			Py_ssize_t cbData;
			if (PyBytes_AsStringAndSize(obhash_item,
				(char **)&((*ppech)->pHash->pbData),
				&cbData)==-1){
				PyErr_SetString(PyExc_TypeError,"Second item of ENCRYPTION_CERTIFICATE_HASH tuple must be a string containing encoded certificate data");
				bSuccess=FALSE;
				} else {
					(*ppech)->pHash->cbData = PyWin_SAFE_DOWNCAST(cbData, Py_ssize_t, DWORD);
				}
			Py_DECREF(obhash_item);
			}

		if (bSuccess){
			obhash_item=PySequence_GetItem(obhash,2);
			bSuccess=PyWinObject_AsWCHAR(obhash_item, &(*ppech)->lpDisplayInformation);
			Py_DECREF(obhash_item);
			}
		Py_DECREF(obhash);
		if (!bSuccess)
			break;
		ppech++;
		}
	return bSuccess;
}


// @pyswig (<o PySID>,bytes,string)|QueryUsersOnEncryptedFile|Returns list of users for an encrypted file as tuples of (SID, certificate hash blob, display info)
static PyObject*
py_QueryUsersOnEncryptedFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(QueryUsersOnEncryptedFile);
	// @pyparm string|FileName||file to query
	PyObject *ret=NULL, *obfname=NULL, *ret_item=NULL;
	WCHAR *fname=NULL;
	DWORD err=0;
	PyObject *obsid=NULL, *obDisplayInformation=NULL;
	PENCRYPTION_CERTIFICATE_HASH_LIST pechl=NULL;

	if (!PyArg_ParseTuple(args,"O:QueryUsersOnEncryptedFile", &obfname))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	err=(*pfnQueryUsersOnEncryptedFile)(fname, &pechl);
	if (err != ERROR_SUCCESS)
		PyWin_SetAPIError("QueryUsersOnEncryptedFile",err);
	else
		ret=PyWinObject_FromPENCRYPTION_CERTIFICATE_HASH_LIST(pechl);

	if (fname!=NULL)
		PyWinObject_FreeWCHAR(fname);
	if (pechl!=NULL)
		(*pfnFreeEncryptionCertificateHashList)(pechl);
	return ret;
}

// @pyswig (<o PySID>,bytes,string)|QueryRecoveryAgentsOnEncryptedFile|Lists recovery agents for file as a tuple of tuples.
// @rdesc The result is a tuple of tuples - ((SID, certificate hash blob, display info),....)
static PyObject*
py_QueryRecoveryAgentsOnEncryptedFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(QueryRecoveryAgentsOnEncryptedFile);
	// @pyparm string|FileName||file to query
	PyObject *ret=NULL, *obfname=NULL, *ret_item=NULL;
	WCHAR *fname=NULL;
	DWORD user_cnt=0, err=0;
	PyObject *obsid=NULL, *obDisplayInformation=NULL;
	PENCRYPTION_CERTIFICATE_HASH_LIST pechl=NULL;
	PENCRYPTION_CERTIFICATE_HASH *user_item=NULL;
	if (!PyArg_ParseTuple(args,"O:QueryRecoveryAgentsOnEncryptedFile", &obfname))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	err=(*pfnQueryRecoveryAgentsOnEncryptedFile)(fname, &pechl);
	if (err != ERROR_SUCCESS)
		PyWin_SetAPIError("QueryRecoveryAgentsOnEncryptedFile",err);
	else
		ret=PyWinObject_FromPENCRYPTION_CERTIFICATE_HASH_LIST(pechl);

	if (fname!=NULL)
		PyWinObject_FreeWCHAR(fname);
	if (pechl!=NULL)
		(*pfnFreeEncryptionCertificateHashList)(pechl);
	return ret;
}

// @pyswig |RemoveUsersFromEncryptedFile|Removes specified certificates from file - if certificate is not found, it is ignored
static PyObject*
py_RemoveUsersFromEncryptedFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(RemoveUsersFromEncryptedFile);
	// @pyparm string|FileName||File from which to remove users
	// @pyparm ((<o PySID>,bytes,string),...)|pHashes||Sequence representing an ENCRYPTION_CERTIFICATE_HASH_LIST structure, as returned by QueryUsersOnEncryptedFile
	PyObject *ret=NULL, *obfname=NULL, *obechl=NULL;
	WCHAR *fname=NULL;
	DWORD err=0;
	ENCRYPTION_CERTIFICATE_HASH_LIST echl;
	ZeroMemory(&echl,sizeof(ENCRYPTION_CERTIFICATE_HASH_LIST));
	if (!PyArg_ParseTuple(args,"OO:RemoveUsersFromEncryptedFile", &obfname, &obechl))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!PyWinObject_AsPENCRYPTION_CERTIFICATE_HASH_LIST(obechl,&echl))
		goto done;

	err=(*pfnRemoveUsersFromEncryptedFile)(fname, &echl);
	if (err != ERROR_SUCCESS)
		PyWin_SetAPIError("RemoveUsersFromEncryptedFile",err);
	else
		ret=Py_None;
	done:
	PyWinObject_FreePENCRYPTION_CERTIFICATE_HASH_LIST(&echl);
	if (fname!=NULL)
		PyWinObject_FreeWCHAR(fname);
	Py_XINCREF(ret);
	return ret;
}

// @pyswig |AddUsersToEncryptedFile|Allows user identified by SID and EFS certificate access to decrypt specified file
static PyObject*
py_AddUsersToEncryptedFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(AddUsersToEncryptedFile);
	// @pyparm string|FileName||File that additional users will be allowed to decrypt
	// @pyparm ((<o PySID>,string,int),...)|pUsers||Sequence representing
	// ENCRYPTION_CERTIFICATE_LIST - elements are sequences consisting of
	// users' Sid, encoded EFS certficate (user must export a .cer to obtain
	// this data), and encoding type (usually 1 for X509_ASN_ENCODING)
	PyObject *ret=NULL, *obfname=NULL, *obecl=NULL;
	WCHAR *fname=NULL;
	DWORD err=0;
	ENCRYPTION_CERTIFICATE_LIST ecl;
	ZeroMemory(&ecl,sizeof(ENCRYPTION_CERTIFICATE_LIST));
	if (!PyArg_ParseTuple(args,"OO:AddUsersToEncryptedFile", &obfname, &obecl))
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	if (!PyWinObject_AsPENCRYPTION_CERTIFICATE_LIST(obecl,&ecl))
		return NULL;

	err=(*pfnAddUsersToEncryptedFile)(fname, &ecl);
	if (err != ERROR_SUCCESS)
		PyWin_SetAPIError("AddUsersToEncryptedFile",err);
	else
		ret=Py_None;
	if (fname!=NULL)
		PyWinObject_FreeWCHAR(fname);
	PyWinObject_FreePENCRYPTION_CERTIFICATE_LIST(&ecl);
	Py_XINCREF(ret);
	return ret;
}

// @pyswig |DuplicateEncryptionInfoFile|Duplicates EFS encryption from one file to another
// @pyseeapi DuplicateEncryptionInfoFile
// @comm Requires Windows XP or later
// @comm Accepts keyword arguments.
static PyObject *py_DuplicateEncryptionInfoFile(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(DuplicateEncryptionInfoFile);
	WCHAR *src=NULL, *dst=NULL;
	PSECURITY_ATTRIBUTES psa;
	PyObject *obsrc, *obdst, *obsa=Py_None, *ret=NULL;
	DWORD disp, attr;
	static char *keywords[]={"SrcFileName","DstFileName","CreationDisposition","Attributes","SecurityAttributes", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOkk|O:DuplicateEncryptionInfoFile", keywords,
		&obsrc,		// @pyparm string|SrcFileName||Encrypted file to read EFS metadata from
		&obdst,		// @pyparm string|DstFileName||File to be encrypted using EFS data from source file
		&disp,		// @pyparm int|CreationDisposition||Specifies whether an existing file should be overwritten (CREATE_NEW or CREATE_ALWAYS)
		&attr,		// @pyparm int|Attributes||File attributes
		&obsa))		// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes|None|Specifies security for destination file
		return NULL;

	if (PyWinObject_AsWCHAR(obsrc, &src, FALSE)
		&&PyWinObject_AsWCHAR(obdst, &dst, FALSE)
		&&PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE)){
		DWORD err=(*pfnDuplicateEncryptionInfoFile)(src, dst, disp, attr, psa);
		if (err==ERROR_SUCCESS){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("DuplicateEncryptionInfoFile", err);
		}
	PyWinObject_FreeWCHAR(src);
	PyWinObject_FreeWCHAR(dst);
	return ret;
}
PyCFunction pfnpy_DuplicateEncryptionInfoFile=(PyCFunction)py_DuplicateEncryptionInfoFile;

// @pyswig (int, buffer, int)|BackupRead|Reads streams of data from a file
// @comm Returns number of bytes read, data buffer, and context pointer for next operation
// If Buffer is None, a new buffer will be created of size NbrOfBytesToRead that can be passed
//	back in subsequent calls

static PyObject*
py_BackupRead(PyObject *self, PyObject *args)
{
	CHECK_PFN(BackupRead);
	// @pyparm <o PyHANDLE>|hFile||File handle opened by CreateFile
	// @pyparm int|NumberOfBytesToRead||Number of bytes to be read from file
	// @pyparm buffer|Buffer||Writeable buffer object that receives data read
	// @pyparm int|bAbort||If true, ends read operation and frees backup context
	// @pyparm int|bProcessSecurity||Indicates whether file's ACL stream should be read
	// @pyparm int|lpContext||Pass 0 on first call, then pass back value returned from last call thereafter
	HANDLE h;
	DWORD bytes_requested, bytes_read;
	BOOL bAbort,bProcessSecurity;
	LPVOID ctxt;
	PyObject *obbuf=NULL, *obbufout=NULL, *obh, *obctxt;

	if (!PyArg_ParseTuple(args, "OlOllO:BackupRead", &obh, &bytes_requested, &obbuf, &bAbort, &bProcessSecurity, &obctxt))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obctxt, &ctxt))
		return NULL;
	PyWinBufferView pybuf;
	if (obbuf==Py_None){
		obbufout=PyBuffer_New(bytes_requested);
		if (obbufout==NULL)
			return NULL;
		if (!pybuf.init(obbufout, true)) {
			Py_DECREF(obbufout);
			return NULL;
			}
		}
	else{
		obbufout=obbuf;
		if (!pybuf.init(obbufout, true))
			return NULL;
		if (pybuf.len() < bytes_requested)
			return PyErr_Format(PyExc_ValueError,"Buffer size (%d) less than requested read size (%d)", pybuf.len(), bytes_requested);
		Py_INCREF(obbufout);
		}
	if (!(*pfnBackupRead)(h, (PBYTE)pybuf.ptr(), bytes_requested, &bytes_read, bAbort, bProcessSecurity, &ctxt)){
		PyWin_SetAPIError("BackupRead");
		Py_DECREF(obbufout);
		return NULL;
		}
	return Py_BuildValue("lNN", bytes_read, obbufout, PyWinLong_FromVoidPtr(ctxt));
}

// @pyswig long|BackupSeek|Seeks forward in a file stream
// @comm Function will only seek to end of current stream, used to seek past bad data
//    or find beginning position for read of next stream
// Returns number of bytes actually moved
static PyObject*
py_BackupSeek(PyObject *self, PyObject *args)
{
	CHECK_PFN(BackupSeek);
	// @pyparm <o PyHANDLE>|hFile||File handle used by a BackupRead operation
	// @pyparm long|NumberOfBytesToSeek||Number of bytes to move forward in current stream
	// @pyparm int|lpContext||Context pointer returned from a BackupRead operation
	HANDLE h;
	ULARGE_INTEGER bytes_to_seek;
	ULARGE_INTEGER bytes_moved;
	LPVOID ctxt;
	PyObject *obbytes_to_seek, *obh, *obctxt;
	if (!PyArg_ParseTuple(args,"OOO:BackupSeek", &obh, &obbytes_to_seek, &obctxt))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obctxt, &ctxt))
		return NULL;
	if (!PyWinObject_AsULARGE_INTEGER(obbytes_to_seek, &bytes_to_seek))
		return NULL;
	bytes_moved.QuadPart=0;
	if (!(*pfnBackupSeek)(h, bytes_to_seek.LowPart, bytes_to_seek.HighPart,
	                   &bytes_moved.LowPart, &bytes_moved.HighPart,
	                   &ctxt)){
	    // function returns false if you attempt to seek past end of current stream, but file pointer
	    //   still moves to start of next stream - consider this as success
		if (bytes_moved.QuadPart==0){
			PyWin_SetAPIError("BackupSeek");
			return NULL;
			}
		}
	return PyWinObject_FromULARGE_INTEGER(bytes_moved);
}

// @pyswig (int,int)|BackupWrite|Restores file data
// @comm Returns number of bytes written and context pointer for next operation
static PyObject*
py_BackupWrite(PyObject *self, PyObject *args)
{
	CHECK_PFN(BackupWrite);
	// @pyparm <o PyHANDLE>|hFile||File handle opened by CreateFile
	// @pyparm int|NumberOfBytesToWrite||Length of data to be written to file
	// @pyparm string|Buffer||A string or buffer object that contains the data to be written
	// @pyparm int|bAbort||If true, ends write operation and frees backup context
	// @pyparm int|bProcessSecurity||Indicates whether ACL's should be restored
	// @pyparm int|lpContext||Pass 0 on first call, then pass back value returned from last call thereafter
	HANDLE h;
	DWORD bytes_to_write, bytes_written;
	BOOL bAbort, bProcessSecurity;
	LPVOID ctxt;
	PyObject *obbuf, *obh, *obctxt;

	if (!PyArg_ParseTuple(args, "OlOllO:BackupWrite", &obh, &bytes_to_write, &obbuf, &bAbort, &bProcessSecurity, &obctxt))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!PyWinLong_AsVoidPtr(obctxt, &ctxt))
		return NULL;
	PyWinBufferView pybuf(obbuf);
	if (!pybuf.ok())
		return NULL;
	if (pybuf.len() < bytes_to_write)
		return PyErr_Format(PyExc_ValueError,"Buffer size (%d) less than requested write size (%d)", pybuf.len(), bytes_to_write);

	if (!(*pfnBackupWrite)(h, (BYTE*)pybuf.ptr(), bytes_to_write, &bytes_written, bAbort, bProcessSecurity, &ctxt)){
		PyWin_SetAPIError("BackupWrite");
		return NULL;
		}
	return Py_BuildValue("lN", bytes_written, PyWinLong_FromVoidPtr(ctxt));
}

// @pyswig |SetFileShortName|Set the 8.3 name of a file
// @comm This function is only available on WinXP and later
// @comm File handle must be opened with FILE_FLAG_BACKUP_SEMANTICS, and SE_RESTORE_NAME privilege must be enabled
static PyObject*
py_SetFileShortName(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetFileShortName);
	HANDLE h;
	WCHAR *shortname=NULL;
	PyObject *obh, *obshortname;
	BOOL bsuccess;
	if (!PyArg_ParseTuple(args, "OO:SetFileShortName",
		&obh,			// @pyparm <o PyHANDLE>|hFile||Handle to a file or directory
		&obshortname))	// @pyparm string|ShortName||The 8.3 name to be applied to the file
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, &h))
		return NULL;
	if (!PyWinObject_AsWCHAR(obshortname, &shortname, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	bsuccess=(*pfnSetFileShortName)(h, (LPCWSTR)shortname);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeWCHAR(shortname);
	if (bsuccess){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("SetFileShortName");
}

// @object CopyProgressRoutine|Python function used as a callback for <om win32file.CopyFileEx> and <om win32file.MoveFileWithProgress><nl>
// Function will receive 9 parameters:<nl>
// (TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred,
//  StreamNumber, CallbackReason, SourceFile, DestinationFile, Data)<nl>
// SourceFile and DestinationFile are <o PyHANDLE>s.
// Data is the context object passed to the calling function.
// All others are longs.<nl>
// CallbackReason will be one of CALLBACK_CHUNK_FINISHED or CALLBACK_STREAM_SWITCH<nl>
// Your implementation of this function must return one of the PROGRESS_* constants.
DWORD CALLBACK CopyFileEx_ProgressRoutine(
  LARGE_INTEGER TotalFileSize,
  LARGE_INTEGER TotalBytesTransferred,
  LARGE_INTEGER StreamSize,
  LARGE_INTEGER StreamBytesTransferred,
  DWORD dwStreamNumber,
  DWORD dwCallbackReason,
  HANDLE hSourceFile,
  HANDLE hDestinationFile,
  LPVOID lpData)
{
	PyObject *args=NULL, *ret=NULL;
	DWORD retcode;
	CEnterLeavePython _celp;
	PyObject **callback_objects=(PyObject **)lpData;
	// Py_BuildValue should catch PyHANDLEs NULL
	args=Py_BuildValue("LLLLkkNNO",
		TotalFileSize, TotalBytesTransferred,
		StreamSize, StreamBytesTransferred,
		dwStreamNumber, dwCallbackReason,
		PyWinLong_FromHANDLE(hSourceFile),
		PyWinLong_FromHANDLE(hDestinationFile),
		callback_objects[1]);
	if (args==NULL)	// Some serious error, cancel operation.
		retcode=PROGRESS_CANCEL;
	else{
		ret=PyObject_Call(callback_objects[0], args, NULL);
		if (ret==NULL)
			retcode=PROGRESS_CANCEL;
		else{
			retcode=PyLong_AsLong(ret);
			if ((retcode==(DWORD)-1) && PyErr_Occurred())
				retcode=PROGRESS_CANCEL;
			}
		}

	Py_XDECREF(args);
	Py_XDECREF(ret);
	return retcode;
}

// @pyswig |CopyFileEx|Restartable file copy with optional progress routine
// @pyseeapi CopyFileEx
// @pyseeapi CopyFileTransacted
// @comm Accepts keyword args.
// @comm On Vista and later, the Transaction arg can be passed to invoke CopyFileTransacted
static PyObject*
py_CopyFileEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obsrc, *obdst, *obcallback=Py_None, *obdata=Py_None, *ret=NULL;
	PyObject *obtrans=Py_None;
	WCHAR *src=NULL, *dst=NULL;
	BOOL bcancel=FALSE, bsuccess;
	LPPROGRESS_ROUTINE callback=NULL;
	LPVOID callback_data=NULL;
	PyObject *callback_objects[2];
	DWORD flags=0;
	HANDLE htrans;
	static char *keywords[]={"ExistingFileName","NewFileName","ProgressRoutine","Data",
		"Cancel","CopyFlags","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OOikO:CopyFileEx", keywords,
		&obsrc,		// @pyparm string|ExistingFileName||File to be copied
		&obdst,		// @pyparm string|NewFileName||Place to which it will be copied
		&obcallback,	// @pyparm <o CopyProgressRoutine>|ProgressRoutine|None|A python function that receives progress updates, can be None
		&obdata,		// @pyparm object|Data|None|An arbitrary object to be passed to the callback function
		&bcancel,		// @pyparm boolean|Cancel|False|Pass True to cancel a restartable copy that was previously interrupted
		&flags,			// @pyparm int|CopyFlags|0|Combination of COPY_FILE_* flags
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction as returned by <om win32transaction.CreateTransaction>
		return NULL;

	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans){
		CHECK_PFN(CopyFileTransacted);
		}
	else{
		CHECK_PFN(CopyFileEx);
		}

	if (obcallback!=Py_None){
		if (!PyCallable_Check(obcallback)){
			PyErr_SetString(PyExc_TypeError,"ProgressRoutine must be callable");
			return NULL;
			}
		callback=CopyFileEx_ProgressRoutine;
		callback_objects[0]=obcallback;
		callback_objects[1]=obdata;
		callback_data=callback_objects;
		}

	if (PyWinObject_AsWCHAR(obsrc, &src, FALSE) && PyWinObject_AsWCHAR(obdst, &dst, FALSE)){
		Py_BEGIN_ALLOW_THREADS
		if (htrans)
			bsuccess=(*pfnCopyFileTransacted)(src, dst, callback, callback_data, &bcancel, flags, htrans);
		else
			bsuccess=(*pfnCopyFileEx)(src, dst, callback, callback_data, &bcancel, flags);
		Py_END_ALLOW_THREADS
		if (!bsuccess){
			// progress routine may have already thrown an exception
			if (!PyErr_Occurred())
				PyWin_SetAPIError("CopyFileEx");
			}
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(src);
	PyWinObject_FreeWCHAR(dst);
	return ret;
}
PyCFunction pfnpy_CopyFileEx=(PyCFunction)py_CopyFileEx;

// @pyswig |MoveFileWithProgress|Moves a file, and reports progress to a callback function
// @comm Only available on Windows 2000 or later
// @comm Accepts keyword arguments.
// @comm On Vista and later, the Transaction arg can be passed to invoke MoveFileTransacted
static PyObject*
py_MoveFileWithProgress(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obsrc, *obdst, *obcallback=Py_None, *obdata=Py_None, *ret=NULL;
	PyObject *obtrans=Py_None;
	WCHAR *src=NULL, *dst=NULL;
	BOOL bsuccess;
	LPPROGRESS_ROUTINE callback=NULL;
	LPVOID callback_data=NULL;
	PyObject *callback_objects[2];
	DWORD flags=0;
	HANDLE htrans;
	static char *keywords[]={"ExistingFileName","NewFileName","ProgressRoutine","Data","Flags","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OOkO:MoveFileWithProgress", keywords,
		&obsrc,		// @pyparm string|ExistingFileName||File or directory to be moved
		&obdst,		// @pyparm string|NewFileName||Destination, can be None if flags contain MOVEFILE_DELAY_UNTIL_REBOOT
		&obcallback,	// @pyparm <o CopyProgressRoutine>|ProgressRoutine|None|A python function that receives progress updates, can be None
		&obdata,	// @pyparm object|Data|None|An arbitrary object to be passed to the callback function
		&flags,		// @pyparm int|Flags|0|Combination of MOVEFILE_* flags
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction (optional).  See <om win32transaction.CreateTransaction>.
		return NULL;

	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans){
		CHECK_PFN(MoveFileTransacted);
		}
	else{
		CHECK_PFN(MoveFileWithProgress);
		}

	if (obcallback!=Py_None){
		if (!PyCallable_Check(obcallback)){
			PyErr_SetString(PyExc_TypeError,"ProgressRoutine must be callable");
			return NULL;
			}
		callback=CopyFileEx_ProgressRoutine;
		callback_objects[0]=obcallback;
		callback_objects[1]=obdata;
		callback_data=callback_objects;
		}

	if (PyWinObject_AsWCHAR(obsrc, &src, FALSE) && PyWinObject_AsWCHAR(obdst, &dst, TRUE)){
		Py_BEGIN_ALLOW_THREADS
		if (htrans)
			bsuccess=(*pfnMoveFileTransacted)(src, dst, callback, callback_data, flags, htrans);
		else
			bsuccess=(*pfnMoveFileWithProgress)(src, dst, callback, callback_data, flags);
		Py_END_ALLOW_THREADS
		if (!bsuccess){
			// progress routine may have already thrown an exception
			if (!PyErr_Occurred())
				PyWin_SetAPIError("MoveFileWithProgress");
			}
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(src);
	PyWinObject_FreeWCHAR(dst);
	return ret;
}
PyCFunction pfnpy_MoveFileWithProgress=(PyCFunction)py_MoveFileWithProgress;

// @pyswig |ReplaceFile|Replaces one file with another
// @comm Only available on Windows 2000 or later
static PyObject*
py_ReplaceFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(ReplaceFile);
	PyObject *obsrc, *obdst, *obbackup=Py_None, *obExclude=Py_None, *obReserved=Py_None, *ret=NULL;
	WCHAR *src=NULL, *dst=NULL, *backup=NULL;
	LPVOID Exclude=NULL, Reserved=NULL;
	BOOL bsuccess;
	DWORD flags=0;
	if (!PyArg_ParseTuple(args, "OO|OkOO:ReplaceFile",
		&obdst,		// @pyparm string|ReplacedFileName||File to be replaced
		&obsrc,		// @pyparm string|ReplacementFileName||File that will replace it
		&obbackup,	// @pyparm string|BackupFileName|None|Place at which to create a backup of the replaced file, can be None
		&flags,		// @pyparm int|ReplaceFlags|0|Combination of REPLACEFILE_* flags
		&obExclude,	// @pyparm None|Exclude|None|Reserved, use None if passed in
		&obReserved))	// @pyparm None|Reserved|None|Reserved, use None if passed in
		return NULL;

	if (obExclude!=Py_None || obReserved!=Py_None){
		PyErr_SetString(PyExc_ValueError,"Exclude and Reserved must be None");
		return NULL;
		}
	if (PyWinObject_AsWCHAR(obsrc, &dst, FALSE)
		&&PyWinObject_AsWCHAR(obdst, &src, FALSE)
		&&PyWinObject_AsWCHAR(obbackup, &backup, TRUE)){
		Py_BEGIN_ALLOW_THREADS
		bsuccess=(*pfnReplaceFile)(dst, src, backup, flags, Exclude, Reserved);
		Py_END_ALLOW_THREADS
		if (bsuccess){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("ReplaceFile");
		}
	PyWinObject_FreeWCHAR(dst);
	PyWinObject_FreeWCHAR(src);
	PyWinObject_FreeWCHAR(backup);
	return ret;
}

// Capsule API replaced PyCObject in 3.2.
void encryptedfilecontextdestructor(PyObject *obctxt){
	if (!PyCapsule_IsValid(obctxt, NULL))
		return;	// should not happen, but maybe print a warning just in case ?
	// Check if context has already been explicitly destroyed
	// The capsule's context is set to this value in CloseEncryptedFileRaw
	if (PyCapsule_GetContext(obctxt) == INVALID_HANDLE_VALUE)
		return;
	void *ctxt = PyCapsule_GetPointer(obctxt, NULL);
	if (pfnCloseEncryptedFileRaw)
		(*pfnCloseEncryptedFileRaw)(ctxt);
}


// @pyswig PyCObject|OpenEncryptedFileRaw|Initiates a backup or restore operation on an encrypted file
// @rdesc Returns a PyCObject containing an operation context that can be passed to
// <om win32file.ReadEncryptedFileRaw> or <om win32file.WriteEncryptedFileRaw>.  Context must be
// destroyed using <om win32file.CloseEncryptedFileRaw>.
// @comm Only available on Windows 2000 or later
static PyObject*
py_OpenEncryptedFileRaw(PyObject *self, PyObject *args)
{
	CHECK_PFN(OpenEncryptedFileRaw);
	CHECK_PFN(CloseEncryptedFileRaw);
	PyObject *obfname, *ret=NULL;
	DWORD flags, err;
	WCHAR *fname=NULL;
	void *ctxt;
	if (!PyArg_ParseTuple(args, "Ok:OpenEncryptedFileRaw",
		&obfname,	// @pyparm string|FileName||Name of file on which to operate
		&flags))	// @pyparm int|Flags||CREATE_FOR_IMPORT, CREATE_FOR_DIR, OVERWRITE_HIDDEN, or 0 for export
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnOpenEncryptedFileRaw)(fname, flags, &ctxt),
	Py_END_ALLOW_THREADS
	if (err!=ERROR_SUCCESS)
		PyWin_SetAPIError("OpenEncryptedFileRaw", err);
	else{
		ret=PyCapsule_New(ctxt, NULL, encryptedfilecontextdestructor);
		if (ret==NULL)
			(*pfnCloseEncryptedFileRaw)(ctxt);
		}
	PyWinObject_FreeWCHAR(fname);
	return ret;
}

// @object ExportCallback|User-defined callback function used with <om win32file.ReadEncryptedFileRaw>.<nl>
// Function is called with 3 parameters: (Data, CallbackContext, Length)<nl>
// &nbsp&nbsp Data: Read-only buffer containing the raw data read from the file.  Must not be referenced outside of the callback function.<nl>
// &nbsp&nbsp CallbackContext: Arbitrary object passed to ReadEncryptedFileRaw.<nl>
// &nbsp&nbsp Length: Number of bytes in the Data buffer.<nl>
// On success, function should return ERROR_SUCCESS.  Otherwise, it can return a win32 error code, or simply raise an exception.
DWORD WINAPI PyExportCallback(PBYTE file_data, PVOID callback_data, ULONG length)
{
	CEnterLeavePython _celp;
	PyObject *args=NULL, *ret=NULL;
	DWORD retcode;
	PyObject **callback_objects=(PyObject **)callback_data;
	PyObject *obfile_data=PyBuffer_FromMemory(file_data, length);
	if (obfile_data==NULL)
		retcode=ERROR_OUTOFMEMORY;
	else{
		args=Py_BuildValue("OOk", obfile_data, callback_objects[1], length);
		if (args==NULL)	// exception already set, return any error code
			retcode=ERROR_OUTOFMEMORY;
		else{
			ret=PyObject_Call(callback_objects[0], args, NULL);
			if (ret==NULL)
				retcode=ERROR_OUTOFMEMORY;	// specific code shouldn't matter
			else
				retcode=PyLong_AsUnsignedLong(ret);
			}
		}
	Py_XDECREF(ret);
	Py_XDECREF(args);
	Py_XDECREF(obfile_data);
	return retcode;
}

// @pyswig |ReadEncryptedFileRaw|Reads the encrypted bytes of a file for backup and restore purposes
// @comm Only available on Windows 2000 or later
static PyObject*
py_ReadEncryptedFileRaw(PyObject *self, PyObject *args)
{
	CHECK_PFN(ReadEncryptedFileRaw);
	PyObject *obcallback, *obcallback_data, *obctxt;
	PVOID ctxt;
	PyObject *callback_objects[2];
	DWORD retcode;
	if (!PyArg_ParseTuple(args, "OOO:ReadEncryptedFileRaw",
		&obcallback,		// @pyparm <o ExportCallBack>|ExportCallback||Python function that receives chunks of data as it is read
		&obcallback_data,	// @pyparm object|CallbackContext||Arbitrary Python object to be passed to callback function
		&obctxt))			// @pyparm PyCObject|Context||Context object returned from <om win32file.OpenEncryptedFileRaw>
		return NULL;
	ctxt=PyCapsule_GetPointer(obctxt, NULL);
	if (ctxt==NULL)
		return NULL;
	if (!PyCallable_Check(obcallback)){
		PyErr_SetString(PyExc_TypeError,"ExportCallback must be callable");
		return NULL;
		}
	callback_objects[0]=obcallback;
	callback_objects[1]=obcallback_data;

	Py_BEGIN_ALLOW_THREADS
	retcode=(*pfnReadEncryptedFileRaw)(PyExportCallback, callback_objects, ctxt);
	Py_END_ALLOW_THREADS
	if (retcode==ERROR_SUCCESS){
		Py_INCREF(Py_None);
		return Py_None;
		}
	// Don't overwrite any error that callback may have thrown
	if (!PyErr_Occurred())
		PyWin_SetAPIError("ReadEncryptedFileRaw",retcode);
	return NULL;
}

// @object ImportCallback|User-defined callback function used with <om win32file.WriteEncryptedFileRaw><nl>
// Function is called with 3 parameters: (Data, CallbackContext, Length)<nl>
// &nbsp&nbsp Data: Writeable buffer to be filled with raw encrypted data.  Buffer memory is only valid within the callback function.<nl>
// &nbsp&nbsp CallbackContext: The arbitrary object passed to WriteEncryptedFileRaw.<nl>
// &nbsp&nbsp Length: Size of the data buffer.<nl>
// Your implementation of this function should return a tuple of 2 ints containing
// an error code (ERROR_SUCCESS on success), and the length of data written to the buffer.<nl>
// Function exits when 0 is returned for the data length.
DWORD WINAPI PyImportCallback(PBYTE file_data, PVOID callback_data, PULONG plength)
{
	CEnterLeavePython _celp;
	PyObject *args=NULL, *ret=NULL;
	DWORD retcode;
	PyObject **callback_objects=(PyObject **)callback_data;
	PyObject *obfile_data=PyBuffer_FromReadWriteMemory(file_data, *plength);
	if (obfile_data==NULL)
		retcode=ERROR_OUTOFMEMORY;
	else{
		args=Py_BuildValue("OOk", obfile_data, callback_objects[1], *plength);
		if (args==NULL)
			retcode=ERROR_OUTOFMEMORY;
		else{
			ret=PyObject_Call(callback_objects[0], args, NULL);
			if (ret==NULL)
				retcode=ERROR_OUTOFMEMORY;
			else if ((!PyTuple_Check(ret)) || (PyTuple_GET_SIZE(ret)!=2)){
				PyErr_SetString(PyExc_TypeError,"ImportCallback must return a tuple of 2 ints");
				retcode=ERROR_OUTOFMEMORY;	// doesn't matter which error code if exception is set
				}
			else if (!PyArg_ParseTuple(ret,"kk", &retcode, plength))
					retcode=ERROR_OUTOFMEMORY;
			}
		}
	Py_XDECREF(ret);
	Py_XDECREF(args);
	Py_XDECREF(obfile_data);
	return retcode;
}

// @pyswig |WriteEncryptedFileRaw|Writes raw bytes to an encrypted file
// @comm Only available on Windows 2000 or later
static PyObject*
py_WriteEncryptedFileRaw(PyObject *self, PyObject *args)
{
	CHECK_PFN(WriteEncryptedFileRaw);
	PyObject *obcallback, *obcallback_data, *obctxt;
	PVOID ctxt;
	PyObject *callback_objects[2];
	DWORD retcode;
	if (!PyArg_ParseTuple(args, "OOO:WriteEncryptedFileRaw",
		&obcallback,		// @pyparm <o ImportCallBack>|ImportCallback||Python function that supplies data to be written
		&obcallback_data,	// @pyparm object|CallbackContext||Arbitrary Python object to be passed to callback function
		&obctxt))			// @pyparm PyCObject|Context||Context object returned from <om win32file.OpenEncryptedFileRaw>
		return NULL;
	ctxt=PyCapsule_GetPointer(obctxt, NULL);
	if (ctxt==NULL)
		return NULL;
	if (!PyCallable_Check(obcallback)){
		PyErr_SetString(PyExc_TypeError,"ExportCallback must be callable");
		return NULL;
		}
	callback_objects[0]=obcallback;
	callback_objects[1]=obcallback_data;

	Py_BEGIN_ALLOW_THREADS
	retcode=(*pfnWriteEncryptedFileRaw)(PyImportCallback, callback_objects, ctxt);
	Py_END_ALLOW_THREADS
	if (retcode==ERROR_SUCCESS){
		Py_INCREF(Py_None);
		return Py_None;
		}
	// Don't overwrite any error that callback may have thrown
	if (!PyErr_Occurred())
		PyWin_SetAPIError("WriteEncryptedFileRaw",retcode);
	return NULL;
}

// @pyswig |CloseEncryptedFileRaw|Frees a context created by <om win32file.OpenEncryptedFileRaw>
// @comm Only available on Windows 2000 or later
static PyObject*
py_CloseEncryptedFileRaw(PyObject *self, PyObject *args)
{
	CHECK_PFN(CloseEncryptedFileRaw);
	PyObject *obctxt;
	if (!PyArg_ParseTuple(args, "O:CloseEncryptedFileRaw",
		&obctxt))	// @pyparm PyCObject|Context||Context object returned from <om win32file.OpenEncryptedFileRaw>
		return NULL;
	// We must nuke our ctxt in the CObject afer closing, else when the
	// object destructs and we attempt to close it a second time, Vista x64
	// crashes.
	// So must bypass the CObject API for this.
	if (!PyCapsule_IsValid(obctxt, NULL))
		return PyErr_Format(PyExc_TypeError, "param must be handle to an encrypted file (got type %s)", obctxt->ob_type->tp_name);
	if (PyCapsule_GetDestructor(obctxt) != encryptedfilecontextdestructor)
		return PyErr_Format(PyExc_TypeError, "param must be handle to an encrypted file (got a CObject with invalid destructor)");
	/* PyCapsule will *not* allow you to set the pointer to NULL, so use its extra context pointer
		to signal that we have already destroyed our context.
		??? Maybe just set the pointer itself to INVALID_HANDLE_VALUE ???
	*/
	if (PyCapsule_GetContext(obctxt) == INVALID_HANDLE_VALUE)
		return PyErr_Format(PyExc_ValueError, "This handle has already been closed");
	void *ctxt = PyCapsule_GetPointer(obctxt, NULL);
	(*pfnCloseEncryptedFileRaw)(ctxt);
	PyCapsule_SetContext(obctxt, INVALID_HANDLE_VALUE);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pyswig <o PyHANDLE>|CreateFileW|Unicode version of CreateFile - see <om win32file.CreateFile> for more information.
// @pyseeapi CreateFile
// @pyseeapi CreateFileTransacted
// @comm If Transaction is specified, CreateFileTransacted will be called (requires Vista or later)
// @comm Accepts keyword arguments.
static PyObject *py_CreateFileW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *filename=NULL;
	PyObject *obfilename, *obsa, *obhtemplate=Py_None,
		*obhtransaction=Py_None, *obminiversion=Py_None, *obextendedparameter=Py_None;
	DWORD desiredaccess, sharemode, creationdisposition, flags;
	USHORT miniversion=0;
	PUSHORT pminiversion=NULL;
	PSECURITY_ATTRIBUTES psa;
	VOID *extendedparameter=NULL;
	HANDLE htemplate, htransaction, hret;
	static char *keywords[]={"FileName","DesiredAccess","ShareMode","SecurityAttributes","CreationDisposition",
		"FlagsAndAttributes","TemplateFile","Transaction","MiniVersion","ExtendedParameter", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OkkOkk|OOOO:CreateFileW", keywords,
		&obfilename,			// @pyparm string|FileName||Name of file
		&desiredaccess,			// @pyparm int|DesiredAccess||Combination of access mode flags.  See MSDN docs.
		&sharemode,				// @pyparm int|ShareMode||Combination of FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_DELETE
		&obsa,					// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes||Specifies security descriptor and handle inheritance, can be None
		&creationdisposition,	// @pyparm int|CreationDisposition||One of CREATE_ALWAYS,CREATE_NEW,OPEN_ALWAYS,OPEN_EXISTING or TRUNCATE_EXISTING
		&flags,					// @pyparm int|FlagsAndAttributes||Combination of FILE_ATTRIBUTE_* and FILE_FLAG_* flags
		&obhtemplate,			// @pyparm <o PyHANDLE>|TemplateFile|None|Handle to file to be used as template, can be None
		&obhtransaction,		// @pyparm <o PyHANDLE>|Transaction|None|Handle to the transaction as returned by <om win32transaction.CreateTransaction>
		&obminiversion,			// @pyparm int|MiniVersion|None|Transacted version of file to open, can be None
		&obextendedparameter))	// @pyparm None|ExtendedParameter|None|Reserved, use only None
		return NULL;

	if (!PyWinObject_AsHANDLE(obhtransaction, &htransaction))
		return NULL;
	if (htransaction)
		CHECK_PFN(CreateFileTransacted);

	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhtemplate, &htemplate))
		return NULL;
	if (obextendedparameter!=Py_None){
		PyErr_SetString(PyExc_TypeError,"ExtendedParameter must be None");
		return NULL;
		}
	if (obminiversion!=Py_None){
		if (!htransaction){
			PyErr_SetString(PyExc_ValueError, "MiniVersion can only be used with a transacted operation");
			return NULL;
			}
		long longversion=PyLong_AsLong(obminiversion);
		if (longversion==-1 && PyErr_Occurred())
			return NULL;
		if ((longversion > USHRT_MAX) || (longversion < 0))
			return PyErr_Format(PyExc_ValueError, "MiniVersion must be in the range 0 - %d", USHRT_MAX);
		miniversion=(USHORT)longversion;
		pminiversion=&miniversion;
		}
	if (!PyWinObject_AsWCHAR(obfilename, &filename, FALSE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	if (htransaction)
		hret=(*pfnCreateFileTransacted)(filename, desiredaccess, sharemode, psa, creationdisposition,
			flags, htemplate, htransaction, pminiversion, extendedparameter);
	else
		hret=CreateFileW(filename, desiredaccess, sharemode, psa, creationdisposition,
			flags, htemplate);
	Py_END_ALLOW_THREADS

	PyWinObject_FreeWCHAR(filename);
	if (hret==INVALID_HANDLE_VALUE)
		return PyWin_SetAPIError("CreateFileW");
	return PyWinObject_FromHANDLE(hret);
}
PyCFunction pfnpy_CreateFileW=(PyCFunction)py_CreateFileW;

// @pyswig |DeleteFileW|Deletes a file
// @pyseeapi DeleteFile
// @pyseeapi DeleteFileTransacted
// @comm If a transaction handle is passed in, DeleteFileTransacted will be called (requires Windows Vista).
// @comm Accepts keyword arguments.
static PyObject *py_DeleteFileW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *filename=NULL;
	PyObject *obfilename, *obhtransaction=Py_None;
	HANDLE htransaction;
	static char *keywords[]={"FileName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:DeleteFileW", keywords,
		&obfilename,		// @pyparm string|FileName||Name of file to be deleted
		&obhtransaction))	// @pyparm <o PyHANDLE>|Transaction|None|Transaction handle as returned by <om win32transaction.CreateTransaction>
		return NULL;
	if (!PyWinObject_AsHANDLE(obhtransaction, &htransaction))
		return NULL;
	if (htransaction)
		CHECK_PFN(DeleteFileTransacted);
	if (!PyWinObject_AsWCHAR(obfilename, &filename, FALSE))
		return NULL;

	BOOL ret;
	Py_BEGIN_ALLOW_THREADS
	if (htransaction)
		ret=(*pfnDeleteFileTransacted)(filename, htransaction);
	else
		ret=DeleteFileW(filename);
	Py_END_ALLOW_THREADS

	PyWinObject_FreeWCHAR(filename);
	if (!ret)
		return PyWin_SetAPIError("DeleteFileW");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnpy_DeleteFileW=(PyCFunction)py_DeleteFileW;


static PyObject *PyObject_FromFILEX_INFO(GET_FILEEX_INFO_LEVELS level, void *p)
{
	switch (level) {
		case GetFileExInfoStandard: {
			WIN32_FILE_ATTRIBUTE_DATA *pa = (WIN32_FILE_ATTRIBUTE_DATA *)p;
			ULARGE_INTEGER fsize;
			fsize.LowPart=pa->nFileSizeLow;
			fsize.HighPart=pa->nFileSizeHigh;
			return Py_BuildValue("iNNNN",
			             pa->dwFileAttributes,
						 PyWinObject_FromFILETIME(pa->ftCreationTime),
						 PyWinObject_FromFILETIME(pa->ftLastAccessTime),
						 PyWinObject_FromFILETIME(pa->ftLastWriteTime),
			             PyWinObject_FromULARGE_INTEGER(fsize));
			break;
		}

		default:
			PyErr_Format(PyExc_RuntimeError, "invalid level for FILEEX_INFO");
			return NULL;
	}
	assert(0); // "not reached";
	return NULL;
}

// @pyswig tuple|GetFileAttributesEx|Retrieves attributes for a specified file or directory.
// @pyseeapi GetFileAttributesEx
// @pyseeapi GetFileAttributesTransacted
// @pyparm string/bytes|FileName||File or directory for which to retrieve information
//  In the usual case, the name is limited to MAX_PATH characters. To extend this
// limit to nearly 32,000 wide characters, call this and prepend r"\\?\" to the path.
// @pyparm int|InfoLevelId|GetFileExInfoStandard|An integer that gives the set of attribute information to obtain.
//  See the Win32 SDK documentation for more information.
// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction (optional).  See <om win32transaction.CreateTransaction>.
//  If this parameter is specified, GetFileAttributesTransacted will be called (requires Vista or later).
// @rdesc The result is a tuple of:
//	@tupleitem 0|int|attributes|File Attributes.  A combination of the win32com.FILE_ATTRIBUTE_* flags.
//	@tupleitem 1|<o PyDateTime>|creationTime|Specifies when the file or directory was created.
//	@tupleitem 2|<o PyDateTime>|lastAccessTime|For a file, specifies when the file was last read from
//		or written to. For a directory, the structure specifies when the directory was created. For
//		both files and directories, the specified date will be correct, but the time of day will
//		always be set to midnight.
//	@tupleitem 3|<o PyDateTime>|lastWriteTime|For a file, the structure specifies when the file was last
//		written to. For a directory, the structure specifies when the directory was created.
//	@tupleitem 4|int/long|fileSize|The size of the file. This member has no meaning for directories.
// @comm Not all file systems can record creation and last access time and not all file systems record
//	them in the same manner. For example, on Windows NT FAT, create time has a resolution of
//	10 milliseconds, write time has a resolution of 2 seconds, and access time has a resolution
//	of 1 day (really, the access date). On NTFS, access time has a resolution of 1 hour.
//	Furthermore, FAT records times on disk in local time, while NTFS records times on disk in UTC,
//	so it is not affected by changes in time zone or daylight saving time.
// @comm Accepts keyword arguments.
// @comm If bytes are passed for the filename, the ANSI Windows functions are called.
static PyObject *py_GetFileAttributesEx(PyObject *self, PyObject *args, PyObject *kwargs, BOOL bUnicode)
{
	BOOL ok;
	char *cname=NULL;
	WCHAR *wname=NULL;
	PyObject *obfname, *obtrans=Py_None, *ret=NULL;
	GET_FILEEX_INFO_LEVELS lvl=GetFileExInfoStandard;
	PVOID buf=NULL;
	size_t bufsize;
	HANDLE htrans;
	BOOL bsuccess=FALSE;
	static char *keywords[]={"FileName","InfoLevelId","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|kO:GetFileAttributesEx", keywords,
		&obfname,
		&lvl,
		&obtrans))
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans){
		if (bUnicode){
			CHECK_PFN(GetFileAttributesTransactedW);
			}
		else{
			CHECK_PFN(GetFileAttributesTransactedA);
			}
		}

	if (bUnicode)
		ok = PyWinObject_AsWCHAR(obfname, &wname, FALSE);
	else
		ok = PyWinObject_AsChars(obfname, &cname, FALSE);
	if (!ok)
		goto done;

	switch (lvl){
		// @flagh InfoLevelId|Information returned
		// @flag GetFileExInfoStandard|Tuple representing a WIN32_FILE_ATTRIBUTE_DATA struc
		case GetFileExInfoStandard:
			bufsize = sizeof(WIN32_FILE_ATTRIBUTE_DATA);
			break;
		default:
			PyErr_Format(PyExc_ValueError, "Level '%d' is not supported", lvl);
			goto done;
		}
	buf = malloc(bufsize);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		goto done;
		}

	Py_BEGIN_ALLOW_THREADS
	// MSDN docs say this returns a DWORD containing the attributes, but it actually acts as a boolean
	if (htrans){
		if (bUnicode)
			ok=(*pfnGetFileAttributesTransactedW)(wname, lvl, buf, htrans);
		else
			ok=(*pfnGetFileAttributesTransactedA)(cname, lvl, buf, htrans);
		}
	else{
		if (bUnicode)
			ok=GetFileAttributesExW(wname, lvl, buf);
		else
			ok=GetFileAttributesExA(cname, lvl, buf);
		}
	Py_END_ALLOW_THREADS

	if (!ok)
		PyWin_SetAPIError("GetFileAttributesEx");
	else
		ret=PyObject_FromFILEX_INFO(lvl, buf);

	done:
	if (buf)
		free(buf);
	PyWinObject_FreeWCHAR(wname);
	PyWinObject_FreeChars(cname);
	return ret;
}

static PyObject *py_GetFileAttributesExW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	return py_GetFileAttributesEx(self, args, kwargs, TRUE);
}

static PyObject *py_GetFileAttributesExA(PyObject *self, PyObject *args, PyObject *kwargs)
{
	return py_GetFileAttributesEx(self, args, kwargs, FALSE);
}
PyCFunction pfnpy_GetFileAttributesExW=(PyCFunction)py_GetFileAttributesExW;
PyCFunction pfnpy_GetFileAttributesEx=(PyCFunction)py_GetFileAttributesExW;

// @pyswig |SetFileAttributesW|Sets a file's attributes
// @pyseeapi SetFileAttributes
// @pyseeapi SetFileAttributesTransacted
// @comm If Transaction is not None, SetFileAttributesTransacted will be called (requires Vista or later)
// @comm Accepts keyword arguments.
static PyObject *py_SetFileAttributesW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *fname=NULL;
	PyObject *obfname, *obtrans=Py_None;
	DWORD attrs;
	HANDLE htrans;
	static char *keywords[]={"FileName","FileAttributes","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|O:SetFileAttributesW", keywords,
		&obfname,	// @pyparm string|FileName||File or directory whose attributes are to be changed
		&attrs,		// @pyparm int|FileAttributes||Combination of FILE_ATTRIBUTE_* flags
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to the transaction.  See <om win32transaction.CreateTransaction>.
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans)
		CHECK_PFN(SetFileAttributesTransacted);
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	BOOL ret;
	if (htrans)
		ret=(*pfnSetFileAttributesTransacted)(fname, attrs, htrans);
	else
		ret=SetFileAttributesW(fname, attrs);

	PyWinObject_FreeWCHAR(fname);
	if (!ret)
		return PyWin_SetAPIError("SetFileAttributesW");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnpy_SetFileAttributesW=(PyCFunction)py_SetFileAttributesW;

// @pyswig |CreateDirectoryExW|Creates a directory
// @pyseeapi CreateDirectoryEx
// @pyseeapi CreateDirectoryTransacted
// @comm If a transaction handle is passed, CreateDirectoryTransacted will be called (requires Vista or later).
// @comm Accepts keyword arguments.
static PyObject *py_CreateDirectoryExW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *dirname=NULL, *templatedir=NULL;
	PyObject *obdirname, *obtrans=Py_None, *obtemplatedir=Py_None, *obsa=Py_None, *ret=NULL;
	HANDLE htrans;
	SECURITY_ATTRIBUTES *psa;
	static char *keywords[]={"TemplateDirectory","NewDirectory","SecurityAttributes","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OO:CreateDirectoryExW", keywords,
		&obtemplatedir,	// @pyparm string|TemplateDirectory||Directory to use as a template, can be None
		&obdirname,		// @pyparm string|NewDirectory||Name of directory to be created
		&obsa,			// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes|None|Security for new directory (optional)
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction (optional).  See <om win32transaction.CreateTransaction>.
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans)
		CHECK_PFN(CreateDirectoryTransacted);
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE))
		return NULL;

	BOOL bsuccess;
	if (PyWinObject_AsWCHAR(obdirname, &dirname, FALSE)
		&& PyWinObject_AsWCHAR(obtemplatedir, &templatedir, TRUE)){
		if (htrans)
			bsuccess=(*pfnCreateDirectoryTransacted)(templatedir, dirname, psa, htrans);
		else
			bsuccess=CreateDirectoryExW(templatedir, dirname, psa);
		if (!bsuccess)
			PyWin_SetAPIError("CreateDirectoryExW");
		else{
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		}
	PyWinObject_FreeWCHAR(dirname);
	PyWinObject_FreeWCHAR(templatedir);
	return ret;
}
PyCFunction pfnpy_CreateDirectoryExW=(PyCFunction)py_CreateDirectoryExW;

// @pyswig |RemoveDirectory|Removes an existing directory
// @pyseeapi RemoveDirectory
// @pyseeapi RemoveDirectoryTransacted
// @comm If a transaction handle is passed in, RemoveDirectoryTransacted will be called (requires Vista or later)
// @comm Accepts keyword arguments.
static PyObject *py_RemoveDirectory(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *dirname=NULL;
	PyObject *obdirname, *obtrans=Py_None, *ret=NULL;
	HANDLE htrans;
	static char *keywords[]={"PathName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:RemoveDirectory", keywords,
		&obdirname,		// @pyparm string|PathName||Name of directory to be removed
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction (optional). See <om win32transaction.CreateTransaction>.
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans)
		CHECK_PFN(RemoveDirectoryTransacted);
	if (!PyWinObject_AsWCHAR(obdirname, &dirname, FALSE))
		return NULL;

	BOOL bsuccess;
	if (htrans)
		bsuccess=(*pfnRemoveDirectoryTransacted)(dirname, htrans);
	else
		bsuccess=RemoveDirectoryW(dirname);
	if (!bsuccess)
		PyWin_SetAPIError("RemoveDirectory");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreeWCHAR(dirname);
	return ret;
}
PyCFunction pfnpy_RemoveDirectory=(PyCFunction)py_RemoveDirectory;

// @pyswig list|FindFilesW|Retrieves a list of matching filenames, using the Windows Unicode API.  An interface to the API FindFirstFileW/FindNextFileW/Find close functions.
// @comm Accepts keyword args.
// @comm FindFirstFileTransacted will be called if a transaction handle is passed in.
static PyObject *py_FindFilesW(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *fileSpec;
	HANDLE htrans;
	PyObject *obfileSpec, *obtrans=Py_None;
	static char *keywords[]={"FileName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords (args, kwargs, "O|O:FindFilesW", keywords,
		&obfileSpec,	// @pyparm string|FileName||A string that specifies a valid directory or path and filename, which can contain wildcard characters (* and ?).
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Transaction handle as returned by <om win32transaction.CreateTransaction>.  Can be None.
						//	If this parameter is not None, FindFirstFileTransacted will be called to perform a transacted search
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans!=NULL)
		CHECK_PFN(FindFirstFileTransacted);
	if (!PyWinObject_AsWCHAR(obfileSpec,&fileSpec,FALSE))
		return NULL;
	WIN32_FIND_DATAW findData;
	// @pyseeapi FindFirstFile
	// @pyseeapi FindFirstFileTransacted
	HANDLE hFind;

	memset(&findData, 0, sizeof(findData));
	if (htrans!=NULL)
		hFind=(*pfnFindFirstFileTransacted)(fileSpec, FindExInfoStandard, &findData,
			FindExSearchNameMatch, NULL, 0, htrans);
	else
		hFind =  ::FindFirstFileW(fileSpec, &findData);
	PyWinObject_FreeWCHAR(fileSpec);
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
	// @rdesc The return value is a list of <o WIN32_FIND_DATA> tuples.
	BOOL ok = TRUE;
	while (ok) {
		PyObject *newItem = PyObject_FromWIN32_FIND_DATAW(&findData);
		if (!newItem) {
			::FindClose(hFind);
			Py_DECREF(retList);
			return NULL;
		}
		PyList_Append(retList, newItem);
		Py_DECREF(newItem);
		// @pyseeapi FindNextFile
		memset(&findData, 0, sizeof(findData));
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
PyCFunction pfnpy_FindFilesW=(PyCFunction)py_FindFilesW;

// @pyswig iterator|FindFilesIterator|Returns an interator based on
// FindFirstFile/FindNextFile. Similar to <om win32file.FindFiles>, but
// avoids the creation of the list for huge directories.
// @comm Accepts keyword args.
// @comm FindFirstFileTransacted will be called if a transaction handle is passed in.
static PyObject *py_FindFilesIterator(PyObject *self, PyObject *args, PyObject *kwargs)
{
	WCHAR *fileSpec;
	HANDLE htrans;
	PyObject *obfileSpec, *obtrans=Py_None;
	static char *keywords[]={"FileName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords (args, kwargs, "O|O:FindFilesIterator", keywords,
		&obfileSpec,	// @pyparm string|FileName||A string that specifies a valid directory or path and filename, which can contain wildcard characters (* and ?).
		&obtrans))		// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction, can be None.
						//	If this parameter is not None, FindFirstFileTransacted will be called to perform a transacted search
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans!=NULL)
		CHECK_PFN(FindFirstFileTransacted);
	if (!PyWinObject_AsWCHAR(obfileSpec,&fileSpec,FALSE))
		return NULL;

	FindFileIterator *it = PyObject_New(FindFileIterator, &FindFileIterator_Type);
	if (it == NULL) {
		PyWinObject_FreeWCHAR(fileSpec);
		return NULL;
	}
	it->seen_first = FALSE;
	it->empty = FALSE;
	it->hFind = INVALID_HANDLE_VALUE;
	memset(&it->buffer, 0, sizeof(it->buffer));

	Py_BEGIN_ALLOW_THREADS
	if (htrans!=NULL)
		it->hFind=(*pfnFindFirstFileTransacted)(fileSpec, FindExInfoStandard, &it->buffer,
			FindExSearchNameMatch, NULL, 0, htrans);
	else
		it->hFind =  ::FindFirstFileW(fileSpec, &it->buffer);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeWCHAR(fileSpec);

	if (it->hFind==INVALID_HANDLE_VALUE) {
		if (::GetLastError()!=ERROR_FILE_NOT_FOUND) {	// this is OK
			Py_DECREF(it);
			return PyWin_SetAPIError("FindFirstFileW");
		}
		it->empty = TRUE;
	}
	return (PyObject *)it;
	// @rdesc The result is a Python iterator, with each next() method
	// returning a <o WIN32_FIND_DATA> tuple.
}
PyCFunction pfnpy_FindFilesIterator=(PyCFunction)py_FindFilesIterator;

// @pyswig [(long, string),...]|FindStreams|List the data streams for a file
// @rdesc Returns a list of tuples containing each stream's size and name
// @comm This uses the API functions FindFirstStreamW, FindNextStreamW and FindClose
// @comm If the Transaction arg is not None, FindFirstStreamTransacted will be called in place of FindFirstStreamW
static PyObject *py_FindStreams(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(FindFirstStream);
	CHECK_PFN(FindNextStream);

	PyObject *obfname, *obtrans=Py_None, *ret=NULL, *ret_item;
	WCHAR *fname=NULL;
	HANDLE hfind, htrans;
	STREAM_INFO_LEVELS lvl =  FindStreamInfoStandard;  // only level that currently exists
	WIN32_FIND_STREAM_DATA fsd;
	DWORD err=0, flags=0, ret_cnt=0;   // flags are reserved, don't even accept as input
	static char *keywords[]={"FileName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:FindStreams", keywords,
		&obfname,	// @pyparm string|FileName||Name of file (or directory) to operate on
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction, can be None
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans!=NULL)
		CHECK_PFN(FindFirstStreamTransacted);
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	if (htrans!=NULL)
		hfind=(*pfnFindFirstStreamTransacted)(fname, lvl, &fsd, flags, htrans);
	else
		hfind=(*pfnFindFirstStream)(fname, lvl, &fsd, flags);
	PyWinObject_FreeWCHAR(fname);
	if (hfind==INVALID_HANDLE_VALUE)
		return PyWin_SetAPIError("FindFirstStreamW");
	ret=PyList_New(0);
	if (ret!=NULL){
		while (1){
			ret_item=Py_BuildValue("Lu", fsd.StreamSize, fsd.cStreamName);
			if ((ret_item==NULL) || (PyList_Append(ret, ret_item)==-1)){
				Py_XDECREF(ret_item);
				Py_DECREF(ret);
				ret=NULL;
				break;
				}
			Py_DECREF(ret_item);
			if (!(*pfnFindNextStream)(hfind, &fsd)){
				err=GetLastError();
				if (err!=ERROR_HANDLE_EOF){
					Py_DECREF(ret);
					ret=NULL;
					PyWin_SetAPIError("FindNextStream",err);
					}
				break;
				}
			}
		}
	FindClose(hfind);
	return ret;
}
PyCFunction pfnpy_FindStreams=(PyCFunction)py_FindStreams;

// @pyswig [string,...]|FindFileNames|Enumerates hard links that point to specified file
// @comm This uses the API functions FindFirstFileNameW, FindNextFileNameW and FindClose
// @comm Available on Vista and later
// @comm If Transaction is specified, a transacted search is performed using FindFirstFileNameTransacted
static PyObject *py_FindFileNames(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(FindFirstFileName);
	CHECK_PFN(FindNextFileName);

	PyObject *obfname, *obtrans=Py_None, *ret=NULL, *ret_item;
	WCHAR *fname=NULL, *linkname=NULL;
	HANDLE hfind=INVALID_HANDLE_VALUE, htrans=NULL;
	DWORD err=0, flags=0;   // flags are reserved, don't even accept as input
	DWORD alloc_size=MAX_PATH, ret_size=0;
	BOOL bfindfirst=TRUE, bsuccess=TRUE;
#ifdef Py_DEBUG
	alloc_size=3;	// test reallocation logic
#endif
	static char *keywords[]={"FileName","Transaction", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:FindFileNames", keywords,
		&obfname,	// @pyparm string|FileName||Name of file for which to find links
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction, can be None
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans!=NULL)
		CHECK_PFN(FindFirstFileNameTransacted);
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	while (TRUE){
		ret_size=alloc_size;
		if (linkname==NULL){
			linkname=(WCHAR *)malloc(alloc_size*sizeof(WCHAR));
			if (linkname==NULL){
				bsuccess=FALSE;
				PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", alloc_size*sizeof(WCHAR));
				break;
				}
			}
		if (bfindfirst){
			if (htrans!=NULL)
				hfind=(*pfnFindFirstFileNameTransacted)(fname, flags, &ret_size, linkname, htrans);
			else
				hfind=(*pfnFindFirstFileName)(fname, flags, &ret_size, linkname);
			bsuccess=(hfind!=INVALID_HANDLE_VALUE);
			if (bsuccess){
				bfindfirst=FALSE;
				ret=PyList_New(0);
				if (ret==NULL){
					bsuccess=FALSE;
					break;
					}
				}
			}
		else
			bsuccess=(*pfnFindNextFileName)(hfind, &ret_size, linkname);
		if (bsuccess){
			// There seems to be some confusion around ret_size - the MS docs
			// don't say whether this includes the trailing \0 or not. #1511
			// reports there's a trailing \0 on filenames, but that was opened
			// many many years after this code was added - so has it changed?
			// Regardless, we just ignore the size when creating the result
			// string so it stops at the first \0.
			ret_item=PyWinObject_FromWCHAR(linkname);
			if ((ret_item==NULL) || (PyList_Append(ret, ret_item)==-1)){
				Py_XDECREF(ret_item);
				bsuccess=FALSE;
				break;
				}
			Py_DECREF(ret_item);
			}
		else{
			err=GetLastError();
			if (err==ERROR_MORE_DATA){
				/* FindNextFileName leaks memory when it fails due to insufficient buffer !
				if (!bfindfirst)
					for (int x=0; x<10000; x++){
						alloc_size=3;
						(*pfnFindNextFileName)(hfind, &alloc_size, linkname);
					}
				*/
				free(linkname);
				linkname=NULL;
				alloc_size=ret_size+1;
				}
			else if (err==ERROR_HANDLE_EOF){
				bsuccess=TRUE;
				break;
				}
			else{
				PyWin_SetAPIError("FindFileNames", err);
				break;
				}
			}
		}
	if (!bsuccess){
		Py_XDECREF(ret);
		ret=NULL;
		}
	if (hfind!=INVALID_HANDLE_VALUE)
		FindClose(hfind);
	PyWinObject_FreeWCHAR(fname);
	if (linkname)
		free(linkname);
	return ret;
}
PyCFunction pfnpy_FindFileNames=(PyCFunction)py_FindFileNames;

// @pyswig string|GetFinalPathNameByHandle|Returns the file name for an open file handle
// @pyseeapi GetFinalPathNameByHandle
// @comm Accepts keyword arguments.
static PyObject *py_GetFinalPathNameByHandle(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(GetFinalPathNameByHandle);
	WCHAR *path=NULL;
	DWORD path_len=0, reqd_len, flags;
	HANDLE hfile;
	PyObject *obhfile, *ret=NULL;
	static char *keywords[]={"File","Flags", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:GetFinalPathNameByHandle", keywords,
		&obhfile,	// @pyparm <o PyHANDLE>|File||An open file handle
		&flags))	// @pyparm int|Flags||Specifies type of path to return. (win32con.FILE_NAME_NORMALIZED,FILE_NAME_OPENED,VOLUME_NAME_DOS,VOLUME_NAME_GUID,VOLUME_NAME_NONE,VOLUME_NAME_NT)
		return NULL;
	if (!PyWinObject_AsHANDLE(obhfile, &hfile))
		return NULL;

	reqd_len=(*pfnGetFinalPathNameByHandle)(hfile, path, path_len, flags);
	if (reqd_len==0)
		return PyWin_SetAPIError("GetFinalPathNameByHandle");
	path_len=reqd_len+1;  // returned valued doesn't include NULL terminator
	path=(WCHAR *)malloc(path_len*sizeof(WCHAR));
	if (path==NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", path_len*sizeof(WCHAR));
	reqd_len=(*pfnGetFinalPathNameByHandle)(hfile, path, path_len, flags);
	if (reqd_len==0)
		PyWin_SetAPIError("GetFinalPathNameByHandle");
	else if (reqd_len > path_len)	// should not happen
		PyErr_Format(PyExc_RuntimeError, "Unexpected increase in reqd_len %d - %d", path_len, reqd_len);
	else
		ret=PyWinObject_FromWCHAR(path,reqd_len);
	free(path);
	return ret;
}
PyCFunction pfnpy_GetFinalPathNameByHandle=(PyCFunction)py_GetFinalPathNameByHandle;

// @pyswig [string,...]|SfcGetNextProtectedFile|Returns list of protected operating system files
// @pyseeapi SfcGetNextProtectedFile
static PyObject *py_SfcGetNextProtectedFile(PyObject *self, PyObject *args)
{
	CHECK_PFN(SfcGetNextProtectedFile);
	PROTECTED_FILE_DATA pfd;
	DWORD err=0;
	HANDLE rpchandle=NULL; // reserved
	PyObject *ret, *ret_item;

	if (!PyArg_ParseTuple(args, ":SfcGetNextProtectedFile"))
		return NULL;
	ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	pfd.FileNumber=0;

	while ((*pfnSfcGetNextProtectedFile)(rpchandle, &pfd)){
		ret_item=PyWinObject_FromWCHAR(pfd.FileName);
		if (ret_item==NULL || PyList_Append(ret, ret_item)==-1){
			Py_XDECREF(ret_item);
			Py_DECREF(ret);
			return NULL;
			}
		Py_DECREF(ret_item);
		}
	err=GetLastError();
	if (err==ERROR_NO_MORE_FILES)
		return ret;
	Py_DECREF(ret);
	return PyWin_SetAPIError("SfcGetNextProtectedFile",err);
}

// @pyswig boolean|SfcIsFileProtected|Checks if a file is protected
static PyObject *py_SfcIsFileProtected(PyObject *self, PyObject *args)
{
	CHECK_PFN(SfcIsFileProtected);
	PyObject *obfname;
	WCHAR *fname;
	HANDLE rpchandle=NULL; // reserved
	BOOL ret;

	if (!PyArg_ParseTuple(args, "O:SfcIsFileProtected",
		&obfname))	// @pyparm string|ProtFileName||Name of file to be checked
		return NULL;
	if (!PyWinObject_AsWCHAR(obfname, &fname, FALSE))
		return NULL;

	ret=(*pfnSfcIsFileProtected)(rpchandle, fname);
	PyWinObject_FreeWCHAR(fname);
	if (!ret){
		DWORD err=GetLastError();
		if (err!=ERROR_FILE_NOT_FOUND)
			return PyWin_SetAPIError("SfcIsFileProtected",err);
		}
	return PyBool_FromLong(ret);
}

// @pyswig string|GetLongPathName|Retrieves the long path for a short path (8.3 filename)
// @comm Accepts keyword args
static PyObject *py_GetLongPathName(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *ret=NULL;
	DWORD pathlen=MAX_PATH+1, retlen;
	WCHAR *short_path=NULL, *long_path=NULL, *long_path_save=NULL;
	PyObject *obfname, *obtrans=Py_None;
	HANDLE htrans;
	static char *keywords[]={"ShortPath","Transaction", NULL};
#ifdef Py_DEBUG
	pathlen=3;	// test reallocation logic
#endif

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:GetLongPathName", keywords,
		&obfname,	// @pyparm string|ShortPath||8.3 path to be expanded
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction.  If specified, GetLongPathNameTransacted will be called.
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;
	if (htrans){
		CHECK_PFN(GetLongPathNameTransacted);
		}
	else{
		CHECK_PFN(GetLongPathName);
		}
	if (!PyWinObject_AsWCHAR(obfname, &short_path, FALSE))
		return NULL;

	while (1){
		if (long_path){
			long_path_save=long_path;
			long_path=(WCHAR *)realloc(long_path, pathlen*sizeof(WCHAR));
			}
		else
			long_path=(WCHAR *)malloc(pathlen*sizeof(WCHAR));
		if (long_path==NULL){
			if (long_path_save)
				free(long_path_save);
			PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", pathlen*sizeof(WCHAR));
			break;
			}
		Py_BEGIN_ALLOW_THREADS
		if (htrans)
			retlen=(*pfnGetLongPathNameTransacted)(short_path, long_path, pathlen, htrans);
		else
			retlen=(*pfnGetLongPathName)(short_path, long_path, pathlen);
		Py_END_ALLOW_THREADS
		if (retlen==0){
			PyWin_SetAPIError("GetLongPathName");
			break;
			}
		if (retlen<=pathlen){
			ret=PyUnicode_FromWideChar(long_path,retlen);
			break;
			}
		pathlen=retlen+1;
		}

	PyWinObject_FreeWCHAR(short_path);
	if (long_path)
		free(long_path);
	return ret;
}
PyCFunction pfnpy_GetLongPathName=(PyCFunction)py_GetLongPathName;

// @pyswig string|GetFullPathName|Returns full path for path passed in
// @comm This function takes either a bytes a unicode string, and returns the same type
//       If unicode is passed in, GetFullPathNameW is called, which supports filenames longer than MAX_PATH
// @comm If Transaction parameter is specified, GetFullPathNameTransacted is called
static PyObject *py_GetFullPathName(PyObject *self, PyObject *args, PyObject *kwargs)
{
	HANDLE htrans;
	PyObject *obtrans=Py_None, *ret=NULL, *obpathin;
	DWORD pathlen=MAX_PATH+1, retlen;
	static char *keywords[]={"FileName","Transaction", NULL};
#ifdef Py_DEBUG
	pathlen=3;	// test reallocation logic
#endif

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:GetFullPathName", keywords,
		&obpathin,	// @pyparm bytes/unicode|FileName||Path on which to operate
		&obtrans))	// @pyparm <o PyHANDLE>|Transaction|None|Handle to a transaction as returned by <om win32transaction.CreateTransaction>
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans))
		return NULL;

	if (TmpWCHAR wpathin=obpathin) {
		if (htrans)
			CHECK_PFN(GetFullPathNameTransactedW);
		WCHAR *wpathret=NULL, *wfilepart, *wpathsave=NULL;
		while (1){
			if (wpathret){
				wpathsave=wpathret;
				wpathret=(WCHAR *)realloc(wpathret,pathlen*sizeof(WCHAR));
				}
			else
				wpathret=(WCHAR *)malloc(pathlen*sizeof(WCHAR));
			if (wpathret==NULL){
				if (wpathsave)
					free(wpathsave);
				PyErr_SetString(PyExc_MemoryError,"GetFullPathNameW: unable to allocate unicode return buffer");
				return NULL;
				}
			Py_BEGIN_ALLOW_THREADS
			if (htrans)
				retlen=(*pfnGetFullPathNameTransactedW)(wpathin, pathlen, wpathret, &wfilepart, htrans);
			else
				retlen=GetFullPathNameW(wpathin, pathlen, wpathret, &wfilepart);
			Py_END_ALLOW_THREADS
			if (retlen==0){
				PyWin_SetAPIError("GetFullPathNameW");
				break;
				}
			if (retlen<=pathlen){
				ret=PyUnicode_FromWideChar(wpathret,retlen);
				break;
				}
			pathlen=retlen;
			}
		free(wpathret);
		return ret;
		}

	PyErr_Clear();
	char *cpathin;
	if (cpathin=PyBytes_AsString(obpathin)){
		if (htrans)
			CHECK_PFN(GetFullPathNameTransactedA);
		char *cpathret=NULL, *cfilepart, *cpathsave=NULL;
		while (1){
			if (cpathret){
				cpathsave=cpathret;
				cpathret=(char *)realloc(cpathret,pathlen);
				}
			else
				cpathret=(char *)malloc(pathlen);
			if (cpathret==NULL){
				if (cpathsave)
					free(cpathsave);
				PyErr_SetString(PyExc_MemoryError,"GetFullPathNameW: unable to allocate unicode return buffer");
				return NULL;
				}
			Py_BEGIN_ALLOW_THREADS
			if (htrans)
				retlen=(*pfnGetFullPathNameTransactedA)(cpathin, pathlen, cpathret, &cfilepart, htrans);
			else
				retlen=GetFullPathNameA(cpathin, pathlen, cpathret, &cfilepart);
			Py_END_ALLOW_THREADS
			if (retlen==0){
				PyWin_SetAPIError("GetFullPathNameA");
				break;
				}
			if (retlen<=pathlen){
				ret=PyBytes_FromStringAndSize(cpathret,retlen);
				break;
				}
			pathlen=retlen;
			}
		free(cpathret);
		}
	return ret;
}
PyCFunction pfnpy_GetFullPathName=(PyCFunction)py_GetFullPathName;

// @pyswig int|Wow64DisableWow64FsRedirection|Disables file system redirection for 32-bit processes running on a 64-bit system
// @rdesc Returns a state value to be passed to <om win32file.Wow64RevertWow64FsRedirection>
// @comm Requires 64-bit XP or later
static PyObject *py_Wow64DisableWow64FsRedirection(PyObject *self, PyObject *args)
{
	VOID *state;
	CHECK_PFN(Wow64DisableWow64FsRedirection);
	if (!PyArg_ParseTuple(args, ":Wow64DisableWow64FsRedirection"))
		return NULL;
	if (!(*pfnWow64DisableWow64FsRedirection)(&state))
		return PyWin_SetAPIError("Wow64DisableWow64FsRedirection");
	return PyWinLong_FromVoidPtr(state);
}

// @pyswig |Wow64RevertWow64FsRedirection|Reenables file system redirection for 32-bit processes running on a 64-bit system
// @comm Requires 64-bit XP or later
static PyObject *py_Wow64RevertWow64FsRedirection(PyObject *self, PyObject *args)
{
	VOID *state;
	CHECK_PFN(Wow64RevertWow64FsRedirection);
	// @pyparm int|OldValue||State returned from Wow64DisableWow64FsRedirection
	if (!PyArg_ParseTuple(args, "O&:Wow64RevertWow64FsRedirection", PyWinLong_AsVoidPtr, &state))
		return NULL;
	if (!(*pfnWow64RevertWow64FsRedirection)(state))
		return PyWin_SetAPIError("Wow64RevertWow64FsRedirection");
	Py_INCREF(Py_None);
	return Py_None;
}
%}

%{
// @pyswig object|GetFileInformationByHandleEx|Retrieves extended file information for an open file handle.
// @comm Available on Vista and later.
// @comm Accepts keyword args.
// @rdesc Type of returned object is determined by the requested information class
// @flagh Class|Returned info
// @flag FileBasicInfo|Dict representing a FILE_BASIC_INFO struct
// @flag FileStandardInfo|Dict representing a FILE_STANDARD_INFO struct
// @flag FileNameInfo|String containing the file name, without the drive letter
// @flag FileCompressionInfo|Dict representing a FILE_COMPRESSION_INFO struct
// @flag FileAttributeTagInfo|Dict representing a FILE_ATTRIBUTE_TAG_INFO struct
// @flag FileIdBothDirectoryInfo|Sequence of dicts representing FILE_ID_BOTH_DIR_INFO structs.  Call in loop until no more files are returned.
// @flag FileIdBothDirectoryRestartInfo|Sequence of dicts representing FILE_ID_BOTH_DIR_INFO structs.
// @flag FileStreamInfo|Sequence of dicts representing FILE_STREAM_INFO structs
static PyObject *py_GetFileInformationByHandleEx(PyObject *self, PyObject *args, PyObject *kwargs)
{
	// According to MSDN, this function is in kernel32.lib in Vista or later, but I can't get it to link
	//	with either Vista or Windows 7 sdks.
	CHECK_PFN(GetFileInformationByHandleEx);
	static char *keywords[] = {"File", "FileInformationClass", NULL};
	HANDLE handle;
	FILE_INFO_BY_HANDLE_CLASS info_class;
	void *buf = NULL;
	DWORD buflen = 0;
	BOOL rc;
	DWORD err = 0;
	PyObject *ret;

	// @pyparm <o PyHANDLE>|File||Handle to a file or directory.  Do not pass a pipe handle.
	// @pyparm int|FileInformationClass||Type of data to return, one of win32file.File*Info values
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i:GetFileInformationByHandleEx", keywords,
		PyWinObject_AsHANDLE, &handle,
		&info_class))
		return NULL;

	switch (info_class){
		case FileBasicInfo:
			buflen = sizeof(FILE_BASIC_INFO);
			break;
		case FileStandardInfo:
			buflen = sizeof(FILE_STANDARD_INFO);
			break;
		case FileNameInfo:
			// FILE_NAME_INFO.FileName is one of those bleeping [1] sized arrays that is treated as variable size.
			buflen = sizeof(FILE_NAME_INFO) + (sizeof(WCHAR) * (MAX_PATH + 1));
			// buflen = sizeof(FILE_NAME_INFO) + (10); // Test reallocation logic
			break;
		case FileCompressionInfo:
			buflen = sizeof(FILE_COMPRESSION_INFO);
			break;
		case FileAttributeTagInfo:
			buflen = sizeof(FILE_ATTRIBUTE_TAG_INFO);
			break;
		// These all return multiple linked structs, allocate extra space.  May need to allow for a
		// size hint to be passed in if large number of results expected.
		case FileIdBothDirectoryInfo:
		case FileIdBothDirectoryRestartInfo:
		case FileStreamInfo:
			buflen = 2048;
			break;
		default:
			PyErr_SetString(PyExc_NotImplementedError, "Unsupported file information class");
			return NULL;
		}

	while (true){
		if (buf)
			free(buf);
		buf = malloc(buflen);
		if (buf == NULL){
			PyErr_NoMemory();
			return NULL;
			}
		Py_BEGIN_ALLOW_THREADS
		rc = (*pfnGetFileInformationByHandleEx)(handle, info_class, buf, buflen);
		// rc = GetFileInformationByHandleEx(handle, info_class, buf, buflen);
		Py_END_ALLOW_THREADS
		if (rc)
			break;
		err = GetLastError();
		// ERROR_MORE_DATA can be returned if:
		//	FileStreamInfo is called on a file with numerous alternate streams
		//  FileNameInfo is called on a file whose name exceeds MAX_PATH
		if (err == ERROR_MORE_DATA){
			buflen *= 2;
			continue;
			}
		// ERROR_NO_MORE_FILES is returned when using FileIdBothDirectoryInfo and enumeration is done.
		// ERROR_HANDLE_EOF is returned when FileStreamInfo is called for a file with no streams
		//	(should only be for directories).
		// Treat either of these as success, and return empty tuple instead of raising an exception
		if ((err == ERROR_NO_MORE_FILES &&
				(info_class == FileIdBothDirectoryInfo || info_class == FileIdBothDirectoryRestartInfo))
			|| (err == ERROR_HANDLE_EOF && info_class == FileStreamInfo))
			rc = true;
		break;
		}

	if (!rc){
		free(buf);
		return PyWin_SetAPIError("GetFileInformationByHandleEx", err);
		}
	switch (info_class){
		case FileBasicInfo:{
			FILE_BASIC_INFO *pbi = (FILE_BASIC_INFO *)buf;
			ret = Py_BuildValue("{s:N, s:N, s:N, s:N, s:k}",
				"CreationTime", PyWinObject_FromTimeStamp(pbi->CreationTime),
				"LastAccessTime", PyWinObject_FromTimeStamp(pbi->LastAccessTime),
				"LastWriteTime", PyWinObject_FromTimeStamp(pbi->LastWriteTime),
				"ChangeTime", PyWinObject_FromTimeStamp(pbi->ChangeTime),
				"FileAttributes", pbi->FileAttributes);
			break;
			}
		case FileStandardInfo:{
			FILE_STANDARD_INFO *psi = (FILE_STANDARD_INFO *)buf;
			ret = Py_BuildValue("{s:N, s:N, s:k, s:N, s:N}",
				"AllocationSize", PyWinObject_FromLARGE_INTEGER(psi->AllocationSize),
				"EndOfFile", PyWinObject_FromLARGE_INTEGER(psi->EndOfFile),
				"NumberOfLinks", psi->NumberOfLinks,
				"DeletePending", PyBool_FromLong(psi->DeletePending),
				"Directory", PyBool_FromLong(psi->Directory));
			break;
			}
		case FileNameInfo:{
			FILE_NAME_INFO *pni = (FILE_NAME_INFO *)buf;
			ret = PyWinObject_FromWCHAR(pni->FileName, pni->FileNameLength/sizeof(WCHAR));
			break;
			}
		case FileCompressionInfo:{
			FILE_COMPRESSION_INFO *pci = (FILE_COMPRESSION_INFO *)buf;
			ret = Py_BuildValue("{s:N, s:H, s:B, s:B, s:B, s:(BBB)}",
				"CompressedFileSize", PyWinObject_FromLARGE_INTEGER(pci->CompressedFileSize),
				"CompressionFormat", pci->CompressionFormat,
				"CompressionUnitShift", pci->CompressionUnitShift,
				"ChunkShift", pci->ChunkShift,
				"ClusterShift", pci->ClusterShift,
				"Reserved", pci->Reserved[0], pci->Reserved[1], pci->Reserved[2]);
			break;
			}
		case FileAttributeTagInfo:{
			FILE_ATTRIBUTE_TAG_INFO *pati = (FILE_ATTRIBUTE_TAG_INFO *)buf;
			ret = Py_BuildValue("{s:k, s:k}",
				"FileAttributes", pati->FileAttributes,
				"ReparseTag", pati->ReparseTag);
			break;
			}
		case FileIdBothDirectoryInfo:
		case FileIdBothDirectoryRestartInfo:{
			FILE_ID_BOTH_DIR_INFO *pdi = (FILE_ID_BOTH_DIR_INFO *)buf;
			if (err == ERROR_NO_MORE_FILES){
				ret = PyTuple_New(0);
				break;
				}
			ULONG file_cnt = 1;
			while (pdi->NextEntryOffset){
				file_cnt++;
				pdi = (FILE_ID_BOTH_DIR_INFO *)((BYTE *)pdi + pdi->NextEntryOffset);
				};
			ret = PyTuple_New(file_cnt);
			if (ret == NULL)
				break;
			pdi = (FILE_ID_BOTH_DIR_INFO *)buf;
			for (ULONG i = 0; i < file_cnt; i++){
				PyObject *file_info = Py_BuildValue("{s:k, s:N, s:N, s:N, s:N, s:N, s:N, s:k, s:k, s:N, s:N, s:N}",
					"FileIndex", pdi->FileIndex,
					"CreationTime", PyWinObject_FromTimeStamp(pdi->CreationTime),
					"LastAccessTime", PyWinObject_FromTimeStamp(pdi->LastAccessTime),
					"LastWriteTime", PyWinObject_FromTimeStamp(pdi->LastWriteTime),
					"ChangeTime", PyWinObject_FromTimeStamp(pdi->ChangeTime),
					"EndOfFile", PyWinObject_FromLARGE_INTEGER(pdi->EndOfFile),
					"AllocationSize", PyWinObject_FromLARGE_INTEGER(pdi->AllocationSize),
					"FileAttributes", pdi->FileAttributes,
					"EaSize", pdi->EaSize,
					"ShortName", PyWinObject_FromWCHAR(pdi->ShortName, pdi->ShortNameLength/sizeof(WCHAR)),
					"FileId", PyWinObject_FromLARGE_INTEGER(pdi->FileId),
					"FileName", PyWinObject_FromWCHAR(pdi->FileName, pdi->FileNameLength/sizeof(WCHAR)));
				if (file_info == NULL){
					Py_DECREF(ret);
					ret = NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, i, file_info);
				pdi = (FILE_ID_BOTH_DIR_INFO *)((BYTE *)pdi + pdi->NextEntryOffset);
				}
			break;
			}
		case FileStreamInfo:{
			FILE_STREAM_INFO *psi = (FILE_STREAM_INFO *)buf;
			if (err == ERROR_HANDLE_EOF){
				ret = PyTuple_New(0);
				break;
				}
			// Function fails if no streams retrieved, so guaranteed to have at least one struct
			DWORD stream_cnt = 1;
			while (psi->NextEntryOffset){
				stream_cnt++;
				psi = (FILE_STREAM_INFO *)((BYTE *)psi + psi->NextEntryOffset);
				};
			ret = PyTuple_New(stream_cnt);
			if (ret == NULL)
				break;
			psi = (FILE_STREAM_INFO *)buf;
			for (DWORD i = 0; i < stream_cnt; i++){
				PyObject *stream_info = Py_BuildValue("{s:N, s:N, s:N}",
					"StreamSize", PyWinObject_FromLARGE_INTEGER(psi->StreamSize),
					"StreamAllocationSize", PyWinObject_FromLARGE_INTEGER(psi->StreamAllocationSize),
					"StreamName", PyWinObject_FromWCHAR(psi->StreamName, psi->StreamNameLength/sizeof(WCHAR)));
				if (stream_info == NULL){
					Py_DECREF(ret);
					ret = NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, i, stream_info);
				psi = (FILE_STREAM_INFO *)((BYTE *)psi + psi->NextEntryOffset);
				};
			break;
			}
		default:
			PyErr_SetString(PyExc_SystemError, "Mismatched case statements");
		}
	free(buf);
	return ret;
}
PyCFunction pfnpy_GetFileInformationByHandleEx=(PyCFunction)py_GetFileInformationByHandleEx;
%}

%{
// @pyswig |SetFileInformationByHandle|Changes file characteristics by file handle
// @comm Available on Vista and later.
// @comm Accepts keyword args.
static PyObject *py_SetFileInformationByHandle(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(SetFileInformationByHandle);
	static char *keywords[] = {"File", "FileInformationClass", "Information", NULL};
	HANDLE handle;
	FILE_INFO_BY_HANDLE_CLASS info_class;
	void *buf = NULL;
	DWORD buflen = 0;
	BOOL rc = FALSE;
	PyObject *info;

	// @pyparm <o PyHANDLE>|File||Handle to a file or directory.  Do not pass a pipe handle.
	// @pyparm int|FileInformationClass||Type of data, one of win32file.File*Info values
	// @pyparm object|Information||Type is dependent on the class to be changed
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&iO:SetFileInformationByHandle", keywords,
		PyWinObject_AsHANDLE, &handle,
		&info_class, &info))
		return NULL;

	// @flagh Class|Type of input
	switch (info_class){
		// @flag FileBasicInfo|Dict representing a FILE_BASIC_INFO struct, containing
		// {"CreationTime":<o PyDateTime>, "LastAccessTime":<o PyDateTime>,  "LastWriteTime":<o PyDateTime>,
		//		"ChangeTime":<o PyDateTime>, "FileAttributes":int}
		case FileBasicInfo:{
			TmpPyObject dummy_tuple = PyTuple_New(0);
			if (dummy_tuple == NULL)
				return NULL;
			buflen = sizeof(FILE_BASIC_INFO);
			FILE_BASIC_INFO *pbi = (FILE_BASIC_INFO *)malloc(buflen);
			if (pbi == NULL)
				break;
			buf = pbi;
			static char *keywords[] = {"CreationTime", "LastAccessTime",  "LastWriteTime",
				"ChangeTime", "FileAttributes", NULL};
			// The times are LARGE_INTEGER's (identical to timestamp), but can be converted as FILETIME's.
			rc = PyArg_ParseTupleAndKeywords(dummy_tuple, info, "O&O&O&O&k", keywords,
				PyWinObject_AsFILETIME, &pbi->CreationTime,
				PyWinObject_AsFILETIME, &pbi->LastAccessTime,
				PyWinObject_AsFILETIME, &pbi->LastWriteTime,
				PyWinObject_AsFILETIME, &pbi->ChangeTime,
				&pbi->FileAttributes);
			break;
			}
		// @flag FileRenameInfo|Dict representing a FILE_RENAME_INFO struct, containing
		// {"ReplaceIfExists":boolean, "RootDirectory":<o PyHANDLE>, "FileName":str}
		// MSDN says the RootDirectory is "A handle to the root directory in which the file to be renamed is located".
		// However, this is actually the destination dir, can be None to stay in same dir.
		case FileRenameInfo:{
			TmpPyObject dummy_tuple = PyTuple_New(0);
			if (dummy_tuple == NULL)
				return NULL;
			// Variable size struct, need to convert filename first to determine full length
			FILE_RENAME_INFO *pri;
			PyObject *obFileName;
			TmpWCHAR FileName;
			DWORD FileNameLength;
			BOOL ReplaceIfExists;
			HANDLE RootDirectory;

			static char *keywords[] = {"ReplaceIfExists", "RootDirectory", "FileName", NULL};
			rc = PyArg_ParseTupleAndKeywords(dummy_tuple, info, "iO&O", keywords,
				&ReplaceIfExists,
				PyWinObject_AsHANDLE, &RootDirectory,
				&obFileName)
				&& PyWinObject_AsWCHAR(obFileName, &FileName, FALSE, &FileNameLength);
			if (!rc)
				return NULL;
			buflen = sizeof(FILE_RENAME_INFO) + (FileNameLength * sizeof(WCHAR));
			pri = (FILE_RENAME_INFO *)malloc(buflen);
			if (pri == NULL)
				break;
			buf = pri;
			pri->ReplaceIfExists = ReplaceIfExists;
			pri->RootDirectory = RootDirectory;
			wcsncpy(pri->FileName, FileName, FileNameLength + 1);
			pri->FileNameLength = FileNameLength * sizeof(WCHAR);
			break;
			}
		// @flag FileDispositionInfo|Boolean indicating if file should be deleted when handle is closed
		case FileDispositionInfo:{
			buflen = sizeof(FILE_DISPOSITION_INFO);
			FILE_DISPOSITION_INFO *pdi = (FILE_DISPOSITION_INFO *)malloc(buflen);
			if (pdi == NULL)
				break;
			buf = pdi;
			// Thought this always succeeded, need to add error checking to other places it's used
			pdi->DeleteFile = PyObject_IsTrue(info);
			rc = pdi->DeleteFile != -1;
			break;
			}
		// @flag FileAllocationInfo|Int giving the allocation size.
		case FileAllocationInfo:{
			buflen = sizeof(FILE_ALLOCATION_INFO);
			FILE_ALLOCATION_INFO *pai = (FILE_ALLOCATION_INFO *)malloc(buflen);
			if (pai == NULL)
				break;
			buf = pai;
			rc = PyWinObject_AsLARGE_INTEGER(info, &pai->AllocationSize);
			break;
			}
		// @flag FileEndOfFileInfo|Int giving the EOF position, cannot be greater than allocated size.
		case FileEndOfFileInfo:{
			buflen = sizeof(FILE_END_OF_FILE_INFO);
			FILE_END_OF_FILE_INFO *peofi = (FILE_END_OF_FILE_INFO *)malloc(buflen);
			if (peofi == NULL)
				break;
			buf = peofi;
			rc = PyWinObject_AsLARGE_INTEGER(info, &peofi->EndOfFile);
			break;
			}
		// @flag FileIoPriorityHintInfo|Int containing the IO priority (IoPriorityHint*)
		case FileIoPriorityHintInfo:{
			buflen = sizeof(FILE_IO_PRIORITY_HINT_INFO);
			FILE_IO_PRIORITY_HINT_INFO *piohi= (FILE_IO_PRIORITY_HINT_INFO *)malloc(buflen);
			if (piohi == NULL)
				break;
			buf = piohi;
			piohi->PriorityHint = (PRIORITY_HINT)PyLong_AsLong(info);
			rc = piohi->PriorityHint != -1 || !PyErr_Occurred();
			break;
			}
		default:
			PyErr_SetString(PyExc_NotImplementedError, "Unsupported file information class");
			return NULL;
		}
	if (buf == NULL){
		PyErr_NoMemory();
		return NULL;
		}
	if (!rc){
		free(buf);
		return NULL;
		}

	Py_BEGIN_ALLOW_THREADS
	rc = (*pfnSetFileInformationByHandle)(handle, info_class, buf, buflen);
	// rc = SetFileInformationByHandle(handle, info_class, buf, buflen);
	Py_END_ALLOW_THREADS
	free(buf);
	if (rc){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("SetFileInformationByHandle");
}
PyCFunction pfnpy_SetFileInformationByHandle=(PyCFunction)py_SetFileInformationByHandle;
%}

%{
// @pyswig <o PyHANDLE>|ReOpenFile|Creates a new handle to an open file
// @comm Available on Vista and later.
// @comm Accepts keyword args.
static PyObject *py_ReOpenFile(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(ReOpenFile);
	static char *keywords[] = {"OriginalFile", "DesiredAccess", "ShareMode", "Flags", NULL};
	HANDLE horig, hret;
	DWORD DesiredAccess, ShareMode, Flags;
	// @pyparm <o PyHANDLE>|OriginalFile||An open file handle
	// @pyparm int|DesiredAccess||Access mode, cannot conflict with original access mode
	// @pyparm int|ShareMode||Sharing mode (FILE_SHARE_*), cannot conflict with original share mode
	// @pyparm int|Flags||Combination of FILE_FLAG_* flags
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&kkk:ReOpenFile", keywords,
		PyWinObject_AsHANDLE, &horig, &DesiredAccess, &ShareMode, &Flags))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	hret = (*pfnReOpenFile)(horig, DesiredAccess, ShareMode, Flags);
	// hret = ReOpenFile(horig, DesiredAccess, ShareMode, Flags);
	Py_END_ALLOW_THREADS
	if (hret == INVALID_HANDLE_VALUE)
		return PyWin_SetAPIError("ReOpenFile");
	return PyWinObject_FromHANDLE(hret);
}
PyCFunction pfnpy_ReOpenFile=(PyCFunction)py_ReOpenFile;
%}

%{
// @pyswig <o PyHANDLE>|OpenFileById|Opens a file by File Id or Object Id
// @comm Available on Vista and later.
// @comm Accepts keyword args.
static PyObject *py_OpenFileById(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(OpenFileById);
	static char *keywords[] = {"File", "FileID", "DesiredAccess", "ShareMode",
		"Flags", "SecurityAttributes", NULL};
	HANDLE hvol, hret;
	DWORD DesiredAccess, ShareMode, Flags;
	PyObject *obsa = Py_None;
	PSECURITY_ATTRIBUTES sa;
	PyObject *obfileid;
	FILE_ID_DESCRIPTOR fileid = {sizeof(FILE_ID_DESCRIPTOR)};

	// @pyparm <o PyHANDLE>|File||Handle to a file on the volume that contains the file to open
	// @pyparm int/<o PyIID>|FileId||File Id or Object Id of the file to open
	// @pyparm int|DesiredAccess||Access mode
	// @pyparm int|ShareMode||Sharing mode (FILE_SHARE_*)
	// @pyparm int|Flags||Combination of FILE_FLAG_* flags
	// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes|None|Reserved, use only None
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&Okkk|O:OpenFileById", keywords,
		PyWinObject_AsHANDLE, &hvol, &obfileid, &DesiredAccess, &ShareMode, &Flags, &obsa))
		return NULL;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &sa, TRUE))
		return NULL;

	fileid.Type = FileIdType;
	if (!PyWinObject_AsLARGE_INTEGER(obfileid, &fileid.FileId)){
		PyErr_Clear();
		fileid.Type = ObjectIdType;
		if (!PyWinObject_AsIID(obfileid, &fileid.ObjectId)){
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "FileId must be an integer or GUID");
			return NULL;
			}
		}

	Py_BEGIN_ALLOW_THREADS
	hret = (*pfnOpenFileById)(hvol, &fileid, DesiredAccess, ShareMode, sa, Flags);
	// hret = OpenFileById(hvol, &fileid, DesiredAccess, ShareMode, sa, Flags);
	Py_END_ALLOW_THREADS
	if (hret == INVALID_HANDLE_VALUE)
		return PyWin_SetAPIError("OpenFileById");
	return PyWinObject_FromHANDLE(hret);
}
PyCFunction pfnpy_OpenFileById=(PyCFunction)py_OpenFileById;
%}


%native (SetVolumeMountPoint) pfnpy_SetVolumeMountPoint;
%native (DeleteVolumeMountPoint) pfnpy_DeleteVolumeMountPoint;
%native (GetVolumeNameForVolumeMountPoint) pfnpy_GetVolumeNameForVolumeMountPoint;
%native (GetVolumePathName) pfnpy_GetVolumePathName;
%native (GetVolumePathNamesForVolumeName) pfnpy_GetVolumePathNamesForVolumeName;

%native (CreateHardLink) pfnpy_CreateHardLink;
%native (CreateSymbolicLink) pfnpy_CreateSymbolicLink;

// end of win2k volume mount functions.
%native (EncryptFile) py_EncryptFile;
%native (DecryptFile) py_DecryptFile;
%native (EncryptionDisable) py_EncryptionDisable;
%native (FileEncryptionStatus) py_FileEncryptionStatus;
%native (QueryUsersOnEncryptedFile) py_QueryUsersOnEncryptedFile;
%native (QueryRecoveryAgentsOnEncryptedFile) py_QueryRecoveryAgentsOnEncryptedFile;
%native (RemoveUsersFromEncryptedFile) py_RemoveUsersFromEncryptedFile;
%native (AddUsersToEncryptedFile) py_AddUsersToEncryptedFile;
%native (DuplicateEncryptionInfoFile) pfnpy_DuplicateEncryptionInfoFile;

%native (BackupRead) py_BackupRead;
%native (BackupSeek) py_BackupSeek;
%native (BackupWrite) py_BackupWrite;
%native (SetFileShortName) py_SetFileShortName;
%native (CopyFileEx) pfnpy_CopyFileEx;
%native (MoveFileWithProgress) pfnpy_MoveFileWithProgress;
%native (ReplaceFile) py_ReplaceFile;
%native (OpenEncryptedFileRaw) py_OpenEncryptedFileRaw;
%native (ReadEncryptedFileRaw) py_ReadEncryptedFileRaw;
%native (WriteEncryptedFileRaw) py_WriteEncryptedFileRaw;
%native (CloseEncryptedFileRaw) py_CloseEncryptedFileRaw;
%native (CreateFileW) pfnpy_CreateFileW;
%native (DeleteFileW) pfnpy_DeleteFileW;
%native (GetFileAttributesEx) pfnpy_GetFileAttributesEx;
%native (GetFileAttributesExW) pfnpy_GetFileAttributesExW;
%native (GetFileInformationByHandleEx) pfnpy_GetFileInformationByHandleEx;
%native (SetFileInformationByHandle) pfnpy_SetFileInformationByHandle;
%native (SetFileAttributesW) pfnpy_SetFileAttributesW;
%native (CreateDirectoryExW) pfnpy_CreateDirectoryExW;
%native (RemoveDirectory) pfnpy_RemoveDirectory;
%native (FindFilesW) pfnpy_FindFilesW;
%native (FindFilesIterator) pfnpy_FindFilesIterator;
%native (FindStreams) pfnpy_FindStreams;
%native (FindFileNames) pfnpy_FindFileNames;
%native (GetFinalPathNameByHandle) pfnpy_GetFinalPathNameByHandle;
%native (GetLongPathName) pfnpy_GetLongPathName;
%native (GetFullPathName) pfnpy_GetFullPathName;

%native (SfcGetNextProtectedFile) py_SfcGetNextProtectedFile;
%native (SfcIsFileProtected) py_SfcIsFileProtected;

%native (Wow64DisableWow64FsRedirection) py_Wow64DisableWow64FsRedirection;
%native (Wow64RevertWow64FsRedirection) py_Wow64RevertWow64FsRedirection;
%native (ReOpenFile) pfnpy_ReOpenFile;
%native (OpenFileById) pfnpy_OpenFileById;


%init %{

	if (PyType_Ready(&FindFileIterator_Type) == -1
		||PyType_Ready(&PyDCB::type) == -1
		||PyType_Ready(&PyCOMSTAT::type) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;

	if (PyDict_SetItemString(d, "error", PyWinExc_ApiError) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;
	if (PyDict_SetItemString(d, "INVALID_HANDLE_VALUE", PyWinLong_FromHANDLE(INVALID_HANDLE_VALUE)) == -1)
		PYWIN_MODULE_INIT_RETURN_ERROR;

	PyDateTime_IMPORT;

	for (PyMethodDef *pmd = win32fileMethods;pmd->ml_name;pmd++)
		if   ((strcmp(pmd->ml_name, "CreateFileW")==0)
			||(strcmp(pmd->ml_name, "DeleteFileW")==0)
			||(strcmp(pmd->ml_name, "MoveFileWithProgress")==0)
			||(strcmp(pmd->ml_name, "CopyFileEx")==0)
			||(strcmp(pmd->ml_name, "GetFileAttributesEx")==0)
			||(strcmp(pmd->ml_name, "GetFileAttributesExW")==0)
			||(strcmp(pmd->ml_name, "SetFileAttributesW")==0)
			||(strcmp(pmd->ml_name, "CreateHardLink")==0)
			||(strcmp(pmd->ml_name, "CreateSymbolicLink")==0)
			||(strcmp(pmd->ml_name, "CreateDirectoryExW")==0)
			||(strcmp(pmd->ml_name, "RemoveDirectory")==0)
			||(strcmp(pmd->ml_name, "FindFilesW")==0)
			||(strcmp(pmd->ml_name, "FindFilesIterator")==0)
			||(strcmp(pmd->ml_name, "FindStreams")==0)
			||(strcmp(pmd->ml_name, "FindFileNames")==0)
			||(strcmp(pmd->ml_name, "GetFinalPathNameByHandle")==0)
			||(strcmp(pmd->ml_name, "SetVolumeMountPoint")==0)
			||(strcmp(pmd->ml_name, "DeleteVolumeMountPoint")==0)
			||(strcmp(pmd->ml_name, "GetVolumeNameForVolumeMountPoint")==0)
			||(strcmp(pmd->ml_name, "GetVolumePathName")==0)
			||(strcmp(pmd->ml_name, "GetVolumePathNamesForVolumeName")==0)
			||(strcmp(pmd->ml_name, "DuplicateEncryptionInfoFile")==0)
			||(strcmp(pmd->ml_name, "GetLongPathName")==0)
			||(strcmp(pmd->ml_name, "GetFullPathName")==0)
			||(strcmp(pmd->ml_name, "GetFileInformationByHandleEx")==0)
			||(strcmp(pmd->ml_name, "SetFileInformationByHandle")==0)
			||(strcmp(pmd->ml_name, "DeviceIoControl")==0)
			||(strcmp(pmd->ml_name, "TransmitFile")==0)
			||(strcmp(pmd->ml_name, "ConnectEx")==0)
			||(strcmp(pmd->ml_name, "ReOpenFile")==0)
			||(strcmp(pmd->ml_name, "OpenFileById")==0)
			||(strcmp(pmd->ml_name, "SetFileTime")==0)
			)
			pmd->ml_flags = METH_VARARGS | METH_KEYWORDS;

	HMODULE hmodule = PyWin_GetOrLoadLibraryHandle("advapi32.dll");
	if (hmodule != NULL) {
		pfnEncryptFile=(EncryptFilefunc)GetProcAddress(hmodule, "EncryptFileW");
		pfnDecryptFile=(DecryptFilefunc)GetProcAddress(hmodule, "DecryptFileW");
		pfnEncryptionDisable=(EncryptionDisablefunc)GetProcAddress(hmodule, "EncryptionDisable");
		pfnFileEncryptionStatus=(FileEncryptionStatusfunc)GetProcAddress(hmodule, "FileEncryptionStatusW");
		pfnQueryUsersOnEncryptedFile=(QueryUsersOnEncryptedFilefunc)GetProcAddress(hmodule, "QueryUsersOnEncryptedFile");
		pfnFreeEncryptionCertificateHashList=(FreeEncryptionCertificateHashListfunc)GetProcAddress(hmodule, "FreeEncryptionCertificateHashList");
		pfnQueryRecoveryAgentsOnEncryptedFile=(QueryRecoveryAgentsOnEncryptedFilefunc)GetProcAddress(hmodule, "QueryRecoveryAgentsOnEncryptedFile");
		pfnRemoveUsersFromEncryptedFile=(RemoveUsersFromEncryptedFilefunc)GetProcAddress(hmodule, "RemoveUsersFromEncryptedFile");
		pfnAddUsersToEncryptedFile=(AddUsersToEncryptedFilefunc)GetProcAddress(hmodule, "AddUsersToEncryptedFile");
		pfnDuplicateEncryptionInfoFile=(DuplicateEncryptionInfoFilefunc)GetProcAddress(hmodule, "DuplicateEncryptionInfoFile");

		pfnOpenEncryptedFileRaw=(OpenEncryptedFileRawfunc)GetProcAddress(hmodule, "OpenEncryptedFileRawW");
		pfnReadEncryptedFileRaw=(ReadEncryptedFileRawfunc)GetProcAddress(hmodule, "ReadEncryptedFileRaw");
		pfnWriteEncryptedFileRaw=(WriteEncryptedFileRawfunc)GetProcAddress(hmodule, "WriteEncryptedFileRaw");
		pfnCloseEncryptedFileRaw=(CloseEncryptedFileRawfunc)GetProcAddress(hmodule, "CloseEncryptedFileRaw");
	}

	hmodule = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
	if (hmodule != NULL) {
		pfnSetVolumeMountPoint=(SetVolumeMountPointfunc)GetProcAddress(hmodule, "SetVolumeMountPointW");
		pfnDeleteVolumeMountPoint=(DeleteVolumeMountPointfunc)GetProcAddress(hmodule, "DeleteVolumeMountPointW");
		pfnGetVolumeNameForVolumeMountPoint=(GetVolumeNameForVolumeMountPointfunc)GetProcAddress(hmodule, "GetVolumeNameForVolumeMountPointW");
		pfnGetVolumePathName=(GetVolumePathNamefunc)GetProcAddress(hmodule, "GetVolumePathNameW");
		pfnGetVolumePathNamesForVolumeName=(GetVolumePathNamesForVolumeNamefunc)GetProcAddress(hmodule, "GetVolumePathNamesForVolumeNameW");

		pfnCreateHardLink=(CreateHardLinkfunc)GetProcAddress(hmodule, "CreateHardLinkW");
		pfnCreateHardLinkTransacted=(CreateHardLinkTransactedfunc)GetProcAddress(hmodule, "CreateHardLinkTransactedW");
		pfnCreateSymbolicLink=(CreateSymbolicLinkfunc)GetProcAddress(hmodule, "CreateSymbolicLinkW");
		pfnCreateSymbolicLinkTransacted=(CreateSymbolicLinkTransactedfunc)GetProcAddress(hmodule, "CreateSymbolicLinkTransactedW");
		pfnBackupRead=(BackupReadfunc)GetProcAddress(hmodule,"BackupRead");
		pfnBackupSeek=(BackupSeekfunc)GetProcAddress(hmodule,"BackupSeek");
		pfnBackupWrite=(BackupWritefunc)GetProcAddress(hmodule,"BackupWrite");
		pfnSetFileShortName=(SetFileShortNamefunc)GetProcAddress(hmodule,"SetFileShortNameW");
		pfnCopyFileEx=(CopyFileExfunc)GetProcAddress(hmodule,"CopyFileExW");
		pfnCopyFileTransacted=(CopyFileTransactedfunc)GetProcAddress(hmodule, "CopyFileTransactedW");
		pfnMoveFileWithProgress=(MoveFileWithProgressfunc)GetProcAddress(hmodule,"MoveFileWithProgressW");
		pfnMoveFileTransacted=(MoveFileTransactedfunc)GetProcAddress(hmodule, "MoveFileTransactedW");
		pfnReplaceFile=(ReplaceFilefunc)GetProcAddress(hmodule,"ReplaceFileW");
		pfnCreateFileTransacted=(CreateFileTransactedfunc)GetProcAddress(hmodule, "CreateFileTransactedW");
		pfnDeleteFileTransacted=(DeleteFileTransactedfunc)GetProcAddress(hmodule, "DeleteFileTransactedW");
		pfnGetFileAttributesTransactedA=(GetFileAttributesTransactedAfunc)GetProcAddress(hmodule, "GetFileAttributesTransactedA");
		pfnGetFileAttributesTransactedW=(GetFileAttributesTransactedWfunc)GetProcAddress(hmodule, "GetFileAttributesTransactedW");
		pfnSetFileAttributesTransacted=(SetFileAttributesTransactedfunc)GetProcAddress(hmodule, "SetFileAttributesTransactedW");
		pfnCreateDirectoryTransacted=(CreateDirectoryTransactedfunc)GetProcAddress(hmodule, "CreateDirectoryTransactedW");
		pfnRemoveDirectoryTransacted=(RemoveDirectoryTransactedfunc)GetProcAddress(hmodule, "RemoveDirectoryTransactedW");
		pfnFindFirstStream=(FindFirstStreamfunc)GetProcAddress(hmodule, "FindFirstStreamW");
		pfnFindNextStream=(FindNextStreamfunc)GetProcAddress(hmodule, "FindNextStreamW");
		pfnFindFirstStreamTransacted=(FindFirstStreamTransactedfunc)GetProcAddress(hmodule, "FindFirstStreamTransactedW");
		pfnFindFirstFileTransacted=(FindFirstFileTransactedfunc)GetProcAddress(hmodule, "FindFirstFileTransactedW");
		pfnFindFirstFileName=(FindFirstFileNamefunc)GetProcAddress(hmodule, "FindFirstFileNameW");
		pfnFindFirstFileNameTransacted=(FindFirstFileNameTransactedfunc)GetProcAddress(hmodule, "FindFirstFileNameTransactedW");
		pfnFindNextFileName=(FindNextFileNamefunc)GetProcAddress(hmodule, "FindNextFileNameW");
		pfnGetFinalPathNameByHandle=(GetFinalPathNameByHandlefunc)GetProcAddress(hmodule, "GetFinalPathNameByHandleW");
		pfnGetLongPathName=(GetLongPathNamefunc)GetProcAddress(hmodule, "GetLongPathNameW");
		pfnGetLongPathNameTransacted=(GetLongPathNameTransactedfunc)GetProcAddress(hmodule, "GetLongPathNameTransactedW");
		pfnGetFullPathNameTransactedW=(GetFullPathNameTransactedWfunc)GetProcAddress(hmodule, "GetFullPathNameTransactedW");
		pfnGetFullPathNameTransactedA=(GetFullPathNameTransactedAfunc)GetProcAddress(hmodule, "GetFullPathNameTransactedA");
		pfnGetFileInformationByHandleEx=(GetFileInformationByHandleExfunc)GetProcAddress(hmodule, "GetFileInformationByHandleEx");
		pfnSetFileInformationByHandle=(SetFileInformationByHandlefunc)GetProcAddress(hmodule, "SetFileInformationByHandle");
		pfnWow64DisableWow64FsRedirection=(Wow64DisableWow64FsRedirectionfunc)GetProcAddress(hmodule, "Wow64DisableWow64FsRedirection");
		pfnWow64RevertWow64FsRedirection=(Wow64RevertWow64FsRedirectionfunc)GetProcAddress(hmodule, "Wow64RevertWow64FsRedirection");
		pfnReOpenFile=(ReOpenFilefunc)GetProcAddress(hmodule, "ReOpenFile");
		pfnOpenFileById=(OpenFileByIdfunc)GetProcAddress(hmodule, "OpenFileById");
	}

	hmodule = PyWin_GetOrLoadLibraryHandle("sfc.dll");
	if (hmodule != NULL) {
		pfnSfcGetNextProtectedFile=(SfcGetNextProtectedFilefunc)GetProcAddress(hmodule, "SfcGetNextProtectedFile");
		pfnSfcIsFileProtected=(SfcIsFileProtectedfunc)GetProcAddress(hmodule, "SfcIsFileProtected");
	}

%}

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

#define FILE_ENCRYPTABLE FILE_ENCRYPTABLE
#define FILE_IS_ENCRYPTED FILE_IS_ENCRYPTED
#define FILE_SYSTEM_ATTR FILE_SYSTEM_ATTR
#define FILE_ROOT_DIR FILE_ROOT_DIR
#define FILE_SYSTEM_DIR FILE_SYSTEM_DIR
#define FILE_UNKNOWN FILE_UNKNOWN
#define FILE_SYSTEM_NOT_SUPPORT FILE_SYSTEM_NOT_SUPPORT
#define FILE_USER_DISALLOWED FILE_USER_DISALLOWED
#define FILE_READ_ONLY FILE_READ_ONLY

#define TF_DISCONNECT TF_DISCONNECT
#define TF_REUSE_SOCKET TF_REUSE_SOCKET
#define TF_WRITE_BEHIND TF_WRITE_BEHIND
#define TF_USE_DEFAULT_WORKER TF_USE_DEFAULT_WORKER
#define TF_USE_SYSTEM_THREAD TF_USE_SYSTEM_THREAD
#define TF_USE_KERNEL_APC TF_USE_KERNEL_APC

// flags used with CopyFileEx
#define COPY_FILE_ALLOW_DECRYPTED_DESTINATION COPY_FILE_ALLOW_DECRYPTED_DESTINATION
#define COPY_FILE_FAIL_IF_EXISTS COPY_FILE_FAIL_IF_EXISTS
#define COPY_FILE_RESTARTABLE COPY_FILE_RESTARTABLE
#define COPY_FILE_OPEN_SOURCE_FOR_WRITE COPY_FILE_OPEN_SOURCE_FOR_WRITE
#define COPY_FILE_COPY_SYMLINK

// return codes from CopyFileEx progress routine
#define PROGRESS_CONTINUE PROGRESS_CONTINUE
#define PROGRESS_CANCEL PROGRESS_CANCEL
#define PROGRESS_STOP PROGRESS_STOP
#define PROGRESS_QUIET PROGRESS_QUIET

// callback reasons from CopyFileEx
#define CALLBACK_CHUNK_FINISHED CALLBACK_CHUNK_FINISHED
#define CALLBACK_STREAM_SWITCH CALLBACK_STREAM_SWITCH

// flags used with ReplaceFile
#define REPLACEFILE_IGNORE_MERGE_ERRORS REPLACEFILE_IGNORE_MERGE_ERRORS
#define REPLACEFILE_WRITE_THROUGH REPLACEFILE_WRITE_THROUGH

// flags for OpenEncryptedFileRaw
#define CREATE_FOR_IMPORT CREATE_FOR_IMPORT
#define CREATE_FOR_DIR CREATE_FOR_DIR
#define OVERWRITE_HIDDEN OVERWRITE_HIDDEN

// Info level for GetFileAttributesEx and GetFileAttributesTransacted (GET_FILEEX_INFO_LEVELS enum)
#define GetFileExInfoStandard 1

// Flags for CreateSymbolicLink/CreateSymbolicLinkTransacted
#define SYMBOLIC_LINK_FLAG_DIRECTORY 1
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 2

// FILE_INFO_BY_HANDLE_CLASS used with GetFileInformationByHandleEx
#define FileBasicInfo FileBasicInfo
#define FileStandardInfo FileStandardInfo
#define FileNameInfo FileNameInfo
#define FileRenameInfo FileRenameInfo
#define FileDispositionInfo FileDispositionInfo
#define FileAllocationInfo FileAllocationInfo
#define FileEndOfFileInfo FileEndOfFileInfo
#define FileStreamInfo FileStreamInfo
#define FileCompressionInfo FileCompressionInfo
#define FileAttributeTagInfo FileAttributeTagInfo
#define FileIdBothDirectoryInfo FileIdBothDirectoryInfo
#define FileIdBothDirectoryRestartInfo FileIdBothDirectoryRestartInfo
#define FileIoPriorityHintInfo FileIoPriorityHintInfo

#define IoPriorityHintVeryLow IoPriorityHintVeryLow
#define IoPriorityHintLow IoPriorityHintLow
#define IoPriorityHintNormal IoPriorityHintNormal

// used with OpenFileById
#define FileIdType FileIdType
#define ObjectIdType ObjectIdType
