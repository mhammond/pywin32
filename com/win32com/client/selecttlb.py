"""Utilities for selecting and enumerating the Type Libraries installed on the system
"""

import win32api, win32con, string

class TypelibSpec:
	def __init__(self, clsid, lcid, major, minor, flags=0):
		self.clsid = str(clsid)
		self.lcid = int(lcid)
		self.major = int(major)
		self.minor = int(minor)
		self.dll = None
		self.desc = None
		self.ver_desc = None
		self.flags = flags
	# For the SelectList
	def __getitem__(self, item):
		if item==0:
			return self.ver_desc
		raise IndexError, "Cant index me!"
	def __cmp__(self, other):
		rc = cmp(string.lower(self.ver_desc or ""), string.lower(other.ver_desc or ""))
		if rc==0:
			rc = cmp(string.lower(self.desc), string.lower(other.desc))
		if rc==0:
			rc = cmp(self.major, other.major)
		if rc==0:
			rc = cmp(self.major, other.minor)
		return rc
		
	def FromTypelib(self, typelib, dllName = None):
		la = typelib.GetLibAttr()
		self.clsid = str(la[0])
		self.lcid = la[1]
		self.major = la[3]
		self.minor = la[4]
		if dllName:
			self.dll = dllName

def EnumKeys(root):
	index = 0
	ret = []
	while 1:
		try:
			item = win32api.RegEnumKey(root, index)
		except win32api.error:
			break
		try:
			val = win32api.RegQueryValue(root, item)
		except win32api.error:
			val = None
			
		ret.append((item, val))
		index = index + 1
	return ret

FLAG_RESTRICTED=1
FLAG_CONTROL=2
FLAG_HIDDEN=4

def EnumTlbs(excludeFlags = 0):
	"""Return a list of TypelibSpec objects, one for each registered library.
	"""
	key = win32api.RegOpenKey(win32con.HKEY_CLASSES_ROOT, "Typelib")
	iids = EnumKeys(key)
	results = []
	for iid, crap in iids:
		key2 = win32api.RegOpenKey(win32con.HKEY_CLASSES_ROOT, "Typelib\\%s" % (iid))
		for version, tlbdesc in EnumKeys(key2):
			key3 = win32api.RegOpenKey(win32con.HKEY_CLASSES_ROOT, "Typelib\\%s\\%s" % (iid, version))
			try:
				# The "FLAGS" are at this point
				flags = int(win32api.RegQueryValue(key3, "FLAGS"))
			except (win32api.error, ValueError):
				flags = 0
			if flags & excludeFlags==0:
				for lcid, crap in EnumKeys(key3):
					key4 = win32api.RegOpenKey(win32con.HKEY_CLASSES_ROOT, "Typelib\\%s\\%s\\%s" % (iid, version, lcid))
					for platform, dll in EnumKeys(key4):
						if platform=="win32":
							major = string.split(version, '.', 1)
							if len(major) < 2:
								major.append('0')
							major, minor = string.atoi(major[0], 16), string.atoi(major[1], 16)
							lcid = string.atoi(lcid,16)
							spec = TypelibSpec(iid, lcid, major, minor, flags)
							spec.desc = tlbdesc
							spec.ver_desc = tlbdesc + " (" + version + ")"
							spec.dll = dll
							results.append(spec)
	return results

def FindTlbsWithDescription(desc):
	"""Find all installed type libraries with the specified description
	"""
	ret = []
	items = EnumTlbs()
	for item in items:
		if item.desc==desc:
			ret.append(item)
	return ret

def SelectTlb(title="Select Library", excludeFlags = 0):
	"""Display a list of all the type libraries, and select one.   Returns None if cancelled
	"""
	import pywin.dialogs.list
	items = EnumTlbs(excludeFlags)
	items.sort()
	rc = pywin.dialogs.list.SelectFromLists(title, items, ["Type Library"])
	if rc is None:
		return None
	return items[rc]

# Test code.
if __name__=='__main__':
	print SelectTlb()
