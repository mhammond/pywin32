# Test code for a VB Program.
#
# This requires the PythonCOM VB Test Harness.
#

import winerror
import pythoncom, win32com.client, win32com.client.dynamic, win32com.client.gencache
from win32com.server.util import NewCollection, wrap
import string

importMsg = """\
**** VB Test harness is not installed ***
  This test requires a VB test program to be built and installed
  on this PC.
"""

### NOTE: VB SUCKS!
### If you delete the DLL built by VB, then reopen VB
### to rebuild the DLL, it loses the IID of the object!!!
### So I will try to avoid this in the future :-)

# Import the type library for the test module.
try:
	win32com.client.gencache.EnsureModule('{32C85CE8-0035-11D3-8546-204C4F4F5020}', 0, 5, 0)
except pythoncom.com_error:
	raise RuntimeError, importMsg

import traceback

error = "VB Test Error"

# Set up a COM object that VB will do some callbacks on.  This is used
# to test byref params for gateway IDispatch.
class TestObject:
	_public_methods_ = ["CallbackVoidOneByRef","CallbackResultOneByRef", "CallbackVoidTwoByRef",
					    "CallbackString","CallbackResultOneByRefButReturnNone",
					    "CallbackVoidOneByRefButReturnNone",
					    "CallbackArrayResult", "CallbackArrayResultOneArrayByRef",
					    "CallbackArrayResultWrongSize"
					   ]
	def CallbackVoidOneByRef(self, intVal):
		return intVal + 1
	def CallbackResultOneByRef(self, intVal):
		return intVal, intVal + 1
	def CallbackVoidTwoByRef(self, int1, int2):
		return int1+int2, int1-int2
	def CallbackString(self, strVal):
		return 0, strVal + " has visited Python"
	def CallbackArrayResult(self, arrayVal):
		ret = []
		for i in arrayVal:
			ret.append(i+1)
		# returning as a list forces it be processed as a single result
		# (rather than a tuple, where it may be interpreted as
		# multiple results for byref unpacking)
		return ret
	def CallbackArrayResultWrongSize(self, arrayVal):
		return list(arrayVal[:-1])
	def CallbackArrayResultOneArrayByRef(self, arrayVal):
		ret = []
		for i in arrayVal:
			ret.append(i+1)
		# See above for list processing.
		return list(arrayVal), ret
	
	def CallbackResultOneByRefButReturnNone(self, intVal):
		return
	def CallbackVoidOneByRefButReturnNone(self, intVal):
		return

def TestVB( vbtest, bUseGenerated ):
	vbtest.LongProperty = -1
	if vbtest.LongProperty != -1:
		raise error, "Could not set the long property correctly."
	vbtest.IntProperty = 10
	if vbtest.IntProperty != 10:
		raise error, "Could not set the integer property correctly."
	vbtest.VariantProperty = 10
	if vbtest.VariantProperty != 10:
		raise error, "Could not set the variant integer property correctly."
	vbtest.StringProperty = "Hello from Python"
	if vbtest.StringProperty != "Hello from Python":
		raise error, "Could not set the string property correctly."
	vbtest.VariantProperty = "Hello from Python"
	if vbtest.VariantProperty != "Hello from Python":
		raise error, "Could not set the variant string property correctly."
	vbtest.VariantProperty = (1.0, 2.0, 3.0)
	if vbtest.VariantProperty != (1.0, 2.0, 3.0):
		raise error, "Could not set the variant property to an array of floats correctly - '%s'." % (vbtest.VariantProperty,)
	

	# Try and use a safe array (note that the VB code has this declared as a VARIANT
	# and I cant work out how to force it to use native arrays!
	# (NOTE Python will convert incoming arrays to tuples, so we pass a tuple, even tho
	# a list works fine - just makes it easier for us to compare the result!
	arrayData = tuple(range(1,100))
	vbtest.ArrayProperty = arrayData
	if vbtest.ArrayProperty != arrayData:
		raise error, "Could not set the array data correctly - got back " + str(vbtest.ArrayProperty)
	# Floats
	arrayData = (1.0, 2.0, 3.0)
	vbtest.ArrayProperty = arrayData
	assert vbtest.ArrayProperty == arrayData, "Could not set the array data correctly - got back '%s'" % (vbtest.ArrayProperty,)
	# Strings.
	arrayData = tuple(string.split("Hello from Python"))
	vbtest.ArrayProperty = arrayData
	assert vbtest.ArrayProperty == arrayData, "Could not set the array data correctly - got back '%s'" % (vbtest.ArrayProperty,)
	# Date and Time?
	# COM objects.
	arrayData = (vbtest, vbtest)
	vbtest.ArrayProperty = arrayData
	assert vbtest.ArrayProperty == arrayData, "Could not set the array data correctly - got back '%s'" % (vbtest.ArrayProperty,)
	# Mixed
	arrayData = (1, 2.0, "3")
	vbtest.ArrayProperty = arrayData
	assert vbtest.ArrayProperty == arrayData, "Could not set the array data correctly - got back '%s'" % (vbtest.ArrayProperty,)

	TestStructs(vbtest)

	assert vbtest.TakeByValObject(vbtest)==vbtest

	# Python doesnt support PUTREF properties without a typeref
	# (although we could)
	if bUseGenerated:
		ob = vbtest.TakeByRefObject(vbtest)
		assert ob[0]==vbtest and ob[1]==vbtest

		# A property that only has PUTREF defined.
		vbtest.VariantPutref = vbtest
		if vbtest.VariantPutref._oleobj_!= vbtest._oleobj_:
			raise error, "Could not set the VariantPutref property correctly."
		# Cant test further types for this VariantPutref, as only
		# COM objects can be stored ByRef.

		# A "set" type property - only works for generated.
		print "Skipping CollectionProperty - how does VB recognize a collection object??"
#		vbtest.CollectionProperty = NewCollection((1,2,"3", "Four"))
#		if vbtest.CollectionProperty != (1,2,"3", "Four"):
#			raise error, "Could not set the Collection property correctly - got back " + str(vbtest.CollectionProperty)

		# This one is a bit strange!  The array param is "ByRef", as VB insists.
		# The function itself also _returns_ the arram param.
		# Therefore, Python sees _2_ result values - one for the result,
		# and one for the byref.
		testData = string.split("Mark was here")
		resultData, byRefParam = vbtest.PassSAFEARRAY(testData)
		# Un unicode everything (only 1.5.2)
		try:
			unicode
		except NameError : # No builtin named Unicode!
			resultData = map(str, resultData)
			byRefParam = map(str, byRefParam)
		if testData != list(resultData):
			raise error, "The safe array data was not what we expected - got " + str(resultData)
		if testData != list(byRefParam):
			raise error, "The safe array data was not what we expected - got " + str(byRefParam)
		testData = [1.0, 2.0, 3.0]
		resultData, byRefParam = vbtest.PassSAFEARRAYVariant(testData)
		assert testData == list(byRefParam)
		assert testData == list(resultData)
		testData = ["hi", "from", "Python"]
		resultData, byRefParam = vbtest.PassSAFEARRAYVariant(testData)
		# Seamless Unicode only in 1.6!
		try:
			unicode
		except NameError : # No builtin named Unicode!
			byRefParam = map(str, byRefParam)
			resultData = map(str, resultData)
		assert testData == list(byRefParam), "Expected '%s', got '%s'" % (testData, list(byRefParam))
		assert testData == list(resultData), "Expected '%s', got '%s'" % (testData, list(resultData))
		# This time, instead of an explicit str() for 1.5, we just
		# pass Unicode, so the result should compare equal
		testData = [1, 2.0, pythoncom.Unicode("3")]
		resultData, byRefParam = vbtest.PassSAFEARRAYVariant(testData)
		assert testData == list(byRefParam)
		assert testData == list(resultData)

		# These are sub's that have a single byref param
		# Result should be just the byref.
		if vbtest.IncrementIntegerParam(1) != 2:
			raise error, "Could not pass an integer byref"

		if vbtest.IncrementIntegerParam() != 1:
			raise error, "Could not pass an omitted integer byref"

		if vbtest.IncrementVariantParam(1) != 2:
			raise error, "Could not pass an int VARIANT byref:"+str(vbtest.IncrementVariantParam(1))

		if vbtest.IncrementVariantParam(1.5) != 2.5:
			raise error, "Could not pass a float VARIANT byref"

		# Can't test IncrementVariantParam with the param omitted as it
		# it not declared in the VB code as "Optional"
		useDispatcher = None
##		import win32com.server.dispatcher
##		useDispatcher = win32com.server.dispatcher.DefaultDebugDispatcher
		callback_ob = wrap(TestObject(), useDispatcher = useDispatcher)
		vbtest.DoSomeCallbacks(callback_ob)

		# Check we fail gracefully for byref safearray results with incorrect size.
		try:
			vbtest.DoCallbackSafeArraySizeFail(callback_ob)
		except pythoncom.com_error, (hr, msg, exc, arg):
			assert exc[1] == "Python COM Server Internal Error", "Didnt get the correct exception - '%s'" % (exc,)

	ret = vbtest.PassIntByVal(1)
	if ret != 2:
		raise error, "Could not increment the integer - "+str(ret)

	# Python doesnt support byrefs without some sort of generated support.
	if bUseGenerated:
		# This is a VB function that takes a single byref
		# Hence 2 return values - function and byref.
		ret = vbtest.PassIntByRef(1)
		if ret != (1,2):
			raise error, "Could not increment the integer - "+str(ret)
		# Check you can leave a byref arg blank.
		ret = vbtest.PassIntByRef()
		if ret != (0,1):
			raise error, "Could not increment the integer with default arg- "+str(ret)

def TestStructs(vbtest):
	try:
		vbtest.IntProperty = "One"
	except pythoncom.com_error, (hr, desc, exc, argErr):
		if hr != winerror.DISP_E_TYPEMISMATCH:
			raise error, "Expected DISP_E_TYPEMISMATCH"

	s = vbtest.StructProperty
	if s.int_val != 99 or str(s.str_val) != "hello":
		raise error, "The struct value was not correct"
	s.str_val = "Hi from Python"
	s.int_val = 11
	if s.int_val != 11 or str(s.str_val) != "Hi from Python":
		raise error, "The struct value didnt persist!"
	
	if s.sub_val.int_val != 66 or str(s.sub_val.str_val) != "sub hello":
		raise error, "The sub-struct value was not correct"
	sub = s.sub_val
	sub.int_val = 22
	if sub.int_val != 22:
		print sub.int_val
		raise error, "The sub-struct value didnt persist!"
		
	if s.sub_val.int_val != 22:
		print s.sub_val.int_val
		raise error, "The sub-struct value (re-fetched) didnt persist!"

	if s.sub_val.array_val[0].int_val != 0 or str(s.sub_val.array_val[0].str_val) != "zero":
		print s.sub_val.array_val[0].int_val
		raise error, "The array element wasnt correct"
	s.sub_val.array_val[0].int_val = 99
	s.sub_val.array_val[1].int_val = 66
	if s.sub_val.array_val[0].int_val != 99 or \
	   s.sub_val.array_val[1].int_val != 66:
		print s.sub_val.array_val[0].int_val
		raise error, "The array element didnt persist."
	# Now pass the struct back to VB
	vbtest.StructProperty = s
	# And get it back again
	s = vbtest.StructProperty
	if s.int_val != 11 or str(s.str_val) != "Hi from Python":
		raise error, "After sending to VB, the struct value didnt persist!"
	if s.sub_val.array_val[0].int_val != 99:
		raise error, "After sending to VB, the struct array value didnt persist!"

	# Now do some object equality tests.
	assert s==s
	assert s != s.sub_val
	import copy
	s2 = copy.copy(s)
	assert s is not s2
	assert s == s2
	s2.int_val = 123
	assert s != s2
	# Make sure everything works with functions
	s2 = vbtest.GetStructFunc()
	assert s==s2
	vbtest.SetStructSub(s2)

	# Create a new structure, and set its elements.
	s = win32com.client.Record("VBStruct", vbtest)
	assert s.int_val == 0, "new struct inst initialized correctly!"
	s.int_val = -1
	vbtest.SetStructSub(s)
	assert vbtest.GetStructFunc().int_val == -1, "new struct didnt make the round trip!"
	# Finally, test stand-alone structure arrays.
	s_array = vbtest.StructArrayProperty
	assert s_array is None, "Expected None from the uninitialized VB array"
	vbtest.MakeStructArrayProperty(3)
	s_array = vbtest.StructArrayProperty
	assert len(s_array)==3
	for i in range(len(s_array)):
		assert s_array[i].int_val == i
		assert s_array[i].sub_val.int_val == i
		assert s_array[i].sub_val.array_val[0].int_val == i
		assert s_array[i].sub_val.array_val[1].int_val == i+1
		assert s_array[i].sub_val.array_val[2].int_val == i+2

	# Some error type checks.
	try:
		s.bad_attribute
		raise RuntimeError, "Could get a bad attribute"
	except AttributeError:
		pass
	m = s.__members__
	assert m[0]=="int_val" and m[1]=="str_val" and m[2]=="ob_val" and m[3]=="sub_val"
		
	# NOTE - a COM error is _not_ acceptable here!
	print "Struct/Record tests passed"

def DoTestAll():
	o = win32com.client.Dispatch("PyCOMVBTest.Tester")
	TestVB(o,1)

	o = win32com.client.dynamic.DumbDispatch("PyCOMVBTest.Tester")
	TestVB(o,0)
		
def TestAll():
	if not __debug__:
		raise RuntimeError, "This must be run in debug mode - we use assert!"
	try:
		DoTestAll()
		print "All tests appear to have worked!"
	except:
		traceback.print_exc()

if __name__=='__main__':
	from util import CheckClean
	TestAll()
	CheckClean()

	pythoncom.CoUninitialize()
