# Color Editor originally by Neil Hodgson, but restructured by mh to integrate
# even tighter into Pythonwin.
import win32ui
import win32con
import win32api
import afxres
import regex
import regsub
import string
import sys
import array
import struct
import traceback

import pywin.scintilla.keycodes
from pywin.scintilla import bindings

from pywin.framework.editor import GetEditorOption, SetEditorOption, GetEditorFontOption, SetEditorFontOption, defaultCharacterFormat
#from pywin.framework.editor import EditorPropertyPage

from pywin.mfc import docview, window, dialog, afxres

# Define a few common markers
MARKER_BOOKMARK = 0
MARKER_BREAKPOINT = 1
MARKER_CURRENT = 2

# XXX - copied from debugger\dbgcon.py
DBGSTATE_NOT_DEBUGGING = 0
DBGSTATE_RUNNING = 1
DBGSTATE_BREAK = 2

from pywin.scintilla.document import CScintillaDocument
from pywin.framework.editor.document import EditorDocumentBase
from pywin.scintilla.scintillacon import * # For the marker definitions
import pywin.scintilla.view

class SyntEditDocument(EditorDocumentBase, CScintillaDocument):
	"A SyntEdit document. "
	def OnOpenDocument(self, filename):
		rc = CScintillaDocument.OnOpenDocument(self, filename)
		self._DocumentStateChanged()
		return rc
	def ReloadDocument(self):
		EditorDocumentBase.ReloadDocument(self)
		self._ApplyOptionalToViews("SCISetSavePoint")
	# All Marker functions are 1 based.
	def MarkerAdd( self, lineNo, marker ):
		self._ApplyOptionalToViews("MarkerAdd", lineNo, marker)
	def MarkerToggle( self, lineNo, marker ):
		self._ApplyOptionalToViews("MarkerToggle", lineNo, marker)
	def MarkerDelete( self, lineNo, marker ):
		self._ApplyOptionalToViews("MarkerDelete", lineNo, marker)
	def MarkerDeleteAll( self, marker ):
		self._ApplyOptionalToViews("MarkerDeleteAll", marker)
	def MarkerGetNext(self, lineNo, marker):
		return self.GetFirstView().MarkerGetNext(lineNo, marker)
	def MarkerAtLine(self, lineNo, marker):
		return self.GetFirstView().MarkerAtLine(lineNo, marker)
	def OnDebuggerStateChange(self, state):
		self._ApplyOptionalToViews("OnDebuggerStateChange", state)

SyntEditViewParent=pywin.scintilla.view.CScintillaView
class SyntEditView(SyntEditViewParent):
	"A view of a SyntEdit.  Obtains data from document."
	def __init__(self, doc):
		SyntEditViewParent.__init__(self, doc)
		self.bCheckingFile = 0

	def OnInitialUpdate(self):
		SyntEditViewParent.OnInitialUpdate(self)
		# set up styles
		self.HookMessage(self.OnRClick,win32con.WM_RBUTTONDOWN)

		# Define the markers
		self.SCIMarkerDefine(MARKER_BOOKMARK, SC_MARK_ROUNDRECT)
		self.SCIMarkerSetBack(MARKER_BOOKMARK, win32api.RGB(0, 0xff, 0xff))
		self.SCIMarkerSetFore(MARKER_BOOKMARK, win32api.RGB(0x0, 0x0, 0x0))

		self.SCIMarkerDefine(MARKER_CURRENT, SC_MARK_ARROW)
		self.SCIMarkerSetBack(MARKER_CURRENT, win32api.RGB(0, 0x7f, 0x7f))

		self._UpdateUIForState()
		self.SCIMarkerDefine(MARKER_BREAKPOINT, SC_MARK_CIRCLE)
		# Marker background depends on debugger state
		self.SCIMarkerSetFore(MARKER_BREAKPOINT, win32api.RGB(0x0, 0, 0))
		# Get the current debugger state.
		try:
			import pywin.debugger
			if pywin.debugger.currentDebugger is None:
				state = DBGSTATE_NOT_DEBUGGING
			else:
				state = pywin.debugger.currentDebugger.debuggerState
		except ImportError:
			state = DBGSTATE_NOT_DEBUGGING
		self.OnDebuggerStateChange(state)
		self.GetDocument().GetDocTemplate().CheckIDLEMenus(self.idle)

	def _GetSubConfigNames(self):
		return ["editor"] # Allow [Keys:Editor] sections to be specific to us

	def DoConfigChange(self):
		SyntEditViewParent.DoConfigChange(self)
		tabSize = GetEditorOption("Tab Size", 4, 2)
		indentSize = GetEditorOption("Indent Size", 4, 2)
		bUseTabs = GetEditorOption("Use Tabs", 0)
		bSmartTabs = GetEditorOption("Smart Tabs", 1)
		ext = self.idle.IDLEExtension("AutoIndent") # Required extension.

		# Auto-indent has very complicated behaviour.  In a nutshell, the only
		# way to get sensible behaviour from it is to ensure tabwidth != indentsize.
		# Further, usetabs will only ever go from 1->0, never 0->1.
		# This is _not_ the behaviour Pythonwin wants:
		# * Tab width is arbitary, so should have no impact on smarts.
		# * bUseTabs setting should reflect how new files are created, and
		#   if Smart Tabs disabled, existing files are edited
		# * If "Smart Tabs" is enabled, bUseTabs should have no bearing
		#   for existing files (unless of course no context can be determined)
		#
		# So for smart tabs we configure the widget with completely dummy
		# values (ensuring tabwidth != indentwidth), ask it to guess, then
		# look at the values it has guessed, and re-configure
		if bSmartTabs:
			ext.config(usetabs=1, tabwidth=5, indentwidth=4)
			ext.set_indentation_params(1)
			if ext.indentwidth==5:
				# Either 5 literal spaces, or a single tab character. Assume a tab
				usetabs = 1
				indentwidth = tabSize
			else:
				# Either Indented with spaces, and indent size has been guessed or
				# an empty file (or no context found - tough!)
				if self.GetTextLength()==0: # emtpy
					usetabs = bUseTabs
					indentwidth = indentSize
				else: # guessed.
					indentwidth = ext.indentwidth
					usetabs = 0
			# Tab size can never be guessed - set at user preference.
			ext.config(usetabs=usetabs, indentwidth=indentwidth, tabwidth=tabSize)
		else:
			# Dont want smart-tabs - just set the options!
			ext.config(usetabs=bUseTabs, tabwidth=tabSize, indentwidth=indentSize)
		self.SCISetTabWidth(tabSize)

		self.SCISetViewWS( GetEditorOption("View Whitespace", 0) )

	def OnDebuggerStateChange(self, state):
		if state == DBGSTATE_NOT_DEBUGGING:
			# Indicate breakpoints arent really usable.
			self.SCIMarkerSetBack(MARKER_BREAKPOINT, win32api.RGB(0xff, 0xff, 0xff))
		else:
			self.SCIMarkerSetBack(MARKER_BREAKPOINT, win32api.RGB(0x80, 0, 0))

	def HookHandlers(self):
		SyntEditViewParent.HookHandlers(self)

		# Idle time document reloaded handlers.
		self.HookMessage(self.OnKillFocus, win32con.WM_KILLFOCUS)
		self.HookMessage(self.OnSetFocus, win32con.WM_SETFOCUS)
		self.GetParentFrame().HookNotify(self.OnModifyAttemptRO, SCN_MODIFYATTEMPTRO)

	def _PrepareUserStateChange(self):
		return self.GetSel(), self.GetFirstVisibleLine()
	def _EndUserStateChange(self, info):
		scrollOff = info[1] - self.GetFirstVisibleLine()
		if scrollOff:
			self.LineScroll(scrollOff)
		# Make sure we dont reset the cursor beyond the buffer.
		max = self.GetTextLength()
		newPos = min(info[0][0], max), min(info[0][1], max)
		self.SetSel(newPos)

	def _UpdateUIForState(self):
		self.SetReadOnly(self.GetDocument()._IsReadOnly())

	def MarkerAdd( self, lineNo, marker ):
		self.SCIMarkerAdd(lineNo-1, marker)

	def MarkerAtLine(self, lineNo, marker):
		markerState = self.SCIMarkerGet(lineNo-1)
		return markerState & (1<<marker)

	def MarkerToggle(self, lineNo, marker):
		lineNo = lineNo - 1 # Make 0 based
		markerState = self.SCIMarkerGet(lineNo)
		if markerState & (1<<marker):
			self.SCIMarkerDelete(lineNo, marker)
		else:
			self.SCIMarkerAdd(lineNo, marker)

	def MarkerDelete(self, lineNo, marker ):
		self.SCIMarkerDelete(lineNo-1, marker)

	def MarkerDeleteAll( self, marker ):
		self.SCIMarkerDeleteAll(marker)

	def MarkerGetNext(self, lineNo, marker ):
		return self.SCIMarkerNext( lineNo-1, 1 << marker )+1
	#######################################
	# The Windows Message or Notify handlers.
	#######################################
	def OnDestroy(self, msg):
		self._DeleteReloadIdleHandler()
		return SyntEditViewParent.OnDestroy(self, msg)
 
	def OnModifyAttemptRO(self, std, extra):
		self.GetDocument().MakeDocumentWritable()

	def OnKillFocus(self,msg):
		self._DeleteReloadIdleHandler()

	def OnSetFocus(self,msg):
		self.CheckExternalDocumentUpdated(self.CheckExternalDocumentUpdated,0)
		self._AddReloadIdleHandler()
		return 1

	def OnRClick(self,params):
		menu = win32ui.CreatePopupMenu()
		self.AppendMenu(menu, "&Locate module", "LocateModule")
		self.AppendMenu(menu, flags=win32con.MF_SEPARATOR)
		self.AppendMenu(menu, "&Undo", "EditUndo")
		self.AppendMenu(menu, '&Redo', 'EditRedo')
		self.AppendMenu(menu, flags=win32con.MF_SEPARATOR)
		self.AppendMenu(menu, 'Cu&t', 'EditCut')
		self.AppendMenu(menu, '&Copy', 'EditCopy')
		self.AppendMenu(menu, '&Paste', 'EditPaste')
		self.AppendMenu(menu, flags=win32con.MF_SEPARATOR)
		self.AppendMenu(menu, '&Select all', 'EditSelectAll')
		self.AppendMenu(menu, 'View &Whitespace', 'ViewWhitespace', checked=self.SCIGetViewWS())
		self.AppendMenu(menu, "&Fixed Font", "ViewFixedFont", checked = self._GetColorizer().bUseFixed)
		self.AppendMenu(menu, flags=win32con.MF_SEPARATOR)
		self.AppendMenu(menu, "&Goto line...", "GotoLine")

		submenu = win32ui.CreatePopupMenu()
		newitems = self.idle.GetMenuItems("edit")
		for text, event in newitems:
			self.AppendMenu(submenu, text, event)

		flags=win32con.MF_STRING|win32con.MF_ENABLED|win32con.MF_POPUP
		menu.AppendMenu(flags, submenu.GetHandle(), "&Source code")

		flags = win32con.TPM_LEFTALIGN|win32con.TPM_LEFTBUTTON|win32con.TPM_RIGHTBUTTON
		menu.TrackPopupMenu(params[5], flags, self)
		return 0
	#######################################
	# The Events
	#######################################
	def _DoMarkerToggle(self, marker, pos = -1):
		if pos==-1:
			pos, end = self.GetSel()
		startLine = self.LineFromChar(pos)
		self.GetDocument().MarkerToggle(startLine+1, marker)

	def ToggleBookmarkEvent(self, event, pos = -1):
		"""Toggle a bookmark at the specified or current position
		"""
		self._DoMarkerToggle(MARKER_BOOKMARK)
		return 0

	def GotoNextBookmarkEvent(self, event, fromPos=-1):
		""" Move to the next bookmark
		"""
		if fromPos==-1:
			fromPos, end = self.GetSel()
		startLine = self.LineFromChar(fromPos)+1 # Zero based line to start
		nextLine = self.MarkerGetNext(startLine+1, MARKER_BOOKMARK)-1
		if nextLine<0:
			nextLine = self.MarkerGetNext(0, MARKER_BOOKMARK)-1
		if nextLine <0 or nextLine == startLine-1:
			win32api.MessageBeep()
		else:
			self.SCIGotoLine(nextLine)
		return 0

	def TabKeyEvent(self, event):
		"""Insert an indent.  If no selection, a single indent, otherwise a block indent
		"""
		# Handle auto-complete first.
		if self.SCIAutoCActive():
			self.SCIAutoCComplete()
			return 0
		# Call the IDLE event.
		return self.bindings.fire("<<smart-indent>>", event)

	def ShowInteractiveWindowEvent(self, event):
		import pywin.framework.interact
		pywin.framework.interact.ShowInteractiveWindow()

	#
	# Support for checking the external file to see if it is changed.
	# We set up an idle time handler only when the view has focus.
	# This handler continually checks the file behind this document, but
	# as it is in idle time, no one notices :-)
	#
	def _AddReloadIdleHandler(self):
		win32ui.GetApp().AddIdleHandler(self.CheckExternalDocumentUpdated)

	def _DeleteReloadIdleHandler(self):
		if win32ui.GetApp().HaveIdleHandler(self.CheckExternalDocumentUpdated):
			win32ui.GetApp().DeleteIdleHandler(self.CheckExternalDocumentUpdated)

	def CheckExternalDocumentUpdated(self, handler, count):
		if self.bCheckingFile: return
		self.bCheckingFile = 1
		try:
			self.GetDocument().CheckExternalDocumentUpdated()
		except:
			traceback.print_exc()
			print "The idle handler checking for changes to the file on disk failed!"
			self._DeleteReloadIdleHandler()
		self.bCheckingFile = 0
		return 0 # No more idle handling required.

from pywin.framework.editor.template import EditorTemplateBase
class SyntEditTemplate(EditorTemplateBase):
	def __init__(self, res=win32ui.IDR_TEXTTYPE, makeDoc=None, makeFrame=None, makeView=None):
		if makeDoc is None: makeDoc = SyntEditDocument
		if makeView is None: makeView = SyntEditView
		self.bSetMenus = 0
		EditorTemplateBase.__init__(self, res, makeDoc, makeFrame, makeView)

	def CheckIDLEMenus(self, idle):
		if self.bSetMenus: return
		self.bSetMenus = 1

		submenu = win32ui.CreatePopupMenu()
		newitems = idle.GetMenuItems("edit")
		flags=win32con.MF_STRING|win32con.MF_ENABLED
		for text, event in newitems:
			id = bindings.event_to_commands.get(event)
			if id is not None:
				keyname = pywin.scintilla.view.configManager.get_key_binding( event, ["editor"] )
				if keyname is not None:
					text = text + "\t" + keyname
				submenu.AppendMenu(flags, id, text)

		mainMenu = self.GetSharedMenu()
		editMenu = mainMenu.GetSubMenu(1)
		editMenu.AppendMenu(win32con.MF_SEPARATOR, 0, "")
		editMenu.AppendMenu(win32con.MF_STRING | win32con.MF_POPUP | win32con.MF_ENABLED, submenu.GetHandle(), "&Source Code")

	def _CreateDocTemplate(self, resourceId):
		return win32ui.CreateDocTemplate(resourceId)

	def CreateWin32uiDocument(self):
		return self.DoCreateDoc()

	def GetPythonPropertyPages(self):
		"""Returns a list of property pages
		"""
		from pywin.scintilla import configui
		return EditorTemplateBase.GetPythonPropertyPages(self) + [configui.ScintillaFormatPropertyPage()]
		
# For debugging purposes, when this module may be reloaded many times.
try:
	win32ui.GetApp().RemoveDocTemplate(editorTemplate)
except NameError:
	pass

editorTemplate = SyntEditTemplate()
win32ui.GetApp().AddDocTemplate(editorTemplate)
