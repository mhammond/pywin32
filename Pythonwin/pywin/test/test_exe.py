#
# Tests launching Pythonwin.exe and running a few tests inside
# (Most tests are done in Python started app in test_pywin.py etc.)
#

import os
import subprocess
import sys
import tempfile
import unittest

import win32ui

user_interaction = False

_indebugger = "pywin.debugger" in sys.modules
file_abs = os.path.abspath(__file__)
src_dir = os.path.dirname(file_abs)
pythonwinexe_path = os.path.dirname(win32ui.__file__) + "\\Pythonwin.exe"


class TestPythonwinExe(unittest.TestCase):
    """Starts up Pythonwin.exe and runs exetestscript.py inside for a few tests"""

    def setUp(self):
        import site

        fh, self.tfn = tempfile.mkstemp(suffix=".testout.txt", prefix="pywintest-")
        os.close(fh)
        scriptpath = src_dir + "\\_exetestscript.py"
        cmd = [pythonwinexe_path, "/new", "/run", scriptpath, self.tfn]
        ##wd = src_dir
        ##wd = os.path.dirname(pythonwinexe_path)
        wd = os.path.dirname(sys.executable)
        usersite = site.getusersitepackages()
        if usersite in pythonwinexe_path and sys.exec_prefix not in pythonwinexe_path:
            # Workaround for Pythonwin.exe to find PythonNN.dll from user
            # install w symlink. This works only when cwd is set to the dir of
            # python.exe / exec_prefix.

            # XXX Pythonwin.exe / win32uihostglue.h could be improved to search
            # the Python DLL itself via registry when local / relative search fails.

            pydll = "Python{}{}.dll".format(*sys.version_info[:2])  # same for 32bit
            src = os.path.dirname(sys.executable) + os.sep + pydll
            dst = os.path.dirname(pythonwinexe_path) + os.sep + pydll
            if not os.path.isfile(dst):
                try:
                    assert os.path.isfile(src)
                    print(f"-- symlink {dst!r} -> {src!r}", file=sys.stderr)
                    os.symlink(src, dst)
                except (OSError, AssertionError) as e:
                    print(f"-- cannot make symlink {dst!r}: {e!r}", file=sys.stderr)
        print(f"-- Starting: {cmd!r} in {wd!r}", file=sys.stderr)
        self.p = subprocess.Popen(cmd, cwd=wd)

    def test_exe(self):
        print("-- Waiting --", file=sys.stderr)
        try:
            rc = self.p.wait(20)
        except subprocess.TimeoutExpired:
            rc = "TIMEOUT"
        with open(self.tfn) as f:
            outs = f.read()
        assert rc == 0, f"rc is {rc!r}, outs={outs!r}"
        assert "Success!" in outs, outs
        print("-- test_exe Ok! --", file=sys.stderr)

    def tearDown(self):
        os.remove(self.tfn)
        print("-- removed '%s' --" % self.tfn, file=sys.stderr)


if __name__ == "__main__":
    if _indebugger:
        t = TestPythonwinExe("test_exe")
        t.debug()
        sys.exit()
    unittest.main()
