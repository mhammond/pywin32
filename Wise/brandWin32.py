import sys
sys.coinit_flags = 2 # Apartment threaded.
import win32api, os, string
from brandutils import *

def doit(buildDesc = None, auto=0, bRebrand = 0, build = None):
	path=win32api.GetFullPathName("..\\win32")
	projectName = "$/Python/Python Win32 Extensions"

	if build is None:	
		build = MakeNewBuildNo(projectName, buildDesc, auto, bRebrand)
	if build is None:
		print "Cancelled"
		return

	import bulkstamp
	bulkstamp.scan( build, path, "desc.txt" )
	subst_dict = {"vss_label" : build }
#	SubstituteVSSInFile(projectName, os.path.join(path,"win32.txt.in"), os.path.join(path,"win32.txt"))
#	SubstituteInFile("win32_ver.wse.in", "win32_ver.wse", subst_dict)
	return 1
	
if __name__=='__main__':	
	doit()
