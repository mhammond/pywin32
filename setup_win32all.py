# The build script used to be called setup_win32all.py
# We will remove this in a few versions, but give people the hint
print "Please use 'setup.py' instead of 'setup_win32all.py'"
import sys, os
mydir=os.path.dirname(sys.argv[0])
execfile(os.path.join(mydir, "setup_win32all.py"))
