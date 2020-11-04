# A Python file that can be used to start Pythonwin, instead of using 
# pythonwin.exe
import win32ui
import pywin.framework.intpyapp
# Pretend this script doesn't exist, or pythonwin tries to edit it
import sys, os
sys.argv[:] = sys.argv[1:] or ['']    # like PySys_SetArgv(Ex)
if sys.path[0] not in ('.', os.getcwd()):
    sys.path.insert(0, os.getcwd())
# And bootstrap the app.
app=win32ui.GetApp()
app.InitInstance()
app.Run()
