build_number=203
# Putting buildno at the top prevents automatic __doc__ assignment, and
# I *want* the build number at the top :)
__doc__="""This is a distutils setup-script for win32all

To build the win32all extensions, simply execute:
  python setup_win32all.py -q build

These extensions require a number of libraries to build, some of which may
require you to install special SDKs or toolkits.  This script will attempt
to build as many as it can, and at the end of the build will report any 
extension modules that could not be built and why.  Early versions of
certain Windows headers will also cause certain modules to be skipped (in
general, the latest "Platform SDK" from:
  http://www.microsoft.com/msdownload/platformsdk/sdkupdate
should be used.

If you don't use the extensions that fail to build, you can ignore these
warnings; if you do use them, you must install the correct libraries.

The 'axdebug' extension requires Active Debugging dev files available here:
  http://support.microsoft.com/default.aspx?kbid=223389
Download Scriptng.exe, unpack the files to a temporary directory and manually
copy the .h and .lib files to your Platform SDK installation's include
and lib directories, respectively -- overwriting some files of the same name.

To install the win32all extensions, execute:
  python setup_win32all.py -q install
  
This will install the built extensions into your site-packages directory,
create an appropriate .pth file, and should leave everything ready to use.
There should be no need to modify the registry.

To build or install debug (_d) versions of these extensions, ensure you have
built or installed a debug version of Python itself, then pass the "--debug"
flag to the build command - eg:
  python setup_win32all.py -q build --debug
or to build and install a debug version:
  python setup_win32all.py -q build --debug install
"""
# Thomas Heller, started in 2000 or so.
#
# Things known to be missing:
# * Newer win32all.exes installed Pythonwin.exe next to python.exe.  This
#   leaves it in the pythonwin directory, but it seems to work fine.
#   It works for Pythonwin with a non-admin install so long as it is started 
#   with the cwd being the Python home directory (our post_install script
#   sets up the Pythonwin shortcut this way)

from distutils.core import setup, Extension, Command
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.command.install_data import install_data
from distutils.dep_util import newer_group, newer
from distutils import dir_util, file_util
from distutils.sysconfig import get_python_lib
from distutils.filelist import FileList
import types, glob
import os, string, sys
import re
import _winreg

# Python 2.2 has no True/False
try:
    True; False
except NameError:
    True=0==0
    False=1==0
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

try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]

# We get upset if the cwd is not our source dir, but it is a PITA to
# insist people manually CD there first!
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))

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
        extra_link_args = extra_link_args or []
        extra_link_args.append("/MACHINE:ix86")
        # Some of our swigged files behave differently in distutils vs
        # MSVC based builds.  Always define DISTUTILS_BUILD so they can tell.
        define_macros = define_macros or []
        define_macros.append(("DISTUTILS_BUILD", None))
        self.pch_header = pch_header
        self.extra_swig_commands = extra_swig_commands or []
        self.windows_h_version = windows_h_version
        self.optional_headers = optional_headers
        self.is_regular_dll = is_regular_dll
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

class WinExt_pythonwin(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file"):
            kw["dsp_file"] = "pythonwin/" + name + ".dsp"
        kw.setdefault("extra_compile_args", []).extend(
                            ['-D_AFXDLL', '-D_AFXEXT','-D_MBCS'])
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "pythonwin"

class WinExt_win32(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file"):
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
        if not kw.has_key("dsp_file"):
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
        else:
            # The MSVC6 included exchange SDK has these libs
            libs += """ MBLOGON ADDRLKUP mapi32 exchinst
                       EDKCFG EDKUTILS EDKMAPI ACLCLS"""
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
        # Try and locate the platform SDK - this prevents the user from needing
        # to manually add these directories via the MSVC UI.
        # (Note that just having them in INCLUDE/LIB does *not* work -
        # distutils thinks it knows better, and resets those vars.
        try:
            key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                                  r"Software\Microsoft\MicrosoftSDK\Directories")
            sdk_dir, ignore = _winreg.QueryValueEx(key, "Install Dir")
        except OSError:
            sdk_dir = None
        if sdk_dir:
            extra = os.path.join(sdk_dir, 'include')
            if extra not in self.include_dirs and os.path.isdir(extra):
                self.include_dirs.insert(0, extra)
            extra = os.path.join(sdk_dir, 'lib')
            if extra not in self.library_dirs and os.path.isdir(extra):
                self.library_dirs.insert(0, extra)
        else:
            print "Warning - can't find an installed platform SDK"
        # Python 2.2 distutils doesn't handle the 'PC'/PCBuild directory for
        # us (it only exists if building from the source tree)
        extra = os.path.join(sys.exec_prefix, 'PC')
        if extra not in self.include_dirs and os.path.isdir(extra):
            self.include_dirs.append(extra)
        extra = os.path.join(sys.exec_prefix, 'PCBuild')
        if extra not in self.library_dirs and os.path.isdir(extra):
            self.library_dirs.append(os.path.join(extra))
        self.excluded_extensions = [] # list of (ext, why)

    def _why_cant_build_extension(self, ext):
        # Return None, or a reason it can't be built.
        if self.windows_h_version is None:
            include_dirs = self.compiler.include_dirs + \
                           os.environ.get("INCLUDE").split(os.pathsep)
            for d in include_dirs:
                look = os.path.join(d, "WINDOWS.H")
                if os.path.isfile(look):
                    # read the fist 100 lines, looking for #define WINVER 0xNN
                    reob = re.compile("#define\WWINVER\W(0x[0-9a-fA-F]+)")
                    f = open(look, "r")
                    for i in range(100):
                        line = f.readline()
                        match = reob.match(line)
                        if match is not None:
                            self.windows_h_version = int(match.group(1), 16)
                            log.info("Found WINDOWS.H version 0x%x in %s" \
                                     % (self.windows_h_version, d))
                            break
                if self.windows_h_version is not None:
                    break
            else:
                raise RuntimeError, "Can't find a version in Windows.h"
        if ext.windows_h_version > self.windows_h_version:
            return "WINDOWS.H with version 0x%x is required, but only " \
                   "version 0x%x is installed." \
                   % (ext.windows_h_version, self.windows_h_version)

        for h in ext.optional_headers:
            for d in self.include_dirs:
                if os.path.isfile(os.path.join(d, h)):
                    break
            else:
                return "The header '%s' can not be located" % (h,)

        common_dirs = self.compiler.library_dirs[:]
        common_dirs += os.environ.get("LIB").split(os.pathsep)
        patched_libs = []
        for lib in ext.libraries:
            if self.found_libraries.has_key(lib.lower()):
                found = self.found_libraries[lib.lower()]
            else:
                look_dirs = common_dirs + ext.library_dirs
                found = self.compiler.find_library_file(look_dirs, lib, self.debug)
                if not found:
                    return "No library '%s'" % lib
                self.found_libraries[lib.lower()] = found
            patched_libs.append(os.path.splitext(os.path.basename(found))[0])
        # axdebug struggles under debug builds - worry about that when I care :)
        if sys.hexversion < 0x2040000 and ext.name == 'axdebug' and self.debug:
            return "axdebug doesn't build in VC6 debug builds (irony!)"
        
        # We update the .libraries list with the resolved library name.
        # This is really only so "_d" works.
        ext.libraries = patched_libs
        return None # no reason - it can be built!

    def build_extensions(self):
        # Is there a better way than this?
        # Just one GUIDS.CPP and it gives trouble on mainwin too
        # Maybe I should just rename the file, but a case-only rename is likely to be
        # worse!
        if ".CPP" not in self.compiler.src_extensions:
            self.compiler._cpp_extensions.append(".CPP")
            self.compiler.src_extensions.append(".CPP")

        # First, sanity-check the 'extensions' list
        self.check_extensions_list(self.extensions)

        self.found_libraries = {}        

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
            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError, "Not a win32 package!"
            self.build_exefile(ext)

        # Not sure how to make this completely generic, and there is no
        # need at this stage.
        path = 'pythonwin\\Scintilla'
        makefile = 'makefile_pythonwin'
        makeargs = ["QUIET=1"]
        if self.debug:
            makeargs.append("DEBUG=1")
        if not self.verbose:
            makeargs.append("/C") # nmake: /C Suppress output messages
        # We build the DLL into our own temp directory, then copy it to the
        # real directory - this avoids the generated .lib/.exp
        build_temp = os.path.abspath(os.path.join(self.build_temp, "scintilla"))
        self.mkpath(build_temp)
        makeargs.append("SUB_DIR_O=%s" % build_temp)
        makeargs.append("SUB_DIR_BIN=%s" % build_temp)

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
        if not hasattr(ext, "depends"):
            ext.depends = [] # 2.2 doesn't have this
        depends = sources + ext.depends
        if not (self.force or newer_group(depends, ext_filename, 'newer')):
            log.debug("skipping '%s' executable (up-to-date)", ext.name)
            return
        else:
            log.info("building '%s' executable", ext.name)

        # First, scan the sources for SWIG definition files (.i), run
        # SWIG on 'em to create .c files, and modify the sources list
        # accordingly.
        sources = self.swig_sources(sources)

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
            return
        self.current_extension = ext

        if not self.mingw32 and ext.pch_header:
            ext.extra_compile_args = ext.extra_compile_args or []
            ext.extra_compile_args.append("/YX"+ext.pch_header)
            pch_name = os.path.join(self.build_temp, ext.name) + ".pch"
            ext.extra_compile_args.append("/Fp"+pch_name)

        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        if sys.version_info < (2,3):
            # 2.3+ - Wrong dir, numbered name
            self.build_temp = os.path.join(self.build_temp, ext.name)
        self.swig_cpp = True
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
        if name == "pywin32_system32.pywintypes":
            extra = self.debug and "_d.dll" or ".dll"
            return r"pywin32_system32\pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name == "pywin32_system32.pythoncom":
            extra = self.debug and "_d.dll" or ".dll"
            return r"pywin32_system32\pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name.endswith("win32.perfmondata"):
            extra = self.debug and "_d.dll" or ".dll"
            return r"win32\perfmondata" + extra
        elif name.endswith("win32.win32popenWin9x"):
            extra = self.debug and "_d.exe" or ".exe"
            return r"win32\win32popenWin9x" + extra
        elif name.endswith("win32.pythonservice"):
            extra = self.debug and "_d.exe" or ".exe"
            return r"win32\pythonservice" + extra
        elif name.endswith("pythonwin.Pythonwin"):
            extra = self.debug and "_d.exe" or ".exe"
            return r"pythonwin\Pythonwin" + extra
        elif name.endswith("isapi.PyISAPI_loader"):
            extra = self.debug and "_d.dll" or ".dll"
            return r"isapi\PyISAPI_loader" + extra
        return build_ext.get_ext_filename(self, name)

    def get_export_symbols(self, ext):
        if ext.is_regular_dll:
            return ext.export_symbols
        return build_ext.get_export_symbols(self, ext)

    def find_swig (self):
        # We know where swig is
        os.environ["SWIG_LIB"] = os.path.abspath(r"swig\swig_lib")
        return os.path.abspath(r"swig\swig.exe")

    def swig_sources (self, sources):
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

class my_compiler(msvccompiler.MSVCCompiler):
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
        ("win32api", "user32 advapi32 shell32 version", False, 0x0500),
        ("win32file", "oleaut32", False, 0x0500),
        ("win32event", "user32", False),
        ("win32clipboard", "gdi32 user32 shell32", False),
        ("win32evtlog", "advapi32 oleaut32", False),
        # win32gui handled below
        ("win32help", "htmlhelp user32 advapi32", False, 0x0500),
        ("win32lz", "lz32", False),
        ("win32net", "netapi32", True),
        ("win32pdh", "", False),
        ("win32pipe", "", False),
        ("win32print", "winspool user32", False),
        ("win32process", "advapi32 user32", False, 0x0500),
        ("win32ras", "rasapi32 user32", False),
        ("win32security", "advapi32 user32", True),
        ("win32service", "advapi32 oleaut32 user32", True, 0x0500),
        ("win32trace", "advapi32", False),
        ("win32wnet", "netapi32 mpr", False),
        ("win32inet", "wininet", False),
    ):

    name, lib_names, is_unicode = info[:3]
    if len(info)>3:
        windows_h_ver = info[3]
    else:
        windows_h_ver = None
    extra_compile_args = []
    if is_unicode:
        extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT']
    ext = WinExt_win32(name, 
                 libraries=lib_names,
                 extra_compile_args = extra_compile_args,
                 windows_h_version = windows_h_ver)
    win32_extensions.append(ext)

# The few that need slightly special treatment
win32_extensions += [
    WinExt_win32("win32gui", 
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           define_macros = [("WIN32GUI", None)],
        ),
    # winxpgui is built from win32gui.i, but sets up different #defines before
    # including windows.h.  It also has an XP style manifest.
    WinExt_win32("winxpgui",
           sources = ["win32/src/winxpgui.rc"],
           dsp_file = "win32/win32gui.dsp",
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           define_macros = [("WIN32GUI",None), ("WINXPGUI",None)],
           extra_swig_commands=["-DWINXPGUI"],
        ),
    WinExt_win32('servicemanager',
           extra_compile_args = ['-DUNICODE', '-D_UNICODE', 
                                 '-DWINNT', '-DPYSERVICE_BUILD_DLL'],
           libraries = "user32 ole32 advapi32 shell32",
           dsp_file = r"win32\Pythonservice servicemanager.dsp",
           windows_h_version = 0x500),
]

# The COM modules.
pythoncom = WinExt_system32('pythoncom',
                   dsp_file=r"com\win32com.dsp",
                   libraries = "oleaut32 ole32 user32",
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   pch_header = "stdafx.h",
                   windows_h_version = 0x500,
                   )
com_extensions = [pythoncom]
com_extensions += [
    WinExt_win32com('adsi', libraries="ACTIVEDS ADSIID"),
    WinExt_win32com('axcontrol', pch_header="axcontrol_pch.h"),
    WinExt_win32com('axscript',
            dsp_file=r"com\Active Scripting.dsp",
            extra_compile_args = ['-DPY_BUILD_AXSCRIPT'],
            pch_header = "stdafx.h"
    ),
    # ActiveDebugging is a mess.  See the comments in the docstring of this
    # module for details on getting it built.
    WinExt_win32com('axdebug',
            dsp_file=r"com\Active Debugging.dsp",
            libraries="axscript ad1",
            pch_header = "stdafx.h",
            optional_headers = ["activdbg.h"],
    ),
    WinExt_win32com('internet'),
    WinExt_win32com('mapi', libraries="mapi32", pch_header="PythonCOM.h"),
    WinExt_win32com_mapi('exchange',
                         libraries="""version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com_mapi('exchdapi',
                         # This still needs work for vs.net.
                         libraries="""DAPI ADDRLKUP exchinst EDKCFG EDKUTILS
                                      EDKMAPI mapi32 version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com('shell', libraries='shell32', pch_header="shell_pch.h"),
    WinExt_win32com('taskscheduler', libraries='mstask'),
    WinExt_win32com('ifilter', libraries='ntquery'),
]

pythonwin_extensions = [
    WinExt_pythonwin("win32ui", extra_compile_args = ['-DBUILD_PYW'],
                     pch_header="stdafx.h"),
    WinExt_pythonwin("win32uiole", pch_header="stdafxole.h",
                     windows_h_version = 0x500),
    WinExt_pythonwin("dde", pch_header="stdafxdde.h"),
]

other_extensions = [
    WinExt_ISAPI('PyISAPI_loader',
           sources=[os.path.join("isapi", "src", s) for s in
                   """PyExtensionObjects.cpp PyFilterObjects.cpp
                      pyISAPI.cpp PythonEng.cpp StdAfx.cpp
                      Utils.cpp
                   """.split()],
           pch_header = "StdAfx.h",
           is_regular_dll = 1,
           export_symbols = """HttpExtensionProc GetExtensionVersion
                               TerminateExtension GetFilterVersion
                               HttpFilterProc TerminateFilter
                               PyISAPISetOptions""".split(),
           ),
]

W32_exe_files = [
    WinExt_win32("win32popenWin9x",
                 libraries = "user32"),
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
    'PyIDistList':          'IMAPIProp',
    'PyIMailUser':          'IMAPIContainer',
    'PyIMAPIContainer':     'IMAPIProp',
    'PyIMAPIFolder':        'IMAPIContainer',
    'PyIMAPIProp':          '', # '' == default base
    'PyIMAPISession':       '',
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
    'PyIADsUser':           'IDispatch',
    'PyIDirectoryObject':   '',
    'PyIDirectorySearch':   '',
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

          'win32comext.shell',
          'win32comext.mapi',
          'win32comext.ifilter',
          'win32comext.internet',
          'win32comext.axcontrol',
          'win32comext.taskscheduler',

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
          ]

# Python 2.2 distutils can't handle py_modules *and* packages,
# but putting 'win32.lib' as a package whinges there is no __init__
if sys.version_info < (2,3):
    packages.append('win32.lib')
    py_modules = None
else:
    py_modules = expand_modules("win32\\lib")

dist = setup(name="pywin32",
      version=build_number,
      description="Python for Window Extensions",
      long_description="Python extensions for Microsoft Windows\n"
                       "Provides access to much of the Win32 API, the\n"
                       "ability to create and use COM objects, and the\n"
                       "Pythonwin environment.",
      author="Mark Hammond (et al)",
      author_email = "mhammond@users.sourceforge.net",
      url="http://sourceforge.net/projects/pywin32/",
      license="PSA",
      cmdclass = { #'install_lib': my_install_lib,
                   'build_ext': my_build_ext,
                   'install_data': my_install_data,
                   },
      options = {"bdist_wininst":
                    {"install_script": "pywin32_postinstall.py",
                     "pre_install_script": "pywin32_preinstall.py",
                     "title": "pywin32 extensions (build %s)" % (build_number,),
                    },
                },

      scripts = ["pywin32_postinstall.py"],

      ext_modules = win32_extensions + com_extensions + pythonwin_extensions + \
                    other_extensions,

      package_dir = {"win32com": "com/win32com",
                     "win32comext": "com/win32comext",
                     "pythonwin": "pythonwin",},
      packages = packages,
      py_modules = py_modules,

      data_files=convert_optional_data_files([
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
                'com/win32comext/shell/test/*.py',
                'com/win32comext/shell/demos/servers/*.py',
                'com/win32comext/shell/demos/*.py',
                'com/win32comext/taskscheduler/test/*.py',
                'com/win32comext/ifilter/demo/*.py',
                'isapi/*.txt',
                'isapi/samples/*.py',
                'isapi/samples/*.txt',
                'isapi/doc/*.html',
                'isapi/test/*.py',
                'isapi/test/*.txt',
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

# Custom script we run at the end of installing - this is the same script
# run by bdist_wininst, but the standard 'install' command doesn't seem
# to have such a concept.
# This child process won't be able to install the system DLLs until our
# process has terminated (as distutils imports win32api!), so we must use
# some 'no wait' executor - spawn seems fine!  We pass the PID of this
# process so the child will wait for us.
# XXX - hmm - a closer look at distutils shows it only uses win32api
# if _winreg fails - and this never should.  Need to revisit this!
if not dist.dry_run and dist.command_obj.has_key('install') \
       and not dist.command_obj.has_key('bdist_wininst'):
    # What executable to use?  This one I guess.
    filename = os.path.join(
                  os.path.dirname(sys.argv[0]), "pywin32_postinstall.py")
    if not os.path.isfile(filename):
        raise RuntimeError, "Can't find pywin32_postinstall.py"
    print "Executing post install script..."
    os.spawnl(os.P_NOWAIT, sys.executable,
              sys.executable, filename,
              "-quiet", "-wait", str(os.getpid()), "-install")
