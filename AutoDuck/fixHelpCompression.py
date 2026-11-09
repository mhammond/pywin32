# fixHelpCompression.py
# Add a compression option to the generated help project file.
import os
import sys

import win32api

fname = sys.argv[1]

if not os.path.exists(fname):
    sys.stderr.write(f"The project file '{fname}' was not found\n")
    sys.exit(1)

win32api.WriteProfileVal("options", "COMPRESS", "12 Hall Zeck", fname)
