build_id="223.1" # may optionally include a ".{patchno}" suffix.
# Putting buildno at the top prevents automatic __doc__ assignment, and
# I *want* the build number at the top :)
__doc__="""This is a distutils setup-script for the pywin32 extensions

To build the pywin32 extensions, simply execute:
  python setup.py -q build
or
  python setup.py -q install
to build and install into your current Python installation.

These extensions require a number of libraries to build, some of which may
require you to install special SDKs or toolkits.  This script will attempt
to build as many as it can, and at the end of the build will report any 
extension modules that could not be built and why.

This has got complicated due to the various different versions of
Visual Studio used - some VS versions are not compatible with some SDK
versions.  Below are the Windows SDK versions required (and the URL - although
these are subject to being changed by MS at any time:)

Python 2.4->2.5:
  Microsoft Windows Software Development Kit Update for Windows Vista (version 6.0)
  http://www.microsoft.com/downloads/en/details.aspx?FamilyID=4377f86d-c913-4b5c-b87e-ef72e5b4e065

Python 2.6+:
  Microsoft Windows SDK for Windows 7 and .NET Framework 4 (version 7.1)
  http://www.microsoft.com/downloads/en/details.aspx?FamilyID=6b6c21d2-2006-4afa-9702-529fa782d63b

If you multiple SDK versions on a single machine, set the MSSDK environment
variable to point at the one you want to use.  Note that using the SDK for
a particular platform (eg, Windows 7) doesn't force you to use that OS as your
build environment.  If the links above don't work, use google to find them.

Building:
---------

To install the pywin32 extensions, execute:
  python setup.py -q install

This will install the built extensions into your site-packages directory,
create an appropriate .pth file, and should leave everything ready to use.
There is no need to modify the registry.

To build or install debug (_d) versions of these extensions, ensure you have
built or installed a debug version of Python itself, then pass the "--debug"
flag to the build command - eg:
  python setup.py -q build --debug
or to build and install a debug version:
  python setup.py -q build --debug install

To build 64bit versions of this:

* py2.5 and earlier - sorry, I've given up in disgust.  Using VS2003 with
  the Vista SDK is just too painful to make work, and VS2005 is not used for
  any released versions of Python. See revision 1.69 of this file for the
  last version that attempted to support and document this process.

*  2.6 and later: On a 64bit OS, just build as you would on a 32bit platform.
   On a 32bit platform (ie, to cross-compile), you must use VS2008 to
   cross-compile Python itself. Note that by default, the 64bit tools are not
   installed with VS2008, so you may need to adjust your VS2008 setup. Then
   use:

      setup.py build --plat-name=win-amd64

   see the distutils cross-compilation documentation for more details.

Creating Distributions:
-----------------------

The make_all.bat batch file will build and create distributions.

Once a distribution has been built and tested, you should ensure that
'git status' shows no dirty files, then create a tag with the format 'bXXX'

The executable installers are uploaded to github.

The "wheel" packages are uploaded to pypi using `twine upload dist/path-to.whl`

"""

import os
import sys
import re
from tempfile import gettempdir

from setuptools import setup
from distutils import log
from distutils.filelist import FileList

if sys.platform == "win32":
    from setup_win import cmdclass, ext_modules, packages, py_modules
else:
    cmdclass = {}
    ext_modules = []
    packages = []
    py_modules = FileList()

# NOTE: somewhat counter-intuitively, a result list a-la:
#  [('Lib/site-packages\\pythonwin', ('pythonwin/license.txt',)),]
# will 'do the right thing' in terms of installing licence.txt into
# 'Lib/site-packages/pythonwin/licence.txt'.  We exploit this to
# get 'com/win32com/whatever' installed to 'win32com/whatever'
def convert_data_files(files):
    ret = []
    for file in files:
        file = os.path.normpath(file)
        if file.find("*") >= 0:
            flist = FileList()
            flist.findall(os.path.dirname(file))
            flist.include_pattern(os.path.basename(file), anchor=0)
            # We never want CVS
            flist.exclude_pattern(re.compile(".*\\\\CVS\\\\"), is_regex=1, anchor=0)
            flist.exclude_pattern("*.pyc", anchor=0)
            flist.exclude_pattern("*.pyo", anchor=0)
            # files with a leading dot upset bdist_msi, and '.*' doesn't
            # work - it matches from the start of the string and we have
            # dir names.  So any '\.' gets the boot.
            flist.exclude_pattern(re.compile(".*\\\\\."), is_regex=1, anchor=0)
            if not flist.files:
                raise RuntimeError("No files match '%s'" % file)
            files_use = flist.files
        else:
            if not os.path.isfile(file):
                raise RuntimeError("No file '%s'" % file)
            files_use = (file,)
        for fname in files_use:
            path_use = os.path.dirname(fname)
            if path_use.startswith("com/") or path_use.startswith("com\\"):
                path_use = path_use[4:]
            ret.append( (path_use, (fname,)))
    return ret

def convert_optional_data_files(files):
    ret = []
    for file in files:
        try:
            temp = convert_data_files([file])
        except RuntimeError, details:
            if not str(details.args[0]).startswith("No file"):
                raise
            log.info('NOTE: Optional file %s not found - skipping' % file)
        else:
            ret.append(temp[0])
    return ret

dist = setup(name="pywin32",
      version=str(build_id),
      description="Python for Window Extensions",
      long_description="Python extensions for Microsoft Windows\n"
                       "Provides access to much of the Win32 API, the\n"
                       "ability to create and use COM objects, and the\n"
                       "Pythonwin environment.",
      author="Mark Hammond (et al)",
      author_email = "mhammond@skippinet.com.au",
      url="https://github.com/mhammond/pywin32",
      license="PSF",
      cmdclass = cmdclass,
      options = {"bdist_wininst":
                    {"install_script": "pywin32_postinstall.py",
                     "title": "pywin32-%s" % (build_id,),
                     "user_access_control": "auto",
                    },
                 "bdist_msi":
                    {"install_script": "pywin32_postinstall.py",
                    },
                },

      scripts = ["pywin32_postinstall.py", "pywin32_testall.py"],

      ext_modules = ext_modules,

      package_dir = {"win32com": "com/win32com",
                     "win32comext": "com/win32comext",
                     "pythonwin": "pythonwin",},
      packages = packages,
      py_modules = py_modules,

      data_files=[('', (os.path.join(gettempdir(),'pywin32.version.txt'),))] + 
        convert_optional_data_files([
                'PyWin32.chm',
                ]) + 
        convert_data_files([
                'pythonwin/pywin/*.cfg',
                'pythonwin/pywin/Demos/*.py',
                'pythonwin/pywin/Demos/app/*.py',
                'pythonwin/pywin/Demos/ocx/*.py',
                'pythonwin/license.txt',
                'win32/license.txt',
                'win32/scripts/*.py',
                'win32/test/*.py',
                'win32/test/win32rcparser/test.rc',
                'win32/test/win32rcparser/test.h',
                'win32/test/win32rcparser/python.ico',
                'win32/test/win32rcparser/python.bmp',
                'win32/Demos/*.py',
                'win32/Demos/images/*.bmp',
                'com/win32com/readme.htm',
                # win32com test utility files.
                'com/win32com/test/*.idl',
                'com/win32com/test/*.js',
                'com/win32com/test/*.sct',
                'com/win32com/test/*.txt',
                'com/win32com/test/*.vbs',
                'com/win32com/test/*.xsl',
                # win32com docs
                'com/win32com/HTML/*.html',
                'com/win32com/HTML/image/*.gif',
                'com/win32comext/adsi/demos/*.py',
                # Active Scripting test and demos.
                'com/win32comext/axscript/test/*.html',
                'com/win32comext/axscript/test/*.py',
                'com/win32comext/axscript/test/*.pys',
                'com/win32comext/axscript/test/*.vbs',
                'com/win32comext/axscript/Demos/*.pys',
                'com/win32comext/axscript/Demos/*.htm*',
                'com/win32comext/axscript/Demos/*.gif',
                'com/win32comext/axscript/Demos/*.asp',
                'com/win32comext/mapi/demos/*.py',
                'com/win32comext/propsys/test/*.py',
                'com/win32comext/shell/test/*.py',
                'com/win32comext/shell/demos/servers/*.py',
                'com/win32comext/shell/demos/*.py',
                'com/win32comext/taskscheduler/test/*.py',
                'com/win32comext/ifilter/demo/*.py',
                'com/win32comext/authorization/demos/*.py',
                'com/win32comext/bits/test/*.py',
                'isapi/*.txt',
                'isapi/samples/*.py',
                'isapi/samples/*.txt',
                'isapi/doc/*.html',
                'isapi/test/*.py',
                'isapi/test/*.txt',
                'adodbapi/*.txt',
                'adodbapi/test/*.py',
                'adodbapi/examples/*.py'
        ]) +
                # The headers and .lib files
                [
                    ('win32/include',    ('win32/src/PyWinTypes.h',)),
                    ('win32com/include', ('com/win32com/src/include/PythonCOM.h',
                                         'com/win32com/src/include/PythonCOMRegister.h',
                                         'com/win32com/src/include/PythonCOMServer.h'))
                ] +
                # And data files convert_data_files can't handle.
                [
                    ('win32com', ('com/License.txt',)),
                    # pythoncom.py doesn't quite fit anywhere else.
                    # Note we don't get an auto .pyc - but who cares?
                    ('', ('com/pythoncom.py',)),
                    ('', ('pywin32.pth',)),
                ],
      )

# If we did any extension building, and report if we skipped any.
if 'build_ext' in dist.command_obj:
    what_string = "built"
    if 'install' in dist.command_obj: # just to be purdy
        what_string += "/installed"
    # Print the list of extension modules we skipped building.
    if 'build_ext' in dist.command_obj:
        excluded_extensions = dist.command_obj['build_ext'].excluded_extensions
        if excluded_extensions:
            print "*** NOTE: The following extensions were NOT %s:" % what_string
            for ext, why in excluded_extensions:
                print " %s: %s" % (ext.name, why)
            print "For more details on installing the correct libraries and headers,"
            print "please execute this script with no arguments (or see the docstring)"
        else:
            print "All extension modules %s OK" % (what_string,)