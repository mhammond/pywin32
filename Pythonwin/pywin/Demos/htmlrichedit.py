
import win32con
import win32ui
import win32api
import sys
import htmllib
import string
import Para
import urllib
import urlparse
import os
import formatter
import glob

from pywin.mfc import docview

maskFlags=win32con.CFM_SIZE|win32con.CFM_FACE|win32con.CFM_CHARSET
styles = { \
	'h1' : (maskFlags, 0, 280, 0, 0, 0, 34, 'Arial'),	\
	'h2' : (maskFlags, 0, 240, 0, 0, 0, 34, 'Arial'),	\
	'h3' : (maskFlags, 0, 220, 0, 0, 0, 34, 'Arial'), \
	'h4' : (maskFlags, 0, 200, 0, 0, 0, 34, 'Arial'), \
	'jt' : (win32con.CFM_COLOR,0,0,0,win32api.RGB(0,0,255)), \
	'default' : (maskFlags|win32con.CFM_BOLD|win32con.CFM_ITALIC, 0, 200, 0, 0, 0, 34, 'Arial') \
}

W4GParent=formatter.AbstractWriter
class RichEditWriter(W4GParent):
	def __init__(self, richedit):
		W4GParent.__init__ (self)
		self.richedit = richedit
	def close(self):
		Trace("Writer closing",3)
	def new_font(self, font):
		self.SetRTFFont(font)
	def new_margin(self, margin, level):
		space = level * 300
		try:
			self.richedit.SetParaFormat((win32con.PFM_STARTINDENT|win32con.PFM_OFFSET|win32con.PFM_NUMBERING, 0,0,space,0,0))
		except win32ui.error:
			pass # fails occasionaly - maybe if cant do _exactly_ (but seems to do best it can!)
	def send_label_data(self, data):
		try:
			off = self.richedit.GetParaFormat()[3]
			self.richedit.SetParaFormat((win32con.PFM_NUMBERING|win32con.PFM_OFFSET, win32con.PFN_BULLET, 0, 0, off+500))
		except win32ui.error:
			pass
	def new_spacing(self, spacing):
		print "new_spacing(%s)" % `spacing`
	def new_styles(self, styles):
		print "new_styles(%s)" % `styles`
	def send_paragraph(self, blankline):
		self.richedit.ReplaceSel('\r\n')
	def send_line_break(self):
		self.richedit.ReplaceSel('\n')
	def send_hor_rule(self):
		pass
#		print "send hor"

	def send_flowing_data(self, data):
		self.richedit.ReplaceSel(data)

	def send_literal_data(self, data):
		print "send literal", data

	def SetRTFFont( self, font ):
		if font is None:
			font = 'default',0,0,0

		face, i, b, tt = font
		if face is None:
			cf = (0,0,0,0,0,0,0,"")
		else:
			try:
				cf = styles[face]
			except KeyError:
				print "Unknown font - %s - ignored " % `face`
				return
		mask = cf[0] | win32con.CFM_ITALIC  | win32con.CFM_BOLD
		effect = cf[1]
		if i:
			effect = effect | win32con.CFE_ITALIC
		if b: 
			effect = effect | win32con.CFE_BOLD
		if tt:
			print "have tt - ignoring"
		cf = mask, effect, cf[2], cf[3], cf[4], cf[5], cf[6], cf[7]
#		print "cf = ", cf
		self.richedit.SetSelectionCharFormat(cf)

WPParent=htmllib.HTMLParser
class RichEditParser(WPParent):
	def __init__(self, formatter, richedit):
		self.richedit = richedit
		WPParent.__init__(self, formatter)
	def close(self):
		WPParent.close(self)
		
	def anchor_bgn(self, href, name, type):
		WPParent.anchor_bgn(self, href, name, type)
		self.richedit.SetSelectionCharFormat((win32con.CFM_COLOR,0,0,0,win32api.RGB(0,0,255)))

	def anchor_end(self):
		self.richedit.SetSelectionCharFormat((win32con.CFM_COLOR,0,0,0,win32api.RGB(0,0,0)))
	# support multiple levels of UL (Unstructured List!)
	def start_meta(self, attrs):
		meta_name = meta_value = None
		for attrname, value in attrs:
			if attrname == 'name':
				meta_name = value
			if attrname == 'value':
				meta_value = value
		if meta_name and meta_value:
			if meta_name == "keywords":
				print "Meta: ", meta_value
	def end_meta(self):
		pass

	def do_img(self, attrs):
		print "do img - ", attrs


class HTMLTemplate(docview.RichEditDocTemplate):
	def __init__(self):
		docview.RichEditDocTemplate.__init__(self, win32ui.IDR_PYTHONTYPE, docview.RichEditDoc, None, docview.RichEditView)
		win32ui.GetApp().AddDocTemplate(self)

try:
	template
except NameError:
	template = HTMLTemplate()

def test():
    import sys, regutil
    # This should be registered as a HTML file
    file = regutil.GetRegisteredHelpFile("Main Python Documentation")
#    if sys.argv[1:]: file = sys.argv[1]
    if not file:
        print "The main Python HTML page is not registered, and no file was specified..."
        return

    try:
        fp = open(file, 'r')
    except IOError, details:
        print "Could not open main Python HTML page '%s'" % (file)
        print details
        return
    
    data = fp.read()
    fp.close()
    doc = template.OpenDocumentFile()
    doc.SetTitle("HTML to RichText demo")
    richedit = doc.GetFirstView()
    from formatter import DumbWriter
    w = RichEditWriter(richedit)
    f = formatter.AbstractFormatter(w)
    p = RichEditParser(f, richedit)
    p.feed(data)
    p.close()
    doc.SetModifiedFlag(0)
    return doc

if __name__=='__main__':
	import demoutils
	if demoutils.NeedGoodGUI():
		test()

