@Echo off
if "%1"=="" goto skip_arg
set PYTHON=%1
goto skip_python:
:skip_arg
set PYTHON=py -2.7
:skip_python
Echo on
echo .
echo .
del *.pyc
echo .
REM unit test
call %PYTHON% adodbapitest.py --time %2 %3 %4 %5
echo .
echo .
REM generic test
call %PYTHON% test_adodbapi_dbapi20.py %2
echo .
echo .
@Echo off
pause Testing complete
