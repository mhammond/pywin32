py -2.7-32 setup.py -q build
@if errorlevel 1 goto failed
py -2.7 setup.py -q build
@if errorlevel 1 goto failed
rem py3k
py -3.5-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.5 setup.py -q build
@if errorlevel 1 goto failed
py -3.6-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.6 setup.py -q build
@if errorlevel 1 goto failed
py -3.7-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.7 setup.py -q build
@if errorlevel 1 goto failed
py -3.8-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.8 setup.py -q build
@if errorlevel 1 goto failed
py -3.9-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.9 setup.py -q build
@if errorlevel 1 goto failed

goto xit
:failed
@echo Oops - failed!
goto xit
:xit
