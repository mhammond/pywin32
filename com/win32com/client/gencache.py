"""Manages the cache of generated Python code.

Description
  This file manages the cache of generated Python code.  When run from the 
  command line, it also provides a number of options for managing that cache.
  
Implementation
  Each typelib is generated into a filename of format "{guid}x{lcid}x{major}x{minor}.py"
  
  An external persistant dictionary maps from all known IIDs in all known type libraries
  to the type library itself.
  
  Thus, whenever Python code knows the IID of an object, it can find the IID, LCID and version of
  the type library which supports it.  Given this information, it can find the Python module
  with the support.
  
  If necessary, this support can be generated on the fly.
  
Hacks, to do, etc
  Currently just uses a pickled dictionary, but should used some sort of indexed file.
  Maybe an OLE2 compound file, or a bsddb file?
"""
import pywintypes, os, string, sys
import pythoncom
import win32com, win32com.client
import glob
import traceback
import CLSIDToClass

# The global dictionary
clsidToTypelib = {}

def __init__():
	# Initialize the module.  Called once automatically
	try:
		_LoadDicts()
	except IOError:
		Rebuild()

pickleVersion = 1
def _SaveDicts():
	import cPickle
	f = open(os.path.join(GetGeneratePath(), "dicts.dat"), "wb")
	try:
		p = cPickle.Pickler(f)
		p.dump(pickleVersion)
		p.dump(clsidToTypelib)
	finally:
		f.close()

def _LoadDicts():
	import cPickle
	try:
		# NOTE: IOError on file open must be caught by caller.
		f = open(os.path.join(win32com.__gen_path__, "dicts.dat"), "rb")
	except AttributeError: # no __gen_path__
		return
	try:
		p = cPickle.Unpickler(f)
		version = p.load()
		global clsidToTypelib
		clsidToTypelib = p.load()
	finally:
		f.close()

def GetGeneratedFileName(clsid, lcid, major, minor):
	"""Given the clsid, lcid, major and  minor for a type lib, return
	the file name (no extension) providing this support.
	"""
	return string.upper(str(clsid))[1:-1] + "x%sx%sx%s" % (lcid, major, minor)

def SplitGeneratedFileName(fname):
	"""Reverse of GetGeneratedFileName()
	"""
	return tuple(string.split(fname,'x',4))
	
def GetGeneratePath():
	"""Returns the name of the path to generate to.
	Checks the directory is OK.
	"""
	try:
		os.mkdir(win32com.__gen_path__)
	except os.error:
		pass
	try:
		fname = os.path.join(win32com.__gen_path__, "__init__.py")
		os.stat(fname)
	except os.error:
		f = open(fname,"w")
		f.write('# Generated file - this directory may be deleted to reset the COM cache...\n')
		f.write('import win32com\n')
		f.write('if __path__[:-1] != win32com.__gen_path__: __path__.append(win32com.__gen_path__)\n')
		f.close()
	
	return win32com.__gen_path__

#
# The helpers for win32com.client.Dispatch and OCX clients.
#
def GetClassForProgID(progid):
	"""Get a Python class for a Program ID
	
	Given a Program ID, return a Python class which wraps the COM object
	
	Returns the Python class, or None if no module is available.
	
	Params
	progid -- A COM ProgramID or IID (eg, "Word.Application")
	"""
	clsid = pywintypes.IID(progid) # This auto-converts named to IDs.
	return GetClassForCLSID(clsid)

def GetClassForCLSID(clsid):
	"""Get a Python class for a CLSID
	
	Given a CLSID, return a Python class which wraps the COM object
	
	Returns the Python class, or None if no module is available.
	
	Params
	clsid -- A COM CLSID (or string repr of one)
	"""
	# first, take a short-cut - we may already have generated support ready-to-roll.
	clsid = str(clsid)
	if CLSIDToClass.HasClass(clsid):
		return CLSIDToClass.GetClass(clsid)
	mod = GetModuleForCLSID(clsid)
	if mod is None:
		return None
	try:
		return CLSIDToClass.GetClass(clsid)
	except KeyError:
		return None

def GetModuleForProgID(progid):
	"""Get a Python module for a Program ID
	
	Given a Program ID, return a Python module which contains the
	class which wraps the COM object.
	
	Returns the Python module, or None if no module is available.
	
	Params
	progid -- A COM ProgramID or IID (eg, "Word.Application")
	"""
	try:
		iid = pywintypes.IID(progid)
	except pywintypes.com_error:
		return None
	return GetModuleForCLSID(iid)
	
def GetModuleForCLSID(clsid):
	"""Get a Python module for a CLSID
	
	Given a CLSID, return a Python module which contains the
	class which wraps the COM object.
	
	Returns the Python module, or None if no module is available.
	
	Params
	progid -- A COM CLSID (ie, not the description)
	"""
	clsid_str = str(clsid)
	try:
		typelibCLSID, lcid, major, minor = clsidToTypelib[clsid_str]
	except KeyError:
		return None

	mod = GetModuleForTypelib(typelibCLSID, lcid, major, minor)
	if mod is not None:
		sub_mod = mod.CLSIDToPackageMap.get(clsid_str)
		if sub_mod is not None:
			sub_mod_name = mod.__name__ + "." + sub_mod
			try:
				__import__(sub_mod_name)
			except ImportError:
				# Force the generation.
				import makepy
				info = typelibCLSID, lcid, major, minor
				makepy.GenerateChildFromTypeLibSpec(sub_mod, info)
				# Generate does an import...
			mod = sys.modules[sub_mod_name]
	return mod

def GetModuleForTypelib(typelibCLSID, lcid, major, minor):
	"""Get a Python module for a type library ID
	
	Given the CLSID of a typelibrary, return an imported Python module, 
	else None
	
	Params
	typelibCLSID -- IID of the type library.
	major -- Integer major version.
	minor -- Integer minor version
	lcid -- Integer LCID for the library.
	"""
	modName = GetGeneratedFileName(typelibCLSID, lcid, major, minor)
	return _GetModule(modName)


def MakeModuleForTypelib(typelibCLSID, lcid, major, minor, progressInstance = None, bGUIProgress = None, bForDemand = 0, bBuildHidden = 1):
	"""Generate support for a type library.
	
	Given the IID, LCID and version information for a type library, generate
	and import the necessary support files.
	
	Returns the Python module.  No exceptions are caught.

	Params
	typelibCLSID -- IID of the type library.
	major -- Integer major version.
	minor -- Integer minor version.
	lcid -- Integer LCID for the library.
	progressInstance -- Instance to use as progress indicator, or None to
	                    use the GUI progress bar.
	"""
	if bGUIProgress is not None:
		print "The 'bGuiProgress' param to 'MakeModuleForTypelib' is obsolete."

	import makepy
	try:
		makepy.GenerateFromTypeLibSpec( (typelibCLSID, lcid, major, minor), progressInstance=progressInstance, bForDemand = bForDemand, bBuildHidden = bBuildHidden)
	except pywintypes.com_error:
		return None
	return GetModuleForTypelib(typelibCLSID, lcid, major, minor)

def MakeModuleForTypelibInterface(typelib_ob, progressInstance = None, bForDemand = 0, bBuildHidden = 1):
	"""Generate support for a type library.
	
	Given a PyITypeLib interface generate and import the necessary support files.  This is useful
	for getting makepy support for a typelibrary that is not registered - the caller can locate
	and load the type library itself, rather than relying on COM to find it.
	
	Returns the Python module.

	Params
	typelib_ob -- The type library itself
	progressInstance -- Instance to use as progress indicator, or None to
	                    use the GUI progress bar.
	"""
	import makepy
	try:
		makepy.GenerateFromTypeLibSpec( typelib_ob, progressInstance=progressInstance, bForDemand = bForDemand, bBuildHidden = bBuildHidden)
	except pywintypes.com_error:
		return None
	tla = typelib_ob.GetLibAttr()
	guid = tla[0]
	lcid = tla[1]
	major = tla[3]
	minor = tla[4]
	return GetModuleForTypelib(guid, lcid, major, minor)

def EnsureModuleForTypelibInterface(typelib_ob, progressInstance = None, bForDemand = 0, bBuildHidden = 1):
	"""Check we have support for a type library, generating if not.
	
	Given a PyITypeLib interface generate and import the necessary
	support files if necessary. This is useful for getting makepy support
	for a typelibrary that is not registered - the caller can locate and
	load the type library itself, rather than relying on COM to find it.
	
	Returns the Python module.

	Params
	typelib_ob -- The type library itself
	progressInstance -- Instance to use as progress indicator, or None to
	                    use the GUI progress bar.
	"""
	tla = typelib_ob.GetLibAttr()
	guid = tla[0]
	lcid = tla[1]
	major = tla[3]
	minor = tla[4]
	try:
		return GetModuleForTypelib(guid, lcid, major, minor)
	except ImportError:
		pass
	# Generate it.
	return MakeModuleForTypelibInterface(typelib_ob, progressInstance, bForDemand, bBuildHidden)

def EnsureModule(typelibCLSID, lcid, major, minor, progressInstance = None, bValidateFile=1, bForDemand = 0, bBuildHidden = 1):
	"""Ensure Python support is loaded for a type library, generating if necessary.
	
	Given the IID, LCID and version information for a type library, check and if
	necessary (re)generate, then import the necessary support files. If we regenerate the file, there
	is no way to totally snuff out all instances of the old module in Python, and thus we will regenerate the file more than necessary,
	unless makepy/genpy is modified accordingly.
	
	
	Returns the Python module.  No exceptions are caught during the generate process.

	Params
	typelibCLSID -- IID of the type library.
	major -- Integer major version.
	minor -- Integer minor version
	lcid -- Integer LCID for the library.
	progressInstance -- Instance to use as progress indicator, or None to
	                    use the GUI progress bar.
	bValidateFile -- Whether or not to perform cache validation or not
	bForDemand -- Should a complete generation happen now, or on demand?
	bBuildHidden -- Should hidden members/attributes etc be generated?
	"""
	bReloadNeeded = 0
	try:
		try:
			#print "Try specified typelib"
			module = GetModuleForTypelib(typelibCLSID, lcid, major, minor)
			#print module
		except ImportError:
			# If we get an ImportError
			# We may still find a valid cache file under a different MinorVersion #
			#print "Loading reg typelib"
			tlbAttr = pythoncom.LoadRegTypeLib(typelibCLSID, major, minor, lcid).GetLibAttr()
			# if the above line doesn't throw a pythoncom.com_error
			# Let's suck it in
			#print "Trying 2nd minor #", tlbAttr[1], tlbAttr[3], tlbAttr[4]
			try:
				module = GetModuleForTypelib(typelibCLSID, tlbAttr[1], tlbAttr[3], tlbAttr[4])
			except ImportError:
				module = None
				minor = tlbAttr[4]
		if bValidateFile:
			filePathPrefix  = "%s\\%s" % (GetGeneratePath(), GetGeneratedFileName(typelibCLSID, lcid, major, minor))
			filePath = filePathPrefix + ".py"
			filePathPyc = filePathPrefix + ".py"
			if __debug__:
				filePathPyc = filePathPyc + "c"
			else:
				filePathPyc = filePathPyc + "o"
			# Verify that type library is up to date.
			#print "Grabbing typelib"
			# If the following doesn't throw an exception, then we have a valid type library
			typLibPath = pythoncom.QueryPathOfRegTypeLib(typelibCLSID, major, minor, lcid)
			tlbAttributes = pythoncom.LoadRegTypeLib(typelibCLSID, major, minor, lcid).GetLibAttr()
			#print "Grabbed typelib: ", tlbAttributes[3], tlbAttributes[4]
			##print module.MinorVersion
			# If we have a differing MinorVersion or genpy has bumped versions, update the file
			import genpy
			if module is not None and (module.MinorVersion != tlbAttributes[4] or genpy.makepy_version != module.makepy_version):
				#print "Version skew: %d, %d" % (module.MinorVersion, tlbAttributes[4])
				# try to erase the bad file from the cache
				try:
					os.unlink(filePath)
				except os.error:
					pass
				try:
					os.unlink(filePathPyc)
				except os.error:
					pass
				if os.path.isdir(filePathPrefix):
					import shutil
					shutil.rmtree(filePathPrefix)
				minor = tlbAttributes[4]
				module = None
				bReloadNeeded = 1
			else:
				if module is not None:
					minor = module.MinorVersion
					filePathPrefix  = "%s\\%s" % (GetGeneratePath(), GetGeneratedFileName(typelibCLSID, lcid, major, minor))
					filePath = filePathPrefix + ".py"
					filePathPyc = filePathPrefix + ".pyc"
					#print "Trying py stat: ", filePath
					fModTimeSet = 0
					try:
						pyModTime = os.stat(filePath)[8]
						fModTimeSet = 1
					except os.error, e:
						# If .py file fails, try .pyc file
						#print "Trying pyc stat", filePathPyc
						try:
							pyModTime = os.stat(filePathPyc)[8]
							fModtimeSet = 1
						except os.error, e:
							pass
					#print "Trying stat typelib", pyModTime
					#print str(typLibPath)
					typLibModTime = os.stat(str(typLibPath[:-1]))[8]
					if fModTimeSet and (typLibModTime > pyModTime):
						bReloadNeeded = 1
						module = None
	except (ImportError, os.error):	
		module = None
	if module is None:
		#print "Rebuilding: ", major, minor
		module = MakeModuleForTypelib(typelibCLSID, lcid, major, minor, progressInstance, bForDemand = bForDemand, bBuildHidden = bBuildHidden)
		# If we replaced something, reload it
		if bReloadNeeded:
			module = reload(module)
			AddModuleToCache(typelibCLSID, lcid, major, minor)
	return module

def EnsureDispatch(prog_id, bForDemand = 1): # New fn, so we default the new demand feature to on!
	"""Given a COM prog_id, return an object that is using makepy support, building if necessary"""
	disp = win32com.client.Dispatch(prog_id)
	if not disp.__dict__.get("CLSID"): # Eeek - no makepy support - try and build it.
		try:
			ti = disp._oleobj_.GetTypeInfo()
			disp_clsid = ti.GetTypeAttr()[0]
			tlb, index = ti.GetContainingTypeLib()
			tla = tlb.GetLibAttr()
			mod = EnsureModule(tla[0], tla[1], tla[3], tla[4], bForDemand=bForDemand)
			GetModuleForCLSID(disp_clsid)
			# Get the class from the module.
			import CLSIDToClass
			disp_class = CLSIDToClass.GetClass(str(disp_clsid))
			disp = disp_class(disp._oleobj_)
		except pythoncom.com_error:
			raise TypeError, "This COM object can not automate the makepy process - please run makepy manually for this object"
	return disp

def AddModuleToCache(typelibclsid, lcid, major, minor, verbose = 1, bFlushNow = 1):
	"""Add a newly generated file to the cache dictionary.
	"""
	fname = GetGeneratedFileName(typelibclsid, lcid, major, minor)
	mod = _GetModule(fname)
	dict = mod.CLSIDToClassMap
	for clsid, cls in dict.items():
		clsidToTypelib[clsid] = (str(typelibclsid), mod.LCID, mod.MajorVersion, mod.MinorVersion)

	dict = mod.CLSIDToPackageMap
	for clsid, name in dict.items():
		clsidToTypelib[clsid] = (str(typelibclsid), mod.LCID, mod.MajorVersion, mod.MinorVersion)
	if bFlushNow:
		_SaveDicts()

def _GetModule(fname):
	"""Given the name of a module in the gen_py directory, import and return it.
	"""
	mod = __import__("win32com.gen_py.%s" % fname)
	return getattr( getattr(mod, "gen_py"), fname)
	
def Rebuild(verbose = 1):
	"""Rebuild the cache indexes from the file system.
	"""
	clsidToTypelib.clear()
	files = glob.glob(win32com.__gen_path__+ "\\*.py")
	if verbose and len(files): # Dont bother reporting this when directory is empty!
		print "Rebuilding cache of generated files for COM support..."
	for file in files:
		name = os.path.splitext(os.path.split(file)[1])[0]
		try:
			iid, lcid, major, minor = string.split(name, "x")
			ok = 1
		except ValueError:
			ok = 0
		if ok:
			try:
				iid = pywintypes.IID("{" + iid + "}")
			except pywintypes.com_error:
				ok = 0
		if ok:
			if verbose:
				print "Checking", name
			try:
				AddModuleToCache(iid, lcid, major, minor, verbose, 0)
			except:
				print "Could not add module %s - %s: %s" % (name, sys.exc_info()[0],sys.exc_info()[1])
		else:
			if verbose and name[0] != '_':
				print "Skipping module", name
	if verbose and len(files): # Dont bother reporting this when directory is empty!
		print "Done."
	_SaveDicts()

def _Dump():
	print "Cache is in directory", win32com.__gen_path__
	files = glob.glob(win32com.__gen_path__+ "\*.py")
	for file in files:
		name = os.path.splitext(os.path.split(file)[1])[0]
		if name[0] != '_':
			mod = _GetModule(name)
			info = SplitGeneratedFileName(name)
			print "%s - %s" % (mod.__doc__, name)

	
__init__()

usageString = """\
  Usage: gencache [-q] [-d] [-r]
  
         -q         - Quiet
         -d         - Dump the cache (typelibrary description and filename).
         -r         - Rebuild the cache dictionary from the existing .py files
"""

def usage():
	print usageString
	sys.exit(1)

if __name__=='__main__':
	import getopt
	try:
		opts, args = getopt.getopt(sys.argv[1:], "qrd")
	except getopt.error, message:
		print message
		usage()

	# we only have options - complain about real args, or none at all!
	if len(sys.argv)==1 or args:
		print usage()
		
	verbose = 1
	for opt, val in opts:
		if opt=='-d': # Dump
			_Dump()
		if opt=='-r':
			Rebuild(verbose)
		if opt=='-q':
			verbose = 0

#if __name__=='__main__':
#	print "Rebuilding cache..."
#	Rebuild(1)		