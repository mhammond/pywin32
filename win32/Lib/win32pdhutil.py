"""Utilities for the win32 Performance Data Helper module

Example:
  To get a single bit of data:
  >>> import win32pdhutil
  >>> win32pdhutil.GetPerformanceAttributes("Memory", "Available Bytes")
  6053888
  >>> win32pdhutil.FindPerformanceAttributesByName("python", counter="Virtual Bytes")
  [22278144]
  
  First example returns data which is not associated with any specific instance.
  
  The second example reads data for a specific instance - hence the list return - 
  it would return one result for each instance of Python running.

  In general, it can be tricky finding exactly the "name" of the data you wish to query.  
  Although you can use <om win32pdh.EnumObjectItems>(None,None,(eg)"Memory", -1) to do this, 
  the easiest way is often to simply use PerfMon to find out the names.
"""

import win32pdh, string, time

error = win32pdh.error

def GetPerformanceAttributes(object, counter, instance = None, inum=-1, format = win32pdh.PDH_FMT_LONG, machine=None):
	path = win32pdh.MakeCounterPath( (machine,object,instance, None, inum,counter) )
	hq = win32pdh.OpenQuery()
	try:
		hc = win32pdh.AddCounter(hq, path)
		try:
			win32pdh.CollectQueryData(hq)
			type, val = win32pdh.GetFormattedCounterValue(hc, format)
			return val
		finally:
			win32pdh.RemoveCounter(hc)
	finally:
		win32pdh.CloseQuery(hq)

def FindPerformanceAttributesByName(instanceName, object = "Process", counter = "ID Process", format = win32pdh.PDH_FMT_LONG, machine = None, bRefresh=0):
	"""Find peformance attributes by (case insensitive) instance name.
	
	Given a process name, return a list with the requested attributes.
	Most useful for returning a tuple of PIDs given a process name.
	"""

	if bRefresh: # PDH docs say this is how you do a refresh.
		win32pdh.EnumObjects(None, machine, 0, 1)
	instanceName = string.lower(instanceName)
	items, instances = win32pdh.EnumObjectItems(None,None,object, -1)
	# Track multiple instances.
	instance_dict = {}
	for instance in instances:
		try:
			instance_dict[instance] = instance_dict[instance] + 1
		except KeyError:
			instance_dict[instance] = 0
		
	ret = []
	for instance, max_instances in instance_dict.items():
		for inum in xrange(max_instances+1):
			if string.lower(instance) == instanceName:
				ret.append(GetPerformanceAttributes(object, counter, instance, inum, format, machine))
	return ret

def ShowAllProcesses():
	object = "Process"
	items, instances = win32pdh.EnumObjectItems(None,None,object, win32pdh.PERF_DETAIL_WIZARD)
	# Need to track multiple instances of the same name.
	instance_dict = {}
	for instance in instances:
		try:
			instance_dict[instance] = instance_dict[instance] + 1
		except KeyError:
			instance_dict[instance] = 0
		
	# Bit of a hack to get useful info.
	items = ["ID Process"] + items[:5]
	print "Process Name", string.join(items,",")
	for instance, max_instances in instance_dict.items():
		for inum in xrange(max_instances+1):
			hq = win32pdh.OpenQuery()
			hcs = []
			for item in items:
				path = win32pdh.MakeCounterPath( (None,object,instance, None, inum, item) )
				hcs.append(win32pdh.AddCounter(hq, path))
			win32pdh.CollectQueryData(hq)
			# as per http://support.microsoft.com/default.aspx?scid=kb;EN-US;q262938, some "%" based
			# counters need two collections
			time.sleep(0.01)
			win32pdh.CollectQueryData(hq)
			print "%-15s\t" % (instance[:15]),
			for hc in hcs:
				type, val = win32pdh.GetFormattedCounterValue(hc, win32pdh.PDH_FMT_LONG)
				print "%5d" % (val),
				win32pdh.RemoveCounter(hc)
			print
			win32pdh.CloseQuery(hq)

def BrowseCallBackDemo(counter):
	machine, object, instance, parentInstance, index, counterName = \
		win32pdh.ParseCounterPath(counter)

	result = GetPerformanceAttributes(object, counterName, instance, index, win32pdh.PDH_FMT_DOUBLE, machine)
	print "Value of '%s' is" % counter, result
	print "Added '%s' on object '%s' (machine %s), instance %s(%d)-parent of %s" % (counterName, object, machine, instance, index, parentInstance)

def browse( callback = BrowseCallBackDemo, title="Python Browser", level=win32pdh.PERF_DETAIL_WIZARD):
	win32pdh.BrowseCounters(None,0, callback, level, title)

def test():
	print "Virtual Bytes = ", FindPerformanceAttributesByName("python", counter="Virtual Bytes")	 
	print "Available Bytes = ", GetPerformanceAttributes("Memory", "Available Bytes")
	print win32pdh.EnumObjectItems(None,None,"Memory", -1)
	
	
	
if __name__=='__main__':
	ShowAllProcesses()
#	test()
	print "Browsing for counters..."
	browse()
