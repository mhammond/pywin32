# Thread and application objects

import object
import win32ui

class WinThread(object.CmdTarget):
	def __init__(self, initObj = None):
		if initObj is None:
			initObj = win32ui.CreateThread()
		object.CmdTarget.__init__(self, initObj)
		
	def InitInstance(self):
		return 1 # default is all OK!
	def ExitInstance(self):
		pass
		

class WinApp(WinThread):
	def __init__(self, initApp = None):
		if initApp is None:
			initApp = win32ui.GetApp()
		WinThread.__init__(self, initApp)
