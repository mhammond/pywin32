 # help.py - help utilities for PythonWin.
import win32api
import win32con
import win32ui
import string
import sys
import regutil
import string, os

def OpenHelpFile(fileName, helpCmd = None, helpArg = None):
	"Open a help file, given a full path"
	# default help arg.
	win32ui.DoWaitCursor(1)
	try:
		ext = os.path.splitext(fileName)[1]
		if string.lower(ext) == ".hlp":
			if helpCmd is None: helpCmd = win32con.HELP_CONTENTS
			win32api.WinHelp( win32ui.GetMainFrame().GetSafeHwnd(), fileName, helpCmd, helpArg)
		else:
			# Hope that the extension is registered, and we know what to do!
			win32api.ShellExecute(0, "open", fileName, None, "", win32con.SW_SHOW)
		return fileName
	finally:
		win32ui.DoWaitCursor(-1)

def ListAllHelpFiles():
	"""Returns a list of (helpDesc, helpFname) for all registered help files
	"""
	import regutil
	retList = []
	try:
		key = win32api.RegOpenKey(regutil.GetRootKey(), regutil.BuildDefaultPythonKey() + "\\Help", 0, win32con.KEY_READ)
	except win32api.error, (code, fn, details):
		import winerror
		if code!=winerror.ERROR_FILE_NOT_FOUND:
			raise win32api.error, (code, fn, desc)
		return retList
	try:
		keyNo = 0
		while 1:
			try:
				helpDesc = win32api.RegEnumKey(key, keyNo)
				helpFile = win32api.RegQueryValue(key, helpDesc)
				retList.append(helpDesc, helpFile)
				keyNo = keyNo + 1
			except win32api.error, (code, fn, desc):
				import winerror
				if code!=winerror.ERROR_NO_MORE_ITEMS:
					raise win32api.error, (code, fn, desc)
				break
	finally:
		win32api.RegCloseKey(key)
	return retList

def SelectAndRunHelpFile():
	from pywin.dialogs import list
	helpFiles = ListAllHelpFiles()
	index = list.SelectFromLists("Select Help file", helpFiles, ["Title"])
	if index is not None:
		OpenHelpFile(helpFiles[index][1])


helpIDMap = None

def SetHelpMenuOtherHelp(mainMenu):
	"""Modifies the main Help Menu to handle all registered help files.
	   mainMenu -- The main menu to modify - usually from docTemplate.GetSharedMenu()
	"""

	# Load all help files from the registry.
	if helpIDMap is None:
		global helpIDMap
		helpIDMap = {}
		cmdID = win32ui.ID_HELP_OTHER
		excludeList = ['Main Python Documentation', 'Pythonwin Reference']
		firstList = ListAllHelpFiles()
		helpDescs = []
		for desc, fname in firstList:
			if desc not in excludeList:
				helpIDMap[cmdID] = (desc, fname)
				win32ui.GetMainFrame().HookCommand(HandleHelpOtherCommand, cmdID)
				cmdID = cmdID + 1

	helpMenu = mainMenu.GetSubMenu(mainMenu.GetMenuItemCount()-1) # Help menu always last.
	otherHelpMenuPos = 2 # cant search for ID, as sub-menu has no ID.
	otherMenu = helpMenu.GetSubMenu(otherHelpMenuPos)
	while otherMenu.GetMenuItemCount():
		otherMenu.DeleteMenu(0, win32con.MF_BYPOSITION)
	
	if helpIDMap:
		for id, (desc, fname) in helpIDMap.items():
			otherMenu.AppendMenu(win32con.MF_ENABLED|win32con.MF_STRING,id, desc)
	else:
		helpMenu.EnableMenuItem(otherHelpMenuPos, win32con.MF_BYPOSITION | win32con.MF_GRAYED)
		
def HandleHelpOtherCommand(cmd, code):
	OpenHelpFile(helpIDMap[cmd][1])
