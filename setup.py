build_id = "305.1"  # may optionally include a ".{patchno}" suffix.

__doc__ = """This is a distutils setup-script for the pywin32 extensions.

The canonical source of truth for supported versions and build environments
is [the github CI](https://github.com/mhammond/pywin32/tree/main/.github/workflows).

To build and install locally for testing etc, you need a build environment
which is capable of building the version of Python you are targeting, then:
  python setup.py -q install

For a debug (_d) version, you need a local debug build of Python, but must use
the release version executable for the build. eg:
  python setup.py -q build --debug install

Cross-compilation from x86 to ARM is well supported (assuming installed vs tools etc) - eg:
  python setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64

Some modules require special SDKs or toolkits to build (eg, mapi/exchange),
which often aren't available in CI. The build process treats them as optional -
instead of a failing, it will report what was skipped, and why. See also
build_env.md, which is getting out of date but might help getting everything
required for an official build - see README.md for that process.
"""
# Originally by Thomas Heller, started in 2000 or so.
import os
import sys
import glob
import re
from tempfile import gettempdir
import platform
import shutil
import subprocess

import winreg

# The rest of our imports.
from setuptools import setup
from distutils.core import Extension
from distutils.command.install import install
from distutils.command.install_lib import install_lib
from setuptools.command.build_ext import build_ext
from distutils.command.build import build
from distutils.command.install_data import install_data

from distutils import log


# some modules need a static CRT to avoid problems caused by them having a
# manifest.
static_crt_modules = ["winxpgui"]


from distutils.dep_util import newer_group
from distutils.filelist import FileList
import distutils.util

build_id_patch = build_id
if not "." in build_id_patch:
    build_id_patch = build_id_patch + ".0"
pywin32_version = "%d.%d.%s" % (
    sys.version_info[0],
    sys.version_info[1],
    build_id_patch,
)
print("Building pywin32", pywin32_version)

try:
    sys.argv.remove("--skip-verstamp")
    skip_verstamp = True
except ValueError:
    skip_verstamp = False

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
dll_base_address = 0x1E200000


class WinExt(Extension):
    # Base class for all win32 extensions, with some predefined
    # library and include dirs, and predefined windows libraries.
    # Additionally a method to parse .def files into lists of exported
    # symbols, and to read
    def __init__(
        self,
        name,
        sources,
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
        windows_h_version=None,  # min version of windows.h needed.
        extra_swig_commands=None,
        is_regular_dll=False,  # regular Windows DLL?
        # list of headers which may not be installed forcing us to
        # skip this extension
        optional_headers=[],
        base_address=None,
        depends=None,
        platforms=None,  # none means 'all platforms'
        implib_name=None,
        delay_load_libraries="",
    ):
        include_dirs = ["com/win32com/src/include", "win32/src"] + include_dirs
        libraries = libraries.split()
        self.delay_load_libraries = delay_load_libraries.split()
        libraries.extend(self.delay_load_libraries)

        extra_link_args = extra_link_args or []
        if export_symbol_file:
            extra_link_args.append("/DEF:" + export_symbol_file)

        # Some of our swigged files behave differently in distutils vs
        # MSVC based builds.  Always define DISTUTILS_BUILD so they can tell.
        define_macros = define_macros or []
        define_macros.append(("DISTUTILS_BUILD", None))
        define_macros.append(("_CRT_SECURE_NO_WARNINGS", None))
        # CRYPT_DECRYPT_MESSAGE_PARA.dwflags is in an ifdef for some unknown reason
        # See github PR #1444 for more details...
        define_macros.append(("CRYPT_DECRYPT_MESSAGE_PARA_HAS_EXTRA_FIELDS", None))
        self.pch_header = pch_header
        self.extra_swig_commands = extra_swig_commands or []
        self.windows_h_version = windows_h_version
        self.optional_headers = optional_headers
        self.is_regular_dll = is_regular_dll
        self.base_address = base_address
        self.platforms = platforms
        self.implib_name = implib_name
        Extension.__init__(
            self,
            name,
            sources,
            include_dirs,
            define_macros,
            undef_macros,
            library_dirs,
            libraries,
            runtime_library_dirs,
            extra_objects,
            extra_compile_args,
            extra_link_args,
            export_symbols,
        )
        self.depends = depends or []  # stash it here, as py22 doesn't have it.

    def finalize_options(self, build_ext):
        # distutils doesn't define this function for an Extension - it is
        # our own invention, and called just before the extension is built.
        if not build_ext.mingw32:
            if self.pch_header:
                self.extra_compile_args = self.extra_compile_args or []

            # bugger - add this to python!
            if build_ext.plat_name == "win32":
                self.extra_link_args.append("/MACHINE:x86")
            else:
                self.extra_link_args.append("/MACHINE:%s" % build_ext.plat_name[4:])

            # like Python, always use debug info, even in release builds
            # (note the compiler doesn't include debug info, so you only get
            # basic info - but its better than nothing!)
            # For now use the temp dir - later we may package them, so should
            # maybe move them next to the output file.
            pch_dir = os.path.join(build_ext.build_temp)
            if not build_ext.debug:
                self.extra_compile_args.append("/Zi")
            self.extra_compile_args.append("/Fd%s\\%s_vc.pdb" % (pch_dir, self.name))
            self.extra_link_args.append("/DEBUG")
            self.extra_link_args.append("/PDB:%s\\%s.pdb" % (pch_dir, self.name))
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
                        self.extra_compile_args.append(
                            '/DMFC_OCC_IMPL_H=\\"%s\\"' % candidate
                        )
                        found_mfc = True
                        break
                if found_mfc:
                    break

        self.extra_compile_args.append("-DUNICODE")
        self.extra_compile_args.append("-D_UNICODE")
        self.extra_compile_args.append("-DWINNT")


class WinExt_pythonwin(WinExt):
    def __init__(self, name, **kw):
        kw.setdefault("extra_compile_args", []).extend(["-D_AFXDLL", "-D_AFXEXT"])

        WinExt.__init__(self, name, **kw)

    def get_pywin32_dir(self):
        return "pythonwin"


class WinExt_pythonwin_subsys_win(WinExt_pythonwin):
    def finalize_options(self, build_ext):
        WinExt_pythonwin.finalize_options(self, build_ext)

        if build_ext.mingw32:
            self.extra_link_args.append("-mwindows")
        else:
            self.extra_link_args.append("/SUBSYSTEM:WINDOWS")

            # Unicode, Windows executables seem to need this magic:
            self.extra_link_args.append("/ENTRY:wWinMainCRTStartup")


class WinExt_win32(WinExt):
    def __init__(self, name, **kw):
        WinExt.__init__(self, name, **kw)

    def get_pywin32_dir(self):
        return "win32"


class WinExt_ISAPI(WinExt):
    def get_pywin32_dir(self):
        return "isapi"


# Note this is used only for "win32com extensions", not pythoncom
# itself - thus, output is "win32comext"
class WinExt_win32com(WinExt):
    def __init__(self, name, **kw):
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
    def __init__(self, name, **kw):
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
            pass  # this version doesn't support 64 bits, so must already be using 32bit key.
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
        if distutils.util.get_platform() in ["win-amd64", "win-arm64"]:
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


class WinExt_pythonservice(WinExt):
    # special handling because it's a "console" exe.
    def finalize_options(self, build_ext):
        WinExt.finalize_options(self, build_ext)

        if build_ext.mingw32:
            self.extra_link_args.append("-mconsole")
            self.extra_link_args.append("-municode")
        else:
            self.extra_link_args.append("/SUBSYSTEM:CONSOLE")

    # pythonservice.exe goes in win32, where it doesn't actually work, but
    # win32serviceutil manages to copy it to where it does.
    def get_pywin32_dir(self):
        return "win32"


################################################################
# Extensions to the distutils commands.

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

        self.plat_dir = {
            "win-amd64": "x64",
            "win-arm64": "arm64",
        }.get(self.plat_name, "x86")

        self.windows_h_version = None

        # The pywintypes library is created in the build_temp
        # directory, so we need to add this to library_dirs
        self.library_dirs.append(self.build_temp)
        self.mingw32 = self.compiler == "mingw32"
        if self.mingw32:
            self.libraries.append("stdc++")

        self.excluded_extensions = []  # list of (ext, why)
        self.swig_cpp = True  # hrm - deprecated - should use swig_opts=-c++??

    def _why_cant_build_extension(self, ext):
        # Return None, or a reason it can't be built.
        # Exclude exchange 32-bit utility libraries from 64-bit
        # builds. Note that the exchange module now builds, but only
        # includes interfaces for 64-bit builds.
        if self.plat_name in ["win-amd64", "win-arm64"] and ext.name == "exchdapi":
            return "No 64-bit library for utility functions available."
        if ext.name == "exchdapi":
            return "Haven't worked out how to build on vs2015"
        # axdebug fails to build on 3.11 due to Python "frame" objects changing.
        # This could be fixed, but is almost certainly not in use any more, so
        # just skip it.
        if ext.name == "axdebug" and sys.version_info > (3, 10):
            return "AXDebug no longer builds on 3.11 and up"

        # winxpgui cannot be build for win-arm64 due to manifest file conflicts
        # skip extension as we probably don't want this extension for win-arm64 platforms
        if self.plat_name == "win-arm64" and ext.name == "winxpgui":
            return "winxpgui extension cannot be build for win-arm64"

        include_dirs = self.compiler.include_dirs + os.environ.get("INCLUDE", "").split(
            os.pathsep
        )
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
                log.debug("Header '%s' not found  in %s", h, look_dirs)
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
                    log.debug("Lib '%s' not found in %s", lib, look_dirs)
                    return "No library '%s'" % lib
                self.found_libraries[lib.lower()] = found
            patched_libs.append(os.path.splitext(os.path.basename(found))[0])

        if ext.platforms and self.plat_name not in ext.platforms:
            return "Only available on platforms %s" % (ext.platforms,)

        # We update the .libraries list with the resolved library name.
        # This is really only so "_d" works.
        ext.libraries = patched_libs
        return None  # no reason - it can be built!

    def _build_scintilla(self):
        path = "pythonwin\\Scintilla"
        makefile = "makefile_pythonwin"
        makeargs = []

        if self.debug:
            makeargs.append("DEBUG=1")
        if not self.verbose:
            makeargs.append("/C")  # nmake: /C Suppress output messages
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
            cs = os.environ.get("comspec", "cmd.exe")
            cmd = cs + ' /c for %I in ("' + build_temp + '",) do @echo %~sI'
            build_temp = os.popen(cmd).read().strip()
            assert os.path.isdir(build_temp), build_temp
        makeargs.append("SUB_DIR_O=%s" % build_temp)
        makeargs.append("SUB_DIR_BIN=%s" % build_temp)
        makeargs.append("DIR_PYTHON=%s" % sys.prefix)

        nmake = "nmake.exe"
        # Attempt to resolve nmake to the same one that our compiler object
        # would use. compiler.spawn() ought to do this, but it does not search
        # its own PATH value for the initial command. It does, however, set it
        # correctly for any subsequent commands.
        try:
            for p in self.compiler._paths.split(os.pathsep):
                if os.path.isfile(os.path.join(p, nmake)):
                    nmake = os.path.join(p, nmake)
                    break
        except (AttributeError, TypeError):
            pass

        cwd = os.getcwd()
        old_env = os.environ.copy()
        os.environ["INCLUDE"] = os.pathsep.join(self.compiler.include_dirs)
        os.environ["LIB"] = os.pathsep.join(self.compiler.library_dirs)
        os.chdir(path)
        try:
            cmd = [nmake, "/nologo", "/f", makefile] + makeargs
            self.compiler.spawn(cmd)
        finally:
            os.chdir(cwd)
            os.environ["INCLUDE"] = old_env.get("INCLUDE", "")
            os.environ["LIB"] = old_env.get("LIB", "")

        # The DLL goes in the Pythonwin directory.
        if self.debug:
            base_name = "scintilla_d.dll"
        else:
            base_name = "scintilla.dll"
        self.copy_file(
            os.path.join(self.build_temp, "scintilla", base_name),
            os.path.join(self.build_lib, "pythonwin"),
        )

    # find the VC base path corresponding to distutils paths, and
    # potentially upgrade for extra include / lib paths (MFC)
    def _check_vc(self):
        vcbase = vcverdir = None
        atlmfc_found = False
        for _dir in self.compiler.library_dirs:
            m = re.search(r"(?i)VC\\([\d.]+\\)?(LIB)\b", _dir)
            if m and not vcbase:
                vcbase = _dir[: m.start(2)]
                vcverdir = m.group(1)
            m = re.search(r"(?i)ATLMFC\\LIB\b", _dir)
            if m:
                atlmfc_found = True  # ATLMFC libs/includes already found by distutils

        if not vcbase and not self.mingw32:
            print("-- compiler.library_dirs:", self.compiler.library_dirs)
            # Error or warn? last hope would be a non-standard build environment
            print("-- Visual C base path not found !?")

        # The afxres.h/atls.lib files aren't always included by default,
        # so find and add them
        if vcbase and not atlmfc_found:
            atls_lib = glob.glob(
                vcbase + r"ATLMFC\lib\{}\atls.lib".format(self.plat_dir)
            )
            if atls_lib:
                self.library_dirs.append(os.path.dirname(atls_lib[0]))
                self.include_dirs.append(
                    os.path.join(
                        os.path.dirname(os.path.dirname(os.path.dirname(atls_lib[0]))),
                        "Include",
                    )
                )
            else:
                print("-- compiler.library_dirs:", self.compiler.library_dirs)
                print("-- ATLMFC paths likely missing (Required for win32ui)")
        return vcbase, vcverdir

    def build_extensions(self):
        # First, sanity-check the 'extensions' list
        self.check_extensions_list(self.extensions)

        self.found_libraries = {}

        if hasattr(self.compiler, "initialize") and not self.compiler.initialized:
            self.compiler.initialize()

        # XXX this distutils class var peek hack should become obsolete
        # (silently) when https://github.com/pypa/distutils/pull/172 is
        # resolved.
        # _why_cant_build_extension() and _build_scintilla() at least need
        # complete VC+SDK inspectable inc / lib dirs.
        classincs = getattr(self.compiler.__class__, "include_dirs", [])
        if classincs:
            print("-- distutils hack to expose all include & lib dirs")
            print("-- orig compiler.include_dirs:", self.compiler.include_dirs)
            print("-- orig compiler.library_dirs:", self.compiler.library_dirs)
            self.compiler.include_dirs += classincs
            self.compiler.__class__.include_dirs = []
            classlibs = getattr(self.compiler.__class__, "library_dirs", [])
            self.compiler.library_dirs += classlibs
            self.compiler.__class__.library_dirs = []
        else:
            print("-- FIX ME ! distutils may expose complete inc/lib dirs again")

        vcbase, vcverdir = self._check_vc()

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
            self.package = ext.get_pywin32_dir()
            ext.finalize_options(self)
            why = self._why_cant_build_extension(ext)
            if why is not None:
                self.excluded_extensions.append((ext, why))
                assert why, "please give a reason, or None"
                print("Skipping %s: %s" % (ext.name, why))
                continue
            self.build_exefile(ext)

        # Error when too many skips
        if len(self.excluded_extensions) > 0.3 * (
            len(self.extensions) + len(W32_exe_files)
        ):
            print("-- compiler.include_dirs:", self.compiler.include_dirs)
            print("-- compiler.library_dirs:", self.compiler.library_dirs)
            raise RuntimeError("Too many extensions skipped, check build environment")

        # Not sure how to make this completely generic, and there is no
        # need at this stage.
        self._build_scintilla()
        # Copy cpp lib files needed to create Python COM extensions
        clib_files = (
            ["win32", "pywintypes%s.lib"],
            ["win32com", "pythoncom%s.lib"],
            ["win32com", "axscript%s.lib"],
        )
        for clib_file in clib_files:
            target_dir = os.path.join(self.build_lib, clib_file[0], "libs")
            if not os.path.exists(target_dir):
                self.mkpath(target_dir)
            suffix = ""
            if self.debug:
                suffix = "_d"
            fname = clib_file[1] % suffix
            self.copy_file(os.path.join(self.build_temp, fname), target_dir)

        # Finally find and copy the MFC redistributable DLLs.
        win32ui_ext = pythonwin_extensions[0]
        if win32ui_ext not in set(self.extensions) - {
            ext for ext, why in self.excluded_extensions
        }:
            return
        if not vcbase:
            raise RuntimeError("Can't find MFC redist DLLs with unkown VC base path")
        redist_globs = [vcbase + r"redist\%s\*MFC\mfc140u.dll" % self.plat_dir]
        m = re.search(r"\\VC\\Tools\\", vcbase)
        if m:
            # typical path on newer Visual Studios - ensure corresponding version
            redist_globs.append(
                vcbase[: m.start()]
                + r"\VC\Redist\MSVC\%s%s\*\mfc140u.dll"
                % (vcverdir or "*\\", self.plat_dir)
            )
        # Only mfcNNNu DLL is required (mfcmNNNX is Windows Forms, rest is ANSI)
        mfc_contents = next(filter(None, map(glob.glob, redist_globs)), [])[:1]
        if not mfc_contents:
            raise RuntimeError("MFC redist DLLs not found like %r!" % redist_globs)

        target_dir = os.path.join(self.build_lib, win32ui_ext.get_pywin32_dir())
        for mfc_content in mfc_contents:
            self.copy_file(mfc_content, target_dir)

    def build_exefile(self, ext):
        _d = self.debug and "_d" or ""

        log.info("building exe '%s'", ext.name)
        leaf_name = f"{ext.get_pywin32_dir()}\\{ext.name}{_d}.exe"
        full_name = os.path.join(self.build_lib, leaf_name)

        sources = list(ext.sources)
        depends = sources + ext.depends
        # unclear why we need to check this!?
        if not (self.force or newer_group(depends, full_name, "newer")):
            log.debug("skipping '%s' executable (up-to-date)", ext.name)
            return
        else:
            log.info("building '%s' executable", ext.name)

        objects = self.compiler.compile(
            sources,
            output_dir=os.path.join(self.build_temp, ext.name),
            include_dirs=ext.include_dirs,
            debug=self.debug,
            extra_postargs=ext.extra_compile_args,
            depends=ext.depends,
        )

        self.compiler.link(
            "executable",
            objects,
            full_name,
            libraries=self.get_libraries(ext),
            library_dirs=ext.library_dirs,
            runtime_library_dirs=ext.runtime_library_dirs,
            extra_postargs=ext.extra_link_args,
            debug=self.debug,
            build_temp=self.build_temp,
        )

    def build_extension(self, ext):
        # Some of these extensions are difficult to build, requiring various
        # hard-to-track libraries et (eg, exchange sdk, etc).  So we
        # check the extension list for the extra libraries explicitly
        # listed.  We then search for this library the same way the C
        # compiler would - if we can't find a library, we exclude the
        # extension from the build.
        # Note we can't do this in advance, as some of the .lib files
        # we depend on may be built as part of the process - thus we can
        # only check an extension's lib files as we are building it.
        why = self._why_cant_build_extension(ext)
        if why is not None:
            assert why, "please give a reason, or None"
            self.excluded_extensions.append((ext, why))
            print("Skipping %s: %s" % (ext.name, why))
            return
        self.current_extension = ext

        ext.finalize_options(self)

        # ensure the SWIG .i files are treated as dependencies.
        for source in ext.sources:
            if source.endswith(".i"):
                self.find_swig()  # for the side-effect of the environment value.
                # Find the swig_lib .i files we care about for dependency tracking.
                ext.swig_deps = glob.glob(
                    os.path.join(os.environ["SWIG_LIB"], "python", "*.i")
                )
                ext.depends.extend(ext.swig_deps)
                break
        else:
            ext.swig_deps = None

        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        want_static_crt = ext.name in static_crt_modules
        if want_static_crt:
            self.compiler.compile_options.remove("/MD")
            self.compiler.compile_options.append("/MT")
            self.compiler.compile_options_debug.remove("/MDd")
            self.compiler.compile_options_debug.append("/MTd")

        try:
            build_ext.build_extension(self, ext)
            # Convincing distutils to create .lib files with the name we
            # need is difficult, so we just hack around it by copying from
            # the created name to the name we need.
            extra = self.debug and "_d.lib" or ".lib"
            if ext.name in ("pywintypes", "pythoncom"):
                # The import libraries are created as PyWinTypes23.lib, but
                # are expected to be pywintypes.lib.
                created = "%s%d%d%s" % (
                    ext.name,
                    sys.version_info[0],
                    sys.version_info[1],
                    extra,
                )
                needed = "%s%s" % (ext.name, extra)
            elif ext.name in ("win32ui",):
                # This one just needs a copy.
                created = needed = ext.name + extra
            else:
                created = needed = None
            if created is not None:
                # To keep us on our toes, MSVCCompiler constructs the .lib files
                # in the same directory as the first source file's object file:
                #    os.path.dirname(objects[0])
                # rather than in the self.build_temp directory
                src = os.path.join(
                    old_build_temp, os.path.dirname(ext.sources[0]), created
                )
                dst = os.path.join(old_build_temp, needed)
                if os.path.abspath(src) != os.path.abspath(dst):
                    self.copy_file(src, dst)
        finally:
            self.build_temp = old_build_temp
            if want_static_crt:
                self.compiler.compile_options.remove("/MT")
                self.compiler.compile_options.append("/MD")
                self.compiler.compile_options_debug.remove("/MTd")
                self.compiler.compile_options_debug.append("/MDd")

    def get_ext_filename(self, name):
        # We need to fixup some target filenames.
        _d = self.debug and "_d" or ""
        if name in ["pywintypes", "pythoncom"]:
            ver = f"{sys.version_info[0]}{sys.version_info[1]}"
            return f"{name}{ver}{_d}.dll"
        if name in ["perfmondata", "PyISAPI_loader"]:
            return f"{name}{_d}.dll"
        # everything else a .pyd - calling base-class might give us a more
        # complicated name, so return a simple one.
        return f"{name}{_d}.pyd"

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
        target_ext = ".cpp"
        for source in sources:
            (base, sext) = os.path.splitext(source)
            if sext == ".i":  # SWIG interface file
                if os.path.split(base)[1] in swig_include_files:
                    continue
                swig_sources.append(source)
                # Patch up the filenames for various special cases...
                if os.path.basename(base) in swig_interface_parents:
                    swig_targets[source] = base + target_ext
                elif (
                    self.current_extension.name == "winxpgui"
                    and os.path.basename(base) == "win32gui"
                ):
                    # More vile hacks.  winxpmodule is built from win32gui.i -
                    # just different #defines are setup for windows.h.
                    new_target = os.path.join(
                        os.path.dirname(base), "winxpgui_swig%s" % (target_ext,)
                    )
                    swig_targets[source] = new_target
                    new_sources.append(new_target)
                else:
                    new_target = "%s_swig%s" % (base, target_ext)
                    new_sources.append(new_target)
                    swig_targets[source] = new_target
            else:
                new_sources.append(source)

        if not swig_sources:
            return new_sources

        swig = self.find_swig()
        for source in swig_sources:
            swig_cmd = [swig, "-python", "-c++"]
            swig_cmd.append(
                "-dnone",
            )  # we never use the .doc files.
            swig_cmd.extend(self.current_extension.extra_swig_commands)
            if distutils.util.get_platform() in ["win-amd64", "win-arm64"]:
                swig_cmd.append("-DSWIG_PY64BIT")
            else:
                swig_cmd.append("-DSWIG_PY32BIT")
            target = swig_targets[source]
            try:
                interface_parent = swig_interface_parents[
                    os.path.basename(os.path.splitext(source)[0])
                ]
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
                        swig_cmd.extend(["-com_interface_parent", interface_parent])

            # This 'newer' check helps python 2.2 builds, which otherwise
            # *always* regenerate the .cpp files, meaning every future
            # build for any platform sees these as dirty.
            # This could probably go once we generate .cpp into the temp dir.
            fqsource = os.path.abspath(source)
            fqtarget = os.path.abspath(target)
            rebuild = self.force or (
                ext and newer_group(ext.swig_deps + [fqsource], fqtarget)
            )

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
            subprocess.Popen(
                [
                    sys.executable,
                    filename,
                    "-install",
                    "-destination",
                    self.install_lib,
                    "-quiet",
                    "-wait",
                    str(os.getpid()),
                ]
            )


class my_install_lib(install_lib):
    def install(self):
        # This is crazy - in setuptools 61.1.0 (and probably some earlier versions), the
        # install_lib and build comments don't agree on where the .py files to install can
        # be found, so we end up with a warning logged:
        # `warning: my_install_lib: 'build\lib.win-amd64-3.7' does not exist -- no Python modules to install`
        # (because they are actually in `build\lib.win-amd64-cpython-37`!)
        # It's not an error though, so we end up with .exe installers lacking our lib files!
        builder = self.get_finalized_command("build")
        if os.path.isdir(builder.build_platlib) and not os.path.isdir(self.build_dir):
            self.build_dir = builder.build_platlib
        # We want a failure to find .py files be an error rather than a warning.
        outfiles = super().install()
        if not outfiles:
            raise RuntimeError("No Python files were found to install")
        return outfiles


def my_new_compiler(**kw):
    if "compiler" in kw and kw["compiler"] in (None, "msvc"):
        return my_compiler()
    return orig_new_compiler(**kw)


# No way to cleanly wedge our compiler sub-class in.
from distutils import ccompiler
from distutils._msvccompiler import MSVCCompiler

orig_new_compiler = ccompiler.new_compiler
ccompiler.new_compiler = my_new_compiler

base_compiler = MSVCCompiler


class my_compiler(base_compiler):
    # Just one GUIDS.CPP and it gives trouble on mainwin too. Maybe I
    # should just rename the file, but a case-only rename is likely to be
    # worse!  This can probably go away once we kill the VS project files
    # though, as we can just specify the lowercase name in the module def.
    _cpp_extensions = base_compiler._cpp_extensions + [".CPP"]
    src_extensions = base_compiler.src_extensions + [".CPP"]

    def link(
        self,
        target_desc,
        objects,
        output_filename,
        output_dir=None,
        libraries=None,
        library_dirs=None,
        runtime_library_dirs=None,
        export_symbols=None,
        debug=0,
        *args,
        **kw,
    ):
        super().link(
            target_desc,
            objects,
            output_filename,
            output_dir,
            libraries,
            library_dirs,
            runtime_library_dirs,
            export_symbols,
            debug,
            *args,
            **kw,
        )
        # Here seems a good place to stamp the version of the built
        # target.  Do this externally to avoid suddenly dragging in the
        # modules needed by this process, and which we will soon try and
        # update.
        # Further, we don't really want to use sys.executable, because that
        # means the build environment must have a current pywin32 installed
        # in every version, which is a bit of a burden only for this.
        # So we assume the "default" Python version (ie, the version run by
        # py.exe) has pywin32 installed.
        # (This creates a chicken-and-egg problem though! We used to work around
        # this by ignoring failure to verstamp, but that's easy to miss. So now
        # allow --skip-verstamp on the cmdline - but if it's not there, the
        # verstamp must work.)
        if not skip_verstamp:
            args = ["py.exe", "-m" "win32verstamp"]
            args.append("--version=%s" % (pywin32_version,))
            args.append("--comments=https://github.com/mhammond/pywin32")
            args.append("--original-filename=%s" % (os.path.basename(output_filename),))
            args.append("--product=PyWin32")
            if "-v" not in sys.argv:
                args.append("--quiet")
            args.append(output_filename)
            try:
                self.spawn(args)
            except Exception:
                print("** Failed to versionstamp the binaries.")
                # py.exe is not yet available for windows-arm64 so version stamp will fail
                # ignore it for now
                if platform.machine() != "ARM64":
                    print(
                        "** If you want to skip this step, pass '--skip-verstamp' on the command-line"
                    )
                    raise

    # Work around bpo-36302/bpo-42009 - it sorts sources but this breaks
    # support for building .mc files etc :(
    def compile(self, sources, **kwargs):
        # re-sort the list of source files but ensure all .mc files come first.
        def key_reverse_mc(a):
            b, e = os.path.splitext(a)
            e = "" if e == ".mc" else e
            return (e, b)

        sources = sorted(sources, key=key_reverse_mc)
        return MSVCCompiler.compile(self, sources, **kwargs)

    def spawn(self, cmd):
        is_link = cmd[0].endswith("link.exe") or cmd[0].endswith('"link.exe"')
        is_mt = cmd[0].endswith("mt.exe") or cmd[0].endswith('"mt.exe"')
        if is_mt:
            # We don't want mt.exe run...
            return
        if is_link:
            # remove /MANIFESTFILE:... and add MANIFEST:NO
            # (but note that for winxpgui, which specifies a manifest via a
            # .rc file, this is ignored by the linker - the manifest specified
            # in the .rc file is still added)
            for i in range(len(cmd)):
                if cmd[i].startswith(("/MANIFESTFILE:", "/MANIFEST:EMBED")):
                    cmd[i] = "/MANIFEST:NO"
                    break
        if is_mt:
            # We want mt.exe run with the original manifest
            for i in range(len(cmd)):
                if cmd[i] == "-manifest":
                    cmd[i + 1] = cmd[i + 1] + ".orig"
                    break
        super().spawn(cmd)
        if is_link:
            # We want a copy of the original manifest so we can use it later.
            for i in range(len(cmd)):
                if cmd[i].startswith("/MANIFESTFILE:"):
                    mfname = cmd[i][14:]
                    shutil.copyfile(mfname, mfname + ".orig")
                    break

    # CCompiler's implementations of these methods completely replace the values
    # determined by the build environment. This seems like a design that must
    # always have been broken, but we work around it here.
    def set_include_dirs(self, dirs):
        self.include_dirs[:0] = dirs

    def set_library_dirs(self, dirs):
        self.library_dirs[:0] = dirs

    def set_libraries(self, libs):
        self.libraries.extend(libs)


################################################################


class my_install_data(install_data):
    """A custom install_data command, which will install it's files
    into the standard directories (normally lib/site-packages).
    """

    def finalize_options(self):
        if self.install_dir is None:
            installobj = self.distribution.get_command_obj("install")
            self.install_dir = installobj.install_lib
        print("Installing data files to %s" % self.install_dir)
        install_data.finalize_options(self)


################################################################

pywintypes = WinExt_system32(
    "pywintypes",
    sources=[
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
    depends=[
        "win32/src/PyWinObjects.h",
        "win32/src/PyWinTypes.h",
        "win32/src/PySoundObjects.h",
        "win32/src/PySecurityObjects.h",
    ],
    extra_compile_args=["-DBUILD_PYWINTYPES"],
    libraries="advapi32 user32 ole32 oleaut32",
    pch_header="PyWinTypes.h",
)

win32_extensions = [pywintypes]

win32_extensions.append(
    WinExt_win32(
        "perfmondata",
        sources=[
            "win32/src/PerfMon/PyPerfMsgs.mc",
            "win32/src/PerfMon/perfmondata.cpp",
        ],
        libraries="advapi32",
        export_symbol_file="win32/src/PerfMon/perfmondata.def",
        is_regular_dll=1,
        depends=[
            "win32/src/PerfMon/perfutil.h",
            "win32/src/PerfMon/PyPerfMonControl.h",
        ],
    ),
)

for info in (
    # (name, libraries, WINVER, sources)
    ("mmapfile", "", None, "win32/src/mmapfilemodule.cpp"),
    ("odbc", "odbc32 odbccp32", None, "win32/src/odbc.cpp"),
    (
        "perfmon",
        "",
        None,
        """
            win32/src/PerfMon/MappingManager.cpp
            win32/src/PerfMon/PerfCounterDefn.cpp
            win32/src/PerfMon/PerfObjectType.cpp
            win32/src/PerfMon/PyPerfMon.cpp
            """,
    ),
    ("timer", "user32", None, "win32/src/timermodule.cpp"),
    ("win32cred", "AdvAPI32 credui", 0x0501, "win32/src/win32credmodule.cpp"),
    (
        "win32crypt",
        "Crypt32 Advapi32",
        0x0500,
        """
            win32/src/win32crypt/win32cryptmodule.cpp
            win32/src/win32crypt/win32crypt_structs.cpp
            win32/src/win32crypt/PyCERTSTORE.cpp
            win32/src/win32crypt/PyCERT_CONTEXT.cpp
            win32/src/win32crypt/PyCRYPTHASH.cpp
            win32/src/win32crypt/PyCRYPTKEY.cpp
            win32/src/win32crypt/PyCRYPTMSG.cpp
            win32/src/win32crypt/PyCRYPTPROV.cpp
            win32/src/win32crypt/PyCTL_CONTEXT.cpp
            """,
    ),
    (
        "win32file",
        "",
        0x0500,
        """
              win32/src/win32file.i
              win32/src/win32file_comm.cpp
              """,
    ),
    ("win32event", "user32", None, "win32/src/win32event.i"),
    (
        "win32clipboard",
        "gdi32 user32 shell32",
        None,
        "win32/src/win32clipboardmodule.cpp",
    ),
    # win32gui handled below
    ("win32job", "user32", 0x0500, "win32/src/win32job.i"),
    ("win32lz", "lz32", None, "win32/src/win32lzmodule.cpp"),
    (
        "win32net",
        "netapi32 advapi32",
        None,
        """
              win32/src/win32net/win32netfile.cpp    win32/src/win32net/win32netgroup.cpp
              win32/src/win32net/win32netmisc.cpp    win32/src/win32net/win32netmodule.cpp
              win32/src/win32net/win32netsession.cpp win32/src/win32net/win32netuse.cpp
              win32/src/win32net/win32netuser.cpp
              """,
    ),
    ("win32pdh", "", None, "win32/src/win32pdhmodule.cpp"),
    ("win32pipe", "", None, "win32/src/win32pipe.i"),
    (
        "win32print",
        "winspool user32 gdi32",
        0x0500,
        "win32/src/win32print/win32print.cpp",
    ),
    ("win32process", "advapi32 user32", 0x0500, "win32/src/win32process.i"),
    ("win32profile", "Userenv", None, "win32/src/win32profilemodule.cpp"),
    ("win32ras", "rasapi32 user32", 0x0500, "win32/src/win32rasmodule.cpp"),
    (
        "win32security",
        "advapi32 user32 netapi32",
        0x0500,
        """
            win32/src/win32security.i
            win32/src/win32security_sspi.cpp win32/src/win32security_ds.cpp
            """,
    ),
    (
        "win32service",
        "advapi32 oleaut32 user32",
        0x0501,
        """
            win32/src/win32service_messages.mc
            win32/src/win32service.i
            """,
    ),
    ("win32trace", "advapi32", None, "win32/src/win32trace.cpp"),
    (
        "win32wnet",
        "netapi32 mpr",
        None,
        """
            win32/src/win32wnet/PyNCB.cpp
            win32/src/win32wnet/PyNetresource.cpp
            win32/src/win32wnet/win32wnet.cpp
            """,
    ),
    (
        "win32inet",
        "wininet",
        0x500,
        """
            win32/src/win32inet.i
            win32/src/win32inet_winhttp.cpp
            """,
    ),
    ("win32console", "kernel32", 0x0501, "win32/src/win32consolemodule.cpp"),
    ("win32ts", "WtsApi32", 0x0501, "win32/src/win32tsmodule.cpp"),
    ("_win32sysloader", "", 0x0501, "win32/src/_win32sysloader.cpp"),
    ("win32transaction", "kernel32", 0x0501, "win32/src/win32transactionmodule.cpp"),
):

    name, lib_names = info[:2]
    windows_h_ver = sources = None
    if len(info) > 2:
        windows_h_ver = info[2]
    if len(info) > 3:
        sources = info[3].split()
    extra_compile_args = []
    ext = WinExt_win32(
        name,
        libraries=lib_names,
        extra_compile_args=extra_compile_args,
        windows_h_version=windows_h_ver,
        sources=sources,
    )
    win32_extensions.append(ext)

# The few that need slightly special treatment
win32_extensions += [
    WinExt_win32(
        "win32evtlog",
        sources="""
                win32\\src\\win32evtlog_messages.mc win32\\src\\win32evtlog.i
                """.split(),
        libraries="advapi32 oleaut32",
        delay_load_libraries="wevtapi",
        windows_h_version=0x0600,
    ),
    WinExt_win32(
        "win32api",
        sources="""
                win32/src/win32apimodule.cpp win32/src/win32api_display.cpp
                """.split(),
        libraries="user32 advapi32 shell32 version",
        delay_load_libraries="powrprof",
        windows_h_version=0x0500,
    ),
    WinExt_win32(
        "win32gui",
        sources="""
                win32/src/win32dynamicdialog.cpp
                win32/src/win32gui.i
               """.split(),
        windows_h_version=0x0500,
        libraries="gdi32 user32 comdlg32 comctl32 shell32",
        define_macros=[("WIN32GUI", None)],
    ),
    # winxpgui is built from win32gui.i, but sets up different #defines before
    # including windows.h.  It also has an XP style manifest.
    WinExt_win32(
        "winxpgui",
        sources="""
                win32/src/winxpgui.rc win32/src/win32dynamicdialog.cpp
                win32/src/win32gui.i
               """.split(),
        libraries="gdi32 user32 comdlg32 comctl32 shell32",
        windows_h_version=0x0500,
        define_macros=[("WIN32GUI", None), ("WINXPGUI", None)],
        extra_swig_commands=["-DWINXPGUI"],
    ),
    # winxptheme
    WinExt_win32(
        "_winxptheme",
        sources=["win32/src/_winxptheme.i"],
        libraries="gdi32 user32 comdlg32 comctl32 shell32 Uxtheme",
        windows_h_version=0x0500,
    ),
]
win32_extensions += [
    WinExt_win32(
        "servicemanager",
        sources=["win32/src/PythonServiceMessages.mc", "win32/src/PythonService.cpp"],
        extra_compile_args=["-DPYSERVICE_BUILD_DLL"],
        libraries="user32 ole32 advapi32 shell32",
        windows_h_version=0x500,
    ),
]

win32_extensions += [
    WinExt_win32(
        "win32help",
        sources=["win32/src/win32helpmodule.cpp"],
        libraries="htmlhelp user32 advapi32",
        windows_h_version=0x500,
    ),
]

dirs = {
    "adsi": "com/win32comext/adsi/src",
    "propsys": "com/win32comext/propsys/src",
    "shell": "com/win32comext/shell/src",
    "axcontrol": "com/win32comext/axcontrol/src",
    "axdebug": "com/win32comext/axdebug/src",
    "axscript": "com/win32comext/axscript/src",
    "directsound": "com/win32comext/directsound/src",
    "ifilter": "com/win32comext/ifilter/src",
    "internet": "com/win32comext/internet/src",
    "mapi": "com/win32comext/mapi/src",
    "authorization": "com/win32comext/authorization/src",
    "taskscheduler": "com/win32comext/taskscheduler/src",
    "bits": "com/win32comext/bits/src",
    "win32com": "com/win32com/src",
}

# The COM modules.
pythoncom = WinExt_system32(
    "pythoncom",
    sources=(
        """
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
                        """
        % dirs
    ).split(),
    depends=(
        """
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
                        """
        % dirs
    ).split(),
    libraries="oleaut32 ole32 user32 urlmon",
    export_symbol_file="com/win32com/src/PythonCOM.def",
    extra_compile_args=["-DBUILD_PYTHONCOM"],
    pch_header="stdafx.h",
    windows_h_version=0x500,
    base_address=dll_base_address,
)
dll_base_address += 0x80000  # pythoncom is large!
com_extensions = [pythoncom]
com_extensions += [
    WinExt_win32com(
        "adsi",
        libraries="ACTIVEDS ADSIID user32 advapi32",
        sources=(
            """
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
                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "axcontrol",
        pch_header="axcontrol_pch.h",
        sources=(
            """
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
                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "axscript",
        sources=(
            """
                        %(axscript)s/AXScript.cpp
                        %(axscript)s/GUIDS.CPP                   %(axscript)s/PyGActiveScript.cpp
                        %(axscript)s/PyGActiveScriptError.cpp    %(axscript)s/PyGActiveScriptParse.cpp
                        %(axscript)s/PyGActiveScriptSite.cpp     %(axscript)s/PyGObjectSafety.cpp
                        %(axscript)s/PyIActiveScript.cpp         %(axscript)s/PyIActiveScriptError.cpp
                        %(axscript)s/PyIActiveScriptParse.cpp    %(axscript)s/PyIActiveScriptParseProcedure.cpp
                        %(axscript)s/PyIActiveScriptSite.cpp     %(axscript)s/PyIMultiInfos.cpp
                        %(axscript)s/PyIObjectSafety.cpp         %(axscript)s/stdafx.cpp
                        """
            % dirs
        ).split(),
        depends=(
            """
                             %(axscript)s/AXScript.h
                             %(axscript)s/guids.h                %(axscript)s/PyGActiveScriptError.h
                             %(axscript)s/PyIActiveScriptError.h %(axscript)s/PyIObjectSafety.h
                             %(axscript)s/PyIProvideMultipleClassInfo.h
                             %(axscript)s/stdafx.h
                             """
            % dirs
        ).split(),
        extra_compile_args=["-DPY_BUILD_AXSCRIPT"],
        implib_name="axscript",
        pch_header="stdafx.h",
    ),
    WinExt_win32com(
        "axdebug",
        libraries="axscript",
        pch_header="stdafx.h",
        sources=(
            """
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
                     """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "internet",
        pch_header="internet_pch.h",
        sources=(
            """
                        %(internet)s/internet.cpp                   %(internet)s/PyIDocHostUIHandler.cpp
                        %(internet)s/PyIHTMLOMWindowServices.cpp    %(internet)s/PyIInternetBindInfo.cpp
                        %(internet)s/PyIInternetPriority.cpp        %(internet)s/PyIInternetProtocol.cpp
                        %(internet)s/PyIInternetProtocolInfo.cpp    %(internet)s/PyIInternetProtocolRoot.cpp
                        %(internet)s/PyIInternetProtocolSink.cpp    %(internet)s/PyIInternetSecurityManager.cpp
                    """
            % dirs
        ).split(),
        depends=["%(internet)s/internet_pch.h" % dirs],
    ),
    WinExt_win32com(
        "mapi",
        libraries="advapi32",
        pch_header="PythonCOM.h",
        include_dirs=["%(mapi)s/mapi_headers" % dirs],
        sources=(
            """
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
                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com_mapi(
        "exchange",
        libraries="advapi32 legacy_stdio_definitions",
        include_dirs=["%(mapi)s/mapi_headers" % dirs],
        sources=(
            """
                                  %(mapi)s/exchange.i         %(mapi)s/exchange.cpp
                                  %(mapi)s/PyIExchangeManageStore.i %(mapi)s/PyIExchangeManageStore.cpp
                                  %(mapi)s/PyIExchangeManageStoreEx.i %(mapi)s/PyIExchangeManageStoreEx.cpp
                                  %(mapi)s/mapiutil.cpp
                                  %(mapi)s/exchangeguids.cpp
                                  %(mapi)s/mapi_stub_library/MapiStubLibrary.cpp
                                  %(mapi)s/mapi_stub_library/StubUtils.cpp
                                  """
            % dirs
        ).split(),
    ),
    WinExt_win32com_mapi(
        "exchdapi",
        libraries="advapi32",
        include_dirs=["%(mapi)s/mapi_headers" % dirs],
        sources=(
            """
                                  %(mapi)s/exchdapi.i         %(mapi)s/exchdapi.cpp
                                  %(mapi)s/mapi_stub_library/MapiStubLibrary.cpp
                                  %(mapi)s/mapi_stub_library/StubUtils.cpp
                                  """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "shell",
        libraries="shell32",
        pch_header="shell_pch.h",
        windows_h_version=0x600,
        sources=(
            """
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

                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "propsys",
        libraries="propsys",
        delay_load_libraries="shell32",
        sources=(
            """
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
                        """
            % dirs
        ).split(),
        implib_name="pypropsys",
    ),
    WinExt_win32com(
        "taskscheduler",
        libraries="mstask",
        sources=(
            """
                        %(taskscheduler)s/taskscheduler.cpp
                        %(taskscheduler)s/PyIProvideTaskPage.cpp
                        %(taskscheduler)s/PyIScheduledWorkItem.cpp
                        %(taskscheduler)s/PyITask.cpp
                        %(taskscheduler)s/PyITaskScheduler.cpp
                        %(taskscheduler)s/PyITaskTrigger.cpp

                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "bits",
        libraries="Bits",
        pch_header="bits_pch.h",
        sources=(
            """
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

                        """
            % dirs
        ).split(),
    ),
    WinExt_win32com(
        "ifilter",
        libraries="ntquery",
        sources=("%(ifilter)s/PyIFilter.cpp" % dirs).split(),
        depends=("%(ifilter)s/PyIFilter.h %(ifilter)s/stdafx.h" % dirs).split(),
    ),
    WinExt_win32com(
        "directsound",
        pch_header="directsound_pch.h",
        sources=(
            """
                        %(directsound)s/directsound.cpp     %(directsound)s/PyDSBCAPS.cpp
                        %(directsound)s/PyDSBUFFERDESC.cpp  %(directsound)s/PyDSCAPS.cpp
                        %(directsound)s/PyDSCBCAPS.cpp      %(directsound)s/PyDSCBUFFERDESC.cpp
                        %(directsound)s/PyDSCCAPS.cpp       %(directsound)s/PyIDirectSound.cpp
                        %(directsound)s/PyIDirectSoundBuffer.cpp %(directsound)s/PyIDirectSoundCapture.cpp
                        %(directsound)s/PyIDirectSoundCaptureBuffer.cpp
                        %(directsound)s/PyIDirectSoundNotify.cpp
                        """
            % dirs
        ).split(),
        depends=(
            """
                        %(directsound)s/directsound_pch.h   %(directsound)s/PyIDirectSound.h
                        %(directsound)s/PyIDirectSoundBuffer.h %(directsound)s/PyIDirectSoundCapture.h
                        %(directsound)s/PyIDirectSoundCaptureBuffer.h %(directsound)s/PyIDirectSoundNotify.h
                        """
            % dirs
        ).split(),
        optional_headers=["dsound.h"],
        libraries="user32 dsound dxguid",
    ),
    WinExt_win32com(
        "authorization",
        libraries="aclui advapi32",
        sources=(
            """
                        %(authorization)s/authorization.cpp
                        %(authorization)s/PyGSecurityInformation.cpp
                        """
            % dirs
        ).split(),
    ),
]

pythonwin_extensions = [
    WinExt_pythonwin(
        "win32ui",
        sources=[
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
        extra_compile_args=["-DBUILD_PYW"],
        pch_header="stdafx.h",
        base_address=dll_base_address,
        depends=[
            "Pythonwin/stdafx.h",
            "Pythonwin/win32uiExt.h",
            "win32/src/PyWinTypes.h",
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
        optional_headers=["afxres.h"],
    ),
    WinExt_pythonwin(
        "win32uiole",
        sources=[
            "Pythonwin/stdafxole.cpp",
            "Pythonwin/win32oleDlgInsert.cpp",
            "Pythonwin/win32oleDlgs.cpp",
            "Pythonwin/win32uiole.cpp",
            "Pythonwin/win32uioleClientItem.cpp",
            "Pythonwin/win32uioledoc.cpp",
        ],
        depends=[
            "Pythonwin/stdafxole.h",
            "Pythonwin/win32oleDlgs.h",
            "Pythonwin/win32uioledoc.h",
        ],
        pch_header="stdafxole.h",
        windows_h_version=0x500,
        optional_headers=["afxres.h"],
    ),
    WinExt_pythonwin(
        "dde",
        sources=[
            "Pythonwin/stddde.cpp",
            "Pythonwin/ddetopic.cpp",
            "Pythonwin/ddeconv.cpp",
            "Pythonwin/ddeitem.cpp",
            "Pythonwin/ddemodule.cpp",
            "Pythonwin/ddeserver.cpp",
        ],
        pch_header="stdafxdde.h",
        depends=["win32/src/stddde.h", "pythonwin/ddemodule.h"],
        optional_headers=["afxres.h"],
    ),
]
# win32ui is large, so we reserve more bytes than normal
dll_base_address += 0x100000

other_extensions = []
other_extensions.append(
    WinExt_ISAPI(
        "PyISAPI_loader",
        sources=[
            os.path.join("isapi", "src", s)
            for s in """PyExtensionObjects.cpp PyFilterObjects.cpp
                  pyISAPI.cpp pyISAPI_messages.mc
                  PythonEng.cpp StdAfx.cpp Utils.cpp
               """.split()
        ],
        # We keep pyISAPI_messages.h out of the depends list, as it is
        # generated and we aren't smart enough to say *only* the .cpp etc
        # depend on it - so the generated .h says the .mc needs to be
        # rebuilt, which re-creates the .h...
        depends=[
            os.path.join("isapi", "src", s)
            for s in """ControlBlock.h FilterContext.h PyExtensionObjects.h
                  PyFilterObjects.h pyISAPI.h
                  PythonEng.h StdAfx.h Utils.h
               """.split()
        ],
        pch_header="StdAfx.h",
        is_regular_dll=1,
        export_symbols="""HttpExtensionProc GetExtensionVersion
                           TerminateExtension GetFilterVersion
                           HttpFilterProc TerminateFilter
                           PyISAPISetOptions WriteEventLogMessage
                           """.split(),
        libraries="advapi32",
    )
)

W32_exe_files = [
    WinExt_pythonservice(
        "pythonservice",
        sources=[
            os.path.join("win32", "src", s)
            for s in "PythonService.cpp PythonService.rc".split()
        ],
        libraries="user32 advapi32 ole32 shell32",
    ),
    WinExt_pythonwin_subsys_win(
        "Pythonwin",
        sources=[
            "Pythonwin/pythonwin.cpp",
            "Pythonwin/pythonwin.rc",
            "Pythonwin/stdafxpw.cpp",
        ],
        depends=[
            "Pythonwin/Win32uiHostGlue.h",
            "Pythonwin/pythonwin.h",
        ],
        optional_headers=["afxres.h"],
    ),
]

# Special definitions for SWIG.
swig_interface_parents = {
    # source file base,     "base class" for generated COM support
    "mapi": None,  # not a class, but module
    "PyIMailUser": "IMAPIContainer",
    "PyIABContainer": "IMAPIContainer",
    "PyIAddrBook": "IMAPIProp",
    "PyIAttach": "IMAPIProp",
    "PyIDistList": "IMAPIContainer",
    "PyIMailUser": "IMAPIContainer",
    "PyIMAPIContainer": "IMAPIProp",
    "PyIMAPIFolder": "IMAPIContainer",
    "PyIMAPIProp": "",  # '' == default base
    "PyIMAPISession": "",
    "PyIMAPIStatus": "IMAPIProp",
    "PyIMAPITable": "",
    "PyIMessage": "IMAPIProp",
    "PyIMsgServiceAdmin": "",
    "PyIMsgServiceAdmin2": "IMsgServiceAdmin",
    "PyIProviderAdmin": "",
    "PyIMsgStore": "IMAPIProp",
    "PyIProfAdmin": "",
    "PyIProfSect": "IMAPIProp",
    "PyIConverterSession": "",
    # exchange and exchdapi
    "exchange": None,
    "exchdapi": None,
    "PyIExchangeManageStore": "",
    "PyIExchangeManageStoreEx": "",
    # ADSI
    "adsi": None,  # module
    "PyIADsContainer": "IDispatch",
    "PyIADsDeleteOps": "IDispatch",
    "PyIADsUser": "IADs",
    "PyIDirectoryObject": "",
    "PyIDirectorySearch": "",
    "PyIDsObjectPicker": "",
    "PyIADs": "IDispatch",
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
            ret.append((path_use, (fname,)))
    return ret


def convert_optional_data_files(files):
    ret = []
    for file in files:
        try:
            temp = convert_data_files([file])
        except RuntimeError as details:
            if not str(details.args[0]).startswith("No file"):
                raise
            log.info("NOTE: Optional file %s not found - skipping" % file)
        else:
            ret.append(temp[0])
    return ret


################################################################
if len(sys.argv) == 1:
    # distutils will print usage - print our docstring first.
    print(__doc__)
    print("Standard usage information follows:")

packages = [
    "win32com",
    "win32com.client",
    "win32com.demos",
    "win32com.makegw",
    "win32com.server",
    "win32com.servers",
    "win32com.test",
    "win32comext.adsi",
    "win32comext.axscript",
    "win32comext.axscript.client",
    "win32comext.axscript.server",
    "win32comext.axdebug",
    "win32comext.propsys",
    "win32comext.shell",
    "win32comext.mapi",
    "win32comext.ifilter",
    "win32comext.internet",
    "win32comext.axcontrol",
    "win32comext.taskscheduler",
    "win32comext.directsound",
    "win32comext.directsound.test",
    "win32comext.authorization",
    "win32comext.bits",
    "pythonwin.pywin",
    "pythonwin.pywin.debugger",
    "pythonwin.pywin.dialogs",
    "pythonwin.pywin.docking",
    "pythonwin.pywin.framework",
    "pythonwin.pywin.framework.editor",
    "pythonwin.pywin.framework.editor.color",
    "pythonwin.pywin.idle",
    "pythonwin.pywin.mfc",
    "pythonwin.pywin.scintilla",
    "pythonwin.pywin.tools",
    "isapi",
    "adodbapi",
]

py_modules = expand_modules("win32\\lib")
ext_modules = (
    win32_extensions + com_extensions + pythonwin_extensions + other_extensions
)

cmdclass = {
    "install": my_install,
    "build": my_build,
    "build_ext": my_build_ext,
    "install_data": my_install_data,
    "install_lib": my_install_lib,
}

classifiers = [
    "Environment :: Win32 (MS Windows)",
    "Intended Audience :: Developers",
    "License :: OSI Approved :: Python Software Foundation License",
    "Operating System :: Microsoft :: Windows",
    "Programming Language :: Python :: 3.7",
    "Programming Language :: Python :: 3.8",
    "Programming Language :: Python :: 3.9",
    "Programming Language :: Python :: 3.10",
    "Programming Language :: Python :: 3.11",
    "Programming Language :: Python :: 3.12",
    "Programming Language :: Python :: Implementation :: CPython",
]

if "bdist_wininst" in sys.argv:
    # fixup https://github.com/pypa/setuptools/issues/3284
    def maybe_fixup_exes():
        import distutils.command.bdist_wininst
        import site

        # setuptools can't find .exe stubs in `site-packages/setuptools/_distutils`
        # but they might exist in the original `lib/distutils`.
        expected_dir = os.path.dirname(distutils.command.bdist_wininst.__file__)
        if not len(glob.glob(f"{expected_dir}/*.exe")):
            # might die, see if we can not!
            for maybe in site.getsitepackages():
                maybe_dir = os.path.abspath(f"{maybe}/../distutils/command")
                if len(glob.glob(f"{maybe_dir}/*.exe")):
                    print(f"pointing setuptools at '{maybe_dir}'")
                    distutils.command.bdist_wininst.__file__ = os.path.join(
                        maybe_dir, "bdist_wininst.py"
                    )
                    break
            else:
                print("can't fixup distutils/setuptools exe stub location, good luck!")

    maybe_fixup_exes()

    # It doesn't really make sense to put README.md as the long description, so
    # keep it short and sweet as it's the first thing shown by the UI.
    long_description = (
        "Python extensions for Microsoft Windows\n"
        "Provides access to much of the Win32 API, the\n"
        "ability to create and use COM objects, and the\n"
        "Pythonwin environment."
    )
    long_description_content_type = "text/plain"
else:
    # For wheels, the readme makes more sense as pypi does something sane
    # with it.
    my_dir = os.path.abspath(os.path.dirname(__file__))
    with open(os.path.join(my_dir, "README.md")) as f:
        long_description = f.read()
    long_description_content_type = "text/markdown"

dist = setup(
    name="pywin32",
    version=str(build_id),
    description="Python for Window Extensions",
    long_description=long_description,
    long_description_content_type=long_description_content_type,
    author="Mark Hammond (et al)",
    author_email="mhammond@skippinet.com.au",
    url="https://github.com/mhammond/pywin32",
    license="PSF",
    classifiers=classifiers,
    cmdclass=cmdclass,
    options={
        "bdist_wininst": {
            "install_script": "pywin32_postinstall.py",
            "title": "pywin32-%s" % (build_id,),
            "user_access_control": "auto",
        },
    },
    scripts=["pywin32_postinstall.py", "pywin32_testall.py"],
    ext_modules=ext_modules,
    package_dir={
        "win32com": "com/win32com",
        "win32comext": "com/win32comext",
        "pythonwin": "pythonwin",
    },
    packages=packages,
    py_modules=py_modules,
    data_files=[("", (os.path.join(gettempdir(), "pywin32.version.txt"),))]
    + convert_optional_data_files(
        [
            "PyWin32.chm",
        ]
    )
    + convert_data_files(
        [
            "pythonwin/pywin/*.cfg",
            "pythonwin/pywin/Demos/*.py",
            "pythonwin/pywin/Demos/app/*.py",
            "pythonwin/pywin/Demos/ocx/*.py",
            "pythonwin/license.txt",
            "win32/license.txt",
            "win32/scripts/*.py",
            "win32/test/*.py",
            "win32/test/win32rcparser/test.rc",
            "win32/test/win32rcparser/test.h",
            "win32/test/win32rcparser/python.ico",
            "win32/test/win32rcparser/python.bmp",
            "win32/Demos/*.py",
            "win32/Demos/images/*.bmp",
            "com/win32com/readme.html",
            # win32com test utility files.
            "com/win32com/test/*.idl",
            "com/win32com/test/*.js",
            "com/win32com/test/*.sct",
            "com/win32com/test/*.txt",
            "com/win32com/test/*.vbs",
            "com/win32com/test/*.xsl",
            # win32com docs
            "com/win32com/HTML/*.html",
            "com/win32com/HTML/image/*.gif",
            "com/win32comext/adsi/demos/*.py",
            # Active Scripting test and demos.
            "com/win32comext/axscript/test/*.py",
            "com/win32comext/axscript/test/*.pys",
            "com/win32comext/axscript/test/*.vbs",
            "com/win32comext/axscript/Demos/*.pys",
            "com/win32comext/axscript/Demos/*.htm*",
            "com/win32comext/axscript/Demos/*.gif",
            "com/win32comext/axscript/Demos/*.asp",
            "com/win32comext/mapi/demos/*.py",
            "com/win32comext/propsys/test/*.py",
            "com/win32comext/shell/test/*.py",
            "com/win32comext/shell/demos/servers/*.py",
            "com/win32comext/shell/demos/*.py",
            "com/win32comext/taskscheduler/test/*.py",
            "com/win32comext/ifilter/demo/*.py",
            "com/win32comext/authorization/demos/*.py",
            "com/win32comext/bits/test/*.py",
            "isapi/*.txt",
            "isapi/samples/*.py",
            "isapi/samples/*.txt",
            "isapi/doc/*.html",
            "isapi/test/*.py",
            "isapi/test/*.txt",
            "adodbapi/*.txt",
            "adodbapi/test/*.py",
            "adodbapi/examples/*.py",
        ]
    )
    +
    # The headers and .lib files
    [
        ("win32/include", ("win32/src/PyWinTypes.h",)),
        (
            "win32com/include",
            (
                "com/win32com/src/include/PythonCOM.h",
                "com/win32com/src/include/PythonCOMRegister.h",
                "com/win32com/src/include/PythonCOMServer.h",
            ),
        ),
    ]
    +
    # And data files convert_data_files can't handle.
    [
        ("win32com", ("com/License.txt",)),
        # pythoncom.py doesn't quite fit anywhere else.
        # Note we don't get an auto .pyc - but who cares?
        ("", ("com/pythoncom.py",)),
        ("", ("pywin32.pth",)),
    ],
)

# If we did any extension building, and report if we skipped any.
if "build_ext" in dist.command_obj:
    what_string = "built"
    if "install" in dist.command_obj:  # just to be purdy
        what_string += "/installed"
    # Print the list of extension modules we skipped building.
    if "build_ext" in dist.command_obj:
        excluded_extensions = dist.command_obj["build_ext"].excluded_extensions
        if excluded_extensions:
            skip_whitelist = {"exchdapi", "exchange", "axdebug", "winxpgui"}
            skipped_ex = []
            print("*** NOTE: The following extensions were NOT %s:" % what_string)
            for ext, why in excluded_extensions:
                print(" %s: %s" % (ext.name, why))
                if ext.name not in skip_whitelist:
                    skipped_ex.append(ext.name)
            print("For more details on installing the correct libraries and headers,")
            print("please execute this script with no arguments (or see the docstring)")
            if skipped_ex:
                print(
                    "*** Non-zero exit status. Missing for complete release build: %s"
                    % skipped_ex
                )
                sys.exit(1000 + len(skipped_ex))
        else:
            print("All extension modules %s OK" % (what_string,))
