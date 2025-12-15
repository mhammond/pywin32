@if "%1"=="quick" goto quick
if exist build\. rd /s/q build
if exist build\. goto couldnt_rm
:quick
py autoduck\make.py
@if errorlevel 1 goto failed

rem Check /build_env.md#build-environment to make sure you have all the required components installed

py -3.9-32 -m build --wheel
py -3.9 -m build --wheel

py -3.10-32 -m build --wheel
py -3.10 -m build --wheel

py -3.11-32 -m build --wheel
py -3.11 -m build --wheel

py -3.12-32 -m build --wheel
py -3.12 -m build --wheel

py -3.13-32 -m build --wheel
py -3.13 -m build --wheel

py -3.14-32 -m build --wheel
py -3.14 -m build --wheel

rem Check /build_env.md#cross-compiling-for-arm64-microsoft-visual-c-141-and-up to make sure you have all the required ARM64 components installed
py -3.10 -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
py -3.11 -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
py -3.12 -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
py -3.13 -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
py -3.14 -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
@exit /b 1
:xit
