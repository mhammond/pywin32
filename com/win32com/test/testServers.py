import pythoncom, win32com.client.dynamic, sys
from util import CheckClean

def TestInterp(interp):
	if interp.Eval("1+1") <> 2:
		raise ValueError, "The interpreter returned the wrong result."
	try:
		interp.Eval(1+1)
		raise ValueError, "The interpreter did not raise an exception"
	except pythoncom.com_error, details:
		import winerror
		if details[0]!=winerror.DISP_E_TYPEMISMATCH:
			raise ValueError, "The interpreter exception was not winerror.DISP_E_TYPEMISMATCH."

def TestConnections():
	import win32com.demos.connect
	win32com.demos.connect.test()

def TestAllInterps():
	numInterps = 0
	try:
		interp = win32com.client.dynamic.Dispatch("Python.Interpreter")
	except pythoncom.com_error:
		print "**** - The Python.Interpreter DLL test server is not available"
		interp = None
	if interp:
		numInterps = numInterps + 1
		TestInterp(interp)
	
	try:
		interp = win32com.client.dynamic.Dispatch("Python.Interpreter", clsctx = pythoncom.CLSCTX_LOCAL_SERVER)
	except pythoncom.com_error:
		print "**** - The Python.Interpreter EXE test server is not available"
		interp = None
	if interp:
		numInterps = numInterps + 1
		TestInterp(interp)

	print "The %d available Python.Interpreter objects worked OK." % (numInterps)

def TestAll():
	TestAllInterps()
	TestConnections()
	
if __name__=='__main__':
	TestAll()
	CheckClean()

