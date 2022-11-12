@if "%1"=="quick" goto quick
@if "%1"=="already_built" goto already_built
if exist build\. rd /s/q build
if exist build\. goto couldnt_rm
:quick
call build_all.bat
@if errorlevel 1 goto failed
py autoduck\make.py
@if errorlevel 1 goto failed
:already_built
rem Now the binaries.

rem (bdist_wininst needs --target-version to name the installers correctly!)
py -3.7-32 setup.py -q bdist_wininst --skip-build --target-version=3.7
py -3.7-32 setup.py -q bdist_wheel --skip-build
py -3.7 setup.py -q bdist_wininst --skip-build --target-version=3.7
py -3.7 setup.py -q bdist_wheel --skip-build

py -3.8-32 setup.py -q bdist_wininst --skip-build --target-version=3.8
py -3.8-32 setup.py -q bdist_wheel --skip-build
py -3.8 setup.py -q bdist_wininst --skip-build --target-version=3.8
py -3.8 setup.py -q bdist_wheel --skip-build

py -3.9-32 setup.py -q bdist_wininst --skip-build --target-version=3.9
py -3.9-32 setup.py -q bdist_wheel --skip-build
py -3.9 setup.py -q bdist_wininst --skip-build --target-version=3.9
py -3.9 setup.py -q bdist_wheel --skip-build

rem 3.10 stopped supporting bdist_wininst, but we can still build them with 3.9
rem (but 32bit builds seem broken doing this :( #1805)
py -3.9 setup.py -q bdist_wininst --skip-build --target-version=3.10
py -3.10-32 setup.py -q bdist_wheel --skip-build
py -3.10 setup.py -q bdist_wheel --skip-build

py -3.9 setup.py -q bdist_wininst --skip-build --target-version=3.11
py -3.11-32 setup.py -q bdist_wheel --skip-build
py -3.11 setup.py -q bdist_wheel --skip-build

py -3.9 setup.py -q bdist_wininst --skip-build --target-version=3.12
py -3.12-32 setup.py -q bdist_wheel --skip-build
py -3.12 setup.py -q bdist_wheel --skip-build

rem ARM64 builds - requires you to select:
rem * "Visual C++ compilers and libraries for ARM64"
rem * "Visual C++ for MFC for ARM64"
rem from "Individual Components" in VS setup.
py -3.10 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64
py -3.11 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64
py -3.12 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
@exit /b 1
:xit
