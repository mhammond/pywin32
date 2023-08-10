@echo off
@REM findstr: Ignore adodbapi until this has been manually ran and merged in adodbapi code
for /f "delims=" %%F in ('dir /b /s ".\*.py" ^| findstr /v /i "adodbapi"') do (
  @REM TODO: Progressively remove the keep-* flags and add --py38-plus
  pyupgrade --keep-percent-format --keep-mock --keep-runtime-typing %%F
)
@echo on
pycln . --config=pycln.toml
isort .
black .
