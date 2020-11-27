# A Python file that can be used to start Pythonwin, instead of using 
# pythonwin.exe
import sys

def _build_app():
    import os
    # Pretend this script doesn't exist, or pythonwin tries to edit it
    sys.argv[:] = sys.argv[1:] or ['']    # like PySys_SetArgv(Ex)
    if sys.path[0] not in ('', '.', os.getcwd()):
        sys.path.insert(0, os.getcwd())
    # And build the app.
    import win32ui
    import pywin.framework.startup  # noqa
    app = win32ui.GetApp()
    app.InitInstance()
    return app

if __name__ == '__main__':
    _org_stdout = sys.stdout
    _build_app().Run()
    sys.stdout = sys.stderr = _org_stdout
