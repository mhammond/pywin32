# Test AXScripting the best we can in an automated fashion...
import win32api, win32pipe, os, sys

def RegisterEngine(verbose = 1):
	import win32com.axscript.client
	file = win32api.GetFullPathName(os.path.join(win32com.axscript.client.__path__[0], "pyscript.py"))
	cmd = '%s "%s" > nul' % (win32api.GetModuleFileName(0), file)
	if verbose:
		print "Registering engine"
#	print cmd
	rc = os.system(cmd)
	if rc:
		print "Registration of engine failed"
	
	
def TestHost(verbose = 1):
	import win32com.axscript
	file = win32api.GetFullPathName(os.path.join(win32com.axscript.__path__[0], "Test\\TestHost.py"))
	cmd = '%s "%s" > nul' % (win32api.GetModuleFileName(0), file)
	if verbose:
		print "Testing Python Scripting host"
#	print cmd
	rc = os.system(cmd)
	if rc:
		print "Execution of TestHost failed"

def TestCScript(verbose = 1):
	import win32com.axscript
	file = win32api.GetFullPathName(os.path.join(win32com.axscript.__path__[0], "Demos\\Client\\wsh\\test.pys"))
	cmd = 'cscript.exe "%s" > nul' % (file)
	if verbose:
		print "Testing Windows Scripting host with Python script"
	rc = os.system(cmd)
	if rc:
		print "Execution of CScript failed"

def TestAll(verbose = 1):
	TestHost(verbose)
	TestCScript(verbose)
	
if __name__=='__main__':
	from util import CheckClean
	TestAll()
	CheckClean()
