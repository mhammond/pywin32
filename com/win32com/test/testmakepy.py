# Test makepy - try and run it over every OCX in the windows system directory.

import sys
import win32api
import traceback
import glob
import os
import string
import traceback

import win32com.test.util
from win32com.client import makepy, selecttlb
import pythoncom

def TestBuildAll(verbose = 1):
    tlbInfos = selecttlb.EnumTlbs()
    for info in tlbInfos:
        if verbose:
            print "%s (%s)" % (info.desc, info.dll)
        try:
            makepy.GenerateFromTypeLibSpec(info)
        except pythoncom.com_error, details:

            print "COM error on", info.desc
            print details
        except KeyboardInterrupt:
            print "Interrupted!"
            raise KeyboardInterrupt
        except:
            print "Failed:", info.desc
            traceback.print_exc()


def TestAll(verbose = 0):
    TestBuildAll(verbose)
    win32com.test.util.CheckClean()

if __name__=='__main__':
    TestAll(1)
