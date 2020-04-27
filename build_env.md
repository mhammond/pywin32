# Build environment for Python 3.5, 3.6, 3.7, 3.8
## Microsoft Visual C++ compiler 14.2
- Double check the version in the [Python wiki](https://wiki.python.org/moin/WindowsCompilers)
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
        - Press `Install` (~ 4.6 GB shown in the overview, but ~ 1.1 GB shown during download)
- Restart your virus scanner
- Restart
## MFC v140
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
## Microsoft Message Compiler
Search the executable

    cd "c:\Program Files (x86)\Windows Kits"
    dir /b /s mc.exe

Append location to the `path` (example)

    set "path=%path%;c:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64"

Test with 

    where mc

## Build
Install Python 3.7.7 with all optional components

    cd \git\pywin32
    python setup.py -q build


