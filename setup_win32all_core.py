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
                  libraries=[],
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
        libraries = ['user32', 'odbc32', 'advapi32', 'version',
                     'oleaut32', 'ole32', 'shell32'] + libraries

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

    def build_extension(self, ext):
        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use a special one for each extension.
        old_build_temp = self.build_temp
        self.swig_cpp = True
        try:
            self.build_temp = os.path.join(self.build_temp, ext.name)

            build_ext.build_extension(self, ext)

            if ext.name not in ("pywintypes", "pythoncom"):
                return

            # The import libraries are created as PyWinTypes23.lib, but
            # are expected to be pywintypes.lib.

            # XXX This has to be changed for mingw32
            extra = self.debug and "_d.lib" or ".lib"
            if ext.name == "pywintypes":
                name1 = "pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
                name2 = "pywintypes%s" % (extra)
            elif ext.name == "pythoncom":
                name1 = "pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
                name2 = "pythoncom%s" % (extra)

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
        if name == "pywintypes":
            extra = self.debug and "_d.dll" or ".dll"
            return "pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name == "pythoncom":
            extra = self.debug and "_d.dll" or ".dll"
            return "pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
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

pywintypes = WinExt('pywintypes',
                    dsp_file = r"win32\PyWinTypes.dsp",
                    extra_compile_args = ['-DBUILD_PYWINTYPES']
                    )

pythoncom = WinExt('pythoncom',
                   dsp_file=r"com\win32com.dsp",
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   )

win32_extensions = []
for name, lib_names, is_unicode in (
        ("perfmon", "", True),
        ("win2kras", "rasapi32", False),
        ("win32api", "", False),
        ("win32file", "", False),
        ("win32event", "", False),
        ("win32clipboard", "gdi32", False),
        ("win32evtlog", "", False),
        # win32gui handled below
        ("win32help", "htmlhelp", False),
        ("win32lz", "lz32", False),
        ("win32net", "netapi32", True),
        ("win32pdh", "", False),
        ("win32pipe", "", False),
        # win32popenWin9x later
        ("win32print", "winspool", False),
        ("win32process", "", False),
        ("win32ras", "rasapi32", False),
        ("win32security", "", True),
        ("win32service", "", True),
        ("win32trace", "", False),
        ("win32wnet", "netapi32 mpr", False),
    ):

    extra_compile_args = []
    if is_unicode:
        extra_compile_args = ['-DUNICODE', '-D_UNICODE', '-DWINNT']
    ext = WinExt(name, 
                 dsp_file = "win32\\" + name + ".dsp",
                 libraries=lib_names.split(),
                 extra_compile_args = extra_compile_args)
    win32_extensions.append(ext)
# The few that need slightly special treatment
win32_extensions += [
    WinExt("win32gui", 
           dsp_file=r"win32\\win32gui.dsp",
           libraries=["gdi32", "comdlg32", "comctl32"],
           extra_compile_args=["-DWIN32GUI"]
        ),
    WinExt('servicemanager',
           extra_compile_args = ['-DUNICODE', '-D_UNICODE', 
                                 '-DWINNT', '-DPYSERVICE_BUILD_DLL'],
           dsp_file = r"win32\Pythonservice servicemanager.dsp")
]
################################################################

setup(name="PyWinTypes",
      version="version",
      description="Python for Window Extensions",
      long_description="",
      author="Mark Hammond (et al)",
      author_email = "mhammond@skippinet.com.au",
      url="http://starship.python.net/crew/mhammond/",
      license="PSA",
      cmdclass = { #'install_lib': my_install_lib,
                   'build_ext': my_build_ext,
                   },

      ext_modules = [pywintypes, pythoncom] + win32_extensions,

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
