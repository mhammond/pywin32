"""Testing pasing object between multiple COM threads

Uses standard COM marshalling to pass objects between threads.  Even 
though Python generally seems to work when you just pass COM objects
between threads, it shouldnt.

This shows the "correct" way to do it.

It shows that although we create new threads to use the Python.Interpreter,
COM marshalls back all calls to that object to the main Python thread,
which must be running a message loop (as this sample does).

When this test is run in "free threaded" mode (at this stage, you must 
manually mark the COM objects as "ThreadingModel=Free", or run from a 
service which has marked itself as free-threaded), then no marshalling
is done, and the Python.Interpreter object start doing the "expected" thing
- ie, it reports being on the same thread as its caller!

Python.exe needs a good way to mark itself as FreeThreaded - at the moment
this is a pain in the but!

"""

import thread, traceback
import win32com.client
import win32event, win32api
import pythoncom

from testservers import TestInterp

freeThreaded = 1

def TestInterpInThread(stopEvent, interp):
	try:
		DoTestInterpInThread(interp)
	finally:
		win32event.SetEvent(stopEvent)

def DoTestInterpInThread(interp):
		try:
			pythoncom.CoInitialize()
			myThread = win32api.GetCurrentThreadId()

			if freeThreaded:
				interp = pythoncom.CoGetInterfaceAndReleaseStream(interp, pythoncom.IID_IDispatch)
				interp = win32com.client.Dispatch(interp)

			TestInterp(interp)
			interp.Exec("import win32api")
			print "The test thread id is %d, Python.Interpreter's thread ID is %d" % (myThread, interp.Eval("win32api.GetCurrentThreadId()"))
			interp = None
			pythoncom.CoUninitialize()
		except:
			traceback.print_exc()

def BeginThreadsSimpleMarshal(numThreads):
	"""Creates multiple threads using simple (but slower) marshalling.
	
	Single interpreter object, but a new stream is created per thread.
	
	Returns the handles the threads will set when complete.
	"""
	interp = win32com.client.Dispatch("Python.Interpreter")
	ret = []
	for i in range(numThreads):
		hEvent = win32event.CreateEvent(None, 0, 0, None)
		interpStream = pythoncom.CoMarshalInterThreadInterfaceInStream(pythoncom.IID_IDispatch, interp._oleobj_)
		thread.start_new(TestInterpInThread, (hEvent, interpStream))
		ret.append(hEvent)
	return ret

#
# NOTE - this doesnt quite work - Im not even sure it should, but Greg reckons
# you should be able to avoid the marshal per thread!
def BeginThreadsFastMarshal(numThreads):
	"""Creates multiple threads using fast (but complex) marshalling.
	
	The marshal stream is created once, and each thread uses the same stream
	
	Returns the handles the threads will set when complete.
	"""
	interp = win32com.client.Dispatch("Python.Interpreter")
	if freeThreaded:
		interp = pythoncom.CoMarshalInterThreadInterfaceInStream(pythoncom.IID_IDispatch, interp._oleobj_)
	ret = []
	
	for i in range(numThreads):
		hEvent = win32event.CreateEvent(None, 0, 0, None)
		thread.start_new(TestInterpInThread, (hEvent, interp))
		ret.append(hEvent)
	return ret

def test(fn):
	print "The main thread is %d" % (win32api.GetCurrentThreadId())
	events = fn(2)
	numFinished = 0
	while 1:
		try:
			# Specifying "bWaitAll" here seems to prevent messages???
			rc = win32event.MsgWaitForMultipleObjects(events, 0, 2000, win32event.QS_ALLINPUT)
			if rc >= win32event.WAIT_OBJECT_0 and rc < win32event.WAIT_OBJECT_0+len(events):
				numFinished = numFinished + 1
				if numFinished >= len(events):
					break
			elif rc==win32event.WAIT_OBJECT_0 + len(events): # a message
				# This is critical - whole apartment model demo will hang.
				pythoncom.PumpWaitingMessages()
			else: # Timeout
				print "Waiting for thread to stop with interfaces=%d, gateways=%d" % (pythoncom._GetInterfaceCount(), pythoncom._GetGatewayCount())
		except KeyboardInterrupt:
			break
			
if __name__=='__main__':
	test(BeginThreadsSimpleMarshal)
#	test(BeginThreadsFastMarshal)
	win32api.Sleep(500)
	# Doing CoUninit here stop Pythoncom.dll hanging when DLLMain shuts-down the process
	pythoncom.CoUninitialize()
	if pythoncom._GetInterfaceCount()!=0 or pythoncom._GetGatewayCount()!=0:
		print "Done with interfaces=%d, gateways=%d" % (pythoncom._GetInterfaceCount(), pythoncom._GetGatewayCount())
	else:
		print "Done."
