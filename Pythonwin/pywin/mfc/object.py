# MFC base classes.
import sys
import win32ui

class Object:
	def __init__(self, initObj = None):
		self.__dict__['_obj_'] = initObj
#		self._obj_ = initObj
		if initObj is not None: initObj.AttachObject(self)
	def __del__(self):
		self.close()
	def __getattr__(self, attr):	# Make this object look like the underlying win32ui one.
		# During cleanup __dict__ is not available, causing recursive death.
		if attr != '__dict__':
			o = self.__dict__.get('_obj_')
			if o is not None:
				return getattr(o, attr)
			# Only raise this error for non "internal" names -
			# Python may be calling __len__, __nonzero__, etc, so
			# we dont want this exception
			if attr[0]!= '_' and attr[-1] != '_':
				raise win32ui.error, "The MFC object has died."
		raise AttributeError, attr

	def OnAttachedObjectDeath(self):
#		print "object", self.__class__.__name__, "dieing"
		self._obj_ = None
	def close(self):
		if self.__dict__.has_key('_obj_'):
			if self._obj_ is not None:
				self._obj_.AttachObject(None)
				self._obj_ = None

class CmdTarget(Object):
	def __init__(self, initObj):
		Object.__init__(self, initObj)
