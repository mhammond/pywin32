# This is a Python 3.x script to build pywin32.  It converts then executes
# the regular setup.py script.
import os
from lib2to3.refactor import RefactoringTool, get_fixers_from_package

fixers = ['lib2to3.fixes.fix_print', 'lib2to3.fixes.fix_except']
options = dict(doctests_only=False, fix=[], list_fixes=[], 
               print_function=False, verbose=False,
               write=True)
r = RefactoringTool(fixers, options)
script = os.path.join(os.path.dirname(__file__), "setup.py")
data = open(script).read()
print("Converting...")
got = r.refactor_string(data, "setup.py")
print("Executing...")
exec(str(got))

