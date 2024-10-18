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

rem Check /build_env.md#build-environment to make sure you have all the required components installed

py -3.8-32 setup.py -q bdist_wheel --skip-build
py -3.8 setup.py -q bdist_wheel --skip-build

py -3.9-32 setup.py -q bdist_wheel --skip-build
py -3.9 setup.py -q bdist_wheel --skip-build

py -3.10-32 setup.py -q bdist_wheel --skip-build
py -3.10 setup.py -q bdist_wheel --skip-build

py -3.11-32 setup.py -q bdist_wheel --skip-build
py -3.11 setup.py -q bdist_wheel --skip-build

py -3.12-32 setup.py -q bdist_wheel --skip-build
py -3.12 setup.py -q bdist_wheel --skip-build

py -3.13-32 setup.py -q bdist_wheel --skip-build
py -3.13 setup.py -q bdist_wheel --skip-build

rem Check /build_env.md#build-environment to make sure you have all the required ARM64 components installed
py -3.10 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64
py -3.11 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64
py -3.12 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64
py -3.13 setup.py -q build_ext --plat-name win-arm64 build --plat-name win-arm64 bdist_wheel --plat-name win-arm64

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
@exit /b 1
:xit
