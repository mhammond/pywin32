build_id="212.1" # may optionally include a ".{patchno}" suffix.
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
Currently, the Vista SDK and DirectX SDK are required to successfully build
all extension modules - note that using the Vista SDK doesn't force you to
use Vista as your build environment.  Please use google to find the SDK -
links to microsoft.com seem to only stay current for a short time.  Also note
that the SDK that comes with VS2008 is not good enough to compile earlier
versions.  If you installed VS2008 after the Vista SDK, try doing a 'repair'
on your SDK install.

Early versions of certain Windows headers/SDK versions will also cause
certain modules to be skipped. If you don't use the extensions that fail to
build, you can ignore these warnings; if you do use them, you must install
the correct libraries.

The 'exchange' extensions require headers that are no longer in any current
SDKs, so these fail to build, but the 'mapi' extension should still build.
Also, 'axdebug' once worked with recent Platform SDKs, with no
additional software was needed - but currently this extension is known to not
build.

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
"""
# Originally by Thomas Heller, started in 2000 or so.
import os, string, sys
import types, glob
import re
import _winreg

# Python version compatibility hacks
try:
    True; False
except NameError:
    True=0==0
    False=1==0

# The rest of our imports.
from distutils.core import setup, Extension, Command
from distutils.command.install import install
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.command.build import build
from distutils.command.install_data import install_data
try:
    from distutils.command.bdist_msi import bdist_msi
except ImportError:
    # py23 and earlier
    bdist_msi = None
try:
    from distutils.msvccompiler import get_build_version
except ImportError:
    # py22
    def get_build_version():
        return 6.0
# nor distutils.log
try:
    from distutils import log
except ImportError:
    class Log:
        def debug(self, msg, *args):
            print msg % args
        def info(self, msg, *args):
            print msg % args
    log = Log()


from distutils.dep_util import newer_group, newer
from distutils import dir_util, file_util
from distutils.sysconfig import get_python_lib
from distutils.filelist import FileList
from distutils.errors import DistutilsExecError
import distutils.util

build_id_patch = build_id
if not "." in build_id_patch:
    build_id_patch = build_id_patch + ".0"
pywin32_version="%d.%d.%s" % (sys.version_info[0], sys.version_info[1],
                              build_id_patch)
print "Building pywin32", pywin32_version

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
    # Finding the Platform SDK install dir is a treat. There can be some
    # dead ends so we only consider the job done if we find the "windows.h"
    # landmark.
    DEBUG = False # can't use log.debug - not setup yet
    landmark = "include\\windows.h"
    # 1. The use might have their current environment setup for the
    #    SDK, in which case the "MSSdk" env var is set.
    sdkdir = os.environ.get("MSSdk")
    if sdkdir:
        if DEBUG:
            print "PSDK: try %MSSdk%: '%s'" % sdkdir
        if os.path.isfile(os.path.join(sdkdir, landmark)):
            return sdkdir
    # 2. The "Install Dir" value in the
    #    HKLM\Software\Microsoft\MicrosoftSDK\Directories registry key
    #    sometimes points to the right thing. However, after upgrading to
    #    the "Platform SDK for Windows Server 2003 SP1" this is dead end.
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                              r"Software\Microsoft\MicrosoftSDK\Directories")
        sdkdir, ignore = _winreg.QueryValueEx(key, "Install Dir")
    except EnvironmentError:
        pass
    else:
        if DEBUG:
            print r"PSDK: try 'HKLM\Software\Microsoft\MicrosoftSDK"\
                   "\Directories\Install Dir': '%s'" % sdkdir
        if os.path.isfile(os.path.join(sdkdir, landmark)):
            return sdkdir
    # 3. Each installed SDK (not just the platform SDK) seems to have GUID
    #    subkey of HKLM\Software\Microsoft\MicrosoftSDK\InstalledSDKs and
    #    it *looks* like the latest installed Platform SDK will be the
    #    only one with an "Install Dir" sub-value.
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                              r"Software\Microsoft\MicrosoftSDK\InstalledSDKs")
        i = 0
        while True:
            guid = _winreg.EnumKey(key, i)
            guidkey = _winreg.OpenKey(key, guid)
            try:
                sdkdir, ignore = _winreg.QueryValueEx(guidkey, "Install Dir")
            except EnvironmentError:
                pass
            else:
                if DEBUG:
                    print r"PSDK: try 'HKLM\Software\Microsoft\MicrosoftSDK"\
                           "\InstallSDKs\%s\Install Dir': '%s'"\
                           % (guid, sdkdir)
                if os.path.isfile(os.path.join(sdkdir, landmark)):
                    return sdkdir
            i += 1
    except EnvironmentError:
        pass
    # 4.  Vista's SDK
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                              r"Software\Microsoft\Microsoft SDKs\Windows")
        sdkdir, ignore = _winreg.QueryValueEx(key, "CurrentInstallFolder")
    except EnvironmentError:
        pass
    else:
        if DEBUG:
            print r"PSDK: try 'HKLM\Software\Microsoft\MicrosoftSDKs"\
                   "\Windows\CurrentInstallFolder': '%s'" % sdkdir
        if os.path.isfile(os.path.join(sdkdir, landmark)):
            return sdkdir

    # 5. Failing this just try a few well-known default install locations.
    progfiles = os.environ.get("ProgramFiles", r"C:\Program Files")
    defaultlocs = [
        os.path.join(progfiles, "Microsoft Platform SDK"),
        os.path.join(progfiles, "Microsoft SDK"),
    ]
    for sdkdir in defaultlocs:
        if DEBUG:
            print "PSDK: try default location: '%s'" % sdkdir
        if os.path.isfile(os.path.join(sdkdir, landmark)):
            return sdkdir

sdk_dir = find_platform_sdk_dir()

class WinExt (Extension):
    # Base class for all win32 extensions, with some predefined
    # library and include dirs, and predefined windows libraries.
    # Additionally a method to parse .def files into lists of exported
    # symbols, and to read 
    def __init__ (self, name, sources=None,
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
                  dsp_file=None,
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
                 ):
        assert dsp_file or sources, "Either dsp_file or sources must be specified"
        libary_dirs = library_dirs,
        include_dirs = ['com/win32com/src/include',
                        'win32/src'] + include_dirs
        libraries=libraries.split()

        if export_symbol_file:
            export_symbols = export_symbols or []
            export_symbols.extend(self.parse_def_file(export_symbol_file))

        if dsp_file:
            sources = sources or []
            sources.extend(self.get_source_files(dsp_file))
        # Some of our swigged files behave differently in distutils vs
        # MSVC based builds.  Always define DISTUTILS_BUILD so they can tell.
        define_macros = define_macros or []
        define_macros.append(("DISTUTILS_BUILD", None))
        self.pch_header = pch_header
        self.extra_swig_commands = extra_swig_commands or []
        self.windows_h_version = windows_h_version
        self.optional_headers = optional_headers
        self.is_regular_dll = is_regular_dll
        self.base_address = base_address
        self.platforms = platforms
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

    def parse_def_file(self, path):
        # Extract symbols to export from a def-file
        result = []
        for line in open(path).readlines():
            line = string.rstrip(line)
            if line and line[0] in string.whitespace:
                tokens = string.split(line)
                if not tokens[0][0] in string.letters:
                    continue
                result.append(string.join(tokens, ','))
        return result

    def get_source_files(self, dsp):
        result = []
        if dsp is None:
            return result
        dsp_path = os.path.dirname(dsp)
        for line in open(dsp, "r"):
            fields = line.strip().split("=", 2)
            if fields[0]=="SOURCE":
                if os.path.splitext(fields[1])[1].lower() in ['.cpp', '.c', '.i', '.rc', '.mc']:
                    pathname = os.path.normpath(os.path.join(dsp_path, fields[1]))
                    result.append(pathname)

        # Sort the sources so that (for example) the .mc file is processed first,
        # building this may create files included by other source files.
        # Note that this requires a patch to distutils' ccompiler classes so that
        # they build the sources in the order given.
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
                # /YX doesn't work in vs2008 or vs2003/64
                if build_ext.plat_name == 'win32' and get_build_version() < 9.0:
                    self.extra_compile_args.append("/YX"+self.pch_header)
                pch_name = os.path.join(build_ext.build_temp, self.name) + ".pch"
                self.extra_compile_args.append("/Fp"+pch_name)

            # bugger - add this to python!
            if build_ext.plat_name=="win32":
                self.extra_link_args.append("/MACHINE:x86")
            else:
                self.extra_link_args.append("/MACHINE:%s" % build_ext.plat_name[4:])
            # Old vs2003 needs this defined (Python itself uses it)
            if get_build_version() < 9.0 and build_ext.plat_name=="win-amd64":
                self.extra_compile_args.append('/D_M_X64')

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
            # ack - but fails with obscure errors in py23 :(
            if sys.version_info > (2,4):
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

            # Try and find the MFC source code, so we can reach inside for
            # some of the ActiveX support we need.  We need to do this late, so
            # the environment is setup correctly.
            # Only used by the win32uiole extensions, but I can't be
            # bothered making a subclass just for this - so they all get it!
            found_mfc = False
            for incl in os.environ.get("INCLUDE", "").split(os.pathsep):
                # first is a "standard" MSVC install, second is the Vista SDK.
                for candidate in ("..\src\occimpl.h", "..\..\src\mfc\occimpl.h"):
                    check = os.path.join(incl, candidate)
                    if os.path.isfile(check):
                        self.extra_compile_args.append('/DMFC_OCC_IMPL_H=\\"%s\\"' % candidate)
                        found_mfc = True
                        break
                if found_mfc:
                    break

class WinExt_pythonwin(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file") and not kw.get("sources"):
            kw["dsp_file"] = "pythonwin/" + name + ".dsp"
        kw.setdefault("extra_compile_args", []).extend(
                            ['-D_AFXDLL', '-D_AFXEXT','-D_MBCS'])
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "pythonwin"

class WinExt_win32(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file") and not kw.get("sources"):
            kw["dsp_file"] = "win32/" + name + ".dsp"
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32"

class WinExt_ISAPI(WinExt):
    def get_pywin32_dir(self):
        return "isapi"

# Note this is used only for "win32com extensions", not pythoncom
# itself - thus, output is "win32comext"
class WinExt_win32com(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file") and not kw.get("sources"):
            kw["dsp_file"] = "com/" + name + ".dsp"
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
class WinExt_win32com_mapi(WinExt_win32com):
    def __init__ (self, name, **kw):
        # The Exchange 2000 SDK seems to install itself without updating 
        # LIB or INCLUDE environment variables.  It does register the core 
        # directory in the registry tho - look it up
        sdk_install_dir = None
        libs = kw.get("libraries", "")
        keyname = "SOFTWARE\Microsoft\Exchange\SDK"
        for root in _winreg.HKEY_LOCAL_MACHINE, _winreg.HKEY_CURRENT_USER:
            try:
                keyob = _winreg.OpenKey(root, keyname)
                value, type_id = _winreg.QueryValueEx(keyob, "INSTALLDIR")
                if type_id == _winreg.REG_SZ:
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
        libs += " Ex2KSdk sadapi mapi32 netapi32"
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
class my_build(build):
    def run(self):
        build.run(self)
        # write a pywin32.version.txt.
        ver_fname = os.path.join(os.environ['temp'], "pywin32.version.txt")
        try:
            f = open(ver_fname, "w")
            f.write("%s\n" % build_id)
            f.close()
        except EnvironmentError, why:
            print "Failed to open '%s': %s" % (ver_fname, why)

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
        # Adjust paths etc for the platform SDK - this prevents the user from
        # needing to manually add these directories via the MSVC UI.  Note
        # that we currently ensure the SDK dirs are before the compiler
        # dirs, so its no problem if they have added these dirs to the UI)

        # (Note that just having them in INCLUDE/LIB does *not* work -
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
        extra = os.path.join(sdk_dir, 'include')
        # should not be possible for the SDK dirs to already be in our
        # include_dirs - they may be in the registry etc from MSVC, but
        # those aren't reflected here...
        assert extra not in self.include_dirs
        # and we will not work as expected if the dirs don't exist
        assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
        self.compiler.add_include_dir(extra)
        # and again for lib dirs.
        extra = os.path.join(sdk_dir, 'lib')
        if is_64bit:
            extra = os.path.join(extra, 'x64')
            assert os.path.isdir(extra), extra
        assert extra not in self.library_dirs # see above
        assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
        self.compiler.add_library_dir(extra)
        # directx sdk sucks - how to locate it automatically?
        # Must manually set DIRECTX_SDK_DIR for now.
        dxsdk_dir = os.environ.get("DIRECTX_SDK_DIR")
        if dxsdk_dir:
            extra = os.path.join(dxsdk_dir, 'include')
            assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
            self.compiler.add_include_dir(extra)
            if is_64bit:
                tail = 'x64'
            else:
                tail = 'x86'
            extra = os.path.join(dxsdk_dir, 'lib', tail)
            assert os.path.isdir(extra), "%s doesn't exist!" % (extra,)
            self.compiler.add_library_dir(extra)

        log.debug("After SDK processing, includes are %s", self.compiler.include_dirs)
        log.debug("After SDK processing, libs are %s", self.compiler.library_dirs)

        # Vista SDKs have a 'VC' directory with headers and libs for older
        # compilers.  We need to hack the support in here so that the
        # directories are after the compiler's own.  As noted above, the
        # only way to ensure they are after the compiler's is to put them
        # in the environment, which has the nice side-effect of working
        # for the rc executable.
        # We know its not needed on vs9...
        if get_build_version() < 9.0:
            if os.path.isdir(os.path.join(sdk_dir, 'VC', 'INCLUDE')):
                os.environ["INCLUDE"] += ";" + os.path.join(sdk_dir, 'VC', 'INCLUDE')
                log.debug("Vista SDK found: %%INCLUDE%% now %s", os.environ["INCLUDE"])
            if os.path.isdir(os.path.join(sdk_dir, 'VC', 'LIB')):
                os.environ["LIB"] += ";" + os.path.join(sdk_dir, 'VC', 'LIB')
                log.debug("Vista SDK found: %%LIB%% now %s", os.environ["LIB"])

        if sys.hexversion < 0x02030000:
            # Python 2.2 distutils doesn't handle the 'PC'/PCBuild directory for
            # us (it only exists if building from the source tree)
            extra = os.path.join(sys.exec_prefix, 'PC')
            if extra not in self.include_dirs and os.path.isdir(extra):
                self.compiler.add_include_dir(extra)
            extra = os.path.join(sys.exec_prefix, 'PCBuild')
            if extra not in self.library_dirs and os.path.isdir(extra):
                self.compiler.add_library_dir(os.path.join(extra))

    def _why_cant_build_extension(self, ext):
        # Return None, or a reason it can't be built.
        include_dirs = self.compiler.include_dirs + \
                       os.environ.get("INCLUDE", "").split(os.pathsep)
        if self.windows_h_version is None:
            for d in include_dirs:
                # We look for _WIN32_WINNT instead of WINVER as the Vista
                # SDK defines _WIN32_WINNT as WINVER and we aren't that clever
                # * Windows Server 2003 SDK sticks this version in WinResrc.h
                # * Vista SDKs stick the version in sdkddkver.h
                for header in ('WINDOWS.H', 'SDKDDKVER.H', "WinResrc.h"):
                    look = os.path.join(d, header)
                    if os.path.isfile(look):
                        # read the fist 100 lines, looking for #define WINVER 0xNN
                        # (Vista SDKs now define this based on _WIN32_WINNT,
                        # which should still be fine for old versions)
                        reob = re.compile("#define\W*_WIN32_WINNT\W*(0x[0-9a-fA-F]+)")
                        f = open(look, "r")
                        for i in range(500):
                            line = f.readline()
                            match = reob.match(line)
                            if match is not None:
                                self.windows_h_version = int(match.group(1), 16)
                                log.info("Found version 0x%x in %s" \
                                         % (self.windows_h_version, look))
                                break
                        else:
                            log.debug("No version in %r - looking for another...", look)
                    if self.windows_h_version is not None:
                        break
                if self.windows_h_version is not None:
                    break
            else:
                raise RuntimeError, "Can't find a version in Windows.h"
        if ext.windows_h_version > self.windows_h_version:
            return "WINDOWS.H with version 0x%x is required, but only " \
                   "version 0x%x is installed." \
                   % (ext.windows_h_version, self.windows_h_version)

        look_dirs = include_dirs
        for h in ext.optional_headers:
            for d in look_dirs:
                if os.path.isfile(os.path.join(d, h)):
                    break
            else:
                log.debug("Looked for %s in %s", h, look_dirs)
                return "The header '%s' can not be located" % (h,)

        common_dirs = self.compiler.library_dirs[:]
        common_dirs += os.environ.get("LIB", "").split(os.pathsep)
        patched_libs = []
        for lib in ext.libraries:
            if self.found_libraries.has_key(lib.lower()):
                found = self.found_libraries[lib.lower()]
            else:
                look_dirs = common_dirs + ext.library_dirs
                found = self.compiler.find_library_file(look_dirs, lib, self.debug)
                if not found:
                    log.debug("Looked for %s in %s", lib, look_dirs)
                    return "No library '%s'" % lib
                self.found_libraries[lib.lower()] = found
            patched_libs.append(os.path.splitext(os.path.basename(found))[0])
        # axdebug struggles under debug builds - worry about that when I care :)
        if sys.hexversion < 0x2040000 and ext.name == 'axdebug' and self.debug:
            return "axdebug doesn't build in VC6 debug builds (irony!)"

        if ext.platforms and self.plat_name not in ext.platforms:
            return "Only available on platforms %s" % (ext.platforms,)

        # We update the .libraries list with the resolved library name.
        # This is really only so "_d" works.
        ext.libraries = patched_libs
        return None # no reason - it can be built!

    def _build_scintilla(self):
        path = 'pythonwin\\Scintilla'
        makefile = 'makefile_pythonwin'
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

        if sdk_dir:
            self._fixup_sdk_dirs()

        # Here we hack a "pywin32" directory (one of 'win32', 'win32com',
        # 'pythonwin' etc), as distutils doesn't seem to like the concept
        # of multiple top-level directories.
        assert self.package is None
        for ext in self.extensions:
            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError, "Not a win32 package!"
            self.build_extension(ext)

        for ext in W32_exe_files:
            ext.finalize_options(self)
            why = self._why_cant_build_extension(ext)
            if why is not None:
                self.excluded_extensions.append((ext, why))
                assert why, "please give a reason, or None"
                print "Skipping %s: %s" % (ext.name, why)
                continue

            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError, "Not a win32 package!"
            self.build_exefile(ext)

        # Not sure how to make this completely generic, and there is no
        # need at this stage.
        self._build_scintilla()
        # Copy cpp lib files needed to create Python COM extensions
        clib_files = (['win32', 'pywintypes%s.lib'],
                      ['win32com', 'pythoncom%s.lib'])
        for clib_file in clib_files:
            target_dir = os.path.join(self.build_lib, clib_file[0], "libs")
            if not os.path.exists(target_dir):
                self.mkpath(target_dir)
            suffix = ""
            if self.debug:
                suffix = "_d"
            fname = clib_file[1] % suffix
            self.copy_file(
                    os.path.join(self.build_temp, fname), target_dir)
        # The MFC DLLs.
        try:
            target_dir = os.path.join(self.build_lib, "pythonwin")
            if sys.hexversion < 0x2040000:
                pass # don't do anything for these early versions.
            elif sys.hexversion < 0x2060000:
                # hrm - there doesn't seem to be a 'redist' directory for this
                # compiler (even the installation CDs only seem to have the MFC
                # DLLs in the "win\system" directory - just grab it from
                # system32 (but we can't even use win32api for that!)
                src = os.path.join(os.environ.get('SystemRoot'), 'System32', 'mfc71.dll')
                if not os.path.isfile(src):
                    raise RuntimeError, "Can't find %r" % (src,)
                self.copy_file(src, target_dir)
            else:
                # On a 64bit host, the value we are looking for is actually in
                # SysWow64Node - but that is only available on xp and later.
                access = _winreg.KEY_READ
                if sys.getwindowsversion()[0] >= 5:
                    access = access | 512 # KEY_WOW64_32KEY
                if self.plat_name == 'win-amd64':
                    plat_dir = "amd64"
                else:
                    plat_dir = "x86"
                # Find the redist directory.
                vckey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                                        r"SOFTWARE\Microsoft\VisualStudio\9.0\Setup\VC",
                                        0, access)
                val, val_typ = _winreg.QueryValueEx(vckey, "ProductDir")
                mfc_dir = os.path.join(val, "redist", plat_dir, "Microsoft.VC90.MFC")
                if not os.path.isdir(mfc_dir):
                    raise RuntimeError, "Can't find the redist dir at %r" % (mfc_dir)
                files = "mfc90.dll mfc90u.dll mfcm90.dll mfcm90u.dll Microsoft.VC90.MFC.manifest".split()
                for f in files:
                    self.copy_file(
                            os.path.join(mfc_dir, f), target_dir)
        except (EnvironmentError, RuntimeError), exc:
            print "Can't find an installed VC for the MFC DLLs:", exc


    def build_exefile(self, ext):
        from types import ListType, TupleType
        sources = ext.sources
        if sources is None or type(sources) not in (ListType, TupleType):
            raise DistutilsSetupError, \
                  ("in 'ext_modules' option (extension '%s'), " +
                   "'sources' must be present and must be " +
                   "a list of source filenames") % ext.name
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
        # 2.2 has no 'depends' param.
        kw = {'output_dir': output_dir,
              'macros': macros,
              'include_dirs': ext.include_dirs,
              'debug': self.debug,
              'extra_postargs': extra_args
        }
        if sys.version_info > (2,3):
            kw["depends"] = ext.depends
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
        if sys.version_info > (2,3):
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
            print "Skipping %s: %s" % (ext.name, why)
            return
        self.current_extension = ext

        ext.finalize_options(self)

        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        if sys.version_info < (2,3):
            # 2.3+ - Wrong dir, numbered name
            self.build_temp = os.path.join(self.build_temp, ext.name)

        try:
            build_ext.build_extension(self, ext)
            # XXX This has to be changed for mingw32
            extra = self.debug and "_d.lib" or ".lib"
            if ext.name in ("pywintypes", "pythoncom"):
                # The import libraries are created as PyWinTypes23.lib, but
                # are expected to be pywintypes.lib.
                name1 = "%s%d%d%s" % (ext.name, sys.version_info[0], sys.version_info[1], extra)
                name2 = "%s%s" % (ext.name, extra)
            else:
                name1 = name2 = ext.name + extra
            # The compiler always creates 'pywintypes22.lib', whereas we
            # actually want 'pywintypes.lib' - copy it over.
            # Worse: 2.3+ MSVCCompiler constructs the .lib file in the same
            # directory as the first source file's object file:
            #    os.path.dirname(objects[0])
            # rather than in the self.build_temp directory
            if sys.version_info > (2,3):
                # 2.3+ - Wrong dir, numbered name
                src = os.path.join(old_build_temp,
                               os.path.dirname(ext.sources[0]),
                               name1)
            else:
                # 2.2 it is in the right dir, just with the 'numbered' named.
                src = os.path.join(self.build_temp, name1)
            dst = os.path.join(old_build_temp, name2)
            if os.path.abspath(src) != os.path.abspath(dst):
                self.copy_file(src, dst)#, update=1)
        finally:
            self.build_temp = old_build_temp

    def get_ext_filename(self, name):
        # The pywintypes and pythoncom extensions have special names
        extra_dll = self.debug and "_d.dll" or ".dll"
        extra_exe = self.debug and "_d.exe" or ".exe"
        if name == "pywin32_system32.pywintypes":
            return r"pywin32_system32\pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        elif name == "pywin32_system32.pythoncom":
            return r"pywin32_system32\pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra_dll)
        elif name.endswith("win32.perfmondata"):
            return r"win32\perfmondata" + extra_dll
        elif name.endswith("win32.win32popenWin9x"):
            return r"win32\win32popenWin9x" + extra_exe
        elif name.endswith("win32.pythonservice"):
            return r"win32\pythonservice" + extra_exe
        elif name.endswith("pythonwin.Pythonwin"):
            return r"pythonwin\Pythonwin" + extra_exe
        elif name.endswith("isapi.PyISAPI_loader"):
            return r"isapi\PyISAPI_loader" + extra_dll
        return build_ext.get_ext_filename(self, name)

    def get_export_symbols(self, ext):
        if ext.is_regular_dll:
            return ext.export_symbols
        return build_ext.get_export_symbols(self, ext)

    def find_swig (self):
        # We know where swig is
        os.environ["SWIG_LIB"] = os.path.abspath(r"swig\swig_lib")
        return os.path.abspath(r"swig\swig.exe")

    def swig_sources (self, sources, ext=None):
        new_sources = []
        swig_sources = []
        swig_targets = {}
        # XXX this drops generated C/C++ files into the source tree, which
        # is fine for developers who want to distribute the generated
        # source -- but there should be an option to put SWIG output in
        # the temp dir.
        # XXX - Note that swig_wince_modules no longer #include the real
        # generated .cpp file (well, they do, but are avoided via the
        # pre-processor.)  So this is no longer a reason we can't generate
        # directly to the temp directory.
        target_ext = '.cpp'
        for source in sources:
            (base, ext) = os.path.splitext(source)
            if ext == ".i":             # SWIG interface file
                if os.path.split(base)[1] in swig_include_files:
                    continue
                swig_sources.append(source)
                # Patch up the filenames for SWIG modules that also build
                # under WinCE - see defn of swig_wince_modules for details
                if os.path.basename(base) in swig_interface_parents:
                    swig_targets[source] = base + target_ext
                elif self.current_extension.name == "winxpgui" and \
                     os.path.basename(base)=="win32gui":
                    # More vile hacks.  winxpmodule is built from win32gui.i -
                    # just different #defines are setup for windows.h.
                    new_target = os.path.join(os.path.dirname(base),
                                              "winxpguimodule") + target_ext
                    swig_targets[source] = new_target
                    new_sources.append(new_target)
                elif os.path.basename(base) in swig_wince_modules:
                    # We need to add this .cpp to the sources, so it
                    # will be built.
                    new_target = base + 'module_win32' + target_ext
                    swig_targets[source] = new_target
                    new_sources.append(new_target)
                else:
                    swig_targets[source] = base + 'module' + target_ext
            else:
                new_sources.append(source)

        if not swig_sources:
            return new_sources

        swig = self.find_swig()

        for source in swig_sources:
            swig_cmd = [swig, "-python", "-c++"]
            swig_cmd.append("-dnone",) # we never use the .doc files.
            swig_cmd.extend(self.current_extension.extra_swig_commands)
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
            if self.force or newer(os.path.abspath(source), os.path.abspath(target)):
                swig_cmd.extend(["-o",
                                 os.path.abspath(target),
                                 os.path.abspath(source)])
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
        if not self.dry_run and not self.skip_build and not self.root:
            # What executable to use?  This one I guess.
            filename = os.path.join(os.path.dirname(this_file), "pywin32_postinstall.py")
            if not os.path.isfile(filename):
                raise RuntimeError, "Can't find '%s'" % (filename,)
            print "Executing post install script..."
            os.spawnl(os.P_NOWAIT, sys.executable,
                      sys.executable, filename,
                      "-quiet", "-wait", str(os.getpid()), "-install")

# As per get_source_files, we need special handling so .mc file is
# processed first.  It appears there was an intention to fix distutils
# itself, but as at 2.4 that hasn't happened.  We need yet more vile
# hacks to get a subclassed compiler in.
# (otherwise we replace all of build_extension!)
def my_new_compiler(**kw):
    if kw.has_key('compiler') and kw['compiler'] in (None, 'msvc'):
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
        # Oh joy of joys.  With latest platform SDKs, VC6 is unable to link
        # debug mode projects.  So we use the VC7 linker.
        old_linker = None
        if debug and sys.hexversion < 0x02040000:
            # msvc compiler uses __ prefix on attributes, making it hard
            # to patch things up. So we get tricky, creating a new compiler
            # after tricking distutils into thinking we are a later version.
            save_env = {}
            for key in "LIB INCLUDE PATH".split():
                save_env[key] = os.environ[key]
            def hack_get_build_version():
                return 7.1
            gbv = msvccompiler.get_build_version
            msvccompiler.get_build_version = hack_get_build_version
            new_compiler = msvccompiler.MSVCCompiler()
            msvccompiler.get_build_version = gbv
            for key in save_env.keys():
                os.environ[key] = save_env[key]

            old_linker = self.linker
            self.linker = new_compiler.linker
        try:
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
                import optparse # win32verstamp will not work without this!
                ok = True
            except ImportError:
                ok = False
            if ok:
                stamp_script = os.path.join(sys.prefix, "Lib", "site-packages",
                                            "win32", "lib", "win32verstamp.py")
                ok = os.path.isfile(stamp_script)
            if ok:
                args = [sys.executable]
                args.append(stamp_script)
                args.append("--version=%s" % (pywin32_version,))
                args.append("--comments=http://pywin32.sourceforge.net")
                args.append("--original-filename=%s" % (os.path.basename(output_filename),))
                args.append("--product=PyWin32")
                if '-v' not in sys.argv:
                    args.append("--quiet")
                args.append(output_filename)
                try:
                    self.spawn(args)
                except DistutilsExecError, msg:
                    log.info("VersionStamp failed: %s", msg)
                    ok = False
            if not ok:
                log.info('Unable to import verstamp, no version info will be added')
        finally:
            if old_linker is not None:
                self.linker = old_linker

    # overriding _setup_compile is the easiest way to get this support in.
    def _setup_compile(self, *args):
        macros, objects, extra, pp_opts, build = \
               msvccompiler.MSVCCompiler._setup_compile(self, *args)
        if sys.hexversion < 0x02040000:
            build_order = ".i .mc .rc .cpp".split()
            decorated = [(build_order.index(ext.lower()), obj, (src, ext))
                         for obj, (src, ext) in build.items()]
            decorated.sort()
            items = [item[1:] for item in decorated]
            # The compiler itself only calls ".items" - leverage that, so that
            # when it does, the list is in the correct order.
            class OnlyItems:
                def __init__(self, items):
                    self._items = items
                def items(self):
                    return self._items
            build = OnlyItems(items)

        return macros, objects, extra, pp_opts, build

################################################################

class my_install_data(install_data):
    """A custom install_data command, which will install it's files
    into the standard directories (normally lib/site-packages).
    """
    def finalize_options(self):
        if self.install_dir is None:
            installobj = self.distribution.get_command_obj('install')
            self.install_dir = installobj.install_lib
        print 'Installing data files to %s' % self.install_dir
        install_data.finalize_options(self)

################################################################

pywintypes = WinExt_system32('pywintypes',
                    dsp_file = r"win32\PyWinTypes.dsp",
                    extra_compile_args = ['-DBUILD_PYWINTYPES'],
                    libraries = "advapi32 user32 ole32 oleaut32",
                    pch_header = "PyWinTypes.h",
                    )

win32_extensions = [pywintypes]

win32_extensions.append(
    WinExt_win32("perfmondata", 
                 libraries="advapi32",
                 extra_compile_args=["-DUNICODE", "-D_UNICODE", "-DWINNT"],
                 export_symbol_file = "win32/src/PerfMon/perfmondata.def",
                 is_regular_dll = 1,
        ),
    )

for info in (
        ("dbi", "", False),
        ("mmapfile", "", False),
        ("odbc", "odbc32 odbccp32 dbi", False),
        ("perfmon", "", True),
        ("timer", "user32", False),
        ("win2kras", "rasapi32", False, 0x0500),
        ("win32api", "user32 advapi32 shell32 version", False, 0x0500, 'win32/src/win32apimodule.cpp win32/src/win32api_display.cpp'),
        ("win32cred", "AdvAPI32 credui", True, 0x0501, 'win32/src/win32credmodule.cpp'),
        ("win32crypt", "Crypt32", False, 0x0500, 'win32/src/win32crypt.i win32/src/win32cryptmodule.cpp'),
        ("win32file", "oleaut32", False, 0x0500),
        ("win32event", "user32", False),
        ("win32clipboard", "gdi32 user32 shell32", False),
        ("win32evtlog", "advapi32 oleaut32", False),
        # win32gui handled below
        ("win32job", "user32", False, 0x0500, 'win32/src/win32job.i win32/src/win32jobmodule.cpp'),
        ("win32lz", "lz32", False),
        ("win32net", "netapi32 advapi32", True, None, """
              win32/src/win32net/win32netfile.cpp    win32/src/win32net/win32netgroup.cpp
              win32/src/win32net/win32netmisc.cpp    win32/src/win32net/win32netmodule.cpp
              win32/src/win32net/win32netsession.cpp win32/src/win32net/win32netuse.cpp
              win32/src/win32net/win32netuser.cpp
              """),
        ("win32pdh", "", False),
        ("win32pipe", "", False),
        ("win32print", "winspool user32 gdi32", False, 0x0500),
        ("win32process", "advapi32 user32", False, 0x0500),
        ("win32profile", "Userenv", True, None, 'win32/src/win32profilemodule.cpp'),
        ("win32ras", "rasapi32 user32", False),
        ("win32security", "advapi32 user32 netapi32", True, 0x0500, """
            win32/src/win32security.i       win32/src/win32securitymodule.cpp
            win32/src/win32security_sspi.cpp win32/src/win32security_ds.cpp
            """),
        ("win32service", "advapi32 oleaut32 user32", True, 0x0501),
        ("win32trace", "advapi32", False),
        ("win32wnet", "netapi32 mpr", False),
        ("win32inet", "wininet", False, 0x500, """
            win32/src/win32inet.i           win32/src/win32inetmodule.cpp
            win32/src/win32inet_winhttp.cpp"""
                        ),
        ("win32console", "kernel32", True, 0x0501, "win32/src/win32consolemodule.cpp"),
        ("win32ts", "WtsApi32", True, 0x0501, "win32/src/win32tsmodule.cpp"),
        ("_win32sysloader", "", False, 0x0501, "win32/src/_win32sysloader.cpp"),
        ("win32transaction", "kernel32", True, 0x0501, "win32/src/win32transactionmodule.cpp"),

    ):

    name, lib_names, is_unicode = info[:3]
    windows_h_ver = sources = None
    if len(info)>3:
        windows_h_ver = info[3]
    if len(info)>4:
        sources = info[4].split()
    extra_compile_args = []
    if is_unicode:
        extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT']
    ext = WinExt_win32(name, 
                 libraries=lib_names,
                 extra_compile_args = extra_compile_args,
                 windows_h_version = windows_h_ver,
                 sources = sources)
    win32_extensions.append(ext)

# The few that need slightly special treatment
win32_extensions += [
    WinExt_win32("win32gui", 
           sources = """
                win32/src/win32dynamicdialog.cpp
                win32/src/win32gui.i win32/src/win32guimodule.cpp
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
                win32/src/win32gui.i win32/src/win32guimodule.cpp
               """.split(),
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           windows_h_version=0x0500,
           define_macros = [("WIN32GUI",None), ("WINXPGUI",None)],
           extra_swig_commands=["-DWINXPGUI"],
        ),
    # winxptheme
    WinExt_win32("_winxptheme",
           sources = ["win32/src/_winxptheme.i", "win32/src/_winxpthememodule.cpp"],
           libraries="gdi32 user32 comdlg32 comctl32 shell32 Uxtheme",
           windows_h_version=0x0500,
           extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT'],
        ),
]
win32_extensions += [
    WinExt_win32('servicemanager',
           extra_compile_args = ['-DUNICODE', '-D_UNICODE', 
                                 '-DWINNT', '-DPYSERVICE_BUILD_DLL'],
           libraries = "user32 ole32 advapi32 shell32",
           dsp_file = r"win32\Pythonservice servicemanager.dsp",
           windows_h_version = 0x500),
]

# win32help uses htmlhelp.lib which is built with MSVC7 and /GS.  This
# causes problems with references to the @__security_check_cookie magic.
# Use bufferoverflowu.lib if it exists.
win32help_libs = "htmlhelp user32 advapi32"
if sdk_dir and os.path.exists(os.path.join(sdk_dir, "Lib", "bufferoverflowu.lib")):
    win32help_libs += " bufferoverflowu"
# but of-course the Vista SDK does it differently...
elif sdk_dir and os.path.exists(os.path.join(sdk_dir, "VC", "Lib", "RunTmChk.lib")):
    win32help_libs += " RunTmChk"
win32_extensions += [
    WinExt_win32('win32help',
                 libraries=win32help_libs,
                 windows_h_version = 0x500),
]

dirs = {
    'adsi' : 'com/win32comext/adsi/src',
    'propsys' : 'com/win32comext/propsys/src',
    'shell' : 'com/win32comext/shell/src',
    'axcontrol' : 'com/win32comext/axcontrol/src',
    'mapi' : 'com/win32comext/mapi/src',
    'authorization' : 'com/win32comext/authorization/src',
    'taskscheduler' : 'com/win32comext/taskscheduler/src',
    'bits' : 'com/win32comext/bits/src',
}

# The COM modules.
pythoncom = WinExt_system32('pythoncom',
                   dsp_file=r"com\win32com.dsp",
                   libraries = "oleaut32 ole32 user32",
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
                        %(axcontrol)s/AXControl.cpp              %(axcontrol)s/PyIOleControl.cpp
                        %(axcontrol)s/PyIOleInPlaceSiteEx.cpp    %(axcontrol)s/PyISpecifyPropertyPages.cpp
                        %(axcontrol)s/PyIObjectWithSite.cpp      %(axcontrol)s/PyIOleInPlaceObject.cpp
                        %(axcontrol)s/PyIOleInPlaceSiteWindowless.cpp  %(axcontrol)s/PyIViewObject.cpp
                        %(axcontrol)s/PyIOleClientSite.cpp       %(axcontrol)s/PyIOleInPlaceSite.cpp
                        %(axcontrol)s/PyIOleObject.cpp           %(axcontrol)s/PyIViewObject2.cpp
                        %(axcontrol)s/PyIOleCommandTarget.cpp
                        """ % dirs).split()),
    WinExt_win32com('axscript',
            dsp_file=r"com\Active Scripting.dsp",
            extra_compile_args = ['-DPY_BUILD_AXSCRIPT'],
            pch_header = "stdafx.h"
    ),
    # ActiveDebugging is a mess.  See the comments in the docstring of this
    # module for details on getting it built.
    WinExt_win32com('axdebug',
            dsp_file=r"com\Active Debugging.dsp",
            libraries="axscript",
            pch_header = "stdafx.h",
            optional_headers = ["activdbg.h"],
    ),
    WinExt_win32com('internet'),
    WinExt_win32com('mapi', libraries="mapi32", pch_header="PythonCOM.h",
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
                        %(mapi)s/PyIMsgStore.i          %(mapi)s/PyIMsgStore.cpp
                        %(mapi)s/PyIProfAdmin.i         %(mapi)s/PyIProfAdmin.cpp
                        %(mapi)s/PyIProfSect.i          %(mapi)s/PyIProfSect.cpp
                        %(mapi)s/PyIMAPIAdviseSink.cpp

                        %(mapi)s/mapiutil.cpp
                        %(mapi)s/mapiguids.cpp
                        """ % dirs).split()),
    WinExt_win32com_mapi('exchange', libraries="version",
                         sources=("""
                                  %(mapi)s/exchange.i         %(mapi)s/exchange.cpp
                                  """ % dirs).split()),
    WinExt_win32com_mapi('exchdapi',
                         sources=("""
                                  %(mapi)s/exchdapi.i         %(mapi)s/exchdapi.cpp
                                  """ % dirs).split()),
    WinExt_win32com('shell', libraries='shell32', pch_header="shell_pch.h",
                    windows_h_version = 0x600,
                    sources=("""
                        %(shell)s/PyIActiveDesktop.cpp
                        %(shell)s/PyIAsyncOperation.cpp
                        %(shell)s/PyIBrowserFrameOptions.cpp
                        %(shell)s/PyICategorizer.cpp
                        %(shell)s/PyICategoryProvider.cpp
                        %(shell)s/PyIColumnProvider.cpp
                        %(shell)s/PyIContextMenu.cpp
                        %(shell)s/PyIContextMenu2.cpp
                        %(shell)s/PyIContextMenu3.cpp
                        %(shell)s/PyICopyHook.cpp
                        %(shell)s/PyIDefaultExtractIconInit.cpp
                        %(shell)s/PyIDeskBand.cpp
                        %(shell)s/PyIDockingWindow.cpp
                        %(shell)s/PyIDropTargetHelper.cpp
                        %(shell)s/PyIEnumExplorerCommand.cpp
                        %(shell)s/PyIEnumIDList.cpp
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
                        %(shell)s/PyIInputObject.cpp
                        %(shell)s/PyINameSpaceTreeControl.cpp
                        %(shell)s/PyIPersistFolder.cpp
                        %(shell)s/PyIPersistFolder2.cpp
                        %(shell)s/PyIQueryAssociations.cpp
                        %(shell)s/PyIShellBrowser.cpp
                        %(shell)s/PyIShellExtInit.cpp
                        %(shell)s/PyIShellFolder.cpp
                        %(shell)s/PyIShellFolder2.cpp
                        %(shell)s/PyIShellIcon.cpp
                        %(shell)s/PyIShellIconOverlay.cpp
                        %(shell)s/PyIShellIconOverlayIdentifier.cpp
                        %(shell)s/PyIShellIconOverlayManager.cpp
                        %(shell)s/PyIShellItem.cpp
                        %(shell)s/PyIShellItemArray.cpp
                        %(shell)s/PyIShellLink.cpp
                        %(shell)s/PyIShellLinkDataList.cpp
                        %(shell)s/PyIShellView.cpp
                        %(shell)s/PyIUniformResourceLocator.cpp
                        %(shell)s/shell.cpp

                        """ % dirs).split()),

    WinExt_win32com('propsys', libraries='propsys',
                    sources=("""
                        %(propsys)s/propsys.cpp
                        """ % dirs).split()),


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
    WinExt_win32com('ifilter', libraries='ntquery'),
    WinExt_win32com('directsound', pch_header='directsound_pch.h',
                    optional_headers = ['dsound.h'],
                    libraries='user32 dsound dxguid'),
    WinExt_win32com('authorization', libraries='aclui advapi32',
                    sources=("""
                        %(authorization)s/authorization.cpp
                        %(authorization)s/PyGSecurityInformation.cpp
                        """ % dirs).split()),
]

pythonwin_extensions = [
    WinExt_pythonwin("win32ui", extra_compile_args = ['-DBUILD_PYW'],
                     pch_header="stdafx.h", base_address=dll_base_address,
                     depends=["Pythonwin/stdafx.h", "Pythonwin/win32uiExt.h"]),
    WinExt_pythonwin("win32uiole", pch_header="stdafxole.h",
                     windows_h_version = 0x500),
    WinExt_pythonwin("dde", pch_header="stdafxdde.h", platforms=['win32']),
    ]
# win32ui is large, so we reserve more bytes than normal
dll_base_address += 0x100000

other_extensions = []
if sys.hexversion >= 0x2030000:
    # GILState stuff too hard pre 2.3!
    other_extensions.append(
        WinExt_ISAPI('PyISAPI_loader',
           sources=[os.path.join("isapi", "src", s) for s in
                   """PyExtensionObjects.cpp PyFilterObjects.cpp
                      pyISAPI.cpp pyISAPI_messages.mc
                      PythonEng.cpp StdAfx.cpp Utils.cpp
                   """.split()],
           depends=[os.path.join("isapi", "src", s) for s in
                   """ControlBlock.h FilterContext.h PyExtensionObjects.h
                      PyFilterObjects.h pyISAPI.h pyISAPI_messages.h
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
    WinExt_win32("win32popenWin9x",
                 libraries = "user32",
                 platforms = ["win32"]),
    WinExt_win32("pythonservice",
                 dsp_file = "win32/PythonService EXE.dsp",
                 extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT'],
                 extra_link_args=["/SUBSYSTEM:CONSOLE"],
                 libraries = "user32 advapi32 ole32 shell32"),
        WinExt_pythonwin("Pythonwin", extra_link_args=["/SUBSYSTEM:WINDOWS"]),
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
    'PyIMsgStore':          'IMAPIProp',
    'PyIProfAdmin':         '',
    'PyIProfSect':          'IMAPIProp',
    # exchange and exchdapi
    'exchange':             None,
    'exchdapi':             None,
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

# A list of modules that can also be built for Windows CE.  These generate
# their .i to _win32.cpp or _wince.cpp.
swig_wince_modules = "win32event win32file win32gui win32process".split()

# .i files that are #included, and hence are not part of the build.  Our .dsp
# parser isn't smart enough to differentiate these.
swig_include_files = "mapilib adsilib".split()

# Helper to allow our script specifications to include wildcards.
def expand_modules(module_dir):
    flist = FileList()
    flist.findall(module_dir)
    flist.include_pattern("*.py")
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
            flist.include_pattern(os.path.basename(file))
            # We never want CVS
            flist.exclude_pattern(re.compile(".*\\\\CVS\\\\"), is_regex=1)
            flist.exclude_pattern("*.pyc")
            flist.exclude_pattern("*.pyo")
            # files with a leading dot upset bdist_msi, and '.*' doesn't
            # work - it matches from the start of the string and we have
            # dir names.  So any '\.' gets the boot.
            flist.exclude_pattern(re.compile(".*\\\\\."), is_regex=1)
            if not flist.files:
                raise RuntimeError, "No files match '%s'" % file
            files_use = flist.files
        else:
            if not os.path.isfile(file):
                raise RuntimeError, "No file '%s'" % file
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
            if not str(details).startswith("No file"):
                raise
            log.info('NOTE: Optional file %s not found - skipping' % file)
        else:
            ret.append(temp[0])
    return ret

################################################################
if len(sys.argv)==1:
    # distutils will print usage - print our docstring first.
    print __doc__
    print "Standard usage information follows:"

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

# Python 2.2 distutils can't handle py_modules *and* packages,
# but putting 'win32.lib' as a package whinges there is no __init__
if sys.version_info < (2,3):
    packages.append('win32.lib')
    py_modules = None
else:
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
           }
    
dist = setup(name="pywin32",
      version=str(build_id),
      description="Python for Window Extensions",
      long_description="Python extensions for Microsoft Windows\n"
                       "Provides access to much of the Win32 API, the\n"
                       "ability to create and use COM objects, and the\n"
                       "Pythonwin environment.",
      author="Mark Hammond (et al)",
      author_email = "mhammond@users.sourceforge.net",
      url="http://sourceforge.net/projects/pywin32/",
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

      scripts = ["pywin32_postinstall.py"],

      ext_modules = ext_modules,

      package_dir = {"win32com": "com/win32com",
                     "win32comext": "com/win32comext",
                     "pythonwin": "pythonwin",},
      packages = packages,
      py_modules = py_modules,

      data_files=[('', (os.path.join(os.environ['temp'],'pywin32.version.txt'),))] + 
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
                'win32/scripts/*',
                'win32/test/*',
                'win32/Demos/*',
                'com/win32com/readme.htm',
                # win32com test utility files.
                'com/win32com/test/*',
                # win32com docs
                'com/win32com/HTML/*',
                'com/win32comext/adsi/demos/*',
                # Active Scripting test and demos.
                'com/win32comext/axscript/test/*',
                'com/win32comext/axscript/Demos/*',
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
                'adodbapi/tests/*.py',
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
if dist.command_obj.has_key('build_ext'):
    what_string = "built"
    if dist.command_obj.has_key('install'): # just to be purdy
        what_string += "/installed"
    # Print the list of extension modules we skipped building.
    if dist.command_obj.has_key('build_ext'):
        excluded_extensions = dist.command_obj['build_ext'].excluded_extensions
        if excluded_extensions:
            print "*** NOTE: The following extensions were NOT %s:" % what_string
            for ext, why in excluded_extensions:
                print " %s: %s" % (ext.name, why)
            print "For more details on installing the correct libraries and headers,"
            print "please execute this script with no arguments (or see the docstring)"
        else:
            print "All extension modules %s OK" % (what_string,)
