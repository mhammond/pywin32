import os, sys
import glob
from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext
from distutils.sysconfig import get_python_lib


if os.path.dirname(sys.argv[0]):
    os.chdir(os.path.dirname(sys.argv[0]))

class build_dll(build_ext):
    def get_export_symbols(self, ext):
        return """HttpExtensionProc GetExtensionVersion TerminateExtension
                  GetFilterVersion HttpFilterProc TerminateFilter
                  PyISAPISetOptions""".split()
    def get_ext_filename(self, name):
        if self.debug:
            ext = "_d.dll"
        else:
            ext = ".dll"
        return name + ext

    def build_extension(self, ext):
        ext.extra_compile_args = ext.extra_compile_args or []
        # PCH support.
        ext.extra_compile_args.append("/YXStdAfx.h")
        pch_name = os.path.join(self.build_temp, ext.name) + ".pch"
        ext.extra_compile_args.append("/Fp"+pch_name)
        build_ext.build_extension(self, ext)
    
    def finalize_options(self):
        build_ext.finalize_options(self)
        self.build_lib = os.path.join(self.build_lib, "isapi")
        print "Build lib is", self.build_lib

sources = """PyExtensionObjects.cpp
PyFilterObjects.cpp
pyISAPI.cpp
PythonEng.cpp
StdAfx.cpp
Utils.cpp
""".split()
sources = [os.path.join("src", s) for s in sources]

data_files = [
    ["isapi/samples", glob.glob("samples/*.py") + glob.glob("samples/*.txt"),],
    ["isapi/README.txt", ['README.txt'],],
]
    
ext = Extension("PyISAPI_loader", sources)

setup(
    name="PyISAPI", 
    version="0.1",
    ext_modules=[ext],
    packages = ["isapi"],
    data_files = data_files,
    options = {"install":
                  {"install_data": get_python_lib()},
              },
    cmdclass = {
               'build_ext': build_dll,
               },
)
