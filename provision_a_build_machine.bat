@echo off
echo . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
echo . This batch command file will start the process of installing, onto this computer,
echo . all of the software needed to build and test a pywin32 distribution.
echo .
echo . (However, if you wish, the build kit may be on Virtual Machine(s) hosted on this computer.)
echo .
echo . If permitted to proceed, it will do the following: . . .
echo .
echo . 1) "git clone" a moderately large system (http://github.com/salt-bevy/salt-bevy)
echo .      into a directory adjacent to the current directory.
echo . 2) Test for the presence of Python3 and the Python launcher for Windows.
echo . 3) Instruct the operator how to install them, if needed.
echo . 4) install a masterless Salt minion from http://bootstrap.saltstack.com
echo . 5) install "sudo.py" into your C:\Windows directory, and make ".py" a working command extension.
echo . 6) "sudo py -3 -m pip install ifaddr pyyaml passlib"
echo . 7) run a large Python script which will guide you through configuring your system
echo . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
echo "If you do not want to do all that, then hit <Ctrl-C> now!"
pause
echo on
if not exist ..\salt-bevy\README.md goto clone_it
  pushd ..\salt-bevy
  git pull
  popd
goto test_python
:clone_it
  pushd ..
  git clone https://github.com/salt-bevy/salt-bevy.git
  popd
:test_python
@echo off

py -3 -c "import sys; print('found --> Python', sys.version); exit(sys.version < '3.5')"
if errorlevel 1 (
echo * * * * * * *
echo Python3 is not installed or is too old.
echo .
echo Open a "Command Prompt" window as an "Administrator", and run the following batch file:
dir /b /s ..\salt-bevy\configure_machine\install_windows_python.bat
echo After installing Python3, re-run this script.
echo * * * * * * *
exit /b
)
pushd ..\salt-bevy
echo .
echo Installing Salt minion . . .
echo Please give permission for installations when prompted.
pause
echo on
pushd configure_machine
call bootstrap_salt.bat
@echo .
@echo Next, installing sudo.py as a system CLI command.
@pause
pushd helpers
py -3 sudo.py --install-sudo-command
@echo .
@echo Next, installing some prerequisite Python modules.
@pause
py -3 sudo.py --pause py -3 -m pip install ifaddr pyyaml passlib
popd
popd
popd
@echo off
echo * * * * * * *
echo So far, so good.
echo .
echo Next, we'll run a configuration program to set up exactly how you intend to use your build system.
echo For a simple build machine, just select the bevy name of "local" and take lots of defaults.
echo .
echo If you want to do complete testing, you can select a name for your bevy (such as "pw32") and plan for a
echo Salt Master virtual machine to provision your mini-network of Linux database server virtual machines.
echo * * * * * * *
pause
..\salt-bevy\join-bevy.bat
