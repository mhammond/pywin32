# fixHelpCompression.py
# Add a compression option to the generated help project file.
import os
import sys

import win32api

fname = sys.argv[1]

try:
    os.stat(fname)
except OSError:
    sys.stderr.write("The project file '{}' was not found\n".format(fname))
    sys.exit(1)

win32api.WriteProfileVal("options", "COMPRESS", "12 Hall Zeck", fname)
