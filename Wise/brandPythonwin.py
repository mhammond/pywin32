import sys
sys.coinit_flags = 2 # Apartment threaded.
import win32api, os, string
from brandutils import *

def doit(pyver=sys.winver, buildDesc=None, auto=0, bRebrand = 0, build = None):
	path=win32api.GetFullPathName("..\\Pythonwin")
	projectName = "$/Python/Pythonwin"

	if build is None:	
		build = MakeNewBuildNo(projectName, buildDesc, auto, bRebrand)
	if build is None:
		print "Cancelled."
		return
	
	import bulkstamp
	major, minor = string.split(pyver, ".")
	bulkstamp.scan( build, path, "desc.txt", major=major, minor=minor )
	subst_dict = {"vss_label" : build }

#	SubstituteVSSInFile(projectName, os.path.join(path,"Pythonwin.txt.in"), os.path.join(path,"Pythonwin.txt"))
#	SubstituteInFile("pywin_ver.wse.in", "pywin_ver.wse", subst_dict)
	return 1

if __name__=='__main__':	
	doit()
