Echo on
echo .
echo .
del *.pyc
echo .
REM unit test
adodbapitest.py
echo .
echo .
REM generic test
@echo . Note: one error is expected (no Warning class)
test_adodbapi_dbapi20.py
echo .
echo .
db_print.py
echo .
pause Testing complete
