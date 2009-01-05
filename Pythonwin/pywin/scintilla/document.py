import win32ui
from pywin.mfc import docview
from pywin import default_scintilla_encoding
import scintillacon
import win32con
import string
import os
import codecs

crlf_bytes = "\r\n".encode("ascii")
lf_bytes = "\n".encode("ascii")

ParentScintillaDocument=docview.Document
class CScintillaDocument(ParentScintillaDocument):
	"A SyntEdit document. "
	def __init__(self, *args):
		self.bom = None # the BOM, if any, read from the file.
		# the encoding we detected from the source.  Might have
		# detected via the BOM or an encoding decl.
		self.source_encoding = None
		ParentScintillaDocument.__init__(self, *args)

	def DeleteContents(self):
		pass

	def OnOpenDocument(self, filename):
		# init data members
		#print "Opening", filename
		self.SetPathName(filename) # Must set this early!
		try:
			# load the text as binary we can get smart
			# about detecting any existing EOL conventions.
			f = open(filename, 'rb')
			try:
				self._LoadTextFromFile(f)
			finally:
				f.close()
		except IOError:
			win32ui.MessageBox("Could not load the file from %s" % filename)
			return 0

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
	def _LoadTextFromFile(self, f):
		# detect EOL mode - we don't support \r only - so find the
		# first '\n' and guess based on the char before.
		l = f.readline()
		# If line ends with \r\n or has no line ending, use CRLF.
		if l.endswith(crlf_bytes) or not l.endswith(lf_bytes):
			eol_mode = scintillacon.SC_EOL_CRLF
		else:
			eol_mode = scintillacon.SC_EOL_LF

		# Detect the encoding.
		# XXX - todo - support pep263 encoding declarations as well as
		# the BOM detection here (but note that unlike our BOM, the
		# encoding declaration could change between loading and saving
		# - particularly with a new file - so it also needs to be
		# implemented at save time.)
		for bom, encoding in (
			(codecs.BOM_UTF8, "utf8"),
			(codecs.BOM_UTF16_LE, "utf_16_le"),
			(codecs.BOM_UTF16_BE, "utf_16_be"),
			):
			if l.startswith(bom):
				self.bom = bom
				self.source_encoding = encoding
				l = l[len(bom):] # remove it.
				break

		# reading by lines would be too slow?  Maybe we can use the
		# incremental encoders? For now just stick with loading the
		# entire file in memory.
		text = l + f.read()

		# Translate from source encoding to UTF-8 bytes for Scintilla
		source_encoding = self.source_encoding
		# This latin1 sucks until we get pep263 support; if we don't
		# know an encoding we just write as binary (maybe we should
		# try ascii to let the 'decoding failed' handling below to
		# provide a nice warning that the file is non-ascii)
		if source_encoding is None:
			source_encoding = 'latin1'
		# we could optimize this by avoiding utf8 to-ing and from-ing,
		# but then we would lose the ability to handle invalid utf8
		# (and even then, the use of encoding aliases makes this tricky)
		# To create an invalid utf8 file:
		# >>> open(filename, "wb").write(codecs.BOM_UTF8+"bad \xa9har\r\n")
		try:
			dec = text.decode(source_encoding)
		except UnicodeError:
			print "WARNING: Failed to decode bytes from %r encoding - treating as latin1" % source_encoding
			dec = text.decode('latin1')
		# and put it back as utf8 - this shouldn't fail.
		text = dec.encode(default_scintilla_encoding)

		view = self.GetFirstView()
		if view.IsWindow():
			# Turn off undo collection while loading 
			view.SendScintilla(scintillacon.SCI_SETUNDOCOLLECTION, 0, 0)
			# Make sure the control isnt read-only
			view.SetReadOnly(0)
			view.SendScintilla(scintillacon.SCI_CLEARALL)
			view.SendMessage(scintillacon.SCI_ADDTEXT, text)
			view.SendScintilla(scintillacon.SCI_SETUNDOCOLLECTION, 1, 0)
			view.SendScintilla(win32con.EM_EMPTYUNDOBUFFER, 0, 0)
			# set EOL mode
			view.SendScintilla(scintillacon.SCI_SETEOLMODE, eol_mode)

	def _SaveTextToFile(self, view, f):
		s = view.GetTextRange() # already decoded from scintilla's encoding
		if self.bom:
			f.write(self.bom)
		source_encoding = self.source_encoding
		if source_encoding is None:
			source_encoding = 'latin1'

		f.write(s.encode(source_encoding))
		self.SetModifiedFlag(0)


	def FinalizeViewCreation(self, view):
		pass

	def HookViewNotifications(self, view):
		parent = view.GetParentFrame()
		parent.HookNotify(ViewNotifyDelegate(self, "OnBraceMatch"), scintillacon.SCN_CHECKBRACE)
		parent.HookNotify(ViewNotifyDelegate(self, "OnMarginClick"), scintillacon.SCN_MARGINCLICK)
		parent.HookNotify(ViewNotifyDelegate(self, "OnNeedShown"), scintillacon.SCN_NEEDSHOWN)

		parent.HookNotify(DocumentNotifyDelegate(self, "OnSavePointReached"), scintillacon.SCN_SAVEPOINTREACHED)
		parent.HookNotify(DocumentNotifyDelegate(self, "OnSavePointLeft"), scintillacon.SCN_SAVEPOINTLEFT)
		parent.HookNotify(DocumentNotifyDelegate(self, "OnModifyAttemptRO"), scintillacon.SCN_MODIFYATTEMPTRO)
		# Tell scintilla what characters should abort auto-complete.
		view.SCIAutoCStops(string.whitespace+"()[]:;+-/*=\\?'!#@$%^&,<>\"'|" )

		if view != self.GetFirstView():
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
			v.SCIMarkerDelete(lineNo-1, marker)
		else:
			v.SCIMarkerAdd(lineNo-1, marker)
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
			func(*args)
	def _ApplyOptionalToViews(self, funcName, *args):
		for view in self.GetAllViews():
			func = getattr(view, funcName, None)
			if func is not None:
				func(*args)
	def GetEditorView(self):
		# Find the first frame with a view,
		# then ask it to give the editor view
		# as it knows which one is "active"
		try:
			frame_gev = self.GetFirstView().GetParentFrame().GetEditorView
		except AttributeError:
			return self.GetFirstView()
		return frame_gev()

# Delegate to the correct view, based on the control that sent it.
class ViewNotifyDelegate:
	def __init__(self, doc, name):
		self.doc = doc
		self.name = name
	def __call__(self, std, extra):
		(hwndFrom, idFrom, code) = std
		for v in self.doc.GetAllViews():
			if v.GetSafeHwnd() == hwndFrom:
				return getattr(v, self.name)(*(std, extra))

# Delegate to the document, but only from a single view (as each view sends it seperately)
class DocumentNotifyDelegate:
	def __init__(self, doc, name):
		self.doc = doc
		self.delegate = getattr(doc, name)
	def __call__(self, std, extra):
		(hwndFrom, idFrom, code) = std
		if hwndFrom == self.doc.GetEditorView().GetSafeHwnd():
				self.delegate(*(std, extra))
