import win32ui
from pywin.mfc import docview
from scintillacon import *
import win32con
import string
import array

ParentScintillaDocument=docview.Document
class CScintillaDocument(ParentScintillaDocument):
	"A SyntEdit document. "
	def DeleteContents(self):
		self.text = ""

	def OnOpenDocument(self, filename):
		# init data members
		#print "Opening", filename
		self.SetPathName(filename) # Must set this early!
		try:
			f = open(filename, 'rb')
			try:
				self.text = f.read()
			finally:
				f.close()
		except IOError:
			win32ui.MessageBox("Could not load the file from %s" % filename)
			return 0

		self._SetLoadedText(self.text)
##		if self.GetFirstView():
##			self.GetFirstView()._SetLoadedText(self.text)
##		self.SetModifiedFlag(0) # No longer dirty
		return 1

	def SaveFile(self, fileName):
		view = self.GetFirstView()
		ok = view.SaveTextFile(fileName)
		if ok:
			view.SCISetSavePoint()
		return ok

	def ApplyFormattingStyles(self):
		self._ApplyOptionalToViews("ApplyFormattingStyles")

	# #####################
	# File related functions
	# Helper to transfer text from the MFC document to the control.
	def _SetLoadedText(self, text):
		view = self.GetFirstView()
		if view.IsWindow():
			# Turn off undo collection while loading 
			view.SendScintilla(SCI_SETUNDOCOLLECTION, 0, 0)
			# Make sure the control isnt read-only
			view.SetReadOnly(0)

			doc = self
			sm = text
			if sm:
				sma = array.array('c', sm)
				(a,l) = sma.buffer_info()
				view.SendScintilla(SCI_CLEARALL)
				view.SendScintilla(SCI_ADDTEXT, l, a)
				sma = None
			view.SendScintilla(SCI_SETUNDOCOLLECTION, 1, 0)
			view.SendScintilla(win32con.EM_EMPTYUNDOBUFFER, 0, 0)

	def FinalizeViewCreation(self, view):
		pass

	def HookViewNotifications(self, view):
		parent = view.GetParentFrame()
		parent.HookNotify(self.OnSavePointReached, SCN_SAVEPOINTREACHED)
		parent.HookNotify(self.OnSavePointLeft, SCN_SAVEPOINTLEFT)
		parent.HookNotify(self.OnModifyAttemptRO, SCN_MODIFYATTEMPTRO)
		parent.HookNotify(ViewNotifyDelegate(self, "OnBraceMatch"), SCN_CHECKBRACE)
		parent.HookNotify(ViewNotifyDelegate(self, "OnMarginClick"), SCN_MARGINCLICK)
		parent.HookNotify(ViewNotifyDelegate(self, "OnNeedShown"), SCN_NEEDSHOWN)

		# Tell scintilla what characters should abort auto-complete.
		view.SCIAutoCStops(string.whitespace+"()[]:;+-/*=\\?'!#@$%^&,<>\"'|" )

		if view == self.GetFirstView():
			pass
		else:
			view.SCISetDocPointer(self.GetFirstView().SCIGetDocPointer())


	def OnSavePointReached(self, std, extra):
		self.SetModifiedFlag(0)

	def OnSavePointLeft(self, std, extra):
		self.SetModifiedFlag(1)

	def OnModifyAttemptRO(self, std, extra):
		self.MakeDocumentWritable()

	# All Marker functions are 1 based.
	def MarkerAdd( self, lineNo, marker ):
		self.GetEditorView().SCIMarkerAdd(lineNo-1, marker)

	def MarkerCheck(self, lineNo, marker ):
		v = self.GetEditorView()
		lineNo = lineNo - 1 # Make 0 based
		markerState = v.SCIMarkerGet(lineNo)
		return markerState & (1<<marker) != 0

	def MarkerToggle( self, lineNo, marker ):
		v = self.GetEditorView()
		if self.MarkerCheck(lineNo, marker):
			v.SCIMarkerDelete(lineNo, marker)
		else:
			v.SCIMarkerAdd(lineNo, marker)
	def MarkerDelete( self, lineNo, marker ):
		self.GetEditorView().SCIMarkerDelete(lineNo-1, marker)
	def MarkerDeleteAll( self, marker ):
		self.GetEditorView().SCIMarkerDeleteAll(marker)
	def MarkerGetNext(self, lineNo, marker):
		return self.GetEditorView().SCIMarkerNext( lineNo-1, 1 << marker )+1
	def MarkerAtLine(self, lineNo, marker):
		markerState = self.GetEditorView().SCIMarkerGet(lineNo-1)
		return markerState & (1<<marker)

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
	def GetEditorView(self):
		# Find the first frame with a view,
		# then ask it to give the editor view
		# as it knows which one is "active"
		frame = self.GetFirstView().GetParentFrame()
		return frame.GetEditorView()

class ViewNotifyDelegate:
	def __init__(self, doc, name):
		self.doc = doc
		self.name = name
	def __call__(self, std, extra):
		(hwndFrom, idFrom, code) = std
		for v in self.doc.GetAllViews():
			if v.GetSafeHwnd() == hwndFrom:
				return apply(getattr(v, self.name), (std, extra))
