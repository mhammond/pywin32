"""Utilities for working with Connections"""
import win32com.server.util, pythoncom

class SimpleConnection:
	"A simple, single connection object"
	def __init__(self, coInstance = None, eventInstance = None, eventCLSID = None):
		self.cp = None
		self.cookie = None
		if not coInstance is None:
			self.Connect(coInstance , eventInstance, eventCLSID)
	
	def __del__(self):
		self.Disconnect()

	def _wrap(self, obj):
		return win32com.server.util.wrap(obj)

	def Connect(self, coInstance, eventInstance, eventCLSID = None, dispatcher = None):
		try:
			oleobj = coInstance._oleobj_
		except AttributeError:
			oleobj = coInstance
		cpc=oleobj.QueryInterface(pythoncom.IID_IConnectionPointContainer)
		if eventCLSID is None: eventCLSID = eventInstance.CLSID
		comEventInstance = self._wrap(eventInstance)
		self.cp=cpc.FindConnectionPoint(eventCLSID)
		self.cookie = self.cp.Advise(comEventInstance)

	def Disconnect(self):
		if not self.cp is None:
			if self.cookie:
				self.cp.Unadvise(self.cookie)
				self.cookie = None
			self.cp = None		
