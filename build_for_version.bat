@echo off
call removebuilt.bat
if errorlevel 1 goto failed
python set_for_version.py %1
if errorlevel 1 goto failed
msdev "Python and Extensions.dsw" /MAKE ALL
if errorlevel 1 goto failed
print "All worked!"
goto xit

:failed
echo Build Failed

:xit
