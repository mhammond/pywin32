from win32gui import *
import sys
import traceback

IDOK=1
IDCANCEL=2

IDD_REMOTE=104
IDC_REMOTE_STATUS=1033
IDI_ICON=102
IDC_BUTTON=1031

CW_USEDEFAULT=0x80000000

SW_HIDE=0
SW_SHOW=5

WM_ACTIVATE=6
WM_COMMAND=273
WM_DESTROY=2
WM_INITDIALOG=272
WM_LBUTTONDOWN=513
WM_LBUTTONUP=514
WM_USER=1024

class StatusDialog:
	def __init__(self):
		try:
			self.hinst = sys.hinst
		except AttributeError:
			import win32api
			self.hinst = win32api.GetModuleHandle(None)

	def Run(self):
		self.messageMap = {
			WM_INITDIALOG : self.OnInitDialog,
			WM_COMMAND : self.OnCommand,
			WM_DESTROY : self.OnDestroy,
			WM_USER+20 : self.OnTaskbarNotify,
		}
		DialogBox(self.hinst, IDD_REMOTE, 0, self.DlgProc)

	def DlgProc(self, hwnd, msg, wparam, lparam):
		fn = self.messageMap.get(msg)
		if fn is None:
			return 0
		try:
			return fn(hwnd, msg, wparam, lparam)
		except:
			traceback.print_exc()
			return 0

	def OnInitDialog(self, hwnd, msg, wparam, lparam):
		self.hwnd = hwnd
		SetWindowText(hwnd, "PythonCE Taskbar Demo")
		# Do it this way...
		hEdit = GetDlgItem(hwnd, IDC_REMOTE_STATUS)
		SetWindowText(hEdit, "Hello from the Taskbar Demo")
		# and the single line way!
		SetDlgItemText(hwnd, IDC_BUTTON, "&Hide")

		# Add the taskbar icon
		hicon = LoadIcon(self.hinst, IDI_ICON)
		flags = NIF_ICON | NIF_MESSAGE | NIF_TIP
		nid = (hwnd, 0, flags, WM_USER+20, hicon, "Python")
		Shell_NotifyIcon(NIM_ADD, nid)
		
		return 1

	def OnDestroy(self, hwnd, msg, wparam, lparam):
		nid = (self.hwnd, 0)
		Shell_NotifyIcon(NIM_DELETE, nid)

	def OnTaskbarNotify(self, hwnd, msg, wparam, lparam):
		if lparam==WM_LBUTTONUP:
			if not IsWindowVisible(hwnd):
				ShowWindow(self.hwnd, SW_SHOW)
			# Ensure on top!
			SetForegroundWindow(hwnd)
		return 1

	def OnCommand(self, hwnd, msg, wparam, lparam):
		p=LOWORD(wparam)
		if p==IDOK or p==IDCANCEL:
			EndDialog(hwnd, 1)
		elif p==IDC_BUTTON:
			ShowWindow(self.hwnd, SW_HIDE)
		return 1

def main():
	d=StatusDialog()
	SetCursor(LoadCursor(0, 0))
	d.Run()

if __name__=='__main__':
	main()
