# Eventually setup_win32all.py will be renamed to setup.py
# When we do that, we will put a warning + the execfile in setup_win32all.py
import sys, os
mydir=os.path.dirname(sys.argv[0])
execfile(os.path.join(mydir, "setup_win32all.py"))
