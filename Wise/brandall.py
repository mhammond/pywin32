import sys
sys.coinit_flags = 2 # Apartment threaded.
import win32api, os, string
#from vssutil import *
import getopt

import sys
sys.path.append(r"..\win32\scripts\VersionStamp")

import brandWin32, brandWin32com, brandPythonwin

#import pythoncom

def doit():
	desc = None
	auto=0
	bRebrand = 0
	opts, args = getopt.getopt(sys.argv[1:], "ad:r")
	for (o,v) in opts:
		if o=='-d':
			desc = v
		if o=='-a':
			auto=1
		if o=='-r':
			bRebrand = 1

	if len(args) != 1:
		print "You must enter the build number"
		return
	build = args[0]
#	projectName = "$/Python/Wise/win32all"
	
#	build = MakeNewBuildNo(projectName, desc, auto, bRebrand)
#	if build is None:
#		print "Cancelled"
#		return

	if not brandWin32.doit(desc, auto, bRebrand, build):
		return

	if not brandWin32com.doit(desc, auto, bRebrand, build):
		return 

	if not brandPythonwin.doit(desc, auto, bRebrand, build):
		return

	import stampWise
	stampWise.UpdateWiseExeName("win32all.wse", "win32all-%s.exe" % (build) )

	import brandutils
	subst_dict = {"build_no" : build }
	brandutils.SubstituteInFile("win32all_ver.wse.in", "win32all_ver.wse", subst_dict)


if __name__=='__main__':	
	doit()
