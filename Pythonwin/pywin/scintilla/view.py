# A general purpose MFC CCtrlView view that uses Scintilla.

import control
import IDLEenvironment # IDLE emulation.
from pywin.mfc import docview
from scintillacon import *
import win32con
import win32api
import win32ui
import afxres
import string
import array
import sys
import types
import __main__ # for attribute lookup
import bindings
import keycodes
import regex

wordbreaks = "._" + string.uppercase + string.lowercase + string.digits

patImport=regex.symcomp('import \(<name>.*\)')

_event_commands = [
	# File menu
	"win32ui.ID_FILE_LOCATE", "win32ui.ID_FILE_CHECK", "afxres.ID_FILE_CLOSE",
	"afxres.ID_FILE_NEW", "afxres.ID_FILE_OPEN", "afxres.ID_FILE_SAVE",
	"afxres.ID_FILE_SAVE_AS", "win32ui.ID_FILE_SAVE_ALL",
	# Edit menu
	"afxres.ID_EDIT_UNDO", "afxres.ID_EDIT_REDO", "afxres.ID_EDIT_CUT",
	"afxres.ID_EDIT_COPY", "afxres.ID_EDIT_PASTE", "afxres.ID_EDIT_SELECT_ALL",
	"afxres.ID_EDIT_FIND", "afxres.ID_EDIT_REPEAT", "afxres.ID_EDIT_REPLACE", 
	# View menu
	"win32ui.ID_VIEW_WHITESPACE", "win32ui.ID_VIEW_FIXED_FONT",
	"win32ui.ID_VIEW_BROWSE", "win32ui.ID_VIEW_INTERACTIVE",
	# Window menu
	"afxres.ID_WINDOW_ARRANGE", "afxres.ID_WINDOW_CASCADE",
	"afxres.ID_WINDOW_NEW", "afxres.ID_WINDOW_SPLIT",
	"afxres.ID_WINDOW_TILE_HORZ", "afxres.ID_WINDOW_TILE_VERT",
	# Others
	"afxres.ID_APP_EXIT", "afxres.ID_APP_ABOUT",
]

_extra_event_commands = [
	("EditDelete", afxres.ID_EDIT_CLEAR),
	("LocateModule", win32ui.ID_FILE_LOCATE),
	("GotoLine", win32ui.ID_EDIT_GOTO_LINE),
	("DbgBreakpointToggle", win32ui.IDC_DBG_ADD),
	("DbgGo", win32ui.IDC_DBG_GO),
	("DbgStepOver", win32ui.IDC_DBG_STEPOVER),
	("DbgStep", win32ui.IDC_DBG_STEP),
	("DbgStepOut", win32ui.IDC_DBG_STEPOUT),
	("DbgBreakpointClearAll", win32ui.IDC_DBG_CLEAR),
	("DbgClose", win32ui.IDC_DBG_CLOSE),
]

event_commands = []
def _CreateEvents():
	for name in _event_commands:
		val = eval(name)
		name_parts = string.split(name, "_")[1:]
		name_parts = map(string.capitalize, name_parts)
		event  =string.join(name_parts,'')
		event_commands.append(event, val)
	for name, id in _extra_event_commands:
		event_commands.append(name, id)

_CreateEvents()
del _event_commands; del _extra_event_commands

command_reflectors = [
	(win32ui.ID_EDIT_UNDO, win32con.WM_UNDO),
	(win32ui.ID_EDIT_REDO, SCI_REDO),
	(win32ui.ID_EDIT_CUT, win32con.WM_CUT),
	(win32ui.ID_EDIT_COPY, win32con.WM_COPY),
	(win32ui.ID_EDIT_PASTE, win32con.WM_PASTE),
	(win32ui.ID_EDIT_CLEAR, win32con.WM_CLEAR),
	(win32ui.ID_EDIT_SELECT_ALL, SCI_SELECTALL),
]

# Supposed to look like an MFC CEditView, but 
# also supports IDLE extensions and other source code generic features.
class CScintillaView(docview.CtrlView, control.CScintillaColorEditInterface):
	def __init__(self, doc):
		docview.CtrlView.__init__(self, doc, "Scintilla", win32con.WS_CHILD | win32con.WS_VSCROLL | win32con.WS_HSCROLL | win32con.WS_CLIPCHILDREN | win32con.WS_VISIBLE)
		self._tabWidth = 8 # Mirror of what we send to Scintilla - never change this directly
		self.bAutoCompleteAttributes = 1
		self.bShowCallTips = 1
		self.bindings = bindings.BindingsManager(self)

		self.idle = IDLEenvironment.IDLEEditorWindow(self)
		self.idle.IDLEExtension("AutoExpand")
	def SendScintilla(self, msg, w=0, l=0):
		return self._obj_.SendMessage(msg, w, l)

	def SCISetTabWidth(self, width):
		# I need to remember the tab-width for the AutoIndent extension.  This may go.
		self._tabWidth = width
		control.CScintillaEditInterface.SCISetTabWidth(self, width)

	def GetTabWidth(self):
		return self._tabWidth

	def HookHandlers(self):
		parent = self.GetParentFrame()

		# Create events for all the menu names.
		for name, val in event_commands:
#			handler = lambda id, code, tosend=val, parent=parent: parent.OnCommand(tosend, 0) and 0
			self.bindings.bind(name, None, cid=val)

		# Hook commands that do nothing other than send Scintilla messages.
		for command, reflection in command_reflectors:
			handler = lambda id, code, ss=self.SendScintilla, tosend=reflection: ss(tosend) and 0
			self.HookCommand(handler, command)

		parent.HookNotify(self.OnSavePointReached, SCN_SAVEPOINTREACHED)
		parent.HookNotify(self.OnSavePointLeft, SCN_SAVEPOINTLEFT)
		self.HookCommand(self.OnCmdViewWS, win32ui.ID_VIEW_WHITESPACE)
		self.HookCommandUpdate(self.OnUpdateViewWS, win32ui.ID_VIEW_WHITESPACE)
		self.HookCommand(self.OnCmdViewFixedFont, win32ui.ID_VIEW_FIXED_FONT)
		self.HookCommandUpdate(self.OnUpdateViewFixedFont, win32ui.ID_VIEW_FIXED_FONT)
		self.HookCommand(self.OnCmdFileLocate, win32ui.ID_FILE_LOCATE)
		self.HookCommand(self.OnCmdEditFind, win32ui.ID_EDIT_FIND)
		self.HookCommand(self.OnCmdEditRepeat, win32ui.ID_EDIT_REPEAT)
		self.HookCommand(self.OnCmdEditReplace, win32ui.ID_EDIT_REPLACE)
		self.HookCommand(self.OnCmdGotoLine, win32ui.ID_EDIT_GOTO_LINE)
		# Key bindings.
		self.HookMessage(self.OnKeyDown, win32con.WM_KEYDOWN)
		self.HookMessage(self.OnKeyDown, win32con.WM_SYSKEYDOWN)
		# Hook colorizer.
		self.HookStyleNotify()

	def OnInitialUpdate(self):
		self.SCISetSavePoint()
		self.SCISetUndoCollection(1)

		self.HookHandlers()

		# Tell scintilla what characters should abort auto-complete.
		self.SCIAutoCStops(string.whitespace+"()[]:;+-/*=\\?'!#@$%^&,<>\"'|" )

		# Load the configuration information.
		self.OnConfigChange()
		try:
			self._SetLoadedText(self.GetDocument().text)
		except AttributeError: # Not one of our docs - thats OK, but the text is their job!
			pass

		self.SetSel()

	def _GetSubConfigNames(self):
		return None # By default we use only sections without sub-sections.

	def OnConfigChange(self):
		self.bindings.prepare_configure()
		try:
			self.DoConfigChange()
			self.Reformat(1)
		finally:
			self.bindings.complete_configure()

	def DoConfigChange(self):
		# Bit of a hack I dont kow what to do about?
		from pywin.framework.editor import GetEditorOption
		self.bAutoCompleteAttributes = GetEditorOption("Autocomplete Attributes", 1)
		self.bShowCallTips = GetEditorOption("Show Call Tips", 1)
		# Update the key map and extension data.
		configManager.configure(self, self._GetSubConfigNames())
		if configManager.last_error:
			win32ui.MessageBox(configManager.last_error, "Configuration Error")

	def OnDestroy(self, msg):
		self.bindings.close()
		self.bindings = None
		self.idle.close()
		self.idle = None
		control.CScintillaColorEditInterface.close(self)
		return docview.CtrlView.OnDestroy(self, msg)

	# Helper to add an event to a menu.
	def AppendMenu(self, menu, text="", event=None, flags = None, checked=0):
		if event is None:
			assert flags is not None, "No event or custom flags!"
			cmdid = 0
		else:
			cmdid = self.bindings.get_command_id(event)
			if cmdid is None:
				# No event of that name - no point displaying it.
				print 'View.AppendMenu(): Unknown event "%s" specified for menu text "%s" - ignored' % (event, text)
				return 
			keyname = configManager.get_key_binding( event, self._GetSubConfigNames() )
			if keyname is not None:
				text = text + "\t" + keyname
		if flags is None: flags = win32con.MF_STRING|win32con.MF_ENABLED
		if checked: flags = flags | win32con.MF_CHECKED
		menu.AppendMenu(flags, cmdid, text)

	def OnKeyDown(self, msg):
		return self.bindings.fire_key_event( msg )

	def GotoEndOfFileEvent(self, event):
		self.SetSel(-1)

	def KeyDotEvent(self, event):
		self.SCIAddText(".")
		if self.bAutoCompleteAttributes:
			self._AutoComplete()

	# View Whitespace UI.
	def OnCmdViewWS(self, cmd, code): # Handle the menu command
		viewWS = self.SCIGetViewWS()
		self.SCISetViewWS(not viewWS)
	def OnUpdateViewWS(self, cmdui): # Update the tick on the UI.
		cmdui.SetCheck(self.SCIGetViewWS())
		cmdui.Enable()

	def OnCmdViewFixedFont(self, cmd, code): # Handle the menu command
		self._GetColorizer().bUseFixed = not self._GetColorizer().bUseFixed
		self.Reformat(0)
	def OnUpdateViewFixedFont(self, cmdui): # Update the tick on the UI.
		c = self._GetColorizer()
		if c is not None: cmdui.SetCheck(c.bUseFixed)
		cmdui.Enable(c is not None)

	def OnCmdEditFind(self, cmd, code):
		import find
		find.ShowFindDialog()
	def OnCmdEditRepeat(self, cmd, code):
		import find
		find.FindNext()
	def OnCmdEditReplace(self, cmd, code):
		import find
		find.ShowReplaceDialog()

	def OnCmdFileLocate(self, cmd, id):
		line=string.strip(self.GetLine())
		import pywin.framework.scriptutils
		if patImport.match(line)==len(line):
			# Module name on this line - locate that!
			modName = patImport.group('name')
			fileName = pywin.framework.scriptutils.LocatePythonFile(modName)
			if fileName is None:
				win32ui.SetStatusText("Can't locate module %s" % modName)
				return 1 # Let the default get it.
			else:
				win32ui.GetApp().OpenDocumentFile(fileName)
		else:
			# Just to a "normal" locate - let the default handler get it.
			return 1
		return 0

	def OnCmdGotoLine(self, cmd, id):
		try:
			lineNo = string.atoi(raw_input("Enter Line Number"))
		except (ValueError, KeyboardInterrupt):
			return 0
		self.SCIGotoLine(lineNo-1)
		return 0

	# #####################
	# File related functions
	# Helper to transfer text from the MFC document to the control.
	def OnSavePointReached(self, std, extra):
		self.GetDocument().SetModifiedFlag(0)

	def OnSavePointLeft(self, std, extra):
		self.GetDocument().SetModifiedFlag(1)

	def _SetLoadedText(self, text):
		if self.IsWindow():
			# Turn off undo collection while loading 
			self.SendScintilla(SCI_SETUNDOCOLLECTION, 0, 0)
			# Make sure the control isnt read-only
			self.SetReadOnly(0)

			doc = self.GetDocument()
			sm = text
			if sm:
				sma = array.array('c', sm)
				(a,l) = sma.buffer_info()
				self.SendScintilla(SCI_CLEARALL)
				self.SendScintilla(SCI_ADDTEXT, l, a)
				sma = None
			self.SendScintilla(SCI_SETUNDOCOLLECTION, 1, 0)
			self.SendScintilla(win32con.EM_EMPTYUNDOBUFFER, 0, 0)

	def SaveTextFile(self, filename):
		doc = self.GetDocument()
		s = self.GetTextRange()
		f  = open(filename, 'wb')
		f.write(s)
		f.close()
		doc.SetModifiedFlag(0)
		return 1

	def _AutoComplete(self):
		ob = self._GetObjectAtPos()
		self.SCICancel() # Cancel tooltips and old auto-complete lists.
		if ob is not None:
			items = []
			try:
				items = items + dir(ob)
			except AttributeError:
				pass # object has no __dict__
			try:
				items = items + dir(ob.__class__)
			except AttributeError:
				pass
			# Reduce __special_names__
			items = filter(lambda word: word[:2]!='__' or word[-2:]!='__', items)
			if items:
				self.SCIAutoCShow(items)

	def _GetObjectAtPos(self, pos=-1):
		left, right = self._GetWordSplit()
		if left: # It is an attribute lookup
			# How is this for a hack!
			namespace = sys.modules.copy()
			namespace.update(__main__.__dict__)
			try:
				return eval(left, namespace)
			except:
				pass
		return None

	def _GetWordSplit(self, pos=-1):
		if pos==-1: pos = self.GetSel()[0]-1 # Character before current one
		limit = self.GetTextLength()
		before = []
		after = []
		index = pos-1
		while index>=0:
			char = self.SCIGetCharAt(index)
			if char not in wordbreaks: break
			before.insert(0, char)
			index = index-1
		index = pos
		while index<=limit:
			char = self.SCIGetCharAt(index)
			if char not in wordbreaks: break
			after.append(char)
			index=index+1
		return string.join(before,''), string.join(after,'')

def LoadConfiguration():
	global configManager
	# Bit of a hack I dont kow what to do about?
	from config import ConfigManager
	configName = rc = win32ui.GetProfileVal("Editor", "Keyboard Config", "default")
	configManager = ConfigManager(configName)
	if configManager.last_error:
		bTryDefault = 0
		msg = "Error loading configuration '%s'\n\n%s" % (configName, configManager.last_error)
		if configName != "default":
			msg = msg + "\n\nThe default configuration will be loaded."
			bTryDefault = 1
		win32ui.MessageBox(msg)
		if bTryDefault:
			configManager = ConfigManager("default")
			if configManager.last_error:
				win32ui.MessageBox("Error loading configuration 'default'\n\n%s" % (configManager.last_error))

configManager = None
LoadConfiguration()
