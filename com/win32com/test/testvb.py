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
	# The new IID
	win32com.client.gencache.EnsureModule('{32C85CE8-0035-11D3-8546-204C4F4F5020}', 0, 1, 0)
except pythoncom.com_error:
	try:
		# The old IID.
		win32com.client.gencache.EnsureModule('{236C9C31-3AD6-11D2-848C-204C4F4F5020}', 0, 9, 0)
	except pythoncom.com_error:
		raise RuntimeError, importMsg

import traceback

error = "VB Test Error"

# Set up a COM object that VB will do some callbacks on.  This is used
# to test byref params for gateway IDispatch.
class TestObject:
	_public_methods_ = ["CallbackVoidOneByRef","CallbackResultOneByRef", "CallbackVoidTwoByRef", "CallbackString"]
	def CallbackVoidOneByRef(self, intVal):
		return intVal + 1
	def CallbackResultOneByRef(self, intVal):
		return intVal, intVal + 1
	def CallbackVoidTwoByRef(self, int1, int2):
		return int1+int2, int1-int2
	def CallbackString(self, strVal):
		return 0, strVal + " has visited Python"


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

	# Try and use a safe array (note that the VB code has this declared as a VARIANT
	# and I cant work out how to force it to use native arrays!
	# (NOTE Python will convert incoming arrays to tuples, so we pass a tuple, even tho
	# a list works fine - just makes it easier for us to compare the result!
	arrayData = tuple(range(1,100))
	vbtest.ArrayProperty = arrayData
	if vbtest.ArrayProperty != arrayData:
		raise error, "Could not set the array data correctly - got back " + str(vbtest.ArrayProperty)

	# Python doesnt support PUTREF properties without a typeref
	# (although we could)
	if bUseGenerated:
		# A property that only has PUTREF defined.
		vbtest.VariantPutref = vbtest
		if vbtest.VariantPutref._oleobj_!= vbtest._oleobj_:
			raise error, "Could not set the VariantPutref property correctly."
		# Cant test further types for this VariantPutref, as only
		# COM objects can be stored ByRef.

		# A "set" type property - only works for generated.
		print "Skipping CollectionProperty - dont know how to make"
		print " VB recognize an object as a collection"
#		vbtest.CollectionProperty = NewCollection((1,2,"3", "Four"))
#		if vbtest.CollectionProperty != (1,2,"3", "Four"):
#			raise error, "Could not set the Collection property correctly - got back " + str(vbtest.CollectionProperty)

		# This one is a bit strange!  The array param is "ByRef", as VB insists.
		# The function itself also _returns_ the arram param.
		# Therefore, Python sees _2_ result values - one for the result,
		# and one for the byref.
		testData = string.split("Mark was here")
		resultData, byRefParam = vbtest.PassSAFEARRAY(testData)
		# Un unicode everything.
		resultData = map(str, resultData)
		byRefParam = map(str, byRefParam)
		if testData != list(resultData):
			raise error, "The safe array data was not what we expected - got " + str(resultData)
		if testData != list(byRefParam):
			raise error, "The safe array data was not what we expected - got " + str(byRefParam)

		if vbtest.IncrementIntegerParam(1) != 2:
			raise error, "Could not pass an integer byref"

		if vbtest.IncrementVariantParam(1) != 2:
			raise error, "Could not pass an int VARIANT byref:"+str(vbtest.IncrementVariantParam(1))

		if vbtest.IncrementVariantParam(1.5) != 2.5:
			raise error, "Could not pass a float VARIANT byref"
		

		vbtest.DoSomeCallbacks(wrap(TestObject()))

	ret = vbtest.PassIntByVal(1)
	if ret != 2:
		raise error, "Could not increment the integer - "+str(ret)

	# Python doesnt support byrefs without some sort of generated support.
	if bUseGenerated:
		ret = vbtest.PassIntByRef(1)
		if ret != (1,2):
			raise error, "Could not increment the integer - "+str(ret)
		# Check you can leave a byref arg blank.
		ret = vbtest.PassIntByRef()
		if ret != (0,1):
			raise error, "Could not increment the integer with default arg- "+str(ret)

	try:
		vbtest.IntProperty = "One"
	except pythoncom.com_error, (hr, desc, exc, argErr):
		if hr != winerror.DISP_E_TYPEMISMATCH:
			raise error, "Expected DISP_E_TYPEMISMATCH"

def DoTestAll():
	o = win32com.client.Dispatch("PyCOMVBTest.Tester")
	TestVB(o,1)

	o = win32com.client.dynamic.DumbDispatch("PyCOMVBTest.Tester")
	TestVB(o,0)
		
def TestAll():
	try:
		DoTestAll()
	except:
		traceback.print_exc()

if __name__=='__main__':
	from util import CheckClean
	TestAll()
	CheckClean()

	pythoncom.CoUninitialize()
