# No cancel button.

from pywin.mfc import dialog
import win32ui
import win32con

def MakeProgressDlgTemplate(caption, staticText = ""):
    style = (win32con.DS_MODALFRAME |
	     win32con.WS_POPUP |
	     win32con.WS_VISIBLE |
	     win32con.WS_CAPTION |
	     win32con.WS_SYSMENU |
	     win32con.DS_SETFONT)
    cs = (win32con.WS_CHILD |
	  win32con.WS_VISIBLE)

    w = 215
    h = 36 # With button
    h = 40

    dlg = [[caption,
	    (0, 0, w, h),
	    style,
	    None,
	    (8, "MS Sans Serif")],
	   ]

    s = win32con.WS_TABSTOP | cs
    
    dlg.append([130, staticText, 1000, (7, 7, w-7, h-32), cs | win32con.SS_LEFT])

#    dlg.append([128,
#		"Cancel",
#		win32con.IDCANCEL,
#		(w - 60, h - 18, 50, 14), s | win32con.BS_PUSHBUTTON])

    return dlg

class CStatusProgressDialog(dialog.Dialog):
	def __init__(self, title, msg = "", maxticks = 100, tickincr = 1):
		self.initMsg = msg
		templ = MakeProgressDlgTemplate(title, msg)
		dialog.Dialog.__init__(self, templ)
		self.maxticks = maxticks
		self.tickincr = tickincr
		self.pbar = None
		
	def OnInitDialog(self):
		rc = dialog.Dialog.OnInitDialog(self)
		self.static = self.GetDlgItem(1000)
		self.pbar = win32ui.CreateProgressCtrl()
		self.pbar.CreateWindow (win32con.WS_CHILD |
						win32con.WS_VISIBLE,
						(10, 30, 310, 44),
						self, 1001)
		self.pbar.SetRange(0, self.maxticks)
		self.pbar.SetStep(self.tickincr)
		self.progress = 0
		self.pincr = 5
		return rc
	
	def Close(self):
		self.EndDialog(0)

	def SetMaxTicks(self, maxticks):
		if self.pbar is not None:
			self.pbar.SetRange(0, maxticks)

	def Tick(self):
		if self.pbar is not None:
			self.pbar.StepIt()
			win32ui.PumpWaitingMessages(0, -1)

	def SetTitle(self, text):
		self.SetWindowText(text)
			
	def SetText(self, text):
		self.SetDlgItemText(1000, text)
      
	def Set(self, pos, max = None):
		if self.pbar is not None:
			self.pbar.SetPos(pos)
			win32ui.PumpWaitingMessages(0, -1)
			if max is not None:
				self.pbar.SetRange(0, max)


def StatusProgressDialog(title, msg = "", maxticks = 100, parent = None):
	d = CStatusProgressDialog (title, msg, maxticks)
	d.CreateWindow (parent)
	return d

def demo():
	d = StatusProgressDialog("A Demo", "Doing something...")
	import win32api
	for i in range(100):
		if i == 50:
			d.SetText("Getting there...")
		if i==90:
			d.SetText("Nearly done...")
		win32api.Sleep(20)
		d.Tick()
	d.Close()
	
if __name__=='__main__':
	demo()
