# distutils setup-script for win32all core dlls, currently only
# pywintypes and pythoncom.
#
# Thomas Heller, startet in 2000 or so.

from distutils.core import setup, Extension, Command
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.dep_util import newer_group
import os, string, sys

class WinExt (Extension):
    # Base class for all win32 extensions, with
    # some predefined library and include dirs,
    # and predefined windows libraries.
    # Additionally a method to parse .def files
    # into lists of exported symbols.
    def __init__ (self, name, sources,
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
                 ):
        libary_dirs = library_dirs,
        include_dirs = ['com/win32com/src/include',
                        'win32/src'] + include_dirs
        libraries = ['user32', 'odbc32', 'advapi32',
                     'oleaut32', 'ole32', 'shell32'] + libraries
        if export_symbol_file:
            export_symbols = export_symbols or []
            export_symbols.extend(self.parse_def_file(export_symbol_file))
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

pywintypes = WinExt('pywintypes',
                       ['win32/src/PyACL.cpp',
                        'win32/src/PyHandle.cpp',
                        'win32/src/PyIID.cpp',
                        'win32/src/PyLARGE_INTEGER.cpp',
                        'win32/src/PyOVERLAPPED.cpp',
                        'win32/src/PySECURITY_ATTRIBUTES.cpp',
                        'win32/src/PySECURITY_DESCRIPTOR.cpp',

                        'win32/src/PySID.cpp',
                        'win32/src/PyTime.cpp',
                        'win32/src/PyUnicode.cpp',
                        'win32/src/PyWinTypesmodule.cpp',
                        ],
                       extra_compile_args = ['-DBUILD_PYWINTYPES']
                       )

# source directories for win32com
com_src = 'com/win32com/src/'
com_ext = 'com/win32com/src/extensions/'

pythoncom = WinExt('pythoncom',
                   [com_src + 'dllmain.cpp',
                    com_src + 'ErrorUtils.cpp',
                    com_src + 'MiscTypes.cpp',
                    com_src + 'oleargs.cpp',
                    com_src + 'PyComHelpers.cpp',
                    com_src + 'PyFactory.cpp',
                    com_src + 'PyGatewayBase.cpp',
                    com_src + 'PyIBase.cpp',
                    com_src + 'PyIClassFactory.cpp',
                    com_src + 'PyIDispatch.cpp',
                    com_src + 'PyIUnknown.cpp',
                    com_src + 'PyRecord.cpp',
                    com_src + 'PyStorage.cpp',
                    com_src + 'PythonCOM.cpp',
                    com_src + 'Register.cpp',
                    com_src + 'stdafx.cpp',
                    com_src + 'univgw.cpp',
                    com_src + 'univgw_dataconv.cpp',
                    
                    com_ext + 'PyFUNCDESC.cpp',
                    com_ext + 'PyGConnectionPoint.cpp',
                    com_ext + 'PyGConnectionPointContainer.cpp',
                    com_ext + 'PyGEnumVariant.cpp',
                    com_ext + 'PyGErrorLog.cpp',
                    com_ext + 'PyGPersist.cpp',
                    com_ext + 'PyGPersistPropertyBag.cpp',
                    com_ext + 'PyGPersistStorage.cpp',
                    com_ext + 'PyGPersistStream.cpp',
                    com_ext + 'PyGPersistStreamInit.cpp',
                    com_ext + 'PyGPropertyBag.cpp',
                    com_ext + 'PyGStream.cpp',
                    com_ext + 'PyIBindCtx.cpp',
                    com_ext + 'PyICatInformation.cpp',
                    com_ext + 'PyICatRegister.cpp',
                    com_ext + 'PyIConnectionPoint.cpp',
                    com_ext + 'PyIConnectionPointContainer.cpp',
                    com_ext + 'PyICreateTypeInfo.cpp',
                    com_ext + 'PyICreateTypeLib.cpp',
                    com_ext + 'PyIEnumCATEGORYINFO.cpp',
                    com_ext + 'PyIEnumConnectionPoints.cpp',
                    com_ext + 'PyIEnumConnections.cpp',
                    com_ext + 'PyIEnumGUID.cpp',
                    com_ext + 'PyIEnumSTATPROPSTG.cpp',
                    com_ext + 'PyIEnumSTATSTG.cpp',
                    com_ext + 'PyIEnumVariant.cpp',
                    com_ext + 'PyIErrorLog.cpp',
                    com_ext + 'PyIExternalConnection.cpp',
                    com_ext + 'PyILockBytes.cpp',
                    com_ext + 'PyIMoniker.cpp',
                    com_ext + 'PyIPersist.cpp',
                    com_ext + 'PyIPersistFile.cpp',
                    com_ext + 'PyIPersistPropertyBag.cpp',
                    com_ext + 'PyIPersistStorage.cpp',
                    com_ext + 'PyIPersistStream.cpp',
                    com_ext + 'PyIPersistStreamInit.cpp',
                    com_ext + 'PyIPropertyBag.cpp',
                    com_ext + 'PyIPropertySetStorage.cpp',
                    com_ext + 'PyIPropertyStorage.cpp',
                    com_ext + 'PyIprovideClassInfo.cpp',
                    com_ext + 'PyIRunningObjectTable.cpp',
                    com_ext + 'PyIServiceProvider.cpp',
                    com_ext + 'PyIStorage.cpp',
                    com_ext + 'PyIStream.cpp',
                    com_ext + 'PyIType.cpp',
                    com_ext + 'PyITypeObjects.cpp',
                    com_ext + 'PyTYPEATTR.cpp',
                    com_ext + 'PyVARDESC.cpp',
                    ],
                   export_symbol_file = 'com/win32com/src/PythonCOM.def',
##                   libraries = ['PyWintypes'],
                   extra_compile_args = ['-DBUILD_PYTHONCOM'],
                   )


class my_install_lib (install_lib):

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

