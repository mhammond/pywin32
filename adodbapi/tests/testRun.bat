if "%1" == "" goto skip_1
set PYTHON=%1
goto skip_python
:skip_1
if not "%PYTHON%"=="" goto skip_python
set PYTHON=python
:skip_python
Echo on
echo .
echo .
del *.pyc
echo .
REM unit test
call %PYTHON% testADOdbapi.py
echo .
echo .
REM generic test
call %PYTHON% test_adodbapi_dbapi20.py
echo .
echo .
pause Testing complete
