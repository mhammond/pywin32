@echo off

rem Batch file for locating and executing SWIG.
rem SWIG binaries are kept under source control,
rem so this batch file should find that version.

if "%SWIG_EXE%"=="" goto not_set
if not exist %SWIG_EXE% goto not_set

echo NOTE: The build process has changed (hopefully for the better!)
echo You can remove your old SWIG install, and the %SWIG_EXE% 
echo environment variable.  All of SWIG is now kept under CVS,
echo and this version will be used if %%SWIG_EXE%% is not set.
goto set 

:not_set
if not exist ..\swig\swig.exe goto not_found
rem Pull a trick to convert the path to a full path.
rem EEEEK - pull a trick so that it also works in Win9x
rem Win9x patch submitted by Howard Lightstone, based on http://www.fpschultze.de/bsh.htm#a4
rem Stick the CWD into a CD variable.
If %OS%'==Windows_NT' (For %%D In (.) Do Set CD=%%~fD&Goto Endcd)
Echo Exit | %COMSPEC% /K Prompt Set CD=$P$_:>%TEMP%.\Tmp.bat
For %%C In (Call Del) Do %%C %TEMP%.\Tmp.bat
:Endcd
rem End of Set CD variable hack.

set SWIG_EXE=%CD%\..\swig\swig.exe
set SWIG_LIB=%CD%\..\swig\swig_lib

:set
cd %1
%SWIG_EXE% %2 %3 %4 %5 %6 %7 %8 %9
goto xit

:not_found
echo *** Can not find SWIG.
echo *** Please see "swig.bat" for how we attempt to locate SWIG.
goto xit

:xit
