# fixHelpCompression.py
# Add a compression option to the generated help project file.
import sys, os, win32api

fname = sys.argv[1]

try:
	os.stat(fname)
except os.error:
	sys.stderr.write("The project file '%s' was not found\n" % (fname))
	sys.exit(1)
	
win32api.WriteProfileVal("options","COMPRESS","12 Hall Zeck", fname)



