# Implements _both_ a connectable client, and a connectable server.
#
# Note that we cheat just a little - the Server in this demo is not created
# via Normal COM - this means we can avoid registering the server.
# However, the server _is_ accessed as a COM object - just the creation
# is cheated on - so this is still working as a fully-fledged server.

import pythoncom
import win32com.server.util
import win32com.server.connect
from win32com.server.exception import Exception

# This is the IID of the Events interface both Client and Server support.
IID_IConnectDemoEvents = pythoncom.MakeIID("{A4988850-49C3-11d0-AE5D-52342E000000}")

# The server which implements
# Create a connectable class, that has a single public method
# 'DoIt', which echos to a single sink 'DoneIt'

class ConnectableServer(win32com.server.connect.ConnectableServer):
	_public_methods_ = ["DoIt"] + win32com.server.connect.ConnectableServer._public_methods_
	_connect_interfaces_ = [IID_IConnectDemoEvents]
	# The single public method that the client can call on us
	# (ie, as a normal COM server, this exposes just this single method.
	def DoIt(self,arg):
		# Simply broadcast a notification.
		self._BroadcastNotify(self.NotifyDoneIt, (arg,))

	def NotifyDoneIt(self, interface, arg):
		interface.Invoke(1000, 0, pythoncom.DISPATCH_METHOD, 1, arg)

# Here is the client side of the connection world.
# Define a COM object which implements the methods defined by the
# IConnectDemoEvents interface.								
class ConnectableClient:
	# This is another cheat - I _know_ the server defines the "DoneIt" event
	# as DISPID==1000 - I also know from the implementation details of COM
	# that the first method in _public_methods_ gets 1000.
	# Normally some explicit DISPID->Method mapping is required.
	_public_methods_ = ["OnDoneIt"]
	# A client must implement QI, and respond to a query for the Event interface.
	# In addition, it must provide a COM object (which server.util.wrap) does.
	def _query_interface_(self, iid):
		import win32com.server.util
		# Note that this seems like a necessary hack.  I am responding to IID_IConnectDemoEvents
		# but only creating an IDispatch gateway object.
		if iid==IID_IConnectDemoEvents: return win32com.server.util.wrap(self)
	# And here is our event method which gets called.
	def OnDoneIt(self, arg):
		print "OnDoneIt with ", repr(arg)

# A simple test script for all this.
# In the real world, it is likely that the code controlling the server
# will be in the same class as that getting the notifications.
def test():
	import win32com.client.dynamic, win32com.client.connect
	import win32com.server.policy
	server = win32com.client.dynamic.Dispatch(win32com.server.util.wrap(ConnectableServer()))
	connection = win32com.client.connect.SimpleConnection()
	connection.Connect(server, ConnectableClient(),IID_IConnectDemoEvents)
	server.DoIt("Hello")
	server.DoIt("Here is a null>"+chr(0)+"<")
	# Aggressive memory leak checking (ie, do nothing!) :-)  All should cleanup OK???

if __name__=='__main__':
	test()

