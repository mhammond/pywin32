@echo off
rem Incomplete assemblies make make peverify hang - delete the target.
if exist %1.exe del %1.exe
cpy.py -w:4 /o:%1.exe /debug %2 %3 %4 %5 %1.py
if errorlevel 1 goto xit
peverify /IL /quiet %1.exe
if errorlevel 1 goto fail_verify
%1.exe
goto xit

:fail_verify
peverify /IL %1.exe 

:xit
