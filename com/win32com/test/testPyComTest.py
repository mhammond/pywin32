# NOTE - Still seems to be a leak here somewhere
# gateway count doesnt hit zero.  Hence the print statements!

import sys; sys.coinit_flags=0 # Must be free-threaded!
import win32api, types, pythoncom, time
import sys, win32com, win32com.client.connect
from win32com.test.util import CheckClean
from win32com.client import constants

importMsg = "**** PyCOMTest is not installed ***\n  PyCOMTest is a Python test specific COM client and server.\n  It is likely this server is not installed on this machine\n  To install the server, you must get the win32com sources\n  and build it using MS Visual C++"

error = "testPyCOMTest error"

from win32com.client import gencache
try:
	PyCOMTest = gencache.EnsureModule('{6BCDCB60-5605-11D0-AE5F-CADD4C000000}', 0, 1, 1)
except pythoncom.com_error:
	PyCOMTest = None

if PyCOMTest is None:
	print "The PyCOMTest module can not be located or generated."
	print importMsg
	raise RuntimeError, importMsg

import sys

verbose = 0

def TestApplyResult(fn, args, result):
	try:
		import string
		fnName = string.split(str(fn))[1]
	except:	
		fnName = str(fn)
	if verbose: 
		print "Testing ", fnName,
	pref = "function " + fnName
	try:
		rc  = apply(fn, args)
		if rc != result:
			raise error, "%s failed - result not %d but %d" % (pref, result, rc)
	except:
		t, v, tb = sys.exc_info()
		tb = None
		raise error, "%s caused exception %s,%s" % (pref, t, v)

	if verbose: print

	
# Simple handler class.  This demo only fires one event.
class RandomEventHandler (PyCOMTest.IPyCOMTestEvent):
	def __init__(self, oobj = None):
		PyCOMTest.IPyCOMTestEvent.__init__(self, oobj)
		self.fireds = {}
	def OnFire(self, no):
		try:
			self.fireds[no] = self.fireds[no] + 1
		except KeyError:
			self.fireds[no] = 0
	def _DumpFireds(self):
		if not self.fireds:
			print "ERROR: Nothing was recieved!"
		for firedId, no in self.fireds.items():
			if verbose:
				print "ID %d fired %d times" % (firedId, no)

def TestDynamic():
	if verbose: print "Testing Dynamic"
	import win32com.client.dynamic
	o = win32com.client.dynamic.DumbDispatch("PyCOMTest.PyCOMTest")

	if verbose: print "Getting counter"
	counter = o.GetSimpleCounter()
	TestCounter(counter, 0)

	if verbose: print "Checking default args"
	rc = o.TestOptionals()
	if  rc[:-1] != ("def", 0, 1) or abs(rc[-1]-3.14)>.01:
		print rc
		raise error, "Did not get the optional values correctly"
	rc = o.TestOptionals("Hi", 2, 3, 1.1)
	if  rc[:-1] != ("Hi", 2, 3) or abs(rc[-1]-1.1)>.01:
		print rc
		raise error, "Did not get the specified optional values correctly"
	rc = o.TestOptionals2(0)
	if  rc != (0, "", 1):
		print rc
		raise error, "Did not get the optional2 values correctly"
	rc = o.TestOptionals2(1.1, "Hi", 2)
	if  rc[1:] != ("Hi", 2) or abs(rc[0]-1.1)>.01:
		print rc
		raise error, "Did not get the specified optional2 values correctly"

#	if verbose: print "Testing structs"
	r = o.GetStruct()
	print str(r.str_value)
	assert r.int_value == 99 and str(r.str_value)=="Hello from C++"
	counter = win32com.client.dynamic.DumbDispatch("PyCOMTest.SimpleCounter")
	TestCounter(counter, 0)
	assert o.DoubleString("foo") == "foofoo"

	l=[]
	TestApplyResult(o.SetVariantSafeArray, (l,), len(l))
	l=[1,2,3,4]
	TestApplyResult(o.SetVariantSafeArray, (l,), len(l))
#	TestApplyResult(o.SetIntSafeArray, (l,), len(l))       Still fails, and probably always will.


def TestGenerated():
	# Create an instance of the server.
	o=PyCOMTest.CoPyCOMTest()
	counter = o.GetSimpleCounter()
	TestCounter(counter, 1)
	
	counter = win32com.client.Dispatch("PyCOMTest.SimpleCounter")
	TestCounter(counter, 1)
	
	i1, i2 = o.GetMultipleInterfaces()
	if type(i1) != types.InstanceType or type(i2) != types.InstanceType:
		# Yay - is now an instance returned!
		raise error,  "GetMultipleInterfaces did not return instances - got '%s', '%s'" % (i1, i2)
	del i1
	del i2

	if verbose: print "Checking default args"
	rc = o.TestOptionals()
	if  rc[:-1] != ("def", 0, 1) or abs(rc[-1]-3.14)>.01:
		print rc
		raise error, "Did not get the optional values correctly"
	rc = o.TestOptionals("Hi", 2, 3, 1.1)
	if  rc[:-1] != ("Hi", 2, 3) or abs(rc[-1]-1.1)>.01:
		print rc
		raise error, "Did not get the specified optional values correctly"
	rc = o.TestOptionals2(0)
	if  rc != (0, "", 1):
		print rc
		raise error, "Did not get the optional2 values correctly"
	rc = o.TestOptionals2(1.1, "Hi", 2)
	if  rc[1:] != ("Hi", 2) or abs(rc[0]-1.1)>.01:
		print rc
		raise error, "Did not get the specified optional2 values correctly"

	if verbose: print "Checking var args"
	o.SetVarArgs("Hi", "There", "From", "Python", 1)
	if o.GetLastVarArgs() != ("Hi", "There", "From", "Python", 1):
		raise error, "VarArgs failed -" + str(o.GetLastVarArgs())
	if verbose: print "Checking getting/passing IUnknown"
	if type(o.GetSetUnknown(o)) !=pythoncom.TypeIIDs[pythoncom.IID_IUnknown]:
		raise error, "GetSetUnknown failed"
	if verbose: print "Checking getting/passing IDispatch"
	if type(o.GetSetDispatch(o)) !=types.InstanceType:
		raise error, "GetSetDispatch failed"
	if verbose: print "Checking getting/passing IDispatch of known type"
	if o.GetSetInterface(o).__class__ != PyCOMTest.IPyCOMTest:
		raise error, "GetSetDispatch failed"

	o.GetSimpleSafeArray(None)
	TestApplyResult(o.GetSimpleSafeArray, (None,), tuple(range(10)))
	resultCheck = tuple(range(5)), tuple(range(10)), tuple(range(20))
	TestApplyResult(o.GetSafeArrays, (None, None, None), resultCheck)

	l=[1,2,3,4]
	TestApplyResult(o.SetVariantSafeArray, (l,), len(l))
	TestApplyResult(o.SetIntSafeArray, (l,), len(l))
	l=[]
	TestApplyResult(o.SetVariantSafeArray, (l,), len(l))
	TestApplyResult(o.SetIntSafeArray, (l,), len(l))
	# Tell the server to do what it does!
	TestApplyResult(o.Test, ("Unused", 99), 1) # A bool function
	TestApplyResult(o.Test, ("Unused", -1), 1) # A bool function
	TestApplyResult(o.Test, ("Unused", 1==1), 1) # A bool function
	TestApplyResult(o.Test, ("Unused", 0), 0)
	TestApplyResult(o.Test, ("Unused", 1==0), 0)
	TestApplyResult(o.Test2, (constants.Attr2,), constants.Attr2)
	TestApplyResult(o.Test3, (constants.Attr2,), constants.Attr2)
	TestApplyResult(o.Test4, (constants.Attr2,), constants.Attr2)
	TestApplyResult(o.Test5, (constants.Attr2,), constants.Attr2)

	now = pythoncom.MakeTime(time.gmtime(time.time()))
	later = pythoncom.MakeTime(time.gmtime(time.time()+1))
	TestApplyResult(o.EarliestDate, (now, later), now)

	assert o.DoubleString("foo") == "foofoo"
	assert o.DoubleInOutString("foo") == "foofoo"

	# Do the connection point thing...
	# Create a connection object.
	if verbose: print "Testing connection points"
	sessions = []
	handler = RandomEventHandler()

	try:
		for i in range(3):
			session = o.Start()
			# Create an event handler instance, and connect it to the server.
			connection = win32com.client.connect.SimpleConnection(o, handler)
			sessions.append((session, connection))

		time.sleep(.5)
	finally:
		# Stop the servers
		for session, connection in sessions:
			o.Stop(session)
			connection.Disconnect()
		if handler: handler._DumpFireds()
	if verbose: print "Finished generated .py test."

def TestCounter(counter, bIsGenerated):
	# Test random access into container
	if verbose: print "Testing counter", `counter`
	import random
	for i in xrange(50):
		num = int(random.random() * len(counter))
		try:
			ret = counter[num]
			if ret != num+1:
				raise error, "Random access into element %d failed - return was %s" % (num,`ret`)
		except IndexError:	
			raise error, "** IndexError accessing collection element %d" % num

	num = 0
	if bIsGenerated:
		counter.SetTestProperty(1)
		counter.TestProperty = 1 # Note this has a second, default arg.
		counter.SetTestProperty(1,2)
		if counter.TestPropertyWithDef != 0:
			raise error, "Unexpected property set value!"
		if counter.TestPropertyNoDef(1) != 1:
			raise error, "Unexpected property set value!"
	else:
		pass
		# counter.TestProperty = 1

	counter.LBound=1
	counter.UBound=10
	if counter.LBound <> 1 or counter.UBound<>10:
		print "** Error - counter did not keep its properties"

	if bIsGenerated:
		bounds = counter.GetBounds()
		if bounds[0]<>1 or bounds[1]<>10:
			raise error, "** Error - counter did not give the same properties back"
		counter.SetBounds(bounds[0], bounds[1])

	for item in counter:
		num = num + 1
	if num <> len(counter):
		raise error, "*** Length of counter and loop iterations dont match ***"
	if num <> 10:
		raise error, "*** Unexpected number of loop iterations ***"

	counter = counter._enum_.Clone() # Test Clone() and enum directly
	counter.Reset()
	num = 0
	for item in counter:
		num = num + 1
	if num <> 10:
		raise error, "*** Unexpected number of loop iterations - got %d ***" % num
	if verbose: print "Finished testing counter"

###############################
##
## Some vtable tests of the interface
##
def TestVTable():
	tester = win32com.client.Dispatch("PyCOMTest.PyCOMTest")
	testee = pythoncom.CoCreateInstance("Python.Test.PyCOMTest", None, pythoncom.CLSCTX_ALL, pythoncom.IID_IUnknown)
	tester.TestMyInterface(testee)

	# We once crashed creating our object with the native interface as
	# the first IID specified.  We must do it _after_ the tests, so that
	# Python has already had the gateway registered from last run.
	iid = pythoncom.InterfaceNames["IPyCOMTest"]
	clsid = "Python.Test.PyCOMTest"
	clsctx = pythoncom.CLSCTX_SERVER
	try:
		testee = pythoncom.CoCreateInstance(clsid, None, clsctx, iid)
	except TypeError:
		# Python can't actually _use_ this interface yet, so this is
		# "expected".  Any COM error is not.
		pass

	

def TestAll():
	try:
		# Make sure server installed
		import win32com.client.dynamic
		win32com.client.dynamic.DumbDispatch("PyCOMTest.PyCOMTest")
	except pythoncom.com_error:
		print importMsg
		return

	print "Testing VTables..."
	TestVTable()

	print "Testing Python COM Test Horse..."
	TestDynamic()
	TestGenerated()

if __name__=='__main__':
	# XXX - todo - Complete hack to crank threading support.
	# Should NOT be necessary
	def NullThreadFunc():
		pass
	import thread
	thread.start_new( NullThreadFunc, () )

	if "-q" not in sys.argv: verbose = 1
	TestAll()
	CheckClean()
	pythoncom.CoUninitialize()
	print "C++ test harness worked OK."

