import sys
sys.coinit_flags = 2 # Apartment threaded.
import win32api, os, string
from brandutils import *


def doit(buildDesc = None, auto=0, bRebrand = 0, build=None):
	path=win32api.GetFullPathName("..\\com\\Build")
	projectName = "$/Python/Python COM"
	
	if build is None:
		build = MakeNewBuildNo(projectName, buildDesc, auto, bRebrand)
	if build is None:
		return

	import bulkstamp
	bulkstamp.scan( build, path, "desc.txt" )
	subst_dict = {"vss_label" : build }
#	SubstituteVSSInFile(projectName, os.path.join(path,"..\\win32com\\changes.txt"), "win32com.txt")
#	SubstituteInFile("win32com_ver.wse.in", "win32com_ver.wse", subst_dict)
	return 1
	
if __name__=='__main__':	
	doit()
