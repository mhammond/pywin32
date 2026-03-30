Pythonwin IDLE directory
------------------------

This directory contains IDLE extensions used by
Pythonwin.  The files in this directory that also appear in the main IDLE
directory are intended be identical to the latest available for IDLE.

If you use IDLE from the CVS sources, then the files should be
identical.  If you have a Python version installed that is more recent
than when this release was made, then you may notice differences.

Pythonwin will look for IDLE extensions first in this directory, then on
the global sys.path.  Thus, if you have IDLE installed and run it from
the CVS sources, you may remove most of the extensions from this
directory, and the latest CVS version will then be used.
