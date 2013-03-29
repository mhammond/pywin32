@Echo off
if "%1"=="" goto skip_arg
set PYTHON=%1
goto skip_python:
:skip_arg
set PYTHON=python
:skip_python
Echo on
echo .
echo .
del *.pyc
echo .
REM unit test
call %PYTHON% adodbapitest.py --all %2 %3
echo .
echo .
REM generic test
call %PYTHON% test_adodbapi_dbapi20.py %2
echo .
echo .
@Echo off
pause Testing complete
