# Test dynamic policy, and running object table.

import pythoncom
import winerror

from win32com.server.exception import Exception

error = "testDynamic error"

iid = pythoncom.MakeIID("{b48969a0-784b-11d0-ae71-d23f56000000}")

class VeryPermissive:
	def _dynamic_(self, name, lcid, wFlags, args):
		if wFlags & pythoncom.DISPATCH_METHOD:
			return apply(getattr(self,name),args)

		if wFlags & pythoncom.DISPATCH_PROPERTYGET:
			try:
				return self.__dict__[name]
			except KeyError: # Probably a method request.
				raise Exception(scode=winerror.DISP_E_MEMBERNOTFOUND)

		if wFlags & (pythoncom.DISPATCH_PROPERTYPUT | pythoncom.DISPATCH_PROPERTYPUTREF):
			setattr(self, name, args[0])
			return

		raise Exception(scode=winerror.E_INVALIDARG, desc="invalid wFlags")

	def write(self, *args):
		if len(args)==0:
			raise Exception(scode=winerror.DISP_E_BADPARAMCOUNT) # Probably call as PROPGET.

		for arg in args[:-1]:
			print str(arg),
		print str(args[-1])

def Test():
	import win32com.server.util, win32com.server.policy
#	import win32dbg;win32dbg.brk()
	ob = win32com.server.util.wrap(VeryPermissive(),usePolicy=win32com.server.policy.DynamicPolicy)
	handle = pythoncom.RegisterActiveObject(ob, iid, 0)
	try:
		import win32com.client.dynamic
		client = win32com.client.dynamic.Dispatch(iid)
		client.ANewAttr = "Hello"
		if client.ANewAttr != "Hello":
			raise error, "Could not set dynamic property"

		v = ["Hello","From","Python",1.4]
		client.TestSequence = v
		if v != list(client.TestSequence):
			raise error, "Dynamic sequences not working!"
			
		client.write("This","output","has","come","via","COM")

		client = None
	finally:
		pythoncom.RevokeActiveObject(handle)

if __name__=='__main__':
	Test()
