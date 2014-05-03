@if "%1"=="quick" goto quick
@if "%1"=="already_built" goto already_built
if exist build\. rm -rf build
if exist build\. goto couldnt_rm
cd autoduck
call make.bat
@if errorlevel 1 goto failed
cd ..
:quick
call build_all.bat
:already_built
rem Now the binaries.

rem Yuck - 2to3 hackery - must nuke bdist dirs as it may hold py3x syntax.
if exist build/bdist.win32/. rm -rf build/bdist.win32
if exist build/bdist.win-amd64/. rm -rf build/bdist.win-amd64
py -2.7-32 setup.py -q sdist
py -2.6-32 setup.py -q bdist_wininst --target-version=2.5 --skip-build
py -2.6-32 setup.py -q bdist_wininst --target-version=2.6 --skip-build
py -2.6-32 setup.py -q bdist_wininst --target-version=2.6 --skip-build --plat-name=win-amd64 
py -2.7-32 setup.py -q bdist_wininst --target-version=2.7 --skip-build
py -2.7-32 setup.py -q bdist_wininst --target-version=2.7 --skip-build --plat-name=win-amd64 

rem Just incase - re-nuke bdist dirs so 2to3 always runs.
if exist build/bdist.win32/. rm -rf build/bdist.win32
if exist build/bdist.win-amd64/. rm -rf build/bdist.win-amd64

py -3.1-32 setup3.py -q bdist_wininst --target-version=3.1 --skip-build
py -3.1-32 setup3.py -q bdist_wininst --target-version=3.1 --skip-build --plat-name=win-amd64 

py -3.2-32 setup3.py -q bdist_wininst --target-version=3.2 --skip-build
py -3.2-32 setup3.py -q bdist_wininst --target-version=3.2 --skip-build --plat-name=win-amd64 

py -3.3-32 setup3.py -q bdist_wininst --target-version=3.3 --skip-build
py -3.3-32 setup3.py -q bdist_wininst --target-version=3.3 --skip-build --plat-name=win-amd64 

py -3.4-32 setup3.py -q bdist_wininst --target-version=3.4 --skip-build
py -3.4-32 setup3.py -q bdist_wininst --target-version=3.4 --skip-build --plat-name=win-amd64 

py -3.5-32 setup3.py -q bdist_wininst --target-version=3.5 --skip-build
py -3.5-32 setup3.py -q bdist_wininst --target-version=3.5 --skip-build --plat-name=win-amd64 

rem And nuke the dirs one more time :)
if exist build/bdist.win32/. rm -rf build/bdist.win32
if exist build/bdist.win-amd64/. rm -rf build/bdist.win-amd64

@goto xit
:couldnt_rm
@echo Could not remove the build directory!
goto xit
:failed
@echo Oops - failed!
goto xit
:xit