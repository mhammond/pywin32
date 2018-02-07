rem All builds below work with version 7.1 of the MSSDK (and possibly later)
set mssdk=c:\mssdk\7.1
py -2.7-32 setup.py -q build
@if errorlevel 1 goto failed
py -2.7 setup.py -q build
@if errorlevel 1 goto failed
rem py3k
py -3.5-32 setup3.py -q build
@if errorlevel 1 goto failed
py -3.5 setup3.py -q build
@if errorlevel 1 goto failed
py -3.6-32 setup3.py -q build
@if errorlevel 1 goto failed
py -3.6 setup3.py -q build
@if errorlevel 1 goto failed
py -3.7-32 setup3.py -q build
@if errorlevel 1 goto failed
py -3.7 setup3.py -q build
@if errorlevel 1 goto failed

goto xit
:failed
@echo Oops - failed!
goto xit
:xit
