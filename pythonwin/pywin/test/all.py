#
# Tests Pythonwin / win32ui
#

import argparse
import os
import sys
import unittest

user_interaction = False

_indebugger = "pywin.debugger" in sys.modules
file_abs = os.path.abspath(__file__)
src_dir = os.path.dirname(file_abs)


if __name__ == "__main__":
    if _indebugger:
        pass

    p = argparse.ArgumentParser(
        description="Test runner for pywin32/Pythonwin", add_help=False
    )
    p.add_argument(
        "-user-interaction",
        "-i",
        action="store_true",
        help="Include tests which require user interaction",
    )
    args, remains = p.parse_known_args()
    user_interaction = args.user_interaction
    if user_interaction:
        print("-- running with user_interaction", file=sys.stderr)
    if "-h" in sys.argv or "--help" in sys.argv:
        p.print_help()

    argv = sys.argv[:1] + ["discover", "--start-directory", src_dir] + remains
    unittest.main(None, argv=argv)  # discover when no tests here
