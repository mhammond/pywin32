from pywin.mfc import dialog
import document
import win32ui
import win32con

from pywin.framework.editor import GetEditorOption, SetEditorOption, DeleteEditorOption, GetEditorFontOption, SetEditorFontOption, defaultCharacterFormat, editorTemplate
import pywin.scintilla.config
######################################################
#
# Property Page for editor options
#
class EditorPropertyPage(dialog.PropertyPage):
	def __init__(self):
		dialog.PropertyPage.__init__(self, win32ui.IDD_PP_EDITOR)
		self.autooptions = []
		self._AddEditorOption(win32ui.IDC_TAB_SIZE, "i", "Tab Size", 4)
		self._AddEditorOption(win32ui.IDC_INDENT_SIZE, "i", "Indent Size", 4)
		self._AddEditorOption(win32ui.IDC_USE_TABS, "i", "Use Tabs", 0)
		self._AddEditorOption(win32ui.IDC_AUTO_RELOAD, "i", "Auto Reload", 1)
		self._AddEditorOption(win32ui.IDC_COMBO1, "i", "Backup Type", document.BAK_DOT_BAK_BAK_DIR)
		self._AddEditorOption(win32ui.IDC_USE_SMART_TABS, "i", "Smart Tabs", 1)
		self._AddEditorOption(win32ui.IDC_VIEW_WHITESPACE, "i", "View Whitespace", 0)
		self._AddEditorOption(win32ui.IDC_AUTOCOMPLETE, "i", "Autocomplete Attributes", 1)
		self._AddEditorOption(win32ui.IDC_CALLTIPS, "i", "Show Call Tips", 1)

		self.AddDDX(win32ui.IDC_VSS_INTEGRATE, "bVSS")
		self.AddDDX(win32ui.IDC_EDITOR_COLOR, "Module", "i")
		mod = GetEditorOption("Module", "")
		self['Module'] = not mod or mod=="pywin.framework.editor.color.coloreditor"

		self.AddDDX(win32ui.IDC_KEYBOARD_CONFIG, "Configs", "l")
		self["Configs"] = pywin.scintilla.config.find_config_files()

	def _AddEditorOption(self, idd, typ, optionName, defaultVal):
		self.AddDDX(idd, optionName, typ)
		self[optionName] = GetEditorOption(optionName, defaultVal)
		self.autooptions.append(optionName, defaultVal)

	def OnInitDialog(self):
		for name, val in self.autooptions:
			self[name] = GetEditorOption(name, val)

		# Note that these MUST be in the same order as the BAK constants.
		cbo = self.GetDlgItem(win32ui.IDC_COMBO1)
		cbo.AddString("None")
		cbo.AddString(".BAK File")
		cbo.AddString("TEMP dir")
		cbo.AddString("Own dir")

		# Source Safe
		bVSS = GetEditorOption("Source Control Module", "") == "pywin.framework.editor.vss"
		self['bVSS'] = bVSS

		rc = dialog.PropertyPage.OnInitDialog(self)

		try:
			self.GetDlgItem(win32ui.IDC_KEYBOARD_CONFIG).SelectString(-1, GetEditorOption("Keyboard Config", "default"))
		except win32ui.error:
			import traceback
			traceback.print_exc()
			pass

		return rc

	def OnOK(self):
		for name, defVal in self.autooptions:
			SetEditorOption(name, self[name])
		if self['bVSS']:
			SetEditorOption("Source Control Module", "pywin.framework.editor.vss")
		else:
			if GetEditorOption("Source Control Module", "")=='pywin.framework.editor.vss':
				SetEditorOption("Source Control Module", "")
		rc = self['Module']
		if rc == 0:
			# Delete the option!
			SetEditorOption('Module', 'pywin.framework.editor.editor')
		else:
			# Color editor!
			DeleteEditorOption('Module')

		# Keyboard config
		configname = self.GetDlgItem(win32ui.IDC_KEYBOARD_CONFIG).GetWindowText()
		if configname:
			if configname == "default":
				DeleteEditorOption("Keyboard Config")
			else:
				SetEditorOption("Keyboard Config", configname)

			import pywin.scintilla.view
			pywin.scintilla.view.LoadConfiguration()

		# Now tell all views we have changed.
		for doc in editorTemplate.GetDocumentList():
			for view in doc.GetAllViews():
				try:
					fn = view.OnConfigChange
				except AttributeError:
					continue
				fn()
		return 1


def testpp():
	ps = dialog.PropertySheet("Editor Options")
	ps.AddPage(EditorPropertyPage())
	ps.DoModal()


