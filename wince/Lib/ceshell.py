# A "Python console" for Windows CE.
#
# Also works on NT/9x (with a few limitations!) - useful for debugging!
#
# Used 2 threads - one for UI (ie, the message loop) and another thread
# for executing Python code.  Uses very simple events to synchronise the 2!

from win32gui import *
from win32event import *
import sys
import string
import thread, threading
import traceback
import code # std module for compilation utilities.
import new
import os
import imp

IDOK=1
IDCANCEL=2

GWL_WNDPROC=-4
FIXED_PITCH=1
ANSI_FIXED_FONT=11

IDC_WAIT = 32514
HWND_TOP=0
CS_VREDRAW=1
CS_HREDRAW=2

CW_USEDEFAULT=0x80000000

WM_CHAR=258
WM_COMMAND=273
WM_DESTROY=2
WM_QUIT=18
WM_SETFOCUS=7
WM_SETFONT=48
WM_SETREDRAW=11
WM_SIZE=5
WM_USER=1024

WHITE_BRUSH=0
SW_SHOW=5
SW_SHOWNORMAL=1

WS_SYSMENU=524288
WS_CLIPCHILDREN=33554432
WS_CHILD=1073741824
WS_VISIBLE=268435456
WS_HSCROLL=1048576
WS_VSCROLL=2097152
if sys.platform=="wince":
	WS_OVERLAPPEDWINDOW=0
else:
	WS_OVERLAPPEDWINDOW=13565952

EM_GETLINECOUNT=186
EM_GETSEL=176
EM_LINEINDEX=187
EM_LINEFROMCHAR=201
EM_LINELENGTH=193
EM_SETSEL=177
EM_REPLACESEL=194

ES_LEFT=0
ES_MULTILINE=4
ES_WANTRETURN=4096
ES_AUTOVSCROLL=64
ES_AUTOHSCROLL=128

IDR_MENU=101
IDM_EXIT=40001
IDM_ABOUT=40002
IDD_ABOUT=40002

if UNICODE:
	TEXT = Unicode
else:
	TEXT = lambda x: x

if sys.platform=="wince":
	OutputDebugString = NKDbgPrintfW
else:
	from win32api import OutputDebugString
	
try:
	sys.ps1
except AttributeError:
	sys.ps1 = ">>> "
	sys.ps2 = "... "

class SimpleShell:
	editMessageMap = {}
	def __init__(self):
		self.bInteract = 0 # Am I interacting?
		self.hwnd = None
		self.hwndEdit = None
		self.outputQueue = []
		self.outputQueueLock = threading.Lock()

		# Allocate some events for thread sync
		self.currentBlockItems = None
		self.eventInteractiveInputAvailable = CreateEvent(None, 0, 0, None)
		self.eventClosed = CreateEvent(None, 0, 0, None)

	def __del__(self):
		print "InteractiveManager dieing"

	def write(self, text):
		text = string.replace(text, "\n", "\r\n")
		self.outputQueueLock.acquire()
		self.outputQueue.append(text)
		self.outputQueueLock.release()
		try:
			PostMessage(self.hwnd, WM_USER, 0, 0)
		except:
			pass
		
	def Run(self):
		PumpMessages()	

	def GetEditMessageMap(self):
		return {WM_CHAR : self.OnEditChar}

	def GetParentMessageMap(self):
		map={}
		map[WM_DESTROY] = self.OnParentDestroy
		map[WM_SIZE] = self.OnParentSize
		map[WM_SETFOCUS] = self.OnParentSetFocus
		map[WM_USER] = self.OnParentUser
		map[WM_COMMAND] = self.OnParentCommand
		return map
		
	def Init(self):
		try:
			self.hinst = GetModuleHandle(None)
		except NameError: # Not on CE??
			self.hinst = sys.hinst # But this is :-)

		InitCommonControls()

		wc = WNDCLASS()
		wc.hInstance = self.hinst
		wc.style=CS_HREDRAW | CS_VREDRAW
		wc.hbrBackground = GetStockObject(WHITE_BRUSH)
		wc.lpszClassName = TEXT("PYTHON_CE")
		# This code passes a dictionary as the "wndproc", rather than a function.
		wc.lpfnWndProc = self.GetParentMessageMap() #self.MainWndProc
		self.classAtom = RegisterClass(wc)
		
		if sys.platform=="wince":
			style = WS_CLIPCHILDREN
		else:
			style = WS_OVERLAPPEDWINDOW
			
		self.hwnd = CreateWindow( self.classAtom, "Python CE", style, \
	                      0, 0, CW_USEDEFAULT, CW_USEDEFAULT, \
	                      0, 0, self.hinst, None)

		left, top, right, bottom = GetClientRect(self.hwnd)

#		print sys.platform, type(sys.platform)
		if sys.platform=="wince":
			self.hCmdBar = CommandBar_Create(self.hinst, self.hwnd, 1)
			CommandBar_InsertMenubar(self.hCmdBar, self.hinst, IDR_MENU, 0)
			CommandBar_AddAdornments(self.hCmdBar, 0, 0)
			top = CommandBar_Height(self.hCmdBar)

		style = WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|ES_LEFT|ES_MULTILINE|ES_WANTRETURN|ES_AUTOHSCROLL
		self.hwndEdit=CreateWindow("EDIT", None, style, \
				      left, top, (right-left), (bottom-top), \
		                      self.hwnd, 0, self.hinst, None)    
	
		self.oldEditWndProc = SetWindowLong(self.hwndEdit, GWL_WNDPROC, self.GetEditMessageMap())# self.EditWndProc)

		if sys.platform != "wince":
			SendMessage(self.hwndEdit, WM_SETFONT, GetStockObject(ANSI_FIXED_FONT), 0)

		ShowWindow(self.hwnd, SW_SHOW)
		UpdateWindow(self.hwnd)

		EnableWindow(self.hwndEdit, 1)
	
		SetFocus(self.hwndEdit)
		SetCursor(LoadCursor(0,0))

	def Term(self):
		UnregisterClass(self.classAtom, self.hinst)
	
	def OnEditChar(self,hWnd, msg, wparam, lparam):
		if self.bInteract and wparam==0x0D: # return key
			HideCaret(hWnd);
			cChar=SendMessage(hWnd, EM_LINEINDEX, -1)
			cLine=SendMessage(hWnd, EM_LINEFROMCHAR, cChar)
			# Find the start of the block
			numLines = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0)
			# GetLine fails as the size is wrong??
			maxLineSize=512
			blockStart = -1
			while cLine >= 0:
				line = str(Edit_GetLine(hWnd, cLine, maxLineSize))
				if line[:4]==sys.ps1:
					blockStart = cLine
					break
				elif line[:4]!=sys.ps2:
					break
				cLine = cLine -1

			if blockStart>=0:
				# Find the end of the block.
				while 1:
					cLine = cLine + 1
					line = str(Edit_GetLine(hWnd, cLine, maxLineSize))
					if line is None or line[:4]!=sys.ps2:
						break
				blockEnd = cLine
				# blockStart is the first line
				# blockEnd is one past the block end.
				firstLine=str(Edit_GetLine(hWnd, blockStart, maxLineSize))[len(sys.ps1):]
				# Special case for an empty command - mimic Python better by writing another ">>>"
				if len(firstLine)==0 and blockStart+1==blockEnd:
					# Empty prompt - write a new one.
					self.write("\n"+sys.ps1)
				else:
					items = [firstLine]
					for cLine in range(blockStart+1, blockEnd):
						items.append( str(Edit_GetLine(hWnd, cLine, maxLineSize))[len(sys.ps2):] )

					# If the block is not at the end of the control, copy it there...
					if blockEnd != numLines:
						self.write("\n%s%s" % (sys.ps1, items[0]))
						for item in items[1:]:
							self.write("\n%s%s" % (sys.ps2, items[0]))
					else:
						# Ready to execute.
						self.currentBlockItems = items
						SetEvent(self.eventInteractiveInputAvailable)

			else:
				# Not in a block - write a new prompt.
				self.write("\n"+sys.ps1)

			ShowCaret(hWnd)
			return 0

		return CallWindowProc(self.oldEditWndProc, hWnd, msg, wparam, lparam)

	def OnParentSize(self, hwnd, msg, wparam, lparam):
		left, top, right, bottom = GetClientRect(hwnd)
		try:
			top=CommandBar_Height(self.hCmdBar);
		except NameError: # Only on CE
			pass
		if self.hwndEdit is not None:
			SetWindowPos(self.hwndEdit, HWND_TOP, left, top, right-left, bottom-top, 0)
			ShowWindow(self.hwndEdit, SW_SHOWNORMAL)

	def OnParentDestroy(self, hwnd, msg, wparam, lparam):
		PostQuitMessage(hwnd)
		# And tell the thread waiting for us we are done!
		SetEvent(self.eventClosed)

	def OnParentSetFocus(self, hwnd, msg, wparam, lparam):
		if self.hwndEdit is not None:
			SetFocus(self.hwndEdit)
	
	def OnParentUser(self, hwnd, msg, wparam, lparam):
		# Out write function post this message.
		# We dequeue the output, and write the text.
		while self.outputQueue:
			self.outputQueueLock.acquire()
			text = string.join(self.outputQueue, '')
			self.outputQueue = []
			self.outputQueueLock.release()
			SendMessage(self.hwndEdit, EM_SETSEL, -2, -2)
			# Now check that we wont fill the control.
			# If so, remove the first lines until we are OK.
			selInfo = SendMessage(self.hwndEdit, EM_GETSEL, 0, 0)
			endPos = HIWORD(selInfo)
			lineLookIndex = 0
			lineLookLength = 0
			while endPos + len(text) - lineLookLength > 29000:
				lineLookIndex = lineLookIndex + 1
				lineLookLength = SendMessage(self.hwndEdit, EM_LINEINDEX, lineLookIndex, 0)
			if lineLookIndex > 0:
				# The SETREDRAW has no effect on CE.  If we really want this
				# I think we must respond to WM_PAINT, and ignore it for the duration
				# the redraw is turned off.
				SendMessage(self.hwndEdit, WM_SETREDRAW, 0, 0)
				SendMessage(self.hwndEdit, EM_SETSEL, 0, lineLookLength)
				SendMessage(self.hwndEdit, EM_REPLACESEL, 0, TEXT(""))
				# And back to the end.
				SendMessage(self.hwndEdit, EM_SETSEL, -2, -2)
				SendMessage(self.hwndEdit, WM_SETREDRAW, 1, 0)
				
			SendMessage(self.hwndEdit, EM_REPLACESEL, 0, TEXT(text))

	def OnParentCommand(self, hwnd, msg, wparam, lparam):
		command = LOWORD(wparam)
		if command == IDM_EXIT:
			DestroyWindow(hwnd);
		elif command == IDM_ABOUT:
			DialogBox(self.hinst, IDD_ABOUT, hwnd, AboutBoxDlgProc)
		return 0

def AboutBoxDlgProc(hwnd, msg, wparam, lparam):
	if msg==WM_COMMAND:
		p=LOWORD(wparam)
		if p==IDOK or p==IDCANCEL:
			EndDialog(hwnd, 1)
		return 1
	return 0

def Interact(shell):
	shell.bInteract = 1
	locals = {}
	copyright = 'Type "copyright", "credits" or "license" for more information.'
	sys.stdout.write("Python %s on %s\n%s\n%s" % (sys.version, sys.platform, copyright, sys.ps1))
	
	while 1:
		rc = WaitForMultipleObjects( (shell.eventInteractiveInputAvailable, shell.eventClosed), 0, INFINITE)
		if rc == WAIT_OBJECT_0:
			codeText = string.join(shell.currentBlockItems, '\n')
			try:
				codeOb = code.compile_command(codeText)
			except SyntaxError:
				sys.stdout.write("\n")
				list = traceback.print_exc(0)
				sys.stdout.write(sys.ps1)
				continue
			except:
				traceback.print_exc()
				continue

			if codeOb is None:
				sys.stdout.write("\n%s" % sys.ps2)
				continue
			sys.stdout.write("\n")

			SetCursor(LoadCursor(0, IDC_WAIT))
			try:
				try:
					exec codeOb in locals
				except SystemExit:
					break
				except:
					exc_type, exc_value, exc_traceback = sys.exc_info()
					l = len(traceback.extract_tb(sys.exc_traceback))
					try: 1/0
					except:
						m = len(traceback.extract_tb(sys.exc_traceback))
					traceback.print_exception(exc_type,
						exc_value, exc_traceback, l-m)
					exc_traceback = None # Prevent a cycle
			finally:
				SetCursor(LoadCursor(0, 0))
				
			sys.stdout.write(sys.ps1)
		else:
			break

def RunCode(shell):
	try:
		# copy sys.argv before we stomp on it!
		sys.appargv = sys.argv[:]
		bKeepOpen = 0
		bInteract = 1
		cmdToExecute = None
		# Process sys.argv, removing args as we process them so any scripts
		# see _their_ argv!
		del sys.argv[0]
		# Remove some params the WCE debugger sometimes adds:
		sys.argv=filter(lambda arg: arg[:4]!="/WCE", sys.argv)
		i=0
		while i < len(sys.argv):
			if not sys.argv[i] or sys.argv[i][0]!='-':
				break
			if sys.argv[i]=='-i':
				bInteract = 1
				del sys.argv[i]
				continue
			elif sys.argv[i]=='-c':
				cmdToExecute = string.join(sys.argv[i+1:], ' ')
				sys.argv = sys.argv[:i-1]
				break
			i = i + 1
		
		if not sys.argv: sys.argv=['']

		if cmdToExecute is not None:
			try:
				exec cmdToExecute
			except:
				traceback.print_exc()
				bKeepOpen = 1
		elif len(sys.argv)>0 and sys.argv[0]:
			# Shift the args back to it sees itself as sys.argv[0]
			# Execute the named script
			fname = sys.argv[0]
			ext = os.path.splitext(fname)[1]
			if ext=='.pyc':
				mode="rb"
				imp_params=("pyc", mode, imp.PY_COMPILED)
			else:
				mode="r"
				imp_params=("py", mode, imp.PY_SOURCE)

			try:
				file = open(fname, mode)
			except IOError, (code, why):
				print "python: can't open %s: %s\n" % (fname, why)
				bKeepOpen = 1
				file = None
			if file:
				try:
					try:
						imp.load_module("__main__", file, fname, imp_params)
					except:
						traceback.print_exc()
						bKeepOpen = 1
				finally:
					file.close()
		else:
			bInteract = 1
		
		if bInteract:
			try:
				Interact(shell)
			except:
				traceback.print_exc()
				bKeepOpen = 1
	
		if not bKeepOpen:
			PostThreadMessage(shellThreadId, WM_QUIT, 0, 0)
	except:
		traceback.print_exc()
	
def main():
	# We run the shell in the main thread, so that when it terminates
	# (accidently or otherwise) the application terminates.
	# A seperate thread is used to execute the Python code.
	__name__ = sys.argv[0]

	# Make "shell" global just for debugging purposes
	# ie, so interactive code can see it via __main__.shell (or "ceshell.shell" on CE)
	global shell 
	shell = SimpleShell()
	global shellThreadId
	shellThreadId = thread.get_ident()
	# Create the windows, but dont start the message loop yet.
	shell.Init()

	# Can now write to the shell - assign the standard files.
	oldOut, oldErr = sys.stdout, sys.stderr
	sys.stderr = shell
	sys.stdout = shell

	# Create the new thread to execute the code.
	thread.start_new(RunCode, (shell,) )
	
	# Now run the shell.
	shell.Run()
	
	shell.Term()

	sys.stdout = oldOut
	sys.stderr = oldErr

# On Windows, run this as a script.
# On CE, this module is imported and main() executed by
# the startup C code.
if __name__=='__main__':
	main()
