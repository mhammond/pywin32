# distutils setup-script for win32all core dlls, currently only
# pywintypes and pythoncom.
#
# Thomas Heller, started in 2000 or so.

from distutils.core import setup, Extension, Command
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.dep_util import newer_group
import os, string, sys

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
        libraries = ['user32', 'odbc32', 'advapi32',
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
                if os.path.splitext(fields[1])[1].lower() in ['.cpp', '.c']:
                    pathname = os.path.normpath(os.path.join(dsp_path, fields[1]))
                    result.append(pathname)
        return result

################################################################

pywintypes = WinExt('pywintypes',
                    dsp_file = r"win32\PyWinTypes.dsp",
                    extra_compile_args = ['-DBUILD_PYWINTYPES']
                    )

pythoncom = WinExt('pythoncom',
                   dsp_file=r"com\win32com.dsp",
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
##                   libraries = ['PyWintypes'],
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   )


class my_install_lib (install_lib):
    # A special install_lib command, which will install into the windows
    # system directory instead of Lib/site-packages
    def finalize_options(self):
        install_lib.finalize_options(self)
        self.install_dir = os.getenv("windir") + '\\system32'

WINVER = tuple(map(int, string.split(sys.winver, '.')))

REGNAMES = {'pywintypes': 'PyWinTypes%d%d' % WINVER,
            'pythoncom': 'PythonCOM%d%d' % WINVER,
            }


class my_build_ext(build_ext):

    def finalize_options(self):
        build_ext.finalize_options(self)

        self.library_dirs.append(self.build_temp)
        self.mingw32 = (self.compiler == "mingw32")
        if self.mingw32:
            self.libraries.append("stdc++")

    def build_extension(self, ext):
        # some source files are compiled for different extensions
        # with special defines. So we cannot use a shared
        # directory for objects, we must use special ones.
        old_temp = self.build_temp
        try:
            self.build_temp = os.path.join(self.build_temp, ext.name)

            build_ext.build_extension(self, ext)

            # After building an extension with non-standard target filename,
            # copy the .lib-file created to the original name.
            name = REGNAMES.get(ext.name, ext.name)
            from distutils.file_util import copy_file, move_file
            extra = ''
            if self.debug:
                extra = '_d'
            if self.mingw32:
                prefix = 'lib'
                extra = extra + '.a'
            else:
                prefix = ''
                extra = extra + '.lib'
            src = os.path.join(self.build_temp, prefix + name + extra)
            dst = os.path.join(self.build_temp, "..", prefix + ext.name + extra)
            self.copy_file(src, dst)#, update=1)

        finally:
            self.build_temp = old_temp


    def get_libraries(self, ext):
        libs = build_ext.get_libraries(self, ext)
        result = []
        for lib in libs:
            plib = REGNAMES.get(string.lower(lib), None)
            if plib and self.debug:
                plib = plib + '_d'
            if plib:
                result.append(plib)
            else:
                result.append(lib)
        return result

    def get_ext_filename(self, name):
        fname = build_ext.get_ext_filename(self, name)
        base, ext = os.path.splitext(fname)
        newbase = REGNAMES.get(name, fname)
        if self.debug:
            return newbase + '_d' + '.dll'
        return newbase + '.dll'

setup(name="PyWinTypes",
      version="version?",
      description="python windows extensions",
      long_description="",
      author="Mark Hammond (et al?)",
      author_email = "Markh@activestate.com",
      url="http://starship.python.net/crew/mhammond/",
      license="???",

      cmdclass = { 'install_lib': my_install_lib,
                   'build_ext': my_build_ext,
                   },

      ext_modules = [pywintypes, pythoncom],

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

