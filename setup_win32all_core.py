# distutils setup-script for win32all core dlls, currently only
# pywintypes and pythoncom.
#
# Thomas Heller, started in 2000 or so.

from distutils.core import setup, Extension, Command
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.dep_util import newer_group
from distutils import log
import os, string, sys

# Python 2.2 has no True/False
try:
    True; False
except NameError:
    True=0==0
    False=1==0

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
                if os.path.splitext(fields[1])[1].lower() in ['.cpp', '.c', '.i']:
                    pathname = os.path.normpath(os.path.join(dsp_path, fields[1]))
                    result.append(pathname)
        return result

class WinExt_win32(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file"):
            kw["dsp_file"] = "win32/" + name + ".dsp"
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32"

class WinExt_win32com(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file"):
            kw["dsp_file"] = "com/" + name + ".dsp"
        kw["libraries"] = kw.get("libraries", "") + " oleaut32 ole32"
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32com/" + self.name

# 'win32com.mapi.exchange' and 'win32com.mapi.exchdapi' currently only
# ones with this special requirement
class WinExt_win32com_mapi(WinExt_win32com):
    def get_pywin32_dir(self):
        return "win32com/mapi"

# A hacky extension class for pywintypesXX.dll and pythoncomXX.dll
class WinExt_system32(WinExt):
    def get_pywin32_dir(self):
        return "system32"

################################################################

class my_install_lib (install_lib):
    # A special install_lib command, which will install into the windows
    # system directory instead of Lib/site-packages

    # XXX Currently broken.  Should only install pywintypes and pythoncom into sysdir
    def finalize_options(self):
        install_lib.finalize_options(self)
        self.install_dir = os.getenv("windir") + '\\system32'

class my_build_ext(build_ext):

    def finalize_options(self):
        build_ext.finalize_options(self)
        # The pywintypes library is created in the build_temp
        # directory, so we need to add this to library_dirs
        self.library_dirs.append(self.build_temp)
        self.mingw32 = (self.compiler == "mingw32")
        if self.mingw32:
            self.libraries.append("stdc++")

    def build_extensions(self):
        # Is there a better way than this?
        # Just one GUIDS.CPP and it gives trouble on mainwin too
        # Maybe I should just rename the file, but a case-only rename is likely to be
        # worse!
        import distutils.msvccompiler
        if ".CPP" not in self.compiler.src_extensions:
            self.compiler._cpp_extensions.append(".CPP")
            self.compiler.src_extensions.append(".CPP")
        assert self.package is None
        for ext in self.extensions:
            try:
                self.package = ext.get_pywin32_dir()
            except AttributeError:
                raise RuntimeError, "Not a win32 package!"
            self.build_extension(ext)

    def build_extension(self, ext):
        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        self.swig_cpp = True
        try:
            self.build_temp = os.path.join(self.build_temp, ext.name)

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
            # MSVCCompiler constructs the .lib file in the same directory
            # as the first source file's object file:
            #    os.path.dirname(objects[0])
            # but we want it in the (old) build_temp directory
            src = os.path.join(self.build_temp,
                               os.path.dirname(ext.sources[0]),
                               name1)
            dst = os.path.join(old_build_temp, name2)
            self.copy_file(src, dst)#, update=1)

        finally:
            self.build_temp = old_build_temp

    def get_ext_filename(self, name):
        # The pywintypes and pythoncom extensions have special names
        if name == "system32.pywintypes":
            extra = self.debug and "_d.dll" or ".dll"
            return "system32\pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name == "system32.pythoncom":
            extra = self.debug and "_d.dll" or ".dll"
            return "system32\pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        return build_ext.get_ext_filename(self, name)

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
        target_ext = '.cpp'
        for source in sources:
            (base, ext) = os.path.splitext(source)
            if ext == ".i":             # SWIG interface file
                # Seems the files SWIG creates are not compiled separately,
                # they are #included somewhere else. So we don't include
                # the generated wrapper in the new_sources list.
                swig_sources.append(source)
                # and win32all has it's own naming convention for the wrappers:
                swig_targets[source] = base + 'module_win32' + target_ext
            else:
                new_sources.append(source)

        if not swig_sources:
            return new_sources

        swig = self.find_swig()
        swig_cmd = [swig, "-python"]
        if self.swig_cpp:
            swig_cmd.append("-c++")

        for source in swig_sources:
            target = swig_targets[source]
            log.info("swigging %s to %s", source, target)
            self.spawn(swig_cmd + ["-o", target, source])

        return new_sources

################################################################

pywintypes = WinExt_system32('pywintypes',
                    dsp_file = r"win32\PyWinTypes.dsp",
                    extra_compile_args = ['-DBUILD_PYWINTYPES'],
                    libraries = "advapi32 user32 ole32 oleaut32",
                    )

win32_extensions = [pywintypes]

for name, lib_names, is_unicode in (
        ("dbi", "", False),
        ("mmapfile", "", False),
        ("odbc", "odbc32 odbccp32 dbi", False),
        ("perfmon", "", True),
        ("timer", "user32", False),
        ("win2kras", "rasapi32", False),
        ("win32api", "user32 advapi32 shell32 version", False),
        ("win32file", "", False),
        ("win32event", "user32", False),
        ("win32clipboard", "gdi32 user32 shell32", False),
        ("win32evtlog", "advapi32", False),
        # win32gui handled below
        ("win32help", "htmlhelp user32 advapi32", False),
        ("win32lz", "lz32", False),
        ("win32net", "netapi32", True),
        ("win32pdh", "", False),
        ("win32pipe", "", False),
        # win32popenWin9x later
        ("win32print", "winspool user32", False),
        ("win32process", "advapi32 user32", False),
        ("win32ras", "rasapi32 user32", False),
        ("win32security", "advapi32 user32", True),
        ("win32service", "advapi32", True),
        ("win32trace", "advapi32", False),
        ("win32wnet", "netapi32 mpr", False),
    ):

    extra_compile_args = []
    if is_unicode:
        extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT']
    ext = WinExt_win32(name, 
                 libraries=lib_names,
                 extra_compile_args = extra_compile_args)
    win32_extensions.append(ext)
# The few that need slightly special treatment
win32_extensions += [
    WinExt_win32("win32gui", 
           libraries="gdi32 user32 comdlg32 comctl32 shell32",
           extra_compile_args=["-DWIN32GUI"]
        ),
    WinExt_win32('servicemanager',
           extra_compile_args = ['-DUNICODE', '-D_UNICODE', 
                                 '-DWINNT', '-DPYSERVICE_BUILD_DLL'],
           libraries = "user32 ole32 advapi32 shell32",
           dsp_file = r"win32\Pythonservice servicemanager.dsp")
]

# The COM modules.
pythoncom = WinExt_system32('pythoncom',
                   dsp_file=r"com\win32com.dsp",
                   libraries = "oleaut32 ole32 user32",
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   )
com_extensions = [pythoncom]
com_extensions += [
    WinExt_win32com('adsi', libraries="ACTIVEDS ADSIID"),
    WinExt_win32com('axcontrol'),
    WinExt_win32com('axscript',
            dsp_file=r"com\Active Scripting.dsp",
            extra_compile_args = ['-DPY_BUILD_AXSCRIPT'],
    ),
    WinExt_win32com('axdebug',
            dsp_file=r"com\Active Debugging.dsp",
            libraries="axscript msdbg",
    ),
    WinExt_win32com('internet'),
    WinExt_win32com('mapi', libraries="mapi32"),
    WinExt_win32com_mapi('exchange',
                         libraries="""MBLOGON ADDRLKUP mapi32 exchinst                         
                                      EDKCFG EDKUTILS EDKMAPI
                                      ACLCLS version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com_mapi('exchdapi',
                         libraries="""DAPI ADDRLKUP exchinst EDKCFG EDKUTILS
                                      EDKMAPI mapi32 version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com('shell', libraries='shell32')
]
################################################################

setup(name="pywin32",
      version="version",
      description="Python for Window Extensions",
      long_description="",
      author="Mark Hammond (et al)",
      author_email = "mhammond@users.sourceforge.net",
      url="http://sourceforge.net/projects/pywin32/",
      license="PSA",
      cmdclass = { #'install_lib': my_install_lib,
                   'build_ext': my_build_ext,
                   },

      ext_modules = win32_extensions + com_extensions,

##      packages=['win32',
    
##                'win32com',
##                'win32com.client',
##                'win32com.demos',
##                'win32com.makegw',
##                'win32com.server',
##                'win32com.servers',
##                'win32com.test',

##                'win32comext.axscript',
##                'win32comext.axscript.client',
##                'win32comext.axscript.server',

##                'win32comext.axscript.demos',      # XXX not a package
##                'win32comext.axscript.demos.client',
##                'win32comext.axscript.demos.client.asp',
##                'win32comext.axscript.demos.client.ie',
##                'win32comext.axscript.demos.client.wsh',
##                'win32comext.axscript.test',       # XXX not a package
##                'win32comext.axdebug',
##                'win32comext.axscript',
##                'win32comext.axscript.client',
##                'win32comext.axscript.server',

##                'pywin',
##                'pywin.debugger',
##                'pywin.dialogs',
##                'pywin.docking',
##                'pywin.framework',
##                'pywin.framework.editor',
##                'pywin.framework.editor.color',
##                'pywin.idle',
##                'pywin.mfc',
##                'pywin.scintilla',
##                'pywin.tools',
##                ],
      )
