@echo off

rem Batch file for locating and executing SWIG.
rem SWIG binaries are kept under source control,
rem so this batch file should find that version.

if "%SWIG_EXE%==" goto not_set
if not exist %SWIG_EXE% goto not_set

echo NOTE: The build process has changed (hopefully for the better!)
echo You can remove your old SWIG install, and the %SWIG_EXE% 
echo environment variable.  All of SWIG is now kept under CVS,
echo and this version will be used if %%SWIG_EXE%% is not set.
goto set 

:not_set
if not exist ..\swig\swig.exe goto not_found
rem Pull a trick to convert the path to a full path.
for %%Iterator in (..\swig\swig.exe) do set SWIG_EXE=%%~fIterator
for /D %%Iterator in (..\swig\swig_lib) do set SWIG_LIB=%%~fIterator

:set
cd %1
%SWIG_EXE% %2 %3 %4 %5 %6 %7 %8 %9
goto xit

:not_found
echo *** Can not find SWIG.
echo *** Please see "swig.bat" for how we attempt to locate SWIG.
goto xit

:xit
