py -3.9-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.9 setup.py -q build
@if errorlevel 1 goto failed
py -3.10-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.10 setup.py -q build
@if errorlevel 1 goto failed
py -3.11-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.11 setup.py -q build
@if errorlevel 1 goto failed
py -3.12-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.12 setup.py -q build
@if errorlevel 1 goto failed
py -3.13-32 setup.py -q build
@if errorlevel 1 goto failed
py -3.13 setup.py -q build
@if errorlevel 1 goto failed

goto xit
:failed
@echo Oops - failed!
@exit /b 1
:xit
