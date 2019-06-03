"""is64bit.Python() --> boolean value of detected Python word size. is64bit.os() --> os build version"""
import sys


def Python():
    if sys.platform == 'cli':  # IronPython
        import System
        return System.IntPtr.Size == 8
    else:
        try:
            return sys.maxsize > 2147483647
        except AttributeError:
            return sys.maxint > 2147483647


def os():
    import platform
    pm = platform.machine()
    if pm != '..' and pm.endswith('64'):  # recent Python (not Iron)
        return True
    else:
        import os
        if 'PROCESSOR_ARCHITEW6432' in os.environ:
            return True  # 32 bit program running on 64 bit Windows
        try:
            # 64 bit Windows 64 bit program
            return os.environ['PROCESSOR_ARCHITECTURE'].endswith('64')
        except IndexError:
            pass  # not Windows
        try:
            # this often works in Linux
            return '64' in platform.architecture()[0]
        except:
            # is an older version of Python, assume also an older os (best we can guess)
            return False


if __name__ == "__main__":
    print("is64bit.Python() =", Python(), "is64bit.os() =", os())
