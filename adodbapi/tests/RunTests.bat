if "%1"=="" goto skip_arg
set PYTHON=%1
:skip_arg
if not "%PYTHON%"=="" goto skip_python
set PYTHON=python
:skip_python
Echo on
echo .
echo .
del *.pyc
echo .
REM unit test
%PYTHON% adodbapitest.py
echo .
echo .
REM generic test
%PYTHON% test_adodbapi_dbapi20.py
echo .
echo .
pause Testing complete
