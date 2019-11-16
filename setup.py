from __future__ import print_function

build_id="227.1" # may optionally include a ".{patchno}" suffix.
# Putting build_id at the top prevents automatic __doc__ assignment, and
# I *want* the build number at the top :)
__doc__="""This is a distutils setup-script for the pywin32 extensions

To build the pywin32 extensions, simply execute:
  python setup.py -q build
or
  python setup.py -q install
to build and install into your current Python installation.

Note that Python 2.7 is the only version of Python 2.x which is supported, and
that Python 3.5 is the earliest Python 3.x supported.

These extensions require a number of libraries to build, some of which may
require you to install special SDKs or toolkits.  This script will attempt
to build as many as it can, and at the end of the build will report any
extension modules that could not be built and why.

At a minimum, you must have the same version of Microsoft Visual C++ that is
used for the Python version you are building, plus the Windows 8.1 SDK. Note
that the Windows 8.1 SDK is often bundled with Microsoft Visual C++.

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
   On a 64bit OS, just build as you would on a 32bit platform.

   On a 32bit platform it may be possible to cross-compile, but this hasn't
   been tested in many years. Regardless, this ability comes directly from
   distutils/setuptools, so see their documentation for more details.

Creating Distributions:
-----------------------

The make_all.bat batch file will build and create distributions.

Once a distribution has been built and tested, you should ensure that
'git status' shows no dirty files, then create a tag with the format 'bXXX'

The executable installers are uploaded to github.

The "wheel" packages are uploaded to pypi using `twine upload dist/path-to.whl`

"""
# Originally by Thomas Heller, started in 2000 or so.
import os
import string
import sys
import glob
import re
from tempfile import gettempdir
import platform
import shutil
import subprocess

is_py3k = sys.version_info > (3,) # get this out of the way early on...

try:
    import winreg # py3k
except ImportError:
    import _winreg as winreg # py2k

# The rest of our imports.
from setuptools import setup
from distutils.core import Extension
from setuptools.command.install import install
from setuptools.command.install_lib import install_lib
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py
from distutils.command.build import build
from distutils.command.install_data import install_data
from distutils.command.build_scripts import build_scripts

try:
    from distutils.command.bdist_msi import bdist_msi
except ImportError:
    # py24 and earlier
    bdist_msi = None

from distutils.msvccompiler import get_build_version
from distutils import log

# some modules need a static CRT to avoid problems caused by them having a
# manifest.
static_crt_modules = ["winxpgui"]


from distutils.dep_util import newer_group
from distutils.sysconfig import get_config_vars
from distutils.filelist import FileList
from distutils.errors import DistutilsExecError, DistutilsSetupError
import distutils.util

# prevent the new in 3.5 suffix of "cpXX-win32" from being added.
# (adjusting both .cp35-win_amd64.pyd and .cp35-win32.pyd to .pyd)
try:
    get_config_vars()["EXT_SUFFIX"] = re.sub("\\.cp\d\d-win((32)|(_amd64))", "", get_config_vars()["EXT_SUFFIX"])
except KeyError:
    pass # no EXT_SUFFIX in this build.

build_id_patch = build_id
if not "." in build_id_patch:
    build_id_patch = build_id_patch + ".0"
pywin32_version="%d.%d.%s" % (sys.version_info[0], sys.version_info[1],
                              build_id_patch)
print("Building pywin32", pywin32_version)

try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]

this_file = os.path.abspath(this_file)
# We get upset if the cwd is not our source dir, but it is a PITA to
# insist people manually CD there first!
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))

# Start address we assign base addresses from.  See comment re
# dll_base_address later in this file...
dll_base_address = 0x1e200000

# We need to know the platform SDK dir before we can list the extensions.
def find_platform_sdk_dir():
    # The user might have their current environment setup for the
    # SDK, in which case "MSSDK_INCLUDE" and "MSSDK_LIB" vars must be set.
    if "MSSDK_INCLUDE" in os.environ and "MSSDK_LIB" in os.environ:
        print("Using SDK as specified in the environment")
        return {
            "include": os.environ["MSSDK_INCLUDE"].split(os.path.pathsep),
            "lib": os.environ["MSSDK_LIB"].split(os.path.pathsep),
        }

    # Windows SDKs up to version 7 use a reg key SOFTWARE\Microsoft\Microsoft SDKs\Windows
    # SDKs 8 and later use SOFTWARE\Microsoft\Windows Kits\Installed Roots
    # We currently target version 8.1
    # (and strangely, via  #1293, it appears there may be a 32 bit version of
    # the SDK available which works OK on 64 bit machines!)
    flags_variants = [winreg.KEY_READ]
    try:
        flags_variants.append(winreg.KEY_READ | winreg.KEY_WOW64_32KEY)
    except AttributeError:
        pass
    for flags in flags_variants:
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                                r"SOFTWARE\Microsoft\Windows Kits\Installed Roots",
                                0,
                                flags)
            installRoot = winreg.QueryValueEx(key, "KitsRoot81")[0]
            break
        except EnvironmentError:
            pass
    else:
        print("Can't find a windows 8.1 sdk")
        return None

    # no idea what these 'um' and 'winv6.3' paths actually mean and whether
    # hard-coding them is appropriate, but here we are...
    include = [os.path.join(installRoot, "include", "um")]
    if not os.path.exists(os.path.join(include[0], "windows.h")):
        print("Found Windows 8.1 sdk in", include, "but it doesn't appear to have windows.h")
        return None
    include.append(os.path.join(installRoot, "include", "shared"))
    lib = [os.path.join(installRoot, "lib", "winv6.3", "um")]
    return {"include": include, "lib": lib}

# Some nasty hacks to prevent most of our extensions using a manifest, as
# the manifest - even without a reference to the CRT assembly - is enough
# to prevent the extension from loading.  For more details, see
# http://bugs.python.org/issue7833 (which has landed for Python 2.7 and on 3.2
# and later, which are all we care about currently)
from distutils.msvc9compiler import MSVCCompiler
MSVCCompiler._orig_spawn = MSVCCompiler.spawn
MSVCCompiler._orig_link = MSVCCompiler.link

# We need to override this method for versions where issue7833 *has* landed
# (ie, 2.7 and 3.2+)
def manifest_get_embed_info(self, target_desc, ld_args):
    _want_assembly_kept = getattr(self, '_want_assembly_kept', False)
    if not _want_assembly_kept:
        return None
    for arg in ld_args:
        if arg.startswith("/MANIFESTFILE:"):
            orig_manifest = arg.split(":", 1)[1]
            if target_desc==self.EXECUTABLE:
                rid = 1
            else:
                rid = 2
            return orig_manifest, rid
    return None

# always monkeypatch it in even though it will only be called in 2.7
# and 3.2+.
MSVCCompiler.manifest_get_embed_info = manifest_get_embed_info

def monkeypatched_spawn(self, cmd):
    is_link = cmd[0].endswith("link.exe") or cmd[0].endswith('"link.exe"')
    is_mt = cmd[0].endswith("mt.exe") or cmd[0].endswith('"mt.exe"')
    _want_assembly_kept = getattr(self, '_want_assembly_kept', False)
    if not _want_assembly_kept and is_mt:
        # We don't want mt.exe run...
        return
    if not _want_assembly_kept and is_link:
        # remove /MANIFESTFILE:... and add MANIFEST:NO
        # (but note that for winxpgui, which specifies a manifest via a
        # .rc file, this is ignored by the linker - the manifest specified
        # in the .rc file is still added)
        for i in range(len(cmd)):
            if cmd[i].startswith("/MANIFESTFILE:"):
                cmd[i] = "/MANIFEST:NO"
                break
    if _want_assembly_kept and is_mt:
        # We want mt.exe run with the original manifest
        for i in range(len(cmd)):
            if cmd[i] == "-manifest":
                cmd[i+1] = cmd[i+1] + ".orig"
                break
    self._orig_spawn(cmd)
    if _want_assembly_kept and is_link:
        # We want a copy of the original manifest so we can use it later.
        for i in range(len(cmd)):
            if cmd[i].startswith("/MANIFESTFILE:"):
                mfname = cmd[i][14:]
                shutil.copyfile(mfname, mfname + ".orig")
                break

def monkeypatched_link(self, target_desc, objects, output_filename, *args, **kw):
    # no manifests for 3.3+
    self._want_assembly_kept = sys.version_info < (3,3) and \
                               (os.path.basename(output_filename).startswith("PyISAPI_loader.dll") or \
                                os.path.basename(output_filename).startswith("perfmondata.dll") or \
                                os.path.basename(output_filename).startswith("win32ui.pyd") or \
                                target_desc==self.EXECUTABLE)
    try:
        return self._orig_link(target_desc, objects, output_filename, *args, **kw)
    finally:
        delattr(self, '_want_assembly_kept')
MSVCCompiler.spawn = monkeypatched_spawn
MSVCCompiler.link = monkeypatched_link


sdk_info = find_platform_sdk_dir()
if not sdk_info:
    print()
    print("It looks like you are trying to build pywin32 in an environment without")
    print("the necessary tools installed. It's much easier to grab binaries!")
    print()
    print("Please read the docstring at the top of this file, or read README.md")
    print("for more information.")
    print()
    raise RuntimeError("Can't find the Windows SDK")

class WinExt (Extension):
    # Base class for all win32 extensions, with some predefined
    # library and include dirs, and predefined windows libraries.
    # Additionally a method to parse .def files into lists of exported
    # symbols, and to read
    def __init__ (self, name, sources,
                  include_dirs=[],
                  define_macros=None,
                  undef_macros=None,
                  library_dirs=[],
                  libraries="",
                  runtime_library_dirs=None,
                  extra_objects=None,
                  extra_compile_args=None,
                  extra_link_args=None,
                  export_symbols=None,
                  export_symbol_file=None,
                  pch_header=None,
                  windows_h_version=None, # min version of windows.h needed.
                  extra_swig_commands=None,
                  is_regular_dll=False, # regular Windows DLL?
                  # list of headers which may not be installed forcing us to
                  # skip this extension
                  optional_headers=[],
                  base_address = None,
                  depends=None,
                  platforms=None, # none means 'all platforms'
                  unicode_mode=None, # 'none'==default or specifically true/false.
                  implib_name=None,
                  delay_load_libraries="",
                 ):
        include_dirs = ['com/win32com/src/include',
                        'win32/src'] + include_dirs
        libraries=libraries.split()
        self.delay_load_libraries=delay_load_libraries.split()
        libraries.extend(self.delay_load_libraries)

        extra_link_args = extra_link_args or []
        if export_symbol_file:
            extra_link_args.append("/DEF:" + export_symbol_file)

        # Some of our swigged files behave differently in distutils vs
        # MSVC based builds.  Always define DISTUTILS_BUILD so they can tell.
        define_macros = define_macros or []
        define_macros.append(("DISTUTILS_BUILD", None))
        define_macros.append(("_CRT_SECURE_NO_WARNINGS", None))
        self.pch_header = pch_header
        self.extra_swig_commands = extra_swig_commands or []
        self.windows_h_version = windows_h_version
        self.optional_headers = optional_headers
        self.is_regular_dll = is_regular_dll
        self.base_address = base_address
        self.platforms = platforms
        self.implib_name = implib_name
        Extension.__init__ (self, name, sources,
                            include_dirs,
                            define_macros,
                            undef_macros,
                            library_dirs,
                            libraries,
                            runtime_library_dirs,
                            extra_objects,
                            extra_compile_args,
                            extra_link_args,
                            export_symbols)
        self.depends = depends or [] # stash it here, as py22 doesn't have it.
        self.unicode_mode = unicode_mode

    def get_source_files(self, dsp):
        result = []
        if dsp is None:
            return result
        dsp_path = os.path.dirname(dsp)
        seen_swigs = []
        for line in open(dsp, "r"):
            fields = line.strip().split("=", 2)
            if fields[0]=="SOURCE":
                ext = os.path.splitext(fields[1])[1].lower()
                if ext in ['.cpp', '.c', '.i', '.rc', '.mc']:
                    pathname = os.path.normpath(os.path.join(dsp_path, fields[1]))
                    result.append(pathname)
                    if ext == '.i':
                        seen_swigs.append(pathname)

        # ack - .dsp files may have references to the generated 'foomodule.cpp'
        # from 'foo.i' - but we now do things differently...
        for ss in seen_swigs:
            base, ext = os.path.splitext(ss)
            nuke = base + "module.cpp"
            try:
                result.remove(nuke)
            except ValueError:
                pass
        # Sort the sources so that (for example) the .mc file is processed first,
        # building this may create files included by other source files.
        build_order = ".i .mc .rc .cpp".split()
        decorated = [(build_order.index(os.path.splitext(fname)[-1].lower()), fname)
                     for fname in result]
        decorated.sort()
        result = [item[1] for item in decorated]
        return result

    def finalize_options(self, build_ext):
        # distutils doesn't define this function for an Extension - it is
        # our own invention, and called just before the extension is built.
        if not build_ext.mingw32:
            if self.pch_header:
                self.extra_compile_args = self.extra_compile_args or []

            # bugger - add this to python!
            if build_ext.plat_name=="win32":
                self.extra_link_args.append("/MACHINE:x86")
            else:
                self.extra_link_args.append("/MACHINE:%s" % build_ext.plat_name[4:])

            # Put our DLL base address in (but not for our executables!)
            if self not in W32_exe_files:
                base = self.base_address
                if not base:
                    base = dll_base_addresses[self.name]
                self.extra_link_args.append("/BASE:0x%x" % (base,))

            # like Python, always use debug info, even in release builds
            # (note the compiler doesn't include debug info, so you only get
            # basic info - but its better than nothing!)
            # For now use the temp dir - later we may package them, so should
            # maybe move them next to the output file.
            pch_dir = os.path.join(build_ext.build_temp)
            if not build_ext.debug:
                self.extra_compile_args.append("/Zi")
            self.extra_compile_args.append("/Fd%s\%s_vc.pdb" %
                                          (pch_dir, self.name))
            self.extra_link_args.append("/DEBUG")
            self.extra_link_args.append("/PDB:%s\%s.pdb" %
                                       (pch_dir, self.name))
            # enable unwind semantics - some stuff needs it and I can't see
            # it hurting
            self.extra_compile_args.append("/EHsc")

            # silence: warning C4163: '__cpuidex' : not available as an intrinsic function
            self.extra_compile_args.append("/wd4163")

            if self.delay_load_libraries:
                self.libraries.append("delayimp")
                for delay_lib in self.delay_load_libraries:
                    self.extra_link_args.append("/delayload:%s.dll" % delay_lib)

            # If someone needs a specially named implib created, handle that
            if self.implib_name:
                implib = os.path.join(build_ext.build_temp, self.implib_name)
                if build_ext.debug:
                    suffix = "_d"
                else:
                    suffix = ""
                self.extra_link_args.append("/IMPLIB:%s%s.lib" % (implib, suffix))
            # Try and find the MFC headers, so we can reach inside for
            # some of the ActiveX support we need.  We need to do this late, so
            # the environment is setup correctly.
            # Only used by the win32uiole extensions, but I can't be
            # bothered making a subclass just for this - so they all get it!
            found_mfc = False
            for incl in os.environ.get("INCLUDE", "").split(os.pathsep):
                # first is a "standard" MSVC install, second is the Vista SDK.
                for candidate in (r"..\src\occimpl.h", r"..\..\src\mfc\occimpl.h"):
                    check = os.path.join(incl, candidate)
                    if os.path.isfile(check):
                        self.extra_compile_args.append('/DMFC_OCC_IMPL_H=\\"%s\\"' % candidate)
                        found_mfc = True
                        break
                if found_mfc:
                    break

        # Handle Unicode - if unicode_mode is None, then it means True
        # for py3k, false for py2
        if self.unicode_mode or self.unicode_mode is None and is_py3k:
            self.extra_compile_args.append("-DUNICODE")
            self.extra_compile_args.append("-D_UNICODE")
            self.extra_compile_args.append("-DWINNT")

class WinExt_pythonwin(WinExt):
    def __init__ (self, name, **kw):
        if 'unicode_mode' not in kw:
            kw['unicode_mode']=None
        kw.setdefault("extra_compile_args", []).extend(
                            ['-D_AFXDLL', '-D_AFXEXT','-D_MBCS'])

        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "pythonwin"

class WinExt_pythonwin_subsys_win(WinExt_pythonwin):
    def finalize_options(self, build_ext):
        WinExt_pythonwin.finalize_options(self, build_ext)

        if build_ext.mingw32:
            self.extra_link_args.append('-mwindows')
        else:
            self.extra_link_args.append('/SUBSYSTEM:WINDOWS')

            if self.unicode_mode or self.unicode_mode is None and is_py3k:
                # Unicode, Windows executables seem to need this magic:
                self.extra_link_args.append('/ENTRY:wWinMainCRTStartup')

class WinExt_win32(WinExt):
    def __init__ (self, name, **kw):
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32"

class WinExt_win32_subsys_con(WinExt_win32):
    def finalize_options(self, build_ext):
        WinExt_win32.finalize_options(self, build_ext)

        if build_ext.mingw32:
            self.extra_link_args.append('-mconsole')
            if self.unicode_mode:
                self.extra_link_args.append('-municode')
        else:
            self.extra_link_args.append('/SUBSYSTEM:CONSOLE')

class WinExt_ISAPI(WinExt):
    def get_pywin32_dir(self):
        return "isapi"

# Note this is used only for "win32com extensions", not pythoncom
# itself - thus, output is "win32comext"
class WinExt_win32com(WinExt):
    def __init__ (self, name, **kw):
        kw["libraries"] = kw.get("libraries", "") + " oleaut32 ole32"

        # COM extensions require later windows headers.
        if not kw.get("windows_h_version"):
            kw["windows_h_version"] = 0x500
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32comext/" + self.name

# Exchange extensions get special treatment:
# * Look for the Exchange SDK in the registry.
# * Output directory is different than the module's basename.
# * Require use of the Exchange 2000 SDK - this works for both VC6 and 7
# NOTE: sadly the old Exchange SDK does *not* include MAPI files - these used
# to be bundled with the Windows SDKs and/or Visual Studio, but no longer are.
class WinExt_win32com_mapi(WinExt_win32com):
    def __init__ (self, name, **kw):
        # The Exchange 2000 SDK seems to install itself without updating
        # LIB or INCLUDE environment variables.  It does register the core
        # directory in the registry tho - look it up
        sdk_install_dir = None
        libs = kw.get("libraries", "")
        keyname = r"SOFTWARE\Microsoft\Exchange\SDK"
        flags = winreg.KEY_READ
        try:
            flags |= winreg.KEY_WOW64_32KEY
        except AttributeError:
            pass # this version doesn't support 64 bits, so must already be using 32bit key.
        for root in winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER:
            try:
                keyob = winreg.OpenKey(root, keyname, 0, flags)
                value, type_id = winreg.QueryValueEx(keyob, "INSTALLDIR")
                if type_id == winreg.REG_SZ:
                    sdk_install_dir = value
                    break
            except WindowsError:
                pass
        if sdk_install_dir is not None:
            d = os.path.join(sdk_install_dir, "SDK", "Include")
            if os.path.isdir(d):
                kw.setdefault("include_dirs", []).insert(0, d)
            d = os.path.join(sdk_install_dir, "SDK", "Lib")
            if os.path.isdir(d):
                kw.setdefault("library_dirs", []).insert(0, d)

        # The stand-alone exchange SDK has these libs
        if distutils.util.get_platform() == 'win-amd64':
            # Additional utility functions are only available for 32-bit builds.
            pass
        else:
            libs += " version user32 advapi32 Ex2KSdk sadapi netapi32"
        kw["libraries"] = libs
        WinExt_win32com.__init__(self, name, **kw)

    def get_pywin32_dir(self):
    # 'win32com.mapi.exchange' and 'win32com.mapi.exchdapi' currently only
    # ones with this special requirement
        return "win32comext/mapi"

# A hacky extension class for pywintypesXX.dll and pythoncomXX.dll
class WinExt_system32(WinExt):
    def get_pywin32_dir(self):
        return "pywin32_system32"

################################################################
# Extensions to the distutils commands.

# Start with 2to3 related stuff for py3k.
do_2to3 = is_py3k
if do_2to3:
    def refactor_filenames(filenames):
        from lib2to3.refactor import RefactoringTool
        # we only need some fixers.
        fixers = """basestring exec except dict import imports next nonzero
                    print raise raw_input long standarderror types unicode
                    urllib xrange""".split()
        fqfixers = ['lib2to3.fixes.fix_' + f for f in fixers]

        options = dict(doctests_only=False, fix=[], list_fixes=[],
                       print_function=False, verbose=False,
                       write=True)
        r = RefactoringTool(fqfixers, options)
        for updated_file in filenames:
            if os.path.splitext(updated_file)[1] not in ['.py', '.pys']:
                continue
            log.info("Refactoring %s" % updated_file)
            try:
                r.refactor_file(updated_file, write=True, doctests_only=False)
                if os.path.exists(updated_file + ".bak"):
                    os.unlink(updated_file + ".bak")
            except Exception:
                log.warn("WARNING: Failed to 2to3 %s: %s" % (updated_file, sys.exc_info()[1]))
else:
    # py2k - nothing to do.
    def refactor_filenames(filenames):
        pass

# 'build_py' command
if do_2to3:
    # Force 2to3 to be run for py3k versions.
    class my_build_py(build_py):
        def finalize_options(self):
            build_py.finalize_options(self)
            # must force as the 2to3 conversion happens in place so an
            # interrupted build can cause py2 syntax files in a py3k build.
            self.force = True

        def run(self):
            self.updated_files = []

            # Base class code
            if self.py_modules:
                self.build_modules()
            if self.packages:
                self.build_packages()
                self.build_package_data()

            # 2to3
            refactor_filenames(self.updated_files)

            # Remaining base class code
            self.byte_compile(self.get_outputs(include_bytecode=0))

        def build_module(self, module, module_file, package):
            res = build_py.build_module(self, module, module_file, package)
            if res[1]:
                # file was copied
                self.updated_files.append(res[0])
            return res
else:
    my_build_py = build_py # default version.

# 'build_scripts' command
if do_2to3:
    class my_build_scripts(build_scripts):
        def copy_file(self, src, dest):
            dest, copied = build_scripts.copy_file(self, src, dest)
            # 2to3
            if not self.dry_run and copied:
                refactor_filenames([dest])
            return dest, copied

else:
    my_build_scripts = build_scripts

# 'build' command
class my_build(build):
    def run(self):
        build.run(self)
        # write a pywin32.version.txt.
        ver_fname = os.path.join(gettempdir(), "pywin32.version.txt")
        try:
            f = open(ver_fname, "w")
            f.write("%s\n" % build_id)
            f.close()
        except EnvironmentError as why:
            print("Failed to open '%s': %s" % (ver_fname, why))

class my_build_ext(build_ext):

    def finalize_options(self):
        build_ext.finalize_options(self)
        self.windows_h_version = None
        # The pywintypes library is created in the build_temp
        # directory, so we need to add this to library_dirs
        self.library_dirs.append(self.build_temp)
        self.mingw32 = (self.compiler == "mingw32")
        if self.mingw32:
            self.libraries.append("stdc++")

        self.excluded_extensions = [] # list of (ext, why)
        self.swig_cpp = True # hrm - deprecated - should use swig_opts=-c++??
        if not hasattr(self, 'plat_name'):
            # Old Python version that doesn't support cross-compile
            self.plat_name = distutils.util.get_platform()

    def _fixup_sdk_dirs(self):
        # Adjust paths etc for the platform SDK - the default paths used by
        # distutils don't include the platform SDK.
        # Note that just having them in INCLUDE/LIB does *not* work -
        # distutils thinks it knows better, and resets those vars (see notes
        # below about how the paths are put together)

        # Called after the compiler is initialized, but before the extensions
        # are built.  NOTE: this means setting self.include_dirs etc will
        # have no effect, so we poke our path changes directly into the
        # compiler (we can't call this *before* the compiler is setup, as
        # then our environment changes would have no effect - see below)

        # distutils puts the path together like so:
        # * compiler command line includes /I entries for each dir in
        #   ext.include_dir + build_ext.include_dir (ie, extension's come first)
        # * The compiler initialization sets the INCLUDE/LIB etc env vars to the
        #   values read from the registry (ignoring anything that was there)

        # We are also at the mercy of how MSVC processes command-line
        # includes vs env vars (presumably environment comes last) - so,
        # moral of the story:
        # * To get a path at the start, it must be at the start of
        #   ext.includes
        # * To get a path at the end, it must be at the end of
        #   os.environ("INCLUDE")
        # Note however that the environment tweaking can only be done after
        # the compiler has set these vars, which is quite late -
        # build_ext.run() - so global environment hacks are done in our
        # build_extensions() override)
        #
        # Also note that none of our extensions have individual include files
        # that must be first - so for practical purposes, any entry in
        # build_ext.include_dirs should 'win' over the compiler's dirs.
        assert self.compiler.initialized # if not, our env changes will be lost!

        is_64bit = self.plat_name == 'win-amd64'
        for extra in sdk_info["include"]:
            # should not be possible for the SDK dirs to already be in our
            # include_dirs - they may be in the registry etc from MSVC, but
            # those aren't reflected here...
            assert extra not in self.include_dirs
            # and we will not work as expected if the dirs don't exist
            assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
            self.compiler.add_include_dir(extra)
        # and again for lib dirs.
        for extra in sdk_info["lib"]:
            extra = os.path.join(extra, 'x64' if is_64bit else 'x86')
            assert os.path.isdir(extra), extra
            assert extra not in self.library_dirs # see above
            assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
            self.compiler.add_library_dir(extra)

        log.debug("After SDK processing, includes are %s", self.compiler.include_dirs)
        log.debug("After SDK processing, libs are %s", self.compiler.library_dirs)

    def _why_cant_build_extension(self, ext):
        # Return None, or a reason it can't be built.
        # Exclude exchange 32-bit utility libraries from 64-bit
        # builds. Note that the exchange module now builds, but only
        # includes interfaces for 64-bit builds.
        if self.plat_name == 'win-amd64' and ext.name == 'exchdapi':
            return "No 64-bit library for utility functions available."
        if get_build_version() >=14:
            if ext.name == 'exchange':
                ext.libraries.append('legacy_stdio_definitions')
            elif ext.name == 'exchdapi':
                return "Haven't worked out how to build on vs2015"
        include_dirs = self.compiler.include_dirs + \
                       os.environ.get("INCLUDE", "").split(os.pathsep)
        if self.windows_h_version is None:
            # Note that we used to try and find WINVER or _WIN32_WINNT macros
            # here defining the version of the Windows SDK we use and check
            # it was late enough for the extension being built. But since we
            # moved to the Windows 8.1 SDK (or later), this isn't necessary
            # as all modules require less than this.
            pass

        look_dirs = include_dirs
        for h in ext.optional_headers:
            for d in look_dirs:
                if os.path.isfile(os.path.join(d, h)):
                    break
            else:
                log.debug("Looked for %s in %s", h, look_dirs)
                return "The header '%s' can not be located." % (h,)

        common_dirs = self.compiler.library_dirs[:]
        common_dirs += os.environ.get("LIB", "").split(os.pathsep)
        patched_libs = []
        for lib in ext.libraries:
            if lib.lower() in self.found_libraries:
                found = self.found_libraries[lib.lower()]
            else:
                look_dirs = common_dirs + ext.library_dirs
                found = self.compiler.find_library_file(look_dirs, lib, self.debug)
                if not found:
                    log.debug("Looked for %s in %s", lib, look_dirs)
                    return "No library '%s'" % lib
                self.found_libraries[lib.lower()] = found
            patched_libs.append(os.path.splitext(os.path.basename(found))[0])

        if ext.platforms and self.plat_name not in ext.platforms:
            return "Only available on platforms %s" % (ext.platforms,)

        # We update the .libraries list with the resolved library name.
        # This is really only so "_d" works.
        ext.libraries = patched_libs
        return None # no reason - it can be built!

    def _build_scintilla(self):
        path = "pythonwin\\Scintilla"
        makefile = "makefile_pythonwin"
        makeargs = []

        if self.debug:
            makeargs.append("DEBUG=1")
        if not self.verbose:
            makeargs.append("/C") # nmake: /C Suppress output messages
            makeargs.append("QUIET=1")
        # We build the DLL into our own temp directory, then copy it to the
        # real directory - this avoids the generated .lib/.exp
        build_temp = os.path.abspath(os.path.join(self.build_temp, "scintilla"))
        self.mkpath(build_temp)
        # Use short-names, as the scintilla makefiles barf with spaces.
        if " " in build_temp:
            # ack - can't use win32api!!!  This is the best I could come up
            # with:
            # C:\>for %I in ("C:\Program Files",) do @echo %~sI
            # C:\PROGRA~1
            cs = os.environ.get('comspec', 'cmd.exe')
            cmd = cs + ' /c for %I in ("' + build_temp + '",) do @echo %~sI'
            build_temp = os.popen(cmd).read().strip()
            assert os.path.isdir(build_temp), build_temp
        makeargs.append("SUB_DIR_O=%s" % build_temp)
        makeargs.append("SUB_DIR_BIN=%s" % build_temp)
        makeargs.append("DIR_PYTHON=%s" % sys.prefix)

        cwd = os.getcwd()
        os.chdir(path)
        try:
            cmd = ["nmake.exe", "/nologo", "/f", makefile] + makeargs
            self.spawn(cmd)
        finally:
            os.chdir(cwd)

        # The DLL goes in the Pythonwin directory.
        if self.debug:
            base_name = "scintilla_d.dll"
        else:
            base_name = "scintilla.dll"
        self.copy_file(
                    os.path.join(self.build_temp, "scintilla", base_name),
                    os.path.join(self.build_lib, "pythonwin"))

    def _build_pycom_loader(self):
        # the base compiler strips out the manifest from modules it builds
        # which can't be done for this module - having the manifest is the
        # reason it needs to exist!
        # At least this is made easier by it not depending on Python itself,
        # so the compile and link are simple...
        suffix = "%d%d" % (sys.version_info[0], sys.version_info[1])
        if self.debug:
            suffix += '_d'
        src = "com\\win32com\\src\\PythonCOMLoader.cpp"
        build_temp = os.path.abspath(self.build_temp)
        obj = os.path.join(build_temp, os.path.splitext(src)[0]+".obj")
        dll = os.path.join(self.build_lib, "pywin32_system32", "pythoncomloader"+suffix+".dll")
        if self.force or newer_group([src], obj, 'newer'):
            ccargs = [self.compiler.cc, '/c']
            if self.debug:
                ccargs.extend(self.compiler.compile_options_debug)
            else:
                ccargs.extend(self.compiler.compile_options)
            ccargs.append('/Fo' + obj)
            ccargs.append(src)
            ccargs.append('/DDLL_DELEGATE=\\"pythoncom%s.dll\\"' % (suffix,))
            self.spawn(ccargs)

        deffile = "com\\win32com\\src\\PythonCOMLoader.def"
        if self.force or newer_group([obj, deffile], dll, 'newer'):
            largs = [self.compiler.linker, '/DLL', '/nologo', '/incremental:no']
            if self.debug:
                largs.append("/DEBUG")
            temp_manifest = os.path.join(build_temp, os.path.basename(dll) + ".manifest")
            largs.append('/MANIFESTFILE:' + temp_manifest)
            largs.append('/PDB:None')
            largs.append("/OUT:" + dll)
            largs.append("/DEF:" + deffile)
            largs.append("/IMPLIB:" + os.path.join(build_temp, "PythonCOMLoader"+suffix+".lib"))
            largs.append(obj)
            self.spawn(largs)
            # and the manifest if one exists.
            if os.path.isfile(temp_manifest):
                out_arg = '-outputresource:%s;2' % (dll,)
                self.spawn(['mt.exe', '-nologo', '-manifest', temp_manifest, out_arg])

    def lookupMfcInVisualStudio(self, mfc_version, mfc_libraries):
        # Looking for the MFC files in the installation paths of the Visual Studios
        plat_dir_64 = "x64"
        mfc_dir = "Microsoft.{}.MFC".format(mfc_version.upper())
        mfc_contents = []
        # 2.7, 3.0, 3.1 and 3.2 all use(d) vs2008 (compiler version 1500)
        if sys.version_info < (3, 3):
            product_key = "SOFTWARE\\Microsoft\\VisualStudio\\9.0\\Setup\\VC"
            plat_dir_64 = "amd64"
            mfc_files = mfc_libraries + ["Microsoft.VC90.MFC.manifest", ]
        # 3.3 and 3.4 use(d) vs2010 (compiler version 1600, crt=10)
        elif sys.version_info < (3, 5):
            product_key = "SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Setup\\VC"
            mfc_files = mfc_libraries
        # 3.5 and later on vs2015 (compiler version 1900, crt=14)
        else:
            product_key = "SOFTWARE\\Microsoft\\VisualStudio\\14.0\\Setup\\VC"
            mfc_files = mfc_libraries

        # On a 64bit host, the value we are looking for is actually in
        # SysWow64Node - but that is only available on xp and later.
        access = winreg.KEY_READ
        if sys.getwindowsversion()[0] >= 5:
            access = access | 512 # KEY_WOW64_32KEY
        if self.plat_name == 'win-amd64':
            plat_dir = plat_dir_64
        else:
            plat_dir = "x86"
        # Find the redist directory.
        vckey = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                               product_key,
                               0,
                               access,
                               )
        val = winreg.QueryValueEx(vckey, "ProductDir")[0]
        mfc_dir = os.path.join(val, "redist", plat_dir, mfc_dir)
        if os.path.isdir(mfc_dir):
            # Ensuring absolute paths
            mfc_contents = [os.path.join(mfc_dir, mfc_file) for mfc_file in mfc_files]
            mfc_contents = [mfc_content for mfc_content in mfc_contents if os.path.exists(mfc_content)]
            # Should have the same length - if not we lost a file!
            if len(mfc_files) is not len(mfc_contents):
                mfc_contents = []
        
        return mfc_contents

    def lookupMfcInWinSxS(self, mfc_version, mfc_libraries):
        mfc_contents = []
        windows_dir = os.getenv("windir", "C:\\Windows")
        if os.path.isdir(windows_dir):
            winsxs_path = os.path.join(windows_dir, "WinSxS")
            if os.path.isdir(winsxs_path):
                mfc_redist_path = None
                winsxs_listdir = os.listdir(winsxs_path)
                winsxs_listdir.sort()
                for entry in winsxs_listdir:
                    if entry.startswith("{}_microsoft.{}.mfc_".format(platform.machine().lower(), mfc_version)) and os.path.isdir(os.path.join(winsxs_path, entry)):
                        for mfc_libary in mfc_libraries:
                            if not os.path.isfile(os.path.join(winsxs_path, entry, mfc_libary)):
                                continue
                        mfc_redist_path = entry
                if mfc_redist_path:
                    mfc_contents = [os.path.join(winsxs_path, mfc_redist_path, mfc_libary) for mfc_libary in mfc_libraries]
                    mfc_manifest_file = os.path.join(winsxs_path, "Manifests", "{}.manifest".format(mfc_redist_path))
                    mfc_signature_file = os.path.join(winsxs_path, "Manifests", "{}.cat".format(mfc_redist_path))
                    if os.path.isfile(mfc_manifest_file): # Looking whether there is a manifest file
                        mfc_contents.append(mfc_manifest_file)
                        if os.path.isfile(mfc_signature_file): # If there is, also add the signaure file
                            mfc_contents.append(mfc_signature_file)
                else:
                    print("Could not find any redist libraries in WinSxS!")
            else:
                print("Could not find WinSxS directory in %WINDIR%.")
        else:
            print("Windows directory not found!")
        
        return mfc_contents

    def build_extensions(self):
        # First, sanity-check the 'extensions' list
        self.check_extensions_list(self.extensions)

        self.found_libraries = {}

        if not hasattr(self.compiler, 'initialized'):
            # 2.3 and earlier initialized at construction
            self.compiler.initialized = True
        else:
            if not self.compiler.initialized:
                self.compiler.initialize()

        self._fixup_sdk_dirs()

        # Here we hack a "pywin32" directory (one of 'win32', 'win32com',
        # 'pythonwin' etc), as distutils doesn't seem to like the concept
        # of multiple top-level directories.
        assert self.package is None
        for ext in self.extensions:
            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError("Not a win32 package!")
            self.build_extension(ext)

        for ext in W32_exe_files:
            ext.finalize_options(self)
            why = self._why_cant_build_extension(ext)
            if why is not None:
                self.excluded_extensions.append((ext, why))
                assert why, "please give a reason, or None"
                print("Skipping %s: %s" % (ext.name, why))
                continue

            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError("Not a win32 package!")
            self.build_exefile(ext)

        # Not sure how to make this completely generic, and there is no
        # need at this stage.
        if sys.version_info > (2, 7) and sys.version_info < (3, 3):
            # only stuff built with msvc9 needs this loader.
            self._build_pycom_loader()
        self._build_scintilla()
        # Copy cpp lib files needed to create Python COM extensions
        clib_files = (['win32', 'pywintypes%s.lib'],
                      ['win32com', 'pythoncom%s.lib'],
                      ['win32com', 'axscript%s.lib'])
        for clib_file in clib_files:
            target_dir = os.path.join(self.build_lib, clib_file[0], "libs")
            if not os.path.exists(target_dir):
                self.mkpath(target_dir)
            suffix = ""
            if self.debug:
                suffix = "_d"
            fname = clib_file[1] % suffix
            self.copy_file(os.path.join(self.build_temp, fname),
                           target_dir
                           )
        # The MFC DLLs.
        target_dir = os.path.join(self.build_lib, "pythonwin")

        # Common values for the MFC lookup over the Visual Studio installation and redist installation.
        if sys.version_info < (3, 3):
            mfc_version = "vc90"
            mfc_libraries = ["mfc90.dll", "mfc90u.dll", "mfcm90.dll", "mfcm90u.dll"]
        # 3.3 and 3.4 use(d) vs2010 (compiler version 1600, crt=10)
        elif sys.version_info < (3, 5):
            mfc_version = "vc100"
            mfc_libraries = ["mfc100u.dll", "mfcm100u.dll"]
        # 3.5 and later on vs2015 (compiler version 1900, crt=14)
        else:
            mfc_version = "vc140"
            mfc_libraries = ["mfc140u.dll", "mfcm140u.dll"]

        mfc_contents = self.lookupMfcInVisualStudio(mfc_version, mfc_libraries)
        if not mfc_contents:
            print("Can't find MFC contents in VisualStudio. Looking into WinSxS now..")
            mfc_contents = self.lookupMfcInWinSxS(mfc_version, mfc_libraries)

        if not mfc_contents:
            raise RuntimeError("No MFC files found!")

        for mfc_content in mfc_contents:
            shutil.copyfile(mfc_content,
                            os.path.join(target_dir, os.path.split(mfc_content)[1]),
                            )


    def build_exefile(self, ext):
        sources = ext.sources
        if sources is None or type(sources) not in (list, tuple):
            raise DistutilsSetupError(
                  ("in 'ext_modules' option (extension '%s'), " +
                   "'sources' must be present and must be " +
                   "a list of source filenames") % ext.name)
        sources = list(sources)

        log.info("building exe '%s'", ext.name)

        fullname = self.get_ext_fullname(ext.name)
        if self.inplace:
            # ignore build-lib -- put the compiled extension into
            # the source tree along with pure Python modules

            modpath = string.split(fullname, '.')
            package = string.join(modpath[0:-1], '.')
            base = modpath[-1]

            build_py = self.get_finalized_command('build_py')
            package_dir = build_py.get_package_dir(package)
            ext_filename = os.path.join(package_dir,
                                        self.get_ext_filename(base))
        else:
            ext_filename = os.path.join(self.build_lib,
                                        self.get_ext_filename(fullname))
        depends = sources + ext.depends
        if not (self.force or newer_group(depends, ext_filename, 'newer')):
            log.debug("skipping '%s' executable (up-to-date)", ext.name)
            return
        else:
            log.info("building '%s' executable", ext.name)

        # First, scan the sources for SWIG definition files (.i), run
        # SWIG on 'em to create .c files, and modify the sources list
        # accordingly.
        sources = self.swig_sources(sources, ext)

        # Next, compile the source code to object files.

        # XXX not honouring 'define_macros' or 'undef_macros' -- the
        # CCompiler API needs to change to accommodate this, and I
        # want to do one thing at a time!

        # Two possible sources for extra compiler arguments:
        #   - 'extra_compile_args' in Extension object
        #   - CFLAGS environment variable (not particularly
        #     elegant, but people seem to expect it and I
        #     guess it's useful)
        # The environment variable should take precedence, and
        # any sensible compiler will give precedence to later
        # command line args.  Hence we combine them in order:
        extra_args = ext.extra_compile_args or []

        macros = ext.define_macros[:]
        for undef in ext.undef_macros:
            macros.append((undef,))
        # Note: custom 'output_dir' needed due to servicemanager.pyd and
        # pythonservice.exe being built from the same .cpp file - without
        # this, distutils gets confused, as they both try and use the same
        # .obj.
        output_dir = os.path.join(self.build_temp, ext.name)
        kw = {'output_dir': output_dir,
              'macros': macros,
              'include_dirs': ext.include_dirs,
              'debug': self.debug,
              'extra_postargs': extra_args,
              'depends': ext.depends,
        }
        objects = self.compiler.compile(sources, **kw)

        # XXX -- this is a Vile HACK!
        #
        # The setup.py script for Python on Unix needs to be able to
        # get this list so it can perform all the clean up needed to
        # avoid keeping object files around when cleaning out a failed
        # build of an extension module.  Since Distutils does not
        # track dependencies, we have to get rid of intermediates to
        # ensure all the intermediates will be properly re-built.
        #
        self._built_objects = objects[:]

        # Now link the object files together into a "shared object" --
        # of course, first we have to figure out all the other things
        # that go into the mix.
        if ext.extra_objects:
            objects.extend(ext.extra_objects)
        extra_args = ext.extra_link_args or []

        # 2.2 has no 'language' support
        kw = { 'libraries': self.get_libraries(ext),
               'library_dirs': ext.library_dirs,
               'runtime_library_dirs': ext.runtime_library_dirs,
               'extra_postargs': extra_args,
               'debug': self.debug,
               'build_temp': self.build_temp,
        }

        # Detect target language, if not provided
        language = ext.language or self.compiler.detect_language(sources)
        kw["target_lang"] = language

        self.compiler.link(
            "executable",
            objects, ext_filename, **kw)

    def build_extension(self, ext):
        # It is well known that some of these extensions are difficult to
        # build, requiring various hard-to-track libraries etc.  So we
        # check the extension list for the extra libraries explicitly
        # listed.  We then search for this library the same way the C
        # compiler would - if we can't find a  library, we exclude the
        # extension from the build.
        # Note we can't do this in advance, as some of the .lib files
        # we depend on may be built as part of the process - thus we can
        # only check an extension's lib files as we are building it.
        why = self._why_cant_build_extension(ext)
        if why is not None:
            self.excluded_extensions.append((ext, why))
            assert why, "please give a reason, or None"
            print("Skipping %s: %s" % (ext.name, why))
            return
        self.current_extension = ext

        ext.finalize_options(self)

        # ensure the SWIG .i files are treated as dependencies.
        for source in ext.sources:
            if source.endswith(".i"):
                self.find_swig() # for the side-effect of the environment value.
                # Find the swig_lib .i files we care about for dependency tracking.
                ext.swig_deps = glob.glob(os.path.join(os.environ["SWIG_LIB"], "python", "*.i"))
                ext.depends.extend(ext.swig_deps)
                break
        else:
            ext.swig_deps = None

        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        want_static_crt = sys.version_info > (2,6) and ext.name in static_crt_modules
        if want_static_crt:
            self.compiler.compile_options.remove('/MD')
            self.compiler.compile_options.append('/MT')
            self.compiler.compile_options_debug.remove('/MDd')
            self.compiler.compile_options_debug.append('/MTd')

        try:
            build_ext.build_extension(self, ext)
            # XXX This has to be changed for mingw32
            # Get the .lib files we need.  This is limited to pywintypes,
            # pythoncom and win32ui - but the first 2 have special names
            extra = self.debug and "_d.lib" or ".lib"
            if ext.name in ("pywintypes", "pythoncom"):
                # The import libraries are created as PyWinTypes23.lib, but
                # are expected to be pywintypes.lib.
                name1 = "%s%d%d%s" % (ext.name, sys.version_info[0], sys.version_info[1], extra)
                name2 = "%s%s" % (ext.name, extra)
            elif ext.name in ("win32ui",):
                name1 = name2 = ext.name + extra
            else:
                name1 = name2 = None
            if name1 is not None:
                # The compiler always creates 'pywintypes22.lib', whereas we
                # actually want 'pywintypes.lib' - copy it over.
                # Worse: 2.3+ MSVCCompiler constructs the .lib file in the same
                # directory as the first source file's object file:
                #    os.path.dirname(objects[0])
                # rather than in the self.build_temp directory
                # 2.3+ - Wrong dir, numbered name
                src = os.path.join(old_build_temp,
                                   os.path.dirname(ext.sources[0]),
                                   name1)
                dst = os.path.join(old_build_temp, name2)
                if os.path.abspath(src) != os.path.abspath(dst):
                    self.copy_file(src, dst)#, update=1)
        finally:
            self.build_temp = old_build_temp
            if want_static_crt:
                self.compiler.compile_options.remove('/MT')
                self.compiler.compile_options.append('/MD')
                self.compiler.compile_options_debug.remove('/MTd')
                self.compiler.compile_options_debug.append('/MDd')

    def get_ext_filename(self, name):
        # The pywintypes and pythoncom extensions have special names
        extra_dll = self.debug and "_d.dll" or ".dll"
        extra_exe = self.debug and "_d.exe" or ".exe"
        # *sob* - python fixed this bug in python 3.1 (bug 6403)
        # So in the fixed versions we only get the base name, and if the
        # output name is simply 'dir\name' we need to nothing.

        # The pre 3.1 pywintypes
        if name == "pywin32_system32.pywintypes":
            return "pywin32_system32\\pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        # 3.1+ pywintypes
        elif name == "pywintypes":
            return "pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        # pre 3.1 pythoncom
        elif name == "pywin32_system32.pythoncom":
            return "pywin32_system32\\pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        # 3.1+ pythoncom
        elif name == "pythoncom":
            return "pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        # Pre 3.1 rest.
        elif name.endswith("win32.perfmondata"):
            return "win32\\perfmondata" + extra_dll
        elif name.endswith("win32.pythonservice"):
            return "win32\\pythonservice" + extra_exe
        elif name.endswith("pythonwin.Pythonwin"):
            return "pythonwin\\Pythonwin" + extra_exe
        elif name.endswith("isapi.PyISAPI_loader"):
            return "isapi\\PyISAPI_loader" + extra_dll
        # The post 3.1 rest
        elif name in ['perfmondata', 'PyISAPI_loader']:
            return name + extra_dll
        elif name in ['pythonservice', 'Pythonwin']:
            return name + extra_exe

        return build_ext.get_ext_filename(self, name)

    def get_export_symbols(self, ext):
        if ext.is_regular_dll:
            return ext.export_symbols
        return build_ext.get_export_symbols(self, ext)

    def find_swig(self):
        if "SWIG" in os.environ:
            swig = os.environ["SWIG"]
        else:
            # We know where our swig is
            swig = os.path.abspath("swig\\swig.exe")
        lib = os.path.join(os.path.dirname(swig), "swig_lib")
        os.environ["SWIG_LIB"] = lib
        return swig

    def swig_sources(self, sources, ext=None):
        new_sources = []
        swig_sources = []
        swig_targets = {}
        # XXX this drops generated C/C++ files into the source tree, which
        # is fine for developers who want to distribute the generated
        # source -- but there should be an option to put SWIG output in
        # the temp dir.
        # Adding py3k to the mix means we *really* need to move to generating
        # to the temp dir...
        target_ext = '.cpp'
        for source in sources:
            (base, sext) = os.path.splitext(source)
            if sext == ".i":             # SWIG interface file
                if os.path.split(base)[1] in swig_include_files:
                    continue
                swig_sources.append(source)
                # Patch up the filenames for various special cases...
                if os.path.basename(base) in swig_interface_parents:
                    swig_targets[source] = base + target_ext
                elif self.current_extension.name == "winxpgui" and \
                     os.path.basename(base)=="win32gui":
                    # More vile hacks.  winxpmodule is built from win32gui.i -
                    # just different #defines are setup for windows.h.
                    new_target = os.path.join(os.path.dirname(base),
                                              "winxpgui_swig%s" % (target_ext,))
                    swig_targets[source] = new_target
                    new_sources.append(new_target)
                else:
                    new_target = '%s_swig%s' % (base, target_ext)
                    new_sources.append(new_target)
                    swig_targets[source] = new_target
            else:
                new_sources.append(source)

        if not swig_sources:
            return new_sources

        swig = self.find_swig()
        for source in swig_sources:
            swig_cmd = [swig, "-python", "-c++"]
            swig_cmd.append("-dnone",) # we never use the .doc files.
            swig_cmd.extend(self.current_extension.extra_swig_commands)
            if not is_py3k:
                swig_cmd.append("-DSWIG_PY2K")
            if distutils.util.get_platform() == 'win-amd64':
                swig_cmd.append("-DSWIG_PY64BIT")
            else:
                swig_cmd.append("-DSWIG_PY32BIT")
            target = swig_targets[source]
            try:
                interface_parent = swig_interface_parents[
                                os.path.basename(os.path.splitext(source)[0])]
            except KeyError:
                # "normal" swig file - no special win32 issues.
                pass
            else:
                # Using win32 extensions to SWIG for generating COM classes.
                if interface_parent is not None:
                    # generating a class, not a module.
                    swig_cmd.append("-pythoncom")
                    if interface_parent:
                        # A class deriving from other than the default
                        swig_cmd.extend(
                                ["-com_interface_parent", interface_parent])

            # This 'newer' check helps python 2.2 builds, which otherwise
            # *always* regenerate the .cpp files, meaning every future
            # build for any platform sees these as dirty.
            # This could probably go once we generate .cpp into the temp dir.
            fqsource = os.path.abspath(source)
            fqtarget = os.path.abspath(target)
            rebuild = self.force or (ext and newer_group(ext.swig_deps + [fqsource], fqtarget))

            # can remove once edklib is no longer used for 32-bit builds
            if source == "com/win32comext/mapi/src/exchange.i":
                rebuild = True

            log.debug("should swig %s->%s=%s", source, target, rebuild)
            if rebuild:
                swig_cmd.extend(["-o", fqtarget, fqsource])
                log.info("swigging %s to %s", source, target)
                out_dir = os.path.dirname(source)
                cwd = os.getcwd()
                os.chdir(out_dir)
                try:
                    self.spawn(swig_cmd)
                finally:
                    os.chdir(cwd)
            else:
                log.info("skipping swig of %s", source)

        return new_sources

class my_install(install):
    def run(self):
        install.run(self)
        # Custom script we run at the end of installing - this is the same script
        # run by bdist_wininst
        # This child process won't be able to install the system DLLs until our
        # process has terminated (as distutils imports win32api!), so we must use
        # some 'no wait' executor - spawn seems fine!  We pass the PID of this
        # process so the child will wait for us.
        # XXX - hmm - a closer look at distutils shows it only uses win32api
        # if _winreg fails - and this never should.  Need to revisit this!
        # If self.root has a value, it means we are being "installed" into
        # some other directory than Python itself (eg, into a temp directory
        # for bdist_wininst to use) - in which case we must *not* run our
        # installer
        if not self.dry_run and not self.root:
            # We must run the script we just installed into Scripts, as it
            # may have had 2to3 run over it.
            filename = os.path.join(self.install_scripts, "pywin32_postinstall.py")
            if not os.path.isfile(filename):
                raise RuntimeError("Can't find '%s'" % (filename,))
            print("Executing post install script...")
            # What executable to use?  This one I guess.
            subprocess.Popen([
                sys.executable, filename,
                "-install",
                "-destination", self.install_lib,
                "-quiet",
                "-wait", str(os.getpid()),
            ])

# As per get_source_files, we need special handling so .mc file is
# processed first.  It appears there was an intention to fix distutils
# itself, but as at 2.4 that hasn't happened.  We need yet more vile
# hacks to get a subclassed compiler in.
# (otherwise we replace all of build_extension!)
def my_new_compiler(**kw):
    if 'compiler' in kw and kw['compiler'] in (None, 'msvc'):
        return my_compiler()
    return orig_new_compiler(**kw)

# No way to cleanly wedge our compiler sub-class in.
from distutils import ccompiler, msvccompiler
orig_new_compiler = ccompiler.new_compiler
ccompiler.new_compiler = my_new_compiler

base_compiler = msvccompiler.MSVCCompiler

class my_compiler(base_compiler):
    # Just one GUIDS.CPP and it gives trouble on mainwin too. Maybe I
    # should just rename the file, but a case-only rename is likely to be
    # worse!  This can probably go away once we kill the VS project files
    # though, as we can just specify the lowercase name in the module def.
    _cpp_extensions = base_compiler._cpp_extensions + [".CPP"]
    src_extensions = base_compiler.src_extensions + [".CPP"]

    def link(self,
              target_desc,
              objects,
              output_filename,
              output_dir=None,
              libraries=None,
              library_dirs=None,
              runtime_library_dirs=None,
              export_symbols=None,
              debug=0, *args, **kw):
        msvccompiler.MSVCCompiler.link( self,
                                        target_desc,
                                        objects,
                                        output_filename,
                                        output_dir,
                                        libraries,
                                        library_dirs,
                                        runtime_library_dirs,
                                        export_symbols,
                                        debug, *args, **kw)
        # Here seems a good place to stamp the version of the built
        # target.  Do this externally to avoid suddenly dragging in the
        # modules needed by this process, and which we will soon try and
        # update.
        try:
            ok = True
        except ImportError:
            ok = False
        if ok:
            stamp_script = os.path.join(sys.prefix,
                                        "Lib",
                                        "site-packages",
                                        "win32",
                                        "lib",
                                        "win32verstamp.py",
                                        )
            ok = os.path.isfile(stamp_script)
        if ok:
            args = [sys.executable]
            args.append(stamp_script)
            args.append("--version=%s" % (pywin32_version,))
            args.append("--comments=https://github.com/mhammond/pywin32")
            args.append("--original-filename=%s" % (os.path.basename(output_filename),))
            args.append("--product=PyWin32")
            if '-v' not in sys.argv:
                args.append("--quiet")
            args.append(output_filename)
            try:
                self.spawn(args)
            except DistutilsExecError as msg:
                log.info("VersionStamp failed: %s", msg)
                ok = False
        if not ok:
            log.info('Unable to import verstamp, no version info will be added')


################################################################

class my_install_data(install_data):
    """A custom install_data command, which will install it's files
    into the standard directories (normally lib/site-packages).
    """
    def finalize_options(self):
        if self.install_dir is None:
            installobj = self.distribution.get_command_obj('install')
            self.install_dir = installobj.install_lib
        print('Installing data files to %s' % self.install_dir)
        install_data.finalize_options(self)

    def copy_file(self, src, dest):
        dest, copied = install_data.copy_file(self, src, dest)
        # 2to3
        if not self.dry_run and copied:
            refactor_filenames([dest])
        return dest, copied

################################################################

pywintypes = WinExt_system32('pywintypes',
    sources = [
        "win32/src/PyACL.cpp",
        "win32/src/PyDEVMODE.cpp",
        "win32/src/PyHANDLE.cpp",
        "win32/src/PyIID.cpp",
        "win32/src/PyLARGE_INTEGER.cpp",
        "win32/src/PyOVERLAPPED.cpp",
        "win32/src/PySECURITY_ATTRIBUTES.cpp",
        "win32/src/PySECURITY_DESCRIPTOR.cpp",
        "win32/src/PySID.cpp",
        "win32/src/PyTime.cpp",
        "win32/src/PyUnicode.cpp",
        "win32/src/PyWAVEFORMATEX.cpp",
        "win32/src/PyWinTypesmodule.cpp",
        ],
    depends = [
        "win32/src/PyWinObjects.h",
        "win32/src/PyWinTypes.h",
        "win32/src/PySoundObjects.h",
        "win32/src/PySecurityObjects.h",
        ],
    extra_compile_args = ['-DBUILD_PYWINTYPES'],
    libraries = "advapi32 user32 ole32 oleaut32",
    pch_header = "PyWinTypes.h",
    )

win32_extensions = [pywintypes]

win32_extensions.append(
    WinExt_win32("perfmondata",
        sources=[
            "win32/src/PerfMon/PyPerfMsgs.mc",
            "win32/src/PerfMon/perfmondata.cpp",
            ],
        libraries="advapi32",
        unicode_mode=True,
        export_symbol_file = "win32/src/PerfMon/perfmondata.def",
        is_regular_dll = 1,
        depends = [
            "win32/src/PerfMon/perfutil.h",
            "win32/src/PerfMon/PyPerfMonControl.h",
            ],
        ),
    )

for info in (
        # (name, libraries, UNICODE, WINVER, sources)
        ("mmapfile", "", None, None, "win32/src/mmapfilemodule.cpp"),
        ("odbc", "odbc32 odbccp32", None, None, "win32/src/odbc.cpp"),
        ("perfmon", "", True, None, """
            win32/src/PerfMon/MappingManager.cpp
            win32/src/PerfMon/PerfCounterDefn.cpp
            win32/src/PerfMon/PerfObjectType.cpp
            win32/src/PerfMon/PyPerfMon.cpp
            """),
        ("timer", "user32", None, None, "win32/src/timermodule.cpp"),
        ("win2kras", "rasapi32", None, 0x0500, "win32/src/win2krasmodule.cpp"),
        ("win32cred", "AdvAPI32 credui", True, 0x0501, 'win32/src/win32credmodule.cpp'),
        ("win32crypt", "Crypt32 Advapi32", True, 0x0500, """
            win32/src/win32crypt/win32cryptmodule.cpp
            win32/src/win32crypt/win32crypt_structs.cpp
            win32/src/win32crypt/PyCERTSTORE.cpp
            win32/src/win32crypt/PyCERT_CONTEXT.cpp
            win32/src/win32crypt/PyCRYPTHASH.cpp
            win32/src/win32crypt/PyCRYPTKEY.cpp
            win32/src/win32crypt/PyCRYPTMSG.cpp
            win32/src/win32crypt/PyCRYPTPROV.cpp
            win32/src/win32crypt/PyCTL_CONTEXT.cpp
            """),
        ("win32file", "", None, 0x0500, """
              win32/src/win32file.i
              win32/src/win32file_comm.cpp
              """),
        ("win32event", "user32", None, None, "win32/src/win32event.i"),
        ("win32clipboard", "gdi32 user32 shell32", None, None, "win32/src/win32clipboardmodule.cpp"),

        # win32gui handled below
        ("win32job", "user32", True, 0x0500, 'win32/src/win32job.i'),
        ("win32lz", "lz32", None, None, "win32/src/win32lzmodule.cpp"),
        ("win32net", "netapi32 advapi32", True, None, """
              win32/src/win32net/win32netfile.cpp    win32/src/win32net/win32netgroup.cpp
              win32/src/win32net/win32netmisc.cpp    win32/src/win32net/win32netmodule.cpp
              win32/src/win32net/win32netsession.cpp win32/src/win32net/win32netuse.cpp
              win32/src/win32net/win32netuser.cpp
              """),
        ("win32pdh", "", True, None, "win32/src/win32pdhmodule.cpp"),
        ("win32pipe", "", None, None, 'win32/src/win32pipe.i win32/src/win32popen.cpp'),
        ("win32print", "winspool user32 gdi32", None, 0x0500, "win32/src/win32print/win32print.cpp"),
        ("win32process", "advapi32 user32", None, 0x0500, "win32/src/win32process.i"),
        ("win32profile", "Userenv", True, None, 'win32/src/win32profilemodule.cpp'),
        ("win32ras", "rasapi32 user32", None, 0x0500, "win32/src/win32rasmodule.cpp"),
        ("win32security", "advapi32 user32 netapi32", True, 0x0500, """
            win32/src/win32security.i
            win32/src/win32security_sspi.cpp win32/src/win32security_ds.cpp
            """),
        ("win32service", "advapi32 oleaut32 user32", True, 0x0501, """
            win32/src/win32service_messages.mc
            win32/src/win32service.i
            """),
        ("win32trace", "advapi32", None, None, "win32/src/win32trace.cpp"),
        ("win32wnet", "netapi32 mpr", None, None, """
            win32/src/win32wnet/PyNCB.cpp
            win32/src/win32wnet/PyNetresource.cpp
            win32/src/win32wnet/win32wnet.cpp
            """),
        ("win32inet", "wininet", None, 0x500, """
            win32/src/win32inet.i
            win32/src/win32inet_winhttp.cpp
            """),
        ("win32console", "kernel32", True, 0x0501, "win32/src/win32consolemodule.cpp"),
        ("win32ts", "WtsApi32", True, 0x0501, "win32/src/win32tsmodule.cpp"),
        ("_win32sysloader", "", None, 0x0501, "win32/src/_win32sysloader.cpp"),
        ("win32transaction", "kernel32", True, 0x0501, "win32/src/win32transactionmodule.cpp"),

    ):

    name, lib_names, unicode_mode = info[:3]
    # unicode_mode == None means "not on py2.6, yes on py3", True means everywhere
    # False means nowhere.
    windows_h_ver = sources = None
    if len(info)>3:
        windows_h_ver = info[3]
    if len(info)>4:
        sources = info[4].split()
    extra_compile_args = []
    ext = WinExt_win32(name,
                 libraries=lib_names,
                 extra_compile_args = extra_compile_args,
                 windows_h_version = windows_h_ver,
                 sources = sources,
                 unicode_mode = unicode_mode)
    win32_extensions.append(ext)

# The few that need slightly special treatment
win32_extensions += [
    WinExt_win32("win32evtlog",
            sources = """
                win32\\src\\win32evtlog_messages.mc win32\\src\\win32evtlog.i
                """.split(),
                libraries="advapi32 oleaut32",
                delay_load_libraries="wevtapi",
                windows_h_version=0x0600
        ),
    WinExt_win32("win32api",
           sources = """
                win32/src/win32apimodule.cpp win32/src/win32api_display.cpp
                """.split(),
           libraries="user32 advapi32 shell32 version",
           delay_load_libraries="powrprof",
           windows_h_version=0x0500,
        ),
    WinExt_win32("win32gui",
           sources = """
                win32/src/win32dynamicdialog.cpp
                win32/src/win32gui.i
               """.split(),
           windows_h_version=0x0500,
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           define_macros = [("WIN32GUI", None)],
        ),
    # winxpgui is built from win32gui.i, but sets up different #defines before
    # including windows.h.  It also has an XP style manifest.
    WinExt_win32("winxpgui",
           sources = """
                win32/src/winxpgui.rc win32/src/win32dynamicdialog.cpp
                win32/src/win32gui.i
               """.split(),
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           windows_h_version=0x0500,
           define_macros = [("WIN32GUI",None), ("WINXPGUI",None)],
           extra_swig_commands=["-DWINXPGUI"],
        ),
    # winxptheme
    WinExt_win32("_winxptheme",
           sources = ["win32/src/_winxptheme.i"],
           libraries="gdi32 user32 comdlg32 comctl32 shell32 Uxtheme",
           windows_h_version=0x0500,
        ),
]
win32_extensions += [
    WinExt_win32('servicemanager',
           sources = ["win32/src/PythonServiceMessages.mc", "win32/src/PythonService.cpp"],
           extra_compile_args = ['-DPYSERVICE_BUILD_DLL'],
           libraries = "user32 ole32 advapi32 shell32",
           windows_h_version = 0x500,
           unicode_mode=True,),
]

win32_extensions += [
    WinExt_win32('win32help',
                 sources = ["win32/src/win32helpmodule.cpp"],
                 libraries="htmlhelp user32 advapi32",
                 windows_h_version = 0x500),
]

dirs = {
    'adsi' : 'com/win32comext/adsi/src',
    'propsys' : 'com/win32comext/propsys/src',
    'shell' : 'com/win32comext/shell/src',
    'axcontrol' : 'com/win32comext/axcontrol/src',
    'axdebug' : 'com/win32comext/axdebug/src',
    'axscript' : 'com/win32comext/axscript/src',
    'directsound' : 'com/win32comext/directsound/src',
    'ifilter' : 'com/win32comext/ifilter/src',
    'internet' : 'com/win32comext/internet/src',
    'mapi' : 'com/win32comext/mapi/src',
    'authorization' : 'com/win32comext/authorization/src',
    'taskscheduler' : 'com/win32comext/taskscheduler/src',
    'bits' : 'com/win32comext/bits/src',
    'win32com' : 'com/win32com/src',
}

# The COM modules.
pythoncom = WinExt_system32('pythoncom',
                   sources=("""
                        %(win32com)s/dllmain.cpp            %(win32com)s/ErrorUtils.cpp
                        %(win32com)s/MiscTypes.cpp          %(win32com)s/oleargs.cpp
                        %(win32com)s/PyComHelpers.cpp       %(win32com)s/PyFactory.cpp
                        %(win32com)s/PyGatewayBase.cpp      %(win32com)s/PyIBase.cpp
                        %(win32com)s/PyIClassFactory.cpp    %(win32com)s/PyIDispatch.cpp
                        %(win32com)s/PyIUnknown.cpp         %(win32com)s/PyRecord.cpp
                        %(win32com)s/extensions/PySTGMEDIUM.cpp %(win32com)s/PyStorage.cpp
                        %(win32com)s/PythonCOM.cpp          %(win32com)s/Register.cpp
                        %(win32com)s/stdafx.cpp             %(win32com)s/univgw.cpp
                        %(win32com)s/univgw_dataconv.cpp    %(win32com)s/extensions/PyFUNCDESC.cpp
                        %(win32com)s/extensions/PyGConnectionPoint.cpp      %(win32com)s/extensions/PyGConnectionPointContainer.cpp
                        %(win32com)s/extensions/PyGEnumVariant.cpp          %(win32com)s/extensions/PyGErrorLog.cpp
                        %(win32com)s/extensions/PyGPersist.cpp              %(win32com)s/extensions/PyGPersistPropertyBag.cpp
                        %(win32com)s/extensions/PyGPersistStorage.cpp       %(win32com)s/extensions/PyGPersistStream.cpp
                        %(win32com)s/extensions/PyGPersistStreamInit.cpp    %(win32com)s/extensions/PyGPropertyBag.cpp
                        %(win32com)s/extensions/PyGStream.cpp               %(win32com)s/extensions/PyIBindCtx.cpp
                        %(win32com)s/extensions/PyICatInformation.cpp       %(win32com)s/extensions/PyICatRegister.cpp
                        %(win32com)s/extensions/PyIConnectionPoint.cpp      %(win32com)s/extensions/PyIConnectionPointContainer.cpp
                        %(win32com)s/extensions/PyICreateTypeInfo.cpp       %(win32com)s/extensions/PyICreateTypeLib.cpp
                        %(win32com)s/extensions/PyICreateTypeLib2.cpp       %(win32com)s/extensions/PyIDataObject.cpp
                        %(win32com)s/extensions/PyIDropSource.cpp           %(win32com)s/extensions/PyIDropTarget.cpp
                        %(win32com)s/extensions/PyIEnumCATEGORYINFO.cpp     %(win32com)s/extensions/PyIEnumConnectionPoints.cpp
                        %(win32com)s/extensions/PyIEnumConnections.cpp      %(win32com)s/extensions/PyIEnumFORMATETC.cpp
                        %(win32com)s/extensions/PyIEnumGUID.cpp             %(win32com)s/extensions/PyIEnumSTATPROPSETSTG.cpp
                        %(win32com)s/extensions/PyIEnumSTATPROPSTG.cpp      %(win32com)s/extensions/PyIEnumSTATSTG.cpp
                        %(win32com)s/extensions/PyIEnumString.cpp           %(win32com)s/extensions/PyIEnumVARIANT.cpp
                        %(win32com)s/extensions/PyIErrorLog.cpp             %(win32com)s/extensions/PyIExternalConnection.cpp
                        %(win32com)s/extensions/PyIGlobalInterfaceTable.cpp %(win32com)s/extensions/PyILockBytes.cpp
                        %(win32com)s/extensions/PyIMoniker.cpp              %(win32com)s/extensions/PyIOleWindow.cpp
                        %(win32com)s/extensions/PyIPersist.cpp              %(win32com)s/extensions/PyIPersistFile.cpp
                        %(win32com)s/extensions/PyIPersistPropertyBag.cpp   %(win32com)s/extensions/PyIPersistStorage.cpp
                        %(win32com)s/extensions/PyIPersistStream.cpp        %(win32com)s/extensions/PyIPersistStreamInit.cpp
                        %(win32com)s/extensions/PyIPropertyBag.cpp          %(win32com)s/extensions/PyIPropertySetStorage.cpp
                        %(win32com)s/extensions/PyIPropertyStorage.cpp      %(win32com)s/extensions/PyIProvideClassInfo.cpp
                        %(win32com)s/extensions/PyIRunningObjectTable.cpp   %(win32com)s/extensions/PyIServiceProvider.cpp
                        %(win32com)s/extensions/PyIStorage.cpp              %(win32com)s/extensions/PyIStream.cpp
                        %(win32com)s/extensions/PyIType.cpp                 %(win32com)s/extensions/PyITypeObjects.cpp
                        %(win32com)s/extensions/PyTYPEATTR.cpp              %(win32com)s/extensions/PyVARDESC.cpp
                        %(win32com)s/extensions/PyICancelMethodCalls.cpp    %(win32com)s/extensions/PyIContext.cpp
                        %(win32com)s/extensions/PyIEnumContextProps.cpp     %(win32com)s/extensions/PyIClientSecurity.cpp
                        %(win32com)s/extensions/PyIServerSecurity.cpp
                        """ % dirs).split(),
                   depends=("""
                        %(win32com)s/include\\propbag.h          %(win32com)s/include\\PyComTypeObjects.h
                        %(win32com)s/include\\PyFactory.h        %(win32com)s/include\\PyGConnectionPoint.h
                        %(win32com)s/include\\PyGConnectionPointContainer.h
                        %(win32com)s/include\\PyGPersistStorage.h %(win32com)s/include\\PyIBindCtx.h
                        %(win32com)s/include\\PyICatInformation.h %(win32com)s/include\\PyICatRegister.h
                        %(win32com)s/include\\PyIDataObject.h    %(win32com)s/include\\PyIDropSource.h
                        %(win32com)s/include\\PyIDropTarget.h    %(win32com)s/include\\PyIEnumConnectionPoints.h
                        %(win32com)s/include\\PyIEnumConnections.h %(win32com)s/include\\PyIEnumFORMATETC.h
                        %(win32com)s/include\\PyIEnumGUID.h      %(win32com)s/include\\PyIEnumSTATPROPSETSTG.h
                        %(win32com)s/include\\PyIEnumSTATSTG.h   %(win32com)s/include\\PyIEnumString.h
                        %(win32com)s/include\\PyIEnumVARIANT.h   %(win32com)s/include\\PyIExternalConnection.h
                        %(win32com)s/include\\PyIGlobalInterfaceTable.h %(win32com)s/include\\PyILockBytes.h
                        %(win32com)s/include\\PyIMoniker.h       %(win32com)s/include\\PyIOleWindow.h
                        %(win32com)s/include\\PyIPersist.h       %(win32com)s/include\\PyIPersistFile.h
                        %(win32com)s/include\\PyIPersistStorage.h %(win32com)s/include\\PyIPersistStream.h
                        %(win32com)s/include\\PyIPersistStreamInit.h %(win32com)s/include\\PyIRunningObjectTable.h
                        %(win32com)s/include\\PyIStorage.h       %(win32com)s/include\\PyIStream.h
                        %(win32com)s/include\\PythonCOM.h        %(win32com)s/include\\PythonCOMRegister.h
                        %(win32com)s/include\\PythonCOMServer.h  %(win32com)s/include\\stdafx.h
                        %(win32com)s/include\\univgw_dataconv.h
                        %(win32com)s/include\\PyICancelMethodCalls.h    %(win32com)s/include\\PyIContext.h
                        %(win32com)s/include\\PyIEnumContextProps.h     %(win32com)s/include\\PyIClientSecurity.h
                        %(win32com)s/include\\PyIServerSecurity.h
                        """ % dirs).split(),
                   libraries = "oleaut32 ole32 user32 urlmon",
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   pch_header = "stdafx.h",
                   windows_h_version = 0x500,
                   base_address = dll_base_address,
                   )
dll_base_address += 0x80000 # pythoncom is large!
com_extensions = [pythoncom]
com_extensions += [
    WinExt_win32com('adsi', libraries="ACTIVEDS ADSIID user32 advapi32",
                    sources=("""
                        %(adsi)s/adsi.i                 %(adsi)s/adsi.cpp
                        %(adsi)s/PyIADsContainer.i      %(adsi)s/PyIADsContainer.cpp
                        %(adsi)s/PyIADsUser.i           %(adsi)s/PyIADsUser.cpp
                        %(adsi)s/PyIADsDeleteOps.i      %(adsi)s/PyIADsDeleteOps.cpp
                        %(adsi)s/PyIDirectoryObject.i   %(adsi)s/PyIDirectoryObject.cpp
                        %(adsi)s/PyIDirectorySearch.i   %(adsi)s/PyIDirectorySearch.cpp
                        %(adsi)s/PyIDsObjectPicker.i    %(adsi)s/PyIDsObjectPicker.cpp

                        %(adsi)s/adsilib.i
                        %(adsi)s/PyADSIUtil.cpp         %(adsi)s/PyDSOPObjects.cpp
                        %(adsi)s/PyIADs.cpp
                        """ % dirs).split()),
    WinExt_win32com('axcontrol', pch_header="axcontrol_pch.h",
                    sources=("""
                        %(axcontrol)s/AXControl.cpp
                        %(axcontrol)s/PyIOleControl.cpp          %(axcontrol)s/PyIOleControlSite.cpp
                        %(axcontrol)s/PyIOleInPlaceActiveObject.cpp
                        %(axcontrol)s/PyIOleInPlaceSiteEx.cpp    %(axcontrol)s/PyISpecifyPropertyPages.cpp
                        %(axcontrol)s/PyIOleInPlaceUIWindow.cpp  %(axcontrol)s/PyIOleInPlaceFrame.cpp
                        %(axcontrol)s/PyIObjectWithSite.cpp      %(axcontrol)s/PyIOleInPlaceObject.cpp
                        %(axcontrol)s/PyIOleInPlaceSiteWindowless.cpp  %(axcontrol)s/PyIViewObject.cpp
                        %(axcontrol)s/PyIOleClientSite.cpp       %(axcontrol)s/PyIOleInPlaceSite.cpp
                        %(axcontrol)s/PyIOleObject.cpp           %(axcontrol)s/PyIViewObject2.cpp
                        %(axcontrol)s/PyIOleCommandTarget.cpp
                        """ % dirs).split()),
    WinExt_win32com('axscript',
                    sources=("""
                        %(axscript)s/AXScript.cpp
                        %(axscript)s/GUIDS.CPP                   %(axscript)s/PyGActiveScript.cpp
                        %(axscript)s/PyGActiveScriptError.cpp    %(axscript)s/PyGActiveScriptParse.cpp
                        %(axscript)s/PyGActiveScriptSite.cpp     %(axscript)s/PyGObjectSafety.cpp
                        %(axscript)s/PyIActiveScript.cpp         %(axscript)s/PyIActiveScriptError.cpp
                        %(axscript)s/PyIActiveScriptParse.cpp    %(axscript)s/PyIActiveScriptParseProcedure.cpp
                        %(axscript)s/PyIActiveScriptSite.cpp     %(axscript)s/PyIMultiInfos.cpp
                        %(axscript)s/PyIObjectSafety.cpp         %(axscript)s/stdafx.cpp
                        """ % dirs).split(),
                    depends=("""
                             %(axscript)s/AXScript.h
                             %(axscript)s/guids.h                %(axscript)s/PyGActiveScriptError.h
                             %(axscript)s/PyIActiveScriptError.h %(axscript)s/PyIObjectSafety.h
                             %(axscript)s/PyIProvideMultipleClassInfo.h
                             %(axscript)s/stdafx.h
                             """ % dirs).split(),
                    extra_compile_args = ['-DPY_BUILD_AXSCRIPT'],
                    implib_name="axscript",
                    pch_header = "stdafx.h"
    ),
    WinExt_win32com('axdebug',
            libraries="axscript",
            pch_header="stdafx.h",
            sources=("""
                    %(axdebug)s/AXDebug.cpp
                    %(axdebug)s/PyIActiveScriptDebug.cpp
                    %(axdebug)s/PyIActiveScriptErrorDebug.cpp
                    %(axdebug)s/PyIActiveScriptSiteDebug.cpp
                    %(axdebug)s/PyIApplicationDebugger.cpp
                    %(axdebug)s/PyIDebugApplication.cpp
                    %(axdebug)s/PyIDebugApplicationNode.cpp
                    %(axdebug)s/PyIDebugApplicationNodeEvents.cpp
                    %(axdebug)s/PyIDebugApplicationThread.cpp
                    %(axdebug)s/PyIDebugCodeContext.cpp
                    %(axdebug)s/PyIDebugDocument.cpp
                    %(axdebug)s/PyIDebugDocumentContext.cpp
                    %(axdebug)s/PyIDebugDocumentHelper.cpp
                    %(axdebug)s/PyIDebugDocumentHost.cpp
                    %(axdebug)s/PyIDebugDocumentInfo.cpp
                    %(axdebug)s/PyIDebugDocumentProvider.cpp
                    %(axdebug)s/PyIDebugDocumentText.cpp
                    %(axdebug)s/PyIDebugDocumentTextAuthor.cpp
                    %(axdebug)s/PyIDebugDocumentTextEvents.cpp
                    %(axdebug)s/PyIDebugDocumentTextExternalAuthor.cpp
                    %(axdebug)s/PyIDebugExpression.cpp
                    %(axdebug)s/PyIDebugExpressionCallBack.cpp
                    %(axdebug)s/PyIDebugExpressionContext.cpp
                    %(axdebug)s/PyIDebugProperties.cpp
                    %(axdebug)s/PyIDebugSessionProvider.cpp
                    %(axdebug)s/PyIDebugStackFrame.cpp
                    %(axdebug)s/PyIDebugStackFrameSniffer.cpp
                    %(axdebug)s/PyIDebugStackFrameSnifferEx.cpp
                    %(axdebug)s/PyIDebugSyncOperation.cpp
                    %(axdebug)s/PyIEnumDebugApplicationNodes.cpp
                    %(axdebug)s/PyIEnumDebugCodeContexts.cpp
                    %(axdebug)s/PyIEnumDebugExpressionContexts.cpp
                    %(axdebug)s/PyIEnumDebugPropertyInfo.cpp
                    %(axdebug)s/PyIEnumDebugStackFrames.cpp
                    %(axdebug)s/PyIEnumRemoteDebugApplications.cpp
                    %(axdebug)s/PyIEnumRemoteDebugApplicationThreads.cpp
                    %(axdebug)s/PyIMachineDebugManager.cpp
                    %(axdebug)s/PyIMachineDebugManagerEvents.cpp
                    %(axdebug)s/PyIProcessDebugManager.cpp
                    %(axdebug)s/PyIProvideExpressionContexts.cpp
                    %(axdebug)s/PyIRemoteDebugApplication.cpp
                    %(axdebug)s/PyIRemoteDebugApplicationEvents.cpp
                    %(axdebug)s/PyIRemoteDebugApplicationThread.cpp
                    %(axdebug)s/stdafx.cpp
                     """ % dirs).split(),
    ),
    WinExt_win32com('internet', pch_header="internet_pch.h",
                    sources=("""
                        %(internet)s/internet.cpp                   %(internet)s/PyIDocHostUIHandler.cpp
                        %(internet)s/PyIHTMLOMWindowServices.cpp    %(internet)s/PyIInternetBindInfo.cpp
                        %(internet)s/PyIInternetPriority.cpp        %(internet)s/PyIInternetProtocol.cpp
                        %(internet)s/PyIInternetProtocolInfo.cpp    %(internet)s/PyIInternetProtocolRoot.cpp
                        %(internet)s/PyIInternetProtocolSink.cpp    %(internet)s/PyIInternetSecurityManager.cpp
                    """ % dirs).split(),
                    depends=["%(internet)s/internet_pch.h" % dirs]),
    WinExt_win32com('mapi', libraries="advapi32", pch_header="PythonCOM.h",
                    include_dirs=["%(mapi)s/mapi_headers" % dirs],
                    sources=("""
                        %(mapi)s/mapi.i                 %(mapi)s/mapi.cpp
                        %(mapi)s/PyIABContainer.i       %(mapi)s/PyIABContainer.cpp
                        %(mapi)s/PyIAddrBook.i          %(mapi)s/PyIAddrBook.cpp
                        %(mapi)s/PyIAttach.i            %(mapi)s/PyIAttach.cpp
                        %(mapi)s/PyIDistList.i          %(mapi)s/PyIDistList.cpp
                        %(mapi)s/PyIMailUser.i          %(mapi)s/PyIMailUser.cpp
                        %(mapi)s/PyIMAPIContainer.i     %(mapi)s/PyIMAPIContainer.cpp
                        %(mapi)s/PyIMAPIFolder.i        %(mapi)s/PyIMAPIFolder.cpp
                        %(mapi)s/PyIMAPIProp.i          %(mapi)s/PyIMAPIProp.cpp
                        %(mapi)s/PyIMAPISession.i       %(mapi)s/PyIMAPISession.cpp
                        %(mapi)s/PyIMAPIStatus.i        %(mapi)s/PyIMAPIStatus.cpp
                        %(mapi)s/PyIMAPITable.i         %(mapi)s/PyIMAPITable.cpp
                        %(mapi)s/PyIMessage.i           %(mapi)s/PyIMessage.cpp
                        %(mapi)s/PyIMsgServiceAdmin.i   %(mapi)s/PyIMsgServiceAdmin.cpp
                        %(mapi)s/PyIMsgServiceAdmin2.i  %(mapi)s/PyIMsgServiceAdmin2.cpp
                        %(mapi)s/PyIProviderAdmin.i     %(mapi)s/PyIProviderAdmin.cpp
                        %(mapi)s/PyIMsgStore.i          %(mapi)s/PyIMsgStore.cpp
                        %(mapi)s/PyIProfAdmin.i         %(mapi)s/PyIProfAdmin.cpp
                        %(mapi)s/PyIProfSect.i          %(mapi)s/PyIProfSect.cpp
                        %(mapi)s/PyIConverterSession.i	%(mapi)s/PyIConverterSession.cpp
                        %(mapi)s/PyIMAPIAdviseSink.cpp
                        %(mapi)s/mapiutil.cpp
                        %(mapi)s/mapiguids.cpp
                        %(mapi)s/mapi_stub_library/MapiStubLibrary.cpp
                        %(mapi)s/mapi_stub_library/StubUtils.cpp
                        """ % dirs).split()),
    WinExt_win32com_mapi('exchange', libraries="advapi32",
                         include_dirs=["%(mapi)s/mapi_headers" % dirs],
                         sources=("""
                                  %(mapi)s/exchange.i         %(mapi)s/exchange.cpp
                                  %(mapi)s/PyIExchangeManageStore.i %(mapi)s/PyIExchangeManageStore.cpp
                                  %(mapi)s/PyIExchangeManageStoreEx.i %(mapi)s/PyIExchangeManageStoreEx.cpp
                                  %(mapi)s/mapiutil.cpp
                                  %(mapi)s/exchangeguids.cpp
                                  %(mapi)s/mapi_stub_library/MapiStubLibrary.cpp
                                  %(mapi)s/mapi_stub_library/StubUtils.cpp
                                  """ % dirs).split()),
    WinExt_win32com_mapi('exchdapi', libraries="advapi32",
                         include_dirs=["%(mapi)s/mapi_headers" % dirs],
                         sources=("""
                                  %(mapi)s/exchdapi.i         %(mapi)s/exchdapi.cpp
                                  %(mapi)s/mapi_stub_library/MapiStubLibrary.cpp
                                  %(mapi)s/mapi_stub_library/StubUtils.cpp
                                  """ % dirs).split()),
    WinExt_win32com('shell', libraries='shell32', pch_header="shell_pch.h",
                    windows_h_version = 0x600,
                    sources=("""
                        %(shell)s/PyIActiveDesktop.cpp
                        %(shell)s/PyIApplicationDestinations.cpp
                        %(shell)s/PyIApplicationDocumentLists.cpp
                        %(shell)s/PyIAsyncOperation.cpp
                        %(shell)s/PyIBrowserFrameOptions.cpp
                        %(shell)s/PyICategorizer.cpp
                        %(shell)s/PyICategoryProvider.cpp
                        %(shell)s/PyIColumnProvider.cpp
                        %(shell)s/PyIContextMenu.cpp
                        %(shell)s/PyIContextMenu2.cpp
                        %(shell)s/PyIContextMenu3.cpp
                        %(shell)s/PyICopyHook.cpp
                        %(shell)s/PyICurrentItem.cpp
                        %(shell)s/PyICustomDestinationList.cpp
                        %(shell)s/PyIDefaultExtractIconInit.cpp
                        %(shell)s/PyIDeskBand.cpp
                        %(shell)s/PyIDisplayItem.cpp
                        %(shell)s/PyIDockingWindow.cpp
                        %(shell)s/PyIDropTargetHelper.cpp
                        %(shell)s/PyIEnumExplorerCommand.cpp
                        %(shell)s/PyIEnumIDList.cpp
                        %(shell)s/PyIEnumObjects.cpp
                        %(shell)s/PyIEnumResources.cpp
                        %(shell)s/PyIEnumShellItems.cpp
                        %(shell)s/PyIEmptyVolumeCache.cpp
                        %(shell)s/PyIEmptyVolumeCacheCallBack.cpp
                        %(shell)s/PyIExplorerBrowser.cpp
                        %(shell)s/PyIExplorerBrowserEvents.cpp
                        %(shell)s/PyIExplorerCommand.cpp
                        %(shell)s/PyIExplorerCommandProvider.cpp
                        %(shell)s/PyIExplorerPaneVisibility.cpp
                        %(shell)s/PyIExtractIcon.cpp
                        %(shell)s/PyIExtractIconW.cpp
                        %(shell)s/PyIExtractImage.cpp
                        %(shell)s/PyIFileOperation.cpp
                        %(shell)s/PyIFileOperationProgressSink.cpp
                        %(shell)s/PyIIdentityName.cpp
                        %(shell)s/PyIInputObject.cpp
                        %(shell)s/PyIKnownFolder.cpp
                        %(shell)s/PyIKnownFolderManager.cpp
                        %(shell)s/PyINameSpaceTreeControl.cpp
                        %(shell)s/PyIObjectArray.cpp
                        %(shell)s/PyIObjectCollection.cpp
                        %(shell)s/PyIPersistFolder.cpp
                        %(shell)s/PyIPersistFolder2.cpp
                        %(shell)s/PyIQueryAssociations.cpp
                        %(shell)s/PyIRelatedItem.cpp
                        %(shell)s/PyIShellBrowser.cpp
                        %(shell)s/PyIShellExtInit.cpp
                        %(shell)s/PyIShellFolder.cpp
                        %(shell)s/PyIShellFolder2.cpp
                        %(shell)s/PyIShellIcon.cpp
                        %(shell)s/PyIShellIconOverlay.cpp
                        %(shell)s/PyIShellIconOverlayIdentifier.cpp
                        %(shell)s/PyIShellIconOverlayManager.cpp
                        %(shell)s/PyIShellItem.cpp
                        %(shell)s/PyIShellItem2.cpp
                        %(shell)s/PyIShellItemArray.cpp
                        %(shell)s/PyIShellItemResources.cpp
                        %(shell)s/PyIShellLibrary.cpp
                        %(shell)s/PyIShellLink.cpp
                        %(shell)s/PyIShellLinkDataList.cpp
                        %(shell)s/PyIShellView.cpp
                        %(shell)s/PyITaskbarList.cpp
                        %(shell)s/PyITransferAdviseSink.cpp
                        %(shell)s/PyITransferDestination.cpp
                        %(shell)s/PyITransferMediumItem.cpp
                        %(shell)s/PyITransferSource.cpp
                        %(shell)s/PyIUniformResourceLocator.cpp
                        %(shell)s/shell.cpp

                        """ % dirs).split()),

    WinExt_win32com('propsys', libraries='propsys', delay_load_libraries='shell32',
                    unicode_mode=True,
                    sources=("""
                        %(propsys)s/propsys.cpp
                        %(propsys)s/PyIInitializeWithFile.cpp
                        %(propsys)s/PyIInitializeWithStream.cpp
                        %(propsys)s/PyINamedPropertyStore.cpp
                        %(propsys)s/PyIPropertyDescription.cpp
                        %(propsys)s/PyIPropertyDescriptionAliasInfo.cpp
                        %(propsys)s/PyIPropertyDescriptionList.cpp
                        %(propsys)s/PyIPropertyDescriptionSearchInfo.cpp
                        %(propsys)s/PyIPropertyEnumType.cpp
                        %(propsys)s/PyIPropertyEnumTypeList.cpp
                        %(propsys)s/PyIPropertyStore.cpp
                        %(propsys)s/PyIPropertyStoreCache.cpp
                        %(propsys)s/PyIPropertyStoreCapabilities.cpp
                        %(propsys)s/PyIPropertySystem.cpp
                        %(propsys)s/PyPROPVARIANT.cpp
                        %(propsys)s/PyIPersistSerializedPropStorage.cpp
                        %(propsys)s/PyIObjectWithPropertyKey.cpp
                        %(propsys)s/PyIPropertyChange.cpp
                        %(propsys)s/PyIPropertyChangeArray.cpp
                        """ % dirs).split(),
                    implib_name="pypropsys",
                    ),


    WinExt_win32com('taskscheduler', libraries='mstask',
                    sources=("""
                        %(taskscheduler)s/taskscheduler.cpp
                        %(taskscheduler)s/PyIProvideTaskPage.cpp
                        %(taskscheduler)s/PyIScheduledWorkItem.cpp
                        %(taskscheduler)s/PyITask.cpp
                        %(taskscheduler)s/PyITaskScheduler.cpp
                        %(taskscheduler)s/PyITaskTrigger.cpp

                        """ % dirs).split()),
    WinExt_win32com('bits', libraries='Bits', pch_header="bits_pch.h",
                    sources=("""
                        %(bits)s/bits.cpp
                        %(bits)s/PyIBackgroundCopyManager.cpp
                        %(bits)s/PyIBackgroundCopyCallback.cpp
                        %(bits)s/PyIBackgroundCopyError.cpp
                        %(bits)s/PyIBackgroundCopyJob.cpp
                        %(bits)s/PyIBackgroundCopyJob2.cpp
                        %(bits)s/PyIBackgroundCopyJob3.cpp
                        %(bits)s/PyIBackgroundCopyFile.cpp
                        %(bits)s/PyIBackgroundCopyFile2.cpp
                        %(bits)s/PyIEnumBackgroundCopyJobs.cpp
                        %(bits)s/PyIEnumBackgroundCopyFiles.cpp

                        """ % dirs).split()),
    WinExt_win32com('ifilter', libraries='ntquery',
                    sources=("%(ifilter)s/PyIFilter.cpp" % dirs).split(),
                    depends=("%(ifilter)s/PyIFilter.h %(ifilter)s/stdafx.h" % dirs).split(),
                    ),
    WinExt_win32com('directsound', pch_header='directsound_pch.h',
                    sources=("""
                        %(directsound)s/directsound.cpp     %(directsound)s/PyDSBCAPS.cpp
                        %(directsound)s/PyDSBUFFERDESC.cpp  %(directsound)s/PyDSCAPS.cpp
                        %(directsound)s/PyDSCBCAPS.cpp      %(directsound)s/PyDSCBUFFERDESC.cpp
                        %(directsound)s/PyDSCCAPS.cpp       %(directsound)s/PyIDirectSound.cpp
                        %(directsound)s/PyIDirectSoundBuffer.cpp %(directsound)s/PyIDirectSoundCapture.cpp
                        %(directsound)s/PyIDirectSoundCaptureBuffer.cpp
                        %(directsound)s/PyIDirectSoundNotify.cpp
                        """ % dirs).split(),
                    depends=("""
                        %(directsound)s/directsound_pch.h   %(directsound)s/PyIDirectSound.h
                        %(directsound)s/PyIDirectSoundBuffer.h %(directsound)s/PyIDirectSoundCapture.h
                        %(directsound)s/PyIDirectSoundCaptureBuffer.h %(directsound)s/PyIDirectSoundNotify.h
                        """ % dirs).split(),
                    optional_headers = ['dsound.h'],
                    libraries='user32 dsound dxguid'),
    WinExt_win32com('authorization', libraries='aclui advapi32',
                    sources=("""
                        %(authorization)s/authorization.cpp
                        %(authorization)s/PyGSecurityInformation.cpp
                        """ % dirs).split()),
]

pythonwin_extensions = [
    WinExt_pythonwin("win32ui",
        sources = [
            "Pythonwin/dbgthread.cpp",
            "Pythonwin/dibapi.cpp",
            "Pythonwin/dllmain.cpp",
            "Pythonwin/pythondoc.cpp",
            "Pythonwin/pythonppage.cpp",
            "Pythonwin/pythonpsheet.cpp",
            "Pythonwin/pythonRichEditCntr.cpp",
            "Pythonwin/pythonRichEditDoc.cpp",
            "Pythonwin/pythonview.cpp",
            "Pythonwin/stdafx.cpp",
            "Pythonwin/win32app.cpp",
            "Pythonwin/win32assoc.cpp",
            "Pythonwin/win32bitmap.cpp",
            "Pythonwin/win32brush.cpp",
            "Pythonwin/win32cmd.cpp",
            "Pythonwin/win32cmdui.cpp",
            "Pythonwin/win32context.cpp",
            "Pythonwin/win32control.cpp",
            "Pythonwin/win32ctledit.cpp",
            "Pythonwin/win32ctrlList.cpp",
            "Pythonwin/win32ctrlRichEdit.cpp",
            "Pythonwin/win32ctrlTree.cpp",
            "Pythonwin/win32dc.cpp",
            "Pythonwin/win32dlg.cpp",
            "Pythonwin/win32dlgbar.cpp",
            "Pythonwin/win32dll.cpp",
            "Pythonwin/win32doc.cpp",
            "win32/src/win32dynamicdialog.cpp",
            "Pythonwin/win32font.cpp",
            "Pythonwin/win32gdi.cpp",
            "Pythonwin/win32ImageList.cpp",
            "Pythonwin/win32menu.cpp",
            "Pythonwin/win32notify.cpp",
            "Pythonwin/win32pen.cpp",
            "Pythonwin/win32prinfo.cpp",
            "Pythonwin/win32prop.cpp",
            "Pythonwin/win32rgn.cpp",
            "Pythonwin/win32RichEdit.cpp",
            "Pythonwin/win32RichEditDocTemplate.cpp",
            "Pythonwin/win32splitter.cpp",
            "Pythonwin/win32template.cpp",
            "Pythonwin/win32thread.cpp",
            "Pythonwin/win32toolbar.cpp",
            "Pythonwin/win32tooltip.cpp",
            "Pythonwin/win32ui.rc",
            "Pythonwin/win32uimodule.cpp",
            "Pythonwin/win32util.cpp",
            "Pythonwin/win32view.cpp",
            "Pythonwin/win32virt.cpp",
            "Pythonwin/win32win.cpp",
            ],
        extra_compile_args = ['-DBUILD_PYW'],
        pch_header="stdafx.h", base_address=dll_base_address,
        depends = [
            "Pythonwin/stdafx.h",
            "Pythonwin/win32uiExt.h",
            "Pythonwin/dibapi.h",
            "Pythonwin/pythoncbar.h",
            "Pythonwin/pythondoc.h",
            "Pythonwin/pythonframe.h",
            "Pythonwin/pythonppage.h",
            "Pythonwin/pythonpsheet.h",
            "Pythonwin/pythonRichEdit.h",
            "Pythonwin/pythonRichEditCntr.h",
            "Pythonwin/pythonRichEditDoc.h",
            "Pythonwin/pythonview.h",
            "Pythonwin/pythonwin.h",
            "Pythonwin/Win32app.h",
            "Pythonwin/win32assoc.h",
            "Pythonwin/win32bitmap.h",
            "Pythonwin/win32brush.h",
            "Pythonwin/win32cmd.h",
            "Pythonwin/win32cmdui.h",
            "Pythonwin/win32control.h",
            "Pythonwin/win32ctrlList.h",
            "Pythonwin/win32ctrlTree.h",
            "Pythonwin/win32dc.h",
            "Pythonwin/win32dlg.h",
            "Pythonwin/win32dlgbar.h",
            "win32/src/win32dynamicdialog.h",
            "Pythonwin/win32dll.h",
            "Pythonwin/win32doc.h",
            "Pythonwin/win32font.h",
            "Pythonwin/win32gdi.h",
            "Pythonwin/win32hl.h",
            "Pythonwin/win32ImageList.h",
            "Pythonwin/win32menu.h",
            "Pythonwin/win32pen.h",
            "Pythonwin/win32prinfo.h",
            "Pythonwin/win32prop.h",
            "Pythonwin/win32rgn.h",
            "Pythonwin/win32RichEdit.h",
            "Pythonwin/win32RichEditDocTemplate.h",
            "Pythonwin/win32splitter.h",
            "Pythonwin/win32template.h",
            "Pythonwin/win32toolbar.h",
            "Pythonwin/win32ui.h",
            "Pythonwin/Win32uiHostGlue.h",
            "Pythonwin/win32win.h",
            ],
        optional_headers=['afxres.h']),
    WinExt_pythonwin("win32uiole",
        sources = [
            "Pythonwin/stdafxole.cpp",
            "Pythonwin/win32oleDlgInsert.cpp",
            "Pythonwin/win32oleDlgs.cpp",
            "Pythonwin/win32uiole.cpp",
            "Pythonwin/win32uioleClientItem.cpp",
            "Pythonwin/win32uioledoc.cpp",
            ],
        depends = [
            "Pythonwin/stdafxole.h",
            "Pythonwin/win32oleDlgs.h",
            "Pythonwin/win32uioledoc.h",
            ],
        pch_header="stdafxole.h",
        windows_h_version = 0x500,
        optional_headers=['afxres.h']),
    WinExt_pythonwin("dde",
        sources = [
            "Pythonwin/stddde.cpp",
            "Pythonwin/ddetopic.cpp",
            "Pythonwin/ddeconv.cpp",
            "Pythonwin/ddeitem.cpp",
            "Pythonwin/ddemodule.cpp",
            "Pythonwin/ddeserver.cpp",
            ],
        pch_header="stdafxdde.h",
        depends=["win32/src/stddde.h", "pythonwin/ddemodule.h"],
        optional_headers=['afxres.h']),
    ]
# win32ui is large, so we reserve more bytes than normal
dll_base_address += 0x100000

other_extensions = []
other_extensions.append(
    WinExt_ISAPI('PyISAPI_loader',
       sources=[os.path.join("isapi", "src", s) for s in
               """PyExtensionObjects.cpp PyFilterObjects.cpp
                  pyISAPI.cpp pyISAPI_messages.mc
                  PythonEng.cpp StdAfx.cpp Utils.cpp
               """.split()],
       # We keep pyISAPI_messages.h out of the depends list, as it is
       # generated and we aren't smart enough to say *only* the .cpp etc
       # depend on it - so the generated .h says the .mc needs to be
       # rebuilt, which re-creates the .h...
       depends=[os.path.join("isapi", "src", s) for s in
               """ControlBlock.h FilterContext.h PyExtensionObjects.h
                  PyFilterObjects.h pyISAPI.h
                  PythonEng.h StdAfx.h Utils.h
               """.split()],
       pch_header = "StdAfx.h",
       is_regular_dll = 1,
       export_symbols = """HttpExtensionProc GetExtensionVersion
                           TerminateExtension GetFilterVersion
                           HttpFilterProc TerminateFilter
                           PyISAPISetOptions WriteEventLogMessage
                           """.split(),
       libraries='advapi32',
       )
)

W32_exe_files = [
    WinExt_win32_subsys_con("pythonservice",
         sources=[os.path.join("win32", "src", s) for s in
                  "PythonService.cpp PythonService.rc".split()],
         unicode_mode = True,
         libraries = "user32 advapi32 ole32 shell32"),
    WinExt_pythonwin_subsys_win("Pythonwin",
        sources = [
            "Pythonwin/pythonwin.cpp",
            "Pythonwin/pythonwin.rc",
            "Pythonwin/stdafxpw.cpp",
            ],
        optional_headers=['afxres.h']),
]

# Special definitions for SWIG.
swig_interface_parents = {
    # source file base,     "base class" for generated COM support
    'mapi':                 None, # not a class, but module
    'PyIMailUser':          'IMAPIContainer',
    'PyIABContainer':       'IMAPIContainer',
    'PyIAddrBook':          'IMAPIProp',
    'PyIAttach':            'IMAPIProp',
    'PyIDistList':          'IMAPIContainer',
    'PyIMailUser':          'IMAPIContainer',
    'PyIMAPIContainer':     'IMAPIProp',
    'PyIMAPIFolder':        'IMAPIContainer',
    'PyIMAPIProp':          '', # '' == default base
    'PyIMAPISession':       '',
    'PyIMAPIStatus':       'IMAPIProp',
    'PyIMAPITable':         '',
    'PyIMessage':           'IMAPIProp',
    'PyIMsgServiceAdmin':   '',
    'PyIMsgServiceAdmin2':  'IMsgServiceAdmin',
    'PyIProviderAdmin':     '',
    'PyIMsgStore':          'IMAPIProp',
    'PyIProfAdmin':         '',
    'PyIProfSect':          'IMAPIProp',
	'PyIConverterSession':	'',
    # exchange and exchdapi
    'exchange':             None,
    'exchdapi':             None,
    'PyIExchangeManageStore': '',
    'PyIExchangeManageStoreEx': '',
    # ADSI
    'adsi':                 None, # module
    'PyIADsContainer':      'IDispatch',
    'PyIADsDeleteOps':      'IDispatch',
    'PyIADsUser':           'IADs',
    'PyIDirectoryObject':   '',
    'PyIDirectorySearch':   '',
    'PyIDsObjectPicker':   '',
    'PyIADs':   'IDispatch',
}

# .i files that are #included, and hence are not part of the build.  Our .dsp
# parser isn't smart enough to differentiate these.
swig_include_files = "mapilib adsilib".split()

# Helper to allow our script specifications to include wildcards.
def expand_modules(module_dir):
    flist = FileList()
    flist.findall(module_dir)
    flist.include_pattern("*.py", anchor=0)
    return [os.path.splitext(name)[0] for name in flist.files]

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
        except RuntimeError as details:
            if not str(details.args[0]).startswith("No file"):
                raise
            log.info('NOTE: Optional file %s not found - skipping' % file)
        else:
            ret.append(temp[0])
    return ret

################################################################
if len(sys.argv)==1:
    # distutils will print usage - print our docstring first.
    print(__doc__)
    print("Standard usage information follows:")

packages=['win32com',
          'win32com.client',
          'win32com.demos',
          'win32com.makegw',
          'win32com.server',
          'win32com.servers',
          'win32com.test',

          'win32comext.adsi',

          'win32comext.axscript',
          'win32comext.axscript.client',
          'win32comext.axscript.server',

          'win32comext.axdebug',

          'win32comext.propsys',
          'win32comext.shell',
          'win32comext.mapi',
          'win32comext.ifilter',
          'win32comext.internet',
          'win32comext.axcontrol',
          'win32comext.taskscheduler',
          'win32comext.directsound',
          'win32comext.directsound.test',
          'win32comext.authorization',
          'win32comext.bits',

          'pythonwin.pywin',
          'pythonwin.pywin.debugger',
          'pythonwin.pywin.dialogs',
          'pythonwin.pywin.docking',
          'pythonwin.pywin.framework',
          'pythonwin.pywin.framework.editor',
          'pythonwin.pywin.framework.editor.color',
          'pythonwin.pywin.idle',
          'pythonwin.pywin.mfc',
          'pythonwin.pywin.scintilla',
          'pythonwin.pywin.tools',
          'isapi',
          'adodbapi',
          ]

py_modules = expand_modules("win32\\lib")
ext_modules = win32_extensions + com_extensions + pythonwin_extensions + \
                    other_extensions

# Build a map of DLL base addresses.  According to Python's PC\dllbase_nt.txt,
# we start at 0x1e200000 and go up in 0x00020000 increments.  A couple of
# our modules just go over this limit, so we use 30000.  We also do it sorted
# so each module gets the same addy each build.
# Note: If a module specifies a base address it still gets a slot reserved
# here which is unused.  We can live with that tho.
names = [ext.name for ext in ext_modules]
names.sort()
dll_base_addresses = {}
for name in names:
    dll_base_addresses[name] = dll_base_address
    dll_base_address += 0x30000


cmdclass = { 'install': my_install,
             'build': my_build,
             'build_ext': my_build_ext,
             'install_data': my_install_data,
             'build_py' : my_build_py,
             'build_scripts' : my_build_scripts,
           }

classifiers = [ 'Environment :: Win32 (MS Windows)',
	            'Intended Audience :: Developers',
	            'License :: OSI Approved :: Python Software Foundation License',
	            'Operating System :: Microsoft :: Windows',
	            'Programming Language :: Python :: 2.7',
	            'Programming Language :: Python :: 3.5',
	            'Programming Language :: Python :: 3.6',
	            'Programming Language :: Python :: 3.7',
	            'Programming Language :: Python :: Implementation :: CPython',
	          ]

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
      classifiers = classifiers,
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
            print("*** NOTE: The following extensions were NOT %s:" % what_string)
            for ext, why in excluded_extensions:
                print(" %s: %s" % (ext.name, why))
            print("For more details on installing the correct libraries and headers,")
            print("please execute this script with no arguments (or see the docstring)")
        else:
            print("All extension modules %s OK" % (what_string,))
