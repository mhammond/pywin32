import sys

# Some cruft to deal with the Pythonwin GUI booting up from a non GUI app.
def _MakeDebuggerGUI():
	app.InitInstance()

isInprocApp = -1
def _CheckNeedGUI():
	global isInprocApp
	if isInprocApp==-1:
		import win32ui
		isInprocApp = win32ui.GetApp().IsInproc()
	if isInprocApp:
		# MAY Need it - may already have one
		need = sys.modules.has_key("pywin.debugger.dbgpyapp")==0
	else:
		need = 0
	if need:
		import pywin.framework.app
		import dbgpyapp
		pywin.framework.app.CreateDefaultGUI(dbgpyapp.DebuggerPythonApp)

	else:
		# Check we have the appropriate editor.
		import pywin.framework.editor
		try:
			import pywin.framework.editor.color.coloreditor
			ok = pywin.framework.editor.editorTemplate==pywin.framework.editor.color.coloreditor.editorTemplate
		except ImportError:
			ok = 0
		if not ok:
			msg = "This debugger requires the Pythonwin color editor.\r\nDebugging can not continue.\r\n\r\nWould you like to make the color editor the default?"
			rc = win32ui.MessageBox(msg, "Can't initialize debugger", win32con.MB_YESNO)
			if rc == win32con.IDYES:
				pywin.framework.editor.WriteDefaultEditorModule("pywin.framework.editor.color.coloreditor")
				win32ui.MessageBox("The debugger will be available when you restart the application.")
			raise RuntimeError, "Can't initialize debugger, as the required editor is not the default"
	return need

# Inject some methods in the top level name-space.
currentDebugger = None # Wipe out any old one on reload.

def _GetCurrentDebugger():
	global currentDebugger
	if currentDebugger is None:
		_CheckNeedGUI()
		import debugger
		currentDebugger = debugger.Debugger()
	return currentDebugger

def GetDebugger():
	# An error here is not nice - as we are probably trying to
	# break into the debugger on a Python error, any
	# error raised by this is usually silent, and causes
	# big problems later!
	try:
		rc = _GetCurrentDebugger()
		rc.GUICheckInit()
		return rc
	except:
		print "Could not create the debugger!"
		import traceback
		traceback.print_exc()
		return None

def close():
	if currentDebugger is not None:
		currentDebugger.close()

def run(cmd,globals=None, locals=None, start_stepping = 1):
	_GetCurrentDebugger().run(cmd, globals,locals, start_stepping)

def runeval(expression, globals=None, locals=None):
	return _GetCurrentDebugger().runeval(expression, globals, locals)

def runcall(*args):
	return apply(_GetCurrentDebugger().runcall, args)

def set_trace():
	import sys
	d = _GetCurrentDebugger()

	if d.frameShutdown: return # App closing

	if d.stopframe != d.botframe:
		# If im not "running"
		return

	sys.settrace(None) # May be hooked
	d.reset()
	d.set_trace()

# "brk" is an alias for "set_trace" ("break" is a reserved word :-(
brk = set_trace

# Post-Mortem interface

def post_mortem(t=None):
	if t is None:
		t = sys.exc_info()[2] # Will be valid if we are called from an except handler.
	if t is None:
		try:
			t = sys.last_traceback
		except AttributeError:
			print "No traceback can be found from which to perform post-mortem debugging!"
			print "No debugging can continue"
			return
	p = _GetCurrentDebugger()
	if p.frameShutdown: return # App closing
	# No idea why I need to settrace to None - it should have been reset by now?
	sys.settrace(None)
	if p.stopframe != p.botframe:
		# If im "running"
		print "Can not perform post-mortem debugging while the debugger is active."
		return
	p.reset()
	while t.tb_next <> None: t = t.tb_next
	p.bAtPostMortem = 1
	p.prep_run(None)
	try:
		p.interaction(t.tb_frame, t)
	finally:
		p.bAtPostMortem = 0
		p.done_run()

def pm(t=None):
	post_mortem(t)
