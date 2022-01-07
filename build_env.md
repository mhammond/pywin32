# Build environment

This describes how to setup the build environment for pywin32.

Double check the compiler version you need in the [Python wiki](https://wiki.python.org/moin/WindowsCompilers)
but note that Python 3.5 -> 3.9 all use version 14.X of the compiler, which,
confusingly, report themselves as V.19XX (eg, note in Python's banner,
3.5's "MSC v.1900", even 3.9b4's "MSC v.1924")

This compiler first shipped with Visual Studio 2015, although Visual Studio 2017
and Visual Studio VS2019 both have this compiler available, just not installed
by default.

In the Visual Studio Installer:

## For Visual Studio 2017:

Locate the "Desktop development with C++" section:

Ensure the following components are installed:
* VC++ 2015.3 v14.00 (v140) toolset for desktop
* Windows 8.1 SDK and UCRT SDK
* Visual C++ MFC for x86 and x64

(You should be able to check everything you need is installed by opening a
command prompt and executing:

% "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64 8.1 -vcvars_ver=14.0

if that works, then executing:

% cl

should report the compiler version:
> Microsoft (R) C/C++ Optimizing Compiler Version 19.00.24234.1 for x64

Note however that it's *not* necessary to configure the environment in this
way to build pywin32 - it's build process should find these tools automatically.

## For Visual Studio 2019
- Install the [Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16#) (`vs_BuildTools.exe` ~ 1 MB)
- Maybe stop your virus scanner
- In `Visual Studio installer`:
  - Select `Visual Studio Build Tools 2019`
    - Press `Modify`
      - In `Visual Studio Build Tools 2019`
        - Check `C++ build tools`
          - In the menue to the right, check:
              - `MSVCv142 - VS 2019 C++ x64/x86 build tools`
              - `Windows 10 SDK`
              - If building for ARM64 (optional): `MSVCv142 - VS 2019 C++ ARM64 build tools`
        - Press `Install` (~ 4.6 GB shown in the overview, but ~ 1.1 GB shown during download)
- Restart your virus scanner
- Restart
### MFC v140
- Install the [Build Tools for Visual Studio 2017 (version 15.9)](https://my.visualstudio.com) (`vs_BuildTools.exe` ~ 1 MB)
- Maybe stop your virus scanner
- In `Visual Studio installer`:
  - Select `Visual Studio Build Tools 2017 (15.9.21)`
    - Press `Modify`
      - In the `Visual Studio Build Tools 2017`
        - Check `C++ build tools`
          - In the menue to the right, at the bottom of the “Optional” section, additionally check:
            - `VC++ 2015.3 v14.00 (v140) toolset for desktop`
    - Press `Install` 
### Microsoft Message Compiler
Search the executable

    cd "c:\Program Files (x86)\Windows Kits"
    dir /b /s mc.exe

Append location to the `path` (example)

    set "path=%path%;c:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64"

Test with 

    where mc

(Note that the above process for 'mc' doesn't appear necessary for VS2017, but
markh hasn't tried with VS2019 - please share your experiences!)

# Build

One everything is setup, just execute:

% python setup.py -q install

from the pywin32 directory.

## Cross-compiling for ARM64 (Microsoft Visual C++ 14.1 and up)
- Follow the `For Visual Studio 2019` instructions above and pick the optional ARM64 build tools
- Follow one option from below to get Python for ARM64:
  1. Build Python for ARM64 and x86/x64 from source:
    - Follow steps 1-4 from [here](https://docs.microsoft.com/en-us/windows/iot-core/developer-tools/python#using-python-on-windows-iot-core-arm64) and stop after you have built for x86/x64 using `pcbuild\build.bat`
  2. Download prebuilt Python ARM64 binaries:
    - Download a Python ARM64 version aligned with the x86/x64 version you want to use for building. Download [NuGet package](https://www.nuget.org/packages/pythonarm64/) until Python.org starts including Windows ARM64 output
    - Change .nupkg file to .zip extension
    - Unzip package
    - Set an environment variable PYTHON_CROSS_LIBS_DIR to point to the Python ARM64 libraries:
      > set "PYTHON_CROSS_LIBS_DIR=C:\path\to\extractedpythonnuget\tools\libs"
- Setup the cross-compilation environment:
  > "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\vc\Auxiliary\Build\vcvarsall.bat" x86_arm64
- Build:
  > python setup.py build --plat-name win-arm64
- Install data files:
  > python setup.py install_data --install-dir build\lib.win-arm64-3.xx
- Install:
  > xcopy /e build\lib.win-arm64-3.xx\\* C:\path\to\pythonarm64\Lib\site-packages\
