import string

def SubstituteInString(inString, evalEnv):
	substChar = "$"
	fields = string.split(inString, substChar)
	newFields = []
	for i in range(len(fields)):
		didSubst = 0
		strVal = fields[i]
		if i%2!=0:
			try:
				strVal = evalEnv[string.lower(strVal)]
				newFields.append(strVal)
				didSubst = 1
			except:
				traceback.print_exc()
				print "Could not substitute", strVal
		if not didSubst:
			newFields.append(strVal)
	return string.join(map(str, newFields), "")

def SubstituteInFile(inName, outName, evalEnv):
	inFile = open(inName, "r")
	try:
		outFile = open(outName, "w")
		try:
			while 1:
				line = inFile.read()
				if not line: break
				outFile.write(SubstituteInString(line, evalEnv))
		finally:
			outFile.close()
	finally:
		inFile.close()

