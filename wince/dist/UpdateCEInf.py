# UpdateCEInf.py
#
# Update the Windows CE .inf file with the Python library.
import sys, string, glob, shutil, os, win32api, py_compile

verbose = 0

class FileSpec:
	def __init__(self, fname):
		self.fname = fname
	def _expand(self, platform = None):
		fname = win32api.ExpandEnvironmentStrings(self.fname)
		if platform:
			path, base = os.path.split(fname)
			path = os.path.join(path, platform)
			fname = os.path.join(path, base)
		rc = glob.glob(fname)
		if not rc: raise RuntimeError, "File spec '%s' resulted in no files" % (fname)
		return self._filter(rc)
	def _filter(self, l):
		return l
	def copy(self, destdir, platform):
		ret = []
		for fname in self._expand(platform):
			base = os.path.basename(fname)
			dest = destdir
			if platform: dest = os.path.join(dest, platform)
			destname = os.path.join(dest, base)
			if verbose:
				print "Copy", fname,"->", destname
			shutil.copyfile(fname, destname)
			ret.append(destname)
		return ret

class SourceFileSpec(FileSpec):
	platformSpecific = 0
	def compile(self, destdir):
		ret = []
		for fname in self._expand():
			base = os.path.basename(fname)
			destname = os.path.join(destdir, base) + "c"
			if verbose:
				print "Compile", fname,"->", destname
			py_compile.compile(fname, destname, base)
			ret.append(destname)
		return ret
	def copy(self, destdir, platform):
		assert platform is None, "Nothing platform specific about a source file"
		return FileSpec.copy(self, destdir, platform) + self.compile(destdir)

class CompiledSourceFileSpec(SourceFileSpec):
	"""Only the .pyc is copied"""
	platformSpecific = 0
	def copy(self, destdir, platform):
		assert platform is None, "Nothing platform specific about a source file"
		return self.compile(destdir)

class COFFFileSpec(FileSpec):
	platformSpecific = 1
	def _filter(self, list):
		ret = []
		for l in list:
			if os.path.splitext(l)[0][-2:]!="_d":
				ret.append(l)
		return ret

class PydFileSpec(COFFFileSpec):
	pass

class SystemFileSpec(COFFFileSpec):
	pass

FileSetMap = {
	'Library Files' : ('data\\Lib', 1),
	'Script Files' : ('data\\Scripts', 2),
	'System Files': ('data\\System', 3),
	'Extension Files': ('data\\DLLs', 4),
}

def UpdateInf( fileName, fileLists ):
	# Copy the fileName to a .bak file.
	tempFileName = os.path.splitext( fileName )[0] + ".$$$"
	bakFileName = os.path.splitext( fileName )[0] + ".bak"
	shutil.copyfile(fileName, tempFileName )
	inFile = open(fileName)
	outFile = open(tempFileName, "w")
	while 1:
		line = inFile.readline()
		if not line: break
		outFile.write(line)
		bSkipToEnd = 0

		checkData = string.split(line, "-", 4)
		checkData = map( string.strip, checkData )
		if len(checkData)==4 and checkData[0]== "; Python Files":
			if checkData[-1] != "Start":
				raise RuntimeError, "The input file is missing the 'Start' tag."

			setName = checkData[1]
			listType = checkData[2]
			try:
				path, sourceDiskNum = FileSetMap[setName]
			except KeyError:
				raise RuntimeError, "Invalid file set name: %s" % line
			try:
				fileList = fileLists[setName]
			except KeyError:
				print "Warning: No files found for file set '%s'" % setName
				fileList = [] # No files

			for file in fileList:
				file = os.path.split(file)[1]
				if listType=="Source Disks":
					newString = "%s = %d" % (file, sourceDiskNum)
				elif listType=="Files":
					newString = "%s,,,0" % file
				else:
					raise RuntimeError, "The tag '%s' has an invalid list type" % line
				
				outFile.write( newString )
				outFile.write( "\n" )

			# Now skip the existing files.
			while 1:
				line = inFile.readline()
				if not line:
					raise RuntimeError, "Could not find an End Tag for %s-%s" % (setName, listType)

				checkEndData = string.split(line, "-", 4)
				checkEndData = map( string.strip, checkEndData )
				if len(checkEndData)==4 and \
				   checkEndData[1]==checkData[1] and \
                                   checkEndData[-1]=="End":
					outFile.write(line)
					break
	# Close both files.
	inFile.close()
	outFile.close()
	shutil.copyfile(fileName, bakFileName)
	shutil.copyfile(tempFileName, fileName )
	os.unlink(tempFileName)


def BuildLibraryFiles(path, ext):
	if ext == ".pyc":
		import compileall
		compileall.compile_dir(path)

	libFiles = []
	for file in glob.glob("%s\\*%s" % (path, ext)):
		libFiles.append(file)

	return libFiles

def BuildExtensions(path):
	libFiles = []
	for file in glob.glob("%s\\*.pyd" % (path)):
		base, ext = os.path.splitext(file)
		if ext in [".pyd", ".dll"] and base[-2:] != "_d":
			libFiles.append(file)

	return libFiles

classMap = {
	'py' : SourceFileSpec,
	'pyc' : CompiledSourceFileSpec,
	'pyd' : PydFileSpec,
	'system' : SystemFileSpec,
}
def BuildFileList(dataFile):
	ret = {}
	lines = open(dataFile).readlines()
	for line in lines:
		line = string.strip(line)
		if not line: continue
		if line[0]==';': continue
		if line[0]=='[' and line[-1]==']':
			curKey = line[1:-1]
			continue
		typ, fname = string.split(line,'=',2)
		klass = classMap.get(string.lower(typ))
		if klass is None:
			raise RuntimeError, "The line '%s' has an invalid file type" % line
		if curKey is None:
			raise RuntimeError, "A [Section] must appear before data lines"
		if not ret.has_key(curKey): ret[curKey] = []
		ret[curKey].append( klass( fname ) )
	return ret

def CopyFilesToDist(files, platforms):
	for dest, sourceDiskNum in FileSetMap.values():
		EnsureCleanDirectory(dest)
	fileCats = files.keys()
	retFiles = {}
	for cat in fileCats:
		retFiles[cat] = thisList = []
		fl = files[cat]
		dest = FileSetMap[cat][0]
		for file in fl:
			if file.platformSpecific:
				for platform in platforms:
					CheckDestDirectory(os.path.join(dest, platform))
					lastFileList = file.copy(dest, platform)
				newList = lastFileList
			else:
				newList = file.copy(dest, None)
			retFiles[cat]=retFiles[cat]+newList
	return retFiles

def CheckDestDirectory(dir):
	if not os.path.isdir(dir):
		os.makedirs(dir)

def EnsureCleanDirectory(dir):
	if not os.path.isdir(dir):
		os.makedirs(dir)
	for file in glob.glob(os.path.join(dir, "*")):
		if os.path.isdir(file):
			EnsureCleanDirectory( file )
		else:
			os.unlink(file)
		
def main():
	try:
		fmap = BuildFileList("FileList.txt")
		builtFiles = CopyFilesToDist(fmap, ["sh","mips"])	
	except RuntimeError, details:
		print "ERROR:", details
		sys.exit(1)

	num = 0
	for l in builtFiles.values():
		num = num + len(l)
	print "Copied %d files - updating .inf file..." % num
	UpdateInf("PythonCE.inf", builtFiles)

if __name__=='__main__':
	if "-v" in sys.argv:
		verbose = 1
	main()