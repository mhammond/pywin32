SWIG for Win32 Extensions Readme
--------------------------------

This is a release of SWIG 1.1 with slight customizations for Windows.
This release of SWIG should be used only for building the various Win32
and COM extensions for Windows.  For the official releases of SWIG and
more information about SWIG, please visit www.swig.org

To use this:
-----------
You need to set 2 environment variables for this to work.

SWIG_LIB: Required by SWIG, and must be set to point to the "swig_lib"
sub-directory in this archive.

SWIG_EXE: Required by the Win32 extension makefiles, and should be the
fully qualified path to the .exe in this archive.  Note that as this
environment variable is used by MSVC, you probably need to set this
variable globally (ie, either in autoexec.bat, or Control Panel->System-
>Environment.

