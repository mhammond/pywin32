COM Test Suite Readme
---------------------

This is not for the feint hearted!

Running the test suite:
-----------------------
* Open a command prompt
* Change to the "win32com\test" directory.
* run "testall.py".  This will perform level 1 testing.
  You may specify 1, 2, or 3 on the command line ("testutil 3")
  to execute more tests.

Requirements:
-------------

There are various requirements for running the test suite.  If you
do not meet some of the requirements, and the test suite fails in
a horrible way, then please feel free to patch the test suite so it
fails "elegantly" :-)

Requirements (put together by Greg recently, and he may hage missed a few :-)
* Python.Interpreter and Python.dictionary be registered. 
  (in win32com\servers directory)

* The COM test Suite be built.  This consists of:
  - Building the C++ Test Project
  - Building the VB DLL, and using "regsvr32" on it to register it.

* Registry be correctly set so that "import win32com.axscript.axscript" 
  suceeds. If you built from sources, this may involve setting the
  "BuildPath" subkey under PythonPath\win32com

* Windows Scripting Host and Scriptlets be installed. 
  See http://msdn.microsoft.com/scripting

* Microsoft Office and Microsoft Exchange be installed
  for "level 2" testing.

