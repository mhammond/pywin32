# Test makepy - try and run it over every OCX in the windows system directory.

import sys
import win32api
import traceback
import glob
import os
import traceback

import win32com.test.util
from win32com.client import makepy, selecttlb, gencache
import pythoncom
import winerror

def TestBuildAll(verbose = 1, bCreateEnums=False, bTypeHints=False):
    num = 0
    tlbInfos = selecttlb.EnumTlbs()
    for info in tlbInfos:
        if verbose:
            print("%s (%s)" % (info.desc, info.dll))
        try:
            makepy.GenerateFromTypeLibSpec(info, None, None, None, None, 0, 1, bCreateEnums, bTypeHints)
#          sys.stderr.write("Attr typeflags for coclass referenced object %s=%d (%d), typekind=%d\n" % (name, refAttr.wTypeFlags, refAttr.wTypeFlags & pythoncom.TYPEFLAG_FDUAL,refAttr.typekind))
            num += 1
        except pythoncom.com_error as details:
            # Ignore these 2 errors, as the are very common and can obscure
            # useful warnings.
            if details.hresult not in [winerror.TYPE_E_CANTLOADLIBRARY,
                              winerror.TYPE_E_LIBNOTREGISTERED]:
                print("** COM error on", info.desc)
                print(details)
        except KeyboardInterrupt:
            print("Interrupted!")
            raise KeyboardInterrupt
        except:
            print("Failed:", info.desc)
            traceback.print_exc()
        if makepy.bForDemandDefault:
            # This only builds enums etc by default - build each
            # interface manually
            tinfo = (info.clsid, info.lcid, info.major, info.minor)
            mod = gencache.EnsureModule(info.clsid, info.lcid, info.major, info.minor)
            for name in mod.NamesToIIDMap.keys():
                makepy.GenerateChildFromTypeLibSpec(name, tinfo)
    return num

def TestAll(verbose = 0):
    num = TestBuildAll(verbose)
    print("Generated and imported", num, "modules")

    if sys.version_info >= (3, 4):
        num = TestBuildAll(verbose, True)
        print("Generated and imported", num, "modules with Python-Enums")
        if sys.version_info >= (3, 7):
            num = TestBuildAll(verbose, False, True)
            print("Generated and imported", num, "modules with type-hints")
            num = TestBuildAll(verbose, True, True)
            print("Generated and imported", num, "modules with type-hints and Python-Enums")
        

    win32com.test.util.CheckClean()

if __name__=='__main__':
    TestAll("-q" not in sys.argv)
