import sys
sys.path.append(r"..\win32\scripts\VersionStamp")

#import vssutil, os
import os

def UpdateWiseExeName(wiseFileName, exeFileName):
	tempFileName = wiseFileName + ".tmp"
	fnew = open(tempFileName, "w")
	f = open(wiseFileName, "r")
	try:
		found = 0
		for line in f.readlines():
			checkfor="  EXE Filename="
			if line[:len(checkfor)]==checkfor:
				line=checkfor+exeFileName + "\n"
				found = 1
			fnew.write(line)
	finally:
		f.close()
		fnew.close()
	if not found:
		raise error, "Could not find the 'EXE Filename' entry"

	bakFileName = os.path.splitext(wiseFileName)[0] + ".old"
	try:
		os.unlink(bakFileName)
	except os.error, why:
		pass

	try:
		os.rename(wiseFileName, bakFileName)
		os.rename(tempFileName, wiseFileName)
	except:
		print "Error with:"
		print "wiseFileName", wiseFileName
		print "bakFileName", bakFileName
		print "tempFileName", tempFileName
		raise

def StampWise(projectName, build):
#	build = vssutil.GetLastBuildNo(projectName)
	UpdateWiseExeName("win32all.wse", "win32all-%s.exe" % (build) )

if __name__=='xxxxxxxxx__main__':
	StampWise("$/Python/Wise/win32all")
	path="..\\win32"
	vssutil.SubstituteVSSInFile("$/Python/Win32", os.path.join(path,"win32.txt.in"), os.path.join(path,"win32.txt"))

	path="..\\com"
	vssutil.SubstituteVSSInFile("$/Python/COM", os.path.join(path,"win32com\\changes.txt"), os.path.join(path,"win32com\\win32com.txt"))

	path ="..\\pythonwin"
	vssutil.SubstituteVSSInFile("$/Python/Pythonwin", os.path.join(path,"Pythonwin.txt.in"), os.path.join(path,"Pythonwin.txt"))


	vssutil.SubstituteVSSInFile("$/Python/com", "win32com_ver.wse.in", "win32com_ver.wse")
	vssutil.SubstituteVSSInFile("$/Python/Win32", "win32_ver.wse.in", "win32_ver.wse")
	vssutil.SubstituteVSSInFile("$/Python/Pythonwin", "pywin_ver.wse.in", "pywin_ver.wse")

