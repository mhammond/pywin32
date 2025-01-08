/*
@doc

@topic win32com.shell and Windows Shell Links|
Following is documentation for the PyIShellLink object.

@ex To create a <o PyIShellLink> object|
	from win32com.shell import shell
	import pythoncom
	shortcut = pythoncom.CoCreateInstance(
		shell.CLSID_ShellLink, None,
		pythoncom.CLSCTX_INPROC_SERVER, shell.IID_IShellLink
	)

@ex To load information from existing shortcut file|
	shortcut.QueryInterface( pythoncom.IID_IPersistFile ).Load( filename )

@ex To save information to a file|
	shortcut.QueryInterface( pythoncom.IID_IPersistFile ).Save( filename, 0 )

@ex This documentation class is based on:
	http://msdn.microsoft.com/isapi/msdnlib.idc?theURL=/library/sdkdoc/shellcc/shell/ifaces/ishelllink/ishelllink.htm
    (TODO: Update to https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ishelllinkw)
	With only minor alterations and notations by Mike Fletcher.
	Errors may be present, read at your own risk.
|
class PyIShellLink( IPersistFile ):
	''' Following is not a functional class, intended solely for documentation '''
	def GetArguments(self):
		'''Retrieves the command-line arguments associated with a shell link object. '''
	def SetArguments(self, argumentString):
		'''Sets the command-line arguments associated with a shell link object.'''

	def GetDescription(self):
		'''Retrieves the description string for a shell link object. '''
	def SetDescription(self, descriptionString):
		'''Sets the description string for a shell link object. '''

	def GetIconLocation(self):
		'''Retrieves the location (path and index) of the icon for a shell link object.
		Returns a tuple of string and integer'''
	def SetIconLocation(self, locationString, iconIndex):
		'''Sets the location (path and index) of the icon for a shell link object. '''

	def GetPath(self, flags):
		'''Retrieves the path and file name of a shell link object.
		Note: flags are available through shell.SLGP_*
			SLGP_SHORTPATH  Retrieves the standard short (8.3 format) file name.
			SLGP_UNCPRIORITY  Retrieves the Universal Naming Convention (UNC) path name
			of the file.
			SLGP_RAWPATH  Retrieves the raw path name. A raw path is something that might
			not exist and may include environment variables that need to be expanded.'''
	def SetPath(self, pathString):
		'''Sets the path and file name of a shell link object. '''
	def SetRelativePath(self, pathString):
		'''Sets the relative path for a shell link object.
		Note: This mechanism allows for moved link files
		to reestablish connection with relative files through
		similar-prefix comparisons'''

	def GetShowCmd(self):
		'''Retrieves the show (SW_) command for a shell link object.'''
	def SetShowCmd(self, constant):
		'''Sets the show (SW_) command for a shell link object.
		Note: constants are defined in win32con, ie. win32con.SW_*
			SW_SHOWNORMAL Activates and displays a window. If the window is minimized or
			maximized, the system restores it to its original size and position. An
			application should specify this flag when displaying the window for the first
			time.
			SW_SHOWMAXIMIZED Activates the window and displays it as a maximized window.
			SW_SHOWMINIMIZED Activates the window and displays it as a minimized window.
		'''

	def GetWorkingDirectory(self):
		'''Retrieves the name of the working directory for a shell link object. '''
	def SetWorkingDirectory(self, pathString):
		'''Sets the name of the working directory for a shell link object.'''

	def Resolve(self, window, flags):
		'''Resolves a shell link by searching for the shell link object and updating the
		shell link path and its list of identifiers (if necessary).
		Notes:
			window is the parent window of a dialog which will pop up if resolution fails
		flags:
			SLR_INVOKE_MSI  Call the Microsoft Windows Installer.
			SLR_NOLINKINFO  Disable distributed link tracking. By default, distributed
				link tracking tracks removable media across multiple devices based on the
				volume name. It also uses the UNC path to track remote file systems whose
				drive letter has changed. Setting SLR_NOLINKINFO disables both types of tracking.
			SLR_NO_UI       Do not display a dialog box if the link cannot be resolved. When
				SLR_NO_UI is set, the high-order word of fFlags can be set to a time-out value
				that specifies the maximum amount of time to be spent resolving the link. The
				function returns if the link cannot be resolved within the time-out duration.
				If the high-order word is set to zero, the time-out duration will be set to the
				default value of 3,000 milliseconds (3 seconds). To specify a value, set the high
				word of fFlags to the desired time-out duration, in milliseconds.
			SLR_NOUPDATE    Do not update the link information.
			SLR_NOSEARCH    Do not execute the search heuristics.
			SLR_NOTRACK     Do not use distributed link tracking.
			SLR_UPDATE      If the link object has changed, update its path and list of identifiers. If SLR_UPDATE is set, you do not need to call IPersistFile::IsDirty to determine whether or not the link object has changed.
		'''

	### Problematic elements
	# The problems below are due primarily to structs used in the API
	def GetIDList(self):
		'''Retrieves the list of item identifiers for a shell link object.
		Note: I do not see how to manipulate these identifiers, they are C structs,
		so I suppose we might be able to unpack them with the struct module.
		However, when I attempt this, I seem to get messed up identifiers
		(lengths of 0 for the individual identifiers) (see malfunctioning code below)'''
	def SetIDList(self, IDList):
		'''Sets the list of item identifiers for a shell link object.
		Note: See comments on GetIDList'''
	def SetHotkey(self, Hotkey):
		'''Sets the hot key for a shell link object.
		Note: New hot key. The virtual key code is in the low-order byte, and the modifier
		flags are in the high-order byte. The modifier flags can be a combination of the
		values specified in the description of the IShellLink::GetHotkey method.
		Note: I cannot find these constants anywhere...
			HOTKEYF_ALT, HOTKEYF_CONTROL, HOTKEYF_EXT, HOTKEYF_SHIFT
		'''
	def GetHotkey (self):
		'''Retrieves the hot key for a shell link object.
		Note: My tests do not seem to be working. at least, the values returned
		seem not to match what the documentation says should be returned.
		I would expect with a Hotkey of CTRL-ALT-T, to get an integer where
		integer & 256 == ord('T'), i.e. 116 or 84, instead I get 1620
		'''

import struct
def readIDList (data):
	''' unpack data into list of identifiers
	The following is not functional!'''
	result = []
	headersize = struct.calcsize('H')
	while data:
		count = struct.unpack('H', data[:headersize])[0]
		if count:
			result.append( data[:count] )
			data = data[count:]
		else:
			raise ValueError, ('ID item of length zero defined', data )

*/
