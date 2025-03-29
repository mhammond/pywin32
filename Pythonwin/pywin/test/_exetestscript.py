#
# small test script run from test_exe.TestPythonExe via separate Pythonwin.exe process
#

import os
import sys
import time
import traceback

import win32con
import win32ui
from pywin.framework import scriptutils

try:
    fnout  # (may be repeated & changed inside Pythonwin debugger)
except NameError:
    fnout = sys.argv[-1]
assert fnout.endswith(".testout.txt")

mf = None
with open(fnout, "w") as out:
    try:
        _clock = time.perf_counter
        print("Start!", file=out)
        mf = win32ui.GetMainFrame()
        file_abs = os.path.abspath(__file__)
        src_dir = os.path.dirname(file_abs)

        # open a source file
        some_fn = src_dir + "\\_dbgscript.py"
        assert some_fn != __file__
        scriptutils.JumpToDocument(some_fn)
        win32ui.PumpWaitingMessages(0, -1)
        assert some_fn == scriptutils.GetActiveFileName()

        # open my own source file and check the text content
        scriptutils.JumpToDocument(__file__)
        win32ui.PumpWaitingMessages(0, -1)
        v = scriptutils.GetActiveEditControl()
        assert file_abs == v.GetDocument().GetPathName()
        t = v.GetTextRange()
        assert "t = v.GetTextRange()" in t
        print("Success!")
        print("Success!", file=out)

        t0 = _clock()
        while _clock() - t0 < 0.05:
            win32ui.PumpWaitingMessages(0, -1)
            time.sleep(0.01)
    except Exception:
        traceback.print_exc(file=out)
        raise
    finally:
        if mf:
            mf.PostMessage(win32con.WM_CLOSE)
