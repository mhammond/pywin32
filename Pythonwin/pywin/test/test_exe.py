#
# Tests launching Pythonwin.exe and running a few tests inside
# (Most tests are done in Python started app in test_pywin.py etc.)
#

import os
import site
import subprocess
import sys
import tempfile
import unittest

import win32ui
from pywin32_testutil import TestSkipped

user_interaction = False

_indebugger = "pywin.debugger" in sys.modules
file_abs = os.path.abspath(__file__)
src_dir = os.path.dirname(file_abs)
pythonwinexe_path = os.path.dirname(win32ui.__file__) + "\\Pythonwin.exe"


class TestPythonwinExe(unittest.TestCase):
    """Starts up Pythonwin.exe and runs exetestscript.py inside for a few tests"""

    def setUp(self):
        if sys.flags.dev_mode:
            raise TestSkipped(
                "This test currently fails in development mode for unknown reasons"
            )

        fh, self.tfn = tempfile.mkstemp(suffix=".testout.txt", prefix="pywintest-")
        os.close(fh)
        usersite = site.getusersitepackages()
        if usersite in pythonwinexe_path and sys.exec_prefix not in pythonwinexe_path:
            # Workaround for Pythonwin.exe to find PythonNN.dll from user
            # install w symlink. This works only when cwd is set to the dir of
            # python.exe / exec_prefix.

            # XXX Pythonwin.exe / win32uihostglue.h could be improved to search
            # the Python DLL itself via registry when local / relative search fails.

            pydll = f"Python{sys.version_info.major}{sys.version_info.minor}.dll"  # same for 32bit
            src = os.path.dirname(sys.executable) + os.sep + pydll
            dst = os.path.dirname(pythonwinexe_path) + os.sep + pydll
            if not os.path.isfile(dst):
                try:
                    self.assertTrue(os.path.isfile(src))
                    print(f"-- symlink {dst!r} -> {src!r}", file=sys.stderr)
                    os.symlink(src, dst)
                except (OSError, AssertionError) as e:
                    print(f"-- cannot make symlink {dst!r}: {e!r}", file=sys.stderr)

    def test_exe(self):
        scriptpath = src_dir + "\\_exetestscript.py"
        cmd = [pythonwinexe_path, "/new", "/run", scriptpath, self.tfn]
        wd = os.path.dirname(sys.executable)

        print(f"-- Starting: '{' '.join(cmd)}' in '{wd}'", file=sys.stderr)
        try:
            rc = subprocess.run(cmd, cwd=wd, timeout=20).returncode
        except subprocess.TimeoutExpired:
            rc = "TIMEOUT"
        with open(self.tfn) as f:
            outs = f.read()
        self.assertEqual(rc, 0, f"outs={outs!r}")
        self.assertIn("Success!", outs)
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
