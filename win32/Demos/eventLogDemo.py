import win32evtlog, traceback
import win32api, win32con
import win32security # To translate NT Sids to account names.

from win32evtlogutil import *

def ReadLog(computer, logType="Application", dumpEachRecord = 0):
	# read the entire log back.
	h=win32evtlog.OpenEventLog(computer, logType)
	numRecords = win32evtlog.GetNumberOfEventLogRecords(h)
	print "There are %d records" % numRecords
	
	num=0
	while 1:
		objects = win32evtlog.ReadEventLog(h, win32evtlog.EVENTLOG_BACKWARDS_READ|win32evtlog.EVENTLOG_SEQUENTIAL_READ, 0)
		if not objects:
			break
		for object in objects:
			# get it for testing purposes, but dont print it.
			msg = str(SafeFormatMessage(object, logType))
			if dumpEachRecord:
				if object.Sid is not None:
					try:
						domain, user, typ = win32security.LookupAccountSid(computer, object.Sid)
						sidDesc = "%s/%s" % (domain, user)
					except win32security.error:
						sidDesc = str(object.Sid)
					print "Following event associated with user", sidDesc
				else:
					print "Following event is not associated with a user:"
				print msg
		num = num + len(objects)

	if numRecords == num:
		print "Successfully read all records"
	else:
		print "Couldn't get all records - reported %d, but found %d" % (numRecords, num)
	win32evtlog.CloseEventLog(h)

def Usage():
	print "Writes an event to the event log."
	print "-l : Write lots (well, a few) different events"
	print "-r : Read and process (but dont print) the event log after writing"
	print "-c computerName : Read the log from the specified computer"

def test():
	# check if running on Windows NT, if not, display notice and terminate
	if win32api.GetVersion() & 0x80000000:
		print "This sample only runs on NT"
		return
		
	import sys, getopt
	opts, args = getopt.getopt(sys.argv[1:], "rlh?c:t:v")
	computer = None

	logType = "Application"
#	dll = win32api.GetModuleFileName(win32api.GetModuleHandle("win32evtlog.pyd"))
#	ReportEvent(appName, 1, strings=["The message text"], data = "Raw\0Data")
#	ReportEvent(appName, 1, strings=["A test security message"], data = "Raw\0Data", eventLogType="Security")
	verbose = 0

	if len(args)>0:
		print "Invalid args - please check the sources"
		return 1	
	for opt, val in opts:
		if opt == '-t':
			logType = val
		if opt == '-c':
			computer = val
		if opt in ['-h', '-?']:
			Usage()
			return
		if opt=='-r':
			ReadLog(computer, logType, verbose > 0)
		if opt=='-l':
			ReportEvent(logType, 2, strings=["The message text for event 2"], data = "Raw\0Data")
			ReportEvent(logType, 1, eventType=win32evtlog.EVENTLOG_WARNING_TYPE, strings=["A warning"], data = "Raw\0Data")
			ReportEvent(logType, 1, eventType=win32evtlog.EVENTLOG_INFORMATION_TYPE, strings=["An info"], data = "Raw\0Data")
		if opt=='-v':
			verbose = verbose + 1
	print "Worked OK."


if __name__=='__main__':
	test()

