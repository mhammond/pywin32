import sys
sys.coinit_flags = 2 # Apartment threaded.
import win32api, os, string
#from vssutil import *
import getopt

sys.path.append(r"..\win32\scripts\VersionStamp")

import brandWin32, brandWin32com, brandPythonwin

def doit():
	desc = None
	auto=0
	bRebrand = 0
	opts, args = getopt.getopt(sys.argv[1:], "ad:rw:")
	for (o,v) in opts:
		if o=='-d':
			desc = v
		if o=='-a':
			auto=1
		if o=='-r':
			bRebrand = 1

	defines = []
	if args[0] == "cvs":
		# days since epoch <wink>
		import time
		build = str(int(time.time() / 3600 / 24))
		build_suffix = time.strftime("%Y%m%d", time.gmtime(time.time()))
		defines.append("PY_DEV_BUILD=1")
		pyver = "%d.%d" % (sys.version_info[0], sys.version_info[1])
		defines.append("PYVER_DOTTED=%s" % (pyver,))
		defines.append("PYVER_NODOT=%d%d" % (sys.version_info[0], sys.version_info[1]))
	else:
		if len(args) != 2:
			print "You must enter the Python version and the build number"
			return
		pyver = args[0]
		build = args[1]
		defines.append("PY_DEV_BUILD=0")
		try:
			if len(pyver.split("."))!=2:
				raise ValueError, "Bad python version '%s'" % pyver
			int(build) # check integer
			defines.append("PYVER_DOTTED=" + pyver)
			defines.append("PYVER_NODOT=" + "".join(pyver.split(".")))
		except ValueError, why:
			print why
			return
		build_suffix = build
	if sys.executable.find("_d")<0:
		print "Oops - can't brand myself - spawing debug version."
		print "XXX - note that readline is gunna screw up"
		base, ext = os.path.splitext(sys.executable)
		exe = base + "_d" + ext
		os.execv(exe, [exe]+sys.argv)
		# Does not return
		assert(False)
#	projectName = "$/Python/Wise/win32all"
	
#	build = MakeNewBuildNo(projectName, desc, auto, bRebrand)
#	if build is None:
#		print "Cancelled"
#		return

	if not brandWin32.doit(pyver, desc, auto, bRebrand, build):
		return

	if not brandWin32com.doit(pyver, desc, auto, bRebrand, build):
		return 

	if not brandPythonwin.doit(pyver, desc, auto, bRebrand, build):
		return

	import stampWise
	stampWise.UpdateWiseExeName("win32all.wse", "win32all-%s.exe" % (build_suffix) )

	import brandutils
	subst_dict = {"build_no" : build,
	               "pyver_dotted" : pyver,
	               "pyver_nodot" : ''.join(pyver.split('.')),
	}
	brandutils.SubstituteInFile("win32all_ver.wse.in", "win32all_ver.wse", subst_dict)
	# and generate the installation file.
	if defines:
		wse_name = "win32all.wse"
		if not os.path.isfile(wse_name):
			print "Can not find '%s' - can not generate installation" % wse_name
			return
		target_name = "win32all-" + str(build) + ".exe"
		if os.path.isfile(target_name):
			print "WARNING: %s already exists - press ENTER to have it killed anyway" % target_name
			raw_input()

		defines_str = ""
		for d in defines:
			defines_str += "/D" + d + " "
		# find the .exe for .wse files.
		ignore, exe_name = win32api.FindExecutable(wse_name)
		cmd = "%s %s /c %s" % (exe_name, defines_str, wse_name)
		print "About to generate WISE installer."
		print "Command: %s" % (cmd,)
		print "Press ENTER to continue"
		raw_input()
		os.system(cmd)

if __name__=='__main__':
	doit()
