# Creates a task-bar icon.  Run from Python.exe to see the
# messages printed.
from win32api import *
from win32gui import *
import win32con

class MainWindow:
	def __init__(self):
		message_map = {
			win32con.WM_DESTROY: self.OnDestroy,
			win32con.WM_COMMAND: self.OnCommand,
			win32con.WM_USER+20 : self.OnTaskbarNotify,
		}
		# Register the Window class.
		wc = WNDCLASS()
		hinst = wc.hInstance = GetModuleHandle(None)
		wc.lpszClassName = "PythonTaskbarDemo"
		wc.style = win32con.CS_VREDRAW | win32con.CS_HREDRAW;
		wc.hCursor = LoadCursor( 0, win32con.IDC_ARROW )
		wc.hbrBackground = win32con.COLOR_WINDOW
		wc.lpfnWndProc = message_map # could also specify a wndproc.
		classAtom = RegisterClass(wc)
		# Create the Window.
		style = win32con.WS_OVERLAPPED | win32con.WS_SYSMENU
		self.hwnd = CreateWindow( classAtom, "Taskbar Demo", style, \
	                0, 0, win32con.CW_USEDEFAULT, win32con.CW_USEDEFAULT, \
	                0, 0, hinst, None)
		UpdateWindow(self.hwnd)

		# Add the taskbar icon
		hicon = LoadIcon(0, win32con.IDI_APPLICATION)
		flags = NIF_ICON | NIF_MESSAGE | NIF_TIP
		nid = (self.hwnd, 0, flags, win32con.WM_USER+20, hicon, "Python Demo")
		Shell_NotifyIcon(NIM_ADD, nid)

	def OnDestroy(self, hwnd, msg, wparam, lparam):
		nid = (self.hwnd, 0)
		Shell_NotifyIcon(NIM_DELETE, nid)
		PostQuitMessage(0) # Terminate the app.

	def OnTaskbarNotify(self, hwnd, msg, wparam, lparam):
		if lparam==win32con.WM_LBUTTONUP:
			print "You clicked me."
		elif lparam==win32con.WM_LBUTTONDBLCLK:
			print "You double-clicked me - goodbye"
			DestroyWindow(self.hwnd)
		elif lparam==win32con.WM_RBUTTONUP:
			print "You right clicked me."
			menu = CreatePopupMenu()
			AppendMenu( menu, win32con.MF_STRING, 1024, "Say Hello")
			AppendMenu( menu, win32con.MF_STRING, 1025, "Exit program" )
			pos = GetCursorPos()
			TrackPopupMenu(menu, win32con.TPM_LEFTALIGN, pos[0], pos[1], 0, self.hwnd, None)
		return 1

	def OnCommand(self, hwnd, msg, wparam, lparam):
		id = LOWORD(wparam)
		if id == 1024:
			print "Hello"
		elif id == 1025:
			print "Goodbye"
			DestroyWindow(self.hwnd)
		else:
			print "Unknown command -", id

def main():
	w=MainWindow()
	PumpMessages()

if __name__=='__main__':
	main()