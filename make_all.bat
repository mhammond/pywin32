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

rem Yuck - 2to3 hackery - must nuke bdist dirs as it may hold py3x syntax.
if exist build/bdist.win32/. rd /s/q build\bdist.win32
if exist build/bdist.win-amd64/. rd /s/q build\bdist.win-amd64
py -2.7-32 setup.py -q bdist_wininst --target-version=2.7 --skip-build
py -2.7-32 setup.py -q bdist_wheel --skip-build
py -2.7 setup.py -q bdist_wininst --target-version=2.7 --skip-build
py -2.7 setup.py -q bdist_wheel --skip-build

rem Just incase - re-nuke bdist dirs so 2to3 always runs.
if exist build/bdist.win32/. rd /s/q build\bdist.win32
if exist build/bdist.win-amd64/. rd /s/q build\bdist.win-amd64

rem *sob* - for some reason 3.5 and later are failing to remove the bdist temp dir
rem due to the mfc DLLs - but the dir can be removed manually.
rem I've excluded the possibility of anti-virus or the indexer.
rem So manually nuke them before builds.
rem @if exist build\bdist.win32 rd /s/q build\bdist.win32 & @if exist build\bdist.amd64 rd /s/q build\bdist.amd64
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

rem And nuke the dirs one more time :)
if exist build/bdist.win32/. rd /s/q build\bdist.win32
if exist build/bdist.win-amd64/. rd /s/q build\bdist.win-amd64

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
goto xit
:xit