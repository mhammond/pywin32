@if "%1"=="quick" goto quick
if exist build\. rd /s/q build
if exist build\. goto couldnt_rm
:quick
py autoduck\make.py
@if errorlevel 1 goto failed

rem Check /build_env.md#build-environment to make sure you have all the required components installed
@echo off
for %%V in (
    3.9
    3.10
    3.11
    3.12
    3.13
    3.14
    3.15
) do (
    py -%%V-32 -m build --wheel
    py -%%V -m build --wheel
)

rem Check /build_env.md#cross-compiling-for-arm64-microsoft-visual-c-141-and-up to make sure you have all the required ARM64 components installed
@echo off
for %%V in (
    3.10
    3.11
    3.12
    3.13
    3.14
    3.15
) do (
    py -%%V -m build --wheel --config-setting=--build-option="build_ext --plat-name=win-arm64 build --plat-name=win-arm64 bdist_wheel --plat-name=win-arm64"
)

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
@exit /b 1
:xit
