g_debugging = 0

import os, string, sys
import py_compile
import traceback
import cStringIO

if g_debugging:
	# Sends debugging output somewhere useful
	import win32traceutil

try:
	import pywise
except ImportError:
	print "This is a module used by the WISE installer."
	print "It will not operate stand-alone"
	raise


# List of COM Servers to be registered by the Win32 stuff		
com_servers = [\
	("AXScript Engine", "win32com.axscript.client.pyscript", "Register", []),
	("Python Interpreter", "win32com.servers.interp", "Register", []),
	("Python Dictionary", "win32com.servers.dictionary", "Register", []),
]

class Cancelled(KeyboardInterrupt):
	"""Exception raised when the user hits the Cancel button
	"""

#############################################
#
# Entry points from the installer
#
#

# One used by the Test script, and runs the test and demo code.
def TestInstall( hWnd, runMode, logFile, strArg ):
	return ApplyEntryPoint(DoTestInstall, (hWnd, runMode, logFile, strArg))

def DoTestInstall( hWnd, runMode, logFile, strArg ):
	try:
		pywise.ProgressInit()
		for args in com_servers:
			apply (RegisterCOMServer, args)

		TestProgress()
	except Cancelled:
		# Just for fun!
		pywise.SetVariable("DISPLAY_MSG", "Cancelled")

# One that compiles all .py files on the strArg
def CompileAll( hWnd, runMode, logFile, strArg ):
	return ApplyEntryPoint(DoCompileAll, (hWnd, runMode, logFile, strArg))

def DoCompileAll( hWnd, runMode, logFile, strArg ):
	pywise.ProgressInit()
	pywise.ProgressSetText("Building installed file list...")
	
	names = []
	if strArg:
		for name in string.split(strArg, ';'):
			names = names + build_dir(name)
	ticks = len(names)
	pywise.ProgressSetRange(0, ticks+1)
	compile_list(names)
	pywise.ProgressSetText("Done")

# One that compiles all .py files on the strArg, and registers COM Servers
def CompileAllAndRegisterCOM( hWnd, runMode, logFile, strArg ):
	return ApplyEntryPoint(DoCompileAllAndRegisterCOM, (hWnd, runMode, logFile, strArg))

def DoCompileAllAndRegisterCOM( hWnd, runMode, logFile, strArg ):
	pywise.ProgressInit()
	pywise.ProgressSetText("Building installed file list...")
	
	names = []
	if strArg:
		args = string.split(strArg, '|')
		for dupCheck in string.split(args[0], ';'):
			mod, dir = string.split(dupCheck,"=")
			FindDuplicates(mod, dir)

		for name in string.split(args[1], ';'):
			names = names + build_dir(name)
	ticks = len(names) + (len(com_servers) * 3) # 3 ticks per server rego.
	pywise.ProgressSetRange(0, ticks+1)
	compile_list(names)
	for args in com_servers:
		apply (RegisterCOMServer, args)
	pywise.ProgressSetText("Done")


#############################################
#
# Entry point utilities
#
#
def ApplyEntryPoint(ep, args):
	try:
		apply(ep, args)
	except Cancelled:
		pass
	except:
		if g_debugging:
			traceback.print_exc()
		f = cStringIO.StringIO()
		traceback.print_exc(file=f)
		msg = f.getvalue()
		msg = "Error in installer script.\r\n%s" % (msg)
		pywise.MessageBox(msg)

#############################################
#
# Test / Demo code
#
#
def TestProgress():
	import time
	sys32 = pywise.GetVariable("SYS32")
	pywise.ProgressSetRange(0, 100)
	pywise.ProgressSetStep(1)
	for i in range(100):
		Progress("Hello number %d at %s" % (i, sys32))
		time.sleep(.1)
	pywise.ProgressDone()

###############################################
#
# Utility functions
#

# Utility to update progress, and check for cancelled.
def Progress(msg = None):
	ok = pywise.ProgressStepIt(msg)
	if not ok:
		raise Cancelled()

# Find duplicate files of the same name already installed
# Takes a list of fileNames, and a list of directories
# we expect the file to exist in (only in one of them!)
def FindDuplicates(fileName, okDir):
	import win32api
	okDir = string.lower(win32api.GetFullPathName(okDir))
	search_path = []
	for path in sys.path:
		try:
			path = string.lower(win32api.GetFullPathName(path))
		except win32api.error:
			continue
		if path != okDir:
			search_path.append(path)

	# Now do the search.
	exts = ['.py', '.pyc', '.pyo', '.pyd']
	for path in search_path:
		found = []
		for ext in exts:
			thisName = os.path.join( path, fileName ) + ext
			if os.path.isfile( thisName ):
				found.append(thisName)
		if found:
			msg = "A duplicate Python module was located on this\n" \
                              "machine.  If this file is not renamed, the\n" \
                              "installed extensions will not function correctly.\n\n" \
                              "%s\n\n" \
                              "Would you like the existing module renamed?" % (found[0])
			rc = pywise.MessageBox(msg, "Duplicate Module Found", pywise.MB_YESNO )
			if rc == pywise.IDYES:
				for thisName in found:
					p, n = os.path.split(thisName)
					newName = os.path.join(p, "_" + n)
					try:
						os.rename(thisName, newName)
					except os.error:
						msg = "%s could not be renamed.  Please locate and remove this file manually.\n\nI did try!" % thisName
						pywise.MessageBox(msg)

###############################################
#
# COM registration  functions
#
def WriteCOMUninstallToLog(info):
	wise_roots = { \
		pywise.HKEY_CLASSES_ROOT : 0,
		pywise.HKEY_CURRENT_USER : 1,
		pywise.HKEY_LOCAL_MACHINE : 2,
		pywise.HKEY_USERS : 3,
	}
	for data in info:
		key = data[0]
		if len(data)>1:
			root = data[1]
		else:
			root = pywise.HKEY_CLASSES_ROOT
		try:
			wiseRoot = wise_roots[root]
		except KeyError:
			print "** Invalid key in uninstall support - ignored"
			continue
		pywise.WriteToLog("RegDB TREE: %s\r\nRegDB Root: %d\r\n" % (key, wiseRoot))

def RegisterCOMServer(desc, module, function, cmdlineList):
	Progress("Registering COM Server " + desc)
	try:
		mod = __import__(module)
		Progress()
		for attr in string.split(module, ".")[1:]:
			mod = getattr(mod, attr)
		fn = getattr(mod, function)
		sys.argv = ['Wise Installer']+cmdlineList
		fn()
		Progress()
		sys.argv = ['Wise Installer', '--unregister_info']+cmdlineList
		ret = fn()
		if ret:
			WriteCOMUninstallToLog(ret)
		else:
			pywise.MessageBox("Warning - COM server %s did not provide uninstall information" % desc)
	except:
		if g_debugging:
			traceback.print_exc()
		msg = "Registration of the %s COM server failed.\r\n" \
		      "Installation will continue, but this server will require manual registration before it will function\r\n\r\n"\
		      "%s: %s" % (desc, sys.exc_info()[0], sys.exc_info()[1])
		pywise.MessageBox(msg)

	
#######################################################
#
# CompileAll related code
#

# Build names of files to update.
def build_dir(dir, maxlevels = 10):
	ret = []
#	print 'Listing', dir, '...'
	try:
		names = os.listdir(dir)
	except os.error:
		print "Can't list", dir
		names = []
	names.sort()
	for name in names:
		fullname = os.path.join(dir, name)
		if os.path.isfile(fullname):
			head, tail = name[:-3], name[-3:]
			if tail == '.py':
#				print 'Flagging compile of', fullname, '...'
				ret.append(fullname)

		elif maxlevels > 0 and \
		     name != os.curdir and name != os.pardir and \
		     os.path.isdir(fullname) and \
		     not os.path.islink(fullname):
			ret = ret + build_dir(fullname, maxlevels - 1)
	return ret

# Some hacks to make py_compile work as advertised.
# Replace builtin "open" with one that returns one extra \n
# than is really in the file.
def hack_open(fname, mode = None):
	if mode is None:
		f = orig_open(fname, "r")
		return HackFile(f)
	else:
		return orig_open(fname, mode)
	
class HackFile:
	def __init__(self, f):
		self.f = f
	def __getattr__(self, attr):
		return getattr(self.f, attr)
	def read(self, num = -1):
		if num==-1:
			return self.f.read() + '\n'
		else:
			return self.f.read(num)

def compile_list(names, writeToLog = 1):
	orig_open = open
	sys.modules['__builtin__'].open = hack_open
	sys.modules['__builtin__'].orig_open = orig_open
	try:
		return do_compile_list(names, writeToLog)
	finally:
		sys.modules['__builtin__'].open = orig_open
		del sys.modules['__builtin__'].orig_open

def do_compile_list(names, writeToLog = 1):
	for fullname in names:
		try:
			nameDisplay = fullname
			if len(fullname) > 30:
				nameDisplay = fullname[-25:]
				try:
					nameDisplay = nameDisplay[string.index(nameDisplay, "\\"):]
				except ValueError:
					pass
				nameDisplay = fullname[:3] + "..." + nameDisplay
			Progress("Compiling " + nameDisplay)
			py_compile.compile(fullname)
			pywise.WriteToLog("File Copy: %s\r\n" % (fullname+'c'))
			pywise.WriteToLog("File Copy: %s\r\n" % (fullname+'o'))
		except KeyboardInterrupt:
			del names[:]
			print '\n[interrupt]'
			break
		except Cancelled, inst:
			print '\n[Cancelled]'
			raise Cancelled, inst
		except:
			if type(sys.exc_type) == type(''):
				exc_type_name = sys.exc_type
			else: exc_type_name = sys.exc_type.__name__
			print 'Cant compile', fullname
			if g_debugging:
				traceback.print_exc()

