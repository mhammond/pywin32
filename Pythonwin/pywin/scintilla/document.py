import win32ui
from pywin.mfc import docview

ParentScintillaDocument=docview.Document
class CScintillaDocument(ParentScintillaDocument):
	"A SyntEdit document. "
	def DeleteContents(self):
		self.text = ""

	def OnOpenDocument(self, filename):
		# init data members
		#print "Opening", filename
		self.SetPathName(filename) # Must set this early!
		try:
			f = open(filename, 'rb')
			try:
				self.text = f.read()
			finally:
				f.close()
		except IOError:
			win32ui.MessageBox("Could not load the file from %s" % filename)
			return 0

		if self.GetFirstView():
			self.GetFirstView()._SetLoadedText(self.text)
		self.SetModifiedFlag(0) # No longer dirty
		return 1
		
	def SaveFile(self, fileName):
		view = self.GetFirstView()
		ok = view.SaveTextFile(fileName)
		if ok:
			self._ApplyOptionalToViews("SCISetSavePoint")
		return ok
		
	def Reformat(self):
		self._ApplyOptionalToViews("Reformat")
