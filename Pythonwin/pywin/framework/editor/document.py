from pywin.mfc import docview, object
from pywin.framework.editor import GetEditorOption
import win32ui
import os
import win32con
import string
import traceback
import win32api

BAK_NONE=0
BAK_DOT_BAK=1
BAK_DOT_BAK_TEMP_DIR=2
BAK_DOT_BAK_BAK_DIR=3

ParentEditorDocument=docview.Document
class EditorDocumentBase(ParentEditorDocument):
	def __init__(self, template):
		self.bAutoReload = GetEditorOption("Auto Reload", 1)
		self.bDeclinedReload = 0 # Has the user declined to reload.
		self.fileStat = None

		# what sort of bak file should I create.
		# default to write to %temp%/bak/filename.ext
		self.bakFileType=GetEditorOption("Backup Type", BAK_DOT_BAK_BAK_DIR)

		# Should I try and use VSS integration?
		self.scModuleName=GetEditorOption("Source Control Module", "")
		self.scModule = None # Loaded when first used.
		# Skip the direct parent
		object.CmdTarget.__init__(self, template.CreateWin32uiDocument())

	# Helper for reflecting functions to views.
	def _ApplyToViews(self, funcName, *args):
		for view in self.GetAllViews():
			func = getattr(view, funcName)
			apply(func, args)
	def _ApplyOptionalToViews(self, funcName, *args):
		for view in self.GetAllViews():
			func = getattr(view, funcName, None)
			if func is not None:
				apply(func, args)

	def OnSaveDocument( self, fileName ):
		win32ui.SetStatusText("Saving file...",1)
		# rename to bak if required.
		dir, basename = os.path.split(fileName)
		if self.bakFileType==BAK_DOT_BAK:
			bakFileName=dir+'\\'+os.path.splitext(basename)[0]+'.bak'
		elif self.bakFileType==BAK_DOT_BAK_TEMP_DIR:
			bakFileName=win32api.GetTempPath()+'\\'+os.path.splitext(basename)[0]+'.bak'
		elif self.bakFileType==BAK_DOT_BAK_BAK_DIR:
			tempPath=os.path.join(win32api.GetTempPath(),'bak')
			try:
				os.mkdir(tempPath,0)
			except os.error:
				pass
			bakFileName=os.path.join(tempPath,basename)
		try:
			os.unlink(bakFileName)	# raise NameError if no bakups wanted.
		except (os.error, NameError):
			pass
		try:
			os.rename(fileName, bakFileName)
		except (os.error, NameError):
			pass
		try:
			self.SaveFile(fileName)
		except IOError, details:
			win32ui.MessageBox("Error - could not save file\r\n\r\n%s"%details)
			return 0
		self.SetModifiedFlag(0) # No longer dirty
		self.bDeclinedReload = 0 # They probably want to know if it changes again!
		win32ui.AddToRecentFileList(fileName)
		self.SetPathName(fileName)
		win32ui.SetStatusText("Ready")
		self._DocumentStateChanged()
		return 1

	# Support for reloading the document from disk - presumably after some
	# external application has modified it (or possibly source control has
	# checked it out.
	def ReloadDocument(self):
		"""Reloads the document from disk.  Assumes the file has
		been saved and user has been asked if necessary - it just does it!
		"""
		win32ui.SetStatusText("Reloading document.  Please wait...", 1)
		self.SetModifiedFlag(0)
		# Loop over all views, saving their state, then reload the document
		views = self.GetAllViews()
		states = []
		for view in views:
			try:
				info = view._PrepareUserStateChange()
			except AttributeError: # Not our editor view?
				info = None
			states.append(info)
		self.OnOpenDocument(self.GetPathName())
		for view, info in map(None, views, states):
			if info is not None:
				view._EndUserStateChange(info)
		win32ui.SetStatusText("Document reloaded.")

	# Reloading the file
	def CheckExternalDocumentUpdated(self):
		if self.bDeclinedReload or not self.GetPathName():
			return
		try:
			newstat = os.stat(self.GetPathName())
		except os.error, (code, msg):
			print "Warning on file %s - %s" % (self.GetPathName(), msg)
			self.bDeclinedReload = 1
			return
		changed = (self.fileStat is None) or \
			self.fileStat[0] != newstat[0] or \
			self.fileStat[6] != newstat[6] or \
			self.fileStat[8] != newstat[8] or \
			self.fileStat[9] != newstat[9]
		if changed:
			question = None
			if self.IsModified():
				question = "%s\r\n\r\nThis file has been modified outside of the source editor.\r\nDo you want to reload it and LOSE THE CHANGES in the source editor?" % self.GetPathName()
				mbStyle = win32con.MB_YESNO | win32con.MB_DEFBUTTON2 # Default to "No"
			else:
				if not self.bAutoReload:
					question = "%s\r\n\r\nThis file has been modified outside of the source editor.\r\nDo you want to reload it?" % self.GetPathName()
					mbStyle = win32con.MB_YESNO # Default to "Yes"
			if question:
				rc = win32ui.MessageBox(question, None, mbStyle)
				if rc!=win32con.IDYES:
					self.bDeclinedReload = 1
					return
			self.ReloadDocument()

	def _DocumentStateChanged(self):
		"""Called whenever the documents state (on disk etc) has been changed
		by the editor (eg, as the result of a save operation)
		"""
		if self.GetPathName():
			try:
				self.fileStat = os.stat(self.GetPathName())
			except os.error:
				self.fileStat = None
		else:
			self.fileStat = None
		self._UpdateUIForState()
		self._ApplyOptionalToViews("_UpdateUIForState")
			
	# Read-only document support - make it obvious to the user
	# that the file is read-only.
	def _IsReadOnly(self):
		return self.fileStat is not None and (self.fileStat[0] & 128)==0

	def _UpdateUIForState(self):
		"""Change the title to reflect the state of the document - 
		eg ReadOnly, Dirty, etc
		"""
		filename = self.GetPathName()
		if not filename: return # New file - nothing to do
		try:
			# This seems necessary so the internal state of the window becomes
			# "visible".  without it, it is still shown, but certain functions
			# (such as updating the title) dont immediately work?
			self.GetFirstView().ShowWindow(win32con.SW_SHOW)
			title = win32ui.GetFileTitle(filename)
		except win32ui.error:
			title = filename
		if self._IsReadOnly():
			title = title + " (read-only)"
		self.SetTitle(title)

	def MakeDocumentWritable(self):
		if not self.scModuleName: # No Source Control support.
			win32ui.SetStatusText("Document is read-only, and no source-control system is configured")
			win32api.MessageBeep()
			return 0

		# We have source control support - check if the user wants to use it.
		msg = "Would you like to check this file out?"
		defButton = win32con.MB_YESNO
		if self.IsModified(): 
			msg = msg + "\r\n\r\nALL CHANGES IN THE EDITOR WILL BE LOST"
			defButton = win32con.MB_YESNO
		if win32ui.MessageBox(msg, None, defButton)!=win32con.IDYES:
			return 0
			
		# Now call on the module to do it.
		if self.scModule is None:
			try:
				self.scModule = __import__(self.scModuleName)
				for part in string.split(self.scModuleName,'.')[1:]:
					self.scModule = getattr(self.scModule, part)
			except:
				traceback.print_exc()
				print "Error loading source control module."
				return 0
		
		if self.scModule.CheckoutFile(self.GetPathName()):
			self.ReloadDocument()
			return 1
		return 0

	def CheckMakeDocumentWritable(self):
		if self._IsReadOnly():
			return self.MakeDocumentWritable()
		return 1

	def SaveModified(self):
		# Called as the document is closed.  If we are about
		# to prompt for a save, bring the document to the foreground.
		if self.IsModified():
			frame = self.GetFirstView().GetParentFrame()
			try:
				frame.MDIActivate()
				frame.AutoRestore()
			except:
				print "Could not bring document to foreground"
		return self._obj_.SaveModified()

