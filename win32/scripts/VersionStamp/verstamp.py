#
# stamp a file with version information
#
# USAGE: python verstamp.py <major> <minor> <version> <fname> <desc> \
#                           [<debug>] [<is_dll>]
#
# For example:
#  C> python verstamp.py 1 4 103 pywintypes.dll "Common Python types for Win32"
#
#  This will store version "1.4.0.103" (ie, build 103 for Python 1.4)
#
#
# <debug> : 0 or 1 based on whether it's a debug build (DEFAULT == 0)
# <is_dll> : 0 or 1 to indicate the file type: DLL vs EXE (DEFAULT == 1)
#

from win32api import BeginUpdateResource, UpdateResource, EndUpdateResource, Unicode
import win32api
#print "Win32api is at", win32api.__file__
U = Unicode

import os
import sys
import struct
import string
import win32api
import pythoncom

VS_FFI_SIGNATURE = 0xFEEF04BD
VS_FFI_STRUCVERSION = 0x00010000
VS_FFI_FILEFLAGSMASK = 0x0000003f
VOS_NT_WINDOWS32 = 0x00040004

g_productname = 'Python'
g_company = ''
g_copyright = 'Copyright (C) A Few Assorted People 1995-1998.  All rights reserved.'
g_trademarks = ''

#
# Set VS_FF_PRERELEASE and DEBUG if Debug
#
def file_flags(debug):
  if debug:
    return 3	# VS_FF_DEBUG | VS_FF_PRERELEASE
  return 0

def file_type(is_dll):
  if is_dll:
    return 2	# VFT_DLL
  return 1	# VFT_APP

def VS_FIXEDFILEINFO(maj, min, sub, build, debug=0, is_dll=1):
  return struct.pack('lllllllllllll',
                     VS_FFI_SIGNATURE,	# dwSignature
                     VS_FFI_STRUCVERSION,	# dwStrucVersion
                     (maj << 16) | min,	# dwFileVersionMS
                     (sub << 16) | build,# dwFileVersionLS
                     (maj << 16) | min,	# dwProductVersionMS
                     (sub << 16) | build,		# dwProductVersionLS
                     VS_FFI_FILEFLAGSMASK,	# dwFileFlagsMask
                     file_flags(debug),	# dwFileFlags
                     VOS_NT_WINDOWS32,	# dwFileOS
                     file_type(is_dll),	# dwFileType
                     0x00000000,	# dwFileSubtype
                     0x00000000,	# dwFileDateMS
                     0x00000000,	# dwFileDateLS
                     )

def nullterm(s):
  try:
    return buffer(unicode(s)) + "\0\0"
  except NameError: # No unicode builtin
    return U(s).raw + '\0\0'

def pad32(s, extra=2):
  # extra is normally 2 to deal with wLength
  l = 4 - ((len(s) + extra) & 3)
  if l < 4:
    return s + ('\0' * l)
  return s

def addlen(s):
  return struct.pack('h', len(s) + 2) + s

def String(key, value):
  key = nullterm(key)
  value = nullterm(value)
  result = struct.pack('hh', len(value)/2, 1)	# wValueLength, wType
  result = result + key
  result = pad32(result) + value
  return addlen(result)

def StringTable(key, data):
  key = nullterm(key)
  result = struct.pack('hh', 0, 1)	# wValueLength, wType
  result = result + key
  for k, v in data.items():
    result = result + String(k, v)
    result = pad32(result)
  return addlen(result)

def StringFileInfo(data):
  result = struct.pack('hh', 0, 1)	# wValueLength, wType
  result = result + nullterm('StringFileInfo')
#  result = pad32(result) + StringTable('040904b0', data)
  result = pad32(result) + StringTable('040904E4', data)
  return addlen(result)

def Var(key, value):
  result = struct.pack('hh', len(value), 0)	# wValueLength, wType
  result = result + nullterm(key)
  result = pad32(result) + value
  return addlen(result)

def VarFileInfo(data):
  result = struct.pack('hh', 0, 1)	# wValueLength, wType
  result = result + nullterm('VarFileInfo')
  result = pad32(result)
  for k, v in data.items():
    result = result + Var(k, v)
  return addlen(result)

def VS_VERSION_INFO(maj, min, sub, build, sdata, vdata, debug=0, is_dll=1):
  ffi = VS_FIXEDFILEINFO(maj, min, sub, build, debug, is_dll)
  result = struct.pack('hh', len(ffi), 0)	# wValueLength, wType
  result = result + nullterm('VS_VERSION_INFO')
  result = pad32(result) + ffi
  result = pad32(result) + StringFileInfo(sdata) + VarFileInfo(vdata)
  return addlen(result)

def stamp(vars, pathname, desc, verbose=0, is_dll=1):
  # For some reason, the API functions report success if the file is open
  # but doesnt work!  Try and open the file for writing, just to see if it is
  # likely the stamp will work!
  try:
    f = open(pathname, "a+b")
    f.close()
  except IOError, why:
    print "WARNING: File %s could not be opened - %s" % (pathname, why)

  try:
    maj = int(vars.get("major"))
    min = int(vars.get("minor"))
    sub = int(vars.get("sub", 0))
    build = int(vars.get("build"))
  except (IndexError, TypeError):
    raise RuntimeError, "The version params must be integers"

  company = vars.get("company", g_company)
  copyright = vars.get("copyright", g_copyright)
  trademarks = vars.get("trademarks", g_trademarks)
  productname = vars.get("product", g_productname)


  vsn = '%s.%s.%s.%s' % (maj, min, sub, build)
  dir, fname = os.path.split(pathname)
  fname = string.upper(fname)
  sdata = {
    'Comments' : desc,
    'CompanyName' : company,
    'FileDescription' : desc,
    'FileVersion' : vsn,
    'InternalName' : fname,
    'LegalCopyright' : copyright,
    'LegalTrademarks' : trademarks,
    'OriginalFilename' : fname,
    'ProductName' : productname,
    'ProductVersion' : vsn,
    }
  vdata = {
    'Translation' : struct.pack('hh', 0x409, 1200),
    }
  vs = VS_VERSION_INFO(maj, min, sub, build, sdata, vdata)

  h = BeginUpdateResource(pathname, 0)
  UpdateResource(h, 16, 1, vs)
  EndUpdateResource(h, 0)

  if verbose:
    print "Stamped:", pathname

if __name__ == '__main__':
  if len(sys.argv) < 6:
    print "ERROR: incorrect invocation. See comments in header of script."
    sys.exit(1)

  maj = string.atoi(sys.argv[1])
  min = string.atoi(sys.argv[2])
  ver = string.atoi(sys.argv[3])

  verbose = 0
  is_dll = 1
  if len(sys.argv) > 6:
    debug = string.atoi(sys.argv[6])
    if len(sys.argv) > 7:
      is_dll = string.atoi(sys.argv[7])

  v={'major':maj,'minor':min,'build':ver}
  stamp(v, sys.argv[4], sys.argv[5], verbose, is_dll)
