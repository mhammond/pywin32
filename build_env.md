# Build environment

This describes how to setup the build environment for pywin32.

Double check the compiler version you need in the [Python wiki](https://wiki.python.org/moin/WindowsCompilers)
but note that Python 3.5+ all use version 14.X of the compiler, which, confusingly,
report themselves as V.19XX (eg, note in Python's banner, 3.5's "MSC v.1900", even 3.13's "MSC v.1941")

This compiler first shipped with Visual Studio 2015, although Visual Studio 2017+
all have this compiler available, just not installed by default.

## For Visual Studio 2017

- Install the [Build Tools for Visual Studio 2017](https://my.visualstudio.com/Downloads?q=Build%20Tools%20for%20Visual%20Studio%202017) (Version 15.0)\
  Public landing page: <https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2017-family>\

In the Visual Studio Installer:

Locate the "Desktop development with C++" section:

Ensure the following components are installed:

- `VC++ 2015.3 v14.00 (v140) toolset for desktop`
- `Windows 10 SDK and UCRT SDK`
- `Visual C++ MFC for x86 and x64`

if you want to cross-compile for ARM, you will need at least the following (from "Individual Components")

- `Visual C++ compilers and libraries for ARM64`
- `Visual C++ for MFC for ARM64`

(You should be able to check everything you need is installed by opening a
command prompt and executing:

```shell
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64 10.0.18362.0 -vcvars_ver=14.0
```

if that works, then executing:

```shell
cl
```

should report the compiler version:
> Microsoft (R) C/C++ Optimizing Compiler Version 19.00.24234.1 for x64

)

Note however that it's *not* necessary to configure the environment in this
way to build pywin32 - it's build process should find these tools automatically.

## For Visual Studio 2019

- Install the [Build Tools for Visual Studio 2019](https://my.visualstudio.com/Downloads?q=Build%20Tools%20for%20Visual%20Studio%202019) (Version 16.0)\
  Public landing page: <https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2019-family>\
- Maybe stop your virus scanner
- In `Visual Studio Installer`:
  - Select `Visual Studio Build Tools 2019`
    - Press `Modify`
      - In `Visual Studio Build Tools 2019`
        - Check `C++ build tools`
          - In the menu to the right, check:
            - `MSVC v142 - VS 2019 C++ x64/x86 build tools`
            - `Windows 10 SDK` (latest offered I guess? At time of writing, 10.0.18362)
            - `C++ MFC for latest v142 build tools (x86 & x64)`
            - `C++ ATL for latest v142 build tools`
        - If building for ARM64 (optional), select the "Individual Components" tab, and search for and select:
              - `MSVC v142 - VS 2019 C++ ARM64 build tools`
              - `C++ MFC for latest v142 build tools (ARM64)`
              - `C++ ATL for latest v142 build tools (ARM64)`
        - Press `Install` (~ 4.6 GB shown in the overview, but ~ 1.1 GB shown during download; approximately double with the ARM64 components)
- Restart your virus scanner
- Restart

## For Visual Studio 2022

- Install the [Build Tools for Visual Studio 2022](https://my.visualstudio.com/Downloads?q=Build%20Tools%20for%20Visual%20Studio%202022) (Version 17.2 [LTSC] or 17.14)\
  Public landing page: <https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2022-family>\
  Note that `17.6` is the last version that supports Windows 8.1. See [Evergreen bootstrappers](https://learn.microsoft.com/en-us/visualstudio/releases/2022/release-history#evergreen-bootstrappers).\
  Direct link for [Build Tools 17.6](https://aka.ms/vs/17/release.ltsc.17.6/vs_buildtools.exe).\
- Maybe stop your virus scanner
- In `Visual Studio Installer`:
  - Select `Visual Studio Build Tools 2022`
    - Press `Modify`
      - In `Visual Studio Build Tools 2022`
        - Check `Desktop development with C++`
          - In the menu to the right, check:
            - `MSVC v143 - VS 2022 C++ x64/x86 build tools`
            - `Windows 10 SDK`
            - `C++ MFC for latest v143 build tools (x86 & x64)`
            - `C++ ATL for latest v143 build tools`
        - If building for ARM64 (optional), select the "Individual Components" tab, and search for and select:
              - `MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools (Latest)`
              - `C++ MFC for latest v143 build tools (ARM64/ARM64EC)`
              - `C++ ATL for latest v143 build tools (ARM64/ARM64EC)`
        - Press `Install`
- Restart your virus scanner
- Restart

## For Visual Studio 2026

- Install the [Visual Studio 2026](https://visualstudio.microsoft.com/downloads/) (`VisualStudioSetup.exe`)\
- Maybe stop your virus scanner
- In `Visual Studio Installer`:
  - Select `Visual Studio Build Tools 2026`
    - Press `Modify`
      - In `Visual Studio Build Tools 2026`
        - Check `Desktop development with C++`
          - In the menu to the right, check:
            - `MSVC Build Tools fir x64/x86 (Latest)` (checked by default)
            - `Windows 11 SDK` (checked by default)
            - `C++ ATL for x64/x86 (Latest MSVC)`
            - `C++ MFC for x64/x86 (Latest MSVC)`
        - If building for ARM64 (optional), select the "Individual Components" tab, and search for and select:
              - `MSVC v145 - VS 2026 C++ ARM64/ARM64EC build tools (Latest)`
              - `C++ MFC for latest v145 build tools (ARM64/ARM64EC)`
              - `C++ ATL for latest v145 build tools (ARM64/ARM64EC)`
        - Press `Install`
- Restart your virus scanner
- Restart

### Microsoft Message Compiler

Search the executable

```shell
cd "c:\Program Files (x86)\Windows Kits"
dir /b /s mc.exe
```

Append location to the `path` (example)

```shell
set "path=%path%;c:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64"
```

Test with

```shell
where mc
```

(Note that the above process for 'mc' doesn't appear necessary for VS2017, but
@mhammond hasn't tried with VS2019 - please share your experiences!)

# Build

Once everything is setup, just execute the following from the pywin32 directory:

```shell
pip install . -v
```

Some modules need obscure SDKs to build - `pip install` should succeed, gracefully
telling you why it failed to build them with the `-v` flag - if the build actually fails with your
configuration, please [open an issue](https://github.com/mhammond/pywin32/issues).

## Cross-compiling for ARM64 (Microsoft Visual C++ 14.1 and up)

- Follow the `For Visual Studio XXXX` instructions above and pick the optional ARM64 build tools

- Download prebuilt Python ARM64 binaries to a temporary location on your machine. You will need this location in a later step.
  - This script downloads a Python ARM64 build [from NuGet](https://www.nuget.org/packages/pythonarm64/#versions-tab) that matches the version you used to run it.

    ```shell
    python .github\workflows\download-arm64-libs.py ./arm64libs
    ```

- Optional: by default, `setuptools` finds the latest Visual Studio install and sets up the ARM64 cross-compilation environment itself. To use a specific install or toolset instead, set up the environment yourself, and tell `setuptools` to use it as is:

    ```shell
    "C:\Program Files (x86)\Microsoft Visual Studio\XXXX\BuildTools\vc\Auxiliary\Build\vcvarsall.bat" x86_arm64
    set DISTUTILS_USE_SDK=1
    ```

  Both are needed. Without `DISTUTILS_USE_SDK`, `setuptools` ignores this environment and runs `vcvarsall.bat` on its own.

- Build the wheel, passing the directory from earlier. Pass `--plat-name=win-arm64` to *each* of the `build_ext`, `build` and `bdist_wheel` commands. Otherwise you may get a mixed platform build and/or linker errors.

    ```shell
    python -m build --wheel --config-setting=--build-option="build_ext -L./arm64libs --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
    ```

- Copy the built wheel to the target machine and install directly:

    ```shell
    pip install "<path to wheel>" -v
    ```
