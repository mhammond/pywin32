import sys, string, os, getopt

# Run this passing a ".i" file as param.  Will generate ".d"

g_com_parent = ""

def GetComments(line, lineNo, lines):
	# Get the comment from this and continuous lines, if they exist.
	data = string.split(line, "//", 2)
	doc = ""
	if len(data)==2: doc=string.strip(data[1])
	lineNo = lineNo + 1
	while lineNo < len(lines):
		line = lines[lineNo]
		data = string.split(line, "//", 2)
		if len(data)!=2:
			break
		if string.strip(data[0]):
			break # Not a continutation!
		if data[1].strip().startswith("@"):
			# new command
			break
		doc = doc + "\n// " + string.strip(data[1])
		lineNo = lineNo + 1
	# This line doesnt match - step back
	lineNo = lineNo - 1
	return doc, lineNo

def make_doc_summary(inFile, outFile):
	methods = []
	modDoc = ""
	modName = ""
	lines = inFile.readlines()
	curMethod = None
	constants = []
	extra_tags = []
	lineNo = 0
	bInRawBlock = 0
	while lineNo < len(lines):
		line = lines[lineNo]
		if bInRawBlock and len(line)>2 and line[:2]=="%}":
			bInRawBlock = 0
		if not bInRawBlock and len(line)>2 and line[:2]=="%{":
			bInRawBlock = 1
		try:
			if line[:7]=="%module":
				extra = string.split(line, "//")
				if len(extra)>1:
					modName = string.strip(extra[0][7:])
					modDoc, lineNo = GetComments(line, lineNo, lines)
				lineNo += 1
			elif line[:7]=="#define" and not bInRawBlock:
				cname = string.split(line)[1]
				doc, lineNo = GetComments(line, lineNo, lines)
				constants.append((cname, doc))
			else:
				try:
					pos = line.index("//")
				except ValueError:
					pass
				else:
					rest = line[pos+2:].strip()
					if rest.startswith("@pymeth"):
						# manual markup - reset the current method.
						curMethod = None
					if rest.startswith("@doc"):
						pass
					elif rest.startswith("@pyswig"):
						doc, lineNo = GetComments(line, lineNo, lines)
						curMethod = doc[8:], []
						methods.append(curMethod)
					elif rest.startswith("@const"):
						doc, lineNo = GetComments(line, lineNo, lines)
					else:
						if rest.startswith("@"):
							doc, lineNo = GetComments(line, lineNo, lines)
							if curMethod:
								curMethod[1].append("// " + doc + '\n')
							else:
								extra_tags.append("// " + doc + '\n')
		except:
			print "Line %d is badly formed - %s" % (lineNo, str(sys.exc_value))
			
		lineNo = lineNo + 1

	# autoduck seems to crash when > ~97 methods.  Loop multiple times, 
	# creating a synthetic module name when this happens.
	max_methods = 90
	method_num = 0
	chunk_number = 0
	while 1:
		these_methods = methods[method_num:method_num+max_methods]
		if not these_methods:
			break
		thisModName = modName
		if g_com_parent:
			thisModName = "Py" + modName
		if chunk_number == 0:
			pass
		elif chunk_number == 1:
			thisModName = thisModName + " (more)"
		else:
			thisModName = thisModName + " (more %d)" % (chunk_number+1,)

		outFile.write("\n")
		for (meth, extras) in these_methods:
			fields = string.split(meth,'|')
			if len(fields)<>3:
				print "**Error - %s does not have enough fields" % meth
			else:
				outFile.write("// @pymethod %s|%s|%s|%s\n" % (fields[0],thisModName,fields[1], fields[2]))
			for extra in extras:
				outFile.write(extra)
		if g_com_parent:
			outFile.write("\n// @object %s|%s" % (thisModName,modDoc))
			outFile.write("\n// <nl>Derived from <o %s>\n" % (g_com_parent))
		else:
			outFile.write("\n// @module %s|%s\n" % (thisModName,modDoc))
		for (meth, extras) in these_methods:
			fields = string.split(meth,'|')
			outFile.write("// @pymeth %s|%s\n" % (fields[1], fields[2]))
		chunk_number += 1
		method_num += max_methods

	outFile.write("\n")
	for extra in extra_tags:
		outFile.write("%s\n" % (extra) )
	for (cname, doc) in constants:
		outFile.write("// @const %s|%s|%s\n" % (modName, cname, doc) )


def doit():
	global g_com_parent
	outName = ""
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'p:o:')
		for o,a in opts:
			if o=='-p':
				g_com_parent = a
			elif o=='-o':
				outName = a
		msg = string.join(args)
	except getopt.error, msg:
		print msg
		print "Usage: %s [-o output_name] [-p com_parent] filename" % sys.argv[0]
		return

	inName = args[0]
	if not outName:
		outName = os.path.splitext(os.path.split(inName)[1])[0] + ".d"
	inFile = open(inName)
	outFile = open(outName, "w")
	outFile.write("// @doc\n// Generated file - built from %s\n// DO NOT CHANGE - CHANGES WILL BE LOST!\n\n" % inName)
	make_doc_summary(inFile, outFile)
	inFile.close()
	outFile.close()

if __name__=='__main__':
	doit()
