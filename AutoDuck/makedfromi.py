import sys, string, os, getopt

# Run this passing a ".i" file as param.  Will generate ".d"

g_com_parent = ""

def GetComments(line, lineNo, lines):
	# Get the comment from this and continouos lines, if they exist.
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
		doc = doc + "\n// " + string.strip(data[1])
		lineNo = lineNo + 1
	# This line doesnt match - step back
	lineNo = lineNo - 1
	return doc, lineNo

def make_doc_summary(inFile, outFile):
	methods = []
	nativeMethods = []
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
					modDoc = string.strip(extra[1])
			elif line[:10]=="// @pyswig":
				curMethod = string.strip(line[10:]), []
				methods.append(curMethod)
			elif line[:11]=="// @pymeth ":
				doc, lineNo = GetComments(line, lineNo, lines)
				nativeMethods.append(line+doc)
			elif line[:7]=="#define" and not bInRawBlock:
				cname = string.split(line)[1]
				doc, lineNo = GetComments(line, lineNo, lines)
				constants.append((cname, doc))
			else:
				pos = string.find(line, '// @')
				if pos>=0:
					doc, lineNo = GetComments(line, lineNo, lines)
					if curMethod:
						curMethod[1].append("// " + doc + '\n')
					else:
						extra_tags.append("// " + doc + '\n')
		except:
			print "Line %d is badly formed - %s" % (lineNo, str(sys.exc_value))
			
		lineNo = lineNo + 1

	# Dump in different order
	if g_com_parent:
		modName = "Py" + modName
		
	for (meth, extras) in methods:
		fields = string.split(meth,'|')
		if len(fields)<>3:
			print "**Error - %s does not have enough fields" % meth
		outFile.write("\n// @pymethod %s|%s|%s|%s\n" % (fields[0],modName,fields[1], fields[2]))
		for extra in extras:
			outFile.write(extra)

	if g_com_parent:
		outFile.write("\n// @object %s|%s" % (modName,modDoc))
		outFile.write("\n// <nl>Derived from <o %s>\n" % (g_com_parent))
	else:
		outFile.write("\n// @module %s|%s\n" % (modName,modDoc))

	for (meth, extras) in methods:
		fields = string.split(meth,'|')
		outFile.write("// @pymeth %s|%s\n" % (fields[1], fields[2]))
	for meth in nativeMethods:
		outFile.write(meth)
	outFile.write("\n")
	for (cname, doc) in constants:
		outFile.write("// @const %s|%s|%s\n" % (modName, cname, doc) )
	for extra in extra_tags:
		outFile.write("%s\n" % (extra) )


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
