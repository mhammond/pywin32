rem @echo off
set src=win32
set zip=zip
set excludes=-x *.pyc -x *.pyo -x *.plg -x *.ncb

move %src%\Build \%src%_build
if exist %src%\Build\. goto bad_move
if exist %src%_src.zip del %src%_src.zip
%zip% -r -9 %src%_src.zip %src%/* %excludes%
move \%src%_build %src%\Build

set src=Pythonwin
move %src%\Build \%src%_build
if exist %src%\Build\. goto bad_move
if exist %src%_src.zip del %src%_src.zip
%zip% -r -9 %src%_src.zip %src%/* %excludes%
move \%src%_build %src%\Build

set src=com
move %src%\Build \%src%_build
move %src%\TestSources\Build \%src%_TestSources_build
if exist %src%\Build\. goto bad_move
if exist win32com_src.zip del win32com_src.zip
%zip% -r -9 win32com_src.zip %src%/* %excludes%
move \%src%_build %src%\Build
move \%src%_testSources_build %src%\TestSources\Build

:tools
rem Special "Rest of my tools" archive
if exist BuildTools.zip del BuildTools.zip
del Wise\*.pyc /s 2>nul
del Autoduck\*.pyc /s 2>nul
move Wise\PyWise\Build \PyWise_build
%zip% -9 BuildTools.zip *.dsw
%zip% -9 -r BuildTools.zip AutoDuck/*
%zip% -9 -r BuildTools.zip Wise/*.wse Wise/*.py Wise/*.txt Wise/PyWise/* "Wise/Install Scripts/*"
%zip% -9    BuildTools.zip Wise/PyWise/*
rem %zip% -9 -r BuildTools.zip d:\dbgsdk\*
%zip% -9 -r BuildTools.zip *.dsw
rem Get the SWIG .I files.
e:
pushd \pythonex\swig1.1
%zip% -9 -r d:BuildTools.zip swig_lib\python\pywintypes.i
%zip% -9 -r d:BuildTools.zip swig_lib\python\pythoncom.i
%zip% -9 -r d:BuildTools.zip swig_lib\python\pywin32.i
popd
d:

move \PyWise_build Wise\PyWise\Build

goto xit

:bad_move
echo A build directory could not be moved!
goto xit

:xit
set src=
set zip=
set excludes=
