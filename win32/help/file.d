/*
@doc

@topic Windows NT Files -- Locking |Python's win32 access for file locking -- flock style

<nl>The need for file locking tends arise every so often. Some people
may be used to flock style locking, which has 4 basic cases:
shared,exclusive,blocking and non-blocking. Shared locking is
typically used when multiple people want to read a file. Exclusive is
for writing.  Blocking means that the process will wait until it is
able to lock the file.  Non-blocking will return immediately and tell
you the lock failed.  In win32 the standard CreateFile api gives you
the ability to do exclusive or shared locking. However, what it does
not give you is the ability to switch between blocking/non-blocking
(it fails immediately -- does not block) To do that, you need to use
LockfileEx -- which can even lock a specific part of a file.

<nl>The basic procedure for doing this is to first call Createfile to 
give you a filehandle. Then call LockfileEx with the filehandle.
Do whatever to the file. Call UnlockfileEx. Then close the filehandle.
(Some of you may want to close the filehandle to kill the locks, it
doesn't work that way with win32, at least according to the msdn)

Below is a class called Flock, which gives you exclusive/shared
locking with non-blocking/blocking abilities. If you can think of any
optimizations or changes, be sure to let me know. 

CreateFile provides many options. It can be used for
files,directories,mailslots,sockets, etc. In this case, we're only
interested in standard files. 

The C++ call looks like this:

HANDLE CreateFile(
  LPCTSTR lpFileName,          
  DWORD dwDesiredAccess,       
  DWORD dwShareMode,           
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  DWORD dwCreationDisposition,  
  DWORD dwFlagsAndAttributes,   
  HANDLE hTemplateFile          
);
 
The python call is virtually the same with:

PyHANDLE = CreateFile( 
	fileName, 
	desiredAccess , 
	shareMode , 
	attributes , 
	creationDisposition , 
	flagsAndAttributes , 
	hTemplateFile 
	)

The module win32con in python is invaluable for setting most of these
attributes.  Besides win32con, you need win32security to create a
security attribute. 

@ex Here is a basic example of the raw program: |

import win32file
import win32con
import win32security
import win32api
import pywintypes

highbits=0xffff0000 #high-order 32 bits of byte range to lock
file="c:\\wilma.txt"
secur_att = win32security.SECURITY_ATTRIBUTES()
secur_att.Initialize()

hfile=win32file.CreateFile( file,\
                            win32con.GENERIC_READ|win32con.GENERIC_WRITE,\
                            win32con.FILE_SHARE_READ|win32con.FILE_SHARE_WRITE,\
                            secur_att,\  #default
                            win32con.OPEN_ALWAYS,\
                            win32con.FILE_ATTRIBUTE_NORMAL , 0 )

ov=pywintypes.OVERLAPPED() #used to indicate starting region to lock
win32file.LockFileEx(hfile,win32con.LOCKFILE_EXCLUSIVE_LOCK,0,highbits,ov)
win32api.Sleep(4000) #do something here
win32file.UnlockFileEx(hfile,0,highbits,ov)
hfile.Close()

<nl>Below, I have fleshed it out with a more useable Flock class.  The
code below works like this: You create an instance of the class,
providing a filename. It will create/access the file in a default way
and provide an hfile filehandle.  If you don't want the
default(shared/blocking), you can then specify in a dictionary what
type of locking you want.  Call the lock method on the file. Do
whatever you want with the hfile filehandle, then call the unlock
method which will remove the locks and close the filehandle.

Looking at the code below, for desiredAccess and shareMode, I have
both read and write on for most flexibility. The OPEN_ALWAYS means
that it will either use the current file or create a new one if none
is to be found. I use default security for the security attributes
option. The lock method basically determines what lock flags should be
used, depending on the type of locking you want and then calls
LockFileEx. An interesting option to LockFileEx is self.highbits.  You
can use that to specify portions of a file to lock instead of the
entire thing. When you're done with whatever you need to do, using the
hfile, filehandle, if necessary, then call the unlock method, to
remove the lock and close the filehandle.


<nl>Now for some code|


class Flock:
	def __init__(self,file):
		self.file=file
		self.type={'LOCK_EX':0,'LOCK_NB':0}
		secur_att = win32security.SECURITY_ATTRIBUTES()
		secur_att.Initialize()
		self.highbits=0xffff0000 #high-order 32 bits of byte range to lock
		#make a handel with read/write and open or create if doesn't exist
		self.hfile=win32file.CreateFile( self.file,\
					win32con.GENERIC_READ|win32con.GENERIC_WRITE,\
					win32con.FILE_SHARE_READ|win32con.FILE_SHARE_WRITE,\
					secur_att,\
					win32con.OPEN_ALWAYS,\
					win32con.FILE_ATTRIBUTE_NORMAL , 0 )
	def lock(self):
		if self.type['LOCK_EX']:  #exclusive locking
			if self.type['LOCK_NB']: #don't wait, non-blocking
				lock_flags=win32con.LOCKFILE_EXCLUSIVE_LOCK|win32con.LOCKFILE_FAIL_IMMEDIATELY
			else: #wait for lock to free
				lock_flags=win32con.LOCKFILE_EXCLUSIVE_LOCK
		else: #shared locking
			if self.type['LOCK_NB']: #don't wait, non-blocking
				lock_flags=win32con.LOCKFILE_FAIL_IMMEDIATELY 
			else:#shared lock wait for lock to free
				lock_flags=0 
		self.ov=pywintypes.OVERLAPPED() #used to indicate starting region to lock
		win32file.LockFileEx(self.hfile,lock_flags,0,self.highbits,self.ov)
	def unlock(self):
		win32file.UnlockFileEx(self.hfile,0,self.highbits,self.ov) #remove locks
		self.hfile.Close()

l=Flock("c:\\a3.txt")
l.type['LOCK_EX']=0
l.type['LOCK_NB']=0

print 'calling lock'
l.lock()
print 'now locked '

win32api.Sleep(1000)
l.unlock()
print 'now unlocked'

@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com       


@topic Recursive directory deletes and special files |Python's win32 access for file properties to enable deletes

<nl>Sometimes you may want to do something like remove entire
directory trees. Python has some great utilities to do that, except
files with special attributes cannot be typically deleted.

<nl>To get around this problem you need to use the win32 call to
SetFileAttributes to be a normal file.

The C++ call looks like this:

BOOL SetFileAttributes(  
  LPCTSTR lpFileName,     
  DWORD dwFileAttributes   
); 

You provide it 2 arguments the filename and the specific attributes
and it returns whether or not it succeeded.

The corresponding python call is:
int = win32api.SetFileAttributes( pathName, attrs )

The only question is where do you get attrs. It is included in the
ever handy win32con module specifically --
win32con.FILE_ATTRIBUTE_*. You can set a file to be read only,
archive, hidden, etc. We are concerned with setting it back to normal,
so we want: win32con.FILE_ATTRIBUTE_NORMAL

The example below can be useful, but, of course, be careful with it,
since it deletes a lot of stuff. It is a recursive function  The example
also makes use of some handy functions from the os module.


@ex Here is a basic example of how to remove a directory tree: |

import win32con
import win32api
import os

def del_dir(self,path):
	for file in os.listdir(path):
		file_or_dir = os.path.join(path,file)
		if os.path.isdir(file_or_dir) and not os.path.islink(file_or_dir):
			del_dir(file_or_dir) #it's a directory reucursive call to function again
		else:
			try:
				os.remove(file_or_dir) #it's a file, delete it
			except:
				#probably failed because it is not a normal file
				win32api.SetFileAttributes(file_or_dir, win32con.FILE_ATTRIBUTE_NORMAL)
				os.remove(file_or_dir) #it's a file, delete it
		os.rmdir(path) #delete the directory here

@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com       


*/








