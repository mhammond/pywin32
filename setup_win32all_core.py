# distutils setup-script for win32all
#
# Thomas Heller, started in 2000 or so.
#
# Things known to be missing:
# * Commented "data files"
# * Install of .exe/.dlls - most .exe files go next to python.exe
# * "dbi" was built as .dll, as odbc depends on it.  does it work?
# * create win32com\gen_py directory post install.
# * Installing the 2 system DLLs to the system directory (just notice post-
#   setup script does this - maybe do this on std "install" too?

from distutils.core import setup, Extension, Command
from distutils.command.install_lib import install_lib
from distutils.command.build_ext import build_ext
from distutils.dep_util import newer_group
from distutils import log
from distutils.sysconfig import get_python_lib
from distutils.filelist import FileList
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
                  pch_header=None
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
            
        self.pch_header = pch_header
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

# Note this is used only for "win32com extensions", not pythoncom
# itself - thus, output is "win32comext"
class WinExt_win32com(WinExt):
    def __init__ (self, name, **kw):
        if not kw.has_key("dsp_file"):
            kw["dsp_file"] = "com/" + name + ".dsp"
        kw["libraries"] = kw.get("libraries", "") + " oleaut32 ole32"
        WinExt.__init__(self, name, **kw)
    def get_pywin32_dir(self):
        return "win32comext/" + self.name

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

    def _why_cant_build_extension(self, ext):
        # Return None, or a reason it can't be built.
        common_dirs = self.compiler.library_dirs
        common_dirs += os.environ.get("LIB").split(os.pathsep)
        for lib in ext.libraries:
            if self.found_libraries.has_key(lib.lower()):
                continue
            for dir in common_dirs + ext.library_dirs:
                if os.path.isfile(os.path.join(dir, lib + ".lib")):
                    self.found_libraries[lib.lower()] = True
                    break
            else:
                return "No library '%s'" % lib

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
        self.excluded_extensions = [] # list of (ext, why)

        # Here we hack a "pywin32" directory into the mix.  Distutils
        # doesn't seem to like the concept of multiple top-level directories.
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

    def build_exefile(self, ext):
        from types import ListType, TupleType
        sources = ext.sources
        if sources is None or type(sources) not in (ListType, TupleType):
            raise DistutilsSetupError, \
                  ("in 'ext_modules' option (extension '%s'), " +
                   "'sources' must be present and must be " +
                   "a list of source filenames") % ext.name
        sources = list(sources)

        print "building exe %s" % ext.name

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

        objects = self.compiler.compile(sources,
                                        output_dir=self.build_temp,
                                        macros=macros,
                                        include_dirs=ext.include_dirs,
                                        debug=self.debug,
                                        extra_postargs=extra_args,
                                        depends=ext.depends)

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

        # Detect target language, if not provided
        language = ext.language or self.compiler.detect_language(sources)

        self.compiler.link(
            "executable",
            objects, ext_filename,
            libraries=self.get_libraries(ext),
            library_dirs=ext.library_dirs,
            runtime_library_dirs=ext.runtime_library_dirs,
            extra_postargs=extra_args,
            debug=self.debug,
            build_temp=self.build_temp,
            target_lang=language)
        
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

        if not self.mingw32 and ext.pch_header:
            ext.extra_compile_args = ext.extra_compile_args or []
            ext.extra_compile_args.append("/YX"+ext.pch_header)

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
            return r"system32\pywintypes%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name == "system32.pythoncom":
            extra = self.debug and "_d.dll" or ".dll"
            return r"system32\pythoncom%d%d%s" % (sys.version_info[0], sys.version_info[1], extra)
        elif name.endswith("win32.perfmondata"):
            extra = self.debug and "_d.dll" or ".dll"
            return r"win32\perfmondata" + extra
        elif name.endswith("win32.win32popenWin9x"):
            extra = self.debug and "_d.exe" or ".exe"
            return r"win32\win32popenWin9x" + extra
        elif name.endswith("pythonwin.Pythonwin"):
            extra = self.debug and "_d.exe" or ".exe"
            return r"pythonwin\Pythonwin" + extra
        return build_ext.get_ext_filename(self, name)

    def get_export_symbols(self, ext):
        if ext.name.endswith("perfmondata"):
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
        target_ext = '.cpp'
        for source in sources:
            (base, ext) = os.path.splitext(source)
            if ext == ".i":             # SWIG interface file
                # Seems the files SWIG creates are not compiled separately,
                # they are #included somewhere else. So we don't include
                # the generated wrapper in the new_sources list.
                swig_sources.append(source)
                # and win32all has it's own naming convention for the wrappers:
                if base.endswith("win32pipe") or base.endswith("win32security"):
                    swig_targets[source] = base + 'module' + target_ext
                else:
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
                    pch_header = "PyWinTypes.h",
                    )

win32_extensions = [pywintypes]

win32_extensions.append(
    WinExt_win32("perfmondata", 
                 libraries="advapi32",
                 extra_compile_args=["-DUNICODE", "-D_UNICODE", "-DWINNT"],
                 export_symbol_file = "win32/src/PerfMon/perfmondata.def",
        ),
    )

for name, lib_names, is_unicode in (
        ("dbi", "", False),
        ("mmapfile", "", False),
        ("odbc", "odbc32 odbccp32 dbi", False),
        ("perfmon", "", True),
        ("timer", "user32", False),
        ("win2kras", "rasapi32", False),
        ("win32api", "user32 advapi32 shell32 version", False),
        ("win32file", "oleaut32", False),
        ("win32event", "user32", False),
        ("win32clipboard", "gdi32 user32 shell32", False),
        ("win32evtlog", "advapi32 oleaut32", False),
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
        ("win32service", "advapi32 oleaut32", True),
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
                   pch_header = "stdafx.h",
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
    WinExt_win32com('axdebug',
            dsp_file=r"com\Active Debugging.dsp",
            libraries="axscript msdbg",
            pch_header = "stdafx.h",
    ),
    WinExt_win32com('internet'),
    WinExt_win32com('mapi', libraries="mapi32", pch_header="PythonCOM.h"),
    WinExt_win32com_mapi('exchange',
                         libraries="""MBLOGON ADDRLKUP mapi32 exchinst                         
                                      EDKCFG EDKUTILS EDKMAPI
                                      ACLCLS version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com_mapi('exchdapi',
                         libraries="""DAPI ADDRLKUP exchinst EDKCFG EDKUTILS
                                      EDKMAPI mapi32 version""",
                         extra_link_args=["/nodefaultlib:libc"]),
    WinExt_win32com('shell', libraries='shell32', pch_header="shell_pch.h")
]

pythonwin_extensions = [
    WinExt_pythonwin("win32ui", extra_compile_args = ['-DBUILD_PYW'],
                     pch_header="stdafx.h"),
    WinExt_pythonwin("win32uiole", pch_header="stdafxole.h"),
    WinExt_pythonwin("dde", pch_header="stdafxdde.h"),
]

W32_exe_files = [
    WinExt_win32("win32popenWin9x",
                 libraries = "user32"),
    WinExt_pythonwin("Pythonwin", extra_link_args=["/SUBSYSTEM:WINDOWS"]),
    ]

# XXX - incomplete, but checking in to avoid conflicts with Thomas ;)
# NOTE: somewhat counter-intuitively, a result list a-la:
#  [('Lib/site-packages\\Pythonwin', ('Pythonwin/license.txt',)),]
# will 'do the right thing' in terms of installing licence.txt into
# 'Lib/site-packages/Pythonwin/licence.txt'.  I intent exploiting this to
# get 'com/wincom/whatever' installed to 'win32com/whatever'
def convert_data_files(files):
    ret = []
    for file in files:
        if file.find("*") >= 0:
            continue
            flist = FileList(os.path.dirname(file))
            if not flist.include_pattern(os.path.basename(file)):
                raise RuntimeError, "No files match '%s'" % file
            found = flist.files
        else:
            if not os.path.isfile(file):
                raise RuntimeError, "No file '%s'" % file
            path = os.path.join("Lib/site-packages", os.path.dirname(file))
            ret.append( (path, (file,)) )
            continue
# xxx - incomplete, but semi-working, and going to bed :)
#            found = [file]
        ret.append( ("Lib/site-packages", found) )
        
    print ret
    return ret
   

################################################################

dist = setup(name="pywin32",
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
      options = {"bdist_wininst": {"install_script": "pywin32_postinstall.py"}},
      
      scripts = ["pywin32_postinstall.py"],
      
      ext_modules = win32_extensions + com_extensions + pythonwin_extensions,

      package_dir = {"win32": "win32",
                     "win32com": "com/win32com",
                     "win32comext": "com/win32comext",
                     "Pythonwin": "Pythonwin"},

      data_files=convert_data_files([
                'Pythonwin/pywin/*.cfg',
                'pywin32.chm',
                'Pythonwin/license.txt',
                'win32/license.txt',
                # win32com readme (doesn't work for cvt_data_files)
                # win32com/license
                # win32com test - *.txt, *.py, *.vbs, *.js, *.sct, *.xsl
                # win32com HTML\*
                # win32com HTML\image\*
                # win32comext\axscript\test - *.py, *.vbs, *.pys
                # win32comext\axscript\demos\ie\*.*
                # win32comext\axscript\demos\wsh\*.*
                # win32comext\axscript\demos\asp\*.*
                 ]),
      packages=['win32',
                'win32com',
                'win32com.client',
                'win32com.demos',
                'win32com.makegw',
                'win32com.server',
                'win32com.servers',
                'win32com.test',

                'win32comext.axscript',
                'win32comext.axscript.client',
                'win32comext.axscript.server',

                'win32comext.axscript.demos',      # XXX not a package
                'win32comext.axscript.demos.client',
                'win32comext.axscript.demos.client.asp',
                'win32comext.axscript.demos.client.ie',
                'win32comext.axscript.demos.client.wsh',
                'win32comext.axscript.test',       # XXX not a package
                'win32comext.axdebug',
                'win32comext.axscript',
                'win32comext.axscript.client',
                'win32comext.axscript.server',

                'win32comext.shell',
                'win32comext.mapi',
                'win32comext.internet',
                'win32comext.axcontrol',

                'Pythonwin',
                'Pythonwin.pywin',
                'Pythonwin.pywin.debugger',
                'Pythonwin.pywin.dialogs',
                'Pythonwin.pywin.docking',
                'Pythonwin.pywin.framework',
                'Pythonwin.pywin.framework.editor',
                'Pythonwin.pywin.framework.editor.color',
                'Pythonwin.pywin.idle',
                'Pythonwin.pywin.mfc',
                'Pythonwin.pywin.scintilla',
                'Pythonwin.pywin.tools',
                ],
      )
# If we did any extension building...
if dist.command_obj.has_key('build_ext'):
    what_string = "built"
    if dist.command_obj.has_key('install'): # just to be purdy
        what_string += "/installed"
    # Print the list of extension modules we skipped building.
    excluded_extensions = dist.command_obj['build_ext'].excluded_extensions
    if excluded_extensions:
        print "*** NOTE: The following extensions were NOT %s:" % what_string
        for ext, why in excluded_extensions:
            print " %s: %s" % (ext.name, why)
    else:
        print "All extension modules %s OK" % (what_string,)
