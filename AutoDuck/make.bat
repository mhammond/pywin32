@echo off
if not x%HHC%==x goto doit
hhc > nul 2>&1
if not errorlevel 1 set HHC=hhc & goto doit

if exist "\Program files\HTML Help Workshop\hhc.exe" set HHC="\Program files\HTML Help Workshop\hhc.exe" & goto doit
if exist "C:\Program files\HTML Help Workshop\hhc.exe" set HHC="C:\Program files\HTML Help Workshop\hhc.exe" & goto doit

echo Can not locate HHC.EXE - please set the HHC environment to point to the .exe
goto xit

:doit
nmake -E -f com.mak %1 %2 %3 %4 %5
if errorlevel 1 goto xit
nmake -E -f win32.mak %1 %2 %3 %4 %5
if errorlevel 1 goto xit
nmake -E -f pythonwin.mak %1 %2 %3 %4 %5
if errorlevel 1 goto xit
rem Make the combined chm.
nmake -E -f bundle.mak %1 %2 %3 %4 %5

:xit

