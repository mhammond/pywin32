# Regedit - a Registry Editor for Python
import win32api, win32ui, win32con, commctrl
from pywin.mfc import window, docview, dialog
import hierlist
import regutil

class SplitterFrame(window.MDIChildWnd):
	def __init__(self):
		# call base CreateFrame
		self.images = None
		window.MDIChildWnd.__init__(self)

	def OnCreateClient(self, cp, context):
		splitter = win32ui.CreateSplitter()
		doc = context.doc
		frame_rect = self.GetWindowRect()
		size = ((frame_rect[2] - frame_rect[0]),
		        (frame_rect[3] - frame_rect[1])/2)
		sub_size = (size[0]/3, size[1])
		splitter.CreateStatic (self, 1, 2)
		# CTreeControl view
		self.keysview = RegistryTreeView(doc)
		# CListControl view
		self.valuesview = RegistryValueView(doc)
		
		splitter.CreatePane (self.keysview, 0, 0, (sub_size))
		splitter.CreatePane (self.valuesview, 0, 1, (0,0)) # size ignored.
		splitter.SetRowInfo(0, size[1] ,0)
		# Setup items in the imagelist
		
		return 1

	def OnItemDoubleClick(self,(hwndFrom, idFrom, code), extra):
		if idFrom==win32ui.AFX_IDW_PANE_FIRST:
			# Tree control
			return None
		elif idFrom==win32ui.AFX_IDW_PANE_FIRST + 1:
			item = self.keysview.SelectedItem()
			self.valuesview.EditValue(item)
			return 0
			# List control
		else:
			return None # Pass it on

	def PerformItemSelected(self,item):
		return self.valuesview.UpdateForRegItem(item)

	def OnDestroy(self, msg):
		window.MDIChildWnd.OnDestroy(self, msg)
		if self.images:
			self.images.DeleteImageList()
			self.images = None

class RegistryTreeView(docview.TreeView):
	def OnInitialUpdate(self):
		rc = self._obj_.OnInitialUpdate()
		self.frame = self.GetParent().GetParent()
		doc = self.GetDocument()
		regroot = doc.root
		subkey = doc.subkey
		self.hierList = hierlist.HierListWithItems( HLIRegistryKey(regroot, subkey, "Root"), win32ui.IDB_HIERFOLDERS, win32ui.AFX_IDW_PANE_FIRST)
		self.hierList.HierInit(self.frame, self.GetTreeCtrl())
		self.hierList.SetStyle(commctrl.TVS_HASLINES | commctrl.TVS_LINESATROOT | commctrl.TVS_HASBUTTONS)
		self.hierList.PerformItemSelected = self.PerformItemSelected

		self.frame.HookNotify(self.frame.OnItemDoubleClick, commctrl.NM_DBLCLK)
#		self.frame.HookNotify(self.frame.OnItemRightClick, commctrl.NM_RCLICK)
		self.HookMessage(self.OnItemRightClick, win32con.WM_RBUTTONUP)
		
	def OnItemRightClick(self, msg):
		menu = win32ui.CreatePopupMenu()
		menu.AppendMenu(win32con.MF_STRING|win32con.MF_ENABLED,1000, "Delete Key")
		self.HookCommand(self.OnDeleteKey, 1000)
		menu.TrackPopupMenu(win32api.GetCursorPos()) # track at mouse position.
		return None

	def OnDeleteKey(self,command, code):
		print "Have Delete Key"
		print self.SelectedItem().GetText()
		print self.SearchSelectedItem().GetText()

	def PerformItemSelected(self, item):
		return self.frame.PerformItemSelected(item)

	def SelectedItem(self):
		return self.hierList.ItemFromHandle(self.hierList.GetSelectedItem())
		
	def SearchSelectedItem(self):
		handle = self.hierList.GetChildItem(0)
		while 1:
#			print "State is", self.hierList.GetItemState(handle, -1)
			if self.hierList.GetItemState(handle, commctrl.TVIS_FOCUSED):
#				print "Item is ", self.hierList.ItemFromHandle(handle)
				return self.hierList.ItemFromHandle(handle)
			handle = self.hierList.GetNextSiblingItem(handle)

class RegistryValueView(docview.ListView):
	def OnInitialUpdate(self):
		hwnd = self._obj_.GetSafeHwnd()
		style = win32api.GetWindowLong(hwnd, win32con.GWL_STYLE);
		win32api.SetWindowLong(hwnd, win32con.GWL_STYLE, (style & ~commctrl.LVS_TYPEMASK) | commctrl.LVS_REPORT); 

		itemDetails = (commctrl.LVCFMT_LEFT, 100, "Name", 0)
		self.InsertColumn(0, itemDetails)
		itemDetails = (commctrl.LVCFMT_LEFT, 500, "Data", 0)
		self.InsertColumn(1, itemDetails)

	def UpdateForRegItem(self, item):
		self.DeleteAllItems()
		hkey = win32api.RegOpenKey(item.keyRoot, item.keyName)
		try:
			valNum = 0
			ret = []
			while 1:
				try:
					res = win32api.RegEnumValue(hkey, valNum)
				except win32api.error:
					break
				name = res[0]
				if not name: name = "(Default)"
				self.InsertItem(valNum, name)
				self.SetItemText(valNum, 1, str(res[1]))
				valNum = valNum + 1
		finally:
			win32api.RegCloseKey(hkey)
	def EditValue(self, item):
		# Edit the current value
		class EditDialog(dialog.Dialog):
			def __init__(self, item):
				self.item = item
				dialog.Dialog.__init__(self, win32ui.IDD_LARGE_EDIT)
			def OnInitDialog(self):
				self.SetWindowText("Enter new value")
				self.GetDlgItem(win32con.IDCANCEL).ShowWindow(win32con.SW_SHOW)
				self.edit = self.GetDlgItem(win32ui.IDC_EDIT1)
				# Modify the edit windows style
				style = win32api.GetWindowLong(self.edit.GetSafeHwnd(), win32con.GWL_STYLE)
				style = style & (~win32con.ES_WANTRETURN)
				win32api.SetWindowLong(self.edit.GetSafeHwnd(), win32con.GWL_STYLE, style)
				self.edit.SetWindowText(str(self.item))
				self.edit.SetSel(-1)
				return dialog.Dialog.OnInitDialog(self)
			def OnDestroy(self,msg):
				self.newvalue = self.edit.GetWindowText()
		
		try:
			index = self.GetNextItem(-1, commctrl.LVNI_SELECTED)
		except win32ui.error:
			return # No item selected.

		if index==0:
			keyVal = ""
		else:
			keyVal = self.GetItemText(index,0)
		# Query for a new value.
		try:
			newVal = self.GetItemsCurrentValue(item, keyVal)
		except TypeError, details:
			win32ui.MessageBox(details)
			return
		
		d = EditDialog(newVal)
		if d.DoModal()==win32con.IDOK:
			try:
				self.SetItemsCurrentValue(item, keyVal, d.newvalue)
			except win32api.error, (rc, fn, desc):
				win32ui.MessageBox("Error setting value\r\n\n%s" % desc)
			self.UpdateForRegItem(item)

	def GetItemsCurrentValue(self, item, valueName):
		hkey = win32api.RegOpenKey(item.keyRoot, item.keyName)
		try:
			val, type = win32api.RegQueryValueEx(hkey, valueName)
			if type != win32con.REG_SZ:
				raise TypeError, "Only strings can be edited"
			return val
		finally:
			win32api.RegCloseKey(hkey)
	
	def SetItemsCurrentValue(self, item, valueName, value):
		# ** Assumes already checked is a string.
		hkey = win32api.RegOpenKey(item.keyRoot, item.keyName , 0, win32con.KEY_SET_VALUE)
		try:
			win32api.RegSetValueEx(hkey, valueName, 0, win32con.REG_SZ, value)
		finally:
			win32api.RegCloseKey(hkey)


class RegTemplate(docview.DocTemplate):
	def __init__(self):
		docview.DocTemplate.__init__(self, win32ui.IDR_PYTHONTYPE, None, SplitterFrame, None)

#	def InitialUpdateFrame(self, frame, doc, makeVisible=1):
#		self._obj_.InitialUpdateFrame(frame, doc, makeVisible) # call default handler.
#		frame.InitialUpdateFrame(doc, makeVisible)

	def OpenRegistryKey(self, root = None, subkey = None): # Use this instead of OpenDocumentFile.
		# Look for existing open document
		if root is None: root = regutil.GetRootKey()
		if subkey is None: subkey = regutil.BuildDefaultPythonKey()
		for doc in self.GetDocumentList():
			if doc.root==root and doc.subkey==subkey:
				doc.GetFirstView().ActivateFrame()
				return doc
		# not found - new one.
		doc = RegDocument(self, root, subkey)
		frame = self.CreateNewFrame(doc)
		doc.OnNewDocument()
		self.InitialUpdateFrame(frame, doc, 1)
		return doc

class RegDocument (docview.Document):
	def __init__(self, template, root, subkey):
		docview.Document.__init__(self, template)
		self.root = root
		self.subkey = subkey
		self.SetTitle("Registry Editor: " + subkey)

	def OnOpenDocument (self, name):
		raise TypeError, "This template can not open files"
		return 0
		

class HLIRegistryKey(hierlist.HierListItem):
	def __init__( self, keyRoot, keyName, userName ):
		self.keyRoot = keyRoot
		self.keyName = keyName
		self.userName = userName
		hierlist.HierListItem.__init__(self)
	def GetText(self):
		return self.userName
	def IsExpandable(self):
		hkey = win32api.RegOpenKey(self.keyRoot, self.keyName)
		try:
			keys, vals, dt = win32api.RegQueryInfoKey(hkey)
			return (keys>0)
		finally:
			win32api.RegCloseKey(hkey)
		
	def GetSubList(self):
		hkey = win32api.RegOpenKey(self.keyRoot, self.keyName)
		win32ui.DoWaitCursor(1)
		try:
			keyNum = 0
			ret = []
			while 1:
				try:
					key = win32api.RegEnumKey(hkey, keyNum)
				except win32api.error:
					break
				ret.append(HLIRegistryKey(self.keyRoot, self.keyName + "\\" + key, key))
				keyNum = keyNum + 1
		finally:
			win32api.RegCloseKey(hkey)
			win32ui.DoWaitCursor(0)
		return ret

template = RegTemplate()

def EditRegistry(root = None, key = None):
	doc=template.OpenRegistryKey(root, key)

if __name__=='__main__':
	EditRegistry()
