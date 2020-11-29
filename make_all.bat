@if "%1"=="quick" goto quick
@if "%1"=="already_built" goto already_built
if exist build\. rd /s/q build
if exist build\. goto couldnt_rm
:quick
call build_all.bat
@if errorlevel 1 goto failed
cd autoduck
call make.bat
@if errorlevel 1 goto failed
cd ..
:already_built
rem Now the binaries.

py -3.5-32 setup.py -q bdist_wininst --skip-build --target-version=3.5
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.5-32 setup.py -q bdist_wheel --skip-build
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.5 setup.py -q bdist_wininst --skip-build --target-version=3.5
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.5 setup.py -q bdist_wheel --skip-build

rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.6-32 setup.py -q bdist_wininst --skip-build --target-version=3.6
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.6-32 setup.py -q bdist_wheel --skip-build
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.6 setup.py -q bdist_wininst --skip-build --target-version=3.6
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.6 setup.py -q bdist_wheel --skip-build

rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.7-32 setup.py -q bdist_wininst --skip-build --target-version=3.7
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.7-32 setup.py -q bdist_wheel --skip-build
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.7 setup.py -q bdist_wininst --skip-build --target-version=3.7
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
py -3.7 setup.py -q bdist_wheel --skip-build

py -3.8-32 setup.py -q bdist_wininst --skip-build --target-version=3.8
py -3.8-32 setup.py -q bdist_wheel --skip-build
py -3.8 setup.py -q bdist_wininst --skip-build --target-version=3.8
py -3.8 setup.py -q bdist_wheel --skip-build

py -3.9-32 setup.py -q bdist_wininst --skip-build --target-version=3.9
py -3.9-32 setup.py -q bdist_wheel --skip-build
py -3.9 setup.py -q bdist_wininst --skip-build --target-version=3.9
py -3.9 setup.py -q bdist_wheel --skip-build

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
goto xit
:xit
