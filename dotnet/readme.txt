Python for .NET - the Python .NET Compiler - May 2001.

*** This has only been tested with .NET build 9254

*** It should run on later builds, but this is far from guaranteed

*** For updates, visit http://www.activestate.com/ASPN/NET/

Complete documentation is in the HTML in the docs directory.

In a nutshell:

* There is a problem if this compiler is installed in a path with a space in
   the name.  For example, installing under "Program Files" will fail to 
   build.

* You must have Python and the Python COM extensions installed.  Either:

  - ActivePython - http://www.activestate.com/ASPN/Downloads/ActivePython
    This includes Python and the COM extensions.

  or

  - Any other Python 1.5.2 or later release, plus the Python COM
    extensions (build 132 or later):
    * http://www.python.org/download/ - Python versions
    * http://www.ActiveState.com/Products/ActivePython/win32all.html for the
      relevent Win32 extensions package.

* Run "nmake" from the root installation directory.  This builds the compiler and
  the runtime (including runtime builtins)

* Run "testall.bat" from the "p2c" directory.  This compiles and runs
  the test suite.  There will be lots of noise printed, but there should
  be no exceptions generated or printed.

* For your amusement, from the "p2c" directory, run the command:

  C:\...> make.bat HelloWorld

  This will invoke the compiler over the trivial HelloWorld.py sample, resulting
  in HelloWorld.exe

* See ManagedPython.html for a more detailed description of the compiler.
  
*** To Remove the compiler ***
To help keep your registry and disk clean, you can run "clean.bat" in the top-level
directory.  This simply runs "nmake clean" over the sub-directories.


All feedback/comments to MarkH@ActiveState.com

-- eof --

