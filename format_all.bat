@echo off
@REM Ignore adodbapi until this has been manually ran and merged in adodbapi code
@REM _dbgscript.py is non-UTF8 on purpose, which is not supported
for /f "delims=" %%F in ('git ls-files **.py* :!:adodbapi/* :!:Pythonwin/pywin/test/_dbgscript.py') do (
  @REM TODO: Remove the --keep-percent-format flag and fix all printf-style string formatting
  pyupgrade --keep-percent-format --py37-plus %%F
)
@echo on
pycln . --config=pycln.toml
isort .
black .
