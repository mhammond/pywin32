from pywin.mfc import dialog
import win32con
import win32ui
import copy

######################################################
# Property Page for syntax formatting options
    
# The standard 16 color VGA palette should always be possible    
paletteVGA = ( ("Black",0,0,0), ("Navy",0,0,128), ("Green",0,128,0), ("Cyan",0,128,128), 
	("Maroon",128,0,0), ("Purple",128,0,128), ("Olive",128,128,0), ("Gray",128,128,128), 
	("Silver",192,192,192), ("Blue",0,0,255), ("Lime",0,255,0), ("Aqua",0,255,255), 
	("Red",255,0,0), ("Fuchsia",255,0,255), ("Yellow",255,255,0), ("White",255,255,255) )

def BGR(b,g,r):		# Colors in font definitions are integers made up of Blue, Green, and Red bytes
    return b*256*256 + g*256 + r

class ScintillaFormatPropertyPage(dialog.PropertyPage):
	def __init__(self, scintillaClass = None):
		self.scintillaClass = scintillaClass
		dialog.PropertyPage.__init__(self, win32ui.IDD_PP_FORMAT)

	def OnInitDialog(self):
		if self.scintillaClass is None:
			import control
			sc = control.CScintillaEdit
		else:
			sc = self.scintillaClass

		self.scintilla = sc()
		style = win32con.WS_CHILD | win32con.WS_VISIBLE | win32con.ES_MULTILINE
		# Convert the rect size
		rect = self.MapDialogRect( (5, 5, 120, 75))
		self.scintilla.CreateWindow(style, rect, self, 111)
		self.scintilla.SCISetViewWS(1)

		colorizer = self.scintilla._GetColorizer()
		self.scintilla.SCIAddText(colorizer.GetSampleText())
		self.scintilla.Reformat()
		self.styles = self.scintilla._GetColorizer().styles

		self.cbo = self.GetDlgItem(win32ui.IDC_COMBO1)
		for c in paletteVGA:
			self.cbo.AddString(c[0])

		self.cboBoldItalic = self.GetDlgItem(win32ui.IDC_COMBO2)
		for item in ["Bold Italic", "Bold", "Italic", "Regular"]:
			self.cboBoldItalic.InsertString(0, item)

		self.butIsDefault = self.GetDlgItem(win32ui.IDC_CHECK1)
		self.listbox = self.GetDlgItem(win32ui.IDC_LIST1)
		self.HookCommand(self.OnListCommand, win32ui.IDC_LIST1)
		names = self.styles.keys()
		names.sort()
		for name in names:
			if self.styles[name].aliased is None:
				self.listbox.AddString(name)
		self.listbox.SetCurSel(0)

		idc = win32ui.IDC_RADIO1
		if not self.scintilla._GetColorizer().bUseFixed: idc = win32ui.IDC_RADIO2
		self.GetDlgItem(idc).SetCheck(1)
		self.UpdateUIForStyle(self.styles[names[0]])

		self.scintilla.HookStyleNotify(self)
		self.HookCommand(self.OnButDefaultFixedFont, win32ui.IDC_BUTTON1)
		self.HookCommand(self.OnButDefaultPropFont, win32ui.IDC_BUTTON2)
		self.HookCommand(self.OnButThisFont, win32ui.IDC_BUTTON3)
		self.HookCommand(self.OnButUseDefaultFont, win32ui.IDC_CHECK1)
		self.HookCommand(self.OnStyleUIChanged, win32ui.IDC_COMBO1)
		self.HookCommand(self.OnStyleUIChanged, win32ui.IDC_COMBO2)
		self.HookCommand(self.OnButFixedOrDefault, win32ui.IDC_RADIO1)
		self.HookCommand(self.OnButFixedOrDefault, win32ui.IDC_RADIO2)

	def GetSelectedStyle(self):
		return self.styles[self.listbox.GetText(self.listbox.GetCurSel())]

	def _DoButDefaultFont(self, extra_flags, attr):
		baseFormat = getattr(self.scintilla._GetColorizer(), attr)
		flags = extra_flags | win32con.CF_SCREENFONTS | win32con.CF_EFFECTS | win32con.CF_FORCEFONTEXIST
		d=win32ui.CreateFontDialog(baseFormat, flags, None, self)
		if d.DoModal()==win32con.IDOK:
			setattr(self.scintilla._GetColorizer(), attr, d.GetCharFormat())
			self.OnStyleUIChanged(0, win32con.BN_CLICKED)

	def OnButDefaultFixedFont(self, id, code):
		if code==win32con.BN_CLICKED:
			self._DoButDefaultFont(win32con.CF_FIXEDPITCHONLY, "baseFormatFixed")
			return 1

	def OnButDefaultPropFont(self, id, code):
		if code==win32con.BN_CLICKED:
			self._DoButDefaultFont(win32con.CF_SCALABLEONLY, "baseFormatProp")
			return 1

	def OnButFixedOrDefault(self, id, code):
		if code==win32con.BN_CLICKED:
			bUseFixed = id == win32ui.IDC_RADIO1
			self.GetDlgItem(win32ui.IDC_RADIO1).GetCheck() != 0
			self.scintilla._GetColorizer().bUseFixed = bUseFixed
			self.scintilla.Reformat(0)
			return 1

	def OnButThisFont(self, id, code):
		if code==win32con.BN_CLICKED:
			flags = win32con.CF_SCREENFONTS | win32con.CF_EFFECTS | win32con.CF_FORCEFONTEXIST
			style = self.GetSelectedStyle()
			d=win32ui.CreateFontDialog(style.format, flags, None, self)
			if d.DoModal()==win32con.IDOK:
				style.format = d.GetCharFormat()
				self.scintilla.Reformat(0)
			return 1

	def OnButUseDefaultFont(self, id, code):
		if code == win32con.BN_CLICKED:
			isDef = self.butIsDefault.GetCheck()
			self.GetDlgItem(win32ui.IDC_BUTTON3).EnableWindow(not isDef)
			if isDef: # Being reset to the default font.
				style = self.GetSelectedStyle()
				style.ForceAgainstDefault()
				self.UpdateUIForStyle(style)
				self.scintilla.Reformat(0)
			else:
				# User wants to override default -
				# do nothing!
				pass

	def OnListCommand(self, id, code):
		if code==win32con.LBN_SELCHANGE:
			style = self.GetSelectedStyle()
			self.UpdateUIForStyle(style)
		return 1

	def UpdateUIForStyle(self, style ):
		format = style.format
		sel = 0
		for c in paletteVGA:
			if format[4] == BGR(c[3], c[2], c[1]):
#				print "Style", style.name, "is", c[0]
				break
			sel = sel + 1
		else:
			sel = -1
		self.cbo.SetCurSel(sel)
		self.butIsDefault.SetCheck(style.IsBasedOnDefault())
		self.GetDlgItem(win32ui.IDC_BUTTON3).EnableWindow(not style.IsBasedOnDefault())
		bold = format[1] & win32con.CFE_BOLD != 0; italic = format[1] & win32con.CFE_ITALIC != 0
		self.cboBoldItalic.SetCurSel( bold*2 + italic )

	def OnStyleUIChanged(self, id, code):
		if code in [win32con.BN_CLICKED, win32con.CBN_SELCHANGE]:
			style = self.GetSelectedStyle()
			self.ApplyUIFormatToStyle(style)
			self.scintilla.Reformat(0)
			return 0
		return 1

	def ApplyUIFormatToStyle(self, style):
		format = style.format
		color = paletteVGA[self.cbo.GetCurSel()]
		effect = 0
		sel = self.cboBoldItalic.GetCurSel()
		if sel==0:
			effect = 0
		elif sel==1:
			effect = win32con.CFE_ITALIC
		elif sel==2:
			effect = win32con.CFE_BOLD
		else:
			effect = win32con.CFE_BOLD | win32con.CFE_ITALIC
		maskFlags=format[0]|win32con.CFM_COLOR|win32con.CFM_BOLD|win32con.CFM_ITALIC
		style.format = (maskFlags, effect, style.format[2], style.format[3], BGR(color[3], color[2], color[1])) + style.format[5:]

	def OnOK(self):
		self.scintilla._GetColorizer().SavePreferences()
		# Now tell _all_ Scintilla controls we can find to reformat
		# themselves.  Only ones attached to the formatter will have
		# any visible changes (although all will reload their options)
		for templ in win32ui.GetApp().GetDocTemplateList( ):
			for d in templ.GetDocumentList( ):
				# Try all documents, but only coloreditor.EditorDocument will respond
				try:
					fn = d.Reformat
				except AttributeError:
					continue
				fn()
		return 1

def test():
	page = ColorEditorPropertyPage()
	sheet = pywin.mfc.dialog.PropertySheet("Test")
	sheet.AddPage(page)
	sheet.CreateWindow()
