# testDictionary.py
#

import win32com.server.util
import win32com.client
import traceback
import pythoncom
import pywintypes
import winerror
L=pywintypes.Unicode


error = "dictionary test error"

def MakeTestDictionary():
	return win32com.client.Dispatch("Python.Dictionary")

def TestDictAgainst(dict,check):
	for key, value in check.items():
		if dict(key) != value:
			raise error, "Indexing for '%s' gave the incorrect value - %s/%s" % (`key`, `dict[key]`, `check[key]`)


def TestDict(quiet=0):
	if not quiet: print "Simple enum test"
	dict = MakeTestDictionary()
	checkDict = {}
	TestDictAgainst(dict, checkDict)

	dict["NewKey"] = "NewValue"
	checkDict["NewKey"] = "NewValue"
	TestDictAgainst(dict, checkDict)

	dict["NewKey"] = None
	del checkDict["NewKey"]
	TestDictAgainst(dict, checkDict)

	try:
		print dict
		raise error, "default method with no args worked when it shouldnt have!"
	except pythoncom.com_error, (hr, desc, exc, argErr):
		if hr != winerror.DISP_E_BADPARAMCOUNT:
			raise error, "Expected DISP_E_BADPARAMCOUNT - got %d (%s)" % (hr, desc)

def doit():
	try:
		TestDict()
	except:
		traceback.print_exc()
		
if __name__=='__main__':
	doit()
	print "Worked OK with %d/%d" % (pythoncom._GetInterfaceCount(), pythoncom._GetGatewayCount())

