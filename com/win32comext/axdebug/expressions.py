import axdebug, gateways
from util import _wrap, _wrap_remove, RaiseNotImpl
import cStringIO, traceback
from pprint import pprint
import winerror
import string
import sys

# Given an object, return a nice string
def MakeNiceString(ob):
	stream = cStringIO.StringIO()
	pprint(ob, stream)
	return string.strip(stream.getvalue())

class ProvideExpressionContexts(gateways.ProvideExpressionContexts):
	pass

class ExpressionContext(gateways.DebugExpressionContext):
	def __init__(self, frame):
		self.frame = frame
	def ParseLanguageText(self, code, radix, delim, flags):
		return _wrap(Expression(self.frame, code, radix, delim, flags), axdebug.IID_IDebugExpression)
	def GetLanguageInfo(self):
#		print "GetLanguageInfo"
		return "Python", "{DF630910-1C1D-11d0-AE36-8C0F5E000000}"
	
class Expression(gateways.DebugExpression):
	def __init__(self, frame, code, radix, delim, flags):
		self.callback = None
		self.frame = frame
		self.code = str(code)
		self.radix = radix
		self.delim = delim
		self.flags = flags
		self.isComplete = 0
		self.result=None
		self.hresult = winerror.E_UNEXPECTED
	def Start(self, callback):
		try:
			try:
				try:
					self.result = eval(self.code, self.frame.f_globals, self.frame.f_locals)
				except SyntaxError:
					exec self.code in self.frame.f_globals, self.frame.f_locals
					self.result = ""
				self.hresult = 0
			except:
				l = traceback.format_exception_only(sys.exc_info()[0], sys.exc_info()[1])
				# l is a list of strings with trailing "\n"
				self.result = string.join(map(lambda s:s[:-1], l), "\n")
				self.hresult = winerror.E_FAIL
		finally:
			self.isComplete = 1
			callback.onComplete()
	def Abort(self):
		print "** ABORT **"
		
	def QueryIsComplete(self):
		return self.isComplete
		
	def GetResultAsString(self):
#		print "GetStrAsResult returning", self.result
		return self.hresult, MakeNiceString(self.result)
	
	def GetResultAsDebugProperty(self):
		result = _wrap(DebugProperty(self.code, self.hresult, self.result), axdebug.IID_IDebugProperty)
		return self.hresult, result

# Constants missing from AXDebug
DBGPROP_INFO_NAME	= 0x1
DBGPROP_INFO_TYPE	= 0x2
DBGPROP_INFO_VALUE	= 0x4
DBGPROP_INFO_FULLNAME	= 0x20
DBGPROP_INFO_ATTRIBUTES	= 0x8
DBGPROP_INFO_DEBUGPROP	= 0x10
DBGPROP_INFO_AUTOEXPAND	= 0x8000000
		
class DebugProperty:
	_com_interfaces_ = [axdebug.IID_IDebugProperty]
	_public_methods_ = ['GetPropertyInfo', 'GetExtendedInfo', 'SetValueAsString', 
	                    'EnumMembers', 'GetParent'
	]
	def __init__(self, code, hresult, result):
		self.code = code
		self.hresult = hresult
		self.result = result
		
	def GetPropertyInfo(self, dwFieldSpec, nRadix):
		# returns a tuple
		name = typ = value = fullname = None
		if dwFieldSpec & DBGPROP_INFO_VALUE:
			value = MakeNiceString(self.result)
		if dwFieldSpec & DBGPROP_INFO_NAME:
			name = self.code
		if dwFieldSpec & DBGPROP_INFO_TYPE:
			if self.hresult:
				typ = "Error"
			else:
				try:
					typ = type(self.result).__name__
				except AttributeError:
					typ = str(type(self.result))
		if dwFieldSpec & DBGPROP_INFO_FULLNAME:
			fullname = self.code
		return name, typ, value, fullname

	def GetExtendedInfo(self): ### Note - not in the framework.
		RaiseNotImpl("DebugProperty::GetExtendedInfo")

	def SetValueAsString(self, value, radix):
		#
		RaiseNotImpl("DebugProperty::SetValueAsString")
		
	def EnumMembers(self, dwFieldSpec, nRadix, iid):
		# Returns IEnumDebugPropertyInfo
		RaiseNotImpl("DebugProperty::EnumMembers")
	
	def GetParent(self):
		# return IDebugProperty
		RaiseNotImpl("DebugProperty::GetParent")
