Pythonwin IDLE directory
------------------------

This directory contains IDLE extensions used by
Pythonwin.  In ALL cases, the files in this directory that also appear
in the main IDLE directory should be indentical to the latest available
for IDLE.  

Eg, If you have Python 1.5.2 installed, the files in this
directory will be later than the IDLE versions.  If you use IDLE from
the CVS sources, then the files should be identical.

Pythonwin will look for IDLE extensions first in this directory, then on
the global sys.path.  Thus, if you have IDLE installed and run it from
the CVS sources, you may remove most of the extensions from this
directory, and the latest CVS version will then be used.
