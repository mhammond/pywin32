#
# test the universal gateway stuff
#

import struct
import univgw
import pythoncom
from win32com.server import util
import traceback
import winerror
import time

class Definition:
  "Completely defines a COM interface."

  def iid(self):
    "Return the IID that we are defining."
    return self._iid

  def vtbl_argsizes(self):
    "Return the size of the arguments to each interface method."
    #def argsize(method, calcsize=struct.calcsize):
    #  return calcsize(method[1])
    def argsize(method):
      return method[2]
    return map(argsize, self._methods)

  def dispatch(self, ob, index, argPtr,
               ReadMemory=univgw.ReadMemory, unpack=struct.unpack):
    "Dispatch a call to an interface method."

    ### WARNING: the Alpha will probably need special work to deal with
    ### the "argPtr" variable. most platforms, though, can simply read the
    ### arguments off of the stack using the stack pointer.

    ### just print some crap for now

    name, fmt, argsize = self._methods[index]
    s = ReadMemory(argPtr, argsize)
    args = unpack(fmt, s)

    #print 'called:', ob, self, index, argPtr
    #print '   iid:', self._iid
    #print '   fmt:', fmt
    #print '  size:', argsize
    #print '  args:', args

    return getattr(self, name)(ob, args)

  if 1:
    _dispatch = dispatch
    def dispatch(self, ob, index, argPtr):
      try:
        return self._dispatch(ob, index, argPtr)
      except:
        traceback.print_exc()
        raise

class IStream(Definition):
  _iid = pythoncom.IID_IStream
  _methods = [
    ('Read', 'PLP', 12),
    ('Write', 'PLP', 12),
    ('Seek', '8sLP', 16),	# LARGE_INTEGER, DWORD, ULARGE_INTEGER*
    ('SetSize', '8s', 8),
    ('CopyTo', 'P8sPP', 20),
    ('Commit', 'L', 4),
    ('Revert', '', 0),
    ('LockRegion', '8s8sL', 20),
    ('UnlockRegion', '8s8sL', 20),
    ('Stat', 'PL', 8),
    ('Clone', 'P', 4),
    ]

  def Read(self, ob, (pv, cb, pcbRead),
           WriteMemory=univgw.WriteMemory, pack=struct.pack):
    if not pv:
      return winerror.E_POINTER
    s = ob.Read(cb)
    WriteMemory(pv, s)
    if pcbRead:
      ### get rid of this pack for more speed!
      WriteMemory(pcbRead, pack('L', len(s)))

  def Write(self, ob, (pv, cb, pcbWritten),
            WriteMemory=univgw.WriteMemory, ReadMemory=univgw.ReadMemory,
            pack=struct.pack):
    if not pv:
      return winerror.E_POINTER
    s = ReadMemory(pv, cb)
    l = ob.Write(s)
    if pcbWritten:
      WriteMemory(pcbWritten, pack('L', l))

  def Seek(self, ob, (dlibMove, dwOrigin, plibNewPosition),
           L64=univgw.L64, strUL64=univgw.strUL64, WriteMemory=univgw.WriteMemory):
    pos = ob.Seek(L64(dlibMove), dwOrigin)
    if plibNewPosition:
      WriteMemory(plibNewPosition, strUL64(pos))

  def SetSize(self, ob, (libNewSize,), UL64=univgw.UL64):
    ob.SetSize(UL64(libNewSize))

  def CopyTo(self, ob, (pstm, cb, pcbRead, pcbWritten),
             WriteMemory=univgw.WriteMemory, interface=univgw.interface,
             IID_IStream=pythoncom.IID_IStream,
             UL64=univgw.UL64, strUL64=univgw.strUL64):
    pstm = interface(pstm, IID_IStream)
    r, w = ob.CopyTo(pstm, UL64(cb))
    if pcbRead:
      WriteMemory(pcbRead, strUL64(r))
    if pcbWritten:
      WriteMemory(pcbWritten, strUL64(w))

  def Commit(self, ob, (grfCommitFlags,)):
    ob.Commit(grfCommitFlags)

  def Revert(self, ob, args):
    ob.Revert()

  def LockRegion(self, ob, (libOffset, cb, dwLockType), UL64=univgw.UL64):
    ob.LockRegion(UL64(libOffset), UL64(cb), dwLockType)

  def UnlockRegion(self, ob, (libOffset, cb, dwLockType), UL64=univgw.UL64):
    ob.UnlockRegion(UL64(libOffset), UL64(cb), dwLockType)

  def Stat(self, ob, (pstatstg, grfStatFlag)):
    print 'IStream::Stat:', ob, pstatstg, grfStatFlag

  def Clone(self, ob, (ppstm,)):
    print 'IStream::Clone:', ob, ppstm

defn = IStream()
#print defn

# create a vtable for the IStream interface
vtbl = univgw.CreateVTable(defn)
#print vtbl

# the Python object class that we're going to expose
class MyStream:
  ### only for the timing tests
  _public_methods_ = ['Write','SetSize','Read', 'Seek', 'Commit', 'Revert', 'LockRegion','CopyTo']
  _com_interfaces_ = [pythoncom.IID_IStream]

  def Read(self, cb):
    return 'this is a very long string that we want to read'[:int(cb)]
  def Write(self, s):
    #print 'MyStream.Write:', `s`
    return len(s)
  def Seek(self, move, origin):
    #print 'MyStream.Seek:', move, origin
    return move
  def SetSize(self, size):
    #print 'MyStream.SetSize:', size
    pass
  def CopyTo(self, pstm, cb):
    #print 'MyStream.CopyTo:', `pstm, cb`
    return cb, cb
  def Commit(self, flags):
    #print 'MyStream.Commit:', flags
    pass
  def Revert(self):
    #print 'MyStream.Revert'
    pass
  def LockRegion(self, offset, cb, type):
    #print 'MyStream.LockRegion:', offset, cb, type
    pass
  def UnlockRegion(self, offset, cb, type):
    print 'MyStream.UnlockRegion:', offset, cb, type
  def Stat(self, *args):	### WHATEVER
    print 'MyStream.Stat:', args
  def Clone(self, *args):	### WHATEVER
    print 'MyStream.Clone:', args

# create the stream, then wrap it with a pair of thunks. the COM2Py thunk
# that we build here becomes the "identity" interface for the stream object.
wrapped = pythoncom.WrapObject(MyStream())
#print wrapped

# create a tear-off IStream gateway for the policy object
ob = univgw.CreateTearOff(wrapped, vtbl, pythoncom.IID_IStream)
#print ob

ob2 = util.wrap(MyStream())
ob2 = ob2.QueryInterface(pythoncom.IID_IStream)
#print ob2

# exercise it
if 0:
  print 'read:', `ob.Read(10)`
  print 'read:', `ob.Read(5)`
  cb = ob.Write('your mom was here')
  print 'wrote:', cb
  newpos = ob.Seek(10, 20)
  print 'newpos:', newpos
  ob.SetSize(5678)
  cb = ob.CopyTo(ob2, 10)
  print 'copied:', cb
  ob.Commit(1234)
  ob.Revert()
  ob.LockRegion(1,2,3)
  ob.UnlockRegion(1,2,3)
  #ob.Stat(...
  #ob.Clone(...

def f1():
  x1, x2 = struct.unpack('LL','\0\0\0\0\0\0\0\0')
  x = x1 | (x2 << 32L)
def f2(T=univgw.L64):
  x = T(struct.unpack('8s','\0\0\0\0\0\0\0\0')[0])

if 1:
  def time_it(f, a):
    t = time.time()
    for i in xrange(50000):
      apply(f, a)
    return time.time() - t
  def time_pair(name, *a):
    t1 = time_it(getattr(ob, name), a)
    t2 = time_it(getattr(ob2, name), a)
    print '%-10s - univgw: %.3f' % (name, t1)
    print '%-10s -  pycom: %.3f' % (name, t2)

  print 'Timing...'

  if 0:
    print time_it(f1,())
    print time_it(f2,())

  if 1:
    #time_pair('Read', 10)
    #time_pair('Write', 'your mom was here')
    #time_pair('Seek', 10, 20)
    #time_pair('SetSize', 1000)
    time_pair('CopyTo', ob2, 1000)
    #time_pair('Commit', 0)
    #time_pair('Revert')
    #time_pair('LockRegion', 10, 20, 30)

import win32api
win32api.OutputDebugString("step 1\n")
ob2 = ob
del ob2
win32api.OutputDebugString("step 2\n")

ob = None

win32api.OutputDebugString("step 3\n")

# in case we're inside Dev Studio, this will pause so you can see
# the output of the script
#print "Press return:",
#raw_input()
