# Does Python source formatting for Scintilla controls.
import win32ui
import win32con
import string
import array

WM_KICKIDLE = 0x036A

debugging = 0
if debugging:
	# Output must go to another process else the result of
	# the printing itself will trigger again trigger a trace.
	import sys, win32traceutil, win32trace 
	def trace(*args):
		win32trace.write(string.join(map(str, args), " ") + "\n")
else:
	trace = lambda *args: None

class Style:
	"""Represents a single format
	"""
	def __init__(self, name, format):
		self.name = name # Name the format representes eg, "String", "Class"
		if type(format)==type(''):
			self.aliased = format
			self.format = None
		else:
			self.format = format
			self.aliased = None
		self.stylenum = None # Not yet registered.
	def IsBasedOnDefault(self):
		return len(self.format)==5
	# If the currently extended font defintion matches the
	# default format, restore the format to the "simple" format.
	def NormalizeAgainstDefault(self, defaultFormat):
		if self.IsBasedOnDefault():
			return 0 # No more to do, and not changed.
		bIsDefault = self.format[7] == defaultFormat[7] and \
		             self.format[2] == defaultFormat[2]
		if bIsDefault:
			self.ForceAgainstDefault()
		return bIsDefault
	def ForceAgainstDefault(self):
		self.format = self.format[:5]

# An abstract formatter
class Formatter:
	def __init__(self, scintilla):
		self.bCompleteWhileIdle = 1
		self.bHaveIdleHandler = 0 # Dont currently have an idle handle
		self.scintilla = scintilla
		self.nextstylenum = 0
		self.baseFormatFixed = (-402653169, 0, 200, 0, 0, 0, 49, 'Courier New')
		self.baseFormatProp = (-402653169, 0, 200, 0, 0, 0, 49, 'Arial')
		self.bUseFixed = 1
		self.styles = {} # Indexed by name
		self.styles_by_id = {} # Indexed by allocated ID.

	def GetSampleText(self):
		return "Sample Text for the Format Dialog"

	def ColorSeg(self, start, end, styleName):
		end = end+1
#		assert end-start>=0, "Can't have negative styling"
		stylenum = self.styles[styleName].stylenum
		while start<end:
			self.style_buffer[start]=chr(stylenum)
			start = start+1
		#self.scintilla.SCISetStyling(end - start + 1, stylenum)

	def RegisterStyle(self, style):
		assert style.stylenum is None, "Style has already been registered"
		assert self.styles.get(self.nextstylenum) is None, "We are reusing a style number!"
		stylenum = style.stylenum = self.nextstylenum
		self.nextstylenum = self.nextstylenum + 1
		self.styles[style.name] = style
		self.styles_by_id[stylenum] = style

	# Update the control with the new style format.
	def _ReformatStyle(self, style):
		assert style.stylenum is not None, "Unregistered style."
		#print "Reformat style", style.name, style.stylenum
		scintilla=self.scintilla
		stylenum = style.stylenum
		# Now we have the style number, indirect for the actual style.
		if style.aliased is not None:
			style = self.styles[style.aliased]
		f=style.format
		if style.IsBasedOnDefault():
			if self.bUseFixed: baseFormat = self.baseFormatFixed
			else: baseFormat = self.baseFormatProp
		else: baseFormat = f
		scintilla.SCIStyleSetFore(stylenum, f[4])
		scintilla.SCIStyleSetFont( stylenum, baseFormat[7])
		if f[1] & 1: scintilla.SCIStyleSetBold(stylenum, 1)
		else: scintilla.SCIStyleSetBold(stylenum, 0)
		if f[1] & 2: scintilla.SCIStyleSetItalic(stylenum, 1)
		else: scintilla.SCIStyleSetItalic(stylenum, 0)
		scintilla.SCIStyleSetSize(stylenum, int(baseFormat[2]/20))

	def GetStyleByNum(self, stylenum):
		return self.styles_by_id[stylenum]

	def Reformat(self, bReload=1):
		if bReload:
			self.LoadPreferences()
		if self.bUseFixed: baseFormat = self.baseFormatFixed
		else: baseFormat = self.baseFormatProp
		for style in self.styles.values():
			if style.aliased is None:
				style.NormalizeAgainstDefault(baseFormat)
			self._ReformatStyle(style)
		self.scintilla.InvalidateRect()

	def ColorizeString(self, str, charStart, styleStart):
		raise RuntimeError, "You must override this method"

	def Colorize(self, start=0, end=-1):
		scintilla = self.scintilla
		stringVal = scintilla.GetTextRange(start, end)
		if start > 0:
			stylenum = scintilla.SCIGetStyleAt(start - 1)
			styleStart = self.GetStyleByNum(stylenum).name
		else:
			styleStart = None
#		trace("Coloring", start, end, end-start, len(stringVal), styleStart, self.scintilla.SCIGetCharAt(start))
		scintilla.SCIStartStyling(start, 31)
		self.style_buffer = array.array("c", chr(0)*len(stringVal))
		self.ColorizeString(stringVal, styleStart)
		scintilla.SCISetStylingEx(self.style_buffer)
		self.style_buffer = None
#		trace("After styling, end styled is", self.scintilla.SCIGetEndStyled())
		if self.bCompleteWhileIdle and not self.bHaveIdleHandler and end!=-1 and end < scintilla.GetTextLength():
			self.bHaveIdleHandler = 1
			win32ui.GetApp().AddIdleHandler(self.DoMoreColoring)
			# Kicking idle makes the app seem slower when initially repainting!
#			win32ui.GetMainFrame().PostMessage(WM_KICKIDLE, 0, 0)

	def DoMoreColoring(self, handler, count):
		try:
			scintilla = self.scintilla
			endStyled = scintilla.SCIGetEndStyled()
			lineStartStyled = scintilla.LineFromChar(endStyled)
			start = scintilla.LineIndex(lineStartStyled)
			end = scintilla.LineIndex(lineStartStyled+1)

			finished = end >= scintilla.GetTextLength()
			self.Colorize(start, end)
		except (win32ui.error, AttributeError):
			# Window may have closed before we finished - no big deal!
			finished = 1

		if finished:
			self.bHaveIdleHandler = 0
			win32ui.GetApp().DeleteIdleHandler(handler)
		return not finished

	# Some functions for loading and saving preferences.  By default
	# an INI file (well, MFC maps this to the registry) is used.
	def LoadPreferences(self):
		self.baseFormatFixed = eval(self.LoadPreference("Base Format Fixed", str(self.baseFormatFixed)))
		self.baseFormatProp = eval(self.LoadPreference("Base Format Proportional", str(self.baseFormatProp)))
		self.bUseFixed = int(self.LoadPreference("Use Fixed", 1))
		for style in self.styles.values():
			new = self.LoadPreference(style.name, str(style.format))
			style.format = eval(new)

	def LoadPreference(self, name, default):
		return win32ui.GetProfileVal("Format", name, default)

	def SavePreferences(self):
		self.SavePreference("Base Format Fixed", str(self.baseFormatFixed))
		self.SavePreference("Base Format Proportional", str(self.baseFormatProp))
		self.SavePreference("Use Fixed", self.bUseFixed)
		for style in self.styles.values():
			if style.aliased is None:
				self.SavePreference(style.name, str(style.format))
	def SavePreference(self, name, value):
		win32ui.WriteProfileVal("Format", name, value)

# A Formatter that knows how to format Python source
from keyword import iskeyword

wordstarts = '_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
wordchars = '._0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
operators = '%^&*()-+=|{}[]:;<>,/?!.~'

STYLE_DEFAULT = "Whitespace"
STYLE_COMMENT = "Comment"
STYLE_NUMBER = "Number"
STYLE_STRING = "String"
STYLE_SQSTRING = "SQ String"
STYLE_TQSSTRING = "TQS String"
STYLE_TQDSTRING = "TQD String"
STYLE_KEYWORD = "Keyword"
STYLE_CLASS = "Class"
STYLE_METHOD = "Method"
STYLE_OPERATOR = "Operator"
STYLE_IDENTIFIER = "Identifier"

STRING_STYLES = [STYLE_STRING, STYLE_SQSTRING, STYLE_TQSSTRING, STYLE_TQDSTRING]

# the default font tuples to use for Python coloring
classfmt	= (0, 1, 200, 0, 16711680) 
keywordfmt	= (0, 1, 200, 0, 8388608)
cmntfmt	= (0, 2, 200, 0, 32768)
quotefmt	= (0, 0, 200, 0, 32896)
nmberfmt	= (0, 0, 200, 0, 8421376)
methodfmt	= (0, 1, 200, 0, 8421376)
dfltfmt	= (0, 0, 200, 0, 8421504)
opfmt	= (0, 1, 200, 0, 0)
idfmt		= (0, 0, 200, 0, 0)

class PythonSourceFormatter(Formatter):
	def __init__(self, scintilla):
		Formatter.__init__(self, scintilla)
		self.SetStyles()

	def GetSampleText(self):
		return "class Sample(Super):\n  def Fn(self):\n    # A bitOPy\n    dest = 'dest.html'\n    timeOut = 1024\n\ta = a + 1\n"

	def LoadStyles(self):
		pass

	def SetStyles(self):
		self.RegisterStyle( Style(STYLE_DEFAULT, dfltfmt ) )
		self.RegisterStyle( Style(STYLE_COMMENT, cmntfmt ) )
		self.RegisterStyle( Style(STYLE_NUMBER, nmberfmt ) )
		self.RegisterStyle( Style(STYLE_STRING, quotefmt ) )
		self.RegisterStyle( Style(STYLE_SQSTRING, STYLE_STRING ) )
		self.RegisterStyle( Style(STYLE_TQSSTRING, STYLE_STRING ) )
		self.RegisterStyle( Style(STYLE_TQDSTRING, STYLE_STRING ) )
		self.RegisterStyle( Style(STYLE_KEYWORD, keywordfmt ) )
		self.RegisterStyle( Style(STYLE_CLASS, classfmt ) )
		self.RegisterStyle( Style(STYLE_METHOD, methodfmt ) )
		self.RegisterStyle( Style(STYLE_OPERATOR, opfmt ) )
		self.RegisterStyle( Style(STYLE_IDENTIFIER, idfmt ) )

	def GetStringStyle(self, pos):
		style = self.styles_by_id[self.scintilla.SCIGetStyleAt(pos)]
		if style.name in STRING_STYLES:
			return style
		return None

	def ClassifyWord(self, cdoc, start, end, prevWord):
		word = cdoc[start:end+1]
		attr = STYLE_IDENTIFIER
		if prevWord == "class":
			attr = STYLE_CLASS
		elif prevWord == "def":
			attr = STYLE_METHOD
		elif cdoc[start] in string.digits:
			attr = STYLE_NUMBER
		elif iskeyword(word):
			attr = STYLE_KEYWORD
		self.ColorSeg(start, end, attr)
		return word

	def ColorizeString(self, str, styleStart):
		if styleStart is None: styleStart = STYLE_DEFAULT
		return self.ColorizePythonCode(str, 0, styleStart)

	def ColorizePythonCode(self, cdoc, charStart, styleStart):
		# Straight translation of C++, should do better
		lengthDoc = len(cdoc)
		if lengthDoc <= charStart: return
		prevWord = ""
		state = styleStart
		chPrev = chPrev2 = chPrev3 = ' '
		chNext = cdoc[charStart]
		chNext2 = cdoc[charStart]
		startSeg = i = charStart
		while i < lengthDoc:
			ch = chNext
			chNext = ' '
			if i+1 < lengthDoc: chNext = cdoc[i+1]
			chNext2 = ' '
			if i+2 < lengthDoc: chNext2 = cdoc[i+2]
			if state == STYLE_DEFAULT:
				if ch in wordstarts:
					self.ColorSeg(startSeg, i - 1, STYLE_DEFAULT)
					state = STYLE_KEYWORD
					startSeg = i
				elif ch == '#':
					self.ColorSeg(startSeg, i - 1, STYLE_DEFAULT)
					state = STYLE_COMMENT
					startSeg = i
				elif ch == '\"':
					self.ColorSeg(startSeg, i - 1, STYLE_DEFAULT)
					startSeg = i
					state = STYLE_COMMENT
					if chNext == '\"' and chNext2 == '\"':
						i = i + 2
						state = STYLE_TQDSTRING
						ch = ' '
						chPrev = ' '
						chNext = ' '
						if i+1 < lengthDoc: chNext = cdoc[i+1]
					else:
						state = STYLE_STRING
				elif ch == '\'':
					self.ColorSeg(startSeg, i - 1, STYLE_DEFAULT)
					startSeg = i
					state = STYLE_COMMENT
					if chNext == '\'' and chNext2 == '\'':
						i = i + 2
						state = STYLE_TQSSTRING
						ch = ' '
						chPrev = ' '
						chNext = ' '
						if i+1 < lengthDoc: chNext = cdoc[i+1]
					else:
						state = STYLE_SQSTRING
				elif ch in operators:
					self.ColorSeg(startSeg, i - 1, STYLE_DEFAULT)
					self.ColorSeg(i, i, STYLE_OPERATOR)
					startSeg = i+1
			elif state == STYLE_KEYWORD:
				if ch not in wordchars:
					prevWord = self.ClassifyWord(cdoc, startSeg, i-1, prevWord)
					state = STYLE_DEFAULT
					startSeg = i
					if ch == '#':
						state = STYLE_COMMENT
					elif ch == '\"':
						if chNext == '\"' and chNext2 == '\"':
							i = i + 2
							state = STYLE_TQDSTRING
							ch = ' '
							chPrev = ' '
							chNext = ' '
							if i+1 < lengthDoc: chNext = cdoc[i+1]
						else:
							state = STYLE_STRING
					elif ch == '\'':
						if chNext == '\'' and chNext2 == '\'':
							i = i + 2
							state = STYLE_TQSSTRING
							ch = ' '
							chPrev = ' '
							chNext = ' '
							if i+1 < lengthDoc: chNext = cdoc[i+1]
						else:
							state = STYLE_SQSTRING
					elif ch in operators:
						self.ColorSeg(startSeg, i, STYLE_OPERATOR)
						startSeg = i+1
			elif state == STYLE_COMMENT:
				if ch == '\r' or ch == '\n':
					self.ColorSeg(startSeg, i-1, STYLE_COMMENT)
					state = STYLE_DEFAULT
					startSeg = i
			elif state == STYLE_STRING:
				if ch == '\\':
					if chNext == '\"' or chNext == '\'' or chNext == '\\':
						i = i + 1
						ch = chNext
						chNext = ' '
						if i+1 < lengthDoc: chNext = cdoc[i+1]
				elif ch == '\"':
					self.ColorSeg(startSeg, i, STYLE_STRING)
					state = STYLE_DEFAULT
					startSeg = i+1
			elif state == STYLE_SQSTRING:
				if ch == '\\':
					if chNext == '\"' or chNext == '\'' or chNext == '\\':
						i = i+1
						ch = chNext
						chNext = ' '
						if i+1 < lengthDoc: chNext = cdoc[i+1]
				elif ch == '\'':
					self.ColorSeg(startSeg, i, STYLE_SQSTRING)
					state = STYLE_DEFAULT
					startSeg = i+1
			elif state == STYLE_TQSSTRING:
				if ch == '\'' and chPrev == '\'' and chPrev2 == '\'' and chPrev3 != '\\':
					self.ColorSeg(startSeg, i, STYLE_TQSSTRING)
					state = STYLE_DEFAULT
					startSeg = i+1
			elif state == STYLE_TQDSTRING and ch == '\"' and chPrev == '\"' and chPrev2 == '\"' and chPrev3 != '\\':
					self.ColorSeg(startSeg, i, STYLE_TQDSTRING)
					state = STYLE_DEFAULT
					startSeg = i+1
			chPrev3 = chPrev2
			chPrev2 = chPrev
			chPrev = ch
			i = i + 1
		if startSeg < lengthDoc:
			if state == STYLE_KEYWORD:
				self.ClassifyWord(cdoc, startSeg, lengthDoc-1, prevWord)
			else:
				self.ColorSeg(startSeg, lengthDoc-1, state)
