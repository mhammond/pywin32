# A Python file that can be used to start Pythonwin, instead of using 
# pythonwin.exe
import win32ui
import pywin.framework.intpyapp
# Pretend this script doesn't exist, or pythonwin tries to edit it
import sys
del sys.argv[0]
# And bootstrap the app.
app=win32ui.GetApp()
app.InitInstance()
app.Run()
